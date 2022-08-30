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
* @file mvHwsSerdesIf.h
*
* @brief
*
* @version   23
********************************************************************************
*/

#ifndef __mvHwsSerdesIf_H
#define __mvHwsSerdesIf_H

#ifdef __cplusplus
extern "C" {
#endif


/* General H Files */
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>

#ifdef MV_HWS_REDUCED_BUILD
#define PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_SET(_var,_value)\
    _var = _value;
#define PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(_var)       \
    _var
#else

#define PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(commonMod.serdesDir.mvHwsSerdesIfSrc._var,_value)

#define PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.serdesDir.mvHwsSerdesIfSrc._var)
#endif

/**
* @enum MV_HWS_SERDES_TX_PATTERN
 *
 * @brief Defines different serdes transmit patterns.
 * Comment: Other - as output value means any other mode not mentioned
 * in this enum;
 * as input means HW default for PHY Test Data 5
 * register;
*/
typedef enum
{
  _1T,
  _2T,
  _4T,
  _5T,
  _8T,
  _10T,
  PRBS7,
  PRBS9,
  PRBS15,
  PRBS23,
  PRBS31,
  DFETraining,
  Other,
  PRBS11,
  PRBS13,


  LAST_SERDES_TX_PATTERN

}MV_HWS_SERDES_TX_PATTERN;

/**
* @enum MV_HWS_SERDES_TEST_GEN_MODE
 *
 * @brief Defines different serdes test generator modes.
*/
typedef enum{

    SERDES_NORMAL,

    SERDES_TEST

} MV_HWS_SERDES_TEST_GEN_MODE;

/**
* @enum MV_HWS_SERDES_DIRECTION
 *
 * @brief Defines SERDES direction enumerator.
*/
typedef enum
{
    RX_DIRECTION,
    TX_DIRECTION

}MV_HWS_SERDES_DIRECTION;


/**
 * @struct MV_HWS_SERDES_TEST_GEN_STATUS
 *
 * @brief Defines serdes test generator results
*/
typedef struct
{
  GT_U32  errorsCntr;
  GT_U64  txFramesCntr;
  GT_U32  lockStatus;

}MV_HWS_SERDES_TEST_GEN_STATUS;

/**
 * @struct MV_HWS_SERDES_EOM_RES
 *
 * @brief Defines serdes EOM results.
*/
typedef struct
{
    GT_U32  horizontalEye;
    GT_U32  verticalEye;
    GT_U32  eomMechanism;
    GT_U32  rowSize;
    GT_U32  *lowerMatrix;
    GT_U32  *upperMatrix;
} MV_HWS_SERDES_EOM_RES;

/**
 * @struct MV_HWS_ALIGN90_PARAMS
 *
 * @brief Defines serdes Align 90 results.
*/
typedef struct
{
    GT_U32 startAlign90;
    GT_U32 rxTrainingCfg;
    GT_U32 osDeltaMax;
    GT_U32 adaptedFfeR;
    GT_U32 adaptedFfeC;

}MV_HWS_ALIGN90_PARAMS;

/**
 * @struct MV_HWS_SAMPLER_RESULTS
 *
 * @brief Defines serdes Sampler results.
*/
typedef struct
{
    GT_U32 bestVal;
    GT_U32 positiveVal;
    GT_U32 negativeVal;

}MV_HWS_SAMPLER_RESULTS;

/**
* @enum MV_HWS_D2D_DIRECTION_MODE_ENT
 *
 * @brief Defines D2D direction enumerator.
*/
typedef enum
{
    HWS_D2D_DIRECTION_MODE_BOTH_E,
    HWS_D2D_DIRECTION_MODE_RX_E,
    HWS_D2D_DIRECTION_MODE_TX_E
}MV_HWS_D2D_DIRECTION_MODE_ENT;

/**
 * @struct MV_HWS_D2D_PRBS_CONFIG_STC
 *
 * @brief Defines D2D PRBS.
*/
typedef struct
{
    MV_HWS_D2D_DIRECTION_MODE_ENT       direction;                  /* common parameter */
    MV_HWS_PORT_FEC_MODE                fecMode;                    /* relate to Lane Tester PRBS */
    GT_BOOL                             userDefPatternEnabled;      /* relate to Lane Tester PRBS */
    GT_U32                              userDefPattern[4];          /* relate to Lane Tester PRBS */
    MV_HWS_SERDES_TX_PATTERN            serdesPattern;              /* relate to Lane Tester PRBS */
    MV_HWS_SERDES_TEST_GEN_MODE         mode;                       /* relate to Lane Tester PRBS */
    MV_HWS_PORT_TEST_GEN_PATTERN        portPattern;                /* relate to PCS/PMA PRBS */
}MV_HWS_D2D_PRBS_CONFIG_STC;

/**
* @internal mvHwsSerdesTestGen function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit
* @param[in] serdesType               - type of SerDes
* @param[in] mode                     - test  or normal
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTestGen
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroup,
    IN GT_U32                    serdesNum,
    IN MV_HWS_SERDES_TX_PATTERN  txPattern,
    IN MV_HWS_SERDES_TYPE        serdesType,
    IN MV_HWS_SERDES_TEST_GEN_MODE mode
);

/**
* @internal mvHwsSerdesTestGenGet function
* @endinternal
*
* @brief   Get configuration of the Serdes test generator/checker.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - type of SerDes
* @param[in] txPattern                - pattern to transmit
*
* @param[out] modePtr               - Results
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - on wrong serdes type
* @retval GT_FAIL                  - HW error
*/
GT_STATUS mvHwsSerdesTestGenGet
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroup,
    IN GT_U32                    serdesNum,
    IN MV_HWS_SERDES_TYPE        serdesType,
    IN MV_HWS_SERDES_TX_PATTERN  *txPatternPtr,
    OUT MV_HWS_SERDES_TEST_GEN_MODE *modePtr
);

/**
* @internal mvHwsSerdesDigitalRfResetToggleSet function
* @endinternal
*
* @brief   Run digital RF Reset/Unreset on current SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] waitTime                 - wait time between Reset/Unreset
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on error
*/
GT_STATUS mvHwsSerdesDigitalRfResetToggleSet
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_U8                   waitTime
);

/**
* @internal mvHwsSerdesTestGenStatus function
* @endinternal
*
* @brief   Read the tested pattern receive error counters and status.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - type of SerDes
* @param[in] txPattern                - pattern to transmit
* @param[in] counterAccumulateMode    - Accumulate result (T/F)
*
* @param[out] status                - Test status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTestGenStatus
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroup,
    IN GT_U32                    serdesNum,
    IN MV_HWS_SERDES_TYPE        serdesType,
    IN MV_HWS_SERDES_TX_PATTERN  txPattern,
    IN GT_BOOL                   counterAccumulateMode,
    OUT MV_HWS_SERDES_TEST_GEN_STATUS *status
);

/**
* @internal mvHwsSerdesEOMGet function
* @endinternal
*
* @brief   Returns the horizontal/vertical Rx eye margin.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - type of SerDes
* @param[in] timeout                  - wait time in msec
*
* @param[out] results                - Results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesEOMGet
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroup,
    IN GT_U32                    serdesNum,
    IN MV_HWS_SERDES_TYPE        serdesType,
    IN GT_U32                    timeout,
    OUT MV_HWS_SERDES_EOM_RES     *results
);

/**
* @internal mvHwsSerdesPolarityConfig function
* @endinternal
*
* @brief   Per SERDES invert the Tx or Rx.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] invertTx                 - invert TX polarity (GT_TRUE - invert, GT_FALSE - don't)
* @param[in] invertRx                 - invert RX polarity (GT_TRUE - invert, GT_FALSE - don't)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPolarityConfig
(
    IN GT_U8             devNum,
    IN GT_U32            portGroup,
    IN GT_U32            serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    IN GT_BOOL           invertTx,
    IN GT_BOOL           invertRx
);

/**
* @internal mvHwsSerdesPolarityConfigGet function
* @endinternal
*
* @brief   Per SERDES invert the Tx or Rx.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] invertTx                 - invert TX polarity (GT_TRUE - invert, GT_FALSE - don't)
* @param[out] invertRx                 - invert RX polarity (GT_TRUE - invert, GT_FALSE - don't)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPolarityConfigGet
(
    IN GT_U8             devNum,
    IN GT_U32            portGroup,
    IN GT_U32            serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    OUT GT_BOOL           *invertTx,
    OUT GT_BOOL           *invertRx
);

/**
* @internal mvHwsSerdesFixAlign90Start function
* @endinternal
*
* @brief   Start fix Align90 process on current SERDES.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] params                   - SERDES configuration parameters that must be restore
*                                      during process stop operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesFixAlign90Start
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    OUT MV_HWS_ALIGN90_PARAMS   *params
);

/**
* @internal mvHwsSerdesFixAlign90Status function
* @endinternal
*
* @brief   Return fix Align90 process current status on current SERDES.
*         Can be run after create port and start Align90.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] trainingStatus          - Training Status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesFixAlign90Status
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    OUT MV_HWS_AUTO_TUNE_STATUS *trainingStatus
);

/**
* @internal mvHwsSerdesFixAlign90Stop function
* @endinternal
*
* @brief   Stop fix Align90 process on current SERDES.
*         Can be run after create port and start Align90.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] params                   - SERDES parameters that must be restored (return by mvHwsComHRev2SerdesFixAlign90Start)
* @param[in] fixAlignPass             - true, if fix Align90 process passed; false otherwise
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesFixAlign90Stop
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    IN MV_HWS_ALIGN90_PARAMS   *params,
    IN GT_BOOL                 fixAlignPass
);

/**
* @internal mvHwsSerdesManualRxConfigGet function
* @endinternal
*
* @brief   Return SERDES RX parameters configured manually .
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] portTuningModePtr       - Tuning mode params
* @param[out] sqlchPtr                 - Squelch threshold.
* @param[out] ffeResPtr                - FFE R
* @param[out] ffeCapPtr                - FFE C
* @param[out] aligPtr                  - align 90 value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesManualRxConfigGet
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    OUT MV_HWS_SERDES_RX_CONFIG_DATA_UNT  *configParams
);

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsSerdesPartialPowerDown function
* @endinternal
*
* @brief   Enable/Disable power down of Tx and Rx on Serdes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - number of SERDESes to configure
* @param[in] serdesType               - serdes type
* @param[in] powerDownRx              - Enable/Disable the power down on Serdes Rx
* @param[in] powerDownTx              - Enable/Disable the power down on Serdes Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPartialPowerDown
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    IN MV_HWS_SERDES_TYPE     serdesType,
    IN GT_BOOL     powerDownRx,
    IN GT_BOOL     powerDownTx
);
#endif
/**
* @internal mvHwsSerdesPartialPowerStatusGet function
* @endinternal
*
* @brief   Get the status of power Tx and Rx on Serdes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - number of SERDESes to configure
* @param[in] serdesType               - serdes type
* @param[out] powerRxStatus            - GT_TRUE - Serdes power
*                                      Rx is down GT_FALSE -
*                                      Serdes power Rx is up
* @param[out] powerTxStatus            - GT_TRUE - Serdes power
*                                      Tx is down GT_FALSE -
*                                      Serdes power Tx is up
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPartialPowerStatusGet
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    IN MV_HWS_SERDES_TYPE     serdesType,
    OUT GT_BOOL     *powerRxStatus,
    OUT GT_BOOL     *powerTxStatus
);

/**
* @internal mvHwsSerdesFirmwareDownload function
* @endinternal
*
* @brief   Download Firmware
*
* @param[in] devNum            - Device Number
* @param[in] portGroup         - Port Group
* @param[in] serdesType        - serdes type
* @param[in] serdesArr         - array of serdes numbers
* @param[in] numOfSerdeses     - The size of serdesArr
* @param[in] firmwareSelector  - FW to download
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
*/
GT_STATUS mvHwsSerdesFirmwareDownload
(
    IN  GT_U8                  devNum,
    IN  GT_U32                 portGroup,
    IN  MV_HWS_SERDES_TYPE     serdesType,
    IN  GT_U32                 serdesArr[],
    IN  GT_U8                  numOfSerdeses,
    IN  GT_U8                  firmwareSelector
);

