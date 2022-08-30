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
* @file mvComPhyH28nmIf.c
*
* @brief Com PHY H (serdes) interface
*
* @version   39
********************************************************************************
*/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdes28nmPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nm/mvComPhyH28nmDb.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nm/mvComPhyH28nmEomIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nm/mvComPhyH28nmIf.h>

#ifndef CO_CPU_RUN
static char* mvHws28nmSerdesTypeGetFunc(void)
{
  return "COM_H_28NM";
}
#endif

#ifdef FreeRTOS
#include <gtOs/gtOsIo.h>
static GT_U8 powerUpWaitTime = 4;
#else
static GT_U8 powerUpWaitTime = 1;
#endif

/************************* Globals *******************************************************/

static GT_BOOL hwsInitDone = GT_FALSE;


static GT_U32 timer_dfe2 = 6;
static GT_U32 timer_dfe1 = 6;
static GT_U32 timer_f0d2 = 6;
static GT_U32 timer_f0d1 = 1;
static GT_U32 timer_dfe3 = 6;
static GT_U32 timer_edge1 = 6;
static GT_U32 timer_edge2 = 6;
static GT_U32 timer_f0b1 = 4;
static GT_U32 timer_f0b2 = 6;

extern GT_U32  cpssDeviceRunCheck_onEmulator(void);
GT_STATUS mvHwsComH28nmCheckSerdesAccess(GT_U8 devNum, GT_UOPT  portGroup, GT_UOPT  serdesNum)
{
  GT_UREG_DATA data;

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
  if (cpssDeviceRunCheck_onEmulator())
  {
      /* Don't check data in emulator */
      return GT_OK;
  }
#endif
  /* check analog reset */
  CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                      SERDES_EXTERNAL_CONFIGURATION_1, &data, 0));

  if (((data >> 3) & 1) == 0)
      return GT_NOT_INITIALIZED;

  return GT_OK;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsComH28nmSerdesTestGen function
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
GT_STATUS mvHwsComH28nmSerdesTestGen
(
    GT_U8                     devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    MV_HWS_SERDES_TEST_GEN_MODE mode
)
{
  GT_U8     seqId;
  GT_UREG_DATA      patternData;

  CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum, portGroup, serdesNum));

  switch(txPattern)
  {
  case   _1T:
    patternData = 1;
    break;
  case _2T:
    patternData = 2;
    break;
  case _5T:
    patternData = 3;
    break;
  case _10T:
    patternData = 4;
    break;
  case PRBS7:
    patternData = 0x80;
    break;
  case  PRBS9:
    patternData = 0x81;
      break;
  case PRBS15:
    patternData = 0x82;
    break;
  case  PRBS23:
    patternData = 0x83;
    break;
  case  PRBS31:
    patternData = 0x84;
      break;
  case DFETraining:
    patternData = 0x12;
    break;
  case Other:
    patternData = 0;
    break;
  default:
    return GT_NOT_SUPPORTED;
  }

  patternData |= 0x20; /* PRBS load continuous mode */

  CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_Pattern_Reg, patternData, 0xFF));

  /* run pattern sequence */
  seqId = (mode == SERDES_NORMAL) ? SERDES_PT_AFTER_PATTERN_NORMAL_SEQ : SERDES_PT_AFTER_PATTERN_TEST_SEQ;
  CHECK_STATUS(mvCfgSerdesSeqExec(devNum, portGroup, serdesNum, seqId));

  return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesTestGenGet function
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
GT_STATUS mvHwsComH28nmSerdesTestGenGet
(
    GT_U8                    devNum,
    GT_UOPT                    portGroup,
    GT_UOPT                    serdesNum,
    MV_HWS_SERDES_TX_PATTERN  *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE *modePtr
)
{
    GT_UREG_DATA data;

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));

    /* get pattern type */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Pattern_Reg, &data, 0xDF));
    switch(data)
    {
        case 1:
            *txPatternPtr = _1T;
            break;
        case 2:
            *txPatternPtr = _2T;
            break;
        case 3:
            *txPatternPtr = _5T;
            break;
        case 4:
            *txPatternPtr = _10T;
            break;
        case 0x80:
            *txPatternPtr = PRBS7;
            break;
        case  0x81:
            *txPatternPtr = PRBS9;
            break;
        case 0x82:
            *txPatternPtr = PRBS15;
            break;
        case  0x83:
            *txPatternPtr = PRBS23;
            break;
        case  0x84:
            *txPatternPtr = PRBS31;
            break;
        case 0x12:
            *txPatternPtr = DFETraining;
            break;
        default:
            *txPatternPtr = Other;
            break;
    }

    /* get PRBS test mode */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_PT_PATTERN_SEL_Reg, &data, 0x8000));
    *modePtr = (0 == data) ? SERDES_NORMAL : SERDES_TEST;

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesTestGenStatus function
* @endinternal
*
* @brief   Read the tested pattern receive error counters and status.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesTestGenStatus
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    GT_BOOL                   counterAccumulateMode,
    MV_HWS_SERDES_TEST_GEN_STATUS *status
)
{
    GT_U32 data;

    txPattern = txPattern;

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_PT_PATTERN_SEL_Reg, &data, 0));
    status->lockStatus = (data >> 12) & 1;

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_ERROR_Cntr_MSB_Reg, &data, 0));
    status->errorsCntr = (data << 16);
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_ERROR_Cntr_LSB_Reg, &data, 0));
    status->errorsCntr += data;
    /* read 48 bit counter */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_FRAME_Cntr_MSB1_Reg, &data, 0));
    status->txFramesCntr.l[1] = (GT_U32)data & 0x0000ffff;
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_FRAME_Cntr_MSB_Reg, &data, 0));
    status->txFramesCntr.l[0] = ((GT_U32)data << 16) & 0xffff0000;
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_FRAME_Cntr_LSB_Reg, &data, 0));
    status->txFramesCntr.l[0] |= (GT_U32)data & 0x0000ffff;

    /* reset counter */
    if (counterAccumulateMode == GT_FALSE)
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_SEL_BITS_Reg, (1 << 11), (1 << 11)));
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_SEL_BITS_Reg, 0, (1 << 11)));
    }

    return 0;
}

/**
* @internal mvHwsComH28nmSerdesPolarityConfig function
* @endinternal
*
* @brief   Per serdes invert the Tx or Rx.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesPolarityConfig
(
    GT_U8   devNum,
    GT_UOPT   portGroup,
    GT_UOPT   serdesNum,
    GT_BOOL invertTx,
    GT_BOOL invertRx
)
{
  GT_UREG_DATA data;

  CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));

  data = ((invertRx) ? 1 : 0) << 11;
  CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_Sync_Pattern_Reg, data, 0x800));
  data = ((invertTx) ? 1 : 0) << 10;
  CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_Sync_Pattern_Reg, data, 0x400));

  return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesPolarityConfigGet function
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
GT_STATUS mvHwsComH28nmSerdesPolarityConfigGet
(
    GT_U8             devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    GT_BOOL           *invertTx,
    GT_BOOL           *invertRx
)
{
    GT_UREG_DATA data;

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));

    /* check analog reset */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                        SERDES_EXTERNAL_CONFIGURATION_1, &data, (1 << 3)));
    if(data == 0)
        return GT_NOT_INITIALIZED;

    /* check digital reset */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                        SERDES_EXTERNAL_CONFIGURATION_1, &data, (1 << 6)));
    if(data == 0)
        return GT_NOT_INITIALIZED;

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Sync_Pattern_Reg, &data, (1 << 10)));
    *invertTx = (data) ? 1 : 0;
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Sync_Pattern_Reg, &data, (1 << 11)));
    *invertRx = (data) ? 1 : 0;

    return GT_OK;
}

#endif

/**
* @internal mvHwsComH28nmSerdesReset function
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
GT_STATUS mvHwsComH28nmSerdesReset
(
    GT_U8      devNum,
    GT_UOPT      portGroup,
    GT_UOPT      serdesNum,
    GT_BOOL    analogReset,
    GT_BOOL    digitalReset,
    GT_BOOL    syncEReset
)
{
    GT_U8 seqId;

    seqId = (analogReset == GT_TRUE) ? SERDES_SD_RESET_SEQ : SERDES_SD_UNRESET_SEQ;
    CHECK_STATUS(mvCfgSerdesSeqExec(devNum, portGroup, serdesNum, seqId));

    seqId = (digitalReset == GT_TRUE) ? SERDES_RF_RESET_SEQ : SERDES_RF_UNRESET_SEQ;
    CHECK_STATUS(mvCfgSerdesSeqExec(devNum, portGroup, serdesNum, seqId));

    seqId = (syncEReset == GT_TRUE) ? SERDES_SYNCE_RESET_SEQ : SERDES_SYNCE_UNRESET_SEQ;
    CHECK_STATUS(mvCfgSerdesSeqExec(devNum, portGroup, serdesNum, seqId));

    return 0;
}

/**
* @internal mvHwsComH28nmSerdesCoreReset function
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
GT_STATUS mvHwsComH28nmSerdesCoreReset
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         serdesNum,
    MV_HWS_RESET    coreReset
)
{
    GT_U8 seqId;

    seqId = (coreReset == RESET) ? SERDES_CORE_RESET_SEQ : SERDES_CORE_UNRESET_SEQ;
    CHECK_STATUS(mvCfgSerdesSeqExec(devNum, portGroup, serdesNum, seqId));

    return 0;
}

/**
* @internal mvHwsComH28nmSerdesDigitalReset function
* @endinternal
*
* @brief   Run digital reset / unreset on current SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] digitalReset             - digital Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesDigitalReset
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         serdesNum,
    MV_HWS_RESET    digitalReset
)
{
    GT_U8 seqId;

    seqId = (digitalReset == RESET) ? SERDES_RF_RESET_SEQ : SERDES_RF_UNRESET_SEQ;
    CHECK_STATUS(mvCfgSerdesSeqExec(devNum, portGroup, serdesNum, seqId));

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesFailVCO function
* @endinternal
*
* @brief   WA for fail VCO in slow corner in cold condition
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - number of Serdes to configure
* @param[in] baudRate                 - Serdes speed
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComH28nmSerdesFailVCO
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    MV_HWS_SERDES_SPEED    baudRate
)
{
    GT_U32  data;

    /* Read Speed_PLL calibration result*/
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Speed_Pll_Calibr_Reg, &data, 0x3F));

    /* if the Speed_PLL calibration value is zero, new value is set to the PLL */
    if (data == 0)
    {
        switch (baudRate)
        {
        case _1_25G:
        case _4_25G:
        case _5G:
        case _5_15625G:
        case _5_625G:
        case _10_9375G:
            data = 0x22;
            break;
        case _3_125G:
        case _3_75G:
            data = 0x10;
            break;
        case _6_25G:
        case _7_5G:
            data = 0x11;
            break;
        case _10_3125G:
            data = 0x25;
            break;
        case _11_25G:
        case _11_5625G:
        case _12_5G:
        case _12_1875G:
            data = 0x38;
            break;
        case SPEED_NA:
        default:
            data = 0x25;
            return GT_FAIL;
        }

        data = 0x100 | (data << 2);
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_KVCO_CAL_Reg, data, 0x1FC));

        /* need to pulse CAL_START the Serdes in order that the setting take effact */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_KVCO_CAL_Reg, 0x8000, 0x8000));

        hwsOsExactDelayPtr(devNum, portGroup, 10); /* delay of 10ms */

        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_KVCO_CAL_Reg, 0, 0x8000));
    }

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesPowerCtrl function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      numOfSer  - number of SERDESes to configure
*                                      serdesArr - collection of SERDESes to configure
* @param[in] powerUp                  - true for power UP
*                                      baudRate  -
*                                      refClock  - ref clock value
*                                      refClockSource - ref clock source (primary line or secondary)
*                                      media     - RXAUI or XAUI
*                                      mode      - 10BIT mode (enable/disable)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesPowerCtrl
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    GT_U8         seqId;
    GT_UREG_DATA  data;
    GT_U8         checkMode;

    /* serdes power up / down */
    if (powerUp)
    {
        /* config media */
        data = (serdesConfigPtr->media == RXAUI_MEDIA) ? (1 << 15) : 0;
        /* config 10BIT mode */
        data += (serdesConfigPtr->busWidth == _10BIT_ON) ? (1 << 14) : 0;

        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                            SERDES_EXTERNAL_CONFIGURATION_0, data, (3 << 14)));

        /* Serdes Analog Un Reset*/
        CHECK_STATUS(mvHwsComH28nmSerdesReset(devNum, portGroup, serdesNum, GT_FALSE, GT_TRUE, GT_TRUE));
        CHECK_STATUS(mvHwsComH28nmSerdesCoreReset(devNum, portGroup, serdesNum, UNRESET));

        /* Reference clock source */
        data = ((serdesConfigPtr->refClockSource == PRIMARY) ? 0 : 1) << 10;
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_MISC_Control0_Reg, data, (1 << 10)));

        /* Serdes Speed */
        switch (serdesConfigPtr->baudRate)
        {
        case _1_25G:
            seqId = SERDES_SPEED_1_25G_SEQ;
            break;
        case _3_125G:
            seqId = SERDES_SPEED_3_125G_SEQ;
            break;
        case _3_75G:
            seqId = SERDES_SPEED_3_75G_SEQ;
            break;
        case _4_25G:
            seqId = SERDES_SPEED_4_25G_SEQ;
            break;
        case _5G:
            seqId = SERDES_SPEED_5G_SEQ;
            break;
        case _5_15625G:
            seqId = SERDES_SPEED_5_15625G_SEQ;
            break;
        case _5_625G:
            seqId = SERDES_SPEED_5_625G_SEQ;
            break;
        case _6_25G:
            seqId = SERDES_SPEED_6_25G_SEQ;
            break;
        case _7_5G:
            seqId = SERDES_SPEED_7_5G_SEQ;
            break;
        case _10_3125G:
            seqId = SERDES_SPEED_10_3125G_SEQ;
            break;
        case _11_25G:
            seqId = SERDES_SPEED_11_25G_SEQ;
            break;
        case _11_5625G:
            seqId = SERDES_SPEED_11_5625G_SEQ;
            break;
        case _12_5G:
            seqId = SERDES_SPEED_12_5G_SEQ;
            break;
        case _10_9375G:
            seqId = SERDES_SPEED_10_9375G_SEQ;
            break;
        case _12_1875G:
            seqId = SERDES_SPEED_12_1875G_SEQ;
            break;
        case SPEED_NA:
        default:
            return GT_FAIL;
        }
        CHECK_STATUS(mvCfgSerdesSeqExec(devNum, portGroup, serdesNum, seqId));

        /* Serdes Power up Ctrl*/
        CHECK_STATUS_EXT(mvCfgSerdesPowerUpCtrl(devNum,portGroup,1,&serdesNum),LOG_ARG_STRING_MAC("serdes sequence with delays"));

        /* WA for fail VCO */
        CHECK_STATUS(mvHwsComH28nmSerdesFailVCO(devNum, portGroup, serdesNum, serdesConfigPtr->baudRate));

        /* VDD calibration start (pulse) */
        data = (1 << 2);
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_Calibr_En_Reg, data, (1 << 2)));

        hwsOsExactDelayPtr(devNum, portGroup, 1);

        /* VDD calibration end (pulse) */
        data = 0;
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_Calibr_En_Reg, data, (1 << 2)));

        /* Note: when changing FeedbackDivider register values(below), SerdesSpeedGet function should be updated as well */
        if (serdesConfigPtr->baudRate == _10_9375G)
        {
            /* change FBDiv to 0x23 give 10.9375G */
            CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_FeedbackDivider_Reg, 0xA234, 0));
        }
        else if ((serdesConfigPtr->baudRate == _11_25G) || (serdesConfigPtr->baudRate == _5_625G))
        {
            /* for _11_25G: change FBDiv to 0x24 give 11.25G, for _5_625G: change FBDiv to 36 give 5.625G */
            CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_FeedbackDivider_Reg, 0xA244, 0));
        }
        else if (serdesConfigPtr->baudRate == _12_1875G)
        {
            /* change FBDiv to 0x27 give 12.1875G */
            CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_FeedbackDivider_Reg, 0xA277, 0));
        }
        else if  (serdesConfigPtr->baudRate == _5_15625G)
        {
            /* change FBDiv to 0x21 give 5.15625G */
            CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_FeedbackDivider_Reg, 0xA214, 0));
        }
        else if (serdesConfigPtr->baudRate == _11_5625G)
        {
            /* change FBDiv to 0x25 give 11.5625G */
            CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_FeedbackDivider_Reg, 0xA254, 0));
        }

        /* wait 1 msec */
        hwsOsExactDelayPtr(devNum, portGroup, powerUpWaitTime);

        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                            SERDES_EXTERNAL_STATUS_0, &data, 0));

        /* Serdes Digital Un Reset */
        CHECK_STATUS(mvHwsComH28nmSerdesReset(devNum, portGroup, serdesNum, GT_FALSE, GT_FALSE, GT_FALSE));
        CHECK_STATUS(mvHwsComH28nmSerdesCoreReset(devNum, portGroup, serdesNum, UNRESET));


        checkMode = 0x1c;
#ifdef ASIC_SIMULATION
        data = 0x1c;
#endif
        if ((data & checkMode) != checkMode)
        {
            return GT_FAIL;
        }
    }

    if (powerUp == GT_FALSE)
    {
        CHECK_STATUS(mvHwsComH28nmSerdesReset(devNum, portGroup, serdesNum, GT_TRUE, GT_FALSE, GT_FALSE));
        CHECK_STATUS(mvHwsComH28nmSerdesCoreReset(devNum, portGroup, serdesNum, RESET));

        seqId = SERDES_SERDES_POWER_DOWN_CTRL_SEQ;
        CHECK_STATUS(mvCfgSerdesSeqExec(devNum, portGroup, serdesNum, seqId));

        CHECK_STATUS(mvHwsComH28nmSerdesReset(devNum, portGroup, serdesNum, GT_TRUE, GT_TRUE, GT_TRUE));
        CHECK_STATUS(mvHwsComH28nmSerdesCoreReset(devNum, portGroup, serdesNum, RESET));
    }

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesAcTerminationCfg function
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
GT_STATUS mvHwsComH28nmSerdesAcTerminationCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    GT_BOOL                 acTermEn
)
{
    GT_U32 data;

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));

    data = (acTermEn == GT_TRUE) ? (1 << 15) : 0;

    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_REV2_RX_FFE_Reg, data, (1 << 15)));

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesRxAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the Rx Training starting.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] rxTraining               - Rx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesRxAutoTuneStart
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     rxTraining
)
{
    GT_U8           seqId;
    GT_UREG_DATA    data;

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum, portGroup, serdesNum));

    CHECK_STATUS(mvHwsComH28nmSerdesAutoTuneCfg(devNum, portGroup, serdesNum, rxTraining, GT_FALSE));

    seqId = (rxTraining) ? SERDES_RX_TRAINING_ENABLE_SEQ : SERDES_RX_TRAINING_DISABLE_SEQ;
    CHECK_STATUS(mvCfgSerdesSeqExec(devNum, portGroup, serdesNum, seqId));

    if (rxTraining == GT_FALSE)
    {
        /* disable RX training, sync TX enbale/disable cfg */
        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                            SERDES_EXTERNAL_CONFIGURATION_1, &data, 0));
        data = ((data >> 2) & 1) << 8;
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_PHY_Isolate_Reg, data, (1 << 8)));
    }

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesAutoTuneCfg function
* @endinternal
*
* @brief   Per SERDES configure parameters for TX training & Rx Training starting
*         Can be run after create port.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesAutoTuneCfg
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     rxTraining,
    GT_BOOL     txTraining
)
{
    GT_U8           seqId;
    GT_UREG_DATA    data;
    GT_STATUS       res;
    GT_U32          ffeR=0;

    if (txTraining && rxTraining)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum, portGroup, serdesNum));

    seqId = (txTraining) ? SERDES_TX_TRAINING_ENABLE_SEQ : SERDES_TX_TRAINING_DISABLE_SEQ;
    CHECK_STATUS(mvCfgSerdesSeqExec(devNum, portGroup, serdesNum, seqId));

    if ((txTraining == GT_TRUE) || (rxTraining == GT_TRUE))
    {
        CHECK_STATUS(mvHwsComH28nmSerdesAdaptPpm(devNum, portGroup, serdesNum, GT_FALSE));

        res = mvHwsSerdesDfeOpti(devNum, portGroup, serdesNum, HWS_DEV_SERDES_TYPE(devNum, serdesNum), &ffeR);
        /* for 28nmRev0 this functionality is not relevant therefore,
           it's not implemented so GT_NOT_IMPLEMENTED is a valid return code */
        if ((res != GT_OK) && (res != GT_NOT_IMPLEMENTED))
        {
            return res;
        }
    }

    if (txTraining == GT_FALSE)
    {
        /* disable TX training, sync TX enbale/disable cfg */
        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                            SERDES_EXTERNAL_CONFIGURATION_1, &data, 0));
        data = ((data >> 2) & 1) << 8;
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_PHY_Isolate_Reg, data, (1 << 8)));
    }

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesTxAutoTuneStart function
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
GT_STATUS mvHwsComH28nmSerdesTxAutoTuneStart
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     txTraining
)
{
    GT_U8           seqId;
    GT_UREG_DATA    data;

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum, portGroup, serdesNum));

    if (txTraining == GT_TRUE)
    {
        /* start training */
        data = (1 << 8);
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                            SERDES_EXTERNAL_CONFIGURATION_3, data, (1 << 8)));
        data = 2;
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_Tx_Train_Reg0, data, 2));
    }
    else
    {
        seqId = SERDES_TX_TRAINING_DISABLE_SEQ;
        CHECK_STATUS(mvCfgSerdesSeqExec(devNum, portGroup, serdesNum, seqId));
    }

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] rxTraining               - Rx Training (true/false)
* @param[in] txTraining               - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesAutoTuneStart
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     rxTraining,
    GT_BOOL     txTraining
)
{
    if (txTraining && rxTraining)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsComH28nmSerdesAutoTuneCfg(devNum, portGroup, serdesNum, rxTraining, txTraining));
    CHECK_STATUS(mvHwsComH28nmSerdesTxAutoTuneStart(devNum, portGroup, serdesNum, txTraining));

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesManualTxConfig function
* @endinternal
*
* @brief   Per SERDES configure the TX parameters: amplitude, 3 TAP Tx FIR.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesManualTxConfig
(
    GT_U8    devNum,
    GT_UOPT  portGroup,
    GT_UOPT  serdesNum,
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT *txConfigPtr
)
{
    GT_UREG_DATA data;

    if (txConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum, portGroup, serdesNum));

    data = (txConfigPtr->txComphyH.txAmpShft << 15);
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_GEN1_SET1_Reg, data, (1 << 15)));

    data = ((txConfigPtr->txComphyH.txAmpAdj ? 1 : 0) << 6);
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_GEN1_SET0_Reg, data, (1 << 6)));

    data = (txConfigPtr->txComphyH.txAmp << 1);
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_GEN1_SET0_Reg, data, 0x3E));

    data = txConfigPtr->txComphyH.txEmph0 + (1 << 4);
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_GEN1_SET2_Reg, data, 0x1F));

    data = (txConfigPtr->txComphyH.txEmph1 << 7) + (1 << 11);
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_GEN1_SET0_Reg, data, 0xF80));

    if (txConfigPtr->txComphyH.slewCtrlEn)
    {
        data = (1 << 15) | ((txConfigPtr->txComphyH.slewRate & 0x7) << 12);
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_GEN1_SET0_Reg, data, 0xF000));
    }
    else
    {
        /* If slewCtrl is not enabled or being disabled, don't touch the rate value. */
        data = 0;
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_GEN1_SET0_Reg, data, 0x8000));
    }

    data = (1 << 2);
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_TX_Train_Preset_Reg, data, (1 << 2)));

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesAutoTuneIsEnabled function
* @endinternal
*
* @brief   Per SERDES check if Rx or Tx training is enabled
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] isTx                     - Rx or Tx
*
* @param[out] isEnable                 - Training Enabled/Disabled (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComH28nmSerdesAutoTuneIsEnabled
(
    GT_U8                    devNum,
    GT_UOPT                  portGroup,
    GT_UOPT                  serdesNum,
    GT_BOOL                  isTx,
    GT_BOOL                  *isEnable
)
{
    GT_U32 offset;
    GT_UREG_DATA data;

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                        SERDES_EXTERNAL_CONFIGURATION_3, &data, 0));

    offset = (isTx) ? 8 : 7;
    *isEnable = (data >> offset) & 0x1;  /* check if Rx/Tx is enabled */

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesAutoTuneStatusGet function
* @endinternal
*
* @brief   Per SERDES check Rx or Tx training status
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] isTx                     - Rx or Tx
*
* @param[out] status                   - Training  (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComH28nmSerdesAutoTuneStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                  portGroup,
    GT_UOPT                  serdesNum,
    GT_BOOL                 isTx,
    MV_HWS_AUTO_TUNE_STATUS *status
)
{
    GT_UREG_DATA data, offset;
    GT_BOOL isEnable;

    /* init status */
    *status = TUNE_NOT_COMPLITED;

    CHECK_STATUS(mvHwsComH28nmSerdesAutoTuneIsEnabled(devNum, portGroup, serdesNum, isTx, &isEnable));

    if(isEnable == GT_FALSE)
    {
        return GT_OK;
    }

    /* prepare status reg mask */
    offset = (isTx) ? 2 : 0; /* bit[3:2] for Tx, bit[1:0] for Rx */

    /* read status */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                        SERDES_EXTERNAL_STATUS_1, &data, 0));
    data = (data >> offset) & 0x3;

    if (data == 0x1)
    {
        *status = TUNE_PASS;
    }
    else if (data == 0x3)
    {
        *status = TUNE_FAIL;
    }

   return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesAutoTuneStatus function
* @endinternal
*
* @brief   Per SERDES check the Rx & Tx training status
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesAutoTuneStatus
(
    GT_U8   devNum,
    GT_UOPT  portGroup,
    GT_UOPT  serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
    GT_U16          timeOut;
    GT_U8           seqId;
    GT_UREG_DATA    data, tx2Data, tx3Data;
    GT_BOOL         isEnable;
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT txConfig;

    /* check if Serdes is in Power-up */
    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum, portGroup, serdesNum));

    /* Rx Status */
    if (rxStatus != NULL)
    {
        *rxStatus = TUNE_NOT_COMPLITED;

        CHECK_STATUS(mvHwsComH28nmSerdesAutoTuneIsEnabled(devNum, portGroup, serdesNum, GT_FALSE, &isEnable));

        if (isEnable == GT_TRUE)
        {
            timeOut = 500;

            while ((*rxStatus == TUNE_NOT_COMPLITED) && (timeOut > 0))
            {
                /* delay 1 ms */
                hwsOsExactDelayPtr(devNum, portGroup, 1);

                CHECK_STATUS(mvHwsComH28nmSerdesAutoTuneStatusGet(devNum, portGroup, serdesNum, GT_FALSE, rxStatus));
                timeOut--;
            }
        }
    }

    /* Tx Status */
    if (txStatus != NULL)
    {
        *txStatus = TUNE_NOT_COMPLITED;

        CHECK_STATUS(mvHwsComH28nmSerdesAutoTuneIsEnabled(devNum, portGroup, serdesNum, GT_TRUE, &isEnable));

        if (isEnable == GT_TRUE)
        {
            timeOut = 500;
            while ((*txStatus == TUNE_NOT_COMPLITED) && (timeOut > 0))
            {
                /* delay 1 ms */
                hwsOsExactDelayPtr(devNum, portGroup, 1);

                CHECK_STATUS(mvHwsComH28nmSerdesAutoTuneStatusGet(devNum, portGroup, serdesNum, GT_TRUE, txStatus));
                timeOut--;
            }
        }
    }

    if ((txStatus != NULL) && (*txStatus != TUNE_NOT_COMPLITED))
    {
        CHECK_STATUS(mvHwsComH28nmSerdesAutoTuneStart(devNum, portGroup, serdesNum, GT_FALSE, GT_FALSE));
    }

    if ((rxStatus != NULL) && (*rxStatus != TUNE_NOT_COMPLITED))
    {
        seqId = SERDES_RX_TRAINING_DISABLE_SEQ;
        CHECK_STATUS(mvCfgSerdesSeqExec(devNum, portGroup, serdesNum, seqId));
    }

    if ((txStatus != NULL) && (*txStatus == TUNE_PASS))
    {
        /* this point means that Tx training pass */
        /* get Manual Tx config parameters */
        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_Tx_Train_Result0, &tx2Data, 0));

        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_Tx_Train_Result1, &tx3Data, 0));

        hwsOsMemSetFuncPtr(&txConfig, 0, sizeof(txConfig));

        txConfig.txComphyH.txAmp     = U32_GET_FIELD(tx2Data, 11, 5);
        txConfig.txComphyH.txAmpAdj  = U32_GET_FIELD(tx2Data, 10, 1);
        txConfig.txComphyH.txAmpShft = U32_GET_FIELD(tx3Data, 7,  1);
        txConfig.txComphyH.txEmph0   = U32_GET_FIELD(tx2Data, 6,  4);
        txConfig.txComphyH.txEmph1   = U32_GET_FIELD(tx2Data, 2,  4);
        txConfig.txComphyH.slewCtrlEn  = 0;
        txConfig.txComphyH.slewRate    = 0;
        CHECK_STATUS(mvHwsComH28nmSerdesManualTxConfig(devNum, portGroup, serdesNum, &txConfig));

        /* update Tx Train Emph 0/1 enable */
        data = (U32_GET_FIELD(tx2Data, 0, 1) << 4);

        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_GEN1_SET2_Reg, data, (1 << 4)));
        data = (U32_GET_FIELD(tx2Data, 1, 1) << 11);

        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_GEN1_SET0_Reg, data, (1 << 11)));
    }

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesAutoTuneStatusShort function
* @endinternal
*
* @brief   Per SERDES check Rx or Tx training status
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      isTx      - Rx or Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesAutoTuneStatusShort
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
    if ((NULL == rxStatus) && (NULL == txStatus))
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));

    if (rxStatus != NULL)
    {
        CHECK_STATUS(mvHwsComH28nmSerdesAutoTuneStatusGet(devNum, portGroup, serdesNum, GT_FALSE, rxStatus));
    }

    if (txStatus != NULL)
    {
        CHECK_STATUS(mvHwsComH28nmSerdesAutoTuneStatusGet(devNum, portGroup, serdesNum, GT_TRUE, txStatus));
    }

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesAutoTuneResult function
* @endinternal
*
* @brief   Per SERDES return the adapted tuning results
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] results                  - the adapted tuning results.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesAutoTuneResult
(
    GT_U8                               devNum,
    GT_UOPT                             portGroup,
    GT_UOPT                             serdesNum,
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *tuneResults
)
{
  GT_UREG_DATA                      data, data1, data2, data3;
  GT_U32                            addr1, addr2;
  MV_HWS_COMPHY_H_AUTO_TUNE_RESULTS *results;
  if (tuneResults == NULL)
  {
    return GT_BAD_PTR;
  }
  CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));
  results = &tuneResults->comphyHResults;
  CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_FFE_Reg, &data, 0));
  results->ffeR = U32_GET_FIELD(data, 12, 3);
  results->ffeC = U32_GET_FIELD(data, 8, 4);

  CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_Data_Phase_Offset_Reg, &data, 0));
  results->sampler = U32_GET_FIELD(data, 9, 7);

  CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_Squelch_FFE_Reg, &data, 0));
  results->sqleuch = U32_GET_FIELD(data, 8, 5);

  CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_TX_Train_Preset_Reg, &data, 0x4));

  addr1 = (data == MV_HWS_COE_FROM_GEN_TABLE) ? COM_H_28NM_GEN1_SET0_Reg : COM_H_28NM_Tx_Train_Result0;
  addr2 = (data == MV_HWS_COE_FROM_GEN_TABLE) ? COM_H_28NM_GEN1_SET2_Reg : COM_H_28NM_Tx_Train_Result1;

  CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      addr1, &data1, 0));

  CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      addr2, &data2, 0));

  CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_GEN1_SET1_Reg, &data3, 0));

  /* if data == 4: then Tx params are taken from GEN table, else from Training results */
  results->txAmp     = (data == MV_HWS_COE_FROM_GEN_TABLE) ? U32_GET_FIELD(data1, 1,  5) : U32_GET_FIELD(data1, 11, 5);
  results->txAmpAdj  = (data == MV_HWS_COE_FROM_GEN_TABLE) ? U32_GET_FIELD(data1, 6,  1) : U32_GET_FIELD(data1, 10, 1);
  results->txEmph1   = (data == MV_HWS_COE_FROM_GEN_TABLE) ? U32_GET_FIELD(data1, 7,  4) : U32_GET_FIELD(data1, 2,  4);
  results->txEmph1En = (data == MV_HWS_COE_FROM_GEN_TABLE) ? U32_GET_FIELD(data1, 11, 1) : U32_GET_FIELD(data1, 1,  1);
  results->txAmpShft = (data == MV_HWS_COE_FROM_GEN_TABLE) ? U32_GET_FIELD(data3, 15, 1) : U32_GET_FIELD(data2, 7,  1);
  results->txEmph0   = (data == MV_HWS_COE_FROM_GEN_TABLE) ? U32_GET_FIELD(data2, 0,  4) : U32_GET_FIELD(data1, 6,  4);
  results->txEmph0En = (data == MV_HWS_COE_FROM_GEN_TABLE) ? U32_GET_FIELD(data2, 4,  1) : U32_GET_FIELD(data1, 0,  1);
  results->slewRate  = U32_GET_FIELD(data1, 12, 3);
  results->slewCtrlEn = U32_GET_FIELD(data1, 15, 1);

  CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_Adapt_DFE_Reg0, &data, 0));
  results->dfeVals[0] = U32_GET_FIELD(data, 10, 6);
  results->dfeVals[1] = U32_GET_FIELD(data, 4, 5);
  /* bit 9 is the sign */
  if ((data >> 9) & 1)
  {
    results->dfeVals[1] = results->dfeVals[1] * (-1);
  }
  results->dfeVals[3] = U32_GET_FIELD(data, 0, 3);
  /* bit 3 is the sign */
  if ((data >> 3) & 1)
  {
    results->dfeVals[3] = results->dfeVals[3] * (-1);
  }

  CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_Adapt_DFE_Reg1, &data, 0));
  results->dfeVals[2] = U32_GET_FIELD(data, 8, 4);
  /* bit 12 is the sign */
  if ((data >> 12) & 1)
  {
    results->dfeVals[2] = results->dfeVals[2] * (-1);
  }
  results->dfeVals[4] = U32_GET_FIELD(data, 4, 3);
  /* bit 7 is the sign */
  if ((data >> 7) & 1)
  {
    results->dfeVals[4] = results->dfeVals[4] * (-1);
  }

  results->dfeVals[5] = U32_GET_FIELD(data, 0, 3);
  /* bit 3 is the sign */
  if ((data >> 3) & 1)
  {
    results->dfeVals[5] = results->dfeVals[5] * (-1);
  }

  return GT_OK;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsComH28nmSerdesPpmConfig function
* @endinternal
*
* @brief   Per SERDES add ability to add/sub PPM from main baud rate.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesPpmConfig
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    MV_HWS_PPM_VALUE  ppmVal
)
{
  GT_UREG_DATA puTxIntp;
  GT_UREG_DATA txInitFoffset;

  CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));

  puTxIntp = 1;
  switch (ppmVal)
  {
  case Neg_3_TAPS:
    txInitFoffset = 0x3;
    break;
  case Neg_2_TAPS:
    txInitFoffset = 0x2;
    break;
  case Neg_1_TAPS:
    txInitFoffset = 0x1;
    break;
  case ZERO_TAPS:
    puTxIntp = 0;
    txInitFoffset = 0;
    break;
  case POS_1_TAPS:
    txInitFoffset = 1023;
    break;
  case POS_2_TAPS:
    txInitFoffset = 1022;
    break;
  case POS_3_TAPS:
    txInitFoffset = 1021;
    break;
  default:
    return GT_BAD_PARAM;
  }

  /*
  CHECK_STATUS(mvHwsComH28nmSerdesReset(devNum,portGroup,serdesNum, GT_FALSE, GT_TRUE, GT_FALSE));
  CHECK_STATUS(mvHwsComH28nmSerdesCoreReset(devNum,portGroup,serdesNum, UNRESET));
  */

  CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_PU_PLL_Reg, (puTxIntp << 11), (1 << 11)));

  CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_Trans_Freq_Reg, txInitFoffset, 0x3FF));

  CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_Trans_Freq_Reg, (1 << 10), (1 << 10)));

  /*
  CHECK_STATUS(mvHwsComH28nmSerdesReset(devNum,portGroup,serdesNum, GT_FALSE, GT_FALSE, GT_FALSE));
  */

  return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesPpmConfigGet function
* @endinternal
*
* @brief   Per SERDES add ability to add/sub PPM from main baud rate.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesPpmConfigGet
(
    GT_U8              devNum,
    GT_UOPT            portGroup,
    GT_UOPT            serdesNum,
    MV_HWS_PPM_VALUE   *ppmVal
)
{
    GT_UREG_DATA txInitFoffset;

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Trans_Freq_Reg, &txInitFoffset, 0x3FF));

    switch (txInitFoffset)
    {
    case 0x3:
        *ppmVal = Neg_3_TAPS;
        break;
    case 0x2:
        *ppmVal = Neg_2_TAPS;
        break;
    case 0x1:
        *ppmVal = Neg_1_TAPS;
        break;
    case 0:
        *ppmVal = ZERO_TAPS;
        break;
    case 1023:
        *ppmVal = POS_1_TAPS;
        break;
    case 1022:
        *ppmVal = POS_2_TAPS;
        break;
    case 1021:
        *ppmVal = POS_3_TAPS;
        break;
    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesAdaptPpm function
* @endinternal
*
* @brief   Set the PPM compensation on Serdes after Rx/TRX training in 10G and above.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - Serdes number
* @param[in] configPpm                - enable/disable the PPM configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesAdaptPpm
(
    GT_U8              devNum,
    GT_UOPT            portGroup,
    GT_UOPT            serdesNum,
    GT_BOOL            configPpm
)
{
    GT_U32 data;

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));

    /* disable the PPM value on Serdes before TX/RX training */
    if (configPpm == GT_FALSE)
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_RX_Reg1, 0x400, 0x7FF));
    }
    else /* set PPM value on Serdes after Stop TX/RX training */
    {
        /* Set DTL_FLOOP_EN=1 */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_DTL_Cntrl_Reg, (1 << 2), (1 << 2)));

        /* rxfoff_extraction_en=1 */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_Rx_Freq_Offset_Read_Reg, (1 << 12), (1 << 12)));

        /* rxfoff_rd_req=1 */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_Rx_Freq_Offset_Read_Reg, (1 << 11), (1 << 11)));

        hwsOsExactDelayPtr(devNum, portGroup, 1); /* 1ms delay */

        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_Rx_Freq_Offset_Read_Reg, &data, 0x3FF));

        /* rxfoff_rd_req=0 */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_Rx_Freq_Offset_Read_Reg, 0, (1 << 11)));

        /* DTL_FLOOP_EN=0 */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_DTL_Cntrl_Reg, 0, (1 << 2)));

        data = (data > 511) ? (data & 0x3FF) + 2 : (data & 0x3FF) - 2;
        data = data & 0x3FF;
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_RX_Reg1, 0x400 + data, 0x7FF));
    }

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesDfeConfig function
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
GT_STATUS mvHwsComH28nmSerdesDfeConfig
(
    GT_U8             devNum,
    GT_UOPT            portGroup,
    GT_UOPT            serdesNum,
    MV_HWS_DFE_MODE   dfeMode,
    GT_REG_DATA       *dfeCfg
)
{
  GT_UREG_DATA data, i;
  GT_UREG_DATA dfeVal[6];

  CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));

  switch(dfeMode)
  {
  case DFE_DISABLE:
      data = 0;
      CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                          SERDES_EXTERNAL_CONFIGURATION_2, data, (1 << 4)));

      CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                          COM_H_28NM_DFE_CFG_Reg, data, (1 << 12)));
      CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                          COM_H_28NM_DFE_Update_Reg, data, 0x3F));
      CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                          COM_H_28NM_GEN1_SET1_Reg, data, (1 << 10)));
    break;
  case DFE_ENABLE:
      data = (1 << 4);
      CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                          SERDES_EXTERNAL_CONFIGURATION_2, data, (1 << 4)));
      data = 0;
      CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                          COM_H_28NM_DFE_CFG_Reg, data, (1 << 12)));
      CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                          COM_H_28NM_DFE_Update_Reg, data, 0x3F));
      data = (1 << 10);
      CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                          COM_H_28NM_GEN1_SET1_Reg, data, (1 << 10)));
    break;
  case DFE_FREEZE:
      data = (1 << 12);
      CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                          COM_H_28NM_DFE_CFG_Reg, data, (1 << 12)));
    break;
  case DFE_FORCE:
    for (i = 0; i < 6; i++)
    {
      dfeVal[i] = (GT_UREG_DATA)dfeCfg[i];
    }
    if (dfeCfg[1] < 0)
    {
      dfeVal[1] = (1 << 6) + ABS(dfeCfg[1]);
    }
    if (dfeCfg[2] < 0)
    {
      dfeVal[2] = (1 << 5) + ABS(dfeCfg[2]);
    }
    if (dfeCfg[3] < 0)
    {
      dfeVal[3] = (1 << 4) + ABS(dfeCfg[3]);
    }
    if (dfeCfg[4] < 0)
    {
      dfeVal[4] = (1 << 4) + ABS(dfeCfg[4]);
    }
    if (dfeCfg[5] < 0)
    {
      dfeVal[5] = (1 << 4) + ABS(dfeCfg[5]);
    }
    data = (dfeVal[0] << 10) + (dfeVal[1] << 4) + dfeVal[3];
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_DFE_F0F2_Reg1, data, 0));
    data = (dfeVal[2] << 8) + (dfeVal[4] << 4) + dfeVal[5];
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_DFE_F3F5_Reg2, data, 0));
    data = 0x3F;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_DFE_Update_Reg, data, 0x3F));
    break;
  default:
    return GT_BAD_PARAM;
  }

  return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesDfeStatus function
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
GT_STATUS mvHwsComH28nmSerdesDfeStatus
(
    GT_U8             devNum,
    GT_UOPT            portGroup,
    GT_UOPT            serdesNum,
    GT_BOOL           *dfeLock,
    GT_REG_DATA       *dfeCfg,
    GT_REG_DATA       *f0Cfg,
    GT_REG_DATA       *savCfg
)
{
    GT_UREG_DATA data, i;
    GT_BOOL signBit;

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));

    if ((dfeCfg == NULL) || (f0Cfg == NULL) ||(savCfg == NULL) ||(dfeLock == NULL)){
        return GT_BAD_PARAM;
    }
    for (i = 0; (f0Cfg) && (i < 3); i++)
    {
        f0Cfg[i] = 0;
    }

    for (i = 0; (savCfg) && (i < 5); i++)
    {
        savCfg[i] = 0;
    }
    for (i = 0; (dfeCfg) && (i < 6); i++)
    {
        dfeCfg[i] = 0;
    }

    /* read basic DFE values */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Adapt_DFE_Reg0, &data, 0));
    dfeCfg[0] = (data >> 10) & 0x3F;
    /* bit 9 is the sign */
    if ((data >> 9) & 1)
    {
        dfeCfg[1] = ((data >> 4) & 0x1F) * (-1);
    }
    else
    {
        dfeCfg[1] = ((data >> 4) & 0x1F);
    }
    /* bit 3 is the sign */
    if ((data >> 3) & 1)
    {
        dfeCfg[3] = (data & 7) * (-1);
    }
    else
    {
        dfeCfg[3] = (data & 7);
    }

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Adapt_DFE_Reg1, &data, 0));
    /* bit 12 is the sign */
    if ((data >> 12) & 1)
    {
        dfeCfg[2] = ((data >> 8) & 0xF) * (-1);
    }
    else
    {
        dfeCfg[2] = ((data >> 8) & 0xF);
    }
    /* bit 7 is the sign */
    if ((data >> 7) & 1)
    {
        dfeCfg[4] = ((data >> 4) & 7) * (-1);
    }
    else
    {
        dfeCfg[4] = ((data >> 4) & 7);
    }
    /* bit 3 is the sign */
    if ((data >> 3) & 1)
    {
        dfeCfg[5] = (data & 7) * (-1);
    }
    else
    {
        dfeCfg[5] = (data & 7);
    }

    /* read DFE lock status */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_DFE_Res_Reg, &data, 0));
    if ((data >> 12) & 1)
    {
        *dfeLock = GT_TRUE;
    }
    else
    {
        *dfeLock = GT_FALSE;
    }

    /* go to page 1 */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Saved_DFE_Values_Reg0, &data, 0));
    f0Cfg[0] = U32_GET_FIELD(data, 10, 6); /* F0a*/
    f0Cfg[1] = U32_GET_FIELD(data, 4, 6);  /* F0b*/

    signBit = U32_GET_FIELD(data, 3, 1);
    savCfg[2] = U32_GET_FIELD(data, 0, 3); /* Sav_f3 */
    if (signBit == GT_TRUE)
    {
        savCfg[2] |= 0x80000000;
    }

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Saved_DFE_Values_Reg1, &data, 0));
    f0Cfg[2] = U32_GET_FIELD(data, 10, 6); /* F0d*/

    signBit = U32_GET_FIELD(data, 9, 1);
    savCfg[0] = U32_GET_FIELD(data, 4, 5); /* Sav_f1 */
    if (signBit == GT_TRUE)
    {
      savCfg[0] |= 0x80000000;
    }

    signBit = U32_GET_FIELD(data, 3, 1);
    savCfg[3] = U32_GET_FIELD(data, 0, 3); /* Sav_f4 */
    if (signBit == GT_TRUE)
    {
      savCfg[3] |= 0x80000000;
    }

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Saved_DFE_Values_Reg2, &data, 0));
    /* Sav_f2 */
    signBit = U32_GET_FIELD(data, 8, 1);
    savCfg[1] = U32_GET_FIELD(data, 4, 4);
    if (signBit == GT_TRUE)
    {
      savCfg[1] |= 0x80000000;
    }

    /* Sav_f5 */
    signBit = U32_GET_FIELD(data, 3, 1);
    savCfg[4] = U32_GET_FIELD(data, 0, 3);
    if (signBit == GT_TRUE)
    {
      savCfg[4] |= 0x80000000;
    }

    return GT_OK;
}
#endif

#ifndef CO_CPU_RUN
/* WA to improve signal detection */
static GT_STATUS mvHwsComHSqlchImp(GT_U8 devNum, GT_UOPT portGroup, GT_UOPT serdesNum)
{
    GT_UREG_DATA data;
    GT_UREG_DATA sq_cmpth_cal, sq_pkcm_cal, sq_thres_cal;

    /* write to 0x1E0 bits [15:14] */
    data = (1 << 14);
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Squelch_Detect_Reg, data, (3 << 14)));
    /* store sq_cmpth_cal */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Squelch_Detect_Reg, &data, 0));
    sq_cmpth_cal = (data >> 8) & 0x3F;

    data = (2 << 14);
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Squelch_Detect_Reg, data, (3 << 14)));
    /* store sq_pkcm_cal */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Squelch_Detect_Reg, &data, 0));
    sq_pkcm_cal = (data >> 8) & 0x3F;

    data = (3 << 14);
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Squelch_Detect_Reg, data, (3 << 14)));
    /* store sq_cmpth_cal */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Squelch_Detect_Reg, &data, 0));
    sq_thres_cal = (data >> 8) & 0x3F;

    /* write results to external SQ */
    if (sq_cmpth_cal < 2)
        sq_cmpth_cal = 5;
    else
        sq_cmpth_cal= sq_cmpth_cal/2;

    data = (1 << 15) + (sq_cmpth_cal << 10) + (sq_pkcm_cal << 4) + (sq_thres_cal & 0xF);
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Ext_Squelch_Calibr_Reg, data, 0));
    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesManualRxConfig function
