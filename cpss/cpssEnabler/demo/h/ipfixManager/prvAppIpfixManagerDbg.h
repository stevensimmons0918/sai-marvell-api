/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file prvAppIpfixManager.h
*
* @brief App demo Ipfix manager debug apis header file.
*
* @version   1
********************************************************************************/

#ifndef __prvAppIpfixManagerDbgh
#define __prvAppIpfixManagerDbgh

#include <gtOs/gtOsTask.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/ipfixManager/prvCpssDxChIpfixManager.h>
#include <cpss/dxCh/dxChxGen/ipfixManager/cpssDxChIpfixManager.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @internal appDemoIpfixManagerDebugCountersReset function
 *
 * @brief  Reset debug counters
 *
 */
GT_STATUS appDemoIpfixManagerDebugCountersReset
(
    GT_VOID
);

/**
 * @internal appDemoIpfixManagerConfigCheck function
 * @endinternal
 *
 * @brief  Read config parameters from firmware and check
 *         they are in sync with the configuration in the application database.
 *
 * @param[in] devNum   -  device number
 *
 * @return GT_OK  - on success
 */
GT_STATUS appDemoIpfixManagerConfigCheck
(
    GT_U8           devNum
);


/**
 * @internal appDemoIpfixManagerDebugCountersDump function
 *
 * @brief  Dump debug counters
 *
 */
GT_STATUS appDemoIpfixManagerDebugCountersDump
(
    GT_VOID
);

/**
 * @internal appDemoIpfixManagerStatisticsDump function
 *
 * @brief  Dump flow statistics
 *
 */
GT_STATUS appDemoIpfixManagerStatisticsDump
(
    GT_VOID
);

/**
 * @internal appDemoIpfixManagerDebugCountersGet function
 *
 * @brief Get debug counters
 *
 * @param[in]  dbgCountersPtr - (pointer to) debug counters structure
 */
GT_STATUS appDemoIpfixManagerDebugCountersGet
(
    PRV_APP_IPFIX_MGR_DBG_COUNTERS_STC *dbgCountersPtr
);

/**
 * @internal appDemoIpfixManagerFlowGet function
 *
 * @brief Get debug counters
 *
 * @param[in]  dbgCountersPtr - (pointer to) debug counters structure
 */
GT_STATUS appDemoIpfixManagerFlowGet
(
    IN  GT_U32                      portGroupId,
    IN  GT_U32                      flowId,
    OUT PRV_APP_IPFIX_MGR_FLOWS_STC *flowPtr
);

/**
 * @internal appDemoIpfixManagerTestParamsSet function
 * @endinternal
 *
 * @brief Set test params database structure
 *
 * @param[in] testParamsPtr - (pointer to) test params structure
 *
 * @retval GT_OK    - on success
 */
GT_STATUS appDemoIpfixManagerTestParamsSet
(
    IN PRV_APP_IPFIX_MGR_TEST_PARAMS_STC *testParamsPtr
);

/**
 * @internal appDemoIpfixManagerTestParamsGet function
 * @endinternal
 *
 * @brief Get test params database structure
 *
 * @param[out] testParamsPtr - (pointer to) test params structure
 *
 * @retval GT_OK    - on success
 */
GT_STATUS appDemoIpfixManagerTestParamsGet
(
    OUT PRV_APP_IPFIX_MGR_TEST_PARAMS_STC *testParamsPtr
);

/**
 * @internal appDemoIpfixManagerCpuRxDumpEnable function
 * @endinternal
 *
 * @brief Enable CPU Rx Dump
 *
 * @param[in] enable - GT_TRUE/GT_FALSE enable/disable
 *
 * @retval GT_OK    - on success
 */
GT_STATUS  appDemoIpfixManagerCpuRxDumpEnable
(
    IN GT_BOOL enable
);

GT_VOID  appDemoIpfixManagerDataRecordDump
(
    PRV_APP_IPFIX_MGR_DATA_PKT_STC   *dataPktPtr
);

GT_STATUS appDemoIpfixManagerAgingEnable
(
    IN GT_BOOL     enable
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvAppIpfixManagerDbgh */
