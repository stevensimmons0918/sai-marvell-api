/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file iDbgPdlSerdes.c
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
 * @brief Debug lib inband-ports module
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
#include <pdl/lib/private/prvPdlLib.h>
#include <iDbgPdl/inband/iDbgPdlInband.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <iDbgPdl/utils/iDbgPdlUtils.h>
#include <pdl/serdes/pdlSerdes.h>
#include <pdl/sfp/pdlSfp.h>
#include <pdl/sfp/pdlSfpDebug.h>

/*****************************************************************************
* FUNCTION NAME: prvIDbgPdlInbandInfoGetWrapper
*
* DESCRIPTION: 
*
*
*****************************************************************************/
static PDL_STATUS prvIDbgPdlInbandInfoGetWrapper (

    /*!     INPUTS:             */
    IN UINT_32                                  dev,
    IN UINT_32                                  port,
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
    OUT PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC  *portDataPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_STATUS pdlStatus;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
  
    IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlPpDbPortAttributesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                                                                   IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port),
                                                                                   IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, portDataPtr));
    return pdlStatus;
}
/*$ END OF prvIDbgPdlInbandInfoGetWrapper */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandGetPortInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandGetPortInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                 i_dev, i_port, index;
    PDL_STATUS                              pdlStatus;
    PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC  portInfo;
    char                                    *transceiverTypeStr, *speedStr, *modeStr;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);
	
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | %-6s | %-11s | %-6s | %-13s | %-5s | %-13s\n", "",    "",     "FRONT",  "NUMBER","",            "",       "",              "",      "");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | %-6s | %-11s | %-6s | %-13s | %-5s | %-13s\n", "",    "",     "PANEL",  " IN"   ,"",            "COPPER", "COPPER",        "FIBER", "FIBER");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | %-6s | %-11s | %-6s | %-13s | %-5s | %-13s\n", "DEV", "PORT", "NUMBER", "GROUP" ,"TRANSCEIVER", "SPEED",  " MODE",         "SPEED", "MODE");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | %-6s | %-11s | %-6s | %-13s | %-5s | %-13s\n", "---", "----", "------", "------","-----------", "------", "-------------", "-----", "-------------");

    if (logicalPort != IDBG_PDL_ID_ALL)
    {
        i_port = logicalPort;
        i_dev = dev;
        pdlStatus = PDL_OK;
    }
    else
        pdlStatus = pdlPpDbFirstPortAttributesGet(&i_dev, &i_port);

    for (; pdlStatus == PDL_OK; pdlStatus = pdlPpDbPortAttributesGetNext(i_dev, i_port, &i_dev, &i_port))
    {
        if ((i_dev != dev ) && (logicalPort == IDBG_PDL_ID_ALL))
            continue;
        if (logicalPort != IDBG_PDL_ID_ALL && logicalPort != i_port)
            break;
        pdlStatus = prvIDbgPdlInbandInfoGetWrapper(dev, i_port,  &portInfo);
        if (pdlStatus != PDL_OK)
            continue;

        pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_TRANSCEIVER_TYPE_E, portInfo.transceiverType, &transceiverTypeStr);
        PDL_CHECK_STATUS(pdlStatus);

        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3d | %-4d | %-6d | %-6d | %-11s | ",
                                        dev, i_port, portInfo.frontPanelNumber, portInfo.portNumberInGroup, transceiverTypeStr);

        for (index = 0; index < portInfo.numOfCopperModes || index < portInfo.numOfFiberModes; index++)
        {
            if (index > 0)
            {
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n %-3s | %-4s | %-6s | %-6s | %-11s | ", "", "", "", "", "");
            }

            if (index < portInfo.numOfCopperModes)
            {
                pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_SPEED_TYPE_E, portInfo.copperModesArr[index].speed, &speedStr);
                PDL_CHECK_STATUS(pdlStatus);
                pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, portInfo.copperModesArr[index].mode, &modeStr);
                PDL_CHECK_STATUS(pdlStatus);
            }
            else
            {
                speedStr = "";
                modeStr = "";
            }

            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-6s | %-13s | ", speedStr, modeStr);

            if (index < portInfo.numOfFiberModes)
            {
                pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_SPEED_TYPE_E, portInfo.fiberModesArr[index].speed, &speedStr);
                PDL_CHECK_STATUS(pdlStatus);
                pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, portInfo.fiberModesArr[index].mode, &modeStr);
                PDL_CHECK_STATUS(pdlStatus);
            }
            else
            {
                speedStr = "";
                modeStr = "";
            }
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-5s | %-13s", speedStr, modeStr);
        }

        IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n %-3s   %-4s   %-6s   %-6s   %-11s   %-6s   %-13s   %-5s   %-13s\n", "---", "----", "------", "------", "-----------", "------", "-------------", "-----", "-------------");
    }
    return PDL_OK;
}
/*$ END OF iDbgPdlInbandGetPortInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandGetPhyInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandGetPhyInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                 i_port, firstPortId, lastPortId, portCount;
    PDL_STATUS                              pdlStatus;
    PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC  portInfo;
    char                                    phyNumStr[20], phyPosStr[20], *ifTypePtr, ifAddressStr[20], ifDevStr[20], ifIdStr[20];
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (logicalPort == IDBG_PDL_ID_ALL) {
        firstPortId = 0;
        pdlStatus = pdlPpDbDevAttributesGet(dev, &portCount);
        PDL_CHECK_STATUS(pdlStatus);
        lastPortId = portCount;
    }
    else {
        firstPortId = logicalPort;
        lastPortId = logicalPort + 1;
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | %-8s | %-9s | %-9s | %-9s | %-9s\n", ""   , ""    , ""      , "POSITION", ""         , ""         , ""         , ""         );
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | %-8s | %-9s | %-9s | %-9s | %-9s\n", "DEV", "PORT", " PHY"  , "  IN"    , "INTERFACE", "INTERFACE", "INTERFACE", "INTERFACE");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | %-8s | %-9s | %-9s | %-9s | %-9s\n", ""   , ""    , "NUMBER", "  PHY"   , "   TYPE  ", "    ID   ", " ADDRESS ", "    DEV  ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | %-8s | %-9s | %-9s | %-9s | %-9s\n", "---", "----", "------", "--------", "---------", "---------", "---------", "---------");

    for (i_port = firstPortId; i_port < lastPortId; i_port++)
    {
        pdlStatus = prvIDbgPdlInbandInfoGetWrapper(dev, i_port,  &portInfo);
        if (pdlStatus != PDL_OK)
            continue;

        if (portInfo.isPhyExists)
        {
            prvPdlOsSnprintf(phyNumStr, 20,  "%d", portInfo.phyData.phyNumber);
            prvPdlOsSnprintf(phyPosStr, 20,  "%d", portInfo.phyData.phyPosition);
            prvPdlOsSnprintf(ifAddressStr, 20, "%d", portInfo.phyData.smiXmsiInterface.address);
            prvPdlOsSnprintf(ifDevStr, 20,   "%d", portInfo.phyData.smiXmsiInterface.dev);
            prvPdlOsSnprintf(ifIdStr, 20,    "%d", portInfo.phyData.smiXmsiInterface.interfaceId);
            pdlStatus = iDbgPdlConvertEnumToString(IDBG_PDL_UTILS_INTERFACE_TYPE, portInfo.phyData.smiXmsiInterface.interfaceType, &ifTypePtr);
            PDL_CHECK_STATUS(pdlStatus);

        }
        else
        {
            prvPdlOsSnprintf(phyNumStr    , 20, "NA");
            prvPdlOsSnprintf(phyPosStr    , 20, "NA");
            prvPdlOsSnprintf(ifAddressStr , 20, "NA");
            prvPdlOsSnprintf(ifDevStr     , 20, "NA");
            prvPdlOsSnprintf(ifIdStr      , 20, "NA");
            ifTypePtr = "NA";
        }

        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3d | %-4d | %-6s | %-8s | %-9s | %-9s | %-9s | %-9s\n",
                                        dev, i_port, phyNumStr, phyPosStr, ifTypePtr, ifIdStr, ifAddressStr, ifDevStr);

        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-3s   %-4s   %-6s   %-8s   %-9s   %-9s   %-9s   %-9s\n", "---", "----", "------", "--------", "---------", "---------", "---------", "---------");
    }
    return PDL_OK;
}
/*$ END OF iDbgPdlInbandGetPhyInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandGetPhyInitInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandGetPhyInitInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                     i_port, firstPortId, lastPortId, portCount, index, numOfModes;
    PDL_STATUS                                  pdlStatus;
    PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC   phyInitValueInfo;
    PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC      portInfo;
    char                                       *transceiverTypeStr, *speedStr;
    PDL_TRANSCEIVER_TYPE_ENT                    transceiverType;
    PDL_PORT_SPEED_ENT                          speed;
    BOOLEAN                                     printDevPort, printTransceiver, printSpeed, printSeparator;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (logicalPort == IDBG_PDL_ID_ALL) {
        firstPortId = 0;
        pdlStatus = pdlPpDbDevAttributesGet(dev, &portCount);
        PDL_CHECK_STATUS(pdlStatus);
        lastPortId = portCount;
    }
    else {
        firstPortId = logicalPort;
        lastPortId = logicalPort + 1;
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | %-11s | %-5s | %-5s | %-6s | %-8s | %-6s | %-6s\n", ""   , ""    , ""      ,""           ,""     ,"PHY"  ,"Device", ""        , ""      , ""      );
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | %-11s | %-5s | %-5s | %-6s | %-8s | %-6s | %-6s\n", "DEV", "PORT", " PHY"  ,"TRANSCEIVER","SPEED","INIT" ,"  OR"  , "Register", "Value" , "Mask"  );
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | %-11s | %-5s | %-5s | %-6s | %-8s | %-6s | %-6s\n", ""   , ""    , "NUMBER",""           ,""     ,"INDEX"," Page" , "Address" , ""      , ""      );
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | %-11s | %-5s | %-5s | %-6s | %-8s | %-6s | %-6s\n", "---", "----", "------","-----------","-----","-----","------", "--------", "------", "------");

    for (i_port = firstPortId; i_port < lastPortId; i_port++)
    {
        pdlStatus = prvIDbgPdlInbandInfoGetWrapper(dev, i_port,  &portInfo);
        if (pdlStatus != PDL_OK)
            continue;

        if (portInfo.isPhyExists == FALSE)
        {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3d | %-4d | %-6s | %-11s | %-5s | %-5s | %-6s | %-8s | %-6s | %-6s\n", dev  , i_port, "NA"    ,""           ,""     , ""      , ""     , ""        , ""      , ""      );
            IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s   %-4s   %-6s   %-11s   %-5s   %-5s   %-6s   %-8s   %-6s   %-6s\n", "---", "----", "------","-----------","-----", "------", "-----", "--------", "------", "------");
            continue;
        }

        printDevPort = TRUE;
        /* run over all transceiver types */
        for (transceiverType = PDL_TRANSCEIVER_TYPE_FIBER_E; transceiverType <= PDL_TRANSCEIVER_TYPE_COPPER_E; transceiverType ++)
        {
            if (transceiverType == portInfo.transceiverType || portInfo.transceiverType == PDL_TRANSCEIVER_TYPE_COMBO_E)
            {
                pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_TRANSCEIVER_TYPE_E, transceiverType, &transceiverTypeStr);
                PDL_CHECK_STATUS(pdlStatus);

                printTransceiver = TRUE;
                if (transceiverType == PDL_TRANSCEIVER_TYPE_COPPER_E)
                    numOfModes = portInfo.numOfCopperModes;
                else
                    numOfModes = portInfo.numOfFiberModes;
                    
                for (index = 0; index < numOfModes; index++)
                {
                    if (transceiverType == PDL_TRANSCEIVER_TYPE_COPPER_E)
                    {
                        speed = portInfo.copperModesArr[index].speed;
                    }
                    else
                    {
                        speed = portInfo.fiberModesArr[index].speed;
                    }

                    printSpeed = TRUE;
                    pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_SPEED_TYPE_E, speed, &speedStr);
                    PDL_CHECK_STATUS(pdlStatus);

                    printSeparator = FALSE;
                    pdlStatus = pdlPpDbPortPhyInitValuesGetFirst(dev, i_port, speed, transceiverType, &phyInitValueInfo);
                    while (pdlStatus == PDL_OK)
                    {
                        printSeparator = TRUE;
                        if (printDevPort)
                        {
                            IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3d | %-4d | %-6d | ", dev, i_port, portInfo.phyData.phyNumber);
                            printDevPort = FALSE;
                        }
                        else
                        {
                            IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | ", "", "", "");
                        }

                        if (printTransceiver)
                        {
                            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-11s | ", transceiverTypeStr);
                            printTransceiver = FALSE;
                        }
                        else
                        {
                            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-11s | ", "");
                        }

                        if (printSpeed)
                        {
                            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-5s | ", speedStr);
                            printSpeed = FALSE;
                        }
                        else
                        {
                            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-5s | ", "");
                        }

                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-5d | %-6d | %-8d | 0x%-4X | 0x%-4X\n",
                            phyInitValueInfo.key.index,
                            phyInitValueInfo.info.devOrPage,
                            phyInitValueInfo.info.registerAddress,
                            phyInitValueInfo.info.value,
                            phyInitValueInfo.info.mask);

                        pdlStatus = pdlPpDbPortPhyInitValuesGetNext(dev, i_port, speed, transceiverType, &phyInitValueInfo);
                    }

                    if (printSeparator)
                    {
                        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s   %-4s   %-6s   %-11s   %-5s   %-5s   %-6s   %-8s   %-6s   %-6s\n", "---", "----", "------","-----------","-----", "-----", "------", "--------", "------", "------");
                        printDevPort = printTransceiver = printSpeed = TRUE;
                    }
                }
            }
        }
    }

    return PDL_OK;
}
/*$ END OF iDbgPdlInbandGetPhyInitInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandGetPhyPostInitInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandGetPhyPostInitInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                     i_port, firstPortId, lastPortId, portCount;
    PDL_STATUS                                  pdlStatus;
    PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC   phyPostInitValueInfo;
    PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC      portInfo;
    BOOLEAN                                     printDevPort, printSeparator;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (logicalPort == IDBG_PDL_ID_ALL) {
        firstPortId = 0;
        pdlStatus = pdlPpDbDevAttributesGet(dev, &portCount);
        PDL_CHECK_STATUS(pdlStatus);
        lastPortId = portCount;
    }
    else {
        firstPortId = logicalPort;
        lastPortId = logicalPort + 1;
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | %-5s | %-6s | %-8s | %-6s | %-6s\n", ""   , ""    , ""      ,"PHY"  ,"Device", ""        , ""      , ""      );
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | %-5s | %-6s | %-8s | %-6s | %-6s\n", "DEV", "PORT", " PHY"  ,"INIT" ,"  OR"  , "Register", "Value" , "Mask"  );
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | %-5s | %-6s | %-8s | %-6s | %-6s\n", ""   , ""    , "NUMBER","INDEX"," Page" , "Address" , ""      , ""      );
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | %-5s | %-6s | %-8s | %-6s | %-6s\n", "---", "----", "------","-----","------", "--------", "------", "------");

    for (i_port = firstPortId; i_port < lastPortId; i_port++)
    {
        printDevPort = TRUE;

        pdlStatus = prvIDbgPdlInbandInfoGetWrapper(dev, i_port,  &portInfo);
        if (pdlStatus != PDL_OK)
            continue;

        if (portInfo.isPhyExists == FALSE)
        {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3d | %-4d | %-6s | %-5s | %-6s | %-8s | %-6s | %-6s\n", dev  , i_port, "NA"    , ""      , ""     , ""        , ""      , ""      );
            IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s   %-4s   %-6s   %-5s   %-6s   %-8s   %-6s   %-6s\n", "---", "----", "------", "------", "-----", "--------", "------", "------");
            continue;
        }

        printSeparator = FALSE;

        pdlStatus = pdlPpDbPortPhyPostInitValuesGetFirst(dev, i_port,  &phyPostInitValueInfo);
        while (pdlStatus == PDL_OK)
        {
            printSeparator = TRUE;
            if (printDevPort)
            {
                IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3d | %-4d | %-6d | ", dev, i_port, portInfo.phyData.phyNumber);
                printDevPort = FALSE;
            }
            else
            {
                IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s | %-4s | %-6s | ", "", "", "");
            }

            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-5d | %-6d | %-8d | 0x%-4X | 0x%-4X\n",
                                            phyPostInitValueInfo.key.index,
                                            phyPostInitValueInfo.info.devOrPage,
                                            phyPostInitValueInfo.info.registerAddress,
                                            phyPostInitValueInfo.info.value,
                                            phyPostInitValueInfo.info.mask);
            pdlStatus = pdlPpDbPortPhyPostInitValuesGetNext(dev, i_port, &phyPostInitValueInfo);
        }
        if (printSeparator)
        {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-3s   %-4s   %-6s   %-5s   %-6s   %-8s   %-6s   %-6s\n", "---", "----", "------", "-----", "------", "--------", "------", "------");
        }
    }

    return PDL_OK;
}
/*$ END OF iDbgPdlInbandGetPhyPostInitInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandGetCount
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandGetCount
(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNum
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_32                         portCount;
    PDL_STATUS                      pdlStatus;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlPpDbDevAttributesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, devNum), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_32, &portCount));
    PDL_CHECK_STATUS(pdlStatus);
      
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("Number of ports: %d\n", portCount);

    return pdlStatus;
}
/*$ END OF iDbgPdlInbandGetCount */


/*****************************************************************************
* FUNCTION NAME: iDbgPdlNetworkGetFrontPanelPortInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlNetworkGetFrontPanelPortInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              groupNumber,
    IN UINT_32                              portNumberInGroup
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                 tempDev, tempPort, firstDev, firstPort, tempGroupNumber, tempNumberInGroup, tempFrontPanel;
    PDL_STATUS                              pdlStatus;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (groupNumber == IDBG_PDL_ID_ALL){   
        pdlStatus = pdlPpDbFirstPortAttributesGet(&firstDev, &firstPort);
        PDL_CHECK_STATUS(pdlStatus);
        
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-17s | %-13s | %-14s |\n", "Group Number", "Group Port Number", "pp-dev-number", "pp-port-number");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-17s | %-13s | %-14s |\n", "------------", "-----------------", "-------------", "--------------");
        
        tempDev = firstDev;
        tempPort = firstPort;        
        for (; pdlStatus == PDL_OK; pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort)){
            pdlStatus = pdlPpPortConvertDevPort2FrontPanel(tempDev, tempPort, &tempGroupNumber, &tempNumberInGroup);
            PDL_CHECK_STATUS(pdlStatus);
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12d | %-17d | %-13d | %-14d |\n",
                tempGroupNumber,
                tempNumberInGroup,
                tempDev,
                tempPort);
        }
    }
    else if (portNumberInGroup == IDBG_PDL_ID_ALL){
        pdlStatus = pdlPpDbFirstFrontPanelGet(&tempFrontPanel);
        PDL_CHECK_STATUS(pdlStatus);
        
        while (pdlStatus == PDL_OK){
            if (groupNumber == tempFrontPanel) break;
            pdlStatus = pdlPpDbFrontPanelGetNext (tempFrontPanel, &tempFrontPanel);     
        }
        
        PDL_CHECK_STATUS(pdlStatus);

        pdlStatus = pdlPpDbFirstPortAttributesGet(&firstDev, &firstPort);
        PDL_CHECK_STATUS(pdlStatus);

        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-17s | %-13s | %-14s |\n", "Group Number", "Group Port Number", "pp-dev-number", "pp-port-number");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-17s | %-13s | %-14s |\n", "------------", "-----------------", "-------------", "--------------");
        
        tempDev = firstDev;
        tempPort = firstPort;
        for (; pdlStatus == PDL_OK; pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort)){
            pdlStatus = pdlPpPortConvertDevPort2FrontPanel(tempDev, tempPort, &tempGroupNumber, &tempNumberInGroup);
            PDL_CHECK_STATUS(pdlStatus);
            if (tempGroupNumber == groupNumber) {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12d | %-17d | %-13d | %-14d |\n",
                tempGroupNumber,
                tempNumberInGroup,
                tempDev,
                tempPort);
            }
        }
    }
    else {
        tempGroupNumber = groupNumber;
        tempNumberInGroup = portNumberInGroup;
        
        pdlStatus = pdlPpPortConvertFrontPanel2DevPort(tempGroupNumber, tempNumberInGroup, &tempDev, &tempPort);
        PDL_CHECK_STATUS(pdlStatus);
        
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-17s | %-13s | %-14s |\n", "Group Number", "Group Port Number", "pp-dev-number", "pp-port-number");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-17s | %-13s | %-14s |\n", "------------", "-----------------", "-------------", "--------------");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12d | %-17d | %-13d | %-14d |\n",
            tempGroupNumber,
            tempNumberInGroup,
            tempDev,
            tempPort);
    }
         
    return PDL_OK;
}
/*$ END OF iDbgPdlNetworkGetFrontPanelPortInfo */


/*****************************************************************************
* FUNCTION NAME: iDbgPdlNetworkGetFrontPanelGroupInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlNetworkGetFrontPanelGroupInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              groupNumber
    )
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                 tempDev, tempPort, firstDev, firstPort, tempGroupNumber, tempNumberInGroup, tempFrontPanel;
    PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC   frontPanelAttributes;
    char                                  * orderPtr, *speedPtr;
    PDL_STATUS                              pdlStatus;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (groupNumber == IDBG_PDL_ID_ALL){          
        pdlStatus = pdlPpDbFirstPortAttributesGet(&firstDev, &firstPort);
        PDL_CHECK_STATUS(pdlStatus);
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-10s | %-9s | %-9s |\n", "Group Number", " Ordering ", "  Speed  ", "Port List");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-10s | %-9s | %-9s |\n", "------------", "----------", "---------", "---------");        
        tempDev = firstDev;
        tempPort = firstPort;        
        for (; pdlStatus == PDL_OK; pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort)){
            pdlStatus = pdlPpPortConvertDevPort2FrontPanel(tempDev, tempPort, &tempGroupNumber, &tempNumberInGroup);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = pdlPpDbFrontPanelAttributesGet(tempGroupNumber, &frontPanelAttributes);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_PANEL_GROUP_ORDERING_TYPE_E, frontPanelAttributes.ordering, &orderPtr);
            PDL_CHECK_STATUS(pdlStatus); 
            pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_SPEED_TYPE_E, frontPanelAttributes.portMaximumSpeed, &speedPtr);
            PDL_CHECK_STATUS(pdlStatus); 
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12d | %-10s | %-9s | %-9d |\n",
                tempGroupNumber,
                orderPtr,
                speedPtr,
                tempNumberInGroup);
        }
    }
    else {
        pdlStatus = pdlPpDbFirstFrontPanelGet(&tempFrontPanel);
        PDL_CHECK_STATUS(pdlStatus);
        while (pdlStatus == PDL_OK){
            if (groupNumber == tempFrontPanel) break;
            pdlStatus = pdlPpDbFrontPanelGetNext (tempFrontPanel, &tempFrontPanel);     
        }
        PDL_CHECK_STATUS(pdlStatus)
        pdlStatus = pdlPpDbFirstPortAttributesGet(&firstDev, &firstPort);
        PDL_CHECK_STATUS(pdlStatus);
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-10s | %-9s | %-9s |\n", "Group Number", " Ordering ", "  Speed  ", "Port List");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-10s | %-9s | %-9s |\n", "------------", "----------", "---------", "---------");
        tempDev = firstDev;
        tempPort = firstPort;
        for (; pdlStatus == PDL_OK; pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort)){
            pdlStatus = pdlPpPortConvertDevPort2FrontPanel(tempDev, tempPort, &tempGroupNumber, &tempNumberInGroup);
            PDL_CHECK_STATUS(pdlStatus);
            if (tempGroupNumber == groupNumber) {
                pdlStatus = pdlPpDbFrontPanelAttributesGet(tempGroupNumber, &frontPanelAttributes);
                PDL_CHECK_STATUS(pdlStatus);
                pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_PANEL_GROUP_ORDERING_TYPE_E, frontPanelAttributes.ordering, &orderPtr);
                PDL_CHECK_STATUS(pdlStatus); 
                pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_SPEED_TYPE_E, frontPanelAttributes.portMaximumSpeed, &speedPtr);
                PDL_CHECK_STATUS(pdlStatus); 
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12d | %-10s | %-9s | %-9d |\n",
                    tempGroupNumber,
                    orderPtr,
                    speedPtr,
                    tempNumberInGroup);
            }
        }
    }
       
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");  
   
    return PDL_OK;
}
/*$ END OF iDbgPdlNetworkGetFrontPanelGroupInfo */


/*****************************************************************************
* FUNCTION NAME: printSpeedMode
*
* DESCRIPTION: Help function to print ports speed/mode list.
*      
*
*****************************************************************************/

PDL_STATUS printSpeedMode (PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC portAttributes)
{

/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                   *speedPtr, *modePtr;
    char                                    speedModeSt[15];
    UINT_32                                 i;
    PDL_STATUS                              pdlStatus;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    switch (portAttributes.transceiverType) {
    case PDL_TRANSCEIVER_TYPE_COPPER_E:
        for (i=0; i<portAttributes.numOfCopperModes; i++){
            pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, portAttributes.copperModesArr[i].mode, &modePtr);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_SPEED_TYPE_E, portAttributes.copperModesArr[i].speed, &speedPtr);
            PDL_CHECK_STATUS(pdlStatus);
            prvPdlOsSnprintf (speedModeSt, 15, "%s, %s ",  modePtr, speedPtr);
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-16s | %-15s |%-12s |\n", "", "", "", speedModeSt, "");
        }
        break;
    case PDL_TRANSCEIVER_TYPE_FIBER_E:
        for (i=0; i<portAttributes.numOfFiberModes; i++){
            pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, portAttributes.fiberModesArr[i].mode, &modePtr);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_SPEED_TYPE_E, portAttributes.fiberModesArr[i].speed, &speedPtr);
            PDL_CHECK_STATUS(pdlStatus);
            prvPdlOsSnprintf (speedModeSt, 15, "%s, %s ",  modePtr, speedPtr);
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-16s | %-15s |%-12s |\n", "", "", "", speedModeSt, "");
        }
        break;
    case PDL_TRANSCEIVER_TYPE_COMBO_E:
        for (i=0; i<portAttributes.numOfCopperModes; i++){
            pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, portAttributes.copperModesArr[i].mode, &modePtr);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_SPEED_TYPE_E, portAttributes.copperModesArr[i].speed, &speedPtr);
            PDL_CHECK_STATUS(pdlStatus);
            prvPdlOsSnprintf (speedModeSt, 15, "%s, %s ",  modePtr, speedPtr);
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-16s | %-15s |%-12s |\n", "", "", "", speedModeSt, "");
        }
        for (i=0; i<portAttributes.numOfFiberModes; i++){
            pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, portAttributes.fiberModesArr[i].mode, &modePtr);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_SPEED_TYPE_E, portAttributes.fiberModesArr[i].speed, &speedPtr);
            PDL_CHECK_STATUS(pdlStatus);
            prvPdlOsSnprintf (speedModeSt, 15, "%s, %s ",  modePtr, speedPtr);
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-16s | %-15s |%-12s |\n", "", "", "", speedModeSt, "");
        }
        break;
    default:
        return PDL_BAD_VALUE;
        break;  
    } 
   return PDL_OK;
}

/*$ END OF printSpeedMode */



/*****************************************************************************
* FUNCTION NAME: iDbgPdlNetworkGetPacketProcessorPortInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlNetworkGetPacketProcessorPortInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              devNumber,
    IN UINT_32                              portNumberInDev
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                 tempDev, tempPort, firstDev, firstPort, tempGroupNumber, tempNumberInGroup;
    char                                  * transceiverTypePtr;
    PDL_PORT_LANE_DATA_STC                  serdesInfo;
    char                                    serdesList[12], tempSerde[8];
    BOOLEAN                                 single = FALSE, flagAll = FALSE;
    PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC  portAttributes;
    PDL_STATUS                              pdlStatus;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (devNumber == IDBG_PDL_ID_ALL){   
        pdlStatus = pdlPpDbFirstPortAttributesGet(&firstDev, &firstPort);
        PDL_CHECK_STATUS(pdlStatus); 
        flagAll = TRUE;   
    }
    else if (portNumberInDev == IDBG_PDL_ID_ALL){
        pdlStatus = pdlPpDbFirstPortAttributesGet(&firstDev, &firstPort);
        PDL_CHECK_STATUS(pdlStatus);
        tempDev = firstDev;
        tempPort = firstPort;
        while (pdlStatus == PDL_OK){
            if (devNumber == tempDev) {
                single = TRUE;
                flagAll = TRUE;
                break;
            }
            pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort);     
        }
        PDL_CHECK_STATUS(pdlStatus);       
    }
    else {

        pdlStatus = pdlPpPortConvertDevPort2FrontPanel(devNumber, portNumberInDev , &tempGroupNumber, &tempNumberInGroup);
        PDL_CHECK_STATUS(pdlStatus); 
        pdlStatus = pdlPpDbPortAttributesGet(devNumber, portNumberInDev, &portAttributes);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_TRANSCEIVER_TYPE_E, portAttributes.transceiverType, &transceiverTypePtr);
        PDL_CHECK_STATUS(pdlStatus); 
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-16s | %-15s |%-12s |\n", "pp-dev-number", "pp-port-number", "transceiver type", "speed/mode list", " serdes list");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-16s | %-15s |%-12s |\n", "-------------", "--------------", "----------------", "---------------", "------------"); 
        memset (&serdesList, '\0', sizeof (serdesList));
        pdlStatus = pdlSerdesPortDbSerdesInfoGetFirst(devNumber, portNumberInDev, &serdesInfo);
        while (pdlStatus == PDL_OK){
            prvPdlOsSnprintf (tempSerde, 8, "%d, ",  serdesInfo.absSerdesNum);
            strncat(serdesList, tempSerde, 3);
            pdlStatus = pdlSerdesPortDbSerdesInfoGetNext(devNumber, portNumberInDev, &serdesInfo, &serdesInfo);
        } 
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13d | %-14d | %-16s | %-15s |%-12s |\n", devNumber, portNumberInDev, transceiverTypePtr, "", serdesList);
        printSpeedMode(portAttributes);
    } 

    if (flagAll){
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-16s | %-15s |%-12s |\n", "pp-dev-number", "pp-port-number", "transceiver type", "speed/mode list", " serdes list");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-16s | %-15s |%-12s |\n", "-------------", "--------------", "----------------", "---------------", "------------");
        tempDev = firstDev;
        tempPort = firstPort;  
        for (; pdlStatus == PDL_OK; pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort)){
            if ((!single) || (tempDev == devNumber)) {
                pdlStatus = pdlPpDbPortAttributesGet(tempDev, tempPort, &portAttributes);
                PDL_CHECK_STATUS(pdlStatus); 
                pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_TRANSCEIVER_TYPE_E, portAttributes.transceiverType, &transceiverTypePtr);
                PDL_CHECK_STATUS(pdlStatus);
                memset (&serdesList, '\0', sizeof (serdesList));
                pdlStatus = pdlSerdesPortDbSerdesInfoGetFirst(tempDev, tempPort, &serdesInfo);
                while (pdlStatus == PDL_OK){
                    prvPdlOsSnprintf (tempSerde, 8, "%d, ",  serdesInfo.absSerdesNum);
                    strncat(serdesList, tempSerde, 3);
                    pdlStatus = pdlSerdesPortDbSerdesInfoGetNext(tempDev, tempPort, &serdesInfo, &serdesInfo);
                } 
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13d | %-14d | %-16s | %-15s |%-12s |\n", tempDev, tempPort, transceiverTypePtr, "", serdesList);                 
                printSpeedMode(portAttributes);

            }
        }
    }
   
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    
    return PDL_OK;
}
/*$ END OF iDbgPdlNetworkGetPacketProcessorPortInfo */



/*****************************************************************************
* FUNCTION NAME: iDbgPdlNetworkGetPacketProcessorPortPhyInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlNetworkGetPacketProcessorPortPhyInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              devNumber,
    IN UINT_32                              portNumberInDev
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                 tempDev, tempPort, firstDev, firstPort, tempGroupNumber, tempNumberInGroup;
    char                                  * phyTypePtr = NULL;
    char                                    phyInterfaceType[18];
    BOOLEAN                                 single = FALSE, flagAll = FALSE;
    PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC  portAttributes;
    PDL_PHY_CONFIGURATION_STC               phyConfig;
    PDL_STATUS                              pdlStatus;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (devNumber == IDBG_PDL_ID_ALL){   
        pdlStatus = pdlPpDbFirstPortAttributesGet(&firstDev, &firstPort);
        PDL_CHECK_STATUS(pdlStatus); 
        flagAll = TRUE;   
    }
    else if (portNumberInDev == IDBG_PDL_ID_ALL){
        pdlStatus = pdlPpDbFirstPortAttributesGet(&firstDev, &firstPort);
        PDL_CHECK_STATUS(pdlStatus);
        tempDev = firstDev;
        tempPort = firstPort;
        while (pdlStatus == PDL_OK){
            if (devNumber == tempDev) {
                single = TRUE;
                flagAll = TRUE;
                break;
            }
            pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort);     
        }
        PDL_CHECK_STATUS(pdlStatus);       
    }
    else {
        pdlStatus = pdlPpPortConvertDevPort2FrontPanel(devNumber, portNumberInDev, &tempGroupNumber, &tempNumberInGroup);
        PDL_CHECK_STATUS(pdlStatus); 
        pdlStatus = pdlPpDbPortAttributesGet(devNumber, portNumberInDev, &portAttributes);
        PDL_CHECK_STATUS(pdlStatus); 
        if (portAttributes.isPhyExists == TRUE){
            pdlStatus = PdlPhyDbPhyConfigurationGet (portAttributes.phyData.phyNumber, &phyConfig);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_PHY_TYPE_TYPE_E, phyConfig.phyType, &phyTypePtr);
            PDL_CHECK_STATUS(pdlStatus);
        }
        else {
            phyTypePtr = "Doesn't exist";
        } 
        switch (portAttributes.phyData.smiXmsiInterface.interfaceType) {
        case PDL_INTERFACE_TYPE_SMI_E:
            strcpy(phyInterfaceType, "smi");
            break;
        case PDL_INTERFACE_TYPE_XSMI_E:
            strcpy(phyInterfaceType, "Xsmi");
            break;
        default:
            strcpy(phyInterfaceType, "Not Found");
            break;
        } 
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-18s | %-10s | %-12s | %-18s | %-11s |\n", "pp-dev-number", "pp-port-number", "     phy type     ", "phy number", "phy position", "phy interface type", "phy address");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-18s | %-10s | %-12s | %-18s | %-11s |\n", "-------------", "--------------", "------------------", "----------", "------------", "------------------", "-----------");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13d | %-14d | %-18s | %-10d | %-12d | %-18s | %-11d |\n", devNumber, portNumberInDev, phyTypePtr, portAttributes.phyData.phyNumber, portAttributes.phyData.phyPosition, phyInterfaceType, portAttributes.phyData.smiXmsiInterface.address);
    } 
   
    if (flagAll == TRUE){
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-18s | %-10s | %-12s | %-18s | %-11s |\n", "pp-dev-number", "pp-port-number", "     phy type     ", "phy number", "phy position", "phy interface type", "phy address");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-18s | %-10s | %-12s | %-18s | %-11s |\n", "-------------", "--------------", "------------------", "----------", "------------", "------------------", "-----------");
        tempDev = firstDev;
        tempPort = firstPort;  
        for (; pdlStatus == PDL_OK; pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort)){
            if ((!single) || (tempDev == devNumber)) {
                pdlStatus = pdlPpDbPortAttributesGet(tempDev, tempPort, &portAttributes);
                PDL_CHECK_STATUS(pdlStatus); 
                if (portAttributes.isPhyExists == TRUE){
                    pdlStatus = PdlPhyDbPhyConfigurationGet (portAttributes.phyData.phyNumber, &phyConfig);
                    PDL_CHECK_STATUS(pdlStatus);
                    pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_PHY_TYPE_TYPE_E, phyConfig.phyType, &phyTypePtr);
                    PDL_CHECK_STATUS(pdlStatus);
                }
                else {
                    phyTypePtr = "Doesn't exist";
                } 
                switch (portAttributes.phyData.smiXmsiInterface.interfaceType) {
                case PDL_INTERFACE_TYPE_SMI_E:
                    strcpy(phyInterfaceType, "smi");
                    break;
                case PDL_INTERFACE_TYPE_XSMI_E:
                    strcpy(phyInterfaceType, "Xsmi");
                    break;
                default:
                    strcpy(phyInterfaceType, "Not Found");
                    break;
                } 
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13d | %-14d | %-18s | %-10d | %-12d | %-18s | %-11d |\n", tempDev, tempPort, phyTypePtr, portAttributes.phyData.phyNumber, portAttributes.phyData.phyPosition, phyInterfaceType, portAttributes.phyData.smiXmsiInterface.address);
            }
        }
    }

    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

    return PDL_OK;
}
/*$ END OF iDbgPdlNetworkGetPacketProcessorPortPhyInfo */



/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandSetPhyId
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandSetPhyId
(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNum,
    IN UINT_32                            portNum,
    IN UINT_32                            phyId
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC  portAttributes;
    PDL_STATUS                              pdlStatus;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlPpDbPortAttributesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, devNum),
                                                                     IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, portNum),
                                                                     IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &portAttributes));
    PDL_CHECK_STATUS(pdlStatus);

    if (portAttributes.isPhyExists == FALSE)
    {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Phy does not exists\n");
        return PDL_NOT_SUPPORTED;
    }

    portAttributes.phyData.phyNumber = phyId;

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlPpDbPortAttributesSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, devNum),
                                                                     IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, portNum),
                                                                     IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &portAttributes));
    return pdlStatus;
}
/*$ END OF iDbgPdlInbandSetPhyId */


/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandSetPhyPosition
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandSetPhyPosition
(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNum,
    IN UINT_32                            portNum,
    IN UINT_32                            phyPosition
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC  portAttributes;
    PDL_STATUS                              pdlStatus;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlPpDbPortAttributesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, devNum),
                                                                     IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, portNum),
                                                                     IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &portAttributes));
    PDL_CHECK_STATUS(pdlStatus);

    if (portAttributes.isPhyExists == FALSE)
    {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Phy does not exists\n");
        return PDL_NOT_SUPPORTED;
    }

    portAttributes.phyData.phyPosition = phyPosition;

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlPpDbPortAttributesSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, devNum),
                                                                     IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, portNum),
                                                                     IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &portAttributes));
    return pdlStatus;
}
/*$ END OF iDbgPdlInbandSetPhyPosition */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandSetInfoPortMode
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandSetInfoPortMode
(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNum,
    IN UINT_32                            portNum,
    IN char                              *transceiverStrPtr,
    IN char                              *speedStrPtr,
    IN char                              *modeStrPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC  portAttributes;
    PDL_TRANSCEIVER_TYPE_ENT                transceiver;
    PDL_INTERFACE_MODE_ENT                  mode;
    PDL_PORT_SPEED_ENT                      speed;
    UINT_32                                 value, index;
    PDL_STATUS                              pdlStatus;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_TRANSCEIVER_TYPE_E, transceiverStrPtr, &value);
    if (pdlStatus != PDL_OK)
    {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Unknown transceiver type. Possibly your xml file typedef section is out-of-date.\n");
        return pdlStatus;
    }
    transceiver = (PDL_TRANSCEIVER_TYPE_ENT)value;

    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_SPEED_TYPE_E, speedStrPtr, &value);
    if (pdlStatus != PDL_OK)
    {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Unknown port speed type. Possibly your xml file typedef section is out-of-date.\n");
        return pdlStatus;
    }
    speed = (PDL_PORT_SPEED_ENT)value;

    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, modeStrPtr, &value);
    if (pdlStatus != PDL_OK)
    {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Unknown port interface mode type. Possibly your xml file typedef section is out-of-date.\n");
        return pdlStatus;
    }
    mode = (PDL_INTERFACE_MODE_ENT)value;

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlPpDbPortAttributesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, devNum),
                                                                     IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, portNum),
                                                                     IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &portAttributes));
    PDL_CHECK_STATUS(pdlStatus);

    if (transceiver == PDL_TRANSCEIVER_TYPE_COMBO_E || transceiver == PDL_TRANSCEIVER_TYPE_FIBER_E)
    {
        if (portAttributes.numOfFiberModes == 0)
        {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("Fiber transceiver is not defined for this port.\n");
            return PDL_BAD_PARAM;
        }

        for (index = 0; index < portAttributes.numOfFiberModes; index ++)
        {
            if (portAttributes.fiberModesArr[index].speed == speed)
            {
                portAttributes.fiberModesArr[index].mode = mode;
                break;
            }
        }

        if (index == portAttributes.numOfFiberModes)
        {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("Speed type is not defined for this port fiber transceiver.\n");
            return PDL_BAD_PARAM;
        }
    }

    if (transceiver == PDL_TRANSCEIVER_TYPE_COMBO_E || transceiver == PDL_TRANSCEIVER_TYPE_COPPER_E)
    {
        if (portAttributes.numOfCopperModes == 0)
        {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("Copper transceiver is not defined for this port.\n");
            return PDL_BAD_PARAM;
        }


        for (index = 0; index < portAttributes.numOfCopperModes; index ++)
        {
            if (portAttributes.copperModesArr[index].speed == speed)
            {
                portAttributes.copperModesArr[index].mode = mode;
                break;
            }
        }

        if (index == portAttributes.numOfCopperModes)
        {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("Speed type is not defined for this port copper transceiver.\n");
            return PDL_BAD_PARAM;
        }
    }

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlPpDbPortAttributesSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, devNum),
                                                                     IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, portNum),
                                                                     IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &portAttributes));
    return pdlStatus;
}
/*$ END OF iDbgPdlInbandSetInfoPortMode */



/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandGetPhyPostInitListInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandGetPhyInitListInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNumber,
    IN UINT_32                            portNumberInDev,
    IN char                             * postInitOrAllInitListPtr,  
    IN char                             * transceiverPtr,  
    IN char                             * speedPtr  
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                     tempGroupNumber, tempNumberInGroup, speed, transceiver, speedValue, transceiverValue;
    PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC   initValues;
    char                                      * phyInitListStr = NULL, *transceiverStr = NULL, *speedStr = NULL;
    PDL_STATUS                                  pdlStatus;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/    
    PDL_UNUSED_PARAM(resultPtr);

    if (postInitOrAllInitListPtr == NULL){
        pdlStatus = pdlPpPortConvertDevPort2FrontPanel(devNumber, portNumberInDev, &tempGroupNumber, &tempNumberInGroup);
        PDL_CHECK_STATUS(pdlStatus); 
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-13s |\n", "pp-dev-number", "pp-port-number", "phy-init-list");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-13s |\n", "-------------", "--------------", "-------------");
        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_SPEED_TYPE_E, speedPtr, &speedValue);
        PDL_CHECK_STATUS(pdlStatus); 
        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_TRANSCEIVER_TYPE_E, transceiverPtr, &transceiverValue);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = pdlPpDbPortPhyInitValuesGetFirst(devNumber, portNumberInDev, speedValue, transceiverValue, &initValues);
        if (pdlStatus == PDL_OK)  
            phyInitListStr = "Exist";
        else 
            phyInitListStr = "Not Exist";
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13d | %-14d | %-13s |\n", devNumber, portNumberInDev, phyInitListStr);
    }
    else if (strcmp(postInitOrAllInitListPtr, "all") == 0){
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-16s | %-5s | %-13s |\n", "pp-dev-number", "pp-port-number", "Transceiver type", "Speed", "phy-init-list");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-16s | %-5s | %-13s |\n", "-------------", "--------------", "----------------", "-----", "-------------");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13d | %-14d | %-16s | %-5s | %-13s |\n", devNumber, portNumberInDev, "", "", ""); 
        for (transceiver=PDL_TRANSCEIVER_TYPE_FIBER_E; transceiver<PDL_TRANSCEIVER_TYPE_LAST_E; transceiver++){
            pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_TRANSCEIVER_TYPE_E, transceiver, &transceiverStr);
            PDL_CHECK_STATUS(pdlStatus);
            for (speed=PDL_PORT_SPEED_10_E; speed<PDL_PORT_SPEED_LAST_E; speed++){
                pdlStatus = pdlLibConvertEnumValueToString (XML_PARSER_ENUM_ID_SPEED_TYPE_E, speed, &speedStr);
                PDL_CHECK_STATUS(pdlStatus);
                pdlStatus = pdlPpDbPortPhyInitValuesGetFirst(devNumber, portNumberInDev, speed, transceiver, &initValues);
                if (pdlStatus == PDL_OK) { 
                    phyInitListStr = "Exist";
                    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-16s | %-5s | %-13s |\n", "", "", transceiverStr, speedStr, phyInitListStr); 
                }         
            }       
        }  
    }
    else if (strcmp(postInitOrAllInitListPtr, "post-init") == 0){
            pdlStatus = pdlPpDbPortPhyPostInitValuesGetFirst(devNumber, portNumberInDev, &initValues);
            if (pdlStatus == PDL_OK)
                phyInitListStr = "Exist";
            else 
                phyInitListStr = "No exist"; 
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-13s |\n", "pp-dev-number", "pp-port-number", "phy-post-init");
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-13s |\n", "-------------", "--------------", "-------------");
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13d | %-14d | %-13s |\n", devNumber, portNumberInDev, phyInitListStr);   
        }
    else return PDL_BAD_VALUE;    
   
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

    return PDL_OK;
}
/*$ END OF iDbgPdlInbandGetPhyInitListtInfo */



