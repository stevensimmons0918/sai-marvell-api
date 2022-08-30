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
* @file mvHwsAc5xRsfecUnits.h
*
* @brief Phoenix port interface header file
*
* @version   1
********************************************************************************
*/
#ifndef __mvHwsAc5xRsfecUnits_H
#define __mvHwsAc5xRsfecUnits_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  /*0*/  PHOENIX_RSFEC_UNIT_RSFEC_CONTROL_P1_BYPASS_CORRECTION_E,
  /*1*/  PHOENIX_RSFEC_UNIT_RSFEC_CONTROL_P1_BYPASS_ERROR_INDICATION_E,
  /*2*/  PHOENIX_RSFEC_UNIT_RSFEC_CONTROL_P1_RSFEC_ENABLE_E,
  /*3*/  PHOENIX_RSFEC_UNIT_RSFEC_CONTROL_P1_KP_ENABLE_E,
  /*4*/  PHOENIX_RSFEC_UNIT_RSFEC_CONTROL_P1_TC_PAD_VALUE_E,
  /*5*/  PHOENIX_RSFEC_UNIT_RSFEC_CONTROL_P1_TC_PAD_ALTER_E,
  /*6*/  PHOENIX_RSFEC_UNIT_RSFEC_STATUS_P1_BYPASS_CORRECTION_ABILITY_E,
  /*7*/  PHOENIX_RSFEC_UNIT_RSFEC_STATUS_P1_BYPASS_INDICATION_ABILITY_E,
  /*8*/  PHOENIX_RSFEC_UNIT_RSFEC_STATUS_P1_HIGH_SER_E,
  /*9*/  PHOENIX_RSFEC_UNIT_RSFEC_STATUS_P1_AMPS_LOCK_E,
  /*10*/  PHOENIX_RSFEC_UNIT_RSFEC_STATUS_P1_FEC_ALIGN_STATUS_E,
  /*11*/  PHOENIX_RSFEC_UNIT_RSFEC_STATUS_P1_TMP_E,
  /*12*/  PHOENIX_RSFEC_UNIT_RSFEC_CCW_LO_P1_RSFEC_CCW_LO_E,
  /*13*/  PHOENIX_RSFEC_UNIT_RSFEC_CCW_HI_P1_RSFEC_CCW_HI_E,
  /*14*/  PHOENIX_RSFEC_UNIT_RSFEC_NCCW_LO_P1_RSFEC_NCCW_LO_E,
  /*15*/  PHOENIX_RSFEC_UNIT_RSFEC_NCCW_HI_P1_RSFEC_NCCW_HI_E,
  /*16*/  PHOENIX_RSFEC_UNIT_RSFEC_LANE_MAP_P1_RSFEC_LANE_MAP_E,
  /*17*/  PHOENIX_RSFEC_UNIT_RSFEC_SYMBLERR0_LO_SYMBLERR0_LO_E,
  /*18*/  PHOENIX_RSFEC_UNIT_RSFEC_SYMBLERR0_HI_SYMBLERR0_HI_E,
  /*19*/  PHOENIX_RSFEC_UNIT_RSFEC_SYMBLERR1_LO_SYMBLERR1_LO_E,
  /*20*/  PHOENIX_RSFEC_UNIT_RSFEC_SYMBLERR1_HI_SYMBLERR1_HI_E,
  /*21*/  PHOENIX_RSFEC_UNIT_RSFEC_SYMBLERR2_LO_SYMBLERR2_LO_E,
  /*22*/  PHOENIX_RSFEC_UNIT_RSFEC_SYMBLERR2_HI_SYMBLERR2_HI_E,
  /*23*/  PHOENIX_RSFEC_UNIT_RSFEC_SYMBLERR3_LO_SYMBLERR3_LO_E,
  /*24*/  PHOENIX_RSFEC_UNIT_RSFEC_SYMBLERR3_HI_SYMBLERR3_HI_E,
  /*25*/  PHOENIX_RSFEC_UNIT_RSFEC_VENDOR_INFO1_VENDOR_AMPS_LOCK_E,
  /*26*/  PHOENIX_RSFEC_UNIT_RSFEC_VENDOR_INFO1_VENDOR_ALIGN_STATUS_LH_E,
  /*27*/  PHOENIX_RSFEC_UNIT_RSFEC_VENDOR_INFO1_VENDOR_MARKER_CHECK_RESTART_E,
  /*28*/  PHOENIX_RSFEC_UNIT_RSFEC_VENDOR_INFO1_RX_DATAPATH_RESTART_E,
  /*29*/  PHOENIX_RSFEC_UNIT_RSFEC_VENDOR_INFO1_TX_DATAPATH_RESTART_E,
  /*30*/  PHOENIX_RSFEC_UNIT_RSFEC_VENDOR_INFO1_RX_DP_OVERFLOW_E,
  /*31*/  PHOENIX_RSFEC_UNIT_RSFEC_VENDOR_INFO1_TX_DP_OVERFLOW_E,
  /*32*/  PHOENIX_RSFEC_UNIT_RSFEC_VENDOR_INFO1_VENDOR_ALIGN_STATUS_LL_E,
  /*33*/  PHOENIX_RSFEC_UNIT_RSFEC_VENDOR_INFO2_VENDOR_AMPS_LOCK_LANES_E,
  /*34*/  PHOENIX_RSFEC_UNIT_RSFEC_VENDOR_INFO2_RS_DECODER_WRITE_ERR_E,
  /*35*/  PHOENIX_RSFEC_UNIT_RSFEC_VENDOR_REVISION_REVISION_E,
  /*36*/  PHOENIX_RSFEC_UNIT_RX_RSFEC_FIFO_LVL_CH1_F91_FIFO_THRESH_1_E,
  /*37*/  PHOENIX_RSFEC_UNIT_RX_RSFEC_FIFO_LVL_CH2_F91_FIFO_THRESH_2_E,
  /*38*/  PHOENIX_RSFEC_UNITNDOR_TX_TEST_PATTERN_TXE_PATTEN_E,
  /*39*/  PHOENIX_RSFEC_UNITNDOR_TX_TEST_PATTERN_TXE_PATOVER_E,
  /*40*/  PHOENIX_RSFEC_UNIT_BASE_R_FEC_CCW_LO_VL1_FEC_CW_LO_E,
  /*41*/  PHOENIX_RSFEC_UNIT_BASE_R_FEC_STATUS_FEC_LOCKED_E,
  /*42*/  PHOENIX_RSFEC_UNIT_RSFEC_NCCW_HI_P0_RSFEC_NCCW_HI_E,
  /*43*/  PHOENIX_RSFEC_UNIT_RSFEC_STATUS_P0_BYPASS_CORRECTION_ABILITY_E,
  /*44*/  PHOENIX_RSFEC_UNIT_RSFEC_STATUS_P0_BYPASS_INDICATION_ABILITY_E,
  /*45*/  PHOENIX_RSFEC_UNIT_RSFEC_STATUS_P0_HIGH_SER_E,
  /*46*/  PHOENIX_RSFEC_UNIT_RSFEC_STATUS_P0_AMPS_LOCK_E,
  /*47*/  PHOENIX_RSFEC_UNIT_RSFEC_STATUS_P0_FEC_ALIGN_STATUS_E,
  /*48*/  PHOENIX_RSFEC_UNIT_RSFEC_STATUS_P0_TMP_E,
  /*49*/  PHOENIX_RSFEC_UNIT_VENDOR_TX_TEST_SYMBOL_TXE_SYMBOL_E,
  /*50*/  PHOENIX_RSFEC_UNIT_BASE_R_FEC_NCCW_LO_VL1_NCERR_CNT_E,
  /*51*/  PHOENIX_RSFEC_UNIT_RX_RSFEC_FIFO_LVL_CH3_F91_FIFO_THRESH_3_E,
  /*52*/  PHOENIX_RSFEC_UNIT_VENDOR_CONTROL_FEC_ENA_E,
  /*53*/  PHOENIX_RSFEC_UNIT_VENDOR_CONTROL_FEC_1LANE0_E,
  /*54*/  PHOENIX_RSFEC_UNIT_VENDOR_CONTROL_FEC_1LANE2_E,
  /*55*/  PHOENIX_RSFEC_UNIT_VENDOR_CONTROL_AM16_CP_DIS_EN_E,
  /*56*/  PHOENIX_RSFEC_UNIT_VENDOR_CONTROL_FEC_EN_S_E,
  /*57*/  PHOENIX_RSFEC_UNIT_RX_RSFEC_FIFO_LVL_CH0_F91_FIFO_THRESH_0_E,
  /*58*/  PHOENIX_RSFEC_UNIT_RSFEC_CCW_HI_P0_RSFEC_CCW_HI_E,
  /*59*/  PHOENIX_RSFEC_UNIT_BASE_R_FEC_CCW_LO_FEC_CCW_LO_E,
  /*60*/  PHOENIX_RSFEC_UNIT_VENDOR_TX_TEST_CONTROL_TXE_RUN_E,
  /*61*/  PHOENIX_RSFEC_UNIT_BASE_R_FEC_CONTROL_FEC_ENABLE_E,
  /*62*/  PHOENIX_RSFEC_UNIT_BASE_R_FEC_CONTROL_FEC_ERROR_INDICATION_ENABLE_E,
  /*63*/  PHOENIX_RSFEC_UNIT_VENDOR_TX_TEST_KEY_TXE_KEY_E,
  /*64*/  PHOENIX_RSFEC_UNIT_BASE_R_FEC_ABILITY_FEC_ABILITY_E,
  /*65*/  PHOENIX_RSFEC_UNIT_BASE_R_FEC_ABILITY_FEC_ERROR_INDICATION_ABILITY_E,
  /*66*/  PHOENIX_RSFEC_UNIT_RSFEC_CCW_LO_P0_RSFEC_CCW_LO_E,
  /*67*/  PHOENIX_RSFEC_UNIT_RSFEC_LANE_MAP_P0_RSFEC_LANE_MAP_E,
  /*68*/  PHOENIX_RSFEC_UNIT_RSFEC_CONTROL_P0_BYPASS_CORRECTION_E,
  /*69*/  PHOENIX_RSFEC_UNIT_RSFEC_CONTROL_P0_BYPASS_ERROR_INDICATION_E,
  /*70*/  PHOENIX_RSFEC_UNIT_RSFEC_CONTROL_P0_RSFEC_ENABLE_E,
  /*71*/  PHOENIX_RSFEC_UNIT_RSFEC_CONTROL_P0_KP_ENABLE_E,
  /*72*/  PHOENIX_RSFEC_UNIT_RSFEC_CONTROL_P0_TC_PAD_VALUE_E,
  /*73*/  PHOENIX_RSFEC_UNIT_RSFEC_CONTROL_P0_TC_PAD_ALTER_E,
  /*74*/  PHOENIX_RSFEC_UNIT_RSFEC_NCCW_LO_P0_RSFEC_NCCW_LO_E,
  /*75*/  PHOENIX_RSFEC_UNIT_BASE_R_FEC_NCCW_LO_FEC_NCCW_LO_E,
    PHOENIX_RSFEC_REGISTER_LAST_E /* should be last */
} MV_HWS_PHOENIX_RSFEC_UNIT_FIELDS_E;

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsAc5xRsfecUnits_H */

