/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlOobPort.c
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
 * @brief Platform driver layer - OOB port related API
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
#include <pdl/phy/private/prvPdlPhy.h>
#include <pdl/oob_port/private/prvPdlOobPort.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/** @brief   Number of prv pdl oob ports */
static UINT_32          prvPdlOobPortCount = 0;
/** @brief   The prv pdl oob port database */
static PDL_DB_PRV_STC   prvPdlOobPortDb;


/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)                                                                  \
    if (__pdlStatus != PDL_OK) {                                                                       \
    if (prvPdlOobPortDebugFlag) {                                                                      \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__pdlStatus,PDL_STATUS_ERROR_PRINT_IDENTIFIER);      \
    }                                                                                                  \
    return __pdlStatus;                                                                                \
    }  
/*macro to check the xml status and debug prints if debug flag on*/
#undef XML_CHECK_STATUS
#define XML_CHECK_STATUS(__xmlStatus)                                                                  \
    if (__xmlStatus != PDL_OK) {                                                                       \
    if (prvPdlOobPortDebugFlag) {                                                                      \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__xmlStatus,XML_STATUS_ERROR_PRINT_IDENTIFIER);      \
    }                                                                                                  \
    return __xmlStatus;                                                                                \
    }  



/**
 * @fn  PDL_STATUS pdlOobPortNumOfObbPortsGet ( OUT UINT_32 * numOfOobsPtr )
 *
 * @brief   Get number of OOB ports
 *
 * @param [out] numOfOobsPtr    number of oob ports in system.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlOobPortNumOfObbPortsGet (
    OUT  UINT_32    * numOfOobsPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

    *numOfOobsPtr = prvPdlOobPortCount;

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlOobPortAttributesGet ( IN UINT_32 portNumber, OUT PDL_OOB_PORT_XML_ATTRIBUTES_STC * portAttributesPtr )
 *
 * @brief   Get OOB port attributes
 *
 * @param [in]  portNumber          OOB port number.
 * @param [out] portAttributesPtr   OOB port attributes.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlOobPortAttributesGet (
    IN   UINT_32                                portNumber,
    OUT  PDL_OOB_PORT_XML_ATTRIBUTES_STC      * portAttributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                       pdlStatus;
    PRV_PDL_OOB_PORT_ENTRY_STC     * oobEntryPtr;
    PRV_PDL_OOB_PORT_KEY_STC         oobKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (portAttributesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    oobKey.portNumber = portNumber;
    pdlStatus = prvPdlDbFind(&prvPdlOobPortDb, &oobKey, (void **)&oobEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);
    *portAttributesPtr = oobEntryPtr->oobInfo;
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.ethId);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.cpuPortNumber);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.portMaximumSpeed);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.phyConfig.phyNumber);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.phyConfig.phyPosition);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.phyConfig.smiXmsiInterface.interfaceId);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.phyConfig.smiXmsiInterface.interfaceType);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.phyConfig.smiXmsiInterface.dev);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.phyConfig.smiXmsiInterface.address);

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlOobPortAttributesSet ( IN UINT_32 portNumber, IN PDL_OOB_PORT_XML_ATTRIBUTES_STC * portAttributesPtr )
 *
 * @brief   Set OOB port attributes
 *
 * @param [in] portNumber           OOB port number.
 * @param [in] portAttributesPtr    OOB port attributes.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlOobPortAttributesSet (
    IN   UINT_32                                portNumber,
    IN   PDL_OOB_PORT_XML_ATTRIBUTES_STC      * portAttributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                       pdlStatus;
    PRV_PDL_OOB_PORT_ENTRY_STC     * oobEntryPtr;
    PRV_PDL_OOB_PORT_KEY_STC         oobKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (portAttributesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->ethId);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->cpuPortNumber);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->portMaximumSpeed);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->phyConfig.phyNumber);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->phyConfig.phyPosition);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->phyConfig.smiXmsiInterface.interfaceId);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->phyConfig.smiXmsiInterface.interfaceType);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->phyConfig.smiXmsiInterface.dev);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->phyConfig.smiXmsiInterface.address);

    oobKey.portNumber = portNumber;
    pdlStatus = prvPdlDbFind(&prvPdlOobPortDb, &oobKey, (void **)&oobEntryPtr);
    if (pdlStatus != PDL_OK)
        return PDL_NOT_FOUND;

    oobEntryPtr->oobInfo = *portAttributesPtr;

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS prvPdlOobPortInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Init oob port module Create oob port DB and initialize
 *
 * @param [in]  xmlId   Xml id.
 *
 * @return  PDL_STATUS.
 */

