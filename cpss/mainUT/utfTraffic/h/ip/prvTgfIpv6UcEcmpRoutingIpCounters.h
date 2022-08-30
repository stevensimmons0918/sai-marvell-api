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
* @file prvTgfIpv6UcEcmpRoutingIpCounters.c
*
* @brief IPV6 UC Routing with Ecmp Indirection and IP counters
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpv6UcEcmpRoutingIpCountersh
#define __prvTgfIpv6UcEcmpRoutingIpCountersh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <trafficEngine/tgfTrafficEngine.h>
#include <common/tgfLpmGen.h>



/**
* @internal prvTgfIpv6UcEcmpRoutingIpCountersBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpv6UcEcmpRoutingIpCountersBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpv6UcEcmpRoutingIpCountersLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
*/
GT_VOID prvTgfIpv6UcEcmpRoutingIpCountersLttRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpv6UcEcmpRoutingIpCountersTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] randomEnable             - whether the ECMP entry is set with  == GT_TRUE
*                                       None
*/
GT_VOID prvTgfIpv6UcEcmpRoutingIpCountersTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpv6UcEcmpRoutingIpCountersConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfIpv6UcEcmpRoutingIpCountersConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpv6UcEcmpRoutingIpCountersh */
