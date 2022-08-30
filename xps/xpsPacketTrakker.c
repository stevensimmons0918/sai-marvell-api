// xpsPacketTrakker.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsPacketTrakker.h"
#include "xpsInterface.h"
#include "xpsPort.h"
#include "xpsInit.h"
#include "xpsLock.h"

#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpsPacketTrakkerInit()
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerDeInit()
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerReadMaxQueueDepth(xpsDevice_t deviceId,
                                            xpsPort_t port, uint32_t queue, uint32_t *depth)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerEnableDeflection(xpsDevice_t deviceId, xpsPort_t port,
                                           uint32_t queue, xpsPort_t newPort, uint32_t newQueue)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerDisableDeflection(xpsDevice_t deviceId, xpPort_t port,
                                            uint32_t queue)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerConfigureCookieSession(xpsDevice_t deviceId,
                                                 xpsPort_t port, uint32_t queue, uint16_t signature, uint32_t *sessionId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerEnableCookieSession(xpsDevice_t deviceId,
                                              uint32_t sessionId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerDisableCookieSession(xpsDevice_t deviceId,
                                               uint32_t sessionId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerTotalCirculatedCookies(xpsDevice_t deviceId,
                                                 uint32_t sessionId, uint32_t *cookieCnt)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerClearCirculatedCookieCount(xpsDevice_t deviceId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerReadDelay(xpsDevice_t deviceId, uint32_t sessionId,
                                    uint64_t *delay)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerReadEvent(xpsDevice_t deviceId,
                                    xpsPacketTrakkerEvent *event)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetEventAlmostFullThreshold(xpsDevice_t deviceId,
                                                      uint32_t *threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerSetEventAlmostFullThreshold(xpsDevice_t deviceId,
                                                      uint32_t threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerSetQueueWmThreshold(xpDevice_t devId,
                                              uint32_t profileId, uint32_t lowWm, uint32_t highWm, uint32_t maxThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetQueueWmThreshold(xpDevice_t devId,
                                              uint32_t profileId, uint32_t* lowWm, uint32_t* highWm, uint32_t* maxThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerBindPacketTrakkerProfileToQueue(xpDevice_t devId,
                                                          xpPort_t port, uint32_t queue, uint32_t profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetPacketTrakkerProfileForQueue(xpDevice_t devId,
                                                          xpPort_t port, uint32_t queue, uint32_t* profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerEnableQueueLengthTracking(xpDevice_t devId,
                                                    uint32_t profileId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerEnablePacketTrakker(xpDevice_t devId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetPacketTrakkerEnable(xpDevice_t devId,
                                                 uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerEnableGlobalStateMirrorMode(xpDevice_t devId,
                                                      xpPtGlobalState_e state, xpPtMirrorModes_e mirrorMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetGlobalStateMirrorMode(xpDevice_t devId,
                                                   xpPtGlobalState_e state, xpPtMirrorModes_e* mirrorMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerEnablePortStateMirrorMode(xpDevice_t devId,
                                                    uint32_t portNum, xpPtPortState_e state, xpPtMirrorModes_e mirrorMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetPortStateMirrorMode(xpDevice_t devId,
                                                 uint32_t portNum, xpPtPortState_e state, xpPtMirrorModes_e* mirrorMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerEnablePeriodicMirror(xpDevice_t devId,
                                               uint32_t portNum, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetPeriodicMirrorEnable(xpDevice_t devId,
                                                  uint32_t portNum, uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerConfigureMirrorPeriod(xpDevice_t devId,
                                                uint32_t portNum, uint32_t mirrorPeriod)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetMirrorPeriod(xpDevice_t devId, uint32_t portNum,
                                          uint32_t* mirrorPeriod)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerEnableGlobalShaper(xpDevice_t devId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetGlobalShaperEnable(xpDevice_t devId,
                                                uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerEnablePortShaper(xpDevice_t devId, uint32_t portNum,
                                           uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetPortShaperEnable(xpDevice_t devId,
                                              uint32_t portNum, uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerConfigureGlobalShaper(xpDevice_t devId,
                                                uint32_t rateKpps, uint32_t maxBucketSize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetGlobalShaperConfiguration(xpDevice_t devId,
                                                       uint32_t* rateKpps, uint32_t* maxBucketSize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerConfigurePortShaper(xpDevice_t devId,
                                              uint32_t portNum, uint32_t rateKpps, uint32_t maxBucketSize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetPortShaperConfiguration(xpDevice_t devId,
                                                     uint32_t portNum, uint32_t* rateKpps, uint32_t* maxBucketSize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerEnableGlobalAging(xpDevice_t devId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetGlobalAgingEnable(xpDevice_t devId,
                                               uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerConfigureGlobalAgingPeriod(xpDevice_t devId,
                                                     uint32_t agingPeriod, uint32_t agingLimit)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetGlobalAgingPeriod(xpDevice_t devId,
                                               uint32_t* agingPeriod, uint32_t* agingLimit)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerSetPortMaxThreshold(xpDevice_t devId,
                                              uint32_t portNum, uint32_t threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetPortMaxThreshold(xpDevice_t devId,
                                              uint32_t portNum, uint32_t* threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerSetPoolWmThreshold(xpDevice_t devId,
                                             uint32_t sharedPoolId, uint32_t lowWmThreshold, uint32_t highWmThreshold,
                                             uint32_t maxThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetPoolWmThreshold(xpDevice_t devId,
                                             uint32_t sharedPoolId, uint32_t* lowWmThreshold, uint32_t* highWmThreshold,
                                             uint32_t* maxThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerSetGlobalPageThreshold(xpsDevice_t devId,
                                                 uint32_t threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetGlobalPageThreshold(xpsDevice_t devId,
                                                 uint32_t *threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerSetMulticastPageThreshold(xpsDevice_t devId,
                                                    uint32_t threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetMulticastPageThreshold(xpsDevice_t devId,
                                                    uint32_t *threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerCreateAqmQProfile(xpsDevice_t devId,
                                            uint32_t *profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerBindAqmQProfileToQueue(xpsDevice_t devId,
                                                 xpsPort_t portNum, uint32_t queueNum, uint32_t profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetAqmQProfileForQueue(xpsDevice_t devId,
                                                 xpsPort_t portNum, uint32_t queueNum, uint32_t *profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerSetAvgQueueWmThreshold(xpsDevice_t devId,
                                                 uint32_t profileId, uint32_t minThreshold, uint32_t maxThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetAvgQueueWmThreshold(xpsDevice_t devId,
                                                 uint32_t profileId, uint32_t *minThreshold, uint32_t *maxThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerUpdatePktTrakkerStartQueueNum(xpDevice_t devId,
                                                        uint32_t portNum, uint32_t queueNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetPktTrakkerStartQueueNum(xpDevice_t devId,
                                                     uint32_t portNum, uint32_t* queueNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerAddAnalyzerInterface(xpsDevice_t devId,
                                               xpsInterfaceId_t analyzerIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerRemoveAnalyzerInterface(xpsDevice_t devId,
                                                  xpsInterfaceId_t analyzerIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerSetSwitchIdentifier(xpsDevice_t devId,
                                              uint32_t switchId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketTrakkerGetSwitchIdentifier(xpsDevice_t devId,
                                              uint32_t *switchId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
