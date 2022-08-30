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
* @file mvHwsCgPcs28nmDb.c
*
* @brief
*
* @version   7
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/cgPcs/mvHwsCgPcs28nmDb.h>

static const MV_OP_PARAMS cgPcsMode1Lane_0SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x4FFF,         0xFFFF},
    {CG_UNIT,           CG_CONV_MARKER0_VL0,                    0x2168c1,       0xFFFFFF},/* fix AM0 markers for 25G-RS-FEC to IEEE format */
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_0,               0x68c1,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_1,               0x3321,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_2,               0x973e,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_3,               0xccde,         0xFFFF},
    {CG_UNIT,           CG_CONTROL_2,                           0x1,            0x1},
    {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x10},
    {CG_UNIT,           CG_CONTROL_2,                           0x200,          0x200},
    {CG_UNIT,           CG_CONTROL_2,                           0x20000,        0x20000},
    {CG_UNIT,           CG_CONTROL_1,                           0x100,          0x100},
    {CG_UNIT,           CG_CONTROL_1,                           0x1000,         0x1000},
    {CG_UNIT,           CG_CONTROL_0,                           0x800000,       0x800000},
    {CG_UNIT,           CG_CONTROL_2,                           0x2000,         0x2000},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0,   0x33,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x4FFF,         0xFFFF},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x10,           0xF0}
};

static const MV_OP_PARAMS cgPcsMode1Lane_1SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x4FFF0000,     0xFFFF0000},
    {CG_UNIT,           CG_CONV_MARKER1_VL0,                    0x2168c1,       0xFFFFFF},/* fix AM0 markers for 25G-RS-FEC to IEEE format */
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_0,               0x68c1,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_1,               0x3321,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_2,               0x973e,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_3,               0xccde,         0xFFFF},
    {CG_UNIT,           CG_CONTROL_2,                           0x1,            0x1},
    {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x10},
    {CG_UNIT,           CG_CONTROL_2,                           0x400,          0x400},
    {CG_UNIT,           CG_CONTROL_2,                           0x40000,        0x40000},
    {CG_UNIT,           CG_CONTROL_1,                           0x200,          0x200},
    {CG_UNIT,           CG_CONTROL_1,                           0x2000,         0x2000},
    {CG_UNIT,           CG_CONTROL_0,                           0x800000,       0x800000},
    {CG_UNIT,           CG_CONTROL_2,                           0x4000,         0x4000},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1,   0x33,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x4FFF,         0xFFFF},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x10,           0xF0}
};

static const MV_OP_PARAMS cgPcsMode1Lane_2SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x4FFF,         0xFFFF},
    {CG_UNIT,           CG_CONV_MARKER2_VL0,                    0x2168c1,       0xFFFFFF},/* fix AM0 markers for 25G-RS-FEC to IEEE format */
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_0,               0x68c1,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_1,               0x3321,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_2,               0x973e,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_3,               0xccde,         0xFFFF},
    {CG_UNIT,           CG_CONTROL_2,                           0x2,            0x2},
    {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x10},
    {CG_UNIT,           CG_CONTROL_2,                           0x800,          0x800},
    {CG_UNIT,           CG_CONTROL_2,                           0x80000,        0x80000},
    {CG_UNIT,           CG_CONTROL_1,                           0x400,          0x400},
    {CG_UNIT,           CG_CONTROL_1,                           0x4000,         0x4000},
    {CG_UNIT,           CG_CONTROL_0,                           0x800000,       0x800000},
    {CG_UNIT,           CG_CONTROL_2,                           0x8000,         0x8000},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2,   0x33,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x4FFF,         0xFFFF},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x10,           0xF0}
};

static const MV_OP_PARAMS cgPcsMode1Lane_3SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x4FFF0000,     0xFFFF0000},
    {CG_UNIT,           CG_CONV_MARKER3_VL0,                    0x2168c1,       0xFFFFFF},/* fix AM0 markers for 25G-RS-FEC to IEEE format */
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_0,               0x68c1,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_1,               0x3321,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_2,               0x973e,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_3,               0xccde,         0xFFFF},
    {CG_UNIT,           CG_CONTROL_2,                           0x2,            0x2},
    {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x10},
    {CG_UNIT,           CG_CONTROL_2,                           0x1000,         0x1000},
    {CG_UNIT,           CG_CONTROL_2,                           0x100000,       0x100000},
    {CG_UNIT,           CG_CONTROL_1,                           0x800,          0x800},
    {CG_UNIT,           CG_CONTROL_1,                           0x8000,         0x8000},
    {CG_UNIT,           CG_CONTROL_0,                           0x800000,       0x800000},
    {CG_UNIT,           CG_CONTROL_2,                           0x10000,        0x10000},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3,   0x33,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x4FFF,         0xFFFF},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x10,           0xF0}
};

static const MV_OP_PARAMS cgPcsMode4LaneSeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_CONTROL_0,                           0xC10000,       0xC10000},
    {CG_UNIT,           CG_CONTROL_1,                           0xFF00,         0xFF00},
    {CG_UNIT,           CG_CONTROL_2,                           0x1E010,        0x1FFE10},
    /* BC3 RM-8564354 - RxFifo Overrun ends with packets corruption entering the Chip Pipe. Need to configure the CG convertor RXFiFo Thd to 26 (0x1A) */
    {CG_UNIT,           CG_DMA_FIFO_CONFIG,                     0x1A,           0x3F},
    {CGPCS_UNIT,        CG_VIRTUAL_LANE_INTERVAL,               0x3FFF,         0xFFFF}
};

#if !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BC2_DEV_SUPPORT) && !defined(AC5_DEV_SUPPORT) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
static const MV_OP_PARAMS cgPcsMode4Lane_MLG_SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000},
    {CG_UNIT,           CG_CONTROL_2,                           0x1E50,         0x1E50},
    {CG_UNIT,           CG_CONTROL_1,                           0xFF00,         0xFF00},
    {CG_UNIT,           CG_CONTROL_0,                           0x800000,       0x800000},
    {CG_UNIT,           CG_CONTROL_2,                           0x1E000,        0x1E000},
    {CGPCS_UNIT,        CG_VL0_0,                               0x4089,         0xFFFF},
    {CGPCS_UNIT,        CG_VL0_1,                               0x9F,           0xFF},
    {CGPCS_UNIT,        CG_VL1_0,                               0x8529,         0xFFFF},
    {CGPCS_UNIT,        CG_VL1_1,                               0x1D,           0xFF},
    {CGPCS_UNIT,        CG_VL2_0,                               0x39AA,         0xFFFF},
    {CGPCS_UNIT,        CG_VL2_1,                               0xE3,           0xFF},
    {CGPCS_UNIT,        CG_VL3_0,                               0x7EBF,         0xFFFF},
    {CGPCS_UNIT,        CG_VL3_1,                               0x4D,           0xFF},
    {CGPCS_UNIT,        CG_VL4_0,                               0x6B14,         0xFFFF},
    {CGPCS_UNIT,        CG_VL4_1,                               0xD7,           0xFF},
    {CGPCS_UNIT,        CG_VL5_0,                               0x8BEE,         0xFFFF},
    {CGPCS_UNIT,        CG_VL5_1,                               0xBA,           0xFF},
    {CGPCS_UNIT,        CG_VL6_0,                               0xDBE1,         0xFFFF},
    {CGPCS_UNIT,        CG_VL6_1,                               0x6C,           0xFF},
    {CGPCS_UNIT,        CG_VL7_0,                               0x2D0,          0xFFFF},
    {CGPCS_UNIT,        CG_VL7_1,                               0x39,           0xFF},
    {CGPCS_UNIT,        CG_VL8_0,                               0xB839,         0xFFFF},
    {CGPCS_UNIT,        CG_VL8_1,                               0x5C,           0xFF},
    {CGPCS_UNIT,        CG_VL9_0,                               0xD2A1,         0xFFFF},
    {CGPCS_UNIT,        CG_VL9_1,                               0xAB,           0xFF},
    {CGPCS_UNIT,        CG_VL10_0,                              0x594A,         0xFFFF},
    {CGPCS_UNIT,        CG_VL10_1,                              0x12,           0xFF},
    {CGPCS_UNIT,        CG_VL11_0,                              0x7898,         0xFFFF},
    {CGPCS_UNIT,        CG_VL11_1,                              0x7,            0xFF},
    {CGPCS_UNIT,        CG_VL12_0,                              0xD355,         0xFFFF},
    {CGPCS_UNIT,        CG_VL12_1,                              0xC6,           0xFF},
    {CGPCS_UNIT,        CG_VL13_0,                              0x9031,         0xFFFF},
    {CGPCS_UNIT,        CG_VL13_1,                              0xC3,           0xFF},
    {CGPCS_UNIT,        CG_VL14_0,                              0xA2B6,         0xFFFF},
    {CGPCS_UNIT,        CG_VL14_1,                              0xCF,           0xFF},
    {CGPCS_UNIT,        CG_VL15_0,                              0x89F,          0xFFFF},
    {CGPCS_UNIT,        CG_VL15_1,                              0xB6,           0xFF},
    {CGPCS_UNIT,        CG_VL16_0,                              0x55BB,         0xFFFF},
    {CGPCS_UNIT,        CG_VL16_1,                              0x9D,           0xFF},
    {CGPCS_UNIT,        CG_VL17_0,                              0x5A8,          0xFFFF},
    {CGPCS_UNIT,        CG_VL17_1,                              0xFC,           0xFF},
    {CGPCS_UNIT,        CG_VL18_0,                              0xA104,         0xFFFF},
    {CGPCS_UNIT,        CG_VL18_1,                              0x94,           0xFF},
    {CGPCS_UNIT,        CG_VL19_0,                              0x7207,         0xFFFF},
    {CGPCS_UNIT,        CG_VL19_1,                              0xDB,           0xFF},
    {MMPCS_UNIT,        PCS40G_COMMON_CONTROL,                  0x200,          0x300},
    {MMPCS_UNIT,        PCS40G_COMMON_CONTROL + 0x2000,         0x200,          0x300},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x10,           0xF0},
    {MMPCS_UNIT,        PCS_RESET_REG + 0x1000,                 0x10,           0xF0},
    {MMPCS_UNIT,        PCS_RESET_REG + 0x2000,                 0x10,           0xF0},
    {MMPCS_UNIT,        PCS_RESET_REG + 0x3000,                 0x10,           0xF0},
    {MMPCS_UNIT,        CHANNEL_CONFIGURATION,                  0x000,          0x300},
    {MMPCS_UNIT,        CHANNEL_CONFIGURATION + 0x1000,         0x000,          0x300},
    {MMPCS_UNIT,        CHANNEL_CONFIGURATION + 0x2000,         0x200,          0x300},
    {MMPCS_UNIT,        CHANNEL_CONFIGURATION + 0x3000,         0x000,          0x300}
};
#endif

static const MV_OP_PARAMS cgPcsRsFec4LaneSeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_CONTROL_2,                           0x1E0000,       0x1E0000},
    /* BC3 RM-8564354 - RxFifo Overrun ends with packets corruption entering the Chip Pipe. Need to configure the CG convertor RXFiFo Thd to 26 (0x1A) */
    {CG_UNIT,           CG_DMA_FIFO_CONFIG,                     0x1A,           0x3F},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0,   0x0,            0x3F},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1,   0x0,            0x3F},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2,   0x0,            0x3F},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3,   0x0,            0x3F}
};

static const MV_OP_PARAMS cgPcsReset1Lane_0SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0x1},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x10},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x1000},
  /*  {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000},*/
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}
};

static const MV_OP_PARAMS cgPcsReset1Lane_1SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0x2},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x20},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x2000},
 /*   {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000},*/
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}
};

static const MV_OP_PARAMS cgPcsReset1Lane_2SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x40},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4000},
  /*  {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000},*/
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}
};

static const MV_OP_PARAMS cgPcsReset1Lane_3SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0x8},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x80},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x8000},
  /*  {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000},*/
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}
};

static const MV_OP_PARAMS cgPcsReset4LaneSeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0xFBFFFFFF}
};

#if !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BC2_DEV_SUPPORT) && !defined(AC5_DEV_SUPPORT) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
static const MV_OP_PARAMS cgPcsReset4Lane_MLG_SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0xFBFFFFFF},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x810,          0xFFF7},
    {MMPCS_UNIT,        PCS_RESET_REG + 0x1000,                 0x810,          0xFFF7},
    {MMPCS_UNIT,        PCS_RESET_REG + 0x2000,                 0x810,          0xFFF7},
    {MMPCS_UNIT,        PCS_RESET_REG + 0x3000,                 0x810,          0xFFF7}
};
#endif

static const MV_OP_PARAMS cgPcsPowerDown1Lane_0SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0x1},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x10},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x1000},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4},
    {CG_UNIT,           CG_CONTROL_0,                           0x0,            0x800000},
    {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x1100},
    {CG_UNIT,           CG_CONTROL_2,                           0x00010,        0x22211},
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x3FFF,         0xFFFF},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0,   0x2A,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF},
    /*reset Lane 0 align markers*/
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_0,               0x7690,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_1,               0x3e47,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_2,               0x896f,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_3,               0xc1b8,         0xFFFF},
    {CG_UNIT,           CG_CONV_MARKER0_VL0,                    0x477690,       0xFFFFFF},/* restore AM0 markers from 25G-RS-FEC to IEEE format */
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}/*,
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4000000}*/
};

static const MV_OP_PARAMS cgPcsPowerDown1Lane_1SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0x2},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x20},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x2000},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4},
    {CG_UNIT,           CG_CONTROL_0,                           0x0,            0x800000},
    {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x2200},
    {CG_UNIT,           CG_CONTROL_2,                           0x00010,        0x44411},
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x3FFF0000,     0xFFFF0000},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1,   0x2A,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF},
    /*reset Lane 0 align markers*/
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_0,               0x7690,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_1,               0x3e47,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_2,               0x896f,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_3,               0xc1b8,         0xFFFF},
    {CG_UNIT,           CG_CONV_MARKER1_VL0,                    0x477690,       0xFFFFFF},/* restore AM0 markers from 25G-RS-FEC to IEEE format */
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}/*,
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4000000}*/
};

static const MV_OP_PARAMS cgPcsPowerDown1Lane_2SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x40},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4000},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4},
    {CG_UNIT,           CG_CONTROL_0,                           0x0,            0x800000},
    {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x4400},
    {CG_UNIT,           CG_CONTROL_2,                           0x00010,        0x88812},
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x3FFF,         0xFFFF},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2,   0x2A,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF},
    /*reset Lane 0 align markers*/
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_0,               0x7690,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_1,               0x3e47,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_2,               0x896f,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_3,               0xc1b8,         0xFFFF},
    {CG_UNIT,           CG_CONV_MARKER2_VL0,                    0x477690,       0xFFFFFF},/* restore AM0 markers from 25G-RS-FEC to IEEE format */
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}/*,
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4000000}*/
};

static const MV_OP_PARAMS cgPcsPowerDown1Lane_3SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0x8},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x80},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x8000},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4},
    {CG_UNIT,           CG_CONTROL_0,                           0x0,            0x800000},
    {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x8800},
    {CG_UNIT,           CG_CONTROL_2,                           0x00010,        0x111012},
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x3FFF0000,     0xFFFF0000},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3,   0x2A,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF},
    /*reset Lane 0 align markers*/
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_0,               0x7690,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_1,               0x3e47,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_2,               0x896f,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_3,               0xc1b8,         0xFFFF},
    {CG_UNIT,           CG_CONV_MARKER3_VL0,                    0x477690,       0xFFFFFF},/* restore AM0 markers from 25G-RS-FEC to IEEE format */
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}/*,
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4000000}*/
};

static const MV_OP_PARAMS cgPcsReducedPowerDown1Lane_0SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0x1},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x10},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x1000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4},
    {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x1100},
    {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x22201},
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x3FFF,         0xFFFF},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0,   0x2A,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF},
    /*reset Lane 0 align markers*/
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_0,               0x7690,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_1,               0x3e47,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_2,               0x896f,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_3,               0xc1b8,         0xFFFF},
    {CG_UNIT,           CG_CONV_MARKER0_VL0,                    0x477690,       0xFFFFFF},/* restore AM0 markers from 25G-RS-FEC to IEEE format */
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}
};

static const MV_OP_PARAMS cgPcsReducedPowerDown1Lane_1SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0x2},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x20},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x2000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4},
    {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x2200},
    {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x44400},
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x3FFF0000,     0xFFFF0000},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1,   0x2A,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF},
    /*reset Lane 0 align markers*/
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_0,               0x7690,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_1,               0x3e47,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_2,               0x896f,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_3,               0xc1b8,         0xFFFF},
    {CG_UNIT,           CG_CONV_MARKER1_VL0,                    0x477690,       0xFFFFFF},/* restore AM0 markers from 25G-RS-FEC to IEEE format */
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}
};

static const MV_OP_PARAMS cgPcsReducedPowerDown1Lane_2SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x40},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4},
    {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x4400},
    {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x88802},
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x3FFF,         0xFFFF},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2,   0x2A,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF},
    /*reset Lane 0 align markers*/
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_0,               0x7690,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_1,               0x3e47,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_2,               0x896f,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_3,               0xc1b8,         0xFFFF},
    {CG_UNIT,           CG_CONV_MARKER2_VL0,                    0x477690,       0xFFFFFF},/* restore AM0 markers from 25G-RS-FEC to IEEE format */
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}
};

static const MV_OP_PARAMS cgPcsReducedPowerDown1Lane_3SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0x8},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x80},
    {CG_UNIT,           CG_RESETS,                              0x0,            0x8000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4},
    {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x8800},
    {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x111000},
    {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x3FFF0000,     0xFFFF0000},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3,   0x2A,           0x3F},
    {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF},
    /*reset Lane 0 align markers*/
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_0,               0x7690,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_1,               0x3e47,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_2,               0x896f,         0xFFFF},
    {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_3,               0xc1b8,         0xFFFF},
    {CG_UNIT,           CG_CONV_MARKER3_VL0,                    0x477690,       0xFFFFFF},/* restore AM0 markers from 25G-RS-FEC to IEEE format */
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}
};

static const MV_OP_PARAMS cgPcsPowerDown4LaneSeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0xFBFFFFFF},
    {CG_UNIT,           CG_CONTROL_0,                           0x0,            0xC10000},
    {CG_UNIT,           CG_CONTROL_1,                           0x0,            0xFF00},
    {CG_UNIT,           CG_CONTROL_2,                           0x10,           0x1FFE10},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0,   0x2A,           0x3F},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1,   0x2A,           0x3F},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2,   0x2A,           0x3F},
    {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3,   0x2A,           0x3F}/*,
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4000000}*/
};

#if !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BC2_DEV_SUPPORT) && !defined(AC5_DEV_SUPPORT) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
static const MV_OP_PARAMS cgPcsPowerDown4_MLG_LaneSeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,           CG_RESETS,                              0x0,            0xFBFFFFFF},
    {CG_UNIT,           CG_CONTROL_0,                           0x10020018,     0xFFFFFFFF},
    {CG_UNIT,           CG_CONTROL_1,                           0x3F,           0xFFFF},
    {CG_UNIT,           CG_CONTROL_2,                           0x190,          0xFFFFFFFF},
    {CGPCS_UNIT,        CG_VL0_0,                               0x68C1,         0xFFFF},
    {CGPCS_UNIT,        CG_VL0_1,                               0x21,           0xFF},
    {CGPCS_UNIT,        CG_VL1_0,                               0x719D,         0xFFFF},
    {CGPCS_UNIT,        CG_VL1_1,                               0x8E,           0xFF},
    {CGPCS_UNIT,        CG_VL2_0,                               0x4B59,         0xFFFF},
    {CGPCS_UNIT,        CG_VL2_1,                               0xE8,           0xFF},
    {CGPCS_UNIT,        CG_VL3_0,                               0x954D,         0xFFFF},
    {CGPCS_UNIT,        CG_VL3_1,                               0x7B,           0xFF},
    {CGPCS_UNIT,        CG_VL4_0,                               0x7F5,          0xFFFF},
    {CGPCS_UNIT,        CG_VL4_1,                               0x9,            0xFF},
    {CGPCS_UNIT,        CG_VL5_0,                               0x14DD,         0xFFFF},
    {CGPCS_UNIT,        CG_VL5_1,                               0xC2,           0xFF},
    {CGPCS_UNIT,        CG_VL6_0,                               0x4A9A,         0xFFFF},
    {CGPCS_UNIT,        CG_VL6_1,                               0x26,           0xFF},
    {CGPCS_UNIT,        CG_VL7_0,                               0x457B,         0xFFFF},
    {CGPCS_UNIT,        CG_VL7_1,                               0x66,           0xFF},
    {CGPCS_UNIT,        CG_VL8_0,                               0x24A0,         0xFFFF},
    {CGPCS_UNIT,        CG_VL8_1,                               0x76,           0xFF},
    {CGPCS_UNIT,        CG_VL9_0,                               0xC968,         0xFFFF},
    {CGPCS_UNIT,        CG_VL9_1,                               0xFB,           0xFF},
    {CGPCS_UNIT,        CG_VL10_0,                              0x6CFD,         0xFFFF},
    {CGPCS_UNIT,        CG_VL10_1,                              0x99,           0xFF},
    {CGPCS_UNIT,        CG_VL11_0,                              0x91B9,         0xFFFF},
    {CGPCS_UNIT,        CG_VL11_1,                              0x55,           0xFF},
    {CGPCS_UNIT,        CG_VL12_0,                              0xB95C,         0xFFFF},
    {CGPCS_UNIT,        CG_VL12_1,                              0xB2,           0xFF},
    {CGPCS_UNIT,        CG_VL13_0,                              0xF81A,         0xFFFF},
    {CGPCS_UNIT,        CG_VL13_1,                              0xBD,           0xFF},
    {CGPCS_UNIT,        CG_VL14_0,                              0xC783,         0xFFFF},
    {CGPCS_UNIT,        CG_VL14_1,                              0xCA,           0xFF},
    {CGPCS_UNIT,        CG_VL15_0,                              0x3635,         0xFFFF},
    {CGPCS_UNIT,        CG_VL15_1,                              0xCD,           0xFF},
    {CGPCS_UNIT,        CG_VL16_0,                              0x31C4,         0xFFFF},
    {CGPCS_UNIT,        CG_VL16_1,                              0x4C,           0xFF},
    {CGPCS_UNIT,        CG_VL17_0,                              0xD6AD,         0xFFFF},
    {CGPCS_UNIT,        CG_VL17_1,                              0xB7,           0xFF},
    {CGPCS_UNIT,        CG_VL18_0,                              0x665F,         0xFFFF},
    {CGPCS_UNIT,        CG_VL18_1,                              0x2A,           0xFF},
    {CGPCS_UNIT,        CG_VL19_0,                              0xF0C0,         0xFFFF},
    {CGPCS_UNIT,        CG_VL19_1,                              0xE5,           0xFF},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x810,          0xFFF7},
    {MMPCS_UNIT,        PCS_RESET_REG + 0x1000,                 0x810,          0xFFF7},
    {MMPCS_UNIT,        PCS_RESET_REG + 0x2000,                 0x810,          0xFFF7},
    {MMPCS_UNIT,        PCS_RESET_REG + 0x3000,                 0x810,          0xFFF7},
    {MMPCS_UNIT,        PCS40G_COMMON_CONTROL,                  0x0,            0x300},
    {MMPCS_UNIT,        PCS40G_COMMON_CONTROL + 0x2000,         0x0,            0x300},
    {MMPCS_UNIT,        CHANNEL_CONFIGURATION,                  0x0,            0x300},
    {MMPCS_UNIT,        CHANNEL_CONFIGURATION + 0x1000,         0x0,            0x300},
    {MMPCS_UNIT,        CHANNEL_CONFIGURATION + 0x2000,         0x0,            0x300},
    {MMPCS_UNIT,        CHANNEL_CONFIGURATION + 0x3000,         0x0,            0x300}/*,
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4000000}*/
};
#endif

static const MV_OP_PARAMS cgPcsUnreset1Lane_0SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x1,            0x1},
    {CG_UNIT,           CG_RESETS,                              0x1000,         0x1000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x4,            0x4},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x800,          0x800},
    {CG_UNIT,           CG_RESETS,                              0x10000000,     0x10000000},
    {CG_UNIT,           CG_RESETS,                              0x10,           0x10}
};

static const MV_OP_PARAMS cgPcsUnreset1Lane_1SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x2,            0x2},
    {CG_UNIT,           CG_RESETS,                              0x2000,         0x2000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x4,            0x4},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x800,          0x800},
    {CG_UNIT,           CG_RESETS,                              0x10000000,     0x10000000},
    {CG_UNIT,           CG_RESETS,                              0x20,           0x20}
};

static const MV_OP_PARAMS cgPcsUnreset1Lane_2SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x4,            0x4},
    {CG_UNIT,           CG_RESETS,                              0x4000,         0x4000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x4,            0x4},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x800,          0x800},
    {CG_UNIT,           CG_RESETS,                              0x10000000,     0x10000000},
    {CG_UNIT,           CG_RESETS,                              0x40,           0x40}
};

static const MV_OP_PARAMS cgPcsUnreset1Lane_3SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0x8,            0x8},
    {CG_UNIT,           CG_RESETS,                              0x8000,         0x8000},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x4,            0x4},
    {MMPCS_UNIT,        PCS_RESET_REG,                          0x800,          0x800},
    {CG_UNIT,           CG_RESETS,                              0x10000000,     0x10000000},
    {CG_UNIT,           CG_RESETS,                              0x80,           0x80}
};

#ifdef MV_HWS_REDUCED_BUILD
#ifdef ALDRIN2_DEV_SUPPORT
static const MV_OP_PARAMS cgPcsUnreset4LaneSeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0xF,            0xF},
    {CG_UNIT,           CG_RESETS,                              0xF0,           0xF0},
    {CG_UNIT,           CG_RESETS,                              0xF000,         0xF000},
    {CG_UNIT,           CG_RESETS,                              0x7b000000,     0x7b000000}
};
#else
static const MV_OP_PARAMS cgPcsUnreset4LaneSeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0xF,            0xF},
    {CG_UNIT,           CG_RESETS,                              0xF0,           0xF0},
    {CG_UNIT,           CG_RESETS,                              0xF000,         0xF000},
    {CG_UNIT,           CG_RESETS,                              0x3b000000,     0x3b000000}
};
#endif
#else
static const MV_OP_PARAMS cgPcsUnreset4LaneSeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0xF,            0xF},
    {CG_UNIT,           CG_RESETS,                              0xF0,           0xF0},
    {CG_UNIT,           CG_RESETS,                              0xF000,         0xF000},
    {CG_UNIT,           CG_RESETS,                              0x3b000000,     0x3b000000}
};
static const MV_OP_PARAMS cgPcsUnreset4LaneUpdateSeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0xF,            0xF},
    {CG_UNIT,           CG_RESETS,                              0xF0,           0xF0},
    {CG_UNIT,           CG_RESETS,                              0xF000,         0xF000},
    {CG_UNIT,           CG_RESETS,                              0x7b000000,     0x7b000000}
};
#endif

#if !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BC2_DEV_SUPPORT) && !defined(AC5_DEV_SUPPORT) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    /* in 100G_MLG CG modes: 4 CG lanes are bounded */
static const MV_OP_PARAMS cgPcsUnreset4Lane_MLG_SeqParams[] = {
    {CG_UNIT,           CG_RESETS,                              0xF,            0xF},
    {CG_UNIT,           CG_RESETS,                              0xF000,         0xF000},
    {CG_UNIT,           CG_RESETS,                              0x33000000,     0x33000000},
    {MMPCS_UNIT, PCS_RESET_REG,                                 0x1,            0x1},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                        0x1,            0x1},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                        0x1,            0x1},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                        0x1,            0x1},
    {MMPCS_UNIT, PCS_RESET_REG,                                 0x4,            0x4},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                        0x4,            0x4},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                        0x4,            0x4},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                        0x4,            0x4},
    {MMPCS_UNIT, PCS_RESET_REG,                                 0x2,            0x2},
    {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                        0x2,            0x2},
    {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                        0x2,            0x2},
    {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                        0x2,            0x2},
    {CG_UNIT,           CG_RESETS,                              0xF0,           0xF0}
};
#endif

static const MV_OP_PARAMS lbNormalSeqParams[] = {
    {CGPCS_UNIT, CGPCS_CONTROL_1, 0x0,       0x4000}
};

static const MV_OP_PARAMS lbTx2RxSeqParams[] = {
    {CGPCS_UNIT, CGPCS_CONTROL_1, 0x4000,    0x4000}
};

static const MV_OP_PARAMS cgPcs_StartSendFaultSeqParams[] = {
    {CGPCS_UNIT, CG_VL0_0,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL0_1,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL1_0,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL1_1,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL2_0,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL2_1,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL3_0,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL3_1,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL4_0,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL4_1,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL5_0,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL5_1,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL6_0,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL6_1,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL7_0,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL7_1,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL8_0,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL8_1,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL9_0,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL9_1,         0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL10_0,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL10_1,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL11_0,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL11_1,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL12_0,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL12_1,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL13_0,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL13_1,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL14_0,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL14_1,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL15_0,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL15_1,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL16_0,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL16_1,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL17_0,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL17_1,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL18_0,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL18_1,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL19_0,        0x0,  0xFFFF},
    {CGPCS_UNIT, CG_VL19_1,        0x0,  0xFFFF},
};

static const MV_OP_PARAMS cgPcs_StopSendFaultSeqParams[] = {
    {CGPCS_UNIT, CG_VL0_0,      0x68C1,   0xFFFF},
    {CGPCS_UNIT, CG_VL0_1,      0x21,     0xFFFF},
    {CGPCS_UNIT, CG_VL1_0,      0x719D,   0xFFFF},
    {CGPCS_UNIT, CG_VL1_1,      0x8E,     0xFFFF},
    {CGPCS_UNIT, CG_VL2_0,      0x4B59,   0xFFFF},
    {CGPCS_UNIT, CG_VL2_1,      0xE8,     0xFFFF},
    {CGPCS_UNIT, CG_VL3_0,      0x954D,   0xFFFF},
    {CGPCS_UNIT, CG_VL3_1,      0x7B,     0xFFFF},
    {CGPCS_UNIT, CG_VL4_0,      0x7F5,    0xFFFF},
    {CGPCS_UNIT, CG_VL4_1,      0x9,      0xFFFF},
    {CGPCS_UNIT, CG_VL5_0,      0x14DD,   0xFFFF},
    {CGPCS_UNIT, CG_VL5_1,      0xC2,     0xFFFF},
    {CGPCS_UNIT, CG_VL6_0,      0x4A9A,   0xFFFF},
    {CGPCS_UNIT, CG_VL6_1,      0x26,     0xFFFF},
    {CGPCS_UNIT, CG_VL7_0,      0x457B,   0xFFFF},
    {CGPCS_UNIT, CG_VL7_1,      0x66,     0xFFFF},
    {CGPCS_UNIT, CG_VL8_0,      0x24A0,   0xFFFF},
    {CGPCS_UNIT, CG_VL8_1,      0x76,     0xFFFF},
    {CGPCS_UNIT, CG_VL9_0,      0xC968,   0xFFFF},
    {CGPCS_UNIT, CG_VL9_1,      0xFB,     0xFFFF},
    {CGPCS_UNIT, CG_VL10_0,     0x6CFD,   0xFFFF},
    {CGPCS_UNIT, CG_VL10_1,     0x99,     0xFFFF},
    {CGPCS_UNIT, CG_VL11_0,     0x91B9,   0xFFFF},
    {CGPCS_UNIT, CG_VL11_1,     0x55,     0xFFFF},
    {CGPCS_UNIT, CG_VL12_0,     0xB95C,   0xFFFF},
    {CGPCS_UNIT, CG_VL12_1,     0xB2,     0xFFFF},
    {CGPCS_UNIT, CG_VL13_0,     0xF81A,   0xFFFF},
    {CGPCS_UNIT, CG_VL13_1,     0xBD,     0xFFFF},
    {CGPCS_UNIT, CG_VL14_0,     0xC783,   0xFFFF},
    {CGPCS_UNIT, CG_VL14_1,     0xCA,     0xFFFF},
    {CGPCS_UNIT, CG_VL15_0,     0x3635,   0xFFFF},
    {CGPCS_UNIT, CG_VL15_1,     0xCD,     0xFFFF},
    {CGPCS_UNIT, CG_VL16_0,     0x31C4,   0xFFFF},
    {CGPCS_UNIT, CG_VL16_1,     0x4C,     0xFFFF},
    {CGPCS_UNIT, CG_VL17_0,     0xD6AD,   0xFFFF},
    {CGPCS_UNIT, CG_VL17_1,     0xB7,     0xFFFF},
    {CGPCS_UNIT, CG_VL18_0,     0x665F,   0xFFFF},
    {CGPCS_UNIT, CG_VL18_1,     0x2A,     0xFFFF},
    {CGPCS_UNIT, CG_VL19_0,     0xF0C0,   0xFFFF},
    {CGPCS_UNIT, CG_VL19_1,     0xE5,     0xFFFF},
};

/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsCgPcs28nmSeqDb[] =
{
    {NULL,                              MV_SEQ_SIZE(0)},                    /* CGPCS_RESET_SEQ        */
    {NULL,                              MV_SEQ_SIZE(0)},                    /* CGPCS_UNRESET_SEQ      */
    {NULL,                              MV_SEQ_SIZE(0)},                    /* CGPCS_MODE_MISC_SEQ    */
    {NULL,                              MV_SEQ_SIZE(0)},                    /* CGPCS_MODE_10_LANE_SEQ */
    {NULL,                              MV_SEQ_SIZE(0)},                    /* CGPCS_MODE_12_LANE_SEQ */
    {lbNormalSeqParams,                 MV_SEQ_SIZE(lbNormalSeqParams)},    /* CGPCS_LPBK_NORMAL_SEQ  */
    {lbTx2RxSeqParams,                  MV_SEQ_SIZE(lbTx2RxSeqParams)},     /* CGPCS_LPBK_TX2RX_SEQ   */

    {cgPcsMode1Lane_0SeqParams,         MV_SEQ_SIZE(cgPcsMode1Lane_0SeqParams)},       /* CGPCS_MODE_1_LANE_0_SEQ       */
    {cgPcsMode1Lane_1SeqParams,         MV_SEQ_SIZE(cgPcsMode1Lane_1SeqParams)},       /* CGPCS_MODE_1_LANE_1_SEQ       */
    {cgPcsMode1Lane_2SeqParams,         MV_SEQ_SIZE(cgPcsMode1Lane_2SeqParams)},       /* CGPCS_MODE_1_LANE_2_SEQ       */
    {cgPcsMode1Lane_3SeqParams,         MV_SEQ_SIZE(cgPcsMode1Lane_3SeqParams)},       /* CGPCS_MODE_1_LANE_3_SEQ       */
    {cgPcsMode4LaneSeqParams,           MV_SEQ_SIZE(cgPcsMode4LaneSeqParams)},         /* CGPCS_MODE_4_LANE_SEQ         */
    {cgPcsRsFec4LaneSeqParams,          MV_SEQ_SIZE(cgPcsRsFec4LaneSeqParams)},        /* CGPCS_RS_FEC_4_LANE_SEQ       */
    {cgPcsReset1Lane_0SeqParams,        MV_SEQ_SIZE(cgPcsReset1Lane_0SeqParams)},      /* CGPCS_RESET_1_LANE_0_SEQ      */
    {cgPcsReset1Lane_1SeqParams,        MV_SEQ_SIZE(cgPcsReset1Lane_1SeqParams)},      /* CGPCS_RESET_1_LANE_1_SEQ      */
    {cgPcsReset1Lane_2SeqParams,        MV_SEQ_SIZE(cgPcsReset1Lane_2SeqParams)},      /* CGPCS_RESET_1_LANE_2_SEQ      */
    {cgPcsReset1Lane_3SeqParams,        MV_SEQ_SIZE(cgPcsReset1Lane_3SeqParams)},      /* CGPCS_RESET_1_LANE_3_SEQ      */
    {cgPcsReset4LaneSeqParams,          MV_SEQ_SIZE(cgPcsReset4LaneSeqParams)},        /* CGPCS_RESET_4_LANE_SEQ        */
    {cgPcsUnreset1Lane_0SeqParams,      MV_SEQ_SIZE(cgPcsUnreset1Lane_0SeqParams)},    /* CGPCS_UNRESET_1_LANE_0_SEQ    */
    {cgPcsUnreset1Lane_1SeqParams,      MV_SEQ_SIZE(cgPcsUnreset1Lane_1SeqParams)},    /* CGPCS_UNRESET_1_LANE_1_SEQ    */
    {cgPcsUnreset1Lane_2SeqParams,      MV_SEQ_SIZE(cgPcsUnreset1Lane_2SeqParams)},    /* CGPCS_UNRESET_1_LANE_2_SEQ    */
    {cgPcsUnreset1Lane_3SeqParams,      MV_SEQ_SIZE(cgPcsUnreset1Lane_3SeqParams)},    /* CGPCS_UNRESET_1_LANE_3_SEQ    */
    {cgPcsUnreset4LaneSeqParams,        MV_SEQ_SIZE(cgPcsUnreset4LaneSeqParams)},      /* CGPCS_UNRESET_4_LANE_SEQ      */
    {cgPcsPowerDown1Lane_0SeqParams,    MV_SEQ_SIZE(cgPcsPowerDown1Lane_0SeqParams)},  /* CGPCS_POWER_DOWN_1_LANE_0_SEQ */
    {cgPcsPowerDown1Lane_1SeqParams,    MV_SEQ_SIZE(cgPcsPowerDown1Lane_1SeqParams)},  /* CGPCS_POWER_DOWN_1_LANE_1_SEQ */
    {cgPcsPowerDown1Lane_2SeqParams,    MV_SEQ_SIZE(cgPcsPowerDown1Lane_2SeqParams)},  /* CGPCS_POWER_DOWN_1_LANE_2_SEQ */
    {cgPcsPowerDown1Lane_3SeqParams,    MV_SEQ_SIZE(cgPcsPowerDown1Lane_3SeqParams)},  /* CGPCS_POWER_DOWN_1_LANE_3_SEQ */
    {cgPcsPowerDown4LaneSeqParams,      MV_SEQ_SIZE(cgPcsPowerDown4LaneSeqParams)},    /* CGPCS_POWER_DOWN_4_LANE_SEQ   */
    {cgPcsReducedPowerDown1Lane_0SeqParams,   MV_SEQ_SIZE(cgPcsReducedPowerDown1Lane_0SeqParams)}, /* CGPCS_REDUCED_POWER_DOWN_1_LANE_0_SEQ */
    {cgPcsReducedPowerDown1Lane_1SeqParams,   MV_SEQ_SIZE(cgPcsReducedPowerDown1Lane_1SeqParams)}, /* CGPCS_REDUCED_POWER_DOWN_1_LANE_1_SEQ */
    {cgPcsReducedPowerDown1Lane_2SeqParams,   MV_SEQ_SIZE(cgPcsReducedPowerDown1Lane_2SeqParams)}, /* CGPCS_REDUCED_POWER_DOWN_1_LANE_2_SEQ */
    {cgPcsReducedPowerDown1Lane_3SeqParams,   MV_SEQ_SIZE(cgPcsReducedPowerDown1Lane_3SeqParams)}, /* CGPCS_REDUCED_POWER_DOWN_1_LANE_3_SEQ */
#if !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BC2_DEV_SUPPORT) && !defined(AC5_DEV_SUPPORT) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    {cgPcsMode4Lane_MLG_SeqParams,      MV_SEQ_SIZE(cgPcsMode4Lane_MLG_SeqParams)},      /* CGPCS_MODE_4_LANE_MLG_SEQ       */
    {cgPcsReset4Lane_MLG_SeqParams,     MV_SEQ_SIZE(cgPcsReset4Lane_MLG_SeqParams)},     /* CGPCS_RESET_4_LANE_MLG_SEQ      */
    {cgPcsPowerDown4_MLG_LaneSeqParams, MV_SEQ_SIZE(cgPcsPowerDown4_MLG_LaneSeqParams)}, /* CGPCS_POWER_DOWN_4_LANE_MLG_SEQ */
    {cgPcsUnreset4Lane_MLG_SeqParams,   MV_SEQ_SIZE(cgPcsUnreset4Lane_MLG_SeqParams)},   /* CGPCS_UNRESET_4_LANE_MLG_SEQ    */
#endif
    {cgPcs_StartSendFaultSeqParams,     MV_SEQ_SIZE(cgPcs_StartSendFaultSeqParams)},    /* CGPCS_START_SEND_FAULT_SEQ */
    {cgPcs_StopSendFaultSeqParams,      MV_SEQ_SIZE(cgPcs_StopSendFaultSeqParams)},     /* CGPCS_STOP_SEND_FAULT_SEQ  */
#if !defined(MV_HWS_REDUCED_BUILD)
    {cgPcsUnreset4LaneUpdateSeqParams,  MV_SEQ_SIZE(cgPcsUnreset4LaneUpdateSeqParams)}  /* CGPCS_UNRESET_4_LANE_UPDATE_SEQ      */
#endif
};

GT_STATUS hwsCgPcs28nmSeqInit(GT_U8 devNum)
{
    devNum = devNum;
    hwsCgPcsPscSeqDb = &hwsCgPcs28nmSeqDb[0];
#if 0
    MV_OP_PARAMS* updateParams;

    if (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2)
    {
        updateParams = &hwsCgPcs28nmSeqDb[CGPCS_UNRESET_4_LANE_SEQ].cfgSeq[3];
        updateParams->operData = 0x7b000000;
        updateParams->mask = 0x7b000000;
    }
#endif
    return GT_OK;
}

GT_STATUS hwsCgPcs28nmSeqGet(MV_HWS_CGPCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MV_CGPCS_LAST_SEQ) ||
      (hwsCgPcs28nmSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsCgPcs28nmSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}
