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
* @file mvHwsMtiPcs200RegDb.h
*
* @brief Hawk port interface header file
*
* @version   1
********************************************************************************
*/
#ifndef __mvHwsMtiPcs200RegDb_H
#define __mvHwsMtiPcs200RegDb_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  /*0*/  PCS200_UNITS_CONTROL1_SPEED_SELECTION_E,
  /*1*/  PCS200_UNITS_CONTROL1_SPEED_SELECTION_6_E,
  /*2*/  PCS200_UNITS_CONTROL1_LOW_POWER_E,
  /*3*/  PCS200_UNITS_CONTROL1_SPEED_SELECTION_13_E,
  /*4*/  PCS200_UNITS_CONTROL1_LOOPBACK_E,
  /*5*/  PCS200_UNITS_CONTROL1_RESET_E,
  /*6*/  PCS200_UNITS_STATUS1_PCS_RECEIVE_LINK_E,
  /*7*/  PCS200_UNITS_STATUS1_FAULT_E,
  /*8*/  PCS200_UNITS_DEVICE_ID0_IDENTIFIER0_E,
  /*9*/  PCS200_UNITS_DEVICE_ID1_IDENTIFIER1_E,
  /*10*/  PCS200_UNITS_SPEED_ABILITY_SPEEDABILITY_8_E,
  /*11*/  PCS200_UNITS_SPEED_ABILITY_SPEEDABILITY_9_E,
  /*12*/  PCS200_UNITS_DEVICES_IN_PKG1_DEVICES_PCSPRESENT_E,
  /*13*/  PCS200_UNITS_DEVICES_IN_PKG2_DEVICES_PKG2_E,
  /*14*/  PCS200_UNITS_CONTROL2_PCS_TYPE_E,
  /*15*/  PCS200_UNITS_STATUS2_RECEIVE_FAULT_E,
  /*16*/  PCS200_UNITS_STATUS2_TRANSMIT_FAULT_E,
  /*17*/  PCS200_UNITS_STATUS2_DEVICE_PRESENT_E,
  /*18*/  PCS200_UNITS_STATUS3_STATUS3_200_E,
  /*19*/  PCS200_UNITS_STATUS3_STATUS3_400_E,
  /*20*/  PCS200_UNITS_PKG_ID0_PACKAGE_IDENTIFIER0_E,
  /*21*/  PCS200_UNITS_PKG_ID1_PACKAGE_IDENTIFIER1_E,
  /*22*/  PCS200_UNITS_BASER_STATUS1_RECEIVE_LINK_E,
  /*23*/  PCS200_UNITS_BASER_STATUS2_ERRORED_CNT_E,
  /*24*/  PCS200_UNITS_BASER_TEST_CONTROL_SELECT_RANDOM_E,
  /*25*/  PCS200_UNITS_BASER_TEST_ERR_CNT_COUNTER_E,
  /*26*/  PCS200_UNITS_BER_HIGH_ORDER_CNT_BER_COUNTER_E,
  /*27*/  PCS200_UNITS_ERR_BLK_HIGH_ORDER_CNT_ERRORED_BLOCKS_COUNTER_E,
  /*28*/  PCS200_UNITS_ERR_BLK_HIGH_ORDER_CNT_HIGH_ORDER_PRESENT_E,
  /*29*/  PCS200_UNITS_MULTILANE_ALIGN_STAT1_LANE_ALIGN_STATUS_E,
  /*30*/  PCS200_UNITS_LANE0_MAPPING_LANE0_MAPPING_E,
  /*31*/  PCS200_UNITS_LANE1_MAPPING_LANE1_MAPPING_E,
  /*32*/  PCS200_UNITS_LANE2_MAPPING_LANE2_MAPPING_E,
  /*33*/  PCS200_UNITS_LANE3_MAPPING_LANE3_MAPPING_E,
  /*34*/  PCS200_UNITS_LANE4_MAPPING_LANE4_MAPPING_E,
  /*35*/  PCS200_UNITS_LANE5_MAPPING_LANE5_MAPPING_E,
  /*36*/  PCS200_UNITS_LANE6_MAPPING_LANE6_MAPPING_E,
  /*37*/  PCS200_UNITS_LANE7_MAPPING_LANE7_MAPPING_E,
    PCS200_UNITS_REGISTER_LAST_E /* should be last */
} MV_HWS_PCS200_UNIT_FIELDS_E;



#ifdef __cplusplus
}
#endif

#endif /* __mvHwsMtiPcs200RegDb_H */

