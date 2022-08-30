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
* @file mvHwsHarrierAnpUnits.c
*
* @brief Harrier ANP register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC harrierAnpUnitsDb[] = {
    /* Port<0> AN Status1 */
  /*0*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_4X_S */
  /*1*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_G_S */
  /*2*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_2P5G_S */
  /*3*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_5GR_S */
  /*4*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_10GKR_S */
  /*5*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_IEEE25GS_S */
  /*6*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_IEEE25GR_S */
  /*7*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_CON25GKR_S */
  /*8*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_CON25GCR_S */
  /*9*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_40GKR4_S */
  /*10*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_40GCR4_S */
  /*11*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_50GKR2_S */
  /*12*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_50GCR2_S */
  /*13*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_100GCR4_S */
  /*14*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_100GCR10_S */
  /*15*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_100GKR4_S */
  /*16*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_100GKP4_S */
  /*17*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_200GKR8_S */
  /*18*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_25GKR2_S */
  /*19*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_CON40GR2_S */
  /*20*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_50GKR4_S */
  /*21*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_50GRCR_S */
  /*22*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_100GRCR2_S */
  /*23*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_200GRCR4_S */
  /*24*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_100GRCR_S */
  /*25*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_200GRCR2_S */
  /*26*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_400GRCR4_S */
  /*27*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_800GRCR8_S */
  /*28*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_CON400GRCR8_S */

    /* Port<0> AN Status2 */
  /*29*/  {/*baseAddr*/ 0x134,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_AP_AA_CLEAR_HCD_S */
  /*30*/  {/*baseAddr*/ 0x134,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_IEEE_AG_ANEG_ENABLE_S */
  /*31*/  {/*baseAddr*/ 0x134,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_AP_AG_RESTART_ANEG_S */
  /*32*/  {/*baseAddr*/ 0x134,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_AP_AG_LINK_S */
  /*33*/  {/*baseAddr*/ 0x134,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_AP_AG_HCD_RESOLVED_S */
  /*34*/  {/*baseAddr*/ 0x134,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_AA_LINK_GOOD_S */
  /*35*/  {/*baseAddr*/ 0x134,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_AP_AG_RX_PAUSE_ENABLE_S */
  /*36*/  {/*baseAddr*/ 0x134,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_AP_AG_TX_PAUSE_ENABLE_S */
  /*37*/  {/*baseAddr*/ 0x134,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_AP_FEC_ENABLE_S */
  /*38*/  {/*baseAddr*/ 0x134,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_AP_RSFEC_ENABLE_S */
  /*39*/  {/*baseAddr*/ 0x134,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_AG_LLFEC_ENABLE_S */

    /* Metal Fix */
  /*40*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*METAL_FIX */

    /* Interrupt Summary Cause */
  /*41*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*INTERRUPT_CAUSE_INT_SUM */
  /*42*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*GLOBAL_INT */
  /*43*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_INT_SUM */
  /*44*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_INT2_SUM */
  /*45*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P1_INT_SUM */
  /*46*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P1_INT2_SUM */
  /*47*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P2_INT_SUM */
  /*48*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P2_INT2_SUM */
  /*49*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P3_INT_SUM */
  /*50*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P3_INT2_SUM */
  /*51*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P4_INT_SUM */
  /*52*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P4_INT2_SUM */
  /*53*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P5_INT_SUM */
  /*54*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P5_INT2_SUM */
  /*55*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P6_INT_SUM */
  /*56*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P6_INT2_SUM */
  /*57*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P7_INT_SUM */
  /*58*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P7_INT2_SUM */

    /* Interrupt Summary Mask */
  /*59*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*INTERRUPT_MASK */
  /*60*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_INTERRUPT1_MASK */
  /*61*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_INTERRUPT2_MASK */
  /*62*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P1_INTERRUPT1_MASK */
  /*63*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P1_INTERRUPT2_MASK */
  /*64*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P2_INTERRUPT1_MASK */
  /*65*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P2_INTERRUPT2_MASK */
  /*66*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P3_INTERRUPT1_MASK */
  /*67*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P3_INTERRUPT2_MASK */
  /*68*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P4_INTERRUPT1_MASK */
  /*69*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P4_INTERRUPT2_MASK */
  /*70*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P5_INTERRUPT1_MASK */
  /*71*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P5_INTERRUPT2_MASK */
  /*72*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P6_INTERRUPT1_MASK */
  /*73*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P6_INTERRUPT2_MASK */
  /*74*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P7_INTERRUPT1_MASK */
  /*75*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P7_INTERRUPT2_MASK */

    /* Last Violation */
  /*76*/  {/*baseAddr*/ 0x9c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*LAST_VIOLATION */

    /* Port<0> PCS Link max timer norm */
  /*77*/  {/*baseAddr*/ 0x164,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_PCS_LINK_MAX_TIMER_NORM */

    /* Port<0> PCS Link max timer ap */
  /*78*/  {/*baseAddr*/ 0x168,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_PCS_LINK_MAX_TIMER_AP */

    /* Port<0> AN Control */
  /*79*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PHY_GEN_AP_OW */
  /*80*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 6 }, /*P0_PHY_GEN_AP_OW_VAL */
  /*81*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_PM_AP_MODE_OW */
  /*82*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_PM_AP_MODE_OW_VAL */
  /*83*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_PM_AP_RESET_RX_OW */
  /*84*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_PM_AP_RESET_RX_OW_VAL */
  /*85*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_PM_ENCLK_AP_FR_OW */
  /*86*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_PM_ENCLK_AP_FR_OW_VAL */
  /*87*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_PM_NORM_X_STATE_OW */
  /*88*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_PM_NORM_X_STATE_OW_VAL */
  /*89*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_PM_ENCLK_AP_SYS_OW */
  /*90*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_PM_ENCLK_AP_SYS_OW_VAL */
  /*91*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_PM_AP_RESET_TX_OW */
  /*92*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_PM_AP_RESET_TX_OW_VAL */
  /*93*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_PM_ENCLK_AP_FT_OW */
  /*94*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_PM_ENCLK_AP_FT_OW_VAL */
  /*95*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_AN_PCS_SEL_OW */
  /*96*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_AN_PCS_SEL_OW_VAL */
  /*97*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_RG_ST_PCSLINK_MAX_TIME_AP_INF_S */
  /*98*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_REG_AP_FORCE_MODE_S */
  /*99*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_RG_BREAK_LINK_TIMER_FAST */
  /*100*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_RG_AUTONEG_WAIT_TIMER_FAST */
  /*101*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*P0_RG_LINK_FAIL_INHIBIT_TIMER_FAST */
  /*102*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*P0_RG_TIMER2_FAST */
  /*103*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*P0_PM_TX_RX_DCLK_4X_EN_OW */
  /*104*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*P0_PM_TXDCLK_4X_EN_OW_VAL */
  /*105*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_PM_RXDCLK_4X_EN_OW_VAL */

    /* Port<0> Control */
  /*106*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*P0_PORT_MODE */
  /*107*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_PCS_EN */
  /*108*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_POWER_DOWN */
  /*109*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 4 }, /*P0_CUSTOM_PHY_GEN */
  /*110*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_RX_INIT_B4_TX_TRAIN */
  /*111*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 2 }, /*P0_CUSTOM_NUM_OF_LANES */
  /*112*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_REG_PWRUP_CNT_CLEAR_S */
  /*113*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_RG_ST_PCSLINK_MAX_TIME_NORM_INF_S */
  /*114*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_RG_MODE_1G_OW */
  /*115*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_RG_MODE_1G_OW_VAL */
  /*116*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_RG_MODE_2P5G_OW */
  /*117*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_RG_MODE_2P5G_OW_VAL */
  /*118*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_RG_MODE_5G_OW */
  /*119*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_RG_MODE_5G_OW_VAL */
  /*120*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_RG_MODE_10G_OW */
  /*121*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_RG_MODE_10G_OW_VAL */
  /*122*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_RG_MODE_25G_OW */
  /*123*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_RG_MODE_25G_OW_VAL */
  /*124*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_RG_MODE_40GR4_OW */
  /*125*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*P0_RG_MODE_40GR4_OW_VAL */
  /*126*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*P0_RG_MODE_40GR2_OW */
  /*127*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*P0_RG_MODE_40GR2_OW_VAL */
  /*128*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*P0_RG_MODE_50GR2_OW */
  /*129*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_RG_MODE_50GR2_OW_VAL */

    /* Port<0> Control1 */
  /*130*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_RG_MODE_50GR_OW */
  /*131*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_RG_MODE_50GR_OW_VAL */
  /*132*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_RG_MODE_100GR4_OW */
  /*133*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_RG_MODE_100GR4_OW_VAL */
  /*134*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_RG_MODE_110GR4_OW */
  /*135*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_RG_MODE_110GR4_OW_VAL */
  /*136*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_RG_MODE_100GR2_OW */
  /*137*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_RG_MODE_100GR2_OW_VAL */
  /*138*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_RG_MODE_107GR2_OW */
  /*139*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_RG_MODE_107GR2_OW_VAL */
  /*140*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_RG_MODE_200GR8_OW */
  /*141*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_RG_MODE_200GR8_OW_VAL */
  /*142*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_RG_MODE_200GR4_OW */
  /*143*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_RG_MODE_200GR4_OW_VAL */
  /*144*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_RG_MODE_400GR8_OW */
  /*145*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_RG_MODE_400GR8_OW_VAL */
  /*146*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_RG_MODE_428GR8_OW */
  /*147*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_RG_MODE_428GR8_OW_VAL */
  /*148*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_RG_MODE_QSGMII_OW */
  /*149*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_RG_MODE_QSGMII_OW_VAL */
  /*150*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_RG_MODE_USGMII_OW */
  /*151*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_RG_MODE_USGMII_OW_VAL */
  /*152*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_RG_MODE_USX2P5G_OW */
  /*153*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_RG_MODE_USX2P5G_OW_VAL */
  /*154*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_RG_MODE_USX5G_OW */
  /*155*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_RG_MODE_USX5G_OW_VAL */
  /*156*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_RG_MODE_USX10G_OW */
  /*157*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*P0_RG_MODE_USX10G_OW_VAL */
  /*158*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*P0_RG_MODE_USX20G_OW */
  /*159*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*P0_RG_MODE_USX20G_OW_VAL */
  /*160*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*P0_RG_MODE_CUSTOM_OW */
  /*161*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_RG_MODE_CUSTOM_OW_VAL */

    /* Port<0> Control2 */
  /*162*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PM_SD_PHY_GEN_TX_OW */
  /*163*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PM_SD_PHY_GEN_RX_OW */
  /*164*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_SD_PHY_GEN_OW */
  /*165*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 6 }, /*P0_SD_PHY_GEN_OW_VAL */
  /*166*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_PM_NR_RX_RESET_OW */
  /*167*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_PM_NR_RX_RESET_OW_VAL */
  /*168*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_PM_AN_RESTART_OW */
  /*169*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_PM_AN_RESTART_OW_VAL */
  /*170*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_PM_NR_TX_RESET_OW */
  /*171*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_PM_NR_TX_RESET_OW_VAL */
  /*172*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_PM_SD_PU_PLL_OW */
  /*173*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_PM_SD_PU_PLL_OW_VAL */
  /*174*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_PM_SD_PU_TX_OW */
  /*175*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_PM_SD_PU_TX_OW_VAL */
  /*176*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_PM_SD_PU_RX_OW */
  /*177*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_PM_SD_PU_RX_OW_VAL */
  /*178*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_PM_SD_SOFTRST_S_OW */
  /*179*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_PM_SD_SOFTRST_S_OW_VAL */
  /*180*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_PM_SD_COUPLE_MODE_EN_OW */
  /*181*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_PM_SD_COUPLE_MODE_EN_OW_VAL */
  /*182*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_PM_SD_TXCLK_SYNC_EN_PLL_OW */
  /*183*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*P0_PM_SD_TXCLK_SYNC_EN_PLL_OW_VAL */
  /*184*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*P0_PM_ST_EN_OW */
  /*185*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*P0_PM_ST_EN_OW_VAL */
  /*186*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*P0_PM_SOFTRST_S_OW */
  /*187*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_PM_SOFTRST_S_OW_VAL */

    /* Port<0> Control3 */
  /*188*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PM_PCS_COUPLE_OW */
  /*189*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PM_PCS_COUPLE_OW_VAL */
  /*190*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_PM_TRAIN_TYPE_MX_OW */
  /*191*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 2 }, /*P0_PM_TRAIN_TYPE_MX_OW_VAL */
  /*192*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_REG_PROG_MAX_TIME_S_INF */
  /*193*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_PM_PWRDN_OW */
  /*194*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_PM_PWRDN_OW_VAL */
  /*195*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_PM_AN_HCD_CLEAR_OW */
  /*196*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_PM_AN_HCD_CLEAR_OW_VAL */
  /*197*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_PM_SD_TX_IDLE_OW */
  /*198*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_PM_SD_TX_IDLE_OW_VAL */
  /*199*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_PM_AN_HCD_RESOLVED_OW */
  /*200*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_PM_AN_HCD_RESOLVED_OW_VAL */
  /*201*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_PM_SD_DFE_UPDATE_DIS_OW */
  /*202*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_PM_SD_DFE_UPDATE_DIS_OW_VAL */
  /*203*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_PM_TX_TRAIN_POLY_SEL_OW */
  /*204*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 2 }, /*P0_PM_TX_TRAIN_POLY_SEL_OW_VAL */
  /*205*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_PM_DATA_GRAY_CODE_EN */
  /*206*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_PM_DATA_PRE_CODE_EN */
  /*207*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_PM_SD_DFE_PAT_DIS_OW */
  /*208*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_PM_SD_DFE_PAT_DIS_OW_VAL */
  /*209*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_PM_SD_DFE_EN_OW */
  /*210*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_PM_SD_DFE_EN_OW_VAL */
  /*211*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_REG_TX_TFIFO_W_UPD */
  /*212*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_REG_TX_TFIFO_R_UPD */
  /*213*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*P0_REG_RX_TFIFO_W_UPD */
  /*214*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*P0_REG_RX_TFIFO_R_UPD */
  /*215*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*P0_TX_TRAIN_ERROR_S_OW */
  /*216*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 2 }, /*P0_TX_TRAIN_ERROR_S_OW_VAL */

    /* Global Control */
  /*217*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*AN_AP_TRAIN_TYPE */
  /*218*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 8 }, /*REG_TXCLK_SYNC_EN_WIDTH_S */
  /*219*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 2 }, /*REG_TRAIN_TYPE_SAMP */
  /*220*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 2 }, /*REG_TRAIN_TYPE_MX_SAMP */
  /*221*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 10 }, /*REG_RESET_PULSE_CONF_DELAY */
  /*222*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*REG_CH_SM_CNT_SATURATE */
  /*223*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*REG_TXSTR_RXSD_CLEAR_CNT */
  /*224*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*REG_TX_SM_CNT_SATURATE */
  /*225*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*REG_RX_SM_CNT_SATURATE */
  /*226*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*REG_TX_READY_LOSS_LATCH_EN */
  /*227*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*REG_DSP_SIGDET_LOSS_LATCH_EN */
  /*228*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*REG_DSP_LOCK_LOSS_LATCH_EN */
  /*229*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*PM_SD_PU_PLL_LATCH_EN */

    /* Global Lane Delay */
  /*230*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 25 }, /*LANE_DELAY */

    /* Port<0> Control5 */
  /*231*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PM_RX_TRAIN_ENABLE_OW */
  /*232*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PM_RX_TRAIN_ENABLE_OW_VAL */
  /*233*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_PM_TX_TRAIN_ENABLE_OW */
  /*234*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_PM_TX_TRAIN_ENABLE_OW_VAL */
  /*235*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_RG_TXRX_MAX_TIMER_INF */
  /*236*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_REG_NORMAL_STATE_LOCK */
  /*237*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_SD_RX_DTL_CLAMP_S_OW */
  /*238*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_SD_RX_DTL_CLAMP_S_OW_VAL */
  /*239*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_DSP_SIGDET_LOSS_PWRDN_INSTEAD_RXSTR */
  /*240*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_IGNORE_DSP_LOCK_LOSS */
  /*241*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_IGNORE_DSP_SIGDET_LOSS */
  /*242*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_IGNORE_TX_READY_LOSS */
  /*243*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_NO_DSP_LOCK_PWRDN_INSTEAD_RXSTR */
  /*244*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_RXSTR_PU_PLL_RX_VALUE */
  /*245*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_TX_READY_LOSS_PWRDN_INSTEAD_TXSTR */
  /*246*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_TXRX_ON_TIMEOUT_PWRDN_INSTEAD_RXSTR */
  /*247*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_TXRX_SD_TIMEOUT_PWRDN_INSTEAD_RXSTR */
  /*248*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_TXSTR_PU_PLL_TX_VALUE */
  /*249*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_NORM_EN_MX_S_OW */
  /*250*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_NORM_EN_MX_S_OW_VAL */
  /*251*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_PWRUP_EN_MX_S_OW */
  /*252*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_PWRUP_EN_MX_S_OW_VAL */
  /*253*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_RXON_EN_MX_S_OW */
  /*254*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_RXON_EN_MX_S_OW_VAL */
  /*255*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_RXSD_EN_MX_S_OW */
  /*256*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_RXSD_EN_MX_S_OW_VAL */
  /*257*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_TXON_EN_MX_S_OW */
  /*258*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_TXON_EN_MX_S_OW_VAL */
  /*259*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_TXRXON_EN_MX_S_OW */
  /*260*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_TXRXON_EN_MX_S_OW_VAL */
  /*261*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_TXRXSD_EN_MX_S_OW */
  /*262*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_TXRXSD_EN_MX_S_OW_VAL */

    /* Port<0> Control4 */
  /*263*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PM_AP_EN_S_OW */
  /*264*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PM_AP_EN_S_OW_VAL */
  /*265*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_PLL_READY_TX_OW */
  /*266*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_PLL_READY_TX_OW_VAL */
  /*267*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_PLL_READY_RX_OW */
  /*268*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_PLL_READY_RX_OW_VAL */
  /*269*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_SQ_DETECTED_LPF_OW */
  /*270*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_SQ_DETECTED_LPF_OW_VAL */
  /*271*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_RX_INIT_DONE_OW */
  /*272*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_RX_INIT_DONE_OW_VAL */
  /*273*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_RX_TRAIN_COMPLETE_OW */
  /*274*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_RX_TRAIN_COMPLETE_OW_VAL */
  /*275*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_RX_TRAIN_FAILED_OW */
  /*276*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_RX_TRAIN_FAILED_OW_VAL */
  /*277*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_TX_TRAIN_COMPLETE_OW */
  /*278*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_TX_TRAIN_COMPLETE_OW_VAL */
  /*279*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_TX_TRAIN_FAILED_OW */
  /*280*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_TX_TRAIN_FAILED_OW_VAL */
  /*281*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_SD_TXCLK_SYNC_START_OUT_OW */
  /*282*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_SD_TXCLK_SYNC_START_OUT_OW_VAL */
  /*283*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_PM_DSP_TXDN_ACK_OW */
  /*284*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_PM_DSP_TXDN_ACK_OW_VAL */
  /*285*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_PM_DSP_RXDN_ACK_OW */
  /*286*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_PM_DSP_RXDN_ACK_OW_VAL */
  /*287*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_PM_DSP_TX_READY_OW */
  /*288*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_PM_DSP_TX_READY_OW_VAL */
  /*289*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_PM_DSP_SIGDET_OW */
  /*290*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*P0_PM_DSP_SIGDET_OW_VAL */
  /*291*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*P0_PM_DSP_LOCK_OW */
  /*292*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*P0_PM_DSP_LOCK_OW_VAL */
  /*293*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*P0_PM_RX_INIT_OW */
  /*294*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_PM_RX_INIT_OW_VAL */

    /* Port<0> CH DSP RXUP Max Timer */
  /*295*/  {/*baseAddr*/ 0x16c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 15 }, /*P0_RG_DSP_RXUP_MAX_TIMER */
  /*296*/  {/*baseAddr*/ 0x16c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_RG_DSP_RXUP_MAX_TIMER_INF */

    /* Port<0> CH ST Min Timer */
  /*297*/  {/*baseAddr*/ 0x158,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_ST_WAIT_MIN_S */

    /* Port<0> CH ST RESET Min Timer */
  /*298*/  {/*baseAddr*/ 0x154,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_ST_RESET_MIN_TIME */

    /* Port<0> CH PCS LOST Min Timer */
  /*299*/  {/*baseAddr*/ 0x150,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_PCS_LOST_MIN_TIMER */

    /* Port<0> CH TXRX Max Timer */
  /*300*/  {/*baseAddr*/ 0x14c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_TXRX_MAX_TIMER */

    /* Port<0> CH TXRX Min Timer */
  /*301*/  {/*baseAddr*/ 0x148,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_TXRX_MIN_TIMER */

    /* Port<0> CH WAIT PWRDN Min Timer */
  /*302*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_WAIT_PWRDN_MIN_TIMER */

    /* Port<0> CH WAIT PWRDN Max Timer */
  /*303*/  {/*baseAddr*/ 0x140,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_WAIT_PWRDN_MAX_TIMER */

    /* Global Clock and Reset */
  /*304*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PWM_SOFT_RESET_ */
  /*305*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PWM_CLK_EN */
  /*306*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_AN_HW_SOFT_RESET_ */
  /*307*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_AN_SW_SOFT_RESET_ */
  /*308*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P1_AN_HW_SOFT_RESET_ */
  /*309*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P1_AN_SW_SOFT_RESET_ */
  /*310*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P2_AN_HW_SOFT_RESET_ */
  /*311*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P2_AN_SW_SOFT_RESET_ */
  /*312*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P3_AN_HW_SOFT_RESET_ */
  /*313*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P3_AN_SW_SOFT_RESET_ */
  /*314*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P4_AN_HW_SOFT_RESET_ */
  /*315*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P4_AN_SW_SOFT_RESET_ */
  /*316*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P5_AN_HW_SOFT_RESET_ */
  /*317*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P5_AN_SW_SOFT_RESET_ */
  /*318*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P6_AN_HW_SOFT_RESET_ */
  /*319*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P6_AN_SW_SOFT_RESET_ */
  /*320*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P7_AN_HW_SOFT_RESET_ */
  /*321*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P7_AN_SW_SOFT_RESET_ */

    /* Global SD Mux Control1 */
  /*322*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*SD_MUX_CONTROL4 */
  /*323*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*SD_MUX_CONTROL5 */
  /*324*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*SD_MUX_CONTROL6 */
  /*325*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 8 }, /*SD_MUX_CONTROL7 */

    /* Global SD Mux Control */
  /*326*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*SD_MUX_CONTROL0 */
  /*327*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*SD_MUX_CONTROL1 */
  /*328*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*SD_MUX_CONTROL2 */
  /*329*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 8 }, /*SD_MUX_CONTROL3 */

    /* Global Timer 10ms */
  /*330*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_10MS */

    /* Global Timer 1s */
  /*331*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_1S */

    /* Global Timer 500ms */
  /*332*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_500MS */

    /* Global Timer 300ms */
  /*333*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_300MS */

    /* Global Timer 200ms */
  /*334*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_200MS */

    /* Global Timer 100ms */
  /*335*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_100MS */

    /* Global Timer 70ms */
  /*336*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_70MS */

    /* Global Timer 50ms */
  /*337*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_50MS */

    /* Global Timer 40ms */
  /*338*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_40MS */

    /* Global Timer 35ms */
  /*339*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_35MS */

    /* Global Timer 12600ms 20s High */
  /*340*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RG_AN_TIMERS_12600MS_HIGH */
  /*341*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*RG_AN_TIMERS_20S_HIGH */

    /* Global Timer 9s 12s High */
  /*342*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RG_AN_TIMERS_9S_HIGH */
  /*343*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*RG_AN_TIMERS_12S_HIGH */

    /* Global Timer 40s Low */
  /*344*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_40S_LOW */

    /* Global Timer 30s Low */
  /*345*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_30S_LOW */

    /* Global Timer 20s Low */
  /*346*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_20S_LOW */

    /* Global Timer 12600ms Low */
  /*347*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_12600MS_LOW */

    /* Global Timer 12s Low */
  /*348*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_12S_LOW */

    /* Global Timer 9s Low */
  /*349*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_9S_LOW */

    /* Global Timer 6s */
  /*350*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_6S */

    /* Global Timer 5s */
  /*351*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_5S */

    /* Global Timer 3150ms */
  /*352*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_3150MS */

    /* Global Timer 2s */
  /*353*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_2S */

    /* Global Timer 30s 40s High */
  /*354*/  {/*baseAddr*/ 0x6c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RG_AN_TIMERS_30S_HIGH */
  /*355*/  {/*baseAddr*/ 0x6c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*RG_AN_TIMERS_40S_HIGH */

    /* Port<0> AN Timers1 */
  /*356*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_RG_LINK_FAIL_INHIBIT_TIMER_FAST_VAL */
  /*357*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*P0_RG_TIMER2_FAST_VAL */

    /* Port<0> AN Timers */
  /*358*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_RG_BREAK_LINK_TIMER_FAST_VAL */
  /*359*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*P0_RG_AUTONEG_WAIT_TIMER_FAST_VAL */

    /* Global AN Revision */
  /*360*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RUNI_OUI_NUM_3_18_A */
  /*361*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 4 }, /*RUNI_CFG_REV_ID_A */
  /*362*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 6 }, /*RUNI_CFG_MODEL_NUM_A */
  /*363*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 6 }, /*RUNI_OUI_NUM_19_24_A */

    /* Global AN Abilities */
  /*364*/  {/*baseAddr*/ 0x70,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RUNI_CFG_ABILITY1_A */
  /*365*/  {/*baseAddr*/ 0x70,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*RUNI_CFG_ABILITY2_A */

    /* Port<0> AN Hang Out */
  /*366*/  {/*baseAddr*/ 0x12c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_AP_IRQ_S */
  /*367*/  {/*baseAddr*/ 0x12c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_AP_RG_3_0_WR_S */
  /*368*/  {/*baseAddr*/ 0x12c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 5 }, /*P0_AG_MODE_S */
  /*369*/  {/*baseAddr*/ 0x12c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_ANEG_INT_S */
  /*370*/  {/*baseAddr*/ 0x12c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_AG_REG_1_6_S */
  /*371*/  {/*baseAddr*/ 0x12c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_AP_AG_LINK_DOWN_PLS_S */
  /*372*/  {/*baseAddr*/ 0x12c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_IEEE_AG_ANEG_ENABLE_PLS_S */

    /* Port<0> AN Tied In */
  /*373*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PM_AP_ANEG_REMOTE_READY_S */
  /*374*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PM_PRBS_MODE_S */
  /*375*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_GRG_3_0_15_S */
  /*376*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 5 }, /*P0_N_AG_MODE_S */
  /*377*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_N_AA_LINK_GOOD_S */
  /*378*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_FAR_CLEAR_RESET_ALL_S */
  /*379*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_FAR_SET_RESTART_ALL_S */
  /*380*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_PM_KR_ENABLE_S */
  /*381*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_PM_LOOPBACK_S */
  /*382*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_POR_FEC_ADV_S */
  /*383*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_RING_OSC_A */
  /*384*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 5 }, /*P0_Z80_AG_MODE_S */
  /*385*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_Z80_FEC_ENABLE_S */
  /*386*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_Z80_LLFEC_ENABLE_S */
  /*387*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_Z80_RESOLVED_S */
  /*388*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_Z80_RSFEC_ENABLE_S */

    /* Port<0> Control6 */
  /*389*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PM_PCS_RX_CLK_ENA_OW */
  /*390*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PM_PCS_RX_CLK_ENA_OW_VAL */
  /*391*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_PM_PCS_TX_CLK_ENA_OW */
  /*392*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_PM_PCS_TX_CLK_ENA_OW_VAL */
  /*393*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_PM_PCS_SD_RX_RESETN_OW */
  /*394*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_PM_PCS_SD_RX_RESETN_OW_VAL */
  /*395*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_PM_PCS_SD_TX_RESETN_OW */
  /*396*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_PM_PCS_SD_TX_RESETN_OW_VAL */
  /*397*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_PM_AN_PCS_CLKOUT_SEL_OW */
  /*398*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_PM_AN_PCS_CLKOUT_SEL_OW_VAL */
  /*399*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_LINK_STATUS_OW */
  /*400*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_LINK_STATUS_OW_VAL */
  /*401*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_REG_INVERT_SD_RX_IN_S */
  /*402*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_REG_INVERT_SD_TX_OUT_S */
  /*403*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 2 }, /*P0_REG_SIGDET_MODE */
  /*404*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_RG_PWRDN_RDY_S */
  /*405*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_RG_SEL_LOS_SIG_S */
  /*406*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_TX_BUSY_NO_PWRDN */
  /*407*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_PCS_CFG_DONE_HW_CLR */
  /*408*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_REG_TX_TRAIN_COMPLETE_LATCH */
  /*409*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_REG_RX_TRAIN_COMPLETE_LATCH */
  /*410*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_REG_TX_TRAIN_FAILED_LATCH */
  /*411*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_REG_RX_TRAIN_FAILED_LATCH */
  /*412*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_REG_AN_RESTART_CNT_EN */
  /*413*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_REG_DSP_LOCK_FAIL_CNT_EN */
  /*414*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_REG_LINK_FAIL_CNT_EN */
  /*415*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*P0_RG_PROG_ENABLE_S */
  /*416*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*P0_RG_FORCE_PG_START_S */
  /*417*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*P0_CMEM_MASK */
  /*418*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*P0_INT_VALID_OW */
  /*419*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_INT_ENABLED_OW */

    /* Port<0> Interrupt Cause */
  /*420*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PORT_INT_SUM */
  /*421*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_AN_RESTART */
  /*422*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_HCD_FOUND */
  /*423*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_AN_GOOD_CK */
  /*424*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_CH_SM_BP_REACHED_INT */
  /*425*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_SD_TX_SM_BP_REACHED_INT */
  /*426*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_SD_RX_SM_BP_REACHED_INT */
  /*427*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_INT_PROG_PWM_NORM_EN_MX_S */
  /*428*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_INT_PROG_PWM_PWRUP_EN_MX_S */
  /*429*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_INT_PROG_PWM_RXON_EN_MX_S */
  /*430*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_INT_PROG_PWM_RXSD_EN_MX_S */
  /*431*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_INT_PROG_PWM_TXON_EN_MX_S */
  /*432*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_INT_PROG_PWM_TXRXON_EN_MX_S */
  /*433*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_INT_PROG_PWM_TXRXSD_EN_MX_S */
  /*434*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_INT_PM_DSP_RXUP_TIME_OUT */
  /*435*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_INT_PM_PCS_LINK_TIMER_OUT */
  /*436*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_INT_RXON_WAIT_TIME_OUT */
  /*437*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_INT_TXON_WAIT_TIME_OUT */
  /*438*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_INT_TXRX_START_WAIT_TIME_OUT */
  /*439*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_INT_PM_PROG_RX_INIT_S */
  /*440*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_INT_PM_PROG_RX_TRAIN_S */
  /*441*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_INT_PM_PROG_TX_TRAIN_S */
  /*442*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_INT_PROG_RX_TIME_OUT */
  /*443*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_INT_PROG_TX_TIME_OUT */
  /*444*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_INT_RX_INIT_TIME_OUT */
  /*445*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_INT_RX_PLL_UP_TIME_OUT */
  /*446*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_INT_RX_TRAIN_TIME_OUT */
  /*447*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*P0_INT_TX_PLL_UP_TIME_OUT */
  /*448*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*P0_INT_TX_TRAIN_TIME_OUT */
  /*449*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*P0_INT_PROG_TIME_OUT */
  /*450*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*P0_INT_WAIT_PWRDN_TIME_OUT */

    /* Port<0> Interrupt Mask */
  /*451*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 30 }, /*P0_PORT_INT_MASK */

    /* Port<0> Status */
  /*452*/  {/*baseAddr*/ 0x1ec,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_REG_TX_TRAIN_COMPLETE */
  /*453*/  {/*baseAddr*/ 0x1ec,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_REG_RX_TRAIN_COMPLETE */
  /*454*/  {/*baseAddr*/ 0x1ec,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_REG_TX_TRAIN_FAILED */
  /*455*/  {/*baseAddr*/ 0x1ec,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_REG_RX_TRAIN_FAILED */
  /*456*/  {/*baseAddr*/ 0x1ec,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_PCS_CFG_DONE */
  /*457*/  {/*baseAddr*/ 0x1ec,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_REG_PROG_PWM_DONE_S */
  /*458*/  {/*baseAddr*/ 0x1ec,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_REG_PROG_RX_DONE_S */
  /*459*/  {/*baseAddr*/ 0x1ec,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_REG_PROG_TX_DONE_S */
  /*460*/  {/*baseAddr*/ 0x1ec,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_PIN_RXDCLK_4X_EN_VAL */
  /*461*/  {/*baseAddr*/ 0x1ec,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_PIN_TXDCLK_4X_EN_VAL */
  /*462*/  {/*baseAddr*/ 0x1ec,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_TXDATA_GRAY_CODE_EN */
  /*463*/  {/*baseAddr*/ 0x1ec,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_RXDATA_GRAY_CODE_EN */
  /*464*/  {/*baseAddr*/ 0x1ec,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_TXDATA_PRE_CODE_EN */
  /*465*/  {/*baseAddr*/ 0x1ec,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_RXDATA_PRE_CODE_EN */

    /* Port<0> AN restart counter */
  /*466*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_AN_RESTART_COUNTER */

    /* Port<0> DSP Lock Fail Counter */
  /*467*/  {/*baseAddr*/ 0x21c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_DSP_LOCK_FAIL_COUTNER */

    /* Port<0> Link Fail Counter */
  /*468*/  {/*baseAddr*/ 0x220,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_LINK_FAIL_COUNTER */

    /* Port<0> Cannel SM control */
  /*469*/  {/*baseAddr*/ 0x110,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*P0_CH_SM_OVERRIDE_CTRL */
  /*470*/  {/*baseAddr*/ 0x110,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_CH_SM_AMDISAM */
  /*471*/  {/*baseAddr*/ 0x110,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_CH_SM_BP_REACHED */
  /*472*/  {/*baseAddr*/ 0x110,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 5 }, /*P0_CH_SM_STATE */

    /* Port<0> SerDes TX SM control */
  /*473*/  {/*baseAddr*/ 0x114,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*P0_SD_TX_SM_OVERRIDE_CTRL */
  /*474*/  {/*baseAddr*/ 0x114,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_SD_TX_SM_AMDISAM */
  /*475*/  {/*baseAddr*/ 0x114,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_SD_TX_SM_BP_REACHED */
  /*476*/  {/*baseAddr*/ 0x114,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*P0_SD_TX_SM_STATE */

    /* Global Interrupt Mask */
  /*477*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*GLOBAL_INT_MASK */

    /* Global Interrupt Cause */
  /*478*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INT_SUM */
  /*479*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*ILLEGAL_ADDRESS_ACCESS */

    /* Global Speed Table2 */
  /*480*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*PM_SPEED_TABLE_27P5G */
  /*481*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 6 }, /*PM_SPEED_TABLE_56G */
  /*482*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 6 }, /*PM_SPEED_TABLE_53P125G */

    /* Global Speed Table1 */
  /*483*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*PM_SPEED_TABLE_10G */
  /*484*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 6 }, /*PM_SPEED_TABLE_10P3125G */
  /*485*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 6 }, /*PM_SPEED_TABLE_20P625G */
  /*486*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 6 }, /*PM_SPEED_TABLE_25P78125G */
  /*487*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 6 }, /*PM_SPEED_TABLE_26P5625G */

    /* Global Speed Table */
  /*488*/  {/*baseAddr*/ 0x7c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*PM_SPEED_TABLE_1P25G */
  /*489*/  {/*baseAddr*/ 0x7c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 6 }, /*PM_SPEED_TABLE_2P578125G */
  /*490*/  {/*baseAddr*/ 0x7c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 6 }, /*PM_SPEED_TABLE_3P125G */
  /*491*/  {/*baseAddr*/ 0x7c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 6 }, /*PM_SPEED_TABLE_5G */
  /*492*/  {/*baseAddr*/ 0x7c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 6 }, /*PM_SPEED_TABLE_5P15625G */

    /* Global AN train type */
  /*493*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_1G */
  /*494*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_2P5G */
  /*495*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_5G */
  /*496*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_10G */
  /*497*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_25G */
  /*498*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_40GR4 */
  /*499*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_40GR2 */
  /*500*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_50GR2 */
  /*501*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_50G */
  /*502*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_100GR4 */
  /*503*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_100GR2 */
  /*504*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_200GR4 */
  /*505*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_400GR8 */
  /*506*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_200GR8 */
  /*507*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*TX_TRAIN_COUPLE_ENABLE */

    /* Port<0> Control7 */
  /*508*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PLL_READY_RX_CLEAN_OW */
  /*509*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PLL_READY_RX_CLEAN_OW_VAL */
  /*510*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_PLL_READY_TX_CLEAN_OW */
  /*511*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_PLL_READY_TX_CLEAN_OW_VAL */
  /*512*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_REG_PROG_NORM_EN_S */
  /*513*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_REG_RXSTR_ABORT_RX_TRAIN_S */
  /*514*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_REG_PROG_PWRUP_EN_S */
  /*515*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_REG_PROG_RXON_EN_S */
  /*516*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_REG_PROG_RXSD_EN_S */
  /*517*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_REG_PROG_TXON_EN_S */
  /*518*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_REG_PROG_TXRXON_EN_S */
  /*519*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_REG_PROG_TXRXSD_EN_S */
  /*520*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_CNT_CLEAR_ON_SD_RST */
  /*521*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_DURATION_CLEAR_ON_SD_RST */
  /*522*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_RX_INIT_OK_CNT_CLEAR */
  /*523*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_RX_INIT_TIMEOUT_CLEAR */
  /*524*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_RX_TRAIN_FAILED_CNT_CLEAR */
  /*525*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_RX_TRAIN_OK_CNT_CLEAR */
  /*526*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_RX_TRAIN_TIMEOUT_CNT_CLEAR */
  /*527*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_TX_TRAIN_FAILED_CNT_CLEAR */
  /*528*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_TX_TRAIN_OK_CNT_CLEAR */
  /*529*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_TX_TRAIN_TIMEOUT_CNT_CLEAR */
  /*530*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_REG_RXSTR_ABORT_RX_INIT_S */
  /*531*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_DSP_RXSTR_ACK_S_OW */
  /*532*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_DSP_RXSTR_ACK_S_OW_VAL */
  /*533*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_DSP_TXSTR_ACK_S_OW */
  /*534*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*P0_DSP_TXSTR_ACK_S_OW_VAL */
  /*535*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*P0_REG_DFE_ADAPTATION_EN_S */
  /*536*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*P0_REG_DFE_AUTO_CTRL_S */
  /*537*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*P0_REG_DFE_FROZEN_S */
  /*538*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_REG_NORM_ST_SQ_DETECTED_MASK_S */

    /* Port<0> Control8 */
  /*539*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PM_TRAIN_TYPE_OW */
  /*540*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 2 }, /*P0_PM_TRAIN_TYPE_OW_VAL */
  /*541*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_RG_TXON_MAX_TIMER_INF */
  /*542*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_RG_RXON_MAX_TIMER_INF */
  /*543*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_REG_PROG_EN_RX_INIT_S */
  /*544*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_REG_PROG_EN_RX_TRAIN_S */
  /*545*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_RX_BUSY_NO_PWRDN */
  /*546*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_REG_PROG_EN_TX_TRAIN_S */
  /*547*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_PCS_CFG_DONE_FIN_S_OW */
  /*548*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_PCS_CFG_DONE_FIN_S_OW_VAL */
  /*549*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_REG_RX_AUTO_RE_TRAIN_S */
  /*550*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_REG_SEL_INIT_DONE_S */
  /*551*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_REG_TX_AUTO_RE_TRAIN_S */
  /*552*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_COMPHY_INT_ACK_MX_S_OW */
  /*553*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_COMPHY_INT_ACK_MX_S_OW_VAL */
  /*554*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_PROG_PWM_DONE_S_OW */
  /*555*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_PROG_PWM_DONE_S_OW_VAL */
  /*556*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_PROG_RX_DONE_S_OW */
  /*557*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_PROG_RX_DONE_S_OW_VAL */
  /*558*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_PROG_TX_DONE_S_OW */
  /*559*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_PROG_TX_DONE_S_OW_VAL */
  /*560*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_OPMODE_MATCH_MX_S_OW */
  /*561*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_OPMODE_MATCH_MX_S_OW_VAL */
  /*562*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_COUPLE_MATCH_MX_S_OW */
  /*563*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_COUPLE_MATCH_MX_S_OW_VAL */
  /*564*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_AP_MATCH_MX_S_OW */
  /*565*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*P0_AP_MATCH_MX_S_OW_VAL */
  /*566*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*P0_PG_EN_MX_S_OW */
  /*567*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*P0_PG_EN_MX_S_OW_VAL */
  /*568*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*P0_INT_DATA_OW */
  /*569*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_INT_CODE_OW */

    /* Port<0> CH TXON Max Timer */
  /*570*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_TXON_MAX_TIMER */

    /* Port<0> CH RXON Max Timer */
  /*571*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_RXON_MAX_TIMER */

    /* Port<0> Interrupt2 Mask */
  /*572*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 10 }, /*P0_PORT_INT2_MASK */

    /* Port<0> Interrupt2 Cause */
  /*573*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PORT_INT2_SUM */
  /*574*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_TX_TFIFO_W_ERR */
  /*575*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_TX_TFIFO_R_ERR */
  /*576*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_RX_TFIFO_W_ERR */
  /*577*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_RX_TFIFO_R_ERR */
  /*578*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_TX_TFIFO_FULL */
  /*579*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_TX_TFIFO_EMPTY */
  /*580*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_RX_TFIFO_FULL */
  /*581*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_RX_TFIFO_EMPTY */
  /*582*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_INT_TX_RESET_WAIT_TIME_OUT */
  /*583*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_INT_RX_RESET_WAIT_TIME_OUT */

    /* Port<0> SerDes RX SM control */
  /*584*/  {/*baseAddr*/ 0x118,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*P0_SD_RX_SM_OVERRIDE_CTRL */
  /*585*/  {/*baseAddr*/ 0x118,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_SD_RX_SM_AMDISAM */
  /*586*/  {/*baseAddr*/ 0x118,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_SD_RX_SM_BP_REACHED */
  /*587*/  {/*baseAddr*/ 0x118,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*P0_SD_RX_SM_STATE */

    /* Port<0> SD TXT OK Max Timer */
  /*588*/  {/*baseAddr*/ 0x1a8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_TX_TRAIN_OK_WAIT_TIMEOUT_S */

    /* Port<0> SD TXT Min Timer */
  /*589*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_TX_TRAIN_MIN_WAIT_S */

    /* Port<0> SD TX PLL UP Min Timer */
  /*590*/  {/*baseAddr*/ 0x1a0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_TX_PLL_UP_MIN_WAIT_S */

    /* Port<0> SD IDLE NAP Min Timer */
  /*591*/  {/*baseAddr*/ 0x19c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_TX_IDLE_WAIT_TIME_OTHER_S */

    /* Port<0> SD IDLE AP Min Timer */
  /*592*/  {/*baseAddr*/ 0x198,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_TX_IDLE_WAIT_TIME_AP_S */

    /* Port<0> SD IDLE RM Min Timer */
  /*593*/  {/*baseAddr*/ 0x194,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_TX_COMPHY_IDLE_REMOVE_MIN_WAIT_S */

    /* Port<0> SD RXT Max Timer */
  /*594*/  {/*baseAddr*/ 0x190,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_TRAIN_TIME_OUT_S */
  /*595*/  {/*baseAddr*/ 0x190,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_REG_RX_TRAIN_TIME_OUT_S_INF */

    /* Port<0> SD RXT OK Max Timer */
  /*596*/  {/*baseAddr*/ 0x18c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_TRAIN_OK_WAIT_TIMEOUT_S */

    /* Port<0> SD RXT Min Timer */
  /*597*/  {/*baseAddr*/ 0x188,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_TRAIN_MIN_WAIT_S */

    /* Port<0> SD RX PLL UP Min Timer */
  /*598*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_PLL_UP_MIN_WAIT_S */

    /* Port<0> SD RX INIT Max Timer */
  /*599*/  {/*baseAddr*/ 0x180,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_INIT_TIME_OUT_S */
  /*600*/  {/*baseAddr*/ 0x180,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_REG_RX_INIT_TIME_OUT_S_INF */

    /* Port<0> SD RX INIT Min Timer */
  /*601*/  {/*baseAddr*/ 0x17c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_INIT_MIN_WAIT_S */

    /* Port<0> SD PROG Max Timer */
  /*602*/  {/*baseAddr*/ 0x178,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_SD_PROG_MAX_TIME_S */
  /*603*/  {/*baseAddr*/ 0x178,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_REG_SD_PROG_MAX_TIME_S_INF */

    /* Port<0> SD PLL UP Max Timer */
  /*604*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_PLL_UP_TIME_OUT_S */
  /*605*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_REG_PLL_UP_TIME_OUT_S_INF */

    /* Port<0> CH PROG Max Timer */
  /*606*/  {/*baseAddr*/ 0x170,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_REG_PROG_MAX_TIME_S */

    /* Port<0> SD TXT Max Timer */
  /*607*/  {/*baseAddr*/ 0x1ac,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_TX_TRAIN_TIME_OUT_S */
  /*608*/  {/*baseAddr*/ 0x1ac,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_REG_TX_TRAIN_TIME_OUT_S_INF */

    /* Port<0> Status1 */
  /*609*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_STAT_DSP_RXSTR_REQ_MX_S */
  /*610*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_STAT_DSP_RXSTR_REQ_S */
  /*611*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_STAT_DSP_TXSTR_REQ_MX_S */
  /*612*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_STAT_DSP_TXSTR_REQ_S */
  /*613*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_STAT_PM_AN_EN_HCD_RESOLVED */
  /*614*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_STAT_PM_AN_HCD_CLEAR */
  /*615*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_STAT_PM_AN_HCD_RESOLVED */
  /*616*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_STAT_PM_AN_PCS_CLKOUT_SEL */
  /*617*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_STAT_PM_AN_PCS_SEL */
  /*618*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_STAT_PM_AN_RESTART */
  /*619*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_STAT_PM_AP_EN_S */
  /*620*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_STAT_PM_AP_MODE_S */
  /*621*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_STAT_PM_AP_RESET_RX_S */
  /*622*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_STAT_PM_AP_RESET_TX_S */
  /*623*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_STAT_PM_ENCLK_AP_FR_S */
  /*624*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_STAT_PM_ENCLK_AP_FT_S */
  /*625*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_STAT_PM_ENCLK_AP_SYS_S */
  /*626*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_100GR2 */
  /*627*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_100GR4 */
  /*628*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_107GR2 */
  /*629*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_10G */
  /*630*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_110GR4 */
  /*631*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_1G */
  /*632*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_200GR4 */
  /*633*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_200GR8 */
  /*634*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_25G */
  /*635*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_2P5G */
  /*636*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_400GR8 */
  /*637*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_40GR2 */
  /*638*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_40GR4 */
  /*639*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_428GR8 */
  /*640*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_50GR */

    /* Port<0> Status3 */
  /*641*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_PU_TX_S */
  /*642*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_SOFTRST_S */
  /*643*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_TXCLK_SYNC_EN_PLL_S */
  /*644*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_STAT_PM_SOFTRST_S */
  /*645*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_STAT_PM_ST_EN_S */
  /*646*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 2 }, /*P0_STAT_PM_TX_TRAIN_POLY_SEL_S */
  /*647*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_STAT_DSP_LOCK_S */
  /*648*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_STAT_DSP_PWRDN_ACK_S */
  /*649*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_STAT_DSP_RXDN_ACK_S */
  /*650*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_STAT_DSP_SIGDET_S */
  /*651*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_STAT_DSP_TXDN_ACK_S */
  /*652*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_STAT_PCS_LOCK_S */
  /*653*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_STAT_TX_READY_S */
  /*654*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_STAT_PLL_READY_RX_CLEAN_S */
  /*655*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_STAT_PLL_READY_RX_S */
  /*656*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_STAT_PLL_READY_TX_CLEAN_S */
  /*657*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_STAT_PLL_READY_TX_S */
  /*658*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_STAT_PM_DSP_LOCK_S */
  /*659*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_STAT_PM_DSP_RXDN_ACK_S */
  /*660*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_STAT_PM_DSP_RXSTR_ACK_S */
  /*661*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_STAT_PM_DSP_SIGDET_S */
  /*662*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_STAT_PM_DSP_TXDN_ACK_S */
  /*663*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_STAT_PM_DSP_TX_READY_S */
  /*664*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*P0_STAT_PM_DSP_TXSTR_ACK_S */
  /*665*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*P0_STAT_PM_RX_INIT_S */
  /*666*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*P0_STAT_PM_RX_TRAIN_ENABLE_S */
  /*667*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_DFE_EN_S */
  /*668*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_DFE_PAT_DIS_S */
  /*669*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_DFE_UPDATE_DIS_S */

    /* Port<0> Status2 */
  /*670*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_50GR2 */
  /*671*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_5G */
  /*672*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_CUSTOM */
  /*673*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_QSGMII */
  /*674*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_USGMII */
  /*675*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_USX10G */
  /*676*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_USX20G */
  /*677*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_USX2P5G */
  /*678*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_USX5G */
  /*679*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_STAT_PM_NORM_X_STATE_S */
  /*680*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_STAT_PM_NR_RESET_TX_S */
  /*681*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_STAT_PM_PCS_COUPLE_S */
  /*682*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_STAT_PM_PCS_RX_CLK_ENA */
  /*683*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_STAT_PM_PCS_SD_RX_RESET_ */
  /*684*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_STAT_PM_PCS_SD_TX_RESET_ */
  /*685*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_STAT_PM_PCS_TX_CLK_ENA */
  /*686*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_STAT_PM_PWRDN_S */
  /*687*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_COUPLE_MODE_EN_S */
  /*688*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 6 }, /*P0_STAT_PM_SD_PHY_GEN_RX_S */
  /*689*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 6 }, /*P0_STAT_PM_SD_PHY_GEN_TX_S */
  /*690*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_PU_PLL_S */
  /*691*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_PU_RX_S */

    /* Port<0> Status4 */
  /*692*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_TX_IDLE_S */
  /*693*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 2 }, /*P0_STAT_PM_TRAIN_TYPE_S */
  /*694*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_STAT_PM_TX_TRAIN_ENABLE_S */
  /*695*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_STAT_RX_INIT_DONE_S */
  /*696*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_STAT_RX_TRAIN_COMPLETE_S */
  /*697*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_STAT_RX_TRAIN_FAILED_S */
  /*698*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_STAT_SD_RX_DTL_CLAMP_S */
  /*699*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_STAT_SD_TXCLK_SYNC_START_OUT_S */
  /*700*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_STAT_SQ_DETECTED_LPF_S */
  /*701*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_STAT_TX_TRAIN_COMPLETE_S */
  /*702*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 2 }, /*P0_STAT_TX_TRAIN_ERROR_L */
  /*703*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 2 }, /*P0_STAT_TX_TRAIN_ERROR_S */
  /*704*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_STAT_TX_TRAIN_FAILED_S */
  /*705*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_STAT_PCS_CFG_DONE_FIN_S */
  /*706*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_STAT_SD_BUSY_RX_S */
  /*707*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_STAT_SD_BUSY_TX_S */
  /*708*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_STAT_PM_PU_RX_REQ_S */
  /*709*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_STAT_PM_PU_TX_REQ_S */
  /*710*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_STAT_PM_ST_NORMAL_S */
  /*711*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_STAT_PM_ST_PWRDN_S */
  /*712*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_STAT_PM_TX_IDLE_S */
  /*713*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 6 }, /*P0_STAT_SD_PHY_GEN_S */
  /*714*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 2 }, /*P0_STAT_CMEM_STATE */

    /* Port<0> Counter5 */
  /*715*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*P0_STAT_TX_TRAIN_OK_CNT */
  /*716*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 10 }, /*P0_STAT_TX_TRAIN_TIMEOUT_CNT */
  /*717*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 10 }, /*P0_PWRUP_CNT_S */

    /* Port<0> Counter4 */
  /*718*/  {/*baseAddr*/ 0x214,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*P0_STAT_RX_TRAIN_OK_CNT */
  /*719*/  {/*baseAddr*/ 0x214,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 10 }, /*P0_STAT_RX_TRAIN_TIMEOUT_CNT */
  /*720*/  {/*baseAddr*/ 0x214,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 10 }, /*P0_STAT_TX_TRAIN_FAILED_CNT */
  /*721*/  {/*baseAddr*/ 0x214,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 2 }, /*P0_STAT_TX_TRAIN_DURATION_L_HI */

    /* Port<0> Counter3 */
  /*722*/  {/*baseAddr*/ 0x210,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*P0_STAT_RX_INIT_OK_CNT */
  /*723*/  {/*baseAddr*/ 0x210,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 10 }, /*P0_STAT_RX_INIT_TIMEOUT_CNT */
  /*724*/  {/*baseAddr*/ 0x210,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 10 }, /*P0_STAT_RX_TRAIN_FAILED_CNT */
  /*725*/  {/*baseAddr*/ 0x210,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 2 }, /*P0_STAT_RX_TRAIN_DURATION_L_HI */

    /* Port<0> Counter2 */
  /*726*/  {/*baseAddr*/ 0x20c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_STAT_TX_TRAIN_DURATION_L */

    /* Port<0> Counter1 */
  /*727*/  {/*baseAddr*/ 0x208,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_STAT_RX_TRAIN_DURATION_L */

    /* Port<0> Counter */
  /*728*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_STAT_RX_INIT_DURATION_L */

    /* Port<0> CMD Line&lt;0&gt; Hi */
  /*729*/  {/*baseAddr*/ 0x324,  /*offsetFormula*/ 0xa90, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_CMD_LINE_0_HI */
  /*730*/  {/*baseAddr*/ 0x324,  /*offsetFormula*/ 0xa90, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*P0_CMD_LINE2_0_HI */

    /* Port<0> CMD Line&lt;0&gt; Lo */
  /*731*/  {/*baseAddr*/ 0x224,  /*offsetFormula*/ 0xa90, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_CMD_LINE0_LO */

    /* Port<0> Control9 */
  /*732*/  {/*baseAddr*/ 0x1e0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_INT_ENABLED_OW_VAL */
  /*733*/  {/*baseAddr*/ 0x1e0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_INT_VALID_OW_VAL */
  /*734*/  {/*baseAddr*/ 0x1e0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 8 }, /*P0_INT_CODE_OW_VAL */
  /*735*/  {/*baseAddr*/ 0x1e0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 22 }, /*P0_INT_DATA_OW_VAL */

    /* Port<0> Status5 */
  /*736*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*P0_STAT_PM_CMEM_ADDR_S */
  /*737*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*P0_STAT_INT_STATE */
  /*738*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 3 }, /*P0_STAT_PROG_STATE_S */
  /*739*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 5 }, /*P0_REG_TX_TFIFO_UW_W */
  /*740*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 5 }, /*P0_REG_TX_TFIFO_UW_R */
  /*741*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 5 }, /*P0_REG_RX_TFIFO_UW_W */
  /*742*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 5 }, /*P0_REG_RX_TFIFO_UW_R */
  /*743*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_STAT_PM_NR_RESET_RX_S */

    /* Port<0> CH PWRDN Min Timer */
  /*744*/  {/*baseAddr*/ 0x13c,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_REG_PWRDN_MIN_TIME_S */

    /* Port<0> SD RX WAIT SQ DET Min Timer */
  /*745*/  {/*baseAddr*/ 0x1b8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_WAIT_SQ_DET_MIN_WAIT_S */

    /* Port<0> SD RX WAIT PLUG Min Timer */
  /*746*/  {/*baseAddr*/ 0x1b4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_WAIT_PLUG_MIN_WAIT_S */

    /* Port<0> SD RX RETRAIN Min Timer */
  /*747*/  {/*baseAddr*/ 0x1b0,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_RETRAIN_TIME_S */

    /* Port<0> Control11 */
  /*748*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PU_RX_BOTH_IN_IDLE */
  /*749*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_REG_PM_SD_PU_RESET_ON_SFTRST_S */
  /*750*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_RX_INIT_S */
  /*751*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_RX_RESTR_S */
  /*752*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_RX_TRAIN_S */
  /*753*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_TX_RESTR_S */
  /*754*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_TX_TRAIN_S */
  /*755*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_NORM_S */
  /*756*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_PWRUP_S */
  /*757*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_RXON_S */
  /*758*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_RXSD_S */
  /*759*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_TXON_S */
  /*760*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_TXRXON_S */
  /*761*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_TXRXSD_S */
  /*762*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_PU_TX_BOTH_IN_IDLE */
  /*763*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_SD_SW_RESETN_OW */
  /*764*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_SD_SW_RESETN_OW_VAL */
  /*765*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_PM_CLOCKOUT_GATER_OW */
  /*766*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_PM_CLOCKOUT_GATER_OW_VAL */
  /*767*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_RG_WAIT_PWRDN_MAX_TIMER_INF */
  /*768*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 6 }, /*P0_PM_SD_PHY_GEN_TX_OW_VAL */
  /*769*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 6 }, /*P0_PM_SD_PHY_GEN_RX_OW_VAL */

    /* Port<0> Control10 */
  /*770*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PM_PU_RX_REQ_S_OW */
  /*771*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PM_PU_RX_REQ_S_OW_VAL */
  /*772*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_PM_PU_TX_REQ_S_OW */
  /*773*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_PM_PU_TX_REQ_S_OW_VAL */
  /*774*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_PM_ST_NORMAL_S_OW */
  /*775*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_PM_ST_NORMAL_S_OW_VAL */
  /*776*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_PM_ST_PWRDN_S_OW */
  /*777*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_PM_ST_PWRDN_S_OW_VAL */
  /*778*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_PM_TX_IDLE_S_OW */
  /*779*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_PM_TX_IDLE_S_OW_VAL */
  /*780*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_RESET_CORE_TX_ACK_ENABLE */
  /*781*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_RESET_CORE_RX_ACK_ENABLE */
  /*782*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_RESET_TX_CORE_ENABLE */
  /*783*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_RESET_RX_CORE_ENABLE */
  /*784*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P0_TXT_DSP_SIGDET_LOSS_PWRDN_INSTEAD_RXSTR */
  /*785*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*P0_TXT_IGNORE_DSP_LOCK_LOSS */
  /*786*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*P0_TXT_IGNORE_DSP_SIGDET_LOSS */
  /*787*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_TXT_IGNORE_TX_READY_LOSS */
  /*788*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_TXT_NO_DSP_LOCK_PWRDN_INSTEAD_RXSTR */
  /*789*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_TXT_RXSTR_PU_PLL_RX_VALUE */
  /*790*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_TXT_TX_READY_LOSS_PWRDN_INSTEAD_TXSTR */
  /*791*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_TXT_TXRX_ON_TIMEOUT_PWRDN_INSTEAD_RXSTR */
  /*792*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*P0_TXT_TXRX_SD_TIMEOUT_PWRDN_INSTEAD_RXSTR */
  /*793*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*P0_TXT_TXSTR_PU_PLL_TX_VALUE */
  /*794*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 26, /*fieldLen*/ 2 }, /*P0_REG_TX_TRAIN_DSP_SIGDET_SEL_S */
  /*795*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*P0_REG_RESET_EN_TX_TRAIN_S */
  /*796*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*P0_REG_RESET_EN_RX_TRAIN_S */
  /*797*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*P0_REG_RESET_EN_RX_INIT_S */
  /*798*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x2a4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*P0_REG_RESET_EN_PU_PLL_S */

    /* Global Control4 */
  /*799*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*REG_SD_DFE_UPDATE_DIS_SAMP */
  /*800*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 2 }, /*REG_SD_DFE_PAT_DIS_SAMP */
  /*801*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 2 }, /*REG_SD_DFE_EN_SAMP */
  /*802*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*REG_SD_TX_IDLE_SAMP */
  /*803*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 2 }, /*REG_TX_TRAIN_ENABLE_SAMP */
  /*804*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 2 }, /*REG_RX_TRAIN_ENABLE_SAMP */
  /*805*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 2 }, /*REG_RX_INIT_SAMP */
  /*806*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*REG_RX_IDLE_COUNT */
  /*807*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*REG_TX_IDLE_COUNT */
  /*808*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*PHY_GEN_TX_DONE_OW */
  /*809*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*PHY_GEN_TX_DONE_OW_VAL */
  /*810*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*PHY_GEN_TX_LOAD_OW */
  /*811*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PHY_GEN_TX_LOAD_OW_VAL */
  /*812*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*PHY_GEN_TX_START_CNT_DONE_OW */
  /*813*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*PHY_GEN_TX_START_CNT_DONE_OW_VAL */
  /*814*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*PHY_GEN_RX_DONE_OW */
  /*815*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*PHY_GEN_RX_DONE_OW_VAL */
  /*816*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*PHY_GEN_RX_LOAD_OW */
  /*817*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*PHY_GEN_RX_LOAD_OW_VAL */
  /*818*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*PHY_GEN_RX_START_CNT_DONE_OW */
  /*819*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*PHY_GEN_RX_START_CNT_DONE_OW_VAL */
  /*820*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*REG_PHY_GEN_DN_FRC */
  /*821*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*REG_TXSTR_REGRET_ENABLE */
  /*822*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*REG_RXSTR_REGRET_ENABLE */

    /* Global Control3 */
  /*823*/  {/*baseAddr*/ 0xa4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REG_NO_PRE_SELECTOR */
  /*824*/  {/*baseAddr*/ 0xa4,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*REG_HCD_RESOLVED_CLEAN */
  /*825*/  {/*baseAddr*/ 0xa4,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 6 }, /*REG_RESET_TO_CLOCK_CNT */
  /*826*/  {/*baseAddr*/ 0xa4,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 6 }, /*REG_CLOCK_TO_RESET_CNT */
  /*827*/  {/*baseAddr*/ 0xa4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 6 }, /*REG_START_AP_MODE_CNT */
  /*828*/  {/*baseAddr*/ 0xa4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 6 }, /*REG_START_SELECTOR_CNT */
  /*829*/  {/*baseAddr*/ 0xa4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 6 }, /*REG_START_UNGATE_CNT */

    /* Global Control2 */
  /*830*/  {/*baseAddr*/ 0xa0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*REG_PU_TX_CONF_DELAY */
  /*831*/  {/*baseAddr*/ 0xa0,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*REG_PU_RX_CONF_DELAY */
  /*832*/  {/*baseAddr*/ 0xa0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 5 }, /*REG_DSP_ON_CONF_DELAY */
  /*833*/  {/*baseAddr*/ 0xa0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 9 }, /*REG_TX_IDLE_CONF_DLY */
  /*834*/  {/*baseAddr*/ 0xa0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 5 }, /*REG_PCS_TX_ON_CONF_DELAY */
  /*835*/  {/*baseAddr*/ 0xa0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 27, /*fieldLen*/ 5 }, /*REG_PCS_RX_ON_CONF_DELAY */

    /* Global Phy Gen Load to Pup Timer */
  /*836*/  {/*baseAddr*/ 0xb8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 24 }, /*REG_PHY_GEN_LOAD_TO_PUP_TIMER */

    /* Global Phy Gen Pdn to Load Timer */
  /*837*/  {/*baseAddr*/ 0xb4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 24 }, /*REG_PHY_GEN_PDN_TO_LOAD_TIMER */

    /* Global SD RX Idle Min Wait */
  /*838*/  {/*baseAddr*/ 0xb0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*REG_RX_IDLE_MIN_WAIT_S */

    /* Global SD TX Idle Min Wait */
  /*839*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*REG_TX_IDLE_MIN_WAIT_S */

    /* Global Reset Rx Core */
  /*840*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*REG_RX_RESET_MIN_WAIT_S */

    /* Global Reset Tx Core */
  /*841*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*REG_TX_RESET_MIN_WAIT_S */

    /* Global Status */
  /*842*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*STAT_SPD_CFG_0 */
  /*843*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*STAT_SPD_CFG_1 */

    /* Global Control5 */
  /*844*/  {/*baseAddr*/ 0xbc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*PIN_SPD_CFG_FOR_R4_AND_R8 */
  /*845*/  {/*baseAddr*/ 0xbc,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*PIN_SPD_CFG_FOR_THE_REST */
  /*846*/  {/*baseAddr*/ 0xbc,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PIN_SPD_CFG_OW */
  /*847*/  {/*baseAddr*/ 0xbc,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 4 }, /*PIN_SPD_CFG_OW_VALUE */

    /* SD TX RESET WAIT Max Timer */
  /*848*/  {/*baseAddr*/ 0xcc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*REG_TX_RESET_WAIT_TIME_OUT_S */
  /*849*/  {/*baseAddr*/ 0xcc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*REG_TX_RESET_WAIT_TIME_OUT_S_INF */

    /* Speed Modulation 0 */
  /*850*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_0_MODULATION */
  /*851*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_1_MODULATION */
  /*852*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_2_MODULATION */
  /*853*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_3_MODULATION */
  /*854*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_4_MODULATION */
  /*855*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_5_MODULATION */
  /*856*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_6_MODULATION */
  /*857*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_7_MODULATION */
  /*858*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_8_MODULATION */
  /*859*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_9_MODULATION */
  /*860*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_10_MODULATION */
  /*861*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_11_MODULATION */
  /*862*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_12_MODULATION */
  /*863*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_13_MODULATION */
  /*864*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_14_MODULATION */
  /*865*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_15_MODULATION */
  /*866*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_16_MODULATION */
  /*867*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_17_MODULATION */
  /*868*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_18_MODULATION */
  /*869*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_19_MODULATION */
  /*870*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_20_MODULATION */
  /*871*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_21_MODULATION */
  /*872*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_22_MODULATION */
  /*873*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_23_MODULATION */
  /*874*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_24_MODULATION */
  /*875*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_25_MODULATION */
  /*876*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_26_MODULATION */
  /*877*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_27_MODULATION */
  /*878*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_28_MODULATION */
  /*879*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_29_MODULATION */
  /*880*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_30_MODULATION */
  /*881*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x4, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE_31_MODULATION */

    /* SD RX RESET WAIT Max Timer */
  /*882*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*REG_RX_RESET_WAIT_TIME_OUT_S */
  /*883*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*REG_RX_RESET_WAIT_TIME_OUT_S_INF */

    /* Train Enable Deassertion Timer */
  /*884*/  {/*baseAddr*/ 0xdc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*TRAIN_EN_DEASSERT_TIMER */
};

