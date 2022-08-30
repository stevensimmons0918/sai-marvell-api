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
* @file mv_hws_avago_if.h
*
* @brief
*
* @version   15
********************************************************************************
*/

#ifndef __mv_hws_avago_if_H
#define __mv_hws_avago_if_H

#ifdef __cplusplus
extern "C" {
#endif
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

/************************* definition *****************************************************/
#define  AVAGO_ETH_CONNECTION         1
#define  AVAGO_I2C_CONNECTION         2
#define  AVAGO_SBUS_CONNECTION        3
#define  MAX_AVAGO_SERDES_ADDRESS     255
#define  NA_VALUE                     0
#define  AVAGO_INVALID_SBUS_ADDR      (-1)
#define  AVAGO_SERDES_INIT_BYPASS_VCO (0xFFFF)
#define  AVAGO_CTLE_BITMAP            32

#define MV_HWS_AVAGO_SPICO_BRDCST_ADDR AVAGO_SERDES_BROADCAST_ADDRESS
#define MV_HWS_AVAGO_SPICO_SBM_ADDR    AVAGO_SBUS_MASTER_ADDRESS

#define MV_HWS_AVAGO_MIN_TEMEPRATURE   0
#define MV_HWS_AVAGO_MAX_TEMPERATURE   115

/*#define  MARVELL_AVAGO_DEBUG_FLAG*/

#ifdef MARVELL_AVAGO_DEBUG_FLAG
#define AVAGO_DBG(s) osPrintf s
#else
#define AVAGO_DBG(s)
#endif /* MARVELL_AVAGO_DEBUG_FLAG */

#define CHECK_AVAGO_RET_CODE_WITH_ACTION(action) \
{ \
    if(aaplSerdesDb[chipIndex]->return_code < 0) \
    { \
        hwsOsPrintf("%s failed (return code %d)\n", __func__, aaplSerdesDb[chipIndex]->return_code); \
        aapl_get_return_code(aaplSerdesDb[chipIndex]); \
        action; \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG); \
    } \
}

#define CHECK_AVAGO_RET_CODE() \
{ \
    if(aaplSerdesDb[chipIndex]->return_code < 0) \
    { \
        hwsOsPrintf("%s failed (return code %d)\n", __func__, aaplSerdesDb[chipIndex]->return_code); \
        aapl_get_return_code(aaplSerdesDb[chipIndex]); \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG); \
    } \
}

/************************* Globals *******************************************************/
/**
* @internal mvHwsAvagoSerdesTxParametersOffsetSet function
* @endinternal
*
* @brief   Initialize Avago related configurations
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] phyPortNum               - port number
* @param[in] serdesSpeed              port speed
* @param[in] serdesNumber             - serdes number
* @param[in] offsets                  - value which stores 12 bits of tx  values
*
* @param[out] baseAddrPtr              - (pointer to) base address of the tx offsets database in FW memory
* @param[out] sizePtr                  - (pointer to) size of the tx offsets database in FW memory
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxParametersOffsetSet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroup,
    IN GT_U32 phyPortNum,
    IN MV_HWS_SERDES_SPEED serdesSpeed,
    IN GT_U8  serdesNumber,
    IN GT_U16 offsets,
    OUT GT_U32 *baseAddrPtr,
    OUT GT_U32 *sizePtr
);

void mvHwsAvagoAaplAddrGet
(
    unsigned int chipIndex,
    GT_UINTPTR   *devAddr,
    unsigned int *devSize
);

int mvHwsAvagoInitializationCheck
(
    unsigned char devNum,
    unsigned int  serdesNum
);

#ifndef MV_HWS_REDUCED_BUILD
GT_BOOL mvHwsAvagoFalconIsCpuSerdes
(
    GT_U8 devNum,
    unsigned int serdesNum,
    unsigned int *chipIndex
);

/**
 * mvHwsAaplSerdesDbInit
 *
 * @param devNum
 *
 * @return int
 *       0  - on success
 *       1  - on error
 */
int mvHwsAaplSerdesDbInit(unsigned char devNum, unsigned int chipsInDevBmp);
#endif

