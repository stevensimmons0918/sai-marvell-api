// xpsPacketTrakker.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 *   \file xpsPacketTrakker.h
 *   \brief This file contains API prototypes and type definitions
 *          for the XPS PTP Manager
 */

#ifndef _XPSPT_H_
#define _XPSPT_H_

#include "xpsInit.h"
#include "xpsEnums.h"
#include "xpsQos.h"
#include "xpsMirror.h"
#include "xpTypes.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief xpPacketTrakkerEventType
 */
typedef enum xpPacketTrakkerEventType
{
    IDLE = 0,
    LOW_MIRROR_THRESHOLD,
    HIGH_MIRROR_THRESHOLD,
    MAX_THRESHOLD
} xpPacketTrakkerEventType_t;

/**
 * \brief xpPacketTrakkerEvent packetTrakker_event_global_fifo in HW
 */
typedef struct xpPacketTrakkerEvent
{
    xpPacketTrakkerEventType_t eventType;    ///< 2 bit value in HW.
    uint32_t          queueNum;     ///< Only need 12 bits.
    uint64_t
    timestamp;    ///< This may start with a 16 bit "HostId" field.
} xpPacketTrakkerEvent_t;

typedef xpPacketTrakkerEvent xpsPacketTrakkerEvent;

/**
 *
 * \brief Initialize the XPS PacketTrakker Manager
 *
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerInit();

/**
 *
 * \brief API to initialize and add device speicific primitives
 *        in primitive manager(s).
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */

XP_STATUS xpsPacketTrakkerAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 *
 * \brief API to de-initialize and remove device speicific primitives
 *        in primitive manager(s).
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerRemoveDevice(xpsDevice_t devId);

/**
 *
 * \brief De-Initialize the PacketTrakker Manager
 *
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerDeInit();

/**
 *
 * \brief This API will read the maximum depth of a queue
 *        monitored by PacketTrakker
 *
 * In the case PacketTrakker features are enabled on a port/queue
 * combination, the Q OLD Counter memory will be repurposed to
 * maintain, via hysteresis, the max queue depth. This counter
 * will be cleared by software after reading as it is not a
 * Clear On Read counter (it's a dynamic counter that is used
 * for other AQM features such as WRED)
 *
 * The resolution of the count is in pages
 *
 * \param [in] deviceId
 * \param [in] port
 * \param [in] queue
 * \param [out] depth
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerReadMaxQueueDepth(xpsDevice_t deviceId,
                                            xpsPort_t port, uint32_t queue, uint32_t *depth);

/**
 *
 * \brief This API will reprogram the qmap table with a new
 *        port/queue combination for deflection queueing
 *
 * The idea behind this API is to provide a new port and queue
 * to deflect traffic on a congested queue to. We will achieve
 * this in software by reprogramming the qmap table with a new
 * absolute queue number corresponding to the new port and queue
 * combination in the location of the q map index
 *
 *
 * \param [in] deviceId
 * \param [in] port
 * \param [in] queue
 * \param [in] newPort
 * \param [in] newQueue
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerEnableDeflection(xpsDevice_t deviceId, xpsPort_t port,
                                           uint32_t queue, xpsPort_t newPort, uint32_t newQueue);

/**
 *
 * \brief This API will undo the qmap table programming made by
 *        deflection queueing
 *
 * This will restore the absolute queue number in its correct
 * location inside the qmap table
 *
 *
 * \param [in] deviceId
 * \param [in] port
 * \param [in] queue
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerDisableDeflection(xpsDevice_t deviceId,
                                            xpsPort_t port, uint32_t queue);

/**
 *
 * \brief API to configure a cookie session
 *
 * A cookie session, sends a token with a signature thorugh the
 * eq and dq pipes and is used to monitor the latency through
 * the queueing system
 *
 * 8 cookies can exist and be enabled at the same time, 1 cookie
 * per group of 16 10 gig ports
 *
 * Each session requires a port and queue combination as well as
 * a signature which is used by hardware to to record a running
 * max latency
 *
 * This API will return the session id associated with the port
 * and queue combination specified. If a session is currently
 * enabled, this API will return an error. To change this
 * configuration, the session must be disabled first
 *
 * When initially configuring a cookie session, the session will
 * be enabled by default
 *
 * \param [in] deviceId
 * \param [in] port
 * \param [in] queue
 * \param [in] signature
 * \param [out] sessionId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerConfigureCookieSession(xpsDevice_t deviceId,
                                                 xpsPort_t port, uint32_t queue, uint16_t signature, uint32_t *sessionId);

/**
 *
 * \brief API to explicitly enable a cookie session indicated by
 *        the session id
 *
 *
 * \param [in] deviceId
 * \param [in] sessionId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerEnableCookieSession(xpsDevice_t deviceId,
                                              uint32_t sessionId);

/**
 *
 * \brief API to explicitly disable a cookie session indicated
 *        by the session id
 *
 *
 * \param [in] deviceId
 * \param [in] sessionId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerDisableCookieSession(xpsDevice_t deviceId,
                                               uint32_t sessionId);

/**
 *
 * \brief API to read the total number of cookie tokens that
 *        have circulated during the time this session has been
 *        enabled
 *
 *
 * \param [in] deviceId
 * \param [in] sessionId
 * \param [out] cookieCnt
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerTotalCirculatedCookies(xpsDevice_t deviceId,
                                                 uint32_t sessionId, uint32_t *cookieCnt);

/**
 *
 * \brief API to clear the circulated cookie counter for a given
 *        session
 *
 *
 * \param [in] deviceId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerClearCirculatedCookieCount(xpsDevice_t deviceId);

/**
 *
 * \brief API to read the currently recorded maximum delay
 *        thorugh the eq/dq pipe as determined by the cookie
 *
 * This API will return the delay in nanoseconds
 *
 *
 * \param [in] deviceId
 * \param [in] sessionId
 * \param [out] delay
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerReadDelay(xpsDevice_t deviceId, uint32_t sessionId,
                                    uint64_t *delay);

/**
 *
 * \brief API to read the head of the PT event fifo
 *
 * This API will return the head of the PT event fifo. By
 * virtue of reading the head, the fifo will pop the event out
 * freeing up space for another event
 *
 *
 * \param [in] deviceId
 * \param [out] event
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerReadEvent(xpsDevice_t deviceId,
                                    xpsPacketTrakkerEvent *event);

/**
 *
 * \brief API to get the PT event fifo almost full threshold
 *
 * \param [in] deviceId
 * \param [out] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetEventAlmostFullThreshold(xpsDevice_t deviceId,
                                                      uint32_t *threshold);

/**
 *
 * \brief API to set the PT event fifo almost full threshold
 *
 * The PT Almost full fifo interrupt will be asserted if the
 * fifo depth crosses this threshold. The default threshold is
 * 12. The depth of the fifo is 16
 *
 * \param [in] deviceId
 * \param [in] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerSetEventAlmostFullThreshold(xpsDevice_t deviceId,
                                                      uint32_t threshold);

/**
* \public
* \brief This API will be used to enable PT features on an
*        existing profile
*
* This assumes that the profile id's are managed by the
* user of XDK
*
* The Low/High Watermarks and Max Threshold values expect page
* resolution
*
* \param [in] deviceId
* \param [in] profileId
* \param [in] highWm
* \param [in] lowWm
* \param [in] maxThreshold
*
* \return XP_STATUS
*/
XP_STATUS xpsPacketTrakkerSetQueueWmThreshold(xpDevice_t deviceId,
                                              uint32_t profileId, uint32_t lowWm, uint32_t highWm, uint32_t maxThreshold);

