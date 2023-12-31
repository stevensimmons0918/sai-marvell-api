/****************************************************
* cmdFS_RAM_cpss.inc
*
* DESCRIPTION:
*       The micro layer for mainCmd environment
*       This file just included in cmdFS_RAM.c
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/

#include <cpss/extServices/private/prvCpssBindFunc.h>

#ifndef CMDFS_RAM_TYPES_ONLY

#define CMD_FS_STRCMP(a,b)    cpssOsStrCmp(a,b)
char* CMD_FS_STRDUP(const char *s)
{
   char *d = cpssOsMalloc(cpssOsStrlen(s)+1);
   if (d != NULL)
       cpssOsStrCpy(d,s);
   return d;
}
char* CMD_FS_STRNCPY(char *d, const char *s, int n)   
{
   char *dPtr;
   if (d == NULL || s == NULL)
       return d;
   for(dPtr = d; *s && n > 0; s++, n--)
       *(dPtr++) = *s;
   *dPtr = 0;
   return d;
}
#define CMD_FS_BZERO(p,l)     cpssOsBzero((char*)(p),l)
#define CMD_FS_BCOPY(s,d,l)   cpssOsMemCpy(d,s,(GT_U32)l)
#define CMD_FS_MALLOC(l)      cpssOsMalloc(l)
#define CMD_FS_FREE(l)        cpssOsFree(l)
#define CMD_FS_TIME()         ((unsigned)cpssOsTime())
#define CMD_FS_CONST_CAST(_a,_t) ((_t)((void*)((GT_UINTPTR)_a)))

#endif /* CMDFS_RAM_TYPES_ONLY */
