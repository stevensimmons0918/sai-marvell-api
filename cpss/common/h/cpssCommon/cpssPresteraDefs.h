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
* @file cpssPresteraDefs.h
*
* @brief Includes general system definitions.
*
* @version   31
********************************************************************************
*/
#ifndef __cpssPresteraDefsh
#define __cpssPresteraDefsh

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/extServices/os/gtOs/cpssOsSem.h>
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpssCommon/private/prvCpssMisc.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* macro to set COVERITY bookmarks */
#define CPSS_COVERITY_NON_ISSUE_BOOKMARK

#define MARVELL_VENDOR_ID 0x11ab

#define GT_SDMA_WA

/* Max number of PPs in system  */
#define PRV_CPSS_MAX_PP_DEVICES_CNS             128

/* Max number of physical ports in PP device */
/* see also CPSS_MAX_PORTS_NUM_CNS  */
#define PRV_CPSS_MAX_PP_PORTS_NUM_CNS           1024

/* Max number of Xbars in system  */
#define PRV_CPSS_MAX_XBAR_DEVICES_CNS           128

/* Max fabric ports in xbar device */
#define PRV_CPSS_MAX_XBAR_FPORTS_NUM_CNS        13

/* max number of ports with MAC (gig/xg/hgl..)*/
/* in Falcon 12.8T there are 256+16 MACs */
#define PRV_CPSS_MAX_MAC_PORTS_NUM_CNS          (256+16)

/* max lanes per port */
#define PRV_CPSS_MAX_PORT_LANES_CNS             8


/* Define the different memory sizes    */
#define _1KB            (0x400)
#define _2KB            (0x800)
#define _4KB            (0x1000)
#define _6KB            (0x1800)
#define _8KB            (0x2000)
#define _10KB           (0x2800)
#define _16KB           (0x4000)
#define _24KB           (0x6000)
#define _32KB           (0x8000)
#define _64KB           (0x10000)
#define _128KB          (0x20000)
#define _256KB          (0x40000)
#define _512KB          (0x80000)
#define _1MB            (0x100000)
#define _2MB            (0x200000)
#define _3MB            (0x300000)
#define _4MB            (0x400000)
#define _6MB            (0x600000)
#define _8MB            (0x800000)
#define _16MB           (0x1000000)
#define _32MB           (0x2000000)
#define _64MB           (0x4000000)
#define _128MB          (0x8000000)
#define _256MB          (0x10000000)
#define _512MB          (0x20000000)
#define _1GB            (0x40000000)
/* Define the different sizes    */
#ifndef _1K
#define _1K            (0x400)
#endif
#define _2K            (0x800)
#define _3K            (0xC00)
#ifndef _4K
#define _4K            (0x1000)
#endif
#define _5K            (0x1400)
#define _6K            (0x1800)
#define _8K            (0x2000)
#define _9K            (0x2400)
#define _10K           (0x2800)
#define _12K           (0x3000)
#define _13K           (13*_1K)
#define _16K           (0x4000)
#define _24K           (0x6000)
#define _28K           (0x7000)
#define _32K           (0x8000)
#define _36K           (0x9000)
#define _48K           (0xc000)

#ifndef _64K
#define _64K           (0x10000)
#endif

#define _128K          (0x20000)
#ifndef _256K
#define _256K          (0x40000)
#endif

#ifndef _512K
#define _512K          (0x80000)
#endif

#ifndef _1M
#define _1M            (0x100000)
#endif

#ifndef _2M
#define _2M            (0x200000)
#endif

#define _3M            (0x300000)

#ifndef _4M
#define _4M            (0x400000)
#endif

#define _6M            (0x600000)

#ifndef _8M
#define _8M            (0x800000)
#endif

#define _12M           (0xC00000)

#ifndef _16M
#define _16M           (0x1000000)
#endif

#define _24M           (0x1800000)
#define _32M           (0x2000000)

#ifndef _64M
#define _64M           (0x4000000)
#endif

#define _128M          (0x8000000)
#define _256M          (0x10000000)
#define _512M          (0x20000000)
#define _1G            (0x40000000)

/* Define single bit masks.             */
#define BIT_0           (0x1)
#define BIT_1           (0x2)
#define BIT_2           (0x4)
#define BIT_3           (0x8)
#define BIT_4           (0x10)
#define BIT_5           (0x20)
#define BIT_6           (0x40)
#define BIT_7           (0x80)
#define BIT_8           (0x100)
#define BIT_9           (0x200)
#define BIT_10          (0x400)
#define BIT_11          (0x800)
#define BIT_12          (0x1000)
#define BIT_13          (0x2000)
#define BIT_14          (0x4000)
#define BIT_15          (0x8000)
#define BIT_16          (0x10000)
#define BIT_17          (0x20000)
#define BIT_18          (0x40000)
#define BIT_19          (0x80000)
#define BIT_20          (0x100000)
#define BIT_21          (0x200000)
#define BIT_22          (0x400000)
#define BIT_23          (0x800000)
#define BIT_24          (0x1000000)
#define BIT_25          (0x2000000)
#define BIT_26          (0x4000000)
#define BIT_27          (0x8000000)
#define BIT_28          (0x10000000)
#define BIT_29          (0x20000000)
#define BIT_30          (0x40000000)
#define BIT_31          (0x80000000)

/* Swap tool */

/* 16bit byte swap. For example 0x1122 -> 0x2211                            */
#if defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 408)
#define CPSS_BYTE_SWAP_16BIT(X) ((GT_U16)__builtin_bswap16(X))
#else /* default */
#define CPSS_BYTE_SWAP_16BIT(X) ((((X) & 0xff) << 8) | (((X) & 0xff00) >> 8))
#endif /* GCC VERSION < 4.8 */

#if defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 403)
/* Use GCC extension for fast byte swap */
#define CPSS_BYTE_SWAP_32BIT(X) ((GT_U32)__builtin_bswap32(X))
#define CPSS_BYTE_SWAP_64BIT(X) ((unsigned long long)__builtin_bswap64(X))
#else /* default */
/* 32bit byte swap. For example 0x11223344 -> 0x44332211                    */
#define CPSS_BYTE_SWAP_32BIT(X) ((((X) & 0xff) << 24) |                       \
                               (((X) & 0xff00) << 8) |                      \
                               (((X) & 0xff0000) >> 8) |                    \
                               (((X) & 0xff000000) >> 24))

/* 64bit byte swap. For example 0x11223344.55667788 -> 0x88776655.44332211  */
#define CPSS_BYTE_SWAP_64BIT(X) ((l64) ((((X) & 0xffULL) << 56) |             \
                                      (((X) & 0xff00ULL) << 40) |           \
                                      (((X) & 0xff0000ULL) << 24) |         \
                                      (((X) & 0xff000000ULL) << 8) |        \
                                      (((X) & 0xff00000000ULL) >> 8) |      \
                                      (((X) & 0xff0000000000ULL) >> 24) |   \
                                      (((X) & 0xff000000000000ULL) >> 40) | \
                                      (((X) & 0xff00000000000000ULL) >> 56)))
#endif /* GCC VERSION < 4.3 */

/* Endianess macros.                                                        */
#ifdef CPU_BE
    #define CPSS_16BIT_LE(X)  CPSS_BYTE_SWAP_16BIT (X)
    #define CPSS_32BIT_LE(X)  CPSS_BYTE_SWAP_32BIT (X)
    #define CPSS_64BIT_LE(X)  CPSS_BYTE_SWAP_64BIT (X)
    #define CPSS_16BIT_BE(X)  (X)
    #define CPSS_32BIT_BE(X)  (X)
    #define CPSS_64BIT_BE(X)  (X)
#else
    #define CPSS_16BIT_LE(X)  (X)
    #define CPSS_32BIT_LE(X)  (X)
    #define CPSS_64BIT_LE(X)  (X)
    #define CPSS_16BIT_BE(X)  CPSS_BYTE_SWAP_16BIT (X)
    #define CPSS_32BIT_BE(X)  CPSS_BYTE_SWAP_32BIT (X)
    #define CPSS_64BIT_BE(X)  CPSS_BYTE_SWAP_64BIT (X)
#endif

/* used by functionsSupportedBmp */
/* PX family device */
#define PRV_CPSS_PX_FUNCTIONS_SUPPORT_CNS           BIT_0
/*bit 1 is unused*/
#define PRV_CPSS_DXCH_FUNCTIONS_SUPPORT_CNS         BIT_2
#define PRV_CPSS_DXCH2_FUNCTIONS_SUPPORT_CNS        BIT_3
#define PRV_CPSS_DXSAL_FUNCTIONS_SUPPORT_CNS        BIT_4
/*bit 5 is unused*/
#define PRV_CPSS_DXCH3_FUNCTIONS_SUPPORT_CNS        BIT_6
#define PRV_CPSS_DXCH_XCAT_FUNCTIONS_SUPPORT_CNS    BIT_7
#define PRV_CPSS_DXCH_LION_FUNCTIONS_SUPPORT_CNS    BIT_8
#define PRV_CPSS_DXCH_XCAT2_FUNCTIONS_SUPPORT_CNS   BIT_9
#define PRV_CPSS_DXCH_LION2_FUNCTIONS_SUPPORT_CNS   BIT_10
/*bits 11,12 is unused*/
#define PRV_CPSS_DXCH_BOBCAT2_FUNCTIONS_SUPPORT_CNS BIT_13
#define PRV_CPSS_DXCH_XCAT3_FUNCTIONS_SUPPORT_CNS   BIT_14
#define PRV_CPSS_DXCH_BOBCAT3_FUNCTIONS_SUPPORT_CNS BIT_15
#define PRV_CPSS_DXCH_CAELUM_FUNCTIONS_SUPPORT_CNS  BIT_16
#define PRV_CPSS_DXCH_ALDRIN_FUNCTIONS_SUPPORT_CNS  BIT_17
#define PRV_CPSS_DXCH_AC3X_FUNCTIONS_SUPPORT_CNS    BIT_18
#define PRV_CPSS_DXCH_ALDRIN2_FUNCTIONS_SUPPORT_CNS BIT_19
#define PRV_CPSS_DXCH_FALCON_FUNCTIONS_SUPPORT_CNS  BIT_20
#define PRV_CPSS_DXCH_AC5P_FUNCTIONS_SUPPORT_CNS    BIT_21
#define PRV_CPSS_DXCH_AC5X_FUNCTIONS_SUPPORT_CNS    BIT_22
#define PRV_CPSS_DXCH_HARRIER_FUNCTIONS_SUPPORT_CNS BIT_23
#define PRV_CPSS_DXCH_IRONMAN_FUNCTIONS_SUPPORT_CNS BIT_24

/* check NULL pointer */
#define CPSS_NULL_PTR_CHECK_MAC(ptr)    if(ptr == NULL) { \
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_ONE_PARAM_FORMAT_MAC(ptr)); \
}

/* Return the mask including "numOfBits" bits. for 0..31 bits   */
#define BIT_MASK_0_31_MAC(numOfBits) (~(0xFFFFFFFF << (numOfBits)))

/* the macro of BIT_MASK_0_31_MAC() in VC will act in wrong way for 32 bits , and will
   result 0 instead of 0xffffffff

   so macro BIT_MASK_MAC - is improvement of BIT_MASK_0_31_MAC to support 32 bits
*/
#define BIT_MASK_MAC(numOfBits)    ((numOfBits) == 32 ? 0xFFFFFFFF : BIT_MASK_0_31_MAC(numOfBits))

/* Calculate the field mask for a given offset & length */
/* e.g.: BIT_MASK_MAC(8,2) = 0xFFFFFCFF                     */
#define FIELD_MASK_NOT_MAC(offset,len)                      \
        (~(BIT_MASK_MAC((len)) << (offset)))

/* Calculate the field mask for a given offset & length */
/* e.g.: BIT_MASK_MAC(8,2) = 0x00000300                     */
#define FIELD_MASK_MAC(offset,len)                      \
        ( (BIT_MASK_MAC((len)) << (offset)) )

/* Returns the info located at the specified offset & length in data.   */
#define U32_GET_FIELD_MAC(data,offset,length)           \
        (((data) >> (offset)) & BIT_MASK_MAC(length))

/* Sets the field located at the specified offset & length in data.     */
#define U32_SET_FIELD_MAC(data,offset,length,val)           \
   (data) = (((data) & FIELD_MASK_NOT_MAC((offset),(length))) | ((val) <<(offset)))

/* for setting data of GT_U16.
   Sets the field located at the specified offset & length in data.     */
#define U16_SET_FIELD_MAC(data,offset,length,val)           \
   (data) = (GT_U16)(((data) & FIELD_MASK_NOT_MAC((offset),(length))) | ((val) <<(offset)))

/* Sets the field located at the specified offset & length in data.
    the field may be with value with more bits then length */
#define U32_SET_FIELD_MASKED_MAC(data,offset,length,val)           \
   (U32_SET_FIELD_MAC((data),(offset),(length),((val) & BIT_MASK_MAC(length))))


/* Sets the field in entry located at the specified offset & length in data.
    startEntryPtr - pointer to start entry data
    offset - number of bits offset from start of entry (value 0..(entryLength-1))
    length - field length (0..31)
    val    - field value (GT_U32)

NOTEs:
   1. the field may be with value with more bits then length
   2. Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer
        GT_BAD_PARAM    - on bad parameter : length > 32 or length == 0

*/
#define U32_SET_FIELD_IN_ENTRY_MAC(startEntryPtr,offset,length,val)             \
{                                                                               \
    GT_STATUS _rc;                                                              \
    _rc = prvCpssFieldValueSet(startEntryPtr,offset,length,val);                \
    if (_rc != GT_OK)                                                           \
    {                                                                           \
        return _rc;                                                             \
    }                                                                           \
}

/* Gets the field in entry located at the specified offset & length in data.
   startEntryPtr - pointer to start entry data
   offset - number of bits offset from start of entry (value 0..(entryLength-1))
   length - field length (0..31)
   startEntrySize - size of the start Entry in bytes to check out of range.
NOTEs:
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer
        GT_BAD_PARAM    - on bad parameter : length > 32 or length == 0

*/
#define U32_GET_FIELD_IN_ENTRY_OOR_CHECK_MAC(startEntryPtr,offset,length, startEntrySize, val) \
{                                                                               \
    GT_STATUS _rc;                                                              \
    _rc = prvCpssFieldValueOorCheckGet(startEntryPtr,offset,length, startEntrySize, &val); \
    if (_rc != GT_OK)                                                           \
    {                                                                           \
        return _rc;                                                             \
    }                                                                           \
}

