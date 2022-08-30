/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfPpuGeneve.h
*
* DESCRIPTION:
*       Programmable Parser Unit (PPU)
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvTgfPpuGen_h
#define __prvTgfPpuGen_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/********************************************************************************/
/*                                Test Functions                                */
/********************************************************************************/

/**
 * @internal prvTgfPpuGeneveBasicConfigSet function
 * @endinternal
 *
 * @brief    Set basic port vlan configuration
 *
 */
GT_VOID prvTgfPpuGeneveBasicConfigSet
(
    GT_VOID
);


/**
 * @internal prvTgfPpuGeneveVariableTunnelLengthConfigSet function
 * @endinternal
 *
 * @brief    Set variable tunnel length configuration
 *
 */
GT_VOID prvTgfPpuGeneveVariableTunnelLengthConfigSet
(
    GT_VOID
);

/**
 * @internal prvTgfPpuGeneveTtiConfigSet function
 * @endinternal
 *
 * @brief    Set TTI configuration for tunnel terminating IPV4 UDP packet.
 *
 */
GT_VOID prvTgfPpuGeneveTtiConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPpuGenevePpuConfigSet function
* @endinternal
*
* @brief  Set configuration for Geneve Packet
*           1. Initialize PPU
*           2. Configure K_stg0, K_stg1, K_stg2 and DAU
*/
GT_VOID prvTgfPpuGenevePpuConfigSet
(
        GT_VOID
);

/**
* @internal prvTgfPpuGeneveTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet and capture:
*         Check captured packet.
*/
GT_VOID prvTgfPpuGeneveTrafficGenerate
(
    GT_BOOL isTunnelTerminated
);

/**
* @internal prvTgfPpuGeneveRestore function
* @endinternal
*
* @brief  Restore Configuration
*/
GT_VOID prvTgfPpuGeneveRestore
(
        GT_VOID
);

/**
* @internal prvTgfPpuGeneveLoopbackConfigure function
* @endinternal
*
* @brief  Set configuration for Geneve Packet
*/
GT_VOID prvTgfPpuGeneveLoopbackConfigure
(
        GT_VOID
);

/**
* @internal prvTgfPpuGeneveLoopbackTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet and capture:
*         Check captured packet.
*/
GT_VOID prvTgfPpuGeneveLoopbackTrafficGenerate
(
        GT_VOID
);

/**
* @internal prvTgfPpuGeneveProtectedWindowTest function
* @endinternal
*
* @brief  Write into protected window and capture interrupt
*/
GT_VOID prvTgfPpuGeneveProtectedWindowTest
(
        GT_VOID
);

/**
* @internal prvTgfPpuGeneveLoopbackErrorTest function
* @endinternal
*
* @brief  Create loopback error and capture interrupt
*/
GT_VOID prvTgfPpuGeneveLoopbackErrorTest
(
        GT_VOID
);

/**
* @internal prvTgfPpuGeneveOorErrorTest function
* @endinternal
*
* @brief  Create out of range error and capture interrupt
*/
GT_VOID prvTgfPpuGeneveOorErrorTest
(
        GT_VOID
);

/**
* @internal prvTgfPpuGeneveDebugCountersTest function
* @endinternal
*
* @brief  Enable debug counters
*         Send packet
*         Read and verify debug counters
*/
GT_VOID prvTgfPpuGeneveDebugCountersTest
(
        GT_VOID
);

/**
* @internal prvTgfPpuGeneveLoopbackRestore function
* @endinternal
*
* @brief  Restore Configuration
*/
GT_VOID prvTgfPpuGeneveLoopbackRestore
(
        GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPpuGen_h */
