/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\serdes\private\prvpdlserdes.h.
 *
 * @brief   Declares the prvpdlserdes class
 */

#ifndef __prvPdlSerdesh

#define __prvPdlSerdesh
/**
********************************************************************************
 * @file prvPdlSerdes.h   
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
 * @brief Platform driver layer - Serdes private declarations and APIs
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/serdes/pdlSerdes.h>
#include <pdl/xml/private/prvXmlParser.h>


#define PRV_PDL_SERDES_SERDESES_TAG_NAME    "packet-processors-serdeses"

#define PRV_PDL_SERDES_PP_TAG_NAME          "Packet-Processor"

#define PRV_PDL_SERDES_PP_DEV_NUM_TAG_NAME  "pp-device-number"

#define PRV_PDL_SERDES_LANE_TAG_NAME        "lane"

#define PRV_PDL_SERDES_LANE_NUM_TAG_NAME    "lane-number"

#define PRV_PDL_SERDES_LANE_ATTR_TAG_NAME   "lane-attributes-group"

#define PRV_PDL_SERDES_LANE_PORTS_TAG_NAME  "lane-connected-ports-group"

#define PRV_PDL_SERDES_L1_INTRFACE_TAG_NAME "L1-interface-connected-pp-ports"


#define PRV_PDL_SERDES_FINE_TUNE_TAG_NAME   "fine-tune"

#define PRV_PDL_SERDES_TXAMPL_TAG_NAME      "txAmp"

#define PRV_PDL_SERDES_TXAMPLADJEN_TAG_NAME "txAmpAdjEn"

#define PRV_PDL_SERDES_TXAMPLSHTEN_TAG_NAME "txAmpShft"

#define PRV_PDL_SERDES_EMPH0_TAG_NAME       "emph0"

#define PRV_PDL_SERDES_EMPH1_TAG_NAME       "emph1"

#define PRV_PDL_SERDES_EMPH_EN_NAME         "txEmphEn"

#define PRV_PDL_SERDES_SLEW_RATE_NAME       "slewRate"

#define PRV_PDL_SERDES_TX_SWAP_TAG_NAME     "tx-swap"

#define PRV_PDL_SERDES_RX_SWAP_TAG_NAME     "rx-swap"

#define PRV_PDL_SERDES_PP_PORT_TAG_NAME     "pp-port-number"
#define PRV_PDL_SERDES_L1_IFMODE_TAG_NAME   "L1-interface-mode"
/**
* @addtogroup Serdes
* @{
*/

 /** 
 * @defgroup Serdes_private Serdes private
 * @{Serdes private definitions and declarations including:
*/

/**
 * @struct  PRV_PDL_LANE_ENTRY_KEY_STC
 *
 * @brief   defines Lane's key data
 */

typedef struct {
    UINT_32                             ppNumber;
    UINT_32                             laneId;
} PRV_PDL_LANE_ENTRY_KEY_STC;

/**
 * @struct  PRV_PDL_LANE_ENTRY_KEY_STC
 *
 * @brief   defines Lane's info data
 */
typedef struct {
    PDL_INTERFACE_MODE_ENT                      interfaceMode;
    PDL_CONNECTOR_TYPE_ENT                      connectorType;
} PRV_PDL_LANE_INFO_ENTRY_KEY_STC;


/**
 * @struct  PRV_PDL_LANE_INFO_ENTRY_STC
 *
 * @brief   defines Lane info data
 */

typedef struct {
    PRV_PDL_LANE_INFO_ENTRY_KEY_STC        key;
    BOOLEAN                                fineTuneTxExist;
    BOOLEAN                                fineTuneRxExist;
    PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC   fineTuneTxAttributes;
    PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC   fineTuneRxAttributes;
} PRV_PDL_LANE_INFO_ENTRY_STC;

/**
 * @struct  PRV_PDL_LANE_ENTRY_STC
 *
 * @brief   defines Lane's relevant data
 */

typedef struct {
    BOOLEAN                             txSwap;
    BOOLEAN                             rxSwap;
    PDL_DB_PRV_STC                      laneInfoDb;             /* DB of PRV_PDL_LANE_INFO_ENTRY_STC*/
} PRV_PDL_LANE_ENTRY_STC;

/**
 * @struct  PRV_PDL_PORT_LANE_KEY_STC
 *
 * @brief   defines structure for sfp key in db
 */

typedef struct {
    UINT_32                 dev;
    UINT_32                 port;
} PRV_PDL_PORT_KEY_STC;

/**
 * @struct  PRV_PDL_PORT_LANE_ENTRY_STC
 *
 * @brief   defines structure for port serdes data
 */

typedef struct {
    UINT_32                 numOfSerdes;
    PDL_DB_PRV_STC          portSerdesInfoDb;     /* DB of PRV_PDL_PORT_LANE_DATA_STC*/
} PRV_PDL_PORT_LANE_ENTRY_STC;


/**
 * @fn  PDL_STATUS prvPdlSerdesPortXmlParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, IN UINT_32 dev, IN UINT_32 port )
 *
 * @brief   Prv pdl serdes XML parser
 *
 * @param   xmlId   Identifier for the XML.
 * @param   dev     The dev.
 * @param   port    The port.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSerdesPortXmlParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    IN  UINT_32                                 dev,
    IN  UINT_32                                 port
);

/**
 * @fn  PDL_STATUS PdlSerdesInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlRootId );
 *
 * @brief   Init serdes module Build serdes DB from XML data
 *
 * @param [in]  xmlRootId   - Xml id.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS PdlSerdesInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlRootId
);

/**
 * @fn  PDL_STATUS prvPdlSerdesDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSerdesDestroy (
    void
);

/* @}*/
/* @}*/

#endif