/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandGetPhyScriptInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandGetPhyScriptInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNumber,
    IN UINT_32                            portNumberInDev,
    IN char                             * postInitPtr,  
    IN char                             * transceiverPtr,  
    IN char                             * speedPtr  
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                     tempGroupNumber, tempNumberInGroup, speedValue, transceiverValue;
    PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC   initValues;
    PDL_STATUS                                  pdlStatus;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    pdlStatus = pdlPpPortConvertDevPort2FrontPanel(devNumber, portNumberInDev, &tempGroupNumber, &tempNumberInGroup);
    PDL_CHECK_STATUS(pdlStatus); 

    if (postInitPtr == NULL){
        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_SPEED_TYPE_E, speedPtr, &speedValue);
        PDL_CHECK_STATUS(pdlStatus); 
        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_TRANSCEIVER_TYPE_E, transceiverPtr, &transceiverValue);
        PDL_CHECK_STATUS(pdlStatus); 
        pdlStatus = pdlPpDbPortPhyInitValuesGetFirst(devNumber, portNumberInDev, speedValue, transceiverValue, &initValues);
        PDL_CHECK_STATUS(pdlStatus); 
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-11s | %-17s | %-9s | %-12s |\n", "pp-dev-number", "pp-port-number", " devOrPage ", " registerAddress ", "  value  ", "    mask    ");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-11s | %-17s | %-9s | %-12s |\n", "-------------", "--------------", "-----------", "-----------------", "---------", "------------");
        while (pdlStatus == PDL_OK){
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13d | %-14d | %-11d | 0x%-15X | 0x%-7X | 0x%-10X |\n", devNumber, portNumberInDev, initValues.info.devOrPage, initValues.info.registerAddress, initValues.info.value, initValues.info.mask);
            pdlStatus = pdlPpDbPortPhyInitValuesGetNext(devNumber, portNumberInDev, speedValue, transceiverValue, &initValues);    
        }
    }
    else {
        if (strcmp(postInitPtr, "post-init") == 0){
            pdlStatus = pdlPpDbPortPhyPostInitValuesGetFirst(devNumber, portNumberInDev, &initValues);
            PDL_CHECK_STATUS(pdlStatus); 
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-11s | %-17s | %-9s | %-12s |\n", "pp-dev-number", "pp-port-number", " devOrPage ", " registerAddress ", "  value  ", "    mask    ");
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-11s | %-17s | %-9s | %-12s |\n", "-------------", "--------------", "-----------", "-----------------", "---------", "------------");
            while (pdlStatus == PDL_OK){
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13d | %-14d | %-11d | 0x%-15X | 0x%-7X | 0x%-10X |\n", devNumber, portNumberInDev, initValues.info.devOrPage, initValues.info.registerAddress, initValues.info.value, initValues.info.mask);
                pdlStatus = pdlPpDbPortPhyPostInitValuesGetNext(devNumber, portNumberInDev, &initValues);    
            }
        }
        else IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Speed parameter is missing");    
    }

    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

    return PDL_OK;
}
/*$ END OF iDbgPdlInbandGetPhyScriptInfo */




/*****************************************************************************
* FUNCTION NAME: printSfpInfo
*
* DESCRIPTION: Help function to print sfp info.
*      
*
*****************************************************************************/

PDL_STATUS printSfpInfo (UINT_32 dev, UINT_32 logicalPort, PDL_SFP_INTERFACE_STC* sfpInterface) 
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                     sfpPresentSt[30] = {0}, sfpLossSt[30] = {0}, sfpTxDisableSt[30] = {0}, sfpTxEnableSt[30] = {0}, *interfaceTypeSt = NULL;
    PDL_STATUS               pdlStatus;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    if (sfpInterface != NULL){
        if (sfpInterface->presentInfo.isSupported){
            pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_INTERFACE_TYPE_E, sfpInterface->presentInfo.interfaceType, &interfaceTypeSt);
            PDL_CHECK_STATUS(pdlStatus);   
            prvPdlOsSnprintf (sfpPresentSt, 30, "%s id:%-4d", interfaceTypeSt, sfpInterface->presentInfo.interfaceId);
        }
        
        else prvPdlOsSnprintf(sfpPresentSt, 30, "%s", "Not Supported");
        if (sfpInterface->txDisableInfo.isSupported){
            pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_INTERFACE_TYPE_E, sfpInterface->txDisableInfo.interfaceType, &interfaceTypeSt);
            PDL_CHECK_STATUS(pdlStatus);   
            prvPdlOsSnprintf (sfpTxDisableSt, 30, "%s id:%-4d", interfaceTypeSt, sfpInterface->txDisableInfo.interfaceId);
        }
        else prvPdlOsSnprintf(sfpTxDisableSt, 30, "%s", "Not Supported");
        if (sfpInterface->txEnableInfo.isSupported){
            pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_INTERFACE_TYPE_E, sfpInterface->txEnableInfo.interfaceType, &interfaceTypeSt);
            PDL_CHECK_STATUS(pdlStatus);   
            prvPdlOsSnprintf (sfpTxEnableSt, 30, "%s id:%-4d", interfaceTypeSt, sfpInterface->txEnableInfo.interfaceId);
        }
        else prvPdlOsSnprintf(sfpTxEnableSt, 30, "%s", "Not Supported");
        if (sfpInterface->lossInfo.isSupported){
            pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_INTERFACE_TYPE_E, sfpInterface->lossInfo.interfaceType, &interfaceTypeSt);
            PDL_CHECK_STATUS(pdlStatus);   
            prvPdlOsSnprintf (sfpLossSt, 30, "%s id:%-4d", interfaceTypeSt, sfpInterface->lossInfo.interfaceId);
        }
        else prvPdlOsSnprintf(sfpLossSt, 30, "%s", "Not Supported");
    }   
    else {
        prvPdlOsSnprintf(sfpPresentSt, 30, "%s", "Not Supported");
        prvPdlOsSnprintf(sfpTxDisableSt, 30, "%s", "Not Supported");
        prvPdlOsSnprintf(sfpTxEnableSt, 30, "%s", "Not Supported");
        prvPdlOsSnprintf(sfpLossSt, 30, "%s", "Not Supported");
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13d | %-14d | %-21s | %-21s | %-21s | %-21s |\n", dev, logicalPort, sfpPresentSt, sfpLossSt, sfpTxEnableSt, sfpTxDisableSt);
    return PDL_OK;
}

/*$ END OF printSfpInfo */



/*****************************************************************************
* FUNCTION NAME: iDbgPdlNetworkGetGbicInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlNetworkGetGbicInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNumber,
    IN UINT_32                            portNumberInDev
    )
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                 tempDev, tempPort, firstDev, firstPort, tempGroupNumber, tempNumberInGroup;
    BOOLEAN                                 single = FALSE, flagAll = FALSE;
    PDL_SFP_INTERFACE_STC                   sfpInterface;
    PDL_STATUS                              pdlStatus;

    /****************************************************************************/
    /*                     F U N C T I O N   L O G I C                          */
    /****************************************************************************/ 
    PDL_UNUSED_PARAM(resultPtr);

    if (devNumber == IDBG_PDL_ID_ALL){   
        pdlStatus = pdlPpDbFirstPortAttributesGet(&firstDev, &firstPort);
        PDL_CHECK_STATUS(pdlStatus); 
        flagAll = TRUE;   
    }
    else if (portNumberInDev == IDBG_PDL_ID_ALL){
        pdlStatus = pdlPpDbFirstPortAttributesGet(&firstDev, &firstPort);
        PDL_CHECK_STATUS(pdlStatus);
        tempDev = firstDev;
        tempPort = firstPort;
        while (pdlStatus == PDL_OK){
            if (devNumber == tempDev) {
                single = TRUE;
                flagAll = TRUE;
                break;
            }
            pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort);     
        }
        PDL_CHECK_STATUS(pdlStatus);       
    }
    else {
        pdlStatus = pdlPpPortConvertDevPort2FrontPanel(devNumber, portNumberInDev , &tempGroupNumber, &tempNumberInGroup);
        PDL_CHECK_STATUS(pdlStatus); 
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-21s | %-21s | %-21s | %-21s |\n", "pp-dev-number", "pp-port-number", "   connected info    ", "         laser       ", "      tx enable      ", "      tx disable     ");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-21s | %-21s | %-21s | %-21s |\n", "-------------", "--------------", "---------------------", "---------------------", "---------------------", "---------------------"); 
        pdlStatus = PdlSfpDebugInterfaceGet(devNumber, portNumberInDev, &sfpInterface);
        if (pdlStatus != PDL_OK)
            printSfpInfo(devNumber, portNumberInDev, NULL);
        else 
            printSfpInfo(devNumber, portNumberInDev, &sfpInterface);
    } 

    if (flagAll){
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-21s | %-21s | %-21s | %-21s |\n", "pp-dev-number", "pp-port-number", "   connected info    ", "         laser       ", "      tx enable      ", "      tx disable     ");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-21s | %-21s | %-21s | %-21s |\n", "-------------", "--------------", "---------------------", "---------------------", "---------------------", "---------------------"); 
        tempDev = firstDev;
        tempPort = firstPort;  
        for (; pdlStatus == PDL_OK; pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort)){
            if ((!single) || (tempDev == devNumber)) {
                pdlStatus = PdlSfpDebugInterfaceGet(tempDev, tempPort, &sfpInterface);
                if (pdlStatus != PDL_OK)
                    printSfpInfo(tempDev, tempPort, NULL);
                else 
                    printSfpInfo(tempDev, tempPort, &sfpInterface);              
            }
        }
    }

    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

    return PDL_OK;
}
/*$ END OF iDbgPdlNetworkGetGbicInfo */



/*****************************************************************************
* FUNCTION NAME: printSfpHwInfo
*
* DESCRIPTION: Help function to print sfp-hw info.
*      
*
*****************************************************************************/

PDL_STATUS printSfpHwInfo (UINT_32 dev, UINT_32 logicalPort, UINT_32 sfpLoss,  UINT_32 sfpPresent, UINT_32 sfpTx) 
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                    * sfpPresentSt = NULL, *sfpLossSt = NULL, *sfpTxSt = NULL;

/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    switch (sfpLoss) {
    case 1:
        sfpLossSt = "True";
        break;
    case 2:
        sfpLossSt = "False";
        break;
    default:
        sfpLossSt= "Not Supported";
        break;
    } 
    switch (sfpPresent) {
    case 1:
        sfpPresentSt = "True";
        break;
    case 2:
        sfpPresentSt = "False";
        break;
    default:
        sfpPresentSt= "Not Supported";
        break;
    } 
    switch (sfpTx) {
    case 1:
        sfpTxSt = "Enable";
        break;
    case 2:
        sfpTxSt = "Disable";
        break;
    default:
        sfpTxSt= "Not Supported";
        break;
    } 
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13d | %-14d | %-14s | %-13s | %-16s | %-13s |\n", dev, logicalPort, sfpPresentSt, sfpLossSt, sfpTxSt, ""); 
    return PDL_OK;
}

/*$ END OF printSfpHwInfo */






/*****************************************************************************
* FUNCTION NAME: iDbgPdlNetworkGetHwGbicInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlNetworkGetHwGbicInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNumber,
    IN UINT_32                            portNumberInDev
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                 tempDev, tempPort, firstDev, firstPort, tempGroupNumber, tempNumberInGroup;
    BOOLEAN                                 single = FALSE, flagAll = FALSE;
    PDL_SFP_PRESENT_ENT                     sfpPresent;
    PDL_SFP_LOSS_ENT                        sfpLoss;
    PDL_SFP_TX_ENT                          sfpTx;
    PDL_STATUS                              pdlStatus;

/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/ 
    PDL_UNUSED_PARAM(resultPtr);

    if (devNumber == IDBG_PDL_ID_ALL){   
        pdlStatus = pdlPpDbFirstPortAttributesGet(&firstDev, &firstPort);
        PDL_CHECK_STATUS(pdlStatus); 
        flagAll = TRUE;   
    }
    else if (portNumberInDev == IDBG_PDL_ID_ALL){
        pdlStatus = pdlPpDbFirstPortAttributesGet(&firstDev, &firstPort);
        PDL_CHECK_STATUS(pdlStatus);
        tempDev = firstDev;
        tempPort = firstPort;
        while (pdlStatus == PDL_OK){
            if (devNumber == tempDev) {
                single = TRUE;
                flagAll = TRUE;
                break;
            }
            pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort);     
        }
        PDL_CHECK_STATUS(pdlStatus);       
    }
    else {
        pdlStatus = pdlPpPortConvertDevPort2FrontPanel(devNumber, portNumberInDev , &tempGroupNumber, &tempNumberInGroup);
        PDL_CHECK_STATUS(pdlStatus); 
        sfpLoss = sfpPresent = sfpTx = 0;
        pdlStatus = pdlSfpHwLossGet(devNumber, portNumberInDev, &sfpLoss);
        pdlStatus = pdlSfpHwPresentGet(devNumber, portNumberInDev, &sfpPresent);    
        pdlStatus = pdlSfpHwTxGet(devNumber, portNumberInDev, &sfpTx);
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-14s | %-13s | %-16s | %-13s |\n", "pp-dev-number", "pp-port-number", "connected info", "  laser   ", "tx enale/disable", "  memory  ");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-14s | %-13s | %-16s | %-13s |\n", "-------------", "--------------", "--------------", "----------", "----------------", "----------");   
        printSfpHwInfo(devNumber, portNumberInDev, sfpLoss, sfpPresent, sfpTx);
    } 

    if (flagAll){
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-14s | %-13s | %-16s | %-13s |\n", "pp-dev-number", "pp-port-number", "connected info", "   laser     ", "tx enale/disable", "   memory    ");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-14s | %-13s | %-16s | %-13s |\n", "-------------", "--------------", "--------------", "-------------", "----------------", "-------------"); 
        tempDev = firstDev;
        tempPort = firstPort;  
        for (; pdlStatus == PDL_OK; pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort)){
            if ((!single) || (tempDev == devNumber)) {    
                sfpLoss = sfpPresent = sfpTx = 0;
                pdlStatus = pdlSfpHwLossGet(tempDev, tempPort, &sfpLoss);               
                pdlStatus = pdlSfpHwPresentGet(tempDev, tempPort, &sfpPresent);                
                pdlStatus = pdlSfpHwTxGet(tempDev, tempPort, &sfpTx);
                printSfpHwInfo(tempDev, tempPort, sfpLoss, sfpPresent, sfpTx);
            }
        }
    }

    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

    return PDL_OK;
}
/*$ END OF iDbgPdlNetworkGetHwGbicInfo */



/*****************************************************************************
* FUNCTION NAME: printFrontPanelValidation
*
* DESCRIPTION: Help function to print front panel validation info.
*      
*
*****************************************************************************/

PDL_STATUS printFrontPanelValidation (UINT_32 tempDev, UINT_32 tempPort, UINT_32 convertDev, UINT_32 convertPort, UINT_32 tempGroupNumber, UINT_32 tempNumberInGroup, PDL_STATUS pdlStatus) 
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC   frontPanelAttributes;
    char                                  * groupInfoPtr = NULL, *portInfoPtr = NULL;
    
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    if ((tempDev == convertDev) && (tempPort == convertPort))
        portInfoPtr = "Pass";
    else
        pdlStatusToString(pdlStatus, &portInfoPtr);
    pdlStatus = pdlPpDbFrontPanelAttributesGet(tempGroupNumber, &frontPanelAttributes);
    if (pdlStatus == PDL_OK)
        groupInfoPtr = "Pass";
    else 
        pdlStatusToString(pdlStatus, &groupInfoPtr);            
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12d | %-11d | %-16s | %-16s |\n", tempGroupNumber, tempNumberInGroup, groupInfoPtr, portInfoPtr);
    return PDL_OK;
}

/*$ END OF printFrontPanelValidation */



