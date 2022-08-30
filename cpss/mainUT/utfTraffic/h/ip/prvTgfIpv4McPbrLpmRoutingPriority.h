/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvTgfIpv4McPbrLpmRoutingPriority.h
*
* @brief Policy and LPM based IPV4 MC Routing.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpv4McPbrLpmRoutingPriorityh
#define __prvTgfIpv4McPbrLpmRoutingPriorityh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <common/tgfLpmGen.h>
#include <common/tgfIpGen.h>

/**
* @internal prvTgfIpv4McPbrLpmRoutingPriorityBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpv4McPbrLpmRoutingPriorityBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpv4McPbrLpmRoutingPriorityPCLEntrySet function
* @endinternal
*
* @brief   Set PCL configuration
*/
GT_VOID prvTgfIpv4McPbrLpmRoutingPriorityPCLConfigSet
(
        GT_VOID
);

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityLpmLeafEntrySet
*           function
* @endinternal
*
* @brief   Set routing priority between LPM and FDB loopup
*
* @param[in] priority                 - priority
* @param[in] applyPbr                 - apply policy based routing
* @param[in] isSGLookup               - GT_TRUE  - (S,G) lookup
*                                       GT_FALSE - (*,G) lookup
* @param[in] deleteExisted            - delete existed prefix
*
*/
GT_VOID prvTgfIpv4McPbrLpmRoutingPriorityLpmLeafEntrySet
(
    IN CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT    priority,
    IN GT_BOOL                                       applyPbr,
    IN GT_BOOL                                       isSGLookup,
    IN GT_BOOL                                       deleteExisted
);

/**
* @internal prvTgfIpv4McPbrLpmRoutingPriorityRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfIpv4McPbrLpmRoutingPriorityRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpv4McPbrLpmRoutingPriorityTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and match with expected values.
*
* @param[in]  expectTraffic    - GT_TRUE  - expect traffic
*                                GT_FALSE - expect no traffic
* @param[in]  expectPbrRouting   GT_TRUE -  expect PBR routing and match pbr Vlan.
*                                GT_FALSE - expect LPM rougint and match lpm vlan
*
*/
GT_VOID prvTgfIpv4McPbrLpmRoutingPriorityTrafficGenerate
(
    GT_BOOL expectTraffic,
    GT_BOOL expectPbrRouting
);

/**
* @internal prvTgfIpv4McPbrLpmRoutingPriorityConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration 
*
*/
GT_VOID prvTgfIpv4McPbrLpmRoutingPriorityConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpv4McPbrLpmRoutingPriorityh */


