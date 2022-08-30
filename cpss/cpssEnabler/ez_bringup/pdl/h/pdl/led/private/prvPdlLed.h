/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\led\private\prvpdlled.h.
 *
 * @brief   Declares the prvpdlled class
 */

#ifndef __prvPdlLedh

#define __prvPdlLedh
/**
********************************************************************************
 * @file prvPdlLed.h   
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
 * @brief Platform driver layer - Led private declarations and APIs
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/led/pdlLed.h>
#include <pdl/led/pdlLedDebug.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <pdl/init/pdlInitLedStream.h>


/**
* @addtogroup Led
* @{
*/

#define PDL_LED_UNSUPPORTED_VALUE           0xFFFF

extern PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC pdlLedTypeStrToEnum;
extern PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC pdlLedStateStrToEnum;
extern PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC pdlLedColorStrToEnum;


 /** 
 * @defgroup Led_private led Private
 * @{LED private definitions and declarations including: 
*/

/**
 * @struct  PDL_LED_PRV_KEY_STC
 *
 * @brief   defines structure for led key in db
 */

typedef struct {
    /** @brief   The development */
    UINT_32                 dev;
    /** @brief   The port */
    UINT_32                 logicalPort;
    /** @brief   Identifier for the LED */
    PDL_LED_ID_ENT          ledId;
} PDL_LED_PRV_KEY_STC;

/**
 * @struct  PDL_LED_GPIO_INTERFACE_STC
 *
 * @brief   defines structure for controlling a gpio led
 */

typedef struct {
    /** @brief   The LED interface id[ pdl LED color last e] */
    PDL_INTERFACE_TYP                       ledInterfaceId[PDL_LED_COLOR_LAST_E];
} PDL_LED_GPIO_INTERFACE_STC;

/**
 * @struct  PDL_LED_GPIO_VALUES_STC
 *
 * @brief   defines structure for controlling a gpio led
 */

typedef struct {
    /** @brief   The color value[ pdl LED color last e] */
    UINT_32                 colorValue[PDL_LED_COLOR_LAST_E];
} PDL_LED_GPIO_VALUES_STC;

/**
 * @struct  PDL_LED_I2C_INTERFACE_STC
 *
 * @brief   defines structure for controlling a i2c led
 */

typedef struct {
    /** @brief   Identifier for the LED interface */
    PDL_INTERFACE_TYP                       ledInterfaceId;
} PDL_LED_I2C_INTERFACE_STC;

/**
 * @struct  PDL_LED_I2C_VALUES_STC
 *
 * @brief   defines structure for controlling an i2c led
 */

typedef struct {
    /** @brief   The value[ pdl LED color last e][pdl LED state last e] */
    UINT_32                 value[PDL_LED_COLOR_LAST_E][PDL_LED_STATE_LAST_E];
} PDL_LED_I2C_VALUES_STC;

typedef struct {
    /** @brief   Identifier for the LED interface */
    PDL_INTERFACE_TYP                       ledInterfaceId;
} PDL_LED_SMI_XSMI_INTERFACE_STC;

typedef struct {
    /** @brief   The value[ pdl LED color last e][pdl LED state last e] */
    PDL_LED_SMI_XSMI_VALUE_STC          value[PDL_LED_COLOR_LAST_E][PDL_LED_STATE_LAST_E];
    PDL_DB_PRV_STC                      initValueList;
} PDL_LED_SMI_XSMI_VALUES_STC;

typedef struct{
    UINT_32                 valueNumber;
}PDL_LED_SMI_XSMI_KEY_STC;

typedef struct {
    PDL_DB_PRV_STC          functionCallDb[PDL_LED_STATE_LAST_E];
} PDL_LED_LEDSTREAM_FUNC_CALLS_STC;

typedef struct {
    UINT_32                 functionCallNumber;
} PDL_LED_LEDSTREAM_FUNC_CALL_KEY_STC;

typedef struct {
    char                    functionName[PDL_XML_MAX_TAG_LEN];
    PDL_DB_PRV_STC          params;
} PDL_LED_LEDSTREAM_FUNC_CALL_INFO_STC;

typedef struct {
    PDL_LED_LEDSTREAM_FUNC_CALL_KEY_STC          key;
    PDL_LED_LEDSTREAM_FUNC_CALL_INFO_STC         info;
} PDL_LED_LEDSTREAM_FUNC_CALL_ENTRY_STC;

typedef struct {
    UINT_32                 keyNumber;
} PDL_LED_LEDSTREAM_FUNC_PARAM_KEY_STC;

typedef struct {
    char                    paramName[PDL_XML_MAX_TAG_LEN];
    UINT_32                 paramValue;
} PDL_LED_LEDSTREAM_FUNC_PARAM_INFO_STC;

typedef struct {
    UINT_8                  ledInterface;
    UINT_8                  ledPosition;
    BOOLEAN                 polarityEnable;
    UINT_32                 classNum;
} PDL_LED_LEDSTREAM_LOGIC_STC;

/**
 * @struct  PDL_LED_PRV_DB_STC
 *
 * @brief   defines structure stored for led in db
 */

typedef struct {
    union {
        /** @brief   The gpio interface */
        PDL_LED_GPIO_INTERFACE_STC          gpioInterface;
        /** @brief   Zero-based index of the 2c interface */
        PDL_LED_I2C_INTERFACE_STC           i2cInterface;
        PDL_LED_SMI_XSMI_INTERFACE_STC      smiInterface;
        PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP  externalDriverId;
    } interface;

    /**
     * @union   values
     *
     * @brief   A values.
     */

    union {
        /** @brief   The gpio values */
        PDL_LED_GPIO_VALUES_STC             gpioValues;
        /** @brief   Zero-based index of the 2c values */
        PDL_LED_I2C_VALUES_STC              i2cValues;
        PDL_LED_SMI_XSMI_VALUES_STC         smiValues;
        PDL_LED_LEDSTREAM_FUNC_CALLS_STC    ledStreamFuncCalls;
        PDL_LED_LEDSTREAM_LOGIC_STC         ledStreamLogic;
    } values;
    /** @brief   The capabilities */
    PDL_LED_XML_CAPABILITIES_STC            capabilities;
} PDL_LED_PRV_DB_STC;

typedef struct {
    PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP    externalDriverId;
    PDL_LED_CALLBACK_INFO_STC               callbackInfo;
} PDL_LED_PRV_CALLBACK_STC;

typedef struct {
    pdlExtDrvInit_FUN                     * ledInitDbFun;
    pdlExtDrvInit_FUN                     * ledInitHwFun;
    pdlLedHwDeviceModeSet_FUN             * ledHwDeviceModeSetFun;
    pdlLedHwPortModeSet_FUN               * ledHwPortModeSetFun;
} PDL_LED_PRV_CALLBACK_INFO_STC;


/**
 * @struct  PDL_LED_PRV_LEDSTREAM_INTERFACE_KEY_STC
 *
 * @brief   defines structure for ledstream interface key in db
 */

typedef struct {
    /** @brief   ledstream interface device number */
    UINT_8                              devNum;
    /** @brief   ledstream interface number */
    UINT_8                              interfaceNumber;
} PDL_LED_PRV_LEDSTREAM_INTERFACE_KEY_STC;


/**
 * @struct  PDL_LED_PRV_LEDSTREAM_INTERFACE_INFO_STC
 *
 * @brief   defines structure for ledstream interface info in db
 */
typedef struct {
    /** @brief   Information describing the public */
    PDL_LED_STEAM_INTERFACE_CONF_STC    publicInfo;
    PDL_DB_PRV_STC                      manipulationList[PDL_LED_COLOR_LAST_E][PDL_LED_STATE_LAST_E];
    PDL_DB_PRV_STC                      leftPortModeManipulationDb;
    PDL_DB_PRV_STC                      rightPortModeManipulationDb;

} PDL_LED_PRV_LEDSTREAM_INTERFACE_INFO_STC;

/**
 * @struct  PDL_LED_PRV_LEDSTREAM_PORT_MODE_KEY_STC
 *
 * @brief   defines structure for ledstream port mode key in db
 */

typedef struct {
    /** @brief   ledstream port mode key index */
    UINT_32                             index;
} PDL_LED_PRV_LEDSTREAM_PORT_MODE_KEY_STC;

/**
 * @struct  PDL_LED_PRV_LEDSTREAM_PORT_MODE_INFO_STC
 *
 * @brief   defines structure for ledstream port mode info in db
 */

typedef struct {
    /** @brief   ledstream port mode description */
    char                                portModeDescription[PDL_XML_MAX_TAG_LEN];
    /** @brief   ledstream port mode class manipulation list */
    PDL_DB_PRV_STC                      manipulationList;
} PDL_LED_PRV_LEDSTREAM_PORT_MODE_INFO_STC;


/* ***************************************************************************
* FUNCTION NAME: prvPdlLedHwDeviceModeSet
*
* DESCRIPTION:   set LED mode (state & color)
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS prvPdlLedHwDeviceModeSet (
    IN  PDL_LED_ID_ENT          ledId,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
);

/*$ END OF prvPdlLedHwDeviceModeSet */

/* ***************************************************************************
* FUNCTION NAME: prvPdlLedHwPortModeSet
*
* DESCRIPTION:   set LED mode (state & color)
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS prvPdlLedHwPortModeSet (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
);

/*$ END OF prvPdlLedHwPortModeSet */

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
);

/*$ END OF prvPdlLedVerifyExternalDrivers */

/**
 * @fn  PDL_STATUS pdlLedInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId );
 *
 * @brief   Init Led module Create led DB and initialize
 *
 * @param [in]  xmlId   - Xml id.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlLedInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
);

/**
 * @fn  PDL_STATUS prvPdlLedDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLedDestroy (
    void
);
/* @}*/
/* @}*/

#endif
