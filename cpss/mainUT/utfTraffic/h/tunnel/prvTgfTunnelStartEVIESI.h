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
* @file prvTgfTunnelStartEVIESI.h
*
* @brief Tunnel Start: Adding EVI and ESI labels for IPv4 (MC) over IPv6 packet
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfTunnelStartEVIESIh
#define __prvTgfTunnelStartEVIESIh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfTunnelStartEVIESIConfigurationSet function
* @endinternal
*
* @brief   Set base Configuration
*
*/
GT_VOID prvTgfTunnelStartEVIESIConfigurationSet
(
    GT_VOID
);


/**
* @internal prvTgfTunnelStartEVIESITrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelStartEVIESITrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartEVIESIConfigurationReset function
* @endinternal
*
* @brief   Reset base Configuration
*
*/
GT_VOID prvTgfTunnelStartEVIESIConfigurationReset
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelStartEVIESIh */



