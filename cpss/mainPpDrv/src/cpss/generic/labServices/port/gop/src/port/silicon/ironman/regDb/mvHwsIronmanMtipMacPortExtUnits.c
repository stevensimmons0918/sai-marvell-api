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
* @file mvHwsIronmanMtipMacPortExtUnits.c
*
* @brief IronmanL MTI MAC Port EXT register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC ironmanMtipMacPortExtUnitsDb[] = {
    /* Port Control */
  /*0*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TX_LOC_FAULT */
  /*1*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*TX_REM_FAULT */
  /*2*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*TX_LI_FAULT */
  /*3*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TOD_SELECT */
  /*4*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*RX_PAUSE_CONTROL */
  /*5*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*RX_PAUSE_OW_VAL */
  /*6*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PAUSE_802_3_REFLECT */
  /*7*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*LOOP_ENA */
  /*8*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FF_TX_CRC */
  /*9*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*MAC_TIME_CAPTURE_EN */
  /*10*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 6 }, /*LED_PORT_NUM */
  /*11*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*LED_PORT_EN */
  /*12*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*LOOP_RX_BLOCK_OUT */
  /*13*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*LOOP_TX_RDY_OUT */
  /*14*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 2 }, /*BACK_PRESSURE_MODE_SELECT */
  /*15*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*TX_LOC_FAULT_SEL */
  /*16*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*TX_REM_FAULT_SEL */

    /* Port Status */
  /*17*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LINK_OK */
  /*18*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RX_TRAFFIC_IND */
  /*19*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*TX_TRAFFIC_IND */
  /*20*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*MAC_TX_EMPTY */
  /*21*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*MAC_TX_ISIDLE */
  /*22*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PFC_MODE */
  /*23*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FF_TX_SEPTY */
  /*24*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FF_RX_EMPTY */
  /*25*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FF_RX_DSAV */
  /*26*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*LINK_OK_CLEAN */
  /*27*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LINK_STATUS_CLEAN */
  /*28*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LPCS_LINK_STATUS_CLEAN */
  /*29*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 8 }, /*MAC_RES_SPEED */
  /*30*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*TSU_RX_READY */
  /*31*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*TSU_TX_READY */
  /*32*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*FF_TX_RDY */
  /*33*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*FF_RX_RDY */

    /* Port Interrupt Cause */
  /*34*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PORT_INTERRUPT_CAUSE_INT_SUM */
  /*35*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*ILLEGAL_ADDRESS_ACCESS */
  /*36*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LINK_OK_CHANGE */
  /*37*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*MAC_LOC_FAULT */
  /*38*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*MAC_REM_FAULT */
  /*39*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*MAC_LI_FAULT */
  /*40*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*MAC_TX_UNDERFLOW */
  /*41*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*EMAC_TX_OVR_ERR */
  /*42*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PMAC_TX_OVR_ERR */
  /*43*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*EMAC_RX_OVERRUN */
  /*44*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*PMAC_RX_OVERRUN */
  /*45*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*FFE_RX_RDY */
  /*46*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*FFP_RX_RDY */
  /*47*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LINK_OK_CLEAN_CHANGE */
  /*48*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*FFP_RX_NOT_RDY */
  /*49*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*FFE_RX_NOT_RDY */
  /*50*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*FFE_RDY2LOAD_BRG_FIFO_OVERRUN */
  /*51*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*FFP_RDY2LOAD_BRG_FIFO_OVERRUN */
  /*52*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*TSU_RX_DFF_ERR */
  /*53*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*TSU_RX_AM_ERR */
  /*54*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*TSU_TX_SYNC_ERR */
  /*55*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*TSQ_TS_FIFO_WR */
  /*56*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*TSQ_TS_FIFO_OVERWRITE */

    /* Port Interrupt Mask */
  /*57*/  {/*baseAddr*/ 0x110,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 22 }, /*INTERRUPT_MASK */

    /* Port Pause and Error Status */
  /*58*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*PAUSE_ON */
  /*59*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*FF_RX_ERR_STAT */

    /* Port Peer Delay */
  /*60*/  {/*baseAddr*/ 0x114,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 30 }, /*PEER_DELAY */
  /*61*/  {/*baseAddr*/ 0x114,  /*offsetFormula*/ 0x0, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*PEER_DELAY_VALID */

    /* Port Xoff Status */
  /*62*/  {/*baseAddr*/ 0x118,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*XOFF_STATUS */

    /* Port Pause Override */
  /*63*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*PAUSE_OVERRIDE_CTRL */
  /*64*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*PAUSE_OVERRIDE_VAL */

    /* Port Xoff Override */
  /*65*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*XOFF_OVERRIDE_CTRL */
  /*66*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*XOFF_OVERRIDE_VAL */

    /* Port Control1 */
  /*67*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*PORT_RES_SPEED */
  /*68*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FORCE_LINK_OK_EN */
  /*69*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*FORCE_LINK_OK_DIS */
  /*70*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*FORCE_TS_IF_PCH_EN */
  /*71*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*FORCE_TS_IF_PCH_DIS */
  /*72*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*FORCE_TS_IF_CF_EN */
  /*73*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*FORCE_TS_IF_CF_DIS */
  /*74*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*FORCE_TS_IF_TSTF_EN */
  /*75*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*FORCE_TS_IF_TSTF_DIS */
  /*76*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*LINK_UP_MUX */
  /*77*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*FORCE_TS_IF_2STEP_EN */
  /*78*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*FORCE_TS_IF_2STEP_DIS */

    /* Port EMAC USX PCH Last Timestamp */
  /*79*/  {/*baseAddr*/ 0x1d8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*EMAC_USX_PCH_LAST_TIMESTAMP */

    /* Port TSX Control */
  /*80*/  {/*baseAddr*/ 0x150,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*C_MODULO_RX */

    /* Port TSX Control4 */
  /*81*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*C_MODULO_RX_OW */
  /*82*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*C_MODULO_TX_OW */
  /*83*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*C_RX_MODE_OW */
  /*84*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*C_MII_CW_DLY_OW */
  /*85*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*C_MII_MK_DLY_OW */
  /*86*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*C_DESKEW_OW */
  /*87*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*C_BLOCKTIME_OW */
  /*88*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*C_BLOCKTIME_DEC_OW */
  /*89*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*C_MARKERTIME_OW */
  /*90*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*C_MARKERTIME_DEC_OW */
  /*91*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*C_BLKS_PER_CLK_OW */
  /*92*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*C_TX_MODE_OW */
  /*93*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*C_MII_TX_MK_CYC_DLY_OW */
  /*94*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*C_MII_TX_CW_CYC_DLY_OW */
  /*95*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*C_TSU_TX_SD_PERIOD_OW */
  /*96*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 15 }, /*C_TSU_TX_SD_PERIOD */

    /* Port TSX Control3 */
  /*97*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*C_RX_MODE */
  /*98*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 3 }, /*C_MII_CW_DLY */
  /*99*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 3 }, /*C_MII_MK_DLY */
  /*100*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*C_BLKS_PER_CLK */
  /*101*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 10 }, /*C_DESKEW */
  /*102*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 5 }, /*C_MII_TX_MK_CYC_DLY */
  /*103*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 5 }, /*C_MII_TX_CW_CYC_DLY */

    /* Port TSX Control2 */
  /*104*/  {/*baseAddr*/ 0x158,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*C_BLOCKTIME */
  /*105*/  {/*baseAddr*/ 0x158,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 9 }, /*C_BLOCKTIME_DEC */
  /*106*/  {/*baseAddr*/ 0x158,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 5 }, /*C_MARKERTIME */
  /*107*/  {/*baseAddr*/ 0x158,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 9 }, /*C_MARKERTIME_DEC */
  /*108*/  {/*baseAddr*/ 0x158,  /*offsetFormula*/ 0x0, /*fieldStart*/ 28, /*fieldLen*/ 3 }, /*C_TX_MODE */

    /* Port TSX Control1 */
  /*109*/  {/*baseAddr*/ 0x154,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*C_MODULO_TX */

    /* Port PMAC Control */
  /*110*/  {/*baseAddr*/ 0x180,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PREEMPTION_LOOP_ENA */
  /*111*/  {/*baseAddr*/ 0x180,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FFP_TX_CRC */
  /*112*/  {/*baseAddr*/ 0x180,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 2 }, /*PREEMPTION_BACK_PRESSURE_MODE_SELECT */
  /*113*/  {/*baseAddr*/ 0x180,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*PREEMPTION_LOOP_RX_BLOCK_OUT */
  /*114*/  {/*baseAddr*/ 0x180,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*PREEMPTION_LOOP_TX_RDY_OUT */
  /*115*/  {/*baseAddr*/ 0x180,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 2 }, /*LED_INDICATION_CTRL */
  /*116*/  {/*baseAddr*/ 0x180,  /*offsetFormula*/ 0x0, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*TX_HOLD_REQ */

    /* Port PMAC Status */
  /*117*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PREEMPTION_ENABLED */
  /*118*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*EXPRESS_MAC_ACTIVE */
  /*119*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PREEMPTION_MAC_ACTIVE */
  /*120*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PREEMPTION_MAC_ACTIVE_OR_FRAME_PREEMPTED */
  /*121*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PREEMPTION_MAC_TX_EMPTY */
  /*122*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FFP_TX_SEPTY */
  /*123*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FFP_RX_EMPTY */
  /*124*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FFP_RX_DSAV */
  /*125*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FF_PMAC_TX_RDY */
  /*126*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*FF_PMAC_RX_RDY */

    /* Port Control2 */
  /*127*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC91_ENA */

    /* Port MAC Clock and Reset Control */
  /*128*/  {/*baseAddr*/ 0x18c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MAC_RESET_ */
  /*129*/  {/*baseAddr*/ 0x18c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MAC_CLK_EN */

    /* External Port Control */
  /*130*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MAC_MUX_CTRL */

    /* Port GPIO Status */
  /*131*/  {/*baseAddr*/ 0x190,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GPIO_STATUS */

    /* Port GPIO Control */
  /*132*/  {/*baseAddr*/ 0x194,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GPIO_CONTROL */

    /* Port PMAC Error Status */
  /*133*/  {/*baseAddr*/ 0x188,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*FFP_RX_ERR_STAT */

    /* Per Port Last Violation */
  /*134*/  {/*baseAddr*/ 0x134,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*PER_PORT_LAST_VIOLATION */

    /* Port FC and PFC Control */
  /*135*/  {/*baseAddr*/ 0x198,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*XOFF_ONLY_FROM_EXP */
  /*136*/  {/*baseAddr*/ 0x198,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MAC_XOFF_GEN_SAMP */
  /*137*/  {/*baseAddr*/ 0x198,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*DUPLICATE_XOFF_E_TO_P */
  /*138*/  {/*baseAddr*/ 0x198,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FC_MODE_MSK_OUT */
  /*139*/  {/*baseAddr*/ 0x198,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 2 }, /*TX_HOLD_SRC */
  /*140*/  {/*baseAddr*/ 0x198,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PMAC_STOP_BY_PAUSE_802_3 */
  /*141*/  {/*baseAddr*/ 0x198,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PMAC_STOP_VAL */

    /* TSQ Timestamp */
  /*142*/  {/*baseAddr*/ 0x1b0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TSQ_TIMESTAMP */

    /* TSQ Signature */
  /*143*/  {/*baseAddr*/ 0x1b4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 14 }, /*TSQ_SIGNATURE_VALUE */
  /*144*/  {/*baseAddr*/ 0x1b4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*TSQ_REQUEST_SOURCE_PORT */
  /*145*/  {/*baseAddr*/ 0x1b4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*TSQ_VALID */

    /* TSQ FIFO Status */
  /*146*/  {/*baseAddr*/ 0x1b8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*TSQ_FIFO_FILL_LEVEL */
  /*147*/  {/*baseAddr*/ 0x1b8,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*TSQ_FIFO_FULL */

    /* TSQ Clear Control */
  /*148*/  {/*baseAddr*/ 0x1bc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TSQ_CLEAR_CONTROL */

    /* Port EMAC USX PCH RX Control */
  /*149*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*EMAC_PCH_RX_PROCESS */
  /*150*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*EMAC_EXTTYPE_BR_EXPRESS_ONLY */
  /*151*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*EMAC_COUNTER_WRAPAROUND_EN */
  /*152*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*EMAC_COUNTER_CLEAR_ON_READ_EN */
  /*153*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*EMAC_EXTTYPE_RSVD_ACCEPT */
  /*154*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*EMAC_PCH_PTP_USE_TS */
  /*155*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*EMAC_PCH_BR_USE_TS */
  /*156*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*EMAC_NO_PCH_ACCEPT */
  /*157*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*EMAC_EXTTYPE_IGNORE_ACCEPT */
  /*158*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*EMAC_EXTTYPE_PTP_ACCEPT */
  /*159*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*EMAC_EXTTYPE_BR_ACCEPT */
  /*160*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*EMAC_PCH_RX_TS_CONVERT */

    /* Port EMAC USX PCH Error By App Counter */
  /*161*/  {/*baseAddr*/ 0x1c8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*EMAC_ERROR_BY_APP_COUNTER */

    /* Port PMAC USX PCH Bad Subport Counter */
  /*162*/  {/*baseAddr*/ 0x1e8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*PMAC_BAD_SUBPORT_COUNTER */

    /* Port PMAC USX PCH Error By App Counter */
  /*163*/  {/*baseAddr*/ 0x1ec,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*PMAC_ERROR_BY_APP_COUNTER */

    /* Port EMAC USX PCH Bad CRC Counter */
  /*164*/  {/*baseAddr*/ 0x1c0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*EMAC_BAD_CRC_COUNTER */

    /* Port EMAC USX PCH Bad Subport Counter */
  /*165*/  {/*baseAddr*/ 0x1c4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*EMAC_BAD_SUBPORT_COUNTER */

    /* Port PMAC USX PCH Good Reserved Counter */
  /*166*/  {/*baseAddr*/ 0x1f8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*PMAC_GOOD_RESERVED_COUNTER */

    /* Port PMAC USX PCH RX Control */
  /*167*/  {/*baseAddr*/ 0x19c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PMAC_PCH_RX_PROCESS */
  /*168*/  {/*baseAddr*/ 0x19c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PMAC_EXTTYPE_BR_EXPRESS_ONLY */
  /*169*/  {/*baseAddr*/ 0x19c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PMAC_COUNTER_WRAPAROUND_EN */
  /*170*/  {/*baseAddr*/ 0x19c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PMAC_COUNTER_CLEAR_ON_READ_EN */
  /*171*/  {/*baseAddr*/ 0x19c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PMAC_EXTTYPE_RSVD_ACCEPT */
  /*172*/  {/*baseAddr*/ 0x19c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PMAC_PCH_PTP_USE_TS */
  /*173*/  {/*baseAddr*/ 0x19c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PMAC_PCH_BR_USE_TS */
  /*174*/  {/*baseAddr*/ 0x19c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PMAC_NO_PCH_ACCEPT */
  /*175*/  {/*baseAddr*/ 0x19c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PMAC_EXTTYPE_IGNORE_ACCEPT */
  /*176*/  {/*baseAddr*/ 0x19c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*PMAC_EXTTYPE_PTP_ACCEPT */
  /*177*/  {/*baseAddr*/ 0x19c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*PMAC_EXTTYPE_BR_ACCEPT */
  /*178*/  {/*baseAddr*/ 0x19c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*PMAC_PCH_RX_TS_CONVERT */

    /* Port PMAC USX PCH TX Control 2 */
  /*179*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PMAC_NEXT_SELF_CLR */
  /*180*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PMAC_NEXT_PCH_NOTS_BAD_CRC */
  /*181*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PMAC_NEXT_PCH_TS_BAD_CRC */
  /*182*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PMAC_NEXT_PCH_NOTS_BAD_SUBPORT */
  /*183*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PMAC_NEXT_PCH_TS_BAD_SUBPORT */
  /*184*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PMAC_NEXT_PCH_IDLE */
  /*185*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PMAC_NEXT_PCH_RSVD */
  /*186*/  {/*baseAddr*/ 0x1a4,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PMAC_NEXT_PCH_EXT_RSVD */

    /* Port PMAC USX PCH Signature */
  /*187*/  {/*baseAddr*/ 0x1e0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*PMAC_ST_CNT_SIGNATURE */

    /* Port EMAC USX PCH Interrupt Mask */
  /*188*/  {/*baseAddr*/ 0x148,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 12 }, /*PORT_EMAC_USX_PCH_INTERRUPT_MASK */

    /* Port EMAC USX PCH Signature */
  /*189*/  {/*baseAddr*/ 0x14c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*EMAC_ST_CNT_SIGNATURE */

    /* Port PMAC USX PCH TX Control 1 */
  /*190*/  {/*baseAddr*/ 0x1a0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PMAC_PCH_NOTS_BR */
  /*191*/  {/*baseAddr*/ 0x1a0,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PMAC_PCH_TS_BR */
  /*192*/  {/*baseAddr*/ 0x1a0,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 16 }, /*PMAC_PCH_SIGNATURE */
  /*193*/  {/*baseAddr*/ 0x1a0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*PMAC_SIGNATURE_LOAD */
  /*194*/  {/*baseAddr*/ 0x1a0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PMAC_SIGNATURE_MODE */
  /*195*/  {/*baseAddr*/ 0x1a0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*PMAC_PCH_USE_ID_TRIG */
  /*196*/  {/*baseAddr*/ 0x1a0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*PMAC_PCH_TX_EN */
  /*197*/  {/*baseAddr*/ 0x1a0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*PMAC_PCH_STD_PREAM_NOTS */
  /*198*/  {/*baseAddr*/ 0x1a0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 4 }, /*PMAC_SUBPORT_ID */

    /* Port PMAC USX PCH Interrupt Cause */
  /*199*/  {/*baseAddr*/ 0x1a8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PORT_PMAC_USX_PCH_INTERRUPT_CAUSE_INT_SUM */
  /*200*/  {/*baseAddr*/ 0x1a8,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PMAC_NOPCH_DETECTED */
  /*201*/  {/*baseAddr*/ 0x1a8,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PMAC_BAD_IDLE_DETECTED */
  /*202*/  {/*baseAddr*/ 0x1a8,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PMAC_GOOD_IDLE_DETECTED */
  /*203*/  {/*baseAddr*/ 0x1a8,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PMAC_BAD_RSVD_DETECTED */
  /*204*/  {/*baseAddr*/ 0x1a8,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PMAC_GOOD_RSVD_DETECTED */
  /*205*/  {/*baseAddr*/ 0x1a8,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PMAC_PCH_IGNORE_DETECTED */
  /*206*/  {/*baseAddr*/ 0x1a8,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PMAC_PCH_PTP_DETECTED */
  /*207*/  {/*baseAddr*/ 0x1a8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PMAC_PCH_BR_PTP_DETECTED */
  /*208*/  {/*baseAddr*/ 0x1a8,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*PMAC_PCH_EXT_RSVD_DETECTED */
  /*209*/  {/*baseAddr*/ 0x1a8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*PMAC_PCH_INVALID_VR_DETECTED */
  /*210*/  {/*baseAddr*/ 0x1a8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*PMAC_PCH_TS_GENERATED */
  /*211*/  {/*baseAddr*/ 0x1a8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*PMAC_PCH_NOTS_GENERATED */

    /* Port PMAC USX PCH Interrupt Mask */
  /*212*/  {/*baseAddr*/ 0x1ac,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 12 }, /*PORT_PMAC_USX_PCH_INTERRUPT_MASK */

    /* Port PMAC USX PCH Bad CRC Counter */
  /*213*/  {/*baseAddr*/ 0x1e4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*PMAC_BAD_CRC_COUNTER */

    /* Port PMAC USX PCH Good Idle Counter */
  /*214*/  {/*baseAddr*/ 0x1f4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*PMAC_GOOD_IDLE_COUNTER */

    /* Port Summary Interrupt Mask */
  /*215*/  {/*baseAddr*/ 0x16c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 3 }, /*PORT_SUMMARY_INTERRUPT_MASK */

    /* Port EMAC USX PCH TX Control 1 */
  /*216*/  {/*baseAddr*/ 0x13c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*EMAC_PCH_NOTS_BR */
  /*217*/  {/*baseAddr*/ 0x13c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*EMAC_PCH_TS_BR */
  /*218*/  {/*baseAddr*/ 0x13c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 16 }, /*EMAC_PCH_SIGNATURE */
  /*219*/  {/*baseAddr*/ 0x13c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*EMAC_SIGNATURE_LOAD */
  /*220*/  {/*baseAddr*/ 0x13c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*EMAC_SIGNATURE_MODE */
  /*221*/  {/*baseAddr*/ 0x13c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*EMAC_PCH_USE_ID_TRIG */
  /*222*/  {/*baseAddr*/ 0x13c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*EMAC_PCH_TX_EN */
  /*223*/  {/*baseAddr*/ 0x13c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*EMAC_PCH_STD_PREAM_NOTS */
  /*224*/  {/*baseAddr*/ 0x13c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 4 }, /*EMAC_SUBPORT_ID */

    /* Port EMAC USX PCH Good Reserved Counter */
  /*225*/  {/*baseAddr*/ 0x1d4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*EMAC_GOOD_RESERVED_COUNTER */

    /* Port EMAC USX PCH Invalid VR Counter */
  /*226*/  {/*baseAddr*/ 0x1cc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*EMAC_INVALID_VR_COUNTER */

    /* Port EMAC USX PCH Good Idle Counter */
  /*227*/  {/*baseAddr*/ 0x1d0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*EMAC_GOOD_IDLE_COUNTER */

    /* Port PMAC USX PCH Total PCH Errors Counter */
  /*228*/  {/*baseAddr*/ 0x164,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*PMAC_TOTAL_PCH_ERRORS_COUNTER */

    /* Port EMAC USX PCH Total PCH Errors Counter */
  /*229*/  {/*baseAddr*/ 0x1dc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*EMAC_TOTAL_PCH_ERRORS_COUNTER */

    /* Port PMAC USX PCH Last Timestamp */
  /*230*/  {/*baseAddr*/ 0x1fc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*PMAC_USX_PCH_LAST_TIMESTAMP */

    /* Port Summary Interrupt Cause */
  /*231*/  {/*baseAddr*/ 0x168,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MAC_PORT_EXT_UNITS_INTERRUPTS_SUMMARY */
  /*232*/  {/*baseAddr*/ 0x168,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PORT_INTERRUPT */
  /*233*/  {/*baseAddr*/ 0x168,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PORT_EMAC_USX_PCH_INTERRUPT */
  /*234*/  {/*baseAddr*/ 0x168,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PORT_PMAC_USX_PCH_INTERRUPT */

    /* Port EMAC USX PCH Interrupt Cause */
  /*235*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*EMAC_PORT_USX_PCH_INTERRUPT_CAUSE_INT_SUM */
  /*236*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*EMAC_NOPCH_DETECTED */
  /*237*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*EMAC_BAD_IDLE_DETECTED */
  /*238*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*EMAC_GOOD_IDLE_DETECTED */
  /*239*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*EMAC_BAD_RSVD_DETECTED */
  /*240*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*EMAC_GOOD_RSVD_DETECTED */
  /*241*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*EMAC_PCH_IGNORE_DETECTED */
  /*242*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*EMAC_PCH_PTP_DETECTED */
  /*243*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*EMAC_PCH_BR_PTP_DETECTED */
  /*244*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*EMAC_PCH_EXT_RSVD_DETECTED */
  /*245*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*EMAC_PCH_INVALID_VR_DETECTED */
  /*246*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*EMAC_PCH_TS_GENERATED */
  /*247*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*EMAC_PCH_NOTS_GENERATED */

    /* Port EMAC USX PCH TX Control 2 */
  /*248*/  {/*baseAddr*/ 0x140,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*EMAC_NEXT_SELF_CLR */
  /*249*/  {/*baseAddr*/ 0x140,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*EMAC_NEXT_PCH_NOTS_BAD_CRC */
  /*250*/  {/*baseAddr*/ 0x140,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*EMAC_NEXT_PCH_TS_BAD_CRC */
  /*251*/  {/*baseAddr*/ 0x140,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*EMAC_NEXT_PCH_NOTS_BAD_SUBPORT */
  /*252*/  {/*baseAddr*/ 0x140,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*EMAC_NEXT_PCH_TS_BAD_SUBPORT */
  /*253*/  {/*baseAddr*/ 0x140,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*EMAC_NEXT_PCH_IDLE */
  /*254*/  {/*baseAddr*/ 0x140,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*EMAC_NEXT_PCH_RSVD */
  /*255*/  {/*baseAddr*/ 0x140,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*EMAC_NEXT_PCH_EXT_RSVD */

    /* Port PMAC USX PCH Invalid VR Counter */
  /*256*/  {/*baseAddr*/ 0x1f0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*PMAC_INVALID_VR_COUNTER */
};

