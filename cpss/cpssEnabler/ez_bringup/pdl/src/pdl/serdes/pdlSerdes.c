/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlSerdes.c   
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
 * @brief Platform driver layer - Button related API
 * 
 * @version   1 
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/serdes/pdlSerdes.h>
#include <pdl/serdes/private/prvPdlSerdes.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/**
 * @struct  PRV_PDL_SERDES_DB_STC
 *
 * @brief   Serdes database.
 */

static PDL_DB_PRV_STC               pdlSerdesDb;            /* list of PRV_PDL_LANE_ENTRY_STC*/
static PDL_DB_PRV_STC               pdlPortSerdesDb;

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)                                                                   \
    if (__pdlStatus != PDL_OK) {                                                                        \
    if (prvPdlSerdesDebugFlag) {                                                                        \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__pdlStatus,PDL_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __pdlStatus;                                                                                 \
    }  
/*macro to check the xml status and debug prints if debug flag on*/
#undef XML_CHECK_STATUS
#define XML_CHECK_STATUS(__xmlStatus)                                                                   \
    if (__xmlStatus != PDL_OK) {                                                                        \
    if (prvPdlSerdesDebugFlag) {                                                                        \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__xmlStatus,XML_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __xmlStatus;                                                                                 \
    }  


/**
 * @fn  static PDL_STATUS prvPdlSerdesTxAttributesGet ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlLaneAttrId, OUT PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC *laneTxFineTuneAttrPtr )
 *
 * @brief   Parse lane's attributes
 *
 * @param [in]  xmlLaneAttrId           Lane attributes xml id.
 * @param [out] laneTxFineTuneAttrPtr   Lane attributes data.
 *
 * @return  PDL_NOT_FOUND if xml parsing wasn't successful because mandatory tag not found.
 */

static PDL_STATUS prvPdlSerdesTxAttributesGet (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP            xmlLaneAttrId,
    OUT PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC    * laneTxFineTuneAttrPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    XML_PARSER_NODE_DESCRIPTOR_TYP  xmlId = xmlLaneAttrId;
    XML_PARSER_RET_CODE_TYP         xmlStatus;
    UINT_32                         maxSize;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (laneTxFineTuneAttrPtr == NULL) {
        return PDL_BAD_PTR;
    }

    memset(laneTxFineTuneAttrPtr, 0, sizeof(PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC));

    xmlStatus = xmlParserGetFirstChild(xmlId, &xmlId);
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        if (xmlParserIsEqualName(xmlId, PRV_PDL_SERDES_TXAMPL_TAG_NAME) == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(laneTxFineTuneAttrPtr->txAmpl);
            xmlStatus = xmlParserGetValue(xmlId, &maxSize, &laneTxFineTuneAttrPtr->txAmpl);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlId, PRV_PDL_SERDES_TXAMPLADJEN_TAG_NAME) == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(laneTxFineTuneAttrPtr->txAmplAdjEn);
            xmlStatus = xmlParserGetValue(xmlId, &maxSize, &laneTxFineTuneAttrPtr->txAmplAdjEn);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlId, PRV_PDL_SERDES_TXAMPLSHTEN_TAG_NAME) == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(laneTxFineTuneAttrPtr->txAmplShtEn);
            xmlStatus = xmlParserGetValue(xmlId, &maxSize, &laneTxFineTuneAttrPtr->txAmplShtEn);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlId, PRV_PDL_SERDES_EMPH0_TAG_NAME) == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(laneTxFineTuneAttrPtr->emph0);
            xmlStatus = xmlParserGetValue(xmlId, &maxSize, &laneTxFineTuneAttrPtr->emph0);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlId, PRV_PDL_SERDES_EMPH1_TAG_NAME) == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(laneTxFineTuneAttrPtr->emph1);
            xmlStatus = xmlParserGetValue(xmlId, &maxSize, &laneTxFineTuneAttrPtr->emph1);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlId, PRV_PDL_SERDES_EMPH_EN_NAME) == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(laneTxFineTuneAttrPtr->txEmphEn);
            xmlStatus = xmlParserGetValue(xmlId, &maxSize, &laneTxFineTuneAttrPtr->txEmphEn);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlId, PRV_PDL_SERDES_SLEW_RATE_NAME) == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(laneTxFineTuneAttrPtr->slewRate);
            xmlStatus = xmlParserGetValue(xmlId, &maxSize, &laneTxFineTuneAttrPtr->slewRate);
            XML_CHECK_STATUS(xmlStatus);
        }
        else {
            PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"INVALID txAttributeInfo");
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling(xmlId, &xmlId);
    }
    return PDL_OK;
}

/*$ END OF prvPdlSerdesTxAttributesGet */

/**
 * @fn  static PDL_STATUS prvPdlSerdesRxAttributesGet ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlLaneAttrId, OUT PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC *laneRxFineTuneAttrPtr )
 *
 * @brief   Parse lane's attributes
 *
 * @param [in]  xmlLaneAttrId           Lane attributes xml id.
 * @param [out] laneRxFineTuneAttrPtr   Lane attributes data.
 *
 * @return  PDL_NOT_FOUND if xml parsing wasn't successful because mandatory tag not found.
 */

static PDL_STATUS prvPdlSerdesRxAttributesGet (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP            xmlLaneAttrId,
    OUT PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC    * laneRxFineTuneAttrPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    XML_PARSER_NODE_DESCRIPTOR_TYP  xmlId = xmlLaneAttrId;
    XML_PARSER_RET_CODE_TYP         xmlStatus;
    UINT_32                         maxSize, dummy;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (laneRxFineTuneAttrPtr == NULL) {
        return PDL_BAD_PTR;
    }

    memset(laneRxFineTuneAttrPtr, 0, sizeof(PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC));

    xmlStatus = xmlParserGetFirstChild(xmlId, &xmlId);
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        if (xmlParserIsEqualName(xmlId, "sqlch") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(laneRxFineTuneAttrPtr->sqlch);
            xmlStatus = xmlParserGetValue(xmlId, &maxSize, &laneRxFineTuneAttrPtr->sqlch);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlId, "ffeRes") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(laneRxFineTuneAttrPtr->ffeRes);
            xmlStatus = xmlParserGetValue(xmlId, &maxSize, &laneRxFineTuneAttrPtr->ffeRes);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlId, "ffeCap") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(laneRxFineTuneAttrPtr->ffeCap);
            xmlStatus = xmlParserGetValue(xmlId, &maxSize, &laneRxFineTuneAttrPtr->ffeCap);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlId, "align90") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(laneRxFineTuneAttrPtr->align90);
            xmlStatus = xmlParserGetValue(xmlId, &maxSize, &laneRxFineTuneAttrPtr->align90);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlId, "dcGain") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(laneRxFineTuneAttrPtr->dcGain);
            xmlStatus = xmlParserGetValue(xmlId, &maxSize, &laneRxFineTuneAttrPtr->dcGain);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlId, "bandWidth") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(laneRxFineTuneAttrPtr->bandWidth);
            xmlStatus = xmlParserGetValue(xmlId, &maxSize, &laneRxFineTuneAttrPtr->bandWidth);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlId, "loopBandwidth") == XML_PARSER_RET_CODE_OK) {
            /* ignore field  - removed */
            maxSize = sizeof(UINT_32);
            xmlStatus = xmlParserGetValue(xmlId, &maxSize, &dummy);
            XML_CHECK_STATUS(xmlStatus);
        }
        else {
            PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"INVALID rxAttributeInfo");
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling(xmlId, &xmlId);
    }
    return PDL_OK;
}

/*$ END OF prvPdlSerdesRxAttributesGet */

/**
 * @fn  static PDL_STATUS prvPdlSerdesSingleLaneGet ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlLaneId, OUT PRV_PDL_LANE_ENTRY_STC *lanePtr )
 *
 * @brief   Parse single serdes's lane data
 *
 * @param [in]  xmlLaneId   Lane xml id.
 * @param [out] lanePtr     Lane data.
 *
 * @return  PDL_NOT_FOUND if xml parsing wasn't successful because mandatory tag not found.
 * @return  PDL_CREATE_ERROR if number of lanes doesn't match xml parsing.
 */

static PDL_STATUS prvPdlSerdesParseSingleLane (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlLaneId,
    IN  UINT_32                             ppNumber
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                                    pdlTagStr[PDL_XML_MAX_TAG_LEN];
    UINT_32                                                 maxSize, value;
    PDL_STATUS                                              pdlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP                          xmlChildId, xmlLaneAttributes, xmlLaneInfo, xmlLaneAttribute;
    XML_PARSER_RET_CODE_TYP                                 xmlStatus, xmlStatus2, xmlStatus3;
    PDL_DB_PRV_ATTRIBUTES_STC                               dbAttributes;
    PRV_PDL_LANE_ENTRY_STC                                  laneEntry;
    PRV_PDL_LANE_ENTRY_STC                                * laneEntryPtr;
    PRV_PDL_LANE_ENTRY_KEY_STC                              laneKey;
    PRV_PDL_LANE_INFO_ENTRY_STC                             laneInfoEntry;
    PRV_PDL_LANE_INFO_ENTRY_STC                           * laneInfoEntryPtr;
    PRV_PDL_LANE_INFO_ENTRY_KEY_STC                         infoKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    memset (&laneEntry, 0, sizeof(laneEntry));
    memset (&laneKey, 0, sizeof(laneKey));
    memset (&laneInfoEntry, 0, sizeof(laneInfoEntry));   
    laneKey.ppNumber = ppNumber;

    xmlStatus = xmlParserGetFirstChild(xmlLaneId, &xmlChildId);
    if (xmlStatus != XML_PARSER_RET_CODE_OK)
        return PDL_NOT_FOUND;

    xmlStatus = xmlParserIsEqualName(xmlChildId, "lane-number");
    if (xmlStatus != XML_PARSER_RET_CODE_OK)
        return PDL_NOT_FOUND;

    maxSize = sizeof(laneKey.laneId);
    xmlStatus = xmlParserGetValue(xmlChildId, &maxSize, &laneKey.laneId);
    if (xmlStatus != XML_PARSER_RET_CODE_OK)
        return PDL_NOT_FOUND;

    xmlStatus = xmlParserGetNextSibling(xmlChildId, &xmlChildId);
    if (xmlStatus != XML_PARSER_RET_CODE_OK)
        return PDL_NOT_FOUND;

    xmlStatus = xmlParserIsEqualName(xmlChildId, "lane-attributes-group");
    if (xmlStatus != XML_PARSER_RET_CODE_OK)
        return PDL_NOT_FOUND;

    dbAttributes.listAttributes.entrySize = sizeof(PRV_PDL_LANE_INFO_ENTRY_STC);
    dbAttributes.listAttributes.keySize = sizeof(PRV_PDL_LANE_INFO_ENTRY_KEY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &laneEntry.laneInfoDb);
    xmlStatus = xmlParserGetFirstChild(xmlChildId, &xmlLaneAttributes);
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);
        if (xmlParserIsEqualName(xmlLaneAttributes, "tx-swap") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(laneEntry.txSwap);
            xmlStatus = xmlParserGetValue(xmlLaneAttributes, &maxSize, &laneEntry.txSwap);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName(xmlLaneAttributes, "rx-swap") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(laneEntry.rxSwap);
            xmlStatus = xmlParserGetValue(xmlLaneAttributes, &maxSize, &laneEntry.rxSwap);
            XML_CHECK_STATUS(xmlStatus);
        }
        else {
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling(xmlLaneAttributes, &xmlLaneAttributes);
    }

    /*check if lane-attributes exists */
    xmlStatus = xmlParserGetNextSibling(xmlChildId, &xmlLaneAttributes);
    if (xmlStatus == XML_PARSER_RET_CODE_OK && xmlParserIsEqualName(xmlLaneAttributes, "lane-attributes-exist") == XML_PARSER_RET_CODE_OK) {     
        /* skip over lane-attributes-exist tag */
        xmlStatus = xmlParserGetNextSibling(xmlLaneAttributes, &xmlLaneAttributes);
    }

    if (xmlStatus == XML_PARSER_RET_CODE_OK) {
        if (xmlParserIsEqualName(xmlLaneAttributes, "lane-attributes") == XML_PARSER_RET_CODE_OK) {     
            /* get child <info> */
            xmlStatus2 = xmlParserGetFirstChild(xmlLaneAttributes, &xmlLaneAttributes);
            while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                /* get to first tag in <info> */
                xmlStatus2 = xmlParserGetFirstChild(xmlLaneAttributes, &xmlLaneInfo);
                while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                    XML_CHECK_STATUS(xmlStatus2);
                    if (xmlParserIsEqualName(xmlLaneInfo, "lane-attribute") == XML_PARSER_RET_CODE_OK) {
                        xmlStatus3 = xmlParserGetFirstChild(xmlLaneInfo, &xmlLaneAttribute);
                        while (xmlStatus3 != XML_PARSER_RET_CODE_NOT_FOUND) {
                            if (xmlParserIsEqualName(xmlLaneAttribute, "tx-fine-tune") == XML_PARSER_RET_CODE_OK) {
                                pdlStatus = prvPdlSerdesTxAttributesGet(xmlLaneAttribute, &laneInfoEntry.fineTuneTxAttributes);
                                PDL_CHECK_STATUS(pdlStatus);
                            }
                            else if (xmlParserIsEqualName(xmlLaneAttribute, "rx-fine-tune") == XML_PARSER_RET_CODE_OK) {
                                pdlStatus = prvPdlSerdesRxAttributesGet(xmlLaneAttribute, &laneInfoEntry.fineTuneRxAttributes);
                                PDL_CHECK_STATUS(pdlStatus);
                            }
                            else if (xmlParserIsEqualName(xmlLaneAttribute, "is-tx-fine-tune-supported") == XML_PARSER_RET_CODE_OK) {
                                maxSize = sizeof(laneInfoEntry.fineTuneTxExist);
                                xmlStatus = xmlParserGetValue(xmlLaneAttribute, &maxSize, &laneInfoEntry.fineTuneTxExist);
                                XML_CHECK_STATUS(xmlStatus);
                            }
                            else if (xmlParserIsEqualName(xmlLaneAttribute, "is-rx-fine-tune-supported") == XML_PARSER_RET_CODE_OK) {
                                maxSize = sizeof(laneInfoEntry.fineTuneRxExist);
                                xmlStatus = xmlParserGetValue(xmlLaneAttribute, &maxSize, &laneInfoEntry.fineTuneRxExist);
                                XML_CHECK_STATUS(xmlStatus);
                            }
                            else {
                                PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlLaneAttributeInfo");
                                return PDL_XML_PARSE_ERROR;
                            }
                            xmlStatus3 = xmlParserGetNextSibling(xmlLaneAttribute, &xmlLaneAttribute);
                        }
                    }
                    else if (xmlParserIsEqualName(xmlLaneInfo, "interface-mode") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(pdlTagStr);
                        xmlStatus = xmlParserGetValue (xmlLaneInfo, &maxSize, &pdlTagStr[0]);
                        XML_CHECK_STATUS(xmlStatus);
                        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, pdlTagStr, &value);
                        PDL_CHECK_STATUS(pdlStatus);
                        infoKey.interfaceMode = (PDL_INTERFACE_MODE_ENT)value;

                    }
                    else if (xmlParserIsEqualName(xmlLaneInfo, "connector-type") == XML_PARSER_RET_CODE_OK){
                        maxSize = sizeof(pdlTagStr);
                        xmlStatus = xmlParserGetValue (xmlLaneInfo, &maxSize, &pdlTagStr[0]);
                        XML_CHECK_STATUS(xmlStatus);
                        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_CONNECTOR_TYPE_TYPE_E, pdlTagStr, &value);
                        PDL_CHECK_STATUS(pdlStatus);
                        infoKey.connectorType = (PDL_CONNECTOR_TYPE_ENT)value;
                    }
                    else {
                        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlLaneInfo NAME");
                        return PDL_XML_PARSE_ERROR;
                    }
                    xmlStatus2 = xmlParserGetNextSibling(xmlLaneInfo, &xmlLaneInfo);
                }
                laneInfoEntry.key = infoKey;
                pdlStatus = prvPdlDbAdd (&laneEntry.laneInfoDb, (void*) &infoKey, (void*) &laneInfoEntry, (void**) &laneInfoEntryPtr); 
                PDL_CHECK_STATUS(pdlStatus);
                xmlStatus2 = xmlParserGetNextSibling(xmlLaneAttributes, &xmlLaneAttributes);
            }
        }
        else if (xmlParserIsEqualName(xmlLaneAttributes, "lane-attributes-info-list") == XML_PARSER_RET_CODE_OK) {
            while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
                XML_CHECK_STATUS(xmlStatus);
                xmlStatus2 = xmlParserGetFirstChild(xmlLaneAttributes, &xmlLaneInfo);
                while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                    XML_CHECK_STATUS(xmlStatus2);
                    if (xmlParserIsEqualName(xmlLaneInfo, "tx-fine-tune") == XML_PARSER_RET_CODE_OK) {
                        pdlStatus = prvPdlSerdesTxAttributesGet(xmlLaneInfo, &laneInfoEntry.fineTuneTxAttributes);
                        PDL_CHECK_STATUS(pdlStatus);
                    }
                    else if (xmlParserIsEqualName(xmlLaneInfo, "rx-fine-tune") == XML_PARSER_RET_CODE_OK) {
                        pdlStatus = prvPdlSerdesRxAttributesGet(xmlLaneInfo, &laneInfoEntry.fineTuneRxAttributes);
                        PDL_CHECK_STATUS(pdlStatus);
                    }
                    else if (xmlParserIsEqualName(xmlLaneInfo, "is-tx-fine-tune-supported") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(laneInfoEntry.fineTuneTxExist);
                        xmlStatus = xmlParserGetValue(xmlLaneInfo, &maxSize, &laneInfoEntry.fineTuneTxExist);
                        XML_CHECK_STATUS(xmlStatus);
                    }
                    else if (xmlParserIsEqualName(xmlLaneInfo, "is-rx-fine-tune-supported") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(laneInfoEntry.fineTuneRxExist);
                        xmlStatus = xmlParserGetValue(xmlLaneInfo, &maxSize, &laneInfoEntry.fineTuneRxExist);
                        XML_CHECK_STATUS(xmlStatus);
                    }
                    else if (xmlParserIsEqualName(xmlLaneInfo, "interface-mode") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(pdlTagStr);
                        xmlStatus = xmlParserGetValue (xmlLaneInfo, &maxSize, &pdlTagStr[0]);
                        XML_CHECK_STATUS(xmlStatus);
                        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, pdlTagStr, &value);
                        PDL_CHECK_STATUS(pdlStatus);
                        infoKey.interfaceMode = (PDL_INTERFACE_MODE_ENT)value;
                    }
                    else if (xmlParserIsEqualName(xmlLaneInfo, "connector-type") == XML_PARSER_RET_CODE_OK){
                        maxSize = sizeof(pdlTagStr);
                        xmlStatus = xmlParserGetValue (xmlLaneInfo, &maxSize, &pdlTagStr[0]);
                        XML_CHECK_STATUS(xmlStatus);
                        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_CONNECTOR_TYPE_TYPE_E, pdlTagStr, &value);
                        PDL_CHECK_STATUS(pdlStatus);
                        infoKey.connectorType = (PDL_CONNECTOR_TYPE_ENT)value;
                    }
                    else {
                        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlLaneAttributeInfo");
                        return PDL_XML_PARSE_ERROR;
                    }
                    xmlStatus2 = xmlParserGetNextSibling(xmlLaneInfo, &xmlLaneInfo);
                }
                laneInfoEntry.key = infoKey;
                pdlStatus = prvPdlDbAdd (&laneEntry.laneInfoDb, (void*) &infoKey, (void*) &laneInfoEntry, (void**) &laneInfoEntryPtr); 
                PDL_CHECK_STATUS(pdlStatus);
                xmlStatus = xmlParserGetNextSibling(xmlLaneAttributes, &xmlLaneAttributes);
            }
        }

        else {
            PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlLaneAttributes NAME");
            return PDL_XML_PARSE_ERROR;
        }
    }

    /* add lane entry to DB */
    pdlStatus = prvPdlDbAdd (&pdlSerdesDb, (void*) &laneKey, (void*) &laneEntry, (void**) &laneEntryPtr);  
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF prvPdlSerdesSingleLaneGet */

/**
 * @fn  static PDL_STATUS prvPdlSerdesSinglePpGet ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlPpId, OUT PRV_PDL_SERDES_ATTRIBUTES_STC *ppPtr )
 *
 * @brief   Parse single packet processor's serdes data
 *
 * @param [in]  xmlPpId Serdes xml id.
 * @param [out] ppPtr   Serdes data.
 *
 * @return  PDL_NOT_FOUND if xml parsing wasn't successful because mandatory tag not found.
 * @return  PDL_CREATE_ERROR if number of lanes doesn't match xml parsing.
 */

static PDL_STATUS prvPdlSerdesParseSinglePp (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP       xmlPpId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    UINT_32                         ppNumber = 0, count = 0, maxSize;
    PDL_STATUS                      pdlStatus;
    XML_PARSER_RET_CODE_TYP         xmlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP  xmlChildId;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    xmlStatus = xmlParserGetFirstChild(xmlPpId, &xmlChildId);
    if (xmlStatus != XML_PARSER_RET_CODE_OK)
        return PDL_NOT_FOUND;

    xmlStatus = xmlParserIsEqualName(xmlChildId, "pp-device-number");
    if (xmlStatus != XML_PARSER_RET_CODE_OK)
        return PDL_NOT_FOUND;

    maxSize = sizeof(ppNumber);
    xmlStatus = xmlParserGetValue(xmlChildId, &maxSize, &ppNumber);
    if (xmlStatus != XML_PARSER_RET_CODE_OK)
        return PDL_NOT_FOUND;

    pdlStatus = xmlParserGetTagCount(xmlPpId, "lane-list", &count);
    PDL_CHECK_STATUS(pdlStatus);

    if (count == 0) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"tag count == 0");
        return PDL_NOT_FOUND;
    }

    xmlStatus = xmlParserGetNextSibling(xmlChildId, &xmlChildId);

    while (xmlStatus == XML_PARSER_RET_CODE_OK)
    {
        XML_CHECK_STATUS(xmlStatus);
        /* validate lane string */
        xmlStatus = xmlParserIsEqualName(xmlChildId, "lane-list");
        if (xmlStatus != XML_PARSER_RET_CODE_OK)
            break;
        pdlStatus = prvPdlSerdesParseSingleLane(xmlChildId, ppNumber);
        PDL_CHECK_STATUS(pdlStatus);
        xmlStatus = xmlParserGetNextSibling(xmlChildId, &xmlChildId);
    }
    return PDL_OK;
}

/*$ END OF prvPdlSerdesSinglePpGet */

/**
 * @fn  PDL_STATUS prvPdlSerdesXMLParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Prv pdl serdes XML parser
 *
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSerdesXMLParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    PDL_STATUS                      pdlStatus;
    XML_PARSER_RET_CODE_TYP         xmlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP  xmlPpId;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    xmlStatus = xmlParserFindByName(xmlId, "SERDES", &xmlPpId);
    /* section is not mandatory */
    if (xmlStatus != XML_PARSER_RET_CODE_OK)
        return PDL_OK;

    XML_CHECK_STATUS(xmlStatus);
    xmlStatus = xmlParserGetFirstChild(xmlPpId, &xmlPpId);
    while (xmlStatus == XML_PARSER_RET_CODE_OK)
    {
        xmlStatus = xmlParserIsEqualName(xmlPpId, "Packet-Processor-list");
        if (xmlStatus != XML_PARSER_RET_CODE_OK)
            return PDL_XML_PARSE_ERROR;
        pdlStatus = prvPdlSerdesParseSinglePp(xmlPpId);
        PDL_CHECK_STATUS(pdlStatus);
        xmlStatus = xmlParserGetNextSibling(xmlPpId, &xmlPpId);
    }
    return PDL_OK;
}

/*$ END OF prvPdlSerdesXMLParser */

/**
 * @fn  PDL_STATUS prvPdlSerdesPortXmlParserLegacyV1 ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, IN UINT_32 dev, IN UINT_32 port )
 *
 * @brief   Prv pdl serdes XML parser for legacy v1 information
 *
 * @param   xmlId   Identifier for the XML.
 * @param   dev     The dev.
 * @param   port    The port.
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlSerdesPortXmlParserLegacyV1 (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    IN  UINT_32                                 dev,
    IN  UINT_32                                 port
)
{
/****************************************************************************/
/*L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/****************************************************************************/
   BOOLEAN                                     isExists;
   UINT_32                                     maxSize, serdesCount = 0;
   PDL_STATUS                                  pdlStatus;
   PRV_PDL_PORT_KEY_STC                        portKey;
   PDL_PORT_LANE_DATA_STC                      laneData, *laneDataPtr;
   XML_PARSER_RET_CODE_TYP                     xmlStatus, xmlStatus2;
   PDL_DB_PRV_ATTRIBUTES_STC                   dbAttributes;
   PDL_PORT_LANE_DATA_KEY_STC                  laneKey;
   PRV_PDL_PORT_LANE_ENTRY_STC                 portEntry, *portPtr;
   XML_PARSER_NODE_DESCRIPTOR_TYP              xmlSerdesChildId, xmlSerdesInfoId;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    maxSize = sizeof(isExists);
    if ((xmlParserIsEqualName (xmlId, "serdes-exist") != XML_PARSER_RET_CODE_OK) ||
        (xmlParserGetIntValue(xmlId, &maxSize, &isExists) != XML_PARSER_RET_CODE_OK) ||
        isExists == FALSE)
    {
        return PDL_NOT_FOUND;
    }

    xmlStatus = xmlParserGetNextSibling(xmlId, &xmlId);

    if ((xmlStatus != XML_PARSER_RET_CODE_OK) ||
        (xmlParserIsEqualName (xmlId, "serdes-group") != XML_PARSER_RET_CODE_OK))
    {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlPpId POINTER OR NAME DON'T MATCH TO: serdes-group, PORT NUMBER: %d, DEVICE NUMBER: %d",port,dev);
        return PDL_ERROR;
    }

    /* serdes group is optional under <port> */
    /* xmlId not pointing to gbic */
    if (xmlParserIsEqualName (xmlId, "serdes-group") != XML_PARSER_RET_CODE_OK) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlPpId POINTER OR NAME DON'T MATCH TO: serdes-group, PORT NUMBER: %d, DEVICE NUMBER: %d",port,dev);
        return PDL_ERROR;
    }

    portKey.dev = dev;
    portKey.port = port;
    dbAttributes.listAttributes.keySize        = sizeof(PDL_PORT_LANE_DATA_KEY_STC);
    dbAttributes.listAttributes.entrySize      = sizeof(PDL_PORT_LANE_DATA_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E, &dbAttributes, &portEntry.portSerdesInfoDb);
    PDL_CHECK_STATUS(pdlStatus);
    xmlStatus = xmlParserGetFirstChild (xmlId, &xmlSerdesChildId);
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);
        if (xmlParserIsEqualName (xmlSerdesChildId, "port-serdes-list") == XML_PARSER_RET_CODE_OK) {
            xmlStatus2 = xmlParserGetFirstChild (xmlSerdesChildId, &xmlSerdesInfoId);
            while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                XML_CHECK_STATUS(xmlStatus2);
                if (xmlParserIsEqualName (xmlSerdesInfoId, "lane-number") == XML_PARSER_RET_CODE_OK) {
                    maxSize = sizeof(laneData.absSerdesNum);
                    xmlStatus = xmlParserGetValue(xmlSerdesInfoId, &maxSize, &laneData.absSerdesNum);
                    XML_CHECK_STATUS(xmlStatus);
                }
                else if (xmlParserIsEqualName (xmlSerdesInfoId, "port-lane-number") == XML_PARSER_RET_CODE_OK) {
                    maxSize = sizeof(laneData.relSerdesNum);
                    xmlStatus = xmlParserGetValue(xmlSerdesInfoId, &maxSize, &laneData.relSerdesNum);
                    XML_CHECK_STATUS(xmlStatus);
                }
                else {
                    PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlSerdesInfoId POINTER OR NAME DON'T MATCH, PORT NUMBER: %d, DEVICE NUMBER: %d",port,dev);
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus2 = xmlParserGetNextSibling(xmlSerdesInfoId, &xmlSerdesInfoId);
            }
        }
        else {
            return PDL_XML_PARSE_ERROR;
        }
        laneKey.relSerdesNum = laneData.relSerdesNum;
        pdlStatus = prvPdlDbAdd (&portEntry.portSerdesInfoDb, (void*) &laneKey, (void*) &laneData, (void**) &laneDataPtr); 
        PDL_CHECK_STATUS(pdlStatus);
        serdesCount++;
        xmlStatus = xmlParserGetNextSibling(xmlSerdesChildId, &xmlSerdesChildId);
    }
    portEntry.numOfSerdes = serdesCount;
    pdlStatus = prvPdlDbAdd (&pdlPortSerdesDb, (void*) &portKey, (void*) &portEntry, (void**) &portPtr); 
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}
/*$ END OF prvPdlSerdesPortXmlParserLegacyV1 */

