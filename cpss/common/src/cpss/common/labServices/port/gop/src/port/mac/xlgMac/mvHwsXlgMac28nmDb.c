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
* @file mvHwsXlgMac28nmDb.c
*
* @brief
*
* @version   4
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMac28nmDb.h>


#ifndef CO_CPU_RUN

#ifdef MV_HWS_REDUCED_BUILD
#if !defined(BC2_DEV_SUPPORT) && !defined(AC3_DEV_SUPPORT) && !defined(AC5_DEV_SUPPORT)
static const  MV_OP_PARAMS mode1LaneSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,               0x2000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x210,      0x5F10}, /* mask value is modified in hwsXlgMac28nmSeqInit */
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x3,        0x1F}
};

#if defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(BOBK_DEV_SUPPORT)
static const MV_OP_PARAMS mode2LaneSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,               0x4000,    0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x810,     0x5F10},  /* mask value is modified in hwsXlgMac28nmSeqInit */
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x3,       0x1F}
};
#else
static const MV_OP_PARAMS mode2LaneSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,               0x4000,    0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x210,     0x5F10},  /* mask value is modified in hwsXlgMac28nmSeqInit */
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x3,       0x1F}
};
#endif

static const MV_OP_PARAMS mode4LaneSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,               0x6000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x810,      0x5F10}, /* mask value is modified in hwsXlgMac28nmSeqInit */
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x3,        0x1F}
};
#else
static const  MV_OP_PARAMS mode1LaneSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,               0x2000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x210,      0x4F10}, /* mask value is modified in hwsXlgMac28nmSeqInit */
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x3,        0x1F}
};
static const MV_OP_PARAMS mode2LaneSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,               0x4000,    0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x210,     0x4F10},  /* mask value is modified in hwsXlgMac28nmSeqInit */
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x3,       0x1F}
};
static const MV_OP_PARAMS mode4LaneSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,               0x6000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x810,      0x4F10}, /* mask value is modified in hwsXlgMac28nmSeqInit */
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x3,        0x1F}
};
#endif
#else
static const  MV_OP_PARAMS mode1LaneSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,               0x2000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x210,      0x4F10}, /* mask value is modified in hwsXlgMac28nmSeqInit */
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x3,        0x1F}
};
static const MV_OP_PARAMS mode2LaneSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,               0x4000,    0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x210,     0x4F10},  /* mask value is modified in hwsXlgMac28nmSeqInit */
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x3,       0x1F}
};
static const MV_OP_PARAMS mode4LaneSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,               0x6000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x810,      0x4F10}, /* mask value is modified in hwsXlgMac28nmSeqInit */
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x3,        0x1F}
};
static const  MV_OP_PARAMS mode1LaneUpdateSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,               0x2000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x210,      0x5F10}, /* mask value is modified in hwsXlgMac28nmSeqInit */
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x3,        0x1F}
};
static const MV_OP_PARAMS mode2LaneUpdateSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,               0x4000,    0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x210,     0x5F10},  /* mask value is modified in hwsXlgMac28nmSeqInit */
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x3,       0x1F}
};
static const MV_OP_PARAMS mode4LaneUpdateSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,               0x6000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x810,      0x5F10}, /* mask value is modified in hwsXlgMac28nmSeqInit */
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x3,        0x1F}
};
static const MV_OP_PARAMS mode2LaneFullUpdateSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,               0x4000,    0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x810,     0x5F10},  /* mask value is modified in hwsXlgMac28nmSeqInit */
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x3,       0x1F}
};
#endif

static const MV_OP_PARAMS resetSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0,  0x0,      (1 << 1)}
};

#if !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BC2_DEV_SUPPORT) && !defined(AC5_DEV_SUPPORT) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    /* in 100G_MLG XLG mode: 4 XLG MACs are bounded */
static const MV_OP_PARAMS reset4Lane_MLG_SeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0,           0x0,    (1 << 1)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0 + 0x1000,  0x0,    (1 << 1)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0 + 0x2000,  0x0,    (1 << 1)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0 + 0x3000,  0x0,    (1 << 1)}
};
#endif

static const MV_OP_PARAMS powerDownSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0,   0x0,        (1 << 1)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,   0x2000,     0x6000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,   (0 << 8),   (1 << 8)}   /* return the PCS mode to default (MMPCS) */
};


#if !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BC2_DEV_SUPPORT) && !defined(AC5_DEV_SUPPORT) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    /* in 100G_MLG XLG mode: 4 XLG MACs are bounded */
static const MV_OP_PARAMS powerDown4Lane_MLG_SeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0,                       0x0,        (1 << 1)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0 + 0x1000,              0x0,        (1 << 1)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0 + 0x2000,              0x0,        (1 << 1)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0 + 0x3000,              0x0,        (1 << 1)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,                       0x2000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3 + 0x1000,              0x2000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3 + 0x2000,              0x2000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3 + 0x3000,              0x2000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,                       0x5210,     0xFFFF},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4 + 0x1000,              0x5210,     0xFFFF},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4 + 0x2000,              0x5210,     0xFFFF},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4 + 0x3000,              0x5210,     0xFFFF},
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,              0x7BE3,     0xFFFF},
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION + 0x1000,     0x7BE3,     0xFFFF},
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION + 0x2000,     0x7BE3,     0xFFFF},
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION + 0x3000,     0x7BE3,     0xFFFF}
};
#endif

static const MV_OP_PARAMS unresetSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0,           (1 << 1),  (1 << 1)}
};

#if !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BC2_DEV_SUPPORT) && !defined(AC5_DEV_SUPPORT) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    /* in 100G_MLG XLG mode: 4 XLG MACs are bounded */
static const MV_OP_PARAMS unreset4Lane_MLG_SeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0,           (1 << 1),  (1 << 1)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0 + 0x1000,  (1 << 1),  (1 << 1)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0 + 0x2000,  (1 << 1),  (1 << 1)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0 + 0x3000,  (1 << 1),  (1 << 1)}
};
#endif

static const MV_OP_PARAMS lbNormalSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER1,           0x0,      (3 << 13)}
};

static const MV_OP_PARAMS lbRx2TxSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER1,          (2 << 13), (3 << 13)}
};

static const MV_OP_PARAMS lbTx2RxSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER1,          (1 << 13), (3 << 13)}
};

/* For BC2 and AC3 only for XLG ports RX is configured via MAC - TX is controlled by FCA unit */
static const MV_OP_PARAMS fcDisableSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0         ,  (0 << 7),  (1 << 7)},
    {MMPCS_UNIT , 0x600 /* FCA Control register offset */, (1 << 1),  (1 << 1)} /* enable bypass FCA */
};
static const MV_OP_PARAMS fcBothSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0,           (1 << 7),  (1 << 7)},
    {MMPCS_UNIT , 0x600                         ,           (0 << 1),  (1 << 1)} /* disable bypass FCA */
};
static const MV_OP_PARAMS fcRxOnlySeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0,           (1 << 7),  (1 << 7)},
    {MMPCS_UNIT , 0x600                         ,           (1 << 1),  (1 << 1)} /* enable bypass FCA */
};
static const MV_OP_PARAMS fcTxOnlySeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0,           (0 << 7),  (1 << 7)},
    {MMPCS_UNIT , 0x600                         ,           (0 << 1),  (1 << 1)} /* disable bypass FCA */
};
#if !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BC2_DEV_SUPPORT) && !defined(AC5_DEV_SUPPORT)
/*  for Bobcat3 port modes: 25G, 50G and 100G_MLG */
/* ===============================================*/
static const MV_OP_PARAMS mode1Lane_25G_SeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x210,      0x5F10},
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x880F,     0xF81F},
    {XLGMAC_UNIT, MSM_DIC_BUDGET_COMPENSATION,                  0x3FFF,     0xFFFF}
};

static const MV_OP_PARAMS mode2Lane_50G_SeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x810,      0x5F10},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,               0x6000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x880F,     0xF81F},
    {XLGMAC_UNIT, MSM_DIC_BUDGET_COMPENSATION,                  0x3FFF,     0xFFFF}
};

static const MV_OP_PARAMS mode1Lane_25G_RSFEC_SeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x210,      0x5F10},
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x880F,     0xF81F},
    {XLGMAC_UNIT, MSM_DIC_BUDGET_COMPENSATION,                  0x23FF,     0xFFFF}
};

static const MV_OP_PARAMS mode2Lane_50G_RSFEC_SeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,               0x810,      0x5F10},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,               0x6000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,      0x880F,     0xF81F},
    {XLGMAC_UNIT, MSM_DIC_BUDGET_COMPENSATION,                  0x23FF,     0xFFFF}
};
#ifndef MV_HWS_REDUCED_BUILD
    /* in 100G_MLG XLG mode: 4 XLG MACs are bounded */
static const MV_OP_PARAMS mode4Lane_MLG_SeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,                       0x6000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3 + 0x1000,              0x2000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3 + 0x2000,              0x6000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3 + 0x3000,              0x2000,     0xE000},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,                       0x810,      0x5F10},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4 + 0x1000,              0x210,      0x5F10},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4 + 0x2000,              0x810,      0x5F10},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4 + 0x3000,              0x210,      0x5F10},
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,              0x880F,     0xF81F},
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION + 0x1000,     0x880F,     0xF81F},
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION + 0x2000,     0x880F,     0xF81F},
    {XLGMAC_UNIT, MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION + 0x3000,     0x880F,     0xF81F},
    {XLGMAC_UNIT, MSM_DIC_BUDGET_COMPENSATION,                          0x3FFF,     0xFFFF},
    {XLGMAC_UNIT, MSM_DIC_BUDGET_COMPENSATION + 0x1000,                 0x1CCC,     0xFFFF},
    {XLGMAC_UNIT, MSM_DIC_BUDGET_COMPENSATION + 0x2000,                 0x3FFF,     0xFFFF},
    {XLGMAC_UNIT, MSM_DIC_BUDGET_COMPENSATION + 0x3000,                 0x1CCC,     0xFFFF}
};
#endif
#endif

/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsXlgMac28nmSeqDb[MV_MAC_XLG_LAST_SEQ] =
{
    {unresetSeqParams,   MV_SEQ_SIZE(unresetSeqParams)},    /* XLGMAC_UNRESET_SEQ     */
    {resetSeqParams,     MV_SEQ_SIZE(resetSeqParams)},      /* XLGMAC_RESET_SEQ       */
    {mode1LaneSeqParams, MV_SEQ_SIZE(mode1LaneSeqParams)},  /* XLGMAC_MODE_1_Lane_SEQ */
    {mode2LaneSeqParams, MV_SEQ_SIZE(mode2LaneSeqParams)},  /* XLGMAC_MODE_2_Lane_SEQ */
    {mode4LaneSeqParams, MV_SEQ_SIZE(mode4LaneSeqParams)},  /* XLGMAC_MODE_4_Lane_SEQ */
    {lbNormalSeqParams,  MV_SEQ_SIZE(lbNormalSeqParams)},   /* XLGMAC_LPBK_NORMAL_SEQ */
    {lbRx2TxSeqParams,   MV_SEQ_SIZE(lbRx2TxSeqParams)},    /* XLGMAC_LPBK_RX2TX_SEQ  */
    {lbTx2RxSeqParams,   MV_SEQ_SIZE(lbTx2RxSeqParams)},    /* XLGMAC_LPBK_TX2RX_SEQ  */
    {fcDisableSeqParams, MV_SEQ_SIZE(fcDisableSeqParams)},  /* XLGMAC_FC_DISABLE_SEQ */
    {fcBothSeqParams,    MV_SEQ_SIZE(fcBothSeqParams)},     /* XLGMAC_FC_BOTH_SEQ */
    {fcRxOnlySeqParams,  MV_SEQ_SIZE(fcRxOnlySeqParams)},   /* XLGMAC_FC_RX_ONLY_SEQ */
    {fcTxOnlySeqParams,  MV_SEQ_SIZE(fcTxOnlySeqParams)},   /* XLGMAC_FC_TX_ONLY_SEQ */
    {powerDownSeqParams, MV_SEQ_SIZE(powerDownSeqParams)},  /* XLGMAC_POWER_DOWN_SEQ */
#if !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BC2_DEV_SUPPORT) && !defined(AC5_DEV_SUPPORT)
    {mode1Lane_25G_SeqParams,       MV_SEQ_SIZE(mode1Lane_25G_SeqParams)},      /* XLGMAC_MODE_1_Lane_25G_SEQ */
    {mode2Lane_50G_SeqParams,       MV_SEQ_SIZE(mode2Lane_50G_SeqParams)},      /* XLGMAC_MODE_2_Lane_50G_SEQ */
    {mode1Lane_25G_RSFEC_SeqParams, MV_SEQ_SIZE(mode1Lane_25G_RSFEC_SeqParams)},/* XLGMAC_MODE_1_Lane_25G_RS_FEC_SEQ */
    {mode2Lane_50G_RSFEC_SeqParams, MV_SEQ_SIZE(mode2Lane_50G_RSFEC_SeqParams)},/* XLGMAC_MODE_2_Lane_50G_RS_FEC_SEQ */
#ifndef MV_HWS_REDUCED_BUILD
    {mode4Lane_MLG_SeqParams,       MV_SEQ_SIZE(mode4Lane_MLG_SeqParams)},      /* XLGMAC_MODE_4_Lane_MLG_SEQ */
    {reset4Lane_MLG_SeqParams,      MV_SEQ_SIZE(reset4Lane_MLG_SeqParams)},     /* XLGMAC_RESET_4_Lane_MLG_SEQ */
    {unreset4Lane_MLG_SeqParams,    MV_SEQ_SIZE(unreset4Lane_MLG_SeqParams)},   /* XLGMAC_UNRESET_4_Lane_MLG_SEQ */
    {powerDown4Lane_MLG_SeqParams,  MV_SEQ_SIZE(powerDown4Lane_MLG_SeqParams)}, /* XLGMAC_POWER_DOWN_4_Lane_MLG_SEQ */
    {mode1LaneUpdateSeqParams,      MV_SEQ_SIZE(mode1LaneUpdateSeqParams)},     /* XLGMAC_MODE_1_Lane_UPDATE_SEQ */
    {mode2LaneUpdateSeqParams,      MV_SEQ_SIZE(mode2LaneUpdateSeqParams)},     /* XLGMAC_MODE_2_Lane_UPDATE_SEQ */
    {mode4LaneUpdateSeqParams,      MV_SEQ_SIZE(mode4LaneUpdateSeqParams)},     /* XLGMAC_MODE_4_Lane_UPDATE_SEQ */
    {mode2LaneFullUpdateSeqParams,  MV_SEQ_SIZE(mode2LaneFullUpdateSeqParams)}, /* XLGMAC_MODE_2_Lane_FULL_UPDATE_SEQ */
#else
    {NULL,               0},                                /* XLGMAC_MODE_4_Lane_MLG_SEQ */
    {NULL,               0},                                /* XLGMAC_RESET_4_Lane_MLG_SEQ */
    {NULL,               0},                                /* XLGMAC_UNRESET_4_Lane_MLG_SEQ */
    {NULL,               0}                                 /* XLGMAC_POWER_DOWN_4_Lane_MLG_SEQ */
#endif
#else
    {NULL,               0},                                /* XLGMAC_MODE_1_Lane_25G_SEQ */
    {NULL,               0},                                /* XLGMAC_MODE_2_Lane_50G_SEQ */
    {NULL,               0},                                /* XLGMAC_MODE_1_Lane_25G_RS_FEC_SEQ */
    {NULL,               0},                                /* XLGMAC_MODE_2_Lane_50G_RS_FEC_SEQ */
    {NULL,               0},                                /* XLGMAC_MODE_4_Lane_MLG_SEQ */
    {NULL,               0},                                /* XLGMAC_RESET_4_Lane_MLG_SEQ */
    {NULL,               0},                                /* XLGMAC_UNRESET_4_Lane_MLG_SEQ */
    {NULL,               0}                                 /* XLGMAC_POWER_DOWN_4_Lane_MLG_SEQ */
#endif

};

GT_STATUS hwsXlgMac28nmSeqInit(GT_U8 devNum)
{
    devNum = devNum;
#if 0
    MV_OP_PARAMS *updateParams;

    /* XLG MAC Sequences update */
    /* Note: This code section doesn't support multiple device (such as AC3 with BobK)
             since it changes the common DB */
    if (HWS_DEV_GOP_REV(devNum) != GOP_28NM_REV1)
    {
        updateParams = &hwsXlgMac28nmSeqDb[XLGMAC_MODE_1_Lane_SEQ].cfgSeq[1];
        updateParams->mask = 0x5F10;
        updateParams = &hwsXlgMac28nmSeqDb[XLGMAC_MODE_2_Lane_SEQ].cfgSeq[1];
        updateParams->mask = 0x5F10;
        updateParams = &hwsXlgMac28nmSeqDb[XLGMAC_MODE_4_Lane_SEQ].cfgSeq[1];
        updateParams->mask = 0x5F10;
    }

    if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
    {
        /* for BobK and Aldrin, TxDMA interface should work in 256bits. */
        updateParams = &hwsXlgMac28nmSeqDb[XLGMAC_MODE_2_Lane_SEQ].cfgSeq[1];
        updateParams->operData = 0x810;
    }
#endif
    return GT_OK;
}

GT_STATUS hwsXlgMac28nmSeqGet(MV_HWS_XLG_MAC_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MV_MAC_XLG_LAST_SEQ) ||
      (hwsXlgMac28nmSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsXlgMac28nmSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}

#endif




