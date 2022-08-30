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
* @file cpssPxPortTxScheduler.h
*
* @brief CPSS implementation for configuring the Physical Port Tx Traffic Class
* Queues. This APIs cover scheduling algorithms and bandwidth management.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxPortTxScheduler_h__
#define __cpssPxPortTxScheduler_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>


/**
* @enum CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT
 *
 * @brief Enumeration of Tx Queue scheduler profile set
*/
typedef enum{

    /** Tx Queue scheduler profile 0 */
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E = 0,

    /** Tx Queue scheduler profile 1 */
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_1_E,

    /** Tx Queue scheduler profile 2 */
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_2_E,

    /** Tx Queue scheduler profile 3 */
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_3_E,

    /** Tx Queue scheduler profile 4 */
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_4_E,

    /** Tx Queue scheduler profile 5 */
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_5_E,

    /** Tx Queue scheduler profile 6 */
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_6_E,

    /** Tx Queue scheduler profile 7 */
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_7_E,

    /** Tx Queue scheduler profile 8 */
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_8_E,

    /** Tx Queue scheduler profile 9 */
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_9_E,

    /** Tx Queue scheduler profile 10 */
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_10_E,

    /** Tx Queue scheduler profile 11 */
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_11_E,

    /** Tx Queue scheduler profile 12 */
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_12_E,

    /** Tx Queue scheduler profile 13 */
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_13_E,

    /** Tx Queue scheduler profile 14 */
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_14_E,

    /** Tx Queue scheduler profile 15 */
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E,

} CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT;

/**
* @enum CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT
 *
 * @brief Enumeration of Transmit Scheduler mode
*/
typedef enum{

    /** packet mode */
    CPSS_PX_PORT_TX_SCHEDULER_WRR_PACKET_MODE_E,

    /** byte mode */
    CPSS_PX_PORT_TX_SCHEDULER_WRR_BYTE_MODE_E

} CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT;

/**
* @enum CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT
 *
 * @brief Enumeration of MTU for DSWRR, resolution of WRR weights for byte
 * based Transmit Scheduler count mode
*/
typedef enum{

    /** Resolution is 64 bytes */
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_64_E,

    /** Resolution is 128 bytes */
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_128_E,

    /** Resolution is 256 bytes */
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_256_E,

    /** Resolution is 512 bytes */
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_512_E,

    /** Resolution is 1K bytes */
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_1K_E,

    /** Resolution is 2K bytes */
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_2K_E,

    /** Resolution is 4K bytes */
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_4K_E,

    /** Resolution is 8K bytes */
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_8K_E,

    /** Resolution is 16K bytes */
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_16K_E,

    /** Resolution is 32K bytes */
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_32K_E,

    /** Resolution is 64K bytes */
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_64K_E

} CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT;

/**
* @enum CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT
 *
 * @brief The Traffic Class Queue Arbritration Group
*/
typedef enum{

    /** @brief Weighted Round Robin
     *  Arbitration Group 0
     */
    CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_0_E,

    /** @brief Weighted Round Robin
     *  Arbitration Group 1
     */
    CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_1_E,

    /** @brief Strict Priority
     *  Arbitration Group
     */
    CPSS_PX_PORT_TX_SCHEDULER_SP_ARB_GROUP_E

} CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT;

/**
* @enum CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT
 *
 * @brief Enumeration of BC (Byte Count) Change enable options.
*/
typedef enum{

    /** BC change is disabled. */
    CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_DISABLE_ALL_E,

    /** BC change is enabled for shaper only. */
    CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SHAPER_ONLY_E,

    /** BC change is enabled for scheduler only. */
    CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SCHEDULER_ONLY_E,

    /** BC change is enabled for both shaper and scheduler. */
    CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SHAPER_AND_SCHEDULER_E

} CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT;


/**
* @internal cpssPxPortTxSchedulerProfileIdSet function
* @endinternal
*
* @brief   Bind a port to scheduler profile set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] profileSet               - The Profile Set in which the scheduler's parameters are
*                                      associated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerProfileIdSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet
);

/**
* @internal cpssPxPortTxSchedulerProfileIdGet function
* @endinternal
*
* @brief   Get scheduler profile set that is binded to the port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] profileSetPtr            - The Profile Set in which the scheduler's parameters are
*                                      associated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerProfileIdGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   *profileSetPtr
);

/**
* @internal cpssPxPortTxSchedulerWrrMtuSet function
* @endinternal
*
* @brief   Set global parameters for WRR scheduler.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] wrrMtu                   - MTU for DSWRR, resolution of WRR weights for byte based
*                                      Transmit Scheduler count mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerWrrMtuSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT   wrrMtu
);

/**
* @internal cpssPxPortTxSchedulerWrrMtuGet function
* @endinternal
*
* @brief   Get global parameters for WRR scheduler.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] wrrMtuPtr                - (pointer to) MTU for DSWRR, resolution of WRR weights
*                                      for byte based Transmit Scheduler count mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssPxPortTxSchedulerWrrMtuGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    OUT CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT   *wrrMtuPtr
);

/**
* @internal cpssPxPortTxSchedulerWrrProfileSet function
* @endinternal
*
* @brief   Set Weighted Round Robin profile on the specified port's
*         Traffic Class Queue.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the wrrWeight
*                                      Parameter is associated.
* @param[in] tcQueue                  - traffic class queue on this Port.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] wrrWeight                - proportion of bandwidth assigned to this queue
*                                      relative to the other queues in this
*                                      Arbitration Group - resolution is 1/255.
*                                      (APPLICABLE RANGES: 0..0xFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If weight will be less than port's MTU (maximum transmit unit) there is
*       possibility for empty WRR loops for given TC queue, but to not tight
*       user and for future ASIC's where this problem will be solved check of
*       wrrWeight 256 > MTU not implemented.
*
*/
GT_STATUS cpssPxPortTxSchedulerWrrProfileSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    IN  GT_U32                                      wrrWeight
);

/**
* @internal cpssPxPortTxSchedulerWrrProfileGet function
* @endinternal
*
* @brief   Get Weighted Round Robin profile on the specified port's
*         Traffic Class Queue.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue on this Port.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which
*                                      the wrrWeight parameter is associated.
*
* @param[out] wrrWeightPtr             - Pointer to proportion of bandwidth assigned to
*                                      this queue relative to the other queues in this
*                                      Arbitration Group - resolution is 1/255.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerWrrProfileGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    OUT GT_U32                                      *wrrWeightPtr
);

/**
* @internal cpssPxPortTxSchedulerArbitrationGroupSet function
* @endinternal
*
* @brief   Set Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
* @param[in] tcQueue                  - traffic class queue.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] arbGroup                 - scheduling arbitration group.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong profile number, device or arbGroup
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerArbitrationGroupSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    IN  CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT     arbGroup
);

/**
* @internal cpssPxPortTxSchedulerArbitrationGroupGet function
* @endinternal
*
* @brief   Get Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
* @param[in] tcQueue                  - traffic class queue.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] arbGroupPtr              - Pointer to scheduling arbitration group.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong profile number, device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerArbitrationGroupGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    OUT CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT     *arbGroupPtr
);

/**
* @internal cpssPxPortTxSchedulerProfileByteCountChangeEnableSet function
* @endinternal
*
* @brief   Enables/Disables profile Byte Count Change of the packet length by per
*         port constant for shaping and/or scheduling rate calculation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - scheduler  index
* @param[in] bcMode                   - byte count change enable options
* @param[in] bcOp                     - addition or subtraction of the per port value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal profile, bcMode, bcOp
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerProfileByteCountChangeEnableSet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT       profile,
    IN  CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT  bcMode,
    IN  CPSS_ADJUST_OPERATION_ENT                       bcOp
);

/**
* @internal cpssPxPortTxSchedulerProfileByteCountChangeEnableGet function
* @endinternal
*
* @brief   Get profile Byte Count Change mode.
*         The byte count change mode changes the length of a packet by per port
*         constant for shaping and/or scheduling rate calculation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - scheduler  index
*
* @param[out] bcModePtr                - (pointer to) byte count change enable options.
* @param[out] bcOpPtr                  - (pointer to) addition or subtraction of the per port value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal profile
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerProfileByteCountChangeEnableGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT       profile,
    OUT CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT  *bcModePtr,
    OUT CPSS_ADJUST_OPERATION_ENT                       *bcOpPtr
);

/**
* @internal cpssPxPortTxSchedulerShaperByteCountChangeValueSet function
* @endinternal
*
* @brief   Sets the number of bytes added/subtracted to the length of every frame
*         for shaping/scheduling rate calculation for given port when
*         Byte Count Change is enabled.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] bcValue                  - number of bytes added to the length of every frame
*                                      for shaping/scheduling rate calculation.
*                                      (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerShaperByteCountChangeValueSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  bcValue
);

/**
* @internal cpssPxPortTxSchedulerShaperByteCountChangeValueGet function
* @endinternal
*
* @brief   Gets the number of bytes added/subtracted to the length of every frame
*         for shaping/scheduling rate calculation for given port when
*         Byte Count Change is enabled.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] bcValuePtr               - (pointer to) number of bytes added to the length of
*                                      every frame for shaping/scheduling rate calculation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerShaperByteCountChangeValueGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *bcValuePtr
);

/**
* @internal cpssPxPortTxSchedulerProfileCountModeSet function
* @endinternal
*
* @brief   Sets the counting mode for scheduler profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - scheduler profile
* @param[in] wrrMode                  - scheduler count mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile or wrong wrrMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerProfileCountModeSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile,
    IN  CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT      wrrMode
);

/**
* @internal cpssPxPortTxSchedulerProfileCountModeGet function
* @endinternal
*
* @brief   Sets the counting mode for scheduler profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - scheduler profile
*
* @param[out] wrrModePtr               - (pointer to) scheduler count mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile or wrong wrrMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS cpssPxPortTxSchedulerProfileCountModeGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile,
    OUT CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT      *wrrModePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxPortTxScheduler_h__ */