/**
 * @fn  PDL_STATUS pdlOobDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Oob debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlOobDebugSet (
    IN  BOOLEAN             state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlOobPortDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlOobDebugSet */



PDL_STATUS prvPdlOobPortInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    XML_PARSER_RET_CODE_TYP         xmlStatus;
    PDL_STATUS                      pdlStatus;
    PDL_DB_PRV_ATTRIBUTES_STC       dbAttributes;
    XML_PARSER_NODE_DESCRIPTOR_TYP  xmlOobId, xmlOobChildId;
    char                            pdlTagStr[PDL_XML_MAX_TAG_LEN];
    UINT_32                         maxSize, value;
    PRV_PDL_OOB_PORT_ENTRY_STC      oobPortEntry, *oobPortEntryPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

    xmlStatus = xmlParserFindByName (xmlId, "OOB-port", &xmlOobId);
    if (xmlStatus == XML_PARSER_RET_CODE_NOT_FOUND) {
        prvPdlOobPortCount = 0;
        return PDL_OK;
    }
    XML_CHECK_STATUS(xmlStatus);

    prvPdlOobPortCount ++;

    dbAttributes.listAttributes.entrySize = sizeof(PRV_PDL_OOB_PORT_ENTRY_STC);
    dbAttributes.listAttributes.keySize = sizeof(PRV_PDL_OOB_PORT_KEY_STC);

    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &prvPdlOobPortDb);
    PDL_CHECK_STATUS(pdlStatus);

    xmlStatus = xmlParserGetFirstChild (xmlOobId, &xmlOobChildId);
    XML_CHECK_STATUS(xmlStatus);
    /*TODO CHANGE ALL DEBUG FLAG*/
    xmlStatus = xmlParserIsEqualName(xmlOobChildId, "speed");
    XML_CHECK_STATUS(xmlStatus);
    maxSize = sizeof(pdlTagStr);
    xmlStatus = xmlParserGetValue (xmlOobChildId, &maxSize, &pdlTagStr[0]);
    XML_CHECK_STATUS(xmlStatus);
    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_SPEED_TYPE_E, pdlTagStr, &value);
    PDL_CHECK_STATUS(pdlStatus);
    oobPortEntry.oobInfo.portMaximumSpeed = (PDL_PORT_SPEED_ENT)value;

    xmlStatus = xmlParserGetNextSibling(xmlOobChildId, &xmlOobChildId);
    XML_CHECK_STATUS(xmlStatus);
    xmlStatus = xmlParserIsEqualName(xmlOobChildId, "eth-id");
    maxSize = sizeof(oobPortEntry.oobInfo.ethId);
    pdlStatus = xmlParserGetValue (xmlOobChildId, &maxSize, &oobPortEntry.oobInfo.ethId);
    PDL_CHECK_STATUS(pdlStatus);

    xmlStatus = xmlParserGetNextSibling(xmlOobChildId, &xmlOobChildId);

    if (xmlStatus == XML_PARSER_RET_CODE_OK)
    {
        xmlStatus = xmlParserIsEqualName(xmlOobChildId, "cpu-port-number");
        oobPortEntry.oobInfo.cpuPortNumber = 0;
        if (xmlStatus == XML_PARSER_RET_CODE_OK)
        {
            maxSize = sizeof(oobPortEntry.oobInfo.cpuPortNumber);
            xmlStatus = xmlParserGetValue(xmlOobChildId, &maxSize, &oobPortEntry.oobInfo.cpuPortNumber);
            XML_CHECK_STATUS(xmlStatus);

            xmlStatus = xmlParserGetNextSibling(xmlOobChildId, &xmlOobChildId);
        }
    }
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserIsEqualName(xmlOobChildId, "phy-read-write-address");
    XML_CHECK_STATUS(xmlStatus);

    pdlStatus = prvPdlPhyPortDataGet (xmlOobChildId, &oobPortEntry.oobInfo.phyConfig);
    PDL_CHECK_STATUS(pdlStatus);

    oobPortEntry.key.portNumber = 1;
    pdlStatus = prvPdlDbAdd(&prvPdlOobPortDb, &oobPortEntry.key, &oobPortEntry, (void **)&oobPortEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;   
}

/**
 * @fn  PDL_STATUS prvPdlOobDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlOobDestroy (
    void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (prvPdlOobPortDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&prvPdlOobPortDb);
        PDL_CHECK_STATUS(pdlStatus);    
    }
    return PDL_OK;
}

/*$ END OF prvPdlOobDestroy */
