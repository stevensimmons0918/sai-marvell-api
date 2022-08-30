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
* @file prvCpssPxErrataMng.h
*
* @brief PX manager for the Errata issues.
*
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssPxErrataMng_h
#define __prvCpssPxErrataMng_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/* number GT_U32 bmps needed to hold the known FEr errata bmp */
#define PRV_CPSS_PX_ERRATA_BMP_MAX_NUM_CNS \
                (1 + (((PRV_CPSS_PX_ERRATA_MAX_NUM_E)+1) >> 5))

/* get if the erratum need to be WA by CPSS
    devNum - the device id of the PX device
    FErNum -- id of the erratum
*/
#define PRV_CPSS_PX_ERRATA_GET_MAC(devNum,FErNum) \
  (((PRV_CPSS_PX_PP_MAC(devNum))->errata.FErArray[(FErNum)>>5] &(1<<((FErNum)&0x1f)))? \
      GT_TRUE : GT_FALSE)

/* set that the erratum need to be WA by CPSS
    devNum - the device id of the PX device
    FErNum -- 1 based number
*/
#define PRV_CPSS_PX_ERRATA_SET_MAC(devNum,FErNum) \
    (PRV_CPSS_PX_PP_MAC(devNum))->errata.FErArray[(FErNum)>>5] |= (1<<((FErNum)&0x1f))

/* set that the erratum NOT need to be WA by CPSS
    devNum - the device id of the PX device
    FErNum -- 1 based number
*/
#define PRV_CPSS_PX_ERRATA_CLEAR_MAC(devNum,FErNum) \
    (PRV_CPSS_PX_PP_MAC(devNum))->errata.FErArray[(FErNum)>>5] &= ~(1<<((FErNum)&0x1f))

/**
* @enum PRV_CPSS_PX_ERRATA_ENT
 *
 * @brief enumerator for the Erratum that partial/fully handled in the CPSS
 * PX code.
*/
typedef enum{

    /** @brief Disabling the Egress Rate Shaper under traffic may hang its
     *  relevant transmit queue.
     *  To disable the Egress Rate Shaper follow the following procedure:
     *  1. Set MaxBucketSize to 0xFFF, Tokens to 0xFFF, SlowRateEn to 0.
     *  2. Set TokenBucketEn to 0.
     *  (FEr#47)
     */
    PRV_CPSS_PX_TOKEN_BUCKET_RATE_SHAPING_DISABLE_WA_E,

    /** @brief all not used mac's LED port number shall be set to 0x3F
     *  (Errata FE-3763116 )
     */
    PRV_CPSS_PX_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E,

    /** @brief Read access to CG MAC registers is masked by a MG
     *  terminator
     *  (Errata FE-8132779)
     */
    PRV_CPSS_PX_RM_MG_TERMINATOR_BYPASS_ENABLE_WA_E,

    /** @brief QCN does not drop CN packets when any of the following clients are not ready:
     *  - inc interface
     *  - write back fifo full
     *  - desc dup interface
     *  (Errata FE-3507014, FE-5440838, FE-809639, FE-3368183)
     *  The device does not identify the CN-Tag and does
     *  not trigger QCN. QCN should be triggered regardless of
     *  whether the CN-tag exists or not.
     *  (Errata RM-171573)
     */
    PRV_CPSS_PX_RM_TXQ_QCN_GLOBAL_MISS_CONFIG_WA_E,

    /** @brief The value of the AVS set point should be changed from
     *  its default value in systems that work with core_clk
     *  smaller than or equal to 450MHz. The default set point
     *  is 1.03V which fits systems with core_clk greater than
     *  450MHz, while the required set point for systems with
     *  core_clk<=450MHz is 0.98V.
     *  (Errata RM-3285928)
     */
    PRV_CPSS_PX_RM_AVS_VDD_LIMITS_WA_E,

    /** @brief Time Application Interface (TAI) minimal TOD drift effect is dependent on
     *  the drift exception threshold.
     *  Description :
     *  The device incorporates a TAI unit, which keeps the time of day.
     *  This unit is used for accurate timestamping and for
     *  the Precision Time Protocol (PTP). The TAI unit can receive an incoming 1
     *  Pulse Per Second (1PPS) through its input
     *  PTP_CLK_IN interface. The device can be configured to mode ClockReceptionAdv;
     *  in this mode the incoming 1PPS is used for both phase alignment and frequency alignment.
     *  In this mode, the <Minimal Drift> configuration determines
     *  the minimal time drift that causes the frequency to be adjusted when an
     *  incoming 1PPS rising edge is received.
     *  However, due to this erratum, the minimal TOD drift effect is dependent
     *  on the drift exception threshold and not only
     *  on minimal drift. The frequency is adjusted in ClockReceptionAdv mode
     *  when the drift is greater than or equal to Min(<Minimal Drift>,
     *  <Drift Exception Threshold>).
     *  WA - Configure the <Drift Exception Threshold> field bits in the
     *  Drift Threshold Configuration Low register to be greater than the
     *  <Minimal Drift> field value in the same register.
     *  (FE-8248885)
     */
    PRV_CPSS_PX_TAI_MINIMAL_TOD_DRIFT_CONSTRAIN_WA_E,

    /** @brief  RM-7969777 WA implemented for for BobK and above.
     *  Frequency adjust time window should be internalized and configured to 0x1 only (default is 0x0!)
     */
    PRV_CPSS_PX_TAI_FREQUENCY_ADJUST_TIME_WINDOW_INIT_VAL_WA_E,

    /** @brief An internal configuration, in the <Send On Change Only> field in the FCA Control register,
     *  determines whether FCA works in normal mode (0x1) and
     *  issues flow control packets only upon status changes, or
     *  in debug mode (0x0) and issues packets upon each
     *  arriving trigger regardless of data-base status.
     *  However, due to this erratum, only normal mode (0x1)
     *  works properly.
     *  (Errata RM-2320268)
     */
    PRV_CPSS_PX_RM_GOP_FCA_MISS_CONFIG_WA_E,

    /** @brief In order to avoid the "slow" port from getting to
     *  credits starvation, it must be configured with at least
     *  the number of 10G Port credits (although it needs less
     *  credits according to the port speed)
     *  (Jira TXQ-1972)
     *  ==============================
     *  ======  must be last ======
     *  ==============================
     */
    PRV_CPSS_PX_SLOW_PORT_TXQ_CRDITS_WA_E,

    /** the MAX value to be used for array sizes */
    PRV_CPSS_PX_ERRATA_MAX_NUM_E

} PRV_CPSS_PX_ERRATA_ENT;

/**
* @struct PRV_CPSS_PX_ERRATA_STC
 *
 * @brief A structure to hold all Errata info that CPSS need to fix
 * (workaround) for the PP .
*/
typedef struct{

    GT_U32 FErArray[PRV_CPSS_PX_ERRATA_BMP_MAX_NUM_CNS];

} PRV_CPSS_PX_ERRATA_STC;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxErrataMng_h */

