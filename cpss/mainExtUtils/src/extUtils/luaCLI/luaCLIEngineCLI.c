/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* luaCLI.c
*
* DESCRIPTION:
*       a LUA CLI implementation
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 37 $
**************************************************************************/
#include <gtOs/gtOsGen.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>
#include <lua.h>
#include <extUtils/luaCLI/luaCLIEngineCLI.h>
#include <gtOs/gtOsMem.h>



#if defined(SHARED_MEMORY)

#include <gtOs/gtOsSharedFunctionPointers.h>
#include <gtOs/gtOsSharedUtil.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif

#include <stdlib.h>

#ifdef USE_CLI_MALLOC
#include <cmdShell/lua/cliMemLib.h>
#define LUACLI_MALLOC(len)          cliMalloc((GT_U32)(len))
#define LUACLI_MALLOC_F             cliMalloc
#define LUACLI_FREE                 cliFree
#else
#define LUACLI_MALLOC               malloc
#define LUACLI_MALLOC_F             malloc
#define LUACLI_FREE                 free
#endif

/***** Externs  ********************************************************/
/* global storage for Lua data */
int luaGlobalStore(lua_State *L);
int luaGlobalGet(lua_State *L);

/* globals to support DXPX change mode*/
GT_U32 luaRestartRequest = 0;
GT_U32 luaModeNum = 0;


/***** Defines  ********************************************************/
#define CLI_BANNER  "\n LUA CLI based on LUA 5.1 from www.lua.org\n"    \
                    " LUA CLI uses Mini-XML engine from www.minixml.org\n"  \
                    "***************************************************\n"     \
                    "               LUA CLI shell ready\n"     \
                    "***************************************************\n\n"

#define CLI_PROMPT  ">"
#define CLI_READLINE_FLAG_HISTORY       1
#define CLI_READLINE_FLAG_COMPLETION    2
#define IS_CLI_CMD_EXIT(line)       \
        (luaCLI_strncmp(line,"CLIexit", 7) == 0 \
          && (line[7] == 0 || line[7] == ' '))
#define IS_CLI_SWITCH_MODE_TO_DX(line)       \
        (luaCLI_strncmp(line,"setModeDX", 9) == 0 \
          && (line[9] == 0 || line[9] == ' '))
#define IS_CLI_SWITCH_MODE_TO_PX(line)       \
        (luaCLI_strncmp(line,"setModePX", 9) == 0 \
          && (line[9] == 0 || line[9] == ' '))

#ifdef LUACLI_MEMSTAT
#define IS_CLI_CMD_MEMSTAT(line)        \
        (luaCLI_strncmp(line,"CLImemstat", 10) == 0 \
          && (line[10] == 0 || line[10] == ' '))
#endif /* LUACLI_MEMSTAT */
#define IS_CLI_CMD_LUA(line)        \
        (luaCLI_strncmp(line,"lua", 3) == 0     \
          && (line[3] == 0 || line[3] == ' '))
#define IS_CLI_CMD_OSSHELL(line)        \
        (luaCLI_strncmp(line,"osShell", 7) == 0 \
          && (line[7] == 0 || line[7] == ' '))
#define IS_CLI_COMMENT(line)        \
        (luaCLI_strncmp(line,"//", 2) == 0)

#define LUA_PROMPT          "lua>"
#define LUA_PROMPT_CONT     "lua>>"

#define CLI_MAX_LINE_LEN            4000
#define CLI_HISTORY_LEN             1000
#define CLI_HISTORY_LEN_OVERHEAD    200

typedef void* (*LUACLI_MALLOC_T)(int size);
LUA_CLI_MALLOC_STC *memMgr;
/* compilation error for SHARED_MEMORY:
error: implicit declaration of function ‘osMemSet’ [-Werror=implicit-function-declaration] */
extern GT_VOID * osMemSet(IN GT_VOID * start,IN int       symbol,IN GT_U32    size);

#if defined(SHARED_MEMORY)
static GT_STATUS luaCLIEngine_initSharedLib
(
    void
);
#endif

/***** Public Functions ************************************************/

/**
* @internal luaCLI_strncmp function
* @endinternal
*
* @brief   strncmp
*
* @retval 0                        - if strings equal
* @retval 1                        - if not equal
*/
static int luaCLI_strncmp(const char *s1, const char *s2, int n)
{
    for (;n;n--,s1++,s2++)
    {
        if (*s1 == *s2)
        {
            if (*s1 == 0)
                return 0;
            continue;
        }
        return 1;
    }
    return 0;
}

#define KEY_ORDINAL_CHAR(key) (((key) & 0xffffff00) == 0)
#define KEY_ENTER       0x0100
#define KEY_CANCEL      0x0200
#define KEY_COMPLETE    0x0300
#define KEY_SHOWCOMPL   0x0400
#define KEY_ERACE       0x0500
#define KEY_DELETE      0x0600
#define KEY_LEFT        0x0700
#define KEY_RIGHT       0x0800
#define KEY_PREV        0x0900
#define KEY_NEXT        0x0a00
#define KEY_FIRST       0x0b00
#define KEY_LAST        0x0c00
#define KEY_HELP        0x0d00
#define KEY_OPERATE_AND_GET_NEXT 0x0e00
#define KEY_CLS         0x0f00


