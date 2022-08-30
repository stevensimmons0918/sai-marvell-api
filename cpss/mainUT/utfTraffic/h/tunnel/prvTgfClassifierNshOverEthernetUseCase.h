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
* @file prvTgfClassifierNshOverEthernetUseCase.h
*
* @brief Classifier NSH over Ethernet use case testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfClassifierNshOverEthernetUseCaseh
#define __prvTgfClassifierNshOverEthernetUseCaseh

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
* internal prvTgfClassifierNshOverEthernetConfigSet function
* @endinternal
*
* @brief   Classifier NSH over Ethernet use case configurations
*/
GT_VOID prvTgfClassifierNshOverEthernetConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfClassifierNshOverEthernetTest function
* @endinternal
*
* @brief   Classifier NSH over Ethernet use case test.
*/
GT_VOID prvTgfClassifierNshOverEthernetTest
(
    GT_VOID
);

/**
* @internal prvTgfClassifierNshOverEthernetVerification function
* @endinternal
*
* @brief   Classifier NSH over Ethernet use case verification.
*/
GT_VOID prvTgfClassifierNshOverEthernetVerification
(
    GT_VOID
);

/**
* @internal prvTgfClassifierNshOverEthernetConfigRestore function
* @endinternal
*
* @brief   Classifier NSH over Ethernet configurations restore.
*/
GT_VOID prvTgfClassifierNshOverEthernetConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfClassifierNshOverEthernetUseCaseh */




