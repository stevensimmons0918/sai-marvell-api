/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\src\xml\pdlxmlstub.c.
 *
 * @brief   Pdlxmlstub class
 */

#include <pdl/xml/private/prvXmlParser.h>

/**
 * @fn  XML_PARSER_RET_CODE_TYP xmlParserFindByName ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, IN char * xmlTagPtr, OUT XML_PARSER_NODE_DESCRIPTOR_TYP * xmlChildIdPtr )
 *
 * @brief   XML parser find by name
 *
 * @param           xmlId           Identifier for the XML.
 * @param [in,out]  xmlTagPtr       If non-null, the XML tag pointer.
 * @param [in,out]  xmlChildIdPtr   If non-null, the XML child identifier pointer.
 *
 * @return  An XML_PARSER_RET_CODE_TYP.
 */

XML_PARSER_RET_CODE_TYP xmlParserFindByName (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    IN  char                              * xmlTagPtr,
    OUT XML_PARSER_NODE_DESCRIPTOR_TYP    * xmlChildIdPtr
)
{
    return PDL_OK;
}

/**
 * @fn  XML_PARSER_RET_CODE_TYP xmlParserGetFirstChild ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlParent, OUT XML_PARSER_NODE_DESCRIPTOR_TYP * xmlChildPtr )
 *
 * @brief   XML parser get first child
 *
 * @param           xmlParent   The XML parent.
 * @param [in,out]  xmlChildPtr If non-null, the XML child pointer.
 *
 * @return  An XML_PARSER_RET_CODE_TYP.
 */

XML_PARSER_RET_CODE_TYP xmlParserGetFirstChild (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlParent,
    OUT XML_PARSER_NODE_DESCRIPTOR_TYP    * xmlChildPtr
)
{
    return XML_PARSER_RET_CODE_NOT_FOUND;
}

/**
 * @fn  XML_PARSER_RET_CODE_TYP xmlParserGetNextSibling ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT XML_PARSER_NODE_DESCRIPTOR_TYP * xmlNextIdPtr )
 *
 * @brief   XML parser get next sibling
 *
 * @param           xmlId           Identifier for the XML.
 * @param [in,out]  xmlNextIdPtr    If non-null, the XML next identifier pointer.
 *
 * @return  An XML_PARSER_RET_CODE_TYP.
 */

XML_PARSER_RET_CODE_TYP xmlParserGetNextSibling (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    OUT XML_PARSER_NODE_DESCRIPTOR_TYP    * xmlNextIdPtr

)
{
    return PDL_OK;
}

/**
 * @fn  XML_PARSER_RET_CODE_TYP xmlParserGetName ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, INOUT UINT_32 * outputSizePtr, OUT char * outputPtr )
 *
 * @brief   XML parser get name
 *
 * @param           xmlId           Identifier for the XML.
 * @param [in,out]  outputSizePtr   If non-null, the output size pointer.
 * @param [in,out]  outputPtr       If non-null, the output pointer.
 *
 * @return  An XML_PARSER_RET_CODE_TYP.
 */

XML_PARSER_RET_CODE_TYP xmlParserGetName (
    IN      XML_PARSER_NODE_DESCRIPTOR_TYP  xmlId,
    INOUT   UINT_32                       * outputSizePtr,
    OUT     char                          * outputPtr
)
{
    return PDL_OK;
}

/**
 * @fn  XML_PARSER_RET_CODE_TYP xmlParserGetValue ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, INOUT UINT_32 * outputSizePtr, OUT void * outputPtr )
 *
 * @brief   XML parser get value
 *
 * @param           xmlId           Identifier for the XML.
 * @param [in,out]  outputSizePtr   If non-null, the output size pointer.
 * @param [in,out]  outputPtr       If non-null, the output pointer.
 *
 * @return  An XML_PARSER_RET_CODE_TYP.
 */

XML_PARSER_RET_CODE_TYP xmlParserGetValue (
    IN      XML_PARSER_NODE_DESCRIPTOR_TYP  xmlId,
    INOUT   UINT_32                       * outputSizePtr,
    OUT     void                          * outputPtr
)
{
    return PDL_OK;
}

/**
 * @fn  XML_PARSER_RET_CODE_TYP xmlParserSetValue ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, IN UINT_32 inputSize, IN void * inputPtr )
 *
 * @brief   XML parser set value
 *
 * @param           xmlId       Identifier for the XML.
 * @param           inputSize   Size of the input.
 * @param [in,out]  inputPtr    If non-null, the input pointer.
 *
 * @return  An XML_PARSER_RET_CODE_TYP.
 */

XML_PARSER_RET_CODE_TYP xmlParserSetValue (
    IN   XML_PARSER_NODE_DESCRIPTOR_TYP xmlId,
    IN   UINT_32                        inputSize,
    IN   void                         * inputPtr
)
{
    return PDL_OK;
}

/**
 * @fn  XML_PARSER_RET_CODE_TYP xmlParserGetTagCount ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, IN char * xmlTagPtr, OUT UINT_32 * countPtr )
 *
 * @brief   XML parser get tag count
 *
 * @param           xmlId       Identifier for the XML.
 * @param [in,out]  xmlTagPtr   If non-null, the XML tag pointer.
 * @param [in,out]  countPtr    If non-null, the count pointer.
 *
 * @return  An XML_PARSER_RET_CODE_TYP.
 */

XML_PARSER_RET_CODE_TYP xmlParserGetTagCount (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP  xmlId,
    IN  char                          * xmlTagPtr,
    OUT UINT_32                       * countPtr

)
{
    return PDL_OK;
}

/**
 * @fn  XML_PARSER_RET_CODE_TYP xmlParserBuild ( IN char * xmlFilePathPtr, OUT XML_PARSER_ROOT_DESCRIPTOR_TYP * rootIdPtr )
 *
 * @brief   XML parser build
 *
 * @param [in,out]  xmlFilePathPtr  If non-null, the XML file path pointer.
 * @param [in,out]  rootIdPtr       If non-null, the root identifier pointer.
 *
 * @return  An XML_PARSER_RET_CODE_TYP.
 */

XML_PARSER_RET_CODE_TYP xmlParserBuild (
    IN  char                              * xmlFilePathPtr,
    OUT XML_PARSER_ROOT_DESCRIPTOR_TYP    * rootIdPtr
)
{
    return PDL_OK;
}