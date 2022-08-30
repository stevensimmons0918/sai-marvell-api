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
* mvHwsMMPcsDb.c
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcs28nmDb.h>

#define MMPCS_LANE2_RESET_SEQ_PARAMS_MAC  \
    {MMPCS_UNIT, PCS_RESET_REG,                         0x800,  0xFFF7}, \
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x800,  0xFFF7}

#define MMPCS_LANE4_RESET_SEQ_PARAMS_MAC  \
    {MMPCS_UNIT, PCS_RESET_REG,                         0x800,  0xFFF7}, \
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x800,  0xFFF7}, \
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x800,  0xFFF7}, \
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x800,  0xFFF7}

#define MMPCS_FABRIC_RESET_LANE2_SEQ_PARAMS_MAC  \
    {MMPCS_UNIT, PCS_RESET_REG,                         0x800,  0xFFFF}, \
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x800,  0xFFFF}

static const MV_OP_PARAMS mmpcsLane1ModeSeqParams[] = {
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x3820, 0xFBFF},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0x0,    0x300}
};

static const MV_OP_PARAMS mmpcsLane2ModeSeqParams[] = {
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x3920, 0xFBFF},
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL + 0x1000,        0x3000, 0x3000},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0,      0x300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x1000,        0x100,  0x300}
};

static const MV_OP_PARAMS mmpcsLane4ModeSeqParams[] = {
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x3A20, 0xFBFF},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0x0,    0x300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x1000,        0x100,  0x300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x2000,        0x200,  0x300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x3000,        0x300,  0x300}
};

#ifndef ALDRIN_DEV_SUPPORT
/*  for Bobcat3 port modes: 25G, 50G  */
/* ================================== */
static const MV_OP_PARAMS mmpcsLane1_25G_ModeSeqParams[] = {
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0xB820, 0xFBFF},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0x0,    0x300}
};

/* 2 x 25G, but 4 PCS lanes for FC-FEC mode */
static const MV_OP_PARAMS mmpcsLane4_50G_ModeSeqParams[] = {
    {MMPCS_UNIT, PCS40G_ALIGNMENT_CYCLE_CONTROL,        0x3FFF, 0xFFFF},

    {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x600,  0x700},
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL + 0x1000,        0x600,  0x700},
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL + 0x2000,        0x600,  0x700},
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL + 0x3000,        0x600,  0x700},

    {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0x0,    0x300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x1000,        0x100,  0x300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x2000,        0x200,  0x300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x3000,        0x300,  0x300},

    {MMPCS_UNIT, PCS_RESET_REG,                         0x10,   0xF0},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x10,   0xF0},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x10,   0xF0},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x10,   0xF0},

    {MMPCS_UNIT, PCS40G_COMMON_CONTROL_2,               0x1,    0x1}
};

static const MV_OP_PARAMS mmpcsLane1_25G_ResetSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x820,  0xFFFF}
};

static const MV_OP_PARAMS mmpcsLane1_25G_PowerDownSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x820,  0xFFFF},
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x0,    0x8300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0x0,    0x300}
};

    /* in 50G PCS mode: 4 PCS lanes are bounded */
static const MV_OP_PARAMS mmpcsLane4_50G_ResetSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x800,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x800,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x800,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x800,  0xFF07}
};

    /* in 50G PCS mode: 4 PCS lanes are bounded */
static const MV_OP_PARAMS mmpcsLane4_50G_PowerDownSeqParams[] = {
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x0,    0x700},
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL + 0x1000,        0x0,    0x700},
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL + 0x2000,        0x0,    0x700},
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL + 0x3000,        0x0,    0x700},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0x0,    0x300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x1000,        0x0,    0x300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x2000,        0x0,    0x300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x3000,        0x0,    0x300},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x810,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x810,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x810,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x810,  0xFFF7},
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x0,    0x8300},
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL_2,               0x0,    0x1}
};

static const MV_OP_PARAMS mmpcsLane1_25G_UnResetSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x820,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x20,   0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x820,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x821,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x825,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x827,  0xFFFF}
};

static const MV_OP_PARAMS mmpcsLane1_26_7G_UnResetSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x810,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x10,   0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x810,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x811,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x815,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x817,  0xFFFF}
};
#endif

static const MV_OP_PARAMS mmpcsMultiLane_StartSendFaultSeqParams[] = {
    /* Lane 0 markers */
    {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_0,             0x0,  0xFFFF},
    {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_1,             0x0,  0xFFFF},
    {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_2,             0x0,  0xFFFF},
    {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_3,             0x0,  0xFFFF},

    /* Lane 1 markers */
    {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_0,             0x0,  0xFFFF},
    {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_1,             0x0,  0xFFFF},
    {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_2,             0x0,  0xFFFF},
    {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_3,             0x0,  0xFFFF},

    /* Lane 2 markers */
    {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_0,             0x0,  0xFFFF},
    {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_1,             0x0,  0xFFFF},
    {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_2,             0x0,  0xFFFF},
    {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_3,             0x0,  0xFFFF},

    /* Lane 3 markers */
    {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_0,             0x0,  0xFFFF},
    {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_1,             0x0,  0xFFFF},
    {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_2,             0x0,  0xFFFF},
    {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_3,             0x0,  0xFFFF}
};

static const MV_OP_PARAMS mmpcsMultiLane_StopSendFaultSeqParams[] = {
    /* Lane 0 markers */
    {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_0,             0x7690,  0xFFFF},
    {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_1,             0x3e47,  0xFFFF},
    {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_2,             0x896f,  0xFFFF},
    {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_3,             0xc1b8,  0xFFFF},

    /* Lane 1 markers */
    {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_0,             0xc4f0,  0xFFFF},
    {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_1,             0x63e6,  0xFFFF},
    {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_2,             0x3b0f,  0xFFFF},
    {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_3,             0x9c19,  0xFFFF},

    /* Lane 2 markers */
    {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_0,             0x65c5,  0xFFFF},
    {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_1,             0x339b,  0xFFFF},
    {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_2,             0x9a3a,  0xFFFF},
    {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_3,             0xcc64,  0xFFFF},

    /* Lane 3 markers */
    {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_0,             0x79a2,  0xFFFF},
    {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_1,             0xd83d,  0xFFFF},
    {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_2,             0x865d,  0xFFFF},
    {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_3,             0x27c2,  0xFFFF}
};

static const MV_OP_PARAMS mmpcsMultiLane_StopSendFaultRsFecSeqParams[] = {
        /* Lane 0 markers */
    {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_0,             0x68c1,  0xFFFF},
    {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_1,             0x3321,  0xFFFF},
    {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_2,             0x973e,  0xFFFF},
    {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_3,             0xccde,  0xFFFF},

    /* Lane 1 markers */
    {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_0,             0xc4f0,  0xFFFF},
    {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_1,             0x63e6,  0xFFFF},
    {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_2,             0x3b0f,  0xFFFF},
    {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_3,             0x9c19,  0xFFFF},

    /* Lane 2 markers */
    {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_0,             0x65c5,  0xFFFF},
    {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_1,             0x339b,  0xFFFF},
    {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_2,             0x9a3a,  0xFFFF},
    {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_3,             0xcc64,  0xFFFF},

    /* Lane 3 markers */
    {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_0,             0x79a2,  0xFFFF},
    {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_1,             0xd83d,  0xFFFF},
    {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_2,             0x865d,  0xFFFF},
    {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_3,             0x27c2,  0xFFFF}
};

/*  for 10G/20G/40G port modes  */
/* ============================ */
static const MV_OP_PARAMS mmpcsLane1ResetSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFFFF}
};

static const MV_OP_PARAMS mmpcsLane2ResetSeqParams[] = {
    MMPCS_LANE2_RESET_SEQ_PARAMS_MAC
};

static const MV_OP_PARAMS mmpcsLane4ResetSeqParams[] = {
    MMPCS_LANE4_RESET_SEQ_PARAMS_MAC
};

static const MV_OP_PARAMS mmpcsLane2PowerDownSeqParams[] = {
    MMPCS_LANE2_RESET_SEQ_PARAMS_MAC,
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x0000, 0x0300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0x0,    0x300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x1000,        0x0,    0x300}
};

static const MV_OP_PARAMS mmpcsLane4PowerDownSeqParams[] = {
    MMPCS_LANE4_RESET_SEQ_PARAMS_MAC,
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x0000, 0x0300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0x0,    0x300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x1000,        0x0,    0x300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x2000,        0x0,    0x300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x3000,        0x0,    0x300}
};

#ifdef AC5_DEV_SUPPORT
static const MV_OP_PARAMS mmpcsLane1UnResetSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x880,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x080,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x880,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x881,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x885,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x887,  0xFF07}
};

static const MV_OP_PARAMS mmpcsLane2UnResetSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x840,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x40,   0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x40,   0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x840,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x841,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x841,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x845,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x845,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x847,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x847,  0xFF07}
};
/* AC5 port 29 extended mode (pcs 50 for 2nd lane) */
static const MV_OP_PARAMS mmpcsFabricUnResetLane2SeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x840,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x40,   0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x40,   0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x840,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x841,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x841,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x845,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x845,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x847,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x847,  0xFF07}
};
#else
static const MV_OP_PARAMS mmpcsLane1UnResetSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x880,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x080,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x880,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x881,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x885,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x887,  0xFFF7}
};

static const MV_OP_PARAMS mmpcsLane2UnResetSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x840,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x40,   0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x40,   0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x840,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x841,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x841,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x845,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x845,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x847,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x847,  0xFFF7}
};
/* AC5 port 29 extended mode (pcs 50 for 2nd lane) */
static const MV_OP_PARAMS mmpcsFabricUnResetLane2SeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x840,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x40,   0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x40,   0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x840,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x841,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x841,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x845,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x845,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x847,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x847,  0xFFF7}
};
#endif
#ifndef MV_HWS_REDUCED_BUILD
static const MV_OP_PARAMS mmpcsLane1UnResetUpdateSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x880,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x080,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x880,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x881,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x885,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x887,  0xFF07}
};

static const MV_OP_PARAMS mmpcsLane2UnResetUpdateSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x840,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x40,   0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x40,   0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x840,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x841,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x841,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x845,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x845,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x847,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x847,  0xFF07}
};
/* AC5 port 29 extended mode (pcs 50 for 2nd lane) */
static const MV_OP_PARAMS mmpcsFabricLane2UnResetUpdateSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x840,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x40,   0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x40,   0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x840,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x841,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x841,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x845,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x845,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x847,  0xFF07},
    {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x847,  0xFF07}
};
#endif


static const MV_OP_PARAMS mmpcsLane4UnResetSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x820,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x820,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x820,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x820,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x20,   0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x20,   0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x20,   0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x20,   0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x820,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x820,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x820,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x820,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x821,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x821,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x821,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x821,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x825,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x825,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x825,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x825,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x827,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x827,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x827,  0xFFF7},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x827,  0xFFF7}
};

/* GOP Rev 3 (BobK) */
/* ================ */
static const MV_OP_PARAMS mmpcsGopRev3Lane1UnResetSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x40,   0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x841,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x845,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x847,  0xFFFF}
};

static const MV_OP_PARAMS mmpcsGopRev3Lane2UnResetSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x820,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x820,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x20,   0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x20,   0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x820,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x820,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x821,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x821,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x825,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x825,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x827,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x827,  0xFFFF}
};

static const MV_OP_PARAMS mmpcsGopRev3Lane4UnResetSeqParams[] = {
    {MMPCS_UNIT, PCS_RESET_REG,                         0x810,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x810,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x810,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x810,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x10,   0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x10,   0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x10,   0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x10,   0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x810,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x810,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x810,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x810,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x811,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x811,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x811,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x811,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x815,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x815,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x815,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x815,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG,                         0x817,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x817,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x817,  0xFFFF},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x817,  0xFFFF}
};
#ifndef ALDRIN_DEV_SUPPORT
/* AlleyCat3 Port 29 only */
/* ====================== */
static const MV_OP_PARAMS mmpcsFabricModeLane2SeqParams[] = {
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x3920, 0xFBFF},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0x0,    0x300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION + (0x1000 * 21), 0x100,  0x300}
};

static const MV_OP_PARAMS mmpcsFabricResetLane2SeqParams[] = {
    MMPCS_FABRIC_RESET_LANE2_SEQ_PARAMS_MAC
};

static const MV_OP_PARAMS mmpcsFabricPowerDownLane2SeqParams[] = {
    MMPCS_FABRIC_RESET_LANE2_SEQ_PARAMS_MAC,
    {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x0000, 0x0300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0x0,    0x300},
    {MMPCS_UNIT, CHANNEL_CONFIGURATION + (0x1000 * 21), 0x0,    0x300}
};
#endif

static const MV_OP_PARAMS mmpcsPcsMode2Lane_0SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x4FFF4FFF,     0xFFFFFFFF},
    {CG_UNIT,           CG_CONTROL_2,                           0x604,          0x615}, /* bit {4} should not be restored to default value in power down if one more 50G port is defined in neighbor slot*/
    {CG_UNIT,           CG_CONTROL_1,                           0x3300,         0x3300},
    {CG_UNIT,           CG_CONTROL_0,                           0x800000,       0x800000}, /* this bit should not be restored to default value in power down if one more 50G port is defined in neighbor slot*/
    {CG_UNIT,           CG_CONTROL_2,                           0x6000,         0x6000},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF},
    {MMPCS_UNIT,        PCS40G_COMMON_CONTROL,                  0x200,          0x300},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x10,           0xF0},
};

static const MV_OP_PARAMS mmpcsPcsMode2Lane_2SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x4FFF4FFF,     0xFFFFFFFF},
    {CG_UNIT,           CG_CONTROL_2,                           0x1808,         0x181A}, /* bit {4} should not be restored to default value in power down if one more 50G port is defined in neighbor slot*/
    {CG_UNIT,           CG_CONTROL_1,                           0xCC00,         0xCC00},
    {CG_UNIT,           CG_CONTROL_0,                           0x800000,       0x800000}, /* this bit should not be restored to default value in power down if one more 50G port is defined in neighbor slot*/
    {CG_UNIT,           CG_CONTROL_2,                           0x18000,        0x18000},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF},
    {MMPCS_UNIT,        PCS40G_COMMON_CONTROL,                  0x200,          0x300},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x10,           0xF0},
};

static const MV_OP_PARAMS mmpcsPcsRsFec2Lane_0SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_CONTROL_2,                           0x60000,        0x60000},
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x4FFF4FFF,     0xFFFFFFFF},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0,   0x33,           0x3F},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1,   0x33,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x4FFF,         0xFFFF}
};

static const MV_OP_PARAMS mmPcsPowerDown2Lane_0SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0x3},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x30},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x3000},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4},
    {CG_UNIT,           CG_CONTROL_0,                           0x0,            0x800000},
    {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x3300},
    {CG_UNIT,           CG_CONTROL_2,                           0x10,           0x66615},
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x3FFF3FFF,     0xFFFFFFFF},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0,   0x2A,           0x3F},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1,   0x2A,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0},
    {MMPCS_UNIT,        PCS40G_COMMON_CONTROL,                  0x0,            0x0300}/*,
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4000000}*/
};

static const MV_OP_PARAMS mmPcsPowerDown2Lane_2SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0xC},
    {CG_UNIT,           CG_RESETS,                              0x0,            0xC0},
    {CG_UNIT,           CG_RESETS,                              0x0,            0xC000},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4},
    {CG_UNIT,           CG_CONTROL_0,                           0x0,            0x800000},
    {CG_UNIT,           CG_CONTROL_1,                           0x0,            0xCC00},
    {CG_UNIT,           CG_CONTROL_2,                           0x10,           0x19981A},
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x3FFF3FFF,     0xFFFFFFFF},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2,   0x2A,           0x3F},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3,   0x2A,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0},
    {MMPCS_UNIT,        PCS40G_COMMON_CONTROL,                  0x0,            0x0300}/*,
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4000000}*/
};

/* reduced power down sequence should be executed when at least one more port is defined for the same GC MAC */
static const MV_OP_PARAMS mmPcsReducedPowerDown2Lane_0SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0x3},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x30},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x3000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4},
    {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x3300},
    {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x66605},
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x3FFF3FFF,     0xFFFFFFFF},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0,   0x2A,           0x3F},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1,   0x2A,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0},
    {MMPCS_UNIT,        PCS40G_COMMON_CONTROL,                  0x0,            0x0300}
};

/* reduced power down sequence should be executed when at least one more port is defined for the same GC MAC */
static const MV_OP_PARAMS mmPcsReducedPowerDown2Lane_2SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0xC},
    {CG_UNIT,           CG_RESETS,                              0x0,            0xC0},
    {CG_UNIT,           CG_RESETS,                              0x0,            0xC000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4},
    {CG_UNIT,           CG_CONTROL_1,                           0x0,            0xCC00},
    {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x19980A},
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x3FFF3FFF,     0xFFFFFFFF},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2,   0x2A,           0x3F},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3,   0x2A,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0},
    {MMPCS_UNIT,        PCS40G_COMMON_CONTROL,                  0x0,            0x0300}
};

static const MV_OP_PARAMS mmpcsPcsRsFec2Lane_2SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_CONTROL_2,                           0x180000,       0x180000},
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x4FFF4FFF,     0xFFFFFFFF},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2,   0x33,           0x3F},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3,   0x33,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x4FFF,         0xFFFF}
};

static const MV_OP_PARAMS mmPcsReset2Lane_0SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0x3},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x30},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x3000},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}
};

static const MV_OP_PARAMS mmPcsReset2Lane_2SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0xC},
    {CG_UNIT,           CG_RESETS,                              0x0,            0xC0},
    {CG_UNIT,           CG_RESETS,                              0x0,            0xC000},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}
};

static const MV_OP_PARAMS mmPcsUnreset2Lane_0SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x3,            0x3},
    {CG_UNIT,           CG_RESETS,                              0x3000,         0x3000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x4,            0x4},
    {CG_UNIT,           CG_RESETS,                              0x10000000,     0x10000000},
    {CG_UNIT,           CG_RESETS,                              0x30,           0x30}
};

static const MV_OP_PARAMS mmPcsUnreset2Lane_2SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0xC,            0xC},
    {CG_UNIT,           CG_RESETS,                              0xC000,         0xC000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x4,            0x4},
    {CG_UNIT,           CG_RESETS,                              0x10000000,     0x10000000},
    {CG_UNIT,           CG_RESETS,                              0xC0,           0xC0}
};


/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsMmPsc28nmSeqDb[] =
{
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_RESET_SEQ                 */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_UNRESET_SEQ               */
    {mmpcsLane1ModeSeqParams,          MV_SEQ_SIZE(mmpcsLane1ModeSeqParams)},          /* MMPCS_MODE_1_LANE_SEQ           */
    {mmpcsLane2ModeSeqParams,          MV_SEQ_SIZE(mmpcsLane2ModeSeqParams)},          /* MMPCS_MODE_2_LANE_SEQ           */
    {mmpcsLane4ModeSeqParams,          MV_SEQ_SIZE(mmpcsLane4ModeSeqParams)},          /* MMPCS_MODE_4_LANE_SEQ           */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_LPBK_NORMAL_SEQ           */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_LPBK_RX2TX_SEQ            */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_LPBK_TX2RX_SEQ            */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_GEN_NORMAL_SEQ            */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_GEN_KRPAT_SEQ             */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_FABRIC_MODE_1_LANE_SEQ    */
#ifndef ALDRIN_DEV_SUPPORT
    {mmpcsFabricModeLane2SeqParams,    MV_SEQ_SIZE(mmpcsFabricModeLane2SeqParams)},    /* MMPCS_FABRIC_MODE_2_LANE_SEQ    */
#else
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_FABRIC_MODE_2_LANE_SEQ    */
#endif
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_FABRIC_MODE_4_LANE_SEQ    */
    {mmpcsLane1ResetSeqParams,         MV_SEQ_SIZE(mmpcsLane1ResetSeqParams)},         /* MMPCS_RESET_1_LANE_SEQ          */
    {mmpcsLane2ResetSeqParams,         MV_SEQ_SIZE(mmpcsLane2ResetSeqParams)},         /* MMPCS_RESET_2_LANE_SEQ          */
    {mmpcsLane4ResetSeqParams,         MV_SEQ_SIZE(mmpcsLane4ResetSeqParams)},         /* MMPCS_RESET_4_LANE_SEQ          */
    {mmpcsLane1UnResetSeqParams,       MV_SEQ_SIZE(mmpcsLane1UnResetSeqParams)},       /* MMPCS_UNRESET_1_LANE_SEQ        */
    {mmpcsLane2UnResetSeqParams,       MV_SEQ_SIZE(mmpcsLane2UnResetSeqParams)},       /* MMPCS_UNRESET_2_LANE_SEQ        */
    {mmpcsLane4UnResetSeqParams,       MV_SEQ_SIZE(mmpcsLane4UnResetSeqParams)},       /* MMPCS_UNRESET_4_LANE_SEQ        */
#ifndef ALDRIN_DEV_SUPPORT
    {mmpcsFabricResetLane2SeqParams,   MV_SEQ_SIZE(mmpcsFabricResetLane2SeqParams)},   /* MMPCS_FABRIC_RESET_2_LANE_SEQ   */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_FABRIC_RESET_4_LANE_SEQ   */
    {mmpcsFabricUnResetLane2SeqParams, MV_SEQ_SIZE(mmpcsFabricUnResetLane2SeqParams)}, /* MMPCS_FABRIC_UNRESET_2_LANE_SEQ */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_FABRIC_UNRESET_4_LANE_SEQ */
#else
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_FABRIC_RESET_2_LANE_SEQ   */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_FABRIC_RESET_4_LANE_SEQ   */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_FABRIC_UNRESET_2_LANE_SEQ */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_FABRIC_UNRESET_4_LANE_SEQ */
#endif
    {mmpcsGopRev3Lane1UnResetSeqParams, MV_SEQ_SIZE(mmpcsGopRev3Lane1UnResetSeqParams)}, /* MMPCS_GOP_REV3_UNRESET_1_LANE_SEQ */
    {mmpcsGopRev3Lane2UnResetSeqParams, MV_SEQ_SIZE(mmpcsGopRev3Lane2UnResetSeqParams)}, /* MMPCS_GOP_REV3_UNRESET_2_LANE_SEQ */
    {mmpcsGopRev3Lane4UnResetSeqParams, MV_SEQ_SIZE(mmpcsGopRev3Lane4UnResetSeqParams)}, /* MMPCS_GOP_REV3_UNRESET_4_LANE_SEQ */
    {mmpcsLane2PowerDownSeqParams,       MV_SEQ_SIZE(mmpcsLane2PowerDownSeqParams)},      /* MMPCS_POWER_DOWN_2_LANE_SEQ        */
    {mmpcsLane4PowerDownSeqParams,       MV_SEQ_SIZE(mmpcsLane4PowerDownSeqParams)},      /* MMPCS_POWER_DOWN_4_LANE_SEQ        */
#ifndef ALDRIN_DEV_SUPPORT
    {mmpcsFabricPowerDownLane2SeqParams, MV_SEQ_SIZE(mmpcsFabricPowerDownLane2SeqParams)}, /* MMPCS_FABRIC_POWER_DOWN_2_LANE_SEQ */
    {mmpcsLane1_25G_ModeSeqParams,      MV_SEQ_SIZE(mmpcsLane1_25G_ModeSeqParams)},        /* MMPCS_MODE_1_LANE_25G_SEQ        */
    {mmpcsLane4_50G_ModeSeqParams,      MV_SEQ_SIZE(mmpcsLane4_50G_ModeSeqParams)},        /* MMPCS_MODE_4_LANE_50G_SEQ        */
    {mmpcsLane1_25G_ResetSeqParams,     MV_SEQ_SIZE(mmpcsLane1_25G_ResetSeqParams)},       /* MMPCS_RESET_1_LANE_25G_SEQ       */
    {mmpcsLane4_50G_ResetSeqParams,     MV_SEQ_SIZE(mmpcsLane4_50G_ResetSeqParams)},       /* MMPCS_RESET_4_LANE_50G_SEQ       */
    {mmpcsLane1_25G_PowerDownSeqParams, MV_SEQ_SIZE(mmpcsLane1_25G_PowerDownSeqParams)},   /* MMPCS_POWER_DOWN_1_LANE_25G_SEQ  */
    {mmpcsLane4_50G_PowerDownSeqParams, MV_SEQ_SIZE(mmpcsLane4_50G_PowerDownSeqParams)},   /* MMPCS_POWER_DOWN_4_LANE_50G_SEQ  */
    {mmpcsLane1_25G_UnResetSeqParams,   MV_SEQ_SIZE(mmpcsLane1_25G_UnResetSeqParams)},      /* MMPCS_UNRESET_1_LANE_25G_SEQ     */
#else
    {NULL,                              MV_SEQ_SIZE(0)},                                   /* MMPCS_FABRIC_POWER_DOWN_2_LANE_SEQ */
    {NULL,                              MV_SEQ_SIZE(0)},                                   /* MMPCS_MODE_1_LANE_25G_SEQ        */
    {NULL,                              MV_SEQ_SIZE(0)},                                   /* MMPCS_MODE_4_LANE_50G_SEQ        */
    {NULL,                              MV_SEQ_SIZE(0)},                                   /* MMPCS_RESET_1_LANE_25G_SEQ       */
    {NULL,                              MV_SEQ_SIZE(0)},                                   /* MMPCS_RESET_4_LANE_50G_SEQ       */
    {NULL,                              MV_SEQ_SIZE(0)},                                   /* MMPCS_POWER_DOWN_1_LANE_25G_SEQ  */
    {NULL,                              MV_SEQ_SIZE(0)},                                   /* MMPCS_POWER_DOWN_4_LANE_50G_SEQ  */
    {NULL,                              MV_SEQ_SIZE(0)},                                   /* MMPCS_UNRESET_1_LANE_25G_SEQ     */
#endif
    {mmpcsMultiLane_StartSendFaultSeqParams,   MV_SEQ_SIZE(mmpcsMultiLane_StartSendFaultSeqParams)},   /* MMPCS_START_SEND_FAULT_MULTI_LANE_SEQ */
    {mmpcsMultiLane_StopSendFaultSeqParams,   MV_SEQ_SIZE(mmpcsMultiLane_StopSendFaultSeqParams)},   /* MMPCS_STOP_SEND_FAULT_MULTI_LANE_SEQ */
    {mmpcsMultiLane_StopSendFaultRsFecSeqParams,   MV_SEQ_SIZE(mmpcsMultiLane_StopSendFaultRsFecSeqParams)},   /* MMPCS_STOP_SEND_FAULT_MULTI_LANE_RS_FEC_SEQ */
#ifndef ALDRIN_DEV_SUPPORT
    {mmpcsLane1_26_7G_UnResetSeqParams,   MV_SEQ_SIZE(mmpcsLane1_26_7G_UnResetSeqParams)},      /* MMPCS_UNRESET_1_LANE_26_7G_SEQ     */
#else
    {NULL,                              MV_SEQ_SIZE(0)},                                    /* MMPCS_UNRESET_1_LANE_26_7G_SEQ     */
#endif
    {mmpcsPcsMode2Lane_0SeqParams,      MV_SEQ_SIZE(mmpcsPcsMode2Lane_0SeqParams)},         /* MMPCS_MODE_2_LANE_0_50G_NO_FEC_SEQ */
    {mmpcsPcsMode2Lane_2SeqParams,      MV_SEQ_SIZE(mmpcsPcsMode2Lane_2SeqParams)},         /* MMPCS_MODE_2_LANE_2_50G_NO_FEC_SEQ */
    {mmpcsPcsRsFec2Lane_0SeqParams,     MV_SEQ_SIZE(mmpcsPcsRsFec2Lane_0SeqParams)},        /* MMPCS_MODE_2_LANE_0_50G_RS_FEC_SEQ */
    {mmpcsPcsRsFec2Lane_2SeqParams,     MV_SEQ_SIZE(mmpcsPcsRsFec2Lane_2SeqParams)},        /* MMPCS_MODE_2_LANE_2_50G_RS_FEC_SEQ */
    {mmPcsPowerDown2Lane_0SeqParams,    MV_SEQ_SIZE(mmPcsPowerDown2Lane_0SeqParams)},       /* MMPCS_POWER_DOWN_2_LANE_0_50G_SEQ */
    {mmPcsPowerDown2Lane_2SeqParams,    MV_SEQ_SIZE(mmPcsPowerDown2Lane_2SeqParams)},       /* MMPCS_POWER_DOWN_2_LANE_2_50G_SEQ */
    {mmPcsReducedPowerDown2Lane_0SeqParams, MV_SEQ_SIZE(mmPcsReducedPowerDown2Lane_0SeqParams)}, /* MMPCS_REDUCED_POWER_DOWN_2_LANE_0_SEQ */
    {mmPcsReducedPowerDown2Lane_2SeqParams, MV_SEQ_SIZE(mmPcsReducedPowerDown2Lane_2SeqParams)}, /* MMPCS_REDUCED_POWER_DOWN_2_LANE_2_SEQ */
    {mmPcsReset2Lane_0SeqParams,        MV_SEQ_SIZE(mmPcsReset2Lane_0SeqParams)},           /* MMPCS_RESET_2_LANE_0_SEQ */
    {mmPcsReset2Lane_2SeqParams,        MV_SEQ_SIZE(mmPcsReset2Lane_2SeqParams)},           /* MMPCS_RESET_2_LANE_2_SEQ */
    {mmPcsUnreset2Lane_0SeqParams,      MV_SEQ_SIZE(mmPcsUnreset2Lane_0SeqParams)},         /* MMPCS_UNRESET_2_LANE_0_SEQ */
    {mmPcsUnreset2Lane_2SeqParams,      MV_SEQ_SIZE(mmPcsUnreset2Lane_2SeqParams)},         /* MMPCS_UNRESET_2_LANE_2_SEQ */
#ifndef MV_HWS_REDUCED_BUILD
    {mmpcsLane1UnResetUpdateSeqParams,  MV_SEQ_SIZE(mmpcsLane1UnResetUpdateSeqParams)},       /* MMPCS_UNRESET_1_LANE_UPDATE_SEQ        */
    {mmpcsLane2UnResetUpdateSeqParams,  MV_SEQ_SIZE(mmpcsLane2UnResetUpdateSeqParams)},       /* MMPCS_UNRESET_2_LANE_UPDATE_SEQ        */
    {mmpcsFabricLane2UnResetUpdateSeqParams,  MV_SEQ_SIZE(mmpcsFabricLane2UnResetUpdateSeqParams)},       /* MMPCS_UNRESET_2_LANE_UPDATE_SEQ        */
#endif

};

GT_STATUS hwsMMPsc28nmSeqInit(void)
{
    return GT_OK;
}

GT_STATUS hwsMMPcs28nmSeqGet(MV_HWS_MM_PCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
  if (seqType >= MV_MM_PCS_LAST_SEQ)
  {
      return GT_NO_SUCH;
  }

  if (hwsMmPsc28nmSeqDb[seqType].cfgSeq == NULL)
  {
      /* check prev version */
      return hwsMMPcsSeqGet(seqType, seqLine, lineNum);
  }

  *seqLine = hwsMmPsc28nmSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}

