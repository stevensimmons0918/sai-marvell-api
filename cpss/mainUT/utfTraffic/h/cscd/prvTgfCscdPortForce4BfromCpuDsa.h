/*******************************************************************************
*              (C), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvTgfCscdPortForce4BfromCpuDsa.h
*
* @brief Test for forced 4B from_cpu DSA tag in outgoing packet (FORWARD command).
* The packet is sent:
* - from a network port to a cascade port
* Tx port is checked: it should or shouldn't contain a 4B from_cpu DSA tag in the packet
* depending on the port is configured to transmit 4B DSA tag.
*
*
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfCscdPortForce4BfromCpuDsa
#define __prvTgfCscdPortForce4BfromCpuDsa

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfCscdPortForce4BfromCpuDsaTest function
* @endinternal
*
* @brief    Check outgoing packet with a 4B from_cpu DSA tag with command FORWARD
*
*/
GT_VOID prvTgfCscdPortForce4BfromCpuDsaTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCscdPortForce4BfromCpuDsa */

