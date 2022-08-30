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
* @file  prvTgfPclIngressReflectAction.h
*
* @brief Reflect action test
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPclIngressReflectActionh
#define __prvTgfPclIngressReflectActionh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPclGen.h>

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_NUM   0

/**
* @internal prvTgfPclIngressReflectActionConfigurationSet function
* @endinternal
*
* @brief  Configure Ingress PCL Rule with:
*         action: byPass bridge and redirect commands
*         and mask : dip
*
*/
extern GT_VOID prvTgfPclIngressReflectActionConfigurationSet
(
    GT_PHYSICAL_PORT_NUM portNum
);

/**
* @internal prvTgfPclIngressReflectActionTrafficGenerateAndCheck function
* @endinternal
*
* @brief  Send traffic and check whether the packet reflects back to source port
*/
GT_VOID prvTgfPclIngressReflectActionTrafficGenerateAndCheck
(
    GT_PHYSICAL_PORT_NUM portNum
);

/**
* @internal prvTgfPclIngressReflectActionRestore function
* @endinternal
*
* @brief  restore test configurations
*/
GT_VOID prvTgfPclIngressReflectActionRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
