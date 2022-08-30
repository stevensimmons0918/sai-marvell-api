/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\oob_port\pdloobport.h.
 *
 * @brief   Declares the pdloobport class
 */

#ifndef __pdlOobPorth

#define __pdlOobPorth
/**
********************************************************************************
 * @file pdlOobPort.h   
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
 * @brief Platform driver layer - OOB port library
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/phy/pdlPhy.h>

 /** 
 * @defgroup OOB_Port OOB Port
 * @{OOB port related functions
*/

/**
 * @struct  PDL_OOB_PORT_XML_ATTRIBUTES_STC
 *
 * @brief   defines port attributes
 */

typedef struct {
  /** @brief The port maximum speed */
  PDL_PORT_SPEED_ENT                portMaximumSpeed;
  /** @brief The CPU port number */
  UINT_32                           cpuPortNumber;
  /** @brief The ETH Id */
  UINT_32                           ethId;
  /** @brief The phy configuration */
  PDL_PHY_PORT_CONFIGURATION_STC    phyConfig;
} PDL_OOB_PORT_XML_ATTRIBUTES_STC;

/**
 * @fn  PDL_STATUS pdlOobPortNumOfObbPortsGet ( OUT UINT_32 * numOfOobsPtr );
 *
 * @brief   Get number of OOB ports
 *
 * @param [out] numOfOobsPtr    number of oob ports in system.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlOobPortNumOfObbPortsGet (
    OUT  UINT_32    * numOfOobsPtr
);

/**
 * @fn  PDL_STATUS pdlOobPortAttributesGet ( IN UINT_32 portNumber, OUT PDL_OOB_PORT_XML_ATTRIBUTES_STC * portAttributesPtr );
 *
 * @brief   Get OOB port attributes
 *
 * @param [in]  portNumber          OOB port number.
 * @param [out] portAttributesPtr   OOB port attributes.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlOobPortAttributesGet (
    IN   UINT_32                                portNumber,
    OUT  PDL_OOB_PORT_XML_ATTRIBUTES_STC      * portAttributesPtr
);


/* @}*/

#endif
