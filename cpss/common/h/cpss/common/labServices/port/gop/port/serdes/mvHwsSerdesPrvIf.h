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
* @file mvHwsSerdesPrvIf.h
*
* @brief
*
* @version   61
********************************************************************************
*/

#ifndef __mvHwsSerdesPrvIf_H
#define __mvHwsSerdesPrvIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>

/* serdes PHY registers offset */
#define COM_H_REV2_PU_TX_INTP_Reg       (0x04)
#define COM_H_REV2_RX_DFE_Reg           (0x08)
#define COM_H_REV2_REG6_Reg             (0x0C)
#define COM_H_REV2_REG7_Reg             (0x10)
#define COM_H_REV2_PPM_Reg              (0x14)
#define COM_H_REV2_RX_FFE_Reg           (0x18)
#define COM_H_REV2_DFE_LOCK_Reg         (0x1C)
#define COM_H_REV2_CFG_Reg              (0x20)
#define COM_H_REV2_CFG_Reg1             (0x24)
#define COM_H_REV2_CFG_Reg2             (0x28)
#define COM_H_REV2_DFE_Reg              (0x2C)
#define COM_H_REV2_DFE_Reg1             (0x30)
#define COM_H_REV2_TX_AMP_Reg           (0x34)
#define COM_H_REV2_GEN1_SET1_Reg        (0x38)
#define COM_H_REV2_PT_PATTERN_SEL_Reg   (0x54)
#define COM_H_REV2_Pattern_Reg          (0x6C)
#define COM_H_REV2_FRAME_Cntr_MSB1_Reg  (0x70)
#define COM_H_REV2_FRAME_Cntr_MSB_Reg   (0x74)
#define COM_H_REV2_FRAME_Cntr_LSB_Reg   (0x78)
#define COM_H_REV2_ERROR_Cntr_MSB_Reg   (0x7C)
#define COM_H_REV2_ERROR_Cntr_LSB_Reg   (0x80)
#define COM_H_REV2_PHY_TEST_OOB_Reg     (0x84)
#define COM_H_REV2_SEL_BITS_Reg         (0x8C)
#define COM_H_REV2_INVERT_Reg           (0x90)
#define COM_H_REV2_Isolate_Reg          (0x98)
#define COM_H_REV2_REG10_Reg            (0xF0)
#define COM_H_REV2_GEN1_SET2_Reg        (0xF4)
#define COM_H_REV2_GEN2_SET2_Reg        (0xF8)

#define COM_H_REV2_TX_AmpDefault_Reg     (0xDC)
#define COM_H_REV2_TX_TRAIN_Reg2        (0x110)
#define COM_H_REV2_TX_TRAIN_Reg3        (0x114)
#define COM_H_REV2_REG1_Reg             (0x11C)
#define COM_H_REV2_REG3_Reg             (0x124)
#define COM_H_REV2_REG4_Reg             (0x128)
#define COM_H_REV2_REG5_Reg             (0x12C)
#define COM_H_REV2_TX_AmpDefault0       (0x138)
#define COM_H_REV2_TX_AmpDefault1       (0x13C)
#define COM_H_REV2_SoftReset_Reg        (0x148)
#define COM_H_REV2_FeedbackDivider_Reg  (0x14C)
#define COM_H_REV2_ForwardDivider_Reg   (0x150)
#define COM_H_REV2_CalibrReg            (0x15C)
#define COM_H_REV2_RxClockAl90          (0x168)
#define COM_H_REV2_TX_EMPH_Reg          (0x178)
#define COM_H_REV2_Pll_Cal_Control_P1   (0x180)
#define COM_H_REV2_OS_PH_OFFSET_Reg     (0x188)
#define COM_H_REV2_DFE_CFG_Reg          (0x19C)
#define COM_H_REV2_OS_PH_Reg            (0x1A0)
#define COM_H_REV2_FFE_Reg              (0x1A4)
#define COM_H_REV2_Ext_Squelch_Calibr   (0x1BC)
#define COM_H_REV2_Pll_Cal_Control0     (0x1C0)
#define COM_H_REV2_REG8_Reg             (0x1E0)
#define COM_H_REV2_REG9_Reg             (0x1E4)
#define COM_H_REV2_CID_Reg              (0x1F8)
#define COM_H_REV2_Loopback_Reg0        (0x28C)
#define COM_H_REV3_G1_SET_3_Reg         (0x440)
#define COM_H_REV3_G1_SET_4_Reg         (0x444)
#define COM_H_REV2_REG2_Reg             (0x1FC)

#define HWS_MAX_SERDES_NUM 10

#define HWS_MAX_PORT_AVAGO_SERDES_NUM   4

#define MV_HWS_SERDES_NOT_ACTIVE    0xffffffff

#define MV_HWS_SERDES_NUM(laneNum)        (laneNum & 0xFFFF)

#define MV_HWS_CONVERT_TO_OPTICAL_BAUD_RATE_MAC(_isOptical,_baudRate) \
    if(_isOptical)                                                                 \
    {                                                                              \
        if      (_baudRate == _1_25G)           _baudRate = _1_25G_SR_LR;          \
        else if (_baudRate == _10_3125G)        _baudRate = _10_3125G_SR_LR;       \
        else if (_baudRate == _25_78125G)       _baudRate = _25_78125G_SR_LR;      \
        else if (_baudRate == _26_5625G)        _baudRate = _26_5625G_SR_LR;       \
        else if (_baudRate == _26_5625G_PAM4)   _baudRate = _26_5625G_PAM4_SR_LR;  \
    }

#define HWS_COMPHY_SERDES_TYPE_CHECK(serdesType) \
        ((serdesType == COM_PHY_C12GP41P2V) || (serdesType == COM_PHY_C28GP4X2) || (serdesType == COM_PHY_C28GP4X1) ||\
         (serdesType == COM_PHY_C28GP4X4)   || (serdesType == COM_PHY_C112GX4)  || (serdesType == COM_PHY_N5XC56GP5X4))

#define HWS_COMPHY_SERDES_MULTILANE_TYPE_CHECK(serdesType) \
        ((serdesType == COM_PHY_C28GP4X4)   || (serdesType == COM_PHY_C112GX4)  || (serdesType == COM_PHY_N5XC56GP5X4))

#define TX_NA                           0xFF
#define AN_PRESETS_NUM                  3

#define SIGNED8_TO_SIGNED32(val)      \
    ((((val) & 0x80))?                \
    ((GT_32)((val) | 0xffffff00)) :   \
    ((GT_32)(val)))

#define SIGNED16_TO_SIGNED32(val)    \
    ((((val) & 0x8000))?              \
    ((GT_32)((val) | 0xffff0000)) :   \
    ((GT_32)(val)))

#define SIGNED8_TO_SIGNED16(val)      \
    ((((val) & 0x80))?                \
    ((GT_16)((val) | 0xff00)) :   \
    ((GT_16)(val)))

#define NA_8BIT         0xFF
#define NA_8BIT_SIGNED  0x7F
#define NA_16BIT        0xFFFF
#define NA_16BIT_SIGNED 0x7FFF
#define NA_32BIT        0xFFFFFFFF

/**
* @enum MV_HWS_COM_PHY_H_SUB_SEQ
* @endinternal
*
* @brief   COMPHY SerDes sequences
*
*/
typedef enum
{
    SERDES_SD_RESET_SEQ,
    SERDES_SD_UNRESET_SEQ,
    SERDES_RF_RESET_SEQ,
    SERDES_RF_UNRESET_SEQ,
    SERDES_SYNCE_RESET_SEQ,
    SERDES_SYNCE_UNRESET_SEQ,

    SERDES_SERDES_POWER_UP_CTRL_SEQ,
    SERDES_SERDES_POWER_DOWN_CTRL_SEQ,
    SERDES_SERDES_RXINT_UP_SEQ,
    SERDES_SERDES_RXINT_DOWN_SEQ,
    SERDES_SERDES_WAIT_PLL_SEQ,

    SERDES_SPEED_1_25G_SEQ,
    SERDES_SPEED_3_125G_SEQ,
    SERDES_SPEED_3_75G_SEQ,
    SERDES_SPEED_4_25G_SEQ,
    SERDES_SPEED_5G_SEQ,
    SERDES_SPEED_6_25G_SEQ,
    SERDES_SPEED_7_5G_SEQ,
    SERDES_SPEED_10_3125G_SEQ,

    SERDES_SD_LPBK_NORMAL_SEQ,
    SERDES_SD_ANA_TX_2_RX_SEQ,
    SERDES_SD_DIG_TX_2_RX_SEQ,
    SERDES_SD_DIG_RX_2_TX_SEQ,

    SERDES_PT_AFTER_PATTERN_NORMAL_SEQ,
    SERDES_PT_AFTER_PATTERN_TEST_SEQ,

    SERDES_RX_TRAINING_ENABLE_SEQ,
    SERDES_RX_TRAINING_DISABLE_SEQ,
    SERDES_TX_TRAINING_ENABLE_SEQ,
    SERDES_TX_TRAINING_DISABLE_SEQ,

    SERDES_SPEED_12_5G_SEQ,
    SERDES_SPEED_3_3G_SEQ,

    SERDES_SPEED_11_5625G_SEQ,
    SERDES_SERDES_PARTIAL_POWER_DOWN_SEQ,
    SERDES_SERDES_PARTIAL_POWER_UP_SEQ,

    SERDES_SPEED_11_25G_SEQ,
    SERDES_CORE_RESET_SEQ,
    SERDES_CORE_UNRESET_SEQ,

    SERDES_FFE_TABLE_LR_SEQ,
    SERDES_FFE_TABLE_SR_SEQ,
    SERDES_SPEED_10_9375G_SEQ,
    SERDES_SPEED_12_1875G_SEQ,
    SERDES_SPEED_5_625G_SEQ,
    SERDES_SPEED_5_15625G_SEQ,

    MV_SERDES_LAST_SEQ

}MV_HWS_COM_PHY_H_SUB_SEQ;

/******************************** enums ***************************************/
/**
* @enum MV_SERDES_SPEED
* @endinternal
*
* @brief   SerDes speeds
*
*/
typedef enum
{
    SERDES_SPEED_1_25G,
    SERDES_SPEED_3_125G,
    SERDES_SPEED_3_3G,
    SERDES_SPEED_3_75G,
    SERDES_SPEED_4_25G,
    SERDES_SPEED_5G,
    SERDES_SPEED_6_25G,
    SERDES_SPEED_7_5G,
    SERDES_SPEED_10_3125G,
    SERDES_SPEED_11_5625G,
    SERDES_SPEED_12_5G,
    SERDES_SPEED_10_9375G,
    SERDES_SPEED_12_1875G,
    SERDES_SPEED_5_625G,
    SERDES_SPEED_5_15625G,

    SERDES_LAST_SPEED

} MV_SERDES_SPEED;

/**
* @enum MV_HWS_DFE_MODE
* @endinternal
*
* @brief   SerDes DFE types
*
*/
typedef enum
{
    /* for COM_PHY */
    DFE_DISABLE,
    DFE_ENABLE,
    DFE_FREEZE,
    DFE_FORCE,

    /* for Avago */
    DFE_STOP_ADAPTIVE,
    DFE_START_ADAPTIVE,
    DFE_ICAL,
    DFE_PCAL,
    DFE_ICAL_VSR,        /* iCal-VSR mode, Bypass DFE TAP tuning */
    DFE_ICAL_BYPASS_CTLE,
    DFE_START_ADAPTIVE_FIXED_FFE,
    DFE_PCAL_FIXED_FFE
}MV_HWS_DFE_MODE;

/**
* @enum MV_HWS_SERDES_LB
* @endinternal
*
* @brief   SerDes Loopback types
*
*/
typedef enum
{
  SERDES_LP_DISABLE,
  SERDES_LP_AN_TX_RX,
  SERDES_LP_DIG_TX_RX,
  SERDES_LP_DIG_RX_TX

}MV_HWS_SERDES_LB;

/* used to hold data required to init the serdes */
/**
* @struct MV_SPEED_CONFIG_REG_DATA
* @endinternal
*
* @brief   SerDes speed config
*
*/
typedef struct
{
    GT_U32  regOffset;
    GT_U32  mask;
    GT_U32  regData[SERDES_LAST_SPEED];

} MV_SPEED_CONFIG_REG_DATA;

/**
* @struct MV_OP_SERDES_PARAMS
* @endinternal
*
* @brief   SerDes operations
*
*/
typedef struct
{
    GT_U16   unitId;
    GT_U32   regOffset;
    GT_U32   regData[SERDES_LAST_SPEED];
    GT_U32   mask;

}MV_OP_SERDES_PARAMS;

/**
* @struct MV_CFG_SERDES_ELEMENT
* @endinternal
*
* @brief   SerDes configuration elements
*
*/
typedef struct
{
    MV_EL_DB_OPERATION op;
    MV_OP_SERDES_PARAMS       params;

}MV_CFG_SERDES_ELEMENT;

/******************************** functores ***************************************/
typedef GT_STATUS (*MV_HWS_SERDES_ARRAY_POWER_CNTRL_FUNC_PTR)
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     numOfSer,
    GT_UOPT                     *serdesArr,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
);

typedef GT_STATUS (*MV_HWS_SERDES_POWER_CNTRL_FUNC_PTR)
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
);

typedef GT_STATUS (*MV_HWS_SERDES_MANUAL_TX_CFG_FUNC_PTR)
(
    GT_U8                               devNum,
    GT_UOPT                             portGroup,
    GT_UOPT                             serdesNum,
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT    *txConfigPtr
);

typedef GT_STATUS (*MV_HWS_SERDES_MANUAL_RX_CFG_FUNC_PTR)
(
    GT_U8                               devNum,
    GT_UOPT                             portGroup,
    GT_UOPT                             serdesNum,
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT    *rxConfigPtr
);

typedef GT_STATUS (*MV_HWS_SERDES_LB_CFG_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    lbMode
);

typedef GT_STATUS (*MV_HWS_SERDES_LB_GET_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    *lbModePtr
);

typedef GT_STATUS (*MV_HWS_SERDES_PPM_CFG_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_PPM_VALUE    ppmVal
);

typedef GT_STATUS (*MV_HWS_SERDES_PPM_GET_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_PPM_VALUE    *ppmVal
);

typedef GT_STATUS (*MV_HWS_SERDES_AUX_COUNTER_START_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum
);

typedef GT_STATUS (*MV_HWS_SERDES_TEST_GEN_FUNC_PTR)
(
    GT_U8                     devNum,
    GT_UOPT                   portGroup,
    GT_UOPT                   serdesNum,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    MV_HWS_SERDES_TEST_GEN_MODE mode
);

typedef GT_STATUS (*MV_HWS_SERDES_TEST_GEN_GET_FUNC_PTR)
(
    GT_U8                     devNum,
    GT_UOPT                   portGroup,
    GT_UOPT                   serdesNum,
    MV_HWS_SERDES_TX_PATTERN  *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE *modePtr
);

typedef char* (*MV_HWS_SERDES_TYPE_GET_FUNC_PTR)(void);

typedef GT_STATUS (*MV_HWS_SERDES_TEST_GEN_STATUS_FUNC_PTR)
(
    GT_U8                     devNum,
    GT_UOPT                   portGroup,
    GT_UOPT                   serdesNum,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    GT_BOOL                   counterAccumulateMode,
    MV_HWS_SERDES_TEST_GEN_STATUS *status
);

typedef GT_STATUS (*MV_HWS_SERDES_EOM_GET_FUNC_PTR)
(
    GT_U8                     devNum,
    GT_UOPT                   portGroup,
    GT_UOPT                   serdesNum,
    GT_UREG_DATA              timeout,
    MV_HWS_SERDES_EOM_RES     *results
);

typedef GT_STATUS (*MV_HWS_SERDES_AUTO_TUNE_CFG_FUNC_PTR)
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL rxTraining,
    GT_BOOL txTraining
);

typedef GT_STATUS (*MV_HWS_SERDES_GEN_BOOL_FUNC_PTR)
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL training
);

typedef GT_STATUS (*MV_HWS_SERDES_AUTO_TUNE_RXTX_START_FUNC_PTR)
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL rxTraining,
    GT_BOOL txTraining
);

typedef GT_STATUS (*MV_HWS_SERDES_AUTO_TUNE_RESULT_FUNC_PTR)
(
    GT_U8                               devNum,
    GT_UOPT                             portGroup,
    GT_UOPT                             serdesNum,
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *tuneResults
);

typedef GT_STATUS (*MV_HWS_SERDES_AUTO_TUNE_STATUS_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
);

typedef GT_STATUS (*MV_HWS_SERDES_RESET_FUNC_PTR)
(
    GT_U8      devNum,
    GT_UOPT    portGroup,
    GT_UOPT    serdesNum,
    GT_BOOL    analogReset,
    GT_BOOL    digitalReset,
    GT_BOOL    syncEReset
);

typedef GT_STATUS (*MV_HWS_SERDES_RESET_EXT_FUNC_PTR)
(
    GT_U8      devNum,
    GT_UOPT    portGroup,
    GT_UOPT    serdesNum,
    GT_BOOL    analogReset,
    GT_BOOL    digitalReset,
    GT_BOOL    syncEReset,
    GT_BOOL    coreReset
);

typedef GT_STATUS (*MV_HWS_SERDES_DFE_CFG_FUNC_PTR)
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    MV_HWS_DFE_MODE   dfeMode,
    GT_REG_DATA       *dfeCfg
);

typedef GT_STATUS (*MV_HWS_SERDES_DFE_STATUS_FUNC_PTR)
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    GT_BOOL           *dfeLock,
    GT_32             *dfeCfg
);

typedef GT_STATUS (*MV_HWS_SERDES_DFE_STATUS_EXT_FUNC_PTR)
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    GT_BOOL           *dfeLock,
    GT_REG_DATA       *dfeCfg,
    GT_REG_DATA       *f0Cfg,
    GT_REG_DATA       *savCfg
);

typedef GT_STATUS (*MV_HWS_SERDES_POLARITY_CFG_FUNC_PTR)
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL invertTx,
    GT_BOOL invertRx
);

typedef GT_STATUS (*MV_HWS_SERDES_POLARITY_CFG_GET_FUNC_PTR)
(
  GT_U8             devNum,
  GT_UOPT           portGroup,
  GT_UOPT           serdesNum,
  GT_BOOL           *invertTx,
  GT_BOOL           *invertRx
);

typedef GT_STATUS (*MV_HWS_SERDES_DFE_OPTI_FUNC_PTR)
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_UREG_DATA  *ffeR
);

typedef GT_STATUS (*MV_HWS_SERDES_FFE_CFG_FUNC_PTR)
(
  GT_U8   devNum,
  GT_UOPT portGroup,
  GT_UOPT serdesNum,
  GT_UREG_DATA  ffeRes,
  GT_UREG_DATA  ffeCap,
  GT_UREG_DATA  alig
);

typedef GT_STATUS (*MV_HWS_SERDES_CALIBRATION_STATUS_FUNC_PTR)
(
 GT_U8   devNum,
 GT_UOPT portGroup,
 GT_UOPT serdesNum,
 MV_HWS_CALIBRATION_RESULTS *results
);

typedef GT_STATUS (*MV_HWS_SERDES_TX_ENABLE_GET_FUNC_PTR)
(
 GT_U8   devNum,
 GT_UOPT portGroup,
 GT_UOPT serdesNum,
 GT_BOOL *enable
);

typedef GT_STATUS (*MV_HWS_SERDES_TX_IF_SELECT_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    GT_UREG_DATA        serdesTxIfNum
);

typedef GT_STATUS (*MV_HWS_SERDES_TX_IF_SELECT_GET_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    GT_UREG_DATA        *serdesTxIfNum
);

typedef GT_STATUS (*MV_HWS_SERDES_SQLCH_CFG_FUNC_PTR)
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_UREG_DATA  sqlch
);

typedef GT_STATUS (*MV_HWS_SERDES_DIGITAL_RESET_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_RESET        digitalReset
);

typedef GT_STATUS (*MV_HWS_SERDES_FIX_AL90_START_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_ALIGN90_PARAMS   *params
);

typedef GT_STATUS (*MV_HWS_SERDES_FIX_AL90_STOP_FUNC_PTR)
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    MV_HWS_ALIGN90_PARAMS *params,
    GT_BOOL     fixAlignPass
);

typedef GT_STATUS (*MV_HWS_SERDES_FIX_AL90_STATUS_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *trainingStatus
);

typedef GT_STATUS (*MV_HWS_SERDES_SEQ_GET_FUNC_PTR)
(
    MV_HWS_COM_PHY_H_SUB_SEQ  seqType,
    MV_OP_PARAMS             *seqLine,
    GT_UOPT                   lineNum
);

typedef GT_STATUS (*MV_HWS_SERDES_SEQ_SET_FUNC_PTR)
(
    GT_BOOL                   firstLine,
    MV_HWS_COM_PHY_H_SUB_SEQ  seqType,
    MV_OP_PARAMS             *seqLine,
    GT_UOPT                   numOfOp
);

typedef GT_STATUS (*MV_HWS_SERDES_SHORT_CORE_RESET_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_RESET        coreReset
);

typedef GT_STATUS (*MV_HWS_SERDES_AUTO_TUNE_STATUS_SHORT_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
);

typedef GT_STATUS (*MV_HWS_SERDES_GEN_FUNC_PTR)
(
    GT_U8    devNum,
    GT_UOPT  portGroup,
    GT_UOPT  serdesNum
);

typedef GT_STATUS (*MV_HWS_SERDES_SPEED_GET_FUNC_PTR)
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    MV_HWS_SERDES_SPEED *rate
);

typedef GT_STATUS (*MV_HWS_SERDES_MANUAL_RX_CFG_GET_FUNC_PTR)
(
    GT_U8                             devNum,
    GT_UOPT                           portGroup,
    GT_UOPT                           serdesNum,
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT  *configParams
);

typedef GT_STATUS (*MV_HWS_SERDES_MANUAL_TX_CFG_GET_FUNC_PTR)
(
    GT_U8                             devNum,
    GT_UOPT                           portGroup,
    GT_UOPT                           serdesNum,
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *configParams
);

typedef GT_STATUS (*MV_HWS_SERDES_GEN_BOOL_GET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 *enable
);

typedef GT_STATUS (*MV_HWS_SERDES_SCAN_SAMPLER_FUNC_PTR)
(
    GT_U8                  devNum,
    GT_U8                  portGroup,
    GT_U32                 serdesNum,
    GT_U32                 waitTime,
    GT_U32                 samplerSelect,
    GT_U32                 errorThreshold,
    MV_HWS_SAMPLER_RESULTS *samplerResults
);

typedef GT_STATUS (*MV_HWS_SERDES_PARTIAL_POWER_DOWN_FUNC_PTR)
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     powerDownRx,
    GT_BOOL     powerDownTx
);

typedef GT_STATUS (*MV_HWS_SERDES_PARTIAL_POWER_STATUS_FUNC_PTR)
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     *powerDownRx,
    GT_BOOL     *powerDownTx
);

typedef GT_STATUS (*MV_HWS_SERDES_ARRAY_HALT_DFE_TUNE_FUNC_PTR)
(
    GT_U8                        devNum,
    GT_UOPT                      portGroup,
    GT_UOPT                      *serdesArr,
    GT_UOPT                      numOfSer
);

typedef GT_STATUS (*MV_HWS_SERDES_LOW_POWER_MODE_ENABLE_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 enableLowPower
);

typedef GT_STATUS (*MV_HWS_SERDES_TEMPERATURE_GET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_32                   *serdesTemperature
);

typedef GT_STATUS (*MV_HWS_SERDES_ENCODING_TYPE_GET_FUNC_PTR)
(
    GT_U8                            devNum,
    GT_U32                           serdesNum,
    MV_HWS_SERDES_ENCODING_TYPE     *txEncodingPtr,
    MV_HWS_SERDES_ENCODING_TYPE     *rxEncodingPtr
);

typedef GT_STATUS (*MV_HWS_SERDES_RX_DATAPATH_CONFIG_GET_FUNC_PTR)
(
    GT_U8                                  devNum,
    GT_U32                                 serdesNum,
    MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC   *rxDatapathConfigPtr
);

typedef GT_STATUS (*MV_HWS_SERDES_RX_PLL_LOCK_GET_FUNC_PTR)
(
    GT_U8                                  devNum,
    GT_U32                                 serdesNum,
    OUT GT_BOOL                            *lockPtr
);

typedef GT_STATUS (*MV_HWS_SERDES_PMD_LOG_GET_FUNC_PTR)
(
    IN  GT_U8                  devNum,
    IN  GT_U32                 serdesNum,
    OUT GT_U32                 *validEntriesPtr,
    OUT GT_CHAR                *pmdLogStr,
    IN  GT_U32                 pmdLogStrSize
);

typedef GT_STATUS (*MV_HWS_SERDES_ERROR_INJECT_FUNC_PTR)
(
    IN GT_U8                        devNum,
    IN GT_UOPT                      portGroup,
    IN GT_UOPT                      serdesNum,
    IN GT_UOPT                      numOfBits,
    IN MV_HWS_SERDES_DIRECTION      serdesDirection
);

typedef GT_STATUS (*MV_HWS_SERDES_DRO_GET_FUNC_PTR)
(
    IN GT_U8                        devNum,
    IN GT_UOPT                      serdesNum,
    OUT GT_U16                      *dro
);

typedef GT_STATUS (*MV_HWS_SERDES_PRESET_OVERRIDE_SET_FUNC_PTR)
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               serdesNum,
    IN  MV_HWS_SERDES_PRESET_OVERRIDE_UNT    *serdesPresetOverride
);

typedef GT_STATUS (*MV_HWS_SERDES_PRESET_OVERRIDE_GET_FUNC_PTR)
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               serdesNum,
    OUT MV_HWS_SERDES_PRESET_OVERRIDE_UNT    *serdesPresetOverridePtr
);

/**
* @internal MV_HWS_SERDES_OPERATION_FUNC_PTR function
* @endinternal
*
* @brief   Port Serdes operation set
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] serdesNum                - Serdes Num
* @param[in] portMode                 - Port Mode
* @param[in] operation                - operation to do
* @param[in] data                     - value to set
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_HWS_SERDES_OPERATION_FUNC_PTR)
(
       IN GT_U8                devNum,
       IN GT_U32               portGroup,
       IN GT_U32               serdesNum,
       IN MV_HWS_PORT_STANDARD portMode,
       IN MV_HWS_PORT_SERDES_OPERATION_MODE_ENT operation,
       IN GT_U32              *data,
       IN GT_U32              *result
);

/**
* @internal MV_HWS_SERDES_FW_DOWNLOAD_FUNC_PTR function
* @endinternal
*
* @brief   Download Firmware
*
* @param[in] devNum            - Device Number
* @param[in] portGroup         - Port Group
* @param[in] serdesArr         - array of serdes numbers
* @param[in] numOfSerdeses     - num of serdeses in serdesArr
* @param[in] firmwareSelector  - FW  to download
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
*/
typedef GT_STATUS (*MV_HWS_SERDES_FW_DOWNLOAD_FUNC_PTR)
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     *serdesArr,
    GT_U8       numOfSerdeses,
    GT_U8       firmwareSelector
);

typedef GT_STATUS (*MV_HWS_SERDES_DB_TX_CFG_GET_FUNC_PTR)
(
    GT_U8                             devNum,
    GT_UOPT                           portGroup,
    GT_UOPT                           serdesNum,
    MV_HWS_SERDES_SPEED               baudRate,
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *configParamsPtr
);

typedef GT_STATUS (*MV_HWS_SERDES_DB_RX_CFG_GET_FUNC_PTR)
(
    GT_U8                             devNum,
    GT_UOPT                           portGroup,
    GT_UOPT                           serdesNum,
    MV_HWS_SERDES_SPEED               baudRate,
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT  *configParamsPtr
);

typedef GT_STATUS (*MV_HWS_SERDES_DUMP_INFO_FUNC_PTR)
(
    GT_U8                           devNum,
    GT_UOPT                         portGroup,
    GT_UOPT                         serdesNum,
    MV_HWS_SERDES_DUMP_TYPE_ENT     dumpType,
    GT_BOOL                         printBool,
    GT_U32                          *outputPtr
);

/******************************** Functions Struct ***************************************/

typedef struct
{
  MV_HWS_SERDES_ARRAY_POWER_CNTRL_FUNC_PTR       serdesArrayPowerCntrlFunc;
  MV_HWS_SERDES_POWER_CNTRL_FUNC_PTR             serdesPowerCntrlFunc;
  MV_HWS_SERDES_MANUAL_TX_CFG_FUNC_PTR           serdesManualTxCfgFunc;
  MV_HWS_SERDES_MANUAL_RX_CFG_FUNC_PTR           serdesManualRxCfgFunc;
  MV_HWS_SERDES_PPM_CFG_FUNC_PTR                 serdesPpmCfgFunc;
  MV_HWS_SERDES_PPM_GET_FUNC_PTR                 serdesPpmGetFunc;
  MV_HWS_SERDES_TYPE_GET_FUNC_PTR                serdesTypeGetFunc;
  MV_HWS_SERDES_EOM_GET_FUNC_PTR                 serdesEomGetFunc;
  MV_HWS_SERDES_AUTO_TUNE_CFG_FUNC_PTR           serdesAutoTuneCfgFunc;
  MV_HWS_SERDES_GEN_BOOL_FUNC_PTR                serdesRxAutoTuneStartFunc;
  MV_HWS_SERDES_GEN_BOOL_FUNC_PTR                serdesTxAutoTuneStartFunc;
  MV_HWS_SERDES_AUTO_TUNE_RXTX_START_FUNC_PTR    serdesAutoTuneStartFunc;
  MV_HWS_SERDES_AUTO_TUNE_RESULT_FUNC_PTR        serdesAutoTuneResultFunc;
  MV_HWS_SERDES_AUTO_TUNE_STATUS_FUNC_PTR        serdesAutoTuneStatusFunc;
  MV_HWS_SERDES_RESET_FUNC_PTR                   serdesResetFunc;
  MV_HWS_SERDES_DFE_CFG_FUNC_PTR                 serdesDfeCfgFunc;
  MV_HWS_SERDES_DFE_STATUS_EXT_FUNC_PTR          serdesDfeStatusExtFunc;
  MV_HWS_SERDES_POLARITY_CFG_FUNC_PTR            serdesPolarityCfgFunc;
  MV_HWS_SERDES_POLARITY_CFG_GET_FUNC_PTR        serdesPolarityCfgGetFunc;
  MV_HWS_SERDES_DFE_OPTI_FUNC_PTR                serdesDfeOptiFunc;
  MV_HWS_SERDES_FFE_CFG_FUNC_PTR                 serdesFfeCfgFunc;
  MV_HWS_SERDES_CALIBRATION_STATUS_FUNC_PTR      serdesCalibrationStatusFunc;
  MV_HWS_SERDES_GEN_BOOL_FUNC_PTR                serdesTxEnableFunc;
  MV_HWS_SERDES_TX_IF_SELECT_GET_FUNC_PTR        serdesTxIfSelectGetFunc;
  MV_HWS_SERDES_SQLCH_CFG_FUNC_PTR               serdesSqlchCfgFunc;
  MV_HWS_SERDES_GEN_BOOL_FUNC_PTR                serdesAcTermCfgFunc;
  MV_HWS_SERDES_RESET_EXT_FUNC_PTR               serdesResetExtFunc;
  MV_HWS_SERDES_SEQ_GET_FUNC_PTR                 serdesSeqGetFunc;
  MV_HWS_SERDES_SHORT_CORE_RESET_FUNC_PTR        serdesCoreReset;
  MV_HWS_SERDES_GEN_FUNC_PTR                     serdesAutoTuneStopFunc;
  MV_HWS_SERDES_GEN_FUNC_PTR                     serdesDfeCheckFunc;
  MV_HWS_SERDES_SPEED_GET_FUNC_PTR               serdesSpeedGetFunc;
  MV_HWS_SERDES_MANUAL_RX_CFG_GET_FUNC_PTR       serdesManualRxCfgGetFunc;
  MV_HWS_SERDES_MANUAL_TX_CFG_GET_FUNC_PTR       serdesManualTxCfgGetFunc;
  MV_HWS_SERDES_GEN_BOOL_GET_FUNC_PTR            serdesSignalDetectGetFunc;
  MV_HWS_SERDES_GEN_BOOL_GET_FUNC_PTR            serdesCdrLockStatusGetFunc;
  MV_HWS_SERDES_POWER_CNTRL_FUNC_PTR             serdesApPowerCntrlFunc;
  MV_HWS_SERDES_LOW_POWER_MODE_ENABLE_FUNC_PTR   serdesLowPowerModeEnableFunc;
  MV_HWS_SERDES_OPERATION_FUNC_PTR               serdesOperFunc;
  MV_HWS_SERDES_TEMPERATURE_GET_FUNC_PTR         serdesTemperatureFunc;
  MV_HWS_SERDES_ENCODING_TYPE_GET_FUNC_PTR       serdesEncodingTypeGetFunc;
  MV_HWS_SERDES_RX_DATAPATH_CONFIG_GET_FUNC_PTR  serdesRxDatapathConfigGetFunc;
  MV_HWS_SERDES_RX_PLL_LOCK_GET_FUNC_PTR         serdesRxPllLockGetFunc;
  MV_HWS_SERDES_DRO_GET_FUNC_PTR                 serdesDroGetFunc;

#ifndef RAVEN_DEV_SUPPORT
  MV_HWS_SERDES_TX_ENABLE_GET_FUNC_PTR           serdesTxEnableGetFunc;
  MV_HWS_SERDES_TX_IF_SELECT_FUNC_PTR            serdesTxIfSelectFunc;
  MV_HWS_SERDES_DIGITAL_RESET_FUNC_PTR           serdesDigitalReset;
  MV_HWS_SERDES_AUTO_TUNE_STATUS_FUNC_PTR        serdesAutoTuneStatusShortFunc;
  MV_HWS_SERDES_ARRAY_HALT_DFE_TUNE_FUNC_PTR     serdesArrayHaltDfeTuneFunc;
  MV_HWS_SERDES_SEQ_SET_FUNC_PTR                 serdesSeqSetFunc;
  MV_HWS_SERDES_GEN_BOOL_FUNC_PTR                serdesSqDetectCfgFunc;
  MV_HWS_SERDES_DFE_STATUS_FUNC_PTR              serdesDfeStatusFunc;
  MV_HWS_SERDES_LB_CFG_FUNC_PTR                  serdesLoopbackCfgFunc;
  MV_HWS_SERDES_LB_GET_FUNC_PTR                  serdesLoopbackGetFunc;
  MV_HWS_SERDES_TEST_GEN_FUNC_PTR                serdesTestGenFunc;
  MV_HWS_SERDES_TEST_GEN_GET_FUNC_PTR            serdesTestGenGetFunc;
  MV_HWS_SERDES_TEST_GEN_STATUS_FUNC_PTR         serdesTestGenStatusFunc;
  MV_HWS_SERDES_AUX_COUNTER_START_FUNC_PTR       serdesAuxCounterStartFunc;
  MV_HWS_SERDES_FIX_AL90_START_FUNC_PTR          serdesFixAlign90Start;
  MV_HWS_SERDES_FIX_AL90_STOP_FUNC_PTR           serdesFixAlign90Stop;
  MV_HWS_SERDES_FIX_AL90_STATUS_FUNC_PTR         serdesFixAlign90Status;
  MV_HWS_SERDES_SCAN_SAMPLER_FUNC_PTR            serdesScanSamplerFunc;
  MV_HWS_SERDES_PARTIAL_POWER_STATUS_FUNC_PTR    serdesPartialPowerStatusGetFunc;
  MV_HWS_SERDES_PARTIAL_POWER_DOWN_FUNC_PTR      serdesPartialPowerDownFunc;
  MV_HWS_SERDES_FW_DOWNLOAD_FUNC_PTR             serdesFirmwareDownloadFunc;
  /*
    MV_HWS_SERDES_DB_TX_CFG_FUNC_PTR               serdesDbTxCfgFunc;
    MV_HWS_SERDES_DB_RX_CFG_FUNC_PTR               serdesDbRxCfgFunc;
  */
  MV_HWS_SERDES_DB_TX_CFG_GET_FUNC_PTR           serdesDbTxCfgGetFunc;
  MV_HWS_SERDES_DB_RX_CFG_GET_FUNC_PTR           serdesDbRxCfgGetFunc;
  MV_HWS_SERDES_DUMP_INFO_FUNC_PTR               serdesDumpInfoFunc;
  MV_HWS_SERDES_PMD_LOG_GET_FUNC_PTR             serdesPMDLogGetFunc;
  MV_HWS_SERDES_ERROR_INJECT_FUNC_PTR            serdesErrorInjectFunc;
  MV_HWS_SERDES_PRESET_OVERRIDE_SET_FUNC_PTR     serdesPresetOverrideSetFunc;
  MV_HWS_SERDES_PRESET_OVERRIDE_GET_FUNC_PTR     serdesPresetOverrideGetFunc;
#endif
}MV_HWS_SERDES_FUNC_PTRS;


/******************************** declarations ***************************************/


/**
* @internal hwsSerdesIfInit function
* @endinternal
*
* @brief   Init all supported serdes types.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsSerdesIfInit
(
    IN GT_U8 deviceNum
);

/**
* @internal mvHwsSerdesPowerCtrl function
* @endinternal
*
* @brief   Init physical port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      serdesType - serdes type
* @param[in] numOfSer                 - number of SERDESes to configure
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] powerUp                  - true for power UP
* @param[in] serdesConfigPtr            Serdes Configuration
*                                       structure:
*                                      baudRate  - Serdes speed
*                                      refClock  - ref clock value
*                                      refClockSource - ref clock source (primary line or secondary)
*                                      media     - RXAUI or XAUI
*                                      mode      - 10BIT mode (enable/disable)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPowerCtrl
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      serdesNum,
    IN GT_BOOL                     powerUp,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
);

/**
* @internal mvHwsSerdesApPowerCtrl function
* @endinternal
*
* @brief   Init physical port for 802.3AP protocol
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] powerUp                  - true for power UP
* @param[in] serdesConfigPtr            Serdes Configuration
*                                       structure:
*                                      baudRate  - Serdes speed
*                                      refClock  - ref clock value
*                                      refClockSource - ref clock source (primary line or secondary)
*                                      media     - RXAUI or XAUI
*                                      mode      - 10BIT mode (enable/disable)
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesApPowerCtrl
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      serdesNum,
    IN GT_BOOL                     powerUp,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
);

/**
* @internal mvHwsSerdesManualRxConfig function
* @endinternal
*
* @brief   Per SERDES configure the RX parameters
* @param[in] devNum            - system device number
* @param[in] portGroup         - port group (core) number
* @param[in] serdesNum         - physical lane number
* @param[in] serdesType        - serdes type
* @param[in] rxConfigPtr       - pointer to unoin of params
*
* @retval 0                    - on success
* @retval 1                    - on error
*/
GT_STATUS mvHwsSerdesManualRxConfig
(
    IN GT_U8                             devNum,
    IN GT_U32                            portGroup,
    IN GT_U32                            serdesNum,
    IN MV_HWS_SERDES_TYPE                serdesType,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT  *rxConfigPtr
);

/**
* @internal mvHwsSerdesManualTxConfig function
* @endinternal
*
* @brief   Per SERDES configure the TX parameters: amplitude, 3 TAP Tx FIR.
*         Can be run after create port.
* @param[in] devNum                     - system device number
* @param[in] portGroup                  - port group (core) number
* @param[in] serdesNum                  - physical serdes number
* @param[in] serdesType                 - serdes type
* @param[in] manTxTuneStcPtr            - pointer to Tx params
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesManualTxConfig
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          serdesNum,
    IN MV_HWS_SERDES_TYPE              serdesType,
    IN MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *manTxTuneStcPtr
);

/**
* @internal mvHwsSerdesManualTxConfigGet function
* @endinternal
*
* @brief   Per SERDES get the configure TX parameters: amplitude, 3 TAP Tx FIR.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - type of serdes (LP/COM_PHY...)
*
* @param[out] manTxTuneStcPtr          - pointer to To
*                                        parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesManualTxConfigGet
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          serdesNum,
    IN MV_HWS_SERDES_TYPE              serdesType,
    OUT MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *manTxTuneStcPtr
);

/**
* @internal mvHwsSerdesLoopback function
* @endinternal
*
* @brief   Perform an internal loopback (SERDES loopback) for debug/BIST use.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - type of serdes (LP/COM_PHY...)
* @param[in] lbMode                   - loopback mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesLoopback
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    IN MV_HWS_SERDES_LB        lbMode
);

/**
* @internal mvHwsSerdesLoopbackGet function
* @endinternal
*
* @brief   Get status of internal loopback (SERDES loopback).
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - type of serdes (LP/COM_PHY...)
*
* @param[out] lbModePtr               - current loopback mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesLoopbackGet
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    OUT MV_HWS_SERDES_LB   *lbModePtr
);

/**
* @internal mvHwsSerdesPpmConfig function
* @endinternal
*
* @brief   Per SERDES add ability to add/sub PPM from main baud rate.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] ppmVal                   - ppm value to config
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPpmConfig
(
    IN GT_U8             devNum,
    IN GT_U32            portGroup,
    IN GT_U32            serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    IN MV_HWS_PPM_VALUE  ppmVal
);

/**
* @internal mvHwsSerdesPpmConfigGet function
* @endinternal
*
* @brief   Per SERDES read PPM value.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] ppmVal                   - PPM current value.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPpmConfigGet
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    OUT MV_HWS_PPM_VALUE    *ppmVal
);

/**
* @internal mvHwsSerdesAutoTuneCfg function
* @endinternal
*
* @brief   Per SERDES configure parameters for TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] rxTraining               - Rx Training (true/false)
* @param[in] txTraining               - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesAutoTuneCfg
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN GT_BOOL rxTraining,
    IN GT_BOOL txTraining
);

/**
* @internal mvHwsSerdesRxAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] rxTraining               - Rx Training (true/false)
*                                      txTraining - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesRxAutoTuneStart
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN GT_BOOL rxTraining
);

/**
* @internal mvHwsSerdesTxAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] txTraining               - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTxAutoTuneStart
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN GT_BOOL txTraining
);

/**
* @internal mvHwsSerdesAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] rxTraining               - Rx Training (true/false)
* @param[in] txTraining               - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesAutoTuneStart
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN GT_BOOL rxTraining,
    IN GT_BOOL txTraining
);

/**
* @internal mvHwsSerdesAutoTuneResult function
* @endinternal
*
* @brief   Per SERDES return the adapted tuning results
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] results                  - the adapted tuning results.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesAutoTuneResult
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              portGroup,
    IN  GT_U32                              serdesNum,
    IN  MV_HWS_SERDES_TYPE                  serdesType,
    OUT MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *results
);

/**
* @internal mvHwsSerdesAutoTuneStatus function
* @endinternal
*
* @brief   Per SERDES check the Rx & Tx training status.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] rxStatus                 - Rx Training status.
* @param[out] txStatus                 - Tx Training status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesAutoTuneStatus
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    OUT MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    OUT MV_HWS_AUTO_TUNE_STATUS *txStatus
);

/**
* @internal mvHwsSerdesReset function
* @endinternal
*
* @brief   Per SERDES Clear the serdes registers (back to defaults.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] serdesType               - serdes type
* @param[in] analogReset              - Analog Reset (On/Off)
* @param[in] digitalReset             - digital Reset (On/Off)
* @param[in] syncEReset               - SyncE Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesReset
(
    IN GT_U8      devNum,
    IN GT_U32     portGroup,
    IN GT_U32     serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    IN GT_BOOL    analogReset,
    IN GT_BOOL    digitalReset,
    IN GT_BOOL    syncEReset
);

/**
* @internal mvHwsSerdesResetExt function
* @endinternal
*
* @brief   Per SERDES Clear the serdes registers (back to defaults.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] serdesNum                - physical serdes number
* @param[in] analogReset              - Analog Reset (On/Off)
* @param[in] digitalReset             - digital Reset (On/Off)
* @param[in] syncEReset               - SyncE Reset (On/Off)
* @param[in] coreReset                - Core Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesResetExt
(
    IN GT_U8      devNum,
    IN GT_U32     portGroup,
    IN GT_U32     serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    IN GT_BOOL    analogReset,
    IN GT_BOOL    digitalReset,
    IN GT_BOOL    syncEReset,
    IN GT_BOOL    coreReset
);

/**
* @internal mvHwsSerdesDfeConfig function
* @endinternal
*
* @brief   Per SERDES configure the DFE parameters.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] dfeMode                  - Enable/disable/freeze/Force
* @param[in] dfeCfg                   - array of dfe configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDfeConfig
(
    IN GT_U8             devNum,
    IN GT_U32            portGroup,
    IN GT_U32            serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    IN MV_HWS_DFE_MODE   dfeMode,
    IN GT_32             *dfeCfg
);

/**
* @internal mvHwsSerdesDfeStatus function
* @endinternal
*
* @brief   Per SERDES get the DFE status and parameters.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] dfeLock                 - Locked/Not locked
* @param[out] dfeCfg                  - array of dfe
*                                       configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDfeStatus
(
    IN GT_U8             devNum,
    IN GT_U32            portGroup,
    IN GT_U32            serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    OUT GT_BOOL           *dfeLock,
    OUT GT_32             *dfeCfg
);

/**
* @internal mvHwsSerdesDfeStatusExt function
* @endinternal
*
* @brief   Per SERDES get the of DFE status and parameters (extended API).
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] dfeLock                  - Locked/Not locked
* @param[out] dfeCfg                   - array of dfe configuration parameters
* @param[out] f0Cfg                    - array of f0 configuration parameters
* @param[out] savCfg                   - array of sav configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDfeStatusExt
(
    IN GT_U8             devNum,
    IN GT_U32            portGroup,
    IN GT_U32            serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    OUT GT_BOOL           *dfeLock,
    OUT GT_32             *dfeCfg,
    OUT GT_32             *f0Cfg,
    OUT GT_32             *savCfg
);

/**
* @internal mvHwsSerdesDfeOpti function
* @endinternal
*
* @brief   Per SERDES find best FFE_R is the min value for scanned DFE range.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] ffeR                     - best value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDfeOpti
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    OUT GT_U32  *ffeR
);

/**
* @internal mvHwsSerdesFfeConfig function
* @endinternal
*
* @brief   Per SERDES configure FFE.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] ffeRes                   - FFE R value
* @param[in] ffeCap                   - FFE C value
* @param[in] alig                     - sampler value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesFfeConfig
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    IN GT_U32  ffeRes,
    IN GT_U32  ffeCap,
    IN GT_U32  alig
);

/**
* @internal mvHwsSerdesCalibrationStatus function
* @endinternal
*
* @brief   Per SERDES return the calibration results.
*         Can be run after create port
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] results                  - the calibration results.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesCalibrationStatus
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      serdesNum,
    IN MV_HWS_SERDES_TYPE          serdesType,
    OUT MV_HWS_CALIBRATION_RESULTS  *results
);

/**
* @internal mvHwsSerdesTxEnable function
* @endinternal
*
* @brief   Enable/Disable Tx.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] enable                   - whether to  or disable Tx.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTxEnable
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN GT_BOOL             enable
);

/**
* @internal mvHwsSerdesTxEnableGet function
* @endinternal
*
* @brief   Enable / Disable transmission of packets in SERDES layer of a port.
*         Use this API to disable Tx for loopback ports.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
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
GT_STATUS mvHwsSerdesTxEnableGet
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    OUT GT_BOOL *enablePtr
);

/**
* @internal mvHwsSerdesTxIfSelect function
* @endinternal
*
* @brief   tx interface select.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - SERDES types
* @param[in] serdesTxIfNum            - interface select number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTxIfSelect
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN GT_U32              serdesTxIfNum
);

/**
* @internal mvHwsSerdesTxIfSelectGet function
* @endinternal
*
* @brief   Return the SERDES Tx interface select number.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - SERDES types
*
* @param[out] serdesTxIfNum            - interface select number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTxIfSelectGet
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    OUT GT_U32              *serdesTxIfNum
);

/**
* @internal mvHwsSerdesSqlchCfg function
* @endinternal
*
* @brief   Configure squelch threshold value.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] sqlchVal                 - squelch threshold value.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesSqlchCfg
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN GT_U32              sqlchVal
);

/**
* @internal mvHwsSerdesTxAutoTuneStop function
* @endinternal
*
* @brief   Per SERDES stop the TX training & Rx Training process
*         Can be run after create port and training start.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTxAutoTuneStop
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType
);

/**
* @internal mvHwsSerdesAutoTuneStatusShort function
* @endinternal
*
* @brief   Per SERDES check Rx or Tx training status
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] rxStatus               - rxStatus
* @param[out] txStatus               - txStatus
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesAutoTuneStatusShort
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    OUT MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    OUT MV_HWS_AUTO_TUNE_STATUS *txStatus
);

/**
* @internal hwsSerdesIfClose function
* @endinternal
*
* @brief   Free all resources allocated by supported SerDes
*          types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsSerdesIfClose
(
    IN GT_U8                   devNum
);

#ifdef MV_HWS_REDUCED_BUILD
extern GT_BOOL serdesExternalFirmware;
#endif
/**
* @internal mvHwsSerdesSetExternalFirmware function
* @endinternal
*
* @brief   Set SerDes firmware to load from external file
*
* @param[in] externalFirmware         - external firmware
*
*/
GT_STATUS mvHwsSerdesSetExternalFirmware
(
    IN GT_BOOL                 externalFirmware
);

/**
* @internal hwsSerdesSeqSet function
* @endinternal
*
* @brief   Set SERDES sequence one line.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] firstLine                - the first line
* @param[in] seqType                  - sequence type
* @param[in] seqLinePtr               - sequence line
* @param[in] numOfOp                  - number of op
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsSerdesSeqSet
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_BOOL firstLine,
    IN MV_HWS_COM_PHY_H_SUB_SEQ seqType,
    IN MV_OP_PARAMS  *seqLine,
    IN GT_U32 numOfOp
);

/**
* @internal hwsSerdesSeqGet function
* @endinternal
*
* @brief   Get SERDES sequence one line.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] seqType                  - sequence type
* @param[in] lineNum                  - line number
*
* @param[out] seqLinePtr               - sequence line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsSerdesSeqGet
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN MV_HWS_COM_PHY_H_SUB_SEQ seqType,
    IN GT_U32 lineNum,
    OUT MV_OP_PARAMS  *seqLine
);

/**
* @internal hwsSerdesGetFuncPtr function
* @endinternal
*
* @brief   Get function structure pointer.
*
* @param[in] devNum                   - system device number
*
* @param[out] hwsFuncsPtr             - serdes function
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsSerdesGetFuncPtr
(
    IN GT_U8                       devNum,
    OUT MV_HWS_SERDES_FUNC_PTRS     ***hwsFuncsPtr
);
void hwsChangeSpeedCfgParams(MV_SERDES_SPEED speed, GT_U32 regOffset, GT_U32 regValue);
void hwsChange1GSpeedCfgV43(void);

/**
* @internal hwsSetSeqParams function
* @endinternal
*
* @brief   update parameter of sequence according to received ID
*
* @param[in] seqId               - ID of sequence
* @param[in] offset              - register offset, number of
*                                  the register to update
* @param[in] data                - data to update
* @param[in] dataMask            - bits to update
*
* @retval GT_OK                        - on success
* @retval GT_NO_SUCH/GT_NOT_SUPPORTED  - on error
*/
GT_STATUS hwsSetSeqParams
(
    IN GT_U32  seqId,
    IN GT_U32  offset,
    IN GT_U32  data,
    IN GT_U32  dataMask
);


/**
* @internal mvHwsSerdesRev2DfeCheck function
* @endinternal
*
* @brief   Check DFE values range and start sampler calibration, if needed.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesRev2DfeCheck
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType
);

/**
* @internal mvHwsSerdesSpeedGet function
* @endinternal
*
* @brief   Return SERDES baud rate.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] rate                    - rate
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesSpeedGet
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    OUT MV_HWS_SERDES_SPEED *rate
);

/**
* @internal mvHwsSerdesArrayPowerCtrl function
* @endinternal
*
* @brief   Init physical port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      serdesType - serdes type
* @param[in] numOfSer                 - number of SERDESes to configure
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] powerUp                  - true for power UP
* @param[in] serdesConfigPtr            Serdes Configuration
*                                       structure:
*                                      baudRate  - Serdes speed
*                                      refClock  - ref clock value
*                                      refClockSource - ref clock source (primary line or secondary)
*                                      media     - RXAUI or XAUI
*                                      mode      - 10BIT mode (enable/disable)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesArrayPowerCtrl
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      numOfSer,
    IN GT_U32                      *serdesArr,
    IN GT_BOOL                     powerUp,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
);
/**
* @internal mvHwsSerdesArrayHaltDfeTune function
* @endinternal
*
* @brief   Safely halt DFE tune.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      serdesType - serdes type
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSer                 - number of SERDESes to
* @param[in] serdesConfigPtr          - serdes config structure
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesArrayHaltDfeTune
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      *serdesArr,
    IN GT_U32                      numOfSer,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
);
/**
* @internal mvHwsSerdesAcTerminationCfg function
* @endinternal
*
* @brief   Configures AC termination on current serdes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] serdesType               - serdes type
* @param[in] acTermEn                 - enable or disable AC termination
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesAcTerminationCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    IN GT_BOOL                 acTermEn
);

/**
* @internal mvHwsSerdesDigitalReset function
* @endinternal
*
* @brief   Run digital reset / unreset on current SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] serdesType               - serdes type
* @param[in] digitalReset             - digital Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDigitalReset
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN MV_HWS_RESET        digitalReset
);

/**
* @internal mvHwsSerdesSqDetectCfg function
* @endinternal
*
* @brief   Serdes sq_detect mask configuration.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] sqDetect                 - true to enable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesSqDetectCfg
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN GT_BOOL             sqDetect
);

/**
* @internal mvHwsSerdesSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on PCS (true/false).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical PCS number
* @param[in] serdesType               - serdes PCS type
* @param[out] signalDet               - if true, signal was
*                                        detected
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesSignalDetectGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    OUT GT_BOOL                 *signalDet
);

/**
* @internal mvHwsSerdesCdrLockStatusGet function
* @endinternal
*
* @brief   Return SERDES CDR lock status (true - locked /false - notlocked).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical PCS number
* @param[in] serdesType               - serdes PCS type
*
* @param[out] cdrLock                  - if true, CDR was locked
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesCdrLockStatusGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    OUT GT_BOOL                 *cdrLock
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
GT_STATUS mvHwsSerdesEncodingTypeGet
(
    IN GT_U8                            devNum,
    IN GT_U32                           serdesNum,
    IN MV_HWS_SERDES_TYPE               serdesType,
    OUT MV_HWS_SERDES_ENCODING_TYPE     *txEncodingPtr,
    OUT MV_HWS_SERDES_ENCODING_TYPE     *rxEncodingPtr
);

/**
* @internal mvHwsSerdesRxDatapathConfigGet function
* @endinternal
*
* @brief   Retrieves the Rx data path configuration values
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] serdesType               - serdes PCS type
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
GT_STATUS mvHwsSerdesRxDatapathConfigGet
(
    IN  GT_U8                                  devNum,
    IN GT_U32                                  serdesNum,
    IN MV_HWS_SERDES_TYPE                      serdesType,
    OUT MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC   *rxDatapathConfigPtr
);

/**
* @internal mvHwsSerdesRxPllLockGet function
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
GT_STATUS mvHwsSerdesRxPllLockGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  serdesNum,
    IN  MV_HWS_SERDES_TYPE      serdesType,
    OUT GT_BOOL                 *lockPtr
);

/**
* @internal mvHwsSerdesPMDLogGet function
* @endinternal
*
* @brief   Display's Tx Training Log
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] serdesNum                - serdes number
*
* @param[out] validEntriesPtr         - (pointer to) number of
*                                        valid entries
* @param[out] pmdLogStr               - (pointer to) struct
*                                       aligned to string
* @param[in] pmdLogStrSize            - max size of pmdLogStr
* 
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPMDLogGet
(
    IN  GT_U8                  devNum,
    IN  GT_U32                 serdesNum,
    OUT GT_U32                 *validEntriesPtr,
    OUT GT_CHAR                *pmdLogStr,
    IN  GT_U32                 pmdLogStrSize
);

/**
* @internal mvHwsSerdesErrorInject function
* @endinternal
*
* @brief   Injects errors into the RX or TX data
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] numOfBits                - Number of error bits to inject
* @param[in] serdesDirection          - Rx or Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*
*/
GT_STATUS mvHwsSerdesErrorInject
(
    IN GT_U8                        devNum,
    IN GT_UOPT                      portGroup,
    IN GT_UOPT                      serdesNum,
    IN GT_UOPT                      numOfBits,
    IN MV_HWS_SERDES_DIRECTION      serdesDirection
);

