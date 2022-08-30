/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssHawkTxqSdqRegFile.h
*
* DESCRIPTION:
*      Describe TXQ SDQ registers (Automaticly generated from CIDER)
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*
*******************************************************************************/
#ifndef _TXQ_HAWK_SDQ_REGFILE_H_
#define _TXQ_HAWK_SDQ_REGFILE_H_

/* start of register global_config*/

#define      TXQ_HAWK_SDQ_GLOBAL_CONFIG_AGING_EN_FIELD_OFFSET           0
#define      TXQ_HAWK_SDQ_GLOBAL_CONFIG_MSG_CNCL_EN_FIELD_OFFSET            1
#define      TXQ_HAWK_SDQ_GLOBAL_CONFIG_SELIG_EN_FIELD_OFFSET           2
#define      TXQ_HAWK_SDQ_GLOBAL_CONFIG_SELIG4DRY_EN_FIELD_OFFSET           3
#define      TXQ_HAWK_SDQ_GLOBAL_CONFIG_SELIG_1K_EN_FIELD_OFFSET            4
#define      TXQ_HAWK_SDQ_GLOBAL_CONFIG_CRDT_CNCL_EN_FIELD_OFFSET           5


#define      TXQ_HAWK_SDQ_GLOBAL_CONFIG_AGING_EN_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_GLOBAL_CONFIG_MSG_CNCL_EN_FIELD_SIZE          1
#define      TXQ_HAWK_SDQ_GLOBAL_CONFIG_SELIG_EN_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_GLOBAL_CONFIG_SELIG4DRY_EN_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_GLOBAL_CONFIG_SELIG_1K_EN_FIELD_SIZE          1
#define      TXQ_HAWK_SDQ_GLOBAL_CONFIG_CRDT_CNCL_EN_FIELD_SIZE         1

/* end of global_config*/


/* start of register SDQ_Metal_Fix*/

#define      TXQ_HAWK_SDQ_SDQ_METAL_FIX_SDQ_METAL_FIX_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SDQ_METAL_FIX_SDQ_METAL_FIX_FIELD_SIZE            32

/* end of SDQ_Metal_Fix*/


/* start of register SDQ_Interrupt_Functional_Cause*/

#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SDQ_FUNC_INTSUM_FIELD_OFFSET           0
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_ILLEGAL_ADDRESS_ACCESS_FIELD_OFFSET            1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_ILLEGAL_TABLE_OVERLAP_FIELD_OFFSET         2
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_QUEUE_CRDT_FIELD_OFFSET          3
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_QUEUE_ENQ_FIELD_OFFSET           4
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_QUEUE_DEQ_FIELD_OFFSET           5
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_QUEUE_QCN_FIELD_OFFSET           6
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_PORT_CRDT_FIELD_OFFSET           7
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_PORT_ENQ_FIELD_OFFSET            8
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_PORT_DEQ_FIELD_OFFSET            9
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_PORT_QCN_FIELD_OFFSET            10
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_PORT_PFC_FIELD_OFFSET            11
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_PORT_SEL_FIELD_OFFSET            12
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_PORT_OUTOFRANGE_FIELD_OFFSET           13
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_PORT_MAP_CONF_NOT_EMPTY_FIELD_OFFSET           14
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_LIST_PTRS_CONF_NOT_EMPTY_FIELD_OFFSET          15


#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SDQ_FUNC_INTSUM_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_ILLEGAL_ADDRESS_ACCESS_FIELD_SIZE          1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_ILLEGAL_TABLE_OVERLAP_FIELD_SIZE           1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_QUEUE_CRDT_FIELD_SIZE            1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_QUEUE_ENQ_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_QUEUE_DEQ_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_QUEUE_QCN_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_PORT_CRDT_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_PORT_ENQ_FIELD_SIZE          1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_PORT_DEQ_FIELD_SIZE          1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_PORT_QCN_FIELD_SIZE          1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_PORT_PFC_FIELD_SIZE          1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_PORT_SEL_FIELD_SIZE          1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_PORT_OUTOFRANGE_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_PORT_MAP_CONF_NOT_EMPTY_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_LIST_PTRS_CONF_NOT_EMPTY_FIELD_SIZE            1

/* end of SDQ_Interrupt_Functional_Cause*/


/* start of register sdq_interrupt_functional_mask*/

#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK0_FIELD_OFFSET           1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK1_FIELD_OFFSET           2
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK2_FIELD_OFFSET           3
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK3_FIELD_OFFSET           4
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK4_FIELD_OFFSET           5
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK5_FIELD_OFFSET           6
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK6_FIELD_OFFSET           7
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK7_FIELD_OFFSET           8
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK8_FIELD_OFFSET           9
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK9_FIELD_OFFSET           10
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK10_FIELD_OFFSET          11
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK11_FIELD_OFFSET          12
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK12_FIELD_OFFSET          13
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK13_FIELD_OFFSET          14
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK14_FIELD_OFFSET          15


#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK0_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK1_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK2_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK3_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK4_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK5_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK6_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK7_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK8_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK9_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK10_FIELD_SIZE            1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK11_FIELD_SIZE            1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK12_FIELD_SIZE            1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK13_FIELD_SIZE            1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_FUNCTIONAL_MASK_SDQ_FUNC_INT_CAUSE_MASK14_FIELD_SIZE            1

/* end of sdq_interrupt_functional_mask*/


/* start of register SDQ_Interrupt_Summary_Mask*/

#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_SUMMARY_MASK_SDQ_INT_SUM_MASK0_FIELD_OFFSET         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_SUMMARY_MASK_SDQ_INT_SUM_MASK1_FIELD_OFFSET         2


#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_SUMMARY_MASK_SDQ_INT_SUM_MASK0_FIELD_SIZE           1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_SUMMARY_MASK_SDQ_INT_SUM_MASK1_FIELD_SIZE           1

/* end of SDQ_Interrupt_Summary_Mask*/


/* start of register SDQ_Interrupt_Summary_Cause*/

#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_SUMMARY_CAUSE_SDQ_INT_SUM_FIELD_OFFSET          0
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_SUMMARY_CAUSE_SDQ_FUNC_INT_SUM_FIELD_OFFSET         1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_SUMMARY_CAUSE_SDQ_DBG_INT_SUM_FIELD_OFFSET          2


#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_SUMMARY_CAUSE_SDQ_INT_SUM_FIELD_SIZE            1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_SUMMARY_CAUSE_SDQ_FUNC_INT_SUM_FIELD_SIZE           1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_SUMMARY_CAUSE_SDQ_DBG_INT_SUM_FIELD_SIZE            1

/* end of SDQ_Interrupt_Summary_Cause*/


/* start of register SDQ_Interrupt_Debug_Mask*/

#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_DEBUG_MASK_SDQ_DEBUG_INT_CAUSE_MASK0_FIELD_OFFSET           1


#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_DEBUG_MASK_SDQ_DEBUG_INT_CAUSE_MASK0_FIELD_SIZE         1

/* end of SDQ_Interrupt_Debug_Mask*/


/* start of register SDQ_Interrupt_Debug_Cause*/

#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_DEBUG_CAUSE_SDQ_DBG_INTSUM_FIELD_OFFSET         0
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_DEBUG_CAUSE_QCN_FIFO_OVERRUN_FIELD_OFFSET           1


#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_DEBUG_CAUSE_SDQ_DBG_INTSUM_FIELD_SIZE           1
#define      TXQ_HAWK_SDQ_SDQ_INTERRUPT_DEBUG_CAUSE_QCN_FIFO_OVERRUN_FIELD_SIZE         1

/* end of SDQ_Interrupt_Debug_Cause*/


/* start of table entry queue_config*/

#define      TXQ_HAWK_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_OFFSET            0
#define      TXQ_HAWK_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_OFFSET          3
#define      TXQ_HAWK_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_OFFSET          7
#define      TXQ_HAWK_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_OFFSET          14


#define      TXQ_HAWK_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_SIZE          1
#define      TXQ_HAWK_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE            4
#define      TXQ_HAWK_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_SIZE            7
#define      TXQ_HAWK_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_SIZE            6

/* end of queue_config*/


/* start of table entry Port_Range_Low*/

#define      TXQ_HAWK_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_SIZE            8

/* end of Port_Range_Low*/


/* start of table entry Port_Range_High*/

#define      TXQ_HAWK_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_SIZE          8

/* end of Port_Range_High*/


/* start of table entry Port_Back_Pressure_Low_Threshold*/

#define      TXQ_HAWK_SDQ_PORT_BACK_PRESSURE_LOW_THRESHOLD_PORT_BP_LOW_TH_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_PORT_BACK_PRESSURE_LOW_THRESHOLD_PORT_BP_LOW_TH_FIELD_SIZE            27

/* end of Port_Back_Pressure_Low_Threshold*/


/* start of table entry Port_Back_Pressure_High_Threshold*/

#define      TXQ_HAWK_SDQ_PORT_BACK_PRESSURE_HIGH_THRESHOLD_PORT_BP_HIGH_TH_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_PORT_BACK_PRESSURE_HIGH_THRESHOLD_PORT_BP_HIGH_TH_FIELD_SIZE          27

/* end of Port_Back_Pressure_High_Threshold*/


/* start of table entry Port_Config*/

#define      TXQ_HAWK_SDQ_PORT_CONFIG_PORT_EN_0_FIELD_OFFSET            0
#define      TXQ_HAWK_SDQ_PORT_CONFIG_PORT_BP_EN_FIELD_OFFSET           1
#define      TXQ_HAWK_SDQ_PORT_CONFIG_CRDT_IGN_0_FIELD_OFFSET           2
#define      TXQ_HAWK_SDQ_PORT_CONFIG_QDRYER_SELIG_TH0_0_FIELD_OFFSET           3
#define      TXQ_HAWK_SDQ_PORT_CONFIG_QDRYER_SELIG_TH1_0_FIELD_OFFSET           8


#define      TXQ_HAWK_SDQ_PORT_CONFIG_PORT_EN_0_FIELD_SIZE          1
#define      TXQ_HAWK_SDQ_PORT_CONFIG_PORT_BP_EN_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_PORT_CONFIG_CRDT_IGN_0_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_PORT_CONFIG_QDRYER_SELIG_TH0_0_FIELD_SIZE         5
#define      TXQ_HAWK_SDQ_PORT_CONFIG_QDRYER_SELIG_TH1_0_FIELD_SIZE         5

/* end of Port_Config*/


/* start of register QCN_Config*/

#define      TXQ_HAWK_SDQ_QCN_CONFIG_QCN_FRQ_FIELD_OFFSET           0


#define      TXQ_HAWK_SDQ_QCN_CONFIG_QCN_FRQ_FIELD_SIZE         2

/* end of QCN_Config*/


/* start of table entry PFC_Control*/

#define      TXQ_HAWK_SDQ_PFC_CONTROL_TC_EN0_FIELD_OFFSET           0


#define      TXQ_HAWK_SDQ_PFC_CONTROL_TC_EN0_FIELD_SIZE         16

/* end of PFC_Control*/


/* start of table entry Pause_TC*/

#define      TXQ_HAWK_SDQ_PAUSE_TC_PAUSE_TC_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_PAUSE_TC_PAUSE_TC_0_FIELD_SIZE            16

/* end of Pause_TC*/


/* start of register Sdq_Idle*/

#define      TXQ_HAWK_SDQ_SDQ_IDLE_SDQ_IDLE_FIELD_OFFSET            0
#define      TXQ_HAWK_SDQ_SDQ_IDLE_ELIG_TOP_IDLE_FIELD_OFFSET           1
#define      TXQ_HAWK_SDQ_SDQ_IDLE_ELIG_STGS_IDLE_FIELD_OFFSET          2
#define      TXQ_HAWK_SDQ_SDQ_IDLE_ELIG_STGD_IDLE_FIELD_OFFSET          3
#define      TXQ_HAWK_SDQ_SDQ_IDLE_ELIG_STGQ_IDLE_FIELD_OFFSET          4
#define      TXQ_HAWK_SDQ_SDQ_IDLE_QDRYER_IDLE_FIELD_OFFSET         5
#define      TXQ_HAWK_SDQ_SDQ_IDLE_SEL_IDLE_FIELD_OFFSET            6
#define      TXQ_HAWK_SDQ_SDQ_IDLE_MSG_IDLE_FIELD_OFFSET            7
#define      TXQ_HAWK_SDQ_SDQ_IDLE_PORT_IDLE_FIELD_OFFSET           8
#define      TXQ_HAWK_SDQ_SDQ_IDLE_GLOBAL_PORT_IDLE_FIELD_OFFSET            17


#define      TXQ_HAWK_SDQ_SDQ_IDLE_SDQ_IDLE_FIELD_SIZE          1
#define      TXQ_HAWK_SDQ_SDQ_IDLE_ELIG_TOP_IDLE_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_SDQ_IDLE_ELIG_STGS_IDLE_FIELD_SIZE            1
#define      TXQ_HAWK_SDQ_SDQ_IDLE_ELIG_STGD_IDLE_FIELD_SIZE            1
#define      TXQ_HAWK_SDQ_SDQ_IDLE_ELIG_STGQ_IDLE_FIELD_SIZE            1
#define      TXQ_HAWK_SDQ_SDQ_IDLE_QDRYER_IDLE_FIELD_SIZE           1
#define      TXQ_HAWK_SDQ_SDQ_IDLE_SEL_IDLE_FIELD_SIZE          1
#define      TXQ_HAWK_SDQ_SDQ_IDLE_MSG_IDLE_FIELD_SIZE          1
#define      TXQ_HAWK_SDQ_SDQ_IDLE_PORT_IDLE_FIELD_SIZE         9
#define      TXQ_HAWK_SDQ_SDQ_IDLE_GLOBAL_PORT_IDLE_FIELD_SIZE          1

/* end of Sdq_Idle*/


/* start of register Illegal_Bad_Address*/

#define      TXQ_HAWK_SDQ_ILLEGAL_BAD_ADDRESS_BAD_ADDRESS_CAPTURE_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_ILLEGAL_BAD_ADDRESS_BAD_ADDRESS_CAPTURE_FIELD_SIZE            32

/* end of Illegal_Bad_Address*/


/* start of register QCN_Message_Drop_Counter*/

#define      TXQ_HAWK_SDQ_QCN_MESSAGE_DROP_COUNTER_QCN_MSG_DRP_CNT_FIELD_OFFSET         0


#define      TXQ_HAWK_SDQ_QCN_MESSAGE_DROP_COUNTER_QCN_MSG_DRP_CNT_FIELD_SIZE           32

/* end of QCN_Message_Drop_Counter*/


/* start of register Queue_Dryer_FIFO_Max_Peak*/

#define      TXQ_HAWK_SDQ_QUEUE_DRYER_FIFO_MAX_PEAK_DRYER_FIFO_MAX_PEAK_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_QUEUE_DRYER_FIFO_MAX_PEAK_DRYER_FIFO_MAX_PEAK_FIELD_SIZE          9

/* end of Queue_Dryer_FIFO_Max_Peak*/


/* start of register PDQ_Message_Output_FIFO_Max_Peak*/

#define      TXQ_HAWK_SDQ_PDQ_MESSAGE_OUTPUT_FIFO_MAX_PEAK_MSG_FIFO_MAX_PEAK_FIELD_OFFSET           0


#define      TXQ_HAWK_SDQ_PDQ_MESSAGE_OUTPUT_FIFO_MAX_PEAK_MSG_FIFO_MAX_PEAK_FIELD_SIZE         9

/* end of PDQ_Message_Output_FIFO_Max_Peak*/


/* start of register QCN_Message_Drop_Counter_Control*/

#define      TXQ_HAWK_SDQ_QCN_MESSAGE_DROP_COUNTER_CONTROL_QCN_MSG_DROP_TRIG_FIELD_OFFSET           0
#define      TXQ_HAWK_SDQ_QCN_MESSAGE_DROP_COUNTER_CONTROL_QCN_MSG_DROP_QUEUE_FIELD_OFFSET          2
#define      TXQ_HAWK_SDQ_QCN_MESSAGE_DROP_COUNTER_CONTROL_QCN_MSG_DROP_PORT_FIELD_OFFSET           10


#define      TXQ_HAWK_SDQ_QCN_MESSAGE_DROP_COUNTER_CONTROL_QCN_MSG_DROP_TRIG_FIELD_SIZE         2
#define      TXQ_HAWK_SDQ_QCN_MESSAGE_DROP_COUNTER_CONTROL_QCN_MSG_DROP_QUEUE_FIELD_SIZE            8
#define      TXQ_HAWK_SDQ_QCN_MESSAGE_DROP_COUNTER_CONTROL_QCN_MSG_DROP_PORT_FIELD_SIZE         5

/* end of QCN_Message_Drop_Counter_Control*/


/* start of register QCN_Message_FIFO_Max_Peak*/

#define      TXQ_HAWK_SDQ_QCN_MESSAGE_FIFO_MAX_PEAK_QCN_MSG_FIFO_MAX_PEAK_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_QCN_MESSAGE_FIFO_MAX_PEAK_QCN_MSG_FIFO_MAX_PEAK_FIELD_SIZE            6

/* end of QCN_Message_FIFO_Max_Peak*/


/* start of register QCN_Message_FIFO_Fill_Level*/

#define      TXQ_HAWK_SDQ_QCN_MESSAGE_FIFO_FILL_LEVEL_QCN_MSG_FIFO_FILL_LEVEL_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_QCN_MESSAGE_FIFO_FILL_LEVEL_QCN_MSG_FIFO_FILL_LEVEL_FIELD_SIZE            6

/* end of QCN_Message_FIFO_Fill_Level*/


/* start of register Elig_Func_Error_Capture*/

#define      TXQ_HAWK_SDQ_ELIG_FUNC_ERROR_CAPTURE_ELIG_ERROR_QUEUE_FIELD_OFFSET         0
#define      TXQ_HAWK_SDQ_ELIG_FUNC_ERROR_CAPTURE_ELIG_ERROR_PORT_FIELD_OFFSET          8
#define      TXQ_HAWK_SDQ_ELIG_FUNC_ERROR_CAPTURE_ELIG_ERROR_SRC_FIELD_OFFSET           13
#define      TXQ_HAWK_SDQ_ELIG_FUNC_ERROR_CAPTURE_ELIG_ERROR_VALID_FIELD_OFFSET         16


#define      TXQ_HAWK_SDQ_ELIG_FUNC_ERROR_CAPTURE_ELIG_ERROR_QUEUE_FIELD_SIZE           8
#define      TXQ_HAWK_SDQ_ELIG_FUNC_ERROR_CAPTURE_ELIG_ERROR_PORT_FIELD_SIZE            5
#define      TXQ_HAWK_SDQ_ELIG_FUNC_ERROR_CAPTURE_ELIG_ERROR_SRC_FIELD_SIZE         3
#define      TXQ_HAWK_SDQ_ELIG_FUNC_ERROR_CAPTURE_ELIG_ERROR_VALID_FIELD_SIZE           1

/* end of Elig_Func_Error_Capture*/


/* start of register Select_Func_Error_Capture*/

#define      TXQ_HAWK_SDQ_SELECT_FUNC_ERROR_CAPTURE_SEL_ERROR_PORT_FIELD_OFFSET         0
#define      TXQ_HAWK_SDQ_SELECT_FUNC_ERROR_CAPTURE_SEL_ERROR_VALID_FIELD_OFFSET            5


#define      TXQ_HAWK_SDQ_SELECT_FUNC_ERROR_CAPTURE_SEL_ERROR_PORT_FIELD_SIZE           5
#define      TXQ_HAWK_SDQ_SELECT_FUNC_ERROR_CAPTURE_SEL_ERROR_VALID_FIELD_SIZE          1

/* end of Select_Func_Error_Capture*/


/* start of table entry Sel_List_Range_Low_0*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_0*/


/* start of table entry Sel_List_Range_Low_1*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_1_SEL_LIST_RANGE_LOW_1_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_1_SEL_LIST_RANGE_LOW_1_0_FIELD_SIZE            8

/* end of Sel_List_Range_Low_1*/


