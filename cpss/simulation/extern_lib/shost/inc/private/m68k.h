/*!**************************************************************************
 *! Title: RND - General definitions for Intel 960 processor
 *!--------------------------------------------------------------------------
 *! Filename: k:\inc\private\m68k.h
 *!--------------------------------------------------------------------------
 *! System, Subsystem: RND common software
 *!--------------------------------------------------------------------------
 *! Authors: Iris
 *!--------------------------------------------------------------------------
 *! Latest Update: 28-Jan-97  12:52:56      Creation Date: 27-Aug-91
 *!**************************************************************************
 *!
 *! General Description:
 *!
 *! This file includes RNDs' general definitions for the  Intel 960
 *!     processor.
 *!
 *!**************************************************************************
 *!
 *! History: $LOG$
 *!
 *!**************************************************************************
 *!*/

/*! Include stdio of PSOS */
#ifndef XOS68K
/* #include <prepc.h>  */
#else
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#endif

#define ANSIC
#define BYTESWAP
#define STREMUL

#define   M68K     1
#define   CPU_TYPE M68360

/*! Processor Bytes oredr. */
/* #define LITTLE_ENDIAN_MEM_FORMAT 0 */
/* #define BIG_ENDIAN_MEM_FORMAT    1 */

#define BYTE_ORDER       BIG_ENDIAN_MEM_FORMAT

#define SUPPORT_64_BIT        1


/*! Processor type definitions. */
typedef char                INT_8;
typedef short               INT_16;
typedef int                 INT_32;
typedef long long           INT_64;

typedef unsigned char       UINT_8;
typedef unsigned short      UINT_16;
typedef unsigned long       UINT_32;
typedef unsigned long long  UINT_64;

/* Pointers typedef for arithmetic & logic operations */
typedef INT_32              INT_ARCH_PTR;
typedef UINT_32             UINT_ARCH_PTR;

#ifndef ERR_HNDL
#define word unsigned short int
#endif

#define uint unsigned int
#define byte char

#define NIL  NULL
#define NULL_PTR NULL

typedef enum
    {
        FALSE_E = 0,
        TRUE_E  = 1
    } boolean_ENT;


/* #define         SUCCESS     0 */
#define         BAD_PARAM   1

typedef char boolean_TYP ;
#define TRUE_CNS  1
#define FALSE_CNS 0

/*!  Define MACRO that calculate the offset */
#undef offsetof
#define offsetof(s_type, memb)  ((UINT_32)(char *)&((s_type *)0)->memb)

/** FOR IP **/

typedef void *pointer_TYP ;
#define ushort unsigned short
#define ulong unsigned long
#define uchar unsigned char
#define far

/** END IP **/


