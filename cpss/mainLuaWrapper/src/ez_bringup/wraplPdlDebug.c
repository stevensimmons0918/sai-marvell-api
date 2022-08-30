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

#include "pdl/sensor/pdlSensor.h"
#include "pdl/interface/private/prvPdlI2c.h"

/*************** UTILS ************************/

/*****************************************************************************
 * FUNCTION NAME: utils_pdl_debug_i2c_read
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
static PDL_STATUS utils_pdl_debug_i2c_read
(
		IN UINT_8 i2cAddr, /* the target device slave address on I2C bus */
		IN UINT_32 i2cBuffLen, /* buffer length */
		IN UINT_8 i2cBusId, /* the I2C bus id */
		IN UINT_32 i2cOffsetType, /* receive mode */
		IN UINT_32 i2cOffset /* internal offset to read from */
		)
{
	PDL_STATUS pdlStatus;
	UINT_8 i, buffer[255];
	pdlStatus = prvPdlI2cReceiveWithOffset(i2cAddr,
			i2cBuffLen,
			i2cBusId,
			i2cOffsetType,
			i2cOffset,
			PDL_INTERFACE_I2C_TRANSACTION_TYPE_OFFSET_AND_DATA,
			(UINT_8*) &buffer);
	PDL_CHECK_STATUS(pdlStatus);
	
	printf("I2C Register Value:\n");
	for (i = 0; i < i2cBuffLen; i++) {
		printf("[0x%X] ", buffer[i]);
	}
	printf("\n");
	return pdlStatus;
}

/*************** WRAPPERS ************************/
/***********************************************************************************************************************************/

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_debug_i2c_read
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_debug_i2c_read(lua_State *L)
{
	int res, i2cAddr, i2cBuffLen, i2cBusId, i2cOffsetType, i2cOffset;
	if (lua_gettop(L) == 5) {
		i2cAddr = lua_tonumber(L, 1);
		i2cBuffLen = lua_tonumber(L, 2);
		i2cBusId = lua_tonumber(L, 3);
		i2cOffsetType = lua_tonumber(L, 4);
		i2cOffset = lua_tonumber(L, 5);
	}
	else {
		lua_pushnil(L);
		return 0;
	}
	
	res = utils_pdl_debug_i2c_read(i2cAddr, i2cBuffLen, i2cBusId, i2cOffsetType, i2cOffset);
	
	lua_pushnumber(L, res);
	return 1;
}
