
/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file iDbgPdlBtn.c   
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
 * @brief Debug lib button module
 * 
 * @version   1 
********************************************************************************
*/
/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/button/pdlBtnDebug.h>
#include <pdl/xml/xmlParser.h>
#include <pdl/lib/pdlLib.h>
#include <iDbgPdl/button/iDbgPdlBtn.h>
#include <iDbgPdl/interface/iDbgPdlInterface.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <iDbgPdl/utils/iDbgPdlUtils.h>

/*****************************************************************************
* FUNCTION NAME: iDbgPdlBtnGetState
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlBtnGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      btnId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	UINT_32                         i, firstBtnId, lastBtnId;
    PDL_STATUS						pdlStatus;
	PDL_BTN_INTERFACE_STC           btnInterface;
    PDL_BTN_XML_CAPABILITIES_STC    btnCapabilities={0};
    BOOLEAN                         single = FALSE;
    char                          * btnNamePtr, *btnInterfaceTypeStr = NULL;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (btnId == IDBG_PDL_ID_ALL) {
        firstBtnId = 0;
        lastBtnId = PDL_BTN_ID_LAST_E;
    }
    else {
        firstBtnId = btnId;
        lastBtnId = btnId + 1;
        single = TRUE;
    }

    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s | %-9s | %-14s | %-8s |\n", "  Btn Id  ", "Supported", "Interface Type", "Push Val");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s | %-9s | %-14s | %-8s |\n", "----------", "---------", "--------------", "--------");

    for (i = firstBtnId; i <lastBtnId; i++) {
        IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlBtnDbCapabilitiesGet,  IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &btnCapabilities));
        if (pdlStatus != PDL_OK && pdlStatus != PDL_NOT_FOUND) {
            return pdlStatus;
        }       
        memset (&btnInterface, 0, sizeof(btnInterface));
        if (btnCapabilities.isSupported == TRUE) {
            IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlBtnDebugInterfaceGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &btnInterface));
            PDL_CHECK_STATUS(pdlStatus);
        }
       
        pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_BUTTON_TYPE_E, i, &btnNamePtr);
        PDL_CHECK_STATUS(pdlStatus);

        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s | %-9s | %-14s | %-8d |\n",
                                        btnNamePtr,
                                        btnCapabilities.isSupported ==  PDL_IS_SUPPORTED_YES_E ? "+":"-",
                                        btnCapabilities.isSupported ? btnInterface.interfaceType == PDL_INTERFACE_TYPE_GPIO_E ? "GPIO":PDL_INTERFACE_TYPE_I2C_E ? "I2C":"Unknown" : "Not relevant",  
                                        btnInterface.values.pushValue);
    }
    pdlStatus = iDbgPdlConvertEnumToString (IDBG_PDL_UTILS_INTERFACE_TYPE, btnInterface.interfaceType, &btnInterfaceTypeStr);
    if ((single) && (pdlStatus == PDL_OK)){
        IDBG_PDL_LOGGER_API_OUTPUT_LOG ("\n");
        pdlStatus = iDbgPdlInterfaceGetInfo (resultPtr, btnInterfaceTypeStr, btnInterface.buttonInterfaceId);
        PDL_CHECK_STATUS(pdlStatus);
    }

    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

    return PDL_OK;
}
/*$ END OF iDbgPdlBtnGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlBtnGetHwStatus
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlBtnGetHwStatus (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      btnId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	UINT_32                         i, firstBtnId, lastBtnId;
    PDL_STATUS						pdlStatus;
    PDL_BTN_XML_CAPABILITIES_STC    btnCapabilities={0};
	PDL_BTN_STATUS_ENT              btnStatus;
    char                          * btnNamePtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);
    if (btnId == IDBG_PDL_ID_ALL) {
        firstBtnId = 0;
        lastBtnId = PDL_BTN_ID_LAST_E;
    }
    else {
        firstBtnId = btnId;
        lastBtnId = btnId + 1;
    }

    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s | %-13s |\n", "  Btn Id  ", "   Status   ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s | %-13s |\n", "----------", "-------------");

    for (i = firstBtnId; i <lastBtnId; i++) {

        IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlBtnDbCapabilitiesGet,  IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &btnCapabilities));
        if (pdlStatus != PDL_OK && pdlStatus != PDL_NOT_FOUND) {
            return pdlStatus;
        }       
        if (btnCapabilities.isSupported == TRUE) {
            IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlBtnHwStatusGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_8, &btnStatus));
            PDL_CHECK_STATUS(pdlStatus);
        }
       
        pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_BUTTON_TYPE_E, i, &btnNamePtr);
        PDL_CHECK_STATUS(pdlStatus);
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s | %-13s |\n",
                                        btnNamePtr,
                                        btnCapabilities.isSupported ? btnStatus == PDL_BTN_STATUS_PRESSED_E ? "Pressed" : "Not Pressed" : "Not Relevant");
    }

    return PDL_OK;
}
/*$ END OF iDbgPdlBtnGetHwStatus */


