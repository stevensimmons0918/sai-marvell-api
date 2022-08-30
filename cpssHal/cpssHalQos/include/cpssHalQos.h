/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/*******************************************************************************
* @file cpssHalQos.h
*
* @brief Internal header which defines API for helpers functions, which are
*        specific for XPS QoS.
*
* @version   01
*******************************************************************************/

#ifndef _cpssHalQos_h_
#define _cpssHalQos_h_

#include "xpsCommon.h"
#include "xpsReasonCodeTable.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "cpssDxChNetIf.h"
#include "cpssDxChPortTx.h"
#include <gtOs/gtGenTypes.h>
#include <cpss/generic/port/cpssPortTx.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPfc.h>
#include "cpssDxChPortCtrl.h"
#include "cpssDxChPortEcn.h"
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>
#include <cpss/generic/policer/cpssGenPolicerTypes.h>

#define CPSS_HAL_QOS_NUM_CPU_QUEUES  (8)
#define CPSS_HAL_MAX_PORT_POLICER_INDEX 2048
#define CPSS_HAL_MAX_FLOW_POLICER_INDEX 1024


GT_STATUS cpssHalQosBindReasonCodeToCpuQueue(uint32_t devId,
                                             CPSS_NET_RX_CPU_CODE_ENT reasonCode, uint32_t queueNum);

GT_STATUS cpssHalQosGetReasonCodeToCpuQueue(uint32_t devId,
                                            CPSS_NET_RX_CPU_CODE_ENT reasonCode, uint32_t *queueNum);

/**
* @internal cpssHalPortTxSharedPoolLimitsSet function
* @endinternal
*
* @brief   Set maximal descriptors and buffers limits for shared pool.
*
* @param[in] devNum                   - physical device number
* @param[in] poolNum                  - Shared pool number.
*                                       Falcon :Range 0..1
* @param[in] maxBufNum                - The number of buffers allocated for a shared pool.
*                                       Falcon:Range   0..0xFFFFF
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range value
*/

GT_STATUS cpssHalPortTxSharedPoolLimitsSet
(
    uint32_t devId,
    uint32_t poolNum,
    uint32_t maxBufNum,
    GT_BOOL  isIngress
);

/**
* @internal  cpssHalPortTxTailDropGlobalParamsSet function
* @endinternal
*
* @brief  Set amount of available buffers for dynamic buffers allocation
*
* @param[in] devNum                   - Device number
* @param[in] resourceMode             - Defines which free buffers resource is used to calculate
*                                       the dynamic buffer limit for the Port/Queue limits(global
*                                       or pool)
* @param[in]  globalAvailableBuffers   - Global amount of available buffers for dynamic buffers allocation
* @param[in]  pool0AvailableBuffers    - Pool 0 amount of available buffers for dynamic buffers allocation
* @param[in]  pool1AvailableBuffers    - Pool 1 amount of available buffers for dynamic buffers allocation
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on invalid input parameters value
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/

GT_STATUS cpssHalPortTxTailDropGlobalParamsSet
(
    uint32_t devId,
    CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT resourcemode,
    uint32_t globalAvailableBuffers,
    uint32_t pool0AvailableBuffers,
    uint32_t pool1AvailableBuffers
);


GT_STATUS cpssHalPortTxTailDropConfigureAvaileblePool
(
    uint32_t devId,
    uint32_t poolId,
    uint32_t poolAvailableBuffers,
    GT_BOOL  isIngress
);

/**
* @internal cpssHalPortTx4TcTailDropProfileSet function
* @endinternal
*
* @brief   Set tail drop profiles limits for particular TC.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                       Drop Parameters (0..7).For Falcon only (0..15)
* @param[in] tailDropProfileParamsPtr -
*                                      the Drop Profile Parameters to associate
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

GT_STATUS cpssHalPortTx4TcTailDropProfileSet(uint32_t devId,
                                             CPSS_PORT_TX_DROP_PROFILE_SET_ENT profileset,
                                             uint8_t trafficClass, CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_STC
                                             *tailDropProfileParamsPtr);

/**
* @internal cpssHalPortPfcGlobalTcThresholdSet function
* @endinternal
*
* @brief   Set global TC enable and threshold
*  When enable, once Global TC counter is above the calculated threshold,
*  PFC OFF message is sent to all ports, for the specific TC.
*
* @param[in] devNum             - device number.
* @param[in] tc                        - Traffic class[0..7]
* @param[in] enable               - Global TC PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             -wrong input parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/

GT_STATUS cpssHalPortPfcGlobalTcThresholdSet(uint32_t devId, uint8_t tc,
                                             GT_BOOL enable, CPSS_DXCH_PFC_THRESHOLD_STC *thresholdCfgPtr,
                                             CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr);

/**
* @internal cpssHalPortPfcPortThresholdSet function
* @endinternal
*
* @brief   Sets  PFC OFF port limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port is more than thresholdCfg limit.
*
* @param[in] devNum                   -       physical device number
* @param[in] portNum                        - physical port number
* @param[in] enable                -          Global port PFC enable option.
* @param[in] thresholdCfgPtr -        (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/

GT_STATUS cpssHalPortPfcPortThresholdSet(uint8_t  devId,
                                         GT_PHYSICAL_PORT_NUM portNum, GT_BOOL enable,
                                         CPSS_DXCH_PFC_THRESHOLD_STC *thresholdCfgPtr,
                                         CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr);

/**
* @internal cpssHalPortXonLimitSet function
* @endinternal
*
* @brief   Sets X-ON port limit. When Flow Control is enabled, X-ON packet sent
*         when the number of buffers for this port is less than xonLimit.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] xonLimit                 - X-ON limit in resolution of 2 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Since in CH3 the HW resolution is 8, in case the xonLimit input
*       parameter is not a multiple of 8 we round it UP to the nearest one.
*       For Bobcat2 buffer resolution in HW is 1.
*
*/

GT_STATUS cpssHalPortXonLimitSet(uint8_t devId,
                                 CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet, uint32_t xonLimit);

/**
* @internal cpssHalPortXoffLimitSet function
* @endinternal
*
* @brief   Sets X-OFF port limit. When Flow Control is enabled, X-OFF packet sent
*         when the number of buffers for this port is less than xoffLimit.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] xoffLimit                - X-OFF limit in resolution of 2 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Since in CH3 the HW resolution is 8, in case the xoffLimit input
*       parameter is not a multiple of 8 we round it UP to the nearest one.
*       For Bobcat2 buffer resolution in HW is 1.
*
*/

GT_STATUS cpssHalPortXoffLimitSet(uint8_t devId,
                                  CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet, uint32_t xoffLimit);

/**
* @internal cpssHalPortRxBufLimitSet function
* @endinternal
*
* @brief   Sets receive buffer limit threshold for the specified port.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] rxBufLimit               - buffer limit threshold in resolution of 4 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Since in CH3 and above, the HW resolution is 16, in case the rxBufLimit
*       input parameter is not a multiple of 16 we round it UP to the nearest one.
*       For Bobcat2 buffer resolution in HW is 1.
*
*/

GT_STATUS cpssHalPortRxBufLimitSet(uint8_t devId,
                                   CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet, uint32_t rxBufLimit);

/**
* @internal cpssHalPortRxFcProfileSet function
* @endinternal
*
* @brief   Bind a port to a flow control profile.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS cpssHalPortRxFcProfileSet(uint8_t devId, GT_PHYSICAL_PORT_NUM portNum,
                                    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet);

GT_STATUS cpssHalGetNumberOfQueuesPerPort(uint8_t devId, uint32_t port,
                                          uint32_t* numQs);

/**
* @internal cpssHalCncQueueStatusLimitSet function
* @endinternal
*
* @brief   Set the limit of queue buffers consumption for triggering queue statistics counting.
*
* @param[in] devId            - device number.
* @param[in] profileSet        - profile set.
* @param[in] tcQueue           - traffic class queue (0..15)
* @param[in] queueLimit        - queue threshold limit.(APPLICABLE RANGES : 0 - 0xfffff)
*
* @retval GT_OK                - on success
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_BAD_PARAM         - on wrong device number or profile or Tc.
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE  - on limit out of range
*/
GT_STATUS cpssHalCncQueueStatusLimitSet
(
    IN GT_U8                               devId,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN GT_U8                               tcQueue,
    IN GT_U32                              queueLimit
);


/**
* @internal cpssHalPortTxBindPortToDpSet function
* @endinternal
*
* @brief   Bind a Physical Port to a specific Drop Profile Set.
*
* @param[in] devId                    - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssHalPortTxBindPortToDpSet
(
    IN  GT_U8                               devId,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet
);

/**
* @internal cpssHalPortPfcCounterGet function
* @endinternal
*
* @brief   Get PFC counter value and watermark per PFC counter and traffic class.
*
* @param[in] devId                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] pfcCounterNum            - PFC counter (APPLICABLE RANGES: 0..127)
* @param[out] pfcCounterValuePtr       - (pointer to) PFC counter value
* @param[out] pfcMaxValuePtr           - (pointer to) PFC watermark counter value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
*@note    In SIP6 pfcCounterNum is used as physical port number.
*@note    In case tcQueue equal 0xFF the counter value refer to all TC's consumed by port.
*/
GT_STATUS cpssHalPortPfcCounterGet
(
    IN  GT_U8   devId,
    IN  GT_U8   tcQueue,
    IN  GT_U32  pfcCounterNum,
    OUT GT_U32  *pfcCounterValuePtr,
    OUT GT_U32  *pfcMaxValuePtr
);

/**
* @internal cpssHalPortTxBindPortToDpGet function
* @endinternal
*
* @brief   Bind a Physical Port to a specific Drop Profile Set.
*
* @param[in] devId                    - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/

GT_STATUS cpssHalPortTxBindPortToDpGet
(
    IN  GT_U8                               devId,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   *profileSetPtr
);
/**
* @internal cpssHalPortTx4TcTailDropProfileGet function
* @endinternal
*
* @brief   Get tail drop profiles limits for particular TC.
*
* @note   APPLICABLE DEVICES:      xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                                           Drop Parameters (0..7).For Falcon only (0..15)
*
* @param[out] tailDropProfileParamsPtr - Pointer to
*                                      the Drop Profile Parameters to associate
*                                      with the Traffic Class in this Profile.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS cpssHalPortTx4TcTailDropProfileGet(uint32_t devId,
                                             CPSS_PORT_TX_DROP_PROFILE_SET_ENT profileSet,
                                             uint8_t trafficClass,
                                             CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_STC *tailDropProfileParamsPtr);

/**
* @internal cpssHalPortPfcPortTcThresholdSet function
* @endinternal
*
* @brief   Sets  PFC OFF port/tc limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port/TC  is more than thresholdCfg limit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tc                        -      Traffic class[0..7]
* @param[in] enable                       port/tc PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssHalPortPfcPortTcThresholdSet
(
    IN  GT_U8                              devNum,
    IN  GT_PHYSICAL_PORT_NUM               portNum,
    IN  GT_U8                              tc,
    IN  CPSS_DXCH_PFC_THRESHOLD_STC        *thresholdCfgPtr
);

/**
* @internal internal_cpssDxChPortPfcPortThresholdSet function
* @endinternal
*
* @brief   Sets  PFC OFF port limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port is more than thresholdCfg limit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -       physical device number
* @param[in] portNum                        - physical port number
* @param[in] enable                -          Global port PFC enable option.
* @param[in] thresholdCfgPtr -        (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssHalPortPfcPortPfcPortThresholdSet
(
    IN GT_U8                              devNum,
    IN GT_PHYSICAL_PORT_NUM               portNum,
    IN GT_BOOL                            enable,
    IN CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
    IN CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
);

/*
 * @internal cpssHalPortTxQueueTxEnableSet function
 * @endinternal
 *
 * @brief  Enable/Disable transmission from a Traffic Class queue
 *         on the specified port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tcQueue               - traffic class queue on this Physical Port
 * @param[in] enable                - GT_TRUE, enable transmission from the queue
 *                                    GT_FALSE, disable transmission from the queue
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortTxQueueTxEnableSet(uint32_t devId, uint32_t port,
                                        uint32_t tcQueue, GT_BOOL enable);

/**
 * @internal cpssHalPortTxQueueSchedulerTypeSet function
 * @endinternal
 *
 * @brief  Set Traffic Class Queue scheduling type -SP/DWRR on
 *         specificed port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tcQueue               - traffic class queue on this Physical Port
 * @param[in] arbGroup              - scheduling arbitration group:
 *                                     1) Strict Priority
 *                                     2) WRR Group - 0 (DWRR)
 *
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortTxQueueSchedulerTypeSet(uint32_t devId, uint32_t port,
                                             uint32_t tcQueue, CPSS_PORT_TX_Q_ARB_GROUP_ENT arbGroup);

/**
 * @internal cpssHalPortTxQueueWRRWeightSet function
 * @endinternal
 *
 * @brief  Set Traffic Class Queue DWRR weight on
 *         specificed port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tcQueue               - traffic class queue on this Physical Port
 * @param[in] weight                - wrr weight
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortTxQueueWRRWeightSet(uint32_t devId, uint32_t port,
                                         uint32_t tcQueue, uint32_t weight);

/**
 * @internal cpssHalBindPortToSchedulerProfileGet function
 * @endinternal
 *
 * @brief  Get scheduler profile of a port.
 *
 * @param[in]  devId                - switch Id
 * @param[in]  port                 - port number
 * @param[out] profile              - The Profile Set in which the scheduler's parameters are
 *                                    associated.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalBindPortToSchedulerProfileGet(uint32_t devId, uint32_t port,
                                               uint32_t *profile);

/**
 * @internal cpssHalBindPortToSchedulerProfileSet function
 * @endinternal
 *
 * @brief  Bind a port to scheduler profile set.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] profileSet            - The Profile Set in which the scheduler's parameters are
 *                                    associated.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalBindPortToSchedulerProfileSet(uint32_t devId, uint32_t port,
                                               uint32_t profile);

/**
 * @internal cpssHalPortTxQMaxRateSet function
 * @endinternal
 *
 * @brief   Set MAX Token Bucket Shaper Profile on
 *          specific queue of specified port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tcQueue               - traffic class queue on this Physical Port
 * @param[in] burstSize             - burst size in units of 4K bytes
 * @param[in] maxRatePtr            - Requested Rate in Kbps
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortTxQMaxRateSet(uint32_t devId, uint32_t port,
                                   uint32_t tcQueue, uint32_t burstSize, uint32_t *maxRatePtr);

/**
 * @internal cpssHalPortTxQMinRateSet function
 * @endinternal
 *
 * @brief   Set MIN Token Bucket Shaper Profile on
 *          specific queue of specified port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tcQueue               - traffic class queue on this Physical Port
 * @param[in] burstSize             - burst size in units of 4K bytes
 * @param[in] minRatePtr            - Requested Rate in Kbps
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortTxQMinRateSet(uint32_t devId, uint32_t port,
                                   uint32_t tcQueue, uint32_t burstSize, uint32_t *minRatePtr);

/**
 * @internal cpssHalPortTxQMaxRateEnableSet function
 * @endinternal
 *
 * @brief   Enable/Disable MAX Token Bucket Shaper Profile on
 *          specific queue of specified port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tcQueue               - traffic class queue on this Physical Port
 * @param[in] enable                - GT_TRUE - enable, GT_FALSE - disable
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortTxQMaxRateEnableSet(uint32_t devId, uint32_t port,
                                         uint32_t tcQueue, GT_BOOL enable);

/**
 * @internal cpssHalPortTxQMinRateEnableSet function
 * @endinternal
 *
 * @brief   Enable/Disable MIN Token Bucket Shaper Profile on
 *          specific queue of specified port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tcQueue               - traffic class queue on this Physical Port
 * @param[in] enable                - GT_TRUE - enable, GT_FALSE - disable
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortTxQMinRateEnableSet(uint32_t devId, uint32_t port,
                                         uint32_t tcQueue, GT_BOOL enable);

/**
 * @internal cpssHalPortShaperRateSet function
 * @endinternal
 *
 * @brief   Set Token Bucket Shaper Profile on specified port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] burstSize             - burst size in units of 4K bytes
 * @param[in] maxRatePtr            - Requested Rate in Kbps
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortShaperRateSet(uint32_t devId, uint32_t port,
                                   uint32_t burstSize, uint32_t *maxRatePtr);

/**
 * @internal cpssHalPortShaperEnableSet function
 * @endinternal
 *
 * @brief   Enable/Disable Token Bucket Shaper on specified port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] enable                - GT_TRUE - enable, GT_FALSE - disable
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortShaperEnableSet(uint32_t devId, uint32_t port,
                                     GT_BOOL enable);

/*** QoS Map APIs ***/

/**
 * @internal cpssHalQoSDot1pToTcMapSet function
 * @endinternal
 *
 * @brief   set a L2 QoS map on specific QoS mapping table.
 *
 * @param[in] devId                 - switch Id
 * @param[in] idx                   - QoS mapping table index
 * @param[in] pcp                   - L2 user priority value
 * @param[in] dei                   - value of CFI/DEI bit
 * @param[in] tc                    - traffic class value to be mapped with pcp & dei.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSDot1pToTcMapSet(uint32_t devId, uint32_t idx, uint32_t pcp,
                                    uint32_t dei, uint32_t tc);

/**
 * @internal cpssHalQoSDot1pToDpMapSet function
 * @endinternal
 *
 * @brief   set a L2 QoS map on specific QoS mapping table.
 *
 * @param[in] devId                 - switch Id
 * @param[in] idx                   - QoS mapping table index
 * @param[in] pcp                   - L2 user priority value
 * @param[in] dei                   - value of CFI/DEI bit
 * @param[in] dp                    - drop precedence value to be mapped with pcp & dei.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSDot1pToDpMapSet(uint32_t devId, uint32_t idx, uint32_t pcp,
                                    uint32_t dei, uint32_t dp);

/**
 * @internal cpssHalQoSDscpToTcMapSet function
 * @endinternal
 *
 * @brief   set a L3 QoS map on specific QoS mapping table.
 *
 * @param[in] devId                 - switch Id
 * @param[in] idx                   - QoS mapping table index
 * @param[in] dscp                  - DSCP value of a IP packet
 * @param[in] tc                    - traffic class value to be mapped with pcp & dei.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSDscpToTcMapSet(uint32_t devId, uint32_t idx, uint32_t dscp,
                                   uint32_t tc);

/**
 * @internal cpssHalQoSDscpToDpMapSet function
 * @endinternal
 *
 * @brief   set a L3 QoS map on specific QoS mapping table.
 *
 * @param[in] devId                 - switch Id
 * @param[in] idx                   - QoS mapping table index
 * @param[in] dscp                  - DSCP value of a IP packet
 * @param[in] dp                    - drop precedence value to be mapped with pcp & dei.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSDscpToDpMapSet(uint32_t devId, uint32_t idx, uint32_t dscp,
                                   uint32_t dp);

/**
 * @internal cpssHalQoSPortIngressQosMappingTableIdxSet function
 * @endinternal
 *
 * @brief  Set ingress QoS mapping table index on
 *         specificed port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] idx                   - Ingress QoS mapping Table index
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSPortIngressQosMappingTableIdxSet(uint32_t devId,
                                                     uint32_t port, uint32_t idx);

/**
 * @internal cpssHalPortQosTrustModeSet function
 * @endinternal
 *
 * @brief Configures port's QoS Trust Mode.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] l2trust               - true  - enable l2 Trust mode
 *                                    false - disable l2 trust mode
 * @param[in] l3trust               - true  - enable l3 Trust mode
 *                                    false - disable l3 trust mode
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortQosTrustModeSet(uint32_t devId, uint32_t port,
                                     GT_BOOL l2Trust, GT_BOOL l3Trust);

/**
 * @internal cpssHalQoSPortEgressQosMappingTableIdxSet function
 * @endinternal
 *
 * @brief  Set egress QoS mapping table index on
 *         specificed port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] idx                   - egress QoS mapping Table index
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSPortEgressQosMappingTableIdxSet(uint32_t devId,
                                                    uint32_t port, uint32_t idx);

/**
 * @internal cpssHalPortEgressQosTcDpMappingEnableSet function
 * @endinternal
 *
 * @brief Enable/disable Egress port (TC, DP) mapping to (UP,EXP,DSCP).
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] enable                - enable/disable Egress port {TC, DP} mapping.
 *                                    GT_TRUE  - enable mapping.
 *                                    GT_FALSE - disable mapping.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortEgressQosTcDpMappingEnableSet(uint32_t devId,
                                                   uint32_t port, GT_BOOL enable);

/**
 * @internal cpssHalPortEgressQosDot1pMappingEnableSet function
 * @endinternal
 *
 * @brief Enable/disable Egress port Dot1p mapping.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] enable                - enable/disable Egress port dot1p mapping.
 *                                    GT_TRUE  - enable mapping.
 *                                    GT_FALSE - disable mapping.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortEgressQosDot1pMappingEnableSet(uint32_t devId,
                                                    uint32_t port, GT_BOOL enable);

/**
 * @internal cpssHalPortEgressQosDscpMappingEnableSet function
 * @endinternal
 *
 * @brief Enable/disable Egress port DSCP.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] enable                - enable/disable Egress port DSCP mapping.
 *                                    GT_TRUE  - enable mapping.
 *                                    GT_FALSE - disable mapping.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortEgressQosDscpMappingEnableSet(uint32_t devId,
                                                   uint32_t port, GT_BOOL enable);

/**
 * @internal cpssHalPortEgressQosDscpMappingEnableGet function
 * @endinternal
 *
 * @brief Get Enable/disable Egress port DSCP.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 *
 * @param[out] enable               - Egress port DSCP mapping status
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortEgressQosDscpMappingEnableGet(uint32_t devId,
                                                   uint32_t port, GT_BOOL *enable);

/**
 * @internal cpssHalQoSTcDpToDot1pMapSet function
 * @endinternal
 *
 * @brief   set a tc,dp to dot1p QoS map on specific egress QoS mapping table.
 *
 * @param[in] devId                 - switch Id
 * @param[in] idx                   - Egress QoS mapping table index
 * @param[in] tc                    - traffic class
 * @param[in] dp                    - drop precedence
 * @param[in] dot1p                 - user priority value
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSTcDpToDot1pMapSet(uint32_t devId, uint32_t idx, uint32_t tc,
                                      uint32_t dp, uint32_t dot1p);

/**
 * @internal cpssHalQoSTcDpToDscpMapSet function
 * @endinternal
 *
 * @brief   set a tc,dp to dscp QoS map on specific egress QoS mapping table.
 *
 * @param[in] devId                 - switch Id
 * @param[in] idx                   - Egress QoS mapping table index
 * @param[in] tc                    - traffic class
 * @param[in] dp                    - drop precedence
 * @param[in] dscp                  - DSCP value
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSTcDpToDscpMapSet(uint32_t devId, uint32_t idx, uint32_t tc,
                                     uint32_t dp, uint32_t dscp);

/**
 * @internal cpssHalPortPfcToQueueMapSet function
 * @endinternal
 *
 * @brief   Sets PFC TC to  queue map.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] pfcPrio               - PFC priority
 * @param[in] queue                 - queue number
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortPfcToQueueMapSet(uint32_t devId, uint32_t port,
                                      uint32_t pfcPrio, uint32_t queue);

/**
 * @internal cpssHalQoSTcRemapTableSet function
 * @endinternal
 *
 * @brief   Sets mapping of TC to egress queue.
 *
 * @param[in] devId                 - switch Id
 * @param[in] tc                    - traffic class
 * @param[in] queue                 - queue number
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSTcRemapTableSet(uint32_t devId, uint32_t tc,
                                    uint32_t queue);

/**
 * @internal cpssHalQoSPortDefaultTcSet function
 * @endinternal
 *
 * @brief   Sets default tc value for a specific port.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tc                    - traffic class
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSPortDefaultTcSet(uint32_t devId, uint32_t port,
                                     uint32_t tc);

/**
 * @internal cpssHalQoSPortDefaultTcGet function
 * @endinternal
 *
 * @brief   Get default tc value of a specific port.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 *
 * @param[out] tc                   - traffic class
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSPortDefaultTcGet(uint32_t devId, uint32_t port,
                                     uint32_t *tc);

/*** PFC APIs ***/

/**
 * @internal cpssHalPortPfcTcEnableSet function
 * @endinternal
 *
 * @brief   Sets PFC enable for specific port.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tc                    - traffic class
 * @param[in] enable                - port PFC enable option
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortPfcTcEnableSet(uint32_t devId, uint32_t port, uint32_t tc,
                                    GT_BOOL enable);

/**
 * @internal cpssHalPortPfcTcEnableGet function
 * @endinternal
 *
 * @brief   Get PFC enable for specific port.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tc                    - traffic class
 * @param[out] enable               - port PFC enable option
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortPfcTcEnableGet(uint32_t devId, uint32_t port, uint32_t tc,
                                    GT_BOOL *enable);


/**
 * @internal cpssHalPortPfcTcResponseEnableSet function
 * @endinternal
 *
 * @brief   Enable /Disable PFC response per port.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tc                    - traffic class
 * @param[in] enable                - port PFC response enable option
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortPfcTcResponseEnableSet(uint32_t devId, uint32_t port,
                                            uint32_t tc, GT_BOOL enable);
/**
* @internal cpssHalPortFlowControlModeSet function
* @endinternal
*
* @brief   Sets Flow Control mode on given port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
* @param[in] fcMode                   - flow control mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Lion and above:
*       This function also configures insertion of DSA tag for PFC frames.
*
*/
GT_STATUS cpssHalPortFlowControlModeSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT fcMode
);

/**
* @internal cpssHalPortTx4TcTailDropWredProfileGet function
* @endinternal
*
* @brief   Get tail drop WRED profile  parameters  for particular TC.
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters (0..15).
* @param[out] tailDropWredProfileParamsPtr -
*                                  (pointer to)the Drop Profile WRED Parameters to associate
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
GT_STATUS cpssHalPortTx4TcTailDropWredProfileGet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    OUT   CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS_STC
    *tailDropWredProfileParamsPtr
);
/**
* @internal cpssDxChPortTxTailDropWredProfileGet function
* @endinternal
*
* @brief   Get tail drop port WRED profile  parameters .
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
*
* @param[out] tailDropWredProfileParamsPtr -
*                                      the Drop Profile WRED Parameters to associate
*                                      with the port  in this Profile set.
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
GT_STATUS cpssHalPortTxTailDropWredProfileGet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    OUT   CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC     *tailDropWredProfileParamsPtr
);

/**
* @internal cpssDxChPortTxTailDropProfileGet function
* @endinternal
*
* @brief Get port dynamic limit alpha and set maximal port's limits of buffers
*       and descriptors.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                       Class Drop Parameters is associated
*
* @param[out] portAlphaPtr            - Pointer to ratio of the free buffers
*                                       used for the port thresholds
*                                   (APPLICABLE DEVICES: Aldrin2; Falcon).
* @param[out] portMaxBuffLimitPtr     - Pointer to maximal number of buffers for a port
* @param[out] portMaxDescrLimitPtr    - Pointer to maximal number of descriptors for a port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalPortTxTailDropProfileGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT    profileSet,
    OUT CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT *portAlphaPtr
);



/**
* @internal cpssHalPortTxTailDropProfileSet function
* @endinternal
*
* @brief Configures port dynamic limit alpha and set maximal port's limits of
*       buffers and descriptors.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                       Class Drop Parameters is associated
* @param[in] portAlpha                - ratio of the free buffers used for the port
*                                       thresholds (APPLICABLE DEVICES: Aldrin2; Falcon).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalPortTxTailDropProfileSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT    profileSet,
    IN  CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT portAlpha
);

/**
* @internal cpssHalPortTxTailDropWredProfileSet function
* @endinternal
*
* @brief   Set tail drop port WRED profile  parameters .
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
*
* @param[in] tailDropWredProfileParamsPtr -
*                                      the Drop Profile WRED Parameters to associate
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
GT_STATUS cpssHalPortTxTailDropWredProfileSet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC     *tailDropWredProfileParamsPtr
);

/**
* @internal cpssDxChPortTx4TcTailDropEcnMarkingProfileSet function
* @endinternal
*
* @brief   Set tail drop ECN  profile parameters  for particular TC.
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters (0..15).
* @param[in] tailDropEcnProfileParamsPtr -
*                                      the Drop Profile ECN Parameters to associate
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
GT_STATUS cpssHalPortTx4TcTailDropEcnMarkingProfileSet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    IN    CPSS_PORT_QUEUE_ECN_PARAMS_STC          *tailDropEcnProfileParamsPtr
);

/**
* @internal cpssDxChPortTx4TcTailDropEcnMarkingProfileGet function
* @endinternal
*
* @brief   Get tail drop ECN  profile parameters  for particular TC.
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters (0..15).
* @param[out] tailDropEcnProfileParamsPtr -(pointer to)
*                                      the Drop Profile ECN Parameters to associate
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
GT_STATUS cpssHalPortTx4TcTailDropEcnMarkingProfileGet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    OUT   CPSS_PORT_QUEUE_ECN_PARAMS_STC          *tailDropEcnProfileParamsPtr
);

/**
* @internal  cpssHalPortTxTcMapToSharedPoolSet function
* @endinternal
*
* @brief  Set tail drop mapping between TC to pool
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum       - Device number
* @param[in] tc           - Traffic class [0..7]
* @param[in] poolNum      - Pool id[0..1]
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on invalid input parameters value
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS cpssHalPortTxTcMapToSharedPoolSet
(
    IN  GT_U8                               devNum,
    IN  GT_U8                               tc,
    IN  GT_U32                              poolNum
);

/**
* @internal  cpssHalPortTxTcMapToSharedPoolGet function
* @endinternal
*
* @brief  Get tail drop mapping between TC to pool
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number
* @param[in] tc                       - traffic class [0..7]
* @param[out]  poolNumPtr             - (Pointer to) Pool id[0..1]
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on invalid input parameters value
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS cpssHalPortTxTcMapToSharedPoolGet
(
    IN  GT_U8                               devNum,
    IN  GT_U8                               tc,
    OUT GT_U32                              *poolNumPtr
);

/**
 * @internal cpssHalQosAddPolicerEntry function
 * @endinternal
 *
 * @brief  Add a policer entry to the device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] stage                 - policer stage - ingress-1,2 & egress
 * @param[in] policer index         - policer index
 * @param[in] entryPtr              - pointer to the metering policer entry to be set
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQosAddPolicerEntry(uint32_t devId,
                                    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
                                    uint32_t policerIndex, CPSS_DXCH3_POLICER_METERING_ENTRY_STC *entryPtr);

/**
 * @internal cpssHalQosRemovePolicerEntry function
 * @endinternal
 *
 * @brief  Remove a policer entry to the device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] portNum               - port number
 * @param[in] client                - client type
 * @param[in] stormIndex            - storm index
 * @param[in] policerIndex          - policer index
 * @param[in] policerMode           - policer mode
 * @param[in] entryPtr              - pointer to the metering policer entry to be set
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQosRemovePolicerEntry(uint32_t devId, uint32_t portNum,
                                       uint32_t client, uint32_t stormIndex, uint32_t policerIndex,
                                       uint32_t policerMode, CPSS_DXCH3_POLICER_METERING_ENTRY_STC *entryPtr);

/*
 * @internal cpssHalPolicerBillingEntrySet function
 * @endinternal
 *
 * @brief   Sets Policer Billing Counters.
 *
 * @param[in] devId                 - switch Id
 * @param[in] stage                 - policer stage - ingress-1,2 & egress
 * @param[in] entryIndex            - policer index
 * @param[in] billingCntrPtr        - pointer to the Billing Counters Entry.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPolicerBillingEntrySet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT      stage,
    IN  GT_U32                                entryIndex,
    IN  CPSS_DXCH3_POLICER_BILLING_ENTRY_STC *billingCntrPtr
);

/**
 * @internal cpssHalQosPolicerPortStormTypeIndexSet function
 * @endinternal
 *
 * @brief  Bind a port to storm type.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] stage                 - Policer Stage type: Ingress #0, Ingress #1.
 * @param[in] stormType             - storm type
 * @param[in] index                 - policer  (APPLICABLE RANGES: 0..3)
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQosPolicerPortStormTypeIndexSet(uint32_t devId,
                                                 uint32_t portNum, CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
                                                 CPSS_DXCH_POLICER_STORM_TYPE_ENT stormType, uint32_t stormIndex);

/**
* @internal cpssDxChPortTx4TcBufNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated on specified port
*         for specified Traffic Class queues.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] trafClass                - trafiic class (0..7)
*
* @param[out] numPtr                   - (pointer to) the number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note: Do not call this API under traffic for Lion devices
*
*/
GT_STATUS cpssHalPortTx4TcBufNumberGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8       trafClass,
    OUT GT_U32      *numPtr
);

/**
* @internal cpssHalCncQueueStatisticInit function
* @endinternal
*
* @brief   Initialise egress queue pass drop cnc client.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalCncQueueStatisticInit
(
    IN  GT_U8 devNum
);


GT_STATUS cpssHalCncQueueStatGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            port,
    IN  GT_U32                            queue,
    OUT uint64_t                          *passedBytePtr,
    OUT uint64_t                          *passedPcktPtr,
    OUT uint64_t                          *droppedBytePtr,
    OUT uint64_t                          *droppedPcktPtr
);

GT_STATUS cpssHalCncQueueCounterGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            port,
    IN  GT_U32                            queue,
    IN  GT_U32                            firstBlock,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT uint64_t                          *totalPassedBytePtr,
    OUT uint64_t                          *totalPassedPcktPtr,
    OUT uint64_t                          *totalDroppedBytePtr,
    OUT uint64_t                          *totalDroppedPcktPtr
);

GT_STATUS cpssHalCncQueueWatermarkGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            port,
    IN  GT_U32                            queue,
    OUT uint64_t                          *watermark
);

/**
* @internal cpssDxChPortPfcPoolHeadroomCountersGet
* @endinternal
*
* @brief  Get headroom current and maximal value for specific pool.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]   devNum                   -physical device number
* @param[in]   poolId                        Pool index[0..1]
* @param[out]  currentValPtr           (pointer to)Current  headroom size
* @param[out] maxPeakValPtr        (pointer to)Maximal headroom size

* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssHalPoolHeadroomCountersGet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       poolId,
    IN  GT_BOOL                     extendedPool,
    OUT GT_U32                      *currentValPtr,
    OUT GT_U32                      *maxPeakValPtr
);

/**
* @internal cpssHalPoolCountersGet function
* @endinternal
*
* @brief   Get buffer pool current occupancy and maximum watermark
*
* @param[in] devNum                  - device number
* @param[in] poolId                  - buffer pool [0..1]
* @param[in] extendedPool            - is extended buffer pool
* @param[out] currentValPtr          - (pointer to) current buffer pool occupancy
* @param[out] maxValPtr              - (pointer to) buffer pool watermark occupancy
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssHalPoolCountersGet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       poolId,
    IN  GT_BOOL                     extendedPool,
    OUT GT_U32                      *currentValPtr,
    OUT GT_U32                      *maxValPtr
);


/**
* @internal cpssDxChPortPfcPortTcHeadroomCounterGet
* @endinternal
*
* @brief  Get current headroom size for specific port/TC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] tc                       - Traffic class[0..7]
* @param[out] ctrValPtr               - (pointer to)Headroom size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssHalPortTcHeadroomCounterGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8       tc,
    OUT GT_U32      *ctrValPtr
);

/**
* @internal  cpssHalPortTxTcMapToSharedPoolSet function
* @endinternal
*
* @brief  Set tail drop mapping between TC to pool
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum       - Device number
* @param[in] tc           - Traffic class [0..7]
* @param[in] poolNum      - Pool id[0..1]
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on invalid input parameters value
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS cpssHalPortTxTcMapToSharedEgressPoolSet
(
    IN  GT_U8                               devNum,
    IN  GT_U8                               tc,
    IN  GT_U32                              poolNum
);

/**
* @internal cpssDxChPortEcnMarkingEnableSet function
* @endinternal
*
* @brief   Enable/Disable ECN marking.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - type of IP stack
* @param[in] enable                   - GT_TRUE: ECN marking enable
*                                      GT_FALSE: ECN marking disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or protocolStack parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
XP_STATUS cpssHalEcnMarkingEnableSet
(
    IN GT_U8                               devNum,
    IN GT_BOOL                             enable
);

/**
 * @internal cpssHalPolicerCounterEntryGet function
 * @endinternal
 *
 * @brief  Retrieve Policer Counters for the given index.
 *
 * @param[in] devId                 - switch Id
 * @param[in] stage                 - Policer stage type
 * @param[in] policerIndex          - policer Billing Counters Entry index
 * @param[out] greenCntr            - Green pkt counter.
 * @param[out] yellowCntr           - yellow pkt counter.
 * @param[out] redCntr              - Red pkt counter.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
 * @retval GT_HW_ERROR              - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
 * @retval GT_BAD_STATE             - on counter entry type mismatch.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */

GT_STATUS cpssHalPolicerCounterEntryGet(uint32_t devId,
                                        CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage, uint32_t policerIndex,
                                        uint64_t *greenCntr, uint64_t *yellowCntr, uint64_t *redCntr);

/**
 * @internal cpssHalPolicerCounterEntryClear function
 * @endinternal
 *
 * @brief  Clear Policer Counters for the given index.
 *
 * @param[in] devId                 - switch Id
 * @param[in] stage                 - Policer stage type.
 * @param[in] policerIndex          - policer Billing Counters Entry index
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
 * @retval GT_HW_ERROR              - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
 * @retval GT_BAD_STATE             - on counter entry type mismatch.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */

GT_STATUS cpssHalPolicerCounterEntryClear(uint32_t devId,
                                          CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
                                          uint32_t policerIndex);

/**
 * @internal cpssHalPolicerEPortTriggerEntrySet function
 * @endinternal
 *
 * @brief   Set Policer trigger entry.
 *
 * @param[in] devId                 - switch Id
 * @param[in] stage                 - Policer stage type.
 * @param[in] ePort                 - ePort i.e polcier trigger table index.
 * @param[in] policerIndex          - eport policer trigger table index
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
 * @retval GT_HW_ERROR              - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
 * @retval GT_BAD_STATE             - on counter entry type mismatch.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */

GT_STATUS cpssHalPolicerEPortTriggerEntrySet(uint32_t devId,
                                             CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
                                             uint32_t ePort, uint32_t policerIndex, bool enable);

/**
 * @internal cpssHalPolicerInit function
 * @endinternal
 *
 * @brief  Policer Init.
 *
 * @param[in] devId                 - switch Id
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_TIMEOUT               - on time out.
 * @retval GT_HW_ERROR              - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
 * @retval GT_BAD_STATE             - on counter entry type mismatch.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssHalPolicerInit(uint32_t devId);

/**
 * @internal cpssHalPolicerDeInit function
 * @endinternal
 *
 * @brief  Policer De-Init.
 *
 * @param[in] devId                 - switch Id
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_TIMEOUT               - on time out.
 * @retval GT_HW_ERROR              - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
 * @retval GT_BAD_STATE             - on counter entry type mismatch.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssHalPolicerDeInit(uint32_t devId);

/**
 * @internal cpssHalPolicerPortMeteringEnableSet function
 * @endinternal
 *
 * @brief  Enable/Disble port policer for given stage.
 *
 * @param[in] devId                 - switch Id
 * @param[in] stage                 - Policer stage type
 * @param[in] portNum               - port number
 * @param[in] enable                - enable/disable port based metering
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
 * @retval GT_HW_ERROR              - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
 * @retval GT_BAD_STATE             - on counter entry type mismatch.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssHalPolicerPortMeteringEnableSet(uint32_t devId, uint32_t stage,
                                              uint32_t portNum, GT_BOOL enable);

/**
 * @internal cpssHalPolicerMruGet function
 * @endinternal
 *
 * @brief   Gets the Policer Maximum Receive Unit size.
 *
 * @param[in] devId                 - switch Id
 * @param[in] stage                 - Policer stage type
 * @param[out] mru                  - MRU value in bytes
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
 * @retval GT_HW_ERROR              - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
 * @retval GT_BAD_STATE             - on counter entry type mismatch.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssHalPolicerMruGet(uint32_t devId, uint32_t stage, uint32_t *mru);

/**
* @internal cpssHalPortPfcTcResourceModeSet
* @endinternal
*
* @brief  Configure mode that count occupied buffers (Packet buffer occupied or Pool occupied).
*              In case Pool mode is selected ,headroom subtraction  can be enabled by settin mode to
*              CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E
*
* @note   APPLICABLE DEVICES:      Falcon.
*
* @param[in] devId                 - switch Id
* @param[in] tc                    - Traffic class [0..7]
* @param[in] mode                  - resource mode (PB/Pool/Pool with headroom subtraction)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssHalPortPfcTcResourceModeSet(uint32_t devId, uint32_t tc,
                                          CPSS_PORT_TX_PFC_RESOURCE_MODE_ENT mode);

/**
* @internal cpssHalPortPfcDbaTcAvailableBuffersSet
* @endinternal
*
* @brief   Set amount of buffers available for dynamic allocation for PFC.
*
* @param[in] devId                 - switch Id
* @param[in] tc                    - TC
* @param[in] availableBuffers      - Available buffers for DBA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssHalPortPfcDbaTcAvailableBuffersSet(uint32_t devId, uint32_t tc,
                                                 uint32_t availableBuffers);

GT_STATUS cpssHalQosPortTrustQosMappingTableIndexGet(uint32_t devId,
                                                     uint32_t port, GT_BOOL *useUpAsIndexPtr, uint32_t *tableIdx);

GT_STATUS cpssHalQosMapInit(uint32_t devId);
GT_STATUS cpssHalPolicerTriggerEntryGet(uint32_t devNum,
                                        CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
                                        GT_U32 ePort,
                                        CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC *entryPtr);


#ifdef __cplusplus
}
#endif

#endif //_cpssHalQos_h_
