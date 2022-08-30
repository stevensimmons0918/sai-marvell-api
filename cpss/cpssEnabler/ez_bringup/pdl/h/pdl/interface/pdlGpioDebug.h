/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\interface\pdlgpiodebug.h.
 *
 * @brief   Declares the pdlgpiodebug class
 */

#ifndef __prvPdlGpioDebugh

#define __prvPdlGpioDebugh
/**
********************************************************************************
 * @file pdlGpioDebug.h  
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
 * @brief Platform driver layer - Gpio interface debug related API (internal)
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>

#define PDL_GPIO_SET_ATTRIBUTE_MASK_DEVICE          1 << 0
#define PDL_GPIO_SET_ATTRIBUTE_MASK_PIN_NUMBER      1 << 1
#define PDL_GPIO_SET_ATTRIBUTE_MASK_TYPE            1 << 2
#define PDL_GPIO_SET_ATTRIBUTE_MASK_VALUE           1 << 3

/**
* @addtogroup Interface
* @{
*/

/**
 * @addtogroup GPIO
 * @{
 */

/** 
 * @defgroup GPIO_Debug GPIO Debug
 * @{defines access method, size, address (including indirect), mask & good value. \n
 *  Supporting:
 * - GPIO interface debug APIs
*/

/****************************** Enum definitions ********************************/

/**
 * @fn  PDL_STATUS pdlGpioDebugSet ( IN BOOLEAN state );
 *
 * @brief   set GPIO module debug on/off
 *
 * @param [in]  state   - debug is turned on/off.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlGpioDebugSet (
    IN  BOOLEAN             state
);

/**
 * @fn  PDL_STATUS PdlGpioDebugDbGetAttributes ( IN PDL_INTERFACE_TYP interfaceId, OUT PDL_INTERFACE_GPIO_STC * attributesPtr );
 *
 * @brief   get GPIO attributes return all GPIO relevant information for given interfaceId based
 *          on information from XML
 *
 * @param [in]  interfaceId     - Interface identifier.
 * @param [out] attributesPtr   - GPIO attributes.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS PdlGpioDebugDbGetAttributes (
    IN  PDL_INTERFACE_TYP                    interfaceId,
    OUT PDL_INTERFACE_GPIO_STC             * attributesPtr
);

/**
 * @fn  PDL_STATUS PdlGpioDebugDbSetAttributes ( IN PDL_INTERFACE_TYP interfaceId, IN PDL_INTERFACE_GPIO_STC * attributesPtr );
 *
 * @brief   set GPIO attributes update GPIO relevant information for given interfaceId based on
 *          information from XML
 *
 * @param [in]  interfaceId     - Interface identifier.
 * @param [in]  attributesPtr   - GPIO attributes.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS PdlGpioDebugDbSetAttributes (
    IN  PDL_INTERFACE_TYP                    interfaceId,
    IN  UINT_32                              attributesMask,
    IN  PDL_INTERFACE_GPIO_STC             * attributesPtr
);

/**
 * @fn  PDL_STATUS pdlGpioDebugHwSetValue ( IN UINT_32 dev, IN UINT_32 pinNumber, IN UINT_32 data )
 *
 * @brief   Wrap pdl gpio hardware set value
 *
 * @param   dev         The development.
 * @param   pinNumber   The pin number.
 * @param   data        The data.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlGpioDebugHwSetValue (
    IN  UINT_32                                 dev,
    IN  UINT_32                                 pinNumber,
    IN  PDL_INTERFACE_GPIO_OFFSET_ENT           offset,
    IN  UINT_32                                 data             /* input data can be 0 or 1*/
);

/* @}*/
/* @}*/
/* @}*/

#endif
