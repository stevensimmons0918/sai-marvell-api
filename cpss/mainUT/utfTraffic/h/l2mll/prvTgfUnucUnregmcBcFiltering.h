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
* @file prvTgfUnucUnregmcBcFiltering.h
*
* @brief L2 MLL Unknown Unicast, Unregistered Multicast and Broadcast filtering
* test definitions
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfUnucUnregmcBcFilteringh
#define __prvTgfUnucUnregmcBcFilteringh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfUnucUnregmcBcFilteringConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfUnucUnregmcBcFilteringConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfUnucUnregmcBcFilteringTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfUnucUnregmcBcFilteringTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfUnucUnregmcBcFilteringConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfUnucUnregmcBcFilteringConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfUnucUnregmcBcFilteringUsePclMirrorToCpu function
* @endinternal
*
* @brief   Set/Unset the need to use PCL mirror_to_cpu action.
*/
GT_VOID prvTgfUnucUnregmcBcFilteringUsePclMirrorToCpu
(
    IN GT_BOOL  usePclMirrorToCpu
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfUnucUnregmcBcFilteringh */


