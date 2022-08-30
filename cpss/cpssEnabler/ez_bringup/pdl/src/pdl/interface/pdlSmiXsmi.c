/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/** 
********************************************************************************
 * @file pdlSmiXsmi.c   
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
 * @brief Platform driver layer - SMI/XSMI related API
 * 
 * @version   1 
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/common/pdlTypes.h>
#include <pdl/interface/private/prvPdlSmiXsmi.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <pdl/interface/pdlSmiXsmiDebug.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

static PDL_DB_PRV_STC           prvPdlSmiXsmiDb;
static UINT_32                  prvPdlSmiXsmiInterfaceCount = 0;          /* counter used as an ID for new interfaces */

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/**
 * @fn  PDL_STATUS prvPdlSmiXsmiXmlIsSmiXsmi ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT BOOLEAN * isSmiXsmi )
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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char        pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    UINT_32     maxSize = PDL_XML_MAX_TAG_LEN;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (isSmiXsmi == NULL) {
        return PDL_BAD_PTR;
    }
    xmlParserGetName(xmlId, &maxSize, &pdlTagStr[0]);
    if (strcmp (pdlTagStr, PDL_SMI_XSMI_TAG_NAME) == 0) {
        *isSmiXsmi = TRUE;
    }
    else {
        *isSmiXsmi = FALSE;
    }

    return PDL_OK;
}
/*$ END OF prvPdlSmiXsmiXmlIsSmiXsmi */

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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                pdlValueStr[PDL_XML_MAX_TAG_LEN] = {0};
    /*char                                pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};*/
    XML_PARSER_RET_CODE_TYP             xmlStatus = PDL_OK, xmlStatus2;
    /*XML_PARSER_NODE_DESCRIPTOR_TYP      xmlSmiXsmiChildId;*/
    UINT_32                             maxSize;
    BOOLEAN                             address = FALSE, type = FALSE, id = FALSE;
    XML_PARSER_NODE_DESCRIPTOR_TYP      xmlSecondaryAddressId;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (smiXsmiInfoPtr == NULL) {
        return PDL_BAD_PTR;
    }
    memset (smiXsmiInfoPtr, 0, sizeof(PDL_INTERFACE_PRV_SMI_XSMI_DB_STC));

    /*maxSize = PDL_XML_MAX_TAG_LEN;
    xmlStatus = xmlParserGetName(xmlId, &maxSize, &pdlTagStr[0]);
    XML_CHECK_STATUS(xmlStatus);    */
    /* xmliD not pointing to smi-xsmi-interface */
    /*if (strcmp (pdlTagStr, PDL_SMI_XSMI_TAG_NAME) != 0) {
        return PDL_ERROR;
    }*/

    /*xmlStatus = xmlParserGetFirstChild (xmlId, &xmlSmiXsmiChildId);*/
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);    
        if (xmlParserIsEqualName (xmlId, "phy-smi-interface-type") == XML_PARSER_RET_CODE_OK) {
            maxSize = PDL_XML_MAX_TAG_LEN;
            xmlStatus = xmlParserGetValue (xmlId, &maxSize, &pdlValueStr[0]);
            if (strcmp (pdlValueStr, "smi") == 0) {
                smiXsmiInfoPtr->publicInfo.interfaceType = PDL_INTERFACE_TYPE_SMI_E;
            }
            else if (strcmp (pdlValueStr, "xsmi") == 0) {
                smiXsmiInfoPtr->publicInfo.interfaceType = PDL_INTERFACE_TYPE_XSMI_E;
            }
            else {
                return PDL_XML_PARSE_ERROR;
            }
            XML_CHECK_STATUS(xmlStatus);
            type = TRUE;
        }
        else if (xmlParserIsEqualName (xmlId, "phy-pp-device") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(smiXsmiInfoPtr->publicInfo.dev);
            xmlStatus = xmlParserGetValue (xmlId, &maxSize, &smiXsmiInfoPtr->publicInfo.dev);
            XML_CHECK_STATUS(xmlStatus);
            id = TRUE;
        }
        else if (xmlParserIsEqualName (xmlId, "phy-smi-xsmi-interface-id") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(smiXsmiInfoPtr->publicInfo.interfaceId);
            xmlStatus = xmlParserGetValue (xmlId, &maxSize, &smiXsmiInfoPtr->publicInfo.interfaceId);
            XML_CHECK_STATUS(xmlStatus);
            id = TRUE;
        }
        else if (xmlParserIsEqualName (xmlId, "phy-smi-xsmi-address") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(smiXsmiInfoPtr->publicInfo.address);
            xmlStatus = xmlParserGetValue (xmlId, &maxSize, &smiXsmiInfoPtr->publicInfo.address);
            XML_CHECK_STATUS(xmlStatus);
            address = TRUE;
        }
        else if (xmlParserIsEqualName (xmlId, "phy-additional-smi-xsmi-address-list") == XML_PARSER_RET_CODE_OK) {
            xmlStatus2 = xmlParserGetFirstChild (xmlId, &xmlSecondaryAddressId);
            while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                XML_CHECK_STATUS(xmlStatus2);
                if (xmlParserIsEqualName (xmlSecondaryAddressId, "phy-smi-xsmi-address") == XML_PARSER_RET_CODE_OK) {
                    maxSize = sizeof(smiXsmiInfoPtr->publicInfo.secondaryAddress);
                    xmlStatus2 = xmlParserGetValue (xmlSecondaryAddressId, &maxSize, &smiXsmiInfoPtr->publicInfo.secondaryAddress);
                    XML_CHECK_STATUS(xmlStatus2);
                    smiXsmiInfoPtr->publicInfo.secondaryAddressSupported = TRUE;
                }
                else if (xmlParserIsEqualName (xmlSecondaryAddressId, "phy-smi-xsmi-address-role") == XML_PARSER_RET_CODE_OK) {
                    maxSize = PDL_XML_MAX_TAG_LEN;
                    xmlStatus2 = xmlParserGetValue (xmlSecondaryAddressId, &maxSize, &pdlValueStr[0]);
                    XML_CHECK_STATUS(xmlStatus2);
                    if (strcmp (pdlValueStr, "secondary") != 0) {
                        /* only secondary is currently supported in additional-smi-xsmi-address-list*/
                        return PDL_NOT_SUPPORTED;
                    }
                }
                else {
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus2 = xmlParserGetNextSibling(xmlSecondaryAddressId, &xmlSecondaryAddressId);
            }
        }
        else {
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling(xmlId, &xmlId);
    }

    /* verify mandatory fields */
    if (type == FALSE || id == FALSE || address == FALSE) {
        return PDL_XML_PARSE_ERROR;
    }
    return PDL_OK;
}

/*$ END OF prvPdlSmiXsmiXmlParser */

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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_INTERFACE_PRV_KEY_STC           smiXsmiKey;
    PDL_INTERFACE_PRV_SMI_XSMI_DB_STC   smiXsmiInfo, * smiXsmiPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfaceIdPtr == NULL || interfaceTypePtr == NULL) {
        return PDL_BAD_PTR;
    }
    pdlStatus = prvPdlSmiXsmiXmlAttributesParser (xmlId, &smiXsmiInfo);
    PDL_CHECK_STATUS(pdlStatus);
    smiXsmiKey.interfaceId = prvPdlSmiXsmiInterfaceCount++;
    pdlStatus = prvPdlDbAdd (&prvPdlSmiXsmiDb, (void*) &smiXsmiKey, (void*) &smiXsmiInfo, (void**) &smiXsmiPtr);
    PDL_CHECK_STATUS(pdlStatus);
    *interfaceIdPtr = smiXsmiKey.interfaceId;
    *interfaceTypePtr = smiXsmiInfo.publicInfo.interfaceType;
    return PDL_OK;

}

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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_INTERFACE_PRV_KEY_STC           smiXsmiKey;
    PDL_INTERFACE_PRV_SMI_XSMI_DB_STC   *smiXsmiPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfaceIdPtr == NULL) {
        return PDL_BAD_PTR;
    }
    smiXsmiKey.interfaceId = prvPdlSmiXsmiInterfaceCount++;
    pdlStatus = prvPdlDbAdd (&prvPdlSmiXsmiDb, (void*) &smiXsmiKey, (void*) smiXsmiInfoPtr, (void**) &smiXsmiPtr);
    PDL_CHECK_STATUS(pdlStatus);
    *interfaceIdPtr = smiXsmiKey.interfaceId;
    return PDL_OK;

}

/*$ END OF prvPdlSmiXsmiAddInterface */

/**
 * @fn  PDL_STATUS pdlSmiXsmiHwGetValue ( IN PDL_INTERFACE_TYP interfaceId, IN PDL_INTERFACE_GPIO_OFFSET_ENT offset, OUT UINT_32 * dataPtr )
 *
 * @brief   Pdl Smi/Xsmi hardware get value
 *
 * @param           interfaceId Identifier for the interface.
 * @param           offset      The offset.
 * @param [in,out]  dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSmiXsmiHwGetValue (
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_8                                  phyDeviceOrPage,
    IN  UINT_16                                 regAddress,
    OUT UINT_16                               * dataPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    UINT_16                                     xsmiData = 0;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_SMI_XSMI_DB_STC         * smiXsmiPtr;
    PDL_INTERFACE_PRV_KEY_STC                   smiXsmiKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    smiXsmiKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&prvPdlSmiXsmiDb, (void*) &smiXsmiKey, (void**) &smiXsmiPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (dataPtr == NULL) {
        return PDL_BAD_PTR;
    }
    switch (smiXsmiPtr->publicInfo.interfaceType) {
    
    case PDL_INTERFACE_TYPE_SMI_E:    	
        prvPdlLock(PDL_OS_LOCK_TYPE_SMI_E);        
        pdlStatus = prvPdlSmiRegRead (smiXsmiPtr->publicInfo.dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, smiXsmiPtr->publicInfo.interfaceId, smiXsmiPtr->publicInfo.address, 22, phyDeviceOrPage, (UINT_32) regAddress, dataPtr);
        if (pdlStatus != PDL_OK) {
            prvPdlUnlock(PDL_OS_LOCK_TYPE_SMI_E);            
        }        
        PDL_CHECK_STATUS(pdlStatus);
        prvPdlUnlock(PDL_OS_LOCK_TYPE_SMI_E);        
        break;
    case PDL_INTERFACE_TYPE_XSMI_E:
        prvPdlLock(PDL_OS_LOCK_TYPE_XSMI_E);
        pdlStatus = prvPdlXsmiRegRead(smiXsmiPtr->publicInfo.dev, smiXsmiPtr->publicInfo.interfaceId, smiXsmiPtr->publicInfo.address, regAddress, phyDeviceOrPage, &xsmiData);
        if (pdlStatus != PDL_OK) {
            prvPdlUnlock(PDL_OS_LOCK_TYPE_XSMI_E);
        }
        PDL_CHECK_STATUS(pdlStatus);
        prvPdlUnlock(PDL_OS_LOCK_TYPE_XSMI_E);
        *dataPtr = (UINT_32) xsmiData;
        break;
    default:
        return PDL_ERROR;
    }
    
    return PDL_OK;
}

/*$ END OF pdlSmiXsmiHwGetValue */

/**
 * @fn  PDL_STATUS pdlSmiXsmiHwSetValue ( IN PDL_INTERFACE_TYP interfaceId, IN PDL_INTERFACE_GPIO_OFFSET_ENT offset, OUT UINT_32 data )
 *
 * @brief   Pdl Smi/Xsmi hardware set value
 *
 * @param           interfaceId Identifier for the interface.
 * @param           offset      The offset.
 * @param [in,out]  dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSmiXsmiHwSetValue (
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_8                                  phyDeviceOrPage,
    IN  UINT_16                                 regAddress,
    IN  UINT_16                                 mask,
    OUT UINT_16                                 data
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/        
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_SMI_XSMI_DB_STC         * smiXsmiPtr;
    PDL_INTERFACE_PRV_KEY_STC                   smiXsmiKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    smiXsmiKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&prvPdlSmiXsmiDb, (void*) &smiXsmiKey, (void**) &smiXsmiPtr);
    PDL_CHECK_STATUS(pdlStatus);
    PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__,"%s - writing  %s dev[%d] interfaceId [%d] address[%d] phyDevOrPage[%d] regAddress[0x%x] mask[0x%x] data[0x%x]\n", __FUNCTION__, smiXsmiPtr->publicInfo.interfaceType==PDL_INTERFACE_TYPE_SMI_E ? "SMI":"XSMI", smiXsmiPtr->publicInfo.dev, smiXsmiPtr->publicInfo.interfaceId, smiXsmiPtr->publicInfo.address, phyDeviceOrPage, regAddress, mask, data);

    switch (smiXsmiPtr->publicInfo.interfaceType) {
    
    case PDL_INTERFACE_TYPE_SMI_E:    	
        prvPdlLock(PDL_OS_LOCK_TYPE_SMI_E);
        pdlStatus = prvPdlSmiRegWrite (smiXsmiPtr->publicInfo.dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, smiXsmiPtr->publicInfo.interfaceId, smiXsmiPtr->publicInfo.address, 22, phyDeviceOrPage, (UINT_32) regAddress, mask, data);
        if (pdlStatus != PDL_OK) {
            prvPdlUnlock(PDL_OS_LOCK_TYPE_SMI_E);            
        }        
        PDL_CHECK_STATUS(pdlStatus);
        prvPdlUnlock(PDL_OS_LOCK_TYPE_SMI_E);        
        break;
    case PDL_INTERFACE_TYPE_XSMI_E:
        prvPdlLock(PDL_OS_LOCK_TYPE_XSMI_E);
        pdlStatus = prvPdlXsmiRegWrite(smiXsmiPtr->publicInfo.dev, smiXsmiPtr->publicInfo.interfaceId, smiXsmiPtr->publicInfo.address, regAddress, phyDeviceOrPage, (UINT_16) mask, (UINT_16) data);
        if (pdlStatus != PDL_OK) {
            prvPdlUnlock(PDL_OS_LOCK_TYPE_XSMI_E);
        }
        PDL_CHECK_STATUS(pdlStatus);
        prvPdlUnlock(PDL_OS_LOCK_TYPE_XSMI_E);
        break;
    default:
        return PDL_ERROR;
    }   
    return PDL_OK;

}

/*$ END OF pdlSmiXsmiHwSetValue */

/**
 * @fn  PDL_STATUS pdlSmiXsmiCountGet ( OUT UINT_32 * countPtr )
 *
 * @brief   Gets number of smi/xsmi interfaces
 *
 * @param [out] countPtr Number of smi/xsmi interfaces
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlSmiXsmiCountGet (
    OUT UINT_32 * countPtr
)
{
/****************************************************************************/
/*L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/****************************************************************************/

/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    if (countPtr == NULL)
        return PDL_BAD_PTR;

    *countPtr = prvPdlSmiXsmiInterfaceCount;

    return PDL_OK;
}

/*$ END OF pdlSmiXsmiCountGet */

/* ***************************************************************************
* FUNCTION NAME: pdlSmiXsmiDebugDbGetAttributes
*
* DESCRIPTION:   get interface attributes from DB
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS pdlSmiXsmiDebugDbGetAttributes (
    IN  PDL_INTERFACE_TYP                    interfaceId,
    OUT PDL_INTERFACE_SMI_XSMI_STC          * attributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_SMI_XSMI_DB_STC         * smiXsmiPtr;
    PDL_INTERFACE_PRV_KEY_STC                   smiXsmiKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    smiXsmiKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&prvPdlSmiXsmiDb, (void*) &smiXsmiKey, (void**) &smiXsmiPtr);
    PDL_CHECK_STATUS (pdlStatus);
    if (attributesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    memcpy (attributesPtr, &smiXsmiPtr->publicInfo, sizeof (PDL_INTERFACE_SMI_XSMI_STC));
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8,  smiXsmiPtr->publicInfo.address);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8,  smiXsmiPtr->publicInfo.dev);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8,  smiXsmiPtr->publicInfo.interfaceId);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8,  smiXsmiPtr->publicInfo.interfaceType);
    return PDL_OK;
}
/*$ END OF pdlSmiXsmiDebugDbGetAttributes */

/* ***************************************************************************
* FUNCTION NAME: pdlSmiXsmiDebugDbSetAttributes
*
* DESCRIPTION:   set interface attributes from DB
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS pdlSmiXsmiDebugDbSetAttributes (
    IN  PDL_INTERFACE_TYP                    interfaceId,
    IN  UINT_32                              attributesMask,
    OUT PDL_INTERFACE_SMI_XSMI_STC         * attributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_SMI_XSMI_DB_STC         * smiXsmiPtr;
    PDL_INTERFACE_PRV_KEY_STC                   smiXsmiKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    smiXsmiKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&prvPdlSmiXsmiDb, (void*) &smiXsmiKey, (void**) &smiXsmiPtr);
    PDL_CHECK_STATUS (pdlStatus);
    if (attributesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    if (attributesMask & PDL_SMI_XSMI_SET_ATTRIBUTE_MASK_DEVICE) {
        smiXsmiPtr->publicInfo.dev = attributesPtr->dev;
    }
    if (attributesMask & PDL_SMI_XSMI_SET_ATTRIBUTE_MASK_INTERFACE_ID) {
        smiXsmiPtr->publicInfo.interfaceId = attributesPtr->interfaceId;
    }
    if (attributesMask & PDL_SMI_XSMI_SET_ATTRIBUTE_MASK_ADDRESS) {
        smiXsmiPtr->publicInfo.address = attributesPtr->address;
    }
    return PDL_OK;
}
/*$ END OF pdlSmiXsmiDebugDbSetAttributes */

/**
 * @fn  PDL_STATUS pdlSmiXsmiInit ( IN void )
 *
 * @brief   Pdl SMI/XSMI initialize
 *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSmiXsmiInit (
    IN  void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    PDL_STATUS                      pdlStatus;
    PDL_DB_PRV_ATTRIBUTES_STC       dbAttributes;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    dbAttributes.listAttributes.keySize = sizeof(PDL_INTERFACE_PRV_KEY_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_INTERFACE_PRV_GPIO_DB_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &prvPdlSmiXsmiDb);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF pdlSmiXsmiInit */

/**
 * @fn  PDL_STATUS prvPdlSmiXsmiDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSmiXsmiDestroy (
    void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlSmiXsmiInterfaceCount = 0;
    if (prvPdlSmiXsmiDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&prvPdlSmiXsmiDb);
        PDL_CHECK_STATUS(pdlStatus);
    }
    return PDL_OK;
}

/*$ END OF prvPdlSmiXsmiDestroy */

