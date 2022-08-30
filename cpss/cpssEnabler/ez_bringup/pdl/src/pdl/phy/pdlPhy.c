/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlPhy.c   
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
 * @brief Platform driver layer - Phy related API
 * 
 * @version   1 
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/phy/pdlPhy.h>
#include <pdl/phy/private/prvPdlPhy.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <pdl/interface/private/prvPdlSmiXsmi.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/


/** @brief   The prv pdl phy database */
static PDL_DB_PRV_STC prvPdlPhyDb;


/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)                                                                 \
    if (__pdlStatus != PDL_OK) {                                                                      \
    if (prvPdlPhyDebugFlag) {                                                                         \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__pdlStatus,PDL_STATUS_ERROR_PRINT_IDENTIFIER);     \
    }                                                                                                 \
    return __pdlStatus;                                                                               \
    }  
/*macro to check the xml status and debug prints if debug flag on*/
#undef XML_CHECK_STATUS
#define XML_CHECK_STATUS(__xmlStatus)                                                                   \
    if (__xmlStatus != PDL_OK) {                                                                        \
    if (prvPdlPhyDebugFlag) {                                                                           \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__xmlStatus,XML_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __xmlStatus;                                                                                 \
    }  

/**
 * @fn  static UINT_32 prvPdlPhyDbArrayKeyToIndex( IN void * keyPtr )
 *
 * @brief   Convert phy id to phy db index
 *
 * @param [in]  keyPtr  phy db key.
 *
 * @return  index to phy array db.
 */

static UINT_32 prvPdlPhyDbArrayKeyToIndex(
    IN void * keyPtr
)
{
    PRV_PDL_PHY_KEY_STC    * phyKeyPtr = (PRV_PDL_PHY_KEY_STC *)keyPtr;

    return phyKeyPtr->phyId;
}

/**
 * @fn  PDL_STATUS PdlPhyInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Init phy module Create phy DB and initialize
 *
 * @param [in]  xmlId   Xml id.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS PdlPhyInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
    char                             pdlTagStr[PDL_XML_MAX_TAG_LEN];
    PDL_STATUS                       pdlStatus;
    XML_PARSER_RET_CODE_TYP          xmlStatus;
    PRV_PDL_PHY_DB_STC               phyInfo, * phyPtr = NULL;
    PRV_PDL_PHY_KEY_STC              phyKey;
    XML_PARSER_NODE_DESCRIPTOR_TYP   xmlPhyId, xmlPhyInstanceId, xmlPhyChildId;
    UINT_32                          maxSize, numOfPhys, value;
    PDL_DB_PRV_ATTRIBUTES_STC        dbAttributes;

    xmlStatus = xmlParserFindByName (xmlId, PRV_PDL_PHYS_TAG_NAME_CNS, &xmlPhyId);
    if (xmlStatus == XML_PARSER_RET_CODE_NOT_FOUND) {
         return PDL_OK;
    }

    xmlStatus = xmlParserGetTagCount(xmlPhyId, PRV_PDL_PHY_TAG_NAME_CNS, &numOfPhys);
    if ((xmlStatus != XML_PARSER_RET_CODE_OK) || (numOfPhys == 0))
        return PDL_NOT_FOUND;

    dbAttributes.arrayAttributes.entrySize = sizeof(PRV_PDL_PHY_DB_STC);
    dbAttributes.arrayAttributes.keySize = sizeof(PRV_PDL_PHY_KEY_STC);
    dbAttributes.arrayAttributes.numOfEntries = numOfPhys;
    dbAttributes.arrayAttributes.keyToIndexFunc = prvPdlPhyDbArrayKeyToIndex;
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_ARRAY_E,
                                &dbAttributes,
                                &prvPdlPhyDb);
    PDL_CHECK_STATUS(pdlStatus);

    xmlStatus = xmlParserGetFirstChild (xmlPhyId, &xmlPhyInstanceId);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserIsEqualName(xmlPhyInstanceId, PRV_PDL_PHY_TAG_NAME_CNS);

    /* go over all phys */
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);

        xmlStatus = xmlParserGetFirstChild (xmlPhyInstanceId, &xmlPhyChildId);
        XML_CHECK_STATUS(xmlStatus);
        xmlStatus = xmlParserIsEqualName(xmlPhyChildId, PRV_PDL_PHY_ID_TAG_NAME_CNS);
        XML_CHECK_STATUS(xmlStatus);

        memset (&phyInfo, 0, sizeof (phyInfo));
        maxSize = sizeof(phyInfo.key.phyId);
        xmlStatus = xmlParserGetValue(xmlPhyChildId, &maxSize, &phyInfo.key.phyId);
        XML_CHECK_STATUS(xmlStatus);

        xmlStatus = xmlParserGetNextSibling(xmlPhyChildId, &xmlPhyChildId);
        XML_CHECK_STATUS(xmlStatus);
        xmlStatus = xmlParserIsEqualName(xmlPhyChildId, PRV_PDL_PHY_TYPE_TAG_NAME_CNS);
        XML_CHECK_STATUS(xmlStatus);

        maxSize = sizeof(pdlTagStr);
        xmlStatus = xmlParserGetValue(xmlPhyChildId, &maxSize, &pdlTagStr);
        XML_CHECK_STATUS(xmlStatus);
        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_PHY_TYPE_TYPE_E, pdlTagStr, &value);
        PDL_CHECK_STATUS(pdlStatus);
        phyInfo.phyInfo.phyType = (PDL_PHY_TYPE_ENT)value;

        xmlStatus = xmlParserGetNextSibling(xmlPhyChildId, &xmlPhyChildId);
        XML_CHECK_STATUS(xmlStatus);
        xmlStatus = xmlParserIsEqualName(xmlPhyChildId, PRV_PDL_PHY_FW_DOWNLOAD_METHOD_TAG_NAME_CNS);
        XML_CHECK_STATUS(xmlStatus);

        maxSize = sizeof(pdlTagStr);
        xmlStatus = xmlParserGetValue(xmlPhyChildId, &maxSize, &pdlTagStr);
        XML_CHECK_STATUS(xmlStatus);
        pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_E, pdlTagStr, &value);
        PDL_CHECK_STATUS(pdlStatus);
        phyInfo.phyInfo.phyDownloadType = (PDL_PHY_DOWNLOAD_TYPE_ENT)value;

        phyKey.phyId = phyInfo.key.phyId; 
        pdlStatus = prvPdlDbAdd(&prvPdlPhyDb, (void*) &phyKey, (void*) &phyInfo, (void**) &phyPtr);
        PDL_CHECK_STATUS(pdlStatus);

        numOfPhys --;

        xmlStatus = xmlParserGetNextSibling (xmlPhyInstanceId, &xmlPhyInstanceId);
    }

    if (numOfPhys){
        PDL_LIB_DEBUG_MAC(prvPdlPhyDebugFlag)(__FUNCTION__, __LINE__,"numOfPhys != 0");
        return PDL_NOT_FOUND;
    }

    return PDL_OK;   
}

/**
 * @fn  PDL_STATUS PdlPhyDbNumberOfPhysGet( OUT UINT_32 * numOfPhysPtr )
 *
 * @brief   Get number of phys
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [out] numOfPhysPtr    number of phys.
 *
 * @return  PDL_STATUS  .
 */

PDL_STATUS PdlPhyDbNumberOfPhysGet(
    OUT UINT_32   * numOfPhysPtr
)
{
    PDL_STATUS      pdlStatus;
    pdlStatus = prvPdlDbGetNumOfEntries(&prvPdlPhyDb, numOfPhysPtr);

    return pdlStatus;
}

/**
 * @fn  PDL_STATUS PdlPhyDbPhyConfigurationGet ( IN UINT_32 phyId, OUT PDL_PHY_CONFIGURATION_STC * phyConfigPtr )
 *
 * @brief   Get phy configuration data
 *
 * @param [in]  phyId           phy identifier starting from 1 up to numOfPhys.
 * @param [out] phyConfigPtr    phy configuration data.
 *
 * @return  PDL_BAD_PARAM if bad pointer supplied.
 */

PDL_STATUS PdlPhyDbPhyConfigurationGet (
    IN  UINT_32                             phyId,
    OUT PDL_PHY_CONFIGURATION_STC         * phyConfigPtr
)
{
    PDL_STATUS                       pdlStatus;
    PRV_PDL_PHY_DB_STC             * phyInfoPtr;
    PRV_PDL_PHY_KEY_STC              phyKey;

    phyKey.phyId = (UINT_8)phyId;
    pdlStatus = prvPdlDbFind(&prvPdlPhyDb, &phyKey, (void **)&phyInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);

    phyConfigPtr->phyType = phyInfoPtr->phyInfo.phyType;
    phyConfigPtr->phyDownloadType = phyInfoPtr->phyInfo.phyDownloadType;

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS prvPdlPhyPortDataGet ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT PDL_PHY_PORT_CONFIGURATION_STC *phyDataPtr )
 *
 * @brief   Parse phy port xml section
 *
 * @param [in]  xmlId       Xml id pointing to 'port-phy-connection' to parse.
 * @param [out] phyDataPtr  phy data.
 *
 * @return  PDL_XML_PARSE_ERROR if xml section coul not be parsed.
 * @return  PDL_BAD_PTR         if supplied pointer is NULL.
 */

/**
 * @fn  PDL_STATUS pdlPhyDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Phy debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPhyDebugSet (
    IN  BOOLEAN             state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlPhyDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlPhyDebugSet */


PDL_STATUS prvPdlPhyPortDataGet (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    OUT PDL_PHY_PORT_CONFIGURATION_STC      *phyDataPtr
)
{
    PDL_STATUS                              pdlStatus;
    XML_PARSER_RET_CODE_TYP                 xmlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP          xmlChildlId;
    UINT_32                                 maxSize;
    PDL_INTERFACE_PRV_SMI_XSMI_DB_STC       smiXsmiInfo;

    if (phyDataPtr == NULL)
    {
        PDL_LIB_DEBUG_MAC(prvPdlPhyDebugFlag)(__FUNCTION__, __LINE__,"phyDataPtr NULL");
        return PDL_BAD_PTR;
    }

    memset(phyDataPtr, 0, sizeof(PDL_PHY_PORT_CONFIGURATION_STC));

    xmlStatus = xmlParserIsEqualName(xmlId, "phy-read-write-address");
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserGetFirstChild(xmlId, &xmlChildlId);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserIsEqualName(xmlChildlId, "phy-number");
    if (xmlStatus == XML_PARSER_RET_CODE_OK)
    {
        maxSize = sizeof(phyDataPtr->phyNumber);
        xmlStatus = xmlParserGetValue(xmlChildlId, &maxSize, &phyDataPtr->phyNumber);
        XML_CHECK_STATUS(xmlStatus);

        xmlStatus = xmlParserGetNextSibling(xmlChildlId, &xmlChildlId);
        if (xmlStatus == XML_PARSER_RET_CODE_OK)
        {
            xmlStatus = xmlParserIsEqualName(xmlChildlId, "phy-position");
            if (xmlStatus == XML_PARSER_RET_CODE_OK)
            {
                maxSize = sizeof(phyDataPtr->phyPosition);
                xmlStatus = xmlParserGetValue(xmlChildlId, &maxSize, &phyDataPtr->phyPosition);
                XML_CHECK_STATUS(xmlStatus);
                if (phyDataPtr->phyPosition > PRV_PDL_PHY_MAX_POSITION_CNS)
                {
                    PDL_LIB_DEBUG_MAC(prvPdlPhyDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL phyPosition");
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus = xmlParserGetNextSibling(xmlChildlId, &xmlChildlId);
            }
            else
            {
                xmlStatus = XML_PARSER_RET_CODE_OK;
            }
        }
    }
    else
    {
        PDL_LIB_DEBUG_MAC(prvPdlPhyDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlChildlId POINTER OR NAME DON'T MATCH TO: phy-number");
        phyDataPtr->phyNumber = PRV_PDL_PHY_ILLEGAL_NUMBER_CNS;
        xmlStatus = XML_PARSER_RET_CODE_OK;
    }
    XML_CHECK_STATUS(xmlStatus);

    /*xmlStatus = xmlParserIsEqualName(xmlChildlId, "smi-xsmi-interface");
    XML_CHECK_STATUS(xmlStatus);*/

    pdlStatus = prvPdlSmiXsmiXmlAttributesParser(xmlChildlId, &smiXsmiInfo);
    PDL_CHECK_STATUS(pdlStatus);

    memcpy (&phyDataPtr->smiXmsiInterface, &smiXsmiInfo.publicInfo, sizeof(PDL_INTERFACE_SMI_XSMI_STC));
    return PDL_OK;
}

/**
 * @fn  PDL_STATUS prvPdlPhyDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPhyDestroy (
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
    if (prvPdlPhyDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&prvPdlPhyDb);
        PDL_CHECK_STATUS(pdlStatus);
    }
    return PDL_OK;
}

/*$ END OF prvPdlPhyDestroy */

