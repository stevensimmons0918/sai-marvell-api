/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\led\pdlled.h.
 *
 * @brief   Declares the pdlled class
 */

#ifndef __pdlLedh

#define __pdlLedh
/**
********************************************************************************
 * @file pdlLed.h   
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

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/private/prvPdlInterface.h>

 /** 
 * @defgroup Led Led
 * @{Led driver definitions and declarations including:
 * - Status get
*/

#define PDL_LED_DONT_CARE_VALUE_CNS     0xFF

typedef enum {
    PDL_LED_TYPE_I2C_E = 0,
    PDL_LED_TYPE_SMI_E,
    PDL_LED_TYPE_XSMI_E,
    PDL_LED_TYPE_GPIO_E,
    PDL_LED_TYPE_SMI_CPU_E,
    PDL_LED_TYPE_LEDSTREAM_E,
    PDL_LED_TYPE_EXTERNAL_DRIVER_E,
    PDL_LED_TYPE_LEDSTREAM_LOGIC_BASED_E,
} PDL_LED_TYPE_ENT;

/**
 * @enum    PDL_LED_ID_ENT
 *
 * @brief   Enumerator for led ids
 */

typedef enum {
    PDL_LED_TYPE_DEVICE_SYSTEM_E,
    PDL_LED_TYPE_DEVICE_RPS_E,
    PDL_LED_TYPE_DEVICE_FAN_E,
    PDL_LED_TYPE_PORT_POE_E,
    PDL_LED_TYPE_PORT_PORT_LED_STATE_E,
    PDL_LED_TYPE_DEVICE_STACK_UNIT1_E,
    PDL_LED_TYPE_DEVICE_STACK_UNIT2_E,
    PDL_LED_TYPE_DEVICE_STACK_UNIT3_E,
    PDL_LED_TYPE_DEVICE_STACK_UNIT4_E,
    PDL_LED_TYPE_DEVICE_STACK_UNIT5_E,
    PDL_LED_TYPE_DEVICE_STACK_UNIT6_E,
    PDL_LED_TYPE_DEVICE_STACK_UNIT7_E,
    PDL_LED_TYPE_DEVICE_STACK_UNIT8_E,
    PDL_LED_TYPE_DEVICE_STACK_MASTER_E,
    PDL_LED_TYPE_DEVICE_LAST_E,
    PDL_LED_TYPE_PORT_FIRST_E = PDL_LED_TYPE_DEVICE_LAST_E,
    PDL_LED_TYPE_PORT_OOB_LEFT_E,
    PDL_LED_TYPE_PORT_OOB_RIGHT_E,
    PDL_LED_TYPE_PORT_INBAND_PORT_LEFT_E,
    PDL_LED_TYPE_PORT_INBAND_PORT_RIGHT_E,
    PDL_LED_TYPE_PORT_LAST_E
} PDL_LED_ID_ENT;

/**
 * @enum    PDL_LED_COLOR_ENT
 *
 * @brief   Enumerator for led colors
 */

typedef enum {
    PDL_LED_COLOR_NONE_E,        /* MUST be first - used for states that don't require a color like off / init */
    PDL_LED_COLOR_GREEN_E,
    PDL_LED_COLOR_AMBER_E,
    PDL_LED_COLOR_LAST_E
} PDL_LED_COLOR_ENT;

/**
 * @enum    PDL_LED_STATE_ENT
 *
 * @brief   Enumerator for led state
 */

typedef enum {
    PDL_LED_STATE_OFF_E,
    PDL_LED_STATE_SOLID_E,
    PDL_LED_STATE_LOWRATEBLINKING_E,
    PDL_LED_STATE_NORMALRATEBLINKING_E,
    PDL_LED_STATE_HIGHRATEBLINKING_E,
    PDL_LED_STATE_INIT_E,
    PDL_LED_STATE_LAST_E
} PDL_LED_STATE_ENT;


/**
 * @struct  PDL_LED_XML_CAPABILITIES_STC
 *
 * @brief   defines if led capabilities
 */

typedef struct {
    /** @brief   The is supported */
    PDL_IS_SUPPORTED_ENT		    isSupported;
    PDL_LED_TYPE_ENT                ledType;
} PDL_LED_XML_CAPABILITIES_STC;


/* external driver function decelerations */

typedef PDL_STATUS pdlLedHwDeviceModeSet_FUN (
    IN  PDL_LED_ID_ENT          ledId,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
);
typedef PDL_STATUS pdlLedHwPortModeSet_FUN (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
);

typedef struct {
    pdlLedHwDeviceModeSet_FUN                 * ledHwDeviceModeSetFun;
    PDL_OS_LOCK_TYPE_ENT                        lockType;
} PDL_LED_CALLBACK_HW_DEVICE_MODE_SET_INFO_STC;

typedef struct {
    pdlLedHwPortModeSet_FUN                   * ledHwPortModeSetFun;
    PDL_OS_LOCK_TYPE_ENT                        lockType;
} PDL_LED_CALLBACK_HW_PORT_MODE_SET_INFO_STC;

typedef struct {
    PDL_CALLBACK_INIT_INFO_STC                      ledInitDbCallbackInfo;
    PDL_CALLBACK_INIT_INFO_STC                      ledInitHwCallbackInfo;
    PDL_LED_CALLBACK_HW_DEVICE_MODE_SET_INFO_STC    ledHwDeviceModeSetCallbackInfo;
    PDL_LED_CALLBACK_HW_PORT_MODE_SET_INFO_STC      ledHwPortModeSetCallbackInfo;
} PDL_LED_CALLBACK_INFO_STC;


/**
 * @fn  PDL_STATUS pdlLedDbDeviceCapabilitiesGet ( IN PDL_LED_ID_ENT ledId, IN UINT_32 dev, IN UINT_32 logicalPort, OUT PDL_LED_XML_CAPABILITIES_STC * capabilitiesPtr );
 *
 * @brief   Get \b led supported capabilities
 *          @note
 *          Based on data retrieved from XML
 *
 * @param [in]  ledId           - led identifier.
 * @param [in]  dev             - device number.
 * @param [in]  port            - port number.
 * @param [out] capabilitiesPtr - led capabilities.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlLedDbDeviceCapabilitiesGet (
    IN  PDL_LED_ID_ENT                  ledId,
    OUT PDL_LED_XML_CAPABILITIES_STC *  capabilitiesPtr
);

/**
 * @fn  PDL_STATUS pdlLedDbPortCapabilitiesGet ( IN PDL_LED_ID_ENT ledId, IN UINT_32 dev, IN UINT_32 logicalPort, OUT PDL_LED_XML_CAPABILITIES_STC * capabilitiesPtr );
 *
 * @brief   Get \b led supported capabilities
 *          @note
 *          Based on data retrieved from XML
 *
 * @param [in]  ledId           - led identifier.
 * @param [in]  dev             - device number.
 * @param [in]  port            - port number.
 * @param [out] capabilitiesPtr - led capabilities.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlLedDbPortCapabilitiesGet (
    IN  PDL_LED_ID_ENT                  ledId,
    IN  UINT_32                         dev,
    IN  UINT_32                         logicalPort,
    OUT PDL_LED_XML_CAPABILITIES_STC *  capabilitiesPtr
);

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
);

/**
 * @fn  PDL_STATUS pdlLedHwPortModeSet ( IN PDL_LED_ID_ENT ledId, IN UINT_32 dev, IN UINT_32 logicalPort, IN PDL_LED_STATE_ENT state, IN PDL_LED_COLOR_ENT color );
 *
 * @brief   set LED mode (state &amp; color)
 *
 * @param [in]  ledId   - led identifier.
 * @param [in]  dev     - device number.
 * @param [in]  port    - port number.
 * @param [in]  state   - led state (off/solid/blinking,etc.)
 * @param [in]  color   - led color (green/amber)
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlLedHwPortModeSet (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
);

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
);

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
);

/*$ END OF pdlLedStreamFunctionNumGet */

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
);

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
);

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
);

/*$ END OF pdlLedStreamFunctionParamGet */

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
);

/*$ END OF pdlLedBindExternalDriver */

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
);

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
);

/*$ END OF pdlLedHwLedStreamInterfaceModeSet */

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
);

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
);

/*$ END OF pdlLedHwLedStreamPortModeSet */



/* @}*/

#endif