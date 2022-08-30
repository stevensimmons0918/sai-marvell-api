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
* @file mvHwsIronmanMtipMac10GBrUnits.c
*
* @brief IronmanL MAC 10G BR register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanMtipMac10GBrUnitsDb[] = {
    /* REVISION */
  /*0*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*CORE_REVISION */
  /*1*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*CORE_VERSION */
  /*2*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CUSTOMER_REVISION */

    /* COMMAND_CONFIG */
  /*3*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PMAC_TX_ENA */
  /*4*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PMAC_RX_ENA */
  /*5*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*HD_ENA */
  /*6*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*CRC_FWD */
  /*7*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PMAC_PAUSE_FWD */
  /*8*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PMAC_PAUSE_IGNORE */
  /*9*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LOOPBACK_EN */
  /*10*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TX_PAD_EN */
  /*11*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*PMAC_SW_RESET */
  /*12*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*PMAC_CNTL_FRAME_ENA */
  /*13*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PHY_TXENA */
  /*14*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*SEND_IDLE */
  /*15*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*PMAC_PAUSE_MASK_P */
  /*16*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*PMAC_PAUSE_MASK_E */
  /*17*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PMAC_PFC_MODE */
  /*18*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*PMAC_PAUSE_PFC_COMP */
  /*19*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*RX_SFD_ANY */
  /*20*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*PMAC_TX_FLUSH */
  /*21*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*TX_LOWP_ENA */
  /*22*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*REG_LOWP_RXEMPTY */
  /*23*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*FLT_TX_STOP */
  /*24*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*PMAC_TX_FIFO_RESET */
  /*25*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*FLT_HDL_DIS */
  /*26*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*PMAC_TX_PAUSE_DIS */
  /*27*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*PMAC_RX_PAUSE_DIS */

    /* PMAC_ADDR_0 */
  /*28*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MAC_ADDRESS_0 */

    /* PMAC_ADDR_1 */
  /*29*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAC_ADDRESS_1 */

    /* PMAC_FRM_LENGTH */
  /*30*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FRM_LENGTH */
  /*31*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*TX_MTU */

    /* PMAC_TX_FIFO_SECTIONS */
  /*32*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*TX_SECTION_FULL */
  /*33*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 4 }, /*TX_SECTION_EMPTY */

    /* USX_PCH_CONTROL */
  /*34*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PCH_ENA */
  /*35*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*CRC_REVERSE */
  /*36*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*TX_FORCE_1S_PTP */
  /*37*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*TX_PTP_DIS */
  /*38*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*TX_TS_CAP_DIS */
  /*39*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*TX_TSID_OVR */
  /*40*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 2 }, /*TX_NOTS_PCH_MODE */
  /*41*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 2 }, /*TX_TS_PCH_MODE */
  /*42*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*RX_KEEP_PCH */
  /*43*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*RX_PTP_DIS */
  /*44*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*RX_NOPTP_USE_FRC */
  /*45*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*RX_CRCERR_USE_FRC */
  /*46*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*RX_SUBPORT_CHK_DIS */
  /*47*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*RX_CRC_CHK_DIS */
  /*48*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*RX_BADCRC_DISCARD */
  /*49*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*RX_NOPCH_CRC_DIS */
  /*50*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*RX_FWD_IDLE */
  /*51*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*RX_FWD_RSVD */
  /*52*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*RX_DROP_FE */
  /*53*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 28, /*fieldLen*/ 4 }, /*SUBPORT */

    /* STATUS */
  /*54*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*RX_LOC_FAULT */
  /*55*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RX_REM_FAULT */
  /*56*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PHY_LOS */
  /*57*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*RX_LOWP */
  /*58*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PMAC_TX_EMPTY */
  /*59*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PMAC_RX_EMPTY */
  /*60*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*RX_LINT_FAULT */
  /*61*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TX_IS_IDLE */
  /*62*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_MERGE_ISIDLE */
  /*63*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*EMAC_TX_EMPTY */
  /*64*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*EMAC_RX_EMPTY */
  /*65*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*AVB_FIFO_OVR */
  /*66*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*PCH_RX_SUBPORT_ERR */
  /*67*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*PCH_RX_CRC_ERR */
  /*68*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*PCH_RX_UNSUP */
  /*69*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*PCH_RX_FRM_DROP */
  /*70*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 28, /*fieldLen*/ 4 }, /*PCH_RX_SUBPORT */

    /* TX_IPG_LENGTH */
  /*71*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TXIPG_DIC_DISABLE */
  /*72*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*TXIPG_RESERVED1 */
  /*73*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*TXIPG_RESERVED2 */
  /*74*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 3 }, /*TXIPG */
  /*75*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*COMPENSATION_HI */
  /*76*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*COMPENSATION */

    /* CRC_MODE */
  /*77*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*DIS_RX_CRC_CHK */
  /*78*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*MCRC_INVERT */

    /* XIF_MODE */
  /*79*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*XGMII */
  /*80*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PAUSETIMERX8 */
  /*81*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*ONESTEP_ENA */
  /*82*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*EMAC_RX_PAUSE_BYPASS */
  /*83*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PMAC_RX_PAUSE_BYPASS */
  /*84*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TX_MAC_RS_ERR */
  /*85*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TS_BINARY_MODE */
  /*86*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*PFC_PULSE_MODE */
  /*87*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*TS_SFD_ENA */

    /* BR_CONTROL */
  /*88*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TX_PREEMPT_EN */
  /*89*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*TX_VERIF_DIS */
  /*90*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 3 }, /*TX_ADDFRAGSIZE */
  /*91*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 7 }, /*TX_VERIFY_TIME */
  /*92*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*TX_PAUSE_PRI_EN */
  /*93*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*TX_FRAG_PAUSE_EN */
  /*94*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*TX_NON_EMPTY_PREEMPT_EN */
  /*95*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*TX_ALLOW_PMAC_IF_NVERIF */
  /*96*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*TX_ALLOW_EMAC_IF_NVERIF */
  /*97*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*BR_COUNT_CLR_ON_RD */
  /*98*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*BR_COUNT_SAT */
  /*99*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*RX_STRICT_PREAMBLE */
  /*100*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*PMAC_TO_EMAC_STATS */
  /*101*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*RX_BR_SMD_DIS */
  /*102*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*RX_BR_STRICT_FRM_ENA */

    /* BR_STATUS */
  /*103*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*TX_VERIFY_STATUS */
  /*104*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TX_PREEMPT_STATUS */
  /*105*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*ASSY_ERR_LH */
  /*106*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*SMD_ERR_LH */
  /*107*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*VERIF_SEEN_LH */
  /*108*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*RESP_SEEN_LH */

    /* BR_TX_FRAG_COUNT */
  /*109*/  {/*baseAddr*/ 0xb0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*BR_TX_FRAG_COUNT */

    /* BR_RX_FRAG_COUNT */
  /*110*/  {/*baseAddr*/ 0xb8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*BR_RX_FRAG_COUNT */

    /* BR_TX_HOLD_COUNT */
  /*111*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*BR_TX_HOLD_COUNT */

    /* BR_RX_SMD_ERR_COUNT */
  /*112*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*BR_RX_SMD_ERR_COUNT */

    /* BR_RX_ASSY_ERR_COUNT */
  /*113*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*BR_RX_ASSY_ERR_COUNT */

    /* BR_RX_ASSY_OK_COUNT */
  /*114*/  {/*baseAddr*/ 0xd8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*BR_RX_ASSY_OK_COUNT */

    /* BR_RX_VERIFY_COUNT */
  /*115*/  {/*baseAddr*/ 0xe0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GOOD */
  /*116*/  {/*baseAddr*/ 0xe0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*BAD */

    /* BR_RX_RESP_COUNT */
  /*117*/  {/*baseAddr*/ 0xe4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GOOD */
  /*118*/  {/*baseAddr*/ 0xe4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*BAD */

    /* BR_TX_VERIFY_COUNT */
  /*119*/  {/*baseAddr*/ 0xe8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VERIFY */
  /*120*/  {/*baseAddr*/ 0xe8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*RESP */

    /* EMAC_COMMAND_CONFIG */
  /*121*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*EMAC_TX_ENA */
  /*122*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*EMAC_RX_ENA */
  /*123*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*EMAC_PAUSE_FWD */
  /*124*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*EMAC_PAUSE_IGNORE */
  /*125*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*EMAC_SW_RESET */
  /*126*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*EMAC_CNTL_FRAME_ENA */
  /*127*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*EMAC_PAUSE_MASK_P */
  /*128*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*EMAC_PAUSE_MASK_E */
  /*129*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*EMAC_PFC_MODE */
  /*130*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*EMAC_PAUSE_PFC_COMP */
  /*131*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*EMAC_TX_FLUSH */
  /*132*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*EMAC_TX_FIFO_RESET */
  /*133*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*EMAC_TX_PAUSE_DIS */
  /*134*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*EMAC_RX_PAUSE_DIS */

    /* EMAC_FRM_LENGTH */
  /*135*/  {/*baseAddr*/ 0x114,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FRM_LENGTH */
  /*136*/  {/*baseAddr*/ 0x114,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*TX_MTU */

    /* EMAC_TX_FIFO_SECTIONS */
  /*137*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*TX_SECTION_FULL */
  /*138*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 5 }, /*TX_SECTION_EMPTY */

    /* EMAC_CL01_PAUSE_QUANTA */
  /*139*/  {/*baseAddr*/ 0x154,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL0_PAUSE_QUANTA */

    /* EMAC_CL01_QUANTA_THRESH */
  /*140*/  {/*baseAddr*/ 0x164,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL0_QUANTA_THRESH */

    /* EMAC_RX_PAUSE_STATUS */
  /*141*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*PAUSESTATUS */

    /* TSU_CONTROL */
  /*142*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TSU_ENA */
  /*143*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MODULO_SEL */
  /*144*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 12 }, /*BLOCK_TIME */
};

