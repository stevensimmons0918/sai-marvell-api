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
#include <pdl/cpu/pdlCpu.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <iDbgPdl/cpu/iDbgPdlCpu.h>

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_cpu_get
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_cpu_get(lua_State *L)
{
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_CPU_E, result, iDbgPdlCpuGetInfo);
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_cpu_set_debug
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_cpu_set_debug(lua_State *L)
{
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	int value;
	if (lua_gettop(L) == 1) {
		value = lua_tointeger(L, 1);
	}
	else {
		lua_pushnil(L);
		return 0;
	}
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_CPU_E, result, iDbgPdlCpuSetDebug,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(BOOLEAN, value));
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_cpu_set_type
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_cpu_set_type(lua_State *L)
{
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	char *cpuTypePtr;
	if (lua_gettop(L) == 1) {
		cpuTypePtr = (char*) lua_tostring(L, 1);
	}
	else {
		lua_pushnil(L);
		return 0;
	}
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_CPU_E, result, iDbgPdlCpuSetType,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(CHAR_PTR, cpuTypePtr));
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
}
