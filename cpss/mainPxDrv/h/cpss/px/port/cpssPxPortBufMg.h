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
* @file cpssPxPortBufMg.h
*
* @brief CPSS PX Port Buffers management API.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxPortBufMgh
#define __cpssPxPortBufMgh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/cpssPortCtrl.h>

/**
* @internal cpssPxPortBufMgGlobalXonLimitSet function
* @endinternal
*
* @brief   Sets the Global Xon limit value. Enables global control over the
*         number of buffers allocated for all ports by Global Xon limit setting.
*         When the total number of buffers allocated reaches this threshold
*         (xonLimit), all ports that support 802.3x Flow Control send an Xon
*         frame.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] xonLimit                 - X-ON limit in resolution of 1 buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on wrong xonLimit.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Global Xon limit must be not be greater than Global Xoff Limit.
*
*/
GT_STATUS cpssPxPortBufMgGlobalXonLimitSet
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           xonLimit
);

/**
* @internal cpssPxPortBufMgGlobalXonLimitGet function
* @endinternal
*
* @brief   Gets the Global Xon limit value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] xonLimitPtr              - (pointer to) the X-ON limit value in resolution
*                                      of 1 buffer.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgGlobalXonLimitGet
(
    IN GT_SW_DEV_NUM    devNum,
    OUT GT_U32          *xonLimitPtr
);

/**
* @internal cpssPxPortBufMgGlobalXoffLimitSet function
* @endinternal
*
* @brief   Sets the Global Xoff limit value. Enables global control over the
*         number of buffers allocated for all ports by Global Xoff limit setting.
*         When the total number of buffers allocated reaches this threshold
*         (xoffLimit), all ports that support 802.3x Flow Control send a pause
*         frame.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] xoffLimit                - X-OFF limit in resolution of 1 buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on wrong xoffLimit.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgGlobalXoffLimitSet
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           xoffLimit
);

/**
* @internal cpssPxPortBufMgGlobalXoffLimitGet function
* @endinternal
*
* @brief   Gets the Global Xoff limit value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] xoffLimitPtr             - (pointer to) the X-OFF limit value in resolution
*                                      of 1 buffer.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgGlobalXoffLimitGet
(
    IN GT_SW_DEV_NUM    devNum,
    OUT GT_U32          *xoffLimitPtr
);

/**
* @internal cpssPxPortBufMgRxProfileSet function
* @endinternal
*
* @brief   Bind a port to a flow control profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or profileSet
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgRxProfileSet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet
);

/**
* @internal cpssPxPortBufMgRxProfileGet function
* @endinternal
*
* @brief   Get the port's flow control profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] profileSetPtr            - (pointer to) the Profile Set in which the Flow Control
*                                      Parameters are associated.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgRxProfileGet
(
    IN  GT_SW_DEV_NUM                    devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    OUT CPSS_PORT_RX_FC_PROFILE_SET_ENT  *profileSetPtr
);

/**
* @internal cpssPxPortBufMgProfileXonLimitSet function
* @endinternal
*
* @brief   Sets X-ON port limit. When Flow Control is enabled, X-ON packet sent
*         when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] xonLimit                 - X-ON limit in resolution of 1 buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval OUT_OF_RANGE             - on wrong xonLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgProfileXonLimitSet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           xonLimit
);

/**
* @internal cpssPxPortBufMgProfileXonLimitGet function
* @endinternal
*
* @brief   Gets the X-ON port limit. When Flow Control is enabled, X-ON packet
*         sent when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters of
*                                      are associated.
*
* @param[out] xonLimitPtr              - (pointer to) the X-ON limit value in resolution
*                                      of 1 buffer.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgProfileXonLimitGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *xonLimitPtr
);

/**
* @internal cpssPxPortBufMgProfileXoffLimitSet function
* @endinternal
*
* @brief   Sets X-OFF port limit. When Flow Control is enabled, X-OFF packet sent
*         when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] xoffLimit                - X-OFF limit in resolution of 1 buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval OUT_OF_RANGE             - on wrong xoffLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgProfileXoffLimitSet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           xoffLimit
);

/**
* @internal cpssPxPortBufMgProfileXoffLimitGet function
* @endinternal
*
* @brief   Gets the X-OFF port limit. When Flow Control is enabled, X-OFF packet
*         sent when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @param[out] xoffLimitPtr             - (pointer to) the X-OFF limit value in resolution
*                                      of 1 buffer.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgProfileXoffLimitGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *xoffLimitPtr
);

/**
* @internal cpssPxPortBufMgProfileRxBufLimitSet function
* @endinternal
*
* @brief   Sets receive buffer limit threshold for the specified profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] rxBufLimit               - buffer limit threshold in resolution of 1 buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval OUT_OF_RANGE             - on wrong rxBufLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgProfileRxBufLimitSet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           rxBufLimit
);

/**
* @internal cpssPxPortBufMgProfileRxBufLimitGet function
* @endinternal
*
* @brief   Gets the receive buffer limit threshold for the specified profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @param[out] rxBufLimitPtr            - (pointer to) the rx buffer limit value in resolution
*                                      of 1 buffer.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgProfileRxBufLimitGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *rxBufLimitPtr
);

/**
* @internal cpssPxPortBufMgGlobalRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numOfBuffersPtr          - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgGlobalRxBufNumberGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_U32           *numOfBuffersPtr
);

/**
* @internal cpssPxPortBufMgRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated for the specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] numOfBuffersPtr          - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgRxBufNumberGet
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32                 *numOfBuffersPtr
);

/**
* @internal cpssPxPortBufMgGlobalPacketNumberGet function
* @endinternal
*
* @brief   Gets total number of unique packets currently in the system.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numOfPacketsPtr          - (pointer to) number of packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgGlobalPacketNumberGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_U32           *numOfPacketsPtr
);

/**
* @internal cpssPxPortBufMgRxMcCntrGet function
* @endinternal
*
* @brief   Gets multicast counter of a certain buffer.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrIdx                  - counter index
*                                      (APPLICABLE RANGES 0..16383)
*
* @param[out] mcCntrPtr                - (pointer to) multicast counter of a certain buffer
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device or cntrIdx
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgRxMcCntrGet
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           cntrIdx,
    OUT GT_U32           *mcCntrPtr
);

/**
* @internal cpssPxPortBufMgTxDmaBurstLimitEnableSet function
* @endinternal
*
* @brief   For a given port Enable/Disable TXDMA burst limit thresholds use.
*         When enabled: instruct the TxDMA to throttle the Transmit Queue Scheduler
*         as a function of the TxDMA FIFOs fill level measured in bytes and descriptor.
*         When disabled: the TxDMA throttles the Transmit Queue Scheduler
*         based on FIFOs fill level measured in descriptors only.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
* @param[in] enable                   - GT_TRUE: TXDMA burst limit thresholds are used.
*                                      GT_FALSE: TXDMA burst limit thresholds are not used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgTxDmaBurstLimitEnableSet
(
    IN GT_SW_DEV_NUM          devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL                enable
);

/**
* @internal cpssPxPortBufMgTxDmaBurstLimitEnableGet function
* @endinternal
*
* @brief   Gets the current status for a given port of TXDMA burst limit thresholds use.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: TXDMA burst limit thresholds are used.
*                                      GT_FALSE: TXDMA burst limit thresholds are not used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgTxDmaBurstLimitEnableGet
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL                *enablePtr
);

/**
* @internal cpssPxPortBufMgTxDmaBurstLimitThresholdsSet function
* @endinternal
*
* @brief   Sets the TXDMA burst limit thresholds for a given port.
*         The TxDMA supports three throttling levels: Normal, Slow and OFF.
*         The levels are controlled using two thresholds (almostFullThreshold and
*         fullThreshold) measuring the FIFOs fill level.
*         Normal - the Transmit Queue scheduler is not throttled.
*         Slow - the Transmit Queue scheduler is throttled.
*         OFF - the Transmit Queue scheduler is paused.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
* @param[in] almostFullThreshold      - almost full threshold in Bytes (0..(0x80  0xFFFF)).
* @param[in] fullThreshold            - full threshold in Bytes (0..(0x80  0xFFFF)).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Resolution of almostFullThreshold and fullThreshold fields in hardware
*       is 128 Bytes.
*       The value is rounded down before write to HW.
*
*/
GT_STATUS cpssPxPortBufMgTxDmaBurstLimitThresholdsSet
(
    IN GT_SW_DEV_NUM          devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32                 almostFullThreshold,
    IN GT_U32                 fullThreshold
);

/**
* @internal cpssPxPortBufMgTxDmaBurstLimitThresholdsGet function
* @endinternal
*
* @brief   Gets the TXDMA burst limit thresholds for a given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
*
* @param[out] almostFullThresholdPtr   - (pointer to) almost full threshold in Bytes.
* @param[out] fullThresholdPtr         - (pointer to) full threshold in Bytes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgTxDmaBurstLimitThresholdsGet
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32                 *almostFullThresholdPtr,
    OUT GT_U32                 *fullThresholdPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxPortBufMgh */

