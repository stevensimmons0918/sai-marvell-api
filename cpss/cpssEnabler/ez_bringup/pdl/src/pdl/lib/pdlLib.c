/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlLib.c
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
 * @brief Platform driver layer - Lib related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <stdarg.h>
#include <pdl/init/pdlInit.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/lib/private/prvPdlLibLinkedList.h>
#include <pdl/lib/private/prvPdlLibArray.h>
#include <pdl/xml/xmlParser.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <pdl/sensor/private/prvPdlSensor.h>
#include <pdl/led/private/prvPdlLed.h>
#include <pdl/fan/private/prvPdlFan.h>

#ifndef _WIN32
int vsnprintf (char * s, size_t n, const char * format, va_list arg );
#endif /*_WIN32*/

#define MIN(a, b)       ((a) < (b) ? (a) : (b))
/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/
UINT_32               prvPdlLibMemConsumption = 0 ;

/** @brief   The pdl prv database function array[ pdl database prv type last e] */
static PDL_DB_PRV_FUNC_STC pdlPrvDbFuncArray[PDL_DB_PRV_TYPE_LAST_E] = {{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL}};
/** @brief   The prv pdl operating system callbacks pointer */
static PDL_OS_CALLBACK_API_STC  prvPdlOsCallbacks = {NULL,NULL,NULL,NULL,
                        {{NULL,NULL}},
                        NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
                        {NULL,NULL,NULL,NULL,NULL}};
static PDL_OS_CALLBACK_API_STC * prvPdlOsCallbacksPtr = &prvPdlOsCallbacks;
static PDL_OS_INIT_TYPE_ENT prvPdlLibCurrentInitType = PDL_OS_INIT_TYPE_FULL_E;

extern PDL_STATUS iDbgPdlLoggerPdlDebugTextAdd(IN char * format, ...);

/** @brief   The prv pdl error status string to enum pairs[ pdl port speed last e] */
static PRV_PDL_LIB_STR_TO_ENUM_STC prvPdlErrorStatusStrToEnumPairs[] = {
    {"PDL_ERROR"                      ,   PDL_ERROR                 },
    {"PDL_OK"                         ,   PDL_OK                    },
    {"PDL_FAIL"                       ,   PDL_FAIL                  },
    {"PDL_BAD_VALUE"                  ,   PDL_BAD_VALUE             },
    {"PDL_OUT_OF_RANGE"               ,   PDL_OUT_OF_RANGE          },
    {"PDL_BAD_PARAM"                  ,   PDL_BAD_PARAM             },
    {"PDL_BAD_PTR"                    ,   PDL_BAD_PTR               },
    {"PDL_BAD_SIZE"                   ,   PDL_BAD_SIZE              },
    {"PDL_BAD_STATE"                  ,   PDL_BAD_STATE             },
    {"PDL_SET_ERROR"                  ,   PDL_SET_ERROR             },
    {"PDL_GET_ERROR"                  ,   PDL_GET_ERROR             },
    {"PDL_CREATE_ERROR"               ,   PDL_CREATE_ERROR          },
    {"PDL_NOT_FOUND"                  ,   PDL_NOT_FOUND             },
    {"PDL_NO_MORE"                    ,   PDL_NO_MORE               },
    {"PDL_NO_SUCH"                    ,   PDL_NO_SUCH               },
    {"PDL_TIMEOUT"                    ,   PDL_TIMEOUT               },
    {"PDL_NO_CHANGE"                  ,   PDL_NO_CHANGE             },
    {"PDL_NOT_SUPPORTED"              ,   PDL_NOT_SUPPORTED         },
    {"PDL_NOT_IMPLEMENTED"            ,   PDL_NOT_IMPLEMENTED       },
    {"PDL_NOT_INITIALIZED"            ,   PDL_NOT_INITIALIZED       },
    {"PDL_NO_RESOURCE"                ,   PDL_NO_RESOURCE           },
    {"PDL_FULL"                       ,   PDL_FULL                  },
    {"PDL_EMPTY"                      ,   PDL_EMPTY                 },
    {"PDL_INIT_ERROR"                 ,   PDL_INIT_ERROR            },
    {"PDL_NOT_READY"                  ,   PDL_NOT_READY             },
    {"PDL_ALREADY_EXIST"              ,   PDL_ALREADY_EXIST         },
    {"PDL_OUT_OF_CPU_MEM"             ,   PDL_OUT_OF_CPU_MEM        },
    {"PDL_ABORTED"                    ,   PDL_ABORTED               },
    {"PDL_NOT_APPLICABLE_DEVICE"      ,   PDL_NOT_APPLICABLE_DEVICE },
    {"PDL_UNFIXABLE_ECC_ERROR"        ,   PDL_UNFIXABLE_ECC_ERROR   },
    {"PDL_UNFIXABLE_BIST_ERROR"       ,   PDL_UNFIXABLE_BIST_ERROR  },
    {"PDL_CHECKSUM_ERROR"             ,   PDL_CHECKSUM_ERROR        },
    {"PDL_XML_PARSE_ERROR"            ,   PDL_XML_PARSE_ERROR       },
    {"PDL_MEM_INIT_ERROR"             ,   PDL_MEM_INIT_ERROR        },
    {"PDL_I2C_ERROR"                  ,   PDL_I2C_ERROR             },

};
/** @brief   The prv pdl port speed string to enum */
static PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC prvPdlErrorStatusStrToEnum = {prvPdlErrorStatusStrToEnumPairs, sizeof(prvPdlErrorStatusStrToEnumPairs)/sizeof(PRV_PDL_LIB_STR_TO_ENUM_STC)};



/** @brief   The xml error status string to enum pairs[ pdl port speed last e] */
static PRV_PDL_LIB_STR_TO_ENUM_STC prvXmlErrorStatusStrToEnumPairs[] = {
    {"XML_CODE_OK"              ,   XML_PARSER_RET_CODE_OK             },
    {"XML_CODE_ERROR"           ,   XML_PARSER_RET_CODE_ERROR          },
    {"XML_CODE_NOT_FOUND"       ,   XML_PARSER_RET_CODE_NOT_FOUND      },
    {"XML_CODE_NO_MEM"          ,   XML_PARSER_RET_CODE_NO_MEM         },
    {"XML_CODE_WRONG_PARAM"     ,   XML_PARSER_RET_CODE_WRONG_PARAM    },
    {"XML_CODE_WRONG_FORMAT"    ,   XML_PARSER_RET_CODE_WRONG_FORMAT   },
    {"XML_CODE_BAD_SIZE"        ,   XML_PARSER_RET_CODE_BAD_SIZE       },

};
/** @brief   The xml error string to enum */
static PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC prvXmlErrorStatusStrToEnum = {prvXmlErrorStatusStrToEnumPairs, sizeof(prvXmlErrorStatusStrToEnumPairs)/sizeof(PRV_PDL_LIB_STR_TO_ENUM_STC)};

/** @brief   The debug flags */
BOOLEAN      prvPdlInitDone = FALSE;
BOOLEAN      prvPdlInitDebugFlag = TRUE;
BOOLEAN      prvPdlBtnDebugFlag = TRUE;
BOOLEAN      prvPdlPpDebugFlag = TRUE;
BOOLEAN      prvPdlSfpDebugFlag = TRUE;
BOOLEAN      prvPdlSerdesDebugFlag = TRUE;
BOOLEAN      prvPdlOobPortDebugFlag = TRUE;
BOOLEAN      prvPdlFanDebugFlag = TRUE;
BOOLEAN      prvPdlSensorDebugFlag = TRUE;
BOOLEAN      prvPdlPowerDebugFlag = TRUE;
BOOLEAN      prvPdlPhyDebugFlag = TRUE;
BOOLEAN      prvPdlLedDebugFlag = TRUE;
BOOLEAN      prvPdlLibDebugFlag = TRUE;
BOOLEAN      prvPdlCpuDebugFlag = TRUE;
BOOLEAN      prvPdlFanControllerDebugFlag = TRUE;
BOOLEAN      prvPdlInterfaceDebugFlag = TRUE;
BOOLEAN      prvPdlI2CInterfaceMuxDebugFlag = TRUE;

UINT_32  prvPdlSpeedEnumToValue[PDL_PORT_SPEED_LAST_E] = {
    10,
    100,
    1000,
    2500,
    5000,
    10000,
    20000,
    24000,
    25000,
    40000,
    50000,
    100000
};

static char     prvPdlLibBoardDescStr[PDL_XML_BOARD_DESC_LEN] = {0};
static BOOLEAN  prvPdlLibBoardDescExist = FALSE;

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)                                                                   \
    if (__pdlStatus != PDL_OK) {                                                                        \
    if (prvPdlLibDebugFlag) {                                                                           \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__pdlStatus,PDL_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __pdlStatus;                                                                                 \
    }
/*macro to check the xml status and debug prints if debug flag on*/
#undef XML_CHECK_STATUS
#define XML_CHECK_STATUS(__xmlStatus)                                                                   \
    if (__xmlStatus != PDL_OK) {                                                                        \
    if (prvPdlLibDebugFlag) {                                                                           \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__xmlStatus,XML_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __xmlStatus;                                                                                 \
    }

/**
 * @fn  PDL_STATUS prvPdlstrcicmp
 *
 * @brief   case insensitive string compate
 *
 * @param [in]  a           first string
 * @param [out] b           second string
 *
 * @return  0               strings are identical (case-ignored)
 * @return  anything else   strings aren't identical
 */

UINT_32 prvPdlStrcicmp(
    IN  char const                  * a,
    IN  char const                  * b
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_32                           c;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    for (;; a++, b++) {
        c = tolower(*a) - tolower(*b);
        if (c != 0 || !*a)
            return c;
    }
    return 0;
}
/*$ END OF prvPdlStrcicmp */

/**
 * @fn  PDL_STATUS pdlLibConvertEnumValueToString ( IN XML_PARSER_ENUM_ID_ENT enumId, IN UINT_32 intValue, OUT char ** strValuePtr )
 *
 * @brief   Convert enum int to string value.
 *
 * @param [in]  enumId          Enum identification.
 * @param [in]  intValue        enum int value to convert.
 * @param [out] strValuePtr     pointer to string value to return.
 *
 * @return  PDL_BAD_PARAM       if invalid id or pointer was supplied.
 * @return  PDL_NOT_INITIALIZED if database wasn't initialized.
 * @return  PDL_NOT_FOUND       if no match was found.
 *
 */
PDL_STATUS pdlLibConvertEnumValueToString (
    IN XML_PARSER_ENUM_ID_ENT   enumId,
    IN UINT_32                  intValue,
    OUT char                 ** strValuePtr
)
{
    XML_PARSER_RET_CODE_TYP xml_ret_code;

    if ((enumId >= XML_PARSER_ENUM_ID_LAST_E) || (strValuePtr == NULL))
        return PDL_BAD_PARAM;

    xml_ret_code = prvXmlParserConvertEnumValueToString(enumId, intValue + 1, strValuePtr);

    if (xml_ret_code == XML_PARSER_RET_CODE_NOT_INITIALIZED)
        return PDL_NOT_INITIALIZED;
    else if (xml_ret_code == XML_PARSER_RET_CODE_NOT_FOUND)
        return PDL_NOT_FOUND;
    else if (xml_ret_code != XML_PARSER_RET_CODE_OK)
        return PDL_ERROR;

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS prvPdlLibValidateEnumValue ( IN XML_PARSER_ENUM_ID_ENT enumId, IN UINT_32 intValue )
 *
 * @brief   Validate enum int value existence.
 *
 * @param [in]  enumId          Enum identification.
 * @param [in]  intValue        enum int value to convert.
 *
 * @return  PDL_BAD_PARAM       if invalid id or pointer was supplied.
 * @return  PDL_NOT_INITIALIZED if database wasn't initialized.
 * @return  PDL_NOT_FOUND       if no match was found.
 *
 */
PDL_STATUS prvPdlLibValidateEnumValue (
    IN XML_PARSER_ENUM_ID_ENT   enumId,
    IN UINT_32                  intValue
)
{
    XML_PARSER_RET_CODE_TYP xml_ret_code;

    if (enumId >= XML_PARSER_ENUM_ID_LAST_E)
        return PDL_BAD_PARAM;

    xml_ret_code = prvXmlParserConvertEnumValueToString(enumId, intValue + 1, NULL);

    if (xml_ret_code == XML_PARSER_RET_CODE_NOT_INITIALIZED)
        return PDL_NOT_INITIALIZED;
    else if (xml_ret_code == XML_PARSER_RET_CODE_NOT_FOUND)
        return PDL_NOT_FOUND;
    else if (xml_ret_code != XML_PARSER_RET_CODE_OK)
        return PDL_ERROR;

    return PDL_OK;
}

/**
 * @fn  PDL_STATUSpdlLibConvertEnumStringToValue ( IN XML_PARSER_ENUM_ID_ENT enumId, IN char * strValuePtr, OUT UINT_32 * intValuePtr )
 *
 * @brief   Convert enum string to int value.
 *
 * @param [in]  enumId          Enum identification.
 * @param [in]  intValuePtr     pointer to string value to convert.
 * @param [out] strValuePtr     enum int value to return.
 *
 * @return  PDL_BAD_PARAM       if invalid id or pointer was supplied.
 * @return  PDL_NOT_INITIALIZED if database wasn't initialized.
 * @return  PDL_NOT_FOUND       if no match was found.
 *
 */
PDL_STATUS pdlLibConvertEnumStringToValue (
    IN XML_PARSER_ENUM_ID_ENT   enumId,
    IN char                   * strValuePtr,
    OUT UINT_32               * intValuePtr
)
{
    XML_PARSER_RET_CODE_TYP xml_ret_code;
    UINT_32                 intValue;

    if ((enumId >= XML_PARSER_ENUM_ID_LAST_E) || (strValuePtr == NULL) || (intValuePtr == NULL))
        return PDL_BAD_PARAM;

    xml_ret_code = prvXmlParserConvertEnumStringToValue(enumId, strValuePtr, &intValue);

    if (xml_ret_code == XML_PARSER_RET_CODE_NOT_INITIALIZED)
        return PDL_NOT_INITIALIZED;
    else if (xml_ret_code == XML_PARSER_RET_CODE_NOT_FOUND)
        return PDL_NOT_FOUND;
    else if (xml_ret_code != XML_PARSER_RET_CODE_OK)
        return PDL_ERROR;

    if (intValue == 0)
        return PDL_NOT_FOUND;

    *intValuePtr = intValue - 1;

    return PDL_OK;
}

/**
 * @brief   Convert external enum string (non pdl) to int value.
 *
 * @param [in]  externalEnumDbArr           external db with enum information (built using xmlParserExternalEnumDbBuild)
 * @param [in]  externalEnumDbSize          external db size
 * @param [in]  enumId                      Enum identification.
 * @param [in]  strValuePtr                 pointer to string value to convert.
 * @param [out] intValuePtr                 enum int value to return.
 *
 * @return  PDL_BAD_PARAM       if invalid id or pointer was supplied.
 * @return  PDL_NOT_INITIALIZED if database wasn't initialized.
 * @return  PDL_NOT_FOUND       if no match was found.
 *
 */
PDL_STATUS pdlLibConvertExternalEnumStringToValue (
    IN XML_PARSER_ENUM_DB_STC          ** externalEnumDbArr,
    IN UINT_32                            externalEnumDbSize,
    IN XML_PARSER_ENUM_ID_ENT             enumId,
    IN char                             * strValuePtr,
    OUT UINT_32                         * intValuePtr
)
{
    XML_PARSER_RET_CODE_TYP xml_ret_code;
    UINT_32                 intValue;

    if ((enumId >= XML_PARSER_ENUM_ID_LAST_E) || (strValuePtr == NULL) || (intValuePtr == NULL))
        return PDL_BAD_PARAM;

    xml_ret_code = prvXmlParserConvertExternalEnumStringToValue(externalEnumDbArr, externalEnumDbSize, enumId, strValuePtr, &intValue);

    if (xml_ret_code == XML_PARSER_RET_CODE_NOT_INITIALIZED)
        return PDL_NOT_INITIALIZED;
    else if (xml_ret_code == XML_PARSER_RET_CODE_NOT_FOUND)
        return PDL_NOT_FOUND;
    else if (xml_ret_code != XML_PARSER_RET_CODE_OK)
        return PDL_ERROR;

    if (intValue == 0)
        return PDL_NOT_FOUND;

    *intValuePtr = intValue - 1;

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS prvPdlLibValidateEnumString ( IN XML_PARSER_ENUM_ID_ENT enumId, IN char * strValuePtr )
 *
 * @brief   Validate enum string value existence.
 *
 * @param [in]  enumId          Enum identification.
 * @param [in]  intValuePtr     pointer to string value to convert.
 *
 * @return  PDL_BAD_PARAM       if invalid id or pointer was supplied.
 * @return  PDL_NOT_INITIALIZED if database wasn't initialized.
 * @return  PDL_NOT_FOUND       if no match was found.
 *
 */
PDL_STATUS prvPdlLibValidateEnumString (
    IN XML_PARSER_ENUM_ID_ENT   enumId,
    IN char                   * strValuePtr
)
{
    XML_PARSER_RET_CODE_TYP xml_ret_code;

    if ((enumId >= XML_PARSER_ENUM_ID_LAST_E) || (strValuePtr == NULL))
        return PDL_BAD_PARAM;

    xml_ret_code = prvXmlParserConvertEnumStringToValue(enumId, strValuePtr, NULL);

    if (xml_ret_code == XML_PARSER_RET_CODE_NOT_INITIALIZED)
        return PDL_NOT_INITIALIZED;
    else if (xml_ret_code == XML_PARSER_RET_CODE_NOT_FOUND)
        return PDL_NOT_FOUND;
    else if (xml_ret_code != XML_PARSER_RET_CODE_OK)
        return PDL_ERROR;

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS prvPdlLibStrToEnumConvert( IN PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC * convertDbPtr, IN char * namePtr, OUT UINT_32 * valuePtr )
 *
 * @brief   Prv pdl library string to enum convert
 *
 * @param [in,out]  convertDbPtr    If non-null, the convert database pointer.
 * @param [in,out]  namePtr         If non-null, the name pointer.
 * @param [in,out]  valuePtr        If non-null, the value pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLibStrToEnumConvert(
    IN  PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC * convertDbPtr,
    IN  char                                * namePtr,
    OUT UINT_32                             * valuePtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_32 i;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (namePtr == NULL || valuePtr == NULL || convertDbPtr == NULL)
        return PDL_BAD_PTR;

    for (i = 0; i < convertDbPtr->size; i++)
    {
        if (prvPdlStrcicmp(namePtr, convertDbPtr->str2Enum[i].namePtr) == 0)
        {
            *valuePtr = convertDbPtr->str2Enum[i].value;
            return PDL_OK;
        }
    }

    return PDL_NOT_FOUND;
}

/*$ END OF prvPdlLibStrToEnumConvert */

/* ***************************************************************************
* FUNCTION NAME: prvPdlLibStrToEnumConvert
*
* DESCRIPTION:   Convert between value (enum) to string according to data found in
*                convertDbPtr. if enum wasn't found in db PDL_NOT_FOUND will be returned
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlLibEnumToStrConvert( IN PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC * convertDbPtr, IN UINT_32 value, OUT char ** namePtrPtr )
 *
 * @brief   Prv pdl library enum to string convert
 *
 * @param [in,out]  convertDbPtr    If non-null, the convert database pointer.
 * @param           value           The value.
 * @param [in,out]  namePtrPtr      If non-null, the name pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLibEnumToStrConvert(
    IN  PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC * convertDbPtr,
    IN  UINT_32                               value,
    OUT char                               ** namePtrPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_32 i;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (namePtrPtr == NULL || convertDbPtr == NULL)
        return PDL_BAD_PTR;

    for (i = 0; i < convertDbPtr->size; i++)
    {
        if (convertDbPtr->str2Enum[i].value == value)
        {
            *namePtrPtr = convertDbPtr->str2Enum[i].namePtr;
            return PDL_OK;
        }
    }

    return PDL_NOT_FOUND;
}

/*$ END OF prvPdlLibStrToEnumConvert */


/*****************************************************************************
* FUNCTION NAME: prvPdlDbCreate
*
* DESCRIPTION:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlDbCreate ( IN PDL_DB_PRV_TYPE_ENT dbType, IN PDL_DB_PRV_ATTRIBUTES_STC * dbAttributes, OUT PDL_DB_PRV_STC * dbPtr )
 *
 * @brief   Prv pdl database create
 *
 * @param           dbType          Type of the database.
 * @param [in,out]  dbAttributes    If non-null, the database attributes.
 * @param [in,out]  dbPtr           If non-null, the database pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlDbCreate (
    IN  PDL_DB_PRV_TYPE_ENT         dbType,
    IN  PDL_DB_PRV_ATTRIBUTES_STC * dbAttributes,
    OUT PDL_DB_PRV_STC            * dbPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (dbAttributes == NULL || dbPtr == NULL || dbType >= PDL_DB_PRV_TYPE_LAST_E){
        return PDL_BAD_PARAM;
    }

    if (pdlPrvDbFuncArray[dbType].dbCreateFun == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return pdlPrvDbFuncArray[dbType].dbCreateFun (dbAttributes, dbPtr);
}
/*$ END OF prvPdlDbCreate */

/*****************************************************************************
* FUNCTION NAME: prvPdlDbAdd
*
* DESCRIPTION:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlDbAdd ( IN PDL_DB_PRV_STC * dbPtr, IN void * keyPtr, IN void * entryPtr, OUT void ** outEntryPtrPtr )
 *
 * @brief   Prv pdl database add
 *
 * @param [in,out]  dbPtr           If non-null, the database pointer.
 * @param [in,out]  keyPtr          If non-null, the key pointer.
 * @param [in,out]  entryPtr        If non-null, the entry pointer.
 * @param [in,out]  outEntryPtrPtr  If non-null, the out entry pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlDbAdd (
    IN  PDL_DB_PRV_STC        * dbPtr,
    IN  void                  * keyPtr,
    IN  void                  * entryPtr,
    OUT void                 ** outEntryPtrPtr
)
{
/*!********s********************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (dbPtr == NULL || entryPtr == NULL || dbPtr->dbType >= PDL_DB_PRV_TYPE_LAST_E){
        return PDL_BAD_PARAM;
    }

    if (pdlPrvDbFuncArray[dbPtr->dbType].dbAddFun == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return pdlPrvDbFuncArray[dbPtr->dbType].dbAddFun (dbPtr, keyPtr, entryPtr, outEntryPtrPtr);

}
/*$ END OF prvPdlDbAdd */

/*****************************************************************************
* FUNCTION NAME: prvPdlDbGetNumOfEntries
*
* DESCRIPTION:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlDbGetNumOfEntries ( IN PDL_DB_PRV_STC * dbPtr, OUT UINT_32 * numPtr )
 *
 * @brief   Prv pdl database get number of entries
 *
 * @param [in,out]  dbPtr   If non-null, the database pointer.
 * @param [in,out]  numPtr  If non-null, number of pointers.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlDbGetNumOfEntries (
    IN  PDL_DB_PRV_STC        * dbPtr,
    OUT UINT_32               * numPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (dbPtr == NULL || numPtr == NULL || dbPtr->dbType >= PDL_DB_PRV_TYPE_LAST_E){
        return PDL_BAD_PARAM;
    }

    if (pdlPrvDbFuncArray[dbPtr->dbType].dbGetNumOfEntriesFun == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return pdlPrvDbFuncArray[dbPtr->dbType].dbGetNumOfEntriesFun (dbPtr, numPtr);
}
/*$ END OF prvPdlDbGetNumOfEntries */

/*****************************************************************************
* FUNCTION NAME: prvPdlDbFind
*
* DESCRIPTION:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlDbFind ( IN PDL_DB_PRV_STC * dbPtr, IN void * keyPtr, OUT void ** outEntryPtrPtr )
 *
 * @brief   Prv pdl database find
 *
 * @param [in,out]  dbPtr           If non-null, the database pointer.
 * @param [in,out]  keyPtr          If non-null, the key pointer.
 * @param [in,out]  outEntryPtrPtr  If non-null, the out entry pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlDbFind (
    IN  PDL_DB_PRV_STC        * dbPtr,
    IN  void                  * keyPtr,
    OUT void                 ** outEntryPtrPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (dbPtr == NULL || keyPtr == NULL || dbPtr->dbType >= PDL_DB_PRV_TYPE_LAST_E){
        return PDL_BAD_PARAM;
    }

    if (pdlPrvDbFuncArray[dbPtr->dbType].dbFindFun == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return pdlPrvDbFuncArray[dbPtr->dbType].dbFindFun (dbPtr, keyPtr, outEntryPtrPtr);

}
/*$ END OF prvPdlDbFind */

/*****************************************************************************
* FUNCTION NAME: prvPdlDbGetNext
*
* DESCRIPTION:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlDbGetNext ( IN PDL_DB_PRV_STC * dbPtr, IN void * keyPtr, OUT void ** outEntryPtrPtr )
 *
 * @brief   Prv pdl database get next
 *
 * @param [in,out]  dbPtr           If non-null, the database pointer.
 * @param [in,out]  keyPtr          If non-null, the key pointer.
 * @param [in,out]  outEntryPtrPtr  If non-null, the out entry pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlDbGetNext (
    IN  PDL_DB_PRV_STC        * dbPtr,
    IN  void                  * keyPtr,
    OUT void                 ** outEntryPtrPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (dbPtr == NULL || dbPtr->dbType >= PDL_DB_PRV_TYPE_LAST_E){
        return PDL_BAD_PARAM;
    }

    if (pdlPrvDbFuncArray[dbPtr->dbType].dbGetNextFun == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return pdlPrvDbFuncArray[dbPtr->dbType].dbGetNextFun (dbPtr, keyPtr, outEntryPtrPtr);

}
/*$ END OF prvPdlDbGetNext */


/*****************************************************************************
* FUNCTION NAME: prvPdlDbGetNextKey
*
* DESCRIPTION:
*
*
*****************************************************************************/

PDL_STATUS prvPdlDbGetNextKey (
    IN  PDL_DB_PRV_STC        * dbPtr,
    IN  void                  * keyPtr,
    OUT void                  * nextkeyPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (dbPtr == NULL || dbPtr->dbType >= PDL_DB_PRV_TYPE_LAST_E){
        return PDL_BAD_PARAM;
    }

    if (pdlPrvDbFuncArray[dbPtr->dbType].dbGetNextKeyFun == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return pdlPrvDbFuncArray[dbPtr->dbType].dbGetNextKeyFun (dbPtr, keyPtr, nextkeyPtr);

}
/*$ END OF prvPdlDbGetNext */

/**
 * @fn  PDL_STATUS prvPdlDbGetFirst ( IN PDL_DB_PRV_STC * dbPtr, OUT void ** outEntryPtrPtr )
 *
 * @brief   find first entry in db
 *
 * @param [in,out]  dbPtr           If non-null, the database pointer.
 * @param [in,out]  outEntryPtrPtr  If non-null, the out entry pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlDbGetFirst (
    IN  PDL_DB_PRV_STC        * dbPtr,
    OUT void                 ** outEntryPtrPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (dbPtr == NULL || outEntryPtrPtr == NULL || dbPtr->dbType >= PDL_DB_PRV_TYPE_LAST_E){
        return PDL_BAD_PARAM;
    }

    if (pdlPrvDbFuncArray[dbPtr->dbType].dbGetFirstFun == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return pdlPrvDbFuncArray[dbPtr->dbType].dbGetFirstFun (dbPtr, outEntryPtrPtr);
}
/*$ END OF prvPdlDbGetFirst */

/*****************************************************************************
* FUNCTION NAME: prvPdlDbDestroy
*
* DESCRIPTION:   Destroy all memory consumed by DB
*
*
*****************************************************************************/

PDL_STATUS prvPdlDbDestroy (
    IN  PDL_DB_PRV_STC            * dbPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (dbPtr == NULL || dbPtr->dbType >= PDL_DB_PRV_TYPE_LAST_E){
        return PDL_BAD_PTR;
    }

    if (pdlPrvDbFuncArray[dbPtr->dbType].dbDestroyFun == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return pdlPrvDbFuncArray[dbPtr->dbType].dbDestroyFun (dbPtr);
}
/*$ END OF prvPdlDbCreate */

/**
 * @fn  void * prvPdlOsMalloc ( IN UINT_32 size )
 *
 * @brief   malloc implementation
 *
 * @param [in]  size    memory size to allocate.
 *
 * @return  pointer to allocated memory or NULL if not possible.
 */

void * prvPdlOsMalloc (
    IN UINT_32                    size
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL)
        return NULL;
    prvPdlLibMemConsumption+=size;
    return prvPdlOsCallbacksPtr->mallocPtr(size);
}
/*$ END OF prvPdlOsMalloc */

/**
 * @fn  void * prvPdlOsCalloc ( IN UINT_32 numOfBlocks, IN UINT_32 blockSize )
 *
 * @brief   calloc implementation
 *
 * @param [in]  numOfBlocks number of memory blocks to allocate.
 * @param [in]  blockSize   memory block size to allocate.
 *
 * @return  pointer to allocated memory or NULL if not possible.
 */

void * prvPdlOsCalloc (
    IN UINT_32                  numOfBlocks,
    IN UINT_32                  blockSize
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    void * ptr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL)
        return NULL;

    ptr = prvPdlOsCallbacksPtr->mallocPtr(numOfBlocks*blockSize);
    if (ptr)
        memset(ptr, 0, numOfBlocks*blockSize);

    return ptr;
}
/*$ END OF prvPdlOsCalloc */

/**
 * @fn  void prvPdlOsFree ( IN void * ptr )
 *
 * @brief   free implementation
 *
 * @param [in]  ptr memory to free.
 */

void prvPdlOsFree (
    IN void         *   ptr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL)
        return;

    prvPdlOsCallbacksPtr->freePtr(ptr);
}
/*$ END OF prvPdlOsFree */

/**
 * @fn  void prvPdlOsPrintf ( IN char * format, ... )
 *
 * @brief   printf implementation
 *
 * @param [in]  format  format of string to print.
 * @param [in]  ...     additional arguments.
 */

void prvPdlOsPrintf (
    IN char * format,
    ...
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char      buffer[1024];
    va_list   argptr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->printStringPtr == NULL)
        return;

    va_start( argptr, format);
#ifndef _WIN32
    vsnprintf( buffer, sizeof(buffer), format, argptr);
#else
    vsnprintf_s( buffer, sizeof(buffer), sizeof(buffer) - 1, format, argptr);
#endif
    va_end( argptr);

    prvPdlOsCallbacksPtr->printStringPtr(buffer);
}
/*$ END OF prvPdlOsPrintf */

/* ***************************************************************************
* FUNCTION NAME: prvPdlOsSnprintf
*
* DESCRIPTION:
*
*
*****************************************************************************/

extern int prvPdlOsSnprintf (
    /*!     INPUTS:             */
    char                *str,
    size_t               size,
    const char          * format,
    ...
    /*!     OUTPUTS:            */
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/
  int       needed_buffer_len;
  UINT_32   needed_buffer_len_u32;
  char      buffer[1024];
  va_list   argptr;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                       */
/*!*************************************************************************/
    va_start( argptr, format);
#ifndef _WIN32
    needed_buffer_len = vsnprintf( buffer, sizeof(buffer), format, argptr);
#else
    needed_buffer_len = vsnprintf_s( buffer, sizeof(buffer), sizeof(buffer) - 1, format, argptr);
#endif
    va_end( argptr);
    if (needed_buffer_len > 1024) {
        return 0;
    }
    needed_buffer_len_u32 = (UINT_32) needed_buffer_len;
    /* also copy the '\0' at end of buffer to str */
    strncpy (str, buffer, MIN(size, needed_buffer_len_u32+1));
    if (needed_buffer_len_u32 + 1 > size) {
        str[size-1] = '\0';
    }
    return needed_buffer_len_u32;
}

/*$ END OF prvPdlOsSnprintf */


/**
 * @fn  BOOLEAN pdlStatusToString ( IN PDL_STATUS pdlStatus, OUT char ** pdlStatusStr)
 *
 * @brief   pdl status codes to string representation
 *
 * @param   pdlStatus             pdl status code
 * @param   pdlStatusStr          identifier for the status type.
 */

extern BOOLEAN pdlStatusToString (
    IN  PDL_STATUS    pdlStatus,
    OUT char        **pdlStatusStr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (pdlStatusStr == NULL)
        return FALSE;

    if (PDL_OK != prvPdlLibEnumToStrConvert(&prvPdlErrorStatusStrToEnum, (UINT_32)pdlStatus, pdlStatusStr))
        return FALSE;

    return TRUE;
}
/*$ END OF pdlStatusToString */

/**
 * @fn  void prvPdlStatusDebugLogHdr ( IN const char * func_name_PTR, IN const char * line_num, IN PDL_STATUS   status, IN UINT_32  pdlIdentifier)
 *
 * @brief   Prv pdl debug log
 *
 * @param   func_name_PTR          The function name pointer.
 * @param   line_num               Describes the format to use.
 * @param   status                 error code.
 * @param   pdlIdentifier          identifier for the status type.
 */

BOOLEAN prvPdlStatusDebugLogHdr (
    IN   const char * func_name_PTR,
    IN   UINT_32      line_num,
    IN   PDL_STATUS   status,
    IN   UINT_32      pdlIdentifier
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char    *     errorStr;
    PDL_STATUS   pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    /*convert status error code to string*/
    if(pdlIdentifier == PDL_STATUS_ERROR_PRINT_IDENTIFIER)
    {
        pdlStatus = (pdlStatusToString(status, &errorStr) == TRUE) ? PDL_OK : PDL_NOT_FOUND;
    }
    else
        pdlStatus = prvPdlLibEnumToStrConvert(&prvXmlErrorStatusStrToEnum, (UINT_32)status, &errorStr);
    PDL_CHECK_STATUS(pdlStatus);
    /* init not completed, debug is not ready yet*/
    if (prvPdlInitDone == FALSE) {
        prvPdlOsCallbacksPtr->printStringPtr("FUNCTION: %s, LINE: %d, ERROR: %s\n", func_name_PTR, line_num,errorStr);
    }
    else {
        if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->debugLogPtr == NULL)
            return FALSE;
        prvPdlOsCallbacksPtr->debugLogPtr("", "FUNCTION: %s, LINE: %d, ERROR: %s", func_name_PTR, line_num,errorStr);
    }
    iDbgPdlLoggerPdlDebugTextAdd("FUNCTION: %s, LINE: %d, ERROR: %s\n", func_name_PTR, line_num,errorStr);
    return TRUE;
}
/*$ END OF prvPdlDebugLogHdr */

/*****************************************************************************
* FUNCTION NAME: prvPdlDebugLogHdr
*
* DESCRIPTION:
*
*
*****************************************************************************/

/**
 * @fn  void prvPdlDebugLogHdr ( IN const char * func_name_PTR, IN   UINT_32 line_num )
 *
 * @brief   Prv pdl debug log
 *
 * @param   func_name_PTR          The function name pointer.
 * @param   line_num               Describes the format to use.
 */

BOOLEAN prvPdlDebugLogHdr (
    IN   const char * func_name_PTR,
    IN   UINT_32      line_num
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    /* init not completed, debug is not ready yet*/
    if (prvPdlInitDone == FALSE) {
        prvPdlOsCallbacksPtr->printStringPtr("FUNCTION: %s, LINE: %d ", func_name_PTR, line_num);
    }
    else {
        if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->debugLogPtr == NULL)
            return FALSE;
        prvPdlOsCallbacksPtr->debugLogPtr("", "FUNCTION: %s, LINE: %d ", func_name_PTR, line_num);
    }
    iDbgPdlLoggerPdlDebugTextAdd("FUNCTION: %s, LINE: %d", func_name_PTR, line_num);
    return TRUE;
}
/*$ END OF prvPdlDebugLogHdr */

/**
 * @fn  void prvPdlDebugRawLog ( IN const char * format, IN ... )
 *
 * @brief   Prv pdl debug log
 *
 * @param   func_name_PTR   The function name pointer.
 * @param   format          Describes the format to use.
 * @param   ...             The ...
 */

void prvPdlDebugRawLog (
    IN const char * func_name_PTR,
    IN UINT_32      line,
    IN const char * format,
    IN ...
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char      buffer[1024], buffer2[200];
    va_list   argptr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    va_start( argptr, format);
#ifndef _WIN32
    vsnprintf( buffer, sizeof(buffer), format, argptr);
#else
    vsnprintf_s( buffer, sizeof(buffer), sizeof(buffer) - 1, format, argptr);
#endif
    va_end( argptr);
    prvPdlOsSnprintf (buffer2, 1024, "%s Line[%d]", func_name_PTR, line);
    /* init not completed, debug is not ready yet*/
    if (prvPdlInitDone == FALSE) {
        prvPdlOsCallbacksPtr->printStringPtr("%s %s", buffer2, buffer);
    }
    else {
        if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->debugLogPtr == NULL)
            return;
        prvPdlOsCallbacksPtr->debugLogPtr(buffer2, buffer);
    }
    iDbgPdlLoggerPdlDebugTextAdd(buffer);
}
/*$ END OF prvPdlDebugLog */

/**
 * @fn  void prvPdlLock ( IN PDL_OS_LOCK_TYPE_ENT lockType )
 *
 * @brief   uses lock mechanism given by higher layer to provide mutual exclusion
 *
 * @param [in]      lockType        what are we protecting
 */

void prvPdlLock (
    IN PDL_OS_LOCK_TYPE_ENT         lockType
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->lockFuncArr[lockType].lockFun == NULL || lockType == PDL_OS_LOCK_TYPE_NONE_E)
        return;
    prvPdlOsCallbacksPtr->lockFuncArr[lockType].lockFun();
}
/*$ END OF prvPdlLock */

/**
 * @fn  void prvPdlUnlock ( IN PDL_OS_LOCK_TYPE_ENT lockType )
 *
 * @brief   uses lock mechanism given by higher layer to provide mutual exclusion
 *
 * @param [in]      lockType        what are we protecting
 */

void prvPdlUnlock (
    IN PDL_OS_LOCK_TYPE_ENT         lockType
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->lockFuncArr[lockType].unlockFun == NULL || lockType == PDL_OS_LOCK_TYPE_NONE_E)
        return;
    prvPdlOsCallbacksPtr->lockFuncArr[lockType].unlockFun();
}
/*$ END OF prvPdlUnlock */

/**
 * @fn  PDL_STATUS prvPdlCmRegRead
 *
 * @brief   Reads PP C&M register using given callback
 *
 * @param [in]      devIdx        device index to read register from
 * @param [in]      regAddr       register address
 * @param [in]      mask          mask to read
 * @param [out]     dataPtr       pointer to where data read will be stored
 */

