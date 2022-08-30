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
* @file prvCpssMath.h
*
* @brief Math operations for CPSS.
*
* @version   5
********************************************************************************
*/
#ifndef __prvCpssMathh
#define __prvCpssMathh

#include <cpss/extServices/os/gtOs/gtGenTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssMathAdd64 function
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
GT_U64  prvCpssMathAdd64
(
    IN  GT_U64 x,
    IN  GT_U64 y
);

/**
* @internal prvCpssMathSub64 function
* @endinternal
*
* @brief   Subtract two 64 bits values.
*
* @param[in] x                        - first value for difference.
* @param[in] y                        - second value for difference
*
* @retval Calculated difference x  - y.
*/
GT_U64  prvCpssMathSub64
(
    IN  GT_U64 x,
    IN  GT_U64 y
);

/**
* @internal prvCpssMathCompare64 function
* @endinternal
*
* @brief   Compare two 64 bits values.
*
* @param[in] x                        - first value.
* @param[in] y                        - second value.
*                                       x > y ==> 1
*                                       x == y ==> 0
*
* @retval x <  y ==>               -1
*/
int  prvCpssMathCompare64
(
    IN  GT_U64 x,
    IN  GT_U64 y
);

/**
* @internal prvCpssMathDiv64By16 function
* @endinternal
*
* @brief   Divides 64 bits value by 16 bits value.
*
* @param[in] x                        - dividend.
* @param[in] y                        - divisor
*                                       Calculated quotient.
*/
GT_U64  prvCpssMathDiv64By16
(
    IN  GT_U64 x,
    IN  GT_U16 y
);

/**
* @internal prvCpssMathMod64By16 function
* @endinternal
*
* @brief   Calculate reminder from division 64-bits value by 16-bits value.
*
* @param[in] x                        - dividend.
* @param[in] y                        - divisor
*                                       Calculated reminder.
*/
GT_U16  prvCpssMathMod64By16
(
    IN  GT_U64 x,
    IN  GT_U16 y
);

/**
* @internal prvCpssMathMul64 function
* @endinternal
*
* @brief   Calculate multiplication of 32-bits value by 32-bits value.
*
* @param[in] x                        - dividend.
* @param[in] y                        - divisor
*                                       Calculated xy.
*/
GT_U64 prvCpssMathMul64
(
    IN GT_U32 x,
    IN GT_U32 y
);

/**
* @internal prvCpssMathDiv64By32 function
* @endinternal
*
* @brief   Calculate quotient and reminder of division of 64-bits value by 32-bits value.
*
* @param[in] x                        - dividend.
* @param[in] y                        - divisor
*
* @param[out] modPtr                   - (pointer to) 32-bit reminder of division.
*                                      Can be NULL.
*
* @retval Calculated 64            -bit x/y.
*/
GT_U64 prvCpssMathDiv64By32
(
    IN  GT_U64  x,
    IN  GT_U32  y,
    OUT GT_U32  *modPtr
);

/**
* @internal prvCpssMathDiv64 function
* @endinternal
*
* @brief   Calculate quotient and reminder of division of 64-bits values
*
* @param[in] x                        - dividend.
* @param[in] y                        - divisor
*
* @param[out] resPtr                  - (pointer to) x/y.
* @param[out] modPtr                  - (pointer to) reminder of division.
*                                       Can be NULL.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on bad pointer to result
* @retval GT_BAD_PARAM             - on y = 0
*
*/
GT_STATUS prvCpssMathDiv64
(
    IN  GT_U64      x,
    IN  GT_U64      y,
    OUT GT_U64      *resPtr,
    OUT GT_U64      *modPtr
);

/**
* @internal prvCpssMathIsPowerOf2 function
* @endinternal
*
* @brief   This routine checks whether number is a power of 2.
*
* @param[in] number                   - the  to verify.
*
* @retval GT_TRUE                  - for true.
* @retval GT_FALSE                 - for false.
*/
GT_BOOL prvCpssMathIsPowerOf2
(
    IN GT_U32 number
);

