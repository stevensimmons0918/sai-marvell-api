/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlPacketProcessor.c   
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
 * @brief Platform driver layer - Packet Processor related API
 * 
 * @version   1 
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <pdl/common/pdlTypes.h>
#include <pdl/phy/private/prvPdlPhy.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdl/packet_processor/private/prvPdlPacketProcessor.h>
#include <pdl/sfp/private/prvPdlSfp.h>
#include <pdl/serdes/private/prvPdlSerdes.h>
#include <pdl/interface/private/prvPdlSmiXsmi.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <pdl/interface/pdlSmiXsmiDebug.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/** @brief   The pdl pp database */
static PDL_PP_XML_ATTRIBUTES_STC        pdlPpDb;
/** @brief   The pdl pp b 2b db[ prv pdl pp b 2 b maximum num] */
static PDL_PP_XML_B2B_ATTRIBUTES_STC    pdlPpB2bDb[PRV_PDL_PP_B2B_MAX_NUM];
/** @brief   The prv pdl front panel database */
static PDL_DB_PRV_STC                   prvPdlFrontPanelDb;
/** @brief   The prv pdl pp port database */
static PDL_DB_PRV_STC                   prvPdlPpPortDb;
/** @brief   The prv pdl mac port database */
static PDL_DB_PRV_STC                   prvPdlMacLogicalPortDb;
/** @brief   Is logical port different from mac port */
static BOOLEAN                          isLogicalPortRequired;

/**************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)                                                                 \
    if (__pdlStatus != PDL_OK) {                                                                      \
    if (prvPdlPpDebugFlag) {                                                                          \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__pdlStatus,PDL_STATUS_ERROR_PRINT_IDENTIFIER);     \
    }                                                                                                 \
    return __pdlStatus;                                                                               \
    }  
/*macro to check the xml status and debug prints if debug flag on*/
#undef XML_CHECK_STATUS
#define XML_CHECK_STATUS(__xmlStatus)                                                                 \
    if (__xmlStatus != PDL_OK) {                                                                      \
    if (prvPdlPpDebugFlag) {                                                                          \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__xmlStatus,XML_STATUS_ERROR_PRINT_IDENTIFIER);     \
    }                                                                                                 \
    return __xmlStatus;                                                                               \
    }  

/*****************************************************************************
* FUNCTION NAME: prvPdlPpPhyRegisterListParse
*
* DESCRIPTION:  parse a list of phy registers
*
*
*****************************************************************************/

/**
 * @fn  static PDL_STATUS prvPdlPpPhyRegisterListParse ( IN PDL_DB_PRV_STC phyRegistersDbPtr, IN char * phyRegistersListStr, INOUT XML_PARSER_NODE_DESCRIPTOR_TYP * xmlPhyConnectionIdPtr )
 *
 * @brief   Prv pdl pp parse phy registers list
 *
 * @param   phyRegistersDbPtr    the DB holding the register list.
 * @param   phyRegistersListStr  Name of the Tag of the register list.
 * @param   xmlPhyConnectionIdPtr   Identifier for the XML Phy register list
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlPpPhyRegisterListParse (
    /*!     INPUTS:             */
    IN PDL_DB_PRV_STC                    * phyRegistersDbPtr,
    IN char                              * phyRegistersListStr,
    /*!     INPUTS / OUTPUTS:   */
    INOUT XML_PARSER_NODE_DESCRIPTOR_TYP * xmlPhyConnectionIdPtr
    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_32                                     maxSize;
    XML_PARSER_RET_CODE_TYP                     xmlStatus = XML_PARSER_RET_CODE_OK;
    PDL_STATUS                                  pdlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP              xmlPhyRegisterId, xmlPhyRegisterValue;
    PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC   phyInitEntry, *phyInitEntryPtr;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/

    while (xmlStatus == XML_PARSER_RET_CODE_OK && xmlParserIsEqualName (*xmlPhyConnectionIdPtr, phyRegistersListStr) == XML_PARSER_RET_CODE_OK) {
        xmlStatus = xmlParserGetFirstChild(*xmlPhyConnectionIdPtr, &xmlPhyRegisterId);
        while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
            XML_CHECK_STATUS(xmlStatus);
            if (xmlParserIsEqualName (xmlPhyRegisterId, "phy-init-index") == XML_PARSER_RET_CODE_OK) {
                maxSize = sizeof(phyInitEntry.key);
                xmlStatus = xmlParserGetValue (xmlPhyRegisterId, &maxSize, &phyInitEntry.key.index);
                XML_CHECK_STATUS(xmlStatus);
            }
            else if (xmlParserIsEqualName (xmlPhyRegisterId, "phy-mask-value") == XML_PARSER_RET_CODE_OK) {
                xmlStatus = xmlParserGetFirstChild(xmlPhyRegisterId, &xmlPhyRegisterValue);
                while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
                    XML_CHECK_STATUS(xmlStatus);
                    if (xmlParserIsEqualName (xmlPhyRegisterValue, "phy-device-or-page") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(phyInitEntry.info.devOrPage);
                        xmlStatus = xmlParserGetValue (xmlPhyRegisterValue, &maxSize, &phyInitEntry.info.devOrPage);
                        XML_CHECK_STATUS(xmlStatus);
                    }
                    else if (xmlParserIsEqualName (xmlPhyRegisterValue, "phy-register") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(phyInitEntry.info.registerAddress);
                        xmlStatus = xmlParserGetValue (xmlPhyRegisterValue, &maxSize, &phyInitEntry.info.registerAddress);
                        XML_CHECK_STATUS(xmlStatus);
                    }
                    else if (xmlParserIsEqualName (xmlPhyRegisterValue, "phy-mask") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(phyInitEntry.info.mask);
                        xmlStatus = xmlParserGetValue (xmlPhyRegisterValue, &maxSize, &phyInitEntry.info.mask);
                        XML_CHECK_STATUS(xmlStatus);
                    }
                    else if (xmlParserIsEqualName (xmlPhyRegisterValue, "phy-value") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(phyInitEntry.info.value);
                        xmlStatus = xmlParserGetValue (xmlPhyRegisterValue, &maxSize, &phyInitEntry.info.value);
                        XML_CHECK_STATUS(xmlStatus);
                    }
                    else {
                        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"Incorrect tag under phy-mask-value");
                        return PDL_XML_PARSE_ERROR;
                    }
                    xmlStatus = xmlParserGetNextSibling(xmlPhyRegisterValue, &xmlPhyRegisterValue);
                }
                pdlStatus = prvPdlDbAdd (phyRegistersDbPtr, (void*) &phyInitEntry.key, (void*) &phyInitEntry, (void**) &phyInitEntryPtr);
                PDL_CHECK_STATUS(pdlStatus);
            }
            else {
                PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"Incorrect tag under phy-init-list");
                return PDL_XML_PARSE_ERROR;
            }
            xmlStatus = xmlParserGetNextSibling(xmlPhyRegisterId, &xmlPhyRegisterId);
        }

        xmlStatus = xmlParserGetNextSibling(*xmlPhyConnectionIdPtr, xmlPhyConnectionIdPtr);
        /*XML_CHECK_STATUS(xmlStatus);*/
    }

    return PDL_OK;
}

/*****************************************************************************
* FUNCTION NAME: prvPdlPpPortInterfaceModesGet
*
* DESCRIPTION: 
*
*
*****************************************************************************/

/**
 * @fn  static PDL_STATUS prvPdlPpPortInterfaceModesGet ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlInterfaceModesGroupId, OUT PDL_L1_INTERFACE_MODE_STC interfaceModeGroupArr[], OUT UINT_32 * numOfModesPtr )
 *
 * @brief   Prv pdl pp port interface modes get
 *
 * @param           xmlInterfaceModesGroupId    Identifier for the XML interface modes group.
 * @param           interfaceModeGroupArr       Array of interface mode groups.
 * @param [in,out]  numOfModesPtr               If non-null, number of modes pointers.
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlPpPortInterfaceModesGet (

    /*!     INPUTS:             */
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlInterfaceModesGroupId,
    OUT PDL_L1_INTERFACE_MODE_STC           interfaceModeGroupArr[],
    OUT UINT_32                           * numOfModesPtr,
    OUT PDL_DB_PRV_STC                      phyInitDbPtr[PDL_PORT_SPEED_LAST_E]
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    XML_PARSER_RET_CODE_TYP             xmlStatus, xmlStatus2;
    PDL_STATUS                          pdlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP      xmlInterfaceModeId, xmlInterfaceModeAttrId;
    char                                pdlTagStr[PDL_XML_MAX_TAG_LEN];
    UINT_32                             maxSize, count = 0, value;
    PDL_DB_PRV_ATTRIBUTES_STC           dbAttributes;
    BOOLEAN                             phyInitDbCreated;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (!(xmlParserIsEqualName(xmlInterfaceModesGroupId, "L1-interface-modes-group") || xmlParserIsEqualName(xmlInterfaceModesGroupId, "copper-L1-interface-modes-group"))) {
        return PDL_BAD_VALUE;
    }

    xmlStatus = xmlParserGetFirstChild(xmlInterfaceModesGroupId, &xmlInterfaceModeId);
    XML_CHECK_STATUS(xmlStatus);
    

    xmlStatus = xmlParserIsEqualName(xmlInterfaceModeId, "L1-interface-mode-list");
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND)
    {
        XML_CHECK_STATUS(xmlStatus);
        xmlStatus2 = xmlParserGetFirstChild(xmlInterfaceModeId, &xmlInterfaceModeAttrId);
        phyInitDbCreated = FALSE;
        while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
            XML_CHECK_STATUS(xmlStatus2);
            if (xmlParserIsEqualName (xmlInterfaceModeAttrId, "speed") == XML_PARSER_RET_CODE_OK) {
                maxSize = sizeof(pdlTagStr);
                xmlStatus = xmlParserGetValue (xmlInterfaceModeAttrId, &maxSize, &pdlTagStr[0]);
                XML_CHECK_STATUS(xmlStatus);
                pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_SPEED_TYPE_E, pdlTagStr, &value);
                PDL_CHECK_STATUS(pdlStatus);
                interfaceModeGroupArr[count].speed = (PDL_PORT_SPEED_ENT)value;
            }
            else if (xmlParserIsEqualName (xmlInterfaceModeAttrId, "mode") == XML_PARSER_RET_CODE_OK) {
                maxSize = sizeof(pdlTagStr);
                xmlStatus = xmlParserGetValue (xmlInterfaceModeAttrId, &maxSize, &pdlTagStr[0]);
                XML_CHECK_STATUS(xmlStatus);
                pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, pdlTagStr, &value);
                PDL_CHECK_STATUS(pdlStatus); 
                interfaceModeGroupArr[count].mode = (PDL_INTERFACE_MODE_ENT)value;
            }
            else if (xmlParserIsEqualName (xmlInterfaceModeAttrId, "phy-init-list") == XML_PARSER_RET_CODE_OK) {
                if (phyInitDbCreated == FALSE) {
                    dbAttributes.listAttributes.entrySize = sizeof(PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC);
                    dbAttributes.listAttributes.keySize = sizeof(PDL_PP_XML_NETWORK_PORT_PHY_INIT_KEY_STC);
                    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                                &dbAttributes,
                                                &phyInitDbPtr[interfaceModeGroupArr[count].speed]);
                    PDL_CHECK_STATUS(pdlStatus);
                    phyInitDbCreated = TRUE;
                }
                pdlStatus = prvPdlPpPhyRegisterListParse(&phyInitDbPtr[interfaceModeGroupArr[count].speed], "phy-init-list", &xmlInterfaceModeAttrId);
                PDL_CHECK_STATUS(pdlStatus);
            }
            else {
                PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"Incorrect tag under L1-interface-mode-list");
                return PDL_XML_PARSE_ERROR;
            }
            xmlStatus2 = xmlParserGetNextSibling(xmlInterfaceModeAttrId, &xmlInterfaceModeAttrId);
        }
        count ++;

        xmlStatus = xmlParserGetNextSibling(xmlInterfaceModeId, &xmlInterfaceModeId);
    }

    *numOfModesPtr = count;

    return (count) ? PDL_OK : PDL_XML_PARSE_ERROR;
}
/*$ END OF prvPdlPpPortInterfaceModes */

/*****************************************************************************
* FUNCTION NAME: prvPdlPpPortGbicGroupParse
*
* DESCRIPTION:  Call gbic parsing api
*
*
*****************************************************************************/

/**
 * @fn  static PDL_STATUS prvPdlPpPortGbicGroupParse ( IN UINT_32 dev, IN UINT_32 logicalPort, IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlGbicGroupId )
 *
 * @brief   Prv pdl pp port gbic group parse
 *
 * @param   dev             The development.
 * @param   logicalPort     The port.
 * @param   xmlGbicGroupId  Identifier for the XML gbic group.
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlPpPortGbicGroupParse (

    /*!     INPUTS:             */
    IN UINT_32                              dev,
    IN UINT_32                              logicalPort,
    IN XML_PARSER_NODE_DESCRIPTOR_TYP       xmlGbicGroupId
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    
    return prvPdlSfpXmlParser(xmlGbicGroupId, dev, logicalPort);
}
/*$ END OF prvPdlPpPortGbicGroupParse */

/*****************************************************************************
* FUNCTION NAME: prvPdlFrontPanelSinglePortGet
*
* DESCRIPTION: 
*
*
*****************************************************************************/

/**
 * @fn  static PDL_STATUS prvPdlFrontPanelSinglePortGet ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlPortId, OUT PRV_PDL_PORT_ENTRY_STC * portEntryPtr )
 *
 * @brief   Prv pdl front panel single port get
 *
 * @param           xmlPortId       Identifier for the XML port.
 * @param [in,out]  portEntryPtr    If non-null, the port entry pointer.
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlFrontPanelSinglePortGet (

    /*!     INPUTS:             */
    IN XML_PARSER_NODE_DESCRIPTOR_TYP      xmlPortId,
    OUT PRV_PDL_PORT_ENTRY_STC           * portEntryPtr
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    char                                    pdlTagStr[PDL_XML_MAX_TAG_LEN];
    UINT_32                                 maxSize, value;
    XML_PARSER_RET_CODE_TYP                 xmlStatus;
    PDL_STATUS                              pdlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP          xmlPortAttrId, xmlPortSerdesId, xmlPhyConnectionId;
    PDL_INTERFACE_PRV_SMI_XSMI_DB_STC       smiInfo;
    PDL_DB_PRV_ATTRIBUTES_STC               dbAttributes;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    xmlStatus = xmlParserIsEqualName(xmlPortId, "port-list");
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserGetFirstChild(xmlPortId, &xmlPortAttrId);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserIsEqualName(xmlPortAttrId, "port-number");
    XML_CHECK_STATUS(xmlStatus);

    maxSize = sizeof(portEntryPtr->data.portNumberInGroup);
    xmlStatus = xmlParserGetValue(xmlPortAttrId, &maxSize, &portEntryPtr->data.portNumberInGroup);
    XML_CHECK_STATUS(xmlStatus);


    xmlStatus = xmlParserGetNextSibling(xmlPortAttrId, &xmlPortAttrId);
    XML_CHECK_STATUS(xmlStatus);


    xmlStatus = xmlParserIsEqualName(xmlPortAttrId, "pp-device-number");
    XML_CHECK_STATUS(xmlStatus);

    maxSize = sizeof(portEntryPtr->key.dev);
    xmlStatus = xmlParserGetValue(xmlPortAttrId, &maxSize, &portEntryPtr->key.dev);
    XML_CHECK_STATUS(xmlStatus);

    if (portEntryPtr->key.dev >= pdlPpDb.numOfPps)
    {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"PORT ENTRY KEY.DEVICE >= NUMBER OF Pp IN DB");
        return PDL_XML_PARSE_ERROR;
    }

    xmlStatus = xmlParserGetNextSibling(xmlPortAttrId, &xmlPortAttrId);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserIsEqualName(xmlPortAttrId, "pp-port-number");
    XML_CHECK_STATUS(xmlStatus);

    maxSize = sizeof(portEntryPtr->key.logicalPort);
    xmlStatus = xmlParserGetValue(xmlPortAttrId, &maxSize, &portEntryPtr->key.logicalPort);
    XML_CHECK_STATUS(xmlStatus);
    
    portEntryPtr->data.macPort = portEntryPtr->key.logicalPort;
 
    if (portEntryPtr->key.logicalPort > 127)
    {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"ENTRY PORT > 127");
        return PDL_XML_PARSE_ERROR;
    }

    xmlStatus = xmlParserGetNextSibling(xmlPortAttrId, &xmlPortAttrId);
    XML_CHECK_STATUS(xmlStatus);

    if (xmlParserIsEqualName (xmlPortAttrId, "logical-port-number") == XML_PARSER_RET_CODE_OK) {
        isLogicalPortRequired = TRUE;
        maxSize = sizeof(portEntryPtr->key.logicalPort);
        xmlStatus = xmlParserGetValue(xmlPortAttrId, &maxSize, &portEntryPtr->key.logicalPort);
        XML_CHECK_STATUS(xmlStatus);
        if (portEntryPtr->key.logicalPort > 127)
        {
            PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"ENTRY PORT > 127");
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling(xmlPortAttrId, &xmlPortAttrId);
        XML_CHECK_STATUS(xmlStatus);
    }
     
    portEntryPtr->data.isPhyExists = FALSE;
    /* skip is-phy-used tag */
    if ((xmlParserIsEqualName(xmlPortAttrId, "is-phy-used") == XML_PARSER_RET_CODE_OK)) {
        xmlStatus = xmlParserGetNextSibling(xmlPortAttrId, &xmlPortAttrId);
        XML_CHECK_STATUS(xmlStatus);
    }
    xmlStatus = xmlParserIsEqualName(xmlPortAttrId, "port-phy");
    if (xmlStatus == XML_PARSER_RET_CODE_OK) {
        xmlStatus = xmlParserGetFirstChild(xmlPortAttrId, &xmlPhyConnectionId);
        XML_CHECK_STATUS(xmlStatus);
        dbAttributes.listAttributes.entrySize = sizeof(PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC);
        dbAttributes.listAttributes.keySize = sizeof(PDL_PP_XML_NETWORK_PORT_PHY_INIT_KEY_STC);
        pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                    &dbAttributes,
                                    &portEntryPtr->phyPostInitValues);
        PDL_CHECK_STATUS(pdlStatus);

        pdlStatus = prvPdlPpPhyRegisterListParse(&portEntryPtr->phyPostInitValues, "phy-post-init-list", &xmlPhyConnectionId);
        PDL_CHECK_STATUS(pdlStatus);
#if 0
        while (xmlParserIsEqualName (xmlPhyConnectionId, "phy-post-init-list") == XML_PARSER_RET_CODE_OK) {

            dbAttributes.listAttributes.entrySize = sizeof(PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC);
            dbAttributes.listAttributes.keySize = sizeof(PDL_PP_XML_NETWORK_PORT_PHY_INIT_KEY_STC);
            pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                        &dbAttributes,
                                        &portEntryPtr->phyPostInitValues);
            PDL_CHECK_STATUS(pdlStatus);
            xmlStatus = xmlParserGetFirstChild(xmlPhyConnectionId, &xmlPhyRegisterId);
            while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
                XML_CHECK_STATUS(xmlStatus);
                if (xmlParserIsEqualName (xmlPhyRegisterId, "phy-init-index") == XML_PARSER_RET_CODE_OK) {
                    maxSize = sizeof(phyInitEntry.key);
                    xmlStatus = xmlParserGetValue (xmlPhyRegisterId, &maxSize, &phyInitEntry.key.index);
                    XML_CHECK_STATUS(xmlStatus);
                }
                else if (xmlParserIsEqualName (xmlPhyRegisterId, "phy-mask-value") == XML_PARSER_RET_CODE_OK) {
                    xmlStatus = xmlParserGetFirstChild(xmlPhyRegisterId, &xmlPhyRegisterValue);
                    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
                        XML_CHECK_STATUS(xmlStatus);
                        if (xmlParserIsEqualName (xmlPhyRegisterValue, "phy-device-or-page") == XML_PARSER_RET_CODE_OK) {
                            maxSize = sizeof(phyInitEntry.info.devOrPage);
                            xmlStatus = xmlParserGetValue (xmlPhyRegisterValue, &maxSize, &phyInitEntry.info.devOrPage);
                            XML_CHECK_STATUS(xmlStatus);
                        }
                        else if (xmlParserIsEqualName (xmlPhyRegisterValue, "phy-register") == XML_PARSER_RET_CODE_OK) {
                            maxSize = sizeof(phyInitEntry.info.registerAddress);
                            xmlStatus = xmlParserGetValue (xmlPhyRegisterValue, &maxSize, &phyInitEntry.info.registerAddress);
                            XML_CHECK_STATUS(xmlStatus);
                        }
                        else if (xmlParserIsEqualName (xmlPhyRegisterValue, "phy-mask") == XML_PARSER_RET_CODE_OK) {
                            maxSize = sizeof(phyInitEntry.info.mask);
                            xmlStatus = xmlParserGetValue (xmlPhyRegisterValue, &maxSize, &phyInitEntry.info.mask);
                            XML_CHECK_STATUS(xmlStatus);
                        }
                        else if (xmlParserIsEqualName (xmlPhyRegisterValue, "phy-value") == XML_PARSER_RET_CODE_OK) {
                            maxSize = sizeof(phyInitEntry.info.value);
                            xmlStatus = xmlParserGetValue (xmlPhyRegisterValue, &maxSize, &phyInitEntry.info.value);
                            XML_CHECK_STATUS(xmlStatus);
                        }
                        else {
                            PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"Incorrect tag under phy-mask-value");
                            return PDL_XML_PARSE_ERROR;
                        }
                        xmlStatus = xmlParserGetNextSibling(xmlPhyRegisterValue, &xmlPhyRegisterValue);
                    }
                    pdlStatus = prvPdlDbAdd (&portEntryPtr->phyPostInitValues, (void*) &phyInitEntry.key, (void*) &phyInitEntry, (void**) &phyInitEntryPtr);
                    PDL_CHECK_STATUS(pdlStatus);
                }
                else {
                    PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"Incorrect tag under phy-init-list");
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus = xmlParserGetNextSibling(xmlPhyRegisterId, &xmlPhyRegisterId);
            }



            xmlStatus = xmlParserGetNextSibling(xmlPhyConnectionId, &xmlPhyConnectionId);
            XML_CHECK_STATUS(xmlStatus);
        }
#endif
        if (xmlParserIsEqualName(xmlPhyConnectionId, "phy-read-write-address") == XML_PARSER_RET_CODE_OK) {
            portEntryPtr->data.isPhyExists = TRUE;
            pdlStatus = prvPdlPhyPortDataGet (xmlPhyConnectionId, &portEntryPtr->data.phyData);
            PDL_CHECK_STATUS(pdlStatus); 
            memcpy (&smiInfo.publicInfo, &portEntryPtr->data.phyData.smiXmsiInterface, sizeof(PDL_INTERFACE_SMI_XSMI_STC));
            pdlStatus = prvPdlSmiXsmiAddInterface(&smiInfo, &portEntryPtr->phyInterfaceId);
            PDL_CHECK_STATUS(pdlStatus);
            portEntryPtr->phyInterfaceType = portEntryPtr->data.phyData.smiXmsiInterface.interfaceType;
        }
        else {
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling(xmlPortAttrId, &xmlPortAttrId);
        XML_CHECK_STATUS(xmlStatus);
    }

    if (xmlParserIsEqualName(xmlPortAttrId, "swap-abcd") != XML_PARSER_RET_CODE_OK ) {
        portEntryPtr->data.swapAbcd = FALSE;
    }
    else {
        maxSize = sizeof(portEntryPtr->data.swapAbcd);
        xmlStatus = xmlParserGetValue(xmlPortAttrId, &maxSize, &portEntryPtr->data.swapAbcd);
        XML_CHECK_STATUS(xmlStatus);
        xmlStatus = xmlParserGetNextSibling(xmlPortAttrId, &xmlPortAttrId);
        XML_CHECK_STATUS(xmlStatus);
    }
    
    xmlStatus = xmlParserIsEqualName(xmlPortAttrId, "transceiver");
    XML_CHECK_STATUS(xmlStatus);

    maxSize = sizeof(pdlTagStr);
    xmlStatus = xmlParserGetValue(xmlPortAttrId, &maxSize, &pdlTagStr[0]);
    XML_CHECK_STATUS(xmlStatus);

    if (PDL_OK != pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_TRANSCEIVER_TYPE_E, pdlTagStr, &value))
    {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL pdlTagStr(DIFFERENT FROM: fiber, copper, combo)");
        return PDL_XML_PARSE_ERROR;
    }
    portEntryPtr->data.transceiverType = (PDL_TRANSCEIVER_TYPE_ENT)value;

    xmlStatus = xmlParserGetNextSibling(xmlPortAttrId, &xmlPortAttrId);
    XML_CHECK_STATUS(xmlStatus);

    if (portEntryPtr->data.transceiverType == PDL_TRANSCEIVER_TYPE_COMBO_E ||
        portEntryPtr->data.transceiverType == PDL_TRANSCEIVER_TYPE_FIBER_E)
    {
        pdlStatus = prvPdlPpPortInterfaceModesGet(xmlPortAttrId, portEntryPtr->data.fiberModesArr, &portEntryPtr->data.numOfFiberModes, portEntryPtr->fiberPhyInitValueDb);
    }
    else
    {
        pdlStatus = prvPdlPpPortInterfaceModesGet(xmlPortAttrId, portEntryPtr->data.copperModesArr, &portEntryPtr->data.numOfCopperModes, portEntryPtr->copperPhyInitValueDb);
    }
    PDL_CHECK_STATUS(pdlStatus);

    if (portEntryPtr->data.transceiverType == PDL_TRANSCEIVER_TYPE_COMBO_E)
    {
        XML_CHECK_STATUS(xmlStatus);

        xmlStatus = xmlParserGetNextSibling(xmlPortAttrId, &xmlPortAttrId);
        XML_CHECK_STATUS(xmlStatus);

        xmlStatus = xmlParserIsEqualName(xmlPortAttrId, "copper-L1-interface-modes-group");
        XML_CHECK_STATUS(xmlStatus);

        pdlStatus = prvPdlPpPortInterfaceModesGet(xmlPortAttrId, portEntryPtr->data.copperModesArr, &portEntryPtr->data.numOfCopperModes, portEntryPtr->copperPhyInitValueDb);
        PDL_CHECK_STATUS(pdlStatus);
    }

    if (portEntryPtr->data.transceiverType == PDL_TRANSCEIVER_TYPE_COMBO_E ||
        portEntryPtr->data.transceiverType == PDL_TRANSCEIVER_TYPE_FIBER_E)
    {
        xmlStatus = xmlParserGetNextSibling(xmlPortAttrId, &xmlPortAttrId);
        PDL_CHECK_STATUS(xmlStatus);

        pdlStatus = prvPdlPpPortGbicGroupParse(portEntryPtr->key.dev, portEntryPtr->key.logicalPort, xmlPortAttrId);
        PDL_CHECK_STATUS(pdlStatus);
    }
    
    /* serdes group is optional under <port> */
    xmlStatus = xmlParserGetNextSibling(xmlPortAttrId, &xmlPortSerdesId);
    if (xmlStatus == XML_PARSER_RET_CODE_OK &&
        ((xmlParserIsEqualName (xmlPortSerdesId, "serdes-exist") == XML_PARSER_RET_CODE_OK) ||
         (xmlParserIsEqualName(xmlPortSerdesId, "serdes-lane-list") == XML_PARSER_RET_CODE_OK)))
    {
        pdlStatus = prvPdlSerdesPortXmlParser (xmlPortSerdesId, portEntryPtr->key.dev, portEntryPtr->key.logicalPort);
        if (pdlStatus != PDL_NOT_FOUND)
        {
            PDL_CHECK_STATUS(pdlStatus);
        }
    }

    return PDL_OK;
}
/*$ END OF prvPdlFrontPanelSinglePortGet */

/*****************************************************************************
* FUNCTION NAME: prvPdlBack2BackSectionGet
*
* DESCRIPTION: 
*
*
*****************************************************************************/

/**
 * @fn  static PDL_STATUS prvPdlBack2BackSectionGetCurrent ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlBack2BackId, OUT PDL_PP_XML_B2B_ATTRIBUTES_STC * b2bAttrPtr )
 *
 * @brief   Prv pdl back 2 back get current version section 
 *
 * @param           xmlBack2BackId  Identifier for the XML back 2 back.
 * @param [in,out]  b2bAttrPtr      If non-null, the 2b attribute pointer.
 *
 * @return  A PDL_STATUS.
 */
static PDL_STATUS prvPdlBack2BackSectionGetCurrent (

    /*!     INPUTS:             */
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlBack2BackId,
    OUT PDL_PP_XML_B2B_ATTRIBUTES_STC     * b2bAttrPtr
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    XML_PARSER_RET_CODE_TYP             xmlStatus;
    PDL_STATUS                          pdlStatus;
    UINT_32                             maxSize, value;
    XML_PARSER_NODE_DESCRIPTOR_TYP      xmlPpB2bChildId;
    char                                pdlTagStr[PDL_XML_MAX_TAG_LEN];
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    xmlStatus = xmlParserIsEqualName(xmlBack2BackId, "Back-to-Back-Port-list");
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserGetFirstChild(xmlBack2BackId, &xmlPpB2bChildId);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserIsEqualName(xmlPpB2bChildId, "left-pp-port-number");
    XML_CHECK_STATUS(xmlStatus);
    maxSize = sizeof(b2bAttrPtr->firstPort);
    xmlStatus = xmlParserGetValue (xmlPpB2bChildId, &maxSize, &b2bAttrPtr->firstPort);
    XML_CHECK_STATUS(xmlStatus);
    b2bAttrPtr->firstMacPort = b2bAttrPtr->firstPort;

    xmlStatus = xmlParserGetNextSibling(xmlPpB2bChildId, &xmlPpB2bChildId);
    XML_CHECK_STATUS(xmlStatus);

    if (xmlParserIsEqualName(xmlPpB2bChildId, "left-logical-port-number") == XML_PARSER_RET_CODE_OK) {
        maxSize = sizeof(b2bAttrPtr->firstPort);
        xmlStatus = xmlParserGetValue (xmlPpB2bChildId, &maxSize, &b2bAttrPtr->firstPort);
        XML_CHECK_STATUS(xmlStatus);
        xmlStatus = xmlParserGetNextSibling(xmlPpB2bChildId, &xmlPpB2bChildId);
        XML_CHECK_STATUS(xmlStatus);
    }

    xmlStatus = xmlParserIsEqualName(xmlPpB2bChildId, "right-pp-port-number");
    maxSize = sizeof(b2bAttrPtr->secondPort);
    XML_CHECK_STATUS(xmlStatus);
    xmlStatus = xmlParserGetValue (xmlPpB2bChildId, &maxSize, &b2bAttrPtr->secondPort);
    XML_CHECK_STATUS(xmlStatus);
    b2bAttrPtr->secondMaclPort = b2bAttrPtr->secondPort;

    xmlStatus = xmlParserGetNextSibling(xmlPpB2bChildId, &xmlPpB2bChildId);
    XML_CHECK_STATUS(xmlStatus);

    if (xmlParserIsEqualName(xmlPpB2bChildId, "right-logical-port-number") == XML_PARSER_RET_CODE_OK) {
        maxSize = sizeof(b2bAttrPtr->secondPort);
        xmlStatus = xmlParserGetValue (xmlPpB2bChildId, &maxSize, &b2bAttrPtr->secondPort);
        XML_CHECK_STATUS(xmlStatus);
        xmlStatus = xmlParserGetNextSibling(xmlPpB2bChildId, &xmlPpB2bChildId);
        XML_CHECK_STATUS(xmlStatus);
    }


    xmlStatus = xmlParserIsEqualName(xmlPpB2bChildId, "speed");
    XML_CHECK_STATUS(xmlStatus);

    maxSize = sizeof(pdlTagStr);
    xmlStatus = xmlParserGetValue (xmlPpB2bChildId, &maxSize, &pdlTagStr[0]);
    XML_CHECK_STATUS(xmlStatus);
    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_SPEED_TYPE_E, pdlTagStr, &value);
    PDL_CHECK_STATUS(pdlStatus);
    b2bAttrPtr->maxSpeed = (PDL_PORT_SPEED_ENT)value;

    xmlStatus = xmlParserGetNextSibling(xmlPpB2bChildId, &xmlPpB2bChildId);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserIsEqualName(xmlPpB2bChildId, "mode");
    XML_CHECK_STATUS(xmlStatus);

    maxSize = sizeof(pdlTagStr);
    xmlStatus = xmlParserGetValue (xmlPpB2bChildId, &maxSize, &pdlTagStr[0]);
    XML_CHECK_STATUS(xmlStatus);
    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, pdlTagStr, &value);
    PDL_CHECK_STATUS(pdlStatus);
    b2bAttrPtr->interfaceMode = (PDL_INTERFACE_MODE_ENT)value;

    xmlStatus = xmlParserGetNextSibling(xmlPpB2bChildId, &xmlPpB2bChildId);
    if ((xmlStatus == XML_PARSER_RET_CODE_OK) && (xmlParserIsEqualName(xmlPpB2bChildId, "left-serdes-lane-list") == XML_PARSER_RET_CODE_OK))
    {
        pdlStatus = prvPdlSerdesPortXmlParser(xmlPpB2bChildId, b2bAttrPtr->firstDev, b2bAttrPtr->firstPort);
        PDL_CHECK_STATUS(pdlStatus);
    }
    while ((xmlStatus == XML_PARSER_RET_CODE_OK) && (xmlParserIsEqualName(xmlPpB2bChildId, "left-serdes-lane-list") == XML_PARSER_RET_CODE_OK))
    {
        xmlStatus = xmlParserGetNextSibling(xmlPpB2bChildId, &xmlPpB2bChildId);
    }
    if ((xmlStatus == XML_PARSER_RET_CODE_OK) && (xmlParserIsEqualName(xmlPpB2bChildId, "right-serdes-lane-list") == XML_PARSER_RET_CODE_OK))
    {
        pdlStatus = prvPdlSerdesPortXmlParser(xmlPpB2bChildId, b2bAttrPtr->secondDev, b2bAttrPtr->secondPort);
        PDL_CHECK_STATUS(pdlStatus);
    }

    return PDL_OK;
}
/*$ END OF prvPdlBack2BackSectionGetCurrent */

/**
 * @fn  static PDL_STATUS prvPdlBack2BackSectionGetLegacyV1 ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlBack2BackId, OUT PDL_PP_XML_B2B_ATTRIBUTES_STC * b2bAttrPtr )
 *
 * @brief   Prv pdl back 2 back section get legacy v1 format
 *
 * @param           xmlBack2BackId  Identifier for the XML back 2 back.
 * @param [in,out]  b2bAttrPtr      If non-null, the 2b attribute pointer.
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlBack2BackSectionGetLegacyV1 (

    /*!     INPUTS:             */
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlBack2BackId,
    OUT PDL_PP_XML_B2B_ATTRIBUTES_STC     * b2bAttrPtr
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    XML_PARSER_RET_CODE_TYP             xmlStatus;
    PDL_STATUS                          pdlStatus;
    UINT_32                             maxSize, value;
    XML_PARSER_NODE_DESCRIPTOR_TYP      xmlPpB2bChildId;
    char                                pdlTagStr[PDL_XML_MAX_TAG_LEN];
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    xmlStatus = xmlParserIsEqualName(xmlBack2BackId, "Back-to-Back-Link-list");
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserGetFirstChild (xmlBack2BackId, &xmlPpB2bChildId);        /* childs of <back-to-back-link> */
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserIsEqualName(xmlPpB2bChildId, "left-pp-device-number");
    XML_CHECK_STATUS(xmlStatus);
    maxSize = sizeof(b2bAttrPtr->firstDev);
    xmlStatus = xmlParserGetValue (xmlPpB2bChildId, &maxSize, &b2bAttrPtr->firstDev);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserGetNextSibling(xmlPpB2bChildId, &xmlPpB2bChildId);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserIsEqualName(xmlPpB2bChildId, "left-pp-port-number");
    maxSize = sizeof(b2bAttrPtr->firstPort);
    xmlStatus = xmlParserGetValue (xmlPpB2bChildId, &maxSize, &b2bAttrPtr->firstPort);
    XML_CHECK_STATUS(xmlStatus);
    b2bAttrPtr->firstMacPort = b2bAttrPtr->firstPort;

    xmlStatus = xmlParserGetNextSibling(xmlPpB2bChildId, &xmlPpB2bChildId);
    XML_CHECK_STATUS(xmlStatus);

    if (xmlParserIsEqualName(xmlPpB2bChildId, "left-logical-port-number") == XML_PARSER_RET_CODE_OK) {
        maxSize = sizeof(b2bAttrPtr->firstPort);
        xmlStatus = xmlParserGetValue (xmlPpB2bChildId, &maxSize, &b2bAttrPtr->firstPort);
        XML_CHECK_STATUS(xmlStatus);
        xmlStatus = xmlParserGetNextSibling(xmlPpB2bChildId, &xmlPpB2bChildId);
        XML_CHECK_STATUS(xmlStatus);
    }

    xmlStatus = xmlParserIsEqualName(xmlPpB2bChildId, "right-pp-device-number");
    maxSize = sizeof(b2bAttrPtr->secondDev);
    xmlStatus = xmlParserGetValue (xmlPpB2bChildId, &maxSize, &b2bAttrPtr->secondDev);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserGetNextSibling(xmlPpB2bChildId, &xmlPpB2bChildId);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserIsEqualName(xmlPpB2bChildId, "right-pp-port-number");
    maxSize = sizeof(b2bAttrPtr->secondPort);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserGetValue (xmlPpB2bChildId, &maxSize, &b2bAttrPtr->secondPort);
    XML_CHECK_STATUS(xmlStatus);
    b2bAttrPtr->secondMaclPort = b2bAttrPtr->secondPort;

    xmlStatus = xmlParserGetNextSibling(xmlPpB2bChildId, &xmlPpB2bChildId);
    XML_CHECK_STATUS(xmlStatus);

    if (xmlParserIsEqualName(xmlPpB2bChildId, "right-logical-port-number") == XML_PARSER_RET_CODE_OK) {
        maxSize = sizeof(b2bAttrPtr->secondPort);
        xmlStatus = xmlParserGetValue (xmlPpB2bChildId, &maxSize, &b2bAttrPtr->secondPort);
        XML_CHECK_STATUS(xmlStatus);
        xmlStatus = xmlParserGetNextSibling(xmlPpB2bChildId, &xmlPpB2bChildId);
        XML_CHECK_STATUS(xmlStatus);
    }

    xmlStatus = xmlParserIsEqualName(xmlPpB2bChildId, "speed");
    XML_CHECK_STATUS(xmlStatus);

    maxSize = sizeof(pdlTagStr);
    xmlStatus = xmlParserGetValue (xmlPpB2bChildId, &maxSize, &pdlTagStr[0]);
    XML_CHECK_STATUS(xmlStatus);
    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_SPEED_TYPE_E, pdlTagStr, &value);
    PDL_CHECK_STATUS(pdlStatus);
    b2bAttrPtr->maxSpeed = (PDL_PORT_SPEED_ENT)value;

    xmlStatus = xmlParserGetNextSibling(xmlPpB2bChildId, &xmlPpB2bChildId);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserIsEqualName(xmlPpB2bChildId, "mode");
    XML_CHECK_STATUS(xmlStatus);

    maxSize = sizeof(pdlTagStr);
    xmlStatus = xmlParserGetValue (xmlPpB2bChildId, &maxSize, &pdlTagStr[0]);
    XML_CHECK_STATUS(xmlStatus);
    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, pdlTagStr, &value);
    PDL_CHECK_STATUS(pdlStatus);
    b2bAttrPtr->interfaceMode = (PDL_INTERFACE_MODE_ENT)value;

    return PDL_OK;
}
/*$ END OF prvPdlBack2BackSectionGetLegacyV1 */

/**
 * @fn  static PDL_STATUS prvPdlFrontPanelGroupGet ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlFrontPanelId, OUT PRV_PDL_FRONT_PANEL_GROUP_ENTRY_STC * frontPanelGroupPtr )
 *
 * @brief   Prv pdl front panel group get
 *
 * @param           xmlFrontPanelId     Identifier for the XML front panel.
 * @param [in,out]  frontPanelGroupPtr  If non-null, the front panel group pointer.
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlFrontPanelGroupGet (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlFrontPanelId,
    OUT PRV_PDL_FRONT_PANEL_GROUP_ENTRY_STC   * frontPanelGroupPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                            pdlTagStr[PDL_XML_MAX_TAG_LEN];
    XML_PARSER_RET_CODE_TYP         xmlStatus;
    PDL_STATUS                      pdlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP  xmlFrontPanelChildId;
    PRV_PDL_PORT_ENTRY_STC          portInfo, * portInfoPtr = NULL;
    PRV_PDL_MAC_PORT_ENTRY_STC      macPortInfo, * macPortInfoPtr = NULL;
    UINT_32                         maxSize, value;
    UINT_32                         curEntriesNum, oldEntriesNum;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

    xmlStatus = xmlParserGetFirstChild (xmlFrontPanelId, &xmlFrontPanelChildId);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserIsEqualName(xmlFrontPanelChildId, "group-number");
    XML_CHECK_STATUS(xmlStatus);

    maxSize = sizeof(frontPanelGroupPtr->key.frontPanelNumber);
    xmlStatus = xmlParserGetValue(xmlFrontPanelChildId, &maxSize, &frontPanelGroupPtr->key.frontPanelNumber);
    XML_CHECK_STATUS(xmlStatus);

    if (frontPanelGroupPtr->key.frontPanelNumber < 1 || frontPanelGroupPtr->key.frontPanelNumber > 4)
    {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL frontPanelGroupPtr->key.frontPanelNumber");
        return PDL_XML_PARSE_ERROR;
    }

    xmlStatus = xmlParserGetNextSibling(xmlFrontPanelChildId, &xmlFrontPanelChildId);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserIsEqualName(xmlFrontPanelChildId, "group-ordering");
    XML_CHECK_STATUS(xmlStatus);

    maxSize = sizeof(pdlTagStr);
    xmlStatus = xmlParserGetValue(xmlFrontPanelChildId, &maxSize, &pdlTagStr[0]);
    XML_CHECK_STATUS(xmlStatus);


    if (strcmp(pdlTagStr, "RightDown") == 0)
        frontPanelGroupPtr->data.ordering = PDL_PP_FRONT_PANEL_GROUP_ORDERING_RIGHT_DOWN_E;
    else if (strcmp(pdlTagStr, "DownRight") == 0)
        frontPanelGroupPtr->data.ordering = PDL_PP_FRONT_PANEL_GROUP_ORDERING_DOWN_RIGHT_E;
    else if (strcmp(pdlTagStr, "Single2ndRow") == 0)
        frontPanelGroupPtr->data.ordering = PDL_PP_FRONT_PANEL_GROUP_ORDERING_SINGLE_2ND_ROW_E;
    else
    {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL pdlTagStr(NOT RightDown OR DownRight)");
        return PDL_XML_PARSE_ERROR;
    }

    xmlStatus = xmlParserGetNextSibling(xmlFrontPanelChildId, &xmlFrontPanelChildId);
    XML_CHECK_STATUS(xmlStatus);


    xmlStatus = xmlParserIsEqualName(xmlFrontPanelChildId, "speed");
    XML_CHECK_STATUS(xmlStatus);
    maxSize = sizeof(pdlTagStr);
    xmlStatus = xmlParserGetValue (xmlFrontPanelChildId, &maxSize, &pdlTagStr[0]);
    XML_CHECK_STATUS(xmlStatus);

    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_SPEED_TYPE_E, pdlTagStr, &value);
    PDL_CHECK_STATUS(pdlStatus);
    frontPanelGroupPtr->data.portMaximumSpeed = (PDL_PORT_SPEED_ENT)value;
    
    xmlStatus = xmlParserGetNextSibling(xmlFrontPanelChildId, &xmlFrontPanelChildId);
    XML_CHECK_STATUS(xmlStatus);

    if (XML_PARSER_RET_CODE_OK == xmlParserIsEqualName(xmlFrontPanelChildId, "prefix-name"))
    {
        xmlStatus = xmlParserGetNextSibling(xmlFrontPanelChildId, &xmlFrontPanelChildId);
        XML_CHECK_STATUS(xmlStatus);
    }

    if (XML_PARSER_RET_CODE_OK == xmlParserIsEqualName(xmlFrontPanelChildId, "first-port-number-shift"))
    {
        maxSize = sizeof(frontPanelGroupPtr->data.firstPortNumberShift);
        xmlStatus = xmlParserGetValue (xmlFrontPanelChildId, &maxSize, &frontPanelGroupPtr->data.firstPortNumberShift);
        XML_CHECK_STATUS(xmlStatus);
        xmlStatus = xmlParserGetNextSibling(xmlFrontPanelChildId, &xmlFrontPanelChildId);
    }
    else
    {
        frontPanelGroupPtr->data.firstPortNumberShift = PRV_PDL_FP_FIRST_PORT_NUMBER_DEFAULT_SHIFT_CNS;
    }
    
    if (XML_PARSER_RET_CODE_OK == xmlParserIsEqualName(xmlFrontPanelChildId, "is-logical-port-required"))
    {
        xmlStatus = xmlParserGetNextSibling(xmlFrontPanelChildId, &xmlFrontPanelChildId);
        XML_CHECK_STATUS(xmlStatus);
    }
    
    isLogicalPortRequired = FALSE;
    
    pdlStatus = prvPdlDbGetNumOfEntries(&prvPdlPpPortDb, &oldEntriesNum);
    PDL_CHECK_STATUS(pdlStatus);

    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND)
    {
        XML_CHECK_STATUS(xmlStatus);

        memset(&portInfo, 0, sizeof(portInfo));
        pdlStatus = prvPdlFrontPanelSinglePortGet(xmlFrontPanelChildId, &portInfo);
        PDL_CHECK_STATUS(pdlStatus);

        portInfo.data.frontPanelNumber = frontPanelGroupPtr->key.frontPanelNumber;
        
        frontPanelGroupPtr->data.isLogicalPortRequired = isLogicalPortRequired;
        
        /* insert instance to port DB */
        pdlStatus = prvPdlDbAdd (&prvPdlPpPortDb, (void*) &portInfo.key, (void*) &portInfo, (void**) &portInfoPtr);
        PDL_CHECK_STATUS(pdlStatus);

        macPortInfo.key.dev = portInfo.key.dev;
        macPortInfo.key.macPort = portInfo.data.macPort;
        macPortInfo.data.dev = portInfo.key.dev;
        macPortInfo.data.logicalPort = portInfo.key.logicalPort;
        /* insert instance to macLogical port DB */
        pdlStatus = prvPdlDbAdd(&prvPdlMacLogicalPortDb, (void*) &macPortInfo.key, (void*) &macPortInfo, (void**) &macPortInfoPtr);
        PDL_CHECK_STATUS(pdlStatus);

        xmlStatus = xmlParserGetNextSibling(xmlFrontPanelChildId, &xmlFrontPanelChildId);
    }

    pdlStatus = prvPdlDbGetNumOfEntries(&prvPdlPpPortDb, &curEntriesNum);
    PDL_CHECK_STATUS(pdlStatus);

    frontPanelGroupPtr->data.numOfPortsInGroup = (curEntriesNum - oldEntriesNum);

    return PDL_OK;
}
/*$ END OF prvPdlFrontPanelGroupGet */

/* ***************************************************************************
* FUNCTION NAME: prvPdlPacketProcessorXMLParser
*
* DESCRIPTION:   Fill all packet processor & network port related information from XML to DB
*
* PARAMETERS:   
*                
* XML structure:
*<number-of-pps>2</number-of-pps>
*<Back-to-Back-Links>
*    <Back-to-Back-Link>
*        <left-pp-device-number>0</left-pp-device-number>
*        <left-pp-port-number>48</left-pp-port-number>
*        <right-pp-device-number>1</right-pp-device-number>
*        <right-pp-port-number>48</right-pp-port-number>
*        <speed>10G</speed>
*    </Back-to-Back-Link>
*</Back-to-Back-Links>
*
*<network-ports>
*    <front-panel-group>
*        <group-number>1</group-number>
*        <group-ordering>RightDown</group-ordering>
*        <speed>10G</speed>
*        <port>
*            <port-number>1</port-number>
*            <pp-device-number>0</pp-device-number>
*            <pp-port-number>2</pp-port-number>
*            <speed>1G</speed>
*            <transceiver>copper</transceiver>
*            <L1-interface-modes-group>
*                <L1-interface-mode>
*                    <speed>100M</speed>
*                    <mode>SGMII</mode>
*                </L1-interface-mode>
*                <L1-interface-mode>
*                    <speed>1G</speed>
*                    <mode>SGMII</mode>
*                </L1-interface-mode>
*            </L1-interface-modes-group>
*        </port>
*        <port>
*            <port-number>2</port-number>
*            <pp-device-number>0</pp-device-number>
*            <pp-port-number>3</pp-port-number>
*            <speed>1G</speed>
*            <transceiver>copper</transceiver>
*            <L1-interface-modes-group>
*                <L1-interface-mode>
*                    <speed>100M</speed>
*                    <mode>SGMII</mode>
*                </L1-interface-mode>
*                <L1-interface-mode>
*                    <speed>1G</speed>
*                    <mode>SGMII</mode>
*                </L1-interface-mode>
*            </L1-interface-modes-group>
*        </port>
*    </front-panel-group>
*    <front-panel-group>
*        <group-number>2</group-number>
*        <group-ordering>RightDown</group-ordering>
*        <speed>1G</speed>
*        <port>
*            <port-number>5</port-number>
*            <pp-device-number>0</pp-device-number>
*            <pp-port-number>46</pp-port-number>
*            <speed>1G</speed>
*            <transceiver>copper</transceiver>
*            <L1-interface-modes-group>
*                <L1-interface-mode>
*                    <speed>100M</speed>
*                    <mode>SGMII</mode>
*                </L1-interface-mode>
*                <L1-interface-mode>
*                    <speed>1G</speed>
*                    <mode>SGMII</mode>
*                </L1-interface-mode>
*            </L1-interface-modes-group>
*        </port>
*        <port>
*            <port-number>6</port-number>
*            <pp-device-number>0</pp-device-number>
*            <pp-port-number>47</pp-port-number>
*            <speed>1G</speed>
*            <transceiver>copper</transceiver>
*            <L1-interface-modes-group>
*                <L1-interface-mode>
*                    <speed>100M</speed>
*                    <mode>SGMII</mode>
*                </L1-interface-mode>
*                <L1-interface-mode>
*                    <speed>1G</speed>
*                    <mode>SGMII</mode>
*                </L1-interface-mode>
*            </L1-interface-modes-group>
*        </port>
*    </front-panel-group>
*</network-ports>
*
*****************************************************************************/

