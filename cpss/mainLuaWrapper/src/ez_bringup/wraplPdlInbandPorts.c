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
#include "iDbgPdl/inband/iDbgPdlInband.h"
#include "iDbgPdl/utils/iDbgPdlUtils.h"
#include "iDbgPdl/logger/iDbgPdlLogger.h"
#include <cpss/common/port/cpssPortCtrl.h>
#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#endif /*CHX_FAMILY*/

#ifndef CHX_FAMILY
#define cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(a,b,c)  GT_NOT_IMPLEMENTED;cpssOsMemSet(c,0,sizeof(CPSS_INTERFACE_INFO_STC))
#define cpssDxChPortInterfaceSpeedSupportGet(a,b,c,d,e)             GT_NOT_IMPLEMENTED;*(e)=GT_FALSE
#define cpssDxChPortInterfaceModeGet(a,b,c)                         GT_NOT_IMPLEMENTED;*(c)=0
#define cpssDxChPortSpeedGet(a,b,c)                                 GT_NOT_IMPLEMENTED;*(c)=0
#endif /*CHX_FAMILY*/


/*************** UTILS ************************/

extern PDL_STATUS iDbgPdlInbandGetPhyInitListInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNumber,
    IN UINT_32                            portNumberInDev,
    IN char                             * postInitOrAllInitListPtr,
    IN char                             * transceiverPtr,
    IN char                             * speedPtr
);

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_network_get_packet_processor_port_phy_init_list_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_network_get_packet_processor_port_phy_init_list_info(lua_State *L)
{
        int dev, port;
        const char *postInitOrAllInitListPtr = NULL, *transceiverPtr = NULL, *speedPtr = NULL;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 3) {
                dev = lua_tointeger(L, 1);
                port = lua_tointeger(L, 2);
                postInitOrAllInitListPtr = lua_tostring(L, 3);
        }
        else
                if (lua_gettop(L) == 4) {
                        dev = lua_tointeger(L, 1);
                        port = lua_tointeger(L, 2);
                        transceiverPtr = lua_tostring(L, 3);
                        speedPtr = lua_tostring(L, 4);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlInbandGetPhyInitListInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, (char *)postInitOrAllInitListPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, (char *)transceiverPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, (char *)speedPtr));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_network_get_packet_processor_port_phy_init_list_info */

extern PDL_STATUS iDbgPdlInbandGetPhyScriptInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNumber,
    IN UINT_32                            portNumberInDev,
    IN char                             * postInitPtr,
    IN char                             * transceiverPtr,
    IN char                             * speedPtr
);

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_network_get_packet_processor_port_phy_script_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_network_get_packet_processor_port_phy_script_info(lua_State *L)
{
        int dev, port;
        const char *postInitPtr = NULL, *transceiverPtr = NULL, *speedPtr = NULL;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 3) {
                dev = lua_tointeger(L, 1);
                port = lua_tointeger(L, 2);
                postInitPtr = lua_tostring(L, 3);
        }
        else
                if (lua_gettop(L) == 4) {
                        dev = lua_tointeger(L, 1);
                        port = lua_tointeger(L, 2);
                        transceiverPtr = lua_tostring(L, 3);
                        speedPtr = lua_tostring(L, 4);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlInbandGetPhyScriptInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, (char *)postInitPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, (char *)transceiverPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, (char *)speedPtr));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_network_get_packet_processor_port_phy_script_info */

extern PDL_STATUS iDbgPdlNetworkGetGbicInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNumber,
    IN UINT_32                            portNumberInDev
    );

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_network_get_packet_processor_port_gbic_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_network_get_packet_processor_port_gbic_info(lua_State *L)
{
        int dev, port;
        const char *portStr = NULL;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                dev = IDBG_PDL_ID_ALL;
                port = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 2) {
                        dev = lua_tointeger(L, 1);
                        portStr = lua_tostring(L, 2);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }

        if (portStr == NULL || strcmp(portStr, "all") == 0)
            port = IDBG_PDL_ID_ALL;
        else
            port = atoi(portStr);

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlNetworkGetGbicInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_network_get_packet_processor_port_gbic_info */

extern PDL_STATUS iDbgPdlNetworkGetHwGbicInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNumber,
    IN UINT_32                            portNumberInDev
);

 /*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_network_get_hw_packet_processor_port_gbic_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_network_get_hw_packet_processor_port_gbic_info(lua_State *L)
{
        int dev, port;
        const char *portStr = NULL;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                dev = IDBG_PDL_ID_ALL;
                port = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 2) {
                        dev = lua_tointeger(L, 1);
                        portStr = lua_tostring(L, 2);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }

        if (portStr == NULL || strcmp(portStr, "all") == 0)
            port = IDBG_PDL_ID_ALL;
        else
            port = atoi(portStr);

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlNetworkGetHwGbicInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_network_get_hw_packet_processor_port_gbic_info */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_inband_get_count
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_inband_get_count(lua_State *L)
{
        int dev;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 1) {
                dev = lua_tonumber(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlInbandGetCount,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}
/*$ END OF wr_utils_pdl_inband_get_count */

extern PDL_STATUS iDbgPdlNetworkGetFrontPanelPortInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              groupNumber,
    IN UINT_32                              portNumberInGroup
);


/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_network_get_front_pannel_port_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_network_get_front_panel_port_info(lua_State *L)
{
        int group, port;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                group = IDBG_PDL_ID_ALL;
                port = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 1) {
                        group = lua_tointeger(L, 1);
                        port = IDBG_PDL_ID_ALL;
                }
                else
                        if (lua_gettop(L) == 2) {
                                group = lua_tointeger(L, 1);
                                port = lua_tointeger(L, 2);
                        }
                        else {
                                lua_pushnil(L);
                                return 0;
                        }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlNetworkGetFrontPanelPortInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, group), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_network_get_front_pannel_port_info */

extern PDL_STATUS iDbgPdlNetworkGetFrontPanelGroupInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              groupNumber
    );

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_network_get_front_panel_group_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_network_get_front_panel_group_info(lua_State *L)
{
        int group;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                group = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 1) {
                        group = lua_tointeger(L, 1);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlNetworkGetFrontPanelGroupInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, group));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_network_get_front_panel_group_info */

extern PDL_STATUS iDbgPdlNetworkGetPacketProcessorPortInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              devNumber,
    IN UINT_32                              portNumberInDev
    );

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_network_get_packet_processor_port_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_network_get_packet_processor_port_info(lua_State *L)
{
        int dev, port;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                dev = IDBG_PDL_ID_ALL;
                port = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 1) {
                        dev = lua_tointeger(L, 1);
                        port = IDBG_PDL_ID_ALL;
                }
                else
                        if (lua_gettop(L) == 2) {
                                dev = lua_tointeger(L, 1);
                                port = lua_tointeger(L, 2);
                        }
                        else {
                                lua_pushnil(L);
                                return 0;
                        }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlNetworkGetPacketProcessorPortInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_network_get_packet_processor_port_info */

extern PDL_STATUS iDbgPdlNetworkGetPacketProcessorPortPhyInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              devNumber,
    IN UINT_32                              portNumberInDev
    );

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_network_get_packet_processor_port_phy_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_network_get_packet_processor_port_phy_info(lua_State *L)
{
        int dev, port;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                dev = IDBG_PDL_ID_ALL;
                port = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 1) {
                        dev = lua_tointeger(L, 1);
                        port = IDBG_PDL_ID_ALL;
                }
                else
                        if (lua_gettop(L) == 2) {
                                dev = lua_tointeger(L, 1);
                                port = lua_tointeger(L, 2);
                        }
                        else {
                                lua_pushnil(L);
                                return 0;
                        }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlNetworkGetPacketProcessorPortPhyInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_network_get_packet_processor_port_phy_info */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_inband_set_info_phy_id
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_inband_set_info_phy_id(lua_State *L)
{
        int dev, port, phy;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 3) {
                dev = lua_tointeger(L, 1);
                port = lua_tointeger(L, 2);
                phy = lua_tointeger(L, 3);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlInbandSetPhyId,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev)
                        , IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port)
                        , IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, phy));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}
/*$ END OF wr_utils_pdl_inband_set_info_phy_id */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_inband_set_info_phy_position
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_inband_set_info_phy_position(lua_State *L)
{
        int dev, port, phy;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 3) {
                dev = lua_tointeger(L, 1);
                port = lua_tointeger(L, 2);
                phy = lua_tointeger(L, 3);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlInbandSetPhyPosition,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev)
                        , IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port)
                        , IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, phy));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}
/*$ END OF wr_utils_pdl_inband_set_info_phy_position */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_inband_set_info_interface_mode
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_inband_set_info_interface_mode(lua_State *L)
{
        int dev, port;
        char *transceiverPtr, *speedPtr, *modePtr;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 5) {
                dev = lua_tointeger(L, 1);
                port = lua_tointeger(L, 2);
                transceiverPtr = (char*) lua_tostring(L, 3);
                speedPtr = (char*) lua_tostring(L, 4);
                modePtr = (char*) lua_tostring(L, 5);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlInbandSetInfoPortMode,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev)
                        , IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port)
                        , IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(CHAR_PTR, transceiverPtr)
                        , IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(CHAR_PTR, speedPtr)
                        , IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(CHAR_PTR, modePtr));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}
