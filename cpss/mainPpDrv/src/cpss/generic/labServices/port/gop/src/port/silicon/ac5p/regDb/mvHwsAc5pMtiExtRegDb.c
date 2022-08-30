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
* @file mvHwsHawkMtiExtRegDb.c
*
* @brief Hawk MTI EXT register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>



const MV_HWS_REG_ADDR_FIELD_STC hawkMtiExtRegDb[] = {
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
  /*48*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_MAC_TX_OVR_ERR */
  /*49*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_RX_OVERRUN */
  /*50*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_HI_BER */
  /*51*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_FF_RX_RDY */
  /*52*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_TSU_RX_DFF_ERR */
  /*53*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_TSU_RX_AM_ERR */
  /*54*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_TSU_TX_SYNC_ERR */

    /* Port<0> Interrupt Mask */
  /*55*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/  1, /*fieldLen*/ 14 }, /*P0_INTERRUPT_MASK */

    /* Global PMA Control */
  /*56*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GC_SD_N2 */
  /*57*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*GC_SD_8X */

    /* Global FEC Control */
  /*58*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GC_FEC_ENA */
  /*59*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*GC_FEC_ERR_ENA */
  /*60*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*GC_FEC91_ENA_IN */
  /*61*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 8 }, /*GC_KP_MODE_IN */

    /* Global Channel Control */
  /*62*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GC_MODE40_ENA_IN0 */
  /*63*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*GC_MODE40_ENA_IN4 */
  /*64*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN0 */
  /*65*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN2 */
  /*66*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN4 */
  /*67*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN6 */
  /*68*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*GC_PCS400_ENA_IN */
  /*69*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*GC_MLG_ENA_IN_0 */
  /*70*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*GC_MLG_ENA_IN_1 */
  /*71*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN0 */
  /*72*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN2 */
  /*73*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN4 */
  /*74*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN6 */
  /*75*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN0 */
  /*76*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN2 */
  /*77*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN4 */
  /*78*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN6 */
  /*79*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*MAC_ABU_WATCHDOG */
  /*80*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PCS_ABU_WATCHDOG */

    /* Global Clock Control */
  /*81*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GC_FAST_1LANE_MODE */
  /*82*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*GC_PACER_10G */
  /*83*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 4 }, /*GC_CFG_CLOCK_RATE */

    /* Global Amps Lock Status */
  /*84*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*AMPS_LOCK */
  /*85*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 16 }, /*PCS000_AMPS_LOCK */

    /* Global FEC Status */
  /*86*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FEC_LOCKED */

    /* Global FEC Error Status */
  /*87*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FEC_CERR */
  /*88*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*FEC_NCERR */

    /* Global Block Lock Status0 */
  /*89*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK0 */
  /*90*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*BLOCK_LOCK1 */
  /*91*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*BLOCK_LOCK2 */
  /*92*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*BLOCK_LOCK3 */
  /*93*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*BLOCK_LOCK4 */
  /*94*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*BLOCK_LOCK5 */
  /*95*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*BLOCK_LOCK6 */
  /*96*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*BLOCK_LOCK7 */
  /*97*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*BLOCK_LOCK8 */
  /*98*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*BLOCK_LOCK9 */
  /*99*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*BLOCK_LOCK10 */
  /*100*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*BLOCK_LOCK11 */
  /*101*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*BLOCK_LOCK12 */
  /*102*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*BLOCK_LOCK13 */
  /*103*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*BLOCK_LOCK14 */
  /*104*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*BLOCK_LOCK15 */
  /*105*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*BLOCK_LOCK16 */
  /*106*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*BLOCK_LOCK17 */
  /*107*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*BLOCK_LOCK18 */
  /*108*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*BLOCK_LOCK19 */

    /* Global Block Lock Status1 */
  /*109*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK20 */
  /*110*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*BLOCK_LOCK21 */
  /*111*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*BLOCK_LOCK22 */
  /*112*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*BLOCK_LOCK23 */
  /*113*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*BLOCK_LOCK24 */
  /*114*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*BLOCK_LOCK25 */
  /*115*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*BLOCK_LOCK26 */
  /*116*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*BLOCK_LOCK27 */
  /*117*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*BLOCK_LOCK28 */
  /*118*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*BLOCK_LOCK29 */
  /*119*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*BLOCK_LOCK30 */
  /*120*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*BLOCK_LOCK31 */
  /*121*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*BLOCK_LOCK32 */
  /*122*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*BLOCK_LOCK33 */
  /*123*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*BLOCK_LOCK34 */
  /*124*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*BLOCK_LOCK35 */
  /*125*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*BLOCK_LOCK36 */
  /*126*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*BLOCK_LOCK37 */
  /*127*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*BLOCK_LOCK38 */
  /*128*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*BLOCK_LOCK39 */

    /* Global 200G Mode Status */
  /*129*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MODE200_IND */
  /*130*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MODE200_8X26_IND */

    /* Seg Port<0> Control */
  /*131*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SEG_P0_TX_LOC_FAULT */
  /*132*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SEG_P0_TX_REM_FAULT */
  /*133*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/  2, /*fieldLen*/ 3 }, /*SEG_P0_TX_AM_SF */
  /*134*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*SEG_P0_TOD_SELECT */
  /*135*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SEG_P0_RX_PAUSE_CONTROL */
  /*136*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SEG_P0_RX_PAUSE_OW_VAL */
  /*137*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*SEG_P0_PAUSE_802_3_REFLECT */
  /*138*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SEG_P0_LOOP_ENA */
  /*139*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*SEG_P0_MASK_SW_RESET */
  /*140*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 11, /*fieldLen*/ 6 }, /*SEG_P0_LED_PORT_NUM */
  /*141*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*SEG_P0_LED_PORT_EN */
  /*142*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*SEG_P0_FF_TX_CRC */
  /*143*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*SEG_P0_FORCE_LINK_OK_EN */
  /*144*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*SEG_P0_FORCE_LINK_OK_DIS */
  /*145*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 21, /*fieldLen*/ 4 }, /*SEG_P0_PORT_RES_SPEED */
  /*146*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*SEG_P0_PORT_RES_SPEED_FROM_HW */
  /*147*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*SEG_P0_LOOP_RX_BLOCK_OUT */
  /*148*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x14, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*SEG_P0_LOOP_TX_RDY_OUT */

    /* Seg Port<0> Status */
  /*149*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SEG_P0_LINK_OK */
  /*150*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SEG_P0_LINK_STATUS */
  /*151*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/  2, /*fieldLen*/ 8 }, /*SEG_P0_MAC_RES_SPEED */
  /*152*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*SEG_P0_RX_TRAFFIC_IND */
  /*153*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*SEG_P0_TX_TRAFFIC_IND */
  /*154*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*SEG_P0_MAC_TX_EMPTY */
  /*155*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SEG_P0_MAC_TX_ISIDLE */
  /*156*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*SEG_P0_ALIGN_LOCK */
  /*157*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*SEG_P0_DEGRADE_SER */
  /*158*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*SEG_P0_HI_SER */
  /*159*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*SEG_P0_PFC_MODE */
  /*160*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 18, /*fieldLen*/ 3 }, /*SEG_P0_RX_AM_SF */
  /*161*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*SEG_P0_FF_TX_SEPTY */
  /*162*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*SEG_P0_FF_RX_EMPTY */
  /*163*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*SEG_P0_FF_RX_DSAV */
  /*164*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*SEG_P0_LINK_OK_CLEAN */
  /*165*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x14, /*fieldStart*/ 25, /*fieldLen*/ 6 }, /*SEG_P0_SD_BIT_SLIP */

    /* Seg Port<0> Interrupt Cause */
  /*166*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SEG_PORT0_INTERRUPT_CAUSE_INT_SUM */
  /*167*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SEG_P0_LINK_OK_CHANGE */
  /*168*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*SEG_P0_LINK_STATUS_CHANGE */
  /*169*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*SEG_P0_MAC_LOC_FAULT */
  /*170*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*SEG_P0_MAC_REM_FAULT */
  /*171*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*SEG_P0_MAC_TX_UNDERFLOW */
  /*172*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SEG_P0_MAC_TX_OVR_ERR */
  /*173*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SEG_P0_RX_OVERRUN */
  /*174*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*SEG_P0_HI_SER */
  /*175*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SEG_P0_DEGRADE_SER */
  /*176*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*SEG_P0_FF_RX_RDY */
  /*177*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*SEG_P0_TSU_RX_DFF_ERR */
  /*178*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*SEG_P0_TSU_RX_AM_ERR */
  /*179*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x14, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SEG_P0_TSU_TX_SYNC_ERR */

    /* Seg Port<0> Interrupt Mask */
  /*180*/  {/*baseAddr*/ 0x6c,  /*offsetFormula*/ 0x14, /*fieldStart*/  1, /*fieldLen*/ 13 }, /*SEG_P0_INTERRUPT_MASK */

    /* Global Last Violation */
  /*181*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*LAST_VIOLATION */

    /* Global Interrupt Mask */
  /*182*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 5 }, /*GLOBAL_INTERRUPT_MASK */

    /* Global Interrupt Cause */
  /*183*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_CAUSE_INT_SUM */
  /*184*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*ILLEGAL_ADDRESS_ACCESS */
  /*185*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*TOD0_SYNC_FIFO_UNDERRUN */
  /*186*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TOD0_SYNC_FIFO_OVERRUN */
  /*187*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*TOD1_SYNC_FIFO_UNDERRUN */
  /*188*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*TOD1_SYNC_FIFO_OVERRUN */

    /* Global Interrupt Summary Cause */
  /*189*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_SUMMARY_INT_SUM */
  /*190*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_INT_SUM */
  /*191*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P1_INT_SUM */
  /*192*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P2_INT_SUM */
  /*193*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P3_INT_SUM */
  /*194*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P4_INT_SUM */
  /*195*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P5_INT_SUM */
  /*196*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P6_INT_SUM */
  /*197*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P7_INT_SUM */
  /*198*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SEG_P0_INT_SUM */
  /*199*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*SEG_P1_INT_SUM */
  /*200*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*GLOBAL_INT_SUM */
  /*201*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*FEC_CE_INT_SUM */
  /*202*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*FEC_NCE_INT_SUM */

    /* Global Interrupt Summary Mask */
  /*203*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 13 }, /*GLOBAL_INTERRUPT_SUMMARY_MASK */

    /* Global Reset Control */
  /*204*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GC_SD_RX_RESET_ */
  /*205*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*GC_SD_TX_RESET_ */
  /*206*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*GC_PORT_RESET_ */
  /*207*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 2 }, /*GC_SEG_PORT_RESET_ */
  /*208*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*GC_PCS000_RESET_ */
  /*209*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 27, /*fieldLen*/ 2 }, /*GC_XPCS_RESET_ */
  /*210*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*GC_F91_RESET_ */
  /*211*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*GC_SG_RESET_ */
  /*212*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*GC_REF_RESET_ */

    /* Global FEC CE Interrupt Mask */
  /*213*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 16 }, /*FEC_CE_INTERRUPT_MASK */

    /* Global FEC CE Interrupt Cause */
  /*214*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_CE_INTERRUPT_CAUSE_INT_SUM */
  /*215*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_CE_0 */
  /*216*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FEC_CE_1 */
  /*217*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FEC_CE_2 */
  /*218*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FEC_CE_3 */
  /*219*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FEC_CE_4 */
  /*220*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FEC_CE_5 */
  /*221*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FEC_CE_6 */
  /*222*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FEC_CE_7 */
  /*223*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*FEC_CE_8 */
  /*224*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*FEC_CE_9 */
  /*225*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*FEC_CE_10 */
  /*226*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*FEC_CE_11 */
  /*227*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*FEC_CE_12 */
  /*228*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*FEC_CE_13 */
  /*229*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*FEC_CE_14 */
  /*230*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*FEC_CE_15 */

    /* Global FEC NCE Interrupt Mask */
  /*231*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 16 }, /*FEC_NCE_INTERRUPT_MASK */

    /* Global FEC NCE Interrupt Cause */
  /*232*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_NCE_INTERRUPT_CAUSE_INT_SUM */
  /*233*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_NCE_0 */
  /*234*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FEC_NCE_1 */
  /*235*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FEC_NCE_2 */
  /*236*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FEC_NCE_3 */
  /*237*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FEC_NCE_4 */
  /*238*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FEC_NCE_5 */
  /*239*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FEC_NCE_6 */
  /*240*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FEC_NCE_7 */
  /*241*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*FEC_NCE_8 */
  /*242*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*FEC_NCE_9 */
  /*243*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*FEC_NCE_10 */
  /*244*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*FEC_NCE_11 */
  /*245*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*FEC_NCE_12 */
  /*246*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*FEC_NCE_13 */
  /*247*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*FEC_NCE_14 */
  /*248*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*FEC_NCE_15 */

    /* Port<0> Pause and Error Status */
  /*249*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x18, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_PAUSE_ON */
  /*250*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x18, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*P0_FF_RX_ERR_STAT */

    /* Seg Port<0> Pause and Err Stat */
  /*251*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x14, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEG_P0_PAUSE_ON */
  /*252*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x14, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*SEG_P0_FF_RX_ERR_STAT */

    /* Global Clock Enable */
  /*253*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MAC_CLK_EN */
  /*254*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 2 }, /*SEG_MAC_CLK_EN */
  /*255*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*MAC_CMN_CLK_EN */
  /*256*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*FEC91_CLK_EN */
  /*257*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 2 }, /*XPCS_CLK_EN */
  /*258*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*PCS000_CLK_EN */
  /*259*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*SGREF_CLK_EN */
  /*260*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*MAC_MAC_CMN_CLK_EN */
  /*261*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*MAC_APP_CMN_CLK_EN */

    /* Global Reset Control2 */
  /*262*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GC_MAC_MAC_CMN_RESET_ */
  /*263*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*GC_MAC_APP_CMN_RESET_ */

    /* Seg Port<0> Peer Delay */
  /*264*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 30 }, /*SEG_P0_PEER_DELAY */
  /*265*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*SEG_P0_PEER_DELAY_VALID */

    /* Port<0> Peer Delay */
  /*266*/  {/*baseAddr*/ 0x14c,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 30 }, /*P0_PEER_DELAY */
  /*267*/  {/*baseAddr*/ 0x14c,  /*offsetFormula*/ 0x4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_PEER_DELAY_VALID */

    /* Seg Port<0> Xoff Status */
  /*268*/  {/*baseAddr*/ 0x16c,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEG_P0_XOFF_STATUS */

    /* Seg Port<0> Pause Override */
  /*269*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEG_P0_PAUSE_OVERRIDE_CTRL */
  /*270*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*SEG_P0_PAUSE_OVERRIDE_VAL */

    /* Seg Port<0> Xoff Override */
  /*271*/  {/*baseAddr*/ 0x17c,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEG_P0_XOFF_OVERRIDE_CTRL */
  /*272*/  {/*baseAddr*/ 0x17c,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*SEG_P0_XOFF_OVERRIDE_VAL */

    /* Port<0> Xoff Status */
  /*273*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_XOFF_STATUS */

    /* Port<0> Pause Override */
  /*274*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_PAUSE_OVERRIDE_CTRL */
  /*275*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*P0_PAUSE_OVERRIDE_VAL */

    /* Port<0> Xoff Override */
  /*276*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_XOFF_OVERRIDE_CTRL */
  /*277*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*P0_XOFF_OVERRIDE_VAL */

    /* Global MAC Metal Fix */
  /*278*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MAC_METAL_FIX */

    /* Global PCS Metal Fix */
  /*279*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*PCS_METAL_FIX */

    /* Port<0> Time Stamp Dispatcher Control 0 */
  /*280*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_FSU_ENABLE */
  /*281*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_AMD_ENABLE */
  /*282*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 10 }, /*P0_FSU_OFFSET */
  /*283*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x4, /*fieldStart*/ 12, /*fieldLen*/ 8 }, /*P0_FSU_RND_DELTA */
  /*284*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_AMD_CNT_TYPE_SEL */
  /*285*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x4, /*fieldStart*/ 25, /*fieldLen*/ 5 }, /*P0_MINIMAL_TX_STOP_TOGGLE */

    /* Port<0> Time Stamp Dispatcher Control 1 */
  /*286*/  {/*baseAddr*/ 0x220,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_AMD_CNT_LOW */
  /*287*/  {/*baseAddr*/ 0x220,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*P0_AMD_CNT_HIGH */

    /* Seg Port<0> Time Stamp Dispatcher Control 0 */
  /*288*/  {/*baseAddr*/ 0x240,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SEG_P0_FSU_ENABLE */
  /*289*/  {/*baseAddr*/ 0x240,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SEG_P0_AMD_ENABLE */
  /*290*/  {/*baseAddr*/ 0x240,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 10 }, /*SEG_P0_FSU_OFFSET */
  /*291*/  {/*baseAddr*/ 0x240,  /*offsetFormula*/ 0x4, /*fieldStart*/ 12, /*fieldLen*/ 8 }, /*SEG_P0_FSU_RND_DELTA */
  /*292*/  {/*baseAddr*/ 0x240,  /*offsetFormula*/ 0x4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*SEG_P0_AMD_CNT_TYPE_SEL */
  /*293*/  {/*baseAddr*/ 0x240,  /*offsetFormula*/ 0x4, /*fieldStart*/ 25, /*fieldLen*/ 5 }, /*SEG_P0_MINIMAL_TX_STOP_TOGGLE */

    /* Seg Port<0> Time Stamp Dispatcher Control 1 */
  /*294*/  {/*baseAddr*/ 0x248,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEG_P0_AMD_CNT_LOW */
  /*295*/  {/*baseAddr*/ 0x248,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*SEG_P0_AMD_CNT_HIGH */

    /* Port<0> TSU Control 0 */
  /*296*/  {/*baseAddr*/ 0x2a0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*P0_TSU_RX_MODE */
  /*297*/  {/*baseAddr*/ 0x2a0,  /*offsetFormula*/ 0x4, /*fieldStart*/  3, /*fieldLen*/ 3 }, /*P0_TSU_TX_MODE */
  /*298*/  {/*baseAddr*/ 0x2a0,  /*offsetFormula*/ 0x4, /*fieldStart*/  6, /*fieldLen*/ 10 }, /*P0_TSU_DESKEW */
  /*299*/  {/*baseAddr*/ 0x2a0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 3 }, /*P0_TSU_MII_MK_DLY */
  /*300*/  {/*baseAddr*/ 0x2a0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 19, /*fieldLen*/ 3 }, /*P0_TSU_MII_CW_DLY */
  /*301*/  {/*baseAddr*/ 0x2a0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 22, /*fieldLen*/ 5 }, /*P0_TSU_MII_TX_MK_CYC_DLY */
  /*302*/  {/*baseAddr*/ 0x2a0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 27, /*fieldLen*/ 5 }, /*P0_TSU_MII_TX_CW_CYC_DLY */

    /* Port<0> TSU Control 1 */
  /*303*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*P0_TSU_BLOCKTIME */
  /*304*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/  5, /*fieldLen*/ 9 }, /*P0_TSU_BLOCKTIME_DEC */
  /*305*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 5 }, /*P0_TSU_MARKERTIME */
  /*306*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 21, /*fieldLen*/ 9 }, /*P0_TSU_MARKERTIME_DEC */
  /*307*/  {/*baseAddr*/ 0x2c0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_TSU_BLKS_PER_CLK */

    /* Port<0> TSU Modulo TX */
  /*308*/  {/*baseAddr*/ 0x260,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_TSU_MODULO_TX */

    /* Port<0> TSU Status */
  /*309*/  {/*baseAddr*/ 0x320,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_TSU_RX_READY */
  /*310*/  {/*baseAddr*/ 0x320,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_TSU_TX_READY */

    /* Port<0> TSU Modulo RX */
  /*311*/  {/*baseAddr*/ 0x280,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_TSU_MODULO_RX */

    /* Port<0> TSU Control 2 */
  /*312*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_C_RX_MODE_OW */
  /*313*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_C_TX_MODE_OW */
  /*314*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_C_BLKS_PER_CLK_OW */
  /*315*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_C_MII_CW_DLY_OW */
  /*316*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_C_MII_MK_DLY_OW */
  /*317*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_C_DESKEW_OW */
  /*318*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_C_MODULO_RX_OW */
  /*319*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_C_MODULO_TX_OW */
  /*320*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_C_BLOCKTIME_INT_OW */
  /*321*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_C_BLOCKTIME_DEC_OW */
  /*322*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_C_MARKERTIME_INT_OW */
  /*323*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_C_MARKERTIME_DEC_OW */
  /*324*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_C_MII_TX_MK_CYC_DLY_OW */
  /*325*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_C_MII_TX_CW_CYC_DLY_OW */
  /*326*/  {/*baseAddr*/ 0x2e0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_C_TSU_TX_SD_PERIOD_OW */

    /* Port<0> Status_2 */
  /*327*/  {/*baseAddr*/ 0x340,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*P0_SD_BIT_SLIP */

    /* Port<0> TSU Control 3 */
  /*328*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 15 }, /*P0_C_TSU_TX_SD_PERIOD_S */
};



