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

#include "pdl/led/pdlLed.h"
#include "pdl/led/private/prvPdlLed.h"
#include "iDbgPdl/led/iDbgPdlLed.h"
#include "iDbgPdl/utils/iDbgPdlUtils.h"
#include "iDbgPdl/logger/iDbgPdlLogger.h"

/*************** UTILS ************************/

/*************** WRAPPERS ************************/
/***********************************************************************************************************************************/

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_led_system_get_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_led_system_get_info(lua_State *L)
{
        int ledId  ;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                ledId = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 1) {
                        ledId = lua_tointeger(L, 1);
                }
                else {
                        lua_pushnil(L);
                        return 1;
                }
        IDBG_PDL_LOGGER_API_EXECUTE("show led system info", IDBG_PDL_LOGGER_CATEGORY_LED_E, result, iDbgPdlLedSystemGetInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ledId));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_led_port_get_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_led_port_get_info(lua_State *L)
{
        int  /*ledId,*/ dev=0, port=0;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                /*ledId = IDBG_PDL_ID_ALL;*/
        }
        else
                if (lua_gettop(L) == 2) {
                        dev = lua_tointeger(L, 1);
                        port = lua_tointeger(L, 2);
                }
                else {
                        lua_pushnil(L);
                        return 1;
                }
        IDBG_PDL_LOGGER_API_EXECUTE("show led port info", IDBG_PDL_LOGGER_CATEGORY_LED_E, result, iDbgPdlLedPortGetInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_led_system_set_hw
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_led_system_set_hw(lua_State *L)
{
        int ledId, state, color;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 2) {
                ledId = IDBG_PDL_ID_ALL;
                state = lua_tointeger(L, 1);
                color = lua_tointeger(L, 2);
        }
        else
                if (lua_gettop(L) == 3) {
                        ledId = lua_tointeger(L, 1);
                        state = lua_tointeger(L, 2);
                        color = lua_tointeger(L, 3);
                }
                else {
                        lua_pushnil(L);
                        return 1;
                }
        IDBG_PDL_LOGGER_API_EXECUTE("set led system hw", IDBG_PDL_LOGGER_CATEGORY_LED_E, result, iDbgPdlLedSystemSetHwStatus,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ledId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, state),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, color));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}

/*$ END OF wr_utils_pdl_led_system_set_hw */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_led_system_set_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_led_system_set_info(lua_State *L)
{
        int ledId, state, color, value, smiDev, smiReg, smiMask;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 7) {
                ledId = lua_tointeger(L, 1);
                state = lua_tointeger(L, 2);
                color = lua_tointeger(L, 3);
                value = lua_tointeger(L, 4);
                smiDev = lua_tointeger(L, 5);
                smiReg = lua_tointeger(L, 6);
                smiMask = lua_tointeger(L, 7);
        }
        else {
                lua_pushnil(L);
                return 1;
        }
        IDBG_PDL_LOGGER_API_EXECUTE("set led system info", IDBG_PDL_LOGGER_CATEGORY_LED_E, result, iDbgPdlLedSetInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ledId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, PDL_LED_DONT_CARE_VALUE_CNS),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, PDL_LED_DONT_CARE_VALUE_CNS),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, state),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, color),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, smiDev),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, smiReg),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, smiMask));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}

/*$ END OF wr_utils_pdl_led_system_set_info */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_led_port_set_hw
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_led_port_set_hw(lua_State *L)
{
        int dev, port, ledId, state, color;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 2) {
                dev = port = ledId = IDBG_PDL_ID_ALL;
                state = lua_tointeger(L, 2);
                color = lua_tointeger(L, 3);
        }
        else
                if (lua_gettop(L) == 5) {
                        dev = lua_tointeger(L, 1);
                        port = lua_tointeger(L, 2);
                        ledId = lua_tointeger(L, 3);
                        state = lua_tointeger(L, 4);
                        color = lua_tointeger(L, 5);
                }
                else {
                        lua_pushnil(L);
                        return 1;
                }
        IDBG_PDL_LOGGER_API_EXECUTE("set led port hw", IDBG_PDL_LOGGER_CATEGORY_LED_E, result, iDbgPdlLedPortSetHwStatus,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ledId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, state),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, color));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}

/*$ END OF wr_utils_pdl_led_port_set_hw */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_led_port_set_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_led_port_set_info(lua_State *L)
{
        int dev, port, ledId, state, color, value, smiDev, smiReg, smiMask;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 9) {
                ledId = lua_tointeger(L, 1);
                dev = lua_tointeger(L, 2);
                port = lua_tointeger(L, 3);
                state = lua_tointeger(L, 4);
                color = lua_tointeger(L, 5);
                value = lua_tointeger(L, 6);
                smiDev = lua_tointeger(L, 7);
                smiReg = lua_tointeger(L, 8);
                smiMask = lua_tointeger(L, 9);
        }
        else {
                lua_pushnil(L);
                return 1;
        }
        IDBG_PDL_LOGGER_API_EXECUTE("set led system info", IDBG_PDL_LOGGER_CATEGORY_LED_E, result, iDbgPdlLedSetInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ledId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, state),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, color),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, smiDev),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, smiReg),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, smiMask));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}

/*$ END OF wr_utils_pdl_led_port_set_info */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_led_set_debug
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_led_set_debug(lua_State *L)
{
        int value;
        PDL_STATUS pdlStatus;
        if (lua_gettop(L) == 1) {
                value = lua_tointeger(L, 1);
                pdlStatus = pdlLedDebugSet(value);
                PDL_CHECK_STATUS(pdlStatus);
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        lua_pushnumber(L, 0);
        return 1;
}

/*$ END OF wr_utils_pdl_led_set_debug */
extern PDL_STATUS iDbgPdlLedRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_LED_ID_ENT                     ledId,
    IN UINT_32                            timeSec
);

/*****************************************************************************
 * FUNCTION NAME:wr_utils_pdl_led_run_validation
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_led_run_validation(lua_State *L)
{
        int ledId, timeSec;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 1) {
                ledId = IDBG_PDL_ID_ALL;
                timeSec = lua_tointeger(L, 1);
        }
        else
                if (lua_gettop(L) == 2) {
                        ledId = lua_tointeger(L, 1);
                        timeSec = lua_tointeger(L, 2);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }
        IDBG_PDL_LOGGER_API_EXECUTE("run led validate device", IDBG_PDL_LOGGER_CATEGORY_LED_E, result, iDbgPdlLedRunValidation,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ledId), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, timeSec));
        lua_pushnumber(L, result.result);
        return 1;
}
/*$ END OF wr_utils_pdl_led_run_validation */

