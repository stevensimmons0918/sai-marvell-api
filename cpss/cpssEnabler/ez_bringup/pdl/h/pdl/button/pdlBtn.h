/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\button\pdlbtn.h.
 *
 * @brief   Declares the pdlbtn class
 */

#ifndef __pdlBtnh

#define __pdlBtnh
/**
********************************************************************************
 * @file pdlBtn.h   
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
 * @brief Platform driver layer - Button related API
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>

 /** 
 * @defgroup Button Button 
 * @{Button driver definitions and declarations
 * 
*/

/**
 * @enum    PDL_BTN_STATUS_ENT
 *
 * @brief   Enumerator for button status
 */

typedef enum {
    PDL_BTN_STATUS_PRESSED_E = 1,
    PDL_BTN_STATUS_NOT_PRESSED_E
}PDL_BTN_STATUS_ENT;

/**
 * @enum    PDL_BTN_ID_ENT
 *
 * @brief   Enumerator for button ids
 */

typedef enum {
    /*! Reset button */
    PDL_BTN_ID_RESET_E = 0,
    /*! Led mode configuration button */
    PDL_BTN_ID_LED_MODE_E,
    PDL_BTN_ID_LAST_E
}PDL_BTN_ID_ENT;

/**
 * @struct  PDL_BTN_XML_CAPABILITIES_STC
 *
 * @brief   defines what is supported by fan \b buttonId
 */

typedef struct {
    /** @brief   Does button support reading operational status (pressed) */
    PDL_IS_SUPPORTED_ENT		    isSupported;
} PDL_BTN_XML_CAPABILITIES_STC;

/**
 * @struct  PDL_BTN_VALUES_STC
 *
 * @brief   defines structure that hold all device specific values
 */

typedef struct {
    /** @brief   The push value */
    UINT_32                               pushValue;
} PDL_BTN_VALUES_STC;

/**
 * @struct  PDL_BTN_INTERFACE_STC
 *
 * @brief   defines interface for working with a button (interface information) + predefined
 *          values
 */

typedef struct {
    /** @brief   Type of the interface */
    PDL_INTERFACE_TYPE_ENT                interfaceType;    
    /** @brief   Identifier for the button interface */
    PDL_INTERFACE_TYP                     buttonInterfaceId;
    /** @brief   The values */
    PDL_BTN_VALUES_STC                    values;
} PDL_BTN_INTERFACE_STC;

/**
 * @fn  PDL_STATUS pdlBtnDbCapabilitiesGet ( IN PDL_BTN_ID_ENT buttonId, OUT PDL_BTN_XML_CAPABILITIES_STC * capabilitiesPtr );
 *
 * @brief   Get \b buttonId supported capabilities
 *          @note
 *          Based on data retrieved from XML
 *
 * @param [in]  buttonId        - button identifier.
 * @param [out] capabilitiesPtr - button capabilities.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlBtnDbCapabilitiesGet (
    IN  PDL_BTN_ID_ENT                  buttonId,
    OUT PDL_BTN_XML_CAPABILITIES_STC  * capabilitiesPtr
);

/**
 * @fn  PDL_STATUS pdlBtnHwStatusGet ( IN PDL_BTN_ID_ENT buttonId, OUT PDL_BTN_STATUS_ENT * statusPtr );
 *
 * @brief   get button operational status (pressed/not pressed)
 *          based on information from HW access
 *
 * @param [in]  buttonId    - button identifier.
 * @param [out] statusPtr   - button status.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlBtnHwStatusGet (
    IN  PDL_BTN_ID_ENT         buttonId,
    OUT PDL_BTN_STATUS_ENT   * statusPtr
);

#endif