/**
* @internal mvHwsSerdesScanSampler function
* @endinternal
*
* @brief   Scan SERDES sampler.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical PCS number
* @param[in] serdesType               - serdes PCS type
* @param[in] waitTime                 - waitTime
* @param[in] samplerSelect            - samplerSelect
* @param[in] errorThreshold           - errorThreshold
*
* @param[out] samplerResults          - Results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesScanSampler
(
    IN GT_U8                  devNum,
    IN GT_U8                  portGroup,
    IN GT_U32                 serdesNum,
    IN MV_HWS_SERDES_TYPE     serdesType,
    IN GT_U32                 waitTime,
    IN GT_U32                 samplerSelect,
    IN GT_U32                 errorThreshold,
    OUT MV_HWS_SAMPLER_RESULTS *samplerResults
);

/**
* @internal mvHwsSerdesLowPowerModeEnable function
* @endinternal
*
* @brief   Enable low-power mode after training.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] serdesType               - serdes type
* @param[in] enableLowPower           - 1 - enable, 0 - disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesLowPowerModeEnable
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    IN GT_BOOL                 enableLowPower
);

GT_STATUS mvHwsSerdesGetInfoBySerdesNum
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      serdesNum,
    OUT GT_U32                      *serdesIndexPtr,
    OUT MV_HWS_PER_SERDES_INFO_PTR  *serdesInfoPtr
);

/**
* @internal mvHwsSerdesSdwAddrCalc function
* @endinternal
*
* @brief   Calculate register address in SDW (SerDes Wrapper)
*
* @param[in]  devNum       - device number
* @param[in]  serdesNum    - serdes number
* @param[in]  regAddr      - reg addr inside SDW
 *@param[in]  baseAddr     - base address of the SDW
 *@param[in]  regType      - EXTERNAL_REG / INTERNAL_REG
* @param[out] addressPtr   - pointer to the output address
*
* @retval 0                 - on success
* @retval 1                 - on error
*/
GT_VOID mvHwsSerdesSdwAddrCalc
(
    IN  GT_U8           devNum,
    IN  GT_UOPT         serdesNum,
    IN  GT_UREG_DATA    regAddr,
    IN  GT_UREG_DATA    baseAddr,
    IN  GT_U32          regType,
    OUT GT_U32          *addressPtr
);

/**
* @internal mvHwsSerdesOperation function
* @endinternal
*
* @brief   set serdes operation
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] portMode                 - port standard metric
* @param[in] operation                - port operation mode
* @param[in] dataPtr                     - data
*
* @param[out] resultPtr                 - results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesOperation
(
   IN GT_U8                       devNum,
   IN GT_U32                      portGroup,
   IN GT_U32                      serdesNum,
   IN MV_HWS_PORT_STANDARD        portMode,
   IN MV_HWS_PORT_SERDES_OPERATION_MODE_ENT  operation,
   IN GT_U32                     *dataPtr,
   OUT GT_U32                    *resultPtr
);

/**
* @internal mvHwsSerdesDbRxConfigGet function
* @endinternal
*
* @brief   Per SERDES get the configure RX parameters:
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] baudRate                 - baud rate
*
* @param[out] MV_HWS_MAN_TUNE_RX_CONFIG_DATA - pointer to Rx
*                                              parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDbRxConfigGet
(
    IN  GT_U8                             devNum,
    IN  GT_UOPT                           portGroup,
    IN  GT_UOPT                           serdesNum,
    IN  MV_HWS_SERDES_TYPE                serdesType,
    IN  MV_HWS_SERDES_SPEED               baudRate,
    OUT MV_HWS_SERDES_RX_CONFIG_DATA_UNT  *configParamsPtr
);

/**
* @internal mvHwsSerdesDumpInfo function
* @endinternal
*
* @brief   Dump SerDes info
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - SerDes number to power up/down
* @param[in]  dumpType  - dump type
* @param[in]  printBool - print to terminal
* @param[out] outputPtr - generic pointer to output data
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsSerdesDumpInfo
(
    IN  GT_U8                           devNum,
    IN  GT_UOPT                         portGroup,
    IN  GT_UOPT                         serdesNum,
    IN  MV_HWS_SERDES_DUMP_TYPE_ENT     dumpType,
    IN  GT_BOOL                         printBool,
    OUT GT_U32                          *outputPtr
);

/**
* @internal mvHwsSerdesDbTxConfigGet function
* @endinternal
*
* @brief   Per SERDES get the configure TX parameters:
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] baudRate                 - baud rate
*
* @param[out] MV_HWS_MAN_TUNE_RX_CONFIG_DATA - pointer to Tx
*                                              parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDbTxConfigGet
(
    IN  GT_U8                             devNum,
    IN  GT_UOPT                           portGroup,
    IN  GT_UOPT                           serdesNum,
    IN  MV_HWS_SERDES_TYPE                serdesType,
    IN  MV_HWS_SERDES_SPEED               baudRate,
    OUT MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *configParamsPtr
);

/**
* @internal mvHwsSerdesTemperatureGet function
* @endinternal
*
* @brief   Read SD temperature
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
*
* @param[out] serdesTemperature       - (pointer to) temperature
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTemperatureGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  serdesNum,
    OUT GT_32                   *serdesTemperature
);

/**
* @internal mvHwsSerdesDroGet function
* @endinternal
*
* @brief   Get DRO (Device Ring Oscillator).
*           Indicates performance of device
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - serdes number
*
* @param[out] dro                     - (pointer to)dro
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDroGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  serdesNum,
    OUT GT_U16                  *dro
);

/**
* @internal mvHwsSerdesPresetOverrideSet function
* @endinternal
*
* @brief   Debug hooks APIs : SetTxLocalPreset, SetRemotePreset, SetCTLEPreset
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - serdes number
* @param[in] serdesPresetOverride     - override preset default paramters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPresetOverrideSet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               serdesNum,
    IN  MV_HWS_SERDES_PRESET_OVERRIDE_UNT    *serdesPresetOverride
);

/**
* @internal mvHwsSerdesPresetOverrideGet function
* @endinternal
*
* @brief   Debug hooks APIs : GetTxLocalPreset, GetRemotePreset, GetCTLEPreset
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - serdes number
* 
* @param[out] serdesPresetOverridePtr - (pointer to)override preset default paramters
* 
* @param[out] val                     - value to get
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPresetOverrideGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               serdesNum,
    OUT MV_HWS_SERDES_PRESET_OVERRIDE_UNT    *serdesPresetOverridePtr
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsSerdesPrvIf_H */


