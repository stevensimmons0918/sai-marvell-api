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
* @file cmdLuaCLI.c
*
* @brief a LUA CLI implementation
*
* @version   37
********************************************************************************
*/

#include <lua.h>
#include <extUtils/luaCLI/luaCLIEngineCLI.h>

#if !defined(ENV_MAINCMD) && !defined(ENV_POSIX)
#  error "No layer defined. Define one of ENV_MAINCMD, ENV_POSIX"
#endif

/***** Include files ***************************************************/

#ifdef ENV_MAINCMD
# include "cmdLua_os_mainCmd.inc"
#endif
#ifdef ENV_POSIX
# include "cmdLua_os_posix.inc"
#endif

#ifdef ENV_MAINCMD
#include <cmdShell/FS/cmdFS.h>
#include <cmdShell/os/cmdShell.h>
#else
#include <cmdFS.h>
#define cmdOsShell NULL
#endif

#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
static GT_VOID *myCpssOsMalloc
(
    IN int size
)
{
    return LUACLI_MALLOC_F(size);
    /*return cpssOsMalloc_MemoryLeakageDbg(size,__FILE__,__LINE__);*/
}
static GT_VOID myCpssOsFree
(
    IN GT_VOID* const memblock
)
{
    LUACLI_FREE(memblock);
    /*cpssOsFree_MemoryLeakageDbg(memblock,__FILE__,__LINE__);*/
}
#endif /*OS_MALLOC_MEMORY_LEAKAGE_DBG*/

/**
* @internal luaCLI_EventLoop function
* @endinternal
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
int luaCLI_EventLoop
(
    IOStreamPTR IOStream
)
{
    GT_STATUS       rc;
    CPSS_LUACLI_INSTANCE_STC luaCLI;

    cmdOsMemSet(&luaCLI, 0, sizeof(luaCLI));

    while(GT_TRUE){
        /* reset global request to restart lua */
        luaRestartRequest = 0;

        rc = luaCLIEngineEventInit(IOStream, &cmdFS, osShellGetFunctionByName,
                                    cmdOsShell, &(luaCLI.L), &(luaCLI.line));

        if ( rc != GT_OK )
            return rc;

        do {
            rc = luaCLIEngine_Event(IOStream, cmdOsShell, &luaCLI, -1);
        } while ((rc == GT_OK) && (luaRestartRequest == 0));

        if ( luaRestartRequest == 1 )
            continue;
        else
            break;
    }

    return rc;
}


