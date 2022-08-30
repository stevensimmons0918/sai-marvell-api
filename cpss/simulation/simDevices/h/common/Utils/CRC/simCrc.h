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
* @file simCrc.h
*
* @brief Includes definitions for CRC calculation function.
*
* @version   5
********************************************************************************
*/
#ifndef __simCrch
#define __simCrch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/********* Include files ******************************************************/
#include <os/simTypes.h>

/**
* @internal simCalcCrc32 function
* @endinternal
*
* @brief   Calculate CRC 32 bit for input data.
*
* @param[in] crc                      - CRC start value.
* @param[in] buffer                   - pointer to the buffer.
* @param[in] byteNum                  - number of bytes in the buffer.
*                                       32 bit CRC.
*
* @note For calculation a new CRC the value of CRC should be 0xffffffff.
*
*/
GT_U32 simCalcCrc32
(
    IN GT_U32 crc,
    IN GT_U8  *buffer,
    IN GT_U32 byteNum
);

/**
* @internal simCalcCrc16 function
* @endinternal
*
* @brief   Calculate CRC 16 bit for input data.
*
* @param[in] crc                      - CRC start value.
* @param[in] buffer                   - pointer to the buffer.
* @param[in] byteNum                  - number of bytes in the buffer.
*                                       32 bit CRC.
*
* @note For calculation a new CRC the value of CRC should be 0xffff.
*
*/
GT_U16 simCalcCrc16
(
    IN GT_U16 crc,
    IN GT_U8  *buffer,
    IN GT_U32 byteNum
);

/**
* @internal simCalcCrc8 function
* @endinternal
*
* @brief   Calculate CRC 8 bit for input data.
*
* @param[in] crc                      - CRC start value.
* @param[in] buffer                   - pointer to the buffer.
* @param[in] byteNum                  - number of bytes in the buffer.
*                                       32 bit CRC.
*
* @note For calculation a new CRC the value of CRC should be 0xff.
*
*/
GT_U8 simCalcCrc8
(
    IN GT_U8  crc,
    IN GT_U8  *buffer,
    IN GT_U32 byteNum
);


/**
* @internal simCalcHashFor70BytesCrc6 function
* @endinternal
*
* @brief   Calculate CRC 6 bit for 70 bytes input data.
*
* @param[in] crc                      - CRC start value.
* @param[in] bufPtr                   - pointer to the  (of 70 bytes)
* @param[in] byteNum                  - number of bytes in the bufPtr -- MUST be 70 bytes
*                                       6 bit CRC.
*/
GT_U32 simCalcHashFor70BytesCrc6
(
    IN GT_U32  crc,
    IN GT_U8  *bufPtr,
    IN GT_U32 byteNum
);

/**
* @internal simCalcHashFor70BytesCrc16 function
* @endinternal
*
* @brief   Calculate CRC 16 bit for 70 bytes input data.
*
* @param[in] crc                      - CRC start value.
* @param[in] bufPtr                   - pointer to the  (of 70 bytes)
* @param[in] byteNum                  - number of bytes in the bufPtr -- MUST be 70 bytes
*                                       16 bit CRC.
*/
GT_U32 simCalcHashFor70BytesCrc16
(
    IN GT_U32  crc,
    IN GT_U8  *bufPtr,
    IN GT_U32 byteNum
);

/**
* @internal simCalcHashFor70BytesCrc32 function
* @endinternal
*
* @brief   Calculate CRC 32 bit for 70 bytes input data.
*         CRC-32 hash value calculation is performed using the CRC-32-IEEE 802.3 polynomial
* @param[in] crc                      - CRC start value.
* @param[in] bufPtr                   - pointer to the  (of 70 bytes)
* @param[in] byteNum                  - number of bytes in the bufPtr -- MUST be 70 bytes
*                                       32 bit CRC.
*/
GT_U32 simCalcHashFor70BytesCrc32
(
    IN GT_U32  crc,
    IN GT_U8  *bufPtr,
    IN GT_U32 byteNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __simCrch */


