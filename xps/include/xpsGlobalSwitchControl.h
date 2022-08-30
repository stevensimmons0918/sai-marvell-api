// xpsGlobalSwitchControl.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsGlobalSwitchControl.h
 * \brief This file contains API prototypes and type definitions
 *        for the Global Sytem wide XPS System Manager
 *
 * For more detailed descriptions of these features and their implementations
 * please see the following:
 *
 * \see xpGlobalSwitchControlMgr.h
 */

#ifndef _xpsGlobalSwitchControl_h_
#define _xpsGlobalSwitchControl_h_

#include "xpEnums.h"
#include "xpsEnums.h"
#include "xpsInit.h"
#include "xpTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \public
 * \brief API that initializes the Global XPS System Mgr
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGlobalSwitchControlInit(void);

/**
 * \public
 * \brief API that initializes the Global XPS System Mgr
 *
 * \param [in] scopeId
 * \return XP_STATUS
 */
XP_STATUS xpsGlobalSwitchControlInitScope(xpsScope_t scopeId);

/**
 * \public
 * \brief API to De-Init the XPS System Mgr
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGlobalSwitchControlDeInit(void);

/**
 * \public
 * \brief API to De-Init the XPS System Mgr
 *
 * \param [in] scopeId
 * \return XP_STATUS
 */
XP_STATUS xpsGlobalSwitchControlDeInitScope(xpsScope_t scopeId);

/**
 * \public
 * \brief API to perform Add Device operations for System Mgr
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGlobalSwitchControlAddDevice(xpsDevice_t devId,
                                          xpsInitType_t initType);

/**
 * \public
 * \brief API to perform Remove Device operations for System Mgr
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGlobalSwitchControlRemoveDevice(xpsDevice_t devId);

/**
 * \public
 * \brief This API resets a XP device identified by its device ID.
 *
 * This API will invoke corresponding device reset and hardware should be reset
 * after successful completion of this API.
 *
 * \param [in] devId Device ID
 *
 * \return int
 */
XP_STATUS xpsGlobalSwitchControlChipReset(xpsDevice_t devId);

/**
 * \brief This API resets dyn modules of XP device identified by its device ID.
 *
 * This API will invoke corresponding device reset and hardware should be reset
 * after successful completion of this API.
 *
 * \param [in] devId Device ID
 * \param [in] resetList Array of field numbers
 * \param [in] numResets number of fields
 *
 * \return int
 */
XP_STATUS xpsGlobalSwitchControlChipIssueDynamicReset(xpsDevice_t devId,
                                                      xpSlaveResetId_e *resetList, uint32_t numResets);

/**
 * \brief This API resets cfg modules of XP device identified by its device ID.
 *
 * This API will invoke corresponding device reset and hardware should be reset
 * after successful completion of this API.
 *
 * \param [in] devId Device ID
 * \param [in] resetList Array of field numbers
 * \param [in] numResets number of fields
 *
 * \return int
 */
XP_STATUS xpsGlobalSwitchControlChipIssueConfigReset(xpsDevice_t devId,
                                                     xpSlaveResetId_e *resetList, uint32_t numResets);

/**
* Enable/Disable symmetric hashing on ETHERNET layer
* SOURCE_MAC ^ DESTINATION_MAC
*
* \param [in] deviceId device Id
* \param [in] enable   1 - enable, 0 - disable
*
* \return XP_STATUS XP_NO_ERR if there is no error
*/
XP_STATUS xpsGlobalSwitchControlSetMacSymmetricHashing(xpsDevice_t deviceId,
                                                       uint32_t enable);

/**
* Enable/Disable symmetric hashing on TCP layer
* SOURCE_PORT ^ DESTINATION_PORT
*
* \param [in] deviceId device Id
* \param [in] enable   1- enable, 0 - disable
*
* \return XP_STATUS XP_NO_ERR if there is no error
*/
XP_STATUS xpsGlobalSwitchControlSetTCPSymmetricHashing(xpsDevice_t deviceId,
                                                       uint32_t enable);

/**
* Enable/Disable symmetric hashing on UDP layer
* SOURCE_PORT ^ DESTINATION_PORT
*
* \param [in] deviceId device Id
* \param [in] enable   1- enable, 0 - disable
*
* \return XP_STATUS XP_NO_ERR if there is no error
*/
XP_STATUS xpsGlobalSwitchControlSetUDPSymmetricHashing(xpsDevice_t deviceId,
                                                       uint32_t enable);

/**
* Enable/Disable symmetric hashing on IPv4 layer
* SOURCE_IP ^ DESTINATION_IP
*
* \param [in] deviceId device Id
* \param [in] enable   1- enable, 0 - disable
*
* \return XP_STATUS XP_NO_ERR if there is no error
*/
XP_STATUS xpsGlobalSwitchControlSetIPv4SymmetricHashing(xpsDevice_t deviceId,
                                                        uint32_t enable);

/**
* Enables the VlanRangeCheck symmetrically across all channels.
* Applies to ETHERNET or PBB layertype only
*
* \param [in] deviceId device Id
* \param [in] enable   if set to 1, enable vlan range check
* \param [in] minVlan  minimum vlan
* \param [in] maxVlan  maximum vlan
*
* \return XP_NO_ERR if the register was set without errors
*/
XP_STATUS xpsGlobalSwitchControlSetVlanRangeCheck(xpDevice_t deviceId,
                                                  uint32_t enable, uint32_t minVlan, uint32_t maxVlan) ;

/**
 * \public
 * \brief Enables the Layer port range check for the given layer and rangeNum symmetrically across all channels
 *
 * \param [in] deviceId Device Id
 * \param [in] rangeNum range number
 * \param [in] layer layer (0 or 1)
 * \param [in] enable   if set to 1, enable Layer range check
 * \param [in] destPortCheck
 *                 if set to 1, check Layer.destinationPort else check Layer.sourcePort; between minPort and maxPort
 * \param [in] minPort  minimum Port against which Layer port needs to
 *                 be checked
 * \param [in] maxPort  maximum Port against which Layer port needs to be checked
 * \param [in] resultLocation which metaData bit will be set if port lies between the range
 *
 * \return XP_NO_ERR if the register was set without errors
 */
XP_STATUS xpsGlobalSwitchControlSetLayerRangeCheck(xpDevice_t deviceId,
                                                   uint32_t rangeNum, uint32_t layer, uint32_t enable, uint32_t destPortCheck,
                                                   uint32_t minPort, uint32_t maxPort, uint32_t resultLocation) ;

/**
 * \public
 * \brief Gets the Layer port range check information for the given rangeNum
 *
 * \param deviceId [in] Device Id
 * \param rangeNum [in] range number
 * \param layer1 [out] layer1 is enabled (0 or 1)
 * \param layer2 [out] layer2 is enabled (0 or 1)
 * \param destPortCheck [out]
 *                 if set to 1, Layer.destinationPort is checked else Layer.sourcePort is checked between minPort and maxPort
 * \param minPort [out]  minimum Port against which Layer port needs to
 *                 be checked
 * \param maxPort [out] maximum Port against which Layer port needs to be checked
 * \param resultLocation [out] which metaData bit will be set if port lies between the range
 *
 * \return XP_NO_ERR if the register was set without errors
 */
XP_STATUS xpsGlobalSwitchControlGetLayerRangeCheck(xpDevice_t deviceId,
                                                   uint32_t rangeNum, uint32_t* layer1, uint32_t* layer2, uint32_t* destPortCheck,
                                                   uint32_t* minPort, uint32_t* maxPort, uint32_t * resultLocation) ;

/**
 * \public
 * \brief Gets the hash field information for this deviceId and layer
 * \param [in] deviceId Device id
 * \param [in] layer
 * \param [out] hashFieldData for this layer
 * \param [out] numFields number of fields
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlGetHashFields(xpDevice_t deviceId,
                                              xpLayerType_t layer, uint32_t **hashFieldData, uint32_t* numFields);

/**
 * \public
 * \brief Set the hashable fields for a specific device
 *
 * \param [in] deviceId   Device Id of device.
 * \param [in] fields array of hashable fields
 * \param [in] size array fields sizes
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGlobalSwitchControlSetHashFields(xpsDevice_t deviceId,
                                              xpHashField* fields, size_t size);

/**
 * \public
 * \brief Set the hashable field params for a given field for a specific device
 *
 * \param [in] deviceId   Device Id
 * \param [in] fields     fields
 * \param [in] offset     offset
 * \param [in] length     length
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGlobalSwitchControlSetHashFieldParams(xpsDevice_t deviceId,
                                                   xpHashField field, uint32_t offset, uint32_t length);

/**
 * \public
 * \brief Set the hashable field params for a given field for a specific device
 *
 * \param [in] deviceId      Device Id
 * \param [in] fields        fields
 * \param [in] offset        offset
 * \param [in] length        length
 * \param [in] hashMaskMSB   maskMSB
 * \param [in] mask          mask
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGlobalSwitchControlSetHashFieldParamsWithMask(xpsDevice_t deviceId,
                                                           xpHashField field, uint32_t offset, uint32_t length, int32_t hashMaskMSB,
                                                           int32_t mask);

/**
 * \public
 * \brief enable/disable the hash layer xor for this deviceId
 *
 * \param [in] deviceId Device id
 * \param [in] layer
 * \param [in] xor field type
 * \param [in] layerEnable 1 for enable, 0 for disable
 * \param [in] fieldEnable 1 for enable, 0 for disable
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlSetHashLayerXor(xpDevice_t deviceId,
                                                xpLayerType_t layer, xpXorFieldType_t xorFieldType, uint32_t layerEnable,
                                                uint32_t fieldEnable);

/**
 * \public
 * \brief Gets if the hash layer xor for this deviceId is enabled/disabled
 *
 * \param [in] deviceId Device id
 * \param [in] layer
 * \param [in] xor field type
 * \param [out] layerEnable 1 for enable, 0 for disable
 * \param [out] fieldEnable 1 for enable, 0 for disable
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlGetHashLayerXor(xpDevice_t deviceId,
                                                xpLayerType_t layer, xpXorFieldType_t xorFieldType, uint32_t* layerEnable,
                                                uint32_t* fieldEnable);

/**
 * For all the packets that contain the given layer, de/selects the other layers that participate in hashA/B or both
 *
 * \param deviceId Device id
 * \param givenLayer given layer
 * \param otherLayers other layers for this layer
 * \param numOtherLayers number of other layers
 * \param hashOption hashA or hashB or both
 * \param enable enable (select) or disable (deselect)

 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlSelectHashingLayers(xpDevice_t deviceId,
                                                    xpLayerType_t givenLayer, xpLayerType_t* otherLayers, uint32_t numOtherLayers,
                                                    xpHashOption_e hashOption, uint8_t enable);

/**
 * \public
 * \brief Disables hashing on a particular layer for this device
 *
 * \param deviceId Device id
 * \param layer layer type
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlDisableLayerHash(xpDevice_t deviceId,
                                                 xpLayerType_t layer);

/**
 * \public
 * \brief sets the LAG hash polynomial for this device and instance
 *
 * \param deviceId   Device id
 * \param instance   which hash instance to select
 * \param polynomialId polynomial number
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlSetLagHashPolynomial(xpDevice_t deviceId,
                                                     xpHashOption_e instance, uint32_t polynomialId) ;

/**
 * \public
 * \brief gets the LAG hash polynomial for this device and
 *           instance
 *
 * \param [in] deviceId   Device id
 * \param [in] instance   which hash instance to select
 * \param [out] polynomialId polynomial number
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlGetLagHashPolynomial(xpDevice_t deviceId,
                                                     xpHashOption_e instance, uint32_t* polynomialId) ;

/**
 * \public
 * \brief sets the LAG hash polynomial seed for this device and
 * instance
 *
 * \param deviceId   Device id
 * \param instance   which hash instance to select
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlSetLagHashPolynomialSeed(xpDevice_t deviceId,
                                                         xpHashOption_e instance, uint32_t seed) ;

/**
 * \public
 * \brief gets the LAG hash polynomial seed for this device and
 * instance
 *
 * \param [in] deviceId   Device id
 * \param [in] instance   which hash instance to select
 * \param [out] seed
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlGetLagHashPolynomialSeed(xpDevice_t deviceId,
                                                         xpHashOption_e instance, uint32_t* seed) ;

/**
 * \public
 * \brief sets the L2Ecmp hash polynomial for this device and instance
 *
 * \param deviceId   Device id
 * \param instance   which hash instance to select
 * \param polynomialId polynomial number
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlSetL2EcmpHashPolynomial(xpDevice_t deviceId,
                                                        xpHashOption_e instance, uint32_t polynomialId) ;

/**
 * \public
 * \brief gets the L2Ecmp hash polynomial id for this device
 * and instance
 *
 * \param [in] deviceId   Device id
 * \param [in] instance   which L2Ecmp hash to select
 * \param [out] polynomialId
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlGetL2EcmpHashPolynomial(xpDevice_t deviceId,
                                                        xpHashOption_e instance, uint32_t* polynomialId) ;

/**
 * \public
 * \brief sets the L2Ecmp hash polynomial seed for this device and
 * instance
 *
 * \param [in] deviceId   Device id
 * \param [in] instance   which hash instance to select
 * \param [in] seed
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlSetL2EcmpHashPolynomialSeed(xpDevice_t deviceId,
                                                            xpHashOption_e instance, uint32_t seed) ;

/**
 * \public
 * \brief gets the L2Ecmp hash polynomial seed for this device
 * and instance
 *
 * \param [in] deviceId   Device id
 * \param [in] instance   which hash to select
 * \param [out] seed      seed for this instance
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlGetL2EcmpHashPolynomialSeed(xpDevice_t deviceId,
                                                            xpHashOption_e instance, uint32_t* seed) ;

/**
 * \public
 * \brief Sets the L3Ecmp hash polynomial for this device and instance
 *
 * \param deviceId   Device id
 * \param instance   which hash instance to select
 * \param polynomialId polynomial number
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlSetL3EcmpHashPolynomial(xpDevice_t deviceId,
                                                        xpHashOption_e instance, uint32_t polynomialId) ;

/**
 * \public
 * \brief gets the L3Ecmp hash polynomial id for this device
 *
 * \param [in] deviceId   Device id
 * \param [in] instance   which L3Ecmp hash to select
 * \param [out] polynomialId
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlGetL3EcmpHashPolynomial(xpDevice_t deviceId,
                                                        xpHashOption_e instance, uint32_t* polynomialId) ;
/**
 * \public
 * \brief sets the L3Ecmp hash polynomial seed for this device and
 * instance
 *
 * \param deviceId   Device id
 * \param instance   which hash instance to select
 * \param seed
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlSetL3EcmpHashPolynomialSeed(xpDevice_t deviceId,
                                                            xpHashOption_e instance, uint32_t seed) ;

/**
 * \public
 * \brief gets the L3Ecmp hash polynomial seed for this device
 * and instance
 *
 * \param [in] deviceId   Device id
 * \param [in] instance   which hash to select
 * \param [out] seed
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlGetL3EcmpHashPolynomialSeed(xpDevice_t deviceId,
                                                            xpHashOption_e instance, uint32_t* seed) ;

/**
* \public
* \brief Sets the LAG hash polynomial and shift for this device and instance
*
* \param deviceId   Device id
* \param instance   which LAG hash to select
* \param polynomialId polynomial number
* \param shift      shift the polynomial
*
* \return XP_STATUS XP_NO_ERR if there is no error
*/
XP_STATUS xpsGlobalSwitchControlSetLagHashPolynomialShift(xpDevice_t deviceId,
                                                          uint32_t instance, uint32_t polynomial, uint32_t shift);

/**
 * \public
 * \brief gets the LAG hash polynomial for this device and
 *           instance
 *
 * \param [in] deviceId   Device id
 * \param [in] instance   which LAG hash to select
 * \param [out] polynomialId polynomial number
 * \param [out] shift bits by which the polynomial will be shifted
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlGetLagHashPolynomialShift(xpDevice_t deviceId,
                                                          uint32_t instance, uint32_t* polynomialId, uint32_t* shift);

/**
 * \public
 * \brief the LAG hash output mask for this device and instance
 *
 * \param deviceId   Device id
 * \param instance   which LAG hash to select
 * \param outputMask output mask
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlSetLagHashOutputMask(xpDevice_t deviceId,
                                                     uint32_t instance, uint32_t outputMask);

/**
 * \public
 * \brief gets the LAG hash output mask for this device and instance
 *
 * \param [in] deviceId   Device id
 * \param [in] instance   which LAG hash to select
 * \param [out] outputMask output mask
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlGetLagHashOutputMask(xpDevice_t deviceId,
                                                     uint32_t instance, uint32_t* outputMask);

/**
 * \public
 * \brief sets the L2Ecmp hash polynomial and shift for this device and instance
 *
 * \param deviceId   Device id
 * \param instance   which L2Ecmp hash to select
 * \param polynomialId polynomial number
 * \param shift      shift the polynomial
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlSetL2EcmpHashPolynomialShift(
    xpDevice_t deviceId, uint32_t instance, uint32_t polynomialId, uint32_t shift);

/**
 * \public
 * \brief gets the L2Ecmp hash polynomial for this device and
 *           instance
 *
 * \param [in] deviceId   Device id
 * \param [in] instance   which L2Ecmp hash to select
 * \param [out] polynomialId polynomial number
 * \param [out] shift bits by which the polynomial will be shifted
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlGetL2EcmpHashPolynomialShift(
    xpDevice_t deviceId, uint32_t instance, uint32_t* polynomialId,
    uint32_t* shift);

/**
 * \public
 * \brief sets the L2Ecmp hash output mask for this device and instance
 *
 * \param deviceId   Device id
 * \param instance   which L2Ecmp hash to select
 * \param outputMask output mask
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlSetL2EcmpHashOutputMask(xpDevice_t deviceId,
                                                        uint32_t instance, uint32_t outputMask);

/**
 * \public
 * \brief gets the L2Ecmp hash output mask for this device and instance
 *
 * \param [in] deviceId   Device id
 * \param [in] instance   which L2Ecmp hash to select
 * \param [out] outputMask output mask
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlGetL2EcmpHashOutputMask(xpDevice_t deviceId,
                                                        uint32_t instance, uint32_t* outputMask);

/**
 * \public
 * \brief sets the L3Ecmp hash polynomial and shift for this device and instance
 *
 * \param deviceId   Device id
 * \param instance   which L3Ecmp hash to select
 * \param polynomialId polynomial number
 * \param shift      shift the polynomial
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlSetL3EcmpHashPolynomialShift(
    xpDevice_t deviceId, uint32_t instance, uint32_t polynomialId, uint32_t shift);

/**
 * \public
 * \brief gets the L3Ecmp hash polynomial for this device and
 *           instance
 *
 * \param [in] deviceId   Device id
 * \param [in] instance   which L3Ecmp hash to select
 * \param [out] polynomialId polynomial number
 * \param [out] shift bits by which the polynomial will be shifted
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlGetL3EcmpHashPolynomialShift(
    xpDevice_t deviceId, uint32_t instance, uint32_t* polynomialId,
    uint32_t* shift);

/**
 * \public
 * \brief sets the L3Ecmp hash output mask for this device and instance
 *
 * \param deviceId   Device id
 * \param instance   which L3Ecmp hash to select
 * \param outputMask output mask
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlSetL3EcmpHashOutputMask(xpDevice_t deviceId,
                                                        uint32_t instance, uint32_t outputMask);

/**
 * \public
 * \brief gets the L3Ecmp hash output mask for this device and instance
 *
 * \param [in] deviceId   Device id
 * \param [in] instance   which L3Ecmp hash to select
 * \param [out] outputMask output mask
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsGlobalSwitchControlGetL3EcmpHashOutputMask(xpDevice_t deviceId,
                                                        uint32_t instance, uint32_t* outputMask);

/**
 * \public
 * \brief sets the timestamp simple config for the txChn
 * \param [in] devId Device id
 * \param [in] portNum
 * \param [in] enable
 * \param [in] dir ingress or egress
 * \param [in] remove
 *
 * \return XP_NO_ERR if the operation is successful
 */
