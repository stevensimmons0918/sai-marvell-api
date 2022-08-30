/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\oob_port\private\prvpdloobport.h.
 *
 * @brief   Declares the prvpdloobport class
 */

#ifndef __prvPdlOobh

#define __prvPdlOobh
/**
********************************************************************************
 * @file prvPdlOobPort.h   
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
 * @brief Platform driver layer - OOB port private declarations and APIs
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/oob_port/pdlOobPort.h>
#include <pdl/xml/private/prvXmlParser.h>

/**
* @addtogroup OOB_Port
* @{
*/

 /** 
 * @defgroup Oob_private OOB Private
 * @{Oob private definitions and declarations including:
*/

/**
 * @struct  PRV_PDL_OOB_PORT_KEY_STC
 *
 * @brief   defines structure for oob port key
 */

typedef struct {
    /** @brief   The port number */
    UINT_32                                 portNumber;
} PRV_PDL_OOB_PORT_KEY_STC;

/**
 * @struct  PRV_PDL_OOB_PORT_ENTRY_STC
 *
 * @brief   defines structure stored for oob port
 */

typedef struct {
    /** @brief   The key */
    PRV_PDL_OOB_PORT_KEY_STC                key;
    /** @brief   Information describing the oob */
    PDL_OOB_PORT_XML_ATTRIBUTES_STC         oobInfo;
} PRV_PDL_OOB_PORT_ENTRY_STC;

/**
 * @fn  PDL_STATUS prvPdlOobPortInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId );
 *
 * @brief   Init oob port module Create oob port DB and initialize
 *
 * @param [in]  xmlId   Xml id.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlOobPortInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
);

/**
 * @fn  PDL_STATUS prvPdlOobDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlOobDestroy (
    void
);
/* @}*/
/* @}*/

#endif
