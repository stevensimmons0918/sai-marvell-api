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
* @file osShellExec.c
*
* @brief win32 microsoft imageHelp library basedf shell
*
* @version   12
********************************************************************************
*/

/***** Include files ***************************************************/

/* WA to avoid next warning :
   due to include to : #include <windows.h>
    c:\program files\microsoft visual studio\vc98\include\rpcasync.h(45) :
    warning C4115: '_RPC_ASYNC_STATE' : named type definition in parentheses
*/
struct _RPC_ASYNC_STATE;

#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/os/cmdExecute.h>

#include <windows.h>
#include <gtOs/gtGenTypes.h>
#include <imagehlp.h>

typedef GT_STATUS (*SHELL_FUNC)(char* command);
/* global, so must be outside #ifdef IMAGE_HELP_SHELL */
SHELL_FUNC osSlellFunction = NULL;

/* for microsoft compiler only */
#if defined(IMAGE_HELP_SHELL) || defined(APPLICATION_SYMBOL_TABLE)

int osShellPrintf(const char* format, ...);


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

typedef int (*FUNCTION16)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*);
typedef int (*FUNCTION17)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*);
typedef int (*FUNCTION18)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*);
typedef int (*FUNCTION19)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*);
typedef int (*FUNCTION20)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*);
typedef int (*FUNCTION21)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*);
typedef int (*FUNCTION22)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*);
typedef int (*FUNCTION23)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*);
typedef int (*FUNCTION24)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*);
typedef int (*FUNCTION25)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*);
typedef int (*FUNCTION26)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*);
typedef int (*FUNCTION27)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*);
typedef int (*FUNCTION28)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*);
typedef int (*FUNCTION29)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*);
typedef int (*FUNCTION30)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*);
typedef int (*FUNCTION31)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*);
typedef int (*FUNCTION32)(
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*,
    void*, void*, void*, void*, void*, void*, void*, void*);


