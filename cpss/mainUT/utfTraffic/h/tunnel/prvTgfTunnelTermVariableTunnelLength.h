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
* @file prvTgfTunnelTermVariableTunnelLength.h
*
* @brief Verify the functionality of Tunnel Term TTI Ipv4 ARP Metadata
*
* @version   1
********************************************************************************
*/
#ifndef prvTgfTunnelTermVariableTunnelLengthh
#define prvTgfTunnelTermVariableTunnelLengthh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfTunnelTermVariableTunnelLengthConfigure function
* @endinternal
*
* @brief  Set configuration for Geneve Packet
*         1. Set Port Configuration
*         2. Configure Tunnel header length profile
*         3. Configure TTI
*/
GT_VOID prvTgfTunnelTermVariableTunnelLengthConfigure
(
        GT_VOID
);

/**
* @internal prvTgfTunnelTermVariableTunnelLengthGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet and capture:
*         Check captured packet.
*/
GT_VOID prvTgfTunnelTermVariableTunnelLengthGenerate
(
        GT_VOID
);

/**
* @internal prvTgfTunnelTermVariableTunnelLengthRestore function
* @endinternal
*
* @brief  Restore Configuration
*/
GT_VOID prvTgfTunnelTermVariableTunnelLengthRestore
(
        GT_VOID
);

/**
* @internal prvTgfVxlanGbpConfigSet function
* @endinternal
*
* @brief  Set configuration for VXLAN-GBP Test
*
* @param[in] protocol       - VXLAN tunnel is ipv4 or ipv6 based
*                            GT_TRUE - VXLAN tunnel is ipv6 based
*                            GT_FALSE - VXLAN tunnel is ipv4 based
* @param[in] udpSrcPortMode - VXLAN tunnel UDP source port mode
*                           GT_TRUE - source port mode is based on IP generic
*                           GT_FALSE - source port mode is based on packet hash
*/
GT_VOID prvTgfVxlanGbpConfigSet
(
    GT_BOOL protocol,
    GT_BOOL udpSrcPortMode
);

/**
* @internal prvTgfVxlanGbpTest function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet and capture:
*         Check captured packet.
*/
GT_VOID prvTgfVxlanGbpTest
(
    GT_VOID
);

/**
* @internal prvTgfVxlanGbpVerification function
* @endinternal
*
* @brief  VXLAN-GBP Packet Verification
*
* @param[in] protocol    - VXLAN tunnel is ipv4 or ipv6 based
*                           GT_TRUE - VXLAN tunnel is ipv6 based
*                           GT_FALSE - VXLAN tunnel is ipv4 based
* @param[in] udpSrcPortMode - VXLAN tunnel UDP source port mode
*                           GT_TRUE - source port mode is based on IP generic
*                           GT_FALSE - source port mode is based on packet hash
*/
GT_VOID prvTgfVxlanGbpVerification
(
    GT_BOOL protocol,
    GT_BOOL udpSrcPortMode
);

/**
* @internal prvTgfVxlanGbpConfigRestore function
* @endinternal
*
* @brief  Restore Configuration
*/
GT_VOID prvTgfVxlanGbpConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prvTgfTunnelTermVariableTunnelLengthbh */