#ifndef ASIC_SIMULATION
/**
* @internal mvHwsAvagoSerdesInit function
* @endinternal
*
* @brief   Initialize Avago related configurations
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mvHwsAvagoSerdesInit
(
    unsigned char devNum,
    unsigned int chipIndex
);
#endif

GT_STATUS mvHwsAvagoConvertSerdesToSbusAddr
(
    unsigned char devNum,
    unsigned int  serdesNum,
    GT_U32  *sbusAddr
);

void mvHwsAvagoAaplInit
(
    unsigned char devNum, unsigned char chipIndex
);

/**
* @internal mvHwsAvagoSerdesManualVosParamsSet function
* @endinternal
*
* @brief   Set the VOS Override parameters in to the DB. Used to manually set the
*         VOS override parameters instead of using the eFuze burned values.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualVosParamsSet
(
    unsigned char  devNum,
    unsigned long  *vosParamsPtr,
    unsigned long  bufferIdx,
    unsigned long  bufferLength
);

GT_STATUS mvHwsAvagoSerdesManualVosMinAndBaseForBc3ParamsSet
(
    unsigned char  devNum,
    unsigned long  minVosValueToSet,
    unsigned long  baseVosValueToSet
);

/**
* @internal mvHwsAvagoSerdesManualVosParamsDBInit function
* @endinternal
*
* @brief   Init the VOS override DB from the DFX.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualVosParamsDBInit
(
    IN unsigned char  devNum
);

/*******************************************************************************
* mvHwsAvagoSerdesAdaptiveStatusGet
*
* DESCRIPTION:
*       Per SERDES check is adaptive pcal is running
*
* INPUTS:
*       devNum    - system device number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       adaptiveIsRuning - true= adaptive running, false=not running
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsAvagoSerdesAdaptiveStatusGet
(
    IN unsigned char   devNum,
    IN unsigned int    serdesNum,
    OUT GT_BOOL         *adaptiveIsRuning
);

/**
* @internal mvHwsAvagoSpicoInterrupt function
* @endinternal
*
* @brief   Issue the interrupt to the Spico processor.
*         The return value is the interrupt number.
*
* @param[out] result                   - spico interrupt return value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSpicoInterrupt
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroup,
    IN  GT_U32 chipIndex,
    IN  GT_U32 sbusAddr,
    IN  GT_U32 interruptCode,
    IN  GT_U32 interruptData,
    OUT GT_32  *result
);

/**
* @internal mvHwsAvagoSerdesSpicoInterrupt function
* @endinternal
*
* @brief   Issue the interrupt to the Spico processor.
*         The return value is the interrupt number.
*
* @param[out] result                   - spico interrupt return value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSpicoInterrupt
(
    IN GT_U8   devNum,
    IN GT_U32    portGroup,
    IN GT_U32    serdesNum,
    IN GT_U32    interruptCode,
    IN GT_U32    interruptData,
    OUT GT_32      *result
);

/*******************************************************************************
*
* @brief   Get the Low Power mode from Avago Serdes
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - physical serdes number
*
* @param[out] mode - enable/disable low power mode
*
* @retval 0 - on success
* @retval 1 - on error
*/
int mvHwsAvagoSerdesLowPowerModeEnableGet
(
    IN unsigned char   devNum,
    IN unsigned int    portGroup,
    IN unsigned int    serdesNum,
    OUT GT_BOOL         *mode
);


/*******************************************************************************
*
* @brief   Get the Temperature (in C) from Avago Serdes
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] chipIndex - Chip Index (Relevant for Falcon Only)
* @param[out] temperature - Serdes  degree value (In Celsius)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTemperatureGet
(
    IN GT_U8    devNum,
    IN GT_U32   portGroup,
    IN GT_U32   chipIndex,
    OUT GT_32   *temperature
);

/**
* @internal mvHwsAvagoSerdesPolarityConfigImpl function
* @endinternal
*
* @brief   Per serdes invert the Tx or Rx.
*         Can be run after create port.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPolarityConfigImpl
(
    IN GT_U8      devNum,
    IN GT_U32     portGroup,
    IN GT_U32     serdesNum,
    IN GT_U32     invertTx,
    IN GT_U32     invertRx
);

/**
* @internal mvHwsAvagoSerdesPolarityConfigGetImpl function
* @endinternal
*
* @brief   Per SERDES invert the Tx or Rx.
*         Can be run after create port.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPolarityConfigGetImpl
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT GT_U32     *invertTx,
    OUT GT_U32     *invertRx
);

/**
* @internal mvHwsAvagoSerdesResetImpl function
* @endinternal
*
* @brief   Per SERDES Clear the serdes registers (back to defaults.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] analogReset              - Analog Reset (On/Off
* @param[in] digitalReset             - digital Reset (On/Off)
* @param[in] syncEReset               - SyncE Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesResetImpl
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_BOOL     analogReset,
    IN GT_BOOL     digitalReset,
    IN GT_BOOL     syncEReset
);

/**
* @internal mvHwsAvagoSerdesArrayPowerCtrlImpl function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesArrayPowerCtrlImpl
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_UOPT     numOfSer,
    IN GT_UOPT     *serdesArr,
    IN GT_U8       powerUp,
    IN GT_U32      divider,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
);

/**
 * @internal mvHwsAvagoSerdesGenericPowerCtrl function
 * @endinternal
 *
 * @brief SerDes debug function
 *
 * @param devNum
 * @param portGroup
 * @param serdesNum
 * @param refclockSrc
 * @param powerUp
 * @param ref_clk_div
 * @param divider
 * @param busWidth
 * @param encoding
 * @param swizzle
 * @param precoding
 * @param gray
 * @param squelchLevel
 * @param rxPolarity
 * @param txPolarity
 * @param initRx
 * @param initTx
 * @param enabletTxOut
 * @param ilb
 * @param prbs31
 *
 * @return GT_STATUS
 */
GT_STATUS mvHwsAvagoSerdesGenericPowerCtrl
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroup,
    IN GT_U32                       serdesNum,
    IN MV_HWS_REF_CLOCK_SRC         refclockSrc,
    IN GT_U8                        powerUp,
    IN GT_U32                       ref_clk_div,
    IN GT_U32                       divider,
    IN MV_HWS_SERDES_BUS_WIDTH_ENT  busWidth,
    IN MV_HWS_SERDES_ENCODING_TYPE  encoding,
    IN GT_BOOL                      swizzle,
    IN GT_BOOL                      precoding,
    IN GT_BOOL                      gray,
    IN GT_U32                       squelchLevel,
    IN GT_BOOL                      rxPolarity,
    IN GT_BOOL                      txPolarity,
    IN GT_BOOL                      initRx,
    IN GT_BOOL                      initTx,
    IN GT_BOOL                      enabletTxOut,
    IN GT_BOOL                      ilb,
    IN GT_BOOL                      prbs31
);

#ifdef RAVEN_DEV_SUPPORT
#define MV_HWS_AVAGO_SERDES_POWER_FIRST_PHASE   0
#define MV_HWS_AVAGO_SERDES_POWER_LAST_PHASE    1
/**
* @internal mvHwsAvagoSerdesArrayPowerCtrlImplPhase function
* @endinternal
*
* @brief   Only in RAVEN (cm3) this funcion is calling Power up
*          SERDES next phase.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesArrayPowerCtrlImplPhase
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_UOPT     numOfSer,
    IN GT_UOPT     *serdesArr,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr,
    IN GT_U8       phase
);
#endif
/**
* @internal mvHwsAvagoSerdesPowerCtrlImpl function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPowerCtrlImpl
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN GT_U8               powerUp,
    IN GT_U32              divider,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
);

/**
* @internal mvHwsAvagoSerdesManualTxDBSet function
* @endinternal
*
* @brief   Initialize Avago tx related configurations
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] phyPortNum               - port number
* @param[in] serdesSpeed              - port speed
* @param[in] serdesNumber             - serdes number
* @param[in] txConfigPtr              - tx config params
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualTxDBSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               portGroup,
    IN GT_U32                               phyPortNum,
    IN MV_HWS_SERDES_SPEED                  serdesSpeed,
    IN GT_U16                               serdesNumber,
    IN MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA *txConfigPtr
);

/**
* @internal mvHwsAvagoSerdesManualRxDBSet function
* @endinternal
*
* @brief   Initialize Avago rx related configurations
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] phyPortNum               - port number
* @param[in] serdesSpeed              - port speed
* @param[in] serdesNumber             - serdes number
* @param[in] rxConfigPtr              - rx config params
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualRxDBSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               portGroup,
    IN GT_U32                               phyPortNum,
    IN MV_HWS_SERDES_SPEED                  serdesSpeed,
    IN GT_U16                               serdesNumber,
    IN MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA *rxConfigPtr
);

/**
* @internal mvHwsAvagoSerdesManualDBClear function
* @endinternal
*
* @brief   Clear Avago serdes manual db
*
* @param[in] devNum                   - device number
* @param[in] phyPortNum               - port number
* @param[in] serdesNumber             - serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualDBClear
(
    IN GT_U8                                devNum,
    IN GT_U32                               phyPortNum,
    IN GT_U16                               serdesNumber
);

#ifdef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsAvagoSerdesManualTxPresetSet function
* @endinternal
*
* @brief   Initialize Avago preset parameters
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] rxTermination            - rxTermination
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if 0
MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA hwsAvagoSerdesTxTune16nmPresetParams[3][AN_PRESETS_NUM] = /* Spico int 0x3D */
{                      /* Preset1[CL136] / Preset[CL72]          Preset2[CL136] / Initialize[CL72]     Preset3[CL136] */
                       /*  atten  post   pre    pre2   pre3      atten  post   pre    pre2   pre3      atten  post   pre    pre2   pre3 */
/*_10_3125G            */{{4     ,0     ,0     ,TX_NA ,TX_NA }, {4     ,12    ,2     ,TX_NA ,TX_NA }, {TX_NA ,TX_NA ,TX_NA ,TX_NA ,TX_NA ,}},
/*_25_78125G           */{{0     ,0     ,0     ,TX_NA ,TX_NA }, {0     ,14    ,2     ,TX_NA ,TX_NA }, {TX_NA ,TX_NA ,TX_NA ,TX_NA ,TX_NA ,}},

                    /* 25.3.2020 doron changed PRESET for PAM4 to 0,2,6 performance improved*/
/*_26_5625G_PAM4       */{{0     ,2     ,6     ,0     ,TX_NA }, {0     ,6     ,6     ,0     ,TX_NA }, {0     ,0     ,10    ,3     ,TX_NA ,}}
};
#endif
GT_STATUS mvHwsAvagoSerdesManualTxPresetDBSet
(
    GT_U8                                devNum,
    GT_U16                               serdesNumber,
    GT_U8                                serdesSpeed,
    GT_U8                                pre,
    GT_U8                                atten,
    GT_U8                                post
);
#endif

/**
* @internal mvHwsAvagoSerdesManualInterconnectDBSet function
* @endinternal
*
* @brief   Initialize interconnect profile configurations
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] phyPortNum               - port number
* @param[in] serdesNumber             - serdes number
* @param[in] interconnectProfile       - interconnect profile
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualInterconnectDBSet
(
    GT_U8                                devNum,
    GT_U32                               portGroup,
    GT_U32                               phyPortNum,
    GT_U16                               serdesNumber,
    GT_U8                                interconnectProfile
);

/**
* @internal mvHwsAvagoSerdesManualInterconnectDBGet function
* @endinternal
*
* @brief   Get interconnect profile configurations
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] phyPortNum               - port number
* @param[in] serdesNumber             - serdes number
* @param[out] interconnectProfile      - interconnect profile
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualInterconnectDBGet
(
    GT_U8                                devNum,
    GT_U32                               portGroup,
    GT_U32                               phyPortNum,
    GT_U16                               serdesNumber,
    GT_U32                               *interconnectProfilePtr
);

GT_STATUS mvHwsAvagoSerdesCalcLevel3
(
    GT_U8                                devNum,
    GT_U32                               portGroup,
    GT_U16                               serdesNum
);

#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) || defined (FALCON_DEV_SUPPORT)
#define AVAGO_LOCK(devNum, chipIndex) mvHwsAvagoAccessLock(devNum, chipIndex)
#define AVAGO_UNLOCK(devNum, chipIndex) mvHwsAvagoAccessUnlock(devNum, chipIndex)
#else
#define AVAGO_LOCK(devNum, chipIndex) mvHwsAvagoAccessLock(devNum)
#define AVAGO_UNLOCK(devNum, chipIndex) mvHwsAvagoAccessUnlock(devNum)
#endif

/**
* @internal mvHwsAvagoAccessLock function
* @endinternal
*
* @brief   Avago device access lock
*/
void mvHwsAvagoAccessLock
(
    unsigned char devNum
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) || defined (FALCON_DEV_SUPPORT)
    ,
    unsigned char chipIndex
#endif
);

/**
* @internal mvHwsAvagoAccessUnlock function
* @endinternal
*
* @brief   Avago device access unlock
*/
void mvHwsAvagoAccessUnlock
(
    unsigned char devNum
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) || defined (FALCON_DEV_SUPPORT)
    ,
    unsigned char chipIndex
#endif
);

/**
* @internal setSbusClkDiv function
* @endinternal
*
* @brief   set SBUS clock divider
*/
GT_STATUS setSbusClkDiv(GT_U32 divClk);

#ifdef __cplusplus
}
#endif

#endif /* __mv_hws_avago_if_H */