* @endinternal
*
* @brief   Per SERDES configure the RX parameters: squelch Threshold, FFE and DFE
*         operation
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical lane number
* @param[in] portTuningMode           - lane tuning mode (short / long)
* @param[in] sqlch                    - Squelch threshold
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesManualRxConfig
(
    IN  GT_U8                               devNum,
    IN  GT_UOPT                             portGroup,
    IN  GT_UOPT                             serdesNum,
    IN  MV_HWS_SERDES_RX_CONFIG_DATA_UNT    *rxConfigPtr
)
{
  GT_UREG_DATA data;
  GT_32  curVal, tmpAlig;
  GT_UREG_DATA sqlchImprove;


  CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum, portGroup, serdesNum));

  if (rxConfigPtr->rxComphyH.portTuningMode == StaticLongReach)
  {
      /* Long Reach - set highest DFE resolution */
      CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                          COM_H_28NM_DFE_Res_Reg, 0xD047, 0));
  }
  else
  {
      /* Short Reach - set almost highest DFE resolution */
      CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                          COM_H_28NM_DFE_Res_Reg, 0xD147, 0));
  }

  /* read bit 15 from 0x1BC, if == 1, set to 0 */
  CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_Ext_Squelch_Calibr_Reg, &data, 0));
  sqlchImprove = (data >> 15) & 1;
  if (sqlchImprove)
  {
      data = 0;
      CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                          COM_H_28NM_Ext_Squelch_Calibr_Reg, data, (1 << 15)));
  }

  data = (1 << 7);
  CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_Squelch_FFE_Reg, data, data));
  data = (rxConfigPtr->rxComphyH.ffeRes << 4);
  CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_Squelch_FFE_Reg, data, (7 << 4)));
  CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_Squelch_FFE_Reg, rxConfigPtr->rxComphyH.ffeCap, 0xF));
  data = (rxConfigPtr->rxComphyH.sqlch << 8);
  CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_Squelch_FFE_Reg, data, 0x1F00));

  /* if bit 15 from 0x1BC was == 1, call to WA */
  if (sqlchImprove)
  {
      hwsOsTimerWkFuncPtr(1);
      mvHwsComHSqlchImp(devNum,portGroup,serdesNum);
  }

  data = ((rxConfigPtr->rxComphyH.dfeEn  ? 1:0) << 4);
  CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                      SERDES_EXTERNAL_CONFIGURATION_2, data, (1 << 4)));

  /*  set bit 7 of COM_H_28NM_OS_PH_OFFSET_Reg os_ph_force*/
  data = (1 << 7);
  CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_OFFSET_Phase_Reg, data, data));
  /* config align90 */
  /* read current align90 value */
  CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_Data_Phase_Offset_Reg, &data, 0));
  curVal = (data >> 9) & 0x7F;
  tmpAlig = rxConfigPtr->rxComphyH.alig;
  /* move to align90 value with step = 4 */
  if (((curVal - 4) > tmpAlig) || ((GT_U32)(curVal + 4) < rxConfigPtr->rxComphyH.alig))
  {
    while((curVal - 4) > tmpAlig)
    {
      curVal -= 4;
      data = (GT_UREG_DATA)curVal;
      CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                          COM_H_28NM_OFFSET_Phase_Reg, data, 0x007F));
      data = (1 << 8);
      CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                          COM_H_28NM_OFFSET_Phase_Reg, data, data));
      data = 0;
      CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                          COM_H_28NM_OFFSET_Phase_Reg, data, data));
    }
    while((GT_U32)(curVal + 4) < rxConfigPtr->rxComphyH.alig)
    {
        curVal += 4;
        data = (GT_UREG_DATA)curVal;
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_OFFSET_Phase_Reg, data, 0x007F));
        data = (1 << 8);
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_OFFSET_Phase_Reg, data, data));
        data = 0;
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                            COM_H_28NM_OFFSET_Phase_Reg, data, data));
    }
  }
  CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_OFFSET_Phase_Reg, rxConfigPtr->rxComphyH.alig, 0x007F));
  data = (1 << 8);
  CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_OFFSET_Phase_Reg, data, data));
  data = 0;
  CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                      COM_H_28NM_OFFSET_Phase_Reg, data, data));

  return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesManualRxConfigGet function
* @endinternal
*
* @brief   Per SERDES returns the RX parameters configurations
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical lane number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesManualRxConfigGet
(
    IN  GT_U8                             devNum,
    IN  GT_UOPT                           portGroup,
    IN  GT_UOPT                           serdesNum,
    OUT MV_HWS_SERDES_RX_CONFIG_DATA_UNT  *configParams
)
{
    GT_U32 data;

    if (configParams == NULL)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum, portGroup, serdesNum));

    /* read port tune mode */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        0x444, &data, (1 << 8)));
    configParams->rxComphyH.portTuningMode  = (data) ? StaticShortReach : StaticLongReach;

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_FFE_Reg, &data, 0));
    configParams->rxComphyH.ffeRes = (data >> 12) & 7;
    configParams->rxComphyH.ffeCap = (data  >> 8) & 0xF;

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Squelch_FFE_Reg, &data, 0));
    configParams->rxComphyH.sqlch = (data >> 8) & 0xF;

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                        SERDES_EXTERNAL_CONFIGURATION_2, &data, 0));
    configParams->rxComphyH.dfeEn = (data >> 4) & 0x1;

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_Data_Phase_Offset_Reg, &data, 0));
    configParams->rxComphyH.alig = (data >> 9) & 0x7F;

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesLoopback function
* @endinternal
*
* @brief   Perform an internal loopback (SERDES loopback) for debug/BIST use.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      serdesType - type of serdes (LP/COM_PHY...)
* @param[in] lbMode                   - loopback mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesLoopback
(
    GT_U8             devNum,
    GT_UOPT            portGroup,
    GT_UOPT            serdesNum,
    MV_HWS_SERDES_LB  lbMode
)
{
  GT_U8     seqId;

  CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));
  switch (lbMode)
  {
  case SERDES_LP_DISABLE:
    seqId = SERDES_SD_LPBK_NORMAL_SEQ;
    break;
  case SERDES_LP_AN_TX_RX:
    seqId = SERDES_SD_ANA_TX_2_RX_SEQ;
    break;

  case SERDES_LP_DIG_TX_RX:
    seqId = SERDES_SD_DIG_TX_2_RX_SEQ;
    break;
  case SERDES_LP_DIG_RX_TX:
    seqId = SERDES_SD_DIG_RX_2_TX_SEQ;
    break;
  default:
    return GT_NOT_SUPPORTED;
    break;
  }
  CHECK_STATUS(mvCfgSerdesSeqExec(devNum, portGroup, serdesNum, seqId));

  return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesLoopbackGet function
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
GT_STATUS mvHwsComH28nmSerdesLoopbackGet
(
    GT_U8             devNum,
    GT_UOPT            portGroup,
    GT_UOPT            serdesNum,
    MV_HWS_SERDES_LB  *lbModePtr
)
{
    GT_UREG_DATA data;

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_SEL_BITS_Reg, &data, 0));
    data = (data >> 12) & 0xF;

    switch(data)
    {
        case 2:
            *lbModePtr = SERDES_LP_AN_TX_RX;
            break;

        case 4:
            *lbModePtr = SERDES_LP_DIG_TX_RX;
            break;

        case 9:
            CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                                COM_H_28NM_PT_PATTERN_SEL_Reg, &data, (0xF0 | 0x8000)));
            if((data & 0xD0) && (data & 0x8000))
            {
                *lbModePtr = SERDES_LP_DIG_RX_TX;
            }
            else
            {
                return GT_NOT_SUPPORTED;
            }
            break;

        case 0:
                *lbModePtr = SERDES_LP_DISABLE;
            break;

        default:
            return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesTxEnable function
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
GT_STATUS mvHwsComH28nmSerdesTxEnable
(
    GT_U8       devNum,
    GT_UOPT      portGroup,
    GT_UOPT      serdesNum,
    GT_BOOL     enable
)
{
    GT_UREG_DATA data;

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));

    data = ((!enable) << 2);
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                        SERDES_EXTERNAL_CONFIGURATION_1, data, (1 << 2)));
    data = ((!enable) << 8);
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_PHY_Isolate_Reg, data, (1 << 8)));

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesTxEnableGet function
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
GT_STATUS mvHwsComH28nmSerdesTxEnableGet
(
    GT_U8   devNum,
    GT_UOPT  portGroup,
    GT_UOPT  serdesNum,
    GT_BOOL *enablePtr
)
{
    GT_UREG_DATA data;

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum,
                                        COM_H_28NM_PHY_Isolate_Reg, &data, 0));

    *enablePtr = ((data >> 8) & 1) ? GT_FALSE : GT_TRUE;

    return GT_OK;
}
#endif

/**
* @internal mvHwsComH28nmSerdesFixAlign90Start function
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
GT_STATUS mvHwsComH28nmSerdesFixAlign90Start
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_ALIGN90_PARAMS   *params
)
{
    GT_U32 align1;
    GT_U32 adapted_ffe_R, adapted_ffe_C;
    GT_U32 tmp, data;

    if (params == NULL)
    {
        return GT_BAD_PARAM;
    }

    /* store strat value of Align90 for feauter calculation */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_Data_Phase_Offset_Reg, &align1, 0));
    params->startAlign90 = align1 >> 9;

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_FFE_Reg, &data, 0));
    adapted_ffe_R = (data >> 12) & 7;
    adapted_ffe_C = (data >> 8) & 0xF;

#if 0
    /* start PRBS if not started yet */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_REV2_PT_PATTERN_SEL_Reg, &data, 0));
    if (!(data & 0x8000)) /* bit 15 - PHY test started */
    {
        /* start PRBS */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_REV2_Pattern_Reg, 0xC4, 0));
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_REV2_PT_PATTERN_SEL_Reg, 0x80E0, 0));
        /* set flag - PRBS startrd by HWS in MetalFix Reg bit 15 */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SD_METAL_FIX, 0x8000, 0x8000));
    }
