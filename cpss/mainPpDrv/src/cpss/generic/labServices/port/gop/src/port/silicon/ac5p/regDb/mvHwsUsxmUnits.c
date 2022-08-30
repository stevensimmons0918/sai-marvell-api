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
* @file mvHwsUsxmUnits.c
*
* @brief Hawk MTI USX Multiplexer register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>



const MV_HWS_REG_ADDR_FIELD_STC hawkUsxmUnitsDb[] = {
    /* CONTROL */
  /*0*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x3000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RESET */

    /* STATUS */
  /*1*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*RECEIVE_LINK_STATUS */
  /*2*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x3000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*RECEIVE_LINK_STATUS_LL */

    /* PORTS_ENA */
  /*3*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*ACTIVE_PORTS_USED */

    /* VL_INTVL */
  /*4*/  {/*baseAddr*/ 0x000c,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*PORT_CYCLE_INTERVAL */

    /* ALIGN_MATCH_ERR */
  /*5*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MISMATCH_ERRCNT */

    /* VL0_BYTE3 */
  /*6*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL0_BYTE3 */

    /* VL0_0 */
  /*7*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL0_0 */

    /* VL0_1 */
  /*8*/  {/*baseAddr*/ 0x0044,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL0_1 */

    /* VL1_0 */
  /*9*/  {/*baseAddr*/ 0x0048,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL1_0 */

    /* VL1_1 */
  /*10*/  {/*baseAddr*/ 0x004c,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL1_1 */

    /* VL2_0 */
  /*11*/  {/*baseAddr*/ 0x0050,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL2_0 */

    /* VL2_1 */
  /*12*/  {/*baseAddr*/ 0x0054,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL2_1 */

    /* VL3_0 */
  /*13*/  {/*baseAddr*/ 0x0058,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL3_0 */

    /* VL3_1 */
  /*14*/  {/*baseAddr*/ 0x005c,  /*offsetFormula*/ 0x3000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL3_1 */
};

