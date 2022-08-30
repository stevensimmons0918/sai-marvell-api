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
* @file sMath.c
*
* @brief
* Mathematical operations on 64 bit values
*
* @version   3
********************************************************************************
*/

#include <os/simEnvDepTypes.h>
#include <common/Utils/Math/sMath.h>
/************* Defines ***********************************************/

/************ Public Functions ************************************************/

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
)
{
    GT_U64  z;
    GT_U32  maxVal;

    maxVal = y.l[0];
    if (x.l[0] > y.l[0])
        maxVal = x.l[0];

    z.l[0] = x.l[0] + y.l[0];           /* low order word sum  */
    z.l[1] = x.l[1] + y.l[1];           /* high order word sum */
    z.l[1] += (z.l[0] < maxVal) ? 1:0;  /* low-word overflow */

    return z;
}

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
)
{
    GT_U64  z;

    z.l[0] = x.l[0] - y.l[0];            /* low order word difference  */
    z.l[1] = x.l[1] - y.l[1];            /* high order word difference */
    z.l[1] -= (x.l[0] < y.l[0]) ? 1 : 0; /* low-word borrow            */

    return z;
}

