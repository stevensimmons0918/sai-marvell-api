/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions global definitions specific to Marvell
 CE Serdes IP: COMPHY_28G_PIPE4_RPLL_X4_1P2V
********************************************************************/
#ifndef MCESD_C28GP4X4_DEFS_H
#define MCESD_C28GP4X4_DEFS_H

#ifdef C28GP4X4

#if C_LINKAGE
#if defined __cplusplus
    extern "C" {
#endif
#endif

/* MCU Firmware constants */
#define C28GP4X4_FW_MAX_SIZE            16384       /* Firmware max size in DWORDS */
#define C28GP4X4_FW_BASE_ADDR           0x10000     /* Firmware base address */
#define C28GP4X4_XDATA_CMN_MAX_SIZE     512         /* XDATA max size in DWORDS */
#define C28GP4X4_XDATA_LANE_MAX_SIZE    1024        /* XDATA max size in DWORDS */
#define C28GP4X4_XDATA_CMN_BASE_ADDR    0xE000      /* Common XDATA base address */
#define C28GP4X4_XDATA_LANE_BASE_ADDR   0x6000      /* Lane XDATA base address */

/* Squelch Detector Threshold Range constants */
#define C28GP4X4_SQ_THRESH_MIN          0
#define C28GP4X4_SQ_THRESH_MAX          0x1F

/* Temperature Sensor constants */
#define C28GP4X4_TSENE_GAIN             394
#define C28GP4X4_TSENE_OFFSET           128900

/* EOM constants */
#define C28GP4X4_EYE_DEFAULT_VOLT_STEPS     32
#define C28GP4X4_EYE_DEFAULT_PHASE_LEVEL    32
#define C28GP4X4_EYE_MAX_VOLT_STEPS         64
#define C28GP4X4_EYE_MAX_PHASE_LEVEL        128
#define C28GP4X4_EYE_VOLT_OFFSET            4
#define C28GP4X4_EYE_LOW_ERROR_THRESH       10

/* Miscellaneous */
#define C28GP4X4_TOTAL_LANES            4

/* PHY Mode */
typedef enum
{
    C28GP4X4_PHYMODE_SATA = 0,
    C28GP4X4_PHYMODE_SAS = 1,
    C28GP4X4_PHYMODE_PCIE = 3,
    C28GP4X4_PHYMODE_SERDES = 4,
    C28GP4X4_PHYMODE_USB3 = 5
} E_C28GP4X4_PHYMODE;

/* SERDES Speeds */
typedef enum
{
    C28GP4X4_SERDES_1P25G = 0,          /* 1.25 Gbps*/
    C28GP4X4_SERDES_2P5G = 12,          /* 2.5 Gbps */
    C28GP4X4_SERDES_2P57813G = 18,      /* 2.57813 Gbps */
    C28GP4X4_SERDES_3P125G = 1,         /* 3.125 Gbps */
    C28GP4X4_SERDES_5G = 17,            /* 5 Gbps */
    C28GP4X4_SERDES_5P15625G = 2,       /* 5.15625 Gbps */
    C28GP4X4_SERDES_6P25G = 3,          /* 6.25 Gbps */
    C28GP4X4_SERDES_10G = 15,           /* 10 Gbps */
    C28GP4X4_SERDES_10P3125G = 4,       /* 10.3125 Gbps */
    C28GP4X4_SERDES_12P1875G = 5,       /* 12.1875 Gbps */
    C28GP4X4_SERDES_12P5G = 6,          /* 12.5 Gbps */
    C28GP4X4_SERDES_20P625G_RING = 13,  /* 20.625 Gbps (RING) */
    C28GP4X4_SERDES_20P625G_LC = 14,    /* 20.625 Gbps (LC) */
    C28GP4X4_SERDES_25P78125G = 7,      /* 25.78125 Gbps */
    C28GP4X4_SERDES_26P5625G = 16,      /* 26.5625 Gbps */
    C28GP4X4_SERDES_27P5G = 8,          /* 27.5 Gbps */
    C28GP4X4_SERDES_28P125G = 9         /* 28.125 Gbps */
} E_C28GP4X4_SERDES_SPEED;

/* Reference Frequency Clock */
typedef enum
{
    C28GP4X4_REFFREQ_25MHZ = 0,    /* 25 MHz */
    C28GP4X4_REFFREQ_30MHZ = 1,    /* 30 MHz */
    C28GP4X4_REFFREQ_40MHZ = 2,    /* 40 MHz */
    C28GP4X4_REFFREQ_50MHZ = 3,    /* 50 MHz */
    C28GP4X4_REFFREQ_62P25MHZ = 4, /* 62.25 MHz */
    C28GP4X4_REFFREQ_100MHZ = 5,   /* 100 MHz */
    C28GP4X4_REFFREQ_125MHZ = 6,   /* 125 MHz */
    C28GP4X4_REFFREQ_156P25MHZ = 7 /* 156.25 MHz */
} E_C28GP4X4_REFFREQ;

/* Reference Clock selection Group */
typedef enum
{
    C28GP4X4_REFCLK_SEL_GROUP1 = 0, /* PIN_REFCLKC_IN_SIDE_A_G1 or PIN_REFCLKC_IN_SIDE_B_G1 */
    C28GP4X4_REFCLK_SEL_GROUP2 = 1  /* PIN_REFCLKC_IN_SIDE_A_G2 or PIN_REFCLKC_IN_SIDE_B_G2 */
} E_C28GP4X4_REFCLK_SEL;

/* TX and RX Data Bus Width */
typedef enum
{
    C28GP4X4_DATABUS_40BIT = 0,
    C28GP4X4_DATABUS_32BIT = 1
} E_C28GP4X4_DATABUS_WIDTH;

/* TX and RX Polarity */
typedef enum
{
    C28GP4X4_POLARITY_NORMAL = 0,
    C28GP4X4_POLARITY_INVERTED = 1
} E_C28GP4X4_POLARITY;

/* Data Path */
typedef enum
{
    C28GP4X4_PATH_LOCAL_ANALOG_LB   = 0,
    C28GP4X4_PATH_EXTERNAL = 1,
    C28GP4X4_PATH_FAR_END_LB = 2,
    C28GP4X4_PATH_UNKNOWN = 3
} E_C28GP4X4_DATAPATH;

/* Training Type */
typedef enum
{
    C28GP4X4_TRAINING_TRX = 0,
    C28GP4X4_TRAINING_RX = 1
} E_C28GP4X4_TRAINING;

/* Training Timeout */
typedef struct
{
    MCESD_BOOL enable;
    MCESD_U16 timeout; /* milliseconds */
} S_C28GP4X4_TRAINING_TIMEOUT;

/* Hardware Pins */
typedef enum
{
    C28GP4X4_PIN_RESET = 0,
    C28GP4X4_PIN_ISOLATION_ENB = 1,
    C28GP4X4_PIN_BG_RDY = 2,
    C28GP4X4_PIN_SIF_SEL = 3,
    C28GP4X4_PIN_MCU_CLK = 4,
    C28GP4X4_PIN_DIRECT_ACCESS_EN = 5,
    C28GP4X4_PIN_PHY_MODE = 6,
    C28GP4X4_PIN_REFCLK_SEL = 7,
    C28GP4X4_PIN_REF_FREF_SEL = 8,
    C28GP4X4_PIN_PHY_GEN_TX0 = 9,
    C28GP4X4_PIN_PHY_GEN_TX1 = 10,
    C28GP4X4_PIN_PHY_GEN_TX2 = 11,
    C28GP4X4_PIN_PHY_GEN_TX3 = 12,
    C28GP4X4_PIN_PHY_GEN_RX0 = 13,
    C28GP4X4_PIN_PHY_GEN_RX1 = 14,
    C28GP4X4_PIN_PHY_GEN_RX2 = 15,
    C28GP4X4_PIN_PHY_GEN_RX3 = 16,
    C28GP4X4_PIN_DFE_EN0 = 17,
    C28GP4X4_PIN_DFE_EN1 = 18,
    C28GP4X4_PIN_DFE_EN2 = 19,
    C28GP4X4_PIN_DFE_EN3 = 20,
    C28GP4X4_PIN_PU_PLL0 = 21,
    C28GP4X4_PIN_PU_PLL1 = 22,
    C28GP4X4_PIN_PU_PLL2 = 23,
    C28GP4X4_PIN_PU_PLL3 = 24,
    C28GP4X4_PIN_PU_RX0 = 25,
    C28GP4X4_PIN_PU_RX1 = 26,
    C28GP4X4_PIN_PU_RX2 = 27,
    C28GP4X4_PIN_PU_RX3 = 28,
    C28GP4X4_PIN_PU_TX0 = 29,
    C28GP4X4_PIN_PU_TX1 = 30,
    C28GP4X4_PIN_PU_TX2 = 31,
    C28GP4X4_PIN_PU_TX3 = 32,
    C28GP4X4_PIN_TX_IDLE0 = 33,
    C28GP4X4_PIN_TX_IDLE1 = 34,
    C28GP4X4_PIN_TX_IDLE2 = 35,
    C28GP4X4_PIN_TX_IDLE3 = 36,
    C28GP4X4_PIN_PU_IVREF = 37,
    C28GP4X4_PIN_RX_TRAIN_ENABLE0 = 38,
    C28GP4X4_PIN_RX_TRAIN_ENABLE1 = 39,
    C28GP4X4_PIN_RX_TRAIN_ENABLE2 = 40,
    C28GP4X4_PIN_RX_TRAIN_ENABLE3 = 41,
    C28GP4X4_PIN_RX_TRAIN_COMPLETE0 = 42,
    C28GP4X4_PIN_RX_TRAIN_COMPLETE1 = 43,
    C28GP4X4_PIN_RX_TRAIN_COMPLETE2 = 44,
    C28GP4X4_PIN_RX_TRAIN_COMPLETE3 = 45,
    C28GP4X4_PIN_RX_TRAIN_FAILED0 = 46,
    C28GP4X4_PIN_RX_TRAIN_FAILED1 = 47,
    C28GP4X4_PIN_RX_TRAIN_FAILED2 = 48,
    C28GP4X4_PIN_RX_TRAIN_FAILED3 = 49,
    C28GP4X4_PIN_TX_TRAIN_ENABLE0 = 50,
    C28GP4X4_PIN_TX_TRAIN_ENABLE1 = 51,
    C28GP4X4_PIN_TX_TRAIN_ENABLE2 = 52,
    C28GP4X4_PIN_TX_TRAIN_ENABLE3 = 53,
    C28GP4X4_PIN_TX_TRAIN_COMPLETE0 = 54,
    C28GP4X4_PIN_TX_TRAIN_COMPLETE1 = 55,
    C28GP4X4_PIN_TX_TRAIN_COMPLETE2 = 56,
    C28GP4X4_PIN_TX_TRAIN_COMPLETE3 = 57,
    C28GP4X4_PIN_TX_TRAIN_FAILED0 = 58,
    C28GP4X4_PIN_TX_TRAIN_FAILED1 = 59,
    C28GP4X4_PIN_TX_TRAIN_FAILED2 = 60,
    C28GP4X4_PIN_TX_TRAIN_FAILED3 = 61,
    C28GP4X4_PIN_SQ_DETECTED_LPF0 = 62,
    C28GP4X4_PIN_SQ_DETECTED_LPF1 = 63,
    C28GP4X4_PIN_SQ_DETECTED_LPF2 = 64,
    C28GP4X4_PIN_SQ_DETECTED_LPF3 = 65,
    C28GP4X4_PIN_RX_INIT0 = 66,
    C28GP4X4_PIN_RX_INIT1 = 67,
    C28GP4X4_PIN_RX_INIT2 = 68,
    C28GP4X4_PIN_RX_INIT3 = 69,
    C28GP4X4_PIN_RX_INIT_DONE0 = 70,
    C28GP4X4_PIN_RX_INIT_DONE1 = 71,
    C28GP4X4_PIN_RX_INIT_DONE2 = 72,
    C28GP4X4_PIN_RX_INIT_DONE3 = 73,
    C28GP4X4_PIN_PRAM_SOC_EN = 74,
    C28GP4X4_PIN_DFE_PAT_DIS0 = 75,
    C28GP4X4_PIN_DFE_PAT_DIS1 = 76,
    C28GP4X4_PIN_DFE_PAT_DIS2 = 77,
    C28GP4X4_PIN_DFE_PAT_DIS3 = 78
} E_C28GP4X4_PIN;

/* TX Equalization Parameters */
typedef enum
{
    C28GP4X4_TXEQ_EM_POST_CTRL = 0,
    C28GP4X4_TXEQ_EM_PEAK_CTRL = 1,
    C28GP4X4_TXEQ_EM_PRE_CTRL = 2,
    C28GP4X4_TXEQ_MARGIN = 3
} E_C28GP4X4_TXEQ_PARAM;

/* CTLE Parameters */
typedef enum
{
    C28GP4X4_CTLE_DATA_RATE = 0,
    C28GP4X4_CTLE_RES1_SEL = 1,
    C28GP4X4_CTLE_RES2_SEL = 2,
    C28GP4X4_CTLE_CAP1_SEL = 3,
    C28GP4X4_CTLE_CAP2_SEL = 4
} E_C28GP4X4_CTLE_PARAM;

/* CDR Parameters */
typedef enum
{
    C28GP4X4_CDR_SELMUFI = 0,
    C28GP4X4_CDR_SELMUFF = 1,
    C28GP4X4_CDR_SELMUPI = 2,
    C28GP4X4_CDR_SELMUPF = 3
} E_C28GP4X4_CDR_PARAM;

typedef enum
{
    C28GP4X4_SR_EN_DISABLE         = 0,
    C28GP4X4_SR_EN_6GBPS_TO_12GBPS = 1,
    C28GP4X4_SR_EN_UNSUPPORTED     = 2,
    C28GP4X4_SR_EN_LESS_THAN_6GBPS = 3
} E_C28GP4X4_SLEWRATE_EN;

/* Slew rate Parameters */
typedef enum
{
    C28GP4X4_SR_CTRL0 = 0,
    C28GP4X4_SR_CTRL1 = 1
} E_C28GP4X4_SLEWRATE_PARAM;

/* Pattern selection */
typedef enum
{
    C28GP4X4_PAT_USER               = 0,
    C28GP4X4_PAT_JITTER_K28P5       = 0x8,
    C28GP4X4_PAT_JITTER_1T          = 0x9,
    C28GP4X4_PAT_JITTER_2T          = 0xA,
    C28GP4X4_PAT_JITTER_4T          = 0xB,
    C28GP4X4_PAT_JITTER_5T          = 0xC,
    C28GP4X4_PAT_JITTER_8T          = 0xD,
    C28GP4X4_PAT_JITTER_10T         = 0xE,
    C28GP4X4_PAT_PRBS7              = 0x10,
    C28GP4X4_PAT_PRBS9              = 0x11,
    C28GP4X4_PAT_PRBS11             = 0x12,
    C28GP4X4_PAT_PRBS11_0           = 0x13,
    C28GP4X4_PAT_PRBS11_1           = 0x14,
    C28GP4X4_PAT_PRBS11_2           = 0x15,
    C28GP4X4_PAT_PRBS11_3           = 0x16,
    C28GP4X4_PAT_PRBS15             = 0x17,
    C28GP4X4_PAT_PRBS16             = 0x18,
    C28GP4X4_PAT_PRBS23             = 0x19,
    C28GP4X4_PAT_PRBS31             = 0x1A,
    C28GP4X4_PAT_PRBS32             = 0x1B,
    C28GP4X4_PAT_SATA_LTDP          = 0x20,
    C28GP4X4_PAT_SATA_HTDP          = 0x21,
    C28GP4X4_PAT_SATA_LFSCP         = 0x22,
    C28GP4X4_PAT_SATA_SSOP          = 0x23,
    C28GP4X4_PAT_SATA_LBP           = 0x24,
    C28GP4X4_PAT_SATA_COMP          = 0x25,
    C28GP4X4_PAT_SAS_JTPAT          = 0x28,
    C28GP4X4_PAT_SAS_MJTPAT         = 0x29,
    C28GP4X4_PAT_SAS_CJTPAT         = 0x2A,
    C28GP4X4_PAT_SAS_DFE_TRAIN      = 0x2B,
    C28GP4X4_PAT_SAS_DFE_TRAIN_DONE = 0x2C,
    C28GP4X4_PAT_SAS_58_SCRAMB_IDLE = 0x2D
} E_C28GP4X4_PATTERN;

/* Pattern: 8B10B Encoding (Optional for PRBS and USER patterns) */
typedef enum
{
    C28GP4X4_ENC_8B10B_DISABLE = 0,
    C28GP4X4_ENC_8B10B_ENABLE = 1,
} E_C28GP4X4_ENC_8B10B;

/* Pattern: SATA Long/Short Pattern */
typedef enum
{
    C28GP4X4_SATA_SHORT = 0,
    C28GP4X4_SATA_LONG = 1,
    C28GP4X4_SATA_NOT_USED = 2
} E_C28GP4X4_SATA_LONGSHORT;

/* Pattern: SATA Initial Disparity*/
typedef enum
{
    C28GP4X4_DISPARITY_NEGATIVE = 0,
    C28GP4X4_DISPARITY_POSITIVE = 1,
    C28GP4X4_DISPARITY_NOT_USED = 2
} E_C28GP4X4_SATA_INITIAL_DISPARITY;

/* CTLE Preset */
typedef enum
{
    C28GP4X4_CTLE_PRESET_DEFAULT    = 0,
    C28GP4X4_CTLE_PRESET_RES1       = 1,
    C28GP4X4_CTLE_PRESET_CAP1       = 2,
    C28GP4X4_CTLE_PRESET_RES2_O     = 3,
    C28GP4X4_CTLE_PRESET_RES2_E     = 4,
    C28GP4X4_CTLE_PRESET_CAP2_O     = 5,
    C28GP4X4_CTLE_PRESET_CAP2_E     = 6,
    C28GP4X4_CTLE_PRESET_PHASE      = 7
} E_C28GP4X4_CTLE_PRESET;

/* Pattern Context */
typedef struct
{
    E_C28GP4X4_PATTERN pattern;
    E_C28GP4X4_ENC_8B10B enc8B10B;   /* 8B10B encoding is optional for PRBS and User patterns */
} S_C28GP4X4_PATTERN;

/* Pattern Comparator Statistics */
typedef struct
{
    MCESD_BOOL lock;
    MCESD_BOOL pass;
    MCESD_U64 totalBits;
    MCESD_U64 totalErrorBits;
} S_C28GP4X4_PATTERN_STATISTICS;

/* Trained Eye Height */
typedef struct
{
    MCESD_U8 f0a;
    MCESD_U8 f0aMax;
    MCESD_U8 f0b;
    MCESD_U8 f0d;
} S_C28GP4X4_TRAINED_EYE_HEIGHT;

/* Eye Measurement Data @ X,Y and X,-Y */
typedef struct
{
    MCESD_32 phase;
    MCESD_U8 voltage;
    MCESD_U32 upperBitCount;
    MCESD_U32 upperBitErrorCount;
    MCESD_U32 lowerBitCount;
    MCESD_U32 lowerBitErrorCount;
} S_C28GP4X4_EOM_DATA;

/* DFE Taps */
typedef enum
{
    C28GP4X4_DFE_F0_N   = 0, /* F0 Negative */
    C28GP4X4_DFE_F0_P   = 1, /* F0 Positive */
    C28GP4X4_DFE_F1     = 2,
    C28GP4X4_DFE_F2_N   = 3, /* F2 Negative */
    C28GP4X4_DFE_F2_P   = 4, /* F2 Positive */
    C28GP4X4_DFE_F3_N   = 5, /* F3 Negative */
    C28GP4X4_DFE_F3_P   = 6, /* F3 Positive */
    C28GP4X4_DFE_F4     = 7,
    C28GP4X4_DFE_F5     = 8,
    C28GP4X4_DFE_F6     = 9,
    C28GP4X4_DFE_F7     = 10,
    C28GP4X4_DFE_F8     = 11,
    C28GP4X4_DFE_F9     = 12,
    C28GP4X4_DFE_F10    = 13,
    C28GP4X4_DFE_F11    = 14,
    C28GP4X4_DFE_F12    = 15,
    C28GP4X4_DFE_F13    = 16,
    C28GP4X4_DFE_F14    = 17,
    C28GP4X4_DFE_F15    = 18,
    C28GP4X4_DFE_FF0    = 19, /* floating tap 0 */
    C28GP4X4_DFE_FF1    = 20, /* floating tap 1 */
    C28GP4X4_DFE_FF2    = 21, /* floating tap 2 */
    C28GP4X4_DFE_FF3    = 22, /* floating tap 3 */
    C28GP4X4_DFE_FF4    = 23, /* floating tap 4 */
    C28GP4X4_DFE_FF5    = 24  /* floating tap 5 */
} E_C28GP4X4_DFE_TAP;

/* Eye Raw Data */
typedef struct
{
    MCESD_32 eyeRawData[(C28GP4X4_EYE_MAX_PHASE_LEVEL * 2) + 1][(C28GP4X4_EYE_MAX_VOLT_STEPS * 2) + 1];
} S_C28GP4X4_EYE_RAW, *S_C28GP4X4_EYE_RAW_PTR;

typedef struct
{
    MCESD_U32  powerLaneMask;
    MCESD_BOOL initTx;
    MCESD_BOOL initRx;
    MCESD_BOOL txOutputEn;
    MCESD_BOOL downloadFw;
    E_C28GP4X4_DATAPATH dataPath;
    E_C28GP4X4_REFCLK_SEL refClkSel;
    E_C28GP4X4_DATABUS_WIDTH dataBusWidth;
    E_C28GP4X4_SERDES_SPEED speed;
    E_C28GP4X4_REFFREQ refFreq;
    MCESD_U32 *fwCodePtr;
    MCESD_U32 fwCodeSizeDW;
    MCESD_U32* cmnXDataPtr;
    MCESD_U32 cmnXDataSizeDW;
    MCESD_U32* laneXDataPtr;
    MCESD_U32 laneXDataSizeDW;
} S_C28GP4X4_PowerOn;

/* DRO Data */
typedef struct
{
    MCESD_U16 dro[10];
} S_C28GP4X4_DRO_DATA;

#define F_C28GP4X4R1P0_MAJOR_REV        FIELD_DEFINE(0xA3F8, 23, 20)
#define F_C28GP4X4R1P0_MINOR_REV        FIELD_DEFINE(0xA3F8, 19, 16)
#define F_C28GP4X4R1P0_MCU_EN_LANE0     FIELD_DEFINE(0xA200, 0, 0)
#define F_C28GP4X4R1P0_MCU_EN_LANE1     FIELD_DEFINE(0xA200, 1, 1)
#define F_C28GP4X4R1P0_MCU_EN_LANE2     FIELD_DEFINE(0xA200, 2, 2)
#define F_C28GP4X4R1P0_MCU_EN_LANE3     FIELD_DEFINE(0xA200, 3, 3)
#define F_C28GP4X4R1P0_MCU_EN_CMN       FIELD_DEFINE(0xA200, 4, 4)
#define F_C28GP4X4R1P0_EN_LANE0         FIELD_DEFINE(0xA334, 0, 0)
#define F_C28GP4X4R1P0_EN_LANE1         FIELD_DEFINE(0xA334, 1, 1)
#define F_C28GP4X4R1P0_EN_LANE2         FIELD_DEFINE(0xA334, 2, 2)
#define F_C28GP4X4R1P0_EN_LANE3         FIELD_DEFINE(0xA334, 3, 3)
#define F_C28GP4X4R1P0_EN_CMN           FIELD_DEFINE(0xA334, 4, 4)
#define F_C28GP4X4R1P0_BROADCAST        FIELD_DEFINE(0xA314, 27, 27)
#define E_C28GP4X4R1P0_LANE_SEL         FIELD_DEFINE(0xA314, 31, 29)
#define F_C28GP4X4R1P0_RX_CK_SEL        FIELD_DEFINE(0x0258, 4, 4)
#define F_C28GP4X4R1P0_PLL_LOCK_RING_RD FIELD_DEFINE(0x2074, 17, 17)
#define F_C28GP4X4R1P0_PLL_LOCK_LCPLL   FIELD_DEFINE(0xA318, 14, 14)
#define F_C28GP4X4R1P0_PLL_READY_TX     FIELD_DEFINE(0x2000, 20, 20)
#define F_C28GP4X4R1P0_PLL_READY_RX     FIELD_DEFINE(0x2100, 24, 24)
#define F_C28GP4X4R1P0_CDR_LOCK         FIELD_DEFINE(0x213C, 30, 30)
#define F_C28GP4X4R1P0_ALIGN90_REF      FIELD_DEFINE(0x0038, 7, 2)
#define F_C28GP4X4R1P0_SQ_THRESH        FIELD_DEFINE(0x0010, 7, 2)
#define F_C28GP4X4R1P0_PHY_GEN_MAX      FIELD_DEFINE(0xE62C, 4, 0)
#define F_C28GP4X4R1P0_MCU_STATUS0      FIELD_DEFINE(0x2230, 31, 0)

/* Data Bus Width */
#define F_C28GP4X4R1P0_TX_SEL_BITS      FIELD_DEFINE(0x2034, 31, 31)
#define F_C28GP4X4R1P0_RX_SEL_BITS      FIELD_DEFINE(0x2104, 31, 31)

/* TX/RX Polarity */
#define F_C28GP4X4R1P0_TXD_INV          FIELD_DEFINE(0x2024, 30, 30)
#define F_C28GP4X4R1P0_RXD_INV          FIELD_DEFINE(0x2148, 29, 29)

/* MCU Firmware Version */
#define F_C28GP4X4R1P0_FW_MAJOR_VER     FIELD_DEFINE(0xE600, 31, 24)
#define F_C28GP4X4R1P0_FW_MINOR_VER     FIELD_DEFINE(0xE600, 23, 16)
#define F_C28GP4X4R1P0_FW_PATCH_VER     FIELD_DEFINE(0xE600, 15, 8)
#define F_C28GP4X4R1P0_FW_BUILD_VER     FIELD_DEFINE(0xE600, 7, 0)
#define F_C28GP4X4R1P0_PROG_RAM_SEL     FIELD_DEFINE(0xA21C, 4, 3)

/* TX Equalization */
#define F_C28GP4X4R1P0_TX_EM_PO_CTRL      FIELD_DEFINE(0x2634, 7, 4)
#define F_C28GP4X4R1P0_TX_EM_PEAK_CTRL    FIELD_DEFINE(0x2634, 15, 12)
#define F_C28GP4X4R1P0_TX_EM_PRE_CTRL     FIELD_DEFINE(0x2634, 11, 8)
#define F_C28GP4X4R1P0_TX_EM_PO_CTRL_EN   FIELD_DEFINE(0x2634, 21, 21)
#define F_C28GP4X4R1P0_TX_EM_PEAK_CTRL_EN FIELD_DEFINE(0x2634, 23, 23)
#define F_C28GP4X4R1P0_TX_EM_PRE_CTRL_EN  FIELD_DEFINE(0x2634, 22, 22)

/* TX Data Error Injection */
#define F_C28GP4X4R1P0_ADD_ERR_EN       FIELD_DEFINE(0x2024, 29, 29)
#define F_C28GP4X4R1P0_ADD_ERR_NUM      FIELD_DEFINE(0x2024, 28, 26)

/* CTLE */
#define F_C28GP4X4R1P0_FFE_DATA_RATE    FIELD_DEFINE(0x0208, 7, 4)
#define F_C28GP4X4R1P0_RES1_SEL         FIELD_DEFINE(0x0200, 7, 4)
#define F_C28GP4X4R1P0_RES2_SEL_E       FIELD_DEFINE(0x0000, 7, 4)
#define F_C28GP4X4R1P0_RES2_SEL_O       FIELD_DEFINE(0x0204, 7, 4)
#define F_C28GP4X4R1P0_CAP1_SEL         FIELD_DEFINE(0x0200, 3, 0)
#define F_C28GP4X4R1P0_CAP2_SEL_E       FIELD_DEFINE(0x0000, 3, 0)
#define F_C28GP4X4R1P0_CAP2_SEL_O       FIELD_DEFINE(0x0204, 3, 0)

/* CDR */
#define F_C28GP4X4R1P0_RX_SELMUFI       FIELD_DEFINE(0x2164, 12, 10)
#define F_C28GP4X4R1P0_RX_SELMUFF       FIELD_DEFINE(0x2164, 15, 13)
#define F_C28GP4X4R1P0_REG_SELMUPI      FIELD_DEFINE(0x022C, 3, 0)
#define F_C28GP4X4R1P0_REG_SELMUPF      FIELD_DEFINE(0x0230, 7, 4)

/* Slew Rate*/
#define F_C28GP4X4R1P0_SLEWRATE_EN      FIELD_DEFINE(0x004C, 7, 6)
#define F_C28GP4X4R1P0_SLEWCTRL0        FIELD_DEFINE(0x004C, 5, 4)
#define F_C28GP4X4R1P0_SLEWCTRL1        FIELD_DEFINE(0x004C, 3, 2)

/* DFE */
#define F_C28GP4X4R1P0_DFE_F0_D_N_E     FIELD_DEFINE(0x2480, 13, 8)
#define F_C28GP4X4R1P0_DFE_F0_D_P_E     FIELD_DEFINE(0x2480, 5, 0)
#define F_C28GP4X4R1P0_DFE_F0_D_N_O     FIELD_DEFINE(0x24A0, 13, 8)
#define F_C28GP4X4R1P0_DFE_F0_D_P_O     FIELD_DEFINE(0x24A0, 5, 0)
#define F_C28GP4X4R1P0_DFE_F1_E         FIELD_DEFINE(0x248C, 5, 0)
#define F_C28GP4X4R1P0_DFE_F1_O         FIELD_DEFINE(0x24AC, 5, 0)
#define F_C28GP4X4R1P0_DFE_F2_D_N_E     FIELD_DEFINE(0x2484, 13, 8)
#define F_C28GP4X4R1P0_DFE_F2_D_P_E     FIELD_DEFINE(0x2484, 5, 0)
#define F_C28GP4X4R1P0_DFE_F2_D_N_O     FIELD_DEFINE(0x24A4, 13, 8)
#define F_C28GP4X4R1P0_DFE_F2_D_P_O     FIELD_DEFINE(0x24A4, 5, 0)
#define F_C28GP4X4R1P0_DFE_F3_D_N_E     FIELD_DEFINE(0x2488, 12, 8)
#define F_C28GP4X4R1P0_DFE_F3_D_P_E     FIELD_DEFINE(0x2488, 4, 0)
#define F_C28GP4X4R1P0_DFE_F3_D_N_O     FIELD_DEFINE(0x24A8, 12, 8)
#define F_C28GP4X4R1P0_DFE_F3_D_P_O     FIELD_DEFINE(0x24A8, 4, 0)
#define F_C28GP4X4R1P0_DFE_F4_E         FIELD_DEFINE(0x248C, 13, 8)
#define F_C28GP4X4R1P0_DFE_F4_O         FIELD_DEFINE(0x24AC, 13, 8)
#define F_C28GP4X4R1P0_DFE_F5_E         FIELD_DEFINE(0x248C, 21, 16)
#define F_C28GP4X4R1P0_DFE_F5_O         FIELD_DEFINE(0x24AC, 21, 16)
#define F_C28GP4X4R1P0_DFE_F6_E         FIELD_DEFINE(0x248C, 29, 24)
#define F_C28GP4X4R1P0_DFE_F6_O         FIELD_DEFINE(0x24AC, 29, 24)
#define F_C28GP4X4R1P0_DFE_F7_E         FIELD_DEFINE(0x2490, 4, 0)
#define F_C28GP4X4R1P0_DFE_F7_O         FIELD_DEFINE(0x24B0, 4, 0)
#define F_C28GP4X4R1P0_DFE_F8_E         FIELD_DEFINE(0x2490, 12, 8)
#define F_C28GP4X4R1P0_DFE_F8_O         FIELD_DEFINE(0x24B0, 12, 8)
#define F_C28GP4X4R1P0_DFE_F9_E         FIELD_DEFINE(0x2490, 20, 16)
#define F_C28GP4X4R1P0_DFE_F9_O         FIELD_DEFINE(0x24B0, 20, 16)
#define F_C28GP4X4R1P0_DFE_F10_E        FIELD_DEFINE(0x2490, 28, 24)
#define F_C28GP4X4R1P0_DFE_F10_O        FIELD_DEFINE(0x24B0, 28, 24)
#define F_C28GP4X4R1P0_DFE_F11_E        FIELD_DEFINE(0x2494, 4, 0)
#define F_C28GP4X4R1P0_DFE_F11_O        FIELD_DEFINE(0x24B4, 4, 0)
#define F_C28GP4X4R1P0_DFE_F12_E        FIELD_DEFINE(0x2494, 12, 8)
#define F_C28GP4X4R1P0_DFE_F12_O        FIELD_DEFINE(0x24B4, 12, 8)
#define F_C28GP4X4R1P0_DFE_F13_E        FIELD_DEFINE(0x2494, 20, 16)
#define F_C28GP4X4R1P0_DFE_F13_O        FIELD_DEFINE(0x24B4, 20, 16)
#define F_C28GP4X4R1P0_DFE_F14_E        FIELD_DEFINE(0x2494, 28, 24)
#define F_C28GP4X4R1P0_DFE_F14_O        FIELD_DEFINE(0x24B4, 28, 24)
#define F_C28GP4X4R1P0_DFE_F15_E        FIELD_DEFINE(0x2498, 4, 0)
#define F_C28GP4X4R1P0_DFE_F15_O        FIELD_DEFINE(0x24B8, 4, 0)
#define F_C28GP4X4R1P0_DFE_FF0_E        FIELD_DEFINE(0x2498, 13, 8)
#define F_C28GP4X4R1P0_DFE_FF0_O        FIELD_DEFINE(0x24B8, 13, 8)
#define F_C28GP4X4R1P0_DFE_FF1_E        FIELD_DEFINE(0x2498, 21, 16)
#define F_C28GP4X4R1P0_DFE_FF1_O        FIELD_DEFINE(0x24B8, 21, 16)
#define F_C28GP4X4R1P0_DFE_FF2_E        FIELD_DEFINE(0x2498, 29, 24)
#define F_C28GP4X4R1P0_DFE_FF2_O        FIELD_DEFINE(0x24B8, 29, 24)
#define F_C28GP4X4R1P0_DFE_FF3_E        FIELD_DEFINE(0x249C, 5, 0)
#define F_C28GP4X4R1P0_DFE_FF3_O        FIELD_DEFINE(0x24BC, 5, 0)
#define F_C28GP4X4R1P0_DFE_FF4_E        FIELD_DEFINE(0x249C, 13, 8)
#define F_C28GP4X4R1P0_DFE_FF4_O        FIELD_DEFINE(0x24BC, 13, 8)
#define F_C28GP4X4R1P0_DFE_FF5_E        FIELD_DEFINE(0x249C, 21, 16)
#define F_C28GP4X4R1P0_DFE_FF5_O        FIELD_DEFINE(0x24BC, 21, 16)

/* TRX Training */
#define F_C28GP4X4R1P0_TRAIN_F0A        FIELD_DEFINE(0x6048, 15, 8)
#define F_C28GP4X4R1P0_TRAIN_F0A_MAX    FIELD_DEFINE(0x6048, 23, 16)
#define F_C28GP4X4R1P0_TRAIN_F0B        FIELD_DEFINE(0x6048, 31, 24)
#define F_C28GP4X4R1P0_TRAIN_F0D        FIELD_DEFINE(0x6048, 7, 0)
#define F_C28GP4X4R1P0_TRX_TRAIN_TIMER  FIELD_DEFINE(0x602C, 15, 0)
#define F_C28GP4X4R1P0_RX_TRAIN_TIMER   FIELD_DEFINE(0x6028, 15, 0)
#define F_C28GP4X4R1P0_TX_TRAIN_TIMEREN FIELD_DEFINE(0x6030, 29, 29)
#define F_C28GP4X4R1P0_RX_TRAIN_TIMEREN FIELD_DEFINE(0x6030, 30, 30)

/* Data Path */
#define F_C28GP4X4R1P0_DIG_RX2TX_LPBKEN FIELD_DEFINE(0x2024, 31, 31)
#define F_C28GP4X4R1P0_ANA_TX2RX_LPBKEN FIELD_DEFINE(0x0244, 6, 6)
#define F_C28GP4X4R1P0_DIG_TX2RX_LPBKEN FIELD_DEFINE(0x2148, 31, 31)
#define F_C28GP4X4R1P0_DTX_FLOOP_EN     FIELD_DEFINE(0xA018, 16, 16)
#define F_C28GP4X4R1P0_DTX_FLOOP_RING   FIELD_DEFINE(0x2060, 10, 10)
#define F_C28GP4X4R1P0_RX_FOFFSET_DIS   FIELD_DEFINE(0x2168, 19, 19)
#define F_C28GP4X4R1P0_DTX_FOFFSET      FIELD_DEFINE(0xA018, 18, 18)
#define F_C28GP4X4R1P0_DTX_FOFFSET_RING FIELD_DEFINE(0x2068, 24, 24)
#define F_C28GP4X4R1P0_PU_LB            FIELD_DEFINE(0x0208, 3, 3)
#define F_C28GP4X4R1P0_PU_LB_DLY        FIELD_DEFINE(0x0208, 2, 2)
#define F_C28GP4X4R1P0_DTL_SQ_DET_EN    FIELD_DEFINE(0x2160, 13, 13)

/* PHY Test */
#define F_C28GP4X4R1P0_PT_TX_PATT_SEL   FIELD_DEFINE(0x2300, 27, 22)
#define F_C28GP4X4R1P0_PT_RX_PATT_SEL   FIELD_DEFINE(0x2300, 21, 16)
#define F_C28GP4X4R1P0_PT_START_RD      FIELD_DEFINE(0x2300, 6, 6)
#define F_C28GP4X4R1P0_PT_SATA_LONG     FIELD_DEFINE(0x2304, 22, 22)
#define F_C28GP4X4R1P0_PT_PRBS_ENC      FIELD_DEFINE(0x2304, 23, 23)
#define F_C28GP4X4R1P0_PT_CNT_47_16     FIELD_DEFINE(0x2314, 31, 0)
#define F_C28GP4X4R1P0_PT_CNT_15_0      FIELD_DEFINE(0x2318, 31, 16)
#define F_C28GP4X4R1P0_PT_ERR_CNT       FIELD_DEFINE(0x231C, 31, 0)
#define F_C28GP4X4R1P0_PT_USER_PAT_7948 FIELD_DEFINE(0x2308, 31, 0)
#define F_C28GP4X4R1P0_PT_USER_PAT_4716 FIELD_DEFINE(0x230C, 31, 0)
#define F_C28GP4X4R1P0_PT_USER_PAT_1500 FIELD_DEFINE(0x2310, 31, 16)
#define F_C28GP4X4R1P0_PT_USER_K_CHAR   FIELD_DEFINE(0x2310, 15, 8)
#define F_C28GP4X4R1P0_PT_PHYREADYFORCE FIELD_DEFINE(0x2300, 28, 28)
#define F_C28GP4X4R1P0_PT_EN_MODE       FIELD_DEFINE(0x2300, 30, 29)
#define F_C28GP4X4R1P0_PT_EN            FIELD_DEFINE(0x2300, 31, 31)
#define F_C28GP4X4R1P0_PT_RST           FIELD_DEFINE(0x2300, 0, 0)
#define F_C28GP4X4R1P0_PT_LOCK          FIELD_DEFINE(0x2310, 0, 0)
#define F_C28GP4X4R1P0_PT_PASS          FIELD_DEFINE(0x2310, 1, 1)
#define F_C28GP4X4R1P0_PT_CNT_RST       FIELD_DEFINE(0x2300, 7, 7)

/* Temperature Sensor */
#define F_C28GP4X4R1P0_TSEN_ADC_DATA    FIELD_DEFINE(0x0A328, 9, 0)
#define F_C28GP4X4R1P0_TSEN_ADC_MODE    FIELD_DEFINE(0x8314, 2, 1)

/* Eye Opening Measurement */
#define F_C28GP4X4R1P0_ESM_PATH_SEL     FIELD_DEFINE(0x6058, 16, 16)
#define F_C28GP4X4R1P0_ESM_DFE_SPLR_EN  FIELD_DEFINE(0x6058, 13, 10)
#define F_C28GP4X4R1P0_ESM_EN           FIELD_DEFINE(0x6058, 18, 18)
#define F_C28GP4X4R1P0_EOM_READY        FIELD_DEFINE(0x6038, 3, 3)
#define F_C28GP4X4R1P0_ESM_PHASE        FIELD_DEFINE(0x6058, 9, 0)
#define F_C28GP4X4R1P0_ESM_VOLTAGE      FIELD_DEFINE(0x6038, 15, 8)
#define F_C28GP4X4R1P0_DFE_CALL         FIELD_DEFINE(0x6038, 4, 4)
#define F_C28GP4X4R1P0_EOM_VLD_CNT_P_E  FIELD_DEFINE(0x2560, 31, 0)
#define F_C28GP4X4R1P0_EOM_VLD_CNT_P_O  FIELD_DEFINE(0x2568, 31, 0)
#define F_C28GP4X4R1P0_EOM_VLD_CNT_N_E  FIELD_DEFINE(0x2564, 31, 0)
#define F_C28GP4X4R1P0_EOM_VLD_CNT_N_O  FIELD_DEFINE(0x256C, 31, 0)
#define F_C28GP4X4R1P0_EOM_ERR_CNT_P_E  FIELD_DEFINE(0x2570, 31, 0)
#define F_C28GP4X4R1P0_EOM_ERR_CNT_P_O  FIELD_DEFINE(0x2578, 31, 0)
#define F_C28GP4X4R1P0_EOM_ERR_CNT_N_E  FIELD_DEFINE(0x2574, 31, 0)
#define F_C28GP4X4R1P0_EOM_ERR_CNT_N_O  FIELD_DEFINE(0x257C, 31, 0)

/* Debug hooks - Access to Default FW parameters */
#define F_C28GP4X4R1P0_LOCAL_TX_PRESET  FIELD_DEFINE(0xE628, 27, 24)    /* LOCAL_TX_PRESET_INDEX */
#define F_C28GP4X4R1P0_TX_PRESET_INDEX  FIELD_DEFINE(0x67F4, 23, 16)
#define F_C28GP4X4R1P0_RXFFE_FORCE_EN   FIELD_DEFINE(0x60CC, 16, 16)    /* RXFFE_FORCE_EN_G0_LANE */
#define F_C28GP4X4R1P0_RXFFE_CAP1_SEL   FIELD_DEFINE(0x60B4, 11, 8)     /* RXFFE_CAP1_SEL_G0_LANE */
#define F_C28GP4X4R1P0_RXFFE_CAP2_SEL_O FIELD_DEFINE(0x60B4, 19, 16)    /* RXFFE_CAP2_SEL_O_G0_LANE */
#define F_C28GP4X4R1P0_RXFFE_CAP2_SEL_E FIELD_DEFINE(0x60B4, 27, 24)    /* RXFFE_CAP2_SEL_E_G0_LANE */
#define F_C28GP4X4R1P0_RXFFE_RES1_SEL   FIELD_DEFINE(0x60B4, 15, 12)    /* RXFFE_RES1_SEL_G0_LANE */
#define F_C28GP4X4R1P0_RXFFE_RES2_SEL_O FIELD_DEFINE(0x60B4, 23, 20)    /* RXFFE_RES2_SEL_O_G0_LANE */
#define F_C28GP4X4R1P0_RXFFE_RES2_SEL_E FIELD_DEFINE(0x60B4, 31, 28)    /* RXFFE_RES2_SEL_E_G0_LANE */
#define F_C28GP4X4R1P0_ALIGN90_REF_G0   FIELD_DEFINE(0x60E4, 7, 0)      /* ALIGN90_REF_G0_LANE */

/* Miscellaneous */
#define F_C28GP4X4R1P0_TX_MARGIN        FIELD_DEFINE(0x2010, 31, 29)

/* DRO Data */
#define F_C28GP4X4R1P0_PCM_EN           FIELD_DEFINE(0x8324, 5, 5)
#define F_C28GP4X4R1P0_DRO_EN           FIELD_DEFINE(0x8328, 7, 7)
#define F_C28GP4X4R1P0_FCLK_EN          FIELD_DEFINE(0xA330, 3, 3)      /* PROCESSMON_FCLK_EN */
#define F_C28GP4X4R1P0_DRO_SEL          FIELD_DEFINE(0x8328, 6, 3)
#define F_C28GP4X4R1P0_FBC_CNT_START    FIELD_DEFINE(0xA330, 0, 0)
#define F_C28GP4X4R1P0_FBC_PLLCAL_CNT_R FIELD_DEFINE(0xA330, 1, 1)      /* FBC_PLLCAL_CNT_READY */
#define F_C28GP4X4R1P0_FBC_PLLCAL_CNT   FIELD_DEFINE(0xA32C, 31, 16)
#define F_C28GP4X4R1P0_FBC_CNT_TIMER    FIELD_DEFINE(0xA32C, 15, 0)

#ifdef C28GP4X4_ISOLATION
/* RX Init */
#define F_C28GP4X4R1P0_RX_INIT          FIELD_DEFINE(0x210C, 7, 7)
#define F_C28GP4X4R1P0_RX_INIT_DONE     FIELD_DEFINE(0x2100, 19, 19)

/* DFE */
#define F_C28GP4X4R1P0_DFE_EN           FIELD_DEFINE(0x2410, 4, 4)

/* Power IV Ref */
#define F_C28GP4X4R1P0_PU_IVREF         FIELD_DEFINE(0xA31C, 1, 1)
#define F_C28GP4X4R1P0_PU_IVREF_FM_REG  FIELD_DEFINE(0xA31C, 0, 0)

/* Power Up Transmitter/Receiver/PLL */
#define F_C28GP4X4R1P0_PU_TX            FIELD_DEFINE(0x2008, 1, 1)
#define F_C28GP4X4R1P0_PU_RX            FIELD_DEFINE(0x2110, 8, 8)
#define F_C28GP4X4R1P0_PU_PLL           FIELD_DEFINE(0x2008, 3, 3)

/* TX Output */
#define F_C28GP4X4R1P0_TX_IDLE          FIELD_DEFINE(0x2014, 18, 18)

/* Phy Mode */
#define F_C28GP4X4R1P0_PHY_MODE         FIELD_DEFINE(0xA314, 26, 24)

/* Reference Frequency */
#define F_C28GP4X4R1P0_REFCLK_SEL       FIELD_DEFINE(0xA318, 13, 13)
#define F_C28GP4X4R1P0_REF_FREF_SEL     FIELD_DEFINE(0xA320, 4, 0)

/* TX/RX Bitrate */
#define F_C28GP4X4R1P0_PHY_GEN_TX       FIELD_DEFINE(0x2004, 4, 0)
#define F_C28GP4X4R1P0_PHY_GEN_RX       FIELD_DEFINE(0x2104, 4, 0)

/* MCU Frequency */
#define F_C28GP4X4R1P0_MCU_FREQ         FIELD_DEFINE(0xE65C, 15, 0)

/* TRX Training */
#define F_C28GP4X4R1P0_RX_TRAIN_ENABLE  FIELD_DEFINE(0x210C, 3, 3)
#define F_C28GP4X4R1P0_RX_TRAIN_COM     FIELD_DEFINE(0x2600, 24, 24)
#define F_C28GP4X4R1P0_RX_TRAIN_FAILED  FIELD_DEFINE(0x2600, 23, 23)
#define F_C28GP4X4R1P0_TX_TRAIN_ENABLE  FIELD_DEFINE(0x2010, 5, 5)
#define F_C28GP4X4R1P0_TX_TRAIN_COM     FIELD_DEFINE(0x2600, 28, 28)
#define F_C28GP4X4R1P0_TX_TRAIN_FAILED  FIELD_DEFINE(0x2600, 27, 27)

/* Squelch Detect */
#define F_C28GP4X4R1P0_DPHY_SQDETECTED  FIELD_DEFINE(0x4008, 1, 1)
#endif

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* C28GP4X4 */

#endif /* defined MCESD_C28GP4X4_DEFS_H */
