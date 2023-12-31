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
* @file mvHwsMtipCpuExtUnits.h
*
* @brief Hawk port interface header file
*
* @version   1
********************************************************************************
*/
#ifndef __mvHwsMtipCpuExtUnits_H
#define __mvHwsMtipCpuExtUnits_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  /*0*/  MTIP_CPU_EXT_UNITS_INTERRUPT_MASK_INTERRUPT_MASK_E,
  /*1*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_PORT_INTERRUPT_CAUSE_INT_SUM_E,
  /*2*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_LINK_OK_CHANGED_E,
  /*3*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_LINK_STATUS_CHANGED_E,
  /*4*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_RX_OVERRUN_E,
  /*5*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_TX_UNDERFLOW_E,
  /*6*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_TX_OVR_ERR_E,
  /*7*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_REM_FAULT_E,
  /*8*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_LOC_FAULT_E,
  /*9*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_LI_FAULT_E,
  /*10*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,
  /*11*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_HI_BER_INTERRUPT_E,
  /*12*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_FF_RX_RDY_E,
  /*13*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_LINK_OK_CLEAN_CHANGED_E,
  /*14*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_LINK_STATUS_CLEAN_CHANGED_E,
  /*15*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_PORT_TSU_RX_AM_ERR_E,
  /*16*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_PORT_TSU_RX_DFF_ERR_E,
  /*17*/  MTIP_CPU_EXT_UNITS_INTERRUPT_CAUSE_PORT_TSU_TX_SYNC_ERR_E,
  /*18*/  MTIP_CPU_EXT_UNITS_STATUS_LINK_OK_E,
  /*19*/  MTIP_CPU_EXT_UNITS_STATUS_LINK_STATUS_E,
  /*20*/  MTIP_CPU_EXT_UNITS_STATUS_TX_TRAFFIC_IND_E,
  /*21*/  MTIP_CPU_EXT_UNITS_STATUS_RX_TRAFFIC_IND_E,
  /*22*/  MTIP_CPU_EXT_UNITS_STATUS_BLOCK_LOCK_E,
  /*23*/  MTIP_CPU_EXT_UNITS_STATUS_HI_BER_E,
  /*24*/  MTIP_CPU_EXT_UNITS_STATUS_PFC_MODE_E,
  /*25*/  MTIP_CPU_EXT_UNITS_STATUS_TX_EMPTY_E,
  /*26*/  MTIP_CPU_EXT_UNITS_STATUS_MAC_RES_SPEED_E,
  /*27*/  MTIP_CPU_EXT_UNITS_STATUS_FF_RX_DSAV_E,
  /*28*/  MTIP_CPU_EXT_UNITS_STATUS_FF_RX_EMPTY_E,
  /*29*/  MTIP_CPU_EXT_UNITS_STATUS_FF_TX_SEPTY_E,
  /*30*/  MTIP_CPU_EXT_UNITS_STATUS_ALIGN_DONE_E,
  /*31*/  MTIP_CPU_EXT_UNITS_STATUS_SG_RX_SYNC_E,
  /*32*/  MTIP_CPU_EXT_UNITS_STATUS_SG_AN_DONE_E,
  /*33*/  MTIP_CPU_EXT_UNITS_STATUS_SG_HD_E,
  /*34*/  MTIP_CPU_EXT_UNITS_STATUS_MAC_TX_ISIDLE_E,
  /*35*/  MTIP_CPU_EXT_UNITS_STATUS_FEC91_ENA_OUT_E,
  /*36*/  MTIP_CPU_EXT_UNITS_STATUS_LINK_OK_CLEAN_E,
  /*37*/  MTIP_CPU_EXT_UNITS_STATUS_LINK_STATUS_CLEAN_E,
  /*38*/  MTIP_CPU_EXT_UNITS_CONTROL_TX_REM_FAULT_E,
  /*39*/  MTIP_CPU_EXT_UNITS_CONTROL_TX_LOC_FAULT_E,
  /*40*/  MTIP_CPU_EXT_UNITS_CONTROL_TX_LI_FAULT_E,
  /*41*/  MTIP_CPU_EXT_UNITS_CONTROL_PHY_LOS_E,
  /*42*/  MTIP_CPU_EXT_UNITS_CONTROL_TOD_SELECT_E,
  /*43*/  MTIP_CPU_EXT_UNITS_CONTROL_PAUSE_802_3_REFLECT_E,
  /*44*/  MTIP_CPU_EXT_UNITS_CONTROL_LOOP_ENA_E,
  /*45*/  MTIP_CPU_EXT_UNITS_CONTROL_MAC_WATCHDOG_ENA_E,
  /*46*/  MTIP_CPU_EXT_UNITS_CONTROL_PCS_WATCHDOG_ENA_E,
  /*47*/  MTIP_CPU_EXT_UNITS_CONTROL_FF_TX_CRC_E,
  /*48*/  MTIP_CPU_EXT_UNITS_CONTROL_LED_PORT_NUM_E,
  /*49*/  MTIP_CPU_EXT_UNITS_CONTROL_LED_PORT_EN_E,
  /*50*/  MTIP_CPU_EXT_UNITS_CONTROL_MASK_SW_RESET_E,
  /*51*/  MTIP_CPU_EXT_UNITS_CONTROL_PORT_RES_SPEED_E,
  /*52*/  MTIP_CPU_EXT_UNITS_CONTROL_FORCE_LINK_STATUS_EN_E,
  /*53*/  MTIP_CPU_EXT_UNITS_CONTROL_FORCE_LINK_STATUS_DIS_E,
  /*54*/  MTIP_CPU_EXT_UNITS_CONTROL_FORCE_LINK_OK_EN_E,
  /*55*/  MTIP_CPU_EXT_UNITS_CONTROL_FORCE_LINK_OK_DIS_E,
  /*56*/  MTIP_CPU_EXT_UNITS_CONTROL_LOOP_TX_RDY_OUT_E,
  /*57*/  MTIP_CPU_EXT_UNITS_CONTROL_LOOP_RX_BLOCK_OUT_E,
  /*58*/  MTIP_CPU_EXT_UNITS_CONTROL_FORCE_TS_IF_CF_EN_E,
  /*59*/  MTIP_CPU_EXT_UNITS_CONTROL_FORCE_TS_IF_CF_DIS_E,
  /*60*/  MTIP_CPU_EXT_UNITS_PCS_METAL_FIX_PORT_PCS_METAL_FIX_E,
  /*61*/  MTIP_CPU_EXT_UNITS_LAST_VIOLATION_PORT_LAST_VIOLATION_E,
  /*62*/  MTIP_CPU_EXT_UNITS_RESET_SD_TX_RESET__E,
  /*63*/  MTIP_CPU_EXT_UNITS_RESET_SD_RX_RESET__E,
  /*64*/  MTIP_CPU_EXT_UNITS_RESET_PORT_RESET__E,
  /*65*/  MTIP_CPU_EXT_UNITS_PAUSE_AND_ERR_STAT_PAUSE_ON_E,
  /*66*/  MTIP_CPU_EXT_UNITS_PAUSE_AND_ERR_STAT_FF_RX_ERR_STAT_E,
  /*67*/  MTIP_CPU_EXT_UNITS_CLOCKS_APP_CLK_EN_E,
  /*68*/  MTIP_CPU_EXT_UNITS_CLOCKS_MAC_CLK_EN_E,
  /*69*/  MTIP_CPU_EXT_UNITS_CLOCKS_SD_PCS_TX_CLK_EN_E,
  /*70*/  MTIP_CPU_EXT_UNITS_CLOCKS_SD_PCS_RX_CLK_EN_E,
  /*71*/  MTIP_CPU_EXT_UNITS_CLOCKS_SGPCS_CLK_EN_E,
  /*72*/  MTIP_CPU_EXT_UNITS_CLOCKS_PCS_10_25G_CLK_EN_E,
  /*73*/  MTIP_CPU_EXT_UNITS_XOFF_STATUS_XOFF_STATUS_E,
  /*74*/  MTIP_CPU_EXT_UNITS_PAUSE_OVERRIDE_PAUSE_OVERRIDE_CTRL_E,
  /*75*/  MTIP_CPU_EXT_UNITS_PAUSE_OVERRIDE_PAUSE_OVERRIDE_VAL_E,
  /*76*/  MTIP_CPU_EXT_UNITS_XOFF_OVERRIDE_XOFF_OVERRIDE_CTRL_E,
  /*77*/  MTIP_CPU_EXT_UNITS_XOFF_OVERRIDE_XOFF_OVERRIDE_VAL_E,
  /*78*/  MTIP_CPU_EXT_UNITS_MAC_METAL_FIX_PORT_MAC_METAL_FIX_E,
  /*79*/  MTIP_CPU_EXT_UNITS_PEER_DELAY_PEER_DELAY_E,
  /*80*/  MTIP_CPU_EXT_UNITS_PEER_DELAY_PEER_DELAY_VAL_E,
  /*81*/  MTIP_CPU_EXT_UNITS_CONTROL2_CYC_TO_STRECH_MAC2REG_E,
  /*82*/  MTIP_CPU_EXT_UNITS_CONTROL2_CYC_TO_STRECH_MAC2APP_E,
  /*83*/  MTIP_CPU_EXT_UNITS_CONTROL2_CYC_TO_STRECH_APP2REG_E,
  /*84*/  MTIP_CPU_EXT_UNITS_CONTROL2_RX_PAUSE_CONTROL_E,
  /*85*/  MTIP_CPU_EXT_UNITS_CONTROL2_RX_PAUSE_OW_VAL_E,
  /*86*/  MTIP_CPU_EXT_UNITTIME_STAMP_DISPATCHER_CONTROL_1_AMD_CNT_LOW_E,
  /*87*/  MTIP_CPU_EXT_UNITTIME_STAMP_DISPATCHER_CONTROL_1_AMD_CNT_HIGH_E,
  /*88*/  MTIP_CPU_EXT_UNITS_TSU_MODULO_TX_PORT_TSU_MODULO_TX_E,
  /*89*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_0_PORT_TSU_C_TX_MODE_E,
  /*90*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_0_PORT_TSU_C_RX_MODE_E,
  /*91*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_0_PORT_TSU_DESKEW_E,
  /*92*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_0_PORT_TSU_MII_MK_DLY_E,
  /*93*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_0_PORT_TSU_MII_CW_DLY_E,
  /*94*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_0_PORT_TSU_C_MII_TX_MK_CYC_DLY_E,
  /*95*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_0_PORT_TSU_C_MII_TX_CW_CYC_DLY_E,
  /*96*/  MTIP_CPU_EXT_UNITS_TSU_MODULO_RX_PORT_TSU_MODULO_RX_E,
  /*97*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_2_C_BLKS_PER_CLK_OW_E,
  /*98*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_2_C_TX_MODE_OW_E,
  /*99*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_2_C_RX_MODE_OW_E,
  /*100*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_2_C_MII_CW_DLY_OW_E,
  /*101*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_2_C_MII_MK_DLY_OW_E,
  /*102*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_2_C_DESKEW_OW_E,
  /*103*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_2_C_MODULO_RX_OW_E,
  /*104*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_2_C_MODULO_TX_OW_E,
  /*105*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_2_C_BLOCKTIME_INT_OW_E,
  /*106*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_2_C_BLOCKTIME_DEC_OW_E,
  /*107*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_2_C_MARKERTIME_INT_OW_E,
  /*108*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_2_C_MARKERTIME_DEC_OW_E,
  /*109*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_2_C_MII_TX_MK_CYC_DLY_OW_E,
  /*110*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_2_C_MII_TX_CW_CYC_DLY_OW_E,
  /*111*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_2_C_TSU_TX_SD_PERIOD_OW_E,
  /*112*/  MTIP_CPU_EXT_UNITS_TIME_STAMP_DISPATCHER_CONTROL_0_FSU_ENABLE_E,
  /*113*/  MTIP_CPU_EXT_UNITS_TIME_STAMP_DISPATCHER_CONTROL_0_AMD_ENABLE_E,
  /*114*/  MTIP_CPU_EXT_UNITS_TIME_STAMP_DISPATCHER_CONTROL_0_FSU_OFFSET_E,
  /*115*/  MTIP_CPU_EXT_UNITS_TIME_STAMP_DISPATCHER_CONTROL_0_FSU_RND_DELTA_E,
  /*116*/  MTIP_CPU_EXT_UNITS_TIME_STAMP_DISPATCHER_CONTROL_0_AMD_CNT_TYPE_SEL_E,
  /*117*/  MTIP_CPU_EXT_UNITS_TIME_STAMP_DISPATCHER_CONTROL_0_MINIMAL_TX_STOP_TOGGLE_E,
  /*118*/  MTIP_CPU_EXT_UNITS_TSU_STATUS_PORT_TSU_RX_READY_E,
  /*119*/  MTIP_CPU_EXT_UNITS_TSU_STATUS_PORT_TSU_TX_READY_E,
  /*120*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_1_PORT_TSU_BLOCKTIME_E,
  /*121*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_1_PORT_TSU_BLOCKTIME_DEC_E,
  /*122*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_1_PORT_TSU_MARKERTIME_E,
  /*123*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_1_PORT_TSU_MARKERTIME_DEC_E,
  /*124*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_1_PORT_TSU_BLKS_PER_CLK_E,
  /*125*/  MTIP_CPU_EXT_UNITS_TSU_CONTROL_3_PORT_TSU_C_TSU_TX_SD_PERIOD_E,
    MTIP_CPU_EXT_UNITS_REGISTER_LAST_E /* should be last */
} MV_HWS_MTIP_CPU_EXT_UNITS_FIELDS_E;



#ifdef __cplusplus
}
#endif

#endif /* __mvHwsMtipCpuExtUnits_H */
