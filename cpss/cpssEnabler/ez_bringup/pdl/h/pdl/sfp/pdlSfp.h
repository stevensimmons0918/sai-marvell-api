/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\sfp\pdlsfp.h.
 *
 * @brief   Declares the pdlsfp class
 */

#ifndef __pdlSfph

#define __pdlSfph
/**
********************************************************************************
 * @file pdlSfp.h   
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
 * @brief Platform driver layer - SFP/GBIC library
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>

 /** 
 * @defgroup SFP SFP
 * @{SFP related functions including
 * - tx enable set/get
 * - loss info get
 * - present info get
 * - eeprom read
*/

/**
 * @enum    PDL_SFP_TX_ENT
 *
 * @brief   Enumerator for SFP tx enable status
 */

typedef enum {
    PDL_SFP_TX_ENABLE_E = 1,
    PDL_SFP_TX_DISABLE_E
}PDL_SFP_TX_ENT;

/**
 * @enum    PDL_SFP_PRESENT_ENT
 *
 * @brief   Enumerator for SFP present status
 */

typedef enum {
    PDL_SFP_PRESENT_TRUE_E = 1,
    PDL_SFP_PRESENT_FALSE_E
}PDL_SFP_PRESENT_ENT;

/**
 * @enum    PDL_SFP_LOSS_ENT
 *
 * @brief   Enumerator for SFP loss status
 */

typedef enum {
    PDL_SFP_LOSS_TRUE_E = 1,
    PDL_SFP_LOSS_FALSE_E
}PDL_SFP_LOSS_ENT;

/**
 * @struct  PDL_SFP_VALUES_STC
 *
 * @brief   defines structure that hold all SFP specific values
 */

typedef struct {
    /** @brief   The present value */
    UINT_32                               presentValue;
    /** @brief   The loss value */
    UINT_32                               lossValue;
    /** @brief   The transmit enable value */
    UINT_32                               txEnableValue;
    /** @brief   The transmit disable value */
    UINT_32                               txDisableValue;
} PDL_SFP_VALUES_STC;

/**
 * @struct  PDL_SFP_INTERFACE_STC
 *
 * @brief   defines interface for working with sfp (interface information) + predefined values
 */

typedef struct {

    /**
     * @struct  presentInfo
     *
     * @brief   Information about the present.
     */

    struct {
        BOOLEAN                                 isSupported;
        PDL_INTERFACE_TYPE_ENT                  interfaceType;    
        PDL_INTERFACE_TYP                       interfaceId;    
    } presentInfo;

    /**
     * @struct  lossInfo
     *
     * @brief   Information about the loss.
     */

    struct {
        BOOLEAN                                 isSupported;
        /** @brief   Type of the interface */
        PDL_INTERFACE_TYPE_ENT                  interfaceType;    
        /** @brief   Identifier for the interface */
        PDL_INTERFACE_TYP                       interfaceId;
    } lossInfo;

    /**
     * @struct  txEnableInfo
     *
     * @brief   Information about the transmit enable.
     */

    struct {
        BOOLEAN                                 isSupported;
        /** @brief   Type of the interface */
        PDL_INTERFACE_TYPE_ENT                  interfaceType;    
        /** @brief   Identifier for the interface */
        PDL_INTERFACE_TYP                       interfaceId;    
    } txEnableInfo;

    /**
     * @struct  txDisableInfo
     *
     * @brief   Information about the transmit disable.
     */

    struct {
        BOOLEAN                                 isSupported;
        /** @brief   Type of the interface */
        PDL_INTERFACE_TYPE_ENT                  interfaceType;    
        /** @brief   Identifier for the interface */
        PDL_INTERFACE_TYP                       interfaceId;    
    } txDisableInfo;

    /**
     * @struct  eepromInfo
     *
     * @brief   Information about the eeprom.
     */

    struct {
        BOOLEAN                                 isSupported;
        /** @brief   Type of the interface */
        PDL_INTERFACE_TYPE_ENT                  interfaceType;    
        /** @brief   Identifier for the interface */
        PDL_INTERFACE_TYP                       interfaceId;    
    } eepromInfo;                               /* used to access EEPROM (get gbic model/name, etc.) */
    /** @brief   The values */
    PDL_SFP_VALUES_STC                          values;
} PDL_SFP_INTERFACE_STC;

/**
 * @fn  PDL_STATUS pdlSfpHwTxGet ( IN UINT_32 dev, IN UINT_32 port, OUT PDL_SFP_TX_ENT * statusPtr );
 *
 * @brief   Get sfp operational status (tx enable/disable)
 *
 * @param [in]  dev         - dev number.
 * @param [in]  port        - port number.
 * @param [out] statusPtr   - sfp status.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlSfpHwTxGet (
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    OUT PDL_SFP_TX_ENT            * statusPtr
);

/**
 * @fn  PDL_STATUS pdlSfpHwTxSet ( IN UINT_32 dev, IN UINT_32 port, IN PDL_SFP_TX_ENT status );
 *
 * @brief   set sfp operational status (tx enable/disable)
 *
 * @param [in]  dev     - dev number.
 * @param [in]  port    - port number.
 * @param [in]  status  - sfp status.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlSfpHwTxSet (
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    IN PDL_SFP_TX_ENT               status
);

/**
 * @fn  PDL_STATUS pdlSfpHwLossGet ( IN UINT_32 dev, IN UINT_32 port, OUT PDL_SFP_LOSS_ENT * statusPtr );
 *
 * @brief   Get sfp loss status
 *
 * @param [in]  dev         - dev number.
 * @param [in]  port        - port number.
 * @param [out] statusPtr   - sfp loss status.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlSfpHwLossGet (
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    OUT PDL_SFP_LOSS_ENT          * statusPtr
);

/**
 * @fn  PDL_STATUS pdlSfpHwPresentGet ( IN UINT_32 dev, IN UINT_32 port, OUT PDL_SFP_PRESENT_ENT * statusPtr );
 *
 * @brief   Get sfp present status
 *
 * @param [in]  dev         - dev number.
 * @param [in]  port        - port number.
 * @param [out] statusPtr   - sfp present status.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlSfpHwPresentGet (
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    OUT PDL_SFP_PRESENT_ENT       * statusPtr
);

/**
 * @fn  PDL_STATUS pdlSfpHwEepromRead ( IN UINT_32 dev, IN UINT_32 port, IN UINT_16 offset, IN UINT_32 length, OUT void * dataPtr );
 *
 * @brief   Read sfp eeprom
 *
 * @param [in]  dev             - dev number.
 * @param [in]  port            - port number.
 * @param [in]  i2cAddress      - i2c address to read from MUST be 0x50 or 0x51
 * @param [in]  offset          - offset.
 * @param [in]  length          - length.
 * @param [out] dataPtr         - data that was read from eeprom will be written to this pointer (MUST
 *                                point to a memory allocation the size of length)
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlSfpHwEepromRead (
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    IN  UINT_8                      i2cAddress,
    IN  UINT_16                     offset,
    IN  UINT_32                     length,
    OUT void                      * dataPtr
);

/**
 * @fn  PDL_STATUS pdlSfpDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Sfp debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSfpDebugSet (
    IN  BOOLEAN             state
);

#endif