/**
 * @fn  static PDL_STATUS prvPdlPacketProcessorXMLParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Prv pdl packet processor XML parser
 *
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlPacketProcessorXMLParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         b2bCount = 0, firstDev = 0, secondDev = 0;
    XML_PARSER_RET_CODE_TYP         xmlStatus;
    PDL_STATUS                      pdlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP  xmlPpId, xmlPpB2bId, xmlNetworkPortId, xmlFrontPanelId;
    UINT_32                         maxSize;
    PRV_PDL_FRONT_PANEL_GROUP_ENTRY_STC frontPanelGroup, *frontPanelGroupPtr;
    char                            sectionNameStr[30];
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

    xmlStatus = xmlParserFindByName (xmlId, "number-of-pps", &xmlPpId);
    /* section is not mandatory */
    if (xmlStatus == XML_PARSER_RET_CODE_NOT_FOUND) {
        return PDL_OK;
    }
    XML_CHECK_STATUS(xmlStatus);

    pdlPpDb.numOfPps = 0;
    maxSize = sizeof(pdlPpDb.numOfPps);
    xmlStatus = xmlParserGetValue (xmlPpId, &maxSize, &pdlPpDb.numOfPps);
    if (xmlStatus != XML_PARSER_RET_CODE_OK)
    {
        if (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND)
            return PDL_XML_PARSE_ERROR;

        pdlPpDb.numOfPps = 1;
    }

    if (pdlPpDb.numOfPps != 1 && pdlPpDb.numOfPps != 2)
    {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL NUM OF Pps(NOT 1 OR 2)");
        return PDL_XML_PARSE_ERROR;
    }

    if (pdlPpDb.numOfPps == 2)
    {
        xmlStatus = xmlParserFindByName (xmlId, "Back-to-Back-Link-Information", &xmlPpB2bId);
        if (xmlStatus != XML_PARSER_RET_CODE_OK)
        {
            xmlStatus = xmlParserFindByName (xmlId, "Back-to-Back-links", &xmlPpB2bId);
            if (xmlStatus != XML_PARSER_RET_CODE_OK)
            {
                PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"No Back-to-Back section found");
                XML_CHECK_STATUS(xmlStatus);
            }
        }

        maxSize = sizeof(sectionNameStr);
        xmlStatus = xmlParserGetName(xmlPpB2bId, &maxSize, sectionNameStr);
        if (xmlStatus != XML_PARSER_RET_CODE_OK)
        {
            PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"Illegal Back-to-Back section name length");
            XML_CHECK_STATUS(xmlStatus);
        }

        xmlStatus = xmlParserGetFirstChild (xmlPpB2bId, &xmlPpB2bId);
        if (strcmp(sectionNameStr, "Back-to-Back-Link-Information") == 0) /* 'current' b2b info section handling */
        {
            XML_CHECK_STATUS(xmlStatus);
            xmlStatus = xmlParserIsEqualName(xmlPpB2bId, "left-pp-device-number");
            XML_CHECK_STATUS(xmlStatus);
            maxSize = sizeof(firstDev);
            xmlStatus = xmlParserGetValue (xmlPpB2bId, &maxSize, &firstDev);
            XML_CHECK_STATUS(xmlStatus);
            xmlStatus = xmlParserGetNextSibling(xmlPpB2bId, &xmlPpB2bId);
            XML_CHECK_STATUS(xmlStatus);

            xmlStatus = xmlParserIsEqualName(xmlPpB2bId, "right-pp-device-number");
            XML_CHECK_STATUS(xmlStatus);
            maxSize = sizeof(secondDev);
            xmlStatus = xmlParserGetValue (xmlPpB2bId, &maxSize, &secondDev);
            XML_CHECK_STATUS(xmlStatus);
            xmlStatus = xmlParserGetNextSibling(xmlPpB2bId, &xmlPpB2bId);
            XML_CHECK_STATUS(xmlStatus);
        }

        while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND)
        {
            XML_CHECK_STATUS(xmlStatus);

            pdlPpB2bDb[b2bCount].firstDev = firstDev;
            pdlPpB2bDb[b2bCount].secondDev = secondDev;

            if (strcmp(sectionNameStr, "Back-to-Back-Link-Information") == 0)
            {
                pdlStatus = prvPdlBack2BackSectionGetCurrent(xmlPpB2bId, &pdlPpB2bDb[b2bCount]);
            }
            else
            {
                pdlStatus = prvPdlBack2BackSectionGetLegacyV1(xmlPpB2bId, &pdlPpB2bDb[b2bCount]);
            }
            PDL_CHECK_STATUS(pdlStatus);

            b2bCount++;
            xmlStatus = xmlParserGetNextSibling(xmlPpB2bId, &xmlPpB2bId);
        }
    }

    pdlPpDb.numOfBackToBackLinksPerPp = b2bCount;

    /* <network-ports> */
    xmlStatus = xmlParserFindByName (xmlId, "Network-ports", &xmlNetworkPortId);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserGetFirstChild (xmlNetworkPortId, &xmlFrontPanelId);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserIsEqualName(xmlFrontPanelId, "front-panel-group-list");

    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);

        memset(&frontPanelGroup, 0, sizeof(PRV_PDL_FRONT_PANEL_GROUP_ENTRY_STC));

        pdlStatus = prvPdlFrontPanelGroupGet(xmlFrontPanelId, &frontPanelGroup);
        PDL_CHECK_STATUS(pdlStatus);

        /* insert instance to front panel DB */
        pdlStatus = prvPdlDbAdd(&prvPdlFrontPanelDb, (void*) &frontPanelGroup.key, (void*) &frontPanelGroup, (void**) &frontPanelGroupPtr);
        PDL_CHECK_STATUS(pdlStatus);

        xmlStatus = xmlParserGetNextSibling (xmlFrontPanelId, &xmlFrontPanelId);
    }

    pdlStatus = prvPdlDbGetNumOfEntries(&prvPdlFrontPanelDb, &pdlPpDb.numOfFrontPanelGroups);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}
/*$ END OF prvPdlPacketProcessorXMLParser */


/**
 * @fn  PDL_STATUS pdlPpDbAttributesGet ( OUT PDL_PP_XML_ATTRIBUTES_STC * ppAttributesPtr )
 *
 * @brief   Get packet processor attributes
 *
 * @param [out] ppAttributesPtr packet processor attributes.
 *
 * @return  PDL_BAD_PTR  illegal pointer supplied.
 */

PDL_STATUS pdlPpDbAttributesGet (
    OUT  PDL_PP_XML_ATTRIBUTES_STC         * ppAttributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (ppAttributesPtr == NULL) {
         PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"ppAttributesPtr NULL");
        return PDL_BAD_PTR;
    }
    memcpy (ppAttributesPtr, &pdlPpDb, sizeof (pdlPpDb));
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, ppAttributesPtr->numOfPps);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, ppAttributesPtr->numOfFrontPanelGroups);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, ppAttributesPtr->numOfBackToBackLinksPerPp);
    return PDL_OK;
}
/*$ END OF pdlPpDbAttributesGet */

/**
 * @fn  PDL_STATUS pdlPpDbB2bAttributesGet ( IN UINT_32 b2bLinkId, OUT PDL_PP_XML_B2B_ATTRIBUTES_STC * b2bAttributesPtr )
 *
 * @brief   Get PP back-to-back link attributes
 *
 * @param [in]  b2bLinkId           b2b link number.
 * @param [out] b2bAttributesPtr    b2b link attributes.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_BAD_PARAM  b2b id is out of range.
 */

PDL_STATUS pdlPpDbB2bAttributesGet (
    IN   UINT_32                                 b2bLinkId,
    OUT  PDL_PP_XML_B2B_ATTRIBUTES_STC         * b2bAttributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (b2bAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"b2bAttributesPtr NULL");
        return PDL_BAD_PTR;
    }
    if (b2bLinkId >= pdlPpDb.numOfBackToBackLinksPerPp) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL VALUE TO b2bLinkId(VALUE: %d)",b2bLinkId);
        return PDL_BAD_PARAM;
    }
    memcpy (b2bAttributesPtr, &pdlPpB2bDb[b2bLinkId], sizeof (PDL_PP_XML_B2B_ATTRIBUTES_STC));
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, b2bAttributesPtr->firstDev);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, b2bAttributesPtr->firstPort);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, b2bAttributesPtr->secondDev);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, b2bAttributesPtr->secondPort);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, b2bAttributesPtr->maxSpeed);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, b2bAttributesPtr->interfaceMode);
    return PDL_OK;
}
/*$ END OF pdlPpDbB2bAttributesGet */

/**
 * @fn  PDL_STATUS pdlPpDbIsB2bLink ( IN UINT_32 dev, IN UINT_32 logicalPort, OUT BOOLEAN * isB2bLinkPtr )
 *
 * @brief   Determines whether PP port is back-to-back link.
 *
 * @param [in]  dev             dev number.
 * @param [in]  logicalPort     port number.
 * @param [out] isB2bLinkPtr    is b2b link.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  no such port was found.
 */
PDL_STATUS pdlPpDbIsB2bLink (
    IN   UINT_32            dev,
    IN   UINT_32            logicalPort,
    OUT  BOOLEAN          * isB2bLinkPtr
)
{
/*****************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*****************************************************************************/
    UINT_32                                 b2bLink;
    PDL_STATUS                              pdlStatus;
    PDL_PP_XML_ATTRIBUTES_STC               ppAttributes;
    PDL_PP_XML_B2B_ATTRIBUTES_STC           b2bAttributes;
    PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC  portAttributes;
/*****************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/*****************************************************************************/

    if (isB2bLinkPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"isB2bLinkPtr NULL");
        return PDL_BAD_PTR;
    }

    pdlStatus = pdlPpDbAttributesGet(&ppAttributes);
    if (PDL_OK != pdlStatus)
    {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"can't get pp attributes");
        return pdlStatus;
    }

    for (b2bLink = 0; b2bLink < ppAttributes.numOfBackToBackLinksPerPp; b2bLink ++)
    {
        pdlStatus = pdlPpDbB2bAttributesGet(b2bLink, &b2bAttributes);
        if (PDL_OK != pdlStatus)
            continue;
        if (b2bAttributes.firstDev == dev && b2bAttributes.firstPort == logicalPort)
        {
            *isB2bLinkPtr = TRUE;
            return PDL_OK;
        }
        if (b2bAttributes.secondDev == dev && b2bAttributes.secondPort == logicalPort)
        {
            *isB2bLinkPtr = TRUE;
            return PDL_OK;
        }
    }

    /* not b2b link - validate port exists */
    if (pdlPpDbPortAttributesGet(dev, logicalPort, &portAttributes) == PDL_OK)
    {
        *isB2bLinkPtr = FALSE;
        return PDL_OK;
    }

    return PDL_NOT_FOUND;
}
/*$ END OF pdlPpDbIsB2bLink */

/**
 * @fn  PDL_STATUS pdlPpDbPortAttributesGet ( IN UINT_32 dev, IN UINT_32 logicalPort, OUT PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC * portAttributesPtr )
 *
 * @brief   Get PP port attributes
 *
 * @param [in]  dev                 dev number.
 * @param [in]  port                logicalPort number.
 * @param [out] portAttributesPtr   port attributes.
 *
 * @return  PDL_BAD_PTR  illegal pointer supplied.
 */

PDL_STATUS pdlPpDbPortAttributesGet (
    IN   UINT_32                                    dev,
    IN   UINT_32                                    logicalPort,   
    OUT  PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC   * portAttributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PRV_PDL_PORT_ENTRY_STC                * portEntryPtr;
    PRV_PDL_PORT_ENTRY_KEY_STC              portEntryKey;

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (portAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"portAttributesPtr NULL, PORT NUM: %d, DEVICE NUM: %d",logicalPort ,dev);
        return PDL_BAD_PTR;
    }
    portEntryKey.dev = dev;
    portEntryKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&prvPdlPpPortDb, (void*) &portEntryKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    memcpy (portAttributesPtr, &portEntryPtr->data, sizeof (PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC));

    return PDL_OK;
}
/*$ END OF pdlPpDbPortAttributesGet */

/**
 * @fn  PDL_STATUS pdlPpDbPortPhySetPostInitValues ()
 *
 * @brief   write port phy post init values
 *
 * @param [in]  dev                 dev number.
 * @param [in]  logicalPort         logicalPort number.
 *
 * @return  PDL_OK              Registers configured ok or not required
 */

PDL_STATUS pdlPpDbPortPhySetPostInitValues (
    IN   UINT_32                                      dev,
    IN   UINT_32                                      logicalPort
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                        pdlStatus;
    PRV_PDL_PORT_ENTRY_STC                          * portEntryPtr;
    PRV_PDL_PORT_ENTRY_KEY_STC                        portEntryKey;
    PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC       * phyPostInitValueInfoPtr;
   
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    portEntryKey.dev = dev;
    portEntryKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&prvPdlPpPortDb, (void*) &portEntryKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlDbGetFirst (&portEntryPtr->phyPostInitValues, (void**) &phyPostInitValueInfoPtr);
    while (pdlStatus == PDL_OK) {
        pdlStatus = pdlSmiXsmiHwSetValue (portEntryPtr->phyInterfaceId,
                                          phyPostInitValueInfoPtr->info.devOrPage,
                                          phyPostInitValueInfoPtr->info.registerAddress,
                                          phyPostInitValueInfoPtr->info.mask,
                                          phyPostInitValueInfoPtr->info.value);
        PDL_CHECK_STATUS(pdlStatus);
        PDL_LIB_DEBUG_MAC(prvPdlPhyDebugFlag)(__FUNCTION__, __LINE__, "devOrPage=[%d] regAddress=[0x%x] mask=0x%x] value=[0x%x] ",phyPostInitValueInfoPtr->info.devOrPage,
                                                                                                                                  phyPostInitValueInfoPtr->info.registerAddress,
                                                                                                                                  phyPostInitValueInfoPtr->info.mask,
                                                                                                                                  phyPostInitValueInfoPtr->info.value);
        pdlStatus = prvPdlDbGetNext (&portEntryPtr->phyPostInitValues, &phyPostInitValueInfoPtr->key, (void**)&phyPostInitValueInfoPtr);
    }

    return PDL_OK;
}
/*$ END OF pdlPpDbPortPhySetPostInitValues */

/**
 * @fn  PDL_STATUS pdlPpDbPortPhyPostInitValuesGetFirst ()
 *
 * @brief   Get first port phy post init values
 *
 * @param [in]  dev              dev number.
 * @param [in]  logicalPort      logicalPort number.
 * @param [out] initValuesPtr    ptr to data.
 *
 * @return  PDL_BAD_PTR          if initValuesPtr is NULL
 * @return  PDL_NO_MORE          No more data available
 */
PDL_STATUS pdlPpDbPortPhyPostInitValuesGetFirst (
    IN  UINT_32                                     dev,
    IN  UINT_32                                     logicalPort,
    OUT PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC  *initValuesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                        pdlStatus;
    PRV_PDL_PORT_ENTRY_STC                          * portEntryPtr;
    PRV_PDL_PORT_ENTRY_KEY_STC                        portEntryKey;
    PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC       * phyPostInitValueInfoPtr;

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (initValuesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    portEntryKey.dev = dev;
    portEntryKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&prvPdlPpPortDb, (void*) &portEntryKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlDbGetFirst (&portEntryPtr->phyPostInitValues, (void**) &phyPostInitValueInfoPtr);
    if (pdlStatus == PDL_OK)
    {
        memcpy(initValuesPtr, phyPostInitValueInfoPtr, sizeof(PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC));
    }
    return pdlStatus;
}
/*$ END OF pdlPpDbPortPhyPostInitValuesGetFirst */

/**
 * @fn  PDL_STATUS pdlPpDbPortPhyPostInitValuesGetNext ()
 *
 * @brief   Get next port phy post init values
 *
 * @param [in]    dev              dev number.
 * @param [in]    logicalPort      logicalPort number.
 * @param [inout] initValuesPtr    ptr to data.
 *
 * @return  PDL_BAD_PTR          if initValuesPtr is NULL
 * @return  PDL_NO_MORE          No more data available
 */
PDL_STATUS pdlPpDbPortPhyPostInitValuesGetNext (
    IN    UINT_32                                     dev,
    IN    UINT_32                                     logicalPort,
    INOUT PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC  *initValuesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                        pdlStatus;
    PRV_PDL_PORT_ENTRY_STC                          * portEntryPtr;
    PRV_PDL_PORT_ENTRY_KEY_STC                        portEntryKey;
    PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC       * phyPostInitValueInfoPtr;

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (initValuesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    portEntryKey.dev = dev;
    portEntryKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&prvPdlPpPortDb, (void*) &portEntryKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlDbGetNext (&portEntryPtr->phyPostInitValues, &initValuesPtr->key, (void**) &phyPostInitValueInfoPtr);
    if (pdlStatus == PDL_OK)
    {
        memcpy(initValuesPtr, phyPostInitValueInfoPtr, sizeof(PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC));
    }
    return pdlStatus;
}
/*$ END OF pdlPpDbPortPhyPostInitValuesGetNext */

/**
 * @fn  PDL_STATUS pdlPpDbPortPhySetInitValues ()
 *
 * @brief   write port phy init values for given speed & transceiver type
 *
 * @param [in]  dev                 dev number.
 * @param [in]  logicalPort         logicalPort number.
 * @param [in]  speed               port speed
 * @param [in]  transceiverType     transceiverType (copper/fiber)
 *
 * @return  PDL_BAD_PARAM       illegal transceiverType
            PDL_NO_SUCH         phy_init configuration is required but not for this speed & transceiverType
            PDL_NOT_SUPPORTED   phy_init configuration isn't required
            PDL_OK              phy_init configured ok
 */

PDL_STATUS pdlPpDbPortPhySetInitValues (
    IN   UINT_32                                      dev,
    IN   UINT_32                                      logicalPort,
    IN   PDL_PORT_SPEED_ENT                           speed,
    IN   PDL_TRANSCEIVER_TYPE_ENT                     transceiverType
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    BOOLEAN                                           phyInitRequired = FALSE;
    UINT_32                                           i, numOfEntries;
    PDL_STATUS                                        pdlStatus;
    PRV_PDL_PORT_ENTRY_STC                          * portEntryPtr;
    PRV_PDL_PORT_ENTRY_KEY_STC                        portEntryKey;
    PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC       * phyInitValueInfoPtr;
   
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    portEntryKey.dev  = dev;
    portEntryKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&prvPdlPpPortDb, (void*) &portEntryKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (transceiverType == PDL_TRANSCEIVER_TYPE_COPPER_E) {
        pdlStatus = prvPdlDbGetFirst (&portEntryPtr->copperPhyInitValueDb[speed], (void**) &phyInitValueInfoPtr);
        while (pdlStatus == PDL_OK) {
            phyInitRequired = TRUE;
            pdlStatus = pdlSmiXsmiHwSetValue (portEntryPtr->phyInterfaceId, 
                                              phyInitValueInfoPtr->info.devOrPage,
                                              phyInitValueInfoPtr->info.registerAddress,
                                              phyInitValueInfoPtr->info.mask,
                                              phyInitValueInfoPtr->info.value);
            PDL_CHECK_STATUS(pdlStatus);
            PDL_LIB_DEBUG_MAC(prvPdlPhyDebugFlag)(__FUNCTION__, __LINE__, "(Copper) devOrPage=[%d] regAddress=[0x%x] mask=0x%x] value=[0x%x] ",phyInitValueInfoPtr->info.devOrPage,
                                                                                                                                               phyInitValueInfoPtr->info.registerAddress,
                                                                                                                                               phyInitValueInfoPtr->info.mask,
                                                                                                                                               phyInitValueInfoPtr->info.value);
            pdlStatus = prvPdlDbGetNext (&portEntryPtr->copperPhyInitValueDb[speed], &phyInitValueInfoPtr->key, (void**)&phyInitValueInfoPtr);
        } 
    }
    else  if (transceiverType == PDL_TRANSCEIVER_TYPE_FIBER_E) {
        pdlStatus = prvPdlDbGetFirst (&portEntryPtr->fiberPhyInitValueDb[speed], (void**) &phyInitValueInfoPtr);
        while (pdlStatus == PDL_OK) {
            phyInitRequired = TRUE;
            pdlStatus = pdlSmiXsmiHwSetValue (portEntryPtr->phyInterfaceId, 
                                              phyInitValueInfoPtr->info.devOrPage,
                                              phyInitValueInfoPtr->info.registerAddress,
                                              phyInitValueInfoPtr->info.mask,
                                              phyInitValueInfoPtr->info.value);
            PDL_CHECK_STATUS(pdlStatus);
            PDL_LIB_DEBUG_MAC(prvPdlPhyDebugFlag)(__FUNCTION__, __LINE__, "(Fiber) devOrPage=[%d] regAddress=[0x%x] mask=0x%x] value=[0x%x] ",phyInitValueInfoPtr->info.devOrPage,
                                                                                                                                              phyInitValueInfoPtr->info.registerAddress,
                                                                                                                                              phyInitValueInfoPtr->info.mask,
                                                                                                                                              phyInitValueInfoPtr->info.value);

            pdlStatus = prvPdlDbGetNext (&portEntryPtr->fiberPhyInitValueDb[speed], &phyInitValueInfoPtr->key, (void**)&phyInitValueInfoPtr);
        } 
    }
    else {
        return PDL_BAD_PARAM;
    }

    /* check if any interface mode requires phy_init for this interface */
    if (phyInitRequired == TRUE) {
        return PDL_OK;
    }
    else {
        for (i = 0; i < PDL_PORT_SPEED_LAST_E; i++) {
            pdlStatus = prvPdlDbGetNumOfEntries (&portEntryPtr->copperPhyInitValueDb[i], & numOfEntries);
            if (pdlStatus == PDL_OK && numOfEntries != 0) {
                return PDL_NO_SUCH;
            }
            pdlStatus = prvPdlDbGetNumOfEntries (&portEntryPtr->fiberPhyInitValueDb[i], & numOfEntries);
            if (pdlStatus == PDL_OK && numOfEntries != 0) {
                return PDL_NO_SUCH;
            }
        }
    }

    return PDL_NOT_SUPPORTED;
}
/*$ END OF pdlPpDbPortPhySetInitValues */

/**
 * @fn  PDL_STATUS pdlPpDbPortPhyInitValuesGetFirst ()
 *
 * @brief   Get first port phy init values
 *
 * @param [in]     dev              dev number.
 * @param [in]     logicalPort      logicalPort number.
 * @param [in]     speed            port speed
 * @param [in]     transceiverType  transceiverType (copper/fiber)
 * @param [inout]  initValuesPtr    ptr to data.
 *
 * @return  PDL_BAD_PARAM        illegal speed or transceiverType
 * @return  PDL_BAD_PTR          if initValuesPtr is NULL
 * @return  PDL_NO_MORE          No more data available
 */
PDL_STATUS pdlPpDbPortPhyInitValuesGetFirst (
    IN    UINT_32                                     dev,
    IN    UINT_32                                     logicalPort,
    IN    PDL_PORT_SPEED_ENT                          speed,
    IN    PDL_TRANSCEIVER_TYPE_ENT                    transceiverType,
    INOUT PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC  *initValuesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                        pdlStatus;
    PRV_PDL_PORT_ENTRY_STC                          * portEntryPtr;
    PRV_PDL_PORT_ENTRY_KEY_STC                        portEntryKey;
    PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC       * phyInitValueInfoPtr;
    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (speed >= PDL_PORT_SPEED_LAST_E)
    {
        return PDL_BAD_PARAM;
    }

    if (initValuesPtr == NULL)
    {
        return PDL_BAD_PTR;
    }

    portEntryKey.dev  = dev;
    portEntryKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&prvPdlPpPortDb, (void*) &portEntryKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (transceiverType == PDL_TRANSCEIVER_TYPE_COPPER_E)
    {
        pdlStatus = prvPdlDbGetFirst (&portEntryPtr->copperPhyInitValueDb[speed], (void**) &phyInitValueInfoPtr);
        if (pdlStatus == PDL_OK)
        {
            memcpy(initValuesPtr, phyInitValueInfoPtr, sizeof(PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC));
        }
    }
    else if (transceiverType == PDL_TRANSCEIVER_TYPE_FIBER_E)
    {
        pdlStatus = prvPdlDbGetFirst (&portEntryPtr->fiberPhyInitValueDb[speed], (void**) &phyInitValueInfoPtr);
        if (pdlStatus == PDL_OK)
        {
            memcpy(initValuesPtr, phyInitValueInfoPtr, sizeof(PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC));
        }
    }
    else {
        pdlStatus = PDL_BAD_PARAM;
    }
    return pdlStatus;
}
/*$ END OF pdlPpDbPortPhyInitValuesGetFirst */

/**
 * @fn  PDL_STATUS pdlPpDbPortPhyInitValuesGetNext ()
 *
 * @brief   Get next port phy init values
 *
 * @param [in]     dev              dev number.
 * @param [in]     logicalPort      logicalPort number.
 * @param [in]     speed            port speed
 * @param [in]     transceiverType  transceiverType (copper/fiber)
 * @param [inout]  initValuesPtr    ptr to data.
 *
 * @return  PDL_BAD_PARAM        illegal speed or transceiverType
 * @return  PDL_BAD_PTR          if initValuesPtr is NULL
 * @return  PDL_NO_MORE          No more data available
 */
PDL_STATUS pdlPpDbPortPhyInitValuesGetNext (
    IN    UINT_32                                     dev,
    IN    UINT_32                                     logicalPort,
    IN    PDL_PORT_SPEED_ENT                          speed,
    IN    PDL_TRANSCEIVER_TYPE_ENT                    transceiverType,
    INOUT PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC  *initValuesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                        pdlStatus;
    PRV_PDL_PORT_ENTRY_STC                          * portEntryPtr;
    PRV_PDL_PORT_ENTRY_KEY_STC                        portEntryKey;
    PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC       * phyInitValueInfoPtr;
     
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (speed >= PDL_PORT_SPEED_LAST_E)
    {
        return PDL_BAD_PARAM;
    }

    if (initValuesPtr == NULL)
    {
        return PDL_BAD_PTR;
    }

    portEntryKey.dev  = dev;
    portEntryKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&prvPdlPpPortDb, (void*) &portEntryKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (transceiverType == PDL_TRANSCEIVER_TYPE_COPPER_E)
    {
        pdlStatus = prvPdlDbGetNext (&portEntryPtr->copperPhyInitValueDb[speed], &initValuesPtr->key, (void**) &phyInitValueInfoPtr);
        if (pdlStatus == PDL_OK)
        {
            memcpy(initValuesPtr, phyInitValueInfoPtr, sizeof(PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC));
        }
    }
    else if (transceiverType == PDL_TRANSCEIVER_TYPE_FIBER_E)
    {
        pdlStatus = prvPdlDbGetNext (&portEntryPtr->fiberPhyInitValueDb[speed], &initValuesPtr->key, (void**) &phyInitValueInfoPtr);
        if (pdlStatus == PDL_OK)
        {
            memcpy(initValuesPtr, phyInitValueInfoPtr, sizeof(PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC));
        }
    }
    else {
        pdlStatus = PDL_BAD_PARAM;
    }
    return pdlStatus;
}
/*$ END OF pdlPpDbPortPhyInitValuesGetNext */

/**
* @public pdlPpDbFirstPortAttributesGet
*
* @brief  Get first existing dev/port attributes.
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*
* @param[out] firstDevPtr                first dev number
* @param[out] firstPortPtr               first port number
*/
PDL_STATUS pdlPpDbFirstPortAttributesGet (
    OUT  UINT_32                                   *firstDevPtr,
    OUT  UINT_32                                   *firstPortPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PRV_PDL_PORT_ENTRY_STC                * portEntryPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (firstDevPtr == NULL || firstPortPtr == NULL) {
        return PDL_BAD_PTR;
    }

    pdlStatus = prvPdlDbGetNext (&prvPdlPpPortDb, NULL, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    *firstDevPtr = portEntryPtr->key.dev;
    *firstPortPtr = portEntryPtr->key.logicalPort;

    return PDL_OK;
}
/*$ END OF pdlPpDbFirstPortAttributesGet */

/**
* @public pdlPpDbPortAttributesGetNext
*
* @brief  Get next existing dev/port.
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*
* @param[in] currDev                    current dev number
* @param[in] currPort                   current port number
* @param[out] nextDevPtr                next dev number
* @param[out] nextPortPtr               next port number
*/
PDL_STATUS pdlPpDbPortAttributesGetNext (
    IN   UINT_32                                    dev,
    IN   UINT_32                                    logicalPort,
    OUT  UINT_32                                  * nextDevPtr,
    OUT  UINT_32                                  * nextPortPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PRV_PDL_PORT_ENTRY_STC                * portEntryPtr;
    PRV_PDL_PORT_ENTRY_KEY_STC              portEntryKey;
   
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (nextDevPtr == NULL || nextPortPtr == NULL) {
        return PDL_BAD_PTR;
    }

    portEntryKey.dev  = dev;
    portEntryKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbGetNext (&prvPdlPpPortDb, (void*) &portEntryKey, (void**) &portEntryPtr);
    if (pdlStatus != PDL_OK) {
        return pdlStatus;
    }

    *nextDevPtr = portEntryPtr->key.dev;
    *nextPortPtr = portEntryPtr->key.logicalPort;

    return PDL_OK;
}
/*$ END OF pdlPpDbPortAttributesGetNext */

/**
* @public pdlPpDbFirstPortAttributesGet
*
* @brief  Get first existing front panel number.
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*
* @param[out] pdlPpDbFirstFrontPanelGet  first front panel number
*/
PDL_STATUS pdlPpDbFirstFrontPanelGet (
    OUT  UINT_32                *firstFrontPanelPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PRV_PDL_FRONT_PANEL_GROUP_ENTRY_STC     *frontPanelEntryPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (firstFrontPanelPtr == NULL) {
        return PDL_BAD_PTR;
    }

    pdlStatus = prvPdlDbGetNext(&prvPdlFrontPanelDb, NULL, (void**) &frontPanelEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    *firstFrontPanelPtr = frontPanelEntryPtr->key.frontPanelNumber;

    return PDL_OK;
}
/*$ END OF pdlPpDbFirstFrontPanelGet */

/**
* @public pdlPpDbFrontPanelGetNext
*
* @brief  Get next existing front panel
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*
* @param[in] frontPanenlNumber          current front panel number
* @param[out] nextFrontPanenlNumberPtr  next front panel number
*/
PDL_STATUS pdlPpDbFrontPanelGetNext (
    IN   UINT_32                frontPanenlNumber,
    OUT  UINT_32                *nextFrontPanenlNumberPtr
)
{
/*****************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PRV_PDL_FRONT_PANEL_GROUP_ENTRY_STC   * frontPanelEntryPtr;
    PRV_PDL_FRONT_PANEL_GROUP_KEY_STC       frontPanelEntryKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (nextFrontPanenlNumberPtr == NULL) {
        return PDL_BAD_PTR;
    }

    frontPanelEntryKey.frontPanelNumber = frontPanenlNumber;

    pdlStatus = prvPdlDbGetNext (&prvPdlFrontPanelDb, (void*) &frontPanelEntryKey, (void**) &frontPanelEntryPtr);
    if (pdlStatus != PDL_OK) {
        return pdlStatus;
    }

    *nextFrontPanenlNumberPtr = frontPanelEntryPtr->key.frontPanelNumber;

    return PDL_OK;
}
/*$ END OF pdlPpDbFrontPanelGetNext */

/**
* @public pdlPpDbDevAttributesGetFirstPort
*
* @brief  Get first existing port of device.
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*
* @param[in]  dev                dev number
* @param[out] firstPortPtr       first port number
*/
PDL_STATUS pdlPpDbDevAttributesGetFirstPort (
    IN   UINT_32       dev,
    OUT  UINT_32       *firstPortPtr
)
{
/****************************************************************************/
/*L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/****************************************************************************/
    PDL_STATUS                              pdlStatus;
    PRV_PDL_PORT_ENTRY_STC                * portEntryPtr;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    if (firstPortPtr == NULL) {
        return PDL_BAD_PTR;
    }

    pdlStatus = prvPdlDbGetNext (&prvPdlPpPortDb, NULL, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    while (dev != portEntryPtr->key.dev)
    {
        pdlStatus = prvPdlDbGetNext (&prvPdlPpPortDb, (void*) &portEntryPtr->key, (void**) &portEntryPtr);
        if (pdlStatus != PDL_OK) {
            return pdlStatus;
        }
    }
    *firstPortPtr = portEntryPtr->key.logicalPort;

    return PDL_OK;
}
/*$ END OF pdlPpDbDevAttributesGetFirstPort */

/**
* @public pdlPpDbDevAttributesGetNextPort
*
* @brief  Get next existing port of device.
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*
* @param[in]  dev                dev number
* @param[in]  currentPort        current port number
* @param[out] nextPortPtr        next port number
*/
PDL_STATUS pdlPpDbDevAttributesGetNextPort (
    IN   UINT_32       dev,
    IN   UINT_32       currentPort,
    OUT  UINT_32       *nextPortPtr
)
{
/****************************************************************************/
/*L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/****************************************************************************/
    PDL_STATUS                              pdlStatus;
    PRV_PDL_PORT_ENTRY_STC                * portEntryPtr;
    PRV_PDL_PORT_ENTRY_KEY_STC              portEntryKey;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    if (nextPortPtr == NULL) {
        return PDL_BAD_PTR;
    }

    portEntryKey.dev  = dev;
    portEntryKey.logicalPort = currentPort;

    pdlStatus = prvPdlDbGetNext (&prvPdlPpPortDb, &portEntryKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    while (dev != portEntryPtr->key.dev)
    {
        pdlStatus = prvPdlDbGetNext (&prvPdlPpPortDb, (void*) &portEntryPtr->key, (void**) &portEntryPtr);
        if (pdlStatus != PDL_OK) {
            return pdlStatus;
        }
    }
    *nextPortPtr = portEntryPtr->key.logicalPort;

    return PDL_OK;
}
/*$ END OF pdlPpDbDevAttributesGetNextPort */

/**
 * @fn  PDL_STATUS pdlPpDbDevAttributesGet ( IN UINT_32 dev, OUT UINT_32 * numOfPortsPtr )
 *
 * @brief   Get number of pp port
 *
 * @param [in]  dev             dev number.
 * @param [out] numOfPortsPtr   device's number of ports.
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlPpDbDevAttributesGet (
    IN   UINT_32                                    dev,
    OUT  UINT_32                                  * numOfPortsPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PRV_PDL_PORT_ENTRY_STC                * portEntryPtr;
    UINT_32                                 count = 0;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (numOfPortsPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"numOfPortsPtr NULL");
        return PDL_BAD_PTR;
    }

    pdlStatus = prvPdlDbGetNext(&prvPdlPpPortDb, NULL, (void**) &portEntryPtr);
    while (pdlStatus == PDL_OK)
    {
        if (dev == portEntryPtr->key.dev)
            count ++;

        pdlStatus = prvPdlDbGetNext(&prvPdlPpPortDb, (void*) &portEntryPtr->key, (void**) &portEntryPtr);

    }

    *numOfPortsPtr = count;

    return PDL_OK;
}
/*$ END OF pdlPpDbPortAttributesGet */

/**
 * @fn  PDL_STATUS pdlPpDbFrontPanelAttributesGet ( IN UINT_32 frontPanelNumber, OUT PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC * frontPanelAttributesPtr )
 *
 * @brief   Get attributes for front panel group
 *
 * @param [in]  frontPanelNumber        front panel group id.
 * @param [out] frontPanelAttributesPtr front panel attributes.
 *
 * @return  PDL_BAD_PTR  illegal pointer supplied.
 */

PDL_STATUS pdlPpDbFrontPanelAttributesGet (
    IN   UINT_32                                 frontPanelNumber,
    OUT  PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC * frontPanelAttributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PRV_PDL_FRONT_PANEL_GROUP_ENTRY_STC   * fpEntryPtr;
    PRV_PDL_FRONT_PANEL_GROUP_KEY_STC       fpEntryKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (frontPanelAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"frontPanelAttributesPtr NULL, FRONT PANEL NUMBER: %d",frontPanelNumber);
        return PDL_BAD_PTR;
    }

    fpEntryKey.frontPanelNumber = frontPanelNumber;
    pdlStatus = prvPdlDbFind (&prvPdlFrontPanelDb, (void*) &fpEntryKey, (void**) &fpEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    memcpy (frontPanelAttributesPtr, &fpEntryPtr->data, sizeof (PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC));
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, fpEntryPtr->data.ordering);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, fpEntryPtr->data.portMaximumSpeed);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, fpEntryPtr->data.firstPortNumberShift);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, fpEntryPtr->data.numOfPortsInGroup);

    return PDL_OK;
}
/*$ END OF pdlPpDbPortAttributesGet */

/**
 * @fn  PDL_STATUS pdlPacketProcessorInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlRootId )
 *
 * @brief   Init Packet Processor module Create DB and initialize
 *
 * @param [in]  xmlRootId   Xml id.
 *
 * @return  PDL_NOT_FOUND if xml parsing wasn't successful because mandatory tag not found.
 * @return  PDL_CREATE_ERROR if number of packet processors doesn't match xml parsing.
 */

PDL_STATUS pdlPacketProcessorInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlRootId
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
    dbAttributes.listAttributes.entrySize = sizeof(PRV_PDL_FRONT_PANEL_GROUP_ENTRY_STC);
    dbAttributes.listAttributes.keySize = sizeof(PRV_PDL_FRONT_PANEL_GROUP_KEY_STC);

    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &prvPdlFrontPanelDb);
    PDL_CHECK_STATUS(pdlStatus);

    dbAttributes.listAttributes.entrySize = sizeof(PRV_PDL_PORT_ENTRY_STC);
    dbAttributes.listAttributes.keySize = sizeof(PRV_PDL_PORT_ENTRY_KEY_STC);

    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &prvPdlPpPortDb);
    PDL_CHECK_STATUS(pdlStatus);

    dbAttributes.listAttributes.entrySize = sizeof(PRV_PDL_MAC_PORT_ENTRY_STC);
    dbAttributes.listAttributes.keySize = sizeof(PRV_PDL_MAC_PORT_ENTRY_KEY_STC);

    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                            &dbAttributes,
                            &prvPdlMacLogicalPortDb);
    PDL_CHECK_STATUS(pdlStatus);

    /* before parsing XML, need to initialize Sfp module for sfp parsing */
    pdlStatus = pdlSfpInit();
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlPacketProcessorXMLParser (xmlRootId);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlPpDbPortAttributesSet ( IN UINT_32 dev, IN UINT_32 port, OUT PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC * portAttributesPtr )
 *
 * @brief   Update PP port attributes
 *
 * @param [in]  dev                 dev number.
 * @param [in]  logicalPort         logicalPort number.
 * @param [out] portAttributesPtr   port attributes.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  port's entry wasn't found.
 */

PDL_STATUS pdlPpDbPortAttributesSet (
    IN   UINT_32                                    dev,
    IN   UINT_32                                    logicalPort,
    OUT  PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC   * portAttributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PRV_PDL_PORT_ENTRY_STC                * portEntryPtr;
    PRV_PDL_PORT_ENTRY_KEY_STC              portEntryKey;
   
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (portAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"portAttributesPtr NULL, PORT NUMBER: %d, DEVICE NUMBER: %d",logicalPort ,dev);
        return PDL_BAD_PTR;
    }

    portEntryKey.dev = dev;
    portEntryKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&prvPdlPpPortDb, (void*) &portEntryKey, (void**) &portEntryPtr);
    if (pdlStatus != PDL_OK)
        return PDL_NOT_FOUND;

    memcpy (&portEntryPtr->data, portAttributesPtr, sizeof (PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC));

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlPpDbB2bAttributesSet ( IN UINT_32 b2bLinkId, OUT PDL_PP_XML_B2B_ATTRIBUTES_STC * b2bAttributesPtr )
 *
 * @brief   Update PP back-to-back link attributes
 *
 * @param [in]  b2bLinkId           b2b link number.
 * @param [out] b2bAttributesPtr    b2b link attributes.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  b2b's entry wasn't found.
 */

PDL_STATUS pdlPpDbB2bAttributesSet (
    IN   UINT_32                                 b2bLinkId,
    OUT  PDL_PP_XML_B2B_ATTRIBUTES_STC         * b2bAttributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (b2bAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"b2bAttributesPtr POINTER NULL");
        return PDL_BAD_PTR;
    }

    if (b2bLinkId >= pdlPpDb.numOfBackToBackLinksPerPp) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL b2bLinkId NUMBER");
        return PDL_NOT_FOUND;
    }

    memcpy (&pdlPpB2bDb[b2bLinkId], b2bAttributesPtr, sizeof (PDL_PP_XML_B2B_ATTRIBUTES_STC));

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlPpDbFrontPanelAttributesSet ( IN UINT_32 frontPanelNumber, OUT PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC * frontPanelAttributesPtr )
 *
 * @brief   Update attributes of front panel group
 *
 * @param [in]  frontPanelNumber        front panel group id.
 * @param [out] frontPanelAttributesPtr front panel attributes.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 */

PDL_STATUS pdlPpDbFrontPanelAttributesSet (
    IN   UINT_32                                 frontPanelNumber,
    OUT  PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC * frontPanelAttributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PRV_PDL_FRONT_PANEL_GROUP_ENTRY_STC   * fpEntryPtr;
    PRV_PDL_FRONT_PANEL_GROUP_KEY_STC       fpEntryKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (frontPanelAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"frontPanelAttributesPtr POINTER NULL, FRONT PANEL NUMBER: %d",frontPanelNumber);
        return PDL_BAD_PTR;
    }

    fpEntryKey.frontPanelNumber = frontPanelNumber;
    pdlStatus = prvPdlDbFind (&prvPdlFrontPanelDb, (void*) &fpEntryKey, (void**) &fpEntryPtr);
    if (pdlStatus != PDL_OK)
        return PDL_NOT_FOUND;

    memcpy (&fpEntryPtr->data, frontPanelAttributesPtr, sizeof (PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC));

    return PDL_OK;
}

/*$ END OF pdlPpDbFrontPanelAttributesSet */

/**
 * @fn  PDL_STATUS pdlPpPortConvertFrontPanel2DevPort ( IN UINT_32 frontPanelNumber, IN UINT_32 portNumberInGroup, OUT UINT_32 * devPtr, OUT UINT_32 * portPtr )
 *
 * @brief   Convert between front panel group &amp; port to dev &amp; port
 *
 * @param [in]  frontPanelNumber    front panel group id.
 * @param [in]  portNumberInGroup   front panel port number.
 * @param [out] devPtr              packet processor device number.
 * @param [out] portPtr             packet processor port number.
 *
 * @return  PDL_OK         convert succesfull.
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 */

PDL_STATUS pdlPpPortConvertFrontPanel2DevPort (
    IN   UINT_32                                 frontPanelNumber,
    IN   UINT_32                                 portNumberInGroup,
    OUT  UINT_32                               * devPtr,
    OUT  UINT_32                               * portPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PRV_PDL_PORT_ENTRY_STC                * portEntryPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (devPtr == NULL || portPtr == NULL) {
       PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "NULL devPtr OR portPtr, FRONT PANEL NUMBER: %d",frontPanelNumber);
       return PDL_BAD_PTR;
    }

    pdlStatus = prvPdlDbGetNext(&prvPdlPpPortDb, NULL, (void**) &portEntryPtr);
    while (pdlStatus == PDL_OK)
    {
        if (portEntryPtr->data.frontPanelNumber == frontPanelNumber && portEntryPtr->data.portNumberInGroup == portNumberInGroup) {
            *devPtr = portEntryPtr->key.dev;
            *portPtr = portEntryPtr->key.logicalPort;
            return PDL_OK;
        }
        pdlStatus = prvPdlDbGetNext(&prvPdlPpPortDb, (void*) &portEntryPtr->key, (void**) &portEntryPtr);
    }
    return PDL_NOT_FOUND;
}

/*$ END OF pdlPpPortConvertFrontPanel2DevPort */


/**
 * @fn  PDL_STATUS pdlPpDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Pp debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPpDebugSet (
    IN  BOOLEAN             state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlPpDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlPpDebugSet */



/**
 * @fn  PDL_STATUS pdlPpPortConvertDevPort2FrontPanel ( IN UINT_32 dev, IN UINT_32 port, OUT UINT_32 * frontPanelNumberPtr, OUT UINT_32 * portNumberInGroupPtr )
 *
 * @brief   Convert between dev &amp; port to front panel group &amp; port
 *
 * @param [in]      dev                     packet processor device number.
 * @param [in]      logicalPort             packet processor port number.
 * @param [out]     frontPanelNumberPtr     front panel group id.
 * @param [in,out]  portNumberInGroupPtr    If non-null, the port number in group pointer.
 *
 * @return  PDL_OK         convert succesfull.
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 *
 * ### param [out]      frontPanelPortPtr   front panel port number.
 */

PDL_STATUS pdlPpPortConvertDevPort2FrontPanel (
    IN  UINT_32                                  dev,
    IN  UINT_32                                  logicalPort,
    OUT UINT_32                                * frontPanelNumberPtr,
    OUT UINT_32                                * portNumberInGroupPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PRV_PDL_PORT_ENTRY_STC                * portEntryPtr;
    PRV_PDL_PORT_ENTRY_KEY_STC              portEntryKey;
    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (frontPanelNumberPtr == NULL || portNumberInGroupPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"frontPanelAttributesPtr POINTER NULL, PORT NUMBER: %d, DEVICE NUMBER: %d",logicalPort ,dev);
        return PDL_BAD_PTR;
    }

    portEntryKey.dev  = dev;
    portEntryKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&prvPdlPpPortDb, (void*) &portEntryKey, (void**) &portEntryPtr);
    if (pdlStatus == PDL_OK) {
        *frontPanelNumberPtr = portEntryPtr->data.frontPanelNumber;
        *portNumberInGroupPtr = portEntryPtr->data.portNumberInGroup;
    }
    return pdlStatus;
}
/*$ END OF pdlPpPortConvertDevPort2FrontPanel */

/**
 * @fn  PDL_STATUS pdlPpPortConvertMacToLogical (IN UINT_32 dev, IN UINT_32 macPort, OUT UINT_32 * logicalPortPtr );
 *
 * @brief   Convert between macPort to logicalPort.
 *
 * @param [in]      dev                     dev number.
 * @param [in]      macPort                 macPort number.
 * @param [out]     logicalPortPtr          logicalPort number ptr.
 * @param [out]     devNumPtr               dev number ptr.
 *
 * @return  PDL_OK         convert succesfull.
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 */

PDL_STATUS pdlPpPortConvertMacToLogical (
    IN  UINT_32                                  dev,
    IN  UINT_32                                  macPort,
    OUT UINT_32                                * logicalPortPtr,
    OUT UINT_32                                * devNumPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PRV_PDL_MAC_PORT_ENTRY_STC                * macPortEntryPtr;
    PRV_PDL_MAC_PORT_ENTRY_KEY_STC              macPortEntryKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if ((logicalPortPtr == NULL) || (devNumPtr == NULL)){
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"logicalPortPtr or devNumPtr POINTER NULL");
        return PDL_BAD_PTR;
    }

    macPortEntryKey.dev = dev;
    macPortEntryKey.macPort = macPort;
    pdlStatus = prvPdlDbFind (&prvPdlMacLogicalPortDb, (void*) &macPortEntryKey, (void**) &macPortEntryPtr);
    if (pdlStatus == PDL_OK) {
        *devNumPtr = macPortEntryPtr->data.dev;
        *logicalPortPtr = macPortEntryPtr->data.logicalPort;
    }
    return pdlStatus;
}
/*$ END OF pdlPpPortConvertMacToLogical */


/**
 * @fn  PDL_STATUS pdlPpPortConvertLogicalToMac (IN UINT_32 dev, IN UINT_32 logicalPort, OUT UINT_32 * macPortPtr );
 *
 * @brief   Convert between logicalPort to macPort.
 *
 * @param [in]      dev                     dev number.
 * @param [in]      logiaclPort             logicalPort number.
 * @param [out]     macPortPtr              macPort number.
 * @param [out]     devNumPtr               dev number ptr.
 *
 * @return  PDL_OK         convert succesfull.
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 */

PDL_STATUS pdlPpPortConvertLogicalToMac (
    IN  UINT_32                                  dev,
    IN  UINT_32                                  logicalPort,
    OUT UINT_32                                * devNumPtr,
    OUT UINT_32                                * macPortPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PRV_PDL_PORT_ENTRY_STC                * portEntryPtr;
    PRV_PDL_PORT_ENTRY_KEY_STC              portEntryKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
     if ((macPortPtr == NULL) || (devNumPtr == NULL)){
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__,"macPortPtr or devNumPtr POINTER NULL");
        return PDL_BAD_PTR;
    }

    portEntryKey.dev = dev;
    portEntryKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&prvPdlPpPortDb, (void*) &portEntryKey, (void**) &portEntryPtr);
    if (pdlStatus == PDL_OK) {
        *devNumPtr = dev;
        *macPortPtr = portEntryPtr->data.macPort;
    }
    return pdlStatus;
}
/*$ END OF pdlPpPortConvertLogicalToMac */

/**
 * @fn  PDL_STATUS prvPdlPacketProcessorDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPacketProcessorDestroy (
    void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i;
    PDL_STATUS                      pdlStatus;
    PRV_PDL_PORT_ENTRY_KEY_STC      portKey;
    PRV_PDL_PORT_ENTRY_STC        * portInfoPtr = NULL;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    pdlStatus = prvPdlDbGetNextKey (&prvPdlPpPortDb, NULL, (void*) &portKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus  = prvPdlDbFind (&prvPdlPpPortDb, (void*) &portKey, (void**) &portInfoPtr);
        PDL_CHECK_STATUS(pdlStatus);
        for (i = 0; i < PDL_PORT_SPEED_LAST_E; i++) {
            if (portInfoPtr->fiberPhyInitValueDb[i].dbPtr != NULL) {
                pdlStatus = prvPdlDbDestroy(&portInfoPtr->fiberPhyInitValueDb[i]);
                PDL_CHECK_STATUS(pdlStatus);
            }
            if (portInfoPtr->copperPhyInitValueDb[i].dbPtr != NULL) {
                pdlStatus = prvPdlDbDestroy(&portInfoPtr->copperPhyInitValueDb[i]);
                PDL_CHECK_STATUS(pdlStatus);
            }

        }
        if (portInfoPtr->phyPostInitValues.dbPtr != NULL) {
            pdlStatus = prvPdlDbDestroy(&portInfoPtr->phyPostInitValues);
            PDL_CHECK_STATUS(pdlStatus);
        }
        pdlStatus  = prvPdlDbGetNextKey (&prvPdlPpPortDb, (void*) &portKey, (void*) &portKey);
    }

    if (prvPdlFrontPanelDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&prvPdlFrontPanelDb);
        PDL_CHECK_STATUS(pdlStatus); 
    }
    if (prvPdlPpPortDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&prvPdlPpPortDb);
        PDL_CHECK_STATUS(pdlStatus);
    }

    /* since this module also initializes the SFP module, it'll need to destroy it */
    pdlStatus = prvPdlSfpDestroy();
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF prvPdlPacketProcessorDestroy */




