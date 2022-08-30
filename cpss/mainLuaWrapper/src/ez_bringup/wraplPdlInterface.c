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

#include <cpss/extServices/private/prvCpssBindFunc.h>

#include "wraplPdl.h"
#include "pdl/interface/pdlGpioDebug.h"
#include "pdl/interface/pdlI2CDebug.h"
#include "pdl/interface/pdlInterfaceDebug.h"
#include "pdl/interface/private/prvPdlInterface.h"
#include "iDbgPdl/utils/iDbgPdlUtils.h"
#include "iDbgPdl/logger/iDbgPdlLogger.h"
#include "iDbgPdl/interface/iDbgPdlInterface.h"
#include "pdl/interface/pdlSmiXsmiDebug.h"
#include "pdl/interface/private/prvPdlGpio.h"
#include "pdl/interface/private/prvPdlI2c.h"
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_interface_set_debug
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_interface_set_debug(lua_State *L)
{
        int value;
        PDL_STATUS pdlStatus;
        if (lua_gettop(L) == 1) {
                value = lua_tointeger(L, 1);
                pdlStatus = pdlInterfaceDebugSet(value);
                PDL_CHECK_STATUS(pdlStatus);
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        lua_pushnumber(L, 0);
        return 1;
}

/*$ END OF wr_utils_pdl_interface_set_debug */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_interface_get_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_interface_get_info(lua_State *L)
{
        int interfaceId;
        char *interfaceTypeStr;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 2) {
                interfaceTypeStr = (char*) lua_tostring(L, 1);
                interfaceId = lua_tointeger(L, 2);
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        IDBG_PDL_LOGGER_API_EXECUTE("show interface info", IDBG_PDL_LOGGER_CATEGORY_INTERFACE_E, result, iDbgPdlInterfaceGetInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, interfaceTypeStr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);
        lua_pushnumber(L, result.result);
        return 1;
}

/*$ END OF wr_utils_pdl_interface_get_info */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_interface_get_hw
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_interface_get_hw(lua_State *L)
{
        int interfaceId, phyDeviceOrPage, redAddress;
        char *interfaceTypeStr;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 2) {
                interfaceTypeStr = (char*) lua_tostring(L, 1);
                interfaceId = lua_tointeger(L, 2);
                IDBG_PDL_LOGGER_API_EXECUTE("show interface hw ", IDBG_PDL_LOGGER_CATEGORY_INTERFACE_E, result, iDbgPdlInterfaceGetHw,
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, interfaceTypeStr),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId));
        }
        else
                if (lua_gettop(L) == 3) {
                        interfaceId = lua_tointeger(L, 1);
                        phyDeviceOrPage = lua_tointeger(L, 2);
                        redAddress = lua_tointeger(L, 3);
                        IDBG_PDL_LOGGER_API_EXECUTE("show interface hw smi/xsmi", IDBG_PDL_LOGGER_CATEGORY_INTERFACE_E, result, iDbgPdlInterfaceSmiXsmiGetHw,
                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId),
                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, phyDeviceOrPage),
                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, redAddress));
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);
        lua_pushnumber(L, result.result);
        return 1;
}

/*$ END OF wr_utils_pdl_interface_get_hw */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_interface_set_hw
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_interface_set_hw(lua_State *L)
{
        int interfaceId, value, phyDeviceOrPage, redAddress, mask;
        char *interfaceTypeStr;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 3) {
                interfaceTypeStr = (char*) lua_tostring(L, 1);
                interfaceId = lua_tointeger(L, 2);
                value = lua_tointeger(L, 3);
                IDBG_PDL_LOGGER_API_EXECUTE("set interface hw", IDBG_PDL_LOGGER_CATEGORY_INTERFACE_E, result, iDbgPdlInterfaceSetHw,
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, interfaceTypeStr),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value));
        }
        else
                if (lua_gettop(L) == 5) {
                        interfaceId = lua_tointeger(L, 1);
                        phyDeviceOrPage = lua_tointeger(L, 2);
                        redAddress = lua_tointeger(L, 3);
                        mask = lua_tointeger(L, 4);
                        value = lua_tointeger(L, 5);
                        IDBG_PDL_LOGGER_API_EXECUTE("set interface hw", IDBG_PDL_LOGGER_CATEGORY_INTERFACE_E, result, iDbgPdlInterfaceSmiXsmiSetHw,
                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId),
                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, phyDeviceOrPage),
                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, redAddress),
                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, mask),
                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value));
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);
        lua_pushnumber(L, result.result);
        return 1;
}

/*$ END OF wr_utils_pdl_interface_set_hw */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_interface_gpio_set_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_interface_gpio_set_info(lua_State *L)
{
        int interfaceId, paramMask, devId, pinNumber;
        char *typeStr;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 5) {
                interfaceId = lua_tointeger(L, 1);
                paramMask = lua_tointeger(L, 2);
                devId = lua_tointeger(L, 3);
                pinNumber = lua_tointeger(L, 4);
                typeStr = (char*) lua_tostring(L, 5);
        }
        else {
                lua_pushnil(L);
                return 1;
        }
        IDBG_PDL_LOGGER_API_EXECUTE("set interface info gpio", IDBG_PDL_LOGGER_CATEGORY_LED_E, result, iDbgPdlInterfaceGpioSetInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, paramMask),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, devId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, pinNumber),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, typeStr));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}

/*$ END OF wr_utils_pdl_interface_gpio_set_info */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_interface_i2c_set_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_interface_i2c_set_info(lua_State *L)
{
        int interfaceId, paramMask, busId, i2cAddress, i2cOffset, i2cMask;
        char *accessSizeStr;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 7) {
                interfaceId = lua_tointeger(L, 1);
                paramMask = lua_tointeger(L, 2);
                busId = lua_tointeger(L, 3);
                i2cAddress = (UINT_8) lua_tointeger(L, 4);
                accessSizeStr = (char*) lua_tostring(L, 5);
                i2cOffset = (UINT_16) lua_tointeger(L, 6);
                i2cMask = lua_tointeger(L, 7);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        IDBG_PDL_LOGGER_API_EXECUTE("set interface info i2c", IDBG_PDL_LOGGER_CATEGORY_LED_E, result, iDbgPdlInterfaceI2cSetInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, paramMask),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, busId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i2cAddress),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, accessSizeStr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i2cOffset),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i2cMask));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}

/*$ END OF wr_utils_pdl_interface_i2c_set_info */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_interface_mux_gpio_set_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_interface_mux_gpio_set_info(lua_State *L)
{
        int interfaceId, muxId, paramMask, devId, pinNumber, value;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 6) {
                interfaceId = lua_tointeger(L, 1);
                muxId = lua_tointeger(L, 2);
                paramMask = lua_tointeger(L, 3);
                devId = lua_tointeger(L, 4);
                pinNumber = (UINT_8) lua_tointeger(L, 5);
                value = lua_tointeger(L, 6);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        IDBG_PDL_LOGGER_API_EXECUTE("set interface info mux gpio", IDBG_PDL_LOGGER_CATEGORY_LED_E, result, iDbgPdlInterfaceMuxGpioSetInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, muxId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, paramMask),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, devId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, pinNumber),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}

/*$ END OF wr_utils_pdl_interface_mux_gpio_set_info */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_interface_mux_i2c_set_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_interface_mux_i2c_set_info(lua_State *L)
{
        int interfaceId, muxId, paramMask, i2cAddress, i2cOffset, i2cMask, value;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 7) {
                interfaceId = lua_tointeger(L, 1);
                muxId = lua_tointeger(L, 2);
                paramMask = lua_tointeger(L, 3);
                i2cAddress = lua_tointeger(L, 4);
                i2cOffset = (UINT_8) lua_tointeger(L, 5);
                i2cMask = (UINT_8) lua_tointeger(L, 6);
                value = lua_tointeger(L, 7);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        IDBG_PDL_LOGGER_API_EXECUTE("set interface info mux i2c", IDBG_PDL_LOGGER_CATEGORY_LED_E, result, iDbgPdlInterfaceMuxI2CSetInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, muxId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, paramMask),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i2cAddress),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i2cOffset),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i2cMask),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}

