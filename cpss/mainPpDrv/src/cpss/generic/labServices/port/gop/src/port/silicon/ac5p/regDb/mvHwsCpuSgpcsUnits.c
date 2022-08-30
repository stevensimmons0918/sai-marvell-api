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
* @file mvHwsCpuSgpcsUnits.c
*
* @brief Hawk MTI CPU SGPCS register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>



const MV_HWS_REG_ADDR_FIELD_STC hawkCpuSgpcsUnitsDb[] = {
    /* PORT_CONTROL */
  /*0*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PORT_SPEED_6 */
  /*1*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PORT_DUPLEX */
  /*2*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*PORT_AN_RESTART */
  /*3*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*PORT_ISOLATE */
  /*4*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*PORT_POWERDOWN */
  /*5*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*PORT_AN_ENABLE */
  /*6*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*PORT_SPEED_13 */
  /*7*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*PORT_LOOPBACK */
  /*8*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PORT_RESET */

    /* PORT_STATUS */
  /*9*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PORT_EXTDCAPABILITY */
  /*10*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PORT_LINKSTATUS */
  /*11*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PORT_ANEGABILITY */
  /*12*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PORT_ANEGCOMPLETE */

    /* PORT_PHY_ID0 */
  /*13*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*PORT_PHYID0 */

    /* PORT_PHY_ID1 */
  /*14*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*PORT_PHYID1 */

    /* PORT_DEV_ABILITY */
  /*15*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*PORT_LD_ABILITY_RSV05 */
  /*16*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PORT_LD_FD */
  /*17*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PORT_LD_HD */
  /*18*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PORT_LD_PS1 */
  /*19*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PORT_LD_PS2 */
  /*20*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 3 }, /*PORT_LD_ABILITY_RSV9 */
  /*21*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*PORT_LD_RF1 */
  /*22*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*PORT_LD_RF2 */
  /*23*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*PORT_LD_ACK */
  /*24*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PORT_LD_NP */

    /* PORT_PARTNER_ABILITY */
  /*25*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*PORT_LP_PABILITY_RSV05 */
  /*26*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PORT_LP_FD */
  /*27*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PORT_LP_HD */
  /*28*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PORT_LP_PS1 */
  /*29*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PORT_LP_PS2 */
  /*30*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*PORT_LP_PABILITY_RSV9 */
  /*31*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 2 }, /*PORT_LP_PABILITY_RSV10 */
  /*32*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*PORT_LP_RF1 */
  /*33*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*PORT_LP_RF2 */
  /*34*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*PORT_LP_ACK */
  /*35*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PORT_LP_NP */

    /* PORT_AN_EXPANSION */
  /*36*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PORT_PAGERECEIVEDREALTIME */
  /*37*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PORT_PAGERECEIVED */
  /*38*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PORT_NEXTPAGEABLE */

    /* PORT_NP_TX */
  /*39*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*PORT_LD_NP_DATA */
  /*40*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*PORT_LD_NP_TOGGLE */
  /*41*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*PORT_LD_NP_ACK2 */
  /*42*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*PORT_LD_NP_MP */
  /*43*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*PORT_LD_NP_LD_NP_ACK */
  /*44*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PORT_LD_NP_NP */

    /* PORT_LP_NP_RX */
  /*45*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*PORT_LP_NP_DATA */
  /*46*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*PORT_LP_NP_TOGGLE */
  /*47*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*PORT_LP_NP_ACK2 */
  /*48*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*PORT_LP_NP_MP */
  /*49*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*PORT_LP_NP_ACK */
  /*50*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PORT_LP_NP_NP */

    /* PORT_SCRATCH */
  /*51*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*PORT_SCRATCH */

    /* PORT_REV */
  /*52*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*PORT_REVISION */

    /* PORT_LINK_TIMER_0 */
  /*53*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PORT_TIMER0 */
  /*54*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 15 }, /*PORT_TIMER15_1 */

    /* PORT_LINK_TIMER_1 */
  /*55*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*PORT_TIMER20_16 */

    /* PORT_IF_MODE */
  /*56*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PORT_SGMII_ENA */
  /*57*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PORT_USE_SGMII_AN */
  /*58*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 2 }, /*PORT_SGMII_SPEED */
  /*59*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PORT_SGMII_DUPLEX */

    /* PORT_DEC_ERR_CNT */
  /*60*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*PORT_DECODER_ERR_COUNTER */

    /* PORT_VENDOR_CONTROL */
  /*61*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PORT_SGPCS_ENA_R */
  /*62*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 3 }, /*PORT_VC_RESERVED */
  /*63*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*PORT_CFG_CLOCK_RATE */
  /*64*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 7 }, /*PORT_VC_RESERVED2 */
  /*65*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PORT_SGPCS_ENA_ST */
};

