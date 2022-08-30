/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\power_supply\private\prvpdlpower.h.
 *
 * @brief   Declares the prvpdlpower class
 */

#ifndef __prvPdlPowerh

#define __prvPdlPowerh
/**
********************************************************************************
 * @file prvPdlPower.h   
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
 * @brief Platform driver layer - Power private declarations and APIs
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/power_supply/pdlPower.h>
#include <pdl/xml/private/prvXmlParser.h>


/**
* @addtogroup Power
* @{
*/

#define PDL_POWER_BANK_INVALID              0xFFFF

/**
 * @struct  PDL_POWER_PSE_ENUM_STR_STC
 *
 * @brief   used for converting between pse enum id and string found inside XML file
 */

typedef struct {
    /** @brief   The name */
    char                            * name;
    /** @brief   Type of the pse */
    PDL_POWER_PORT_TYPE_ENT           pseType;
} PDL_POWER_PSE_ENUM_STR_STC;


 /** 
 * @defgroup Power_private Power Private
 * @{Putton private definitions and declarations including: 
*/

/**
 * @struct  PDL_POWER_PORT_PRV_KEY_STC
 *
 * @brief   defines structure for power port key in db
 */

typedef struct {
    /** @brief   The development */
    UINT_32                                 dev;
    /** @brief   The port */
    UINT_32                                 logicalPort;
} PDL_POWER_PORT_PRV_KEY_STC;

/**
 * @struct  PDL_POWER_PORT_PRV_DB_STC
 *
 * @brief   defines structure stored for power port in db
 */

typedef struct {
    /** @brief   The poe capability */
    PDL_POWER_POE_CAPABILITY_ENT            poeCapability;
    /** @brief   Information describing the pse */
    PDL_POWER_PSE_PORT_INFO_STC             pseInfo;
    /** @brief   The pd port interface */
    PDL_POWER_PD_PORT_INTERFACE_STC         pdPortInterface;
} PDL_POWER_PORT_PRV_DB_STC;

typedef struct {
    UINT_32                                 index;
} PDL_POWER_RPS_STATUS_PRV_KEY_STC;

typedef struct {
    PDL_POWER_RPS_STATUS_PRV_KEY_STC        key;
    PDL_INTERFACE_TYPE_ENT                  interfaceType;    
    PDL_INTERFACE_TYP                       interfaceId;
    PDL_POWER_PS_VALUES_STC                 values;
} PDL_POWER_RPS_STATUS_PRV_INTERFACE_STC;

typedef struct {
    UINT_8                                  number;
} PDL_POWER_PSE_LIST_PRV_KEY_STC;

/**
 * @fn  PDL_STATUS pdlPowerInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId );
 *
 * @brief   Init power module Create button DB and initialize
 *
 * @param [in]  xmlId   - Xml id.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlPowerInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
);

/**
 * @fn  PDL_STATUS prvPdlPowerDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPowerDestroy (
    void
);

/* @}*/
/* @}*/

#endif
