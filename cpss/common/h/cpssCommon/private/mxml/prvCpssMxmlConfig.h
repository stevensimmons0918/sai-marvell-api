/* mxml-config.h.  Generated from config.h.in by configure.  */
/*
 * "$Id: config.h.in 387 2009-04-18 17:05:52Z mike $"
 *
 * Configuration file for Mini-XML, a small XML-like file parsing library.
 *
 * Copyright 2003-2009 by Michael Sweet.
 *
 * Copyright 2011 Marvell International Ltd.
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation,  version 2 of the License.
 * The license can be found at:
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 *
 * 2011 - Adopted to Marvell CPSS LUA CLI - added _mxml_ prefix for all ANSI functions
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/*
 * Include necessary headers...
 */
#ifndef _prvCpssMxmlConfig_h_
#define _prvCpssMxmlConfig_h_


#include <cpss/common/cpssTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#if defined CLI_STANDALONE ||  defined SHARED_MEMORY
#include <stdlib.h>
#endif

typedef GT_SIZE_T mxml_size_t;
#ifndef EOF
#define EOF (-1)
#endif


/*
 * Version number...
 */

#define MXML_VERSION "Mini-XML v2.6"



/*
 * Inline function support...
 */

/* Replaced by GT_INLINE
#define inline
*/


/*
 * Define prototypes for string functions as needed...
 */

extern char *_mxml_strdup(const char *);
extern char *_mxml_strdupf(const char *, ...);
extern char *_mxml_vstrdupf(const char *, va_list);
extern int  _mxml_snprintf(char *, size_t, const char *, ...);
extern int  _mxml_vsnprintf(char *, size_t, const char *, va_list);


#if defined CLI_STANDALONE ||  defined SHARED_MEMORY
#define _mxml_malloc  malloc
#define _mxml_free    free
#define _mxml_realloc realloc
#else
#ifndef USE_CLI_MALLOC
#define _mxml_malloc  cpssOsMalloc
#define _mxml_free    cpssOsFree
#define _mxml_realloc cpssOsRealloc
#else
  /* private memory manager used */
void cliFree(void* ptr);
void* cliMalloc(int size);
void* cliRealloc(void* ptr, int nsize);
#define _mxml_free    cliFree
#define _mxml_malloc  cliMalloc
#define _mxml_realloc cliRealloc
#endif
#endif

void* _mxml_calloc(mxml_size_t nmemb, mxml_size_t size);
#define _mxml_strcmp  cpssOsStrCmp
#define _mxml_strcpy  cpssOsStrCpy
#define _mxml_strlen  cpssOsStrlen
#define _mxml_memcpy  cpssOsMemCpy
#define _mxml_memset  cpssOsMemSet
void* _mxml_memmove(void* dest, void *src, mxml_size_t n);
int _mxml_strncmp(const char* s1, const char *s2, mxml_size_t n);
char* _mxml_strncpy(char* dest, const char *src, mxml_size_t n);
#define _mxml_strchr  cpssOsStrChr
int _mxml_isalnum(char ch);
const char* _mxml_strrchr(const char* s, char ch);
long _mxml_strtol(char *nptr, char **endptr, int base);
#define _mxml_sprintf cpssOsSprintf



/*
 * End of "$Id: config.h.in 387 2009-04-18 17:05:52Z mike $".
 */

#endif
