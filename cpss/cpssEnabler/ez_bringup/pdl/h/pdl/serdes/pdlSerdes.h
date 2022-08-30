/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\serdes\pdlserdes.h.
 *
 * @brief   Declares the pdlserdes class
 */

#ifndef __pdlSerdesh

#define __pdlSerdesh
/**
********************************************************************************
 * @file pdlSerdes.h
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
 * @brief Platform driver layer - SERDES library
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/lib/private/prvPdlLib.h>

/** 
 * @defgroup Serdes Serdes
 * @{SERDES public definitions and declarations
*/

#define PDL_SERDES_MAX_NUM_OF_PORTS_CNS     127
#define PDL_SERDES_MAX_NUM_OF_LANES_CNS     4


/**
* @struct  PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC
*
* @brief   A fine tune struct.
*/
typedef struct {
    UINT_8                          txAmpl;
    BOOLEAN                         txAmplAdjEn;
    BOOLEAN                         txAmplShtEn;
    UINT_8                          emph0;
    UINT_8                          emph1;
    BOOLEAN                         txEmphEn;
    UINT_8                          slewRate;
} PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC;

/**
* @struct  PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC
*
* @brief   A fine tune struct.
*/
typedef struct {
    UINT_32                         sqlch;
    UINT_32                         ffeRes;
    UINT_32                         ffeCap;
    UINT_32                         align90;
    UINT_32                         dcGain;
    UINT_32                         bandWidth;
} PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC;


/**
 * @struct  PDL_LANE_POLARITY_ATTRIBUTES_STC
 *
 * @brief   defines Lane's polarity attributes
 */

typedef struct {
    BOOLEAN                             txSwap;
    BOOLEAN                             rxSwap;   
} PDL_LANE_POLARITY_ATTRIBUTES_STC;

/**
 * @struct  PRV_PDL_PORT_LANE_DATA_STC
 *
 * @brief   defines structure for port serdes data entry in db
 */

typedef struct {
    UINT_32                 relSerdesNum;       
} PDL_PORT_LANE_DATA_KEY_STC;

/**
 * @struct  PRV_PDL_PORT_LANE_DATA_STC
 *
 * @brief   defines structure for port serdes data entry in db
 */

typedef struct {
    UINT_32                absSerdesNum;       /* 0..257*/
    UINT_8                 relSerdesNum;       /* 0..7*/
} PDL_PORT_LANE_DATA_STC;

/**
 * @fn  PDL_STATUS pdlSerdesDbPolarityAttrGet( IN UINT_32 ppId, IN UINT_32 laneId, OUT PDL_LANE_POLARITY_ATTRIBUTES_STC * polarityAttributesPtr );
 *
 * @brief   Get attributes of serdes for specified interfaceMode & connectorType
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                    devId
 * @param [in]  laneId                  serdes's absolute lane identifier.
 * @param [out] PolarityAttributesPtr   lane polarity attributes.
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given laneId
 * @return  PDL_BAD_OK                  lane polarity attributes found and returned
 */

PDL_STATUS pdlSerdesDbPolarityAttrGet(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    OUT PDL_LANE_POLARITY_ATTRIBUTES_STC                  * polarityAttributesPtr
);

/**
 * @fn  PDL_STATUS pdlSerdesDbPolarityAttrSet( IN UINT_32 ppId, IN UINT_32 laneId, IN PDL_LANE_POLARITY_ATTRIBUTES_STC * polarityAttributesPtr );
 *
 * @brief   Set polarity attributes of serdes
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                    devId
 * @param [in]  laneId                  serdes's absolute lane identifier.
 * @param [in]  polarityAttributesPtr   lane polarity attributes.
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given laneId
 * @return  PDL_BAD_OK                  lane polarity attributes found and returned
 */

PDL_STATUS pdlSerdesDbPolarityAttrSet(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    IN  PDL_LANE_POLARITY_ATTRIBUTES_STC                  * polarityAttributesPtr
);
/*$ END OF pdlSerdesDbPolarityAttrSet */

/**
 * @fn  PDL_STATUS pdlSerdesDbFineTuneAttrSet( IN UINT_32 ppId, IN UINT_32 laneId, IN PDL_INTERFACE_MODE_ENT interfaceMode IN PDL_CONNECTOR_TYPE_ENT connectorType, IN PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC * fineTuneTxAttributesPtr , IN PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC * fineTuneRxAttributesPtr );
 *
 * @brief   Set attributes of serdes for specified interfaceMode & connectorType
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                                devId
 * @param [in]  laneId                              serdes's absolute lane identifier.
 * @param [in]  interfaceMode                       interface mode.
 * @param [in]  connectorType                       connector Type
 * @param [out] fineTuneTxAttributesPtr             lane fine tune tx params or NULL.
 * @param [out] fineTuneRxAttributesPtr             lane fine tune rx params or NULL.
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given interfaceMode & connectorType
 * @return  PDL_OK                      lane attributes found and returned
 */
PDL_STATUS pdlSerdesDbFineTuneAttrSet(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    IN  PDL_INTERFACE_MODE_ENT                              interfaceMode,
    IN  PDL_CONNECTOR_TYPE_ENT                              connectorType,
    IN  PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC              * fineTuneTxAttributesPtr,
    IN  PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC              * fineTuneRxAttributesPtr
);
/*$ END OF pdlSerdesDbFineTuneAttrSet */

/**
 * @fn  PDL_STATUS pdlSerdesDbFineTuneAttrGet( IN UINT_32 ppId, IN UINT_32 laneId, IN PDL_INTERFACE_MODE_ENT interfaceMode IN PDL_CONNECTOR_TYPE_ENT connectorType, OUT PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC * fineTuneTxAttributesPtr );
 *
 * @brief   Get attributes of serdes for specified interfaceMode & connectorType
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                                devId
 * @param [in]  laneId                              serdes's absolute lane identifier.
 * @param [in]  interfaceMode                       interface mode.
 * @param [in]  connectorType                       connector Type
 * @param [out] fineTuneTxAttributesExistsPtr       lane fine tune tx params exist.
 * @param [out] fineTuneTxAttributesPtr             lane fine tune tx params.
 * @param [out] fineTuneRxAttributesExistsPtr       lane fine tune rx params exist.
 * @param [out] fineTuneRxAttributesPtr             lane fine tune rx params.
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given interfaceMode & connectorType
 * @return  PDL_BAD_OK                  lane attributes found and returned
 */

PDL_STATUS pdlSerdesDbFineTuneAttrGet(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    IN  PDL_INTERFACE_MODE_ENT                              interfaceMode,
    IN  PDL_CONNECTOR_TYPE_ENT                              connectorType,
    OUT BOOLEAN                                           * fineTuneTxAttributesExistsPtr,
    OUT PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC              * fineTuneTxAttributesPtr,
    OUT BOOLEAN                                           * fineTuneRxAttributesExistsPtr,
    OUT PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC              * fineTuneRxAttributesPtr
);

/**
 * @fn  PDL_STATUS pdlSerdesDbFineTuneAttrGetFirst( IN UINT_32 ppId, IN UINT_32 laneId, OUT PDL_INTERFACE_MODE_ENT *interfaceModePtr, OUT PDL_CONNECTOR_TYPE_ENT *connectorTypePtr );
 *
 * @brief   Get serdes first interfaceMode and connectorType 's
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                                devId
 * @param [in]  laneId                              serdes's absolute lane identifier.
 * @param [out] interfaceModePtr                    first interface mode.
 * @param [out] connectorTypePtr                    first connector Type
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given interfaceMode & connectorType
 * @return  PDL_BAD_OK                  lane attributes found and returned
 */
PDL_STATUS pdlSerdesDbFineTuneAttrGetFirst(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    OUT PDL_INTERFACE_MODE_ENT                             *interfaceModePtr,
    OUT PDL_CONNECTOR_TYPE_ENT                             *connectorTypePtr
);
/*$ END OF pdlSerdesDbFineTuneAttrGetFirst */

/**
 * @fn  PDL_STATUS pdlSerdesDbFineTuneAttrGetNext( IN UINT_32 ppId, IN UINT_32 laneId, IN PDL_INTERFACE_MODE_ENT currInterfaceMode, IN PDL_CONNECTOR_TYPE_ENT currConnectorType, OUT PDL_INTERFACE_MODE_ENT *nextInterfaceModePtr, OUT PDL_CONNECTOR_TYPE_ENT *nextConnectorTypePtr );
 *
 * @brief   Get serdes next interfaceMode and connectorType 's
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                                    devId
 * @param [in]  laneId                                  serdes's absolute lane identifier.
 * @param [in]  currInterfaceMode                       current interface mode.
 * @param [in]  currConnectorType                       current connector Type
 * @param [out] nextInterfaceModePtr                    next interface mode.
 * @param [out] nextConnectorTypePtr                    next connector Type
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given interfaceMode & connectorType
 * @return  PDL_BAD_OK                  lane attributes found and returned
 */
PDL_STATUS pdlSerdesDbFineTuneAttrGetNext(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    IN  PDL_INTERFACE_MODE_ENT                              currInterfaceMode,
    IN  PDL_CONNECTOR_TYPE_ENT                              currConnectorType,
    OUT PDL_INTERFACE_MODE_ENT                             *nextInterfaceModePtr,
    OUT PDL_CONNECTOR_TYPE_ENT                             *nextConnectorTypePtr
);
/*$ END OF pdlSerdesDbFineTuneAttrGetNext */

/**
 * @fn  PDL_STATUS pdlSerdesPortDbSerdesGetFirst( IN UINT_32 dev, IN UINT_32 port, OUT PDL_PORT_LANE_DATA_STC * serdesInfo );
 *
 * @brief   Get info for first serdes
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in]  dev                     packet processor identifier.
 * @param [in]  port                    port
 * @param [out] serdesInfo              first serdes connected to port
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given dev & port
 * @return  PDL_BAD_OK                  lane attributes found and returned
 */
PDL_STATUS pdlSerdesPortDbSerdesInfoGetFirst(
    IN  UINT_32                                             dev,
    IN  UINT_32                                             port,
    OUT PDL_PORT_LANE_DATA_STC                            * serdesInfoPtr
);

/*$ END OF pdlSerdesPortDbSerdesInfoGetFirst */

/**
 * @fn  PDL_STATUS pdlSerdesPortDbSerdesInfoGet( IN UINT_32 ppId, IN UINT_32 laneId, IN PDL_INTERFACE_MODE_ENT interfaceMode, OUT PDL_L1_INTERFACE_CONNECTED_PORTS_GROUP_DATA_STC * portsGroupAttributesPtr );
 *
 * @brief   Get port serdes information for dev & port
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in]  dev                     packet processor identifier.
 * @param [in]  port                    port
 * @param [in]  serdesInfoPtr           current serdes info
 * @param [out] serdesNextInfoPtr       next port's serdes info
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find port index for given dev & port
 * @return  PDL_BAD_OK                  lane attributes found and returned
 */
PDL_STATUS pdlSerdesPortDbSerdesInfoGetNext(
    IN  UINT_32                                             dev,
    IN  UINT_32                                             port,
    IN  PDL_PORT_LANE_DATA_STC                            * serdesInfoPtr,
    OUT PDL_PORT_LANE_DATA_STC                            * serdesNextInfoPtr
);

/*$ END OF pdlSerdesPortDbSerdesInfoGetNext */

/**
 * @fn  PDL_STATUS pdlSerdesDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Serdes debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSerdesDebugSet (
    IN  BOOLEAN             state
);

#endif
