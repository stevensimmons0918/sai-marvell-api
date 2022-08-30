/*******************************************************************************
 *              (c), Copyright 2018, Marvell International Ltd.                 *
 * THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
 * NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
 * OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
 * DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
 * THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
 * IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
 ********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <lua.h>

#include "wraplPdl.h"

#include "pdl/button/pdlBtn.h"
#include "iDbgPdl/button/iDbgPdlBtn.h"
#include "iDbgPdl/utils/iDbgPdlUtils.h"
#include "iDbgPdl/logger/iDbgPdlLogger.h"

/*************** UTILS ************************/

typedef enum {
        PDL_BTN_UTILS_CMD_SET_INFO_E = 0,
        PDL_BTN_UTILS_CMD_SET_DEBUG_E
} PDL_BTN_UTILS_CMD_SET_ENT;

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_btn_get
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_btn_get(lua_State *L)
{
        int btnId;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                btnId = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 1) {
                        btnId = lua_tointeger(L, 1);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }
        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_BUTTON_E, result, iDbgPdlBtnGetInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, btnId));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_btn_get_hw_status
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_btn_get_hw_status(lua_State *L)
{
        int btnId;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                btnId = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 1) {
                        btnId = lua_tointeger(L, 1);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }
        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_BUTTON_E, result, iDbgPdlBtnGetHwStatus,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, btnId));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_btn_set
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_btn_set(lua_State *L)
{
        int cmdId, btnId, value;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 2) {
                cmdId = lua_tointeger(L, 1);
                value = lua_tointeger(L, 2);
                btnId = 0;
        }
        else
                if (lua_gettop(L) == 3) {
                        cmdId = lua_tointeger(L, 1);
                        btnId = lua_tointeger(L, 2);
                        value = lua_tointeger(L, 3);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_BUTTON_E, result, iDbgPdlBtnSetPushValue,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, cmdId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, btnId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value));

        lua_pushnumber(L, result.result);
        return 1;
}
extern PDL_STATUS iDbgPdlBtnGetValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                                btnId
    );

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_btn_get_validation
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_btn_get_validation(lua_State *L)
{
        int btnId;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                btnId = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 1) {
                        btnId = lua_tointeger(L, 1);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }
        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_BUTTON_E, result, iDbgPdlBtnGetValidation,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, btnId));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}