/* start of table entry Sel_List_Range_Low_2*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_2_SEL_LIST_RANGE_LOW_2_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_2_SEL_LIST_RANGE_LOW_2_0_FIELD_SIZE            8

/* end of Sel_List_Range_Low_2*/


/* start of table entry Sel_List_Range_Low_3*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_3_SEL_LIST_RANGE_LOW_3_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_3_SEL_LIST_RANGE_LOW_3_0_FIELD_SIZE            8

/* end of Sel_List_Range_Low_3*/


/* start of table entry Sel_List_Range_Low_4*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_4_SEL_LIST_RANGE_LOW_4_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_4_SEL_LIST_RANGE_LOW_4_0_FIELD_SIZE            8

/* end of Sel_List_Range_Low_4*/


/* start of table entry Sel_List_Range_Low_5*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_5_SEL_LIST_RANGE_LOW_5_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_5_SEL_LIST_RANGE_LOW_5_0_FIELD_SIZE            8

/* end of Sel_List_Range_Low_5*/


/* start of table entry Sel_List_Range_Low_6*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_6_SEL_LIST_RANGE_LOW_6_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_6_SEL_LIST_RANGE_LOW_6_0_FIELD_SIZE            8

/* end of Sel_List_Range_Low_6*/


/* start of table entry Sel_List_Range_Low_7*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_7_SEL_LIST_RANGE_LOW_7_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_7_SEL_LIST_RANGE_LOW_7_0_FIELD_SIZE            8

/* end of Sel_List_Range_Low_7*/


/* start of table entry Sel_List_Range_Low_8*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_8_SEL_LIST_RANGE_LOW_8_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_8_SEL_LIST_RANGE_LOW_8_0_FIELD_SIZE            8

/* end of Sel_List_Range_Low_8*/


/* start of table entry Sel_List_Range_Low_9*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_9_SEL_LIST_RANGE_LOW_9_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_9_SEL_LIST_RANGE_LOW_9_0_FIELD_SIZE            8

/* end of Sel_List_Range_Low_9*/


/* start of table entry Sel_List_Range_Low_10*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_10_SEL_LIST_RANGE_LOW_10_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_10_SEL_LIST_RANGE_LOW_10_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_10*/


/* start of table entry Sel_List_Range_Low_11*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_11_SEL_LIST_RANGE_LOW_11_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_11_SEL_LIST_RANGE_LOW_11_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_11*/


/* start of table entry Sel_List_Range_Low_12*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_12_SEL_LIST_RANGE_LOW_12_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_12_SEL_LIST_RANGE_LOW_12_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_12*/


/* start of table entry Sel_List_Range_Low_13*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_13_SEL_LIST_RANGE_LOW_13_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_13_SEL_LIST_RANGE_LOW_13_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_13*/


/* start of table entry Sel_List_Range_Low_14*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_14_SEL_LIST_RANGE_LOW_14_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_14_SEL_LIST_RANGE_LOW_14_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_14*/


/* start of table entry Sel_List_Range_Low_15*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_15_SEL_LIST_RANGE_LOW_15_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_15_SEL_LIST_RANGE_LOW_15_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_15*/


/* start of table entry Sel_List_Range_Low_16*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_16_SEL_LIST_RANGE_LOW_16_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_16_SEL_LIST_RANGE_LOW_16_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_16*/


/* start of table entry Sel_List_Range_Low_17*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_17_SEL_LIST_RANGE_LOW_17_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_17_SEL_LIST_RANGE_LOW_17_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_17*/


/* start of table entry Sel_List_Range_Low_18*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_18_SEL_LIST_RANGE_LOW_18_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_18_SEL_LIST_RANGE_LOW_18_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_18*/


/* start of table entry Sel_List_Range_Low_19*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_19_SEL_LIST_RANGE_LOW_19_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_19_SEL_LIST_RANGE_LOW_19_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_19*/


/* start of table entry Sel_List_Range_Low_20*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_20_SEL_LIST_RANGE_LOW_20_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_20_SEL_LIST_RANGE_LOW_20_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_20*/


/* start of table entry Sel_List_Range_Low_21*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_21_SEL_LIST_RANGE_LOW_21_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_21_SEL_LIST_RANGE_LOW_21_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_21*/


/* start of table entry Sel_List_Range_Low_22*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_22_SEL_LIST_RANGE_LOW_22_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_22_SEL_LIST_RANGE_LOW_22_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_22*/


/* start of table entry Sel_List_Range_Low_23*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_23_SEL_LIST_RANGE_LOW_23_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_23_SEL_LIST_RANGE_LOW_23_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_23*/


/* start of table entry Sel_List_Range_Low_24*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_24_SEL_LIST_RANGE_LOW_24_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_24_SEL_LIST_RANGE_LOW_24_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_24*/


/* start of table entry Sel_List_Range_Low_25*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_25_SEL_LIST_RANGE_LOW_25_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_25_SEL_LIST_RANGE_LOW_25_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_25*/


/* start of table entry Sel_List_Range_Low_26*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_26_SEL_LIST_RANGE_LOW_26_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_26_SEL_LIST_RANGE_LOW_26_0_FIELD_SIZE          8

/* end of Sel_List_Range_Low_26*/


/* start of table entry Sel_List_Range_High_0*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_0*/


/* start of table entry Sel_List_Range_High_1*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_1_SEL_LIST_RANGE_HIGH_1_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_1_SEL_LIST_RANGE_HIGH_1_0_FIELD_SIZE          8

/* end of Sel_List_Range_High_1*/


/* start of table entry Sel_List_Range_High_2*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_2_SEL_LIST_RANGE_HIGH_2_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_2_SEL_LIST_RANGE_HIGH_2_0_FIELD_SIZE          8

/* end of Sel_List_Range_High_2*/


/* start of table entry Sel_List_Range_High_3*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_3_SEL_LIST_RANGE_HIGH_3_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_3_SEL_LIST_RANGE_HIGH_3_0_FIELD_SIZE          8

/* end of Sel_List_Range_High_3*/


/* start of table entry Sel_List_Range_High_4*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_4_SEL_LIST_RANGE_HIGH_4_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_4_SEL_LIST_RANGE_HIGH_4_0_FIELD_SIZE          8

/* end of Sel_List_Range_High_4*/


/* start of table entry Sel_List_Range_High_5*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_5_SEL_LIST_RANGE_HIGH_5_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_5_SEL_LIST_RANGE_HIGH_5_0_FIELD_SIZE          8

/* end of Sel_List_Range_High_5*/


/* start of table entry Sel_List_Range_High_6*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_6_SEL_LIST_RANGE_HIGH_6_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_6_SEL_LIST_RANGE_HIGH_6_0_FIELD_SIZE          8

/* end of Sel_List_Range_High_6*/


/* start of table entry Sel_List_Range_High_7*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_7_SEL_LIST_RANGE_HIGH_7_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_7_SEL_LIST_RANGE_HIGH_7_0_FIELD_SIZE          8

/* end of Sel_List_Range_High_7*/


/* start of table entry Sel_List_Range_High_8*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_8_SEL_LIST_RANGE_HIGH_8_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_8_SEL_LIST_RANGE_HIGH_8_0_FIELD_SIZE          8

/* end of Sel_List_Range_High_8*/


/* start of table entry Sel_List_Range_High_9*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_9_SEL_LIST_RANGE_HIGH_9_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_9_SEL_LIST_RANGE_HIGH_9_0_FIELD_SIZE          8

/* end of Sel_List_Range_High_9*/


/* start of table entry Sel_List_Range_High_10*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_10_SEL_LIST_RANGE_HIGH_10_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_10_SEL_LIST_RANGE_HIGH_10_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_10*/


/* start of table entry Sel_List_Range_High_11*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_11_SEL_LIST_RANGE_HIGH_11_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_11_SEL_LIST_RANGE_HIGH_11_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_11*/


/* start of table entry Sel_List_Range_High_12*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_12_SEL_LIST_RANGE_HIGH_12_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_12_SEL_LIST_RANGE_HIGH_12_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_12*/


/* start of table entry Sel_List_Range_High_13*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_13_SEL_LIST_RANGE_HIGH_13_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_13_SEL_LIST_RANGE_HIGH_13_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_13*/


/* start of table entry Sel_List_Range_High_14*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_14_SEL_LIST_RANGE_HIGH_14_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_14_SEL_LIST_RANGE_HIGH_14_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_14*/


/* start of table entry Sel_List_Range_High_15*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_15_SEL_LIST_RANGE_HIGH_15_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_15_SEL_LIST_RANGE_HIGH_15_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_15*/


/* start of table entry Sel_List_Range_High_16*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_16_SEL_LIST_RANGE_HIGH_16_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_16_SEL_LIST_RANGE_HIGH_16_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_16*/


/* start of table entry Sel_List_Range_High_17*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_17_SEL_LIST_RANGE_HIGH_17_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_17_SEL_LIST_RANGE_HIGH_17_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_17*/


/* start of table entry Sel_List_Range_High_18*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_18_SEL_LIST_RANGE_HIGH_18_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_18_SEL_LIST_RANGE_HIGH_18_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_18*/


/* start of table entry Sel_List_Range_High_19*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_19_SEL_LIST_RANGE_HIGH_19_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_19_SEL_LIST_RANGE_HIGH_19_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_19*/


/* start of table entry Sel_List_Range_High_20*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_20_SEL_LIST_RANGE_HIGH_20_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_20_SEL_LIST_RANGE_HIGH_20_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_20*/


/* start of table entry Sel_List_Range_High_21*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_21_SEL_LIST_RANGE_HIGH_21_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_21_SEL_LIST_RANGE_HIGH_21_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_21*/


/* start of table entry Sel_List_Range_High_22*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_22_SEL_LIST_RANGE_HIGH_22_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_22_SEL_LIST_RANGE_HIGH_22_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_22*/


/* start of table entry Sel_List_Range_High_23*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_23_SEL_LIST_RANGE_HIGH_23_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_23_SEL_LIST_RANGE_HIGH_23_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_23*/


/* start of table entry Sel_List_Range_High_24*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_24_SEL_LIST_RANGE_HIGH_24_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_24_SEL_LIST_RANGE_HIGH_24_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_24*/


/* start of table entry Sel_List_Range_High_25*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_25_SEL_LIST_RANGE_HIGH_25_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_25_SEL_LIST_RANGE_HIGH_25_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_25*/


/* start of table entry Sel_List_Range_High_26*/

#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_26_SEL_LIST_RANGE_HIGH_26_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_26_SEL_LIST_RANGE_HIGH_26_0_FIELD_SIZE            8

/* end of Sel_List_Range_High_26*/


/* start of table entry Sel_List_Enable_0*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_SIZE            1

/* end of Sel_List_Enable_0*/


/* start of table entry Sel_List_Enable_1*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_1_SEL_LIST_EN_1_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_1_SEL_LIST_EN_1_0_FIELD_SIZE          1

/* end of Sel_List_Enable_1*/


/* start of table entry Sel_List_Enable_2*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_2_SEL_LIST_EN_2_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_2_SEL_LIST_EN_2_0_FIELD_SIZE          1

/* end of Sel_List_Enable_2*/


/* start of table entry Sel_List_Enable_3*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_3_SEL_LIST_EN_3_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_3_SEL_LIST_EN_3_0_FIELD_SIZE          1

/* end of Sel_List_Enable_3*/


/* start of table entry Sel_List_Enable_4*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_4_SEL_LIST_EN_4_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_4_SEL_LIST_EN_4_0_FIELD_SIZE          1

/* end of Sel_List_Enable_4*/


/* start of table entry Sel_List_Enable_5*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_5_SEL_LIST_EN_5_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_5_SEL_LIST_EN_5_0_FIELD_SIZE          1

/* end of Sel_List_Enable_5*/


/* start of table entry Sel_List_Enable_6*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_6_SEL_LIST_EN_6_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_6_SEL_LIST_EN_6_0_FIELD_SIZE          1

/* end of Sel_List_Enable_6*/


/* start of table entry Sel_List_Enable_7*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_7_SEL_LIST_EN_7_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_7_SEL_LIST_EN_7_0_FIELD_SIZE          1

/* end of Sel_List_Enable_7*/


/* start of table entry Sel_List_Enable_8*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_8_SEL_LIST_EN_8_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_8_SEL_LIST_EN_8_0_FIELD_SIZE          1

/* end of Sel_List_Enable_8*/


/* start of table entry Sel_List_Enable_9*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_9_SEL_LIST_EN_9_0_FIELD_OFFSET            0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_9_SEL_LIST_EN_9_0_FIELD_SIZE          1

/* end of Sel_List_Enable_9*/


/* start of table entry Sel_List_Enable_10*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_10_SEL_LIST_EN_10_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_10_SEL_LIST_EN_10_0_FIELD_SIZE            1

/* end of Sel_List_Enable_10*/


/* start of table entry Sel_List_Enable_11*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_11_SEL_LIST_EN_11_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_11_SEL_LIST_EN_11_0_FIELD_SIZE            1

/* end of Sel_List_Enable_11*/


/* start of table entry Sel_List_Enable_12*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_12_SEL_LIST_EN_12_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_12_SEL_LIST_EN_12_0_FIELD_SIZE            1

/* end of Sel_List_Enable_12*/


/* start of table entry Sel_List_Enable_13*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_13_SEL_LIST_EN_13_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_13_SEL_LIST_EN_13_0_FIELD_SIZE            1

/* end of Sel_List_Enable_13*/


/* start of table entry Sel_List_Enable_14*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_14_SEL_LIST_EN_14_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_14_SEL_LIST_EN_14_0_FIELD_SIZE            1

/* end of Sel_List_Enable_14*/


/* start of table entry Sel_List_Enable_15*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_15_SEL_LIST_EN_15_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_15_SEL_LIST_EN_15_0_FIELD_SIZE            1

/* end of Sel_List_Enable_15*/


/* start of table entry Sel_List_Enable_16*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_16_SEL_LIST_EN_16_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_16_SEL_LIST_EN_16_0_FIELD_SIZE            1

/* end of Sel_List_Enable_16*/


/* start of table entry Sel_List_Enable_17*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_17_SEL_LIST_EN_17_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_17_SEL_LIST_EN_17_0_FIELD_SIZE            1

/* end of Sel_List_Enable_17*/


/* start of table entry Sel_List_Enable_18*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_18_SEL_LIST_EN_18_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_18_SEL_LIST_EN_18_0_FIELD_SIZE            1

/* end of Sel_List_Enable_18*/


/* start of table entry Sel_List_Enable_19*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_19_SEL_LIST_EN_19_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_19_SEL_LIST_EN_19_0_FIELD_SIZE            1

/* end of Sel_List_Enable_19*/


/* start of table entry Sel_List_Enable_20*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_20_SEL_LIST_EN_20_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_20_SEL_LIST_EN_20_0_FIELD_SIZE            1

/* end of Sel_List_Enable_20*/


/* start of table entry Sel_List_Enable_21*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_21_SEL_LIST_EN_21_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_21_SEL_LIST_EN_21_0_FIELD_SIZE            1

/* end of Sel_List_Enable_21*/


/* start of table entry Sel_List_Enable_22*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_22_SEL_LIST_EN_22_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_22_SEL_LIST_EN_22_0_FIELD_SIZE            1

/* end of Sel_List_Enable_22*/


