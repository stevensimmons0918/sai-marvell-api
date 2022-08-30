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
* @file cpssPxPortTxShaper.h
*
* @brief CPSS implementation for configuring the Physical Port Tx Shaper
* This covers:
* - shaping on queues and the logical port
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxPortTxShaper_H
#define __cpssPxPortTxShaper_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/px/cos/cpssPxCos.h>

/**
* @enum CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT
 *
 * @brief Enumeration of shaper granularity.
*/
typedef enum{

    /** Granularity is 64 core clock cycles. */
    CPSS_PX_PORT_TX_SHAPER_GRANULARITY_64_CORE_CLOCKS_E,

    /** Granularity is 1024 core clock cycles. */
    CPSS_PX_PORT_TX_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E,

    /** Granularity is 128 core clock cycles. */
    CPSS_PX_PORT_TX_SHAPER_GRANULARITY_128_CORE_CLOCKS_E,

    /** Granularity is 256 core clock cycles. */
    CPSS_PX_PORT_TX_SHAPER_GRANULARITY_256_CORE_CLOCKS_E

} CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT;


/**
* @struct CPSS_PX_PORT_TX_SHAPER_CONFIG_STC
 *
 * @brief Shaper configurations.
*/
typedef struct{

    /** @brief tokens rate for shapers(1..15).
     *  Granularity of this field defined by portsTokensRateGran.
     */
    GT_U32 tokensRate;

    /** @brief slow rate ratio. Tokens Update rate for ports with
     *  slow rate divided to the ratio (1..16).
     */
    GT_U32 slowRateRatio;

    /** Defines the granularity tokensRate. */
    CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT tokensRateGran;

    /** @brief packet length in bytes for decreasing the shaper token
     *  bucket in packet based mode for non-CPU ports (0..0xFFFFFF).
     */
    GT_U32 portsPacketLength;

    /** @brief packet length in bytes for decreasing the
     *  shaper token bucket in packet based mode for CPU port (0..0xFFFFFF).
     *  Comments:
     *  None.
     */
    GT_U32 cpuPacketLength;

} CPSS_PX_PORT_TX_SHAPER_CONFIG_STC;


/**
* @enum CPSS_PX_PORT_TX_SHAPER_MODE_ENT
 *
 * @brief Token Bucket Shaper mode
*/
typedef enum{

    /** @brief shaper rate is based on the packet's
     *  Byte Count
     */
    CPSS_PX_PORT_TX_SHAPER_BYTE_MODE_E,

    /** @brief shaper rate is based on the number of
     *  packets
     */
    CPSS_PX_PORT_TX_SHAPER_PACKET_MODE_E

} CPSS_PX_PORT_TX_SHAPER_MODE_ENT;


/**
* @internal cpssPxPortTxShaperEnableSet function
* @endinternal
*
* @brief   Enable/Disable Token Bucket rate shaping on specified port of
*         specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE,  Shaping
*                                      GT_FALSE, disable Shaping
* @param[in] avbModeEnable            - AVB mode
*                                      GT_TRUE - AVB mode enabled
*                                      GT_FALSE - AVB mode disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable,
    IN  GT_BOOL              avbModeEnable
);

/**
* @internal cpssPxPortTxShaperEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable Token Bucket rate shaping status on specified port of
*         specified device.
*
* @note   APPLICABLE DEVICES:      Pipe,
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) token Bucket rate shaping status.
* @param[out] avbModeEnablePtr         - (pointer to) AVB mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperEnableGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL              *enablePtr,
    OUT GT_BOOL              *avbModeEnablePtr
);


/**
* @internal cpssPxPortTxShaperProfileSet function
* @endinternal
*
* @brief   Set Token Bucket Shaper Profile on specified port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] burstSize                - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16M burst size)
* @param[in,out] maxRatePtr               - Requested Rate in Kbps or packets per second
*                                      according to port shaper mode.
* @param[in,out] maxRatePtr               - (pointer to) the actual Rate value in Kbps
*                                      or packets per second.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperProfileSet
(
    IN    GT_SW_DEV_NUM         devNum,
    IN    GT_PHYSICAL_PORT_NUM  portNum,
    IN    GT_U32                burstSize,
    INOUT GT_U32               *maxRatePtr
);


/**
* @internal cpssPxPortTxShaperProfileGet function
* @endinternal
*
* @brief   Get Token Bucket Shaper Profile on specified port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] burstSizePtr             - (pointer to) burst size in units of 4K bytes
* @param[out] maxRatePtr               - (pointer to) the actual Rate value in Kbps
*                                      or packets per second.
* @param[out] enablePtr                - (pointer to) Token Bucket rate shaping status.
*                                      - GT_TRUE - Token Bucket rate shaping is enabled.
*                                      - GT_FALSE - Token Bucket rate shaping is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperProfileGet
(
    IN GT_SW_DEV_NUM    devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_U32  *burstSizePtr,
    OUT GT_U32  *maxRatePtr,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssPxPortTxShaperModeSet function
* @endinternal
*
* @brief   Set Shaper mode, packet or byte based for given port.
*         Shapers are configured by cpssPxPortTxShaperProfileSet or
*         cpssPxPortTxShaperQueueProfileSet
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] mode                     - shaper mode: byte count or packet number based.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperModeSet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PX_PORT_TX_SHAPER_MODE_ENT mode
);

/**
* @internal cpssPxPortTxShaperModeGet function
* @endinternal
*
* @brief   Get Shaper mode, packet or byte based for given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] modePtr                  - (pointer to) shaper mode: byte count or packet number based.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperModeGet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PX_PORT_TX_SHAPER_MODE_ENT *modePtr
);

/**
* @internal cpssPxPortTxShaperBaselineSet function
* @endinternal
*
* @brief   Set Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline, the respective queue/port
*         is not served.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] baseline                 - Token Bucket Baseline value in bytes(0..0xFFFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range baseline
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Token Bucket Baseline must be configured as follows:
*       1. At least MTU.
*       2. If PFC response is enabled, the Baseline must be
*       at least 0x3FFFC0, see:
*       cpssPxPortPfcEnableSet.
*       cpssPxPortTxSchedulerDeficitModeEnableSet.
*       3. When packet based shaping is enabled, the following used as
*       shaper's MTU:
*       CPSS_PX_PORT_TX_SHAPER_CONFIG_STC. portsPacketLength
*       CPSS_PX_PORT_TX_SHAPER_CONFIG_STC. cpuPacketLength
*       see:
*       cpssPxPortTxShaperConfigurationSet.
*
*/
GT_STATUS cpssPxPortTxShaperBaselineSet
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32   baseline
);

/**
* @internal cpssPxPortTxShaperBaselineGet function
* @endinternal
*
* @brief   Get Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline, the respective queue/port
*         is not served.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
*
* @param[out] baselinePtr              - (pointer to) Token Bucket Baseline value in bytes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperBaselineGet
(
    IN GT_SW_DEV_NUM    devNum,
    OUT GT_U32   *baselinePtr
);

/**
* @internal cpssPxPortTxShaperConfigurationSet function
* @endinternal
*
* @brief   Set global configuration for shaper.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] configsPtr               - (pointer to) shaper configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When packet based shaping is enabled, the following used as shaper's MTU:
*       configsPtr->portsPacketLength
*       configsPtr->cpuPacketLength
*       see:
*       cpssPxPortTxShaperBaselineSet.
*
*/
GT_STATUS cpssPxPortTxShaperConfigurationSet
(
    IN GT_SW_DEV_NUM    devNum,
    IN  CPSS_PX_PORT_TX_SHAPER_CONFIG_STC *configsPtr
);

/**
* @internal cpssPxPortTxShaperConfigurationGet function
* @endinternal
*
* @brief   Get global configuration for shaper.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
*
* @param[out] configsPtr               - (pointer to) shaper configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssPxPortTxShaperConfigurationGet
(
    IN GT_SW_DEV_NUM    devNum,
    OUT CPSS_PX_PORT_TX_SHAPER_CONFIG_STC *configsPtr
);


/**
* @internal cpssPxPortTxShaperQueueEnableSet function
* @endinternal
*
* @brief   Enable/Disable shaping of transmitted traffic from a specified Traffic
*         Class Queue and specified port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this port (0..7)
* @param[in] enable                   - GT_TRUE,  shaping on this traffic queue
*                                      GT_FALSE, disable shaping on this traffic queue
* @param[in] avbModeEnable            - AVB mode
*                                      GT_TRUE - AVB mode enabled
*                                      GT_FALSE - AVB mode disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperQueueEnableSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                tcQueue,
    IN  GT_BOOL               enable,
    IN  GT_BOOL               avbModeEnable
);

/**
* @internal cpssPxPortTxShaperQueueEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable shaping status
*         of transmitted traffic from a specified Traffic
*         Class Queue and specified port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this port (0..7)
*
* @param[out] enablePtr                - (pointer to) token Bucket rate shaping status.
* @param[out] avbModeEnablePtr         - (pointer to) AVB mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperQueueEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tcQueue,
    OUT GT_BOOL                *enablePtr,
    OUT GT_BOOL                *avbModeEnablePtr
);


/**
* @internal cpssPxPortTxShaperQueueProfileSet function
* @endinternal
*
* @brief   Set Shaper Profile for Traffic Class Queue of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this Logical Port
* @param[in] burstSize                - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16M burst size)
* @param[in,out] maxRatePtr               - Requested Rate in Kbps or packets per second.
* @param[in,out] maxRatePtr               - (pointer to) the actual Rate value in Kbps or
*                                      or packets per second.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperQueueProfileSet
(
    IN    GT_SW_DEV_NUM     devNum,
    IN    GT_PHYSICAL_PORT_NUM        portNum,
    IN    GT_U32       tcQueue,
    IN    GT_U32       burstSize,
    INOUT GT_U32       *maxRatePtr
);

/**
* @internal cpssPxPortTxShaperQueueProfileGet function
* @endinternal
*
* @brief   Get Shaper Profile for Traffic Class Queue of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on the port
*
* @param[out] burstSizePtr             - (pointer to) burst size in units of 4K bytes
* @param[out] maxRatePtr               - (pointer to) the actual Rate value in Kbps
*                                      or packets per second.
* @param[out] enablePtr                - (pointer to) Token Bucket rate shaping status.
*                                      - GT_TRUE - Token Bucket rate shaping is enabled.
*                                      - GT_FALSE - Token Bucket rate shaping is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperQueueProfileGet
(
    IN GT_SW_DEV_NUM          devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                tcQueue,
    OUT GT_U32               *burstSizePtr,
    OUT GT_U32               *maxRatePtr,
    OUT GT_BOOL              *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxPortTxh */

