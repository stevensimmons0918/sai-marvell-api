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
* @file prvTgfBrgVlanDsaTagCheckOnRouting.h
*
* @brief DSA tag Vlan Id assignment for untagged packet
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgVlanDsaTagCheckOnRouting
#define __prvTgfBrgVlanDsaTagCheckOnRouting

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfCommon.h>

/**
* @internal prvTgfBrgVlanDsaTagCheckOnRoutingConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID prvTgfBrgVlanDsaTagCheckOnRoutingConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanDsaTagCheckOnRoutingCscdConfigSet function
* @endinternal
*
* @brief   Set ingress cascade configuration
*/
GT_VOID prvTgfBrgVlanDsaTagCheckOnRoutingCscdConfigSet
(
		GT_VOID
);

/**
* @internal prvTgfBrgVlanDsaTagCheckOnRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfBrgVlanDsaTagCheckOnRoutingTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanDsaTagCheckOnRoutingConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanDsaTagCheckOnRoutingConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgVlanDsaTagCheckOnRouting */
