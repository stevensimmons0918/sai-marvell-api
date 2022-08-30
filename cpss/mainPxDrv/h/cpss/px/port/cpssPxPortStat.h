/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssPxPortStat.h
*
* @brief CPSS implementation for port MAC MIB counters.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxPortStat_h
#define __cpssPxPortStat_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/cos/cpssCosTypes.h>
#include <cpss/common/port/cpssPortStat.h>

/**
* @enum CPSS_PX_PORT_MAC_COUNTER_ENT
 *
 * @brief Enumeration of the port MAC MIB Counters.
*/
typedef enum{

    /** @brief The sum of lengths of all good Ethernet frames received, namely,
     *  frames that are not bad frames or MAC Control packets. This sum does
     *  not include 802.3x pause messages, but does include bridge control packets
     *  like LCAP and BPDU.
     */
    CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E

    /** The sum of lengths of all bad Ethernet frames received. */
    ,CPSS_PX_PORT_MAC_COUNTER_BAD_OCTETS_RECEIVED_E

    /** @brief Invalid frame transmitted when one of the following occurs:
     *  - A frame with a bad CRC was read from the memory.
     *  - Underrun occurs.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_CRC_ERRORS_SENT_E

    /** @brief Number of Ethernet Unicast frames received that are not bad
     *  Ethernet frames or MAC Control packets.
     *  Note that this number includes Bridge Control packets such as
     *  LACP and BPDU.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_GOOD_UNICAST_FRAMES_RECEIVED_E

    /** @brief Number of good frames received that had a Broadcast
     *  destination MAC Address.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_RECEIVED_E

    /** @brief The number of good frames received that had a Multicast
     *  destination MAC Address.
     *  NOTE: This does NOT include 802.3 Flow Control messages,
     *  as they are considered MAC Control packets.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_RECEIVED_E

    /** @brief The number of received and transmitted good and bad frames
     *  that are 64 bytes in size.
     *  NOTE: This does not include MAC Control Frames.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_FRAMES_64_OCTETS_E

    /** @brief The number of received and transmitted good and bad frames
     *  that are 65 to 127 bytes in size.
     *  NOTE: This does not include MAC Control Frames.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_FRAMES_65_TO_127_OCTETS_E

    /** @brief The number of received and transmitted good and bad frames
     *  that are 128 to 255 bytes in size.
     *  NOTE: This does not include MAC Control Frames.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_FRAMES_128_TO_255_OCTETS_E

    /** @brief The number of received and transmitted good and bad frames
     *  that are 256 to 511 bytes in size.
     *  NOTE: This does not include MAC Control Frames.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_FRAMES_256_TO_511_OCTETS_E

    /** @brief The number of received and transmitted good and bad frames
     *  that are 512 to 1023 bytes in size.
     *  NOTE: This does not include MAC Control Frames.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_FRAMES_512_TO_1023_OCTETS_E

    /** @brief The number of received and transmitted good and bad frames
     *  depend on mode set by : cpssPxPortMacOversizedPacketsCounterModeSet().
     *  1. that are 1024 to max
     *  or
     *  2. that are 1519 to max bytes in size.
     *  or
     *  3. that are 1523 to max bytes in size.
     *  NOTE: This does not include MAC Control Frames.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E

    /** @brief The number of received and transmitted of good and bad frames
     *  depend on mode set by : cpssPxPortMacOversizedPacketsCounterModeSet().
     *  1. deferred transmitted frames - relevant to GIG MAC in half duplex mode.
     *  or
     *  2. that are 1024 to 1518 bytes in size.
     *  NOTE: This does not include MAC Control Frames.
     *  or
     *  3. that are 1024 to 1522 bytes in size.
     *  NOTE: This does not include MAC Control Frames.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E

    /** @brief The sum of lengths of all good Ethernet frames sent from this
     *  MAC. This does not include 802.3 Flow Control frames or
     *  packets with Transmit Error Event counted in CRCErrorSent.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E

    /** @brief Number of good frames sent that had a Unicast destination MAC
     *  Address.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E

    /** @brief Number of good frames sent that had a Multicast destination
     *  MAC Address. This does not include 802.3 Flow Control frames.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_SENT_E

    /** @brief Number of good frames sent that had a Broadcast destination
     *  MAC Address. This does not include 802.3 Flow Control frames.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_SENT_E

    /** Number of 802.x3 Flow Control frames sent. */
    ,CPSS_PX_PORT_MAC_COUNTER_FC_SENT_E

    /** Number of 802.3x Flow Control frames received. */
    ,CPSS_PX_PORT_MAC_COUNTER_FC_RECEIVED_E

    /** @brief Number of instances that the port was unable to receive packets
     *  due to insufficient bandwidth to one of the packet processor
     *  internal resources.
     *  This counter counts overruns on the Rx FIFO.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_RECEIVED_FIFO_OVERRUN_E

    /** Number of undersized packets received. */
    ,CPSS_PX_PORT_MAC_COUNTER_UNDERSIZE_E

    /** Number of fragments received. */
    ,CPSS_PX_PORT_MAC_COUNTER_FRAGMENTS_E

    /** Number of oversize packets received. */
    ,CPSS_PX_PORT_MAC_COUNTER_OVERSIZE_E

    /** Number of jabber packets received. */
    ,CPSS_PX_PORT_MAC_COUNTER_JABBER_E

    /** Number of Rx Error events seen by the receive side of the MAC. */
    ,CPSS_PX_PORT_MAC_COUNTER_RX_ERROR_FRAME_RECEIVED_E

    /** Number of CRC error events. */
    ,CPSS_PX_PORT_MAC_COUNTER_BAD_CRC_E

    /** @brief If working in Full Duplex, this is the PFC counter.
     *  If working in Half Duplex, this is the transmit with collision
     *  counter.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_COLLISION_E

    /** @brief Late collision If working in Full Duplex, this is the Unknown FC counter.
     *  If working in Half Duplex, this is the transmit with late collision
     *  counter.
     */
    ,CPSS_PX_PORT_MAC_COUNTER_LATE_COLLISION_E

    /** must be last (not to be used as counter) */
    ,CPSS_PX_PORT_MAC_COUNTER____LAST____E

} CPSS_PX_PORT_MAC_COUNTER_ENT;


/**
* @struct CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC
 *
 * @brief Structure of TXQ drop counters.
*/
typedef struct{

    GT_U32 egrMirrorDropCntr;

    /** egress STC dropped counter value */
    GT_U32 egrStcDropCntr;

    /** egress QCN dropped counter value */
    GT_U32 egrQcnDropCntr;

    /** dropped PFC triggered events counter value */
    GT_U32 dropPfcEventsCntr;

    /** @brief total number of discarded descriptors
     *  (clear events to the Buffer Manager)
     */
    GT_U32 clearPacketsDroppedCounter;

} CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC;


/*
 * typedef: struct CPSS_PX_PORT_MAC_COUNTERS_STC
 *
 * Description: array of ALL counters of 'port MAC MIB Counters'
 *
 *  mibCounter[] - array of ALL counters of 'port MAC MIB Counters'
 *                 index into the array is the 'Specific counter type'
 *                  according to enum : CPSS_PX_PORT_MAC_COUNTER_ENT
 */
typedef struct
{
    GT_U64 mibCounter[CPSS_PX_PORT_MAC_COUNTER____LAST____E];
} CPSS_PX_PORT_MAC_COUNTERS_STC;

/**
* @enum CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT
 *
 * @brief Enumeration of oversized packets counter mode.
 * that relate to counters of:
 * CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E
 * CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E
*/
typedef enum{

    /** @brief counter CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E
     *  counts total frames sent/received which have a byte count of 1024 or more.
     *  counter CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E
     *  counts number of sent deferred frames - relevant to GIG MAC in half duplex mode.
     */
    CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E,

    /** @brief counter CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E
     *  counts total frames sent/received which have a byte count of 1519 or more.
     *  counter CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E
     *  counts total frames sent/received which have a byte count of 1024 to 1518.
     */
    CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E,

    /** @brief counter CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E
     *  counts total frames sent/received which have a byte count of 1523 or more.
     *  counter CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E
     *  counts total frames sent/received which have a byte count of 1024 to 1522.
     */
    CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E

} CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT;

/**
* @struct CPSS_PX_PORT_EGRESS_CNTR_STC
 *
 * @brief Structure of egress counters for Pipe.
*/
typedef struct{

    /** Number of transmitted packets. */
    GT_U32 outFrames;

    /** @brief Number of packets that were filtered due to TxQ
     *  congestion.
     */
    GT_U32 txqFilterDisc;

} CPSS_PX_PORT_EGRESS_CNTR_STC;


/**
* @internal cpssPxPortMacCounterGet function
* @endinternal
*
* @brief   Gets specific MAC MIB counter of a specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] cntrName                 - specific counter name
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacCounterGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_PORT_MAC_COUNTER_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
);

/**
* @internal cpssPxPortMacCountersOnPortGet function
* @endinternal
*
* @brief   Gets ALL MAC MIB counters of a specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] countersPtr              - (pointer to) counters values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacCountersOnPortGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PX_PORT_MAC_COUNTERS_STC   *countersPtr
);

/**
* @internal cpssPxPortMacOversizedPacketsCounterModeSet function
* @endinternal
*
* @brief   Sets MIB counters behavior for oversized packets on given port,
*         that relate to counters:
*         CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E
*         CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  NONE.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] counterMode              - oversized packets counter mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacOversizedPacketsCounterModeSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  counterMode
);

/**
* @internal cpssPxPortMacOversizedPacketsCounterModeGet function
* @endinternal
*
* @brief   Gets MIB counters behavior for oversized packets on given port,
*         that relate to counters:
*         CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E
*         CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] counterModePtr           - (pointer to) oversized packets counter mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacOversizedPacketsCounterModeGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  *counterModePtr
);

/**
* @internal cpssPxPortMacCountersClearOnReadSet function
* @endinternal
*
* @brief   Enable or disable MAC Counters 'Clear on read' on the port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  clear on read for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacCountersClearOnReadSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable
);

/**
* @internal cpssPxPortMacCountersClearOnReadGet function
* @endinternal
*
* @brief   Get 'Clear on read' status of MAC Counters on the port
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - (pointer to) 'Clear on read' status for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacCountersClearOnReadGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssPxPortMacCountersEnableSet function
* @endinternal
*
* @brief   Enable or disable MAC Counters update for this port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   -  update of MAC counters
*                                      GT_FALSE = MAC counters update for this port is disabled.
*                                      GT_TRUE = MAC counters update for this port is enabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical
*                                       port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacCountersEnableSet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  enable
);

/**
* @internal cpssPxPortMacCountersEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disable) of MAC Counters update
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - state of the MAC counters
*                                      GT_FALSE = MAC counters update for this port is disabled.
*                                      GT_TRUE  = MAC counters update for this port is enabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical
*                                       port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
GT_STATUS cpssPxPortMacCountersEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssPxPortMacCounterCaptureGet function
* @endinternal
*
* @brief   Gets the captured Ethernet MAC counter for a specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] cntrName                 - specific counter name
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacCounterCaptureGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PX_PORT_MAC_COUNTER_ENT    cntrName,
    OUT GT_U64                          *cntrValuePtr
);

/**
* @internal cpssPxPortMacCountersCaptureOnPortGet function
* @endinternal
*
* @brief   Gets captured Ethernet MAC counter for a particular Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacCountersCaptureOnPortGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PX_PORT_MAC_COUNTERS_STC   *portMacCounterSetArrayPtr
);

/**
* @internal cpssPxPortMacCountersCaptureTriggerSet function
* @endinternal
*
* @brief   The function triggers a capture of MIB counters for specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number, whose counters are to be captured.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - previous capture operation isn't finished.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To get the counters, see:
*       cpssPxPortMacCountersCaptureOnPortGet,
*       cpssPxPortMacCounterCaptureGet.
*
*/
GT_STATUS cpssPxPortMacCountersCaptureTriggerSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);

/**
* @internal cpssPxPortMacCountersCaptureTriggerGet function
* @endinternal
*
* @brief   The function gets status of a capture of MIB counters for specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] captureIsDonePtr         - pointer to status of Capture counter Trigger
*                                      GT_TRUE  - capture is done.
*                                      GT_FALSE - capture action is in proccess.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To get the counters, see:
*       cpssPxPortMacCountersCaptureOnPortGet,
*       cpssPxPortMacCounterCaptureGet.
*
*/
GT_STATUS cpssPxPortMacCountersCaptureTriggerGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *captureIsDonePtr
);

/**
* @internal cpssPxPortMacCountersOnCgPortGet function
* @endinternal
*
* @brief   Gets the CG mac mib counters as is without any conversion.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] cgMibStcPtr              - (pointer to) CG mac mib structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*/
GT_STATUS cpssPxPortMacCountersOnCgPortGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_PORT_MAC_CG_COUNTER_SET_STC    *cgMibStcPtr
);

/**
* @internal cpssPxPortStatTxDebugCountersGet function
* @endinternal
*
* @brief   Gets egress TxQ drop counters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] dropCntrStcPtr           - (pointer to) drop counter structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortStatTxDebugCountersGet
(
    IN  GT_SW_DEV_NUM                            devNum,
    OUT CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC  *dropCntrStcPtr
);

/**
* @internal cpssPxPortEgressCntrModeSet function
* @endinternal
*
* @brief   Configure a set of egress counters that work in specified egress
*         counters mode on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrSetNum               - counter set number.
*                                      (APPLICABLE RANGE: 0..1).
* @param[in] setModeBmp               - counter mode bitmap. Supported counter mode:
*                                      CPSS_EGRESS_CNT_TC_E,
*                                      CPSS_EGRESS_CNT_DP_E,
*                                      CPSS_EGRESS_CNT_PORT_E.
*                                      Example:
*                                      CPSS_EGRESS_CNT_TC_E | CPSS_EGRESS_CNT_DP_E.
* @param[in] portNum                  - physical port number, if corresponding bit in setModeBmp is 1.
* @param[in] tc                       - traffic class queue, if corresponding bit in setModeBmp is 1.
*                                      (APPLICABLE RANGE: 0..7).
* @param[in] dpLevel                  - drop precedence level, if corresponding bit in setModeBmp is 1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported DP levels:
*       CPSS_DP_GREEN_E,
*       CPSS_DP_RED_E.
*
*/
GT_STATUS cpssPxPortEgressCntrModeSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          cntrSetNum,
    IN  CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          tc,
    IN  CPSS_DP_LEVEL_ENT               dpLevel
);

/**
* @internal cpssPxPortEgressCntrModeGet function
* @endinternal
*
* @brief   Get configuration of egress counters set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrSetNum               - counter set number.
*                                      (APPLICABLE RANGE: 0..1).
*
* @param[out] setModeBmpPtr            - Pointer to counter mode bitmap. Supported counter mode:
*                                      CPSS_EGRESS_CNT_TC_E,
*                                      CPSS_EGRESS_CNT_DP_E,
*                                      CPSS_EGRESS_CNT_PORT_E.
* @param[out] portNumPtr               - Pointer to the egress port of the packets counted by
*                                      the set of counters
* @param[out] tcPtr                    - Pointer to the tc of the packets counted by the set
*                                      of counters.
* @param[out] dpLevelPtr               - Pointer to the dp of the packets counted by the set
*                                      of counters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortEgressCntrModeGet
(
    IN   GT_SW_DEV_NUM                   devNum,
    IN   GT_U32                          cntrSetNum,
    OUT  CPSS_PORT_EGRESS_CNT_MODE_ENT   *setModeBmpPtr,
    OUT  GT_PHYSICAL_PORT_NUM            *portNumPtr,
    OUT  GT_U32                          *tcPtr,
    OUT  CPSS_DP_LEVEL_ENT               *dpLevelPtr
);

/**
* @internal cpssPxPortEgressCntrsGet function
* @endinternal
*
* @brief   Gets a egress counters from specific counter-set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrSetNum               - counter set number.
*                                      (APPLICABLE RANGE: 0..1).
*
* @param[out] egrCntrPtr               - (pointer to) structure of egress counters current values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortEgressCntrsGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          cntrSetNum,
    OUT CPSS_PX_PORT_EGRESS_CNTR_STC    *egrCntrPtr
);

/**
* @internal cpssPxPortMacCountersRxHistogramEnableSet function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram counters for
*         received packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  updating of the counters for received packets
*                                      GT_TRUE  - counters are updated.
*                                      GT_FALSE - counters are not updated.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device or physical port number.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssPxPortMacCountersRxHistogramEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/**
* @internal cpssPxPortMacCountersTxHistogramEnableSet function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram counters for
*         transmitted packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  updating of the counters for transmitted packets
*                                      GT_TRUE  - counters are updated.
*                                      GT_FALSE - counters are not updated.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacCountersTxHistogramEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/**
* @internal cpssPxPortMacCountersRxHistogramEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disabled) of the RMON Etherstat histogram counters
*         for received packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - state of the counters for received packets
*                                      GT_TRUE  - counters are enabled.
*                                      GT_FALSE - counters are disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
GT_STATUS cpssPxPortMacCountersRxHistogramEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssPxPortMacCountersTxHistogramEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disabled) of the RMON Etherstat histogram counters
*         for transmitted packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - state of the counters for transmitted packets
*                                      GT_TRUE  - counters are enabled.
*                                      GT_FALSE - counters are disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
GT_STATUS cpssPxPortMacCountersTxHistogramEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxPortStat_h */

