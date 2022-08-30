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
* @file cpssPxPortTxTailDrop.h
*
* @brief CPSS implementation for configuring Tail Drop of the Physical Port Tx
* Traffic Class Queues.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxPortTxTailDrop_h__
#define __cpssPxPortTxTailDrop_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>


/**
* @enum CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT
 *
 * @brief There are Drop Profile sets of parameters. This enumeration specifies
 * which Drop Profile set a given physical port is associated with.
*/
typedef enum{

    /** Drop Profile Set #0 */
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E = 0,

    /** Drop Profile Set #1 */
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_1_E,

    /** Drop Profile Set #2 */
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_2_E,

    /** Drop Profile Set #3 */
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_3_E,

    /** Drop Profile Set #4 */
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_4_E,

    /** Drop Profile Set #5 */
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_5_E,

    /** Drop Profile Set #6 */
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_6_E,

    /** Drop Profile Set #7 */
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_7_E,

    /** Drop Profile Set #8 */
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_8_E,

    /** Drop Profile Set #9 */
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_9_E,

    /** Drop Profile Set #10 */
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_10_E,

    /** Drop Profile Set #11 */
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_11_E,

    /** Drop Profile Set #12 */
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_12_E,

    /** Drop Profile Set #13 */
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_13_E,

    /** Drop Profile Set #14 */
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_14_E,

    /** Drop Profile Set #15 */
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E

} CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT;

/* Check the Tail Drop Profile set range */
#define PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_CHECK_MAC(profile)          \
    if ((GT_U32)(profile) > (GT_U32) CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E)  \
    {                                                                         \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);        \
    }

/**
* @enum CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT
 *
 * @brief Enumeration of how packet buffer consumption is calculated for Tail Drop
 * and Scheduling.
*/
typedef enum{

    /** @brief The packet's buffer count is the actual number of buffers occupied
     *  by the received packet.
     */
    CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_BYTE_E,

    /** The packet's buffer count is a fixed configurable value. */
    CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_PACKET_E

} CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT;

/**
* @enum CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT
 *
 * @brief Drop Precedence (DP) enable mode to use the shared pool.
*/
typedef enum{

    /** None of the packets (with any DP) can use the shared pool. */
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DISABLE_E,

    /** All packets (with any DP) can use the shared pool. */
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ALL_E,

    /** Only packets with DP0 can use the shared pool. */
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_E,

    /** Packets with DP0 and DP1 can use the shared pool. */
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_DP1_E

} CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT;

/**
* @enum CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT
 *
 * @brief The Tail Drop Dynamic Buffers Allocation (DBA) ratio of the free buffers
 * used for queue thresholds.
 * Alpha = 1 enables a single congested port to get 50% of free packet
 * buffers. Higher Alphas will give more weight for a single queue allowing
 * better utilization of the packet memory.
 * Amount of buffers for dynamic allocation per port calculated as:
 * PortAlpha
 * DynamicFactor = FreeBuffers -------------------------------------
 * 1 + SumOfAlphaOfAllDestinationPorts
*/
typedef enum{

    /** ratio is 0 */
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E,

    /** ratio is 0.25 */
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E,

    /** ratio is 0.5 */
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E,

    /** ratio is 1 */
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E,

    /** ratio is 2 */
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E,

    /** ratio is 4 */
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E

} CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT;

