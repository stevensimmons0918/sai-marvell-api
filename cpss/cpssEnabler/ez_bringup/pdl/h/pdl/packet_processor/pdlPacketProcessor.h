/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\packet_processor\pdlpacketprocessor.h.
 *
 * @brief   Declares the pdlpacketprocessor class
 */

#ifndef __pdlPacketProcessorh

#define __pdlPacketProcessorh
/**
********************************************************************************
 * @file pdlPacketProcessor.h   
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
 * @brief Platform driver layer - Packet Processor related API
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/phy/pdlPhy.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <pdl/phy/pdlPhy.h>

 /** 
 * @defgroup Packet_Processor Packet Processor
 * @{
 * 
*/

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

/**
 * @struct  PDL_PP_XML_ATTRIBUTES_STC
 *
 * @brief   defines packet processor attributes
 */

typedef struct {
    /** @brief   Number of pps */
    UINT_32                             numOfPps;
    /** @brief   Number of back to back links per pps */
    UINT_32                             numOfBackToBackLinksPerPp;
    /** @brief   Number of front panel groups */
    UINT_32                             numOfFrontPanelGroups;
} PDL_PP_XML_ATTRIBUTES_STC;

/**
 * @enum    PDL_PP_FRONT_PANEL_GROUP_ORDERING_ENT
 *
 * @brief   Enumerator for panel port ordering type
 */

typedef enum {
  PDL_PP_FRONT_PANEL_GROUP_ORDERING_RIGHT_DOWN_E,
  PDL_PP_FRONT_PANEL_GROUP_ORDERING_DOWN_RIGHT_E,
  PDL_PP_FRONT_PANEL_GROUP_ORDERING_SINGLE_2ND_ROW_E,
} PDL_PP_FRONT_PANEL_GROUP_ORDERING_ENT;

/**
 * @struct  PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC
 *
 * @brief   defines front panel attributes
 */

typedef struct {
    /** @brief   The ordering */
    PDL_PP_FRONT_PANEL_GROUP_ORDERING_ENT   ordering;
    /** @brief   The port maximum speed */
    PDL_PORT_SPEED_ENT	                    portMaximumSpeed;
    /** @brief   The first port number shift */
    UINT_32                                 firstPortNumberShift;
    /** @brief   islogical port different from mac port */
    BOOLEAN                                 isLogicalPortRequired;
    /** @brief   Number of ports in groups */
    UINT_32                                 numOfPortsInGroup;
} PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC;

/**
 * @struct  PDL_L1_INTERFACE_MODE_STC
 *
 * @brief   defines L1 interface mode attributes
 */

typedef struct {
    /** @brief   The speed */
    PDL_PORT_SPEED_ENT          speed;
    /** @brief   The mode */
    PDL_INTERFACE_MODE_ENT      mode;
} PDL_L1_INTERFACE_MODE_STC;

/**
 * @struct  PDL_PP_XML_MAC_LOGICAL_PORT_ATTRIBUTES_STC
 *
 * @brief   defines mac-logical port attributes
 */

typedef struct {
    /** @brief   The dev number */
    UINT_32                             dev;
    /** @brief   The logical port */
    UINT_32                             logicalPort;
} PDL_PP_XML_MAC_LOGICAL_PORT_ATTRIBUTES_STC;

/**
 * @struct  PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC
 *
 * @brief   defines network port attributes
 */

typedef struct {
    /** @brief   Group the port number in belongs to */
    UINT_32                             portNumberInGroup;
    /** @brief   The mac port */
    UINT_32                             macPort;
    /** @brief   The front panel number */
    UINT_32                             frontPanelNumber;
    /** @brief   True if is phy exists, false if not */
    BOOLEAN                             isPhyExists;
    /** @brief   Information describing the phy */
    PDL_PHY_PORT_CONFIGURATION_STC      phyData;
    /** @brief   Type of the transceiver */
    PDL_TRANSCEIVER_TYPE_ENT            transceiverType;
    /** @brief   swap tx and rx lanes ABCD to DCBA */
    BOOLEAN            					swapAbcd;
    /** @brief   Number of fiber modes */
    UINT_32                             numOfFiberModes;
    /** @brief   The fiber modes arr[ pdl port speed last e] */
    PDL_L1_INTERFACE_MODE_STC           fiberModesArr[PDL_PORT_SPEED_LAST_E];
    /** @brief   Number of copper modes */
    UINT_32                             numOfCopperModes;
    /** @brief   The copper modes arr[ pdl port speed last e] */
    PDL_L1_INTERFACE_MODE_STC           copperModesArr[PDL_PORT_SPEED_LAST_E];
} PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC;


/**
 * @struct  PDL_PP_XML_B2B_ATTRIBUTES_STC
 *
 * @brief   defines back-to-back attributes
 */

typedef struct {
    /** @brief   The first device */
    UINT_32                             firstDev;
    /** @brief   The first port */
    UINT_32                             firstPort;
    UINT_32                             firstMacPort;
    /** @brief   The second device */
    UINT_32                             secondDev;
    /** @brief   The second port */
    UINT_32                             secondPort;
    UINT_32                             secondMaclPort;
    /** @brief   The maximum speed */
    PDL_PORT_SPEED_ENT                  maxSpeed;
    /** @brief   The interface mode */
    PDL_INTERFACE_MODE_ENT              interfaceMode;
} PDL_PP_XML_B2B_ATTRIBUTES_STC;

/**
 * @struct  PDL_PP_XML_NETWORK_PORT_PHY_INIT_KEY_STC
 *
 * @brief   defines key to phy init values
 */
typedef struct {
    UINT_32                                     index;
} PDL_PP_XML_NETWORK_PORT_PHY_INIT_KEY_STC;

/**
 * @struct  PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC
 *
 * @brief   defines entry of phy init values
 */
typedef struct {
    PDL_PP_XML_NETWORK_PORT_PHY_INIT_KEY_STC    key;
    PDL_PHY_SMI_XSMI_VALUE_STC                  info;
} PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC;

/**
 * @fn  PDL_STATUS pdlPpDbAttributesGet ( OUT PDL_PP_XML_ATTRIBUTES_STC * ppAttributesPtr );
 *
 * @brief   Get packet processor attributes
 *
 * @param [out] ppAttributesPtr packet processor attributes.
 *
 * @return  PDL_BAD_PTR  illegal pointer supplied.
 */

PDL_STATUS pdlPpDbAttributesGet (
    OUT  PDL_PP_XML_ATTRIBUTES_STC         * ppAttributesPtr
);

/**
 * @fn  PDL_STATUS pdlPpDbB2bAttributesGet ( IN UINT_32 b2bLinkId, OUT PDL_PP_XML_B2B_ATTRIBUTES_STC * b2bAttributesPtr );
 *
 * @brief   Get PP back-to-back link attributes
 *
 * @param [in]  b2bLinkId           b2b link number.
 * @param [out] b2bAttributesPtr    b2b link attributes.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_BAD_PARAM  b2b id is out of range.
 */

PDL_STATUS pdlPpDbB2bAttributesGet (
    IN   UINT_32                                 b2bLinkId,
    OUT  PDL_PP_XML_B2B_ATTRIBUTES_STC         * b2bAttributesPtr
);

/**
 * @fn  PDL_STATUS pdlPpDbB2bAttributesSet ( IN UINT_32 b2bLinkId, OUT PDL_PP_XML_B2B_ATTRIBUTES_STC * b2bAttributesPtr )
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
 * @fn  PDL_STATUS pdlPpDbIsB2bLink ( IN UINT_32 dev, IN UINT_32 logicalPort, OUT BOOLEAN * isB2bLinkPtr )
 *
 * @brief   Determines whether PP port is back-to-back link.
 *
 * @param [in]  dev             dev number.
 * @param [in]  logicalPort     port number.
 * @param [out] isB2bLinkPtr    is b2b link.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  no such port was found.
 */
PDL_STATUS pdlPpDbIsB2bLink (
    IN   UINT_32            dev,
    IN   UINT_32            logicalPort,
    OUT  BOOLEAN          * isB2bLinkPtr
);
/*$ END OF pdlPpDbIsB2bLink */

/**
* @public pdlPpDbFirstPortAttributesGet
*
* @brief  Get first existing front panel number.
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*
* @param[out] pdlPpDbFirstFrontPanelGet  first front panel number
*/
PDL_STATUS pdlPpDbFirstFrontPanelGet (
    OUT  UINT_32                *firstFrontPanelPtr
);

/**
* @public pdlPpDbFrontPanelGetNext
*
* @brief  Get next existing front panel
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*
* @param[in] frontPanenlNumber          current front panel number
* @param[out] nextFrontPanenlNumberPtr  next front panel number
*/
PDL_STATUS pdlPpDbFrontPanelGetNext (
    IN   UINT_32                frontPanenlNumber,
    OUT  UINT_32                *nextFrontPanenlNumberPtr
);

/**
 * @fn  PDL_STATUS pdlPpDbFrontPanelAttributesGet ( IN UINT_32 frontPanelNumber, OUT PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC * frontPanelAttributesPtr );
 *
 * @brief   Get attributes for front panel group
 *
 * @param [in]  frontPanelNumber        front panel group id.
 * @param [out] frontPanelAttributesPtr front panel attributes.
 *
 * @return  PDL_BAD_PTR  illegal pointer supplied.
 */

PDL_STATUS pdlPpDbFrontPanelAttributesGet (
    IN   UINT_32                                 frontPanelNumber,
    OUT  PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC * frontPanelAttributesPtr
);

/**
 * @fn  PDL_STATUS pdlPpDbFrontPanelAttributesSet ( IN UINT_32 frontPanelNumber, OUT PDL_PP_XML_FRONT_PANEL_ATTRIBUTES_STC * frontPanelAttributesPtr )
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
 * @fn  PDL_STATUS pdlPpDbPortAttributesGet ( IN UINT_32 dev, IN UINT_32 logicalPort, OUT PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC * portAttributesPtr );
 *
 * @brief   Get PP port attributes
 *
 * @param [in]  dev                 dev number.
 * @param [in]  logicalPort         port number.
 * @param [out] portAttributesPtr   port attributes.
 *
 * @return  PDL_BAD_PTR  illegal pointer supplied.
 */

PDL_STATUS pdlPpDbPortAttributesGet (
    IN   UINT_32                                    dev,
    IN   UINT_32                                    logicalPort,
    OUT  PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC   * portAttributesPtr
);

/**
 * @fn  PDL_STATUS pdlPpDbDevAttributesGet ( IN UINT_32 dev, OUT UINT_32 * numOfPortsPtr );
 *
 * @brief   Get number of PP port
 *
 * @param [in]  dev             dev number.
 * @param [out] numOfPortsPtr   device's number of ports.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlPpDbDevAttributesGet (
    IN   UINT_32                                    dev,
    OUT  UINT_32                                  * numOfPortsPtr
);

/**
 * @fn  PDL_STATUS pdlPpPortConvertFrontPanel2DevPort ( IN UINT_32 frontPanelNumber, IN UINT_32 portNumberInGroup, OUT UINT_32 * devPtr, OUT UINT_32 * portPtr );
 *
 * @brief   Convert between front panel group &amp; port to dev &amp; port
 *
 * @param [in]      frontPanelNumber    front panel group id.
 * @param           portNumberInGroup   Group the port number in belongs to.
 * @param [in,out]  devPtr              If non-null, the development pointer.
 * @param [in,out]  portPtr             If non-null, the port pointer.
 *
 * @return  PDL_OK         convert succesfull.
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 */

PDL_STATUS pdlPpPortConvertFrontPanel2DevPort (
    IN   UINT_32                                 frontPanelNumber,
    IN   UINT_32                                 portNumberInGroup,
    OUT  UINT_32                               * devPtr,
    OUT  UINT_32                               * portPtr
);

/**
 * @fn  PDL_STATUS pdlPpPortConvertDevPort2FrontPanel ( IN UINT_32 dev, IN UINT_32 logicalPort, OUT UINT_32 * frontPanelNumberPtr, OUT UINT_32 * portNumberInGroupPtr );
 *
 * @brief   Convert between dev &amp; port to front panel group &amp; port
 *
 * @param [in]      dev                     packet processor device number.
 * @param [in]      logicalPort             packet processor port number.
 * @param [out]     frontPanelNumberPtr     front panel group id.
 * @param [in,out]  portNumberInGroupPtr    If non-null, the port number in group pointer.
 *
 * @return  PDL_OK         convert succesfull.
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 */

PDL_STATUS pdlPpPortConvertDevPort2FrontPanel (
    IN  UINT_32                                  dev,
    IN  UINT_32                                  logicalPort,
    OUT UINT_32                                * frontPanelNumberPtr,
    OUT UINT_32                                * portNumberInGroupPtr
);

/**
 * @fn  PDL_STATUS pdlPpPortConvertMacToLogical (IN UINT_32 dev, IN UINT_32 macPort, OUT UINT_32 * logicalPortPtr );
 *
 * @brief   Convert between macPort to logicalPort.
 *
 * @param [in]      dev                     dev number.
 * @param [in]      macPort                 macPort number.
 * @param [out]     logicalPortPtr          logicalPort number.
 *
 * @return  PDL_OK         convert succesfull.
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 */

PDL_STATUS pdlPpPortConvertMacToLogical (
    IN  UINT_32                                  dev,
    IN  UINT_32                                  macPort,
    OUT UINT_32                               *  logicalPortPtr,
    OUT UINT_32                               *  devNum
);


/**
 * @fn  PDL_STATUS pdlPpPortConvertLogicalToMac (IN UINT_32 dev, IN UINT_32 logicalPort, OUT UINT_32 * macPortPtr );
 *
 * @brief   Convert between logicalPort to macPort.
 *
 * @param [in]      dev                     dev number.
 * @param [in]      logiaclPort             logicalPort number.
 * @param [out]     macPortPtr              macPort number.
 *
 * @return  PDL_OK         convert succesfull.
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 */

PDL_STATUS pdlPpPortConvertLogicalToMac (
    IN  UINT_32                                  dev,
    IN  UINT_32                                  logicalPort,
    OUT UINT_32                               *  macPortPtr,
    OUT UINT_32                               *  devNum
);
/**
* @public pdlPpDbFirstPortAttributesGet
*
* @brief  Get first existing dev/port attributes.
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*
* @param[out] firstDevPtr                first dev number
* @param[out] firstPortPtr               first port number
*/

PDL_STATUS pdlPpDbFirstPortAttributesGet (
    OUT  UINT_32                                   *firstDevPtr,
    OUT  UINT_32                                   *firstPortPtr
);

/**
* @public pdlPpDbPortAttributesGetNext
*
* @brief  Get next existing dev/port.
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*
* @param[in] currDev                    current dev number
* @param[in] currPort                   current port number
* @param[out] nextDevPtr                next dev number
* @param[out] nextPortPtr               next port number
*/
PDL_STATUS pdlPpDbPortAttributesGetNext (
    IN   UINT_32                                    dev,
    IN   UINT_32                                    logicalPort,
    OUT  UINT_32                                   *nextDevPtr,
    OUT  UINT_32                                   *nextPortPtr
);

/**
* @public pdlPpDbDevAttributesGetFirstPort
*
* @brief  Get first existing port of device.
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*
* @param[in]  dev                dev number
* @param[out] firstPortPtr       first port number
*/
PDL_STATUS pdlPpDbDevAttributesGetFirstPort (
    IN   UINT_32       dev,
    OUT  UINT_32       *firstPortPtr
);
/*$ END OF pdlPpDbDevAttributesGetFirstPort */

/**
* @public pdlPpDbDevAttributesGetNextPort
*
* @brief  Get next existing port of device.
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*
* @param[in]  dev                dev number
* @param[in]  currentPort        current port number
* @param[out] nextPortPtr        next port number
*/
PDL_STATUS pdlPpDbDevAttributesGetNextPort (
    IN   UINT_32       dev,
    IN   UINT_32       currentPort,
    OUT  UINT_32       *nextPortPtr
);
/*$ END OF pdlPpDbDevAttributesGetNextPort */

/**
 * @fn  PDL_STATUS pdlPpDbPortPhySetInitValues ()
 *
 * @brief   write port phy init values for given speed & transceiver type
 *
 * @param [in]  dev                 dev number.
 * @param [in]  logicalPort         port number.
 * @param [in]  speed               port speed
 * @param [in]  transceiverType     transceiverType (copper/fiber)
 *
 * @return  PDL_BAD_PARAM       illegal transceiverType
            PDL_NO_SUCH         phy_init configuration is required but not for this speed & transceiverType
            PDL_NOT_SUPPORTED   phy_init configuration isn't required
            PDL_OK              phy_init configured ok
 */
 PDL_STATUS pdlPpDbPortPhySetInitValues (
    IN   UINT_32                                      dev,
    IN   UINT_32                                      logicalPort,
    IN   PDL_PORT_SPEED_ENT                           speed,
    IN   PDL_TRANSCEIVER_TYPE_ENT                     transceiverType
);

