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
* @file marvell_API_mapping.h
*
* @brief API mappings from POSIX to hws
*
*
* @version   1
********************************************************************************
*/

#ifndef __marvell_API_mapping_H
#define __marvell_API_mapping_H

#define  NOT_USED_IN_HWS    0

/* Filename argument - only needed in host due to FW space limitation */
#ifndef MV_HWS_FREE_RTOS
#ifdef  WIN32
#define __AVAGO_FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1) :(__FILE__))
#else
#define __AVAGO_FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : (__FILE__))
#endif /* WIN32 */
#else
#define __AVAGO_FILENAME__ "NA"
#endif /* defined(MV_HWS_FREE_RTOS) && !defined(__FILENAME__)*/

/* In FW side we do not contain the extra arguments in the log message*/
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    /* In host side we have enought space to log all log arguments */
    EXT void mvHws_aapl_log_printf(Aapl_t *aapl, Aapl_log_type_t log_sel, const char *caller, char* fileName, int line, const char *fmt, ...);
    EXT int  mvHws_aapl_fail(Aapl_t *aapl, const char *caller, char * fileName, int line, const char *fmt, ...);
#else
    #if (defined(MICRO_INIT) && defined(CM3))
        /* In MICRO-INIT CM3 FW side we also do not contain the message argument, only function name*/
        EXT int  mvHws_aapl_fail(Aapl_t *aapl, const char *caller);
    #else
        /* In FW side we do not contain the extra arguments in the log message*/
        EXT void mvHws_aapl_log_printf(Aapl_t *aapl, Aapl_log_type_t log_sel, const char *caller, const char * fmt);
        EXT int  mvHws_aapl_fail(Aapl_t *aapl, const char *caller, const char *fmt);
    #endif
#endif

#ifndef MV_HWS_FREE_RTOS
    /* In host side we have enought space to log all log arguments */
    #define aapl_log_printf(aapl, type, function, line, arg...)    mvHws_aapl_log_printf(aapl, type, function, __AVAGO_FILENAME__, line, arg)
    #define aapl_fail(aapl, function, line, arg...)                mvHws_aapl_fail(aapl, function, __AVAGO_FILENAME__, line, arg)
#else
    #if (defined(MICRO_INIT) && defined(CM3))
        /* In MICRO-INIT CM3 FW side we also do not contain the message argument, only function name */
        #define aapl_log_printf(aapl, type, function, line, msg, arg...)
        #define aapl_fail(aapl, function, line, msg, arg...)                mvHws_aapl_fail(aapl, function)
    #else
        /* In FW side we do not contain the extra arguments in the log message */
        #define aapl_log_printf(aapl, type, function, line, msg, arg...)    mvHws_aapl_log_printf(aapl, type, function, msg)
        #define aapl_fail(aapl, function, line, msg, arg...)                mvHws_aapl_fail(aapl, function, msg)
    #endif /* MICRO_INIT */
#endif /* MV_HWS_FREE_RTOS */

#if defined MV_HWS_FREE_RTOS
    #undef AAPL_FREE
    #undef AAPL_MEMSET
    #undef AAPL_MALLOC
    #undef AAPL_REALLOC
    #undef AAPL_ALLOW_I2C
    #undef AAPL_ALLOW_AACS
    #undef AAPL_ENABLE_DIAG
    #undef AAPL_LOG_TIME_STAMPS
    #undef AAPL_ENABLE_AACS_SERVER
    #undef AAPL_ENABLE_EYE_MEASUREMENT

    #define AAPL_ALLOW_I2C                  0
    #define AAPL_ALLOW_AACS                 0
    #define AAPL_ENABLE_AACS_SERVER         0
    #define AAPL_ENABLE_DIAG                0
    #define AAPL_ENABLE_EYE_MEASUREMENT     0

    extern void* pvPortMalloc(size_t xWantedSize);
    extern void  vPortFree(void *pv);

    #define AAPL_MEMSET             memset
    #define AAPL_MALLOC(sz)         pvPortMalloc(sz)  /* AAPL uses this for malloc  */
    #define AAPL_REALLOC(ptr,sz)    realloc(ptr,sz)
    #define AAPL_FREE(ptr)          vPortFree(ptr)    /* AAPL uses this for free    */

    /* macro for GCC > 7.3.0 for acknowledging switch-case fallthrough */
    #if defined(__GNUC__) && __GNUC__ >= 7
    #define GT_ATTR_FALLTHROUGH  __attribute__ ((fallthrough))
    #else
    #define GT_ATTR_FALLTHROUGH  ((void)0)
    #endif


    #define GT_UNUSED_PARAM(x) (void)x

#else /* !MV_HWS_FREE_RTOS */
    #include <cpss/common/labServices/port/gop/common/os/hwsEnv.h>

    /* redefine alloc macroces */
    #undef  AAPL_MALLOC
    #undef  AAPL_REALLOC
    #undef  AAPL_FREE
    #undef  AAPL_MEMSET
    #define AAPL_MALLOC(sz)             hwsMalloc(sz)         /**< AAPL uses this for malloc  */
    #define AAPL_REALLOC(ptr,sz)        hwsRealloc(ptr,sz)    /**< AAPL uses this for realloc */
    #define AAPL_FREE(ptr)              hwsFree(ptr)          /**< AAPL uses this for free    */
    #define AAPL_MEMSET(ptr,sym,sz)     hwsMemSet(ptr,sym,sz) /**< AAPL uses this for memset  */

    /* redefine stdio APIS */
    #define sprintf                     hwsSprintf
    #define snprintf                    hwsSnprintf
    #define vsprintf                    hwsVsprintf
    #define vsnprintf                   hwsVsnprintf

#ifdef CPSS_BLOB
    /* redefine ctype macros */
    #undef tolower
    #undef isspace
    #undef isdigit
    #undef isxdigit

    #define tolower(_c)  (((_c)>='A' && (_c)<='Z')?((_c)+('a'-'A')):(_c))
    #define isspace(_c)  (((_c)==' '||(_c)=='\t'||(_c)=='\r'||(_c)=='\n')?1:0)
    #define isdigit(_c)  (((_c)>='0' && (_c)<='9')?1:0)
    #define isxdigit(_c)  ((((_c)>='0' && (_c)<='9')||((_c)>='a' && (_c)<='f')||((_c)>='A' && (_c)<='F'))?1:0)

    #define MS_SLEEP    hwsDelayMs

    /* redefine str* APIs */
    #undef strcpy
    #undef strlen
    #undef strchr
    #undef strncmp
    #undef strrchr
    #define strcpy      hwsStrCpy
    #define strlen      hwsStrLen
    #define strchr      hwsStrChr
    #define strncmp     hwsStrNCmp
    #define strrchr     hwsStrrChr

    long hws_strtol(const char *nptr, char **endptr, int base);
    unsigned long hws_strtoul(const char *nptr, char **endptr, int base);
    #define strtol      hws_strtol
    #define strtoul     hws_strtoul

    void *hws_memmove(void *dest, void *src, unsigned len);
    #define memmove     hws_memmove

#endif /* CPSS_BLOB */

#endif /* defined MV_HWS_FREE_RTOS */

#endif /* __marvell_API_mapping_H */

