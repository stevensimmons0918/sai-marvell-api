// xpsGlobalSwitchControl.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsGlobalSwitchControl.h"
#include "xpsLock.h"
#include "xpsCommon.h"
#include "cpssHalPort.h"
#include "cpssHalDevice.h"


#ifdef __cplusplus
extern "C"
{
#endif

XP_STATUS xpsGlobalSwitchControlInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlAddDevice(xpsDevice_t devId,
                                          xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlChipReset(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlChipIssueDynamicReset(xpsDevice_t devId,
                                                      xpSlaveResetId_e *resetList, uint32_t numResets)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlChipIssueConfigReset(xpsDevice_t devId,
                                                     xpSlaveResetId_e *resetList, uint32_t numResets)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetMacSymmetricHashing(xpsDevice_t deviceId,
                                                       uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetTCPSymmetricHashing(xpsDevice_t deviceId,
                                                       uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetUDPSymmetricHashing(xpsDevice_t deviceId,
                                                       uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetIPv4SymmetricHashing(xpsDevice_t deviceId,
                                                        uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetVlanRangeCheck(xpDevice_t deviceId,
                                                  uint32_t enable, uint32_t minVlan, uint32_t maxVlan)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetLayerRangeCheck(xpDevice_t deviceId,
                                                   uint32_t rangeNum, uint32_t layer, uint32_t enable, uint32_t destPortCheck,
                                                   uint32_t minPort, uint32_t maxPort, uint32_t resultLocation)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetLayerRangeCheck(xpDevice_t deviceId,
                                                   uint32_t rangeNum, uint32_t* layer1, uint32_t* layer2, uint32_t* destPortCheck,
                                                   uint32_t* minPort, uint32_t* maxPort, uint32_t * resultLocation)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetHashFields(xpDevice_t deviceId,
                                              xpLayerType_t layer, uint32_t **hashFieldData, uint32_t* numFields)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetHashFields(xpsDevice_t deviceId,
                                              xpHashField* fields, size_t size)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetHashFieldParams(xpsDevice_t deviceId,
                                                   xpHashField field, uint32_t offset, uint32_t length)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetHashFieldParamsWithMask(xpsDevice_t deviceId,
                                                           xpHashField field, uint32_t offset, uint32_t length, int32_t hashMaskMSB,
                                                           int32_t mask)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetHashLayerXor(xpDevice_t deviceId,
                                                xpLayerType_t layer, xpXorFieldType_t xorFieldType, uint32_t layerEnable,
                                                uint32_t fieldEnable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetHashLayerXor(xpDevice_t deviceId,
                                                xpLayerType_t layer, xpXorFieldType_t xorFieldType, uint32_t* layerEnable,
                                                uint32_t* fieldEnable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSelectHashingLayers(xpDevice_t deviceId,
                                                    xpLayerType_t givenLayer, xpLayerType_t* otherLayers, uint32_t numOtherLayers,
                                                    xpHashOption_e hashOption, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlDisableLayerHash(xpDevice_t deviceId,
                                                 xpLayerType_t layer)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetLagHashPolynomial(xpDevice_t deviceId,
                                                     xpHashOption_e instance, uint32_t polynomialId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetLagHashPolynomial(xpDevice_t deviceId,
                                                     xpHashOption_e instance, uint32_t* polynomialId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetLagHashPolynomialSeed(xpDevice_t deviceId,
                                                         xpHashOption_e instance, uint32_t seed)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetLagHashPolynomialSeed(xpDevice_t deviceId,
                                                         xpHashOption_e instance, uint32_t* seed)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetL2EcmpHashPolynomial(xpDevice_t deviceId,
                                                        xpHashOption_e instance, uint32_t polynomialId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetL2EcmpHashPolynomial(xpDevice_t deviceId,
                                                        xpHashOption_e instance, uint32_t* polynomialId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetL2EcmpHashPolynomialSeed(xpDevice_t deviceId,
                                                            xpHashOption_e instance, uint32_t seed)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetL2EcmpHashPolynomialSeed(xpDevice_t deviceId,
                                                            xpHashOption_e instance, uint32_t* seed)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetL3EcmpHashPolynomial(xpDevice_t deviceId,
                                                        xpHashOption_e instance, uint32_t polynomialId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetL3EcmpHashPolynomial(xpDevice_t deviceId,
                                                        xpHashOption_e instance, uint32_t* polynomialId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetL3EcmpHashPolynomialSeed(xpDevice_t deviceId,
                                                            xpHashOption_e instance, uint32_t seed)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetL3EcmpHashPolynomialSeed(xpDevice_t deviceId,
                                                            xpHashOption_e instance, uint32_t* seed)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetLagHashPolynomialShift(xpDevice_t deviceId,
                                                          uint32_t instance, uint32_t polynomial, uint32_t shift)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetLagHashPolynomialShift(xpDevice_t deviceId,
                                                          uint32_t instance, uint32_t* polynomial, uint32_t* shift)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetLagHashOutputMask(xpDevice_t deviceId,
                                                     uint32_t instance, uint32_t outputMask)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetLagHashOutputMask(xpDevice_t deviceId,
                                                     uint32_t instance, uint32_t* outputMask)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetL2EcmpHashPolynomialShift(
    xpDevice_t deviceId, uint32_t instance, uint32_t polynomial, uint32_t shift)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetL2EcmpHashPolynomialShift(
    xpDevice_t deviceId, uint32_t instance, uint32_t* polynomial, uint32_t* shift)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetL2EcmpHashOutputMask(xpDevice_t deviceId,
                                                        uint32_t instance, uint32_t outputMask)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetL2EcmpHashOutputMask(xpDevice_t deviceId,
                                                        uint32_t instance, uint32_t* outputMask)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetL3EcmpHashPolynomialShift(
    xpDevice_t deviceId, uint32_t instance, uint32_t polynomial, uint32_t shift)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetL3EcmpHashPolynomialShift(
    xpDevice_t deviceId, uint32_t instance, uint32_t* polynomial, uint32_t* shift)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetL3EcmpHashOutputMask(xpDevice_t deviceId,
                                                        uint32_t instance, uint32_t outputMask)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetL3EcmpHashOutputMask(xpDevice_t deviceId,
                                                        uint32_t instance, uint32_t* outputMask)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlEnableTimeStamp(xpDevice_t devId,
                                                uint32_t portNum, bool enable, XP_DIR_E dir, bool remove)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlEnableTimeStampReplace(xpDevice_t devId,
                                                       uint32_t portNum, bool enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetTimeStampInfo(xpDevice_t devId,
                                                 uint32_t portNum, XP_DIR_E dir, uint32_t *enable, uint32_t *replace,
                                                 uint32_t *remove)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsGlobalSwitchControlEnableErrorPacketForwarding(xpDevice_t deviceId,
                                                            uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetRxCutThruEnable(xpDevice_t deviceId,
                                                   uint32_t portNum, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetRxCutThruEnable(xpDevice_t deviceId,
                                                   uint32_t portNum, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetTxCutThruEnable(xpDevice_t deviceId,
                                                   uint32_t portNum, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetTxCutThruEnable(xpDevice_t deviceId,
                                                   uint32_t portNum, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlSetTxCutThruMode(xpDevice_t deviceId,
                                                 xpBypassMode_e bypassMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetTxCutThruMode(xpDevice_t deviceId,
                                                 xpBypassMode_e *bypassMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetMaxPorts(xpDevice_t devId,
                                            uint8_t* deviceMaxPorts)
{

    int numGlobalPorts;

    cpssHalSetDeviceSwitchId(devId);
    cpssHalGetMaxGlobalPorts(&numGlobalPorts);
    *deviceMaxPorts = (uint8_t)numGlobalPorts;

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetMcpuPortNumber(xpDevice_t devId,
                                                  uint32_t* mcpuPortNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetMcpuSdePortNumber(xpDevice_t devId,
                                                     uint32_t* mcpuSdePortNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetScpuPortNumber(xpDevice_t devId,
                                                  uint32_t* scpuPortNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetScpuSdePortNumber(xpDevice_t devId,
                                                     uint32_t* scpuSdePortNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetLoopback0PortNumber(xpDevice_t devId,
                                                       uint32_t* lpbk0PortNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetLoopback1PortNumber(xpDevice_t devId,
                                                       uint32_t* lpbk1PortNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetMacMaxChannelNumber(xpDevice_t devId,
                                                       uint8_t* macMaxChanNum)
{
    return cpssHalPortGlobalSwitchControlGetMacMaxChannelNumber(devId,
                                                                macMaxChanNum);
}

XP_STATUS xpsGlobalSwitchControlGetMaxNumPhysicalPorts(xpDevice_t devId,
                                                       uint16_t* maxNumPhyPorts)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
XP_STATUS xpsGlobalSwitchControlSetCutThruDefaultPktSize(xpDevice_t deviceId,
                                                         uint32_t pktSize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetCutThruDefaultPktSize(xpDevice_t deviceId,
                                                         uint32_t *pktSize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGlobalSwitchControlGetCpuPhysicalPortNum(xpDevice_t deviceId,
                                                      uint32_t* portNum)
{
    XPS_LOCK(xpsGlobalSwitchControlGetCpuPhysicalPortNum);

    return cpssHalGlobalSwitchControlGetCpuPhysicalPortNum(deviceId, portNum);
}

#ifdef __cplusplus
}
#endif
