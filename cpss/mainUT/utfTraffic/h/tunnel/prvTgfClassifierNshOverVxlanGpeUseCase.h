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
* @file prvTgfClassifierNshOverVxlanGpeUseCase.h
*
* @brief Classifier NSH over VXLAN-GPE use case testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfClassifierNshOverVxlanGpeUseCaseh
#define __prvTgfClassifierNshOverVxlanGpeUseCaseh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfTunnelGen.h>

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/**
* internal prvTgfClassifierNshOverVxlanGpeConfigSet function
* @endinternal
*
* @brief   Classifier NSH over VXLAN-GPE use case configurations
*/
GT_VOID prvTgfClassifierNshOverVxlanGpeConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfClassifierNshOverVxlanGpeTest function
* @endinternal
*
* @brief   Classifier NSH over VXLAN-GPE use case test.
*/
GT_VOID prvTgfClassifierNshOverVxlanGpeTest
(
    GT_VOID
);

/**
* @internal prvTgfClassifierNshOverVxlanGpeVerification function
* @endinternal
*
* @brief   Classifier NSH over VXLAN-GPE use case verification.
*/
GT_VOID prvTgfClassifierNshOverVxlanGpeVerification
(
    GT_VOID
);

/**
* @internal prvTgfClassifierNshOverVxlanGpeConfigRestore function
* @endinternal
*
* @brief   Classifier NSH over VXLAN-GPE configurations restore.
*/
GT_VOID prvTgfClassifierNshOverVxlanGpeConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfClassifierNshOverVxlanGpeUseCaseh */




