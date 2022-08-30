/*******************************************************************************
*              (c), Copyright 2015, Marvell International Ltd.                 *
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
* @file luaCLIEngine_C_API.c
*
* @brief a C API for LUA CLI Engine
*
* @version   1
********************************************************************************
*/

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <extUtils/luaCLI/luaCLIEngine_C_API.h>
#include <extUtils/luaCLI/luaCLIEngineCLI.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

typedef struct outBufferStreamSTCT {
    IOStreamSTC common;
    char        *buf;
    GT_U32      len;
    GT_U32      curr;
} outBufferStreamSTC;
static int outBufferStreamDestroy(IOStreamPTR stream GT_UNUSED)
{
    return 0;
}
static int outBufferStreamRead(IOStreamPTR stream GT_UNUSED, void* bufferPtr GT_UNUSED, int bufferLength GT_UNUSED)
{
    return 0;
}
static int outBufferStreamReadChar(IOStreamPTR stream GT_UNUSED, char* charPtr GT_UNUSED, int timeOut GT_UNUSED)
{
    return -1;
}
static int outBufferStreamReadLine(IOStreamPTR stream GT_UNUSED, char* bufferPtr GT_UNUSED, int bufferLength GT_UNUSED)
{
    return 0;
}
static int outBufferStreamWrite(IOStreamPTR stream, const void* bufferPtr, int bufferLength)
{
    return stream->writeBuf(stream, (const char*)bufferPtr, bufferLength);
}
static int outBufferStreamWriteBuf(struct IOStreamSTC* stream, const char* s, int len)
{
    outBufferStreamSTC *strm = (outBufferStreamSTC*)stream;
    int cnt;
    for (cnt = 0; (cnt < len) && (strm->curr < strm->len); cnt++)
    {
        strm->buf[strm->curr++] = s[cnt];
    }
    return cnt;
}
static int outBufferStreamWriteLine(IOStreamPTR stream GT_UNUSED, const char *s)
{
    return stream->writeBuf(stream, s, (int)cpssOsStrlen(s));
}
static int outBufferStreamConnected(IOStreamPTR stream GT_UNUSED)
{
    return 1;
}
static int outBufferStreamSetTtyMode(IOStreamPTR stream GT_UNUSED, int mode GT_UNUSED)
{
    return 0;
}

/**
* @internal outBufferStreamInit function
* @endinternal
*
* @brief   Initialize outBufferStreamSTC
*
* @retval stream                   - destination stream
*/
static void outBufferStreamInit(
    outBufferStreamSTC *stream,
    char               *buf,
    GT_U32              len
)
{
    stream->common.destroy = outBufferStreamDestroy;
    stream->common.read = outBufferStreamRead;
    stream->common.readChar = outBufferStreamReadChar;
    stream->common.readLine = outBufferStreamReadLine;
    stream->common.write = outBufferStreamWrite;
    stream->common.writeBuf = outBufferStreamWriteBuf;
    stream->common.writeLine = outBufferStreamWriteLine;
    stream->common.connected = outBufferStreamConnected;
    stream->common.grabStd = NULL;
    stream->common.setTtyMode = outBufferStreamSetTtyMode;
    stream->common.getFd = NULL;
    stream->common.isConsole = GT_FALSE;
    stream->common.flags =
        IO_STREAM_FLAG_O_CRLF |
        IO_STREAM_FLAG_I_CANON |
        IO_STREAM_FLAG_I_ECHO;
    stream->common.wasCR = GT_FALSE;
    stream->buf = buf;
    stream->len = len;
    stream->curr = 0;
}


#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
static GT_VOID *myCpssOsMalloc
(
    IN int size
)
{
    return cpssOsMalloc_MemoryLeakageDbg(size,__FILE__,__LINE__);
}
static GT_VOID myCpssOsFree
(
    IN GT_VOID* const memblock
)
{
    cpssOsFree_MemoryLeakageDbg(memblock,__FILE__,__LINE__);
}
#endif /*OS_MALLOC_MEMORY_LEAKAGE_DBG*/


typedef void* (*LUACLI_MALLOC_T)(int size);
/**
* @internal luaCLIEngineNewInstance function
* @endinternal
*
* @brief   Init new Lua state, enable minimal functionality
*         Should be used to create luaCLI and for new Lua task
* @param[in] fs                       - FS API pointer (optional)
*                                      This file system should be implemented
*                                      to allow Lua load .lua scripts
*                                      Also it is used by scripts to store/load files
*                                      If this parameter is ommited (NULL) then
*                                      Lua will try to open files using fopen()/fread()...
* @param[in] getFunctionByName        - (optional)
*                                      This function is used to convert function name
*                                      to pointer.
*                                      Marvell's luaCLI implementation (the complete set
*                                      of Lua scripts) widely use it to don't create
*                                      wrapper for each CPSS API
*
* @param[out] luaPtr                   - Lua instance
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS luaCLIEngineNewInstance
(
    IN  genFS_PTR                fs,
    IN  LUA_CLI_GETFUNCBYNAME_T  getFunctionByName,
    OUT lua_State              **luaPtr
)
{
    lua_State          *L = NULL;
    int                 rc;

#ifdef LUACLI_MEMSTAT
    memMgr->totalUsed = 0;
    memMgr->nChunks = 0;
#endif /* LUACLI_MEMSTAT */
    /*TODO: IOStream */
    rc = luaCLIEngine_init(NULL, fs, getFunctionByName, &L);
    if (L == NULL && rc != GT_OK)
        rc = GT_FAIL;
    if (rc != GT_OK)
    {
        cpssOsFree(memMgr);
        return rc;
    }
    *luaPtr = L;

    return GT_OK;
}

/**
* @internal luaCLIEngineDestroy function
* @endinternal
*
* @brief   Destroy luaCLI instance
*
* @param[in] L                        - Lua instance
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS luaCLIEngineDestroy
(
    IN lua_State    *L
)
{
    LUA_CLI_MALLOC_STC *memMgr;

    if (!L)
        return GT_BAD_PARAM;

    memMgr = (LUA_CLI_MALLOC_STC*)luaCLIEngineGetData(L, "_memMgr", 0);

    lua_close(L);
    if (memMgr != NULL)
    {
        cpssOsFree(memMgr);
    }
    return GT_OK;
}

/**
* @internal luaCLIEngineExec function
* @endinternal
*
* @brief   Execute luaCLI command
*
* @param[in] L                        - Lua instance
* @param[in] cmd                      - luaCLI command
*                                      For example: "show version"
* @param[in,out] outBufferLength          - output buffer length
*
* @param[out] outBuffer                - output buffer
* @param[in,out] outBufferLength          - output buffer length
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS luaCLIEngineExec
(
    IN  lua_State  *L,
    IN  const char *cmd,
    OUT char       *outBuffer,
    INOUT GT_U32   *outBufferLength
)
{
    GT_STATUS   rc = GT_OK;
    /* initialize output stream instance */
    outBufferStreamSTC  streamI;
    IOStreamPTR stream = (IOStreamPTR)(&streamI);
    outBufferStreamInit(&streamI, outBuffer, *outBufferLength);

    /* set output to buffer */
    lua_pushlightuserdata(L, stream);
    lua_setglobal(L, "_IOStream");

    /* execute command */
    lua_getglobal(L, "cmdLuaCLI_execute");
    if (lua_isfunction(L, -1))
    {
        int status;
        lua_pushstring(L, cmd);
        status = lua_pcall(L, 1, 1, 0);
        if (status && !lua_isnil(L, -1))
        {
            /* any error while loading or executing */
            const char *msg = lua_tostring(L, -1);
            if (msg == NULL) msg = "(error object is not a string)";
            stream->writeLine(stream, "Lua error: ");
            stream->writeLine(stream, msg);
            stream->writeLine(stream, "\n");
            lua_pop(L, 1);
            rc = GT_FAIL;
        }
        if (!status)
        {
            if (lua_isboolean(L, -1))
                rc = lua_toboolean(L, -1) ? GT_OK : GT_FAIL;
            lua_pop(L, 1);
        }
    }
    else
    {
        lua_pop(L, 1);
        stream->writeLine(stream, "cmdLuaCLI_execute() not defined");
        rc = GT_FAIL;
    }

    /* restore output to null */
    lua_pushlightuserdata(L, NULL);
    lua_setglobal(L, "_IOStream");
    *outBufferLength = streamI.curr;

    return rc;
}

/**
* @internal luaCLIEngineDoLua function
* @endinternal
*
* @brief   Execute lua script
*
* @param[in] L                        - Lua instance
* @param[in] luaScript                - lua script
*                                      For example: "print(to_string({1,2,3}))"
* @param[in,out] outBufferLength          - output buffer length
*
* @param[out] outBuffer                - output buffer
* @param[in,out] outBufferLength          - output buffer length
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS luaCLIEngineDoLua
(
    IN  lua_State  *L,
    IN  const char *luaScript,
    OUT char       *outBuffer,
    INOUT GT_U32   *outBufferLength
)
{
    GT_STATUS   rc = GT_OK;
    int         status, top;
    /* initialize output stream instance */
    outBufferStreamSTC  streamI;
    IOStreamPTR stream = (IOStreamPTR)(&streamI);
    outBufferStreamInit(&streamI, outBuffer, *outBufferLength);

    /* set output to buffer */
    lua_pushlightuserdata(L, stream);
    lua_setglobal(L, "_IOStream");

    /* execute command */
    top = lua_gettop(L);
    status = luaL_loadbuffer(L, luaScript, cpssOsStrlen(luaScript), "=luaScript");
    if (status == 0)
    {
        status = lua_pcall(L, 0, 0, 0);
    }
    if (status && !lua_isnil(L, -1))
    {
        /* any error while loading or executing */
        const char *msg = lua_tostring(L, -1);
        if (msg == NULL) msg = "(error object is not a string)";
        stream->writeLine(stream, "Lua error: ");
        stream->writeLine(stream, msg);
        stream->writeLine(stream, "\n");
        rc = GT_FAIL;
    }
    lua_settop(L, top);

    /* restore output to null */
    lua_pushlightuserdata(L, NULL);
    lua_setglobal(L, "_IOStream");
    *outBufferLength = streamI.curr;

    return rc;
}


/**
* @internal luaCLIEngineLoadModule function
* @endinternal
*
* @brief   Load new luaCLI module
*         This command executes require("module_name")
*         module_name.lua must be in genFS
* @param[in] L                        - Lua instance
* @param[in] moduleName               - Module name (w/o .lua extension)
* @param[in,out] outBufferLength          - output buffer length
*
* @param[out] outBuffer                - output buffer
* @param[in,out] outBufferLength          - output buffer length
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note
*       EXAMPLE:
*       luaCLIEngineLoadModule(L, "examples/run", NULL, 0);
*
*/
GT_STATUS luaCLIEngineLoadModule
(
    IN  lua_State  *L,
    IN  const char *moduleName,
    OUT char       *outBuffer,
    INOUT GT_U32   *outBufferLength
)
{
    GT_STATUS rc;
    lua_pushfstring(L, "require(\"%s\")", moduleName);
    rc = luaCLIEngineDoLua(L, lua_tostring(L, -1), outBuffer, outBufferLength);
    lua_pop(L, 1);
    return rc;
}

