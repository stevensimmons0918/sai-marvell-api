/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlInterface.c   
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
 * @brief Platform driver layer - Interface related API
 * 
 * @version   1 
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/interface/private/prvPdlGpio.h>
#include <pdl/interface/private/prvPdlI2c.h>
#include <pdl/interface/private/prvPdlSmiXsmi.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/interface/pdlSmiXsmiDebug.h>
#include <pdl/xml/private/prvXmlParser.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/


/** @brief   The interface type string to enum pairs */
static PRV_PDL_LIB_STR_TO_ENUM_STC prvPdlInterfaceTypeStrToEnumPairs[] = {
    {"i2c"      , PDL_INTERFACE_TYPE_I2C_E },
    {"smi"      , PDL_INTERFACE_TYPE_SMI_E  },   
    {"xsmi"     , PDL_INTERFACE_TYPE_XSMI_E },  
    {"gpio"     , PDL_INTERFACE_TYPE_GPIO_E },  
    {"smi_cpu"  , PDL_INTERFACE_TYPE_SMI_CPU_E },
    {"external" , PDL_INTERFACE_TYPE_EXTERNAL_E }
};
/** @brief   The interface type string to enum */
PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC prvPdlInterfaceTypeStrToEnum = {prvPdlInterfaceTypeStrToEnumPairs, sizeof(prvPdlInterfaceTypeStrToEnumPairs)/sizeof(PRV_PDL_LIB_STR_TO_ENUM_STC)};

/* ***************************************************************************
* FUNCTION NAME: prvPdlInterfaceXmlIsInterface
*
* DESCRIPTION:  returns TRUE if xmlId is pointing to a GPIO interface
*
* PARAMETERS:   
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlInterfaceXmlIsInterface ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT BOOLEAN * isInterface )
 *
 * @brief   Prv pdl interface XML is interface
 *
 * @param           xmlId       Identifier for the XML.
 * @param [in,out]  isInterface If non-null, true if is interface, false if not.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlInterfaceXmlIsInterface (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    OUT BOOLEAN                               * isInterface
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    BOOLEAN                             isGpio, isI2c;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (isInterface == NULL) {
        return PDL_BAD_PTR;
    }
    pdlStatus = prvPdlGpioXmlIsGpio(xmlId, &isGpio);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlI2cXmlIsI2c(xmlId, &isI2c);
    PDL_CHECK_STATUS(pdlStatus);

    *isInterface = FALSE;
    if (isGpio) {
        *isInterface = TRUE;
    }
    if (isI2c) {
        *isInterface = TRUE;
    }

    return PDL_OK;
}
/*$ END OF prvPdlInterfaceXmlIsInterface */


/* ***************************************************************************
* FUNCTION NAME: prvPdlInterfaceXmlParser
*
* DESCRIPTION:  parse XML instance of interface, insert it into DB and return a reference to it
*
* PARAMETERS:   
*
* RETURNES:     PDL_OK                      -       SUCCESS
*               PDL_BAD_PTR                 -       NULL pointers as param
*               PDL_XML_PARSE_ERROR         -       xmlId isn't pointing to an interface
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlInterfaceXmlParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT PDL_INTERFACE_TYPE_ENT * interfaceTypePtr, OUT PDL_INTERFACE_TYP * interfaceIdPtr )
 *
 * @brief   Prv pdl interface XML parser
 *
 * @param           xmlId               Identifier for the XML.
 * @param [in,out]  interfaceTypePtr    If non-null, the interface type pointer.
 * @param [in,out]  interfaceIdPtr      If non-null, the interface identifier pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlInterfaceXmlParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    OUT PDL_INTERFACE_TYPE_ENT            * interfaceTypePtr,
    OUT PDL_INTERFACE_TYP                 * interfaceIdPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    BOOLEAN                             isGpio, isI2c, isSmiXsmi;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfaceTypePtr == NULL || interfaceIdPtr == NULL) {
        return PDL_BAD_PTR;
    }

    pdlStatus = prvPdlGpioXmlIsGpio(xmlId, &isGpio);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlI2cXmlIsI2c(xmlId, &isI2c);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlSmiXsmiXmlIsSmiXsmi(xmlId, &isSmiXsmi);
    PDL_CHECK_STATUS(pdlStatus);

    if (isGpio) {
        *interfaceTypePtr = PDL_INTERFACE_TYPE_GPIO_E;
        return prvPdlGpioXmlParser (xmlId, interfaceIdPtr);
    }
    else if (isI2c) {
        *interfaceTypePtr = PDL_INTERFACE_TYPE_I2C_E;
        return prvPdlI2cXmlParser (xmlId, interfaceIdPtr);
    }
    else if (isSmiXsmi) {
        return prvPdlSmiXsmiXmlParser (xmlId, interfaceIdPtr, interfaceTypePtr);
    }
    else {
        return PDL_XML_PARSE_ERROR;
    }

    return PDL_XML_PARSE_ERROR;
}

/*$ END OF prvPdlInterfaceXmlParser */

/* ***************************************************************************
* FUNCTION NAME: prvPdlInterfaceXmlIsValue
*
* DESCRIPTION:  returns TRUE if xmlId is pointing to a interface value
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS prvPdlInterfaceXmlIsValue (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    OUT BOOLEAN                               * isValuePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                             maxSize, value;    
    XML_PARSER_RET_CODE_TYP             xmlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (isValuePtr == NULL) {
        return PDL_BAD_PTR;
    }

    if ((xmlParserIsEqualName (xmlId, "i2c-read-value") == XML_PARSER_RET_CODE_OK) ||
        (xmlParserIsEqualName (xmlId, "i2c-write-value") == XML_PARSER_RET_CODE_OK)){
        *isValuePtr = TRUE;
    }
    else if ((xmlParserIsEqualName (xmlId, "gpio-write-value") == XML_PARSER_RET_CODE_OK) ||
             (xmlParserIsEqualName (xmlId, "gpio-read-value") == XML_PARSER_RET_CODE_OK) || 
             (xmlParserIsEqualName (xmlId, "led-pin-value") == XML_PARSER_RET_CODE_OK)) {
        maxSize = sizeof(value);
        xmlStatus = xmlParserGetValue (xmlId, &maxSize, &value);
        XML_CHECK_STATUS(xmlStatus);
        if (value != 0 && value != 1) {
            return PDL_OUT_OF_RANGE;
        }
        *isValuePtr = TRUE;
    }
    else {
        *isValuePtr = FALSE;
    }
    
    return PDL_OK;
}
/*$ END OF prvPdlInterfaceXmlIsValue */

/* ***************************************************************************
* FUNCTION NAME: prvPdlInterfaceXmlParser
*
* DESCRIPTION:  parse XML instance of interface, & value. insert interface into DB and return a reference to it and to value
*
* PARAMETERS:   
*
* RETURNES:     PDL_OK                      -       SUCCESS
*               PDL_BAD_PTR                 -       NULL pointers as param
*               PDL_XML_PARSE_ERROR         -       xmlId isn't pointing to an interface
*                
*****************************************************************************/

PDL_STATUS prvPdlInterfaceAndValueXmlParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    IN  UINT_32                             sizeOfValue,
    OUT PDL_INTERFACE_TYPE_ENT            * interfaceTypePtr,
    OUT PDL_INTERFACE_TYP                 * interfaceIdPtr,
    OUT void                              * valuePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    BOOLEAN                                 isValue, isInterface;
    PDL_STATUS                              pdlStatus;
    XML_PARSER_RET_CODE_TYP                 xmlStatus;   
    XML_PARSER_NODE_DESCRIPTOR_TYP          xmlInterfaceId;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfaceTypePtr == NULL || interfaceIdPtr == NULL || valuePtr == NULL) {
        return PDL_BAD_PTR;
    }
    xmlStatus = xmlParserGetFirstChild (xmlId, &xmlInterfaceId);
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);
        pdlStatus = prvPdlInterfaceXmlIsInterface(xmlInterfaceId, &isInterface);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlInterfaceXmlIsValue (xmlInterfaceId, &isValue);
        PDL_CHECK_STATUS(pdlStatus);
        if (isInterface) {
            pdlStatus = prvPdlInterfaceXmlParser (xmlInterfaceId, interfaceTypePtr, interfaceIdPtr);
            PDL_CHECK_STATUS(pdlStatus);
        }        
        else if (isValue) {
            xmlStatus = xmlParserGetValue (xmlInterfaceId, &sizeOfValue, valuePtr);
            XML_CHECK_STATUS(xmlStatus);
        }
        else {
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling (xmlInterfaceId, &xmlInterfaceId);
    }

    return PDL_OK;
}

/*$ END OF prvPdlInterfaceAndValueXmlParser */

/* ***************************************************************************
* FUNCTION NAME: prvPdlInterfaceHwGetValue
*
* DESCRIPTION:  get Interface value
*
* PARAMETERS:   
*              
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlInterfaceHwGetValue ( IN PDL_INTERFACE_TYPE_ENT interfaceType, IN PDL_INTERFACE_TYP interfaceId, OUT UINT_32 * dataPtr )
 *
 * @brief   Prv pdl interface hardware get value
 *
 * @param           interfaceType   Type of the interface.
 * @param           interfaceId     Identifier for the interface.
 * @param [in,out]  dataPtr         If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlInterfaceHwGetValue (
    IN  PDL_INTERFACE_TYPE_ENT                   interfaceType,    
    IN  PDL_INTERFACE_TYP                        interfaceId,
    OUT UINT_32                                * dataPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    switch (interfaceType) {
    case PDL_INTERFACE_TYPE_GPIO_E:
        return prvPdlGpioHwGetValue (interfaceId, PDL_INTERFACE_GPIO_OFFSET_INPUT_E, dataPtr);
        break;
    case PDL_INTERFACE_TYPE_I2C_E:
        return prvPdlI2CHwGetValue (interfaceId, dataPtr);
        break;
    /* SMI/XSMI should be used with API */
    case PDL_INTERFACE_TYPE_SMI_E:
    case PDL_INTERFACE_TYPE_XSMI_E:
        return PDL_ERROR;
        break;
    default:
        return PDL_ERROR;
    }
    return PDL_OK;
}

/*$ END OF prvPdlInterfaceHwGetValue */

/**
 * @fn  PDL_STATUS pdlInterfaceDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Interface debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlInterfaceDebugSet (
    IN  BOOLEAN             state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlInterfaceDebugFlag = state;
    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlI2CInterfaceMuxDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl I2C Interface Mux debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlI2CInterfaceMuxDebugSet (
    IN  BOOLEAN             state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlI2CInterfaceMuxDebugFlag = state;
    return PDL_OK;
}

/* ***************************************************************************
* FUNCTION NAME: prvPdlInterfaceHwSetValue
*
* DESCRIPTION:  set Interface value
*
* PARAMETERS:   
*              
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlInterfaceHwSetValue ( IN PDL_INTERFACE_TYPE_ENT interfaceType, IN PDL_INTERFACE_TYP interfaceId, OUT UINT_32 data )
 *
 * @brief   Prv pdl interface hardware set value
 *
 * @param   interfaceType   Type of the interface.
 * @param   interfaceId     Identifier for the interface.
 * @param   data            The data.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlInterfaceHwSetValue (
    IN  PDL_INTERFACE_TYPE_ENT                   interfaceType,    
    IN  PDL_INTERFACE_TYP                        interfaceId,
    OUT UINT_32                                  data
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    switch (interfaceType) {
    case PDL_INTERFACE_TYPE_GPIO_E:
        return prvPdlGpioHwSetValue (interfaceId, PDL_INTERFACE_GPIO_OFFSET_INPUT_E, data);
        break;
    case PDL_INTERFACE_TYPE_I2C_E:
        return prvPdlI2CHwSetValue (interfaceId, data);
        break;
    default:
        return PDL_ERROR;
    }
    return PDL_OK;
}
/*$ END OF prvPdlInterfaceHwSetValue */

/**
 * @fn  PDL_STATUS pdlInterfaceCountGet ( IN PDL_INTERFACE_TYPE_ENT type, OUT UINT_32 * countPtr )
 *
 * @brief   Gets number of interfaces
 *
 * @param [in] type      interface type
 * @param [out] countPtr Number of smi/xsmi interfaces
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlInterfaceCountGet (
    IN PDL_INTERFACE_TYPE_ENT   type,
    OUT UINT_32                *countPtr
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

    switch (type)
    {
        case PDL_INTERFACE_TYPE_GPIO_E:
            return prvPdlGpioCountGet(countPtr);
        case PDL_INTERFACE_TYPE_I2C_E:
            return prvPdlI2cCountGet(countPtr);
        case PDL_INTERFACE_TYPE_SMI_E:
        case PDL_INTERFACE_TYPE_XSMI_E:
            return pdlSmiXsmiCountGet(countPtr);
        case PDL_INTERFACE_TYPE_SMI_CPU_E:
        default:
            return PDL_BAD_PARAM;
    }

    return PDL_OK;
}
