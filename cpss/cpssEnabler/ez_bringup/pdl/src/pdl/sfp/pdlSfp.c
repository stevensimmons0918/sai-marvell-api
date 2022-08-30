/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlSfp.c   
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
 * @brief Platform driver layer - SFP related API
 * 
 * @version   1 
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/sfp/pdlSfp.h>
#include <pdl/sfp/private/prvPdlSfp.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <pdl/sfp/pdlSfpDebug.h>
#include <pdl/interface/private/prvPdlI2c.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/** @brief   The pdl sfp database */
static PDL_DB_PRV_STC   pdlSfpDb = {0,NULL};

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)                                                                  \
    if (__pdlStatus != PDL_OK) {                                                                       \
    if (prvPdlSfpDebugFlag) {                                                                          \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__pdlStatus,PDL_STATUS_ERROR_PRINT_IDENTIFIER);      \
    }                                                                                                  \
    return __pdlStatus;                                                                                \
    }  
/*macro to check the xml status and debug prints if debug flag on*/
#undef XML_CHECK_STATUS
#define XML_CHECK_STATUS(__xmlStatus)                                                                   \
    if (__xmlStatus != PDL_OK) {                                                                        \
    if (prvPdlSfpDebugFlag) {                                                                           \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__xmlStatus,XML_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __xmlStatus;                                                                                 \
    }  


/** @brief   The interface type string to enum pairs */
static PRV_PDL_LIB_STR_TO_ENUM_STC prvPdSfpValueTypeStrToEnumPairs[] = {
    {"eeprom"       , PDL_SFP_VALUE_EEPROM_E },
    {"loss"         , PDL_SFP_VALUE_LOSS_E  },
    {"present"      , PDL_SFP_VALUE_PRESENT_E },
    {"tx_enable"    , PDL_SFP_VALUE_TX_ENABLE_E },
    {"tx_disable"   , PDL_SFP_VALUE_TX_DISABLE_E }
};
/** @brief   The interface type string to enum */
PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC prvPdSfpValueTypeStrToEnum = {prvPdSfpValueTypeStrToEnumPairs, sizeof(prvPdSfpValueTypeStrToEnumPairs)/sizeof(PRV_PDL_LIB_STR_TO_ENUM_STC)};

/* ***************************************************************************
* FUNCTION NAME: pdlSfpHwTxGet
*
* DESCRIPTION:   Get sfp operational status (tx enable/disable)
*
* PARAMETERS:   
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlSfpHwTxGet ( IN UINT_32 dev, IN UINT_32 logicalPort, OUT PDL_SFP_TX_ENT * statusPtr )
 *
 * @brief   Pdl sfp hardware transmit get
 *
 * @param           dev         The device number.
 * @param           port        The port.
 * @param [in,out]  statusPtr   If non-null, the status pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSfpHwTxGet (
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    OUT PDL_SFP_TX_ENT            * statusPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                             data;
    PDL_STATUS                          pdlStatus;
    PDL_SFP_PRV_DB_STC 	              * sfpPtr;
    PDL_SFP_PRV_KEY_STC                 sfpKey;

/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

    if (statusPtr == NULL) {
         PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"statusPtr POINTER NULL, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort ,dev);
        return PDL_BAD_PTR;
    }
    sfpKey.dev = dev;
    sfpKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&pdlSfpDb, (void*) &sfpKey, (void*) &sfpPtr);
    PDL_CHECK_STATUS (pdlStatus);
    pdlStatus = prvPdlInterfaceHwGetValue (sfpPtr->publicInfo.txEnableInfo.interfaceType, sfpPtr->publicInfo.txEnableInfo.interfaceId, &data);
    PDL_CHECK_STATUS (pdlStatus);
	PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"[dev %d port %d] read %x expected %x",dev, logicalPort, data, sfpPtr->publicInfo.values.txEnableValue);
    if (data == sfpPtr->publicInfo.values.txEnableValue) {
        *statusPtr = PDL_SFP_TX_ENABLE_E;
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"[dev %d port %d] read %x expected %x",dev, logicalPort, data, sfpPtr->publicInfo.values.txDisableValue);
        *statusPtr = PDL_SFP_TX_DISABLE_E;
    }
    return PDL_OK;
}

/*$ END OF pdlSfpHwTxGet */

/* ***************************************************************************
* FUNCTION NAME: pdlSfpHwTxSet
*
* DESCRIPTION:   set sfp operational status (tx enable/disable)
*
* PARAMETERS:   
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlSfpHwTxSet ( IN UINT_32 dev, IN UINT_32 logicalPort, IN PDL_SFP_TX_ENT status )
 *
 * @brief   Pdl sfp hardware transmit set
 *
 * @param   dev     the device number.
 * @param   port    The port.
 * @param   status  The status.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSfpHwTxSet (
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    IN PDL_SFP_TX_ENT               status
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_SFP_PRV_DB_STC 	              * sfpPtr;
    PDL_SFP_PRV_KEY_STC                 sfpKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    sfpKey.dev = dev;
    sfpKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&pdlSfpDb, (void*) &sfpKey, (void*) &sfpPtr);
    PDL_CHECK_STATUS (pdlStatus);
    if (status == PDL_SFP_TX_ENABLE_E) {
        pdlStatus = prvPdlInterfaceHwSetValue (sfpPtr->publicInfo.txEnableInfo.interfaceType, sfpPtr->publicInfo.txEnableInfo.interfaceId, sfpPtr->publicInfo.values.txEnableValue);
    }
    else {
        pdlStatus = prvPdlInterfaceHwSetValue (sfpPtr->publicInfo.txDisableInfo.interfaceType, sfpPtr->publicInfo.txDisableInfo.interfaceId, sfpPtr->publicInfo.values.txDisableValue);    
    }
    PDL_CHECK_STATUS (pdlStatus);
    return PDL_OK;
}

/*$ END OF pdlSfpHwTxSet */

/* ***************************************************************************
* FUNCTION NAME: pdlSfpHwLossGet
*
* DESCRIPTION:   Get sfp loss status
*
* PARAMETERS:   
*                
*****************************************************************************/

/*$ END OF PdlSfpHwTxSet */

/**
 * @fn  PDL_STATUS pdlSfpHwLossGet ( IN UINT_32 dev, IN UINT_32 logicalPort, OUT PDL_SFP_LOSS_ENT * statusPtr )
 *
 * @brief   Pdl sfp hardware loss get
 *
 * @param           dev         the device number.
 * @param           port        The port.
 * @param [in,out]  statusPtr   If non-null, the status pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSfpHwLossGet (
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    OUT PDL_SFP_LOSS_ENT          * statusPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                             data = 0;
    PDL_STATUS                          pdlStatus;
    PDL_SFP_PRV_DB_STC 	              * sfpPtr;
    PDL_SFP_PRV_KEY_STC                 sfpKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (statusPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL statusPtr, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort,dev);
        return PDL_BAD_PTR;
    }
    sfpKey.dev = dev;
    sfpKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&pdlSfpDb, (void*) &sfpKey, (void*) &sfpPtr);
    PDL_CHECK_STATUS (pdlStatus);
    pdlStatus = prvPdlInterfaceHwGetValue (sfpPtr->publicInfo.lossInfo.interfaceType, sfpPtr->publicInfo.lossInfo.interfaceId, &data);
    PDL_CHECK_STATUS (pdlStatus);
    PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"[dev %d port %d] read %x expected %x",dev, logicalPort, data, sfpPtr->publicInfo.values.lossValue);
    if (data == sfpPtr->publicInfo.values.lossValue) {
        *statusPtr = PDL_SFP_LOSS_TRUE_E;
    }
    else {
        *statusPtr = PDL_SFP_LOSS_FALSE_E;
    }
    return PDL_OK;
}

/*$ END OF pdlSfpHwLossGet */

/* ***************************************************************************
* FUNCTION NAME: pdlSfpHwPresentGet
*
* DESCRIPTION:   Get sfp present status
*
* PARAMETERS:   
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlSfpHwPresentGet ( IN UINT_32 dev, IN UINT_32 logicalPort, OUT PDL_SFP_PRESENT_ENT * statusPtr )
 *
 * @brief   Pdl sfp hardware present get
 *
 * @param           dev         the device number.
 * @param           port        The port.
 * @param [in,out]  statusPtr   If non-null, the status pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSfpHwPresentGet (
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    OUT PDL_SFP_PRESENT_ENT       * statusPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                             data = 0;
    PDL_STATUS                          pdlStatus;
    PDL_SFP_PRV_DB_STC 	              * sfpPtr;
    PDL_SFP_PRV_KEY_STC                 sfpKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (statusPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL statusPtr, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort,dev);
        return PDL_BAD_PTR;
    }
    sfpKey.dev = dev;
    sfpKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&pdlSfpDb, (void*) &sfpKey, (void*) &sfpPtr);
    PDL_CHECK_STATUS (pdlStatus);
    pdlStatus = prvPdlInterfaceHwGetValue (sfpPtr->publicInfo.presentInfo.interfaceType, sfpPtr->publicInfo.presentInfo.interfaceId, &data);
    PDL_CHECK_STATUS (pdlStatus);
    PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"[dev %d port %d] read %x expected %x",dev, logicalPort, data, sfpPtr->publicInfo.values.presentValue);
    if (data == sfpPtr->publicInfo.values.presentValue) {
        *statusPtr = PDL_SFP_PRESENT_TRUE_E;
    }
    else {
        *statusPtr = PDL_SFP_PRESENT_FALSE_E;
    }
    return PDL_OK;
}

/*$ END OF pdlSfpHwPresentGet */

/* ***************************************************************************
* FUNCTION NAME: pdlSfpHwEepromRead
*
* DESCRIPTION:   Read sfp eeprom
*
* PARAMETERS:   
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlSfpHwEepromRead ( IN UINT_32 dev, IN UINT_32 logicalPort, IN UINT_16 offset, IN UINT_32 length, OUT void * dataPtr )
 *
 * @brief   Pdl sfp hardware eeprom read
 *
 * @param           dev     the device number.
 * @param           port            The port.
 * @param [in]      i2cAddress      i2c address to read from MUST be 0x50 or 0x51
 * @param           offset          The offset.
 * @param           length          The length.
 * @param [in,out]  dataPtr         If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSfpHwEepromRead (
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    IN  UINT_8                      i2cAddress,
    IN  UINT_16                     offset,
    IN  UINT_32                     length,
    OUT void                      * dataPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_SFP_PRV_DB_STC 	              * sfpPtr;
    PDL_SFP_PRV_KEY_STC                 sfpKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (dataPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL statusPtr, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort,dev);
        return PDL_BAD_PTR;
    }
    sfpKey.dev = dev;
    sfpKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&pdlSfpDb, (void*) &sfpKey, (void*) &sfpPtr);
    PDL_CHECK_STATUS (pdlStatus);
    pdlStatus = pdlI2CHwGetBufferWithOffset (sfpPtr->publicInfo.eepromInfo.interfaceId, i2cAddress, offset, length, dataPtr);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF pdlSfpHwEepromRead */

/* ***************************************************************************
* FUNCTION NAME: prvPdlSfpInfoParser
*
* DESCRIPTION:   Get pointer to laserIsLossInfo/gbicConnectedInfo/txIsEnabledControl/txIsDisabledControl and parse it
*
* RETURN VALUE:  interface type, interface id and value
*
* PARAMETERS:   
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlSfpInfoParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, IN UINT_32 valueSize, OUT PDL_INTERFACE_TYPE_ENT * interfaceTypePtr, OUT PDL_INTERFACE_TYP * interfaceIdPtr, OUT UINT_32 * valuePtr )
 *
 * @brief   Prv pdl sfp information parser
 *
 * @param           xmlId               Identifier for the XML.
 * @param           valueSize           Size of the value.
 * @param [in,out]  interfaceTypePtr    If non-null, the interface type pointer.
 * @param [in,out]  interfaceIdPtr      If non-null, the interface identifier pointer.
 * @param [in,out]  valuePtr            If non-null, the value pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSfpInfoParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    IN  UINT_32                                 valueSize,
    OUT PDL_INTERFACE_TYPE_ENT                * interfaceTypePtr,
    OUT PDL_INTERFACE_TYP                     * interfaceIdPtr,
    OUT UINT_32                               * valuePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    XML_PARSER_RET_CODE_TYP                     xmlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP              xmlInterfaceId;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfaceIdPtr == NULL || interfaceTypePtr == NULL || valuePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL POINTER(valuePtr OR interfaceTypePtr OR interfaceIdPtr)");
        return PDL_BAD_PTR;
    }
    xmlStatus = xmlParserGetFirstChild (xmlId, &xmlInterfaceId);
    XML_CHECK_STATUS(xmlStatus);
    /* skip read-interface-select */
    if ((xmlParserIsEqualName (xmlInterfaceId, "read-interface-select") == XML_PARSER_RET_CODE_OK) || 
        (xmlParserIsEqualName (xmlInterfaceId, "write-interface-select") == XML_PARSER_RET_CODE_OK)){
        xmlStatus = xmlParserGetNextSibling (xmlInterfaceId, &xmlInterfaceId);
        XML_CHECK_STATUS(xmlStatus);
    }       

    /* if gpio-group or i2c-group container skip to interface container*/
    if ((xmlParserIsEqualName (xmlInterfaceId, "read-i2c-interface")   == XML_PARSER_RET_CODE_OK) ||
        (xmlParserIsEqualName (xmlInterfaceId, "write-i2c-interface")  == XML_PARSER_RET_CODE_OK) ||
        (xmlParserIsEqualName (xmlInterfaceId, "read-gpio-interface")  == XML_PARSER_RET_CODE_OK) ||
        (xmlParserIsEqualName (xmlInterfaceId, "write-gpio-interface") == XML_PARSER_RET_CODE_OK)) {
        pdlStatus = prvPdlInterfaceAndValueXmlParser(xmlInterfaceId, valueSize, interfaceTypePtr, interfaceIdPtr, valuePtr);
        PDL_CHECK_STATUS(pdlStatus);
    }      
    else {
        /* couldn't find interface information */
        return PDL_NOT_FOUND;
    }
    return PDL_OK;
}

/*$ END OF prvPdlSfpInfoParser */

/* ***************************************************************************
* FUNCTION NAME: prvPdlSfpXmlParser
*
* DESCRIPTION:   Parse SFP section from XML
*
* PARAMETERS:   
*
* XML structure:
*<gbic>
*   <gbicConnectedInfo>
*       <gpio-group>
*           <gpio-read-only-interface>
*	            <device-number type=gpio-device-number-type></device-number>
*		        <pin-number type=gpio-pin-number-type></pin-number>
*	        </gpio-read-only-interface>
*       </gpio-group>
*	    <value></value>			
*   </gbicConnectedInfo>
*   <laserIsLossInfo>
*       <i2c-group>
*			<i2c-interface>
*               <bus-id type=i2c-bus-id-type></bus-id>
*			    <address type=i2c-address-type></address>				
*			    <offset type=i2c-offset-type></offset>
*			    <access type=i2c-access-type></access>
*			    <mask type=i2c-mask-value-type></mask>
*			</i2c-interface>
*           <value></value>
*       </i2c-group>
*   </laserIsLossInfo>
*
*   <txIsEnabledControl>
*       <gpio-group>
*           <gpio-read-only-interface>
*	            <device-number type=gpio-device-number-type></device-number>
*		        <pin-number type=gpio-pin-number-type></pin-number>
*	        </gpio-read-only-interface>
*	    <value></value>			
*       <gpio-group>
*   </txIsEnabledControl>
*
*   <txIsDisabledControl>
*       <gpio-group>
*           <gpio-read-only-interface>
*	            <device-number type=gpio-device-number-type></device-number>
*		        <pin-number type=gpio-pin-number-type></pin-number>
*	        </gpio-read-only-interface>
*	    <value></value>			
*       <gpio-group>
*   </txIsDisabledControl>
*</gbic>
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlSfpXmlParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, IN UINT_32 dev, IN UINT_32 port )
 *
 * @brief   Prv pdl sfp XML parser
 *
 * @param   xmlId   Identifier for the XML.
 * @param   dev     the device number.
 * @param   port    The port.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSfpXmlParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    IN  UINT_32                                 dev,
    IN  UINT_32                                 logicalPort
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    XML_PARSER_RET_CODE_TYP                     xmlStatus;
    PDL_SFP_PRV_DB_STC 	                        sfpInfo;
    PDL_SFP_PRV_DB_STC 	                      * sfpPtr = NULL;
    PDL_SFP_PRV_KEY_STC                         sfpKey;
    XML_PARSER_NODE_DESCRIPTOR_TYP              xmlSfpChildId, xmlEepromInfoId;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    /* xmliD not pointing to gbic */
    if (xmlParserIsEqualName (xmlId, "gbic-group") != XML_PARSER_RET_CODE_OK) {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NAME OF XML NODE DON'T MATCH: gbic-group, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort ,dev);
        return PDL_ERROR;
    }
    memset (&sfpInfo, 0, sizeof (sfpInfo));
    xmlStatus = xmlParserGetFirstChild (xmlId, &xmlSfpChildId);
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);
        if (xmlParserIsEqualName (xmlSfpChildId, "gbicConnectedInfo") == XML_PARSER_RET_CODE_OK) {
            pdlStatus = prvPdlSfpInfoParser (xmlSfpChildId, sizeof (sfpInfo.publicInfo.values.presentValue), &sfpInfo.publicInfo.presentInfo.interfaceType, &sfpInfo.publicInfo.presentInfo.interfaceId, &sfpInfo.publicInfo.values.presentValue);
            PDL_CHECK_STATUS(pdlStatus);
            sfpInfo.publicInfo.presentInfo.isSupported = TRUE;
        }
        else if (xmlParserIsEqualName (xmlSfpChildId, "laserIsLossInfo") == XML_PARSER_RET_CODE_OK) {
            pdlStatus = prvPdlSfpInfoParser (xmlSfpChildId, sizeof (sfpInfo.publicInfo.values.lossValue), &sfpInfo.publicInfo.lossInfo.interfaceType, &sfpInfo.publicInfo.lossInfo.interfaceId, &sfpInfo.publicInfo.values.lossValue);
            PDL_CHECK_STATUS(pdlStatus);
            sfpInfo.publicInfo.lossInfo.isSupported = TRUE;
        }
        else if (xmlParserIsEqualName (xmlSfpChildId, "txIsEnabledControl") == XML_PARSER_RET_CODE_OK) {
            pdlStatus = prvPdlSfpInfoParser (xmlSfpChildId, sizeof (sfpInfo.publicInfo.values.txEnableValue), &sfpInfo.publicInfo.txEnableInfo.interfaceType, &sfpInfo.publicInfo.txEnableInfo.interfaceId, &sfpInfo.publicInfo.values.txEnableValue);
            PDL_CHECK_STATUS(pdlStatus);
            sfpInfo.publicInfo.txEnableInfo.isSupported = TRUE;
        }
        else if (xmlParserIsEqualName (xmlSfpChildId, "txIsDisabledControl") == XML_PARSER_RET_CODE_OK) {
            pdlStatus = prvPdlSfpInfoParser (xmlSfpChildId, sizeof (sfpInfo.publicInfo.values.txDisableValue), &sfpInfo.publicInfo.txDisableInfo.interfaceType, &sfpInfo.publicInfo.txDisableInfo.interfaceId, &sfpInfo.publicInfo.values.txDisableValue);
            PDL_CHECK_STATUS(pdlStatus);
            sfpInfo.publicInfo.txDisableInfo.isSupported = TRUE;
        }
        else if (xmlParserIsEqualName (xmlSfpChildId, "memoryControl") == XML_PARSER_RET_CODE_OK) {
            xmlStatus = xmlParserGetFirstChild (xmlSfpChildId, &xmlEepromInfoId);
            XML_CHECK_STATUS(xmlStatus);
            /* in case value tag exists, skip it (it's irrelevant to memoryControl interface) */
            if (xmlParserIsEqualName (xmlEepromInfoId, "value") == XML_PARSER_RET_CODE_OK) {
                xmlStatus = xmlParserGetNextSibling(xmlEepromInfoId, &xmlEepromInfoId);
            }
            xmlStatus = prvPdlInterfaceXmlParser (xmlEepromInfoId, &sfpInfo.publicInfo.eepromInfo.interfaceType, &sfpInfo.publicInfo.eepromInfo.interfaceId);
            XML_CHECK_STATUS(xmlStatus);
            sfpInfo.publicInfo.eepromInfo.isSupported = TRUE;
        }
        else {
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling(xmlSfpChildId, &xmlSfpChildId);
    }
    sfpKey.dev = dev;
    sfpKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbAdd (&pdlSfpDb, (void*) &sfpKey, (void*) &sfpInfo, (void**) &sfpPtr);     
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;

}
/*$ END OF prvPdlSfpXmlParser */




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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlSfpDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlSfpDebugSet */


/**
 * @fn  PDL_STATUS PdlSfpDebugInterfaceGet ( IN UINT_32 dev, IN UINT_32 logicalPort, OUT PDL_SFP_INTERFACE_STC * interfacePtr );
 *
 * @brief   Get sfp access information
 *          @note
 *          Based on data retrieved from XML
 *
 * @param [in]  dev             - dev number.
 * @param [in]  port            - port number.
 * @param [out] interfacePtr    - sfp interface pointer.
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS PdlSfpDebugInterfaceGet (
    IN  UINT_32                   dev,
    IN  UINT_32                   logicalPort,
    OUT PDL_SFP_INTERFACE_STC   * interfacePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_SFP_PRV_DB_STC                * sfpPtr;
    PDL_SFP_PRV_KEY_STC                 sfpKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL interfacePtr, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort,dev);
        return PDL_BAD_PTR;
    }
    sfpKey.dev = dev;
    sfpKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&pdlSfpDb, (void*) &sfpKey, (void*) &sfpPtr);
    if (pdlStatus == PDL_OK) {
        memcpy (interfacePtr, &sfpPtr->publicInfo, sizeof (sfpPtr->publicInfo));
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->eepromInfo.interfaceType);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->eepromInfo.interfaceId);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->lossInfo.interfaceType);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->lossInfo.interfaceId);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->values.lossValue);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->presentInfo.interfaceType);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->presentInfo.interfaceId);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->values.presentValue);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->txEnableInfo.interfaceType);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->txEnableInfo.interfaceId);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->values.txEnableValue);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->txDisableInfo.interfaceType);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->txDisableInfo.interfaceId);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->values.txDisableValue);
    }
    else {
        return pdlStatus;
    }

    return PDL_OK;
}

/*$ END OF pdlBtnDebugInterfaceGet */

/**
 * @fn  PDL_STATUS PdlSfpDebugInterfaceSet ( IN UINT_32 dev, IN UINT_32 logicalPort, IN PDL_SFP_INTERFACE_STC * interfacePtr );
 *
 * @brief   Set sfp access information (used to change goodValue of present / loss / tx enable
 *          interfaces
 *          @note
 *          Used to change data retrieved from XML, can later be saved
 *
 * @param [in]  dev           - dev number.
 * @param [in]  port          - port number.
 * @param [in]  interfacePtr  - sfp interface pointer.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS PdlSfpDebugInterfaceSet (
    IN  UINT_32                   dev,
    IN  UINT_32                   logicalPort,
    IN PDL_SFP_INTERFACE_STC    * interfacePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_SFP_PRV_DB_STC                * sfpPtr;
    PDL_SFP_PRV_KEY_STC                 sfpKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL interfacePtr, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort,dev);
        return PDL_BAD_PTR;
    }
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->eepromInfo.interfaceType);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->eepromInfo.interfaceId);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->lossInfo.interfaceType);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->lossInfo.interfaceId);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->values.lossValue);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->presentInfo.interfaceType);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->presentInfo.interfaceId);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->values.presentValue);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->txEnableInfo.interfaceType);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->txEnableInfo.interfaceId);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->values.txEnableValue);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->txDisableInfo.interfaceType);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->txDisableInfo.interfaceId);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->values.txDisableValue);
    sfpKey.dev = dev;
    sfpKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&pdlSfpDb, (void*) &sfpKey, (void*) &sfpPtr);
    PDL_CHECK_STATUS (pdlStatus);
    memcpy (&sfpPtr->publicInfo, interfacePtr, sizeof (sfpPtr->publicInfo));
    return PDL_OK;
}
/*$ END OF pdlBtnDebugInterfaceSet */

/**
 * @fn  PDL_STATUS pdlSfpInit ( IN void )
 *
 * @brief   Pdl sfp initialize
 *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlSfpInit (
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
    dbAttributes.listAttributes.entrySize = sizeof(PDL_SFP_PRV_DB_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_SFP_PRV_KEY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &pdlSfpDb);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF pdlSfpInit */


/**
 * @fn  PDL_STATUS prvPdlSfpDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSfpDestroy (
    void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (pdlSfpDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlSfpDb);
        PDL_CHECK_STATUS(pdlStatus);    
    }
    return PDL_OK;
}

/*$ END OF prvPdlSfpDestroy */