#endif

    /* force C & R */
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x18, ((1<<7) + (adapted_ffe_R<<4) + adapted_ffe_C), 0xff));

    /* store os_delta_max_2 */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x320, &data, 0));
    params->osDeltaMax = data & 0x1f;
    /* os_delta_max_2 = 31 */
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x320, 0x1f, 0x1f));

    /* store RX training control register 1 */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x238, &tmp, 0));
    params->rxTrainingCfg = tmp;

    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x238, 0, 0));

    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x330, 0, (1 << 7)));

    /* clear bit 11 of register 0x284 */
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x284, 0x8078, 0xC87C));

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, &data, 0));
    data = (data >> 7) & 0xF;
    /* Check if baud rate is below 7.5G */
    if (data < 0xC)
    {
        /* disable DFE pattern protect in these frequencies */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_2, (1 << 5), (1 << 5)));
        hwsOsExactDelayPtr(devNum, portGroup, 1);
    }
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x1AC, (timer_dfe2 << 3), 7 << 3));
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x1AC, (timer_dfe1 << 6), 7 << 6));

    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x340, timer_f0d2, 7));
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x340, (timer_f0b2 << 3), (7 << 3)));
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x340, (timer_edge2 << 6), (7 << 6)));
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x27C, timer_f0d1, 7));
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x27C, (timer_f0b1 << 3), (7 << 3)));
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x27C, (timer_edge1 << 6), (7 << 6)));
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x27C, (timer_dfe3 << 9), (7 << 9)));

     /* start PRBS if not started yet */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_PT_PATTERN_SEL_Reg, &data, 0));
    if (!(data & 0x8000)) /* bit 15 - PHY test started */
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_Pattern_Reg, 0xC4, 0));
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_PT_PATTERN_SEL_Reg, 0x80E0, 0));
    }

    /* start Rx training */
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_3, 1<<7, 1<<7));

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesFixAlign90Status function
* @endinternal
*
* @brief   Return fix Align90 process current status on current SERDES.
*         Can be run after create port and start Align90.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesFixAlign90Status
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *trainingStatus
)
{
    GT_U32 data;

    if (trainingStatus == NULL)
    {
        return GT_BAD_PARAM;
    }
    *trainingStatus = TUNE_NOT_COMPLITED;

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_STATUS_1, &data, 0));
    if (data & 1)
    {
        *trainingStatus = ((data >> 1) & 1) ? TUNE_FAIL : TUNE_PASS;
    }

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesForceAlign90Set function
* @endinternal
*
* @brief   Perform force Align90 value and pulse after updating the Align90
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] forceAlign90             - value of Align90
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsComH28nmSerdesForceAlign90Set
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    GT_U32  forceAlign90
)
{
    /* Set fource Align90 value and set os_ph_force (bit #7) */
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_REV2_OS_PH_OFFSET_Reg, 0x80+forceAlign90, 0xFF));

    /* update Align90 */
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_REV2_OS_PH_OFFSET_Reg, 1<<8, 1<<8));

    /* pulse os_ph_valid */
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_REV2_OS_PH_OFFSET_Reg, 0, 1<<8));

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesFixAlign90Stop function
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
GT_STATUS mvHwsComH28nmSerdesFixAlign90Stop
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 serdesNum,
    MV_HWS_ALIGN90_PARAMS *params,
    GT_BOOL fixAlignPass
)
{
    GT_U32 align;

    if (params == NULL)
    {
        return GT_BAD_PARAM;
    }

    /* clear Rx training */
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_3, 0, 1<<7));

#if 0
    /* stop PRBS if started by HWS */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SD_METAL_FIX, &data, 0));

    if (data & 0x8000) /* bit 15 - PHY test started */
    {
        CHECK_STATUS(genRegisterSet(devNum, portGroup, baseAddr+COM_H_REV2_Pattern_Reg, 0, 0));
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_PT_PATTERN_SEL_Reg, 0x80E0, 0));
        CHECK_STATUS(genRegisterSet(devNum, portGroup, baseAddr+COM_H_REV2_PT_PATTERN_SEL_Reg, 0, 0));
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_PT_PATTERN_SEL_Reg, 0x80E0, 0));
        CHECK_STATUS(genRegisterSet(devNum, portGroup, extBaseAddr+SD_METAL_FIX, 0, 0x8000));
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_PT_PATTERN_SEL_Reg, 0x80E0, 0));
    }
#endif

    /* put back read register */
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x238, params->rxTrainingCfg, 0));
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, 0x320, params->osDeltaMax, 0x1f));

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_REV2_OS_PH_Reg, &align, 0));
    if (!fixAlignPass)
    {
        align = params->startAlign90;
    }
    else
    {
        align = (params->startAlign90 + (align >> 9))/2;
    }

    /* Set force Align90 value */
    CHECK_STATUS(mvHwsComH28nmSerdesForceAlign90Set(devNum, portGroup, serdesNum, align));

    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_REV2_Pattern_Reg, 0, 0));

    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_REV2_PT_PATTERN_SEL_Reg, 0, 0));

    return GT_OK;
}


/**
* @internal mvHwsComH28nmSerdesTxIfSelect function
* @endinternal
*
* @brief   tx interface select.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      enable    - whether to enable or disable Tx.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesTxIfSelect
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    GT_UREG_DATA        serdesTxIfNum
)
{

    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_3, serdesTxIfNum, 7));

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesTxIfSelectGet function
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
GT_STATUS mvHwsComH28nmSerdesTxIfSelectGet
(
    GT_U8      devNum,
    GT_U32     portGroup,
    GT_U32     serdesNum,
    GT_U32     *serdesTxIfNum
)
{
    GT_UREG_DATA data;

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                    SERDES_EXTERNAL_CONFIGURATION_3, &data, 0x7));

    if (data > 3)
    {
        return GT_BAD_PARAM;
    }

    *serdesTxIfNum = data;

    return GT_OK;
}

/**
* @internal mvHwsComH28nmDfeCheck function
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
GT_STATUS mvHwsComH28nmDfeCheck
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum
)
{
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesCalibrationStatus function
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
GT_STATUS mvHwsComH28nmSerdesCalibrationStatus
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      serdesNum,
    MV_HWS_CALIBRATION_RESULTS  *results
)
{
    GT_U32 data;
    GT_U32 vddCal[7];
    GT_U32 i;

    CHECK_STATUS(mvHwsComH28nmCheckSerdesAccess(devNum,portGroup,serdesNum));

    /* Cal Done & Speed PLL */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_KVCO_CAL_Reg, &data, 0));
    results->calDone = (data >> 14) & 0x1;
    results->speedPll = data  & 0xF;

    /* Tx Imp Cal */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_IMP_CAL_Reg, &data, 0));
    results->txImpCal = (data >> 4) & 0xF;

    /* Rx Imp Cal */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_IMP_TXSsc_Reg, &data, 0));
    results->rxImpCal = (data >> 8) & 0xF;

    /* Process Cal */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_Cal_Reg_11, &data, 0));
    results->ProcessCal = (data >> 8) & 0xF;

    /* Sell V data */
    for (i=1; i<=7; i++)
    {
        /* Select VDD calibration */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_Cal_Reg_12, (i << 5), (0xF << 5)));
        /* Read VDD calibration result */
        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_Cal_Reg_10, &vddCal[i-1], 0x1F));
    }

        /* FFE Done - not relevant for Serdes 28nm */
        results->ffeDone = GT_FALSE;

    results->sellvTxClk     = vddCal[0];
    results->sellvTxData    = vddCal[1];
    results->sellvTxIntp    = vddCal[2];
    results->sellvTxDrv     = vddCal[3];
    results->sellvTxDig     = vddCal[4];
    results->sellvRxSample  = vddCal[6];

    /* Sampler 0-6 */
    for (i = 0; i < 7; i++)
    {
        /* Select Sampler */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_Cal_Reg_7, (i << 8), (0x7 << 8)));
        /* Read Sampler */
        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_Cal_Reg_15, (GT_U32*)&results->ffeCal[i], 0x7F));
    }

    /* Cal OS Phase */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_Cal_Reg_11, (GT_U32*)&results->ffeCal[7], 0x7F));

    return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesSamplerOffsetSet function
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
GT_STATUS mvHwsComH28nmSerdesSamplerOffsetSet
(
    GT_U8        devNum,
    GT_UOPT      portGroup,
    GT_UOPT      serdesNum,
    GT_UREG_DATA  samplSel,
    GT_UREG_DATA  value
)
{
    /* write the sampler select */
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_SamplerCalSelect, (samplSel << 8), (0x7 << 8)));

    /* write the value */
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_SamplerCalRd, (1 << 15), (1 << 15) ));
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_SamplerCalRd, (value << 7), (0x7F << 7) ));
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_SamplerCalSelect , (1 << 13), (1 << 13) ));

    /* delay 1ms*/
    hwsOsExactDelayPtr(devNum, portGroup, 1);

    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_SamplerCalSelect, 0, (1 << 13) ));
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, INTERNAL_REG, serdesNum, COM_H_28NM_SamplerCalRd, 0, (1 << 15) ));

    return GT_OK;

}

#ifndef CO_CPU_RUN
static GT_STATUS mvHwsComH28nmSerdesSamplerSmooth
(
    GT_U32      errorThreshold,
    GT_U32      *errorArr
)
{
   GT_U32 i;

   for( i = 0; i < MV_HWS_SERDES_SCAN_LENGTH ; i++ )
   {
      errorArr[i] = errorArr[i] > errorThreshold ? errorArr[i] : 0;
   }

   return GT_OK;
}

static GT_STATUS mvHwsComH28nmSerdesSamplerFindMaxWinMarkers
(
    GT_U32      *errorArr,
    GT_U32      *leftMarker,
    GT_U32      *rightMarker
)
{
   GT_U32 i;
   GT_U32 maxWinSize = 0, tempWinSize = 0;
   GT_U32 left = 0, tempLeft = 0;
   GT_U32 right = 0;
   GT_BOOL counting = GT_FALSE;

   for(i = 0; i < MV_HWS_SERDES_SCAN_WIN_LENGTH ; i++)
   {
      if(errorArr[ i ] == 0)
      {
         if(counting == GT_TRUE)
         {
            /* already during count */
            tempWinSize++;
         }
         else
         {
            /* start counting window size */
            counting = GT_TRUE;
            tempLeft = i;
            tempWinSize = 1;
         }
      }

      if((( errorArr[i] != 0 ) && (counting == GT_TRUE)) ||
         (( errorArr[i] == 0 ) && (i == (MV_HWS_SERDES_SCAN_WIN_LENGTH - 1))))
      {
         /* stop counting */
         counting = GT_FALSE;
         if(tempWinSize > maxWinSize)
         {
            left = tempLeft;
            right = i - 1;
            maxWinSize = tempWinSize;
         }
     }
   }

   *leftMarker = left;
   *rightMarker = right;

   return GT_OK;
}

