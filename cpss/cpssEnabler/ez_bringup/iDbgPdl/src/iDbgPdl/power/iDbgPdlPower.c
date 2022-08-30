/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file iDbgPdlPower.c
 * @copyright
 *    (c), Copyright 2001, Marvell International Ltd.
 *    THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 *    NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 *    OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 *    DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 *    THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,
 *    IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
********************************************************************************
 * 
 * @brief Debug lib power module
 * 
 * @version   1 
********************************************************************************
*/
/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdl/packet_processor/private/prvPdlPacketProcessor.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/power_supply/pdlPowerDebug.h>
#include <pdl/power_supply/pdlPower.h>
#include <iDbgPdl/power/iDbgPdlPower.h>
#include <iDbgPdl/interface/iDbgPdlInterface.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <iDbgPdl/utils/iDbgPdlUtils.h>


/*****************************************************************************
* FUNCTION NAME: iDbgPdlPowerGetInfo
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlPowerGetInfo (   
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN char                              *powerId
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_POWER_XML_CAPABILITIES_STC          capabilities;
    PDL_POWER_PS_INTERFACE_STC              interfacePs;
    PDL_POWER_PS_TYPE_ENT                   typePs = PDL_POWER_PS_TYPE_PS_E;
    char                                  * powerInterfaceTypeStr = NULL, * powerTypeStr = NULL;                                   
    BOOLEAN                                 flagAll = FALSE;
    PDL_STATUS                              pdlStatus;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    pdlStatus = pdlPowerDbCapabilitiesGet(&capabilities);
    PDL_CHECK_STATUS(pdlStatus);
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-13s | %-14s | %-13s |\n", "power-id", " active-info ", "interface-type", " interface-id");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-13s | %-14s | %-13s |\n", "--------", "-------------", "--------------", "-------------");
    if (strcmp(powerId, "all") == 0){
        flagAll = TRUE;
    }
    if (flagAll == FALSE){ 
        if  ((strcmp(powerId, "rps") == 0) && (capabilities.RpsIsSupported == FALSE)){
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-13s | %-14s | %-13s |\n", powerId, "Not supported", "Not supported", "Not supported");
        }
        else {
            if (strcmp(powerId, "rps") == 0)
                typePs = PDL_POWER_PS_TYPE_RPS_E;
            pdlStatus = pdlPowerDebugStatusAccessInfoGet(typePs, &interfacePs);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = iDbgPdlConvertEnumToString (IDBG_PDL_UTILS_INTERFACE_TYPE, interfacePs.interfaceType, &powerInterfaceTypeStr);
            PDL_CHECK_STATUS(pdlStatus);
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-13d | %-14s | %-13d |\n", powerId, interfacePs.values.activeValue, powerInterfaceTypeStr, interfacePs.interfaceId); 
            IDBG_PDL_LOGGER_API_OUTPUT_LOG ("\n");
            pdlStatus = iDbgPdlInterfaceGetInfo (resultPtr, powerInterfaceTypeStr, interfacePs.interfaceId);
            PDL_CHECK_STATUS(pdlStatus);
        }
    }
    else {
        for (; typePs<PDL_POWER_PS_TYPE_PD_E; typePs++){
            if ((typePs == PDL_POWER_PS_TYPE_RPS_E) && (capabilities.RpsIsSupported == FALSE)){
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-13s | %-14s | %-13s |\n", "rps", "Not supported", "Not supported", "Not supported");
            }
            else {
                if (typePs == PDL_POWER_PS_TYPE_PS_E)
                    powerTypeStr = "ps";
                else if (typePs == PDL_POWER_PS_TYPE_RPS_E)
                    powerTypeStr = "rps";
                pdlStatus = pdlPowerDebugStatusAccessInfoGet(typePs, &interfacePs);
                PDL_CHECK_STATUS(pdlStatus);
                pdlStatus = iDbgPdlConvertEnumToString (IDBG_PDL_UTILS_INTERFACE_TYPE, interfacePs.interfaceType, &powerInterfaceTypeStr);
                PDL_CHECK_STATUS(pdlStatus);
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-13d | %-14s | %-13d |\n", powerTypeStr, interfacePs.values.activeValue, powerInterfaceTypeStr, interfacePs.interfaceId);            
            }
        }
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

    return PDL_OK;
}

/*$ END OF iDbgPdlPowerGetInfo */


/*****************************************************************************
* FUNCTION NAME: iDbgPdlPowerGetHwInfo
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlPowerGetHwInfo (   
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN char                             * powerId
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_POWER_XML_CAPABILITIES_STC          capabilities;
    PDL_POWER_PS_TYPE_ENT                   typePs = PDL_POWER_PS_TYPE_PS_E;
    PDL_POWER_STATUS_ENT                    powerStatus;
    PDL_POWER_RPS_STATUS_ENT                rpsStatus;
    char                                  * powerTypeStr = NULL, * powerStatusStr = NULL, * rpsStatusStr = NULL;                                   
    BOOLEAN                                 flagAll = FALSE;
    PDL_STATUS                              pdlStatus;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    pdlStatus = pdlPowerDbCapabilitiesGet(&capabilities);
    PDL_CHECK_STATUS(pdlStatus);
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-14s | %-14s |\n", "power-id", " power-status ", "  rps-status  ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-14s | %-14s |\n", "--------", "--------------", "--------------");
    if (strcmp(powerId, "all") == 0){
        flagAll = TRUE;
    }
    if (flagAll == FALSE){ 
        if  ((strcmp(powerId, "rps") == 0) && (capabilities.RpsIsSupported == FALSE)){
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-14s | %-14s |\n", powerId, "Not supported", "Not supported");
        }
        else {
            if (strcmp(powerId, "rps") == 0)
                typePs = PDL_POWER_PS_TYPE_RPS_E;
            pdlStatus = pdlPowerHwStatusGet(typePs, &powerStatus, &rpsStatus);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_POWER_STATUS_TYPE_E, rpsStatus, &rpsStatusStr);
            PDL_CHECK_STATUS(pdlStatus);
            if (powerStatus == PDL_POWER_STATUS_ACTIVE_E)
                powerStatusStr = "Active";
            else 
                powerStatusStr = "Not Active";
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-14s | %-14s |\n", powerId, powerStatusStr, rpsStatusStr); 
        }
    }
    else {
        for (; typePs<PDL_POWER_PS_TYPE_PD_E; typePs++){
            if ((typePs == PDL_POWER_PS_TYPE_RPS_E) && (capabilities.RpsIsSupported == FALSE)){
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-14s | %-14s |\n", "rps", "Not supported", "Not supported", "Not supported");
            }
            else {
                if (typePs == PDL_POWER_PS_TYPE_PS_E)
                    powerTypeStr = "ps";
                else if (typePs == PDL_POWER_PS_TYPE_RPS_E)
                    powerTypeStr = "rps";
                pdlStatus = pdlPowerHwStatusGet(typePs, &powerStatus, &rpsStatus);
                PDL_CHECK_STATUS(pdlStatus);
                pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_POWER_STATUS_TYPE_E, rpsStatus, &rpsStatusStr);
                PDL_CHECK_STATUS(pdlStatus);
                if (powerStatus == PDL_POWER_STATUS_ACTIVE_E)
                    powerStatusStr = "Active";
                else 
                    powerStatusStr = "Not Active";
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-14s | %-14s |\n", powerTypeStr, powerStatusStr, rpsStatusStr);             
            }
        }
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

    return PDL_OK;
}

/*$ END OF iDbgPdlPowerGetHwInfo */



/*****************************************************************************
* FUNCTION NAME: iDbgPdlPowerRunValidation
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlPowerRunValidation (   
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN char                              *powerId
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_POWER_XML_CAPABILITIES_STC          capabilities;
    PDL_POWER_PS_TYPE_ENT                   typePs = PDL_POWER_PS_TYPE_PS_E;
    PDL_POWER_PS_INTERFACE_STC              interfacePs;
    PDL_POWER_STATUS_ENT                    powerStatus;
    PDL_POWER_RPS_STATUS_ENT                rpsStatus;
    char                                  * powerTypeStr = NULL, * statusStr = NULL, * infoStr = NULL;                                   
    BOOLEAN                                 flagAll = FALSE;
    PDL_STATUS                              pdlStatus;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    pdlStatus = pdlPowerDbCapabilitiesGet(&capabilities);
    PDL_CHECK_STATUS(pdlStatus);
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-14s | %-14s |\n", "power-id", "     Info     ", "    status    ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-14s | %-14s |\n", "--------", "--------------", "--------------");
    if (strcmp(powerId, "all") == 0){
        flagAll = TRUE;
    }
    if (flagAll == FALSE){ 
        if  ((strcmp(powerId, "rps") == 0) && (capabilities.RpsIsSupported == FALSE)){
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-14s | %-14s |\n", powerId, "Not supported", "Not supported");
        }
        else {
            if (strcmp(powerId, "rps") == 0)
                typePs = PDL_POWER_PS_TYPE_RPS_E;
            pdlStatus = pdlPowerHwStatusGet(typePs, &powerStatus, &rpsStatus);
            if (pdlStatus == PDL_OK)
                statusStr = "Pass";
            else 
                pdlStatusToString(pdlStatus, &statusStr); 
            pdlStatus = pdlPowerDbCapabilitiesGet(&capabilities);
            if (pdlStatus == PDL_OK){
                pdlStatus = pdlPowerDebugStatusAccessInfoGet(typePs, &interfacePs);
                if (pdlStatus == PDL_OK){
                    infoStr = "Pass";
                }
                else pdlStatusToString(pdlStatus, &infoStr);
            }
            else pdlStatusToString(pdlStatus, &infoStr);
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-14s | %-14s |\n", powerId, infoStr, statusStr); 
        }
    }
    else {
        for (; typePs<PDL_POWER_PS_TYPE_PD_E; typePs++){
            if ((typePs == PDL_POWER_PS_TYPE_RPS_E) && (capabilities.RpsIsSupported == FALSE)){
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-14s | %-14s |\n", "rps", "Not supported", "Not supported", "Not supported");
            }
            else {
                if (typePs == PDL_POWER_PS_TYPE_PS_E)
                    powerTypeStr = "ps";
                else if (typePs == PDL_POWER_PS_TYPE_RPS_E)
                    powerTypeStr = "rps";
                pdlStatus = pdlPowerHwStatusGet(typePs, &powerStatus, &rpsStatus);
                if (pdlStatus == PDL_OK)
                    statusStr = "Pass";
                else 
                    pdlStatusToString(pdlStatus, &statusStr); 
                pdlStatus = pdlPowerDbCapabilitiesGet(&capabilities);
                if (pdlStatus == PDL_OK){
                    pdlStatus = pdlPowerDebugStatusAccessInfoGet(typePs, &interfacePs);
                    if (pdlStatus == PDL_OK){
                        infoStr = "Pass";
                    }
                    else pdlStatusToString(pdlStatus, &infoStr);
                }
                else pdlStatusToString(pdlStatus, &infoStr); 
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-14s | %-14s |\n", powerTypeStr, infoStr, statusStr);
            }
        }
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

    return PDL_OK;
}

/*$ END OF iDbgPdlPowerRunValidation */
            
            
            
            
            
   