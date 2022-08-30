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
* @file mvHwsMtiMac400RegDb.c
*
* @brief Hawk MTI MAC400/100 register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>


const MV_HWS_REG_ADDR_FIELD_STC hawkMtiMac100RegDb[] = {
    /* REVISION */
  /*0*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*CORE_REVISION */
  /*1*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*CORE_VERSION */
  /*2*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CUSTOMER_REVISION */

    /* SCRATCH */
  /*3*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*SCRATCH */

    /* COMMAND_CONFIG */
  /*4*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TX_ENA */
  /*5*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RX_ENA */
  /*6*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PROMIS_EN */
  /*7*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PAD_EN */
  /*8*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*CRC_FWD */
  /*9*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PAUSE_FWD */
  /*10*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PAUSE_IGNORE */
  /*11*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_ADDR_INS */
  /*12*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LOOPBACK_EN */
  /*13*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TX_PAD_EN */
  /*14*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*SW_RESET */
  /*15*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*CNTL_FRAME_ENA */
  /*16*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*RX_ERR_DISC */
  /*17*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PHY_TXENA */
  /*18*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*SEND_IDLE */
  /*19*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PFC_MODE */
  /*20*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*PAUSE_PFC_COMP */
  /*21*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*RX_SFD_ANY */
  /*22*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*TX_FLUSH */
  /*23*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*TX_LOWP_ENA */
  /*24*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*TX_FIFO_RESET */
  /*25*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*FLT_HDL_DIS */
  /*26*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*TX_PAUSE_DIS */
  /*27*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*RX_PAUSE_DIS */
  /*28*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*SHORT_PREAMBLE */
  /*29*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*NO_PREAMBLE */

    /* MAC_ADDR_0 */
  /*30*/  {/*baseAddr*/ 0x000c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MAC_ADDRESS_0 */

    /* MAC_ADDR_1 */
  /*31*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAC_ADDRESS_1 */

    /* FRM_LENGTH */
  /*32*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FRM_LENGTH */
  /*33*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*TX_MTU */

    /* RX_FIFO_SECTIONS */
  /*34*/  {/*baseAddr*/ 0x001c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RX_SECTION_FULL */
  /*35*/  {/*baseAddr*/ 0x001c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*RX_SECTION_EMPTY */

    /* TX_FIFO_SECTIONS */
  /*36*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*TX_SECTION_FULL */
  /*37*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*TX_SECTION_EMPTY */

    /* MDIO_CFG_STATUS */
  /*38*/  {/*baseAddr*/ 0x0030,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MDIO_BUSY */
  /*39*/  {/*baseAddr*/ 0x0030,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MDIO_READ_ERROR */
  /*40*/  {/*baseAddr*/ 0x0030,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 3 }, /*MDIO_HOLD_TIME_SETTING */
  /*41*/  {/*baseAddr*/ 0x0030,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*MDIO_DISABLE_PREAMBLE */
  /*42*/  {/*baseAddr*/ 0x0030,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*MDIO_CLAUSE45 */
  /*43*/  {/*baseAddr*/ 0x0030,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 9 }, /*MDIO_CLOCK_DIVISOR */

    /* MDIO_COMMAND */
  /*44*/  {/*baseAddr*/ 0x0034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*DEVICE_ADDRESS */
  /*45*/  {/*baseAddr*/ 0x0034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 5 }, /*PORT_ADDRESS */
  /*46*/  {/*baseAddr*/ 0x0034,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*READ_ADDRESS_POST_INCREMENT */
  /*47*/  {/*baseAddr*/ 0x0034,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*NORMAL_READ_TRANSACTION */

    /* MDIO_DATA */
  /*48*/  {/*baseAddr*/ 0x0038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MDIO_DATA */

    /* MDIO_REGADDR */
  /*49*/  {/*baseAddr*/ 0x003c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MDIO_REGADDR */

    /* STATUS */
  /*50*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*RX_LOC_FAULT */
  /*51*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RX_REM_FAULT */
  /*52*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PHY_LOS */
  /*53*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TS_AVAIL */
  /*54*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*RX_LOWP */
  /*55*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*TX_EMPTY */
  /*56*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*RX_EMPTY */
  /*57*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*RX_LINT_FAULT */
  /*58*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TX_IS_IDLE */

    /* TX_IPG_LENGTH */
  /*59*/  {/*baseAddr*/ 0x0044,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*TXIPG */
  /*60*/  {/*baseAddr*/ 0x0044,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*COMPENSATION */

    /* CRC_MODE */
  /*61*/  {/*baseAddr*/ 0x0048,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*DISABLE_RX_CRC_CHECK */
  /*62*/  {/*baseAddr*/ 0x0048,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*CRC_1BYTE */
  /*63*/  {/*baseAddr*/ 0x0048,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*CRC_2BYTE */
  /*64*/  {/*baseAddr*/ 0x0048,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*CRC_0BYTE */

    /* CL01_PAUSE_QUANTA */
  /*65*/  {/*baseAddr*/ 0x0054,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL0_PAUSE_QUANTA */
  /*66*/  {/*baseAddr*/ 0x0054,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL1_PAUSE_QUANTA */

    /* CL23_PAUSE_QUANTA */
  /*67*/  {/*baseAddr*/ 0x0058,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL2_PAUSE_QUANTA */
  /*68*/  {/*baseAddr*/ 0x0058,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL3_PAUSE_QUANTA */

    /* CL45_PAUSE_QUANTA */
  /*69*/  {/*baseAddr*/ 0x005c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL4_PAUSE_QUANTA */
  /*70*/  {/*baseAddr*/ 0x005c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL5_PAUSE_QUANTA */

    /* CL67_PAUSE_QUANTA */
  /*71*/  {/*baseAddr*/ 0x0060,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL6_PAUSE_QUANTA */
  /*72*/  {/*baseAddr*/ 0x0060,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL7_PAUSE_QUANTA */

    /* CL01_QUANTA_THRESH */
  /*73*/  {/*baseAddr*/ 0x0064,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL0_QUANTA_THRESH */
  /*74*/  {/*baseAddr*/ 0x0064,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL1_QUANTA_THRESH */

    /* CL23_QUANTA_THRESH */
  /*75*/  {/*baseAddr*/ 0x0068,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL2_QUANTA_THRESH */
  /*76*/  {/*baseAddr*/ 0x0068,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL3_QUANTA_THRESH */

    /* CL45_QUANTA_THRESH */
  /*77*/  {/*baseAddr*/ 0x006c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL4_QUANTA_THRESH */
  /*78*/  {/*baseAddr*/ 0x006c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL5_QUANTA_THRESH */

    /* CL67_QUANTA_THRESH */
  /*79*/  {/*baseAddr*/ 0x0070,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL6_QUANTA_THRESH */
  /*80*/  {/*baseAddr*/ 0x0070,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL7_QUANTA_THRESH */

    /* RX_PAUSE_STATUS */
  /*81*/  {/*baseAddr*/ 0x0074,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*PAUSESTATUS */

    /* TS_TIMESTAMP */
  /*82*/  {/*baseAddr*/ 0x007c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TS_TIMESTAMP */

    /* XIF_MODE */
  /*83*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*XGMII */
  /*84*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PAUSETIMERX8 */
  /*85*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*ONESTEPENA */
  /*86*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*RX_PAUSE_BYPASS */
  /*87*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TX_MAC_RS_ERR */
  /*88*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*CFG_1STEP_DELTA_MODE */
  /*89*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*CFG_1STEP_DELAY_MODE */
  /*90*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*CFG_1STEP_BINARY_MODE */
  /*91*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*CFG_1STEP_64UPD_MODE */
  /*92*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*CFG_RX_CNT_MODE */
  /*93*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*CFG_PFC_PULSE_MODE */
  /*94*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*CFG_PFC_LP_MODE */
  /*95*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*CFG_PFC_LP_16PRI */

    /* CL89_PAUSE_QUANTA */
  /*96*/  {/*baseAddr*/ 0x0084,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL8_PAUSE_QUANTA */
  /*97*/  {/*baseAddr*/ 0x0084,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL9_PAUSE_QUANTA */

    /* CL1011_PAUSE_QUANTA */
  /*98*/  {/*baseAddr*/ 0x0088,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL10_PAUSE_QUANTA */
  /*99*/  {/*baseAddr*/ 0x0088,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL11_PAUSE_QUANTA */

    /* CL1213_PAUSE_QUANTA */
  /*100*/  {/*baseAddr*/ 0x008c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL12_PAUSE_QUANTA */
  /*101*/  {/*baseAddr*/ 0x008c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL13_PAUSE_QUANTA */

    /* CL1415_PAUSE_QUANTA */
  /*102*/  {/*baseAddr*/ 0x0090,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL14_PAUSE_QUANTA */
  /*103*/  {/*baseAddr*/ 0x0090,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL15_PAUSE_QUANTA */

    /* CL89_QUANTA_THRESH */
  /*104*/  {/*baseAddr*/ 0x0094,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL8_QUANTA_THRESH */
  /*105*/  {/*baseAddr*/ 0x0094,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL9_QUANTA_THRESH */

    /* CL1011_QUANTA_THRESH */
  /*106*/  {/*baseAddr*/ 0x0098,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL10_QUANTA_THRESH */
  /*107*/  {/*baseAddr*/ 0x0098,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL11_QUANTA_THRESH */

    /* CL1213_QUANTA_THRESH */
  /*108*/  {/*baseAddr*/ 0x009c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL12_QUANTA_THRESH */
  /*109*/  {/*baseAddr*/ 0x009c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL13_QUANTA_THRESH */

    /* CL1415_QUANTA_THRESH */
  /*110*/  {/*baseAddr*/ 0x00a0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL14_QUANTA_THRESH */
  /*111*/  {/*baseAddr*/ 0x00a0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL15_QUANTA_THRESH */
};

const MV_HWS_REG_ADDR_FIELD_STC hawkMtiMac400RegDb[] = {
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
  /*7*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*CRC_FWD */
  /*8*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PAUSE_FWD */
  /*9*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PAUSE_IGNORE */
  /*10*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_ADDR_INS */
  /*11*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LOOPBACK_EN */
  /*12*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TX_PAD_EN */
  /*13*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*SW_RESET */
  /*14*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*CMD_FRAME_ENA */
  /*15*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PHY_TXENA */
  /*16*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*SEND_IDLE */
  /*17*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*NO_LGTH_CHECK */
  /*18*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PFC_MODE */
  /*19*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*PAUSE_PFC_COMP */
  /*20*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*RX_SFD_ANY */
  /*21*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*TX_FLUSH */
  /*22*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*TX_FIFO_RESET */
  /*23*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*FLT_HDL_DIS */
  /*24*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*INV_LOOP */

    /* MAC_ADDR_0 */
  /*25*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MAC_ADDRESS_0 */

    /* MAC_ADDR_1 */
  /*26*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAC_ADDRESS_1 */

    /* FRM_LENGTH */
  /*27*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FRM_LENGTH */
  /*28*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*FRM_LENGTH_TX_MTU */

    /* RX_FIFO_SECTIONS */
  /*29*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RX_SECTION_AVAIL */
  /*30*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*RX_SECTION_EMPTY */

    /* TX_FIFO_SECTIONS */
  /*31*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*TX_SECTION_AVAIL_THRESHOLD */
  /*32*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*TX_SECTION_EMPTY_THRESHOLD */

    /* HASHTABLE_LOAD */
  /*33*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*HASH_TABLE_ADDRESS */
  /*34*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*ENABLE_MULTICAST_FRAME */

    /* MDIO_CFG_STATUS */
  /*35*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MDIO_BUSY */
  /*36*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MDIO_READ_ERROR */
  /*37*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 3 }, /*MDIO_HOLD_TIME_SETTING */
  /*38*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*MDIO_DISABLE_PREAMBLE */
  /*39*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*MDIO_CLAUSE45 */
  /*40*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 9 }, /*MDIO_CLOCK_DIVISOR */

    /* MDIO_COMMAND */
  /*41*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*DEVICE_ADDRESS */
  /*42*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 5 }, /*PORT_ADDRESS */
  /*43*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*READ_ADDRESS_POST_INCREMENT */
  /*44*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*NORMAL_READ_TRANSACTION */

    /* MDIO_DATA */
  /*45*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MDIO_DATA */

    /* MDIO_REGADDR */
  /*46*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MDIO_REGADDR */

    /* STATUS */
  /*47*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*RX_LOC_FAULT */
  /*48*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RX_REM_FAULT */
  /*49*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PHY_LOS */
  /*50*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TS_AVAIL */
  /*51*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*TX_EMPTY */
  /*52*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*RX_EMPTY */
  /*53*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TX_ISIDLE */

    /* TX_IPG_LENGTH */
  /*54*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*TXIPG */
  /*55*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 13 }, /*TXIPGCOMPENSATION */

    /* CL01_PAUSE_QUANTA */
  /*56*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL0_PAUSE_QUANTA */
  /*57*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL1_PAUSE_QUANTA */

    /* CL23_PAUSE_QUANTA */
  /*58*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL2_PAUSE_QUANTA */
  /*59*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL3_PAUSE_QUANTA */

    /* CL45_PAUSE_QUANTA */
  /*60*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL4_PAUSE_QUANTA */
  /*61*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL5_PAUSE_QUANTA */

    /* CL67_PAUSE_QUANTA */
  /*62*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL6_PAUSE_QUANTA */
  /*63*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL7_PAUSE_QUANTA */

    /* CL01_QUANTA_THRESH */
  /*64*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL0_QUANTA_THRESH */
  /*65*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL1_QUANTA_THRESH */

    /* CL23_QUANTA_THRESH */
  /*66*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL2_QUANTA_THRESH */
  /*67*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL3_QUANTA_THRESH */

    /* CL45_QUANTA_THRESH */
  /*68*/  {/*baseAddr*/ 0x6c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL4_QUANTA_THRESH */
  /*69*/  {/*baseAddr*/ 0x6c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL5_QUANTA_THRESH */

    /* CL67_QUANTA_THRESH */
  /*70*/  {/*baseAddr*/ 0x70,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL6_QUANTA_THRESH */
  /*71*/  {/*baseAddr*/ 0x70,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL7_QUANTA_THRESH */

    /* RX_PAUSE_STATUS */
  /*72*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RX_PAUSE_STATUS */

    /* TS_TIMESTAMP */
  /*73*/  {/*baseAddr*/ 0x7c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TS_TIMESTAMP */

    /* XIF_MODE */
  /*74*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*ONE_STEP_ENA */
  /*75*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*PFC_PULSE_MODE */
  /*76*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*PFC_LP_MODE */
  /*77*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PFC_LP_16PRI */

    /* CL89_PAUSE_QUANTA */
  /*78*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL8_PAUSE_QUANTA */
  /*79*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL9_PAUSE_QUANTA */

    /* CL1011_PAUSE_QUANTA */
  /*80*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL10_PAUSE_QUANTA */
  /*81*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL11_PAUSE_QUANTA */

    /* CL1213_PAUSE_QUANTA */
  /*82*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL12_PAUSE_QUANTA */
  /*83*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL13_PAUSE_QUANTA */

    /* CL1415_PAUSE_QUANTA */
  /*84*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL14_PAUSE_QUANTA */
  /*85*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL15_PAUSE_QUANTA */

    /* CL89_QUANTA_THRESH */
  /*86*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL8_QUANTA_THRESH */
  /*87*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL9_QUANTA_THRESH */

    /* CL1011_QUANTA_THRESH */
  /*88*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL10_QUANTA_THRESH */
  /*89*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL11_QUANTA_THRESH */

    /* CL1213_QUANTA_THRESH */
  /*90*/  {/*baseAddr*/ 0x9c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL12_QUANTA_THRESH */
  /*91*/  {/*baseAddr*/ 0x9c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL13_QUANTA_THRESH */

    /* CL1415_QUANTA_THRESH */
  /*92*/  {/*baseAddr*/ 0xa0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL14_QUANTA_THRESH */
  /*93*/  {/*baseAddr*/ 0xa0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL15_QUANTA_THRESH */
};