/**
* @internal mvHwsComH28nmSerdesScanSampler function
* @endinternal
*
* @param[in] devNum
* @param[in] portGroup
* @param[in] serdesNum
*                                      OUTPUT
*                                      None.
*/
GT_STATUS mvHwsComH28nmSerdesScanSampler
(
    GT_U8     devNum,
    GT_U8     portGroup,
    GT_U32    serdesNum,
    GT_U32    waitTime,
    GT_U32    samplerSelect,
    GT_U32    errorThreshold,
    MV_HWS_SAMPLER_RESULTS *samplerResults
)
{
    GT_U32 errorArr[MV_HWS_SERDES_SCAN_LENGTH];
    GT_U32 i;
    GT_U32 left = 0;
    GT_U32 right = 0;
    GT_U32 midPos;
    GT_U32 midNeg;
    GT_32  iBestVal;

    for(i = 0; i < MV_HWS_SERDES_SCAN_LENGTH ; i++)
    {
        CHECK_STATUS(mvHwsComH28nmSerdesSamplerOffsetSet(devNum, portGroup, serdesNum, samplerSelect, i));

        CHECK_STATUS(hwsSerdesRegSetFuncPtr( devNum, portGroup,INTERNAL_REG, serdesNum, COM_H_28NM_SEL_BITS_Reg, ( 1 << 11 ), ( 1 << 11 )));

        CHECK_STATUS(hwsSerdesRegSetFuncPtr( devNum, portGroup,INTERNAL_REG, serdesNum, COM_H_28NM_SEL_BITS_Reg, 0, ( 1 << 11 )));

        /* delay */
        hwsOsExactDelayPtr(devNum, devNum, waitTime);

        /* read error counter */
        CHECK_STATUS(hwsSerdesRegGetFuncPtr( devNum, portGroup,INTERNAL_REG, serdesNum, COM_H_28NM_ERROR_Cntr_LSB_Reg, &errorArr[i], 0));
    }

    CHECK_STATUS(mvHwsComH28nmSerdesSamplerSmooth(errorThreshold, errorArr));

    /* scan low half */
    left = right = 0;
    CHECK_STATUS(mvHwsComH28nmSerdesSamplerFindMaxWinMarkers(errorArr, &left, &right));

    midPos = ( right + left ) / 2;

    samplerResults->positiveVal = midPos;

    /* scan high half */
    left = right = 0;
    CHECK_STATUS(mvHwsComH28nmSerdesSamplerFindMaxWinMarkers(&errorArr[MV_HWS_SERDES_SCAN_WIN_LENGTH], &left, &right));

    midNeg = ( ( right + left ) / 2 ) + MV_HWS_SERDES_SCAN_WIN_LENGTH;

    samplerResults->negativeVal = midNeg;

    iBestVal = midPos + ( 64 - midNeg );
    samplerResults->bestVal = ( iBestVal ) > 0 ? (GT_U32)iBestVal : (GT_U32)(MV_HWS_SERDES_SCAN_WIN_LENGTH - iBestVal);

    CHECK_STATUS(mvHwsComH28nmSerdesSamplerOffsetSet(devNum, portGroup, serdesNum, samplerSelect, samplerResults->bestVal));

    return GT_OK;
}


#endif

/*****************************************************************************************/

/**
* @internal mvHwsComH28nmIfInit function
* @endinternal
*
* @brief   Init Com_H serdes configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmIfInit(MV_HWS_SERDES_FUNC_PTRS **funcPtrArray)
{

    if (hwsInitDone == GT_FALSE)
    {
        /* init sequences DB */
        CHECK_STATUS(hwsComPhyH28nmSeqInit());
    }

    /* new implementation */
    if(!funcPtrArray[COM_PHY_28NM])
    {
        funcPtrArray[COM_PHY_28NM] = (MV_HWS_SERDES_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_SERDES_FUNC_PTRS));
        if(!funcPtrArray[COM_PHY_28NM])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[COM_PHY_28NM], 0, sizeof(MV_HWS_SERDES_FUNC_PTRS));
    }
    funcPtrArray[COM_PHY_28NM]->serdesPowerCntrlFunc         = mvHwsComH28nmSerdesPowerCtrl;
    funcPtrArray[COM_PHY_28NM]->serdesManualTxCfgFunc        = mvHwsComH28nmSerdesManualTxConfig;
    funcPtrArray[COM_PHY_28NM]->serdesAutoTuneCfgFunc        = mvHwsComH28nmSerdesAutoTuneCfg;
    funcPtrArray[COM_PHY_28NM]->serdesRxAutoTuneStartFunc    = mvHwsComH28nmSerdesRxAutoTuneStart;
    funcPtrArray[COM_PHY_28NM]->serdesTxAutoTuneStartFunc    = mvHwsComH28nmSerdesTxAutoTuneStart;
    funcPtrArray[COM_PHY_28NM]->serdesAutoTuneStartFunc      = mvHwsComH28nmSerdesAutoTuneStart;
    funcPtrArray[COM_PHY_28NM]->serdesAutoTuneResultFunc     = mvHwsComH28nmSerdesAutoTuneResult;
    funcPtrArray[COM_PHY_28NM]->serdesAutoTuneStatusFunc     = mvHwsComH28nmSerdesAutoTuneStatus;
    funcPtrArray[COM_PHY_28NM]->serdesAutoTuneStatusShortFunc= mvHwsComH28nmSerdesAutoTuneStatusShort;
    funcPtrArray[COM_PHY_28NM]->serdesResetFunc              = mvHwsComH28nmSerdesReset;
    funcPtrArray[COM_PHY_28NM]->serdesDigitalReset           = mvHwsComH28nmSerdesDigitalReset;
    funcPtrArray[COM_PHY_28NM]->serdesCoreReset              = mvHwsComH28nmSerdesCoreReset;
    funcPtrArray[COM_PHY_28NM]->serdesFixAlign90Start        = mvHwsComH28nmSerdesFixAlign90Start;
    funcPtrArray[COM_PHY_28NM]->serdesFixAlign90Stop         = mvHwsComH28nmSerdesFixAlign90Stop;
    funcPtrArray[COM_PHY_28NM]->serdesFixAlign90Status       = mvHwsComH28nmSerdesFixAlign90Status;
#ifndef CO_CPU_RUN
    funcPtrArray[COM_PHY_28NM]->serdesSeqGetFunc             = hwsComPhy28nmSeqGet;
    funcPtrArray[COM_PHY_28NM]->serdesDfeCfgFunc             = mvHwsComH28nmSerdesDfeConfig;
    funcPtrArray[COM_PHY_28NM]->serdesManualRxCfgFunc        = mvHwsComH28nmSerdesManualRxConfig;
    funcPtrArray[COM_PHY_28NM]->serdesLoopbackCfgFunc        = mvHwsComH28nmSerdesLoopback;
    funcPtrArray[COM_PHY_28NM]->serdesLoopbackGetFunc        = mvHwsComH28nmSerdesLoopbackGet;
    funcPtrArray[COM_PHY_28NM]->serdesPpmCfgFunc             = mvHwsComH28nmSerdesPpmConfig;
    funcPtrArray[COM_PHY_28NM]->serdesPpmGetFunc             = mvHwsComH28nmSerdesPpmConfigGet;
    funcPtrArray[COM_PHY_28NM]->serdesTestGenFunc            = mvHwsComH28nmSerdesTestGen;
    funcPtrArray[COM_PHY_28NM]->serdesTestGenGetFunc         = mvHwsComH28nmSerdesTestGenGet;
    funcPtrArray[COM_PHY_28NM]->serdesTypeGetFunc            = mvHws28nmSerdesTypeGetFunc;
    funcPtrArray[COM_PHY_28NM]->serdesTestGenStatusFunc      = mvHwsComH28nmSerdesTestGenStatus;
    funcPtrArray[COM_PHY_28NM]->serdesEomGetFunc             = mvHwsComH28nmSerdesEOMGet;
    funcPtrArray[COM_PHY_28NM]->serdesDfeStatusExtFunc       = mvHwsComH28nmSerdesDfeStatus;
    funcPtrArray[COM_PHY_28NM]->serdesPolarityCfgFunc        = mvHwsComH28nmSerdesPolarityConfig;
    funcPtrArray[COM_PHY_28NM]->serdesPolarityCfgGetFunc     = mvHwsComH28nmSerdesPolarityConfigGet;
    funcPtrArray[COM_PHY_28NM]->serdesCalibrationStatusFunc  = mvHwsComH28nmSerdesCalibrationStatus;
    funcPtrArray[COM_PHY_28NM]->serdesTxEnableFunc           = mvHwsComH28nmSerdesTxEnable;
    funcPtrArray[COM_PHY_28NM]->serdesTxEnableGetFunc        = mvHwsComH28nmSerdesTxEnableGet;
    funcPtrArray[COM_PHY_28NM]->serdesTxIfSelectFunc         = mvHwsComH28nmSerdesTxIfSelect;
    funcPtrArray[COM_PHY_28NM]->serdesTxIfSelectGetFunc      = mvHwsComH28nmSerdesTxIfSelectGet;
    funcPtrArray[COM_PHY_28NM]->serdesAcTermCfgFunc          = mvHwsComH28nmSerdesAcTerminationCfg;
    funcPtrArray[COM_PHY_28NM]->serdesDfeCheckFunc           = mvHwsComH28nmDfeCheck;
    funcPtrArray[COM_PHY_28NM]->serdesManualRxCfgGetFunc     = mvHwsComH28nmSerdesManualRxConfigGet;
    funcPtrArray[COM_PHY_28NM]->serdesScanSamplerFunc        = mvHwsComH28nmSerdesScanSampler;

#endif

    hwsInitDone = GT_TRUE;

    return GT_OK;
}


