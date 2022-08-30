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
* @file mvHwsUsxmUnits.h
*
* @brief Hawk port interface header file
*
* @version   1
********************************************************************************
*/
#ifndef __mvHwsUsxmUnits_H
#define __mvHwsUsxmUnits_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  /*0*/  USXM_UNITS_CONTROL_RESET_E,
  /*1*/  USXM_UNITS_STATUS_RECEIVE_LINK_STATUS_E,
  /*2*/  USXM_UNITS_STATUS_RECEIVE_LINK_STATUS_LL_E,
  /*3*/  USXM_UNITS_PORTS_ENA_ACTIVE_PORTS_USED_E,
  /*4*/  USXM_UNITS_VL_INTVL_PORT_CYCLE_INTERVAL_E,
  /*5*/  USXM_UNITS_ALIGN_MATCH_ERR_MISMATCH_ERRCNT_E,
  /*6*/  USXM_UNITS_VL0_BYTE3_VL0_BYTE3_E,
  /*7*/  USXM_UNITS_VL0_0_VL0_0_E,
  /*8*/  USXM_UNITS_VL0_1_VL0_1_E,
  /*9*/  USXM_UNITS_VL1_0_VL1_0_E,
  /*10*/  USXM_UNITS_VL1_1_VL1_1_E,
  /*11*/  USXM_UNITS_VL2_0_VL2_0_E,
  /*12*/  USXM_UNITS_VL2_1_VL2_1_E,
  /*13*/  USXM_UNITS_VL3_0_VL3_0_E,
  /*14*/  USXM_UNITS_VL3_1_VL3_1_E,
    USXM_UNITS_REGISTER_LAST_E /* should be last */
} MV_HWS_USXM_UNITS_FIELDS_E;



#ifdef __cplusplus
}
#endif

#endif /* __mvHwsUsxmUnits_H */

