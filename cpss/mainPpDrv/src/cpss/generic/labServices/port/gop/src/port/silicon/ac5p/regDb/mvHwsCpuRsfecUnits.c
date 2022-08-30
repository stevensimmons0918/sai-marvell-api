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
* @file mvHwsCpuRsfecUnits.c
*
* @brief Hawk MTI USX Multiplexer register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>



const MV_HWS_REG_ADDR_FIELD_STC hawkCpuRsfecUnitsDb[] = {
    /* RSFEC_CONTROL */
  /*0*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BYPASS_CORRECTION */
  /*1*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*BYPASS_ERROR_INDICATION */

    /* RSFEC_STATUS */
  /*2*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BYPASS_CORRECTION_ABILITY */
  /*3*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*BYPASS_INDICATION_ABILITY */
  /*4*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*HIGH_SER */
  /*5*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*AMPS_LOCK */
  /*6*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*FEC_ALIGN_STATUS */
  /*7*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*FEC_PCS_ALIGN_STATUS */

    /* RSFEC_CCW_LO */
  /*8*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RSFEC_CCW_LO */

    /* RSFEC_CCW_HI */
  /*9*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RSFEC_CCW_HI */

    /* RSFEC_NCCW_LO */
  /*10*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RSFEC_NCCW_LO */

    /* RSFEC_NCCW_HI */
  /*11*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RSFEC_NCCW_HI */

    /* RSFEC_SYMBLERR0_LO */
  /*12*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR0_LO */

    /* RSFEC_SYMBLERR0_HI */
  /*13*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR0_HI */

    /* RSFEC_SYMBLERR1_LO */
  /*14*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR1_LO */

    /* RSFEC_SYMBLERR1_HI */
  /*15*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR1_HI */

    /* RSFEC_SYMBLERR2_LO */
  /*16*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR2_LO */

    /* RSFEC_SYMBLERR2_HI */
  /*17*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR2_HI */

    /* RSFEC_SYMBLERR3_LO */
  /*18*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR3_LO */

    /* RSFEC_SYMBLERR3_HI */
  /*19*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SYMBLERR3_HI */

    /* RSFEC_VENDOR_INFO1 */
  /*20*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*VENDOR_AMPS_LOCK */
  /*21*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*VENDOR_ALIGN_STATUS_LH */
  /*22*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*VENDOR_MARKER_CHECK_RESTART */
  /*23*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*VENDOR_RX_DATAPATH_RESTART */
  /*24*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*VENDOR_TX_DATAPATH_RESTART */
  /*25*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*VENDOR_RX_DP_OVERFLOW */
  /*26*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*VENDOR_TX_DP_OVERFLOW */
  /*27*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*VENDOR_ALIGN_STATUS_LL */

    /* RSFEC_VENDOR_REVISION */
  /*28*/  {/*baseAddr*/ 0x20c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*REVISION */

    /* RSFEC_VENDOR_CONTROL */
  /*29*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*RS_FEC_ENABLE */
  /*30*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RS_FEC_ENABLE_STATUS */

    /* RSFEC_VENDOR_FEC_ABILITY */
  /*31*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*FEC_ABILITY */

    /* RSFEC_VENDOR_INFO2 */
  /*32*/  {/*baseAddr*/ 0x208,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VENDOR_INFO2_RSVD */

    /* RSFEC_VENDOR_FEC_CONTROL */
  /*33*/  {/*baseAddr*/ 0x304,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FEC_CONTROL */

    /* RSFEC_VENDOR_FEC_STATUS */
  /*34*/  {/*baseAddr*/ 0x308,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FEC_STATUS */

    /* RSFEC_VENDOR_VL0_CCW_LO */
  /*35*/  {/*baseAddr*/ 0x30c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL0_CCW_LO */

    /* RSFEC_VENDOR_VL0_NCCW_LO */
  /*36*/  {/*baseAddr*/ 0x310,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL0_NCCW_LO */

    /* RSFEC_VENDOR_COUNTER_HI */
  /*37*/  {/*baseAddr*/ 0x315,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VENDOR_COUNTER_HI */
};

