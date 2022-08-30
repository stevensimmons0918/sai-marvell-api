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
* @file prvTgfOamSrv6OneSegmentUseCase.h
*
* @brief OAM Egress Node use case testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfOamSrv6OneSegmentUseCaseh
#define __prvTgfOamSrv6OneSegmentUseCaseh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <oam/prvTgfOamGeneral.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/**
* internal tgfSrv6OneSegmentConfigSet function
* @endinternal
*
* @brief   SRV6 One Segment use case configurations
*/
GT_VOID tgfSrv6OneSegmentConfigSet
(
    GT_VOID
);

/**
* @internal tgfSrv6OneSegmentTest function
* @endinternal
*
* @brief  SRv6 One segment use case test.
*/
GT_VOID tgfSrv6OneSegmentTest
(
    GT_VOID
);

/**
* @internal tgfSrv6OneSegmentConfigRestore function
* @endinternal
*
* @brief   SRv6 One segment use case configurations restore.
*/
GT_VOID tgfSrv6OneSegmentConfigRestore
(
    GT_VOID
);

/**
* internal tgfSrv6TwoSegmentConfigSet function
* @endinternal
*
* @brief   SRV6 Two Segment use case configurations
*/
GT_VOID tgfSrv6TwoSegmentConfigSet
(
    GT_VOID
);

/**
* internal tgfSrv6ThreeSegmentConfigSet function
* @endinternal
*
* @brief   SRV6 Three Segment use case configurations
*/
GT_VOID tgfSrv6ThreeSegmentConfigSet
(
    GT_VOID
);

/**
* @internal tgfSrv6TwoThreeSegmentTest function
* @endinternal
*
* @brief  SRv6 Two/Three segment use case test.
*/
GT_VOID tgfSrv6TwoThreeSegmentTest
(
    CPSS_DXCH_PHA_THREAD_TYPE_ENT           threadType
);

/**
* @internal tgfSrv6TwoThreeSegmentConfigRestore function
* @endinternal
*
* @brief   SRv6 Two/Three segment use case configurations restore.
*/
GT_VOID tgfSrv6TwoThreeSegmentConfigRestore
(
    GT_VOID
);

/**
* internal tgfSrv6BestEffortTunnelConfigSet function
* @endinternal
*
* @brief   SRV6 Best Effort Tunnel use case configurations
*/
GT_VOID tgfSrv6BestEffortTunnelConfigSet
(
    GT_VOID
);

/**
* @internal tgfSrv6BestEffortTunnelTest function
* @endinternal
*
* @brief  SRv6 Best Effort Tunnel use case test.
*/
GT_VOID tgfSrv6BestEffortTunnelTest
(
    GT_VOID
);

/**
* @internal tgfSrv6BestEffortTunnelConfigRestore function
* @endinternal
*
* @brief   SRv6 Best Effort Tunnel use case configurations restore.
*/
GT_VOID tgfSrv6BestEffortTunnelConfigRestore
(
    GT_VOID
);

/**
* internal tgfSrv6BestEffortTunnelParseOnBorderNodeConfigSet function
* @endinternal
*
* @brief   SRV6 Best Effort Tunnel Parse on Border Node use case configurations
*/
GT_VOID tgfSrv6BestEffortTunnelParseOnBorderNodeConfigSet
(
    GT_VOID
);

/**
* @internal tgfSrv6BestEffortTunnelParseOnBorderNodeTest function
* @endinternal
*
* @brief  SRv6 Best Effort Tunnel Parse on Border Node use case test.
*/
GT_VOID tgfSrv6BestEffortTunnelParseOnBorderNodeTest
(
    GT_VOID
);

/**
* @internal tgfSrv6BestEffortTunnelParseOnBorderNodeConfigRestore function
* @endinternal
*
* @brief   SRv6 Best Effort Tunnel Parse on Border Node use case configurations restore.
*/
GT_VOID tgfSrv6BestEffortTunnelParseOnBorderNodeConfigRestore
(
    GT_VOID
);

/**
* internal tgfSrv6BestEffortTunnelParseOnLeafNodeConfigSet function
* @endinternal
*
* @brief   SRV6 Best Effort Tunnel Parse on Leaf Node use case configurations
*/
GT_VOID tgfSrv6BestEffortTunnelParseOnLeafNodeConfigSet
(
    GT_VOID
);

/**
* @internal tgfSrv6BestEffortTunnelParseOnLeafNodeTest function
* @endinternal
*
* @brief  SRv6 Best Effort Tunnel Parse on Leaf Node use case test.
*/
GT_VOID tgfSrv6BestEffortTunnelParseOnLeafNodeTest
(
    GT_VOID
);

