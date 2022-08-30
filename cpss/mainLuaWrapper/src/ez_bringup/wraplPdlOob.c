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
#include "pdl/oob_port/pdlOobPort.h"
#include "iDbgPdl/oob/iDbgPdlOob.h"
#include "iDbgPdl/utils/iDbgPdlUtils.h"
#include "iDbgPdl/logger/iDbgPdlLogger.h"

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_oob_set_phy_position
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_oob_set_phy_position(lua_State *L)
{
	int oobId, phyPos;
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	if (lua_gettop(L) == 2) {
		oobId = lua_tointeger(L, 1);
		phyPos = lua_tointeger(L, 2);
	}
	else {
		lua_pushnil(L);
		return 0;
	}
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_OOB_E, result, iDbgPdlOobSetPhyPosition,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId),
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, phyPos));
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_oob_set_phy_id
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_oob_set_phy_id(lua_State *L)
{
	int oobId, phy;
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	if (lua_gettop(L) == 2) {
		oobId = lua_tointeger(L, 1);
		phy = lua_tointeger(L, 2);
	}
	else {
		lua_pushnil(L);
		return 0;
	}
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_OOB_E, result, iDbgPdlOobSetPhyId,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId),
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, phy));
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_oob_set_max_speed
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_oob_set_max_speed(lua_State *L)
{
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	int oobId;
	char *speedPtr;
	if (lua_gettop(L) == 2) {
		oobId = lua_tointeger(L, 1);
		speedPtr = (char*) lua_tostring(L, 2);
	}
	else {
		lua_pushnil(L);
		return 0;
	}
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_OOB_E, result, iDbgPdlOobSetMaxSpeed,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId),
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(CHAR_PTR, speedPtr));
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_oob_set_cpu_port
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_oob_set_cpu_port(lua_State *L)
{
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	int oobId, cpuPort;
	if (lua_gettop(L) == 2) {
		oobId = lua_tointeger(L, 1);
		cpuPort = lua_tointeger(L, 2);
	}
	else {
		lua_pushnil(L);
		return 0;
	}
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_OOB_E, result, iDbgPdlOobSetCpuPort,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId),
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, cpuPort));
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_oob_set_eth_id
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_oob_set_eth_id(lua_State *L)
{
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	int oobId, ethId;
	if (lua_gettop(L) == 2) {
		oobId = lua_tointeger(L, 1);
		ethId = lua_tointeger(L, 2);
	}
	else {
		lua_pushnil(L);
		return 0;
	}
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_OOB_E, result, iDbgPdlOobSetEthId,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId),
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ethId));
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_oob_count_get
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_oob_count_get(lua_State *L)
{
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	if (lua_gettop(L) > 0) {
		lua_pushnil(L);
		return 0;
	}
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_OOB_E, result, iDbgPdlOobGetCount);
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_oob_get
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_oob_info_get(lua_State *L)
{
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	int oobId;
	const char *portStr;
	if (lua_gettop(L) == 1) {
		portStr = lua_tostring(L, 1);
	}
	else {
		lua_pushnil(L);
		return 0;
	}
	
	if (strcmp(portStr, "all") == 0)
	oobId = IDBG_PDL_ID_ALL;
	else
	oobId = atoi(portStr);
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_OOB_E, result, iDbgPdlOobGetInfo,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId));
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_oob_set_debug
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_oob_set_debug(lua_State *L)
{
	int value;
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	if (lua_gettop(L) == 1) {
		value = lua_tointeger(L, 2);
	}
	else {
		lua_pushnil(L);
		return 0;
	}
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_OOB_E, result, iDbgPdlOobSetDebug,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(BOOLEAN, value));
	
	lua_pushnumber(L, result.result);
	return 1;
}

extern PDL_STATUS iDbgPdlOobRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                                oobId
);

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_network_run_oob_validation
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_network_run_oob_validation(lua_State *L)
{
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	int oobId;
	if (lua_gettop(L) == 0) {
		oobId = IDBG_PDL_ID_ALL;
	}
	else {
		lua_pushnil(L);
		return 0;
	}
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_OOB_E, result, iDbgPdlOobRunValidation,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId));
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
}

/*$ END OF wr_utils_pdl_network_run_oob_validation*/