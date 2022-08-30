// xpsQos.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsQos.h"

#include "xpsInit.h"
#include "xpsState.h"
#include "xpsLock.h"
#include "xpsCommon.h"
#include "xpsGlobalSwitchControl.h"

#include "cpssDxChNetIf.h"
#include "cpssHalUtil.h"
#include "openXpsCopp.h"
#include "cpssHalQos.h"
#include "cpssHalSys.h"
#include "cpssHalDevice.h"

#ifdef __cplusplus
extern "C"
{
#endif
extern void cpssHalDumpUtlRateLimiterDump(uint32_t devId,
                                          CPSS_DXCH_TABLE_ENT tableType, uint32_t startIndex, uint32_t endIndex);
#define XPS_MAX_AQM_PROFILES 128
/*
 * Local Enum and Structure defintions
 */

/**
 * \typedef xpsQosAqmProfileInfo_t
 * \brief AQM Profile info stored per device
 */
typedef struct xpsQosAqmProfileInfo_t
{
    /**
     * Device Id is used as the key since the information in this struct is stored per device
     */
    xpsDevice_t deviceId;
    /**
     * aqmProfilesUsedMask[i] = 1 => profile used and 0 => empty
     */
    uint8_t aqmProfilesUsedMask[XPS_MAX_AQM_PROFILES];

    /**
     * portProfilesUsedMask[i] = 1 => profile used and 0 => empty
     */
    uint8_t portProfilesUsedMask[XPS_MAX_AQM_PROFILES];
} xpsQosAqmProfileInfo_t;

/**
 * \var qosDbHandle
 * \public
 * \brief Qos Manager State Database Handle
 *
 */
// per device to store @xpsQosAqmProfileInfo_t

/**
 * \fn qosAqmDevKeyCompare
 * \brief qos Aqm Device Key Compare function
 *
 * This API is used by the state manager as a method to compare
 * keys for xpsQosAqmProfileInfo_t
 *
 * \param [in] void* key1
 * \param [in] void* key2
 *
 * \return int32_t
 */


// helper function to retrieve profile information for this device






/*
 * API Implementation
 */

XP_STATUS xpsQosInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
AQM Profile Manager
*/

XP_STATUS xpsQosAqmCreateAqmQProfile(xpsDevice_t devId, uint32_t *profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmDeleteAqmQProfile(xpsDevice_t devId, uint32_t profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmCreateAqmProfile(xpsDevice_t devId, uint32_t *profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmDeleteAqmProfile(xpsDevice_t devId, uint32_t profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * Enable/Disable the enqueue of a packet
 */
XP_STATUS xpsQosAqmSetEnqueueEnable(xpDevice_t devId, xpPort_t devPort,
                                    uint32_t queueNum, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetEnqueueEnable(xpDevice_t devId, xpPort_t devPort,
                                    uint32_t queueNum, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * Bind profiles to port and queue
 */
XP_STATUS xpsQosAqmBindAqmQProfileToQueue(xpsDevice_t devId, xpsPort_t devPort,
                                          uint32_t queueNum, uint32_t profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmBindAqmQProfileToColorAwareQueue(xpsDevice_t devId,
                                                    xpsPort_t devPort, uint32_t queueNum, xpQosColor_e color, uint32_t profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmBindAqmProfileToPort(xpsDevice_t devId, xpsPort_t devPort,
                                        uint32_t profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * Get profiles for port and queue
 */

XP_STATUS xpsQosAqmGetAqmQProfileForQueue(xpsDevice_t devId, xpsPort_t devPort,
                                          uint32_t queueNum, uint32_t *profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetAqmQProfileForColorAwareQueue(xpsDevice_t devId,
                                                    xpsPort_t devPort, uint32_t queueNum, xpQosColor_e color, uint32_t *profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetAqmProfileForPort(xpsDevice_t devId, xpsPort_t devPort,
                                        uint32_t queueNum, uint32_t *profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * WRED APIs
 */

XP_STATUS xpsQosAqmConfigureWred(xpsDevice_t devId, uint32_t profileId,
                                 uint32_t minThreshold, uint32_t maxThreshold, uint32_t weight,
                                 uint32_t maxDropProb)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetWredConfiguration(xpsDevice_t devId, uint32_t profileId,
                                        uint32_t *minThreshold, uint32_t *maxThreshold, uint32_t *weight,
                                        uint32_t *maxDropProb)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmConfigureWredMode(xpsDevice_t devId, uint32_t profileId,
                                     xpWredMode_e wredMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetWredMode(xpsDevice_t devId, uint32_t profileId,
                               xpWredMode_e *wredMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsRateLimiterDisplayTable(xpsDevice_t devId,
                                     uint32_t *numOfValidEntries, uint32_t startIndex, uint32_t endIndex,
                                     char * logFile, uint32_t detailFormat, uint32_t silentMode,
                                     uint32_t tblecpyIndx)

{
    XPS_FUNC_ENTRY_LOG();

    cpssHalDumpUtlRateLimiterDump(devId,
                                  CPSS_DXCH_SIP5_TABLE_EQ_TO_CPU_RATE_LIMITER_CONFIG_E, startIndex, endIndex);

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmDisplayWredParams(xpsDevice_t devId, uint32_t profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * DCTCP APIs
 */

XP_STATUS xpsQosAqmConfigureQueueDctcpMarkThreshold(xpsDevice_t devId,
                                                    uint32_t profileId, uint32_t markThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetQueueDctcpMarkThreshold(xpsDevice_t devId,
                                              uint32_t profileId, uint32_t *markThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmSetQueueDctcpEnable(xpsDevice_t devId, uint32_t profileId,
                                       uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetQueueDctcpEnable(xpsDevice_t devId, uint32_t profileId,
                                       uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmDisplayQueueDctcpParams(xpsDevice_t devId,
                                           uint32_t profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmConfigurePortDctcpMarkThreshold(xpsDevice_t devId,
                                                   uint32_t profileId, uint32_t markThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetPortDctcpMarkThreshold(xpsDevice_t devId,
                                             uint32_t profileId, uint32_t *markThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmSetPortDctcpEnable(xpsDevice_t devId, uint32_t profileId,
                                      uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetPortDctcpEnable(xpsDevice_t devId, uint32_t profileId,
                                      uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmDisplayPortDctcpParams(xpsDevice_t devId, uint32_t profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * Tail Drop APIs
 */

XP_STATUS xpsQosAqmConfigureQueuePageTailDropThresholdForProfile(
    xpsDevice_t devId, uint32_t profileId, uint32_t lengthMaxThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmConfigureQueuePageTailDropThreshold(xpsDevice_t devId,
                                                       xpsPort_t devPort, uint32_t queueNum, uint32_t lengthMaxThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmConfigureQueuePageTailDropThresholdForColor(
    xpsDevice_t devId, xpsPort_t devPort, uint32_t queueNum,
    xpQosColor_e color, uint32_t lengthMaxThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetQueuePageTailDropThresholdForProfile(xpsDevice_t devId,
                                                           uint32_t profileId, uint32_t *lengthMaxThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetQueuePageTailDropThreshold(xpsDevice_t devId,
                                                 xpsPort_t devPort, uint32_t queueNum, uint32_t* lengthMaxThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetQueuePageTailDropThresholdForColor(xpsDevice_t devId,
                                                         xpsPort_t devPort, uint32_t queueNum, xpQosColor_e color,
                                                         uint32_t* lengthMaxThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmUseProfileBasedTailDropThreshold(xpsDevice_t devId,
                                                    uint32_t profileId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetProfileBasedTailDropThresholdEnable(xpsDevice_t devId,
                                                          uint32_t profileId, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmDisplayQueueTailDropParams(xpsDevice_t devId,
                                              uint32_t profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmConfigurePortPageTailDropThreshold(xpsDevice_t devId,
                                                      uint32_t profileId, uint32_t lengthMaxThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetPortPageTailDropThreshold(xpsDevice_t devId,
                                                uint32_t profileId, uint32_t *lengthMaxThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmDisplayPortTailDropParams(xpsDevice_t devId,
                                             uint32_t profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * Phantom Q APIs
 */

XP_STATUS xpsQosAqmSetPhantomQEnable(xpsDevice_t devId, uint32_t profileId,
                                     uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetPhantomQEnable(xpsDevice_t devId, uint32_t profileId,
                                     uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmDisplayPhantomQParams(xpsDevice_t devId, uint32_t profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * Dynamic Pool and Thresholding APIs
 */

XP_STATUS xpsQosAqmSetQueueDynamicPoolEnable(xpsDevice_t devId,
                                             xpPort_t devPort, uint32_t queueNum, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetQueueDynamicPoolEnable(xpsDevice_t devId,
                                             xpPort_t devPort, uint32_t queueNum, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmBindQueueToDynamicPool(xpsDevice_t devId, xpPort_t devPort,
                                          uint32_t queueNum, uint32_t dynPoolId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetQueueToDynamicPoolBinding(xpsDevice_t devId,
                                                xpPort_t devPort, uint32_t queueNum, uint32_t *dynPoolId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmSetPortDynamicPoolEnable(xpsDevice_t devId, xpPort_t devPort,
                                            uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetPortDynamicPoolEnable(xpsDevice_t devId, xpPort_t devPort,
                                            uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmBindPortToDynamicPool(xpsDevice_t devId, xpPort_t devPort,
                                         uint32_t dynPoolId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetPortToDynamicPoolBinding(xpsDevice_t devId,
                                               xpPort_t devPort, uint32_t* dynPoolId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmSetPortdynPoolId(xpsDevice_t devId, xpPort_t devPort,
                                    uint32_t dynPoolId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetPortdynPoolId(xpsDevice_t devId, xpPort_t devPort,
                                    uint32_t *dynPoolId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmSetDynamicPoolThreshold(xpsDevice_t devId,
                                           uint32_t dynPoolId, uint32_t threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetDynamicPoolThreshold(xpsDevice_t devId,
                                           uint32_t dynPoolId, uint32_t *threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmSetDynamicPoolTotalThreshold(xpsDevice_t devId,
                                                uint32_t dynPoolId, uint32_t threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetDynamicPoolTotalThreshold(xpsDevice_t devId,
                                                uint32_t dynPoolId, uint32_t* threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmConfigureDynamicPoolAlpha(xpsDevice_t devId,
                                             uint32_t dynPoolId, xpDynThldFraction_e fraction, xpDynThldCoeff_e coefficient)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetDynamicPoolAlpha(xpsDevice_t devId, uint32_t dynPoolId,
                                       xpDynThldFraction_e *fraction, xpDynThldCoeff_e *coefficient)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmConfigureDynamicPoolAlphaForColor(xpsDevice_t devId,
                                                     uint32_t dynPoolId, xpQosColor_e color, xpDynThldFraction_e fraction,
                                                     xpDynThldCoeff_e coefficient)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetDynamicPoolAlphaForColor(xpsDevice_t devId,
                                               uint32_t dynPoolId, xpQosColor_e color, xpDynThldFraction_e *fraction,
                                               xpDynThldCoeff_e *coefficient)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetDynamicPoolCurrentSharedUsed(xpDevice_t devId,
                                                   uint32_t dynPoolId, uint32_t *counter)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetDynamicPoolCurrentTotalUsed(xpDevice_t devId,
                                                  uint32_t dynPoolId, uint32_t *counter)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * Global Thresholds
 */

XP_STATUS xpsQosAqmSetGlobalPacketThreshold(xpsDevice_t devId,
                                            uint32_t threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetGlobalPacketThreshold(xpsDevice_t devId,
                                            uint32_t *threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmSetGlobalPageThreshold(xpsDevice_t devId, uint32_t threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetGlobalPageThreshold(xpsDevice_t devId,
                                          uint32_t *threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * Global Multicast Thresholds
 */

XP_STATUS xpsQosAqmSetMulticastPacketThreshold(xpsDevice_t devId,
                                               uint32_t threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetMulticastPacketThreshold(xpsDevice_t devId,
                                               uint32_t *threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmSetMulticastPageThreshold(xpsDevice_t devId,
                                             uint32_t threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetMulticastPageThreshold(xpsDevice_t devId,
                                             uint32_t *threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmSetGlobalEcnThresholdEnable(xpDevice_t devId,
                                               uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetGlobalEcnThresholdEnable(xpDevice_t devId,
                                               uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmSetGlobalEcnThreshold(xpDevice_t devId, uint32_t threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetGlobalEcnThreshold(xpDevice_t devId, uint32_t* threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * Per Port/Queue Thresholds
 */

XP_STATUS xpsQosAqmConfigureQueuePacketTailDropThreshold(xpsDevice_t devId,
                                                         xpsPort_t devPort, uint32_t queueNum, uint32_t threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetQueuePacketTailDropThreshold(xpsDevice_t devId,
                                                   xpsPort_t devPort, uint32_t queueNum, uint32_t *threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmConfigurePortPacketTailDropThreshold(xpsDevice_t devId,
                                                        xpsPort_t devPort, uint32_t threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetPortPacketTailDropThreshold(xpsDevice_t devId,
                                                  xpsPort_t devPort, uint32_t *threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


/*
 * Q-Mapping Control APIs
 */
XP_STATUS xpsQosAqmGetQmapTableIndex(xpsDevice_t devId, xpPort_t destPort,
                                     xpPort_t srcPort, uint32_t mc, uint32_t txqSelVec, uint32_t tc,
                                     uint32_t *qmapIdx, uint32_t *queueLoc)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetQueueAbsoluteNumber(xpsDevice_t devId, xpPort_t devPort,
                                          uint32_t queueNum, uint32_t *absQNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetQueueAtQmapIndex(xpsDevice_t devId, uint32_t qmapIdx,
                                       uint32_t queueLoc, uint32_t *absQNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmSetQueueAtQmapIndex(xpsDevice_t devId, uint32_t qmapIdx,
                                       uint32_t queueLoc, uint32_t absQNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * Miscellaneous AQM Apis
 */

XP_STATUS xpsQosAqmGetAqmQTotalProfiles(xpsDevice_t devId,
                                        uint32_t* numProfiles)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetAqmPortTotalProfiles(xpsDevice_t devId,
                                           uint32_t* numProfiles)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetNumberOfQueuesPerPort(xpsDevice_t devId, xpPort_t port,
                                            uint32_t* numQs)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS     retVal      = XP_NO_ERR;
    uint32_t      cpuPortNum  = 0;
    XP_DEV_TYPE_T devType;

    if (numQs == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    cpssHalGetDeviceType(devId, &devType);

    retVal = xpsGlobalSwitchControlGetCpuPhysicalPortNum(devId, &cpuPortNum);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get CPU portNum, ret:%d\n", retVal);
        return retVal;
    }

    if (port == cpuPortNum)
    {
        *numQs = cpssHalSys_param_maxNumOfQueues(devType);
    }
    else
    {
        /* TxQ per port in HW is 0 - 7.
         * Queues 8 - 23 are added as WA for SNMP queue stats TCs.
         * These are dummy queues in SW and dont have any presence in ASIC*/
        *numQs = 24;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetNumberOfUnicastQueuesPerPort(xpsDevice_t devId,
                                                   xpPort_t port, uint32_t* numUcQs)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS     retVal      = XP_NO_ERR;
    XP_DEV_TYPE_T devType;

    if (numUcQs == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    retVal = cpssHalGetDeviceType(devId, &devType);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    *numUcQs = cpssHalSys_param_maxNumOfQueues(devType);

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsQosAqmGetNumberOfMulticastQueuesPerPort(xpsDevice_t devId,
                                                     xpPort_t port, uint32_t *numMcQs)
{
    XPS_FUNC_ENTRY_LOG();

    if (numMcQs == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    *numMcQs = 0;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmSetQueueGuarThresholdGranularity(xpsDevice_t devId,
                                                    xpQGuarThresholdGranularity_e granularity)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmGetQueueGuarThresholdGranularity(xpsDevice_t devId,
                                                    xpQGuarThresholdGranularity_e* granularity)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmDisplayAqmQProfile(xpsDevice_t devId, uint32_t profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosAqmDisplayAqmProfile(xpsDevice_t devId, uint32_t profileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
xps CPU Storm Control section
*/

XP_STATUS xpsQosCpuScSetReasonCodeTableEntry(xpsDevice_t devId, uint32_t index,
                                             uint32_t destPort, uint32_t tc, uint32_t enableTruncation, uint32_t enableMod)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosCpuScSetRctDestPort(xpsDevice_t devId, uint32_t index,
                                    uint32_t destPort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosCpuScSetRctTc(xpsDevice_t devId, uint32_t index, uint32_t tc)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosCpuScSetRctTruncation(xpsDevice_t devId, uint32_t index,
                                      uint32_t enableTruncation)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosCpuScSetRctEnableMod(xpsDevice_t devId, uint32_t index,
                                     uint32_t enableMod)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosCpuScGetReasonCodeTableEntry(xpsDevice_t devId, uint32_t index,
                                             uint32_t *destPort, uint32_t *tc, uint32_t *enableTruncation,
                                             uint32_t *enableMod)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosCpuScGetRctDestPort(xpsDevice_t devId, uint32_t index,
                                    uint32_t *destPort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosCpuScGetRctTc(xpsDevice_t devId, uint32_t index, uint32_t *tc)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosCpuScGetRctTruncation(xpsDevice_t devId, uint32_t index,
                                      uint32_t *enableTruncation)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosCpuScGetRctEnableMod(xpsDevice_t devId, uint32_t index,
                                     uint32_t *enableMod)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosCpuScGetQueueGroupId(xpsDevice_t devId, xpPort_t devPort,
                                     uint32_t queueNum, uint32_t *queueGroupId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosCpuScGetDmaQueueGroupId(xpsDevice_t devId, uint32_t dmaQNum,
                                        uint32_t *dmaQueueGroupId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosCpuScCpuQGrpToDmaQGrpMapAdd(xpsDevice_t devId,
                                            uint32_t cpuQueueGroupNum, uint32_t dmaQueueGroupNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosCpuScCpuQGrpToDmaQGrpMapGet(xpsDevice_t devId,
                                            uint32_t cpuQueueGroupNum, uint32_t *dmaQueueGroupNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosCpuScCpuQGrpToDmaQGrpMapRemove(xpsDevice_t devId,
                                               uint32_t cpuQueueGroupNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosCpuScAddReasonCodeToCpuQueueMap(xpsDevice_t devId,
                                                xpCpuTypes_e cpuType, uint32_t reasonCode, uint32_t queueNum)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS status;
    (void)cpuType;
    uint8_t count = 0;//Max number of CPU codes
    CPSS_NET_RX_CPU_CODE_ENT cpuCode[MAX_CPU_CODE_COUNT];

    if (reasonCode >= XPS_COPP_CTRL_PKT_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid reason code %d, " "supported number of reason codes %d\n", reasonCode,
              XPS_COPP_CTRL_PKT_MAX);
        return XP_ERR_VALUE_OUT_OF_RANGE;
    }
    /* Convert xps reason code to cpss cpu code */
    (void)xpsUtilConvertReasonCodeToCpssCpuCode((xpsCoppCtrlPkt_e)reasonCode,
                                                cpuCode, &count);

    if (queueNum >= CPSS_HAL_QOS_NUM_CPU_QUEUES)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid queue number %d, "
              "supported number of queues %d\n", queueNum, CPSS_HAL_QOS_NUM_CPU_QUEUES);
        return XP_ERR_INVALID_QUEUE;
    }

    for (int i =0; i < count; i++)
    {
        status = cpssHalQosBindReasonCodeToCpuQueue(devId, cpuCode[i], queueNum);
        if (GT_OK != status)
        {
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosCpuScGetReasonCodeToCpuQueueMap(xpsDevice_t devId,
                                                xpCpuTypes_e cpuType, uint32_t reasonCode, uint32_t *queueNum)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS status;
    (void)cpuType;
    uint8_t count = 0;//Max number of CPU codes
    CPSS_NET_RX_CPU_CODE_ENT cpuCode[MAX_CPU_CODE_COUNT];

    if (reasonCode >= XPS_COPP_CTRL_PKT_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid reason code %d, " "supported number of reason codes %d\n", reasonCode,
              XPS_COPP_CTRL_PKT_MAX);
        return XP_ERR_VALUE_OUT_OF_RANGE;
    }

    if (!queueNum)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "queueNum is a NULL pointer\n");
        return XP_ERR_NULL_POINTER;
    }

    /* Convert xps reason code to cpss cpu code */
    (void)xpsUtilConvertReasonCodeToCpssCpuCode((xpsCoppCtrlPkt_e)reasonCode,
                                                cpuCode, &count);
    for (int i =0; i < count; i++)
    {
        status = cpssHalQosGetReasonCodeToCpuQueue(devId, cpuCode[i], queueNum);
        if (GT_OK != status)
        {
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * 802.1Qbb: PFC configuration APIs
 */

/*
 * Generic PFC Feature Configuration APIs
 */

XP_STATUS xpsQosFcSetPfcRcvEn(xpsDevice_t devId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPfcRcvEn(xpsDevice_t devId, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPfcPriority(xpsDevice_t devId, xpsPort_t devPort,
                                 uint32_t queueNum, uint8_t prio)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPfcPriority(xpsDevice_t devId, xpsPort_t devPort,
                                 uint32_t queueNum, uint32_t* prio)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetNumPfcPrioritiesSupported(xpsDevice_t devId,
                                               uint32_t *totalSupportedPrio)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcBindPfcPriorityToTc(xpsDevice_t devId, uint32_t priority,
                                      uint32_t pfcTrafficClass)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPfcPriorityToTcBinding(xpsDevice_t devId,
                                            uint32_t priority, uint32_t *pfcTrafficClass)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * PFC: Port Group Static Threshold section
 */

XP_STATUS xpsQosFcBindPortToPfcGroupStaticProfile(xpsDevice_t devId,
                                                  uint32_t portId, uint32_t pfcProfileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortToPfcGroupStaticProfileBinding(xpsDevice_t devId,
                                                        uint32_t portId, uint32_t *pfcProfileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPortPfcGroupStaticXonThreshold(xpsDevice_t devId,
                                                    uint32_t portPfcProfileId, uint32_t xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortPfcGroupStaticXonThreshold(xpsDevice_t devId,
                                                    uint32_t portPfcProfileId, uint32_t *xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPortPfcGroupStaticXoffThreshold(xpsDevice_t devId,
                                                     uint32_t portPfcProfileId, uint32_t xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortPfcGroupStaticXoffThreshold(xpsDevice_t devId,
                                                     uint32_t portPfcProfileId, uint32_t *xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * PFC: Global Tc Static Threshold section
 */

XP_STATUS xpsQosFcSetGlobalTcPfcStaticXonThreshold(xpsDevice_t devId,
                                                   uint32_t trafficClass, uint32_t xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetGlobalTcPfcStaticXonThreshold(xpsDevice_t devId,
                                                   uint32_t trafficClass, uint32_t *xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetGlobalTcPfcStaticXoffThreshold(xpsDevice_t devId,
                                                    uint32_t trafficClass, uint32_t xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetGlobalTcPfcStaticXoffThreshold(xpsDevice_t devId,
                                                    uint32_t trafficClass, uint32_t *xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * PFC: Port/TC Static Threshold section
 */

XP_STATUS xpsQosFcBindPortToPortTcPfcStaticProfile(xpsDevice_t devId,
                                                   uint32_t portId, uint32_t  pfcProfileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortToPortTcPfcStaticProfile(xpsDevice_t devId,
                                                  uint32_t portId, uint32_t *pfcProfileId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPortTcPfcStaticXonThresholdForProfile(xpsDevice_t devId,
                                                           uint32_t pfcProfileId, uint32_t trafficClass, uint32_t xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortTcPfcStaticXonThresholdForProfile(xpsDevice_t devId,
                                                           uint32_t pfcProfileId, uint32_t trafficClass, uint32_t *xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPortTcPfcStaticXoffThresholdForProfile(xpsDevice_t devId,
                                                            uint32_t pfcProfileId, uint32_t trafficClass, uint32_t xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortTcPfcStaticXoffThresholdForProfile(xpsDevice_t devId,
                                                            uint32_t pfcProfileId, uint32_t trafficClass, uint32_t *xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPortTcPfcStaticXonThreshold(xpDevice_t devId,
                                                 xpPort_t devPort, uint32_t pfcTrafficClass, uint32_t xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortTcPfcStaticXonThreshold(xpDevice_t devId,
                                                 xpPort_t devPort, uint32_t pfcTrafficClass, uint32_t *xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPortTcPfcStaticXoffThreshold(xpDevice_t devId,
                                                  xpPort_t devPort, uint32_t pfcTrafficClass, uint32_t xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortTcPfcStaticXoffThreshold(xpDevice_t devId,
                                                  xpPort_t devPort, uint32_t pfcTrafficClass, uint32_t *xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * PFC: Global Static Threshold section
 */

XP_STATUS xpsQosFcSetGlobalPfcStaticXonThreshold(xpsDevice_t devId,
                                                 uint32_t xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetGlobalPfcStaticXonThreshold(xpsDevice_t devId,
                                                 uint32_t *xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetGlobalPfcStaticXoffThreshold(xpsDevice_t devId,
                                                  uint32_t xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetGlobalPfcStaticXoffThreshold(xpsDevice_t devId,
                                                  uint32_t *xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * PFC: Port Group/Tc Static Threshold section
 */

XP_STATUS xpsQosFcSetPortPfcGroupTcStaticXonThreshold(xpsDevice_t devId,
                                                      uint32_t portPfcProfileId, uint32_t trafficClass, uint32_t xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortPfcGroupTcStaticXonThreshold(xpsDevice_t devId,
                                                      uint32_t portPfcProfileId, uint32_t trafficClass, uint32_t *xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPortPfcGroupTcStaticXoffThreshold(xpsDevice_t devId,
                                                       uint32_t portPfcProfileId, uint32_t trafficClass, uint32_t xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortPfcGroupTcStaticXoffThreshold(xpsDevice_t devId,
                                                       uint32_t portPfcProfileId, uint32_t trafficClass, uint32_t *xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * 802.1Qbb Priority Flow Control Port Configuration APIs
 */

XP_STATUS xpsQosFcSetPortPfcXonThreshold(xpDevice_t devId, uint32_t port,
                                         uint32_t xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortPfcXonThreshold(xpDevice_t devId, uint32_t port,
                                         uint32_t *xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPortPfcXoffThreshold(xpDevice_t devId, uint32_t port,
                                          uint32_t xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortPfcXoffThreshold(xpDevice_t devId, uint32_t port,
                                          uint32_t *xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * Configure 802.1Qbb PFC Dynamic Thresholds and bind Dynamic Pools for a Port/Traffic Class Combination
 */

XP_STATUS xpsQosFcEnablePfcDynamicThresholding(xpDevice_t devId,
                                               uint32_t devPort, uint32_t trafficClass, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPfcDynamicThresholdingEnable(xpDevice_t devId,
                                                  uint32_t devPort, uint32_t trafficClass, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcBindPfcTcToDynamicPool(xpsDevice_t devId,
                                         uint32_t pfcTrafficClass, uint32_t dynPoolId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPfcTcToDynamicPoolBinding(xpsDevice_t devId,
                                               uint32_t pfcTrafficClass, uint32_t *dynPoolId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcUnbindPfcTcFromDynamicPool(xpsDevice_t devId,
                                             uint32_t pfcTrafficClass, uint32_t dynPoolId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPfcTcDynamicPoolTotalSize(xpsDevice_t devId,
                                               uint32_t dynPoolId, uint32_t poolSizeInPages)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPfcTcDynamicPoolTotalSize(xpsDevice_t devId,
                                               uint32_t dynPoolId, uint32_t *poolSizeInPages)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPfcTcDynamicPoolSharedSize(xpsDevice_t devId,
                                                uint32_t dynPoolId, uint32_t poolSizeInPages)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPfcTcDynamicPoolSharedSize(xpsDevice_t devId,
                                                uint32_t dynPoolId, uint32_t *poolSizeInPages)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPfcTcDynamicPoolGuaranteedSize(xpsDevice_t devId,
                                                    uint32_t dynPoolId, uint32_t poolSizeInPages)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPfcTcDynamicPoolGuaranteedSize(xpsDevice_t devId,
                                                    uint32_t dynPoolId, uint32_t *poolSizeInPages)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcConfigurePfcTcDynamicPoolAlpha(xpsDevice_t devId,
                                                 uint32_t dynPoolId, xpDynThldFraction_e fraction, xpDynThldCoeff_e coefficient)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPfcTcDynamicPoolAlpha(xpsDevice_t devId,
                                           uint32_t dynPoolId, xpDynThldFraction_e *fraction,
                                           xpDynThldCoeff_e *coefficient)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPfcTcDynamicXonCalculationEnable(xpDevice_t devId,
                                                      xpPort_t devPort, uint32_t pfcTrafficClass, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPfcTcDynamicXonCalculationEnable(xpDevice_t devId,
                                                      xpPort_t devPort, uint32_t pfcTrafficClass, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcConfigurePfcTcDynamicPoolXoffToXonRatio(xpsDevice_t devId,
                                                          uint32_t dynPoolId, xpDynThldOperator_e operation, xpDynThldCoeff_e betaCoeff,
                                                          xpDynThldCoeff_e gammaCoeff)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPfcTcDynamicPoolXoffToXonRatio(xpsDevice_t devId,
                                                    uint32_t dynPoolId, xpDynThldOperator_e *operation, xpDynThldCoeff_e *betaCoeff,
                                                    xpDynThldCoeff_e *gammaCoeff)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPfcDynamicPoolCurrentSharedUsed(xpDevice_t devId,
                                                     uint32_t dynPoolId, uint32_t *counter)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPfcDynamicPoolMaxSharedUsed(xpDevice_t devId,
                                                 uint32_t dynPoolId, uint32_t clearOnRead, uint32_t *counter)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * Flow Control: Port Threshold section
 */
XP_STATUS xpsQosFcSetPortFcXonThreshold(xpsDevice_t devId, uint32_t port,
                                        uint32_t xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortFcXonThreshold(xpsDevice_t devId, uint32_t port,
                                        uint32_t *xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPortFcXoffThreshold(xpsDevice_t devId, uint32_t port,
                                         uint32_t xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortFcXoffThreshold(xpsDevice_t devId, uint32_t port,
                                         uint32_t *xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPortFcFullThreshold(xpsDevice_t devId, uint32_t port,
                                         uint32_t fullThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortFcFullThreshold(xpsDevice_t devId, uint32_t port,
                                         uint32_t *fullThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * Flow Control: Port Group Threshold section
 */

XP_STATUS xpsQosFcBindPortToPortFcGroup(xpsDevice_t devId, uint32_t port,
                                        uint32_t portFcGroup)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortToPortFcGroupBinding(xpsDevice_t devId, uint32_t port,
                                              uint32_t *portFcGroup)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPortFcGroupXonThreshold(xpsDevice_t devId,
                                             uint32_t portFcGroup, uint32_t xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortFcGroupXonThreshold(xpsDevice_t devId,
                                             uint32_t portFcGroup, uint32_t *xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPortFcGroupXoffThreshold(xpsDevice_t devId,
                                              uint32_t portFcGroup, uint32_t xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortFcGroupXoffThreshold(xpsDevice_t devId,
                                              uint32_t portFcGroup, uint32_t *xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetPortFcGroupFullThreshold(xpsDevice_t devId,
                                              uint32_t portFcGroup, uint32_t fullThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPortFcGroupFullThreshold(xpsDevice_t devId,
                                              uint32_t portFcGroup, uint32_t *fullThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * Flow Control: Global Threshold section
 */

XP_STATUS xpsQosFcSetGlobalFcXonThreshold(xpsDevice_t devId,
                                          uint32_t xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetGlobalFcXonThreshold(xpsDevice_t devId,
                                          uint32_t *xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetGlobalFcXoffThreshold(xpsDevice_t devId,
                                           uint32_t xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetGlobalFcXoffThreshold(xpsDevice_t devId,
                                           uint32_t *xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcSetGlobalFcFullThreshold(xpsDevice_t devId,
                                           uint32_t fullThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetGlobalFcFullThreshold(xpsDevice_t devId,
                                           uint32_t *fullThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetPfcPortTcCounter(xpDevice_t devId, uint32_t portNum,
                                      uint32_t pfcTrafficClass, uint32_t *counter)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetHeadroomCount(xpDevice_t devId, uint32_t portNum,
                                   uint32_t priority, uint32_t clearOnRead, uint64_t *count)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetMaxHeadroomCount(xpDevice_t devId, uint32_t portNum,
                                      uint32_t priority, uint32_t clearOnRead, uint64_t *count)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcConfigureMaxHeadroomThreshold(xpDevice_t devId,
                                                uint32_t portNum, uint32_t priority, uint32_t threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosFcGetMaxHeadroomThreshold(xpDevice_t devId, uint32_t portNum,
                                          uint32_t priority, uint32_t *threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
    IBUFFER PARSER CONFIGURATIONS
*/

XP_STATUS xpsQosIbConfigureGlobalMacDaForPriorityAssignment(xpsDevice_t devId,
                                                            uint32_t profileId, macAddr_t macDa, macAddr_t macDaMask)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetGlobalMacDaForPriorityAssignment(xpsDevice_t devId,
                                                      uint32_t profileId, macAddr_t macDa, macAddr_t macDaMask)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbConfigureGlobalMacDaPriority(xpsDevice_t devId,
                                               uint32_t profileId, uint32_t priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetGlobalMacDaPriority(xpsDevice_t devId, uint32_t profileId,
                                         uint32_t *priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbConfigureGlobalMacDaPriorityEnable(xpsDevice_t devId,
                                                     uint32_t profileId, uint32_t priorityEn)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetGlobalMacDaPriorityEnable(xpsDevice_t devId,
                                               uint32_t profileId, uint32_t *priorityEn)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbConfigureGlobalEthertypeForPriorityAssignment(
    xpsDevice_t devId, uint32_t profileId, uint32_t etherType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetGlobalEthertypeForPriorityAssignment(xpsDevice_t devId,
                                                          uint32_t profileId, uint32_t *etherType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbConfigureGlobalEthertypePriority(xpsDevice_t devId,
                                                   uint32_t profileId, uint32_t priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetGlobalEthertypePriority(xpsDevice_t devId,
                                             uint32_t profileId, uint32_t *priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbConfigureGlobalEthertypePriorityEnable(xpsDevice_t devId,
                                                         uint32_t profileId, uint32_t priorityEn)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetGlobalEthertypePriorityEnable(xpsDevice_t devId,
                                                   uint32_t profileId, uint32_t *priorityEn)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbConfigurePortMplsPriorityEnable(xpsDevice_t devId,
                                                  xpPort_t portNum, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetPortMplsPriorityEnable(xpsDevice_t devId, xpPort_t portNum,
                                            uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbConfigurePortMplsPriority(xpsDevice_t devId, xpPort_t portNum,
                                            uint32_t exp, uint32_t priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetPortMplsPriority(xpsDevice_t devId, xpPort_t portNum,
                                      uint32_t exp, uint32_t *priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbConfigurePortL3PriorityEnable(xpsDevice_t devId,
                                                xpPort_t portNum, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetPortL3PriorityEnable(xpsDevice_t devId, xpPort_t portNum,
                                          uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbConfigurePortL3Priority(xpsDevice_t devId, xpPort_t portNum,
                                          uint32_t dscp, uint32_t priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetPortL3Priority(xpsDevice_t devId, xpPort_t portNum,
                                    uint32_t dscp, uint32_t *priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbConfigurePortL2PriorityEnable(xpsDevice_t devId,
                                                xpPort_t portNum, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetPortL2PriorityEnable(xpsDevice_t devId, xpPort_t portNum,
                                          uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbConfigurePortL2Priority(xpsDevice_t devId, xpPort_t portNum,
                                          uint32_t pcp, uint32_t priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetPortL2Priority(xpsDevice_t devId, xpPort_t portNum,
                                    uint32_t pcp, uint32_t *priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbConfigurePortDefaultPriority(xpsDevice_t devId,
                                               xpPort_t portNum, uint32_t priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetPortDefaultPriority(xpsDevice_t devId, xpPort_t portNum,
                                         uint32_t *priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
    IBUFFER BUFFER CONFIGURATIONS
*/

XP_STATUS xpsQosIbConfigurePortLosslessXonThreshold(xpsDevice_t devId,
                                                    uint32_t portNum, uint32_t xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetPortLosslessXonThreshold(xpsDevice_t devId,
                                              uint32_t portNum, uint32_t *xonThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbConfigurePortLosslessXoffThreshold(xpsDevice_t devId,
                                                     uint32_t portNum, uint32_t xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetPortLosslessXoffThreshold(xpsDevice_t devId,
                                               uint32_t portNum, uint32_t *xoffThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbConfigurePortLosslessDropThreshold(xpsDevice_t devId,
                                                     uint32_t portNum, uint32_t dropThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetPortLosslessDropThreshold(xpsDevice_t devId,
                                               uint32_t portNum, uint32_t *dropThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbConfigurePortLossyDropThresholdPerPriority(xpsDevice_t devId,
                                                             uint32_t portNum, uint32_t priority, uint32_t dropThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetPortLossyDropThresholdPerPriority(xpsDevice_t devId,
                                                       uint32_t portNum, uint32_t priority, uint32_t *dropThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbEnableLosslessPerPortPriority(xpsDevice_t devId,
                                                uint32_t portNum, uint32_t priority, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosIbGetLosslessConfigPerPortPriority(xpsDevice_t devId,
                                                   uint32_t portNum, uint32_t priority, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * XPS Port Ingress Qos Map Section
 */

XP_STATUS xpsQosPortIngressSetTrafficClassForL2QosProfile(xpsDevice_t devId,
                                                          xpQosMapPfl_t profile, uint32_t pcpVal, uint32_t deiVal, uint32_t tc)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetTrafficClassForL2QosProfile(xpsDevice_t devId,
                                                          xpQosMapPfl_t profile, uint32_t pcpVal, uint32_t deiVal, uint32_t *tc)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressSetTrafficClassForL3QosProfile(xpsDevice_t devId,
                                                          xpQosMapPfl_t profile, uint32_t dscpVal, uint32_t tc)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetTrafficClassForL3QosProfile(xpsDevice_t devId,
                                                          xpQosMapPfl_t profile, uint32_t dscpVal, uint32_t *tc)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressSetTrafficClassForMplsQosProfile(xpsDevice_t devId,
                                                            xpQosMapPfl_t profile, uint32_t expVal, uint32_t tc)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetTrafficClassForMplsQosProfile(xpsDevice_t devId,
                                                            xpQosMapPfl_t profile, uint32_t expVal, uint32_t *tc)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressSetDropPrecedenceForL2QosProfile(xpsDevice_t devId,
                                                            xpQosMapPfl_t profile, uint32_t pcpVal, uint32_t deiVal, uint32_t dp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetDropPrecedenceForL2QosProfile(xpsDevice_t devId,
                                                            xpQosMapPfl_t profile, uint32_t pcpVal, uint32_t deiVal, uint32_t *dp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressSetDropPrecedenceForL3QosProfile(xpsDevice_t devId,
                                                            xpQosMapPfl_t profile, uint32_t dscpVal, uint32_t dp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetDropPrecedenceForL3QosProfile(xpsDevice_t devId,
                                                            xpQosMapPfl_t profile, uint32_t dscpVal, uint32_t *dp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressSetDropPrecedenceForMplsQosProfile(xpsDevice_t devId,
                                                              xpQosMapPfl_t profile, uint32_t expVal, uint32_t dp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetDropPrecedenceForMplsQosProfile(xpsDevice_t devId,
                                                              xpQosMapPfl_t profile, uint32_t expVal, uint32_t *dp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressRemapPriorityForL2QosProfile(xpsDevice_t devId,
                                                        xpQosMapPfl_t profile, uint32_t pcpVal,
                                                        uint32_t deiVal, uint32_t remapPcpVal, uint32_t remapDeiVal,
                                                        uint32_t  remapDscpVal, uint32_t remapExpVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetRemappedPriorityForL2QosProfile(xpsDevice_t devId,
                                                              xpQosMapPfl_t profile, uint32_t pcpVal,
                                                              uint32_t deiVal, uint32_t  *remapPcpVal, uint32_t *remapDeiVal,
                                                              uint32_t  *remapDscpVal, uint32_t *remapExpVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
XP_STATUS xpsQosPortIngressRemapPriorityForL3QosProfile(xpsDevice_t devId,
                                                        xpQosMapPfl_t profile, uint32_t dscpVal,
                                                        uint32_t remapPcpVal, uint32_t remapDeiVal, uint32_t remapDscpVal,
                                                        uint32_t remapExpVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetRemappedPriorityForL3QosProfile(xpsDevice_t devId,
                                                              xpQosMapPfl_t profile, uint32_t dscpVal,
                                                              uint32_t *remapPcpVal, uint32_t *remapDeiVal, uint32_t *remapDscpVal,
                                                              uint32_t *remapExpVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressRemapPriorityForMplsQosProfile(xpsDevice_t devId,
                                                          xpQosMapPfl_t profile, uint32_t expVal,
                                                          uint32_t remapPcpVal, uint32_t remapDeiVal, uint32_t remapDscpVal,
                                                          uint32_t remapExpVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetRemappedPriorityForMplsQosProfile(
    xpsDevice_t devId, xpQosMapPfl_t profile, uint32_t expVal,
    uint32_t *remapPcpVal, uint32_t *remapDeiVal, uint32_t *remapDscpVal,
    uint32_t *remapExpVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressSetPortDefaultL2QosPriority(xpsDevice_t devId,
                                                       xpPort_t devPort, uint32_t pcpVal, uint32_t deiVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetPortDefaultL2QosPriority(xpsDevice_t devId,
                                                       xpPort_t devPort, uint32_t *pcpVal, uint32_t *deiVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressSetPortDefaultL3QosPriority(xpsDevice_t devId,
                                                       xpPort_t devPort, uint32_t dscpVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetPortDefaultL3QosPriority(xpsDevice_t devId,
                                                       xpPort_t devPort, uint32_t *dscpVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressSetPortDefaultMplsQosPriority(xpsDevice_t devId,
                                                         xpPort_t devPort, uint32_t expVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetPortDefaultMplsQosPriority(xpsDevice_t devId,
                                                         xpPort_t devPort, uint32_t *expVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressSetPortDefaultTrafficClass(xpsDevice_t devId,
                                                      xpPort_t devPort, uint32_t tc)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetPortDefaultTrafficClass(xpsDevice_t devId,
                                                      xpPort_t devPort, uint32_t *tc)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressSetPortDefaultDropPrecedence(xpsDevice_t devId,
                                                        xpPort_t devPort, uint32_t dp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetPortDefaultDropPrecedence(xpsDevice_t devId,
                                                        xpPort_t devPort, uint32_t *dp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressSetL2QosProfileForPort(xpsDevice_t devId,
                                                  xpPort_t devPort, xpQosMapPfl_t profile, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetL2QosProfileForPort(xpsDevice_t devId,
                                                  xpPort_t devPort, xpQosMapPfl_t *profile, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressSetL3QosProfileForPort(xpsDevice_t devId,
                                                  xpPort_t devPort, xpQosMapPfl_t profile, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetL3QosProfileForPort(xpsDevice_t devId,
                                                  xpPort_t devPort, xpQosMapPfl_t *profile, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressSetMplsQosProfileForPort(xpsDevice_t devId,
                                                    xpPort_t devPort, xpQosMapPfl_t profile, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressGetMplsQosProfileForPort(xpsDevice_t devId,
                                                    xpPort_t devPort, xpQosMapPfl_t *profile, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressSetTrustL2ForPort(xpsDevice_t devId,
                                             xpPort_t devPort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressSetTrustL3ForPort(xpsDevice_t devId,
                                             xpPort_t devPort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosPortIngressSetTrustMplsForPort(xpsDevice_t devId,
                                               xpPort_t devPort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetPcpDeiRemarkEn(xpsDevice_t devId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetPcpDeiRemarkEnable(xpsDevice_t devId, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetDscpRemarkEn(xpsDevice_t devId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetDscpRemarkEnable(xpsDevice_t devId, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
xps Queue counter section
*/

XP_STATUS xpsQosQcGetQueueFwdPacketCountForPort(xpsDevice_t devId,
                                                xpPort_t port, uint32_t queue, uint64_t *count, uint32_t *wrap)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcGetQueueDropPacketCountForPort(xpsDevice_t devId,
                                                 xpPort_t port, uint32_t queue, uint64_t *count, uint32_t *wrap)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcGetQueueFwdByteCountForPort(xpsDevice_t devId, xpPort_t port,
                                              uint32_t queue, uint64_t *count, uint32_t *wrap)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcGetQueueDropByteCountForPort(xpsDevice_t devId, xpPort_t port,
                                               uint32_t queue, uint64_t *count, uint32_t *wrap)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcGetCurrentQueuePacketDepth(xpsDevice_t devId, xpPort_t port,
                                             uint32_t queue, uint32_t *count)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcGetCurrentQueuePageDepth(xpsDevice_t devId, xpPort_t port,
                                           uint32_t queue, uint32_t *count)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcGetCurrentPortPageDepth(xpsDevice_t devId, uint32_t port,
                                          uint32_t *count)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcGetCurrentH1PageDepth(xpsDevice_t devId, uint32_t h1Node,
                                        uint32_t *count)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcGetCurrentH2PageDepth(xpsDevice_t devId, uint32_t h2Node,
                                        uint32_t *count)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsQosQcGetQueueAveragePageLength(xpsDevice_t devId, xpPort_t port,
                                            uint32_t queue, uint32_t *count)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcGetQueueOldPageLength(xpsDevice_t devId, xpPort_t port,
                                        uint32_t queue, uint32_t *count)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcGetPfcPageCount(xpsDevice_t devId, xpPort_t port,
                                  uint32_t priority, uint32_t *count)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcEnableFwdPktCountClearOnRead(xpsDevice_t devId,
                                               uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcIsEnableFwdPktCountClearOnRead(xpsDevice_t devId,
                                                 uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsQosQcEnableFwdPktByteCountClearOnRead(xpsDevice_t devId,
                                                   uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcIsEnableFwdPktByteCountClearOnRead(xpsDevice_t devId,
                                                     uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcEnableDropPktCountClearOnRead(xpsDevice_t devId,
                                                uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcIsEnableDropPktCountClearOnRead(xpsDevice_t devId,
                                                  uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcEnableDropPktByteCountClearOnRead(xpsDevice_t devId,
                                                    uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcIsEnableDropPktByteCountClearOnRead(xpsDevice_t devId,
                                                      uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcGetGlobalPageCount(xpsDevice_t devId, uint32_t *count)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosQcGetGlobalMulticastPageCount(xpsDevice_t devId,
                                              uint32_t *count)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * XPS Port Shaper Section
 */

XP_STATUS xpsQosShaperEnablePortShaping(xpsDevice_t devId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperIsPortShapingEnabled(xpsDevice_t devId, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperSetPortShaperMtu(xpsDevice_t devId, uint32_t mtuInBytes)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperGetPortShaperMtu(xpsDevice_t devId, uint32_t *mtuInBytes)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperSetPortShaperEmptyThreshold(xpsDevice_t devId,
                                                  uint32_t emptyThld)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperGetPortShaperEmptyThreshold(xpsDevice_t devId,
                                                  uint32_t *emptyThld)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperConfigurePortShaper(xpsDevice_t devId, xpPort_t devPort,
                                          uint64_t rateKbps, uint32_t maxBurstByteSize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperGetPortShaperConfiguration(xpsDevice_t devId,
                                                 xpPort_t devPort, uint64_t *rateKbps, uint32_t *maxBurstByteSize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperSetPortShaperEnable(xpsDevice_t devId, xpPort_t devPort,
                                          uint32_t enableShaper)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperGetPortShaperEnable(xpsDevice_t devId, xpPort_t devPort,
                                          uint32_t *enableShaper)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperGetPortShaperTableIndex(xpsDevice_t devId,
                                              xpPort_t devPort, uint32_t *index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * XPS Queue Shaping Feature Enable APIs
 */

XP_STATUS xpsQosShaperEnableQueueShaping(xpsDevice_t devId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperIsQueueShapingEnabled(xpsDevice_t devId, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperSetQueueShaperMtu(xpsDevice_t devId,  uint32_t mtuInBytes)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperGetQueueShaperMtu(xpsDevice_t devId, uint32_t *mtuInBytes)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperSetQueueShaperEmptyThreshold(xpsDevice_t devId,
                                                   uint32_t emptyThld)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperGetQueueShaperEmptyThreshold(xpsDevice_t devId,
                                                   uint32_t *emptyThld)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * XPS Queue Slow Shaper Section
 */

XP_STATUS xpsQosShaperConfigureQueueSlowShaper(xpsDevice_t devId,
                                               xpPort_t devPort, uint32_t queueNum, uint64_t rateKbps,
                                               uint32_t maxBurstByteSize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperGetQueueSlowShaperConfiguation(xpsDevice_t devId,
                                                     xpPort_t devPort, uint32_t queueNum, uint64_t *rateKbps,
                                                     uint32_t *maxBurstByteSize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperSetQueueSlowShaperEnable(xpsDevice_t devId,
                                               xpPort_t devPort, uint32_t queueNum, uint32_t enableShaper)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperGetQueueSlowShaperEnable(xpsDevice_t devId,
                                               xpPort_t devPort, uint32_t queueNum, uint32_t *enableShaper)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperGetQueueSlowShaperTableIndex(xpsDevice_t devId,
                                                   xpPort_t devPort, uint32_t queueNum, uint32_t *index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * XPS Queue Fast Shaper Section
 */

XP_STATUS xpsQosShaperBindQueueToFastShaper(xpsDevice_t devId, xpPort_t devPort,
                                            uint32_t queueNum, uint32_t *fastShaperNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperConfigureQueueFastShaper(xpsDevice_t devId,
                                               uint32_t fastShaperNum, uint64_t rateKbps, uint32_t maxBurstByteSize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperGetQueueFastShaperConfiguation(xpsDevice_t devId,
                                                     uint32_t fastShaperNum, uint64_t *rateKbps, uint32_t *maxBurstSize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperSetQueueFastShaperEnable(xpsDevice_t devId,
                                               uint32_t fastShaperNum, uint32_t enableShaper)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperGetQueueFastShaperEnable(xpsDevice_t devId,
                                               uint32_t fastShaperNum, uint32_t *enableShaper)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosShaperGetQueueFastShaperTableIndex(xpsDevice_t devId,
                                                   xpPort_t devPort, uint32_t queueNum, uint32_t *index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * XPS Scheduler APIs
 */

XP_STATUS xpsQosSetQueueSchedulerDWRR(xpsDevice_t devId, xpsPort_t devPort,
                                      uint32_t queueNum, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetQueueSchedulerDWRR(xpsDevice_t devId, xpsPort_t devPort,
                                      uint32_t queueNum, uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetQueueSchedulerDWRRWeight(xpsDevice_t devId,
                                            xpsPort_t devPort, uint32_t queueNum, uint32_t weight)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetQueueSchedulerDWRRWeight(xpsDevice_t devId,
                                            xpsPort_t devPort, uint32_t queueNum, uint32_t* weight)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetQueueSchedulerDWRRMtu(xpsDevice_t devId, uint32_t mtuInBytes)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetQueueSchedulerDWRRMtu(xpsDevice_t devId,
                                         uint32_t *mtuInBytes)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetQueueSchedulerSP(xpsDevice_t devId, xpsPort_t devPort,
                                    uint32_t queueNum, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetQueueSchedulerSP(xpsDevice_t devId, xpsPort_t devPort,
                                    uint32_t queueNum, uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetQueueSchedulerSPPriority(xpsDevice_t devId,
                                            xpsPort_t devPort, uint32_t queueNum, uint32_t priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetQueueSchedulerSPPriority(xpsDevice_t devId,
                                            xpsPort_t devPort, uint32_t queueNum, uint32_t *priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetQueueToPortPath(xpsDevice_t devId, uint32_t portNum,
                                   uint32_t queueNum, xpTxqQueuePathToPort_t* path)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetPortDequeueEnable(xpsDevice_t devId, uint32_t portNum,
                                     uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetPortDequeueEnable(xpsDevice_t devId, uint32_t portNum,
                                     uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetQueueDequeueEnable(xpsDevice_t devId, uint32_t portNum,
                                      uint32_t queueNum, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetQueueDequeueEnable(xpsDevice_t devId, uint32_t portNum,
                                      uint32_t queueNum, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * H1 Scheduler APIs
 */

XP_STATUS xpsQosSetH1SchedulerDWRR(xpsDevice_t devId,
                                   xpTxqQueuePathToPort_t path, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetH1SchedulerDWRR(xpsDevice_t devId,
                                   xpTxqQueuePathToPort_t path, uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetH1SchedulerDWRRWeight(xpsDevice_t devId,
                                         xpTxqQueuePathToPort_t path, uint32_t weight)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetH1SchedulerDWRRWeight(xpsDevice_t devId,
                                         xpTxqQueuePathToPort_t path, uint32_t* weight)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetH1SchedulerSP(xpsDevice_t devId, xpTxqQueuePathToPort_t path,
                                 uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetH1SchedulerSP(xpsDevice_t devId, xpTxqQueuePathToPort_t path,
                                 uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetH1SchedulerSPPriority(xpsDevice_t devId,
                                         xpTxqQueuePathToPort_t path, uint32_t priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetH1SchedulerSPPriority(xpsDevice_t devId,
                                         xpTxqQueuePathToPort_t path, uint32_t *priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * H2 Scheduler APIs
 */

XP_STATUS xpsQosSetH2SchedulerDWRR(xpsDevice_t devId,
                                   xpTxqQueuePathToPort_t path, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetH2SchedulerDWRR(xpsDevice_t devId,
                                   xpTxqQueuePathToPort_t path, uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetH2SchedulerDWRRWeight(xpsDevice_t devId,
                                         xpTxqQueuePathToPort_t path, uint32_t weight)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetH2SchedulerDWRRWeight(xpsDevice_t devId,
                                         xpTxqQueuePathToPort_t path, uint32_t* weight)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetH2SchedulerSP(xpsDevice_t devId, xpTxqQueuePathToPort_t path,
                                 uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetH2SchedulerSP(xpsDevice_t devId, xpTxqQueuePathToPort_t path,
                                 uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetH2SchedulerSPPriority(xpsDevice_t devId,
                                         xpTxqQueuePathToPort_t path, uint32_t priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetH2SchedulerSPPriority(xpsDevice_t devId,
                                         xpTxqQueuePathToPort_t path, uint32_t *priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * Port Scheduler APIs
 */

XP_STATUS xpsQosSetPortSchedulerDWRR(xpsDevice_t devId, uint32_t portNum,
                                     uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetPortSchedulerDWRR(xpsDevice_t devId, uint32_t portNum,
                                     uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetPortSchedulerDWRRWeight(xpsDevice_t devId, uint32_t portNum,
                                           uint32_t weight)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetPortSchedulerDWRRWeight(xpsDevice_t devId, uint32_t portNum,
                                           uint32_t* weight)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetPortSchedulerSP(xpsDevice_t devId, uint32_t portNum,
                                   uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetPortSchedulerSP(xpsDevice_t devId, uint32_t portNum,
                                   uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSetPortSchedulerSPPriority(xpsDevice_t devId, uint32_t portNum,
                                           uint32_t priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosGetPortSchedulerSPPriority(xpsDevice_t devId, uint32_t portNum,
                                           uint32_t *priority)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}



XP_STATUS xpsQosSchedulerSetPortPacingInterval(xpsDevice_t devId,
                                               XP_SPEED speed, uint32_t numCycles)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSchedulerGetPortPacingInterval(xpsDevice_t devId,
                                               XP_SPEED speed, uint32_t *numCycles)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSchedulerSetH2PacingInterval(xpsDevice_t devId,
                                             uint32_t numCycles)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSchedulerGetH2PacingInterval(xpsDevice_t devId,
                                             uint32_t *numCycles)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSchedulerSetH1PacingInterval(xpsDevice_t devId,
                                             uint32_t numCycles)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSchedulerGetH1PacingInterval(xpsDevice_t devId,
                                             uint32_t *numCycles)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSchedulerSetQueuePacingInterval(xpsDevice_t devId,
                                                uint32_t numCycles)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosSchedulerGetQueuePacingInterval(xpsDevice_t devId,
                                                uint32_t *numCycles)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*
 * XPS Egress Cos Map APIs
 */

XP_STATUS xpsQosSetEgressQosMapEn(xpsDevice_t devId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapAdd(xpsDevice_t devId, xpVif_t eVif, uint32_t tc,
                                uint32_t dp, xpEgressCosMapData_t map, xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapUpdate(xpsDevice_t devId, uint32_t index,
                                   xpEgressCosMapData_t map)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapGetIndex(xpsDevice_t devId, xpVif_t eVif,
                                     uint32_t tc, uint32_t dp, int *index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapGetEntry(xpsDevice_t devId, xpVif_t eVif,
                                     uint32_t tc, uint32_t dp, xpEgressCosMapData_t *map)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapGetEntryByIndex(xpsDevice_t devId, uint32_t index,
                                            xpEgressCosMapData_t *map)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapRemoveEntry(xpsDevice_t devId, xpVif_t eVif,
                                        uint32_t tc, uint32_t dp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapRemoveEntryByIndex(xpsDevice_t devId,
                                               uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapSetL2QosPriority(xpsDevice_t devId, uint32_t index,
                                             uint32_t pcpVal, uint32_t deiVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapSetL3QosPriority(xpsDevice_t devId, uint32_t index,
                                             uint32_t dscpVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapSetL2QosPriorityEn(xpsDevice_t devId,
                                               uint32_t index, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapSetL3QosPriorityEn(xpsDevice_t devId,
                                               uint32_t index, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapGetL2QosPriority(xpsDevice_t devId, uint32_t index,
                                             uint32_t *pcpVal, uint32_t *deiVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapGetL3QosPriority(xpsDevice_t devId, uint32_t index,
                                             uint32_t *dscpVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapGetL2QosPriorityEn(xpsDevice_t devId,
                                               uint32_t index, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapGetL3QosPriorityEn(xpsDevice_t devId,
                                               uint32_t index, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapFlushEntryByEgressVif(xpsDevice_t devId,
                                                  xpVif_t vif)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapFlushEntryByEgressVifWithShadowUpdate(
    xpsDevice_t devId, xpVif_t vif, uint8_t updateShadow)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
XP_STATUS xpsQosEgressQosMapSetRehashLevel(xpsDevice_t devId,
                                           uint8_t numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapGetRehashLevel(xpsDevice_t devId,
                                           uint8_t* numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsQosEgressQosMapClearBucketState(xpDevice_t devId,
                                             uint8_t tblCopyIdx)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
#ifdef __cplusplus
}
#endif

