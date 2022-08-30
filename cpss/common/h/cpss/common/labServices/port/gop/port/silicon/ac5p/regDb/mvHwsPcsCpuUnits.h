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
* @file mvHwsPcsCpuUnits.h
*
* @brief Hawk port interface header file
*
* @version   1
********************************************************************************
*/
#ifndef __mvHwsPcsCpuUnits_H
#define __mvHwsPcsCpuUnits_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  /*0*/  CPU_PCS_UNITS_CONTROL1_PORT_SPEED_SELECTION_E,
  /*1*/  CPU_PCS_UNITS_CONTROL1_PORT_SPEED_ALWAYS1_E,
  /*2*/  CPU_PCS_UNITS_CONTROL1_PORT_LOW_POWER_E,
  /*3*/  CPU_PCS_UNITS_CONTROL1_PORT_SPEED_SELECT_ALWAYS1_E,
  /*4*/  CPU_PCS_UNITS_CONTROL1_PORT_LOOPBACK_E,
  /*5*/  CPU_PCS_UNITS_CONTROL1_PORT_RESET_E,
  /*6*/  CPU_PCS_UNITS_STATUS1_LOW_POWER_ABILITY_E,
  /*7*/  CPU_PCS_UNITS_STATUS1_PCS_RECEIVE_LINK_E,
  /*8*/  CPU_PCS_UNITS_STATUS1_FAULT_E,
  /*9*/  CPU_PCS_UNITS_STATUS1_RX_LPI_ACTIVE_E,
  /*10*/  CPU_PCS_UNITS_STATUS1_TX_LPI_ACTIVE_E,
  /*11*/  CPU_PCS_UNITS_STATUS1_RX_LPI_E,
  /*12*/  CPU_PCS_UNITS_STATUS1_TX_LPI_E,
  /*13*/  CPU_PCS_UNITS_DEVICE_ID0_IDENTIFIER0_E,
  /*14*/  CPU_PCS_UNITS_DEVICE_ID1_IDENTIFIER1_E,
  /*15*/  CPU_PCS_UNITS_SPEED_ABILITY_C10GETH_E,
  /*16*/  CPU_PCS_UNITS_SPEED_ABILITY_C10PASS_TS_E,
  /*17*/  CPU_PCS_UNITS_SPEED_ABILITY_C40G_E,
  /*18*/  CPU_PCS_UNITS_SPEED_ABILITY_C100G_E,
  /*19*/  CPU_PCS_UNITS_SPEED_ABILITY_C25G_E,
  /*20*/  CPU_PCS_UNITS_DEVICES_IN_PKG1_CLAUSE22_E,
  /*21*/  CPU_PCS_UNITS_DEVICES_IN_PKG1_PMD_PMA_E,
  /*22*/  CPU_PCS_UNITS_DEVICES_IN_PKG1_WIS_PRES_E,
  /*23*/  CPU_PCS_UNITS_DEVICES_IN_PKG1_PCS_PRES_E,
  /*24*/  CPU_PCS_UNITS_DEVICES_IN_PKG1_PHY_XS_E,
  /*25*/  CPU_PCS_UNITS_DEVICES_IN_PKG1_DTE_XS_E,
  /*26*/  CPU_PCS_UNITS_DEVICES_IN_PKG1_TC_PRES_E,
  /*27*/  CPU_PCS_UNITS_DEVICES_IN_PKG2_CLAUSE22_E,
  /*28*/  CPU_PCS_UNITS_DEVICES_IN_PKG2_DEVICE1_E,
  /*29*/  CPU_PCS_UNITS_DEVICES_IN_PKG2_DEVICE2_E,
  /*30*/  CPU_PCS_UNITS_CONTROL2_PCS_TYPE_E,
  /*31*/  CPU_PCS_UNITS_STATUS2_C10GBASE_R_E,
  /*32*/  CPU_PCS_UNITS_STATUS2_C10GBASE_X_E,
  /*33*/  CPU_PCS_UNITS_STATUS2_C10GBASE_W_E,
  /*34*/  CPU_PCS_UNITS_STATUS2_C10GBASE_T_E,
  /*35*/  CPU_PCS_UNITS_STATUS2_C40GBASE_R_E,
  /*36*/  CPU_PCS_UNITS_STATUS2_C100GBASE_R_E,
  /*37*/  CPU_PCS_UNITS_STATUS2_C25GBASE_R_E,
  /*38*/  CPU_PCS_UNITS_STATUS2_RECEIVE_FAULT_E,
  /*39*/  CPU_PCS_UNITS_STATUS2_TRANSMIT_FAULT_E,
  /*40*/  CPU_PCS_UNITS_STATUS2_DEVICE_PRESENT_E,
  /*41*/  CPU_PCS_UNITS_PKG_ID0_IDENTIFIER_E,
  /*42*/  CPU_PCS_UNITS_PKG_ID1_IDENTIFIER_E,
  /*43*/  CPU_PCS_UNITS_BASER_STATUS1_BLOCK_LOCK_E,
  /*44*/  CPU_PCS_UNITS_BASER_STATUS1_HIGH_BER_E,
  /*45*/  CPU_PCS_UNITS_BASER_STATUS1_RECEIVE_LINK_E,
  /*46*/  CPU_PCS_UNITS_BASER_STATUS2_ERRORED_CNT_E,
  /*47*/  CPU_PCS_UNITS_BASER_STATUS2_BER_COUNTER_E,
  /*48*/  CPU_PCS_UNITS_BASER_STATUS2_HIGH_BER_E,
  /*49*/  CPU_PCS_UNITS_BASER_STATUS2_BLOCK_LOCK_E,
  /*50*/  CPU_PCS_UNITS_SEED_A0_SEED_E,
  /*51*/  CPU_PCS_UNITS_SEED_A1_SEED_E,
  /*52*/  CPU_PCS_UNITS_SEED_A2_SEED_E,
  /*53*/  CPU_PCS_UNITS_SEED_A3_SEED_E,
  /*54*/  CPU_PCS_UNITS_SEED_B0_SEED_E,
  /*55*/  CPU_PCS_UNITS_SEED_B1_SEED_E,
  /*56*/  CPU_PCS_UNITS_SEED_B2_SEED_E,
  /*57*/  CPU_PCS_UNITS_SEED_B3_SEED_E,
  /*58*/  CPU_PCS_UNITS_BASER_TEST_CONTROL_DATA_PATTERN_SEL_E,
  /*59*/  CPU_PCS_UNITS_BASER_TEST_CONTROL_SELECT_SQUARE_E,
  /*60*/  CPU_PCS_UNITS_BASER_TEST_CONTROL_RX_TESTPATTERN_E,
  /*61*/  CPU_PCS_UNITS_BASER_TEST_CONTROL_TX_TESTPATTERN_E,
  /*62*/  CPU_PCS_UNITS_BASER_TEST_CONTROL_SELECT_RANDOM_E,
  /*63*/  CPU_PCS_UNITS_BASER_TEST_ERR_CNT_COUNTER_E,
  /*64*/  CPU_PCS_UNITS_BER_HIGH_ORDER_CNT_BER_COUNTER_E,
  /*65*/  CPU_PCS_UNITS_ERR_BLK_HIGH_ORDER_CNT_ERRORED_BLOCKS_COUNTER_E,
  /*66*/  CPU_PCS_UNITS_ERR_BLK_HIGH_ORDER_CNT_HIGH_ORDER_PRESENT_E,
  /*67*/  CPU_PCS_UNITS_MULTILANE_ALIGN_STAT1_LANE_ALIGN_STATUS_E,
  /*68*/  CPU_PCS_UNITS_VENDOR_SCRATCH_SCRATCH_E,
  /*69*/  CPU_PCS_UNITS_VENDOR_CORE_REV_REVISION_E,
  /*70*/  CPU_PCS_UNITS_VENDOR_PCS_MODE_ENA_CLAUSE49_E,
  /*71*/  CPU_PCS_UNITS_VENDOR_PCS_MODE_DISABLE_MLD_E,
  /*72*/  CPU_PCS_UNITS_VENDOR_PCS_MODE_HI_BER25_E,
  /*73*/  CPU_PCS_UNITS_VENDOR_PCS_MODE_ST_ENA_CLAUSE49_E,
  /*74*/  CPU_PCS_UNITS_VENDOR_PCS_MODE_ST_DISABLE_MLD_E,
  /*75*/  CPU_PCS_UNITS_MULTILANE_ALIGN_STAT3_LANE0_ALIGNMENT_MARKER_LOCK_E,
  /*76*/  CPU_PCS_UNITS_MULTILANE_ALIGN_STAT3_LANE1_ALIGNMENT_MARKER_LOCK_E,
  /*77*/  CPU_PCS_UNITS_MULTILANE_ALIGN_STAT3_LANE2_ALIGNMENT_MARKER_LOCK_E,
  /*78*/  CPU_PCS_UNITS_MULTILANE_ALIGN_STAT3_LANE3_ALIGNMENT_MARKER_LOCK_E,
  /*79*/  CPU_PCS_UNITS_VL_INTVL_ALIGNMENT_MARKERS_INTERVAL_E,
  /*80*/  CPU_PCS_UNITS_VENDOR_TX_LANE_THRESH_TX_FIFO_ALMOST_FULL_THRESHOLD_E,
  /*81*/  CPU_PCS_UNITS_VL0_0_M0_E,
  /*82*/  CPU_PCS_UNITS_VL0_0_M1_E,
  /*83*/  CPU_PCS_UNITS_VL0_1_M2_E,
  /*84*/  CPU_PCS_UNITS_VL1_0_M0_E,
  /*85*/  CPU_PCS_UNITS_VL1_0_M1_E,
  /*86*/  CPU_PCS_UNITS_VL1_1_M2_E,
  /*87*/  CPU_PCS_UNITS_VL2_0_M0_E,
  /*88*/  CPU_PCS_UNITS_VL2_0_M1_E,
  /*89*/  CPU_PCS_UNITS_VL2_1_M2_E,
  /*90*/  CPU_PCS_UNITS_VL3_0_M0_E,
  /*91*/  CPU_PCS_UNITS_VL3_0_M1_E,
  /*92*/  CPU_PCS_UNITS_VL3_1_M2_E,
  /*93*/  CPU_PCS_UNITS_VENDOR_GB_SHIFT_GB_SHIFT_E,
  /*94*/  CPU_PCS_UNITS_VENDOR_LAYENCY_RX_PATH_LATENCY_E,
  /*95*/  CPU_PCS_UNITS_VENDOR_LAYENCY_TX_PATH_LATENCY_E,
    CPU_PCS_UNITS_REGISTER_LAST_E /* should be last */
} MV_HWS_CPU_PCS_UNITS_FIELDS_E;

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsPcsCpuUnits_H */