/**
* @struct CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC
 *
 * @brief The Tail Drop profile Traffic Class parameters.
*/
typedef struct{

    /** @brief the number of buffers that can be allocated to all of
     *  the port's TC Tx queues, for a port that uses this
     *  profile.
     *  for GREEN packets
     */
    GT_U32 dp0MaxBuffNum;

    /** @brief the number of descriptors that can be allocated to all
     *  of the port's TC Tx queues, for a port that uses this
     *  profile.
     *  for GREEN packets
     */
    GT_U32 dp0MaxDescNum;

    /** @brief the number of buffers that can be allocated to all of
     *  the port's TC Tx queues, for a port that uses this
     *  profile.
     *  for YELLOW packets
     */
    GT_U32 dp1MaxBuffNum;

    /** @brief the number of descriptors that can be allocated to all
     *  of the port's TC Tx queues, for a port that uses this
     *  profile.
     *  for YELLOW packets
     */
    GT_U32 dp1MaxDescNum;

    /** @brief the number of buffers that can be allocated to all of
     *  the port's TC Tx queues, for a port that uses this
     *  profile.
     *  for RED packets
     */
    GT_U32 dp2MaxBuffNum;

    /** @brief the number of descriptors that can be allocated to all
     *  of the port's TC Tx queues, for a port that uses this
     *  profile.
     *  for RED packets
     */
    GT_U32 dp2MaxDescNum;

    /** @brief the number of buffers that can be allocated to all of
     *  the port's TC Tx queues, for a port that uses this
     *  profile.
     */
    GT_U32 tcMaxBuffNum;

    /** @brief the number of descriptors that can be allocated to all
     *  of the port's TC Tx queues, for a port that uses this
     *  profile.
     *  dp0AlphaNum  - the ratio of the free buffers that can be used for
     *  the queue threshold.
     *  for GREEN packets
     *  dp1AlphaNum  - the ratio of the free buffers that can be used for
     *  the queue threshold.
     *  for YELLOW packets
     *  dp2AlphaNum  - the ratio of the free buffers that can be used for
     *  the queue threshold.
     *  for RED packets
     */
    GT_U32 tcMaxDescNum;

    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT dp0QueueAlpha;

    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT dp1QueueAlpha;

    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT dp2QueueAlpha;

} CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC;

/**
* @struct CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC
 *
 * @brief Tail Drop limits enabling for Weigthed Random Tail Drop.
*/
typedef struct{

    /** @brief enable/disable WRTD for (Queue,DP) descriptor/buffer
     *  limits.
     */
    GT_BOOL tcDpLimit;

    /** enable/disable Port descriptor/buffer limits. */
    GT_BOOL portLimit;

    /** enable/disable Queue descriptor/buffer limits. */
    GT_BOOL tcLimit;

    /** enable/disable Shared Pool descriptor/buffer limits. */
    GT_BOOL sharedPoolLimit;

} CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC;

/**
* @struct CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC
 *
 * @brief The Tail Drop WRTD masked lsbs' for each limit type.
*/
typedef struct{

    /** The number of lsbs' masked for WRTD for (Queue,DP) limits. */
    GT_U32 tcDp;

    /** The number of lsbs' masked for WRTD for Port limits. */
    GT_U32 port;

    /** The number of lsbs' masked for WRTD for Queue limits. */
    GT_U32 tc;

    /** The number of lsbs' masked for WRTD for Shared Pool limits. */
    GT_U32 pool;

} CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC;

/**
* @enum CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT
 *
 * @brief Enumeration of shared pool allocation policy
*/
typedef enum{

    /** Unconstrained. A port may consume the entire shared pool. */
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_UNCONSTRAINED_E,

    /** @brief Constrained. The Port Tail Drop Threshold limits the total number
     *  of resources occupied by a port.
     */
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_CONSTRAINED_E

} CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT;


/**
* @internal cpssPxPortTxTailDropProfileIdSet function
* @endinternal
*
* @brief   Bind a Physical Port to a specific Drop Profile Set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] profileSet               - the Profile Set in which the Traffic Class Drop Parameters
*                                      is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPxPortTxTailDropProfileIdSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet
);

/**
* @internal cpssPxPortTxTailDropProfileIdGet function
* @endinternal
*
* @brief   Get Drop Profile Set according to a Physical Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] profileSetPtr            - pointer to the Profile Set in which the Traffic Class
*                                      Drop Parameters is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileIdGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   *profileSetPtr
);

/**
* @internal cpssPxPortTxTailDropUcEnableSet function
* @endinternal
*
* @brief   Enable/Disable tail-dropping for all packets based on the profile limits.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -  device number.
* @param[in] enable                   -  GT_TRUE  - Tail Drop is enabled.
*                                      GT_FALSE - The Tail Drop limits for all packets are
*                                      ignored and packet is dropped only
*                                      when the Tx Queue has reached its global
*                                      descriptors limit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropUcEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
);

/**
* @internal cpssPxPortTxTailDropUcEnableGet function
* @endinternal
*
* @brief   Get enable/disable tail-dropping for all packets based on the profile
*         limits.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - pointer to tail drop status:
*                                      GT_TRUE  - Tail Drop is enabled.
*                                      GT_FALSE - The Tail Drop limits for all packets are ignored
*                                      and packet is dropped only when the Tx Queue has
*                                      reached its global descriptors limit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropUcEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssPxPortTxTailDropProfileSet function
* @endinternal
*
* @brief   Set maximal port's limits of buffers and descriptors.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Traffic Class Drop
*                                      Parameters is associated
* @param[in] portMaxBuffLimit         - maximal number of buffers for a port.
*                                      (APPLICABLE RANGES: 0..0x0FFFFF).
* @param[in] portMaxDescLimit         - maximal number of descriptors for a port.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
* @param[in] portAlpha                - ratio of the free buffers used for the port
*                                      thresholds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       portAlpha
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      portMaxBuffLimit,
    IN  GT_U32                                      portMaxDescLimit,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT     portAlpha
);

/**
* @internal cpssPxPortTxTailDropProfileGet function
* @endinternal
*
* @brief   Get maximal port's limits of buffers and descriptors.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Profile Set in which the Traffic Class Drop Parameters
*                                      is associated.
*
* @param[out] portMaxBuffLimitPtr      - pointer to maximal number of buffers for a port.
* @param[out] portMaxDescLimitPtr      - pointer to maximal number of descriptors for a
*                                      port.
* @param[out] portAlphaPtr             - pointer to the ratio of the free buffers used for
*                                      the port thresholds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet,
    OUT GT_U32                                      *portMaxBuffLimitPtr,
    OUT GT_U32                                      *portMaxDescLimitPtr,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT     *portAlphaPtr
);

/**
* @internal cpssPxPortTxTailDropPacketModeLengthSet function
* @endinternal
*
* @brief   Configure the packet length used for Tail Drop in packet mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] length                   - Defines the number of buffers consumed by a packet for
*                                      Tail Drop.
*                                      (APPLICABLE RANGES: 0..63).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on length out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function should be called only when traffic disabled
*       Mode configuration for Pipe performed per profile.
*
*/
GT_STATUS cpssPxPortTxTailDropPacketModeLengthSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          length
);

/**
* @internal cpssPxPortTxTailDropPacketModeLengthGet function
* @endinternal
*
* @brief   Get the packet length used for Tail Drop in packet mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] lengthPtr                - (pointer to) the number of buffers consumed by a packet for
*                                      Tail Drop.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropPacketModeLengthGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *lengthPtr
);

/**
* @internal cpssPxPortTxTailDropProfileBufferConsumptionModeSet function
* @endinternal
*
* @brief   Sets the packet buffer consumption mode per Tail Drop profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - tail drop profile
* @param[in] mode                     - buffer consumption mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile or wrong mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileBufferConsumptionModeSet
(
    IN  GT_SW_DEV_NUM                                           devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT               profile,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT   mode
);

/**
* @internal cpssPxPortTxTailDropProfileBufferConsumptionModeGet function
* @endinternal
*
* @brief   Gets the packet buffer consumption mode per Tail Drop profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - tail drop profile
*
* @param[out] modePtr                  - (pointer to) buffer consumption mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS cpssPxPortTxTailDropProfileBufferConsumptionModeGet
(
    IN  GT_SW_DEV_NUM                                           devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT               profile,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT   *modePtr
);

/**
* @internal cpssPxPortTxTailDropMaskSharedBuffEnableSet function
* @endinternal
*
* @brief   Enable/Disable shared buffer indication masking. When enabled, the tail
*         drop mechanism will ignore the buffer fill level indication.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Enable buffer fill level masking.
*                                      GT_FALSE - Disable buffer fill level masking.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropMaskSharedBuffEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
);

/**
* @internal cpssPxPortTxTailDropMaskSharedBuffEnableGet function
* @endinternal
*
* @brief   Gets status of shared buffer indication masking. When enabled, the tail
*         drop mechanism will ignore the buffer fill level indication.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) shared buffer indication masking.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS cpssPxPortTxTailDropMaskSharedBuffEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssPxPortTxTailDropSharedBuffMaxLimitSet function
* @endinternal
*
* @brief   Set max shared buffer limit value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] maxSharedBufferLimit     - maximal shared buffer limit.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on illegal maxSharedBufferLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropSharedBuffMaxLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          maxSharedBufferLimit
);

/**
* @internal cpssPxPortTxTailDropSharedBuffMaxLimitGet function
* @endinternal
*
* @brief   Get max shared buffer limit value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] maxSharedBufferLimitPtr  - (pointer to) maximal shared buffer limit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS cpssPxPortTxTailDropSharedBuffMaxLimitGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *maxSharedBufferLimitPtr
);

/**
* @internal cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet function
* @endinternal
*
* @brief   Enable/disable Weighted Random Tail Drop Threshold to overcome
*         synchronization.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - the Profile Set in which the Traffic Class Drop Parameters
*                                      is associated.
* @param[in] dp                       - Drop Precedence.
*                                      (APPLICABLE RANGES: 0..2).
* @param[in] tc                       - the Traffic Class.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] enablerPtr               - (pointer to) Tail Drop limits enabling for Weigthed Random
*                                      Tail Drop
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile,
    IN  GT_U32                                      dp,
    IN  GT_U32                                      tc,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC  *enablerPtr
);

/**
* @internal cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet function
* @endinternal
*
* @brief   Get Random Tail drop Threshold status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - the Profile Set in which the Traffic Class Drop Parameters
*                                      is associated.
* @param[in] dp                       - Drop Precedence
*                                      (APPLICABLE RANGES: 0..2).
* @param[in] tc                       - the Traffic Class.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] enablerPtr               - (pointer to) Tail Drop limits enabling for Weigthed Random
*                                      Tail Drop.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile,
    IN  GT_U32                                      dp,
    IN  GT_U32                                      tc,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC  *enablerPtr
);

/**
* @internal cpssPxPortTxTailDropWrtdMasksSet function
* @endinternal
*
* @brief   Sets Weighted Random Tail Drop (WRTD) masks.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] maskLsbPtr               - WRTD masked least significant bits.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropWrtdMasksSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC *maskLsbPtr
);

/**
* @internal cpssPxPortTxTailDropWrtdMasksGet function
* @endinternal
*
* @brief   Gets Weighted Random Tail Drop (WRTD) masks.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] maskLsbPtr               - (pointer to) WRTD masked least significant bits.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropWrtdMasksGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC *maskLsbPtr
);

/**
* @internal cpssPxPortTxTailDropProfileTcSharingSet function
* @endinternal
*
* @brief   Enable/Disable usage of the shared descriptors / buffer pool for
*         packets with the traffic class (tc) that are transmited via a port that
*         is associated with the Profile (profile). Sets the shared pool
*         associated for traffic class and profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - the Profile Set in which the Traffic Class Drop Parameters
*                                      is associated.
* @param[in] tc                       - the Traffic Class.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] enableMode               - Drop Precedence (DPs) enabled mode for sharing.
* @param[in] poolNum                  - shared pool associated.
*                                      (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileTcSharingSet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT       profile,
    IN  GT_U32                                          tc,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    enableMode,
    IN  GT_U32                                          poolNum
);

/**
* @internal cpssPxPortTxTailDropProfileTcSharingGet function
* @endinternal
*
* @brief   Get usage of the shared descriptors / buffer pool status for packets
*         with the traffic class (tc) that are transmited via a port that is
*         associated with the Profile (profile). Get the shared pool associated
*         for traffic class and Profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - the Profile Set in which the Traffic Class Drop
*                                      Parameters is associated.
* @param[in] tc                       - the Traffic Class.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] enableModePtr            - (pointer to) Drop Precedence (DPs) enabled mode for
*                                      sharing.
* @param[out] poolNumPtr               - (pointer to) shared pool associated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, pfSet
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileTcSharingGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT       profile,
    IN  GT_U32                                          tc,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    *enableModePtr,
    OUT GT_U32                                          *poolNumPtr
);

/**
* @internal cpssPxPortTxTailDropProfileTcSet function
* @endinternal
*
* @brief   Set tail drop profiles limits for particular TC.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - the Profile Set in which the Traffic Class
*                                      Drop Parameters is associated.
* @param[in] tc                       - the Traffic Class associated with this set
*                                      of Drop Parameters.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] tailDropProfileParamsPtr - the Drop Profile Parameters to associate
*                                      with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileTcSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile,
    IN  GT_U32                                      tc,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    *tailDropProfileParamsPtr
);

/**
* @internal cpssPxPortTxTailDropProfileTcGet function
* @endinternal
*
* @brief   Get tail drop profiles limits for particular TC.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - the Profile Set in which the Traffic Class
*                                      Drop Parameters is associated.
* @param[in] tc                       - the Traffic Class associated with this set
*                                      of Drop Parameters.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] tailDropProfileParamsPtr - (pointer to) the Drop Profile Parameters to
*                                      associate with the Traffic Class in this
*                                      Profile.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileTcGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile,
    IN  GT_U32                                      tc,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    *tailDropProfileParamsPtr
);

/**
* @internal cpssPxPortTxTailDropTcBuffNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated on specified port
*         for specified Traffic Class queues.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] tc                       - traffic class.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] numberPtr                - (pointer to) the number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropTcBuffNumberGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tc,
    OUT GT_U32                  *numberPtr
);

/**
* @internal cpssPxPortTxTailDropMcastPcktDescLimitSet function
* @endinternal
*
* @brief   Set maximal descriptors limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mcastMaxDescNum          - The number of descriptors allocated for multicast
*                                      packets.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropMcastPcktDescLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          mcastMaxDescNum
);

/**
* @internal cpssPxPortTxTailDropMcastPcktDescLimitGet function
* @endinternal
*
* @brief   Get maximal descriptors limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] mcastMaxDescNumPtr       - (pointer to) the number of descriptors allocated
*                                      for multicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropMcastPcktDescLimitGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *mcastMaxDescNumPtr
);

/**
* @internal cpssPxPortTxTailDropMcastBuffersLimitSet function
* @endinternal
*
* @brief   Set maximal buffers limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mcastMaxBuffNum          - The number of buffers allocated for multicast packets.
*                                      (APPLICABLE RANGES: 0..0xFFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range mcastMaxBuffNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropMcastBuffersLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          mcastMaxBuffNum
);

/**
* @internal cpssPxPortTxTailDropMcastBuffersLimitGet function
* @endinternal
*
* @brief   Get maximal buffers limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] mcastMaxBuffNumPtr       - (pointer to) the number of buffers allocated for
*                                      multicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropMcastBuffersLimitGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *mcastMaxBuffNumPtr
);

/**
* @internal cpssPxPortTxTailDropMcastDescNumberGet function
* @endinternal
*
* @brief   Gets total number of multi-target descriptors allocated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Multi-target descriptors are descriptors that are duplicated to
*       multiple egress targets, e.g. Multicast, Broadcast, or unknown Unicast
*       flooded packets.
*
*/
GT_STATUS cpssPxPortTxTailDropMcastDescNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *numberPtr
);

/**
* @internal cpssPxPortTxTailDropMcastBuffNumberGet function
* @endinternal
*
* @brief   Gets total number of multi-target buffers allocated (virtual buffers).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numberPtr                - number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The buffers associated with enqueued multi-target descriptors are
*       counted multiple times, once for each multi-target descriptor instance.
*       Multi-target descriptors are descriptors that are duplicated to
*       multiple egress targets, e.g. Multicast, Broadcast, or unknown Unicast
*       flooded packets.
*
*/
GT_STATUS cpssPxPortTxTailDropMcastBuffNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *numberPtr
);

/**
* @internal cpssPxPortTxTailDropSharedPolicySet function
* @endinternal
*
* @brief   Sets shared pool allocation policy for enqueuing of packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] policy                   - shared pool allocation policy.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or wrong policy
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropSharedPolicySet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT policy
);

/**
* @internal cpssPxPortTxTailDropSharedPolicyGet function
* @endinternal
*
* @brief   Gets shared pool allocation policy for enqueuing of packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] policyPtr                - (pointer to) shared pool allocation policy.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropSharedPolicyGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT *policyPtr
);

/**
* @internal cpssPxPortTxTailDropSharedPoolLimitsSet function
* @endinternal
*
* @brief   Set maximal descriptors and buffers limits for shared pool.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] poolNum                  - Shared pool number.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] maxBuffNum               - The number of buffers allocated for a shared pool.
*                                      (APPLICABLE RANGES: 0..0xFFFFF).
* @param[in] maxDescNum               - The number of descriptors allocated for a shared pool.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range value
*/
GT_STATUS cpssPxPortTxTailDropSharedPoolLimitsSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          poolNum,
    IN  GT_U32          maxBuffNum,
    IN  GT_U32          maxDescNum
);

/**
* @internal cpssPxPortTxTailDropSharedPoolLimitsGet function
* @endinternal
*
* @brief   Get maximal descriptors and buffers limits for shared pool.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] poolNum                  - Shared pool number.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] maxBuffNumPtr            - (pointer to) the number of buffers allocated for a
*                                      shared pool.
* @param[out] maxDescNumPtr            - (pointer to) the number of descriptors allocated
*                                      for a shared pool.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropSharedPoolLimitsGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          poolNum,
    OUT GT_U32          *maxBuffNumPtr,
    OUT GT_U32          *maxDescNumPtr
);

/**
* @internal cpssPxPortTxTailDropSharedResourceDescNumberGet function
* @endinternal
*
* @brief   Gets total number of descriptors allocated from the shared descriptors
*         pool.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong poolNum or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropSharedResourceDescNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          tcQueue,
    OUT GT_U32          *numberPtr
);

/**
* @internal cpssPxPortTxTailDropSharedResourceBuffNumberGet function
* @endinternal
*
* @brief   Gets the number of virtual buffers enqueued from the shared buffer pool.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] numberPtr                - number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong poolNum or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropSharedResourceBuffNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          tcQueue,
    OUT GT_U32          *numberPtr
);

/**
* @internal cpssPxPortTxTailDropGlobalDescNumberGet function
* @endinternal
*
* @brief   Gets total number of descriptors allocated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropGlobalDescNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *numberPtr
);

/**
* @internal cpssPxPortTxTailDropGlobalBuffNumberGet function
* @endinternal
*
* @brief   Gets total number of virtual buffers enqueued.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numberPtr                - number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The buffers associated with enqueued multi-target descriptors are
*       counted multiple times, once for each multi-target descriptor instance.
*
*/
GT_STATUS cpssPxPortTxTailDropGlobalBuffNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *numberPtr
);

/**
* @internal cpssPxPortTxTailDropDescNumberGet function
* @endinternal
*
* @brief   Gets the current number of descriptors allocated per specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
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
GT_STATUS cpssPxPortTxTailDropDescNumberGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *numberPtr
);

/**
* @internal cpssPxPortTxTailDropBuffNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated per specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] numberPtr                - number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropBuffNumberGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    OUT  GT_U32                 *numberPtr
);

/**
* @internal cpssPxPortTxTailDropDbaModeEnableSet function
* @endinternal
*
* @brief   Enable/disable Dynamic Buffers Allocation (DBA) mode of resource
*         management.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE -  DBA mode enabled
*                                      GT_FALSE - DBA mode disabled.
*/
GT_STATUS cpssPxPortTxTailDropDbaModeEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
);

/**
* @internal cpssPxPortTxTailDropDbaModeEnableGet function
* @endinternal
*
* @brief   Get state of Dynamic Buffers Allocation (DBA) mode of resource
*         management.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - pointer to DBA mode state.
*                                      RETURN:
*                                      GT_OK                    - on success
*                                      GT_FAIL                  - on error
*                                      GT_BAD_PTR               - on NULL pointer
*                                      GT_BAD_PARAM             - on wrong device number
*                                      GT_HW_ERROR              - on hardware error
*                                      GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS cpssPxPortTxTailDropDbaModeEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssPxPortTxTailDropDbaAvailableBuffSet function
* @endinternal
*
* @brief   Set amount of buffers available for dynamic allocation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] availableBuff            - amount of buffers available for dynamic allocation.
*                                      (APPLICABLE RANGES: 0..0x0FFFFF).
*
* @note Valid only if DBA mode enabled.
*
*/
GT_STATUS cpssPxPortTxTailDropDbaAvailableBuffSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          availableBuff
);

/**
* @internal cpssPxPortTxTailDropDbaAvailableBuffGet function
* @endinternal
*
* @brief   Get amount of buffers available for dynamic allocation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] availableBuffPtr         - pointer to amount of buffers available for dynamic
*                                      allocation.
*                                      RETURN:
*                                      GT_OK                    - on success
*                                      GT_FAIL                  - on error
*                                      GT_BAD_PTR               - on NULL pointer
*                                      GT_BAD_PARAM             - on wrong device number
*                                      GT_HW_ERROR              - on hardware error
*                                      GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS cpssPxPortTxTailDropDbaAvailableBuffGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *availableBuffPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxPortTxTailDrop_h__ */

