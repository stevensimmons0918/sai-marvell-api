/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssFalconTxqQfcRegFile.h
*
* DESCRIPTION:
*      Describe TXQ QFC  registers (Automaticly generated from CIDER)
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef _TXQ_QFC_REGFILE_H_
#define _TXQ_QFC_REGFILE_H_

/* start of table entry uburst_map*/

#define      TXQ_QFC_UBURST_MAP_UBURST_PROFILE_FIELD_OFFSET         0
#define      TXQ_QFC_UBURST_MAP_UBURST_ENABLE_FIELD_OFFSET          5


#define      TXQ_QFC_UBURST_MAP_UBURST_PROFILE_FIELD_SIZE           5
#define      TXQ_QFC_UBURST_MAP_UBURST_ENABLE_FIELD_SIZE            1

/* end of uburst_map*/


/* start of table entry uburst_profile_threshold*/

#define      TXQ_QFC_UBURST_PROFILE_THRESHOLD_PROFILE_THRESHOLD_VALUE_FIELD_OFFSET           0
#define      TXQ_QFC_UBURST_PROFILE_THRESHOLD_PROFILE_THRESHOLD_TRIGGER_TYPE_FIELD_OFFSET            20


#define      TXQ_QFC_UBURST_PROFILE_THRESHOLD_PROFILE_THRESHOLD_VALUE_FIELD_SIZE         20
#define      TXQ_QFC_UBURST_PROFILE_THRESHOLD_PROFILE_THRESHOLD_TRIGGER_TYPE_FIELD_SIZE          2

/* end of uburst_profile_threshold*/


/* start of table entry uburst_event_fifo*/

#define      TXQ_QFC_UBURST_EVENT_FIFO_EVENT_DATA_0_FIELD_OFFSET            0


#define      TXQ_QFC_UBURST_EVENT_FIFO_EVENT_DATA_0_FIELD_SIZE          32

/* end of uburst_event_fifo*/


/* start of table entry port_tc_pfc_threshold*/

#define      TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_GUR_THRESHOLD_FIELD_OFFSET         0
#define      TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ALPHA_FIELD_OFFSET         20
#define      TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ENABLE_FIELD_OFFSET            24


#define      TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_GUR_THRESHOLD_FIELD_SIZE           20
#define      TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ALPHA_FIELD_SIZE           4
#define      TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ENABLE_FIELD_SIZE          1

/* end of port_tc_pfc_threshold*/


/* start of table entry port_pfc_threshold*/

#define      TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_GUR_THRESHOLD_FIELD_OFFSET         0
#define      TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_ALPHA_FIELD_OFFSET         20
#define      TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_ENABLE_FIELD_OFFSET            24


#define      TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_GUR_THRESHOLD_FIELD_SIZE           20
#define      TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_ALPHA_FIELD_SIZE           4
#define      TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_ENABLE_FIELD_SIZE          1

/* end of port_pfc_threshold*/


/* start of table entry global_tc_pfc_threshold*/

#define      TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_GUR_THRESHOLD_FIELD_OFFSET           0
#define      TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ALPHA_FIELD_OFFSET           20
#define      TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ENABLE_FIELD_OFFSET          24


#define      TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_GUR_THRESHOLD_FIELD_SIZE         20
#define      TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ALPHA_FIELD_SIZE         4
#define      TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ENABLE_FIELD_SIZE            1

/* end of global_tc_pfc_threshold*/


/* start of register global_pb_limit*/

#define      TXQ_QFC_GLOBAL_PB_LIMIT_GLOBAL_PB_LIMIT_FIELD_OFFSET           0


#define      TXQ_QFC_GLOBAL_PB_LIMIT_GLOBAL_PB_LIMIT_FIELD_SIZE         20

/* end of global_pb_limit*/


/* start of table entry Port_PFC_Hysteresis_Conf*/

#define      TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_PORT_XON_OFFSET_VALUE_FIELD_OFFSET            0
#define      TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_PORT_XON_MODE_FIELD_OFFSET            10


#define      TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_PORT_XON_OFFSET_VALUE_FIELD_SIZE          10
#define      TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_PORT_XON_MODE_FIELD_SIZE          1

/* end of Port_PFC_Hysteresis_Conf*/


/* start of table entry TC_PFC_Hysteresis_Conf*/

#define      TXQ_QFC_TC_PFC_HYSTERESIS_CONF_TC_XON_OFFSET_VALUE_FIELD_OFFSET            0
#define      TXQ_QFC_TC_PFC_HYSTERESIS_CONF_TC_XON_MODE_FIELD_OFFSET            10


#define      TXQ_QFC_TC_PFC_HYSTERESIS_CONF_TC_XON_OFFSET_VALUE_FIELD_SIZE          10
#define      TXQ_QFC_TC_PFC_HYSTERESIS_CONF_TC_XON_MODE_FIELD_SIZE          1

/* end of TC_PFC_Hysteresis_Conf*/


/* start of register QFC_Interrupt_functional_Cause*/

#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_QFC_FUNCTIONAL_INTSUM_FIELD_OFFSET          0
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_FIELD_OFFSET           1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_ILLEGAL_ADDRESS_ACCESS_FIELD_OFFSET         2
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_QUEUE_FIELD_ENQ_OUT_OF_RANGE_INT_FIELD_OFFSET           3
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_QUEUE_FIELD_DEQ_OUT_OF_RANGE_INT_FIELD_OFFSET           4
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_TARGET_PORT_FIELD_ENQ_OUT_OF_RANGE_INT_FIELD_OFFSET         5
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_TARGET_PORT_FIELD_DEQ_OUT_OF_RANGE_INT_FIELD_OFFSET         6
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_SOURCE_PORT_FIELD_ENQ_OUT_OF_RANGE_INT_FIELD_OFFSET         7
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_SOURCE_PORT_FIELD_DEQ_OUT_OF_RANGE_INT_FIELD_OFFSET         8
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_QL_COUNTER_OVERFLOW_FIELD_OFFSET            9
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_QL_COUNTER_UNDERFLOW_FIELD_OFFSET           10
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PL_COUNTER_OVERFLOW_FIELD_OFFSET            11
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PL_COUNTER_UNDERFLOW_FIELD_OFFSET           12
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PFC_COUNTER_OVERFLOW_FIELD_OFFSET           13
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PFC_COUNTER_UNDERFLOW_FIELD_OFFSET          14
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_COUNTER_OVERFLOW_FIELD_OFFSET         15
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_COUNTER_UNDERFLOW_FIELD_OFFSET            16
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_READ_MALFUNCTION_INT_FIELD_OFFSET            17
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PB_SIZE_LESS_THAN_PB_USED_INT_FIELD_OFFSET          18
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PORT_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_FIELD_OFFSET           19
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_FIELD_OFFSET         20
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_AGG_BUFFER_COUNT_OVERFLOW_FIELD_OFFSET          21


#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_QFC_FUNCTIONAL_INTSUM_FIELD_SIZE            1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_ILLEGAL_ADDRESS_ACCESS_FIELD_SIZE           1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_QUEUE_FIELD_ENQ_OUT_OF_RANGE_INT_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_QUEUE_FIELD_DEQ_OUT_OF_RANGE_INT_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_TARGET_PORT_FIELD_ENQ_OUT_OF_RANGE_INT_FIELD_SIZE           1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_TARGET_PORT_FIELD_DEQ_OUT_OF_RANGE_INT_FIELD_SIZE           1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_SOURCE_PORT_FIELD_ENQ_OUT_OF_RANGE_INT_FIELD_SIZE           1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_SOURCE_PORT_FIELD_DEQ_OUT_OF_RANGE_INT_FIELD_SIZE           1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_QL_COUNTER_OVERFLOW_FIELD_SIZE          1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_QL_COUNTER_UNDERFLOW_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PL_COUNTER_OVERFLOW_FIELD_SIZE          1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PL_COUNTER_UNDERFLOW_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PFC_COUNTER_OVERFLOW_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PFC_COUNTER_UNDERFLOW_FIELD_SIZE            1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_COUNTER_OVERFLOW_FIELD_SIZE           1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_COUNTER_UNDERFLOW_FIELD_SIZE          1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_READ_MALFUNCTION_INT_FIELD_SIZE          1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PB_SIZE_LESS_THAN_PB_USED_INT_FIELD_SIZE            1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PORT_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_FIELD_SIZE           1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_CAUSE_AGG_BUFFER_COUNT_OVERFLOW_FIELD_SIZE            1

/* end of QFC_Interrupt_functional_Cause*/


/* start of register QFC_Interrupt_functional_Mask*/

#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK1_FIELD_OFFSET           1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK2_FIELD_OFFSET           2
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK3_FIELD_OFFSET           3
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK4_FIELD_OFFSET           4
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK5_FIELD_OFFSET           5
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK6_FIELD_OFFSET           6
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK7_FIELD_OFFSET           7
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK8_FIELD_OFFSET           8
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK9_FIELD_OFFSET           9
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK10_FIELD_OFFSET          10
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK11_FIELD_OFFSET          11
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK12_FIELD_OFFSET          12
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK13_FIELD_OFFSET          13
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK14_FIELD_OFFSET          14
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK15_FIELD_OFFSET          15
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK16_FIELD_OFFSET          16
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK17_FIELD_OFFSET          17
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK18_FIELD_OFFSET          18
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK19_FIELD_OFFSET          19
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK20_FIELD_OFFSET          20
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK21_FIELD_OFFSET          21


#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK1_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK2_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK3_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK4_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK5_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK6_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK7_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK8_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK9_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK10_FIELD_SIZE            1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK11_FIELD_SIZE            1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK12_FIELD_SIZE            1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK13_FIELD_SIZE            1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK14_FIELD_SIZE            1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK15_FIELD_SIZE            1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK16_FIELD_SIZE            1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK17_FIELD_SIZE            1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK18_FIELD_SIZE            1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK19_FIELD_SIZE            1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK20_FIELD_SIZE            1
#define      TXQ_QFC_QFC_INTERRUPT_FUNCTIONAL_MASK_QFC_INTERRUPT_CAUSE_MASK21_FIELD_SIZE            1

/* end of QFC_Interrupt_functional_Mask*/


/* start of register Address_Violation*/

#define      TXQ_QFC_ADDRESS_VIOLATION_ADDRESS_VIOLATION_FIELD_OFFSET           0


#define      TXQ_QFC_ADDRESS_VIOLATION_ADDRESS_VIOLATION_FIELD_SIZE         32

/* end of Address_Violation*/


/* start of register qfc_interrupt_debug_cause*/

#define      TXQ_QFC_QFC_INTERRUPT_DEBUG_CAUSE_QFC_DEBUG_INTSUM_FIELD_OFFSET            0
#define      TXQ_QFC_QFC_INTERRUPT_DEBUG_CAUSE_PFC_MSG_DROP_COUNTER_WRAPAROUND_INT_FIELD_OFFSET         1


#define      TXQ_QFC_QFC_INTERRUPT_DEBUG_CAUSE_QFC_DEBUG_INTSUM_FIELD_SIZE          1
#define      TXQ_QFC_QFC_INTERRUPT_DEBUG_CAUSE_PFC_MSG_DROP_COUNTER_WRAPAROUND_INT_FIELD_SIZE           1

/* end of qfc_interrupt_debug_cause*/


/* start of register QFC_Interrupt_Debug_Mask*/

#define      TXQ_QFC_QFC_INTERRUPT_DEBUG_MASK_QFC_DEBUG_INTERRUPT_CAUSE_MASK1_FIELD_OFFSET          1


#define      TXQ_QFC_QFC_INTERRUPT_DEBUG_MASK_QFC_DEBUG_INTERRUPT_CAUSE_MASK1_FIELD_SIZE            1

/* end of QFC_Interrupt_Debug_Mask*/


/* start of register QFC_Interrupt_Summary_Cause*/

#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_CAUSE_QFC_INTSUM_FIELD_OFFSET            0
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_CAUSE_QFC_FUNCTIONAL_INTSUM_FIELD_OFFSET         1
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_CAUSE_QFC_DEBUG_INTSUM_FIELD_OFFSET          2
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_CAUSE_HR_CROSSED_THRESHOLD_REG_INTSUM_FIELD_OFFSET           3
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_CAUSE_HR_CROSSED_THRESHOLD_REG_1_INTSUM_FIELD_OFFSET         4
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_CAUSE_HR_CROSSED_THRESHOLD_REG_2_INTSUM_FIELD_OFFSET         5


#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_CAUSE_QFC_INTSUM_FIELD_SIZE          1
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_CAUSE_QFC_FUNCTIONAL_INTSUM_FIELD_SIZE           1
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_CAUSE_QFC_DEBUG_INTSUM_FIELD_SIZE            1
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_CAUSE_HR_CROSSED_THRESHOLD_REG_INTSUM_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_CAUSE_HR_CROSSED_THRESHOLD_REG_1_INTSUM_FIELD_SIZE           1
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_CAUSE_HR_CROSSED_THRESHOLD_REG_2_INTSUM_FIELD_SIZE           1

/* end of QFC_Interrupt_Summary_Cause*/


/* start of register QFC_Interrupt_Summary_Mask*/

#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_MASK_QFC_FUNCTIONAL_INTSUM_MASK_FIELD_OFFSET         1
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_MASK_QFC_DEBUG_INTSUM_MASK_FIELD_OFFSET          2
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_MASK_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_INTSUM_MASK_FIELD_OFFSET         3
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_MASK_QFC_HR_CROSSED_THRESHOLD_REG_1_CAUSE_INTSUM_MASK_FIELD_OFFSET           4
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_MASK_QFC_HR_CROSSED_THRESHOLD_REG_2_CAUSE_INTSUM_MASK_FIELD_OFFSET           5


#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_MASK_QFC_FUNCTIONAL_INTSUM_MASK_FIELD_SIZE           1
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_MASK_QFC_DEBUG_INTSUM_MASK_FIELD_SIZE            1
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_MASK_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_INTSUM_MASK_FIELD_SIZE           1
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_MASK_QFC_HR_CROSSED_THRESHOLD_REG_1_CAUSE_INTSUM_MASK_FIELD_SIZE         1
#define      TXQ_QFC_QFC_INTERRUPT_SUMMARY_MASK_QFC_HR_CROSSED_THRESHOLD_REG_2_CAUSE_INTSUM_MASK_FIELD_SIZE         1

/* end of QFC_Interrupt_Summary_Mask*/

/* start of table entry HR_Crossed_Threshold_Reg_Cause*/

#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_HR_CROSSED_THRESHOLD_REG_INTSUM_FIELD_OFFSET            0
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_0_TC_CROSSED_HR_THRESHOLD_FIELD_OFFSET         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_TC_1_CROSSED_HR_THRESHOLD_FIELD_OFFSET         2
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_TC_2_CROSSED_HR_THRESHOLD_FIELD_OFFSET         3
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_TC_3_CROSSED_HR_THRESHOLD_FIELD_OFFSET         4
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_TC_4_CROSSED_HR_THRESHOLD_FIELD_OFFSET         5
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_TC_5_CROSSED_HR_THRESHOLD_FIELD_OFFSET         6
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_TC_6_CROSSED_HR_THRESHOLD_FIELD_OFFSET         7
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_TC_7_CROSSED_HR_THRESHOLD_FIELD_OFFSET         8
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_1_TC_CROSSED_HR_THRESHOLD_FIELD_OFFSET         9
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_1_TC_1_CROSSED_HR_THRESHOLD_FIELD_OFFSET           10
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_1_TC_2_CROSSED_HR_THRESHOLD_FIELD_OFFSET           11
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_1_TC_3_CROSSED_HR_THRESHOLD_FIELD_OFFSET           12
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_1_TC_4_CROSSED_HR_THRESHOLD_FIELD_OFFSET           13
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_1_TC_5_CROSSED_HR_THRESHOLD_FIELD_OFFSET           14
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_1_TC_6_CROSSED_HR_THRESHOLD_FIELD_OFFSET           15
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_1_TC_7_CROSSED_HR_THRESHOLD_FIELD_OFFSET           16
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_2_TC_CROSSED_HR_THRESHOLD_FIELD_OFFSET         17
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_2_TC_1_CROSSED_HR_THRESHOLD_FIELD_OFFSET           18
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_2_TC_2_CROSSED_HR_THRESHOLD_FIELD_OFFSET           19
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_2_TC_3_CROSSED_HR_THRESHOLD_FIELD_OFFSET           20
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_2_TC_4_CROSSED_HR_THRESHOLD_FIELD_OFFSET           21
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_2_TC_5_CROSSED_HR_THRESHOLD_FIELD_OFFSET           22
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_2_TC_6_CROSSED_HR_THRESHOLD_FIELD_OFFSET           23
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_2_TC_7_CROSSED_HR_THRESHOLD_FIELD_OFFSET           24


#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_HR_CROSSED_THRESHOLD_REG_INTSUM_FIELD_SIZE          1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_0_TC_CROSSED_HR_THRESHOLD_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_TC_1_CROSSED_HR_THRESHOLD_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_TC_2_CROSSED_HR_THRESHOLD_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_TC_3_CROSSED_HR_THRESHOLD_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_TC_4_CROSSED_HR_THRESHOLD_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_TC_5_CROSSED_HR_THRESHOLD_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_TC_6_CROSSED_HR_THRESHOLD_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_TC_7_CROSSED_HR_THRESHOLD_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_1_TC_CROSSED_HR_THRESHOLD_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_1_TC_1_CROSSED_HR_THRESHOLD_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_1_TC_2_CROSSED_HR_THRESHOLD_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_1_TC_3_CROSSED_HR_THRESHOLD_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_1_TC_4_CROSSED_HR_THRESHOLD_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_1_TC_5_CROSSED_HR_THRESHOLD_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_1_TC_6_CROSSED_HR_THRESHOLD_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_1_TC_7_CROSSED_HR_THRESHOLD_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_2_TC_CROSSED_HR_THRESHOLD_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_2_TC_1_CROSSED_HR_THRESHOLD_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_2_TC_2_CROSSED_HR_THRESHOLD_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_2_TC_3_CROSSED_HR_THRESHOLD_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_2_TC_4_CROSSED_HR_THRESHOLD_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_2_TC_5_CROSSED_HR_THRESHOLD_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_2_TC_6_CROSSED_HR_THRESHOLD_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_CAUSE_PORT_2_TC_7_CROSSED_HR_THRESHOLD_FIELD_SIZE         1

/* end of HR_Crossed_Threshold_Reg_Cause*/


/* start of table entry HR_Crossed_Threshold_Reg_Mask*/

#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_0_TC_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_TC_1_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET         2
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_TC_2_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET         3
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_TC_3_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET         4
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_TC_4_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET         5
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_TC_5_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET         6
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_TC_6_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET         7
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_TC_7_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET         8
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_1_TC_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET         9
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_1_TC_1_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET           10
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_1_TC_2_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET           11
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_1_TC_3_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET           12
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_1_TC_4_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET           13
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_1_TC_5_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET           14
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_1_TC_6_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET           15
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_1_TC_7_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET           16
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_2_TC_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET         17
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_2_TC_1_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET           18
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_2_TC_2_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET           19
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_2_TC_3_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET           20
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_2_TC_4_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET           21
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_2_TC_5_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET           22
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_2_TC_6_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET           23
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_2_TC_7_CROSSED_HR_THRESHOLD_MASK_FIELD_OFFSET           24


#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_0_TC_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_TC_1_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_TC_2_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_TC_3_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_TC_4_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_TC_5_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_TC_6_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_TC_7_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_1_TC_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_1_TC_1_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_1_TC_2_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_1_TC_3_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_1_TC_4_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_1_TC_5_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_1_TC_6_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_1_TC_7_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_2_TC_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE           1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_2_TC_1_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_2_TC_2_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_2_TC_3_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_2_TC_4_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_2_TC_5_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_2_TC_6_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE         1
#define      TXQ_QFC_HR_CROSSED_THRESHOLD_REG_MASK_PORT_2_TC_7_CROSSED_HR_THRESHOLD_MASK_FIELD_SIZE         1

/* end of HR_Crossed_Threshold_Reg_Mask*/


/* start of table entry local_port_source_port_config*/

#define      TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_GLOBAL_SOURCE_PORT_FIELD_OFFSET           0
#define      TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_TYPE_FIELD_OFFSET         9


#define      TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_GLOBAL_SOURCE_PORT_FIELD_SIZE         9
#define      TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_TYPE_FIELD_SIZE           2

/* end of local_port_source_port_config*/


/* start of register qfc_metal_fix*/

#define      TXQ_QFC_QFC_METAL_FIX_QFC_METAL_FIX_FIELD_OFFSET           0


#define      TXQ_QFC_QFC_METAL_FIX_QFC_METAL_FIX_FIELD_SIZE         32

/* end of qfc_metal_fix*/


/* start of register Global_PFC_conf*/

#define      TXQ_QFC_GLOBAL_PFC_CONF_GLOBAL_PFC_ENABLE_FIELD_OFFSET         0
#define      TXQ_QFC_GLOBAL_PFC_CONF_PDX_CONGESTION_PFC_ENABLE_FIELD_OFFSET         1
#define      TXQ_QFC_GLOBAL_PFC_CONF_PB_CONGESTION_PFC_ENABLE_FIELD_OFFSET          2
#define      TXQ_QFC_GLOBAL_PFC_CONF_PB_AVAILABLE_BUFFERS_FIELD_OFFSET          3
#define      TXQ_QFC_GLOBAL_PFC_CONF_IA_TC_PFC_ENABLE_FIELD_OFFSET          23
#define      TXQ_QFC_GLOBAL_PFC_CONF_HR_COUNTING_ENABLE_FIELD_OFFSET            31


#define      TXQ_QFC_GLOBAL_PFC_CONF_GLOBAL_PFC_ENABLE_FIELD_SIZE           1
#define      TXQ_QFC_GLOBAL_PFC_CONF_PDX_CONGESTION_PFC_ENABLE_FIELD_SIZE           1
#define      TXQ_QFC_GLOBAL_PFC_CONF_PB_CONGESTION_PFC_ENABLE_FIELD_SIZE            1
#define      TXQ_QFC_GLOBAL_PFC_CONF_PB_AVAILABLE_BUFFERS_FIELD_SIZE            20
#define      TXQ_QFC_GLOBAL_PFC_CONF_IA_TC_PFC_ENABLE_FIELD_SIZE            8
#define      TXQ_QFC_GLOBAL_PFC_CONF_HR_COUNTING_ENABLE_FIELD_SIZE          1

