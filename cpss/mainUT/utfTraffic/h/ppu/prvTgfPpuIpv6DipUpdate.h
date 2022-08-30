/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfPpuIpv6DipUpdate.h
*
* DESCRIPTION:
*       Update Ipv6 DIP using Programmable Parser Unit (PPU)
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvTgfPpuIpv6DipUpdate_h
#define __prvTgfPpuIpv6DipUpdate_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @internal prvTgfPpuIpv6DipUpdateBaseConfigSet function
 * @endinternal
 *
 * @brief  set basic port and vlan configuration
*/
GT_VOID prvTgfPpuIpv6DipUpdateBaseConfigSet
(
    GT_VOID
);

/**
 * @internal prvTgfPpuIpv6DipUpdatePpuConfigSet function
 * @endinternal
 *
 * @brief  set PPU configuration
 */
GT_VOID prvTgfPpuIpv6DipUpdatePpuConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPpuIpv6DipUpdateRoutingConfigSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
*/
GT_VOID prvTgfPpuIpv6DipUpdateRoutingConfigSet
(
    GT_VOID
);

/**
 * @internal prvTgfPpuIpv6DipUpdatePclConfigSet function
 * @endinternal
 *
 * @brief  Set IPCL config to match upate ipv6 dip
 */
GT_VOID prvTgfPpuIpv6DipUpdatePclConfigSet
(
    GT_VOID
);

/**
 * @internal prvTgfPpuIpv6DipUpdateTrafficGenerate function
 * @endinternal
 *
 * @brief  Generate Traffic and compare results
 */
GT_VOID prvTgfPpuIpv6DipUpdateTrafficGenerate
(
    GT_VOID
);

/**
 * @internal prvTgfPpuIpv6DipUpdateRestore function
 * @endinternal
 *
 * @brief Restore original configuration.
 */
GT_VOID prvTgfPpuIpv6DipUpdateRestore
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPpuIpv6DipUpdate_h */

