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

#include "pdl/phy/pdlPhy.h"
#include "iDbgPdl/phy/iDbgPdlPhy.h"
#include "iDbgPdl/utils/iDbgPdlUtils.h"
#include "iDbgPdl/logger/iDbgPdlLogger.h"

/*************** WRAPPERS ************************/

/*****************************************************************************
 * FUNCTION NAME: wr_pdl_phy_show_count
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_pdl_phy_show_count(lua_State *L)
{
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PHY_E, result, iDbgPdlPhyGetCount);
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}

/*****************************************************************************
 * FUNCTION NAME: wr_pdl_phy_show_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_pdl_phy_show_info(lua_State *L)
{
        int phyId;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                phyId = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 1) {
                        phyId = lua_tointeger(L, 1);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PHY_E, result, iDbgPdlPhyGetInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, phyId));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_pdl_phy_show_info */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_phy_set_debug
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_pdl_phy_set_debug(lua_State *L)
{
        int value;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 1) {
                value = lua_tonumber(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PHY_E, result, iDbgPdlPhySetDebug,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(BOOLEAN, value));

        lua_pushnumber(L, result.result);
        return 1;
}

/*$ END OF  wr_utils_pdl_phy_set_debug */
extern PDL_STATUS iDbgPdlPhyRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                                phyId
);

/*****************************************************************************
 * FUNCTION NAME: wr_pdl_phy_run_validation
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_pdl_phy_run_validation(lua_State *L)
{
        int phyId;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                phyId = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 1) {
                        phyId = lua_tointeger(L, 1);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PHY_E, result, iDbgPdlPhyRunValidation,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, phyId));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_pdl_phy_run_validation */

