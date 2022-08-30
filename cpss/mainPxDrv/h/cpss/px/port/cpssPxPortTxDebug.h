/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file cpssPxPortTxDebug.h
*
* @brief CPSS implementation for configuring the Physical Port Tx Traffic Class
* Queues.
* This covers:
* - queuing enable/disable
* - transmit enable/disable
* - limits of total descriptors in egress queues
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxPortTxDebug_h__
#define __cpssPxPortTxDebug_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/px/cos/private/prvCpssPxCoS.h>


/*
    Values for the tail drop dump bitmap
*/
#define CPSS_PX_PORT_TX_DEBUG_DUMP_TAIL_DROP_GLOBAL_CONFIG              0x01
#define CPSS_PX_PORT_TX_DEBUG_DUMP_TAIL_DROP_PROFILES                   0x02
#define CPSS_PX_PORT_TX_DEBUG_DUMP_TAIL_DROP_PORT_TO_TD_PROFILE         0x04
#define CPSS_PX_PORT_TX_DEBUG_DUMP_TAIL_DROP_SCHEDULER_CONFIG           0x08
#define CPSS_PX_PORT_TX_DEBUG_DUMP_TAIL_DROP_WRTD_INFO                  0x10
#define CPSS_PX_PORT_TX_DEBUG_DUMP_TAIL_DROP_RESOURCE_ALLOCATION        0x20
#define CPSS_PX_PORT_TX_DEBUG_DUMP_TAIL_DROP_SHARE_POOL                 0x40
#define CPSS_PX_PORT_TX_DEBUG_DUMP_TX_Q_STATUS                          0x80
#define CPSS_PX_PORT_TX_DEBUG_DUMP_ALL                                  0xFF


/**
* @internal cpssPxPortTxDebugQueueingEnableSet function
* @endinternal
*
* @brief   Enable/Disable enqueuing to a Traffic Class queue on the specified port
*         of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this device.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] enable                   - GT_TRUE,  enqueuing to the queue
*                                      GT_FALSE, disable enqueuing to the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugQueueingEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tcQueue,
    IN  GT_BOOL                 enable
);

/**
* @internal cpssPxPortTxDebugQueueingEnableGet function
* @endinternal
*
* @brief   Get the status of enqueuing to a Traffic Class queue on the specified
*         port of specified device (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this device.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] enablePtr                - GT_TRUE, enqueuing to the queue enabled
*                                      GT_FALSE, enqueuing to the queue disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugQueueingEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tcQueue,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssPxPortTxDebugQueueTxEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmission from a Traffic Class queue on the specified
*         port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this Physical Port.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] enable                   - GT_TRUE,  transmission from the queue
*                                      GT_FALSE, disable transmission from the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugQueueTxEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tcQueue,
    IN  GT_BOOL                 enable
);

/**
* @internal cpssPxPortTxDebugQueueTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission from a Traffic Class queue on the
*         specified port of specified device (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this Physical Port.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] enablePtr                - GT_TRUE, transmission from the queue enabled
*                                      GT_FALSE, transmission from the queue disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugQueueTxEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tcQueue,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssPxPortTxDebugResourceHistogramThresholdSet function
* @endinternal
*
* @brief   Sets threshold for Histogram counter increment.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrNum                  - Histogram Counter number.
*                                      (APPLICABLE RANGES: 0..3).
* @param[in] threshold                - If the Global Descriptors Counter exceeds this Threshold,
*                                      the Histogram Counter is incremented by 1.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugResourceHistogramThresholdSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          cntrNum,
    IN  GT_U32          threshold
);

/**
* @internal cpssPxPortTxDebugResourceHistogramThresholdGet function
* @endinternal
*
* @brief   Gets threshold for Histogram counter increment.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrNum                  - Histogram Counter number.
*                                      (APPLICABLE RANGES: 0..3).
*
* @param[out] thresholdPtr             - (pointer to) if the Global Descriptors Counter
*                                      exceeds this Threshold, the Histogram Counter
*                                      is incremented by 1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugResourceHistogramThresholdGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          cntrNum,
    OUT GT_U32          *thresholdPtr
);

/**
* @internal cpssPxPortTxDebugResourceHistogramCounterGet function
* @endinternal
*
* @brief   Gets Histogram Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrNum                  - Histogram Counter number.
*                                      (APPLICABLE RANGES: 0..3).
*
* @param[out] cntrPtr                  - (pointer to) Histogram Counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Histogram Counter is cleared on read.
*
*/
GT_STATUS cpssPxPortTxDebugResourceHistogramCounterGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          cntrNum,
    OUT GT_U32          *cntrPtr
);

/**
* @internal cpssPxPortTxDebugGlobalDescLimitSet function
* @endinternal
*
* @brief   Set limits of total descriptors in all egress queues
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] limit                    - total descriptor  value.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on limit out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Improper configuration of descriptors limit may affect entire system
*       behavior.
*
*/
GT_STATUS cpssPxPortTxDebugGlobalDescLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          limit
);

/**
* @internal cpssPxPortTxDebugGlobalDescLimitGet function
* @endinternal
*
* @brief   Get limits of total descriptors in all egress queues
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] limitPtr                 - pointer to total descriptor limit value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugGlobalDescLimitGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *limitPtr
);

/**
* @internal cpssPxPortTxDebugGlobalQueueTxEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmission of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE,  transmission
*                                      GT_FALSE, disable transmission
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugGlobalQueueTxEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
);

/**
* @internal cpssPxPortTxDebugGlobalQueueTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission of specified device (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) transmission state:
*                                      GT_TRUE, transmission enabled
*                                      GT_FALSE, transmission disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugGlobalQueueTxEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssPxPortTxDebugDescNumberGet function
* @endinternal
*
* @brief   Gets the current number of descriptors allocated per specified port.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugDescNumberGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U16                  *numberPtr
);

/**
* @internal cpssPxPortTxDebugQueueDumpAll function
* @endinternal
*
* @brief   Dumps all related Tx Queue information.
*         That includes the static configuration and resources allocated at run-time
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - SW device number
* @param[in] tailDropDumpBmp          - bitmap to indicate what info to show (0xff for all)
*
* @retval GT_OK                    - on success, match found for devNum.
* @retval GT_BAD_PARAM             - on tailDropDumpBmp not in range.
*/
GT_STATUS cpssPxPortTxDebugQueueDumpAll
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          tailDropDumpBmp
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxPortTxDebug_h__ */