/* end of Global_PFC_conf*/

/* start of table entry Port_TC_Available_buffers_Conf*/

#define      TXQ_QFC_PORT_TC_AVAILABLE_BUFFERS_CONF_TC_AVAILABLE_BUFFERS_FIELD_OFFSET           0


#define      TXQ_QFC_PORT_TC_AVAILABLE_BUFFERS_CONF_TC_AVAILABLE_BUFFERS_FIELD_SIZE         20

/* end of Port_TC_Available_buffers_Conf*/


/* start of table entry Global_TC_Available_buffers_Conf*/

#define      TXQ_QFC_GLOBAL_TC_AVAILABLE_BUFFERS_CONF_GLOBAL_TC_AVAILABLE_BUFFERS_FIELD_OFFSET          0


#define      TXQ_QFC_GLOBAL_TC_AVAILABLE_BUFFERS_CONF_GLOBAL_TC_AVAILABLE_BUFFERS_FIELD_SIZE            20

/* end of Global_TC_Available_buffers_Conf*/


/* start of table entry Global_counters*/

#define      TXQ_QFC_GLOBAL_COUNTERS_TC_COUNTER_VAL_FIELD_OFFSET            0


#define      TXQ_QFC_GLOBAL_COUNTERS_TC_COUNTER_VAL_FIELD_SIZE          20

/* end of Global_counters*/


/* start of register PFC_messages_Drop_counter*/

#define      TXQ_QFC_PFC_MESSAGES_DROP_COUNTER_PFC_DROP_COUNTER_FIELD_OFFSET            0


#define      TXQ_QFC_PFC_MESSAGES_DROP_COUNTER_PFC_DROP_COUNTER_FIELD_SIZE          32

/* end of PFC_messages_Drop_counter*/


/* start of table entry group_port_tc_status*/

#define      TXQ_QFC_GROUP_PORT_TC_STATUS_GROUP_PORT_TC_FIELD_OFFSET            0


#define      TXQ_QFC_GROUP_PORT_TC_STATUS_GROUP_PORT_TC_FIELD_SIZE          32

/* end of group_port_tc_status*/


/* start of table entry group_ports_status*/

#define      TXQ_QFC_GROUP_PORTS_STATUS_GROUP_PORT_PFC_STATUS_FIELD_OFFSET          0


#define      TXQ_QFC_GROUP_PORTS_STATUS_GROUP_PORT_PFC_STATUS_FIELD_SIZE            4

/* end of group_ports_status*/


/* start of register global_PFC_status*/

#define      TXQ_QFC_GLOBAL_PFC_STATUS_PB_CROSSED_LIMIT_STATUS_FIELD_OFFSET         0
#define      TXQ_QFC_GLOBAL_PFC_STATUS_PDX_CONGESTION_STATUS_FIELD_OFFSET           1
#define      TXQ_QFC_GLOBAL_PFC_STATUS_GLOBAL_TC_PFC_STATUS_FIELD_OFFSET            2
#define      TXQ_QFC_GLOBAL_PFC_STATUS_IA_TC_PFC_STATUS_FIELD_OFFSET            10


#define      TXQ_QFC_GLOBAL_PFC_STATUS_PB_CROSSED_LIMIT_STATUS_FIELD_SIZE           1
#define      TXQ_QFC_GLOBAL_PFC_STATUS_PDX_CONGESTION_STATUS_FIELD_SIZE         1
#define      TXQ_QFC_GLOBAL_PFC_STATUS_GLOBAL_TC_PFC_STATUS_FIELD_SIZE          8
#define      TXQ_QFC_GLOBAL_PFC_STATUS_IA_TC_PFC_STATUS_FIELD_SIZE          8

/* end of global_PFC_status*/


/* start of register Uburst_Event_FIFO_Max_Peak*/

#define      TXQ_QFC_UBURST_EVENT_FIFO_MAX_PEAK_UBURST_EVEN_FIFO_MAX_PEAK_STATUS_FIELD_OFFSET           0


#define      TXQ_QFC_UBURST_EVENT_FIFO_MAX_PEAK_UBURST_EVEN_FIFO_MAX_PEAK_STATUS_FIELD_SIZE         7

/* end of Uburst_Event_FIFO_Max_Peak*/


/* start of register Uburst_Event_FIFO_Min_Peak*/

#define      TXQ_QFC_UBURST_EVENT_FIFO_MIN_PEAK_UBURST_EVENT_FIFO_MIN_PEAK_STATUS_FIELD_OFFSET          0


#define      TXQ_QFC_UBURST_EVENT_FIFO_MIN_PEAK_UBURST_EVENT_FIFO_MIN_PEAK_STATUS_FIELD_SIZE            7

/* end of Uburst_Event_FIFO_Min_Peak*/


/* start of register PFC_Messages_counter*/

#define      TXQ_QFC_PFC_MESSAGES_COUNTER_PFC_MESSAGES_COUNTER_FIELD_OFFSET         0


#define      TXQ_QFC_PFC_MESSAGES_COUNTER_PFC_MESSAGES_COUNTER_FIELD_SIZE           32

/* end of PFC_Messages_counter*/


/* start of register Xoff_Size_Indirect_Read_Access*/

#define      TXQ_QFC_XOFF_SIZE_INDIRECT_READ_ACCESS_XOFF_SIZE_TC_FIELD_OFFSET           0
#define      TXQ_QFC_XOFF_SIZE_INDIRECT_READ_ACCESS_XOFF_SIZE_LOCAL_PORT_FIELD_OFFSET           3
#define      TXQ_QFC_XOFF_SIZE_INDIRECT_READ_ACCESS_XOFF_SIZE_VALUE_FIELD_OFFSET            7
#define      TXQ_QFC_XOFF_SIZE_INDIRECT_READ_ACCESS_XOFF_SIZE_VALUE_VALID_FIELD_OFFSET          27


#define      TXQ_QFC_XOFF_SIZE_INDIRECT_READ_ACCESS_XOFF_SIZE_TC_FIELD_SIZE         3
#define      TXQ_QFC_XOFF_SIZE_INDIRECT_READ_ACCESS_XOFF_SIZE_LOCAL_PORT_FIELD_SIZE         4
#define      TXQ_QFC_XOFF_SIZE_INDIRECT_READ_ACCESS_XOFF_SIZE_VALUE_FIELD_SIZE          20
#define      TXQ_QFC_XOFF_SIZE_INDIRECT_READ_ACCESS_XOFF_SIZE_VALUE_VALID_FIELD_SIZE            1

/* end of Xoff_Size_Indirect_Read_Access*/


/* start of table entry Global_HR_counters*/

#define      TXQ_QFC_GLOBAL_HR_COUNTERS_GLOBAL_TC_HR_COUNTER_VAL_FIELD_OFFSET           0


#define      TXQ_QFC_GLOBAL_HR_COUNTERS_GLOBAL_TC_HR_COUNTER_VAL_FIELD_SIZE         20

/* end of Global_HR_counters*/


/* start of register HR_Counters_Indirect_Read_Access*/

#define      TXQ_QFC_HR_COUNTERS_INDIRECT_READ_ACCESS_HR_COUNTER_INDIRECT_ACCESS_TC_FIELD_OFFSET            0
#define      TXQ_QFC_HR_COUNTERS_INDIRECT_READ_ACCESS_HR_COUNTER_INDIRECT_ACCESS_LOCAL_PORT_FIELD_OFFSET            3
#define      TXQ_QFC_HR_COUNTERS_INDIRECT_READ_ACCESS_HR_COUNTER_INDIRECT_ACCESS_VAL_FIELD_OFFSET           7
#define      TXQ_QFC_HR_COUNTERS_INDIRECT_READ_ACCESS_HR_COUNTER_INDIRECT_ACCESS_VAL_VALID_FIELD_OFFSET         27


#define      TXQ_QFC_HR_COUNTERS_INDIRECT_READ_ACCESS_HR_COUNTER_INDIRECT_ACCESS_TC_FIELD_SIZE          3
#define      TXQ_QFC_HR_COUNTERS_INDIRECT_READ_ACCESS_HR_COUNTER_INDIRECT_ACCESS_LOCAL_PORT_FIELD_SIZE          4
#define      TXQ_QFC_HR_COUNTERS_INDIRECT_READ_ACCESS_HR_COUNTER_INDIRECT_ACCESS_VAL_FIELD_SIZE         20
#define      TXQ_QFC_HR_COUNTERS_INDIRECT_READ_ACCESS_HR_COUNTER_INDIRECT_ACCESS_VAL_VALID_FIELD_SIZE           1

/* end of HR_Counters_Indirect_Read_Access*/


/* start of table entry Port_HR_Counters_Threshold*/

#define      TXQ_QFC_PORT_HR_COUNTERS_THRESHOLD_PORT_HR_COUNTER_THRESHOLD_FIELD_OFFSET          0


#define      TXQ_QFC_PORT_HR_COUNTERS_THRESHOLD_PORT_HR_COUNTER_THRESHOLD_FIELD_SIZE            20

/* end of Port_HR_Counters_Threshold*/


/* start of register Port_TC_HR_Counter_Min_Max_Conf*/

#define      TXQ_QFC_PORT_TC_HR_COUNTER_MIN_MAX_CONF_MIN_MAX_HR_COUNTER_LATCHING_TC_FIELD_OFFSET            0
#define      TXQ_QFC_PORT_TC_HR_COUNTER_MIN_MAX_CONF_MIN_MAX_HR_COUNTER_LATCHING_PORT_FIELD_OFFSET          3


#define      TXQ_QFC_PORT_TC_HR_COUNTER_MIN_MAX_CONF_MIN_MAX_HR_COUNTER_LATCHING_TC_FIELD_SIZE          3
#define      TXQ_QFC_PORT_TC_HR_COUNTER_MIN_MAX_CONF_MIN_MAX_HR_COUNTER_LATCHING_PORT_FIELD_SIZE            4

/* end of Port_TC_HR_Counter_Min_Max_Conf*/


/* start of register HR_Counter_Max_Peak*/

#define      TXQ_QFC_HR_COUNTER_MAX_PEAK_HR_COUNTER_MAX_VALUE_FIELD_OFFSET          0


#define      TXQ_QFC_HR_COUNTER_MAX_PEAK_HR_COUNTER_MAX_VALUE_FIELD_SIZE            20

/* end of HR_Counter_Max_Peak*/


/* start of register HR_Counter_Min_Peak*/

#define      TXQ_QFC_HR_COUNTER_MIN_PEAK_HR_COUNTER_MIN_VALUE_FIELD_OFFSET          0


#define      TXQ_QFC_HR_COUNTER_MIN_PEAK_HR_COUNTER_MIN_VALUE_FIELD_SIZE            20

/* end of HR_Counter_Min_Peak*/

/* manual - start of register Indirect Access data*/

#define      TXQ_QFC_INDIRECT_ACCESS_DATA_VALID_FIELD_OFFSET            20

#define      TXQ_QFC_INDIRECT_ACCESS_DATA_VALID_FIELD_SIZE          1


#define      TXQ_QFC_INDIRECT_ACCESS_COUNTER_DATA_FIELD_OFFSET            0

#define      TXQ_QFC_INDIRECT_ACCESS_COUNTER_DATA_FIELD_SIZE          20

/* end of Indirect Access data*/


/* manual - start of register  uburst_event_fifo_[0]*/

#define      TXQ_QFC_UBURST_EVENT_FIFO_0_TIMESTAMP_NANO_FIELD_OFFSET            0

#define      TXQ_QFC_UBURST_EVENT_FIFO_0_TIMESTAMP_NANO_FIELD_SIZE              30

#define      TXQ_QFC_UBURST_EVENT_FIFO_0_TIMESTAMP_SECONDS_BIT_0_FIELD_OFFSET              30

#define      TXQ_QFC_UBURST_EVENT_FIFO_0_TIMESTAMP_SECONDS_BIT_0_FIELD_SIZE                 2




/* manual - end  of register  uburst_event_fifo_[0]*/


/* manual - start of register  uburst_event_fifo_[1]*/

#define      TXQ_QFC_UBURST_EVENT_FIFO_1_TIMESTAMP_SECONDS_BITS_15_1_FIELD_OFFSET               0

#define      TXQ_QFC_UBURST_EVENT_FIFO_1_TIMESTAMP_SECONDS_BITS_15_1_FIELD_SIZE                 14

#define      TXQ_QFC_UBURST_EVENT_FIFO_1_Q_ID_FIELD_OFFSET               14

#define      TXQ_QFC_UBURST_EVENT_FIFO_1_Q_ID_FIELD_SIZE                 9

#define      TXQ_QFC_UBURST_EVENT_FIFO_1_THRESHOLD_CROSSED_FIELD_OFFSET               27

#define      TXQ_QFC_UBURST_EVENT_FIFO_1_THRESHOLD_CROSSED_FIELD_SIZE                 1

#define      TXQ_QFC_UBURST_EVENT_FIFO_1_TYPE_UBURST_EVENT_FIELD_OFFSET               30

#define      TXQ_QFC_UBURST_EVENT_FIFO_1_TYPE_UBURST_EVENT_FIELD_SIZE                 1





/* manual - end  of register  uburst_event_fifo_[1]*/





#endif