/* Gets the field in entry located at the specified offset & length in data.
   startEntryPtr - pointer to start entry data
   offset - number of bits offset from start of entry (value 0..(entryLength-1))
   length - field length (0..31)
NOTEs:
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer
        GT_BAD_PARAM    - on bad parameter : length > 32 or length == 0

*/
#define U32_GET_FIELD_IN_ENTRY_MAC(startEntryPtr,offset,length,val)             \
{                                                                               \
    GT_STATUS _rc;                                                              \
    _rc = prvCpssFieldValueGet(startEntryPtr,offset,length,&val);               \
    if (_rc != GT_OK)                                                           \
    {                                                                           \
        return _rc;                                                             \
    }                                                                           \
}

/* Gets the field value from HW format according to 'Field ID'.
   startEntryPtr - pointer to start entry data
   fieldsInfoArr[] - array of fields info.
   fieldId - the 'Field ID'  - index in fieldsInfoArr[].
NOTEs:
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer
        GT_BAD_PARAM    - on bad parameter : length > 32 or length == 0

*/
#define U32_GET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,val)        \
{                                                                               \
    GT_STATUS _rc;                                                              \
    _rc = prvCpssFieldFromEntry_GT_U32_Get(startEntryPtr,fieldsInfoArr,fieldId,&val);   \
    if (_rc != GT_OK)                                                           \
    {                                                                           \
        return _rc;                                                             \
    }                                                                           \
}

/* Gets the field max possible value according to 'Field ID'.
   fieldsInfoArr[] - array of fields info.
   fieldId - the 'Field ID'  - index in fieldsInfoArr[].
NOTEs:
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer
        GT_BAD_PARAM    - on bad parameter : length > 32 or length == 0

*/
#define U32_GET_FIELD_MAX_VAL_BY_ID_MAC(fieldsInfoArr,fieldId,val)        \
{                                                                               \
    GT_STATUS _rc;                                                              \
    _rc = prvCpssFieldMaxVal_GT_U32_Get(fieldsInfoArr,fieldId,&val);   \
    if (_rc != GT_OK)                                                           \
    {                                                                           \
        return _rc;                                                             \
    }                                                                           \
}

/* Sets the field value to HW format according to 'Field ID'.
   startEntryPtr - pointer to start entry data
   fieldsInfoArr[] - array of fields info.
   fieldId - the 'Field ID'  - index in fieldsInfoArr[].
NOTEs:
   1. the field MUST NOT be with value with more bits then length
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer
        GT_BAD_PARAM    - on bad parameter : info.length > 32 or info.length == 0
                            or  val > maxValue

*/
#define U32_SET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,val)        \
{                                                                               \
    GT_STATUS _rc;                                                              \
    _rc = prvCpssFieldToEntry_GT_U32_Set(startEntryPtr,fieldsInfoArr,fieldId,val);   \
    if (_rc != GT_OK)                                                           \
    {                                                                           \
        return _rc;                                                             \
    }                                                                           \
}

/* Set field's bits in the HW data entry(GT_U32) according to fieldsInfoPtr, fieldId.
   If successful raise appropriate bits in the mask(GT_U32) too.
   NOTES:
   1. the field MUST NOT be with value with more bits then length
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer
        GT_BAD_PARAM    - on bad parameter : info.length > 32 or info.length == 0
                            or  val > maxValue
 */
#define U32_SET_FIELD_AND_MASK_BY_ID_MAC(hwDataArr, hwMaskArr, fieldsInfoPtr, fieldId, val) \
    do {                                                                                    \
        GT_STATUS rc;                                                                       \
        rc = prvCpssFieldToEntryAndMask_GT_U32_Set(hwDataArr, hwMaskArr,                    \
                                                   fieldsInfoPtr, fieldId, val);            \
        if (rc != GT_OK)                                                                    \
        {                                                                                   \
            return rc;                                                                      \
        };                                                                                  \
    } while (0);


/* Set 'any number of bits' value to a field in a table entry.
   the bits are in little Endian order in the array of words.

   startEntryPtr - pointer to start entry data
   fieldsInfoArr[] - array of fields info.
   fieldId - the 'Field ID'  - index in fieldsInfoArr[].
   valArr[] - the array of values to set
NOTEs:
   1. the field MUST NOT be with value with more bits then length
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer
        GT_BAD_PARAM    - on bad parameter : info.length == 0 or  valueArr[...] > maxValue

*/
#define U_ANY_SET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,valArr)   \
{                                                                               \
    GT_STATUS _rc;                                                              \
    _rc = prvCpssFieldToEntry_GT_Any_Set(startEntryPtr,fieldsInfoArr,fieldId,valArr);   \
    if (_rc != GT_OK)                                                           \
    {                                                                           \
        return _rc;                                                             \
    }                                                                           \
}

/* Set 6 bytes of MacAddr to a 'mac address' field (48 bits) in a table entry.

   startEntryPtr - pointer to start entry data
   fieldsInfoArr[] - array of fields info.
   fieldId - the 'Field ID'  - index in fieldsInfoArr[].
   _macAddrBytes[] - the array of 6 bytes (mac Addr)
NOTEs:
   1. the field MUST NOT be with value with more bits then length
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer
        GT_BAD_PARAM    - on bad parameter : info.length == 0 or  valueArr[...] > maxValue

*/
#define MAC_ADDR_SET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_macAddrBytes)   \
{                                                                             \
    GT_U32    _wordsArr[2];                                                   \
                                                                              \
    /* convert the 6 bytes of mac address to 2 words */                       \
    _wordsArr[0] =                                                            \
        (_macAddrBytes)[2] << 24 |                                            \
        (_macAddrBytes)[3] << 16 |                                            \
        (_macAddrBytes)[4] <<  8 |                                            \
        (_macAddrBytes)[5] <<  0;                                             \
    _wordsArr[1] =                                                            \
        (_macAddrBytes)[0] << 8 |                                             \
        (_macAddrBytes)[1] << 0 ;                                             \
                                                                              \
    U_ANY_SET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_wordsArr); \
}

