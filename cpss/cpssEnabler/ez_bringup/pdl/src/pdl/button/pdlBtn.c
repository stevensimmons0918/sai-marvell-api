/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlBtn.c   
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
 * @brief Platform driver layer - Button related API
 * 
 * @version   1 
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/button/pdlBtn.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/button/private/prvPdlBtn.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

static PRV_PDL_LIB_STR_TO_ENUM_STC pdlBtnNameStrToEnumPairs[PDL_BTN_ID_LAST_E] = {
    {"Reset", PDL_BTN_ID_RESET_E},
    {"LED", PDL_BTN_ID_LED_MODE_E},
};
PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC pdlBtnNameStrToEnum = {pdlBtnNameStrToEnumPairs, PDL_BTN_ID_LAST_E};

static PDL_DB_PRV_STC   pdlBtnDb = {0,NULL};

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)                                                                   \
    if (__pdlStatus != PDL_OK) {                                                                        \
    if (prvPdlBtnDebugFlag) {                                                                           \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__pdlStatus,PDL_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __pdlStatus;                                                                                 \
    }  
/*macro to check the xml status and debug prints if debug flag on*/
#undef XML_CHECK_STATUS
#define XML_CHECK_STATUS(__xmlStatus)                                                                   \
    if (__xmlStatus != PDL_OK) {                                                                        \
    if (prvPdlBtnDebugFlag) {                                                                           \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__xmlStatus,XML_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __xmlStatus;                                                                                 \
    }  

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/* ***************************************************************************
* FUNCTION NAME: pdlBtnDbCapabilitiesGet
*
* DESCRIPTION:   Get buttonId supported capabilities Based on data retrieved from XML
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS pdlBtnDbCapabilitiesGet (
    IN  PDL_BTN_ID_ENT                  buttonId,
    OUT PDL_BTN_XML_CAPABILITIES_STC  * capabilitiesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_BTN_PRV_DB_STC 	              * buttonPtr;
    PDL_BTN_PRV_KEY_STC                 btnKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (capabilitiesPtr == NULL) {
        PDL_LIB_DEBUG_MAC (prvPdlBtnDebugFlag)(__FUNCTION__, __LINE__, "NULL POINTER");
        return PDL_BAD_PTR;
    }
    btnKey.btnId = buttonId;    
    pdlStatus = prvPdlDbFind (&pdlBtnDb, (void*) &btnKey, (void*) &buttonPtr);
    if (pdlStatus == PDL_NOT_FOUND) {
        capabilitiesPtr->isSupported = FALSE;
        return PDL_OK;
    }
    PDL_CHECK_STATUS (pdlStatus);	
    memcpy (capabilitiesPtr, &buttonPtr->btnCapabilities, sizeof(buttonPtr->btnCapabilities));
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, buttonPtr->btnCapabilities);
    return PDL_OK;
}

/*$ END OF pdlBtnDbCapabilitiesGet */

/* ***************************************************************************
* FUNCTION NAME: pdlBtnHwStatusGet
*
* DESCRIPTION:   get button operational status (pressed/not pressed)
*                based on information from HW access
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS pdlBtnHwStatusGet (
    IN  PDL_BTN_ID_ENT                 buttonId,
    OUT PDL_BTN_STATUS_ENT         *   statusPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                             data;
    PDL_STATUS                          pdlStatus;
    PDL_BTN_PRV_DB_STC 	              * buttonPtr;
    PDL_BTN_PRV_KEY_STC                 btnKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (statusPtr == NULL) {
        PDL_LIB_DEBUG_MAC (prvPdlBtnDebugFlag)(__FUNCTION__, __LINE__, "NULL statusPtr");
        return PDL_BAD_PTR;
    }
    btnKey.btnId = buttonId;
    pdlStatus = prvPdlDbFind (&pdlBtnDb, (void*) &btnKey, (void*) &buttonPtr);
	PDL_CHECK_STATUS (pdlStatus);
    pdlStatus = prvPdlInterfaceHwGetValue (buttonPtr->btnInterface.interfaceType, buttonPtr->btnInterface.buttonInterfaceId, &data);
	PDL_CHECK_STATUS (pdlStatus);
    if (data == buttonPtr->btnInterface.values.pushValue) {
        *statusPtr = PDL_BTN_STATUS_PRESSED_E;
    }
    else {
        *statusPtr = PDL_BTN_STATUS_NOT_PRESSED_E;
    }
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, *statusPtr);
    return PDL_OK;
}

/*$ END OF pdlBtnHwStatusGet */

/* ***************************************************************************
* FUNCTION NAME: pdlBtnDebugInterfaceGet
*
* DESCRIPTION:   Get buttonId access information
*
* PARAMETERS:   
*                
*
*****************************************************************************/
PDL_STATUS pdlBtnDebugInterfaceGet (
    IN  PDL_BTN_ID_ENT            buttonId,
    OUT PDL_BTN_INTERFACE_STC   * interfacePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_BTN_PRV_DB_STC 	              * buttonPtr;
    PDL_BTN_PRV_KEY_STC                 btnKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC (prvPdlBtnDebugFlag)(__FUNCTION__, __LINE__, "NULL POINTER");
        return PDL_BAD_PTR;
    }
    btnKey.btnId = buttonId;
    pdlStatus = prvPdlDbFind (&pdlBtnDb, (void*) &btnKey, (void**) &buttonPtr);
    PDL_CHECK_STATUS (pdlStatus);	
    memcpy (interfacePtr, &buttonPtr->btnInterface, sizeof (buttonPtr->btnInterface));
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->buttonInterfaceId);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->interfaceType);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->values.pushValue);
    return PDL_OK;
}

/*$ END OF pdlBtnDebugInterfaceGet */

/* ***************************************************************************
* FUNCTION NAME: pdlBtnDebugInterfaceSet
*
* DESCRIPTION:   Used to change data retrieved from XML, can later be saved
*
* PARAMETERS:   
*                
*
*****************************************************************************/
PDL_STATUS pdlBtnDebugInterfaceSet (
    IN  PDL_BTN_ID_ENT            buttonId,
    IN  PDL_BTN_VALUES_STC      * valuesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_BTN_PRV_DB_STC 	              * btnPtr;
    PDL_BTN_PRV_KEY_STC                 btnKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (valuesPtr == NULL) {
        PDL_LIB_DEBUG_MAC (prvPdlBtnDebugFlag)(__FUNCTION__, __LINE__, "NULL POINTER");
        return PDL_BAD_PTR;
    }
    btnKey.btnId = buttonId;
    pdlStatus = prvPdlDbFind (&pdlBtnDb, (void*) &btnKey, (void**) &btnPtr);
    PDL_CHECK_STATUS (pdlStatus);
    memcpy (&btnPtr->btnInterface.values, valuesPtr, sizeof (PDL_BTN_VALUES_STC));
    return PDL_OK;

}

/*$ END OF pdlBtnDebugInterfaceSet */

/* ***************************************************************************
* FUNCTION NAME: prvPdlBtnXMLParser
*
* DESCRIPTION:   Fill all button related information from XML to DB
*
* PARAMETERS:   
*                
* XML structure:
*<buttons>
*   <ResetButton>
*       <gpio-group>
*           <gpio-read-only-interface>
*			    <device-number type=gpio-device-number-type>255</device-number>
*			    <pin-number type=gpio-pin-number-type>14</pin-number>
*           </gpio-read-only-interface>
*           <push-value type=mpp-pin-value-type>0</push-value>    
*       </gpio-group>
*   </ResetButton>
*</buttons>
*
*****************************************************************************/

