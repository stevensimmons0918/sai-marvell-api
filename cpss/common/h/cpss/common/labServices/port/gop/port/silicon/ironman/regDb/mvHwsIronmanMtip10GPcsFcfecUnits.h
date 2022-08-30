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
* @file mvHwsIronmanMtip10GPcsFcfecUnits.h
*
* @brief IronmanL port interface header file
*
* @version   1
********************************************************************************
*/
#ifndef __mvHwsIronmanMtip10GPcsFcfecUnits_H
#define __mvHwsIronmanMtip10GPcsFcfecUnits_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  /*0*/  IRONMAN_PCS_FCFEC_VL_INTVL_MARKER_COUNTER_E,
  /*1*/  IRONMAN_PCS_FCFEC_VL_INTVL_HI_MARKER_COUNTERHI_E,
  /*2*/  IRONMAN_PCS_FCFEC_SDCONFIG_TX_THRESHOLD_E,
  /*3*/  IRONMAN_PCS_FCFEC_SDCONFIG_TOGGLE_EN_E,
  /*4*/  IRONMAN_PCS_FCFEC_FC_FEC_ABILITY_FC_ABILITY_E,
  /*5*/  IRONMAN_PCS_FCFEC_FC_FEC_STATUS_FC_LOCKED_E,
  /*6*/  IRONMAN_PCS_FCFEC_CONTROL_RESET_E,
  /*7*/  IRONMAN_PCS_FCFEC_ALIGN_MATCH_ERR_ALIGNMATCHERR_E,
  /*8*/  IRONMAN_PCS_FCFEC_VL0_0_M0_E,
  /*9*/  IRONMAN_PCS_FCFEC_VL0_0_M1_E,
  /*10*/  IRONMAN_PCS_FCFEC_VL3_0_M0_E,
  /*11*/  IRONMAN_PCS_FCFEC_VL3_0_M1_E,
  /*12*/  IRONMAN_PCS_FCFEC_FC_FEC_COUNTER_HI_COUNTER_E,
  /*13*/  IRONMAN_PCS_FCFEC_VL2_0_M0_E,
  /*14*/  IRONMAN_PCS_FCFEC_VL2_0_M1_E,
  /*15*/  IRONMAN_PCS_FCFEC_FC_FEC_CCW_LO_COUNTER_E,
  /*16*/  IRONMAN_PCS_FCFEC_STATUS_LINK_STATUS_REALTIME_E,
  /*17*/  IRONMAN_PCS_FCFEC_STATUS_LINK_STATUS_E,
  /*18*/  IRONMAN_PCS_FCFEC_STATUS_SD_BIT_SLIP_E,
  /*19*/  IRONMAN_PCS_FCFEC_PORTS_ENA_NUMPORTS_E,
  /*20*/  IRONMAN_PCS_FCFEC_VL0_BYTE3_VL0BYTE3_E,
  /*21*/  IRONMAN_PCS_FCFEC_VL1_0_M0_E,
  /*22*/  IRONMAN_PCS_FCFEC_VL1_0_M1_E,
  /*23*/  IRONMAN_PCS_FCFEC_VL0_1_M2_E,
  /*24*/  IRONMAN_PCS_FCFEC_VL0_1_PAD_E,
  /*25*/  IRONMAN_PCS_FCFEC_FC_FEC_NCCW_LO_COUNTER_E,
  /*26*/  IRONMAN_PCS_FCFEC_VL2_1_M2_E,
  /*27*/  IRONMAN_PCS_FCFEC_VL2_1_PAD_E,
  /*28*/  IRONMAN_PCS_FCFEC_VL3_1_M2_E,
  /*29*/  IRONMAN_PCS_FCFEC_VL3_1_PAD_E,
  /*30*/  IRONMAN_PCS_FCFEC_VL1_1_M2_E,
  /*31*/  IRONMAN_PCS_FCFEC_VL1_1_PAD_E,
  /*32*/  IRONMAN_PCS_FCFEC_FC_FEC_CONTROL_FC_ENABLE_E,
  /*33*/  IRONMAN_PCS_FCFEC_FC_FEC_CONTROL_FC_ERROR_ENABLE_E,
    IRONMAN_PCS_FCFEC_REGISTER_LAST_E /* should be last */
} MV_HWS_IRONMAN_PCS_FCFEC_UNITS_E;

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsIronmanMtip10GPcsFcfecUnits_H */

