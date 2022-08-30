/* *****************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/**
 * @file mpdTypes.h
 * @brief Global general type declarations
 *
 */

#ifndef __mpdTypesh

#define __mpdTypesh

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef _WIN32
#include <stdint.h>

    /**
     * @typedef int8_t INT_8
     *
     * @brief  singed 8 bit integer
     */

    /**
     * @typedef int8_t INT_8
     *
     * @brief   Defines an alias representing the int 8
     */

    typedef int8_t   INT_8;

    /**
     * @typedef int16_t INT_16
     *
     * @brief  singed 16 bit integer
     */

    /**
     * @typedef int16_t INT_16
     *
     * @brief   Defines an alias representing the int 16
     */

    typedef int16_t  INT_16;

    /**
     * @typedef int32_t INT_32
     *
     * @brief  singed 32 bit integer
     */

    /**
     * @typedef int32_t INT_32
     *
     * @brief   Defines an alias representing the int 32
     */

    typedef int32_t  INT_32;

    /**
     * @typedef uint8_t UINT_8
     *
     * @brief  unsigned 8 bit integer
     */

    /**
     * @typedef uint8_t UINT_8
     *
     * @brief   Defines an alias representing the 8
     */

    typedef uint8_t  UINT_8;

    /**
     * @typedef uint16_t UINT_16
     *
     * @brief  unsigned 16 bit integer
     */

    /**
     * @typedef uint16_t UINT_16
     *
     * @brief   Defines an alias representing the 16
     */

    typedef uint16_t UINT_16;

    /**
     * @typedef uint32_t UINT_32
     *
     * @brief  unsigned 32 bit integer
     */

    /**
     * @typedef uint32_t UINT_32
     *
     * @brief   Defines an alias representing the 32
     */

    typedef uint32_t UINT_32;
#else

    /**
     * @typedef char INT_8
     *
     * @brief  singed 8 bit integer
     */

    /**
     * @typedef char INT_8
     *
     * @brief   Defines an alias representing the int 8
     */

    typedef char  INT_8;

    /**
     * @typedef short INT_16
     *
     * @brief  singed 16 bit integer
     */

    /**
     * @typedef short INT_16
     *
     * @brief   Defines an alias representing the int 16
     */

    typedef short INT_16;

    /**
     * @typedef int INT_32
     *
     * @brief  singed 32 bit integer
     */

    /**
     * @typedef int INT_32
     *
     * @brief   Defines an alias representing the int 32
     */

    typedef int  INT_32;

    /**
     * @typedef unsigned char UINT_8
     *
     * @brief  unsigned 8 bit integer
     */

    /**
     * @typedef unsigned char UINT_8
     *
     * @brief   Defines an alias representing the 8
     */

    typedef unsigned char  UINT_8;

    /**
     * @typedef unsigned short UINT_16
     *
     * @brief  unsigned 16 bit integer
     */

    /**
     * @typedef unsigned short UINT_16
     *
     * @brief   Defines an alias representing the 16
     */

    typedef unsigned short UINT_16;

    /**
     * @typedef unsigned int UINT_32
     *
     * @brief  unsigned 32 bit integer
     */

    /**
     * @typedef unsigned int UINT_32
     *
     * @brief   Defines an alias representing the 32
     */

    typedef unsigned int  UINT_32;
#endif

/* the 'GT_' types are CPSS types that should not be defined if already exists */
#ifndef __gtEnvDeph
/**
 * @typedef UINT_8 GT_U8
 *
 * @brief  unsigned 8 bit integer
 */

/**
 * @typedef UINT_8 GT_U8
 *
 * @brief   Defines an alias representing the gt u 8
 */

typedef UINT_8          GT_U8;
typedef UINT_16         GT_U16;

/**
 * @typedef UINT_32 GT_U32
 *
 * @brief   Defines an alias representing the gt u 32
 */

typedef UINT_32         GT_U32;
#endif /*__gtEnvDeph*/
/**
 * @typedef UINT_8 BOOLEAN
 *
 * @brief  Boolean definition
 */

/**
 * @typedef UINT_8 BOOLEAN
 *
 * @brief   Defines an alias representing the boolean
 */

typedef UINT_8 BOOLEAN;

#ifdef TRUE
#undef TRUE
#endif

/* True Value */
#define TRUE  1

#ifdef FALSE
#undef FALSE
#endif

/* False Value */
#define FALSE 0

/* Max 8 bit unsigned value */
#define MAX_UINT_8           ((UINT_8) 0xFF)

/* Max 16 bit unsigned value */
#define MAX_UINT_16          ((UINT_16)0xFFFF)

/* Max 32 bit unsigned value */
#define MAX_UINT_32          ((UINT_32)0xFFFFFFFF)

/* brief indicates this is an input parameter */
#define IN

/* indicates this is an output parameter */
#define OUT

/* indicates this is an input and output parameter */
#define INOUT


typedef enum {
    MPD_OK_E,            /**< OK */
    MPD_NOT_SUPPORTED_E, /**< Not Supported */
    MPD_OP_FAILED_E      /**< Failed */
} MPD_RESULT_ENT;

/* use MPD_UNUSED_PARAM to avoid next compilation errors/warnings       */
/* we compile GCC with very strict compilation warnings as errors */
/* VC10 : warning C4100: 'resultPtr' : unreferenced formal parameter    */
/* GCC error: unused parameter 'resultPtr' [-Werror=unused-parameter]   */
#define MPD_UNUSED_PARAM(x) x = x

#if defined(__GNUC__) && __GNUC__ >= 7
 #define MPD_FALLTHROUGH __attribute__ ((fallthrough));
#else
 #define MPD_FALLTHROUGH ((void)0);
#endif /* __GNUC__ >= 7 */

/*#ifndef _WIN32
#define MPD_FALLTHROUGH __attribute__ ((fallthrough));
#else
#define MPD_FALLTHROUGH
#endif
*/
#endif
