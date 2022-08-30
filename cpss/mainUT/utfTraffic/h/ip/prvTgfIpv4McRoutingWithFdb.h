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
* @file prvTgfIpv4McRoutingWithFdb.h
*
* @brief Basic IPV4 UC Routing
*
* @version   9
********************************************************************************
*/
#ifndef __prvTgfIpv4McRoutingWithFdbh
#define __prvTgfIpv4McRoutingWithFdbh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfIpGen.h>

/**
* @internal prvTgfBasicIpv4McRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpv4McRoutingWithFdbBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpv4McRoutingWithFdbBaseVrfIdSet function
* @endinternal
*
* @brief   Assign VrfId
*
* @param   index - 0/1 first/second VrfId assingment
*/
GT_VOID prvTgfIpv4McRoutingWithFdbBaseVrfIdSet
(
    GT_U32 index
);

/**
* @internal prvTgfIpv4McRoutingWithFdbPclActionVrfIdAssignRouteConfigSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfIpv4McRoutingWithFdbPclActionVrfIdAssignRouteConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv4McRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
GT_VOID prvTgfIpv4McRoutingWithFdbRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpv4McRoutingWithFdbLpmLeafSet function
* @endinternal
*
* @brief   Set LPM Leaf Configuration with priority
*/
GT_VOID prvTgfIpv4McRoutingWithFdbLpmLeafSet
(
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT priority
);

/**
* @internal prvTgfBasicIpv4McRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpv4McRoutingWithFdbTrafficGenerate
(
    GT_BOOL expectFdbRouting
);

/**
* @internal prvTgfBasicIpv4McRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfIpv4McRoutingWithFdbConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpv4McRoutingWithFdbh */
