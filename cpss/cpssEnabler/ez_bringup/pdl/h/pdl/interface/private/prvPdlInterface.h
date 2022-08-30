/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\interface\private\prvpdlinterface.h.
 *
 * @brief   Declares the prvpdlinterface class
 */

#ifndef __prvPdlInterfaceh

#define __prvPdlInterfaceh
/**
********************************************************************************
 * @file prvPdlInterface.h  
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
 * @brief Platform driver layer - Private Interface related API (internal)
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/lib/private/prvPdlLib.h>

/**
* @addtogroup Interface
* @{
*/

/** 
 * @defgroup Interface_private Interface Private
 * @{defines private structures used to store interface information from XML inside db 
*/
#define PDL_INTERFACE_INVALID_INTERFACE_ID      0xFFFFFFFF

/**
 * @enum    PDL_INTERFACE_MPP_INIT_VALUE_ENT
 *
 * @brief   Enumeration MPP init value.
 */

typedef enum {
    PDL_INTERFACE_MPP_INIT_VALUE_0_E,
    PDL_INTERFACE_MPP_INIT_VALUE_1_E,
    PDL_INTERFACE_MPP_INIT_VALUE_HW_E,
    PDL_INTERFACE_MPP_INIT_VALUE_LAST_E
}PDL_INTERFACE_MPP_INIT_VALUE_ENT;


/* size of above enumerator */
#define PDL_INTERFACE_I2C_ACCESS_SIZE_COUNT     3

 /**
  * @struct PDL_INTERFACE_OFFSET_MASK_VALUE_STC
  *
  * @brief  offset/mask/value container
  */

 typedef struct {
    /** @brief   The base address */
    UINT_32 baseAddress;
    /** @brief   The offset */
    UINT_32 offset;
    /** @brief   The mask */
    UINT_32	mask;
    /** @brief   The value */
    UINT_32	value;
} PDL_INTERFACE_OFFSET_MASK_VALUE_STC;

/**
 * @typedef UINT_32 PDL_INTERFACE_TYP
 *
 * @brief   Defines an alias representing the pdl interface typ
 */

/* internal driver id is reserved from 1000 */
#define PDL_INTERFACE_INTERNAL_DRIVER_ID_MIN            1000
#define PDL_INTERFACE_INTERNAL_DRIVER_ID_BOARD          1000
#define PDL_INTERFACE_INTERNAL_DRIVER_ID_POE            1001
#define PDL_INTERFACE_INTERNAL_DRIVER_ID_PHY            1002
#define PDL_INTERFACE_INTERNAL_DRIVER_ID_PP             1003
#define PDL_INTERFACE_INTERNAL_DRIVER_ID_CPU            1004



/** @brief   The interface type string to enum */
extern PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC prvPdlInterfaceTypeStrToEnum;

/**
 * @struct  PDL_INTERFACE_PRV_KEY_STC
 *
 * @brief   Interface key \n this structure is the key for searching interface in DB
 */

typedef struct {
    /** @brief   Identifier for the interface */
    PDL_INTERFACE_TYP                   interfaceId;
} PDL_INTERFACE_PRV_KEY_STC;

/**
 * @struct  PDL_INTERFACE_PRV_I2C_MUX_KEY_STC
 *
 * @brief   Interface key \n this structure is the key for searching interface in DB
 */

typedef struct {
    /** @brief   The multiplexer number */
    UINT_32                   muxNum;
} PDL_INTERFACE_PRV_I2C_MUX_KEY_STC;

 /**
  * @struct PDL_INTERFACE_PRV_I2C_DB_STC
  *
  * @brief  I2C db information \n this structure contains all that is required in order to perform
  *         a full (logical) I2C transaction.
  */

 typedef struct {
    /** @brief   Information describing the public */
    PDL_INTERFACE_I2C_STC	             publicInfo;
    /** @brief   The multiplexer database */
    PDL_DB_PRV_STC                       muxDb;

} PDL_INTERFACE_PRV_I2C_DB_STC;

 /**
  * @struct PDL_INTERFACE_PRV_GPIO_DB_STC
  *
  * @brief  GPIO db information \n this structure contains all that is required in order to
  *         perform a full (logical) GPIO transaction.
  */

 typedef struct {
    /** @brief   Information describing the public */
    PDL_INTERFACE_GPIO_STC	             publicInfo;

} PDL_INTERFACE_PRV_GPIO_DB_STC;

typedef enum {
    PDL_INTERFACE_PARSER_COOKIE_SMI_INFO_E,
    PDL_INTERFACE_PARSER_COOKIE_LAST_E
} PDL_INTERFACE_PARSER_COOKIE_ENT;

typedef struct {
    UINT_32                                     dev;
    UINT_32                                     port;
} PDL_INTERFACE_PARSER_COOKIE_SMI_INFO_STC;

typedef union {
    PDL_INTERFACE_PARSER_COOKIE_SMI_INFO_STC    smiInfo;
} PDL_INTERFACE_PARSERS_COOKIE_UNT;

typedef struct {
    PDL_INTERFACE_PARSER_COOKIE_ENT             cookieType;
    PDL_INTERFACE_PARSERS_COOKIE_UNT            cookieInfo;
} PDL_INTERFACE_PARSER_COOKIE_STC;


/**
 * @fn  PDL_STATUS prvPdlInterfaceXmlParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT PDL_INTERFACE_TYPE_ENT * interfaceTypePtr, OUT PDL_INTERFACE_TYP * interfaceIdPtr );
 *
 * @brief   parse XML instance of interface, insert it into DB and return a reference to it
 *
 * @param [in]  xmlId               -   XML descriptor pointing to a specific tag.
 * @param [in]  interfaceTypePtr    -   Type of interface retrieved from XML.
 * @param [out] interfaceIdPtr      -   GPIO interface id.
 *
 * @return  PDL_OK                      -       SUCCESS PDL_BAD_PTR                 -       NULL
 *          pointers as param PDL_XML_PARSE_ERROR         -       xmlId isn't pointing to an
 *          interface.
 */

PDL_STATUS prvPdlInterfaceXmlParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    OUT PDL_INTERFACE_TYPE_ENT                * interfaceTypePtr,
    OUT PDL_INTERFACE_TYP                     * interfaceIdPtr
);

/*$ END OF prvPdlInterfaceHwGetValue */

/* ***************************************************************************
* FUNCTION NAME: prvPdlInterfaceXmlParser
*
* DESCRIPTION:  parse XML instance of interface, & value. insert interface into DB and return a reference to it and to value
*
* PARAMETERS:   
*
* RETURNES:     PDL_OK                      -       SUCCESS
*               PDL_BAD_PTR                 -       NULL pointers as param
*               PDL_XML_PARSE_ERROR         -       xmlId isn't pointing to an interface
*                
*****************************************************************************/

PDL_STATUS prvPdlInterfaceAndValueXmlParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    IN  UINT_32                             sizeOfValue,
    OUT PDL_INTERFACE_TYPE_ENT            * interfaceTypePtr,
    OUT PDL_INTERFACE_TYP                 * interfaceIdPtr,
    OUT void                              * valuePtr
);

/**
 * @fn  PDL_STATUS prvPdlInterfaceXmlIsInterface ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT BOOLEAN * isInterface );
 *
 * @brief   returns TRUE if xmlId is pointing to an interface
 *
 * @param [in]  xmlId       -   XML descriptor pointing to a specific tag.
 * @param [out] isInterface -   Does current XML location specify an interface.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlInterfaceXmlIsInterface (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    OUT BOOLEAN                               * isInterface
);

/* @}*/
/* @}*/

#endif
