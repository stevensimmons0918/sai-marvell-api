/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\phy\private\prvpdlphy.h.
 *
 * @brief   Declares the prvpdlphy class
 */

#ifndef __prvPdlPhyh

#define __prvPdlPhyh
/**
********************************************************************************
 * @file prvPdlPhy.h   
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
 * @brief Platform driver layer - Phy private declarations and APIs
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/phy/pdlPhy.h>
#include <pdl/xml/private/prvXmlParser.h>


#define PRV_PDL_PHYS_TAG_NAME_CNS       "PHYs"

#define PRV_PDL_PHY_TAG_NAME_CNS        "phy-list"

#define PRV_PDL_PHY_ID_TAG_NAME_CNS     "phy-number"

#define PRV_PDL_PHY_TYPE_TAG_NAME_CNS   "phy-type"

#define PRV_PDL_PHY_FW_DOWNLOAD_METHOD_TAG_NAME_CNS   "download-method"

#define PRV_PDL_PHY_MAX_POSITION_CNS        7
#define PRV_PDL_PHY_SMI_ADDR_MAX_VALUE_CNS  31
#define PRV_PDL_PHY_ILLEGAL_NUMBER_CNS      0xFF
/**
* @addtogroup Phy
* @{
*/

 /** 
 * @defgroup Phy_private
 * @{Phy private definitions and declarations including:
*/

/**
 * @struct  PRV_PDL_PHY_KEY_STC
 *
 * @brief   defines structure for phy key
 */

typedef struct {
    /** @brief   Identifier for the phy */
    UINT_8                                  phyId;
} PRV_PDL_PHY_KEY_STC;

/**
 * @struct  PRV_PDL_PHY_DB_STC
 *
 * @brief   defines structure stored for phy
 */

typedef struct {
    /** @brief   The key */
    PRV_PDL_PHY_KEY_STC                     key;
    /** @brief   Information describing the phy */
    PDL_PHY_CONFIGURATION_STC               phyInfo;
} PRV_PDL_PHY_DB_STC;

/**
 * @fn  PDL_STATUS prvPdlPhyPortDataGet ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT PDL_PHY_PORT_CONFIGURATION_STC *phyDataPtr );
 *
 * @brief   Parse phy port xml section
 *
 * @param [in]  xmlId       Xml id pointing to 'port-phy-connection' to parse.
 * @param [out] phyDataPtr  phy data.
 *
 * @return  PDL_XML_PARSE_ERROR if xml section coul not be parsed.
 * @return  PDL_BAD_PTR         if supplied pointer is NULL.
 */

PDL_STATUS prvPdlPhyPortDataGet (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    OUT PDL_PHY_PORT_CONFIGURATION_STC      *phyDataPtr
);

/**
 * @fn  PDL_STATUS PdlPhyInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId );
 *
 * @brief   Init phy module Create phy DB and initialize
 *
 * @param [in]  xmlId   Xml id.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS PdlPhyInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
);

/**
 * @fn  PDL_STATUS prvPdlPhyDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPhyDestroy (
    void
);
/* @}*/
/* @}*/

#endif
