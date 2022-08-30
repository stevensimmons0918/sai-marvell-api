/* basic_defs.h
 *
 * Driver Framework v4, Basic Definitions.
 *
 * This file provides a number of basic definitions and can be customized for
 * any compiler.
 */

/*****************************************************************************
* Copyright (c) 2007-2016 INSIDE Secure B.V. All Rights Reserved.
*
* This confidential and proprietary software may be used only as authorized
* by a licensing agreement from INSIDE Secure.
*
* The entire notice above must be reproduced on all authorized copies that
* may only be made to the extent permitted by a licensing agreement from
* INSIDE Secure.
*
* For more information or support, please go to our online support system at
* https://customersupport.insidesecure.com.
* In case you do not have an account for this system, please send an e-mail
* to ESSEmbeddedHW-Support@insidesecure.com.
*****************************************************************************/

#ifndef INCLUDE_GUARD_BASIC_DEFS_H
#define INCLUDE_GUARD_BASIC_DEFS_H

/* include cpss environment types as LINUX, GT_INLINE etc. */
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>

/* ============ Compiler-based Configuration ============ */

/*#if __STDC_VERSION__ < 199901L */
/*#error "BasicDefs: C standards older than C99 not supported" */
/*#endif */

#ifdef __cplusplus
#error "BasicDefs: C++ standard not supported"
#endif

#ifdef _MSC_VER
#define BASICDEFS_COMPILER_MICROSOFT_V1
#define BASICDEFS_TYPES_V1
#define BASICDEFS_BOOL_V1
/* Microsoft compiler only supports "inline" in C++ mode and */
/* expects __inline in C mode */
#define BASICDEFS_INLINE_V1

#define __func__    __FUNCTION__
#endif

#ifdef LINUX         /* change it from linux to LINUX to fit CPSS format */
#ifdef __KERNEL__    /* change it from module to __KERNAEL__ to fit CPSS format */
/* kernel loadable module */
#define BASICDEFS_COMPILER_LINUX_V1
#define BASICDEFS_INLINE_BUILTIN
#include <linux/types.h>
#else
/* user mode */
#define BASICDEFS_COMPILER_LINUX_V1
#define BASICDEFS_TYPES_STDINT
#define BASICDEFS_BOOL_V1
#define BASICDEFS_INLINE_BUILTIN
#endif
#endif

#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 199901L
#define BASICDEFS_TYPES_STDINT
#define BASICDEFS_BOOL_V1
#define BASICDEFS_INLINE_BUILTIN
#endif
#endif

#ifdef __CYGWIN__
#define BASICDEFS_COMPILER_CYGWIN_V1
#define BASICDEFS_TYPES_STDINT
#define BASICDEFS_BOOL_V1
#define BASICDEFS_INLINE_BUILTIN
#endif

#ifdef __MINGW64__
#define BASICDEFS_COMPILER_LINUX_V1
#define BASICDEFS_TYPES_STDINT
#define BASICDEFS_BOOL_V1
#define BASICDEFS_INLINE_BUILTIN
#endif
/* ============ Basic Types ============ */

#ifdef BASICDEFS_TYPES_STDINT
/* ISO-C99 */
#include <stdint.h>
#endif

#ifdef BASICDEFS_TYPES_V1
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
#endif

#define true GT_TRUE
#define false GT_FALSE

/* ============ Boolean type ============ */

#ifdef BASICDEFS_BOOL_V1
/* ISO-C99 */
/*#include <stdbool.h>*/
#define bool GT_BOOL
#endif

/* ============ NULL ============ */

#ifndef NULL
#define NULL 0
#endif

/* ============ MASK_n_BITS ============ */

#define MASK_1_BIT      (BIT_1 - 1)
#define MASK_2_BITS     (BIT_2 - 1)
#define MASK_3_BITS     (BIT_3 - 1)
#define MASK_4_BITS     (BIT_4 - 1)
#define MASK_5_BITS     (BIT_5 - 1)
#define MASK_6_BITS     (BIT_6 - 1)
#define MASK_7_BITS     (BIT_7 - 1)
#define MASK_8_BITS     (BIT_8 - 1)
#define MASK_9_BITS     (BIT_9 - 1)
#define MASK_10_BITS    (BIT_10 - 1)
#define MASK_11_BITS    (BIT_11 - 1)
#define MASK_12_BITS    (BIT_12 - 1)
#define MASK_13_BITS    (BIT_13 - 1)
#define MASK_14_BITS    (BIT_14 - 1)
#define MASK_15_BITS    (BIT_15 - 1)
#define MASK_16_BITS    (BIT_16 - 1)
#define MASK_17_BITS    (BIT_17 - 1)
#define MASK_18_BITS    (BIT_18 - 1)
#define MASK_19_BITS    (BIT_19 - 1)
#define MASK_20_BITS    (BIT_20 - 1)
#define MASK_21_BITS    (BIT_21 - 1)
#define MASK_22_BITS    (BIT_22 - 1)
#define MASK_23_BITS    (BIT_23 - 1)
#define MASK_24_BITS    (BIT_24 - 1)
#define MASK_25_BITS    (BIT_25 - 1)
#define MASK_26_BITS    (BIT_26 - 1)
#define MASK_27_BITS    (BIT_27 - 1)
#define MASK_28_BITS    (BIT_28 - 1)
#define MASK_29_BITS    (BIT_29 - 1)
#define MASK_30_BITS    (BIT_30 - 1)
#define MASK_31_BITS    (BIT_31 - 1)

/* ============ IDENTIFIER_NOT_USED ============ */

#define IDENTIFIER_NOT_USED(_v)   (void)(_v)

/* ============ inline ============ */

#ifdef BASICDEFS_INLINE_V1
#ifndef inline
#define inline __inline
#endif
#endif

/* Set inline command to be taken from CPSS inline definition */  /*doronf*/
#ifdef BASICDEFS_INLINE_BUILTIN
#ifndef inline
#define inline  GT_INLINE
#endif
#endif


#endif /* Inclusion Guard */


/* end of file basic_defs.h */
