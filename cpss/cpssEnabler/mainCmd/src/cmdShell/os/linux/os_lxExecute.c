/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file os_lxExecute.c
*
* @brief vxworks extended system wrapper layer
*
* this module implements pipe io and standard io redirection routines
*
* @version   10
********************************************************************************
*/

/***** Include files ***************************************************/
/*#include <prestera/os/gtOs.h>*/
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#if 1
/* No dynamic libarrys in uCLinux */
#include <dlfcn.h>
#endif
#include <cmdShell/os/cmdExecute.h>

#if 0
typedef struct
{
    char * name_PTR;
    void (*function_PTR)();
} CMD_ENTRY_TYP;
#define FUNC(NAME) extern void NAME(void)
#define COMMAND(NAME) {#NAME, MAME}
FUNC(cpssInitSystem);
static CMD_ENTRY_TYP commands[] =  {COMMAND(cpssInitSystem)};
#endif

typedef int (*FUNC_PTR)();

/*******************************************************************************
* osShellGetFunctionByName
*
* DESCRIPTION:
*       Lookup executable file symbol table for function
*
* INPUTS:
*       funcName - null terminated command string
*
* OUTPUTS:
*       None
*
* RETURNS:
*       NULL        - if function is not found
*       function pointer
*
* COMMENTS:
*       None
*
*******************************************************************************/
#ifndef STATIC_LINKED_APPDEMO
GT_VOIDFUNCPTR osShellGetFunctionByName
(
    IN  const char* funcName
)
{
    return (GT_VOIDFUNCPTR)dlsym(/* RTLD_DEFAULT */ ((void *) 0), (char*)funcName);
}
#else /* defined(STATIC_LINKED_APPDEMO) */
typedef struct {
    const char *name;
    GT_VOIDFUNCPTR funcPtr;
} SYMTBL_ENTRY_STC;

extern SYMTBL_ENTRY_STC __SymbolTable[];
extern int              __SymbolTableLength;

GT_VOIDFUNCPTR osShellGetFunctionByName
(
    IN  const char* funcName
)
{
    int iFirst = 0;
    int iLast = __SymbolTableLength;
    int iMiddle, iRet;

    if (funcName == NULL)
        return (GT_VOIDFUNCPTR)NULL;

    while (iLast >= iFirst)
    {
        /*iMiddle = (iLast + iFirst) / 2;*/
        iMiddle = iFirst + (iLast - iFirst) / 2;
        if (iMiddle >= __SymbolTableLength || iMiddle < 0)
            return (GT_VOIDFUNCPTR)NULL;

        iRet = strcmp(__SymbolTable[iMiddle].name, funcName);

        if (iRet == 0)
            return __SymbolTable[iMiddle].funcPtr;
        if (iRet > 0)
            iLast = iMiddle - 1;
        else
            iFirst = iMiddle + 1;
    }

    return (GT_VOIDFUNCPTR)NULL;
}
#endif

/**
* @internal osShellParseCmdLine function
* @endinternal
*
* @brief   parse command line
*
* @param[in,out] s                        - null terminated command tring, will be modified
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS osShellParseCmdLine
(
    INOUT   char    *s,
    OUT     char*   *cmd,
    OUT     GT_UINTPTR argv[],
    OUT     GT_32  *argc
)
{
    enum {
        ST_SPACE,
        ST_NUMERIC,
        ST_STRING,
        ST_STRING_Q,
        ST_STRING_HEX0,
        ST_STRING_HEX1
    } state = ST_SPACE;
    char *tok_begin = s;
    char *tok_end = s;
    GT_U32  number = 0;


    *argc = 0;
    /* skip leading spaces */
    while (*s == ' ' || *s == '\t')
        s++;

    /* get command name */
    *cmd = s;
    while (strchr(" \t\n,", *s) == NULL)
        s++;
    if (*s == 0)
    {
        return GT_OK;
    }
    *s++ = 0;

    /* parse parameters */
    for (; *s; s++)
    {
        switch (state)
        {
            case ST_SPACE:
                if (strchr(" \t\n,", *s) != NULL)
                    continue;
                argv[*argc] = 0;
                (*argc)++;
                if (*s == '"')
                {
                    state = ST_STRING;
                    tok_begin = s;
                    tok_end = s;
                    argv[*argc-1] = (GT_UINTPTR)tok_begin;
                }
                else
                {
                    state = ST_NUMERIC;
                    tok_begin = s;
                }
                continue;
            case ST_NUMERIC:
                if (strchr(" \t\n,", *s) != NULL)
                {
                    *s = 0;
                    argv[*argc-1] = strtoul(tok_begin, 0, 0);
                    state = ST_SPACE;
                    continue;
                }
                continue;
            case ST_STRING:
                if (*s == '"')
                {
                    *tok_end = 0;
                    state = ST_SPACE;
                    continue;
                }
                if (*s == '\\')
                {
                    state = ST_STRING_Q;
                    continue;
                }
                *tok_end++ = *s;
                continue;
            case ST_STRING_Q:
                switch (*s)
                {
                    case 'x': state = ST_STRING_HEX0; number = 0; continue;
                    case 'r': *tok_end++ = '\r'; break;
                    case 't': *tok_end++ = '\t'; break;
                    case 'n': *tok_end++ = '\n'; break;
                    default:
                        *tok_end++ = *s;
                }
                state = ST_STRING;
                continue;
            case ST_STRING_HEX0:
                if (strchr("0123456789abcdef", *s) == NULL)
                {
                    *tok_end++ = (char)number;
                    state = ST_STRING;
                    continue;
                }
                number = (*s > '9')
                    ? (*s + 10 - 'a')
                    : (*s - '0');
                state = ST_STRING_HEX1;
                continue;
            case ST_STRING_HEX1:
                if (strchr("0123456789abcdef", *s) != NULL)
                {
                    number *= 16;
                    number += (*s > '9')
                        ? (*s + 10 - 'a')
                        : (*s - '0');
                }
                *tok_end++ = (char)number;
                state = ST_STRING;
                continue;
        }
    }
    if (state == ST_SPACE)
        return GT_OK;

    if (state == ST_NUMERIC)
    {
        argv[*argc-1] = strtoul(tok_begin, 0, 0);
    }
    else
    {
        /* string */
        *tok_end = 0;
    }
    return GT_OK;
}