/*****************************************************************************
* FUNCTION NAME: iDbgPdlNetworkRunFrontPanelValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlNetworkRunFrontPanelValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              groupNumber,
    IN UINT_32                              portNumber
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                 firstDev, firstPort, tempDev, tempPort, tempGroupNumber, tempNumberInGroup, convertDev, convertPort, tempFrontPanel;
    PDL_STATUS                              pdlStatus;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (groupNumber == IDBG_PDL_ID_ALL){          
        pdlStatus = pdlPpDbFirstPortAttributesGet(&firstDev, &firstPort);
        PDL_CHECK_STATUS(pdlStatus);
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-11s | %-16s | %-16s |\n", "Group Number", "Port Number", "   Group Info   ", "   Port Info    ");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-11s | %-16s | %-16s |\n", "------------", "-----------", "----------------", "----------------");           
        tempDev = firstDev;
        tempPort = firstPort;        
        for (; pdlStatus == PDL_OK; pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort)){
            pdlStatus = pdlPpPortConvertDevPort2FrontPanel(tempDev, tempPort, &tempGroupNumber, &tempNumberInGroup);
            pdlStatus = pdlPpPortConvertFrontPanel2DevPort(tempGroupNumber, tempNumberInGroup, &convertDev, &convertPort);
            printFrontPanelValidation (tempDev, tempPort, convertDev, convertPort, tempGroupNumber, tempNumberInGroup, pdlStatus); 
        }    
    }
    else if (portNumber == IDBG_PDL_ID_ALL) {
        pdlStatus = pdlPpDbFirstFrontPanelGet(&tempFrontPanel);
        PDL_CHECK_STATUS(pdlStatus);
        while (pdlStatus == PDL_OK){
            if (groupNumber == tempFrontPanel) break;
            pdlStatus = pdlPpDbFrontPanelGetNext (tempFrontPanel, &tempFrontPanel);     
        }
        PDL_CHECK_STATUS(pdlStatus)
        pdlStatus = pdlPpDbFirstPortAttributesGet(&firstDev, &firstPort);
        PDL_CHECK_STATUS(pdlStatus);
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-11s | %-16s | %-16s |\n", "Group Number", "Port Number", "   Group Info   ", "   Port Info    ");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-11s | %-16s | %-16s |\n", "------------", "-----------", "----------------", "----------------"); 
        tempDev = firstDev;
        tempPort = firstPort;
        for (; pdlStatus == PDL_OK; pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort)){
            pdlStatus = pdlPpPortConvertDevPort2FrontPanel(tempDev, tempPort, &tempGroupNumber, &tempNumberInGroup);
            PDL_CHECK_STATUS(pdlStatus);
            if (tempGroupNumber == groupNumber) {
                pdlStatus =  pdlPpPortConvertFrontPanel2DevPort(tempGroupNumber, tempNumberInGroup, &convertDev, &convertPort);
                printFrontPanelValidation (tempDev, tempPort, convertDev, convertPort, tempGroupNumber, tempNumberInGroup, pdlStatus);                 
            }
        }
    }
    else {      
        pdlStatus = pdlPpPortConvertFrontPanel2DevPort(groupNumber, portNumber, &tempDev, &tempPort);
        PDL_CHECK_STATUS(pdlStatus);  
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-11s | %-16s | %-16s |\n", "Group Number", "Port Number", "   Group Info   ", "   Port Info    ");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-11s | %-16s | %-16s |\n", "------------", "-----------", "----------------", "----------------"); 
        pdlStatus = pdlPpPortConvertDevPort2FrontPanel(tempDev, tempPort, &tempGroupNumber, &tempNumberInGroup);
        pdlStatus =  pdlPpPortConvertFrontPanel2DevPort(tempGroupNumber, tempNumberInGroup, &convertDev, &convertPort);
        printFrontPanelValidation (tempDev, tempPort, convertDev, convertPort, tempGroupNumber, tempNumberInGroup, pdlStatus);   
    }
 
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");  

    return PDL_OK;
}
/*$ END OF iDbgPdlNetworkRunFrontPanelValidation */



/*****************************************************************************
* FUNCTION NAME: iDbgPdlNetworkRunPacketProcessorValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlNetworkRunPacketProcessorValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              devNumber,
    IN UINT_32                              portNumberInDev
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                 tempDev, tempPort, firstDev, firstPort, tempGroupNumber, tempNumberInGroup;
    char                                  * sfpPresentSt = NULL, *sfpLossSt = NULL, *sfpTxSt = NULL;
    BOOLEAN                                 single = FALSE, flagAll = FALSE;
    PDL_SFP_PRESENT_ENT                     sfpPresent;
    PDL_SFP_LOSS_ENT                        sfpLoss;
    PDL_SFP_TX_ENT                          sfpTx;
    PDL_STATUS                              pdlStatus;

/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (devNumber == IDBG_PDL_ID_ALL){   
        pdlStatus = pdlPpDbFirstPortAttributesGet(&firstDev, &firstPort);
        PDL_CHECK_STATUS(pdlStatus); 
        flagAll = TRUE;   
    }
    else if (portNumberInDev == IDBG_PDL_ID_ALL){
        pdlStatus = pdlPpDbFirstPortAttributesGet(&firstDev, &firstPort);
        PDL_CHECK_STATUS(pdlStatus);
        tempDev = firstDev;
        tempPort = firstPort;
        while (pdlStatus == PDL_OK){
            if (devNumber == tempDev) {
                single = TRUE;
                flagAll = TRUE;
                break;
            }
            pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort);     
        }
        PDL_CHECK_STATUS(pdlStatus);       
    }
    else {
        pdlStatus = pdlPpPortConvertDevPort2FrontPanel(devNumber, portNumberInDev , &tempGroupNumber, &tempNumberInGroup);
        PDL_CHECK_STATUS(pdlStatus); 
        sfpLoss = sfpPresent = sfpTx = 0;
        pdlStatus = pdlSfpHwLossGet(devNumber, portNumberInDev, &sfpLoss);
        if (pdlStatus == PDL_OK)
            sfpLossSt = "Pass";
        else 
            pdlStatusToString(pdlStatus, &sfpLossSt);            
        pdlStatus = pdlSfpHwPresentGet(devNumber, portNumberInDev, &sfpPresent);    
        if (pdlStatus == PDL_OK)
            sfpPresentSt = "Pass";
        else 
            pdlStatusToString(pdlStatus, &sfpPresentSt);    
        if (sfpPresent == PDL_SFP_PRESENT_TRUE_E) {
            pdlStatus = pdlSfpHwTxGet(devNumber, portNumberInDev, &sfpTx);
            if (pdlStatus == PDL_OK){                
                pdlStatus =  pdlSfpHwTxSet(devNumber, portNumberInDev, PDL_SFP_TX_ENABLE_E);
                if (pdlStatus == PDL_OK){                   
                    pdlStatus = pdlSfpHwTxSet(devNumber, portNumberInDev, PDL_SFP_TX_DISABLE_E);
                    if (pdlStatus == PDL_OK){
                        pdlStatus = pdlSfpHwTxSet(devNumber, portNumberInDev, sfpTx);
                        if (pdlStatus == PDL_OK){
                            sfpTxSt = "Pass";
                        }
                        else pdlStatusToString (pdlStatus, &sfpTxSt);
                    }
                    else pdlStatusToString (pdlStatus, &sfpTxSt);
                }
                else pdlStatusToString (pdlStatus, &sfpTxSt);
            }
            else pdlStatusToString (pdlStatus, &sfpTxSt);
        }
        else sfpTxSt = "Not Supported";
         
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-14s | %-13s | %-16s | %-13s |\n", "pp-dev-number", "pp-port-number", "connected info", "  laser   ", "tx enale/disable", "  memory  ");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-14s | %-13s | %-16s | %-13s |\n", "-------------", "--------------", "--------------", "----------", "----------------", "----------");   
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13d | %-14d | %-14s | %-13s | %-16s | %-13s |\n", devNumber, portNumberInDev, sfpPresentSt, sfpLossSt, sfpTxSt, ""); 
    } 

    if (flagAll){
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-14s | %-13s | %-16s | %-13s |\n", "pp-dev-number", "pp-port-number", "connected info", "   laser     ", "tx enale/disable", "   memory    ");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13s | %-14s | %-14s | %-13s | %-16s | %-13s |\n", "-------------", "--------------", "--------------", "-------------", "----------------", "-------------"); 
        tempDev = firstDev;
        tempPort = firstPort;  
        for (; pdlStatus == PDL_OK; pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort)){
            if ((!single) || (tempDev == devNumber)) {    
                sfpLoss = sfpPresent = sfpTx = 0;
                pdlStatus = pdlSfpHwLossGet(tempDev, tempPort, &sfpLoss);
                if (pdlStatus == PDL_OK)
                    sfpLossSt = "Pass";
                else 
                    pdlStatusToString(pdlStatus, &sfpLossSt);            
                pdlStatus = pdlSfpHwPresentGet(tempDev, tempPort, &sfpPresent);    
                if (pdlStatus == PDL_OK)
                    sfpPresentSt = "Pass";
                else 
                    pdlStatusToString(pdlStatus, &sfpPresentSt);    
                if (sfpPresent == PDL_SFP_PRESENT_TRUE_E) {
                    pdlStatus = pdlSfpHwTxGet(tempDev, tempPort, &sfpTx);
                    if (pdlStatus == PDL_OK){                
                        pdlStatus =  pdlSfpHwTxSet(tempDev, tempPort, PDL_SFP_TX_ENABLE_E);
                        if (pdlStatus == PDL_OK){                   
                            pdlStatus = pdlSfpHwTxSet(tempDev, tempPort, PDL_SFP_TX_DISABLE_E);
                            if (pdlStatus == PDL_OK){
                                pdlStatus = pdlSfpHwTxSet(tempDev, tempPort, sfpTx);
                                if (pdlStatus == PDL_OK){
                                    sfpTxSt = "Pass";
                                }
                                else pdlStatusToString (pdlStatus, &sfpTxSt);
                            }
                            else pdlStatusToString (pdlStatus, &sfpTxSt);
                        }
                        else pdlStatusToString (pdlStatus, &sfpTxSt);
                    }
                    else pdlStatusToString (pdlStatus, &sfpTxSt);
                }
                else sfpTxSt = "Not Supported";   
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-13d | %-14d | %-14s | %-13s | %-16s | %-13s |\n", tempDev, tempPort, sfpPresentSt, sfpLossSt, sfpTxSt, ""); 
            }
        }
    }

    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

    return PDL_OK;
}

/*$ END OF iDbgPdlNetworkRunPacketProcessorValidation */

