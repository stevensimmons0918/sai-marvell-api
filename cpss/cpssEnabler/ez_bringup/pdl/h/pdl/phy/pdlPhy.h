/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\phy\pdlphy.h.
 *
 * @brief   Declares the pdlphy class
 */

#ifndef __pdlPhyh

#define __pdlPhyh
/**
********************************************************************************
 * @file pdlPhy.h   
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
 * @brief Platform driver layer - PHY library
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>
 /** 
 * @defgroup PHY PHY
 * @{
*/

/**
 * @struct  PDL_PHY_CONFIGURATION_STC
 *
 * @brief   defines phy configuration parameters
 */

typedef struct {
    /** @brief   Type of the phy */
    PDL_PHY_TYPE_ENT                    phyType;
    PDL_PHY_DOWNLOAD_TYPE_ENT           phyDownloadType;
} PDL_PHY_CONFIGURATION_STC;

/**
 * @struct  PDL_PHY_PORT_CONFIGURATION_STC
 *
 * @brief   defines phy configuration parameters
 */

typedef struct {
    /** @brief   The phy number */
    UINT_32                             phyNumber;
    /** @brief   The phy position */
    UINT_32                             phyPosition;

    /**
     * @struct  smiXmsiInterface
     *
     * @brief   A smi xmsi interface.
     */
    PDL_INTERFACE_SMI_XSMI_STC          smiXmsiInterface;
} PDL_PHY_PORT_CONFIGURATION_STC;

typedef struct {
    UINT_8                  devOrPage;
    UINT_16                 registerAddress;
    UINT_16                 value;
    UINT_16                 mask;
} PDL_PHY_SMI_XSMI_VALUE_STC;

/**
 * @fn  PDL_STATUS PdlPhyDbNumberOfPhysGet( OUT UINT_32 * numOfPhysPtr );
 *
 * @brief   Get number of phys
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [out] numOfPhysPtr    number of phys.
 *
 * @return  PDL_STATUS  .
 */

PDL_STATUS PdlPhyDbNumberOfPhysGet(
    OUT UINT_32                           * numOfPhysPtr
);

/**
 * @fn  PDL_STATUS PdlPhyDbPhyConfigurationGet ( IN UINT_32 phyId, OUT PDL_PHY_CONFIGURATION_STC * phyConfigPtr );
 *
 * @brief   Get phy configuration data
 *
 * @param [in]  phyId           phy identifier starting from 1 up to numOfPhys.
 * @param [out] phyConfigPtr    phy configuration data.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS PdlPhyDbPhyConfigurationGet (
    IN  UINT_32                             phyId,
    OUT PDL_PHY_CONFIGURATION_STC         * phyConfigPtr
);

/**
 * @fn  PDL_STATUS pdlPhyDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Phy debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPhyDebugSet (
    IN  BOOLEAN             state
);


/* @}*/

#endif