/**
* @internal luaCLI_readkey function
* @endinternal
*
* @brief   read a key from tty, parse key sequences to logical keys
*/
static int luaCLI_readkey(IOStreamPTR IOStream, int timeout)
{
    int state;
    /* VT100 key sequences */
    struct {
        char    *seq;
        int     key;
    } keyseq[] = {
        { "\033",   KEY_CANCEL },
        { "\t",     KEY_COMPLETE },
        { "\010",   KEY_ERACE },    /* backspace */
        { "\177",   KEY_ERACE },
        { "\04",    KEY_DELETE },   /* ctrl-D */
        { "\033[D", KEY_LEFT },     /* left */
        { "\033[C", KEY_RIGHT },    /* right */
        { "\033[A", KEY_PREV },     /* up */
        { "\020",   KEY_PREV },     /* Ctrl-P */
        { "\033[B", KEY_NEXT },     /* down */
        { "\016",   KEY_NEXT },     /* ctrl-N */
        { "\01",    KEY_FIRST },    /* ctrl-A */
        { "\05",    KEY_LAST },     /* ctrl-E */
#if 0
        { "\06",    KEY_SHOWCOMPL },/* ctrl-F */
#endif
        { "\07",    KEY_HELP },     /* ctrl-G */
        { "\017",   KEY_OPERATE_AND_GET_NEXT }, /* Ctrl-O */
        { "\014",   KEY_CLS },      /* ctrl-L */
        { NULL,     0 }
    };
    char buf[10];
    int ret, k, matches;


    state = 0;
    while (IOStream->connected(IOStream))
    {
        ret = IOStream->readChar(IOStream, buf+state, timeout);
        if (ret < 0)
        {
            /* error */
            return -1;
        }
        if (ret > 0)
        {
            if (state == 0)
            {
                /* Handle both CR and LF as KEY_ENTER. Ignore LF after CR */
                if (buf[0] == '\r')
                {
                    IOStream->wasCR = 1;
                    return KEY_ENTER;
                }
                if (buf[0] == '\n')
                {
                    if (IOStream->wasCR)
                    {
                        IOStream->wasCR = 0;
                        /* skip LF after CR */
                        continue;
                    }
                    IOStream->wasCR = 0;
                    return KEY_ENTER;
                }
            }
            IOStream->wasCR = 0;
            state++;
        }
        matches = 0;
        for (k = 0; keyseq[k].seq; k++)
        {
            if (cpssOsMemCmp(keyseq[k].seq, buf, state) == 0)
            {
                matches++;
            }
        }
        if (matches == 0)
        {
            if (state == 1)
                return (int)((unsigned char)buf[0]);
            return '?';
        }
        if (matches == 1)
        {
            for (k = 0; keyseq[k].seq; k++)
            {
                if (cpssOsMemCmp(keyseq[k].seq, buf, state) == 0)
                    break;
            }
            if (keyseq[k].seq[state] == 0)
                return keyseq[k].key;
            /* read till end of seq */
            continue;
        }
        /* matches > 1 */
        if (ret > 0)
            continue;
        /* timeout */
        buf[state] = 0;
        for (k = 0; keyseq[k].seq; k++)
        {
            /* one key matched */
            if (cpssOsMemCmp(keyseq[k].seq, buf, state + 1) == 0)
                return keyseq[k].key;
        }
        /* sequence not completed */
        return 0;
        }
    return -1;
}

/**
* @internal writeSeq1 function
* @endinternal
*
* @brief   Write control sequence to terminal with single parameter
*
* @param[in] IOStream
* @param[in] fmt
* @param[in] param
*/
static void writeSeq1(
    IOStreamPTR    IOStream,
    const char      *fmt,
    int             param
)
{
    char buf[20];
    int  l;
    l = cpssOsSprintf(buf, fmt, param);
    IOStream->write(IOStream, buf, l);
}

/**
* @internal shiftRight function
* @endinternal
*
* @brief   shift the line right 'count' chars from positon 'p'
*         shift line on terminal
* @param[in] IOStream
* @param[in] l
* @param[in] l
* @param[in] l
* @param[in] p
* @param[in] count
*
* @param[out] l
* @param[out] l
*/
static int shiftRight(
    IOStreamPTR    IOStream,
    char            *line,
    int             len,
    int             *l,
    int             p,
    int             count
)
{
    int k;
    if (count <= 0)
    {
        return 0;
    }
    if (*l + count > len)
    {
        return 0;
    }
    if (p > *l)
    {
        return 0;
    }
    if (p == *l)
    {
        *l += count;
        /* pad with spaces */
        for (k = 0; k < count; k++)
            line[p+k] = ' ';
        return 1;
    }
    /* shift string */
    for (k = *l-1; k >= p; k--)
        line[k+count] = line[k];
    *l += count;
    /* pad with spaces */
    for (k = 0; k < count; k++)
        line[p+k] = ' ';
    /* shift tty */
    writeSeq1(IOStream, "\033[%d@", count);
    return 1;
}

/**
* @internal shiftLeft function
* @endinternal
*
* @brief   shift the line left 'count' chars to positon 'p'
*         shift line on terminal
* @param[in] IOStream
* @param[in] l
* @param[in] l
* @param[in] l
* @param[in] p
* @param[in] count
*
* @param[out] l
* @param[out] l
*/
static int shiftLeft(
    IOStreamPTR    IOStream,
    char            *line,
    int             len GT_UNUSED,
    int             *l,
    int             p,
    int             count
)
{
    int k;
    if (count <= 0)
    {
        return 0;
    }
    if (p + count > *l)
    {
        return 0;
    }
    /* shift string */
    for (k = p; k + count < *l; k++)
        line[k] = line[k+count];
    *l -= count;
    /* shift tty */
    writeSeq1(IOStream, "\033[%dP", count);
    return 1;
}


