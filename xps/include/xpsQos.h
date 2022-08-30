// xpsQos.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

/**
 * \file xpsQos.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Qos Manager
 *
 * For more detailed descriptions of these features and their implementations
 * please see the following:
 *
 * \see xpAqmMgr.h
 * \see xpPortIngressQosMgr.h
 * \see xpQueueCounterMgr.h
 * \see xpSchedulerMgr.h
 * \see xpShaperMgr.h
 * \see xpCpuStormControlMgr.h
 * \see xpFlowControlMgr.h*
 */

#ifndef _xpsQos_h_
#define _xpsQos_h_

#include "xpTypes.h"
#include "xpEnums.h"
#include "xpsEnums.h"
#include "openXpsQos.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XP_MAX_NUM_QUEUES_PER_PORT            (32)

/**
 * \brief API that initializes the XPS Qos Mgr
 *
 * This API will register for the state databases that are
 * needed by the Qos Mgr code base for a scope
 *
 *\param [in] scopeId
 * \return XP_STATUS
 */
XP_STATUS xpsQosInitScope(xpsScope_t scopeId);

/**
 * \brief API to De-Init the XPS Qos Mgr
 *
 * This API will Deregister all state databases for Qos Mgr for a scope
 *
 *\param [in] scopeId
 * \return XP_STATUS
 */
XP_STATUS xpsQosDeInitScope(xpsScope_t scopeId);

/**
 * \brief API to perform Add Device operations for Qos Mgr
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief API to perform Remove Device operations for Qos Mgr
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosRemoveDevice(xpsDevice_t devId);

/*
AQM Profile Manager
*/

/**
 * \brief This API explicitly creates an AQM Q profile from a
 *        pool of free AQM Q Profiles
 *
 * The Traffic Manager system of XPA has a notion of AQM Q Profiles
 * which are used to configure Active Queue Management on
 * queues
 *
 * Active Queue Management (AQM for short) is the method by which
 * queues and other nodes in a hierarchy are allocated resources,
 * and congestion characterized and dealt with
 *
 * The XPS layer provides methods to allocate a new AQM Q Profile,
 * bind a new AQM Q Profile to a Q and explictly delete an AQM Q
 * Profile from use
 *
 * The XPS layer also provides methods to configure existing AQM
 * Q profiles for various AQM Features
 *
 * \see xpsQosAqmDeleteAqmQProfile
 * \see xpsQosAqmBindAqmQProfileToQueue
 *
 *
 * \param [in] devId
 * \param [out] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmCreateAqmQProfile(xpsDevice_t devId, uint32_t *profileId);

/**
 * \brief This API explicitly deletes and AQM Q profile and
 *        returns the id back into a free pool of AQM Q Profiles
 *
 * Please note that to use this API, it is expected that this
 * AQM Q Profile is not currently bound to any queues. To unbind
 * a Q from a profile, simply bind another AQM Q Profile to it
 *
 *
 * \param [in] devId
 * \param [in] profileId
 *
 * \return XP_STATUS - error if there is no more space to create a profile
 */
XP_STATUS xpsQosAqmDeleteAqmQProfile(xpsDevice_t devId, uint32_t profileId);

/**
 * \brief This API explicitly creates an AQM profile from a pool
 *        of free AQM Q Profiles
 *
 * The Traffic Manager system of XPA has a notion of AQM
 * Profiles which are used to configure Active Queue Management
 * on ports and other hierarchies
 *
 * Active Queue Management (AQM for short) is the method by which
 * ports and other nodes in a hierarchy are allocated resources,
 * and congestion characterized and dealt with
 *
 * The XPS layer provides methods to allocate a new AQM Profile,
 * bind a new AQM Profile to a Port and explictly delete an AQM
 * Profile from use
 *
 * The XPS layer also provides methods to configure existing AQM
 * profiles for various AQM Features
 *
 * \param [in] devId
 * \param [out] profileId
 *
 * \return XP_STATUS - error if there is no more space to create a profile
 */
XP_STATUS xpsQosAqmCreateAqmProfile(xpsDevice_t devId, uint32_t *profileId);

/**
 * \brief This API explicitly deletes and AQM profile and
 *        returns the id back into a free pool of AQM Profiles
 *
 * Please note that to use this API, it is expected that this
 * AQM Profile is not currently bound to any queues. To unbind a
 * from a profile, simply bind another AQM Profile to it
 *
 *
 * \param [in] devId
 * \param [in] profileId
 *
 *
 * \return XP_STATUS - error if there is no more space to create a profile
 */
XP_STATUS xpsQosAqmDeleteAqmProfile(xpsDevice_t devId, uint32_t profileId);

/*
 * Enable/Disable the enqueue of a packet
 */

/**
 * \brief Enable or Disable packets being enqueued
 *
 * This API provides the ability to enable or disable a queue's
 * enqueue functionality. Packets destined to a queue whose
 * enqueue is disabled will be dropped
 *
 * This control does not impact the dequeue functionality of a
 * queue. This is controlled separately
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmSetEnqueueEnable(xpDevice_t devId, xpPort_t devPort,
                                    uint32_t queueNum, uint32_t enable);

/**
 * \brief Get whether or not a queue's enqueue is enabled
 *
 * This API provides the ability to retrieve whether or not a
 * queue's enqueue functionality is enabled. Packets destined to
 * a queue whose enqueue is disabled will be dropped
 *
 * This control does not impact the dequeue functionality of a
 * queue. This is controlled separately
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetEnqueueEnable(xpDevice_t devId, xpPort_t devPort,
                                    uint32_t queueNum, uint32_t *enable);

/*
 * Bind profiles to port and queue
 */

/**
 * \brief Bind the AQM Queue profile for a queue
 *
 * The XPA Traffic Manager subsystem uses a notion of AQM Q
 * profiles which dictate how Active Queue Management takes
 * place on a queue
 *
 * Active Queue Management (AQM for short) is a series of
 * methods used to manage resources and congesiton at egress
 * queues. It is the fundamental method of allocating packet
 * memory to queues and controlling of congestion by virtue of
 * this allocation
 *
 * This API will be used to create a binding of a cratfted AQM Q
 * profile with a queue in the Traffic Manager
 *
 * Please note that Profiles can be bound to multiple queues and
 * each queue that this profile is bound contains the exact same
 * configuration of the Active Queue Management scheme designed
 * by the QOS Implementer
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [in] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmBindAqmQProfileToQueue(xpsDevice_t devId, xpsPort_t devPort,
                                          uint32_t queueNum, uint32_t profileId);

/**
 * \brief Bind the AQM Queue profile for a color-aware queue
 *
 * The XPA Traffic Manager subsystem uses a notion of AQM Q
 * profiles which dictate how Active Queue Management takes
 * place on a queue
 *
 * Active Queue Management (AQM for short) is a series of
 * methods used to manage resources and congesiton at egress
 * queues. It is the fundamental method of allocating packet
 * memory to queues and controlling of congestion by virtue of
 * this allocation
 *
 * This API will be used to create a binding of a cratfted AQM Q
 * profile with a color-aware queue in the Traffic Manager
 *
 * Please note that Profiles can be bound to multiple queues and
 * each queue that this profile is bound contains the exact same
 * configuration of the Active Queue Management scheme designed
 * by the QOS Implementer
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [in] color
 * \param [in] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmBindAqmQProfileToColorAwareQueue(xpsDevice_t devId,
                                                    xpsPort_t devPort, uint32_t queueNum, xpQosColor_e color, uint32_t profileId);

/**
 * \brief Bind the AQM profile for a port
 *
 * The XPA Traffic Manager subsystem uses a notion of AQM
 * profiles which dictate how Active Queue Management takes
 * place on a port and other hierarchies
 *
 * Active Queue Management (AQM for short) is a series of
 * methods used to manage resources and congesiton at egress
 * queues and other nodes. It is the fundamental method of
 * allocating packet memory to queues and controlling of
 * congestion by virtue of this allocation
 *
 * Please note that Profiles can be bound to mulitple ports and
 * hierarchies and each node that this profile is bound to
 * contains the exact same configuration of the Active Queue
 * Management scheme designed by the QOS Implementer
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmBindAqmProfileToPort(xpsDevice_t devId, xpsPort_t devPort,
                                        uint32_t profileId);

/*
 * Get profiles for port and queue
 */

/**
 * \brief Get the AQM Queue profile for a queue
 *
 * This API retrieves the AQM Q profile id that is bound to a
 * particular queue
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [in] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetAqmQProfileForQueue(xpsDevice_t devId, xpsPort_t devPort,
                                          uint32_t queueNum, uint32_t *profileId);

/**
 * \brief Get the AQM Queue profile for a queue
 *
 * This API retrieves the AQM Q profile id that is bound to a
 * particular color-aware queue
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [in] color
 * \param [in] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetAqmQProfileForColorAwareQueue(xpsDevice_t devId,
                                                    xpsPort_t devPort, uint32_t queueNum, xpQosColor_e color, uint32_t *profileId);

/**
 * \brief Get the AQM profile for a port
 *
 * This API retreives the AQM profile id that is bound to a
 * particular port
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [in] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetAqmProfileForPort(xpsDevice_t devId, xpsPort_t devPort,
                                        uint32_t queueNum, uint32_t *profileId);

/*
 * WRED APIs
 */

/**
 * \brief Configure WRED on an AQM Q Profile
 *
 * Weighted Random Early Detection (WRED for short) is an Active
 * Queue Management Scheme that is used to smoothen out
 * congestion due to a bursty flow over a period of time
 *
 * This smoothening happens by virtue of detecting congestion
 * "early" in other words once a queue starts to build, the
 * Queuing system will start to randomly drop or mark packets at
 * a well known rate. This rate grows linerarly with the average
 * queue length
 *
 * What this means is that as your queue builds up over time,
 * your likelihood of dropping or marking  a packet increases up
 * until you cross a max threshold. Once you cross this max
 * threshold, you will always either drop or mark your packets
 * with ECN
 *
 * Weighted RED is different from regular RED in that a weight
 * can be applied on the calculation of the average queue length
 * at the time of AQM. The calculation of the average queue
 * length generally follows this formula:
 *
 *      * avg q len = o * (1 - 2^-weight) + c * (2^-weight)
 *
 * Where o is the old queue length and c is the current q
 * length. If your weight is a large number you will bias the
 * new average q length calculation in favor of the old length
 * and vice versa
 *
 * Weighted RED allows the user a great amount of control over
 * this graph:
 *
 * WRED Mark/Drop Curve
 *
 *  Mark/Drop P|             ____________
 *           1 |            |
 *             |            |
 *             |            |
 *       P max |           /
 *             |         /
 *             |       /
 *            _|_____/______|____________
 *             |    min   max       avg q len
 *
 * This API will configure WRED parameters into an AQM Q Profile
 *
 * The WRED min and max thresholds are in pages
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [in] minThreshold
 * \param [in] maxThreshold
 * \param [in] weight
 * \param [in] maxDropProb
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmConfigureWred(xpsDevice_t devId, uint32_t profileId,
                                 uint32_t minThreshold, uint32_t maxThreshold, uint32_t weight,
                                 uint32_t maxDropProb);

/**
 * \brief Get configured WRED parameters on an AQM Q Profile
 *
 *
 * \param [in] devId
 * \param [out] profileId
 * \param [out] minThreshold
 * \param [out] maxThreshold
 * \param [out] avgQLengthWeight
 * \param [out] maxDropProb
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetWredConfiguration(xpsDevice_t devId, uint32_t profileId,
                                        uint32_t* minThreshold, uint32_t* maxThreshold, uint32_t* avgQLengthWeight,
                                        uint32_t* maxDropProb);

/**
 * \brief Configure WRED modes.
 *
 * There are three distinct WRED modes supported in XPA:
 *      * Tail Drop only / Disable WRED
 *      * WRED Drop
 *      * WRED Mark ECN
 *
 * This API will configure one of these three modes into an AQM
 * Q Profile
 *
 * The default configuration of WRED Mode is always disable,
 * also known as tail drop only
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [in] wredMode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmConfigureWredMode(xpsDevice_t devId, uint32_t profileId,
                                     xpWredMode_e wredMode);

/**
 * \brief Get WRED mode
 *
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [out] wredMode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetWredMode(xpsDevice_t devId, uint32_t profileId,
                               xpWredMode_e* wredMode);

/**
 * \brief Display WRED parameters like min/max threshold,
 *        weight, mark drop probability.
 *
 *
 * \param [in] devId
 * \param [in] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmDisplayWredParams(xpsDevice_t devId, uint32_t profileId);

/*
 * DCTCP APIs
 */

/**
 * \brief Configure Queue DCTCP mark threshold
 *
 * Data Center TCP is a feature used by TCP to indicate
 * congestion while minimizing latency in a switch
 *
 * DCTCP allows for shallow queue depth and marks ECN when
 * queue depth builds, the idea here is to send ECN to the
 * sender or receiver of a TCP session and notify the TCP stack
 * to shrink or expand the TCP Congestion Window according to
 * queue utilization on a switch in the network
 *
 * Since ECN marking is always forward facing, DCTCP institutes
 * a protocol to notify the sender to shrink it's congestion
 * window there by reducing the amount of traffic in the system.
 * This is useful to reduce overall packet latency within a
 * network because the sender and receiver have a much better
 * picture of congestion
 *
 * IE: When a low latency queue is starting to build, which
 * would add average latency to a TCP session, the sender will
 * be notified by the received implementing DCTCP to shrink its
 * window and by virtue of a smaller window reduces the number
 * of packets sent
 *
 * ECN is used as a realtime notification status of a switch in
 * the network's congestion
 *
 * XPA's implementation of DCTCP marking allows for a user to
 * configure a low mark threshold for a queue independent of a
 * queue's resource allocation. If a queue builds up past it's
 * DCTCP Mark Threshold, ECN will be marked on subsequent
 * packets
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [in] markThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmConfigureQueueDctcpMarkThreshold(xpsDevice_t devId,
                                                    uint32_t profileId, uint32_t markThreshold);

/**
 * \brief Get configured Queue DCTCP mark threshold
 *
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [out] markThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetQueueDctcpMarkThreshold(xpsDevice_t devId,
                                              uint32_t profileId, uint32_t* markThreshold);

/**
 * \brief Enable/Disable Queue DCTCP Marking of ECN
 *
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmSetQueueDctcpEnable(xpsDevice_t devId, uint32_t profileId,
                                       uint32_t enable);

/**
 * \brief Get enable/disable Queue DCTCP Marking of ECN
 *
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetQueueDctcpEnable(xpsDevice_t devId, uint32_t profileId,
                                       uint32_t* enable);

/**
 * \brief Display Queue DCTCP parameters
 *
 *
 * \param [in] devId
 * \param [in] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmDisplayQueueDctcpParams(xpsDevice_t devId,
                                           uint32_t profileId);

/**
 * \brief Configure Port DCTCP mark threshold
 *
 * Data Center TCP is a feature used by TCP to indicate
 * congestion while minimizing latency in a switch
 *
 * DCTCP allows for shallow queue depth and marks ECN when
 * queue depth builds, the idea here is to send ECN to the
 * sender or receiver of a TCP session and notify the TCP stack
 * to shrink or expand the TCP Congestion Window according to
 * queue utilization on a switch in the network
 *
 * Since ECN marking is always forward facing, DCTCP institutes
 * a protocol to notify the sender to shrink it's congestion
 * window there by reducing the amount of traffic in the system.
 * This is useful to reduce overall packet latency within a
 * network because the sender and receiver have a much better
 * picture of congestion
 *
 * IE: When a low latency queue is starting to build, which
 * would add average latency to a TCP session, the sender will
 * be notified by the received implementing DCTCP to shrink its
 * window and by virtue of a smaller window reduces the number
 * of packets sent
 *
 * ECN is used as a realtime notification status of a switch in
 * the network's congestion
 *
 * XPA's implementation of DCTCP marking allows for a user to
 * configure a low mark threshold for a queue independent of a
 * queue's resource allocation. If a queue builds up past it's
 * DCTCP Mark Threshold, ECN will be marked on subsequent
 * packets
 *
 * XPA supports DCTCP marking and allocation on a queue and a
 * port level. If a port detects congestion based off of it's
 * DCTCP mark threshold, all packets egressing from that queue
 * will be marked with ECN
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [in] markThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmConfigurePortDctcpMarkThreshold(xpsDevice_t devId,
                                                   uint32_t profileId, uint32_t markThreshold);

/**
 * \brief Get configured Port DCTCP mark threshold
 *
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [out] markThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetPortDctcpMarkThreshold(xpsDevice_t devId,
                                             uint32_t profileId, uint32_t* markThreshold);

/**
 * \brief Enable/disable Port DCTCP
 *
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmSetPortDctcpEnable(xpsDevice_t devId, uint32_t profileId,
                                      uint32_t enable);

/**
 * \brief Get enable/disable Port DCTCP
 *
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetPortDctcpEnable(xpsDevice_t devId, uint32_t profileId,
                                      uint32_t* enable);

/**
 * \brief Display Port DCTCP parameters
 *
 *
 * \param [in] devId
 * \param [in] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmDisplayPortDctcpParams(xpsDevice_t devId,
                                          uint32_t profileId);

/*
 * Tail Drop APIs
 */

/**
 * \brief Configure Queue tail drop max threshold in pages.
 *
 * Tail drop is the fundamental method of resource allocation
 * used in switches today. It relies on instantaneous feedback
 * of queue utilization to dictate whether or not a packet
 * should be enqueued
 *
 * For example, if a tail drop threshold is set to 120 pages,
 * and the queue at time T currently has 115 pages enqueued. At
 * time T+1 a packet of size 6 pages comes in and requests
 * whether or not it can be enqueued, the Tail Drop threshold
 * will be crossed for this packet and AQM should not allow for
 * it by enqueued
 *
 * Unlike WRED which uses historical data of queuing, Tail Drop
 * uses only instantaneous information
 *
 * This Tail Drop threshold is in pages, which is a byte
 * resolution, there is another tail drop threshold in packets.
 * Both can exist at the same time and either one can decide to
 * drop a packet
 *
 * This API is used to configure the static page tail drop
 * threshold for devices that have AQM Q profile based static
 * thresholding
 *
 * Devices that have per queue guaranteed thresholds will use
 *
 * \see xpsQosAqmConfigureQueuePageTailDropThreshold
 *
 * Devices that have color-aware queue based guaranteed
 * thresholds will use
 *
 * \see xpsQosAqmConfigureQueuePageTailDropThresholdForColor
 *
 * In addition, this is the API that will need to be used to
 * allocate dedicated tokens per queue
 *
 * \see xpsQosAqmConfigureQueuePacketTailDropThreshold
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [in] lengthMaxThreshold
 *
 * \return XP_STATUS
     */
XP_STATUS xpsQosAqmConfigureQueuePageTailDropThresholdForProfile(
    xpsDevice_t devId, uint32_t profileId, uint32_t lengthMaxThreshold);

/**
 * \brief Configure Queue tail drop max threshold in pages.
 *
 * Tail drop is the fundamental method of resource allocation
 * used in switches today. It relies on instantaneous feedback
 * of queue utilization to dictate whether or not a packet
 * should be enqueued
 *
 * For example, if a tail drop threshold is set to 120 pages,
 * and the queue at time T currently has 115 pages enqueued. At
 * time T+1 a packet of size 6 pages comes in and requests
 * whether or not it can be enqueued, the Tail Drop threshold
 * will be crossed for this packet and AQM should not allow for
 * it by enqueued
 *
 * Unlike WRED which uses historical data of queuing, Tail Drop
 * uses only instantaneous information
 *
 * This Tail Drop threshold is in pages, which is a byte
 * resolution, there is another tail drop threshold in packets.
 * Both can exist at the same time and either one can decide to
 * drop a packet
 *
 * This API is used to configure the static page tail drop
 * threshold for devices that have per queue based static
 * thresholding
 *
 * Devices that have AQM Q Profile based guaranteed thresholds
 * will use
 *
 * \see xpsQosAqmConfigureQueuePageTailDropThresholdForProfile
 *
 * Devices that have color-aware queue based guaranteed
 * thresholds will use
 *
 * \see xpsQosAqmConfigureQueuePageTailDropThresholdForColor
 *
 * In addition, this is the API that will need to be used to
 * allocate dedicated tokens per queue
 *
 * \see xpsQosAqmConfigureQueuePacketTailDropThreshold
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [in] lengthMaxThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmConfigureQueuePageTailDropThreshold(xpsDevice_t devId,
                                                       xpsPort_t devPort, uint32_t queueNum, uint32_t lengthMaxThreshold);

/**
 * \brief Configure Queue color-aware tail drop max threshold
 *        in pages.
 *
 * Tail drop is the fundamental method of resource allocation
 * used in switches today. It relies on instantaneous feedback
 * of queue utilization to dictate whether or not a packet
 * should be enqueued
 *
 * For example, if a tail drop threshold is set to 120 pages,
 * and the queue at time T currently has 115 pages enqueued. At
 * time T+1 a packet of size 6 pages comes in and requests
 * whether or not it can be enqueued, the Tail Drop threshold
 * will be crossed for this packet and AQM should not allow for
 * it by enqueued
 *
 * Unlike WRED which uses historical data of queuing, Tail Drop
 * uses only instantaneous information
 *
 * This Tail Drop threshold is in pages, which is a byte
 * resolution, there is another tail drop threshold in packets.
 * Both can exist at the same time and either one can decide to
 * drop a packet
 *
 * This API is used to configure the static color-aware page tail drop
 * threshold for devices that have color-aware queue based static
 * thresholding
 *
 * Devices that have AQM Q Profile based guaranteed thresholds
 * will use
 *
 * \see xpsQosAqmConfigureQueuePageTailDropThresholdForProfile
 *
 * Devices that have color-unaware based guaranteed thresholds
 * will use
 *
 * \see xpsQosAqmConfigureQueuePageTailDropThreshold
 *
 * In addition, this is the API that will need to be used to
 * allocate dedicated tokens per queue
 *
 * \see xpsQosAqmConfigureQueuePacketTailDropThreshold
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [in] color
 * \param [in] lengthMaxThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmConfigureQueuePageTailDropThresholdForColor(
    xpsDevice_t devId, xpsPort_t devPort, uint32_t queueNum, xpQosColor_e color,
    uint32_t lengthMaxThreshold);

/**
 * \brief Get configured Queue tail drop max threshold in pages.
 *
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [out] lengthMaxThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetQueuePageTailDropThresholdForProfile(xpsDevice_t devId,
                                                           uint32_t profileId, uint32_t* lengthMaxThreshold);

/**
 * \brief Get configured Queue tail drop max threhsold in pages
 *        per queue
 *
 * This API is only supported for devices that have per queue
 * static thresholds (guaranteed buffer)
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queuNum
 * \param [out] lengthMaxThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetQueuePageTailDropThreshold(xpsDevice_t devId,
                                                 xpsPort_t devPort, uint32_t queueNum, uint32_t* lengthMaxThreshold);

/**
 * \brief Get configured Queue color aware tail drop max threhsold
 *        in pages per queue
 *
 * This API is only supported for devices that have per queue color
 * aware static thresholds (guaranteed buffer)
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queuNum
 * \param [in] color
 * \param [out] lengthMaxThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetQueuePageTailDropThresholdForColor(xpsDevice_t devId,
                                                         xpsPort_t devPort, uint32_t queueNum, xpQosColor_e color,
                                                         uint32_t* lengthMaxThreshold);

/**
 * \brief Enable profile based tail drop threshold
 *
 * In some devices, tail drop threshold supported per [port,
 * queue] cannot be set more than 32k. For PFC, we need to set
 * queue page tail drop threshold to max 96k for which we can
 * use the lengthMaxThreshold in AQM profile. This API enables
 * to use AQM Q profile based tail drop threshold.
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t profileId
 * \param [in] uint32_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmUseProfileBasedTailDropThreshold(xpsDevice_t devId,
                                                    uint32_t profileId, uint32_t enable);

/**
 * \brief Get profile based tail drop threshold enable
 *
 * In some devices, tail drop threshold supported per [port,
 * queue] cannot be set more than 32k. For PFC, we need to set
 * queue page tail drop threshold to max 96k for which we can
 * use the lengthMaxThreshold in AQM profile. This API enables
 * to use AQM Q profile based tail drop threshold.
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t profileId
 * \param [out] uint32_t *enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetProfileBasedTailDropThresholdEnable(xpsDevice_t devId,
                                                          uint32_t profileId, uint32_t *enable);

/**
 * \brief Display queue tail drop max threshold in pages
 *
 *
 * \param [in] devId
 * \param [in] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmDisplayQueueTailDropParams(xpsDevice_t devId,
                                              uint32_t profileId);

/**
 * \brief Configure Port tail drop max threshold in pages.
 *
 * Tail drop is the fundamental method of resource allocation
 * used in switches today. It relies on instantaneous feedback
 * of port utilization to dictate whether or not a packet
 * should be enqueued
 *
 * For example, if a tail drop threshold is set to 120 pages,
 * and the port at time T currently has 115 pages enqueued. At
 * time T+1 a packet of size 6 pages comes in and requests
 * whether or not it can be enqueued, the Tail Drop threshold
 * will be crossed for this packet and AQM should not allow for
 * it by enqueued
 *
 * Unlike WRED which uses historical data of queuing, Tail Drop
 * uses only instantaneous information
 *
 * This Tail Drop threshold is in pages, which is a byte
 * resolution, there is another tail drop threshold in packets.
 * Both can exist at the same time and either one can decide to
 * drop a packet
 *
 * The port tail drop mechanism compares the aggregate queue
 * utilization against the port tail drop threshold
 *
 * \see xpsQosAqmConfigurePortPacketTailDropThreshold
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [in] lengthMaxThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmConfigurePortPageTailDropThreshold(xpsDevice_t devId,
                                                      uint32_t profileId, uint32_t lengthMaxThreshold);

/**
 * \brief Get configured Port tail drop max threshold in pages.
 *
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [out] lengthMaxThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetPortPageTailDropThreshold(xpsDevice_t devId,
                                                uint32_t profileId, uint32_t* lengthMaxThreshold);

/**
 * \brief Display port tail drop max threshold in pages
 *
 *
 * \param [in] devId
 * \param [in] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmDisplayPortTailDropParams(xpsDevice_t devId,
                                             uint32_t profileId);

/*
 * Phantom Q APIs
 */

/**
 * \brief Enable/disable Phantom Q Marking
 *
 * Phantom Queuing is also used to prioritize lower latency
 * traffic in the system and maintain shallow queue depths. It's
 * high level implmentation involves a "phantom" or "shadow"
 * queue apart from the regular queue
 *
 * Phantom Queueing uses the notion of link utilzation instead
 * of buffer occupancy to detect congesiton. So if a link is
 * being heavily utilized, phantom queueing will mark ECN. This
 * is done effectively by treating the phantom queue as a lower
 * speed queue (link) than the real queue
 *
 * The XPA implementation of Phantom queuing requires a queue
 * shaper set at lower speed than the desired link occupancy
 * this will trigger marking when the throughput exceeds the
 * shaping rate
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmSetPhantomQEnable(xpsDevice_t devId, uint32_t profileId,
                                     uint32_t enable);

/**
 * \brief Get Enable/disable Phantom Q Marking
 *
 *
 * \param [in] devId
 * \param [in] profileId
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetPhantomQEnable(xpsDevice_t devId, uint32_t profileId,
                                     uint32_t* enable);

/**
 * \brief Display Phantom Q parameters
 *
 *
 * \param [in] devId
 * \param [in] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmDisplayPhantomQParams(xpsDevice_t devId, uint32_t profileId);

/*
 * Dynamic Pool and Thresholding APIs
 */

/**
 * \brief Enable/disable Queue Dynamic Pool
 *
 * Along with allocating dedicated resources to a queue, the XPA
 * introduces a notion of a Queue Shared Resource Pool. This
 * shared resouce pool accounts pages used by a series of
 * queue's associated with it. This effectively increases the
 * burst capacity of a single queue without tying up pages for
 * that queue
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmSetQueueDynamicPoolEnable(xpsDevice_t devId,
                                             xpPort_t devPort, uint32_t queueNum, uint32_t enable);

/**
 * \brief Get Enable/disable Queue Dynamic Pool
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetQueueDynamicPoolEnable(xpsDevice_t devId,
                                             xpPort_t devPort, uint32_t queueNum, uint32_t* enable);

/**
 * \brief Bind Queue Dynamic Pool to a profile
 *
 * This API will bind a Queue Dynamic Pool to an AQM Q Profile
 * that has been bound to a queue
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [in] dynPoolId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmBindQueueToDynamicPool(xpsDevice_t devId, xpPort_t devPort,
                                          uint32_t queueNum, uint32_t dynPoolId);

/**
 * \brief Get Queue Dynamic Pool for a profile
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] dynPoolId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetQueueToDynamicPoolBinding(xpsDevice_t devId,
                                                xpPort_t devPort, uint32_t queueNum, uint32_t* dynPoolId);

/**
 * \brief Enable/disable Port dynamic pool
 *
 * This API will enable or disable using a bound dynamic pool
 * for a port
 *
 * A Port can also participate in dynamic pool concepts, similar
 * to queues
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmSetPortDynamicPoolEnable(xpsDevice_t devId, xpPort_t devPort,
                                            uint32_t enable);

/**
 * \brief Get Enable/disable Port dynamic pool
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetPortDynamicPoolEnable(xpsDevice_t devId, xpPort_t devPort,
                                            uint32_t* enable);

/**
 * \brief Bind port dynamic pool to a profile
 *
 * This API will bind a port dynamic pool to an AQM profile that
 * is bound to a port
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] dynPoolId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmBindPortToDynamicPool(xpsDevice_t devId, xpPort_t devPort,
                                         uint32_t dynPoolId);

/**
 * \brief Get port dynamic pool for a profile
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [out] dynPoolId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetPortToDynamicPoolBinding(xpsDevice_t devId,
                                               xpPort_t devPort, uint32_t* dynPoolId);

/**
 * \brief Set Dynamic Pool Shared portion's threshold
 *
 * Each Dynamic pool can consist of two portions
 *  - A Shared portion with dynamic thresholding
 *  - A Guaranteed portion with static thresholding
 *
 * This API will set the size of the shared portion of this
 * dynamic pool in pages
 *
 * For devices that only have a statically thresholded Dynamic
 * Pool implementation (may also be known as a shared pool),
 * this API is the one to use when setting the size of the
 * Static pool
 *
 * IE: Shared portion of a Dynamic Pool == Static Size of a
 * Shared Pool
 *
 * The Total Dynamic Pool Threshold for Lossy Traffic must also
 * be set by using the following formula:
 *
 *  - Total Shared Size + (Num Queues Bound * Guar Space per
 *    Queue) + 1024 pages of HW margin
 *
 * The API used to set the Total Dynamic Pool Size is:
 *
 * \see xpsQosAqmSetDynamicPoolTotalThreshold
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [in] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmSetDynamicPoolThreshold(xpsDevice_t devId,
                                           uint32_t dynPoolId, uint32_t threshold);

/**
 * \brief Get Dynamic Pool Shared portion's threshold
 *
 * Each Dynamic pool can consist of two portions
 *  - A Shared portion with dynamic thresholding
 *  - A Guaranteed portion with static thresholding
 *
 * This API will get the size of the shared portion of this
 * dynamic pool in pages
 *
 * For devices that only have a statically thresholded Dynamic
 * Pool implementation (may also be known as a shared pool),
 * this API is the one to use when getting the size of the
 * Static pool
 *
 * IE: Shared portion of a Dynamic Pool == Static Size of a
 * Shared Pool
 *
 * The Total Dynamic Pool Threshold for Lossy Traffic must also
 * be set by using the following formula:
 *
 *  - Total Shared Size + (Num Queues Bound * Guar Space per
 *    Queue) + 1024 pages of HW margin
 *
 * The API used to get the Total Dynamic Pool Size is:
 *
 * \see xpQosAqmGetDynamicPoolTotalThreshold
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [out] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetDynamicPoolThreshold(xpsDevice_t devId,
                                           uint32_t dynPoolId, uint32_t *threshold);

/**
 * \brief Set the Dynamic Pool's Total Threshold
 *
 * Each Dynamic pool can consist of two portions
 *  - A Shared portion with dynamic thresholding
 *  - A Guaranteed portion with static thresholding
 *
 * This API is used to configure the total size of a Dynamic
 * Pool in pages
 *
 * The total size of a Dynamic Pool is defined as:
 *
 *  - Total Shared Size + (Num Queues Bound * Guar Space per
 *    Queue) + 1024 pages of HW margin
 *
 * To configure the Shared Portion's threshold of a Dynamic
 * Pool, use this API:
 *
 * \see xpsQosAqmGetDynamicPoolSharedThreshold
 *
 * To bind a Queue to a Dynamic Pool, use this API:
 *
 * \see xpsQosAqmBindQueueToDynamicPool
 *
 * To enable a Queue to use a Dynamic Pool after binding, use
 * this API
 *
 * \see xpsQosAqmGetQueueDynamicPoolEnable
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [in] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmSetDynamicPoolTotalThreshold(xpsDevice_t devId,
                                                uint32_t dynPoolId, uint32_t threshold);

/**
 * \brief Set the Dynamic Pool's Total Threshold
 *
 * Each Dynamic pool can consist of two portions
 *  - A Shared portion with dynamic thresholding
 *  - A Guaranteed portion with static thresholding
 *
 * This API is used to configure the total size of a Dynamic
 * Pool in pages
 *
 * The total size of a Dynamic Pool is defined as:
 *
 *  - Total Shared Size + (Num Queues Bound * Guar Space per
 *    Queue) + 1024 pages of HW margin
 *
 * To get the Shared Portion's threshold of a Dynamic
 * Pool, use this API:
 *
 * \see xpsQosAqmGetDynamicPoolSharedThreshold
 *
 * To get the Queue to Dynamic Pool Binding, use this API:
 *
 * \see xpsQosAqmGetQueueToDynamicPoolBinding
 *
 * To get whether a Queue uses a Dynamic Pool after binding, use
 * this API
 *
 * \see xpsQosAqmGetQueueDynamicPoolEnable
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [out] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetDynamicPoolTotalThreshold(xpsDevice_t devId,
                                                uint32_t dynPoolId, uint32_t* threshold);

/**
 * \public
 * \brief Get a configured Alpha Value for a Dynamic Lossy Pool
 *
 * A Dynamic Pool allows for multiple queues to both increase
 * their total Burst Capacity by virtue of borrowing pages from
 * other Queues, and maintain fairness in Packet Memory
 * utilization
 *
 * In the case where a Shared Pool is simply statically
 * thresholded, a single Queue can use up the entire shared pool
 * in a large burst scenario, which leads to Head-Of-Line
 * blocking for all other Queues that can use this Shared Packet
 * Memory resource
 *
 * The ideal Shared Packet Memory accounting algorithm would
 * allow for a single Queue to increase its dynamic burst
 * capcity when congested and also maintain enough Packet Memory
 * to not starve other potential queues from utilizing buffers
 *
 * This can be achieved using a Dynamic Thresholding algorithm
 * that derives the Queue Tail Drop threshold based off the
 * current overall pool buffer utilization
 *
 * This formula requies an "alpha" coefficient to be set in
 * order to set the agressiveness of the queue utilization of
 * the dynamic pool
 *
 * The Queue Dynamic Tail Drop formula is as follows:
 *
 * <pre>
 *
 * q_dyn_td_thld = alpha(DP_size - DP_used)
 *
 * </pre>
 *
 * During AQM, a Lossy queue will decide to drop a packet due to
 * current Queue Buffer utilization if the following equation is
 * satsified
 *
 * <pre>
 *
 * (q_cur_len + new_pkt_len > max(q_dyn_td_thld, q_guar_td_thld)) ? drop pkt : fwd pkt
 *
 * </pre>
 *
 * When the system reaches a steady state of congestion, we can
 * approximate each queue length as per the following formula
 *
 * <pre>
 *                     alpha
 * q_cur_len = max(------------ * DP_Size, q_guar_td_thld)
 *                 1 + (alpha)N
 *
 * Where N is the number of Active Queues in the Dynamic Pool
 *
 * </pre>
 *
 * In addition at steady state, the approximate available buffer
 * in the Dynamic Pool's shared section will follow this
 * formula:
 *
 * <pre>
 *                    1
 * dp_unused = (------------ * DP_Size)
 *              1 + (alpha)N
 *
 * Where N is the number of Active Queues in the Dynamic Pool
 *
 * </pre>
 *
 * The following graph shows a picture of how the number of
 * congested queues actively using a dynamic pool will impact
 * the burst size of a single queue
 *
 * <pre>
 *
 *  Avg    |
 *  Lossy  |  |--|
 *  PM     |  |++|
 *  util   |  |++|  |--|
 *  per    |  |++|  |++|
 *  active |  |++|  |++|  |--|
 *  queue  |  |++|  |++|  |++|
 *         |  |++|  |++|  |++|  |--|
 *         |  |++|  |++|  |++|  |++|  |--|
 *         |  |++|  |++|  |++|  |++|  |++|  |--|  |--|
 *        -|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--
 *         |   1     2     3     4     5     6     7
 *
 *          Number of Active Queues using the Dynamic Pool
 *
 * </pre>
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [out] fraction
 * \param [out] coefficient
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmConfigureDynamicPoolAlpha(xpsDevice_t devId,
                                             uint32_t dynPoolId, xpDynThldFraction_e fraction, xpDynThldCoeff_e coefficient);

/**
 * \brief Get a configured Alpha Value for a Dynamic Lossy Pool
 *
 * A Dynamic Pool allows for multiple queues to both increase
 * their total Burst Capacity by virtue of borrowing pages from
 * other Queues, and maintain fairness in Packet Memory
 * utilization
 *
 * In the case where a Shared Pool is simply statically
 * thresholded, a single Queue can use up the entire shared pool
 * in a large burst scenario, which leads to Head-Of-Line
 * blocking for all other Queues that can use this Shared Packet
 * Memory resource
 *
 * The ideal Shared Packet Memory accounting algorithm would
 * allow for a single Queue to increase its dynamic burst
 * capcity when congested and also maintain enough Packet Memory
 * to not starve other potential queues from utilizing buffers
 *
 * This can be achieved using a Dynamic Thresholding algorithm
 * that derives the Queue Tail Drop threshold based off the
 * current overall pool buffer utilization
 *
 * This formula requies an "alpha" coefficient to be set in
 * order to set the agressiveness of the queue utilization of
 * the dynamic pool
 *
 * The Queue Dynamic Tail Drop formula is as follows:
 *
 * <pre>
 *
 * q_dyn_td_thld = alpha(DP_size - DP_used)
 *
 * </pre>
 *
 * During AQM, a Lossy queue will decide to drop a packet due to
 * current Queue Buffer utilization if the following equation is
 * satsified
 *
 * <pre>
 *
 * (q_cur_len + new_pkt_len > max(q_dyn_td_thld, q_guar_td_thld)) ? drop pkt : fwd pkt
 *
 * </pre>
 *
 * When the system reaches a steady state of congestion, we can
 * approximate each queue length as per the following formula
 *
 * <pre>
 *                     alpha
 * q_cur_len = max(------------ * DP_Size, q_guar_td_thld)
 *                 1 + (alpha)N
 *
 * Where N is the number of Active Queues in the Dynamic Pool
 *
 * </pre>
 *
 * In addition at steady state, the approximate available buffer
 * in the Dynamic Pool's shared section will follow this
 * formula:
 *
 * <pre>
 *                    1
 * dp_unused = (------------ * DP_Size)
 *              1 + (alpha)N
 *
 * Where N is the number of Active Queues in the Dynamic Pool
 *
 * </pre>
 *
 * The following graph shows a picture of how the number of
 * congested queues actively using a dynamic pool will impact
 * the burst size of a single queue
 *
 * <pre>
 *
 *  Avg    |
 *  Lossy  |  |--|
 *  PM     |  |++|
 *  util   |  |++|  |--|
 *  per    |  |++|  |++|
 *  active |  |++|  |++|  |--|
 *  queue  |  |++|  |++|  |++|
 *         |  |++|  |++|  |++|  |--|
 *         |  |++|  |++|  |++|  |++|  |--|
 *         |  |++|  |++|  |++|  |++|  |++|  |--|  |--|
 *        -|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--
 *         |   1     2     3     4     5     6     7
 *
 *          Number of Active Queues using the Dynamic Pool
 *
 * </pre>
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [out] fraction
 * \param [out] coefficient
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetDynamicPoolAlpha(xpsDevice_t devId, uint32_t dynPoolId,
                                       xpDynThldFraction_e *fraction, xpDynThldCoeff_e *coefficient);

/**
 * \public
 * \brief Get a configured Alpha Value for a color-aware Dynamic Lossy Pool
 *
 * A Dynamic Pool allows for multiple color-aware queues to both increase
 * their total Burst Capacity by virtue of borrowing pages from
 * other Queues, and maintain fairness in Packet Memory
 * utilization
 *
 * In the case where a Shared Pool is simply statically
 * thresholded, a single Queue can use up the entire shared pool
 * in a large burst scenario, which leads to Head-Of-Line
 * blocking for all other Queues that can use this Shared Packet
 * Memory resource
 *
 * The ideal Shared Packet Memory accounting algorithm would
 * allow for a single Queue to increase its dynamic burst
 * capcity when congested and also maintain enough Packet Memory
 * to not starve other potential queues from utilizing buffers
 *
 * This can be achieved using a Dynamic Thresholding algorithm
 * that derives the Queue Tail Drop threshold based off the
 * current overall pool buffer utilization
 *
 * This formula requies an "alpha" coefficient to be set in
 * order to set the agressiveness of the queue utilization of
 * the dynamic pool
 *
 * The Queue Dynamic Tail Drop formula is as follows:
 *
 * <pre>
 *
 * q_dyn_td_thld = alpha(DP_size - DP_used)
 *
 * </pre>
 *
 * During AQM, a Lossy queue will decide to drop a packet due to
 * current Queue Buffer utilization if the following equation is
 * satsified
 *
 * <pre>
 *
 * (q_cur_len + new_pkt_len > max(q_dyn_td_thld, q_guar_td_thld)) ? drop pkt : fwd pkt
 *
 * </pre>
 *
 * When the system reaches a steady state of congestion, we can
 * approximate each queue length as per the following formula
 *
 * <pre>
 *                     alpha
 * q_cur_len = max(------------ * DP_Size, q_guar_td_thld)
 *                 1 + (alpha)N
 *
 * Where N is the number of Active Queues in the Dynamic Pool
 *
 * </pre>
 *
 * In addition at steady state, the approximate available buffer
 * in the Dynamic Pool's shared section will follow this
 * formula:
 *
 * <pre>
 *                    1
 * dp_unused = (------------ * DP_Size)
 *              1 + (alpha)N
 *
 * Where N is the number of Active Queues in the Dynamic Pool
 *
 * </pre>
 *
 * The following graph shows a picture of how the number of
 * congested queues actively using a dynamic pool will impact
 * the burst size of a single queue
 *
 * <pre>
 *
 *  Avg    |
 *  Lossy  |  |--|
 *  PM     |  |++|
 *  util   |  |++|  |--|
 *  per    |  |++|  |++|
 *  active |  |++|  |++|  |--|
 *  queue  |  |++|  |++|  |++|
 *         |  |++|  |++|  |++|  |--|
 *         |  |++|  |++|  |++|  |++|  |--|
 *         |  |++|  |++|  |++|  |++|  |++|  |--|  |--|
 *        -|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--
 *         |   1     2     3     4     5     6     7
 *
 *          Number of Active Queues using the Dynamic Pool
 *
 * </pre>
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [out] fraction
 * \param [out] coefficient
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmConfigureDynamicPoolAlphaForColor(xpsDevice_t devId,
                                                     uint32_t dynPoolId, xpQosColor_e color, xpDynThldFraction_e fraction,
                                                     xpDynThldCoeff_e coefficient);

/**
 * \brief Get a configured Alpha Value for a color-aware Dynamic Lossy Pool
 *
 * A Dynamic Pool allows for multiple color-aware queues to both increase
 * their total Burst Capacity by virtue of borrowing pages from
 * other Queues, and maintain fairness in Packet Memory
 * utilization
 *
 * In the case where a Shared Pool is simply statically
 * thresholded, a single Queue can use up the entire shared pool
 * in a large burst scenario, which leads to Head-Of-Line
 * blocking for all other Queues that can use this Shared Packet
 * Memory resource
 *
 * The ideal Shared Packet Memory accounting algorithm would
 * allow for a single Queue to increase its dynamic burst
 * capcity when congested and also maintain enough Packet Memory
 * to not starve other potential queues from utilizing buffers
 *
 * This can be achieved using a Dynamic Thresholding algorithm
 * that derives the Queue Tail Drop threshold based off the
 * current overall pool buffer utilization
 *
 * This formula requies an "alpha" coefficient to be set in
 * order to set the agressiveness of the queue utilization of
 * the dynamic pool
 *
 * The Queue Dynamic Tail Drop formula is as follows:
 *
 * <pre>
 *
 * q_dyn_td_thld = alpha(DP_size - DP_used)
 *
 * </pre>
 *
 * During AQM, a Lossy queue will decide to drop a packet due to
 * current Queue Buffer utilization if the following equation is
 * satsified
 *
 * <pre>
 *
 * (q_cur_len + new_pkt_len > max(q_dyn_td_thld, q_guar_td_thld)) ? drop pkt : fwd pkt
 *
 * </pre>
 *
 * When the system reaches a steady state of congestion, we can
 * approximate each queue length as per the following formula
 *
 * <pre>
 *                     alpha
 * q_cur_len = max(------------ * DP_Size, q_guar_td_thld)
 *                 1 + (alpha)N
 *
 * Where N is the number of Active Queues in the Dynamic Pool
 *
 * </pre>
 *
 * In addition at steady state, the approximate available buffer
 * in the Dynamic Pool's shared section will follow this
 * formula:
 *
 * <pre>
 *                    1
 * dp_unused = (------------ * DP_Size)
 *              1 + (alpha)N
 *
 * Where N is the number of Active Queues in the Dynamic Pool
 *
 * </pre>
 *
 * The following graph shows a picture of how the number of
 * congested queues actively using a dynamic pool will impact
 * the burst size of a single queue
 *
 * <pre>
 *
 *  Avg    |
 *  Lossy  |  |--|
 *  PM     |  |++|
 *  util   |  |++|  |--|
 *  per    |  |++|  |++|
 *  active |  |++|  |++|  |--|
 *  queue  |  |++|  |++|  |++|
 *         |  |++|  |++|  |++|  |--|
 *         |  |++|  |++|  |++|  |++|  |--|
 *         |  |++|  |++|  |++|  |++|  |++|  |--|  |--|
 *        -|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--
 *         |   1     2     3     4     5     6     7
 *
 *          Number of Active Queues using the Dynamic Pool
 *
 * </pre>
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [out] fraction
 * \param [out] coefficient
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetDynamicPoolAlphaForColor(xpsDevice_t devId,
                                               uint32_t dynPoolId, xpQosColor_e color, xpDynThldFraction_e *fraction,
                                               xpDynThldCoeff_e *coefficient);

/**
 * \brief This method gets the queue dynamic pool shared used
 *        for given dynamic pool id.
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t dynPoolId
 * \param [out] uint32_t* counter
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetDynamicPoolCurrentSharedUsed(xpDevice_t devId,
                                                   uint32_t dynPoolId, uint32_t *counter);

/**
 * \brief This method gets the queue dynamic pool total used
 *        for given dynamic pool id.
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t dynPoolId
 * \param [out] uint32_t* counter
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetDynamicPoolCurrentTotalUsed(xpDevice_t devId,
                                                  uint32_t dynPoolId, uint32_t *counter);

/*
 * Global Thresholds
 */

/**
 * \brief Configure global packet threshold
 *
 * In a device, there are a fixed number of pages and tokens
 * that can be used at a same time. Usually the number of pages
 * add up to the size of the total packet memory
 *
 * When implementing QOS, a Panic level may be required for a
 * case of a storm of traffic which eats up tokens. To prevent
 * the system from hanging, a global packet threshold set
 * below the total number of tokens can be used to implement
 * that Panic level so that there will be tokens available in
 * the system for ingressing packets
 *
 * \param [in] devId
 * \param [in] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmSetGlobalPacketThreshold(xpsDevice_t devId,
                                            uint32_t threshold);

/**
 * \brief Get configured global packet threshold
 *
 * This API returns the configured global packet threshold
 *
 * \param [in] devId
 * \param [out] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetGlobalPacketThreshold(xpsDevice_t devId,
                                            uint32_t *threshold);

/**
 * \brief Configure global page threshold
 *
 * In a device, there are a fixed number of pages and tokens
 * that can be used at a same time. Usually the number of pages
 * add up to the size of the total packet memory
 *
 * When implementing QOS, a Panic level may be required for a
 * case of a storm of traffic which eats up pages. To prevent
 * the system from hanging, a global packet threshold set
 * below the total number of tokens can be used to implement
 * that Panic level so that there will be pages available in
 * the system for ingressing packets
 *
 * \param [in] devId
 * \param [in] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmSetGlobalPageThreshold(xpsDevice_t devId,
                                          uint32_t threshold);

/**
 * \brief Get configured global page threshold
 *
 * This API returns the configured global page threshold
 *
 * \param [in] devId
 * \param [out] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetGlobalPageThreshold(xpsDevice_t devId,
                                          uint32_t *threshold);

/*
 * Global Multicast Thresholds
 */

/**
 * \brief Configure global multicast packet threshold
 *
 * Similar to a global packet threshold, XPA supports a global
 * Multicast packet threshold. This can be used to limit the
 * number of multicast tokens in the system at a given time.
 * This feature is useful to limit the impact of multicast
 * traffic with respect to overall system performance
 *
 * \param [in] devId
 * \param [in] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmSetMulticastPacketThreshold(xpsDevice_t devId,
                                               uint32_t threshold);

/**
 * \brief Get configured global multicast packet threshold
 *
 * This API will return the configured multicast packet
 * threshold
 *
 * \param [in] devId
 * \param [out] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetMulticastPacketThreshold(xpsDevice_t devId,
                                               uint32_t *threshold);

/**
 * \brief Configure global multicast page threshold
 *
 * Similar to a global packet threshold, XPA supports a global
 * Multicast packet threshold. This can be used to limit the
 * number of multicast pages in the system at a given time.
 * This feature is useful to limit the impact of multicast
 * traffic with respect to overall system performance
 *
 * \param [in] devId
 * \param [in] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmSetMulticastPageThreshold(xpsDevice_t devId,
                                             uint32_t threshold);

/**
 * \brief get configured global multicast page threshold
 *
 * This API will return the configured multicast page
 * threshold
 *
 * \param [in] devId
 * \param [in] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetMulticastPageThreshold(xpsDevice_t devId,
                                             uint32_t *threshold);

/**
 * \brief Enable/Disable Global ECN threshold
 *
 * \param [in] devId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmSetGlobalEcnThresholdEnable(xpDevice_t devId,
                                               uint32_t enable);

/**
 * \brief get Enable/Disable Global ECN threshold
 *
 * \param [in]  devId
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetGlobalEcnThresholdEnable(xpDevice_t devId,
                                               uint32_t* enable);

/**
 * \brief set configured global ECN threshold
 *
 * \param [in] devId
 * \param [in] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmSetGlobalEcnThreshold(xpDevice_t devId, uint32_t threshold);

/**
 * \brief get configured global ECN threshold
 *
 * \param [in] devId
 * \param [out] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetGlobalEcnThreshold(xpDevice_t devId, uint32_t* threshold);

/*
 * Per Port/Queue Thresholds
 */

/**
 * \brief Configure queue packet tail drop threshold
 *
 * Tail drop is the fundamental method of resource allocation
 * used in switches today. It relies on instantaneous feedback
 * of queue utilization to dictate whether or not a packet
 * should be enqueued
 *
 * For example, if a tail drop threshold is set to 120 packets,
 * and the queue at time T currently has 119 enqueued. At time
 * T+1 2 packets arrive and requests whether or not they can be
 * enqueued from different sources, the Tail Drop threshold will
 * be crossed by the second requestor and AQM should not
 * allow for it by enqueued
 *
 * Unlike WRED which uses historical data of queuing, Tail Drop
 * uses only instantaneous information
 *
 * This Tail Drop threshold is in packets, there is another tail
 * drop threshold in pages. Both can exist at the same time and
 * either one can decide to drop a packet
 *
 * \see xpAqmMgr::configureQueuePageTailDropThreshold
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [in] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmConfigureQueuePacketTailDropThreshold(xpsDevice_t devId,
                                                         xpsPort_t devPort, uint32_t queueNum, uint32_t threshold);

/**
 * \brief Configure port tail drop threshold in packet count
 *
 * Tail drop is the fundamental method of resource allocation
 * used in switches today. It relies on instantaneous feedback
 * of port utilization to dictate whether or not a packet
 * should be enqueued
 *
 * For example, if a tail drop threshold is set to 120 packets,
 * and the port at time T currently has 119 enqueued. At time
 * T+1 2 packets arrive and requests whether or not they can be
 * enqueued from different sources, the Tail Drop threshold will
 * be crossed by the second requestor and AQM should not
 * allow for it by enqueued
 *
 * Unlike WRED which uses historical data of queuing, Tail Drop
 * uses only instantaneous information
 *
 * This Tail Drop threshold is in packets, which is a byte
 * resolution, there is another tail drop threshold in packets.
 * Both can exist at the same time and either one can decide to
 * drop a packet
 *
 * The port tail drop mechanism compares the aggregate queue
 * utilization against the port tail drop threshold
 *
 * \see xpAqmMgr::configurePortPageTailDropThreshold
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetQueuePacketTailDropThreshold(xpsDevice_t devId,
                                                   xpsPort_t devPort, uint32_t queueNum, uint32_t *threshold);

/**
 * \brief Get configured port tail drop packet threshold
 *
 * This API will retrieve the port packet tail drop threshold
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmConfigurePortPacketTailDropThreshold(xpsDevice_t devId,
                                                        xpsPort_t devPort, uint32_t threshold);

/**
 * \brief Get configured port tail drop threshold in packet
 *        count
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetPortPacketTailDropThreshold(xpsDevice_t devId,
                                                  xpsPort_t devPort, uint32_t *threshold);

/*
 * Q-Mapping Control APIs
 */

/**
 * \brief This API will return the absolute queue number stored
 *        at the qmap index and queue location passed in
 *
 * This API returns the absolute queue number stored at the
 * index and location specified by the caller
 *
 * There is a qmap table that exists in the Traffic Manager
 * which is used to overlay a mapping funciton of a set of input
 * bits like destination port, source port, traffic class,
 * multicast etc... to an absolute queue number which will be
 * used to enqueue a packet
 *
 *
 * \param [in] devId
 * \param [in] qmapIdx
 * \param [in] queueLoc
 * \param [out] absQNum
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetQueueAtQmapIndex(xpsDevice_t devId, uint32_t qmapIdx,
                                       uint32_t queueLoc, uint32_t *absQNum);


/*
 * Miscellaneous AQM Apis
 */

/**
 * \brief Returns the total profiles this device supports from
 *        hardware
 *
 * \param [in]    devId
 * \param [out]   numProfiles
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetAqmQTotalProfiles(xpsDevice_t devId,
                                        uint32_t* numProfiles);

/**
 * \brief Returns the total profiles this device supports from
 *        hardware
 *
 * \param [in]    devId
 * \param [out]   numProfiles
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetAqmPortTotalProfiles(xpsDevice_t devId,
                                           uint32_t* numProfiles);

/**
 * \brief Returns the total number of queues per port for this
 *        device
 *
 * \param [in]   devId
 * \param [in]   port Device port number
 * \param [out]  numQs Total number of queues per port
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetNumberOfQueuesPerPort(xpsDevice_t devId, xpPort_t port,
                                            uint32_t* numQs);

/**
 * \brief Returns the total number of unicast queues per port for this
 *        device
 *
 * \param [in]   devId Device id
 * \param [in]   port Device port number
 * \param [out]  numUcastQs Number of unicast queues per port
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetNumberOfUnicastQueuesPerPort(xpsDevice_t devId,
                                                   xpPort_t port, uint32_t* numUcQs);

/**
 * \brief Returns the total number of multicast queues per port for this
 *        device
 *
 * \param [in]   devId Device id
 * \param [in]   port Device port number
 * \param [out]  numMcastQs Number of multicast queues per port
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetNumberOfMulticastQueuesPerPort(xpsDevice_t devId,
                                                     xpPort_t port, uint32_t* numMcQs);

/**
 * \brief This method sets the queue guaranteed threshold
 *        granularity. Granularity has to be in power of 2.
 *
 * \param [in] devId
 * \param [in] granularity
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmSetQueueGuarThresholdGranularity(xpsDevice_t devId,
                                                    xpQGuarThresholdGranularity_e granularity);

/**
 * \brief This method gets the queue guaranteed threshold
 *        granularity.
 *
 * \param [in] devId
 * \param [out] granularity
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmGetQueueGuarThresholdGranularity(xpsDevice_t devId,
                                                    xpQGuarThresholdGranularity_e* granularity);

/**
 * \brief print the AQM profile for given Id
 *
 * \param [in] devId
 * \param [in] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmDisplayAqmProfile(xpsDevice_t devId, uint32_t profileId);

/**
 * \brief print the AQM Queue profile for given Id
 *
 * \param [in] devId
 * \param [in] profileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosAqmDisplayAqmQProfile(xpsDevice_t devId, uint32_t profileId);

/*
xps CPU Storm Control section
*/

/**
 * \brief API to set Reason Code Table Entry
 *
 * \param [in] devId
 * \param [in] index
 * \param [in] destPort
 * \param [in] tc
 * \param [in] enableTruncation
 * \param [in] enableMod
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosCpuScSetReasonCodeTableEntry(xpsDevice_t devId, uint32_t index,
                                             uint32_t destPort, uint32_t tc, uint32_t enableTruncation, uint32_t enableMod);

/**
 * \brief API to set the destination port for a Reason Code Table Entry
 *
 * \param [in] devId
 * \param [in] index
 * \param [in] destPort
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosCpuScSetRctDestPort(xpsDevice_t devId, uint32_t index,
                                    uint32_t destPort);

/**
 * \brief API to set the traffic class for a Reason Code Table Entry
 *
 * \param [in] devId
 * \param [in] index
 * \param [in] tc
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosCpuScSetRctTc(xpsDevice_t devId, uint32_t index, uint32_t tc);

/**
 * \brief API to enable truncation for a Reason Code Table Entry.
 * Packets hitting an rct entry with truncation bit set will be truncated before being redirected to the RCT destination port.
 *
 * \param [in] devId
 * \param [in] index
 * \param [in] enableTruncation
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosCpuScSetRctTruncation(xpsDevice_t devId, uint32_t index,
                                      uint32_t enableTruncation);

/**
 * \brief API to enable modifications for a Reason Code Table Entry
 * Packets hitting an rct entry with enable modification set will be modified on egress before being redirected to the RCT destination port.
 *
 * \param [in] devId
 * \param [in] index
 * \param [in] enableMod
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosCpuScSetRctEnableMod(xpsDevice_t devId, uint32_t index,
                                     uint32_t enableMod);

/**
 * \brief API to get Reason Code Table Entry
 *
 * \param [in] devId
 * \param [in] index
 * \param [out] *destPort
 * \param [out] *tc
 * \param [out] *enableTruncation
 * \param [out] *enableMod
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosCpuScGetReasonCodeTableEntry(xpsDevice_t devId, uint32_t index,
                                             uint32_t *destPort, uint32_t *tc, uint32_t *enableTruncation,
                                             uint32_t *enableMod);

/**
 * \brief API to get the destination port for a Reason Code Table Entry
 *
 * \param [in] devId
 * \param [in] index
 * \param [out] *destPort
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosCpuScGetRctDestPort(xpsDevice_t devId, uint32_t index,
                                    uint32_t *destPort);

/**
 * \brief API to get the traffic class for a Reason Code Table Entry
 *
 * \param [in] devId
 * \param [in] index
 * \param [out] *tc
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosCpuScGetRctTc(xpsDevice_t devId, uint32_t index, uint32_t *tc);

/**
 * \brief API to get the truncation enable bit for a Reason Code Table Entry.
 *
 * \param [in] devId
 * \param [in] index
 * \param [out] *enableTruncation
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosCpuScGetRctTruncation(xpsDevice_t devId, uint32_t index,
                                      uint32_t *enableTruncation);

/**
 * \brief API to get the enable modifications bit for a Reason Code Table Entry
 *
 * \param [in] devId
 * \param [in] index
 * \param [out] *enableMod
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosCpuScGetRctEnableMod(xpsDevice_t devId, uint32_t index,
                                     uint32_t *enableMod);

/**
 * \brief API to get queue group number to which the queue is connected
 *
 * This API will return the queue group number that corresponds to the
 * destination port and queue combination
 *
 * For example, queue group number for port 176 and queue 25 is 243,
 * where each queue group has 8 queues.
 *   - queue absolute index / number of queues per queue group
 *
 * The queue group number is mainly used at the time of adding CPU queue
 * group to DMA queue group map configuration
 *
 * \see xpsQosCpuScCpuQGrpToDmaQGrpMapAdd
 * \see xpsQosCpuScCpuQGrpToDmaQGrpMapGet
 * \see xpsQosCpuScCpuQGrpToDmaQGrpMapRemove
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] queueGroupId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosCpuScGetQueueGroupId(xpsDevice_t devId, xpPort_t devPort,
                                     uint32_t queueNum, uint32_t *queueGroupId);

/**
 * \brief API to get DMA queue group number
 *
 * This API will return the DMA queue group number that corresponds to DMA queue
 *
 * There are 64 PCIe Tx DMAs which are internally divided into 8 DMA queue groups.
 * DMA queue group number represents start of 8 consecutive Tx DMA queues.
 *
 * For example, 64 PCIe Tx DMAs to DMA queue group mapping
 *  0  through 7  Tx DMAs - 0 (dmaQueueGroupId)
 *  8  through 15 Tx DMAs - 8 (dmaQueueGroupId)
 *  16 through 23 Tx DMAs - 16 (dmaQueueGroupId)
 *  ....
 *  56 through 63 Tx DMAs - 56 (dmaQueueGroupId)
 *
 * The DMA queue group number is mainly used at the time of adding CPU queue group to
 * DMA queue group map configuration
 *
 * \see xpsQosCpuScCpuQGrpToDmaQGrpMapAdd
 * \see xpsQosCpuScCpuQGrpToDmaQGrpMapGet
 * \see xpsQosCpuScCpuQGrpToDmaQGrpMapRemove
 *
 * \param [in] devId
 * \param [in] dmaQNum
 * \param [out] dmaQueueGroupId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosCpuScGetDmaQueueGroupId(xpsDevice_t devId, uint32_t dmaQNum,
                                        uint32_t *dmaQueueGroupId);

/**
 * \brief API to add the CPU queue group to DMA queue group map entry
 *
 * There are 64 PCIe Tx DMAs and each TO CPU Queue is associated with one of the 64 Tx DMAs.
 * This API helps to associate each of the queues designated to host CPU to one of the Tx
 * DMAs(0 through 63).
 *
 * Each map entry represents, 8 queues in the queue group are assigned to 8 consecutive
 * DMA queues starting from DMA queue group number
 *
 * \see xpsQosCpuScGetQueueGroupId
 * \see xpsQosCpuScGetDmaQueueGroupId
 *
 * \param [in] devId
 * \param [in] cpuQueueGroupNum
 * \param [in] dmaQueueGroupNum
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosCpuScCpuQGrpToDmaQGrpMapAdd(xpsDevice_t devId,
                                            uint32_t cpuQueueGroupNum, uint32_t dmaQueueGroupNum);

/**
 * \brief API to get the CPU queue group to DMA queue group map entry
 *
 * \see xpsQosCpuScCpuQGrpToDmaQGrpMapAdd
 * \see xpsQosCpuScCpuQGrpToDmaQGrpMapRemove
 *
 * \param [in] devId
 * \param [in] cpuQueueGroupNum
 * \param [out] dmaQueueGroupNum
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosCpuScCpuQGrpToDmaQGrpMapGet(xpsDevice_t devId,
                                            uint32_t cpuQueueGroupNum, uint32_t *dmaQueueGroupNum);

/**
 * \brief API to remove the CPU queue group to DMA queue group map entry
 *
 * This API will remove the CPU queue group to DMA queue group mapping
 *
 * \see xpsQosCpuScGetQueueGroupId
 *
 * \param [in] devId
 * \param [in] cpuQueueGroupNum
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosCpuScCpuQGrpToDmaQGrpMapRemove(xpsDevice_t devId,
                                               uint32_t cpuQueueGroupNum);

/**
 * \brief API to get CPU queue num from reason code
 *
 * \param [in] devId
 * \param [in] cpuType
 * \param [in] reasonCode
 * \param [out] queueNum
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosCpuScGetReasonCodeToCpuQueueMap(xpsDevice_t devId,
                                                xpCpuTypes_e cpuType, uint32_t reasonCode, uint32_t *queueNum);

/*
 * 802.1Qbb: PFC configuration APIs
 */

/*
 * Generic PFC Feature Configuration APIs
 */

/**
 * \brief This method enables Pfc receive.
 *
 * \param [in] devId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPfcRcvEn(xpsDevice_t devId, uint32_t enable);

/**
 * \brief This method get the status of Pfc receive.
 *
 * \param [in] devId
 * \param [out] *enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPfcRcvEn(xpsDevice_t devId, uint32_t *enable);

/**
 * \brief This method gets the PF priority for a port.
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPfcPriority(xpsDevice_t devId, xpsPort_t devPort,
                                 uint32_t queueNum, uint32_t* priority);

/**
 * \brief This method will return the number of PFC priority groups
 *        that can be supported by a device
 *
 * In XPA, some devices support 8 Lossless traffic classes, and
 * others support less. In any case, XPA supports an arbitrary
 * mapping of PFC Priority to internal PFC Traffic Class, which
 * is used for accounting purposes
 *
 * \param [in]  xpDevice_t devId
 * \param [Out] uint32_t& totalSupportedPrio
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetNumPfcPrioritiesSupported(xpsDevice_t devId,
                                               uint32_t *totalSupportedPrio);

/**
 * \brief This method will create a binding of an external PFC
 *        Priority to an internal Traffic Class
 *
 * In XPA, some devices support 8 Lossless traffic classes, and
 * others support less. In any case, XPA supports an arbitrary
 * mapping of PFC Priority to internal PFC Traffic Class, which
 * is used for accounting purposes
 *
 * When a PFC pause frame is sent back upstream, the internal
 * PFC Traffic Class is mapped back to the external PFC Priority
 * and it's corresponding bit is set in the 802.1Qbb Pause
 * Frame's Priority Vector
 *
 *
 * \param [in] devId
 * \param [in] priority
 * \param [in] pfcTrafficClass
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcBindPfcPriorityToTc(xpsDevice_t devId, uint32_t priority,
                                      uint32_t pfcTrafficClass);

/**
 * \brief This method will retreive the binding of an external
 *        PFC Priority to an internal Traffic Class
 *
 * In XPA, some devices support 8 Lossless traffic classes, and
 * others support less. In any case, XPA supports an arbitrary
 * mapping of PFC Priority to internal PFC Traffic Class, which
 * is used for accounting purposes
 *
 * When a PFC pause frame is sent back upstream, the internal
 * PFC Traffic Class is mapped back to the external PFC Priority
 * and it's corresponding bit is set in the 802.1Qbb Pause
 * Frame's Priority Vector
 *
 *
 * \param [in] devId
 * \param [in] priority
 * \param [in] pfcTrafficClass
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPfcPriorityToTcBinding(xpsDevice_t devId,
                                            uint32_t priority, uint32_t *pfcTrafficClass);

/*
 * Configure 802.1Qbb PFC Static Thresholds for a Group of Ports
 */

/**
 * \brief This method binds a port to a port pfc profile.
 *
 * These PFC profiles are the equivalent of a PFC Port Group.
 * The threshold set in the Port PFC profile cover all ports
 * that are bound to this profile
 *
 * IE: If there are three ports that use PFC Static Port Profile
 * number 2, the aggregate RX count of all priorities for the
 * three ports will be used to compare against the PFC Static
 * Port Profile Number 2's XOFF threshold
 *
 * If this aggregate count crosses the XOFF threshold of this
 * Port Group, all three ports will send PFC pause frames back
 * to the sender with all of its bits set in the priority vector
 * with max quanta
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] portPfcGroupPflId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcBindPortToPfcGroupStaticProfile(xpsDevice_t devId,
                                                  uint32_t portId, uint32_t portPfcGroupPflId);

/**
 * \brief This method gets the port pfc profile a port is bound to.
 *
 * These PFC profiles are the equivalent of a PFC Port Group.
 * The threshold set in the Port PFC profile cover all ports
 * that are bound to this profile
 *
 * IE: If there are three ports that use PFC Static Port Profile
 * number 2, the aggregate RX count of all priorities for the
 * three ports will be used to compare against the PFC Static
 * Port Profile Number 2's XOFF threshold
 *
 * If this aggregate count crosses the XOFF threshold of this
 * Port Group, all three ports will send PFC pause frames back
 * to the sender with all of its bits set in the priority vector
 * with max quanta
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [out] portPfcGroupPflId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortToPfcGroupStaticProfileBinding(xpsDevice_t devId,
                                                        uint32_t portId, uint32_t *portPfcGroupPflId);

/**
 * \brief This method sets the XON threshold for a port profile.
 *
 * \param [in] devId
 * \param [in] portPfcGroupPflId
 * \param [in] xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPortPfcGroupStaticXonThreshold(xpsDevice_t devId,
                                                    uint32_t portPfcGroupPflId, uint32_t xonThreshold);

/**
 * \brief This method gets the XON threshold for a port profile.
 *
 * \param [in] devId
 * \param [in] portPfcGroupPflId
 * \param [out] *xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortPfcGroupStaticXonThreshold(xpsDevice_t devId,
                                                    uint32_t portPfcGroupPflId, uint32_t *xonThreshold);

/**
 * \brief This method sets the XOFF threshold for a port profile.
 *
 * \param [in] devId
 * \param [in] portPfcGroupPflId
 * \param [in] xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPortPfcGroupStaticXoffThreshold(xpsDevice_t devId,
                                                     uint32_t portPfcGroupPflId, uint32_t xoffThreshold);

/**
 * \brief This method gets the XOFF threshold for a port profile.
 *
 * \param [in] devId
 * \param [in] portPfcGroupPflId
 * \param [out] *xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortPfcGroupStaticXoffThreshold(xpsDevice_t devId,
                                                     uint32_t portPfcGroupPflId, uint32_t *xoffThreshold);

/*
 * Configure 802.1Qbb PFC Thresholds Globally per Traffic Class
 */

/**
 * \brief This method sets the XON threshold for a traffic class.
 *
 * \param [in] devId
 * \param [in] trafficClass
 * \param [in] xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetGlobalTcPfcStaticXonThreshold(xpsDevice_t devId,
                                                   uint32_t trafficClass, uint32_t xonThreshold);

/**
 * \brief This method gets the XON threshold for a traffic class.
 *
 * \param [in] devId
 * \param [in] trafficClass
 * \param [out] *xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetGlobalTcPfcStaticXonThreshold(xpsDevice_t devId,
                                                   uint32_t trafficClass, uint32_t *xonThreshold);

/**
 * \brief This method sets the XOFF threshold for a traffic class.
 *
 * \param [in] devId
 * \param [in] trafficClass
 * \param [in] xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetGlobalTcPfcStaticXoffThreshold(xpsDevice_t devId,
                                                    uint32_t trafficClass, uint32_t xoffThreshold);

/**
 * \brief This method gets the XOFF threshold for a traffic class.
 *
 * \param [in] devId
 * \param [in] trafficClass
 * \param [out] *xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetGlobalTcPfcStaticXoffThreshold(xpsDevice_t devId,
                                                    uint32_t trafficClass, uint32_t *xoffThreshold);

/*
 * Configure 802.1Qbb PFC Thresholds for a Port/Traffic Class Combination
 */

/**
 * \brief This method binds a port to a pfc profile.
 *
 * This is used only when pfc is being configured on a per port
 * per traffic class.
 *
 * \param [in] devId
 * \param [in] portId
 * \param [in] pfcProfileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcBindPortToPortTcPfcStaticProfile(xpsDevice_t devId,
                                                   uint32_t portId, uint32_t pfcProfileId);

/**
 * \brief This method gets the port profile a port is bound to.
 *
 * This is used only when pfc is being configured on a per port
 * per traffic class.
 *
 * \param [in] devId
 * \param [in] portId
 * \param [out] *pfcProfileId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortToPortTcPfcStaticProfile(xpsDevice_t devId,
                                                  uint32_t portId, uint32_t *pfcProfileId);

/**
 * \brief This method sets the XON threshold for a traffic class and port profile.
 *
 * \param [in] devId
 * \param [in] pfcProfileId
 * \param [in] trafficClass
 * \param [in] xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPortTcPfcStaticXonThresholdForProfile(xpsDevice_t devId,
                                                           uint32_t pfcProfileId, uint32_t trafficClass, uint32_t xonThreshold);

/**
 * \brief This method gets the XON threshold for a traffic class and port profile.
 *
 * \param [in] devId
 * \param [in] pfcProfileId
 * \param [in] trafficClass
 * \param [out] *xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortTcPfcStaticXonThresholdForProfile(xpsDevice_t devId,
                                                           uint32_t pfcProfileId, uint32_t trafficClass, uint32_t *xonThreshold);

/**
 * \brief This method sets the XOFF threshold for a traffic class and port profile.
 *
 * \param [in] devId
 * \param [in] pfcProfileId
 * \param [in] trafficClass
 * \param [in] xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPortTcPfcStaticXoffThresholdForProfile(xpsDevice_t devId,
                                                            uint32_t pfcProfileId, uint32_t trafficClass, uint32_t xoffThreshold);

/**
 * \brief This method gets the XOFF threshold for a traffic class and port profile.
 *
 * \param [in] devId
 * \param [in] pfcProfileId
 * \param [in] trafficClass
 * \param [out] *xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortTcPfcStaticXoffThresholdForProfile(xpsDevice_t devId,
                                                            uint32_t pfcProfileId, uint32_t trafficClass, uint32_t *xoffThreshold);

/**
 * \brief This method sets the XON threshold for a traffic class
 *        (ie: priority) on a per port basis
 *
 * In some devices in the XPA family, the progrmaming model to
 * configure PFC thresholds statically are on a per profile
 * basis, where a PFC port/tc profile must first be bound to a
 * port and then the profile has its thresholds configured.
 * Examples of devices that use this model are:
 *  - XP80
 *
 * In other devices, the programming model to configure PFC
 * thresholds statically are on a per port/tc basis, which means
 * that every combination of port and pfc traffic class (or
 * priority) can have a unique configuration of thresholds.
 * Examples of devices that use this model are:
 *  - XP70
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] pfcTrafficClass
 * \param [in] xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPortTcPfcStaticXonThreshold(xpDevice_t devId,
                                                 xpPort_t devPort, uint32_t pfcTrafficClass, uint32_t xonThreshold);

/**
 * \brief This method gets the XON threshold for a traffic class
 *        (ie: priority) on a per port basis
 *
 * In some devices in the XPA family, the progrmaming model to
 * configure PFC thresholds statically are on a per profile
 * basis, where a PFC port/tc profile must first be bound to a
 * port and then the profile has its thresholds configured.
 * Examples of devices that use this model are:
 *  - XP80
 *
 * In other devices, the programming model to configure PFC
 * thresholds statically are on a per port/tc basis, which means
 * that every combination of port and pfc traffic class (or
 * priority) can have a unique configuration of thresholds.
 * Examples of devices that use this model are:
 *  - XP70
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] pfcTrafficClass
 * \param [out] xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortTcPfcStaticXonThreshold(xpDevice_t devId,
                                                 xpPort_t devPort, uint32_t pfcTrafficClass, uint32_t *xonThreshold);

/**
 * \brief This method sets the XOFF threshold for a traffic
 *        class (ie: priority) on a per port basis
 *
 * In some devices in the XPA family, the progrmaming model to
 * configure PFC thresholds statically are on a per profile
 * basis, where a PFC port/tc profile must first be bound to a
 * port and then the profile has its thresholds configured.
 * Examples of devices that use this model are:
 *  - XP80
 *
 * In other devices, the programming model to configure PFC
 * thresholds statically are on a per port/tc basis, which means
 * that every combination of port and pfc traffic class (or
 * priority) can have a unique configuration of thresholds.
 * Examples of devices that use this model are:
 *  - XP70
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] pfcTrafficClass
 * \param [in] xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPortTcPfcStaticXoffThreshold(xpDevice_t devId,
                                                  xpPort_t devPort, uint32_t pfcTrafficClass, uint32_t xoffThreshold);

/**
 * \brief This method gets the XOFF threshold for a traffic
 *        class (ie: priority) on a per port basis
 *
 * In some devices in the XPA family, the progrmaming model to
 * configure PFC thresholds statically are on a per profile
 * basis, where a PFC port/tc profile must first be bound to a
 * port and then the profile has its thresholds configured.
 * Examples of devices that use this model are:
 *  - XP80
 *
 * In other devices, the programming model to configure PFC
 * thresholds statically are on a per port/tc basis, which means
 * that every combination of port and pfc traffic class (or
 * priority) can have a unique configuration of thresholds.
 * Examples of devices that use this model are:
 *  - XP70
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] pfcTrafficClass
 * \param [out] xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortTcPfcStaticXoffThreshold(xpDevice_t devId,
                                                  xpPort_t devPort, uint32_t pfcTrafficClass, uint32_t *xoffThreshold);

/*
 * PFC: Global Static Threshold section
 */

/**
 * \brief This method sets the global XON threshold
 *
 * \param [in] devId
 * \param [in] xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetGlobalPfcStaticXonThreshold(xpsDevice_t devId,
                                                 uint32_t xonThreshold);

/**
 * \brief This method gets the global XON threshold
 *
 * \param [in] devId
 * \param [out] *xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetGlobalPfcStaticXonThreshold(xpsDevice_t devId,
                                                 uint32_t *xonThreshold);

/**
 * \brief This method sets the global XOFF threshold
 *
 * \param [in] devId
 * \param [in] xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetGlobalPfcStaticXoffThreshold(xpsDevice_t devId,
                                                  uint32_t xoffThreshold);

/**
 * \brief This method gets the global XOFF threshold
 *
 * \param [in] devId
 * \param [out] *xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetGlobalPfcStaticXoffThreshold(xpsDevice_t devId,
                                                  uint32_t *xoffThreshold);

/*
 * PFC: Port Group/Tc Static Threshold section
 */

/**
 * \brief This method sets the XON threshold for a traffic class
 *        and portGroup profile.
 *
 * \param [in] devId
 * \param [in] portPfcGroupPflId
 * \param [in] trafficClass
 * \param [in] xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPortPfcGroupTcStaticXonThreshold(xpsDevice_t devId,
                                                      uint32_t portPfcGroupPflId, uint32_t trafficClass, uint32_t xonThreshold);

/**
 * \public
 * \brief This method gets the XON threshold for a traffic class
 *        and portGroup profile.
 *
 * \param [in] devId
 * \param [in] portPfcGroupPflId
 * \param [in] trafficClass
 * \param [out] *xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortPfcGroupTcStaticXonThreshold(xpsDevice_t devId,
                                                      uint32_t portPfcGroupPflId, uint32_t trafficClass, uint32_t *xonThreshold);

/**
 * \brief This method sets the XOFF threshold for a traffic
 *        class and portGroup profile.
 *
 * \param [in] devId
 * \param [in] portPfcGroupPflId
 * \param [in] trafficClass
 * \param [in] xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPortPfcGroupTcStaticXoffThreshold(xpsDevice_t devId,
                                                       uint32_t portPfcGroupPflId, uint32_t trafficClass, uint32_t xoffThreshold);

/**
 * \brief This method gets the XOFF threshold for a traffic
 *        class and portGroup profile.
 *
 * \param [in] devId
 * \param [in] portPfcGroupPflId
 * \param [in] trafficClass
 * \param [out] *xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortPfcGroupTcStaticXoffThreshold(xpsDevice_t devId,
                                                       uint32_t portPfcGroupPflId, uint32_t trafficClass, uint32_t *xoffThreshold);

/*
 * 802.1Qbb Priority Flow Control Port Configuration APIs
 */

/**
 * \public
 * \brief This method will set an XON Threshold in pages for a
 *        port globally
 *
 * There are several thresholds that are compared against when
 * making a decision to resume admittance of new packets, this
 * threshold is set for a port globally in the device
 *
 * \param [in] devId
 * \param [in] port
 * \param [in] xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPortPfcXonThreshold(xpDevice_t devId, uint32_t port,
                                         uint32_t xonThreshold);

/**
 * \public
 * \brief This method will get the configured XON Threshold in
 *        pages for a port globally
 *
 * There are several thresholds that are compared against when
 * making a decision to resume admittance of new packets, this
 * threshold is set for a port globally in the device
 *
 * \param [in] devId
 * \param [in] port
 * \param [out] *xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortPfcXonThreshold(xpDevice_t devId, uint32_t port,
                                         uint32_t *xonThreshold);

/**
 * \public
 * \brief This method will set an XOFF Threshold in pages for a
 *        port globally
 *
 * There are several thresholds that are compared against when
 * making a decision to resume admittance of new packets, this
 * threshold is set for a port globally in the device
 *
 * \param [in] devId
 * \param [in] port
 * \param [in] xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPortPfcXoffThreshold(xpDevice_t devId, uint32_t port,
                                          uint32_t xoffThreshold);

/**
 * \public
 * \brief This method will get the configured XOFF Threshold in
 *        pages for a port globally
 *
 * There are several thresholds that are compared against when
 * making a decision to resume admittance of new packets, this
 * threshold is set for a port globally in the device
 *
 * \param [in] devId
 * \param [in] port
 * \param [out] *xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortPfcXoffThreshold(xpDevice_t devId, uint32_t port,
                                          uint32_t *xoffThreshold);

/*
 * Configure 802.1Qbb PFC Dynamic Thresholds and bind Dynamic Pools for a Port/Traffic Class Combination
 */

/**
 * \brief This API is used to enable or disable a device's PFC
 *        Dynamic Thresholding functionality
 *
 * PFC Dynamic Thresholding is the name of the feature that
 * enables using a dynamicaly thresholding pool of shared
 * resources for lossless traffic.
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] trafficClass
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcEnablePfcDynamicThresholding(xpDevice_t devId,
                                               uint32_t devPort, uint32_t trafficClass, uint32_t enable);

/**
 * \brief This API is used to get whether or not a device's PFC
 *        Dynamic Thresholding functionality is enabled
 *
 * PFC Dynamic Thresholding is the name of the feature that
 * enables using a dynamicaly thresholding pool of shared
 * resources for lossless traffic.
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] trafficClass
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPfcDynamicThresholdingEnable(xpDevice_t devId,
                                                  uint32_t devPort, uint32_t trafficClass, uint32_t *enable);

/**
 * \brief This method will bind one of the supported Internal
 *        PFC Traffic Classes to a Dynamic Pool
 *
 * This API will bind a PFC internal Traffic Class to a Dynamic
 * Pool. Once done, all active source ports with Lossless
 * Priorities can use this Dynamic Pool to increase their Max
 * Packet Memory utilization prior to sending PFC Pause Frames
 * back to source port
 *
 * PFC Dynamic Thresholding operates in a similar model as
 * Dynamic Tail Drop Thresholding. In that a pool of packet
 * memory can be shared across multiple Source Ports, Traffic
 * Class nodes with respect to buffer accounting
 *
 * The Dynamic Pool will allow for a single source port, tc PFC
 * node to be able to use a larger share of the packet memory
 * under certain circumstances
 *
 * What this allows is for a better utilization of the packet
 * memory allocated for lossless traffic by providing a fair
 * method of sharing a set of packet memory pages. A single
 * port, tc may be the source port for many flows going to many
 * destination ports and may require a larger number of pages
 * associated with it
 *
 * Binding a TC to a pool will allow for sharing of the ingress
 * packet buffer accounting scheme to provide for both a fair
 * use of a shared packet memory and for a larger capacity for a
 * single source port, tc PFC node before requiring to send PFC
 * Pause frames (XOFF) to the source
 *
 * This improves performance because we can wait a longer period
 * of time by virtue of this elastic buffer before having to
 * send a pause frame, resulting in fewer pause frames sent to
 * the source port. This decreases the overall latency of a
 * lossless flow
 *
 *
 * \param [in] devId
 * \param [in] pfcTrafficClass
 * \param [in] dynPoolId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcBindPfcTcToDynamicPool(xpsDevice_t devId,
                                         uint32_t pfcTrafficClass, uint32_t dynPoolId);

/**
 * \brief This method get the binding of one of the supported
 *        Internal PFC Traffic Classes to a Dynamic Pool
 *
 * This API will return the binding of an Internal PFC Traffic
 * Class to a Dynamic Pool
 *
 * PFC Dynamic Thresholding operates in a similar model as
 * Dynamic Tail Drop Thresholding. In that a pool of packet
 * memory can be shared across multiple Source Ports, Traffic
 * Class nodes with respect to buffer accounting
 *
 * The Dynamic Pool will allow for a single source port, tc PFC
 * node to be able to use a larger share of the packet memory
 * under certain circumstances
 *
 * What this allows is for a better utilization of the packet
 * memory allocated for lossless traffic by providing a fair
 * method of sharing a set of packet memory pages. A single
 * port, tc may be the source port for many flows going to many
 * destination ports and may require a larger number of pages
 * associated with it
 *
 * Binding a TC to a pool will allow for sharing of the ingress
 * packet buffer accounting scheme to provide for both a fair
 * use of a shared packet memory and for a larger capacity for a
 * single source port, tc PFC node before requiring to send PFC
 * Pause frames (XOFF) to the source
 *
 * This improves performance because we can wait a longer period
 * of time by virtue of this elastic buffer before having to
 * send a pause frame, resulting in fewer pause frames sent to
 * the source port. This decreases the overall latency of a
 * lossless flow
 *
 *
 * \param [in] devId
 * \param [in] pfcTrafficClass
 * \param [out] dynPoolId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPfcTcToDynamicPoolBinding(xpsDevice_t devId,
                                               uint32_t pfcTrafficClass, uint32_t *dynPoolId);

/**
 * \brief This method unbinds one of the supported Internal PFC
 *        Traffic Classes from a Dynamic Pool
 *
 * This API will unbind an Internal PFC Traffic Class from a
 * Dynamic Pool
 *
 * This API will disable Lossless Dynamic Pool Accounting for a
 * supported PFC Traffic Class. Unbound PFC Traffic Classes will
 * only use Static Xoff/Xon Thresholds
 *
 * PFC Dynamic Thresholding operates in a similar model as
 * Dynamic Tail Drop Thresholding. In that a pool of packet
 * memory can be shared across multiple Source Ports, Traffic
 * Class nodes with respect to buffer accounting
 *
 * The Dynamic Pool will allow for a single source port, tc PFC
 * node to be able to use a larger share of the packet memory
 * under certain circumstances
 *
 * What this allows is for a better utilization of the packet
 * memory allocated for lossless traffic by providing a fair
 * method of sharing a set of packet memory pages. A single
 * port, tc may be the source port for many flows going to many
 * destination ports and may require a larger number of pages
 * associated with it
 *
 * Binding a TC to a pool will allow for sharing of the ingress
 * packet buffer accounting scheme to provide for both a fair
 * use of a shared packet memory and for a larger capacity for a
 * single source port, tc PFC node before requiring to send PFC
 * Pause frames (XOFF) to the source
 *
 * This improves performance because we can wait a longer period
 * of time by virtue of this elastic buffer before having to
 * send a pause frame, resulting in fewer pause frames sent to
 * the source port. This decreases the overall latency of a
 * lossless flow
 *
 *
 * \param [in] devId
 * \param [in] pfcTrafficClass
 * \param [in] dynPoolId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcUnbindPfcTcFromDynamicPool(xpsDevice_t devId,
                                             uint32_t pfcTrafficClass, uint32_t dynPoolId);


/**
 * \public
 * \brief This method will set a selected Dynamic Pool's Total
 *        Buffer size for Lossless traffic
 *
 * This method is used to configure the total amount of Packet
 * Memory to reserve including the RTT Margin across PFC
 * Internal Traffic Classes that are bound to this specifc
 * Dynamic Pool
 *
 * When carving out a region of a Packet Memory to be used for
 * by Dynamic Pool for Lossless Traffic, XPA requires that a
 * portion of this Dynamic Pool is utilized as Shared, and the
 * rest of this Dynamic Pool consists of Guaranteed Thresholds
 *
 * A Lossless Dynamic Pool can be visualized below:
 *
 * <pre>
 *
 *  LOSSLESS DYNAMIC POOL ID: 1
 *
 *  -----------------------------  <-----
 *  |        RTT MARGIN         |   Total Size = RTT Margin + Shared Size + Guaranteed
 *  |---------------------------|  <-----
 *  |      SHARED: PFC TC 3     |   Shared Size: 2000 Pages
 *  |                           |   Dynamic XOFF/XON
 *  |                           |
 *  |                           |
 *  |---------------------------|  <-----
 *  |   |   |   |   |   |   |   |
 *  | G | G | G | G | G | G | G |  Guaranteed XOFF: 150 Pages
 *  |   |   |   |   |   |   |   |  Guaranteed XON:  75 Pages
 *  -----------------------------
 *
 * </pre>
 *
 * From the diagram we can derive that Dynamic Pool 1 has been
 * reserved for Lossless Traffic coming from Internal PFC
 * Traffic Class 3
 *
 * The total amount of Shared PFC Lossless Packet Memory is 2000
 * Pages, and there are currently 7 ports in the system that are
 * configured to handle Lossless Traffic
 *
 * When configuring the Shared Size of a Lossless Dynamic Pool
 * it's important to consider to also remember to account for
 * the total Round Trip Time (RTT) of a Lossless Flow as a
 * margin of error.
 *
 * So given the following formula for RTT:
 *
 * <pre>
 *     bytes = xoff_threshold * 256 + 2 * MTU + 12 * 64 +
 *              (2 *link_delay * port_speed / 8)
 * </pre>
 *
 * In the above example, a portion of the configured Shared Size
 * of 2000 pages will not include the RTT Margin calculated
 *
 * This API's responsiblity is to configure the Total Size of a
 * Lossless Dynamic Pool in pages including the RTT Margin in
 * its calculation
 *
 * In order to configure the guaranteed portion of this Lossless
 * PFC Dynamic Pool, the Port, PFC TC static profiles are used
 * for some XPA Devices such as XP80. Other XPA devices will use
 * a per port/tc configuration of guarantees
 *
 * \see xpsQosFcBindPortToPortTcPfcStaticProfile
 * \see xpsQosFcSetPortTcPfcStaticXonThresholdForProfile
 * \see xpsQosFcSetPortTcPfcStaticXoffThresholdForProfile
 * \see xpsQosFcSetPortTcPfcStaticXonThreshold
 * \see xpsQosFcSetPortTcPfcStaticXoffThreshold
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [in] poolSizeInPages
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPfcTcDynamicPoolTotalSize(xpsDevice_t devId,
                                               uint32_t dynPoolId, uint32_t poolSizeInPages);

/**
 * \public
 * \brief This method will get a selected Dynamic Pool's Total
 *        Buffer size for Lossless traffic
 *
 * This method is used to configure the total amount of Packet
 * Memory to reserve including the RTT Margin across PFC
 * Internal Traffic Classes that are bound to this specifc
 * Dynamic Pool
 *
 * When carving out a region of a Packet Memory to be used for
 * by Dynamic Pool for Lossless Traffic, XPA requires that a
 * portion of this Dynamic Pool is utilized as Shared, and the
 * rest of this Dynamic Pool consists of Guaranteed Thresholds
 *
 * A Lossless Dynamic Pool can be visualized below:
 *
 * <pre>
 *
 *  LOSSLESS DYNAMIC POOL ID: 1
 *
 *  -----------------------------  <-----
 *  |        RTT MARGIN         |   Total Size = RTT Margin + Shared Size + Guaranteed
 *  |---------------------------|  <-----
 *  |      SHARED: PFC TC 3     |   Shared Size: 2000 Pages
 *  |                           |   Dynamic XOFF/XON
 *  |                           |
 *  |                           |
 *  |---------------------------|  <-----
 *  |   |   |   |   |   |   |   |
 *  | G | G | G | G | G | G | G |  Guaranteed XOFF: 150 Pages
 *  |   |   |   |   |   |   |   |  Guaranteed XON:  75 Pages
 *  -----------------------------
 *
 * </pre>
 *
 * From the diagram we can derive that Dynamic Pool 1 has been
 * reserved for Lossless Traffic coming from Internal PFC
 * Traffic Class 3
 *
 * The total amount of Shared PFC Lossless Packet Memory is 2000
 * Pages, and there are currently 7 ports in the system that are
 * configured to handle Lossless Traffic
 *
 * When configuring the Shared Size of a Lossless Dynamic Pool
 * it's important to consider to also remember to account for
 * the total Round Trip Time (RTT) of a Lossless Flow as a
 * margin of error.
 *
 * So given the following formula for RTT:
 *
 * <pre>
 *     bytes = xoff_threshold * 256 + 2 * MTU + 12 * 64 +
 *              (2 *link_delay * port_speed / 8)
 * </pre>
 *
 * In the above example, a portion of the configured Shared Size
 * of 2000 pages will not include the RTT Margin calculated
 *
 * This API's responsiblity is to configure the Total Size of a
 * Lossless Dynamic Pool in pages including the RTT Margin in
 * its calculation
 *
 * In order to configure the guaranteed portion of this Lossless
 * PFC Dynamic Pool, the Port, PFC TC static profiles are used
 * for some XPA Devices such as XP80. Other XPA devices will use
 * a per port/tc configuration of guarantees
 *
 * \see xpsQosFcBindPortToPortTcPfcStaticProfile
 * \see xpsQosFcSetPortTcPfcStaticXonThresholdForProfile
 * \see xpsQosFcSetPortTcPfcStaticXoffThresholdForProfile
 * \see xpsQosFcSetPortTcPfcStaticXonThreshold
 * \see xpsQosFcSetPortTcPfcStaticXoffThreshold
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [out] poolSizeInPages
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPfcTcDynamicPoolTotalSize(xpsDevice_t devId,
                                               uint32_t dynPoolId, uint32_t *poolSizeInPages);

/**
 * \brief This method will set a selected Dynamic Pool's Shared
 *        Buffer size for Lossless traffic
 *
 * This method is used to configure the amount of Packet Memory
 * to reserve as shared across PFC Internal Traffic Classes that
 * are bound to this specifc Dynamic Pool
 *
 * When carving out a region of a Packet Memory to be used for
 * by Dynamic Pool for Lossless Traffic, XPA requires that a
 * portion of this Dynamic Pool is utilized as Shared, and the
 * rest of this Dynamic Pool consists of Guaranteed Thresholds
 *
 * A Lossless Dynamic Pool can be visualized below:
 *
 * <pre>
 *
 *  LOSSLESS DYNAMIC POOL ID: 1
 *
 *  -----------------------------  <-----
 *  |        RTT MARGIN         |   Total Size = RTT Margin + Shared Size + Guaranteed
 *  |---------------------------|  <-----
 *  |      SHARED: PFC TC 3     |   Shared Size: 2000 Pages
 *  |                           |   Dynamic XOFF/XON
 *  |                           |
 *  |                           |
 *  |---------------------------|  <-----
 *  |   |   |   |   |   |   |   |
 *  | G | G | G | G | G | G | G |  Guaranteed XOFF: 150 Pages
 *  |   |   |   |   |   |   |   |  Guaranteed XON:  75 Pages
 *  -----------------------------
 *
 * </pre>
 *
 * From the diagram we can derive that Dynamic Pool 1 has been
 * reserved for Lossless Traffic coming from Internal PFC
 * Traffic Class 3
 *
 * The total amount of Shared PFC Lossless Packet Memory is 2000
 * Pages, and there are currently 7 ports in the system that are
 * configured to handle Lossless Traffic
 *
 * When configuring the Shared Size of a Lossless Dynamic Pool
 * it's important to consider to also remember to account for
 * the total Round Trip Time (RTT) of a Lossless Flow as a
 * margin of error.
 *
 * So given the following formula for RTT:
 *
 * <pre>
 *     bytes = xoff_threshold * 256 + 2 * MTU + 12 * 64 +
 *              (2 *link_delay * port_speed / 8)
 * </pre>
 *
 * This RTT margin of error should be reflected
 * in the Total Size of this Dynamic Pool which can be set using
 *
 * \see xpsQosAqmSetDynamicPoolTotalThreshold
 *
 * In the above example, a portion of the configured Shared Size
 * of 2000 pages will not include the RTT Margin calculated
 *
 * This API's responsiblity is to configure the Shared Size of a
 * Lossless Dynamic Pool in pages
 *
 * In order to configure the guaranteed portion of this Lossless
 * PFC Dynamic Pool, the Port, PFC TC static profiles are used
 *
 * \see xpsQosFcBindPortToPortTcPfcStaticProfile
 * \see xpsQosFcSetPortTcPfcStaticXonThreshold
 * \see xpsQosFcSetPortTcPfcStaticXoffThreshold
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [in] poolSizeInPages
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPfcTcDynamicPoolSharedSize(xpsDevice_t devId,
                                                uint32_t dynPoolId, uint32_t poolSizeInPages);

/**
 * \brief This method will get a selected Dynamic Pool's Shared
 *        Buffer size for Lossless traffic
 *
 * This method is used to retreive the configuration of the
 * amount of Packet Memory that was reserved as shared across
 * PFC Internal Traffic Classes that are bound to this specifc
 * Dynamic Pool
 *
 * When carving out a region of a Packet Memory to be used for
 * by Dynamic Pool for Lossless Traffic, XPA requires that a
 * portion of this Dynamic Pool is utilized as Shared, and the
 * rest of this Dynamic Pool consists of Guaranteed Thresholds
 *
 * A Lossless Dynamic Pool can be visualized below:
 *
 * <pre>
 *
 *  LOSSLESS DYNAMIC POOL ID: 1
 *
 *  -----------------------------  <-----
 *  |        RTT MARGIN         |   Total Size = RTT Margin + Shared Size + Guaranteed
 *  |---------------------------|  <-----
 *  |      SHARED: PFC TC 3     |   Shared Size: 2000 Pages
 *  |                           |   Dynamic XOFF/XON
 *  |                           |
 *  |                           |
 *  |---------------------------|  <-----
 *  |   |   |   |   |   |   |   |
 *  | G | G | G | G | G | G | G |  Guaranteed XOFF: 150 Pages
 *  |   |   |   |   |   |   |   |  Guaranteed XON:  75 Pages
 *  -----------------------------
 *
 * </pre>
 *
 * From the diagram we can derive that Dynamic Pool 1 has been
 * reserved for Lossless Traffic coming from Internal PFC
 * Traffic Class 3
 *
 * The total amount of Shared PFC Lossless Packet Memory is 2000
 * Pages, and there are currently 7 ports in the system that are
 * configured to handle Lossless Traffic
 *
 * When configuring the Shared Size of a Lossless Dynamic Pool
 * it's important to consider to also remember to account for
 * the total Round Trip Time (RTT) of a Lossless Flow as a
 * margin of error.
 *
 * So given the following formula for RTT:
 *
 * <pre>
 *     bytes = xoff_threshold * 256 + 2 * MTU + 12 * 64 +
 *              (2 *link_delay * port_speed / 8)
 * </pre>
 *
 * This RTT margin of error should be reflected
 * in the Total Size of this Dynamic Pool which can be set using
 *
 * \see xpsQosAqmSetDynamicPoolTotalThreshold
 *
 * In the above example, a portion of the configured Shared Size
 * of 2000 pages will not include the RTT Margin calculated
 *
 * This API's responsiblity is to configure the Shared Size of a
 * Lossless Dynamic Pool in pages
 *
 * In order to configure the guaranteed portion of this Lossless
 * PFC Dynamic Pool, the Port, PFC TC static profiles are used
 *
 * \see xpsQosFcBindPortToPortTcPfcStaticProfile
 * \see xpsQosFcSetPortTcPfcStaticXonThreshold
 * \see xpsQosFcSetPortTcPfcStaticXoffThreshold
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [out] poolSizeInPages
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPfcTcDynamicPoolSharedSize(xpsDevice_t devId,
                                                uint32_t dynPoolId, uint32_t *poolSizeInPages);

/**
 * \brief This method will set a selected Dynamic Pool's
 *        Guaranteed Buffer size for Lossless traffic
 *
 * This method is used to configure the total amount of Packet
 * Memory to reserve as guaranteed for each node bound to a PFC
 * Dynamic Pool
 *
 * When carving out a region of a Packet Memory to be used for
 * by Dynamic Pool for Lossless Traffic, XPA requires that a
 * portion of this Dynamic Pool is utilized as Shared, and the
 * rest of this Dynamic Pool consists of Guaranteed Thresholds
 *
 * A Lossless Dynamic Pool can be visualized below:
 *
 * <pre>
 *
 *  LOSSLESS DYNAMIC POOL ID: 1
 *
 *  -----------------------------  <-----
 *  |        RTT MARGIN         |   Total Size = RTT Margin + Shared Size + Guaranteed
 *  |---------------------------|  <-----
 *  |      SHARED: PFC TC 3     |   Shared Size: 2000 Pages
 *  |                           |   Dynamic XOFF/XON
 *  |                           |
 *  |                           |
 *  |---------------------------|  <-----
 *  |   |   |   |   |   |   |   |
 *  | G | G | G | G | G | G | G |  Guaranteed XOFF: 150 Pages
 *  |   |   |   |   |   |   |   |  Guaranteed XON:  75 Pages
 *  -----------------------------
 *
 * </pre>
 *
 * From the diagram we can derive that Dynamic Pool 1 has been
 * reserved for Lossless Traffic coming from Internal PFC
 * Traffic Class 3
 *
 * The total amount of Shared PFC Lossless Packet Memory is 2000
 * Pages, and there are currently 7 ports in the system that are
 * configured to handle Lossless Traffic
 *
 * When configuring the Shared Size of a Lossless Dynamic Pool
 * it's important to consider to also remember to account for
 * the total Round Trip Time (RTT) of a Lossless Flow as a
 * margin of error.
 *
 * So given the following formula for RTT:
 *
 * <pre>
 *     bytes = xoff_threshold * 256 + 2 * MTU + 12 * 64 +
 *              (2 *link_delay * port_speed / 8)
 * </pre>
 *
 * This RTT margin of error should be reflected
 * in the Total Size of this Dynamic Pool which can be set using
 *
 * \see xpsQosFcSetPfcTcDynamicPoolTotalSize
 *
 * In the above example, a portion of the configured Shared Size
 * of 2000 pages will not include the RTT Margin calculated
 *
 * This API's responsiblity is to configure the Shared Size of a
 * Lossless Dynamic Pool in pages
 *
 * In order to configure the guaranteed portion of this Lossless
 * PFC Dynamic Pool, the Port, PFC TC static profiles are used
 * for some XPA Devices such as XP80. Other XPA devices will use
 * a per port/tc configuration of guarantees
 *
 * \see xpsQosFcBindPortToPortTcPfcStaticProfile
 * \see xpsQosFcSetPortTcPfcStaticXonThresholdForProfile
 * \see xpsQosFcSetPortTcPfcStaticXoffThresholdForProfile
 * \see xpsQosFcSetPortTcPfcStaticXonThreshold
 * \see xpsQosFcSetPortTcPfcStaticXoffThreshold
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [in] poolSizeInPages
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPfcTcDynamicPoolGuaranteedSize(xpsDevice_t devId,
                                                    uint32_t dynPoolId, uint32_t poolSizeInPages);

/**
 * \brief This method will get a selected Dynamic Pool's
 *        Guaranteed Buffer size for Lossless traffic
 *
 * This method is used to retrieve the the total amount of
 * Packet Memory to reserve as guaranteed for each node bound to
 * a PFC Dynamic Pool
 *
 * This method is used to configure the amount of Packet Memory
 * to reserve as shared across PFC Internal Traffic Classes that
 * are bound to this specifc Dynamic Pool
 *
 * When carving out a region of a Packet Memory to be used for
 * by Dynamic Pool for Lossless Traffic, XPA requires that a
 * portion of this Dynamic Pool is utilized as Shared, and the
 * rest of this Dynamic Pool consists of Guaranteed Thresholds
 *
 * A Lossless Dynamic Pool can be visualized below:
 *
 * <pre>
 *
 *  LOSSLESS DYNAMIC POOL ID: 1
 *
 *  -----------------------------  <-----
 *  |        RTT MARGIN         |   Total Size = RTT Margin + Shared Size + Guaranteed
 *  |---------------------------|  <-----
 *  |      SHARED: PFC TC 3     |   Shared Size: 2000 Pages
 *  |                           |   Dynamic XOFF/XON
 *  |                           |
 *  |                           |
 *  |---------------------------|  <-----
 *  |   |   |   |   |   |   |   |
 *  | G | G | G | G | G | G | G |  Guaranteed XOFF: 150 Pages
 *  |   |   |   |   |   |   |   |  Guaranteed XON:  75 Pages
 *  -----------------------------
 *
 * </pre>
 *
 * From the diagram we can derive that Dynamic Pool 1 has been
 * reserved for Lossless Traffic coming from Internal PFC
 * Traffic Class 3
 *
 * The total amount of Shared PFC Lossless Packet Memory is 2000
 * Pages, and there are currently 7 ports in the system that are
 * configured to handle Lossless Traffic
 *
 * When configuring the Shared Size of a Lossless Dynamic Pool
 * it's important to consider to also remember to account for
 * the total Round Trip Time (RTT) of a Lossless Flow as a
 * margin of error.
 *
 * So given the following formula for RTT:
 *
 * <pre>
 *     bytes = xoff_threshold * 256 + 2 * MTU + 12 * 64 +
 *              (2 *link_delay * port_speed / 8)
 * </pre>
 *
 * This RTT margin of error should be reflected
 * in the Total Size of this Dynamic Pool which can be set using
 *
 * \see xpFlowControlMgr::setPfcTcDynamicPoolTotalSize
 *
 * In the above example, a portion of the configured Shared Size
 * of 2000 pages will not include the RTT Margin calculated
 *
 * This API's responsiblity is to configure the Shared Size of a
 * Lossless Dynamic Pool in pages
 *
 * In order to configure the guaranteed portion of this Lossless
 * PFC Dynamic Pool, the Port, PFC TC static profiles are used
 * for some XPA Devices such as XP80. Other XPA devices will use
 * a per port/tc configuration of guarantees. How
 *
 * \see xpsQosFcBindPortToPortTcPfcStaticProfile
 * \see xpsQosFcSetPortTcPfcStaticXonThresholdForProfile
 * \see xpsQosFcSetPortTcPfcStaticXoffThresholdForProfile
 * \see xpsQosFcSetPortTcPfcStaticXonThreshold
 * \see xpsQosFcSetPortTcPfcStaticXoffThreshold
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [out] poolSizeInPages
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPfcTcDynamicPoolGuaranteedSize(xpsDevice_t devId,
                                                    uint32_t dynPoolId, uint32_t *poolSizeInPages);

/**
 * \brief This method will configure an "Alpha" value for a
 *        selected Dynamic Pool
 *
 * This API will configure an "Alpha" value for a given Dynamic
 * Pool
 *
 * This Alpha value is used as a coeffcient in the following
 * formula which will be used to determine the Dynamic XOFF
 * Threshold for a given Source Port, PFC TC
 *
 * <pre>
 *
 * xoff_dyn_thld = alpha(PFC_dp_size - PFC_dp_used_pages)
 *
 * </pre>
 *
 * This formula effectively caps the maximum utilization of a
 * single PFC Node's (Source Port, Traffic Class accounting
 * node) utlization of the Shared Portion of the Dynamic Pool
 *
 * The "Alpha" value plays a role of a coefficient in a ratio
 * calculation indicating how much a single PFC Node can use of
 * a shared region
 *
 * Depending on the value of "Alpha", a single PFC Node can use
 * either a very large portion of the pool, or a very small
 * portion. The "Alpha" parameter effectively provides a method
 * of fair utilization of this Shared Packet Memory Region
 *
 * The following graph demonstrates the effect the "Alpha"
 * coefficient has on fair utilization of a Lossless Dynamic
 * Pool
 *
 * <pre>
 *
 *  Avg    |
 *  PFC    |  |--|
 *  PM     |  |++|
 *  util   |  |++|  |--|
 *  per    |  |++|  |++|
 *  active |  |++|  |++|  |--|
 *  node   |  |++|  |++|  |++|
 *         |  |++|  |++|  |++|  |--|
 *         |  |++|  |++|  |++|  |++|  |--|
 *         |  |++|  |++|  |++|  |++|  |++|  |--|  |--|
 *        -|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--
 *         |   1     2     3     4     5     6     7
 *
 *          Number of PFC Nodes using the Dynamic Pool
 *
 * </pre>
 *
 * In this scheme, the approximate amount of Packet Memory each
 * PFC Node will utilize when the system is in a quiescent state
 * is:
 *
 * <pre>
 *                                              alpha
 * PFC_node_util_avg = max(xoff_guar_thld, --------------- * DP_Size)
 *                                          1 + (alpha)N
 *
 * Where N is the number of Active PFC Nodes using this Dynamic Pool
 *
 * </pre>
 *
 * Another aspect of using an "Alpha" Coefficient to dynamically
 * cap a Single PFC Node's utilization of the Packet Memory is
 * that we can guarantee that there is at least some space left
 * for a newly active PFC Node to account for new ingresing
 * traffic
 *
 * This principle follows this equation:
 *
 * <pre>
 *                            1
 * DP_avg_free_space = --------------- * DP_Size
 *                      1 + (alpha)N
 *
 * Where N is the number of Active PFC Nodes using this Dynamic Pool
 *
 * </pre>
 *
 * The decision on when to send an Xoff frame to a source port
 * for a given lossless TC is made when the following equation
 * is satisifed:
 *
 * <pre>
 *
 * xoff_thld = max(xoff_dyn_thld, xoff_guar_thld)
 *
 * </pre>
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [in] fraction
 * \param [in] coefficient
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcConfigurePfcTcDynamicPoolAlpha(xpsDevice_t devId,
                                                 uint32_t dynPoolId, xpDynThldFraction_e fraction, xpDynThldCoeff_e coefficient);

/**
 * \brief This method will retrieve a configured "Alpha" value
 *        for a selected Dynamic Pool
 *
 * This API will retrieve a configured "Alpha" value for a given
 * Dynamic Pool
 *
 * This Alpha value is used as a coeffcient in the following
 * formula which will be used to determine the Dynamic XOFF
 * Threshold for a given Source Port, PFC TC
 *
 * <pre>
 *
 * xoff_dyn_thld = alpha(PFC_dp_size - PFC_dp_used_pages)
 *
 * </pre>
 *
 * This formula effectively caps the maximum utilization of a
 * single PFC Node's (Source Port, Traffic Class accounting
 * node) utlization of the Shared Portion of the Dynamic Pool
 *
 * The "Alpha" value plays a role of a coefficient in a ratio
 * calculation indicating how much a single PFC Node can use of
 * a shared region
 *
 * Depending on the value of "Alpha", a single PFC Node can use
 * either a very large portion of the pool, or a very small
 * portion. The "Alpha" parameter effectively provides a method
 * of fair utilization of this Shared Packet Memory Region
 *
 * The following graph demonstrates the effect the "Alpha"
 * coefficient has on fair utilization of a Lossless Dynamic
 * Pool
 *
 * <pre>
 *
 *  Avg    |
 *  PFC    |  |--|
 *  PM     |  |++|
 *  util   |  |++|  |--|
 *  per    |  |++|  |++|
 *  active |  |++|  |++|  |--|
 *  node   |  |++|  |++|  |++|
 *         |  |++|  |++|  |++|  |--|
 *         |  |++|  |++|  |++|  |++|  |--|
 *         |  |++|  |++|  |++|  |++|  |++|  |--|  |--|
 *        -|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--
 *         |   1     2     3     4     5     6     7
 *
 *          Number of PFC Nodes using the Dynamic Pool
 *
 * </pre>
 *
 * In this scheme, the approximate amount of Packet Memory each
 * PFC Node will utilize when the system is in a quiescent state
 * is:
 *
 * <pre>
 *                                              alpha
 * PFC_node_util_avg = max(xoff_guar_thld, --------------- * DP_Size)
 *                                          1 + (alpha)N
 *
 * Where N is the number of Active PFC Nodes using this Dynamic Pool
 *
 * </pre>
 *
 * Another aspect of using an "Alpha" Coefficient to dynamically
 * cap a Single PFC Node's utilization of the Packet Memory is
 * that we can guarantee that there is at least some space left
 * for a newly active PFC Node to account for new ingresing
 * traffic
 *
 * This principle follows this equation:
 *
 * <pre>
 *                            1
 * DP_avg_free_space = --------------- * DP_Size
 *                      1 + (alpha)N
 *
 * Where N is the number of Active PFC Nodes using this Dynamic Pool
 *
 * </pre>
 *
 * The decision on when to send an Xoff frame to a source port
 * for a given lossless TC is made when the following equation
 * is satisifed:
 *
 * <pre>
 *
 * xoff_thld = max(xoff_dyn_thld, xoff_guar_thld)
 *
 * </pre>
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [out] fraction
 * \param [out] coefficient
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPfcTcDynamicPoolAlpha(xpsDevice_t devId,
                                           uint32_t dynPoolId, xpDynThldFraction_e *fraction,
                                           xpDynThldCoeff_e *coefficient);

/**
 * \public
 * \brief This API will enable or disable calculating the XON
 *        threshold dynamically based off the dynamically
 *        calculated XOFF threshold
 *
 * When using a Dynamic Pool to increase burst tolerance for
 * lossless traffic by dynamically calculating XOFF thresholds,
 * some devices in the XPA family also support dynamically
 * deriving the XON threshold based on the currently calculated
 * XOFF threshold
 *
 * This feature usually requires configuring an XON to XOFF
 * ratio parameter which can be configured using:
 *
 * \see xpsQosFcConfigurePfcTcDynamicPoolXoffToXonRatio
 *
 * However, some devices in the XPA family support turning this
 * feature off and reverting back to a statically configured XON
 * threshold on a per node basis
 *
 * There are pros and cons to each approach:
 * - When disabled, traffic will be less bursty, but lossless
 *   latency may be greater
 * - When enabled, traffic will be bursty, but lossless latency
 *   may be lower
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] pfcTrafficClass
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPfcTcDynamicXonCalculationEnable(xpDevice_t devId,
                                                      xpPort_t devPort, uint32_t pfcTrafficClass, uint32_t enable);

/**
 * \public
 * \brief This API will get whether or not calculating the XON
 *        threshold dynamically based off the dynamically
 *        calculated XOFF threshold is enabled
 *
 * When using a Dynamic Pool to increase burst tolerance for
 * lossless traffic by dynamically calculating XOFF thresholds,
 * some devices in the XPA family also support dynamically
 * deriving the XON threshold based on the currently calculated
 * XOFF threshold
 *
 * This feature usually requires configuring an XON to XOFF
 * ratio parameter which can be configured using:
 *
 * \see xpsQosFcConfigurePfcTcDynamicPoolXoffToXonRatio
 *
 * However, some devices in the XPA family support turning this
 * feature off and reverting back to a statically configured XON
 * threshold on a per node basis
 *
 * There are pros and cons to each approach:
 * - When disabled, traffic will be less bursty, but lossless
 *   latency may be greater
 * - When enabled, traffic will be bursty, but lossless latency
 *   may be lower
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] pfcTrafficClass
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPfcTcDynamicXonCalculationEnable(xpDevice_t devId,
                                                      xpPort_t devPort, uint32_t pfcTrafficClass, uint32_t *enable);

/**
 * \brief This API will configure a desired Xoff to Xon Ratio
 *        used to determine the Dynamic Xon Threshold
 *
 * Considering using a Dynamic Pool to allow for a fairer
 * utilization of a Shared Packet Memory section results in a
 * Dynamic Xoff Threshold, there needs to be some method of
 * configuring the Xon Threshold dynamically as well
 *
 * As stated in \ref fcpfc0, the Xoff Threshold and Xon window
 * form a window which is usually around 1 Round Trip Time of
 * propagation delay worth of Buffer, we can consider the Xon
 * Threshold a function of the RTT
 *
 * Since the Xoff threshold is usually also a function of RTT,
 * it can be stated that the Xon threshold is a function of the
 * Xoff threshold by the transitive property
 *
 * Knowing this, there is a way to configure the parameters of a
 * simple equation to calculate a Dynamic Xon Threshold based
 * off of the calculated Dynamic Xoff Threshold using the
 * "Alpha" parameter
 *
 * \see xpsQosFcConfigurePfcTcDynamicPoolAlpha
 *
 * The equation used to calculate the Xoff Threshold dynamically
 * in the XPA architecture takes three parameters:
 *      - Beta
 *      - Gamma
 *      - Operator
 *
 * These paramters are used as follows:
 *
 * <pre>
 *
 *  xon_dyn_thld = (beta +/- gamma) * xoff_dyn_thld
 *
 * </pre>
 *
 * This provides a way of configuring what amounts to a dynamic
 * sliding window of hysteresis based off the current dyanmic
 * utilization of the Shared Lossless buffer amongst various PFC
 * Nodes
 *
 * This diagram illustrates the sliding window in effect based
 * off of the current overall utilization of the Lossless
 * Dynamic Pool
 *
 * <pre>
 *
 * With 1 PFC Node using the Lossless Dynamic Pool with a Shared size of 1000 pages
 *
 *      ========
 *      |      |
 *      |      |   /\
 *      | XOFF |   |  TX XOFF Pause Frame   Xoff Dyn Thld = 500 Pages
 *      |      |
 *      |      |
 *      |      |
 *      | XON  |   |  TX XON Pause Frames   Xon Dyn Thld  = 250 Pages (1/2 ratio)
 *      |      |  \/                        Beta = 1
 *      |      |                            Gamma = 1/2
 *      |      |                            Operator = -
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      ========
 *
 * With 2 PFC Nodes using the Lossless Dynamic Pool
 *
 *      ========
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      |      |   /\
 *      | XOFF |   |  TX XOFF Pause Frames  Xoff Dyn Thld = 333 Pages
 *      |      |
 *      |      |
 *      |      |
 *      | XON  |   |  TX XON Pause Frame    Xon Dyn Thld = 166 Pages (1/2 ratio)
 *      |      |  \/                        Beta = 1
 *      |      |                            Gamma = 1/2
 *      |      |                            Operator = -
 *      |      |
 *      |      |
 *      ========
 *
 * </pre>
 *
 * The Xon threshold used by the XPA PFC Algorithm follows this
 * equation:
 *
 * <pre>
 *
 * xon_thld = max(xon_guar_thld, xon_dyn_thld)
 *
 * </pre>
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [in] operation
 * \param [in] betaCoeff
 * \param [in] gammaCoeff
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcConfigurePfcTcDynamicPoolXoffToXonRatio(xpsDevice_t devId,
                                                          uint32_t dynPoolId, xpDynThldOperator_e operation, xpDynThldCoeff_e betaCoeff,
                                                          xpDynThldCoeff_e gammaCoeff);

/**
 * \brief This API will retrieve a configured Xoff to Xon Ratio
 *        used to determine the Dynamic Xon Threshold
 *
 * Considering using a Dynamic Pool to allow for a fairer
 * utilization of a Shared Packet Memory section results in a
 * Dynamic Xoff Threshold, there needs to be some method of
 * configuring the Xon Threshold dynamically as well
 *
 * As stated in \ref fcpfc0, the Xoff Threshold and Xon window
 * form a window which is usually around 1 Round Trip Time of
 * propagation delay worth of Buffer, we can consider the Xon
 * Threshold a function of the RTT
 *
 * Since the Xoff threshold is usually also a function of RTT,
 * it can be stated that the Xon threshold is a function of the
 * Xoff threshold by the transitive property
 *
 * Knowing this, there is a way to configure the parameters of a
 * simple equation to calculate a Dynamic Xon Threshold based
 * off of the calculated Dynamic Xoff Threshold using the
 * "Alpha" parameter
 *
 * \see xpsQosFcConfigurePfcTcDynamicPoolAlpha
 *
 * The equation used to calculate the Xoff Threshold dynamically
 * in the XPA architecture takes three parameters:
 *      - Beta
 *      - Gamma
 *      - Operator
 *
 * These paramters are used as follows:
 *
 * <pre>
 *
 *  xon_dyn_thld = (beta +/- gamma) * xoff_dyn_thld
 *
 * </pre>
 *
 * This provides a way of configuring what amounts to a dynamic
 * sliding window of hysteresis based off the current dyanmic
 * utilization of the Shared Lossless buffer amongst various PFC
 * Nodes
 *
 * This diagram illustrates the sliding window in effect based
 * off of the current overall utilization of the Lossless
 * Dynamic Pool
 *
 * <pre>
 *
 * With 1 PFC Node using the Lossless Dynamic Pool with a Shared size of 1000 pages
 *
 *      ========
 *      |      |
 *      |      |   /\
 *      | XOFF |   |  TX XOFF Pause Frame   Xoff Dyn Thld = 500 Pages
 *      |      |
 *      |      |
 *      |      |
 *      | XON  |   |  TX XON Pause Frames   Xon Dyn Thld  = 250 Pages (1/2 ratio)
 *      |      |  \/                        Beta = 1
 *      |      |                            Gamma = 1/2
 *      |      |                            Operator = -
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      ========
 *
 * With 2 PFC Nodes using the Lossless Dynamic Pool
 *
 *      ========
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      |      |
 *      |      |   /\
 *      | XOFF |   |  TX XOFF Pause Frames  Xoff Dyn Thld = 333 Pages
 *      |      |
 *      |      |
 *      |      |
 *      | XON  |   |  TX XON Pause Frame    Xon Dyn Thld = 166 Pages (1/2 ratio)
 *      |      |  \/                        Beta = 1
 *      |      |                            Gamma = 1/2
 *      |      |                            Operator = -
 *      |      |
 *      |      |
 *      ========
 *
 * </pre>
 *
 * The Xon threshold used by the XPA PFC Algorithm follows this
 * equation:
 *
 * <pre>
 *
 * xon_thld = max(xon_guar_thld, xon_dyn_thld)
 *
 * </pre>
 *
 * \param [in] devId
 * \param [in] dynPoolId
 * \param [out] operation
 * \param [out] betaCoeff
 * \param [out] gammaCoeff
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPfcTcDynamicPoolXoffToXonRatio(xpsDevice_t devId,
                                                    uint32_t dynPoolId, xpDynThldOperator_e *operation, xpDynThldCoeff_e *betaCoeff,
                                                    xpDynThldCoeff_e *gammaCoeff);

/**
 * \brief Per-dynamic pool shared used
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t dynPoolId
 * \param [out] uint32_t* counter
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPfcDynamicPoolCurrentSharedUsed(xpDevice_t devId,
                                                     uint32_t dynPoolId, uint32_t *counter);

/**
 * \brief Maximum value observed in the per-dynamic pool shared
 *        used since it was last reset
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t dynPoolId
 * \param [in] uint32_t clearOnRead
 * \param [out] uint32_t* counter
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPfcDynamicPoolMaxSharedUsed(xpDevice_t devId,
                                                 uint32_t dynPoolId, uint32_t clearOnRead, uint32_t *counter);

/*
 * 802.1X: Flow Control API Section
 */

/*
 * Configure 802.1X Flow Control Thresholds for a Single Port
 */

/**
 * \brief This method sets the XON threshold for a port.
 *
 * \param [in] devId
 * \param [in] port
 * \param [in] xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPortFcXonThreshold(xpsDevice_t devId, uint32_t port,
                                        uint32_t xonThreshold);

/**
 * \brief This method gets the XON threshold for a port.
 *
 * \param [in] devId
 * \param [in] port
 * \param [out] xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortFcXonThreshold(xpsDevice_t devId, uint32_t port,
                                        uint32_t *xonThreshold);

/**
 * \brief This method sets the XOFF threshold for a port.
 *
 * \param [in] devId
 * \param [in] port
 * \param [in] xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPortFcXoffThreshold(xpsDevice_t devId, uint32_t port,
                                         uint32_t xoffThreshold);

/**
 * \brief This method gets the XOFF threshold for a port.
 *
 * \param [in] devId
 * \param [in] port
 * \param [out] xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortFcXoffThreshold(xpsDevice_t devId, uint32_t port,
                                         uint32_t *xoffThreshold);

/**
 * \brief This method sets the FULL threshold for a port.
 *
 * \param [in] devId
 * \param [in] port
 * \param [in] fullThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPortFcFullThreshold(xpsDevice_t devId, uint32_t port,
                                         uint32_t fullThreshold);

/**
 * \brief This method gets the FULL threshold for a port.
 *
 * \param [in] devId
 * \param [in] port
 * \param [out] fullThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortFcFullThreshold(xpsDevice_t devId, uint32_t port,
                                         uint32_t *fullThreshold);

/*
 * Configure 802.1X Flow Control Thresholds for a Port Group
 */

/**
 * \brief This method includes Port to Logical Port Group Mapping
 *
 * \param [in] devId
 * \param [in] port
 * \param [in] portFcGroup
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcBindPortToPortFcGroup(xpsDevice_t devId, uint32_t port,
                                        uint32_t portFcGroup);

/**
 * \brief This method gets Port to Logical Port Group Mapping
 *
 * \param [in] devId
 * \param [in] port
 * \param [out] portFcGroup
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortToPortFcGroupBinding(xpsDevice_t devId, uint32_t port,
                                              uint32_t *portFcGroup);

/**
 * \brief This method sets the XON threshold for a port FC group
 *
 * \param [in] devId
 * \param [in] portFcGroup
 * \param [in] xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPortFcGroupXonThreshold(xpsDevice_t devId,
                                             uint32_t portFcGroup, uint32_t xonThreshold);

/**
 * \brief This method gets the XON threshold for a port FC group.
 *
 * \param [in] devId
 * \param [in] portFcGroup
 * \param [out] xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortFcGroupXonThreshold(xpsDevice_t devId,
                                             uint32_t portFcGroup, uint32_t *xonThreshold);

/**
 * \brief This method sets the XOFF threshold for a port FC group
 *
 * \param [in] devId
 * \param [in] portFcGroup
 * \param [in] xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPortFcGroupXoffThreshold(xpsDevice_t devId,
                                              uint32_t portFcGroup, uint32_t xoffThreshold);

/**
 * \brief This method gets the XOFF threshold for a port FC group.
 *
 * \param [in] devId
 * \param [in] portFcGroup
 * \param [out] xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortFcGroupXoffThreshold(xpsDevice_t devId,
                                              uint32_t portFcGroup, uint32_t *xoffThreshold);

/**
 * \brief This method sets the FULL threshold for a port FC group
 *
 * \param [in] devId
 * \param [in] portFcGroup
 * \param [in] fullThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetPortFcGroupFullThreshold(xpsDevice_t devId,
                                              uint32_t portFcGroup, uint32_t fullThreshold);

/**
 * \brief This method gets the FULL threshold for a port FC group.
 *
 * \param [in] devId
 * \param [in] portFcGroup
 * \param [out] fullThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPortFcGroupFullThreshold(xpsDevice_t devId,
                                              uint32_t portFcGroup, uint32_t *fullThreshold);

/*
 * Configure 802.1X Flow Control Global Thresholds
 */

/**
 * \brief This method sets the Global XON threshold.
 *
 * \param [in] devId
 * \param [in] xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetGlobalFcXonThreshold(xpsDevice_t devId,
                                          uint32_t xonThreshold);

/**
 * \brief This method gets the Global XON threshold.
 *
 * \param [in] devId
 * \param [out] xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetGlobalFcXonThreshold(xpsDevice_t devId,
                                          uint32_t *xonThreshold);

/**
 * \brief This method sets the Global XOFF threshold.
 *
 * \param [in] devId
 * \param [in] xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetGlobalFcXoffThreshold(xpsDevice_t devId,
                                           uint32_t xoffThreshold);

/**
 * \brief This method gets the Global XOFF threshold.
 *
 * \param [in] devId
 * \param [out] xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetGlobalFcXoffThreshold(xpsDevice_t devId,
                                           uint32_t *xoffThreshold);

/**
 * \brief This method sets the Global FULL threshold.
 *
 * \param [in] devId
 * \param [in] fullThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcSetGlobalFcFullThreshold(xpsDevice_t devId,
                                           uint32_t fullThreshold);

/**
 * \brief This method gets the Global FULL threshold.
 *
 * \param [in] devId
 * \param [out] fullThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetGlobalFcFullThreshold(xpsDevice_t devId,
                                           uint32_t *fullThreshold);

/**
 * \brief Per-port per-tc pfc counter
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t  pfcTrafficClass
 * \param [out] uint32_t* counter
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetPfcPortTcCounter(xpDevice_t devId, uint32_t portNum,
                                      uint32_t pfcTrafficClass, uint32_t *counter);

/**
 * \brief Counts the number of pages used since last PFC/Pause
 *        XOFF was sent.
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t priority
 * \param [in] uint32_t clearOnRead
 * \param [out] uint64_t *count
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetHeadroomCount(xpDevice_t devId, uint32_t portNum,
                                   uint32_t priority, uint32_t clearOnRead, uint64_t *count);

/**
 * \brief Tracks the maximum number ever observed in the
 *        corresponding headroom counter.
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t priority
 * \param [in] uint32_t clearOnRead
 * \param [out] uint64_t *count
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetMaxHeadroomCount(xpDevice_t devId, uint32_t portNum,
                                      uint32_t priority, uint32_t clearOnRead, uint64_t *count);

/**
 * \brief This config is used for MAD NIC protection which is
 *        only supported on some XP devices.
 *
 * A page counter is incremented when a PFC XOFF event is last
 * sent. Whenever this counter value is equal to or greater to
 * this configurable page_cnt_threshold, any new incoming
 * packets is dropped.
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t priority
 * \param [in] uint32_t threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcConfigureMaxHeadroomThreshold(xpDevice_t devId,
                                                uint32_t portNum, uint32_t priority, uint32_t threshold);

/**
 * \brief This config is used for MAD NIC protection which is
 *        only supported on some XP devices.
 *
 * A page counter is incremented when a PFC XOFF event is last
 * sent. Whenever this counter value is equal to or greater to
 * this configurable page_cnt_threshold, any new incoming
 * packets is dropped.
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t priority
 * \param [out] uint32_t* threshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosFcGetMaxHeadroomThreshold(xpDevice_t devId, uint32_t portNum,
                                          uint32_t priority, uint32_t *threshold);

/*
    IBUFFER PARSER CONFIGURATIONS
*/

/**
 * \brief Set MAC_DA/MAC_DA_MASK for MAC DA based priority
 *        assignment
 *
 * - Sets the MAC DA which is used to match against the
 * packet's MAC DA for priority assignment.
 * - Sets the MAC DA Mask which is used to apply to the packet's
 *   MAC DA before matching against the configured MAC DA
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t profileId
 * \param [in] macAddr_t macDa
 * \param [in] macAddr_t macDaMask
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigureGlobalMacDaForPriorityAssignment(xpsDevice_t devId,
                                                            uint32_t profileId, macAddr_t macDa, macAddr_t macDaMask);

/**
 * \brief Get MAC_DA/MAC_DA_MASK for MAC DA based priority
 *        assignment
 *
 * - Gets the MAC DA which is used to match against the
 * packet's MAC DA for priority assignment.
 * - Gets the MAC DA Mask which is used to apply to the packet's
 *   MAC DA before matching against the configured MAC DA
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t profileId
 * \param [out] macAddr_t macDa
 * \param [out] macAddr_t macDaMask
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetGlobalMacDaForPriorityAssignment(xpsDevice_t devId,
                                                      uint32_t profileId, macAddr_t macDa, macAddr_t macDaMask);

/**
 * \brief Enable MAC DA matching for MAC DA priority assignment
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t profileId
 * \param [in] uint32_t priorityEn
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigureGlobalMacDaPriorityEnable(xpsDevice_t devId,
                                                     uint32_t profileId, uint32_t priorityEn);

/**
 * \brief Get enable bit whether matching of MAC DA is enabled
 *        or not
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t profileId
 * \param [out] uint32_t* priorityEn
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetGlobalMacDaPriorityEnable(xpsDevice_t devId,
                                               uint32_t profileId, uint32_t *priorityEn);

/**
 * \brief Sets PFC priority assigned to the packet if there is a
 *        MAC DA match. If multiple matches occur, the TC
 *        associated with the highest index is used
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t profileId
 * \param [in] uint32_t priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigureGlobalMacDaPriority(xpsDevice_t devId,
                                               uint32_t profileId, uint32_t priority);

/**
 * \brief Gets PFC priority assigned to the packet if there is a
 *        MAC DA match.
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t profileId
 * \param [out] uint32_t* priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetGlobalMacDaPriority(xpsDevice_t devId, uint32_t profileId,
                                         uint32_t *priority);

/**
 * \brief Sets ethertype value to match against the ethertype
 *        field of the packet for priority assignment.
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t profileId
 * \param [in] uint32_t etherType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigureGlobalEthertypeForPriorityAssignment(
    xpsDevice_t devId, uint32_t profileId, uint32_t etherType);

/**
 * \brief Gets ethertype value to match against the ethertype
 *        field of the packet for priority assignment.
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t profileId
 * \param [out] uint32_t* etherType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetGlobalEthertypeForPriorityAssignment(xpsDevice_t devId,
                                                          uint32_t profileId, uint32_t *etherType);

/**
 * \brief Enable ethertype matching for priority assignment
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t profileId
 * \param [in] uint32_t priorityEn
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigureGlobalEthertypePriorityEnable(xpsDevice_t devId,
                                                         uint32_t profileId, uint32_t priorityEn);

/**
 * \brief Get enable bit whether matching of ethertype is
 *        enabled or not
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t profileId
 * \param [out] uint32_t* priorityEn
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetGlobalEthertypePriorityEnable(xpsDevice_t devId,
                                                   uint32_t profileId, uint32_t *priorityEn);

/**
 * \brief Set PFC TC assigned to the packet if there is an
 *        Ethertype patch. If multiple matches occur, the TC
 *        associated with the highest index is used
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t profileId
 * \param [in] uint32_t priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigureGlobalEthertypePriority(xpsDevice_t devId,
                                                   uint32_t profileId, uint32_t priority);

/**
 * \brief Get PFC TC assigned to the packet if there is an
 *        Ethertype patch.
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t profileId
 * \param [out] uint32_t* priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetGlobalEthertypePriority(xpsDevice_t devId,
                                             uint32_t profileId, uint32_t *priority);

/**
 * \brief Enable MPLS based priority assignment
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigurePortMplsPriorityEnable(xpsDevice_t devId,
                                                  uint32_t portNum, uint32_t enable);

/**
 * \brief Get whether MPLS matching is enabled or not
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [out] uint32_t* enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetPortMplsPriorityEnable(xpsDevice_t devId, uint32_t portNum,
                                            uint32_t *enable);

/**
 * \brief Map 3-bit EXP field in the MPLS header to the actual
 *        3-bit value used for PFC priority assignment
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t exp
 * \param [in] uint32_t priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigurePortMplsPriority(xpsDevice_t devId, uint32_t portNum,
                                            uint32_t exp, uint32_t priority);

/**
 * \brief Get PFC priority assigned mapped to the given EXP
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t exp
 * \param [out] uint32_t* priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetPortMplsPriority(xpsDevice_t devId, uint32_t portNum,
                                      uint32_t exp, uint32_t *priority);

/**
 * \brief Enable L3 based priority assignment
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigurePortL3PriorityEnable(xpsDevice_t devId,
                                                uint32_t portNum, uint32_t enable);

/**
 * \brief Get whether L3 matching is enabled or not
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [out] uint32_t* enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetPortL3PriorityEnable(xpsDevice_t devId, uint32_t portNum,
                                          uint32_t *enable);

/**
 * \brief Map 6-bit DSCP field in the IPv4/IPv6 header to the
 *        actual 3-bit value used for PFC priority assignment
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t dscp
 * \param [in] uint32_t priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigurePortL3Priority(xpsDevice_t devId, uint32_t portNum,
                                          uint32_t dscp, uint32_t priority);

/**
 * \brief Get PFC priority assigned mapped to the given DSCP
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t dscp
 * \param [out] uint32_t* priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetPortL3Priority(xpsDevice_t devId, uint32_t portNum,
                                    uint32_t dscp, uint32_t *priority);

/**
 * \brief Enable L2 based priority assignment
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigurePortL2PriorityEnable(xpsDevice_t devId,
                                                uint32_t portNum, uint32_t enable);

/**
 * \brief Get whether L2 matching is enabled or not
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [out] uint32_t* enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetPortL2PriorityEnable(xpsDevice_t devId, uint32_t portNum,
                                          uint32_t *enable);

/**
 * \brief Map 3-bit PCP field in the L2 header Tag to the
 *        actual 3-bit value used for PFC priority assignment
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t pcp
 * \param [in] uint32_t priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigurePortL2Priority(xpsDevice_t devId, uint32_t portNum,
                                          uint32_t pcp, uint32_t priority);

/**
 * \brief Get PFC priority assigned mapped to the given pcp
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t pcp
 * \param [out] uint32_t* priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetPortL2Priority(xpsDevice_t devId, uint32_t portNum,
                                    uint32_t pcp, uint32_t *priority);

/**
 * \brief The default PFC TC assigned to the packet if there is
 *        no matching
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigurePortDefaultPriority(xpsDevice_t devId,
                                               uint32_t portNum, uint32_t priority);

/**
 * \brief Get the default PFC TC assigned to the packet if there
 *        is no matching
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [out] uint32_t* priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetPortDefaultPriority(xpsDevice_t devId, uint32_t portNum,
                                         uint32_t *priority);

/*
    IBUFFER BUFFER CONFIGURATIONS
*/

/**
 * \brief Configure Lossless XON threshold per port in iBuffer.
 *        XON will be asserted for all priorities to the port.
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigurePortLosslessXonThreshold(xpsDevice_t devId,
                                                    uint32_t portNum, uint32_t xonThreshold);

/**
 * \brief Get Lossless XON threshold per port in iBuffer. XON
 *        will be asserted for all priorities to the port.
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [out] uint32_t* xonThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetPortLosslessXonThreshold(xpsDevice_t devId,
                                              uint32_t portNum, uint32_t *xonThreshold);

/**
 * \brief Configure Lossless XOFF threshold per port in iBuffer.
 *        XOFF will be asserted for all priorities to the port.
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigurePortLosslessXoffThreshold(xpsDevice_t devId,
                                                     uint32_t portNum, uint32_t xoffThreshold);

/**
 * \brief Get Lossless XOFF threshold per port in iBuffer. XOFF
 *        will be asserted for all priorities to the port.
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [out] uint32_t* xoffThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetPortLosslessXoffThreshold(xpsDevice_t devId,
                                               uint32_t portNum, uint32_t *xoffThreshold);

/**
 * \brief Configure Lossless Drop threshold  in iBuffer above
 *        which the lossless traffic is dropped
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t dropThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigurePortLosslessDropThreshold(xpsDevice_t devId,
                                                     uint32_t portNum, uint32_t dropThreshold);

/**
 * \brief Get Lossless Drop threshold in iBuffer above which the
 *        lossless traffic is dropped
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [out] uint32_t* dropThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetPortLosslessDropThreshold(xpsDevice_t devId,
                                               uint32_t portNum, uint32_t *dropThreshold);

/**
 * \brief Configure Lossy drop threshold per port in iBuffer,
 *        priority above which lossy traffic for the
 *        corresponding priority is no longer accepted
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t priority
 * \param [in] uint32_t dropThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbConfigurePortLossyDropThresholdPerPriority(xpsDevice_t devId,
                                                             uint32_t portNum, uint32_t priority, uint32_t dropThreshold);

/**
 * \brief Get Lossy drop threshold per port in iBuffer, priority
 *        above which lossy traffic for the corresponding
 *        priority is no longer accepted
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t priority
 * \param [out] uint32_t* dropThreshold
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetPortLossyDropThresholdPerPriority(xpsDevice_t devId,
                                                       uint32_t portNum, uint32_t priority, uint32_t *dropThreshold);

/**
 * \brief Configure whether the port, priority is lossless or
 *        not
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t priority
 * \param [in] uint32_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbEnableLosslessPerPortPriority(xpsDevice_t devId,
                                                uint32_t portNum, uint32_t priority, uint32_t enable);

/**
 * \brief Get whether the port, priority is lossless or
 *        not
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t priority
 * \param [out] uint32_t* enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosIbGetLosslessConfigPerPortPriority(xpsDevice_t devId,
                                                   uint32_t portNum, uint32_t priority, uint32_t *enable);

/*
 * XPS Port Ingress Qos Map
 */

/**
 * \brief API to configure a traffic class for a given MPLS QOS
 *        Profile
 *
 * This API will allow a user to set a Traffic Class for a given
 * MPLS QOS Profile and EXP value
 *
 * There are 8 QOS profiles for each type of QOS marking
 * (L2/L3/MPLS) supported . Each profile provides the ability to
 * configure EXP to traffic class mapping independently
 *
 *
 * \param [in] devId
 * \param [in] profile
 * \param [in] expVal
 * \param [in] tc
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressSetTrafficClassForMplsQosProfile(xpsDevice_t devId,
                                                            xpQosMapPfl_t profile, uint32_t expVal, uint32_t tc);

/**
 * \brief API to retrieve the configured traffic class for a
 *        given MPLS QOS Profile
 *
 * This API will allow a user to get a Traffic Class for a given
 * MPLS QOS Profile and EXP value
 *
 * There are 8 QOS Profiles for each type of QOS marking
 * (L2/L3/MPLS) supported. Each profile provides the ability to
 * configure EXP to traffic class mapping independently
 *
 * \param [in] devId
 * \param [in] profile
 * \param [in] expVal
 * \param [out] tc
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressGetTrafficClassForMplsQosProfile(xpsDevice_t devId,
                                                            xpQosMapPfl_t profile, uint32_t expVal, uint32_t *tc);

/**
 * \brief API to configure a drop precedence for a given MPLS
 *        QOS Profile
 *
 * This API will allow a user to set a Drop Precedence for a
 * given MPLS QOS Profile and EXP value
 *
 * There are 8 QOS profiles for each type of QOS marking
 * (L2/L3/MPLS) supported. Each profile provides the ability to configure EXP to drop precedence mapping
 * independently
 *
 * \param [in] devId
 * \param [in] profile
 * \param [in] expVal
 * \param [in] dp
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressSetDropPrecedenceForMplsQosProfile(xpsDevice_t devId,
                                                              xpQosMapPfl_t profile, uint32_t expVal, uint32_t dp);

/**
 * \brief API to retrieve the configured drop precedence for a
 *        given MPLS QOS Profile
 *
 * This API will allow a user to get a Drop Precedence for a
 * given MPLS QOS Profile and EXP value
 *
 * There are 8 QOS Profiles for each type of QOS marking
 * (L2/L3/MPLS) supported. Each profile provides the ability to
 * configure EXP to drop precedence mapping independently
 *
 * \param [in] devId
 * \param [in] profile
 * \param [in] dscpVal
 * \param [out] dp
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressGetDropPrecedenceForMplsQosProfile(xpsDevice_t devId,
                                                              xpQosMapPfl_t profile, uint32_t expVal, uint32_t *dp);

/**
 * \brief API to retrieve remapped QOS Priority values based on
 *        an ingress PCP/DEI received from a packet for a
 *        profile
 *
 * This API will allow a user to retrieve the remapped Qos
 * priority values retrieved from a .1q tagged packet or
 * from the port default PCP/DEI values
 *
 * There are 8 QOS Profiles for each type of QOS marking
 * (L2/L3/MPLS) supported. Each profile provides the ability to
 * remap QOS priorities indepdently
 *
 * \param [in] devId
 * \param [in] profile
 * \param [in] pcpVal
 * \param [in] deiVal
 * \param [out] remapPcpVal
 * \param [out] remapDeiVal
 * \param [out] rmapDscpVal
 * \param [out] remapExpVal
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressGetRemappedPriorityForL2QosProfile(xpsDevice_t devId,
                                                              xpQosMapPfl_t profile,
                                                              uint32_t   pcpVal,         uint32_t      deiVal,
                                                              uint32_t  *remapPcpVal,    uint32_t     *remapDeiVal,
                                                              uint32_t  *remapDscpVal,   uint32_t     *remapExpVal);

/**
 * \brief API to retrieve remapped QOS Priority values based on
 *        an ingress DSCP received from a packet for a profile
 *
 * This API will allow a user to retrieve the remapped QOS
 * priority values retrieved from an IP packet or from the port
 * default DSCP value
 *
 * There are 8 QOS Profiles for each type of QOS marking
 * (L2/L3/MPLS) supported. Each profile provides the ability to
 * remap QOS Priorities independently
 *
 * \param [in] devId
 * \param [in] profile
 * \param [in] dscpVal
 * \param [out] remapPcpVal
 * \param [out] remapDeiVal
 * \param [out] remapDscpVal
 * \param [out] remapExpVal
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressGetRemappedPriorityForL3QosProfile(xpsDevice_t devId,
                                                              xpQosMapPfl_t profile, uint32_t dscpVal,
                                                              uint32_t *remapPcpVal, uint32_t *remapDeiVal, uint32_t *remapDscpVal,
                                                              uint32_t *remapExpVal);

/**
 * \brief API to remap QOS Priority values based on an ingress
 *        EXP recieved from a packet for a profile
 *
 * This API will allow a user to remap the QOS Priority values
 * based off the value retrieved from an MPLS packet or from the
 * port default EXP value
 *
 * There are 8 QOS Profiles for each type of QOS marking
 * (L2/L3/MPLS) supported. Each profile provides the ability to
 * remap QOS Priorities independently
 *
 * \param [in] devId
 * \param [in] profile
 * \param [in] expVal
 * \param [in] remapPcpVal
 * \param [in] remapDeiVal
 * \param [in] remapDscpVal
 * \param [in] remapExpVal
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressRemapPriorityForMplsQosProfile(xpsDevice_t devId,
                                                          xpQosMapPfl_t profile, uint32_t expVal,
                                                          uint32_t remapPcpVal, uint32_t remapDeiVal, uint32_t remapDscpVal,
                                                          uint32_t remapExpVal);

/**
 * \brief API to retrieve remapped QOS Priority values based on
 *        an ingress EXP received from a packet for a profile
 *
 * This API will allow a user to retrieve the remapped Qos
 * Priority values retrieved from an MPLS packet or from the
 * port default EXP value
 *
 * There are 8 QOS Profiles for each type of QOS marking
 * (L2/L3/MPLS) supported. Each profile provides the ability to
 * remap QOS Priorities independently
 *
 * \param [in] devId
 * \param [in] profile
 * \param [in] expVal
 * \param [out] remapPcpVal
 * \param [out] remapDeiVal
 * \param [out] remapDscpVal
 * \param [out] remapExpVal
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressGetRemappedPriorityForMplsQosProfile(
    xpsDevice_t devId, xpQosMapPfl_t profile, uint32_t expVal,
    uint32_t *remapPcpVal, uint32_t *remapDeiVal, uint32_t *remapDscpVal,
    uint32_t *remapExpVal);

/**
 * \brief API to set the Port Default PCP/DEI values
 *
 * This API will set the port default PCP/DEI values for all
 * incoming L2 packets
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] pcpVal
 * \param [in] deiVal
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressSetPortDefaultL2QosPriority(xpsDevice_t devId,
                                                       xpPort_t devPort, uint32_t pcpVal, uint32_t deiVal);

/**
 * \brief API to retrieve the Port Default PCP/DEI values
 *
 * This API will get the port default PCP/DEI values for all
 * incoming L2 packets
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [out] pcpVal
 * \param [out] deiVal
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressGetPortDefaultL2QosPriority(xpsDevice_t devId,
                                                       xpPort_t devPort, uint32_t *pcpVal, uint32_t *deiVal);

/**
 * \brief API to get the Port Default DSCP value
 *
 * This API will get the port default DSCP value for all
 * incoming IP packets
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [out] dscpVal
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressGetPortDefaultL3QosPriority(xpsDevice_t devId,
                                                       xpPort_t devPort, uint32_t *dscpVal);

/**
 * \brief API to set the Port Default EXP value
 *
 * This API will set the port default EXP value for all incoming
 * MPLS packets
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] expVal
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressSetPortDefaultMplsQosPriority(xpsDevice_t devId,
                                                         xpPort_t devPort, uint32_t expVal);

/**
 * \brief API to get the Port Default EXP value
 *
 * This API will get the port default EXP value for all incoming
 * MPLS packets
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [out] expVal
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressGetPortDefaultMplsQosPriority(xpsDevice_t devId,
                                                         xpPort_t devPort, uint32_t *expVal);

/**
 * \brief API to get the Port Default Traffic Class
 *
 * This API will get the port default Traffic Class for all
 * incoming packets
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [out] tc
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressGetPortDefaultTrafficClass(xpsDevice_t devId,
                                                      xpPort_t devPort, uint32_t* tc);

/**
 * \brief API to get the Port Default Drop Precedence
 *
 * This API will get the port default Drop Precedence for all
 * incoming packets
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [out] dp
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressGetPortDefaultDropPrecedence(xpsDevice_t devId,
                                                        xpPort_t devPort, uint32_t *dp);

/**
 * \brief API to get the L2 QOS profile for a port
 *
 * This API will get a L2 QOS profile for a port as well as
 * enable or disable its application to all incoming packets
 *
 * There is a precedence that is followed for the QOS Remarking
 * scheme are
 *  - MPLS
 *  - IP
 *  - L2
 *  - Port default
 *
 * If MPLS Profile is enabled, all QOS priorities are taken from
 * the MPLS Profile entry that was hit
 *
 * This API enables the L2 QOS Profile application. If other,
 * higher in precedence, profiles are enabled, they take
 * precedence
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [out] profile
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressGetL2QosProfileForPort(xpsDevice_t devId,
                                                  xpPort_t devPort, xpQosMapPfl_t *profile, uint32_t *enable);

/**
 * \brief API to get the L3 QOS profile for a port
 *
 * This API will get a L3 QOS profile for a port as well as
 * enable or disable its application to all incoming packets
 *
 * There is a precedence that is followed for the QOS Remarking
 * scheme are
 *  - MPLS
 *  - IP
 *  - L2
 *  - Port default
 *
 * If MPLS Profile is enabled, all QOS priorities are taken from
 * the MPLS Profile entry that was hit
 *
 * This API enables the L3 QOS Profile application. If other,
 * higher in precedence, profiles are enabled, they take
 * precedence
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [out] profile
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressGetL3QosProfileForPort(xpsDevice_t devId,
                                                  xpPort_t devPort, xpQosMapPfl_t *profile, uint32_t *enable);

/**
 * \brief API to set the MPLS QOS profile for a port
 *
 * This API will set a MPLS QOS profile for a port as well as
 * enable or disable its application to all incoming packets
 *
 * There is a precedence that is followed for the QOS Remarking
 * scheme are
 *  - MPLS
 *  - IP
 *  - L2
 *  - Port default
 *
 * If MPLS Profile is enabled, all QOS priorities are taken from
 * the MPLS Profile entry that was hit
 *
 * This API enables the MPLS QOS Profile application. If other,
 * higher in precedence, profiles are enabled, they take
 * precedence
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] profile
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressSetMplsQosProfileForPort(xpsDevice_t devId,
                                                    xpPort_t devPort, xpQosMapPfl_t profile, uint32_t enable);

/**
 * \brief API to get the MPLS QOS profile for a port
 *
 * This API will get a MPLS QOS profile for a port as well as
 * enable or disable its application to all incoming packets
 *
 * There is a precedence that is followed for the QOS Remarking
 * scheme are
 *  - MPLS
 *  - IP
 *  - L2
 *  - Port default
 *
 * If MPLS Profile is enabled, all QOS priorities are taken from
 * the MPLS Profile entry that was hit
 *
 * This API enables the MPLS QOS Profile application. If other,
 * higher in precedence, profiles are enabled, they take
 * precedence
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [out] profile
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressGetMplsQosProfileForPort(xpsDevice_t devId,
                                                    xpPort_t devPort, xpQosMapPfl_t *profile, uint32_t *enable);

/**
 * \brief API to enable the MPLS QOS Trust profile
 *
 * This profile by default trusts only the MPLS Priorities and
 * keeps the incoming EXP. Traffic Class and Drop Precedence are
 * not set by this API, they will need to be set separately
 *
 *
 * \param [in] devId
 * \param [in] devPort
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosPortIngressSetTrustMplsForPort(xpsDevice_t devId,
                                               xpPort_t devPort);

/**
 * \brief API to Get PCP/DEI Remark Enable/Disable Globally
 *
 * \param [in] devId
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetPcpDeiRemarkEnable(xpsDevice_t devId, uint32_t *enable);


/**
 * \brief API to Get DSCP Remark Enable/Disable Globally
 *
 * \param [in] devId
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetDscpRemarkEnable(xpsDevice_t devId, uint32_t *enable);


/**
 * \brief API to read the current depth of a Queue associated
 *        with a given port using packet resolution
 *
 * This counter is a running counter, reading this counter will
 * not result in a clear
 *
 * \param [in] devId
 * \param [in] port
 * \param [in] queue
 * \param [out] count
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcGetCurrentQueuePacketDepth(xpsDevice_t devId, xpPort_t port,
                                             uint32_t queue, uint32_t *count);

/**
 * \brief API to read the current depth of a Queue associated
 *        with a given port using page resolution
 *
 * This counter is a running counter, reading this counter will
 * not result in a clear
 *
 * \param [in] devId
 * \param [in] port
 * \param [in] queue
 * \param [out] count
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcGetCurrentQueuePageDepth(xpsDevice_t devId, xpPort_t port,
                                           uint32_t queue, uint32_t *count);

/**
 * \brief API to read the current depth of a port using page resolution
 *
 * This counter is a running counter, reading this counter will
 * not result in a clear
 *
 * \param [in] devId
 * \param [in] port
 * \param [out] count
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcGetCurrentPortPageDepth(xpsDevice_t devId, uint32_t port,
                                          uint32_t *count);

/**
 * \brief API to read the current depth of a h1Node using page resolution
 *
 * This counter is a running counter, reading this counter will
 * not result in a clear
 *
 * \param [in] devId
 * \param [in] port
 * \param [out] count
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcGetCurrentH1PageDepth(xpsDevice_t devId, uint32_t h1Node,
                                        uint32_t *count);

/**
 * \brief API to read the current depth of a h2Node using page resolution
 *
 * This counter is a running counter, reading this counter will
 * not result in a clear
 *
 * \param [in] devId
 * \param [in] port
 * \param [out] count
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcGetCurrentH2PageDepth(xpsDevice_t devId, uint32_t h2Node,
                                        uint32_t *count);

/**
 * \brief API to read the average length of a Queue associated
 *        with a given port using page resolution
 *
 * This counter is a running counter, reading this counter will
 * not result in a clear
 *
 * This counter is used by hardware to implement WRED
 *
 * \param [in] devId
 * \param [in] port
 * \param [in] queue
 * \param [out] count
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcGetQueueAveragePageLength(xpsDevice_t devId, xpPort_t port,
                                            uint32_t queue, uint32_t *count);

/**
 * \brief API to read the old length of a Queue associated with
 *        a given port using page resolution
 *
 * This counter is a running counter, reading this counter will
 * not result in a clear
 *
 * This counter is used by hardware to implement WRED
 *
 * \param [in] devId
 * \param [in] port
 * \param [in] queue
 * \param [out] count
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcGetQueueOldPageLength(xpsDevice_t devId, xpPort_t port,
                                        uint32_t queue, uint32_t *count);

/**
 * \brief API to enable clear on read for packet forward
 *        counters
 *
 * This API will configure clear on read capabilities for each
 * q packet forward counter
 *
 * \param [in] devId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcEnableFwdPktCountClearOnRead(xpsDevice_t devId,
                                               uint32_t enable);

/**
 * \brief API to test if clear on read is configured for packet
 *        forward counters
 *
 * This API will return whether or not clear on read
 * capabilities are configured for each q packet forward counter
 *
 * \param [in] devId
 * \param [out] *enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcIsEnableFwdPktCountClearOnRead(xpsDevice_t devId,
                                                 uint32_t *enable);

/**
 * \brief API to enable clear on read for byte forward counters
 *
 * This API will configure clear on read capabilities for each
 * q byte forward counter
 *
 * \param [in] devId
 * \param [in] *enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcEnableFwdPktByteCountClearOnRead(xpsDevice_t devId,
                                                   uint32_t enable);

/**
 * \brief API to test if clear on read is configured for byte
 *        forward counters
 *
 * This API will return whether or not clear on read
 * capabilities are configured for each q byte forward counter
 *
 * \param [in] devId
 * \param [out] *enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcIsEnableFwdPktByteCountClearOnRead(xpsDevice_t devId,
                                                     uint32_t *enable);

/**
 * \brief API to enable clear on read for packet drop
 *        counters
 *
 * This API will configure clear on read capabilities for each
 * q packet drop counter
 *
 * \param [in] devId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcEnableDropPktCountClearOnRead(xpsDevice_t devId,
                                                uint32_t enable);

/**
 * \brief API to test if clear on read is configured for packet
 *        drop counters
 *
 * This API will return whether or not clear on read
 * capabilities are configured for each q packet drop
 * counter
 *
 * \param [in] devId
 * \param [out] *enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcIsEnableDropPktCountClearOnRead(xpsDevice_t devId,
                                                  uint32_t *enable);

/**
 * \brief API to enable clear on read for byte drop counters
 *
 * This API will configure clear on read capabilities for each
 * q byte drop counter
 *
 * \param [in] devId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcEnableDropPktByteCountClearOnRead(xpsDevice_t devId,
                                                    uint32_t enable);

/**
 * \brief API to test if clear on read is configured for byte
 *        drop counters
 *
 * This API will return whether or not clear on read
 * capabilities are configured for each q byte drop
 * counter
 *
 * \param [in] devId
 * \param [out] *enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcIsEnableDropPktByteCountClearOnRead(xpsDevice_t devId,
                                                      uint32_t *enable);

/**
 * \brief API to read the PFC count of a port and priority
 *        combination
 *
 * This counter is a running counter, reading this counter will
 * not result in a clear
 *
 * This counter is used by hardware to implement PFC
 *
 * \param [in] devId
 * \param [in] port
 * \param [in] priority
 * \param [out] count
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcGetPfcPageCount(xpsDevice_t devId, xpPort_t port,
                                  uint32_t priority, uint32_t *count);

/**
 * \brief
 *
 * \param [in] devId
 * \param [out] count
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcGetGlobalPageCount(xpsDevice_t devId, uint32_t *count);

/**
 * \brief
 *
 * \param [in] devId
 * \param [out] count
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosQcGetGlobalMulticastPageCount(xpsDevice_t devId,
                                              uint32_t *count);

/*
 * XPS Port Shaper Section
 */

/**
 * \brief This API is used to enable the Port Shaping feature
 *
 * This API is used to enable the Port Shapers as a feature.
 * There are individual enables for each port shaper, but they
 * do not have an impact unless shaping at the port as a feature
 * is enabled first
 *
 * \param [in] devId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperEnablePortShaping(xpsDevice_t devId, uint32_t enable);

/**
 * \brief This API is used to retrieve whether or not Port
 *        Shaping as a feature is enabled
 *
 * This API is used to determine whether or not Port Shaping
 * as a feature is enabled for this device
 *
 * \param [in] devId
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperIsPortShapingEnabled(xpsDevice_t devId, uint32_t *enable);

/**
 * \brief This API is used to configure the MTU size of the
 *        token bucket used by Port Shapers
 *
 * The MTU is a granularity used by the XPA token bucket
 * implementation to describe the saturated token bucket size as
 * well as the operational empty token bucket value. For finer
 * grained control over shapers, especially for specific packet
 * sizes, this control might prove useful
 *
 *
 * \param [in] devId
 * \param [in] mtuInBytes
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperSetPortShaperMtu(xpsDevice_t devId, uint32_t mtuInBytes);

/**
 * \brief This API is used to retrieve the MTU size of the token
 *        bucket used by Port Shapers
 *
 * The MTU is a granularity used by the XPA token bucket
 * implementation to describe the saturated token bucket size as
 * well as the operational empty token bucket value. For finer
 * grained control over shapers, especially for specific packet
 * sizes, this control might prove useful
 *
 *
 * \param [in] devId
 * \param [out] mtuInBytes
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperGetPortShaperMtu(xpsDevice_t devId, uint32_t *mtuInBytes);


/**
 * \brief This API is used to set the value of "zero", or
 *        operationally empty token bucket threshold used by
 *        Port Shapers
 *
 * For some devices in the XPA family, there is are separate
 * controls over the shaper token bucket threhsolds for token
 * bucket saturation value and token bucket empty value. For
 * devices that have separate control, this API can be used to
 * set a "zero" or empty threshold value
 *
 *
 * \param [in] devId
 * \param [in] emptyThld
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperSetPortShaperEmptyThreshold(xpsDevice_t devId,
                                                  uint32_t emptyThld);

/**
 * \brief This API is used to get the value of "zero", or
 *        operationally empty token bucket threshold used by
 *        Port Shapers
 *
 * For some devices in the XPA family, there is are separate
 * controls over the shaper token bucket threhsolds for token
 * bucket saturation value and token bucket empty value. For
 * devices that have separate control, this API can be used to
 * set a "zero" or empty threshold value
 *
 *
 * \param [in] devId
 * \param [out] emptyThld
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperGetPortShaperEmptyThreshold(xpsDevice_t devId,
                                                  uint32_t *emptyThld);

/**
 * \brief This API returns a port's shaper configuration
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [out] rateKbps
 * \param [out] maxBurstByteSize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperGetPortShaperConfiguration(xpsDevice_t devId,
                                                 xpPort_t devPort, uint64_t *rateKbps, uint32_t *maxBurstByteSize);

/**
 * \brief This API retrieves whether or not shaping is enabled a
 *        at particular port
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [out] enableShaper
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperGetPortShaperEnable(xpsDevice_t devId, xpPort_t devPort,
                                          uint32_t *enableShaper);

/**
 * \brief This API returns the index of the shaper memory used
 *        to configure this ports shaper attributes
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [out] index
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperGetPortShaperTableIndex(xpsDevice_t devId,
                                              xpPort_t devPort, uint32_t *index);

/*
 * XPS Queue Shaping enable section
 */

/**
 * \brief This API is used to enable the Queue Shaping feature
 *
 * This API is used to enable the Queue Shapers as a feature.
 * There are individual enables for each queue shaper, but they
 * do not have an impact unless shaping at the queue as a feature
 * is enabled first
 *
 * \param [in] devId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperEnableQueueShaping(xpsDevice_t devId, uint32_t enable);

/**
 * \brief This API is used to retrieve whether or not Queue
 *        Shaping as a feature is enabled
 *
 * This API is used to determine whether or not Queue Shaping
 * as a feature is enabled for this device
 *
 * \param [in] devId
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperIsQueueShapingEnabled(xpsDevice_t devId,
                                            uint32_t *enable);

/**
 * \brief This API is used to configure the MTU size of the
 *        token bucket used by Queue Shapers
 *
 * The MTU is a granularity used by the XPA token bucket
 * implementation to describe the saturated token bucket size as
 * well as the operational empty token bucket value. For finer
 * grained control over shapers, especially for specific packet
 * sizes, this control might prove useful
 *
 *
 * \param [in] devId
 * \param [in] mtuInBytes
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperSetQueueShaperMtu(xpsDevice_t devId,
                                        uint32_t mtuInBytes);

/**
 * \brief This API is used to retrieve the MTU size of the token
 *        bucket used by Queue Shapers
 *
 * The MTU is a granularity used by the XPA token bucket
 * implementation to describe the saturated token bucket size as
 * well as the operational empty token bucket value. For finer
 * grained control over shapers, especially for specific packet
 * sizes, this control might prove useful
 *
 *
 * \param [in] devId
 * \param [out] mtuInBytes
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperGetQueueShaperMtu(xpsDevice_t devId,
                                        uint32_t *mtuInBytes);

/**
 * \brief This API is used to set the value of "zero", or
 *        operationally empty token bucket threshold used by
 *        Queue Shapers
 *
 * For some devices in the XPA family, there is are separate
 * controls over the shaper token bucket threhsolds for token
 * bucket saturation value and token bucket empty value. For
 * devices that have separate control, this API can be used to
 * set a "zero" or empty threshold value
 *
 *
 * \param [in] devId
 * \param [in] emptyThld
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperSetQueueShaperEmptyThreshold(xpsDevice_t devId,
                                                   uint32_t emptyThld);

/**
 * \brief This API is used to get the value of "zero", or
 *        operationally empty token bucket threshold used by
 *        Queue Shapers
 *
 * For some devices in the XPA family, there is are separate
 * controls over the shaper token bucket threhsolds for token
 * bucket saturation value and token bucket empty value. For
 * devices that have separate control, this API can be used to
 * set a "zero" or empty threshold value
 *
 *
 * \param [in] devId
 * \param [out] emptyThld
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperGetQueueShaperEmptyThreshold(xpsDevice_t devId,
                                                   uint32_t *emptyThld);

/*
 * XPS Queue Slow Shaper Section
 */

/**
 * \brief This API configures a Queue Slow Shaper
 *
 * There are two types of queue shapers supported for XPA
 *      - Fast
 *      - Slow
 *
 * This API is used to configure a Queue Slow shaper. Each Queue
 * has an associated slow shaper, unlike fast shapers which
 * require a shaper num to queue binding
 *
 * Slow shapers are less accurate than fast shapers due to the
 * implementation of the shaper state as a memory and not flops
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [in] rateKbps
 * \param [in] maxBurstByteSize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperConfigureQueueSlowShaper(xpsDevice_t devId,
                                               xpPort_t devPort, uint32_t queueNum, uint64_t rateKbps,
                                               uint32_t maxBurstByteSize);

/**
 * \brief This API retrieves a Queue's Slow Shaper Configuration
 *
 * There are two types of queue shapers suppoorted in XPA
 *      - Fast
 *      - Slow
 *
 * This API is used to retrieve a Queue Slow Shaper's
 * configuration
 *
 * Slow shapers are less accurate than fast shapers due to the
 * implementaiton of the shaper state as a memory and not flops
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] rateKbps
 * \param [out] maxBurstByteSize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperGetQueueSlowShaperConfiguation(xpsDevice_t devId,
                                                     xpPort_t devPort, uint32_t queueNum, uint64_t *rateKbps,
                                                     uint32_t *maxBurstByteSize);

/**
 * \brief This API enables a Queue's Slow Shaper
 *
 * There are two types of queue shapers supported in XPA
 *      - Fast
 *      - Slow
 *
 * This API is used to enable or disable a Queue Slow Shaper
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [in] enableShaper
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperSetQueueSlowShaperEnable(xpsDevice_t devId,
                                               xpPort_t devPort, uint32_t queueNum, uint32_t enableShaper);

/**
 * \brief This API retrieves a Queue Slow Shaper's enable state
 *
 * There are two types of queue shapers supported in XPA
 *      - Fast
 *      - Slow
 *
 * This API is used to get whether or not a Queue Slow Shaper is
 * enabled
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] enableShaper
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperGetQueueSlowShaperEnable(xpsDevice_t devId,
                                               xpPort_t devPort, uint32_t queueNum, uint32_t *enableShaper);

/**
 * \brief This API retrieves a Queue Slow Shaper's index into
 *        the shaper memory
 *
 * There are two types of queue shapers supported in XPA
 *      - Fast
 *      - Slow
 *
 * This API is used to return the index of the Queue Slow Shaper
 * memory that contains the Slow Shaper configuration for a
 * given port and queue
 *
 * This API is useful for debugging shaper configuration
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] index
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperGetQueueSlowShaperTableIndex(xpsDevice_t devId,
                                                   xpPort_t devPort, uint32_t queueNum, uint32_t *index);

/*
 * XPS Queue Fast Shaper Section
 */

/**
 * \brief This API binds a queue to a fast shaper
 *
 * There are two types of queue shapers supported for XPA
 *      - Fast
 *      - Slow
 *
 * This API is used to bind a fast shaper to a queue. There is a
 * mapping of queues within the confines of a Txq Pipe (channel)
 * to Fast Shapers
 *
 * This API will not check to see if an existing queue is mapped
 * to a shaper index, nor will it maintain a state if a queue is
 * mapped to any index as the sdk is designed to be stateless
 *
 * Fast Shapers are much more accurate in shaping than slow due
 * to their implementation as flops and not a memory
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] fastShaperNum
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperBindQueueToFastShaper(xpsDevice_t devId, xpPort_t devPort,
                                            uint32_t queueNum, uint32_t *fastShaperNum);

/**
 * \brief This API configures a Queue Fast Shaper
 *
 * There are two types of queue shapers supported for XPA
 *      - Fast
 *      - Slow
 *
 * This API is used to configure a Queue Fast shaper. A Fast
 * shaper must be bound to a queue prior to being enabled.
 *
 * This API will not check to see if an existing queue is mapped
 * to a shaper index, nor will it maintain a state if a queue is
 * mapped to any index as the sdk is designed to be stateless
 *
 * Fast Shapers are much more accurate in shaping than slow due
 * to their implementation as flops and not a memory
 *
 *
 * \param [in] devId
 * \param [in] fastShaperNum
 * \param [in] rateKbps
 * \param [in] maxBurstByteSize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperConfigureQueueFastShaper(xpsDevice_t devId,
                                               uint32_t fastShaperNum, uint64_t rateKbps, uint32_t maxBurstByteSize);

/**
 * \brief This API retrieves a Queue's Fast Shaper Configuration
 *
 * There are two types of queue shapers supported for XPA
 *      - Fast
 *      - Slow
 *
 * This API is used to retrieve a Queue Fast shaper
 * configuration. A Fast shaper must be bound to a queue prior
 * to being enabled.
 *
 * This API will not check to see if an existing queue is mapped
 * to a shaper index, nor will it maintain a state if a queue is
 * mapped to any index as the sdk is designed to be stateless
 *
 * Fast Shapers are much more accurate in shaping than slow due
 * to their implementation as flops and not a memory
 *
 *
 * \param [in] devId
 * \param [in] fastShaperNum
 * \param [out] rateKbps
 * \param [out] maxBurstSize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperGetQueueFastShaperConfiguation(xpsDevice_t devId,
                                                     uint32_t fastShaperNum, uint64_t *rateKbps, uint32_t *maxBurstSize);

/**
 * \brief This API enables a Queue's Fast Shaper
 *
 * There are two types of queue shapers supported for XPA
 *      - Fast
 *      - Slow
 *
 * This API is used to enable or disable a Queue Fast shaper. A
 * Fast shaper must be bound to a queue prior to being enabled.
 *
 * This API will not check to see if an existing queue is mapped
 * to a shaper index, nor will it maintain a state if a queue is
 * mapped to any index as the sdk is designed to be stateless
 *
 * Fast Shapers are much more accurate in shaping than slow due
 * to their implementation as flops and not a memory
 *
 * \param [in] devId
 * \param [in] fastShaperNum
 * \param [in] enableShaper
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperSetQueueFastShaperEnable(xpsDevice_t devId,
                                               uint32_t fastShaperNum, uint32_t enableShaper);

/**
 * \brief This API retrieves a Queue's Fast Shaper enable state
 *
 * There are two types of queue shapers supported for XPA
 *      - Fast
 *      - Slow
 *
 * This API is used to retrieve whether or not a Queue Fast
 * shaper is enabled. A Fast shaper must be bound to a queue
 * prior to being enabled.
 *
 * This API will not check to see if an existing queue is mapped
 * to a shaper index, nor will it maintain a state if a queue is
 * mapped to any index as the sdk is designed to be stateless
 *
 * Fast Shapers are much more accurate in shaping than slow due
 * to their implementation as flops and not a memory
 *
 * \param [in] devId
 * \param [in] fastShaperNum
 * \param [out] enableShaper
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperGetQueueFastShaperEnable(xpsDevice_t devId,
                                               uint32_t fastShaperNum, uint32_t *enableShaper);

/**
 * \brief This API retrieves a Queue Fast Shaper's index into
 *        the shaper memory
 *
 * There are two types of queue shapers supported in XPA
 *      - Fast
 *      - Slow
 *
 * This API is used to return the index of the Queue Fast Shaper
 * memory that contains the Slow Shaper configuration for a
 * given port and queue
 *
 * Fast Shapers are much more accurate in shaping than slow due
 * to their implementation as flops and not a memory
 *
 * This API is useful for debugging shaper configuration
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] index
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosShaperGetQueueFastShaperTableIndex(xpsDevice_t devId,
                                                   xpPort_t devPort, uint32_t queueNum, uint32_t *index);

/*
 * XPS Scheduler APIs
 */

/**
 * \brief This method enables DWRR scheduling for a particular port.
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetQueueSchedulerDWRR(xpsDevice_t devId, xpsPort_t devPort,
                                      uint32_t queueNum, uint32_t* enable);

/**
 * \brief This method gets the DWRR weights for a particular
 *        port
 *
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] weight
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetQueueSchedulerDWRRWeight(xpsDevice_t devId,
                                            xpsPort_t devPort, uint32_t queueNum, uint32_t* weight);

/**
 * \brief This method will configure the Queue Shaped DWRR Mtu
 *        value
 *
 * The DWRR implementation in XPA is a shaped DWRR
 * implementation. A Shaped DWRR implementation leads to more
 * control over the rate of traffic that is scheduled out in a
 * weighted round robin manner
 *
 *
 * \param [in] devId
 * \param [in] mtuInBytes
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSetQueueSchedulerDWRRMtu(xpsDevice_t devId,
                                         uint32_t mtuInBytes);

/**
 * \brief This method will retrieve the configured Queue Shaped
 *        DWRR Mtu value
 *
 * The DWRR implementation in XPA is a shaped DWRR
 * implementation. A Shaped DWRR implementation leads to more
 * control over the rate of traffic that is scheduled out in a
 * weighted round robin manner
 *
 * \param [in] devId
 * \param [out] mtuInBytes
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetQueueSchedulerDWRRMtu(xpsDevice_t devId,
                                         uint32_t *mtuInBytes);

/**
 * \brief This method enables Strict Priority scheduling for a
 *        particular port.
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetQueueSchedulerSP(xpsDevice_t devId, xpsPort_t devPort,
                                    uint32_t queueNum, uint32_t* enable);

/**
 * \brief This method gets the SP priority for a particular port
 *
 * \param [in] devId
 * \param [in] devPort
 * \param [in] queueNum
 * \param [out] priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetQueueSchedulerSPPriority(xpsDevice_t devId,
                                            xpsPort_t devPort, uint32_t queueNum, uint32_t *priority);

/**
 * \brief This method sets whether or not a queue is enabled for dequeue
 *
 * \param [in] devId
 * \param [in] portNum
 * \param [in] queueNum
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSetQueueDequeueEnable(xpDevice_t devId, uint32_t portNum,
                                      uint32_t queueNum, uint32_t enable);

/**
 * \brief This method gets whether or not a queue is enabled for dequeue
 *
 * \param [in] devId
 * \param [in] portNum
 * \param [in] queueNum
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetQueueDequeueEnable(xpDevice_t devId, uint32_t portNum,
                                      uint32_t queueNum, uint32_t *enable);

/*
 * H1 Scheduler APIs
 */

/**
 * \public
 * \brief This method enables DWRR scheduling for a particular
 *        H1.
 *
 * \param [in] devId
 * \param [in] path
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSetH1SchedulerDWRR(xpsDevice_t devId,
                                   xpTxqQueuePathToPort_t path, uint32_t enable);

/**
 * \public
 * \brief This method checks if DWRR scheduling for a particular
 *        H1 is enabled.
 *
 * \param [in] devId
 * \param [in] path
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetH1SchedulerDWRR(xpsDevice_t devId,
                                   xpTxqQueuePathToPort_t path, uint32_t* enable);

/**
 * \public
 * \brief This method sets the DWRR weights for a particular
 *        H1.
 *
 * \param [in] devId
 * \param [in] path
 * \param [in] weight
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSetH1SchedulerDWRRWeight(xpsDevice_t devId,
                                         xpTxqQueuePathToPort_t path, uint32_t weight);

/**
 * \public
 * \brief This method sets the DWRR weights for a particular
 *        H1.
 *
 * \param [in] devId
 * \param [in] path
 * \param [in] weight
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetH1SchedulerDWRRWeight(xpsDevice_t devId,
                                         xpTxqQueuePathToPort_t path, uint32_t* weight);

/**
 * \public
 * \brief This method enables Strict Priority scheduling for a
 *        particular H1.
 *
 * \param [in] devId
 * \param [in] path
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSetH1SchedulerSP(xpsDevice_t devId, xpTxqQueuePathToPort_t path,
                                 uint32_t enable);

/**
 * \public
 * \brief This method checks if Strict Priority scheduling for a
 *        particular H1 is enabled.
 *
 * \param [in] devId
 * \param [in] path
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetH1SchedulerSP(xpsDevice_t devId, xpTxqQueuePathToPort_t path,
                                 uint32_t* enable);

/**
 * \public
 * \brief This method sets SP priority for a particular H1.
 *
 * \param [in] devId
 * \param [in] path
 * \param [in] priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSetH1SchedulerSPPriority(xpsDevice_t devId,
                                         xpTxqQueuePathToPort_t path, uint32_t priority);

/**
 * \public
 * \brief This method gets SP priority for a particular H1.
 *
 * \param [in] devId
 * \param [in] path
 * \param [out] priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetH1SchedulerSPPriority(xpsDevice_t devId,
                                         xpTxqQueuePathToPort_t path, uint32_t *priority);

/*
 * H2 Scheduler APIs
 */
/**
 * \public
 * \brief This method enables DWRR scheduling for a particular
 *        H2.
 *
 * \param [in] devId
 * \param [in] path
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSetH2SchedulerDWRR(xpsDevice_t devId,
                                   xpTxqQueuePathToPort_t path, uint32_t enable);

/**
 * \public
 * \brief This method checks if DWRR scheduling for a particular
 *        H2 is enabled.
 *
 * \param [in] devId
 * \param [in] path
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetH2SchedulerDWRR(xpsDevice_t devId,
                                   xpTxqQueuePathToPort_t path, uint32_t* enable);

/**
 * \public
 * \brief This method sets the DWRR weights for a particular H2.
 *
 * \param [in] devId
 * \param [in] path
 * \param [in] weight
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSetH2SchedulerDWRRWeight(xpsDevice_t devId,
                                         xpTxqQueuePathToPort_t path, uint32_t weight);

/**
 * \public
 * \brief This method gets the DWRR weights for a particular
 *        H2
 *
 * \param [in] devId
 * \param [in] path
 * \param [out] weight
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetH2SchedulerDWRRWeight(xpsDevice_t devId,
                                         xpTxqQueuePathToPort_t path, uint32_t* weight);

/**
 * \public
 * \brief This method enables Strict Priority scheduling for a
 *        particular H2.
 *
 * \param [in] devId
 * \param [in] path
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSetH2SchedulerSP(xpsDevice_t devId, xpTxqQueuePathToPort_t path,
                                 uint32_t enable);

/**
 * \public
 * \brief This method checks if Strict Priority scheduling for a
 *        particular H2 is enabled.
 *
 * \param [in] devId
 * \param [in] path
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetH2SchedulerSP(xpsDevice_t devId, xpTxqQueuePathToPort_t path,
                                 uint32_t* enable);

/**
 * \public
 * \brief This method sets SP priority for a particular H2.
 *
 * \param [in] devId
 * \param [in] path
 * \param [in] priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSetH2SchedulerSPPriority(xpsDevice_t devId,
                                         xpTxqQueuePathToPort_t path, uint32_t priority);

/**
 * \public
 * \brief This method gets SP priority for a particular H2.
 *
 * \param [in] devId
 * \param [in] path
 * \param [out] priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetH2SchedulerSPPriority(xpsDevice_t devId,
                                         xpTxqQueuePathToPort_t path, uint32_t *priority);

/*
 * Port Scheduler APIs
 */

/**
 * \public
 * \brief This method enables DWRR scheduling for a particular
 *        port.
 *
 * \param [in] devId
 * \param [in] portNum
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSetPortSchedulerDWRR(xpsDevice_t devId, uint32_t portNum,
                                     uint32_t enable);

/**
 * \public
 * \brief This method checks if DWRR scheduling for a particular
 *        port is enabled.
 *
 * \param [in] devId
 * \param [in] portNum
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetPortSchedulerDWRR(xpsDevice_t devId, uint32_t portNum,
                                     uint32_t* enable);

/**
 * \public
 * \brief This method sets the DWRR weights for a particular
 *        port.
 *
 * \param [in] devId
 * \param [in] portNum
 * \param [in] weight
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSetPortSchedulerDWRRWeight(xpsDevice_t devId, uint32_t portNum,
                                           uint32_t weight);

/**
 * \public
 * \brief This method gets the DWRR weights for a particular
 *        port
 *
 * \param [in] devId
 * \param [in] portNum
 * \param [out] weight
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetPortSchedulerDWRRWeight(xpsDevice_t devId, uint32_t portNum,
                                           uint32_t* weight);

/**
 * \public
 * \brief This method enables Strict Priority scheduling for a
 *        particular port.
 *
 * \param [in] devId
 * \param [in] portNum
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSetPortSchedulerSP(xpsDevice_t devId, uint32_t portNum,
                                   uint32_t enable);

/**
 * \public
 * \brief This method checks if Strict Priority scheduling for a
 *        particular port is enabled.
 *
 * \param [in] devId
 * \param [in] portNum
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetPortSchedulerSP(xpsDevice_t devId, uint32_t portNum,
                                   uint32_t* enable);

/**
 * \public
 * \brief This method sets SP priority for a particular port.
 *
 * \param [in] devId
 * \param [in] portNum
 * \param [in] priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSetPortSchedulerSPPriority(xpsDevice_t devId, uint32_t portNum,
                                           uint32_t priority);

/**
 * \public
 * \brief This method gets SP priority for a particular port.
 *
 * \param [in] devId
 * \param [in] portNum
 * \param [out] priority
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetPortSchedulerSPPriority(xpsDevice_t devId, uint32_t portNum,
                                           uint32_t *priority);

/**
 * \public
 * \brief This method gets the queue to port path
 *
 * \param [in] devId
 * \param [in] portNum
 * \param [in] queueNum
 * \param [out] path
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetQueueToPortPath(xpsDevice_t devId, uint32_t portNum,
                                   uint32_t queueNum, xpTxqQueuePathToPort_t* path);

/**
 * \brief This method sets whether or not a port is enabled for dequeue
 *
 * \param [in] devId
 * \param [in] portNum
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSetPortDequeueEnable(xpDevice_t devId, uint32_t portNum,
                                     uint32_t enable);

/**
 * \brief This method gets whether or not a port is enabled for dequeue
 *
 * \param [in] devId
 * \param [in] portNum
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosGetPortDequeueEnable(xpDevice_t devId, uint32_t portNum,
                                     uint32_t *enable);

/**
 * \brief This method is used to set the pacing interval for
 *        ports operating at a certain speed
 *
 * In XPA, all schedulers operate with a concept of "pace".
 * Pacing is used as one of the "checkmarks" during the
 * arbitration process of a scheduler. A pacer will mask out a
 * node for a fixed number of cycles after it has been scheduled
 * to introduce a well known interval for selection
 *
 * For example, a port operating at 100G with a 600MHz clock can
 * be paced at 4 clocks, which will allow for an egress packet
 * rate of 150 Mpps.
 *  - 600MHz / 4 = 150 Mpps
 *
 * This means that a single port ("node") can be selected once
 * every 4 clocks, even though a scheduler will select a port
 * ("node") on every clock cycle
 *
 * NOTE: Only use this API if instructed by Marvell
 *
 *
 * \param [in] devId
 * \param [in] speed
 * \param [in] numCycles
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSchedulerSetPortPacingInterval(xpsDevice_t devId,
                                               XP_SPEED speed, uint32_t numCycles);

/**
 * \brief This method is used to get the pacing interval for
 *        ports operating at a certain speed
 *
 * In XPA, all schedulers operate with a concept of "pace".
 * Pacing is used as one of the "checkmarks" during the
 * arbitration process of a scheduler. A pacer will mask out a
 * node for a fixed number of cycles after it has been scheduled
 * to introduce a well known interval for selection
 *
 * For example, a port operating at 100G with a 600MHz clock can
 * be paced at 4 clocks, which will allow for an egress packet
 * rate of 150 Mpps.
 *  - 600MHz / 4 = 150 Mpps
 *
 * This means that a single port ("node") can be selected once
 * every 4 clocks, even though a scheduler will select a port
 * ("node") on every clock cycle
 *
 * NOTE: Only use this API if instructed by Marvell
 *
 * \param [in] devId
 * \param [in] speed
 * \param [out] numCycles
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSchedulerGetPortPacingInterval(xpsDevice_t devId,
                                               XP_SPEED speed, uint32_t *numCycles);

/**
 * \brief This method is used to set the pacing interval for an
 *        H2 node
 *
 * In XPA, all schedulers operate with a concept of "pace".
 * Pacing is used as one of the "checkmarks" during the
 * arbitration process of a scheduler. A pacer will mask out a
 * node for a fixed number of cycles after it has been scheduled
 * to introduce a well known interval for selection
 *
 * For example, an H2 servicing traffic at 100G with a 600MHz
 * clock can be paced at 8 clocks, which will allow for an
 * egress packet rate of 75 Mpps.
 *  - 600MHz / 8 = 75 Mpps
 *
 * This means that a single H2 ("node") can be selected once
 * every 8 clocks, even though a scheduler will select a h2
 * ("node") on every clock cycle
 *
 * Please note that 8 clocks may or may not be the default pacer
 * value for H2, we use 8 here as an example
 *
 * NOTE: Only use this API if instructed by Marvell
 *
 * \param [in] devId
 * \param [in] numCycles
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSchedulerSetH2PacingInterval(xpsDevice_t devId,
                                             uint32_t numCycles);

/**
 * \brief This method is used to get the pacing interval for an
 *        H2 node
 *
 * In XPA, all schedulers operate with a concept of "pace".
 * Pacing is used as one of the "checkmarks" during the
 * arbitration process of a scheduler. A pacer will mask out a
 * node for a fixed number of cycles after it has been scheduled
 * to introduce a well known interval for selection
 *
 * For example, an H2 servicing traffic at 100G with a 600MHz
 * clock can be paced at 8 clocks, which will allow for an
 * egress packet rate of 75 Mpps.
 *  - 600MHz / 8 = 75 Mpps
 *
 * This means that a single H2 ("node") can be selected once
 * every 8 clocks, even though a scheduler will select a h2
 * ("node") on every clock cycle
 *
 * Please note that 8 clocks may or may not be the default pacer
 * value for H2, we use 8 here as an example
 *
 * NOTE: Only use this API if instructed by Marvell
 *
 * \param [in] devId
 * \param [out] numCycles
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSchedulerGetH2PacingInterval(xpsDevice_t devId,
                                             uint32_t *numCycles);

/**
 * \brief This method is used to set the pacing interval for an
 *        H1 node
 *
 * In XPA, all schedulers operate with a concept of "pace".
 * Pacing is used as one of the "checkmarks" during the
 * arbitration process of a scheduler. A pacer will mask out a
 * node for a fixed number of cycles after it has been scheduled
 * to introduce a well known interval for selection
 *
 * For example, an H1 servicing traffic at 100G with a 600MHz
 * clock can be paced at 8 clocks, which will allow for an
 * egress packet rate of 75 Mpps.
 *  - 600MHz / 8 = 75 Mpps
 *
 * This means that a single H1 ("node") can be selected once
 * every 8 clocks, even though a scheduler will select a h1
 * ("node") on every clock cycle
 *
 * Please note that 8 clocks may or may not be the default pacer
 * value for H1, we use 8 here as an example
 *
 * NOTE: Only use this API if instructed by Marvell
 *
 * \param [in] devId
 * \param [in] numCycles
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSchedulerSetH1PacingInterval(xpsDevice_t devId,
                                             uint32_t numCycles);

/**
 * \brief This method is used to get the pacing interval for an
 *        H1 node
 *
 * In XPA, all schedulers operate with a concept of "pace".
 * Pacing is used as one of the "checkmarks" during the
 * arbitration process of a scheduler. A pacer will mask out a
 * node for a fixed number of cycles after it has been scheduled
 * to introduce a well known interval for selection
 *
 * For example, an H1 servicing traffic at 100G with a 600MHz
 * clock can be paced at 8 clocks, which will allow for an
 * egress packet rate of 75 Mpps.
 *  - 600MHz / 8 = 75 Mpps
 *
 * This means that a single H1 ("node") can be selected once
 * every 8 clocks, even though a scheduler will select a h1
 * ("node") on every clock cycle
 *
 * Please note that 8 clocks may or may not be the default pacer
 * value for H1, we use 8 here as an example
 *
 * NOTE: Only use this API if instructed by Marvell
 *
 * \param [in] devId
 * \param [out] numCycles
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSchedulerGetH1PacingInterval(xpsDevice_t devId,
                                             uint32_t *numCycles);

/**
 * \brief This method is used to set the pacing interval for an
 *        queue node
 *
 * In XPA, all schedulers operate with a concept of "pace".
 * Pacing is used as one of the "checkmarks" during the
 * arbitration process of a scheduler. A pacer will mask out a
 * node for a fixed number of cycles after it has been scheduled
 * to introduce a well known interval for selection
 *
 * For example, an queue servicing traffic at 100G with a 600MHz
 * clock can be paced at 16 clocks, which will allow for an
 * egress packet rate of 37.5 Mpps.
 *  - 600MHz / 16 = 37.5 Mpps
 *
 * This means that a single Q ("node") can be selected once
 * every 16 clocks, even though a scheduler will select a q
 * ("node") on every clock cycle
 *
 * Please note that 16 clocks may or may not be the default
 * pacer value for Q, we use 16 here as an example
 *
 * NOTE: Only use this API if instructed by Marvell
 *
 * \param [in] devId
 * \param [in] numCycles
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSchedulerSetQueuePacingInterval(xpsDevice_t devId,
                                                uint32_t numCycles);

/**
 * \brief This method is used to get the pacing interval for an
 *        queue node
 *
 * In XPA, all schedulers operate with a concept of "pace".
 * Pacing is used as one of the "checkmarks" during the
 * arbitration process of a scheduler. A pacer will mask out a
 * node for a fixed number of cycles after it has been scheduled
 * to introduce a well known interval for selection
 *
 * For example, an queue servicing traffic at 100G with a 600MHz
 * clock can be paced at 16 clocks, which will allow for an
 * egress packet rate of 37.5 Mpps.
 *  - 600MHz / 16 = 37.5 Mpps
 *
 * This means that a single Q ("node") can be selected once
 * every 16 clocks, even though a scheduler will select a q
 * ("node") on every clock cycle
 *
 * Please note that 16 clocks may or may not be the default
 * pacer value for Q, we use 16 here as an example
 *
 * NOTE: Only use this API if instructed by Marvell
 *
 * \param [in] devId
 * \param [out] numCycles
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSchedulerGetQueuePacingInterval(xpsDevice_t devId,
                                                uint32_t *numCycles);

/*
 * XPS Egress Cos Map APIs
 */

/**
 * \brief API to Control Egress Qos Map Enable/Disable Globally
 *
 * \param [in] devId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosSetEgressQosMapEn(xpsDevice_t devId, uint32_t enable);

/**
 * \brief API to adds a egress cos map entry
 *
 * \param [in] devId
 * \param [in] eVif
 * \param [in] tc
 * \param [in] dp
 * \param [in] map
 * \param [out] indexList
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapAdd(xpsDevice_t devId, xpVif_t eVif, uint32_t tc,
                                uint32_t dp, xpEgressCosMapData_t map, xpsHashIndexList_t *indexList);

/**
 * \brief API to update a egress cos map entry
 *
 * \param [in] devId
 * \param [in] index
 * \param [in] map
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapUpdate(xpsDevice_t devId, uint32_t index,
                                   xpEgressCosMapData_t map);

/**
 * \brief API to get the index for a egress cos map entry
 *
 * \param [in] devId
 * \param [in] eVif
 * \param [in] tc
 * \param [in] dp
 * \param [out] index
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapGetIndex(xpsDevice_t devId, xpVif_t eVif,
                                     uint32_t tc, uint32_t dp, int *index);

/**
 * \brief API to get a egress cos map entry
 *
 * \param [in] devId
 * \param [in] eVif
 * \param [in] tc
 * \param [in] dp
 * \param [out] map
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapGetEntry(xpsDevice_t devId, xpVif_t eVif,
                                     uint32_t tc, uint32_t dp, xpEgressCosMapData_t *map);

/**
 * \brief API to get a egress cos map entry for a given index
 *
 * \param [in] devId
 * \param [in] index
 * \param [out] map
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapGetEntryByIndex(xpsDevice_t devId, uint32_t index,
                                            xpEgressCosMapData_t *map);

/**
 * \brief API to remove a egress cos map entry
 *
 * \param [in] devId
 * \param [in] eVif
 * \param [in] tc
 * \param [in] dp
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapRemoveEntry(xpsDevice_t devId, xpVif_t eVif,
                                        uint32_t tc, uint32_t dp);

/**
 * \brief API to remove a egress cos map entry for a given index
 *
 * \param [in] devId
 * \param [in] index
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapRemoveEntryByIndex(xpsDevice_t devId,
                                               uint32_t index);

/**
 * \brief
 * \brief API to set the PCP/DEI values for a egress cos map entry
 *
 * \param [in] devId
 * \param [in] index
 * \param [in] pcpVal
 * \param [in] deiVal
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapSetL2QosPriority(xpsDevice_t devId, uint32_t index,
                                             uint32_t pcpVal, uint32_t deiVal);

/**
 * \brief API to set the DSCP value for a egress cos map entry
 *
 * \param [in] devId
 * \param [in] index
 * \param [in] dscpVal
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapSetL3QosPriority(xpsDevice_t devId, uint32_t index,
                                             uint32_t dscpVal);

/**
 * \brief API to set the PCP/DEI enable value enable for a egress cos map entry
 *
 * \param [in] devId
 * \param [in] index
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapSetL2QosPriorityEn(xpsDevice_t devId,
                                               uint32_t index, uint32_t enable);

/**
 * \brief API to set the DSCP enable value for a egress cos map entry
 *
 * \param [in] devId
 * \param [in] index
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapSetL3QosPriorityEn(xpsDevice_t devId,
                                               uint32_t index, uint32_t enable);

/**
 * \brief API to get the PCP/DEI values for a egress cos map entry
 *
 * \param [in] devId
 * \param [in] index
 * \param [out] pcpVal
 * \param [out] deiVal
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapGetL2QosPriority(xpsDevice_t devId, uint32_t index,
                                             uint32_t *pcpVal, uint32_t *deiVal);

/**
 * \brief API to get the DSCP value for a egress cos map entry
 *
 * \param [in] devId
 * \param [in] index
 * \param [out] dscpVal
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapGetL3QosPriority(xpsDevice_t devId, uint32_t index,
                                             uint32_t *dscpVal);

/**
 * \brief API to get the PCP/DEI enable value for a egress cos map entry
 *
 * \param [in] devId
 * \param [in] index
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapGetL2QosPriorityEn(xpsDevice_t devId,
                                               uint32_t index, uint32_t *enable);

/**
 * \brief API to get the DSCP enable value for a egress cos map entry
 *
 * \param [in] devId
 * \param [in] index
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapGetL3QosPriorityEn(xpsDevice_t devId,
                                               uint32_t index, uint32_t *enable);

/**
 * \brief API to flush a egress cos map entry for given egress VIF
 *
 * \param [in] devId
 * \param [in] eVif
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapFlushEntryByEgressVif(xpsDevice_t devId,
                                                  xpVif_t vif);

/**
 * \brief API to flush a egress cos map entry for given egress VIF, and
 *        update the SW shadow if updateShadow == 1
 *
 * \param [in] devId
 * \param [in] eVif
 * \param [in] updateShadow
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapFlushEntryByEgressVifWithShadowUpdate(
    xpsDevice_t devId, xpVif_t vif, uint8_t updateShadow);

/**
 * \brief API that initializes the XPS Qos Mgr
 *
 * This API will register for the state databases that are
 * needed by the Qos Mgr code base
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosInit(void);

/**
 * \brief API to De-Init the XPS Qos Mgr
 *
 * This API will Deregister all state databases for Qos Mgr
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosDeInit(void);

/**
 * \brief This method sets rehash level for egress cos map table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapSetRehashLevel(xpsDevice_t devId,
                                           uint8_t numOfRehashLevels);

/**
 * \brief This method gets rehash level for egress cos map table
 *
 * \param [in] devId
 * \param [out] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsQosEgressQosMapGetRehashLevel(xpsDevice_t devId,
                                           uint8_t* numOfRehashLevels);

/**
 * \brief Clear Bucket State in the table for given device
 *
 * \param [in] devId
 * \param [in] tblCopyIdx
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsQosEgressQosMapClearBucketState(xpDevice_t devId,
                                             uint8_t tblCopyIdx);

XP_STATUS xpsRateLimiterDisplayTable(xpsDevice_t devId,
                                     uint32_t *numOfValidEntries, uint32_t startIndex, uint32_t endIndex,
                                     char * logFile, uint32_t detailFormat, uint32_t silentMode,
                                     uint32_t tblecpyIndx);
#ifdef __cplusplus
}
#endif

#endif
