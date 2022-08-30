/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssCommon.c
*
* DESCRIPTION:
*       Provides wrappers for register access from lua and device number validation
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 6 $
*******************************************************************************/


#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/prvCpssDrvObj.h>
#include <cpss/common/config/cpssGenCfg.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpssCommon/wraplCpssDeviceInfo.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>

#include <stdlib.h>
#include <gtOs/gtOsTimer.h>
#include <lua.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


extern void cpssInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
);

int check_wraplCpssCommon()
{
    cpssOsPrintf("File wraplCpssCommon.c inside");
    return 1;
}

int wrlIsDevIdValid(
    lua_State *L
)
{
    lua_pushboolean(L,
            (int) prvCpssIsDevExists((GT_U8) lua_tointeger(L, 1)));
    return 1;
}

int wrlDevList(
    lua_State *L
)
{
    GT_U8 devId = 0xFF;
    int c = 0;

    lua_newtable(L);

    while (GT_OK == prvWrlDevFilterNextDevGet(devId, &devId))
    {
        lua_pushnumber(L, devId);
        lua_rawseti(L, -2, ++c);
    }
    return 1;
}

int wrlReadRegister(
    lua_State *L
)
{
    GT_U32      regData;
    GT_STATUS   rc;

    rc = prvCpssDrvHwPpReadRegister(
        (GT_8)lua_tointeger(L, 1),
        (GT_U32)lua_tointeger(L, 2),
        &regData);

    if (rc != GT_OK)
    {
        lua_pushfstring(L, "%d", rc);
        return 1;
    }

    lua_pushnumber(L, regData);
    return 1;
}

int wrlWriteRegister(
    lua_State *L
)
{
    GT_STATUS   rc;

    rc = prvCpssDrvHwPpWriteRegister(
        (GT_8)lua_tointeger(L, 1),
        (GT_U32)lua_tointeger(L, 2),
        (GT_U32)lua_tointeger(L, 3));

    lua_pushnumber(L, rc);
    return 1;
}
int wrlGetSystemTime(
    lua_State *L
)
{
    GT_U32  seconds;
    GT_U32  nanoseconds;
    lua_Number ret;

    if (osTimeRT(&seconds,&nanoseconds)!=GT_OK)
        lua_pushnil(L);
    else
    {
        ret =seconds+nanoseconds/1000000000.;
            lua_pushnumber(L,ret);
    }

    return 1;
}

/*******************************************************************************
* luaCLI_getTraceDBLine
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Reads a line from the trace database, either read or write and returns the
*       values to lua
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* INPUTS:
*       L                   - lua state (in stack should be index of line and
*                              accessType for read or write)
*
* OUTPUTS:
*       1 Address of the write/read operation, nil if failed
*       2 Data of the write/read operation, nil if failed
*       3 The bitmap of the write/read operation, nil if failed
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int luaCLI_getTraceDBLine(lua_State *L)
{
#ifndef CPSS_APP_PLATFORM
    GT_U32 index;
    OUT APP_DEMO_CPSS_HW_ACCESS_DB_STC dbLine;

    if ((lua_gettop(L)==1) &&  (lua_isnumber(L, 1)))
    {
        index = (GT_U32) lua_tointeger(L, 1);
    }
    else
    {
            lua_pushnil(L);
            return 1;
    }
    if (appDemoTraceHwAccessDbLine(index,&dbLine)!=GT_OK)
    {
            lua_pushnil(L);
            return 1;
    }

    lua_pushnumber(L,dbLine.addr);
    lua_pushnumber(L,dbLine.mask);
    lua_pushnumber(L,dbLine.data);
    lua_pushnumber(L,dbLine.accessParamsBmp);

/*
    if (data_high == 0)
        lua_pushnumber(L, data);
    else
    {
        char hexstring[20];
        cpssOsSprintf(hexstring, "0x%08x%08x", data_high, data);
        lua_pushstring(L, hexstring);
    }*/
    return 4;
#else
    lua_pushnil(L);
    return 1;
#endif
}

extern GT_STATUS   osShellExecute(IN  char*   command);
/*******************************************************************************
* luaShellExecute
*
* DESCRIPTION:
*       allow to call any 'C' function from LUA.
*
* INPUTS:
*       L       - lua_State
*       [1] - the string to operate the command + it's parameters
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0 - error or function not found
*       1 - function was called
*
* COMMENTS:
*
*******************************************************************************/
int luaShellExecute(lua_State *L)
{
#ifndef CLI_STANDALONE
    const char      *commmand_and_params;
    char      *non_const_commmand_and_params;
    size_t          len;
    GT_STATUS       rc;

    if (! lua_isstring(L, 1))
    {
        /*error*/
        return 0;
    }

    commmand_and_params = lua_tolstring(L, 1, &len);

    cpssOsPrintf("luaShellExecute: %s \n",commmand_and_params);

    non_const_commmand_and_params = (char*)commmand_and_params;

    rc = osShellExecute(non_const_commmand_and_params);

    lua_pushinteger(L, rc);
    return 1;
#else
    GT_UNUSED_PARAM(L);
    return 0;
#endif /*CLI_STANDALONE*/
}

#ifndef CLI_STANDALONE
/*******************************************************************************
* luaConsoleClearScreen
*
* DESCRIPTION:
*       'clear screen' of the 'console application'
*
* INPUTS:
*       L       - lua_State
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0 - ok
*      -1 - on error
*
* COMMENTS:
*
*******************************************************************************/
int luaConsoleClearScreen(lua_State *L)
{
    GT_UNUSED_PARAM(L);

    #ifdef _WIN32
        system("cls");
    #else
     {
        int rc;
        rc = system("clear");
        if (rc == -1)
        {
            return rc;
        }
     }
    #endif

    return 0;
}

static unsigned __TASKCONV myLuaThread
(
    IN GT_VOID * inputString
)
{
    GT_STATUS rc;
    char *non_const_commmand_and_params = (char *)inputString;

    cpssOsPrintf("STARTED (parallel run of): %s \n",non_const_commmand_and_params);

    rc = osShellExecute(non_const_commmand_and_params);
    if (rc != GT_OK)
    {
        cpssOsPrintf("ERROR : non exist function : %s \n",non_const_commmand_and_params);
    }

    cpssOsPrintf("ENDED (parallel run of): %s \n",non_const_commmand_and_params);

    cpssOsFree(non_const_commmand_and_params);

    return 0;
}

static GT_STATUS createMyThread(const char      *commmand_and_params)
{
    static GT_U32   threadId = 0;
    GT_TASK tid;
    static char myName[30];
    char *non_const_commmand_and_params;

    non_const_commmand_and_params = cpssOsMalloc(cpssOsStrlen(commmand_and_params) + 1);
    if(non_const_commmand_and_params == NULL)
    {
        cpssOsPrintf("createMyThread : failed no alloc [%d] bytes \n",cpssOsStrlen(commmand_and_params));
        return GT_FAIL;
    }

    cpssOsStrCpy(non_const_commmand_and_params,commmand_and_params);

    threadId++;

    cpssOsSprintf(myName,"luaParalel_%d",threadId);

    if(osTaskCreate(myName, 0 , 0x2000,
        myLuaThread,(GT_VOID*)non_const_commmand_and_params, &tid) != GT_OK)
    {
        return GT_FAIL;
    }

    return GT_OK;
}
#endif /*!CLI_STANDALONE*/

/*******************************************************************************
* luaShellExecute_inOtherThread
*
* DESCRIPTION:
*       allow to call any 'C' function from LUA , but called in new dedicated
*       thread for this operation
*
* INPUTS:
*       L       - lua_State
*       [1] - the string to operate the command + it's parameters
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0 - error or function not found
*       1 - function was called
*
* COMMENTS:
*
*******************************************************************************/
int luaShellExecute_inOtherThread(lua_State *L)
{
#ifndef CLI_STANDALONE
    GT_STATUS rc;
    const char      *commmand_and_params;
    size_t          len;

    if (! lua_isstring(L, 1))
    {
        /*error*/
        return 0;
    }

    commmand_and_params = lua_tolstring(L, 1, &len);

    rc = createMyThread(commmand_and_params);
    return (rc == GT_OK) ? 1 : 0;
#else
    GT_UNUSED_PARAM(L);
    return 0;
#endif /*CLI_STANDALONE*/
}

/*******************************************************************************
* wrlCpssInitSystemGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting info about the cpssInitSystem parameters that used when called.
*
* APPLICABLE DEVICES:
*        All.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       L                     - lua state
*
* OUTPUTS:
*       3 parameters pushed to lua stack
*       index 0 - boardIdx
*       index 1 - boardRevId
*       index 2 - reloadEeprom
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssInitSystemGet(lua_State *L)
{
#if !defined (CLI_STANDALONE)
    GT_U32  boardIdx;
    GT_U32  boardRevId;
    GT_U32  reloadEeprom;
#if !defined(CPSS_APP_PLATFORM)
    cpssInitSystemGet(&boardIdx,&boardRevId,&reloadEeprom);
#else
    boardIdx = 29;
    boardRevId = 1;
    reloadEeprom = 0;
#endif
    lua_pushnumber(L, boardIdx      );
    lua_pushnumber(L, boardRevId    );
    lua_pushnumber(L, reloadEeprom  );
    return 3;
#else
    GT_UNUSED_PARAM(L);
    return 0;
#endif /*CLI_STANDALONE*/
}

/* check if the EZ_BRINGUP is part of the compilation */
int luaIsInclude_EZ_BRINGUP(lua_State *L)
{
    #ifdef EZ_BRINGUP
        lua_pushnumber(L, 1);
    #else
        lua_pushnumber(L, 0);
    #endif
    return 1;
}

/* define stub for not implemented function that used by LUA by 'require/safeRequire' */
#define LUA_WRAP_STUB(wrFunc)   \
    int wrFunc(lua_State *L){L=L;return 0;}

#ifndef INCLUDE_MPD
/*currently next need stub*/
LUA_WRAP_STUB(wr_utils_pdl_inband_get_phy_post_init_info)
LUA_WRAP_STUB(wr_utils_mpd_get_mdix_oper)
LUA_WRAP_STUB(wr_utils_mpd_get_ext_vct_params)
LUA_WRAP_STUB(wr_utils_mpd_get_cable_len)
LUA_WRAP_STUB(wr_utils_mpd_get_autoneg_remote_capabilities)
LUA_WRAP_STUB(wr_utils_mpd_get_link_partner_pause_capable)
LUA_WRAP_STUB(wr_utils_mpd_get_internal_oper_status)
LUA_WRAP_STUB(wr_utils_mpd_get_dte_status)
LUA_WRAP_STUB(wr_utils_mpd_get_temperature)
LUA_WRAP_STUB(wr_utils_mpd_get_revision)
LUA_WRAP_STUB(wr_utils_mpd_set_autoneg_enable)
LUA_WRAP_STUB(wr_utils_mpd_set_autoneg_disable)
LUA_WRAP_STUB(wr_utils_mpd_set_duplex_mode)
LUA_WRAP_STUB(wr_utils_mpd_set_speed)
LUA_WRAP_STUB(wr_utils_mpd_set_combo_media_type)
LUA_WRAP_STUB(wr_utils_mpd_set_mdix_mode)
LUA_WRAP_STUB(wr_utils_mpd_set_vct_test)
LUA_WRAP_STUB(wr_utils_mpd_set_reset_phy)
LUA_WRAP_STUB(wr_utils_mpd_set_phy_disable_oper)
LUA_WRAP_STUB(wr_utils_mpd_set_advertise_fc)
LUA_WRAP_STUB(wr_utils_mpd_set_present_notification)
LUA_WRAP_STUB(wr_utils_mpd_set_smi_access)
LUA_WRAP_STUB(wr_utils_mpd_set_xsmi_access)
LUA_WRAP_STUB(wr_utils_mpd_set_loop_back)
LUA_WRAP_STUB(wr_utils_mpd_set_check_link_up)

LUA_WRAP_STUB(wr_utils_mpd_get_mdix_admin)
LUA_WRAP_STUB(wr_utils_mpd_get_autoneg_admin)
LUA_WRAP_STUB(wr_utils_mpd_get_green_power_consumption)
LUA_WRAP_STUB(wr_utils_mpd_get_green_readiness)
LUA_WRAP_STUB(wr_utils_mpd_get_cable_len_no_range)
LUA_WRAP_STUB(wr_utils_mpd_get_phy_kind_and_media)
LUA_WRAP_STUB(wr_utils_mpd_get_eee_capability)
LUA_WRAP_STUB(wr_utils_mpd_get_vct_capability)
LUA_WRAP_STUB(wr_utils_mpd_get_autoneg_support)
LUA_WRAP_STUB(wr_utils_mpd_set_restart_autoneg)
LUA_WRAP_STUB(wr_utils_mpd_set_power_modules)
LUA_WRAP_STUB(wr_utils_mpd_set_eee_advertize_capability)
LUA_WRAP_STUB(wr_utils_mpd_set_eee_master_enable)
LUA_WRAP_STUB(wr_utils_mpd_set_lpi_exit_time)
LUA_WRAP_STUB(wr_utils_mpd_set_lpi_enter_time)
LUA_WRAP_STUB(wr_utils_mpd_print_phy_db)
LUA_WRAP_STUB(wr_utils_mpd_print_phy_db_full)
LUA_WRAP_STUB(wr_utils_mpd_mdio_c145_write)
LUA_WRAP_STUB(wr_utils_mpd_mdio_c145_read)
LUA_WRAP_STUB(wr_utils_mpd_mdio_c122_write)
LUA_WRAP_STUB(wr_utils_mpd_mdio_c122_read)
LUA_WRAP_STUB(wr_utils_mpd_mtd_debug_reg_dump)
#endif /*!INCLUDE_MPD*/
