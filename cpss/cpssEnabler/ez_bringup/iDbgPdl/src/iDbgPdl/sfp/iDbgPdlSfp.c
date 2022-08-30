/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file iDbgPdlSfp.c
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
 * @brief Debug lib sfp module
 * 
 * @version   1 
********************************************************************************
*/
/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/sfp/pdlSfp.h>
#include <pdl/sfp/pdlSfpDebug.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <iDbgPdl/sfp/iDbgPdlSfp.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <iDbgPdl/utils/iDbgPdlUtils.h>

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSfpGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlSfpGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i_port, tempDev, tempPort, firstDev, lastDev;
	PDL_STATUS						pdlStatus;
    BOOLEAN                         portListArr[128];
    PDL_SFP_INTERFACE_STC           sfpInfo;
    PDL_PP_XML_ATTRIBUTES_STC       ppAttributes;
    PDL_PP_XML_B2B_ATTRIBUTES_STC   b2bAttributes;
    char                           *str;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (dev == IDBG_PDL_ID_ALL)
    {
        firstDev = 0;
        lastDev = firstDev + 2;
    }
    else
    {
        firstDev = dev;
        lastDev = firstDev + 1;
    }

    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6s | %-4s | %-15s | %-15s\n", "DEVICE", "PORT", "     TYPE"     , "     DATA");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6s   %-4s   %-15s   %-15s\n", "------", "----", "--------------", "--------------------");

    for (tempDev = firstDev; tempDev < lastDev; tempDev ++)
    {
        memset(portListArr, 0, sizeof(portListArr));

        for (pdlStatus = pdlPpDbDevAttributesGetFirstPort(tempDev, &tempPort);
             pdlStatus == PDL_OK;
             pdlStatus = pdlPpDbDevAttributesGetNextPort(tempDev, tempPort, &tempPort))
        {
            if (tempPort >= sizeof(portListArr))
            {
                IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Illegal port number encountered: dev %lu port %lu\n", tempDev, tempPort);
                return PDL_OUT_OF_RANGE;
            }

            if ((logicalPort == IDBG_PDL_ID_ALL) || (logicalPort == tempPort))
            {
                portListArr[tempPort] = TRUE;
                if (logicalPort == tempPort)
                    break;
            }
        }

        pdlStatus = pdlPpDbAttributesGet(&ppAttributes);
        PDL_CHECK_STATUS(pdlStatus);
        for (tempPort = 0; tempPort < ppAttributes.numOfBackToBackLinksPerPp; tempPort++)
        {
            pdlStatus = pdlPpDbB2bAttributesGet(tempPort, &b2bAttributes);
            PDL_CHECK_STATUS(pdlStatus);
            if (b2bAttributes.firstDev == tempDev)
            {
                if (b2bAttributes.firstPort >= sizeof(portListArr))
                {
                    IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Illegal b2b left port number encountered: dev %lu port %lu\n", b2bAttributes.firstDev, b2bAttributes.firstPort);
                    return PDL_OUT_OF_RANGE;
                }
                if ((logicalPort == IDBG_PDL_ID_ALL) || (logicalPort == b2bAttributes.firstPort))
                {
                    portListArr[b2bAttributes.firstPort] = TRUE;
                }
            }
            if (b2bAttributes.secondDev == tempDev)
            {
                if (b2bAttributes.secondPort >= sizeof(portListArr))
                {
                    IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Illegal b2b left port number encountered: dev %lu port %lu\n", b2bAttributes.secondDev, b2bAttributes.secondPort);
                    return PDL_OUT_OF_RANGE;
                }
                if ((logicalPort == IDBG_PDL_ID_ALL) || (logicalPort == b2bAttributes.secondPort))
                {
                    portListArr[b2bAttributes.secondPort] = TRUE;
                }
            }
        }

        for (i_port = 0; i_port < sizeof(portListArr); i_port++)
        {
            if (portListArr[i_port] == FALSE)
                continue;

            IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, PdlSfpDebugInterfaceGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempDev),
                                                                                          IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i_port),
                                                                                          IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &sfpInfo));
            if (pdlStatus == PDL_OK)
            {
                if (iDbgPdlConvertEnumToString(IDBG_PDL_UTILS_INTERFACE_TYPE, sfpInfo.eepromInfo.interfaceType, &str) != PDL_OK)
                    str = "parsing error";
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6d | %-4d | %-15s | \n", tempDev, i_port, "EEPROM INFO");
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | %-15s: %d\n", "------", "----", "", "Interface Id", sfpInfo.eepromInfo.interfaceId);
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | %-15s: %s\n", "", "", "", "Interface Type", str);
                if (iDbgPdlConvertEnumToString(IDBG_PDL_UTILS_INTERFACE_TYPE, sfpInfo.lossInfo.interfaceType, &str) != PDL_OK)
                    str = "parsing error";
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | \n", "", "", "LOSS INFO");
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | %-15s: %d\n", "", "", "", "Interface Id", sfpInfo.lossInfo.interfaceId);
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | %-15s: %s\n", "", "", "", "Interface Type", str);
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | %-15s: %d\n", "", "", "", "Value", sfpInfo.values.lossValue);

                if (iDbgPdlConvertEnumToString(IDBG_PDL_UTILS_INTERFACE_TYPE, sfpInfo.presentInfo.interfaceType, &str) != PDL_OK)
                    str = "parsing error";
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | \n", "", "", "PRESENT INFO");
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | %-15s: %d\n", "", "", "", "Interface Id", sfpInfo.presentInfo.interfaceId);
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | %-15s: %s\n", "", "", "", "Interface Type", str);
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | %-15s: %d\n", "", "", "", "Value", sfpInfo.values.presentValue);

                if (iDbgPdlConvertEnumToString(IDBG_PDL_UTILS_INTERFACE_TYPE, sfpInfo.txDisableInfo.interfaceType, &str) != PDL_OK)
                    str = "parsing error";
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | \n", "", "", "TX DISABLE INFO");
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | %-15s: %d\n", "", "", "", "Interface Id", sfpInfo.txDisableInfo.interfaceId);
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | %-15s: %s\n", "", "", "", "Interface Type", str);
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | %-15s: %d\n", "", "", "", "Value", sfpInfo.values.txDisableValue);

                if (iDbgPdlConvertEnumToString(IDBG_PDL_UTILS_INTERFACE_TYPE, sfpInfo.txEnableInfo.interfaceType, &str) != PDL_OK)
                    str = "parsing error";
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | \n", "", "", "TX ENABLE INFO");
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | %-15s: %d\n", "", "", "", "Interface Id", sfpInfo.txEnableInfo.interfaceId);
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | %-15s: %s\n", "", "", "", "Interface Type", str);
                IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | %-15s | %-15s: %d\n", "", "", "", "Value", sfpInfo.values.txEnableValue);

                IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6s   %-4s   %-15s   %-15s\n", "------", "----", "--------------", "--------------------");
            }
        }
    }
    return PDL_OK;
}
/*$ END OF iDbgPdlSfpGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSfpGetHwInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
#define PRV_IDBGPDL_SHOW_DEVPORT_MAC(flag, dev, port)   \
    if (flag)\
    {\
        IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6d | %-4d | ", dev, port);\
        flag = FALSE;\
    }\
	else\
	{\
		IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-6s   %-4s | ", "", "");\
	}

PDL_STATUS iDbgPdlSfpGetHwInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i_port, tempDev, tempPort, firstDev, lastDev;
	PDL_STATUS						pdlStatus;
    BOOLEAN                         portListArr[128], showDevPort;
    PDL_PP_XML_ATTRIBUTES_STC       ppAttributes;
    PDL_PP_XML_B2B_ATTRIBUTES_STC   b2bAttributes;
    PDL_SFP_LOSS_ENT                lossStatus;
    PDL_SFP_TX_ENT                  txStatus;
    PDL_SFP_PRESENT_ENT             presentStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (dev == IDBG_PDL_ID_ALL)
    {
        firstDev = 0;
        lastDev = firstDev + 2;
    }
    else
    {
        firstDev = dev;
        lastDev = firstDev + 1;
    }

    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6s | %-4s | %-15s | %-15s\n", "DEVICE", "PORT", "     TYPE"     , "     DATA");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6s   %-4s   %-15s   %-15s\n", "------", "----", "--------------", "--------------------");

    for (tempDev = firstDev; tempDev < lastDev; tempDev ++)
    {
        memset(portListArr, 0, sizeof(portListArr));

        for (pdlStatus = pdlPpDbDevAttributesGetFirstPort(tempDev, &tempPort);
             pdlStatus == PDL_OK;
             pdlStatus = pdlPpDbDevAttributesGetNextPort(tempDev, tempPort, &tempPort))
        {
            if (tempPort >= sizeof(portListArr))
            {
                IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Illegal port number encountered: dev %lu port %lu\n", tempDev, tempPort);
                return PDL_OUT_OF_RANGE;
            }

            if ((logicalPort == IDBG_PDL_ID_ALL) || (logicalPort == tempPort))
            {
                portListArr[tempPort] = TRUE;
                if (logicalPort == tempPort)
                    break;
            }
        }

        pdlStatus = pdlPpDbAttributesGet(&ppAttributes);
        PDL_CHECK_STATUS(pdlStatus);
        for (tempPort = 0; tempPort < ppAttributes.numOfBackToBackLinksPerPp; tempPort++)
        {
            pdlStatus = pdlPpDbB2bAttributesGet(tempPort, &b2bAttributes);
            PDL_CHECK_STATUS(pdlStatus);
            if (b2bAttributes.firstDev == tempDev)
            {
                if (b2bAttributes.firstPort >= sizeof(portListArr))
                {
                    IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Illegal b2b left port number encountered: dev %lu port %lu\n", b2bAttributes.firstDev, b2bAttributes.firstPort);
                    return PDL_OUT_OF_RANGE;
                }
                if ((logicalPort == IDBG_PDL_ID_ALL) || (logicalPort == b2bAttributes.firstPort))
                {
                    portListArr[b2bAttributes.firstPort] = TRUE;
                }
            }
            if (b2bAttributes.secondDev == tempDev)
            {
                if (b2bAttributes.secondPort >= sizeof(portListArr))
                {
                    IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Illegal b2b left port number encountered: dev %lu port %lu\n", b2bAttributes.secondDev, b2bAttributes.secondPort);
                    return PDL_OUT_OF_RANGE;
                }
                if ((logicalPort == IDBG_PDL_ID_ALL) || (logicalPort == b2bAttributes.secondPort))
                {
                    portListArr[b2bAttributes.secondPort] = TRUE;
                }
            }
        }

        for (i_port = 0; i_port < sizeof(portListArr); i_port++)
        {
            if (portListArr[i_port] == FALSE)
                continue;

            showDevPort = TRUE;

            IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlSfpHwLossGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempDev),
                                                                                  IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i_port),
                                                                                  IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_32, &lossStatus));
            if (pdlStatus == PDL_OK)
            {
                PRV_IDBGPDL_SHOW_DEVPORT_MAC(showDevPort, tempDev, i_port);
                IDBG_PDL_LOGGER_API_OUTPUT_LOG ("%-15s | %-15s: %s\n", "LOSS DATA", "Status", lossStatus == PDL_SFP_LOSS_TRUE_E ? "true" : "false");
            }

            IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlSfpHwTxGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempDev),
                                                                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i_port),
                                                                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_32, &txStatus));
            if (pdlStatus == PDL_OK)
            {
                PRV_IDBGPDL_SHOW_DEVPORT_MAC(showDevPort, tempDev, i_port);
                IDBG_PDL_LOGGER_API_OUTPUT_LOG ("%-15s | %-15s: %s\n", "TX DATA", "Enabled", txStatus == PDL_SFP_TX_ENABLE_E ? "true" : "false");
            }

            IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlSfpHwPresentGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempDev),
                                                                                     IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i_port),
                                                                                     IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_32, &presentStatus));
            if (pdlStatus == PDL_OK)
            {
                PRV_IDBGPDL_SHOW_DEVPORT_MAC(showDevPort, tempDev, i_port);
                IDBG_PDL_LOGGER_API_OUTPUT_LOG ("%-15s | %-15s: %s\n", "PRESENT DATA", "Present", presentStatus == PDL_SFP_PRESENT_TRUE_E ? "true" : "false");
            }

            if (showDevPort == FALSE)
            {
                IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6s   %-4s   %-15s   %-15s\n", "------", "----", "--------------", "--------------------");
            }
        }
    }
    return PDL_OK;
}
/*$ END OF iDbgPdlSfpGetHwInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSfpSetInfo
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlSfpSetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort,
    IN char                             * sfpValueTypeStr,
    IN char                             * interfaceTypeStr,
    IN UINT_32                            interfaceId,
    IN UINT_32                            value /* if needed */
)
{
/*****************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*****************************************************************************/
    PDL_STATUS                      pdlStatus;
    PDL_INTERFACE_TYPE_ENT          interfaceType;
    PDL_SFP_VALUE_TYPE_ENT          sfpValueType;
    PDL_SFP_INTERFACE_STC           interfaceStc;
/*****************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/*****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    pdlStatus = iDbgPdlConvertStringToEnum(IDBG_PDL_UTILS_SFP_VALUE_TYPE, sfpValueTypeStr, (UINT_32*) &sfpValueType);
    if (pdlStatus != PDL_OK)
    {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d port %d] illegal sfp value type [%s]\n", dev, logicalPort, sfpValueTypeStr);
        return pdlStatus;
    }

    pdlStatus = iDbgPdlConvertStringToEnum(IDBG_PDL_UTILS_INTERFACE_TYPE, interfaceTypeStr, (UINT_32*) &interfaceType);
    if (pdlStatus != PDL_OK)
    {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d port %d] illegal interface type [%s]\n", dev, logicalPort, interfaceTypeStr);
        return pdlStatus;
    }

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus,
                                PdlSfpDebugInterfaceGet,
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, logicalPort),
                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &interfaceStc));
    if (pdlStatus != PDL_OK)
    {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d port %d] sfp data not found\n", dev, logicalPort);
        return pdlStatus;
    }

    switch (sfpValueType)
    {
    	case PDL_SFP_VALUE_EEPROM_E:
    		interfaceStc.eepromInfo.interfaceId = interfaceId;
    		interfaceStc.eepromInfo.interfaceType = interfaceType;
    		break;
    	case PDL_SFP_VALUE_LOSS_E:
    		interfaceStc.lossInfo.interfaceId = interfaceId;
    		interfaceStc.lossInfo.interfaceType = interfaceType;
    		interfaceStc.values.lossValue = value;
    		break;
    	case PDL_SFP_VALUE_PRESENT_E:
    		interfaceStc.presentInfo.interfaceId = interfaceId;
    		interfaceStc.presentInfo.interfaceType = interfaceType;
    		interfaceStc.values.presentValue = value;
    		break;
    	case PDL_SFP_VALUE_TX_ENABLE_E:
    		interfaceStc.txEnableInfo.interfaceId = interfaceId;
    		interfaceStc.txEnableInfo.interfaceType = interfaceType;
    		interfaceStc.values.txEnableValue = value;
    		break;
    	case PDL_SFP_VALUE_TX_DISABLE_E:
    		interfaceStc.txDisableInfo.interfaceId = interfaceId;
    		interfaceStc.txDisableInfo.interfaceType = interfaceType;
    		interfaceStc.values.txDisableValue = value;
			break;
    	default:
    		IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d port %d] illegal sfp value type [%s]\n", dev, logicalPort, sfpValueTypeStr);
    		return PDL_BAD_PARAM;
    }

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus,
                                PdlSfpDebugInterfaceSet,
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, logicalPort),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &interfaceStc));
    if (pdlStatus != PDL_OK)
    {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d port %d] bad input parameter. [interface type %s] [interface id %d]",
                                         dev, logicalPort, interfaceTypeStr, interfaceId);
        if ( sfpValueType != PDL_SFP_VALUE_EEPROM_E )
        {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("[value %d]", value);
		}
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
        return pdlStatus;
    }

    return PDL_OK;
}
/*$ END OF iDbgPdlSfpSetInfo */
/*****************************************************************************
* FUNCTION NAME: iDbgPdlSfpHwEepromRead
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlSfpHwEepromRead (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort,
    IN UINT_8                             i2cAddress,
    IN UINT_16                            offset,
    IN UINT_32                            length
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
	UINT_32      readLen;
    UINT_16      i = 0, j;
	UINT_8       regReadVal[32];
	PDL_STATUS	 pdlStatus;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_API_OUTPUT_LOG("device %d port %d addr 0x%X offset 0x%X:\n", dev, logicalPort, i2cAddress, offset);
	while (i < length)
	{
		readLen = (length - i <= 32) ? length - i : 32;
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[0x%02X] ", i + offset);
        IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlSfpHwEepromRead, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                                                   IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, logicalPort),
                                                                   IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_8, i2cAddress),
                                                                   IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_16, offset + i),
                                                                   IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, readLen),
                                                                   IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &regReadVal[0]));
        if (pdlStatus == PDL_OK)
        {
			for (j = 0; j < readLen; j++)
			{
				IDBG_PDL_LOGGER_API_OUTPUT_LOG("%02X ", regReadVal[j]);
			}
			IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
        }
        i += (UINT_16) readLen;
	}
    return PDL_OK;
}
/*$ END OF iDbgPdlSfpHwEepromRead */