PDL_STATUS prvPdlBtnXMLParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                             pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    PDL_STATUS                       pdlStatus;
    XML_PARSER_RET_CODE_TYP          xmlStatus, xmlStatus2;
    PDL_BTN_ID_ENT                   btnId;
    PDL_BTN_PRV_DB_STC               btnInfo;
    PDL_BTN_PRV_DB_STC             * buttonPtr = NULL;
    PDL_BTN_PRV_KEY_STC              btnKey;
    XML_PARSER_NODE_DESCRIPTOR_TYP   xmlBtnId, xmlBtnListId, xmlBtnInstanceId, xmlBtnChildId;
    UINT_32                          maxSize;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    xmlStatus = xmlParserFindByName (xmlId, PDL_BTN_TAG_NAME, &xmlBtnId);
    /* section is not mandatory */
    if (xmlStatus == XML_PARSER_RET_CODE_NOT_FOUND) {
        return PDL_OK;
    }
    XML_CHECK_STATUS(xmlStatus);
    xmlStatus = xmlParserGetFirstChild (xmlBtnId, &xmlBtnListId);
    /* go over all buttons */
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);
        if (xmlParserIsEqualName (xmlBtnListId, "buttons-list") == XML_PARSER_RET_CODE_OK) {
            xmlStatus2 = xmlParserGetFirstChild (xmlBtnListId, &xmlBtnInstanceId);
            memset (&btnInfo, 0, sizeof (btnInfo));
            btnInfo.btnCapabilities.isSupported = TRUE;
            while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {                
                XML_CHECK_STATUS(xmlStatus2);
                if (xmlParserIsEqualName (xmlBtnInstanceId, "button-id") == XML_PARSER_RET_CODE_OK) {
                    maxSize = PDL_XML_MAX_TAG_LEN;
                    xmlParserGetValue(xmlBtnInstanceId, &maxSize, &pdlTagStr[0]);
                    pdlStatus =  pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_BUTTON_TYPE_E, pdlTagStr, (UINT_32*) &btnId);
                    PDL_CHECK_STATUS(pdlStatus);
                    btnKey.btnId = btnId;
                }
                else if (xmlParserIsEqualName (xmlBtnInstanceId, "push-value") == XML_PARSER_RET_CODE_OK) {
                    xmlStatus2 = xmlParserGetFirstChild (xmlBtnInstanceId, &xmlBtnChildId);
                    XML_CHECK_STATUS(xmlStatus2);
                    /* skip read-interface-select */
                    if (xmlParserIsEqualName (xmlBtnChildId, "read-interface-select") == XML_PARSER_RET_CODE_OK) {
                        xmlStatus2 = xmlParserGetNextSibling (xmlBtnChildId, &xmlBtnChildId);
                        XML_CHECK_STATUS(xmlStatus2);
                    }
                    /* if gpio-group or i2c-group container skip to interface container*/
                    if ((xmlParserIsEqualName (xmlBtnChildId, "read-i2c-interface") == XML_PARSER_RET_CODE_OK) ||
                        (xmlParserIsEqualName (xmlBtnChildId, "read-gpio-interface") == XML_PARSER_RET_CODE_OK)) {
                        maxSize = sizeof(btnInfo.btnInterface.values.pushValue);
                        pdlStatus = prvPdlInterfaceAndValueXmlParser(xmlBtnChildId, maxSize, &btnInfo.btnInterface.interfaceType, &btnInfo.btnInterface.buttonInterfaceId, &btnInfo.btnInterface.values.pushValue);                        
                        PDL_CHECK_STATUS(pdlStatus);
                    }    
                    pdlStatus = prvPdlDbAdd (&pdlBtnDb, (void*) &btnKey, (void*) &btnInfo, (void**) &buttonPtr);
		            PDL_CHECK_STATUS(pdlStatus);
                }
                else {
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus2 = xmlParserGetNextSibling(xmlBtnInstanceId, &xmlBtnInstanceId);            
            }
        }    
        else {
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling (xmlBtnListId, &xmlBtnListId);
    }
    return PDL_OK;   
}

/*$ END OF prvPdlBtnXMLParser */

/* ***************************************************************************
* FUNCTION NAME: pdlBtnInit
*
* DESCRIPTION:   Init button module
*                Create button DB and initialize
*
* PARAMETERS:   
*                
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlBtnDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Btn debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlBtnDebugSet (
    IN  BOOLEAN             state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlBtnDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlBtnDebugSet */

PDL_STATUS pdlBtnInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
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
    dbAttributes.listAttributes.entrySize = sizeof(PDL_BTN_PRV_DB_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_BTN_PRV_KEY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &pdlBtnDb);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlBtnXMLParser (xmlId);
    PDL_CHECK_STATUS(pdlStatus);	

    return PDL_OK;
}

/*$ END OF pdlBtnInit */

/**
 * @fn  PDL_STATUS prvPdlBtnDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlBtnDestroy (
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
    if (pdlBtnDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlBtnDb);
        PDL_CHECK_STATUS(pdlStatus);
    }
    return PDL_OK;
}

/*$ END OF prvPdlBtnDestroy */
