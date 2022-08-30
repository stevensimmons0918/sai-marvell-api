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
* @file prvTgfBrgNestedVlanPerTargetPort.h
*
* @brief this file contains test functions of nested VLAN per ePort
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfBrgNestedVlanPerTargetPorth
#define __prvTgfBrgNestedVlanPerTargetPorth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <trafficEngine/tgfTrafficEngine.h>

/**
* @internal prvTgfBrgNestedVlanPerTargetPortTrafficGenerate function
* @endinternal
*
*/
GT_VOID prvTgfBrgNestedVlanPerTargetPortTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgNestedVlanPerTargetPortConfigurationSet function
* @endinternal
*
* @brief
*/
GT_VOID prvTgfBrgNestedVlanPerTargetPortConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgNestedVlanPerTargetPortConfigurationRestore function
* @endinternal
*
* @brief
*/
GT_VOID prvTgfBrgNestedVlanPerTargetPortConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgNestedVlanPerTargetPort.h */



