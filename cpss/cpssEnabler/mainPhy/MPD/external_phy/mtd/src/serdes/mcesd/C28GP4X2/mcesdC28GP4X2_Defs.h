/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions global definitions specific to Marvell
 CE Serdes IP: COMPHY_28G_PIPE4_X2
********************************************************************/
#ifndef MCESD_C28GP4X2_DEFS_H
#define MCESD_C28GP4X2_DEFS_H

#ifdef C28GP4X2

#if C_LINKAGE
#if defined __cplusplus
    extern "C" {
#endif
#endif

/* MCU Firmware constants */
#define C28GP4X2_FW_MAX_SIZE            16384       /* Firmware max size in DWORDS */
#define C28GP4X2_FW_BASE_ADDR           0x10000     /* Firmware base address */
#define C28GP4X2_XDATA_MAX_SIZE         512         /* XDATA max size in DWORDS */
#define C28GP4X2_XDATA_CMN_BASE_ADDR    0xE000      /* Common XDATA base address */
#define C28GP4X2_XDATA_LANE_BASE_ADDR   0x6000      /* Lane XDATA base address */

/* Squelch Detector Threshold Range constants */
#define C28GP4X2_SQ_THRESH_MIN          0
#define C28GP4X2_SQ_THRESH_MAX          0x1F

/* Temperature Sensor constants */
#define C28GP4X2_TSENE_GAIN             394
#define C28GP4X2_TSENE_OFFSET           128900

/* PHY Mode */
typedef enum
{
    C28GP4X2_PHYMODE_SATA = 0,
    C28GP4X2_PHYMODE_SAS = 1,
    C28GP4X2_PHYMODE_PCIE = 3,
    C28GP4X2_PHYMODE_SERDES = 4,
    C28GP4X2_PHYMODE_USB3 = 5
} E_C28GP4X2_PHYMODE;

/* SERDES Speeds */
typedef enum
{
    C28GP4X2_SERDES_1P25G = 0,      /* 1.25 Gbps*/
    C28GP4X2_SERDES_2P5G = 12,      /* 2.5 Gbps */
    C28GP4X2_SERDES_3P125G = 1,     /* 3.125 Gbps */
    C28GP4X2_SERDES_10P3125G = 4,   /* 10.3125 Gbps */
    C28GP4X2_SERDES_20P625G = 13,   /* 20.625 Gbps */
    C28GP4X2_SERDES_25P78125G = 7,  /* 25.78125 Gbps */
    C28GP4X2_SERDES_28P125G = 9     /* 28.125 Gbps */
} E_C28GP4X2_SERDES_SPEED;

/* Reference Frequency Clock */
typedef enum
{
    C28GP4X2_REFFREQ_25MHZ = 0,    /* 25 MHz */
    C28GP4X2_REFFREQ_30MHZ = 1,    /* 30 MHz */
    C28GP4X2_REFFREQ_40MHZ = 2,    /* 40 MHz */
    C28GP4X2_REFFREQ_50MHZ = 3,    /* 50 MHz */
    C28GP4X2_REFFREQ_62P25MHZ = 4, /* 62.25 MHz */
    C28GP4X2_REFFREQ_100MHZ = 5,   /* 100 MHz */
    C28GP4X2_REFFREQ_125MHZ = 6,   /* 125 MHz */
    C28GP4X2_REFFREQ_156P25MHZ = 7 /* 156.25 MHz */
} E_C28GP4X2_REFFREQ;

/* Reference Clock selection Group */
typedef enum
{
    C28GP4X2_REFCLK_SEL_GROUP1 = 0, /* PIN_REFCLKC_IN_SIDE_A_G1 or PIN_REFCLKC_IN_SIDE_B_G1 */
    C28GP4X2_REFCLK_SEL_GROUP2 = 1  /* PIN_REFCLKC_IN_SIDE_A_G2 or PIN_REFCLKC_IN_SIDE_B_G2 */
} E_C28GP4X2_REFCLK_SEL;

/* TX and RX Data Bus Width */
typedef enum
{
    C28GP4X2_DATABUS_40BIT = 0,
    C28GP4X2_DATABUS_32BIT = 1
} E_C28GP4X2_DATABUS_WIDTH;

/* TX and RX Polarity */
typedef enum
{
    C28GP4X2_POLARITY_NORMAL = 0,
    C28GP4X2_POLARITY_INVERTED = 1
} E_C28GP4X2_POLARITY;

/* Data Path */
typedef enum
{
    C28GP4X2_PATH_EXTERNAL = 1,
    C28GP4X2_PATH_FAR_END_LB = 2,
    C28GP4X2_PATH_UNKNOWN = 3
} E_C28GP4X2_DATAPATH;

/* Training Type */
typedef enum
{
    C28GP4X2_TRAINING_TRX = 0,
    C28GP4X2_TRAINING_RX = 1
} E_C28GP4X2_TRAINING;

/* Training Timeout */
typedef struct
{
    MCESD_BOOL enable;
    MCESD_U16 timeout; /* milliseconds */
} S_C28GP4X2_TRAINING_TIMEOUT;

/* Hardware Pins */
typedef enum
{
    C28GP4X2_PIN_RESET = 0,
    C28GP4X2_PIN_ISOLATION_ENB = 1,
    C28GP4X2_PIN_BG_RDY = 2,
    C28GP4X2_PIN_SIF_SEL = 3,
    C28GP4X2_PIN_MCU_CLK = 4,
    C28GP4X2_PIN_DIRECT_ACCESS_EN = 5,
    C28GP4X2_PIN_PHY_MODE = 6,
    C28GP4X2_PIN_REFCLK_SEL = 7,
    C28GP4X2_PIN_REF_FREF_SEL = 8,
    C28GP4X2_PIN_PHY_GEN_TX0 = 9,
    C28GP4X2_PIN_PHY_GEN_TX1 = 10,
    C28GP4X2_PIN_PHY_GEN_RX0 = 11,
    C28GP4X2_PIN_PHY_GEN_RX1 = 12,
    C28GP4X2_PIN_DFE_EN0 = 13,
    C28GP4X2_PIN_DFE_EN1 = 14,
    C28GP4X2_PIN_PU_PLL0 = 15,
    C28GP4X2_PIN_PU_PLL1 = 16,
    C28GP4X2_PIN_PU_RX0 = 17,
    C28GP4X2_PIN_PU_RX1 = 18,
    C28GP4X2_PIN_PU_TX0 = 19,
    C28GP4X2_PIN_PU_TX1 = 20,
    C28GP4X2_PIN_TX_IDLE0 = 21,
    C28GP4X2_PIN_TX_IDLE1 = 22,
    C28GP4X2_PIN_PU_IVREF = 23,
    C28GP4X2_PIN_RX_TRAIN_ENABLE0 = 24,
    C28GP4X2_PIN_RX_TRAIN_ENABLE1 = 25,
    C28GP4X2_PIN_RX_TRAIN_COMPLETE0 = 26,
    C28GP4X2_PIN_RX_TRAIN_COMPLETE1 = 27,
    C28GP4X2_PIN_RX_TRAIN_FAILED0 = 28,
    C28GP4X2_PIN_RX_TRAIN_FAILED1 = 29,
    C28GP4X2_PIN_TX_TRAIN_ENABLE0 = 30,
    C28GP4X2_PIN_TX_TRAIN_ENABLE1 = 31,
    C28GP4X2_PIN_TX_TRAIN_COMPLETE0 = 32,
    C28GP4X2_PIN_TX_TRAIN_COMPLETE1 = 33,
    C28GP4X2_PIN_TX_TRAIN_FAILED0 = 34,
    C28GP4X2_PIN_TX_TRAIN_FAILED1 = 35,
    C28GP4X2_PIN_SQ_DETECTED_LPF0 = 36,
    C28GP4X2_PIN_SQ_DETECTED_LPF1 = 37,
    C28GP4X2_PIN_RX_INIT0 = 38,
    C28GP4X2_PIN_RX_INIT1 = 39,
    C28GP4X2_PIN_RX_INIT_DONE0 = 40,
    C28GP4X2_PIN_RX_INIT_DONE1 = 41
} E_C28GP4X2_PIN;

/* TX Equalization Parameters */
typedef enum
{
    C28GP4X2_TXEQ_EM_POST_CTRL = 0,
    C28GP4X2_TXEQ_EM_PEAK_CTRL = 1,
    C28GP4X2_TXEQ_EM_PRE_CTRL = 2,
    C28GP4X2_TXEQ_MARGIN = 3
} E_C28GP4X2_TXEQ_PARAM;

/* CTLE Parameters */
typedef enum
{
    C28GP4X2_CTLE_DATA_RATE = 0,
    C28GP4X2_CTLE_RES1_SEL = 1,
    C28GP4X2_CTLE_RES2_SEL = 2,
    C28GP4X2_CTLE_CAP1_SEL = 3,
    C28GP4X2_CTLE_CAP2_SEL = 4
} E_C28GP4X2_CTLE_PARAM;

/* CDR Parameters */
typedef enum
{
    C28GP4X2_CDR_SELMUFI = 0,
    C28GP4X2_CDR_SELMUFF = 1,
    C28GP4X2_CDR_SELMUPI = 2,
    C28GP4X2_CDR_SELMUPF = 3
} E_C28GP4X2_CDR_PARAM;

typedef enum
{
    C28GP4X2_SR_EN_DISABLE         = 0,
    C28GP4X2_SR_EN_6GBPS_TO_12GBPS = 1,
    C28GP4X2_SR_EN_UNSUPPORTED     = 2,
    C28GP4X2_SR_EN_LESS_THAN_6GBPS = 3
} E_C28GP4X2_SLEWRATE_EN;

/* Slew rate Parameters */
typedef enum
{
    C28GP4X2_SR_CTRL0 = 0,
    C28GP4X2_SR_CTRL1 = 1
} E_C28GP4X2_SLEWRATE_PARAM;

/* Pattern selection */
typedef enum
{
    C28GP4X2_PAT_USER               = 0,
    C28GP4X2_PAT_JITTER_K28P5       = 0x8,
    C28GP4X2_PAT_JITTER_1T          = 0x9,
    C28GP4X2_PAT_JITTER_2T          = 0xA,
    C28GP4X2_PAT_JITTER_4T          = 0xB,
    C28GP4X2_PAT_JITTER_5T          = 0xC,
    C28GP4X2_PAT_JITTER_8T          = 0xD,
    C28GP4X2_PAT_JITTER_10T         = 0xE,
    C28GP4X2_PAT_PRBS7              = 0x10,
    C28GP4X2_PAT_PRBS9              = 0x11,
    C28GP4X2_PAT_PRBS11             = 0x12,
    C28GP4X2_PAT_PRBS11_0           = 0x13,
    C28GP4X2_PAT_PRBS11_1           = 0x14,
    C28GP4X2_PAT_PRBS11_2           = 0x15,
    C28GP4X2_PAT_PRBS11_3           = 0x16,
    C28GP4X2_PAT_PRBS15             = 0x17,
    C28GP4X2_PAT_PRBS16             = 0x18,
    C28GP4X2_PAT_PRBS23             = 0x19,
    C28GP4X2_PAT_PRBS31             = 0x1A,
    C28GP4X2_PAT_PRBS32             = 0x1B,
    C28GP4X2_PAT_SATA_LTDP          = 0x20,
    C28GP4X2_PAT_SATA_HTDP          = 0x21,
    C28GP4X2_PAT_SATA_LFSCP         = 0x22,
    C28GP4X2_PAT_SATA_SSOP          = 0x23,
    C28GP4X2_PAT_SATA_LBP           = 0x24,
    C28GP4X2_PAT_SATA_COMP          = 0x25,
    C28GP4X2_PAT_SAS_JTPAT          = 0x28,
    C28GP4X2_PAT_SAS_MJTPAT         = 0x29,
    C28GP4X2_PAT_SAS_CJTPAT         = 0x2A,
    C28GP4X2_PAT_SAS_DFE_TRAIN      = 0x2B,
    C28GP4X2_PAT_SAS_DFE_TRAIN_DONE = 0x2C,
    C28GP4X2_PAT_SAS_58_SCRAMB_IDLE = 0x2D
} E_C28GP4X2_PATTERN;

/* Pattern: 8B10B Encoding (Optional for PRBS and USER patterns) */
typedef enum
{
    C28GP4X2_ENC_8B10B_DISABLE = 0,
    C28GP4X2_ENC_8B10B_ENABLE = 1,
} E_C28GP4X2_ENC_8B10B;

/* Pattern: SATA Long/Short Pattern */
typedef enum
{
    C28GP4X2_SATA_SHORT = 0,
    C28GP4X2_SATA_LONG = 1,
    C28GP4X2_SATA_NOT_USED = 2
} E_C28GP4X2_SATA_LONGSHORT;

/* Pattern: SATA Initial Disparity*/
typedef enum
{
    C28GP4X2_DISPARITY_NEGATIVE = 0,
    C28GP4X2_DISPARITY_POSITIVE = 1,
    C28GP4X2_DISPARITY_NOT_USED = 2
} E_C28GP4X2_SATA_INITIAL_DISPARITY;

/* Pattern Context */
typedef struct
{
    E_C28GP4X2_PATTERN pattern;
    E_C28GP4X2_ENC_8B10B enc8B10B;   /* 8B10B encoding is optional for PRBS and User patterns */
} S_C28GP4X2_PATTERN;

/* Pattern Comparator Statistics */
typedef struct
{
    MCESD_BOOL lock;
    MCESD_BOOL pass;
    MCESD_U64 totalBits;
    MCESD_U64 totalErrorBits;
} S_C28GP4X2_PATTERN_STATISTICS;

/* Trained Eye Height */
typedef struct
{
    MCESD_U8 f0a;
    MCESD_U8 f0aMax;
    MCESD_U8 f0b;
    MCESD_U8 f0d;
} S_C28GP4X2_TRAINED_EYE_HEIGHT;

/* Eye Measurement Data @ X,Y and X,-Y */
typedef struct
{
    MCESD_32 phase;
    MCESD_U8 voltage;
    MCESD_U32 upperBitCount;
    MCESD_U32 upperBitErrorCount;
    MCESD_U32 lowerBitCount;
    MCESD_U32 lowerBitErrorCount;
} S_C28GP4X2_EOM_DATA;

/* DFE Taps */
typedef enum
{
    C28GP4X2_DFE_F0_N   = 0, /* F0 Negative */
    C28GP4X2_DFE_F0_P   = 1, /* F0 Positive */
    C28GP4X2_DFE_F1     = 2,
    C28GP4X2_DFE_F2_N   = 3, /* F2 Negative */
    C28GP4X2_DFE_F2_P   = 4, /* F2 Positive */
    C28GP4X2_DFE_F3_N   = 5, /* F3 Negative */
    C28GP4X2_DFE_F3_P   = 6, /* F3 Positive */
    C28GP4X2_DFE_F4     = 7,
    C28GP4X2_DFE_F5     = 8,
    C28GP4X2_DFE_F6     = 9,
    C28GP4X2_DFE_F7     = 10,
    C28GP4X2_DFE_F8     = 11,
    C28GP4X2_DFE_F9     = 12,
    C28GP4X2_DFE_F10    = 13,
    C28GP4X2_DFE_F11    = 14,
    C28GP4X2_DFE_F12    = 15,
    C28GP4X2_DFE_F13    = 16,
    C28GP4X2_DFE_F14    = 17,
    C28GP4X2_DFE_F15    = 18,
    C28GP4X2_DFE_FF0    = 19, /* floating tap 0 */
    C28GP4X2_DFE_FF1    = 20, /* floating tap 1 */
    C28GP4X2_DFE_FF2    = 21, /* floating tap 2 */
    C28GP4X2_DFE_FF3    = 22, /* floating tap 3 */
    C28GP4X2_DFE_FF4    = 23, /* floating tap 4 */
    C28GP4X2_DFE_FF5    = 24  /* floating tap 5 */
} E_C28GP4X2_DFE_TAP;

#define F_C28GP4X2R2P0_MAJOR_REV            FIELD_DEFINE(0xA3F8, 23, 30)
#define F_C28GP4X2R2P0_MINOR_REV            FIELD_DEFINE(0xA3F8, 19, 16)
#define F_C28GP4X2R2P0_MCU_EN_LANE0         FIELD_DEFINE(0xA200, 0, 0)
#define F_C28GP4X2R2P0_MCU_EN_LANE1         FIELD_DEFINE(0xA200, 1, 1)
#define F_C28GP4X2R2P0_EN_LANE0             FIELD_DEFINE(0xA334, 0, 0)
#define F_C28GP4X2R2P0_EN_LANE1             FIELD_DEFINE(0xA334, 1, 1)
#define F_C28GP4X2R2P0_BROADCAST            FIELD_DEFINE(0xA314, 27, 27)
#define E_C28GP4X2R2P0_LANE_SEL             FIELD_DEFINE(0xA314, 31, 29)
#define F_C28GP4X2R2P0_RX_CK_SEL            FIELD_DEFINE(0x0258, 4, 4)
#define F_C28GP4X2R2P0_PLL_LOCK_RING        FIELD_DEFINE(0xA318, 15, 15)
#define F_C28GP4X2R2P0_PLL_LOCK_LCPLL       FIELD_DEFINE(0xA318, 14, 14)
#define F_C28GP4X2R2P0_PLL_READY_TX         FIELD_DEFINE(0x2000, 20, 20)
#define F_C28GP4X2R2P0_PLL_READY_RX         FIELD_DEFINE(0x2100, 24, 24)
#define F_C28GP4X2R2P0_CDR_LOCK             FIELD_DEFINE(0x213C, 30, 30)
#define F_C28GP4X2R2P0_ALIGN90_REF          FIELD_DEFINE(0x0038, 7, 2)
#define F_C28GP4X2R2P0_SQ_THRESH            FIELD_DEFINE(0x0010, 7, 2)
#define F_C28GP4X2R2P0_PHY_GEN_MAX          FIELD_DEFINE(0xE62C, 3, 0)
#define F_C28GP4X2R2P0_MCU_STATUS0          FIELD_DEFINE(0x2230, 31, 0)

/* Data Bus Width */
#define F_C28GP4X2R2P0_TX_SEL_BITS          FIELD_DEFINE(0x2034, 31, 31)
#define F_C28GP4X2R2P0_RX_SEL_BITS          FIELD_DEFINE(0x2104, 31, 31)

/* TX/RX Polarity */
#define F_C28GP4X2R2P0_TXD_INV              FIELD_DEFINE(0x2024, 30, 30)
#define F_C28GP4X2R2P0_RXD_INV              FIELD_DEFINE(0x2148, 29, 29)

/* MCU Firmware Version */
#define F_C28GP4X2R2P0_FW_MAJOR_VER         FIELD_DEFINE(0xE600, 31, 24)
#define F_C28GP4X2R2P0_FW_MINOR_VER         FIELD_DEFINE(0xE600, 23, 16)
#define F_C28GP4X2R2P0_FW_PATCH_VER         FIELD_DEFINE(0xE600, 15, 8)
#define F_C28GP4X2R2P0_FW_BUILD_VER         FIELD_DEFINE(0xE600, 7, 0)
#define F_C28GP4X2R2P0_PROG_RAM_SEL         FIELD_DEFINE(0xA22C, 4, 3)

/* TX Equalization */
#define F_C28GP4X2R2P0_TX_EM_PO_CTRL        FIELD_DEFINE(0x2634, 7, 4)
#define F_C28GP4X2R2P0_TX_EM_PEAK_CTRL      FIELD_DEFINE(0x2634, 15, 12)
#define F_C28GP4X2R2P0_TX_EM_PRE_CTRL       FIELD_DEFINE(0x2634, 11, 8)

/* TX Data Error Injection */
#define F_C28GP4X2R2P0_ADD_ERR_EN           FIELD_DEFINE(0x2024, 29, 29)
#define F_C28GP4X2R2P0_ADD_ERR_NUM          FIELD_DEFINE(0x2024, 28, 26)

/* CTLE */
#define F_C28GP4X2R2P0_FFE_DATA_RATE        FIELD_DEFINE(0x0208, 7, 4)
#define F_C28GP4X2R2P0_RES1_SEL             FIELD_DEFINE(0x0200, 7, 4)
#define F_C28GP4X2R2P0_RES2_SEL_E           FIELD_DEFINE(0x0000, 7, 4)
#define F_C28GP4X2R2P0_RES2_SEL_O           FIELD_DEFINE(0x0204, 7, 4)
#define F_C28GP4X2R2P0_CAP1_SEL             FIELD_DEFINE(0x0200, 3, 0)
#define F_C28GP4X2R2P0_CAP2_SEL_E           FIELD_DEFINE(0x0000, 3, 0)
#define F_C28GP4X2R2P0_CAP2_SEL_O           FIELD_DEFINE(0x0204, 3, 0)

/* CDR */
#define F_C28GP4X2R2P0_RX_SELMUFI           FIELD_DEFINE(0x2164, 12, 10)
#define F_C28GP4X2R2P0_RX_SELMUFF           FIELD_DEFINE(0x2164, 15, 13)
#define F_C28GP4X2R2P0_REG_SELMUPI          FIELD_DEFINE(0x022C, 3, 0)
#define F_C28GP4X2R2P0_REG_SELMUPF          FIELD_DEFINE(0x0230, 7, 4)

/* Slew Rate*/
#define F_C28GP4X2R2P0_SLEWRATE_EN          FIELD_DEFINE(0x004C, 7, 6)
#define F_C28GP4X2R2P0_SLEWCTRL0            FIELD_DEFINE(0x004C, 5, 4)
#define F_C28GP4X2R2P0_SLEWCTRL1            FIELD_DEFINE(0x004C, 3, 2)

/* DFE */
#define F_C28GP4X2R2P0_DFE_F0_D_N_E         FIELD_DEFINE(0x2480, 13, 8)
#define F_C28GP4X2R2P0_DFE_F0_D_P_E         FIELD_DEFINE(0x2480, 5, 0)
#define F_C28GP4X2R2P0_DFE_F0_D_N_O         FIELD_DEFINE(0x24A0, 13, 8)
#define F_C28GP4X2R2P0_DFE_F0_D_P_O         FIELD_DEFINE(0x24A0, 5, 0)
#define F_C28GP4X2R2P0_DFE_F1_E             FIELD_DEFINE(0x248C, 5, 0)
#define F_C28GP4X2R2P0_DFE_F1_O             FIELD_DEFINE(0x24AC, 5, 0)
#define F_C28GP4X2R2P0_DFE_F2_D_N_E         FIELD_DEFINE(0x2484, 13, 8)
#define F_C28GP4X2R2P0_DFE_F2_D_P_E         FIELD_DEFINE(0x2484, 5, 0)
#define F_C28GP4X2R2P0_DFE_F2_D_N_O         FIELD_DEFINE(0x24A4, 13, 8)
#define F_C28GP4X2R2P0_DFE_F2_D_P_O         FIELD_DEFINE(0x24A4, 5, 0)
#define F_C28GP4X2R2P0_DFE_F3_D_N_E         FIELD_DEFINE(0x2488, 12, 8)
#define F_C28GP4X2R2P0_DFE_F3_D_P_E         FIELD_DEFINE(0x2488, 4, 0)
#define F_C28GP4X2R2P0_DFE_F3_D_N_O         FIELD_DEFINE(0x24A8, 12, 8)
#define F_C28GP4X2R2P0_DFE_F3_D_P_O         FIELD_DEFINE(0x24A8, 4, 0)
#define F_C28GP4X2R2P0_DFE_F4_E             FIELD_DEFINE(0x248C, 13, 8)
#define F_C28GP4X2R2P0_DFE_F4_O             FIELD_DEFINE(0x24AC, 13, 8)
#define F_C28GP4X2R2P0_DFE_F5_E             FIELD_DEFINE(0x248C, 21, 16)
#define F_C28GP4X2R2P0_DFE_F5_O             FIELD_DEFINE(0x24AC, 21, 16)
#define F_C28GP4X2R2P0_DFE_F6_E             FIELD_DEFINE(0x248C, 29, 24)
#define F_C28GP4X2R2P0_DFE_F6_O             FIELD_DEFINE(0x24AC, 29, 24)
#define F_C28GP4X2R2P0_DFE_F7_E             FIELD_DEFINE(0x2490, 4, 0)
#define F_C28GP4X2R2P0_DFE_F7_O             FIELD_DEFINE(0x24B0, 4, 0)
#define F_C28GP4X2R2P0_DFE_F8_E             FIELD_DEFINE(0x2490, 12, 8)
#define F_C28GP4X2R2P0_DFE_F8_O             FIELD_DEFINE(0x24B0, 12, 8)
#define F_C28GP4X2R2P0_DFE_F9_E             FIELD_DEFINE(0x2490, 20, 16)
#define F_C28GP4X2R2P0_DFE_F9_O             FIELD_DEFINE(0x24B0, 20, 16)
#define F_C28GP4X2R2P0_DFE_F10_E            FIELD_DEFINE(0x2490, 28, 24)
#define F_C28GP4X2R2P0_DFE_F10_O            FIELD_DEFINE(0x24B0, 28, 24)
#define F_C28GP4X2R2P0_DFE_F11_E            FIELD_DEFINE(0x2494, 4, 0)
#define F_C28GP4X2R2P0_DFE_F11_O            FIELD_DEFINE(0x24B4, 4, 0)
#define F_C28GP4X2R2P0_DFE_F12_E            FIELD_DEFINE(0x2494, 12, 8)
#define F_C28GP4X2R2P0_DFE_F12_O            FIELD_DEFINE(0x24B4, 12, 8)
#define F_C28GP4X2R2P0_DFE_F13_E            FIELD_DEFINE(0x2494, 20, 16)
#define F_C28GP4X2R2P0_DFE_F13_O            FIELD_DEFINE(0x24B4, 20, 16)
#define F_C28GP4X2R2P0_DFE_F14_E            FIELD_DEFINE(0x2494, 28, 24)
#define F_C28GP4X2R2P0_DFE_F14_O            FIELD_DEFINE(0x24B4, 28, 24)
#define F_C28GP4X2R2P0_DFE_F15_E            FIELD_DEFINE(0x2498, 4, 0)
#define F_C28GP4X2R2P0_DFE_F15_O            FIELD_DEFINE(0x24B8, 4, 0)
#define F_C28GP4X2R2P0_DFE_FF0_E            FIELD_DEFINE(0x2498, 13, 8)
#define F_C28GP4X2R2P0_DFE_FF0_O            FIELD_DEFINE(0x24B8, 13, 8)
#define F_C28GP4X2R2P0_DFE_FF1_E            FIELD_DEFINE(0x2498, 21, 16)
#define F_C28GP4X2R2P0_DFE_FF1_O            FIELD_DEFINE(0x24B8, 21, 16)
#define F_C28GP4X2R2P0_DFE_FF2_E            FIELD_DEFINE(0x2498, 29, 24)
#define F_C28GP4X2R2P0_DFE_FF2_O            FIELD_DEFINE(0x24B8, 29, 24)
#define F_C28GP4X2R2P0_DFE_FF3_E            FIELD_DEFINE(0x249C, 5, 0)
#define F_C28GP4X2R2P0_DFE_FF3_O            FIELD_DEFINE(0x24BC, 5, 0)
#define F_C28GP4X2R2P0_DFE_FF4_E            FIELD_DEFINE(0x249C, 13, 8)
#define F_C28GP4X2R2P0_DFE_FF4_O            FIELD_DEFINE(0x24BC, 13, 8)
#define F_C28GP4X2R2P0_DFE_FF5_E            FIELD_DEFINE(0x249C, 21, 16)
#define F_C28GP4X2R2P0_DFE_FF5_O            FIELD_DEFINE(0x24BC, 21, 16)

/* TRX Training */
#define F_C28GP4X2R2P0_TRAIN_F0A            FIELD_DEFINE(0x6048, 15, 8)
#define F_C28GP4X2R2P0_TRAIN_F0A_MAX        FIELD_DEFINE(0x6048, 23, 16)
#define F_C28GP4X2R2P0_TRAIN_F0B            FIELD_DEFINE(0x6048, 31, 24)
#define F_C28GP4X2R2P0_TRAIN_F0D            FIELD_DEFINE(0x6048, 7, 0)
#define F_C28GP4X2R2P0_TRX_TRAIN_TIMER      FIELD_DEFINE(0x602C, 15, 0)
#define F_C28GP4X2R2P0_RX_TRAIN_TIMER       FIELD_DEFINE(0x6028, 15, 0)
#define F_C28GP4X2R2P0_TX_TRAIN_TIMER_EN    FIELD_DEFINE(0x6030, 29, 29)
#define F_C28GP4X2R2P0_RX_TRAIN_TIMER_EN    FIELD_DEFINE(0x6030, 30, 30)

/* Data Path */
#define F_C28GP4X2R2P0_DIG_RX2TX_LPBK_EN    FIELD_DEFINE(0x2024, 31, 31)
#define F_C28GP4X2R2P0_ANA_TX2RX_LPBK_EN    FIELD_DEFINE(0x0244, 6, 6)
#define F_C28GP4X2R2P0_DIG_TX2RX_LPBK_EN    FIELD_DEFINE(0x2148, 31, 31)
#define F_C28GP4X2R2P0_DTX_FLOOP_EN         FIELD_DEFINE(0xA018, 16, 16)
#define F_C28GP4X2R2P0_DTX_FLOOP_EN_RING    FIELD_DEFINE(0xA018, 17, 17)
#define F_C28GP4X2R2P0_RX_FOFFSET_DIS       FIELD_DEFINE(0x2168, 19, 19)
#define F_C28GP4X2R2P0_DTX_FOFFSET          FIELD_DEFINE(0xA018, 18, 18)
#define F_C28GP4X2R2P0_DTX_FOFFSET_RING     FIELD_DEFINE(0xA018, 19, 19)
#define F_C28GP4X2R2P0_PU_LB                FIELD_DEFINE(0x0208, 3, 3)
#define F_C28GP4X2R2P0_PU_LB_DLY            FIELD_DEFINE(0x0208, 2, 2)

/* PHY Test */
#define F_C28GP4X2R2P0_PT_TX_PATTERN_SEL    FIELD_DEFINE(0x2300, 27, 22)
#define F_C28GP4X2R2P0_PT_RX_PATTERN_SEL    FIELD_DEFINE(0x2300, 21, 16)
#define F_C28GP4X2R2P0_PT_START_RD          FIELD_DEFINE(0x2300, 6, 6)
#define F_C28GP4X2R2P0_PT_SATA_LONG         FIELD_DEFINE(0x2304, 22, 22)
#define F_C28GP4X2R2P0_PT_PRBS_ENC          FIELD_DEFINE(0x2304, 23, 23)
#define F_C28GP4X2R2P0_PT_CNT_47_16         FIELD_DEFINE(0x2314, 31, 0)
#define F_C28GP4X2R2P0_PT_CNT_15_0          FIELD_DEFINE(0x2318, 31, 16)
#define F_C28GP4X2R2P0_PT_ERR_CNT           FIELD_DEFINE(0x231C, 31, 0)
#define F_C28GP4X2R2P0_PT_USER_PAT_79_48    FIELD_DEFINE(0x2308, 31, 0)
#define F_C28GP4X2R2P0_PT_USER_PAT_47_16    FIELD_DEFINE(0x230C, 31, 0)
#define F_C28GP4X2R2P0_PT_USER_PAT_15_0     FIELD_DEFINE(0x2310, 31, 16)
#define F_C28GP4X2R2P0_PT_USER_K_CHAR       FIELD_DEFINE(0x2310, 15, 8)
#define F_C28GP4X2R2P0_PT_PHYREADY_FORCE    FIELD_DEFINE(0x2300, 28, 28)
#define F_C28GP4X2R2P0_PT_EN_MODE           FIELD_DEFINE(0x2300, 30, 29)
#define F_C28GP4X2R2P0_PT_EN                FIELD_DEFINE(0x2300, 31, 31)
#define F_C28GP4X2R2P0_PT_RST               FIELD_DEFINE(0x2300, 0, 0)
#define F_C28GP4X2R2P0_PT_LOCK              FIELD_DEFINE(0x2310, 0, 0)
#define F_C28GP4X2R2P0_PT_PASS              FIELD_DEFINE(0x2310, 1, 1)
#define F_C28GP4X2R2P0_PT_CNT_RST           FIELD_DEFINE(0x2300, 7, 7)

/* Temperature Sensor */
#define F_C28GP4X2R2P0_TSEN_ADC_DATA        FIELD_DEFINE(0x0A328, 9, 0)
#define F_C28GP4X2R2P0_TSEN_ADC_MODE        FIELD_DEFINE(0x8314, 2, 1)

/* Eye Opening Measurement */
#define F_C28GP4X2R2P0_ESM_PATH_SEL         FIELD_DEFINE(0x6058, 16, 16)
#define F_C28GP4X2R2P0_ESM_DFE_SPLR_EN      FIELD_DEFINE(0x6058, 13, 10)
#define F_C28GP4X2R2P0_ESM_EN               FIELD_DEFINE(0x6058, 18, 18)
#define F_C28GP4X2R2P0_EOM_READY            FIELD_DEFINE(0x6038, 3, 3)
#define F_C28GP4X2R2P0_ESM_PHASE            FIELD_DEFINE(0x6058, 9, 0)
#define F_C28GP4X2R2P0_ESM_VOLTAGE          FIELD_DEFINE(0x6038, 15, 8)
#define F_C28GP4X2R2P0_DFE_CALL             FIELD_DEFINE(0x6038, 4, 4)
#define F_C28GP4X2R2P0_EOM_VLD_CNT_P_E      FIELD_DEFINE(0x2560, 31, 0)
#define F_C28GP4X2R2P0_EOM_VLD_CNT_P_O      FIELD_DEFINE(0x2568, 31, 0)
#define F_C28GP4X2R2P0_EOM_VLD_CNT_N_E      FIELD_DEFINE(0x2564, 31, 0)
#define F_C28GP4X2R2P0_EOM_VLD_CNT_N_O      FIELD_DEFINE(0x256C, 31, 0)
#define F_C28GP4X2R2P0_EOM_ERR_CNT_P_E      FIELD_DEFINE(0x2570, 31, 0)
#define F_C28GP4X2R2P0_EOM_ERR_CNT_P_O      FIELD_DEFINE(0x2578, 31, 0)
#define F_C28GP4X2R2P0_EOM_ERR_CNT_N_E      FIELD_DEFINE(0x2574, 31, 0)
#define F_C28GP4X2R2P0_EOM_ERR_CNT_N_O      FIELD_DEFINE(0x257C, 31, 0)

/* Miscellaneous */
#define F_C28GP4X2R2P0_TX_MARGIN            FIELD_DEFINE(0x2010, 31, 29)

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* C28GP4X2 */

#endif /* defined MCESD_C28GP4X2_DEFS_H */
