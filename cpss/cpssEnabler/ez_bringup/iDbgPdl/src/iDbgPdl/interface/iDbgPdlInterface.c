/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file iDbgPdlInterface.c   
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
 * @brief Debug lib interface module
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
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <iDbgPdl/utils/iDbgPdlUtils.h>
#include <pdl/interface/pdlGpioDebug.h>
#include <pdl/interface/pdlI2CDebug.h>
#include <pdl/interface/pdlSmiXsmiDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN char                             * interfaceTypeStr,
    IN PDL_INTERFACE_TYP                  interfaceId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                          * gpioType, gpioInitValueStr[10], *i2cAcessSize;
    UINT_32                         i;
    PDL_STATUS						pdlStatus;
    PDL_INTERFACE_TYPE_ENT          interfaceType;
    PDL_INTERFACE_GPIO_STC          gpioInfo;
    PDL_INTERFACE_MUX_STC           muxInfo;
    PDL_INTERFACE_I2C_STC           i2cInfo;
    PDL_INTERFACE_SMI_XSMI_STC      smiInfo;    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    pdlStatus = iDbgPdlConvertStringToEnum (IDBG_PDL_UTILS_INTERFACE_TYPE, interfaceTypeStr, (UINT_32*) &interfaceType);
    PDL_CHECK_STATUS(pdlStatus);
    switch (interfaceType) {
    case PDL_INTERFACE_TYPE_GPIO_E:
        pdlStatus = PdlGpioDebugDbGetAttributes(interfaceId, &gpioInfo);
        PDL_CHECK_STATUS(pdlStatus);
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-11s | %-10s | %-26s | %-10s |\n", "Interface-ID", "GPIO Device", "Pin Number", "            Type          ", "Init Value");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-11s | %-10s | %-26s | %-10s |\n", "------------", "-----------", "----------", "--------------------------", "----------");
        pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_MPP_PIN_MODE_TYPE_E, gpioInfo.type, &gpioType);
        PDL_CHECK_STATUS(pdlStatus);
        if (gpioInfo.initialValue == PDL_INTERFACE_MPP_INIT_VALUE_HW_E) {
            prvPdlOsSnprintf (gpioInitValueStr, 10, "N/A");
        }
        else {
            prvPdlOsSnprintf(gpioInitValueStr, 10, "%d", gpioInfo.initialValue);
        }
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12d | %-11d | %-10d | %-26s | %-10s |\n", interfaceId, gpioInfo.dev, gpioInfo.pinNumber, gpioType, gpioInitValueStr);
        break;
    case PDL_INTERFACE_TYPE_I2C_E:
        pdlStatus = PdlI2CDebugDbGetAttributes(interfaceId, &i2cInfo);
        PDL_CHECK_STATUS(pdlStatus);
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-10s | %-7s | %-11s | %-8s | %-10s | %-9s |\n", "Interface-ID", "I2C Bus-Id", "Address", "Access Size", " Offset ", "   Mask   ", "Mux Count");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12s | %-10s | %-7s | %-11s | %-8s | %-10s | %-9s |\n", "------------", "----------", "-------", "-----------", "--------", "----------", "---------");
        pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_I2C_ACCESS_TYPE_E, i2cInfo.accessSize, &i2cAcessSize);
        PDL_CHECK_STATUS(pdlStatus);      
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-12d | %-10d | 0x%02x    | %-11s | 0x%04x   | 0x%08x | %-9d |\n", interfaceId, i2cInfo.busId, i2cInfo.i2cAddress, i2cAcessSize, i2cInfo.offset, i2cInfo.mask, i2cInfo.numberOfMuxes-1);
        /* show all muxes */
        if (i2cInfo.numberOfMuxes > 1) {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-7s | %-4s | %-20s |\n", "Mux Num", "Type", "      Attribute     ");
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-7s | %-4s | %-20s |\n", "-------", "----", "--------------------");
            for (i = 1; i < i2cInfo.numberOfMuxes; i++) {
                pdlStatus = PdlI2CDebugDbGetMuxAttributes(interfaceId, i, &muxInfo);
                PDL_CHECK_STATUS(pdlStatus);
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("#%-6d | %-4s | %-20s |\n", i, muxInfo.interfaceType==PDL_INTERFACE_TYPE_GPIO_E ? "GPIO":"I2C", "");
                switch (muxInfo.interfaceType) {
                case PDL_INTERFACE_TYPE_GPIO_E:
                    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-7s | %-4s | %-8s: %-10d |\n", "", "", "Device:", muxInfo.interfaceInfo.gpioInterface.dev);
                    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-7s | %-4s | %-8s: %-10d |\n", "", "", "Pin:", muxInfo.interfaceInfo.gpioInterface.pinNumber);
                    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-7s | %-4s | %-8s: %-10d |\n", "", "", "Value:", muxInfo.interfaceInfo.gpioInterface.pushValue);
                    break;
                case PDL_INTERFACE_TYPE_I2C_E:
                    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-7s | %-4s | %-8s: 0x%02x       |\n", "", "", "Address:", muxInfo.interfaceInfo.i2cMux.baseAddress);
                    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-7s | %-4s | %-8s: 0x%08x |\n", "", "", "Mask:", muxInfo.interfaceInfo.i2cMux.mask);
                    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-7s | %-4s | %-8s: 0x%04x     |\n", "", "", "Offset:", muxInfo.interfaceInfo.i2cMux.offset);
                    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-7s | %-4s | %-8s: %-10d |\n", "", "", "Value:", muxInfo.interfaceInfo.i2cMux.value);
                    break;
                default:
                    return PDL_BAD_VALUE;
                }
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-7s | %-4s | -------------------- |\n", "", "");
            }
        }
        break;
    case PDL_INTERFACE_TYPE_SMI_E:
    case PDL_INTERFACE_TYPE_XSMI_E:
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-14s | %-6s | %-15s | %-8s |\n", "Interface Type", "Device", "Phy InterfaceId", "Address ");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-14s | %-6s | %-15s | %-8s |\n", "--------------", "------", "---------------", "--------");
        pdlStatus = pdlSmiXsmiDebugDbGetAttributes(interfaceId, &smiInfo);
        PDL_CHECK_STATUS(pdlStatus);
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-14s | %-6d | %-15d | %-8d |\n", smiInfo.interfaceType==PDL_INTERFACE_TYPE_SMI_E ?"SMI":"XSMI",
                                                                          smiInfo.dev, smiInfo.interfaceId, smiInfo.address);
        break;
    default:
        return PDL_BAD_PARAM;
    }
    return PDL_OK;
}
/*$ END OF iDbgPdlInterfaceGetInfo */


/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceGetHw
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceGetHw (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN char                             * interfaceTypeStr,
    IN PDL_INTERFACE_TYP                  interfaceId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         value;
    PDL_STATUS						pdlStatus;
    PDL_INTERFACE_TYPE_ENT          interfaceType;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    pdlStatus = iDbgPdlConvertStringToEnum (IDBG_PDL_UTILS_INTERFACE_TYPE, interfaceTypeStr, (UINT_32*) &interfaceType);
    PDL_CHECK_STATUS(pdlStatus);
    if (interfaceType == PDL_INTERFACE_TYPE_SMI_E || interfaceType == PDL_INTERFACE_TYPE_XSMI_E) {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Please use show interface hw smi/xsmi with smi/xsmi specific parameters");
        return PDL_BAD_PARAM;
    }
    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, prvPdlInterfaceHwGetValue ,IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceType),
                                                                      IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId),
                                                                      IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &value));
    PDL_CHECK_STATUS(pdlStatus);
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("HW Value: %d\n", value);
    return PDL_OK;

}

/*$ END OF iDbgPdlInterfaceGetHw */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceSmiXsmiGetHw
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceSmiXsmiGetHw (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_INTERFACE_TYP                  interfaceId,
    IN UINT_32                            phyDeviceOrPage,
    IN UINT_32                            regAddress
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_16                         value;
    PDL_STATUS						pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    pdlStatus = pdlSmiXsmiHwGetValue(interfaceId, (UINT_8) phyDeviceOrPage, (UINT_16) regAddress, &value);
    PDL_CHECK_STATUS(pdlStatus);   
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("HW Value: %d\n", value);
    return PDL_OK;

}

/*$ END OF iDbgPdlInterfaceGetHw */


/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceSetHw
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceSetHw (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN char                             * interfaceTypeStr,
    IN PDL_INTERFACE_TYP                  interfaceId,
    IN UINT_32                            value
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS						pdlStatus;
    PDL_INTERFACE_TYPE_ENT          interfaceType;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    pdlStatus = iDbgPdlConvertStringToEnum (IDBG_PDL_UTILS_INTERFACE_TYPE, interfaceTypeStr, (UINT_32*) &interfaceType);
    PDL_CHECK_STATUS(pdlStatus);
    if (interfaceType == PDL_INTERFACE_TYPE_SMI_E || interfaceType == PDL_INTERFACE_TYPE_XSMI_E) {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Please use set interface hw smi/xsmi with smi/xsmi specific parameters");
        return PDL_BAD_PARAM;
    }
    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, prvPdlInterfaceHwSetValue ,IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceType),
                                                                      IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId),
                                                                      IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value));
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;

}

/*$ END OF iDbgPdlInterfaceSetHw */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceSmiXsmiSetHw
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceSmiXsmiSetHw (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_INTERFACE_TYP                  interfaceId,
    IN UINT_32                            phyDeviceOrPage,
    IN UINT_32                            regAddress,
    IN UINT_32                            mask,
    IN UINT_32                            value
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS						pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    pdlStatus = pdlSmiXsmiHwSetValue(interfaceId, (UINT_8) phyDeviceOrPage, (UINT_16) regAddress, (UINT_16) mask, (UINT_16) value);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;

}

/*$ END OF iDbgPdlInterfaceSmiXsmiSetHw */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceGpioSetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceGpioSetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_INTERFACE_TYP                  interfaceId,
    IN UINT_32                            paramMask,
    IN UINT_32                            devId,
    IN UINT_32                            pinNumber,
    IN char                             * typeStr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         typeId;
    PDL_STATUS						pdlStatus;
    PDL_INTERFACE_GPIO_STC          attributesInfo;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    attributesInfo.dev = (UINT_8) devId;
    attributesInfo.pinNumber = (UINT_8) pinNumber;
    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_MPP_PIN_MODE_TYPE_E, typeStr, (UINT_32*) &typeId);
    if (pdlStatus != PDL_OK) {
        typeId = 0;
    }
    attributesInfo.type = typeId;
    IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, PdlGpioDebugDbSetAttributes ,IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId),
                                                                                      IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, paramMask),
                                                                                      IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &attributesInfo));
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;

}

/*$ END OF iDbgPdlInterfaceGpioSetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceI2cSetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceI2cSetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_INTERFACE_TYP                  interfaceId,
    IN UINT_32                            paramMask,
    IN UINT_32                            busId,
    IN UINT_32                            i2cAddress,
    IN char                             * accessSizeStr,
    IN UINT_32                            offset,
    IN UINT_32                            mask
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         accessSize;
    PDL_STATUS						pdlStatus;
    PDL_INTERFACE_I2C_STC           attributesInfo;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_I2C_ACCESS_TYPE_E, accessSizeStr, (UINT_32*) &accessSize);
    if (pdlStatus != PDL_OK) {
        accessSize = 0;
    }
    attributesInfo.accessSize = accessSize;
    attributesInfo.busId = busId;
    attributesInfo.i2cAddress = (UINT_8) i2cAddress;
    attributesInfo.mask = mask;
    attributesInfo.offset = (UINT_16) offset;

    IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, PdlI2CDebugDbSetAttributes ,IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId),
                                                                                     IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, paramMask),
                                                                                     IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &attributesInfo));
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;

}

/*$ END OF iDbgPdlInterfaceI2cSetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceMuxGpioSetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceMuxGpioSetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_INTERFACE_TYP                  interfaceId,
    IN UINT_32                            muxId,
    IN UINT_32                            paramMask,
    IN UINT_32                            devId,
    IN UINT_32                            pinNumber,
    IN UINT_32                            value
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS						pdlStatus;
    PDL_INTERFACE_MUX_STC           muxInfo;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    muxInfo.interfaceInfo.gpioInterface.dev = (UINT_8) devId;
    muxInfo.interfaceInfo.gpioInterface.pinNumber = (UINT_8) pinNumber;
    muxInfo.interfaceInfo.gpioInterface.pushValue = (UINT_8) value;
    IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, PdlI2CDebugDbSetMuxAttributes ,IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId),
                                                                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, muxId),
                                                                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, paramMask),
                                                                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &muxInfo));
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;

}

/*$ END OF iDbgPdlInterfaceMuxGpioSetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceMuxI2CSetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceMuxI2CSetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_INTERFACE_TYP                  interfaceId,
    IN UINT_32                            muxId,
    IN UINT_32                            paramMask,
    IN UINT_32                            i2cAddress,
    IN UINT_32                            offset,
    IN UINT_32                            mask,
    IN UINT_32                            value
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS						pdlStatus;
    PDL_INTERFACE_MUX_STC           muxInfo;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    muxInfo.interfaceInfo.i2cMux.baseAddress = (UINT_8) i2cAddress;
    muxInfo.interfaceInfo.i2cMux.offset = (UINT_16) offset;
    muxInfo.interfaceInfo.i2cMux.mask = mask;
    muxInfo.interfaceInfo.i2cMux.value = value;

    IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, PdlI2CDebugDbSetMuxAttributes ,IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId),
                                                                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, muxId),
                                                                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, paramMask),
                                                                                        IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &muxInfo));
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;

}

/*$ END OF iDbgPdlInterfaceMuxI2CSetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceSmiXsmiSetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceSmiXsmiSetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_INTERFACE_TYP                  interfaceId,
    IN UINT_32                            paramMask,
    IN UINT_32                            smiDev,
    IN UINT_32                            phyInterfaceId,
    IN UINT_32                            smiAddress
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS						pdlStatus;
    PDL_INTERFACE_SMI_XSMI_STC      smiInfo;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    smiInfo.dev = (UINT_8) smiDev;
    smiInfo.interfaceId = (UINT_8) phyInterfaceId;
    smiInfo.address = (UINT_8) smiAddress;
    IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlSmiXsmiDebugDbSetAttributes ,IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceId),
                                                                                         IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, paramMask),
                                                                                         IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &smiInfo));
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;

}

/*$ END OF iDbgPdlInterfaceSmiXsmiSetInfo */
