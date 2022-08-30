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
* @file prvTgfEgressMirrorErspanUseCase.h
*
* @brief Egress mirroring ERSPAN Type II and Type III use cases testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfEgressMirrorErspanUseCaseh
#define __prvTgfEgressMirrorErspanUseCaseh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <mirror/prvTgfMirror.h>

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/**
* internal prvTgfEgressMirrorErspanConfigSet function
* @endinternal
*
* @brief   Egress mirroring ERSPAN use case configurations
*/
GT_VOID prvTgfEgressMirrorErspanConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorErspanTest function
* @endinternal
*
* @brief   Egress mirroring ERSPAN use case test.
*/
GT_VOID prvTgfEgressMirrorErspanTest
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorErspanVerification function
* @endinternal
*
* @brief   Egress mirroring ERSPAN use case verification.
*/
GT_VOID prvTgfEgressMirrorErspanVerification
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorErspanConfigRestore function
* @endinternal
*
* @brief   Egress mirroring ERSPAN configurations restore.
*/
GT_VOID prvTgfEgressMirrorErspanConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIngressMirrorErspanUseCaseh */



