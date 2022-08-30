/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlLed.c   
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
 * @brief Platform driver layer - Led related API
 * 
 * @version   1 
********************************************************************************
*/


/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/led/pdlLed.h>
#include <pdl/led/pdlLedDebug.h>
#include <pdl/led/private/prvPdlLed.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdl/packet_processor/private/prvPdlPacketProcessor.h>
#include <pdl/interface/private/prvPdlSmiXsmi.h>
#include <pdl/interface/pdlSmiXsmiDebug.h>
#include <pdl/interface/private/prvPdlGpio.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)                                                                   \
    if (__pdlStatus != PDL_OK) {                                                                        \
    if (prvPdlLedDebugFlag) {                                                                           \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__pdlStatus,PDL_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __pdlStatus;                                                                                 \
    }  
/*macro to check the xml status and debug prints if debug flag on*/
#undef XML_CHECK_STATUS
#define XML_CHECK_STATUS(__xmlStatus)                                                                  \
    if (__xmlStatus != PDL_OK) {                                                                       \
    if (prvPdlLedDebugFlag) {                                                                          \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__xmlStatus,XML_STATUS_ERROR_PRINT_IDENTIFIER);      \
    }                                                                                                  \
    return __xmlStatus;                                                                                \
    }  



/** @brief   The pdl LED port database */
static PDL_DB_PRV_STC       pdlLedPortDb = {0,NULL};                                 /* DB for ports led */
/** @brief   The pdl LED device db[ pdl LED type device last e] */
static PDL_LED_PRV_DB_STC   pdlLedDeviceDb[PDL_LED_TYPE_DEVICE_LAST_E];              /* DB for system led */
/** @brief   The pdl LEDSTREAM interface db */
static PDL_DB_PRV_STC       pdlLedStreamInterfaceDb = {0,NULL};                      /* DB for ledstream interfaces */

/** @brief   The pdl LED state string to enum pairs[ pdl LED state last e] */
static PRV_PDL_LIB_STR_TO_ENUM_STC pdlLedStateStrToEnumPairs[PDL_LED_STATE_LAST_E] = {
    {"off", PDL_LED_STATE_OFF_E},
    {"solid", PDL_LED_STATE_SOLID_E},
    {"lowRateblinking", PDL_LED_STATE_LOWRATEBLINKING_E},
    {"normalRateblinking", PDL_LED_STATE_NORMALRATEBLINKING_E},
    {"highRateblinking", PDL_LED_STATE_HIGHRATEBLINKING_E},
    {"initialize", PDL_LED_STATE_INIT_E}
};
/** @brief   The pdl LED state string to enum */
PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC pdlLedStateStrToEnum = {pdlLedStateStrToEnumPairs, PDL_LED_STATE_LAST_E};

/** @brief   The pdl LED color string to enum pairs[ pdl LED color last e] */
static PRV_PDL_LIB_STR_TO_ENUM_STC pdlLedColorStrToEnumPairs[PDL_LED_COLOR_LAST_E] = {
    {"any", PDL_LED_COLOR_NONE_E},
    {"green", PDL_LED_COLOR_GREEN_E},
    {"amber", PDL_LED_COLOR_AMBER_E}
};
/** @brief   The pdl LED color string to enum */
PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC pdlLedColorStrToEnum = {pdlLedColorStrToEnumPairs, PDL_LED_COLOR_LAST_E};

/** @brief   The pdl LED type string to enum pairs[ pdl LED type port last e] */
static PRV_PDL_LIB_STR_TO_ENUM_STC pdlLedTypeStrToEnumPairs[PDL_LED_TYPE_PORT_LAST_E] = {
    {"system", PDL_LED_TYPE_DEVICE_SYSTEM_E},
    {"rps", PDL_LED_TYPE_DEVICE_RPS_E},
    {"fan", PDL_LED_TYPE_DEVICE_FAN_E},
    {"poe", PDL_LED_TYPE_PORT_POE_E},
    {"port_led_state", PDL_LED_TYPE_PORT_PORT_LED_STATE_E},
    {"unit1", PDL_LED_TYPE_DEVICE_STACK_UNIT1_E},
    {"unit2", PDL_LED_TYPE_DEVICE_STACK_UNIT2_E},
    {"unit3", PDL_LED_TYPE_DEVICE_STACK_UNIT3_E},
    {"unit4", PDL_LED_TYPE_DEVICE_STACK_UNIT4_E},
    {"unit5", PDL_LED_TYPE_DEVICE_STACK_UNIT5_E},
    {"unit6", PDL_LED_TYPE_DEVICE_STACK_UNIT6_E},
    {"unit7", PDL_LED_TYPE_DEVICE_STACK_UNIT7_E},
    {"unit8", PDL_LED_TYPE_DEVICE_STACK_UNIT8_E},
    {"master", PDL_LED_TYPE_DEVICE_STACK_MASTER_E},
    {"illegal", PDL_LED_TYPE_DEVICE_LAST_E},
    {"oob_left", PDL_LED_TYPE_PORT_OOB_LEFT_E},
    {"oob_right", PDL_LED_TYPE_PORT_OOB_RIGHT_E},
    {"port_left", PDL_LED_TYPE_PORT_INBAND_PORT_LEFT_E},
    {"port_right", PDL_LED_TYPE_PORT_INBAND_PORT_RIGHT_E},
};

/** @brief   The pdl LED type string to enum */
PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC pdlLedTypeStrToEnum = {pdlLedTypeStrToEnumPairs, PDL_LED_TYPE_PORT_LAST_E};

/** @brief   The pdl LED XML string to enum pairs[] */
static PRV_PDL_LIB_STR_TO_ENUM_STC pdlLedXMLStrToEnumPairs[] = {
    {"SystemLed", PDL_LED_TYPE_DEVICE_SYSTEM_E},
    {"RPSLed", PDL_LED_TYPE_DEVICE_RPS_E},
    {"FanLed", PDL_LED_TYPE_DEVICE_FAN_E},
    {"PoeLed", PDL_LED_TYPE_PORT_POE_E},
    {"PortLedStateLed", PDL_LED_TYPE_PORT_PORT_LED_STATE_E},
    {"1", PDL_LED_TYPE_DEVICE_STACK_UNIT1_E},
    {"2", PDL_LED_TYPE_DEVICE_STACK_UNIT2_E},
    {"3", PDL_LED_TYPE_DEVICE_STACK_UNIT3_E},
    {"4", PDL_LED_TYPE_DEVICE_STACK_UNIT4_E},
    {"5", PDL_LED_TYPE_DEVICE_STACK_UNIT5_E},
    {"6", PDL_LED_TYPE_DEVICE_STACK_UNIT6_E},
    {"7", PDL_LED_TYPE_DEVICE_STACK_UNIT7_E},
    {"8", PDL_LED_TYPE_DEVICE_STACK_UNIT8_E},
    {"stack-led-master", PDL_LED_TYPE_DEVICE_STACK_MASTER_E}
};
/** @brief   The pdl LED XML string to enum */
static PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC pdlLedXMLStrToEnum = {pdlLedXMLStrToEnumPairs, sizeof (pdlLedXMLStrToEnumPairs) / sizeof(PRV_PDL_LIB_STR_TO_ENUM_STC)};
static PDL_DB_PRV_STC   pdlLedCallbacksDb =  {0,NULL};

static PDL_LED_PRV_CALLBACK_INFO_STC pdlLedCallbacksInternal = {
    NULL,
    NULL,
    &prvPdlLedHwDeviceModeSet,
    &prvPdlLedHwPortModeSet
};

static BOOLEAN          pdlLedInternalDriverBind = FALSE;



/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/* ***************************************************************************
* FUNCTION NAME: pdlLedDbDeviceCapabilitiesGet
*
* DESCRIPTION:   Get led supported capabilities Based on data retrieved from XML
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS pdlLedDbDeviceCapabilitiesGet (
    IN  PDL_LED_ID_ENT                  ledId,
    OUT PDL_LED_XML_CAPABILITIES_STC *  capabilitiesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                              * ledStr;
    PDL_STATUS                          pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (capabilitiesPtr == NULL) {
        pdlStatus = prvPdlLibEnumToStrConvert (&pdlLedTypeStrToEnum, ledId, &ledStr);
        PDL_CHECK_STATUS(pdlStatus);      
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"capabilitiesPtr NULL, LedId %s",ledStr);
        return PDL_BAD_PTR;
    }

    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        memcpy (capabilitiesPtr, &pdlLedDeviceDb[ledId].capabilities, sizeof(PDL_LED_XML_CAPABILITIES_STC));
    }
    else {
        pdlStatus = prvPdlLibEnumToStrConvert (&pdlLedTypeStrToEnum, ledId, &ledStr);
        PDL_CHECK_STATUS(pdlStatus);      
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"capabilitiesPtr NULL, LedId %s",ledStr);
        return PDL_BAD_PARAM;
    }
    return PDL_OK;
}

/*$ END OF pdlLedDbDeviceCapabilitiesGet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedDbPortCapabilitiesGet
*
* DESCRIPTION:   Get led supported capabilities Based on data retrieved from XML
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS pdlLedDbPortCapabilitiesGet (
    IN  PDL_LED_ID_ENT                  ledId,
    IN  UINT_32                         dev,
    IN  UINT_32                         logicalPort,
    OUT PDL_LED_XML_CAPABILITIES_STC *  capabilitiesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                              * ledStr;
    PDL_STATUS                          pdlStatus;
    PDL_LED_PRV_DB_STC 	              * ledPtr;
    PDL_LED_PRV_KEY_STC                 ledKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (capabilitiesPtr == NULL) {
        pdlStatus = prvPdlLibEnumToStrConvert (&pdlLedTypeStrToEnum, ledId, &ledStr);
        PDL_CHECK_STATUS(pdlStatus);      
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"capabilitiesPtr NULL, ledId %s, PORT NUMBER: %d, DEVICE NUMBER: %d",ledStr, logicalPort, dev);
        return PDL_BAD_PTR;
    }

    if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E){
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlDbFind (&pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS (pdlStatus);
        memcpy (capabilitiesPtr, &ledPtr->capabilities, sizeof(PDL_LED_XML_CAPABILITIES_STC));
    }
    else {
        pdlStatus = prvPdlLibEnumToStrConvert (&pdlLedTypeStrToEnum, ledId, &ledStr);
        PDL_CHECK_STATUS(pdlStatus);      
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"capabilitiesPtr NULL, ledId %s, PORT NUMBER: %d, DEVICE NUMBER: %d",ledStr, logicalPort, dev);
        return PDL_BAD_PARAM;
    }
    return PDL_OK;
}

/*$ END OF pdlLedDbPortCapabilitiesGet */


/* ***************************************************************************
* FUNCTION NAME: pdlSensorDbGetCallbacks
*
* DESCRIPTION:   bind external driver with callback set & id
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS prvPdlLedDbGetCallbacks (
    IN  UINT_32                              ledId,
    IN  UINT_32                              dev,
    IN  UINT_32                              logicalPort,
    OUT PDL_LED_PRV_CALLBACK_STC         **  callbacksPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                  * ledStr;
    PDL_STATUS                              pdlStatus, pdlStatus2;
    PDL_LED_PRV_KEY_STC                     ledKey;
    PDL_LED_PRV_DB_STC 	                  * ledPtr;
    PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP    ledDriverId = PDL_INTERFACE_INTERNAL_DRIVER_ID_BOARD;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/  
    if (callbacksPtr == NULL) {
        return PDL_BAD_PTR;
    }

    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        if (pdlLedDeviceDb[ledId].capabilities.ledType == PDL_LED_TYPE_EXTERNAL_DRIVER_E) {
            ledDriverId = pdlLedDeviceDb[ledId].interface.externalDriverId;
        }
    }
    else if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E){
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlDbFind (&pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS (pdlStatus);
        if (ledPtr->capabilities.ledType == PDL_LED_TYPE_EXTERNAL_DRIVER_E) {
            ledDriverId = ledPtr->interface.externalDriverId;
        }
    }

    pdlStatus = prvPdlDbFind (&pdlLedCallbacksDb, (void*) &(ledDriverId), (void**) callbacksPtr);
    if (pdlStatus != PDL_OK) {
        pdlStatus2 = prvPdlLibEnumToStrConvert (&pdlLedTypeStrToEnum, ledId, &ledStr);
        PDL_CHECK_STATUS(pdlStatus2);      
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"can't find callback for ledId=[%s] dev=[%d] port=[%d] pdlStatus=[%d]",ledStr, dev, logicalPort, pdlStatus);
    }
    PDL_CHECK_STATUS(pdlStatus);
  
    return PDL_OK;
}

/*$ END OF prvPdlSensorDbGetCallbacks */

/* ***************************************************************************
* FUNCTION NAME: pdlLedHwDeviceModeSet
*
* DESCRIPTION:   set LED mode (state & color)
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS pdlLedHwDeviceModeSet (
    IN  PDL_LED_ID_ENT          ledId,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/   
    PDL_STATUS                            pdlStatus;
    PDL_LED_PRV_CALLBACK_STC            * callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    pdlStatus = prvPdlLedDbGetCallbacks (ledId, PDL_LED_DONT_CARE_VALUE_CNS, PDL_LED_DONT_CARE_VALUE_CNS, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if(callbacksPtr->callbackInfo.ledHwDeviceModeSetCallbackInfo.ledHwDeviceModeSetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->callbackInfo.ledHwDeviceModeSetCallbackInfo.lockType);
    pdlStatus = callbacksPtr->callbackInfo.ledHwDeviceModeSetCallbackInfo.ledHwDeviceModeSetFun(ledId, state, color);
    prvPdlUnlock(callbacksPtr->callbackInfo.ledHwDeviceModeSetCallbackInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}

/*$ END OF pdlLedHwDeviceModeSet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedHwPortModeSet
*
* DESCRIPTION:   set LED mode (state & color)
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS pdlLedHwPortModeSet (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/   
    PDL_STATUS                            pdlStatus;
    PDL_LED_PRV_CALLBACK_STC            * callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    pdlStatus = prvPdlLedDbGetCallbacks (ledId, dev, logicalPort, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if(callbacksPtr->callbackInfo.ledHwPortModeSetCallbackInfo.ledHwPortModeSetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->callbackInfo.ledHwPortModeSetCallbackInfo.lockType);
    pdlStatus = callbacksPtr->callbackInfo.ledHwPortModeSetCallbackInfo.ledHwPortModeSetFun(ledId, dev, logicalPort, state, color);
    prvPdlUnlock(callbacksPtr->callbackInfo.ledHwPortModeSetCallbackInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}

/*$ END OF pdlLedHwPortModeSet */


/* ***************************************************************************
* FUNCTION NAME: prvPdlLedSetLed
*
* DESCRIPTION:   set LED mode (state & color)
*
* PARAMETERS:   
*                
*****************************************************************************/


static PDL_STATUS prvPdlLedSetLed (
    IN  PDL_LED_PRV_DB_STC    * ledPtr,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/   
    UINT_32                                 i;
    PDL_STATUS                              pdlStatus;
    PDL_LED_SMI_XSMI_VALUE_STC            * smiValuesInfoPtr;
    PDL_LED_SMI_XSMI_KEY_STC                initValueKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (ledPtr == NULL) {
        return PDL_BAD_PTR;
    }

    switch (ledPtr->capabilities.ledType) {
        case PDL_LED_TYPE_GPIO_E:
            if (state == PDL_LED_STATE_SOLID_E || state == PDL_LED_STATE_NORMALRATEBLINKING_E) {
                /* turn active color on and shut down in-active color */
                for (i = PDL_LED_COLOR_GREEN_E; i < PDL_LED_COLOR_LAST_E; i++) {                    
                    if (ledPtr->interface.gpioInterface.ledInterfaceId[color] != PDL_INTERFACE_INVALID_INTERFACE_ID) {
                        if (color == i) {
                            pdlStatus = prvPdlGpioHwSetValue (ledPtr->interface.gpioInterface.ledInterfaceId[i], PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E, ledPtr->values.gpioValues.colorValue[i]);
                            PDL_CHECK_STATUS(pdlStatus);
                            /* write solid or blinking to active color gpio */
                            pdlStatus = prvPdlGpioHwSetValue (ledPtr->interface.gpioInterface.ledInterfaceId[i], PDL_INTERFACE_GPIO_OFFSET_BLINKING_E, state == PDL_LED_STATE_NORMALRATEBLINKING_E ? 1 : 0);
                            PDL_CHECK_STATUS(pdlStatus);
                        }
                        else if (ledPtr->interface.gpioInterface.ledInterfaceId[i] != PDL_INTERFACE_INVALID_INTERFACE_ID) {
                            /* write off and not blinking to non-active color gpio */
                            pdlStatus = prvPdlGpioHwSetValue (ledPtr->interface.gpioInterface.ledInterfaceId[i], PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E, ledPtr->values.gpioValues.colorValue[i] ? 0 : 1);
                            PDL_CHECK_STATUS(pdlStatus);      
                            pdlStatus = prvPdlGpioHwSetValue (ledPtr->interface.gpioInterface.ledInterfaceId[i], PDL_INTERFACE_GPIO_OFFSET_BLINKING_E, 0);
                            PDL_CHECK_STATUS(pdlStatus);
                        }
                    }
                }
            }
            /* turn off all colors */
            else if (state == PDL_LED_STATE_OFF_E) {
                for (i = PDL_LED_COLOR_GREEN_E; i < PDL_LED_COLOR_LAST_E; i++) {
                    if (ledPtr->interface.gpioInterface.ledInterfaceId[i] != PDL_INTERFACE_INVALID_INTERFACE_ID) {
                        pdlStatus = prvPdlGpioHwSetValue (ledPtr->interface.gpioInterface.ledInterfaceId[i], PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E, ledPtr->values.gpioValues.colorValue[i] ? 0 : 1);
                        PDL_CHECK_STATUS(pdlStatus);  
                        pdlStatus = prvPdlGpioHwSetValue (ledPtr->interface.gpioInterface.ledInterfaceId[i], PDL_INTERFACE_GPIO_OFFSET_BLINKING_E, 0);
                        PDL_CHECK_STATUS(pdlStatus);
                    }
                }
            }
            else if (state == PDL_LED_STATE_INIT_E) {
                 /* Do nothing here - init stage of GPIOs Output is done elsewhere */
            }
            else {
                PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL state");
                return PDL_BAD_VALUE;
            }
            break;
        case PDL_LED_TYPE_I2C_E:
            if (ledPtr->values.i2cValues.value[color][state] == PDL_LED_UNSUPPORTED_VALUE) {
                return PDL_NOT_SUPPORTED;
            }
            pdlStatus = prvPdlInterfaceHwSetValue (PDL_INTERFACE_TYPE_I2C_E, ledPtr->interface.i2cInterface.ledInterfaceId, ledPtr->values.i2cValues.value[color][state]);
            PDL_CHECK_STATUS(pdlStatus);  
            break;
        case PDL_LED_TYPE_SMI_E:
        case PDL_LED_TYPE_XSMI_E:
            /* special handling for init state which is a list of values */
            if (state == PDL_LED_STATE_INIT_E) {
                initValueKey.valueNumber = 0;
                pdlStatus = prvPdlDbGetFirst (&ledPtr->values.smiValues.initValueList, (void*) &smiValuesInfoPtr);
                PDL_CHECK_STATUS (pdlStatus);
                while (pdlStatus == PDL_OK) {
                    pdlStatus = pdlSmiXsmiHwSetValue(ledPtr->interface.smiInterface.ledInterfaceId, smiValuesInfoPtr->devOrPage, smiValuesInfoPtr->registerAddress, smiValuesInfoPtr->mask, smiValuesInfoPtr->value);
                    PDL_CHECK_STATUS (pdlStatus);                
                    pdlStatus = prvPdlDbGetNext (&ledPtr->values.smiValues.initValueList, &initValueKey, (void*)&smiValuesInfoPtr);
                    initValueKey.valueNumber++;
                }
            }
            else {
                /* if mask equals 0, this color & state isn't supported */
                if (ledPtr->values.smiValues.value[color][state].mask == 0) {
                    return PDL_NOT_SUPPORTED;
                }
                smiValuesInfoPtr = &ledPtr->values.smiValues.value[color][state];
                pdlStatus = pdlSmiXsmiHwSetValue(ledPtr->interface.smiInterface.ledInterfaceId, smiValuesInfoPtr->devOrPage, smiValuesInfoPtr->registerAddress, smiValuesInfoPtr->mask, smiValuesInfoPtr->value);
                PDL_CHECK_STATUS (pdlStatus);
            }
            break;
        case PDL_LED_TYPE_LEDSTREAM_E:
            return PDL_NOT_SUPPORTED;
        case PDL_LED_TYPE_EXTERNAL_DRIVER_E:
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"Internal driver can't handle external led");
            return PDL_BAD_VALUE;
            break;
        default:
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL interfaceType");
            return PDL_ERROR;
    }
    return PDL_OK;
}

/*$ END OF prvPdlLedSetLed */


/* ***************************************************************************
* FUNCTION NAME: prvPdlLedHwDeviceModeSet
*
* DESCRIPTION:   set LED mode (state & color)
*
* PARAMETERS:   
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlLedHwDeviceModeSet ( IN PDL_LED_ID_ENT ledId, IN PDL_LED_STATE_ENT state, IN PDL_LED_COLOR_ENT color )
 *
 * @brief   Pdl LED hardware mode set
 *
 * @param   ledId   Identifier for the LED.
 * @param   state   The state.
 * @param   color   The color.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLedHwDeviceModeSet (
    IN  PDL_LED_ID_ENT          ledId,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/   
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    char                                  * stateStr, *colorStr, *ledStr; 
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    pdlStatus = prvPdlLibEnumToStrConvert (&pdlLedStateStrToEnum, state, &stateStr);
    PDL_CHECK_STATUS(pdlStatus);  
    pdlStatus = prvPdlLibEnumToStrConvert (&pdlLedColorStrToEnum, color, &colorStr);
    PDL_CHECK_STATUS(pdlStatus);  
    pdlStatus = prvPdlLibEnumToStrConvert (&pdlLedTypeStrToEnum, ledId, &ledStr);
    PDL_CHECK_STATUS(pdlStatus);      
    PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"%s, state[%s] color[%s]",ledStr, stateStr, colorStr);

    /* init / turn off are color invariant */
    if (state == PDL_LED_STATE_INIT_E || state == PDL_LED_STATE_OFF_E) {
        color = PDL_LED_COLOR_NONE_E;
    }
    /* device led */
    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        ledPtr = &pdlLedDeviceDb[ledId];
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL ledId %s",ledStr);
        return PDL_BAD_PARAM;
    }
    if (ledPtr->capabilities.isSupported == PDL_IS_SUPPORTED_NO_E) {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"Unsupported ledId %d",ledId);
        return PDL_NOT_SUPPORTED;
    }
    pdlStatus = prvPdlLedSetLed (ledPtr, state, color);
    if (pdlStatus != PDL_OK) {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"Error setting led %s state=%s color=%s",ledStr, stateStr, colorStr);
    }
    return pdlStatus;
}

/*$ END OF prvPdlLedHwDeviceModeSet */

/**
 * @fn  PDL_STATUS prvPdlLedHwPortModeSet ( IN PDL_LED_ID_ENT ledId, IN UINT_32 dev, IN UINT_32 logicalPort, IN PDL_LED_STATE_ENT state, IN PDL_LED_COLOR_ENT color )
 *
 * @brief   Pdl LED hardware mode set
 *
 * @param   ledId   Identifier for the LED.
 * @param   dev     The development.
 * @param   port    The port.
 * @param   state   The state.
 * @param   color   The color.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLedHwPortModeSet (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/   
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    PDL_LED_PRV_KEY_STC                     ledKey;
    char                                  * stateStr, *colorStr, *ledStr; 
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    pdlStatus = prvPdlLibEnumToStrConvert (&pdlLedStateStrToEnum, state, &stateStr);
    PDL_CHECK_STATUS(pdlStatus);  
    pdlStatus = prvPdlLibEnumToStrConvert (&pdlLedColorStrToEnum, color, &colorStr);
    PDL_CHECK_STATUS(pdlStatus);  
    pdlStatus = prvPdlLibEnumToStrConvert (&pdlLedTypeStrToEnum, ledId, &ledStr);
    PDL_CHECK_STATUS(pdlStatus);      
    PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"%s, Port [%d], Device[%d] state[%s] color[%s]",ledStr, logicalPort, dev, stateStr, colorStr);

    /* init / turn off are color invariant */
    if (state == PDL_LED_STATE_INIT_E || state == PDL_LED_STATE_OFF_E) {
        color = PDL_LED_COLOR_NONE_E;
    }
    /* port led */
    if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E){
            ledKey.dev = dev;
            ledKey.logicalPort = logicalPort;
            ledKey.ledId = ledId;
            pdlStatus = prvPdlDbFind (&pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
            PDL_CHECK_STATUS (pdlStatus);
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL ledId, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PARAM;
    }
    
    pdlStatus = prvPdlLedSetLed (ledPtr, state, color);
    if (pdlStatus != PDL_OK) {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"Error setting led %s dev[%d] port[%d] state=%s color=%s",ledStr, dev, logicalPort, stateStr, colorStr);
    }
    return pdlStatus;
}

/*$ END OF prvPdlLedHwPortModeSet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedDebugInfoSet
*
* DESCRIPTION:   change LED value for given state & color
*
* PARAMETERS:   
*               
* RETURN VALUES: PDL_OK             -   success
                 PDL_NOT_FOUND      -   led doesn't exists in XML
                 PDL_BAD_STATE      -   given state&color doesn't exist for this led type (GPIO)
                 PDL_EMPTY          -   user is trying to set a value that doesn't already exist in XML
                 PDL_NOT_SUPPORTED  -   changing ledstream led value or init values for smi led aren't supported
*****************************************************************************/

PDL_STATUS pdlLedDebugInfoSet (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color,
	IN	UINT_32					value,
    IN  UINT_32                 devOrPage,
    IN  UINT_32                 registerAddress,
    IN  UINT_32                 mask
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/   
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    PDL_LED_PRV_KEY_STC                     ledKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    /* init / turn off are color invariant */
    if (state == PDL_LED_STATE_INIT_E || state == PDL_LED_STATE_OFF_E) {
        color = PDL_LED_COLOR_NONE_E;
    }
    /* device led */
    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        ledPtr = &pdlLedDeviceDb[ledId];
    }
    /* port led */
    else if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E){
            ledKey.dev = dev;
            ledKey.logicalPort = logicalPort;
            ledKey.ledId = ledId;
            pdlStatus = prvPdlDbFind (&pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
            PDL_CHECK_STATUS (pdlStatus);
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL ledId, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PARAM;
    }
	
    switch (ledPtr->capabilities.ledType) {
        case PDL_LED_TYPE_GPIO_E:
            /* validity checks */
			/* only solid state value can be set for GPIO leds, other values are automatically deducted*/
			if (state != PDL_LED_STATE_SOLID_E) {
				return PDL_BAD_STATE;
			}
            if (value > 1) {
                return  PDL_OUT_OF_RANGE;
            }
			if (ledPtr->interface.gpioInterface.ledInterfaceId[color] != PDL_INTERFACE_INVALID_INTERFACE_ID) {
                ledPtr->values.gpioValues.colorValue[color] = value;
			}
			else {
				return PDL_EMPTY;
			}
            break;
        case PDL_LED_TYPE_I2C_E:
            if (ledPtr->values.i2cValues.value[color][state] != PDL_LED_UNSUPPORTED_VALUE) {
                ledPtr->values.i2cValues.value[color][state] = value;
            }
			else {
				return PDL_EMPTY;
			}
            break;
        case PDL_LED_TYPE_SMI_E:
        case PDL_LED_TYPE_XSMI_E:
            /* init state not supported for set info since it might be a list of values */
            if (state == PDL_LED_STATE_INIT_E) {
                return PDL_NOT_SUPPORTED;
            }
            /* validity checks */
            if (mask == 0) {
                return  PDL_OUT_OF_RANGE;
            }
            if (ledPtr->values.smiValues.value[color][state].mask != 0) {
                ledPtr->values.smiValues.value[color][state].devOrPage = (UINT_8)devOrPage;
                ledPtr->values.smiValues.value[color][state].value = (UINT_16)value;
                ledPtr->values.smiValues.value[color][state].registerAddress = (UINT_16)registerAddress;
                ledPtr->values.smiValues.value[color][state].mask = (UINT_16)mask;
            }
            else {
                return PDL_EMPTY;
            }
            break;
        case PDL_LED_TYPE_LEDSTREAM_E:
            return PDL_NOT_SUPPORTED;
        default:
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL interfaceType, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort,dev);
            return PDL_ERROR;
    }
    return PDL_OK;
}

/*$ END OF pdlLedDebugInfoSet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedDebugInterfaceGet
*
* DESCRIPTION:   Get led access information Based on data retrieved from XML
*
* PARAMETERS:   
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlLedDebugInterfaceGetFirst ( IN PDL_LED_ID_ENT ledId, IN UINT_32 dev, IN UINT_32 logicalPort, IN PDL_LED_STATE_ENT state, IN PDL_LED_COLOR_ENT color, OUT PDL_LED_INTERFACE_STC * interfacePtr )
 *
 * @brief   Pdl LED debug interface get
 *
 * @param           ledId           Identifier for the LED.
 * @param           dev             The development.
 * @param           port            The port.
 * @param           state           The state.
 * @param           color           The color.
 * @param [in,out]  interfacePtr    If non-null, the interface pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlLedDebugInterfaceGetFirst (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color,
    OUT PDL_LED_INTERFACE_STC * interfacePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_LED_PRV_DB_STC 	              * ledPtr;
    PDL_LED_PRV_KEY_STC                 ledKey;
    PDL_LED_SMI_XSMI_VALUE_STC        * smiValuesInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"interfacePtr NULL, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PTR;
    }
    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        ledPtr = &pdlLedDeviceDb[ledId];
    }
    else if (ledId > PDL_LED_TYPE_DEVICE_LAST_E && ledId < PDL_LED_TYPE_PORT_LAST_E){
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlDbFind (&pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS (pdlStatus);
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL ledId, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_VALUE;
    }
	if (color >= PDL_LED_COLOR_LAST_E) {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL color, PORT NUMBER: %d, DEVICE NUMBER: %d COLOR: %d", logicalPort, dev, color);
        return PDL_BAD_VALUE;
	}
	if (state >= PDL_LED_STATE_LAST_E) {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL state, PORT NUMBER: %d, DEVICE NUMBER: %d STATE: %d", logicalPort, dev, state);
        return PDL_BAD_VALUE;
	}

    interfacePtr->interfaceType = ledPtr->capabilities.ledType;
    switch (ledPtr->capabilities.ledType) {
    case PDL_LED_TYPE_GPIO_E:
        interfacePtr->ledInterfaceId = ledPtr->interface.gpioInterface.ledInterfaceId[color];
        interfacePtr->values.value.gpioValue = ledPtr->values.gpioValues.colorValue[color];        
        break;
    case PDL_LED_TYPE_I2C_E:
        interfacePtr->ledInterfaceId = ledPtr->interface.i2cInterface.ledInterfaceId;
        interfacePtr->values.value.i2cValue = ledPtr->values.i2cValues.value[color][state];
        break;
    case PDL_LED_TYPE_SMI_E:
    case PDL_LED_TYPE_XSMI_E:
        if (state == PDL_LED_STATE_INIT_E && color == PDL_LED_COLOR_NONE_E) {
            interfacePtr->ledInterfaceId = ledPtr->interface.smiInterface.ledInterfaceId;
            pdlStatus = prvPdlDbGetFirst (&ledPtr->values.smiValues.initValueList, (void*) &smiValuesInfoPtr);
            PDL_CHECK_STATUS (pdlStatus);
            memcpy (&interfacePtr->values.value.smiValue, smiValuesInfoPtr, sizeof(PDL_LED_SMI_XSMI_VALUE_STC));
        }
        else {    
            interfacePtr->ledInterfaceId = ledPtr->interface.smiInterface.ledInterfaceId;
            memcpy (&interfacePtr->values.value.smiValue, &(ledPtr->values.smiValues.value[color][state]), sizeof(PDL_LED_SMI_XSMI_VALUE_STC));
        }
        break;
    case PDL_LED_TYPE_EXTERNAL_DRIVER_E:
        interfacePtr->ledInterfaceId = ledPtr->interface.externalDriverId;
        break;
    default:
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL interfaceType, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_ERROR;
    }
    return PDL_OK;
}

/*$ END OF pdlLedDebugInterfaceGetFirst */

/* ***************************************************************************
* FUNCTION NAME: pdlLedDebugInterfaceGetNext
*
* DESCRIPTION:   Get led access information Based on data retrieved from XML 
*                (next is only relevant for multi entry values like init in smi/xsmi led)
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS pdlLedDebugInterfaceGetNext (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color,
    IN  UINT_32                 key,
    OUT PDL_LED_INTERFACE_STC * interfacePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_LED_PRV_DB_STC 	              * ledPtr;
    PDL_LED_PRV_KEY_STC                 ledKey;
    PDL_LED_SMI_XSMI_VALUE_STC        * smiValuesInfoPtr;
    PDL_LED_SMI_XSMI_KEY_STC            initValueKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"interfacePtr NULL, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PTR;
    }
    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        ledPtr = &pdlLedDeviceDb[ledId];
    }
    else if (ledId > PDL_LED_TYPE_DEVICE_LAST_E && ledId < PDL_LED_TYPE_PORT_LAST_E){
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlDbFind (&pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS (pdlStatus);
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL ledId, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_VALUE;
    }
    interfacePtr->interfaceType = ledPtr->capabilities.ledType;
    switch (ledPtr->capabilities.ledType) {
    case PDL_LED_TYPE_GPIO_E:
        return PDL_NO_MORE;
        break;
    case PDL_LED_TYPE_I2C_E:
        return PDL_NO_MORE;
        break;
    case PDL_LED_TYPE_SMI_E:
    case PDL_LED_TYPE_XSMI_E:
        if (state == PDL_LED_STATE_INIT_E && color == PDL_LED_COLOR_NONE_E) {
            interfacePtr->ledInterfaceId = ledPtr->interface.smiInterface.ledInterfaceId;
            initValueKey.valueNumber = key;
            pdlStatus = prvPdlDbGetNext (&ledPtr->values.smiValues.initValueList, &initValueKey, (void*)&smiValuesInfoPtr);
            PDL_CHECK_STATUS (pdlStatus);
            memcpy (&interfacePtr->values.value.smiValue, smiValuesInfoPtr, sizeof(PDL_LED_SMI_XSMI_VALUE_STC));
        }
        else {    
            return PDL_NO_MORE;
        }
        break;
    default:
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL interfaceType, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort,dev);
        return PDL_ERROR;
    }
    return PDL_OK;
}

/*$ END OF pdlLedDebugInterfaceGetNext */

/* ***************************************************************************
* FUNCTION NAME: pdlLedDebugInterfaceSet
*
* DESCRIPTION:   Set led access information
*
* PARAMETERS:   
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlLedDebugInterfaceSet ( IN PDL_LED_ID_ENT ledId, IN UINT_32 dev, IN UINT_32 logicalPort, IN PDL_LED_STATE_ENT state, IN PDL_LED_COLOR_ENT color, IN PDL_LED_VALUES_STC * valuesPtr )
 *
 * @brief   Pdl LED debug interface set
 *
 * @param           ledId       Identifier for the LED.
 * @param           dev         The development.
 * @param           port        The port.
 * @param           state       The state.
 * @param           color       The color.
 * @param [in,out]  valuesPtr   If non-null, the values pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlLedDebugInterfaceSet (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color,
    IN  PDL_LED_VALUES_STC    * valuesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_LED_PRV_DB_STC 	              * ledPtr;
    PDL_LED_PRV_KEY_STC                 ledKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (valuesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"valuesPtr NULL, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort,dev);
        return PDL_BAD_PTR;
    }
    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        ledPtr = &pdlLedDeviceDb[ledId];
    }
    else if (ledId > PDL_LED_TYPE_DEVICE_LAST_E && ledId < PDL_LED_TYPE_PORT_LAST_E){
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlDbFind (&pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS (pdlStatus);
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL ledId, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort,dev);
        return PDL_BAD_VALUE;
    }
    switch (ledPtr->capabilities.ledType) {
    case PDL_LED_TYPE_GPIO_E:
        return PDL_NOT_SUPPORTED;
    case PDL_LED_TYPE_I2C_E:
        ledPtr->values.i2cValues.value[color][state] = valuesPtr->value.i2cValue;
        break;
    default:
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL interfaceType, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort,dev);
        return PDL_ERROR;
    }
    return PDL_OK;
}

/*$ END OF pdlLedDebugInterfaceGet */

/**
 * @fn  PDL_STATUS prvPdlStackNumToLedId ( )
 *
 * @brief   convert stack unit number to led id
 *
 * @param   unitNum         number of unit in stack
 * @param   xmlledIdId      Identifier for stack led
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlStackNumToLedId (
    IN   UINT_8                              unitNum,
    OUT  PDL_LED_ID_ENT                    * ledIdPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (ledIdPtr == NULL) {
        return PDL_BAD_PTR;
    }
    if (unitNum > 8) {
        return PDL_BAD_PARAM;
    }
    *ledIdPtr = PDL_LED_TYPE_DEVICE_STACK_UNIT1_E + (unitNum-1);
    return PDL_OK;
}
/*$ END OF prvPdlStackNumToLedId */

