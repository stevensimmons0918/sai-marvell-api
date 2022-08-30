/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlGpio.c   
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
 * @brief Platform driver layer - Gpio related API
 * 
 * @version   1 
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/common/pdlTypes.h>
#include <pdl/interface/private/prvPdlGpio.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <pdl/packet_processor/private/prvPdlPacketProcessor.h>
#include <pdl/interface/private/prvPdlGpio.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <pdl/interface/pdlGpioDebug.h>
#include <pdl/cpu/pdlCpu.h>
#ifdef LINUX_HW
    #include <sys/mman.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
#endif

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/
/*! Zero array helper */
#define PDL_PP_INIT_REGS_ZERO_ARRAY       {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0}}
/*! Zero array helper */
#define PDL_PP_TYPE_REGS_ZERO_ARRAY       {{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0}}
/** @brief   Number of pdl gpio interfaces */
static UINT_32                  prvPdlGpioInterfaceCount = 0; /* counter used as an ID for new interfaces */
UINT_32                         pdlGpioMppCount = 0;          /* counter used as an ID for new interfaces */
/** @brief   The pdl gpio database */
PDL_DB_PRV_STC                  pdlGpioDb;
PDL_DB_PRV_STC                  pdlGpioMppDb;
/** @brief   The pdl gpio memory CPU registers */
static UINTPTR                  pdlGpioMemCpuRegisters = 0;
/** @brief   Type of the pdl gpio CPU */
static PDL_CPU_FAMILY_TYPE_ENT  pdlGpioCpuType;
/** @brief   The pdl gpio mpp mask[ 8] */
UINT_32                         pdlGpioMppMask[8] = {0xF, 0xF0, 0xF00, 0xF000, 0xF0000, 0xF00000, 0xF000000, 0xF0000000};
BOOLEAN                         pdlMppInitialized = FALSE;

/** @brief   The pdl gpio offset string to enum pairs[] */
static PRV_PDL_LIB_STR_TO_ENUM_STC pdlGpioOffsetStrToEnumPairs[] = {
    {"output", PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E},
    {"type", PDL_INTERFACE_GPIO_OFFSET_TYPE_E},
    {"blinking", PDL_INTERFACE_GPIO_OFFSET_BLINKING_E},
    {"input", PDL_INTERFACE_GPIO_OFFSET_INPUT_E},
};
/** @brief   The pdl gpio offset string to enum */
static PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC pdlGpioOffsetStrToEnum = {pdlGpioOffsetStrToEnumPairs, sizeof(pdlGpioOffsetStrToEnumPairs) / sizeof(PRV_PDL_LIB_STR_TO_ENUM_STC)};

#ifdef LINUX_HW
static INT_32                          memfd = -1;
#endif

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/* ***************************************************************************
* FUNCTION NAME: prvPdlGpioHwGetValue
*
* DESCRIPTION:   get gpio value
*
* PARAMETERS:   
*                
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlGpioHwGetValue ( IN PDL_INTERFACE_TYP interfaceId, IN PDL_INTERFACE_GPIO_OFFSET_ENT offset, OUT UINT_32 * dataPtr )
 *
 * @brief   Pdl gpio hardware get value
 *
 * @param           interfaceId Identifier for the interface.
 * @param           offset      The offset.
 * @param [in,out]  dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlGpioHwGetValue (
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  PDL_INTERFACE_GPIO_OFFSET_ENT           offset,
    OUT UINT_32                               * dataPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                     gpioOutputRegister, data, pinNormalized;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_GPIO_DB_STC             * gpioPtr;
    PDL_INTERFACE_PRV_KEY_STC                   gpioKey;
    char                                      * offsetStr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (pdlMppInitialized == FALSE) {
        prvPdlGpioMppInit();
        pdlMppInitialized = TRUE;
    }
    gpioKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&pdlGpioDb, (void*) &gpioKey, (void**) &gpioPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (dataPtr == NULL) {
        return PDL_BAD_PTR;
    }
    PDL_GPIO_GET_BASE_REG_MAC (gpioPtr->publicInfo.pinNumber, gpioOutputRegister);
    if (gpioOutputRegister == 0) {
        return PDL_ERROR;
    }
    gpioOutputRegister += offset;
    prvPdlLibEnumToStrConvert (&pdlGpioOffsetStrToEnum, offset, &offsetStr);

    if (gpioPtr->publicInfo.dev == PDL_GPIO_CPU_DEV) {
        if (pdlGpioMemCpuRegisters == 0) {
#ifdef LINUX_HW
            return PDL_NOT_IMPLEMENTED;
#else
            return PDL_OK;
#endif
        }
        data = *((volatile UINT_32*)(pdlGpioMemCpuRegisters + gpioOutputRegister));
        PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__,"%s reading CPU MPP#%d from address [0x%x] value is [0x%x] offset type [%s]\n", __FUNCTION__, gpioPtr->publicInfo.pinNumber, (pdlGpioMemCpuRegisters + gpioOutputRegister), data, offsetStr);
    }
    /* packet processor GPIO */
    else {
        pdlStatus = prvPdlCmRegRead ((GT_U8)gpioPtr->publicInfo.dev, gpioOutputRegister, PDL_MASK_32_BIT, &data);
        PDL_CHECK_STATUS(pdlStatus);
        PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__,"%s - reading dev#%d MPP#%d from address [0x%x] value is [0x%x] offset type [%s]\n", __FUNCTION__, gpioPtr->publicInfo.dev, gpioPtr->publicInfo.pinNumber, gpioOutputRegister, data, offsetStr);
    }
    /* apply the pin number mask */
    pinNormalized = gpioPtr->publicInfo.pinNumber % 32;
    *dataPtr = (data & (1 << pinNormalized)) >> pinNormalized;    
    return PDL_OK;
}

/*$ END OF prvPdlGpioHwGetValue */

/* ***************************************************************************
* FUNCTION NAME: prvPdlGpioPinHwSetValue
*
* DESCRIPTION:   set gpio value, used also in I2C for GPIO muxes
*
* PARAMETERS:   
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlGpioPinHwSetValue ( IN UINT_32 dev, IN UINT_32 pinNumber, IN PDL_INTERFACE_GPIO_OFFSET_ENT offset, IN UINT_32 data )
 *
 * @brief   Prv pdl gpio hardware set value
 *
 * @param   dev         The development.
 * @param   pinNumber   The pin number.
 * @param   offset      The offset.
 * @param   data        The data.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlGpioPinHwSetValue (
    IN  UINT_32                                 dev,
    IN  UINT_32                                 pinNumber,
    IN  PDL_INTERFACE_GPIO_OFFSET_ENT           offset,
    IN  UINT_32                                 data             /* input data can be 0 or 1*/
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                     value = 0, gpioOutputRegister, pinNormalized;
    char                                      * offsetStr;
    PDL_STATUS                                  pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (data > 1) {
        return PDL_BAD_VALUE;
    }

    PDL_GPIO_GET_BASE_REG_MAC (pinNumber, gpioOutputRegister);
    pinNormalized = pinNumber % 32;
    prvPdlLibEnumToStrConvert (&pdlGpioOffsetStrToEnum, offset, &offsetStr);

    if (gpioOutputRegister == 0) {
        return PDL_ERROR;
    }
    gpioOutputRegister += offset;

    if (dev == PDL_GPIO_CPU_DEV) {
        if (pdlGpioMemCpuRegisters == 0) {
#ifdef LINUX_HW
            return PDL_NOT_IMPLEMENTED;
#else
            return PDL_OK;
#endif
        }
        value |= *((volatile UINT_32*)(pdlGpioMemCpuRegisters + gpioOutputRegister)) & ~(1 << pinNormalized);
        value |= data << pinNormalized;
        PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__,"%s - writing CPU MPP#%d value=[%d] to address [0x%x] offset type [%s]\n", __FUNCTION__, pinNumber, data, (pdlGpioMemCpuRegisters + gpioOutputRegister), offsetStr);
        *((volatile UINT_32*)(pdlGpioMemCpuRegisters + gpioOutputRegister)) = value;
    }
    /* packet processor GPIO */
    else {
        value = data << pinNormalized;
        PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__,"%s - writing PP#%d MPP#%d value=[0%d] to address [0x%x] offset type [%s]\n", __FUNCTION__, dev, pinNumber, data, (pdlGpioMemCpuRegisters + gpioOutputRegister), offsetStr);
        pdlStatus = prvPdlCmRegWrite ((GT_U8)dev, gpioOutputRegister, 1 << pinNormalized, value);
        PDL_CHECK_STATUS(pdlStatus);
    }
    return PDL_OK;
}

/*$ END OF prvPdlGpioPinHwSetValue */

/* ***************************************************************************
* FUNCTION NAME: prvPdlGpioHwSetValue
*
* DESCRIPTION:   set gpio value
*
* PARAMETERS:   
*                
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlGpioHwSetValue ( IN PDL_INTERFACE_TYP interfaceId, IN PDL_INTERFACE_GPIO_OFFSET_ENT offset, IN UINT_32 data )
 *
 * @brief   Pdl gpio hardware set value
 *
 * @param   interfaceId Identifier for the interface.
 * @param   offset      The offset.
 * @param   data        The data.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlGpioHwSetValue (
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  PDL_INTERFACE_GPIO_OFFSET_ENT           offset,
    IN  UINT_32                                 data             /* input data will can be 0 or 1*/
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_GPIO_DB_STC             * gpioPtr;
    PDL_INTERFACE_PRV_KEY_STC                   gpioKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (pdlMppInitialized == FALSE) {
        prvPdlGpioMppInit();
        pdlMppInitialized = TRUE;
    }
    gpioKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&pdlGpioDb, (void*) &gpioKey, (void**) &gpioPtr);
    if (pdlStatus != PDL_OK) {
        return pdlStatus;
    }
    if (gpioPtr->publicInfo.type == PDL_INTERFACE_GPIO_TYPE_READ_E) {
        return PDL_SET_ERROR;
    }
    return prvPdlGpioPinHwSetValue (gpioPtr->publicInfo.dev, gpioPtr->publicInfo.pinNumber, offset, data);
}

/*$ END OF prvPdlGpioHwSetValue */

/*$ END OF prvPdlGpioHwSetValue */

/* ***************************************************************************
* FUNCTION NAME: WrapPdlGpioHwSetValue
*
* DESCRIPTION:   wrapper to set gpio value, used also in I2C for GPIO muxes
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlGpioDebugHwSetValue ( IN UINT_32 dev, IN UINT_32 pinNumber, IN UINT_32 data )
 *
 * @brief   Wrap pdl gpio hardware set value
 *
 * @param   dev         The development.
 * @param   pinNumber   The pin number.
 * @param   data        The data.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlGpioDebugHwSetValue (
    IN  UINT_32                                 dev,
    IN  UINT_32                                 pinNumber,
    IN  PDL_INTERFACE_GPIO_OFFSET_ENT           offset,
    IN  UINT_32                                 data             /* input data can be 0 or 1*/
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    return prvPdlGpioPinHwSetValue (dev, pinNumber, offset, data);
}

/*$ END OF WrapPdlGpioHwSetValue */

/* ***************************************************************************
* FUNCTION NAME: PdlGpioDebugDbGetAttributes
*
* DESCRIPTION:  get GPIO attributes
*               return all GPIO relevant information for given interfaceId
*               based on information from XML
*
* PARAMETERS:   
*                
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS PdlGpioDebugDbGetAttributes ( IN PDL_INTERFACE_TYP interfaceId, OUT PDL_INTERFACE_GPIO_STC * attributesPtr )
 *
 * @brief   Pdl gpio debug database get attributes
 *
 * @param           interfaceId     Identifier for the interface.
 * @param [in,out]  attributesPtr   If non-null, the attributes pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS PdlGpioDebugDbGetAttributes (
    IN  PDL_INTERFACE_TYP                    interfaceId,
    OUT PDL_INTERFACE_GPIO_STC             * attributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_GPIO_DB_STC             * gpioPtr;
    PDL_INTERFACE_PRV_KEY_STC                   gpioKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    gpioKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&pdlGpioDb, (void*) &gpioKey, (void**) &gpioPtr);
    if (pdlStatus != PDL_OK) {
        return pdlStatus;
    }
    if (attributesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    memcpy (attributesPtr, &gpioPtr->publicInfo, sizeof(PDL_INTERFACE_GPIO_STC));
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, gpioPtr->publicInfo.dev);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, gpioPtr->publicInfo.initialValue);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, gpioPtr->publicInfo.pinNumber);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, gpioPtr->publicInfo.pushValue);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, gpioPtr->publicInfo.type);
    return PDL_OK;
}
/*$ END OF PdlGpioDebugDbGetAttributes */

/* ***************************************************************************
* FUNCTION NAME: PdlGpioDebugDbSetAttributes
*
* DESCRIPTION:  set GPIO attributes
*               update GPIO relevant information for given interfaceId
*               based on information from XML
*
* PARAMETERS:   
*                
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS PdlGpioDebugDbSetAttributes ( IN PDL_INTERFACE_TYP interfaceId, OUT PDL_INTERFACE_GPIO_STC * attributesPtr )
 *
 * @brief   Pdl gpio debug database set attributes
 *
 * @param           interfaceId     Identifier for the interface.
 * @param [in,out]  attributesPtr   If non-null, the attributes pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS PdlGpioDebugDbSetAttributes (
    IN  PDL_INTERFACE_TYP                    interfaceId,
    IN  UINT_32                              attributesMask,
    OUT PDL_INTERFACE_GPIO_STC             * attributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_GPIO_DB_STC             * gpioPtr;
    PDL_INTERFACE_PRV_KEY_STC                   gpioKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    gpioKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&pdlGpioDb, (void*) &gpioKey, (void**) &gpioPtr);
    if (pdlStatus != PDL_OK) {
        return pdlStatus;
    }
    if (attributesPtr == NULL) {
        return PDL_BAD_PTR;
    }
    if (attributesMask & PDL_GPIO_SET_ATTRIBUTE_MASK_DEVICE) {
        gpioPtr->publicInfo.dev = attributesPtr->dev;
    }
    if (attributesMask & PDL_GPIO_SET_ATTRIBUTE_MASK_PIN_NUMBER) {
        gpioPtr->publicInfo.pinNumber = attributesPtr->pinNumber;
    }
    if (attributesMask & PDL_GPIO_SET_ATTRIBUTE_MASK_TYPE) {
        gpioPtr->publicInfo.type = attributesPtr->type;
    }
    
    return PDL_OK;
}
/*$ END OF PdlGpioDebugDbSetAttributes */

/* ***************************************************************************
* FUNCTION NAME: prvPdlGpioXmlIsGpio
*
* DESCRIPTION:  returns TRUE if xmlId is pointing to a GPIO interface
*
* PARAMETERS:   
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlGpioXmlIsGpio ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT BOOLEAN * isGpio )
 *
 * @brief   Prv pdl gpio XML is gpio
 *
 * @param           xmlId   Identifier for the XML.
 * @param [in,out]  isGpio  If non-null, true if is gpio, false if not.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlGpioXmlIsGpio (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    OUT BOOLEAN                               * isGpio
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char        pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    UINT_32     maxSize = PDL_XML_MAX_TAG_LEN;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (isGpio == NULL) {
        return PDL_BAD_PTR;
    }
    xmlParserGetName(xmlId, &maxSize, &pdlTagStr[0]);
    if (strcmp (pdlTagStr, PDL_GPIO_READ_ONLY_TAG_NAME) == 0 || strcmp(pdlTagStr, PDL_GPIO_WRITE_TAG_NAME) == 0) {
        *isGpio = TRUE;
    }
    else {
        *isGpio = FALSE;
    }

    return PDL_OK;
}
/*$ END OF prvPdlGpioXmlIsGpio */

/* ***************************************************************************
* FUNCTION NAME: prvPdlGpioPinXmlParser
*
* DESCRIPTION:  parse XML instance of gpio pin (for i2c mux)
*
* PARAMETERS:   
*
* REMARKS:      Used by pdlI2cXmlParser
*               
*
* XML structure:
*
*       <pin>        
*           <pin-number> 5 </pin-number>
*           <push-value> 0 </push-value>
*       </pin>

*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlGpioPinXmlParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT UINT_32 * pinNumberPtr, OUT UINT_32 * valuePtr )
 *
 * @brief   Prv pdl gpio pin XML parser
 *
 * @param           xmlId           Identifier for the XML.
 * @param [in,out]  devNumberPtr    If non-null, the dev number pointer.
 * @param [in,out]  pinNumberPtr    If non-null, the pin number pointer.
 * @param [in,out]  valuePtr        If non-null, the value pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlGpioPinXmlParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP  xmlId,
    OUT UINT_32	                      * devNumberPtr,
    OUT UINT_32	                      * pinNumberPtr,
    OUT UINT_32	                      * valuePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    UINT_32                             mask = 0, maxSize;
    XML_PARSER_NODE_DESCRIPTOR_TYP      xmlMuxChildId;
    XML_PARSER_RET_CODE_TYP             xmlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (pinNumberPtr == NULL || valuePtr == NULL) {
        return PDL_ERROR;
    }

    xmlStatus = xmlParserGetFirstChild (xmlId, &xmlMuxChildId);
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);
        maxSize = PDL_XML_MAX_TAG_LEN;
        xmlParserGetName(xmlMuxChildId, &maxSize, &pdlTagStr[0]);
        if (strstr (pdlTagStr, PDL_GPIO_MUX_PIN_NUMBER_NAME) != 0) {
            maxSize = sizeof (UINT_32);
            xmlStatus = xmlParserGetValue (xmlMuxChildId, &maxSize, pinNumberPtr);
            XML_CHECK_STATUS(xmlStatus);
            mask |= 1;
        }
        else if (strstr (pdlTagStr, PDL_GPIO_MUX_PIN_VALUE_NAME) != 0) {
            maxSize = sizeof (UINT_32);
            xmlStatus = xmlParserGetValue (xmlMuxChildId, &maxSize, valuePtr);
            XML_CHECK_STATUS(xmlStatus);
            mask |= 1 << 1;
        }
        else if (strstr (pdlTagStr, PDL_GPIO_MUX_PIN_DEV_NAME) != 0) {
            maxSize = sizeof (UINT_32);
            xmlStatus = xmlParserGetValue (xmlMuxChildId, &maxSize, devNumberPtr);
            XML_CHECK_STATUS(xmlStatus);
            mask |= 1 << 2;
        }
        else {
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling(xmlMuxChildId, &xmlMuxChildId);
    }
    if (mask == 0x7) {
        return PDL_OK;
    }
    else {
        return PDL_XML_PARSE_ERROR;
    }
}

/*$ END OF prvPdlI2cXmlMuxParser */

/* ***************************************************************************
* FUNCTION NAME: prvPdlGpioXmlParser
*
* DESCRIPTION:  parse XML instance of GPIO interface, insert it into DB and return a reference to it
*
* PARAMETERS:   
*                
* XML structure:
*<gpio-group>
*   <gpio-read-only-interface>
*	    <device-number type=gpio-device-number-type>255</device-number>
*		<pin-number type=gpio-pin-number-type>14</pin-number>
*	</gpio-read-only-interface>
*	<push-value type=mpp-pin-value-type>0</push-value>			
*</gpio-group>	
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlGpioXmlParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT PDL_INTERFACE_TYP * interfaceIdPtr )
 *
 * @brief   Prv pdl gpio XML parser
 *
 * @param           xmlId           Identifier for the XML.
 * @param [in,out]  interfaceIdPtr  If non-null, the interface identifier pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlGpioXmlParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    OUT PDL_INTERFACE_TYP                     * interfaceIdPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                        pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    BOOLEAN                                     pinFound = FALSE, devFound = FALSE, mppFound = FALSE;
    PDL_STATUS                                  pdlStatus;
    XML_PARSER_RET_CODE_TYP                     xmlStatus;
    PDL_INTERFACE_PRV_GPIO_DB_STC               gpioInfo;
    PDL_INTERFACE_PRV_GPIO_DB_STC             * gpioPtr = NULL;
    PDL_INTERFACE_PRV_KEY_STC                   gpioKey;
    XML_PARSER_NODE_DESCRIPTOR_TYP              xmlGpioChildId;
    UINT_32                                     maxSize, i;
    PDL_INTERFACE_PRV_GPIO_DB_STC             * mppPtr;
    PDL_INTERFACE_PRV_KEY_STC                   mppKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfaceIdPtr == NULL) {
        return PDL_BAD_PTR;
    }
    maxSize = PDL_XML_MAX_TAG_LEN;
    xmlStatus = xmlParserGetName(xmlId, &maxSize, &pdlTagStr[0]);
    XML_CHECK_STATUS(xmlStatus);    
    /* xmliD not pointing to gpio-group*/
    if (strcmp (pdlTagStr, PDL_GPIO_READ_ONLY_TAG_NAME) != 0 && strcmp (pdlTagStr, PDL_GPIO_WRITE_TAG_NAME) != 0) {
        return PDL_ERROR;
    }

    /* go over all gpio child tags */
    xmlStatus = xmlParserGetFirstChild (xmlId, &xmlGpioChildId);
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);    
        maxSize = PDL_XML_MAX_TAG_LEN;
        xmlParserGetName(xmlGpioChildId, &maxSize, &pdlTagStr[0]);
        if ((strcmp (pdlTagStr, PDL_GPIO_PIN_READ_NUMBER_NAME) == 0) || (strcmp (pdlTagStr, PDL_GPIO_PIN_WRITE_NUMBER_NAME) == 0)) {
            maxSize = sizeof(gpioInfo.publicInfo.pinNumber);
            xmlStatus = xmlParserGetValue (xmlGpioChildId, &maxSize, &gpioInfo.publicInfo.pinNumber);
            XML_CHECK_STATUS(xmlStatus);
            pinFound = TRUE;
        }
        else if ((strcmp (pdlTagStr, PDL_GPIO_PIN_READ_DEV_NAME) == 0) || (strcmp (pdlTagStr, PDL_GPIO_PIN_WRITE_DEV_NAME) == 0)) {
            maxSize = sizeof(gpioInfo.publicInfo.dev);
            xmlStatus = xmlParserGetValue (xmlGpioChildId, &maxSize, &gpioInfo.publicInfo.dev);
            XML_CHECK_STATUS(xmlStatus);
            devFound = TRUE;
        }
        else {
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling(xmlGpioChildId, &xmlGpioChildId);
    }

    /* value is not mandatory */
    if (pinFound == FALSE || devFound == FALSE) {
        return PDL_XML_PARSE_ERROR;
    }

    /* find matching MPP pin and update gpio type */
    for (i = 0 ; i < pdlGpioMppCount; i++) {
        mppKey.interfaceId = i;
        pdlStatus  = prvPdlDbFind (&pdlGpioMppDb, (void*) &mppKey, (void**) &mppPtr);
        if (pdlStatus != PDL_OK) {
            PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__,"%s can't find mppId=[%d] !!! MPP AREN'T INITIALIZED !!!\n", __FUNCTION__, i);
            return pdlStatus;
        }
        if (mppPtr->publicInfo.pinNumber == gpioInfo.publicInfo.pinNumber && mppPtr->publicInfo.dev == gpioInfo.publicInfo.dev) {
            gpioInfo.publicInfo.type = mppPtr->publicInfo.type;
            gpioInfo.publicInfo.initialValue = mppPtr->publicInfo.initialValue;
            mppFound = TRUE;
            break;
        }
    }
    if (mppFound == FALSE) {
        PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__,"%s can't find matching MPP for gpio pin #[%d] on dev[%d] !!!\n", __FUNCTION__, gpioInfo.publicInfo.pinNumber, gpioInfo.publicInfo.dev);
        return PDL_XML_PARSE_ERROR;
    }

    gpioKey.interfaceId = prvPdlGpioInterfaceCount++;
    pdlStatus = prvPdlDbAdd (&pdlGpioDb, (void*) &gpioKey, (void*) &gpioInfo, (void**) &gpioPtr);
    PDL_CHECK_STATUS(pdlStatus);

    *interfaceIdPtr = gpioKey.interfaceId;
    return PDL_OK;
}
/*$ END OF prvPdlGpioXmlParser */

