/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlCpu.c
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
 * @brief Platform driver layer - CPU related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/xml/private/prvXmlParser.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/cpu/pdlCpu.h>
#include <pdl/cpu/private/prvPdlCpu.h>
#include <pdl/cpu/pdlCpuDebug.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/** @brief   Type of the prv pdl CPU */
static PDL_CPU_TYPE_ENT         prvPdlCpuType = PDL_CPU_TYPE_LAST_E;
/** @brief   Type of the prv pdl CPU family */
static PDL_CPU_FAMILY_TYPE_ENT  prvPdlCpuFamilyType;
static PDL_DB_PRV_STC           prvPdlCpuSdmaMapDb = {0, 0};

static UINT_32                  prvPdlNumOfUsbDevices = 0;
static BOOLEAN                  prvPdlNumOfUsbDevicesConfigured = FALSE;
/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)                                                                   \
    if (__pdlStatus != PDL_OK) {                                                                        \
    if (prvPdlCpuDebugFlag) {                                                                           \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__pdlStatus,PDL_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __pdlStatus;                                                                             \
    }
/*macro to check the xml status and debug prints if debug flag on*/
#undef XML_CHECK_STATUS
#define XML_CHECK_STATUS(__xmlStatus)                                                                   \
    if (__xmlStatus != PDL_OK) {                                                                        \
    if (prvPdlCpuDebugFlag) {                                                                           \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__xmlStatus,XML_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __xmlStatus;                                                                             \
    }


/** @brief   The cpu family string to enum pairs */
static PRV_PDL_LIB_STR_TO_ENUM_STC prvPdlCpuFamilyStrToEnumPairs[] = {
    {"MSYS" , PDL_CPU_FAMILY_TYPE_MSYS_E },
    {"AXP"  , PDL_CPU_FAMILY_TYPE_AXP_E  },
    {"A38X" , PDL_CPU_FAMILY_TYPE_A38X_E },
};
/** @brief   The xml error string to enum */
PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC prvPdlCpuFamilyStrToEnum = {prvPdlCpuFamilyStrToEnumPairs, sizeof(prvPdlCpuFamilyStrToEnumPairs)/sizeof(PRV_PDL_LIB_STR_TO_ENUM_STC)};

/**
 * @fn  PDL_STATUS pdlCpuTypeGet ( OUT PDL_CPU_TYPE_ENT * cpuTypePtr )
 *
 * @brief   Get cpu type
 *
 * @param [out] cpuTypePtr  cpu type.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlCpuTypeGet (
    OUT PDL_CPU_TYPE_ENT          * cpuTypePtr
)
{
    if (prvPdlCpuType == PDL_CPU_TYPE_LAST_E)
        return PDL_NOT_INITIALIZED;

    if (cpuTypePtr == NULL)
        return PDL_BAD_PTR;

    * cpuTypePtr = prvPdlCpuType;

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlCpuTypeSet ( IN PDL_CPU_TYPE_ENT cpuType )
 *
 * @brief   Set cpu type
 *
 * @param [in] cpuType  cpu type.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlCpuTypeSet (
    IN PDL_CPU_TYPE_ENT          cpuType
)
{
    if (prvPdlCpuType == PDL_CPU_TYPE_LAST_E)
        return PDL_NOT_INITIALIZED;

    switch (cpuType)
    {
            /* MSYS Family */
        case PDL_CPU_TYPE_XP_EMBEDDED_E:
            prvPdlCpuFamilyType = PDL_CPU_FAMILY_TYPE_MSYS_E;
            break;
        case PDL_CPU_TYPE_ARMADA_MV78130_E:
        case PDL_CPU_TYPE_ARMADA_MV78160_E:
        case PDL_CPU_TYPE_ARMADA_MV78230_E:
        case PDL_CPU_TYPE_ARMADA_MV78260_E:
        case PDL_CPU_TYPE_ARMADA_MV78460_E:
            prvPdlCpuFamilyType = PDL_CPU_FAMILY_TYPE_AXP_E;
            break;
        case PDL_CPU_TYPE_ARMADA_88F6810_E:
        case PDL_CPU_TYPE_ARMADA_88F6811_E:
        case PDL_CPU_TYPE_ARMADA_88F6820_E:
        case PDL_CPU_TYPE_ARMADA_88F6821_E:
        case PDL_CPU_TYPE_ARMADA_88F6W21_E:
        case PDL_CPU_TYPE_ARMADA_88F6828_E:
            prvPdlCpuFamilyType = PDL_CPU_FAMILY_TYPE_A38X_E;
            break;
        default:
            prvPdlCpuType = PDL_CPU_TYPE_LAST_E;
            return PDL_BAD_PARAM;
    }

    prvPdlCpuType = cpuType;

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlCpuFamilyTypeGet ( OUT PDL_CPU_FAMILY_TYPE_ENT * cpuFamilyTypePtr )
 *
 * @brief   Get cpu family type
 *
 * @param [out] cpuFamilyTypePtr    cpu family type.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlCpuFamilyTypeGet (
    OUT PDL_CPU_FAMILY_TYPE_ENT          * cpuFamilyTypePtr
)
{
    if (prvPdlCpuType == PDL_CPU_TYPE_LAST_E)
        return PDL_NOT_INITIALIZED;

    * cpuFamilyTypePtr = prvPdlCpuFamilyType;

    return PDL_OK;
}

/* ***************************************************************************
* FUNCTION NAME: pdlCpuNumOfUsbDevicesGet
*
* DESCRIPTION:   Get number of USB devices on board
*
* PARAMETERS:
*
*****************************************************************************/

PDL_STATUS pdlCpuNumOfUsbDevicesGet (
    OUT UINT_32          * numOfUsbDevicesPtr
)
{
    if (numOfUsbDevicesPtr == NULL)
        return PDL_BAD_PTR;

    if (prvPdlNumOfUsbDevicesConfigured == FALSE) {
        *numOfUsbDevicesPtr = 0;
        return PDL_OK;
    }

    * numOfUsbDevicesPtr = prvPdlNumOfUsbDevices;

    return PDL_OK;
}

/*$ END OF pdlCpuNumOfUsbDevicesGet */


/**
 * @fn  PDL_STATUS prvPdlCpuInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlRootId )
 *
 * @brief   Init cpu module Create CPU DB and initialize
 *
 * @param [in]  xmlRootId   Xml id.
 *
 * @return  PDL_STATUS.
 */

/**
 * @fn  PDL_STATUS pdlCpuDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Cpu debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlCpuDebugSet (
    IN  BOOLEAN             state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlCpuDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlCpuDebugSet */

/* ***************************************************************************
* FUNCTION NAME: pdlCpuSdmaMapDbGetFirst
*
* DESCRIPTION:
*
* PARAMETERS:
*
*
*****************************************************************************/

PDL_STATUS pdlCpuSdmaMapDbGetFirst(
    OUT PDL_CPU_SDMA_MAP_INFO_STC     * cpuSdmaMapInfoPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                   pdlStatus;
    PDL_CPU_SDMA_MAP_INFO_STC                  * tempCpuSdmaMapInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (cpuSdmaMapInfoPtr == NULL) {
        return PDL_BAD_PTR;
    }
    pdlStatus = prvPdlDbGetFirst (&prvPdlCpuSdmaMapDb, (void**) &tempCpuSdmaMapInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);
    memcpy (cpuSdmaMapInfoPtr, tempCpuSdmaMapInfoPtr, sizeof(PDL_CPU_SDMA_MAP_INFO_STC));
    return PDL_OK;
}
/*$ END OF pdlCpuSdmaMapDbGetFirst */

/* ***************************************************************************
* FUNCTION NAME: pdlCpuSdmaMapDbGetFirst
*
* DESCRIPTION:
*
* PARAMETERS:
*
*
*****************************************************************************/

PDL_STATUS pdlCpuSdmaMapDbGetNext(
    IN  PDL_CPU_SDMA_MAP_INFO_STC     * cpuSdmaMapInfoPtr,
    OUT PDL_CPU_SDMA_MAP_INFO_STC     * cpuSdmaMapNextInfoPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                   pdlStatus;
    PDL_CPU_SDMA_MAP_INFO_STC                  * tempCpuSdmaMapInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (cpuSdmaMapInfoPtr == NULL || cpuSdmaMapNextInfoPtr == NULL) {
        return PDL_BAD_PTR;
    }
    pdlStatus = prvPdlDbGetNext(&prvPdlCpuSdmaMapDb, &cpuSdmaMapInfoPtr->key, (void **)&tempCpuSdmaMapInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);
    memcpy (cpuSdmaMapNextInfoPtr, tempCpuSdmaMapInfoPtr, sizeof(PDL_CPU_SDMA_MAP_INFO_STC));
    return PDL_OK;
}
/*$ END OF pdlCpuSdmaMapDbGetNext */

PDL_STATUS prvPdlCpuInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlRootId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                            pdlTagStr[PDL_XML_MAX_TAG_LEN];
    PDL_STATUS                      pdlStatus;
    XML_PARSER_RET_CODE_TYP         xmlStatus, xmlStatus2;
    UINT_32                         maxSize, value;
    XML_PARSER_NODE_DESCRIPTOR_TYP  xmlCpuId, xmlCpuMapId, xmlCpuMapListId, xmlCpuMapInstanceId, xmlUsbDevNumId;
    PDL_CPU_SDMA_MAP_KEY_STC        cpuSdmaMapKey;
    PDL_CPU_SDMA_MAP_INFO_STC       cpuSdmaMapInfo;
    PDL_CPU_SDMA_MAP_INFO_STC     * cpuSdmaMapInfoPtr = NULL;
    PDL_DB_PRV_ATTRIBUTES_STC       dbAttributes;

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

    xmlStatus = xmlParserFindByName (xmlRootId, "CPU-info", &xmlCpuId);
    /* section is not mandatory */
    if (xmlStatus == XML_PARSER_RET_CODE_NOT_FOUND) {
        xmlStatus = xmlParserFindByName (xmlRootId, "CPU-info-current", &xmlCpuId);
        if (xmlStatus == XML_PARSER_RET_CODE_NOT_FOUND) {
            return PDL_OK;
        }
    }
    XML_CHECK_STATUS(xmlStatus);

    /* check if num of usb devices info exist */
    xmlStatus = xmlParserFindByName (xmlCpuId, "num-of-usb-devices", &xmlUsbDevNumId);
    if (xmlStatus == XML_PARSER_RET_CODE_OK) {
        maxSize = sizeof(prvPdlNumOfUsbDevices);
        xmlStatus = xmlParserGetValue(xmlUsbDevNumId, &maxSize, &prvPdlNumOfUsbDevices);
        XML_CHECK_STATUS(xmlStatus);
        prvPdlNumOfUsbDevicesConfigured = TRUE;
    }

    xmlStatus = xmlParserGetFirstChild (xmlCpuId, &xmlCpuId);
    XML_CHECK_STATUS(xmlStatus);

    xmlStatus = xmlParserFindByName (xmlCpuId, "cpu-type", &xmlCpuId);
    XML_CHECK_STATUS(xmlStatus);

    maxSize = PDL_XML_MAX_TAG_LEN;
    xmlStatus = xmlParserGetValue(xmlCpuId, &maxSize, &pdlTagStr);
    XML_CHECK_STATUS(xmlStatus);

    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_CPU_TYPE_TYPE_E, pdlTagStr, &value);
    PDL_CHECK_STATUS(pdlStatus);
    prvPdlCpuType = (PDL_CPU_TYPE_ENT)value;

    switch (prvPdlCpuType)
    {
        /* MSYS Family */
        case PDL_CPU_TYPE_XP_EMBEDDED_E:
            prvPdlCpuFamilyType = PDL_CPU_FAMILY_TYPE_MSYS_E;
            break;
        case PDL_CPU_TYPE_ARMADA_MV78130_E:
        case PDL_CPU_TYPE_ARMADA_MV78160_E:
        case PDL_CPU_TYPE_ARMADA_MV78230_E:
        case PDL_CPU_TYPE_ARMADA_MV78260_E:
        case PDL_CPU_TYPE_ARMADA_MV78460_E:
            prvPdlCpuFamilyType = PDL_CPU_FAMILY_TYPE_AXP_E;
            break;
        case PDL_CPU_TYPE_ARMADA_88F6810_E:
        case PDL_CPU_TYPE_ARMADA_88F6811_E:
        case PDL_CPU_TYPE_ARMADA_88F6820_E:
        case PDL_CPU_TYPE_ARMADA_88F6821_E:
        case PDL_CPU_TYPE_ARMADA_88F6W21_E:
        case PDL_CPU_TYPE_ARMADA_88F6828_E:
            prvPdlCpuFamilyType = PDL_CPU_FAMILY_TYPE_A38X_E;
            break;
        default:
            prvPdlCpuType = PDL_CPU_TYPE_LAST_E;
            return PDL_BAD_PARAM;
    }

    /* parse cpu sdma port mapping */
    xmlStatus = xmlParserFindByName (xmlRootId, "CPU-SDMA-port-mapping", &xmlCpuMapId);
    /* section is not mandatory */
    if (xmlStatus == XML_PARSER_RET_CODE_OK) {
        dbAttributes.listAttributes.keySize = sizeof(PDL_CPU_SDMA_MAP_KEY_STC);
        dbAttributes.listAttributes.entrySize = sizeof(PDL_CPU_SDMA_MAP_INFO_STC);
        pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                    &dbAttributes,
                                    &prvPdlCpuSdmaMapDb);
        PDL_CHECK_STATUS(pdlStatus);
        xmlStatus = xmlParserGetFirstChild (xmlCpuMapId, &xmlCpuMapListId);
        /* go over all buttons */
        while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
            XML_CHECK_STATUS(xmlStatus);
            if (xmlParserIsEqualName (xmlCpuMapListId, "cpu-sdma-port-list") == XML_PARSER_RET_CODE_OK) {
                xmlStatus2 = xmlParserGetFirstChild (xmlCpuMapListId, &xmlCpuMapInstanceId);
                memset (&cpuSdmaMapInfo, 0, sizeof (cpuSdmaMapInfo));
                while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                    XML_CHECK_STATUS(xmlStatus2);
                    if (xmlParserIsEqualName (xmlCpuMapInstanceId, "cpu-sdma-port-index") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(cpuSdmaMapKey.index);
                        xmlStatus = xmlParserGetValue (xmlCpuMapInstanceId, &maxSize, &cpuSdmaMapKey.index);
                        XML_CHECK_STATUS(xmlStatus);
                    }
                    else if (xmlParserIsEqualName (xmlCpuMapInstanceId, "pp-device-number") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(cpuSdmaMapKey.index);
                        xmlStatus = xmlParserGetValue (xmlCpuMapInstanceId, &maxSize, &cpuSdmaMapInfo.dev);
                        XML_CHECK_STATUS(xmlStatus);
                    }
                    else if (xmlParserIsEqualName (xmlCpuMapInstanceId, "pp-port-number") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(cpuSdmaMapKey.index);
                        xmlStatus = xmlParserGetValue (xmlCpuMapInstanceId, &maxSize, &cpuSdmaMapInfo.macPort);
                        XML_CHECK_STATUS(xmlStatus);
                    }
                    else if (xmlParserIsEqualName (xmlCpuMapInstanceId, "logical-port-number") == XML_PARSER_RET_CODE_OK) {
                        maxSize = sizeof(cpuSdmaMapKey.index);
                        xmlStatus = xmlParserGetValue (xmlCpuMapInstanceId, &maxSize, &cpuSdmaMapInfo.logicalPort);
                        XML_CHECK_STATUS(xmlStatus);
                    }
                    else{
                        return PDL_XML_PARSE_ERROR;
                    }
                    xmlStatus2 = xmlParserGetNextSibling(xmlCpuMapInstanceId, &xmlCpuMapInstanceId);
                }
            }
            else {
                return PDL_XML_PARSE_ERROR;
            }
            cpuSdmaMapInfo.key.index = cpuSdmaMapKey.index;
            pdlStatus = prvPdlDbAdd (&prvPdlCpuSdmaMapDb, (void*) &cpuSdmaMapKey, (void*) &cpuSdmaMapInfo, (void**) &cpuSdmaMapInfoPtr);
            PDL_CHECK_STATUS(pdlStatus);
            xmlStatus = xmlParserGetNextSibling (xmlCpuMapListId, &xmlCpuMapListId);
        }
    }
    return PDL_OK;
}
