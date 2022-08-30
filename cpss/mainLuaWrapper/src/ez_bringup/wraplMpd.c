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

#include <mpd.h>
#include <mpdPrv.h>
#include <mpdDebug.h>

/*************** WRAPPERS ************************/

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_mdix_oper
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_mdix_oper(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_MDIX_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("get mdix oper operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

if (params.phyMdix.mode == MPD_MDI_MODE_MEDIA_E)
                        {
                printf("mdix oper mode is: MDI\n");
        }
        else
                if (params.phyMdix.mode == MPD_MDIX_MODE_MEDIA_E)
                                {
                        printf("mdix oper mode is: MDIX\n");
                }
                else
                {
                        printf("mdix oper mode is: AUTO\n");
                }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_get_mdix_oper */


/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_mdix_admin
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_mdix_admin(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_MDIX_ADMIN_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("get mdix admin operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

if (params.phyMdix.mode == MPD_MDI_MODE_MEDIA_E)
                        {
                printf("mdix oper mode is: MDI\n");
        }
        else
                if (params.phyMdix.mode == MPD_MDIX_MODE_MEDIA_E)
                                {
                        printf("mdix oper mode is: MDIX\n");
                }
                else
                {
                        printf("mdix oper mode is: AUTO\n");
                }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_get_mdix_oper */


/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_ext_vct_params
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_ext_vct_params(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        const char *testTypeStr;
        MPD_VCT_TEST_TYPE_ENT test_type;
        if (lua_gettop(L) == 2) {
                rel_ifIndex = lua_tointeger(L, 1);
                testTypeStr = lua_tostring(L, 2);
                if (strcmp(testTypeStr, "MPD_VCT_TEST_TYPE_CABLECHANNEL1_E") == 0)
                test_type = MPD_VCT_TEST_TYPE_CABLECHANNEL1_E;
                else
                        if (strcmp(testTypeStr, "MPD_VCT_TEST_TYPE_CABLECHANNEL2_E") == 0)
                        test_type = MPD_VCT_TEST_TYPE_CABLECHANNEL2_E;
                        else
                                if (strcmp(testTypeStr, "MPD_VCT_TEST_TYPE_CABLECHANNEL3_E") == 0)
                                test_type = MPD_VCT_TEST_TYPE_CABLECHANNEL3_E;
                                else
                                        if (strcmp(testTypeStr, "MPD_VCT_TEST_TYPE_CABLECHANNEL4_E") == 0)
                                        test_type = MPD_VCT_TEST_TYPE_CABLECHANNEL4_E;
                                        else
                                                if (strcmp(testTypeStr, "MPD_VCT_TEST_TYPE_CABLEPOLARITY1_E") == 0)
                                                test_type = MPD_VCT_TEST_TYPE_CABLEPOLARITY1_E;
                                                else
                                                        if (strcmp(testTypeStr, "MPD_VCT_TEST_TYPE_CABLEPOLARITY2_E") == 0)
                                                        test_type = MPD_VCT_TEST_TYPE_CABLEPOLARITY2_E;
                                                        else
                                                                if (strcmp(testTypeStr, "MPD_VCT_TEST_TYPE_CABLEPOLARITY3_E") == 0)
                                                                test_type = MPD_VCT_TEST_TYPE_CABLEPOLARITY3_E;
                                                                else
                                                                        if (strcmp(testTypeStr, "MPD_VCT_TEST_TYPE_CABLEPOLARITY4_E") == 0)
                                                                        test_type = MPD_VCT_TEST_TYPE_CABLEPOLARITY4_E;
                                                                        else
                                                                                if (strcmp(testTypeStr, "MPD_VCT_TEST_TYPE_CABLEPAIRSKEW1_E") == 0)
                                                                                test_type = MPD_VCT_TEST_TYPE_CABLEPAIRSKEW1_E;
                                                                                else
                                                                                        if (strcmp(testTypeStr, "MPD_VCT_TEST_TYPE_CABLEPAIRSKEW2_E") == 0)
                                                                                        test_type = MPD_VCT_TEST_TYPE_CABLEPAIRSKEW2_E;
                                                                                        else
                                                                                                if (strcmp(testTypeStr, "MPD_VCT_TEST_TYPE_CABLEPAIRSKEW3_E") == 0)
                                                                                                test_type = MPD_VCT_TEST_TYPE_CABLEPAIRSKEW3_E;
                                                                                                else
                                                                                                        if (strcmp(testTypeStr, "MPD_VCT_TEST_TYPE_CABLEPAIRSKEW4_E") == 0)
                                                                                                        test_type = MPD_VCT_TEST_TYPE_CABLEPAIRSKEW4_E;
                                                                                                        else
                                                                                                        {
                                                                                                                lua_pushnil(L);
                                                                                                                return 0;
                                                                                                        }
                params.phyExtVct.testType = test_type;
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_EXT_VCT_PARAMS_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("get ext vct params operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        printf("test result us %d\n", params.phyExtVct.result);

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_get_ext_vct_params */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_cable_len
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_cable_len(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_CABLE_LEN_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("get cable len operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        if (params.phyCableLen.cableLength == MPD_CABLE_LENGTH_LESS_THAN_50M_E) {
                printf("cable length is less than 50 meter\n");
        }
        else
                if (params.phyCableLen.cableLength == MPD_CABLE_LENGTH_50M_80M_E) {
                        printf("cable length is 50-80 meter\n");
                }
                else
                        if (params.phyCableLen.cableLength == MPD_CABLE_LENGTH_80M_110M_E) {
                                printf("cable length is 80-110 meter\n");
                        }
                        else
                                if (params.phyCableLen.cableLength == MPD_CABLE_LENGTH_110M_140M_E) {
                                        printf("cable length is 110-140 meter\n");
                                }
                                else
                                        if (params.phyCableLen.cableLength == MPD_CABLE_LENGTH_MORE_THAN_140M_E) {
                                                printf("cable length is more than 140 meter\n");
                                        }
                                        else {
                                                printf("cable length is unknown\n");
                                        }

        if (params.phyCableLen.accurateLength != 0)
        printf("accurate length is %d\n", params.phyCableLen.accurateLength);

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_get_cable_len */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_autoneg_remote_capabilities
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_autoneg_remote_capabilities(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("get autoneg remote capabilities operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        if (params.phyRemoteAutoneg.capabilities == MPD_AUTONEG_CAPABILITIES_DEFAULT_CNS)
        {
                printf("autoneg remote capabilities are default\n");
        }
        else
                if (params.phyRemoteAutoneg.capabilities == MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS)
                {
                        printf("autoneg remote capabilities are unknown\n");
                }
                else
                {
                        printf("autoneg remote capabilities are ");
                        if (params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENHALF_CNS)
                        printf("10MH ");
                        if (params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENFULL_CNS)
                        printf("10MF ");
                        if (params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS)
                        printf("100MH ");
                        if (params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS)
                        printf("100MF ");
                        if (params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS)
                        printf("1GH ");
                        if (params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS)
                        printf("1GF ");
                        if (params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS)
                        printf("2.5GF ");
                        if (params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS)
                        printf("5GF ");
                        if (params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS)
                        printf("10GF ");
                        if (params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_25G_FULL_CNS)
                        printf("25GF ");
                        if (params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_40G_FULL_CNS)
                        printf("40GF ");
                        if (params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_50G_FULL_CNS)
                        printf("50GF ");
                        if (params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_100G_FULL_CNS)
                        printf("100GF ");
                        printf("\n");
                }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_get_autoneg_remote_capabilities */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_autoneg_admin
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_autoneg_admin(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_AUTONEG_ADMIN_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("get autoneg admin operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        if (params.phyAutoneg.capabilities == MPD_AUTONEG_CAPABILITIES_DEFAULT_CNS)
        {
                printf("autoneg admin capabilities are default\n");
        }
        else
                if (params.phyAutoneg.capabilities == MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS)
                {
                        printf("autoneg admin capabilities are unknown\n");
                }
                else
                {
                        printf("autoneg admin capabilities are ");
                        if (params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENHALF_CNS)
                        printf("10MH ");
                        if (params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENFULL_CNS)
                        printf("10MF ");
                        if (params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS)
                        printf("100MH ");
                        if (params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS)
                        printf("100MF ");
                        if (params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS)
                        printf("1GH ");
                        if (params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS)
                        printf("1GF ");
                        if (params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS)
                        printf("2.5GF ");
                        if (params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS)
                        printf("5GF ");
                        if (params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS)
                        printf("10GF ");
                        if (params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_25G_FULL_CNS)
                        printf("25GF ");
                        if (params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_40G_FULL_CNS)
                        printf("40GF ");
                        if (params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_50G_FULL_CNS)
                        printf("50GF ");
                        if (params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_100G_FULL_CNS)
                        printf("100GF ");
                        printf("\n");
                }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_get_autoneg_admin */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_link_partner_pause_capable
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_link_partner_pause_capable(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("get link partner pause capable operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        if (params.phyLinkPartnerFc.pauseCapable == TRUE)
        {
                printf("link partner is pause capable\n");
        }
        else
        {
                printf("link partner is not pause capable\n");
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_get_link_partner_pause_capable */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_internal_oper_status
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_internal_oper_status(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("get internal oper status operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        if (params.phyInternalOperStatus.isOperStatusUp == FALSE)
        {
                printf("relIfindex %d internal oper status is not UP\n", rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        printf("relIfindex %d internal oper status is UP\n", rel_ifIndex);

        if (params.phyInternalOperStatus.phySpeed == MPD_SPEED_LAST_E)
                        {
                printf("speed in NA\n");
        }
        else
        {
                switch (params.phyInternalOperStatus.phySpeed)
                {
                        case MPD_SPEED_10M_E:
                                printf("speed is 10M\n");
                        break;
                        case MPD_SPEED_100M_E:
                                printf("speed is 100M\n");
                        break;
                        case MPD_SPEED_1000M_E:
                                printf("speed is 1000M\n");
                        break;
                        case MPD_SPEED_10000M_E:
                                printf("speed is 10000M\n");
                        break;
                        case MPD_SPEED_2500M_E:
                                printf("speed is 2500M\n");
                        break;
                        case MPD_SPEED_5000M_E:
                                printf("speed is 5000M\n");
                        break;
                        case MPD_SPEED_20000M_E:
                                printf("speed is 20000M\n");
                        break;
                        default:
                                printf("speed support should be added\n");
                }
                if (params.phyInternalOperStatus.duplexMode == TRUE)
                {
                        printf("duplex mode is full\n");
                }
                else
                {
                        printf("duplex mode is half\n");
                }
        }
        if (params.phyInternalOperStatus.mediaType == MPD_COMFIG_PHY_MEDIA_TYPE_COPPER_E)
                        {
                printf("media type is copper\n");
        }
        else
                if (params.phyInternalOperStatus.mediaType == MPD_COMFIG_PHY_MEDIA_TYPE_FIBER_E)
                                {
                        printf("media type is fiber\n");
                }
                else
                {
                        printf("media type is invalid\n");
                }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_get_internal_oper_status */


/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_green_power_consumption
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_green_power_consumption(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        char error[20] = "not supported";
        MPD_RESULT_ENT mpd_result;
        BOOLEAN getMax = FALSE, portUp = FALSE;
        const char *energyDetectStr;
        const char *shortReachStr;
        const char *portSpeedStr;
        MPD_GREEN_SET_ENT shortReach = MPD_GREEN_NO_SET_E, energyDetetct = MPD_GREEN_NO_SET_E;
        MPD_SPEED_ENT portSpeed = MPD_SPEED_LAST_E;
    
        if (lua_gettop(L) == 6) {
            rel_ifIndex = lua_tointeger(L, 1);
            getMax = lua_toboolean(L, 2);
            energyDetectStr = lua_tostring(L, 3);
            shortReachStr = lua_tostring(L, 4);
            portUp = lua_toboolean(L, 5);
            portSpeedStr = lua_tostring(L, 6); 
          
            if (strcmp(energyDetectStr, "MPD_GREEN_NO_SET_E") == 0)
                energyDetetct = MPD_GREEN_NO_SET_E;
            else if (strcmp(energyDetectStr, "MPD_GREEN_ENABLE_E") == 0)
                energyDetetct = MPD_GREEN_ENABLE_E; 
            else if (strcmp(energyDetectStr, "MPD_GREEN_DISABLE_E") == 0)
                energyDetetct = MPD_GREEN_DISABLE_E;
            if (strcmp(shortReachStr, "MPD_GREEN_NO_SET_E") == 0)
                shortReach = MPD_GREEN_NO_SET_E;
            else if (strcmp(shortReachStr, "MPD_GREEN_ENABLE_E") == 0)
                shortReach = MPD_GREEN_ENABLE_E; 
            else if (strcmp(shortReachStr, "MPD_GREEN_DISABLE_E") == 0)
                shortReach = MPD_GREEN_DISABLE_E;
            if (strcmp(portSpeedStr, "MPD_SPEED_10M_E") == 0)
                portSpeed = MPD_SPEED_10M_E;
            else if (strcmp(portSpeedStr, "MPD_SPEED_100M_E") == 0)
                portSpeed = MPD_SPEED_100M_E;
            if (strcmp(portSpeedStr, "MPD_SPEED_1000M_E") == 0)
                portSpeed = MPD_SPEED_1000M_E;
            else if (strcmp(portSpeedStr, "MPD_SPEED_10000M_E") == 0)
                portSpeed = MPD_SPEED_10000M_E;
            if (strcmp(portSpeedStr, "MPD_SPEED_2500M_E") == 0)
                portSpeed = MPD_SPEED_2500M_E;
            else if (strcmp(portSpeedStr, "MPD_SPEED_5000M_E") == 0)
                portSpeed = MPD_SPEED_5000M_E;
            if (strcmp(portSpeedStr, "MPD_SPEED_20000M_E") == 0)
                portSpeed = MPD_SPEED_20000M_E;
            else if (strcmp(portSpeedStr, "MPD_SPEED_40G_E") == 0)
                portSpeed = MPD_SPEED_40G_E;
            if (strcmp(portSpeedStr, "MPD_SPEED_100G_E") == 0)
                portSpeed = MPD_SPEED_100G_E;
            else if (strcmp(portSpeedStr, "MPD_SPEED_50G_E") == 0)
                portSpeed = MPD_SPEED_50G_E;
            if (strcmp(portSpeedStr, "MPD_SPEED_25G_E") == 0)
                portSpeed = MPD_SPEED_25G_E;
            else if (strcmp(portSpeedStr, "MPD_SPEED_200G_E") == 0)
                portSpeed = MPD_SPEED_200G_E;
            if (strcmp(portSpeedStr, "MPD_SPEED_400G_E") == 0)
                portSpeed = MPD_SPEED_400G_E;
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        params.phyPowerConsumptions.shortReach = shortReach;
        params.phyPowerConsumptions.energyDetetct = energyDetetct;
        params.phyPowerConsumptions.portSpeed = portSpeed;
        params.phyPowerConsumptions.getMax = getMax; 
        params.phyPowerConsumptions.portUp = portUp;
        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E, &params);
        if (mpd_result != MPD_OK_E){
            if (mpd_result == MPD_OP_FAILED_E){
                    sprintf(error, "failed");
            }
            printf("get green power consumption operation is %s on relIfindex %d\n", error, rel_ifIndex);
            lua_pushnumber(L, 0);
            return 1;
        }

        printf("get green power consumption operation on relIfindex %d is %d\n", 
                rel_ifIndex, params.phyPowerConsumptions.greenConsumption);

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_get_green_power_consumption */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_green_readiness
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_green_readiness(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        char error[20] = "not supported";
        MPD_RESULT_ENT mpd_result;
        const char *readinessTypeStr;
        MPD_GREEN_READINESS_TYPE_ENT readinessType = MPD_GREEN_READINESS_TYPE_SR_E;
        
        if (lua_gettop(L) == 2) {
            rel_ifIndex = lua_tointeger(L, 1);
            readinessTypeStr = lua_tostring(L, 2);
            if (strcmp(readinessTypeStr, "MPD_GREEN_READINESS_TYPE_SR_E") == 0)
                readinessType = MPD_GREEN_READINESS_TYPE_SR_E;
            else if (strcmp(readinessTypeStr, "MPD_GREEN_READINESS_TYPE_ED_E") == 0){
                readinessType = MPD_GREEN_READINESS_TYPE_ED_E;
            }
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        params.phyGreen.type = readinessType;

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_GREEN_READINESS_E, &params);
        if (mpd_result != MPD_OK_E){
            if (mpd_result == MPD_OP_FAILED_E){
                    sprintf(error, "failed");
            }
            printf("get green readiness operation is %s on relIfindex %d\n", error, rel_ifIndex);
            lua_pushnumber(L, 0);
            return 1;
        }

        printf("get green readiness is on relIfindex %d:\n", rel_ifIndex);
        printf("readiness: %s\n");
        if (params.phyGreen.readiness == MPD_GREEN_READINESS_OPRNORMAL_E){
            printf("OPRNORMAL\n");
        }
        else if (params.phyGreen.readiness == MPD_GREEN_READINESS_FIBER_E){
            printf("FIBER\n");
        }
        else if (params.phyGreen.readiness == MPD_GREEN_READINESS_COMBO_FIBER_E){
            printf("COMBO FIBER\n");
        }
        else if (params.phyGreen.readiness == MPD_GREEN_READINESS_NOT_SUPPORTED_E){
            printf("NOT SUPPORTED\n");
        }
        else if (params.phyGreen.readiness == MPD_GREEN_READINESS_ALWAYS_ENABLED_E){
            printf("ALWAYS_ENABLED\n");
        }

        printf("Short reach speeds are %d:\n");
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_10M_CNS){
            printf("10M ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_10M_HD_CNS){
            printf("10M_HD ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_100M_CNS){
            printf("100M ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_100M_HD_CNS){
            printf("100M_HD ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_1G_CNS){
            printf("1G ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_10G_CNS){
            printf("10G ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_2500M_CNS){
            printf("2500M ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_5G_CNS){
            printf("5G ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_12G_CNS){
            printf("12G ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_16G_CNS){
            printf("16G ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_13600M_CNS){
            printf("13600M ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_20G_CNS){
            printf("20G ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_40G_CNS){
            printf("40G ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_100G_CNS){
            printf("100G ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_25G_CNS){
            printf("25G ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_50G_CNS){
            printf("50G ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_24G_CNS){
            printf("24G ");
        }
        if (params.phyGreen.srSpeeds & MPD_SPEED_CAPABILITY_200G_CNS){
            printf("200G ");
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_get_green_readiness */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_cable_len_no_range
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_cable_len_no_range(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E, &params);
        if (mpd_result != MPD_OK_E){
            if (mpd_result == MPD_OP_FAILED_E)
                            {
                    sprintf(error, "failed");
            }
            printf("get cable len no range operation is %s on relIfindex %d\n", error, rel_ifIndex);
            lua_pushnumber(L, 0);
            return 1;
        }
        
        printf("Cable length is %d\n", params.phyCableLenNoRange.cableLen);

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_get_cable_len_no_range */


/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_phy_kind_and_media
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_phy_kind_and_media(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E, &params);
        if (mpd_result != MPD_OK_E){
                if (mpd_result == MPD_OP_FAILED_E)
                {
                        sprintf(error, "failed");
                }
                printf("get kind and media operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        printf("get kind and media operation on relIfindex %d:\n", rel_ifIndex);
        printf("phyKind is: ");
        if (params.phyKindAndMedia.phyKind == MPD_KIND_COPPER_E){
            printf("COPPER\n");
        }
        else if (params.phyKindAndMedia.phyKind == MPD_KIND_SFP_E){
            printf("SFP\n");
        }
        else if (params.phyKindAndMedia.phyKind == MPD_KIND_COMBO_E){
            printf("COMBO\n");
        }
        else {
            printf("INVALID\n");
        }

        if (params.phyKindAndMedia.isSfpPresent){
            printf("Sfp Present:TRUE\n");
        }
        else {
            printf("Sfp Present:FALSE\n");
        }
        
        printf("Active media:\n");
        if (params.phyKindAndMedia.mediaType == MPD_COMFIG_PHY_MEDIA_TYPE_COPPER_E){
            printf("COPPER\n");
        }
        if (params.phyKindAndMedia.mediaType == MPD_COMFIG_PHY_MEDIA_TYPE_FIBER_E){
            printf("FIBER\n");
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_get_phy_kind_and_media */


/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_eee_capability
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_eee_capability(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_EEE_CAPABILITY_E, &params);
        if (mpd_result != MPD_OK_E){
                if (mpd_result == MPD_OP_FAILED_E)
                {
                        sprintf(error, "failed");
                }
                printf("get eee capability operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        printf("get eee capability operation on relIfindex %d:\n eee ability speeds are:\n", rel_ifIndex);

        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_10M_CNS){
            printf("10M ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_10M_HD_CNS){
            printf("10M_HD ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_100M_CNS){
            printf("100M ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_100M_HD_CNS){
            printf("100M_HD ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_1G_CNS){
            printf("1G ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_10G_CNS){
            printf("10G ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_2500M_CNS){
            printf("2500M ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_5G_CNS){
            printf("5G ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_12G_CNS){
            printf("12G ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_16G_CNS){
            printf("16G ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_13600M_CNS){
            printf("13600M ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_20G_CNS){
            printf("20G ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_40G_CNS){
            printf("40G ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_100G_CNS){
            printf("100G ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_25G_CNS){
            printf("25G ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_50G_CNS){
            printf("50G ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_24G_CNS){
            printf("24G ");
        }
        if (params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_200G_CNS){
            printf("200G ");
        }
        printf("\n");
        lua_pushnumber(L, 0);
        return 1;
}
/*$ END OF wr_utils_mpd_get_eee_capability */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_vct_capability
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_vct_capability(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_VCT_CAPABILITY_E, &params);
        if (mpd_result != MPD_OK_E){
                if (mpd_result == MPD_OP_FAILED_E)
                {
                        sprintf(error, "failed");
                }
                printf("get vct capability operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        printf("get vct capability operation on relIfindex %d: supported speeds are:\n", rel_ifIndex);
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_10M_CNS){
            printf("10M ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_10M_HD_CNS){
            printf("10M_HD ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_100M_CNS){
            printf("100M ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_100M_HD_CNS){
            printf("100M_HD ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_1G_CNS){
            printf("1G ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_10G_CNS){
            printf("10G ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_2500M_CNS){
            printf("2500M ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_5G_CNS){
            printf("5G ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_12G_CNS){
            printf("12G ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_16G_CNS){
            printf("16G ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_13600M_CNS){
            printf("13600M ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_20G_CNS){
            printf("20G ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_40G_CNS){
            printf("40G ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_100G_CNS){
            printf("100G ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_25G_CNS){
            printf("25G ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_50G_CNS){
            printf("50G ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_24G_CNS){
            printf("24G ");
        }
        if (params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_200G_CNS){
            printf("200G ");
        }

        lua_pushnumber(L, 0);
        return 1;
}

/*$ END OF wr_utils_mpd_get_vct_capability */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_dte_status
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_dte_status(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_DTE_STATUS_E, &params);
        if (mpd_result != MPD_OK_E){
                if (mpd_result == MPD_OP_FAILED_E)
                {
                        sprintf(error, "failed");
                }
                printf("get dte status operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        if (params.phyDteStatus.dteDetetcted == TRUE)
        {
                printf("dte status is detected\n");
        }
        else
        {
                printf("dte status is not detected\n");
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_get_dte_status */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_temperature
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_temperature(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_TEMPERATURE_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("get temperature operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        printf("temperature is %d\n", params.phyTemperature.temperature);

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_get_temperature */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_revision
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_revision(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_REVISION_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("get revision operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        switch (params.phyRevision.phyType)
        {
                case MPD_TYPE_88E1543_E:
                        printf("relIfindex %d phyType is 1543\n", rel_ifIndex);
                break;
                case MPD_TYPE_88E1545_E:
                        printf("relIfindex %d phyType is 1545\n", rel_ifIndex);
                break;
                case MPD_TYPE_88E1548_E:
                        printf("relIfindex %d phyType is 1548\n", rel_ifIndex);
                break;
                case MPD_TYPE_88E1680_E:
                        printf("relIfindex %d phyType is 1680\n", rel_ifIndex);
                break;
                case MPD_TYPE_88E1680L_E:
                        printf("relIfindex %d phyType is 1680L\n", rel_ifIndex);
                break;
                case MPD_TYPE_88E151x_E:
                        printf("relIfindex %d phyType is 151x\n", rel_ifIndex);
                break;
                case MPD_TYPE_88E3680_E:
                        printf("relIfindex %d phyType is 3680\n", rel_ifIndex);
                break;
                case MPD_TYPE_88X32x0_E:
                        printf("relIfindex %d phyType is 32x0\n", rel_ifIndex);
                break;
                case MPD_TYPE_88X33x0_E:
                        printf("relIfindex %d phyType is 33x0\n", rel_ifIndex);
                break;
                case MPD_TYPE_88X20x0_E:
                        printf("relIfindex %d phyType is 1543\n", rel_ifIndex);
                break;
                case MPD_TYPE_88X2180_E:
                        printf("relIfindex %d phyType is 2180\n", rel_ifIndex);
                break;
                default:
                        printf("relIfindex %d has no PHY\n", rel_ifIndex);
        }
        printf("revision is %d\n", params.phyRevision.revision);

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_get_revision */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_get_autoneg_support
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_get_autoneg_support(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_AUTONEG_SUPPORT_E, &params);
        if (mpd_result != MPD_OK_E){
                if (mpd_result == MPD_OP_FAILED_E)
                {
                        sprintf(error, "failed");
                }
                printf("get autoneg support operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        printf("get autoneg support operation on relIfindex %d: PhyAutoneg capabilities capabilities are\n", rel_ifIndex);
        if (params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_TENHALF_CNS)
            printf("10MH ");
        if (params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_TENFULL_CNS)
            printf("10MF ");
        if (params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS)
            printf("100MH ");
        if (params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS)
            printf("100MF ");
        if (params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS)
            printf("1GH ");
        if (params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS)
            printf("1GF ");
        if (params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS)
            printf("2.5GF ");
        if (params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS)
            printf("5GF ");
        if (params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS)
            printf("10GF ");
        if (params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_25G_FULL_CNS)
            printf("25GF ");
        if (params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_40G_FULL_CNS)
            printf("40GF ");
        if (params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_50G_FULL_CNS)
            printf("50GF ");
        if (params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_100G_FULL_CNS)
            printf("100GF ");
        
        printf("\n");
        
        lua_pushnumber(L, 0);
        return 1;     
}
/*$ END OF wr_utils_mpd_get_autoneg_support */


/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_autoneg_enable
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_autoneg_enable(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        const char *autonegStr1 = "", *autonegStr2 = "", *autonegStr3 = "", *autonegStr4 = "", *autonegStr5 = "", *autonegStr6 = "", *autonegStr7 = "",
                        *autonegStr8 = "";
        MPD_AUTONEGPREFERENCE_ENT autoneg_preference;
        MPD_AUTONEG_CAPABILITIES_TYP autoneg_capabilities = 0;
        if (lua_gettop(L) >= 3) {
                params.phyAutoneg.enable = MPD_AUTO_NEGOTIATION_ENABLE_E;
                rel_ifIndex = lua_tointeger(L, 1);
                autonegStr1 = lua_tostring(L, 2);
                if (strcmp(autonegStr1, "MPD_AUTONEGPREFERENCE_MASTER_E") == 0)
                autoneg_preference = MPD_AUTONEGPREFERENCE_MASTER_E;
                else
                autoneg_preference = MPD_AUTONEGPREFERENCE_SLAVE_E;
                params.phyAutoneg.masterSlave = autoneg_preference;
                autonegStr1 = lua_tostring(L, 3);
                if (lua_gettop(L) >= 4)
                autonegStr2 = lua_tostring(L, 4);
                if (lua_gettop(L) >= 5)
                autonegStr3 = lua_tostring(L, 5);
                if (lua_gettop(L) >= 6)
                autonegStr4 = lua_tostring(L, 6);
                if (lua_gettop(L) >= 7)
                autonegStr5 = lua_tostring(L, 7);
                if (lua_gettop(L) >= 8)
                autonegStr6 = lua_tostring(L, 8);
                if (lua_gettop(L) >= 9)
                autonegStr7 = lua_tostring(L, 9);
                if (lua_gettop(L) >= 10)
                autonegStr8 = lua_tostring(L, 10);
                if (lua_gettop(L) >= 11)
                                {
                        lua_pushnil(L);
                        return 0;
                }
                if ((strcmp(autonegStr1, "MPD_AUTONEG_CAPABILITIES_TENHALF_CNS") == 0) || (strcmp(autonegStr2, "MPD_AUTONEG_CAPABILITIES_TENHALF_CNS") == 0)
                                || (strcmp(autonegStr3, "MPD_AUTONEG_CAPABILITIES_TENHALF_CNS") == 0) || (strcmp(autonegStr4, "MPD_AUTONEG_CAPABILITIES_TENHALF_CNS") == 0)
                                || (strcmp(autonegStr5, "MPD_AUTONEG_CAPABILITIES_TENHALF_CNS") == 0) || (strcmp(autonegStr6, "MPD_AUTONEG_CAPABILITIES_TENHALF_CNS") == 0)
                                || (strcmp(autonegStr7, "MPD_AUTONEG_CAPABILITIES_TENHALF_CNS") == 0) || (strcmp(autonegStr8, "MPD_AUTONEG_CAPABILITIES_TENHALF_CNS") == 0))
                autoneg_capabilities |= MPD_AUTONEG_CAPABILITIES_TENHALF_CNS;
                if ((strcmp(autonegStr1, "MPD_AUTONEG_CAPABILITIES_TENFULL_CNS") == 0) || (strcmp(autonegStr2, "MPD_AUTONEG_CAPABILITIES_TENFULL_CNS") == 0)
                                || (strcmp(autonegStr3, "MPD_AUTONEG_CAPABILITIES_TENFULL_CNS") == 0) || (strcmp(autonegStr4, "MPD_AUTONEG_CAPABILITIES_TENFULL_CNS") == 0)
                                || (strcmp(autonegStr5, "MPD_AUTONEG_CAPABILITIES_TENFULL_CNS") == 0) || (strcmp(autonegStr6, "MPD_AUTONEG_CAPABILITIES_TENFULL_CNS") == 0)
                                || (strcmp(autonegStr7, "MPD_AUTONEG_CAPABILITIES_TENFULL_CNS") == 0) || (strcmp(autonegStr8, "MPD_AUTONEG_CAPABILITIES_TENFULL_CNS") == 0))
                autoneg_capabilities |= MPD_AUTONEG_CAPABILITIES_TENFULL_CNS;
                if ((strcmp(autonegStr1, "MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS") == 0) || (strcmp(autonegStr2, "MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS") == 0)
                                || (strcmp(autonegStr3, "MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS") == 0) || (strcmp(autonegStr4, "MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS") == 0)
                                || (strcmp(autonegStr5, "MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS") == 0) || (strcmp(autonegStr6, "MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS") == 0)
                                || (strcmp(autonegStr7, "MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS") == 0) || (strcmp(autonegStr8, "MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS") == 0))
                autoneg_capabilities |= MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS;
                if ((strcmp(autonegStr1, "MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS") == 0) || (strcmp(autonegStr2, "MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS") == 0)
                                || (strcmp(autonegStr3, "MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS") == 0) || (strcmp(autonegStr4, "MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS") == 0)
                                || (strcmp(autonegStr5, "MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS") == 0) || (strcmp(autonegStr6, "MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS") == 0)
                                || (strcmp(autonegStr7, "MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS") == 0) || (strcmp(autonegStr8, "MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS") == 0))
                autoneg_capabilities |= MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS;
                if ((strcmp(autonegStr1, "MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS") == 0) || (strcmp(autonegStr2, "MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS") == 0)
                                || (strcmp(autonegStr3, "MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS") == 0) || (strcmp(autonegStr4, "MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS") == 0)
                                || (strcmp(autonegStr5, "MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS") == 0) || (strcmp(autonegStr6, "MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS") == 0)
                                || (strcmp(autonegStr7, "MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS") == 0) || (strcmp(autonegStr8, "MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS") == 0))
                autoneg_capabilities |= MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS;
                if ((strcmp(autonegStr1, "MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS") == 0) || (strcmp(autonegStr2, "MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS") == 0)
                                || (strcmp(autonegStr3, "MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS") == 0) || (strcmp(autonegStr4, "MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS") == 0)
                                || (strcmp(autonegStr5, "MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS") == 0) || (strcmp(autonegStr6, "MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS") == 0)
                                || (strcmp(autonegStr7, "MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS") == 0) || (strcmp(autonegStr8, "MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS") == 0))
                autoneg_capabilities |= MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS;
                if ((strcmp(autonegStr1, "MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS") == 0) || (strcmp(autonegStr2, "MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS") == 0)
                                || (strcmp(autonegStr3, "MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS") == 0) || (strcmp(autonegStr4, "MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS") == 0)
                                || (strcmp(autonegStr5, "MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS") == 0) || (strcmp(autonegStr6, "MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS") == 0)
                                || (strcmp(autonegStr7, "MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS") == 0) || (strcmp(autonegStr8, "MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS") == 0))
                autoneg_capabilities |= MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS;
                if ((strcmp(autonegStr1, "MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS") == 0) || (strcmp(autonegStr2, "MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS") == 0)
                                || (strcmp(autonegStr3, "MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS") == 0) || (strcmp(autonegStr4, "MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS") == 0)
                                || (strcmp(autonegStr5, "MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS") == 0) || (strcmp(autonegStr6, "MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS") == 0)
                                || (strcmp(autonegStr7, "MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS") == 0) || (strcmp(autonegStr8, "MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS") == 0))
                autoneg_capabilities |= MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS;
                params.phyAutoneg.capabilities = autoneg_capabilities;
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_AUTONEG_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set autoneg enable operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_autoneg_enable */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_autoneg_disable
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_autoneg_disable(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                params.phyAutoneg.enable = MPD_AUTO_NEGOTIATION_DISABLE_E;
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_AUTONEG_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set autoneg disable operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_autoneg_disable */


/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_restart_autoneg
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_restart_autoneg(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_RESTART_AUTONEG_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set restart autoneg operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_restart_autoneg */


/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_duplex_mode
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_duplex_mode(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        const char *duplex_modeStr;
        MPD_DUPLEX_ADMIN_ENT duplex_mode;
        if (lua_gettop(L) == 2) {
                rel_ifIndex = lua_tointeger(L, 1);
                duplex_modeStr = lua_tostring(L, 2);
                if (strcmp(duplex_modeStr, "MPD_DUPLEX_ADMIN_MODE_HALF_E") == 0)
                duplex_mode = MPD_DUPLEX_ADMIN_MODE_HALF_E;
                else
                duplex_mode = MPD_DUPLEX_ADMIN_MODE_FULL_E;
                params.phyDuplex.mode = duplex_mode;
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_DUPLEX_MODE_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set duplex mode operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_duplex_mode */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_speed
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_speed(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        const char *speedStr;
        MPD_SPEED_ENT port_speed;
        if (lua_gettop(L) == 2) {
                rel_ifIndex = lua_tointeger(L, 1);
                speedStr = lua_tostring(L, 2);
                if (strcmp(speedStr, "MPD_SPEED_10M_E") == 0)
                port_speed = MPD_SPEED_10M_E;
                else
                        if (strcmp(speedStr, "MPD_SPEED_100M_E") == 0)
                        port_speed = MPD_SPEED_100M_E;
                        else
                                if (strcmp(speedStr, "MPD_SPEED_1000M_E") == 0)
                                port_speed = MPD_SPEED_1000M_E;
                                else
                                        if (strcmp(speedStr, "MPD_SPEED_2500M_E") == 0)
                                        port_speed = MPD_SPEED_2500M_E;
                                        else
                                                if (strcmp(speedStr, "MPD_SPEED_5000M_E") == 0)
                                                port_speed = MPD_SPEED_5000M_E;
                                                else
                                                        if (strcmp(speedStr, "MPD_SPEED_10000M_E") == 0)
                                                        port_speed = MPD_SPEED_10000M_E;
                                                        else
                                                                if (strcmp(speedStr, "MPD_SPEED_20000M_E") == 0)
                                                                port_speed = MPD_SPEED_20000M_E;
                                                                else
                                                                {
                                                                        lua_pushnil(L);
                                                                        return 0;
                                                                }
                params.phySpeed.speed = port_speed;
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_SPEED_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set speed operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_speed */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_combo_media_type
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_combo_media_type(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        const char *str;
        MPD_SPEED_ENT port_speed;
        MPD_COMBO_MODE_ENT combo_mode;
        if (lua_gettop(L) == 4) {
                rel_ifIndex = lua_tointeger(L, 1);
                params.internal.phyFiberParams.fiberPresent = lua_tointeger(L, 2);
                str = lua_tostring(L, 3);
                if (strcmp(str, "MPD_COMBO_MODE_FORCE_FIBER_E") == 0)
                combo_mode = MPD_COMBO_MODE_FORCE_FIBER_E;
                else
                        if (strcmp(str, "MPD_COMBO_MODE_FORCE_COPPER_E") == 0)
                        combo_mode = MPD_COMBO_MODE_FORCE_COPPER_E;
                        else
                                if (strcmp(str, "MPD_COMBO_MODE_PREFER_FIBER_E") == 0)
                                combo_mode = MPD_COMBO_MODE_PREFER_FIBER_E;
                                else
                                        if (strcmp(str, "MPD_COMBO_MODE_PREFER_COPPER_E") == 0)
                                        combo_mode = MPD_COMBO_MODE_PREFER_COPPER_E;
                                        else
                                        {
                                                lua_pushnil(L);
                                                return 0;
                                        }
                params.internal.phyFiberParams.comboMode = combo_mode;
                str = lua_tostring(L, 4);
                if (strcmp(str, "MPD_SPEED_10M_E") == 0)
                port_speed = MPD_SPEED_10M_E;
                else
                        if (strcmp(str, "MPD_SPEED_100M_E") == 0)
                        port_speed = MPD_SPEED_100M_E;
                        else
                                if (strcmp(str, "MPD_SPEED_1000M_E") == 0)
                                port_speed = MPD_SPEED_1000M_E;
                                else
                                        if (strcmp(str, "MPD_SPEED_2500M_E") == 0)
                                        port_speed = MPD_SPEED_2500M_E;
                                        else
                                                if (strcmp(str, "MPD_SPEED_5000M_E") == 0)
                                                port_speed = MPD_SPEED_5000M_E;
                                                else
                                                        if (strcmp(str, "MPD_SPEED_10000M_E") == 0)
                                                        port_speed = MPD_SPEED_10000M_E;
                                                        else
                                                                if (strcmp(str, "MPD_SPEED_20000M_E") == 0)
                                                                port_speed = MPD_SPEED_20000M_E;
                                                                else
                                                                {
                                                                        lua_pushnil(L);
                                                                        return 0;
                                                                }
                params.internal.phyFiberParams.phySpeed = port_speed;
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, PRV_MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set combo media type operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_combo_media_type */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_mdix_mode
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_mdix_mode(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        const char *mdix_modeStr;
        MPD_MDIX_MODE_TYPE_ENT mdix_mode;
        if (lua_gettop(L) == 2) {
                rel_ifIndex = lua_tointeger(L, 1);
                mdix_modeStr = lua_tostring(L, 2);
                if (strcmp(mdix_modeStr, "MPD_MDI_MODE_MEDIA_E") == 0)
                mdix_mode = MPD_MDI_MODE_MEDIA_E;
                else
                        if (strcmp(mdix_modeStr, "MPD_MDIX_MODE_MEDIA_E") == 0)
                        mdix_mode = MPD_MDIX_MODE_MEDIA_E;
                        else
                        mdix_mode = MPD_AUTO_MODE_MEDIA_E;
                params.phyMdix.mode = mdix_mode;
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_MDIX_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set duplex mode operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_mdix_mode */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_vct_test
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_vct_test(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_VCT_TEST_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set vct test operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        if (params.phyVct.testResult == MPD_VCT_RESULT_CABLE_OK_E)
        printf("vct test result is ok\n");
        else
                if (params.phyVct.testResult == MPD_VCT_RESULT_2_PAIR_CABLE_E)
                printf("vct test result is 2 pair cable\n");
                else
                        if (params.phyVct.testResult == MPD_VCT_RESULT_NO_CABLE_E)
                        printf("vct test result is no cable\n");
                        else
                                if (params.phyVct.testResult == MPD_VCT_RESULT_OPEN_CABLE_E)
                                printf("vct test result is open cable\n");
                                else
                                        if (params.phyVct.testResult == MPD_VCT_RESULT_SHORT_CABLE_E)
                                        printf("vct test result is short cable\n");
                                        else
                                                if (params.phyVct.testResult == MPD_VCT_RESULT_BAD_CABLE_E)
                                                printf("vct test result is bad cable\n");
                                                else
                                                printf("vct test result is impedance misMmatch\n");
        printf("vct cable length is %d\n", params.phyVct.cableLength);

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_vct_test */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_reset_phy
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_reset_phy(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_RESET_PHY_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set reset phy operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_reset_phy */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_phy_disable_oper
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_phy_disable_oper(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        BOOLEAN forceLinkDown;
        if (lua_gettop(L) == 2) {
                rel_ifIndex = lua_tointeger(L, 1);
                forceLinkDown = lua_tointeger(L, 2);
                params.phyDisable.forceLinkDown = forceLinkDown;
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_PHY_DISABLE_OPER_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set disable oper operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_phy_disable_oper */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_advertise_fc
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_advertise_fc(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        BOOLEAN advertiseFc;
        if (lua_gettop(L) == 2) {
                rel_ifIndex = lua_tointeger(L, 1);
                advertiseFc = lua_tointeger(L, 2);
                params.phyFc.advertiseFc = advertiseFc;
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_ADVERTISE_FC_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set advertise fc operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_advertise_fc */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_present_notification
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_present_notification(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 3) {
                rel_ifIndex = lua_tointeger(L, 1);
                params.phySfpPresentNotification.isSfpPresent = lua_tointeger(L, 2);
                params.phySfpPresentNotification.sfpWa = lua_tointeger(L, 3);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set present notification operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_present_notification */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_smi_access
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_smi_access(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        MPD_MDIO_ACCESS_TYPE_ENT smi_access_type;
        rel_ifIndex = lua_tointeger(L, 1);
        if (lua_gettop(L) == 4)
                        {
                smi_access_type = lua_tointeger(L, 2);
                if (smi_access_type == MPD_MDIO_ACCESS_WRITE_E)
                {
                        printf("set smi access type write: omitted data parameter on relIfindex %d\n", rel_ifIndex);
                        lua_pushnumber(L, 0);
                        return 1;
                }
        }
        else
                if (lua_gettop(L) == 5)
                                {
                        smi_access_type = lua_tointeger(L, 2);
                        if (smi_access_type == MPD_MDIO_ACCESS_READ_E)
                        {
                                printf("set smi access type read: unnecessary data parameter on relIfindex %d\n", rel_ifIndex);
                                lua_pushnumber(L, 0);
                                return 1;
                        }
                        params.phyMdioAccess.data = lua_tointeger(L, 5);
                }
                else
                {
                        lua_pushnil(L);
                        return 0;
                }

        params.phyMdioAccess.type = smi_access_type;
        params.phyMdioAccess.deviceOrPage = lua_tointeger(L, 3);
        params.phyMdioAccess.address = lua_tointeger(L, 4);

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_MDIO_ACCESS_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set smi access operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        if (smi_access_type == MPD_MDIO_ACCESS_READ_E)
        {
                printf("data that is read is %d\n", params.phyMdioAccess.data);
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_smi_access */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_xsmi_access
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_xsmi_access(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        MPD_MDIO_ACCESS_TYPE_ENT xsmi_access_type;
        rel_ifIndex = lua_tointeger(L, 1);
        if (lua_gettop(L) == 4)
                        {
                xsmi_access_type = lua_tointeger(L, 2);
                if (xsmi_access_type == MPD_MDIO_ACCESS_WRITE_E)
                {
                        printf("set xsmi access type write: omitted data parameter on relIfindex %d\n", rel_ifIndex);
                        lua_pushnumber(L, 0);
                        return 1;
                }
        }
        else
                if (lua_gettop(L) == 5)
                                {
                        xsmi_access_type = lua_tointeger(L, 2);
                        if (xsmi_access_type == MPD_MDIO_ACCESS_READ_E)
                        {
                                printf("set xsmi access type read: unnecessary data parameter on relIfindex %d\n", rel_ifIndex);
                                lua_pushnumber(L, 0);
                                return 1;
                        }
                        params.phyMdioAccess.data = lua_tointeger(L, 5);
                }
                else
                {
                        lua_pushnil(L);
                        return 0;
                }

        params.phyMdioAccess.type = xsmi_access_type;
        params.phyMdioAccess.deviceOrPage = lua_tointeger(L, 3);
        params.phyMdioAccess.address = lua_tointeger(L, 4);

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_MDIO_ACCESS_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set xsmi access operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        if (xsmi_access_type == MPD_MDIO_ACCESS_READ_E)
        {
                printf("data that is read is %d\n", params.phyMdioAccess.data);
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_xsmi_access */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_loop_back
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_loop_back(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        BOOLEAN loop_back_mode;
        if (lua_gettop(L) == 2) {
                rel_ifIndex = lua_tointeger(L, 1);
                loop_back_mode = lua_tointeger(L, 2);
                params.internal.phyLoopback.enable = loop_back_mode;
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, PRV_MPD_OP_CODE_SET_LOOP_BACK_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set loop back operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_loop_back */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_check_link_up
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_check_link_up(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set check link up operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        if (params.internal.phyLinkStatus.phyValidStatus == TRUE)
        printf("phy valid status is TRUE\n");
        else
        printf("phy valid status is FALSE\n");

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_check_link_up */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_power_modules
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_power_modules(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        const char *energyDetectStr;
        const char *shortReachStr;
        BOOLEAN resetPhy = FALSE;
        MPD_GREEN_SET_ENT shortReach = MPD_GREEN_NO_SET_E, energyDetetct = MPD_GREEN_NO_SET_E;
        
        char error[20] = "not supported";
        if (lua_gettop(L) == 4) {
                rel_ifIndex = lua_tointeger(L, 1);
                energyDetectStr = lua_tostring(L, 2);
                shortReachStr = lua_tostring(L, 3);
                resetPhy = lua_toboolean(L, 4);
                if (strcmp(energyDetectStr, "MPD_GREEN_NO_SET_E") == 0)
                    energyDetetct = MPD_GREEN_NO_SET_E;
                else if (strcmp(energyDetectStr, "MPD_GREEN_ENABLE_E") == 0)
                    energyDetetct = MPD_GREEN_ENABLE_E; 
                else if (strcmp(energyDetectStr, "MPD_GREEN_DISABLE_E") == 0)
                    energyDetetct = MPD_GREEN_DISABLE_E;
                if (strcmp(shortReachStr, "MPD_GREEN_NO_SET_E") == 0)
                    shortReach = MPD_GREEN_NO_SET_E;
                else if (strcmp(shortReachStr, "MPD_GREEN_ENABLE_E") == 0)
                    shortReach = MPD_GREEN_ENABLE_E; 
                else if (strcmp(shortReachStr, "MPD_GREEN_DISABLE_E") == 0)
                    shortReach = MPD_GREEN_DISABLE_E;
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        params.phyPowerModules.energyDetetct = energyDetetct;
        params.phyPowerModules.shortReach = shortReach;
        params.phyPowerModules.performPhyReset = resetPhy;

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_POWER_MODULES_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set power modules is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_check_link_up */


/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_eee_advertize_capability
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_eee_advertize_capability(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        const char *speedStr;
        MPD_SPEED_ENT speed;
        BOOLEAN advEnable =FALSE;
    
        if (lua_gettop(L) == 3) {
                rel_ifIndex = lua_tointeger(L, 1);
                advEnable = lua_toboolean(L, 2);
                speedStr = lua_tostring(L, 3);
                if (strcmp(speedStr, "MPD_SPEED_10M_E") == 0)
                speed = MPD_SPEED_10M_E;
                else
                        if (strcmp(speedStr, "MPD_SPEED_100M_E") == 0)
                        speed = MPD_SPEED_100M_E;
                        else
                                if (strcmp(speedStr, "MPD_SPEED_1000M_E") == 0)
                                speed = MPD_SPEED_1000M_E;
                                else
                                        if (strcmp(speedStr, "MPD_SPEED_2500M_E") == 0)
                                        speed = MPD_SPEED_2500M_E;
                                        else
                                                if (strcmp(speedStr, "MPD_SPEED_5000M_E") == 0)
                                                speed = MPD_SPEED_5000M_E;
                                                else
                                                        if (strcmp(speedStr, "MPD_SPEED_10000M_E") == 0)
                                                        speed = MPD_SPEED_10000M_E;
                                                        else
                                                                if (strcmp(speedStr, "MPD_SPEED_20000M_E") == 0)
                                                                speed = MPD_SPEED_20000M_E;
                                                                else
                                                                {
                                                                        lua_pushnil(L);
                                                                        return 0;
                                                                }
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        params.phyEeeAdvertize.advEnable = advEnable;
        params.phyEeeAdvertize.speedBitmap = speed;
        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set eee advertize operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_eee_advertize_capability */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_eee_master_enable
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_eee_master_enable(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        BOOLEAN masterEnable = FALSE;
    
        if (lua_gettop(L) == 2) {
                rel_ifIndex = lua_tointeger(L, 1);
                masterEnable = lua_toboolean(L, 2);
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        params.phyEeeMasterEnable.masterEnable = masterEnable;
        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set eee master enable operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_eee_master_enable */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_lpi_exit_time
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_lpi_exit_time(lua_State *L)
{
        UINT_32 rel_ifIndex;
        UINT_16 exitTime;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        const char *eeeSpeedStr;
        MPD_EEE_SPEED_ENT eeeSpeed;
    
        if (lua_gettop(L) == 3) {
                rel_ifIndex = lua_tointeger(L, 1);
                exitTime = lua_tointeger(L, 2);
                eeeSpeedStr = lua_tostring(L, 3);
                if (strcmp(eeeSpeedStr, "MPD_EEE_SPEED_10M_E") == 0)
                    eeeSpeed = MPD_EEE_SPEED_10M_E;
                else
                    if (strcmp(eeeSpeedStr, "MPD_EEE_SPEED_100M_E") == 0)
                        eeeSpeed = MPD_EEE_SPEED_100M_E;
                    else
                        if (strcmp(eeeSpeedStr, "MPD_EEE_SPEED_1G_E") == 0)
                            eeeSpeed = MPD_EEE_SPEED_1G_E;
                        else
                            if (strcmp(eeeSpeedStr, "MPD_EEE_SPEED_10G_E") == 0)
                                eeeSpeed = MPD_EEE_SPEED_10G_E;
                            else
                                if (strcmp(eeeSpeedStr, "MPD_EEE_SPEED_2500M_E") == 0)
                                    eeeSpeed = MPD_EEE_SPEED_2500M_E;
                                else
                                    if (strcmp(eeeSpeedStr, "MPD_EEE_SPEED_5G_E") == 0)
                                        eeeSpeed = MPD_EEE_SPEED_5G_E;
                                    else
                                    {
                                        lua_pushnil(L);
                                        return 0;
                                    }
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        params.phyEeeLpiTime.time_us = exitTime;
        params.phyEeeLpiTime.speed = eeeSpeed;

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_LPI_EXIT_TIME_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set lpi exit time operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_lpi_exit_time */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_set_lpi_enter_time
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_set_lpi_enter_time(lua_State *L)
{
        UINT_32 rel_ifIndex;
        UINT_16 exitTime;
        MPD_OPERATIONS_PARAMS_UNT params;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
        const char *eeeSpeedStr;
        MPD_EEE_SPEED_ENT eeeSpeed;
    
        if (lua_gettop(L) == 3) {
                rel_ifIndex = lua_tointeger(L, 1);
                exitTime = lua_tointeger(L, 2);
                eeeSpeedStr = lua_tostring(L, 3);
                if (strcmp(eeeSpeedStr, "MPD_EEE_SPEED_10M_E") == 0)
                    eeeSpeed = MPD_EEE_SPEED_10M_E;
                else
                    if (strcmp(eeeSpeedStr, "MPD_EEE_SPEED_100M_E") == 0)
                        eeeSpeed = MPD_EEE_SPEED_100M_E;
                    else
                        if (strcmp(eeeSpeedStr, "MPD_EEE_SPEED_1G_E") == 0)
                            eeeSpeed = MPD_EEE_SPEED_1G_E;
                        else
                            if (strcmp(eeeSpeedStr, "MPD_EEE_SPEED_10G_E") == 0)
                                eeeSpeed = MPD_EEE_SPEED_10G_E;
                            else
                                if (strcmp(eeeSpeedStr, "MPD_EEE_SPEED_2500M_E") == 0)
                                    eeeSpeed = MPD_EEE_SPEED_2500M_E;
                                else
                                    if (strcmp(eeeSpeedStr, "MPD_EEE_SPEED_5G_E") == 0)
                                        eeeSpeed = MPD_EEE_SPEED_5G_E;
                                    else
                                    {
                                        lua_pushnil(L);
                                        return 0;
                                    }
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        params.phyEeeLpiTime.time_us = exitTime;
        params.phyEeeLpiTime.speed = eeeSpeed;

        mpd_result = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_LPI_ENTER_TIME_E, &params);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("set lpi exit time operation is %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_set_lpi_enter_time */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_print_phy_db
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_print_phy_db(lua_State *L)
{
        UINT_32 rel_ifIndex = 0;
        BOOLEAN fullDb = FALSE;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
       
        mpd_result = prvMpdDebugPrintPortDb(rel_ifIndex, fullDb);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("print phy db %s %d\n", error);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_print_phy_db */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_print_phy_db_full
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_print_phy_db_full(lua_State *L)
{
        UINT_32 rel_ifIndex = 0;
        BOOLEAN fullDb = TRUE;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
 
        mpd_result = prvMpdDebugPrintPortDb(rel_ifIndex, fullDb);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("print phy db full %s %d\n", error);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_print_phy_db_full */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_mdio_c145_write
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_mdio_c145_write(lua_State *L)
{
        UINT_32 rel_ifIndex, dev, address, val;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
    
        if (lua_gettop(L) == 4) {
                rel_ifIndex = lua_tointeger(L, 1);
                dev = lua_tointeger(L, 2);
                address = lua_tointeger(L, 3);
                val = lua_tointeger(L, 4);
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        mpd_result = prvMpdMdioDebugCl45WriteRegister(rel_ifIndex, dev, address, val);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("mpd mdio c145 write %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_mdio_c145_write */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_mdio_c145_read
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_mdio_c145_read(lua_State *L)
{
        UINT_32 rel_ifIndex, dev, address;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
    
        if (lua_gettop(L) == 3) {
                rel_ifIndex = lua_tointeger(L, 1);
                dev = lua_tointeger(L, 2);
                address = lua_tointeger(L, 3);
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        mpd_result = prvMpdMdioDebugCl45ReadRegister(rel_ifIndex, dev, address);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("mpd mdio c145 read %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_mdio_c145_read */


/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_mdio_c122_write
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_mdio_c122_write(lua_State *L)
{
        UINT_32 rel_ifIndex, page, address, val;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
    
        if (lua_gettop(L) == 4) {
                rel_ifIndex = lua_tointeger(L, 1);
                page = lua_tointeger(L, 2);
                address = lua_tointeger(L, 3);
                val = lua_tointeger(L, 4);
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        mpd_result = prvMpdMdioDebugCl22WriteRegister(rel_ifIndex, page, address, val);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("mpd mdio c122 write %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_mdio_c122_write */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_mdio_c122_read
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_mdio_c122_read(lua_State *L)
{
        UINT_32 rel_ifIndex, page, address, readAll;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
    
        if (lua_gettop(L) == 3) {
                rel_ifIndex = lua_tointeger(L, 1);
                page = lua_tointeger(L, 2);
                address = lua_tointeger(L, 3);
                readAll = lua_toboolean(L, 4);
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        mpd_result = prvMpdMdioDebugCl22ReadRegister(readAll, rel_ifIndex, page, address);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("mpd mdio c122 read %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}
/*$ END OF wr_utils_mpd_mdio_c122_read */


/*****************************************************************************
 * FUNCTION NAME: wr_utils_mpd_mtd_debug_reg_dump
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_mpd_mtd_debug_reg_dump(lua_State *L)
{
        UINT_32 rel_ifIndex;
        MPD_RESULT_ENT mpd_result;
        char error[20] = "not supported";
    
        if (lua_gettop(L) == 1) {
                rel_ifIndex = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        mpd_result = prvMpdMtdDebugRegDump(rel_ifIndex);
        if (mpd_result != MPD_OK_E)
                        {
                if (mpd_result == MPD_OP_FAILED_E)
                                {
                        sprintf(error, "failed");
                }
                printf("mpd mtd debug reg dump %s on relIfindex %d\n", error, rel_ifIndex);
                lua_pushnumber(L, 0);
                return 1;
        }

        lua_pushnumber(L, 0);
        return 1;

}

/*$ END OF wr_utils_mpd_mtd_debug_reg_dump */