/*$ END OF wr_utils_pdl_inband_set_info_interface_mode */

typedef struct {
        CPSS_PORT_SPEED_ENT cpssSpeed;
        PDL_PORT_SPEED_ENT pdlSpeed;
        char *speedStr;
} wr_utils_cpssSpeed2pdl_STC;
#define wr_utils_add_speed_MAC(cpss, pdl, _name) {CPSS_PORT_SPEED_ ## cpss ## _E, PDL_PORT_SPEED_ ## pdl ## _E, # _name}
static wr_utils_cpssSpeed2pdl_STC cpssSpeed2pdl_ARR[] = {
wr_utils_add_speed_MAC(10 , 10 , 10M),
wr_utils_add_speed_MAC(100 , 100 , 100M),
wr_utils_add_speed_MAC(1000 , 1000 , 1G),
wr_utils_add_speed_MAC(10000 , 10000 , 10G),
wr_utils_add_speed_MAC(12000 , LAST , 12G),
wr_utils_add_speed_MAC(2500 , 2500 , 2.5G),
wr_utils_add_speed_MAC(5000 , 5000 , 5G),
wr_utils_add_speed_MAC(13600 , LAST , 13.6G),
wr_utils_add_speed_MAC(20000 , 20000 , 20G),
wr_utils_add_speed_MAC(40000 , 40000 , 40G),
wr_utils_add_speed_MAC(16000 , LAST , 16G),
wr_utils_add_speed_MAC(15000 , LAST , 15G),
wr_utils_add_speed_MAC(75000 , LAST , 75G),
wr_utils_add_speed_MAC(100G , 100000 , 100G),
wr_utils_add_speed_MAC(50000 , 50000 , 50G),
wr_utils_add_speed_MAC(140G , LAST , 140G),
wr_utils_add_speed_MAC(11800 , LAST , 118G),
wr_utils_add_speed_MAC(47200 , LAST , 47.2G),
wr_utils_add_speed_MAC(22000 , LAST , 22G),
wr_utils_add_speed_MAC(23600 , 24000 , 24G),
wr_utils_add_speed_MAC(12500 , LAST , 12.5G),
wr_utils_add_speed_MAC(25000 , 25000 , 25G),
wr_utils_add_speed_MAC(107G , LAST , 107G),
wr_utils_add_speed_MAC(29090 , LAST , 29.09G),
wr_utils_add_speed_MAC(200G , LAST , 200G),
wr_utils_add_speed_MAC(400G , LAST , 400G),
wr_utils_add_speed_MAC(102G , LAST , 102G),
wr_utils_add_speed_MAC(52500 , LAST , 52.5G),
wr_utils_add_speed_MAC(26700 , LAST , 26.7G),
wr_utils_add_speed_MAC(NA , LAST , NA),
};

extern BOOLEAN cpss2pdlSpeedConvert(
                IN CPSS_PORT_SPEED_ENT cpssSpeed,
                OUT PDL_PORT_SPEED_ENT *pdlSpeedPtr,
                OUT const char **speedStr
                )
{
        UINT_32 i;

        for (i = 0; i < sizeof(cpssSpeed2pdl_ARR) / sizeof(cpssSpeed2pdl_ARR[0]); i++)
                        {
                if (cpssSpeed2pdl_ARR[i].cpssSpeed == cpssSpeed)
                                {
                        *speedStr = cpssSpeed2pdl_ARR[i].speedStr;
                        if (cpssSpeed2pdl_ARR[i].pdlSpeed != PDL_PORT_SPEED_LAST_E)
                                        {
                                *pdlSpeedPtr = cpssSpeed2pdl_ARR[i].pdlSpeed;
                                return TRUE;
                        }

                        return FALSE;
                }
        }
        *speedStr = "BAD";
        return FALSE;
}

extern BOOLEAN pdl2cpssSpeedConvert(
                IN PDL_PORT_SPEED_ENT pdlSpeed,
                OUT CPSS_PORT_SPEED_ENT *cpssSpeedPtr,
                OUT const char **speedStr
                )
{
        UINT_32 i;

        for (i = 0; i < sizeof(cpssSpeed2pdl_ARR) / sizeof(cpssSpeed2pdl_ARR[0]); i++)
                        {
                if (cpssSpeed2pdl_ARR[i].pdlSpeed == pdlSpeed)
                                {
                        *speedStr = cpssSpeed2pdl_ARR[i].speedStr;
                        if (cpssSpeed2pdl_ARR[i].pdlSpeed != PDL_PORT_SPEED_LAST_E)
                                        {
                                *cpssSpeedPtr = cpssSpeed2pdl_ARR[i].cpssSpeed;
                                return TRUE;
                        }

                        return FALSE;
                }
        }
        *speedStr = "BAD";
        return FALSE;
}

