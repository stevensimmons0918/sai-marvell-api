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
* @file mvHwsMtiRsfecUnits.c
*
* @brief Hawk MTI RSFEC register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>


const MV_HWS_REG_ADDR_FIELD_STC hawkMtiRsFecUnitsDb[] = {
    /* p0_RSFEC_CONTROL */
  /*0*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x20, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_BYPASS_CORRECTION */
  /*1*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x20, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_BYPASS_ERROR_INDICATION */
  /*2*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x20, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_DEGRADE_ENABLE */
  /*3*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x20, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_AM16_COPY_DIS */
  /*4*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x20, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_KP_ENABLE */
  /*5*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x20, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_TC_PAD_VALUE */
  /*6*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x20, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_TC_PAD_ALTER */

    /* p0_RSFEC_STATUS */
  /*7*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x20, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_BYPASS_CORRECTION_ABILITY */
  /*8*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x20, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_BYPASS_INDICATION_ABILITY */
  /*9*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x20, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_HIGH_SER */
  /*10*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x20, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_DEGRADE_SER_ABILITY */
  /*11*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x20, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_DEGRADE_SER */
  /*12*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x20, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_RX_AM_SF2 */
  /*13*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x20, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_RX_AM_SF1 */
  /*14*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x20, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_RX_AM_SF0 */
  /*15*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x20, /*fieldStart*/  8, /*fieldLen*/ 4 }, /*P0_AMPS_LOCK */
  /*16*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x20, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_FEC_ALIGN_STATUS */

    /* p0_RSFEC_CCW_LO */
  /*17*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x20, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_RSFEC_CCW_LO */

    /* p0_RSFEC_CCW_HI */
  /*18*/  {/*baseAddr*/ 0x000c,  /*offsetFormula*/ 0x20, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_RSFEC_CCW_HI */

    /* p0_RSFEC_NCCW_LO */
  /*19*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x20, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_RSFEC_NCCW_LO */

    /* p0_RSFEC_NCCW_HI */
  /*20*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x20, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_RSFEC_NCCW_HI */

    /* p0_RSFEC_LANE_MAP */
  /*21*/  {/*baseAddr*/ 0x0018,  /*offsetFormula*/ 0x20, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*P0_RSFEC_LANE_MAP */

    /* p0_RSFEC_DEC_THRESH */
  /*22*/  {/*baseAddr*/ 0x001c,  /*offsetFormula*/ 0x20, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*P0_RSFEC_DEC_THRESH */

    /* HISER_CW */
  /*23*/  {/*baseAddr*/ 0x0100,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*HISER_CW */

    /* HISER_THRESH */
  /*24*/  {/*baseAddr*/ 0x0104,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*HISER_THRESH */

    /* HISER_TIME */
  /*25*/  {/*baseAddr*/ 0x0108,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*HISER_TIME */

    /* DEGRADE_SET_CW */
  /*26*/  {/*baseAddr*/ 0x0110,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*DEGRADE_SET_CW */

    /* DEGRADE_SET_CW_HI */
  /*27*/  {/*baseAddr*/ 0x0114,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*DEGRADE_SET_CW_HI */

    /* DEGRADE_SET_THRESH */
  /*28*/  {/*baseAddr*/ 0x0118,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*DEGRADE_SET_THRESH */

    /* DEGRADE_SET_THRESH_HI */
  /*29*/  {/*baseAddr*/ 0x011c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*DEGRADE_SET_THRESH_HI */

    /* DEGRADE_CLEAR_CW */
  /*30*/  {/*baseAddr*/ 0x0120,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*DEGRADE_CLEAR_CW */

    /* DEGRADE_CLEAR_CW_HI */
  /*31*/  {/*baseAddr*/ 0x0124,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*DEGRADE_CLEAR_CW_HI */

    /* DEGRADE_CLEAR_THRESH */
  /*32*/  {/*baseAddr*/ 0x0128,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*DEGRADE_CLEAR_THRESH */

    /* DEGRADE_CLEAR_THRESH_HI */
  /*33*/  {/*baseAddr*/ 0x012c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*DEGRADE_CLEAR_THRESH_HI */

    /* VENDOR_VL0_0 */
  /*34*/  {/*baseAddr*/ 0x0180,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM0 */
  /*35*/  {/*baseAddr*/ 0x0180,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*UM1 */

    /* VENDOR_VL0_1 */
  /*36*/  {/*baseAddr*/ 0x0184,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM2 */

    /* VENDOR_VL1_0 */
  /*37*/  {/*baseAddr*/ 0x0188,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM0 */
  /*38*/  {/*baseAddr*/ 0x0188,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*UM1 */

    /* VENDOR_VL1_1 */
  /*39*/  {/*baseAddr*/ 0x018c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM2 */

    /* VENDOR_VL2_0 */
  /*40*/  {/*baseAddr*/ 0x0190,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM0 */
  /*41*/  {/*baseAddr*/ 0x0190,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*UM1 */

    /* VENDOR_VL2_1 */
  /*42*/  {/*baseAddr*/ 0x0194,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM2 */

    /* VENDOR_VL3_0 */
  /*43*/  {/*baseAddr*/ 0x0198,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM0 */
  /*44*/  {/*baseAddr*/ 0x0198,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*UM1 */

    /* VENDOR_VL3_1 */
  /*45*/  {/*baseAddr*/ 0x019c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM2 */

    /* VENDOR_VL4_0 */
  /*46*/  {/*baseAddr*/ 0x01a0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM0 */
  /*47*/  {/*baseAddr*/ 0x01a0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*UM1 */

    /* VENDOR_VL4_1 */
  /*48*/  {/*baseAddr*/ 0x01a4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM2 */

    /* VENDOR_VL5_0 */
  /*49*/  {/*baseAddr*/ 0x01a8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM0 */
  /*50*/  {/*baseAddr*/ 0x01a8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*UM1 */

    /* VENDOR_VL5_1 */
  /*51*/  {/*baseAddr*/ 0x01ac,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM2 */

    /* VENDOR_VL6_0 */
  /*52*/  {/*baseAddr*/ 0x01b0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM0 */
  /*53*/  {/*baseAddr*/ 0x01b0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*UM1 */

    /* VENDOR_VL6_1 */
  /*54*/  {/*baseAddr*/ 0x01b4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM2 */

    /* VENDOR_VL7_0 */
  /*55*/  {/*baseAddr*/ 0x01b8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM0 */
  /*56*/  {/*baseAddr*/ 0x01b8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*UM1 */

    /* VENDOR_VL7_1 */
  /*57*/  {/*baseAddr*/ 0x01bc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM2 */

    /* VENDOR_VL8_0 */
  /*58*/  {/*baseAddr*/ 0x01c0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM0 */
  /*59*/  {/*baseAddr*/ 0x01c0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*UM1 */

    /* VENDOR_VL8_1 */
  /*60*/  {/*baseAddr*/ 0x01c4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM2 */

    /* VENDOR_VL9_0 */
  /*61*/  {/*baseAddr*/ 0x01c8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM0 */
  /*62*/  {/*baseAddr*/ 0x01c8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*UM1 */

    /* VENDOR_VL9_1 */
  /*63*/  {/*baseAddr*/ 0x01cc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM2 */

    /* VENDOR_VL10_0 */
  /*64*/  {/*baseAddr*/ 0x01d0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM0 */
  /*65*/  {/*baseAddr*/ 0x01d0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*UM1 */

    /* VENDOR_VL10_1 */
  /*66*/  {/*baseAddr*/ 0x01d4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM2 */

    /* VENDOR_VL11_0 */
  /*67*/  {/*baseAddr*/ 0x01d8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM0 */
  /*68*/  {/*baseAddr*/ 0x01d8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*UM1 */

    /* VENDOR_VL11_1 */
  /*69*/  {/*baseAddr*/ 0x01dc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM2 */

    /* VENDOR_VL12_0 */
  /*70*/  {/*baseAddr*/ 0x01e0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM0 */
  /*71*/  {/*baseAddr*/ 0x01e0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*UM1 */

    /* VENDOR_VL12_1 */
  /*72*/  {/*baseAddr*/ 0x01e4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM2 */

    /* VENDOR_VL13_0 */
  /*73*/  {/*baseAddr*/ 0x01e8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM0 */
  /*74*/  {/*baseAddr*/ 0x01e8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*UM1 */

    /* VENDOR_VL13_1 */
  /*75*/  {/*baseAddr*/ 0x01ec,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM2 */

    /* VENDOR_VL14_0 */
  /*76*/  {/*baseAddr*/ 0x01f0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM0 */
  /*77*/  {/*baseAddr*/ 0x01f0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*UM1 */

    /* VENDOR_VL14_1 */
  /*78*/  {/*baseAddr*/ 0x01f4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM2 */

    /* VENDOR_VL15_0 */
  /*79*/  {/*baseAddr*/ 0x01f8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM0 */
  /*80*/  {/*baseAddr*/ 0x01f8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*UM1 */

    /* VENDOR_VL15_1 */
  /*81*/  {/*baseAddr*/ 0x01fc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*UM2 */

    /* RSFEC_SYMBLERR0_LO */
  /*82*/  {/*baseAddr*/ 0x0200,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR0_LO */

    /* RSFEC_SYMBLERR0_HI */
  /*83*/  {/*baseAddr*/ 0x0204,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR0_HI */

    /* RSFEC_SYMBLERR1_LO */
  /*84*/  {/*baseAddr*/ 0x0208,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR1_LO */

    /* RSFEC_SYMBLERR1_HI */
  /*85*/  {/*baseAddr*/ 0x020c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR1_HI */

    /* RSFEC_SYMBLERR2_LO */
  /*86*/  {/*baseAddr*/ 0x0210,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR2_LO */

    /* RSFEC_SYMBLERR2_HI */
  /*87*/  {/*baseAddr*/ 0x0214,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR2_HI */

    /* RSFEC_SYMBLERR3_LO */
  /*88*/  {/*baseAddr*/ 0x0218,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR3_LO */

    /* RSFEC_SYMBLERR3_HI */
  /*89*/  {/*baseAddr*/ 0x021c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR3_HI */

    /* RSFEC_SYMBLERR4_LO */
  /*90*/  {/*baseAddr*/ 0x0220,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR4_LO */

    /* RSFEC_SYMBLERR4_HI */
  /*91*/  {/*baseAddr*/ 0x0224,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR4_HI */

    /* RSFEC_SYMBLERR5_LO */
  /*92*/  {/*baseAddr*/ 0x0228,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR5_LO */

    /* RSFEC_SYMBLERR5_HI */
  /*93*/  {/*baseAddr*/ 0x022c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR5_HI */

    /* RSFEC_SYMBLERR6_LO */
  /*94*/  {/*baseAddr*/ 0x0230,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR6_LO */

    /* RSFEC_SYMBLERR6_HI */
  /*95*/  {/*baseAddr*/ 0x0234,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR6_HI */

    /* RSFEC_SYMBLERR7_LO */
  /*96*/  {/*baseAddr*/ 0x0238,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR7_LO */

    /* RSFEC_SYMBLERR7_HI */
  /*97*/  {/*baseAddr*/ 0x023c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR7_HI */

    /* RSFEC_SYMBLERR8_LO */
  /*98*/  {/*baseAddr*/ 0x0240,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR8_LO */

    /* RSFEC_SYMBLERR8_HI */
  /*99*/  {/*baseAddr*/ 0x0244,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR8_HI */

    /* RSFEC_SYMBLERR9_LO */
  /*100*/  {/*baseAddr*/ 0x0248,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR9_LO */

    /* RSFEC_SYMBLERR9_HI */
  /*101*/  {/*baseAddr*/ 0x024c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR9_HI */

    /* RSFEC_SYMBLERR10_LO */
  /*102*/  {/*baseAddr*/ 0x0250,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR10_LO */

    /* RSFEC_SYMBLERR10_HI */
  /*103*/  {/*baseAddr*/ 0x0254,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR10_HI */

    /* RSFEC_SYMBLERR11_LO */
  /*104*/  {/*baseAddr*/ 0x0258,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR11_LO */

    /* RSFEC_SYMBLERR11_HI */
  /*105*/  {/*baseAddr*/ 0x025c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR11_HI */

    /* RSFEC_SYMBLERR12_LO */
  /*106*/  {/*baseAddr*/ 0x0260,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR12_LO */

    /* RSFEC_SYMBLERR12_HI */
  /*107*/  {/*baseAddr*/ 0x0264,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR12_HI */

    /* RSFEC_SYMBLERR13_LO */
  /*108*/  {/*baseAddr*/ 0x0268,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR13_LO */

    /* RSFEC_SYMBLERR13_HI */
  /*109*/  {/*baseAddr*/ 0x026c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR13_HI */

    /* RSFEC_SYMBLERR14_LO */
  /*110*/  {/*baseAddr*/ 0x0270,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR14_LO */

    /* RSFEC_SYMBLERR14_HI */
  /*111*/  {/*baseAddr*/ 0x0274,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR14_HI */

    /* RSFEC_SYMBLERR15_LO */
  /*112*/  {/*baseAddr*/ 0x0278,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR15_LO */

    /* RSFEC_SYMBLERR15_HI */
  /*113*/  {/*baseAddr*/ 0x027c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR15_HI */

    /* RSFEC_VENDOR_INFO1 */
  /*114*/  {/*baseAddr*/ 0x0284,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*VENDOR_AMPS_LOCK */
  /*115*/  {/*baseAddr*/ 0x0284,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*VENDOR_ALIGN_STATUS_LH */
  /*116*/  {/*baseAddr*/ 0x0284,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*VENDOR_MARKER_CHECK_RESTART */
  /*117*/  {/*baseAddr*/ 0x0284,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*VENDOR_ALIGN_STATUS_LL */

    /* RSFEC_VENDOR_INFO2 */
  /*118*/  {/*baseAddr*/ 0x0288,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VENDOR_AMPS_LOCK_LANES */

    /* RSFEC_VENDOR_REVISION */
  /*119*/  {/*baseAddr*/ 0x028c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*REVISION */

    /* RSFEC_VENDOR_ALIGN_STATUS */
  /*120*/  {/*baseAddr*/ 0x0290,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*RSFEC_VENDOR_ALIGN_STATUS */
};

