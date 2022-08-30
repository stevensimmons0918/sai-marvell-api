/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file iDbgPdlPp.c
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
 * @brief Debug lib packet-processor module
 * 
 * @version   1 
********************************************************************************
*/
/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <iDbgPdl/packetProcessor/iDbgPdlPp.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <iDbgPdl/utils/iDbgPdlUtils.h>

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPpGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlPpGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_PP_XML_ATTRIBUTES_STC       ppAttributes;
	PDL_STATUS						pdlStatus;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlPpDbAttributesGet, IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &ppAttributes));
    PDL_CHECK_STATUS(pdlStatus);

    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-19s | %-18s | %-8s\n", "Packet processors", "Back-to-back Links", "Front panels");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-19s | %-18s | %-8s\n", "-----------------", "------------------", "------------");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-19d | %-18d | %-8d\n", ppAttributes.numOfPps, ppAttributes.numOfBackToBackLinksPerPp, ppAttributes.numOfFrontPanelGroups);

    return PDL_OK;
}
/*$ END OF iDbgPdlPpGetInfo */


/*****************************************************************************
* FUNCTION NAME: iDbgPdlPpRunValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlPpRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
    )
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_PP_XML_ATTRIBUTES_STC       ppAttributes;
    PDL_STATUS						pdlStatus;
    char                          * infoPtr;
    /****************************************************************************/
    /*                     F U N C T I O N   L O G I C                          */
    /****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    pdlStatus = pdlPpDbAttributesGet (&ppAttributes);
    
    if (pdlStatus == PDL_OK)
        infoPtr = "Pass";
    else pdlStatusToString (pdlStatus, &infoPtr);
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("|%-25s|\n", "  Packet processor Info  ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("|%-25s|\n", "-------------------------");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("|%-25s|\n", infoPtr);

    return PDL_OK;
}
/*$ END OF iDbgPdlPpRunValidation */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPpGetFp
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlPpGetFp (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            fpId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	PDL_STATUS						    pdlStatus;
    PDL_PP_XML_ATTRIBUTES_STC           ppAttributes;
    PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC frontPanelAttributes;
    UINT_32                             firstFpId, lastFpId, index;
    char                               *grOrderStr, *maxSpeedStr;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (fpId == IDBG_PDL_ID_ALL)
    {
        pdlStatus = pdlPpDbAttributesGet(&ppAttributes);
        PDL_CHECK_STATUS(pdlStatus);
        firstFpId = 1;
        lastFpId = ppAttributes.numOfFrontPanelGroups + 1;
    }
    else
    {
        firstFpId = fpId;
        lastFpId = fpId + 1;
    }

    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-14s | %-14s | %-9s | %-23s | %-23s\n", "FRONT PANEL ID", "GROUP ORDERING", "MAX SPEED", "FIRST PORT NUMBER SHIFT", "NUMBER OF PORTS IN GROUP");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-14s | %-14s | %-9s | %-23s | %-23s\n", "--------------", "--------------", "---------", "-----------------------", "------------------------");
    for (index = firstFpId; index < lastFpId; index ++)
    {
        IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlPpDbFrontPanelAttributesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, index), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &frontPanelAttributes));
        if (pdlStatus == PDL_OK)
        {
            pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_SPEED_TYPE_E, frontPanelAttributes.portMaximumSpeed, &maxSpeedStr);
            PDL_CHECK_STATUS(pdlStatus);
            if (frontPanelAttributes.ordering == PDL_PP_FRONT_PANEL_GROUP_ORDERING_RIGHT_DOWN_E)
                grOrderStr = "RightDown";
            else if (frontPanelAttributes.ordering == PDL_PP_FRONT_PANEL_GROUP_ORDERING_DOWN_RIGHT_E)
                grOrderStr = "DownRight";
            else
            {
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("FUNCTION: %s LINE %d: ILLEGAL group ordering %d\n", __FUNCTION__, __LINE__, frontPanelAttributes.ordering);
                return PDL_BAD_STATE;
            }
            PDL_CHECK_STATUS(pdlStatus);
            IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-14d | %-14s | %-9s | %-23d | %-23d\n", index,
                                                                                      grOrderStr,
                                                                                      maxSpeedStr,
                                                                                      frontPanelAttributes.firstPortNumberShift,
                                                                                      frontPanelAttributes.numOfPortsInGroup);
        }
    }
    
    return PDL_OK;
}
/*$ END OF iDbgPdlPpGetFp */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPpSetDebug
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlPpSetDebug
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

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlPpDebugSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(BOOLEAN, enable));

    return pdlStatus;
}

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPpSetFp
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlPpSetFp (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            fpId,
    IN char                             * grOrderStr,
    IN char                             * maxSpeedStr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC   frontPanelAttributes;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlPpDbFrontPanelAttributesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, fpId), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &frontPanelAttributes));
    if (pdlStatus != PDL_OK)
    {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[front panel %d] Panel doesn't exist\n", fpId);
        return pdlStatus;
    }

    if (maxSpeedStr)
    {
        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_SPEED_TYPE_E, maxSpeedStr, (UINT_32*) &frontPanelAttributes.portMaximumSpeed);
        if (pdlStatus != PDL_OK)
        {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("[front panel %d] ILLEGAL max speed %s\n", fpId, maxSpeedStr);
            return pdlStatus;
        }
    }

    if (grOrderStr)
    {
        if (strcmp(grOrderStr, "RightDown") == 0)
            frontPanelAttributes.ordering = PDL_PP_FRONT_PANEL_GROUP_ORDERING_RIGHT_DOWN_E;
        else if (strcmp(grOrderStr, "DownRight") == 0)
            frontPanelAttributes.ordering = PDL_PP_FRONT_PANEL_GROUP_ORDERING_DOWN_RIGHT_E;
        else
        {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("[front panel %d] ILLEGAL group ordering %s\n", fpId, grOrderStr);
            return PDL_BAD_STATE;
        }
    }

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlPpDbFrontPanelAttributesSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, fpId), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &frontPanelAttributes));

    return pdlStatus;
}
/*$ END OF iDbgPdlPpSetFp */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPpSetB2b
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlPpSetB2b (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            b2bId,
    IN char                             * maxSpeedStr,
    IN char                             * ifModeStr,
    IN void                             * firstDevicePtr,
    IN void                             * firstPortPtr,
    IN void                             * secondDevicePtr,
    IN void                             * secondPortPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_PP_XML_B2B_ATTRIBUTES_STC       b2bAttributes, b2bAttributes2;
    PDL_PP_XML_ATTRIBUTES_STC           ppAttributes;
    UINT_32                             index;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlPpDbB2bAttributesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, b2bId), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &b2bAttributes));
    if (pdlStatus != PDL_OK)
    {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[b2b link %d] Link doesn't exist\n", b2bId);
        return pdlStatus;
    }

    if (maxSpeedStr)
    {
        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_SPEED_TYPE_E, maxSpeedStr, (UINT_32*) &b2bAttributes.maxSpeed);
        if (pdlStatus != PDL_OK)
        {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("[b2b link %d] ILLEGAL max speed %s\n", b2bId, maxSpeedStr);
            return pdlStatus;
        }
    }

    if (ifModeStr)
    {
        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, ifModeStr, (UINT_32*) &b2bAttributes.interfaceMode);
        if (pdlStatus != PDL_OK)
        {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("[b2b link %d] ILLEGAL interface mode %s\n", b2bId, ifModeStr);
            return pdlStatus;
        }
    }

    if (firstDevicePtr)
    {
    	b2bAttributes.firstDev = *(UINT_32 *)firstDevicePtr;
    }

    if (firstPortPtr)
    {
    	b2bAttributes.firstPort = *(UINT_32 *)firstPortPtr;
    }

    if (secondDevicePtr)
    {
    	b2bAttributes.secondDev = *(UINT_32 *)secondDevicePtr;
    }

    if (secondPortPtr)
    {
    	b2bAttributes.secondPort = *(UINT_32 *)secondPortPtr;
    }

    if (firstDevicePtr || firstPortPtr || secondDevicePtr || secondPortPtr)
    {
        /* check consistency of first dev/port and second dev/port */
        pdlStatus = pdlPpDbAttributesGet(&ppAttributes);
        PDL_CHECK_STATUS(pdlStatus);
        for (index = 0; index < ppAttributes.numOfBackToBackLinksPerPp; index ++)
        {
            if (b2bAttributes.firstDev == b2bAttributes.secondDev)
            {
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("[b2b link %d] First device can't be same as second device !\n", index);
            	return PDL_BAD_PARAM;
            }

            if (b2bId == index)
                continue;

            IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlPpDbB2bAttributesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, index), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &b2bAttributes2));
            if (pdlStatus != PDL_OK)
                continue;

            if ((b2bAttributes.firstDev == b2bAttributes2.firstDev) && (b2bAttributes.firstPort == b2bAttributes2.firstPort))
            {
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("First device/port on link %d is the same as first device/port on link %d !\n", b2bId, index);
            	return PDL_BAD_PARAM;
            }

            if ((b2bAttributes.firstDev == b2bAttributes2.secondDev) && (b2bAttributes.firstPort == b2bAttributes2.secondPort))
            {
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("First device/port on link %d is the same as second device/port on link %d !\n", b2bId, index);
            	return PDL_BAD_PARAM;
            }

            if ((b2bAttributes.secondDev == b2bAttributes2.firstDev) && (b2bAttributes.secondPort == b2bAttributes2.firstPort))
            {
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("Second device/port on link %d is the same as first device/port on link %d !\n", b2bId, index);
            	return PDL_BAD_PARAM;
            }

            if ((b2bAttributes.secondDev == b2bAttributes2.secondDev) && (b2bAttributes.secondPort == b2bAttributes2.secondPort))
            {
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("First device/port on link %d is the same as second device/port on link %d !\n", b2bId, index);
            	return PDL_BAD_PARAM;
            }
        }
    }

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlPpDbB2bAttributesSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, b2bId), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &b2bAttributes));

    return pdlStatus;
}
/*$ END OF iDbgPdlPpSetB2b */

