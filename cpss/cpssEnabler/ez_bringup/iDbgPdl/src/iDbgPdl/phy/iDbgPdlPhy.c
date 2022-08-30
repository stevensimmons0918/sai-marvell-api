/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file iDbgPdlPhy.c
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
#include <pdl/phy/pdlPhy.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <iDbgPdl/phy/iDbgPdlPhy.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <iDbgPdl/utils/iDbgPdlUtils.h>

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPhyGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlPhyGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      phyId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, firstPhyId, lastPhyId, phyCount, temp;
	PDL_STATUS						pdlStatus;
	PDL_PHY_CONFIGURATION_STC       phyInfo;
    char                          * phyTypeString = NULL, *phyDownloadTypeString = NULL;
    UINT_32                         dev, port;
    PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC  portAttributes;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (phyId == IDBG_PDL_ID_ALL) {
        firstPhyId = 0;
        pdlStatus = PdlPhyDbNumberOfPhysGet(&phyCount);
        PDL_CHECK_STATUS(pdlStatus);
        lastPhyId = phyCount;
    }
    else {
        firstPhyId = phyId;
        lastPhyId = phyId + 1;
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-9s | %-15s | %-13s | %s\n", "PHY ID",    "PHY Type",        "Download Type", "Connected Ports (dev/port)");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-9s | %-15s | %-13s | %s\n", "---------", "---------------", "-------------", "--------------------------");

    for (i = firstPhyId; i <lastPhyId; i++) {
        IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, PdlPhyDbPhyConfigurationGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &phyInfo));
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_PHY_TYPE_TYPE_E, phyInfo.phyType, &phyTypeString);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_E, phyInfo.phyDownloadType, &phyDownloadTypeString);
        PDL_CHECK_STATUS(pdlStatus);

        IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-9d | %-15s | %-13s | ", i, (phyTypeString == NULL) ? "NA" : phyTypeString, (phyDownloadTypeString == NULL) ? "NA" : phyDownloadTypeString);

        for (pdlStatus = pdlPpDbFirstPortAttributesGet(&dev, &port), temp = 0; pdlStatus == PDL_OK; pdlStatus = pdlPpDbPortAttributesGetNext(dev, port, &dev, &port))
        {
            pdlStatus = pdlPpDbPortAttributesGet(dev, port, &portAttributes);
            PDL_CHECK_STATUS(pdlStatus);
            if (portAttributes.isPhyExists && portAttributes.phyData.phyNumber == i)
            {
                /* up to 4 dev/port pairs in one line */
                if (temp && ((temp % 4) == 0))
                {
                    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n %-9s | %-15s | %-13s | ", "", "", "");
                }
                IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %d/%02d ", dev, port);
                temp ++;
            }
        }
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    }
    return PDL_OK;
}
/*$ END OF iDbgPdlPhyGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPhyGetCount
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlPhyGetCount (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         phyCount;
	PDL_STATUS						pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, PdlPhyDbNumberOfPhysGet, IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &phyCount));
    PDL_CHECK_STATUS(pdlStatus);
  
    IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Number of phys: %d\n", phyCount);

    return PDL_OK;
}
/*$ END OF iDbgPdlPhyGetCount */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPhySetDebug
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlPhySetDebug
(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN BOOLEAN                            enable
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_STATUS                      pdlStatus;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlPhyDebugSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(BOOLEAN, enable));

    return pdlStatus;
}


/*****************************************************************************
* FUNCTION NAME: iDbgPdlPhyRunValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlPhyRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      phyId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, firstPhyId, lastPhyId, phyCount;
    PDL_STATUS						pdlStatus;
    PDL_PHY_CONFIGURATION_STC       phyInfo;
    char                          * phyTypeSt = NULL, *PhyInfoSt = NULL;
 
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (phyId == IDBG_PDL_ID_ALL) {
        firstPhyId = 0;
        pdlStatus = PdlPhyDbNumberOfPhysGet(&phyCount);
        lastPhyId = phyCount;
    }
    else {
        firstPhyId = phyId;
        lastPhyId = phyId + 1;
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-10s | %-16s | %-16s |\n", "  PHY ID  ", "    PHY TYPE    ", "      INFO      ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-10s | %-16s | %-16s |\n", "----------", "----------------", "----------------");

    for (i = firstPhyId; i <lastPhyId; i++) {
        pdlStatus = PdlPhyDbPhyConfigurationGet(i, &phyInfo);
        if (pdlStatus == PDL_OK){
            pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_PHY_TYPE_TYPE_E, phyInfo.phyType, &phyTypeSt);
            PDL_CHECK_STATUS(pdlStatus);
            PhyInfoSt = "Pass";
        }
        else {
            pdlStatusToString (pdlStatus, &PhyInfoSt);
            phyTypeSt = PhyInfoSt;
        }
        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-10d | %-16s | %-16s |\n", i, phyTypeSt, PhyInfoSt);
    }

    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

    return PDL_OK;
}
/*$ END OF iDbgPdlPhyRunValidation */
