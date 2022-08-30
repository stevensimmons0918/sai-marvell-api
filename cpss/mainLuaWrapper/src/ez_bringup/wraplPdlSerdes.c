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
#include "pdl/serdes/pdlSerdes.h"
#include "iDbgPdl/serdes/iDbgPdlSerdes.h"
#include "iDbgPdl/utils/iDbgPdlUtils.h"
#include "iDbgPdl/logger/iDbgPdlLogger.h"
#include "pdl/lib/pdlLib.h"
#include <pdl/packet_processor/private/prvPdlPacketProcessor.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/private/prvCpssPortManager.h>
#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#endif /*CHX_FAMILY*/
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
/*#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>*/


#ifndef CHX_FAMILY
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/common/port/cpssPortManager.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#define cpssDxChPortSerdesPolarityGet(a,b,c,d,e)            GT_NOT_IMPLEMENTED; *(d) = 0;*(e) = 0
#define cpssDxChPortManagerPortParamsGet(a,b,c)             GT_NOT_IMPLEMENTED;cpssOsMemSet(c,0,sizeof(CPSS_PM_PORT_PARAMS_STC))
#define cpssDxChPortSerdesManualTxConfigGet(a,b,c,d)        GT_NOT_IMPLEMENTED;cpssOsMemSet(d,0,sizeof(CPSS_PORT_SERDES_TX_CONFIG_STC))
#define cpssDxChPortSerdesManualRxConfigGet(a,b,c,d)        GT_NOT_IMPLEMENTED;cpssOsMemSet(d,0,sizeof(CPSS_PORT_SERDES_RX_CONFIG_STC))
#define CPSS_DXCH_PORT_SERDES_SPEED_ENT                     GT_U32
#define cpssDxChPortSerdesTuningGet(a,b,c,d,e)              GT_NOT_IMPLEMENTED;cpssOsMemSet(e,0,sizeof(CPSS_PORT_SERDES_TUNE_STC))
#define cpssDxChPortSerdesLaneTuningSet(a,b,c,d,e)          GT_NOT_IMPLEMENTED
#define cpssDxChPortSerdesPolaritySet(a,b,c,d,e)            GT_NOT_IMPLEMENTED
#define cpssDxChPortModeSpeedSet(a,b,c,d,e)                 GT_NOT_IMPLEMENTED
extern GT_STATUS hwsPortModeParamsGetToBuffer
(
   IN GT_U8                    devNum,
   IN GT_U32                   portGroup,
   IN GT_U32                   portNum,
   IN MV_HWS_PORT_STANDARD     portMode,
   OUT MV_HWS_PORT_INIT_PARAMS  *portParamsBuffer
);
#endif /*!CHX_FAMILY*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
/*************** UTILS ************************/

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_serdes_get_info_all
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
static int wr_utils_pdl_serdes_get_info_all(lua_State *L)
{
        int dev;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 2) {
                dev = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_SERDES_E, result, iDbgPdlSerdesGetInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, IDBG_PDL_ID_ALL));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_serdes_get_info_all */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_serdes_get_info
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_serdes_get_info(lua_State *L)
{
        int dev, port;
        const char *portStr;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 2) {
                dev = lua_tointeger(L, 1);
                portStr = lua_tostring(L, 2);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        if (strcmp(portStr, "all") == 0)
        return wr_utils_pdl_serdes_get_info_all(L);

        port = atoi(portStr);

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_SERDES_E, result, iDbgPdlSerdesGetInfo,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_serdes_get_info */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_serdes_get_polarity_all
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_serdes_get_polarity_all(lua_State *L)
{
        int dev;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 2) {
                dev = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_SERDES_E, result, iDbgPdlSerdesGetPolarity,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, IDBG_PDL_ID_ALL));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_serdes_get_polarity_all */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_serdes_get_polarity
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_serdes_get_polarity(lua_State *L)
{
        int dev, lane;
        const char *laneStr;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 2) {
                dev = lua_tointeger(L, 1);
                laneStr = lua_tostring(L, 2);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        if (strcmp(laneStr, "all") == 0)
        return wr_utils_pdl_serdes_get_polarity_all(L);

        lane = atoi(laneStr);

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_SERDES_E, result, iDbgPdlSerdesGetPolarity,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, lane));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_serdes_get_polarity */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_serdes_get_fine_tune_all
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
static int wr_utils_pdl_serdes_get_fine_tune_all(lua_State *L)
{
        int dev;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 2) {
                dev = lua_tointeger(L, 1);
        }
        else {
                lua_pushnil(L);
                return 0;
        }
        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_SERDES_E, result, iDbgPdlSerdesGetFineTune,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, IDBG_PDL_ID_ALL));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_serdes_get_fine_tune_all */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_serdes_get_fine_tune
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_serdes_get_fine_tune(lua_State *L)
{
        int dev, lane;
        const char *laneStr;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 2) {
                dev = lua_tointeger(L, 1);
                laneStr = lua_tostring(L, 2);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        if (strcmp(laneStr, "all") == 0)
        return wr_utils_pdl_serdes_get_fine_tune_all(L);

        lane = atoi(laneStr);

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_SERDES_E, result, iDbgPdlSerdesGetFineTune,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, lane));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;

}
/*$ END OF wr_utils_pdl_serdes_get_fine_tune */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_serdes_set_debug
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_serdes_set_debug(lua_State *L)
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

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_SERDES_E, result, iDbgPdlSerdesSetDebug,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(BOOLEAN, enable));

        lua_pushnumber(L, result.result);
        return 1;
}
/*$ END OF wr_utils_pdl_serdes_set_debug */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_serdes_lane_to_port
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
static BOOLEAN wr_utils_pdl_serdes_lane_to_port(

IN UINT_32 dev,
                IN UINT_32 absLane,

                OUT UINT_32 *portPtr,
                OUT UINT_32 *relLanePtr
                )
{
        UINT_32 port, b2bLink;
        PDL_PORT_LANE_DATA_STC serdesInfo;
        PDL_PP_XML_ATTRIBUTES_STC ppAttributes;
        PDL_PP_XML_B2B_ATTRIBUTES_STC b2bAttributes;
        PDL_STATUS pdlStatus;
        for (pdlStatus = pdlPpDbDevAttributesGetFirstPort(dev, &port);
                        pdlStatus == PDL_OK;
                        pdlStatus = pdlPpDbDevAttributesGetNextPort(dev, port, &port))
                                        {
                for (pdlStatus = pdlSerdesPortDbSerdesInfoGetFirst(dev, port, &serdesInfo);
                                pdlStatus == PDL_OK;
                                pdlStatus = pdlSerdesPortDbSerdesInfoGetNext(dev, port, &serdesInfo, &serdesInfo))
                                                {
                        if (serdesInfo.absSerdesNum == absLane)
                                        {
                                *portPtr = port;
                                *relLanePtr = serdesInfo.relSerdesNum;
                                return TRUE;
                        }
                }
        }

        pdlStatus = pdlPpDbAttributesGet(&ppAttributes);
        if (pdlStatus == PDL_OK)
        {
                for (b2bLink = 0; b2bLink < ppAttributes.numOfBackToBackLinksPerPp; b2bLink++)
                                {
                        pdlStatus = pdlPpDbB2bAttributesGet(b2bLink, &b2bAttributes);
                        if (pdlStatus == PDL_OK)
                        {
                                if (b2bAttributes.firstDev == dev)
                                                {
                                        port = b2bAttributes.firstPort;

                                        for (pdlStatus = pdlSerdesPortDbSerdesInfoGetFirst(dev, port, &serdesInfo);
                                                        pdlStatus == PDL_OK;
                                                        pdlStatus = pdlSerdesPortDbSerdesInfoGetNext(dev, port, &serdesInfo, &serdesInfo))
                                                                        {
                                                if (serdesInfo.absSerdesNum == absLane)
                                                                {
                                                        *portPtr = port;
                                                        *relLanePtr = serdesInfo.relSerdesNum;
                                                        return TRUE;
                                                }
                                        }
                                }
                                if (b2bAttributes.secondDev == dev)
                                                {
                                        port = b2bAttributes.secondPort;

                                        for (pdlStatus = pdlSerdesPortDbSerdesInfoGetFirst(dev, port, &serdesInfo);
                                                        pdlStatus == PDL_OK;
                                                        pdlStatus = pdlSerdesPortDbSerdesInfoGetNext(dev, port, &serdesInfo, &serdesInfo))
                                                                        {
                                                if (serdesInfo.absSerdesNum == absLane)
                                                                {
                                                        *portPtr = port;
                                                        *relLanePtr = serdesInfo.relSerdesNum;
                                                        return TRUE;
                                                }
                                        }
                                }
                        }
                }
        }

        return FALSE;
}
/*$ END OF wr_utils_pdl_serdes_lane_to_port */

#define wr_util_serdes_check_comphy_tx_param(_cpss_param, _pdl_param) \
    if (fineTuneTxAttributes._pdl_param != serdesTxCfg.txTune.comphy._cpss_param) \
    { \
        if (is_ok) \
        { \
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("FAILED\n"); \
        } \
        is_ok = FALSE; \
        if (verbose) \
        { \
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d: Incorrect tx parameter [%s] - [configured %d] [expected %d]\n",  \
                dev, lane, # _pdl_param, serdesTxCfg.txTune.comphy._cpss_param, fineTuneTxAttributes._pdl_param); \
        } \
    }

#define wr_util_serdes_check_comphy_rx_param(_cpss_param, _pdl_param) \
    if (fineTuneRxAttributes._pdl_param != serdesRxCfg.rxTune.comphy._cpss_param) \
    { \
        if (is_ok) \
        { \
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("FAILED\n"); \
        } \
        is_ok = FALSE; \
        if (verbose) \
        { \
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d: Incorrect rx parameter [%s] - [configured %d] [expected %d]\n",  \
                dev, lane, # _pdl_param, serdesRxCfg.rxTune.comphy._cpss_param, fineTuneRxAttributes._pdl_param); \
        } \
    }

#define wr_util_serdes_check_avago_tx_param(_cpss_param, _pdl_param) \
    if (fineTuneTxAttributes._pdl_param != serdesTxCfg.txTune.avago._cpss_param) \
    { \
        if (is_ok) \
        { \
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("FAILED\n"); \
        } \
        is_ok = FALSE; \
        if (verbose) \
        { \
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d: Incorrect tx parameter [%s] - [configured %d] [expected %d]\n",  \
            dev, lane, # _pdl_param, serdesTxCfg.txTune.avago._cpss_param, fineTuneTxAttributes._pdl_param); \
        } \
    }

#define wr_util_serdes_check_avago_rx_param(_cpss_param, _pdl_param) \
    if (fineTuneRxAttributes._pdl_param != serdesRxCfg.rxTune.avago._cpss_param) \
    { \
        if (is_ok) \
        { \
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("FAILED\n"); \
        } \
        is_ok = FALSE; \
        if (verbose) \
        { \
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d: Incorrect rx parameter [%s] - [configured %d] [expected %d]\n",  \
            dev, lane, # _pdl_param, serdesRxCfg.rxTune.avago._cpss_param, fineTuneRxAttributes._pdl_param); \
        } \
    }

#define wr_util_serdes_out_tx_param(_pdl_param) \
    "[%s = %d] ", # _pdl_param, fineTuneTxAttributes._pdl_param

#define wr_util_serdes_out_rx_param(_pdl_param) \
    "[%s = %d] ", # _pdl_param, fineTuneRxAttributes._pdl_param

#define wr_util_set_result(result, rc)  \
    if (!result.result)                 \
        result.result = rc
extern BOOLEAN pdl2cpssIfModeConvert(
                IN PDL_INTERFACE_MODE_ENT pdlIfMode,
                OUT CPSS_PORT_INTERFACE_MODE_ENT *cpssIfModePtr,
                OUT const char **ifModeStr
                );
extern BOOLEAN pdl2cpssSpeedConvert(
                IN PDL_PORT_SPEED_ENT pdlSpeed,
                OUT CPSS_PORT_SPEED_ENT *cpssSpeedPtr,
                OUT const char **speedStr
                );

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_serdes_run_validation_handler
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_serdes_run_validation_handler(
                lua_State *L,
                GT_U32 inDev,
                GT_U32 inLane,
                GT_U32 verbose
                )
{
        UINT_32 lane, dev, port, firstDev, lastDev, relLane, lastResult;
        const char *ifModeStr, *connectorTypeStr;
        PDL_STATUS pdlStatus, pdlStatus2;
        PDL_PP_XML_ATTRIBUTES_STC ppAttributes;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        PDL_LANE_POLARITY_ATTRIBUTES_STC polarityAttributes;
        GT_BOOL invertTx, invertRx;
        GT_STATUS cpssStatus;
        PDL_INTERFACE_MODE_ENT interfaceMode;
        PDL_CONNECTOR_TYPE_ENT connectorType;
        BOOLEAN fineTuneTxAttributesExists, fineTuneRxAttributesExists,
                        is_ok, is_exist,
                        cpssFineTuneTxAttributesExists, cpssFineTuneRxAttributesExists;
        PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC fineTuneTxAttributes;
        PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC fineTuneRxAttributes;
        CPSS_PORT_SERDES_TX_CONFIG_STC serdesTxCfg;
        CPSS_PORT_SERDES_RX_CONFIG_STC serdesRxCfg;
        CPSS_PM_PORT_PARAMS_STC portParamsStc;
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
        result.result = iDbgPdlLoggerCommandLogStart(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_PP_E, "wr_utils_pdl_serdes_run_validation_handler");
        if (result.result != PDL_OK)
        {
                printf("Unable to run the test!\n");
                lua_pushnumber(L, result.result);
                return 1;
        }

        result.result = iDbgPdlLoggerApiRunStart("wr_utils_pdl_serdes_run_validation_handler", IDBG_PDL_LOGGER_TYPEOF_LAST_CNS);
        if (result.result != PDL_OK)
        {
                printf("Unable to run the test!\n");
                iDbgPdlLoggerCommandEnd(&result);
                lua_pushnumber(L, result.result);
                return 1;
        }

        for (dev = firstDev; dev < lastDev; dev++)
                        {
                if (inLane == IDBG_PDL_ID_ALL)
                {
                        lane = 0;
                }
                else
                {
                        lane = inLane;
                }

                while (1)
                {
                        IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlSerdesDbPolarityAttrGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, lane),
                                        IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &polarityAttributes));
                        if ((inLane == IDBG_PDL_ID_ALL && pdlStatus == PDL_NOT_FOUND) ||
                                        (inLane != IDBG_PDL_ID_ALL && inLane != lane))
                        break;
                        PDL_CHECK_STATUS(pdlStatus);

                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Validate lane %d/%02d polarity configuration - ", dev, lane);

                        if (wr_utils_pdl_serdes_lane_to_port(dev, lane, &port, &relLane) == FALSE)
                        {
                                wr_util_set_result(result, 1);
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("FAILED\n");
                                if (verbose)
                                {
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d: Isn't connected to any port\n", dev, lane);
                                }
                                lane++;
                                continue;
                        }

                        cpssStatus = cpssDxChPortSerdesPolarityGet(dev, port, relLane, &invertTx, &invertRx);
                        if (cpssStatus != GT_OK)
                        {
                                wr_util_set_result(result, 2);
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("FAILED\n");
                                if (verbose)
                                {
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d: Can't retrieve polarity configuration from CPSS (error %d)\n", dev, lane, cpssStatus);
                                }
                                lane++;
                                continue;
                        }

                        if ((polarityAttributes.txSwap != invertTx) || (polarityAttributes.rxSwap != invertRx))
                                        {
                                wr_util_set_result(result, 3);
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("FAILED\n");
                                if (verbose)
                                {
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d: Wrong configuration CURRENT txSwap %5s rxSwap %5s EXPECTED txSwap %5s rxSwap %5s\n",
                                                        dev, lane, invertTx ? "true" : "false", invertRx ? "true" : "false",
                                                        polarityAttributes.txSwap ? "true" : "false", polarityAttributes.rxSwap ? "true" : "false");
                                }
                                lane++;
                                continue;
                        }

                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("PASSED\n");
                        if (verbose)
                        {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d: txSwap %5s rxSwap %5s\n", dev, lane, invertTx ? "true" : "false", invertRx ? "true" : "false");
                        }
                        lane++;
                }

                if (inLane == IDBG_PDL_ID_ALL)
                {
                        lane = 0;
                }
                else
                {
                        lane = inLane;
                }

                while (1)
                {
                        /* check lane existence */
                        IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlSerdesDbPolarityAttrGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, lane),
                                        IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &polarityAttributes));
                        if ((inLane == IDBG_PDL_ID_ALL && pdlStatus == PDL_NOT_FOUND) ||
                                        (inLane != IDBG_PDL_ID_ALL && inLane != lane))
                        break;
                        PDL_CHECK_STATUS(pdlStatus);

                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Validate lane %d/%02d fine tuning configuration - ", dev, lane);

                        /* check lane connection */
                        if (wr_utils_pdl_serdes_lane_to_port(dev, lane, &port, &relLane) == FALSE)
                        {
                                wr_util_set_result(result, 100);
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("FAILED\n");
                                if (verbose)
                                {
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d: Isn't connected to any port\n", dev, lane);
                                }
                                lane++;
                                continue;
                        }

                        lastResult = 0;

                        cpssFineTuneRxAttributesExists = FALSE;
                        cpssFineTuneTxAttributesExists = FALSE;
                        cpssStatus = cpssDxChPortManagerPortParamsGet(dev, port, &portParamsStc);
                        if (cpssStatus == GT_OK)
                        {
                                if (portParamsStc.portParamsType.regPort.laneParams[relLane].validLaneParamsBitMask & CPSS_PM_LANE_PARAM_RX_E)
                                cpssFineTuneRxAttributesExists = TRUE;
                                if (portParamsStc.portParamsType.regPort.laneParams[relLane].validLaneParamsBitMask & CPSS_PM_LANE_PARAM_TX_E)
                                cpssFineTuneTxAttributesExists = TRUE;
                        }

                        is_exist = FALSE;
                        for (pdlStatus2 = pdlSerdesDbFineTuneAttrGetFirst(dev, lane, &interfaceMode, &connectorType);
                                        pdlStatus2 == PDL_OK;
                                        pdlStatus2 = pdlSerdesDbFineTuneAttrGetNext(dev, lane, interfaceMode, connectorType, &interfaceMode, &connectorType))
                                                        {
                                is_exist = TRUE;
                                IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlSerdesDbFineTuneAttrGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, lane),
                                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceMode),
                                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, connectorType),
                                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(BOOLEAN, &fineTuneTxAttributesExists),
                                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &fineTuneTxAttributes),
                                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(BOOLEAN, &fineTuneRxAttributesExists),
                                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &fineTuneRxAttributes));
                                if (pdlStatus != PDL_OK)
                                {
                                        if (!lastResult)
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("FAILED\n");
                                        lastResult = 10;
                                        if (verbose)
                                        {
                                                pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, interfaceMode, (char**) &ifModeStr);
                                                pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_CONNECTOR_TYPE_TYPE_E, connectorType, (char**) &connectorTypeStr);
                                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d mode %s connector %s: Can't retrieve configuration from PDL\n", dev, lane, ifModeStr,
                                                                connectorTypeStr);
                                        }
                                        continue;
                                }

                                cpssStatus = cpssDxChPortSerdesManualTxConfigGet(dev, port, relLane, &serdesTxCfg);
                                if ((cpssStatus != GT_OK && fineTuneTxAttributesExists) || (cpssFineTuneTxAttributesExists != fineTuneTxAttributesExists))
                                                {
                                        if (!lastResult)
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("FAILED\n");
                                        lastResult = 12;
                                        if (verbose)
                                        {
                                                if (cpssStatus != GT_OK && fineTuneTxAttributesExists)
                                                                {
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d: Can't retrieve TX fine tuning configuration from CPSS (error %d)\n", dev, lane,
                                                                        cpssStatus);
                                                }
                                                else
                                                {
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d: TX fine tuning configuration mismatch CPSS [%ss] PDL [%ss]\n",
                                                                        dev, lane, cpssFineTuneTxAttributesExists ? "exist" : "not exist", fineTuneTxAttributesExists ? "exist" : "not exist");
                                                }
                                        }
                                        fineTuneTxAttributesExists = FALSE;
                                }
                                cpssStatus = cpssDxChPortSerdesManualRxConfigGet(dev, port, relLane, &serdesRxCfg);
                                if ((cpssStatus != GT_OK && fineTuneRxAttributesExists) || (cpssFineTuneRxAttributesExists != fineTuneRxAttributesExists))
                                                {
                                        if (!lastResult)
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("FAILED\n");
                                        lastResult = 13;
                                        if (cpssStatus != GT_OK && fineTuneRxAttributesExists)
                                                        {
                                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d: Can't retrieve RX fine tuning configuration from CPSS (error %d)\n", dev, lane,
                                                                cpssStatus);
                                        }
                                        else
                                        {
                                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d: RX fine tuning configuration mismatch CPSS [%ss] PDL [%ss]\n",
                                                                dev, lane, cpssFineTuneRxAttributesExists ? "exist" : "not exist", fineTuneRxAttributesExists ? "exist" : "not exist");
                                        }
                                        fineTuneRxAttributesExists = FALSE;
                                }
                                is_ok = TRUE;
                                while (fineTuneTxAttributesExists)
                                {
                                        switch (serdesTxCfg.type)
                                        {
                                                case CPSS_PORT_SERDES_COMPHY_H_E:
                                                        wr_util_serdes_check_comphy_tx_param(txAmp, txAmpl)
                                                        ;
                                                        wr_util_serdes_check_comphy_tx_param(txAmpAdjEn, txAmplAdjEn)
                                                        ;
                                                        wr_util_serdes_check_comphy_tx_param(emph0, emph0)
                                                        ;
                                                        wr_util_serdes_check_comphy_tx_param(emph1, emph1)
                                                        ;
                                                        wr_util_serdes_check_comphy_tx_param(txAmpShft, txAmplShtEn)
                                                        ;
                                                        wr_util_serdes_check_comphy_tx_param(txEmphEn, txEmphEn)
                                                        ;
                                                break;
                                                case CPSS_PORT_SERDES_AVAGO_E:
                                                        wr_util_serdes_check_avago_tx_param(atten, txAmpl)
                                                        ;
                                                        wr_util_serdes_check_avago_tx_param(post, emph0)
                                                        ;
                                                        wr_util_serdes_check_avago_tx_param(pre, emph1)
                                                        ;
                                                break;
                                                default:
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("wr_utils_pdl_serdes_run_validation_handler - Unsupported serdes_type");
                                        }
                                }
                                while (fineTuneRxAttributesExists)
                                {
                                        switch (serdesRxCfg.type)
                                        {
                                                case CPSS_PORT_SERDES_COMPHY_H_E:
                                                        wr_util_serdes_check_comphy_rx_param(dcGain, dcGain)
                                                        ;
                                                        wr_util_serdes_check_comphy_rx_param(bandWidth, bandWidth)
                                                        ;
                                                        wr_util_serdes_check_comphy_rx_param(ffeR, ffeRes)
                                                        ;
                                                        wr_util_serdes_check_comphy_rx_param(ffeC, ffeCap)
                                                        ;
                                                        wr_util_serdes_check_comphy_rx_param(sqlch, sqlch)
                                                        ;
                                                        wr_util_serdes_check_comphy_rx_param(align90, align90)
                                                        ;
                                                break;
                                                case CPSS_PORT_SERDES_AVAGO_E:
                                                        wr_util_serdes_check_avago_rx_param(BW, bandWidth)
                                                        ;
                                                        wr_util_serdes_check_avago_rx_param(DC, dcGain)
                                                        ;
                                                        wr_util_serdes_check_avago_rx_param(sqlch, sqlch)
                                                        ;
                                                        wr_util_serdes_check_avago_rx_param(LF, ffeRes)
                                                        ;
                                                        wr_util_serdes_check_avago_rx_param(HF, ffeCap)
                                                        ;
                                                break;
                                                default:
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("wr_utils_pdl_serdes_run_validation_handler - Unsupported serdes_type");
                                        }
                                }

                                if (is_ok)
                                {
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("PASSED\n");
                                        if (verbose)
                                        {
                                                if (fineTuneTxAttributesExists)
                                                {
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d tx attributes: ", dev, lane);
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(wr_util_serdes_out_tx_param(txAmpl));
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(wr_util_serdes_out_tx_param(txAmplAdjEn));
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(wr_util_serdes_out_tx_param(txAmplShtEn));
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(wr_util_serdes_out_tx_param(emph0));
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(wr_util_serdes_out_tx_param(emph1));
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(wr_util_serdes_out_tx_param(txEmphEn));
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(wr_util_serdes_out_tx_param(slewRate));
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
                                                }
                                                if (fineTuneRxAttributesExists)
                                                {
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d rx attributes: ", dev, lane);
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(wr_util_serdes_out_rx_param(sqlch));
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(wr_util_serdes_out_rx_param(ffeRes));
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(wr_util_serdes_out_rx_param(ffeCap));
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(wr_util_serdes_out_rx_param(align90));
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(wr_util_serdes_out_rx_param(dcGain));
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(wr_util_serdes_out_rx_param(bandWidth));
                                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
                                                }
                                        }
                                }
                        }
                        if (is_exist == FALSE)
                        {
                                is_ok = TRUE;
                                cpssStatus = cpssDxChPortSerdesManualTxConfigGet(dev, port, relLane, &serdesTxCfg);
                                if (cpssStatus == GT_OK && cpssFineTuneTxAttributesExists)
                                                {
                                        if (!lastResult)
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("FAILED\n");
                                        lastResult = 12;
                                        is_ok = FALSE;
                                        if (verbose)
                                        {
                                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d: TX fine tuning configuration mismatch CPSS [exists] PDL [not exists]\n", dev, lane);
                                        }
                                }
                                cpssStatus = cpssDxChPortSerdesManualRxConfigGet(dev, port, relLane, &serdesRxCfg);
                                if (cpssStatus == GT_OK && cpssFineTuneRxAttributesExists)
                                                {
                                        if (!lastResult)
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("FAILED\n");
                                        lastResult = 13;
                                        is_ok = FALSE;
                                        if (verbose)
                                        {
                                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d: RX fine tuning configuration mismatch CPSS [exists] PDL [not exists]\n", dev, lane);
                                        }
                                }
                                if (is_ok)
                                {
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("PASSED\n");
                                        if (verbose)
                                        {
                                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d lane %d: NO fine tuning configuration required\n", dev, lane);
                                        }
                                }
                        }
                        lane++;
                        if (lastResult && !result.result)
                        result.result = lastResult;
                }
        }

        iDbgPdlLoggerApiResultLog(&result);
        iDbgPdlLoggerCommandEnd(&result);

        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}
/*$ END OF wr_utils_pdl_serdes_run_validation_handler */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_serdes_run_validation
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_serdes_run_validation(lua_State *L)
{
        int verbose, lane, dev;
        const char *laneStr;
        if (lua_gettop(L) == 3) {
                dev = lua_tonumber(L, 1);
                laneStr = lua_tostring(L, 2);
                verbose = lua_tonumber(L, 3);
        }
        else
                if (lua_gettop(L) == 2) {
                        dev = lua_tonumber(L, 1);
                        laneStr = lua_tostring(L, 2);
                        verbose = FALSE;
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }

        if (strcmp(laneStr, "all") == 0)
                        {
                lane = IDBG_PDL_ID_ALL;
        }
        else
        {
                lane = atoi(laneStr);
        }

        return wr_utils_pdl_serdes_run_validation_handler(L, dev, lane, verbose);
}
/*$ END OF wr_utils_pdl_serdes_run_validation */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_serdes_run_validation_dev_all
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_serdes_run_validation_dev_all(lua_State *L)
{
        int verbose, lane;
        const char *laneStr;
        if (lua_gettop(L) == 2) {
                laneStr = lua_tostring(L, 1);
                verbose = lua_tonumber(L, 2);
        }
        else
                if (lua_gettop(L) == 1) {
                        laneStr = lua_tostring(L, 1);
                        verbose = FALSE;
                }
                else {
                        lua_pushnil(L);
                        return 0;
                }

        if (strcmp(laneStr, "all") == 0)
                        {
                lane = IDBG_PDL_ID_ALL;
        }
        else
        {
                lane = atoi(laneStr);
        }

        return wr_utils_pdl_serdes_run_validation_handler(L, IDBG_PDL_ID_ALL, lane, verbose);
}
/*$ END OF wr_utils_pdl_serdes_run_validation_dev_all */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_serdes_set_tx_fine_tune
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
typedef struct {
        UINT_32 value;
        void *paramPtr;
} wr_utils_pdl_serdes_set_params_TYP;

int wr_utils_pdl_serdes_set_tx_fine_tune(lua_State *L)
{
        int i;
        wr_utils_pdl_serdes_set_params_TYP params[10];
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 10)
                        {
                for (i = 0; i < lua_gettop(L); i++)
                                {
                        if (lua_isnil(L, i + 1) == FALSE)
                        {
                                if (lua_isnumber(L, i + 1))
                                                {
                                        params[i].value = lua_tonumber(L, i + 1);
                                        params[i].paramPtr = &params[i].value;
                                }
                                else
                                        if (lua_isstring(L, i + 1))
                                                        {
                                                params[i].paramPtr = (void*) lua_tostring(L, i + 1);
                                        }
                                        else
                                        {
                                                params[i].value = lua_toboolean(L, i + 1);
                                                params[i].paramPtr = &params[i].value;
                                        }
                        }
                        else
                        params[i].paramPtr = NULL;
                }
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L),
                        IDBG_PDL_LOGGER_CATEGORY_SERDES_E,
                        result,
                        iDbgPdlSerdesSetTxFineTune,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[0].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[1].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[2].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[3].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[4].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[5].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[6].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[7].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[8].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[9].paramPtr));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}
/*$ END OF wr_utils_pdl_serdes_set_tx_fine_tune */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_serdes_set_rx_fine_tune
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_serdes_set_rx_fine_tune(lua_State *L)
{
        int i;
        wr_utils_pdl_serdes_set_params_TYP params[10];
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 10)
                        {
                for (i = 0; i < lua_gettop(L); i++)
                                {
                        if (lua_isnil(L, i + 1) == FALSE)
                        {
                                if (lua_isnumber(L, i + 1))
                                                {
                                        params[i].value = lua_tonumber(L, i + 1);
                                        params[i].paramPtr = &params[i].value;
                                }
                                else
                                        if (lua_isstring(L, i + 1))
                                                        {
                                                params[i].paramPtr = (void*) lua_tostring(L, i + 1);
                                        }
                                        else
                                        {
                                                params[i].value = lua_toboolean(L, i + 1);
                                                params[i].paramPtr = &params[i].value;
                                        }
                        }
                        else
                        params[i].paramPtr = NULL;
                }
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L),
                        IDBG_PDL_LOGGER_CATEGORY_SERDES_E,
                        result,
                        iDbgPdlSerdesSetRxFineTune,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[0].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[1].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[2].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[3].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[4].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[5].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[6].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[7].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[8].paramPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, params[9].paramPtr));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}
/*$ END OF wr_utils_pdl_serdes_set_tx_fine_tune */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_serdes_set_polarity
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_serdes_set_polarity(lua_State *L)
{
        int dev, lane;
        BOOLEAN tx_swap, *txSwapPtr = &tx_swap, rx_swap, *rxSwapPtr = &rx_swap;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        if (lua_gettop(L) == 4)
                        {
                dev = lua_tointeger(L, 1);
                lane = lua_tointeger(L, 2);
                if (lua_isnil(L, 3))
                txSwapPtr = NULL;
                else
                tx_swap = lua_toboolean(L, 3);
                if (lua_isnil(L, 4))
                rxSwapPtr = NULL;
                else
                rx_swap = lua_toboolean(L, 4);
        }
        else {
                lua_pushnil(L);
                return 0;
        }

        IDBG_PDL_LOGGER_API_EXECUTE(WR_PDL_CMDLINE_GET_MAC(L),
                        IDBG_PDL_LOGGER_CATEGORY_SERDES_E,
                        result,
                        iDbgPdlSerdesSetPolarity,
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, lane),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, txSwapPtr),
                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, rxSwapPtr));
        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}
/*$ END OF wr_utils_pdl_serdes_set_polarity */

/*****************************************************************************
 * FUNCTION NAME: wr_utils_pdl_serdes_apply_config
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
int wr_utils_pdl_serdes_apply_config(lua_State *L)
{
        /****************************************************************************/
        /*L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
        /****************************************************************************/
        UINT_32 dev, port;
        PDL_PORT_SPEED_ENT pdlSpeed;
        PDL_STATUS pdlStatus, pdlStatus2;
        PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC portAttributes;
        PDL_PP_XML_ATTRIBUTES_STC ppAttributes;
        UINT_32 index;
        PDL_PP_XML_B2B_ATTRIBUTES_STC b2bAttributes;
        CPSS_PORT_SPEED_ENT cpssSpeed;
        GT_STATUS cpssRc;
        MV_HWS_PORT_STANDARD hwsIfMode;
        MV_HWS_PORT_INIT_PARAMS portInitParams;
        CPSS_PORT_INTERFACE_MODE_ENT cpssIfMode;
        PDL_INTERFACE_MODE_ENT pdlIfMode=0;
        const char *speedStr, *ifModeStr;
        PDL_PORT_LANE_DATA_STC serdesInfo;
        BOOLEAN fineTuneTxAttributesExists, fineTuneRxAttributesExists, verbose;
        PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC fineTuneTxAttributes;
        PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC fineTuneRxAttributes;
        PDL_LANE_POLARITY_ATTRIBUTES_STC polarityAttributes;
        PDL_CONNECTOR_TYPE_ENT connectorType;
        CPSS_PORT_SERDES_TUNE_STC cpssSerdesTuneValues;
        CPSS_DXCH_PORT_SERDES_SPEED_ENT cpssSerdesSpeed;
        CPSS_PORTS_BMP_STC portsBmp;
        IDBG_PDL_LOGGER_RESULT_DATA_STC result;
        /****************************************************************************/
        /*                      F U N C T I O N   L O G I C                         */
        /****************************************************************************/
        if (lua_gettop(L) < 4)
        verbose = FALSE;
        else
        verbose = lua_tointeger(L, 4);

        if (lua_gettop(L) < 3)
                        {
                lua_pushnil(L);
                return 0;
        }

        dev = lua_tointeger(L, 1);
        port = lua_tointeger(L, 2);
        speedStr = lua_tostring(L, 3);

        memset(&result, 0, sizeof(result));
        result.result = iDbgPdlLoggerCommandLogStart(WR_PDL_CMDLINE_GET_MAC(L), IDBG_PDL_LOGGER_CATEGORY_SERDES_E, "wr_utils_pdl_serdes_apply_config");
        if (result.result != PDL_OK)
        {
                printf("Unable to run the test!\n");
                lua_pushnumber(L, result.result);
                return 1;
        }

        result.result = iDbgPdlLoggerApiRunStart("wr_utils_pdl_serdes_apply_config", IDBG_PDL_LOGGER_TYPEOF_LAST_CNS);
        if (result.result != PDL_OK)
        {
                printf("Unable to run the test!\n");
                iDbgPdlLoggerCommandEnd(&result);
                lua_pushnumber(L, result.result);
                return 1;
        }

        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_SPEED_TYPE_E, (char*) speedStr, &pdlSpeed);
        if (pdlStatus != PDL_OK)
        {
                if (verbose)
                {
                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d] [port %d]: [Unknown speed %s(%d)]\n", dev, port, speedStr, pdlSpeed);
                }
                wr_util_set_result(result, PDL_BAD_PARAM);
                goto end;
        }

        if (FALSE == pdl2cpssSpeedConvert(pdlSpeed, &cpssSpeed, &speedStr))
                        {
                if (verbose)
                {
                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d] [port %d]: [Can't convert to cpss speed %s(%d)]\n", dev, port, speedStr, pdlSpeed);
                }
                wr_util_set_result(result, PDL_BAD_PARAM);
                goto end;
        }

        /* find out interface mode matching speed */
        pdlStatus = pdlPpDbPortAttributesGet(dev, port, &portAttributes);
        if (pdlStatus == PDL_OK)
        {
                pdlStatus = PDL_NOT_FOUND;
                for (index = 0; index < portAttributes.numOfCopperModes; index++)
                                {
                        if (portAttributes.copperModesArr[index].speed == pdlSpeed)
                                        {
                                pdlStatus = PDL_OK;
                                pdlIfMode = portAttributes.copperModesArr[index].mode;
                                break;
                        }
                }
                if (pdlStatus == PDL_NOT_FOUND)
                {
                        for (index = 0; index < portAttributes.numOfFiberModes; index++)
                                        {
                                if (portAttributes.fiberModesArr[index].speed == pdlSpeed)
                                                {
                                        pdlStatus = PDL_OK;
                                        pdlIfMode = portAttributes.fiberModesArr[index].mode;
                                        break;
                                }
                        }
                }
        }
        else
                if (pdlStatus == PDL_NOT_FOUND)
                {
                        /* maybe it is b2b link */
                        pdlStatus = pdlPpDbAttributesGet(&ppAttributes);
                        if (pdlStatus == PDL_OK)
                        {
                                pdlStatus = PDL_NOT_FOUND;
                                for (index = 0; index < ppAttributes.numOfBackToBackLinksPerPp; index++)
                                                {
                                        if (PDL_OK == pdlPpDbB2bAttributesGet(index, &b2bAttributes))
                                                        {
                                                if (((b2bAttributes.firstDev == dev && port == b2bAttributes.firstPort) ||
                                                                (b2bAttributes.secondDev == dev && port == b2bAttributes.secondPort)) &&
                                                                pdlSpeed == b2bAttributes.maxSpeed)
                                                                                {
                                                        pdlStatus = PDL_OK;
                                                        pdlIfMode = b2bAttributes.interfaceMode;
                                                        break;
                                                }
                                        }
                                }
                        }
                }

        if (pdlStatus != PDL_OK)
        {
                if (verbose)
                {
                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d] [port %d]: [Can't find interface mode for supplied speed %s]\n", dev, port, speedStr);
                }
                wr_util_set_result(result, PDL_BAD_PARAM);
                goto end;
        }

        if (FALSE == pdl2cpssIfModeConvert(pdlIfMode, &cpssIfMode, &ifModeStr))
                        {

                if (verbose)
                {
                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("dev %d port %d: [Unknown interface mode %d]\n", dev, port, pdlIfMode);
                }
                wr_util_set_result(result, PDL_BAD_PARAM);
                goto end;
        }

        cpssRc = prvCpssCommonPortIfModeToHwsTranslate((GT_U8) dev, cpssIfMode, cpssSpeed, &hwsIfMode);
        if (cpssRc != GT_OK)
        {
                if (verbose)
                {
                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d] [port %d] [ifMode %s] [speed: %s]: [Can't convert to port mode]\n", dev, port, ifModeStr, speedStr);
                }
                wr_util_set_result(result, PDL_BAD_PARAM);
                goto end;
        }

        cpssRc = hwsPortModeParamsGetToBuffer((GT_U8) dev, 0, port, hwsIfMode, &portInitParams);
        if (cpssRc != GT_OK)
        {
                if (verbose)
                {
                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d] [port %d] [ifMode %s] [speed %s]: [Can't get port parameters]\n", dev, port, ifModeStr, speedStr);
                }
                wr_util_set_result(result, PDL_BAD_PARAM);
                goto end;
        }

        switch (portInitParams.serdesSpeed)
        {
#ifdef CHX_FAMILY
                case SPEED_NA:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_NA_E;
                break;
                case _1_25G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_1_25_E;
                break;
                case _3_125G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_3_125_E;
                break;
                case _3_33G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_3_333_E;
                break;
                case _3_75G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_3_75_E;
                break;
                case _4_25G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_4_25_E;
                break;
                case _5G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_5_E;
                break;
                case _6_25G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_6_25_E;
                break;
                case _7_5G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_7_5_E;
                break;
                case _10_3125G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_10_3125_E;
                break;
                case _11_25G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_11_25_E;
                break;
                case _11_5625G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_11_5625_E;
                break;
                case _12_5G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_12_5_E;
                break;
                case _10_9375G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_10_9375_E;
                break;
                case _12_1875G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_12_1875_E;
                break;
                case _5_625G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_5_625_E;
                break;
                case _5_15625G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_5_156_E;
                break;
                case _12_8906G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_12_8906_E;
                break;
                case _20_625G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_20_625_E;
                break;
                case _25_78125G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_25_78125_E;
                break;
                case _27_5G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_27_5_E;
                break;
                case _28_05G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_28_05_E;
                break;
                case _26_25G:
                        cpssSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_26_25_E;
                break;
#else
                case SPEED_NA:
                        cpssSerdesSpeed = 0;
                break;
#endif /*CHX_FAMILY*/
                default:
                        if (verbose)
                        {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d] [port %d]: [Can't convert serdes speed %d]\n", dev, port, portInitParams.serdesSpeed);
                        }
                        wr_util_set_result(result, PDL_BAD_PARAM);
                        goto end;
        }

        for (pdlStatus = pdlSerdesPortDbSerdesInfoGetFirst(dev, port, &serdesInfo);
                        pdlStatus == PDL_OK;
                        pdlStatus = pdlSerdesPortDbSerdesInfoGetNext(dev, port, &serdesInfo, &serdesInfo))
                                        {
                for (connectorType = PDL_CONNECTOR_TYPE_SFPPLUS_E; connectorType < PDL_CONNECTOR_TYPE_LAST_E; connectorType++)
                                {
                        pdlStatus2 = pdlSerdesDbFineTuneAttrGet(dev, serdesInfo.absSerdesNum, pdlIfMode, connectorType,
                                        &fineTuneTxAttributesExists, &fineTuneTxAttributes,
                                        &fineTuneRxAttributesExists, &fineTuneRxAttributes);
                        if (pdlStatus2 == PDL_OK)
                        {
                                memset(&cpssSerdesTuneValues, 0, sizeof(cpssSerdesTuneValues));
                                cpssRc = cpssDxChPortSerdesTuningGet((GT_U8) dev,
                                                port,
                                                serdesInfo.relSerdesNum,
                                                cpssSerdesSpeed,
                                                &cpssSerdesTuneValues);
                                if ((cpssRc != GT_OK) && verbose)
                                                {
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d] [port %d] [abs lane %d] [serdes speed %d]: [Can't set fine tune parameters]\n", dev, port,
                                                        serdesInfo.absSerdesNum, cpssSerdesSpeed);
                                }
                                if (fineTuneTxAttributesExists)
                                {
                                        cpssSerdesTuneValues.txTune.comphy.emph0 = fineTuneTxAttributes.emph0;
                                        cpssSerdesTuneValues.txTune.comphy.emph1 = fineTuneTxAttributes.emph1;
                                        cpssSerdesTuneValues.txTune.comphy.txAmp = fineTuneTxAttributes.txAmpl;
                                        cpssSerdesTuneValues.txTune.comphy.txAmpAdjEn = (GT_U32) fineTuneTxAttributes.txAmplAdjEn;
                                        cpssSerdesTuneValues.txTune.comphy.txAmpShft = (GT_BOOL) fineTuneTxAttributes.txAmplShtEn;
                                        cpssSerdesTuneValues.txTune.comphy.txEmphEn = (GT_BOOL) fineTuneTxAttributes.txEmphEn;
                                }
                                if (fineTuneRxAttributesExists)
                                {
                                        cpssSerdesTuneValues.rxTune.comphy.align90 = fineTuneRxAttributes.align90;
                                        cpssSerdesTuneValues.rxTune.comphy.bandWidth = fineTuneRxAttributes.bandWidth;
                                        cpssSerdesTuneValues.rxTune.comphy.dcGain = fineTuneRxAttributes.dcGain;
                                        cpssSerdesTuneValues.rxTune.comphy.ffeC = fineTuneRxAttributes.ffeCap;
                                        cpssSerdesTuneValues.rxTune.comphy.ffeR = fineTuneRxAttributes.ffeRes;
                                        cpssSerdesTuneValues.rxTune.comphy.sqlch = fineTuneRxAttributes.sqlch;
                                }
                                cpssRc = cpssDxChPortSerdesLaneTuningSet((GT_U8) dev,
                                                0,
                                                serdesInfo.absSerdesNum,
                                                cpssSerdesSpeed,
                                                &cpssSerdesTuneValues);
                                if ((cpssRc != GT_OK) && verbose)
                                                {
                                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d] [port %d] [abs lane %d] [serdes speed %d]: [Can't get set fine tune parameters]\n", dev, port,
                                                        serdesInfo.absSerdesNum, cpssSerdesSpeed);
                                }
                        }
                }
                pdlStatus2 = pdlSerdesDbPolarityAttrGet(dev, serdesInfo.absSerdesNum, &polarityAttributes);
                if (pdlStatus2 == PDL_OK)
                {
                        cpssRc = cpssDxChPortSerdesPolaritySet((GT_U8) dev,
                                        0,
                                        1 << serdesInfo.relSerdesNum,
                                        (GT_BOOL) polarityAttributes.txSwap,
                                        (GT_BOOL) polarityAttributes.rxSwap);
                        if ((cpssRc != GT_OK) && verbose)
                                        {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d] [port %d] [rel lane %d (abs %d)]: [Can't set polarity parameters]\n", dev, port,
                                                serdesInfo.relSerdesNum, serdesInfo.absSerdesNum);
                        }
                }
        }
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
        CPSS_PORTS_BMP_PORT_ENABLE_MAC(&portsBmp, port, GT_TRUE);
        cpssRc = cpssDxChPortModeSpeedSet((GT_U8) dev, &portsBmp, GT_TRUE, cpssIfMode, cpssSpeed);
        if (cpssRc != GT_OK)
        {
                if (verbose)
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d] [port %d] [abs lane %d]: [Can't finalize configuration]\n", dev, port, serdesInfo.absSerdesNum);
                wr_util_set_result(result, PDL_FAIL);
                goto end;
        }

        end:
        iDbgPdlLoggerApiResultLog(&result);
        iDbgPdlLoggerCommandEnd(&result);

        if (result.outDescriptionPtr)
        printf("%s", result.outDescriptionPtr);

        lua_pushnumber(L, result.result);
        return 1;
}
