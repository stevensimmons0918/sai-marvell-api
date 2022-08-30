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
* @file mvHwsAc5xPcs25Units.c
*
* @brief Phoenix MTI PCS25 register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>



const MV_HWS_REG_ADDR_FIELD_STC ac5xPcs25UnitsDb[] = {
    /* PORT_SPEED_ABILITY */
  /*0*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*C10GETH */
  /*1*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*C10PASS_TS */
  /*2*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*C40G */
  /*3*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*C100G */
  /*4*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*C25G */

    /* PORT_CONTROL2 */
  /*5*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*PCS_TYPE */

    /* PORT_BASER_TEST_ERR_CNT */
  /*6*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*COUNTER */

    /* PORT_VENDOR_VL2_1 */
  /*7*/  {/*baseAddr*/ 0x834,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*M2 */

    /* PORT_VENDOR_LATENCY */
  /*8*/  {/*baseAddr*/ 0x848,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 9 }, /*RX_PATH_LATENCY */
  /*9*/  {/*baseAddr*/ 0x848,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 7 }, /*TX_PATH_LATENCY */

    /* PORT_BER_HIGH_ORDER_CNT */
  /*10*/  {/*baseAddr*/ 0xb0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*BER_COUNTER */

    /* PORT_VENDOR_CORE_REV */
  /*11*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*REVISION */

    /* PORT_VENDOR_GB_SHIFT */
  /*12*/  {/*baseAddr*/ 0x844,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*GB_SHIFT */

    /* PORT_PKG_ID0 */
  /*13*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER */

    /* PORT_SEED_A0 */
  /*14*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* PORT_SEED_A2 */
  /*15*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* PORT_BASER_TEST_CONTROL */
  /*16*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*DATA_PATTERN_SEL */
  /*17*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SELECT_SQUARE */
  /*18*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*RX_TESTPATTERN */
  /*19*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TX_TESTPATTERN */
  /*20*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SELECT_RANDOM */

    /* PORT_VENDOR_SCRATCH */
  /*21*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SCRATCH */

    /* PORT_VENDOR_PCS_MODE */
  /*22*/  {/*baseAddr*/ 0x840,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*ENA_CLAUSE49 */
  /*23*/  {/*baseAddr*/ 0x840,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*DISABLE_MLD */
  /*24*/  {/*baseAddr*/ 0x840,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*HI_BER25 */
  /*25*/  {/*baseAddr*/ 0x840,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*ST_ENA_CLAUSE49 */
  /*26*/  {/*baseAddr*/ 0x840,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*ST_DISABLE_MLD */

    /* PORT_MULTILANE_ALIGN_STAT3 */
  /*27*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LANE0_ALIGNMENT_MARKER_LOCK */
  /*28*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LANE1_ALIGNMENT_MARKER_LOCK */
  /*29*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LANE2_ALIGNMENT_MARKER_LOCK */
  /*30*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*LANE3_ALIGNMENT_MARKER_LOCK */

    /* PORT_BASER_STATUS1 */
  /*31*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK */
  /*32*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*HIGH_BER */
  /*33*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*RECEIVE_LINK */

    /* PORT_BASER_STATUS2 */
  /*34*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*ERRORED_CNT */
  /*35*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 6 }, /*BER_COUNTER */
  /*36*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*HIGH_BER */
  /*37*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*BLOCK_LOCK */

    /* PORT_SEED_B2 */
  /*38*/  {/*baseAddr*/ 0xa0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* PORT_VENDOR_VL3_0 */
  /*39*/  {/*baseAddr*/ 0x838,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*M0 */
  /*40*/  {/*baseAddr*/ 0x838,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*M1 */

    /* PORT_DEVICES_IN_PKG2 */
  /*41*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*CLAUSE22 */
  /*42*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*DEVICE1 */
  /*43*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*DEVICE2 */

    /* PORT_PKG_ID1 */
  /*44*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER */

    /* PORT_SEED_A1 */
  /*45*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* PORT_SEED_B1 */
  /*46*/  {/*baseAddr*/ 0x9c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* PORT_ERR_BLK_HIGH_ORDER_CNT */
  /*47*/  {/*baseAddr*/ 0xb4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 14 }, /*ERRORED_BLOCKS_COUNTER */
  /*48*/  {/*baseAddr*/ 0xb4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*HIGH_ORDER_PRESENT */

    /* PORT_VENDOR_VL_INTVL */
  /*49*/  {/*baseAddr*/ 0x808,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ALIGNMENT_MARKERS_INTERVAL */

    /* PORT_STATUS2 */
  /*50*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*C10GBASE_R */
  /*51*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*C10GBASE_X */
  /*52*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*C10GBASE_W */
  /*53*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*C10GBASE_T */
  /*54*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*C40GBASE_R */
  /*55*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*C100GBASE_R */
  /*56*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*C25GBASE_R */
  /*57*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*RECEIVE_FAULT */
  /*58*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TRANSMIT_FAULT */
  /*59*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 2 }, /*DEVICE_PRESENT */

    /* PORT_SEED_A3 */
  /*60*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*SEED */

    /* PORT_SEED_B3 */
  /*61*/  {/*baseAddr*/ 0xa4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*SEED */

    /* PORT_VENDOR_TX_LANE_THRESH */
  /*62*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*TX_FIFO_ALMOST_FULL_THRESHOLD */

    /* PORT_VENDOR_VL0_1 */
  /*63*/  {/*baseAddr*/ 0x824,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*M2 */

    /* PORT_VENDOR_VL1_0 */
  /*64*/  {/*baseAddr*/ 0x828,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*M0 */
  /*65*/  {/*baseAddr*/ 0x828,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*M1 */

    /* PORT_VENDOR_VL2_0 */
  /*66*/  {/*baseAddr*/ 0x830,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*M0 */
  /*67*/  {/*baseAddr*/ 0x830,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*M1 */

    /* PORT_STATUS1 */
  /*68*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LOW_POWER_ABILITY */
  /*69*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PCS_RECEIVE_LINK */
  /*70*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FAULT */
  /*71*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*RX_LPI_ACTIVE */
  /*72*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_LPI_ACTIVE */
  /*73*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*RX_LPI */
  /*74*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TX_LPI */

    /* PORT_DEVICE_ID1 */
  /*75*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER1 */

    /* PORT_VENDOR_VL0_0 */
  /*76*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*M0 */
  /*77*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*M1 */

    /* PORT_VENDOR_VL3_1 */
  /*78*/  {/*baseAddr*/ 0x83c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*M2 */

    /* PORT_CONTROL1 */
  /*79*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 4 }, /*PORT_SPEED_SELECTION */
  /*80*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PORT_SPEED_ALWAYS1 */
  /*81*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*PORT_LOW_POWER */
  /*82*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*PORT_SPEED_SELECT_ALWAYS1 */
  /*83*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*PORT_LOOPBACK */
  /*84*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PORT_RESET */

    /* PORT_DEVICE_ID0 */
  /*85*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER0 */

    /* PORT_DEVICES_IN_PKG1 */
  /*86*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*CLAUSE22 */
  /*87*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PMD_PMA */
  /*88*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*WIS_PRES */
  /*89*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PCS_PRES */
  /*90*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PHY_XS */
  /*91*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*DTE_XS */
  /*92*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*TC_PRES */

    /* PORT_SEED_B0 */
  /*93*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* PORT_MULTILANE_ALIGN_STAT1 */
  /*94*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LANE_ALIGN_STATUS */

    /* PORT_VENDOR_VL1_1 */
  /*95*/  {/*baseAddr*/ 0x82c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*M2 */
};