/**
* @internal prvCpssMathLog2 function
* @endinternal
*
* @brief   This function calculates log(x) by base 2.
*
* @param[in] num                      - The number to perform the calculation on, This number must be a
*                                      power of 2.
*
* @param[out] resultPtr                - pointer to log(num) result.
*                                       GT_OK on success,
*                                       GT_FAIL otherwise (if the number is not a power of 2).
*/
GT_STATUS prvCpssMathLog2
(
    IN  GT_U32  num,
    OUT GT_U32  *resultPtr
);

/**
* @internal prvCpssMathPowerOf2 function
* @endinternal
*
* @brief   Calculate power of 2
*
* @param[in] exponent                 - the exponent, a value between 0 to 7
*
* @retval GT_BAD_PARAM             - if exponent is not between 0 to 7
* @retval GT_OK                    - otherwise
*/
GT_STATUS prvCpssMathPowerOf2
(
    IN  GT_U8   exponent,
    OUT GT_U8   *result
);

/**
* @internal prvCpssRoundDiv function
* @endinternal
*
* @brief   Divide and round a value with accuracy of 3 digits.
*
* @param[in] value                    - Value to divide.
* @param[in] divisor                  - the divisor.
*
* @param[out] resultPtr                - pointer to result after round
*                                       GT_OK
*/
GT_STATUS prvCpssRoundDiv
(
    IN  GT_U32  value,
    IN  GT_U32  divisor,
    OUT GT_U32 *resultPtr
);

/**
* @internal prvCpssMathShl64 function
* @endinternal
*
* @brief   Shift 64-bit value left by 1.
*
* @param[in] x    - value to shift left.
*
* @retval Shifted 64-bit value x << 1
*/
GT_U64 prvCpssMathShl64
(
    IN  GT_U64      x
);

/**
* @internal prvCpssMathXor64 function
* @endinternal
*
* @brief   Calculate XOR of 2  64-bits numbers.
*
* @param[in] x  - first value.
* @param[in] y  - second value.
*
* @retval Xored 64-bit value
*/
GT_U64 prvCpssMathXor64
(
    IN  GT_U64      x ,
    IN  GT_U64      y
);

/* long numbers operaions                                      */
/* long number is an array of GT_U32                           */
/* n-members array is a (n * 32) bits unsigned integer number  */
/* a[0] - bits 31:0, a[1] - bits 63:32, a[2] - bits 95:64 ...  */

/**
* @internal prvCpssLongNumberAdd32 function
* @endinternal
*
* @brief   Add 32-bit number to long number.
*
* @param[inout] arr   - array of 32-bit parts of long number.
* @param[in] arrSize  - amount of 32-bit parts of long number.
* @param[in] x        - 32-bit value for operation.
*
*/
GT_VOID prvCpssLongNumberAdd32
(
    INOUT  GT_U32      *arr,
    IN     GT_U32      arrSize,
    IN     GT_U32      x
);

/**
* @internal prvCpssLongNumberAddLong function
* @endinternal
*
* @brief   Add long number to long number.
*
* @param[inout] arr      - array of 32-bit parts of long number.
* @param[in] arrSize     - amount of 32-bit parts of long number.
* @param[in] incArr      - array of 32-bit parts of long number to be added.
* @param[in] incArrSize  - amount of 32-bit parts of long number to be added.
*                          must not be greater than arrSize, otherwize truncated
*
*/
GT_VOID prvCpssLongNumberAddLong
(
    INOUT  GT_U32      *arr,
    IN     GT_U32      arrSize,
    INOUT  GT_U32      *incArr,
    IN     GT_U32      incArrSize
);

/**
* @internal prvCpssLongNumberSubtract32 function
* @endinternal
*
* @brief   Subtract 32-bit number from long number.
*
* @param[inout] arr   - array of 32-bit parts of long number.
* @param[in] arrSize  - amount of 32-bit parts of long number.
* @param[in] x        - 32-bit value for operation.
*
*/
GT_VOID prvCpssLongNumberSubtract32
(
    INOUT  GT_U32      *arr,
    IN     GT_U32      arrSize,
    IN     GT_U32      x
);

/**
* @internal prvCpssLongNumberSubtractLong function
* @endinternal
*
* @brief   Subtract long number from long number.
*
* @param[inout] arr      - array of 32-bit parts of long number.
* @param[in] arrSize     - amount of 32-bit parts of long number.
* @param[in] decArr      - array of 32-bit parts of long number to be subtracted.
* @param[in] decArrSize  - amount of 32-bit parts of long number to be subtracted.
*                          must not be greater than arrSize, otherwize truncated
*
*/
GT_VOID prvCpssLongNumberSubtractLong
(
    INOUT  GT_U32      *arr,
    IN     GT_U32      arrSize,
    INOUT  GT_U32      *decArr,
    IN     GT_U32      decArrSize
);

/**
* @internal prvCpssLongNumberMultiply32 function
* @endinternal
*
* @brief   Multiply long number by 32-bit number.
*
* @param[inout] arr     - array of 32-bit parts of long number.
* @param[in] arrSize    - amount of 32-bit parts of long number.
* @param[in] x          - 32-bit value for operation.
*
*/
GT_VOID prvCpssLongNumberMultiply32
(
    INOUT  GT_U32      *arr,
    IN     GT_U32      arrSize,
    IN     GT_U32      x
);

/**
* @internal prvCpssLongNumberDevide32 function
* @endinternal
*
* @brief   Devide long number by 32-bit number.
*
* @param[inout] arr       - array of 32-bit parts of long number.
* @param[in] arrSize      - amount of 32-bit parts of long number.
* @param[in] x            - 32-bit value for operation.
* @param[out] reminderPtr - pointer to reminder.
*
*/
GT_VOID prvCpssLongNumberDevide32
(
    INOUT  GT_U32      *arr,
    IN     GT_U32      arrSize,
    IN     GT_U32      x,
    OUT    GT_U32      *reminderPtr
);

/**
* @internal prvCpssTimeSecondsAndNanoDiff function
* @endinternal
*
* @brief   Exact time difference for pairs seconds and nano.
*
* @param[in]  startSec      - start time seconds.
* @param[in]  startNano     - start time nano seconds.
* @param[in]  stopSec       - stop time seconds.
* @param[in]  stopNano      - stop time nano seconds.
* @param[out] resultSecPtr  - pointer to result seconds.
* @param[out] resultNanoPtr - pointer to result nano seconds.
*
*/
void prvCpssTimeSecondsAndNanoDiff
(
    IN     GT_U32  startSec,
    IN     GT_U32  startNano,
    IN     GT_U32  stopSec,
    IN     GT_U32  stopNano,
    OUT    GT_U32  *resultSecPtr,
    OUT    GT_U32  *resultNanoPtr
);

/**
* @internal prvCpssTimeSecondsAndNanoCompare function
* @endinternal
*
* @brief   Exact time compare for pairs seconds and nano.
*
* @param[in]  time1Sec      - time1 seconds.
* @param[in]  time1Nano     - time1 nano seconds.
* @param[in]  time2Sec      - time2 seconds.
* @param[in]  time2Nano     - time2 nano seconds.
*
* @retval 0                 - on equal (time1 == time2)
* @retval -1                - on less (time1 < time2)
* @retval 1                 - on greater (time1 > time2)
*/
GT_32 prvCpssTimeSecondsAndNanoCompare
(
    IN     GT_U32  time1Sec,
    IN     GT_U32  time1Nano,
    IN     GT_U32  time2Sec,
    IN     GT_U32  time2Nano
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssMathh */


