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
* @file mvHwsUsxRsfecUnits.c
*
* @brief Hawk MTI USX Multiplexer register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>



const MV_HWS_REG_ADDR_FIELD_STC hawkUsxRsfecUnitsDb[] = {
    /* RSFEC_CONTROL */
  /*0*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BYPASS_CORRECTION */
  /*1*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x3000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*BYPASS_ERROR_INDICATION */
  /*2*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x3000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FEC91_ENA */
  /*3*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x3000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TC_PAD_VALUE */

    /* RSFEC_STATUS */
  /*4*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BYPASS_CORRECTION_ABILITY */
  /*5*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x3000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*BYPASS_INDICATION_ABILITY */
  /*6*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x3000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*HIGH_SER */
  /*7*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x3000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*AMPS_LOCK */
  /*8*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x3000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*FEC_ALIGN_STATUS */
  /*9*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x3000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*FEC_PCS_ALIGN_STATUS */

    /* RSFEC_CCW_LO */
  /*10*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RSFEC_CCW_LO */

    /* RSFEC_CCW_HI */
  /*11*/  {/*baseAddr*/ 0x000c,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RSFEC_CCW_HI */

    /* RSFEC_NCCW_LO */
  /*12*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RSFEC_NCCW_LO */

    /* RSFEC_NCCW_HI */
  /*13*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RSFEC_NCCW_HI */

    /* RSFEC_SYMBLERR0_LO */
  /*14*/  {/*baseAddr*/ 0x0028,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR0_LO */

    /* RSFEC_SYMBLERR0_HI */
  /*15*/  {/*baseAddr*/ 0x002c,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR0_HI */

    /* RSFEC_SYMBLERR1_LO */
  /*16*/  {/*baseAddr*/ 0x0030,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR1_LO */

    /* RSFEC_SYMBLERR1_HI */
  /*17*/  {/*baseAddr*/ 0x0034,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR1_HI */

    /* RSFEC_SYMBLERR2_LO */
  /*18*/  {/*baseAddr*/ 0x0038,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR2_LO */

    /* RSFEC_SYMBLERR2_HI */
  /*19*/  {/*baseAddr*/ 0x003c,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR2_HI */

    /* RSFEC_SYMBLERR3_LO */
  /*20*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR3_LO */

    /* RSFEC_SYMBLERR3_HI */
  /*21*/  {/*baseAddr*/ 0x004c,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR3_HI */

    /* RSFEC_VENDOR_INFO1 */
  /*22*/  {/*baseAddr*/ 0x0204,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*VENDOR_AMPS_LOCK */
  /*23*/  {/*baseAddr*/ 0x0204,  /*offsetFormula*/ 0x3000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*VENDOR_ALIGN_STATUS_LH */
  /*24*/  {/*baseAddr*/ 0x0204,  /*offsetFormula*/ 0x3000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*VENDOR_MARKER_CHECK_RESTART */
  /*25*/  {/*baseAddr*/ 0x0204,  /*offsetFormula*/ 0x3000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*VENDOR_RX_DATAPATH_RESTART */
  /*26*/  {/*baseAddr*/ 0x0204,  /*offsetFormula*/ 0x3000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*VENDOR_TX_DATAPATH_RESTART */
  /*27*/  {/*baseAddr*/ 0x0204,  /*offsetFormula*/ 0x3000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*VENDOR_RX_DP_OVERFLOW */
  /*28*/  {/*baseAddr*/ 0x0204,  /*offsetFormula*/ 0x3000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*VENDOR_TX_DP_OVERFLOW */
  /*29*/  {/*baseAddr*/ 0x0204,  /*offsetFormula*/ 0x3000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*VENDOR_ALIGN_STATUS_LL */

    /* RSFEC_VENDOR_REVISION */
  /*30*/  {/*baseAddr*/ 0x020c,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*REVISION */

    /* RSFEC_VENDOR_DECODER_THRESHOLD */
  /*31*/  {/*baseAddr*/ 0x0228,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*RSFEC_DECODER_THRESHOLD */

    /* RSFEC_VENDOR_TX_FIFO_THRESHOLD */
  /*32*/  {/*baseAddr*/ 0x022c,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*TX_LANE_THRESH */
  /*33*/  {/*baseAddr*/ 0x022c,  /*offsetFormula*/ 0x3000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*TOGGLE_EN */

    /* BASE_R_FEC_CW_HI */
  /*34*/  {/*baseAddr*/ 0x314,   /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FEC_CW_HI */

    /* BASE_R_FEC_CONTROL */
  /*35*/  {/*baseAddr*/ 0x304,   /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_ENABLE */
  /*36*/  {/*baseAddr*/ 0x304,   /*offsetFormula*/ 0x3000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_ERROR_INDICATION_ENABLE */

    /* BASE_R_FEC_ABILITY */
  /*37*/  {/*baseAddr*/ 0x300,   /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_ABILITY */
  /*38*/  {/*baseAddr*/ 0x300,   /*offsetFormula*/ 0x3000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_ERROR_INDICATION_ABILITY */

    /* BASE_R_FEC_STATUS */
  /*39*/  {/*baseAddr*/ 0x308,   /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_LOCKED */

    /* BASE_R_FEC_CCW_LO */
  /*40*/  {/*baseAddr*/ 0x30c,   /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FEC_CCW_LO */

    /* BASE_R_FEC_NCCW_LO */
  /*41*/  {/*baseAddr*/ 0x310,   /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FEC_NCCW_LO */
};

