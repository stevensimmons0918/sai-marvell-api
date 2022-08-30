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
* @file mvHwsMtiPcs200Units.c
*
* @brief Hawk MTI PCS200 register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>


const MV_HWS_REG_ADDR_FIELD_STC hawkMtiPcs200UnitsDb[] = {
    /* CONTROL1 */
  /*0*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 4 }, /*SPEED_SELECTION */
  /*1*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SPEED_SELECTION_6 */
  /*2*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LOW_POWER */
  /*3*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SPEED_SELECTION_13 */
  /*4*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LOOPBACK */
  /*5*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RESET */

    /* STATUS1 */
  /*6*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PCS_RECEIVE_LINK */
  /*7*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FAULT */

    /* DEVICE_ID0 */
  /*8*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER0 */

    /* DEVICE_ID1 */
  /*9*/  {/*baseAddr*/ 0x000c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER1 */

    /* SPEED_ABILITY */
  /*10*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*SPEEDABILITY_8 */
  /*11*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SPEEDABILITY_9 */

    /* DEVICES_IN_PKG1 */
  /*12*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*DEVICES_PCSPRESENT */

    /* DEVICES_IN_PKG2 */
  /*13*/  {/*baseAddr*/ 0x0018,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*DEVICES_PKG2 */

    /* CONTROL2 */
  /*14*/  {/*baseAddr*/ 0x001c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*PCS_TYPE */

    /* STATUS2 */
  /*15*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*RECEIVE_FAULT */
  /*16*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TRANSMIT_FAULT */
  /*17*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 2 }, /*DEVICE_PRESENT */

    /* STATUS3 */
  /*18*/  {/*baseAddr*/ 0x0024,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*STATUS3_200 */
  /*19*/  {/*baseAddr*/ 0x0024,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*STATUS3_400 */

    /* PKG_ID0 */
  /*20*/  {/*baseAddr*/ 0x0038,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*PACKAGE_IDENTIFIER0 */

    /* PKG_ID1 */
  /*21*/  {/*baseAddr*/ 0x003c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*PACKAGE_IDENTIFIER1 */

    /* BASER_STATUS1 */
  /*22*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*RECEIVE_LINK */

    /* BASER_STATUS2 */
  /*23*/  {/*baseAddr*/ 0x0084,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*ERRORED_CNT */

    /* BASER_TEST_CONTROL */
  /*24*/  {/*baseAddr*/ 0x00a8,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SELECT_RANDOM */

    /* BASER_TEST_ERR_CNT */
  /*25*/  {/*baseAddr*/ 0x00ac,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*COUNTER */

    /* BER_HIGH_ORDER_CNT */
  /*26*/  {/*baseAddr*/ 0x00b0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*BER_COUNTER */

    /* ERR_BLK_HIGH_ORDER_CNT */
  /*27*/  {/*baseAddr*/ 0x00b4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 14 }, /*ERRORED_BLOCKS_COUNTER */
  /*28*/  {/*baseAddr*/ 0x00b4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*HIGH_ORDER_PRESENT */

    /* MULTILANE_ALIGN_STAT1 */
  /*29*/  {/*baseAddr*/ 0x00c8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LANE_ALIGN_STATUS */

    /* LANE0_MAPPING */
  /*30*/  {/*baseAddr*/ 0x0100,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE0_MAPPING */

    /* LANE1_MAPPING */
  /*31*/  {/*baseAddr*/ 0x0104,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE1_MAPPING */

    /* LANE2_MAPPING */
  /*32*/  {/*baseAddr*/ 0x0108,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE2_MAPPING */

    /* LANE3_MAPPING */
  /*33*/  {/*baseAddr*/ 0x010c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE3_MAPPING */

    /* LANE4_MAPPING */
  /*34*/  {/*baseAddr*/ 0x0110,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE4_MAPPING */

    /* LANE5_MAPPING */
  /*35*/  {/*baseAddr*/ 0x0114,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE5_MAPPING */

    /* LANE6_MAPPING */
  /*36*/  {/*baseAddr*/ 0x0118,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE6_MAPPING */

    /* LANE7_MAPPING */
  /*37*/  {/*baseAddr*/ 0x011c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*LANE7_MAPPING */
};