/**
* @internal tgfSrv6BestEffortTunnelParseOnLeafNodeConfigRestore function
* @endinternal
*
* @brief   SRv6 Best Effort Tunnel Parse on Leaf Node use case configurations restore.
*/
GT_VOID tgfSrv6BestEffortTunnelParseOnLeafNodeConfigRestore
(
    GT_VOID
);

/**
* internal tgfSrv6GSidTunnelOneSrhContainerConfigSet function
* @endinternal
*
* @brief   SRV6 G-SID Tunnel One SRH Container use case configurations
*/
GT_VOID tgfSrv6GSidTunnelOneSrhContainerConfigSet
(
    GT_VOID
);

/**
* @internal tgfSrv6GSidTunnelOneSrhContainerTest function
* @endinternal
*
* @brief  SRv6 G-SID Tunnel One SRH Container use case test.
*/
GT_VOID tgfSrv6GSidTunnelOneSrhContainerTest
(
    GT_VOID
);

/**
* @internal tgfSrv6GSidTunnelOneSrhContainerConfigRestore function
* @endinternal
*
* @brief   SRv6 G-SID Tunnel One SRH Container use case configurations restore.
*/
GT_VOID tgfSrv6GSidTunnelOneSrhContainerConfigRestore
(
    GT_VOID
);

/**
* internal tgfSrv6GSidTunnelTwoSrhContainerConfigSet function
* @endinternal
*
* @brief   SRV6 G-SID Tunnel Two SRH Container use case configurations
*/
GT_VOID tgfSrv6GSidTunnelTwoSrhContainerConfigSet
(
    GT_VOID
);

/**
* @internal tgfSrv6GSidTunnelTwoSrhContainerTest function
* @endinternal
*
* @brief  SRv6 G-SID Tunnel Two SRH Container use case test.
*/
GT_VOID tgfSrv6GSidTunnelTwoSrhContainerTest
(
    CPSS_DXCH_PHA_THREAD_TYPE_ENT           threadType
);

/**
* @internal tgfSrv6GSidTunnelTwoSrhContainerConfigRestore function
* @endinternal
*
* @brief   SRv6 G-SID Tunnel Two SRH Container use case configurations restore.
*/
GT_VOID tgfSrv6GSidTunnelTwoSrhContainerConfigRestore
(
    GT_VOID
);

/**
* internal tgfSrv6GSidTunnelThreeSrhContainerConfigSet function
* @endinternal
*
* @brief   SRV6 G-SID Tunnel Three SRH Container use case configurations
*/
GT_VOID tgfSrv6GSidTunnelThreeSrhContainerConfigSet
(
    GT_VOID
);

/**
* @internal tgfSrv6GSidTunnelThreeSrhContainerTest function
* @endinternal
*
* @brief  SRv6 G-SID Tunnel Three SRH Container use case test.
*/
GT_VOID tgfSrv6GSidTunnelThreeSrhContainerTest
(
    CPSS_DXCH_PHA_THREAD_TYPE_ENT           threadType
);

/**
* @internal tgfSrv6GSidTunnelThreeSrhContainerConfigRestore function
* @endinternal
*
* @brief   SRv6 G-SID Tunnel Three SRH Container use case configurations restore.
*/
GT_VOID tgfSrv6GSidTunnelThreeSrhContainerConfigRestore
(
    GT_VOID
);

/**
* @internal tgfSrv6GSidTunnelCoc32EndNodeConfigSet function
* @endinternal
*
* @brief   SRV6 G-SID Tunnel Coc32 End Node use case configurations
*/
GT_VOID tgfSrv6GSidTunnelCoc32EndNodeConfigSet
(
    GT_VOID
);

/**
* @internal tgfSrv6GSidTunnelCoc32EndNodeTest function
* @endinternal
*
* @brief  SRv6 G-SID Tunnel Coc32 End Node use case test.
*/
GT_VOID tgfSrv6GSidTunnelCoc32EndNodeTest
(
    GT_VOID
);

/**
* @internal tgfSrv6GSidTunnelCoc32EndNodeConfigRestore function
* @endinternal
*
* @brief   SRv6 G-SID Tunnel Coc32 End Node use case configurations restore.
*/
GT_VOID tgfSrv6GSidTunnelCoc32EndNodeConfigRestore
(
    GT_VOID
);

GT_VOID tgfSrv6TunnelEndNodeDipUpdateVerificationTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfOamSrv6OneSegmentUseCaseh */