typedef struct {
        CPSS_PORT_INTERFACE_MODE_ENT cpssIfMode;
        char *cpssIfModeStr;
        PDL_INTERFACE_MODE_ENT pdlIfMode;
} wr_utils_cpssIfMode2pdl_STC;
#define wr_utils_add_mode_MAC(cpss, pdl) {CPSS_PORT_INTERFACE_MODE_ ## cpss ## _E, # cpss, PDL_INTERFACE_MODE_ ## pdl ## _E}

static wr_utils_cpssIfMode2pdl_STC cpssIfMode2pdl_ARR[] = {
wr_utils_add_mode_MAC(MII , MII),
wr_utils_add_mode_MAC(SGMII , SGMII),
wr_utils_add_mode_MAC(XGMII , XGMII),
wr_utils_add_mode_MAC(1000BASE_X , 1000BASE_X),
wr_utils_add_mode_MAC(QX , QX),
wr_utils_add_mode_MAC(HX , HX),
wr_utils_add_mode_MAC(RXAUI , RXAUI),
wr_utils_add_mode_MAC(100BASE_FX , 100BASE_FX),
wr_utils_add_mode_MAC(QSGMII , QSGMII),
wr_utils_add_mode_MAC(KR , KR),
wr_utils_add_mode_MAC(SR_LR , SR_LR),
wr_utils_add_mode_MAC(XHGS , XHGS),
wr_utils_add_mode_MAC(KR2 , KR2),
wr_utils_add_mode_MAC(REDUCED_10BIT , LAST),
wr_utils_add_mode_MAC(REDUCED_GMII , LAST),
wr_utils_add_mode_MAC(MGMII , LAST),
wr_utils_add_mode_MAC(GMII , LAST),
wr_utils_add_mode_MAC(MII_PHY , LAST),
wr_utils_add_mode_MAC(XLG , LAST),
wr_utils_add_mode_MAC(LOCAL_XGMII , LAST),
wr_utils_add_mode_MAC(HGL , LAST),
wr_utils_add_mode_MAC(CHGL_12 , LAST),
wr_utils_add_mode_MAC(ILKN12 , LAST),
wr_utils_add_mode_MAC(ILKN16 , LAST),
wr_utils_add_mode_MAC(ILKN24 , LAST),
wr_utils_add_mode_MAC(ILKN4 , LAST),
wr_utils_add_mode_MAC(ILKN8 , LAST),
wr_utils_add_mode_MAC(KR4 , LAST),
wr_utils_add_mode_MAC(SR_LR2 , LAST),
wr_utils_add_mode_MAC(SR_LR4 , LAST),
wr_utils_add_mode_MAC(MLG_40G_10G_40G_10G, LAST),
wr_utils_add_mode_MAC(KR_C , LAST),
wr_utils_add_mode_MAC(CR_C , LAST),
wr_utils_add_mode_MAC(KR2_C , LAST),
wr_utils_add_mode_MAC(CR2_C , LAST),
wr_utils_add_mode_MAC(CR , LAST),
wr_utils_add_mode_MAC(CR2 , LAST),
wr_utils_add_mode_MAC(CR4 , LAST),
wr_utils_add_mode_MAC(KR_S , LAST),
wr_utils_add_mode_MAC(CR_S , LAST),
wr_utils_add_mode_MAC(KR8 , LAST),
/*    wr_utils_add_mode_MAC(CR8                , LAST),*/
/*    wr_utils_add_mode_MAC(SR_LR8             , LAST),*/
wr_utils_add_mode_MAC(XHGS_SR , LAST),
wr_utils_add_mode_MAC(NA , LAST),
};

extern BOOLEAN cpss2pdlIfModeConvert(
                IN CPSS_PORT_INTERFACE_MODE_ENT cpssIfMode,
                OUT PDL_INTERFACE_MODE_ENT *pdlIfModePtr,
                OUT const char **ifModeStr
                )
{
        UINT_32 i;

        for (i = 0; i < sizeof(cpssIfMode2pdl_ARR) / sizeof(cpssIfMode2pdl_ARR[0]); i++)
                        {
                if (cpssIfMode2pdl_ARR[i].cpssIfMode == cpssIfMode)
                                {
                        *ifModeStr = cpssIfMode2pdl_ARR[i].cpssIfModeStr;
                        if (cpssIfMode2pdl_ARR[i].pdlIfMode != PDL_INTERFACE_MODE_LAST_E)
                                        {
                                *pdlIfModePtr = cpssIfMode2pdl_ARR[i].pdlIfMode;
                                return TRUE;
                        }
                        return FALSE;
                }
        }
        *ifModeStr = "BAD";
        return FALSE;
}

