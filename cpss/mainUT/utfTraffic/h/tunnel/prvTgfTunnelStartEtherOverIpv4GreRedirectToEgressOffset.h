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
* @file prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressOffset.h
*
* @brief Tunnel start Ethernet over Ipv4 GRE redirect to egress (with offset)
*
* @version   2
********************************************************************************
*/


/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressBridgeConfigSetOffset function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressBridgeConfigSetOffset
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressTunnelConfigSetOffset function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressTunnelConfigSetOffset
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressPclConfigSetOffset function
* @endinternal
*
* @brief   Set Pcl Configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressPclConfigSetOffset
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressTrafficGenerateOffset function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:11,
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressTrafficGenerateOffset
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressConfigurationRestoreOffset function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressConfigurationRestoreOffset
(
    GT_VOID
);