/**
* @internal luaCLI_WriteConsoleLine function
* @endinternal
*
* @brief   Write Console# line
* @param[in] IOStream
* @param[in] luaCLI     - luaCLI structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS luaCLI_WriteConsoleLine
(
    IOStreamPTR                 IOStream,
    CPSS_LUACLI_INSTANCE_STC    *luaCLI
)
{
    luaCLI->len = 0;
    luaCLI->pos = 0;
    luaCLI->history_len = 0;
    luaCLI->history_pos = 0;

    IOStream->setTtyMode(IOStream, 0);

    /*! put current prompt */
    if (luaCLI->prompt != NULL)
    {
        IOStream->writeLine(IOStream, luaCLI->prompt);
    }
    else
    {
        lua_getglobal(luaCLI->L, "cmdLuaCLI_prompt");
        if (lua_type(luaCLI->L, -1) == LUA_TSTRING)
        {
            IOStream->write(IOStream, lua_tostring(luaCLI->L, -1), (int)lua_objlen(luaCLI->L, -1));
        }
        else
        {
            luaCLI->prompt = CLI_PROMPT;
            IOStream->writeLine(IOStream, luaCLI->prompt);
        }
        lua_pop(luaCLI->L, 1);
    }


    if (luaCLI->flags & CLI_READLINE_FLAG_HISTORY)
    {
        lua_getglobal(luaCLI->L, "cmdLuaCLI_history");
        luaCLI->history_len = (int)lua_objlen(luaCLI->L, -1);
        lua_pop(luaCLI->L, 1);
        luaCLI->history_pos = luaCLI->history_len + 1;
        /* KEY_OPERATE_AND_GET_NEXT was pressed */
        lua_getglobal(luaCLI->L, "cmdLuaCLI_history_next");
        if (lua_isnumber(luaCLI->L, -1))
        {
            luaCLI->pos = (int)lua_tonumber(luaCLI->L, -1);
            lua_pop(luaCLI->L, 1);
            lua_pushnil(luaCLI->L);
            lua_setglobal(luaCLI->L, "cmdLuaCLI_history_next");
            if (luaCLI->pos > 0 && luaCLI->pos <= luaCLI->history_len)
            {
                const char    *s;
                luaCLI->history_pos -= luaCLI->pos;
                lua_getglobal(luaCLI->L, "cmdLuaCLI_history");
                lua_rawgeti(luaCLI->L, -1, luaCLI->history_pos);
                luaCLI->len = (int)lua_objlen(luaCLI->L, -1);
                s = lua_tostring(luaCLI->L, -1);
                for (luaCLI->pos = 0; luaCLI->pos < luaCLI->len; (luaCLI->pos)++)
                    luaCLI->line[luaCLI->pos] = s[luaCLI->pos];
                lua_pop(luaCLI->L, 2);
                IOStream->write(IOStream, luaCLI->line, luaCLI->pos);
            }
        }
        else
        {
            lua_pop(luaCLI->L, 1);
        }
    }

    return GT_OK;
}

/**
* @internal luaCLI_ReadKey function
* @endinternal
*
* @brief   readline implementation including the following
*         - line editing
*         - autocompletion
*         - history (not implemented yet)
* @param[in] IOStream
* @param[in] CPSS_LUACLI_INSTANCE_STC - luaCLI isntance
*       structure
*
* @param[in] timeout    - [Xms] (-1 for blocking)
*
* @param[out] key
*                                       if luaCLI->len < 0 then
*                                       error line length
*/
static int luaCLI_ReadKey
(
    IOStreamPTR                 IOStream,
    CPSS_LUACLI_INSTANCE_STC   *luaCLI,
    int                         timeout
)
{
    int     key = 0;
    int     ncompl = 0;

    do{
        key = luaCLI_readkey(IOStream, timeout);
        if (key < 0)
        {
            IOStream->setTtyMode(IOStream, 1);
            luaCLI->len = -1;
            return key;
        }
        if (key == 0){
            if (timeout == -1)
                continue;
            else
                return key;
        }
        if (key == KEY_COMPLETE && (luaCLI->flags & CLI_READLINE_FLAG_COMPLETION) != 0)
        {
            ncompl++;
#if 0
            if (ncompl == 2)
                key = KEY_SHOWCOMPL;
#endif
        } else {
            ncompl = 0;
        }
        if (key == KEY_ENTER)
        {
            IOStream->write(IOStream, "\r\n", 2);
            break;
        }
        if ((luaCLI->flags && CLI_READLINE_FLAG_HISTORY) &&
                (key == KEY_OPERATE_AND_GET_NEXT))
        {
            if (luaCLI->history_pos <= luaCLI->history_len)
            {
                lua_pushnumber(luaCLI->L, luaCLI->history_len + 1 - luaCLI->history_pos);
                lua_setglobal(luaCLI->L, "cmdLuaCLI_history_next");
            }
            IOStream->write(IOStream, "\r\n", 2);
            break;
        }
#define BELL() IOStream->write(IOStream, "\a",1)
        if (KEY_ORDINAL_CHAR(key) && key == '?' && luaCLI->pos == luaCLI->len)
        {
            /* ? typed at the last position enforces help */
            key = KEY_HELP;
        }
        if (KEY_ORDINAL_CHAR(key))
        {
            /* ordinal character */
            if (luaCLI->len+1 >= CLI_MAX_LINE_LEN)
            {
                BELL();
                continue;
            }
            if (!shiftRight(IOStream, luaCLI->line, CLI_MAX_LINE_LEN, &luaCLI->len, luaCLI->pos, 1))
            {
                BELL();
                continue;
            }
            luaCLI->line[(luaCLI->pos)++] = (char)key;
            IOStream->write(IOStream, luaCLI->line + luaCLI->pos - 1, 1);
            continue;
        }
        switch (key) {
            case KEY_ERACE:
                if (luaCLI->pos)
                {
                    if (luaCLI->pos == luaCLI->len)
                    {
                        (luaCLI->pos)--;
                        (luaCLI->len)--;
                        IOStream->write(IOStream, "\b \b", 3);
                    }
                    else
                    {
                        (luaCLI->pos)--;
                        IOStream->write(IOStream, "\b", 1);
                        if (!shiftLeft(IOStream, luaCLI->line, CLI_MAX_LINE_LEN, &luaCLI->len, luaCLI->pos, 1))
                        {
                            BELL();
                        }
                    }
                }
                break;
            case KEY_DELETE:
                if (!shiftLeft(IOStream, luaCLI->line, CLI_MAX_LINE_LEN, &luaCLI->len, luaCLI->pos, 1))
                {
                    BELL();
                }
                break;
            case KEY_LEFT:
                if (luaCLI->pos)
                {
                    (luaCLI->pos)--;
                    IOStream->write(IOStream, "\033[D",3);
                }
                else
                {
                    BELL();
                }
                break;
            case KEY_RIGHT:
                if (luaCLI->pos < luaCLI->len)
                {
                    (luaCLI->pos)++;
                    IOStream->write(IOStream, "\033[C",3);
                }
                else
                {
                    BELL();
                }
                break;
            case KEY_FIRST:
                if (luaCLI->pos)
                {
                    writeSeq1(IOStream, "\033[%dD",luaCLI->pos);
                    luaCLI->pos = 0;
                }
                else
                {
                    BELL();
                }
                break;
            case KEY_LAST:
                if (luaCLI->pos < luaCLI->len)
                {
                    writeSeq1(IOStream, "\033[%dC",luaCLI->len-luaCLI->pos);
                    luaCLI->pos = luaCLI->len;
                }
                else
                {
                    BELL();
                }
                break;
            case KEY_PREV:
            case KEY_NEXT:
                if ((luaCLI->flags & CLI_READLINE_FLAG_HISTORY) == 0)
                {
                    BELL();
                    break;
                }
                if ((key == KEY_PREV && luaCLI->history_pos - 1 > 0)
                    || (key == KEY_NEXT && luaCLI->history_pos <= luaCLI->history_len))
                {
                    const char *s;
                    if (luaCLI->pos)
                    {
                        writeSeq1(IOStream, "\033[%dD",luaCLI->pos);
                        luaCLI->pos = 0;
                    }
                    IOStream->write(IOStream, "\033[0K", 4);
                    lua_getglobal(luaCLI->L, "cmdLuaCLI_history");

                    if (key == KEY_PREV)
                    {
                        if (luaCLI->history_pos == luaCLI->history_len+1)
                        {
                            lua_pushlstring(luaCLI->L, luaCLI->line, luaCLI->len);
                            lua_rawseti(luaCLI->L, -2, luaCLI->history_pos);
                        }
                        (luaCLI->history_pos)--;
                    }
                    else
                    {
                        (luaCLI->history_pos)++;
                    }
                    lua_rawgeti(luaCLI->L, -1, luaCLI->history_pos);
                    luaCLI->len = (int)lua_objlen(luaCLI->L, -1);
                    s = lua_tostring(luaCLI->L, -1);
                    for (luaCLI->pos = 0; luaCLI->pos < luaCLI->len; (luaCLI->pos)++)
                        luaCLI->line[luaCLI->pos] = s[luaCLI->pos];
                    lua_pop(luaCLI->L, 2);
                    IOStream->write(IOStream, luaCLI->line, luaCLI->pos);
                }
                else
                {
                    BELL();
                }
                break;
#if 0
#define KEY_CANCEL
#endif
            case KEY_COMPLETE:
                if ((luaCLI->flags & CLI_READLINE_FLAG_COMPLETION) == 0)
                {
                    BELL();
                    break;
                }
                lua_getglobal(luaCLI->L, "cmdLuaCLI_complete");
                if (lua_isfunction(luaCLI->L, -1))
                {
                    int status;
                    lua_pushlstring(luaCLI->L, luaCLI->line, luaCLI->pos);
                    status = lua_pcall(luaCLI->L, 1, 2, 0);
                    if (status && !lua_isnil(luaCLI->L, -1))
                    {
                        /* any error while loading or executing */
                        const char *msg = lua_tostring(luaCLI->L, -1);
                        if (msg == NULL) msg = "(error object is not a string)";
                        IOStream->writeLine(IOStream, "Lua error: ");
                        IOStream->writeLine(IOStream, msg);
                        IOStream->writeLine(IOStream, "\n");
                        lua_pop(luaCLI->L, 1);
                        break;
                    }
                }
                else
                {
                    lua_pop(luaCLI->L, 1);
                    lua_pushliteral(luaCLI->L, "");
                    lua_newtable(luaCLI->L);
                }
                /* string expected */
                if (!lua_isstring(luaCLI->L, -2))
                {
                    lua_pop(luaCLI->L, 2);
                    BELL();
                    continue;
                }
                if (lua_objlen(luaCLI->L, -2) > 0)
                {
                    const char    *s;
                    int     k, n;

                    n = (int)lua_objlen(luaCLI->L, -2);
                    if (!shiftRight(IOStream, luaCLI->line, CLI_MAX_LINE_LEN, &luaCLI->len, luaCLI->pos, n))
                    {
                        lua_pop(luaCLI->L, 2);
                        BELL();
                        continue;
                    }
                    s = lua_tostring(luaCLI->L, -2);
                    for (k = 0; k < n; k++)
                        luaCLI->line[luaCLI->pos + k] = s[k];
                    IOStream->write(IOStream, luaCLI->line + luaCLI->pos, n);
                    luaCLI->pos += n;
                    if (lua_objlen(luaCLI->L, -1) < 2)
                    {
                        ncompl = 0;
                    }
                    lua_pop(luaCLI->L, 2);
                    break;
                }

                lua_pop(luaCLI->L, 2);

                /*
                    The order of the cases is IMPORTANT!

                    in case of no Completion,
                    continue to Help in order to show
                    the current options for the user  */
                GT_ATTR_FALLTHROUGH;
            case KEY_HELP:
                if ((luaCLI->flags & CLI_READLINE_FLAG_COMPLETION) == 0)
                {
                    BELL();
                    break;
                }
                lua_getglobal(luaCLI->L, "cmdLuaCLI_help");
                if (lua_isfunction(luaCLI->L, -1))
                {
                    int status;
                    lua_pushlstring(luaCLI->L, luaCLI->line, luaCLI->pos);
                    status = lua_pcall(luaCLI->L, 1, 1, 0);
                    if (status && !lua_isnil(luaCLI->L, -1))
                    {
                        /* any error while loading or executing */
                        const char *msg = lua_tostring(luaCLI->L, -1);
                        if (msg == NULL) msg = "(error object is not a string)";
                        IOStream->writeLine(IOStream, "?\r\nLua error: ");
                        IOStream->writeLine(IOStream, msg);
                        IOStream->writeLine(IOStream, "\n");
                        lua_pop(luaCLI->L, 1);
                        break;
                    }
                }
                else
                {
                    lua_pop(luaCLI->L, 1);
                    lua_pushliteral(luaCLI->L, "");
                }
                if (lua_objlen(luaCLI->L, -1) <= 0)
                {
                    /* no help */
                    lua_pop(luaCLI->L, 1);
                    BELL();
                    break;
                }
                /* help received */
                IOStream->write(IOStream, "?\r\n", 3);
                /* string.gsub(s, '\n', '\r\n') */
                lua_getglobal(luaCLI->L, "string");
                lua_getfield(luaCLI->L, -1, "gsub");
                lua_remove(luaCLI->L, -2);
                lua_insert(luaCLI->L, -2);
                lua_pushliteral(luaCLI->L, "\n");
                lua_pushliteral(luaCLI->L, "\r\n");
                lua_call(luaCLI->L, 3, 1);
                IOStream->write(IOStream, lua_tostring(luaCLI->L, -1), (int)lua_objlen(luaCLI->L, -1));
                lua_pop(luaCLI->L, 1);
                IOStream->write(IOStream, "\r\n", 2);
                /*! put current prompt */
                if (luaCLI->prompt != NULL)
                {
                    IOStream->writeLine(IOStream, luaCLI->prompt);
                }
                else
                {
                    lua_getglobal(luaCLI->L, "cmdLuaCLI_prompt");
                    IOStream->write(IOStream, lua_tostring(luaCLI->L, -1), (int)lua_objlen(luaCLI->L, -1));
                    lua_pop(luaCLI->L, 1);
                }
                IOStream->write(IOStream, luaCLI->line, luaCLI->len);
                if (luaCLI->pos < luaCLI->len)
                    writeSeq1(IOStream, "\033[%dD", luaCLI->len-luaCLI->pos);
                break;

            case KEY_SHOWCOMPL:
                if ((luaCLI->flags & CLI_READLINE_FLAG_COMPLETION) == 0)
                {
                    BELL();
                    break;
                }
                lua_getglobal(luaCLI->L, "cmdLuaCLI_complete");
                if (lua_isfunction(luaCLI->L, -1))
                {
                    int status;
                    lua_pushlstring(luaCLI->L, luaCLI->line, luaCLI->pos);
                    status = lua_pcall(luaCLI->L, 1, 2, 0);
                    if (status && !lua_isnil(luaCLI->L, -1))
                    {
                        /* any error while loading or executing */
                        const char *msg = lua_tostring(luaCLI->L, -1);
                        if (msg == NULL) msg = "(error object is not a string)";
                        IOStream->writeLine(IOStream, "Lua error: ");
                        IOStream->writeLine(IOStream, msg);
                        IOStream->writeLine(IOStream, "\n");
                        lua_pop(luaCLI->L, 1);
                        break;
                    }
                }
                else
                {
                    lua_pop(luaCLI->L, 1);
                    lua_pushliteral(luaCLI->L, "");
                    lua_newtable(luaCLI->L);
                }
                /* table expected */
                if (!lua_istable(luaCLI->L, -1))
                {
                    lua_pop(luaCLI->L, 2);
                    BELL();
                    continue;
                }
                if (lua_objlen(luaCLI->L, -1) < 1)
                {
                    lua_pop(luaCLI->L, 2);
                    BELL();
                    continue;
                }
                IOStream->write(IOStream, "\r\n", 2);
                lua_pushnil(luaCLI->L);  /* first key */
                while (lua_next(luaCLI->L, -2) != 0)
                {
                    /* uses 'key' (at index -2) and 'value' (at index -1) */
                    IOStream->write(IOStream, "\t",1);
                    IOStream->write(IOStream, lua_tostring(luaCLI->L, -1), (int)lua_objlen(luaCLI->L, -1));
                    /* removes 'value'; keeps 'key' for next iteration */
                    lua_pop(luaCLI->L, 1);
                }

                lua_pop(luaCLI->L, 2);

                IOStream->write(IOStream, "\r\n", 2);
                /*! put current prompt */
                if (luaCLI->prompt != NULL)
                {
                    IOStream->writeLine(IOStream, luaCLI->prompt);
                }
                else
                {
                    lua_getglobal(luaCLI->L, "cmdLuaCLI_prompt");
                    IOStream->write(IOStream, lua_tostring(luaCLI->L, -1), (int)lua_objlen(luaCLI->L, -1));
                    lua_pop(luaCLI->L, 1);
                }
                IOStream->write(IOStream, luaCLI->line, luaCLI->len);
                if (luaCLI->pos < luaCLI->len)
                    writeSeq1(IOStream, "\033[%dD", luaCLI->len-luaCLI->pos);
                break;

            case KEY_OPERATE_AND_GET_NEXT:
                BELL();
                break;
            case KEY_CLS:
                IOStream->writeLine(IOStream, "\033[H\033[J");
                /*! put current prompt */
                if (luaCLI->prompt != NULL)
                {
                    IOStream->writeLine(IOStream, luaCLI->prompt);
                }
                else
                {
                    lua_getglobal(luaCLI->L, "cmdLuaCLI_prompt");
                    IOStream->write(IOStream, lua_tostring(luaCLI->L, -1), (int)lua_objlen(luaCLI->L, -1));
                    lua_pop(luaCLI->L, 1);
                }
                IOStream->write(IOStream, luaCLI->line, luaCLI->len);
                if (luaCLI->pos < luaCLI->len)
                    writeSeq1(IOStream, "\033[%dD", luaCLI->len-luaCLI->pos);
                break;

            default:
                {
                    char aa[40];
                    sprintf(aa, "<KEY=0x%04x>", key);
                    IOStream->writeLine(IOStream, aa);
                }
        }
    }while (IOStream->connected(IOStream) && luaCLI->pos < CLI_MAX_LINE_LEN);
    IOStream->setTtyMode(IOStream, 1);

    if (luaCLI->len < CLI_MAX_LINE_LEN)
    {
        luaCLI->line[luaCLI->len] = 0;
    }
    /* if not empty push line to history */
    if (luaCLI->len > 0 && (luaCLI->flags & CLI_READLINE_FLAG_HISTORY) != 0)
    {
        if (luaCLI->history_len + 1 >= CLI_HISTORY_LEN + CLI_HISTORY_LEN_OVERHEAD)
        {
            /* cleanup history: keep CLI_HISTORY_LEN lines only */
            lua_getglobal(luaCLI->L, "cmdLuaCLI_history");
            lua_newtable(luaCLI->L);
            for (luaCLI->pos = 1; luaCLI->pos < CLI_HISTORY_LEN; (luaCLI->pos)++)
            {
                lua_rawgeti(luaCLI->L, -2, luaCLI->history_len - CLI_HISTORY_LEN + luaCLI->pos + 1);
                lua_rawseti(luaCLI->L, -2, luaCLI->pos);
            }
            luaCLI->history_len = (int)lua_objlen(luaCLI->L, -1);
            lua_setglobal(luaCLI->L, "cmdLuaCLI_history");
            lua_pop(luaCLI->L, 1);
        }
        lua_getglobal(luaCLI->L, "cmdLuaCLI_history");
        lua_pushlstring(luaCLI->L, luaCLI->line, luaCLI->len);
        lua_rawseti(luaCLI->L, -2, luaCLI->history_len + 1);
        lua_pop(luaCLI->L, 1);
    }

    return key;
}

/**
* @internal luaCLI_readLine function
* @endinternal
*
* @brief   Read line
*/
static int luaCLI_readLine(lua_State *L)
{
    IOStreamPTR IOStream;
    static LUA_CLI_MALLOC_STC *memMgr;
    CPSS_LUACLI_INSTANCE_STC luaCLI;

    cpssOsMemSet(&luaCLI, 0, sizeof(luaCLI));
    luaCLI.prompt = "?";
    luaCLI.L = L;
    luaCLI.state = STATE_READKEY_CLI_E;
    IOStream = (IOStreamPTR)luaCLIEngineGetData(luaCLI.L, "_IOStream", 0);
    memMgr = (LUA_CLI_MALLOC_STC*)luaCLIEngineGetData(luaCLI.L, "_memMgr", 0);
    if (IOStream == NULL || memMgr == NULL)
        return 0;

    luaCLI.line = (char*)memMgr->malloc(CLI_MAX_LINE_LEN);

    if (lua_gettop(luaCLI.L) >= 1 && lua_isstring(luaCLI.L, 1))
    {
        luaCLI.prompt = lua_tostring(luaCLI.L, 1);
    }
    if (lua_gettop(luaCLI.L) >= 2 && lua_toboolean(luaCLI.L, 2))
    {
        luaCLI.flags |= CLI_READLINE_FLAG_HISTORY;
    }
    if (lua_gettop(luaCLI.L) >= 2 && lua_toboolean(luaCLI.L, 3))
    {
        luaCLI.flags |= CLI_READLINE_FLAG_COMPLETION;
    }

    luaCLI_WriteConsoleLine(IOStream, &luaCLI);

    if (luaCLI_ReadKey(IOStream, &luaCLI, -1) < 0)
    {
        memMgr->free(luaCLI.line);
        return 0;
    }
    lua_pushstring(luaCLI.L, luaCLI.line);
    memMgr->free(luaCLI.line);
    return 1;
}


/**
* @internal luaCLIEngine_init function
* @endinternal
*
* @brief   Init Lua CLI
*
* @param[in] IOStream
*                                      memstat     - pointer to memstat structure or NULL
*
* @param[out] luaPtr
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
int luaCLIEngine_init
(
    IN IOStreamPTR              IOStream,
    IN genFS_PTR                fs,
    IN LUA_CLI_GETFUNCBYNAME_T  getFunctionByName,
    lua_State                 **luaPtr
)
{

    lua_State *L;

    if ( *luaPtr!=NULL ) {
        /*In a case of reconnection, need only to update the stream*/
        lua_pushlightuserdata(*luaPtr, IOStream);
        lua_setglobal(*luaPtr, "_IOStream");
        return 0;
    }

#if defined SHARED_MEMORY

    if (osWrapperIsOpen() == GT_FALSE)
    {
       /* Must be called before use any of OS functions. */
        osWrapperOpen(NULL);
    }
#endif

    if (memMgr == NULL)
    {
        memMgr = (LUA_CLI_MALLOC_STC*) osMalloc(sizeof(*memMgr));
        if(NULL == memMgr)
        {
          return GT_FAIL;
        }
    }

#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
    memMgr->malloc = myCpssOsMalloc;
    memMgr->free = myCpssOsFree;
#else
    memMgr->malloc = (LUACLI_MALLOC_T)LUACLI_MALLOC_F;
    memMgr->free = LUACLI_FREE;
#endif

    L = luaCLIEngine_newstate(memMgr, IOStream, fs, getFunctionByName);
    *luaPtr = L;
    if (L == NULL)
    {
        return GT_FAIL;
    }

    lua_register(L, "cmdLuaCLI_readLine", luaCLI_readLine);

    lua_pushstring(L, CLI_PROMPT);
    lua_setglobal(L, "cmdLuaCLI_prompt");

    lua_newtable(L);
    lua_setglobal(L, "cmdLuaCLI_history");

    lua_newtable(L);
    lua_setglobal(L, "CLI_running_Cfg_Tbl");

    /* load libLuaCliDefs.lua */
    lua_getglobal(L, "cmdLuaCLI_require");
    lua_pushstring(L, "cmdLuaCLIDefs");
    lua_call(L, 1, 0);

    /* check if cmdLuaCLIDefs loaded */
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "loaded");
    lua_getfield(L, -1, "cmdLuaCLIDefs");
    if (!lua_isboolean(L, -1) || !lua_toboolean(L, -1))
    {
        lua_getglobal(L, "print");
        lua_pushstring(L, "*** cmdLuaCLIDefs not loaded, type CLIexit to exit luaCLI");
        lua_call(L, 1, 0);
    }
    lua_pop(L, 3);

    /*Don't call cmdLuaCLI_change_mode(): this should be done in cmdLuaCLIDefs */
#if 0
    /* set default mode */
    lua_getglobal(L, "cmdLuaCLI_change_mode");
    if (lua_isfunction(L, -1))
    {
        lua_pushstring(L, "");
        if (lua_pcall(L, 1, 0, 0) != 0)
        {
            lua_pushcfunction(L, luaPrint);
            lua_insert(L, -2);
            lua_pushstring(L, "Failed to execute cmdLuaCLI_change_mode(\"\"): ");
            lua_insert(L, -2);
            lua_concat(L, 2);
            lua_call(L, 1, 0);
        }
    }
    else
    {
        lua_pop(L, 1);
    }
#endif

    return 0;
}

/**
* @internal lua_status_incomplete function
* @endinternal
*
* @param[in] L                        - lua_State
* @param[in] status                   - return  of luaL_loadbuffer()
*                                       1 if status == incomplete
*                                       0 otherwise
*
* @note need to be updated to don't use strstr()
*
*/
static int lua_status_incomplete
(
    lua_State   *L,
    int         status
)
{
    if (status == LUA_ERRSYNTAX)
    {
        size_t lmsg;
        const char *msg = lua_tolstring(L, -1, &lmsg);
        const char *tp = msg + lmsg - (sizeof(LUA_QL("<eof>")) - 1);
        if (strstr(msg, LUA_QL("<eof>")) == tp)
        {
            lua_pop(L, 1);
            return 1;
        }
    }
    return 0;  /* else... */
}

/**
* @internal luaCLIEngineEventInit function
* @endinternal
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS luaCLIEngineEventInit
(
    IN IOStreamPTR              IOStream,
    IN genFS_PTR                fs,
    IN LUA_CLI_GETFUNCBYNAME_T  getFunctionByName,
    IN GT_STATUS (*osShell)(IOStreamPTR IOStream),
    IN lua_State               **L,
    IN char                    **linePtr
)
{
    GT_UNUSED_PARAM(osShell);

    if (IOStream->customPtr != NULL)
    {
        *L = (lua_State *)IOStream->customPtr;
        /* set output stream */
        lua_pushlightuserdata(*L, IOStream);
        lua_setglobal(*L, "_IOStream");

        if (memMgr == NULL)
            memMgr = (LUA_CLI_MALLOC_STC*)luaCLIEngineGetData(*L, "_memMgr", 0);
    }
    else
    {
        GT_U32 time_start_sec, time_start_ns, time_end_sec, time_end_ns;
        long diff_sec, diff_nsec;
        char buf[128];

#if defined SHARED_MEMORY
    (void)luaCLIEngine_initSharedLib();
#endif

        /* start timer */
        cpssOsTimeRT(&time_start_sec, &time_start_ns);

        luaCLIEngine_init(IOStream, fs, getFunctionByName, L);
        if (*L == NULL)
        {
            return GT_FAIL;
        }

        /* stop timer */
        cpssOsTimeRT(&time_end_sec, &time_end_ns);
        if (time_end_ns < time_start_ns)
        {
            time_end_ns += 1000000000;
            time_end_sec--;
        }
        diff_sec = time_end_sec - time_start_sec;
        diff_nsec = time_end_ns - time_start_ns;
        cpssOsSprintf(buf, "Entering LuaCLI took %f msec\n", diff_sec * 1000 + (double)diff_nsec / 1000000);
#ifndef REMOVE_CLI_PRINTS
        IOStream->writeLine(IOStream, buf);

#ifdef CLI_BANNER
        IOStream->writeLine(IOStream, CLI_BANNER);
#endif
#endif
    }

    if (memMgr == NULL)
        return GT_FAIL;

    *linePtr = (char*)memMgr->malloc(CLI_MAX_LINE_LEN);

    return GT_OK;
}



