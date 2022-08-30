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
* mvComPhyH28nmIf.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 15 $
*
*******************************************************************************/

#ifndef __mvAvagoIf_H
#define __mvAvagoIf_H

/* General H Files */
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>

#ifdef __cplusplus
extern "C" {
#endif


/************************* Globals *******************************************************/
/* define for HAL functions and Avago_serdes_hal_type_t */
#define AAPL_ENABLE_INTERNAL_FUNCTIONS

#define RAVEN_NUM_OF_DEVICES            6

#define BOBCAT3_MAX_AVAGO_SERDES_NUMBER 75
#define BOBK_MAX_AVAGO_SERDES_NUMBER    40
#define ALDRIN_MAX_AVAGO_SERDES_NUMBER  40
#define PIPE_MAX_AVAGO_SERDES_NUMBER    16
#define ALDRIN2_MAX_AVAGO_SERDES_NUMBER 73
#define RAVEN_MAX_AVAGO_SERDES_NUMBER   16

/*
    Avago DFE table values: (table param - 0x5BXX, param data - 0xXX):
    data_bit[0]- high - disabling automatic one-shot pCal after iCal
    data_bit[3]- high - disable tap1 from the 14 DFE taps
    we using bit[3] for disabling DFE tap1 before running iCal
    and enabling it back after iCal finish successfully to allow
    tap1 for DFE. Tap can still be changed in pCal (DFE) tunning
*/
#define TAP1_AVG_DISABLE                0x5B01
#define TAP1_AVG_ENABLE                 0x5B09
#define SBC_CLK_DIV_FALCON_UNIT0_BASE_ADDRESS         0x0067F828
#define SBC_TSEN_CLK_DIV_FALCON_UNIT0_BASE_ADDRESS    0x00644804

#define FALCON_MAX_AVAGO_SERDES_NUMBER  (256+16)

/* timeout for blocking rx training operation */
#define MV_AVAGO_TRAINING_TIMEOUT       6000

#ifndef MAX_AVAGO_SERDES_NUMBER
#ifdef ALDRIN_DEV_SUPPORT
#define MAX_AVAGO_SERDES_NUMBER         ALDRIN_MAX_AVAGO_SERDES_NUMBER
#elif defined(BOBK_DEV_SUPPORT)
#define MAX_AVAGO_SERDES_NUMBER         BOBK_MAX_AVAGO_SERDES_NUMBER
#elif defined(BC3_DEV_SUPPORT)
#define MAX_AVAGO_SERDES_NUMBER         BOBCAT3_MAX_AVAGO_SERDES_NUMBER
#elif defined(PIPE_DEV_SUPPORT)
#define MAX_AVAGO_SERDES_NUMBER         PIPE_MAX_AVAGO_SERDES_NUMBER
#elif defined(ALDRIN2_DEV_SUPPORT)
#define MAX_AVAGO_SERDES_NUMBER         ALDRIN2_MAX_AVAGO_SERDES_NUMBER
#elif defined(RAVEN_DEV_SUPPORT)
#define MAX_AVAGO_SERDES_NUMBER         RAVEN_MAX_AVAGO_SERDES_NUMBER
#else /* in CPSS appDemo no xxx_DEV_SUPPORT flags lets take maximum for now,
host CPU memory can live with it */
#define MAX_AVAGO_SERDES_NUMBER         FALCON_MAX_AVAGO_SERDES_NUMBER
#endif
#endif

#ifdef ASIC_SIMULATION
#undef  MAX_AVAGO_SERDES_NUMBER
#define MAX_AVAGO_SERDES_NUMBER         FALCON_MAX_AVAGO_SERDES_NUMBER
#endif

#define HWS_AVAGO_16NM_TXRX_PARAMS_NUM_OF_PROFILES 3

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (FALCON_DEV_SUPPORT)
#define RAVEN_MAX_AVAGO_CHIP_NUMBER (16)
#else
#define RAVEN_MAX_AVAGO_CHIP_NUMBER (1)
#endif

/**
* @enum MV_HWS_RX_TRAINING_MODES
* @endinternal
*
* @brief   RX training modes
*
*/

typedef enum
{
    IGNORE_RX_TRAINING,
    ONE_SHOT_DFE_TUNING,                /* Single time iCal */
    START_CONTINUE_ADAPTIVE_TUNING,     /* adaptive pCal */
    STOP_CONTINUE_ADAPTIVE_TUNING,
    ONE_SHOT_DFE_VSR_TUNING             /* iCal-VSR mode, Bypass DFE TAP tuning */

}MV_HWS_RX_TRAINING_MODES;

/**
* @enum MV_HWS_TX_TRAINING_MODES
* @endinternal
*
* @brief   TX training modes
*
*/

typedef enum
{
    IGNORE_TX_TRAINING,
    START_TRAINING,
    STOP_TRAINING

}MV_HWS_TX_TRAINING_MODES;

/**
* @enum MV_HWS_PORT_AUTO_TUNE_MODE
 *
 * @brief Avago Serdes 16nm HAL types
*/
typedef enum
{
    HWS_SERDES_GLOBAL_TUNE_PARAMS,     /* Load Clock vernier settings into HW */
    HWS_SERDES_M4_TUNE_PARAMS,         /* Read/set M4 Tune Parameters */
    HWS_SERDES_RXCLK_SELECT,           /* Read/set vernier settings */
    HWS_SERDES_TXEQ_FFE,               /* Read/set Tx Eq settings */
    HWS_SERDES_RXEQ_CTLE,              /* Read/set Rx CTLE settings */
    HWS_SERDES_RXEQ_DFE,               /* Read/set Rx DFE settings */
    HWS_SERDES_RXEQ_FFE,               /* Read/set Rx FFE settings */
    HWS_SERDES_DATA_CHANNEL_INPUTS,    /* Read/set data DAC offset from mid point */
    HWS_SERDES_DATA_CHANNEL_OFFSET,
    HWS_SERDES_DATA_CHANNEL_CAL,       /* Read/set data VOS calibrated mid point */
    HWS_SERDES_TEST_CHANNEL_INPUTS,    /* Read/set test DAC offset from mid point */
    HWS_SERDES_TEST_CHANNEL_OFFSET,
    HWS_SERDES_TEST_CHANNEL_CAL,       /* Read/set test VOS calibrated mid point */
    HWS_SERDES_PAM4_EYE,               /* Read PAM4 eye parameters */
    HWS_SERDES_PAM4_LEVELS,            /* Read PAM4 eye middles */
    HWS_SERDES_GRADIENT_INPUTS,        /* Read gradient inputs */
    HWS_SERDES_DATA_CHANNEL_CAL_DELTA, /* Read deltas between data and test channels */
    HWS_SERDES_CLK_VERNIER_CAL,        /* Read Rx clock vernier cals */
    HWS_SERDES_TEMPERATURE,            /* Read/set temperature */
    HWS_SERDES_ENVELOPE_CONFIG,        /* Read/set link train envelope config */
    HWS_SERDES_KR_TRAINING_CONFIG,
    HWS_SERDES_PLL_GAIN
}MV_HWS_AVAGO_SERDES_HAL_TYPES;

/**
* @enum MV_HWS_AVAGO_SERDES_HAL_FUNC_TYPES
 *
 * @brief Avago Serdes 16nm HAL function types
*/
typedef enum
{
    HWS_SERDES_HAL_RXCLK_VERNIER_APPLY,  /* Load Clock vernier settings into HW */
    HWS_SERDES_HAL_TXEQ_FFE_APPLY,       /* Load Tx Eq settings into HW */
    HWS_SERDES_HAL_DATA_CHANNEL_APPLY,   /* Load Data channel VOS settings into HW */
    HWS_SERDES_HAL_TEST_CHANNEL_APPLY,   /* Load Test channel VOS settings into HW */
    HWS_SERDES_HAL_RXEQ_FFE_APPLY,       /* Load RxFFE settings into HW */
    HWS_SERDES_HAL_RXEQ_CTLE_APPLY,      /* Load CTLE  settings into HW */
    HWS_SERDES_HAL_RXEQ_DFE_APPLY        /* Load DFE   settings into HW */

}MV_HWS_AVAGO_SERDES_HAL_FUNC_TYPES;

/**
* @struct MV_HWS_AVG_BLOCK
 *
 * @brief values to read from AVG block
 * to apply VOS corrected values
*/
typedef struct{

    /** @brief decides how the data is arranged.
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     */
    GT_U8 revision;

    /** @brief the number of AVG block lines.
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     */
    GT_U8 avgBlockLines;

    /** @brief minimum VOS value.
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     */
    GT_U8 minVal;

    /** @brief base VOS value.
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     */
    GT_U8 baseVal;

    /** @brief number of 7 VOS values per serdes
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     */
    GT_U8 bitsPerSds;

} MV_HWS_AVG_BLOCK;

/**
* @internal mvHwsAvagoEthDriverInit function
* @endinternal
*
* @brief   Initialize Avago related configurations
*
* @param[in] devNum                  - Device Number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoEthDriverInit
(
   IN GT_U8 chipIndex
);

/**
* @internal avagoSerdesAacsServerExec function
* @endinternal
*
* @brief   Initialize the Avago AACS Server on device
*
* @param[in] startDevNum             - First device number to
*                                       initialized
* @param[in] endDevNum               - Last  device number to
*                                       initialized
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int avagoSerdesAacsServerExec
(
    IN unsigned char   startDevNum,
    IN unsigned char   endDevNum
);

/**
* @internal mvHwsAvagoIfInit function
* @endinternal
*
* @brief   Init Avago configuration sequences and IF functions.
*
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - array for function registration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoIfInit
(
    IN GT_U8                   devNum,
    IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray
);

/**
* @internal mvHwsAvago16nmIfInit function
* @endinternal
*
* @param[in] devNum                   - system device number
* @param[in] ravenDevBmp              - raven devices supported
* @param[in] funcPtrArray             - array for function registration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvago16nmIfInit
(
    IN GT_U8  devNum,
    IN GT_U32 ravenDevBmp,
    IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray
);


/**
* @internal mvHwsAvagoSerdesVosParamsGet function
* @endinternal
*
* @brief   Get the VOS Override parameters from the local DB.
*
* @param[in] devNum                   - system device number
*
* @param[out] vosParamsPtr            - serdes vos parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesVosParamsGet
(
    IN unsigned char  devNum,
    OUT unsigned long  *vosParamsPtr
);

/**
* @internal mvHwsAvagoIfClose function
* @endinternal
*
* @brief   Release all system resources allocated by Serdes IF functions.

* @param[in] devNum                   - system device number
*/
void mvHwsAvagoIfClose
(
    IN GT_U8 devNum
);

#ifndef CO_CPU_RUN
/**
* @internal mvHwsAvagoSerdesTestGen function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit ("Other" means HW default - K28.5
*                                      [alternate running disparity])
* @param[in] mode                     - test  or normal
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTestGen
(
    IN GT_U8                     devNum,
    IN GT_UOPT                   portGroup,
    IN GT_UOPT                   serdesNum,
    IN MV_HWS_SERDES_TX_PATTERN  txPattern,
    IN MV_HWS_SERDES_TEST_GEN_MODE mode
);

/**
* @internal mvHwsAvagoSerdesTestGenGet function
* @endinternal
*
* @brief   Get configuration of the Serdes test generator/checker.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] txPatternPtr             - pattern to transmit ("Other" means any mode not
*                                      included explicitly in MV_HWS_SERDES_TX_PATTERN type)
* @param[out] modePtr                  - test mode or normal
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - unexpected pattern
* @retval GT_FAIL                  - HW error
*/
GT_STATUS mvHwsAvagoSerdesTestGenGet
(
    IN GT_U8                    devNum,
    IN GT_UOPT                  portGroup,
    IN GT_UOPT                  serdesNum,
    OUT MV_HWS_SERDES_TX_PATTERN  *txPatternPtr,
    OUT MV_HWS_SERDES_TEST_GEN_MODE *modePtr
);

/**
* @internal mvHwsAvagoSerdesTestGenStatus function
* @endinternal
*
* @brief   Read the tested pattern receive error counters and status.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit
* @param[in] counterAccumulateMode    - Enable/Disable reset the accumulation of error counters
*
* @param[out] status                   - test generator status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTestGenStatus
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroup,
    IN GT_U32                    serdesNum,
    IN MV_HWS_SERDES_TX_PATTERN  txPattern,
    IN GT_BOOL                   counterAccumulateMode,
    OUT MV_HWS_SERDES_TEST_GEN_STATUS *status
);

/**
* @internal mvHwsAvagoSerdesAuxCounterStart function
* @endinternal
*
* @brief   Start aux_counters.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAuxCounterStart
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          serdesNum
);
#endif

#ifndef CO_CPU_RUN
/**
* @internal mvHwsAvagoSerdesPolarityConfig function
* @endinternal
*
* @brief   Per serdes invert the Tx or Rx.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] invertTx                 - invert TX polarity (GT_TRUE - invert, GT_FALSE - don't)
* @param[in] invertRx                 - invert RX polarity (GT_TRUE - invert, GT_FALSE - don't)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPolarityConfig
(
    IN GT_U8   devNum,
    IN GT_UOPT portGroup,
    IN GT_UOPT serdesNum,
    IN GT_BOOL invertTx,
    IN GT_BOOL invertRx
);

/**
* @internal mvHwsAvagoSerdesPolarityConfigGet function
* @endinternal
*
* @brief   Per SERDES invert the Tx or Rx.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] invertTx                 - invert TX polarity (GT_TRUE - invert, GT_FALSE - don't)
* @param[out] invertRx                 - invert RX polarity (GT_TRUE - invert, GT_FALSE - don't)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPolarityConfigGet
(
    IN GT_U8             devNum,
    IN GT_UOPT           portGroup,
    IN GT_UOPT           serdesNum,
    OUT GT_BOOL          *invertTx,
    OUT GT_BOOL          *invertRx
);
#endif

/**
* @internal mvHwsAvagoSerdesReset function
* @endinternal
*
* @brief   Per SERDES Clear the serdes registers (back to defaults.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] analogReset              - Analog Reset (On/Off)
* @param[in] digitalReset             - digital Reset (On/Off)
* @param[in] syncEReset               - SyncE Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesReset
(
    IN GT_U8      devNum,
    IN GT_UOPT    portGroup,
    IN GT_UOPT    serdesNum,
    IN GT_BOOL    analogReset,
    IN GT_BOOL    digitalReset,
    IN GT_BOOL    syncEReset
);

/**
* @internal mvHwsAvagoSerdesCoreReset function
* @endinternal
*
* @brief   Run core reset / unreset on current SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] coreReset                - core Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesCoreReset
(
    IN GT_U8           devNum,
    IN GT_UOPT         portGroup,
    IN GT_UOPT         serdesNum,
    IN MV_HWS_RESET    coreReset
);

/**
* @internal mvHwsAvagoSerdesDigitalReset function
* @endinternal
*
* @brief   Run digital RESET/UNRESET (RF) on current SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] digitalReset             - digital Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesDigitalReset
(
    IN GT_U8           devNum,
    IN GT_UOPT         portGroup,
    IN GT_UOPT         serdesNum,
    IN MV_HWS_RESET    digitalReset
);

/**
* @internal mvHwsAvagoSerdesPowerCtrl function
* @endinternal
*
* @brief   Power Up/Down Serdes number
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - Serdes number
* @param[in] powerUp   - True for PowerUP, False for PowerDown
* @param[in] serdesConfig - pointer to Serdes paramters:
*      refClock  - ref clock value
*      refClockSource - ref clock source (primary line or
*      secondary)
*      baudRate - Serdes speed
*      busWidth - Serdes bus modes: 10Bits/20Bits/40Bits
*      media - RXAUI or XAUI
*      encoding - Rx & Tx data encoding NRZ/PAM4
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvagoSerdesPowerCtrl
(
    IN GT_U8                       devNum,
    IN GT_UOPT                     portGroup,
    IN GT_UOPT                     serdesNum,
    IN GT_BOOL                     powerUp,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
);

/**
* @internal mvHwsAvagoSerdesAcTerminationCfg function
* @endinternal
*
* @brief   Configures AC termination on current serdes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] acTermEn                 - enable or disable AC termination
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAcTerminationCfg
(
   IN GT_U8                   devNum,
   IN GT_U32                  portGroup,
   IN GT_U32                  serdesNum,
   IN GT_BOOL                 acTermEn
);

/**
* @internal mvHwsAvagoSerdesArrayPowerCtrl function
* @endinternal
*
* @brief   Power Up/Down on list of SERDESes on device
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] numOfSer  - number of SERDESes to configure
* @param[in] serdesArr - collection of SERDESes to configure
* @param[in] powerUp   - True for PowerUP, False for PowerDown
* @param[in] serdesConfigPtr - pointer to Serdes params struct
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvagoSerdesArrayPowerCtrl
(
    IN GT_U8                       devNum,
    IN GT_UOPT                     portGroup,
    IN GT_UOPT                     numOfSer,
    IN GT_UOPT                     *serdesArr,
    IN GT_BOOL                     powerUp,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
);

/**
* @internal mvHwsAvagoSerdesArrayHaltDfeTune function
* @endinternal
*
* @brief   Halts DFE tune for safe SerDes power down
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - physical serdes number
*
* @param[out] numOfSer                 - number of serdes's
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesArrayHaltDfeTune
(
    IN GT_U8 devNum,
    IN GT_UOPT portGroup,
    IN GT_UOPT *serdesArr,
    OUT GT_UOPT numOfSer
);

#ifndef CO_CPU_RUN
/**
* @internal mvHwsAvagoSerdesBulkPowerCtrl function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @param[in] numOfSer                 - number of serdes
* @param[in] serdesArr                - collection of SERDESes
*                                       to configure
* @param[in] powerUp                  - power up
* @param[in] baudRate                 - current system baud rate
* @param[in] refClock                 - ref clock value
* @param[in] refClockSource           - ref clock source
*                                       (primary line or secondary)
* @param[in] media                    - RXAUI or XAUI
* @param[in] mode                     - 10BIT  (enable/disable)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesBulkPowerCtrl
(
    IN GT_U8                   numOfSer,
    IN MV_HWS_ELEMENTS_ARRAY   *serdesArr,
    IN GT_BOOL                 powerUp,
    IN MV_HWS_SERDES_SPEED     baudRate,
    IN MV_HWS_REF_CLOCK        refClock,
    IN MV_HWS_REF_CLOCK_SRC    refClockSource,
    IN MV_HWS_SERDES_MEDIA     media,
    IN MV_HWS_SERDES_BUS_WIDTH_ENT  mode
);
#endif

/**
* @internal mvHwsAvagoSerdesRxAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] rxTraining               - Rx Training (true/false)
*                                      txTraining - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesRxAutoTuneStart
(
    IN GT_U8   devNum,
    IN GT_UOPT portGroup,
    IN GT_UOPT serdesNum,
    IN GT_BOOL rxTraining
);

/**
* @internal mvHwsAvagoSerdesAutoTuneCfg function
* @endinternal
*
* @brief   Per SERDES configure parameters for TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      rxTraining - Rx Training (true/false)
* @param[in] rxTraining               - Rx Training (true/false)
* @param[in] txTraining               - Tx Training (true/false)
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneCfg
(
    IN GT_U8   devNum,
    IN GT_UOPT portGroup,
    IN GT_UOPT serdesNum,
    IN GT_BOOL rxTraining,
    IN GT_BOOL txTraining
);

/**
* @internal mvHwsAvagoSerdesTxAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      rxTraining - Rx Training (true/false)
* @param[in] txTraining               - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxAutoTuneStart
(
    IN GT_U8   devNum,
    IN GT_UOPT portGroup,
    IN GT_UOPT serdesNum,
    IN GT_BOOL txTraining
);

/**
* @internal mvHwsAvagoSerdesAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting.
*         This function calls to the mvHwsAvagoSerdesAutoTuneStartExt, which includes
*         all the functional options.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] rxTraining               - Rx Training (true (AVAGO_DFE_ICAL) /false)
* @param[in] txTraining               - Tx Training (true (AVAGO_PMD_TRAIN) /false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneStart
(
    IN GT_U8   devNum,
    IN GT_UOPT portGroup,
    IN GT_UOPT serdesNum,
    IN GT_BOOL rxTraining,
    IN GT_BOOL txTraining
);

/**
* @internal mvHwsAvagoSerdesManualTxConfig function
* @endinternal
*
* @brief   Per SERDES configure the TX parameters
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] xConfigPtr               - tx configure data
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualTxConfig
(
    IN GT_U8   devNum,
    IN GT_UOPT portGroup,
    IN GT_UOPT serdesNum,
    IN MV_HWS_SERDES_TX_CONFIG_DATA_UNT *txConfigPtr
);

/**
* @internal mvHwsAvagoSerdesManualTxConfigGet function
* @endinternal
*
* @brief   Per SERDES get the configure TX parameters: amplitude, 3 TAP Tx FIR.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] configParams             - Manual Tx params structures:
*                                      txAmp     - Tx Attenuator [0...31]
*                                      txAmpAdj  - not used in Avago serdes
*                                      emph0     - Post-Cursor: for BobK/Aldrin (IP_Rev=0xde) [-31...31],
*                                      for Bobcat3 (IP_Rev=0xcd) [0...31]
*                                      emph1     - Pre-Cursor:  for BobK/Aldrin (IP_Rev=0xde) [-31...31],
*                                      for Bobcat3 (IP_Rev=0xcd) [0...31]
*                                      slewRate  - Slew rate  [0...3], 0 is fastest
*                                      (the SlewRate parameter is not supported in Avago Serdes 28nm)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualTxConfigGet
(
    IN GT_U8                           devNum,
    IN GT_UOPT                         portGroup,
    IN GT_UOPT                         serdesNum,
    OUT MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *txConfigPtr
);

/**
* @internal mvHwsAvago16nmSerdesManualRxConfig function
* @endinternal
*
* @brief   Set the Serdes Manual Rx config
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] rxConfigPtr              - Rx config data
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvago16nmSerdesManualRxConfig
(
    IN GT_U8                                devNum,
    IN GT_UOPT                              portGroup,
    IN GT_UOPT                              serdesNum,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT    *rxConfigPtr
);

/**
* @internal mvHwsAvagoSerdesCdrLockStatusGet function
* @endinternal
*
* @brief   Reads and resets the signal_ok_deasserted signal.
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - physical serdes number
*
* @param[out]  enable   - (pointer to) enable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesCdrLockStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 *enable
);

/**
* @internal mvHwsSerdesEncodingTypeGet function
* @endinternal
*
* @brief   Retrieves the Tx and Rx line encoding values.
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - physical PCS number
* @param[in] serdesType               - serdes PCS type
*
* @param[out] txEncodingPtr           - NRZ/PAM4
* @param[out] rxEncodingPtr           - NRZ/PAM4
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEncodingTypeGet
(
    GT_U8                            devNum,
    GT_U32                           serdesNum,
    MV_HWS_SERDES_ENCODING_TYPE     *txEncodingPtr,
    MV_HWS_SERDES_ENCODING_TYPE     *rxEncodingPtr
);

/**
* @internal mvHwsAvagoSerdesRxDatapathConfigGet function
* @endinternal
*
* @brief   Retrieves the Rx data path configuration values
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - physical PCS number
*
* @param[out] rxDatapathConfigPtr     - (pointer to)struct:
*                                        polarityInvert (true/false)
*                                        grayEnable     (true/false)
*                                        precodeEnable  (true/false)
*                                        swizzleEnable  (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesRxDatapathConfigGet
(
    GT_U8                                  devNum,
    GT_U32                                 serdesNum,
    MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC   *rxDatapathConfigPtr
);

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
/**
* @internal mvHwsAvagoSerdesRxPllLockGet function
* @endinternal
*
* @brief   Returns whether or not the RX PLL is frequency locked
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] lockPtr                 - (pointer to) lock state
*                                        GT_TRUE =  locked
*                                        GT_FALSE = not locked
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesRxPllLockGet
(
    GT_U8                 devNum,
    GT_U32                serdesNum,
    GT_BOOL               *lockPtr
);
#endif /* #ifndef MV_HWS_REDUCED_BUILD_EXT_CM3 */

/**
* @internal mvHwsAvagoSerdesSignalLiveDetectGet function
* @endinternal
*
* @brief   Per SERDES get live indication is signal detected.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] signalDet                - TRUE if signal detected and FALSE otherwise.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalLiveDetectGet
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT GT_BOOL     *signalDet
);

/**
* @internal mvHwsAvagoSerdesSignalDetectGet function
* @endinternal
*
* @brief   Per SERDES get indication is signal detected.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] signalDet                - TRUE if signal detected and FALSE otherwise.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalDetectGet
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT GT_BOOL     *signalDet
);

/**
* @internal mvHwsAvagoPllStableGet function
* @endinternal
*
* @brief   Wait for stable PLL signal
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - SerDes number
*
* @param[out] pllStable                - PLL Stable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoPllStableGet
(
    IN GT_U8       evNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT GT_BOOL     *pllStable
);

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3


/**
* @internal mvHwsAvagoSerdesSignalOkChange function
* @endinternal
*
* @brief   Per SERDES get indication check CDR lock and Signal
*          Detect change.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] signalOkChange                - TRUE if signal
*       detected and FALSE otherwise.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalOkChange
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT GT_BOOL     *signalOkChange
);
#endif
/**
* @internal mvHwsAvagoSerdesRxSignalCheck function
* @endinternal
*
* @brief   Per SERDES check there is Rx Signal and indicate if Serdes is ready for Tuning or not
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] rxSignal                 - Serdes is ready for Tuning or not
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesRxSignalCheck
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    OUT MV_HWS_AUTO_TUNE_STATUS     *rxSignal
);

/**
* @internal mvHwsAvagoSerdesTxAutoTuneStatusShort function
* @endinternal
*
* @brief   Per SERDES check the Tx training status
*         This function is necessary for 802.3ap functionality
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] txStatus                - Tx tune status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxAutoTuneStatusShort
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    OUT MV_HWS_AUTO_TUNE_STATUS *txStatus
);

/**
* @internal mvHwsAvagoSerdesRxAutoTuneStatusShort function
* @endinternal
*
* @brief   Per SERDES check the Tx training status
*         This function is necessary for 802.3ap functionality
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] txStatus                - Tx tune status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesRxAutoTuneStatusShort
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    OUT MV_HWS_AUTO_TUNE_STATUS *rxStatus
);

/**
* @internal mvHwsAvagoSerdesAutoTuneStatus function
* @endinternal
*
* @brief   Per SERDES check the Rx & Tx training status
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] rxStatus                - Rx tune status
* @param[out] txStatus                - Tx tune status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneStatus
(
    IN GT_U8   devNum,
    IN GT_UOPT portGroup,
    IN GT_UOPT serdesNum,
    OUT MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    OUT MV_HWS_AUTO_TUNE_STATUS *txStatus
);

/**
* @internal mvHwsAvagoSerdesTxAutoTuneStop function
* @endinternal
*
* @brief   Per SERDES stop the TX training
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxAutoTuneStop
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum
);

/**
* @internal mvHwsAvagoSerdesAutoTuneResult function
* @endinternal
*
* @brief   Per SERDES return the adapted tuning results
*          Can be run after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[out] results                 - the adapted tuning
*                                       results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneResult
(
    IN  GT_U8                               devNum,
    IN  GT_UOPT                             portGroup,
    IN  GT_UOPT                             serdesNum,
    OUT MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *tuneResults
);

/**
* @internal mvHwsAvago16nmSerdesAutoTuneResult function
* @endinternal
*
* @brief    Per SERDES return the adapted tuning results
*           Can be run after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[out] results                 - the adapted tuning
*                                       results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvago16nmSerdesAutoTuneResult
(
    GT_U8                               devNum,
    GT_UOPT                             portGroup,
    GT_UOPT                             serdesNum,
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *tuneResults
);

#ifndef CO_CPU_RUN
/**
* @internal mvHwsAvagoSerdesPpmConfig function
* @endinternal
*
* @brief   Per SERDES add ability to add/sub PPM from main baud rate.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] ppmVal                   - PPM value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPpmConfig
(
    IN GT_U8             devNum,
    IN GT_UOPT           portGroup,
    IN GT_UOPT           serdesNum,
    IN MV_HWS_PPM_VALUE  ppmVal
);

/**
* @internal mvHwsAvagoSerdesPpmConfigGet function
* @endinternal
*
* @brief   Per SERDES add ability to add/sub PPM from main baud rate.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] ppmVal                - PPL value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPpmConfigGet
(
    IN GT_U8              devNum,
    IN GT_UOPT            portGroup,
    IN GT_UOPT            serdesNum,
    OUT MV_HWS_PPM_VALUE   *ppmVal
);

/**
* @internal mvHwsAvagoSerdesAdaptPpm function
* @endinternal
*
* @brief   Set the PPM compensation on Serdes after Rx/TRX training in 10G and above.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serder number
* @param[in] configPpm                - senable/disable the PPM
*                                       configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAdaptPpm
(
    IN GT_U8              devNum,
    IN GT_UOPT            portGroup,
    IN GT_UOPT            serdesNum,
    IN GT_BOOL            configPpm
);

/**
* @internal mvHwsAvagoSerdesDfeConfig function
* @endinternal
*
* @brief   Per SERDES configure the DFE parameters.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] dfeMode                  - Enable/disable/freeze/Force
* @param[in] dfeCfg                   - array of dfe configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesDfeConfig
(
    IN GT_U8             devNum,
    IN GT_UOPT           portGroup,
    IN GT_UOPT           serdesNum,
    IN MV_HWS_DFE_MODE   dfeMode,
    IN GT_REG_DATA       *dfeCfg
);

/**
* @internal mvHwsAvago16nmSerdesDfeConfig function
* @endinternal
*
* @brief   Per SERDES configure the DFE parameters.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] dfeMode                  - Enable/disable/freeze/Force
* @param[in] dfeCfg                   - array of dfe configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvago16nmSerdesDfeConfig
(
    IN GT_U8               devNum,
    IN GT_UOPT             portGroup,
    IN GT_UOPT             serdesNum,
    IN MV_HWS_DFE_MODE     dfeMode,
    IN GT_REG_DATA         *dfeCfg
);

/**
* @internal mvHwsAvagoSerdesTemperatureTransmitSet function
* @endinternal
*
* @brief   Per SERDES transmit Tj
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] temperature              - temperature
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTemperatureTransmitSet
(
    IN GT_U8               devNum,
    IN GT_UOPT             portGroup,
    IN GT_UOPT             serdesNum,
    IN GT_32               temperature
);

/**
* @internal mvHwsAvagoSerdesDfeStatus function
* @endinternal
*
* @brief   Per SERDES get the DFE status and parameters.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] dfeLock                  - Locked/Not locked
* @param[out] dfeCfg                   - array of dfe configuration parameters
* @param[out] f0Cfg                    - array of f0 configuration parameters
* @param[out] savCfg                   - array of sav configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesDfeStatus
(
    IN GT_U8             devNum,
    IN GT_UOPT           portGroup,
    IN GT_UOPT           serdesNum,
    OUT GT_BOOL          *dfeLock,
    OUT GT_REG_DATA      *dfeCfg,
    OUT GT_REG_DATA      *f0Cfg,
    OUT GT_REG_DATA      *savCfg
);

/**
* @internal mvHwsAvagoDfeCheck function
* @endinternal
*
* @brief   Check DFE values range and start sampler calibration, if needed.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoDfeCheck
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum
);
#endif

#ifndef CO_CPU_RUN
/**
* @internal mvHwsAvagoSerdesLoopback function
* @endinternal
*
* @brief   Perform an Internal/External SERDES loopback mode for Debug use
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - physical Serdes number
* @param[in] lbMode    - loopback mode
*
* @retval 0 - on success
* @retval 1 - on error
*
* @note  - In 'AN_TX_RX' mode, the Rx & Tx Serdes Polarity
*          configurations should be disabled, for getting LinkUp
*          and Traffic on port.
*        - After return to 'Normal' mode from 'AN_TX_RX' mode,
*          the Rx & Tx Serdes Polarity configuration should be
*          restored, for re-synch again the PCS and MAC units
*          and getting LinkUp and Traffic on port.
*        - Switching between 'AN_TX_RX'/'DIG_RX_TX' modes
*          requires returning to the 'Normal' mode before.
*
*/
GT_STATUS mvHwsAvagoSerdesLoopback
(
    IN GT_U8             devNum,
    IN GT_UOPT           portGroup,
    IN GT_UOPT           serdesNum,
    IN MV_HWS_SERDES_LB  lbMode
);

/**
* @internal mvHwsAvagoSerdesLoopbackGet function
* @endinternal
*
* @brief   Get status of internal loopback (SERDES loopback).
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      serdesType - type of serdes (LP/COM_PHY...)
*
* @param[out] lbModePtr                - current loopback mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesLoopbackGet
(
    IN GT_U8             devNum,
    IN GT_UOPT           portGroup,
    IN GT_UOPT           serdesNum,
    OUT MV_HWS_SERDES_LB *lbModePtr
);

/**
* @internal mvHwsAvagoSerdesTxEnable function
* @endinternal
*
* @brief   Enable/Disable Tx.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] enable                   - whether to  or disable Tx.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxEnable
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_BOOL     enable
);

/**
* @internal mvHwsAvagoSerdesTxEnableGet function
* @endinternal
*
* @brief   Enable / Disable transmission of packets in SERDES layer of a port.
*         Use this API to disable Tx for loopback ports.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] enablePtr                - GT_TRUE  - Enable transmission of packets in
*                                      SERDES layer of a port
*                                      - GT_FALSE - Disable transmission of packets in
*                                      SERDES layer of a port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, serdesNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
*
* @note Disabling transmission of packets in SERDES layer of a port causes
*       to link down of devices that are connected to the port.
*
*/
GT_STATUS mvHwsAvagoSerdesTxEnableGet
(
    IN GT_U8   devNum,
    IN GT_UOPT portGroup,
    IN GT_UOPT serdesNum,
    OUT GT_BOOL *enablePtr
);
#endif

/**
* @internal mvHwsAvagoSerdesTxIfSelect function
* @endinternal
*
* @brief   tx interface select.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      enable    - whether to enable or disable Tx.
* @param[in] serdesTxIfNum            - number of serdes Tx
*       interface
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxIfSelect
(
    IN GT_U8               devNum,
    IN GT_UOPT             portGroup,
    IN GT_UOPT             serdesNum,
    IN GT_UREG_DATA        serdesTxIfNum
);

/**
* @internal mvHwsAvagoSerdesTxIfSelectGet function
* @endinternal
*
* @brief   Return the SERDES Tx interface select number.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      serdesType  - SERDES types
*
* @param[out] serdesTxIfNum            - interface select number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxIfSelectGet
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    OUT GT_U32             *serdesTxIfNum
);

/**
* @internal mvHwsAvagoSerdesCalibrationStatus function
* @endinternal
*
* @brief   Per SERDES return the calibration results.
*         Can be run after create port
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] results                  - the calibration results.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesCalibrationStatus
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      serdesNum,
    OUT MV_HWS_CALIBRATION_RESULTS *results
);

/**
* @internal mvHwsAvagoSerdesFixAlign90Start function
* @endinternal
*
* @brief   Start fix Align90 process on current SERDES.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] params                   - pointer to structure to store SERDES configuration parameters that must be restore
*                                      during process stop operation
*
* @param[out] params                   - SERDES configuration parameters that must be restore
*                                      during process stop operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesFixAlign90Start
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    INOUT MV_HWS_ALIGN90_PARAMS   *params
);

/**
* @internal mvHwsAvagoSerdesFixAlign90Status function
* @endinternal
*
* @brief   Return fix Align90 process current status on current SERDES.
*         Can be run after create port and start Align90.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] trainingStatus          - Training Status
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesFixAlign90Status
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    OUT MV_HWS_AUTO_TUNE_STATUS *trainingStatus
);

/**
* @internal mvHwsAvagoSerdesFixAlign90Stop function
* @endinternal
*
* @brief   Stop fix Align90 process on current SERDES.
*         Can be run after create port and start Align90.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] params                   - SERDES parameters that must be restored (return by mvHwsComHRev2SerdesFixAlign90Start)
* @param[in] fixAlignPass             - true, if fix Align90 process passed; false otherwise
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesFixAlign90Stop
(
    IN GT_U8  devNum,
    IN GT_U32 portGroup,
    IN GT_U32 serdesNum,
    IN MV_HWS_ALIGN90_PARAMS *params,
    IN GT_BOOL fixAlignPass
);

/**
* @internal mvHwsAvagoSerdesSamplerOffsetSet function
* @endinternal
*
* @brief   Serdes Sampler offset set.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] samplSel                 - sampler select
* @param[in] value                    - offset value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSamplerOffsetSet
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_UREG_DATA     samplSel,
    IN GT_UREG_DATA     value
);

/**
* @internal mvHwsAvagoSerdesManualCtleConfig function
* @endinternal
*
* @brief   Set the Serdes Manual CTLE config for DFE
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] ctleConfigPtr            - CTLE Params
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualCtleConfig
(
    IN GT_U8                             devNum,
    IN GT_UOPT                           portGroup,
    IN GT_UOPT                           serdesNum,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT  *rxConfigPtr
);

/*******************************************************************************
*
* @brief   Set the Low Power mode from Avago Serdes
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - physical serdes number*
* @param[in] enableLowPower    - enable/disable low power mode
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvagoSerdesLowPowerModeEnable
(
    IN GT_U8                   devNum,
    IN GT_UOPT                 portGroup,
    IN GT_UOPT                 serdesNum,
    IN GT_BOOL                 enableLowPower
);

/**
* @internal mvHwsAvago16nmSerdesPmroMetricGet function
* @endinternal
*
* @brief   Measures process performance of the chip
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum - physical serdes number
*
* @param[out] metricPtr - pointer to a number indicating
*       propagation delay through a set of gates and
*       interconnects. A higher number indicates faster process
*       performance.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvago16nmSerdesPmroMetricGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       serdesNum,
    OUT GT_U16       *metricPtr
);

/**
* @internal mvHwsAvagoSerdesManualCtleConfigGet function
* @endinternal
*
* @brief   Get the Serdes CTLE (RX) configurations
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical lane number
*
* @param[out] configParams             - Ctle params structures:
*         dcGain         - DC-Gain              (rang: 0-255)
*         lowFrequency   - CTLE Low-Frequency   (rang: 0-15)
*         highFrequency  - CTLE High-Frequency  (rang: 0-15)
*         bandWidth      - CTLE Band-width      (rang: 0-15)
*         squelch        - Signal OK threshold  (rang: 0-308)
*         gainshape1     - CTLE gainshape1
*         gainshape2     - CTLE gainshape2
*         shortChannelEn - Enable/Disable Short channel
*         dfeGAIN        - DFE Gain Tap strength  (rang: 0-255)
*         dfeGAIN2       - DFE Gain Tap2 strength (rang: 0-255)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualCtleConfigGet
(
    IN GT_U8                               devNum,
    IN GT_UOPT                             portGroup,
    IN GT_UOPT                             serdesNum,
    OUT MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    *configParams
);

/**
* @internal mvHwsAvagoSerdesErrorInject function
* @endinternal
*
* @brief   Injects errors into the RX or TX data
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] numOfBits                - Number of error bits to inject (max=65535)
* @param[in] serdesDirection          - Rx or Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesErrorInject
(
    IN GT_U8                      devNum,
    IN GT_UOPT                    portGroup,
    IN GT_UOPT                    serdesNum,
    IN GT_UOPT                    numOfBits,
    IN MV_HWS_SERDES_DIRECTION    serdesDirection
);

/**
* @internal mvHwsAvagoSerdesEyeMetricGet function
* @endinternal
*
* @brief   Get the simple eye metric (mV) in range [0..1000]
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[out] eoMatricValue           - eye metric (mV) [0..1000]
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEyeMetricGet
(
    IN GT_U8       devNum,
    IN GT_U8       portGroup,
    IN GT_U8       serdesNum,
    OUT GT_U32      *eoMatricValue
);

/**
* @internal mvHwsAvagoSerdesSignalOkCfg function
* @endinternal
*
* @brief   Set the signal OK threshold on Serdes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] signalThreshold          - Signal OK threshold (0-15)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalOkCfg
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_UOPT     signalThreshold
);

/**
* @internal mvHwsAvagoSerdesSignalOkThresholdGet function
* @endinternal
*
* @brief   Get the signal OK threshold on Serdes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] signalThreshold          - Signal OK threshold (0-15)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalOkThresholdGet
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    OUT GT_UOPT     *signalThreshold
);

/**
* @internal mvHwsAvagoSerdesShiftSamplePoint function
* @endinternal
*
* @brief   Shift Serdes sampling point earlier in time
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] delay                    - set the  (0-0xF)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesShiftSamplePoint
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_U32      delay
);

/**
* @internal mvHwsAvagoSerdesEnhanceTune function
* @endinternal
*
* @brief   Set the ICAL with shifted sampling point to find best sampling point
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes (0...15)
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTune
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_U8       min_LF,
    IN GT_U8       max_LF
);

/**
* @internal mvHwsAvagoSerdesScanLowFrequency function
* @endinternal
*
* @brief   Scan Low frequency algorithm
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
 *@param[in] phyPortNum               - physical port number
 *@param[in] portMode                 - HWS port mode
* @param[in] serdesArr                - array of serdes's
* @param[in] numOfSerdes              - number of serdes's
* @param[in] minlF                    - minimum LF for reference
* @param[in] maxLf                    - maximum LF for reference
* @param[in] hf                       - HF for reference
 *@param[in] bw                       - BW for reference
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesScanLowFrequency
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_U32      minLf,
    IN GT_U32      maxLf,
    IN GT_U32      hf,
    IN GT_U32      bw
);

/**
* @internal mvHwsAvago16nmSerdesHalGet function
* @endinternal
*
* @brief   Retrieves the indicated HAL register value
*
* @param[in] devNum    - system device number
* @param[in] serdesNum - serdes number
* @param[in] halType   - HAL types
* @param[in] regSelect - HAL register selector
*
* @param[out] halRegValue - HAL register value
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvago16nmSerdesHalGet
(
    IN GT_U8                           devNum,
    IN GT_U32                          serdesNum,
    IN MV_HWS_AVAGO_SERDES_HAL_TYPES   halType,
    IN GT_U32                          regSelect,
    OUT GT_32                          *halRegValue
);

/**
* @internal mvHwsAvago16nmSerdesHalSet function
* @endinternal
*
* @brief   Sets the indicated HAL register value
*
* @param[in] devNum    - system device number
* @param[in] serdesNum - serdes number
* @param[in] halType   - HAL types
* @param[in] regSelect - HAL register selector
* @param[in] registerValue - Value to write into the register
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvago16nmSerdesHalSet
(
    IN GT_U8                           devNum,
    IN GT_U32                          serdesNum,
    IN MV_HWS_AVAGO_SERDES_HAL_TYPES   halType,
    IN GT_U32                          regSelect,
    IN GT_32                           registerValue
);

/**
* @internal mvHwsAvago16nmSerdesHalSetIfValid function
* @endinternal
*
* @brief   Sets the indicated HAL register value
*
* @param[in] devNum    -     system device number
* @param[in] serdesNum -     serdes number
* @param[in] halType   -     HAL types
* @param[in] regSelect -     HAL register selector
* @param[in] registerValue - Value to write into the register
* @param[in] invalidDef -    Skip writing if
*                             registerValue == invalidDef
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvago16nmSerdesHalSetIfValid
(
    IN GT_U8                           devNum,
    IN GT_U32                          serdesNum,
    IN MV_HWS_AVAGO_SERDES_HAL_TYPES   halType,
    IN GT_U32                          regSelect,
    IN GT_32                           registerValue,
    IN GT_U32                          invalidDef
);

/**
* @internal mvHwsAvagoSignalDetectInvert function
* @endinternal
*
* @brief   Per SERDES get indication check CDR lock and Signal
*          Detect change.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] invert                - TRUE if invert signal
*                                     detect
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSignalDetectInvert
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT GT_BOOL     invert
);

/**
* @internal mvHwsAvagoSyncOkErrataFix function
* @endinternal
*
* @brief   Per SERDES fix for wrong signal detect indication with 25G RS-FEC mode.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSyncOkErrataFix
(
    IN GT_U8                            devNum,
    IN GT_U32                           portGroup,
    IN GT_U32                           serdesNum
);

/**
* @internal mvHwsAvago16nmSerdesHalFunc function
* @endinternal
*
* @brief   Invokes the indicated HAL function
*
* @param[in] devNum      - system device number
* @param[in] serdesNum   - serdes number
* @param[in] halFuncType - Type of HAL function
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvago16nmSerdesHalFunc
(
    IN GT_U8       devNum,
    IN GT_U32      serdesNum,
    IN MV_HWS_AVAGO_SERDES_HAL_FUNC_TYPES  halFuncType
);

#ifndef MV_HWS_FREE_RTOS
/**
* @internal mvHwsAvagoSerdesEnhanceTuneLite function
* @endinternal
*
* @brief   Set the PCAL with shifted sampling point to find best sampling point
*         This API runs only for AP port after linkUp indication and before running
*         the Rx-Training Adative pCal
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] serdesSpeed              - speed of serdes
* @param[in] min_dly                  - Minimum delay_cal value:
*                                      - for Serdes speed 10G (25...28)
*                                      - for Serdes speed 25G (15...19)
* @param[in] max_dly                  - Maximum delay_cal value:
*                                      - for Serdes speed 10G (30)
*                                      - for Serdes speed 25G (21)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTuneLite
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN MV_HWS_SERDES_SPEED     serdesSpeed,
    IN GT_U8                   min_dly,
    IN GT_U8                   max_dly
);
#endif /* MV_HWS_FREE_RTOS */

/**
* @internal mvHwsAvagoSerdesEnhanceTuneLitePhase1 function
* @endinternal
*
* @brief   Set the PCAL with shifted sampling point to find best sampling point.
*         This API runs only for AP port after linkUp indication and before running
*         the Rx-Training Adative pCal.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] serdesSpeed              - speed of serdes
* @param[in] currentDelaySize         - delays array size
* @param[in] currentDelayPtr          - delays array
* @param[in] inOutI                   - iteration index between different phases
* @param[in] best_eye                 - best eyes array to update
* @param[in] best_dly                 - best delays array to update
* @param[in] subPhase                 - assist flag to know which code to execute in this sub-phase
* @param[in] min_dly                  - Minimum delay_cal value: (rang: 0-31)
* @param[in] max_dly                  - Maximum delay_cal value: (rang: 0-31)
*
* @param[out] inOutI                   - iteration index between different phases
* @param[out] best_eye                 - best eyes array to update
* @param[out] best_dly                 - best delays array to update
* @param[out] subPhase                 - assist flag to know which code to execute in this sub-phase
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTuneLitePhase1
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN MV_HWS_SERDES_SPEED     serdesSpeed,
    IN GT_U8       currentDelaySize,
    IN GT_U8       *currentDelayPtr,
    INOUT GT_U8       *inOutI,
    INOUT GT_U16      *best_eye,
    INOUT GT_U8       *best_dly,
    INOUT GT_U8       *subPhase,
    IN GT_U8       min_dly,
    IN GT_U8       max_dly
);

/**
* @internal mvHwsAvagoSerdesEnhanceTuneLitePhase2 function
* @endinternal
*
* @brief   Set shift sample point to with the best delay_cal value.
*         This API runs only for AP port after linkUp indication and before running
*         the Rx-Training Adative pCal
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] best_dly                 - best delay to set on serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTuneLitePhase2
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_U8       *best_dly
);

/**
* @internal mvHwsAvagoSerdeCpllOutputRefClkGet function
* @endinternal
*
* @brief   Return the output frequency of CPLL reference clock source of SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] refClk                   - CPLL reference clock frequency
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdeCpllOutputRefClkGet
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT MV_HWS_CPLL_OUTPUT_FREQUENCY    *refClk
);

/**
* @internal mvHwsAvago16nmDividerGet function
* @endinternal
*
* @brief   Return SERDES clock divider value.
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - Serdes Number
*
* @param[out] dividerValue           - sereds clock divider value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvago16nmDividerGet
(
    IN GT_U8                 devNum,
    IN GT_U32                serdesNum,
    OUT GT_U32               *dividerValue
);

/**
* @internal mvHwsAvagoSerdeSpeedGet function
* @endinternal
*
* @brief   Return SERDES baud rate.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdeSpeedGet
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    OUT MV_HWS_SERDES_SPEED *rate
);

/**
* @internal mvHwsAvagoSerdesEomUiGet function
* @endinternal
*
* @brief   Return SERDES baud rate in Ui.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] baudRatePtr              - current system baud rate in pico-seconds.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEomUiGet
(
    IN GT_U8             devNum,
    IN GT_U32            portGroup,
    IN GT_U32            serdesNum,
    OUT GT_U32           *baudRatePtr
);

/**
* @internal mvHwsAvagoSerdesPollingSet function
* @endinternal
*
* @brief   Set the Polling on Avago Serdes register address
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - number of SERDESes to configure
* @param[in] regAddr                  - Serdes register address (offset) to access
* @param[in] mask                     - register read mask
* @param[in] numOfLoops               - polling on spico ready
* @param[in] delay                    -  in ms after register get value
* @param[in] expectedValue            - value for stopping the polling on register
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPollingSet
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_U32      regAddr,
    IN GT_U32      mask,
    IN GT_U32      numOfLoops,
    IN GT_U32      delay,
    IN GT_U32      expectedValue
);

/**
* @internal mvHwsAvagoSerdesAutoTuneStatusShort function
* @endinternal
*
* @brief   Check the Serdes Rx or Tx training status
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] rxStatus                 - Rx-Training status
* @param[out] txStatus                 - Tx-Training status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneStatusShort
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    OUT MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    OUT MV_HWS_AUTO_TUNE_STATUS *txStatus
);

/**
* @internal mvHwsAvagoSerdesVoltageGet function
* @endinternal
*
* @brief   Get the voltage (in mV) from Avago Serdes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] chipIndex                - chip index
*
* @param[out] voltage                  - Serdes  value (in mV)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesVoltageGet
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     chipIndex,
    OUT GT_UOPT    *voltage
);

/**
* @struct MV_HWS_AVAGO_SERDES_EYE_GET_RESULT
 *
 * @brief A struct containing the results for Serdes Eye Monitoring
 * For Caelum, Aldrin, AC3X, Bobcat3.
*/
typedef struct{

    char *matrixPtr;

    unsigned int                 x_points;

    unsigned int                 y_points;

    char *vbtcPtr;

    char *hbtcPtr;

    unsigned int                 height_mv;

    unsigned int                 width_mui;

} MV_HWS_AVAGO_SERDES_EYE_GET_RESULT;

/**
* @struct MV_HWS_AVAGO_SERDES_EYE_GET_INPUT
 *
 * @brief A struct containing the input parameters for Serdes Eye Monitoring
 * For Caelum, Aldrin, AC3X, Bobcat3.
*/
typedef struct{

    unsigned int            min_dwell_bits;

    unsigned int            max_dwell_bits;

} MV_HWS_AVAGO_SERDES_EYE_GET_INPUT;

/**
* @internal mvHwsAvagoSerdesMultiLaneCtleBiasConfig function
* @endinternal
*
* @brief   Per SERDES set the CTLE Bias value, according to data base.
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - physical serdes number
* @param[in] numOfLanes               - number of lanes
* @param[in] ctleBitMapIndex          - index of ctle bias bit
*                                        map
*
* @param[out] ctleBiasPtr              - pointer to ctle bias value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesMultiLaneCtleBiasConfig
(
    IN GT_U8 devNum,
    IN unsigned int serdesNum,
    IN GT_U8 numOfLanes,
    IN unsigned int ctleBitMapIndex,
    OUT GT_U32 *ctleBiasPtr
);

/**
* @internal mvHwsAvagoSbusReset function
* @endinternal
*
* @brief   Per SERDES perform sbus reset
*
* @param[in] devNum                  - system device number
* @param[in] serdesNum               - physical serdes number
* @param[in] reset                   - reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSbusReset
(
    IN GT_U8 devNum,
    IN GT_U32 serdesNum,
    IN GT_BOOL reset
);

/**
* @internal mvHwsAvagoSerdesEyeGet function
* @endinternal
*
* @brief   Per SERDES perform sbus reset
*
* @param[in] devNum                  - system device number
* @param[in] serdesNum               - physical serdes number
* @param[in] eye_inputPtr            - pointer to input parameters structure
*
* @param[out] eye_resultsPtr         - pointer to results structure
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && !defined(CPSS_BLOB)
unsigned int mvHwsAvagoSerdesEyeGet
(
    IN unsigned char                       devNum,
    IN unsigned int                        serdesNum,
    IN MV_HWS_AVAGO_SERDES_EYE_GET_INPUT  *eye_inputPtr,
    OUT MV_HWS_AVAGO_SERDES_EYE_GET_RESULT *eye_resultsPtr
);

#endif

/**
* @internal mvHwsAvagoSerdesVosOverrideModeSet function
* @endinternal
*
* @brief   Set the override mode of the serdeses VOS parameters. If the override mode
*         is set to true, the VOS parameters will be overriden, if set to false, they will
*         not be overriden.
* @param[in] devNum                   - device number.
* @param[in] vosOverride              - GT_TRUE means override, GT_FALSE means no override.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesVosOverrideModeSet
(
    IN GT_U8   devNum,
    IN GT_BOOL vosOverride
);

/**
* @internal mvHwsAvagoSerdesVosOverrideModeGet function
* @endinternal
*
* @brief   Get the override mode of the serdeses VOS parameters. If the override mode
*         is set to true, the VOS parameters will be overriden, if set to false, they will
*         not be overriden.
* @param[in] devNum                   - device number.
*
* @param[out] vosOverridePtr           - (pointer to) current VOS override mode.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesVosOverrideModeGet
(
    IN GT_U8   devNum,
    OUT GT_BOOL *vosOverridePtr
);

/**
* @internal mvHwsAvagoSerdesManualVosParamsGet function
* @endinternal
*
* @brief   Get the VOS Override parameters from the local DB.
*
* @param[in] devNum                   - device number.
*
* @param[out] vosParamsPtr            - Get the VOS params.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualVosParamsGet
(
    IN unsigned char  devNum,
    OUT unsigned long  *vosParamsPtr
);

/**
* @internal mvHwsAvagoSerdesAutoTuneStartExt function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] rxTraining               - Rx Training modes
* @param[in] txTraining               - Tx Training modes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneStartExt
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN MV_HWS_RX_TRAINING_MODES    rxTraining,
    IN MV_HWS_TX_TRAINING_MODES    txTraining
);

/**
* @internal mvHwsPortAvagoDfeCfgSet function
* @endinternal
*
* @brief   run training for Avago SerDes.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - phyPortNum.
* @param[in] portMode                 - Port Mode.
* @param[in] dfeMode                  - training mode.

* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoDfeCfgSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_DFE_MODE         dfeMode
);

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT)

/**
* @internal mvHwsAvagoSerdesBc3VosConfig function
* @endinternal
*
* @brief   Reading the HDFuse and locating the AVG block where the compressed VOS
*         data parameters are.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - system device number
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesBc3VosConfig
(
    IN unsigned char devNum
);

/**
* @internal mvHwsAvagoSerdesRxSignalOkGet function
* @endinternal
*
* @brief   Get the rx_signal_ok indication from o_core_status serdes macro.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
*
* @param[out] signalOk                 - (pointer to) whether or not rx_signal_ok is up
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesRxSignalOkGet
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT GT_BOOL     *signalOk
);

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetectByPhaseInitDb function
* @endinternal
*
* @brief   Init Hws DB of edge-detect-by-phase algorithm used by port manager.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
* @param[in] timeOut                  - timeout value for the edge detection
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectByPhaseInitDb
(
    IN GT_U8   devNum,
    IN GT_U32  portNum,
    IN GT_U32  timeOut
);

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetectPhase1 function
* @endinternal
*
* @brief   Set DFE dwell_time parameter. This is phase1 of the by-phase edge detect
*         algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] numOfSerdeses            - num of serdeses in the serdeses array
* @param[in] serdesArr                - serdeses array
* @param[in] threshEO                 - Eye monitor threshold
* @param[in] dwellTime                - define dwell value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectPhase1
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      numOfSerdeses,
    IN GT_U32      *serdesArr,
    IN GT_32       threshEO,
    IN GT_U32      dwellTime
);

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetectPhase2_1 function
* @endinternal
*
* @brief   Launching pCal and take starting time for the pCal process. This is
*         phase2_1 of the by-phase edge detect algorithm.
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
* @param[in] portGroup                - port group (core) number
* @param[in] numOfSerdeses            - num of serdeses in the serdeses array
* @param[in] serdesArr                - serdeses array
* @param[in] threshEO                 - Eye monitor threshold
* @param[in] dwellTime                - define dwell value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectPhase2_1
(
    IN GT_U8       devNum,
    IN GT_U32      portNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      numOfSerdeses,
    IN GT_U32      *serdesArr,
    IN GT_32       threshEO,
    IN GT_U32      dwellTime
);

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetectPhase2_2 function
* @endinternal
*
* @brief   Calculate pCal time duration, check for timeout or for peer TxAmp change
*         (edge-detection). This is phase2_2 of the by-phase edge detect algorithm.
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
* @param[in] portGroup                - port group (core) number
* @param[in] numOfSerdeses            - num of serdeses in the serdeses array
* @param[in] serdesArr                - serdeses array
* @param[in] threshEO                 - Eye monitor threshold
* @param[in] dwellTime                - define dwell value
* @param[in] continueExecute          - whether or not algorithm execution should be continued
* @param[in] isInNonBlockWaitPtr      - whether or not algorithm is in non-block waiting time
*
* @param[out] continueExecute          - whether or not algorithm execution should be continued
* @param[out] isInNonBlockWaitPtr      - whether or not algorithm is in non-block waiting time
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectPhase2_2
(
    IN GT_U8       devNum,
    IN GT_U32      portNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      numOfSerdeses,
    IN GT_U32      *serdesArr,
    IN GT_32       threshEO,
    IN GT_U32      dwellTime,
    INOUT GT_BOOL     *continueExecute,
    INOUT GT_BOOL     *isInNonBlockWaitPtr
);

/**
* @internal mvHwsAvagoSerdesEnhaceTuneByPhaseInitDb function
* @endinternal
*
* @brief   Init Hws DB of enhance-tune-by-phase algorithm used by port manager.
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhaceTuneByPhaseInitDb
(
    IN GT_U8   devNum,
    IN GT_U32  phyPortNum
);

/**
* @internal mvHwsAvagoSerdesOneShotTuneByPhaseInitDb function
* @endinternal
*
* @brief   Init Hws DB of one shot -tune-by-phase algorithm for
* KR2/CR2 mode, used by port manager.
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesOneShotTuneByPhaseInitDb
(
    IN GT_U8   devNum,
    IN GT_U32  phyPortNum
);

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase1 function
* @endinternal
*
* @brief   Move sample point and launch iCal. this is phase1 of the phased enhance-tune
*         algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] currentDelayPtr          - delays attay to set initial values with
* @param[in] currentDelaySize         - delay array size
* @param[in] best_eye                 -  array to set initial values with
*
* @param[out] currentDelayPtr          - delays attay to set initial values with
* @param[out] currentDelaySize         - delay array size
* @param[out] best_eye                 -  array to set initial values with
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase1
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    INOUT GT_U32      **currentDelayPtr,
    INOUT GT_U32      *currentDelaySize,
    INOUT GT_U32      *best_eye
);

/**
* @internal mvHwsAvagoSerdesPrePostTuningByPhase function
* @endinternal
*
* @brief   reset/unreset CG_UNIT Rx GearBox Register, and clock
*          from SerDes to GearBox before running iCal.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - phyPortNum.
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] resetEnable              - reset True/False.
* @param[in] fecCorrect               - FEC type.
* @param[in] portMode                 - Port Mode.
* @param[out] coreStatusReady                 -  array that
*       indicates if the core status of all lanes is ready
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPrePostTuningByPhase
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      phyPortNum,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_BOOL     resetEnable,
    OUT GT_BOOL     *coreStatusReady,
    IN MV_HWS_PORT_FEC_MODE  fecCorrect,
    IN MV_HWS_PORT_STANDARD  portMode
);

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase2 function
* @endinternal
*
* @brief   Getting LF values from serdeses. this is phase2 of the phased enhance-tune
*         algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] best_LF                  - array to be filled with LF values from serdes
*
* @param[out] best_LF                  - array to be filled with LF values from serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase2
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    INOUT GT_U32      *best_LF
);

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase3 function
* @endinternal
*
* @brief   Move sample point, then launch pCal than calculate eye. this is phase3
*         of the phased enhance-tune algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] currentDelaySize         - delays array size
* @param[in] currentDelayPtr          - delays array
* @param[in] inOutI                   - iteration index between different phases
* @param[in] best_LF                  - best LF array to update
* @param[in] best_eye                 - best eyes array to update
* @param[in] best_dly                 - best delays array to update
* @param[in] subPhase                 - assist flag to know which code to execute in this sub-phase
*
* @param[out] inOutI                   - iteration index between different phases
* @param[out] best_LF                  - best LF array to update
* @param[out] best_eye                 - best eyes array to update
* @param[out] best_dly                 - best delays array to update
* @param[out] subPhase                 - assist flag to know which code to execute in this sub-phase
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase3
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_U32      currentDelaySize,
    IN GT_U32      *currentDelayPtr,
    INOUT GT_U32      *inOutI,
    INOUT GT_U32      *best_LF,
    INOUT GT_U32      *best_eye,
    INOUT GT_U32      *best_dly,
    INOUT GT_U8       *subPhase
);

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase4 function
* @endinternal
*
* @brief   Move sample point to the best delay value that was found in previous phase
*         then launch iCal. this is phase4 of the phased enhance-tune algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] best_dly                 - best delay to set the serdes with
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase4
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_U32      *best_dly
);

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase5 function
* @endinternal
*
* @brief   Get LF values from serdes then launch iCal. this is phase5 of the phased
*         enhance-tune algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] LF1_Arr                  - LF array to fill with values
*
* @param[out] LF1_Arr                  - LF array to fill with values
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase5
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    INOUT int         *LF1_Arr
);

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase6 function
* @endinternal
*
* @brief   Read LF and Gain from serdes after iCal was launched in previous phase,
*         than calculate best LF and Gain, write to serdes and launch pCal.
*         this is phase6 of the phased enhance-tune algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] LF1_Arr                  - LF array to fill with values
* @param[in] min_LF                   - minimum LF for reference
* @param[in] max_LF                   - maimum LF for reference
* @param[in] LF1_Arr                  - previous LF values for reference
* @param[in] best_eye                 - besy eye for printing purpose
* @param[in] best_dly                 - besy delay for printing purpose
* @param[in] inOutKk                  - iterator index
* @param[in] continueExecute          - whether or not to continue algorithm execution
*
* @param[out] inOutKk                  - iterator index
* @param[out] continueExecute          - whether or not to continue algorithm execution
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase6
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_U8       min_LF,
    IN GT_U8       max_LF,
    IN int         *LF1_Arr,
    IN GT_U32      *best_eye,
    IN GT_U32      *best_dly,
    INOUT GT_U8       *inOutKk,
    INOUT GT_BOOL     *continueExecute
);


/**
* @internal mvHwsAvagoAdaptiveCtlePortEnableSet function
* @endinternal
*
* @brief  enable adaptive ctle on specific port
*
* @param[in] devNum                   - system device number
* @param[in] portNum                   - port number
* @param[in] enable                   - GT_TRUE - enable
*                                       GT_FALSE - disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoAdaptiveCtlePortEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  phyPortNum,
    IN GT_BOOL enable
);


/**
* @internal mvHwsAvagoAdaptiveCtleSerdesesInitDb function
* @endinternal
*
* @brief   Init Hws DB of adaptive ctle values.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoAdaptiveCtleSerdesesInitDb
(
    IN GT_U8   devNum
);

/**
* @internal mvHwsAvagoAdaptiveCtleBasedTemperaturePhase1
*           function
* @endinternal
*
* @brief   Run Steady State apdative ctle algorithm. this
*          feature do delay and LF Calibration based Temperature
*          to improve the EO per serdes. in case it does't
*          improved, do rollback.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] portIndex                - port number
* @param[in] serdesArr                - port's serdeses arrray
* @param[in] numOfSerdes              - serdeses number
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoAdaptiveCtleBasedTemperaturePhase1
(
    IN GT_U8      devNum,
    IN GT_UOPT    portGroup,
    IN GT_U32     portIndex,
    IN GT_U16     *serdesArr,
    IN GT_UOPT    numOfSerdes
);


/**
* @internal mvHwsAvagoAdaptiveCtleBasedTemperaturePhase2
*           function
* @endinternal
*
* @brief   Run Steady State apdative ctle algorithm. this
*          feature do delay and LF Calibration based Temperature
*          to improve the EO per serdes. in case it does't
*          improved, do rollback.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] portIndex                - port number
* @param[in] serdesArr                - port's serdeses arrray
* @param[in] numOfSerdes              - serdeses number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoAdaptiveCtleBasedTemperaturePhase2
(
    IN GT_U8      devNum,
    IN GT_UOPT    portGroup,
    IN GT_U32     portIndex,
    IN GT_U16     *serdesArr,
    IN GT_UOPT    numOfSerdes
);

#endif

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT)
/**
* @internal mvHwsAvagoSerdesDefaultRxTerminationSet function
* @endinternal
*
* @brief   Initialize Avago envelope related configurations
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] rxTermination            - rxTermination
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesDefaultRxTerminationSet
(
    GT_U8                                devNum,
    GT_U32                               portGroup,
    GT_U8                                rxTermination
);

#endif

/**
* @internal hwsAvagoSerdesTxRxTuneParamsArrayGetSize function
* @endinternal
*
* @brief   return the number of lines in the parameters database
*
*
* @retval GT_U8                        - array size
*/
GT_U8 hwsAvagoSerdesTxRxTuneParamsArrayGetSize(void);


/**
* @internal mvHwsAvagoSerdesOperation function
* @endinternal
*
* @brief   Sets on port extra operations needed
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] operation                - operation to do
* @param[in] data                     - data
* @param[out] result                  - return data
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesOperation
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      serdesNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_SERDES_OPERATION_MODE_ENT  operation,
    GT_U32                      *data,
    GT_U32                      *result
);

#ifdef __cplusplus
}
#endif

#endif /* __mvAvagoIf_H */


