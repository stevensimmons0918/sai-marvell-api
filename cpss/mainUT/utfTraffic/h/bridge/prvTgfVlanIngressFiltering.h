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
* @file prvTgfVlanIngressFiltering.h
*
* @brief VLAN Ingress Filtering Tagged Packets
*
* @version   1.4
********************************************************************************
*/
#ifndef __prvTgfBrgVlanIngressFilteringh
#define __prvTgfBrgVlanIngressFilteringh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgVlanIngrFltConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgVlanIngrFltConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanIngrFltTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgVlanIngrFltTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanIngrFltConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanIngrFltConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanIngrFltInvalidVidGet function
* @endinternal
*
* @brief   Get invalid VLAN that is used for the test
*/
GT_U16 prvTgfBrgVlanIngrFltInvalidVidGet
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgVlanIngressFilteringh */



