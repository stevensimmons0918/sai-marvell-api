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
#include <string.h>

#include "wraplPdl.h"

#include "iDbgPdl/packetProcessor/iDbgPdlPp.h"
#include "iDbgPdl/utils/iDbgPdlUtils.h"
#include "iDbgPdl/logger/iDbgPdlLogger.h"

/*************** UTILS ************************/

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_pp_get_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_pp_get_info(lua_State *L)
{
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlPpGetInfo);
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
	
}
/*$ END OF wr_utils_pdl_pp_get_info */

extern PDL_STATUS iDbgPdlPpRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
    );

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_pp_run_validation
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_pp_run_validation(lua_State *L)
{
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlPpRunValidation);
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
	
}
/*$ END OF wr_utils_pdl_pp_run_validation */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_pp_get_b2b
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_pp_get_b2b(lua_State *L)
{
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlPpGetInfo);
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
	
}
/*$ END OF wr_utils_pdl_pp_get_b2b */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_pp_get_front_panel
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_pp_get_front_panel(lua_State *L)
{
	int fpId;
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	if (lua_gettop(L) == 0) {
		fpId = IDBG_PDL_ID_ALL;
	}
	else
		if (lua_gettop(L) == 1) {
			fpId = lua_tointeger(L, 1);
		}
		else {
			lua_pushnil(L);
			return 0;
		}
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlPpGetFp,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, fpId));
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
	
}
/*$ END OF wr_utils_pdl_pp_get_front_panel */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_pp_set_debug
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_pp_set_debug(lua_State *L)
{
	int enable;
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	if (lua_gettop(L) == 1) {
		enable = lua_tonumber(L, 1);
	}
	else {
		lua_pushnil(L);
		return 0;
	}
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlPpSetDebug,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(BOOLEAN, enable));
	
	lua_pushnumber(L, result.result);
	return 1;
}
/*$ END OF wr_utils_pdl_pp_set_debug */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_pp_set_front_panel
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_pp_set_front_panel(lua_State *L)
{
	int fpId;
	char *speedStr, *orderStr;
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	if (lua_gettop(L) == 3) {
		fpId = lua_tointeger(L, 1);
		orderStr = (char*) lua_tostring(L, 2);
		speedStr = (char*) lua_tostring(L, 3);
	}
	else {
		lua_pushnil(L);
		return 0;
	}
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L),
			IDBG_PDL_LOGGER_CATEGORY_PP_E,
			result,
			iDbgPdlPpSetFp,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, fpId),
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(CHAR_PTR, orderStr),
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(CHAR_PTR, speedStr));
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
	
}
/*$ END OF wr_utils_pdl_pp_set_front_panel */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_pp_set_b2b
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_pp_set_b2b(lua_State *L)
{
	int b2bLinkId;
	char *speedStr, *modeStr;
	UINT_32 firstDevice, *firstDevicePtr = NULL,
			firstPort, *firstPortPtr = NULL,
			secondDevice, *secondDevicePtr = NULL,
			secondPort, *secondPortPtr = NULL;
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	if (lua_gettop(L) == 7) {
		b2bLinkId = lua_tointeger(L, 1);
		speedStr = (char*) lua_tostring(L, 2);
		modeStr = (char*) lua_tostring(L, 3);
		if (lua_isnil(L, 4) == FALSE)
		{
			firstDevice = lua_tonumber(L, 4);
			firstDevicePtr = &firstDevice;
		}
		if (lua_isnil(L, 5) == FALSE)
		{
			firstPort = lua_tonumber(L, 5);
			firstPortPtr = &firstPort;
		}
		if (lua_isnil(L, 6) == FALSE)
		{
			secondDevice = lua_tonumber(L, 6);
			secondDevicePtr = &secondDevice;
		}
		if (lua_isnil(L, 7) == FALSE)
		{
			secondPort = lua_tonumber(L, 7);
			secondPortPtr = &secondPort;
		}
	}
	else {
		lua_pushnil(L);
		return 0;
	}
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L),
			IDBG_PDL_LOGGER_CATEGORY_PP_E,
			result,
			iDbgPdlPpSetB2b,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, b2bLinkId),
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(CHAR_PTR, speedStr),
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(CHAR_PTR, modeStr),
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, firstDevicePtr),
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, firstPortPtr),
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, secondDevicePtr),
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, secondPortPtr));
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
	
}
/*$ END OF wr_utils_pdl_pp_set_b2b */
