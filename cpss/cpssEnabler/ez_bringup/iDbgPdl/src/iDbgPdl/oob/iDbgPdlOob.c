/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file iDbgPdlOob.c
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
#include <pdl/oob_port/pdlOobPort.h>
#include <pdl/oob_port/pdlOobPortDebug.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <iDbgPdl/oob/iDbgPdlOob.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <iDbgPdl/utils/iDbgPdlUtils.h>

/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      oobId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, firstOobId, lastOobId, oobCount;
	PDL_STATUS						pdlStatus;
	PDL_OOB_PORT_XML_ATTRIBUTES_STC oobInfo;
    char                          * oobMaxSpeedString = NULL;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (oobId == IDBG_PDL_ID_ALL) {
        firstOobId = 1;
        pdlStatus = pdlOobPortNumOfObbPortsGet(&oobCount);
        PDL_CHECK_STATUS(pdlStatus);
        lastOobId = oobCount + 1;
    }
    else {
        firstOobId = oobId;
        lastOobId = oobId + 1;
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-5s | %-3s | %-4s | %-6s | %-8s\n", "   ", "     ", "   ", "    ", "      ", "POSITION");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-5s | %-3s | %-4s | %-6s | %-8s\n", "Oob", " Max ", "   ", "CPU ", " PHY  ", "   IN   ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-5s | %-3s | %-4s | %-6s | %-8s\n", "ID ", "speed", "Eth", "port", "NUMBER", "   PHY  ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-5s | %-3s | %-4s | %-6s | %-8s\n", "---", "-----", "---", "----", "------", "--------");

    for (i = firstOobId; i < lastOobId; i++) {
        IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlOobPortAttributesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &oobInfo));
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_SPEED_TYPE_E, oobInfo.portMaximumSpeed, &oobMaxSpeedString);
        PDL_CHECK_STATUS(pdlStatus);

        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3d | %-5s | %-3d | %-4d | %-6d | %-8d\n", i, (oobMaxSpeedString == NULL) ? "NA" : oobMaxSpeedString,
                                                                                     oobInfo.ethId, oobInfo.cpuPortNumber,
                                                                                     oobInfo.phyConfig.phyNumber, oobInfo.phyConfig.phyPosition);
    }
    return PDL_OK;
}
/*$ END OF iDbgPdlOobGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobGetCount
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobGetCount (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         oobCount;
	PDL_STATUS						pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlOobPortNumOfObbPortsGet, IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_32, &oobCount));
    PDL_CHECK_STATUS(pdlStatus);
  
    IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Number of Oob ports: %d\n", oobCount);

    return PDL_OK;
}
/*$ END OF iDbgPdlOobGetCount */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobSetDebug
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobSetDebug
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

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlOobDebugSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(BOOLEAN, enable));

    return pdlStatus;
}

/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobSetEthId
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobSetEthId (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      oobId,
    IN UINT_32                  	      ethId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	PDL_STATUS						pdlStatus;
	PDL_OOB_PORT_XML_ATTRIBUTES_STC oobInfo;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlOobPortAttributesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId),
                                                                    IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &oobInfo));
    PDL_CHECK_STATUS(pdlStatus);

    oobInfo.ethId = ethId;

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlOobPortAttributesSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId),
                                                                    IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &oobInfo));
    return pdlStatus;
}
/*$ END OF iDbgPdlOobSetEthId */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobSetCpuPort
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobSetCpuPort (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      oobId,
    IN UINT_32                  	      cpuPort
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	PDL_STATUS						pdlStatus;
	PDL_OOB_PORT_XML_ATTRIBUTES_STC oobInfo;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlOobPortAttributesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId),
                                                                    IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &oobInfo));
    PDL_CHECK_STATUS(pdlStatus);

    oobInfo.cpuPortNumber = cpuPort;

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlOobPortAttributesSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId),
                                                                    IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &oobInfo));
    return pdlStatus;
}
/*$ END OF iDbgPdlOobSetCpuPort */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobSetMaxSpeed
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobSetMaxSpeed (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      oobId,
    IN char                             * speedStrPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	PDL_STATUS						pdlStatus;
	PDL_OOB_PORT_XML_ATTRIBUTES_STC oobInfo;
    PDL_PORT_SPEED_ENT              speed;
    UINT_32                         value;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_SPEED_TYPE_E, speedStrPtr, &value);
    if (pdlStatus != PDL_OK)
    {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Unknown port speed type. Possibly your xml file typedef section is out-of-date.\n");
        return pdlStatus;
    }
    speed = (PDL_PORT_SPEED_ENT)value;

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlOobPortAttributesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId),
                                                                    IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &oobInfo));
    PDL_CHECK_STATUS(pdlStatus);

    oobInfo.portMaximumSpeed = speed;

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlOobPortAttributesSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId),
                                                                    IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &oobInfo));
    return pdlStatus;
}
/*$ END OF iDbgPdlOobSetMaxSpeed */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobSetPhyId
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobSetPhyId (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      oobId,
    IN UINT_32                            phyId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	PDL_STATUS						pdlStatus;
	PDL_OOB_PORT_XML_ATTRIBUTES_STC oobInfo;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlOobPortAttributesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId),
                                                                    IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &oobInfo));
    PDL_CHECK_STATUS(pdlStatus);

    oobInfo.phyConfig.phyNumber = phyId;

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlOobPortAttributesSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId),
                                                                    IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &oobInfo));
    return pdlStatus;
}
/*$ END OF iDbgPdlOobSetPhyId */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobSetPhyPosition
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobSetPhyPosition (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      oobId,
    IN UINT_32                            phyPos
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	PDL_STATUS						pdlStatus;
	PDL_OOB_PORT_XML_ATTRIBUTES_STC oobInfo;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlOobPortAttributesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId),
                                                                    IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &oobInfo));
    PDL_CHECK_STATUS(pdlStatus);

    oobInfo.phyConfig.phyPosition = phyPos;

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlOobPortAttributesSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, oobId),
                                                                    IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &oobInfo));
    return pdlStatus;
}
/*$ END OF iDbgPdlOobSetPhyPosition */



/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobRunValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      oobId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, firstOobId, lastOobId, oobCount;
    PDL_STATUS						pdlStatus;
    PDL_OOB_PORT_XML_ATTRIBUTES_STC oobInfo;
    char                          * oobInfoSt = NULL;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);
    PDL_UNUSED_PARAM(oobId);

    firstOobId = 1;
    pdlStatus = pdlOobPortNumOfObbPortsGet(&oobCount);
    PDL_CHECK_STATUS(pdlStatus);
    lastOobId = oobCount + 1;

    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s | %-25s |\n", "  oob Id  ", "        oob Info         ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s | %-25s |\n", "----------", "-------------------------");

    if (oobCount > 0) {
        for (i = firstOobId; i < lastOobId; i++) {
            pdlStatus = pdlOobPortAttributesGet(i, &oobInfo);
            if (pdlStatus == PDL_OK)
                oobInfoSt = "Pass";
            else
                pdlStatusToString (pdlStatus, &oobInfoSt);
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10d | %-25s |\n", i, oobInfoSt);
        }
    }
    else IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s | %-25s |\n", "", "Not supported");
    
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");  

    return PDL_OK;
}
/*$ END OF iDbgPdlOobRunValidation */
