/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\packet_processor\private\prvpdlpacketprocessor.h.
 *
 * @brief   Declares the prvpdlpacketprocessor class
 */

#ifndef __prvPdlPacketProcessorh

#define __prvPdlPacketProcessorh
/**
********************************************************************************
 * @file prvPdlPacketProcessor.h   
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
 * @brief Platform driver layer - Packet Processor private declarations and APIs
 * 
 * @version   1 
********************************************************************************
*/
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>

/**
* @addtogroup Packet_Processor
* @{
*/
 /** 
 * @defgroup Packet_processor_private Packet Processor Private
 * @{Button private definitions and declarations including:
*/

/*! Max number of Back to Back packet processors */
#define PRV_PDL_PP_B2B_MAX_NUM                          10

#define PRV_PDL_FP_FIRST_PORT_NUMBER_DEFAULT_SHIFT_CNS  48

/**
 * @struct  PRV_PDL_PORT_ENTRY_KEY_STC
 *
 * @brief   defines structure for port key in db
 */

typedef struct {
    /** @brief   The development */
    UINT_32                                 dev;
    /** @brief   The mac port */
    UINT_32                                 logicalPort;
} PRV_PDL_PORT_ENTRY_KEY_STC;


/**
 * @struct  PRV_PDL_PORT_ENTRY_STC
 *
 * @brief   defines structure for port entry in db
 */

typedef struct {
    /** @brief   The key */
    PRV_PDL_PORT_ENTRY_KEY_STC              key;
    /** @brief   The data */
    PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC  data;
    PDL_DB_PRV_STC                          fiberPhyInitValueDb[PDL_PORT_SPEED_LAST_E];
    PDL_DB_PRV_STC                          copperPhyInitValueDb[PDL_PORT_SPEED_LAST_E];
    PDL_INTERFACE_TYPE_ENT                  phyInterfaceType;    
    PDL_INTERFACE_TYP                       phyInterfaceId;
    PDL_DB_PRV_STC                          phyPostInitValues;
} PRV_PDL_PORT_ENTRY_STC;

/**
 * @struct  PRV_PDL_MAC_PORT_ENTRY_KEY_STC
 *
 * @brief   defines structure for logical port key in db
 */

typedef struct {
    /** @brief   The development */
    UINT_32                                 dev;
    /** @brief   The logical port */
    UINT_32                                 macPort;
} PRV_PDL_MAC_PORT_ENTRY_KEY_STC;


typedef struct {
    /** @brief   The key */
    PRV_PDL_MAC_PORT_ENTRY_KEY_STC              key;
    /** @brief   The data */       
    PDL_PP_XML_MAC_LOGICAL_PORT_ATTRIBUTES_STC  data;                                
} PRV_PDL_MAC_PORT_ENTRY_STC;

/**
 * @struct  PRV_PDL_FRONT_PANEL_GROUP_KEY_STC
 *
 * @brief   defines structure for front panel key in db
 */

typedef struct {
    /** @brief   The front panel number */
    UINT_32                                 frontPanelNumber;
} PRV_PDL_FRONT_PANEL_GROUP_KEY_STC;

/**
 * @struct  PRV_PDL_FRONT_PANEL_GROUP_ENTRY_STC
 *
 * @brief   defines structure for front panel entry in db
 */

typedef struct {
    /** @brief   The key */
    PRV_PDL_FRONT_PANEL_GROUP_KEY_STC       key;

    /** @brief   The data */
    PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC   data;
} PRV_PDL_FRONT_PANEL_GROUP_ENTRY_STC;

/**
 * @fn  PDL_STATUS pdlPacketProcessorInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlRootId );
 *
 * @brief   Init Packet Processor module Create DB and initialize
 *
 * @param [in]  xmlRootId   Xml id.
 *
 * @return  PDL_NOT_FOUND if xml parsing wasn't successful because mandatory tag not found.
 * @return  PDL_CREATE_ERROR if number of packet processors doesn't match xml parsing.
 */

 PDL_STATUS pdlPacketProcessorInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlRootId
);

/**
 * @fn  PDL_STATUS pdlPpDbPortAttributesSet ( IN UINT_32 dev, IN UINT_32 port, OUT PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC * portAttributesPtr );
 *
 * @brief   Update PP port attributes
 *
 * @param [in]  dev                 dev number.
 * @param [in]  port                port number.
 * @param [out] portAttributesPtr   port attributes.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  port's entry wasn't found.
 */

PDL_STATUS pdlPpDbPortAttributesSet (
    IN   UINT_32                                    dev,
    IN   UINT_32                                    logicalPort,
    OUT  PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC   * portAttributesPtr
);

/**
 * @fn  PDL_STATUS pdlPpDbB2bAttributesSet ( IN UINT_32 b2bLinkId, OUT PDL_PP_XML_B2B_ATTRIBUTES_STC * b2bAttributesPtr );
 *
 * @brief   Update PP back-to-back link attributes
 *
 * @param [in]  b2bLinkId           b2b link number.
 * @param [out] b2bAttributesPtr    b2b link attributes.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  b2b's entry wasn't found.
 */

PDL_STATUS pdlPpDbB2bAttributesSet (
    IN   UINT_32                                 b2bLinkId,
    OUT  PDL_PP_XML_B2B_ATTRIBUTES_STC         * b2bAttributesPtr
);

/**
 * @fn  PDL_STATUS pdlPpDbFrontPanelAttributesSet ( IN UINT_32 frontPanelNumber, OUT PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC * frontPanelAttributesPtr );
 *
 * @brief   Update attributes of front panel group
 *
 * @param [in]  frontPanelNumber        front panel group id.
 * @param [out] frontPanelAttributesPtr front panel attributes.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 */

PDL_STATUS pdlPpDbFrontPanelAttributesSet (
    IN   UINT_32                                 frontPanelNumber,
    OUT  PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC * frontPanelAttributesPtr
);

/**
 * @fn  PDL_STATUS prvPdlPacketProcessorDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPacketProcessorDestroy (
    void
);

/* @}*/
/* @}*/

#endif
