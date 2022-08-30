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
* @file prvCpssPxPortDynamicPAGCDAlgo.h
*
* @brief GCD (greatest common divider) algorithm
*
* @version   1
********************************************************************************
*/
#ifndef __PRV_CPSS_PX_DYN_PA_GCD_ALGO_H
#define __PRV_CPSS_PX_DYN_PA_GCD_ALGO_H

#include <cpss/common/cpssTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssPxDynPAGCDAlgo function
* @endinternal
*
* @brief   GCD (greatest common divider) algorithm
*
* @note   APPLICABLE DEVICES:      None.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] a                        - first number
* @param[in] b                        - second number
*                                       gcd of 2 numbers
*/
GT_U32 prvCpssPxDynPAGCDAlgo
(
    IN GT_U32 a, 
    IN GT_U32 b
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

