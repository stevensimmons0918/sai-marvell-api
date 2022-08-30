/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\sfp\private\prvpdlsfp.h.
 *
 * @brief   Declares the prvpdlsfp class
 */

#ifndef __prvPdlSfph

#define __prvPdlSfph
/**
********************************************************************************
 * @file prvPdlSfp.h   
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
 * @brief Platform driver layer - SFP private declarations and APIs
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/sfp/pdlSfp.h>
#include <pdl/xml/private/prvXmlParser.h>


/**
* @addtogroup Sfp
* @{
*/

/**
 * @struct  PDL_SFP_PRV_KEY_STC
 *
 * @brief   defines structure for sfp key in db
 */

typedef struct {
    /** @brief   The development */
    UINT_32                 dev;
    /** @brief   The port */
    UINT_32                 logicalPort;
} PDL_SFP_PRV_KEY_STC;

/**
 * @struct  PDL_SFP_PRV_DB_STC
 *
 * @brief   defines structure stored for sfp in db
 */

typedef struct {
    /** @brief   Information describing the public */
    PDL_SFP_INTERFACE_STC                   publicInfo;
} PDL_SFP_PRV_DB_STC;

/**
 * @fn  PDL_STATUS prvPdlSfpXmlParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, IN UINT_32 dev, IN UINT_32 port );
 *
 * @brief   Parse SFP section from XML
 *
 * @param [in]  xmlId   - Xml Id.
 * @param [in]  dev     - dev number.
 * @param [in]  port    - port number.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlSfpXmlParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    IN  UINT_32                                 dev,
    IN  UINT_32                                 logicalPort
);

/**
 * @fn  PDL_STATUS pdlSfpInit ( IN void );
 *
 * @brief   Init SFP module Create button DB and initialize
 *
 * @param   void    The void.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlSfpInit (
    IN  void
);

/**
 * @fn  PDL_STATUS prvPdlSfpDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSfpDestroy (
    void
);
/* @}*/
/* @}*/

#endif
