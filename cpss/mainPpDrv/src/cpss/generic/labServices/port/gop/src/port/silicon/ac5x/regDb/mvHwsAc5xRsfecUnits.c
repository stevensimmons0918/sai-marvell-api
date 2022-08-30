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
* @file mvHwsAc5xRsfecUnits.c
*
* @brief Phoenix MTI RSFEC register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>


const MV_HWS_REG_ADDR_FIELD_STC ac5xMtiRsFecUnitsDb[] = {
    /* p1_RSFEC_CONTROL */
  /*0*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x20, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P1_BYPASS_CORRECTION */
  /*1*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x20, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P1_BYPASS_ERROR_INDICATION */
  /*2*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x20, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P1_RSFEC_ENABLE */
  /*3*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x20, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P1_KP_ENABLE */
  /*4*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x20, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P1_TC_PAD_VALUE */
  /*5*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x20, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P1_TC_PAD_ALTER */

    /* p1_RSFEC_STATUS */
  /*6*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x20, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P1_BYPASS_CORRECTION_ABILITY */
  /*7*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x20, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P1_BYPASS_INDICATION_ABILITY */
  /*8*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x20, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P1_HIGH_SER */
  /*9*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x20, /*fieldStart*/  8, /*fieldLen*/ 4 }, /*P1_AMPS_LOCK */
  /*10*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x20, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P1_FEC_ALIGN_STATUS */
  /*11*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x20, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P1_TMP */

    /* p1_RSFEC_CCW_LO */
  /*12*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x20, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P1_RSFEC_CCW_LO */

    /* p1_RSFEC_CCW_HI */
  /*13*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x20, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P1_RSFEC_CCW_HI */

    /* p1_RSFEC_NCCW_LO */
  /*14*/  {/*baseAddr*/ 0x110,  /*offsetFormula*/ 0x20, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P1_RSFEC_NCCW_LO */

    /* p1_RSFEC_NCCW_HI */
  /*15*/  {/*baseAddr*/ 0x114,  /*offsetFormula*/ 0x20, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P1_RSFEC_NCCW_HI */

    /* p1_RSFEC_LANE_MAP */
  /*16*/  {/*baseAddr*/ 0x118,  /*offsetFormula*/ 0x20, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*P1_RSFEC_LANE_MAP */

    /* RSFEC_SYMBLERR0_LO */
  /*17*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR0_LO */

    /* RSFEC_SYMBLERR0_HI */
  /*18*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR0_HI */

    /* RSFEC_SYMBLERR1_LO */
  /*19*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR1_LO */

    /* RSFEC_SYMBLERR1_HI */
  /*20*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR1_HI */

    /* RSFEC_SYMBLERR2_LO */
  /*21*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR2_LO */

    /* RSFEC_SYMBLERR2_HI */
  /*22*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR2_HI */

    /* RSFEC_SYMBLERR3_LO */
  /*23*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR3_LO */

    /* RSFEC_SYMBLERR3_HI */
  /*24*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR3_HI */

    /* RSFEC_VENDOR_INFO1 */
  /*25*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*VENDOR_AMPS_LOCK */
  /*26*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*VENDOR_ALIGN_STATUS_LH */
  /*27*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*VENDOR_MARKER_CHECK_RESTART */
  /*28*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*RX_DATAPATH_RESTART */
  /*29*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*TX_DATAPATH_RESTART */
  /*30*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*RX_DP_OVERFLOW */
  /*31*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_DP_OVERFLOW */
  /*32*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*VENDOR_ALIGN_STATUS_LL */

    /* RSFEC_VENDOR_INFO2 */
  /*33*/  {/*baseAddr*/ 0x208,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*VENDOR_AMPS_LOCK_LANES */
  /*34*/  {/*baseAddr*/ 0x208,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*RS_DECODER_WRITE_ERR */

    /* RSFEC_VENDOR_REVISION */
  /*35*/  {/*baseAddr*/ 0x20c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*REVISION */

    /* RX_RSFEC_FIFO_LVL_CH1 */
  /*36*/  {/*baseAddr*/ 0x22c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 7 }, /*F91_FIFO_THRESH_1 */

    /* RX_RSFEC_FIFO_LVL_CH2 */
  /*37*/  {/*baseAddr*/ 0x230,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 7 }, /*F91_FIFO_THRESH_2 */

    /* VENDOR_TX_TEST_PATTERN */
  /*38*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*TXE_PATTEN */
  /*39*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*TXE_PATOVER */

    /* BASE_R_FEC_CCW_LO_VL1 */
  /*40*/  {/*baseAddr*/ 0x314,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FEC_CW_LO */

    /* BASE_R_FEC_STATUS */
  /*41*/  {/*baseAddr*/ 0x308,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_LOCKED */

    /* p0_RSFEC_NCCW_HI */
  /*42*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_RSFEC_NCCW_HI */

    /* p0_RSFEC_STATUS */
  /*43*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_BYPASS_CORRECTION_ABILITY */
  /*44*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_BYPASS_INDICATION_ABILITY */
  /*45*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_HIGH_SER */
  /*46*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 4 }, /*P0_AMPS_LOCK */
  /*47*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_FEC_ALIGN_STATUS */
  /*48*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_TMP */

    /* VENDOR_TX_TEST_SYMBOL */
  /*49*/  {/*baseAddr*/ 0x214,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*TXE_SYMBOL */

    /* BASE_R_FEC_NCCW_LO_VL1 */
  /*50*/  {/*baseAddr*/ 0x318,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*NCERR_CNT */

    /* RX_RSFEC_FIFO_LVL_CH3 */
  /*51*/  {/*baseAddr*/ 0x234,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 7 }, /*F91_FIFO_THRESH_3 */

    /* VENDOR_CONTROL */
  /*52*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FEC_ENA */
  /*53*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FEC_1LANE0 */
  /*54*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FEC_1LANE2 */
  /*55*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*AM16_CP_DIS_EN */
  /*56*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 4 }, /*FEC_EN_S */

    /* RX_RSFEC_FIFO_LVL_CH0 */
  /*57*/  {/*baseAddr*/ 0x228,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 7 }, /*F91_FIFO_THRESH_0 */

    /* p0_RSFEC_CCW_HI */
  /*58*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_RSFEC_CCW_HI */

    /* BASE_R_FEC_CCW_LO */
  /*59*/  {/*baseAddr*/ 0x30c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FEC_CCW_LO */

    /* VENDOR_TX_TEST_CONTROL */
  /*60*/  {/*baseAddr*/ 0x21c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TXE_RUN */

    /* BASE_R_FEC_CONTROL */
  /*61*/  {/*baseAddr*/ 0x304,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_ENABLE */
  /*62*/  {/*baseAddr*/ 0x304,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_ERROR_INDICATION_ENABLE */

    /* VENDOR_TX_TEST_KEY */
  /*63*/  {/*baseAddr*/ 0x210,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*TXE_KEY */

    /* BASE_R_FEC_ABILITY */
  /*64*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_ABILITY */
  /*65*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_ERROR_INDICATION_ABILITY */

    /* p0_RSFEC_CCW_LO */
  /*66*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_RSFEC_CCW_LO */

    /* p0_RSFEC_LANE_MAP */
  /*67*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*P0_RSFEC_LANE_MAP */

    /* p0_RSFEC_CONTROL */
  /*68*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_BYPASS_CORRECTION */
  /*69*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_BYPASS_ERROR_INDICATION */
  /*70*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_RSFEC_ENABLE */
  /*71*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_KP_ENABLE */
  /*72*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_TC_PAD_VALUE */
  /*73*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_TC_PAD_ALTER */

    /* p0_RSFEC_NCCW_LO */
  /*74*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_RSFEC_NCCW_LO */

    /* BASE_R_FEC_NCCW_LO */
  /*75*/  {/*baseAddr*/ 0x310,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FEC_NCCW_LO */
};

