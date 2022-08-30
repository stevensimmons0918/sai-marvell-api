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

#include <pdl/common/pdlTypes.h>
#include "pdl/packet_processor/pdlPacketProcessor.h"
#include "iDbgPdl/utils/iDbgPdlUtils.h"
#include "iDbgPdl/logger/iDbgPdlLogger.h"
#include "iDbgPdl/power/iDbgPdlPower.h"
#include <cpss/common/port/cpssPortCtrl.h>

/*************** UTILS ************************/

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_power_get_port_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_power_get_port_info(lua_State *L)
{
        const char *powerId;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 1) {
                powerId = lua_tostring(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlPowerGetInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, (char *)powerId));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_power_get_port_info */

extern PDL_STATUS iDbgPdlPowerGetHwInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN char                             * powerId
);


/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_power_get_hw_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_power_get_hw_info(lua_State *L)
{
        const char *powerId;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 1) {
                powerId = lua_tostring(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlPowerGetHwInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, (char *)powerId));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_power_get_hw_info */

extern PDL_STATUS iDbgPdlPowerRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN char                              *powerId
);

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_power_run_validation
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_power_run_validation(lua_State *L)
{
        const char *powerId;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 1) {
                powerId = lua_tostring(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlPowerRunValidation,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, (char *)powerId));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_power_run_validation */
