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
* @file prvTgfOamEgressNodeUseCase.h
*
* @brief OAM Egress Node use case testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfOamEgressNodeUseCaseh
#define __prvTgfOamEgressNodeUseCaseh

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
* internal tgfOamEgressNodeConfigSet function
* @endinternal
*
* @brief   OAM Egress Node use case configurations
*/
GT_VOID tgfOamEgressNodeConfigSet
(
    GT_VOID
);

/**
* @internal tgfOamEgressNodeTest function
* @endinternal
*
* @brief  OAM Egress Node use case test.
*/
GT_VOID tgfOamEgressNodeTest
(
    GT_VOID
);

/**
* @internal tgfOamEgressNodeConfigRestore function
* @endinternal
*
* @brief   OAM Egress Node use case configurations restore.
*/
GT_VOID tgfOamEgressNodeConfigRestore
(
    GT_VOID
);

/**
* internal tgfEgressMirroringMetadataVerificationConfigSet function
* @endinternal
*
* @brief   Egress Mirroring verification use case - Configuration
*/
GT_VOID tgfEgressMirroringMetadataVerificationConfigSet
(
    GT_VOID
);

/**
* internal tgfSrv6ThreeSegmentConfigSet function
* @endinternal
*
* @brief   Egress Mirroring verification use case - Traffic and verify
*/
GT_VOID tgfEgressMirroringMetadataVerificationTrafficTest
(
    GT_VOID
);

/**
* internal tgfSrv6ThreeSegmentConfigSet function
* @endinternal
*
* @brief   Egress Mirroring verification use case - Configuration restore
*/
GT_VOID tgfEgressMirroringMetadataVerificationConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfOamEgressNodeUseCaseh */



