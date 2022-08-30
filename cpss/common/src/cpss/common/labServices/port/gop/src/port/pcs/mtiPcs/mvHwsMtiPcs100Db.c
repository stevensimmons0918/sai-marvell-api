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
* @file mvHwsMtiPcs100Db.c
*
* @brief
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcsDb.h>

static const MV_OP_PARAMS mtiPcs100resetSeqParams[] = {
    {MTI_PCS100_UNIT, MTI_PCS_CONTROL1,     /*dummy write 0x8000*/0x0000,     0x8000}  /*CONTROL1.Reset.set(1)*/

};

static const MV_OP_PARAMS mtiPcs100unresetSeqParams[] = {
    {MTI_PCS100_UNIT, MTI_PCS_CONTROL1,            0x8000,     0x8000}  /*CONTROL1.Reset.set(1)*/
   /* {UNIT_MTI_PCS100, MTI_PCS_CONTROL1,            0x0,     0x8000} */ /*CONTROL1.Reset.set(0)*/

};

static const MV_OP_PARAMS mtiPcs100PowerDownSeqParams[] = {
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_VL_INTVL,     0x3FFF,     0xFFFF}, /*VENDOR_VL_INTVL.Marker_counter;*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x2,        0x2},    /*VENDOR_PCS_MODE.Disable_mld;*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x1,        0x1},    /*VENDOR_PCS_MODE.Ena_clause49;*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x4},    /*VENDOR_PCS_MODE.Hi_ber25.set(0);*/

    /* Marker values for 100G-KR4 / 100G-KR2 (no FEC dependency) */
    {MTI_PCS100_UNIT, MTI_PCS_VL0_0,               0x68c1,     0xFFFF}, /*VL0_0.Vl0_0.set(16'h68c1)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL0_1,               0x0021,     0xFFFF}, /*VL0_1.Vl0_1.set(8'h21)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL1_0,               0x719d,     0xFFFF}, /*VL1_0.Vl1_0.set(16'h719d)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL1_1,               0x008e,     0xFFFF}, /*VL1_1.Vl1_1.set(8'h8e)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL2_0,               0x4b59,     0xFFFF}, /*VL2_0.Vl2_0.set(16'h4b59)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL2_1,               0x00e8,     0xFFFF}, /*VL2_1.Vl2_1.set(8'he8)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL3_0,               0x954d,     0xFFFF}, /*VL3_0.Vl3_0.set(16'h954d)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL3_1,               0x007b,     0xFFFF}, /*VL3_1.Vl3_1.set(8'h7b)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL4_0,               0x07f5,     0xFFFF}, /*VL4_0.Vl4_0.set(16'h7f5)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL4_1,               0x0009,     0xFFFF}, /*VL4_1.Vl4_1.set(8'h9)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL5_0,               0x14dd,     0xFFFF}, /*VL5_0.Vl5_0.set(16'h14dd)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL5_1,               0x00c2,     0xFFFF}, /*VL5_1.Vl5_1.set(8'hc2)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL6_0,               0x4a9a,     0xFFFF}, /*VL6_0.Vl6_0.set(16'h4a9a)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL6_1,               0x0026,     0xFFFF}, /*VL6_1.Vl6_1.set(8'h26)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL7_0,               0x457b,     0xFFFF}, /*VL7_0.Vl7_0.set(16'h457b)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL7_1,               0x0066,     0xFFFF}, /*VL7_1.Vl7_1.set(8'h66)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL8_0,               0x24a0,     0xFFFF}, /*VL8_0.Vl8_0.set(16'h24a0)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL8_1,               0x0076,     0xFFFF}, /*VL8_1.Vl8_1.set(8'h76)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL9_0,               0xc968,     0xFFFF}, /*VL9_0.Vl9_0.set(16'hc968)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL9_1,               0x00fb,     0xFFFF}, /*VL9_1.Vl9_1.set(8'hfb)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL10_0,              0x6cfd,     0xFFFF}, /*VL10_0.Vl10_0.set(16'h6cfd)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL10_1,              0x0099,     0xFFFF}, /*VL10_1.Vl10_1.set(8'h99)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL11_0,              0x91b9,     0xFFFF}, /*VL11_0.Vl11_0.set(16'h91b9)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL11_1,              0x0055,     0xFFFF}, /*VL11_1.Vl11_1.set(8'h55)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL12_0,              0xb95c,     0xFFFF}, /*VL12_0.Vl12_0.set(16'h)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL12_1,              0x00b2,     0xFFFF}, /*VL12_1.Vl12_1.set(8'hb2)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL13_0,              0xf81a,     0xFFFF}, /*VL13_0.Vl13_0.set(16'hf81a)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL13_1,              0x00bd,     0xFFFF}, /*VL13_1.Vl13_1.set(8'hbd)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL14_0,              0xc783,     0xFFFF}, /*VL14_0.Vl14_0.set(16'hc783)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL14_1,              0x00ca,     0xFFFF}, /*VL14_1.Vl14_1.set(8'hca)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL15_0,              0x3635,     0xFFFF}, /*VL15_0.Vl15_0.set(16'h3635)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL15_1,              0x00cd,     0xFFFF}, /*VL15_1.Vl15_1.set(8'hcd)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL16_0,              0x31c4,     0xFFFF}, /*VL16_0.Vl16_0.set(16'h31c4)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL16_1,              0x004c,     0xFFFF}, /*VL16_1.Vl16_1.set(8'h4c)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL17_0,              0xd6ad,     0xFFFF}, /*VL17_0.Vl17_0.set(16'hd6ad)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL17_1,              0x00b7,     0xFFFF}, /*VL17_1.Vl17_1.set(8'hb7)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL18_0,              0x665f,     0xFFFF}, /*VL18_0.Vl18_0.set(16'h665f)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL18_1,              0x002a,     0xFFFF}, /*VL18_1.Vl18_1.set(8'h2a)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL19_0,              0xf0c0,     0xFFFF}, /*VL19_0.Vl19_0.set(16'hf0c0)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL19_1,              0x00e5,     0xFFFF}, /*VL19_1.Vl19_1.set(8'he5)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_CONTROL,           0x0,        0x200}, /*RSFEC_CONTROL.Tc_pad_value.set(1'b1)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_DEC_THRESH,        0x33,       0x3F}, /*RSFEC_CONTROL.Tc_pad_value.set(1'b1)*/

};

static const MV_OP_PARAMS mtiPcs100PowerDownNoRsFecSeqParams[] = {
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_VL_INTVL,     0x3FFF,     0xFFFF}, /*VENDOR_VL_INTVL.Marker_counter;*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x2,        0x2},    /*VENDOR_PCS_MODE.Disable_mld;*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x1,        0x1},    /*VENDOR_PCS_MODE.Ena_clause49;*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x4},    /*VENDOR_PCS_MODE.Hi_ber25.set(0);*/

    /* Marker values for 100G-KR4 / 100G-KR2 (no FEC dependency) */
    {MTI_PCS100_UNIT, MTI_PCS_VL0_0,               0x68c1,     0xFFFF}, /*VL0_0.Vl0_0.set(16'h68c1)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL0_1,               0x0021,     0xFFFF}, /*VL0_1.Vl0_1.set(8'h21)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL1_0,               0x719d,     0xFFFF}, /*VL1_0.Vl1_0.set(16'h719d)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL1_1,               0x008e,     0xFFFF}, /*VL1_1.Vl1_1.set(8'h8e)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL2_0,               0x4b59,     0xFFFF}, /*VL2_0.Vl2_0.set(16'h4b59)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL2_1,               0x00e8,     0xFFFF}, /*VL2_1.Vl2_1.set(8'he8)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL3_0,               0x954d,     0xFFFF}, /*VL3_0.Vl3_0.set(16'h954d)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL3_1,               0x007b,     0xFFFF}, /*VL3_1.Vl3_1.set(8'h7b)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL4_0,               0x07f5,     0xFFFF}, /*VL4_0.Vl4_0.set(16'h7f5)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL4_1,               0x0009,     0xFFFF}, /*VL4_1.Vl4_1.set(8'h9)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL5_0,               0x14dd,     0xFFFF}, /*VL5_0.Vl5_0.set(16'h14dd)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL5_1,               0x00c2,     0xFFFF}, /*VL5_1.Vl5_1.set(8'hc2)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL6_0,               0x4a9a,     0xFFFF}, /*VL6_0.Vl6_0.set(16'h4a9a)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL6_1,               0x0026,     0xFFFF}, /*VL6_1.Vl6_1.set(8'h26)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL7_0,               0x457b,     0xFFFF}, /*VL7_0.Vl7_0.set(16'h457b)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL7_1,               0x0066,     0xFFFF}, /*VL7_1.Vl7_1.set(8'h66)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL8_0,               0x24a0,     0xFFFF}, /*VL8_0.Vl8_0.set(16'h24a0)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL8_1,               0x0076,     0xFFFF}, /*VL8_1.Vl8_1.set(8'h76)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL9_0,               0xc968,     0xFFFF}, /*VL9_0.Vl9_0.set(16'hc968)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL9_1,               0x00fb,     0xFFFF}, /*VL9_1.Vl9_1.set(8'hfb)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL10_0,              0x6cfd,     0xFFFF}, /*VL10_0.Vl10_0.set(16'h6cfd)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL10_1,              0x0099,     0xFFFF}, /*VL10_1.Vl10_1.set(8'h99)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL11_0,              0x91b9,     0xFFFF}, /*VL11_0.Vl11_0.set(16'h91b9)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL11_1,              0x0055,     0xFFFF}, /*VL11_1.Vl11_1.set(8'h55)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL12_0,              0xb95c,     0xFFFF}, /*VL12_0.Vl12_0.set(16'h)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL12_1,              0x00b2,     0xFFFF}, /*VL12_1.Vl12_1.set(8'hb2)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL13_0,              0xf81a,     0xFFFF}, /*VL13_0.Vl13_0.set(16'hf81a)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL13_1,              0x00bd,     0xFFFF}, /*VL13_1.Vl13_1.set(8'hbd)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL14_0,              0xc783,     0xFFFF}, /*VL14_0.Vl14_0.set(16'hc783)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL14_1,              0x00ca,     0xFFFF}, /*VL14_1.Vl14_1.set(8'hca)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL15_0,              0x3635,     0xFFFF}, /*VL15_0.Vl15_0.set(16'h3635)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL15_1,              0x00cd,     0xFFFF}, /*VL15_1.Vl15_1.set(8'hcd)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL16_0,              0x31c4,     0xFFFF}, /*VL16_0.Vl16_0.set(16'h31c4)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL16_1,              0x004c,     0xFFFF}, /*VL16_1.Vl16_1.set(8'h4c)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL17_0,              0xd6ad,     0xFFFF}, /*VL17_0.Vl17_0.set(16'hd6ad)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL17_1,              0x00b7,     0xFFFF}, /*VL17_1.Vl17_1.set(8'hb7)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL18_0,              0x665f,     0xFFFF}, /*VL18_0.Vl18_0.set(16'h665f)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL18_1,              0x002a,     0xFFFF}, /*VL18_1.Vl18_1.set(8'h2a)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL19_0,              0xf0c0,     0xFFFF}, /*VL19_0.Vl19_0.set(16'hf0c0)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL19_1,              0x00e5,     0xFFFF}, /*VL19_1.Vl19_1.set(8'he5)*/

};

static const MV_OP_PARAMS mtiPcs100XgModeSeqParams[] = { /* MTI_PCS_XG_MODE_SEQ - speed up to 25G */
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x4},    /*VENDOR_PCS_MODE.Hi_ber25.set(0);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x2,        0x2},    /*VENDOR_PCS_MODE.Disable_mld.set(1);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x1,        0x1},    /*VENDOR_PCS_MODE.Ena_clause49.set(1);*/
};

static const MV_OP_PARAMS mtiPcs100Xg25ModeSeqParams[] = { /* MTI_PCS_XG_MODE_SEQ - speed 25G no rs fec*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x4,        0x4},    /*VENDOR_PCS_MODE.Hi_ber25.set(1);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x2,        0x2},    /*VENDOR_PCS_MODE.Disable_mld.set(1);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x1,        0x1},    /*VENDOR_PCS_MODE.Ena_clause49.set(1);*/
};

static const MV_OP_PARAMS mtiPcs100Xg25RsFecModeSeqParams[] = { /* MTI_PCS_XG_25_RS_FEC_MODE_SEQ - 25G with rs fec*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_VL_INTVL,     0x4FFF,     0xFFFF}, /*VENDOR_VL_INTVL.Marker_counter.set(0x4FFF);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x4,        0x4},    /*VENDOR_PCS_MODE.Hi_ber25.set(1);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x2},    /*VENDOR_PCS_MODE.Disable_mld.set(0);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x1,        0x1},    /*VENDOR_PCS_MODE.Ena_clause49.set(1);*/
    {MTI_PCS100_UNIT, MTI_PCS_VL0_0,               0x68c1,     0xFFFF}, /*VL0_0.Vl0_0.set(16'h68c1)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL0_1,               0x0021,     0xFFFF}, /*VL0_1.Vl0_1.set(8'h21)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL1_0,               0xc4f0,     0xFFFF}, /*VL1_0.Vl1_0.set(16'hc4f0)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL1_1,               0x00e6,     0xFFFF}, /*VL1_1.Vl1_1.set(8'he6)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL2_0,               0x65c5,     0xFFFF}, /*VL2_0.Vl2_0.set(16'h65c5)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL2_1,               0x009b,     0xFFFF}, /*VL2_1.Vl2_1.set(8'h9b)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL3_0,               0x79a2,     0xFFFF}, /*VL3_0.Vl3_0.set(16'h79a2)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL3_1,               0x003d,     0xFFFF}, /*VL3_1.Vl3_1.set(8'h3d)*/

};


static const MV_OP_PARAMS mtiPcs100Xlg50R1ModeSeqParams[] = { /* speed  50G r1*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_VL_INTVL,     0x4FFF,     0xFFFF}, /*VENDOR_VL_INTVL.Marker_counter.set(4FFF);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x2},    /*VENDOR_PCS_MODE.Disable_mld.set(0);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x1},    /*VENDOR_PCS_MODE.Ena_clause49.set(0);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x4},    /*VENDOR_PCS_MODE.Hi_ber25.set(0);*/

    /* Marker values for 50G_R2,50G_R1 and 40G_R4 (no FEC dependency) */
    {MTI_PCS100_UNIT, MTI_PCS_VL0_0,               0x7690,     0xFFFF}, /*VL0_0.Vl0_0.set(16'h7690)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL0_1,               0x0047,     0xFFFF}, /*VL0_1.Vl0_1.set(8'h47)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL1_0,               0xc4f0,     0xFFFF}, /*VL1_0.Vl1_0.set(16'hc4f0)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL1_1,               0x00e6,     0xFFFF}, /*VL1_1.Vl1_1.set(8'he6)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL2_0,               0x65c5,     0xFFFF}, /*VL2_0.Vl2_0.set(16'h65c5)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL2_1,               0x009b,     0xFFFF}, /*VL2_1.Vl2_1.set(8'h9b)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL3_0,               0x79a2,     0xFFFF}, /*VL3_0.Vl3_0.set(16'h79a2)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL3_1,               0x003d,     0xFFFF}, /*VL3_1.Vl3_1.set(8'h3d)*/

   /* {UNIT_MTI_PCS100, MTI_PCS_CONTROL1,            0x8000,     0x8000} */ /*CONTROL1.Reset.set(1)*/
};


static const MV_OP_PARAMS mtiPcs100Xlg50R2RsFecModeSeqParams[] = { /* speed  50G r2 rs fec 528*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_VL_INTVL,     0x4FFF,     0xFFFF}, /*VENDOR_VL_INTVL.Marker_counter.set(4FFF);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x2},    /*VENDOR_PCS_MODE.Disable_mld.set(0);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x1},    /*VENDOR_PCS_MODE.Ena_clause49.set(0);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x4},    /*VENDOR_PCS_MODE.Hi_ber25.set(0);*/

    /* Marker values for 50G_R2,50G_R1 and 40G_R4 (no FEC dependency) */
    {MTI_PCS100_UNIT, MTI_PCS_VL0_0,               0x7690,     0xFFFF}, /*VL0_0.Vl0_0.set(16'h7690)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL0_1,               0x0047,     0xFFFF}, /*VL0_1.Vl0_1.set(8'h47)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL1_0,               0xc4f0,     0xFFFF}, /*VL1_0.Vl1_0.set(16'hc4f0)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL1_1,               0x00e6,     0xFFFF}, /*VL1_1.Vl1_1.set(8'he6)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL2_0,               0x65c5,     0xFFFF}, /*VL2_0.Vl2_0.set(16'h65c5)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL2_1,               0x009b,     0xFFFF}, /*VL2_1.Vl2_1.set(8'h9b)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL3_0,               0x79a2,     0xFFFF}, /*VL3_0.Vl3_0.set(16'h79a2)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL3_1,               0x003d,     0xFFFF}, /*VL3_1.Vl3_1.set(8'h3d)*/

    /************* MTI_RSFEC::Configure(FEC_TYPE _50G_CONSORTIUM); **********/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_CONTROL,           0x200,     0x200}, /*RSFEC_CONTROL.Tc_pad_value.set(1'b1)*/

   /* {UNIT_MTI_PCS100, MTI_PCS_CONTROL1,            0x8000,     0x8000} */ /*CONTROL1.Reset.set(1)*/
};

static const MV_OP_PARAMS mtiPcs100XlgModeSeqParams[] = { /* speed  40G,50G */
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_VL_INTVL,     0x3FFF,     0xFFFF}, /*VENDOR_VL_INTVL.Marker_counter.set(4FFF);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x2},    /*VENDOR_PCS_MODE.Disable_mld.set(0);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x1},    /*VENDOR_PCS_MODE.Ena_clause49.set(0);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x4},    /*VENDOR_PCS_MODE.Hi_ber25.set(0);*/

    /* Marker values for 50G_R2,50G_R1 and 40G_R4 (no FEC dependency) */
    {MTI_PCS100_UNIT, MTI_PCS_VL0_0,               0x7690,     0xFFFF}, /*VL0_0.Vl0_0.set(16'h7690)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL0_1,               0x0047,     0xFFFF}, /*VL0_1.Vl0_1.set(8'h47)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL1_0,               0xc4f0,     0xFFFF}, /*VL1_0.Vl1_0.set(16'hc4f0)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL1_1,               0x00e6,     0xFFFF}, /*VL1_1.Vl1_1.set(8'he6)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL2_0,               0x65c5,     0xFFFF}, /*VL2_0.Vl2_0.set(16'h65c5)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL2_1,               0x009b,     0xFFFF}, /*VL2_1.Vl2_1.set(8'h9b)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL3_0,               0x79a2,     0xFFFF}, /*VL3_0.Vl3_0.set(16'h79a2)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL3_1,               0x003d,     0xFFFF}, /*VL3_1.Vl3_1.set(8'h3d)*/

   /* {UNIT_MTI_PCS100, MTI_PCS_CONTROL1,            0x8000,     0x8000} */ /*CONTROL1.Reset.set(1)*/
};