XP_STATUS xpsGlobalSwitchControlEnableTimeStamp(xpDevice_t devId,
                                                uint32_t portNum, bool enable, XP_DIR_E dir, bool remove);

/**
 * \public
 * \brief sets the timestamp replace config for the txChn
 * \param [in] devId Device id
 * \param [in] portNum
 * \param [in] enable
 *
 * \return XP_NO_ERR if the operation is successful
 */
XP_STATUS xpsGlobalSwitchControlEnableTimeStampReplace(xpDevice_t devId,
                                                       uint32_t portNum, bool enable);

/**
 * \public
 * \brief gets the timestamp simple config
 * \param [in]  devId Device id
 * \param [in]  portNum
 * \param [in]  dir ingress or egress
 * \param [out] enable
 * \param [out] replace
 * \param [out] remove
 *
 * \return XP_NO_ERR if the operation is successful
 */
XP_STATUS xpsGlobalSwitchControlGetTimeStampInfo(xpDevice_t devId,
                                                 uint32_t portNum, XP_DIR_E dir, uint32_t *enable, uint32_t *replace,
                                                 uint32_t *remove);


/**
 * \public
 * \brief Enables forwarding of error packets through the
 *        pipeline
 *
 * \param deviceId Device ID
 * \param enable if set to 1, enable error packet
 *        forwarding
 *
 * \return XP_NO_ERR if the register was set without errors
 */
XP_STATUS xpsGlobalSwitchControlEnableErrorPacketForwarding(xpDevice_t deviceId,
                                                            uint32_t enable);

/**
 * \public
 * \brief API to enable cut-thru processing on RX per port
 *
 * \param [in] deviceId
 * \param [in] portNum
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGlobalSwitchControlSetRxCutThruEnable(xpDevice_t deviceId,
                                                   uint32_t portNum, uint32_t enable);

/**
 * \public
 * \brief API to retrieve whether or not a port operates in
 *        cut-thru on RX
 *
 * \param [in] deviceId
 * \param [in] portNum
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGlobalSwitchControlGetRxCutThruEnable(xpDevice_t deviceId,
                                                   uint32_t portNum, uint32_t *enable);

/**
 * \public
 * \brief API to enable cut-thru processing on TX per port
 *
 * This API will instruct the Traffic Manager on a per port
 * Basis to honor to global cut-thru mode settings or operate in
 * Store and Forward on TX as configured by
 *
 * \see xpGlobalSwitchControlMgrSetTxCutThruMode
 *
 * \param [in] deviceId
 * \param [in] portNum
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGlobalSwitchControlSetTxCutThruEnable(xpDevice_t deviceId,
                                                   uint32_t portNum, uint32_t enable);

/**
 * \public
 * \brief API to get whether or not a port honors the global
 *        cut-thru mode setting on TX
 *
 * \see xpGlobalSwitchControlMgrSetTxCutThruMode
 *
 * \param [in] deviceId
 * \param [in] portNum
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGlobalSwitchControlGetTxCutThruEnable(xpDevice_t deviceId,
                                                   uint32_t portNum, uint32_t *enable);

/**
 * \public
 * \brief API to set the cut thru bypass mode
 *
 * This method is used to set cut thru bypass mode globally in
 * the traffic manager
 *
 * The traffic manager supports three modes of cut-thru:
 *  - Bypass AQM of all traffic
 *  - Bypass AQM of cut-thru marked traffic
 *  - Do not bypass aqm for all traffic
 *
 * When AQM is performed, the traffic manager will treat packets
 * as Store and Forward even if they were processed as cut-thru
 * on RX. This means prior to sending the packet to the TDMA,
 * the Traffic Manager will wait for the end-of-packet signal
 * for this token
 *
 * \param [in] deviceId
 * \param [in] bypassMode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGlobalSwitchControlSetTxCutThruMode(xpDevice_t deviceId,
                                                 xpBypassMode_e bypassMode);

/**
 * \public
 * \brief API to get the cut thru bypass mode
 *
 * This method is used to get the cut thru bypass mode globally
 * from the traffic manager
 *
 * The traffic manager supports three modes of cut-thru:
 *  - Bypass AQM of all traffic
 *  - Bypass AQM of cut-thru marked traffic
 *  - Do not bypass aqm for all traffic
 *
 * When AQM is performed, the traffic manager will treat packets
 * as Store and Forward even if they were processed as cut-thru
 * on RX. This means prior to sending the packet to the TDMA,
 * the Traffic Manager will wait for the end-of-packet signal
 * for this token
 *
 * \param [in] deviceId
 * \param [in] bypassMode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGlobalSwitchControlGetTxCutThruMode(xpDevice_t deviceId,
                                                 xpBypassMode_e *bypassMode);

/**
 * \brief To get system max ports.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [out] deviceMaxPorts max system ports
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsGlobalSwitchControlGetMaxPorts(xpDevice_t devId,
                                            uint8_t* deviceMaxPorts);

/**
 * \brief To get mcpu port number.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [out] mcpuPortNum MCPU port number
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsGlobalSwitchControlGetMcpuPortNumber(xpDevice_t devId,
                                                  uint32_t* mcpuPortNum);

/**
 * \brief To get mcpu sde port number.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [out] mcpuSdePortNum MCPU port number
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsGlobalSwitchControlGetMcpuSdePortNumber(xpDevice_t devId,
                                                     uint32_t* mcpuSdePortNum);

/**
 * \brief To get scpu port number.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [out] scpuPortNum SCPU port number
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsGlobalSwitchControlGetScpuPortNumber(xpDevice_t devId,
                                                  uint32_t* scpuPortNum);

/**
 * \brief To get scpu sde port number.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [out] scpuSdePortNum SCPU port number
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsGlobalSwitchControlGetScpuSdePortNumber(xpDevice_t devId,
                                                     uint32_t* scpuSdePortNum);

/**
 * \brief To get loopback0 port number.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [out] lpbk0PortNum Loopback0 port number
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsGlobalSwitchControlGetLoopback0PortNumber(xpDevice_t devId,
                                                       uint32_t* lpbk0PortNum);

/**
 * \brief To get loopback1 port number.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [out] lpbk1PortNum Loopback1 port number
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsGlobalSwitchControlGetLoopback1PortNumber(xpDevice_t devId,
                                                       uint32_t* lpbk1PortNum);

/**
 * \brief To get max channel of mac.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [out] macMaxChanNum mac max channel number
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsGlobalSwitchControlGetMacMaxChannelNumber(xpDevice_t devId,
                                                       uint8_t* macMaxChanNum);

/**
 * \brief To get max num of physical ports
 * \param [in] devId device id. Valid values are 0-63
 * \param [out] maxNumPhyPorts
 * \return [XP_STATUS] status On success XP_NO_ERR.
 */
XP_STATUS xpsGlobalSwitchControlGetMaxNumPhysicalPorts(xpDevice_t devId,
                                                       uint16_t* maxNumPhyPorts);
/* \public
* \brief API to set the cut through default packet size globally
*
* This method is used to set the pktBC register with the user provided value
* The default packet size is set for accounting the number of bytes in cut-through mode.
* The default packet size has a default value: 1500 and is user configurable now
*
* \param [in] deviceId   Device Id
* \param [in] pktSize    Packet Size in bytes
*
* \return XP_STATUS
*/

XP_STATUS xpsGlobalSwitchControlSetCutThruDefaultPktSize(xpDevice_t deviceId,
                                                         uint32_t pktSize);

/**
 * \public
 * \brief API to get the default packet size set in cut through mode
 *
 * \param [in] deviceId   Device Id
 * \param [out] pktSize   Packet Size in bytes
 *
 * \return XP_STATUS
 */

XP_STATUS xpsGlobalSwitchControlGetCutThruDefaultPktSize(xpDevice_t deviceId,
                                                         uint32_t *pktSize);

/**
 * \public
 * \brief API to get the CPU physical port number
 *
 * \param [in] deviceId   Device Id
 * \param [out] portNum   CPU physical port number
 *
 * \return XP_STATUS
 */

XP_STATUS xpsGlobalSwitchControlGetCpuPhysicalPortNum(xpDevice_t deviceId,
                                                      uint32_t *portNum);

#ifdef __cplusplus
}
#endif

#endif // _xpsGlobalSwitchControl_h_
