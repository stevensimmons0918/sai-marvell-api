/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\src\xml\private\prvxmlparserbuilder.c.
 *
 * @brief   Prvxmlparserbuilder class
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <pdl/xml/xmlParser.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <pdl/xml/private/prvXmlParserData.h>
#include <pdl/xml/private/prvXmlParserBuilder.h>
#include <pdl/lib/private/prvPdlLib.h>

#define XML_PARSER_DEBUG_LOG(x) prvPdlOsPrintf x

/**
 * @brief   The prv XML parser debug opened descriptors[ prv XML parser maximum number of opened
 *          trees cns]
 */

XML_PARSER_ROOT_DESCRIPTOR_TYP prvXmlParserDebugOpenedDescriptors[PRV_XML_PARSER_MAX_NUM_OF_OPENED_TREES_CNS] = {0};

/**
 * @struct  _PRV_XML_PARSER_XML_TAG_DATA_STCT
 *
 * @brief   A prv XML parser XML tag data stct.
 */

typedef struct _PRV_XML_PARSER_XML_TAG_DATA_STCT
{
    /** @brief   The value pointer */
    char              * strValuePtr;
    /** @brief   The end pointer */
    char              * endPtr;
    /** @brief   True if is leaf, false if not */
    BOOLEAN             isLeaf;
    /** @brief   True if is closing character, false if not */
    BOOLEAN             isClosingChar;
    /** @brief   The start of file pointer */
    char              * startOfFilePtr;
}PRV_XML_PARSER_XML_TAG_DATA_STC;

/**
 * @struct  XML_GET_NEXT_DATA_STC
 *
 * @brief   An XML get next data stc.
 */

typedef struct 
{
    /** @brief   The context */
    PRV_XML_PARSER_NODE_STC   * context;
    /** @brief   The current */
    PRV_XML_PARSER_NODE_STC   * current;
    /** @brief   True to initialize done */
    BOOLEAN                     init_done;
} XML_GET_NEXT_DATA_STC;

/**
 * @struct  XML_GET_TAG_COUNT_STC
 *
 * @brief   An XML get tag count stc.
 */

typedef struct 
{
    /** @brief   The name pointer */
    char            * namePtr;
    /** @brief   Number of */
    UINT_32           count;
    /** @brief   True to initialize done */
    BOOLEAN           init_done;
} XML_GET_TAG_COUNT_STC;

/**
 * @typedef BOOLEAN (*PRV_XML_PARSER_TRAVERSE_HANDLER_PTR)(PRV_XML_PARSER_NODE_STC ** nodePtr, void * cookiePtr)
 *
 * @brief   Defines an alias representing the cookie pointer
 */

typedef BOOLEAN (*PRV_XML_PARSER_TRAVERSE_HANDLER_PTR)(PRV_XML_PARSER_NODE_STC ** nodePtr, void * cookiePtr);

/**
 * @typedef void (*PRV_XML_PARSER_INT_VALUE_GET_HANDLER_PTR)(char * strValuePtr, void * valuePtr, PRV_XML_PARSER_SIMPLE_DATA_TYPE_FORMAT_ENT *formatPtr)
 *
 * @brief   Defines an alias representing the *format pointer
 */

typedef void (*PRV_XML_PARSER_INT_VALUE_GET_HANDLER_PTR)(char * strValuePtr, void * valuePtr, PRV_XML_PARSER_SIMPLE_DATA_TYPE_FORMAT_ENT *formatPtr);


static PRV_XML_PARSER_TYPEDEF_DATA_UNT * prvXmlParserEnumsDbArr[XML_PARSER_ENUM_ID_LAST_E];

static char * prvXmlParserEnumId2NameMappingtArr[XML_PARSER_ENUM_ID_LAST_E] = {

    /* XML_PARSER_ENUM_ID_I2C_ACCESS_TYPE_E,                     */ "i2c-access-type"                   ,
    /* XML_PARSER_ENUM_ID_MPP_PIN_MODE_TYPE_E,                   */ "mpp-pin-mode-type"                 ,
    /* XML_PARSER_ENUM_ID_MPP_PIN_INIT_VALUE_TYPE_E,             */ "mpp-pin-init-value-type"           ,
    /* XML_PARSER_ENUM_ID_INTERFACE_TYPE_E,                      */ "interface-type"                    ,
    /* XML_PARSER_ENUM_ID_SMI_XSMI_TYPE_TYPE_E,                  */ "smi-xsmi-type-type"                ,
    /* XML_PARSER_ENUM_ID_PANEL_GROUP_ORDERING_TYPE_E,           */ "panel-group-ordering-type"         ,
    /* XML_PARSER_ENUM_ID_SPEED_TYPE_E,                          */ "speed-type"                        ,
    /* XML_PARSER_ENUM_ID_TRANSCEIVER_TYPE_E,                    */ "transceiver-type"                  ,
    /* XML_PARSER_ENUM_ID_CONNECTOR_TYPE_TYPE_E,                 */ "connector-type-type"               ,
    /* XML_PARSER_ENUM_ID_L1_INTERFACE_MODE_TYPE_E,              */ "L1-interface-mode-type"            ,
    /* XML_PARSER_ENUM_ID_FAN_ROLE_TYPE_E,                       */ "fan-role-type"                     ,
    /* XML_PARSER_ENUM_ID_HW_FAN_OPERATION_TYPE_E,               */ "hw-fan-operation-type"             ,
    /* XML_PARSER_ENUM_ID_FAN_ADT7476_FAULT_DETECTION_TYPE_E,    */ "fan-ADT7476-fault-detection-type"  ,
    /* XML_PARSER_ENUM_ID_FAN_TYPE_E,                            */ "fan-type"                          ,
    /* XML_PARSER_ENUM_ID_BANK_SOURCE_TYPE_E,                    */ "bank-source-type"                  ,
    /* XML_PARSER_ENUM_ID_PSEPORT_TYPE_E,                        */ "PSEport-type"                      ,
    /* XML_PARSER_ENUM_ID_POWER_STATUS_TYPE_E,                   */ "power-status-type"                 ,
    /* XML_PARSER_ENUM_ID_PHY_TYPE_TYPE_E,                       */ "phy-type-type"                     ,
    /* XML_PARSER_ENUM_ID_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_E,   */ "phy-firmware-download-method-type" ,
    /* XML_PARSER_ENUM_ID_COLOR_TYPE_E,                          */ "color-type"                        ,
    /* XML_PARSER_ENUM_ID_LED_STATE_TYPE_E,                      */ "led-state-type"                    ,
    /* XML_PARSER_ENUM_ID_LED_GPIO_I2C_INTERFACE_TYPE_E,         */ "led-gpio-i2c-interface-type"       ,
    /* XML_PARSER_ENUM_ID_LED_GPIO_I2C_PHY_INTERFACE_TYPE_E,     */ "led-gpio-i2c-phy-interface-type"   ,
    /* XML_PARSER_ENUM_ID_LED_GPIO_I2C_PHY_pp_INTERFACE_TYPE_E,  */ "led-gpio-i2c-phy-pp-interface-type",
    /* XML_PARSER_ENUM_ID_CPU_TYPE_TYPE_E,                       */ "cpu-type-type"                     ,
    /* XML_PARSER_ENUM_ID_BUTTON_TYPE_E,                         */ "button-type"                       ,
    /* XML_PARSER_ENUM_ID_SENSOR_HW_TYPE_E,                      */ "sensor-hw-type"                    ,
    /* XML_PARSER_ENUM_ID_THRESHOLD_TYPE_E,                      */ "threshold-type"                    ,
    /* XML_PARSER_ENUM_ID_I2C_TRANSACTION_TYPE_E,                */ "i2c-transaction-type"              ,
    /* XML_PARSER_ENUM_ID_PSE_TYPE_E,                            */ "PSE-type"                          ,
    /* XML_PARSER_ENUM_ID_PHY_SMI_XSML_ROLE_TYPE_E,              */ "smi-xsmi-address-role-type"        ,
    /* XML_PARSER_ENUM_ID_PORT_LED_SUPPORTED_TYPE_E,             */ "port-leds-supported-type"          ,
    /* XML_PARSER_ENUM_ID_LED_STREAM_ORDER_MODE_TYPE_E,          */ "led-stream-order-mode-type"        ,
    /* XML_PARSER_ENUM_ID_LED_STREAM_BLINK_DUTY_CYCLE_TYPE_E,    */ "led-stream-blink-duty-cycle-type"  ,
    /* XML_PARSER_ENUM_ID_LED_STREAM_BLINK_DURATION_TYPE_E,      */ "led-stream-blink-duration-type"    ,
    /* XML_PARSER_ENUM_ID_LED_STREAM_PULSE_STRETCH_TYPE_E,       */ "led-stream-pulse-stretch-type"     ,
    /* XML_PARSER_ENUM_ID_LED_STREAM_CLASS5_SELECT_TYPE_E,       */ "led-stream-class5-select-type"     , 
    /* XML_PARSER_ENUM_ID_LED_STREAM_CLASS13_SELECT_TYPE_E,      */ "led-stream-class13-select-type"    ,
    /* XML_PARSER_ENUM_ID_LED_STREAM_CLOCK_OUT_FREQUENCY_TYPE_E, */ "led-stream-clock-out-frequency-type" ,
    /* XML_PARSER_ENUM_ID_LED_STREAM_BLINK_SELECT_TYPE_E,        */ "led-stream-blink-select-type"      ,
    /* XML_PARSER_ENUM_ID_LED_STREAM_PORT_TYPE_E,                */ "led-stream-port-type"              ,

};


#define PRV_XML_PARSER_IS_WHITE_SPACE_MAC(ch)  (((ch) == ' ') || ((ch) == '\t') || ((ch) == '\r') || ((ch) == '\n'))

#define PRV_XML_PARSER_MIN_KEYWORD_STR_CNS      "min"

#define PRV_XML_PARSER_MAX_KEYWORD_STR_CNS      "max"

#define PRV_XML_PARSER_INTEGER_RANGE_STR_CNS    "range"

#define PRV_XML_PARSER_STRING_LENGTH_STR_CNS    "length"


#define PRV_XML_PARSER_BOOLEAN_STR_FALSE_CNS    "false"

#define PRV_XML_PARSER_BOOLEAN_STR_TRUE_CNS     "true"


#define PRV_XML_PARSER_TYPE_EMPTY_STR_CNS       "empty"

#define PRV_XML_PARSER_TYPE_BOOLEAN_STR_CNS     "boolean"

#define PRV_XML_PARSER_TYPE_UINT8_STR_CNS       "uint8"

#define PRV_XML_PARSER_TYPE_UINT16_STR_CNS      "uint16"

#define PRV_XML_PARSER_TYPE_UINT32_STR_CNS      "uint32"

#define PRV_XML_PARSER_TYPE_ENUM_STR_CNS        "enumeration"

#define PRV_XML_PARSER_TYPE_STRING_STR_CNS      "string"

#define PRV_XML_PARSER_TYPE_BITS_STR_CNS        "bits"

/**
 * @fn  static PRV_XML_PARSER_NODE_STC * prvXmlParserNodeAdd ( IN PRV_XML_PARSER_NODE_STC * parentNodePtr, IN char * namePtr, IN char * valuePtr )
 *
 * @brief   Add new xml node to tree.
 *
 * @param [in]  parentNodePtr   Pointer to parent node.
 * @param [in]  namePtr         XML node name.
 * @param [in]  valuePtr        XML node value.
 *
 * @return  Pointer to created node or NULL if node couldn't be created.
 */

static PRV_XML_PARSER_NODE_STC * prvXmlParserNodeAdd (
    IN PRV_XML_PARSER_NODE_STC  * parentNodePtr,
    IN char                     * namePtr,
    IN char                     * valuePtr
)
{
    PRV_XML_PARSER_NODE_STC * node = (PRV_XML_PARSER_NODE_STC *)prvPdlOsCalloc(1, sizeof(PRV_XML_PARSER_NODE_STC));
    if (!node)
        return NULL;

    node->namePtr = XML_PARSER_STRDUP_MAC(namePtr);
    if (!node->namePtr)
    {
        prvPdlOsFree(node);
        return NULL;
    }
    if (valuePtr)
    {
        node->type = PRV_XML_PARSER_NODE_TYPE_LEAF_E;
        node->valuePtr = XML_PARSER_STRDUP_MAC(valuePtr);
        if (!node->valuePtr)
        {
            prvPdlOsFree(node->namePtr);
            prvPdlOsFree(node);
            return NULL;
        }
        node->isValid = TRUE;
    }
    else
    {
        if (strcmp(namePtr, "typedef") == 0)
            node->type = PRV_XML_PARSER_NODE_TYPE_TYPE_E;
        else
            node->type = PRV_XML_PARSER_NODE_TYPE_GROUP_E;
    }
    if (parentNodePtr->lastChildPtr)
    {
        parentNodePtr->lastChildPtr->nextSiblingPtr = node;
    }
    if (!parentNodePtr->firstChildPtr)
    {
        parentNodePtr->firstChildPtr = node;
    }
    parentNodePtr->lastChildPtr = node;

    node->parentPtr = parentNodePtr;

    return node;
}

/**
 * @fn  static BOOLEAN prvXmlParserAuxDataAdd ( IN PRV_XML_PARSER_NODE_STC * nodePtr, IN char * typePtr )
 *
 * @brief   Updates auxiliary xml node data.
 *
 * @param [in]  nodePtr Pointer to xml node.
 * @param [in]  typePtr Type name.
 *
 * @return  TRUE on success or FALSE if memory allocation failed.
 */

static BOOLEAN prvXmlParserAuxDataAdd (
    IN PRV_XML_PARSER_NODE_STC * nodePtr, 
    IN char                    * typePtr
)
{
    nodePtr->auxData.attrNamePtr = XML_PARSER_STRDUP_MAC(typePtr);
    if (!nodePtr->auxData.attrNamePtr)
    {
        return FALSE;
    }

    return TRUE;
}

/**
 * @fn  static BOOLEAN prvXmlParserIsEndOfTag( IN char * chPtr )
 *
 * @brief   Checks if char is the end of XML tag.
 *
 * @param [in]  chPtr   Pointer to char.
 *
 * @return  TRUE if char is the end of XML tag.
 */

static BOOLEAN prvXmlParserIsEndOfTag(
    IN char * chPtr
)
{
    if (!chPtr || *chPtr == '>')
        return TRUE;

    return FALSE;
}

/**
 * @fn  static BOOLEAN prvXmlParserAreTagsEqual( IN char * firstPtr, IN char * secondPtr )
 *
 * @brief   Checks if two tags have same name
 *
 * @param [in]  firstPtr    Pointer to first string.
 * @param [in]  secondPtr   Pointer to second string.
 *
 * @return  TRUE if tags are the same.
 */

static BOOLEAN prvXmlParserAreTagsEqual(
    IN char * firstPtr, 
    IN char * secondPtr
)
{
    if (firstPtr && *firstPtr == '<') firstPtr ++;
    if (secondPtr && *secondPtr == '<') secondPtr ++;
    if (secondPtr && *secondPtr == '/') secondPtr ++;
    while (firstPtr && *firstPtr != '>' && PRV_XML_PARSER_IS_WHITE_SPACE_MAC(*firstPtr)) firstPtr ++;
    while (secondPtr && *secondPtr != '>' && PRV_XML_PARSER_IS_WHITE_SPACE_MAC(*secondPtr)) secondPtr ++;
    while (1)
    {
        if ((prvXmlParserIsEndOfTag(firstPtr) || PRV_XML_PARSER_IS_WHITE_SPACE_MAC(*firstPtr)) && (prvXmlParserIsEndOfTag(secondPtr) || PRV_XML_PARSER_IS_WHITE_SPACE_MAC(*secondPtr)))
            break;
        if (((prvXmlParserIsEndOfTag(firstPtr) || PRV_XML_PARSER_IS_WHITE_SPACE_MAC(*firstPtr)) && !(prvXmlParserIsEndOfTag(secondPtr) || PRV_XML_PARSER_IS_WHITE_SPACE_MAC(*secondPtr))) || 
            ((prvXmlParserIsEndOfTag(secondPtr) || PRV_XML_PARSER_IS_WHITE_SPACE_MAC(*secondPtr)) && !(prvXmlParserIsEndOfTag(firstPtr) || PRV_XML_PARSER_IS_WHITE_SPACE_MAC(*firstPtr))))
            return FALSE;
        if (*firstPtr != *secondPtr)
            return FALSE;
        firstPtr ++;
        secondPtr ++;
    }

    return TRUE;
}

/**
 * @fn  static void prvXmlParserErrDebugOut( IN char * fileStartPtr, IN char * errStrPtr )
 *
 * @brief   Print error debug string (limited number of chars)
 *
 * @param [in]  fileStartPtr    Pointer to start of the file.
 * @param [in]  errStrPtr       Pointer to problematic string.
 */

static void prvXmlParserErrDebugOut(
    IN char       * fileStartPtr,
    IN char       * errStrPtr
)
{
    UINT_32 maxCount = 80, line = 1;
    char * startOfLinePtr = fileStartPtr;

    /* find error line */
    while (fileStartPtr != errStrPtr)
    {
        if (*fileStartPtr == '\n' && *(fileStartPtr + 1) == '\r')
        {
            fileStartPtr += 2;
            line ++;
            startOfLinePtr = fileStartPtr;
            continue;
        }

        if (*fileStartPtr == '\n')
        {
            line ++;
            startOfLinePtr = fileStartPtr + 1;
        }

        fileStartPtr ++;
    }

    XML_PARSER_DEBUG_LOG(("Encountered problem @line %d:", line));
    while (startOfLinePtr && *startOfLinePtr && *startOfLinePtr != '\n' && (maxCount--))
    {
        XML_PARSER_DEBUG_LOG(("%c", *startOfLinePtr));
        startOfLinePtr ++;
    }
    XML_PARSER_DEBUG_LOG(("\n"));
}

/**
 * @fn  static BOOLEAN prvXmlParserTagNameGet( IN PRV_XML_PARSER_XML_TAG_DATA_STC * tagDataPtr, OUT char * bufferPtr, IN UINT_32 buf_size )
 *
 * @brief   Parses xml string and returns name of xml tag
 *
 * @param [in]  tagDataPtr  Pointer to data of xml string.
 * @param [out] bufferPtr   Pointer to buffer for parsed xml.
 * @param [in]  buf_size    Max size of bufferPtr.
 *
 * @return  True if it succeeds, false if it fails.
 */

static BOOLEAN prvXmlParserTagNameGet(
    IN  PRV_XML_PARSER_XML_TAG_DATA_STC       * tagDataPtr,
    OUT char                                  * bufferPtr, 
    IN  UINT_32                                 buf_size
)
{
    char * buffer_end = bufferPtr + buf_size,
         * tagPtr = tagDataPtr->strValuePtr,
         * origStr = tagPtr;

    /* find tag's start */
    while (tagPtr && PRV_XML_PARSER_IS_WHITE_SPACE_MAC(*tagPtr)) tagPtr ++;
    if (tagPtr && *tagPtr == '<')
        tagPtr ++;
    if (tagPtr && *tagPtr == '/')
        tagPtr ++;

    /* read and copy tag's name */
    while (tagPtr && *tagPtr != '>')
    {
        if (*tagPtr == '<')
        {
            prvXmlParserErrDebugOut(tagDataPtr->startOfFilePtr, origStr);
            return FALSE;
        }

        if (PRV_XML_PARSER_IS_WHITE_SPACE_MAC(*tagPtr) || (*tagPtr == '/' && (tagPtr + 1) && *(tagPtr + 1) == '>'))
            break;

        *bufferPtr = *tagPtr;
        tagPtr ++;
        bufferPtr ++;
        if (bufferPtr == buffer_end)
        {
            bufferPtr --;
            break;
        }
    }
    *bufferPtr = 0;
    return TRUE;
}

/**
 * @fn  static XML_PARSER_RET_CODE_TYP prvXmlParserAttrValueGet( IN PRV_XML_PARSER_XML_TAG_DATA_STC * tagDataPtr, OUT char * bufferPtr, IN UINT_32 buf_size )
 *
 * @brief   Parses xml string and returns value of attribute if exists
 *
 * @param [in]  tagDataPtr  Pointer to data of xml string.
 * @param [out] bufferPtr   Pointer to buffer for parsed xml.
 * @param [in]  buf_size    Max size of bufferPtr.
 *
 * @return  XML_PARSER_RET_CODE_OK if attribute was found.
 * @return  XML_PARSER_RET_CODE_NOT_FOUND if attribute was not found.
 * @return  XML_PARSER_RET_CODE_ERROR if parsing error was encountered.
 */

static XML_PARSER_RET_CODE_TYP prvXmlParserAttrValueGet(
    IN  PRV_XML_PARSER_XML_TAG_DATA_STC       * tagDataPtr,
    OUT char                                  * bufferPtr, 
    IN  UINT_32                                 buf_size
)
{
    char * buffer_end = bufferPtr + buf_size,
         * tagPtr = tagDataPtr->strValuePtr,
         * origStr = tagPtr;
    BOOLEAN is_found = FALSE;

    if (!tagPtr || *tagPtr != '=')
    {
        *bufferPtr = 0;
        return XML_PARSER_RET_CODE_NOT_FOUND;
    }

    tagPtr ++;

    /* skip opening parentheses */
    if (tagPtr && *tagPtr != '\"')
    {
        XML_PARSER_DEBUG_LOG(("No opening parentheses\n->\t"));
        prvXmlParserErrDebugOut(tagDataPtr->startOfFilePtr, origStr);
        *bufferPtr = 0;
        return XML_PARSER_RET_CODE_ERROR;
    }
    else
    {
        tagPtr ++;
    }

    while (tagPtr && *tagPtr != '>')
    {
        if (PRV_XML_PARSER_IS_WHITE_SPACE_MAC(*tagPtr) || (*tagPtr == '\"') || (*tagPtr == '/' && (tagPtr + 1) && *(tagPtr + 1) == '>'))
            break;

        *bufferPtr = *tagPtr;
        tagPtr ++;
        bufferPtr ++;
        is_found = TRUE;
        if (bufferPtr == buffer_end)
        {
            bufferPtr --;
            break;
        }
    }
    *bufferPtr = 0;

    /* drop closing parentheses */
    if (is_found)
    {
        if (tagPtr && *tagPtr != '\"')
        {
            XML_PARSER_DEBUG_LOG(("No closing parentheses\n->\t"));
            prvXmlParserErrDebugOut(tagDataPtr->startOfFilePtr, origStr);
            *bufferPtr = 0;
            return XML_PARSER_RET_CODE_ERROR;
        }
        else
        {
            tagPtr ++;
        }
    }
    return XML_PARSER_RET_CODE_OK;
}

/**
 * @fn  static XML_PARSER_RET_CODE_TYP prvXmlParserAttrNameGet( IN PRV_XML_PARSER_XML_TAG_DATA_STC * tagDataPtr, OUT char * bufferPtr, IN UINT_32 buf_size )
 *
 * @brief   Parses xml string and returns name of attribute if exists
 *
 * @param [in]  tagDataPtr  Pointer to data of xml string.
 * @param [out] bufferPtr   Pointer to buffer for parsed xml.
 * @param [in]  buf_size    Max size of bufferPtr.
 *
 * @return  XML_PARSER_RET_CODE_OK if attribute was found.
 * @return  XML_PARSER_RET_CODE_NOT_FOUND if attribute was not found.
 * @return  XML_PARSER_RET_CODE_ERROR if parsing error was encountered.
 */

static XML_PARSER_RET_CODE_TYP prvXmlParserAttrNameGet(
    IN  PRV_XML_PARSER_XML_TAG_DATA_STC       * tagDataPtr,
    OUT char                                  * bufferPtr, 
    IN  UINT_32                                 buf_size
)
{
    char * buffer_end = bufferPtr + buf_size,
         * tagPtr = tagDataPtr->strValuePtr,
         * startOfNamePTR = NULL;

    /* skip to possible attribute */
    while (tagPtr && *tagPtr != '>' && *tagPtr != ' ') tagPtr++;
    while (tagPtr && *tagPtr != '>' && *tagPtr == ' ') tagPtr++;

    startOfNamePTR = tagPtr;

    /* find '=' char -indicating attribute */
    while (tagPtr && *tagPtr != '>' && *tagPtr != '=') tagPtr++;

    if (tagPtr == NULL || *tagPtr != '=')
    {
        *bufferPtr = 0;
        return XML_PARSER_RET_CODE_NOT_FOUND;
    }

    while (startOfNamePTR && startOfNamePTR != tagPtr)
    {
        *bufferPtr = *startOfNamePTR;
        startOfNamePTR ++;
        bufferPtr ++;
        if (bufferPtr == buffer_end)
        {
            bufferPtr --;
            break;
        }
    }
    *bufferPtr = 0;
    tagDataPtr->strValuePtr = tagPtr;

    return XML_PARSER_RET_CODE_OK;
}

/**
 * @fn  static BOOLEAN prvXmlParserTagValueGet( IN PRV_XML_PARSER_XML_TAG_DATA_STC * tagDataPtr, OUT char * bufferPtr, IN UINT_32 buf_size )
 *
 * @brief   Parses xml string and returns value of xml tag
 *
 * @param [in]  tagDataPtr  Pointer to data of xml string.
 * @param [out] bufferPtr   Pointer to buffer for parsed xml.
 * @param [in]  buf_size    Max size of bufferPtr.
 *
 * @return  True if it succeeds, false if it fails.
 */

static BOOLEAN prvXmlParserTagValueGet(
    IN  PRV_XML_PARSER_XML_TAG_DATA_STC       * tagDataPtr,
    OUT char                                  * bufferPtr, 
    IN  UINT_32                                 buf_size
)
{
    char * buffer_end = bufferPtr + buf_size, * tagPtr = tagDataPtr->strValuePtr;

    /* find start of the tag */
    if (*tagPtr == '<')
        tagPtr ++;
    if (*tagPtr == '/')
        tagPtr ++;

    /* skip tag's name */
    while (tagPtr && *tagPtr != 0 && *tagPtr != '>')
    {
        if (*tagPtr == '<')
        {
            prvXmlParserErrDebugOut(tagDataPtr->startOfFilePtr, tagPtr);
            return FALSE;
        }
        tagPtr ++;
    }
    tagPtr ++;

    /* get tag's value */
    while (tagPtr && *tagPtr != 0 && *tagPtr != '<')
    {
        if (*tagPtr == '>')
        {
            prvXmlParserErrDebugOut(tagDataPtr->startOfFilePtr, tagPtr);
            return FALSE;
        }

        *bufferPtr = *tagPtr;
        tagPtr ++;
        bufferPtr ++;
        if (bufferPtr == buffer_end)
        {
            bufferPtr --;
            break;
        }
    }
    
    *bufferPtr = 0;

    return TRUE;
}

/**
 * @fn  static BOOLEAN prvXmlParserGetNextTag( IN char * streamPtr, OUT PRV_XML_PARSER_XML_TAG_DATA_STC * tagPtr )
 *
 * @brief   Walks over string and find start of the next xml tag
 *
 * @param [in]  streamPtr   Pointer to xml string.
 * @param [out] tagPtr      Pointer to next tag in the string.
 *
 * @return  True if it succeeds, false if it fails.
 */

static BOOLEAN prvXmlParserGetNextTag(
    IN  char                             * streamPtr, 
    OUT PRV_XML_PARSER_XML_TAG_DATA_STC  * tagPtr
)
{
    char * startChPtr = streamPtr, * stopChPtr = NULL;

    tagPtr->isLeaf = tagPtr->isClosingChar = FALSE;
    tagPtr->endPtr = NULL;

    while (1)
    {
        /* find start of the tag */
        while (startChPtr && *startChPtr && *startChPtr != '<' && *startChPtr != '>') startChPtr++;

        if (!startChPtr || *startChPtr != '<')
        {
            if (startChPtr && !(*startChPtr))
            {
                /* end of file */
                break;
            }
            prvXmlParserErrDebugOut(tagPtr->startOfFilePtr, streamPtr);
            return FALSE;
        }

        /* skip comments */
        if ((startChPtr+1) && (*(startChPtr+1) == '!' && strstr(startChPtr + 1, "!--")))
        {
            startChPtr += 4;
            while (startChPtr)
            {
                if (*startChPtr == '-' && (startChPtr + 1) && *(startChPtr + 1) == '-' && (startChPtr + 2) && *(startChPtr + 2) == '>')
                {
                    startChPtr += 3;
                    break;
                }
                startChPtr ++;
            }
            continue;
        }

        /* find end of the tag */
        stopChPtr = startChPtr + 1;
        while (stopChPtr && *stopChPtr && *stopChPtr != '<' && *stopChPtr != '>') stopChPtr++;

        if (!stopChPtr || *stopChPtr != '>')
        {
            prvXmlParserErrDebugOut(tagPtr->startOfFilePtr, startChPtr);
            return FALSE;
        }

        /* check if this is self-ending tag */
        if (*(stopChPtr - 1) == '/')
            tagPtr->isLeaf = TRUE;

        /* check if this is end tag */
        if (*(startChPtr + 1) == '/')
            tagPtr->isClosingChar = TRUE;

        /* skip header */
        if (*(startChPtr+1) == '?' && strstr(startChPtr+1, "?xml"))
        {
            /* find end of header tag */
            while (startChPtr)
            {
                if (*startChPtr == '?' && (startChPtr + 1) && *(startChPtr + 1) == '>')
                {
                    startChPtr += 2;
                    break;
                }
                startChPtr ++;
            }
            continue;
        }

        tagPtr->strValuePtr = startChPtr;
        tagPtr->endPtr = stopChPtr + 1;
        return TRUE;
    }

    tagPtr->strValuePtr = NULL;
    return TRUE;
}

/**
 * @fn  static PRV_XML_PARSER_SIMPLE_DATA_TYPE_ENT prvXmlParserIsSimpleType( IN char * typePtr )
 *
 * @brief   Test for simple type
 *
 * @param [in]  typePtr String describing typedef.
 *
 * @return  simple type enum or PRV_XML_PARSER_SIMPLE_DATA_TYPE_NONE_E.
 */

static PRV_XML_PARSER_SIMPLE_DATA_TYPE_ENT prvXmlParserIsSimpleType(
    IN char * typePtr
)
{
    if (strcmp(typePtr, PRV_XML_PARSER_TYPE_EMPTY_STR_CNS) == 0)
        return PRV_XML_PARSER_SIMPLE_DATA_TYPE_EMPTY_E;
    else if (strcmp(typePtr, PRV_XML_PARSER_TYPE_BOOLEAN_STR_CNS) == 0)
        return PRV_XML_PARSER_SIMPLE_DATA_TYPE_BOOLEAN_E;
    else if (strcmp(typePtr, PRV_XML_PARSER_TYPE_UINT8_STR_CNS) == 0)
        return PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT8_E;
    else if (strcmp(typePtr, PRV_XML_PARSER_TYPE_UINT16_STR_CNS) == 0)
        return PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT16_E;
    else if (strcmp(typePtr, PRV_XML_PARSER_TYPE_UINT32_STR_CNS) == 0)
        return PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT32_E;
    else if (strcmp(typePtr, PRV_XML_PARSER_TYPE_STRING_STR_CNS) == 0)
        return PRV_XML_PARSER_SIMPLE_DATA_TYPE_STRING_E;
    else if (strcmp(typePtr, PRV_XML_PARSER_TYPE_ENUM_STR_CNS) == 0)
        return PRV_XML_PARSER_SIMPLE_DATA_TYPE_ENUM_E;
    else if (strcmp(typePtr, PRV_XML_PARSER_TYPE_BITS_STR_CNS) == 0)
        return PRV_XML_PARSER_SIMPLE_DATA_TYPE_BITS_E;
    else
        return PRV_XML_PARSER_SIMPLE_DATA_TYPE_NONE_E;
}

/**
 * @fn  static PRV_XML_PARSER_TYPEDEF_NODE_STC * prvXmlParserSimpleTypedefAdd( IN char * typedefNamePtr, IN PRV_XML_PARSER_SIMPLE_DATA_TYPE_ENT data_type, IN UINT_32 min, IN UINT_32 max )
 *
 * @brief   Creates new typedef node in tree for simple typedef
 *
 * @param [in]  typedefNamePtr  String describing typedef.
 * @param [in]  data_type       Enum of simple type to use for current typedef.
 * @param [in]  min             Minimal value of range if applicable (integer or string)
 * @param [in]  max             Minimal value of range if applicable (integer or string)
 *
 * @return  Pointer to created typedef node or NULL.
 */

static PRV_XML_PARSER_TYPEDEF_NODE_STC * prvXmlParserSimpleTypedefAdd(
    IN char                                   * typedefNamePtr,
    IN PRV_XML_PARSER_SIMPLE_DATA_TYPE_ENT      data_type,
    IN UINT_32                                  min,
    IN UINT_32                                  max
)
{
    PRV_XML_PARSER_TYPEDEF_NODE_STC * new_type = (PRV_XML_PARSER_TYPEDEF_NODE_STC *)prvPdlOsCalloc(1, sizeof(PRV_XML_PARSER_TYPEDEF_NODE_STC));
    if (!new_type)
        return NULL;

    new_type->type = data_type;
    new_type->namePtr = XML_PARSER_STRDUP_MAC(typedefNamePtr);
    if (!new_type->namePtr)
    {
        prvPdlOsFree(new_type);
        return NULL;
    }

    if (data_type == PRV_XML_PARSER_SIMPLE_DATA_TYPE_STRING_E)
    {
        new_type->data.string_value.min = min;
        new_type->data.string_value.max = max;
    }
    else if ((data_type == PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT8_E) ||
             (data_type == PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT16_E) ||
             (data_type == PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT32_E))
    {
        new_type->data.int_value.min = min;
        new_type->data.int_value.max = max;
    }

    return new_type;
}

/**
 * @fn  static PRV_XML_PARSER_TYPEDEF_NODE_STC * prvXmlParserResolvedTypedefAdd( IN char * typeNamePtr, IN PRV_XML_PARSER_TYPEDEF_NODE_STC * resolvedTypePtr )
 *
 * @brief   Creates new typedef node in tree for resolved typedef. Meaning this new typedef rely
 *          on previously defined typedefs and doesn't enough (or any) additional information on
 *          itsown.
 *
 * @param [in,out]  typeNamePtr     If non-null, the type name pointer.
 * @param [in]      resolvedTypePtr Pointer to resolved typedef to use for current.
 *
 * @return  Pointer to created typedef node or NULL.
 *
 * ### param [in]       typedefNamePtr  String describing typedef.
 */

static PRV_XML_PARSER_TYPEDEF_NODE_STC * prvXmlParserResolvedTypedefAdd(
    IN char                             * typeNamePtr, 
    IN PRV_XML_PARSER_TYPEDEF_NODE_STC  * resolvedTypePtr
)
{
    PRV_XML_PARSER_TYPEDEF_NODE_STC * new_type = (PRV_XML_PARSER_TYPEDEF_NODE_STC *)prvPdlOsCalloc(1, sizeof(PRV_XML_PARSER_TYPEDEF_NODE_STC));
    if (!new_type)
        return NULL;

    new_type->type = PRV_XML_PARSER_SIMPLE_DATA_TYPE_NONE_E;
    new_type->namePtr = XML_PARSER_STRDUP_MAC(typeNamePtr);
    if (!new_type->namePtr)
    {
        prvPdlOsFree(new_type);
        return NULL;
    }

    new_type->resolvedPtr = resolvedTypePtr;

    return new_type;
}

/**
 * @fn  static PRV_XML_PARSER_TYPEDEF_NODE_STC * prvXmlParserResolve( IN PRV_XML_PARSER_TYPEDEF_NODE_STC * typedefRootPtr, IN char * typeNamePtr)
 *
 * @brief   Tries to resolve typedef to simple type by walking on tree.
 *
 * @param [in]      typedefRootPtr  Pointer to typedef root node.
 * @param [in,out]  typeNamePtr     If non-null, the type name pointer.
 *
 * @return  Pointer to found simple typedef node or NULL.
 *
 * ### param [in]       typedefNamePtr  String describing typedef to resolve.
 */

static PRV_XML_PARSER_TYPEDEF_NODE_STC * prvXmlParserResolve(
    IN PRV_XML_PARSER_TYPEDEF_NODE_STC  * typedefRootPtr, 
    IN char                             * typeNamePtr)
{
    if (typeNamePtr == NULL)
        return NULL;

    while (typedefRootPtr)
    {
        if (strcmp(typeNamePtr, typedefRootPtr->namePtr) == 0)
        {
            if (typedefRootPtr->resolvedPtr)
                return typedefRootPtr->resolvedPtr;

            return typedefRootPtr;
        }

        typedefRootPtr = typedefRootPtr->nextPtr;
    }

    return NULL;
}

/**
 * @fn  static void prvXmlParserGetValueUint8( IN char * strValuePtr, OUT void * valuePtr, OUT PRV_XML_PARSER_SIMPLE_DATA_TYPE_FORMAT_ENT * formatPtr )
 *
 * @brief   Handler for converting string to integer
 *
 * @param [in]  strValuePtr String representation.
 * @param [out] valuePtr    Generic container to store the extracted value.
 * @param [out] formatPtr   Format of the read integer (decimal or hex)
 */

static void prvXmlParserGetValueUint8(
    IN  char                                         * strValuePtr, 
    OUT void                                         * valuePtr, 
    OUT PRV_XML_PARSER_SIMPLE_DATA_TYPE_FORMAT_ENT   * formatPtr
)
{
    UINT_32     intValue;
    if (strlen(strValuePtr) < 3 || strValuePtr[0] != '0' || (strValuePtr[1] != 'x' && strValuePtr[1] != 'X'))
    {
        XML_PARSER_SSCANF_MAC(strValuePtr, "%d", &intValue);
        if (formatPtr)
            *formatPtr = PRV_XML_PARSER_SIMPLE_DATA_FORMAT_DECIMAL_E;
    }
    else
    {
        XML_PARSER_SSCANF_MAC(&strValuePtr[2], "%x", &intValue);
        if (formatPtr)
            *formatPtr = PRV_XML_PARSER_SIMPLE_DATA_FORMAT_HEX_E;
    }
    *(UINT_8 *)valuePtr = (UINT_8)intValue;
}

/**
 * @fn  static void prvXmlParserGetValueUint16( IN char * strValuePtr, OUT void * valuePtr, OUT PRV_XML_PARSER_SIMPLE_DATA_TYPE_FORMAT_ENT * formatPtr )
 *
 * @brief   Handler for converting string to integer
 *
 * @param [in]  strValuePtr String representation.
 * @param [out] valuePtr    Generic container to store the extracted value.
 * @param [out] formatPtr   Format of the read integer (decimal or hex)
 */

static void prvXmlParserGetValueUint16(
    IN  char                                         * strValuePtr, 
    OUT void                                         * valuePtr, 
    OUT PRV_XML_PARSER_SIMPLE_DATA_TYPE_FORMAT_ENT   * formatPtr
)
{
    if (strlen(strValuePtr) < 3 || strValuePtr[0] != '0' || (strValuePtr[1] != 'x' && strValuePtr[1] != 'X'))
    {
        XML_PARSER_SSCANF_MAC(strValuePtr, "%d", (UINT_16 *)valuePtr);
        if (formatPtr)
            *formatPtr = PRV_XML_PARSER_SIMPLE_DATA_FORMAT_DECIMAL_E;
    }
    else
    {
        XML_PARSER_SSCANF_MAC(&strValuePtr[2], "%x", (UINT_16 *)valuePtr);
        if (formatPtr)
            *formatPtr = PRV_XML_PARSER_SIMPLE_DATA_FORMAT_HEX_E;
    }
}

/**
 * @fn  static void prvXmlParserGetValueUint32( IN char * strValuePtr, OUT void * valuePtr, OUT PRV_XML_PARSER_SIMPLE_DATA_TYPE_FORMAT_ENT * formatPtr )
 *
 * @brief   Handler for converting string to integer
 *
 * @param [in]  strValuePtr String representation.
 * @param [out] valuePtr    Generic container to store the extracted value.
 * @param [out] formatPtr   Format of the read integer (decimal or hex)
 */

static void prvXmlParserGetValueUint32(
    IN  char                                         * strValuePtr, 
    OUT void                                         * valuePtr, 
    OUT PRV_XML_PARSER_SIMPLE_DATA_TYPE_FORMAT_ENT   * formatPtr
)
{
    if (strlen(strValuePtr) < 3 || strValuePtr[0] != '0' || (strValuePtr[1] != 'x' && strValuePtr[1] != 'X'))
    {
        XML_PARSER_SSCANF_MAC(strValuePtr, "%lu", (UINT_32 *)valuePtr);
        if (formatPtr)
            *formatPtr = PRV_XML_PARSER_SIMPLE_DATA_FORMAT_DECIMAL_E;
    }
    else
    {
        XML_PARSER_SSCANF_MAC(&strValuePtr[2], "%x", (UINT_32 *)valuePtr);
        if (formatPtr)
            *formatPtr = PRV_XML_PARSER_SIMPLE_DATA_FORMAT_HEX_E;
    }
}

/**
 * @fn  static BOOLEAN prvXmlParserTypeHasRange( IN PRV_XML_PARSER_NODE_STC * typedefPtr, IN PRV_XML_PARSER_SIMPLE_DATA_TYPE_ENT dataType )
 *
 * @brief   Find if type has values restrictions (i.e. minimum and maximum).
 *
 * @param [in]  typedefPtr  Pointer to typedef group.
 * @param [in]  dataType    Simple data type of the group.
 *
 * @return  TRUE if type has range or FALSE if type is all values (According to actual type)
 */

static BOOLEAN prvXmlParserTypeHasRange(
    IN PRV_XML_PARSER_NODE_STC              * typedefPtr, 
    IN PRV_XML_PARSER_SIMPLE_DATA_TYPE_ENT    dataType
)
{
    PRV_XML_PARSER_NODE_STC *childPtr;

    /* check for range/length child */
    childPtr = typedefPtr->firstChildPtr;
    while (childPtr)
    {
        if (((dataType == PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT8_E ||
              dataType == PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT16_E ||
              dataType == PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT32_E) &&
              strcmp(childPtr->namePtr, PRV_XML_PARSER_INTEGER_RANGE_STR_CNS) == 0) ||
            (dataType == PRV_XML_PARSER_SIMPLE_DATA_TYPE_STRING_E &&
             (strcmp(childPtr->namePtr, PRV_XML_PARSER_STRING_LENGTH_STR_CNS) == 0)))
            return TRUE;
        childPtr = childPtr->nextSiblingPtr;
    }

    /* check for min/max tags */
    childPtr = typedefPtr->nextSiblingPtr;
    while (childPtr)
    {
        if (((dataType == PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT8_E ||
             dataType == PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT16_E ||
             dataType == PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT32_E ||
             dataType == PRV_XML_PARSER_SIMPLE_DATA_TYPE_STRING_E) &&
             (strcmp(childPtr->namePtr, PRV_XML_PARSER_MIN_KEYWORD_STR_CNS) == 0 ||
              strcmp(childPtr->namePtr, PRV_XML_PARSER_MAX_KEYWORD_STR_CNS) == 0)) ||
              (dataType == PRV_XML_PARSER_SIMPLE_DATA_TYPE_STRING_E && strcmp(childPtr->namePtr, PRV_XML_PARSER_STRING_LENGTH_STR_CNS) == 0))
            return TRUE;
        childPtr = childPtr->nextSiblingPtr;
    }

    return FALSE;
}

/*****************************************************************************
* FUNCTION NAME: prvXmlParserTypeRangeGet
* @fn  static void prvXmlParserTypeRangeGet( IN char * valueStrPtr, OUT UINT_32 * minPtr, OUT UINT_32 * maxPtr )
*
* @brief   Extract type range (i.e. minimum and maximum values).
*
* @param [in]  valueStrPtr Pointer to typedef values' range (in format of min .. max).
* @param [out] minPtr      Minimal value if exists.
* @param [out] maxPtr      Maximal value if exists.
*****************************************************************************/
static void prvXmlParserTypeRangeGet (

    /*!     INPUTS:             */
    IN  char                * valueStrPtr,
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
    OUT UINT_32             * minPtr,
    OUT UINT_32             * maxPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    char *tagPtr = valueStrPtr;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    /* find separator */
    while (tagPtr && PRV_XML_PARSER_IS_WHITE_SPACE_MAC(*tagPtr)) tagPtr++;
    while (tagPtr && *tagPtr && *tagPtr != '.') tagPtr++;
    if (tagPtr && *tagPtr == '.' && ((tagPtr+1) != NULL) && (*(tagPtr+1) == '.'))
    {
        *tagPtr = 0;
        prvXmlParserGetValueUint32(valueStrPtr, minPtr, NULL);
        tagPtr = tagPtr + 2;
        while (tagPtr && PRV_XML_PARSER_IS_WHITE_SPACE_MAC(*tagPtr)) tagPtr++;
        if (tagPtr)
            prvXmlParserGetValueUint32(tagPtr, maxPtr, NULL);
    }
}
/*$ END OF prvXmlParserTypeRangeGet */

/**
 * @fn  static void prvXmlParserTypeRangeMinMaxGet( IN PRV_XML_PARSER_NODE_STC * typedefPtr, OUT UINT_32 * minPtr, OUT UINT_32 * maxPtr )
 *
 * @brief   Extract type range (i.e. minimum and maximum values).
 *
 * @param [in]  typedefPtr  Pointer to typedef group.
 * @param [out] minPtr      Minimal value if exists.
 * @param [out] maxPtr      Maximal value if exists.
 */

static void prvXmlParserTypeRangeMinMaxGet(
    IN PRV_XML_PARSER_NODE_STC              * typedefPtr, 
    OUT UINT_32                             * minPtr,
    OUT UINT_32                             * maxPtr
)
{
    PRV_XML_PARSER_NODE_STC *childPtr = typedefPtr->firstChildPtr;

    if (childPtr)
    {
        if (strcmp(childPtr->namePtr, PRV_XML_PARSER_INTEGER_RANGE_STR_CNS) == 0 || strcmp(childPtr->namePtr, PRV_XML_PARSER_STRING_LENGTH_STR_CNS) == 0)
        {
            childPtr = childPtr->firstChildPtr;
            while (childPtr && strcmp(childPtr->namePtr, "value"))
                childPtr = childPtr->nextSiblingPtr;

            if (childPtr && childPtr->valuePtr)
            {
                /* find separator */
                prvXmlParserTypeRangeGet(childPtr->valuePtr, minPtr, maxPtr);
            }
            return;
        }
    }
    else
    {
        childPtr = typedefPtr->nextSiblingPtr;
        while (childPtr)
        {
            if (strcmp(childPtr->namePtr, PRV_XML_PARSER_MIN_KEYWORD_STR_CNS) == 0 && childPtr->valuePtr)
            {
                prvXmlParserGetValueUint32(childPtr->valuePtr, minPtr, NULL);
            }
            else if (strcmp(childPtr->namePtr, PRV_XML_PARSER_MAX_KEYWORD_STR_CNS) == 0 && childPtr->valuePtr)
            {
                prvXmlParserGetValueUint32(childPtr->valuePtr, maxPtr, NULL);
            }
            else if (strcmp(childPtr->namePtr, PRV_XML_PARSER_STRING_LENGTH_STR_CNS) == 0 && childPtr->firstChildPtr)
            {
                prvXmlParserTypeRangeGet(childPtr->firstChildPtr->valuePtr, minPtr, maxPtr);
                return;
            }

            childPtr = childPtr->nextSiblingPtr;
        }
    }
}

/**
 * @fn  static PRV_XML_PARSER_TYPEDEF_NODE_STC * prvXmlParserSimpleTypeGet( IN PRV_XML_PARSER_TYPEDEF_NODE_STC * typedefRootPtr, IN PRV_XML_PARSER_SIMPLE_DATA_TYPE_ENT dataType )
 *
 * @brief   Get simple data type node. No data validation is required.
 *
 * @param [in]  typedefRootPtr  Pointer to typedef root.
 * @param [in]  dataType        Simple data type to get.
 *
 * @return  Null if it fails, else a pointer to a PRV_XML_PARSER_TYPEDEF_NODE_STC.
 */

static PRV_XML_PARSER_TYPEDEF_NODE_STC * prvXmlParserSimpleTypeGet(
    IN PRV_XML_PARSER_TYPEDEF_NODE_STC        * typedefRootPtr, 
    IN PRV_XML_PARSER_SIMPLE_DATA_TYPE_ENT      dataType
)
{
    switch (dataType)
    {
        case PRV_XML_PARSER_SIMPLE_DATA_TYPE_EMPTY_E:
            return typedefRootPtr;
        case PRV_XML_PARSER_SIMPLE_DATA_TYPE_BOOLEAN_E:
            return typedefRootPtr->nextPtr;
        case PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT8_E:
            return typedefRootPtr->nextPtr->nextPtr;
        case PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT16_E:
            return typedefRootPtr->nextPtr->nextPtr->nextPtr;
        case PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT32_E:
            return typedefRootPtr->nextPtr->nextPtr->nextPtr->nextPtr;
        case PRV_XML_PARSER_SIMPLE_DATA_TYPE_STRING_E:
            return typedefRootPtr->nextPtr->nextPtr->nextPtr->nextPtr->nextPtr;
        case PRV_XML_PARSER_SIMPLE_DATA_TYPE_NONE_E:
        case PRV_XML_PARSER_SIMPLE_DATA_TYPE_ENUM_E:
        case PRV_XML_PARSER_SIMPLE_DATA_TYPE_BITS_E:
            break;
    }

    /* to prevent compiler warnings */
    return NULL;
}

/**
 * @fn  static PRV_XML_PARSER_TYPEDEF_NODE_STC * prvXmlParserEnumAdd( IN PRV_XML_PARSER_NODE_STC * enumNodePtr, IN UINT_32 index, PRV_XML_PARSER_TYPEDEF_DATA_UNT * dataPtr)
 *
 * @brief   Get enum data type node.
 *
 * @param [in]  enumNodePtr  Pointer to node containing enum data.
 * @param [in]  index        index to array.
 * @param [out] dataPtr      data to return.
 *
 * @return  Null if it fails, else a pointer to a PRV_XML_PARSER_TYPEDEF_NODE_STC.
 */
static BOOLEAN prvXmlParserEnumAdd (

    /*!     INPUTS:             */
    IN  PRV_XML_PARSER_NODE_STC           * enumNodePtr,
    IN  UINT_32                             index,

    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
    OUT PRV_XML_PARSER_TYPEDEF_DATA_UNT   * dataPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PRV_XML_PARSER_NODE_STC   * childPtr = enumNodePtr->firstChildPtr;
    BOOLEAN                     name = FALSE, value = FALSE;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    while (childPtr)
    {
        if (childPtr && strcmp(childPtr->namePtr, "name") == 0)
        {
            dataPtr->enum_value.strEnumArr[index] = XML_PARSER_STRDUP_MAC(childPtr->valuePtr);
            name = TRUE;
        }
        else if (childPtr && strcmp(childPtr->namePtr, "value") == 0)
        {
            prvXmlParserGetValueUint8(childPtr->valuePtr, &dataPtr->enum_value.intEnumArr[index], NULL);
            value = TRUE;
        }

        childPtr = childPtr->nextSiblingPtr;
    }

    if (name == FALSE || value == FALSE)
    {
        XML_PARSER_DEBUG_LOG(("\"enum\" is not complete \n"));
        return FALSE;
    }

    return TRUE;
}

/**
 * @fn  static PRV_XML_PARSER_TYPEDEF_NODE_STC * prvXmlParserBitAdd( IN PRV_XML_PARSER_NODE_STC * bitNodePtr, IN UINT_32 index, PRV_XML_PARSER_TYPEDEF_DATA_UNT * dataPtr)
 *
 * @brief   Get single bit data type node.
 *
 * @param [in]  bitNodePtr   Pointer to node containing bit data.
 * @param [in]  index        index to array.
 * @param [out] dataPtr      data to return.
 *
 * @return  Null if it fails, else a pointer to a PRV_XML_PARSER_TYPEDEF_NODE_STC.
 */
static BOOLEAN prvXmlParserBitAdd (

    /*!     INPUTS:             */
    IN  PRV_XML_PARSER_NODE_STC           * bitNodePtr,
    IN  UINT_32                             index,

    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
    OUT PRV_XML_PARSER_TYPEDEF_DATA_UNT   * dataPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PRV_XML_PARSER_NODE_STC   * childPtr = bitNodePtr->firstChildPtr;
    BOOLEAN                     name = FALSE, value = FALSE;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    while (childPtr)
    {
        if (childPtr && strcmp(childPtr->namePtr, "name") == 0)
        {
            dataPtr->bits_value.strBitArr[index] = XML_PARSER_STRDUP_MAC(childPtr->valuePtr);
            name = TRUE;
        }
        else if (childPtr && strcmp(childPtr->namePtr, "position") == 0)
        {
            prvXmlParserGetValueUint32(childPtr->valuePtr, &dataPtr->bits_value.intBitArr[index], NULL);
            value = TRUE;
        }

        childPtr = childPtr->nextSiblingPtr;
    }

    if (name == FALSE || value == FALSE)
    {
        XML_PARSER_DEBUG_LOG(("\"bit\" is not complete \n"));
        return FALSE;
    }

    return TRUE;
}

/**
 * @fn  static BOOLEAN prvXmlParserHandleSingleTypedef( IN PRV_XML_PARSER_TYPEDEF_NODE_STC * typedefRootPtr, IN PRV_XML_PARSER_TYPEDEF_NODE_STC * currentTypedefPtr, IN PRV_XML_PARSER_NODE_STC * typedefXmlPtr )
 *
 * @brief   Parse xml node containing single typedef and create new fully resolved typedef node.
 *
 * @param [in]  typedefRootPtr      Pointer to typedef root.
 * @param [in]  currentTypedefPtr   Pointer to last added typedef node.
 * @param [in]  typedefXmlPtr       Pointer to xml node containing type to parse.
 *
 * @return  FALSE if invalid type was found or memory allocation failed.
 */

static BOOLEAN prvXmlParserHandleSingleTypedef(
    IN PRV_XML_PARSER_TYPEDEF_NODE_STC *  typedefRootPtr,
    IN PRV_XML_PARSER_TYPEDEF_NODE_STC  * currentTypedefPtr,
    IN PRV_XML_PARSER_NODE_STC          * typedefXmlPtr
)
{
    PRV_XML_PARSER_SIMPLE_DATA_TYPE_ENT data_type;
    PRV_XML_PARSER_TYPEDEF_NODE_STC *resolvedTypePtr;
    PRV_XML_PARSER_NODE_STC         *childPtr;
    PRV_XML_PARSER_TYPEDEF_NODE_STC *new_type;
    UINT_32                          count, min, max;
    char                            *typeNamePtr, *typeTypePtr;

    if (typedefXmlPtr == NULL || 
        typedefXmlPtr->firstChildPtr == NULL ||
        strcmp(typedefXmlPtr->firstChildPtr->namePtr, "name") ||
        typedefXmlPtr->firstChildPtr->valuePtr == NULL)
            return FALSE;

    typeNamePtr = typedefXmlPtr->firstChildPtr->valuePtr;
    childPtr = prvXmlParseFindTagByName(typedefXmlPtr, "type", TRUE);
    if (childPtr == NULL)
    {
        XML_PARSER_DEBUG_LOG(("No \"type\" in %s \n", typeNamePtr));
        return FALSE;
    }
    typeTypePtr = childPtr->valuePtr;

    data_type = prvXmlParserIsSimpleType(typeTypePtr);
    if (prvXmlParserTypeHasRange(typedefXmlPtr->firstChildPtr, data_type) == FALSE)
    {
        switch (data_type)
        {
            case PRV_XML_PARSER_SIMPLE_DATA_TYPE_EMPTY_E:
            case PRV_XML_PARSER_SIMPLE_DATA_TYPE_BOOLEAN_E:
            case PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT8_E:
            case PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT16_E:
            case PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT32_E:
            case PRV_XML_PARSER_SIMPLE_DATA_TYPE_STRING_E:
                currentTypedefPtr->nextPtr = prvXmlParserResolvedTypedefAdd(typeNamePtr, prvXmlParserSimpleTypeGet(typedefRootPtr, data_type));
                break;
            case PRV_XML_PARSER_SIMPLE_DATA_TYPE_NONE_E:
                /* try to resolve complex data type */
                resolvedTypePtr = prvXmlParserResolve(typedefRootPtr, typeTypePtr);
                if (resolvedTypePtr)
                {
                    currentTypedefPtr->nextPtr = prvXmlParserResolvedTypedefAdd(typeNamePtr, resolvedTypePtr);
                    return TRUE;
                }
                return FALSE;
            case PRV_XML_PARSER_SIMPLE_DATA_TYPE_ENUM_E:
                count = 0;
                childPtr = typedefXmlPtr->firstChildPtr;
                while (childPtr)
                {
                    if (strcmp(childPtr->namePtr, "enum") == 0)
                    {
                        count ++;
                    }

                    childPtr = childPtr->nextSiblingPtr;
                }

                if (count == 0)
                {
                    XML_PARSER_DEBUG_LOG(("No \"enum\" values in %s \n", typeNamePtr));
                    return FALSE;
                }

                new_type = prvXmlParserSimpleTypedefAdd(typeNamePtr, PRV_XML_PARSER_SIMPLE_DATA_TYPE_ENUM_E, 0, 0);
                if (new_type == NULL) {
                	XML_PARSER_DEBUG_LOG(("Out Of Memory \n"));
                	return FALSE;
                }
                new_type->data.enum_value.arr_size = count;
                new_type->data.enum_value.strEnumArr = (char **)prvPdlOsMalloc(count * sizeof(char *));
                new_type->data.enum_value.intEnumArr = (UINT_8 *)prvPdlOsMalloc(count * sizeof(UINT_8));
                if (new_type->data.enum_value.strEnumArr == NULL || new_type->data.enum_value.intEnumArr == NULL)
                    break;

                childPtr = typedefXmlPtr->firstChildPtr;
                count = 0;
                while (childPtr)
                {
                    if (strcmp(childPtr->namePtr, "enum") == 0)
                    {
                        if (FALSE == prvXmlParserEnumAdd(childPtr, count, &new_type->data))
                        {
                            XML_PARSER_DEBUG_LOG(("No \"enum\" value in %s \n", typeNamePtr));
                            return FALSE;
                        }
                        count ++;
                    }
                    childPtr = childPtr->nextSiblingPtr;
                }

                currentTypedefPtr->nextPtr = new_type;
                break;
            case PRV_XML_PARSER_SIMPLE_DATA_TYPE_BITS_E:
                count = 0;
                childPtr = typedefXmlPtr->firstChildPtr;
                while (childPtr)
                {
                    if (strcmp(childPtr->namePtr, "bit") == 0)
                    {
                        count ++;
                    }

                    childPtr = childPtr->nextSiblingPtr;
                }

                if (count == 0)
                {
                    XML_PARSER_DEBUG_LOG(("No \"bit\" values in %s \n", typeNamePtr));
                    return FALSE;
                }

                new_type = prvXmlParserSimpleTypedefAdd(typeNamePtr, PRV_XML_PARSER_SIMPLE_DATA_TYPE_BITS_E, 0, 0);
                if (new_type == NULL) {
                    XML_PARSER_DEBUG_LOG(("Out Of Memory \n"));
                    return FALSE;
                }
                new_type->data.bits_value.arr_size = count;
                new_type->data.bits_value.strBitArr = (char **)prvPdlOsMalloc(count * sizeof(char *));
                new_type->data.bits_value.intBitArr = (UINT_32 *)prvPdlOsMalloc(count * sizeof(UINT_32));
                if (new_type->data.bits_value.strBitArr == NULL || new_type->data.bits_value.intBitArr == NULL)
                    break;

                childPtr = typedefXmlPtr->firstChildPtr;
                count = 0;
                while (childPtr)
                {
                    if (strcmp(childPtr->namePtr, "bit") == 0)
                    {
                        if (FALSE == prvXmlParserBitAdd(childPtr, count, &new_type->data))
                        {
                            XML_PARSER_DEBUG_LOG(("No \"bit\" value in %s \n", typeNamePtr));
                            return FALSE;
                        }
                        count ++;
                    }
                    childPtr = childPtr->nextSiblingPtr;
                }

                currentTypedefPtr->nextPtr = new_type;
                break;
        }
    }
    else
    {
        /* for data type having range is definitely one of the simple data types */
        prvXmlParserTypeRangeMinMaxGet(typedefXmlPtr->firstChildPtr, &min, &max);
        currentTypedefPtr->nextPtr = prvXmlParserSimpleTypedefAdd(typeNamePtr, data_type, min, max);
    }

    return currentTypedefPtr->nextPtr ? TRUE : FALSE;
}

/**
 * @fn  static PRV_XML_PARSER_TYPEDEF_NODE_STC * prvXmlParserTypedefTreeBuild( IN PRV_XML_PARSER_NODE_STC * typedefXmlPtr )
 *
 * @brief   Parse xml node containing single typedef and create new fully resolved typedef node.
 *
 * @param [in]  typedefXmlPtr   Pointer to xml node containing typedefs group.
 *
 * @return  Pointer to typedef list.
 */

static PRV_XML_PARSER_TYPEDEF_NODE_STC * prvXmlParserTypedefTreeBuild(
    IN PRV_XML_PARSER_NODE_STC      * typedefXmlPtr
)
{
    PRV_XML_PARSER_NODE_STC         * node = typedefXmlPtr->firstChildPtr;
    PRV_XML_PARSER_TYPEDEF_NODE_STC * type, * typedef_root;

    /* add all simple data types */
    type = typedef_root = prvXmlParserSimpleTypedefAdd(PRV_XML_PARSER_TYPE_EMPTY_STR_CNS, PRV_XML_PARSER_SIMPLE_DATA_TYPE_EMPTY_E, 0, 0);
    if (! typedef_root)
        return NULL;
    type->nextPtr = prvXmlParserSimpleTypedefAdd(PRV_XML_PARSER_TYPE_BOOLEAN_STR_CNS,   PRV_XML_PARSER_SIMPLE_DATA_TYPE_BOOLEAN_E,  0, 0);          type = type->nextPtr;
    type->nextPtr = prvXmlParserSimpleTypedefAdd(PRV_XML_PARSER_TYPE_UINT8_STR_CNS,     PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT8_E,    0, 255);        type = type->nextPtr;
    type->nextPtr = prvXmlParserSimpleTypedefAdd(PRV_XML_PARSER_TYPE_UINT16_STR_CNS,    PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT16_E,   0, 65535);      type = type->nextPtr;
    type->nextPtr = prvXmlParserSimpleTypedefAdd(PRV_XML_PARSER_TYPE_UINT32_STR_CNS,    PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT32_E,   0, 0xFFFFFFFF); type = type->nextPtr;
    type->nextPtr = prvXmlParserSimpleTypedefAdd(PRV_XML_PARSER_TYPE_STRING_STR_CNS,    PRV_XML_PARSER_SIMPLE_DATA_TYPE_STRING_E,   0, 0);          type = type->nextPtr;

    /* resolve all other types */
    while (node)
    {
        if (strcmp(node->namePtr, "typedef") == 0)
        {
            if (prvXmlParserHandleSingleTypedef(typedef_root, type, node))
                type = type->nextPtr;
        }
        node = node->nextSiblingPtr;
    }
    return typedef_root;
}

/**
 * @fn  static void prvXmlParserTypedefTreeDestroy( IN PRV_XML_PARSER_TYPEDEF_NODE_STC * typedefRootPtr )
 *
 * @brief   Walk over typedefs tree and free all allocated memory.
 *
 * @param [in]  typedefRootPtr  Pointer to typedefs tree root.
 */

static void prvXmlParserTypedefTreeDestroy(
    IN PRV_XML_PARSER_TYPEDEF_NODE_STC * typedefRootPtr
)
{
    PRV_XML_PARSER_TYPEDEF_NODE_STC   * nextPtr;
    UINT_8                              index;

    while (typedefRootPtr)
    {
        nextPtr = typedefRootPtr->nextPtr;

        if (typedefRootPtr->namePtr)
            prvPdlOsFree(typedefRootPtr->namePtr);
        if (typedefRootPtr->type == PRV_XML_PARSER_SIMPLE_DATA_TYPE_ENUM_E)
        {
            if (typedefRootPtr->data.enum_value.intEnumArr)
                prvPdlOsFree(typedefRootPtr->data.enum_value.intEnumArr);
            if (typedefRootPtr->data.enum_value.strEnumArr)
            {
                for (index = 0; index < typedefRootPtr->data.enum_value.arr_size; index++)
                {
                    if (typedefRootPtr->data.enum_value.strEnumArr[index])
                        prvPdlOsFree(typedefRootPtr->data.enum_value.strEnumArr[index]);
                }
                prvPdlOsFree(typedefRootPtr->data.enum_value.strEnumArr);
            }
        }
        else if (typedefRootPtr->type == PRV_XML_PARSER_SIMPLE_DATA_TYPE_BITS_E)
        {
            if (typedefRootPtr->data.bits_value.intBitArr)
                prvPdlOsFree(typedefRootPtr->data.bits_value.intBitArr);
            if (typedefRootPtr->data.bits_value.strBitArr)
            {
                for (index = 0; index < typedefRootPtr->data.bits_value.arr_size; index++)
                {
                    if (typedefRootPtr->data.bits_value.strBitArr[index])
                        prvPdlOsFree(typedefRootPtr->data.bits_value.strBitArr[index]);
                }
                prvPdlOsFree(typedefRootPtr->data.bits_value.strBitArr);
            }
        }


        prvPdlOsFree(typedefRootPtr);

        typedefRootPtr = nextPtr;
    }
}

/**
 * @fn  static BOOLEAN prvXmlParserResolveType ( IN PRV_XML_PARSER_NODE_STC ** nodePtr, IN void * cookiePtr )
 *
 * @brief   Find (resolve) type of each leaf and validate its value
 *
 * @param [in]  nodePtr     Pointer to xml node.
 * @param [in]  cookiePtr   Pointer to typedefs tree.
 *
 * @return  True if it succeeds, false if it fails.
 */

static BOOLEAN prvXmlParserResolveType (
    IN PRV_XML_PARSER_NODE_STC ** nodePtr, 
    IN void                     * cookiePtr
)
{
    UINT_32                                       i, len;
    PRV_XML_PARSER_NODE_STC                     * tmp = *nodePtr;
    PRV_XML_PARSER_TYPEDEF_NODE_STC             * typedefs = (PRV_XML_PARSER_TYPEDEF_NODE_STC *)cookiePtr, *act_typedef;
    PRV_XML_PARSER_INT_VALUE_GET_HANDLER_PTR      getIntValueAPiPtr;
    char                                        * pch, * token;

    if (tmp->type == PRV_XML_PARSER_NODE_TYPE_LEAF_E)
    {
        /* find type of the node in typedefs tree */
        tmp->auxData.typedefPtr = prvXmlParserResolve(typedefs, tmp->auxData.attrNamePtr);
        if (tmp->auxData.typedefPtr)
        {
            /* validate data of the leaf */
            act_typedef = tmp->auxData.typedefPtr;
            if (act_typedef->resolvedPtr)
                act_typedef = act_typedef->resolvedPtr;
            switch (act_typedef->type)
            {
                case PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT8_E:
                case PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT16_E:
                case PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT32_E:
                    if (act_typedef->type == PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT8_E)
                        getIntValueAPiPtr = prvXmlParserGetValueUint8;
                    else if (act_typedef->type == PRV_XML_PARSER_SIMPLE_DATA_TYPE_UINT16_E)
                        getIntValueAPiPtr = prvXmlParserGetValueUint16;
                    else
                        getIntValueAPiPtr = prvXmlParserGetValueUint32;
                    getIntValueAPiPtr(tmp->valuePtr, &tmp->auxData.data.value, &tmp->auxData.data.format);
                    if ((unsigned long)act_typedef->data.int_value.min > tmp->auxData.data.value.uint32Value || (unsigned long)act_typedef->data.int_value.max < tmp->auxData.data.value.uint32Value)
                    {
                        tmp->isValid = FALSE;
                        XML_PARSER_DEBUG_LOG((" INVALID LEAF <%s> FOUND: TYPE [%s] VALUE = [%s(%d)] ACTUAL RANGE <%d,%d>\n", 
                            tmp->namePtr, act_typedef->namePtr, tmp->valuePtr, tmp->auxData.data.value.uint32Value, (unsigned long)act_typedef->data.int_value.min, (unsigned long)act_typedef->data.int_value.max));
                    }
                    break;
                case PRV_XML_PARSER_SIMPLE_DATA_TYPE_BOOLEAN_E:
                    if (strcmp(tmp->valuePtr, PRV_XML_PARSER_BOOLEAN_STR_FALSE_CNS) == 0)
                        tmp->auxData.data.value.boolValue = FALSE;
                    else if (strcmp(tmp->valuePtr, PRV_XML_PARSER_BOOLEAN_STR_TRUE_CNS) == 0)
                        tmp->auxData.data.value.boolValue = TRUE;
                    else
                    {
                        tmp->isValid = FALSE;
                        XML_PARSER_DEBUG_LOG((" INVALID LEAF <%s> FOUND: VALUE = [%s] ACTUAL RANGE <%s,%s>\n", 
                            tmp->namePtr, tmp->valuePtr, PRV_XML_PARSER_BOOLEAN_STR_FALSE_CNS, PRV_XML_PARSER_BOOLEAN_STR_TRUE_CNS));
                    }
                    break;
                case PRV_XML_PARSER_SIMPLE_DATA_TYPE_STRING_E:
                    tmp->auxData.data.value.strOrIntValue.stringValuePtr = tmp->valuePtr;
                    len = strlen(tmp->valuePtr);
                    if ((act_typedef->data.string_value.min != 0 || act_typedef->data.string_value.max != 0) && 
                        (act_typedef->data.string_value.min > len || act_typedef->data.string_value.max < len))
                    {
                        tmp->isValid = FALSE;
                        XML_PARSER_DEBUG_LOG((" INVALID LEAF <%s> FOUND: VALUE = [%s] LENGTH [%d] ACTUAL RANGE <%d,%d>\n", 
                            tmp->namePtr, tmp->valuePtr, len, act_typedef->data.string_value.min, act_typedef->data.string_value.max));
                    }
                    break;
                case PRV_XML_PARSER_SIMPLE_DATA_TYPE_ENUM_E:
                    tmp->auxData.data.value.strOrIntValue.stringValuePtr = tmp->valuePtr;
                    for (i = 0; i < act_typedef->data.enum_value.arr_size; i++)
                    {
                        if (strcmp(tmp->valuePtr, act_typedef->data.enum_value.strEnumArr[i]) == 0)
                        {
                            tmp->auxData.data.value.strOrIntValue.uint32Value = (UINT_32)act_typedef->data.enum_value.intEnumArr[i];
                            break;
                        }
                    }
                    if (i == act_typedef->data.enum_value.arr_size)
                    {
                        tmp->isValid = FALSE;
                        XML_PARSER_DEBUG_LOG((" INVALID LEAF <%s> FOUND: VALUE = [%s] ACTUAL RANGE <", tmp->namePtr, tmp->valuePtr));
                        for (i = 0; i < act_typedef->data.enum_value.arr_size; i++)
                        {
                            XML_PARSER_DEBUG_LOG(("%s", act_typedef->data.enum_value.strEnumArr[i]));
                            if (i < act_typedef->data.enum_value.arr_size - 1)
                                XML_PARSER_DEBUG_LOG((" , "));
                        }
                        XML_PARSER_DEBUG_LOG((">\n"));
                    }
                    break;
                case PRV_XML_PARSER_SIMPLE_DATA_TYPE_BITS_E:
                    tmp->auxData.data.value.strOrIntValue.stringValuePtr = tmp->valuePtr;
                    tmp->auxData.data.value.strOrIntValue.uint32Value = 0;
                    /* parse all values */
                    token = XML_PARSER_STRDUP_MAC(tmp->valuePtr);
                    token = XML_PARSER_STRTOK_MAC(token, " ", &pch);
                    while (token)
                    {
                        for (i = 0; i < act_typedef->data.bits_value.arr_size; i++)
                        {
                            if (strcmp(token, act_typedef->data.bits_value.strBitArr[i]) == 0)
                            {
                                tmp->auxData.data.value.strOrIntValue.uint32Value |= (1 << act_typedef->data.bits_value.intBitArr[i]);
                                break;
                            }
                        }
                        if (i == act_typedef->data.bits_value.arr_size)
                        {
                            tmp->isValid = FALSE;
                            XML_PARSER_DEBUG_LOG((" INVALID LEAF <%s> FOUND: VALUE = [%s] ACTUAL RANGE <", tmp->namePtr, token));
                            for (i = 0; i < act_typedef->data.bits_value.arr_size; i++)
                            {
                                XML_PARSER_DEBUG_LOG(("%s", act_typedef->data.bits_value.strBitArr[i]));
                                if (i < act_typedef->data.bits_value.arr_size - 1)
                                    XML_PARSER_DEBUG_LOG((" , "));
                            }
                            XML_PARSER_DEBUG_LOG((">\n"));
                        }
                        token = XML_PARSER_STRTOK_MAC(NULL, " ", &pch);
                    }
                    break;
                default:
                    break;
            }
        }
        else
        {
            /* type wasn't found */
            tmp->isValid = FALSE;
        }
    }

    return FALSE;
}

/**
 * @fn  static PRV_XML_PARSER_NODE_STC * prvXmlParserTraverseTree( IN PRV_XML_PARSER_NODE_STC * rootPtr, IN PRV_XML_PARSER_TRAVERSE_HANDLER_PTR handlerFuncPtr, IN PRV_XML_PARSER_CALL_ORDER_ENT order, IN void * cookiePtr )
 *
 * @brief   Walk over tree and call handler. Tree structure: ROOT |=> (firsChildPtr)  NODE -------
 *          --|=> (firsChildPtr)  NODE
 *          |                    ||           |                    ||
 *          |                    ||           |                    || (nextSiblingPtr)
 *          |             (nextSiblingPtr)    |                    \/
 *          |                    ||           |                   NODE
 *          |                    ||           |                    ||
 *          |                    \/           |                    || (nextSiblingPtr)
 *          |                   NODE          |                    \/
 *          |                    ||           |=> (lastChildPtr)  NODE
 *          |              (nextSiblingPtr)
 *          |                    ||
 *          |                    \/
 *          |=> (lastChildPtr)  NODE
 *
 * @param [in]  rootPtr         Pointer to xml tree root.
 * @param [in]  handlerFuncPtr  Pointer to handler to call for each node.
 * @param [in]  order           When to call the handler (on entering the node or on exit)
 * @param [in]  cookiePtr       Caller specific data.
 *
 * @return  Pointer to current node if handler returns TRUE.
 */

static PRV_XML_PARSER_NODE_STC * prvXmlParserTraverseTree(
    IN PRV_XML_PARSER_NODE_STC                * rootPtr,
    IN PRV_XML_PARSER_TRAVERSE_HANDLER_PTR      handlerFuncPtr,
    IN PRV_XML_PARSER_CALL_ORDER_ENT            order,
    IN void                                   * cookiePtr
)
{
    PRV_XML_PARSER_NODE_STC * nodePtr = rootPtr, *nextPtr;

    if (rootPtr->type == PRV_XML_PARSER_NODE_TYPE_LEAF_E)
    {
        if (TRUE == handlerFuncPtr(&nodePtr, cookiePtr))
            return nodePtr;
        return NULL;
    }

    while (nodePtr)
    {
        if (order == PRV_XML_PARSER_CALL_ON_ENTER_E)
        {
            if (TRUE == handlerFuncPtr(&nodePtr, cookiePtr))
                return nodePtr;
        }

        /* first go to child */
        if (nodePtr->firstChildPtr)
        {
            nodePtr = nodePtr->firstChildPtr;
        }
        /* next go to sibling */
        else if (nodePtr->nextSiblingPtr)
        {
            nextPtr = nodePtr->nextSiblingPtr;
            if (order == PRV_XML_PARSER_CALL_ON_EXIT_E)
            {
                if (TRUE == handlerFuncPtr(&nodePtr, cookiePtr))
                    return nodePtr;
            }

            nodePtr = nextPtr;
        }
        /* last go up to parent(s) */
        else
        {
            while (nodePtr && nodePtr != rootPtr)
            {
                if (nodePtr->parentPtr)
                {
                    nextPtr = nodePtr->parentPtr;

                    if (order == PRV_XML_PARSER_CALL_ON_EXIT_E)
                    {
                        if (TRUE == handlerFuncPtr(&nodePtr, cookiePtr))
                            return nodePtr;
                    }

                    nodePtr = nextPtr;
                }

                if (!nodePtr || nodePtr == rootPtr)
                    break;

                /* go to parent sibling next */
                if (nodePtr->nextSiblingPtr)
                {
                    nextPtr = nodePtr->nextSiblingPtr;
                    if (order == PRV_XML_PARSER_CALL_ON_EXIT_E)
                    {
                        if (TRUE == handlerFuncPtr(&nodePtr, cookiePtr))
                            return nodePtr;
                    }

                    nodePtr = nextPtr;
                    break;
                }
            }
            if (!nodePtr || nodePtr == rootPtr)
                break;
        }
    }

    return NULL;
}

/**
 * @fn  static BOOLEAN prvXmlParserIsTagName ( IN PRV_XML_PARSER_NODE_STC ** nodePtr, IN void * cookiePtr )
 *
 * @brief   Compare xml node name to string.
 *
 * @param [in]  nodePtr     Pointer to xml node.
 * @param [in]  cookiePtr   String to compare to.
 *
 * @return  TRUE if node's name equals to searched string.
 */

static BOOLEAN prvXmlParserIsTagName (
    IN PRV_XML_PARSER_NODE_STC      ** nodePtr,
    IN void                          * cookiePtr
)
{
    PRV_XML_PARSER_SEARCH_INFO_STC *miscInfoPTR = (PRV_XML_PARSER_SEARCH_INFO_STC *)cookiePtr;
    if (miscInfoPTR->isCaseSensitive)
    {
        if (strcmp((*nodePtr)->namePtr, (char *)miscInfoPTR->strPtr) == 0)
            return TRUE;
    }
    else
    {
        if (XML_PARSER_STRICMP_MAC((*nodePtr)->namePtr, (char *)miscInfoPTR->strPtr) == 0)
            return TRUE;
    }

    return FALSE;
}

/**
 * @fn  static BOOLEAN prvXmlParserIsNodeValid ( IN PRV_XML_PARSER_NODE_STC ** nodePtr, IN void * cookiePtr )
 *
 * @brief   Checks if valid field is set.
 *
 * @param [in]  nodePtr     Pointer to xml node.
 * @param [in]  cookiePtr   Not used.
 *
 * @return  TRUE for invalid node.
 */

static BOOLEAN prvXmlParserIsNodeValid (
    IN PRV_XML_PARSER_NODE_STC      ** nodePtr,
    IN void                          * cookiePtr
)
{
    PDL_UNUSED_PARAM(cookiePtr);

    if ((*nodePtr)->isValid == FALSE)
    {
        XML_PARSER_DEBUG_LOG(("found illegal node %s\n", (*nodePtr)->namePtr));
        return TRUE;
    }

    return FALSE;
}

/**
 * @fn  static BOOLEAN prvXmlParserNodeCount ( IN PRV_XML_PARSER_NODE_STC ** nodePtr, IN void * cookiePtr )
 *
 * @brief   Count number of nodes whose name matching searched string.
 *
 * @param [in]  nodePtr     Pointer to xml node.
 * @param [in]  cookiePtr   String to compare to.
 *
 * @return  True if it succeeds, false if it fails.
 */

static BOOLEAN prvXmlParserNodeCount (
    IN PRV_XML_PARSER_NODE_STC      ** nodePtr,
    IN void                          * cookiePtr
)
{
    XML_GET_TAG_COUNT_STC * dataPtr = (XML_GET_TAG_COUNT_STC *)cookiePtr;

    if (strcmp((*nodePtr)->namePtr, dataPtr->namePtr) == 0)
        dataPtr->count ++;

    return FALSE;
}

/**
 * @fn  static BOOLEAN prvXmlParserNodeDestroy ( IN PRV_XML_PARSER_NODE_STC ** nodePtr, IN void * cookiePtr )
 *
 * @brief   Releases all memory allocated for this node.
 *
 * @param [in]  nodePtr     Pointer to xml node.
 * @param [in]  cookiePtr   Not used.
 *
 * @return  True if it succeeds, false if it fails.
 */

static BOOLEAN prvXmlParserNodeDestroy (
    IN PRV_XML_PARSER_NODE_STC      ** nodePtr,
    IN void                          * cookiePtr
)
{
    PRV_XML_PARSER_NODE_STC *dataPtr = *nodePtr;

    PDL_UNUSED_PARAM(cookiePtr);

    if (dataPtr == NULL)
        return FALSE;

    if (dataPtr->namePtr)
        prvPdlOsFree(dataPtr->namePtr);

    if (dataPtr->auxData.attrNamePtr)
        prvPdlOsFree(dataPtr->auxData.attrNamePtr);
    
    if (dataPtr->valuePtr)
        prvPdlOsFree(dataPtr->valuePtr);

    prvPdlOsFree(dataPtr);

    return FALSE;
}

/** Service routines for application interfaces */

/**
 * @fn  PRV_XML_PARSER_ROOT_STC * prvXmlParserBuilderRootCreate ( IN char * rootNamePtr, IN char * xmlFilePathPtr )
 *
 * @brief   Creates root node for database.
 *
 * @param [in]  rootNamePtr     XML root node name.
 * @param [in]  xmlFilePathPtr  XML file name used to build database.
 *
 * @return  NULL if root couldn't be created.
 */

PRV_XML_PARSER_ROOT_STC * prvXmlParserBuilderRootCreate (
    IN char * rootNamePtr,
    IN char * tagPefixPtr,
    IN char * xmlFilePathPtr
)
{
    PRV_XML_PARSER_ROOT_STC * rootPtr = (PRV_XML_PARSER_ROOT_STC *)prvPdlOsCalloc(1, sizeof(PRV_XML_PARSER_ROOT_STC));
    PRV_XML_PARSER_NODE_STC * nodePtr;

    if (!rootPtr)
        return NULL;

    nodePtr = &rootPtr->node;

    nodePtr->namePtr = XML_PARSER_STRDUP_MAC(rootNamePtr);
    if (!nodePtr->namePtr)
    {
        prvPdlOsFree(rootPtr);
        return NULL;
    }

    rootPtr->filenamePtr = XML_PARSER_STRDUP_MAC(xmlFilePathPtr);
    if (!rootPtr->filenamePtr)
    {
        prvPdlOsFree(nodePtr->namePtr);
        prvPdlOsFree(rootPtr);
        return NULL;
    }

    rootPtr->tagPefixPtr = prvPdlOsCalloc(1, strlen(tagPefixPtr) + 2);
    if (!rootPtr->tagPefixPtr)
    {
        prvPdlOsFree(nodePtr->namePtr);
        prvPdlOsFree(rootPtr->filenamePtr);
        prvPdlOsFree(rootPtr);
        return NULL;
    }
    memcpy(rootPtr->tagPefixPtr, tagPefixPtr, strlen(tagPefixPtr));
    rootPtr->tagPefixPtr[strlen(tagPefixPtr)] = ':';
    rootPtr->tagPefixPtr[strlen(tagPefixPtr) + 1] = '\0';

    nodePtr->isValid = TRUE;

    return rootPtr;
}

/**
 * @fn  PRV_XML_PARSER_ROOT_STC * prvXmlParserValidateAndRemovePrefix ( IN char * stringPtr, IN char * tagPefixPtr, OUT char * tagPefixPtr )
 *
 * @brief   Validates and removes prefix from xml tag string.
 *
 * @param [in]  stringPtr       XML string containing both prefix and tag.
 * @param [in]  tagPefixPtr     XML prefix to remove from string.
 * @param [out] tagNamePtr      actual XML tag name.
 *
 * @return  XML_PARSER_RET_CODE_WRONG_FORMAT if prefix could not be found.
 */

XML_PARSER_RET_CODE_TYP prvXmlParserValidateAndRemovePrefix (
    IN  char * stringPtr,
    IN  char * tagPefixPtr,
    OUT char * tagNamePtr
)
{
    /* validate prefix in tag */
    if (strstr(stringPtr, tagPefixPtr) != stringPtr)
    {
        return XML_PARSER_RET_CODE_WRONG_FORMAT;
    }
    else
    {
        memmove(tagNamePtr, &stringPtr[strlen(tagPefixPtr)], strlen(stringPtr) - strlen(tagPefixPtr) + 1);
    }

    return XML_PARSER_RET_CODE_OK;
}

/**
 * @fn  XML_PARSER_RET_CODE_TYP prvXmlParserBuildTree(PRV_XML_PARSER_ROOT_STC * rootPtr, char * xmlFileDataPtr)
 *
 * @brief   Builds database from XML data.
 *
 * @param [in]  rootPtr         Pointer to root or the tree.
 * @param [in]  xmlFileDataPtr  XML data.
 *
 * @return  XML_PARSER_RET_CODE_NOT_FOUND if file could not be opened XML_PARSER_RET_CODE_NO_MEM
 *          if out of memory occurred while building database  
 *          XML_PARSER_RET_CODE_WRONG_FORMAT if XML file format is invalid.
 */

XML_PARSER_RET_CODE_TYP prvXmlParserBuildTree(PRV_XML_PARSER_ROOT_STC * rootPtr, char * xmlFileDataPtr)
{
    PRV_XML_PARSER_XML_TAG_DATA_STC openingTagData, closingTagData;
    char                            nameArr[100], valueArr[100], *prefixPtr = rootPtr->tagPefixPtr;
    PRV_XML_PARSER_NODE_STC       * nodePtr = &rootPtr->node, * leafPtr;
    XML_PARSER_RET_CODE_TYP         rc;
    BOOLEAN                         handle_opening_root = TRUE, handle_closing_root = TRUE, handle_first_table = FALSE;
    char                            first_table_prefix[50];

    memset(first_table_prefix, 0, sizeof(first_table_prefix));
    XML_PARSER_STRCAT_MAC(first_table_prefix, sizeof(first_table_prefix), "xmlns:");
    XML_PARSER_STRCAT_MAC(first_table_prefix, sizeof(first_table_prefix) - strlen(first_table_prefix), prefixPtr);
    /* remove ':' char */
    first_table_prefix[strlen(first_table_prefix) - 1] = '\0';

    openingTagData.startOfFilePtr = closingTagData.startOfFilePtr = xmlFileDataPtr;

    /* find first opening tag */
    if (FALSE == prvXmlParserGetNextTag(xmlFileDataPtr, &openingTagData))
        return XML_PARSER_RET_CODE_WRONG_FORMAT;

    while (openingTagData.strValuePtr)
    {
        /* find next closing tag */
        if (!openingTagData.isLeaf || !openingTagData.isClosingChar)
        {
            closingTagData.strValuePtr = openingTagData.endPtr;
            if (FALSE == prvXmlParserGetNextTag(closingTagData.strValuePtr, &closingTagData))
                return XML_PARSER_RET_CODE_WRONG_FORMAT;
        }

        /* if this is an leaf ==>> self-ending tag OR opening tag name == closing tag name */
        if (openingTagData.isLeaf || (openingTagData.strValuePtr && closingTagData.strValuePtr && closingTagData.isClosingChar && prvXmlParserAreTagsEqual(openingTagData.strValuePtr, closingTagData.strValuePtr)))
        {
            if (FALSE == prvXmlParserTagNameGet(&openingTagData, nameArr, sizeof(nameArr) - 1))
                return XML_PARSER_RET_CODE_WRONG_FORMAT;

            /* validate prefix in tag */
            if (prvXmlParserValidateAndRemovePrefix(nameArr, prefixPtr, nameArr) != XML_PARSER_RET_CODE_OK)
            {
                XML_PARSER_DEBUG_LOG(("No prefix in name\n->\t"));
                prvXmlParserErrDebugOut(xmlFileDataPtr, openingTagData.strValuePtr);
                return XML_PARSER_RET_CODE_WRONG_FORMAT;
            }

            if (!openingTagData.isLeaf)
            {
                if (FALSE == prvXmlParserTagValueGet(&openingTagData, valueArr, sizeof(valueArr) - 1))
                    return XML_PARSER_RET_CODE_WRONG_FORMAT;
            }
            else
            {
                valueArr[0] = '\0';
            }

            leafPtr = prvXmlParserNodeAdd(nodePtr, nameArr, valueArr);
            if (!leafPtr)
                return XML_PARSER_RET_CODE_NO_MEM;

            rc = prvXmlParserAttrNameGet(&openingTagData, nameArr, sizeof(nameArr) - 1);
            if (XML_PARSER_RET_CODE_OK == rc)
            {
                /* validate prefix in tag */
                if (prvXmlParserValidateAndRemovePrefix(nameArr, prefixPtr, nameArr) != XML_PARSER_RET_CODE_OK)
                {
                    XML_PARSER_DEBUG_LOG(("No prefix in attribute\n->\t"));
                    prvXmlParserErrDebugOut(xmlFileDataPtr, openingTagData.strValuePtr);
                    return XML_PARSER_RET_CODE_WRONG_FORMAT;
                }

                if (prvXmlParserAttrValueGet(&openingTagData, nameArr, sizeof(nameArr) - 1) != XML_PARSER_RET_CODE_OK)
                {
                    XML_PARSER_DEBUG_LOG(("No value to existing attribute\n->\t"));
                    prvXmlParserErrDebugOut(xmlFileDataPtr, openingTagData.strValuePtr);
                    return XML_PARSER_RET_CODE_WRONG_FORMAT;
                }

                if (FALSE == prvXmlParserAuxDataAdd(leafPtr, nameArr))
                    return XML_PARSER_RET_CODE_NO_MEM;
            }
            else if (XML_PARSER_RET_CODE_ERROR == rc)
            {
                prvXmlParserErrDebugOut(xmlFileDataPtr, openingTagData.strValuePtr);
                return XML_PARSER_RET_CODE_WRONG_FORMAT;
            }

            if (!openingTagData.isLeaf)
            {
                closingTagData.strValuePtr = closingTagData.endPtr;
                if (FALSE == prvXmlParserGetNextTag(closingTagData.strValuePtr, &openingTagData))
                    return XML_PARSER_RET_CODE_WRONG_FORMAT;
            }
            else
            {
                /* if self-ending leaf - next tag is different from current => restart tagging */
                openingTagData.strValuePtr = openingTagData.endPtr;
                if (FALSE == prvXmlParserGetNextTag(openingTagData.strValuePtr, &openingTagData))
                    return XML_PARSER_RET_CODE_WRONG_FORMAT;
            }
        }
        else
        {
            if (!openingTagData.isClosingChar)
            {
                if (FALSE == prvXmlParserTagNameGet(&openingTagData, nameArr, sizeof(nameArr) - 1))
                    return XML_PARSER_RET_CODE_WRONG_FORMAT;

                /* handle root */
                if (strcmp(nameArr, "data") == 0 && handle_opening_root == TRUE)
                {
                    handle_opening_root = FALSE;
                    nodePtr = prvXmlParserNodeAdd(nodePtr, nameArr, NULL);
                    if (!nodePtr)
                        return XML_PARSER_RET_CODE_NO_MEM;
                    handle_first_table = TRUE;
                }
                else
                {
                    /* validate prefix in tag */
                    if (prvXmlParserValidateAndRemovePrefix(nameArr, prefixPtr, nameArr) != XML_PARSER_RET_CODE_OK)
                    {
                        XML_PARSER_DEBUG_LOG(("No prefix in name\n->\t"));
                        prvXmlParserErrDebugOut(xmlFileDataPtr, openingTagData.strValuePtr);
                        return XML_PARSER_RET_CODE_WRONG_FORMAT;
                    }

                    nodePtr = prvXmlParserNodeAdd(nodePtr, nameArr, NULL);
                    if (!nodePtr)
                        return XML_PARSER_RET_CODE_NO_MEM;

                    /* for non-leaf nodes, there is still possible to have attributes (i.e. typedefs) */
                    rc = prvXmlParserAttrNameGet(&openingTagData, nameArr, sizeof(nameArr) - 1);
                    if (XML_PARSER_RET_CODE_OK == rc)
                    {
                        if (handle_first_table == FALSE)
                        {
                            /* validate prefix in tag */
                            if (prvXmlParserValidateAndRemovePrefix(nameArr, prefixPtr, nameArr) != XML_PARSER_RET_CODE_OK)
                            {
                                XML_PARSER_DEBUG_LOG(("No prefix in attribute\n->\t"));
                                prvXmlParserErrDebugOut(xmlFileDataPtr, openingTagData.strValuePtr);
                                return XML_PARSER_RET_CODE_WRONG_FORMAT;
                            }
                        }
                        else
                        {
                            /* validate prefix in tag */
                            if (prvXmlParserValidateAndRemovePrefix(nameArr, first_table_prefix, nameArr) != XML_PARSER_RET_CODE_OK)
                            {
                                XML_PARSER_DEBUG_LOG(("No prefix in attribute\n->\t"));
                                prvXmlParserErrDebugOut(xmlFileDataPtr, openingTagData.strValuePtr);
                                return XML_PARSER_RET_CODE_WRONG_FORMAT;
                            }

                            handle_first_table = FALSE;
                        }

                        if (prvXmlParserAttrValueGet(&openingTagData, nameArr, sizeof(nameArr) - 1) != XML_PARSER_RET_CODE_OK)
                        {
                            XML_PARSER_DEBUG_LOG(("No value to existing attribute\n->\t"));
                            prvXmlParserErrDebugOut(xmlFileDataPtr, openingTagData.strValuePtr);
                            return XML_PARSER_RET_CODE_WRONG_FORMAT;
                        }

                        if (FALSE == prvXmlParserAuxDataAdd(nodePtr, nameArr))
                            return XML_PARSER_RET_CODE_NO_MEM;
                    }
                    else if (XML_PARSER_RET_CODE_ERROR == rc)
                    {
                        return XML_PARSER_RET_CODE_WRONG_FORMAT;
                    }
                }
                openingTagData = closingTagData;
            }
            else
            {
                openingTagData.strValuePtr = closingTagData.endPtr;
                if (FALSE == prvXmlParserGetNextTag(openingTagData.strValuePtr, &openingTagData))
                    return XML_PARSER_RET_CODE_WRONG_FORMAT;
            }
        }

        /* found closing tag => go up the tree and validate xml structure if possible */
        while (openingTagData.isClosingChar)
        {
            if (!openingTagData.isLeaf)
            {
                if (FALSE == prvXmlParserTagNameGet(&openingTagData, nameArr, sizeof(nameArr) - 1))
                    return XML_PARSER_RET_CODE_WRONG_FORMAT;

                /* validate prefix in tag */
                if (prvXmlParserValidateAndRemovePrefix(nameArr, prefixPtr, nameArr) != XML_PARSER_RET_CODE_OK)
                {
                    if (handle_closing_root == TRUE && strcmp(nameArr, "data") == 0)
                    {
                        handle_closing_root = FALSE;
                    }
                    else
                    {
                        XML_PARSER_DEBUG_LOG(("No prefix in name\n->\t"));
                        prvXmlParserErrDebugOut(xmlFileDataPtr, openingTagData.strValuePtr);
                        return XML_PARSER_RET_CODE_WRONG_FORMAT;
                    }
                }

                if (strcmp(nameArr, nodePtr->namePtr) == 0)
                {
                    nodePtr->isValid = TRUE;
                    nodePtr = nodePtr->parentPtr;
                }
                else
                {
                    prvXmlParserErrDebugOut(xmlFileDataPtr, openingTagData.strValuePtr);
                    return XML_PARSER_RET_CODE_WRONG_FORMAT;
                }
            }
            openingTagData.strValuePtr = openingTagData.endPtr;
            if (FALSE == prvXmlParserGetNextTag(openingTagData.strValuePtr, &openingTagData))
                return XML_PARSER_RET_CODE_WRONG_FORMAT;
        }

        if (nodePtr == NULL)
        {
            /* reached ROOT - xml missing opening tag */
            return XML_PARSER_RET_CODE_WRONG_FORMAT;
        }
    }

    return XML_PARSER_RET_CODE_OK;
}

/**
 * @fn  PRV_XML_PARSER_NODE_STC * prvXmlParseFindTagByName(PRV_XML_PARSER_NODE_STC * context, char * name, BOOLEAN isCaseSensitive)
 *
 * @brief   Find next xml node by name.
 *
 * @param [in,out]  context         If non-null, the context.
 * @param [in]      name            XML tag to search for.
 * @param           isCaseSensitive True if is case sensitive, false if not.
 *
 * @return  Pointer to xml node or NULL if not found.
 *
 * ### param [in]       contextPtr  Search under this node (in tree)
 */

PRV_XML_PARSER_NODE_STC * prvXmlParseFindTagByName(PRV_XML_PARSER_NODE_STC * context, char * name, BOOLEAN isCaseSensitive)
{
    PRV_XML_PARSER_SEARCH_INFO_STC  miscInfo;

    miscInfo.isCaseSensitive = isCaseSensitive;
    miscInfo.strPtr = name;
    return prvXmlParserTraverseTree(context, prvXmlParserIsTagName,  PRV_XML_PARSER_CALL_ON_ENTER_E, (void *)&miscInfo);
}

/**
 * @fn  XML_PARSER_RET_CODE_TYP prvXmlParserGetTagCount(PRV_XML_PARSER_NODE_STC * contextPtr, char * xmlTagPtr, UINT_32 * countPtr)
 *
 * @brief   Count xml node by name.
 *
 * @param [in]  contextPtr  Search under this node (in tree)
 * @param [in]  xmlTagPtr   XML tag to search for.
 * @param [out] countPtr    - Number of encountered tag's.
 *
 * @return  An XML_PARSER_RET_CODE_TYP.
 */

XML_PARSER_RET_CODE_TYP prvXmlParserGetTagCount(PRV_XML_PARSER_NODE_STC * contextPtr,  char  * xmlTagPtr, UINT_32  * countPtr)
{
    XML_GET_TAG_COUNT_STC   cookie;

    cookie.namePtr = xmlTagPtr;
    cookie.count   = 0;

    prvXmlParserTraverseTree(contextPtr, prvXmlParserNodeCount,  PRV_XML_PARSER_CALL_ON_ENTER_E, (void *)&cookie);
    *countPtr = cookie.count;

    return XML_PARSER_RET_CODE_OK;
}

/**
 * @fn  BOOLEAN prvXmlParserEnumDbBuild ( IN PRV_XML_PARSER_ROOT_STC * rootPtr )
 *
 * @brief   Build enum database.
 *
 * @param [in]  rootPtr         XML tree instance.
 *
 * @return  XML_PARSER_RET_CODE_NOT_INITIALIZED if database wasn't initialized
 * @return  XML_PARSER_RET_CODE_NOT_FOUND       if encountered unknown.
 *
 */
static XML_PARSER_RET_CODE_TYP prvXmlParserEnumDbBuild (
    IN PRV_XML_PARSER_ROOT_STC * rootPtr
)
{
    PRV_XML_PARSER_TYPEDEF_NODE_STC    *typedefNodePtr, *actNodePtr;
    XML_PARSER_ENUM_ID_ENT              enumId;

    if (rootPtr == NULL || rootPtr->typedefsPtr == NULL)
    {
        prvPdlOsPrintf("XML not initialized\n");
        return XML_PARSER_RET_CODE_NOT_INITIALIZED;
    }

    typedefNodePtr = rootPtr->typedefsPtr;
    while (typedefNodePtr)
    {
        actNodePtr = typedefNodePtr;
        if (actNodePtr->resolvedPtr)
            actNodePtr = actNodePtr->resolvedPtr;
        if (actNodePtr->type == PRV_XML_PARSER_SIMPLE_DATA_TYPE_ENUM_E)
        {
            /* find id by name */
            for (enumId = XML_PARSER_ENUM_ID_I2C_ACCESS_TYPE_E; enumId < XML_PARSER_ENUM_ID_LAST_E; enumId ++)
            {
                if (strcmp(prvXmlParserEnumId2NameMappingtArr[enumId], actNodePtr->namePtr) == 0)
                    break;
            }

            if (enumId == XML_PARSER_ENUM_ID_LAST_E)
            {
                prvPdlOsPrintf("Unregistered enum %s\n", actNodePtr->namePtr);
                return XML_PARSER_RET_CODE_NOT_FOUND;
            }

            prvXmlParserEnumsDbArr[enumId] = (PRV_XML_PARSER_TYPEDEF_DATA_UNT *)&actNodePtr->data.enum_value;
        }
        typedefNodePtr = typedefNodePtr->nextPtr;
    }

    return XML_PARSER_RET_CODE_OK;
}

/**
 * @fn  XML_PARSER_RET_CODE_TYP prvXmlParserResolveTypes(PRV_XML_PARSER_ROOT_STC * rootPtr, BOOLEAN isEzbXml, PRV_XML_PARSER_NODE_STC * typedefsPtr)
 *
 * @brief   Traverse tree, resolve nodes' types and validate node's data.
 *
 * @param [in]  rootPtr     Pointer to tree's root.
 * @param [in]  izEzbXML    Used to indicate whether to build string&enum conversion tables according to EZB DB or external DBs
 * @param [in]  typedefsPtr Pointer to tree's typedefs section.
 *
 * @return  XML_PARSER_RET_CODE_NO_MEM if memory allocation failed.
 * @return  XML_PARSER_RET_CODE_WRONG_FORMAT if invalid node was found.
 */

XML_PARSER_RET_CODE_TYP prvXmlParserResolveTypes(PRV_XML_PARSER_ROOT_STC * rootPtr, BOOLEAN isEzbXml, PRV_XML_PARSER_NODE_STC * typedefsPtr)
{
    XML_PARSER_RET_CODE_TYP ret_code = XML_PARSER_RET_CODE_OK;

    rootPtr->typedefsPtr = prvXmlParserTypedefTreeBuild(typedefsPtr);
    if (rootPtr->typedefsPtr == NULL)
        return XML_PARSER_RET_CODE_NO_MEM;

    while (typedefsPtr->nextSiblingPtr)
    {
        if (prvXmlParserTraverseTree(typedefsPtr->nextSiblingPtr, prvXmlParserResolveType, PRV_XML_PARSER_CALL_ON_ENTER_E, (void *)rootPtr->typedefsPtr))
            return XML_PARSER_RET_CODE_WRONG_FORMAT;

        typedefsPtr = typedefsPtr->nextSiblingPtr;
    }

    if (isEzbXml) {
        ret_code = prvXmlParserEnumDbBuild(rootPtr);
        return ret_code;
    }

    return XML_PARSER_RET_CODE_OK;
}

/**
 * @fn  XML_PARSER_RET_CODE_TYP prvXmlParserValidateData(PRV_XML_PARSER_ROOT_STC * rootPtr)
 *
 * @brief   Validate tree data.
 *
 * @param [in,out]  rootPtr If non-null, the root pointer.
 *
 * @return  XML_PARSER_RET_CODE_WRONG_FORMAT if invalid node was found.
 *
 * ### param [in]       root    Pointer to tree's root.
 */

XML_PARSER_RET_CODE_TYP prvXmlParserValidateData(PRV_XML_PARSER_ROOT_STC * rootPtr)
{
    if (prvXmlParserTraverseTree(&rootPtr->node, prvXmlParserIsNodeValid,  PRV_XML_PARSER_CALL_ON_ENTER_E, NULL))
        return XML_PARSER_RET_CODE_WRONG_FORMAT;

    return XML_PARSER_RET_CODE_OK;
}

/**
 * @fn  void prvXmlParserDestroyNode(PRV_XML_PARSER_ROOT_STC ** rootPtr)
 *
 * @brief   Free tree memory.
 *
 * @param [in]  rootPtr Pointer to tree's root.
 */

void prvXmlParserDestroyNode(PRV_XML_PARSER_ROOT_STC ** rootPtr)
{
    if (*rootPtr == NULL)
        return;

    prvXmlParserTraverseTree(&(*rootPtr)->node, prvXmlParserNodeDestroy,  PRV_XML_PARSER_CALL_ON_EXIT_E, NULL);
    prvXmlParserTypedefTreeDestroy((*rootPtr)->typedefsPtr);
    if ((*rootPtr)->filenamePtr)
        prvPdlOsFree((*rootPtr)->filenamePtr);
    if ((*rootPtr)->tagPefixPtr)
        prvPdlOsFree((*rootPtr)->tagPefixPtr);
    if ((*rootPtr)->node.namePtr)
        prvPdlOsFree((*rootPtr)->node.namePtr);
    prvPdlOsFree(*rootPtr);
}

/**
 * @fn  char * prvXmlParserStrdup(char * origStringPtr)
 *
 * @brief   Duplicate null-terminated string. This function allocates memory for its operation
 *          that needs to be freed.
 *
 * @param [in]  origStringPtr   string to duplicate.
 *
 * @return  pointer to new string or NULL if no memory or origStringPtr is NULL.
 */

char * prvXmlParserStrdup(char * origStringPtr)
{
    char      * newStringPtr;
    UINT_32     len;

    if (origStringPtr == NULL)
        return NULL;

    len = strlen(origStringPtr) + 1;
    newStringPtr = (char *)prvPdlOsMalloc(len);
    if (newStringPtr == NULL)
        return NULL;

    memcpy(newStringPtr, origStringPtr, len);

    return newStringPtr;
}

/**
 * @fn  BOOLEAN prvXmlParserDebugGetNextOpenedRoot ( INT_32 * indexPtr )
 *
 * @brief   Update value of the current node
 *
 * @param [in,out]  indexPtr    If non-null, the index pointer.
 *
 * @return  XML_PARSER_RET_CODE_WRONG_PARAM if invalid node supplied.
 * @return  XML_PARSER_RET_CODE_WRONG_FORMAT if inputPtr does not pass data format validation.
 * @return  XML_PARSER_RET_CODE_NO_MEM if memory allocation failed.
 *
 * ### param [in]       xmlId       XML node descriptor.
 * ### param [in]       inputSize   size of inputPtr.
 * ### param [in]       inputPtr    Value of the node according to typedef in XML file.
 */

BOOLEAN prvXmlParserDebugGetNextOpenedRoot (
    INT_8  * indexPtr
)
{
    UINT_8         i;

    if (indexPtr == NULL)
        return FALSE;

    if (*indexPtr < 0)
        i = 0;
    else
        i = (UINT_8)*indexPtr + 1;

    for (; i < PRV_XML_PARSER_MAX_NUM_OF_OPENED_TREES_CNS; i ++)
    {
        if (prvXmlParserDebugOpenedDescriptors[i] != 0)
        {
            prvPdlOsPrintf("%02d) Used file: %s\n", i, ((PRV_XML_PARSER_ROOT_STC *)prvXmlParserDebugOpenedDescriptors[i])->filenamePtr);
            *indexPtr = (INT_32)i;
            return TRUE;
        }
    }

    return FALSE;
}

/**
 * @fn  static BOOLEAN prvXmlParserPrintNode ( IN PRV_XML_PARSER_NODE_STC ** nodePtr, IN void * cookiePtr )
 *
 * @brief   Print node name and parameters.
 *
 * @param [in]  nodePtr     Pointer to xml node.
 * @param [in]  cookiePtr   String to compare to.
 *
 * @return  TRUE if node's name equals to searched string.
 */

static BOOLEAN prvXmlParserPrintNode (
    IN PRV_XML_PARSER_NODE_STC      ** nodePtr,
    IN void                          * cookiePtr
)
{
    PRV_XML_PARSER_NODE_STC           * tmpPtr = *nodePtr;
    PRV_XML_PARSER_SEARCH_INFO_STC    * debugInfoPtr = (PRV_XML_PARSER_SEARCH_INFO_STC *)cookiePtr;

    while (tmpPtr && tmpPtr != debugInfoPtr->contextPtr)
    {
        tmpPtr = tmpPtr->parentPtr;
        prvPdlOsPrintf("\t");
    }

    if (debugInfoPtr->printWithValues)
        prvPdlOsPrintf("[\'%s\'] [Value %s] [Attr %s] [Type %d] [isValid %d]\n",
            (*nodePtr)->namePtr, (*nodePtr)->valuePtr, (*nodePtr)->auxData.attrNamePtr, (*nodePtr)->type, (*nodePtr)->isValid);
    else
        prvPdlOsPrintf("[\'%s\']\n", (*nodePtr)->namePtr);

    return FALSE;
}

/**
 * @fn  BOOLEAN prvXmlParserDebugShowTreeByRoot ( UINT_32 rootIndex, char *tagPtr, BOOLEAN printWithValues )
 *
 * @brief   Update value of the current node
 *
 * @param           rootIndex       Zero-based index of the root.
 * @param [in,out]  tagPtr          If non-null, the tag pointer.
 * @param           printWithValues True to print with values.
 *
 * @return  XML_PARSER_RET_CODE_WRONG_PARAM if invalid node supplied.
 * @return  XML_PARSER_RET_CODE_WRONG_FORMAT if inputPtr does not pass data format validation.
 * @return  XML_PARSER_RET_CODE_NO_MEM if memory allocation failed.
 *
 * ### param [in]       xmlId       XML node descriptor.
 * ### param [in]       inputSize   size of inputPtr.
 * ### param [in]       inputPtr    Value of the node according to typedef in XML file.
 */

BOOLEAN prvXmlParserDebugShowTreeByRoot (
    UINT_32         rootIndex,
    char           *tagPtr,
    BOOLEAN         printWithValues
)
{
    XML_PARSER_NODE_DESCRIPTOR_TYP      context;
    PRV_XML_PARSER_SEARCH_INFO_STC      debugInfo;

    if (rootIndex >= PRV_XML_PARSER_MAX_NUM_OF_OPENED_TREES_CNS)
    {
        prvPdlOsPrintf("Out of range index\n");
    }
    else if (prvXmlParserDebugOpenedDescriptors[rootIndex] == 0)
    {
        prvPdlOsPrintf("Unused index\n");
    }
    else
    {
        context = XML_PARSER_ROOT2NODE_CONVERT_MAC(prvXmlParserDebugOpenedDescriptors[rootIndex]);

        if (tagPtr)
            context = prvXmlParseFindTagByName((PRV_XML_PARSER_NODE_STC *)context, tagPtr, FALSE);

        if (context)
        {
            debugInfo.contextPtr      = (PRV_XML_PARSER_NODE_STC *)context;
            debugInfo.printWithValues = printWithValues;
            prvXmlParserTraverseTree((PRV_XML_PARSER_NODE_STC *)context, prvXmlParserPrintNode,  PRV_XML_PARSER_CALL_ON_ENTER_E, (void *)&debugInfo);
        }
    }

    return TRUE;
}

/**
 * @fn  XML_PARSER_RET_CODE_TYP prvXmlParserConvertEnumValueToString ( IN XML_PARSER_ENUM_ID_ENT enumId, IN UINT_32 intValue, OUT char ** strValuePtr )
 *
 * @brief   Convert enum int to string value.
 *
 * @param [in]  enumId          Enum identification.
 * @param [in]  intValue        enum int value to convert.
 * @param [out] strValuePtr     pointer to string value to return if != NULL.
 *
 * @return  XML_PARSER_RET_CODE_NOT_INITIALIZED if database wasn't initialized
 * @return  XML_PARSER_RET_CODE_NOT_FOUND       if no match was found.
 *
 */
XML_PARSER_RET_CODE_TYP prvXmlParserConvertEnumValueToString (
    IN XML_PARSER_ENUM_ID_ENT   enumId,
    IN UINT_32                  intValue,
    OUT char                 ** strValuePtr
)
{
    UINT_32                             index;
    PRV_XML_PARSER_TYPEDEF_DATA_UNT    *enumPtr;

    enumPtr = prvXmlParserEnumsDbArr[enumId];

    if (enumPtr == NULL)
        return XML_PARSER_RET_CODE_NOT_INITIALIZED;

    for (index = 0; index < enumPtr->enum_value.arr_size; index ++)
    {
        if (enumPtr->enum_value.intEnumArr[index] == intValue)
        {
            if (strValuePtr)
                *strValuePtr = enumPtr->enum_value.strEnumArr[index];
            return XML_PARSER_RET_CODE_OK;
        }
    }

    return XML_PARSER_RET_CODE_NOT_FOUND;
}

/**
 * @fn  XML_PARSER_RET_CODE_TYP prvXmlParserConvertEnumStringToValue ( IN XML_PARSER_ENUM_ID_ENT enumId, IN char * strValuePtr, OUT UINT_32 * intValuePtr )
 *
 * @brief   Convert enum string to int value.
 *
 * @param [in]  enumId          Enum identification.
 * @param [in]  intValuePtr     pointer to string value to convert.
 * @param [out] strValuePtr     enum int value to return if != NULL.
 *
 * @return  XML_PARSER_RET_CODE_NOT_INITIALIZED if database wasn't initialized.
 * @return  XML_PARSER_RET_CODE_NOT_FOUND       if no match was found.
 *
 */
XML_PARSER_RET_CODE_TYP prvXmlParserConvertEnumStringToValue (
    IN XML_PARSER_ENUM_ID_ENT   enumId,
    IN char                   * strValuePtr,
    OUT UINT_32               * intValuePtr
)
{
    UINT_32                             index;
    PRV_XML_PARSER_TYPEDEF_DATA_UNT    *enumPtr;

    enumPtr = prvXmlParserEnumsDbArr[enumId];

    if (enumPtr == NULL)
        return XML_PARSER_RET_CODE_NOT_INITIALIZED;

    for (index = 0; index < enumPtr->enum_value.arr_size; index ++)
    {
        if (strcmp(enumPtr->enum_value.strEnumArr[index], strValuePtr) == 0)
        {
            if (intValuePtr)
                *intValuePtr = enumPtr->enum_value.intEnumArr[index];
            return XML_PARSER_RET_CODE_OK;
        }
    }

    return XML_PARSER_RET_CODE_NOT_FOUND;
}


/**
 * @fn  XML_PARSER_RET_CODE_TYP prvXmlParserConvertExternalEnumStringToValue ( IN XML_PARSER_ENUM_ID_ENT enumId, IN char * strValuePtr, OUT UINT_32 * intValuePtr )
 *
 * @brief   Convert external enum string to int value.
 *
 * @param [in]  externalEnumDbArr           external db with enum information (built using xmlParserExternalEnumDbBuild)
 * @param [in]  externalEnumDbSize          external db size
 * @param [in]  enumId                      Enum identification.
 * @param [in]  strValuePtr                 pointer to string value to convert.
 * @param [out] intValuePtr                 enum int value to return if != NULL.
 *
 * @return  XML_PARSER_RET_CODE_NOT_INITIALIZED if database wasn't initialized.
 * @return  XML_PARSER_RET_CODE_NOT_FOUND       if no match was found.
 *
 */
XML_PARSER_RET_CODE_TYP prvXmlParserConvertExternalEnumStringToValue (
    IN XML_PARSER_ENUM_DB_STC          ** externalEnumDbArr,
    IN UINT_32                            externalEnumDbSize,
    IN XML_PARSER_ENUM_ID_ENT             enumId,
    IN char                             * strValuePtr,
    OUT UINT_32                         * intValuePtr
)
{
    UINT_32                             index;
    XML_PARSER_ENUM_DB_STC            *enumPtr;

    PDL_UNUSED_PARAM(externalEnumDbSize);

    enumPtr = externalEnumDbArr[enumId];

    if (enumPtr == NULL)
        return XML_PARSER_RET_CODE_NOT_INITIALIZED;

    for (index = 0; index < enumPtr->arr_size; index ++)
    {
        if (strcmp(enumPtr->strEnumArr[index], strValuePtr) == 0)
        {
            if (intValuePtr)
                *intValuePtr = enumPtr->intEnumArr[index];
            return XML_PARSER_RET_CODE_OK;
        }
    }

    return XML_PARSER_RET_CODE_NOT_FOUND;
}