/**
 * @fn  PDL_STATUS pdlPpDbPortPhyInitValuesGetFirst ()
 *
 * @brief   Get first port phy init values
 *
 * @param [in]     dev              dev number.
 * @param [in]     logicalPort      port number.
 * @param [in]     speed            port speed
 * @param [in]     transceiverType  transceiverType (copper/fiber)
 * @param [inout]  initValuesPtr    ptr to data.
 *
 * @return  PDL_BAD_PARAM        illegal speed or transceiverType
 * @return  PDL_BAD_PTR          if initValuesPtr is NULL
 * @return  PDL_NO_MORE          No more data available
 */
PDL_STATUS pdlPpDbPortPhyInitValuesGetFirst (
    IN    UINT_32                                     dev,
    IN    UINT_32                                     logicalPort,
    IN    PDL_PORT_SPEED_ENT                          speed,
    IN    PDL_TRANSCEIVER_TYPE_ENT                    transceiverType,
    INOUT PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC  *initValuesPtr
);
/*$ END OF pdlPpDbPortPhyInitValuesGetFirst */

/**
 * @fn  PDL_STATUS pdlPpDbPortPhyInitValuesGetNext ()
 *
 * @brief   Get next port phy init values
 *
 * @param [in]     dev              dev number.
 * @param [in]     logicalPort      port number.
 * @param [in]     speed            port speed
 * @param [in]     transceiverType  transceiverType (copper/fiber)
 * @param [inout]  initValuesPtr    ptr to data.
 *
 * @return  PDL_BAD_PARAM        illegal speed or transceiverType
 * @return  PDL_BAD_PTR          if initValuesPtr is NULL
 * @return  PDL_NO_MORE          No more data available
 */
PDL_STATUS pdlPpDbPortPhyInitValuesGetNext (
    IN    UINT_32                                     dev,
    IN    UINT_32                                     logicalPort,
    IN    PDL_PORT_SPEED_ENT                          speed,
    IN    PDL_TRANSCEIVER_TYPE_ENT                    transceiverType,
    INOUT PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC  *initValuesPtr
);
/*$ END OF pdlPpDbPortPhyInitValuesGetNext */

 /**
 * @fn  PDL_STATUS pdlPpDbPortPhySetPostInitValues ()
 *
 * @brief   write port phy post init values
 *
 * @param [in]  dev                 dev number.
 * @param [in]  logicalPort         port number.
 *
 * @return  PDL_OK              Registers configured ok or not required
 */

PDL_STATUS pdlPpDbPortPhySetPostInitValues (
    IN   UINT_32                                      dev,
    IN   UINT_32                                      logicalPort
);

/**
 * @fn  PDL_STATUS pdlPpDbPortPhyPostInitValuesGetFirst ()
 *
 * @brief   Get first port phy post init values
 *
 * @param [in]  dev              dev number.
 * @param [in]  logicalPort      port number.
 * @param [out] initValuesPtr    ptr to data.
 *
 * @return  PDL_BAD_PTR          if initValuesPtr is NULL
 * @return  PDL_NO_MORE          No more data available
 */
PDL_STATUS pdlPpDbPortPhyPostInitValuesGetFirst (
    IN  UINT_32                                     dev,
    IN  UINT_32                                     logicalPort,
    OUT PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC  *initValuesPtr
);
/*$ END OF pdlPpDbPortPhyPostInitValuesGetFirst */

/**
 * @fn  PDL_STATUS pdlPpDbPortPhyPostInitValuesGetNext ()
 *
 * @brief   Get next port phy post init values
 *
 * @param [in]    dev              dev number.
 * @param [in]    logicalPort      port number.
 * @param [inout] initValuesPtr    ptr to data.
 *
 * @return  PDL_BAD_PTR          if initValuesPtr is NULL
 * @return  PDL_NO_MORE          No more data available
 */
PDL_STATUS pdlPpDbPortPhyPostInitValuesGetNext (
    IN    UINT_32                                     dev,
    IN    UINT_32                                     logicalPort,
    INOUT PDL_PP_XML_NETWORK_PORT_PHY_INIT_INFO_STC  *initValuesPtr
);
/*$ END OF pdlPpDbPortPhyPostInitValuesGetNext */

/**
 * @fn  PDL_STATUS pdlPpDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Pp debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPpDebugSet (
    IN  BOOLEAN             state
);

/* @}*/

#endif
