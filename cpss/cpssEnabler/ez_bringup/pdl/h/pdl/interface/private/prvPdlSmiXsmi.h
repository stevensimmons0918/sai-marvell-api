/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\interface\private\prvPdlSmiXsmi.h.
 *
 * @brief   Declares the prvPdlSmiXsmi class
 */

#ifndef __prvPdlSmiXsmih
#define __prvPdlSmiXsmih
/**
********************************************************************************
 * @file prvPdlSmiXsmi.h  
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
 * @brief Platform driver layer - Private SMI/XSMI related API (internal)
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/xml/private/prvXmlParser.h>

/**
* @addtogroup Interface
* @{
*/

/**
* @addtogroup SMI/XSMI
* @{
*/

#define CPSS_PORT_GROUP_UNAWARE_MODE_CNS    0
#define PDL_SMI_XSMI_TAG_NAME               "smi-xsmi-interface"

 typedef struct {
    PDL_INTERFACE_SMI_XSMI_STC	             publicInfo;
} PDL_INTERFACE_PRV_SMI_XSMI_DB_STC;

 /**
 * @fn  PDL_STATUS prvPdlSmiXsmiXmlAttributesParser ()
 *
 * @brief   get attributes for smi-xsmi interface
 *
 * @param           xmlId               Identifier for the XML.
 * @param [out]     smiXsmiInfoPtr      info for smi-xsmi interface that has been retrived from XML
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSmiXsmiXmlAttributesParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    OUT PDL_INTERFACE_PRV_SMI_XSMI_DB_STC     * smiXsmiInfoPtr
);

/* @fn  PDL_STATUS prvPdlSmiXsmiXmlIsSmiXsmi ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT BOOLEAN * isSmiXsmi )
 *
 * @brief   Prv pdl XML is xmi/xsmi
 *
 * @param           xmlId   -   Identifier for the XML.
 * @param [out] isSmiXsmi   -   is xmlId pointing to a smi/xsmi interface.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSmiXsmiXmlIsSmiXsmi (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    OUT BOOLEAN                               * isSmiXsmi
);

/*$ END OF prvPdlSmiXsmiXmlIsSmiXsmi */

/**
 * @fn  PDL_STATUS prvPdlSmiXsmiXmlParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT PDL_INTERFACE_TYP * interfaceIdPtr, OUT PDL_INTERFACE_TYPE_ENT * interfaceTypePtr)
 *
 * @brief   SMI/XSMI xml parser
 *
 * @param           xmlId               Identifier for the XML.
 * @param [out]     interfaceIdPtr      If non-null, the interface identifier pointer.
 * @param [out]     interfaceTypePtr    If non-null, the interface type pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSmiXsmiXmlParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    OUT PDL_INTERFACE_TYP                     * interfaceIdPtr,
    OUT PDL_INTERFACE_TYPE_ENT                * interfaceTypePtr
);

/*$ END OF prvPdlSmiXsmiXmlParser */

/* ***************************************************************************
* FUNCTION NAME: prvPdlSmiXsmiAddInterface
*
* DESCRIPTION:   add smi/xsmi interface with given attributes and return it's interfaceId
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS prvPdlSmiXsmiAddInterface (
    IN  PDL_INTERFACE_PRV_SMI_XSMI_DB_STC     * smiXsmiInfoPtr,
    OUT PDL_INTERFACE_TYP                     * interfaceIdPtr
);

/*$ END OF prvPdlSmiXsmiAddInterface */

/**
 * @fn  PDL_STATUS PdlSmiXsmiInit ( IN void )
 *
 * @brief   Pdl SMI/XSMI initialize
 *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSmiXsmiInit (
    IN  void
);


/**
 * @fn  PDL_STATUS prvPdlSmiXsmiDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSmiXsmiDestroy (
    void
);

/* @}*/
/* @}*/
/* @}*/

#endif
