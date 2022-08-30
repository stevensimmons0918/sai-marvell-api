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
* @file mvHwsAc5xMtiExtRegDb.c
*
* @brief Phoenix MTI EXT register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>



const MV_HWS_REG_ADDR_FIELD_STC ac5xMtiExtRegDb[] = {
    /* Port<0> Control */
  /*0*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_TX_LOC_FAULT */
  /*1*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_TX_REM_FAULT */
  /*2*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_TX_LI_FAULT */
  /*3*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_TOD_SELECT */
  /*4*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_RX_PAUSE_CONTROL */
  /*5*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_RX_PAUSE_OW_VAL */
  /*6*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_PAUSE_802_3_REFLECT */
  /*7*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_LOOP_ENA */
  /*8*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_FF_TX_CRC */
  /*9*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_MASK_SW_RESET_OUT */
  /*10*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/ 10, /*fieldLen*/ 6 }, /*P0_LED_PORT_NUM */
  /*11*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_LED_PORT_EN */
  /*12*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_LOOP_RX_BLOCK_OUT */
  /*13*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_LOOP_TX_RDY_OUT */
  /*14*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/ 19, /*fieldLen*/ 4 }, /*P0_PORT_RES_SPEED */
  /*15*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_PORT_RES_SPEED_FROM_HW */

    /* Port<0> Status */
  /*16*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_LINK_OK */
  /*17*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_LINK_STATUS */
  /*18*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_LPCS_LINK_STATUS */
  /*19*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_LPCS_RX_SYNC */
  /*20*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_LPCS_AN_DONE */
  /*21*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/  5, /*fieldLen*/ 8 }, /*P0_MAC_RES_SPEED */
  /*22*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_XL_HI_BER */
  /*23*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_RSFEC_ALIGNED */
  /*24*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_RX_TRAFFIC_IND */
  /*25*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_TX_TRAFFIC_IND */
  /*26*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_MAC_TX_EMPTY */
  /*27*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_MAC_TX_ISIDLE */
  /*28*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_MAC_TX_TS_FRM_OUT */
  /*29*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_PFC_MODE */
  /*30*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_FF_TX_SEPTY */
  /*31*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_FF_RX_EMPTY */
  /*32*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_FF_RX_DSAV */
  /*33*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_LINK_OK_CLEAN */
  /*34*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_LINK_STATUS_CLEAN */
  /*35*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_LPCS_LINK_STATUS_CLEAN */

    /* Port<0> Marker Status */
  /*36*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x18, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_MARKER_INS_CNT */
  /*37*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x18, /*fieldStart*/ 16, /*fieldLen*/ 15 }, /*P0_MARKER_INS_CNT_100 */

    /* Port<0> Interrupt Cause */
  /*38*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PORT0_INTERRUPT_CAUSE_INT_SUM */
  /*39*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_LINK_OK_CHANGE */
  /*40*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_LINK_STATUS_CHANGE */
  /*41*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_LPCS_LINK_STATUS_CHANGE */
  /*42*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_MAC_LOC_FAULT */
  /*43*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_MAC_REM_FAULT */
  /*44*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_MAC_LI_FAULT */
  /*45*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_MAC_TX_UNDERFLOW */
  /*46*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_MAC_TX_OVR_ERR */
  /*47*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_RX_OVERRUN */
  /*48*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_HI_BER */
  /*49*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_FF_RX_RDY */
  /*50*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_TSU_RX_DFF_ERR */
  /*51*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_TSU_RX_AM_ERR */
  /*52*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_TSU_TX_SYNC_ERR */
  /*53*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_TSD_NON_ACCURATE_PTP */

    /* Port<0> Interrupt Mask */
  /*54*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/  1, /*fieldLen*/ 15 }, /*P0_INTERRUPT_MASK */

    /* Global PMA Control */
  /*55*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*GC_SD_N2 */

    /* Global FEC Control */
  /*56*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*GC_FEC_ENA */
  /*57*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 4 }, /*GC_FEC_ERR_ENA */
  /*58*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 4 }, /*GC_FEC91_ENA_IN */
  /*59*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 4 }, /*GC_KP_MODE_IN */

    /* Global Channel Control */
  /*60*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GC_MODE40_ENA_IN */
  /*61*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN */
  /*62*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN0 */
  /*63*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN2 */
  /*64*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN0 */
  /*65*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN2 */
  /*66*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*GC_MLG_ENA_IN */
  /*67*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*GC_SCRAMBLER_BYPASS_100G */
  /*68*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*GC_DESCR_BYPASS_100G */
  /*69*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*BER_TIMER_SHORT */
  /*70*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*MAC_ABU_WATCHDOG_ENA */
  /*71*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*PCS_ABU_WATCHDOG_ENA */
  /*72*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 3 }, /*CYC_TO_STRETCH_MAC2APP */
  /*73*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 3 }, /*CYC_TO_STRETCH_MAC2REG */
  /*74*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 28, /*fieldLen*/ 3 }, /*CYC_TO_STRETCH_APP2REG */

    /* Global Clock Control */
  /*75*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*GC_FAST_1LANE_MODE */
  /*76*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 4 }, /*GC_PACER_10G */

    /* Global Amps Lock Status */
  /*77*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*AMPS_LOCK */

    /* Global FEC Status */
  /*78*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*FEC_LOCKED */

    /* Global FEC Error Status */
  /*79*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*FEC_CERR */
  /*80*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*FEC_NCERR */

    /* Global Block Lock Status0 */
  /*81*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK0 */
  /*82*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*BLOCK_LOCK1 */
  /*83*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*BLOCK_LOCK2 */
  /*84*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*BLOCK_LOCK3 */
  /*85*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*BLOCK_LOCK4 */
  /*86*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*BLOCK_LOCK5 */
  /*87*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*BLOCK_LOCK6 */
  /*88*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*BLOCK_LOCK7 */
  /*89*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*BLOCK_LOCK8 */
  /*90*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*BLOCK_LOCK9 */
  /*91*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*BLOCK_LOCK10 */
  /*92*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*BLOCK_LOCK11 */
  /*93*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*BLOCK_LOCK12 */
  /*94*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*BLOCK_LOCK13 */
  /*95*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*BLOCK_LOCK14 */
  /*96*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*BLOCK_LOCK15 */
  /*97*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*BLOCK_LOCK16 */
  /*98*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*BLOCK_LOCK17 */
  /*99*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*BLOCK_LOCK18 */
  /*100*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*BLOCK_LOCK19 */

    /* Global Last Violation */
  /*101*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*LAST_VIOLATION */

    /* Global Interrupt Mask */
  /*102*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_MASK */

    /* Global Interrupt Cause */
  /*103*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_CAUSE_INT_SUM */
  /*104*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*ILLEGAL_ADDRESS_ACCESS */

    /* Global MAC Metal Fix */
  /*105*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MAC_METAL_FIX */

    /* Global Interrupt Summary Cause */
  /*106*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_SUMMARY_INT_SUM */
  /*107*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_INT_SUM */
  /*108*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P1_INT_SUM */
  /*109*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P2_INT_SUM */
  /*110*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P3_INT_SUM */
  /*111*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*GLOBAL_INT_SUM */
  /*112*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FEC_CE_INT_SUM */
  /*113*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FEC_NCE_INT_SUM */

    /* Global Interrupt Summary Mask */
  /*114*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 7 }, /*GLOBAL_INTERRUPT_SUMMARY_MASK */

    /* Global Reset Control */
  /*115*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*GC_SD_RX_RESET_ */
  /*116*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 4 }, /*GC_SD_TX_RESET_ */
  /*117*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 4 }, /*GC_PORT_RESET_ */
  /*118*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*GC_XPCS_RESET_ */
  /*119*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*GC_F91_RESET_ */
  /*120*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*GC_SG_RESET_ */

    /* Global FEC CE Interrupt Mask */
  /*121*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 8 }, /*FEC_CE_INTERRUPT_MASK */

    /* Global FEC CE Interrupt Cause */
  /*122*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_CE_INTERRUPT_CAUSE_INT_SUM */
  /*123*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_CE_0 */
  /*124*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FEC_CE_1 */
  /*125*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FEC_CE_2 */
  /*126*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FEC_CE_3 */
  /*127*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FEC_CE_4 */
  /*128*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FEC_CE_5 */
  /*129*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FEC_CE_6 */
  /*130*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FEC_CE_7 */

    /* Global FEC NCE Interrupt Mask */
  /*131*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 8 }, /*FEC_NCE_INTERRUPT_MASK */

    /* Global FEC NCE Interrupt Cause */
  /*132*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_NCE_INTERRUPT_CAUSE_INT_SUM */
  /*133*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_NCE_0 */
  /*134*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FEC_NCE_1 */
  /*135*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FEC_NCE_2 */
  /*136*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FEC_NCE_3 */
  /*137*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FEC_NCE_4 */
  /*138*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FEC_NCE_5 */
  /*139*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FEC_NCE_6 */
  /*140*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FEC_NCE_7 */

    /* Port<0> Pause and Error Status */
  /*141*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x18, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_PAUSE_ON */
  /*142*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x18, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*P0_FF_RX_ERR_STAT */

    /* Global Clock Enable */
  /*143*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*MAC_CLK_EN */
  /*144*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*MAC_CMN_CLK_EN */
  /*145*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*FEC91_CLK_EN */
  /*146*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 2 }, /*XPCS_CLK_EN */
  /*147*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*PCS000_CLK_EN */
  /*148*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*SGREF_CLK_EN */
  /*149*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*MAC_MAC_CMN_CLK_EN */
  /*150*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*MAC_APP_CMN_CLK_EN */

    /* Global Reset Control2 */
  /*151*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GC_MAC_MAC_CMN_RESET_ */
  /*152*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*GC_MAC_APP_CMN_RESET_ */

    /* Port<0> Peer Delay */
  /*153*/  {/*baseAddr*/ 0x14c,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 30 }, /*P0_PEER_DELAY */
  /*154*/  {/*baseAddr*/ 0x14c,  /*offsetFormula*/ 0x4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_PEER_DELAY_VALID */

    /* Port<0> Xoff Status */
  /*155*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_XOFF_STATUS */

    /* Port<0> Pause Override */
  /*156*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_PAUSE_OVERRIDE_CTRL */
  /*157*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*P0_PAUSE_OVERRIDE_VAL */

    /* Port<0> Xoff Override */
  /*158*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_XOFF_OVERRIDE_CTRL */
  /*159*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*P0_XOFF_OVERRIDE_VAL */

    /* Global PCS Metal Fix */
  /*160*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*PCS_METAL_FIX */

    /* Port<0> Control1 */
  /*161*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_FORCE_LINK_STATUS_EN */
  /*162*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_FORCE_LINK_STATUS_DIS */
  /*163*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_FORCE_LPCS_LINK_STATUS_EN */
  /*164*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_FORCE_LPCS_LINK_STATUS_DIS */
  /*165*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_FORCE_XL_LINK_STATUS_EN */
  /*166*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_FORCE_XL_LINK_STATUS_DIS */
  /*167*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_FORCE_LINK_OK_EN */
  /*168*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_FORCE_LINK_OK_DIS */
  /*169*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_FORCE_TS_IF_CF_EN */
  /*170*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_FORCE_TS_IF_CF_DIS */
  /*171*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_FORCE_TS_IF_TSTF_EN */
  /*172*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_FORCE_TS_IF_TSTF_DIS */

    /* Port<0> TSU Modulo RX */
  /*173*/  {/*baseAddr*/ 0x2a0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_TSU_MODULO_RX */

    /* Port<0> TSU Control 0 */
  /*174*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*P0_TSU_RX_MODE */
  /*175*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/  3, /*fieldLen*/ 3 }, /*P0_TSU_TX_MODE */
  /*176*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/  6, /*fieldLen*/ 10 }, /*P0_TSU_DESKEW */
  /*177*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 3 }, /*P0_TSU_MII_MK_DLY */
  /*178*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 19, /*fieldLen*/ 3 }, /*P0_TSU_MII_CW_DLY */
  /*179*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 22, /*fieldLen*/ 5 }, /*P0_TSU_MII_TX_MK_CYC_DLY */
  /*180*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 27, /*fieldLen*/ 5 }, /*P0_TSU_MII_TX_CW_CYC_DLY */

    /* Port <0> TSU Control 2 */
  /*181*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_C_RX_MODE_OW */
  /*182*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_C_TX_MODE_OW */
  /*183*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_C_BLKS_PER_CLK_OW */
  /*184*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_C_MII_CW_DLY_OW */
  /*185*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_C_MII_MK_DLY_OW */
  /*186*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_C_DESKEW_OW */
  /*187*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_C_MODULO_RX_OW */
  /*188*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_C_MODULO_TX_OW */
  /*189*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_C_BLOCKTIME_INT_OW */
  /*190*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_C_BLOCKTIME_DEC_OW */
  /*191*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_C_MARKERTIME_INT_OW */
  /*192*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_C_MARKERTIME_DEC_OW */
  /*193*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_C_MII_TX_MK_CYC_DLY_OW */
  /*194*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_C_MII_TX_CW_CYC_DLY_OW */
  /*195*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_C_TSU_TX_SD_PERIOD_OW */

    /* Port<0> TSU Status */
  /*196*/  {/*baseAddr*/ 0x340,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_TSU_RX_READY */
  /*197*/  {/*baseAddr*/ 0x340,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_TSU_TX_READY */

    /* Port<0> Time Stamp Dispatcher Control 1 */
  /*198*/  {/*baseAddr*/ 0x240,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_AMD_CNT_LOW */
  /*199*/  {/*baseAddr*/ 0x240,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*P0_AMD_CNT_HIGH */

    /* Port<0> TSU Modulo TX */
  /*200*/  {/*baseAddr*/ 0x280,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_TSU_MODULO_TX */

    /* Port <0> TSU Control 3 */
  /*201*/  {/*baseAddr*/ 0x320,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 15 }, /*P0_C_TSU_TX_SD_PERIOD_S */

    /* Port<0> Status_2 */
  /*202*/  {/*baseAddr*/ 0x360,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*P0_SD_BIT_SLIP */

    /* Port<0> Time Stamp Dispatcher Control 0 */
  /*203*/  {/*baseAddr*/ 0x220,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_FSU_ENABLE */
  /*204*/  {/*baseAddr*/ 0x220,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_AMD_ENABLE */
  /*205*/  {/*baseAddr*/ 0x220,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 10 }, /*P0_FSU_OFFSET */
  /*206*/  {/*baseAddr*/ 0x220,  /*offsetFormula*/ 0x4, /*fieldStart*/ 12, /*fieldLen*/ 8 }, /*P0_FSU_RND_DELTA */
  /*207*/  {/*baseAddr*/ 0x220,  /*offsetFormula*/ 0x4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_AMD_CNT_TYPE_SEL */
  /*208*/  {/*baseAddr*/ 0x220,  /*offsetFormula*/ 0x4, /*fieldStart*/ 25, /*fieldLen*/ 5 }, /*P0_MINIMAL_TX_STOP_TOGGLE */

    /* Port<0> TSU Control 1 */
  /*209*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*P0_TSU_BLOCKTIME */
  /*210*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  5, /*fieldLen*/ 9 }, /*P0_TSU_BLOCKTIME_DEC */
  /*211*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 5 }, /*P0_TSU_MARKERTIME */
  /*212*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 21, /*fieldLen*/ 9 }, /*P0_TSU_MARKERTIME_DEC */
  /*213*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_TSU_BLKS_PER_CLK */

    /* Port<0> Time Stamp Dispatcher Control 2 */
  /*214*/  {/*baseAddr*/ 0x260,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*P0_MINIMAL_EMPTY_FOR_STOP_TX */
};



