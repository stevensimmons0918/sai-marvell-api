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
* @file mvHwsMacCpuRegDb.c
*
* @brief Hawk CPU MAC register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>


const MV_HWS_REG_ADDR_FIELD_STC hawkCpuMacUnitsDb[] = {
    /* REVISION */
  /*0*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*CORE_REVISION */
  /*1*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*CORE_VERSION */
  /*2*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CUSTOMER_REVISION */

    /* SCRATCH */
  /*3*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*SCRATCH */

    /* COMMAND_CONFIG */
  /*4*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TX_ENA */
  /*5*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RX_ENA */
  /*6*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PROMIS_EN */
  /*7*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PAD_EN */
  /*8*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*CRC_FWD */
  /*9*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PAUSE_FWD */
  /*10*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PAUSE_IGNORE */
  /*11*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_ADDR_INS */
  /*12*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LOOPBACK_EN */
  /*13*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TX_PAD_EN */
  /*14*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*SW_RESET */
  /*15*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*CNTL_FRAME_ENA */
  /*16*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*RX_ERR_DISC */
  /*17*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PHY_TXENA */
  /*18*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*SEND_IDLE */
  /*19*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PFC_MODE */
  /*20*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*PAUSE_PFC_COMP */
  /*21*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*RX_SFD_ANY */
  /*22*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*TX_FLUSH */
  /*23*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*TX_LOWP_ENA */
  /*24*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*TX_FIFO_RESET */
  /*25*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*FLT_HDL_DIS */
  /*26*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*SHORT_PREAMBLE */
  /*27*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*NO_PREAMBLE */

    /* MAC_ADDR_0 */
  /*28*/  {/*baseAddr*/ 0x000c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MAC_ADDRESS_0 */

    /* MAC_ADDR_1 */
  /*29*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAC_ADDRESS_1 */

    /* FRM_LENGTH */
  /*30*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FRM_LENGTH */
  /*31*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*TX_MTU */

    /* RX_FIFO_SECTIONS */
  /*32*/  {/*baseAddr*/ 0x001c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RX_SECTION_FULL */
  /*33*/  {/*baseAddr*/ 0x001c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*RX_SECTION_EMPTY */

    /* TX_FIFO_SECTIONS */
  /*34*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*TX_SECTION_FULL */
  /*35*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*TX_SECTION_EMPTY */

    /* MDIO_CFG_STATUS */
  /*36*/  {/*baseAddr*/ 0x0030,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MDIO_BUSY */
  /*37*/  {/*baseAddr*/ 0x0030,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MDIO_READ_ERROR */
  /*38*/  {/*baseAddr*/ 0x0030,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 3 }, /*MDIO_HOLD_TIME_SETTING */
  /*39*/  {/*baseAddr*/ 0x0030,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*MDIO_DISABLE_PREAMBLE */
  /*40*/  {/*baseAddr*/ 0x0030,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*MDIO_CLAUSE45 */
  /*41*/  {/*baseAddr*/ 0x0030,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 9 }, /*MDIO_CLOCK_DIVISOR */

    /* MDIO_COMMAND */
  /*42*/  {/*baseAddr*/ 0x0034,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*DEVICE_ADDRESS */
  /*43*/  {/*baseAddr*/ 0x0034,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 5 }, /*PORT_ADDRESS */
  /*44*/  {/*baseAddr*/ 0x0034,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*READ_ADDRESS_POST_INCREMENT */
  /*45*/  {/*baseAddr*/ 0x0034,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*NORMAL_READ_TRANSACTION */

    /* MDIO_DATA */
  /*46*/  {/*baseAddr*/ 0x0038,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MDIO_DATA */

    /* MDIO_REGADDR */
  /*47*/  {/*baseAddr*/ 0x003c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MDIO_REGADDR */

    /* STATUS */
  /*48*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*RX_LOC_FAULT */
  /*49*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RX_REM_FAULT */
  /*50*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PHY_LOS */
  /*51*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TS_AVAIL */
  /*52*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*RX_LOWP */
  /*53*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*TX_EMPTY */
  /*54*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*RX_EMPTY */
  /*55*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*RX_LINT_FAULT */
  /*56*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TX_IS_IDLE */

    /* TX_IPG_LENGTH */
  /*57*/  {/*baseAddr*/ 0x0044,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*TXIPG */
  /*58*/  {/*baseAddr*/ 0x0044,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*COMPENSATION */

    /* CRC_MODE */
  /*59*/  {/*baseAddr*/ 0x0048,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*DISABLE_RX_CRC_CHECK */
  /*60*/  {/*baseAddr*/ 0x0048,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*CRC_1BYTE */
  /*61*/  {/*baseAddr*/ 0x0048,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*CRC_2BYTE */
  /*62*/  {/*baseAddr*/ 0x0048,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*CRC_0BYTE */

    /* CL01_PAUSE_QUANTA */
  /*63*/  {/*baseAddr*/ 0x0054,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL0_PAUSE_QUANTA */
  /*64*/  {/*baseAddr*/ 0x0054,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL1_PAUSE_QUANTA */

    /* CL23_PAUSE_QUANTA */
  /*65*/  {/*baseAddr*/ 0x0058,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL2_PAUSE_QUANTA */
  /*66*/  {/*baseAddr*/ 0x0058,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL3_PAUSE_QUANTA */

    /* CL45_PAUSE_QUANTA */
  /*67*/  {/*baseAddr*/ 0x005c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL4_PAUSE_QUANTA */
  /*68*/  {/*baseAddr*/ 0x005c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL5_PAUSE_QUANTA */

    /* CL67_PAUSE_QUANTA */
  /*69*/  {/*baseAddr*/ 0x0060,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL6_PAUSE_QUANTA */
  /*70*/  {/*baseAddr*/ 0x0060,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL7_PAUSE_QUANTA */

    /* CL01_QUANTA_THRESH */
  /*71*/  {/*baseAddr*/ 0x0064,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL0_QUANTA_THRESH */
  /*72*/  {/*baseAddr*/ 0x0064,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL1_QUANTA_THRESH */

    /* CL23_QUANTA_THRESH */
  /*73*/  {/*baseAddr*/ 0x0068,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL2_QUANTA_THRESH */
  /*74*/  {/*baseAddr*/ 0x0068,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL3_QUANTA_THRESH */

    /* CL45_QUANTA_THRESH */
  /*75*/  {/*baseAddr*/ 0x006c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL4_QUANTA_THRESH */
  /*76*/  {/*baseAddr*/ 0x006c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL5_QUANTA_THRESH */

    /* CL67_QUANTA_THRESH */
  /*77*/  {/*baseAddr*/ 0x0070,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL6_QUANTA_THRESH */
  /*78*/  {/*baseAddr*/ 0x0070,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL7_QUANTA_THRESH */

    /* RX_PAUSE_STATUS */
  /*79*/  {/*baseAddr*/ 0x0074,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*PAUSESTATUS */

    /* TS_TIMESTAMP */
  /*80*/  {/*baseAddr*/ 0x007c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TS_TIMESTAMP */

    /* XIF_MODE */
  /*81*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*XGMII */
  /*82*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PAUSETIMERX8 */
  /*83*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*ONESTEPENA */
  /*84*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*RX_PAUSE_BYPASS */
  /*85*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TX_MAC_RS_ERR */

    /* CL89_PAUSE_QUANTA */
  /*86*/  {/*baseAddr*/ 0x0084,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL8_PAUSE_QUANTA */
  /*87*/  {/*baseAddr*/ 0x0084,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL9_PAUSE_QUANTA */

    /* CL1011_PAUSE_QUANTA */
  /*88*/  {/*baseAddr*/ 0x0088,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL10_PAUSE_QUANTA */
  /*89*/  {/*baseAddr*/ 0x0088,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL11_PAUSE_QUANTA */

    /* CL1213_PAUSE_QUANTA */
  /*90*/  {/*baseAddr*/ 0x008c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL12_PAUSE_QUANTA */
  /*91*/  {/*baseAddr*/ 0x008c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL13_PAUSE_QUANTA */

    /* CL1415_PAUSE_QUANTA */
  /*92*/  {/*baseAddr*/ 0x0090,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL14_PAUSE_QUANTA */
  /*93*/  {/*baseAddr*/ 0x0090,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL15_PAUSE_QUANTA */

    /* CL89_QUANTA_THRESH */
  /*94*/  {/*baseAddr*/ 0x0094,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL8_QUANTA_THRESH */
  /*95*/  {/*baseAddr*/ 0x0094,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL9_QUANTA_THRESH */

    /* CL1011_QUANTA_THRESH */
  /*96*/  {/*baseAddr*/ 0x0098,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL10_QUANTA_THRESH */
  /*97*/  {/*baseAddr*/ 0x0098,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL11_QUANTA_THRESH */

    /* CL1213_QUANTA_THRESH */
  /*98*/  {/*baseAddr*/ 0x009c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL12_QUANTA_THRESH */
  /*99*/  {/*baseAddr*/ 0x009c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL13_QUANTA_THRESH */

    /* CL1415_QUANTA_THRESH */
  /*100*/  {/*baseAddr*/ 0x00a0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL14_QUANTA_THRESH */
  /*101*/  {/*baseAddr*/ 0x00a0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL15_QUANTA_THRESH */
};
