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
#include "pdl/fan/pdlFan.h"
#include "iDbgPdl/fan/iDbgPdlFan.h"
#include "iDbgPdl/utils/iDbgPdlUtils.h"
#include "iDbgPdl/logger/iDbgPdlLogger.h"

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_fan_get_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_fan_get_info(lua_State *L)
{
        int fanControllerId, fanNumber;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                fanControllerId = IDBG_PDL_ID_ALL;
                fanNumber = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 1) {
                        fanControllerId = lua_tointeger(L, 1);
                        fanNumber = IDBG_PDL_ID_ALL;
                }
                else
                        if (lua_gettop(L) == 2) {
                                fanControllerId = lua_tointeger(L, 1);
                                fanNumber = lua_tointeger(L, 2);
                        }
                        else {
                                lua_pushnil(L);
                                return 0;
                        }
        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_FAN_E, result, iDbgPdlFanGetInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, fanControllerId), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, fanNumber));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}

/*$ END OF wr_utils_pdl_fan_get_info */

extern PDL_STATUS iDbgPdlFanRunControllerValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                                fanControllerId,
    IN UINT_32                            fanNumber
    );

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_fan_run_controller_validation
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_fan_run_controller_validation(lua_State *L)
{
        int fanControllerId, fanNumber;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                fanControllerId = IDBG_PDL_ID_ALL;
                fanNumber = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 1) {
                        fanControllerId = lua_tointeger(L, 1);
                        fanNumber = IDBG_PDL_ID_ALL;
                }
                else
                        if (lua_gettop(L) == 2) {
                                fanControllerId = lua_tointeger(L, 1);
                                fanNumber = lua_tointeger(L, 2);
                        }
                        else {
                                lua_pushnil(L);
                                return 0;
                        }
        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_FAN_E, result, iDbgPdlFanRunControllerValidation,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, fanControllerId), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, fanNumber));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}

/*$ END OF wr_utils_pdl_fan_run_controller_validation */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_fan_get_hw_status
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_fan_get_hw_status(lua_State *L)
{
        int fanControllerId, fanId;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                fanControllerId = IDBG_PDL_ID_ALL;
                fanId = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 2) {
                        fanControllerId = lua_tointeger(L, 1);
                        fanId = lua_tointeger(L, 2);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }
        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_FAN_E, result, iDbgPdlFanGetHwStatus,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, fanControllerId), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, fanId));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}

/*$ END OF wr_utils_pdl_fan_get_hw_status */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_fan_set
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_fan_set(lua_State *L)
{
        int cmdId, fanControllerId, fanId, value;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 2) {
                cmdId = lua_tointeger(L, 1);
                fanControllerId = IDBG_PDL_ID_ALL;
                fanId = IDBG_PDL_ID_ALL;
                value = lua_tointeger(L, 2);
        }
        else
                if (lua_gettop(L) == 3) {
                        cmdId = lua_tointeger(L, 1);
                        fanControllerId = lua_tointeger(L, 2);
                        fanId = IDBG_PDL_ID_ALL;
                        value = lua_tointeger(L, 3);
                }
                else
                        if (lua_gettop(L) == 4) {
                                cmdId = lua_tointeger(L, 1);
                                fanControllerId = lua_tointeger(L, 2);
                                fanId = lua_tointeger(L, 3);
                                value = lua_tointeger(L, 4);
                        }
                        else {
                                lua_pushnil(L);
                                return 0;
                        }
        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_FAN_E, result, iDbgPdlFanSetHwStatus,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, cmdId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, fanControllerId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, fanId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);

        return 1;
}

PDL_STATUS iDbgPdlFanGetControllerinfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      fanControllerId
);

/*****************************************************************************
 * FUNCTION NAME:  wr_utils_pdl_fan_get_controller_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_fan_get_controller_info(lua_State *L)
{
        int fanControllerId;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                fanControllerId = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 1) {
                        fanControllerId = lua_tointeger(L, 1);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }
        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_FAN_E, result, iDbgPdlFanGetControllerinfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, fanControllerId));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}

extern PDL_STATUS  iDbgPdlFanControllerGetHw (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                                fanControllerId
    );


/*$ END OF wr_utils_pdl_fan_get_controller_info  */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_fan_get_fan_controller_hw
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_fan_get_fan_controller_hw(lua_State *L)
{
        int fanControllerId;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                fanControllerId = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 1) {
                        fanControllerId = lua_tointeger(L, 1);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }
        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_FAN_E, result, iDbgPdlFanControllerGetHw,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, fanControllerId));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}

/*$ END OF wr_utils_pdl_fan_get_fan_controller_hw */

