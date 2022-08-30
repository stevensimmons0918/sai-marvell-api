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
* @file prvTgfIpNonExactMatchRoutingIpv6Mc.h
*
* @brief Non-exact match IPv6 MC prefix routing.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpNonExactMatchRoutingIpv6Mc
#define __prvTgfIpNonExactMatchRoutingIpv6Mc

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfIpGen.h>

/**
* @internal prvTgfIpNonExactMatchRoutingIpv6McBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpNonExactMatchRoutingIpv6McBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpNonExactMatchRoutingIpv6McRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfIpNonExactMatchRoutingIpv6McRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpNonExactMatchRoutingIpv6McTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpNonExactMatchRoutingIpv6McTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpNonExactMatchRoutingIpv6McConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 1. Restore Route Configuration
*       2. Restore Base Configuration
*
*/
GT_VOID prvTgfIpNonExactMatchRoutingIpv6McConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpNonExactMatchRoutingIpv6Mc */

