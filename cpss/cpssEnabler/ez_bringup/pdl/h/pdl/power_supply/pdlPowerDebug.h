/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\power_supply\pdlpowerdebug.h.
 *
 * @brief   Declares the pdlpowerdebug class
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

#include <pdl/power_supply/pdlPower.h>

/**
* @addtogroup Power_Supply
* @{
*/

 /** 
 * @defgroup Power_supply_Debug Power Supply Debug
 * @{power supply driver debug APIs
 */

/**
 * @fn  PDL_STATUS pdlPowerDebugStatusAccessInfoGet ( IN PDL_POWER_PS_TYPE_ENT type, OUT PDL_POWER_PS_INTERFACE_STC * interfacePtr );
 *
 * @brief   get PS interface specification
 *
 * @param [in]  type            - power supply type (PS or RPS)
 * @param [out] interfacePtr    - power supply interface pointer.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlPowerDebugStatusAccessInfoGet (
    IN  PDL_POWER_PS_TYPE_ENT          type,
    OUT PDL_POWER_PS_INTERFACE_STC   * interfacePtr 
);

/**
 * @fn  PDL_STATUS pdlPowerDebugPseActiveAccessInfoGet ( IN UINT_32 device, IN UINT_32 port, OUT PDL_POWER_PORT_PSE_INTERFACE_STC * interfacePtr );
 *
 * @brief   get PSE interface specification
 *
 * @param [in]  device          .
 * @param [in]  port            .
 * @param [out] interfacePtr    - power supply interface pointer.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlPowerDebugPseActiveAccessInfoGet (
    IN  UINT_32                             device,
    IN  UINT_32                             logicalPort,
    OUT PDL_POWER_PORT_PSE_INTERFACE_STC  * interfacePtr
);

/**
 * @fn  PDL_STATUS pdlPowerDebugPdActiveAccessInfoGet ( IN UINT_32 device, IN UINT_32 port, OUT PDL_POWER_PD_PORT_INTERFACE_STC * interfacePtr );
 *
 * @brief   get PD interface specification
 *
 * @param [in]  device          .
 * @param [in]  port            .
 * @param [out] interfacePtr    - get active access information.
 *
 * @return  TBD.
 */

PDL_STATUS pdlPowerDebugPdActiveAccessInfoGet (
    IN  UINT_32                             device,
    IN  UINT_32                             logicalPort,
    OUT PDL_POWER_PD_PORT_INTERFACE_STC   * interfacePtr
);

/* @}*/
/* @}*/

#endif