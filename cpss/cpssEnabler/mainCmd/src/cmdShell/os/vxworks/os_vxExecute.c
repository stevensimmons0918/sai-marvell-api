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
* @file os_vxExecute.c
*
* @brief vxworks extended system wrapper layer
*
* this module implements pipe io and standard io redirection routines
*
* @version   8
********************************************************************************
*/

/***** Include files ***************************************************/
/*#include <prestera/os/gtOs.h>*/

#include <cmdShell/os/cmdExecute.h>

#include <ioLib.h>
#include <ptyDrv.h>
#include <version.h>
#include <sysSymTbl.h> 
#include <symLib.h> 

/* detect vxWorks version */
#ifdef _WRS_VXWORKS_MAJOR
#if _WRS_VXWORKS_MAJOR >= 5
/* generate "execute" fuinction for vxWorks versions that don't support it */
#define IMPL_VX_WORKS_EXECUTE_FUNC
#endif /*_WRS_VXWORKS_MAJOR >= 5*/
#endif /*_WRS_VXWORKS_MAJOR*/

#include "shellLib.h"

#ifdef IMPL_VX_WORKS_EXECUTE_FUNC

/*  patch */

#include <string.h> 

#define EXECUTE_SCRATCH_PAD_SIZE 128

static GT_U32 execute_scratch_pad[EXECUTE_SCRATCH_PAD_SIZE];

typedef int (*FUNCTION0)(void);
typedef int (*FUNCTION1)(void*);
typedef int (*FUNCTION2)(void*, void*);
typedef int (*FUNCTION3)(void*, void*, void*);
typedef int (*FUNCTION4)(void*, void*, void*, void*);
typedef int (*FUNCTION5)(void*, void*, void*, void*, void*);
typedef int (*FUNCTION6)(void*, void*, void*, void*, void*, void*);
typedef int (*FUNCTION7)(void*, void*, void*, void*, void*, void*, void*);
typedef int (*FUNCTION8)(void*, void*, void*, void*, void*, void*, void*,
    void*);
typedef int (*FUNCTION9)(void*, void*, void*, void*, void*, void*, void*,
    void*, void*);
typedef int (*FUNCTION10)(void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*);
typedef int (*FUNCTION11)(void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*);
typedef int (*FUNCTION12)(void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*);
typedef int (*FUNCTION13)(void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*);
typedef int (*FUNCTION14)(void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*);
typedef int (*FUNCTION15)(void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*);


typedef GT_STATUS (*SHELL_FUNC)(char* command);
/* global */

static int callFunction
(
    IN void* funcAddr,
    IN GT_U32* paramArray,
    IN GT_U32  paramAmount
)
{
    switch(paramAmount)
    {
        case 0:
            return ((FUNCTION0)funcAddr)();
        case 1:
            return ((FUNCTION1)funcAddr)((void*)paramArray[0]);
        case 2:
            return ((FUNCTION2)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1]);
        case 3:
            return ((FUNCTION3)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2]);
        case 4:
            return ((FUNCTION4)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3]);
        case 5:
            return ((FUNCTION5)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4]);
        case 6:
            return ((FUNCTION6)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5]);
        case 7:
            return ((FUNCTION7)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6]);
        case 8:
            return ((FUNCTION8)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7]);
        case 9:
            return ((FUNCTION9)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8]);
        case 10:
            return ((FUNCTION10)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9]);
        case 11:
            return ((FUNCTION11)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10]);
        case 12:
            return ((FUNCTION12)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11]);
        case 13:
            return ((FUNCTION13)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12]);
        case 14:
            return ((FUNCTION14)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13]);
        case 15:
            return ((FUNCTION15)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14]);
        default: break;
    }
    return -1;
}

/* returns amount of parameters, -1 on parse error */
static int parceCmd
(
    IN  char    *command,
    IN  GT_U32  parmArraySize, /* in words */
    OUT char    *cmdName,
    OUT GT_U32  *parmArray
)
{
    GT_U8* p = (GT_U8*)command;
    GT_U8* arrEnd = (GT_U8*)parmArray + (parmArraySize * sizeof(GT_U32));
    int i, val;
    int paramAmount;
    GT_BOOL done;
    GT_BOOL isAddress;

    val = 0; /* to prevent compiler warning */
    paramAmount = 0;

    for (i = 0; (*p > ' '); p++, i++)
    {
        cmdName[i] = *p;
    }
    cmdName[i] = 0;

    while (1)
    {
        /* skip blanks and tabs and no more than one ',' */
        for (; ((*p > 0) && (*p <= ' ')); p++) {};
        if (*p == 0)
        {
            return paramAmount;
        }
        if ((*p == ',') && (paramAmount != 0))
        {
            p++;
            for (; ((*p > 0) && (*p <= ' ')); p++) {};
            if (*p == 0)
            {
                return paramAmount;
            }
        }

        isAddress = GT_FALSE;
        done = FALSE;

        /* case of string, 0x22 is a Quote code */
        if ((! done) && (*p == 0x22))
        {
            done = TRUE;
            p++;
            /* search for the second quote or end of string */
            for (i = 0; ((p[i] > 0) && (p[i] != 0x22)); i++) {};

            arrEnd -= (i + 1);
            strncpy(arrEnd, p, i);
            arrEnd[i] = 0;
            val = (GT_U32)arrEnd;
            /* new position - after closing quote or on '\0' */
            p += (p[i] == 0) ? i : (i + 1);
        }

        if ((! done) && (*p == '@'))
        {
            /* the next number is address in execute_scratch_pad */
            isAddress = GT_TRUE;
            p ++;
        }

        /* case of hex number */
        if ((! done) && (p[0] == '0') && ((p[1] == 'x') || (p[1] == 'X')))
        {
            done = TRUE;
            val = 0;
            p += 2;

            for (; ((*p > ' ') && (*p != ',')); p++)
            {
                val *= 16;

                if ((*p >= '0') && (*p <= '9'))
                {
                    val += *p - '0';
                    continue;
                }

                if ((*p >= 'a') && (*p <= 'f'))
                {
                    val += *p - 'a' + 10;
                    continue;
                }

                if ((*p >= 'A') && (*p <= 'F'))
                {
                    val += *p - 'A' + 10;
                    continue;
                }

                return -1;
            }
        }

        /* decimal number */
        if (! done)
        {
            val = 0;
            done = TRUE;

            for (; ((*p > ' ') && (*p != ',')); p++)
            {
                val *= 10;

                if ((*p >= '0') && (*p <= '9'))
                {
                    val += *p - '0';
                    continue;
                }

                return -1;
            }
        }

        if (isAddress != GT_FALSE)
        {
            if (val >= EXECUTE_SCRATCH_PAD_SIZE)
            {
                /* error */
                return -1;
            }
            val = (int)&(execute_scratch_pad[val]);
        }

        parmArray[paramAmount] = val;
        paramAmount ++;
    }
}

int execute_scratch_pad_clear(void)
{
    memset(execute_scratch_pad, 0, sizeof(execute_scratch_pad));
    return 0;
}

int execute_scratch_pad_set(int index, int value)
{
    if (index >= EXECUTE_SCRATCH_PAD_SIZE)
    {
        /* error */
        return -1;
    }
    execute_scratch_pad[index] = value;
    return 0;
}

int execute_scratch_pad_dump(int index, int amount)
{
    int i;
    if (index >= EXECUTE_SCRATCH_PAD_SIZE)
    {
        /* error */
        return -1;
    }

    for (i = 0; (i < amount); i++)
    {
        if ((index + i) >= EXECUTE_SCRATCH_PAD_SIZE)
        {
            /* error */
            break;
        }
        printf(" %08X ", execute_scratch_pad[index + i]);
        if (((i + 1) % 8) == 0)
        {
            printf("\n");
        }
    }
    printf("\n");

    return 0;
}

int execute (char* command)
{
    GT_STATUS rc;
    char funcName[128];
    GT_U32 paramArray[128]; /* 512 bytes for strings */
    int  paramAmount;
    void* funcAddr;

    if ((command == NULL) || (command[0] == 0))
    {
        return GT_OK;
    }

    paramAmount = parceCmd(
        command,
        (sizeof(paramArray) / sizeof(paramArray[0])),
        funcName, paramArray);
    if (0 > paramAmount)
    {
        printf("Parce command error\n");
        return GT_FAIL;
    }

    funcAddr = (void*) osShellGetFunctionByName(funcName);
    if (funcAddr == NULL)
    {
        printf("Command not found in system table\n");
        return GT_FAIL;
    }

    rc = callFunction(funcAddr, paramArray, paramAmount);
    printf("function called, rc: 0x%X\n", rc);

    return rc;
}

#endif /*IMPL_VX_WORKS_EXECUTE_FUNC*/

/**
* @internal osShellGetFunctionByName function
* @endinternal
*
* @brief   Lookup executable file symbol table for function
*
* @param[in] funcName                 - null terminated command string
*
* @retval NULL                     - if function is not found
*                                       function pointer
*/
GT_VOIDFUNCPTR osShellGetFunctionByName
(
    IN  const char* funcName
)
{
    GT_STATUS rc;
    char* funcAddr;
    SYM_TYPE funcType;

    rc = symFindByName(
        sysSymTbl,    /* ID of symbol table to look in */
        (char*)funcName,     /* symbol name to look for */
        &funcAddr,    /* where to return symbol value */
        &funcType     /* where to return symbol type */);
    if (rc != GT_OK)
    {
        return NULL;
    }

    return (GT_VOIDFUNCPTR)funcAddr;
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
    if (execute(command) != 0)
        return GT_FAIL;
    else
        return GT_OK;

}



