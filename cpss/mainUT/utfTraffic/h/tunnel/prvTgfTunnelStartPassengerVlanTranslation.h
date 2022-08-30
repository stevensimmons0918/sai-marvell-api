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
* @file prvTgfTunnelStartPassengerVlanTranslation.h
*
* @brief Tunnel Start: Vlan Translation on passenger.
*
* @version   1
********************************************************************************
*/


#ifndef __prvTgfTunnelStartPassengerVlanTranslation
#define __prvTgfTunnelStartPassengerVlanTranslation

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfTunnelGen.h>

/** @internal prvTgfTunnelStartPassengerVlanTranslationConfigSet function
* @endinternal
*
* @brief   Set tcam tti configuration
*/
GT_VOID prvTgfTunnelStartPassengerVlanTranslationConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartPassengerVlanTranslationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelStartPassengerVlanTranslationTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartPassengerVlanTranslationConfigRestore function
* @endinternal
*
* @brief   Restore Configuration
*/
GT_VOID prvTgfTunnelStartPassengerVlanTranslationConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelStartPassengerVlanTranslation */
