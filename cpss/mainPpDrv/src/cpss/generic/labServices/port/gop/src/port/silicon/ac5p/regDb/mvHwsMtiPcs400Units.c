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
* @file mvHwsMtiPcs400Units.c
*
* @brief Hawk MTI PCS400 register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>


const MV_HWS_REG_ADDR_FIELD_STC hawkMtiPcs400UnitsDb[] = {
    /* CONTROL1 */
  /*0*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*SPEED_SELECTION */
  /*1*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*SPEED_SELECTION_3 */
  /*2*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 2 }, /*SPEED_SELECTION_5_4 */
  /*3*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SPEED_SELECTION_6 */
  /*4*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LOW_POWER */
  /*5*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SPEED_SELECTION_13 */
  /*6*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LOOPBACK */
  /*7*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RESET */

    /* STATUS1 */
  /*8*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PCS_RECEIVE_LINK */
  /*9*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FAULT */

    /* DEVICE_ID0 */
  /*10*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER0 */

    /* DEVICE_ID1 */
  /*11*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER1 */

    /* SPEED_ABILITY */
  /*12*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*SPEEDABILITY_8 */
  /*13*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SPEEDABILITY_9 */

    /* DEVICES_IN_PKG1 */
  /*14*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*DEVICES_PCSPRESENT */

    /* DEVICES_IN_PKG2 */
  /*15*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*DEVICES_PKG2 */

    /* CONTROL2 */
  /*16*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*PCS_TYPE */

    /* STATUS2 */
  /*17*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*RECEIVE_FAULT */
  /*18*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TRANSMIT_FAULT */
  /*19*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 2 }, /*DEVICE_PRESENT */

    /* STATUS3 */
  /*20*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*STATUS3_200 */
  /*21*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*STATUS3_400 */

    /* PKG_ID0 */
  /*22*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*PACKAGE_IDENTIFIER0 */

    /* PKG_ID1 */
  /*23*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*PACKAGE_IDENTIFIER1 */

    /* BASER_STATUS1 */
  /*24*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*RECEIVE_LINK */

    /* BASER_STATUS2 */
  /*25*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*ERRORED_CNT */

    /* BASER_TEST_CONTROL */
  /*26*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SELECT_RANDOM */

    /* BASER_TEST_ERR_CNT */
  /*27*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*COUNTER */

    /* BER_HIGH_ORDER_CNT */
  /*28*/  {/*baseAddr*/ 0xb0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*BER_COUNTER */

    /* ERR_BLK_HIGH_ORDER_CNT */
  /*29*/  {/*baseAddr*/ 0xb4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 14 }, /*ERRORED_BLOCKS_COUNTER */
  /*30*/  {/*baseAddr*/ 0xb4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*HIGH_ORDER_PRESENT */

    /* MULTILANE_ALIGN_STAT1 */
  /*31*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LANE_ALIGN_STATUS */

    /* MULTILANE_ALIGN_STAT3 */
  /*32*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LANE0_MARKER_LOCK */
  /*33*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LANE1_MARKER_LOCK */
  /*34*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LANE2_MARKER_LOCK */
  /*35*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*LANE3_MARKER_LOCK */
  /*36*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*LANE4_MARKER_LOCK */
  /*37*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*LANE5_MARKER_LOCK */
  /*38*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*LANE6_MARKER_LOCK */
  /*39*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*LANE7_MARKER_LOCK */

    /* MULTILANE_ALIGN_STAT4 */
  /*40*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LANE8_MARKER_LOCK */
  /*41*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LANE9_MARKER_LOCK */
  /*42*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LANE10_MARKER_LOCK */
  /*43*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*LANE11_MARKER_LOCK */
  /*44*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*LANE12_MARKER_LOCK */
  /*45*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*LANE13_MARKER_LOCK */
  /*46*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*LANE14_MARKER_LOCK */
  /*47*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*LANE15_MARKER_LOCK */

    /* LANE0_MAPPING */
  /*48*/  {/*baseAddr*/ 0x640,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE0_MAPPING */

    /* LANE1_MAPPING */
  /*49*/  {/*baseAddr*/ 0x644,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE1_MAPPING */

    /* LANE2_MAPPING */
  /*50*/  {/*baseAddr*/ 0x648,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE2_MAPPING */

    /* LANE3_MAPPING */
  /*51*/  {/*baseAddr*/ 0x64c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE3_MAPPING */

    /* LANE4_MAPPING */
  /*52*/  {/*baseAddr*/ 0x650,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE4_MAPPING */

    /* LANE5_MAPPING */
  /*53*/  {/*baseAddr*/ 0x654,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE5_MAPPING */

    /* LANE6_MAPPING */
  /*54*/  {/*baseAddr*/ 0x658,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE6_MAPPING */

    /* LANE7_MAPPING */
  /*55*/  {/*baseAddr*/ 0x65c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE7_MAPPING */

    /* LANE8_MAPPING */
  /*56*/  {/*baseAddr*/ 0x660,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE8_MAPPING */

    /* LANE9_MAPPING */
  /*57*/  {/*baseAddr*/ 0x664,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE9_MAPPING */

    /* LANE10_MAPPING */
  /*58*/  {/*baseAddr*/ 0x668,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE10_MAPPING */

    /* LANE11_MAPPING */
  /*59*/  {/*baseAddr*/ 0x66c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE11_MAPPING */

    /* LANE12_MAPPING */
  /*60*/  {/*baseAddr*/ 0x670,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE12_MAPPING */

    /* LANE13_MAPPING */
  /*61*/  {/*baseAddr*/ 0x674,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE13_MAPPING */

    /* LANE14_MAPPING */
  /*62*/  {/*baseAddr*/ 0x678,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE14_MAPPING */

    /* LANE15_MAPPING */
  /*63*/  {/*baseAddr*/ 0x67c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE15_MAPPING */

    /* VENDOR_SCRATCH */
  /*64*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SCRATCH */

    /* VENDOR_CORE_REV */
  /*65*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*REVISION */

    /* VENDOR_VL_INTVL */
  /*66*/  {/*baseAddr*/ 0x808,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 15 }, /*VL_INTVL */

    /* VENDOR_TX_LANE_THRESH */
  /*67*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*TX_LANETHRESH */

    /* VENDOR_TX_CDMII_PACE */
  /*68*/  {/*baseAddr*/ 0x81c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*TX_CDMII_PACE */

    /* VENDOR_AM_0 */
  /*69*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*CM0 */
  /*70*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*CM1 */

    /* VENDOR_AM_1 */
  /*71*/  {/*baseAddr*/ 0x824,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*CM2 */

    /* VENDOR_DBGINFO0 */
  /*72*/  {/*baseAddr*/ 0xa00,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*DBGINFO0_0 */
  /*73*/  {/*baseAddr*/ 0xa00,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*DBGINFO0_1 */
  /*74*/  {/*baseAddr*/ 0xa00,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*DBGINFO0_2 */
  /*75*/  {/*baseAddr*/ 0xa00,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*DBGINFO0_3 */
  /*76*/  {/*baseAddr*/ 0xa00,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*DBGINFO0_4 */
  /*77*/  {/*baseAddr*/ 0xa00,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*DBGINFO0_5 */
  /*78*/  {/*baseAddr*/ 0xa00,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*DBGINFO0_6 */
  /*79*/  {/*baseAddr*/ 0xa00,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*DBGINFO0_8 */
  /*80*/  {/*baseAddr*/ 0xa00,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*DBGINFO0_9 */
  /*81*/  {/*baseAddr*/ 0xa00,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*DBGINFO0_10 */
  /*82*/  {/*baseAddr*/ 0xa00,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*DBGINFO0_11 */
  /*83*/  {/*baseAddr*/ 0xa00,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*DBGINFO0_12 */
  /*84*/  {/*baseAddr*/ 0xa00,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*DBGINFO0_13 */
  /*85*/  {/*baseAddr*/ 0xa00,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*DBGINFO0_14 */

    /* VENDOR_DBGINFO1 */
  /*86*/  {/*baseAddr*/ 0xa04,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*DBGINFO1_0 */
  /*87*/  {/*baseAddr*/ 0xa04,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*DBGINFO1_4 */
  /*88*/  {/*baseAddr*/ 0xa04,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*DBGINFO1_5 */
  /*89*/  {/*baseAddr*/ 0xa04,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*DBGINFO1_6 */

    /* VENDOR_DBGINFO2 */
  /*90*/  {/*baseAddr*/ 0xa08,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*DBGINFO2 */

    /* VENDOR_DBGINFO3 */
  /*91*/  {/*baseAddr*/ 0xa0c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*DBGINFO3_0 */
  /*92*/  {/*baseAddr*/ 0xa0c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*DBGINFO3_1 */
  /*93*/  {/*baseAddr*/ 0xa0c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*DBGINFO3_2 */
  /*94*/  {/*baseAddr*/ 0xa0c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 2 }, /*DBGINFO3_4 */
  /*95*/  {/*baseAddr*/ 0xa0c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 2 }, /*DBGINFO3_8 */
};

