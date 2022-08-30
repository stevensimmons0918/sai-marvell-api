/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsPortMiscIf.h
*
* @brief
*
* @version   24
********************************************************************************
*/

#ifndef __mvHwsPortMiscIf_H
#define __mvHwsPortMiscIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* size of PAM4 eyes array*/
#define CPSS_PAM4_EYES_ARRAY_SIZE_CNS 6

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>

#define UNPERMITTED_SD_SPEED_INDEX 0xff

/**
* @enum MV_HWS_PORT_AUTO_TUNE_MODE
 *
 * @brief Defines different port auto tuning modes.
*/
typedef enum
{
    RxTrainingOnly = 1,
    TRxTuneCfg,
    TRxTuneStart,
    TRxTuneStatus,

    /* for AP */
    TRxTuneStatusNonBlocking,
    TRxTuneStop,

    /* for Avago */
    RxTrainingAdative,       /* run DFE_START_ADAPTIVE mode */
    RxTrainingStopAdative,   /* run DFE_STOP_ADAPTIVE  mode */
    RxTrainingWaitForFinish, /* run ICAL and wait for completeness */
    RxTrainingVsr,            /* run ICAL-VSR mode, Bypass DFE TAP tuning  */
    FixedCtleTraining,        /* run ICAL and bypass CTLE block */
    RxStatusNonBlocking

}MV_HWS_PORT_AUTO_TUNE_MODE;

/**
 * @enum MV_HWS_AUTO_TUNE_STATUS
 *
 * @brief This enum includes auto tune status
*/
typedef enum
{
  TUNE_PASS,
  TUNE_FAIL,
  TUNE_NOT_COMPLITED,
  TUNE_READY,
  TUNE_NOT_READY, /* relevant for Avago only. This state indicates that there is no Rx SerDEs Signal to start training */
  TUNE_RESET

}MV_HWS_AUTO_TUNE_STATUS;

/**
 * @enum MV_HWS_PPM_VALUE
 *
 * @brief This enum includes taps values
*/
typedef enum
{
  Neg_3_TAPS,
  Neg_2_TAPS,
  Neg_1_TAPS,
  ZERO_TAPS,
  POS_1_TAPS,
  POS_2_TAPS,
  POS_3_TAPS

}MV_HWS_PPM_VALUE;

/**
* @enum MV_HWS_PORT_MAN_TUNE_MODE
 *
 * @brief Defines different port manual tuning modes.
*/
typedef enum{

    StaticLongReach,

    StaticShortReach

} MV_HWS_PORT_MAN_TUNE_MODE;

/**
 * @struct MV_HWS_AUTO_TUNE_STATUS_RES
 *
 * @brief This struct include auto tune result
*/
typedef struct
{
    MV_HWS_AUTO_TUNE_STATUS rxTune;
    MV_HWS_AUTO_TUNE_STATUS txTune;

}MV_HWS_AUTO_TUNE_STATUS_RES;

/**
 * @struct MV_HWS_MAN_TUNE_COMPHY_H_TX_CONFIG_DATA
 *
 * @brief This struct include auto tune conphyH
*/
typedef struct
{
    /********************************/
    /* ComPhyH 40nm and 28nm params */
    /********************************/
    GT_U32                txAmp;
    GT_U32/*GT_BOOL*/     txAmpAdj;
    GT_32                 txEmph0;
    GT_32                 txEmph1;
    GT_U32/*GT_BOOL*/     txAmpShft;
    GT_U32/*GT_BOOL*/     txEmphEn;
    GT_U32                slewRate;
    GT_U32/*GT_BOOL*/     slewCtrlEn;
}MV_HWS_MAN_TUNE_COMPHY_H_TX_CONFIG_DATA;

/**
 * @struct MV_HWS_MAN_TUNE_COMPHY_TX_CONFIG_DATA
 *
 * @brief Comphy Tx tune parameters
*/
typedef struct _MV_HWS_MAN_TUNE_COMPHY_TX_CONFIG_DATA
{
    GT_U8 pre;
    GT_U8 peak;
    GT_U8 post;
}MV_HWS_MAN_TUNE_COMPHY_TX_CONFIG_DATA;

/**
 * @struct MV_HWS_MAN_TUNE_COMPHY_C12G_TX_CONFIG_DATA
 *
 * @brief Comphy Tx tune parameters
*/
typedef struct _MV_HWS_MAN_TUNE_COMPHY_C12G_TX_CONFIG_DATA
{
    GT_U8 pre;
    GT_U8 peak;
    GT_U8 post;
}MV_HWS_MAN_TUNE_COMPHY_C12G_TX_CONFIG_DATA;

/**
 * @struct MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA
 *
 * @brief Comphy Tx tune parameters
*/
typedef struct _MV_HWS_MAN_TUNE_COMPHY_C28G_TX_ONFIG_DATA
{
    GT_U8 pre;
    GT_U8 peak;
    GT_U8 post;
}MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA;


/**
 * @struct MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA
 *
 * @brief Comphy Tx tune parameters
*/
typedef struct
{
    GT_8 pre2;
    GT_8 pre;
    GT_8 main;
    GT_8 post;
}MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA;

/**
 * @struct MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA
 *
 * @brief Comphy Tx tune parameters
*/
typedef struct
{
    GT_8 pre2;
    GT_8 pre;
    GT_8 main;
    GT_8 post;
    GT_8 usr;
}MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA;

/**
 * @struct MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_TX_CONFIG_DATA
 *
 * @brief Comphy Tx tune parameters
*/
typedef struct _MV_HWS_MAN_TUNE_COMPHY_TX_CONFIG_DATA MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_TX_CONFIG_DATA;

/**
 * @struct MV_HWS_MAN_TUNE_COMPHY_C28GP4_TX_CONFIG_DATA
 *
 * @brief Comphy Tx tune parameters
*/
typedef struct _MV_HWS_MAN_TUNE_COMPHY_TX_CONFIG_DATA MV_HWS_MAN_TUNE_COMPHY_C28GP4_TX_CONFIG_DATA;

/**
 * @struct MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA
 *
 * @brief This struct include avago TX params
*/
typedef struct _MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA
{
    /****************************/
    /* AVAGO 28nm & 16nm params */
    /****************************/
    GT_U8   atten;  /* Serdes Attenuator [0...31] */

    GT_8    post;   /* Serdes Post-Cursor:
                       for Caelum; Aldrin; AC3X; Aldrin2(for Serdes 24-71); Pipe(for Serdes 0-11) devices: [-31...31]
                       for Bobcat3; Aldrin2(for Serdes 0-23); Pipe(for Serdes 12-15) device: [0...31]
                       for Raven device: [even values: -18...18] */

    GT_8    pre;    /* Serdes Pre-Cursor:
                       for Caelum; Aldrin; AC3X; Aldrin2(for Serdes 24-71); Pipe(for Serdes 0-11) devices: [-31...31]
                       for Bobcat3; Aldrin2(for Serdes 0-23); Pipe(for Serdes 12-15) device: [0...31]
                       for Raven device: [even values: -10...10] */
    /*********************/
    /* AVAGO 16nm params */
    /*********************/
    GT_8    pre2;   /* Serdes Pre2-Cursor: for Raven device: [-15...15] */
    GT_8    pre3;   /* Serdes Pre3-Cursor: for Raven device: [-1, 0, 1] */

}MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA;

/**
 * @struct MV_HWS_AVAGO_TX_OVERRIDE_CONFIG_DATA
 *
 * @brief This struct include avago TX params
*/
typedef struct _MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA MV_HWS_AVAGO_TX_OVERRIDE_CONFIG_DATA;

/**
 * @struct MV_HWS_MAN_TUNE_AVAGO_16NM_TX_CONFIG_DATA,
 *
 * @brief This struct include avago TX params
*/
typedef struct _MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA MV_HWS_MAN_TUNE_AVAGO_16NM_TX_CONFIG_DATA;

/**
 * @struct MV_HWS_COMPHY_H_AUTO_TUNE_RESULTS
 *
 * @brief This struct include auto tune result
*/
typedef struct
{
    GT_U32 ffeR;
    GT_U32 ffeC;
    GT_U32 sampler;
    GT_U32 sqleuch;
    GT_U32 txAmp;      /* In ComPhyH Serdes Amplitude */
    GT_32  txEmph0;    /* for ComPhyH Serdes [0...15] */
    GT_32  txEmph1;    /* for ComPhyH Serdes [0...15] */
    GT_U32 align90;
    GT_32  dfeVals[6];

    /* Lion2B additions */
    GT_U32  txAmpAdj;
    GT_U32  txAmpShft;
    GT_U32  txEmph0En;
    GT_U32  txEmph1En;
    GT_U32  slewRate;
    GT_BOOL slewCtrlEn;
}MV_HWS_COMPHY_H_AUTO_TUNE_RESULTS;

/**
 * @struct MV_HWS_AVAGO_AUTO_TUNE_RESULTS
 *
 * @brief This struct include auto tune result
*/
typedef struct
{
    GT_U32 sqleuch;

    /* Avago Rx params: 28nm and 16nm */
    GT_U32  DC;
    GT_U32  LF;
    GT_U32  HF;
    GT_U32  BW;
    GT_U32  EO;
    GT_32   DFE[13];

    /* Avago Rx params: 16nm */
    GT_U8   gainshape1;     /* CTLE gainshape1 [0-3] */
    GT_U8   gainshape2;     /* CTLE gainshape2 [0-3] */
    GT_BOOL shortChannelEn; /* Enable/Disable Short channel  */
    GT_U8   dfeGAIN;        /* DFE Gain Tap strength  [0-255] */
    GT_U8   dfeGAIN2;       /* DFE Gain Tap2 strength [0-255] */

    /* AVAGO Tx params: 28nm and 16nm */
    MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA    avagoStc;

    /* AVAGO Rx-FFE params: 16nm */
    GT_32     rxffe_pre2;
    GT_32     rxffe_pre1;
    GT_32     rxffe_post1;
    GT_32     rxffe_bflf;
    GT_32     rxffe_bfhf;
    GT_32     rxffe_datarate;
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT) || defined (FALCON_DEV_SUPPORT)
    GT_32     rxffe_minPre1;
    GT_32     rxffe_maxPre1;
    GT_32     rxffe_minPre2;
    GT_32     rxffe_maxPre2;
    GT_U32    minLf;
    GT_U32    maxLf;
    GT_U32    minHf;
    GT_U32    maxHf;
    /* AVAGO Rx-Vernier params: 16nm */
    GT_U32    vernier_upper_odd_dly;
    GT_U32    vernier_upper_even_dly;
    GT_U32    vernier_middle_odd_dly;
    GT_U32    vernier_middle_even_dly;
    GT_U32    vernier_lower_odd_dly;
    GT_U32    vernier_lower_even_dly;
    GT_U32    vernier_test_odd_dly;
    GT_U32    vernier_test_even_dly;
    GT_U32    vernier_edge_odd_dly;
    GT_U32    vernier_edge_even_dly;
    GT_U32    vernier_tap_dly;

    GT_U32    pam4EyesArr[CPSS_PAM4_EYES_ARRAY_SIZE_CNS];
    GT_U8     termination;
    GT_32     rxffe_minPost;
    GT_32     rxffe_maxPost;
    GT_U32    coldEnvelope;
    GT_U32    hotEnvelope;

#endif /* #ifndef MV_HWS_REDUCED_BUILD_EXT_CM3 */

}MV_HWS_AVAGO_AUTO_TUNE_RESULTS;

/**
 * @struct MV_HWS_MAN_TUNE_COMPHY_H_RX_CONFIG_DATA
 *
 * @brief This struct include auto tune result
*/
typedef struct
{
    GT_U32                    sqlch;
    GT_U32                    ffeRes;
    GT_U32                    ffeCap;
    GT_BOOL                   dfeEn;
    GT_U32                    alig;
    MV_HWS_PORT_MAN_TUNE_MODE portTuningMode;

}MV_HWS_MAN_TUNE_COMPHY_H_RX_CONFIG_DATA;

/**
 * @struct MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA
 *
 * @brief This struct include CTLE data
*/
typedef struct _MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA
{
    /****************************/
    /* AVAGO 28nm & 16nm params */
    /****************************/
    GT_U16      dcGain;         /* rang: 0-255 */
    GT_U16      lowFrequency;   /* rang: 0-15  */
    GT_U16      highFrequency;  /* rang: 0-15  */
    GT_U16      bandWidth;      /* rang: 0-15  */
    GT_U16      squelch;        /* rang: 0-308 */

    /*********************/
    /* AVAGO 16nm params */
    /*********************/
    GT_U8      gainshape1;     /* CTLE gainshape1 [0-3] */
    GT_U8      gainshape2;     /* CTLE gainshape2 [0-3] */
    GT_BOOL    shortChannelEn; /* Enable/Disable Short channel  */
    GT_U8      dfeGAIN;        /* DFE Gain Tap strength  [0-255] */
    GT_U8      dfeGAIN2;       /* DFE Gain Tap2 strength [0-255] */

}MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA;

/**
 * @struct MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA
 *
 * @brief This struct include CTLE data
*/
typedef struct _MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA;

/**
 * @struct MV_HWS_MAN_TUNE_AVAGO_16NM_RX_CONFIG_DATA
 *
 * @brief This struct include RX data
*/
typedef struct
{
    /* basic */
    GT_U8   lowFrequency;
    GT_U8   highFrequency;
    GT_U8   bandWidth;
    GT_U8   dcGain;
    GT_U8   gainShape1;
    GT_U8   gainShape2;
    GT_U8   ffeFix;
    GT_U8   shortChannelEn;
    GT_8    bfLf;
    GT_8    bfHf;

    /* advanced */
    GT_U8   minLf;
    GT_U8   maxLf;
    GT_U8   minHf;
    GT_U8   maxHf;
    GT_8    minPre1;
    GT_8    maxPre1;
    GT_8    minPre2;
    GT_8    maxPre2;
    GT_8    minPost;
    GT_8    maxPost;
    GT_U16  squelch;

    /* main */
    GT_U8   iCalEffort;
    GT_U8   pCalEffort;
    GT_U8   dfeDataRate;
    GT_U8   dfeCommon;
    GT_U16  int11d;
    GT_U16  int17d;

    /* misc */
    GT_U8   termination;
    GT_16   pre1PosGradient;
    GT_16   pre1NegGradient;
    GT_16   pre2PosGradient;
    GT_16   pre2NegGradient;
    GT_16   hfPosGradient;
    GT_16   hfNegGradient;
    GT_U8   agcTargetLow;
    GT_U8   agcTargetHigh;
    GT_U8   coldEnvelope;
    GT_U8   hotEnvelope;
} MV_HWS_MAN_TUNE_AVAGO_16NM_RX_CONFIG_DATA;

/**
 * @struct MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA
 *
 * @brief This struct include RX data
*/
typedef struct
{
    /* Basic (CTLE) */
    GT_U8 resSel;
    GT_U8 resShift;
    GT_U8 capSel;

    /* Advanced (CDR) */
    GT_U8 selmufi;
    GT_U8 selmuff;
    GT_U8 selmupi;
    GT_U8 selmupf;

    GT_16 squelch;

} MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA;

/**
 * @struct MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA
 *
 * @brief This struct include RX data
*/
typedef struct
{
    /* Basic (CTLE) */
    GT_U32 dataRate;
    GT_U32 res1Sel;
    GT_U32 res2Sel;
    GT_U32 cap1Sel;
    GT_U32 cap2Sel;

    /* Advanced (CDR) */
    GT_U32 selmufi;
    GT_U32 selmuff;
    GT_U32 selmupi;
    GT_U32 selmupf;

    /* Advanced (Thresholds) */
    GT_U32 midpointLargeThresKLane;
    GT_U32 midpointSmallThresKLane;
    GT_U32 midpointLargeThresCLane;
    GT_U32 midpointSmallThresCLane;

    /* Advanced (DFE) */
    GT_U32 dfeResF0aHighThresInitLane;
    GT_U32 dfeResF0aHighThresEndLane;

    GT_16  squelch;

} MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA;

/**
 * @struct MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA
 *
 * @brief This struct include RX data
*/
typedef struct
{
    /* Basic (CTLE) */
    /* 1st stage GM Main */
    GT_U32 current1Sel;
    GT_U32 rl1Sel;
    GT_U32 rl1Extra;
    GT_U32 res1Sel;
    GT_U32 cap1Sel;
    GT_U32 cl1Ctrl;
    GT_U32 enMidFreq;
    GT_U32 cs1Mid;
    GT_U32 rs1Mid;
    /* 1st stage TIA */
    GT_U32 rfCtrl;
    GT_U32 rl1TiaSel;
    GT_U32 rl1TiaExtra;
    GT_U32 hpfRSel1st;
    GT_U32 current1TiaSel;
    /* 2nd Stage */
    GT_U32 rl2Tune;
    GT_U32 rl2Sel;
    GT_U32 rs2Sel;
    GT_U32 current2Sel;
    GT_U32 cap2Sel;
    GT_U32 hpfRsel2nd;

    /* Advanced (CDR) */
    GT_U32 selmufi;
    GT_U32 selmuff;
    GT_U32 selmupi;
    GT_U32 selmupf;

    GT_U8  squelch;

} MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA;

/**
 * @struct MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA
 *
 * @brief This struct include RX data
*/
typedef struct
{
    /* Basic (CTLE) */
    GT_U32 cur1Sel;
    GT_U32 rl1Sel;
    GT_U32 rl1Extra;
    GT_U32 res1Sel;
    GT_U32 cap1Sel;
    GT_U32 enMidfreq;
    GT_U32 cs1Mid;
    GT_U32 rs1Mid;
    GT_U32 cur2Sel;
    GT_U32 rl2Sel;
    GT_U32 rl2TuneG;
    GT_U32 res2Sel;
    GT_U32 cap2Sel;

    /* Advanced (CDR) */
    GT_U32 selmufi;
    GT_U32 selmuff;
    GT_U32 selmupi;
    GT_U32 selmupf;

    GT_16  squelch;

} MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA;

/**
 * @struct MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA
 *
 * @brief This struct includes Rx FFE, Rx CTLE parameters
*/
typedef struct
{
    GT_U8   dcGain;
    GT_U8   lowFrequency;
    GT_U8   highFrequency;
    GT_U8   bandWidth;
    GT_U8   gainShape1;
    GT_U8   gainShape2;
    GT_U8   minLf;
    GT_U8   maxLf;
    GT_U8   minHf;
    GT_U8   maxHf;
    GT_8    bfLf;
    GT_8    bfHf;
    GT_8    minPre1;
    GT_8    maxPre1;
    GT_8    minPre2;
    GT_8    maxPre2;
    GT_8    minPost;
    GT_8    maxPost;
    GT_U16  squelch;
    GT_U8   shortChannelEn;
    GT_U8   termination;
    GT_U8   coldEnvelope;
    GT_U8   hotEnvelope;
} MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA;

/**
 * @struct MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS
 *
 * @brief This struct represent the tx and rx tune parameters
 *        for avago 16nm device
*/
typedef struct
{
    MV_HWS_SERDES_SPEED                         serdesSpeed;
    MV_HWS_MAN_TUNE_AVAGO_16NM_TX_CONFIG_DATA   txTuneData;
    MV_HWS_MAN_TUNE_AVAGO_16NM_RX_CONFIG_DATA   rxTuneData;
} MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS;

/**
 * @struct MV_HWS_AVAGO_TXRX_TUNE_PARAMS
 *
 * @brief This struct include CTLE and TX data
*/
typedef struct
{
    MV_HWS_SERDES_SPEED                     serdesSpeed;
    MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA    txParams;
    MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA    rxParams;

}MV_HWS_AVAGO_TXRX_TUNE_PARAMS;

#define MV_HWS_SERDES_TXRX_TUNE_PARAMS_DB_TX_VALID          0x1
#define MV_HWS_SERDES_TXRX_TUNE_PARAMS_DB_RX_VALID          0x2

/**
 * @struct MV_HWS_AVAGO_TXRX_OVERRIDE_CONFIG_DATA
 *
 * @brief This struct include Tx/Rx override data
*/
typedef struct
{
    GT_U8                                valid;
    GT_U8                                interconnectProfile;
    MV_HWS_AVAGO_TX_OVERRIDE_CONFIG_DATA txData;
    MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA rxData;
} MV_HWS_AVAGO_TXRX_OVERRIDE_CONFIG_DATA;

/**
 * @struct MV_HWS_ETL_CONFIG_DATA
 *
 * @brief This struct include ETL data
*/
typedef struct
{
    GT_U8     etlMinDelay;
    GT_U8     etlMaxDelay;
    GT_U8     etlEnableOverride;
}MV_HWS_ETL_CONFIG_DATA;

#define MV_HWS_MAN_TUNE_CTLE_CONFIG_SQLCH_CNS           0x1
#define MV_HWS_MAN_TUNE_CTLE_CONFIG_LF_CNS              0x2
#define MV_HWS_MAN_TUNE_CTLE_CONFIG_HF_CNS              0x4
#define MV_HWS_MAN_TUNE_CTLE_CONFIG_DCGAIN_CNS          0x8
#define MV_HWS_MAN_TUNE_CTLE_CONFIG_BANDWIDTH_CNS       0x10
#define MV_HWS_MAN_TUNE_ETL_MIN_DELAY_CNS               0x40
#define MV_HWS_MAN_TUNE_ETL_MAX_DELAY_CNS               0x80
#define MV_HWS_MAN_TUNE_ETL_ENABLE_CNS                  0x100

#define MV_HWS_MAN_TUNE_CTLE_CONFIG_10G_INDEX_CNS       0
#define MV_HWS_MAN_TUNE_CTLE_CONFIG_25G_INDEX_CNS       1
/**
 * @struct MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE
 *
 * @brief This struct include CTLE override
*/
typedef struct
{
    GT_U32                              serdesSpeed;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    ctleParams;
    MV_HWS_ETL_CONFIG_DATA              etlParams;
    GT_U16                              fieldOverrideBmp;
}MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE;

/**
 * @struct MV_HWS_TX_TUNE_PARAMS
 *
 * @brief This struct include TX tune params
*/
typedef struct
{
    GT_U32    txAmp;
    GT_U32    txEmph0;
    GT_U32    txEmph1;

}MV_HWS_TX_TUNE_PARAMS;

/**
 * @struct MV_HWS_CALIBRATION_RESULTS
 *
 * @brief This struct include calibration results
*/
typedef struct
{
    GT_BOOL calDone;
    GT_U32  txImpCal;
    GT_U32  rxImpCal;
    GT_U32  ProcessCal;
    GT_U32  speedPll;
    GT_U32  sellvTxClk;
    GT_U32  sellvTxData;
    GT_U32  sellvTxIntp;
    GT_U32  sellvTxDrv;
    GT_U32  sellvTxDig;
    GT_U32  sellvRxSample;
    GT_BOOL ffeDone;
    GT_32   ffeCal[8];

}MV_HWS_CALIBRATION_RESULTS;

/**
* @struct MV_HWS_PORT_STATUS_STC
 *
 * @brief Port status pcs and mac status parameters.
*/
typedef struct{
    /** @brief RS-FEC alignment status for every channel with 4 bit
     *         per Quad Block
     */
    GT_U32 rsfecAligned;

    /** @brief Indicates high bit error rate
     */
    GT_U32 hiBer;

    /** @brief 100G Eth Alignment Marker Lock indication.
        Relevant only for even ports (0,2,4,6) and only when set to 100G
     */
    GT_U32 alignDone;

    /** @brief Asserts (1) when the internal (Clause 37) autonegotiation function completed
     */
    GT_U32 lpcsAnDone;

    /** @brief Asserts (1) when the receiver detected comma
     *         characters and 10B alignment has been achieved.
     */
    GT_U32 lpcsRxSync;

    /** @brief low rates PCS link.
     */
    GT_U32 lpcsLinkStatus;

    /** @brief High symbol error rate detected.
     */
    GT_U32 hiSer;

    /** @brief Symbol error rate passed the configurable threshold
     */
    GT_U32 degradeSer;

    /** @brief PCS locked indication.
     */
    GT_U32 alignLock;

    /** @brief PCS link = PCS is locked and aligned.
     */
    GT_U32 linkStatus;

    /** @brief Link OK = link_status AND no faults are present.
     */
    GT_U32 linkOk;

    /** @brief PHY indicates loss-of-signal. Represents value of pin
     *         "phy_los"
     */
    GT_U32 phyLos;

    /** @brief Remote Fault Status.
     */
    GT_U32 rxRemFault;

    /** @brief Local Fault Status.
     */
    GT_U32 rxLocFault;

    /** @brief indicates a fault condition idetected.
     */
    GT_U32 fault;

    /** @brief     indicates PCS receive link up.
     */
    GT_U32 pcsReceiveLink;

    /** @brief     Device present. When bits are 10 = device
     *             responding at this address..
     */
    GT_U32 devicePresent;

    /** @brief     Transmit fault. 1=Fault condition on transmit
     *             path. Latched high.
     */
    GT_U32 transmitFault;

     /** @brief     Receive fault. 1=Fault condition on receive path
     *             Latched high.
     */
    GT_U32 receiveFault;
    /** @brief     BER counter.
     */
    GT_U32 berCounter;
    /** @brief     Errored blocks counter.
     */
    GT_U32 errorBlockCounter;
} MV_HWS_PORT_STATUS_STC;

/**
* @enum MV_HWS_PORT_TEST_GEN_PATTERN
 *
 * @brief Defines different port (PCS) test generator patterns.
*/
typedef enum
{
  TEST_GEN_PRBS7,
  TEST_GEN_PRBS23,
  TEST_GEN_CJPAT,
  TEST_GEN_CRPAT,
  TEST_GEN_KRPAT,
  TEST_GEN_Normal,
  TEST_GEN_PRBS31

}MV_HWS_PORT_TEST_GEN_PATTERN;

/**
* @enum MV_HWS_PORT_TEST_GEN_ACTION
 *
 * @brief Defines different port test generator actions.
*/
typedef enum{

    NORMAL_MODE,

    TEST_MODE

} MV_HWS_PORT_TEST_GEN_ACTION;


/**
* @enum MV_HWS_PORT_LB_TYPE
 *
 * @brief Defines different port loop back types.
 * DISABLE_LB - disable loop back
 * RX_2_TX_LB - configure port to send back all received packets
 * TX_2_RX_LB - configure port to receive back all sent packets
*/
typedef enum{

    DISABLE_LB,

    RX_2_TX_LB,

    TX_2_RX_LB,

    TX_2_RX_DIGITAL_LB

} MV_HWS_PORT_LB_TYPE;

/**
* @enum MV_HWS_UNIT
 *
 * @brief Defines different port loop back levels.
*/
typedef enum{

    HWS_MAC,

    HWS_PCS,

    HWS_PMA,

    HWS_MPF

} MV_HWS_UNIT;

/**
 * @struct MV_HWS_TEST_GEN_STATUS
 *
 * @brief This struct include test gen status
*/
typedef struct
{
  GT_U32  totalErrors;
  GT_U32  goodFrames;
  GT_U32  checkerLock;

}MV_HWS_TEST_GEN_STATUS;

/**
* @enum MV_HWS_PORT_PRESET_CMD
 *
 * @brief Defines PRESET commands.
*/
typedef enum{

    INIT = 1,

    PRESET = 2,

    PRESET_NA = 0xFFFF

} MV_HWS_PORT_PRESET_CMD;

/**
 * @struct MV_HWS_SERDES_TUNING_PARAMS
 *
 * @brief This struct include tuning params
*/
typedef struct
{
    MV_HWS_PORT_PRESET_CMD  presetCmdVal;
    GT_BOOL                 eyeCheckEnVal;
    GT_U32                  gen1TrainAmp;
    GT_U32                  gen1TrainEmph0;
    GT_U32                  gen1TrainEmph1;
    GT_BOOL                 gen1AmpAdj;
    GT_U32                  initialAmp;
    GT_U32                  initialEmph0;
    GT_U32                  initialEmph1;
    GT_U32                  presetAmp;
    GT_U32                  presetEmph0;
    GT_U32                  presetEmph1;

}MV_HWS_SERDES_TUNING_PARAMS;

/**
* @struct CPSS_DXCH_RSFEC_UNITS_STC
 *
 * @brief RSFEC unit counters.
*/
typedef struct {
    /** @brief Indicates, when 1 that the RS-FEC receiver has
     *         locked on incoming data and deskew completed. */
    GT_U32   fecAlignStatus;
    /** @brief RS-FEC receive all lanes aligned status. */
    GT_U32   ampsLock;
    /** @brief     error indication bypass is enabled and high
     *             symbol error rate is found */
    GT_U32   highSer;
    /** @brief Symbol errors counter */
    GT_U32   symbolError[16];
} MV_HWS_RSFEC_STATUS_STC;

/**
* @struct MV_HWS_RSFEC_COUNTERS_STC
 *
 * @brief RSFEC counters .
*/
typedef struct {
    /** @brief correctedFecCodeword  */
    GT_U64   correctedFecCodeword;
    /** @brief uncorrectedFecCodeword. */
    GT_U64   uncorrectedFecCodeword;
    /** @brief correctFecCodeword. */
    GT_U64   correctFecCodeword;
    /** @brief Symbol errors counter */
    GT_U32   symbolError[4];
    /** @brief total_cw_received */
    GT_U64   total_cw_received;

} MV_HWS_RSFEC_COUNTERS_STC;

/**
* @struct MV_HWS_FCFEC_COUNTERS_STC
 *
 * @brief FCFEC counters .
*/
typedef struct {
    /** @brief numReceivedBlocks  */
    GT_U32   numReceivedBlocks;
    /** @brief numReceivedBlocksNoError. */
    GT_U32   numReceivedBlocksNoError;
    /** @brief numReceivedBlocksCorrectedError */
    GT_U32   numReceivedBlocksCorrectedError;
    /** @brief numReceivedBlocksUncorrectedError  */
    GT_U32   numReceivedBlocksUncorrectedError;
    /** @brief numReceivedCorrectedErrorBits. */
    GT_U32   numReceivedCorrectedErrorBits;
    /** @brief numReceivedUncorrectedErrorBits */
    GT_U32   numReceivedUncorrectedErrorBits;
    /** @brief  BlocksCorrectedError for every lane array   */
    GT_U32   blocksCorrectedError[4];
    /** @brief  BlocksUncorrectedError for every lane array   */
    GT_U32   blocksUncorrectedError[4];
} MV_HWS_FCFEC_COUNTERS_STC;

/**
* @enum MV_HWS_PORT_FLOW_CONTROL_ENT
 *
 * @brief Enumeration of Port FC Direction enablers.
*/
typedef enum{

    /** Both disabled */
    MV_HWS_PORT_FLOW_CONTROL_DISABLE_E = GT_FALSE,

    /** Both enabled */
    MV_HWS_PORT_FLOW_CONTROL_RX_TX_E = GT_TRUE,

    /** Only Rx FC enabled */
    MV_HWS_PORT_FLOW_CONTROL_RX_ONLY_E,

    /** Only Tx FC enabled */
    MV_HWS_PORT_FLOW_CONTROL_TX_ONLY_E

} MV_HWS_PORT_FLOW_CONTROL_ENT;

/**
* @enum MV_HWS_PORT_SERDES_OPERATION_MODE_ENT
 *
 * @brief Defines operations to do on port.
*/
typedef enum{

    MV_HWS_PORT_SERDES_OPERATION_PRECODING_E = 0x1,

    MV_HWS_PORT_SERDES_OPERATION_DFE_COMMON_E = 0x2,

    MV_HWS_PORT_SERDES_OPERATION_CALC_LEVEL3_E = 0x4,

    MV_HWS_PORT_SERDES_OPERATION_RESET_RXTX_E = 0x8,

    MV_HWS_PORT_SERDES_OPERATION_RES_CAP_SEL_TUNING_E = 0x10,

    MV_HWS_PORT_SERDES_OPERATION_RX_TRAIN_E = 0x20,

    MV_HWS_PORT_SERDES_OPERATION_IS_FIBER_E = 0x40,

} MV_HWS_PORT_SERDES_OPERATION_MODE_ENT;
/* =============================================================== */
/**
 * @union MV_HWS_SERDES_RX_OVERRIDE_CONFIG_DATA_UNT
 *
 * @brief Defines serdes rx configuration.
*/
typedef union {
    MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA     rxAvago;
#if ((!defined MV_HWS_REDUCED_BUILD)) || defined(AC5_DEV_SUPPORT)
    MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA    rxComphyC12GP41P2V;
#endif

}MV_HWS_SERDES_RX_OVERRIDE_CONFIG_DATA_UNT;

/**
* @union MV_HWS_SERDES_TX_OVERRIDE_CONFIG_DATA_UNT
 *
 * @brief Defines serdes tx configuration.
*/
typedef union {
    MV_HWS_AVAGO_TX_OVERRIDE_CONFIG_DATA     txAvago;
#if ((!defined MV_HWS_REDUCED_BUILD)) || defined(AC5_DEV_SUPPORT)
    MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_TX_CONFIG_DATA   txComphyC12GP41P2V;
#endif

}MV_HWS_SERDES_TX_OVERRIDE_CONFIG_DATA_UNT;
/* =============================================================== */
/**
* @union MV_HWS_SERDES_RX_CONFIG_DATA_UNT
 *
 * @brief Defines serdes rx configuration.
*/
typedef union {
#ifndef ALDRIN_DEV_SUPPORT
    MV_HWS_MAN_TUNE_COMPHY_H_RX_CONFIG_DATA             rxComphyH;
#endif
    MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA                rxAvago;
    MV_HWS_MAN_TUNE_AVAGO_16NM_RX_CONFIG_DATA           rxAvago16nm;
    MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA    rxComphyC12GP41P2V;
    MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA        rxComphyC28GP4;
    MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA         rxComphyC112G;
    MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA          rxComphyC56G;

}MV_HWS_SERDES_RX_CONFIG_DATA_UNT;

/**
 * @union MV_HWS_SERDES_TX_CONFIG_DATA_UNT
 *
 * @brief Defines serdes tx configuration.
*/
typedef union {
#ifndef ALDRIN_DEV_SUPPORT
    MV_HWS_MAN_TUNE_COMPHY_H_TX_CONFIG_DATA             txComphyH;
#endif
    MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA                txAvago;
    MV_HWS_MAN_TUNE_COMPHY_C12G_TX_CONFIG_DATA          txComphyC12GP41P2V;
    MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA          txComphyC28GP4;
    MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA         txComphyC112G;
    MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA          txComphyC56G;

}MV_HWS_SERDES_TX_CONFIG_DATA_UNT;

/**
 * @struct MV_HWS_SERDES_TXRX_TUNE_PARAMS
 *
 * @brief This struct include Tx and RX data
*/
typedef struct
{
    MV_HWS_SERDES_SPEED                 serdesSpeed;
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT    txParams;
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT    rxParams;

}MV_HWS_SERDES_TXRX_TUNE_PARAMS;

/* =============================================================== */
/**
 * @struct MV_HWS_COMPHY_C12GP41P2V_AUTO_TUNE_RESULTS
 *
 * @brief This struct include auto tune result
*/
typedef struct
{
    MV_HWS_MAN_TUNE_COMPHY_C12G_TX_CONFIG_DATA          txComphyC12GP41P2V;
    MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA    rxComphyC12GP41P2V;
    GT_U8                                               eo;
    GT_U16                                              align90;
    GT_U16                                              sampler;
    GT_U32                                              slewRateCtrl0;
    GT_U32                                              slewRateCtrl1;
    GT_32                                               dfe[12];
    GT_U8                                               ffeSettingForce;
    GT_U8                                               adaptedResSel;
    GT_U8                                               adaptedCapSel;
    GT_U8                                               analogPre;
    GT_U8                                               analogPeak;
    GT_U8                                               analogPost;
}MV_HWS_COMPHY_C12GP41P2V_AUTO_TUNE_RESULTS;

/**
 * @struct MV_HWS_COMPHY_C28GP4_AUTO_TUNE_RESULTS
 *
 * @brief This struct include auto tune result
*/
typedef struct
{
    MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA      txComphyC28GP4;
    MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA    rxComphyC28GP4;
    GT_U8                                           eo;
    GT_U16                                          align90;
    GT_U16                                          sampler;
    GT_U32                                          slewRateCtrl0;
    GT_U32                                          slewRateCtrl1;
    GT_32                                           dfe[25];
}MV_HWS_COMPHY_C28GP4_AUTO_TUNE_RESULTS;

/**
 * @struct MV_HWS_COMPHY_C112G_AUTO_TUNE_RESULTS
 *
 * @brief This struct include auto tune result
*/
typedef struct
{
    MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA     txComphyC112G;
    MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA     rxComphyC112G;
    GT_U8                                           eo;
    GT_U32                                          align90AnaReg;
    GT_16                                           align90;
    GT_U16                                          sampler;
    GT_U32                                          slewRateCtrl0;
    GT_U32                                          slewRateCtrl1;
    GT_32                                           dfe[40];
}MV_HWS_COMPHY_C112G_AUTO_TUNE_RESULTS;

/**
 * @struct MV_HWS_COMPHY_C56G_AUTO_TUNE_RESULTS
 *
 * @brief This struct include auto tune result
*/
typedef struct
{
    MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA     txComphyC56G;
    MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA     rxComphyC56G;
    GT_U8                                           eo;
    GT_U16                                          align90;
    GT_U16                                          sampler;
    GT_U32                                          slewRateCtrl0;
    GT_U32                                          slewRateCtrl1;
    GT_32                                           dfe[26];
}MV_HWS_COMPHY_C56G_AUTO_TUNE_RESULTS;

/**
 * @union MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT
 *
 * @brief Auto tune results struct per SerDes type
*/
typedef union {
    MV_HWS_COMPHY_H_AUTO_TUNE_RESULTS             comphyHResults;
    MV_HWS_AVAGO_AUTO_TUNE_RESULTS                avagoResults;
    MV_HWS_COMPHY_C12GP41P2V_AUTO_TUNE_RESULTS    comphyC12GP41P2VResults;
    MV_HWS_COMPHY_C28GP4_AUTO_TUNE_RESULTS        comphyC28GP4Results;
    MV_HWS_COMPHY_C112G_AUTO_TUNE_RESULTS         comphyC112GResults;
    MV_HWS_COMPHY_C56G_AUTO_TUNE_RESULTS          comphyC56GResults;

}MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT;

/**
 * @struct MV_HWS_COMPHY_C28GP4_PRESET_OVERRIDE
 *
 * @brief serdes preset default parameters to override for ComPhy 28G
*/
typedef struct
{
    GT_U32  localCtlePresetDefault;
    GT_U32  localCtlePresetRes1;
    GT_U32  localCtlePresetCap1;
    GT_U32  localCtlePresetRes2_O;
    GT_U32  localCtlePresetRes2_E;
    GT_U32  localCtlePresetCap2_O;
    GT_U32  localCtlePresetCap2_E;
    GT_U32  localCtlePresetPhase;
    GT_U32  txLocalPreset;
    GT_U32  remotePresetRequestType;
}MV_HWS_COMPHY_C28GP4_PRESET_OVERRIDE;

/**
 * @union MV_HWS_SERDES_PRESET_OVERRIDE_UNT
 *
 * @brief serdes preset default parameters to override
*/
typedef union {
    MV_HWS_COMPHY_C28GP4_PRESET_OVERRIDE        comphyC28GP4PresetOverride;
}MV_HWS_SERDES_PRESET_OVERRIDE_UNT;

/* =============================================================== */
/**
* @internal mvHwsPortAutoTuneSet function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    void *                      results
);

/**
* @internal mvHwsPortEnhanceTuneLite function
* @endinternal
*
* @brief   Set the PCAL with shifted sampling point to find best sampling point
*         This API runs only for AP port after linkUp indication and before running
*         the Rx-Training Adative pCal
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
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
GT_STATUS mvHwsPortEnhanceTuneLite
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8       min_dly,
    GT_U8       max_dly
);

/**
* @internal mvHwsPortEnhanceTuneLiteSetByPhase function
* @endinternal
*
* @brief   This function calls Enhance-Tune Lite algorithm phases.
*         The operation is relevant only for AP port with Avago Serdes
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
* @param[in] phase                    -  to perform
* @param[in] min_dly                  - Minimum delay_cal value: (rang: 0-31)
* @param[in] max_dly                  - Maximum delay_cal value: (rang: 0-31)
*
* @param[out] phaseFinishedPtr         - whether or not phase is finished
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortEnhanceTuneLiteSetByPhase
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8       phase,
    GT_U8       *phaseFinishedPtr,
    GT_U8       min_dly,
    GT_U8       max_dly
);

/**
* @internal mvHwsAvagoSerdesDefaultMinMaxDlyGet function
* @endinternal
*
* @brief   This function returns the default minimum and maximum delay
*         values according to the given port mode
* @param[in] portMode                 - mode type of port
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesDefaultMinMaxDlyGet
(
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8                   *min_dly_ptr,
    GT_U8                   *max_dly_ptr
);

/**
* @internal mvHwsPortEnhanceTuneLitePhase1 function
* @endinternal
*
* @brief   Set the PCAL with shifted sampling point to find best sampling point
*         This API runs only for AP port after linkUp indication and before running
*         the Rx-Training Adative pCal
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] min_dly                  - Minimum delay_cal value:
*                                      - for Serdes speed 10G (25...28)
*                                      - for Serdes speed 25G (7...21)
* @param[in] max_dly                  - Maximum delay_cal value:
*                                      - for Serdes speed 10G (30)
*                                      - for Serdes speed 25G (23)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortEnhanceTuneLitePhase1
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8       min_dly,
    GT_U8       max_dly
);

/**
* @internal mvHwsPortEnhanceTuneLiteByPhaseInitDb function
* @endinternal
*
* @brief   Init HWS DB of EnhanceTuneLite by phase algorithm used for AP port
*
* @param[in] devNum                   - system device number
* @param[in] phyPortIndex             - ap port number index
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortEnhanceTuneLiteByPhaseInitDb
(
    GT_U8   devNum,
    GT_U32  phyPortIndex
);

/**
* @internal mvHwsPortAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] optAlgoMask              - bit mask for optimization algorithms
* @param[in] portTuningMode           - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneSetExt
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    GT_U32                  optAlgoMask,
    void *                  results
);

/**
* @internal mvHwsPortAvagoAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] optAlgoMask              - bit mask for optimization algorithms
* @param[in] portTuningMode           - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoAutoTuneSetExt
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    GT_U32                      optAlgoMask,
    void                        *results
);

/**
* @internal mvHwsPortGeneralAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum             - system device number
* @param[in] portGroup          - port group (core) number
* @param[in] phyPortNum         - physical port number
* @param[in] portMode           - port standard metric
* @param[in] optAlgoMask        - bit mask for optimization algorithms
* @param[in] portTuningMode     - port tuning mode
* @param[out] results           - port tuning results (optional)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortGeneralAutoTuneSetExt
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    GT_U32                      optAlgoMask,
    void                        *results
);

/**
* @internal mvHwsPortOperation function
* @endinternal
*
* @brief   Sets operation on serdes that belongs to the port
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] operation                - port operation mode
* @param[in] dataPtr                     - data
*
* @param[out] resultPtr                 - results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortOperation
(
   IN GT_U8                       devNum,
   IN GT_U32                      portGroup,
   IN GT_U32                      phyPortNum,
   IN MV_HWS_PORT_STANDARD        portMode,
   IN MV_HWS_PORT_SERDES_OPERATION_MODE_ENT  operation,
   IN GT_U32                     *dataPtr,
   OUT GT_U32                    *resultPtr
);

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPortAutoTuneOptimization function
* @endinternal
*
* @brief   Run optimization for the training results.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] optAlgoMask              bit mask of optimization algorithms
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneOptimization
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  optAlgoMask
);
#endif

/**
* @internal mvHwsPortAutoTuneDelayInit function
* @endinternal
*
* @brief   Sets the delay values which are used in Serdes training optimization
*         algorithm
* @param[in] dynamicDelayInterval     - determines the number of training iteration in
*                                      which the delay will be executed (DFE algorithm)
* @param[in] dynamicDelayDuration     - delay duration in mSec (DFE algorithm)
* @param[in] staticDelayDuration      - delay duration in mSec (Align90 algorithm)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneDelayInit
(
    GT_U32     dynamicDelayInterval, /* DFE */
    GT_U32     dynamicDelayDuration, /* DFE */
    GT_U32     staticDelayDuration   /* Align90 */
);