/**
 * @fn  PDL_STATUS prvPdlSerdesPortXmlParserCurrent ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, IN UINT_32 dev, IN UINT_32 port )
 *
 * @brief   Prv pdl serdes XML parser for current serdes information
 *
 * @param   xmlId   Identifier for the XML.
 * @param   dev     The dev.
 * @param   port    The port.
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlSerdesPortXmlParserCurrent (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    IN  UINT_32                                 dev,
    IN  UINT_32                                 port
)
{
/****************************************************************************/
/*L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/****************************************************************************/
   UINT_32                                     maxSize, serdesCount = 0;
   PDL_STATUS                                  pdlStatus;
   PRV_PDL_PORT_KEY_STC                        portKey;
   PDL_PORT_LANE_DATA_STC                      laneData, *laneDataPtr;
   XML_PARSER_RET_CODE_TYP                     xmlStatus, xmlStatus2;
   PDL_DB_PRV_ATTRIBUTES_STC                   dbAttributes;
   PDL_PORT_LANE_DATA_KEY_STC                  laneKey;
   PRV_PDL_PORT_LANE_ENTRY_STC                 portEntry, *portPtr;
   XML_PARSER_NODE_DESCRIPTOR_TYP              xmlSerdesInfoId;
   char                                        listNameStr[30];
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    maxSize = sizeof(listNameStr);
    xmlStatus = xmlParserGetName (xmlId, &maxSize, listNameStr);
    if (xmlStatus != XML_PARSER_RET_CODE_OK)
    {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL serdes list name size. Expected (less than %d) Received (%d)", sizeof(listNameStr), maxSize);
        return PDL_BAD_VALUE;
    }

    portKey.dev = dev;
    portKey.port = port;
    dbAttributes.listAttributes.keySize        = sizeof(PDL_PORT_LANE_DATA_KEY_STC);
    dbAttributes.listAttributes.entrySize      = sizeof(PDL_PORT_LANE_DATA_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E, &dbAttributes, &portEntry.portSerdesInfoDb);
    PDL_CHECK_STATUS(pdlStatus);
    while (xmlStatus == XML_PARSER_RET_CODE_OK && (xmlParserIsEqualName (xmlId, listNameStr) == XML_PARSER_RET_CODE_OK))
    {
        xmlStatus2 = xmlParserGetFirstChild (xmlId, &xmlSerdesInfoId);
        while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
            XML_CHECK_STATUS(xmlStatus2);
            if (xmlParserIsEqualName (xmlSerdesInfoId, "lane-number") == XML_PARSER_RET_CODE_OK) {
                maxSize = sizeof(laneData.absSerdesNum);
                xmlStatus = xmlParserGetValue(xmlSerdesInfoId, &maxSize, &laneData.absSerdesNum);
                XML_CHECK_STATUS(xmlStatus);
            }
            else if (xmlParserIsEqualName (xmlSerdesInfoId, "port-lane-number") == XML_PARSER_RET_CODE_OK) {
                maxSize = sizeof(laneData.relSerdesNum);
                xmlStatus = xmlParserGetValue(xmlSerdesInfoId, &maxSize, &laneData.relSerdesNum);
                XML_CHECK_STATUS(xmlStatus);
            }
            else {
                PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlSerdesInfoId POINTER OR NAME DON'T MATCH, PORT NUMBER: %d, DEVICE NUMBER: %d",port,dev);
                return PDL_XML_PARSE_ERROR;
            }
            xmlStatus2 = xmlParserGetNextSibling(xmlSerdesInfoId, &xmlSerdesInfoId);
        }

        laneKey.relSerdesNum = laneData.relSerdesNum;
        pdlStatus = prvPdlDbAdd (&portEntry.portSerdesInfoDb, (void*) &laneKey, (void*) &laneData, (void**) &laneDataPtr); 
        PDL_CHECK_STATUS(pdlStatus);
        serdesCount++;        
        xmlStatus = xmlParserGetNextSibling(xmlId, &xmlId);
    }
    portEntry.numOfSerdes = serdesCount;
    pdlStatus = prvPdlDbAdd (&pdlPortSerdesDb, (void*) &portKey, (void*) &portEntry, (void**) &portPtr); 
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}
/*$ END OF prvPdlSerdesPortXmlParserCurrent */

/**
 * @fn  PDL_STATUS prvPdlSerdesPortXmlParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, IN UINT_32 dev, IN UINT_32 port )
 *
 * @brief   Prv pdl serdes XML parser
 *
 * @param   xmlId   Identifier for the XML.
 * @param   dev     The dev.
 * @param   port    The port.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlSerdesPortXmlParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    IN  UINT_32                                 dev,
    IN  UINT_32                                 port
)
{
/****************************************************************************/
/*L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/****************************************************************************/
    PDL_STATUS pdlStatus;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    /* check if serdes information is current or legacy format */
    if (xmlParserIsEqualName (xmlId, "serdes-exist") == XML_PARSER_RET_CODE_OK) {
        pdlStatus = prvPdlSerdesPortXmlParserLegacyV1 (xmlId, dev, port);
    }
    else {
        pdlStatus = prvPdlSerdesPortXmlParserCurrent (xmlId, dev, port);
    }

    return pdlStatus;
}

/**
 * @fn  PDL_STATUS pdlSerdesDbPolarityAttrGet( IN UINT_32 ppId, IN UINT_32 laneId, OUT PDL_LANE_POLARITY_ATTRIBUTES_STC * polarityAttributesPtr );
 *
 * @brief   Get polarity attributes of serdes
 *
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                    devId
 * @param [in]  laneId                  serdes's absolute lane identifier.
 * @param [out] polarityAttributesPtr   lane polarity attributes.
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given laneId
 * @return  PDL_BAD_OK                  lane polarity attributes found and returned
 */

PDL_STATUS pdlSerdesDbPolarityAttrGet(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    OUT PDL_LANE_POLARITY_ATTRIBUTES_STC                  * polarityAttributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PRV_PDL_LANE_ENTRY_STC                                * laneEntryPtr;
    PRV_PDL_LANE_ENTRY_KEY_STC                              laneKey;
/****************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

    if (polarityAttributesPtr == NULL)
    {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"polarityAttributesPtr NULL, LANE ID: %d, DEVICE ID: %d",laneId,ppId);
        return PDL_BAD_PTR;
    }
    laneKey.ppNumber = ppId;
    laneKey.laneId = laneId;
    pdlStatus = prvPdlDbFind(&pdlSerdesDb, &laneKey, (void **)&laneEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);
    polarityAttributesPtr->rxSwap = laneEntryPtr->rxSwap;
    polarityAttributesPtr->txSwap = laneEntryPtr->txSwap;
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(BOOLEAN, laneEntryPtr->txSwap);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(BOOLEAN, laneEntryPtr->rxSwap);
    return PDL_OK;
}
/*$ END OF pdlSerdesDbPolarityAttrGet */

/**
 * @fn  PDL_STATUS pdlSerdesDbPolarityAttrSet( IN UINT_32 ppId, IN UINT_32 laneId, IN PDL_LANE_POLARITY_ATTRIBUTES_STC * polarityAttributesPtr );
 *
 * @brief   Set polarity attributes of serdes
 *
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                    devId
 * @param [in]  laneId                  serdes's absolute lane identifier.
 * @param [in]  polarityAttributesPtr   lane polarity attributes.
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given laneId
 * @return  PDL_BAD_OK                  lane polarity attributes found and returned
 */

PDL_STATUS pdlSerdesDbPolarityAttrSet(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    IN  PDL_LANE_POLARITY_ATTRIBUTES_STC                  * polarityAttributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PRV_PDL_LANE_ENTRY_STC                                * laneEntryPtr;
    PRV_PDL_LANE_ENTRY_KEY_STC                              laneKey;
/****************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

    if (polarityAttributesPtr == NULL)
    {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"polarityAttributesPtr NULL, LANE ID: %d, DEVICE ID: %d",laneId,ppId);
        return PDL_BAD_PTR;
    }
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(BOOLEAN, polarityAttributesPtr->txSwap);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(BOOLEAN, polarityAttributesPtr->rxSwap);

    laneKey.ppNumber = ppId;
    laneKey.laneId = laneId;
    pdlStatus = prvPdlDbFind(&pdlSerdesDb, &laneKey, (void **)&laneEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);
    laneEntryPtr->rxSwap = polarityAttributesPtr->rxSwap;
    laneEntryPtr->txSwap = polarityAttributesPtr->txSwap;
    return PDL_OK;
}
/*$ END OF pdlSerdesDbPolarityAttrSet */

/**
 * @fn  PDL_STATUS pdlSerdesDbFineTuneAttrSet( IN UINT_32 ppId, IN UINT_32 laneId, IN PDL_INTERFACE_MODE_ENT interfaceMode IN PDL_CONNECTOR_TYPE_ENT connectorType, IN PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC * fineTuneTxAttributesPtr , IN PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC * fineTuneRxAttributesPtr );
 *
 * @brief   Set attributes of serdes for specified interfaceMode & connectorType
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                                devId
 * @param [in]  laneId                              serdes's absolute lane identifier.
 * @param [in]  interfaceMode                       interface mode.
 * @param [in]  connectorType                       connector Type
 * @param [out] fineTuneTxAttributesPtr             lane fine tune tx params or NULL.
 * @param [out] fineTuneRxAttributesPtr             lane fine tune rx params or NULL.
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given interfaceMode & connectorType
 * @return  PDL_OK                      lane attributes found and returned
 */
PDL_STATUS pdlSerdesDbFineTuneAttrSet(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    IN  PDL_INTERFACE_MODE_ENT                              interfaceMode,
    IN  PDL_CONNECTOR_TYPE_ENT                              connectorType,
    IN  PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC              * fineTuneTxAttributesPtr,
    IN  PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC              * fineTuneRxAttributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PRV_PDL_LANE_ENTRY_STC                                * laneEntryPtr;
    PRV_PDL_LANE_ENTRY_KEY_STC                              laneKey;
    PRV_PDL_LANE_INFO_ENTRY_STC                           * laneInfoEntryPtr;
    PRV_PDL_LANE_INFO_ENTRY_KEY_STC                         laneInfoKey;
/****************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (fineTuneTxAttributesPtr == NULL && fineTuneRxAttributesPtr == NULL)
    {
         PDL_LIB_DEBUG_MAC (prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"fineTuneTxAttributesPtr NULL, LANE ID: %d, DEVICE ID: %d",laneId,ppId);
         return PDL_BAD_PTR;
    }
    laneKey.ppNumber = ppId;
    laneKey.laneId = laneId;
    pdlStatus = prvPdlDbFind(&pdlSerdesDb, &laneKey, (void **)&laneEntryPtr);
    if (pdlStatus == PDL_NOT_FOUND)
    {
        PDL_LIB_DEBUG_MAC (prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"LANE ID: %d, DEVICE ID: %d NOT FOUND",laneId,ppId);
        return PDL_BAD_PARAM;
    }
    PDL_CHECK_STATUS(pdlStatus);

    /* now find tune params relevant to connector type & interface mode */
    laneInfoKey.connectorType = connectorType;
    laneInfoKey.interfaceMode = interfaceMode;
    pdlStatus = prvPdlDbFind(&laneEntryPtr->laneInfoDb, &laneInfoKey, (void **)&laneInfoEntryPtr);
    if (pdlStatus == PDL_NOT_FOUND)
    {
        PDL_LIB_DEBUG_MAC (prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"LANE ID: %d, DEVICE ID: %d INTERFACE MODE: %d CONNECTOR: %d NOT FOUND",laneId,ppId, interfaceMode, connectorType);
        return PDL_BAD_PARAM;
    }
    PDL_CHECK_STATUS(pdlStatus);

    if (laneInfoEntryPtr->fineTuneTxExist == FALSE && fineTuneTxAttributesPtr) {
        PDL_LIB_DEBUG_MAC (prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"TX Attributes doesn't exist and fineTuneTxAttributesPtr is not NULL, LANE ID: %d, DEVICE ID: %d",laneId,ppId);
        return PDL_BAD_PTR;
    }

    if (laneInfoEntryPtr->fineTuneRxExist == FALSE && fineTuneRxAttributesPtr) {
        PDL_LIB_DEBUG_MAC (prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"RX Attributes doesn't exist and fineTuneRxAttributesPtr is not NULL, LANE ID: %d, DEVICE ID: %d",laneId,ppId);
        return PDL_BAD_PTR;
    }

    if (fineTuneTxAttributesPtr) {
        memcpy (&laneInfoEntryPtr->fineTuneTxAttributes, fineTuneTxAttributesPtr, sizeof (PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC));
    }
    if (fineTuneRxAttributesPtr) {
        memcpy (&laneInfoEntryPtr->fineTuneRxAttributes, fineTuneRxAttributesPtr, sizeof (PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC));
    }
    return PDL_OK;
}
/*$ END OF pdlSerdesDbFineTuneAttrSet */

/**
 * @fn  PDL_STATUS pdlSerdesDbFineTuneAttrGet( IN UINT_32 ppId, IN UINT_32 laneId, IN PDL_INTERFACE_MODE_ENT interfaceMode IN PDL_CONNECTOR_TYPE_ENT connectorType, OUT PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC * fineTuneTxAttributesPtr );
 *
 * @brief   Get attributes of serdes for specified interfaceMode & connectorType
 *
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                                devId
 * @param [in]  laneId                              serdes's absolute lane identifier.
 * @param [in]  interfaceMode                       interface mode.
 * @param [in]  connectorType                       connector Type
 * @param [out] fineTuneTxAttributesExistsPtr       lane fine tune tx params exist.
 * @param [out] fineTuneTxAttributesPtr             lane fine tune tx params.
 * @param [out] fineTuneRxAttributesExistsPtr       lane fine tune rx params exist.
 * @param [out] fineTuneRxAttributesPtr             lane fine tune rx params.
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given interfaceMode & connectorType
 * @return  PDL_OK                      lane attributes found and returned
 */
PDL_STATUS pdlSerdesDbFineTuneAttrGet(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    IN  PDL_INTERFACE_MODE_ENT                              interfaceMode,
    IN  PDL_CONNECTOR_TYPE_ENT                              connectorType,
    OUT BOOLEAN                                           * fineTuneTxAttributesExistsPtr,
    OUT PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC              * fineTuneTxAttributesPtr,
    OUT BOOLEAN                                           * fineTuneRxAttributesExistsPtr,
    OUT PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC              * fineTuneRxAttributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PRV_PDL_LANE_ENTRY_STC                                * laneEntryPtr;
    PRV_PDL_LANE_ENTRY_KEY_STC                              laneKey;
    PRV_PDL_LANE_INFO_ENTRY_STC                           * laneInfoEntryPtr;
    PRV_PDL_LANE_INFO_ENTRY_KEY_STC                         laneInfoKey;
/****************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

    if (fineTuneTxAttributesExistsPtr == NULL)
    {
         PDL_LIB_DEBUG_MAC (prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"fineTuneTxAttributesExistsPtr NULL, LANE ID: %d, DEVICE ID: %d",laneId,ppId);
         return PDL_BAD_PTR;
    }
    if (fineTuneTxAttributesPtr == NULL)
    {
        PDL_LIB_DEBUG_MAC (prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"fineTuneTxAttributesPtr NULL, LANE ID: %d, DEVICE ID: %d",laneId,ppId);
        return PDL_BAD_PTR;
    }
    if (fineTuneRxAttributesExistsPtr == NULL)
    {
        PDL_LIB_DEBUG_MAC (prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"fineTuneRxAttributesExistsPtr NULL, LANE ID: %d, DEVICE ID: %d",laneId,ppId);
        return PDL_BAD_PTR;
    }
    if (fineTuneRxAttributesPtr == NULL)
    {
        PDL_LIB_DEBUG_MAC (prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"fineTuneRxAttributesPtr NULL, LANE ID: %d, DEVICE ID: %d",laneId,ppId);
        return PDL_BAD_PTR;
    }
    laneKey.ppNumber = ppId;
    laneKey.laneId = laneId;
    pdlStatus = prvPdlDbFind(&pdlSerdesDb, &laneKey, (void **)&laneEntryPtr);
    if (pdlStatus == PDL_NOT_FOUND)
    {
        PDL_LIB_DEBUG_MAC (prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"LANE ID: %d, DEVICE ID: %d NOT FOUND",laneId,ppId);
        return PDL_NOT_FOUND;
    }
    PDL_CHECK_STATUS(pdlStatus);

    /* now find tune params relevant to connector type & interface mode */
    laneInfoKey.connectorType = connectorType;
    laneInfoKey.interfaceMode = interfaceMode;
    pdlStatus = prvPdlDbFind(&laneEntryPtr->laneInfoDb, &laneInfoKey, (void **)&laneInfoEntryPtr);
    if (pdlStatus == PDL_NOT_FOUND)
    {
        PDL_LIB_DEBUG_MAC (prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"LANE ID: %d, DEVICE ID: %d INTERFACE MODE: %d CONNECTOR: %d NOT FOUND",laneId,ppId, interfaceMode, connectorType);
        return PDL_NOT_FOUND;
    }
    PDL_CHECK_STATUS(pdlStatus);

    *fineTuneTxAttributesExistsPtr = laneInfoEntryPtr->fineTuneTxExist;
    *fineTuneRxAttributesExistsPtr = laneInfoEntryPtr->fineTuneRxExist;
    if (laneInfoEntryPtr->fineTuneTxExist) {
        memcpy (fineTuneTxAttributesPtr, &laneInfoEntryPtr->fineTuneTxAttributes, sizeof (PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC));
    }
    if (laneInfoEntryPtr->fineTuneRxExist) {
        memcpy (fineTuneRxAttributesPtr, &laneInfoEntryPtr->fineTuneRxAttributes, sizeof (PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC));
    }
    return PDL_OK;
}
/*$ END OF pdlSerdesDbFineTuneAttrGet */

/**
 * @fn  PDL_STATUS pdlSerdesDbFineTuneAttrGetFirst( IN UINT_32 ppId, IN UINT_32 laneId, OUT PDL_INTERFACE_MODE_ENT *interfaceModePtr, OUT PDL_CONNECTOR_TYPE_ENT *connectorTypePtr );
 *
 * @brief   Get serdes first interfaceMode and connectorType 's
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                                devId
 * @param [in]  laneId                              serdes's absolute lane identifier.
 * @param [out] interfaceModePtr                    first interface mode.
 * @param [out] connectorTypePtr                    first connector Type
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given interfaceMode & connectorType
 * @return  PDL_BAD_OK                  lane attributes found and returned
 */
PDL_STATUS pdlSerdesDbFineTuneAttrGetFirst(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    OUT PDL_INTERFACE_MODE_ENT                             *interfaceModePtr,
    OUT PDL_CONNECTOR_TYPE_ENT                             *connectorTypePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PRV_PDL_LANE_ENTRY_KEY_STC                              laneKey;
    PRV_PDL_LANE_ENTRY_STC                                * laneEntryPtr;
    PRV_PDL_LANE_INFO_ENTRY_STC                           * laneInfoEntryPtr;
/****************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

    if (interfaceModePtr == NULL || connectorTypePtr == NULL)
    {
         PDL_LIB_DEBUG_MAC (prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"interfaceModePtr/connectorTypePtr NULL, LANE ID: %d, DEVICE ID: %d",laneId,ppId);
         return PDL_BAD_PTR;
    }
    laneKey.ppNumber = ppId;
    laneKey.laneId   = laneId;
    pdlStatus = prvPdlDbFind(&pdlSerdesDb, &laneKey, (void **)&laneEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    /* now find tune params relevant to connector type & interface mode */
    pdlStatus = prvPdlDbGetFirst(&laneEntryPtr->laneInfoDb, (void **)&laneInfoEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (pdlStatus == PDL_OK) {
        *interfaceModePtr = laneInfoEntryPtr->key.interfaceMode;
        *connectorTypePtr = laneInfoEntryPtr->key.connectorType;
        return PDL_OK;
    }
    PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"CAN'T FIND PDL");
    return PDL_NOT_FOUND;
}
/*$ END OF pdlSerdesDbFineTuneAttrGetFirst */

/**
 * @fn  PDL_STATUS pdlSerdesDbFineTuneAttrGetNext( IN UINT_32 ppId, IN UINT_32 laneId, IN PDL_INTERFACE_MODE_ENT currInterfaceMode, IN PDL_CONNECTOR_TYPE_ENT currConnectorType, OUT PDL_INTERFACE_MODE_ENT *nextInterfaceModePtr, OUT PDL_CONNECTOR_TYPE_ENT *nextConnectorTypePtr );
 *
 * @brief   Get serdes next interfaceMode and connectorType 's
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                                    devId
 * @param [in]  laneId                                  serdes's absolute lane identifier.
 * @param [in]  currInterfaceMode                       current interface mode.
 * @param [in]  currConnectorType                       current connector Type
 * @param [out] nextInterfaceModePtr                    next interface mode.
 * @param [out] nextConnectorTypePtr                    next connector Type
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given interfaceMode & connectorType
 * @return  PDL_BAD_OK                  lane attributes found and returned
 */
PDL_STATUS pdlSerdesDbFineTuneAttrGetNext(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    IN  PDL_INTERFACE_MODE_ENT                              currInterfaceMode,
    IN  PDL_CONNECTOR_TYPE_ENT                              currConnectorType,
    OUT PDL_INTERFACE_MODE_ENT                             *nextInterfaceModePtr,
    OUT PDL_CONNECTOR_TYPE_ENT                             *nextConnectorTypePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PRV_PDL_LANE_ENTRY_STC                                * laneEntryPtr;
    PRV_PDL_LANE_ENTRY_KEY_STC                              laneKey;
    PRV_PDL_LANE_INFO_ENTRY_STC                           * laneInfoEntryPtr;
    PRV_PDL_LANE_INFO_ENTRY_KEY_STC                         laneInfoKey;
/****************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

    if (nextInterfaceModePtr == NULL || nextConnectorTypePtr == NULL)
    {
         PDL_LIB_DEBUG_MAC (prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"nextInterfaceModePtr/nextConnectorTypePtr NULL, LANE ID: %d, DEVICE ID: %d",laneId,ppId);
         return PDL_BAD_PTR;
    }
    laneKey.ppNumber = ppId;
    laneKey.laneId   = laneId;
    pdlStatus = prvPdlDbFind(&pdlSerdesDb, &laneKey, (void **)&laneEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    /* now find tune params relevant to connector type & interface mode */
    laneInfoKey.interfaceMode = currInterfaceMode;
    laneInfoKey.connectorType = currConnectorType;
    pdlStatus = prvPdlDbGetNext(&laneEntryPtr->laneInfoDb, &laneInfoKey, (void **)&laneInfoEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (pdlStatus == PDL_OK) {
        *nextInterfaceModePtr = laneInfoEntryPtr->key.interfaceMode;
        *nextConnectorTypePtr = laneInfoEntryPtr->key.connectorType;
        return PDL_OK;
    }
    PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"CAN'T FIND PDL");
    return PDL_NOT_FOUND;
}
/*$ END OF pdlSerdesDbFineTuneAttrGetNext */

/**
 * @fn  PDL_STATUS pdlSerdesPortDbSerdesGetFirst( IN UINT_32 dev, IN UINT_32 port, OUT PDL_PORT_LANE_DATA_STC * serdesInfo );
 *
 * @brief   Get info for first serdes
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in]  dev                     packet processor identifier.
 * @param [in]  port                    port
 * @param [out] serdesInfo              first serdes connected to port
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given dev & port
 * @return  PDL_BAD_OK                  lane attributes found and returned
 */
PDL_STATUS pdlSerdesPortDbSerdesInfoGetFirst(
    IN  UINT_32                                             dev,
    IN  UINT_32                                             port,
    OUT PDL_PORT_LANE_DATA_STC                            * serdesInfoPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PRV_PDL_PORT_KEY_STC            portKey;
    PDL_PORT_LANE_DATA_STC        * laneInfoPtr;
    PRV_PDL_PORT_LANE_ENTRY_STC   * portEntryPtr;
    PDL_STATUS                      pdlStatus;
/*****************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (serdesInfoPtr == NULL)
    {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"serdesInfoPtr NULL, PORT NUMBER: %d, DEVICE NUMBER: %d",port,dev);
        return PDL_BAD_PTR;
    }
    portKey.dev = dev;
    portKey.port = port;
    pdlStatus = prvPdlDbFind(&pdlPortSerdesDb, &portKey, (void **)&portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlDbGetFirst(&portEntryPtr->portSerdesInfoDb, (void**)&laneInfoPtr);
    memcpy (serdesInfoPtr, laneInfoPtr, sizeof(PDL_PORT_LANE_DATA_STC));
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, serdesInfoPtr->absSerdesNum);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, serdesInfoPtr->relSerdesNum);
    return PDL_OK;
}

/*$ END OF pdlSerdesPortDbSerdesInfoGetFirst */


/**
 * @fn  PDL_STATUS pdlSerdesPortDbSerdesInfoGet( IN UINT_32 ppId, IN UINT_32 laneId, IN PDL_INTERFACE_MODE_ENT interfaceMode, OUT PDL_L1_INTERFACE_CONNECTED_PORTS_GROUP_DATA_STC * portsGroupAttributesPtr );
 *
 * @brief   Get port serdes information for dev & port
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in]  dev                     packet processor identifier.
 * @param [in]  port                    port
 * @param [in]  serdesInfoPtr           current serdes info
 * @param [out] serdesNextInfoPtr       next port's serdes info
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find port index for given dev & port
 * @return  PDL_BAD_OK                  lane attributes found and returned
 */
PDL_STATUS pdlSerdesPortDbSerdesInfoGetNext(
    IN  UINT_32                                             dev,
    IN  UINT_32                                             port,
    IN  PDL_PORT_LANE_DATA_STC                            * serdesInfoPtr,
    OUT PDL_PORT_LANE_DATA_STC                            * serdesNextInfoPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PRV_PDL_PORT_KEY_STC            portKey;
    PRV_PDL_PORT_LANE_ENTRY_STC   * portEntryPtr;
    PDL_PORT_LANE_DATA_KEY_STC      portSerdesKey;
    PDL_PORT_LANE_DATA_STC        * portSerdesinfoPtr;
    PDL_STATUS                      pdlStatus;
/****************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (serdesInfoPtr == NULL || serdesNextInfoPtr == NULL)
    {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__,"serdesInfoPtr OR serdesNextInfoPtr NULL, PORT NUMBER: %d, DEVICE NUMBER: %d",port,dev);
        return PDL_BAD_PTR;
    }
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_8, serdesInfoPtr->absSerdesNum);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_8, serdesInfoPtr->relSerdesNum);

    portKey.dev = dev;
    portKey.port = port;
    pdlStatus = prvPdlDbFind(&pdlPortSerdesDb, &portKey, (void **)&portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);
    /* key for entry in portSerdesDb is relSerdes number */
    portSerdesKey.relSerdesNum = serdesInfoPtr->relSerdesNum;
    pdlStatus = prvPdlDbGetNext(&portEntryPtr->portSerdesInfoDb, &portSerdesKey, (void **)&portSerdesinfoPtr);
    PDL_CHECK_STATUS(pdlStatus);

    memcpy (serdesNextInfoPtr, portSerdesinfoPtr, sizeof (PDL_PORT_LANE_DATA_STC));
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, serdesNextInfoPtr->absSerdesNum);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, serdesNextInfoPtr->relSerdesNum);

    return PDL_OK;
}
/*$ END OF pdlSerdesPortDbSerdesInfoGetNext */

/**
 * @fn  PDL_STATUS pdlSerdesDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Serdes debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSerdesDebugSet (
    IN  BOOLEAN             state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlSerdesDebugFlag = state;
    return PDL_OK;
}
/*$ END OF pdlSerdesDebugSet */

/**
 * @fn  PDL_STATUS PdlSerdesInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlRootId )
 *
 * @brief   Init serdes module Build serdes DB from XML data
 *
 * @param [in]  xmlRootId   Xml id.
 *
 * @return  PDL_NOT_FOUND if xml parsing wasn't successful because mandatory tag not found.
 * @return  PDL_CREATE_ERROR if number of packet processors doesn't match xml parsing.
 */
PDL_STATUS PdlSerdesInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlRootId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
    PDL_DB_PRV_ATTRIBUTES_STC       dbAttributes;
/****************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    dbAttributes.listAttributes.keySize        = sizeof(PRV_PDL_LANE_ENTRY_KEY_STC);
    dbAttributes.listAttributes.entrySize      = sizeof(PRV_PDL_LANE_ENTRY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E, &dbAttributes, &pdlSerdesDb);
    PDL_CHECK_STATUS(pdlStatus);

    dbAttributes.listAttributes.keySize        = sizeof(PRV_PDL_PORT_KEY_STC);
    dbAttributes.listAttributes.entrySize      = sizeof(PRV_PDL_PORT_LANE_ENTRY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E, &dbAttributes, &pdlPortSerdesDb);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlSerdesXMLParser (xmlRootId);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}
/*$ END OF PdlSerdesInit */

/**
 * @fn  PDL_STATUS prvPdlSerdesDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSerdesDestroy (
    void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
    PRV_PDL_PORT_KEY_STC            portKey;
    PRV_PDL_PORT_LANE_ENTRY_STC   * portPtr;
    PRV_PDL_LANE_ENTRY_KEY_STC      laneKey;
    PRV_PDL_LANE_ENTRY_STC        * laneEntryPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    pdlStatus = prvPdlDbGetNextKey (&pdlPortSerdesDb, NULL, (void*) &portKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus  = prvPdlDbFind (&pdlPortSerdesDb, (void*) &portKey, (void**) &portPtr);
        PDL_CHECK_STATUS(pdlStatus);
        if (portPtr->portSerdesInfoDb.dbPtr != NULL) {
            pdlStatus = prvPdlDbDestroy(&portPtr->portSerdesInfoDb);
            PDL_CHECK_STATUS(pdlStatus);
        }
        pdlStatus  = prvPdlDbGetNextKey (&pdlPortSerdesDb, (void*) &portKey, (void*) &portKey);
    }

    pdlStatus = prvPdlDbGetNextKey (&pdlSerdesDb, NULL, (void*) &laneKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus  = prvPdlDbFind (&pdlSerdesDb, (void*) &laneKey, (void**) &laneEntryPtr);
        PDL_CHECK_STATUS(pdlStatus);
        if (laneEntryPtr->laneInfoDb.dbPtr != NULL) {
            pdlStatus = prvPdlDbDestroy(&laneEntryPtr->laneInfoDb);
            PDL_CHECK_STATUS(pdlStatus);
        }
        pdlStatus = prvPdlDbGetNextKey (&pdlSerdesDb, (void*) &laneKey, (void*) &laneKey);
    }

    if (pdlSerdesDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlSerdesDb);
        PDL_CHECK_STATUS(pdlStatus);
    }
    if (pdlPortSerdesDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlPortSerdesDb);
        PDL_CHECK_STATUS(pdlStatus);
    }
    return PDL_OK;
}

/*$ END OF prvPdlSerdesDestroy */


