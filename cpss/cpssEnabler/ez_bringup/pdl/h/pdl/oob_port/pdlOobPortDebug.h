/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\oob_port\pdlPobPortDebug.h.
 *
 * @brief   Declares the pdloobport class
 */

#ifndef __pdlOobPortDebugh
#define __pdlOobPortDebugh
/**
********************************************************************************
 * @file pdlOobPortDebug.h   
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
#include <pdl/oob_port/pdlOobPort.h>

/**
 * @fn  PDL_STATUS pdlOobPortAttributesSet ( IN UINT_32 portNumber, IN PDL_OOB_PORT_XML_ATTRIBUTES_STC * portAttributesPtr )
 *
 * @brief   Set OOB port attributes
 *
 * @param [in] portNumber           OOB port number.
 * @param [in] portAttributesPtr    OOB port attributes.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlOobPortAttributesSet (
    IN   UINT_32                                portNumber,
    IN   PDL_OOB_PORT_XML_ATTRIBUTES_STC      * portAttributesPtr
);

/**
 * @fn  PDL_STATUS pdlOobDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Oob debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlOobDebugSet (
    IN  BOOLEAN             state
);

 
#endif