/**
 * @fn  PDL_STATUS prvPdlGpioMppXmlParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId)
 *
 * @brief   Parse XML MPP section
 *
 * @param           xmlId           Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlGpioMppXmlParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                        pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    PDL_STATUS                                  pdlStatus;
    XML_PARSER_RET_CODE_TYP                     xmlStatus, xmlStatus2;
    PDL_INTERFACE_PRV_GPIO_DB_STC               mppInfo;
    PDL_INTERFACE_PRV_GPIO_DB_STC             * mppPtr = NULL;
    PDL_INTERFACE_PRV_KEY_STC                   mppKey;
    XML_PARSER_NODE_DESCRIPTOR_TYP              xmlMppId, xmlMppDeviceId, xmlMppDeviceInfoId, xmlMppPinId, xmlMppPinInfoId;
    UINT_32                                     maxSize;
    UINT_8                                      mppPin, mppDev = 0;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    mppKey.interfaceId = 0;
    xmlStatus = xmlParserFindByName (xmlId, "MPP-GPIO-pins", &xmlMppId);
    /* section is not mandatory */
    if (xmlStatus == XML_PARSER_RET_CODE_NOT_FOUND) {
        return PDL_OK;
    }
    XML_CHECK_STATUS(xmlStatus);
    /* get to device tag */
    xmlStatus = xmlParserGetFirstChild (xmlMppId, &xmlMppDeviceId);
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        xmlStatus = xmlParserGetFirstChild (xmlMppDeviceId, &xmlMppDeviceInfoId);
        XML_CHECK_STATUS(xmlStatus);
        if (xmlParserIsEqualName (xmlMppDeviceInfoId, "mpp-device-number") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(mppDev);
            xmlStatus2 = xmlParserGetValue(xmlMppDeviceInfoId, &maxSize, &mppDev);
            XML_CHECK_STATUS(xmlStatus2);
        }
        xmlStatus2 = xmlParserGetNextSibling (xmlMppDeviceInfoId, &xmlMppPinId);
        while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
            XML_CHECK_STATUS(xmlStatus2);
            mppInfo.publicInfo.initialValue = PDL_INTERFACE_MPP_INIT_VALUE_HW_E;
            xmlStatus2 = xmlParserGetFirstChild (xmlMppPinId, &xmlMppPinInfoId);
            while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                if (xmlParserIsEqualName (xmlMppPinInfoId, "pin-number") == XML_PARSER_RET_CODE_OK) {
                    maxSize = sizeof(mppPin);
                    xmlStatus2 = xmlParserGetValue(xmlMppPinInfoId, &maxSize, &mppInfo.publicInfo.pinNumber);
                    XML_CHECK_STATUS(xmlStatus2);
                }
                else if (xmlParserIsEqualName (xmlMppPinInfoId, "pin-mode") == XML_PARSER_RET_CODE_OK) {
                    maxSize = PDL_XML_MAX_TAG_LEN;
                    xmlParserGetValue(xmlMppPinInfoId, &maxSize, &pdlTagStr[0]);
                    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_MPP_PIN_MODE_TYPE_E, pdlTagStr, (UINT_32*) &mppInfo.publicInfo.type);
                    PDL_CHECK_STATUS(pdlStatus);      
                }
                else if (xmlParserIsEqualName (xmlMppPinInfoId, "pin-description") == XML_PARSER_RET_CODE_OK) {
                    maxSize = PDL_XML_MAX_TAG_LEN;
                    xmlParserGetValue(xmlMppPinInfoId, &maxSize, &pdlTagStr[0]);
                    /*pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_MPP_PIN_MODE_TYPE_E, pdlTagStr, (UINT_32*) &mppInfo.publicInfo.type);
                    PDL_CHECK_STATUS(pdlStatus);      */
                }
                else if (xmlParserIsEqualName (xmlMppPinInfoId, "pin-initial-value") == XML_PARSER_RET_CODE_OK) {
                    maxSize = PDL_XML_MAX_TAG_LEN;
                    xmlStatus = xmlParserGetValue (xmlMppPinInfoId, &maxSize, &pdlTagStr[0]);
                    XML_CHECK_STATUS(xmlStatus);
                    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_MPP_PIN_INIT_VALUE_TYPE_E, pdlTagStr, (UINT_32*) &mppInfo.publicInfo.initialValue);
                    PDL_CHECK_STATUS (pdlStatus);
                }
                else {
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus2 = xmlParserGetNextSibling (xmlMppPinInfoId, &xmlMppPinInfoId);
            }
            mppInfo.publicInfo.dev = mppDev;
            pdlStatus = prvPdlDbAdd (&pdlGpioMppDb, (void*) &mppKey, (void*) &mppInfo, (void**) &mppPtr);
            xmlStatus2 = xmlParserGetNextSibling (xmlMppPinId, &xmlMppPinId);
            mppKey.interfaceId++;
        }
        xmlStatus = xmlParserGetNextSibling (xmlMppDeviceId, &xmlMppDeviceId);
    }
    pdlGpioMppCount = mppKey.interfaceId;
    return PDL_OK;
}

/*$ END OF prvPdlGpioMppXmlParser */

/* ***************************************************************************
* FUNCTION NAME: prvPdlGpioCpuTypeInit
*
* DESCRIPTION:   Initialize CPU type
*
* PARAMETERS:   
*                
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlGpioCpuTypeInit ( IN void )
 *
 * @brief   Prv pdl gpio CPU type initialize
 *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlGpioCpuTypeInit (
    IN          void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
#ifdef LINUX_HW
	UINT_32 len;
    char buf[2000];
    char cpu_info_path[] = "/proc/cpuinfo";
    FILE* fd = NULL;
    char cpu_msys[] = "Marvell Msys";
    char cpu_axp[]  = "Marvell Armada XP";
    char cpu_a38x[] = "Marvell Armada 38";
#endif
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
#define MIN(a,b) (((a)<(b))?(a):(b))
#ifdef LINUX_HW
    fd = fopen(cpu_info_path,"r");
    if (NULL == fd) {
        pdlGpioCpuType = PDL_CPU_FAMILY_TYPE_MSYS_E;
    }
    else {
    	len = fread(buf, 1, sizeof(buf), fd);
    	buf[MIN(len, sizeof(buf)-1)] = '\0';
        if (strstr(buf, cpu_msys) != NULL) {
            pdlGpioCpuType = PDL_CPU_FAMILY_TYPE_MSYS_E;
        }
        else if (strstr(buf, cpu_axp) != NULL) {
            pdlGpioCpuType = PDL_CPU_FAMILY_TYPE_AXP_E;
        }
        else if (strstr(buf, cpu_a38x) != NULL) {
            pdlGpioCpuType = PDL_CPU_FAMILY_TYPE_A38X_E;
        }
        else {
            pdlGpioCpuType = PDL_CPU_FAMILY_TYPE_MSYS_E;
        }
        fclose(fd);
    }
#else
    pdlGpioCpuType = PDL_CPU_FAMILY_TYPE_MSYS_E;
#endif
    return PDL_OK;
}

/*$ END OF prvPdlGpioCpuTypeInit */

/* ***************************************************************************
* FUNCTION NAME: prvPdlGpioMppInit
*
* DESCRIPTION:   Init MPPs
*                SHOULD be called after data on all GPIO interfaces has been collected to DB
*                Writes 0 in case MPP is used to the MPP init register
*                and write I/O to the gpio input/output register
*
* PARAMETERS:   
*                
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlGpioMppInit ( IN void )
 *
 * @brief   Prv pdl gpio mpp initialize
 *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlGpioMppInit (
    IN  void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/  
#ifdef LINUX_HW
    UINT_32                                     data, j;
#endif
    UINT_32                                     maxDev = 0, pinNumber, relPinNumber, i, gpioWord, cpuInoutWord[PDL_GPIO_NUM_OF_TYPE_REGS] = {0}, ppInoutWord[PDL_MAX_PP_NUM][PDL_GPIO_NUM_OF_TYPE_REGS] = PDL_PP_TYPE_REGS_ZERO_ARRAY;
    UINT_32                                     mppWord, cpuMppMask[PDL_GPIO_MPP_NUM_OF_INIT_REGS] = {0}, ppMppMask[PDL_MAX_PP_NUM][PDL_GPIO_MPP_NUM_OF_INIT_REGS] = PDL_PP_INIT_REGS_ZERO_ARRAY; /*mpp word has to be set to 0xF for each GPIO that is being used  */
    UINT_32                                     cpuOutputWord[PDL_GPIO_NUM_OF_TYPE_REGS] = {0} , ppOutputWord[PDL_MAX_PP_NUM][PDL_GPIO_MPP_NUM_OF_INIT_REGS] = PDL_PP_INIT_REGS_ZERO_ARRAY;
    UINT_32                                     cpuOutputMask[PDL_GPIO_NUM_OF_TYPE_REGS] = {0}, ppOutputMask[PDL_MAX_PP_NUM][PDL_GPIO_MPP_NUM_OF_INIT_REGS] = PDL_PP_INIT_REGS_ZERO_ARRAY;
    UINT_32                                     cpuInitMask[PDL_GPIO_NUM_OF_TYPE_REGS] = {0}, ppInitMask[PDL_MAX_PP_NUM][PDL_GPIO_MPP_NUM_OF_INIT_REGS] = PDL_PP_INIT_REGS_ZERO_ARRAY;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_GPIO_DB_STC             * mppPtr;
    PDL_INTERFACE_PRV_KEY_STC                   mppKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    for (i = 0 ; i < pdlGpioMppCount; i++) {
        mppKey.interfaceId = i;
        pdlStatus  = prvPdlDbFind (&pdlGpioMppDb, (void*) &mppKey, (void**) &mppPtr);
        if (pdlStatus != PDL_OK) {
            PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__,"%s can't find mppId=[%d] !!! MPP AREN'T INITIALIZED !!!\n", __FUNCTION__, i);
            return pdlStatus;
        }
        pinNumber = mppPtr->publicInfo.pinNumber;
        relPinNumber = pinNumber % 32;
        mppWord = pinNumber / 8;
        gpioWord = pinNumber / 32;
        if (mppWord >= PDL_GPIO_MPP_NUM_OF_INIT_REGS || gpioWord >= PDL_GPIO_NUM_OF_TYPE_REGS ||
            (mppPtr->publicInfo.dev != PDL_GPIO_CPU_DEV && mppPtr->publicInfo.dev >= PDL_MAX_PP_NUM)) {
            PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__,"%s bad size mppWord=[%d] gpioWord=[%d] dev=[%d] !!! MPP AREN'T INITIALIZED !!!\n", __FUNCTION__, mppWord, gpioWord, mppPtr->publicInfo.dev);
            return PDL_BAD_SIZE;
        }
        if (mppPtr->publicInfo.dev == PDL_GPIO_CPU_DEV) {
            cpuMppMask[mppWord] |= PDL_GPIO_MPP_GET_INIT_MASK_VALUE (pinNumber);
            /* input gpios bit should be set to 1 */
            if (mppPtr->publicInfo.type == PDL_INTERFACE_GPIO_TYPE_READ_E) {
                cpuInoutWord[gpioWord] |= 1 << relPinNumber;
            }
            else {
                if (mppPtr->publicInfo.initialValue != PDL_INTERFACE_MPP_INIT_VALUE_HW_E) {
                    cpuOutputWord[gpioWord] |= mppPtr->publicInfo.initialValue << relPinNumber;
                    cpuInitMask[gpioWord] |= 1 << relPinNumber;
                }
                cpuOutputMask[gpioWord] |= 1 << relPinNumber;
            }
        }
        else {
            if (mppPtr->publicInfo.dev > maxDev) {
                maxDev = mppPtr->publicInfo.dev;
            }
            ppMppMask[mppPtr->publicInfo.dev][mppWord] |= PDL_GPIO_MPP_GET_INIT_MASK_VALUE (pinNumber);
            /* input gpios bit should be set to 1 */
            if (mppPtr->publicInfo.type == PDL_INTERFACE_GPIO_TYPE_READ_E) {
                ppInoutWord[mppPtr->publicInfo.dev][gpioWord] |= 1 << relPinNumber;
            }
            /* set default value for output GPIOs as the not pushed value */
            else {
                if (mppPtr->publicInfo.initialValue != PDL_INTERFACE_MPP_INIT_VALUE_HW_E) {                
                    ppOutputWord[mppPtr->publicInfo.dev][gpioWord] |= mppPtr->publicInfo.initialValue << relPinNumber;
                    ppInitMask[mppPtr->publicInfo.dev][gpioWord] |= 1 << relPinNumber;
                }
                ppOutputMask[mppPtr->publicInfo.dev][gpioWord] |= 1 << relPinNumber;
            }
        }
    }

