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
* @file mvHwsIronmanMtip10GPcsLpcsUnits.h
*
* @brief IronmanL port interface header file
*
* @version   1
********************************************************************************
*/
#ifndef __mvHwsIronmanMtip10GPcsLpcsUnits_H
#define __mvHwsIronmanMtip10GPcsLpcsUnits_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  /*0*/  IRONMAN_LPCS_CONTROL_SPEED_6_E,
  /*1*/  IRONMAN_LPCS_CONTROL_DUPLEX_E,
  /*2*/  IRONMAN_LPCS_CONTROL_ANRESTART_E,
  /*3*/  IRONMAN_LPCS_CONTROL_ISOLATE_E,
  /*4*/  IRONMAN_LPCS_CONTROL_POWERDOWN_E,
  /*5*/  IRONMAN_LPCS_CONTROL_ANENABLE_E,
  /*6*/  IRONMAN_LPCS_CONTROL_SPEED_13_E,
  /*7*/  IRONMAN_LPCS_CONTROL_LOOPBACK_E,
  /*8*/  IRONMAN_LPCS_CONTROL_RESET_E,
  /*9*/  IRONMAN_LPCS_LINK_TIMER_1_TIMER20_16_E,
  /*10*/  IRONMAN_LPCS_DECODE_ERRORS_DECODEERRORS_E,
  /*11*/  IRONMAN_LPCS_SCRATCH_SCRATCH_E,
  /*12*/  IRONMAN_LPCS_LINK_TIMER_0_TIMER0_E,
  /*13*/  IRONMAN_LPCS_LINK_TIMER_0_TIMER15_1_E,
  /*14*/  IRONMAN_LPCS_TX_IPG_LENGTH_TXIPGLENGTH_E,
  /*15*/  IRONMAN_LPCS_PHY_ID_0_PHYID_E,
  /*16*/  IRONMAN_LPCS_PHY_ID_1_PHYID_E,
  /*17*/  IRONMAN_LPCS_USXGMII_REP_USXGMIIREP_E,
  /*18*/  IRONMAN_LPCS_USXGMII_REP_USXGMII2_5_E,
  /*19*/  IRONMAN_LPCS_NP_TX_DATA_E,
  /*20*/  IRONMAN_LPCS_NP_TX_TOGGLE_E,
  /*21*/  IRONMAN_LPCS_NP_TX_ACK2_E,
  /*22*/  IRONMAN_LPCS_NP_TX_MP_E,
  /*23*/  IRONMAN_LPCS_NP_TX_ACK_E,
  /*24*/  IRONMAN_LPCS_NP_TX_NP_E,
  /*25*/  IRONMAN_LPCS_LP_NP_RX_DATA_E,
  /*26*/  IRONMAN_LPCS_LP_NP_RX_TOGGLE_E,
  /*27*/  IRONMAN_LPCS_LP_NP_RX_ACK2_E,
  /*28*/  IRONMAN_LPCS_LP_NP_RX_MP_E,
  /*29*/  IRONMAN_LPCS_LP_NP_RX_ACK_E,
  /*30*/  IRONMAN_LPCS_LP_NP_RX_NP_E,
  /*31*/  IRONMAN_LPCS_IF_MODE_SGMII_ENA_E,
  /*32*/  IRONMAN_LPCS_IF_MODE_USE_SGMII_AN_E,
  /*33*/  IRONMAN_LPCS_IF_MODE_SGMII_SPEED_E,
  /*34*/  IRONMAN_LPCS_IF_MODE_SGMII_DUPLEX_E,
  /*35*/  IRONMAN_LPCS_IF_MODE_IFMODE_RSV5_E,
  /*36*/  IRONMAN_LPCS_IF_MODE_IFMODE_TX_PREAMBLE_SYNC_E,
  /*37*/  IRONMAN_LPCS_IF_MODE_IFMODE_RX_PREAMBLE_SYNC_E,
  /*38*/  IRONMAN_LPCS_IF_MODE_IFMODE_MODE_XGMII_BASEX_E,
  /*39*/  IRONMAN_LPCS_IF_MODE_IFMODE_SEQ_ENA_E,
  /*40*/  IRONMAN_LPCS_IF_MODE_RX_BR_DIS_E,
  /*41*/  IRONMAN_LPCS_DEV_ABILITY_ABILITY_RSV05_E,
  /*42*/  IRONMAN_LPCS_DEV_ABILITY_FD_E,
  /*43*/  IRONMAN_LPCS_DEV_ABILITY_HD_E,
  /*44*/  IRONMAN_LPCS_DEV_ABILITY_PS1_E,
  /*45*/  IRONMAN_LPCS_DEV_ABILITY_PS2_E,
  /*46*/  IRONMAN_LPCS_DEV_ABILITY_ABILITY_RSV9_E,
  /*47*/  IRONMAN_LPCS_DEV_ABILITY_RF1_E,
  /*48*/  IRONMAN_LPCS_DEV_ABILITY_RF2_E,
  /*49*/  IRONMAN_LPCS_DEV_ABILITY_ACK_E,
  /*50*/  IRONMAN_LPCS_DEV_ABILITY_NP_E,
  /*51*/  IRONMAN_LPCS_REV_REVISION_E,
  /*52*/  IRONMAN_LPCS_STATUS_EXTDCAPABILITY_E,
  /*53*/  IRONMAN_LPCS_STATUS_LINKSTATUS_E,
  /*54*/  IRONMAN_LPCS_STATUS_ANEGABILITY_E,
  /*55*/  IRONMAN_LPCS_STATUS_ANEGCOMPLETE_E,
  /*56*/  IRONMAN_LPCS_PARTNER_ABILITY_PABILITY_RSV05_E,
  /*57*/  IRONMAN_LPCS_PARTNER_ABILITY_FD_E,
  /*58*/  IRONMAN_LPCS_PARTNER_ABILITY_HD_E,
  /*59*/  IRONMAN_LPCS_PARTNER_ABILITY_PS1_E,
  /*60*/  IRONMAN_LPCS_PARTNER_ABILITY_PS2_E,
  /*61*/  IRONMAN_LPCS_PARTNER_ABILITY_PABILITY_RSV9_E,
  /*62*/  IRONMAN_LPCS_PARTNER_ABILITY_PABILITY_RSV10_E,
  /*63*/  IRONMAN_LPCS_PARTNER_ABILITY_RF1_E,
  /*64*/  IRONMAN_LPCS_PARTNER_ABILITY_RF2_E,
  /*65*/  IRONMAN_LPCS_PARTNER_ABILITY_ACK_E,
  /*66*/  IRONMAN_LPCS_PARTNER_ABILITY_NP_E,
  /*67*/  IRONMAN_LPCS_AN_EXPANSION_PAGERECEIVED_E,
  /*68*/  IRONMAN_LPCS_AN_EXPANSION_NEXTPAGEABLE_E,
    IRONMAN_LPCS_UNITS_REGISTER_LAST_E /* should be last */
} MV_HWS_IRONMAN_LPCS_UNITS_E;

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsIronmanMtip10GPcsLpcsUnits_H */

