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
* @file cmdEngine.c
*
* @brief commander database engine code, responsible for parsing single
* command and execution of appropriate api function declared in
* cmdBase.h header file.
*
* @version   15
********************************************************************************
*/

/***** Include files ***************************************************/

#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/os/cmdExecute.h>
#include <cmdShell/shell/cmdParser.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/shell/cmdEngine.h>
#include <gtUtil/gtTimeMeasure.h>
#include <cmdShell/common/cmdWrapUtils.h>

static GT_CHAR cmdDupBuffer[CMD_MAX_BUFFER];
static GT_U32 cmdDupBufferOffset = 0;

/* commands input argument buffer */
static GT_UINTPTR inputBuffer[CMD_MAX_ARGS];

/* commands fields input argument buffer */
static GT_UINTPTR fieldBuffer[CMD_MAX_FIELDS];

/* commands output argument buffer */
static GT_CHAR outputBuffer[CMD_MAX_BUFFER];

/* Number of input fields */
static GT_U32        numFields = 0;

/* function (command) to invoke */
static cmdWrapperFuncPtr   wrapperRef;

/* Mutual exclusion semaphore to protect
 *    inputBuffer, fieldBuffer, outputBuffer, wrapperRef, etc
 * Used by cmdEventRun(), cmdWrapperRun()
 */
static GT_SEM  gRunMtx;

/* The command task wait on this semaphore */
static GT_SEM  gSemCmd;

/* The processing task wait on this semaphore */
static GT_SEM  gSemProc;

/* Processing task ID */
static GT_U32  gProcTaskId;

/* Function invokation return code */
static GT_STATUS gRc;

/* Is it a shell command*/
static GT_BOOL isShellCmd;

/* Shell input buffer pointer */
static GT_CHAR *gShellInBufferPtr;

/* If the timeout has expired */
static GT_U32 timeOutExpired = 0;

#ifdef ASIC_SIMULATION_ENV_FORBIDDEN
    /* Wait maximum 300 seconds (10 times more than 'regular') */
    /* as the ASIM environment works very SLOW (on slow Linux laptops - that are not SERVERs) */
    /* see JIRA : https://jira.cavium.com/browse/IPBUSW-8310 */
    /* JIRA : IPBUSW-8310 : [ASIM-106xx switch] Command timeout expired with cpssInitSystem */
    #define CMD_PROC_TIMOUT 300000
#else
    /* Wait maximum 30 seconds */
    #define CMD_PROC_TIMOUT 30000
#endif

/* Wait maximum 3 minutes */
static GT_U32 cmdSysTimeout = (CMD_PROC_TIMOUT * 6);

/* command time measure enable*/
static GT_BOOL prvCmdTimeMeasureEnable = GT_FALSE;

#define CMD_EXE_TASK_PRIO (CMD_THREAD_PRIO - 1)

/* The shell message */
/* the prefix @@@0!!!### needed to notify GUI that command ended */
#define SHELL_CMD_MSG "@@@0!!!### shell command executed\r\n"
#define SHELL_CMD_MSG_LENGTH sizeof(SHELL_CMD_MSG)
#if 0
#define DEBUG_ENGINE_STREAM
#endif

/**
* @internal wrCmdTimeMeasureEnableSet function
* @endinternal
*
* @brief   Set Command Time Measuring enable.
*/
static CMD_STATUS wrCmdTimeMeasureEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    prvCmdTimeMeasureEnable = (GT_BOOL)inArgs[0];

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrGtTimeExactDumpNodes function
* @endinternal
*
* @brief   Dump Exact Time Nodes.
*/
static CMD_STATUS wrGtTimeExactDumpNodes
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    gtTimeExactDumpNodes();

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrGtTimeExactResetNodes function
* @endinternal
*
* @brief   Set Command Time Measuring enable.
*/
static CMD_STATUS wrGtTimeExactResetNodes
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    gtTimeExactResetNodes();

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

static CMD_COMMAND dbCommands[] =
{
    {"cmdTimeMeasureEnableSet",
     &wrCmdTimeMeasureEnableSet,
     1, 0},
    {"gtTimeExactDumpNodes",
     &wrGtTimeExactDumpNodes,
     0, 0},
    {"gtTimeExactResetNodes",
     &wrGtTimeExactResetNodes,
     0, 0}
};