/**
* @internal mvHwsPortAdaptiveCtleBasedTemperature function
* @endinternal
*
* @brief  Run Steady State apdative ctle algorithm. this
*          feature do delay and LF Calibration based Temperature
*          to improve the EO per serdes. in case it does't
*          improved, do rollback.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] phase                    - algorithm phase
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAdaptiveCtleBasedTemperature
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  phase
);

/**
* @internal mvHwsPortManTuneSet function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManTuneSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_MAN_TUNE_MODE portTuningMode,
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *tuneParams
);

/**
* @internal mvHwsPortManualRxConfig function
* @endinternal
*
* @brief   configures SERDES Rx parameters for all SERDES lanes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*                                      configParams- pointer to array of the config params structures
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualRxConfig
(
    GT_U8                                    devNum,
    GT_U32                                   portGroup,
    GT_U32                                   phyPortNum,
    MV_HWS_PORT_STANDARD                     portMode,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT      *configParams
);

/**
* @internal mvHwsPortManualTxConfig function
* @endinternal
*
* @brief   configures SERDES tx parameters for all SERDES lanes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] configParams             pointer to array of the config params structures
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualTxConfig
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_MAN_TUNE_MODE       portTuningMode,
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *manTxTuneStcPtr
);

/**
* @internal mvHwsPortManualTxConfigGet function
* @endinternal
*
* @brief   Get SERDES tx parameters for first SERDES lane.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] configParams             pointer to array of the config params structures:
*                                      txAmp     Tx amplitude
*                                      txAmpAdj  TX adjust
*                                      txemph0   TX emphasis 0
*                                      txemph1   TX emphasis 1
*                                      txAmpShft TX amplitude shift
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualTxConfigGet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_MAN_TUNE_MODE       portTuningMode,
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *manTxTuneStcPtr
);
#ifndef ALDRIN_DEV_SUPPORT
/**
* @internal mvHwsPortTxDefaultsSet function
* @endinternal
*
* @brief   Set the default Tx parameters on port.
*         Relevant for Bobcat2 and Alleycat3 devices.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxDefaultsSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);
#endif
/**
* @internal mvHwsPortManualCtleConfig function
* @endinternal
*
* @brief   configures SERDES Ctle parameters for specific one SERDES lane or
*         for all SERDES lanes on port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] laneNum                  - Serdes number on port or
*                                      0xFF for all Serdes numbers on port
* @param[in] configParams             pointer to array of the config params structures
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualCtleConfig
(
    GT_U8                               devNum,
    GT_U32                              portGroup,
    GT_U32                              phyPortNum,
    MV_HWS_PORT_STANDARD                portMode,
    GT_U8                               laneNum,
    MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA    *configParams
);

/**
* @internal mvHwsPortManualCtleConfigGet function
* @endinternal
*
* @brief   Get SERDES Ctle parameters of each SERDES lane.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] laneNum                  - Serdes number on port or
*                                      0xFF for all Serdes numbers on port
*
* @param[out] configParams[]           - array of Ctle params structures, Ctle parameters of each SERDES lane.
*                                      Ctle params structures:
*                                      dcGain         DC-Gain value        (rang: 0-255)
*                                      lowFrequency   CTLE Low-Frequency   (rang: 0-15)
*                                      highFrequency  CTLE High-Frequency  (rang: 0-15)
*                                      bandWidth      CTLE Band-width      (rang: 0-15)
*                                      squelch        Signal OK threshold  (rang: 0-310)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualCtleConfigGet
(
    GT_U8                               devNum,
    GT_U32                              portGroup,
    GT_U32                              phyPortNum,
    MV_HWS_PORT_STANDARD                portMode,
    GT_U8                               laneNum,
    MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA    configParams[]
);

/**
* @internal mvHwsPortTestGenerator function
* @endinternal
*
* @brief   Activate the port related PCS Tx generator and Rx checker control.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portPattern              - port tuning mode
* @param[in] actionMode               -
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTestGenerator
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_TEST_GEN_PATTERN  portPattern,
    MV_HWS_PORT_TEST_GEN_ACTION   actionMode
);

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPortTestGeneratorStatus function
* @endinternal
*
* @brief   Get test errors - every get clears the errors.
*         Can be run any time after delete port or after power up
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*                                      portTuningMode - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTestGeneratorStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_TEST_GEN_PATTERN  portPattern,
    MV_HWS_TEST_GEN_STATUS        *status
);
#endif
/**
* @internal mvHwsPortPPMSet function
* @endinternal
*
* @brief   Increase/decrease Tx clock on port (added/sub ppm).
*         Can be run only after create port not under traffic.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portPPM                  - limited to +/- 3 taps
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPPMSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PPM_VALUE        portPPM
);

/**
* @internal mvHwsPortPPMGet function
* @endinternal
*
* @brief   Check the entire line configuration, return ppm value in case of match in all
*         or error in case of different configuration.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] portPPM                  - current PPM
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPPMGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PPM_VALUE        *portPPM
);

/**
* @internal mvHwsPortLoopbackSet function
* @endinternal
*
* @brief   Activates the port loopback modes.
*         Can be run only after create port not under traffic.
* @param[in] devNum     - system device number
* @param[in] portGroup  - port group (core) number
* @param[in] phyPortNum - physical port number
* @param[in] portMode   - port standard metric
* @param[in] lpPlace    - unit for loopback configuration
* @param[in] lpType     - loopback type
*
* @retval 0 - on success
* @retval 1 - on error
*
* @note  For Serdes Loopback modes:
*        - In 'AN_TX_RX' mode, the Rx & Tx Serdes Polarity
*          configurations should be disabled, for getting LinkUp
*          and Traffic on port.
*        - After return to 'Normal' mode, the Rx & Tx Serdes
*          Polarity configuration should be restored, for
*          re-synch again the PCS and MAC units and getting
*          LinkUp and Traffic on port.
*        - Switching between 'AN_TX_RX'/'DIG_RX_TX' modes
*          requires returning to the 'Normal' mode before.
*
*/
GT_STATUS mvHwsPortLoopbackSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_UNIT             lpPlace,
    MV_HWS_PORT_LB_TYPE     lbType
);

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPortLoopbackStatusGet function
* @endinternal
*
* @brief   Retrive MAC loopback status.
*         Can be run only after create port not under traffic.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lpPlace                  - unit for loopback configuration
*
* @param[out] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_UNIT             lpPlace,
    MV_HWS_PORT_LB_TYPE     *lbType
);
#endif
/**
* @internal mvHwsPortLinkStatusGet function
* @endinternal
*
* @brief   Returns the port link status.
*         Can be run any time.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLinkStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
);

/**
* @internal mvHwsPortPolaritySet function
* @endinternal
*
* @brief   Defines the port polarity of the Serdes lanes (Tx/Rx).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] txInvMask                - bitmap of 32 bit, each bit represent Serdes
* @param[in] rxInvMask                - bitmap of 32 bit, each bit represent Serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPolaritySet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_32                   txInvMask,
    GT_32                   rxInvMask
);

/**
* @internal mvHwsPortTxEnable function
* @endinternal
*
* @brief   Turn of the port Tx according to selection.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] enable                   - enable/disable port Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxEnable
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
);


/**
* @internal mvHwsPortTxEnableGet function
* @endinternal
*
* @brief   Retrieve the status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxEnableGet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL serdesTxStatus[]
);

/**
* @internal mvHwsPortSignalDetectGet function
* @endinternal
*
* @brief   Retrieve the signal detected status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] signalDet[]              per serdes.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSignalDetectGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 signalDet[]
);

/**
* @internal mvHwsPortCdrLockStatusGet function
* @endinternal
*
* @brief   Retrieve the CDR lock status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] cdrLockStatus[]          per serdes.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortCdrLockStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 cdrLockStatus[]
);

/**
* @internal mvHwsPortVosOverrideControlModeSet function
* @endinternal
*
* @brief   Set the override mode of the VOS parameters for all ports. If the override mode
*         is set to true, the VOS parameters will be overriden, if set to false, they will
*         not be overriden.
* @param[in] devNum                   - device number.
* @param[in] phyPortNum               - physical port number
* @param[in] vosOverride              - GT_TRUE means override, GT_FALSE means no override.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVosOverrideControlModeSet
(
    GT_U8 devNum,
    GT_U32 phyPortNum,
    GT_BOOL vosOverride
);

/**
* @internal mvHwsPortVosOverrideControlModeGet function
* @endinternal
*
* @brief   Set the override mode of the VOS parameters for all ports. If the override mode
*         is set to true, the VOS parameters will be overriden, if set to false, they will
*         not be overriden.
* @param[in] devNum                   - device number.
*
* @param[out] vosOverridePtr           - (pointer to) current VOS override mode.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVosOverrideControlModeGet
(
    GT_U8   devNum,
    GT_BOOL *vosOverridePtr
);

/**
* @internal hwsPortFaultSendSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending fault signals to partner.
*         on single lane, the result will be local-fault on the sender and remote-fault on the receiver,
*         on multi-lane there will be local-fault on both sides, and there won't be align lock
*         at either side.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] send                     - send or stop sending
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortFaultSendSet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL send
);

GT_STATUS mvHwsPortLkbPortSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U16                          portNum,
    MV_HWS_PORT_STANDARD            portMode,
    GT_U16                          pairNum,
    GT_BOOL                         add
);

GT_STATUS mvHwsPortLkbRegisterApPortSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U16                          apPortNum,
    GT_U16                          pairNum
);

/**
* @internal mvHwsPortMacPcsStatus function
* @endinternal
*
* @brief   port mac pcs status collector.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
*
* @param[out] portStatusPtr           - port Status
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS mvHwsPortMacPcsStatus
(
    IN GT_U8    devNum,
    IN GT_U32  portGroup,
    IN GT_U32    portNum,
    OUT MV_HWS_PORT_STATUS_STC *portStatusPtr
);

/**
* @internal mvHwsRsfecStatusGet function
* @endinternal
*
* @brief   RSFEC status collector.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portGroup                - port group (core) number
*
* @param[out] rsfecCountersPtr          - RSFEC status:
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - RS_FEC_544_514 not support
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is
*         NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS mvHwsRsfecStatusGet
(
    IN GT_U8            devNum,
    IN GT_U32           portGroup,
    IN GT_U32           portNum,
    OUT MV_HWS_RSFEC_STATUS_STC *rsfecStatusPtr
);

/**
* @internal mvHwsFcFecCounterGet function
* @endinternal
*
* @brief   Return FC-FEC counters.
*
* @note   APPLICABLE DEVICES:         AC5P; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portGroup                - port group (core) number
*
* @param[out] fcfecCountersPtr        - pointer to struct that
*                                       will contain the FC-FEC
*                                       counters
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - non FC FEC
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is
*         NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS mvHwsFcFecCounterGet
(
    IN  GT_U8                      devNum,
    IN  GT_U32                     portGroup,
    IN  GT_U32                     portNum,
    OUT MV_HWS_FCFEC_COUNTERS_STC  *fcfecCountersPtr
);


/**
* @internal mvHwsRsFecCorrectedError function
* @endinternal
*
* @brief   Return RS-FEC counters.
*
* @note   APPLICABLE DEVICES:       Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portGroup                - port group (core) number
*
* @param[out] rsfecCountersPtr        - pointer to struct that
*                                       will contain the RS-FEC
*                                       counters
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - RS_FEC_544_514 not support
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is
*         NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS mvHwsRsFecCorrectedError
(
    IN GT_U8    devNum,
    IN GT_U32    portGroup,
    IN GT_U32    portNum,
    OUT MV_HWS_RSFEC_COUNTERS_STC *rsfecCountersPtr
);

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPortSendLocalFaultSet function
* @endinternal
*
* @brief   Configure the PCS to Enable/Disable Send Local Fault.
*          This is done by send fault signals to partner, the result will be local-fault on the
*          sender and remote-fault on the receiver.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] enable                   - GT_TRUE for Fast link DOWN, otherwise normal/UP.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSendLocalFaultSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 enable
);

/**
* @internal mvHwsPortFastLinkDownGet function
* @endinternal
*
* @brief   Get status of Fast link Down/Up.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] pcsType                  - port pcs type
*
* @param[out] status                  - current status of Fast Link.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFastLinkDownGet
(
    IN GT_U8                devNum,
    IN GT_U32               portGroup,
    IN GT_U32               phyPortNum,
    IN MV_HWS_PORT_PCS_TYPE pcsType,
    OUT GT_BOOL            *status
);

/**
* @internal mvHwsUsxExtSerdesGroupParamsGet function
* @endinternal
*
* @brief  USX Serdes group parameters get
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical port number
* @param[in] portMode              - port mode
* @param[out] *firstPortInGroupPtr - (pointer to) first port in USX SD group
* @param[out] *nextPortStepPtr     - (pointer to) next port step
* @param[out] *numOfPortsPtr       - (pointer to) number of ports in USX SD group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsUsxExtSerdesGroupParamsGet
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT GT_U32                  *firstPortInGroupPtr,
    OUT GT_U32                  *nextPortStepPtr,
    OUT GT_U32                  *numOfPortsPtr
);


#endif
/**
* @internal mvHwsSerdesSpeedToIndex function
* @endinternal
*
* @brief  convert serdes speed to index
*
* @param[in] devNum                - system device number
* @param[in] sdSpeed               - serdes speed
* @param[out] *speedIdxPtr       - (pointer to) serdes index
*
* @retval 0                  - on success
* @retval 1                  - on error
*/

GT_U8 mvHwsSerdesSpeedToIndex
(
    IN GT_U8                    devNum,
    IN GT_U32                   sdSpeed
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsPortMiscIf_H */



