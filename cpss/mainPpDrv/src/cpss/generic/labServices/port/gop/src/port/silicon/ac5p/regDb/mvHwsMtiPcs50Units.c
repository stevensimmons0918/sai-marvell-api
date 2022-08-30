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
* @file mvHwsMtiPcs100Units.c
*
* @brief Hawk MTI PCS100 register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>


const MV_HWS_REG_ADDR_FIELD_STC hawkMtiPcs50UnitsDb[] = {
    /* CONTROL1 */
  /*0*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 4 }, /*SPEED_SELECTION */
  /*1*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SPEED_ALWAYS1 */
  /*2*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LOW_POWER */
  /*3*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SPEED_SELECT_ALWAYS1 */
  /*4*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LOOPBACK */
  /*5*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RESET */

    /* STATUS1 */
  /*6*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LOW_POWER_ABILITY */
  /*7*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PCS_RECEIVE_LINK */
  /*8*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FAULT */
  /*9*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*RX_LPI_ACTIVE */
  /*10*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_LPI_ACTIVE */
  /*11*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*RX_LPI */
  /*12*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TX_LPI */

    /* DEVICE_ID0 */
  /*13*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER0 */

    /* DEVICE_ID1 */
  /*14*/  {/*baseAddr*/ 0x000c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER1 */

    /* SPEED_ABILITY */
  /*15*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*C10GETH */
  /*16*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*C10PASS_TS */
  /*17*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*C40G */
  /*18*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*C100G */
  /*19*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*C25G */
  /*20*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*C50G */

    /* DEVICES_IN_PKG1 */
  /*21*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*CLAUSE22 */
  /*22*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PMD_PMA */
  /*23*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*WIS_PRES */
  /*24*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PCS_PRES */
  /*25*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PHY_XS */
  /*26*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*DTE_XS */
  /*27*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*TC_PRES */

    /* DEVICES_IN_PKG2 */
  /*28*/  {/*baseAddr*/ 0x0018,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*CLAUSE22 */
  /*29*/  {/*baseAddr*/ 0x0018,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*DEVICE1 */
  /*30*/  {/*baseAddr*/ 0x0018,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*DEVICE2 */

    /* CONTROL2 */
  /*31*/  {/*baseAddr*/ 0x001c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*PCS_TYPE */

    /* STATUS2 */
  /*32*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*C10GBASE_R */
  /*33*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*C10GBASE_X */
  /*34*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*C10GBASE_W */
  /*35*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*C10GBASE_T */
  /*36*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*C40GBASE_R */
  /*37*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*C100GBASE_R */
  /*38*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*C25GBASE_R */
  /*39*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*C50GBASE_R */
  /*40*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*RECEIVE_FAULT */
  /*41*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TRANSMIT_FAULT */
  /*42*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 2 }, /*DEVICE_PRESENT */

    /* PKG_ID0 */
  /*43*/  {/*baseAddr*/ 0x0038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER */

    /* PKG_ID1 */
  /*44*/  {/*baseAddr*/ 0x003c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER */

    /* BASER_STATUS1 */
  /*45*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK */
  /*46*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*HIGH_BER */
  /*47*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*RECEIVE_LINK */

    /* BASER_STATUS2 */
  /*48*/  {/*baseAddr*/ 0x0084,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*ERRORED_CNT */
  /*49*/  {/*baseAddr*/ 0x0084,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 6 }, /*BER_COUNTER */
  /*50*/  {/*baseAddr*/ 0x0084,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*HIGH_BER */
  /*51*/  {/*baseAddr*/ 0x0084,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*BLOCK_LOCK */

    /* SEED_A0 */
  /*52*/  {/*baseAddr*/ 0x0088,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_A1 */
  /*53*/  {/*baseAddr*/ 0x008c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_A2 */
  /*54*/  {/*baseAddr*/ 0x0090,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_A3 */
  /*55*/  {/*baseAddr*/ 0x0094,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*SEED */

    /* SEED_B0 */
  /*56*/  {/*baseAddr*/ 0x0098,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_B1 */
  /*57*/  {/*baseAddr*/ 0x009c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_B2 */
  /*58*/  {/*baseAddr*/ 0x00a0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_B3 */
  /*59*/  {/*baseAddr*/ 0x00a4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*SEED */

    /* BASER_TEST_CONTROL */
  /*60*/  {/*baseAddr*/ 0x00a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*DATA_PATTERN_SEL */
  /*61*/  {/*baseAddr*/ 0x00a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SELECT_SQUARE */
  /*62*/  {/*baseAddr*/ 0x00a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*RX_TESTPATTERN */
  /*63*/  {/*baseAddr*/ 0x00a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TX_TESTPATTERN */
  /*64*/  {/*baseAddr*/ 0x00a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SELECT_RANDOM */

    /* BASER_TEST_ERR_CNT */
  /*65*/  {/*baseAddr*/ 0x00ac,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*COUNTER */

    /* BER_HIGH_ORDER_CNT */
  /*66*/  {/*baseAddr*/ 0x00b0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*BER_COUNTER */

    /* ERR_BLK_HIGH_ORDER_CNT */
  /*67*/  {/*baseAddr*/ 0x00b4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 14 }, /*ERRORED_BLOCKS_COUNTER */
  /*68*/  {/*baseAddr*/ 0x00b4,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*HIGH_ORDER_PRESENT */

    /* MULTILANE_ALIGN_STAT1 */
  /*69*/  {/*baseAddr*/ 0x00c8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LANE0_BLOCK_LOCK */
  /*70*/  {/*baseAddr*/ 0x00c8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LANE1_BLOCK_LOCK */
  /*71*/  {/*baseAddr*/ 0x00c8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LANE2_BLOCK_LOCK */
  /*72*/  {/*baseAddr*/ 0x00c8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*LANE3_BLOCK_LOCK */
  /*73*/  {/*baseAddr*/ 0x00c8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LANE_ALIGN_STATUS */

    /* MULTILANE_ALIGN_STAT3 */
  /*74*/  {/*baseAddr*/ 0x00d0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LANE0_MARKER_LOCK */
  /*75*/  {/*baseAddr*/ 0x00d0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LANE1_MARKER_LOCK */
  /*76*/  {/*baseAddr*/ 0x00d0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LANE2_MARKER_LOCK */
  /*77*/  {/*baseAddr*/ 0x00d0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*LANE3_MARKER_LOCK */

    /* BIP_ERR_CNT_LANE0 */
  /*78*/  {/*baseAddr*/ 0x0320,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*BIP_ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE1 */
  /*79*/  {/*baseAddr*/ 0x0324,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*BIP_ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE2 */
  /*80*/  {/*baseAddr*/ 0x0328,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*BIP_ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE3 */
  /*81*/  {/*baseAddr*/ 0x032c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*BIP_ERROR_COUNTER */

    /* LANE0_MAPPING */
  /*82*/  {/*baseAddr*/ 0x0640,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*LANE_MAPPING */

    /* LANE1_MAPPING */
  /*83*/  {/*baseAddr*/ 0x0644,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*LANE_MAPPING */

    /* LANE2_MAPPING */
  /*84*/  {/*baseAddr*/ 0x0648,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*LANE_MAPPING */

    /* LANE3_MAPPING */
  /*85*/  {/*baseAddr*/ 0x064c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*LANE_MAPPING */

    /* VENDOR_SCRATCH */
  /*86*/  {/*baseAddr*/ 0x0800,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SCRATCH */

    /* VENDOR_CORE_REV */
  /*87*/  {/*baseAddr*/ 0x0804,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*REVISION */

    /* VENDOR_VL_INTVL */
  /*88*/  {/*baseAddr*/ 0x0808,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MARKER_COUNTER */

    /* VENDOR_TXLANE_THRESH */
  /*89*/  {/*baseAddr*/ 0x080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*THRESHOLD0 */
  /*90*/  {/*baseAddr*/ 0x080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*THRESHOLD1 */
  /*91*/  {/*baseAddr*/ 0x080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 4 }, /*THRESHOLD2 */
  /*92*/  {/*baseAddr*/ 0x080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 4 }, /*THRESHOLD3 */

    /* VL0_0 */
  /*93*/  {/*baseAddr*/ 0x0900,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL0_0 */

    /* VL0_1 */
  /*94*/  {/*baseAddr*/ 0x0904,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL0_1 */

    /* VL1_0 */
  /*95*/  {/*baseAddr*/ 0x0908,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL1_0 */

    /* VL1_1 */
  /*96*/  {/*baseAddr*/ 0x090c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL1_1 */

    /* VL2_0 */
  /*97*/  {/*baseAddr*/ 0x0910,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL2_0 */

    /* VL2_1 */
  /*98*/  {/*baseAddr*/ 0x0914,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL2_1 */

    /* VL3_0 */
  /*99*/  {/*baseAddr*/ 0x0918,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL3_0 */

    /* VL3_1 */
  /*100*/  {/*baseAddr*/ 0x091c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL3_1 */

    /* VENDOR_PCS_MODE */
  /*101*/  {/*baseAddr*/ 0x0840,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*ENA_CLAUSE49 */
  /*102*/  {/*baseAddr*/ 0x0840,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*DISABLE_MLD */
  /*103*/  {/*baseAddr*/ 0x0840,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*HI_BER25 */
  /*104*/  {/*baseAddr*/ 0x0840,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*HI_BER5 */
  /*105*/  {/*baseAddr*/ 0x0840,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*ST_ENA_CLAUSE49 */
  /*106*/  {/*baseAddr*/ 0x0840,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*ST_DISABLE_MLD */
  /*107*/  {/*baseAddr*/ 0x0840,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*ST_HI_BER25 */
  /*108*/  {/*baseAddr*/ 0x0840,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*ST_HI_BER5 */

    /* VENDOR_MIRROR_VL0_0 */
  /*109*/  {/*baseAddr*/ 0x0820,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MIRROR_M0 */
  /*110*/  {/*baseAddr*/ 0x0820,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*MIRROR_M1 */

    /* VENDOR_MIRROR_VL0_1 */
  /*111*/  {/*baseAddr*/ 0x0824,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MIRROR_M2 */

    /* VENDOR_MIRROR_VL1_0 */
  /*112*/  {/*baseAddr*/ 0x0828,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MIRROR_M0 */
  /*113*/  {/*baseAddr*/ 0x0828,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*MIRROR_M1 */

    /* VENDOR_MIRROR_VL1_1 */
  /*114*/  {/*baseAddr*/ 0x082c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MIRROR_M2 */

    /* VENDOR_MIRROR_VL2_0 */
  /*115*/  {/*baseAddr*/ 0x0830,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MIRROR_M0 */
  /*116*/  {/*baseAddr*/ 0x0830,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*MIRROR_M1 */

    /* VENDOR_MIRROR_VL2_1 */
  /*117*/  {/*baseAddr*/ 0x0834,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MIRROR_M2 */

    /* VENDOR_MIRROR_VL3_0 */
  /*118*/  {/*baseAddr*/ 0x0838,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MIRROR_M0 */
  /*119*/  {/*baseAddr*/ 0x0838,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*MIRROR_M1 */

    /* VENDOR_MIRROR_VL3_1 */
  /*120*/  {/*baseAddr*/ 0x083c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MIRROR_M2 */
};