extern BOOLEAN pdl2cpssIfModeConvert(
                IN PDL_INTERFACE_MODE_ENT pdlIfMode,
                OUT CPSS_PORT_INTERFACE_MODE_ENT *cpssIfModePtr,
                OUT const char **ifModeStr
                )
{
        UINT_32 i;

        for (i = 0; i < sizeof(cpssIfMode2pdl_ARR) / sizeof(cpssIfMode2pdl_ARR[0]); i++)
                        {
                if (cpssIfMode2pdl_ARR[i].pdlIfMode == pdlIfMode)
                                {
                        *ifModeStr = cpssIfMode2pdl_ARR[i].cpssIfModeStr;
                        if (cpssIfMode2pdl_ARR[i].pdlIfMode != PDL_INTERFACE_MODE_LAST_E)
                                        {
                                *cpssIfModePtr = cpssIfMode2pdl_ARR[i].cpssIfMode;
                                return TRUE;
                        }
                        return FALSE;
                }
        }
        *ifModeStr = "BAD";
        return FALSE;
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_inband_validate_info_handler
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_is_b2b_port(
                GT_U32 inDev,
                GT_U32 inPort
                )
{
        UINT_32 b2bLink;
        PDL_STATUS pdlStatus;
        PDL_PP_XML_ATTRIBUTES_STC ppAttributes;
        PDL_PP_XML_B2B_ATTRIBUTES_STC b2bAttributes;

        pdlStatus = pdlPpDbAttributesGet(&ppAttributes);
        if (PDL_OK != pdlStatus)
        return FALSE;

        for (b2bLink = 0; b2bLink < ppAttributes.numOfBackToBackLinksPerPp; b2bLink++)
                        {
                pdlStatus = pdlPpDbB2bAttributesGet(b2bLink, &b2bAttributes);
                if (PDL_OK != pdlStatus)
                continue;
                if (b2bAttributes.firstDev == inDev && b2bAttributes.firstPort == inPort)
                return TRUE;
                if (b2bAttributes.secondDev == inDev && b2bAttributes.secondPort == inPort)
                return TRUE;
        }

        return FALSE;
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_inband_validate_info_handler
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_inband_validate_info_handler(
                lua_State *L,
                GT_U32 inDev,
                GT_U32 inPort,
                GT_U32 is_verbose
                )
{
        int lastResult=0;
        GT_U32  i;
        UINT_32 dev, port, /*portCount, */firstDev, lastDev, /*firstPort, LastPort,*/ combinations_found;
        BOOLEAN is_found=FALSE;
        const char *ifModeStr, *speedStr;
        PDL_STATUS pdlStatus;
        GT_STATUS cpssStatus;
        GT_BOOL is_supported_mode_speed;
        BOOLEAN is_supported_mode, is_supported_speed, is_ok;
        CPSS_PORT_INTERFACE_MODE_ENT cpssIfMode;
        PDL_INTERFACE_MODE_ENT pdlIfMode=0;
        CPSS_PORT_SPEED_ENT cpssSpeed;
        PDL_PORT_SPEED_ENT pdlSpeed=0;
        PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC portAttributes;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        CPSS_INTERFACE_INFO_STC portInfo;
        PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC frontPanelAttributes;
        PDL_L1_INTERFACE_MODE_STC *currentL1Ptr;
        PDL_PP_XML_ATTRIBUTES_STC ppAttributes;
        if (inDev == IDBG_PDL_ID_ALL)
        {
                pdlStatus = pdlPpDbAttributesGet(&ppAttributes);
                PDL_CHECK_STATUS(pdlStatus);
                firstDev = 0;
                lastDev = ppAttributes.numOfPps;
        }
        else
        {
                firstDev = inDev;
                lastDev = firstDev + 1;
        }

        memset(&result, 0, sizeof(result));
        result.result = iDbgPdlLoggerCommandLogStart(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, "wr_utils_pdl_inband_validate_info_handler");
        iDbgPdlLoggerApiRunStart("wr_utils_pdl_inband_validate_info_handler", IDBG_PDL_LOGGER_TYPEOF_LAST_CNS);

        for (dev = firstDev; dev < lastDev; dev++)
                        {
                if (inPort == IDBG_PDL_ID_ALL)
                {
                        pdlStatus = pdlPpDbDevAttributesGetFirstPort(dev, &port);
                        PDL_CHECK_STATUS(pdlStatus);
                }
                else
                {
                        port = inPort;
                        pdlStatus = PDL_OK;
                }

                lastResult = 0;
                for (;
                                pdlStatus == PDL_OK;
                                pdlStatus = pdlPpDbDevAttributesGetNextPort(dev, port, &port))
                                                {
                        if (inPort != IDBG_PDL_ID_ALL && port != inPort)
                        break;

                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" Validating port %d/%02d mapping - ", dev, port);
                        cpssStatus = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(dev, port, &portInfo);
                        if (cpssStatus == GT_OK)
                        {
                                if ((portInfo.type != CPSS_INTERFACE_PORT_E) || (portInfo.devPort.hwDevNum != dev) || (portInfo.devPort.portNum != port))
                                                {
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" FAILED\n");
                                        lastResult = 10;
                                }
                                else
                                {
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" PASSED\n");
                                }
                        }
                        else
                                if (cpssStatus == GT_NOT_APPLICABLE_DEVICE)
                                {
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" SKIPPED\n");
                                }
                                else
                                {
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" FAILED\n");
                                        lastResult = 11;
                                }
                }

                if (lastResult)
                {
                        result.result = lastResult;
                }

                IDBG_PDL_LOGGER_PDL_DEBUG_LOG(" %-3s | %-4s | %-20s | %-9s | %-11s | %s\n", "DEV", "PORT", "MODE", "SPEED", "CPSS EXISTS", "PDL EXISTS");
                IDBG_PDL_LOGGER_PDL_DEBUG_LOG(" %-3s | %-4s | %-20s | %-9s | %-11s | %s\n", "---", "----", "-------------", "---------", "-----------", "----------");

                if (inPort == IDBG_PDL_ID_ALL)
                {
                        pdlStatus = pdlPpDbDevAttributesGetFirstPort(dev, &port);
                        PDL_CHECK_STATUS(pdlStatus);
                }
                else
                {
                        port = inPort;
                        pdlStatus = PDL_OK;
                }

                lastResult = 0;
                for (;
                                pdlStatus == PDL_OK;
                                pdlStatus = pdlPpDbDevAttributesGetNextPort(dev, port, &port))
                                                {
                        if (inPort != IDBG_PDL_ID_ALL && port != inPort)
                        break;

                        is_ok = TRUE;
                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" Validating port %d/%02d supported configuration - ", dev, port);

                        pdlStatus = pdlPpDbPortAttributesGet(dev, port, &portAttributes);
                        if (pdlStatus != PDL_OK)
                        {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG(" FAILED\n");
                                if (is_verbose)
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d port %d: PORT doesn't exist\n", dev, port);
                                if (!lastResult)
                                lastResult = 4;
                                continue;
                        }
                        combinations_found = 0;
                        IDBG_PDL_LOGGER_PDL_DEBUG_LOG(" %-3d | %-4d | ", dev, port);
                        for (cpssIfMode = CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E; cpssIfMode < CPSS_PORT_INTERFACE_MODE_NA_E + 1; cpssIfMode++)
                                        {
                                is_supported_mode = cpss2pdlIfModeConvert(cpssIfMode, &pdlIfMode, &ifModeStr);
                                IDBG_PDL_LOGGER_PDL_DEBUG_LOG("%-20s | ", ifModeStr);
                                for (cpssSpeed = CPSS_PORT_SPEED_10_E; cpssSpeed < CPSS_PORT_SPEED_NA_E + 1; cpssSpeed++)
                                                {
                                        is_supported_speed = cpss2pdlSpeedConvert(cpssSpeed, &pdlSpeed, &speedStr);
                                        IDBG_PDL_LOGGER_PDL_DEBUG_LOG("%-9s | ", speedStr);
                                        cpssStatus = cpssDxChPortInterfaceSpeedSupportGet(dev, port, cpssIfMode, cpssSpeed, &is_supported_mode_speed);
                                        if (cpssStatus != GT_OK)
                                        is_supported_mode_speed = GT_FALSE;

                                        IDBG_PDL_LOGGER_PDL_DEBUG_LOG("%-11s | ", is_supported_mode_speed ? "V" : "-");

                                        is_found = FALSE;
                                        if ((is_supported_mode == TRUE) && (is_supported_speed == TRUE))
                                                        {
                                                for (i = 0; i < portAttributes.numOfCopperModes; i++)
                                                                {
                                                        if ((portAttributes.copperModesArr[i].speed == pdlSpeed) && (portAttributes.copperModesArr[i].mode == pdlIfMode))
                                                                        {
                                                                is_found = TRUE;
                                                                combinations_found++;
                                                                break;
                                                        }
                                                }
                                                for (i = 0; i < portAttributes.numOfFiberModes; i++)
                                                                {
                                                        if ((portAttributes.fiberModesArr[i].speed == pdlSpeed) && (portAttributes.fiberModesArr[i].mode == pdlIfMode))
                                                                        {
                                                                is_found = TRUE;
                                                                combinations_found++;
                                                                break;
                                                        }
                                                }

                                        }

                                        IDBG_PDL_LOGGER_PDL_DEBUG_LOG("%s\n", is_found ? "V" : "-");

                                        if ((is_supported_mode_speed == FALSE) && is_found)
                                                        {
                                                if (is_ok)
                                                IDBG_PDL_LOGGER_API_OUTPUT_LOG(" FAILED\n");
                                                if (is_verbose)
                                                IDBG_PDL_LOGGER_API_OUTPUT_LOG(
                                                                "dev %d port %d: CPSS doesn't support mode/speed combinations which PDL DOES support: [mode: %s] [speed: %s]\n", dev, port,
                                                                ifModeStr, speedStr);
                                                if (!lastResult)
                                                lastResult = 1;
                                                is_ok = FALSE;
                                        }

                                        if (is_supported_mode == FALSE && (cpssIfMode != CPSS_PORT_INTERFACE_MODE_NA_E))
                                                        {
                                                IDBG_PDL_LOGGER_PDL_DEBUG_LOG(" %-3s | %-4s | ", "", "");
                                                break;
                                        }
                                        else
                                        {
                                                if (cpssSpeed != CPSS_PORT_SPEED_NA_E)
                                                IDBG_PDL_LOGGER_PDL_DEBUG_LOG(" %-3s | %-4s | %-20s | ", "", "", "");
                                                else
                                                        if (cpssIfMode != CPSS_PORT_INTERFACE_MODE_NA_E)
                                                        IDBG_PDL_LOGGER_PDL_DEBUG_LOG(" %-3s | %-4s | ", "", "");
                                                continue;
                                        }
                                }
                        }
                        if (combinations_found != portAttributes.numOfCopperModes + portAttributes.numOfFiberModes)
                                        {
                                if (is_ok)
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG(" FAILED\n");
                                if (is_verbose)
                                {
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d port %d: CPSS number of supported mode/speed combinations doesn't match that of PDL:\n", dev, port);
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Found in PDL %d expected %d\n", combinations_found,
                                                        portAttributes.numOfCopperModes + portAttributes.numOfFiberModes);
                                }
                                if (!lastResult)
                                lastResult = 3;
                                is_ok = FALSE;
                        }
                        if (is_ok)
                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" PASSED\n");
                }

                if (!result.result)
                result.result = lastResult;

                if (inPort == IDBG_PDL_ID_ALL)
                {
                        pdlStatus = pdlPpDbDevAttributesGetFirstPort(dev, &port);
                        PDL_CHECK_STATUS(pdlStatus);
                }
                else
                {
                        port = inPort;
                        pdlStatus = PDL_OK;
                }

                lastResult = 0;
                for (;
                                pdlStatus == PDL_OK;
                                pdlStatus = pdlPpDbDevAttributesGetNextPort(dev, port, &port))
                                                {
                        if (inPort != IDBG_PDL_ID_ALL && port != inPort)
                        break;

                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" Validating port %d/%02d current configuration - ", dev, port);

                        pdlStatus = pdlPpDbPortAttributesGet(dev, port, &portAttributes);
                        if (pdlStatus != PDL_OK)
                        {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG(" FAILED\n");
                                if (!lastResult)
                                lastResult = 20;
                                continue;
                        }

                        pdlStatus = pdlPpDbFrontPanelAttributesGet(portAttributes.frontPanelNumber, &frontPanelAttributes);
                        if (pdlStatus != PDL_OK)
                        {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG(" FAILED\n");
                                if (!lastResult)
                                lastResult = 21;
                                continue;
                        }

                        cpssStatus = cpssDxChPortInterfaceModeGet(dev, port, &cpssIfMode);
                        if (cpssStatus != GT_OK)
                        cpssIfMode = CPSS_PORT_INTERFACE_MODE_NA_E;

                        is_supported_mode = cpss2pdlIfModeConvert(cpssIfMode, &pdlIfMode, &ifModeStr);
                        IDBG_PDL_LOGGER_PDL_DEBUG_LOG("%-20s | ", ifModeStr);

                        cpssStatus = cpssDxChPortSpeedGet(dev, port, &cpssSpeed);
                        if (cpssStatus != GT_OK)
                        cpssIfMode = CPSS_PORT_SPEED_NA_E;

                        is_supported_speed = cpss2pdlSpeedConvert(cpssSpeed, &pdlSpeed, &speedStr);
                        IDBG_PDL_LOGGER_PDL_DEBUG_LOG("%s", speedStr);

                        currentL1Ptr = NULL;
                        if (is_supported_mode && is_supported_speed)
                                        {
                                for (i = 0; i < portAttributes.numOfCopperModes; i++)
                                                {
                                        if ((portAttributes.copperModesArr[i].speed == pdlSpeed) && (portAttributes.copperModesArr[i].mode == pdlIfMode))
                                                        {
                                                currentL1Ptr = &portAttributes.copperModesArr[i];
                                                break;
                                        }
                                }
                                if (!is_found)
                                {
                                        for (i = 0; i < portAttributes.numOfFiberModes; i++)
                                                        {
                                                if ((portAttributes.fiberModesArr[i].speed == pdlSpeed) && (portAttributes.fiberModesArr[i].mode == pdlIfMode))
                                                                {
                                                        currentL1Ptr = &portAttributes.fiberModesArr[i];
                                                        break;
                                                }
                                        }
                                }
                        }
                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %s\n", currentL1Ptr ? "PASSED" : "FAILED");

                        if (currentL1Ptr && is_verbose)
                                        {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d port %d: mode %s speed %s\n", dev, port, ifModeStr, speedStr);
                        }

                        if (currentL1Ptr == NULL)
                        {
                                if (is_verbose)
                                {
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d port %d: PDL incosistency found. Configuration of mode %s speed %s wasn't found in PDL database\n",
                                                        dev, port, ifModeStr, speedStr);
                                }

                                if (!lastResult)
                                lastResult = 22;
                        }

                }
        }

        if (!result.result)
        result.result = lastResult;

        iDbgPdlLoggerApiResultLog(&result);
        iDbgPdlLoggerCommandEnd(&result);

        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}