#define PRV_NUM_OF_COMMANDS_CNS (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/*******************************************************************************
* cmdProcTask
*
* DESCRIPTION:
*       Command's processor.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK - on success
*       GT_FAIL - on fialure
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS __TASKCONV cmdProcTask(GT_VOID *params)
{

    GT_STATUS rc; /* Return code */

    GT_UNUSED_PARAM(params);
/* for private versions (for ex. automatic cpssInitSystem before shell) */
#ifdef PRE_SHELL_FUNCTION
    {
        /* declaration */
        GT_VOID PRE_SHELL_FUNCTION(GT_VOID);

        PRE_SHELL_FUNCTION();
    }
#endif /*PRE_SHELL_FUNCTION*/

    while (GT_TRUE)
    {
        /* Wait for incomming command to invoke */
        rc = cmdOsSigSemWait(gSemProc, CPSS_OS_SEM_WAIT_FOREVER_CNS);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (isShellCmd == GT_TRUE)
        {
            if (prvCmdTimeMeasureEnable != GT_FALSE)
            {
                GT_DEFINE_TIME_EXACT_NODE("SHELL_NOT_GALTIS_COMMAND_START");
            }
            gRc = osShellExecute(gShellInBufferPtr);
            if (prvCmdTimeMeasureEnable != GT_FALSE)
            {
                GT_DEFINE_TIME_EXACT_NODE("SHELL_NOT_GALTIS_COMMAND_END");
            }
        }
        else
        {
    #ifdef DEBUG_ENGINE_STREAM
            cmdOsPrintf("\nCalled function\n");
    #endif
            if (prvCmdTimeMeasureEnable != GT_FALSE)
            {
                GT_DEFINE_TIME_EXACT_NODE("SHELL_GALTIS_COMMAND_START");
            }
            gRc = wrapperRef(inputBuffer, fieldBuffer, numFields, (GT_8*)outputBuffer);
            if (prvCmdTimeMeasureEnable != GT_FALSE)
            {
                GT_DEFINE_TIME_EXACT_NODE("SHELL_GALTIS_COMMAND_END");
            }
        }

        /* Signal command that invokation comleted */
        rc = cmdOsSigSemSignal(gSemCmd);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
}

/**
* @internal cmdEngineInit function
* @endinternal
*
* @brief   initializes engine. Spawn new task for command execution.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fialure
*/
GT_STATUS cmdEngineInit(GT_VOID)
{

    GT_STATUS rc; /* Return code*/
    GT_U32    stackSize; /* task stack size */

    /* add Galtis commands */
    rc = cmdInitLibrary(dbCommands, PRV_NUM_OF_COMMANDS_CNS);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Allocate EventRun mutex. Use semaphore because it locks resources
     * for long period
     */
    rc = cmdOsSigSemBinCreate("cmdShellMtx", CPSS_OS_SEMB_FULL_E, &gRunMtx);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Allocate command semaphore */
    rc = cmdOsSigSemBinCreate("cmdShellSem", CPSS_OS_SEMB_EMPTY_E, &gSemCmd);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Allocate command Processor semaphore */
    rc = cmdOsSigSemBinCreate("cmdProcSem", CPSS_OS_SEMB_EMPTY_E, &gSemProc);
    if (rc != GT_OK)
    {
        return rc;
    }

#ifdef GM_USED
    /* GM need huge stack size to take care long recursive calls */
    stackSize = 5000000;
#else
    stackSize = 0x20000; /* 128K */
#endif

    /* Create command execute task*/
    if (cmdOsTaskCreate(
            "cmdExec",
            CMD_EXE_TASK_PRIO,     /* thread priority      */
            stackSize,             /* thread stack size    */
            cmdProcTask,
            NULL,
            &gProcTaskId) != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cmdClearDupBuff function
* @endinternal
*
* @brief   initializes (clears) internal cyclic buffer, used as static input and
*         field container.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS cmdClearDupBuff(GT_VOID)
{
    cmdDupBufferOffset = 0;
    return GT_OK;
}

/**
* @internal cmdDuplicateString function
* @endinternal
*
* @brief   duplicates null terminated string and stores it in static buffer.
*
* @param[in] src                      - pointer to source NULL terminatyed string to be duplicated
*
* @param[out] dest                     - pointer to "safe" duplicated string
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure (out of memory - increase CMD_MAX_BUFFER size)
*/
static GT_STATUS cmdDuplicateString
(
    GT_CHAR *src,
    GT_CHAR **dest
)
{
    GT_U32 length = cmdOsStrlen(src) + 1;
    if (length + cmdDupBufferOffset > CMD_MAX_BUFFER)
    {
        *dest = NULL;
        return GT_FAIL;
    }
    cmdOsStrCpy(cmdDupBuffer + cmdDupBufferOffset, src);
    *dest = cmdDupBuffer + cmdDupBufferOffset;
    cmdDupBufferOffset += length;
    return GT_OK;
}

/**
* @internal cmdReadFields function
* @endinternal
*
* @brief   insert into fields[] the table fields of the calling command - if the
*         table has 2 fields fields[0] = table field 1, fields[1] = table field 2
* @param[in] fieldBuffer              - fields ascii buffer read from command line
*
* @param[out] fields[]                 - the recived table fields
* @param[out] numFields                - number of fields read
*
* @retval GT_OK                    - read successful
* @retval GT_FAIL                  - insufficient or too many fields in buffer
*
* @note none
*
*/
static GT_STATUS cmdReadFields
(
    IN  GT_CHAR   *fieldBuffer,
    OUT GT_UINTPTR  fields[],
    OUT GT_U32 *numFields
)
{
    GT_U32 fieldEnd = 0, field = 0, value;
    GT_CHAR *string;
    CMD_DEFAULT_TOKENS token;

    if (cmdParseInit(fieldBuffer) != GT_OK)
        return GT_FAIL;     /* internal error */

    while (!fieldEnd) {
        if (cmdParseRead() != GT_OK)
            return GT_FAIL;     /* not enough arguments */
        if (cmdParseGet(&value) != GT_OK)
            return GT_FAIL;
        token = (CMD_DEFAULT_TOKENS)value;
        switch (token) {
            case tokDECIMAL:
            case tokHEXADECIMAL:
                if (cmdParseGetNumeric(fields + (field++)) != GT_OK)
                    return GT_FAIL;
                break;
            case tokSTRING:
                if (cmdParseGetString(&string) != GT_OK)
                    return GT_FAIL;
                if (cmdDuplicateString(string,
                    (GT_CHAR**) (fields + (field++))) != GT_OK)
                        return GT_FAIL;
                break;
            case (tokEOF):
                fieldEnd = 1;
                break;
            default:
                return GT_FAIL;
        }
    }
    *numFields = field;
    return (field > 0) ? GT_OK : GT_FAIL;
}

/**
* @internal cmdEventFields function
* @endinternal
*
* @brief   parses input buffer, and tells whether command has fields as input
*
* @param[in] inBuffer                 - null terminated string holding command buffer
*
* @retval GT_TRUE                  - fields need to be read
* @retval GT_FALSE                 - command has no fields as input
*/
GT_BOOL cmdEventFields
(
    GT_CHAR *inBuffer
)
{
    CMD_COMMAND  *command;
    GT_CHAR      *commandName;
    GT_32         token;

    if (cmdParseInit(inBuffer) != GT_OK)
        return GT_FALSE;    /* internal error */

    if (cmdParseRead() != GT_OK)
        return GT_FALSE;    /* internal error */

    if (cmdParseGet((GT_U32 *)&token) != GT_OK)
        return GT_FALSE;    /* internal error */

    if (token == tokEOF)
        return GT_FALSE;

    if (cmdParseGetConstant(&commandName) != GT_OK)
        return GT_FALSE;    /* invalid syntax */

    if (cmdGetCommand(commandName, &command) != GT_OK)
        return GT_FALSE;

    return (command->funcFields > 0) ? GT_TRUE : GT_FALSE;
}


/**
* @internal cmdEventRunTimeoutSet function
* @endinternal
*
* @brief   Set timeout for running a single command by cmdEventRun function.
*
* @param[in] cmdTimeout               - command run timeout
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdEventRunTimeoutSet
(
    IN GT_U32 cmdTimeout
)
{
    cmdSysTimeout = cmdTimeout;

    return GT_OK;
}


/**
* @internal cmdEventRun function
* @endinternal
*
* @brief   command interpreter; parses and executes single command stored
*         in null terminated string.
* @param[in] inBuffer                 - null terminated string holding command buffer
* @param[in] inFieldBuffer            - null terminated string holding field values
*
* @param[out] outBuffer                - pointer to null terminated string holding output
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
CMD_STATUS cmdEventRun
(
    GT_CHAR *inBuffer,
    GT_CHAR *inFieldBuffer,
    GT_CHAR **outBuffer
)
{

    CMD_COMMAND  *command;
    GT_CHAR      *commandName, *string;
    GT_U32        args, i;
    GT_32         token;
    GT_STATUS     status;
    CMD_DEFAULT_TOKENS token_;


    if (cmdParseInit(inBuffer) != GT_OK)
    {
        return CMD_AGENT_ERROR;     /* internal error */
    }

    if (cmdParseRead() != GT_OK)
    {
        return CMD_AGENT_ERROR;     /* internal error */
    }

    if (cmdParseGet((GT_U32 *)&token) != GT_OK)
    {
        return CMD_AGENT_ERROR;     /* internal error */
    }
    token_ = (CMD_DEFAULT_TOKENS)token;
    if (token_ == tokEOF)
    {
        *outBuffer = "";
        return CMD_OK;
    }

    cmdOsSigSemWait(gRunMtx, CPSS_OS_SEM_WAIT_FOREVER_CNS);

    cmdClearDupBuff();
    status = cmdParseGetConstant(&commandName);
    if (status != GT_OK)
    {
        cmdOsSigSemSignal(gRunMtx);
        return CMD_SYNTAX_ERROR;    /* invalid syntax */
    }
    /* Non command shell*/
    isShellCmd = GT_FALSE;
    if ((inBuffer[0] == '@') && (inBuffer[1] == '@'))
    {
        isShellCmd = GT_TRUE;
        gShellInBufferPtr = inBuffer + 2;

    }

    /* Reset time out */
    timeOutExpired = 0;

    if ((isShellCmd == GT_FALSE) &&
        ((status = cmdGetCommand(commandName, &command)) == GT_OK))
    {
        args = command->funcArgs;
        /* argument parsing pipeline */
        for (i = 0; i < args; i++)
        {
            if (i != 0)
            {
                if (cmdParseRead() != GT_OK)
                {
                    cmdOsSigSemSignal(gRunMtx);
                    return CMD_AGENT_ERROR;     /* internal error */
                }
                if (cmdParseGet((GT_U32 *)&token) != GT_OK)
                {
                    cmdOsSigSemSignal(gRunMtx);
                    return CMD_AGENT_ERROR;     /* internal error */
                }
                token_ = (CMD_DEFAULT_TOKENS)token;
                if (token_ != tokCOMMA) {
                    if (token_ == tokEOF)
                    {
                        cmdOsSigSemSignal(gRunMtx);
                        return CMD_ARG_UNDERFLOW;
                    }
                    cmdOsSigSemSignal(gRunMtx);
                    return CMD_SYNTAX_ERROR;    /* invalid syntax */
                }
            }

            if (cmdParseRead() != GT_OK)
            {
                cmdOsSigSemSignal(gRunMtx);
                return CMD_SYNTAX_ERROR;        /* invalid syntax */
            }
            if (cmdParseGet((GT_U32 *)&token) != GT_OK)
            {
                cmdOsSigSemSignal(gRunMtx);
                return CMD_ARG_UNDERFLOW;
            }
            token_ = (CMD_DEFAULT_TOKENS)token;
            switch (token_)
            {
                /* numeric argument */
                case tokDECIMAL:
                case tokHEXADECIMAL:
                    if(cmdParseGetNumeric((GT_UINTPTR *)(inputBuffer + i)) != GT_OK)
                    {
                        cmdOsSigSemSignal(gRunMtx);
                        return CMD_AGENT_ERROR;   /* invalid / not enough arguments */
                    }
                    break;
                /* character buffer: pass as a pointer to string */
                case tokSTRING:
                    if (cmdParseGetString(&string) != GT_OK)
                    {
                        cmdOsSigSemSignal(gRunMtx);
                        return CMD_AGENT_ERROR;
                    }
                    if (cmdDuplicateString(string,
                        (GT_CHAR**) (inputBuffer + i)) != GT_OK)
                    {
                        cmdOsSigSemSignal(gRunMtx);
                        return CMD_AGENT_ERROR;
                    }
                    break;
                case (tokEOF):
                    cmdOsSigSemSignal(gRunMtx);
                    return CMD_ARG_UNDERFLOW;
                default:
                    cmdOsSigSemSignal(gRunMtx);
                    return CMD_SYNTAX_ERROR;
            }
        }

        /* read and parse fields, if available */
        if (command->funcFields > 0)
        {
            if(cmdReadFields(inFieldBuffer,fieldBuffer,&numFields)
               != GT_OK)
            {
                cmdOsSigSemSignal(gRunMtx);
                return CMD_SYNTAX_ERROR;
            }
        }

        /* parsing successfull: execute api function */
        /* Take the semaphore - update operation to */
        wrapperRef = command->funcReference;

        /* Signal proc taske that a new command is ready */
        status = cmdOsSigSemSignal(gSemProc);
        if (status != GT_OK)
        {
            cmdOsSigSemSignal(gRunMtx);
            return CMD_AGENT_ERROR;
        }

        while (timeOutExpired < cmdSysTimeout)
        {
            status = cmdOsSigSemWait(gSemCmd, CMD_PROC_TIMOUT);
            /* Time out expired */
            if (status == GT_TIMEOUT)
            {
                timeOutExpired += CMD_PROC_TIMOUT;
            }
            else
            {
                *outBuffer = outputBuffer;
                cmdOsSigSemSignal(gRunMtx);
                return status;
            }
        }

    }
    else
    {
        /* This is a shell command and not Galtis */
        /* shell command may be already recognized by "@@" prefix */
        if (isShellCmd == GT_FALSE)
        {
            isShellCmd = GT_TRUE;
            gShellInBufferPtr = inBuffer;
        }

        /* Signal proc task that a new command is ready */
        status = cmdOsSigSemSignal(gSemProc);
        if (status != GT_OK)
        {
            cmdOsSigSemSignal(gRunMtx);
            return CMD_AGENT_ERROR;
        }

        while (timeOutExpired < cmdSysTimeout)
        {
            status = cmdOsSigSemWait(gSemCmd, CMD_PROC_TIMOUT);
            /* Time out expired - wait 3 minutes*/
            if (status == GT_TIMEOUT)
            {
                timeOutExpired += CMD_PROC_TIMOUT;
            }
            else
            {
                if (gRc == GT_OK)
                {
                    /* Copy the shell massage to the output buffer*/
                    cmdOsMemCpy(outputBuffer, SHELL_CMD_MSG, SHELL_CMD_MSG_LENGTH);
                    *outBuffer = outputBuffer;
                    cmdOsSigSemSignal(gRunMtx);
                    return CMD_OK;
                }

                cmdOsSigSemSignal(gRunMtx);
                return CMD_SYNTAX_ERROR; /* command not found */
            }
        }
    }

    cmdOsPrintf("\nCommand timeout expired.\n");

    cmdOsSigSemSignal(gRunMtx);
    return CMD_OK;
}

/**
* @internal cmdParseLine function
* @endinternal
*
* @brief   Parses single command stored in null terminated string.
*
* @param[in] inBuffer                 - null terminated string holding command buffer
* @param[in] inFieldBuffer            - null terminated string holding field values
*
* @param[out] isShellCmd               - GT_TRUE, if it is shell command.
*
* @retval CMD_OK                   - if there are no problems,
*/
CMD_STATUS cmdParseLine
(
    IN  GT_CHAR      *inBuffer,
    IN  GT_CHAR      *inFieldBuffer,
    OUT GT_BOOL      *isShellCmd
)
{
    GT_CHAR      *commandName, *string;
    CMD_COMMAND  *command;
    GT_U32       numFields;          /* number of fields   */
    GT_32        token;
    GT_U32       args, i;
    GT_STATUS    status;
    CMD_DEFAULT_TOKENS token_;

    if (cmdParseInit(inBuffer) != GT_OK)
        return CMD_AGENT_ERROR;     /* internal error */

    if (cmdParseRead() != GT_OK)
        return CMD_AGENT_ERROR;     /* internal error */

    if (cmdParseGet((GT_U32 *)&token) != GT_OK)
        return CMD_AGENT_ERROR;     /* internal error */

    token_ = (CMD_DEFAULT_TOKENS)token;
    if (token_ == tokEOF)
    {
        return CMD_OK;
    }

    cmdClearDupBuff();
    status = cmdParseGetConstant(&commandName);
    if (status != GT_OK)
    {
        return CMD_SYNTAX_ERROR;
    }

        /* Non command shell*/
    *isShellCmd = GT_FALSE;

    /* get command name */
    if ((status = cmdGetCommand(commandName, &command)) == GT_OK)
    {
        args = command->funcArgs;
        /* argument parsing pipeline */
        for (i = 0; i < args; i++)
        {
            if (i != 0)
            {
                if (cmdParseRead() != GT_OK)
                    return CMD_AGENT_ERROR;     /* internal error */

                if (cmdParseGet((GT_U32 *)&token) != GT_OK)
                    return CMD_AGENT_ERROR;     /* internal error */
                token_ = (CMD_DEFAULT_TOKENS)token;

                if (token_ != tokCOMMA) {
                    if (token_ == tokEOF)
                    {
                        return CMD_ARG_UNDERFLOW;
                    }

                    return CMD_SYNTAX_ERROR;    /* invalid syntax */
                }
            }

            if (cmdParseRead() != GT_OK)
            {
                return CMD_SYNTAX_ERROR;        /* invalid syntax */
            }


            if (cmdParseGet((GT_U32 *)&token) != GT_OK)
            {
                return CMD_ARG_UNDERFLOW;
            }
            token_ = (CMD_DEFAULT_TOKENS)token;

            switch (token_) {
                /* numeric argument */
                case tokDECIMAL:
                case tokHEXADECIMAL:
                    if(cmdParseGetNumeric(inputBuffer + i) != GT_OK)
                        /* invalid / not enough arguments */
                        return CMD_AGENT_ERROR;
                    break;
                /* character buffer: pass as a pointer to string */
                case tokSTRING:
                    if (cmdParseGetString(&string) != GT_OK)
                        return CMD_AGENT_ERROR;
                    if (cmdDuplicateString(string,
                        (GT_CHAR**) (inputBuffer + i)) != GT_OK)
                            return CMD_AGENT_ERROR;
                    break;
                case (tokEOF):
                    return CMD_ARG_UNDERFLOW;
                default:
                    return CMD_SYNTAX_ERROR;
            }
        }

        /* read and parse fields, if available */
        if (command->funcFields > 0)
        {
            if(cmdReadFields(inFieldBuffer,fieldBuffer,&numFields)
               != GT_OK)
            {
                return CMD_SYNTAX_ERROR;
            }
        }
    }
    else
    {
        /* This is a shell command and not Galtis*/
        *isShellCmd = GT_TRUE;
        gShellInBufferPtr = inBuffer;
    }

    return CMD_OK;
}


#if defined(CMD_LUA_CLI) && defined(CMD_LUA_GALTIS)

#include <lua.h>
#include "lua/cmdLua.h"


int cmdLuaGaltisWrapper = 0;
static lua_State *gL = NULL;
static GT_BOOL fieldOutputAppendMode = GT_FALSE;
static int nresults;
static int nfields;
static int fieldsIndex;

/*******************************************************************************
* cmdLuaGaltisWrapperPush
*
* DESCRIPTION:
*       Push to stack one galtis output value
*
* INPUTS:
*       format          - the incoming arguments format (%d%c%x%s...). The %f
*                         can be used for adding a pre-prepared fields output
*                         string (by using fieldOutput function)
*       ap              - list of arguments to put in the resultString. For %f
*                         format no argument is needed
*
* OUTPUTS:
*       a value pushed to gL stack
*
* RETURNS:
*       pointer to next item in format string
*
* COMMENTS:
*
*******************************************************************************/
const GT_CHAR* cmdLuaGaltisWrapperPush(const GT_CHAR* format, va_list *argP)
{
    const GT_CHAR *p = format;
    GT_32 arsize, idx;

    while (*p && *p != '%')
        p++;

    if (!*p)
        return p;

    switch (*++p) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case '*':
            if (*p == '*')
            {
                p++;
                arsize = va_arg(*argP, GT_32);
            } else {
                arsize = 0;
                while (*p >= '0' && *p <= '9')
                    arsize = (arsize * 10) + (*p++ - '0');
            }
            switch (*p) {
                case 'b':
                case 'B':
                    {
                        GT_U8* dest;
                        GT_U8* array = va_arg(*argP, GT_U8*);

                        dest = (GT_U8*)lua_newuserdata(gL, arsize);
                        for (idx = 0; idx < arsize; idx++) {
                            dest[idx] = array[idx];
                        }
                        cmdLuaSetCustomType(gL, -1, CMD_LUA_TYPE_BYTEARRAY);
                    }
                    break;
                case 'w':
                case 'W':
                    {
                        GT_U8* dest;
                        GT_U16* array = va_arg(*argP, GT_U16*);

                        dest = (GT_U8*)lua_newuserdata(gL, arsize*2);
                        for (idx = 0; idx < arsize; idx++) {
                            dest[idx*2]   = (GT_U8)((array[idx]>>8) & 0xff);
                            dest[idx*2+1] = (GT_U8)( array[idx]     & 0xff);
                        }
                        cmdLuaSetCustomType(gL, -1, CMD_LUA_TYPE_U16ARRAY);
                    }
                    break;
                case 'd':
                case 'D':
                    {
                        GT_U8* dest;
                        GT_U32* array = va_arg(*argP, GT_U32*);

                        dest = (GT_U8*)lua_newuserdata(gL, arsize*4);
                        for (idx = 0; idx < arsize; idx++) {
                            dest[idx*4]   = (GT_U8)((array[idx]>>24) & 0xff);
                            dest[idx*4+1] = (GT_U8)((array[idx]>>16) & 0xff);
                            dest[idx*4+2] = (GT_U8)((array[idx]>>8)  & 0xff);
                            dest[idx*4+3] = (GT_U8)( array[idx]      & 0xff);
                        }
                        cmdLuaSetCustomType(gL, -1, CMD_LUA_TYPE_U32ARRAY);
                    }
                    break;
            }
            break;
        case 'x':
        case 'X':
            /* get the data and point to next parameter in the variable list */
            lua_pushnumber(gL, va_arg(*argP, GT_U32));
            break;
        case 'd':
        case 'D':
            /* get the data and point to next parameter in the variable list */
            lua_pushnumber(gL, va_arg(*argP, GT_32));
            break;

        case 'l':
        case 'L':
            {
                GT_U8* dest;
                GT_U64 *big;
                big = va_arg(*argP, GT_U64*);

                dest = (GT_U8*)lua_newuserdata(gL, 8);
                dest[0] = (GT_U8)((big->l[0]>>24) & 0xff);
                dest[1] = (GT_U8)((big->l[0]>>16) & 0xff);
                dest[2] = (GT_U8)((big->l[0]>>8)  & 0xff);
                dest[3] = (GT_U8)( big->l[0]      & 0xff);
                dest[4] = (GT_U8)((big->l[1]>>24) & 0xff);
                dest[5] = (GT_U8)((big->l[1]>>16) & 0xff);
                dest[6] = (GT_U8)((big->l[1]>>8)  & 0xff);
                dest[7] = (GT_U8)( big->l[1]      & 0xff);
                cmdLuaSetCustomType(gL, -1, CMD_LUA_TYPE_U64);
            }
            break;

        case 's':
        case 'S':
            /* get the data and point to next parameter in the variable list */
            lua_pushstring(gL, va_arg(*argP, GT_8*));
            break;
        default:
            break;
    }
    p++;
    return p;
}


