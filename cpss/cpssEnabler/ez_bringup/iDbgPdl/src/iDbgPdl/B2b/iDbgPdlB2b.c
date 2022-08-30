/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file iDbgPdlPower.c
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
 * @brief Debug lib power module
 * 
 * @version   1 
********************************************************************************
*/
/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdl/packet_processor/private/prvPdlPacketProcessor.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/serdes/pdlSerdes.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <iDbgPdl/interface/iDbgPdlInterface.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <iDbgPdl/utils/iDbgPdlUtils.h>


/*****************************************************************************
* FUNCTION NAME: iDbgPdlB2bGetInfo
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlB2bGetInfo (                     
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_32                                 b2bLink;
    PDL_PP_XML_ATTRIBUTES_STC               ppAttributes;
    PDL_PP_XML_B2B_ATTRIBUTES_STC           b2bAttributes;
    PDL_PORT_LANE_DATA_STC                  serdesInfo;
    char                                    leftSerdesList[12], rightSerdesList[12], tempSerde[5];
    char                                  * speedPtr = NULL, * modePtr = NULL;
    BOOLEAN                                 isSerdes;
    PDL_STATUS                              pdlStatus;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    pdlStatus =  pdlPpDbAttributesGet(&ppAttributes);
    PDL_CHECK_STATUS(pdlStatus); 
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-11s | %-7s | %-8s | %-12s | %-13s | %-18s | %-18s | %-5s | %-10s | %-12s | %-12s |\n"," link list ", "left-pp", "right-pp", "left-pp-port", "right-pp-port", "left-logical-port", "right-logical-port", "speed", "   mode   ", " left-serdes", "right-serdes");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-11s | %-7s | %-8s | %-12s | %-13s | %-18s | %-18s | %-5s | %-10s | %-12s | %-12s |\n","-----------", "-------", "--------", "------------", "-------------", "-----------------", "------------------", "-----", "----------", "------------", "------------");
    for (b2bLink = 0; b2bLink < ppAttributes.numOfBackToBackLinksPerPp; b2bLink ++) {
        pdlStatus = pdlPpDbB2bAttributesGet(b2bLink, &b2bAttributes);
        PDL_CHECK_STATUS(pdlStatus);
        isSerdes = FALSE;
        memset (&leftSerdesList, '\0', sizeof (leftSerdesList));
        pdlStatus = pdlSerdesPortDbSerdesInfoGetFirst(b2bAttributes.firstDev, b2bAttributes.firstPort, &serdesInfo);
        while (pdlStatus == PDL_OK){
            isSerdes = TRUE;
            prvPdlOsSnprintf (tempSerde, 4, "%d, ",  serdesInfo.absSerdesNum);
            strncat(leftSerdesList, tempSerde, 3);
            pdlStatus = pdlSerdesPortDbSerdesInfoGetNext(b2bAttributes.firstDev, b2bAttributes.firstPort, &serdesInfo, &serdesInfo);
        }
        if (!isSerdes) strcpy(leftSerdesList, "Not Found");
        isSerdes = FALSE;
        memset (&rightSerdesList, '\0', sizeof (rightSerdesList));
        pdlStatus = pdlSerdesPortDbSerdesInfoGetFirst(b2bAttributes.secondDev, b2bAttributes.secondPort, &serdesInfo);
        while (pdlStatus == PDL_OK){
            isSerdes = TRUE;
            prvPdlOsSnprintf (tempSerde, 4, "%d, ",  serdesInfo.absSerdesNum);
            strncat(rightSerdesList, tempSerde, 3);
            pdlStatus = pdlSerdesPortDbSerdesInfoGetNext(b2bAttributes.secondDev, b2bAttributes.secondPort, &serdesInfo, &serdesInfo);
        }
        if (!isSerdes) strcpy(rightSerdesList, "Not Found");
        pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, b2bAttributes.interfaceMode, &modePtr);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_SPEED_TYPE_E, b2bAttributes.maxSpeed, &speedPtr);
        PDL_CHECK_STATUS(pdlStatus);
        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-11d | %-7d | %-8d | %-12d | %-13d | %-18d | %-18d | %-5s | %-10s | %-12s | %-12s |\n", b2bLink, b2bAttributes.firstDev, b2bAttributes.firstPort, b2bAttributes.secondDev, b2bAttributes.secondPort, b2bAttributes.firstMacPort, b2bAttributes.secondMaclPort, speedPtr, modePtr, leftSerdesList, rightSerdesList);
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

    return PDL_OK;
}

/*$ END OF iDbgPdlB2bGetInfo */



/*****************************************************************************
* FUNCTION NAME: iDbgPdlB2bRunValidation
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlB2bRunValidation (                     
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_32                                 b2bLink;
    PDL_PP_XML_ATTRIBUTES_STC               ppAttributes;
    PDL_PP_XML_B2B_ATTRIBUTES_STC           b2bAttributes;
    char                                  * InfoSt = NULL;
    PDL_STATUS                              pdlStatus;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    pdlStatus =  pdlPpDbAttributesGet(&ppAttributes);
    PDL_CHECK_STATUS(pdlStatus); 
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-11s | %-15s |\n"," link list ", "     Info      ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-11s | %-15s |\n","-----------", "---------------"); 
    for (b2bLink = 0; b2bLink < ppAttributes.numOfBackToBackLinksPerPp; b2bLink ++) {
        pdlStatus = pdlPpDbB2bAttributesGet(b2bLink, &b2bAttributes);
        if (pdlStatus == PDL_OK){
            InfoSt = "Pass";
        }
        else pdlStatusToString (pdlStatus, &InfoSt);
        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-11d | %-15s |\n", b2bLink, InfoSt);
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

    return PDL_OK;
}

/*$ END OF iDbgPdlB2bRunValidation */

            
            
            
   