static const MV_OP_PARAMS mtiPcs100CgModeSeqParams[] = { /* speed above 50G */
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_VL_INTVL,     0x3FFF,       0xFFFF}, /*VENDOR_VL_INTVL.Marker_counter.set(3FFF);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x2}, /*VENDOR_PCS_MODE.Disable_mld.set(0);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x1}, /*VENDOR_PCS_MODE.Ena_clause49.set(0);*/
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x4},    /*VENDOR_PCS_MODE.Hi_ber25.set(0);*/

    /* Marker values for 100G-KR4 / 100G-KR2 (no FEC dependency) */
    {MTI_PCS100_UNIT, MTI_PCS_VL0_0,               0x68c1,     0xFFFF}, /*VL0_0.Vl0_0.set(16'h68c1)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL0_1,               0x0021,     0xFFFF}, /*VL0_1.Vl0_1.set(8'h21)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL1_0,               0x719d,     0xFFFF}, /*VL1_0.Vl1_0.set(16'h719d)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL1_1,               0x008e,     0xFFFF}, /*VL1_1.Vl1_1.set(8'h8e)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL2_0,               0x4b59,     0xFFFF}, /*VL2_0.Vl2_0.set(16'h4b59)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL2_1,               0x00e8,     0xFFFF}, /*VL2_1.Vl2_1.set(8'he8)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL3_0,               0x954d,     0xFFFF}, /*VL3_0.Vl3_0.set(16'h954d)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL3_1,               0x007b,     0xFFFF}, /*VL3_1.Vl3_1.set(8'h7b)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL4_0,               0x07f5,     0xFFFF}, /*VL4_0.Vl4_0.set(16'h7f5)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL4_1,               0x0009,     0xFFFF}, /*VL4_1.Vl4_1.set(8'h9)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL5_0,               0x14dd,     0xFFFF}, /*VL5_0.Vl5_0.set(16'h14dd)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL5_1,               0x00c2,     0xFFFF}, /*VL5_1.Vl5_1.set(8'hc2)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL6_0,               0x4a9a,     0xFFFF}, /*VL6_0.Vl6_0.set(16'h4a9a)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL6_1,               0x0026,     0xFFFF}, /*VL6_1.Vl6_1.set(8'h26)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL7_0,               0x457b,     0xFFFF}, /*VL7_0.Vl7_0.set(16'h457b)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL7_1,               0x0066,     0xFFFF}, /*VL7_1.Vl7_1.set(8'h66)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL8_0,               0x24a0,     0xFFFF}, /*VL8_0.Vl8_0.set(16'h24a0)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL8_1,               0x0076,     0xFFFF}, /*VL8_1.Vl8_1.set(8'h76)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL9_0,               0xc968,     0xFFFF}, /*VL9_0.Vl9_0.set(16'hc968)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL9_1,               0x00fb,     0xFFFF}, /*VL9_1.Vl9_1.set(8'hfb)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL10_0,              0x6cfd,     0xFFFF}, /*VL10_0.Vl10_0.set(16'h6cfd)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL10_1,              0x0099,     0xFFFF}, /*VL10_1.Vl10_1.set(8'h99)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL11_0,              0x91b9,     0xFFFF}, /*VL11_0.Vl11_0.set(16'h91b9)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL11_1,              0x0055,     0xFFFF}, /*VL11_1.Vl11_1.set(8'h55)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL12_0,              0xb95c,     0xFFFF}, /*VL12_0.Vl12_0.set(16'h)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL12_1,              0x00b2,     0xFFFF}, /*VL12_1.Vl12_1.set(8'hb2)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL13_0,              0xf81a,     0xFFFF}, /*VL13_0.Vl13_0.set(16'hf81a)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL13_1,              0x00bd,     0xFFFF}, /*VL13_1.Vl13_1.set(8'hbd)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL14_0,              0xc783,     0xFFFF}, /*VL14_0.Vl14_0.set(16'hc783)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL14_1,              0x00ca,     0xFFFF}, /*VL14_1.Vl14_1.set(8'hca)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL15_0,              0x3635,     0xFFFF}, /*VL15_0.Vl15_0.set(16'h3635)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL15_1,              0x00cd,     0xFFFF}, /*VL15_1.Vl15_1.set(8'hcd)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL16_0,              0x31c4,     0xFFFF}, /*VL16_0.Vl16_0.set(16'h31c4)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL16_1,              0x004c,     0xFFFF}, /*VL16_1.Vl16_1.set(8'h4c)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL17_0,              0xd6ad,     0xFFFF}, /*VL17_0.Vl17_0.set(16'hd6ad)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL17_1,              0x00b7,     0xFFFF}, /*VL17_1.Vl17_1.set(8'hb7)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL18_0,              0x665f,     0xFFFF}, /*VL18_0.Vl18_0.set(16'h665f)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL18_1,              0x002a,     0xFFFF}, /*VL18_1.Vl18_1.set(8'h2a)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL19_0,              0xf0c0,     0xFFFF}, /*VL19_0.Vl19_0.set(16'hf0c0)*/
    {MTI_PCS100_UNIT, MTI_PCS_VL19_1,              0x00e5,     0xFFFF} /*VL19_1.Vl19_1.set(8'he5)*/

    /*{UNIT_MTI_PCS100, MTI_PCS_CONTROL1,            0x8000,     0x8000} */ /*CONTROL1.Reset.set(1)*/

};

static const MV_OP_PARAMS mtiPcs100LbNormalSeqParams[] = {
    {MTI_PCS100_UNIT, MTI_PCS_CONTROL1,               0x0,       0x4000},
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,        0x1,       0x1}    /*VENDOR_PCS_MODE.Ena_clause49.set(1);*/
};

static const MV_OP_PARAMS mtiPcs100LbTx2RxSeqParams[] = {
    {MTI_PCS100_UNIT, MTI_PCS_CONTROL1,               0x4000,    0x4000}
};
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) || defined (FALCON_DEV_SUPPORT)
static MV_OP_PARAMS mtiPcs100LbTx2RxWaSeqParams[] = {
    {MTI_PCS100_UNIT, MTI_PCS_CONTROL1,               0x4000,    0x4000},
    {MTI_PCS100_UNIT, MTI_PCS_VENDOR_PCS_MODE,        0x0,       0x1}    /*VENDOR_PCS_MODE.Ena_clause49.set(0);*/
};
#endif
/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsMtiPcs100SeqDb[] =
{
    {mtiPcs100resetSeqParams,            MV_SEQ_SIZE(mtiPcs100resetSeqParams)},           /* MTI_PCS_RESET_SEQ            */
    {mtiPcs100unresetSeqParams,          MV_SEQ_SIZE(mtiPcs100unresetSeqParams)},         /* MTI_PCS_UNRESET_SEQ          */
    {mtiPcs100PowerDownSeqParams,        MV_SEQ_SIZE(mtiPcs100PowerDownSeqParams)},       /* MTI_PCS_POWER_DOWN_SEQ       */

    {mtiPcs100XgModeSeqParams,           MV_SEQ_SIZE(mtiPcs100XgModeSeqParams)},          /* MTI_PCS_XG_MODE_SEQ     */
    {mtiPcs100Xg25ModeSeqParams,         MV_SEQ_SIZE(mtiPcs100Xg25ModeSeqParams)},        /* MTI_PCS_XG_25_MODE_SEQ */
    {mtiPcs100Xg25RsFecModeSeqParams,    MV_SEQ_SIZE(mtiPcs100Xg25RsFecModeSeqParams)},   /* MTI_PCS_XG_25_RS_FEC_MODE_SEQ */

    {mtiPcs100Xlg50R1ModeSeqParams,      MV_SEQ_SIZE(mtiPcs100Xlg50R1ModeSeqParams)},     /* MTI_PCS_XLG_50R1_MODE_SEQ    */
    {mtiPcs100Xlg50R2RsFecModeSeqParams, MV_SEQ_SIZE(mtiPcs100Xlg50R2RsFecModeSeqParams)},/* MTI_PCS_XLG_50R2_RS_FEC_MODE_SEQ    */
    {mtiPcs100XlgModeSeqParams,          MV_SEQ_SIZE(mtiPcs100XlgModeSeqParams)},         /* MTI_PCS_XlG_MODE_SEQ    */

    {mtiPcs100CgModeSeqParams,           MV_SEQ_SIZE(mtiPcs100CgModeSeqParams)},          /* MTI_PCS_CG_MODE_SEQ     */

    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_200R4_MODE_SEQ     */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_200R8_MODE_SEQ     */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_400R8_MODE_SEQ     */

    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_START_SEND_FAULT_SEQ */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_STOP_SEND_FAULT_SEQ  */
    {mtiPcs100LbNormalSeqParams,         MV_SEQ_SIZE(mtiPcs100LbNormalSeqParams)},        /* MTI_PCS_LPBK_NORMAL_SEQ      */
    {mtiPcs100LbTx2RxSeqParams,          MV_SEQ_SIZE(mtiPcs100LbTx2RxSeqParams)},         /* MTI_PCS_LPBK_TX2RX_SEQ       */

    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_200G_R4_POWER_DOWN_SEQ */
    {mtiPcs100PowerDownNoRsFecSeqParams, MV_SEQ_SIZE(mtiPcs100PowerDownNoRsFecSeqParams)}, /* MTI_PCS_POWER_DOWN_NO_RS_FEC_SEQ */
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) || defined (FALCON_DEV_SUPPORT)
    {mtiPcs100LbTx2RxWaSeqParams,        MV_SEQ_SIZE(mtiPcs100LbTx2RxWaSeqParams)}       /* MTI_PCS_LPBK_TX2RX_WA_SEQ */
#endif

};

GT_STATUS hwsMtiPcs100SeqGet(MV_HWS_MTI_PCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
  if (seqType >= MTI_PCS_LAST_SEQ)
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsMtiPcs100SeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}





