/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfUnifiedSrIpv6UseCase.h
*
* DESCRIPTION:
*       MPLS Unified SR use case API
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvTgfUnifiedSrIpv6UseCaseh
#define __prvTgfUnifiedSrIpv6UseCaseh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfUnifiedSrIpv6ConfigurationSet function
* @endinternal
*
* @brief   Set Entropy Label Configuration
*/
GT_VOID prvTgfUnifiedSrIpv6ConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfUnifiedSrIpv6TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfUnifiedSrIpv6TrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfUnifiedSrIpv6ConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @retval GT_OK                    - on success
*/
GT_VOID prvTgfUnifiedSrIpv6ConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
#endif /* __cplusplus */

#endif /* __prvTgfUnifiedSrIpv6UseCaseh */
