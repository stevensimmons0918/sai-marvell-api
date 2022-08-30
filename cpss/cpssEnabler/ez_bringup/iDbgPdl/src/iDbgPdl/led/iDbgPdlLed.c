/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file iDbgPdlSensor.c
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
 * @brief Debug lib button module
 * 
 * @version   1 
********************************************************************************
*/
/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/

#include <stdio.h>
#include <stdlib.h>
#include <pdl/led/pdlLed.h>
#include <pdl/led/pdlLedDebug.h>
#include <pdl/led/private/prvPdlLed.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <iDbgPdl/utils/iDbgPdlUtils.h>
#ifndef _WIN32
#include <unistd.h>
extern int usleep(unsigned int);
#endif

/*****************************************************************************
* FUNCTION NAME: iDbgPdSingleLedShow
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdSingleLedShow (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_LED_ID_ENT                     ledId,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, j, key, functionNum, paramNum, paramValue;
    PDL_STATUS						pdlStatus;
    PDL_LED_XML_CAPABILITIES_STC    ledCapabilities;
    PDL_LED_INTERFACE_STC           ledInterface;
    PDL_LED_STATE_ENT               state = PDL_LED_STATE_LAST_E;
    PDL_LED_COLOR_ENT               color = PDL_LED_COLOR_LAST_E;
    char                          * ledTypePtr, *ledColorPtr, *ledStatePtr;
    char                            ledStreamFunctionName[100], ledStreamParamName[100], ledStreamReadableParamName[100], tempStr[120];
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlLedDbDeviceCapabilitiesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ledId),
                                                                              IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &ledCapabilities));
    }
    else if (ledId > PDL_LED_TYPE_PORT_FIRST_E && ledId < PDL_LED_TYPE_PORT_LAST_E) {
        IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlLedDbPortCapabilitiesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ledId),
                                                                            IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                                                            IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, logicalPort),
                                                                            IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &ledCapabilities));
    }
    else {
        return PDL_BAD_PARAM;
    }
    PDL_CHECK_STATUS(pdlStatus);
    if (ledCapabilities.isSupported) {
        pdlStatus = iDbgPdlConvertEnumToString (IDBG_PDL_UTILS_LED_TYPE, ledId, &ledTypePtr);
        PDL_CHECK_STATUS(pdlStatus);
        if (ledCapabilities.ledType == PDL_LED_TYPE_GPIO_E) {
            state = PDL_LED_STATE_SOLID_E;
            for (color = PDL_LED_COLOR_GREEN_E; color < PDL_LED_COLOR_LAST_E; color++) {
                IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlLedDebugInterfaceGetFirst,IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ledId),
                                                                                    IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                                                                    IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, logicalPort),
                                                                                    IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, state),
                                                                                    IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, color),
                                                                                    IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &ledInterface));               
                if (pdlStatus == PDL_OK && ledInterface.values.value.gpioValue != PDL_LED_UNSUPPORTED_VALUE) {
                    pdlStatus = iDbgPdlConvertEnumToString (IDBG_PDL_UTILS_LED_COLOR, color, &ledColorPtr);
                    PDL_CHECK_STATUS(pdlStatus);
                    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-20s | %-8s | %-12d | %-18d |\n",
                                                    ledTypePtr,
                                                    "GPIO",
                                                    "Not Relevant",
                                                    ledColorPtr,
                                                    ledInterface.ledInterfaceId,
                                                    ledInterface.values.value.gpioValue);
                }
            }
        }
        else if (ledCapabilities.ledType == PDL_LED_TYPE_I2C_E || 
                 ledCapabilities.ledType == PDL_LED_TYPE_SMI_E ||
                 ledCapabilities.ledType == PDL_LED_TYPE_XSMI_E) {
            for (state = PDL_LED_STATE_OFF_E; state < PDL_LED_STATE_LAST_E; state ++) {
                for (color = PDL_LED_COLOR_NONE_E; color < PDL_LED_COLOR_LAST_E; color++) {
                    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlLedDebugInterfaceGetFirst,IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ledId),
                                                                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                                                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, logicalPort),
                                                                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, state),
                                                                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, color),
                                                                                        IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &ledInterface));               
                    if (pdlStatus == PDL_OK) {
                        pdlStatus = iDbgPdlConvertEnumToString (IDBG_PDL_UTILS_LED_COLOR, color, &ledColorPtr);
                        PDL_CHECK_STATUS(pdlStatus);
                        pdlStatus = iDbgPdlConvertEnumToString (IDBG_PDL_UTILS_LED_STATE, state, &ledStatePtr);
                        PDL_CHECK_STATUS(pdlStatus);
                        if (ledCapabilities.ledType == PDL_LED_TYPE_I2C_E && ledInterface.values.value.i2cValue != PDL_LED_UNSUPPORTED_VALUE) {
                            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-20s | %-8s | %-12d | %-18d |\n",
                                                            ledTypePtr,
                                                            "I2C",
                                                            ledStatePtr,
                                                            ledColorPtr,
                                                            ledInterface.ledInterfaceId,
                                                            ledInterface.values.value.i2cValue);
                        }
                        else if ((ledCapabilities.ledType == PDL_LED_TYPE_SMI_E || ledCapabilities.ledType == PDL_LED_TYPE_XSMI_E) &&
                                 ledInterface.values.value.smiValue.value != PDL_LED_UNSUPPORTED_VALUE) {
                            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-20s | %-8s | %-12d | %-10s: %6d |\n",
                                                            ledTypePtr,
                                                            ledCapabilities.ledType == PDL_LED_TYPE_SMI_E ? "SMI" : "XSMI",
                                                            ledStatePtr,
                                                            ledColorPtr,
                                                            ledInterface.ledInterfaceId,
                                                            "devOrPage", ledInterface.values.value.smiValue.devOrPage);
                            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-20s | %-8s | %-12s | %-10s: 0x%04x |\n", "", "", "", "", "", "regAddress", ledInterface.values.value.smiValue.registerAddress);
                            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-20s | %-8s | %-12s | %-10s: 0x%04x |\n", "", "", "", "", "", "value", ledInterface.values.value.smiValue.value);
                            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-20s | %-8s | %-12s | %-10s: 0x%04x |\n", "", "", "", "", "", "mask", ledInterface.values.value.smiValue.mask);
                            /* SMI init state might have several init values */
                            if (state == PDL_LED_STATE_INIT_E && color == PDL_LED_COLOR_NONE_E) {
                                key = 0;
                                pdlStatus = pdlLedDebugInterfaceGetNext (ledId, dev, logicalPort, state, color, key, &ledInterface);
                                while (pdlStatus == PDL_OK) {
                                    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-20s | %-8s | %-12s | %-10s: %6d |\n", "", "", "", "", "", "devOrPage", ledInterface.values.value.smiValue.devOrPage);
                                    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-20s | %-8s | %-12s | %-10s: 0x%04x |\n", "", "", "", "", "", "regAddress", ledInterface.values.value.smiValue.registerAddress);
                                    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-20s | %-8s | %-12s | %-10s: 0x%04x |\n", "", "", "", "", "", "value", ledInterface.values.value.smiValue.value);
                                    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-20s | %-8s | %-12s | %-10s: 0x%04x |\n", "", "", "", "", "", "mask", ledInterface.values.value.smiValue.mask);
                                    key++;
                                    pdlStatus = pdlLedDebugInterfaceGetNext (ledId, dev, logicalPort, state, color, key, &ledInterface);
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (ledCapabilities.ledType == PDL_LED_TYPE_LEDSTREAM_E) {
            for (state = PDL_LED_STATE_OFF_E; state < PDL_LED_STATE_LAST_E; state ++) {
                pdlStatus = pdlLedStreamFunctionNumGet (ledId, dev, logicalPort, state, &functionNum);
                if (pdlStatus == PDL_OK) {
                    pdlStatus = iDbgPdlConvertEnumToString (IDBG_PDL_UTILS_LED_STATE, state, &ledStatePtr);
                    PDL_CHECK_STATUS(pdlStatus);
                    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-46s | %-18s |\n", ledTypePtr,"LedStream",ledStatePtr,"");
                    for (i = 0; i < functionNum; i++) {
                        IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlLedStreamFunctionGet,IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ledId),
                                                                                       IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                                                                       IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, logicalPort),
                                                                                       IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, state),
                                                                                       IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i),
                                                                                       IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, sizeof(ledStreamFunctionName)/sizeof(char)),
                                                                                       IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &ledStreamFunctionName[0]));          
                        prvPdlOsSnprintf (tempStr, 120, "func #%d: %s", i, ledStreamFunctionName);
                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-67s |\n", "", "", tempStr);
                        IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlLedStreamFunctionParamNumGet,IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ledId),
                                                                                               IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                                                                               IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, logicalPort),
                                                                                               IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, state),
                                                                                               IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i),
                                                                                               IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &paramNum));     
                        PDL_CHECK_STATUS(pdlStatus);
                        for (j = 0; j < paramNum; j++) {
                            pdlStatus = pdlLedStreamFunctionParamGet (ledId, dev, logicalPort, state, i, j, sizeof(ledStreamParamName)/sizeof(char), &ledStreamParamName[0], &paramValue);
                            PDL_CHECK_STATUS(pdlStatus);
                            pdlStatus = iDbgPdlStringReplace (ledStreamParamName, "&gt;", ">", 100, ledStreamReadableParamName);                            
                            PDL_CHECK_STATUS(pdlStatus);
                            prvPdlOsSnprintf (tempStr, 120, "param #%d: %s", j, ledStreamReadableParamName);
                            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s | %-15s | %-46s | %-18d |\n", "", "", tempStr, paramValue);
                        }
                    }
                    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | ------------------------------------------------------------------- |\n", "", "");
                }
            }
        }
        else if (ledCapabilities.ledType == PDL_LED_TYPE_EXTERNAL_DRIVER_E) {
            IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlLedDebugInterfaceGetFirst,IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ledId),
                                                                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                                                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, logicalPort),
                                                                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, PDL_LED_STATE_OFF_E),
                                                                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, PDL_LED_COLOR_NONE_E),
                                                                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &ledInterface));  
			if (pdlStatus == PDL_OK) {
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-20s | %-8s | %-12d | %-18s |\n",
											    ledTypePtr,
											    "EXTERNAL",
											    "----",
											    "----",
											    ledInterface.ledInterfaceId,
											    "---");
            }
        }
        else {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Unsupported led type %d\n", ledCapabilities.ledType);
        }
    }
    return PDL_OK;
}

/*$ END OF iDbgPdSingleLedShow */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlLedSystemGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlLedSystemGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_LED_ID_ENT                     ledId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, firstLedId, lastLedId;
    PDL_STATUS						pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (ledId == IDBG_PDL_ID_ALL) {
        firstLedId = PDL_LED_TYPE_DEVICE_SYSTEM_E;
        lastLedId = PDL_LED_TYPE_DEVICE_LAST_E;
    }
    else {
        firstLedId = ledId;
        lastLedId = ledId + 1;
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-20s | %-8s | %-12s | %-18s |\n", "      Led ID      ", "Interface Type", "       State        ", "Color", "Interface ID", "       Value      ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-20s | %-8s | %-12s | %-18s |\n", "------------------", "--------------", "--------------------", "-----", "------------", "------------------");

    for (i = firstLedId; i < lastLedId; i++) {
        pdlStatus = iDbgPdSingleLedShow(resultPtr, i, PDL_LED_DONT_CARE_VALUE_CNS, PDL_LED_DONT_CARE_VALUE_CNS);
        PDL_CHECK_STATUS(pdlStatus);
    }
    return PDL_OK;
}
/*$ END OF iDbgPdlLedSystemGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlLedPortGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlLedPortGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS						pdlStatus;
    PDL_LED_ID_ENT                  ledId;
    PDL_LED_XML_CAPABILITIES_STC    ledCapabilities;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-20s | %-8s | %-12s | %-18s |\n", "      Led ID      ", "Interface Type", "       State        ", "Color", "Interface ID", "       Value      ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-18s | %-15s | %-20s | %-8s | %-12s | %-18s |\n", "------------------", "--------------", "--------------------", "-----", "------------", "------------------");
    for (ledId = PDL_LED_TYPE_PORT_OOB_LEFT_E; ledId < PDL_LED_TYPE_PORT_LAST_E; ledId++) {
        pdlStatus = pdlLedDbPortCapabilitiesGet( ledId, dev, logicalPort, &ledCapabilities);
        if (pdlStatus == PDL_OK && ledCapabilities.isSupported == TRUE) {
            pdlStatus = iDbgPdSingleLedShow(resultPtr, ledId, dev, logicalPort);
            PDL_CHECK_STATUS(pdlStatus);    
        }
    }
    return PDL_OK;
}
/*$ END OF iDbgPdlLedPortGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlFanSetHwStatus
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlLedSystemSetHwStatus (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_LED_ID_ENT                     ledId,
    IN PDL_LED_STATE_ENT                  state,
    IN PDL_LED_COLOR_ENT                  color
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, firstLedId, lastLedId;
    PDL_STATUS						pdlStatus;
    PDL_LED_XML_CAPABILITIES_STC    ledCapabilities;
    PDL_LED_COLOR_ENT               normalizedColor = color;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (ledId == IDBG_PDL_ID_ALL) {
        firstLedId = PDL_LED_TYPE_DEVICE_SYSTEM_E;
        lastLedId = PDL_LED_TYPE_DEVICE_LAST_E;
    }
    else {
        firstLedId = ledId;
        lastLedId = ledId + 1;
    }
    if (state == PDL_LED_STATE_OFF_E || state == PDL_LED_STATE_INIT_E) {
        normalizedColor = PDL_LED_COLOR_NONE_E;
    }
    for (i = firstLedId; i < lastLedId; i++) {
        pdlStatus = pdlLedDbDeviceCapabilitiesGet (ledId, &ledCapabilities);
        if (pdlStatus == PDL_OK && ledCapabilities.isSupported) {
            IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlLedHwDeviceModeSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i),
                                                                          IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, state),
                                                                          IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, normalizedColor));
            PDL_CHECK_STATUS(pdlStatus);
        }
    }
    return PDL_OK;
}

/*$ END OF iDbgPdlLedSystemSetHwStatus */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlLedSystemSetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlLedSetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_LED_ID_ENT                     ledId,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort,
    IN PDL_LED_STATE_ENT                  state,
    IN PDL_LED_COLOR_ENT                  color,
    IN UINT_32                            value,
    IN UINT_32                            smiDevOrPage,
    IN UINT_32                            smiReg,
    IN UINT_32                            smiMask
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS						pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlLedDebugInfoSet,IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ledId),
                                                                            IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                                                            IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, logicalPort),
                                                                            IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, state),
                                                                            IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, color),
                                                                            IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value),
                                                                            IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, smiDevOrPage),
                                                                            IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, smiReg),
                                                                            IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, smiMask));
    switch (pdlStatus) {
    case PDL_EMPTY:
        IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Led state & color don't exist in DB - Can't add new entries, only change existing ones\n");
        break;
    case PDL_BAD_STATE:
        IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Only \"on\" state is configurable for GPIO leds\n");
        break;
    case PDL_NOT_FOUND:
        IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Led isn't found in DB\n");
        break;
    case PDL_NOT_SUPPORTED:
        IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Changing ledstream/init values for smi led isn't supported\n");
        break;
    case PDL_OUT_OF_RANGE:
        IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Value is out of range.\nGPIO allowed range is [0..1]\nSMI/XSMI mask allowed range is [1..65535]\n");
        break;
    default:
        break;
    }
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}

/*$ END OF iDbgPdlLedSystemSetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlLedSinglePortSetHwStatus
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlLedSinglePortSetHwStatus (
	IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort,
    IN PDL_LED_ID_ENT                     ledId,
    IN PDL_LED_STATE_ENT                  state,
    IN PDL_LED_COLOR_ENT                  color
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                          * ledTypeStr, * pdlStatusStr;
	PDL_STATUS						pdlStatus, pdlStatus2;
	PDL_LED_XML_CAPABILITIES_STC    ledCapabilities;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

	pdlStatus = pdlLedDbPortCapabilitiesGet (ledId,dev, logicalPort, &ledCapabilities);
	if (pdlStatus == PDL_OK && ledCapabilities.ledType == PDL_LED_TYPE_LEDSTREAM_E) {
        pdlStatus = iDbgPdlConvertEnumToString (IDBG_PDL_UTILS_LED_TYPE, ledId, &ledTypeStr);
        PDL_CHECK_STATUS(pdlStatus);
		IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Can't set led port hw mode for dev[%d] port[%d] ledId[%s] since it's a ledstream led\n", dev, logicalPort, ledTypeStr);
		return PDL_NOT_SUPPORTED;
	}
	if (pdlStatus == PDL_OK && ledCapabilities.isSupported) {
		pdlStatus = pdlLedHwPortModeSet(ledId, dev, logicalPort, state, color);
    }
	if (pdlStatus != PDL_OK) {
		pdlStatusToString(pdlStatus, &pdlStatusStr);
        pdlStatus2 = iDbgPdlConvertEnumToString (IDBG_PDL_UTILS_LED_TYPE, ledId, &ledTypeStr);
        PDL_CHECK_STATUS(pdlStatus2);
		IDBG_PDL_LOGGER_API_OUTPUT_LOG ("failed to set dev[%d] port[%d] ledId[%s] error is[%s]\n", dev, logicalPort, ledTypeStr, pdlStatusStr);
	}
    return pdlStatus;
}

/*$ END OF iDbgPdlLedSinglePortSetHwStatus */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlLedPortSetHwStatus
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlLedPortSetHwStatus (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort,
    IN PDL_LED_ID_ENT                     ledId,
    IN PDL_LED_STATE_ENT                  state,
    IN PDL_LED_COLOR_ENT                  color
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         tempDev, tempPort, tempLedId;
    PDL_STATUS						pdlStatus;
    PDL_LED_COLOR_ENT               normalizedColor = color;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (state == PDL_LED_STATE_OFF_E || state == PDL_LED_STATE_INIT_E) {
        normalizedColor = PDL_LED_COLOR_NONE_E;
    }

    if (ledId == IDBG_PDL_ID_ALL) {
        for (pdlStatus = pdlPpDbFirstPortAttributesGet(&tempDev, &tempPort); 
             pdlStatus == PDL_OK; 
             pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort)) 
        {
            for (tempLedId = PDL_LED_TYPE_PORT_FIRST_E + 1; tempLedId < PDL_LED_TYPE_PORT_LAST_E; tempLedId++) {
                iDbgPdlLedSinglePortSetHwStatus (resultPtr, tempDev, tempPort, tempLedId, state, normalizedColor);
            }
        }
    }
    else {
        iDbgPdlLedSinglePortSetHwStatus (resultPtr, dev, logicalPort, ledId, state, normalizedColor);
    }
    return PDL_OK;
}

/*$ END OF iDbgPdlLedPortSetHwStatus */



/*****************************************************************************
* FUNCTION NAME: iDbgPdlLedRunValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlLedRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_LED_ID_ENT                     ledId,
    IN UINT_32                            timeSec 
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, firstLedId, lastLedId;
    char                          * ledTypeStr = NULL, * ledStateStr = NULL,* ledColorStr = NULL, * infoStr = NULL, * hwStr = NULL;
    PDL_LED_TYPE_ENT                ledType;
    PDL_LED_STATE_ENT               ledState;
    PDL_LED_COLOR_ENT               ledColor;
    PDL_LED_XML_CAPABILITIES_STC    capabilitiesPtr;
    PDL_STATUS						pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (ledId == IDBG_PDL_ID_ALL) {
        firstLedId = PDL_LED_TYPE_DEVICE_SYSTEM_E;
        lastLedId = PDL_LED_TYPE_DEVICE_LAST_E;
    }
    else {
        firstLedId = ledId;
        lastLedId = ledId + 1;
    }
    printf("%-15s | %-15s | %-62s|\n", "    Led ID     ", "     Info      ", "                              Hw                              ");
    printf("%-15s | %-15s | %-55s|\n", "---------------", "---------------", "--------------------------------------------------------------"); 
    printf("%-15s | %-15s | %-15s | %-20s | %-20s |\n", "               ", "               ", "     Color     ", "       State        ", "       Hw info      ");
    printf("%-15s | %-15s | %-15s | %-20s | %-20s |\n", "", "", "---------------", "--------------------", "--------------------");    
    for (i = firstLedId; i < lastLedId; i++) {
        pdlStatus = iDbgPdlConvertEnumToString (IDBG_PDL_UTILS_LED_TYPE, i, &ledTypeStr);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = pdlLedDbDeviceCapabilitiesGet (i, &capabilitiesPtr);
        if (pdlStatus == PDL_OK){
            if (capabilitiesPtr.isSupported == TRUE){
                pdlStatus =  pdlLedTypeGet(i, PDL_LED_DONT_CARE_VALUE_CNS , PDL_LED_DONT_CARE_VALUE_CNS, &ledType);
                if (pdlStatus == PDL_OK){
                    infoStr = "Pass";
                }
                else pdlStatusToString(pdlStatus, &infoStr);
                printf("%-15s | %-15s | %-15s | %-20s | %-20s |\n", ledTypeStr, infoStr, "", "", "");
                for (ledColor = PDL_LED_COLOR_NONE_E; ledColor< PDL_LED_COLOR_LAST_E; ledColor++){
                    for (ledState = PDL_LED_STATE_OFF_E; ledState<PDL_LED_STATE_LAST_E; ledState++){             
                        pdlStatus = pdlLedHwDeviceModeSet(i, ledState, ledColor);
                        if (pdlStatus == PDL_OK){
                            hwStr = "Pass";
                        }
                        else pdlStatusToString(pdlStatus, &hwStr);
                        pdlStatus = iDbgPdlConvertEnumToString (IDBG_PDL_UTILS_LED_COLOR, ledColor, &ledColorStr);
                        PDL_CHECK_STATUS(pdlStatus);
                        pdlStatus = iDbgPdlConvertEnumToString (IDBG_PDL_UTILS_LED_STATE, ledState, &ledStateStr);
                        PDL_CHECK_STATUS(pdlStatus); 
                        #ifndef _WIN32
                        usleep(timeSec*1000);  
                        #endif
                        printf("%-15s | %-15s | %-15s | %-20s | %-20s |\n", "", "", ledColorStr, ledStateStr,  hwStr);                                           
                    }
                }
            }
            else {
                infoStr = hwStr = ledColorStr = ledStateStr = "Not Supported";
                printf("%-15s | %-15s | %-15s | %-20s | %-20s |\n", ledTypeStr, infoStr, hwStr, ledColorStr, ledStateStr);               
            }
        }
        else {
            pdlStatusToString(pdlStatus, &infoStr);
            hwStr = ledColorStr = ledStateStr = infoStr;
            printf("%-15s | %-15s | %-15s | %-20s | %-20s |\n", ledTypeStr, infoStr, hwStr, ledColorStr, ledStateStr);           
        }
        printf("%-15s | %-15s | %-15s | %-20s | %-20s |\n", "---------------", "---------------", "---------------", "--------------------", "--------------------");
    }    
    printf("\n\n\n");
    return PDL_OK;
}
/*$ END OF iDbgPdlLedRunValidation */


