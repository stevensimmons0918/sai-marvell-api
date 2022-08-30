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
* @file mvHwsMifRegDb.c
*
* @brief Hawk MIF EXT register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

/* These enums and parameters taken from cider MIF unit:
   MIF Global registers
   MIF Rx registers
   MIF Tx registers
*/

const MV_HWS_REG_ADDR_FIELD_STC hawkMifRegistersDb[] = {
    /* Mif Type8 Channel Mapping0 */
  /*0*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*T8_TX_CHID_NUM */
  /*1*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 6 }, /*T8_RX_CHID_NUM */
  /*2*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T8_CH_TAG */
  /*3*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*T8_PFC_SIZE */
  /*4*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*T8_CLK_EN */
  /*5*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*T8_CH_LOOPBACK_EN */
  /*6*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*T8_CH_SW_RESET */
  /*7*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*T8_RX_GRACEFUL_MODE */

    /* Mif Type32 Channel Mapping0 */
  /*8*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*T32_TX_CHID_NUM */
  /*9*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 6 }, /*T32_RX_CHID_NUM */
  /*10*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T32_CH_TAG */
  /*11*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*T32_PFC_SIZE */
  /*12*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*T32_CLK_EN */
  /*13*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*T32_CH_LOOPBACK_EN */
  /*14*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*T32_CH_SW_RESET */
  /*15*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*T32_RX_GRACEFUL_MODE */

    /* Mif Type 128 Channel Mapping0 */
  /*16*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*T128_TX_CHID_NUM */
  /*17*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 6 }, /*T128_RX_CHID_NUM */
  /*18*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T128_CH_TAG */
  /*19*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x4, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*T128_PFC_SIZE */
  /*20*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x4, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*T128_CLK_EN */
  /*21*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x4, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*T128_CH_LOOPBACK_EN */
  /*22*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x4, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*T128_CH_SW_RESET */
  /*23*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x4, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*T128_RX_GRACEFUL_MODE */

    /* Mif Global Interrupt Summary Cause */
  /*24*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MIF_GLOBAL_INTERRUPT_SUM */
  /*25*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MIF_T8_TX_PROTOCOL_VIOLATION_INT */
  /*26*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*MIF_T8_TX_OVERWRITE_INT */
  /*27*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*MIF_T32_TX_PROTOCOL_VIOLATION_INT */
  /*28*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*MIF_T32_TX_OVERWRITE_INT */
  /*29*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*MIF_T128_TX_PROTOCOL_VIOLATION_INT */
  /*30*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*MIF_T128_TX_OVERWRITE_INT */
  /*31*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*MIF_T8_RX_PROTOCOL_VIOLATION_INT */
  /*32*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*MIF_T8_RX_OVERWRITE_INT */
  /*33*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*MIF_T32_RX_PROTOCOL_VIOLATION_INT */
  /*34*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*MIF_T32_RX_OVERWRITE_INT */
  /*35*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*MIF_T128_RX_PROTOCOL_VIOLATION_INT */
  /*36*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*MIF_T128_RX_OVERWRITE_INT */
  /*37*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*MIF_BAD_ACC_SUM */

    /* Mif Bad Address */
  /*38*/  {/*baseAddr*/ 0x110,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MIF_BAD_ADDR_ACC */

    /* Mif Bad Address Interrupt Cause */
  /*39*/  {/*baseAddr*/ 0x114,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MIF_BAD_ADD_INTERRUPT_SUM */
  /*40*/  {/*baseAddr*/ 0x114,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MIF_BAD_ADD_INT */

    /* Mif Bad Address Interrupt Mask */
  /*41*/  {/*baseAddr*/ 0x118,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MIF_BAD_ADDR_INTERRUPT_MASK */

    /* Mif Global Interrupt Summary Mask */
  /*42*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MIF_T8_TX_PROTOCOL_VIOLATION_INT_MSK */
  /*43*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*MIF_T8_TX_OVERWRITE_INT_MSK */
  /*44*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*MIF_T32_TX_PROTOCOL_VIOLATION_INT_MSK */
  /*45*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*MIF_T32_TX_OVERWRITE_INT_MSK */
  /*46*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*MIF_T128_TX_PROTOCOL_VIOLATION_INT_MSK */
  /*47*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*MIF_T128_TX_OVERWRITE_INT_MSK */
  /*48*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*MIF_T8_RX_PROTOCOL_VIOLATION_INT_MSK */
  /*49*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*MIF_T8_RX_OVERWRITE_INT_MSK */
  /*50*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*MIF_T32_RX_PROTOCOL_VIOLATION_INT_MSK */
  /*51*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*MIF_T32_RX_OVERWRITE_INT_MSK */
  /*52*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*MIF_T128_RX_PROTOCOL_VIOLATION_INT_MSK */
  /*53*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*MIF_T128_RX_OVERWRITE_INT_MSK */
  /*54*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*MIF_BAD_ACC_SUM_MSK */

    /* Mif Metal Fix */
  /*55*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MFIX */
/*****************************************************************************************************/
/*****************************************************************************************************/
/*****************************************************************************************************/
/*****************************************************************************************************/
/*****************************************************************************************************/
    /* Mif Type 8 Rx Control */
  /*0*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN0 */
  /*1*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN1 */
  /*2*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN2 */
  /*3*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN3 */
  /*4*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN4 */
  /*5*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN5 */
  /*6*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN6 */
  /*7*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN7 */
  /*8*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN8 */
  /*9*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN9 */
  /*10*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN10 */
  /*11*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN11 */
  /*12*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN12 */
  /*13*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN13 */
  /*14*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN14 */
  /*15*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN15 */
  /*16*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN16 */
  /*17*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN17 */
  /*18*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN18 */
  /*19*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN19 */
  /*20*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN20 */
  /*21*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN21 */
  /*22*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN22 */
  /*23*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN23 */
  /*24*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*MIF_T8_RX_EN24 */

    /* Mif Type 32 Rx Control */
  /*25*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MIF_T32_RX_EN0 */
  /*26*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MIF_T32_RX_EN1 */
  /*27*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*MIF_T32_RX_EN2 */
  /*28*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*MIF_T32_RX_EN3 */
  /*29*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*MIF_T32_RX_EN4 */
  /*30*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*MIF_T32_RX_EN5 */
  /*31*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*MIF_T32_RX_EN6 */
  /*32*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*MIF_T32_RX_EN7 */
  /*33*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*MIF_T32_RX_EN8 */
  /*34*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*MIF_T32_RX_EN9 */
  /*35*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*MIF_T32_RX_EN10 */
  /*36*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*MIF_T32_RX_EN11 */
  /*37*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*MIF_T32_RX_EN12 */
  /*38*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*MIF_T32_RX_EN13 */
  /*39*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*MIF_T32_RX_EN14 */
  /*40*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*MIF_T32_RX_EN15 */

    /* Mif Type 128 Rx Control */
  /*41*/  {/*baseAddr*/ 0x808,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MIF_T128_RX_EN0 */
  /*42*/  {/*baseAddr*/ 0x808,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MIF_T128_RX_EN1 */

    /* Mif Type 8 Rx Protocol Violation Interrupt Cause */
  /*43*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_SUM */
  /*44*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT0 */
  /*45*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT1 */
  /*46*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT2 */
  /*47*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT3 */
  /*48*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT4 */
  /*49*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT5 */
  /*50*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT6 */
  /*51*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT7 */
  /*52*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT8 */
  /*53*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT9 */
  /*54*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT10 */
  /*55*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT11 */
  /*56*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT12 */
  /*57*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT13 */
  /*58*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT14 */
  /*59*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT15 */
  /*60*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT16 */
  /*61*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT17 */
  /*62*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT18 */
  /*63*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT19 */
  /*64*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT20 */
  /*65*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT21 */
  /*66*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT22 */
  /*67*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT23 */
  /*68*/  {/*baseAddr*/ 0x900,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT24 */

    /* Mif Type 8 Rx Protocol Violation Interrupt Mask */
  /*69*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK0 */
  /*70*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK1 */
  /*71*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK2 */
  /*72*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK3 */
  /*73*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK4 */
  /*74*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK5 */
  /*75*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK6 */
  /*76*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK7 */
  /*77*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK8 */
  /*78*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK9 */
  /*79*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK10 */
  /*80*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK11 */
  /*81*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK12 */
  /*82*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK13 */
  /*83*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK14 */
  /*84*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK15 */
  /*85*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK16 */
  /*86*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK17 */
  /*87*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK18 */
  /*88*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK19 */
  /*89*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK20 */
  /*90*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK21 */
  /*91*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK22 */
  /*92*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK23 */
  /*93*/  {/*baseAddr*/ 0x904,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*T8_RX_PROTOCOL_VIOLATION_INT_MSK24 */

    /* Mif Type 8 Rx Overwrite Interrupt Cause */
  /*94*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_SUM */
  /*95*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT0 */
  /*96*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT1 */
  /*97*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT2 */
  /*98*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT3 */
  /*99*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT4 */
  /*100*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT5 */
  /*101*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT6 */
  /*102*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT7 */
  /*103*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT8 */
  /*104*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT9 */
  /*105*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT10 */
  /*106*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT11 */
  /*107*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT12 */
  /*108*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT13 */
  /*109*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT14 */
  /*110*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT15 */
  /*111*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT16 */
  /*112*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT17 */
  /*113*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT18 */
  /*114*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT19 */
  /*115*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT20 */
  /*116*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT21 */
  /*117*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT22 */
  /*118*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT23 */
  /*119*/  {/*baseAddr*/ 0x908,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT24 */

    /* Mif Type 8 Rx Overwrite Interrupt Mask */
  /*120*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK0 */
  /*121*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK1 */
  /*122*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK2 */
  /*123*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK3 */
  /*124*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK4 */
  /*125*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK5 */
  /*126*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK6 */
  /*127*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK7 */
  /*128*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK8 */
  /*129*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK9 */
  /*130*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK10 */
  /*131*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK11 */
  /*132*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK12 */
  /*133*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK13 */
  /*134*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK14 */
  /*135*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK15 */
  /*136*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK16 */
  /*137*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK17 */
  /*138*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK18 */
  /*139*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK19 */
  /*140*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK20 */
  /*141*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK21 */
  /*142*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK22 */
  /*143*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK23 */
  /*144*/  {/*baseAddr*/ 0x90c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*T8_RX_OVERWRITE_VIOLATION_INT_MSK24 */

    /* Mif Type 32 Rx Protocol Violation Interrupt Cause */
  /*145*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_SUM */
  /*146*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT0 */
  /*147*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT1 */
  /*148*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT2 */
  /*149*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT3 */
  /*150*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT4 */
  /*151*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT5 */
  /*152*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT6 */
  /*153*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT7 */
  /*154*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT8 */
  /*155*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT9 */
  /*156*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT10 */
  /*157*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT11 */
  /*158*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT12 */
  /*159*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT13 */
  /*160*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT14 */
  /*161*/  {/*baseAddr*/ 0x910,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT15 */

    /* Mif Type 32 Rx Protocol Violation Interrupt Mask */
  /*162*/  {/*baseAddr*/ 0x914,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_MSK0 */
  /*163*/  {/*baseAddr*/ 0x914,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_MSK1 */
  /*164*/  {/*baseAddr*/ 0x914,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_MSK2 */
  /*165*/  {/*baseAddr*/ 0x914,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_MSK3 */
  /*166*/  {/*baseAddr*/ 0x914,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_MSK4 */
  /*167*/  {/*baseAddr*/ 0x914,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_MSK5 */
  /*168*/  {/*baseAddr*/ 0x914,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_MSK6 */
  /*169*/  {/*baseAddr*/ 0x914,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_MSK7 */
  /*170*/  {/*baseAddr*/ 0x914,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_MSK8 */
  /*171*/  {/*baseAddr*/ 0x914,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_MSK9 */
  /*172*/  {/*baseAddr*/ 0x914,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_MSK10 */
  /*173*/  {/*baseAddr*/ 0x914,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_MSK11 */
  /*174*/  {/*baseAddr*/ 0x914,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_MSK12 */
  /*175*/  {/*baseAddr*/ 0x914,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_MSK13 */
  /*176*/  {/*baseAddr*/ 0x914,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_MSK14 */
  /*177*/  {/*baseAddr*/ 0x914,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T32_RX_PROTOCOL_VIOLATION_INT_MSK15 */

    /* Mif Type 32 Rx Overwrite Interrupt Cause */
  /*178*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_SUM */
  /*179*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT0 */
  /*180*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT1 */
  /*181*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT2 */
  /*182*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT3 */
  /*183*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT4 */
  /*184*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT5 */
  /*185*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT6 */
  /*186*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT7 */
  /*187*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT8 */
  /*188*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT9 */
  /*189*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT10 */
  /*190*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT11 */
  /*191*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT12 */
  /*192*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT13 */
  /*193*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT14 */
  /*194*/  {/*baseAddr*/ 0x918,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT15 */

    /* Mif Type 32 Rx Overwrite Interrupt Mask */
  /*195*/  {/*baseAddr*/ 0x91c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_MSK0 */
  /*196*/  {/*baseAddr*/ 0x91c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_MSK1 */
  /*197*/  {/*baseAddr*/ 0x91c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_MSK2 */
  /*198*/  {/*baseAddr*/ 0x91c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_MSK3 */
  /*199*/  {/*baseAddr*/ 0x91c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_MSK4 */
  /*200*/  {/*baseAddr*/ 0x91c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_MSK5 */
  /*201*/  {/*baseAddr*/ 0x91c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_MSK6 */
  /*202*/  {/*baseAddr*/ 0x91c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_MSK7 */
  /*203*/  {/*baseAddr*/ 0x91c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_MSK8 */
  /*204*/  {/*baseAddr*/ 0x91c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_MSK9 */
  /*205*/  {/*baseAddr*/ 0x91c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_MSK10 */
  /*206*/  {/*baseAddr*/ 0x91c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_MSK11 */
  /*207*/  {/*baseAddr*/ 0x91c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_MSK12 */
  /*208*/  {/*baseAddr*/ 0x91c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_MSK13 */
  /*209*/  {/*baseAddr*/ 0x91c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_MSK14 */
  /*210*/  {/*baseAddr*/ 0x91c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T32_RX_OVERWRITE_VIOLATION_INT_MSK15 */

    /* Mif Type 128 Rx Protocol Violation Interrupt Cause */
  /*211*/  {/*baseAddr*/ 0x920,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*T128_RX_PROTOCOL_VIOLATION_INT_SUM */
  /*212*/  {/*baseAddr*/ 0x920,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T128_RX_PROTOCOL_VIOLATION_INT0 */
  /*213*/  {/*baseAddr*/ 0x920,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T128_RX_PROTOCOL_VIOLATION_INT1 */

    /* Mif Type 128 Rx Protocol Violation Interrupt Mask */
  /*214*/  {/*baseAddr*/ 0x924,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T128_RX_PROTOCOL_VIOLATION_INT_MSK0 */
  /*215*/  {/*baseAddr*/ 0x924,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T128_RX_PROTOCOL_VIOLATION_INT_MSK1 */

    /* Mif Type 128 Rx Overwrite Interrupt Cause */
  /*216*/  {/*baseAddr*/ 0x928,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*T128_RX_OVERWRITE_VIOLATION_INT_SUM */
  /*217*/  {/*baseAddr*/ 0x928,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T128_RX_OVERWRITE_VIOLATION_INT0 */
  /*218*/  {/*baseAddr*/ 0x928,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T128_RX_OVERWRITE_VIOLATION_INT1 */

    /* Mif Type 128 Rx Overwrite Interrupt Mask */
  /*219*/  {/*baseAddr*/ 0x92c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T128_RX_OVERWRITE_VIOLATION_INT_MSK0 */
  /*220*/  {/*baseAddr*/ 0x92c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T128_RX_OVERWRITE_VIOLATION_INT_MSK1 */

    /* Mif Type 8 Rx Status 0 */
  /*221*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 7 }, /*T8_RX_BYTE_CNT */
  /*222*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T8_RX_FIFO_FULL */
  /*223*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T8_RX_FIFO_EMPTY */
  /*224*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x4, /*fieldStart*/ 10, /*fieldLen*/ 2 }, /*T8_RX_PROT_VIO_TYPE */
  /*225*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T8_RX_BUS_MASTER */

    /* Mif Type 32 Rx Status 0 */
  /*226*/  {/*baseAddr*/ 0x8a0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 7 }, /*T32_RX_SC_BYTE_CNT */
  /*227*/  {/*baseAddr*/ 0x8a0,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T32_RX_FIFO_FULL */
  /*228*/  {/*baseAddr*/ 0x8a0,  /*offsetFormula*/ 0x4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T32_RX_FIFO_EMPTY */
  /*229*/  {/*baseAddr*/ 0x8a0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 10, /*fieldLen*/ 2 }, /*T32_RX_PROT_VIO_TYPE */
  /*230*/  {/*baseAddr*/ 0x8a0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T32_RX_BUS_MASTER */

    /* Mif Type 128 Rx Status 0 */
  /*231*/  {/*baseAddr*/ 0x8e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 7 }, /*T128_RX_SC_BYTE_CNT */
  /*232*/  {/*baseAddr*/ 0x8e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T128_RX_FIFO_FULL */
  /*233*/  {/*baseAddr*/ 0x8e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T128_RX_FIFO_EMPTY */
  /*234*/  {/*baseAddr*/ 0x8e0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 10, /*fieldLen*/ 2 }, /*T128_RX_PROT_VIO_TYPE */
  /*235*/  {/*baseAddr*/ 0x8e0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T128_RX_BUS_MASTER */

    /* Mif Type 8 Rx Good Packets Count0 */
  /*236*/  {/*baseAddr*/ 0xb00,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T8_RX_GOOD_PACKET_CNT */

    /* Mif Type 8 Rx Bad Packets Counter0 */
  /*237*/  {/*baseAddr*/ 0xb80,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T8_RX_BAD_PACKET_CNT */

    /* Mif Type 8 Rx Discarded Packets Count0 */
  /*238*/  {/*baseAddr*/ 0xc00,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T8_RX_DISCARDED_PACKET_CNT */

    /* Mif Type 32 Rx Good Packets Count0 */
  /*239*/  {/*baseAddr*/ 0xd00,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T32_RX_GOOD_PACKET_CNT */

    /* Mif Type 32 Rx Bad Packets Count0 */
  /*240*/  {/*baseAddr*/ 0xd40,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T32_RX_BAD_PACKET_CNT */

    /* Mif Type 32 Rx Discarded Packets Count0 */
  /*241*/  {/*baseAddr*/ 0xd80,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T32_RX_DISCARDED_PACKET_CNT */

    /* Mif Type 128 Rx Good Packets Count0 */
  /*242*/  {/*baseAddr*/ 0xe00,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T128_RX_GOOD_PACKET_CNT */

    /* Mif Type 128 Rx Bad Packets Count0 */
  /*243*/  {/*baseAddr*/ 0xe10,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T128_RX_BAD_PACKET_CNT */

    /* Mif Type 128 Rx Discarded Packets Count0 */
  /*244*/  {/*baseAddr*/ 0xe20,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T128_RX_DISCARDED_PACKET_CNT */

    /* Mif Type 8 Rx Pfc Control */
  /*245*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN0 */
  /*246*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN1 */
  /*247*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN2 */
  /*248*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN3 */
  /*249*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN4 */
  /*250*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN5 */
  /*251*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN6 */
  /*252*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN7 */
  /*253*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN8 */
  /*254*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN9 */
  /*255*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN10 */
  /*256*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN11 */
  /*257*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN12 */
  /*258*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN13 */
  /*259*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN14 */
  /*260*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN15 */
  /*261*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN16 */
  /*262*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN17 */
  /*263*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN18 */
  /*264*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN19 */
  /*265*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN20 */
  /*266*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN21 */
  /*267*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN22 */
  /*268*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN23 */
  /*269*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*MIF_T8_RX_PFC_EN24 */

    /* Mif Type 32 Rx Pfc Control */
  /*270*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MIF_T32_RX_PFC_EN0 */
  /*271*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MIF_T32_RX_PFC_EN1 */
  /*272*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*MIF_T32_RX_PFC_EN2 */
  /*273*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*MIF_T32_RX_PFC_EN3 */
  /*274*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*MIF_T32_RX_PFC_EN4 */
  /*275*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*MIF_T32_RX_PFC_EN5 */
  /*276*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*MIF_T32_RX_PFC_EN6 */
  /*277*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*MIF_T32_RX_PFC_EN7 */
  /*278*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*MIF_T32_RX_PFC_EN8 */
  /*279*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*MIF_T32_RX_PFC_EN9 */
  /*280*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*MIF_T32_RX_PFC_EN10 */
  /*281*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*MIF_T32_RX_PFC_EN11 */
  /*282*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*MIF_T32_RX_PFC_EN12 */
  /*283*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*MIF_T32_RX_PFC_EN13 */
  /*284*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*MIF_T32_RX_PFC_EN14 */
  /*285*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*MIF_T32_RX_PFC_EN15 */

    /* Mif Type 128 Rx Pfc Control */
  /*286*/  {/*baseAddr*/ 0x814,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MIF_T128_RX_PFC_EN0 */
  /*287*/  {/*baseAddr*/ 0x814,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MIF_T128_RX_PFC_EN1 */

    /* MIF Rx FIFO ready threshold */
  /*288*/  {/*baseAddr*/ 0x818,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*MIF_T8_RX_FIFO_THRESHOLD */
  /*289*/  {/*baseAddr*/ 0x818,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 4 }, /*MIF_T32_RX_FIFO_THRESHOLD */
  /*290*/  {/*baseAddr*/ 0x818,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 4 }, /*MIF_T128_RX_FIFO_THRESHOLD */
/*****************************************************************************************************/
/*****************************************************************************************************/
/*****************************************************************************************************/
/*****************************************************************************************************/
/*****************************************************************************************************/
/*****************************************************************************************************/
/*****************************************************************************************************/
    /* Mif Type 128 Tx Control  */
  /*0*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MIF_T128_TX_EN0 */
  /*1*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MIF_T128_TX_EN1 */

    /* Mif Type 8 Tx Control  */
  /*2*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN0 */
  /*3*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN1 */
  /*4*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN2 */
  /*5*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN3 */
  /*6*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN4 */
  /*7*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN5 */
  /*8*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN6 */
  /*9*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN7 */
  /*10*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN8 */
  /*11*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN9 */
  /*12*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN10 */
  /*13*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN11 */
  /*14*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN12 */
  /*15*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN13 */
  /*16*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN14 */
  /*17*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN15 */
  /*18*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN16 */
  /*19*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN17 */
  /*20*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN18 */
  /*21*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN19 */
  /*22*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN20 */
  /*23*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN21 */
  /*24*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN22 */
  /*25*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN23 */
  /*26*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*MIF_T8_TX_EN24 */

    /* Mif Type 32 Tx Control */
  /*27*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MIF_T32_TX_EN0 */
  /*28*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MIF_T32_TX_EN1 */
  /*29*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*MIF_T32_TX_EN2 */
  /*30*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*MIF_T32_TX_EN3 */
  /*31*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*MIF_T32_TX_EN4 */
  /*32*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*MIF_T32_TX_EN5 */
  /*33*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*MIF_T32_TX_EN6 */
  /*34*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*MIF_T32_TX_EN7 */
  /*35*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*MIF_T32_TX_EN8 */
  /*36*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*MIF_T32_TX_EN9 */
  /*37*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*MIF_T32_TX_EN10 */
  /*38*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*MIF_T32_TX_EN11 */
  /*39*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*MIF_T32_TX_EN12 */
  /*40*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*MIF_T32_TX_EN13 */
  /*41*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*MIF_T32_TX_EN14 */
  /*42*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*MIF_T32_TX_EN15 */

    /* Mif Link Status Filter  */
  /*43*/  {/*baseAddr*/ 0x1700,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*LINK_FILTER_SIZE */

    /* Mif Type 8 Link Fsm Control 0 */
  /*44*/  {/*baseAddr*/ 0x1710,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*T8_LINK_FSM_DISABLE */
  /*45*/  {/*baseAddr*/ 0x1710,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T8_FORCE_LINK_STATUS */
  /*46*/  {/*baseAddr*/ 0x1710,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T8_LINK_STATUS_VALUE */
  /*47*/  {/*baseAddr*/ 0x1710,  /*offsetFormula*/ 0x4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T8_EOP_AT_LINK_DOWN */

    /* Mif Type 32 Link Fsm Control 0 */
  /*48*/  {/*baseAddr*/ 0x1790,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*T32_LINK_FSM_DISABLE */
  /*49*/  {/*baseAddr*/ 0x1790,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T32_FORCE_LINK_STATUS */
  /*50*/  {/*baseAddr*/ 0x1790,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T32_LINK_STATUS_VALUE */
  /*51*/  {/*baseAddr*/ 0x1790,  /*offsetFormula*/ 0x4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T32_EOP_AT_LINK_DOWN */

    /* Mif Type 128 Link Fsm Control 0 */
  /*52*/  {/*baseAddr*/ 0x17d0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*T128_LINK_FSM_DISABLE */
  /*53*/  {/*baseAddr*/ 0x17d0,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T128_FORCE_LINK_STATUS */
  /*54*/  {/*baseAddr*/ 0x17d0,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T128_LINK_STATUS_VALUE */
  /*55*/  {/*baseAddr*/ 0x17d0,  /*offsetFormula*/ 0x4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T128_EOP_AT_LINK_DOWN */

    /* Mif Type 8 Tx Protocol Violation Interrupt Cause */
  /*56*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT */
  /*57*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT0 */
  /*58*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT1 */
  /*59*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT2 */
  /*60*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT3 */
  /*61*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT4 */
  /*62*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT5 */
  /*63*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT6 */
  /*64*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT7 */
  /*65*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT8 */
  /*66*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT9 */
  /*67*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT10 */
  /*68*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT11 */
  /*69*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT12 */
  /*70*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT13 */
  /*71*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT14 */
  /*72*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT15 */
  /*73*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT16 */
  /*74*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT17 */
  /*75*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT18 */
  /*76*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT19 */
  /*77*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT20 */
  /*78*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT21 */
  /*79*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT22 */
  /*80*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT23 */
  /*81*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT24 */

    /* Mif Type 8 Tx Protocol Violation Interrupt Mask */
  /*82*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK0 */
  /*83*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK1 */
  /*84*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK2 */
  /*85*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK3 */
  /*86*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK4 */
  /*87*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK5 */
  /*88*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK6 */
  /*89*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK7 */
  /*90*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK8 */
  /*91*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK9 */
  /*92*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK10 */
  /*93*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK11 */
  /*94*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK12 */
  /*95*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK13 */
  /*96*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK14 */
  /*97*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK15 */
  /*98*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK16 */
  /*99*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK17 */
  /*100*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK18 */
  /*101*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK19 */
  /*102*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK20 */
  /*103*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK21 */
  /*104*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK22 */
  /*105*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK23 */
  /*106*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*T8_TX_PROTOCOL_VIOLATION_INT_MSK24 */

    /* Mif Type 8 Tx Overwrite Interrupt Cause */
  /*107*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_SUM */
  /*108*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT0 */
  /*109*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT1 */
  /*110*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT2 */
  /*111*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT3 */
  /*112*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT4 */
  /*113*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT5 */
  /*114*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT6 */
  /*115*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT7 */
  /*116*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT8 */
  /*117*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT9 */
  /*118*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT10 */
  /*119*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT11 */
  /*120*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT12 */
  /*121*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT13 */
  /*122*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT14 */
  /*123*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT15 */
  /*124*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT16 */
  /*125*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT17 */
  /*126*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT18 */
  /*127*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT19 */
  /*128*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT20 */
  /*129*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT21 */
  /*130*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT22 */
  /*131*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT23 */
  /*132*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT24 */

    /* Mif Type 8 Tx Overwrite Interrupt Mask */
  /*133*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK0 */
  /*134*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK1 */
  /*135*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK2 */
  /*136*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK3 */
  /*137*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK4 */
  /*138*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK5 */
  /*139*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK6 */
  /*140*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK7 */
  /*141*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK8 */
  /*142*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK9 */
  /*143*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK10 */
  /*144*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK11 */
  /*145*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK12 */
  /*146*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK13 */
  /*147*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK14 */
  /*148*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK15 */
  /*149*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK16 */
  /*150*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK17 */
  /*151*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK18 */
  /*152*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK19 */
  /*153*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK20 */
  /*154*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK21 */
  /*155*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK22 */
  /*156*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK23 */
  /*157*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*T8_TX_OVERWRITE_VIOLATION_INT_MSK24 */

    /* Mif Type 32 Tx Protocol Violation Interrupt Cause */
  /*158*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_SUM */
  /*159*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT0 */
  /*160*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT1 */
  /*161*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT2 */
  /*162*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT3 */
  /*163*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT4 */
  /*164*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT5 */
  /*165*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT6 */
  /*166*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT7 */
  /*167*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT8 */
  /*168*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT9 */
  /*169*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT10 */
  /*170*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT11 */
  /*171*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT12 */
  /*172*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT13 */
  /*173*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT14 */
  /*174*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT15 */

    /* Mif Type 32 Tx Protocol Violation Interrupt Mask */
  /*175*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_MSK0 */
  /*176*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_MSK1 */
  /*177*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_MSK2 */
  /*178*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_MSK3 */
  /*179*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_MSK4 */
  /*180*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_MSK5 */
  /*181*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_MSK6 */
  /*182*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_MSK7 */
  /*183*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_MSK8 */
  /*184*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_MSK9 */
  /*185*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_MSK10 */
  /*186*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_MSK11 */
  /*187*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_MSK12 */
  /*188*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_MSK13 */
  /*189*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_MSK14 */
  /*190*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T32_TX_PROTOCOL_VIOLATION_INT_MSK15 */

    /* Mif Type 32 Tx Overwrite Interrupt Cause */
  /*191*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_SUM */
  /*192*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT0 */
  /*193*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT1 */
  /*194*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT2 */
  /*195*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT3 */
  /*196*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT4 */
  /*197*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT5 */
  /*198*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT6 */
  /*199*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT7 */
  /*200*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT8 */
  /*201*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT9 */
  /*202*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT10 */
  /*203*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT11 */
  /*204*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT12 */
  /*205*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT13 */
  /*206*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT14 */
  /*207*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT15 */

    /* Mif Type 32 Tx Overwrite Interrupt Mask */
  /*208*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_MSK0 */
  /*209*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_MSK1 */
  /*210*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_MSK2 */
  /*211*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_MSK3 */
  /*212*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_MSK4 */
  /*213*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_MSK5 */
  /*214*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_MSK6 */
  /*215*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_MSK7 */
  /*216*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_MSK8 */
  /*217*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_MSK9 */
  /*218*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_MSK10 */
  /*219*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_MSK11 */
  /*220*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_MSK12 */
  /*221*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_MSK13 */
  /*222*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_MSK14 */
  /*223*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*T32_TX_OVERWRITE_VIOLATION_INT_MSK15 */

    /* Mif Type 128 Tx Protocol Violation Interrupt Cause */
  /*224*/  {/*baseAddr*/ 0x1120,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*T128_TX_PROTOCOL_VIOLATION_INT_SUM */
  /*225*/  {/*baseAddr*/ 0x1120,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T128_TX_PROTOCOL_VIOLATION_INT0 */
  /*226*/  {/*baseAddr*/ 0x1120,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T128_TX_PROTOCOL_VIOLATION_INT1 */

    /* Mif Type 128 Tx Protocol Violation Interrupt Mask */
  /*227*/  {/*baseAddr*/ 0x1124,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T128_TX_PROTOCOL_VIOLATION_INT_MSK0 */
  /*228*/  {/*baseAddr*/ 0x1124,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T128_TX_PROTOCOL_VIOLATION_INT_MSK1 */

    /* Mif Type 128 Tx Overwrite Interrupt Cause */
  /*229*/  {/*baseAddr*/ 0x1128,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*T128_TX_OVERWRITE_VIOLATION_INT_SUM */
  /*230*/  {/*baseAddr*/ 0x1128,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T128_TX_OVERWRITE_VIOLATION_INT0 */
  /*231*/  {/*baseAddr*/ 0x1128,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T128_TX_OVERWRITE_VIOLATION_INT1 */

    /* Mif Type 128 Tx Overwrite Interrupt Mask */
  /*232*/  {/*baseAddr*/ 0x112c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*T128_TX_OVERWRITE_VIOLATION_INT_MSK0 */
  /*233*/  {/*baseAddr*/ 0x112c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*T128_TX_OVERWRITE_VIOLATION_INT_MSK1 */

    /* Mif Type 8 Tx Status 0 */
  /*234*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 7 }, /*T8_TX_SC_BYTE_CNT */
  /*235*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T8_TX_FIFO_FULL */
  /*236*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T8_TX_FIFO_EMPTY */
  /*237*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x4, /*fieldStart*/ 10, /*fieldLen*/ 2 }, /*T8_TX_PROT_VIO_TYPE */
  /*238*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*T8_TX_CREDIT_CNT */
  /*239*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x4, /*fieldStart*/ 24, /*fieldLen*/ 8 }, /*T8_TX_CREDIT_DRIFT_CNT */

    /* Mif Type 32 Tx Status 0 */
  /*240*/  {/*baseAddr*/ 0x10a0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 7 }, /*T32_TX_SC_BYTE_CNT */
  /*241*/  {/*baseAddr*/ 0x10a0,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T32_TX_FIFO_FULL */
  /*242*/  {/*baseAddr*/ 0x10a0,  /*offsetFormula*/ 0x4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T32_TX_FIFO_EMPTY */
  /*243*/  {/*baseAddr*/ 0x10a0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 10, /*fieldLen*/ 2 }, /*T32_TX_PROT_VIO_TYPE */
  /*244*/  {/*baseAddr*/ 0x10a0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*T32_TX_CREDIT_CNT */
  /*245*/  {/*baseAddr*/ 0x10a0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 24, /*fieldLen*/ 8 }, /*T32_TX_CREDIT_DRIFT_CNT */

    /* Mif Type 128 Tx Status 0 */
  /*246*/  {/*baseAddr*/ 0x10e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 7 }, /*T128_TX_SC_BYTE_CNT */
  /*247*/  {/*baseAddr*/ 0x10e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T128_TX_FIFO_FULL */
  /*248*/  {/*baseAddr*/ 0x10e0,  /*offsetFormula*/ 0x4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T128_TX_FIFO_EMPTY */
  /*249*/  {/*baseAddr*/ 0x10e0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 10, /*fieldLen*/ 2 }, /*T128_TX_PROT_VIO_TYPE */
  /*250*/  {/*baseAddr*/ 0x10e0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*T128_TX_CREDIT_CNT */
  /*251*/  {/*baseAddr*/ 0x10e0,  /*offsetFormula*/ 0x4, /*fieldStart*/ 24, /*fieldLen*/ 8 }, /*T128_TX_CREDIT_DRIFT_CNT */

    /* Mif Type 8 Tx Credit 0 */
  /*252*/  {/*baseAddr*/ 0x1200,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*T8_TX_CREDIT_ALLOC */
  /*253*/  {/*baseAddr*/ 0x1200,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T8_IGNORE_MAC_FILL_LEVEL */
  /*254*/  {/*baseAddr*/ 0x1200,  /*offsetFormula*/ 0x4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T8_IGNORE_MIF_FILL_LEVEL */

    /* Mif Type 32 Tx Credit 0 */
  /*255*/  {/*baseAddr*/ 0x1290,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*T32_TX_CREDIT_ALLOC */
  /*256*/  {/*baseAddr*/ 0x1290,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T32_IGNORE_MAC_FILL_LEVEL */
  /*257*/  {/*baseAddr*/ 0x1290,  /*offsetFormula*/ 0x4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T32_IGNORE_MIF_FILL_LEVEL */

    /* Mif Type 128tx Credit 0 */
  /*258*/  {/*baseAddr*/ 0x12d0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*T128_TX_CREDIT_ALLOC */
  /*259*/  {/*baseAddr*/ 0x12d0,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*T128_IGNORE_MAC_FILL_LEVEL */
  /*260*/  {/*baseAddr*/ 0x12d0,  /*offsetFormula*/ 0x4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*T128_IGNORE_MIF_FILL_LEVEL */

    /* Mif Type 8 Tx Good Packets Count0 */
  /*261*/  {/*baseAddr*/ 0x1300,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T8_TX_GOOD_PACKET_CNT */

    /* Mif Type 8 Tx Bad Packets Count0 */
  /*262*/  {/*baseAddr*/ 0x1380,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T8_TX_BAD_PACKET_CNT */

    /* Mif Type 8 Tx Discarded Packets Count0 */
  /*263*/  {/*baseAddr*/ 0x1400,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T8_TX_DISCARDED_PACKET_CNT */

    /* Mif Type 8 Tx Link Fail Count0 */
  /*264*/  {/*baseAddr*/ 0x1480,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*T8_TX_LINK_FAIL_CNT */

    /* Mif Type 32 Tx Good Packets Count0 */
  /*265*/  {/*baseAddr*/ 0x1500,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T32_TX_GOOD_PACKET_CNT */

    /* Mif Type 32 Tx Bad Packets Count0 */
  /*266*/  {/*baseAddr*/ 0x1540,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T32_TX_BAD_PACKET_CNT */

    /* Mif Type 32 Tx Discarded Packets Count0 */
  /*267*/  {/*baseAddr*/ 0x1580,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T32_TX_DISCARDED_PACKET_CNT */

    /* Mif Type 32 Tx Link Fail Count0 */
  /*268*/  {/*baseAddr*/ 0x15c0,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*T32_TX_LINK_FAIL_CNT */

    /* Mif Type 128 Tx Good Packets Count0 */
  /*269*/  {/*baseAddr*/ 0x1600,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T128_TX_GOOD_PACKET_CNT */

    /* Mif Type 128 Tx Bad Packets Count0 */
  /*270*/  {/*baseAddr*/ 0x1610,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T128_TX_BAD_PACKET_CNT */

    /* Mif Type 128 Tx Discarded Packets Count0 */
  /*271*/  {/*baseAddr*/ 0x1620,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*T128_TX_DISCARDED_PACKET_CNT */

    /* Mif Type 128 Tx Link Fail Count0 */
  /*272*/  {/*baseAddr*/ 0x1630,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*T128_TX_LINK_FAIL_CNT */

    /* Mif Type 8 Tx Pfc Control */
  /*273*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN0 */
  /*274*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN1 */
  /*275*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN2 */
  /*276*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN3 */
  /*277*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN4 */
  /*278*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN5 */
  /*279*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN6 */
  /*280*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN7 */
  /*281*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN8 */
  /*282*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN9 */
  /*283*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN10 */
  /*284*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN11 */
  /*285*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN12 */
  /*286*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN13 */
  /*287*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN14 */
  /*288*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN15 */
  /*289*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN16 */
  /*290*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN17 */
  /*291*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN18 */
  /*292*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN19 */
  /*293*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN20 */
  /*294*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN21 */
  /*295*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN22 */
  /*296*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN23 */
  /*297*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*MIF_T8_TX_PFC_EN24 */

    /* Mif Type 32 Tx Pfc Control */
  /*298*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MIF_T32_TX_PFC_EN0 */
  /*299*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MIF_T32_TX_PFC_EN1 */
  /*300*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*MIF_T32_TX_PFC_EN2 */
  /*301*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*MIF_T32_TX_PFC_EN3 */
  /*302*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*MIF_T32_TX_PFC_EN4 */
  /*303*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*MIF_T32_TX_PFC_EN5 */
  /*304*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*MIF_T32_TX_PFC_EN6 */
  /*305*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*MIF_T32_TX_PFC_EN7 */
  /*306*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*MIF_T32_TX_PFC_EN8 */
  /*307*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*MIF_T32_TX_PFC_EN9 */
  /*308*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*MIF_T32_TX_PFC_EN10 */
  /*309*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*MIF_T32_TX_PFC_EN11 */
  /*310*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*MIF_T32_TX_PFC_EN12 */
  /*311*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*MIF_T32_TX_PFC_EN13 */
  /*312*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*MIF_T32_TX_PFC_EN14 */
  /*313*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*MIF_T32_TX_PFC_EN15 */

    /* Mif Type 128 Tx Pfc Control */
  /*314*/  {/*baseAddr*/ 0x1014,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MIF_T128_TX_PFC_EN0 */
  /*315*/  {/*baseAddr*/ 0x1014,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MIF_T128_TX_PFC_EN1 */
};