/**
 * @fn  PDL_STATUS prvPdlLedXMLSingleGpioLedParser (  )
 *
 * @brief   Prv pdl GPIO LED XML parser
 *
 * @param   dev     dev if relevant. for system led use PDL_LED_DONT_CARE_VALUE_CNS
 * @param   port    port if relevant. for system led use PDL_LED_DONT_CARE_VALUE_CNS
 * @param   ledId   Identifier for the led being parsed
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLedXMLSingleGpioLedParser (
    IN  UINT_32                             dev,
    IN  UINT_32                             logicalPort,
    IN  PDL_LED_ID_ENT                      ledId,
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                    pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    UINT_32                                 maxSize, value;
    PDL_LED_COLOR_ENT                       ledColor = PDL_LED_COLOR_NONE_E;
    XML_PARSER_NODE_DESCRIPTOR_TYP          xmlLedChildId, xmlLedColorId;
    XML_PARSER_RET_CODE_TYP                 xmlStatus2 = XML_PARSER_RET_CODE_OK, xmlStatus3;
    PDL_STATUS                              pdlStatus;
    PDL_INTERFACE_TYPE_ENT                  interfaceType;
    PDL_INTERFACE_TYP                       interfaceId = XML_PARSER_NOT_INITIALIZED_CNS;
    PDL_LED_PRV_DB_STC                      portLedInfo, *portLedInfoPtr;
    PDL_LED_PRV_KEY_STC                     portLedKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    memset (&portLedInfo, 0, sizeof(portLedInfo));
    memset (&portLedKey, 0, sizeof(portLedKey));
    /* initialize all led color interfaceId to invalid */
    for (ledColor = PDL_LED_COLOR_NONE_E; ledColor < PDL_LED_COLOR_LAST_E; ledColor++) {
        if (dev == PDL_LED_DONT_CARE_VALUE_CNS || logicalPort == PDL_LED_DONT_CARE_VALUE_CNS) {
            pdlLedDeviceDb[ledId].interface.gpioInterface.ledInterfaceId[ledColor] = PDL_INTERFACE_INVALID_INTERFACE_ID;
            pdlLedDeviceDb[ledId].values.gpioValues.colorValue[ledColor] = PDL_LED_UNSUPPORTED_VALUE;

        }
        else {
            portLedInfo.interface.gpioInterface.ledInterfaceId[ledColor] = PDL_INTERFACE_INVALID_INTERFACE_ID;
            portLedInfo.values.gpioValues.colorValue[ledColor] = PDL_LED_UNSUPPORTED_VALUE;
        }
    }
    xmlLedChildId = xmlId;
    /* go over all colors*/
    while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus2);        
        if (xmlParserIsEqualName (xmlLedChildId, "led-gpio-color-list") == XML_PARSER_RET_CODE_OK) {
            xmlStatus3 = xmlParserGetFirstChild (xmlLedChildId, &xmlLedColorId);
            while (xmlStatus3 != XML_PARSER_RET_CODE_NOT_FOUND) {
                XML_CHECK_STATUS(xmlStatus3);
                if (xmlParserIsEqualName (xmlLedColorId, "led-color") == XML_PARSER_RET_CODE_OK) {
                    maxSize = PDL_XML_MAX_TAG_LEN;
                    xmlParserGetValue(xmlLedColorId, &maxSize, &pdlTagStr[0]);
                    pdlStatus = prvPdlLibStrToEnumConvert (&pdlLedColorStrToEnum, pdlTagStr, (UINT_32*) &ledColor);
                    PDL_CHECK_STATUS(pdlStatus);
                }
                else if (xmlParserIsEqualName (xmlLedColorId, "led-gpio") == XML_PARSER_RET_CODE_OK) {
                    maxSize = sizeof(value);
                    pdlStatus = prvPdlInterfaceAndValueXmlParser(xmlLedColorId, maxSize, &interfaceType, &interfaceId, &value);
                    PDL_CHECK_STATUS(pdlStatus);
                }
                else {
                    PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlLedColorId POINTER OR NAME DON'T MATCH TO: pin-value");
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus3 = xmlParserGetNextSibling (xmlLedColorId, &xmlLedColorId);
            }
            if (ledColor < PDL_LED_COLOR_LAST_E) {
            	if (interfaceId == XML_PARSER_NOT_INITIALIZED_CNS) {
                    PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"INVALID interfaceId (not set)");
                    return PDL_XML_PARSE_ERROR;
            	}
				/* insert to system led DB */
				if (dev == PDL_LED_DONT_CARE_VALUE_CNS || logicalPort == PDL_LED_DONT_CARE_VALUE_CNS) {
					pdlLedDeviceDb[ledId].values.gpioValues.colorValue[ledColor] = value;
					pdlLedDeviceDb[ledId].interface.gpioInterface.ledInterfaceId[ledColor] = interfaceId;
					pdlLedDeviceDb[ledId].capabilities.isSupported = TRUE;
					pdlLedDeviceDb[ledId].capabilities.ledType = PDL_LED_TYPE_GPIO_E;
				}
				/* create port led info */
				else {
					portLedInfo.capabilities.ledType = PDL_LED_TYPE_GPIO_E;
					portLedInfo.capabilities.isSupported = TRUE;
					portLedInfo.values.gpioValues.colorValue[ledColor] = value;
					portLedInfo.interface.gpioInterface.ledInterfaceId[ledColor] = interfaceId;
				}
            }
        }
        else {
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlLedChildId POINTER OR NAME DON'T MATCH TO: color");
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus2 = xmlParserGetNextSibling (xmlLedChildId, &xmlLedChildId);           /* get next color */
    }
    /**/
    if (dev != PDL_LED_DONT_CARE_VALUE_CNS && logicalPort != PDL_LED_DONT_CARE_VALUE_CNS) {
        portLedKey.dev = dev;
        portLedKey.logicalPort = logicalPort;
        portLedKey.ledId = ledId;
        pdlStatus = prvPdlDbAdd (&pdlLedPortDb, (void*) &portLedKey, (void*) &portLedInfo, (void**) &portLedInfoPtr);
        PDL_CHECK_STATUS (pdlStatus);
    }
    return PDL_OK;
}

/**
 * @fn  PDL_STATUS prvPdlLedXMLSingleExternalDriverLedParser (  )
 *
 * @brief   Prv pdl ExternalDriver LED XML parser
 *
 * @param   dev     dev if relevant. for system led use PDL_LED_DONT_CARE_VALUE_CNS
 * @param   port    port if relevant. for system led use PDL_LED_DONT_CARE_VALUE_CNS
 * @param   ledId   Identifier for the led being parsed
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLedXMLSingleExternalDriverLedParser (
    IN  UINT_32                             dev,
    IN  UINT_32                             logicalPort,
    IN  PDL_LED_ID_ENT                      ledId,
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                      portLedInfo, *portLedInfoPtr;
    PDL_LED_PRV_KEY_STC                     portLedKey;
    XML_PARSER_RET_CODE_TYP                 xmlStatus=XML_PARSER_RET_CODE_OK;
    UINT_32                                 maxSize, extDrvId;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

    if (xmlParserIsEqualName (xmlId, "external-driver-id") == XML_PARSER_RET_CODE_OK) {
        memset (&portLedInfo, 0, sizeof(portLedInfo));
        memset (&portLedKey, 0, sizeof(portLedKey));

        maxSize = sizeof(extDrvId);
        xmlStatus = xmlParserGetValue(xmlId, &maxSize, &extDrvId);
        XML_CHECK_STATUS(xmlStatus);

        /* port led */
        if (dev != PDL_LED_DONT_CARE_VALUE_CNS && logicalPort != PDL_LED_DONT_CARE_VALUE_CNS) {
            portLedKey.dev = dev;
            portLedKey.logicalPort = logicalPort;
            portLedKey.ledId = ledId;
            portLedInfo.interface.externalDriverId = extDrvId;
            portLedInfo.capabilities.ledType = PDL_LED_TYPE_EXTERNAL_DRIVER_E;
            portLedInfo.capabilities.isSupported = TRUE;
            pdlStatus = prvPdlDbAdd (&pdlLedPortDb, (void*) &portLedKey, (void*) &portLedInfo, (void**) &portLedInfoPtr);
            PDL_CHECK_STATUS (pdlStatus);
        }
        else { /* insert to system led DB */
            pdlLedDeviceDb[ledId].interface.externalDriverId = extDrvId;
            pdlLedDeviceDb[ledId].capabilities.isSupported = TRUE;
            pdlLedDeviceDb[ledId].capabilities.ledType = PDL_LED_TYPE_EXTERNAL_DRIVER_E;
        }
    }

    return PDL_OK;
}

/*$ END OF prvPdlLedXMLSingleExternalDriverLedParser */

/**
 * @fn  PDL_STATUS prvPdlLedXMLSingleI2cLedParser (  )
 *
 * @brief   Prv pdl I2C LED XML parser
 *
 * @param   dev     dev if relevant. for system led use PDL_LED_DONT_CARE_VALUE_CNS
 * @param   port    port if relevant. for system led use PDL_LED_DONT_CARE_VALUE_CNS
 * @param   ledId   Identifier for the led being parsed
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLedXMLSingleI2cLedParser (
    IN  UINT_32                             dev,
    IN  UINT_32                             logicalPort,
    IN  PDL_LED_ID_ENT                      ledId,
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                    pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    UINT_32                                 maxSize;
    PDL_LED_COLOR_ENT                       ledColor = PDL_LED_COLOR_NONE_E;
    PDL_LED_STATE_ENT                       ledState;
    XML_PARSER_NODE_DESCRIPTOR_TYP          xmlLedColorId, xmlI2cChildId, xmlI2cLedId, xmlI2cLedColorValueId;
    XML_PARSER_RET_CODE_TYP                 xmlStatus2, xmlStatus = XML_PARSER_RET_CODE_OK;
    PDL_STATUS                              pdlStatus;
    PDL_INTERFACE_TYPE_ENT                  interfaceType;
    PDL_INTERFACE_TYP                       interfaceId;
    PDL_LED_PRV_DB_STC                      portLedInfo, *portLedInfoPtr;
    PDL_LED_PRV_KEY_STC                     portLedKey;
    BOOLEAN                                 state = FALSE, color = FALSE, isInterface;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    memset (&portLedInfo, 0, sizeof(portLedInfo));
    memset (&portLedInfo.values, PDL_LED_UNSUPPORTED_VALUE, sizeof (portLedInfo.values));
    memset (&portLedKey, 0, sizeof(portLedKey));
    /* init all color & states to unsupported */
    for (ledColor = 0; ledColor < PDL_LED_COLOR_LAST_E; ledColor++) {
        for (ledState = 0; ledState < PDL_LED_STATE_LAST_E; ledState++) {
            portLedInfo.values.i2cValues.value[ledColor][ledState] = PDL_LED_UNSUPPORTED_VALUE;
        }
    }
    ledColor = PDL_LED_COLOR_NONE_E;
    portLedInfo.capabilities.ledType = PDL_LED_TYPE_I2C_E;
    xmlI2cChildId = xmlId;
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        if (xmlParserIsEqualName (xmlI2cChildId, "led-i2c") == XML_PARSER_RET_CODE_OK) {       
            xmlStatus2 = xmlParserGetFirstChild (xmlI2cChildId, &xmlI2cLedId);
            while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                pdlStatus = prvPdlInterfaceXmlIsInterface(xmlI2cLedId, &isInterface);
                PDL_CHECK_STATUS(pdlStatus); 
                if (isInterface) {
                    pdlStatus = prvPdlInterfaceXmlParser (xmlI2cLedId, &interfaceType, &interfaceId);
                    PDL_CHECK_STATUS(pdlStatus); 
                    portLedInfo.interface.i2cInterface.ledInterfaceId = interfaceId;
                }
                else if (xmlParserIsEqualName (xmlI2cLedId, "led-i2c-init-value") == XML_PARSER_RET_CODE_OK) {
                    maxSize = sizeof (portLedInfo.values.i2cValues.value[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_INIT_E]);
                    xmlStatus = xmlParserGetValue(xmlI2cLedId, &maxSize, &portLedInfo.values.i2cValues.value[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_INIT_E]);
                    XML_CHECK_STATUS(xmlStatus);
                }
                else if (xmlParserIsEqualName (xmlI2cLedId, "led-i2c-off-value") == XML_PARSER_RET_CODE_OK) {
                    maxSize = sizeof (portLedInfo.values.i2cValues.value[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_OFF_E]);
                    xmlStatus = xmlParserGetValue(xmlI2cLedId, &maxSize, &portLedInfo.values.i2cValues.value[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_OFF_E]);
                    XML_CHECK_STATUS(xmlStatus);
                }
                else {
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus2 = xmlParserGetNextSibling (xmlI2cLedId, &xmlI2cLedId);    
            }
        }
        else if (xmlParserIsEqualName (xmlI2cChildId, "led-i2c-color-list") == XML_PARSER_RET_CODE_OK) {
            xmlStatus2 = xmlParserGetFirstChild (xmlI2cChildId, &xmlLedColorId);
            while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                XML_CHECK_STATUS(xmlStatus2);
                if (xmlParserIsEqualName (xmlLedColorId, "led-color") == XML_PARSER_RET_CODE_OK) {
                    maxSize = PDL_XML_MAX_TAG_LEN;
                    xmlParserGetValue(xmlLedColorId, &maxSize, &pdlTagStr[0]);
                    pdlStatus = prvPdlLibStrToEnumConvert (&pdlLedColorStrToEnum, pdlTagStr, (UINT_32*) &ledColor);
                    PDL_CHECK_STATUS(pdlStatus);
                    color = TRUE;
                }
                else if (xmlParserIsEqualName (xmlLedColorId, "led-state") == XML_PARSER_RET_CODE_OK) {
                    maxSize = PDL_XML_MAX_TAG_LEN;
                    xmlParserGetValue(xmlLedColorId, &maxSize, &pdlTagStr[0]);
                    pdlStatus = prvPdlLibStrToEnumConvert (&pdlLedStateStrToEnum, pdlTagStr, (UINT_32*) &ledState);
                    PDL_CHECK_STATUS(pdlStatus);
                    state = TRUE;
                }
                else if (xmlParserIsEqualName (xmlLedColorId, "led-i2c") == XML_PARSER_RET_CODE_OK) {
                    xmlStatus = xmlParserGetFirstChild (xmlLedColorId, &xmlI2cLedColorValueId);
                    if (xmlParserIsEqualName (xmlI2cLedColorValueId, "led-i2c-value") == XML_PARSER_RET_CODE_OK) {
                        if (state == FALSE || color == FALSE || ledState >= PDL_LED_STATE_LAST_E || ledColor >= PDL_LED_COLOR_LAST_E) {
                            return PDL_XML_PARSE_ERROR;
                        }
                        maxSize = sizeof (portLedInfo.values.i2cValues.value[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_INIT_E]);
                        xmlStatus = xmlParserGetValue(xmlI2cLedColorValueId, &maxSize, &portLedInfo.values.i2cValues.value[ledColor][ledState]);
                        XML_CHECK_STATUS(xmlStatus);
                    }
                    else {
                        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"incorrect tag inside led-i2c-color-list -> led-i2c");
                        return PDL_XML_PARSE_ERROR;
                    }
                }
                else {
                    PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"INVALID i2c-group led-mode XML tag");
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus2 = xmlParserGetNextSibling (xmlLedColorId, &xmlLedColorId);    
            }                
        }
        else {
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"INVALID i2c-group XML tag");
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling (xmlI2cChildId, &xmlI2cChildId);    
    }
    if (dev != PDL_LED_DONT_CARE_VALUE_CNS && logicalPort != PDL_LED_DONT_CARE_VALUE_CNS) {
        portLedKey.dev = dev;
        portLedKey.logicalPort = logicalPort;
        portLedKey.ledId = ledId;
        portLedInfo.capabilities.isSupported = TRUE;
        pdlStatus = prvPdlDbAdd (&pdlLedPortDb, (void*) &portLedKey, (void*) &portLedInfo, (void**) &portLedInfoPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }
    /* device led */
    else {
        pdlLedDeviceDb[ledId].interface.i2cInterface.ledInterfaceId = portLedInfo.interface.i2cInterface.ledInterfaceId;
        pdlLedDeviceDb[ledId].capabilities.isSupported = TRUE;
        pdlLedDeviceDb[ledId].capabilities.ledType = PDL_LED_TYPE_I2C_E;
        memcpy (&pdlLedDeviceDb[ledId].values.i2cValues, &portLedInfo.values.i2cValues, sizeof(pdlLedDeviceDb[ledId].values.i2cValues));
    }
    return PDL_OK;
}

/*$ END OF prvPdlLedXMLSingleI2cLedParser */

PDL_STATUS prvPdlLedXMLSmiXsmiValuesParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    OUT PDL_LED_SMI_XSMI_VALUE_STC        * smiValuesInfoPtr
)
{

/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                 maxSize;
    XML_PARSER_NODE_DESCRIPTOR_TYP          xmlLedValueId;
    XML_PARSER_RET_CODE_TYP                 xmlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (smiValuesInfoPtr == NULL) {
        return PDL_BAD_PTR;
    }
    memset (smiValuesInfoPtr, 0, sizeof(PDL_LED_SMI_XSMI_VALUE_STC));
    xmlStatus = xmlParserGetFirstChild (xmlId, &xmlLedValueId);
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);
        if (xmlParserIsEqualName (xmlLedValueId, "phy-device-or-page") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof (smiValuesInfoPtr->devOrPage);
            xmlStatus = xmlParserGetValue(xmlLedValueId, &maxSize, &smiValuesInfoPtr->devOrPage);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName (xmlLedValueId, "phy-register") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof (smiValuesInfoPtr->registerAddress);
            xmlStatus = xmlParserGetValue(xmlLedValueId, &maxSize, &smiValuesInfoPtr->registerAddress);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName (xmlLedValueId, "phy-mask") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof (smiValuesInfoPtr->mask);
            xmlStatus = xmlParserGetValue(xmlLedValueId, &maxSize, &smiValuesInfoPtr->mask);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName (xmlLedValueId, "phy-value") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof (smiValuesInfoPtr->value);
            xmlStatus = xmlParserGetValue(xmlLedValueId, &maxSize, &smiValuesInfoPtr->value);
            XML_CHECK_STATUS(xmlStatus);
        }
        else {
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling (xmlLedValueId, &xmlLedValueId);
    }
    return PDL_OK;
}

/**
 * @fn  PDL_STATUS prvPdlLedXMLSingleSmiXsmiLedParser ()
 *
 * @brief   Prv pdl SMI/XSMI LED XML parser
 *
 * @param   dev     dev if relevant. for system led use PDL_LED_DONT_CARE_VALUE_CNS
 * @param   port    port if relevant. for system led use PDL_LED_DONT_CARE_VALUE_CNS
 * @param   ledId   Identifier for the led being parsed
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLedXMLSingleSmiXsmiLedParser (
    IN  UINT_32                             dev,
    IN  UINT_32                             logicalPort,
    IN  PDL_LED_ID_ENT                      ledId,
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                    pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    UINT_32                                 maxSize;
    PDL_LED_COLOR_ENT                       ledColor = PDL_LED_COLOR_NONE_E;
    PDL_LED_STATE_ENT                       ledState;
    XML_PARSER_NODE_DESCRIPTOR_TYP          xmlLedId, xmlLedChildId, xmlLedValueId, xmlLedColorId, xmlLedSmiId;
    XML_PARSER_RET_CODE_TYP                 xmlStatus=XML_PARSER_RET_CODE_OK, xmlStatus2, xmlStatus3;
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                      portLedInfo, *portLedInfoPtr;
    PDL_LED_PRV_KEY_STC                     portLedKey;
    PDL_LED_SMI_XSMI_VALUE_STC              smiValuesInfo, *smiValueInfoPtr;
    BOOLEAN                                 state = FALSE, color = FALSE;    
    PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC  portInfo;
    PDL_LED_SMI_XSMI_KEY_STC                initValueKey;
    PDL_DB_PRV_ATTRIBUTES_STC               dbAttributes;
    PDL_INTERFACE_PRV_SMI_XSMI_DB_STC       smiInfo;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    /* no support for smi/xsmi system leds */
    if (dev == PDL_LED_DONT_CARE_VALUE_CNS || logicalPort == PDL_LED_DONT_CARE_VALUE_CNS) {
        return PDL_BAD_PARAM;
    }
    memset (&portLedInfo, 0, sizeof(portLedInfo));
    memset (&portLedKey, 0, sizeof(portLedKey));
    /* get dev&port xsmi/xsmi interface */
    pdlStatus = pdlPpDbPortAttributesGet (dev, logicalPort, &portInfo); 
    PDL_CHECK_STATUS(pdlStatus);
    smiInfo.publicInfo.address = (UINT_8) portInfo.phyData.smiXmsiInterface.address;
    smiInfo.publicInfo.dev = (UINT_8) portInfo.phyData.smiXmsiInterface.dev;
    smiInfo.publicInfo.interfaceId = (UINT_8) portInfo.phyData.smiXmsiInterface.interfaceId;
    smiInfo.publicInfo.interfaceType = portInfo.phyData.smiXmsiInterface.interfaceType;
    pdlStatus = prvPdlSmiXsmiAddInterface(&smiInfo, &portLedInfo.interface.smiInterface.ledInterfaceId);
    switch (portInfo.phyData.smiXmsiInterface.interfaceType) {
    case PDL_INTERFACE_TYPE_SMI_E:
        portLedInfo.capabilities.ledType = PDL_LED_TYPE_SMI_E;
        break;
    case PDL_INTERFACE_TYPE_XSMI_E:
        portLedInfo.capabilities.ledType = PDL_LED_TYPE_XSMI_E;
        break;
    default:
        return PDL_CREATE_ERROR;
    }
    portLedInfo.capabilities.isSupported = TRUE;
    /* initialize all led color interfaceId to invalid */
    for (ledColor = PDL_LED_COLOR_NONE_E; ledColor < PDL_LED_COLOR_LAST_E; ledColor++) {
        for (ledState = PDL_LED_STATE_OFF_E; ledState < PDL_LED_STATE_LAST_E; ledState++) {
            portLedInfo.values.smiValues.value[ledColor][ledState].value = PDL_LED_UNSUPPORTED_VALUE;
        }
    }
    dbAttributes.listAttributes.entrySize = sizeof(PDL_LED_SMI_XSMI_VALUE_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_LED_SMI_XSMI_KEY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &portLedInfo.values.smiValues.initValueList);
    initValueKey.valueNumber=0;
    PDL_CHECK_STATUS(pdlStatus);

    xmlLedId = xmlId;
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {        
        if (xmlParserIsEqualName (xmlLedId, "led-phy") == XML_PARSER_RET_CODE_OK) {
            xmlStatus2 = xmlParserGetFirstChild (xmlId, &xmlLedChildId);
            XML_CHECK_STATUS (xmlStatus2);
            while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {      
                if ((xmlParserIsEqualName (xmlLedChildId, "led-phy-off-exist") == XML_PARSER_RET_CODE_OK) ||
                    (xmlParserIsEqualName (xmlLedChildId, "led-phy-init-values-exist") == XML_PARSER_RET_CODE_OK)){
                    /* do nothing */
                }
                else if (xmlParserIsEqualName (xmlLedChildId, "led-phy-off-value") == XML_PARSER_RET_CODE_OK) {
                    ledState = PDL_LED_STATE_OFF_E;
                    xmlStatus3 = xmlParserGetFirstChild(xmlLedChildId, &xmlLedValueId);
                    XML_CHECK_STATUS(xmlStatus3);
                    pdlStatus = prvPdlLedXMLSmiXsmiValuesParser (xmlLedValueId, &smiValuesInfo);
                    PDL_CHECK_STATUS(pdlStatus);
                    memcpy (&portLedInfo.values.smiValues.value[PDL_LED_COLOR_NONE_E][ledState], &smiValuesInfo, sizeof (smiValuesInfo));
                }
                else if (xmlParserIsEqualName (xmlLedChildId, "led-phy-init-list") == XML_PARSER_RET_CODE_OK) {
                    /* init value is a list */
                    ledState = PDL_LED_STATE_INIT_E;
                    xmlStatus3 = xmlParserGetFirstChild(xmlLedChildId, &xmlLedValueId);
                    while (xmlStatus3 != XML_PARSER_RET_CODE_NOT_FOUND) {
                        XML_CHECK_STATUS(xmlStatus3);
                        if ((xmlParserIsEqualName (xmlLedValueId, "led-phy-init-number") == XML_PARSER_RET_CODE_OK) ||
                            (xmlParserIsEqualName (xmlLedValueId, "led-phy-init-comment") == XML_PARSER_RET_CODE_OK)) {
                            /* skip these tags*/
                        }
                        else if (xmlParserIsEqualName (xmlLedValueId, "phy-mask-value") == XML_PARSER_RET_CODE_OK) {
                            pdlStatus = prvPdlLedXMLSmiXsmiValuesParser (xmlLedValueId, &smiValuesInfo);
                            PDL_CHECK_STATUS(pdlStatus);
                        }
                        else {
                            return PDL_XML_PARSE_ERROR;
                        }
                        xmlStatus3 = xmlParserGetNextSibling (xmlLedValueId, &xmlLedValueId);
                    }
                    /* insert into init-values list */
                    pdlStatus = prvPdlDbAdd (&portLedInfo.values.smiValues.initValueList, (void*) &initValueKey, (void*) &smiValuesInfo, (void**) &smiValueInfoPtr);
                    PDL_CHECK_STATUS (pdlStatus);
                    initValueKey.valueNumber++;                        
                }
                else {
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus2 = xmlParserGetNextSibling (xmlLedChildId, &xmlLedChildId);
            }
        }
        else if (xmlParserIsEqualName (xmlLedId, "led-color-list") == XML_PARSER_RET_CODE_OK) {
            state = FALSE;
            color = FALSE;
            xmlStatus2 = xmlParserGetFirstChild (xmlLedId, &xmlLedColorId);
            while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                XML_CHECK_STATUS(xmlStatus2);
                if (xmlParserIsEqualName (xmlLedColorId, "led-color") == XML_PARSER_RET_CODE_OK) {
                    maxSize = PDL_XML_MAX_TAG_LEN;
                    xmlParserGetValue(xmlLedColorId, &maxSize, &pdlTagStr[0]);
                    pdlStatus = prvPdlLibStrToEnumConvert (&pdlLedColorStrToEnum, pdlTagStr, (UINT_32*) &ledColor);
                    PDL_CHECK_STATUS(pdlStatus);
                    color = TRUE;
                }
                else if (xmlParserIsEqualName (xmlLedColorId, "led-state") == XML_PARSER_RET_CODE_OK) {
                    maxSize = PDL_XML_MAX_TAG_LEN;
                    xmlParserGetValue(xmlLedColorId, &maxSize, &pdlTagStr[0]);
                    pdlStatus = prvPdlLibStrToEnumConvert (&pdlLedStateStrToEnum, pdlTagStr, (UINT_32*) &ledState);
                    PDL_CHECK_STATUS(pdlStatus);
                    state = TRUE;
                }
                else if (xmlParserIsEqualName (xmlLedColorId, "led-phy") == XML_PARSER_RET_CODE_OK) {
                    if (state == FALSE || color == FALSE || ledState >= PDL_LED_STATE_LAST_E || ledColor >= PDL_LED_COLOR_LAST_E) {
                        return PDL_XML_PARSE_ERROR;
                    }
                    xmlStatus3 = xmlParserGetFirstChild (xmlLedColorId, &xmlLedSmiId);
                    XML_CHECK_STATUS(xmlStatus3);
                    pdlStatus = prvPdlLedXMLSmiXsmiValuesParser (xmlLedSmiId, &smiValuesInfo);
                    PDL_CHECK_STATUS(pdlStatus);
                    memcpy (&portLedInfo.values.smiValues.value[ledColor][ledState], &smiValuesInfo, sizeof (smiValuesInfo));
                }
                else {
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus2 = xmlParserGetNextSibling (xmlLedColorId, &xmlLedColorId);
            }
        }
        else {
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"INVALID phy led XML tag");
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling (xmlLedId, &xmlLedId);
    }
    portLedKey.dev = dev;
    portLedKey.logicalPort = logicalPort;
    portLedKey.ledId = ledId;
    pdlStatus = prvPdlDbAdd (&pdlLedPortDb, (void*) &portLedKey, (void*) &portLedInfo, (void**) &portLedInfoPtr);
    return pdlStatus;
}

/*$ END OF prvPdlLedXMLSingleSmiXsmiLedParser */

/****************************************************************************
* FUNCTION NAME: prvPdlLedXMLLedStreamValuesParser
*
* DESCRIPTION:   
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS prvPdlLedXMLLedStreamValuesParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    IN  PDL_LED_STATE_ENT                   ledState,
    OUT PDL_LED_PRV_DB_STC                * portLedInfoPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    UINT_32                                 maxSize;
    XML_PARSER_RET_CODE_TYP                 xmlStatus, xmlStatus2, xmlStatus3;
    XML_PARSER_NODE_DESCRIPTOR_TYP          xmlLedFunctionCallId, xmlLedFunctionCallInfoId, xmlLedFunctionParamInfoId;
    PDL_STATUS                              pdlStatus;
    PDL_DB_PRV_ATTRIBUTES_STC               dbAttributes;
    PDL_LED_LEDSTREAM_FUNC_CALL_INFO_STC    funcInfo, *funcInfoPtr;
    PDL_LED_LEDSTREAM_FUNC_CALL_KEY_STC     funcKey;
    PDL_LED_LEDSTREAM_FUNC_PARAM_INFO_STC   paramInfo, *paramInfoPtr;
    PDL_LED_LEDSTREAM_FUNC_PARAM_KEY_STC    paramKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    dbAttributes.listAttributes.entrySize = sizeof(PDL_LED_LEDSTREAM_FUNC_CALL_INFO_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_LED_LEDSTREAM_FUNC_CALL_KEY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &portLedInfoPtr->values.ledStreamFuncCalls.functionCallDb[ledState]);
    PDL_CHECK_STATUS(pdlStatus);
    funcKey.functionCallNumber = 0;
    xmlStatus = xmlParserGetFirstChild (xmlId, &xmlLedFunctionCallId);
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        memset (&funcInfo, 0, sizeof(funcInfo));
        dbAttributes.listAttributes.entrySize = sizeof(PDL_LED_LEDSTREAM_FUNC_PARAM_INFO_STC);
        dbAttributes.listAttributes.keySize = sizeof(PDL_LED_LEDSTREAM_FUNC_PARAM_KEY_STC);
        pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                    &dbAttributes,
                                    &funcInfo.params);
        paramKey.keyNumber = 0;
        PDL_CHECK_STATUS(pdlStatus);
        if (xmlParserIsEqualName (xmlLedFunctionCallId, "function-call-list") == XML_PARSER_RET_CODE_OK)  {
            xmlStatus2 = xmlParserGetFirstChild (xmlLedFunctionCallId, &xmlLedFunctionCallInfoId);
            while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                if ((xmlParserIsEqualName (xmlLedFunctionCallInfoId, "function-call-number") == XML_PARSER_RET_CODE_OK) ||
                    (xmlParserIsEqualName (xmlLedFunctionCallInfoId, "function-call-comment") == XML_PARSER_RET_CODE_OK)) {
                    /* do nothing */
                }
                else if (xmlParserIsEqualName (xmlLedFunctionCallInfoId, "function-call-name") == XML_PARSER_RET_CODE_OK) {
                    maxSize = PDL_XML_MAX_TAG_LEN;
                    xmlStatus = xmlParserGetValue (xmlLedFunctionCallInfoId, &maxSize, &funcInfo.functionName);
                    XML_CHECK_STATUS (xmlStatus);
                }
                else if (xmlParserIsEqualName (xmlLedFunctionCallInfoId, "function-call-parameters-list") == XML_PARSER_RET_CODE_OK) {
                    xmlStatus3 = xmlParserGetFirstChild (xmlLedFunctionCallInfoId, &xmlLedFunctionParamInfoId);
                    while (xmlStatus3 != XML_PARSER_RET_CODE_NOT_FOUND) {
                        if (xmlParserIsEqualName (xmlLedFunctionParamInfoId, "function-call-parameter-name") == XML_PARSER_RET_CODE_OK) {
                            maxSize = PDL_XML_MAX_TAG_LEN;
                            xmlStatus = xmlParserGetValue (xmlLedFunctionParamInfoId, &maxSize, &paramInfo.paramName);
                            XML_CHECK_STATUS (xmlStatus);
                        }
                        else if (xmlParserIsEqualName (xmlLedFunctionParamInfoId, "function-call-parameter-value") == XML_PARSER_RET_CODE_OK) {
                            maxSize = sizeof(paramInfo.paramValue);
                            xmlStatus = xmlParserGetValue (xmlLedFunctionParamInfoId, &maxSize, &paramInfo.paramValue);
                            XML_CHECK_STATUS (xmlStatus);
                        }
                        else if (xmlParserIsEqualName (xmlLedFunctionParamInfoId, "function-call-parameter-comment") == XML_PARSER_RET_CODE_OK) {
                            /* do nothing*/
                        }
                        else {
                            return PDL_XML_PARSE_ERROR;
                        }
                        xmlStatus3 = xmlParserGetNextSibling (xmlLedFunctionParamInfoId, &xmlLedFunctionParamInfoId);
                    }
                    /* insert param info to funcinfo db */
                    pdlStatus = prvPdlDbAdd (&funcInfo.params, (void*) &paramKey, (void*) &paramInfo, (void**) &paramInfoPtr);
                    PDL_CHECK_STATUS (pdlStatus);
                    paramKey.keyNumber++;
                }
                else {
                    return PDL_XML_PARSE_ERROR;
                }

                xmlStatus2 = xmlParserGetNextSibling (xmlLedFunctionCallInfoId, &xmlLedFunctionCallInfoId);
            }
        }
        else {
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling (xmlLedFunctionCallId, &xmlLedFunctionCallId);
        pdlStatus = prvPdlDbAdd (&portLedInfoPtr->values.ledStreamFuncCalls.functionCallDb[ledState], (void*) &funcKey, (void*) &funcInfo, (void**) &funcInfoPtr);
        PDL_CHECK_STATUS (pdlStatus);
        funcKey.functionCallNumber++;
    }
    return PDL_OK;
}

/*$ END OF prvPdlLedXMLLedStreamValuesParser */

/**
 * @fn  PDL_STATUS prvPdlLedXMLSingleLedStreamLedParser ()
 *
 * @brief   Prv pdl LedStream LED XML parser
 *
 * @param   dev     dev if relevant. for system led use PDL_LED_DONT_CARE_VALUE_CNS
 * @param   port    port if relevant. for system led use PDL_LED_DONT_CARE_VALUE_CNS
 * @param   ledId   Identifier for the led being parsed
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLedXMLSingleLedStreamLedParser (
    IN  UINT_32                             dev,
    IN  UINT_32                             logicalPort,
    IN  PDL_LED_ID_ENT                      ledId,
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                    pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    BOOLEAN                                 state = FALSE;
    UINT_32                                 maxSize;
    PDL_LED_COLOR_ENT                       ledColor = PDL_LED_COLOR_NONE_E;
    PDL_LED_STATE_ENT                       ledState = PDL_LED_STATE_LAST_E;
    XML_PARSER_NODE_DESCRIPTOR_TYP          xmlLedId, xmlLedOperationChildId, xmlLedColorId;
    XML_PARSER_RET_CODE_TYP                 xmlStatus = XML_PARSER_RET_CODE_OK, xmlStatus2;
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                      portLedInfo, *portLedInfoPtr;
    PDL_LED_PRV_KEY_STC                     portLedKey;    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    /* no support for ledstream system leds */
    if (dev == PDL_LED_DONT_CARE_VALUE_CNS || logicalPort == PDL_LED_DONT_CARE_VALUE_CNS) {
        return PDL_BAD_PARAM;
    }
    memset (&portLedInfo, 0, sizeof(portLedInfo));
    memset (&portLedKey, 0, sizeof(portLedKey));
    portLedInfo.capabilities.ledType = PDL_LED_TYPE_LEDSTREAM_E;
    portLedInfo.capabilities.isSupported = TRUE;
    xmlLedId = xmlId;
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        if (xmlParserIsEqualName (xmlLedId, "led-pp") == XML_PARSER_RET_CODE_OK) {
            xmlStatus2 = xmlParserGetFirstChild (xmlId, &xmlLedOperationChildId);
            XML_CHECK_STATUS(xmlStatus2);
            while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {      
                if (xmlParserIsEqualName (xmlLedOperationChildId, "led-pp-off-value") == XML_PARSER_RET_CODE_OK) {
                    ledState = PDL_LED_STATE_OFF_E;
                    pdlStatus = prvPdlLedXMLLedStreamValuesParser (xmlLedOperationChildId, ledState, &portLedInfo);
                    PDL_CHECK_STATUS(pdlStatus);
                }
                else if (xmlParserIsEqualName (xmlLedOperationChildId, "led-pp-init-value") == XML_PARSER_RET_CODE_OK) {
                    ledState = PDL_LED_STATE_INIT_E;
                    pdlStatus = prvPdlLedXMLLedStreamValuesParser (xmlLedOperationChildId, ledState, &portLedInfo);
                    PDL_CHECK_STATUS(pdlStatus);
                }
                else {
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus2 = xmlParserGetNextSibling (xmlLedOperationChildId, &xmlLedOperationChildId);
            }
        }
        else if (xmlParserIsEqualName (xmlLedId, "led-color-list") == XML_PARSER_RET_CODE_OK) {
            xmlStatus2 = xmlParserGetFirstChild (xmlLedId, &xmlLedColorId);
            while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                XML_CHECK_STATUS(xmlStatus2);
                if (xmlParserIsEqualName (xmlLedColorId, "led-color") == XML_PARSER_RET_CODE_OK) {
                    maxSize = PDL_XML_MAX_TAG_LEN;
                    xmlParserGetValue(xmlLedColorId, &maxSize, &pdlTagStr[0]);
                    pdlStatus = prvPdlLibStrToEnumConvert (&pdlLedColorStrToEnum, pdlTagStr, (UINT_32*) &ledColor);
                    PDL_CHECK_STATUS(pdlStatus);
                    if (ledColor != PDL_LED_COLOR_GREEN_E) {
                        /* only green color supported for ledstream led */
                        return PDL_XML_PARSE_ERROR;
                    }
                }
                else if (xmlParserIsEqualName (xmlLedColorId, "led-state") == XML_PARSER_RET_CODE_OK) {
                    maxSize = PDL_XML_MAX_TAG_LEN;
                    xmlParserGetValue(xmlLedColorId, &maxSize, &pdlTagStr[0]);
                    pdlStatus = prvPdlLibStrToEnumConvert (&pdlLedStateStrToEnum, pdlTagStr, (UINT_32*) &ledState);
                    PDL_CHECK_STATUS(pdlStatus);
                    state = TRUE;
                }
                else if (xmlParserIsEqualName (xmlLedColorId, "led-pp") == XML_PARSER_RET_CODE_OK) {
                    if (state == FALSE || ledState >= PDL_LED_STATE_LAST_E) {
                        return PDL_XML_PARSE_ERROR;
                    }
                    pdlStatus = prvPdlLedXMLLedStreamValuesParser (xmlLedColorId, ledState, &portLedInfo);
                    PDL_CHECK_STATUS(pdlStatus);
                }
                else {
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus2 = xmlParserGetNextSibling (xmlLedColorId, &xmlLedColorId);
            }

        }
        else {
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling (xmlLedId, &xmlLedId);
    }
    portLedKey.dev = dev;
    portLedKey.logicalPort = logicalPort;
    portLedKey.ledId = ledId;
    pdlStatus = prvPdlDbAdd (&pdlLedPortDb, (void*) &portLedKey, (void*) &portLedInfo, (void**) &portLedInfoPtr);
    return pdlStatus;
}

/*$ END OF prvPdlLedXMLSingleLedStreamLedParser */


/**
 * @fn  PDL_STATUS prvPdlLedXMLSingleLedParser ( IN PDL_LED_ID_ENT ledId IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Prv pdl LED XML parser
 *
 * @param   ledId   Identifier for the led being parsed
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLedXMLSingleLedParser (
    IN  UINT_32                             dev,
    IN  UINT_32                             logicalPort,
    IN  PDL_LED_ID_ENT                      ledId,
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                    pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    UINT_32                                 maxSize;    
    PDL_STATUS                              pdlStatus;
    XML_PARSER_RET_CODE_TYP                 xmlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (xmlParserIsEqualName (xmlId, "led-interface-select") == XML_PARSER_RET_CODE_OK) {
        maxSize = PDL_XML_MAX_TAG_LEN;
        xmlStatus = xmlParserGetValue(xmlId, &maxSize, &pdlTagStr[0]);
        XML_CHECK_STATUS(xmlStatus);
        xmlStatus = xmlParserGetNextSibling (xmlId, &xmlId);
        XML_CHECK_STATUS(xmlStatus);
        if (strcmp (pdlTagStr, "led-interface-gpio") == 0) {
            pdlStatus = prvPdlLedXMLSingleGpioLedParser(dev, logicalPort, ledId, xmlId);
            PDL_CHECK_STATUS(pdlStatus);
        }
        else if (strcmp (pdlTagStr, "led-interface-i2c") == 0) {
            pdlStatus = prvPdlLedXMLSingleI2cLedParser(dev, logicalPort, ledId, xmlId);
            PDL_CHECK_STATUS(pdlStatus);
        }
        else if (strcmp (pdlTagStr, "led-interface-phy") == 0) {
            pdlStatus = prvPdlLedXMLSingleSmiXsmiLedParser(dev, logicalPort, ledId, xmlId);
            PDL_CHECK_STATUS(pdlStatus);
        }
        else if (strcmp (pdlTagStr, "led-interface-pp") == 0) {
            pdlStatus = prvPdlLedXMLSingleLedStreamLedParser(dev, logicalPort, ledId, xmlId);
            PDL_CHECK_STATUS(pdlStatus);
        }
        else if (strcmp (pdlTagStr, "led-interface-external-driver") == 0) {
            pdlStatus = prvPdlLedXMLSingleExternalDriverLedParser(dev, logicalPort, ledId, xmlId);
            PDL_CHECK_STATUS(pdlStatus);
        }
        else {
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"unrecognized led type");
            return PDL_XML_PARSE_ERROR;
        }
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"Led doesn't include the led-interface-select tag");
        return PDL_XML_PARSE_ERROR;
    }

    return PDL_OK;
}

/*$ END OF prvPdlLedXMLSingleLedParser */

/****************************************************************************
* FUNCTION NAME: prvPdlLedXMLLedStreamManipulationParser
*
* DESCRIPTION:   
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS prvPdlLedXMLLedStreamManipulationParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP        xmlId,
    OUT PDL_LED_CLASS_MANIPULATION_INFO_STC * manipulationInfoPtr,
    OUT PDL_LED_CLASS_MANIPULATION_KEY_STC  * manipulationKeyPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    char                                    pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    UINT_32                                 maxSize, value;
    XML_PARSER_RET_CODE_TYP                 xmlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP          xmlPortLedstreamManipulationId;
    PDL_STATUS                              pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (manipulationInfoPtr == NULL || manipulationKeyPtr == NULL) {
        return PDL_BAD_PTR;
    }
    memset (manipulationInfoPtr, 0, sizeof (PDL_LED_CLASS_MANIPULATION_INFO_STC));
    xmlStatus = xmlParserGetFirstChild (xmlId, &xmlPortLedstreamManipulationId);
    XML_CHECK_STATUS(xmlStatus);
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);
        if (xmlParserIsEqualName(xmlPortLedstreamManipulationId, "invertEnable") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof (manipulationInfoPtr->classManipulation.invertEnable);
            xmlStatus = xmlParserGetValue(xmlPortLedstreamManipulationId, &maxSize, &manipulationInfoPtr->classManipulation.invertEnable);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlPortLedstreamManipulationId, "manipulation-index") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof (manipulationKeyPtr->manipulationIndex);
            xmlStatus = xmlParserGetValue(xmlPortLedstreamManipulationId, &maxSize, &manipulationKeyPtr->manipulationIndex);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlPortLedstreamManipulationId, "blinkEnable") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof (manipulationInfoPtr->classManipulation.blinkEnable);
            xmlStatus = xmlParserGetValue(xmlPortLedstreamManipulationId, &maxSize, &manipulationInfoPtr->classManipulation.blinkEnable);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlPortLedstreamManipulationId, "blinkSelect") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(pdlTagStr);
            xmlStatus = xmlParserGetValue (xmlPortLedstreamManipulationId, &maxSize, &pdlTagStr[0]);
            XML_CHECK_STATUS(xmlStatus);
            pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_LED_STREAM_BLINK_SELECT_TYPE_E, pdlTagStr, &value);
            PDL_CHECK_STATUS(pdlStatus);
            manipulationInfoPtr->classManipulation.blinkSelect = (PDL_LED_BLINK_SELECT_ENT)value;
        }
        else if (xmlParserIsEqualName(xmlPortLedstreamManipulationId, "forceEnable") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof (manipulationInfoPtr->classManipulation.forceEnable);
            xmlStatus = xmlParserGetValue(xmlPortLedstreamManipulationId, &maxSize, &manipulationInfoPtr->classManipulation.forceEnable);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlPortLedstreamManipulationId, "forceData") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof (manipulationInfoPtr->classManipulation.forceData);
            xmlStatus = xmlParserGetValue(xmlPortLedstreamManipulationId, &maxSize, &manipulationInfoPtr->classManipulation.forceData);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlPortLedstreamManipulationId, "pulseStretchEnable") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof (manipulationInfoPtr->classManipulation.pulseStretchEnable);
            xmlStatus = xmlParserGetValue(xmlPortLedstreamManipulationId, &maxSize, &manipulationInfoPtr->classManipulation.pulseStretchEnable);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlPortLedstreamManipulationId, "disableOnLinkDown") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof (manipulationInfoPtr->classManipulation.disableOnLinkDown);
            xmlStatus = xmlParserGetValue(xmlPortLedstreamManipulationId, &maxSize, &manipulationInfoPtr->classManipulation.disableOnLinkDown);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlPortLedstreamManipulationId, "classNum") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof (manipulationInfoPtr->classNum);
            xmlStatus = xmlParserGetValue(xmlPortLedstreamManipulationId, &maxSize, &manipulationInfoPtr->classNum);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName (xmlPortLedstreamManipulationId, "portType") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(pdlTagStr);
            xmlStatus = xmlParserGetValue (xmlPortLedstreamManipulationId, &maxSize, &pdlTagStr[0]);
            XML_CHECK_STATUS(xmlStatus);
            pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_LED_STREAM_PORT_TYPE_E, pdlTagStr, &value);
            PDL_CHECK_STATUS(pdlStatus);
            manipulationInfoPtr->portType = (PDL_LED_PORT_TYPE_ENT)value;
        }
        else {
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"Ledstream manipulation unrecognized tag");
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling (xmlPortLedstreamManipulationId, &xmlPortLedstreamManipulationId);
    }
    return PDL_OK;
}

/*$ END OF prvPdlLedXMLLedStreamManipulationParser */

/****************************************************************************
* FUNCTION NAME: prvPdlLedXMLLedStreamManipulationParser
*
* DESCRIPTION:   
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS prvPdlLedXMLLedstreamInterfaceManipulationParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    OUT PDL_DB_PRV_STC                    * manipulationDbPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    XML_PARSER_RET_CODE_TYP                 xmlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP          xmlPortLedstreamManipulationListId;
    PDL_STATUS                              pdlStatus;
    PDL_DB_PRV_ATTRIBUTES_STC               dbAttributes;
    PDL_LED_CLASS_MANIPULATION_INFO_STC     manipulationInfo, *manipulationInfoPtr;
    PDL_LED_CLASS_MANIPULATION_KEY_STC      manipulationKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (manipulationDbPtr == NULL) {
        return PDL_BAD_PTR;
    }
    dbAttributes.listAttributes.entrySize = sizeof(PDL_LED_CLASS_MANIPULATION_INFO_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_LED_CLASS_MANIPULATION_KEY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                manipulationDbPtr);
    PDL_CHECK_STATUS(pdlStatus);
    xmlStatus = xmlParserGetFirstChild (xmlId, &xmlPortLedstreamManipulationListId);
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        if (xmlParserIsEqualName(xmlPortLedstreamManipulationListId, "manipulation-list") == XML_PARSER_RET_CODE_OK) {
            pdlStatus = prvPdlLedXMLLedStreamManipulationParser (xmlPortLedstreamManipulationListId, &manipulationInfo, &manipulationKey);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = prvPdlDbAdd (manipulationDbPtr, (void*) &manipulationKey, (void*) &manipulationInfo, (void**) &manipulationInfoPtr);
            PDL_CHECK_STATUS (pdlStatus);
        }
        else if ((xmlParserIsEqualName(xmlPortLedstreamManipulationListId, "led-color") == XML_PARSER_RET_CODE_OK) ||
                 (xmlParserIsEqualName(xmlPortLedstreamManipulationListId, "led-state") == XML_PARSER_RET_CODE_OK) ||
                 (xmlParserIsEqualName(xmlPortLedstreamManipulationListId, "ledstream-port-mode-index") == XML_PARSER_RET_CODE_OK) ||
                 (xmlParserIsEqualName(xmlPortLedstreamManipulationListId, "ledstream-port-mode-description") == XML_PARSER_RET_CODE_OK)) {
            /* Do nothing */
        }
        else {
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"Ledstream manipulation list unrecognized tag");
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling (xmlPortLedstreamManipulationListId, &xmlPortLedstreamManipulationListId);
    }
    return PDL_OK;
}

/*$ END OF prvPdlLedXMLLedstreamInterfaceManipulationParser */

/****************************************************************************
* FUNCTION NAME: prvPdlLedXMLLedstreamPortModesParser
*
* DESCRIPTION:   
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS prvPdlLedXMLLedstreamPortModesParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    OUT PDL_DB_PRV_STC                    * portModesDbPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    UINT_32                                 maxSize;
    XML_PARSER_RET_CODE_TYP                 xmlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP          xmlPortLedstreamPortModesListId, xmlPortLedstreamPortModesListChildId;
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_LEDSTREAM_PORT_MODE_KEY_STC portModekey;
    PDL_LED_PRV_LEDSTREAM_PORT_MODE_INFO_STC    portModeInfo, * portModeInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (portModesDbPtr == NULL) {
        return PDL_BAD_PTR;
    }

    xmlStatus = xmlParserGetFirstChild (xmlId, &xmlPortLedstreamPortModesListId);
    XML_CHECK_STATUS(xmlStatus);
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        if (xmlParserIsEqualName(xmlPortLedstreamPortModesListId, "ledstream-port-mode-list") == XML_PARSER_RET_CODE_OK) {
            xmlStatus = xmlParserGetFirstChild (xmlPortLedstreamPortModesListId, &xmlPortLedstreamPortModesListChildId);
            XML_CHECK_STATUS(xmlStatus);
            maxSize = sizeof(portModekey.index);
            xmlStatus = xmlParserGetValue (xmlPortLedstreamPortModesListChildId, &maxSize, &portModekey.index);
            XML_CHECK_STATUS(xmlStatus);
            xmlStatus = xmlParserGetNextSibling (xmlPortLedstreamPortModesListChildId, &xmlPortLedstreamPortModesListChildId);
            XML_CHECK_STATUS(xmlStatus);
            maxSize = sizeof(portModeInfo.portModeDescription);
            xmlStatus = xmlParserGetValue (xmlPortLedstreamPortModesListChildId, &maxSize, &portModeInfo.portModeDescription[0]);
            XML_CHECK_STATUS(xmlStatus);
            pdlStatus = prvPdlLedXMLLedstreamInterfaceManipulationParser (xmlPortLedstreamPortModesListId, &portModeInfo.manipulationList);
            PDL_CHECK_STATUS(pdlStatus);       
        }
        else {
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"Ledstream-port-mode-list unrecognized tag");
            return PDL_XML_PARSE_ERROR;
        }
        pdlStatus = prvPdlDbAdd (portModesDbPtr, (void*) &portModekey, (void*) &portModeInfo, (void**) &portModeInfoPtr);
        PDL_CHECK_STATUS(pdlStatus);
        xmlStatus = xmlParserGetNextSibling (xmlPortLedstreamPortModesListId, &xmlPortLedstreamPortModesListId);
    }

    return PDL_OK;
}

/*$ END OF prvPdlLedXMLLedstreamPortModesParser */

/****************************************************************************
* FUNCTION NAME: prvPdlLedXMLLedstreamInterfaceParser
*
* DESCRIPTION:   
*
* PARAMETERS:   
*                
*****************************************************************************/
PDL_STATUS prvPdlLedXMLLedstreamInterfaceParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                            pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    UINT_32                                         maxSize, value;    
    PDL_STATUS                                      pdlStatus;
    PDL_LED_COLOR_ENT                               ledColor = PDL_LED_COLOR_NONE_E;
    PDL_LED_STATE_ENT                               ledState;
    XML_PARSER_RET_CODE_TYP                         xmlStatus, xmlStatus2;
    PDL_DB_PRV_ATTRIBUTES_STC                       dbAttributes;
    XML_PARSER_NODE_DESCRIPTOR_TYP                  xmlPortLedstreamInterfaceId, xmlPortLedstreamInterfaceConfigId, xmlPortLedstreamInterfaceModesId, xmlPortLedstreamInterfaceColorListId, xmlPortLedstreamPortModeId;
    PDL_LED_PRV_LEDSTREAM_INTERFACE_KEY_STC         ledStreaminterfaceKey;
    PDL_LED_PRV_LEDSTREAM_INTERFACE_INFO_STC        ledStreaminterfaceInfo, * ledStreaminterfaceInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    memset (&ledStreaminterfaceKey, 0, sizeof(ledStreaminterfaceKey));
    memset (&ledStreaminterfaceInfo, 0, sizeof(ledStreaminterfaceInfo));
    dbAttributes.listAttributes.keySize = sizeof(PDL_LED_PRV_LEDSTREAM_PORT_MODE_KEY_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_LED_PRV_LEDSTREAM_PORT_MODE_INFO_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &ledStreaminterfaceInfo.leftPortModeManipulationDb);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &ledStreaminterfaceInfo.rightPortModeManipulationDb);
    PDL_CHECK_STATUS(pdlStatus);

    if (xmlParserIsEqualName (xmlId, "Ledstream-interface-list") == XML_PARSER_RET_CODE_OK) {
        xmlStatus = xmlParserGetFirstChild (xmlId, &xmlPortLedstreamInterfaceId);
        XML_CHECK_STATUS(xmlStatus);
        while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
            XML_CHECK_STATUS(xmlStatus);
            if (xmlParserIsEqualName(xmlPortLedstreamInterfaceId, "ledstream-interface-number") == XML_PARSER_RET_CODE_OK) {
                maxSize = sizeof (ledStreaminterfaceKey.interfaceNumber);
                xmlStatus = xmlParserGetValue(xmlPortLedstreamInterfaceId, &maxSize, &ledStreaminterfaceKey.interfaceNumber);
                XML_CHECK_STATUS(xmlStatus);
            }
            else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceId, "pp-device-number") == XML_PARSER_RET_CODE_OK) {
                maxSize = sizeof (ledStreaminterfaceKey.devNum);
                xmlStatus = xmlParserGetValue(xmlPortLedstreamInterfaceId, &maxSize, &ledStreaminterfaceKey.devNum);
                XML_CHECK_STATUS(xmlStatus);
            }
            else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceId, "ledstream-interface-configSet") == XML_PARSER_RET_CODE_OK) {
                xmlStatus2 = xmlParserGetFirstChild (xmlPortLedstreamInterfaceId, &xmlPortLedstreamInterfaceConfigId);
                XML_CHECK_STATUS(xmlStatus2);
                while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                    XML_CHECK_STATUS(xmlStatus2);
                    if (xmlParserIsEqualName(xmlPortLedstreamInterfaceConfigId, "ledOrganize") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(pdlTagStr);
                        xmlStatus = xmlParserGetValue (xmlPortLedstreamInterfaceConfigId, &maxSize, &pdlTagStr[0]);
                        XML_CHECK_STATUS(xmlStatus);
                        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_LED_STREAM_ORDER_MODE_TYPE_E, pdlTagStr, &value);
                        PDL_CHECK_STATUS(pdlStatus);
                        ledStreaminterfaceInfo.publicInfo.ledOrganize = (PDL_LED_ORDER_MODE_ENT)value;
                    }
                    else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceConfigId, "disableOnLinkDown") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(ledStreaminterfaceInfo.publicInfo.disableOnLinkDown);
                        xmlStatus = xmlParserGetValue(xmlPortLedstreamInterfaceConfigId, &maxSize, &ledStreaminterfaceInfo.publicInfo.disableOnLinkDown);
                        XML_CHECK_STATUS(xmlStatus);
                    }
                    else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceConfigId, "blink0DutyCycle") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(pdlTagStr);
                        xmlStatus = xmlParserGetValue (xmlPortLedstreamInterfaceConfigId, &maxSize, &pdlTagStr[0]);
                        XML_CHECK_STATUS(xmlStatus);
                        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_LED_STREAM_BLINK_DUTY_CYCLE_TYPE_E, pdlTagStr, &value);
                        PDL_CHECK_STATUS(pdlStatus);
                        ledStreaminterfaceInfo.publicInfo.blink0DutyCycle = (PDL_LED_BLINK_DUTY_CYCLE_ENT)value;
                    }
                    else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceConfigId, "blink0Duration") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(pdlTagStr);
                        xmlStatus = xmlParserGetValue (xmlPortLedstreamInterfaceConfigId, &maxSize, &pdlTagStr[0]);
                        XML_CHECK_STATUS(xmlStatus);
                        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_LED_STREAM_BLINK_DURATION_TYPE_E, pdlTagStr, &value);
                        PDL_CHECK_STATUS(pdlStatus);
                        ledStreaminterfaceInfo.publicInfo.blink0Duration = (PDL_LED_BLINK_DURATION_ENT)value;
                    }
                    else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceConfigId, "blink1DutyCycle") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(pdlTagStr);
                        xmlStatus = xmlParserGetValue (xmlPortLedstreamInterfaceConfigId, &maxSize, &pdlTagStr[0]);
                        XML_CHECK_STATUS(xmlStatus);
                        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_LED_STREAM_BLINK_DUTY_CYCLE_TYPE_E, pdlTagStr, &value);
                        PDL_CHECK_STATUS(pdlStatus);
                        ledStreaminterfaceInfo.publicInfo.blink1DutyCycle = (PDL_LED_BLINK_DUTY_CYCLE_ENT)value;
                    }
                    else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceConfigId, "blink1Duration") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(pdlTagStr);
                        xmlStatus = xmlParserGetValue (xmlPortLedstreamInterfaceConfigId, &maxSize, &pdlTagStr[0]);
                        XML_CHECK_STATUS(xmlStatus);
                        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_LED_STREAM_BLINK_DURATION_TYPE_E, pdlTagStr, &value);
                        PDL_CHECK_STATUS(pdlStatus);
                        ledStreaminterfaceInfo.publicInfo.blink1Duration = (PDL_LED_BLINK_DURATION_ENT)value;
                    }
                    else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceConfigId, "pulseStretch") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(pdlTagStr);
                        xmlStatus = xmlParserGetValue (xmlPortLedstreamInterfaceConfigId, &maxSize, &pdlTagStr[0]);
                        XML_CHECK_STATUS(xmlStatus);
                        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_LED_STREAM_PULSE_STRETCH_TYPE_E, pdlTagStr, &value);
                        PDL_CHECK_STATUS(pdlStatus);
                        ledStreaminterfaceInfo.publicInfo.pulseStretch = (PDL_LED_PULSE_STRETCH_ENT)value;
                    }
                    else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceConfigId, "ledStart") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(ledStreaminterfaceInfo.publicInfo.ledStart);
                        xmlStatus = xmlParserGetValue(xmlPortLedstreamInterfaceConfigId, &maxSize, &ledStreaminterfaceInfo.publicInfo.ledStart);
                        XML_CHECK_STATUS(xmlStatus);
                    }
                    else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceConfigId, "ledEnd") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(ledStreaminterfaceInfo.publicInfo.ledEnd);
                        xmlStatus = xmlParserGetValue(xmlPortLedstreamInterfaceConfigId, &maxSize, &ledStreaminterfaceInfo.publicInfo.ledEnd);
                        XML_CHECK_STATUS(xmlStatus);
                    }
                    else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceConfigId, "clkInvert") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(ledStreaminterfaceInfo.publicInfo.clkInvert);
                        xmlStatus = xmlParserGetValue(xmlPortLedstreamInterfaceConfigId, &maxSize, &ledStreaminterfaceInfo.publicInfo.clkInvert);
                        XML_CHECK_STATUS(xmlStatus);
                    }
                    else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceConfigId, "class5select") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(pdlTagStr);
                        xmlStatus = xmlParserGetValue (xmlPortLedstreamInterfaceConfigId, &maxSize, &pdlTagStr[0]);
                        XML_CHECK_STATUS(xmlStatus);
                        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_LED_STREAM_CLASS5_SELECT_TYPE_E, pdlTagStr, &value);
                        PDL_CHECK_STATUS(pdlStatus);
                        ledStreaminterfaceInfo.publicInfo.class5select = (PDL_LED_CLASS_5_SELECT_ENT)value;
                    }
                    else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceConfigId, "class13select") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(pdlTagStr);
                        xmlStatus = xmlParserGetValue (xmlPortLedstreamInterfaceConfigId, &maxSize, &pdlTagStr[0]);
                        XML_CHECK_STATUS(xmlStatus);
                        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_LED_STREAM_CLASS13_SELECT_TYPE_E, pdlTagStr, &value);
                        PDL_CHECK_STATUS(pdlStatus);
                        ledStreaminterfaceInfo.publicInfo.class13select = (PDL_LED_CLASS_13_SELECT_ENT)value;
                    }
                    else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceConfigId, "invertEnable") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(ledStreaminterfaceInfo.publicInfo.invertEnable);
                        xmlStatus = xmlParserGetValue(xmlPortLedstreamInterfaceConfigId, &maxSize, &ledStreaminterfaceInfo.publicInfo.invertEnable);
                        XML_CHECK_STATUS(xmlStatus);
                    }
                    else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceConfigId, "ledClockFrequency") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(pdlTagStr);
                        xmlStatus = xmlParserGetValue (xmlPortLedstreamInterfaceConfigId, &maxSize, &pdlTagStr[0]);
                        XML_CHECK_STATUS(xmlStatus);
                        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_LED_STREAM_CLOCK_OUT_FREQUENCY_TYPE_E, pdlTagStr, &value);
                        PDL_CHECK_STATUS(pdlStatus);
                        ledStreaminterfaceInfo.publicInfo.ledClockFrequency = (PDL_LED_CLOCK_OUT_FREQUENCY_ENT)value;
                    }
                    else {
                        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"unrecognized ledstream-interface-configSet tag");
                        return PDL_XML_PARSE_ERROR;
                    }
                    xmlStatus2 = xmlParserGetNextSibling (xmlPortLedstreamInterfaceConfigId, &xmlPortLedstreamInterfaceConfigId);
                }
            }
            else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceId, "ledstream-interface-init-exist") == XML_PARSER_RET_CODE_OK) {
                /* Do nothing */
            }
            else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceId, "ledstream-interface-init-value") == XML_PARSER_RET_CODE_OK) {
                pdlStatus = prvPdlLedXMLLedstreamInterfaceManipulationParser (xmlPortLedstreamInterfaceId, &ledStreaminterfaceInfo.manipulationList[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_INIT_E]);
                PDL_CHECK_STATUS(pdlStatus);       
            }
            else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceId, "ledstream-interface-modes") == XML_PARSER_RET_CODE_OK) {
                xmlStatus2 = xmlParserGetFirstChild (xmlPortLedstreamInterfaceId, &xmlPortLedstreamInterfaceModesId);
                XML_CHECK_STATUS(xmlStatus2);
                while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                    XML_CHECK_STATUS(xmlStatus2);
                    if (xmlParserIsEqualName(xmlPortLedstreamInterfaceModesId, "ledstream-interface-off-exist") == XML_PARSER_RET_CODE_OK) {
                        /* Do nothing */
                    }
                    else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceModesId, "ledstream-interface-off-value") == XML_PARSER_RET_CODE_OK) {
                        pdlStatus = prvPdlLedXMLLedstreamInterfaceManipulationParser (xmlPortLedstreamInterfaceModesId, &ledStreaminterfaceInfo.manipulationList[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_OFF_E]);
                        PDL_CHECK_STATUS(pdlStatus);       
                    }
                    else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceModesId, "interface-mode-list") == XML_PARSER_RET_CODE_OK) {
                        xmlStatus2 = xmlParserGetFirstChild (xmlPortLedstreamInterfaceModesId, &xmlPortLedstreamInterfaceColorListId);
                        XML_CHECK_STATUS(xmlStatus2);
                        maxSize = PDL_XML_MAX_TAG_LEN;
                        xmlParserGetValue(xmlPortLedstreamInterfaceColorListId, &maxSize, &pdlTagStr[0]);
                        pdlStatus = prvPdlLibStrToEnumConvert (&pdlLedColorStrToEnum, pdlTagStr, (UINT_32*) &ledColor);
                        PDL_CHECK_STATUS(pdlStatus);
                        xmlStatus2 = xmlParserGetNextSibling (xmlPortLedstreamInterfaceColorListId, &xmlPortLedstreamInterfaceColorListId);
                        xmlParserGetValue(xmlPortLedstreamInterfaceColorListId, &maxSize, &pdlTagStr[0]);
                        pdlStatus = prvPdlLibStrToEnumConvert (&pdlLedStateStrToEnum, pdlTagStr, (UINT_32*) &ledState);
                        PDL_CHECK_STATUS(pdlStatus);
                        pdlStatus = prvPdlLedXMLLedstreamInterfaceManipulationParser (xmlPortLedstreamInterfaceModesId, &ledStreaminterfaceInfo.manipulationList[ledColor][ledState]);
                        PDL_CHECK_STATUS(pdlStatus);       
                    }
                    else if (xmlParserIsEqualName(xmlPortLedstreamInterfaceModesId, "ledstream-port-modes") == XML_PARSER_RET_CODE_OK) {
                        xmlStatus2 = xmlParserGetFirstChild (xmlPortLedstreamInterfaceModesId, &xmlPortLedstreamPortModeId);
                        XML_CHECK_STATUS(xmlStatus2);
                        while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                            XML_CHECK_STATUS(xmlStatus2);
                            if ((xmlParserIsEqualName(xmlPortLedstreamPortModeId, "LeftLed-port-modes") == XML_PARSER_RET_CODE_OK) ||
                                (xmlParserIsEqualName(xmlPortLedstreamPortModeId, "SingleLed-operational-value") == XML_PARSER_RET_CODE_OK)) {
                                pdlStatus = prvPdlLedXMLLedstreamPortModesParser (xmlPortLedstreamPortModeId, &ledStreaminterfaceInfo.leftPortModeManipulationDb);
                                PDL_CHECK_STATUS(pdlStatus);       
                            }
                            else if (xmlParserIsEqualName(xmlPortLedstreamPortModeId, "RightLed-port-modes") == XML_PARSER_RET_CODE_OK) {
                                pdlStatus = prvPdlLedXMLLedstreamPortModesParser (xmlPortLedstreamPortModeId, &ledStreaminterfaceInfo.rightPortModeManipulationDb);
                                PDL_CHECK_STATUS(pdlStatus);       
                            }
                            else {
                                PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"unrecognized ledstream-port-modes tag");
                                return PDL_XML_PARSE_ERROR;
                            }            
                            xmlStatus2 = xmlParserGetNextSibling (xmlPortLedstreamPortModeId, &xmlPortLedstreamPortModeId);
                        }

                    }
                    else {
                        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"unrecognized ledstream-interface-modes tag");
                        return PDL_XML_PARSE_ERROR;
                    } 
                    xmlStatus2 = xmlParserGetNextSibling (xmlPortLedstreamInterfaceModesId, &xmlPortLedstreamInterfaceModesId);
                }
            }
            else {
                PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"unrecognized ledstream-interface-list tag");
                return PDL_XML_PARSE_ERROR;
            }            
            xmlStatus = xmlParserGetNextSibling (xmlPortLedstreamInterfaceId, &xmlPortLedstreamInterfaceId);
        }
        /* add entry to DB */
        pdlStatus = prvPdlDbAdd (&pdlLedStreamInterfaceDb, (void*) &ledStreaminterfaceKey, (void*) &ledStreaminterfaceInfo, (void**) &ledStreaminterfaceInfoPtr);
        PDL_CHECK_STATUS (pdlStatus);

    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"Ledstream interface doesn't include the ledstream-interface-number tag");
        return PDL_XML_PARSE_ERROR;
    }
    return PDL_OK;
}

/*$ END OF prvPdlLedXMLLedstreamInterfaceParser */

/****************************************************************************
* FUNCTION NAME: prvPdlLedXMLLedstreamPortParser
*
* DESCRIPTION:   
*
* PARAMETERS:   
*                
*****************************************************************************/
PDL_STATUS prvPdlLedXMLLedstreamPortParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                 maxSize, frontPanelNumber, portNumberInGroup, dev, logicalPort;    
    PDL_STATUS                              pdlStatus;
    XML_PARSER_RET_CODE_TYP                 xmlStatus, xmlStatus2;
    XML_PARSER_NODE_DESCRIPTOR_TYP          xmlPortLedstreamPortId, xmlPortLedstreamPortInitId;
    PDL_LED_PRV_DB_STC                      portLedInfo, *portLedInfoPtr;
    PDL_LED_PRV_KEY_STC                     portLedKey;    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    memset (&portLedInfo, 0, sizeof(portLedInfo));
    memset (&portLedKey, 0, sizeof(portLedKey));
    portLedInfo.capabilities.ledType = PDL_LED_TYPE_LEDSTREAM_LOGIC_BASED_E;
    portLedInfo.capabilities.isSupported = TRUE;
    if (xmlParserIsEqualName (xmlId, "LedstreamPortLeds-list") == XML_PARSER_RET_CODE_OK) {
        xmlStatus = xmlParserGetFirstChild (xmlId, &xmlPortLedstreamPortId);
        XML_CHECK_STATUS(xmlStatus);
        while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
            XML_CHECK_STATUS(xmlStatus);
            if (xmlParserIsEqualName (xmlPortLedstreamPortId, "panel-group-number") == XML_PARSER_RET_CODE_OK) {
                maxSize = sizeof(frontPanelNumber);
                xmlStatus = xmlParserGetValue (xmlPortLedstreamPortId, &maxSize, &frontPanelNumber);
                XML_CHECK_STATUS(xmlStatus);
            }
            else if (xmlParserIsEqualName (xmlPortLedstreamPortId, "panel-port-number") == XML_PARSER_RET_CODE_OK) {
                maxSize = sizeof(portNumberInGroup);
                xmlStatus = xmlParserGetValue (xmlPortLedstreamPortId, &maxSize, &portNumberInGroup);
                XML_CHECK_STATUS(xmlStatus);
            }
            else if (xmlParserIsEqualName(xmlPortLedstreamPortId, "Ledstream-port-init") == XML_PARSER_RET_CODE_OK) {
                xmlStatus2 = xmlParserGetFirstChild (xmlPortLedstreamPortId, &xmlPortLedstreamPortInitId);
                XML_CHECK_STATUS(xmlStatus2);
                while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                    XML_CHECK_STATUS(xmlStatus2);
                    if (xmlParserIsEqualName (xmlPortLedstreamPortInitId, "ledInterface") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof (portLedInfo.values.ledStreamLogic.ledInterface);
                        xmlStatus2 = xmlParserGetValue(xmlPortLedstreamPortInitId, &maxSize, &portLedInfo.values.ledStreamLogic.ledInterface);
                        XML_CHECK_STATUS(xmlStatus2);
                    }
                    else if (xmlParserIsEqualName (xmlPortLedstreamPortInitId, "ledPosition") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof (portLedInfo.values.ledStreamLogic.ledPosition);
                        xmlStatus2 = xmlParserGetValue(xmlPortLedstreamPortInitId, &maxSize, &portLedInfo.values.ledStreamLogic.ledPosition);
                        XML_CHECK_STATUS(xmlStatus2);
                    }
                    else if (xmlParserIsEqualName (xmlPortLedstreamPortInitId, "polarityEnable") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof (portLedInfo.values.ledStreamLogic.polarityEnable);
                        xmlStatus2 = xmlParserGetValue(xmlPortLedstreamPortInitId, &maxSize, &portLedInfo.values.ledStreamLogic.polarityEnable);
                        XML_CHECK_STATUS(xmlStatus2);
                    }
                    else if (xmlParserIsEqualName (xmlPortLedstreamPortInitId, "classNum") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof (portLedInfo.values.ledStreamLogic.classNum);
                        xmlStatus2 = xmlParserGetValue(xmlPortLedstreamPortInitId, &maxSize, &portLedInfo.values.ledStreamLogic.classNum);
                        XML_CHECK_STATUS(xmlStatus2);
                    }
                    else {
                        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"unrecognized Ledstream-port-init tag");
                        return PDL_XML_PARSE_ERROR;
                    }            
                    xmlStatus2 = xmlParserGetNextSibling (xmlPortLedstreamPortInitId, &xmlPortLedstreamPortInitId);
                }
            }
            else {
                PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"unrecognized LedstreamPortLeds-list tag");
                return PDL_XML_PARSE_ERROR;
            }            
            xmlStatus = xmlParserGetNextSibling (xmlPortLedstreamPortId, &xmlPortLedstreamPortId);
        }
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"Ledstream port doesn't include the LedstreamPortLeds-list tag");
        return PDL_XML_PARSE_ERROR;
    }
    pdlStatus = pdlPpPortConvertFrontPanel2DevPort (frontPanelNumber, portNumberInGroup, &dev, &logicalPort);
    PDL_CHECK_STATUS(pdlStatus);
    portLedKey.dev = dev;
    portLedKey.logicalPort = logicalPort;
    portLedKey.ledId = PDL_LED_TYPE_PORT_INBAND_PORT_LEFT_E;
    pdlStatus = prvPdlDbAdd (&pdlLedPortDb, (void*) &portLedKey, (void*) &portLedInfo, (void**) &portLedInfoPtr);
    return pdlStatus;
}

/*$ END OF prvPdlLedXMLLedstreamPortParser */


/* ***************************************************************************
* FUNCTION NAME: prvPdlLedXMLParser
*
* DESCRIPTION:   Fill all led related information from XML to DB
*
* PARAMETERS:   
*                
* XML structure:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlLedXMLParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Prv pdl LED XML parser
 *
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLedXMLParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                             pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    UINT_8                           stackUnitId;
    UINT_32                          maxSize, frontPanelNumber = XML_PARSER_NOT_INITIALIZED_CNS, portNumberInGroup = XML_PARSER_NOT_INITIALIZED_CNS, dev, logicalPort;
    PDL_LED_ID_ENT                   ledId;
    XML_PARSER_NODE_DESCRIPTOR_TYP   xmlSensorId, xmlLedInstanceId, xmlLedInfoId, xmlStackLedId, xmlStackLedInstanceId, xmlPortLedsId, xmlPortLedsChildId, xmlPortLedLeftId;
    XML_PARSER_NODE_DESCRIPTOR_TYP   xmlLedStreamId, xmlLedStreamPortId;
    XML_PARSER_RET_CODE_TYP          xmlStatus, xmlStatus2, xmlStatus3;
    PDL_STATUS                       pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    xmlStatus = xmlParserFindByName (xmlId, "LEDs", &xmlSensorId);
    /* section is not mandatory */
    if (xmlStatus == XML_PARSER_RET_CODE_NOT_FOUND) {
        return PDL_OK;
    }
    XML_CHECK_STATUS(xmlStatus);
    xmlStatus = xmlParserGetFirstChild (xmlSensorId, &xmlLedInstanceId);
    /* go over all leds */
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);
        maxSize = PDL_XML_MAX_TAG_LEN;
        xmlParserGetName(xmlLedInstanceId, &maxSize, &pdlTagStr[0]);
        if (xmlParserIsEqualName (xmlLedInstanceId, "StackLeds") == XML_PARSER_RET_CODE_OK) {
            xmlStatus = xmlParserGetFirstChild (xmlLedInstanceId, &xmlStackLedId);
            while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
                if (xmlParserIsEqualName (xmlStackLedId, "stack-led-master") == XML_PARSER_RET_CODE_OK) {
                    xmlStatus = xmlParserGetFirstChild (xmlStackLedId, &xmlLedInfoId);
                    XML_CHECK_STATUS(xmlStatus);
                    pdlStatus = prvPdlLedXMLSingleLedParser (PDL_LED_DONT_CARE_VALUE_CNS, PDL_LED_DONT_CARE_VALUE_CNS, PDL_LED_TYPE_DEVICE_STACK_MASTER_E, xmlLedInfoId);
                    PDL_CHECK_STATUS(pdlStatus);      
                }
                else if (xmlParserIsEqualName (xmlStackLedId, "stackLed-list") == XML_PARSER_RET_CODE_OK) {
                    xmlStatus = xmlParserGetFirstChild (xmlStackLedId, &xmlStackLedInstanceId);
                    XML_CHECK_STATUS(xmlStatus);
                    maxSize = sizeof(stackUnitId);
                    xmlParserGetValue(xmlStackLedInstanceId, &maxSize, &stackUnitId);
                    pdlStatus = prvPdlStackNumToLedId (stackUnitId, &ledId);
                    PDL_CHECK_STATUS(pdlStatus);   
                    xmlStatus = xmlParserGetNextSibling (xmlStackLedInstanceId, &xmlStackLedInstanceId);
                    XML_CHECK_STATUS(xmlStatus);
                    xmlStatus = xmlParserGetFirstChild (xmlStackLedInstanceId, &xmlStackLedInstanceId);
                    XML_CHECK_STATUS(xmlStatus);
                    pdlStatus = prvPdlLedXMLSingleLedParser (PDL_LED_DONT_CARE_VALUE_CNS, PDL_LED_DONT_CARE_VALUE_CNS, ledId, xmlStackLedInstanceId);
                    PDL_CHECK_STATUS(pdlStatus);       
                }
                xmlStatus = xmlParserGetNextSibling (xmlStackLedId, &xmlStackLedId); 
            }            
        }
        else if (xmlParserIsEqualName (xmlLedInstanceId, "PortsLeds") == XML_PARSER_RET_CODE_OK) {
            xmlStatus = xmlParserGetFirstChild (xmlLedInstanceId, &xmlPortLedsId);
            while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
                XML_CHECK_STATUS(xmlStatus);
                if (xmlParserIsEqualName (xmlPortLedsId, "PortLeds-list") == XML_PARSER_RET_CODE_OK) {
                    xmlStatus2 = xmlParserGetFirstChild (xmlPortLedsId, &xmlPortLedsChildId);
                    while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                        XML_CHECK_STATUS(xmlStatus2);
                        if ((xmlParserIsEqualName (xmlPortLedsChildId, "LeftLed") == XML_PARSER_RET_CODE_OK) ||
                           (xmlParserIsEqualName (xmlPortLedsChildId, "RightLed") == XML_PARSER_RET_CODE_OK) ||
                           (xmlParserIsEqualName (xmlPortLedsChildId, "SingleLed") == XML_PARSER_RET_CODE_OK)) {
                               if ((xmlParserIsEqualName (xmlPortLedsChildId, "LeftLed") == XML_PARSER_RET_CODE_OK) ||
                                   (xmlParserIsEqualName (xmlPortLedsChildId, "SingleLed") == XML_PARSER_RET_CODE_OK)) {
                                   ledId = PDL_LED_TYPE_PORT_INBAND_PORT_LEFT_E;
                               }
                               else {
                                   ledId = PDL_LED_TYPE_PORT_INBAND_PORT_RIGHT_E;
                               }
                               if (frontPanelNumber == XML_PARSER_NOT_INITIALIZED_CNS || portNumberInGroup == XML_PARSER_NOT_INITIALIZED_CNS) {
                            	   PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"INVALID frontPanelNumber/portNumberInGroup (not set)");
                            	   return PDL_XML_PARSE_ERROR;
                               }
                                xmlStatus3 = xmlParserGetFirstChild (xmlPortLedsChildId, &xmlPortLedLeftId);
                                XML_CHECK_STATUS(xmlStatus3);
                                pdlStatus = pdlPpPortConvertFrontPanel2DevPort (frontPanelNumber, portNumberInGroup, &dev, &logicalPort);
                                PDL_CHECK_STATUS(pdlStatus);
                                pdlStatus = prvPdlLedXMLSingleLedParser (dev, logicalPort, ledId, xmlPortLedLeftId);
                                PDL_CHECK_STATUS(pdlStatus);
                        }
                        else if (xmlParserIsEqualName (xmlPortLedsChildId, "panel-group-number") == XML_PARSER_RET_CODE_OK) {
                            maxSize = sizeof(frontPanelNumber);
                            xmlStatus3 = xmlParserGetValue (xmlPortLedsChildId, &maxSize, &frontPanelNumber);
                            XML_CHECK_STATUS(xmlStatus3);
                        }
                        else if (xmlParserIsEqualName (xmlPortLedsChildId, "panel-port-number") == XML_PARSER_RET_CODE_OK) {
                            maxSize = sizeof(portNumberInGroup);
                            xmlStatus3 = xmlParserGetValue (xmlPortLedsChildId, &maxSize, &portNumberInGroup);
                            XML_CHECK_STATUS(xmlStatus3);                           
                        }
                        else {
                            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"INVALID port leds XML tag");
                            return PDL_XML_PARSE_ERROR;
                        }
                        xmlStatus2 = xmlParserGetNextSibling (xmlPortLedsChildId, &xmlPortLedsChildId); 
                    }
                    
                }
                else if (xmlParserIsEqualName (xmlPortLedsId, "PortLeds-supported-type") == XML_PARSER_RET_CODE_OK) {
                    /* ignore port-led-supported-type tag */
                }
                else {
                    PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"INVALID ports leds XML tag");
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus = xmlParserGetNextSibling (xmlPortLedsId, &xmlPortLedsId);
            }
        }
        else if (xmlParserIsEqualName (xmlLedInstanceId, "LedstreamLeds") == XML_PARSER_RET_CODE_OK) {
            xmlStatus = xmlParserGetFirstChild (xmlLedInstanceId, &xmlLedStreamId);
            while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
                XML_CHECK_STATUS(xmlStatus);
                if (xmlParserIsEqualName (xmlLedStreamId, "Ledstream-interface-list") == XML_PARSER_RET_CODE_OK) {
                    pdlStatus = prvPdlLedXMLLedstreamInterfaceParser (xmlLedStreamId);
                    PDL_CHECK_STATUS(pdlStatus);       
                }
                else if (xmlParserIsEqualName (xmlLedStreamId, "LedstreamPortsLeds") == XML_PARSER_RET_CODE_OK) {
                    xmlStatus2 = xmlParserGetFirstChild (xmlLedStreamId, &xmlLedStreamPortId);
                    while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                        XML_CHECK_STATUS(xmlStatus2);
                        pdlStatus = prvPdlLedXMLLedstreamPortParser (xmlLedStreamPortId);
                        PDL_CHECK_STATUS(pdlStatus);       
                        xmlStatus2 = xmlParserGetNextSibling (xmlLedStreamPortId, &xmlLedStreamPortId);
                    }
                }
                else {
                    PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__,"INVALID LedstreamLeds XML tag");
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus = xmlParserGetNextSibling (xmlLedStreamId, &xmlLedStreamId);
            }
        }
        else if (strstr (pdlTagStr, "is-supported") != 0) {
            /* ignore is-supported tag */
        }
        else {
            maxSize = PDL_XML_MAX_TAG_LEN;
            xmlParserGetName(xmlLedInstanceId, &maxSize, &pdlTagStr[0]);
            pdlStatus = prvPdlLibStrToEnumConvert (&pdlLedXMLStrToEnum, pdlTagStr, (UINT_32*) &ledId);
            PDL_CHECK_STATUS(pdlStatus);
            xmlStatus = xmlParserGetFirstChild (xmlLedInstanceId, &xmlLedInfoId);
            XML_CHECK_STATUS(xmlStatus);
            pdlStatus = prvPdlLedXMLSingleLedParser (PDL_LED_DONT_CARE_VALUE_CNS, PDL_LED_DONT_CARE_VALUE_CNS, ledId, xmlLedInfoId);
            PDL_CHECK_STATUS(pdlStatus);       
        }
        xmlStatus = xmlParserGetNextSibling (xmlLedInstanceId, &xmlLedInstanceId); 
    }
    return PDL_OK;
}

/*$ END OF prvPdlLedXMLParser */

/* ***************************************************************************
* FUNCTION NAME: pdlLedTypeGet
*
* DESCRIPTION:   get LED type (i2c/gpio/smi/xsmi)
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS pdlLedTypeGet (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    OUT PDL_LED_TYPE_ENT      * ledTypePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/   
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    PDL_LED_PRV_KEY_STC                     ledKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (ledTypePtr == NULL) {
        return PDL_BAD_PTR;
    } 

    /* device led */
    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        ledPtr = &pdlLedDeviceDb[ledId];
    }
    /* port led */
    else if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E){
            ledKey.dev = dev;
            ledKey.logicalPort = logicalPort;
            ledKey.ledId = ledId;
            pdlStatus = prvPdlDbFind (&pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
            PDL_CHECK_STATUS (pdlStatus);
    }
    else {
        return PDL_BAD_PARAM;
    }
    *ledTypePtr = ledPtr->capabilities.ledType;
    return PDL_OK;
}

/*$ END OF pdlLedTypeGet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedStreamFunctionNumGet
*
* DESCRIPTION:   get number of function calls for given ledstream
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS pdlLedStreamFunctionNumGet (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    OUT UINT_32               * functionNumPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/   
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    PDL_LED_PRV_KEY_STC                     ledKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (functionNumPtr == NULL) {
        return PDL_BAD_PTR;
    } 

    if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E){
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlDbFind (&pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS (pdlStatus);
    }
    else {
        return PDL_BAD_PARAM;
    }
    pdlStatus = prvPdlDbGetNumOfEntries (&ledPtr->values.ledStreamFuncCalls.functionCallDb[state], functionNumPtr);
    PDL_CHECK_STATUS (pdlStatus);
    return PDL_OK;  
}

/*$ END OF pdlLedStreamFunctionGet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedStreamFunctionGet
*
* DESCRIPTION:   get function call string for ledstream led
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS pdlLedStreamFunctionGet (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  UINT_32                 functionId,
    IN  UINT_32                 functionNameSize,
    OUT char                  * functionNamePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/   
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    PDL_LED_PRV_KEY_STC                     ledKey;
    PDL_LED_LEDSTREAM_FUNC_CALL_INFO_STC  * funcInfoPtr;
    PDL_LED_LEDSTREAM_FUNC_CALL_KEY_STC     funcKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (functionNamePtr == NULL) {
        return PDL_BAD_PTR;
    } 

    if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E){
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlDbFind (&pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS (pdlStatus);
    }
    else {
        return PDL_BAD_PARAM;
    }
    funcKey.functionCallNumber = functionId;
    pdlStatus = prvPdlDbFind (&ledPtr->values.ledStreamFuncCalls.functionCallDb[state], &funcKey, (void*) &funcInfoPtr);
    PDL_CHECK_STATUS (pdlStatus);
    if (functionNameSize < strlen(funcInfoPtr->functionName)+1) {
        return PDL_NO_RESOURCE;
    }
    memcpy (functionNamePtr, funcInfoPtr->functionName, strlen(funcInfoPtr->functionName)+1);
    return PDL_OK;  
}

/*$ END OF pdlLedStreamFunctionGet */


/* ***************************************************************************
* FUNCTION NAME: pdlLedStreamFunctionParamNumGet
*
* DESCRIPTION:   get number of parameters for given function
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS pdlLedStreamFunctionParamNumGet (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  UINT_32                 functionId,
    OUT UINT_32               * paramNumPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/   
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    PDL_LED_PRV_KEY_STC                     ledKey;
    PDL_LED_LEDSTREAM_FUNC_CALL_KEY_STC     funcKey;
    PDL_LED_LEDSTREAM_FUNC_CALL_INFO_STC  * funcInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (paramNumPtr == NULL) {
        return PDL_BAD_PTR;
    } 

    if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E){
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlDbFind (&pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS (pdlStatus);
    }
    else {
        return PDL_BAD_PARAM;
    }
    funcKey.functionCallNumber = functionId;
    pdlStatus = prvPdlDbFind (&ledPtr->values.ledStreamFuncCalls.functionCallDb[state], &funcKey, (void*) &funcInfoPtr);
    PDL_CHECK_STATUS (pdlStatus);
    pdlStatus = prvPdlDbGetNumOfEntries (&funcInfoPtr->params, paramNumPtr);
    PDL_CHECK_STATUS (pdlStatus);
    return PDL_OK;  
}

/*$ END OF pdlLedStreamFunctionParamNumGet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedStreamFunctionParamGet
*
* DESCRIPTION:   get parameter string & value for function
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS pdlLedStreamFunctionParamGet (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  UINT_32                 functionId,
    IN  UINT_32                 paramId,
    IN  UINT_32                 paramNameSize,
    OUT char                  * paramNamePtr,
    OUT UINT_32               * paramValuePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/   
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    PDL_LED_PRV_KEY_STC                     ledKey;
    PDL_LED_LEDSTREAM_FUNC_CALL_KEY_STC     funcKey;
    PDL_LED_LEDSTREAM_FUNC_CALL_INFO_STC  * funcInfoPtr;
    PDL_LED_LEDSTREAM_FUNC_PARAM_INFO_STC * paramInfoPtr;
    PDL_LED_LEDSTREAM_FUNC_PARAM_KEY_STC    paramKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (paramNamePtr == NULL || paramValuePtr == NULL) {
        return PDL_BAD_PTR;
    } 

    if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E){
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlDbFind (&pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS (pdlStatus);
    }
    else {
        return PDL_BAD_PARAM;
    }
    funcKey.functionCallNumber = functionId;
    pdlStatus = prvPdlDbFind (&ledPtr->values.ledStreamFuncCalls.functionCallDb[state], &funcKey, (void*) &funcInfoPtr);
    PDL_CHECK_STATUS (pdlStatus);

    paramKey.keyNumber = paramId;
    pdlStatus = prvPdlDbFind (&funcInfoPtr->params, &paramKey, (void*) &paramInfoPtr);
    PDL_CHECK_STATUS (pdlStatus);

    if (paramNameSize < strlen(paramInfoPtr->paramName)+1) {
        return PDL_NO_RESOURCE;
    }
    memcpy (paramNamePtr, paramInfoPtr->paramName, strlen(paramInfoPtr->paramName)+1);
    *paramValuePtr = paramInfoPtr->paramValue;
    return PDL_OK;  
}

/*$ END OF pdlLedStreamFunctionParamGet */


/**
 * @fn  PDL_STATUS pdlLedDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Led debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlLedDebugSet (
    IN  BOOLEAN             state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlLedDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlLedDebugSet */

/* ***************************************************************************
* FUNCTION NAME: prvPdlLedBindDriver
*
* DESCRIPTION:   bind driver with callback set & id
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS prvPdlLedBindDriver(
	IN PDL_LED_CALLBACK_INFO_STC            * callbacksInfoPtr,
	IN PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP   externalDriverId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    PDL_STATUS                                    pdlStatus;
    PDL_DB_PRV_ATTRIBUTES_STC                     dbAttributes;
    PDL_LED_PRV_CALLBACK_STC                      callbacksInfo, * callbackInfoPtr = NULL;                 
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    /*create callbacks db if needed*/
    if(pdlLedCallbacksDb.dbPtr == NULL){
        dbAttributes.listAttributes.entrySize = sizeof(PDL_LED_PRV_CALLBACK_STC);
        dbAttributes.listAttributes.keySize = sizeof(PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP);
        pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,  &dbAttributes,  &pdlLedCallbacksDb);
        PDL_CHECK_STATUS(pdlStatus);
    }

    /* verify entry with this externalDriverId doesn't exist already */
    pdlStatus = prvPdlDbFind (&pdlLedCallbacksDb, (void*) &(externalDriverId), (void**) &callbackInfoPtr);
    if (pdlStatus == PDL_OK) {
        return PDL_ALREADY_EXIST;
    }

    callbacksInfo.externalDriverId = externalDriverId;
    memcpy (&callbacksInfo.callbackInfo, callbacksInfoPtr, sizeof (PDL_LED_CALLBACK_INFO_STC));
    /*add to db*/ 
    pdlStatus = prvPdlDbAdd (&pdlLedCallbacksDb, (void*) &externalDriverId, (void*) &callbacksInfo, (void**) &callbackInfoPtr); 
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}

/*$ END OF prvPdlLedBindDriver */

/* ***************************************************************************
* FUNCTION NAME: prvPdlLedBindInternalDriver
*
* DESCRIPTION:   bind internal driver with callback set & id
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS prvPdlLedBindInternalDriver(
	IN PDL_LED_PRV_CALLBACK_INFO_STC         * callbacksInfoPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    PDL_LED_CALLBACK_INFO_STC                  callbacksInfo;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    memset (&callbacksInfo, 0, sizeof(callbacksInfo));
    callbacksInfo.ledHwDeviceModeSetCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.ledHwDeviceModeSetCallbackInfo.ledHwDeviceModeSetFun = callbacksInfoPtr->ledHwDeviceModeSetFun;
    callbacksInfo.ledHwPortModeSetCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.ledHwPortModeSetCallbackInfo.ledHwPortModeSetFun = callbacksInfoPtr->ledHwPortModeSetFun;
    callbacksInfo.ledInitDbCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.ledInitDbCallbackInfo.initFun = callbacksInfoPtr->ledInitDbFun;
    callbacksInfo.ledInitHwCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.ledInitHwCallbackInfo.initFun = callbacksInfoPtr->ledInitHwFun;
    return prvPdlLedBindDriver (&callbacksInfo, PDL_INTERFACE_INTERNAL_DRIVER_ID_BOARD);
}

/*$ END OF prvPdlLedBindInternalDriver */

/* ***************************************************************************
* FUNCTION NAME: pdlLedBindExternalDriver
*
* DESCRIPTION:   bind external driver with callback set & id
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS pdlLedBindExternalDriver(
	IN PDL_LED_CALLBACK_INFO_STC            * callbacksInfoPtr,
	IN PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP   externalDriverId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
             
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (externalDriverId >= PDL_INTERFACE_INTERNAL_DRIVER_ID_MIN) {
        PDL_LIB_DEBUG_MAC(prvPdlSensorDebugFlag)(__FUNCTION__, __LINE__, "ids from [%d] are reserved for internal usage",PDL_INTERFACE_INTERNAL_DRIVER_ID_MIN);
        return PDL_OUT_OF_RANGE;
    }
    return prvPdlLedBindDriver (callbacksInfoPtr, externalDriverId);
}

/*$ END OF pdlLedBindExternalDriver */

/* ***************************************************************************
* FUNCTION NAME: prvPdlLedVerifyExternalDrivers
*
* DESCRIPTION:   verify all external drivers have been binded
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS prvPdlLedVerifyExternalDrivers(
	IN void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                       ledId;
    PDL_STATUS                                    pdlStatus;
    PDL_LED_PRV_KEY_STC                           ledKey;
    PDL_LED_PRV_CALLBACK_STC                    * callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    for (ledId = 0; ledId < PDL_LED_TYPE_DEVICE_LAST_E; ledId++) {
        if (pdlLedDeviceDb[ledId].capabilities.isSupported == TRUE) {
            pdlStatus = prvPdlLedDbGetCallbacks (ledId, PDL_LED_DONT_CARE_VALUE_CNS, PDL_LED_DONT_CARE_VALUE_CNS, &callbacksPtr);
            PDL_CHECK_STATUS(pdlStatus);
        }
    }   
    pdlStatus = prvPdlDbGetNextKey (&pdlLedPortDb, NULL, (void*) &ledKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlLedDbGetCallbacks (ledKey.ledId, ledKey.dev, ledKey.logicalPort, &callbacksPtr);
        PDL_CHECK_STATUS(pdlStatus);        
        pdlStatus = prvPdlDbGetNextKey (&pdlLedPortDb, (void*) &ledKey, (void*) &ledKey);
    }

    return PDL_OK;
}

/*$ END OF prvPdlLedVerifyExternalDrivers */