/**
* @internal cmdLuaGaltisWrapperOutput function
* @endinternal
*
* @brief   captures galtis output
*/
GT_VOID cmdLuaGaltisWrapperOutput(GT_STATUS status, const GT_CHAR *format, va_list ap)
{
    const GT_CHAR *p = format;
    if (!gL)
        return;
#if 0
    lua_getglobal(gL, "print");
    lua_pushfstring(gL, "cmdLuaGaltisWrapperOutput() status=%d format=%s",status,format);
    lua_call(gL, 1, 0);
#endif
    lua_pushnumber(gL, status);
    nresults = 1;

    if (!*p)
        return;
    if (*p != '%')
    {
        lua_pushstring(gL, format);
        nresults++;
        return;
    }
    while (*p)
    {
        if (*p != '%')
            break;
        if (p[1] == 'f' || p[1] == 'F')
        {
            p += 2;
            lua_pushvalue(gL, fieldsIndex);
            nresults++;
        } else {
            p = cmdLuaGaltisWrapperPush(p, &ap);
            nresults++;
        }
    }
}

/**
* @internal cmdLuaGaltisWrapperFieldOutput function
* @endinternal
*
* @brief   captures galtis output
*         builds table field on the gL stack at index fieldsIndex
*/
GT_VOID cmdLuaGaltisWrapperFieldOutput(const GT_CHAR *format, va_list ap)
{
    const GT_CHAR *p = format;

    if (!gL)
        return;
#if 0
    lua_getglobal(gL, "print");
    lua_pushfstring(gL, "cmdLuaGaltisWrapperFieldOutput() format=%s",format);
    lua_call(gL, 1, 0);
#endif

    if (fieldOutputAppendMode == GT_FALSE)
    {
        lua_newtable(gL);
        lua_replace(gL, fieldsIndex);
        nfields = 0;
    }
    fieldOutputAppendMode = GT_FALSE;

    while (*p)
    {
        p = cmdLuaGaltisWrapperPush(p, &ap);
        lua_rawseti(gL, fieldsIndex, ++nfields);
    }
}

/**
* @internal cmdLuaGaltisWrapperFieldSetAppendMode function
* @endinternal
*
* @brief   causes fieldOutput to continue the output string
*/
GT_VOID cmdLuaGaltisWrapperFieldSetAppendMode(GT_VOID)
{
    if (!gL)
        return;
    lua_getglobal(gL, "print");
    lua_pushliteral(gL, "cmdLuaGaltisWrapperFieldSetAppendMode()");
    fieldOutputAppendMode = GT_TRUE;
}

static int cmdLuaValueToGT32(lua_State *L, int i, GT_UINTPTR *ret)
{
    switch (lua_type(L, i))
    {
        case LUA_TNIL:
            *ret = 0;
            return 0;
        case LUA_TBOOLEAN:
            *ret = lua_toboolean(L, i) ? 1 : 0;
            return 0;
        case LUA_TNUMBER:
            *ret = (GT_32)lua_tointeger(L, i);
            return 0;
        case LUA_TSTRING:
            *ret = (GT_32)lua_tostring(L, i);
            return 0;
        case LUA_TUSERDATA:
            /* encode in hex and pass as string */
            /* not supported yet */
            /* return 0 */
            break;
        /* special case for GT_U64?? */
    }
    return 1;
}
/**
* @internal cmdLuaRunGaltisWrapper function
* @endinternal
*
* @brief   Wrapper to execute existing Galtis Wrappers
*/
int cmdLuaRunGaltisWrapper(lua_State *L)
{
    CMD_COMMAND     *command;
    GT_STATUS       status;
    int             numArgs;
    int             n, i;
    int             err;

    n = lua_gettop(L);
    if (n < 1 || !lua_isstring(L, 1))
    {
        lua_pushnumber(L, GT_BAD_PARAM);
        return 1;
    }

    status = cmdGetCommand(lua_tostring(L, 1), &command);
    if (status != GT_OK)
    {
        lua_pushnumber(L, GT_NOT_SUPPORTED);
        return 1;
    }

    /* wrapper exists */
    cmdOsSigSemWait(gRunMtx, CPSS_OS_SEM_WAIT_FOREVER_CNS);

    numArgs = 0;
    numFields = 0;
    for (err = 0,i = 2; i <= n; i++)
    {
        if (lua_type(L,i) != LUA_TTABLE)
        {
            if (numArgs < CMD_MAX_ARGS)
            {
                err = cmdLuaValueToGT32(L, i, inputBuffer+numArgs);
                if (!err)
                    numArgs++;
            } else {
                err = 1;
            }
        }
        else
        {
            /* LUA_TTABLE */
            int k;
            for (k = 1; ; k++)
            {
                lua_rawgeti(L, i, k); /* get value */
                if (lua_isnil(L, -1))
                {
                    lua_pop(L, 1);
                    break;
                }
                if (numFields < CMD_MAX_FIELDS)
                {
                    err = cmdLuaValueToGT32(L, -1, fieldBuffer+numFields);
                    if (!err)
                        numFields++;
                } else {
                    err = 1;
                }
                lua_pop(L, 1);
            }
        }
        if (err)
            break;
    }

    if (err || numArgs < command->funcArgs || numFields < command->funcFields)
    {
        cmdOsSigSemSignal(gRunMtx);
        lua_pushnumber(L, GT_BAD_PARAM);
        return 1;
    }

    nresults = 0;
    nfields = 0;

    lua_newtable(L);
    fieldsIndex = lua_gettop(L);

    /* Enable galtisOutput hook */
    gL = L;
    cmdLuaGaltisWrapper = 1;

    isShellCmd = GT_FALSE;
    wrapperRef = command->funcReference;

    /* Reset time out */
    timeOutExpired = 0;

    /* Signal proc taske that a new command is ready */
    status = cmdOsSigSemSignal(gSemProc);
    if (status != GT_OK)
    {
        cmdLuaGaltisWrapper = 0;
        cmdOsSigSemSignal(gRunMtx);
        lua_pushnumber(L, CMD_AGENT_ERROR);
        return 1;
    }

    while (timeOutExpired < cmdSysTimeout)
    {
        status = cmdOsSigSemWait(gSemCmd, CMD_PROC_TIMOUT);
        /* Time out expired */
        if (status == GT_TIMEOUT)
        {
            timeOutExpired += CMD_PROC_TIMOUT;
        }
        else
        {
            /* Reset timeout */
            timeOutExpired = 0;
            break;
        }
    }
    /* disable galtisOutput hook */
    cmdLuaGaltisWrapper = 0;
    gL = NULL;


    if (timeOutExpired >= cmdSysTimeout)
    {
        /* Time out expired */
        lua_pushnumber(L, GT_TIMEOUT);
        n = 1;
    } else {
        n = nresults;
    }

    cmdOsSigSemSignal(gRunMtx);

    return nresults;
}

#endif /* defined(CMD_LUA_CLI) && defined(CMD_LUA_GALTIS) */

#if defined(CMD_LUA_CLI) && !defined(CMD_LUA_GALTIS)
    /* stubs to make cmdWrapUtils happy */
    int cmdLuaGaltisWrapper = 0;
    GT_VOID cmdLuaGaltisWrapperOutput(GT_STATUS status, const GT_CHAR *format, va_list ap) {GT_UNUSED_PARAM(status); GT_UNUSED_PARAM(format); GT_UNUSED_PARAM(ap);}
    GT_VOID cmdLuaGaltisWrapperFieldOutput(const GT_CHAR *format, va_list ap) {GT_UNUSED_PARAM(format); GT_UNUSED_PARAM(ap);}
    GT_VOID cmdLuaGaltisWrapperFieldSetAppendMode(GT_VOID) {}
#endif /* defined(CMD_LUA_CLI) && !defined(CMD_LUA_GALTIS) */


