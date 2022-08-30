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
* @file sMath.h
*
* @brief
* Mathematical operations on 64 bit values
*
* @version   4
********************************************************************************
*/

#ifndef __sMathh
#define __sMathh

#include <os/simTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvSimMathAdd64 function
* @endinternal
*
* @brief   Summarize two 64 bits values.
*
* @param[in] x                        - first value for sum.
* @param[in] y                        - second value for sum
*                                       Calculated sum.
*
* @note This function does not care 65 bit overflow.
*
*/
GT_U64  prvSimMathAdd64
(
    IN  GT_U64 x,
    IN  GT_U64 y
);

/**
* @internal prvSimMathSub64 function
* @endinternal
*
* @brief   Subtract two 64 bits values.
*
* @param[in] x                        - first value for difference.
* @param[in] y                        - second value for difference
*
* @retval Calculated difference x  - y.
*/
GT_U64  prvSimMathSub64
(
    IN  GT_U64 x,
    IN  GT_U64 y
);

/**
 * CNV_U32_TO_U64
 *
 * DESCRIPTION:
 *       Macro converts 32 bits value to 64 bits values.
 *
 * INPUTS:
 *       value32 - 32 bits value to convert from.
 *       value64 - 64 bits value to convert to.
 *
 *
 **/
#define CNV_U32_TO_U64(value32, value64) \
{ \
    value64.l[0] = value32; \
    value64.l[1] = 0; \
}

/* compare two unsigned GT_U32 values:
 *     RETURNS:
 *       0 - two values are identical
 *      < 0 (negative) - value a less than value b
 *      > 0 (positive) - value a greater than value b   */
#define COMPARE_TWO_INT_VALUES_MAC(a, b) \
    ((((a) >= (b)) ? (((a) == (b)) ? 0 : 1) : (-1)))

/**
 * CMP_TWO_U64_VALUES_MAC
 *
 * DESCRIPTION:
 *       Macro compares two 64 bits values.
 *
 * INPUTS:
 *       first64    - first 64 bits value.
 *       second64   - second 64 bits value.
 * RETURN:
 *       0 - two values are identical
 *      < 0 (negative) - first value less than second value
 *      > 0 (positive) - first value greater than second value
 **/
#define COMPARE_TWO_U64_VALUES_MAC(first64, second64) \
  ((((first64).l[0] == (second64).l[0]) && ((first64).l[1] == (second64).l[1])) ? 0 : \
   (((first64).l[1] == (second64).l[1])) ? \
        COMPARE_TWO_INT_VALUES_MAC((first64).l[0], (second64).l[0]) : \
        COMPARE_TWO_INT_VALUES_MAC((first64).l[1], (second64).l[1]))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sMathh */