/* Set 16 bytes of ipv6 address to a 'ipv6 address' field (128 bits) in a table entry.

   startEntryPtr - pointer to start entry data
   fieldsInfoArr[] - array of fields info.
   fieldId - the 'Field ID'  - index in fieldsInfoArr[].
   _ipv6AddrBytes[] - the array of 16 bytes (ipv6 Addr)
NOTEs:
   1. the field MUST NOT be with value with more bits then length
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer
        GT_BAD_PARAM    - on bad parameter : info.length == 0 or  valueArr[...] > maxValue

*/
#define IPV6_ADDR_SET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_ipv6AddrBytes)    \
{                                                                                            \
    GT_U32    _wordsArr[4];                                                                  \
                                                                                             \
    /* convert the 16 bytes of mac address to 4 words */                                     \
     _wordsArr[0] =                                                                          \
        (_ipv6AddrBytes)[12] << 24 |                                                         \
        (_ipv6AddrBytes)[13] << 16 |                                                         \
        (_ipv6AddrBytes)[14] <<  8 |                                                         \
        (_ipv6AddrBytes)[15] <<  0 ;                                                         \
    _wordsArr[1] =                                                                           \
        (_ipv6AddrBytes)[8]  << 24 |                                                         \
        (_ipv6AddrBytes)[9]  << 16 |                                                         \
        (_ipv6AddrBytes)[10] <<  8 |                                                         \
        (_ipv6AddrBytes)[11] <<  0 ;                                                         \
    _wordsArr[2] =                                                                           \
        (_ipv6AddrBytes)[4] << 24 |                                                          \
        (_ipv6AddrBytes)[5] << 16 |                                                          \
        (_ipv6AddrBytes)[6] <<  8 |                                                          \
        (_ipv6AddrBytes)[7] <<  0 ;                                                          \
    _wordsArr[3] =                                                                           \
        (_ipv6AddrBytes)[0] << 24 |                                                          \
        (_ipv6AddrBytes)[1] << 16 |                                                          \
        (_ipv6AddrBytes)[2] <<  8 |                                                          \
        (_ipv6AddrBytes)[3] <<  0;                                                           \
                                                                                             \
    U_ANY_SET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_wordsArr);                \
}
/* Set 9 bytes of Reduce Action to a 'reduce action' field (68 bits) in a table entry.

   startEntryPtr - pointer to start entry data
   fieldsInfoArr[] - array of fields info.
   fieldId - the 'Field ID'  - index in fieldsInfoArr[].
   _reduceActionBytes[] - the array of 9 bytes (reduce action)
   _expandedActionIndex - the expanded action index to set into reduced action
NOTEs:
   1. the field MUST NOT be with value with more bits then length
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer
        GT_BAD_PARAM    - on bad parameter : info.length == 0 or  valueArr[...] > maxValue

*/
#define REDUCED_ACTION_SET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_reduceActionBytes,_expandedActionIndex)   \
{                                                                                                    \
    GT_U32    _wordsArr[3];                                                                          \
                                                                                                     \
    /* convert the 9 bytes of reduce action to 3 words */                                            \
    _wordsArr[0] = _reduceActionBytes[0]<<0     |                                                    \
                   _reduceActionBytes[1]<<8     |                                                    \
                   _reduceActionBytes[2]<<16    |                                                    \
                   _reduceActionBytes[3]<<24;                                                        \
    _wordsArr[1] = _reduceActionBytes[4]<<0     |                                                    \
                   _reduceActionBytes[5]<<8     |                                                    \
                   _reduceActionBytes[6]<<16    |                                                    \
                   _reduceActionBytes[7]<<24;                                                        \
    _wordsArr[2] = (_expandedActionIndex&0xF);                                                       \
                                                                                                     \
    U_ANY_SET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_wordsArr);                        \
}

/* Get 9 bytes of Reduce Action to a 'reduce action' field (68 bits) in a table entry.

   startEntryPtr - pointer to start entry data
   fieldsInfoArr[] - array of fields info.
   fieldId - the 'Field ID'  - index in fieldsInfoArr[].
OUT:
   _reduceActionBytes[] - the array of 8 bytes (reduce action)
   _expandedActionIndex - the expanded action index to set into reduced action
NOTEs:
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer

*/
#define REDUCED_ACTION_GET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_reduceActionBytes,_expandedActionIndex)   \
{                                                                                                   \
    GT_U32    _wordsArr[3];                                                                         \
                                                                                                    \
    U_ANY_GET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_wordsArr);                       \
                                                                                                    \
    /* convert the 3 words to 8 bytes of reduce action plus expandedActionIndex */                  \
    (_reduceActionBytes)[0] = (GT_U8)(_wordsArr[0] >>  0);                                          \
    (_reduceActionBytes)[1] = (GT_U8)(_wordsArr[0] >>  8);                                          \
    (_reduceActionBytes)[2] = (GT_U8)(_wordsArr[0] >>  16);                                         \
    (_reduceActionBytes)[3] = (GT_U8)(_wordsArr[0] >>  24);                                         \
    (_reduceActionBytes)[4] = (GT_U8)(_wordsArr[1] >>  0);                                          \
    (_reduceActionBytes)[5] = (GT_U8)(_wordsArr[1] >>  8);                                          \
    (_reduceActionBytes)[6] = (GT_U8)(_wordsArr[1] >>  16);                                         \
    (_reduceActionBytes)[7] = (GT_U8)(_wordsArr[1] >>  24);                                         \
    (_expandedActionIndex) = (GT_U8)((_wordsArr[2]&0xF) >>  0);                                     \
}

/* Set 5 bytes of Key to a 'key' field (40 bits) in a table entry.

   startEntryPtr - pointer to start entry data
   fieldsInfoArr[] - array of fields info.
   fieldId - the 'Field ID'  - index in fieldsInfoArr[].
   _keyPatternBytes[] - the array of 5 bytes (key bits[0-39])
NOTEs:
   1. the field MUST NOT be with value with more bits then length
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer
        GT_BAD_PARAM    - on bad parameter : info.length == 0 or  valueArr[...] > maxValue

*/
#define KEY_SET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_keyPatternBytes)               \
{                                                                                                   \
    GT_U32    _wordsArr[2];                                                                         \
                                                                                                    \
    /* convert the 5 bytes of key to 2 words */                                                     \
    _wordsArr[0] = _keyPatternBytes[0]<<0   |                                                       \
                   _keyPatternBytes[1]<<8   |                                                       \
                   _keyPatternBytes[2]<<16  |                                                       \
                   _keyPatternBytes[3]<<24;                                                         \
    _wordsArr[1] = _keyPatternBytes[4];                                                             \
                                                                                                    \
    U_ANY_SET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_wordsArr);                       \
}

/* Get 5 bytes of Key to a 'key' field (40 bits) in a table entry.

   startEntryPtr - pointer to start entry data
   fieldsInfoArr[] - array of fields info.
   fieldId - the 'Field ID'  - index in fieldsInfoArr[].
OUT:
   _keyPatternBytes[] - the array of 5 bytes (key bits[0-39])
NOTEs:
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer

*/
#define KEY_GET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_keyPatternBytes)            \
{                                                                                                \
    GT_U32    _wordsArr[2];                                                                      \
                                                                                                 \
    U_ANY_GET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_wordsArr);                    \
                                                                                                 \
    /* convert the 3 words to 8 bytes of reduce action plus expandedActionIndex */               \
    (_keyPatternBytes)[0] = (GT_U8)(_wordsArr[0] >>  0);                                         \
    (_keyPatternBytes)[1] = (GT_U8)(_wordsArr[0] >>  8);                                         \
    (_keyPatternBytes)[2] = (GT_U8)(_wordsArr[0] >>  16);                                        \
    (_keyPatternBytes)[3] = (GT_U8)(_wordsArr[0] >>  24);                                        \
    (_keyPatternBytes)[4] = (GT_U8)(_wordsArr[1] >>  0);                                         \
}

/* Set 14 bytes of Key to a 'key only' field (112 bits) in a table entry.

   startEntryPtr - pointer to start entry data
   fieldsInfoArr[] - array of fields info.
   fieldId - the 'Field ID'  - index in fieldsInfoArr[].
   _keyPatternBytes[] - the array of 14 bytes (key 112 bits)
NOTEs:
   1. the field MUST NOT be with value with more bits then length
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer
        GT_BAD_PARAM    - on bad parameter : info.length == 0 or  valueArr[...] > maxValue
*/
#define KEY_ONLY_SET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_keyPatternBytes)          \
{                                                                                                   \
    GT_U32    _wordsArr[4];                                                                         \
                                                                                                    \
    /* convert the 5 bytes of key to 2 words */                                                     \
    _wordsArr[0] = _keyPatternBytes[0]<<0   |                                                       \
                   _keyPatternBytes[1]<<8   |                                                       \
                   _keyPatternBytes[2]<<16  |                                                       \
                   _keyPatternBytes[3]<<24;                                                         \
    _wordsArr[1] = _keyPatternBytes[4]<<0   |                                                       \
                   _keyPatternBytes[5]<<8   |                                                       \
                   _keyPatternBytes[6]<<16  |                                                       \
                   _keyPatternBytes[7]<<24;                                                         \
    _wordsArr[2] = _keyPatternBytes[8]<<0   |                                                       \
                   _keyPatternBytes[9]<<8   |                                                       \
                   _keyPatternBytes[10]<<16 |                                                       \
                   _keyPatternBytes[11]<<24;                                                        \
    _wordsArr[3] = (_keyPatternBytes[12]&0xFFFF)<<0 |                                               \
                   (_keyPatternBytes[13]&0xFFFF)<<8;                                                \
                                                                                                    \
    U_ANY_SET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_wordsArr);                       \
}

/* Get 5 bytes of Key to a 'key' field (40 bits) in a table entry.

   startEntryPtr - pointer to start entry data
   fieldsInfoArr[] - array of fields info.
   fieldId - the 'Field ID'  - index in fieldsInfoArr[].
OUT:
   _keyPatternBytes[] - the array of 5 bytes (key bits[0-39])
NOTEs:
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer

*/
#define KEY_ONLY_GET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_keyPatternBytes)             \
{                                                                                                \
    GT_U32    _wordsArr[4];                                                                      \
                                                                                                 \
    U_ANY_GET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_wordsArr);                    \
                                                                                                 \
    /* convert the 3 words to 8 bytes of reduce action plus expandedActionIndex */               \
    (_keyPatternBytes)[0] = (GT_U8)(_wordsArr[0] >>  0);                                         \
    (_keyPatternBytes)[1] = (GT_U8)(_wordsArr[0] >>  8);                                         \
    (_keyPatternBytes)[2] = (GT_U8)(_wordsArr[0] >>  16);                                        \
    (_keyPatternBytes)[3] = (GT_U8)(_wordsArr[0] >>  24);                                        \
    (_keyPatternBytes)[4] = (GT_U8)(_wordsArr[1] >>  0);                                         \
    (_keyPatternBytes)[5] = (GT_U8)(_wordsArr[1] >>  8);                                         \
    (_keyPatternBytes)[6] = (GT_U8)(_wordsArr[1] >>  16);                                        \
    (_keyPatternBytes)[7] = (GT_U8)(_wordsArr[1] >>  24);                                        \
    (_keyPatternBytes)[8] = (GT_U8)(_wordsArr[2] >>  0);                                         \
    (_keyPatternBytes)[9] = (GT_U8)(_wordsArr[2] >>  8);                                         \
    (_keyPatternBytes)[10] = (GT_U8)(_wordsArr[2] >> 16);                                        \
    (_keyPatternBytes)[11] = (GT_U8)(_wordsArr[2] >>  24);                                       \
    (_keyPatternBytes)[12] = (GT_U8)(_wordsArr[3] >>  0);                                        \
    (_keyPatternBytes)[13] = (GT_U8)(_wordsArr[3] >>  8);                                        \
}

/* Get the start bit and the length of specific field in entry format.
   fieldsInfoArr[] - array of fields info.
   fieldId - the 'Field ID'  - index in fieldsInfoArr[].
OUT:
    startBit     - the start bit of the field.
    numOfBits    - the number of bits of the field.
NOTEs:
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer
*/
#define GET_FIELD_INFO_BY_ID_MAC(fieldsInfoArr,fieldId,startBit,numOfBits)      \
{                                                                               \
    GT_STATUS _rc;                                                              \
    _rc = prvCpssFieldInEntryInfoGet(fieldsInfoArr,fieldId,&startBit,&numOfBits); \
    if (_rc != GT_OK)                                                           \
    {                                                                           \
        return _rc;                                                             \
    }                                                                           \
}

/* Get 'any number of bits' value to a field in a table entry.
   the bits returned are in little Endian order in the array of words.

   startEntryPtr - pointer to start entry data
   fieldsInfoArr[] - array of fields info.
   fieldId - the 'Field ID'  - index in fieldsInfoArr[].
OUT:
   valArr[] - the array of values to get
NOTEs:
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer

*/
#define U_ANY_GET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,valArr)        \
{                                                                               \
    GT_STATUS _rc;                                                              \
    _rc = prvCpssFieldToEntry_GT_Any_Get(startEntryPtr,fieldsInfoArr,fieldId,valArr);   \
    if (_rc != GT_OK)                                                           \
    {                                                                           \
        return _rc;                                                             \
    }                                                                           \
}

/* Get 6 bytes of MacAddr from a 'mac address' field (48 bits) in a table entry.

   startEntryPtr - pointer to start entry data
   fieldsInfoArr[] - array of fields info.
   fieldId - the 'Field ID'  - index in fieldsInfoArr[].
OUT:
   _macAddrBytes[] - the array of 6 bytes (mac Addr)
NOTEs:
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer

*/
#define MAC_ADDR_GET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_macAddrBytes)   \
{                                                                             \
    GT_U32    _wordsArr[2];                                                   \
                                                                              \
    U_ANY_GET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_wordsArr); \
                                                                              \
    /* convert the 2 words to 6 bytes of mac address */                       \
    (_macAddrBytes)[0] = (GT_U8)(_wordsArr[1] >>  8);                         \
    (_macAddrBytes)[1] = (GT_U8)(_wordsArr[1] >>  0);                         \
    (_macAddrBytes)[2] = (GT_U8)(_wordsArr[0] >> 24);                         \
    (_macAddrBytes)[3] = (GT_U8)(_wordsArr[0] >> 16);                         \
    (_macAddrBytes)[4] = (GT_U8)(_wordsArr[0] >>  8);                         \
    (_macAddrBytes)[5] = (GT_U8)(_wordsArr[0] >>  0);                         \
}

/* Get 16 bytes of IPv6 Address from a 'ipv6 address' field (128 bits) in a table entry.

   startEntryPtr - pointer to start entry data
   fieldsInfoArr[] - array of fields info.
   fieldId - the 'Field ID'  - index in fieldsInfoArr[].
OUT:
   _ipv6AddrBytes[] - the array of 16 bytes (ipv6 Addr)
NOTEs:
   Return values:
        GT_OK           - no error
        GT_BAD_PTR      - on NULL pointer

*/
#define IPV6_ADDR_GET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_ipv6AddrBytes)   \
{                                                                               \
    GT_U32    _wordsArr[4];                                                     \
                                                                                \
    U_ANY_GET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_wordsArr);   \
                                                                                \
    /* convert the 4 words to 16 bytes of ipv6 address */                       \
    (_ipv6AddrBytes)[0]  = (GT_U8)(_wordsArr[3] >> 24);                         \
    (_ipv6AddrBytes)[1]  = (GT_U8)(_wordsArr[3] >> 16);                         \
    (_ipv6AddrBytes)[2]  = (GT_U8)(_wordsArr[3] >>  8);                         \
    (_ipv6AddrBytes)[3]  = (GT_U8)(_wordsArr[3] >>  0);                         \
    (_ipv6AddrBytes)[4]  = (GT_U8)(_wordsArr[2] >> 24);                         \
    (_ipv6AddrBytes)[5]  = (GT_U8)(_wordsArr[2] >> 16);                         \
    (_ipv6AddrBytes)[6]  = (GT_U8)(_wordsArr[2] >>  8);                         \
    (_ipv6AddrBytes)[7]  = (GT_U8)(_wordsArr[2] >>  0);                         \
    (_ipv6AddrBytes)[8]  = (GT_U8)(_wordsArr[1] >> 24);                         \
    (_ipv6AddrBytes)[9]  = (GT_U8)(_wordsArr[1] >> 16);                         \
    (_ipv6AddrBytes)[10] = (GT_U8)(_wordsArr[1] >>  8);                         \
    (_ipv6AddrBytes)[11] = (GT_U8)(_wordsArr[1] >>  0);                         \
    (_ipv6AddrBytes)[12] = (GT_U8)(_wordsArr[0] >> 24);                         \
    (_ipv6AddrBytes)[13] = (GT_U8)(_wordsArr[0] >> 16);                         \
    (_ipv6AddrBytes)[14] = (GT_U8)(_wordsArr[0] >>  8);                         \
    (_ipv6AddrBytes)[15] = (GT_U8)(_wordsArr[0] >>  0);                         \
}

/* calc the word index for offset from start of entry , use with OFFSET_TO_BIT_MAC */
#define OFFSET_TO_WORD_MAC(_offset)     ((_offset) >> 5)

/* calc the bit index for offset from start of entry , use with OFFSET_TO_WORD_MAC */
#define OFFSET_TO_BIT_MAC(_offset)     ((_offset) & 0x1f)


/* Sets the field located at the specified offset & length in data.
   NOTE :
   1. with out clearing the field ---> Assume that that the field was
      already cleared !!!
   2. the field MUST NOT be with value with more bits then length
   3. length is NOT USED !!!
*/
#define U32_SET_FIELD_NO_CLEAR_MAC(data,offset,length,val)           \
   (data) |= ((val) <<(offset))

/* Sets the field located at the specified offset & length in data.
   NOTE :
   1. with out clearing the field ---> Assume that that the field was
      already cleared !!!
   2. the field may be with value with more bits then length
*/
#define U32_SET_FIELD_NO_CLEAR_MASKED_MAC(data,offset,length,val)    \
    U32_SET_FIELD_NO_CLEAR_MAC(data,offset,length,((val) & BIT_MASK_MAC(length) ))


#define CALC_MASK_MAC(fieldLen, fieldOffset, mask)     \
            if(((fieldLen) + (fieldOffset)) >= 32)     \
             (mask) = (GT_U32)(0 - (1<< (fieldOffset)));    \
            else                                   \
          (mask) = (((1<<((fieldLen) + (fieldOffset)))) - (1 << (fieldOffset)))

#define BYTESWAP_MAC(data)                      \
        (((data) << 24))  |                 \
        (((data) & 0xff00)      << 8)   |   \
        (((data) & 0xff0000)    >> 8)   |   \
        (((data) >> 24))


/* A macro for performing left shifting on a 64 bit data            */
/* highData -   A 32 bit word including the MSB part of the data    */
/* lowData  -   A 32 bit word including the LSB part of the data    */
/* shift    -   Number of bits to shift.                            */
/* overFlow -   The (left) part of highData which overflowed.       */
#define U64_SHIFT_LEFT_MAC(overFlow,highData,lowData,shift)         \
        (overFlow) = ((overFlow) << (shift)) | ((highData) >> (32 - (shift))); \
        (highData) = ((highData) << (shift));                      \
        (highData)|= ((lowData) >> (32 - (shift)));              \
        (lowData)  = ((lowData) << (shift))

#define U64_SHIFT_RIGHT_MAC(highData,lowData,underFlow,shift)         \
        (underFlow) = ((underFlow) >> (shift)) | ((lowData) << (32 - (shift)));\
        (lowData)   = ((lowData) >> (shift));                      \
        (lowData)  |= ((highData) << (32 - (shift)));              \
        (highData)  = ((highData) >> (shift))

#define BOOL2BIT_MAC(x) (((x) == GT_FALSE) ? 0 : 1)

#define BIT2BOOL_MAC(x) (((x) == 1) ? GT_TRUE : GT_FALSE)
/* Return the minimum of x & y. */
#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif  /* MIN */
#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif  /* MAX */


#define U8_SWAP_BITS(x)         \
        ((((x)&0x80)>>7)|(((x)&0x40)>>5)|(((x)&0x20)>>3)|(((x)&0x10)>>1) \
         |(((x)&0x1)<<7) | (((x)&0x2)<<5) | (((x)&0x4)<<3) |(((x)&0x8)<<1) )


/* the MACRO is integer that has bit number 'x' is 1
   and all other bits are 0 */
#define BYTES_ARRAY_BIT_MASK(x)   \
        (1 << (x))

/* Get the byte of the x bit from an array of bytes */
#define BYTES_ARRAY_GET_BYTE(a,x) \
        ( ( (GT_U8 *) a)[(x)/8] )

/* checks wheter bit 'x' in 'a' is set and than returns TRUE,  */
/* otherwise return FALSE.                                     */
#define BYTES_ARRAY_CHKBIT(a,x) \
    ( ( BYTES_ARRAY_GET_BYTE(a,x) & BYTES_ARRAY_BIT_MASK((x)%8) ) >> ((x)%8) )

/* Clear (reset) bit 'x' in an array of bytes */
#define BYTES_ARRAY_CLRBIT(a,x) \
    (BYTES_ARRAY_GET_BYTE(a,x) &=  ~(BYTES_ARRAY_BIT_MASK((x)%8) ) )

/* Clear (reset) bit 'x' in an array of bytes */
#define BYTES_ARRAY_SETBIT(a,x) \
    (BYTES_ARRAY_GET_BYTE(a,x) |=  BYTES_ARRAY_BIT_MASK((x)%8) )

/*   INVERT bit 'x' in in an array of bytes. */
#define BYTES_ARRAY_INVBIT(a,x) \
    ( BYTES_ARRAY_GET_BYTE(a,x) ^= BYTES_ARRAY_BIT_MASK((x)%8) )

/* Get/set a nibble from/to up to U32 field
 * Nibble 0 represent bits 0:3 etc
 */
#define GT_NIBBLE_SIZE_CNS (4)
#define GT_GET_NIBBLE_MAC(field, nibbleIdx) \
            U32_GET_FIELD_MAC(field, (nibbleIdx * GT_NIBBLE_SIZE_CNS), GT_NIBBLE_SIZE_CNS);

#define GT_SET_NIBBLE_MAC(field, nibbleIdx, val) \
            U32_SET_FIELD_MAC(field, (nibbleIdx * GT_NIBBLE_SIZE_CNS), GT_NIBBLE_SIZE_CNS, val);

/* check that a number is not out of range of max_bit_num */
#define CHECK_BITS_DATA_RANGE_MAC(number,max_bit_num) \
        ( number < ( 1 << max_bit_num )) ? GT_TRUE:GT_FALSE

/* macros to convert X/Y format to pattarn/mask format and
   convert pattern/mask format to X/Y format */
/* the function "bit-pair ==> bit" name coded f(0,0), f(0,1), f(1,0), f(1,1) */

/* f(0,0)=0, f(0,1)=0, f(1,0)=1, f(1,1)=0 */
#define PRV_BIT_OPER_0010_MAC(_x,_y) ((_x) & (~ (_y)))

/* f(0,0)=0, f(0,1)=1, f(1,0)=1, f(1,1)=0 */
#define PRV_BIT_OPER_0110_MAC(_x,_y) ((_x) ^ (_y))

/* f(0,0)=0, f(0,1)=0, f(1,0)=0, f(1,1)=1 */
#define PRV_BIT_OPER_0001_MAC(_x,_y) ((_x) & (_y))

/* f(0,0)=0, f(0,1)=1, f(1,0)=0, f(1,1)=0 */
#define PRV_BIT_OPER_0100_MAC(_x,_y) ((~ (_x)) & (_y))

/* convert bits to number of words needed to occupy the bits */
#define BITS_TO_WORDS_MAC(bits) (((bits) + 31) / 32)



#ifndef __gtCommonDefsh
/**
* @enum GT_INT_TYPE
 *
 * @brief GT device interrupt connection type
*/
typedef enum{

    /** @brief interrupt line is connected to packet
     *  processor GPP pin 0.
     */
    GT_INT_2_PP_GPP0 = 0,

    /** @brief interrupt line is connected to packet
     *  processor GPP pin 1.
     */
    GT_INT_2_PP_GPP1,

    /** @brief interrupt line is connected to packet
     *  processor GPP pin 2.
     */
    GT_INT_2_PP_GPP2,

    /** @brief interrupt line is connected to packet
     *  processor uplink GPP.
     */
    GT_INT_2_UPLINK_GPP,

    /** @brief interrupt is directly connected to PCI A
     *  interrupt
     */
    GT_INT_2_PCI_A,

    /** @brief interrupt is directly connected to PCI B
     *  interrupt
     */
    GT_INT_2_PCI_B,

    /** @brief interrupt is directly connected to PCI C
     *  interrupt
     */
    GT_INT_2_PCI_C,

    /** @brief interrupt is directly connected to PCI D
     *  interrupt
     */
    GT_INT_2_PCI_D,

    /** interrupt is connected in some other manner. */
    GT_INT_OTHER

} GT_INT_TYPE;


/**
* @struct GT_INT
 *
 * @brief GT device Interrupt definition
*/
typedef struct{

    /** interrupt connection type. */
    GT_INT_TYPE intType;

    /** @brief The interrupt vector number this device is connected to in
     *  case of PCI or other interrupt, or the packet processor device
     *  number connected to by its GPP pin.
     */
    GT_U32 intVecNum;

    /** The interrupt mask to enable GT interrupts (used in Mips) */
    GT_UINTPTR intMask;

} GT_INT;
#endif  /* __gtCommonDefsh */

/* binary semaphore for CPSS events resources mutual exclusion access protection */
/* Lock based on prvCpssLockSem mutex */
#define PRV_CPSS_LOCK_BY_MUTEX \
    cpssOsMutexLock(PRV_SHARED_GLOBAL_VAR_GET( \
        commonMod.genericHwInitDir.prvCpssLockSem))
#define PRV_CPSS_UNLOCK_BY_MUTEX \
    cpssOsMutexUnlock(PRV_SHARED_GLOBAL_VAR_GET( \
        commonMod.genericHwInitDir.prvCpssLockSem))

/*
  macros for no mutual exclusion protection by mutex
  use this macro to : - maintain cpss api regular structure appearance locking & unlocking
                      - mention that this api doesn't use mutex knowingly
*/
#define CPSS_NO_MUTEX_API_LOCK_MAC
#define CPSS_NO_MUTEX_API_UNLOCK_MAC

/* macros for the CPSS mutual exclusion protection locking & unlocking */
/* flow\mechanism.                                                     */
/* _resv1 & _resv2 are for future use.                                 */
#ifdef CPSS_API_LOCK_PROTECTION
#include <cpss/extServices/os/gtOs/cpssOsSem.h>

/* CPSS API mutual exclusion protection by mutex (deviceless) */
#define PRV_CPSS_MAX_FUNCTIONALITY_RESOLUTION_CNS 8

/* CPSS API mutual exclusion protection by mutex (per device) */
#define PRV_CPSS_MAX_DEV_FUNCTIONALITY_RESOLUTION_CNS 4

/*macro for API used for configuration interface functionality*/
#define PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS    (0)
/*macro for API used for  network interface functionality*/
#define PRV_CPSS_FUNCTIONALITY_RXTX_CNS             (1)
/*macro for API used for TRUNK MANAGER functionality (configurations - no traffic) */
#define PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRUNK_MANAGER_CNS (2)
/*macro for API used for IPFIX MANAGER functionality (configurations - no traffic) */
#define PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS (3)

/*macro for API used for FDB MANAGER functionality (configurations - no traffic) */
#define PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS      (3)
/*macro for API used for EXACT_MATCH MANAGER functionality (configurations - no traffic) */
#define PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS (4)
/*macro for API used for LPM MANAGER functionality (configurations - no traffic) */
#define PRV_CPSS_FUNCTIONALITY_LPM_MANAGER_CNS (5)
/*macro for API used for TCAM MANAGER functionality (configurations - no traffic) */
#define PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS (6)
/*macro for API used for SYSTEM RECOVERY functionality (configurations - no traffic) */
#define PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS (7)

/*macro for API that does not use device number and does not have nested CPSS API calls*/
#define CPSS_ZERO_LEVEL_API_LOCK_MAC\
    do\
    {\
        if( (NULL != cpssSharedGlobalVarsPtr)&& \
            (GT_TRUE ==PRV_SHARED_GLOBAL_VAR_GET(sysConfigPhase1Done)))\
        {\
            cpssOsMutexLock(PRV_SHARED_GLOBAL_VAR_GET( \
                commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtxZeroLevel));\
        }\
        else \
        {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"Mutex global shared DB is not initialized\n"); \
        }\
    }\
    while(0);


#define CPSS_ZERO_LEVEL_API_LOCK_NO_RETURN_MAC\
        do\
        {\
            if( (NULL != cpssSharedGlobalVarsPtr)&& \
                (GT_TRUE ==PRV_SHARED_GLOBAL_VAR_GET(sysConfigPhase1Done)))\
            {\
                cpssOsMutexLock(PRV_SHARED_GLOBAL_VAR_GET( \
                    commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtxZeroLevel));\
            }\
            else \
            {\
                CPSS_LOG_ERROR_MAC("Mutex global shared DB is not initialized\n"); \
            }\
        }\
        while(0);