static int callFunction
(
    IN void* funcAddr,
    IN GT_INTPTR* paramArray,
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
        case 16:
            return ((FUNCTION16)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15]);
        case 17:
            return ((FUNCTION17)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15], (void*)paramArray[16]);
        case 18:
            return ((FUNCTION18)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15], (void*)paramArray[16],
                    (void*)paramArray[17]);
        case 19:
            return ((FUNCTION19)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15], (void*)paramArray[16],
                    (void*)paramArray[17], (void*)paramArray[18]);
        case 20:
            return ((FUNCTION20)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15], (void*)paramArray[16],
                    (void*)paramArray[17], (void*)paramArray[18],
                    (void*)paramArray[19]);
        case 21:
            return ((FUNCTION21)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15], (void*)paramArray[16],
                    (void*)paramArray[17], (void*)paramArray[18],
                    (void*)paramArray[19], (void*)paramArray[20]);
        case 22:
            return ((FUNCTION22)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15], (void*)paramArray[16],
                    (void*)paramArray[17], (void*)paramArray[18],
                    (void*)paramArray[19], (void*)paramArray[20],
                    (void*)paramArray[21]);
        case 23:
            return ((FUNCTION23)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15], (void*)paramArray[16],
                    (void*)paramArray[17], (void*)paramArray[18],
                    (void*)paramArray[19], (void*)paramArray[20],
                    (void*)paramArray[21], (void*)paramArray[22]);
        case 24:
            return ((FUNCTION24)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15], (void*)paramArray[16],
                    (void*)paramArray[17], (void*)paramArray[18],
                    (void*)paramArray[19], (void*)paramArray[20],
                    (void*)paramArray[21], (void*)paramArray[22],
                    (void*)paramArray[23]);
        case 25:
            return ((FUNCTION25)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15], (void*)paramArray[16],
                    (void*)paramArray[17], (void*)paramArray[18],
                    (void*)paramArray[19], (void*)paramArray[20],
                    (void*)paramArray[21], (void*)paramArray[22],
                    (void*)paramArray[23], (void*)paramArray[24]);
        case 26:
            return ((FUNCTION26)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15], (void*)paramArray[16],
                    (void*)paramArray[17], (void*)paramArray[18],
                    (void*)paramArray[19], (void*)paramArray[20],
                    (void*)paramArray[21], (void*)paramArray[22],
                    (void*)paramArray[23], (void*)paramArray[24],
                    (void*)paramArray[25]);
        case 27:
            return ((FUNCTION27)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15], (void*)paramArray[16],
                    (void*)paramArray[17], (void*)paramArray[18],
                    (void*)paramArray[19], (void*)paramArray[20],
                    (void*)paramArray[21], (void*)paramArray[22],
                    (void*)paramArray[23], (void*)paramArray[24],
                    (void*)paramArray[25], (void*)paramArray[26]);
        case 28:
            return ((FUNCTION28)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15], (void*)paramArray[16],
                    (void*)paramArray[17], (void*)paramArray[18],
                    (void*)paramArray[19], (void*)paramArray[20],
                    (void*)paramArray[21], (void*)paramArray[22],
                    (void*)paramArray[23], (void*)paramArray[24],
                    (void*)paramArray[25], (void*)paramArray[26],
                    (void*)paramArray[27]);
        case 29:
            return ((FUNCTION29)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15], (void*)paramArray[16],
                    (void*)paramArray[17], (void*)paramArray[18],
                    (void*)paramArray[19], (void*)paramArray[20],
                    (void*)paramArray[21], (void*)paramArray[22],
                    (void*)paramArray[23], (void*)paramArray[24],
                    (void*)paramArray[25], (void*)paramArray[26],
                    (void*)paramArray[27], (void*)paramArray[28]);
        case 30:
            return ((FUNCTION30)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15], (void*)paramArray[16],
                    (void*)paramArray[17], (void*)paramArray[18],
                    (void*)paramArray[19], (void*)paramArray[20],
                    (void*)paramArray[21], (void*)paramArray[22],
                    (void*)paramArray[23], (void*)paramArray[24],
                    (void*)paramArray[25], (void*)paramArray[26],
                    (void*)paramArray[27], (void*)paramArray[28],
                    (void*)paramArray[29]);
        case 31:
            return ((FUNCTION31)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15], (void*)paramArray[16],
                    (void*)paramArray[17], (void*)paramArray[18],
                    (void*)paramArray[19], (void*)paramArray[20],
                    (void*)paramArray[21], (void*)paramArray[22],
                    (void*)paramArray[23], (void*)paramArray[24],
                    (void*)paramArray[25], (void*)paramArray[26],
                    (void*)paramArray[27], (void*)paramArray[28],
                    (void*)paramArray[29], (void*)paramArray[30]);
        case 32:
            return ((FUNCTION32)funcAddr)((void*)paramArray[0],
                    (void*)paramArray[1], (void*)paramArray[2],
                    (void*)paramArray[3], (void*)paramArray[4],
                    (void*)paramArray[5], (void*)paramArray[6],
                    (void*)paramArray[7], (void*)paramArray[8],
                    (void*)paramArray[9], (void*)paramArray[10],
                    (void*)paramArray[11], (void*)paramArray[12],
                    (void*)paramArray[13], (void*)paramArray[14],
                    (void*)paramArray[15], (void*)paramArray[16],
                    (void*)paramArray[17], (void*)paramArray[18],
                    (void*)paramArray[19], (void*)paramArray[20],
                    (void*)paramArray[21], (void*)paramArray[22],
                    (void*)paramArray[23], (void*)paramArray[24],
                    (void*)paramArray[25], (void*)paramArray[26],
                    (void*)paramArray[27], (void*)paramArray[28],
                    (void*)paramArray[29], (void*)paramArray[30],
                    (void*)paramArray[31]);
        default: break;
    }
    return -1;
}

/* returns amount of parameters, -1 on parse error */
static int parseCmd
(
    IN  char    *command,
    IN  GT_U32  parmArraySize, /* in words */
    OUT char    *cmdName,
    IN  GT_U32  cmdNameSize, /* in chars */
    OUT GT_INTPTR  *parmArray
)
{
    GT_U8* p = (GT_U8*)command;
    GT_U8* arrEnd = (GT_U8*)parmArray + (parmArraySize * sizeof(GT_INTPTR));
    int i;
    int paramAmount;
    GT_BOOL done;
    GT_BOOL minusFound;
    GT_BOOL isAddress;

    GT_INTPTR val = 0; /* to prevent compiler warning */
    paramAmount = 0;

    for (i = 0; (*p > ' '); p++, i++)
    {
        if(i >= (cmdNameSize - 1))
        {
            /* allow p to continue parsing ... */
        }
        else
        {
            cmdName[i] = *p;
        }
    }

    if(i >= cmdNameSize)
    {
        /* command too long */
        cmdName[cmdNameSize - 1] = 0;
    }
    else
    {
        cmdName[i] = 0;
    }

    while (1)
    {
        /* skip blanks and tabs and no more then one ',' */
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

        isAddress  = GT_FALSE;
        done       = GT_FALSE;
        minusFound = GT_FALSE;

        /* case of string, 0x22 is a Quote code */
        if ((! done) && (*p == 0x22))
        {
            done = GT_TRUE;
            p++;
            /* search for the second quote or end of string */
            for (i = 0; ((p[i] > 0) && (p[i] != 0x22)); i++) {};

            arrEnd -= (i + 1);
            strncpy(arrEnd, p, i);
            arrEnd[i] = 0;
            val = (GT_INTPTR)arrEnd;
            /* new position - after closing quote or on '\0' */
            p += (p[i] == 0) ? i : (i + 1);
        }

        if ((! done) && (*p == '@'))
        {
            /* the next number is address in execute_scratch_pad */
            isAddress = GT_TRUE;
            p ++;
        }

        if ((! done) && (! isAddress) && (*p == '-'))
        {
            /* the next number is address in execute_scratch_pad */
            minusFound = GT_TRUE;
            p ++;
        }

        /* case of hex number */
        if ((! done) && (p[0] == '0') && ((p[1] == 'x') || (p[1] == 'X')))
        {
            done = GT_TRUE;
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
            done = GT_TRUE;

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
            val = (GT_INTPTR)&(execute_scratch_pad[val]);
        }

        if (minusFound)
        {
            val = (0 - val);
        }

        parmArray[paramAmount] = val;
        paramAmount ++;
    }

    /* dummy, never reached */
    return 0;
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

    if ((index+(amount-1)) >= EXECUTE_SCRATCH_PAD_SIZE)
    {
        /* error */
        return -1;
    }

    for (i = 0; (i < amount); i++)
    {
        osShellPrintf(" %08X ", execute_scratch_pad[index + i]);
        if (((i + 1) % 8) == 0)
        {
            osShellPrintf("\n");
        }
    }
    osShellPrintf("\n");

    return 0;
}

static GT_STATUS imageHelpShell
(
    IN  char*   command
)
{
    char funcName[128];
    GT_INTPTR paramArray[256]; /* 2048 bytes for strings */
    int  paramAmount;
    GT_VOIDFUNCPTR  funcPtr;
    GT_STATUS rc;

    if ((command == NULL) || (command[0] == 0))
    {
        return GT_OK;
    }

    paramAmount = parseCmd(
        command,
        256,/*num elements in paramArray*/
        funcName,
        128,/*num chars in funcName*/
        paramArray);
    if (0 > paramAmount)
    {
        return GT_FAIL;
    }

    funcPtr = osShellGetFunctionByName(funcName);
    if (!funcPtr)
    {
        return GT_FAIL;
    }

    rc = callFunction(
        (void*)funcPtr, paramArray, paramAmount);

    osShellPrintf("return code is %d\r\n", rc);

    return GT_OK;
}

#endif /* defined(IMAGE_HELP_SHELL) || defined(APPLICATION_SYMBOL_TABLE) */

#if defined(IMAGE_HELP_SHELL) && !defined(APPLICATION_SYMBOL_TABLE)
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
GT_VOIDFUNCPTR osShellGetFunctionByName
(
    IN  const char* funcName
)
{
    /* prototype workaround */
    typedef BOOL (__stdcall *SYM_GET_SYM_BY_NAME_PTR)
    (
        HANDLE hProcess,
        const char* Name,
        PIMAGEHLP_SYMBOL Symbol
    );
    SYM_GET_SYM_BY_NAME_PTR _SymGetSymFromName =
            (SYM_GET_SYM_BY_NAME_PTR)SymGetSymFromName;

    IMAGEHLP_SYMBOL symbol;

    symbol.SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
    symbol.MaxNameLength = 0;

    if (! _SymGetSymFromName(
        GetCurrentProcess(), funcName, &symbol))
    {
        return NULL;
    }
    return (GT_VOIDFUNCPTR)symbol.Address;
}

void imageHelpShellInit()
{
    char appExePath[1024];              /* Application executable path */
    char * cmdLinePtr = &appExePath[0]; /* Pointer to application path */
    int lastSlashIndex = 0;             /* Last slash in application path */

    /* Get full application executable path including application name */
    GetModuleFileName(NULL, cmdLinePtr, 1024);

    /* Search for the last slash symbol */
    cmdLinePtr = strchr(cmdLinePtr, '\\');
    while(cmdLinePtr != NULL)
    {
        lastSlashIndex = cmdLinePtr - &appExePath[0];
        cmdLinePtr = strchr(++cmdLinePtr, '\\');
    }

    /* Remove executable name from string */
    appExePath[lastSlashIndex] = 0;

    /* init imageHelp library */
    SymInitialize(
        GetCurrentProcess(),
        appExePath /*UserSearchPath*/, TRUE /*fInvadeProcess*/);

    /* bind osCmdShell to this shell  */
    osSlellFunction = imageHelpShell;
}


#endif /* defined(IMAGE_HELP_SHELL) && !defined(APPLICATION_SYMBOL_TABLE) */

#ifdef APPLICATION_SYMBOL_TABLE

#include <string.h>
#include <stdlib.h>

typedef struct {
    const char *name;
    GT_VOIDFUNCPTR funcPtr;
} SYMTBL_ENTRY_STC;

extern SYMTBL_ENTRY_STC __SymbolTable[];
static int __SymbolTableLength = 0;


/**
* @internal qsortRecordCompareFunc function
* @endinternal
*
* @brief   Compares two CMD_COMMAND records by command name
*         Required by qsort()
* @param[in] a                        - pointer to record
* @param[in] b                        - pointer to record
*                                       integer less than, equal to, or greater than zero if
*                                       a is less than, equal to, or greater than b
*
* @note none
*
*/
static int qsortRecordCompareFunc(
        const void* a,
        const void* b
)
{
    return strcmp(
            ((const SYMTBL_ENTRY_STC*)a)->name,
            ((const SYMTBL_ENTRY_STC*)b)->name);
}

void imageHelpShellInit()
{
    /* calculate length of symbol table */
    while (__SymbolTable[__SymbolTableLength].name)
        __SymbolTableLength++;
    /* sort table to speedup search */
    qsort(__SymbolTable, __SymbolTableLength, sizeof(__SymbolTable[0]), qsortRecordCompareFunc);

    /* bind osCmdShell to this shell  */
    osSlellFunction = imageHelpShell;
}

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

#endif /* APPLICATION_SYMBOL_TABLE */