/*****************************************************************************
* FUNCTION NAME: prvIDbgPdlPpPortModeValidation
*
* DESCRIPTION:   
*      
*
*****************************************************************************/
static PDL_STATUS prvIDbgPdlPpPortModeValidation (
    IN UINT_32                         dev,
    IN UINT_32                         port,
    IN UINT_32                         macPort,
    IN PDL_PORT_SPEED_ENT              speed,
    IN PDL_INTERFACE_MODE_ENT          mode,
    INOUT UINT_32                   * errorCountPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                      * modeStr, * speedStr;
    BOOLEAN                                     res;
    PDL_STATUS                                  pdlStatus;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    if (errorCountPtr == NULL) {
        return PDL_BAD_PTR;
    }
    res = prvPdlLibPortModeSupported ((UINT_8)dev, macPort, speed, mode);
    if (res == FALSE) {
        if (*errorCountPtr == 0) {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("| %-5s | %-5s | %-6s | %-14s | %-14s |\n", " dev ", "port " , "speed " , "interface mode", "  Status  ");
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("| %-5s | %-5s | %-6s | %-14s | %-14s |\n", "-----", "-----" , "------" , "--------------", "----------");
        }
        pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_SPEED_TYPE_E, speed, &speedStr);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, mode, &modeStr);
        PDL_CHECK_STATUS(pdlStatus);
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("| %-5d | %-5d | %-6s | %-14s | %-14s |\n", dev, port, speedStr, modeStr, "Not supported");
        *errorCountPtr = *errorCountPtr + 1;
    }
    return PDL_OK;
}

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPpPortRunValidation
*
* DESCRIPTION:   Verify all port interface modes & speed are supported by device
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlPpPortRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                     i, dev, port, count = 0;
    PDL_STATUS						            pdlStatus, pdlStatus2;
    PDL_PORT_SPEED_ENT                          speed;
    PDL_INTERFACE_MODE_ENT                      mode;
    PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC      portAttr;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_API_OUTPUT_LOG("| %-56s |\n", "    Packet processor port interface mode validation    ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("| %-56s |\n", "-------------------------------------------------------");

    for (dev = 0; dev < 2; dev ++) {
        for (pdlStatus = pdlPpDbDevAttributesGetFirstPort(dev, &port);
             pdlStatus == PDL_OK;
             pdlStatus = pdlPpDbDevAttributesGetNextPort(dev, port, &port)) {
                pdlStatus2 = pdlPpDbPortAttributesGet(dev, port, &portAttr);
                if (pdlStatus2 == PDL_OK) {
                    for (i = 0; i < portAttr.numOfCopperModes; i++) {
                        speed = portAttr.copperModesArr[i].speed;
                        mode = portAttr.copperModesArr[i].mode;
                        pdlStatus2 = prvIDbgPdlPpPortModeValidation (dev, port, portAttr.macPort, speed, mode, &count);
                        PDL_CHECK_STATUS(pdlStatus2);
                    }
                    for (i = 0; i < portAttr.numOfFiberModes; i++) {
                        speed = portAttr.fiberModesArr[i].speed;
                        mode = portAttr.fiberModesArr[i].mode;
                        pdlStatus2 = prvIDbgPdlPpPortModeValidation (dev, port, portAttr.macPort, speed, mode, &count);
                        PDL_CHECK_STATUS(pdlStatus2);
                    }
                }
            }
    }
    if (count == 0) {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("| %-56s |\n", "Pass");
    }

    return PDL_OK;
}
/*$ END OF iDbgPdlPpPortRunValidation */


