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
* @file mvHwsMtiMac100BrRegDb.c
*
* @brief Hawk MTI MAC100 BR register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>


const MV_HWS_REG_ADDR_FIELD_STC hawkMtiMac100BrRegDb[] = {
    /* REVISION */
  /*0*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*CORE_REVISION */
  /*1*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*CORE_VERSION */
  /*2*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CUSTOMER_REVISION */

    /* SCRATCH */
  /*3*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*SCRATCH */

    /* COMMAND_CONFIG */
  /*4*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TX_ENA */
  /*5*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RX_ENA */
  /*6*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PROMIS_EN */
  /*7*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PAD_EN */
  /*8*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*CRC_FWD */
  /*9*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PAUSE_FWD */
  /*10*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PAUSE_IGNORE */
  /*11*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_ADDR_INS */
  /*12*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LOOPBACK_EN */
  /*13*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TX_PAD_EN */
  /*14*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*SW_RESET */
  /*15*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*CNTL_FRAME_ENA */
  /*16*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PHY_TXENA */
  /*17*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*SEND_IDLE */
  /*18*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*PMAC_PAUSE_MASK_P */
  /*19*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*PMAC_PAUSE_MASK_E */
  /*20*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PFC_MODE */
  /*21*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*PAUSE_PFC_COMP */
  /*22*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*RX_SFD_ANY */
  /*23*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*TX_FLUSH */
  /*24*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*TX_LOWP_ENA */
  /*25*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*REG_LOWP_RXEMPTY */
  /*26*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*FLT_TX_STOP */
  /*27*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*TX_FIFO_RESET */
  /*28*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*FLT_HDL_DIS */
  /*29*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*PMAC_TX_PAUSE_DIS */
  /*30*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*PMAC_RX_PAUSE_DIS */
  /*31*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*SHORT_PREAMBLE */
  /*32*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*NO_PREAMBLE */

    /* PMAC_MAC_ADDR_0 */
  /*33*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MAC_ADDRESS_0 */

    /* PMAC_MAC_ADDR_1 */
  /*34*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAC_ADDRESS_1 */

    /* PMAC FRM_LENGTH */
  /*35*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FRM_LENGTH */
  /*36*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*TX_MTU */

    /* PMAC_RX_FIFO_SECTIONS */
  /*37*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RX_SECTION_FULL */
  /*38*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*RX_SECTION_EMPTY */

    /* PMAC_TX_FIFO_SECTIONS */
  /*39*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*TX_SECTION_FULL */
  /*40*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*TX_SECTION_EMPTY */

    /* MDIO_CFG_STATUS */
  /*41*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MDIO_BUSY */
  /*42*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MDIO_READ_ERROR */
  /*43*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 3 }, /*MDIO_HOLD_TIME_SETTING */
  /*44*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*MDIO_DISABLE_PREAMBLE */
  /*45*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*MDIO_CLAUSE45 */
  /*46*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 9 }, /*MDIO_CLOCK_DIVISOR */

    /* MDIO_COMMAND */
  /*47*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*DEVICE_ADDRESS */
  /*48*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 5 }, /*PORT_ADDRESS */
  /*49*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*READ_ADDRESS_POST_INCREMENT */
  /*50*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*NORMAL_READ_TRANSACTION */

    /* MDIO_DATA */
  /*51*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MDIO_DATA */

    /* MDIO_REGADDR */
  /*52*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MDIO_REGADDR */

    /* STATUS */
  /*53*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*RX_LOC_FAULT */
  /*54*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RX_REM_FAULT */
  /*55*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PHY_LOS */
  /*56*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TS_AVAIL */
  /*57*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*RX_LOWP */
  /*58*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PMAC_TX_EMPTY */
  /*59*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PMAC_RX_EMPTY */
  /*60*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*RX_LINT_FAULT */
  /*61*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TX_IS_IDLE */
  /*62*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_MERGE_ISIDLE */
  /*63*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*EMAC_TX_EMPTY */
  /*64*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*EMAC_RX_EMPTY */

    /* TX_IPG_LENGTH */
  /*65*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*TXIPG */
  /*66*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*COMPENSATION */

    /* CRC_MODE */
  /*67*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*DISABLE_RX_CRC_CHECK */
  /*68*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*CRC_1BYTE */
  /*69*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*CRC_2BYTE */
  /*70*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*CRC_0BYTE */

    /* PMAC_CL01_PAUSE_QUANTA */
  /*71*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL0_PAUSE_QUANTA */
  /*72*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL1_PAUSE_QUANTA */

    /* PMAC_CL23_PAUSE_QUANTA */
  /*73*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL2_PAUSE_QUANTA */
  /*74*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL3_PAUSE_QUANTA */

    /* PMAC_CL45_PAUSE_QUANTA */
  /*75*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL4_PAUSE_QUANTA */
  /*76*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL5_PAUSE_QUANTA */

    /* PMAC_CL67_PAUSE_QUANTA */
  /*77*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL6_PAUSE_QUANTA */
  /*78*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL7_PAUSE_QUANTA */

    /* PMAC_CL01_QUANTA_THRESH */
  /*79*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL0_QUANTA_THRESH */
  /*80*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL1_QUANTA_THRESH */

    /* PMAC_CL23_QUANTA_THRESH */
  /*81*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL2_QUANTA_THRESH */
  /*82*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL3_QUANTA_THRESH */

    /* PMAC_CL45_QUANTA_THRESH */
  /*83*/  {/*baseAddr*/ 0x6c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL4_QUANTA_THRESH */
  /*84*/  {/*baseAddr*/ 0x6c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL5_QUANTA_THRESH */

    /* PMAC_CL67_QUANTA_THRESH */
  /*85*/  {/*baseAddr*/ 0x70,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL6_QUANTA_THRESH */
  /*86*/  {/*baseAddr*/ 0x70,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL7_QUANTA_THRESH */

    /* TS_TIMESTAMP */
  /*87*/  {/*baseAddr*/ 0x7c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TS_TIMESTAMP */

    /* XIF_MODE */
  /*88*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*XGMII */
  /*89*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PAUSETIMERX8 */
  /*90*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*ONESTEPENA */
  /*91*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*EMAC_RX_PAUSE_BYPASS */
  /*92*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PMAC_RX_PAUSE_BYPASS */
  /*93*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TX_MAC_RS_ERR */
  /*94*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*CFG_1STEP_DELTA_MODE */
  /*95*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*CFG_1STEP_DELAY_MODE */
  /*96*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*CFG_1STEP_BINARY_MODE */
  /*97*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*CFG_1STEP_64UPD_MODE */
  /*98*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*CFG_PFC_PULSE_MODE */

    /* PMAC_CL89_PAUSE_QUANTA */
  /*99*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL8_PAUSE_QUANTA */
  /*100*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL9_PAUSE_QUANTA */

    /* PMAC_CL1011_PAUSE_QUANTA */
  /*101*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL10_PAUSE_QUANTA */
  /*102*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL11_PAUSE_QUANTA */

    /* PMAC_CL1213_PAUSE_QUANTA */
  /*103*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL12_PAUSE_QUANTA */
  /*104*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL13_PAUSE_QUANTA */

    /* PMAC_CL1415_PAUSE_QUANTA */
  /*105*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL14_PAUSE_QUANTA */
  /*106*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL15_PAUSE_QUANTA */

    /* PMAC_CL89_QUANTA_THRESH */
  /*107*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL8_QUANTA_THRESH */
  /*108*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL9_QUANTA_THRESH */

    /* PMAC_CL1011_QUANTA_THRESH */
  /*109*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL10_QUANTA_THRESH */
  /*110*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL11_QUANTA_THRESH */

    /* PMAC_CL1213_QUANTA_THRESH */
  /*111*/  {/*baseAddr*/ 0x9c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL12_QUANTA_THRESH */
  /*112*/  {/*baseAddr*/ 0x9c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL13_QUANTA_THRESH */

    /* PMAC_CL1415_QUANTA_THRESH */
  /*113*/  {/*baseAddr*/ 0xa0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL14_QUANTA_THRESH */
  /*114*/  {/*baseAddr*/ 0xa0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL15_QUANTA_THRESH */

    /* BR_RX_FRAG_COUNT */
  /*115*/  {/*baseAddr*/ 0xb8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RX_FRAG_COUNT */

    /* BR_RX_VERIFY_COUNT */
  /*116*/  {/*baseAddr*/ 0xe0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*RX_VERIFY_COUNT_GOOD */
  /*117*/  {/*baseAddr*/ 0xe0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*RX_VERIFY_COUNT_BAD */

    /* EMAC_CL01_PAUSE_QUANTA */
  /*118*/  {/*baseAddr*/ 0x154,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL0_PAUSE_QUANTA */
  /*119*/  {/*baseAddr*/ 0x154,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL1_PAUSE_QUANTA */

    /* EMAC_MAC_ADDR_1 */
  /*120*/  {/*baseAddr*/ 0x110,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAC_ADDRESS_1 */

    /* BR_RX_ASSY_ERR_COUNT */
  /*121*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*RX_ASSY_ERR_COUNT */

    /* EMAC_RX_FIFO_SECTIONS */
  /*122*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RX_SECTION_FULL */
  /*123*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*RX_SECTION_EMPTY */

    /* BR_RX_ASSY_OK COUNT */
  /*124*/  {/*baseAddr*/ 0xd8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RX_ASSY_OK_COUNT */

    /* BR_RX_RESPONSE_COUNT */
  /*125*/  {/*baseAddr*/ 0xe4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*RX_RESP_COUNT_GOOD */
  /*126*/  {/*baseAddr*/ 0xe4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*RX_RESP_COUNT_BAD */

    /* BR_CONTROL */
  /*127*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TX_PREEMPT_EN */
  /*128*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*TX_VERIF_DIS */
  /*129*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 3 }, /*TX_ADDFRAGSIZE */
  /*130*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 7 }, /*TX_VERIFY_TIME */
  /*131*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*TX_PAUSE_PRI_EN */
  /*132*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*TX_FRAG_PAUSE_EN */
  /*133*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*TX_NON_EMPTY_PREEMPT_EN */
  /*134*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*TX_ALLOW_PMAC_IF_NVERIF */
  /*135*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*TX_ALLOW_EMAC_IF_NVERIF */
  /*136*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*BR_COUNT_CLR_ON_RD */
  /*137*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*BR_COUNT_SAT */
  /*138*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*RX_STRICT_PREAMBLE */
  /*139*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*PMAC_TO_EMAC_STATS */
  /*140*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*BR_RX_SMD_DIS */

    /* BR_TX_HOLD_COUNT */
  /*141*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TX_HOLD_COUNT */

    /* BR_TX_VERIF_COUNT */
  /*142*/  {/*baseAddr*/ 0xe8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*TX_VERIF_COUNT */
  /*143*/  {/*baseAddr*/ 0xe8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*TX_RESP_COUNT */

    /* EMAC_CL45_QUANTA_THRESH */
  /*144*/  {/*baseAddr*/ 0x16c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL4_QUANTA_THRESH */
  /*145*/  {/*baseAddr*/ 0x16c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL5_QUANTA_THRESH */

    /* EMAC_CL23_QUANTA_THRESH */
  /*146*/  {/*baseAddr*/ 0x168,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL2_QUANTA_THRESH */
  /*147*/  {/*baseAddr*/ 0x168,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL3_QUANTA_THRESH */

    /* EMAC_CL1213_PAUSE_QUANTA */
  /*148*/  {/*baseAddr*/ 0x18c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL12_PAUSE_QUANTA */
  /*149*/  {/*baseAddr*/ 0x18c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL13_PAUSE_QUANTA */

    /* EMAC_CL1415_PAUSE_QUANTA */
  /*150*/  {/*baseAddr*/ 0x190,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL14_PAUSE_QUANTA */
  /*151*/  {/*baseAddr*/ 0x190,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL15_PAUSE_QUANTA */

    /* EMAC_CL1213_QUANTA_THRESH */
  /*152*/  {/*baseAddr*/ 0x19c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL12_QUANTA_THRESH */
  /*153*/  {/*baseAddr*/ 0x19c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL13_QUANTA_THRESH */

    /* EMAC_MAC_ADDR_0 */
  /*154*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MAC_ADDRESS_0 */

    /* BR_RX_SMD_ERR_COUNT */
  /*155*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*RX_SMD_ERR_COUNT */

    /* EMAC_CL45_PAUSE_QUANTA */
  /*156*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL4_PAUSE_QUANTA */
  /*157*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL5_PAUSE_QUANTA */

    /* EMAC_CL67_QUANTA_THRESH */
  /*158*/  {/*baseAddr*/ 0x170,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL6_QUANTA_THRESH */
  /*159*/  {/*baseAddr*/ 0x170,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL7_QUANTA_THRESH */

    /* EMAC_CL01_QUANTA_THRESH */
  /*160*/  {/*baseAddr*/ 0x164,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL0_QUANTA_THRESH */
  /*161*/  {/*baseAddr*/ 0x164,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL1_QUANTA_THRESH */

    /* EMAC_CL67_PAUSE_QUANTA */
  /*162*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL6_PAUSE_QUANTA */
  /*163*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL7_PAUSE_QUANTA */

    /* EMAC_CL23_PAUSE_QUANTA */
  /*164*/  {/*baseAddr*/ 0x158,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL2_PAUSE_QUANTA */
  /*165*/  {/*baseAddr*/ 0x158,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL3_PAUSE_QUANTA */

    /* EMAC_CL89_QUANTA_THRESH */
  /*166*/  {/*baseAddr*/ 0x194,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL8_QUANTA_THRESH */
  /*167*/  {/*baseAddr*/ 0x194,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL9_QUANTA_THRESH */

    /* EMAC_CL1415_QUANTA_THRESH */
  /*168*/  {/*baseAddr*/ 0x1a0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL14_QUANTA_THRESH */
  /*169*/  {/*baseAddr*/ 0x1a0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL15_QUANTA_THRESH */

    /* BR_TX_FRAG_COUNT */
  /*170*/  {/*baseAddr*/ 0xb0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TX_FRAG_COUNT */

    /* EMAC_CL89_PAUSE_QUANTA */
  /*171*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL8_PAUSE_QUANTA */
  /*172*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL9_PAUSE_QUANTA */

    /* EMAC_TX_FIFO_SECTIONS */
  /*173*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*TX_SECTION_FULL */
  /*174*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*TX_SECTION_EMPTY */

    /* BR_STATUS */
  /*175*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*TX_VERIFY_STATUS */
  /*176*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TX_PREEMPT_STATUS */
  /*177*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*ASSY_ERR_LH */
  /*178*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*SMD_ERR_LH */
  /*179*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*VERIF_SEEN_LH */
  /*180*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*RESP_SEEN_LH */

    /* EMAC_CL1011_PAUSE_QUANTA */
  /*181*/  {/*baseAddr*/ 0x188,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL10_PAUSE_QUANTA */
  /*182*/  {/*baseAddr*/ 0x188,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL11_PAUSE_QUANTA */

    /* EMAC_CL1011_QUANTA_THRESH */
  /*183*/  {/*baseAddr*/ 0x198,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL10_QUANTA_THRESH */
  /*184*/  {/*baseAddr*/ 0x198,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL11_QUANTA_THRESH */

    /* EMAC_RX_PAUSE_STATUS */
  /*185*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*EMAC_RX_PAUSE_STATUS */

    /* EMAC FRM_LENGTH */
  /*186*/  {/*baseAddr*/ 0x114,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FRM_LENGTH */
  /*187*/  {/*baseAddr*/ 0x114,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*TX_MTU */

    /* EMAC_COMMAND_CONFIG */
  /*188*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TX_ENA */
  /*189*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RX_ENA */
  /*190*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PROMIS_EN */
  /*191*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PAUSE_FWD */
  /*192*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PAUSE_IGNORE */
  /*193*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_ADDR_INS */
  /*194*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LOOPBACK_EN */
  /*195*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*SW_RESET */
  /*196*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*CNTL_FRAME_ENA */
  /*197*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*EMAC_PAUSE_MASK_P */
  /*198*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*EMAC_PAUSE_MASK_E */
  /*199*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PFC_MODE */
  /*200*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*PAUSE_PFC_COMP */
  /*201*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*TX_FLUSH */
  /*202*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*TX_FIFO_RESET */
  /*203*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*EMAC_TX_PAUSE_DIS */
  /*204*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*EMAC_RX_PAUSE_DIS */
};