/**
* @internal osShellExecute function
* @endinternal
*
* @brief   execute command through OS shell
*
* @param[in] command                  - null terminated  string
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS   osShellExecute
(
    IN  char*   command
)
{
    FUNC_PTR func;
    char *copy;
    char  * cmd;
    GT_UINTPTR argv[32];
    GT_32 argc;
    GT_STATUS rc;

    if (!command)
        return GT_OK;

    copy = cmdOsMalloc(2048);
    if(copy == NULL)
        return GT_NO_RESOURCE;

    strncpy(copy, command, 2047);
    copy[2047] = '\0';
    if (osShellParseCmdLine(copy, &cmd, argv, &argc) != GT_OK)
    {
        cmdOsFree(copy);
        return GT_FAIL;
    }

    func = (FUNC_PTR) osShellGetFunctionByName((char*)cmd);
    if (0 == func)
    {
        cmdOsFree(copy);
        return GT_FAIL;
    }
    switch (argc)
    {
        case 0:
            rc = (func)();
            break;
        case 1:
            rc = (func)(argv[0]);
            break;
        case 2:
            rc = (func)(argv[0],argv[1]);
            break;
        case 3:
            rc = (func)(argv[0],argv[1],argv[2]);
            break;
        case 4:
            rc = (func)(argv[0],argv[1],argv[2],argv[3]);
            break;
        case 5:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4]);
            break;
        case 6:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5]);
            break;
        case 7:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6]);
            break;
        case 8:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7]);
            break;
        case 9:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8]);
            break;
        case 10:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8],argv[9]);
            break;
        case 11:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8],argv[9],argv[10]);
            break;
        case 12:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8],argv[9],argv[10],argv[11]);
            break;
        case 13:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8],argv[9],argv[10],argv[11],argv[12]);
            break;
        case 14:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8],argv[9],argv[10],argv[11],argv[12],argv[13]);
            break;
        case 15:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8],argv[9],argv[10],argv[11],argv[12],argv[13],argv[14]);
            break;
        case 16:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8],argv[9],argv[10],argv[11],argv[12],argv[13],argv[14],argv[15]);
            break;
        case 17:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8],argv[9],argv[10],argv[11],argv[12],argv[13],argv[14],argv[15],argv[16]);
            break;
        case 18:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8],argv[9],argv[10],argv[11],argv[12],argv[13],argv[14],argv[15],argv[16],
                    argv[17]);
            break;
        case 19:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8],argv[9],argv[10],argv[11],argv[12],argv[13],argv[14],argv[15],argv[16],
                    argv[17],argv[18]);
            break;
        case 20:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8],argv[9],argv[10],argv[11],argv[12],argv[13],argv[14],argv[15],argv[16],
                    argv[17],argv[18],argv[19]);
            break;
        case 21:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8],argv[9],argv[10],argv[11],argv[12],argv[13],argv[14],argv[15],argv[16],
                    argv[17],argv[18],argv[19],argv[20]);
            break;
        case 22:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8],argv[9],argv[10],argv[11],argv[12],argv[13],argv[14],argv[15],argv[16],
                    argv[17],argv[18],argv[19],argv[20],argv[21]);
            break;
        case 23:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8],argv[9],argv[10],argv[11],argv[12],argv[13],argv[14],argv[15],argv[16],
                    argv[17],argv[18],argv[19],argv[20],argv[21],argv[22]);
            break;
        case 24:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8],argv[9],argv[10],argv[11],argv[12],argv[13],argv[14],argv[15],argv[16],
                    argv[17],argv[18],argv[19],argv[20],argv[21],argv[22],argv[23]);
            break;
        default:
            cmdOsFree(copy);
            return GT_FAIL;
    }

    cmdOsFree(copy);
    cmdOsPrintf("return code is %d\n", rc);
    return rc;
}