/**
 * \public
 * \brief This API will be used to get PT features on an
 *        existing profile
 *
 * This assumes that the profile id's are managed by the user of
 * XDK
 *
 * The Low/High Watermarks and Max Threshold values expect page
 * resolution
 *
 * \param [in] deviceId
 * \param [in] profileId
 * \param [out] highWm
 * \param [out] lowWm
 * \param [out] maxThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetQueueWmThreshold(xpDevice_t deviceId,
                                              uint32_t profileId, uint32_t* lowWm, uint32_t* highWm, uint32_t* maxThreshold);

/**
 * \public
 * \brief This API will apply a PT Q profile to a port/queue
 *        combination
 *
 * \param [in] deviceId
 * \param [in] port
 * \param [in] queue
 * \param [in] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerBindPacketTrakkerProfileToQueue(xpDevice_t deviceId,
                                                          xpPort_t port, uint32_t queue, uint32_t profileId);

/**
 * \public
 * \brief This API will get a PT Q profile for a port/queue
 *        combination
 *
 * \param [in] deviceId
 * \param [in] port
 * \param [in] queue
 * \param [out] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetPacketTrakkerProfileForQueue(xpDevice_t deviceId,
                                                          xpPort_t port, uint32_t queue, uint32_t* profileId);

/**
 * \public
 * \brief This API enables Queue Length tracking
 *
 * \param xpDevice_t [in] devId
 * \param uint32_t [in] profileId
 * \param uint32_t [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerEnableQueueLengthTracking(xpDevice_t devId,
                                                    uint32_t profileId, uint32_t enable);

/**
 * \public
 * \brief Enable Packet Trakker feature
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerEnablePacketTrakker(xpDevice_t devId,
                                              uint32_t enable);

/**
 * \public
 * \brief Get whether Packet Trakker feature is enable or not
 *
 * \param [in] xpDevice_t devId
 * \param [out] uint32_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetPacketTrakkerEnable(xpDevice_t devId,
                                                 uint32_t* enable);

/**
 * \public
 * \brief Enable mirror mode for controlling mirroring for
 *        global states
 *
 * - Global page threshold
 * - Global multicast page threshold
 * - Shared Pool <0-7> Watermark
 *
 * \param [in] xpDevice_t devId
 * \param [in] xpPtGlobalState_e state
 * \param [in] xpPtMirrorModes_e mirrorMode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerEnableGlobalStateMirrorMode(xpDevice_t devId,
                                                      xpPtGlobalState_e state, xpPtMirrorModes_e mirrorMode);

/**
 * \public
 * \brief Get mirror mode configuration for controlling
 *        mirroring for global states
 *
 * - Global page threshold
 * - Global multicast page threshold
 * - Shared Pool <0-7> Watermark
 *
 * \param [in] xpDevice_t devId
 * \param [in] xpPtGlobalState_e state
 * \param [out] xpPtMirrorModes_e mirrorMode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetGlobalStateMirrorMode(xpDevice_t devId,
                                                   xpPtGlobalState_e state, xpPtMirrorModes_e* mirrorMode);

/**
 * \public
 * \brief Enable mirror mode for controlling mirroring for
 *        port states
 *
 * - Port max page threshold
 * - Per Port Queue WM
 * - Per Port Queue Dynamic Threshold
 * - Per Port Queue Average Length WM
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] xpPtPortState_e state
 * \param [in] xpPtMirrorModes_e mirrorMode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerEnablePortStateMirrorMode(xpDevice_t devId,
                                                    uint32_t portNum, xpPtPortState_e state, xpPtMirrorModes_e mirrorMode);

/**
 * \public
 * \brief Get mirror mode configuration for controlling
 *        mirroring for port states
 *
 * - Port max page threshold
 * - Per Port Queue WM
 * - Per Port Queue Dynamic Threshold
 * - Per Port Queue Average Length WM
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] xpPtPortState_e state
 * \param [out] xpPtMirrorModes_e mirrorMode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetPortStateMirrorMode(xpDevice_t devId,
                                                 uint32_t portNum, xpPtPortState_e state, xpPtMirrorModes_e* mirrorMode);

/**
 * \public
 * \brief Enable periodic packet mirroring for exporting global
 *        and port-queue states
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerEnablePeriodicMirror(xpDevice_t devId,
                                               uint32_t portNum, uint32_t enable);

/**
 * \public
 * \brief Check if periodic mirroring is enable or not
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [out] uint32_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetPeriodicMirrorEnable(xpDevice_t devId,
                                                  uint32_t portNum, uint32_t* enable);

/**
 * \public
 * \brief Configure mirror period in msecs
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t mirrorPeriod
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerConfigureMirrorPeriod(xpDevice_t devId,
                                                uint32_t portNum, uint32_t mirrorPeriod);

/**
 * \public
 * \brief Get mirror period configuration
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [out] uint32_t mirrorPeriod
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetMirrorPeriod(xpDevice_t devId, uint32_t portNum,
                                          uint32_t* mirrorPeriod);

/**
 * \public
 * \brief Enable Global shaper
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerEnableGlobalShaper(xpDevice_t devId, uint32_t enable);

/**
 * \public
 * \brief Check if global shaper is enable or not
 *
 * \param [in] xpDevice_t devId
 * \param [out] uint32_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetGlobalShaperEnable(xpDevice_t devId,
                                                uint32_t* enable);

/**
 * \public
 * \brief Enable per port shaper
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerEnablePortShaper(xpDevice_t devId, uint32_t portNum,
                                           uint32_t enable);

/**
 * \public
 * \brief Check if port shaper is enable or not
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [out] uint32_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetPortShaperEnable(xpDevice_t devId,
                                              uint32_t portNum, uint32_t* enable);

/**
 * \public
 * \brief Configure global shaper parameters
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t rateKpps
 * \param [in] uint32_t maxBucketSize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerConfigureGlobalShaper(xpDevice_t devId,
                                                uint32_t rateKpps, uint32_t maxBucketSize);

/**
 * \public
 * \brief Get global shaper configuration
 *
 * \param [in] xpDevice_t devId
 * \param [out] uint32_t rateKpps
 * \param [out] uint32_t maxBucketSize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetGlobalShaperConfiguration(xpDevice_t devId,
                                                       uint32_t* rateKpps, uint32_t* maxBucketSize);

/**
 * \public
 * \brief Configure port shaper parameters
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t rateKpps
 * \param [in] uint32_t maxBucketSize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerConfigurePortShaper(xpDevice_t devId,
                                              uint32_t portNum, uint32_t rateKpps, uint32_t maxBucketSize);

/**
 * \public
 * \brief Get port shaper configuration
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [out] uint32_t rateKpps
 * \param [out] uint32_t maxBucketSize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetPortShaperConfiguration(xpDevice_t devId,
                                                     uint32_t portNum, uint32_t* rateKpps, uint32_t* maxBucketSize);

/**
 * \public
 * \brief Enable global aging of states
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerEnableGlobalAging(xpDevice_t devId, uint32_t enable);

/**
 * \public
 * \brief Check if aging is enabled or not
 *
 * \param [in] xpDevice_t devId
 * \param [out] uint32_t& enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetGlobalAgingEnable(xpDevice_t devId,
                                               uint32_t* enable);

/**
 * \public
 * \brief Configure aging peroid
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t agingPeriod
 * \param [in] uint32_t agingLimit
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerConfigureGlobalAgingPeriod(xpDevice_t devId,
                                                     uint32_t agingPeriod, uint32_t agingLimit);

/**
 * \public
 * \brief Get aging period configurations
 *
 * \param [in] xpDevice_t devId
 * \param [out] uint32_t agingPeriod
 * \param [out] uint32_t agingLimit
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetGlobalAgingPeriod(xpDevice_t devId,
                                               uint32_t* agingPeriod, uint32_t* agingLimit);

/**
 * \public
 * \brief Set port max threshold in pages
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerSetPortMaxThreshold(xpDevice_t devId,
                                              uint32_t portNum, uint32_t threshold);

/**
 * \public
 * \brief Get port max threshold in pages
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t& threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetPortMaxThreshold(xpDevice_t devId,
                                              uint32_t portNum, uint32_t* threshold);
/**
 * \public
 * \brief Set shared pool watermark threshold
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t sharedPoolId
 * \param [in] uint32_t lowWmThreshold
 * \param [in] uint32_t highWmThreshold
 * \param [in] uint32_t maxThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerSetPoolWmThreshold(xpDevice_t devId,
                                             uint32_t sharedPoolId, uint32_t lowWmThreshold, uint32_t highWmThreshold,
                                             uint32_t maxThreshold);
/**
 * \public
 * \brief Get shared pool watermark threshold
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t sharedPoolId
 * \param [out] uint32_t lowWmThreshold
 * \param [out] uint32_t highWmThreshold
 * \param [out] uint32_t maxThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetPoolWmThreshold(xpDevice_t devId,
                                             uint32_t sharedPoolId, uint32_t* lowWmThreshold, uint32_t* highWmThreshold,
                                             uint32_t* maxThreshold);
/**
 * \public
 * \brief Configure global page threshold
 *
 * \param [in] devId
 * \param [in] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerSetGlobalPageThreshold(xpsDevice_t devId,
                                                 uint32_t threshold);
/**
 * \public
 * \brief Get configured global page threshold
 *
 * This API returns the configured global page threshold
 *
 * \param [in] devId
 * \param [out] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetGlobalPageThreshold(xpsDevice_t devId,
                                                 uint32_t *threshold);
/**
 * \public
 * \brief Configure global multicast page threshold
 *
 * \param [in] devId
 * \param [in] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerSetMulticastPageThreshold(xpsDevice_t devId,
                                                    uint32_t threshold);
/**
 * \public
 * \brief get configured global multicast page threshold
 *
 * This API will return the configured multicast page
 * threshold
 *
 * \param [in] devId
 * \param [out] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetMulticastPageThreshold(xpsDevice_t devId,
                                                    uint32_t *threshold);
/**
 * \brief This API explicitly creates an AQM Q profile from a pool of free AQM Q Profiles
 *
 * \param [in] devId
 * \param [out] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerCreateAqmQProfile(xpsDevice_t devId,
                                            uint32_t *profileId);
/**
 * \brief Bind the AQM Queue profile for a queue
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [in] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerBindAqmQProfileToQueue(xpsDevice_t devId,
                                                 xpsPort_t portNum, uint32_t queueNum, uint32_t profileId);
/**
 * \brief Get the AQM Queue profile for a queue
 *
 * This API retrieves the AQM Q profile id that is bound to a
 * particular queue
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetAqmQProfileForQueue(xpsDevice_t devId,
                                                 xpsPort_t portNum, uint32_t queueNum, uint32_t *profileId);
/**
 * \brief Configure Average WM Threshold
 *
 * This API will configure WRED parameters into an AQM Q Profile
 *
 * The WRED min and max thresholds are in pages
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [in] minThreshold
 * \param [in] maxThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerSetAvgQueueWmThreshold(xpsDevice_t devId,
                                                 uint32_t profileId, uint32_t minThreshold, uint32_t maxThreshold);
/**
 * \brief Get configured WM Threshold on an AQM Q Profile
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [out] minThreshold
 * \param [out] maxThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetAvgQueueWmThreshold(xpsDevice_t devId,
                                                 uint32_t profileId, uint32_t *minThreshold, uint32_t *maxThreshold);

/**
 * \public
 * \brief update start queue number
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t ptgNum
 * \param [in] uint32_t queueNum
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerUpdatePktTrakkerStartQueueNum(xpDevice_t devId,
                                                        uint32_t portNum, uint32_t queueNum);

/**
 * \public
 * \brief Get start queue number
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t ptgNum
 * \param [out] uint32_t& queueNum
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetPktTrakkerStartQueueNum(xpDevice_t devId,
                                                     uint32_t portNum, uint32_t* queueNum);

/**
 * \brief Add an PT analyzer interface to an existing analyzer session.
 * Allocates an mdt node for the analyzer interface and maintains this new state
 * for a scope.
 *
 * \param [in] devId
 * \param [in] analyzerIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerAddAnalyzerInterface(xpsDevice_t devId,
                                               xpsInterfaceId_t analyzerIntfId);

/**
 * \brief Remove a particular PT analyzer interface from an analyzer session for a particular device.
 *
 * \param [in] devId
 * \param [in] analyzerIntfId
 *
 */
XP_STATUS xpsPacketTrakkerRemoveAnalyzerInterface(xpsDevice_t devId,
                                                  xpsInterfaceId_t analyzerIntfId);

/**
 * \public
 * \brief This API will be used to set the user defined switchId to identify the device
 *
 * \param [in] deviceId
 * \param [in] switchId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerSetSwitchIdentifier(xpsDevice_t devId,
                                              uint32_t switchId);

/**
 * \public
 * \brief This API will be used to get the switchId set by the user
 *
 * \param [in] deviceId
 * \param [out] switchId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPacketTrakkerGetSwitchIdentifier(xpsDevice_t devId,
                                              uint32_t *switchId);


#ifdef __cplusplus
}
#endif

#endif //_XPSPT_H_
