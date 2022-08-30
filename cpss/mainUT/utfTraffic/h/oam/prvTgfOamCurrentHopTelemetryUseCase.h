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
* @file prvTgfOamCurrentHopTelemetryUseCase.h
*
* @brief OAM Current Hop telemetry usecase testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfOamCurrentHopTelemetryUseCaseh
#define __prvTgfOamCurrentHopTelemetryUseCaseh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <oam/prvTgfOamGeneral.h>

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/**
* internal tgfOamCurrentHopTelemetryConfigSet function
* @endinternal
*
* @brief   CurrentHop Telemetry use case configurations
*/
GT_VOID tgfOamCurrentHopTelemetryConfigSet
(
    GT_VOID
);

/**
* @internal tgfOamCurrentHopTelemetryTest function
* @endinternal
*
* @brief   CurrentHop Telemetry use case test.
*/
GT_VOID tgfOamCurrentHopTelemetryTest
(
    GT_VOID
);

/**
* @internal tgfOamCurrentHopTelemetryConfigRestore function
* @endinternal
*
* @brief   CurrentHop Telemetry use case configurations restore.
*/
GT_VOID tgfOamCurrentHopTelemetryConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfOamGenericKeepaliveh */



