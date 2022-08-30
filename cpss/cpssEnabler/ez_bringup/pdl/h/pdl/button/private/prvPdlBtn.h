/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\button\private\prvpdlbtn.h.
 *
 * @brief   Declares the prvpdlbtn class
 */

#ifndef __prvPdlBtnh

#define __prvPdlBtnh
/**
********************************************************************************
 * @file prvPdlBtn.h   
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
 * @brief Platform driver layer - Button private declarations and APIs
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/button/pdlBtn.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <pdl/lib/private/prvPdlLib.h>


/**
* @addtogroup Button
* @{
*/

 /** 
 * @defgroup Button_private Button Private
 * @{Button private definitions and declarations including: 
*/


 /** 
 * @def PDL_BTN_TAG_NAME
 * @brief Button TAG name (in XML)
*/
#define PDL_BTN_TAG_NAME    "Buttons"
/** 
 * @def PDL_BTN_VALUE_NAME
 * @brief push-value TAG name (in XML)
*/#define PDL_BTN_VALUE_NAME  "push-value"

extern PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC pdlBtnNameStrToEnum;

/**
 * @struct  PDL_BTN_PRV_KEY_STC
 *
 * @brief   defines structure for button key in db
 */

typedef struct {
    /** @brief   Identifier for the button */
    PDL_BTN_ID_ENT                        btnId;
} PDL_BTN_PRV_KEY_STC;

/**
 * @struct  PDL_BTN_PRV_DB_STC
 *
 * @brief   defines structure stored for button in db
 */

typedef struct {
    /** @brief   The button interface */
    PDL_BTN_INTERFACE_STC                   btnInterface;
    /** @brief   The button capabilities */
    PDL_BTN_XML_CAPABILITIES_STC            btnCapabilities;
} PDL_BTN_PRV_DB_STC;

/**
 * @struct  PDL_BTN_ENUM_STR_STC
 *
 * @brief   used for converting between button enum id and string found inside XML file
 */

typedef struct {
    /** @brief   The name */
    char                    * name;
    /** @brief   Identifier for the button */
    PDL_BTN_ID_ENT            buttonId;
} PDL_BTN_ENUM_STR_STC;

/**
 * @fn  PDL_STATUS pdlBtnInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId );
 *
 * @brief   Init button module Create button DB and initialize
 *
 * @param [in]  xmlId   - Xml id.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlBtnInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
);

/**
 * @fn  PDL_STATUS prvPdlBtnDestroy ()
 *
 * @brief   release all memory allocated by module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlBtnDestroy (
    void
);

/* @}*/
/* @}*/

#endif