/**
* @internal luaCLIEngine_Event function
* @endinternal
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS luaCLIEngine_Event
(
    IN IOStreamPTR              IOStream,
    IN GT_STATUS (*osShell)(IOStreamPTR IOStream),
    CPSS_LUACLI_INSTANCE_STC   *luaCLI,
    IN int                      timeout
)
{
    int status;
    int lua_line_idx = 0;
    int key;

    /* accept and handle input as long as connected */
    if (IOStream->connected(IOStream))
    {
        while(GT_TRUE) {
            switch ( luaCLI->state ) {
            case STATE_WRITE_CONSOLE_CLI_E:
                    luaCLI->prompt = NULL;
                    luaCLI->flags = CLI_READLINE_FLAG_HISTORY | CLI_READLINE_FLAG_COMPLETION;
                    luaCLI_WriteConsoleLine(IOStream, luaCLI);
                    luaCLI->state = STATE_READKEY_CLI_E;
                    GT_ATTR_FALLTHROUGH;
            case STATE_READKEY_CLI_E:
                    key = luaCLI_ReadKey(IOStream, luaCLI, timeout);

                    if ( luaCLI->len < 0 )
                    {
                        /* input failed, possible connection failed, don't terminate the luaCLI instance- keep it on hold*/
                        memMgr->free(luaCLI->line);
                        luaCLI->line = NULL;
                        return GT_NO_RESOURCE;
                    }

                    if ( key == KEY_ENTER && !luaCLI->line[0] ){
                        luaCLI->state = STATE_WRITE_CONSOLE_CLI_E;
                        continue;
                    }

                    else if ( key == 0 )
                        return GT_OK;
                    if (IS_CLI_COMMENT(luaCLI->line))
                    { /* in case comment ("//") entered from CLI, state machine writes new "#Console" line */
                        luaCLI->state = STATE_WRITE_CONSOLE_CLI_E;
                        continue;
                    }
                    if (IS_CLI_CMD_EXIT(luaCLI->line))
                    {
                        goto closeSessionLabel;
                    }
                    if (IS_CLI_SWITCH_MODE_TO_DX(luaCLI->line))
                    {
                        /* restart Lua in DX mode (if exists) */
                        luaRestartRequest = 1;
                        luaModeNum = 0;
                        luaCLI->state = STATE_WRITE_CONSOLE_CLI_E;
                        goto closeSessionLabel;
                    }
                    if (IS_CLI_SWITCH_MODE_TO_PX(luaCLI->line))
                    {
                        /* restart Lua in PX mode (if exists) */
                        luaRestartRequest = 1;
                        luaModeNum = 1;
                        luaCLI->state = STATE_WRITE_CONSOLE_CLI_E;
                        goto closeSessionLabel;
                    }

                    /* implemented in mainCmd only */
                    if (IS_CLI_CMD_OSSHELL(luaCLI->line) && osShell != NULL)
                    {
                        osShell(IOStream);
                        luaCLI->state = STATE_WRITE_CONSOLE_CLI_E;
                        continue;
                    }
        #ifdef  LUACLI_MEMSTAT
                    if (IS_CLI_CMD_MEMSTAT(luaCLI->line))
                    {
                        cpssOsSprintf(luaCLI->line, "Memory statistic:\n\ttotalUsed=%d\n\tnChunks=%d\n",
                                memMgr->totalUsed, memMgr->nChunks);
                        IOStream->writeLine(IOStream, luaCLI->line);
                        luaCLI->state = STATE_WRITE_CONSOLE_CLI_E;
                        continue;
                    }
        #endif /* LUACLI_MEMSTAT */
                    if (IS_CLI_CMD_LUA(luaCLI->line))
                    {
                        lua_pushliteral(luaCLI->L, "");
                        lua_line_idx = lua_gettop(luaCLI->L);
                        luaCLI->state = STATE_LUA_E;
                        continue;
                    }
                    /* execute line */
                    lua_getglobal(luaCLI->L, "cmdLuaCLI_execute");
                    if (lua_isfunction(luaCLI->L, -1))
                    {
                        int status;
                        lua_pushstring(luaCLI->L, luaCLI->line);
                        status = lua_pcall(luaCLI->L, 1, 0, 0);
                        if (status && !lua_isnil(luaCLI->L, -1))
                        {
                            /* any error while loading or executing */
                            const char *msg = lua_tostring(luaCLI->L, -1);
                            if (msg == NULL) msg = "(error object is not a string)";
                            IOStream->writeLine(IOStream, "Lua error: ");
                            IOStream->writeLine(IOStream, msg);
                            IOStream->writeLine(IOStream, "\n");
                            lua_pop(luaCLI->L, 1);
                        }
                    }
                    else
                    {
                        lua_pop(luaCLI->L, 1);
                    }
                    luaCLI->state = STATE_WRITE_CONSOLE_CLI_E;
                    continue;
            case STATE_LUA_E:
                    luaCLI->prompt = lua_strlen(luaCLI->L, -1) == 0 ? LUA_PROMPT : LUA_PROMPT_CONT;
                    luaCLI->flags = CLI_READLINE_FLAG_HISTORY;
                    luaCLI_WriteConsoleLine(IOStream, luaCLI);
                    key = luaCLI_ReadKey(IOStream, luaCLI, -1);
                    if ( luaCLI->len < 0)
                    {
                        /* input faled, possible connection failed */
                        goto closeSessionLabel;
                    }

                    if (luaCLI->line[0] == '.' && luaCLI->line[1] == 0)
                    {
                        /* single dot at line, end lua mode */
                        lua_settop(luaCLI->L, lua_line_idx);
                        lua_pop(luaCLI->L, 1);
                        luaCLI->state = STATE_WRITE_CONSOLE_CLI_E;
                        continue;
                    }
                    if (lua_strlen(luaCLI->L, -1) > 0)
                    {
                        lua_pushliteral(luaCLI->L, "\n");
                        lua_concat(luaCLI->L, 2);
                    }
                    lua_pushstring(luaCLI->L, luaCLI->line);
                    lua_concat(luaCLI->L, 2);
                    status = luaL_loadbuffer(luaCLI->L,
                                lua_tostring(luaCLI->L, lua_line_idx),
                                lua_strlen(luaCLI->L, lua_line_idx),
                                "=stdin");
                    if (lua_status_incomplete(luaCLI->L, status))
                    {
                        /* line incomplete, do next line */
                        continue;
                    }
                    if (status == 0)
                    {
                        status = lua_pcall(luaCLI->L, 0, 0, 0);
                    }
                    if (status && !lua_isnil(luaCLI->L, -1))
                    {
                        /* any error while loading or executing */
                        const char *msg = lua_tostring(luaCLI->L, -1);
                        if (msg == NULL) msg = "(error object is not a string)";
                        IOStream->writeLine(IOStream, "Lua error: ");
                        IOStream->writeLine(IOStream, msg);
                        IOStream->writeLine(IOStream, "\n");
                    }

                    lua_settop(luaCLI->L, lua_line_idx);
                    lua_pop(luaCLI->L, 1);
                    lua_pushliteral(luaCLI->L, "");
                    continue;
            }
            return GT_OK;
        }
    }

closeSessionLabel:
    memMgr->free(luaCLI->line);

    /* exit calling function */
    lua_getglobal(luaCLI->L, "cmdLuaCLI_atexit");
    if (lua_isfunction(luaCLI->L, -1))
    {
        lua_call(luaCLI->L, 0, 0);
    }

    lua_close(luaCLI->L);
    return GT_FAIL; /* close session */
}

#if defined SHARED_MEMORY
GT_STATUS luaCLIEngine_initSharedLib
(
    void
)
{
    GT_STATUS rc;
    GT_BOOL cpssAslrSupport = cpssNonSharedGlobalVars.nonVolatileDb.generalVars.aslrSupport;
    CPSS_EXT_DRV_FUNC_BIND_STC  extDrvFuncBindInfo;
    CPSS_OS_FUNC_BIND_STC       osFuncBind;
    CPSS_TRACE_FUNC_BIND_STC    traceFuncBindInfo;
    if((GT_TRUE == cpssAslrSupport)&&(GT_FALSE == prvCpssGlobalDbExistGet()))
    {
        /*
                    We must be LUA stand alone context if we got here.
                    So need to load shared memory and initialize callbacks.
                 */

        /* Shared memory approach on ARM should use dlsym-based workaround for pointers to functions */
        rc = shrMemGetDefaultOsBindFuncsThruDynamicLoader(&osFuncBind);
        if (rc != GT_OK)
        {
          return GT_FAIL;
        }
        /* Shared memory approach on ARM should use dlsym-based workaround for pointers to functions */
        rc = shrMemGetDefaultExtDrvFuncs(&extDrvFuncBindInfo);
        if (rc != GT_OK)
        {
          return GT_FAIL;
        }

       /* use NULL pointer to avoid trash stored by cpssExtServicesBind.
                The cpssExtServicesBind will use default "not implemented" callbacks in this case.  */
        osMemSet(&traceFuncBindInfo, 0, sizeof(traceFuncBindInfo));

        rc = cpssExtServicesBind(&extDrvFuncBindInfo, &osFuncBind, &traceFuncBindInfo);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = cpssGlobalDbInit(GT_TRUE,GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

#endif
