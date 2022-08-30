// xpsMac.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsMac.h"
#include "xpsPort.h"
#include "xpsInit.h"
#include "xpsLock.h"
#include "xpsUtil.h"
#include "xpsAcl.h"
#include "gtGenTypes.h"
#include "cpssHalMac.h"
#include "cpssHalPort.h"
#include "cpssHalPhy.h"
#include "cpssHalProfile.h"
#include "cpssHalConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpsMacSwitchMacConfigMode(xpsDevice_t devId, uint8_t macNum,
                                    xpMacConfigMode newMode, xpFecMode fecMode, uint8_t enableFEC)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSwitchMacConfigModeWithLinkStatusControl(xpsDevice_t devId,
                                                         uint8_t macNum, xpMacConfigMode newMode, xpFecMode fecMode, uint8_t enableFEC,
                                                         uint8_t keepPortDown)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetMacConfigModeDebug(xpsDevice_t devId, uint32_t portNum,
                                      xpMacConfigMode
                                      macConfig) //For debug purpose only, which will write the mac mode in HW.
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPortGroupInit(xpsDevice_t devId, uint8_t macNum,
                              xpMacConfigMode macConfig, xpSpeed speed, uint8_t initSerdes,
                              uint8_t prbsTestMode, uint8_t firmwareUpload, xpFecMode fecMode,
                              uint8_t enableFEC)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPortInit(xpDevice_t devId, uint32_t portNum,
                         xpMacConfigMode macConfigMode, xpFecMode fecMode, uint8_t enableFEC)
{
    return cpssHalMacPortInit(devId, portNum, macConfigMode, fecMode, enableFEC);
}

XP_STATUS xpsMacPortGroupInitWithLinkStatusControl(xpDevice_t devId,
                                                   uint8_t ptgNum, xpMacConfigMode macConfigMode, xpSpeed speed,
                                                   uint8_t initSerdes, uint8_t prbsTestMode, uint8_t firmwareUpload,
                                                   xpFecMode fecMode, uint8_t enableFEC, uint8_t keepPortDown)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPortInitWithLinkStatusControl(xpDevice_t devId,
                                              uint32_t portNum,
                                              xpMacConfigMode macConfigMode, uint8_t cpssIntfType,
                                              uint8_t initSerdes, uint8_t prbsTestMode,
                                              uint8_t firmwareUpload, xpFecMode fecMode, uint8_t enableFEC,
                                              uint8_t keepPortDown, uint8_t initPort)
{
    XPS_LOCK(xpsMacPortInitWithLinkStatusControl);
    return cpssHalMacPortInitWithLinkStatusControl(devId, portNum, macConfigMode,
                                                   (CPSS_PORT_INTERFACE_MODE_ENT)cpssIntfType,
                                                   (bool)initSerdes, (bool)prbsTestMode, (bool)firmwareUpload, fecMode, enableFEC,
                                                   keepPortDown, initPort);
}

