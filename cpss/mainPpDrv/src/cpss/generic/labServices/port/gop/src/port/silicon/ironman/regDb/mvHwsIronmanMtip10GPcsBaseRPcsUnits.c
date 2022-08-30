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
* @file mvHwsIronmanMtip10GPcsBaseRPcsUnits.c
*
* @brief IronmanL 10G PCS BASE_R PCS register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanBaseRPcsUnitsDb[] = {
    /* DEVICE_ID0 */
  /*0*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER */

    /* PKG_ID0 */
  /*1*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER */

    /* BASER_TEST_ERR_CNT */
  /*2*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*COUNTER */

    /* VENDORMX_CORE_REV */
  /*3*/  {/*baseAddr*/ 0xf4,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*REVISION */

    /* STATUS2 */
  /*4*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*C10GBASE_R */
  /*5*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*C10GBASE_X */
  /*6*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*C10GBASE_W */
  /*7*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*C10GBASE_T */
  /*8*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*C40GBASE_R */
  /*9*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*C100GBASE_R */
  /*10*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*C25GBASE_R */
  /*11*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*C50GBASE_R */
  /*12*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*RECEIVE_FAULT */
  /*13*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TRANSMIT_FAULT */
  /*14*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 2 }, /*DEVICE_PRESENT */

    /* SEED_B2 */
  /*15*/  {/*baseAddr*/ 0xa0,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* VENDORMX_TXLANE_THRESH */
  /*16*/  {/*baseAddr*/ 0xfc,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*THRESHOLD */

    /* SPEED_ABILITY */
  /*17*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*C10GETH */
  /*18*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*C10PASS_TS */
  /*19*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*C40G */
  /*20*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*C100G */
  /*21*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*C25G */
  /*22*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x100, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*C50G */

    /* WAKE_ERR_COUNTER */
  /*23*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*COUNTER */

    /* SEED_B0 */
  /*24*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* DEVICE_ID1 */
  /*25*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER */

    /* BASER_STATUS1 */
  /*26*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK */
  /*27*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*HIGH_BER */
  /*28*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x100, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*RECEIVE_LINK */

    /* SEED_A3 */
  /*29*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*SEED */

    /* SEED_B3 */
  /*30*/  {/*baseAddr*/ 0xa4,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*SEED */

    /* BER_HIGH_ORDER_CNT */
  /*31*/  {/*baseAddr*/ 0xb0,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*BER_COUNTER */

    /* CONTROL1 */
  /*32*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 4 }, /*SPEED_SELECTION */
  /*33*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x100, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SPEED_ALWAYS1 */
  /*34*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LOW_POWER */
  /*35*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x100, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SPEED_SELECT_ALWAYS1 */
  /*36*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LOOPBACK */
  /*37*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x100, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RESET */

    /* DEVICES_IN_PKG1 */
  /*38*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*CLAUSE22 */
  /*39*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PMD_PMA */
  /*40*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*WIS_PRES */
  /*41*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PCS_PRES */
  /*42*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PHY_XS */
  /*43*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x100, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*DTE_XS */
  /*44*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x100, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*TC_PRES */

    /* BASER_TEST_CONTROL */
  /*45*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*DATA_PATTERN_SEL */
  /*46*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SELECT_SQUARE */
  /*47*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*RX_TESTPATTERN */
  /*48*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TX_TESTPATTERN */
  /*49*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SELECT_RANDOM */

    /* DEVICES_IN_PKG2 */
  /*50*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x100, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*CLAUSE22 */
  /*51*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*DEVICE1 */
  /*52*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x100, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*DEVICE2 */

    /* EEE_CTRL_CAPABILITY */
  /*53*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LPI_FW */
  /*54*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x100, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*EEE_10GBASE_KR */
  /*55*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*EEE_40GBASE_RAWAKE */
  /*56*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x100, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*EEE_40GBASE_RSLEEP */
  /*57*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*EEE_25GBASE_RAWAKE */
  /*58*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*EEE_25GBASE_RSLEEP */
  /*59*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x100, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*EEE_100GBASE_RAWAKE */
  /*60*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*EEE_50GBASE_RAWAKE */

    /* BASER_STATUS2 */
  /*61*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*ERRORED_CNT */
  /*62*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 6 }, /*BER_COUNTER */
  /*63*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*HIGH_BER */
  /*64*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x100, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*BLOCK_LOCK */

    /* SEED_A0 */
  /*65*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* CONTROL2 */
  /*66*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*PCS_TYPE */

    /* PKG_ID1 */
  /*67*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER */

    /* SEED_A1 */
  /*68*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_A2 */
  /*69*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* VENDORMX_SCRATCH */
  /*70*/  {/*baseAddr*/ 0xf0,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SCRATCH */

    /* VENDORMX_PCS_MODE */
  /*71*/  {/*baseAddr*/ 0xf8,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*ENA_CLAUSE49 */
  /*72*/  {/*baseAddr*/ 0xf8,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*DISABLE_MLD */
  /*73*/  {/*baseAddr*/ 0xf8,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*HI_BER25 */
  /*74*/  {/*baseAddr*/ 0xf8,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*HI_BER5 */
  /*75*/  {/*baseAddr*/ 0xf8,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*ST_ENA_CLAUSE49 */
  /*76*/  {/*baseAddr*/ 0xf8,  /*offsetFormula*/ 0x100, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*ST_DISABLE_MLD */
  /*77*/  {/*baseAddr*/ 0xf8,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*ST_HIBER25 */
  /*78*/  {/*baseAddr*/ 0xf8,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*ST_HIBER5 */

    /* STATUS1 */
  /*79*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LOW_POWER_ABILITY */
  /*80*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PCS_RECEIVE_LINK */
  /*81*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FAULT */
  /*82*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*RX_LPI_ACTIVE */
  /*83*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x100, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_LPI_ACTIVE */
  /*84*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*RX_LPI */
  /*85*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TX_LPI */

    /* SEED_B1 */
  /*86*/  {/*baseAddr*/ 0x9c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* ERR_BLK_HIGH_ORDER_CNT */
  /*87*/  {/*baseAddr*/ 0xb4,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 14 }, /*ERRORED_BLOCKS_COUNTER */
  /*88*/  {/*baseAddr*/ 0xb4,  /*offsetFormula*/ 0x100, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*HIGH_ORDER_PRESENT */

    /* MULTILANE_ALIGN_STAT1 */
  /*89*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK_L */
};