/*$ END OF wr_utils_pdl_inband_validate_info_handler */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_inband_validate_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_inband_validate_info(lua_State *L)
{
        int dev, port, verbose;
        const char *portStr;
        if (lua_gettop(L) == 2) {
                dev = lua_tointeger(L, 1);
                portStr = lua_tostring(L, 2);
                verbose = FALSE;
        }
        else
                if (lua_gettop(L) == 3) {
                        dev = lua_tointeger(L, 1);
                        portStr = lua_tostring(L, 2);
                        verbose = lua_tointeger(L, 3);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }

        if (strcmp(portStr, "all") == 0)
                        {
                port = IDBG_PDL_ID_ALL;
        }
        else
        {
                port = atoi(portStr);
        }

        return wr_utils_pdl_inband_validate_info_handler(L, dev, port, verbose);
}

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_inband_validate_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_inband_validate_info_all_dev(lua_State *L)
{
        int port, verbose;
        const char *portStr;

        if (lua_gettop(L) == 1) {
                portStr = lua_tostring(L, 2);
                verbose = FALSE;
        }
        else
                if (lua_gettop(L) == 2) {
                        portStr = lua_tostring(L, 1);
                        verbose = lua_tointeger(L, 2);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }

        if (strcmp(portStr, "all") == 0)
                        {
                port = IDBG_PDL_ID_ALL;
        }
        else
        {
                port = atoi(portStr);
        }

        return wr_utils_pdl_inband_validate_info_handler(L, IDBG_PDL_ID_ALL, port, verbose);
}

