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
* @file mvHwsHawkMtiExtBrRegDb.c
*
* @brief Hawk MTI EXT register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>



const MV_HWS_REG_ADDR_FIELD_STC hawkMtiExtBrRegDb[] = {
    /* Port<0> Control */
  /*0*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_TX_LOC_FAULT */
  /*1*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_TX_REM_FAULT */
  /*2*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_TX_LI_FAULT */
  /*3*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_TOD_SELECT */
  /*4*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_RX_PAUSE_CONTROL */
  /*5*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_RX_PAUSE_OW_VAL */
  /*6*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_PAUSE_802_3_REFLECT */
  /*7*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_LOOP_ENA */
  /*8*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_MASK_SW_RESET */
  /*9*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/  9, /*fieldLen*/ 6 }, /*P0_LED_PORT_NUM */
  /*10*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_LED_PORT_EN */
  /*11*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_FF_TX_CRC */
  /*12*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_FORCE_LINK_OK_EN */
  /*13*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_FORCE_LINK_OK_DIS */
  /*14*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/ 19, /*fieldLen*/ 4 }, /*P0_PORT_RES_SPEED */
  /*15*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_PORT_RES_SPEED_FROM_HW */
  /*16*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_LOOP_RX_BLOCK_OUT */
  /*17*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x18, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_LOOP_TX_RDY_OUT */

    /* Port<0> Status */
  /*18*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_LINK_OK */
  /*19*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_LINK_STATUS */
  /*20*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_LPCS_LINK_STATUS */
  /*21*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_LPCS_RX_SYNC */
  /*22*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_LPCS_AN_DONE */
  /*23*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/  5, /*fieldLen*/ 8 }, /*P0_MAC_RES_SPEED */
  /*24*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_ALIGN_DONE */
  /*25*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_BER_TIMER_DONE */
  /*26*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_HI_BER */
  /*27*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_RSFEC_ALIGNED */
  /*28*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_RX_TRAFFIC_IND */
  /*29*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_TX_TRAFFIC_IND */
  /*30*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_MAC_TX_EMPTY */
  /*31*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_MAC_TX_ISIDLE */
  /*32*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_MAC_TX_TS_FRM_OUT */
  /*33*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_PFC_MODE */
  /*34*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_FF_TX_SEPTY */
  /*35*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_FF_RX_EMPTY */
  /*36*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_FF_RX_DSAV */
  /*37*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_LINK_OK_CLEAN */

    /* Port<0> Marker Status */
  /*38*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x18, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_MARKER_INS_CNT */
  /*39*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x18, /*fieldStart*/ 16, /*fieldLen*/ 15 }, /*P0_MARKER_INS_CNT_100 */

    /* Port<0> Interrupt Cause */
  /*40*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PORT0_INTERRUPT_CAUSE_INT_SUM */
  /*41*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_LINK_OK_CHANGE */
  /*42*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_LINK_STATUS_CHANGE */
  /*43*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_LPCS_LINK_STATUS_CHANGE */
  /*44*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_MAC_LOC_FAULT */
  /*45*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_MAC_REM_FAULT */
  /*46*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_MAC_LI_FAULT */
  /*47*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_MAC_TX_UNDERFLOW */
  /*48*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_EMAC_TX_OVR_ERR */
  /*49*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_PMAC_TX_OVR_ERR */
  /*50*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_EMAC_RX_OVERRUN */
  /*51*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_PMAC_RX_OVERRUN */
  /*52*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_HI_BER */
  /*53*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_FF_RX_RDY */
  /*54*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_TSU_RX_DFF_ERR */
  /*55*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_TSU_RX_AM_ERR */
  /*56*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_TSU_TX_SYNC_ERR */
  /*57*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_TSD_NON_ACCURATE_PTP */

    /* Port<0> Interrupt Mask */
  /*58*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/  1, /*fieldLen*/ 17 }, /*P0_INTERRUPT_MASK */

    /* Global PMA Control */
  /*59*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GC_SD_N2 */
  /*60*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*GC_SD_8X */

    /* Global FEC Control */
  /*61*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GC_FEC_ENA */
  /*62*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*GC_FEC_ERR_ENA */
  /*63*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*GC_FEC91_ENA_IN */
  /*64*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 8 }, /*GC_KP_MODE_IN */

    /* Global Channel Control */
  /*65*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GC_MODE40_ENA_IN0 */
  /*66*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*GC_MODE40_ENA_IN4 */
  /*67*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN0 */
  /*68*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN2 */
  /*69*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN4 */
  /*70*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN6 */
  /*71*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*GC_PCS400_ENA_IN */
  /*72*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*GC_MLG_ENA_IN_0 */
  /*73*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*GC_MLG_ENA_IN_1 */
  /*74*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN0 */
  /*75*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN2 */
  /*76*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN4 */
  /*77*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN6 */
  /*78*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN0 */
  /*79*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN2 */
  /*80*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN4 */
  /*81*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN6 */
  /*82*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*MAC_ABU_WATCHDOG */
  /*83*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PCS_ABU_WATCHDOG */
  /*84*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 3 }, /*CYC_TO_STRETCH_MAC2APP */
  /*85*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 3 }, /*CYC_TO_STRETCH_MAC2REG */
  /*86*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 28, /*fieldLen*/ 3 }, /*CYC_TO_STRETCH_APP2REG */

    /* Global Clock Control */
  /*87*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GC_FAST_1LANE_MODE */
  /*88*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*GC_PACER_10G */
  /*89*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 4 }, /*GC_CFG_CLOCK_RATE */

    /* Global Amps Lock Status */
  /*90*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*AMPS_LOCK */
  /*91*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 16 }, /*PCS000_AMPS_LOCK */

    /* Global FEC Status */
  /*92*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FEC_LOCKED */

    /* Global FEC Error Status */
  /*93*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FEC_CERR */
  /*94*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*FEC_NCERR */

    /* Global Block Lock Status0 */
  /*95*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK0 */
  /*96*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*BLOCK_LOCK1 */
  /*97*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*BLOCK_LOCK2 */
  /*98*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*BLOCK_LOCK3 */
  /*99*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*BLOCK_LOCK4 */
  /*100*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*BLOCK_LOCK5 */
  /*101*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*BLOCK_LOCK6 */
  /*102*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*BLOCK_LOCK7 */
  /*103*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*BLOCK_LOCK8 */
  /*104*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*BLOCK_LOCK9 */
  /*105*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*BLOCK_LOCK10 */
  /*106*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*BLOCK_LOCK11 */
  /*107*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*BLOCK_LOCK12 */
  /*108*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*BLOCK_LOCK13 */
  /*109*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*BLOCK_LOCK14 */
  /*110*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*BLOCK_LOCK15 */
  /*111*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*BLOCK_LOCK16 */
  /*112*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*BLOCK_LOCK17 */
  /*113*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*BLOCK_LOCK18 */
  /*114*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*BLOCK_LOCK19 */

    /* Global Block Lock Status1 */
  /*115*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK20 */
  /*116*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*BLOCK_LOCK21 */
  /*117*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*BLOCK_LOCK22 */
  /*118*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*BLOCK_LOCK23 */
  /*119*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*BLOCK_LOCK24 */
  /*120*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*BLOCK_LOCK25 */
  /*121*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*BLOCK_LOCK26 */
  /*122*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*BLOCK_LOCK27 */
  /*123*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*BLOCK_LOCK28 */
  /*124*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*BLOCK_LOCK29 */
  /*125*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*BLOCK_LOCK30 */
  /*126*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*BLOCK_LOCK31 */
  /*127*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*BLOCK_LOCK32 */
  /*128*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*BLOCK_LOCK33 */
  /*129*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*BLOCK_LOCK34 */
  /*130*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*BLOCK_LOCK35 */
  /*131*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*BLOCK_LOCK36 */
  /*132*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*BLOCK_LOCK37 */
  /*133*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*BLOCK_LOCK38 */
  /*134*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*BLOCK_LOCK39 */

    /* Global 200G Mode Status */
  /*135*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MODE200_IND */
  /*136*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MODE200_8X26_IND */

    /* Seg Port<0> Control */
  /*137*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SEG_P0_TX_LOC_FAULT */
  /*138*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SEG_P0_TX_REM_FAULT */
  /*139*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/  2, /*fieldLen*/ 3 }, /*SEG_P0_TX_AM_SF */
  /*140*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*SEG_P0_TOD_SELECT */
  /*141*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SEG_P0_RX_PAUSE_CONTROL */
  /*142*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SEG_P0_RX_PAUSE_OW_VAL */
  /*143*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*SEG_P0_PAUSE_802_3_REFLECT */
  /*144*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SEG_P0_LOOP_ENA */
  /*145*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*SEG_P0_MASK_SW_RESET */
  /*146*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 11, /*fieldLen*/ 6 }, /*SEG_P0_LED_PORT_NUM */
  /*147*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*SEG_P0_LED_PORT_EN */
  /*148*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*SEG_P0_FF_TX_CRC */
  /*149*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*SEG_P0_FORCE_LINK_OK_EN */
  /*150*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*SEG_P0_FORCE_LINK_OK_DIS */
  /*151*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 21, /*fieldLen*/ 4 }, /*SEG_P0_PORT_RES_SPEED */
  /*152*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*SEG_P0_PORT_RES_SPEED_FROM_HW */
  /*153*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*SEG_P0_LOOP_RX_BLOCK_OUT */
  /*154*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*SEG_P0_LOOP_TX_RDY_OUT */

    /* Seg Port<0> Status */
  /*155*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SEG_P0_LINK_OK */
  /*156*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SEG_P0_LINK_STATUS */
  /*157*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/  2, /*fieldLen*/ 8 }, /*SEG_P0_MAC_RES_SPEED */
  /*158*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*SEG_P0_RX_TRAFFIC_IND */
  /*159*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*SEG_P0_TX_TRAFFIC_IND */
  /*160*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*SEG_P0_MAC_TX_EMPTY */
  /*161*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SEG_P0_MAC_TX_ISIDLE */
  /*162*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*SEG_P0_ALIGN_LOCK */
  /*163*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*SEG_P0_DEGRADE_SER */
  /*164*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*SEG_P0_HI_SER */
  /*165*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*SEG_P0_PFC_MODE */
  /*166*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 18, /*fieldLen*/ 3 }, /*SEG_P0_RX_AM_SF */
  /*167*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*SEG_P0_FF_TX_SEPTY */
  /*168*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*SEG_P0_FF_RX_EMPTY */
  /*169*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*SEG_P0_FF_RX_DSAV */
  /*170*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*SEG_P0_LINK_OK_CLEAN */
  /*171*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 25, /*fieldLen*/ 6 }, /*SEG_P0_SD_BIT_SLIP */

    /* Seg Port<0> Interrupt Cause */
  /*172*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SEG_PORT0_INTERRUPT_CAUSE_INT_SUM */
  /*173*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SEG_P0_LINK_OK_CHANGE */
  /*174*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*SEG_P0_LINK_STATUS_CHANGE */
  /*175*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*SEG_P0_MAC_LOC_FAULT */
  /*176*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*SEG_P0_MAC_REM_FAULT */
  /*177*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*SEG_P0_MAC_TX_UNDERFLOW */
  /*178*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SEG_P0_MAC_TX_OVR_ERR */
  /*179*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SEG_P0_RX_OVERRUN */
  /*180*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*SEG_P0_HI_SER */
  /*181*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SEG_P0_DEGRADE_SER */
  /*182*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*SEG_P0_FF_RX_RDY */
  /*183*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*SEG_P0_TSU_RX_DFF_ERR */
  /*184*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*SEG_P0_TSU_RX_AM_ERR */
  /*185*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SEG_P0_TSU_TX_SYNC_ERR */
  /*186*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*SEG_P0_TSD_NON_ACCURATE_PTP */

    /* Seg Port<0> Interrupt Mask */
  /*187*/  {/*baseAddr*/ 0x6c,  /*offsetFormula*/ 0x14, /*fieldStart*/  1, /*fieldLen*/ 14 }, /*SEG_P0_INTERRUPT_MASK */

    /* Global Last Violation */
  /*188*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*LAST_VIOLATION */

    /* Global Interrupt Mask */
  /*189*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 5 }, /*GLOBAL_INTERRUPT_MASK */

    /* Global Interrupt Cause */
  /*190*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_CAUSE_INT_SUM */
  /*191*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*ILLEGAL_ADDRESS_ACCESS */
  /*192*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*TOD0_SYNC_FIFO_UNDERRUN */
  /*193*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TOD0_SYNC_FIFO_OVERRUN */
  /*194*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*TOD1_SYNC_FIFO_UNDERRUN */
  /*195*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*TOD1_SYNC_FIFO_OVERRUN */

    /* Global Interrupt Summary Cause */
  /*196*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_SUMMARY_INT_SUM */
  /*197*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_INT_SUM */
  /*198*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P1_INT_SUM */
  /*199*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P2_INT_SUM */
  /*200*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P3_INT_SUM */
  /*201*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P4_INT_SUM */
  /*202*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P5_INT_SUM */
  /*203*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P6_INT_SUM */
  /*204*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P7_INT_SUM */
  /*205*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SEG_P0_INT_SUM */
  /*206*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*SEG_P1_INT_SUM */
  /*207*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*GLOBAL_INT_SUM */
  /*208*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*FEC_CE_INT_SUM */
  /*209*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*FEC_NCE_INT_SUM */

    /* Global Interrupt Summary Mask */
  /*210*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 13 }, /*GLOBAL_INTERRUPT_SUMMARY_MASK */

    /* Global Reset Control */
  /*211*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GC_SD_RX_RESET_ */
  /*212*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*GC_SD_TX_RESET_ */
  /*213*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*GC_PORT_RESET_ */
  /*214*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 2 }, /*GC_SEG_PORT_RESET_ */
  /*215*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*GC_PCS000_RESET_ */
  /*216*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 27, /*fieldLen*/ 2 }, /*GC_XPCS_RESET_ */
  /*217*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*GC_F91_RESET_ */
  /*218*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*GC_SG_RESET_ */
  /*219*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*GC_REF_RESET_ */

    /* Global FEC CE Interrupt Mask */
  /*220*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 16 }, /*FEC_CE_INTERRUPT_MASK */

    /* Global FEC CE Interrupt Cause */
  /*221*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_CE_INTERRUPT_CAUSE_INT_SUM */
  /*222*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_CE_0 */
  /*223*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FEC_CE_1 */
  /*224*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FEC_CE_2 */
  /*225*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FEC_CE_3 */
  /*226*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FEC_CE_4 */
  /*227*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FEC_CE_5 */
  /*228*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FEC_CE_6 */
  /*229*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FEC_CE_7 */
  /*230*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*FEC_CE_8 */
  /*231*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*FEC_CE_9 */
  /*232*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*FEC_CE_10 */
  /*233*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*FEC_CE_11 */
  /*234*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*FEC_CE_12 */
  /*235*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*FEC_CE_13 */
  /*236*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*FEC_CE_14 */
  /*237*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*FEC_CE_15 */

    /* Global FEC NCE Interrupt Mask */
  /*238*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 16 }, /*FEC_NCE_INTERRUPT_MASK */

    /* Global FEC NCE Interrupt Cause */
  /*239*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_NCE_INTERRUPT_CAUSE_INT_SUM */
  /*240*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_NCE_0 */
  /*241*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FEC_NCE_1 */
  /*242*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FEC_NCE_2 */
  /*243*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FEC_NCE_3 */
  /*244*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FEC_NCE_4 */
  /*245*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FEC_NCE_5 */
  /*246*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FEC_NCE_6 */
  /*247*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FEC_NCE_7 */
  /*248*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*FEC_NCE_8 */
  /*249*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*FEC_NCE_9 */
  /*250*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*FEC_NCE_10 */
  /*251*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*FEC_NCE_11 */
  /*252*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*FEC_NCE_12 */
  /*253*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*FEC_NCE_13 */
  /*254*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*FEC_NCE_14 */
  /*255*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*FEC_NCE_15 */

    /* Port<0> Pause and Error Status */
  /*256*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x18, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_PAUSE_ON */
  /*257*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x18, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*P0_FF_RX_ERR_STAT */

    /* Seg Port<0> Pause and Err Stat */
  /*258*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x14, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEG_P0_PAUSE_ON */
  /*259*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x14, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*SEG_P0_FF_RX_ERR_STAT */

    /* Global Clock Enable */
  /*260*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MAC_CLK_EN */
  /*261*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 2 }, /*SEG_MAC_CLK_EN */
  /*262*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*MAC_CMN_CLK_EN */
  /*263*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*FEC91_CLK_EN */
  /*264*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 2 }, /*XPCS_CLK_EN */
  /*265*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*PCS000_CLK_EN */
  /*266*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*SGREF_CLK_EN */
  /*267*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*MAC_MAC_CMN_CLK_EN */
  /*268*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*MAC_APP_CMN_CLK_EN */

    /* Global Reset Control2 */
  /*269*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GC_MAC_MAC_CMN_RESET_ */
  /*270*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*GC_MAC_APP_CMN_RESET_ */

    /* Seg Port<0> Peer Delay */
  /*271*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 30 }, /*SEG_P0_PEER_DELAY */
  /*272*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*SEG_P0_PEER_DELAY_VALID */

    /* Port<0> Peer Delay */
  /*273*/  {/*baseAddr*/ 0x14c,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 30 }, /*P0_PEER_DELAY */
  /*274*/  {/*baseAddr*/ 0x14c,  /*offsetFormula*/ 0x4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_PEER_DELAY_VALID */

    /* Seg Port<0> Xoff Status */
  /*275*/  {/*baseAddr*/ 0x16c,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEG_P0_XOFF_STATUS */

    /* Seg Port<0> Pause Override */
  /*276*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEG_P0_PAUSE_OVERRIDE_CTRL */
  /*277*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*SEG_P0_PAUSE_OVERRIDE_VAL */

    /* Seg Port<0> Xoff Override */
  /*278*/  {/*baseAddr*/ 0x17c,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEG_P0_XOFF_OVERRIDE_CTRL */
  /*279*/  {/*baseAddr*/ 0x17c,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*SEG_P0_XOFF_OVERRIDE_VAL */

    /* Port<0> Xoff Status */
  /*280*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_XOFF_STATUS */

    /* Port<0> Pause Override */
  /*281*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_PAUSE_OVERRIDE_CTRL */
  /*282*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*P0_PAUSE_OVERRIDE_VAL */

    /* Port<0> Xoff Override */
  /*283*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_XOFF_OVERRIDE_CTRL */
  /*284*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*P0_XOFF_OVERRIDE_VAL */

    /* Global MAC Metal Fix */
  /*285*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MAC_METAL_FIX */

    /* Global PCS Metal Fix */
  /*286*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*PCS_METAL_FIX */

    /* Port<0> Time Stamp Dispatcher Control 0 */
  /*287*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_FSU_ENABLE */
  /*288*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_AMD_ENABLE */
  /*289*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 10 }, /*P0_FSU_OFFSET */
  /*290*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x4, /*fieldStart*/ 12, /*fieldLen*/ 8 }, /*P0_FSU_RND_DELTA */
  /*291*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_AMD_CNT_TYPE_SEL */
  /*292*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x4, /*fieldStart*/ 25, /*fieldLen*/ 5 }, /*P0_MINIMAL_TX_STOP_TOGGLE */

    /* Port<0> Time Stamp Dispatcher Control 1 */
  /*293*/  {/*baseAddr*/ 0x220,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_AMD_CNT_LOW */
  /*294*/  {/*baseAddr*/ 0x220,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*P0_AMD_CNT_HIGH */

    /* Seg Port<0> Time Stamp Dispatcher Control 0 */
  /*295*/  {/*baseAddr*/ 0x260,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SEG_P0_FSU_ENABLE */
  /*296*/  {/*baseAddr*/ 0x260,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SEG_P0_AMD_ENABLE */
  /*297*/  {/*baseAddr*/ 0x260,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 10 }, /*SEG_P0_FSU_OFFSET */
  /*298*/  {/*baseAddr*/ 0x260,  /*offsetFormula*/ 0x4, /*fieldStart*/ 12, /*fieldLen*/ 8 }, /*SEG_P0_FSU_RND_DELTA */
  /*299*/  {/*baseAddr*/ 0x260,  /*offsetFormula*/ 0x4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*SEG_P0_AMD_CNT_TYPE_SEL */
  /*300*/  {/*baseAddr*/ 0x260,  /*offsetFormula*/ 0x4, /*fieldStart*/ 25, /*fieldLen*/ 5 }, /*SEG_P0_MINIMAL_TX_STOP_TOGGLE */

    /* Seg Port<0> Time Stamp Dispatcher Control 1 */
  /*301*/  {/*baseAddr*/ 0x268,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEG_P0_AMD_CNT_LOW */
  /*302*/  {/*baseAddr*/ 0x268,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*SEG_P0_AMD_CNT_HIGH */

    /* Port<0> TSU Control 0 */
  /*303*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*P0_TSU_RX_MODE */
  /*304*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/  3, /*fieldLen*/ 3 }, /*P0_TSU_TX_MODE */
  /*305*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/  6, /*fieldLen*/ 10 }, /*P0_TSU_DESKEW */
  /*306*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 3 }, /*P0_TSU_MII_MK_DLY */
  /*307*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 19, /*fieldLen*/ 3 }, /*P0_TSU_MII_CW_DLY */
  /*308*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 22, /*fieldLen*/ 5 }, /*P0_TSU_MII_TX_MK_CYC_DLY */
  /*309*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 27, /*fieldLen*/ 5 }, /*P0_TSU_MII_TX_CW_CYC_DLY */

    /* Port<0> TSU Modulo TX */
  /*310*/  {/*baseAddr*/ 0x280,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_TSU_MODULO_TX */

    /* Port<0> TSU Control 1 */
  /*311*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*P0_TSU_BLOCKTIME */
  /*312*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  5, /*fieldLen*/ 9 }, /*P0_TSU_BLOCKTIME_DEC */
  /*313*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 5 }, /*P0_TSU_MARKERTIME */
  /*314*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 21, /*fieldLen*/ 9 }, /*P0_TSU_MARKERTIME_DEC */
  /*315*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_TSU_BLKS_PER_CLK */

    /* Port<0> TSU Status */
  /*316*/  {/*baseAddr*/ 0x340,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_TSU_RX_READY */
  /*317*/  {/*baseAddr*/ 0x340,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_TSU_TX_READY */

    /* Port<0> TSU Modulo RX */
  /*318*/  {/*baseAddr*/ 0x2a0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_TSU_MODULO_RX */

    /* Port<0> TSU Control 2 */
  /*319*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_C_RX_MODE_OW */
  /*320*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_C_TX_MODE_OW */
  /*321*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_C_BLKS_PER_CLK_OW */
  /*322*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_C_MII_CW_DLY_OW */
  /*323*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_C_MII_MK_DLY_OW */
  /*324*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_C_DESKEW_OW */
  /*325*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_C_MODULO_RX_OW */
  /*326*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_C_MODULO_TX_OW */
  /*327*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_C_BLOCKTIME_INT_OW */
  /*328*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_C_BLOCKTIME_DEC_OW */
  /*329*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_C_MARKERTIME_INT_OW */
  /*330*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_C_MARKERTIME_DEC_OW */
  /*331*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_C_MII_TX_MK_CYC_DLY_OW */
  /*332*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_C_MII_TX_CW_CYC_DLY_OW */
  /*333*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_C_TSU_TX_SD_PERIOD_OW */

    /* Port<0> Status_2 */
  /*334*/  {/*baseAddr*/ 0x360,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*P0_SD_BIT_SLIP */

    /* Port<0> TSU Control 3 */
  /*335*/  {/*baseAddr*/ 0x320,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 15 }, /*P0_C_TSU_TX_SD_PERIOD_S */

    /* Port<0> PMAC Control */
  /*336*/  {/*baseAddr*/ 0x380,  /*offsetFormula*/ 0x4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_PREEMPTION_LOOP_ENA */
  /*337*/  {/*baseAddr*/ 0x380,  /*offsetFormula*/ 0x4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_PREEMPTION_LOOP_RX_BLOCK_OUT */
  /*338*/  {/*baseAddr*/ 0x380,  /*offsetFormula*/ 0x4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_PREEMPTION_LOOP_TX_RDY_OUT */
  /*339*/  {/*baseAddr*/ 0x380,  /*offsetFormula*/ 0x4, /*fieldStart*/ 26, /*fieldLen*/ 2 }, /*P0_LED_INDICATION_CTRL */
  /*340*/  {/*baseAddr*/ 0x380,  /*offsetFormula*/ 0x4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*P0_TX_HOLD_REQ */
  /*341*/  {/*baseAddr*/ 0x380,  /*offsetFormula*/ 0x4, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*P0_TX_HOLD_REQ_FROM_HW */

    /* Port<0> PMAC Status */
  /*342*/  {/*baseAddr*/ 0x3a0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PREEMPTION_ENABLED */
  /*343*/  {/*baseAddr*/ 0x3a0,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_EXPRESS_MAC_ACTIVE */
  /*344*/  {/*baseAddr*/ 0x3a0,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_PREEMPTION_MAC_ACTIVE */
  /*345*/  {/*baseAddr*/ 0x3a0,  /*offsetFormula*/ 0x4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_PREEMPTION_MAC_ACTIVE_OR_FRAME_PREEMPTED */
  /*346*/  {/*baseAddr*/ 0x3a0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_PREEMPTION_MAC_TX_EMPTY */
  /*347*/  {/*baseAddr*/ 0x3a0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_FF_PREEMPTION_TX_SEPTY */
  /*348*/  {/*baseAddr*/ 0x3a0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_FF_PREEMPTION_RX_EMPTY */
  /*349*/  {/*baseAddr*/ 0x3a0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_FF_PREEMPTION_RX_DSAV */

    /* Global Clock Divider Control */
  /*350*/  {/*baseAddr*/ 0x3c0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 21 }, /*CLK_RATIO */

    /* Port<0> Time Stamp Dispatcher Control 2 */
  /*351*/  {/*baseAddr*/ 0x240,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*P0_MINIMAL_EMPTY_FOR_STOP_TX */
  /*352*/  {/*baseAddr*/ 0x240,  /*offsetFormula*/ 0x4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_FSU_PREEMPTION_HOLD_EN */
  /*353*/  {/*baseAddr*/ 0x240,  /*offsetFormula*/ 0x4, /*fieldStart*/  7, /*fieldLen*/ 4 }, /*P0_HOLD_KEPT_ACTIVE */

    /* Seg Port<0> Time Stamp Dispatcher Control 2 */
  /*354*/  {/*baseAddr*/ 0x270,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*SEG_P0_MINIMAL_EMPTY_FOR_STOP_TX */
  /*355*/  {/*baseAddr*/ 0x270,  /*offsetFormula*/ 0x4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SEG_P0_FSU_PREEMPTION_HOLD_EN */
  /*356*/  {/*baseAddr*/ 0x270,  /*offsetFormula*/ 0x4, /*fieldStart*/  7, /*fieldLen*/ 4 }, /*SEG_P0_HOLD_KEPT_ACTIVE */
};