/* start of table entry Sel_List_Enable_23*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_23_SEL_LIST_EN_23_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_23_SEL_LIST_EN_23_0_FIELD_SIZE            1

/* end of Sel_List_Enable_23*/


/* start of table entry Sel_List_Enable_24*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_24_SEL_LIST_EN_24_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_24_SEL_LIST_EN_24_0_FIELD_SIZE            1

/* end of Sel_List_Enable_24*/


/* start of table entry Sel_List_Enable_25*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_25_SEL_LIST_EN_25_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_25_SEL_LIST_EN_25_0_FIELD_SIZE            1

/* end of Sel_List_Enable_25*/


/* start of table entry Sel_List_Enable_26*/

#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_26_SEL_LIST_EN_26_0_FIELD_OFFSET          0


#define      TXQ_HAWK_SDQ_SEL_LIST_ENABLE_26_SEL_LIST_EN_26_0_FIELD_SIZE            1

/* end of Sel_List_Enable_26*/


/* start of table entry queue_cfg*/

#define      TXQ_HAWK_SDQ_QUEUE_CFG_QUEUE_EN_FIELD_OFFSET           0
#define      TXQ_HAWK_SDQ_QUEUE_CFG_AGING_TH_FIELD_OFFSET           1
#define      TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_OFFSET            17
#define      TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET            28
#define      TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_OFFSET           39


#define      TXQ_HAWK_SDQ_QUEUE_CFG_QUEUE_EN_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_QUEUE_CFG_AGING_TH_FIELD_SIZE         16
#define      TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE          11
#define      TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE          11
#define      TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE         11

/* end of queue_cfg*/


/* start of table entry queue_crdt_blnc*/

#define      TXQ_HAWK_SDQ_QUEUE_CRDT_BLNC_TSTMP_FIELD_OFFSET            0
#define      TXQ_HAWK_SDQ_QUEUE_CRDT_BLNC_CRDT_BLNC_FIELD_OFFSET            16


#define      TXQ_HAWK_SDQ_QUEUE_CRDT_BLNC_TSTMP_FIELD_SIZE          16
#define      TXQ_HAWK_SDQ_QUEUE_CRDT_BLNC_CRDT_BLNC_FIELD_SIZE          19

/* end of queue_crdt_blnc*/


/* start of table entry queue_elig_state*/

#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_CRDT_FIELD_OFFSET            0
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_QCN_FIELD_OFFSET         2
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_DRY_FIELD_OFFSET         3
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_INPORTCNT_FIELD_OFFSET           4
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_INSEL_FIELD_OFFSET           5
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_ENQ_FIELD_OFFSET         6
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_SELIG_FIELD_OFFSET           7
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_ELIG4SEL_FIELD_OFFSET            8
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_SELIG4DRY_FIELD_OFFSET           9
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_PFC_FIELD_OFFSET         10


#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_CRDT_FIELD_SIZE          2
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_QCN_FIELD_SIZE           1
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_DRY_FIELD_SIZE           1
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_INPORTCNT_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_INSEL_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_ENQ_FIELD_SIZE           1
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_SELIG_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_ELIG4SEL_FIELD_SIZE          1
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_SELIG4DRY_FIELD_SIZE         1
#define      TXQ_HAWK_SDQ_QUEUE_ELIG_STATE_PFC_FIELD_SIZE           1

/* end of queue_elig_state*/


/* start of table entry port_crdt_blnc*/

#define      TXQ_HAWK_SDQ_PORT_CRDT_BLNC_PORT_QCNT_FIELD_OFFSET         0
#define      TXQ_HAWK_SDQ_PORT_CRDT_BLNC_PORT_BLNC_FIELD_OFFSET         9


#define      TXQ_HAWK_SDQ_PORT_CRDT_BLNC_PORT_QCNT_FIELD_SIZE           9
#define      TXQ_HAWK_SDQ_PORT_CRDT_BLNC_PORT_BLNC_FIELD_SIZE           27

/* end of port_crdt_blnc*/


/* start of table entry sel_list_ptrs*/

#define      TXQ_HAWK_SDQ_SEL_LIST_PTRS_RD_PTR_FIELD_OFFSET         0
#define      TXQ_HAWK_SDQ_SEL_LIST_PTRS_RD_PTR_WRP_FIELD_OFFSET         8
#define      TXQ_HAWK_SDQ_SEL_LIST_PTRS_WR_PTR_FIELD_OFFSET         9
#define      TXQ_HAWK_SDQ_SEL_LIST_PTRS_WR_PTR_WRP_FIELD_OFFSET         17


#define      TXQ_HAWK_SDQ_SEL_LIST_PTRS_RD_PTR_FIELD_SIZE           8
#define      TXQ_HAWK_SDQ_SEL_LIST_PTRS_RD_PTR_WRP_FIELD_SIZE           1
#define      TXQ_HAWK_SDQ_SEL_LIST_PTRS_WR_PTR_FIELD_SIZE           8
#define      TXQ_HAWK_SDQ_SEL_LIST_PTRS_WR_PTR_WRP_FIELD_SIZE           1

/* end of sel_list_ptrs*/


#endif
