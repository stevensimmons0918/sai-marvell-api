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
* @file mvHwsMtipUsxExtUnits.c
*
* @brief Hawk MTI USX EXT register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>



const MV_HWS_REG_ADDR_FIELD_STC hawkMtipUsxExtUnitsDb[] = {
    /* Port<0> Control */
  /*0*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x28, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_TX_LOC_FAULT */
  /*1*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x28, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_TX_REM_FAULT */
  /*2*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x28, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_TX_LI_FAULT */
  /*3*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x28, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_TOD_SELECT */
  /*4*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x28, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_RX_PAUSE_CONTROL */
  /*5*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x28, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_RX_PAUSE_OW_VAL */
  /*6*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x28, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_PAUSE_802_3_REFLECT */
  /*7*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x28, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_LOOP_ENA */
  /*8*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x28, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_FF_TX_CRC */
  /*9*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x28, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_MASK_SW_RESET_OUT */
  /*10*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x28, /*fieldStart*/ 10, /*fieldLen*/ 6 }, /*P0_LED_PORT_NUM */
  /*11*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x28, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_LED_PORT_EN */
  /*12*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x28, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_LOOP_RX_BLOCK_OUT */
  /*13*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x28, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_LOOP_TX_RDY_OUT */

    /* Port<0> Status */
  /*14*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_LINK_OK */
  /*15*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_LINK_STATUS */
  /*16*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_LPCS_LINK_STATUS */
  /*17*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_LPCS_RX_SYNC */
  /*18*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_LPCS_AN_DONE */
  /*19*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/  5, /*fieldLen*/ 8 }, /*P0_MAC_RES_SPEED */
  /*20*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_HI_BER */
  /*21*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_RX_TRAFFIC_IND */
  /*22*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_TX_TRAFFIC_IND */
  /*23*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_MAC_TX_EMPTY */
  /*24*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_MAC_TX_ISIDLE */
  /*25*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_PFC_MODE */
  /*26*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_FF_TX_SEPTY */
  /*27*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_FF_RX_EMPTY */
  /*28*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_FF_RX_DSAV */
  /*29*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*CHANNELS_ENA_IND */
  /*30*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*USXG_ENA_IND */
  /*31*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_LINK_OK_CLEAN */
  /*32*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_LINK_STATUS_CLEAN */
  /*33*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x28, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_LPCS_LINK_STATUS_CLEAN */

    /* Port<0> Interrupt Cause */
  /*34*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x28, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PORT0_INTERRUPT_CAUSE_INT_SUM */
  /*35*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x28, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_LINK_OK_CHANGE */
  /*36*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x28, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_LINK_STATUS_CHANGE */
  /*37*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x28, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_LPCS_LINK_STATUS_CHANGE */
  /*38*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x28, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_MAC_LOC_FAULT */
  /*39*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x28, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_MAC_REM_FAULT */
  /*40*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x28, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_MAC_LI_FAULT */
  /*41*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x28, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_MAC_TX_UNDERFLOW */
  /*42*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x28, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_MAC_TX_OVR_ERR */
  /*43*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x28, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_RX_OVERRUN */
  /*44*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x28, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_HI_BER_INT */
  /*45*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x28, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_FF_RX_RDY */
  /*46*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x28, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_LINK_OK_CLEAN_CHANGE */
  /*47*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x28, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_LINK_STATUS_CLEAN_CHANGE */
  /*48*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x28, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_LPCS_LINK_STATUS_CLEAN_CHANGE */
  /*49*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x28, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_LPCS_AN_DONE_INT */

    /* Port<0> Interrupt Mask */
  /*50*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x28, /*fieldStart*/  1, /*fieldLen*/ 15 }, /*P0_INTERRUPT_MASK */

    /* Global Last Violation */
  /*51*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*LAST_VIOLATION */

    /* Global Interrupt Mask */
  /*52*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 15 }, /*GLOBAL_INTERRUPT_MASK */

    /* Global Interrupt Cause */
  /*53*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_CAUSE_INT_SUM */
  /*54*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*ILLEGAL_ADDRESS_ACCESS */
  /*55*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FEC91_CERR_INT_0 */
  /*56*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FEC91_NCERR_INT_0 */
  /*57*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FEC91_HI_SER_INT_0 */
  /*58*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FEC91_CERR_INT_1 */
  /*59*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FEC91_NCERR_INT_1 */
  /*60*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FEC91_HI_SER_INT_1 */
  /*61*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TSU_RX_DFF_ERR_0 */
  /*62*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TSU_RX_AM_ERR_0 */
  /*63*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*TSU_RX_DFF_ERR_1 */
  /*64*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TSU_RX_AM_ERR_1 */
  /*65*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*TSU_TX_SYNC_ERR_0 */
  /*66*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*TSU_TX_SYNC_ERR_1 */
  /*67*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*INT_NON_ACCURATE_PTP_0 */
  /*68*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*INT_NON_ACCURATE_PTP_1 */

    /* Global MAC Metal Fix */
  /*69*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MAC_METAL_FIX */

    /* Global Interrupt Summary Cause */
  /*70*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_SUMMARY_INT_SUM */
  /*71*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_INT_SUM */
  /*72*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P1_INT_SUM */
  /*73*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P2_INT_SUM */
  /*74*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P3_INT_SUM */
  /*75*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P4_INT_SUM */
  /*76*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P5_INT_SUM */
  /*77*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P6_INT_SUM */
  /*78*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P7_INT_SUM */
  /*79*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*GLOBAL_INT_SUM */

    /* Global Interrupt Summary Mask */
  /*80*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 9 }, /*GLOBAL_INTERRUPT_SUMMARY_MASK */

    /* Port<0> Pause and Error Status */
  /*81*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x28, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_PAUSE_ON */
  /*82*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x28, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*P0_FF_RX_ERR_STAT */

    /* Port<0> Peer Delay */
  /*83*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x28, /*fieldStart*/  0, /*fieldLen*/ 30 }, /*P0_PEER_DELAY */
  /*84*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x28, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_PEER_DELAY_VALID */

    /* Port<0> Xoff Status */
  /*85*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x28, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_XOFF_STATUS */

    /* Port<0> Pause Override */
  /*86*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x28, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_PAUSE_OVERRIDE_CTRL */
  /*87*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x28, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*P0_PAUSE_OVERRIDE_VAL */

    /* Port<0> Xoff Override */
  /*88*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x28, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_XOFF_OVERRIDE_CTRL */
  /*89*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x28, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*P0_XOFF_OVERRIDE_VAL */

    /* Global PCS Metal Fix */
  /*90*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*PCS_METAL_FIX */

    /* Global Status */
  /*91*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK_L_0 */
  /*92*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*ALIGN_LOCK_0 */
  /*93*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*ALIGN_ERR_IND_0 */
  /*94*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*MULTIPLEXER_ACTIVE_0 */
  /*95*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FEC91_ENA_OUT_0 */
  /*96*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FEC91_AMPS_LOCK_0 */
  /*97*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FEC91_ALIGN_DONE_0 */
  /*98*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FEC91_CERR_0 */
  /*99*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FEC91_NCERR_0 */
  /*100*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*FEC91_HI_SER_0 */
  /*101*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*BLOCK_LOCK_L_1 */
  /*102*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*ALIGN_LOCK_1 */
  /*103*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*ALIGN_ERR_IND_1 */
  /*104*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*MULTIPLEXER_ACTIVE_1 */
  /*105*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*FEC91_ENA_OUT_1 */
  /*106*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*FEC91_AMPS_LOCK_1 */
  /*107*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*FEC91_ALIGN_DONE_1 */
  /*108*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*FEC91_CERR_1 */
  /*109*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*FEC91_NCERR_1 */
  /*110*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*FEC91_HI_SER_1 */
  /*111*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*TSU_RX_READY_0 */
  /*112*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*TSU_RX_READY_1 */
  /*113*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*TSU_TX_READY_0 */
  /*114*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*TSU_TX_READY_1 */

    /* Global Control */
  /*115*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SD0_N2 */
  /*116*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SD1_N2 */
  /*117*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*BER_TIMER_SHORT */
  /*118*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PCS_ABU_WATCHDOG */
  /*119*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*MAC_ABU_WATCHDOG */
  /*120*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 4 }, /*CYC_TO_STRETCH_MAC2APP */
  /*121*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 4 }, /*CYC_TO_STRETCH_MAC2REG */
  /*122*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 4 }, /*CYC_TO_STRETCH_APP2REG */

    /* Global MAC Clock and Reset Control */
  /*123*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PORT0_MAC_RESET_ */
  /*124*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PORT1_MAC_RESET_ */
  /*125*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PORT2_MAC_RESET_ */
  /*126*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PORT3_MAC_RESET_ */
  /*127*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PORT4_MAC_RESET_ */
  /*128*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PORT5_MAC_RESET_ */
  /*129*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PORT6_MAC_RESET_ */
  /*130*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PORT7_MAC_RESET_ */
  /*131*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PORT0_MAC_CLK_EN */
  /*132*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*PORT1_MAC_CLK_EN */
  /*133*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*PORT2_MAC_CLK_EN */
  /*134*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*PORT3_MAC_CLK_EN */
  /*135*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*PORT4_MAC_CLK_EN */
  /*136*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*PORT5_MAC_CLK_EN */
  /*137*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*PORT6_MAC_CLK_EN */
  /*138*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PORT7_MAC_CLK_EN */
  /*139*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*CMN_MAC_RESET_ */
  /*140*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*CMN_MAC_CLK_EN */
  /*141*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*CMN_APP_RESET_ */
  /*142*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*CMN_APP_CLK_EN */

    /* Global PCS Clock and Reset Control */
  /*143*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SD0_TX_RESET_ */
  /*144*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PCS_FEC_SD0_TX_CLK_RESET_ */
  /*145*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PCS_FEC_SD0_TX_CLK_EN */
  /*146*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*SD0_RX_RESET_ */
  /*147*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PCS_FEC_SD0_RX_CLK_RESET_ */
  /*148*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PCS_FEC_SD0_RX_CLK_EN */
  /*149*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SD1_TX_RESET_ */
  /*150*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PCS_FEC_SD1_TX_CLK_RESET_ */
  /*151*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PCS_FEC_SD1_TX_CLK_EN */
  /*152*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SD1_RX_RESET_ */
  /*153*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*PCS_FEC_SD1_RX_CLK_RESET_ */
  /*154*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*PCS_FEC_SD1_RX_CLK_EN */
  /*155*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*PCS_FEC_MAC_CLK_RESET_ */
  /*156*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*PCS_FEC_MAC_CLK_EN */
  /*157*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*PCS_SG_MAC_CLK_RESET_ */
  /*158*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PCS_SG_MAC_CLK_EN */
  /*159*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*PCS_MAC_CLK_RESET_ */
  /*160*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*PCS_MAC_CLK_EN */

    /* Port<0> Control1 */
  /*161*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x28, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*P0_PORT_RES_SPEED */
  /*162*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x28, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_FORCE_LINK_STATUS_EN */
  /*163*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x28, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_FORCE_LINK_STATUS_DIS */
  /*164*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x28, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_FORCE_LPCS_LINK_STATUS_EN */
  /*165*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x28, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_FORCE_LPCS_LINK_STATUS_DIS */
  /*166*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x28, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_FORCE_LINK_OK_EN */
  /*167*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x28, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_FORCE_LINK_OK_DIS */
  /*168*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x28, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_FORCE_TS_IF_PCH_EN */
  /*169*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x28, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_FORCE_TS_IF_PCH_DIS */
  /*170*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x28, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_FORCE_TS_IF_CF_EN */
  /*171*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x28, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_FORCE_TS_IF_CF_DIS */
  /*172*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x28, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_FORCE_TS_IF_TSTF_EN */
  /*173*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x28, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_FORCE_TS_IF_TSTF_DIS */

    /* Global USX PCH Control */
  /*174*/  {/*baseAddr*/ 0x16c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*USX_PCH_SIGNATURE_MODE */
  /*175*/  {/*baseAddr*/ 0x16c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*USX_PCH_GEN_CRC_ERROR */
  /*176*/  {/*baseAddr*/ 0x16c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 4 }, /*USX_PCH_ACTION_FOR_TS */
  /*177*/  {/*baseAddr*/ 0x16c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*USX_PCH_PACKET_TYPE */
  /*178*/  {/*baseAddr*/ 0x16c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 16 }, /*USX_PCH_RESERVED31_16 */
  /*179*/  {/*baseAddr*/ 0x16c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 8 }, /*USX_PCH_RESERVED39_32 */

    /* Port<0> USX PCH Last timestamp */
  /*180*/  {/*baseAddr*/ 0x170,  /*offsetFormula*/ 0x10, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_USX_PCH_LAST_TIMESTAMP */

    /* Port<0> USX PCH Signature control */
  /*181*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x10, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_USX_PCH_SIGNATURE_CONTROL */

    /* Port<0> USX PCH CRC errors counter */
  /*182*/  {/*baseAddr*/ 0x178,  /*offsetFormula*/ 0x10, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_USX_PCH_CRC_ERRORS_COUNTER */

    /* Port<0> USX PCH Control */
  /*183*/  {/*baseAddr*/ 0x17c,  /*offsetFormula*/ 0x10, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_USX_PCH_RX_PROCESS */
  /*184*/  {/*baseAddr*/ 0x17c,  /*offsetFormula*/ 0x10, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_USX_PCH_RX_CRC_CHECK */
  /*185*/  {/*baseAddr*/ 0x17c,  /*offsetFormula*/ 0x10, /*fieldStart*/  2, /*fieldLen*/ 4 }, /*P0_USX_PCH_SUB_PORT_ID */
  /*186*/  {/*baseAddr*/ 0x17c,  /*offsetFormula*/ 0x10, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*P0_USX_PCH_EXTENSION_FIELD_TYPE */

    /* Port<0> TSX Control */
  /*187*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x14, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_C_MODULO_RX */

    /* Port<0> TSX Control4 */
  /*188*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x14, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_C_MODULO_RX_OW */
  /*189*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x14, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_C_MODULO_TX_OW */
  /*190*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x14, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_C_RX_MODE_OW */
  /*191*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x14, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_C_MII_CW_DLY_OW */
  /*192*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x14, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_C_MII_MK_DLY_OW */
  /*193*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x14, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_C_DESKEW_OW */
  /*194*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x14, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_C_BLOCKTIME_OW */
  /*195*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x14, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_C_BLOCKTIME_DEC_OW */
  /*196*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x14, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_C_MARKERTIME_OW */
  /*197*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x14, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_C_MARKERTIME_DEC_OW */
  /*198*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x14, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_C_BLKS_PER_CLK_OW */
  /*199*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x14, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_C_TX_MODE_OW */
  /*200*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x14, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_C_MII_TX_MK_CYC_DLY_OW */
  /*201*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x14, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_C_MII_TX_CW_CYC_DLY_OW */
  /*202*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x14, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_C_TSU_TX_SD_PERIOD_OW */
  /*203*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x14, /*fieldStart*/ 15, /*fieldLen*/ 15 }, /*P0_C_TSU_TX_SD_PERIOD */

    /* Port<0> TSX Control3 */
  /*204*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x14, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*P0_C_RX_MODE */
  /*205*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x14, /*fieldStart*/  3, /*fieldLen*/ 3 }, /*P0_C_MII_CW_DLY */
  /*206*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x14, /*fieldStart*/  6, /*fieldLen*/ 3 }, /*P0_C_MII_MK_DLY */
  /*207*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x14, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_C_BLKS_PER_CLK */
  /*208*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x14, /*fieldStart*/ 10, /*fieldLen*/ 10 }, /*P0_C_DESKEW */
  /*209*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x14, /*fieldStart*/ 20, /*fieldLen*/ 5 }, /*P0_C_MII_TX_MK_CYC_DLY */
  /*210*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x14, /*fieldStart*/ 25, /*fieldLen*/ 5 }, /*P0_C_MII_TX_CW_CYC_DLY */

    /* Port<0> TSX Control2 */
  /*211*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x14, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*P0_C_BLOCKTIME */
  /*212*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x14, /*fieldStart*/  5, /*fieldLen*/ 9 }, /*P0_C_BLOCKTIME_DEC */
  /*213*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x14, /*fieldStart*/ 14, /*fieldLen*/ 5 }, /*P0_C_MARKERTIME */
  /*214*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x14, /*fieldStart*/ 19, /*fieldLen*/ 9 }, /*P0_C_MARKERTIME_DEC */
  /*215*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x14, /*fieldStart*/ 28, /*fieldLen*/ 3 }, /*P0_C_TX_MODE */

    /* Port<0> TSX Control1 */
  /*216*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x14, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_C_MODULO_TX */

    /* Port<0> TSD Control1 */
  /*217*/  {/*baseAddr*/ 0x21c,  /*offsetFormula*/ 0x8, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_FSU_ENABLE */
  /*218*/  {/*baseAddr*/ 0x21c,  /*offsetFormula*/ 0x8, /*fieldStart*/  1, /*fieldLen*/ 10 }, /*P0_FSU_OFFSET */
  /*219*/  {/*baseAddr*/ 0x21c,  /*offsetFormula*/ 0x8, /*fieldStart*/ 11, /*fieldLen*/ 8 }, /*P0_FSU_RND_DELTA */
  /*220*/  {/*baseAddr*/ 0x21c,  /*offsetFormula*/ 0x8, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_AMD_ENABLE */
  /*221*/  {/*baseAddr*/ 0x21c,  /*offsetFormula*/ 0x8, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_AMD_CNT_TYPE_SEL */
  /*222*/  {/*baseAddr*/ 0x21c,  /*offsetFormula*/ 0x8, /*fieldStart*/ 21, /*fieldLen*/ 5 }, /*P0_MINIMAL_TX_STOP_TOGGLE */
  /*223*/  {/*baseAddr*/ 0x21c,  /*offsetFormula*/ 0x8, /*fieldStart*/ 26, /*fieldLen*/ 5 }, /*P0_MINIMAL_EMPTY_FOR_STOP_TX */

    /* Port<0> TSD Control */
  /*224*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x8, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_AMD_CNT_LOW */
  /*225*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x8, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*P0_AMD_CNT_HIGH */

    /* Global Status2 */
  /*226*/  {/*baseAddr*/ 0x228,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*SD_BIT_SLIP_0 */
  /*227*/  {/*baseAddr*/ 0x228,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 6 }, /*SD_BIT_SLIP_1 */

    /* Port<0> TSD Control2 */
  /*228*/  {/*baseAddr*/ 0x220,  /*offsetFormula*/ 0xc, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_FSU_PREEMPTION_HOLD_EN */
  /*229*/  {/*baseAddr*/ 0x220,  /*offsetFormula*/ 0xc, /*fieldStart*/  1, /*fieldLen*/ 4 }, /*P0_HOLD_KEPT_ACTIVE */
};



