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
* @file prvTgfSffNshVxlanGpeToEthernetUseCase.h
*
* @brief SFF NSH VXLAN-GPE to Ethernet use case testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfSffNshVxlanGpeToEthernetUseCaseh
#define __prvTgfSffNshVxlanGpeToEthernetUseCaseh

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
* internal prvTgfSffNshVxlanGpeToEthernetConfigSet function
* @endinternal
*
* @brief   SFF NSH VXLAN-GPE to Ethernet use case configurations
*/
GT_VOID prvTgfSffNshVxlanGpeToEthernetConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfSffNshVxlanGpeToEthernetTest function
* @endinternal
*
* @brief   SFF NSH VXLAN-GPE to Ethernet use case test.
*/
GT_VOID prvTgfSffNshVxlanGpeToEthernetTest
(
    GT_VOID
);

/**
* @internal prvTgfSffNshVxlanGpeToEthernetVerification function
* @endinternal
*
* @brief   SFF NSH VXLAN-GPE to Ethernet use case verification.
*/
GT_VOID prvTgfSffNshVxlanGpeToEthernetVerification
(
    GT_VOID
);

/**
* @internal prvTgfSffNshVxlanGpeToEthernetConfigRestore function
* @endinternal
*
* @brief   SFF NSH VXLAN-GPE to Ethernet configurations restore.
*/
GT_VOID prvTgfSffNshVxlanGpeToEthernetConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfSffNshVxlanGpeToEthernetUseCaseh */