PDL_STATUS prvPdlCmRegRead (
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    OUT GT_U32    * dataPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ppCmRegReadClbk == NULL)
        return PDL_NOT_IMPLEMENTED;
    return prvPdlOsCallbacksPtr->ppCmRegReadClbk(devIdx, regAddr, mask, dataPtr);
#else
    return PDL_OK;
#endif
}
/*$ END OF prvPdlCmRegRead */

/**
 * @fn  PDL_STATUS prvPdlCmRegWrite
 *
 * @brief   Writes PP C&M register using given callback
 *
 * @param [in]      devIdx        device index to write register to
 * @param [in]      regAddr       register address
 * @param [in]      mask          mask to read
 * @param [in]      data          data to write
 */

PDL_STATUS prvPdlCmRegWrite (
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    IN  GT_U32      data
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ppCmRegWriteClbk == NULL)
        return PDL_NOT_IMPLEMENTED;
    return prvPdlOsCallbacksPtr->ppCmRegWriteClbk(devIdx, regAddr, mask, data);
#else
    return PDL_OK;
#endif
}
/*$ END OF prvPdlCmRegWrite */

/**
 * @fn  PDL_STATUS prvPdlRegRead
 *
 * @brief   Reads PP register using given callback
 *
 * @param [in]      devIdx        device index to read register from
 * @param [in]      regAddr       register address
 * @param [in]      mask          mask to read
 * @param [out]     dataPtr       pointer to where data read will be stored
 */

PDL_STATUS prvPdlRegRead (
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    OUT GT_U32    * dataPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ppRegReadClbk == NULL)
        return PDL_NOT_IMPLEMENTED;
    return prvPdlOsCallbacksPtr->ppRegReadClbk(devIdx, regAddr, mask, dataPtr);
#else
    return PDL_OK;
#endif
}
/*$ END OF prvPdlRegRead */

/**
 * @fn  PDL_STATUS prvPdlRegWrite
 *
 * @brief   Writes PP register using given callback
 *
 * @param [in]      devIdx        device index to write register to
 * @param [in]      regAddr       register address
 * @param [in]      mask          mask to read
 * @param [in]      data          data to write
 */

PDL_STATUS prvPdlRegWrite (
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    IN  GT_U32      data
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ppRegWriteClbk == NULL)
        return PDL_NOT_IMPLEMENTED;
    return prvPdlOsCallbacksPtr->ppRegWriteClbk(devIdx, regAddr, mask, data);
#else
    return PDL_OK;
#endif
}
/*$ END OF prvPdlRegWrite */

/**
 * @fn  PDL_STATUS prvPdlSmiRegRead
 *
 * @brief   Reads SMI register using given callback
 *
 * @param [in]      devIdx        device index to read register from
 * @param [in]      portGroupsBmp bitmap of ports
 * @param [in]      smiInterface  smi interface (0..3)
 * @param [in]      smiAddr       smi adresss
 * @param [in]      phyPageSelReg phy page selected register
 * @param [in]      phyPage       phy page number
 * @param [in]      regAddr       register address
 * @param [out]     dataPtr       pointer to where data read will be stored
 */

PDL_STATUS prvPdlSmiRegRead (
    IN  GT_U8      devIdx,
    IN  GT_U32     portGroupsBmp,
    IN  GT_U32     smiInterface,
    IN  GT_U32     smiAddr,
    IN  GT_U8      phyPageSelReg,
    IN  GT_U8      phyPage,
    IN  GT_U32     regAddr,
    OUT GT_U16    *dataPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->smiRegReadClbk == NULL)
        return PDL_NOT_IMPLEMENTED;
    return prvPdlOsCallbacksPtr->smiRegReadClbk(devIdx, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, phyPage, regAddr, dataPtr);
}
/*$ END OF prvPdlSmiRegRead */

/**
 * @fn  PDL_STATUS prvPdlSmiRegWrite
 *
 * @brief   Reads SMI register using given callback
 *
 * @param [in]      devIdx        device index to write register to
 * @param [in]      portGroupsBmp bitmap of ports
 * @param [in]      smiInterface  smi interface (0..3)
 * @param [in]      smiAddr       smi adresss
 * @param [in]      phyPageSelReg phy page selected register
 * @param [in]      phyPage       phy page number
 * @param [in]      regAddr       register address
 * @param [out]     dataPtr       data to write
 */

PDL_STATUS prvPdlSmiRegWrite (
    IN  GT_U8      devIdx,
    IN  GT_U32     portGroupsBmp,
    IN  GT_U32     smiInterface,
    IN  GT_U32     smiAddr,
    IN  GT_U8      phyPageSelReg,
    IN  GT_U8      phyPage,
    IN  GT_U32     regAddr,
    IN  GT_U16     mask,
    IN  GT_U16     data
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    GT_U16          oldData, newData;
    PDL_STATUS      pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->smiRegWriteClbk == NULL || prvPdlOsCallbacksPtr->smiRegReadClbk == NULL)
        return PDL_NOT_IMPLEMENTED;
    pdlStatus = prvPdlOsCallbacksPtr->smiRegReadClbk(devIdx, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, phyPage, regAddr, &oldData);
    PDL_CHECK_STATUS(pdlStatus);
    newData = (~mask & oldData) | (mask & data);
    return prvPdlOsCallbacksPtr->smiRegWriteClbk(devIdx, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, phyPage, regAddr, newData);
}
/*$ END OF prvPdlSmiRegWrite */

/**
 * @fn  PDL_STATUS prvPdlSmiPpuLock
 *
 * @brief   Control the SMI MAC polling unit using given callback
 *
 * @param [in]      devIdx        device index
 * @param [in]      smiInterface  smi interface (0..3)
 * @param [in]      lock          stop/start smi auto polling unit
 * @param [out]     prevLockStatePtr previous state of the lock
 */

PDL_STATUS prvPdlSmiPpuLock (
    IN  GT_U8       devIdx,
    IN  GT_U32      smiInterface,
    IN  BOOLEAN     lock,
    OUT BOOLEAN    *prevLockStatePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->smiPpuLockClbk == NULL)
        return PDL_NOT_IMPLEMENTED;
    return prvPdlOsCallbacksPtr->smiPpuLockClbk(devIdx, smiInterface, lock, prevLockStatePtr);
}
/*$ END OF prvPdlSmiPpuLock */

/**
 * @fn  PDL_STATUS prvPdlXsmiRegRead
 *
 * @brief   Reads XSMI register using given callback
 *
 * @param [in]      devIdx        device index to read register from
 * @param [in]      xsmiInterface Xsmi interface (0..3)
 * @param [in]      xsmiAddr      Xsmi adresss
 * @param [in]      regAddr       register address
 * @param [in]      phyDev        the PHY device to read from (APPLICABLE RANGES: 0..31).
 * @param [out]     dataPtr       pointer to where data read will be stored
 */

PDL_STATUS prvPdlXsmiRegRead (
    IN  GT_U8      devIdx,
    IN  GT_U32     xsmiInterface,
    IN  GT_U32     xsmiAddr,
    IN  GT_U32     regAddr,
    IN  GT_U32     phyDev,
    OUT GT_U16     *dataPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->xsmiRegReadClbk == NULL)
        return PDL_NOT_IMPLEMENTED;
    return prvPdlOsCallbacksPtr->xsmiRegReadClbk(devIdx, xsmiInterface, xsmiAddr, regAddr, phyDev, dataPtr);
}
/*$ END OF prvPdlXsmiRegRead */


/**
 * @fn  PDL_STATUS prvPdlXsmiRegWrite
 *
 * @brief   Reads SMI register using given callback
 *
 * @param [in]      devIdx        device index to write register to
 * @param [in]      portGroupsBmp bitmap of ports
 * @param [in]      smiInterface  smi interface (0..3)
 * @param [in]      smiAddr       smi adresss
 * @param [in]      regAddr       register address
 * @param [out]     data          data to write
 */

PDL_STATUS prvPdlXsmiRegWrite (
    IN  GT_U8      devIdx,
    IN  GT_U32     xsmiInterface,
    IN  GT_U32     xsmiAddr,
    IN  GT_U32     regAddr,
    IN  GT_U32     phyDev,
    IN  GT_U16     mask,
    IN  GT_U16     data
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    GT_U16          oldData, newData;
    PDL_STATUS      pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->xsmiRegWriteClbk == NULL || prvPdlOsCallbacksPtr->xsmiRegReadClbk == NULL)
        return PDL_NOT_IMPLEMENTED;
    pdlStatus = prvPdlOsCallbacksPtr->xsmiRegReadClbk(devIdx, xsmiInterface, xsmiAddr, regAddr, phyDev, &oldData);
    PDL_CHECK_STATUS(pdlStatus);
    newData = (~mask & oldData) | (mask & data);
    return prvPdlOsCallbacksPtr->xsmiRegWriteClbk(devIdx, xsmiInterface, xsmiAddr, regAddr, phyDev, newData);
}
/*$ END OF prvPdlXsmiRegWrite */

/**
 * @fn  void prvPdlI2cResultHandler
 *
 * @brief   Call to given call-back which handle I2C operation result
 *
 * @param [in]      i2c_ret_status  return status from I2C operation
 * @param [in]      slave_address   I2C device (slave) address
 * @param [in]      bus_id          I2C bus id
 * @param [in]      offset          I2C offset
 * @param [in]      i2c_write       whether the operation was I2C write or read
 */
void prvPdlI2cResultHandler (
    /*!     INPUTS:             */
    IN  PDL_STATUS i2c_ret_status,
    IN  UINT_8     slave_address,
    IN  UINT_8     bus_id,
    IN  UINT_8     offset,
    IN  BOOLEAN    i2c_write
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/

    if (prvPdlOsCallbacksPtr && prvPdlOsCallbacksPtr->i2cResultHandlerClbk)
        prvPdlOsCallbacksPtr->i2cResultHandlerClbk(i2c_ret_status, slave_address, bus_id, offset, i2c_write);
}
/*$ END OF prvPdli2cHandler */

/**
 * @fn  BOOLEAN prvPdlXmlArchiveUncompressHandler (IN  char  *archiveFileNamePtr, OUT char  *xmlFileNamePtr)
 *
 * @brief   Call to given call-back which handle I2C operation result
 *
 * @param [in]      archiveFileNamePtr        archive name to uncompress
 * @param [out]     xmlFileNamePtr            resulting xml file name after uncompress process
 * @param [out]     signatureFileNamePtr      resulting signature file name after uncompress process
 */
extern BOOLEAN prvPdlXmlArchiveUncompressHandler (
    IN  char       *archiveFileNamePtr,
    OUT char       *xmlFileNamePtr,
    OUT char       *signatureFileNamePtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (archiveFileNamePtr == NULL || xmlFileNamePtr == NULL || signatureFileNamePtr == NULL || prvPdlOsCallbacksPtr == NULL)
        return FALSE;

    if (prvPdlOsCallbacksPtr->arXmlUncompressClbk)
        return prvPdlOsCallbacksPtr->arXmlUncompressClbk(archiveFileNamePtr, xmlFileNamePtr, signatureFileNamePtr);
    /*else
        strcmp(xmlFileNamePtr, xmlFileNamePtr); statement with no effect [-Werror=unused-value] */

    return TRUE;
}
/*$ END OF prvPdlXmlArchiveUncompressHandler */

/**
 * @fn  BOOLEAN prvPdlXmlVerificationHandler (IN  char  *xmlFileNamePtr, IN char  *signatureFileNamePtr)
 *
 * @brief   Call to given call-back which handle XML signature verification
 *
 * @param [in]      xmlFileNamePtr          xml file name
 * @param [out]     signatureFileNamePtr    signature file name
 */
extern BOOLEAN prvPdlXmlVerificationHandler (
    IN  char       *xmlFileNamePtr,
    IN  char       *signatureFileNamePtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    /* only verify XML for full init type not for remote stack units */
    if (prvPdlLibCurrentInitType != PDL_OS_INIT_TYPE_FULL_E)
        return TRUE;

    if (xmlFileNamePtr == NULL || signatureFileNamePtr == NULL || prvPdlOsCallbacksPtr == NULL)
        return FALSE;

    if (prvPdlOsCallbacksPtr->vrXmlVerifyClbk)
        return prvPdlOsCallbacksPtr->vrXmlVerifyClbk(xmlFileNamePtr, signatureFileNamePtr);

    return TRUE;
}
/*$ END OF prvPdlXmlVerificationHandler */

/**
 * @fn  PDL_STATUS prvPdlLedStreamPortPositionSet
 *
 * @brief   set ledstream port position
 *
 * @param [in]      devIdx        device index
 * @param [in]      portNum       port number
 * @param [in]      position      led stream port position
 */

PDL_STATUS prvPdlLedStreamPortPositionSet (
    IN  GT_U8                   devIdx,
    IN  GT_U32                  portNum,
    IN  GT_U32                  position
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledStreamPortPositionSetClbk == NULL)
        return PDL_NOT_IMPLEMENTED;
    return prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledStreamPortPositionSetClbk(devIdx, portNum, position);
#else
    return PDL_OK;
#endif
}
/*$ END OF prvPdlLedStreamPortPositionSet */

/**
 * @fn  PDL_STATUS prvPdlLedStreamPortClassPolarityInvertEnableSet
 *
 * @brief   set ledstream port polarity
 *
 * @param [in]      devIdx        device index
 * @param [in]      portNum       port number
 * @param [in]      classNum      class number
 * @param [in]      invertEnable  polarity invert enable/disable
 */

PDL_STATUS prvPdlLedStreamPortClassPolarityInvertEnableSet (
   IN  GT_U8                           devIdx,
   IN  GT_U32                          portNum,
   IN  GT_U32                          classNum,
   IN  BOOLEAN                         invertEnable
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledStreamPortClassPolarityInvertEnableSetClbk == NULL)
        return PDL_NOT_IMPLEMENTED;
    return prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledStreamPortClassPolarityInvertEnableSetClbk(devIdx, portNum, classNum, invertEnable);
#else
    return PDL_OK;
#endif
}
/*$ END OF prvPdlLedStreamPortClassPolarityInvertEnableSet */

/**
 * @fn  PDL_STATUS prvPdlLedStreamConfigSet
 *
 * @brief   set ledstream configuration
 *
 * @param [in]      devIdx                  device index
 * @param [in]      ledInterfaceNum         ledstream interface number
 * @param [in]      ledConfPtr              led stream configuration paramters
 */

PDL_STATUS prvPdlLedStreamConfigSet (
    IN  GT_U8                               devIdx,
    IN  GT_U32                              ledInterfaceNum,
    IN  PDL_LED_STEAM_INTERFACE_CONF_STC   *ledConfPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledStreamConfigSetClbk == NULL)
        return PDL_NOT_IMPLEMENTED;
    return prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledStreamConfigSetClbk(devIdx, ledInterfaceNum, ledConfPtr);
#else
    return PDL_OK;
#endif
}
/*$ END OF prvPdlLedStreamConfigSet */

/**
 * @fn  PDL_STATUS prvPdlLedStreamClassManipulationGet
 *
 * @brief   ledstream manipulation get value
 *
 * @param [in]      devIdx                  device index
 * @param [in]      ledInterfaceNum         ledstream interface number
 * @param [in]      portType                port type
 * @param [in]      classNum                class number
 * @param [out]     classParamsPtr          value
 */

PDL_STATUS prvPdlLedStreamClassManipulationGet (
    IN  GT_U8                           devIdx,
    IN  GT_U32                          ledInterfaceNum,
    IN  PDL_LED_PORT_TYPE_ENT           portType,
    IN  GT_U32                          classNum,
    OUT PDL_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledsStreamClassManipulationGetClbk == NULL)
        return PDL_NOT_IMPLEMENTED;
    return prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledsStreamClassManipulationGetClbk(devIdx, ledInterfaceNum, portType, classNum, classParamsPtr);
#else
    return PDL_OK;
#endif
}

/*$ END OF prvPdlLedStreamClassManipulationGet */

/**
 * @fn  PDL_STATUS prvPdlLedStreamClassManipulationSet
 *
 * @brief   ledstream manipulation set value
 *
 * @param [in]      devIdx                  device index
 * @param [in]      ledInterfaceNum         ledstream interface number
 * @param [in]      portType                port type
 * @param [in]      classNum                class number
 * @param [in]      classParamsPtr          value
 */

PDL_STATUS prvPdlLedStreamClassManipulationSet (
    IN  GT_U8                           devIdx,
    IN  GT_U32                          ledInterfaceNum,
    IN  PDL_LED_PORT_TYPE_ENT           portType,
    IN  GT_U32                          classNum,
    IN  PDL_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledsStreamClassManipulationSetClbk == NULL)
        return PDL_NOT_IMPLEMENTED;
    return prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledsStreamClassManipulationSetClbk(devIdx, ledInterfaceNum, portType, classNum, classParamsPtr);
#else
    return PDL_OK;
#endif
}

/*$ END OF prvPdlLedStreamClassManipulationSet */

/* ***************************************************************************
* FUNCTION NAME: pdlLibInit
*
* DESCRIPTION:   Init database module
*                Bind all implemented DB callbacks
*
* PARAMETERS:
*
* [IN] callbacksPTR           application-specific implementation for os services
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlLibDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Lib debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlLibDebugSet (
    IN  BOOLEAN             state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlLibDebugFlag = state;
    return PDL_OK;
}

/* ***************************************************************************
* FUNCTION NAME: prvPdlLibStrICmp
*
* DESCRIPTION:   case insensitive string compare
*
*****************************************************************************/

UINT_32 prvPdlLibStrICmp (
    /*!     INPUTS:             */
    const char *string1Ptr,  /* string1_PTR - pointer to the first string */
    const char *string2Ptr   /* string2_PTR - pointer to the second string */
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
        INT_32 c1;
        INT_32 c2;
        INT_32 ret;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/

        if ((string1Ptr == NULL) && (string2Ptr == NULL))
            return 0;
        if (string1Ptr == NULL)
            return -1;
        if (string2Ptr == NULL)
            return 1;

        for(; ; )
        {
                c1 = *string1Ptr++;
                c2 = *string2Ptr++;
                if ('a' <= c1 && c1 <= 'z')
                        c1 += ('A' - 'a');

                if ('a' <= c2 && c2 <= 'z')
                        c2 += ('A' - 'a');

                ret = c1 - c2;
                if (ret || !c1)
                        break;
        }
        return ret;
}
/*$ END OF prvPdlLibStrICmp */

/* ***************************************************************************
* FUNCTION NAME: prvPdlLibStrdup
*
* DESCRIPTION:   string duplication using local memory allocation callback
*
*****************************************************************************/

char *prvPdlLibStrdup (
    /*!     INPUTS:             */
    const char *str1
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_32             len;
    char              * new_str;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (str1 == NULL) {
        return NULL;
    }
    len = strlen (str1) + 1;
    new_str = (char*) prvPdlOsMalloc (len);
    if (new_str != NULL) {
        memcpy (new_str, str1, len);
    }
    return new_str;
}
/*$ END OF prvPdlLibStrdup */

/* ***************************************************************************
* FUNCTION NAME: prvPdlLibStrtok_r
*
* DESCRIPTION:      The function STRINGG_strtok_r() breaks the string s into a sequence
*                   of tokens, each of which is delimited by a character from the string
*                   pointed to by sep.
*
*
*****************************************************************************/

char *prvPdlLibStrtok_r (
    /*!     INPUTS:             */
    char        *s1_PTR,        /* s1_PTR - Points to NULL, or the string from
                                    which to extract tokens. */
    /*!     INPUTS:             */
    const char  *delim_PTR,     /* delim_PTR - Points to a null-terminated set of
                                   delimiter characters that separate the
                                   tokens. */
    /*!     INPUTS / OUTPUTS:   */
    char        **save_ptr_PTR  /* save_ptr_PTR - Is a value-return parameter used by
                                   the function to record its progress
                                   through s. */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    char    *token_start_PTR;
    char    *token_end_PTR;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if ( (delim_PTR == NULL) || (save_ptr_PTR == NULL) ) {
        return NULL;
    }

/*  s2 = delim :
    The address of a pointer to a character, which the function can
    use to store information necessary for it to continue scanning the
    same string. */

    if ( s1_PTR == NULL && (*save_ptr_PTR == NULL) ) return NULL;
    if ( s1_PTR == NULL ) s1_PTR = *save_ptr_PTR;

    /* Scan leading delimiters.  */
    token_start_PTR = s1_PTR + strspn(s1_PTR, delim_PTR);
    if (*token_start_PTR == '\0') return NULL;

    /* Find the end of the token.  */
    token_end_PTR = strpbrk(token_start_PTR, delim_PTR);
    if (token_end_PTR == NULL) {
        /* This token finishes the string.  */
        *save_ptr_PTR = strchr(token_start_PTR, '\0');
    }
    else {
        /* Terminate the token and make *SAVE_PTR point past it.  */
        *token_end_PTR = '\0';
        *save_ptr_PTR  = token_end_PTR + 1;
    }
    return token_start_PTR;
}
/*$ END OF prvPdlLibStrtok_r */

/* ***************************************************************************
* FUNCTION NAME: prvPdlLibPortModeSupported
*
* DESCRIPTION:   verify port interface mode & speed are supported by HW
*
*****************************************************************************/

BOOLEAN prvPdlLibPortModeSupported(
    UINT_8                      devIdx,
    UINT_32                     mac_port,
    PDL_PORT_SPEED_ENT          speed,
    PDL_INTERFACE_MODE_ENT      interface_mode
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->portModeSupportedClbk == NULL)
        return PDL_NOT_IMPLEMENTED;
    return prvPdlOsCallbacksPtr->portModeSupportedClbk(devIdx, mac_port, speed, interface_mode);
}
/*$ END OF prvPdlRegWrite */


/**
 * @fn  PDL_STATUS prvPdlLibBoardDescriptionInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlRootId )
 *
 * @brief   parse board description section
 *
 * @param [in]  xmlRootId   Xml root id.
 *
 * @return  PDL_OK                  success
 */

PDL_STATUS prvPdlLibBoardDescriptionInit(
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlRootId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    XML_PARSER_RET_CODE_TYP          xmlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP   xmlBoardDescId;
    UINT_32                          maxSize;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    xmlStatus = xmlParserFindByName (xmlRootId, "Board-description", &xmlBoardDescId);
    /* section is not mandatory */
    if (xmlStatus == XML_PARSER_RET_CODE_NOT_FOUND) {
        return PDL_OK;
    }
    maxSize = PDL_XML_BOARD_DESC_LEN;
    xmlParserGetValue(xmlBoardDescId, &maxSize, &prvPdlLibBoardDescStr[0]);
    prvPdlLibBoardDescExist = TRUE;
    return PDL_OK;
}
/*$ END OF prvPdlLibBoardDescriptionInit */

/**
 * @fn  PDL_STATUS pdlLibBoardDescriptionDbGet ( IN UINT_32 strLen, OUT char *boardDescStrPtr )
 *
 * @brief   get board description string
 *
 * @param [in]   strLen             given string pointer max size (length)
 * @param [out]  boardDescStrPtr    string pointer
 *
 * @return  PDL_OK                  success
 * @return  PDL_BAD_PTR             illegal pointer supplied.
 * @return  PDL_NOT_INITIALIZED     board description doesn't exist in XML
 * @return  PDL_BAD_SIZE            board description from XML is longer than given string pointer
 */

PDL_STATUS pdlLibBoardDescriptionDbGet(
    IN  UINT_32               strLen,
    OUT char                * boardDescStrPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (boardDescStrPtr == NULL) {
        return PDL_BAD_PTR;
    }
    if (prvPdlLibBoardDescExist == FALSE) {
        return PDL_NOT_INITIALIZED;
    }
    if (strlen(prvPdlLibBoardDescStr) > strLen) {
        return PDL_BAD_SIZE;
    }
    strcpy (boardDescStrPtr, prvPdlLibBoardDescStr);
    return PDL_OK;
}
/*$ END OF pdlLibBoardDescriptionDbGet */

/**
 * @fn  PDL_STATUS pdlLibInit ( IN PDL_OS_CALLBACK_API_STC * callbacksPTR )
 *
 * @brief   Pdl library initialize
 *
 * @param [in,out]  callbacksPTR    If non-null, the callbacks pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlLibInit (
    IN PDL_OS_CALLBACK_API_STC    * callbacksPTR,
    IN PDL_OS_INIT_TYPE_ENT         initType
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlLibCurrentInitType = initType;

    if (callbacksPTR == NULL) {
        return PDL_BAD_PTR;
    }

    memcpy(prvPdlOsCallbacksPtr, callbacksPTR, sizeof(PDL_OS_CALLBACK_API_STC));

    pdlPrvDbFuncArray[PDL_DB_PRV_TYPE_LIST_E].dbCreateFun = prvPdlDbLinkedListCreate;
    pdlPrvDbFuncArray[PDL_DB_PRV_TYPE_LIST_E].dbAddFun = prvPdlDbLinkedListAdd;
    pdlPrvDbFuncArray[PDL_DB_PRV_TYPE_LIST_E].dbFindFun = prvPdlDbLinkedListFind;
    pdlPrvDbFuncArray[PDL_DB_PRV_TYPE_LIST_E].dbGetNextFun = prvPdlDbLinkedListGetNext;
    pdlPrvDbFuncArray[PDL_DB_PRV_TYPE_LIST_E].dbGetNumOfEntriesFun = prvPdlDbLinkedListGetNumOfEntries;
    pdlPrvDbFuncArray[PDL_DB_PRV_TYPE_LIST_E].dbGetFirstFun = prvPdlDbLinkedListGetFirst;
    pdlPrvDbFuncArray[PDL_DB_PRV_TYPE_LIST_E].dbGetNextKeyFun = prvPdlDbLinkedListGetNextKey;
    pdlPrvDbFuncArray[PDL_DB_PRV_TYPE_LIST_E].dbDestroyFun = prvPdlDbLinkedListDestroy;

    pdlPrvDbFuncArray[PDL_DB_PRV_TYPE_ARRAY_E].dbCreateFun = prvPdlDbArrayCreate;
    pdlPrvDbFuncArray[PDL_DB_PRV_TYPE_ARRAY_E].dbAddFun = prvPdlDbArrayAdd;
    pdlPrvDbFuncArray[PDL_DB_PRV_TYPE_ARRAY_E].dbFindFun = prvPdlDbArrayFind;
    pdlPrvDbFuncArray[PDL_DB_PRV_TYPE_ARRAY_E].dbGetNextFun = prvPdlDbArrayGetNext;
    pdlPrvDbFuncArray[PDL_DB_PRV_TYPE_ARRAY_E].dbGetNumOfEntriesFun = prvPdlDbArrayGetNumOfEntries;
    pdlPrvDbFuncArray[PDL_DB_PRV_TYPE_ARRAY_E].dbGetFirstFun = prvPdlDbArrayGetFirst;
    pdlPrvDbFuncArray[PDL_DB_PRV_TYPE_ARRAY_E].dbDestroyFun = prvPdlDbArrayDestroy;

    return PDL_OK;
}

/*$ END OF PdlBtnInit */
