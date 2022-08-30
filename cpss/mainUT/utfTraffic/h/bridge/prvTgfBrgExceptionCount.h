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
* @file prvTgfBrgExceptionCount.h
*
* @brief Fdb ipv4 uc routing checking Aging enable/disable functionality
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgExceptionCounth
#define __prvTgfBrgExceptionCounth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <common/tgfBridgeGen.h>


/**
* @internal prvTgfRouterBrgExceptionCountConfigurationSet function
* @endinternal
*
* @brief   Set Configuration to enable special services for bridged traffic
*
*/
GT_VOID prvTgfRouterBrgExceptionCountConfigurationSet
(
    GT_VOID
);


/**
* @internal prvTgfRouterBrgExceptionCountConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfRouterBrgExceptionCountConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgExceptionCountTrafficGenerate function
* @endinternal
*
* @brief   Checked expection count by comparing register value
*/
GT_VOID prvTgfBrgExceptionCountTrafficGenerate
(
	GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgExceptionCounth */



