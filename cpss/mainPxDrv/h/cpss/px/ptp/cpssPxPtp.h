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
* @file cpssPxPtp.h
*
* @brief CPSS Pipe Precision Time Protocol APIs
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxPtph
#define __cpssPxPtph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpss/common/cpssTypes.h>

/**
* @enum CPSS_PX_PTP_TAI_CLOCK_MODE_ENT
 *
 * @brief This enum defines the clock mode.
*/
typedef enum{

    /** Clock connection disabled */
    CPSS_PX_PTP_TAI_CLOCK_MODE_DISABLED_E,

    /** Generate external clock */
    CPSS_PX_PTP_TAI_CLOCK_MODE_OUTPUT_E,

    /** Receive clock and update TOD */
    CPSS_PX_PTP_TAI_CLOCK_MODE_INPUT_E,

} CPSS_PX_PTP_TAI_CLOCK_MODE_ENT;

/**
* @enum CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT
 *
 * @brief This enum defines TAI input clock selection mode.
*/
typedef enum{

    /** TAI PTP PLL clock selection mode. */
    CPSS_PX_PTP_TAI_CLOCK_SELECT_PTP_PLL_E,

    /** TAI Core PLL clock selection mode. */
    CPSS_PX_PTP_TAI_CLOCK_SELECT_CORE_PLL_E

} CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT;

/**
* @enum CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT
 *
 * @brief This enum defines PTP clock frequency in Mhz.
*/
typedef enum{

    CPSS_PX_PTP_10_FREQ_E,

    CPSS_PX_PTP_20_FREQ_E,

    CPSS_PX_PTP_25_FREQ_E

}CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT;

/**
* @struct CPSS_PX_PTP_TAI_TOD_STEP_STC
 *
 * @brief Structure for value of TOD Step.
*/
typedef struct{

    /** @brief whole nanosecond part of the value
     *  (APPLICABLE RANGES: 0..0xFFFF)
     */
    GT_U32 nanoSeconds;

    /** @brief fractional nanosecond part of the value.
     *  Value measured in (1/(2^32)) units.
     */
    GT_U32 fracNanoSeconds;

} CPSS_PX_PTP_TAI_TOD_STEP_STC;

/**
* @enum CPSS_PX_PTP_TAI_TOD_TYPE_ENT
 *
 * @brief This enum defines type of value of TOD (Time Of Day) counter.
*/
typedef enum{

    /** @brief The value of the TOD
     *  when the trigger generated for external device.
     */
    CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E,

    /** Generate function Mask. */
    CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E,

    /** @brief The value to update the TOD
     *  by loading it instead the current TOD
     *  or adding it to the current TOD
     *  or subtracting it from the current TOD.
     */
    CPSS_PX_PTP_TAI_TOD_TYPE_LOAD_VALUE_E,

    /** @brief Copy of the value of the TOD
     *  made by Capture command.
     */
    CPSS_PX_PTP_TAI_TOD_TYPE_CAPTURE_VALUE0_E,

    /** @brief Copy of the value of the TOD
     *  made by Capture command.
     */
    CPSS_PX_PTP_TAI_TOD_TYPE_CAPTURE_VALUE1_E

} CPSS_PX_PTP_TAI_TOD_TYPE_ENT;

/**
* @struct CPSS_PX_PTP_TAI_TOD_COUNT_STC
 *
 * @brief Structure for TOD Counter.
*/
typedef struct{

    /** nanosecond value of the time stamping TOD counter */
    GT_U32 nanoSeconds;

    /** @brief second value of the time stamping TOD counter
     *  48 LSBits are significant.
     */
    GT_U64 seconds;

    /** @brief fractional nanosecond part of the value.
     *  30 MSBits are significant.
     */
    GT_U32 fracNanoSeconds;

} CPSS_PX_PTP_TAI_TOD_COUNT_STC;

/**
* @enum CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT
 *
 * @brief This enum defines possible TOD (Time Of Day) counter functions.
*/
typedef enum{

    /** @brief Copy the value from the TOD
     *  counter shadow to the TOD counter register
     */
    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_UPDATE_E,

    /** @brief Add the value of the TOD
     *  counter shadow to the TOD counter register
     */
    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_INCREMENT_E,

    /** @brief Copy the value of the TOD
     *  counter to the TOD counter shadow register
     */
    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_CAPTURE_E,

    /** @brief update the value of the TOD
     *  Counter by subtracting a preconfigured offset.
     */
    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_DECREMENT_E,

    /** @brief as INCREMENT,
     *  but the TOD Counter modified by a preconfigured
     *  offset over a given period of time.
     */
    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E,

    /** @brief as DECREMENT,
     *  but the TOD Counter modified by a preconfigured
     *  offset over a given period of time.
     */
    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E,

    /** @brief Copy the value
     *  from the TOD counter shadow to the fractional
     *  nanosecond drift register
     */
    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E,

    /** No Operation. */
    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_NOP_E

} CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT;

/**
* @struct CPSS_PX_PTP_TSU_CONTROL_STC
 *
 * @brief Structure for TSU (Time Stamping Unit) Control.
*/
typedef struct{

    /** @brief GT_TRUE
     *  GT_FALSE - the unit disabled.
     */
    GT_BOOL unitEnable;

} CPSS_PX_PTP_TSU_CONTROL_STC;

/**
* @enum CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT
 *
 * @brief This enum defines the packet counter type.
*/
typedef enum{

    /** @brief Counts the total number
     *  of packets that are subject to any kind of time stamping action in
     *  the egress time stamping unit (TSU).
     */
    CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E,

    /** Counts PTP V1 packets */
    CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V1_E,

    /** Counts PTP V2 packets */
    CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V2_E,

    /** @brief Counts Add
     *  Corrected Time Action packets
     */
    CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ADD_CORR_TIME_E

} CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT;

/*
 * typedef: CPSS_PX_PTP_TYPE_INDEX
 *
 * Description: scalar representing PTP type index
 *          (APPLICABLE RANGE: 0..31)
 */
typedef GT_U32  CPSS_PX_PTP_TYPE_INDEX;

/* Number of PTP types */
#define CPSS_PX_PTP_TYPE_NUM_CNS 32

/* Number of UDBs in PTP type key */
#define CPSS_PX_PTP_TYPE_UDB_NUM_CNS 6

/* Number of UDBPs in PTP type key */
#define CPSS_PX_PTP_TYPE_UDBP_NUM_CNS 2

/* Maximal offset for UDBs and UDBPs */
#define CPSS_PX_PTP_UDB_OFFSET_MAX_CNS 127

/**
* @struct CPSS_PX_PTP_TYPE_KEY_STC
 *
 * @brief This structure defines port PTP type key.
*/
typedef struct{

    CPSS_PX_UDB_PAIR_KEY_STC portUdbPairArr[CPSS_PX_PTP_TYPE_UDBP_NUM_CNS];

    CPSS_PX_UDB_KEY_STC portUdbArr[CPSS_PX_PTP_TYPE_UDB_NUM_CNS];

} CPSS_PX_PTP_TYPE_KEY_STC;

/**
* @struct CPSS_PX_PTP_TYPE_KEY_FORMAT_STC
 *
 * @brief PTP Type Key Table format
*/
typedef struct{

    /** if PTP the packet has UDP as L4 protocol */
    GT_BOOL isUdp;

    /** @brief source port profile index
     *  (APPLICABLE RANGES: 0..127)
     */
    GT_U32 srcPortProfile;

    CPSS_PX_UDB_PAIR_DATA_STC udbPairsArr[CPSS_PX_PTP_TYPE_UDBP_NUM_CNS];

    GT_U8 udbArr[CPSS_PX_PTP_TYPE_UDB_NUM_CNS];

} CPSS_PX_PTP_TYPE_KEY_FORMAT_STC;

/**
* @enum CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT
 *
 * @brief PulseIn configuration mode.
*/
typedef enum{

    /** @brief Pulse IN connected to PTP_PULSE_IN
     */
    CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E,

    /** @brief Pulse IN is connected to CLOCK_IN
     */
    CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E,

    /** @brief Pulse IN is disabled
     */
    CPSS_PX_PTP_TAI_PULSE_IN_DISABLED_E,
} CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT;

/**
* @internal cpssPxPtpTaiCaptureOverwriteEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Capture Overwrite Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - Capture Overwrite Enable.
*                                      When both TOD Capture registers are valid and capture
*                                      trigger arrives:
*                                      GT_TRUE  - the TOD Capture Value0 is overwritten.
*                                      GT_FALSE - the TOD Capture Value0 unchanged.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiCaptureOverwriteEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
);

/**
* @internal cpssPxPtpTaiCaptureOverwriteEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Capture Overwrite Enable
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) Capture Overwrite Enable.
*                                      When both TOD Capture registers are valid and capture
*                                      trigger arrives:
*                                      GT_TRUE  - the TOD Capture Value0 is overwritten.
*                                      GT_FALSE - the TOD Capture Value0 unchanged.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiCaptureOverwriteEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssPxPtpTaiClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Cycle.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] seconds                  -  part of cycle.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] nanoSeconds              - nano seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..0x3FFFFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiClockCycleSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          seconds,
    IN  GT_U32          nanoSeconds
);

/**
* @internal cpssPxPtpTaiClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Cycle.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] secondsPtr               - (pointer to) seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiClockCycleGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *secondsPtr,
    OUT GT_U32          *nanoSecondsPtr
);

/**
* @internal cpssPxPtpTaiClockModeSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] clockMode                - clock interface using mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiClockModeSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_PTP_TAI_CLOCK_MODE_ENT  clockMode
);

/**
* @internal cpssPxPtpTaiClockModeGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] clockModePtr             - (pointer to) clock interface using mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiClockModeGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    OUT CPSS_PX_PTP_TAI_CLOCK_MODE_ENT  *clockModePtr
);

/**
* @internal cpssPxPtpTaiExternalPulseWidthSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) External Pulse Width.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] extPulseWidth            - External Pulse Width.
*                                      width of the pulse in nanoseconds of generated output trigger.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on bad extPulseWidth
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiExternalPulseWidthSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          extPulseWidth
);

/**
* @internal cpssPxPtpTaiExternalPulseWidthGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) External Pulse Width
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] extPulseWidthPtr         -  (pointer to) External Pulse Width.
*                                      width of the pulse in nanoseconds of generated output trigger.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiExternalPulseWidthGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *extPulseWidthPtr
);

/**
* @internal cpssPxPtpTaiFractionalNanosecondDriftSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] fracNanoSecond           - (pointer to) fractional nano seconds drift.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiFractionalNanosecondDriftSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_32           fracNanoSecond
);

/**
* @internal cpssPxPtpTaiFractionalNanosecondDriftGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] fracNanoSecondPtr        - (pointer to) fractional nano seconds drift.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiFractionalNanosecondDriftGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_32           *fracNanoSecondPtr
);


/**
* @internal cpssPxPtpTaiGracefulStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Graceful Step.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] gracefulStep             - Graceful Step in nanoseconds.
*                                      (APPLICABLE RANGES: 0..31.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on wrong gracefulStep
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiGracefulStepSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          gracefulStep
);

/**
* @internal cpssPxPtpTaiGracefulStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Graceful Step.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] gracefulStepPtr          - (pointer to) Graceful Step in nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiGracefulStepGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *gracefulStepPtr
);

/**
* @internal cpssPxPtpTaiIncomingClockCounterEnableSet function
* @endinternal
*
* @brief   Set enabled state of TAI (Time Application Interface) Incoming Clock
*         Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -  counting.
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiIncomingClockCounterEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
);

/**
* @internal cpssPxPtpTaiIncomingClockCounterEnableGet function
* @endinternal
*
* @brief   Get enabled state of TAI (Time Application Interface) Incoming Clock
*         Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)enable counting.
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiIncomingClockCounterEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssPxPtpTaiIncomingClockCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Clock Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to)value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiIncomingClockCounterGet
(
    IN  GT_U8                                 devNum,
    OUT GT_U32                                *valuePtr
);

/**
* @internal cpssPxPtpTaiIncomingTriggerCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Trigger Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to)value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiIncomingTriggerCounterGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *valuePtr
);

/**
* @internal cpssPxPtpTaiInputClockSelectSet function
* @endinternal
*
* @brief   Configures input TAI clock's selection.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] clockSelect              - input clock selection mode.
* @param[in] clockFrequency           - Frequency of the PTP PLL clock (default is 25).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiInputClockSelectSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT    clockSelect,
    IN  CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT      clockFrequency
);

/**
* @internal cpssPxPtpTaiInputClockSelectGet function
* @endinternal
*
* @brief   Get input TAI clock's selection.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] clockSelectPtr           - (pointer to) input clock selection mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiInputClockSelectGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    OUT CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT    *clockSelectPtr
);

/**
* @internal cpssPxPtpTaiInputTriggersCountEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Count Input Triggers Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - Count Input Triggers Enable.
*                                      GT_TRUE  - each incoming trigger is counted.
*                                      GT_FALSE - incoming triggers not counted.
*                                      see cpssPxPtpTaiIncomingTriggerCounterSet/Get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiInputTriggersCountEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
);

/**
* @internal cpssPxPtpTaiInputTriggersCountEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Count Input Triggers Enable
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) Count Input Triggers Enable.
*                                      GT_TRUE  - each incoming trigger is counted.
*                                      GT_FALSE - incoming triggers not counted.
*                                      see cpssPxPtpTaiIncomingTriggerCounterSet/Get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiInputTriggersCountEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssPxPtpTaiOutputTriggerEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -   output trigger generation.
*                                      GT_TRUE  - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note use cpssPxPtpTaiTodSet with
*       CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E and
*       the time to trigger generation.
*
*/
GT_STATUS cpssPxPtpTaiOutputTriggerEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
);

/**
* @internal cpssPxPtpTaiOutputTriggerEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enable output trigger generation.
*                                      GT_TRUE  - enable GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiOutputTriggerEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssPxPtpTaiPClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PClock Cycle.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] nanoSeconds              - nano seconds cycle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiPClockCycleSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          nanoSeconds
);

/**
* @internal cpssPxPtpTaiPClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PClock Cycle.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds cycle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiPClockCycleGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *nanoSecondsPtr
);

/**
* @internal cpssPxPtpTaiPClockDriftAdjustEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - PTP PClock Drift Adjust Enable.
*                                      GT_TRUE - the PClk is affected by the <Drift Adjustment> or not.
*                                      GT_FALSE - the PClk reflects the frequency of the free running TOD,
*                                      without any frequency adjustments.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiPClockDriftAdjustEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
);

/**
* @internal cpssPxPtpTaiPtpPClockDriftAdjustEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) PTP PClock Drift Adjust Enable.
*                                      GT_TRUE - the PClk is affected by the <Drift Adjustment> or not.
*                                      GT_FALSE - the PClk reflects the frequency of the free running TOD,
*                                      without any frequency adjustments.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiPtpPClockDriftAdjustEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssPxPtpTaiTodCaptureStatusGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Capture Status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] captureIndex             - Capture Index.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] validPtr                 - (pointer to)Capture is valid.
*                                      GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiTodCaptureStatusGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          captureIndex,
    OUT GT_BOOL         *validPtr
);

/**
* @internal cpssPxPtpTaiTodSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD values.
*         The TOD will be updated by triggering an appropriate function.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] todValueType             - type of TOD value.
*                                      Valid types are
*                                      CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E,
*                                      CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E,
*                                      CPSS_PX_PTP_TAI_TOD_TYPE_LOAD_VALUE_E.
* @param[in] todValuePtr              - (pointer to) TOD value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API does not activate any triggers, other APIs does it
*
*/
GT_STATUS cpssPxPtpTaiTodSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PTP_TAI_TOD_TYPE_ENT        todValueType,
    IN  CPSS_PX_PTP_TAI_TOD_COUNT_STC       *todValuePtr
);

/**
* @internal cpssPxPtpTaiTodGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD values.
*         The TOD was captured by triggering an appropriate function.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] todValueType             - type of TOD value.
*
* @param[out] todValuePtr              - (pointer to) TOD value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiTodGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_PTP_TAI_TOD_TYPE_ENT    todValueType,
    OUT CPSS_PX_PTP_TAI_TOD_COUNT_STC   *todValuePtr
);

/**
* @internal cpssPxPtpTaiTodStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiTodStepSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_PTP_TAI_TOD_STEP_STC    *todStepPtr
);

/**
* @internal cpssPxPtpTaiTodStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiTodStepGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    OUT CPSS_PX_PTP_TAI_TOD_STEP_STC    *todStepPtr
);

/**
* @internal cpssPxPtpTaiTodUpdateCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Update Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to)TOD Update Counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Counter is Clear On Read. Value incremented at each
*       TOD Update/Increment/Decrement function.
*
*/
GT_STATUS cpssPxPtpTaiTodUpdateCounterGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *valuePtr
);

/**
* @internal cpssPxPtpTaiTodCounterFunctionSet function
* @endinternal
*
* @brief   The function configures type of TOD counter action that will be
*         performed once triggered by cpssPxPtpTaiTodCounterFunctionTriggerSet()
*         API or by pulse from an external interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] function                 - One of the four possible TOD counter functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiTodCounterFunctionSet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT function
);

/**
* @internal cpssPxPtpTaiTodCounterFunctionGet function
* @endinternal
*
* @brief   Get type of TOD counter action that will be performed once triggered by
*         cpssPxPtpTaiTodCounterFunctionTriggerSet() API or by pulse from an
*         external interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] functionPtr              - (pointer to) One of the four possible TOD counter
*                                      functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiTodCounterFunctionGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    OUT CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT *functionPtr
);

/**
* @internal cpssPxPtpTaiTodCounterFunctionTriggerSet function
* @endinternal
*
* @brief   Trigger TOD (Time of Day) counter function accorging to the function set
*         by cpssPxPtpTaiTodCounterFunctionSet().
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - when previous TOD triggered action is not finished yet
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiTodCounterFunctionTriggerSet
(
    IN GT_SW_DEV_NUM    devNum
);

/**
* @internal cpssPxPtpTaiTodCounterFunctionTriggerGet function
* @endinternal
*
* @brief   Get status of trigger TOD (Time of Day) counter function.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - The trigger is on, the TOD
*                                      function is not finished.
*                                      GT_FALSE - The trigger is off, the TOD function is
*                                      finished.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiTodCounterFunctionTriggerGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssPxPtpTsuControlSet function
* @endinternal
*
* @brief   Set TSU (Time Stamp Unit) Control.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] controlPtr               - (pointer to) control structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTsuControlSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_PTP_TSU_CONTROL_STC *controlPtr
);

/**
* @internal cpssPxPtpTsuControlGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) Control.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] controlPtr               - (pointer to) control structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTsuControlGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PTP_TSU_CONTROL_STC *controlPtr
);

/**
* @internal cpssPxPtpTsuCountersClear function
* @endinternal
*
* @brief   Clear All TSU (Time Stamp Unit) Packet counters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTsuCountersClear
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);

/**
* @internal cpssPxPtpTsuPacketCouterGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) packet counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] counterType              - counter type (see enum)
*
* @param[out] valuePtr                 - (pointer to) counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTsuPacketCouterGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT counterType,
    OUT GT_U32                                  *valuePtr
);

/**
* @internal cpssPxPtpPortTypeKeySet function
* @endinternal
*
* @brief   Set port PTP type key info.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] portKeyPtr               - (pointer to) port PTP type key.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpPortTypeKeySet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_PX_PTP_TYPE_KEY_STC *portKeyPtr
);

/**
* @internal cpssPxPtpPortTypeKeyGet function
* @endinternal
*
* @brief   Get port PTP type key info.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] portKeyPtr               - (pointer to) port PTP type key.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpPortTypeKeyGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    OUT CPSS_PX_PTP_TYPE_KEY_STC *portKeyPtr
);

/**
* @internal cpssPxPtpTypeKeyEntrySet function
* @endinternal
*
* @brief   Set PTP type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] ptpTypeIndex             - index of the PTP type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] keyDataPtr               - (pointer to) PTP type key data.
* @param[in] keyMaskPtr               - (pointer to) PTP type key mask.
*                                      The PTP type key mask is AND styled one.
*                                      Mask bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM like lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM like lookup.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTypeKeyEntrySet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PTP_TYPE_INDEX                  ptpTypeIndex,
    IN  CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         *keyDataPtr,
    IN  CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         *keyMaskPtr
);

/**
* @internal cpssPxPtpTypeKeyEntryGet function
* @endinternal
*
* @brief   Get PTP type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] ptpTypeIndex             - index of the PTP type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] keyDataPtr               - (pointer to) PTP type key data.
* @param[out] keyMaskPtr               - (pointer to) PTP type key mask.
*                                      The PTP type key mask is AND styled one.
*                                      Mask bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM like lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM like lookup.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTypeKeyEntryGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PTP_TYPE_INDEX                  ptpTypeIndex,
    OUT CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         *keyDataPtr,
    OUT CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         *keyMaskPtr
);

/**
* @internal cpssPxPtpTypeKeyEntryEnableSet function
* @endinternal
*
* @brief   Enable/disable PTP type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] ptpTypeIndex             - index of the PTP type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] enable                   - Enable or disable PTP type key table entry
*                                      GT_TRUE - enable PTP type key table entry,
*                                      GT_FALSE - disable PTP type key table entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTypeKeyEntryEnableSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PTP_TYPE_INDEX                      ptpTypeIndex,
    IN  GT_BOOL                                     enable
);

/**
* @internal cpssPxPtpTypeKeyEntryEnableGet function
* @endinternal
*
* @brief   Get enabled state of PTP type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] ptpTypeIndex             - index of the PTP type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] enablePtr                - (pointer to) Enable or disable PTP type key table entry
*                                      GT_TRUE - enable PTP type key table entry,
*                                      GT_FALSE - disable PTP type key table entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTypeKeyEntryEnableGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PTP_TYPE_INDEX                      ptpTypeIndex,
    OUT GT_BOOL                                     *enablePtr
);

/**
* @internal cpssPxPtpTaiPClockOutputEnableSet function
* @endinternal
*
* @brief   Set PTP PCLK OUT as clock source for RCVR_CLOCK_OUT[1] pin enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   -  or disable PTP PCLK OUT clock source for
*                                      RCVR_CLOCK_OUT[1] pin:
*                                      GT_TRUE  - enable PTP PCLK OUT as clock source for
*                                      RCVR_CLOCK_OUT[1] pin.
*                                      GT_FALSE - disable PTP PCLK OUT as clock source for
*                                      RCVR_CLOCK_OUT[1] pin.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiPClockOutputEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_BOOL       enable
);

/**
* @internal cpssPxPtpTaiPClockOutputEnableGet function
* @endinternal
*
* @brief   Get PTP PCLK OUT as clock source for RCVR_CLOCK_OUT[1] pin enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] enablePtr                - (pointer to) PTP PCLK OUT as clock source for
*                                      RCVR_CLOCK_OUT[1] pin enabled state.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiPClockOutputEnableGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT GT_BOOL       *enablePtr
);

/**
* @internal cpssPxPtpTaiPhaseUpdateEnableSet function
* @endinternal
*
* @brief   Set configuration of the PPS rising edge phase enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - configuration of the PPS rising edge phase enable.
*                                      GT_TRUE  - configuration of the PPS rising edge phase
* @param[in] enable- configuration of the PPS rising edge phase enable.
*                                      GT_FALSE - configuration of the PPS rising edge phase
*                                      disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiPhaseUpdateEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_BOOL       enable
);

/**
* @internal cpssPxPtpTaiPhaseUpdateEnableGet function
* @endinternal
*
* @brief   Get configuration of the PPS rising edge phase enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) configuration of the PPS rising edge
*                                      phase enabled state.
*                                      GT_TRUE  - configuration of the PPS rising edge phase
*                                      enabled.
*                                      GT_FALSE - configuration of the PPS rising edge phase
*                                      disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiPhaseUpdateEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssPxPtpTaiPulseInModeSet function
* @endinternal
*
* @brief  Set TAI PTP pulse IN mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number.
* @param[in] pulseMode             - TAI PulseIN signal mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssPxPtpTaiPulseInModeSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT       pulseMode
);

/**
* @internal cpssPxPtpTaiPulseInModeGet function
* @endinternal
*
* @brief  Get TAI PTP pulse IN mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number.
* @param[out] pulseMode            - (pointer to) TAI PulseIN signal mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_STATE             - on bad state
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssPxPtpTaiPulseInModeGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    OUT CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT       *pulseModePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxPtp */

