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
* @file mvHwsUsxPcsUnits.c
*
* @brief Hawk MTI USX PCS register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>



const MV_HWS_REG_ADDR_FIELD_STC hawkUsxPcsUnitsDb[] = {
    /* PORT<0>_CONTROL1 */
  /*0*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 4 }, /*P0_SPEED_SELECTION */
  /*1*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x100, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_SPEED_ALWAYS1 */
  /*2*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_LOW_POWER */
  /*3*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x100, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_SPEED_SELECT_ALWAYS1 */
  /*4*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_LOOPBACK */
  /*5*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x100, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_RESET */

    /* PORT<0>_STATUS1 */
  /*6*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_LOW_POWER_ABILITY */
  /*7*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_PCS_RECEIVE_LINK */
  /*8*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_FAULT */
  /*9*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_RX_LPI_ACTIVE */
  /*10*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x100, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_TX_LPI_ACTIVE */
  /*11*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_RX_LPI */
  /*12*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_TX_LPI */

    /* PORT<0>_DEVICE_ID0 */
  /*13*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_IDENTIFIER0 */

    /* PORT<0>_DEVICE_ID1 */
  /*14*/  {/*baseAddr*/ 0x000c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_IDENTIFIER1 */

    /* PORT<0>_SPEED_ABILITY */
  /*15*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_C10GETH */
  /*16*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_C10PASS_TS */
  /*17*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_C40G */
  /*18*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_C100G */
  /*19*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_C25G */
  /*20*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x100, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_C50G */

    /* PORT<0>_DEVICES_IN_PKG1 */
  /*21*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_CLAUSE22 */
  /*22*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PMD_PMA */
  /*23*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_WIS_PRES */
  /*24*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_PCS_PRES */
  /*25*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_PHY_XS */
  /*26*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x100, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_DTE_XS */
  /*27*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x100, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_TC_PRES */

    /* PORT<0>_DEVICES_IN_PKG2 */
  /*28*/  {/*baseAddr*/ 0x0018,  /*offsetFormula*/ 0x100, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_CLAUSE22 */
  /*29*/  {/*baseAddr*/ 0x0018,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_DEVICE1 */
  /*30*/  {/*baseAddr*/ 0x0018,  /*offsetFormula*/ 0x100, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_DEVICE2 */

    /* PORT<0>_CONTROL2 */
  /*31*/  {/*baseAddr*/ 0x001c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*P0_PCS_TYPE */

    /* PORT<0>_STATUS2 */
  /*32*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_C10GBASE_R */
  /*33*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_C10GBASE_X */
  /*34*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_C10GBASE_W */
  /*35*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_C10GBASE_T */
  /*36*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_C40GBASE_R */
  /*37*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x100, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_C100GBASE_R */
  /*38*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_C25GBASE_R */
  /*39*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_C50GBASE_R */
  /*40*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_RECEIVE_FAULT */
  /*41*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_TRANSMIT_FAULT */
  /*42*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x100, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_C2_5GBASE_T */
  /*43*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x100, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_C5GBASE_T */
  /*44*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 2 }, /*P0_DEVICE_PRESENT */

    /* PORT<0>_PKG_ID0 */
  /*45*/  {/*baseAddr*/ 0x0038,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_IDENTIFIER */

    /* PORT<0>_PKG_ID1 */
  /*46*/  {/*baseAddr*/ 0x003c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_IDENTIFIER */

    /* PORT<0>_BASER_STATUS1 */
  /*47*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_BLOCK_LOCK */
  /*48*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_HIGH_BER */
  /*49*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x100, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_RECEIVE_LINK */

    /* PORT<0>_BASER_STATUS2 */
  /*50*/  {/*baseAddr*/ 0x0084,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*P0_ERRORED_CNT */
  /*51*/  {/*baseAddr*/ 0x0084,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 6 }, /*P0_BER_COUNTER */
  /*52*/  {/*baseAddr*/ 0x0084,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_HIGH_BER */
  /*53*/  {/*baseAddr*/ 0x0084,  /*offsetFormula*/ 0x100, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_BLOCK_LOCK */

    /* PORT<0>_SEED_A0 */
  /*54*/  {/*baseAddr*/ 0x0088,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_SEED */

    /* PORT<0>_SEED_A1 */
  /*55*/  {/*baseAddr*/ 0x008c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_SEED */

    /* PORT<0>_SEED_A2 */
  /*56*/  {/*baseAddr*/ 0x0090,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_SEED */

    /* PORT<0>_SEED_A3 */
  /*57*/  {/*baseAddr*/ 0x0094,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*P0_SEED */

    /* PORT<0>_SEED_B0 */
  /*58*/  {/*baseAddr*/ 0x0098,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_SEED */

    /* PORT<0>_SEED_B1 */
  /*59*/  {/*baseAddr*/ 0x009c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_SEED */

    /* PORT<0>_SEED_B2 */
  /*60*/  {/*baseAddr*/ 0x00a0,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_SEED */

    /* PORT<0>_SEED_B3 */
  /*61*/  {/*baseAddr*/ 0x00a4,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*P0_SEED */

    /* PORT<0>_BASER_TEST_CONTROL */
  /*62*/  {/*baseAddr*/ 0x00a8,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_DATA_PATTERN_SEL */
  /*63*/  {/*baseAddr*/ 0x00a8,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_SELECT_SQUARE */
  /*64*/  {/*baseAddr*/ 0x00a8,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_RX_TESTPATTERN */
  /*65*/  {/*baseAddr*/ 0x00a8,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_TX_TESTPATTERN */
  /*66*/  {/*baseAddr*/ 0x00a8,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_SELECT_RANDOM */

    /* PORT<0>_BASER_TEST_ERR_CNT */
  /*67*/  {/*baseAddr*/ 0x00ac,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_COUNTER */

    /* PORT<0>_BER_HIGH_ORDER_CNT */
  /*68*/  {/*baseAddr*/ 0x00b0,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_BER_COUNTER */

    /* PORT<0>_ERR_BLK_HIGH_ORDER_CNT */
  /*69*/  {/*baseAddr*/ 0x00b4,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 14 }, /*P0_ERRORED_BLOCKS_COUNTER */
  /*70*/  {/*baseAddr*/ 0x00b4,  /*offsetFormula*/ 0x100, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_HIGH_ORDER_PRESENT */

    /* PORT<0>_MULTILANE_ALIGN_STAT1 */
  /*71*/  {/*baseAddr*/ 0x00c8,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_LANE0_BLOCK_LOCK */

    /* PORT<0>_VENDOR_SCRATCH */
  /*72*/  {/*baseAddr*/ 0x00f0,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_SCRATCH */

    /* PORT<0>_VENDOR_CORE_REV */
  /*73*/  {/*baseAddr*/ 0x00f4,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_REVISION */

    /* PORT<0>_VENDOR_TXLANE_THRESH */
  /*74*/  {/*baseAddr*/ 0x00fc,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*P0_THRESHOLD0 */

    /* PORT<0>_VENDOR_PCS_MODE */
  /*75*/  {/*baseAddr*/ 0x00f8,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_ENA_CLAUSE49 */
  /*76*/  {/*baseAddr*/ 0x00f8,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_DISABLE_MLD */
  /*77*/  {/*baseAddr*/ 0x00f8,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_HI_BER25 */
  /*78*/  {/*baseAddr*/ 0x00f8,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_HI_BER5 */
  /*79*/  {/*baseAddr*/ 0x00f8,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_ST_ENA_CLAUSE49 */
  /*80*/  {/*baseAddr*/ 0x00f8,  /*offsetFormula*/ 0x100, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_ST_DISABLE_MLD */
  /*81*/  {/*baseAddr*/ 0x00f8,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P0_ST_HI_BER25 */
  /*82*/  {/*baseAddr*/ 0x00f8,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_ST_HI_BER5 */

    /* PORT<0>_STATUS3 */
  /*83*/  {/*baseAddr*/ 0x0024,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_C200GBASE_R */
  /*84*/  {/*baseAddr*/ 0x0024,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_C400GBASE_R */
};