XP_STATUS xpsMacPortGroupDeInit(xpsDevice_t devId, uint8_t macNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIsPortInited(xpsDevice_t devId, uint32_t portNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetMacNumForPortNum(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t *macNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacConfigModeGet(xpDevice_t devId, uint32_t portNum,
                              xpMacConfigMode *macConfigMode)
{
    XPS_LOCK(xpsMacConfigModeGet);

    return cpssHalMacConfigModeGet(devId, portNum, macConfigMode);
}

XP_STATUS xpsMacIsPortNumValid(xpsDevice_t devId, uint32_t portNum,
                               uint8_t *valid)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPortDeInit(xpsDevice_t devId, uint32_t portNum)
{
    XPS_FUNC_ENTRY_LOG();

    cpssHalMacPortDeInit(devId, portNum);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacEventHandler(xpsDevice_t devId, uint8_t macNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPortLoopbackEnable(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t enable)
{

    XPS_LOCK(xpsMacPortLoopbackEnable);

    return cpssHalSerdesLoopback(devId, portNum, enable);

}

XP_STATUS xpsMacPortLoopbackEnableGet(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *status)
{


    XPS_LOCK(xpsMacPortLoopbackEnableGet);

    return cpssHalSerdesLoopbackGet(devId, portNum, status);

}

XP_STATUS xpsMacPortFarEndLoopbackEnable(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPortSerdesTune(xpsDevice_t devId, xpsPort_t *portList,
                               uint32_t numOfPort, xpSerdesDfeTuneMode_t tuneMode, uint8_t force)
{
    XPS_LOCK(xpsMacPortSerdesTune);

    if (portList == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    return cpssHalMacPortSerdesTune(devId, portList, numOfPort,  tuneMode, force);
}

XP_STATUS xpsMacPortSerdesTxTune(xpsDevice_t devId, xpsPort_t *portList,
                                 uint32_t numOfPort, uint8_t force)
{
    XPS_LOCK(xpsMacPortSerdesTxTune);

    if (portList == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }

    return cpssHalMacPortSerdesTxTune(devId, portList, numOfPort, force);
}

XP_STATUS xpsMacPortSerdesDfeWait(xpsDevice_t devId, xpsPort_t portNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPortSerdesDfeRunning(xpsDevice_t devId, xpsPort_t portNum,
                                     uint8_t *isDfeRunning)
{
    XPS_LOCK(xpsMacPortSerdesDfeRunning);
    return  cpssHalMacPortSerdesIsDfeRunning(devId, portNum, isDfeRunning);

}

XP_STATUS xpsMacPortSerdesGetSignalOk(xpsDevice_t devId, xpsPort_t portNum,
                                      uint8_t *signalOk)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRxCrcCheckEnable(xpsDevice_t devId, uint32_t portNum,
                                 uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRxStripFcsEnable(xpsDevice_t devId, uint32_t portNum,
                                 uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRxFlowControlDecodeEnable(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetRxPreambleLen(xpsDevice_t devId, uint32_t portNum,
                                 xpRxPreambleLenBytes length)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetRxJabberFrmSize(xpsDevice_t devId, uint32_t portNum,
                                   uint16_t jabberFrmSize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetTxPriorityPauseVector(xpsDevice_t devId, uint32_t portNum,
                                         uint16_t txPriPauseVector)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPauseFrameGen(xpsDevice_t devId, uint32_t portNum,
                                 uint8_t *status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetPauseFrameGen(xpsDevice_t devId, uint32_t portNum,
                                 uint8_t status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetMacFlowCtrlFrmDestAddr(xpsDevice_t devId, uint32_t portNum,
                                          macAddr_t macAddr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetMacFlowCtrlFrmSrcAddr(xpsDevice_t devId, uint32_t portNum,
                                         macAddr_t macAddr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetXonPauseTime(xpsDevice_t devId, uint32_t portNum,
                                uint16_t pauseTime)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetXoffPauseTime(xpsDevice_t devId, uint32_t portNum,
                                 uint16_t pauseTime)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxTimeStampValue(xpsDevice_t devId, uint32_t portNum,
                                    uint64_t *timeStampVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTimeStampValidStatus(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t *valid)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTimeStampId(xpsDevice_t devId, uint32_t portNum,
                               uint8_t *timeStampId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetFlowCtrlPauseTime(xpsDevice_t devId, uint32_t portNum,
                                     uint16_t pauseTime)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetRxMaxFrmLen(xpsDevice_t devId, uint32_t portNum,
                               uint16_t frameSize)
{

    XPS_LOCK(xpsMacSetRxMaxFrmLen);

    return cpssHalPortSetMru(devId, portNum, frameSize);
}

XP_STATUS xpsMacSetTxIfgLen(xpsDevice_t devId, uint32_t portNum,
                            uint8_t ifgLength)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetTxJabberFrmSize(xpsDevice_t devId, uint32_t portNum,
                                   uint16_t jabberFrmSize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacTxIfgCtrlPerFrameEnable(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacTxCrcCheckEnable(xpsDevice_t devId, uint32_t portNum,
                                 uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacTxFcsInvertEnable(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacTxFlowControlEnable(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacTxPriorityFlowControlEnable(xpsDevice_t devId, uint32_t portNum,
                                            uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetTxPreambleLen(xpsDevice_t devId, uint32_t portNum,
                                 xpPreambleLenBytes bytes)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPortSoftReset(xpsDevice_t devId, uint32_t portNum,
                              uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRxtxPortEnable(xpsDevice_t devId, uint32_t portNum,
                               uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacTxPortEnable(xpsDevice_t devId, uint32_t portNum,
                             uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRxPortEnable(xpsDevice_t devId, uint32_t portNum,
                             uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacStatCounterReset(xpsDevice_t devId, uint32_t portNum)
{
    XPS_LOCK(xpsMacStatCounterReset);

    return cpssHalMacStatCounterReset(devId, portNum);
}

XP_STATUS xpsMacStatCounterCreateAclRuleForV4V6(xpsDevice_t devId,
                                                uint32_t portNum,
                                                uint32_t *counterId_Ingress_v4, uint32_t *counterId_Ingress_v6,
                                                uint32_t *counterId_Egress_v4,  uint32_t *counterId_Egress_v6)
{
    XPS_LOCK(xpsMacStatCounterCreateAclRuleForV4V6);

    XP_STATUS status = XP_NO_ERR;

    /*Create ingress v4 counter*/
    status = cpssHalMacStatCounterCreateAclRuleForV4V6(devId, portNum,
                                                       counterId_Ingress_v4,
                                                       XPS_PCL_STAGE_INGRESS, XPS_PCL_IS_IPV4);
    if (XP_NO_ERR != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to create acl rule of ingress v4 counter for port: %d \n", portNum);
        return status;
    }

    /*Create ingress v6 counter*/
    status = cpssHalMacStatCounterCreateAclRuleForV4V6(devId, portNum,
                                                       counterId_Ingress_v6,
                                                       XPS_PCL_STAGE_INGRESS, XPS_PCL_IS_IPV6);
    if (XP_NO_ERR != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to create acl rule of ingress v6 counter for port: %d \n", portNum);
        return status;
    }

    /*Create egress v4 counter*/
    status = cpssHalMacStatCounterCreateAclRuleForV4V6(devId, portNum,
                                                       counterId_Egress_v4,
                                                       XPS_PCL_STAGE_EGRESS, XPS_PCL_IS_IPV4);
    if (XP_NO_ERR != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to create acl rule of egress v4 for port: %d \n", portNum);
        return status;
    }

    /*Create egress v6 counter*/
    status = cpssHalMacStatCounterCreateAclRuleForV4V6(devId, portNum,
                                                       counterId_Egress_v6,
                                                       XPS_PCL_STAGE_EGRESS, XPS_PCL_IS_IPV6);
    if (XP_NO_ERR != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to create acl rule of egress v6 for port: %d \n", portNum);
        return status;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsMacStatCounterRemoveAclRuleForV4V6(xpsDevice_t devId,
                                                uint32_t portNum,
                                                uint32_t counterId_Ingress_v4, uint32_t counterId_Ingress_v6,
                                                uint32_t counterId_Egress_v4,  uint32_t counterId_Egress_v6)
{
    XPS_LOCK(xpsMacStatCounterRemoveAclRuleForV4V6);

    XP_STATUS status = XP_NO_ERR;

    /*Remove ingress v4 counter*/
    status = cpssHalMacStatCounterRemoveAclRuleForV4V6(devId, portNum,
                                                       counterId_Ingress_v4,
                                                       XPS_PCL_STAGE_INGRESS, XPS_PCL_IS_IPV4);
    if (XP_NO_ERR != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to remove acl rule of ingress v4 counter for port: %d \n", portNum);
        return status;
    }

    /*Remove ingress v6 counter*/
    status = cpssHalMacStatCounterRemoveAclRuleForV4V6(devId, portNum,
                                                       counterId_Ingress_v6,
                                                       XPS_PCL_STAGE_INGRESS, XPS_PCL_IS_IPV6);
    if (XP_NO_ERR != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to remove acl rule ingress v6 counter for port: %d \n", portNum);
        return status;
    }

    /*Remove egress v4 counter*/
    status = cpssHalMacStatCounterRemoveAclRuleForV4V6(devId, portNum,
                                                       counterId_Egress_v4,
                                                       XPS_PCL_STAGE_EGRESS, XPS_PCL_IS_IPV4);
    if (XP_NO_ERR != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to remove acl rule egress v4 for port: %d \n", portNum);
        return status;
    }

    /*Remove egress v6 counter*/
    status = cpssHalMacStatCounterRemoveAclRuleForV4V6(devId, portNum,
                                                       counterId_Egress_v6,
                                                       XPS_PCL_STAGE_EGRESS, XPS_PCL_IS_IPV6);
    if (XP_NO_ERR != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to remove acl rule egress v6 for port: %d \n", portNum);
        return status;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetCounterV4V6Stats(xpsDevice_t devId, uint32_t portNum,
                                    uint32_t counterId_Ingress_v4, uint32_t counterId_Ingress_v6,
                                    uint32_t counterId_Egress_v4, uint32_t counterId_Egress_v6,
                                    uint64_t *v4TxPkts,  uint64_t *v4TxBytes,  uint64_t *v4RxPkts,
                                    uint64_t *v4RxBytes,
                                    uint64_t *v6TxPkts,  uint64_t *v6TxBytes,  uint64_t *v6RxPkts,
                                    uint64_t *v6RxBytes)
{
    XPS_LOCK(xpsMacGetCounterV4V6Stats);
    return cpssHalMacGetCounterV4V6Stats(devId, portNum,
                                         counterId_Ingress_v4, counterId_Ingress_v6,
                                         counterId_Egress_v4, counterId_Egress_v6,
                                         v4TxPkts, v4TxBytes, v4RxPkts, v4RxBytes,
                                         v6TxPkts, v6TxBytes, v6RxPkts, v6RxBytes);
}

XP_STATUS xpsMacPortEnable(xpsDevice_t devId, uint32_t portNum, uint8_t enable)
{
    XPS_LOCK(xpsMacPortEnable);

    return cpssHalMacPortEnable(devId, portNum, enable);
}

XP_STATUS xpsMacPortEnableRange(xpsDevice_t devId, uint8_t fromPortNum,
                                uint8_t toPortNum, xpPortEnable enable)
{
    for (; fromPortNum <= toPortNum; fromPortNum++)
    {
        cpssHalMacPortEnable(devId, fromPortNum, enable);
    }
    return XP_NO_ERR;
}

XP_STATUS xpsMacPortReset(xpsDevice_t devId, uint8_t enable, uint32_t portNum,
                          xpResetPortLevel reset)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacMacLoopback(xpsDevice_t devId, uint32_t portNum,
                            xpMacLoopbackLevel loopback, uint8_t enable)
{

    XPS_LOCK(xpsMacMacLoopback);
    return cpssHalMacLoopback(devId, portNum, loopback, enable);

}

XP_STATUS xpsMacMacLoopbackGet(xpsDevice_t devId, uint32_t portNum,
                               xpMacLoopbackLevel loopback,  uint8_t *status)
{

    XPS_LOCK(xpsMacMacLoopbackGet);
    return cpssHalMacLoopbackGet(devId, portNum, loopback, status);
}

XP_STATUS xpsMacGmiiLoopbackEnableGet(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSgmiiSerdesLoopbackEnableGet(xpsDevice_t devId,
                                             uint32_t portNum,
                                             uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacConfigMdioMaster(xpsDevice_t devId, uint32_t portNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacDeconfigMdioMaster(xpsDevice_t devId, uint32_t portNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacExtPHYRead(xpsDevice_t devId, uint32_t portNum, uint8_t phyAddr,
                           uint8_t phyRegAddr, uint16_t *value)
{

    XPS_LOCK(xpsMacExtPHYRead);

    return cpssHalReadPhyReg(devId, portNum, phyAddr, phyRegAddr,
                             value); //phyAddr as "pageNum"
}

XP_STATUS xpsMacExtPHYWrite(xpsDevice_t devId, uint32_t portNum,
                            uint8_t phyAddr,
                            uint8_t phyRegAddr, uint16_t value)
{

    XPS_LOCK(xpsMacExtPHYWrite);

    return cpssHalWritePhyReg(devId, portNum, phyAddr, phyRegAddr,
                              value); //phyAddr as "pageNum"
}

XP_STATUS xpsMacGetPortSpeed(xpsDevice_t devId, uint32_t portNum,
                             xpSpeed *speed)
{
    XPS_LOCK(xpsMacGetPortSpeed);

    return cpssHalMacGetPortSpeed(devId, portNum, speed);
}

XP_STATUS xpsMacGetPortOperSpeed(xpsDevice_t devId, uint32_t portNum,
                                 xpSpeed *speed)
{
    XPS_LOCK(xpsMacGetPortOperSpeed);

    return cpssHalGetPortOperSpeed(devId, portNum, speed);
}


XP_STATUS xpsMacGetPortDefaultSpeed(xpsDevice_t devId, uint32_t portNum,
                                    xpSpeed *speed)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetPortAutoNeg(xpsDevice_t devId, uint32_t portNum,
                               uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPortAutoNegRestart(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t restart)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPortAutoNegEnable(xpsDevice_t devId, uint32_t portNum,
                                     uint8_t *status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPortAutoNegDone(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t *status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPortAutoNegSyncStatus(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t *status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacIsMdioMasterConf(xpsDevice_t devId, uint32_t portNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetMdioClkDivisonCtrl(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t divCtrlRatio)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacEventHandlerDeRegister(xpsDevice_t devId, uint32_t portNum,
                                       xpEventType eventType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacEventHandlerRegister(xpsDevice_t devId, uint32_t portNum,
                                     xpEventType eventType, xpEventHandler eventHandler)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacInterruptEnable(xpsDevice_t devId, uint32_t portNum,
                                xpEventType eventType, bool enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetCounterStats(xpsDevice_t devId, uint32_t portNum,
                                uint8_t fromStatNum, uint8_t toStatNum, xp_Statistics *stat)
{


    XPS_LOCK(xpsMacGetCounterStats);

    return cpssHalMacGetCounterStats(devId, portNum, stat);

}


XP_STATUS xpsMacGetCounterStatsClearOnRead(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t fromStatNum, uint8_t toStatNum, xp_Statistics *stat)
{
    XP_STATUS ret = XP_NO_ERR;

    XPS_LOCK(xpsMacGetCounterStatsClearOnRead);

    if ((ret = xpsMacSetReadCounterClear(devId, portNum, 1)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Set read counter clear failed");
        return ret;
    }

    if ((ret = xpsMacSetReadCounterPriority(devId, portNum,
                                            0)) != XP_NO_ERR)   //Couldn't find the matching function for this in cpss

    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Set read counter prioriy failed");
        return ret;
    }

    if ((ret = cpssHalMacGetCounterStats(devId, portNum, stat)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get counter stats failed");
        return ret;
    }

    if ((ret = xpsMacSetReadCounterClear(devId, portNum, 0)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Set read counter clear failed");
        return ret;
    }

    return ret;
}

XP_STATUS xpsMacGetCounterStatsDirect(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t fromStatNum, uint8_t toStatNum, xp_Statistics *stat)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacCounterStatsRxPkts(xpsDevice_t devId, uint32_t portNum,
                                   uint64_t *rxTotalPkts)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacCounterStatsTxPkts(xpsDevice_t devId, uint32_t portNum,
                                   uint64_t *txTotalPkts)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacCounterStatsRxOctets(xpsDevice_t devId, uint32_t portNum,
                                     uint64_t *rxTotalOctets)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacCounterStatsTxOctets(xpsDevice_t devId, uint32_t portNum,
                                     uint64_t *txTotalOctets)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsFaultStatus(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t *status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPcsDoubleSpeedEnable(xpsDevice_t devId, uint32_t portNum,
                                     uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacEncoding10gEnable(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPcsInterruptMaskEnable(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsInterruptMaskEnable(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPcsInterruptTestEnable(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetPcsTestPatternSeedA(xpsDevice_t devId, uint32_t portNum,
                                       uint64_t seed)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetPcsTestPatternSeedB(xpsDevice_t devId, uint32_t portNum,
                                       uint64_t seed)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPcsDataPatternSelect(xpsDevice_t devId, uint32_t portNum,
                                     xpPCSDataPattern dataPattern)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPcsTestPatternSelect(xpsDevice_t devId, uint32_t portNum,
                                     xpPCSTestPattern testPattern)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetPcsRxTxTestModeEnable(xpsDevice_t devId, uint32_t portNum,
                                         xpPCSEnableTestMode testMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetPcsScrambledIdealTestPattern(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsLockStatus(xpsDevice_t devId, uint32_t portNum,
                                 uint8_t *pcsLock)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsHiBerStatus(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t *pcsHiBer)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsBlockLockStatus(xpsDevice_t devId, uint32_t portNum,
                                      uint32_t *blockLock)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsAlignmentLock(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t *status, uint32_t *alignmentLock)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsDebugDeskewOverFlow(xpsDevice_t devId, uint32_t portNum,
                                          uint32_t *deskewOverFlowErr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsDebugTxGearboxFifoErr(xpsDevice_t devId, uint32_t portNum,
                                            uint8_t* fifoErr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsLaneMappingStatus(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t laneNum, uint8_t *laneMapping)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRxPreambleLen(xpsDevice_t devId, uint32_t portNum,
                                 xpRxPreambleLenBytes *length)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsFullThreshold(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t *maxEntries)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsTxTestModeEnable(xpsDevice_t devId, uint32_t portNum,
                                       xpPCSEnableTestMode *testMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsRxTestModeEnable(xpsDevice_t devId, uint32_t portNum,
                                       xpPCSEnableTestMode *testMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsInterruptTestEnable(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsTestPatternSeedA(xpsDevice_t devId, uint32_t portNum,
                                       uint64_t *seed)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsTestPatternSeedB(xpsDevice_t devId, uint32_t portNum,
                                       uint64_t *seed)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsMacGetEncoding10gEnable(xpsDevice_t devId, uint32_t portNum,
                                     uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsMacPcsEnable(xpsDevice_t devId, uint32_t portNum, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPcsBypassScramblerEnable(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPcsUseShortTimerEnable(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPcsIgnoreSignalOkEnable(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPcsBypassTxBufEnable(xpsDevice_t devId, uint32_t portNum,
                                     uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetPcsFullThreshold(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t maxEntries)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPcsLowLatencyModeEnable(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsLowLatencyModeEnable(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacFecEnable(xpsDevice_t devId, uint32_t portNum, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFcFecAbility(xpsDevice_t devId, uint32_t portNum,
                                uint8_t *status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFcFecErrIndicationAbility(xpsDevice_t devId,
                                             uint32_t portNum,
                                             uint8_t *status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacFcFecEnable(xpsDevice_t devId, uint32_t portNum, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacFcFecErrToPcsEnable(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFcFecCorrectedBlockCnt(xpsDevice_t devId, uint32_t portNum,
                                          uint32_t *value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFcFecUncorrectedBlockCnt(xpsDevice_t devId, uint32_t portNum,
                                            uint32_t *value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacFcFecSoftReset(xpsDevice_t devId, uint32_t portNum,
                               uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFcFecSoftReset(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRsFecBypassEnable(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecEnaErrorIndication(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t *error)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecAbility(xpsDevice_t devId, uint32_t portNum,
                                uint8_t *status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecBypassCorrAbility(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t *status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecErrIndicationAbi(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t *error)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecCorrectedBlockCnt(xpsDevice_t devId, uint32_t portNum,
                                          uint32_t *value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecUncorrectedBlockCnt(xpsDevice_t devId, uint32_t portNum,
                                            uint32_t *value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecSymbolErrCnt(xpsDevice_t devId, uint32_t portNum,
                                     uint8_t laneNum, uint32_t *value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRsFecSoftReset(xpsDevice_t devId, uint32_t portNum,
                               uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetBackPlaneAbility(xpsDevice_t devId, uint32_t portNum,
                                    xpBackPlaneAbilityModes backPlaneAbilityMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetBackPlaneAbility(xpsDevice_t devId, uint32_t portNum,
                                    xpBackPlaneAbilityModes *backPlaneAbilityMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetResetAllChannel(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetResetAllChannel(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetTxFifoThreshold4ch(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t thresold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxFifoThreshold4ch(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *thresold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetTxFifoThreshold2ch(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t thresold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxFifoThreshold2ch(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *thresold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetTxFifoThreshold1ch(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t thresold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxFifoThreshold1ch(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *thresold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetTxInterfaceType(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxInterfaceType(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetRxInterfaceType(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRxInterfaceType(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetAnRxSelector(xpsDevice_t devId, uint32_t portNum,
                                uint8_t *rxSelector)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetAnRxNonce(xpsDevice_t devId, uint32_t portNum,
                             uint8_t *rxNonce)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxActive(xpsDevice_t devId, uint32_t portNum,
                            uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxError(xpsDevice_t devId, uint32_t portNum, uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxUnderrun(xpsDevice_t devId, uint32_t portNum,
                              uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxPause(xpsDevice_t devId, uint32_t portNum, uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRxActive(xpsDevice_t devId, uint32_t portNum,
                            uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRxCrcError(xpsDevice_t devId, uint32_t portNum,
                              uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRxError(xpsDevice_t devId, uint32_t portNum, uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRxOverflow(xpsDevice_t devId, uint32_t portNum,
                              uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRxPause(xpsDevice_t devId, uint32_t portNum, uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetTxFifoOverFlowErrorInt(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxFifoOverFlowErrorInt(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetTxFifoOverFlowErrorIntEnable(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxFifoOverFlowErrorIntEnable(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFecErrorBlockInt(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFecUncorrectedBlockInt(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFecBlockLockInt(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFecBlockLockLostInt(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFecBlockLockChangedInt(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetFecErrorBlockIntEnable(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFecErrorBlockIntEnable(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetFecUncorrectedBlockIntEnable(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFecUncorrectedBlockIntEnable(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetFecBlockLockIntEnable(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFecBlockLockIntEnable(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetFecBlockLockLostIntEnable(xpsDevice_t devId,
                                             uint32_t portNum,
                                             uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFecBlockLockLostIntEnable(xpsDevice_t devId,
                                             uint32_t portNum,
                                             uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetFecBlockLockChangedIntEnable(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFecBlockLockChangedIntEnable(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetFecErrorBlockIntTest(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetFecUncorrectedBlockIntTest(xpsDevice_t devId,
                                              uint32_t portNum, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetFecBlockLockIntTest(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetFecBlockLockLostIntTest(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetFecBlockLockChangedIntTest(xpsDevice_t devId,
                                              uint32_t portNum, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetSgmiiIntEnable(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetSgmiiIntEnable(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetSgmiiIntTest(xpsDevice_t devId, uint32_t portNum,
                                uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetSgmiiIntTest(xpsDevice_t devId, uint32_t portNum,
                                uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetReadCounterClear(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t enable)
{

    XPS_LOCK(xpsMacSetReadCounterClear);
    return cpssHalMacSetReadCounterClear(devId, portNum, enable);

}

XP_STATUS xpsMacGetReadCounterClear(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetReadCounterPriority(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetReadCounterPriority(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetTxDrainMode(xpsDevice_t devId, uint32_t portNum,
                               uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxDrainMode(xpsDevice_t devId, uint32_t portNum,
                               uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetRxEarlyEofDetection(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRxEarlyEofDetection(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetRsFecDebugShortAMP(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecDebugShortAMP(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetRsFecDebugTestInit(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecDebugTestInit(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetRsFecCWTestStopASM(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecCWTestStopASM(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetBpanSerdesTxSel(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetBpanSerdesTxSel(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetBpanMode(xpsDevice_t devId, uint32_t portNum, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetBpanMode(xpsDevice_t devId, uint32_t portNum,
                            uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetBpanRxGearboxReSync(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetBpanRxGearboxReSync(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecInterruptStatus(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecAmcLock(xpsDevice_t devId, uint32_t portNum,
                                uint8_t *amcLock)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecLane0InternalMap(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t *lanMap)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecLane1InternalMap(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t *lanMap)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecLane2InternalMap(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t *lanMap)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecLane3InternalMap(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t *lanMap)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetLinkStatus(xpsDevice_t devId, uint32_t portNum,
                              uint8_t *status)
{
    XPS_LOCK(xpsMacGetLinkStatus);
    return cpssHalMacGetLinkStatus(devId, portNum, status);
}

void xpsMacPrintAllPortStatus(xpsDevice_t devId)
{

    return cpssHalMacPrintAllPortStatus(devId);

}

XP_STATUS xpsMacGetPortStatus(xpsDevice_t devId, uint32_t portNum,
                              uint8_t *linkStatus, uint8_t *serdesStatus, uint8_t *faultStatus)
{
    XPS_LOCK(xpsMacGetPortStatus);

    return cpssHalMacGetPortStatus(devId, portNum, (bool*)linkStatus,
                                   (bool*)faultStatus, (bool*)serdesStatus);
}

XP_STATUS xpsMacSetLinkStatus(xpsDevice_t devId, uint32_t portNum,
                              uint8_t status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacLinkStatusInterruptEnableSet(xpDevice_t devId, uint32_t portNum,
                                             uint8_t status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacLinkStatusInterruptEnableGet(xpDevice_t devId, uint32_t portNum,
                                             uint8_t *status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRxCrcCheckEnable(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRxStripFcsEnable(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRxFlowControlDecodeEnable(xpsDevice_t devId,
                                             uint32_t portNum,
                                             uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRxMaxFrmLen(xpsDevice_t devId, uint32_t portNum,
                               uint16_t *frameSize)
{

    XPS_LOCK(xpsMacGetRxMaxFrmLen);
    return cpssHalPortGetMru(devId, portNum, frameSize);
}
XP_STATUS xpsMacGetTxPriorityPauseVector(xpsDevice_t devId, uint32_t portNum,
                                         uint16_t *txPriPauseVector)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxIfgLen(xpsDevice_t devId, uint32_t portNum,
                            uint8_t *ifgLength)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxJabberFrmSize(xpsDevice_t devId, uint32_t portNum,
                                   uint16_t *jabberFrmSize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRxJabberFrmSize(xpsDevice_t devId, uint32_t portNum,
                                   uint16_t *jabberFrmSize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxIfgCtrlPerFrameEnable(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFlowCtrlPauseTime(xpsDevice_t devId, uint32_t portNum,
                                     uint16_t *pauseTime)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetXoffPauseTime(xpsDevice_t devId, uint32_t portNum,
                                 uint16_t *pauseTime)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetXonPauseTime(xpsDevice_t devId, uint32_t portNum,
                                uint16_t *pauseTime)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetTimeStampId(xpsDevice_t devId, uint32_t portNum,
                               uint8_t timeStampId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxFcsInvertEnable(xpsDevice_t devId, uint32_t portNum,
                                     uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxFlowControlEnable(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxPriorityFlowControlEnable(xpsDevice_t devId,
                                               uint32_t portNum, uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxCrcCheckEnable(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxPreambleLen(xpsDevice_t devId, uint32_t portNum,
                                 xpPreambleLenBytes *bytes)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRxtxPortEnable(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetTxPortEnable(xpsDevice_t devId, uint32_t portNum,
                                uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRxPortEnable(xpsDevice_t devId, uint32_t portNum,
                                uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPortSoftReset(xpsDevice_t devId, uint32_t portNum,
                                 uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPortReset(xpsDevice_t devId, uint8_t *enable,
                             uint32_t portNum, xpResetPortLevel reset)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetMdioClkDivisonCtrl(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *divCtrlRatio)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsTestPatternSelect(xpsDevice_t devId, uint32_t portNum,
                                        xpPCSTestPattern *testPattern)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsScrambledIdealTestPattern(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFcFecEnable(xpsDevice_t devId, uint32_t portNum,
                               uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetFcFecErrToPcsEnable(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecBypassEnable(xpsDevice_t devId, uint32_t portNum,
                                     uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacFecModeSet(xpDevice_t devId, uint32_t portNum,
                           xpFecMode fecMode)
{
    XPS_LOCK(xpsMacFecModeSet);

    return cpssHalMacFecModeSet(devId, portNum, fecMode);
}

XP_STATUS xpsMacFecModeGet(xpDevice_t devId, uint32_t portNum,
                           xpFecMode *fecMode)
{
    XPS_LOCK(xpsMacFecModeGet);

    return cpssHalMacFecModeGet(devId, portNum, fecMode);
}

XP_STATUS xpsMacGetPcsUseShortTimerEnable(xpDevice_t devId, uint32_t portNum,
                                          uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsEnable(xpDevice_t devId, uint32_t portNum,
                             uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsDoubleSpeedEnable(xpDevice_t devId, uint32_t portNum,
                                        uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsBypassTxBufEnable(xpDevice_t devId, uint32_t portNum,
                                        uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsBypassScramblerEnable(xpDevice_t devId, uint32_t portNum,
                                            uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsIgnoreSignalOkEnable(xpDevice_t devId, uint32_t portNum,
                                           uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPcsDataPatternSelect(xpDevice_t devId, uint32_t portNum,
                                        xpPCSDataPattern *dataPattern)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetRsFecEnaErrorIndication(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t error)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetRsFecSoftReset(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetMacFlowCtrlFrmSrcAddr(xpsDevice_t devId, uint32_t portNum,
                                         macAddr_t macAddr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetMacFlowCtrlFrmDestAddr(xpsDevice_t devId, uint32_t portNum,
                                          macAddr_t macAddr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacXpMacWrite(xpsDevice_t devId, uint32_t portNum,
                           uint32_t regAddr,
                           uint16_t regValue)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacXpMacRead(xpsDevice_t devId, uint32_t portNum, uint32_t regAddr,
                          uint16_t *regValue)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRegisterPlatformSerdesInitFunc(platformSerdesInitFunc func)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRegisterGetChassisPortInfoFunc(getChassisPortInfoFunc func)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacGetPortInfo(xpDevice_t devId, uint32_t port, uint8_t *ptgNum,
                            char *chassisPortName, bool *serdesStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRxFlowControlFilterEnable(xpDevice_t devId, uint32_t portNum,
                                          uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRxFlowControlFilterEnableGet(xpDevice_t devId, uint32_t portNum,
                                             uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPcsOperationalStatusGet(xpDevice_t devId, uint32_t portNum,
                                        uint8_t *pcsOperation)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacDisableMacInterrupts(xpDevice_t devId, uint8_t macNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacDisableMacInterruptsForChannel(xpDevice_t devId, uint8_t macNum,
                                               uint8_t channelNum, xpMacConfigMode macConfigMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacTxFaultGenSet(xpDevice_t devId, uint32_t portNum,
                              xpMacTxFaultGen faultType, uint16_t value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacTxFaultGenGet(xpDevice_t devId, uint32_t portNum,
                              xpMacTxFaultGen faultType, uint16_t *value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPcsDecodeTrapGet(xpDevice_t devId, uint32_t portNum,
                                 xpDecodeTrap *trap)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPcsDecodeTrapSet(xpDevice_t devId, uint32_t portNum,
                                 xpDecodeTrap trap)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPortSerdesSignalOverride(xpDevice_t devId, uint32_t portNum,
                                         uint8_t serdesSignalVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacLinkStatusOverride(xpDevice_t devId, uint32_t portNum,
                                   uint8_t value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacFaultStatusOverride(xpDevice_t devId, uint32_t portNum,
                                    uint8_t value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacBackPlaneAutoNegProcessSet(xpDevice_t devId, uint32_t portNum,
                                           uint16_t ANability, uint16_t ANctrl, int is25GModeIEEE)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacpcsRErrBlckAndPcsBerCntGet(xpDevice_t devId, uint32_t portNum,
                                           uint32_t *pcsRErrBlckCnt, uint32_t *pcsBerCnt)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacBackPlaneAutoNegSendConsortiumNextPage(xpDevice_t devId,
                                                       uint32_t portNum, uint16_t ANability, uint16_t ANctrl)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacBackPlaneAutoNegConfigureNullNextPage(xpDevice_t devId,
                                                      uint32_t portNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacBackPlaneAutoNegIsDone(xpDevice_t devId, uint32_t portNum,
                                       uint16_t *AN_status, uint16_t *AN_next_page)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacBackPlaneAutoNegProcessReset(xpDevice_t devId, uint32_t portNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsMacBackPlaneAutoNegStatusGet(xpDevice_t devId, uint32_t portNum,
                                          uint16_t *AN_status, uint16_t *ANability, uint16_t *remoteANability,
                                          uint16_t *ANctrl, uint16_t *remoteANctrl)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

int32_t xpsMacSerdesAacsServer(xpDevice_t devId, uint8_t serdesId, int tcpPort)
{
    XPS_FUNC_ENTRY_LOG();
    int32_t temp = 0;
    XPS_FUNC_EXIT_LOG();
    return temp;

}

XP_STATUS xpsMacSerdesLinkTrainingProcessSet(xpDevice_t devId, uint32_t portNum,
                                             xpMacConfigMode macConfigMode, bool fixedSpeed_an, uint8_t LT_timeoutDis)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSerdesLinkTrainingStatusGetLoop(xpDevice_t devId,
                                                uint32_t portNum, xpMacConfigMode macConfigMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSerdesLinkTrainingClear(xpDevice_t devId, uint32_t portNum,
                                        xpMacConfigMode macConfigMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacBackPlaneAutoNegSendConsortiumExtNextPage(xpDevice_t devId,
                                                          uint32_t portNum, uint16_t ANability, uint16_t ANctrl)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacBackPlaneAutoNegSuccessAction(xpDevice_t devId,
                                              uint32_t portNum,
                                              uint16_t ANability, uint16_t RemoteANability, uint16_t ANctrl,
                                              uint16_t RemoteANctrl, uint8_t dfeTuneEn, uint8_t serdesInit, uint8_t ltEnable,
                                              uint8_t ltTimeoutDis)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPortSerdesTuneConditionGet(xpsDevice_t devId, xpsPort_t portNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacPrintPortStat(xpDevice_t devId, uint32_t portNum,
                              xp_Statistics *stat)
{

    return cpssHalMacPrintPortStat(devId, portNum, stat);

}

XP_STATUS xpsMacSgmiiAutoNegSet(xpsDevice_t devId, uint32_t portNum,
                                uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSgmiiAutoNegGet(xpsDevice_t devId, uint32_t portNum,
                                uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSgmiiAutoNegRestartSet(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSgmiiAutoNegRestartGet(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSgmiiDisparityErrCntGet(xpDevice_t devId, uint32_t portNum,
                                        uint16_t *count)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSgmiiInvalidCodeCntGet(xpDevice_t devId, uint32_t portNum,
                                       uint16_t *count)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacCustomAlignmentMarkerLenSet(xpsDevice_t devId, uint32_t portNum,
                                            uint32_t value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacCustomAlignmentMarkerLenGet(xpsDevice_t devId, uint32_t portNum,
                                            uint32_t *value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRsFecProgAmpLockLenSet(xpsDevice_t devId, uint32_t portNum,
                                       uint32_t value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRsFecProgAmpLockLenGet(xpsDevice_t devId, uint32_t portNum,
                                       uint32_t *value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRsFecProgCwAmpLenSet(xpsDevice_t devId, uint32_t portNum,
                                     uint16_t value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRsFecProgCwAmpLenGet(xpsDevice_t devId, uint32_t portNum,
                                     uint16_t *value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
XP_STATUS xpsMacSerdesRxLaneRemapSet(xpDevice_t devId, uint32_t portNum,
                                     uint8_t laneNum, uint8_t serdesLaneNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSerdesRxLaneRemapGet(xpDevice_t devId, uint32_t portNum,
                                     uint8_t laneNum, uint8_t *serdesLaneNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSerdesTxLaneRemapSet(xpDevice_t devId, uint32_t portNum,
                                     uint8_t laneNum, uint8_t serdesLaneNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSerdesTxLaneRemapGet(xpDevice_t devId, uint32_t portNum,
                                     uint8_t laneNum, uint8_t *serdesLaneNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRsFecAlwaysUseClause49EnableSet(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRsFecNeverUseClause49EnableSet(xpsDevice_t devId,
                                               uint32_t portNum, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRsFecPcsScramblerEnableSet(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRsFecMlgScramblerEnableSet(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRsFecAlwaysUseClause49EnableGet(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRsFecNeverUseClause49EnableGet(xpsDevice_t devId,
                                               uint32_t portNum, uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRsFecPcsScramblerEnableGet(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacRsFecMlgScramblerEnableGet(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacAppFifoPortMapSet(xpDevice_t devId, uint32_t portNum,
                                  uint8_t value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacAppFifoPortMapGet(xpDevice_t devId, uint32_t portNum,
                                  uint8_t *value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacLinkStatusOverrideGet(xpDevice_t devId, uint32_t portNum,
                                      uint8_t *status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacFaultStatusOverrideGet(xpDevice_t devId, uint32_t portNum,
                                       uint8_t *status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacTv80RunFirmware(xpDevice_t devId, const char *z80FwPath)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacIsPortStatsAvailable(xpDevice_t devId, uint32_t portNum,
                                     uint8_t *isAvailable)
{
    XPS_FUNC_ENTRY_LOG();

    *isAvailable = 1;
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSerdesReInit(xpsDevice_t devId, uint32_t portNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMacSetPortAnEnable(xpsDevice_t devId, uint32_t portNum, int enable)
{
    if (portNum > MAX_PORTNUM)
    {
        return XP_ERR_OUT_OF_RANGE;
    }
    cpssHalPortMacAnEnable(devId, portNum, enable);
    return XP_NO_ERR;

}

int xpsMacGetPortAnEnable(xpsDevice_t devId, uint32_t portNum)
{

    return cpssHalPortMacAnEnableGet(devId, portNum);
}

XP_STATUS xpsMacGetPortFdAbility(xpDevice_t devId, uint32_t portNum,
                                 uint32_t *portAdvertFdAbility)
{

    return cpssHalMacGetPortFdAbility(devId, portNum, portAdvertFdAbility);

}
XP_STATUS xpsMacPortANLtRemoteAbilityGet(xpDevice_t devId, uint32_t port,
                                         uint16_t *portRemoteANAbility, uint16_t *portRemoteANCtrl)
{

    return cpssHalMacPortANLtRemoteAbilityGet(devId, port, portRemoteANAbility,
                                              portRemoteANCtrl);
}

XP_STATUS xpsMacSetPortCableLen(xpsDevice_t devId, uint32_t portNum,
                                int cableType)
{
    if (portNum > MAX_PORTNUM)
    {
        return XP_ERR_OUT_OF_RANGE;
    }
    cpssHalPortMacCableLenSet(devId, portNum, cableType);
    return XP_NO_ERR;

}

XP_STATUS xpsMacSetPlatformConfigFileName(char *fileName)
{
    if (fileName==NULL)
    {
        return XP_ERR_NULL_POINTER;
    }
    else
    {
        cpssHalPlatformConfigFileNameSet(fileName);
    }
    return XP_NO_ERR;

}

#ifdef __cplusplus
}
#endif