/**
* @internal mvHwsPortTxRxDefaultsDbEntryGet function
* @endinternal
*
* @brief   Get the default Tx and CTLE(Rx) parameters of the
*          defaults DB entry for the proper port mode
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesSpeed              - SerDes speed/baud rate
*                                       applicable for Avago
*                                       based devices
* @param[out] txParamsPtr             - pointer of tx params to
*                                       store the db entry
*                                       params in
* @param[out] ctleParamsPtr           - pointer of rx (ctle)
*                                       params to store the db
*                                       entry params in
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
GT_STATUS mvHwsPortTxRxDefaultsDbEntryGet
(
    GT_U8                   devNum,
    MV_HWS_SERDES_SPEED     serdesSpeed,
    MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA    *txParamsPtr,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA        *ctleParamsPtr
);
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */
/*
#ifdef CHX_FAMILY
HWS_MUTEX serdesAccessMutex = 0;
#endif*/ /* defined(CHX_FAMILY) */

#if defined(MV_HWS_REDUCED_BUILD) && !defined(AC5_DEV_SUPPORT)
#define SERDES_LOCK(devNum, serdesType)
#define SERDES_UNLOCK(devNum, serdesType)
#else
#define SERDES_LOCK(devNum, serdesType)   if ((serdesType >= COM_PHY_C12GP41P2V) && (serdesType < SERDES_LAST)) mvHwsSerdesAccessLock(devNum);
#define SERDES_UNLOCK(devNum, serdesType) if ((serdesType >= COM_PHY_C12GP41P2V) && (serdesType < SERDES_LAST)) mvHwsSerdesAccessUnlock(devNum);
#endif

#ifdef MV_HWS_FREE_RTOS
extern GT_STATUS mvPortCtrlSerdesLock(void);
extern GT_STATUS mvPortCtrlSerdesUnlock(void);
#endif /* MV_HWS_FREE_RTOS */
/**
* @internal mvHwsSerdesAccessLock function
* @endinternal
*
* @brief   Serdes device access lock
*/
void mvHwsSerdesAccessLock
(
    unsigned char devNum
);

/**
* @internal mvHwsSerdesAccessUnlock function
* @endinternal
*
* @brief   Serdes device access unlock
*/
void mvHwsSerdesAccessUnlock
(
    unsigned char devNum
);

#ifdef __cplusplus
}
#endif
#endif /* __mvHwsSerdesIf_H */


