/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\interface\pdli2cdebug.h.
 *
 * @brief   Declares the pdli 2cdebug class
 */

#ifndef __prvPdlI2CDebugh

#define __prvPdlI2CDebugh
/**
********************************************************************************
 * @file pdlI2CDebug.h  
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
 * @brief Platform driver layer - I2C interface debug related API (internal)
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>

/**
* @addtogroup Interface
* @{
*/

/**
* @addtogroup I2C
* @{
*/

/** 
 * @defgroup I2C_Debug I2C Debug
 * @{defines access method, size, address (including indirect), mask & good value. \n
 *  Supporting:
 * - I2C interface debug APIs
*/

#define PDL_I2C_SET_ATTRIBUTE_MASK_BUS_ID           1 << 0
#define PDL_I2C_SET_ATTRIBUTE_MASK_ADDRESS          1 << 1
#define PDL_I2C_SET_ATTRIBUTE_MASK_ACCESS_SIZE      1 << 2
#define PDL_I2C_SET_ATTRIBUTE_MASK_OFFSET           1 << 3
#define PDL_I2C_SET_ATTRIBUTE_MASK_MASK             1 << 4
#define PDL_I2C_SET_ATTRIBUTE_MASK_VALUE            1 << 5


/****************************** Enum definitions ********************************/

/**
 * @fn  PDL_STATUS pdlI2cDebugSet ( IN BOOLEAN state );
 *
 * @brief   set I2C module debug on/off
 *
 * @param [in]  state   - debug is turned on/off.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlI2cDebugSet (
    IN  BOOLEAN             state
);

/**
 * @fn  PDL_STATUS PdlI2CDebugDbGetAttributes ( IN PDL_INTERFACE_TYP interfaceId, OUT PDL_INTERFACE_I2C_STC * attributesPtr );
 *
 * @brief   get I2C attributes return all I2C relevant information for given interfaceId based on
 *          information from XML
 *
 * @param [in]  interfaceId     - Interface identifier.
 * @param [out] attributesPtr   - I2C attributes.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS PdlI2CDebugDbGetAttributes (
    IN  PDL_INTERFACE_TYP                   interfaceId,
    OUT PDL_INTERFACE_I2C_STC             * attributesPtr
);


/**
 * @fn  PDL_STATUS PdlI2CDebugDbSetAttributes ( IN PDL_INTERFACE_TYP interfaceId, IN PDL_INTERFACE_I2C_STC * attributesPtr );
 *
 * @brief   set I2C attributes update I2C relevant information for given interfaceId based on
 *          information from XML
 *
 * @param [in]  interfaceId     - Interface identifier.
 * @param [in]  attributesPtr   - I2C attributes.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS PdlI2CDebugDbSetAttributes (
    IN  PDL_INTERFACE_TYP                   interfaceId,
    IN  UINT_32                             attributesMask,
    IN  PDL_INTERFACE_I2C_STC             * attributesPtr
);

/**
 * @fn  PDL_STATUS PdlI2CDebugDbGetMuxAttributes ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 muxNumber, OUT PDL_INTERFACE_MUX_STC * muxAttributesPtr );
 *
 * @brief   get I2C Mux attributes based on information from XML
 *
 * @param [in]  interfaceId         - Interface identifier.
 * @param [in]  muxNumber           - muxNumber.
 * @param [out] muxAttributesPtr    - stored mux attributes.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS PdlI2CDebugDbGetMuxAttributes (
    IN  PDL_INTERFACE_TYP                   interfaceId,
    IN  UINT_32                             muxNumber,
    OUT PDL_INTERFACE_MUX_STC             * muxAttributesPtr
);

/**
 * @fn  PDL_STATUS PdlI2CDebugDbSetMuxAttributes ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 muxId, IN PDL_INTERFACE_MUX_STC * muxAttributesPtr );
 *
 * @brief   set I2C Mux attributes update I2C MUX relevant information for given mux_number based
 *          on information from XML
 *
 * @param [in]  interfaceId         - Interface identifier.
 * @param [in]  muxId               - muxId.
 * @param [in]  muxAttributesPtr    - new mux attributes.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS PdlI2CDebugDbSetMuxAttributes (
    IN  PDL_INTERFACE_TYP                   interfaceId,
    IN  UINT_32                             muxNumber,
    IN  UINT_32                             attributesMask,
    IN  PDL_INTERFACE_MUX_STC             * muxAttributesPtr
);

/* @}*/
/* @}*/
/* @}*/

#endif
