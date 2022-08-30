/*************************************************************************
* luaCLIEngine.h
*
* DESCRIPTION:
*       luaCLI engine
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
**************************************************************************/

#ifndef __luaCLIEngineCLI_h__
#define __luaCLIEngineCLI_h__

/***** Includes  ********************************************************/
#include <extUtils/luaCLI/luaCLIEngine.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern LUA_CLI_MALLOC_STC *memMgr;
extern GT_U32 luaRestartRequest;
extern GT_U32 luaModeNum;

typedef enum { STATE_WRITE_CONSOLE_CLI_E, STATE_READKEY_CLI_E, STATE_LUA_E } CONSOLE_STATE;

/**
 * @struct CPSS_LUACLI_INSTANCE_STC
 *
 * @brief LUA CLI instance structure
*/
typedef struct{
    lua_State       *L;
    const char      *prompt;
    int             flags;
    char            *line;
    CONSOLE_STATE   state;
    int             history_len;
    int             history_pos;
    int             len;
    int             pos;
} CPSS_LUACLI_INSTANCE_STC;

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
);

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
);

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
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __luaCLIEngineCLI_h__ */