/* ***************************************************************************
* FUNCTION NAME: pdlLedInitLedStream
*
* DESCRIPTION:   this function will initialize ledstream port's position, polarity and interface settings
*                this should be executed when ports links are forced down (cpssDxChPortForceLinkDownEnableSet)
*                only neccassery when ledstream leds are configured in XML
*                will return PDL_NO_SUCH if nothing needs to be done
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS pdlLedInitLedStream(
	IN UINT_32              devIdx
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                    pdlStatus;
    PDL_LED_PRV_KEY_STC                           ledKey;
    PDL_LED_PRV_DB_STC 	                        * ledPtr;
    PDL_LED_PRV_LEDSTREAM_INTERFACE_KEY_STC       ledStreamInterfaceKey;
    PDL_LED_PRV_LEDSTREAM_INTERFACE_INFO_STC    * ledStreamInterfaceInfoPtr;
    PDL_LED_CLASS_MANIPULATION_KEY_STC            ledStreamManipulationKey;
    PDL_LED_CLASS_MANIPULATION_INFO_STC         * ledStreamManipulationInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    /* go over ledsream ports and set port polarity and position */
    pdlStatus = prvPdlDbGetNextKey (&pdlLedPortDb, NULL, (void*) &ledKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlDbFind (&pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS (pdlStatus);
        if (ledKey.dev == devIdx && ledPtr->capabilities.ledType == PDL_LED_TYPE_LEDSTREAM_LOGIC_BASED_E) {
            pdlStatus = prvPdlLedStreamPortPositionSet ((GT_U8)ledKey.dev, ledKey.logicalPort, 0x3F);
            PDL_CHECK_STATUS (pdlStatus);
            pdlStatus = prvPdlLedStreamPortPositionSet ((GT_U8)ledKey.dev, ledKey.logicalPort, ledPtr->values.ledStreamLogic.ledPosition);
            PDL_CHECK_STATUS (pdlStatus);
            pdlStatus = prvPdlLedStreamPortClassPolarityInvertEnableSet ((GT_U8)ledKey.dev, ledKey.logicalPort, ledPtr->values.ledStreamLogic.classNum, ledPtr->values.ledStreamLogic.polarityEnable);
            PDL_CHECK_STATUS (pdlStatus);
        }
        pdlStatus = prvPdlDbGetNextKey (&pdlLedPortDb, (void*) &ledKey, (void*) &ledKey);
    }
    /* set interface parameters */
    pdlStatus = prvPdlDbGetNextKey (&pdlLedStreamInterfaceDb, NULL, (void*) &ledStreamInterfaceKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlDbFind (&pdlLedStreamInterfaceDb, (void*) &ledStreamInterfaceKey, (void*) &ledStreamInterfaceInfoPtr);
        PDL_CHECK_STATUS (pdlStatus);
        if (ledStreamInterfaceKey.devNum == devIdx) {
            pdlStatus = prvPdlLedStreamConfigSet ((GT_U8)devIdx, ledStreamInterfaceKey.interfaceNumber, &ledStreamInterfaceInfoPtr->publicInfo);
            PDL_CHECK_STATUS (pdlStatus);
            /* set interface init manipulations */
            pdlStatus = prvPdlDbGetNextKey (&ledStreamInterfaceInfoPtr->manipulationList[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_INIT_E], NULL, (void*) &ledStreamManipulationKey);
            while (pdlStatus == PDL_OK) {
                pdlStatus = prvPdlDbFind (&ledStreamInterfaceInfoPtr->manipulationList[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_INIT_E], (void*) &ledStreamManipulationKey, (void*) &ledStreamManipulationInfoPtr);
                PDL_CHECK_STATUS (pdlStatus);
                    pdlStatus = prvPdlLedStreamClassManipulationSet ((GT_U8)devIdx, ledStreamInterfaceKey.interfaceNumber, ledStreamManipulationInfoPtr->portType, ledStreamManipulationInfoPtr->classNum, &ledStreamManipulationInfoPtr->classManipulation);
                    PDL_CHECK_STATUS (pdlStatus);
                pdlStatus = prvPdlDbGetNextKey (&ledStreamInterfaceInfoPtr->manipulationList[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_INIT_E], (void*) &ledStreamManipulationKey, (void*) &ledStreamManipulationKey);
            }
        }
        pdlStatus = prvPdlDbGetNextKey (&pdlLedStreamInterfaceDb, (void*) &ledStreamInterfaceKey, (void*) &ledStreamInterfaceKey);
    }

    return PDL_OK;
}

/*$ END OF pdlLedInitLedStream */

/* ***************************************************************************
* FUNCTION NAME: pdlLedHwLedStreamInterfaceSet
*
* DESCRIPTION:   this function will set interface level class manipualations for the given state & color 
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS pdlLedHwLedStreamInterfaceModeSet(
	IN  UINT_32                 devIdx,
    IN  UINT_8                  interfaceNumber,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                    pdlStatus;
    PDL_LED_COLOR_ENT                             adjustedColor;
    PDL_LED_PRV_LEDSTREAM_INTERFACE_KEY_STC       ledStreamInterfaceKey;
    PDL_LED_PRV_LEDSTREAM_INTERFACE_INFO_STC    * ledStreamInterfaceInfoPtr;
    PDL_LED_CLASS_MANIPULATION_KEY_STC            ledStreamManipulationKey;
    PDL_LED_CLASS_MANIPULATION_INFO_STC         * ledStreamManipulationInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    ledStreamInterfaceKey.devNum = (UINT_8)devIdx;
    ledStreamInterfaceKey.interfaceNumber = interfaceNumber;
    pdlStatus = prvPdlDbFind (&pdlLedStreamInterfaceDb, (void*) &ledStreamInterfaceKey, (void*) &ledStreamInterfaceInfoPtr);
    PDL_CHECK_STATUS (pdlStatus);

    if (state == PDL_LED_STATE_INIT_E || state == PDL_LED_STATE_OFF_E) {
        adjustedColor = PDL_LED_COLOR_NONE_E;
    }
    else {
        adjustedColor = color;
    }

    pdlStatus = prvPdlDbGetNextKey (&ledStreamInterfaceInfoPtr->manipulationList[adjustedColor][state], NULL, (void*) &ledStreamManipulationKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlDbFind (&ledStreamInterfaceInfoPtr->manipulationList[adjustedColor][state], (void*) &ledStreamManipulationKey, (void*) &ledStreamManipulationInfoPtr);
        PDL_CHECK_STATUS (pdlStatus);
            pdlStatus = prvPdlLedStreamClassManipulationSet ((GT_U8)devIdx, ledStreamInterfaceKey.interfaceNumber, ledStreamManipulationInfoPtr->portType, ledStreamManipulationInfoPtr->classNum, &ledStreamManipulationInfoPtr->classManipulation);
            PDL_CHECK_STATUS (pdlStatus);
        pdlStatus = prvPdlDbGetNextKey (&ledStreamInterfaceInfoPtr->manipulationList[adjustedColor][state], (void*) &ledStreamManipulationKey, (void*) &ledStreamManipulationKey);
    }
    
    return PDL_OK;
}

/*$ END OF pdlLedHwLedStreamInterfaceSet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedStreamDbPortInterfaceGet
*
* DESCRIPTION:   get given port ledstream interface number
*
* PARAMETERS:   
*
*****************************************************************************/

PDL_STATUS pdlLedStreamDbPortInterfaceGet(
	IN  UINT_32              devIdx,
    IN  UINT_32              logicalPort,
    OUT UINT_8             * interfaceIndexPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                    pdlStatus;
    PDL_LED_PRV_KEY_STC                           ledKey;
    PDL_LED_PRV_DB_STC 	                        * ledPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfaceIndexPtr == NULL) {
        return PDL_BAD_PTR;
    }
    ledKey.dev = devIdx;
    ledKey.logicalPort = logicalPort;
    ledKey.ledId = PDL_LED_TYPE_PORT_INBAND_PORT_LEFT_E;
    pdlStatus = prvPdlDbFind (&pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
    if (pdlStatus != PDL_OK || ledPtr->capabilities.ledType != PDL_LED_TYPE_LEDSTREAM_LOGIC_BASED_E) {
        return PDL_NO_SUCH;
    }
    *interfaceIndexPtr = ledPtr->values.ledStreamLogic.ledInterface;        
    return PDL_OK;

}

/*$ END OF pdlLedStreamDbPortInterfaceGet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedHwLedStreamPortModeSet
*
* DESCRIPTION:   set given port to execute manipulation index
*
* PARAMETERS:   
*
*****************************************************************************/

PDL_STATUS pdlLedHwLedStreamPortModeSet(
    IN  PDL_LED_ID_ENT      ledId,
    IN  UINT_32             devIdx,
    IN  UINT_32             logicalPort,
    IN  UINT_32             modeIndex
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_8                                        ledInterface;
    PDL_STATUS                                    pdlStatus;
    PDL_LED_PRV_KEY_STC                           ledKey;
    PDL_LED_PRV_DB_STC 	                        * ledPtr;
    PDL_LED_CLASS_MANIPULATION_STC                classManipulationCurrentValue, classManipulationValue;
    PDL_LED_CLASS_MANIPULATION_KEY_STC            ledStreamManipulationKey;
    PDL_LED_CLASS_MANIPULATION_INFO_STC         * ledStreamManipulationInfoPtr;
    PDL_LED_PRV_LEDSTREAM_PORT_MODE_KEY_STC       ledStreamPortModeKey;
    PDL_LED_PRV_LEDSTREAM_PORT_MODE_INFO_STC    * ledStreamPortModeInfoPtr;
    PDL_LED_PRV_LEDSTREAM_INTERFACE_KEY_STC       ledStreamInterfaceKey;
    PDL_LED_PRV_LEDSTREAM_INTERFACE_INFO_STC    * ledStreamInterfaceInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    ledKey.dev = devIdx;
    ledKey.logicalPort = logicalPort;
    ledKey.ledId = PDL_LED_TYPE_PORT_INBAND_PORT_LEFT_E;
    pdlStatus = prvPdlDbFind (&pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
    if (pdlStatus != PDL_OK || ledPtr->capabilities.ledType != PDL_LED_TYPE_LEDSTREAM_LOGIC_BASED_E) {
        return PDL_NO_SUCH;
    }
    ledInterface = ledPtr->values.ledStreamLogic.ledInterface;
    ledStreamInterfaceKey.devNum = (UINT_8)devIdx;
    ledStreamInterfaceKey.interfaceNumber = ledInterface;
    pdlStatus = prvPdlDbFind (&pdlLedStreamInterfaceDb, (void*) &ledStreamInterfaceKey, (void*) &ledStreamInterfaceInfoPtr);
    PDL_CHECK_STATUS (pdlStatus);

    ledStreamPortModeKey.index = modeIndex;
    if (ledId == PDL_LED_TYPE_PORT_INBAND_PORT_LEFT_E) {
        pdlStatus = prvPdlDbFind (&ledStreamInterfaceInfoPtr->leftPortModeManipulationDb, (void*) &ledStreamPortModeKey, (void*) &ledStreamPortModeInfoPtr);
        PDL_CHECK_STATUS (pdlStatus);
    }
    else if (ledId == PDL_LED_TYPE_PORT_INBAND_PORT_RIGHT_E) {
        pdlStatus = prvPdlDbFind (&ledStreamInterfaceInfoPtr->rightPortModeManipulationDb, (void*) &ledStreamPortModeKey, (void*) &ledStreamPortModeInfoPtr);
        PDL_CHECK_STATUS (pdlStatus);
    }
    else {
        return PDL_BAD_PARAM;
    }

    pdlStatus = prvPdlDbGetNextKey (&ledStreamPortModeInfoPtr->manipulationList, NULL, (void*) &ledStreamManipulationKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlDbFind (&ledStreamPortModeInfoPtr->manipulationList, (void*) &ledStreamManipulationKey, (void*) &ledStreamManipulationInfoPtr);
        PDL_CHECK_STATUS (pdlStatus);
            pdlStatus = prvPdlLedStreamClassManipulationGet ((GT_U8)devIdx, ledInterface, ledStreamManipulationInfoPtr->portType, ledStreamManipulationInfoPtr->classNum, &classManipulationCurrentValue);
            PDL_CHECK_STATUS (pdlStatus);
            memcpy (&classManipulationValue, &ledStreamManipulationInfoPtr->classManipulation, sizeof (PDL_LED_CLASS_MANIPULATION_STC));
            if (classManipulationValue.forceData == 1) {
                classManipulationValue.forceData = classManipulationCurrentValue.forceData | 1 << ledPtr->values.ledStreamLogic.ledPosition;
            }
            else {
                classManipulationValue.forceData = classManipulationCurrentValue.forceData & ~(1 << ledPtr->values.ledStreamLogic.ledPosition);
            }
            pdlStatus = prvPdlLedStreamClassManipulationSet ((GT_U8)devIdx, ledInterface, ledStreamManipulationInfoPtr->portType, ledStreamManipulationInfoPtr->classNum, &classManipulationValue);
            PDL_CHECK_STATUS (pdlStatus);
        pdlStatus = prvPdlDbGetNextKey (&ledStreamPortModeInfoPtr->manipulationList, (void*) &ledStreamManipulationKey, (void*) &ledStreamManipulationKey);
    }
    
    return PDL_OK;

}

/*$ END OF pdlLedStreamDbPortInterfaceGet */

PDL_STATUS pdlLedInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
    PDL_DB_PRV_ATTRIBUTES_STC       dbAttributes;
    PDL_LED_PRV_CALLBACK_STC      * callbackInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    dbAttributes.listAttributes.keySize = sizeof(PDL_LED_PRV_KEY_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_LED_PRV_DB_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &pdlLedPortDb);
    PDL_CHECK_STATUS(pdlStatus);

    dbAttributes.listAttributes.keySize = sizeof(PDL_LED_PRV_LEDSTREAM_INTERFACE_KEY_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_LED_PRV_LEDSTREAM_INTERFACE_INFO_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &pdlLedStreamInterfaceDb);
    PDL_CHECK_STATUS(pdlStatus);

    if (pdlLedInternalDriverBind == FALSE) {
        pdlStatus = prvPdlLedBindInternalDriver (&pdlLedCallbacksInternal);
        PDL_CHECK_STATUS(pdlStatus);
        pdlLedInternalDriverBind = TRUE;
    }

    memset (&pdlLedDeviceDb, 0, sizeof(pdlLedDeviceDb));

    pdlStatus = prvPdlLedXMLParser (xmlId);
    PDL_CHECK_STATUS(pdlStatus);

    /* init all external fan drivers */
    pdlStatus = prvPdlDbGetFirst(&pdlLedCallbacksDb, (void **)&callbackInfoPtr);
    while (pdlStatus == PDL_OK) {
        if(callbackInfoPtr->callbackInfo.ledInitDbCallbackInfo.initFun != NULL) {
            prvPdlLock(callbackInfoPtr->callbackInfo.ledInitDbCallbackInfo.lockType);
            callbackInfoPtr->callbackInfo.ledInitDbCallbackInfo.initFun();
            prvPdlUnlock(callbackInfoPtr->callbackInfo.ledInitDbCallbackInfo.lockType);
        }
        pdlStatus = prvPdlDbGetNext(&pdlLedCallbacksDb, (void*)&callbackInfoPtr->externalDriverId, (void **)&callbackInfoPtr);
    }

    return PDL_OK;
}

/*$ END OF pdlLedInit */

/**
 * @fn  PDL_STATUS prvPdlLedDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLedDestroy (
    void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                 i, j;
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC 	                  * ledPtr;
    PDL_LED_PRV_KEY_STC                     ledKey;
    PDL_LED_LEDSTREAM_FUNC_CALL_INFO_STC  * funcInfoPtr;
    PDL_LED_LEDSTREAM_FUNC_CALL_KEY_STC     funcKey;
    PDL_LED_PRV_LEDSTREAM_INTERFACE_KEY_STC       ledStreamInterfaceKey;
    PDL_LED_PRV_LEDSTREAM_INTERFACE_INFO_STC    * ledStreamInterfaceInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    pdlStatus = prvPdlDbGetNextKey (&pdlLedPortDb, NULL, (void*) &ledKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus  = prvPdlDbFind (&pdlLedPortDb, (void*) &ledKey, (void**) &ledPtr);
        PDL_CHECK_STATUS(pdlStatus);
        if (ledPtr->capabilities.ledType == PDL_LED_TYPE_LEDSTREAM_E) {
            for (i = 0; i < PDL_LED_STATE_LAST_E; i++) {
                if (ledPtr->values.ledStreamFuncCalls.functionCallDb[i].dbPtr != NULL) {
                    pdlStatus = prvPdlDbGetNextKey (&ledPtr->values.ledStreamFuncCalls.functionCallDb[i], NULL, (void*) &funcKey);
                    while (pdlStatus == PDL_OK) {
                        pdlStatus  = prvPdlDbFind (&ledPtr->values.ledStreamFuncCalls.functionCallDb[i], (void*) &funcKey, (void**) &funcInfoPtr);
                        PDL_CHECK_STATUS(pdlStatus);
                        pdlStatus = prvPdlDbDestroy (&funcInfoPtr->params);
                        PDL_CHECK_STATUS(pdlStatus);
                        pdlStatus = prvPdlDbGetNextKey (&ledPtr->values.ledStreamFuncCalls.functionCallDb[i], (void*) &funcKey, (void*) &funcKey);
                    }
                    pdlStatus = prvPdlDbDestroy (&ledPtr->values.ledStreamFuncCalls.functionCallDb[i]);
                    PDL_CHECK_STATUS(pdlStatus);
                }
            }
        }
        if (ledPtr->capabilities.ledType == PDL_LED_TYPE_SMI_E || ledPtr->capabilities.ledType == PDL_LED_TYPE_XSMI_E) {
            pdlStatus = prvPdlDbDestroy (&ledPtr->values.smiValues.initValueList);
            PDL_CHECK_STATUS(pdlStatus);
        }
        pdlStatus = prvPdlDbGetNextKey (&pdlLedPortDb, (void*) &ledKey, (void*) &ledKey);
    }

    if (pdlLedPortDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlLedPortDb);
        PDL_CHECK_STATUS(pdlStatus);    
    }
 
    /* destroy ledstream manipulation lists */
    pdlStatus = prvPdlDbGetNextKey (&pdlLedStreamInterfaceDb, NULL, (void*) &ledStreamInterfaceKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlDbFind (&pdlLedStreamInterfaceDb, (void*) &ledStreamInterfaceKey, (void*) &ledStreamInterfaceInfoPtr);
        PDL_CHECK_STATUS (pdlStatus);
        for (i = 0; i < PDL_LED_COLOR_LAST_E; i++) {
            for (j = 0; j < PDL_LED_COLOR_LAST_E; j++) {
                if (ledStreamInterfaceInfoPtr->manipulationList[i][j].dbPtr != NULL) {
                    pdlStatus = prvPdlDbDestroy (&ledStreamInterfaceInfoPtr->manipulationList[i][j]);
                    PDL_CHECK_STATUS(pdlStatus);
                }
            }
        }
        pdlStatus = prvPdlDbGetNextKey (&pdlLedStreamInterfaceDb, (void*) &ledStreamInterfaceKey, (void*) &ledStreamInterfaceKey);
    }


    if (pdlLedStreamInterfaceDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlLedStreamInterfaceDb);
        PDL_CHECK_STATUS(pdlStatus);    
    }
    pdlLedInternalDriverBind = FALSE;
    if (pdlLedCallbacksDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlLedCallbacksDb);
        PDL_CHECK_STATUS(pdlStatus);
    }
    
    return PDL_OK;
}

/*$ END OF prvPdlLedDestroy */
