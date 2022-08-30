/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\sfp\pdlsfpdebug.h.
 *
 * @brief   Declares the pdlsfpdebug class
 */

#ifndef __pdlSfpDebugh

#define __pdlSfpDebugh
/**
********************************************************************************
 * @file pdlSfpDebug.h   
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
 * @brief Platform driver layer - SFP/GBIC debug library
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/sfp/pdlSfp.h>

/**
* @addtogroup SFP
* @{
*/
 /** 
 * @defgroup SFP_debug  SFP Debug
 * @{SFP debug related functions
*/

extern PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC prvPdSfpValueTypeStrToEnum;

/**
 * @enum    PDL_SFP_VALUE_TYPE_ENT
 *
 * @brief   Enumerator for SFP value types
 */

typedef enum {
    PDL_SFP_VALUE_EEPROM_E = 1,
    PDL_SFP_VALUE_LOSS_E,
    PDL_SFP_VALUE_PRESENT_E,
    PDL_SFP_VALUE_TX_ENABLE_E,
    PDL_SFP_VALUE_TX_DISABLE_E
}PDL_SFP_VALUE_TYPE_ENT;

/**
 * @fn  PDL_STATUS PdlSfpDebugInterfaceGet ( IN UINT_32 dev, IN UINT_32 port, OUT PDL_SFP_INTERFACE_STC * interfacePtr );
 *
 * @brief   Get sfp access information
 *          @note
 *          Based on data retrieved from XML
 *
 * @param [in]  dev             - dev number.
 * @param [in]  port            - port number.
 * @param [out] interfacePtr    - sfp interface pointer.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS PdlSfpDebugInterfaceGet (
    IN  UINT_32                   dev,
    IN  UINT_32                   port,
    OUT PDL_SFP_INTERFACE_STC   * interfacePtr
);

/**
 * @fn  PDL_STATUS PdlSfpDebugInterfaceSet ( IN UINT_32 dev, IN UINT_32 port, IN PDL_SFP_INTERFACE_STC * interfacePtr );
 *
 * @brief   Set sfp access information (used to change goodValue of present / loss / tx enable
 *          interfaces
 *          @note
 *          Used to change data retrieved from XML, can later be saved
 *
 * @param [in]  dev          - dev number.
 * @param [in]  port         - port number.
 * @param [in]  interfacePtr - sfp interface pointer.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS PdlSfpDebugInterfaceSet (
    IN  UINT_32                   dev,
    IN  UINT_32                   port,
    IN PDL_SFP_INTERFACE_STC    * interfacePtr
);


/* @}*/
/* @}*/

#endif
