/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\led\pdlleddebug.h.
 *
 * @brief   Declares the pdlleddebug class
 */

#ifndef __pdlLedDebugh

#define __pdlLedDebugh
/**
********************************************************************************
 * @file pdlLedDebug.h   
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
 * @brief Platform driver layer - LED related debug API
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/led/pdlLed.h>

/**
* @addtogroup Led
* @{
*/

typedef struct {
    UINT_8                  devOrPage;
    UINT_16                 registerAddress;
    UINT_16                 value;
    UINT_16                 mask;
} PDL_LED_SMI_XSMI_VALUE_STC;

/**
 * @struct  PDL_LED_VALUES_STC
 *
 * @brief   defines structure that hold all device specific values
 */

typedef struct {
    /** @brief   The LED value */
    union {
        UINT_32                           gpioValue;
        UINT_32                           i2cValue;
        PDL_LED_SMI_XSMI_VALUE_STC        smiValue;
    } value;
} PDL_LED_VALUES_STC;

/**
 * @struct  PDL_LED_INTERFACE_STC
 *
 * @brief   defines interface for working with a led (interface information) + predefined values
 */

typedef struct {
    /** @brief   Type of the interface */
    PDL_INTERFACE_TYPE_ENT                interfaceType;
    /** @brief   Identifier for the LED interface */
    PDL_INTERFACE_TYP                     ledInterfaceId;
    /** @brief   The values */
    PDL_LED_VALUES_STC                    values;          /* this will hold the value relevant to the color & mode given by user in PdlLedDebugInterfaceGet */
} PDL_LED_INTERFACE_STC;


 /** 
 * @defgroup Led_Debug Led Debug
 * @{LED driver debug APIs
 */

/**
 * @fn  PDL_STATUS pdlLedDebugInterfaceGetFirst ( IN PDL_LED_ID_ENT ledId, IN UINT_32 dev, IN UINT_32 port, IN PDL_LED_STATE_ENT state, IN PDL_LED_COLOR_ENT color, OUT PDL_LED_INTERFACE_STC * interfacePtr );
 *
 * @brief   Get led access information
 *          @note
 *          Based on data retrieved from XML
 *
 * @param [in]  ledId           - led identifier.
 * @param [in]  dev             - device number.
 * @param [in]  port            - port number.
 * @param [in]  state           - led state (off/solid/blinking,etc.)
 * @param [in]  color           - led color (green/amber)
 * @param [out] interfacePtr    - led interface pointer with value relevant to given state &amp;
 *                              color.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlLedDebugInterfaceGetFirst (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 port,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color,
    OUT PDL_LED_INTERFACE_STC * interfacePtr
);

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
    IN  UINT_32                 port,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color,
    IN  UINT_32                 key,
    OUT PDL_LED_INTERFACE_STC * interfacePtr
);

/*$ END OF pdlLedDebugInterfaceGetNext */

/**
 * @fn  PDL_STATUS pdlLedDebugInterfaceSet ( IN PDL_LED_ID_ENT ledId, IN UINT_32 dev, IN UINT_32 port, IN PDL_LED_STATE_ENT state, IN PDL_LED_COLOR_ENT color, IN PDL_LED_VALUES_STC * valuesPtr );
 *
 * @brief   Set buttonId access information
 *          @note
 *          Used to change data retrieved from XML, can later be saved
 *
 * @param [in]  ledId       - led identifier.
 * @param [in]  dev         - device number.
 * @param [in]  port        - port number.
 * @param [in]  state       - led state (off/solid/blinking,etc.)
 * @param [in]  color       - led color (green/amber)
 * @param [in]  valuesPtr   - led value pointer with value relevant to given state &amp; color.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlLedDebugInterfaceSet (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 port,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color,
    IN  PDL_LED_VALUES_STC    * valuesPtr
);

/*$ END OF pdlLedDebugInterfaceSet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedDebugInfoSet
*
* DESCRIPTION:   change LED value for given state & color
*
* PARAMETERS:   
*               
* RETURN VALUES: PDL_OK             -   success
                 PDL_NOT_FOUND      -   led doesn't exists in XML
                 PDL_BAD_PARAM      -   given state&color doesn't exist for this led type
                 PDL_NOT_SUPPORTED  -   user is trying to set a value that doesn't already exist in XML
*****************************************************************************/

PDL_STATUS pdlLedDebugInfoSet (
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 port,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color,
	IN	UINT_32					value,
    IN  UINT_32                 devOrPage,
    IN  UINT_32                 registerAddress,
    IN  UINT_32                 mask
);

/*$ END OF pdlLedDebugInfoSet */

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
);

/*$ END OF pdlLedDebugSet */

/* @}*/
/* @}*/
#endif