extern PDL_STATUS iDbgPdlNetworkRunFrontPanelValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              groupNumber,
    IN UINT_32                              portNumber
);


/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_network_run_fromt_panel_validation
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_network_run_fromt_panel_validation(lua_State *L)
{
        int group, port;
        const char *portStr;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                group = IDBG_PDL_ID_ALL;
                port = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 2) {
                        group = lua_tointeger(L, 1);
                        portStr = lua_tostring(L, 2);
                        if (strcmp(portStr, "all") == 0)
                        port = IDBG_PDL_ID_ALL;
                        else
                        port = atoi(portStr);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlNetworkRunFrontPanelValidation,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, group), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_network_run_fromt_panel_validation */

extern PDL_STATUS iDbgPdlNetworkRunPacketProcessorValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              devNumber,
    IN UINT_32                              portNumberInDev
);


/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_network_run_packet_processor_port_validation
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_network_run_packet_processor_port_validation(lua_State *L)
{
        int dev, port;
        const char *portStr;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 0) {
                dev = IDBG_PDL_ID_ALL;
                port = IDBG_PDL_ID_ALL;
        }
        else
                if (lua_gettop(L) == 2) {
                        dev = lua_tointeger(L, 1);
                        portStr = lua_tostring(L, 2);
                        if (strcmp(portStr, "all") == 0)
                        port = IDBG_PDL_ID_ALL;
                        else
                        port = atoi(portStr);
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, result, iDbgPdlNetworkRunPacketProcessorValidation,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_network_run_packet_processor_port_validation */

