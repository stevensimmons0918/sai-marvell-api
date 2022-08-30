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
* @file cpssPxPortPfc.h
*
* @brief CPSS implementation for Priority Flow Control functionality.
*
* @version   23
********************************************************************************
*/

#ifndef __cpssPxPortPfch
#define __cpssPxPortPfch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/px/cpssPxTypes.h>
#include <cpss/px/port/cpssPxPortTxTailDrop.h>
#include <cpss/px/port/cpssPxPortTxScheduler.h>

/**
* @enum CPSS_PX_PORT_PFC_COUNT_MODE_ENT
 *
 * @brief Enumeration of PFC Counting modes.
*/
typedef enum{

    /** Counting buffers. PFC thresholds are set in buffers. */
    CPSS_PX_PORT_PFC_COUNT_BUFFERS_MODE_E,

    /** Counting packets. PFC thresholds are set in packets. */
    CPSS_PX_PORT_PFC_COUNT_PACKETS_E

} CPSS_PX_PORT_PFC_COUNT_MODE_ENT;

/**
* @enum CPSS_PX_PORT_PFC_ENABLE_ENT
 *
 * @brief Enumeration of PFC enable options.
*/
typedef enum{

    /** PFC triggering only enabled. */
    CPSS_PX_PORT_PFC_ENABLE_TRIGGERING_ONLY_E,

    /** Both PFC triggering and response are enabled. */
    CPSS_PX_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E

} CPSS_PX_PORT_PFC_ENABLE_ENT;

/**
* @struct CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC
 *
 * @brief PFC Profile configurations.
*/
typedef struct{

    /** Xon threshold. */
    GT_U32 xonThreshold;

    /** Xoff threshold. */
    GT_U32 xoffThreshold;

    /** defines which part of the free buffers are available to the port or queue for XON */
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT xonAlpha;

    /** @brief defines which part of the free buffers are available to the port or queue for XOFF.
     *  The XON/XOFF limits are calculated as follows:
     *  Limit = Guaranteed Buffers + Dynamic buffers
     *  The Dynamic Factor is calculated based on the amount of free buffers.
     *  Free Buffers = Total Available Buffers - Global Used Buffers
     *  Dynamic buffer for the queue is defined by following:
     *  queueNBuffs =xxxAlpha (Free Buffers/(sum of all alphas +1)))
     *  Comments:
     *  None.
     */
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT xoffAlpha;

} CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC;


/**
* @internal cpssPxPortPfcEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) response functionality.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] pfcEnable                - PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC enable option
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If PFC response is enabled, the shaper’s baseline must be
*       at least 0x3FFFC0, see:
*       cpssPxPortTxShaperBaselineSet.
*       Note: Triggering cannot be disabled by this API.
*
*/
GT_STATUS cpssPxPortPfcEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN CPSS_PX_PORT_PFC_ENABLE_ENT pfcEnable
);

/**
* @internal cpssPxPortPfcEnableGet function
* @endinternal
*
* @brief   Get the status of PFC response functionality.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*
* @param[out] pfcEnablePtr             - (pointer to) PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering is enabled by default.
*
*/
GT_STATUS cpssPxPortPfcEnableGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT CPSS_PX_PORT_PFC_ENABLE_ENT *pfcEnablePtr
);

/**
* @internal cpssPxPortPfcProfileIndexSet function
* @endinternal
*
* @brief   Binds a source port to a PFC profile.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range profile index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcProfileIndexSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32   profileIndex
);

/**
* @internal cpssPxPortPfcProfileIndexGet function
* @endinternal
*
* @brief   Gets the port's PFC profile.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] profileIndexPtr          - (pointer to) profile index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcProfileIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *profileIndexPtr
);

/**
* @internal cpssPxPortPfcDbaAvailableBuffersSet function
* @endinternal
*
* @brief   Define the number of buffers that are Available for the PFC Dynamic threshold calculations.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*                                      buffNum      - number of buffers available for DBA
*                                      (APPLICABLE RANGE: 0..0xFFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range buffers number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcDbaAvailableBuffersSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_U32         buffsNum
);

/**
* @internal cpssPxPortPfcDbaAvailableBuffersGet function
* @endinternal
*
* @brief   Get the number of buffers that are Available for the PFC Dynamic threshold calculations.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*                                      OUTPUTS:
* @param[in] buffsNumPtr              - pointer to number of buffers available for DBA.
*
* @param[out] buffsNumPtr              - pointer to number of buffers available for DBA.
*                                      OUTPUTS:
*                                      None
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range buffers number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcDbaAvailableBuffersGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT GT_U32         *buffsNumPtr
);


/**
* @internal cpssPxPortPfcProfileQueueConfigSet function
* @endinternal
*
* @brief   Sets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7)
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] pfcProfileCfgPtr         - pointer to PFC Profile configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssPxPortPfcCountingModeSet.
*
*/
GT_STATUS cpssPxPortPfcProfileQueueConfigSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_U32   profileIndex,
    IN GT_U32    tcQueue,
    IN CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
);

/**
* @internal cpssPxPortPfcProfileQueueConfigGet function
* @endinternal
*
* @brief   Gets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7)
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
*
* @param[out] pfcProfileCfgPtr         - pointer to PFC Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssPxPortPfcCountingModeSet.
*
*/
GT_STATUS cpssPxPortPfcProfileQueueConfigGet
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32   profileIndex,
    IN  GT_U32    tcQueue,
    OUT CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
);


/**
* @internal cpssPxPortPfcCountingModeSet function
* @endinternal
*
* @brief   Sets PFC counting mode.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] pfcCountMode             - PFC counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcCountingModeSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN CPSS_PX_PORT_PFC_COUNT_MODE_ENT  pfcCountMode
);

/**
* @internal cpssPxPortPfcCountingModeGet function
* @endinternal
*
* @brief   Gets PFC counting mode.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*
* @param[out] pfcCountModePtr          - (pointer to) PFC counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcCountingModeGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT CPSS_PX_PORT_PFC_COUNT_MODE_ENT  *pfcCountModePtr
);


/**
* @internal cpssPxPortPfcGlobalDropEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC global drop.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE: Enable PFC global drop.
*                                      GT_FALSE: Disable PFC global drop.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To configure drop threshold use cpssPxPortPfcGlobalQueueConfigSet.
*
*/
GT_STATUS cpssPxPortPfcGlobalDropEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssPxPortPfcGlobalDropEnableGet function
* @endinternal
*
* @brief   Gets the current status of PFC global drop.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of PFC functionality
*                                      GT_TRUE:  PFC global drop enabled.
*                                      GT_FALSE: PFC global drop disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcGlobalDropEnableGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssPxPortPfcGlobalQueueConfigSet function
* @endinternal
*
* @brief   Sets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] xoffThreshold            - XOFF threshold
*                                      (APPLICABLE RANGES: 0..0x1FFFFF)
* @param[in] dropThreshold            - Drop threshold. When a global counter with given tcQueue
*                                      (APPLICABLE RANGES: 0..0x1FFFFF)
* @param[in] xonThreshold             - XON threshold
*                                      (APPLICABLE RANGES: 0..0x1FFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. dropThreshold is used when PFC global drop is enabled.
*       See cpssPxPortPfcGlobalDropEnableSet.
*       2. All thresholds are set in buffers or packets.
*       See cpssPxPortPfcCountingModeSet.
*
*/
GT_STATUS cpssPxPortPfcGlobalQueueConfigSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_U32    tcQueue,
    IN GT_U32   xoffThreshold,
    IN GT_U32   dropThreshold,
    IN GT_U32   xonThreshold
);

/**
* @internal cpssPxPortPfcGlobalQueueConfigGet function
* @endinternal
*
* @brief   Gets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
*
* @param[out] xoffThresholdPtr         - (pointer to) Xoff threshold.
* @param[out] dropThresholdPtr         - (pointer to) Drop threshold. When a global counter with given tcQueue
*                                      crosses up the dropThreshold the packets are dropped.
* @param[out] xonThresholdPtr          - (pointer to) Xon threshold.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssPxPortPfcCountingModeSet.
*
*/
GT_STATUS cpssPxPortPfcGlobalQueueConfigGet
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32    tcQueue,
    OUT GT_U32   *xoffThresholdPtr,
    OUT GT_U32   *dropThresholdPtr,
    OUT GT_U32   *xonThresholdPtr
);

/**
* @internal cpssPxPortPfcTimerMapEnableSet function
* @endinternal
*
* @brief   Enables mapping of PFC timer to priority queue for given scheduler profile.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] enable                   - Determines whether PFC timer to Priority Queue map
*                                      is used.
*                                      GT_TRUE: PFC timer to Priority Queue map used.
*                                      GT_FALSE: PFC timer to Priority Queue map bypassed.
*                                      1:1 mapping between a timer in PFC frame to an egress queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To bind port to scheduler profile use:
*       cpssPxPortTxBindPortToSchedulerProfileSet.
*
*/
GT_STATUS cpssPxPortPfcTimerMapEnableSet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_BOOL                                 enable

);

/**
* @internal cpssPxPortPfcTimerMapEnableGet function
* @endinternal
*
* @brief   Get the status of PFS timer to priority queue mapping for given
*         scheduler profile.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile Set
*
* @param[out] enablePtr                - (pointer to) status of PFC timer to Priority Queue
*                                      mapping.
*                                      GT_TRUE: PFC timer to Priority Queue map used.
*                                      GT_FALSE: PFC timer to Priority Queue map bypassed.
*                                      1:1 mapping between a timer in PFC frame to an egress queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcTimerMapEnableGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    OUT GT_BOOL                                *enablePtr

);

/**
* @internal cpssPxPortPfcTimerToQueueMapSet function
* @endinternal
*
* @brief   Sets PFC timer to priority queue map.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] pfcTimer                 - PFC timer (0..7)
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC timer
* @retval GT_OUT_OF_RANGE          - on out of traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcTimerToQueueMapSet
(
    IN  GT_SW_DEV_NUM     devNum,
    IN  GT_U32    pfcTimer,
    IN  GT_U32    tcQueue

);

/**
* @internal cpssPxPortPfcTimerToQueueMapGet function
* @endinternal
*
* @brief   Gets PFC timer to priority queue map.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] pfcTimer                 - PFC timer (0..7).
*
* @param[out] tcQueuePtr               - (pointer to) traffic class queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC timer
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcTimerToQueueMapGet
(
    IN  GT_SW_DEV_NUM     devNum,
    IN  GT_U32    pfcTimer,
    OUT GT_U32    *tcQueuePtr

);

/**
* @internal cpssPxPortPfcShaperToPortRateRatioSet function
* @endinternal
*
* @brief   Sets shaper rate to port speed ratio on given scheduler profile
*         and traffic class queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] shaperToPortRateRatio    - shaper rate to port speed ratio
*                                      in percentage (0..100).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number ,profile set
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range shaper rate to port speed ratio
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To bind port to scheduler profile use:
*       cpssPxPortTxBindPortToSchedulerProfileSet.
*
*/
GT_STATUS cpssPxPortPfcShaperToPortRateRatioSet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT       profileSet,
    IN  GT_U32                                          tcQueue,
    IN  GT_U32                                          shaperToPortRateRatio

);

/**
* @internal cpssPxPortPfcShaperToPortRateRatioGet function
* @endinternal
*
* @brief   Gets shaper rate to port speed ratio on given scheduler profile
*         and traffic class queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] shaperToPortRateRatioPtr - (pointer to)shaper rate to port speed ratio
*                                      in percentage.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number ,profile set
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcShaperToPortRateRatioGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT       profileSet,
    IN  GT_U32                                          tcQueue,
    OUT GT_U32                                          *shaperToPortRateRatioPtr

);

/**
* @internal prvCpssPxPortPfcCascadeEnableSet function
* @endinternal
*
* @brief   Enable/Disable insertion of DSA tag for PFC frames
*         transmitted on given port.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
* @param[in] enable                   - GT_TRUE - PFC packets are transmitted with a DSA tag.
*                                      GT_FALSE - PFC packets are transmitted without DSA tag.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortPfcCascadeEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         enable
);

/**
* @internal cpssPxPortPfcForwardEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of PFC frames to the ingress
*         pipeline of a specified port.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward PFC frames to the ingress pipe,
*                                      GT_FALSE: do not forward PFC frames to the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered as a PFC frame if all of the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is 01-01
*       - Packet’s MAC DA is 01-80-C2-00-00-01 or the port’s configured MAC Address
*
*/
GT_STATUS cpssPxPortPfcForwardEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_BOOL enable
);


/**
* @internal cpssPxPortPfcForwardEnableGet function
* @endinternal
*
* @brief   Get status of PFC frames forwarding
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - current forward PFC frames status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered as a PFC frame if all of the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is 01-01
*       - Packet’s MAC DA is 01-80-C2-00-00-01 or the port’s configured MAC Address
*
*/
GT_STATUS cpssPxPortPfcForwardEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssPxPortPfcPacketClassificationEnableSet function
* @endinternal
*
* @brief   Enable/Disable the ingress control pipe to parse and terminate received
*         PFC frames
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.(APPLICABLE RANGES: 0..31)
* @param[in] udbpIndex                - user defined byte pair index, where opcode will be set (APPLICABLE RANGES: 0..3)
* @param[in] enable                   -  or disable PFC packet classification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note If the configuration is disabled, flow control frames are processed by
*       the control pipe as regular data frames (that is they are subject to
*       learning, forwarding, filtering and mirroring) but they are not
*       terminated and the PFC timers are not extracted.
*
*/
GT_STATUS cpssPxPortPfcPacketClassificationEnableSet
(

    IN GT_SW_DEV_NUM                devNum,
    IN CPSS_PX_PACKET_TYPE          packetType,
    IN GT_U32                       udbpIndex,
    IN GT_BOOL                      enable
);

/**
* @internal cpssPxPortPfcPacketTypeGet function
* @endinternal
*
* @brief   Get PFC packet type
*
* @param[in] devNum                   - device number.
*
* @param[out] packetTypePtr            - (pointer to) packet type index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssPxPortPfcPacketTypeGet
(
    IN  GT_SW_DEV_NUM            devNum,
    OUT CPSS_PX_PACKET_TYPE      *packetTypePtr
);

/**
* @internal cpssPxPortPfcQueueCounterGet function
* @endinternal
*
* @brief   Get the number buffers/packets per proirity queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue on the port
*
* @param[out] cntPtr                   - (pointer to) number buffers/packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, tcQueue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Buffer or packet counting mode is set by cpssPxPortPfcCountingModeSet.
*
*/
GT_STATUS cpssPxPortPfcQueueCounterGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32   tcQueue,
    OUT GT_U32  *cntPtr
);


/**
* @internal cpssPxPortPfcSourcePortToPfcCounterSet function
* @endinternal
*
* @brief   Set PFC mapping of a physical port to a PFC counter.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] pfcCounterNum            - PFC counter (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcSourcePortToPfcCounterSet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32  pfcCounterNum
);


/**
* @internal cpssPxPortPfcSourcePortToPfcCounterGet function
* @endinternal
*
* @brief   Get PFC mapping of a physical port to a PFC counter.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] pfcCounterNumPtr         (pointer to) PFC counter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssPxPortPfcSourcePortToPfcCounterGet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *pfcCounterNumPtr
);


/**
* @internal cpssPxPortPfcCounterGet function
* @endinternal
*
* @brief   Get PFC counter value per PFC counter and traffic class.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] pfcCounterNum            - PFC counter (APPLICABLE RANGES: 0..127)
*
* @param[out] pfcCounterValuePtr       - (pointer to) PFC counter value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssPxPortPfcCounterGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32   tcQueue,
    IN  GT_U32  pfcCounterNum,
    OUT GT_U32  *pfcCounterValuePtr
);

/**
* @internal cpssPxPortPfcXonMessageFilterEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) filtering of
*         continuous flow control XON messages to priority shaper.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE:  Enable  PFC XON Message Filter.
*                                      GT_FALSE: Disable PFC XON Message Filter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or enable option
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Enable this feature in a system with periodic flow control XON messages.
*
*/
GT_STATUS cpssPxPortPfcXonMessageFilterEnableSet
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssPxPortPfcXonMessageFilterEnableGet function
* @endinternal
*
* @brief   Gets current status of the filtering of
*         continuous flow control XON messages to priority shaper.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) Enable  PFC XON Message Filter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcXonMessageFilterEnableGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_BOOL *enablePtr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxPortPfch */




