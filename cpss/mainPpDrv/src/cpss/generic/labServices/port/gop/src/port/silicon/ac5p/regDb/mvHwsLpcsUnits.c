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
* @file mvHwsLpcsUnits.c
*
* @brief Hawk MTI LPCS register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>



const MV_HWS_REG_ADDR_FIELD_STC hawkLpcsUnitsDb[] = {
    /* PORT<0>_CONTROL */
  /*0*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x80, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_SPEED_6 */
  /*1*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x80, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_DUPLEX */
  /*2*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x80, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_ANRESTART */
  /*3*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x80, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*IP0_SOLATE */
  /*4*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x80, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_POWERDOWN */
  /*5*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x80, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_ANENABLE */
  /*6*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x80, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_SPEED_13 */
  /*7*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x80, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_LOOPBACK */
  /*8*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x80, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_RESET */

    /* PORT<0>_STATUS */
  /*9*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_EXTDCAPABILITY */
  /*10*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x80, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_LINKSTATUS */
  /*11*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x80, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_ANEGABILITY */
  /*12*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x80, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_ANEGCOMPLETE */

    /* PORT<0>_PHY_ID0 */
  /*13*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_PHYID0 */

    /* PORT<0>_PHY_ID1 */
  /*14*/  {/*baseAddr*/ 0x000c,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_PHYID1 */

    /* PORT<0>_DEV_ABILITY */
  /*15*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*P0_LD_ABILITY_RSV05 */
  /*16*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x80, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_LD_FD */
  /*17*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x80, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_LD_HD */
  /*18*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x80, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_LD_PS1 */
  /*19*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x80, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_LD_PS2 */
  /*20*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x80, /*fieldStart*/  9, /*fieldLen*/ 3 }, /*P0_LD_ABILITY_RSV9 */
  /*21*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x80, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_LD_RF1 */
  /*22*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x80, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_LD_RF2 */
  /*23*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x80, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_LD_ACK */
  /*24*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x80, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_LD_NP */

    /* PORT<0>_PARTNER_ABILITY */
  /*25*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*P0_LP_PABILITY_RSV05 */
  /*26*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x80, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_LP_FD */
  /*27*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x80, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_LP_HD */
  /*28*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x80, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_LP_PS1 */
  /*29*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x80, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_LP_PS2 */
  /*30*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x80, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_LP_PABILITY_RSV9 */
  /*31*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x80, /*fieldStart*/ 10, /*fieldLen*/ 2 }, /*P0_LP_PABILITY_RSV10 */
  /*32*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x80, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_LP_RF1 */
  /*33*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x80, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_LP_RF2 */
  /*34*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x80, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_LP_ACK */
  /*35*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x80, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_LP_NP */

    /* PORT<0>_AN_EXPANSION */
  /*36*/  {/*baseAddr*/ 0x0018,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PAGERECEIVEDREALTIME */
  /*37*/  {/*baseAddr*/ 0x0018,  /*offsetFormula*/ 0x80, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PAGERECEIVED */
  /*38*/  {/*baseAddr*/ 0x0018,  /*offsetFormula*/ 0x80, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_NEXTPAGEABLE */

    /* PORT<0>_NP_TX */
  /*39*/  {/*baseAddr*/ 0x001c,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*P0_LD_NP_DATA */
  /*40*/  {/*baseAddr*/ 0x001c,  /*offsetFormula*/ 0x80, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_LD_NP_TOGGLE */
  /*41*/  {/*baseAddr*/ 0x001c,  /*offsetFormula*/ 0x80, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_LD_NP_ACK2 */
  /*42*/  {/*baseAddr*/ 0x001c,  /*offsetFormula*/ 0x80, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_LD_NP_MP */
  /*43*/  {/*baseAddr*/ 0x001c,  /*offsetFormula*/ 0x80, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_LD_NP_LD_NP_ACK */
  /*44*/  {/*baseAddr*/ 0x001c,  /*offsetFormula*/ 0x80, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_LD_NP_NP */

    /* PORT<0>_LP_NP_RX */
  /*45*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*P0_LP_NP_DATA */
  /*46*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x80, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P0_LP_NP_TOGGLE */
  /*47*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x80, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_LP_NP_ACK2 */
  /*48*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x80, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P0_LP_NP_MP */
  /*49*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x80, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P0_LP_NP_ACK */
  /*50*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x80, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P0_LP_NP_NP */

    /* PORT<0>_SCRATCH */
  /*51*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_SCRATCH */

    /* PORT<0>_REV */
  /*52*/  {/*baseAddr*/ 0x0044,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_REVISION */

    /* PORT<0>_LINK_TIMER_0 */
  /*53*/  {/*baseAddr*/ 0x0048,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_TIMER0 */
  /*54*/  {/*baseAddr*/ 0x0048,  /*offsetFormula*/ 0x80, /*fieldStart*/  1, /*fieldLen*/ 15 }, /*P0_TIMER15_1 */

    /* PORT<0>_LINK_TIMER_1 */
  /*55*/  {/*baseAddr*/ 0x004c,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*P0_TIMER20_16 */

    /* PORT<0>_IF_MODE */
  /*56*/  {/*baseAddr*/ 0x0050,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_SGMII_ENA */
  /*57*/  {/*baseAddr*/ 0x0050,  /*offsetFormula*/ 0x80, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_USE_SGMII_AN */
  /*58*/  {/*baseAddr*/ 0x0050,  /*offsetFormula*/ 0x80, /*fieldStart*/  2, /*fieldLen*/ 2 }, /*P0_SGMII_SPEED */
  /*59*/  {/*baseAddr*/ 0x0050,  /*offsetFormula*/ 0x80, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_SGMII_DUPLEX */
  /*60*/  {/*baseAddr*/ 0x0050,  /*offsetFormula*/ 0x80, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_IFMODE_RSV5 */
  /*61*/  {/*baseAddr*/ 0x0050,  /*offsetFormula*/ 0x80, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_IFMODE_TX_PREAMBLE_SYNC */
  /*62*/  {/*baseAddr*/ 0x0050,  /*offsetFormula*/ 0x80, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_IFMODE_RX_PREAMBLE_SYNC */
  /*63*/  {/*baseAddr*/ 0x0050,  /*offsetFormula*/ 0x80, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_IFMODE_MODE_XGMII_BASEX */
  /*64*/  {/*baseAddr*/ 0x0050,  /*offsetFormula*/ 0x80, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_IFMODE_SEQ_ENA */

    /* PORT<0>_DECODE_ERRORS */
  /*65*/  {/*baseAddr*/ 0x0054,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_DECODEERRORS */

    /* PORT<0>_USXGMII_REP */
  /*66*/  {/*baseAddr*/ 0x0058,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*USXGMIIREP */
  /*67*/  {/*baseAddr*/ 0x0058,  /*offsetFormula*/ 0x80, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P0_USXGMII2_5 */

    /* GMODE */
  /*68*/  {/*baseAddr*/ 0x03e0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*LPCS_ENABLE */
  /*69*/  {/*baseAddr*/ 0x03e0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*QSGMII_0_ENABLE */
  /*70*/  {/*baseAddr*/ 0x03e0,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*QSGMII_1_ENABLE */
  /*71*/  {/*baseAddr*/ 0x03e0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*USGMII8_ENABLE */
  /*72*/  {/*baseAddr*/ 0x03e0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*USGMII_SCRAMBLE_ENABLE */

    /* GSTATUS */
  /*73*/  {/*baseAddr*/ 0x03e4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GSYNC_STATUS */
  /*74*/  {/*baseAddr*/ 0x03e4,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*GAN_DONE_STATUS */

    /* CFG_CLOCK_RATE */
  /*75*/  {/*baseAddr*/ 0x03f0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*CFGCLOCKRATE */

    /* USXGMII_ENABLE_INDICATION */
  /*76*/  {/*baseAddr*/ 0x03f4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*USXGMII_ENABLE_IND */

    /* PORT<0>_TX_IPG_LENGTH */
  /*77*/  {/*baseAddr*/ 0x005c,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 7 }, /*P0_TX_IPG_LENGTH */

    /* USXM1_PORTS_ENA */
  /*78*/  {/*baseAddr*/ 0x03ec,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*USXM1_PORTS_ENA */

    /* USXM0_PORTS_ENA */
  /*79*/  {/*baseAddr*/ 0x03e8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*USXM0_PORTS_ENA */
};

