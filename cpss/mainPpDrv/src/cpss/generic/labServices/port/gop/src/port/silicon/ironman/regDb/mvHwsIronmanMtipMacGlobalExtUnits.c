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
* @file mvHwsIronmanMtipMacGlobalExtUnits.c
*
* @brief IronmanL MTI MAC Global EXT register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC ironmanMtipMacGlobalExtUnitsDb[] = {
    /* Global Last Violation */
  /*0*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*LAST_VIOLATION */

    /* Global MAC Metal Fix */
  /*1*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MAC_METAL_FIX */

    /* Global Control */
  /*2*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*MAC_ABU_WATCHDOG */
  /*3*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 4 }, /*CYC_TO_STRETCH_MAC2APP */
  /*4*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 4 }, /*CYC_TO_STRETCH_MAC2REG */
  /*5*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 4 }, /*CYC_TO_STRETCH_APP2REG */
  /*6*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*XOFF_GEN_ALWAYS_SYNC */

    /* Global MAC Clock and Reset Conrol */
  /*7*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*CMN_MAC_RESET_ */
  /*8*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*CMN_MAC_CLK_EN */
  /*9*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*CMN_APP_RESET_ */
  /*10*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*CMN_APP_CLK_EN */

    /* Global Clock Divider Control */
  /*11*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 21 }, /*CLK_RATIO */

    /* Global USX PCH Control */
  /*12*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 2 }, /*PACKET_TYPE_PCH */
  /*13*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 2 }, /*PACKET_TYPE_NO_PCH */
  /*14*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 2 }, /*PACKET_TYPE_IDLE */
  /*15*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 2 }, /*PACKET_TYPE_RESERVED */
  /*16*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 2 }, /*PCH_EXTTYPE_IGNORE */
  /*17*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 2 }, /*PCH_EXTTYPE_PTP */
  /*18*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 2 }, /*PCH_EXTTYPE_PTP_BR */
  /*19*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 2 }, /*PCH_EXTTYPE_RESERVED */
  /*20*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 2 }, /*PCH_PREEMPTION_FRAME_TYPE_HI_PRIO_FRAME */
  /*21*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 2 }, /*PCH_PREEMPTION_FRAME_TYPE_VERIFY_RESPOND_FRAME */
  /*22*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 2 }, /*PCH_TYPE_OF_VERIFY_RESPOND_VER */
  /*23*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 2 }, /*PCH_TYPE_OF_VERIFY_RESPOND_RES */

    /* Global Interrupt Summary Cause */
  /*24*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_SUMMARY_INT_SUM */
  /*25*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*GLOBAL_INT_SUM */
  /*26*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_INT_SUM */
  /*27*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P1_INT_SUM */
  /*28*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P2_INT_SUM */
  /*29*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P3_INT_SUM */
  /*30*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P4_INT_SUM */
  /*31*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P5_INT_SUM */
  /*32*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P6_INT_SUM */
  /*33*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P7_INT_SUM */

    /* Global Interrupt Summary Mask */
  /*34*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 9 }, /*GLOBAL_INTERRUPT_SUMMARY_MASK */

    /* Global Interrupt Cause */
  /*35*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_CAUSE_INT_SUM */
  /*36*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*ILLEGAL_ADDRESS_ACCESS */
  /*37*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*ABU_BAD_ADDRESS_ERROR */
  /*38*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*ABU_WATCHDOG_ERROR */

    /* Global Interrupt Mask */
  /*39*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 3 }, /*GLOBAL_INTERRUPT_MASK */
};

