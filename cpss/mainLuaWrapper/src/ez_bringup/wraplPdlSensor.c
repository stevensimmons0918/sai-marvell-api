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

#include "pdl/sensor/pdlSensor.h"
#include "iDbgPdl/sensor/iDbgPdlSensor.h"
#include "iDbgPdl/utils/iDbgPdlUtils.h"
#include "iDbgPdl/logger/iDbgPdlLogger.h"

/*************** UTILS ************************/

typedef enum {
	PDL_SENSOR_UTILS_CMD_GET_INFO_E = 0,
	PDL_SENSOR_UTILS_CMD_GET_TEMP_E,
	PDL_SENSOR_UTILS_CMD_GET_NUM_E
} PDL_SENSOR_UTILS_CMD_GET_ENT;

typedef enum {
	PDL_SENSOR_UTILS_CMD_SET_INFO_E = 0,
	PDL_SENSOR_UTILS_CMD_SET_DEBUG_E
} PDL_SENSOR_UTILS_CMD_SET_ENT;

/*****************************************************************************
 * FUNCTION NAME: utils_pdl_sensor_set
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
static PDL_STATUS utils_pdl_sensor_set
(
		IN UINT_32 cmdId,
		IN UINT_32 value
		)
{
	PDL_STATUS pdlStatus;

	
	switch (cmdId) {
/*    case PDL_SENSOR_UTILS_CMD_SET_INFO_E:

        break;*/
		case PDL_SENSOR_UTILS_CMD_SET_DEBUG_E:
			pdlStatus = pdlSensorDebugSet(value);
		break;
			
		default:
			pdlStatus = PDL_BAD_VALUE;
		break;
	}
	PDL_CHECK_STATUS(pdlStatus);
	return pdlStatus;
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_sensor_get_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_sensor_get_info(lua_State *L)
{
	int sensorId;
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	if (lua_gettop(L) == 0) {
		sensorId = IDBG_PDL_ID_ALL;
	}
	else
		if (lua_gettop(L) == 1) {
			sensorId = lua_tointeger(L, 1);
		}
		else {
			lua_pushnil(L);
			return 0;
		}
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_SENSOR_E, result, iDbgPdlSensorGetInfo,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, sensorId));
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
	
}

/*$ END OF wr_utils_pdl_sensor_get_info */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_sensor_get_temperature
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_sensor_get_temperature(lua_State *L)
{
	int sensorId;
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	if (lua_gettop(L) == 0) {
		sensorId = IDBG_PDL_ID_ALL;
	}
	else
		if (lua_gettop(L) == 1) {
			sensorId = lua_tointeger(L, 1);
		}
		else {
			lua_pushnil(L);
			return 0;
		}
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_SENSOR_E, result, iDbgPdlSensorGetTemperature,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, sensorId));
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
	
}

/*$ END OF wr_utils_pdl_sensor_get_temperature */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_sensor_get_count
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_sensor_get_count(lua_State *L)
{
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_SENSOR_E, result, iDbgPdlSensorGetCount);
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
	
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_sensor_set
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_sensor_set(lua_State *L)
{
	int res, cmdId, value;
	if (lua_gettop(L) == 2) {
		cmdId = lua_tonumber(L, 1);
		value = lua_tonumber(L, 2);
	}
	else {
		lua_pushnil(L);
		return 0;
	}
	
	res = utils_pdl_sensor_set(cmdId, value);
	
	lua_pushnumber(L, res);
	return 1;
}

extern PDL_STATUS iDbgPdlSensorRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                                sensorNumber
);

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_sensor_run_validation
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_sensor_run_validation(lua_State *L)
{
	int sensorNumber;
	IDBG_PDL_LOGGER_RESULT_DATA_STC result;
	if (lua_gettop(L) == 0) {
		sensorNumber = IDBG_PDL_ID_ALL;
	}
	else
		if (lua_gettop(L) == 1) {
			sensorNumber = lua_tointeger(L, 1);
		}
		else {
			lua_pushnil(L);
			return 0;
		}
	
	IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_SENSOR_E, result, iDbgPdlSensorRunValidation,
			IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, sensorNumber));
	if (result.outDescriptionPtr)
	printf("%s", result.outDescriptionPtr);
	
	lua_pushnumber(L, result.result);
	return 1;
	
}
/*$ END OF wr_utils_pdl_sensor_run_validation */