#ifdef LINUX_HW
    /* write MPP configuration */
    /* CPU MPP */
    if (pdlGpioMemCpuRegisters == 0) {
        return PDL_NOT_IMPLEMENTED;
    }
    /* configure MPP registers (set 4 bits to 0 incase GPIO is used) */
    for (i = 0; i < PDL_GPIO_MPP_NUM_OF_INIT_REGS; i++) {
        /* each 4 bits represent MPP. write 0 with mask F to set as MPP */
        data = 0;
        data |= *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_MPP_INIT_BASE_REG + i * 4)) & ~cpuMppMask[i];
        *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_MPP_INIT_BASE_REG + i * 4)) = data;
    }
    /* write default output value */
    data = cpuOutputWord[0];
    data |= *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_OUT_MPP_0_31)) & ~cpuInitMask[0];
    *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_OUT_MPP_0_31)) = data;
    data = cpuOutputWord[1];
    data |= *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_OUT_MPP_32_63)) & ~cpuInitMask[1];
    *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_OUT_MPP_32_63)) = data;
    /* set i/o type */
	data = *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_SET_TYPE_MPP_0_31));
	/* First, raise Input pins */
	data |= cpuInoutWord[0];
	/* Then, clear Output pins */
	data &= ~(cpuOutputMask[0]);
	/* Finally, update */
	*((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_SET_TYPE_MPP_0_31)) = data;

	data = *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_SET_TYPE_MPP_32_63));
	/* First, raise Input pins */
	data |= cpuInoutWord[1];
	/* Then, clear Output pins */
	data &= ~(cpuOutputMask[1]);
	/* Finally, update */
	*((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_SET_TYPE_MPP_32_63)) = data;

    /* PP MPP */
    for (i = 0; i <= maxDev; i++) {
        /* configure MPP registers (set 4 bits to 0 incase GPIO is used) */
        for (j = 0; j < PDL_GPIO_MPP_NUM_OF_INIT_REGS; j++) {
            /* each 4 bits represent MPP. write 0 with mask F to set as MPP */
            pdlStatus = prvPdlCmRegWrite ((GT_U8)i, PDL_GPIO_MPP_INIT_BASE_REG + j * 4, ppMppMask[i][j], 0);
            PDL_CHECK_STATUS(pdlStatus);
        }
        /* write default output value */
        pdlStatus = prvPdlCmRegWrite ((GT_U8)i, PDL_GPIO_OUT_MPP_0_31, ppOutputMask[i][0], ppInitMask[i][0]);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlCmRegWrite ((GT_U8)i, PDL_GPIO_OUT_MPP_32_63, ppOutputMask[i][1], ppInitMask[i][1]);
        PDL_CHECK_STATUS(pdlStatus);
        
        /* set i/o type */
		pdlStatus = prvPdlCmRegRead ((GT_U8)i, PDL_GPIO_SET_TYPE_MPP_0_31, PDL_MASK_32_BIT, &data);
		PDL_CHECK_STATUS(pdlStatus);
		/* First, raise Input pins */
		data |= ppInoutWord[i][0];
		/* Then, clear Output pins */
		data &= ~(ppOutputMask[i][0]);
		/* Finally, update */
		pdlStatus = prvPdlCmRegWrite ((GT_U8)i, PDL_GPIO_SET_TYPE_MPP_0_31, PDL_MASK_32_BIT, data);
		PDL_CHECK_STATUS(pdlStatus);

		pdlStatus = prvPdlCmRegRead ((GT_U8)i, PDL_GPIO_SET_TYPE_MPP_32_63, PDL_MASK_32_BIT, &data);
		PDL_CHECK_STATUS(pdlStatus);
		/* First, raise Input pins */
		data |= ppInoutWord[i][1];
		/* Then, clear Output pins */
		data &= ~(ppOutputMask[i][1]);
		/* Finally, update */
		pdlStatus = prvPdlCmRegWrite ((GT_U8)i, PDL_GPIO_SET_TYPE_MPP_32_63, PDL_MASK_32_BIT, data);
		PDL_CHECK_STATUS(pdlStatus);

    }

#endif
    return PDL_OK;
}

/*$ END OF prvPdlGpioMppInit */

/**
 * @fn  PDL_STATUS prvPdlGpioCountGet ( OUT UINT_32 * countPtr )
 *
 * @brief   Gets number of GPIO interfaces
 *
 * @param[out] countPtr Number of gpio interfaces
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS prvPdlGpioCountGet (
    OUT UINT_32 * countPtr
)
{
/****************************************************************************/
/*L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/****************************************************************************/

/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    if (countPtr == NULL)
        return PDL_BAD_PTR;

    *countPtr = prvPdlGpioInterfaceCount;

    return PDL_OK;
}

/* ***************************************************************************
* FUNCTION NAME: PdlGpioInit
*
* DESCRIPTION:   Init GPIO module
*                Create GPIO DB and initialize
*
* PARAMETERS:   
*                
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlGpioInit ( IN void )
 *
 * @brief   Pdl gpio initialize
 *
 * @param   initType    type of init performed (full will also map CPU memory space)
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlGpioInit (
    IN PDL_OS_INIT_TYPE_ENT         initType
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
    dbAttributes.listAttributes.keySize = sizeof(PDL_INTERFACE_PRV_KEY_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_INTERFACE_PRV_GPIO_DB_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &pdlGpioDb);
    PDL_CHECK_STATUS(pdlStatus);

    dbAttributes.listAttributes.keySize = sizeof(PDL_INTERFACE_PRV_KEY_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_INTERFACE_PRV_GPIO_DB_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &pdlGpioMppDb);
    PDL_CHECK_STATUS(pdlStatus);

    if (initType == PDL_OS_INIT_TYPE_FULL_E) {
        prvPdlGpioCpuTypeInit();

#ifdef LINUX_HW
        /* memory map CPU GPIO */
        memfd = open(PDL_MEM_DEV_MEM_NAME, O_RDWR);
        if (memfd < 0) {
            return PDL_MEM_INIT_ERROR;
        }
        pdlGpioMemCpuRegisters =  (UINTPTR) mmap((void*) NULL                /* Addr */,
                                            PDL_CPU_REGISTERS_REGION_SIZE    /* length */,
                                            PROT_READ | PROT_WRITE           /* protection */,
                                            MAP_SHARED                       /* flags */,
                                            memfd                            /* file descriptor */,
                                            PDL_CPU_REGISTERS_REGION         /* offset */);
#endif
    }

    return PDL_OK;
}

/*$ END OF prvPdlGpioInit */

/**
 * @fn  PDL_STATUS prvPdlGpioDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlGpioDestroy (
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
    prvPdlGpioInterfaceCount = 0;
    pdlGpioMppCount = 0;

    if (pdlGpioDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlGpioDb);
        PDL_CHECK_STATUS(pdlStatus);
    }
    if (pdlGpioMppDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlGpioMppDb);
        PDL_CHECK_STATUS(pdlStatus);
    }

    return PDL_OK;
}

/*$ END OF prvPdlGpioDestroy */