/*****************************************************************************
* FUNCTION NAME: iDbgPdlBtnSetPushValue
*
* DESCRIPTION:   change the push value for given button
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlBtnSetPushValue (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN IDBG_PDL_BTN_SET_CMD_ENT           cmdType,
    IN UINT_32                  	      btnId,
    IN UINT_32                            value
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_BTN_VALUES_STC              btnValue;
    PDL_STATUS                      pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    switch (cmdType) {
    case IDBG_PDL_BTN_SET_CMD_INFO_E:
        btnValue.pushValue = value;
        IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlBtnDebugInterfaceSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, btnId), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &btnValue));
        PDL_CHECK_STATUS(pdlStatus);
        break;
    case IDBG_PDL_BTN_SET_CMD_DEBUG_E:
        pdlStatus = pdlBtnDebugSet((BOOLEAN)value);
        PDL_CHECK_STATUS(pdlStatus);
        break;
    default:
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Unknown cmdId");
        return PDL_BAD_VALUE;
        break;
    }
    return pdlStatus;
}

/*$ END OF iDbgPdlBtnSetPushValue */


/*****************************************************************************
* FUNCTION NAME: iDbgPdlBtnGetValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlBtnGetValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      btnId
    )
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, firstBtnId, lastBtnId;
    PDL_STATUS						pdlStatus, pdlStatusCapabilities, pdlStatusHw = PDL_OK;
    PDL_BTN_XML_CAPABILITIES_STC    btnCapabilities={0};
    PDL_BTN_STATUS_ENT              btnStatus;
    char                            * btnNamePtr = NULL, * pdlStatusStr = NULL;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (btnId == IDBG_PDL_ID_ALL) {
        firstBtnId = 0;
        lastBtnId = PDL_BTN_ID_LAST_E;
    }
    else {
        firstBtnId = btnId;
        lastBtnId = btnId + 1;
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s | %-16s | %-14s |\n", "  Btn Id  ", "    Info    ", "   Status   ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s | %-16s | %-14s |\n", "----------", "  ---------  "," ---------- ");
    for (i = firstBtnId; i <lastBtnId; i++) {
        pdlStatusCapabilities = pdlBtnDbCapabilitiesGet(i, &btnCapabilities);
        pdlStatusToString(pdlStatusCapabilities, &pdlStatusStr);
        if (pdlStatusCapabilities == PDL_OK && btnCapabilities.isSupported == PDL_IS_SUPPORTED_YES_E) {
            pdlStatusHw = pdlBtnHwStatusGet(i, &btnStatus);
            pdlStatusToString(pdlStatusHw, &pdlStatusStr);
        }
        pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_BUTTON_TYPE_E, i, &btnNamePtr);
        PDL_CHECK_STATUS(pdlStatus);
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s | %-16s | %-14s |\n", btnNamePtr,
        															pdlStatusCapabilities == PDL_OK ? btnCapabilities.isSupported == PDL_IS_SUPPORTED_YES_E ? "Supported" : "Not Supported" : pdlStatusStr,
																	btnCapabilities.isSupported == PDL_IS_SUPPORTED_YES_E ? pdlStatusHw == PDL_OK ?  "Pass" : pdlStatusStr : "Not Supported");
    }

    return PDL_OK;
}
/*$ END OF iDbgPdlBtnGetValidation */