/*$ END OF wr_utils_pdl_interface_mux_i2c_set_info */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_interface_smi_xsmi_set_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_interface_smi_xsmi_set_info(lua_State *L)
{
        int interfaceId, paramMask, smiDevice, phyInterfaceId, smiAddress;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 5) {
                interfaceId = lua_tointeger(L, 1);
                paramMask = lua_tointeger(L, 2);
                smiDevice = lua_tointeger(L, 3);
                phyInterfaceId = (UINT_8) lua_tointeger(L, 4);
                smiAddress = (UINT_8) lua_tointeger(L, 5);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        IDBG_PDL_LOGGER_API_EXECUTE("set interface info mux i2c", IDBG_PDL_LOGGER_CATEGORY_LED_E, result, iDbgPdlInterfaceSmiXsmiSetInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, paramMask),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, smiDevice),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, phyInterfaceId),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, smiAddress));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}

/*$ END OF wr_utils_pdl_interface_smi_xsmi_set_info */

#define wr_util_set_result(result, rc)  \
    if (!result.result)                 \
        result.result = rc

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_interface_validation
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_interface_run_validation(lua_State *L)
{
        const char *interfaceTypeStr, *interfaceIdStr;
        int interfaceId, verbose;
        UINT_32 value, count, index, subInterator, subInteratorEnd=0, offset;
        UINT_16 value16;
        PDL_STATUS pdlStatus;
        PDL_INTERFACE_TYPE_ENT interfaceType;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        PDL_INTERFACE_GPIO_STC gpioAttributes;
        PDL_INTERFACE_I2C_STC i2cAttributes;
        PDL_INTERFACE_SMI_XSMI_STC smiXsmiAttributes;
        PDL_INTERFACE_GPIO_OFFSET_ENT gpioOffsetArr[] = { PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E, PDL_INTERFACE_GPIO_OFFSET_TYPE_E,
                        PDL_INTERFACE_GPIO_OFFSET_BLINKING_E, PDL_INTERFACE_GPIO_OFFSET_INPUT_E };
        if (lua_gettop(L) == 3) {
                interfaceTypeStr = lua_tostring(L, 1);
                interfaceIdStr = lua_tostring(L, 2);
                verbose = lua_tointeger(L, 3);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        cpssOsMemSet(&smiXsmiAttributes,0,sizeof(smiXsmiAttributes));

        if (strcmp(interfaceIdStr, "all") == 0)
        interfaceId = IDBG_PDL_ID_ALL;
        else
        interfaceId = atoi(interfaceIdStr);

        memset(&result, 0, sizeof(result));
        result.result = iDbgPdlLoggerCommandLogStart(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_SERDES_E, "wr_utils_pdl_interface_run_validation");
        if (result.result != PDL_OK)
        {
                printf("Unable to run the test!\n");
                lua_pushnumber(L, result.result);
                return 1;
        }

        result.result = iDbgPdlLoggerApiRunStart("wr_utils_pdl_interface_run_validation", IDBG_PDL_LOGGER_TYPEOF_LAST_CNS);
        if (result.result != PDL_OK)
        {
                printf("Unable to run the test!\n");
                iDbgPdlLoggerCommandEnd(&result);
                lua_pushnumber(L, result.result);
                return 1;
        }

        pdlStatus = iDbgPdlConvertStringToEnum(IDBG_PDL_UTILS_INTERFACE_TYPE, (char*) interfaceTypeStr, &value);
        if (pdlStatus != PDL_OK)
        {
                if (verbose)
                {
                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[Unknown interface type %s]\n", interfaceTypeStr);
                }
                wr_util_set_result(result, PDL_BAD_PARAM);
                goto end;
        }

        interfaceType = (PDL_INTERFACE_TYPE_ENT) value;

        switch (interfaceType)
        {
                case PDL_INTERFACE_TYPE_GPIO_E:
                case PDL_INTERFACE_TYPE_I2C_E:
                case PDL_INTERFACE_TYPE_SMI_E:
                case PDL_INTERFACE_TYPE_XSMI_E:
                        break;
                case PDL_INTERFACE_TYPE_SMI_CPU_E:
                        default:
                        if (verbose)
                        {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("[type %s(%d)]: [Unsupported interface type]\n", interfaceTypeStr, interfaceType);
                        }
                        wr_util_set_result(result, PDL_BAD_PARAM);
                        goto end;
        }

        if (interfaceId == IDBG_PDL_ID_ALL)
        {
                pdlStatus = pdlInterfaceCountGet(interfaceType, &count);
                if (pdlStatus != PDL_OK)
                {
                        if (verbose)
                        {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("[type %s(%d)]: [Can't get interface count]\n", interfaceTypeStr, interfaceType);
                        }
                        wr_util_set_result(result, PDL_BAD_PARAM);
                        goto end;
                }
                index = 0;
        }
        else
        {
                index = count = interfaceId;
                count++;
        }

        for (; index < count; index++)
                        {
                switch (interfaceType)
                {
                        case PDL_INTERFACE_TYPE_GPIO_E:
                                IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, PdlGpioDebugDbGetAttributes, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, index),
                                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &gpioAttributes))
                                ;
                                subInteratorEnd = sizeof(gpioOffsetArr) / sizeof(gpioOffsetArr[0]) + 1;
                        break;
                        case PDL_INTERFACE_TYPE_I2C_E:
                                IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, PdlI2CDebugDbGetAttributes, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, index),
                                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &i2cAttributes))
                                ;
                                subInteratorEnd = 1;
                        break;
                        case PDL_INTERFACE_TYPE_SMI_E:
                                case PDL_INTERFACE_TYPE_XSMI_E:
                                IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlSmiXsmiDebugDbGetAttributes, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, index),
                                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &smiXsmiAttributes))
                                ;
                                subInteratorEnd = 1;
                        break;
                        default: break;
                }
                if (pdlStatus != PDL_OK)
                {
                        if (verbose)
                        {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("[id %02d] [type %s]: [Can't get interface attributes]\n", index, interfaceTypeStr);
                        }
                        wr_util_set_result(result, PDL_BAD_PARAM);
                        goto end;
                }

                if ((interfaceType == PDL_INTERFACE_TYPE_SMI_E || interfaceType == PDL_INTERFACE_TYPE_XSMI_E) && (smiXsmiAttributes.interfaceType != interfaceType))
                continue;

                for (subInterator = 0; subInterator < subInteratorEnd; subInterator++)
                                {
                        /* currently only gpio require different values - all others use 0 */
                        offset = gpioOffsetArr[subInterator];

                        switch (interfaceType)
                        {
                                case PDL_INTERFACE_TYPE_GPIO_E:
                                        IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, prvPdlGpioHwGetValue, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, index),
                                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, offset),
                                                        IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_32, &value))
                                        ;
                                break;
                                case PDL_INTERFACE_TYPE_I2C_E:
                                        IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, prvPdlI2CHwGetValue, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, index),
                                                        IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_32, &value))
                                        ;
                                break;
                                case PDL_INTERFACE_TYPE_SMI_E:
                                        case PDL_INTERFACE_TYPE_XSMI_E:
                                        if (smiXsmiAttributes.interfaceType == interfaceType)
                                                        {
                                                IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlSmiXsmiHwGetValue, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, index),
                                                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_8, smiXsmiAttributes.dev),
                                                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_16, 0),
                                                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_16, &value16));
                                                value = (UINT_32) value16;
                                        }
                                break;
                            default: break;
                        }

                        if (pdlStatus != PDL_OK && !result.result)
                        result.result = 1;

                        if (verbose)
                        {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("[id %02d] [type %s]", index, interfaceTypeStr);
                                if (interfaceType == PDL_INTERFACE_TYPE_SMI_E || interfaceType == PDL_INTERFACE_TYPE_XSMI_E)
                                                {
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" [dev %d]:", smiXsmiAttributes.dev);
                                }
                                else
                                        if (interfaceType == PDL_INTERFACE_TYPE_GPIO_E)
                                                        {
                                                IDBG_PDL_LOGGER_API_OUTPUT_LOG(" [offset %02d]:", offset);
                                        }
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG(" [date read: %lu] - [%s]\n", value, pdlStatus == PDL_OK ? "SUCCESS" : "FAIL");
                        }
                        else
                        {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("[id %02d]: [%s]\n", index, pdlStatus == PDL_OK ? "SUCCESS" : "FAIL");
                        }
                }
        }
        end:
        iDbgPdlLoggerApiResultLog(&result);
        iDbgPdlLoggerCommandEnd(&result);

        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}
