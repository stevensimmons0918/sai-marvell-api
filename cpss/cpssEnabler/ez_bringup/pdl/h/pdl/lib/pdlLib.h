/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\button\pdllib.h.
 *
 * @brief   Declares the pdllib class
 */

#ifndef __pdlLibh
#define __pdlLibh
/**
********************************************************************************
 * @file pdlLib.h   
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
 * @brief Platform driver layer - LIB related API
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/xml/xmlParser.h>

 /** 
 * @defgroup LIB
 * @{LIB definitions and declarations
 * 
*/

#define PDL_IS_SPEED_GREATER(__a, __b)  (prvPdlSpeedEnumToValue[__a] > prvPdlSpeedEnumToValue[__b])

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
);

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
extern PDL_STATUS pdlLibConvertEnumValueToString (
    IN XML_PARSER_ENUM_ID_ENT   enumId,
    IN UINT_32                  intValue,
    OUT char                 ** strValuePtr
);

/**
 * @fn  PDL_STATUS pdlLibConvertEnumStringToValue ( IN XML_PARSER_ENUM_ID_ENT enumId, IN char * strValuePtr, OUT UINT_32 * intValuePtr )
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
extern PDL_STATUS pdlLibConvertEnumStringToValue (
    IN XML_PARSER_ENUM_ID_ENT   enumId,
    IN char                   * strValuePtr,
    OUT UINT_32               * intValuePtr
);

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
);

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
);

/*$ END OF pdlLibBoardDescriptionDbGet */

#endif
