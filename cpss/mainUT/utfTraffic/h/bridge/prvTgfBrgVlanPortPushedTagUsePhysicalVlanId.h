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
* @file prvTgfBrgVlanPortPushedTagUsePhysicalVlanId.h
*
* @brief Test to check whether the push tag VLAN Id is mapped to physical port or
*        ePort VLAN Id based on use_physical_port_vlan_id field status.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgVlanPortPushedTagUsePhysicalVlanId
#define __prvTgfBrgVlanPortPushedTagUsePhysicalVlanId

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfBrgVlanPortPushedTagUsePhysicalVlanId function
* @endinternal
*
* @brief    Check outgoing packet has pushed VLAN tag with specific VLAN Id
*
*/
GT_VOID prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgVlanPortPushedTagUsePhysicalVlanId */