#define CPSS_ZERO_LEVEL_API_UNLOCK_MAC\
    do\
    {\
        if( (NULL != cpssSharedGlobalVarsPtr)&& \
            (GT_TRUE ==PRV_SHARED_GLOBAL_VAR_GET(sysConfigPhase1Done)))\
        {\
            cpssOsMutexUnlock(PRV_SHARED_GLOBAL_VAR_GET( \
                commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtxZeroLevel));\
        } \
        else \
        {\
            CPSS_LOG_ERROR_MAC("Mutex global shared DB is not initialized\n"); \
        }\
    }\
    while(0);

/*
_devNum - device number
_functionality - TRUE for network I/F API ,FALSE otherwise
*/


#define CPSS_API_LOCK_MAC(_devNum,_functionality)\
    if (_devNum < PRV_CPSS_MAX_PP_DEVICES_CNS)\
    {\
        if( (NULL != cpssSharedGlobalVarsPtr)&& \
            (GT_TRUE ==PRV_SHARED_GLOBAL_VAR_GET(sysConfigPhase1Done)))\
        {\
            cpssOsMutexLock(PRV_SHARED_GLOBAL_VAR_GET( \
                commonMod.genericHwInitDir.prvCpssApiLockByDevNumAndFunctionalityMtx[_devNum][_functionality]));\
        }\
        else \
        {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"Mutex global shared DB is not initialized\n"); \
        }\
    }

/* MACRO like CPSS_API_LOCK_MAC but without _devNum range checks. This MACRO should be used in code
   that already checked that _devNum < PRV_CPSS_MAX_PP_DEVICES_CNS */
#define CPSS_API_LOCK_NO_CHECKS_MAC(_devNum,_functionality)\
    cpssOsMutexLock(PRV_SHARED_GLOBAL_VAR_GET( \
            commonMod.genericHwInitDir.prvCpssApiLockByDevNumAndFunctionalityMtx[_devNum][_functionality]));


#define CPSS_API_LOCK_DEVICELESS_MAC(_functionality)\
    {\
        if( (NULL != cpssSharedGlobalVarsPtr)&& \
            (GT_TRUE ==PRV_SHARED_GLOBAL_VAR_GET(sysConfigPhase1Done)))\
        {\
         cpssOsMutexLock(PRV_SHARED_GLOBAL_VAR_GET( \
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[_functionality])); \
        }\
        else \
        {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"Mutex global shared DB is not initialized\n"); \
        }\
    }

/*Used for void functions*/

#define CPSS_API_LOCK_DEVICELESS_NO_RETURN_MAC(_functionality)\
    {\
        if( (NULL != cpssSharedGlobalVarsPtr)&& \
            (GT_TRUE ==PRV_SHARED_GLOBAL_VAR_GET(sysConfigPhase1Done)))\
        {\
         cpssOsMutexLock(PRV_SHARED_GLOBAL_VAR_GET( \
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[_functionality])); \
        }\
        else \
        {\
            CPSS_LOG_ERROR_MAC("Mutex global shared DB is not initialized\n"); \
        }\
    }



#define CPSS_API_UNLOCK_MAC(_devNum,_functionality)\
    if (_devNum < PRV_CPSS_MAX_PP_DEVICES_CNS)\
    {\
        if( (NULL != cpssSharedGlobalVarsPtr)&& \
            (GT_TRUE ==PRV_SHARED_GLOBAL_VAR_GET(sysConfigPhase1Done)))\
        {\
         cpssOsMutexUnlock(PRV_SHARED_GLOBAL_VAR_GET( \
            commonMod.genericHwInitDir.prvCpssApiLockByDevNumAndFunctionalityMtx[_devNum][_functionality]));\
        }\
        else \
        {\
            CPSS_LOG_ERROR_MAC("Mutex global shared DB is not initialized\n"); \
        }\
    }

/* MACRO like CPSS_API_UNLOCK_MAC but without _devNum range checks. This MACRO should be used in code
   that already checked that _devNum < PRV_CPSS_MAX_PP_DEVICES_CNS */
#define CPSS_API_UNLOCK_NO_CHECKS_MAC(_devNum,_functionality)\
    cpssOsMutexUnlock(PRV_SHARED_GLOBAL_VAR_GET( \
        commonMod.genericHwInitDir.prvCpssApiLockByDevNumAndFunctionalityMtx[_devNum][_functionality]));

#define CPSS_API_UNLOCK_DEVICELESS_MAC(_functionality)\
    {\
        if( (NULL != cpssSharedGlobalVarsPtr)&& \
            (GT_TRUE ==PRV_SHARED_GLOBAL_VAR_GET(sysConfigPhase1Done)))\
        {\
        cpssOsMutexUnlock(PRV_SHARED_GLOBAL_VAR_GET( \
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[_functionality]));\
        }\
        else \
        {\
            CPSS_LOG_ERROR_MAC("Mutex global shared DB is not initialized\n"); \
        }\
    }

#else
#define CPSS_API_LOCK_MAC(_resv1, _resv2)
#define CPSS_API_UNLOCK_MAC(_resv1, _resv2)
#define CPSS_ZERO_LEVEL_API_LOCK_MAC
#define CPSS_ZERO_LEVEL_API_UNLOCK_MAC
#define CPSS_ZERO_LEVEL_API_LOCK_NO_RETURN_MAC
#define CPSS_API_LOCK_DEVICELESS_MAC(_functionality)
#define CPSS_API_LOCK_DEVICELESS_NO_RETURN_MAC(_functionality)
#define CPSS_API_UNLOCK_DEVICELESS_MAC(_functionality)
#define CPSS_API_UNLOCK_NO_CHECKS_MAC(_devNum,_functionality)
#define CPSS_API_LOCK_NO_CHECKS_MAC(_devNum,_functionality)
#endif /* CPSS_API_LOCK_PROTECTION */

extern CPSS_OS_MUTEX prvCpssIntrScanMtx;
#define PRV_CPSS_INT_SCAN_LOCK() cpssOsMutexLock(prvCpssIntrScanMtx)
#define PRV_CPSS_INT_SCAN_UNLOCK() cpssOsMutexUnlock(prvCpssIntrScanMtx)

#if (defined ASIC_SIMULATION)
    /*  check if the device runs over WM : as native WM or as 'asim env' */
    /* in WM native : always '1' */
    #define CPSS_DEV_IS_WM_NATIVE_OR_ASIM_MAC(_devNum)  1
#elif (defined MV_HWS_FREE_RTOS) /* !ASIC_SIMULATION */
    /* check if the device runs over WM : as native WM or as 'asim env' */
    /* in MV_HWS_FREE_RTOS : always 0                                   */
    #define CPSS_DEV_IS_WM_NATIVE_OR_ASIM_MAC(_devNum)  0
#else /* !ASIC_SIMULATION and !MV_HWS_FREE_RTOS */
    /* check if the device runs over WM : as native WM or as 'asim env' */
    /* in non-WM native : depend if this is 'asim' system               */
    /* in non-asim env  : will be '0'                                   */
    /* in     asim env  : will be '1'                                   */
    #define CPSS_DEV_IS_WM_NATIVE_OR_ASIM_MAC(_devNum)  \
        (PRV_SHARED_GLOBAL_VAR_GET(isWmDeviceInSimEnv))
#endif /* ASIC_SIMULATION */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPresteraDefsh */

