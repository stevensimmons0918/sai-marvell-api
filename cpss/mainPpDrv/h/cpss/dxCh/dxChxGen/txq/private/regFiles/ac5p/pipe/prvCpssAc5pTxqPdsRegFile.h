/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssHawkTxqPdsRegFile.h
*
* DESCRIPTION:
*      Describe TXQ PDS registers (Automaticly generated from CIDER)
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*
*******************************************************************************/
#ifndef _TXQ_HAWK_PDS_REGFILE_H_
#define _TXQ_HAWK_PDS_REGFILE_H_

/* start of register Max_PDS_size_limit_for_pdx*/

#define      TXQ_HAWK_PDS_MAX_PDS_SIZE_LIMIT_FOR_PDX_MAX_PDS_SIZE_LIMIT_FOR_PDX_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_MAX_PDS_SIZE_LIMIT_FOR_PDX_MAX_PDS_SIZE_LIMIT_FOR_PDX_FIELD_SIZE          12

/* end of Max_PDS_size_limit_for_pdx*/


/* start of table entry Queue_PDS_Profile*/

#define      TXQ_HAWK_PDS_QUEUE_PDS_PROFILE_QUEUE_PROFILE_3_0_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_QUEUE_PDS_PROFILE_QUEUE_PROFILE_3_0_FIELD_SIZE            4

/* end of Queue_PDS_Profile*/


/* start of register PDS_Metal_Fix*/

#define      TXQ_HAWK_PDS_PDS_METAL_FIX_PDS_METAL_FIX_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_PDS_METAL_FIX_PDS_METAL_FIX_FIELD_SIZE            32

/* end of PDS_Metal_Fix*/


/* start of register Global_PDS_CFG*/

#define      TXQ_HAWK_PDS_GLOBAL_PDS_CFG_DIS_FRAG_MEM_RD_BYP_FIELD_OFFSET           0
#define      TXQ_HAWK_PDS_GLOBAL_PDS_CFG_DONT_FIX_PB_ECC_ERRORS_FIELD_OFFSET            1
#define      TXQ_HAWK_PDS_GLOBAL_PDS_CFG_CANC_RX_FIFO_ST_ACCL_FIELD_OFFSET          2
#define      TXQ_HAWK_PDS_GLOBAL_PDS_CFG_CANC_TX_FIFO_ST_ACCL_FIELD_OFFSET          3
#define      TXQ_HAWK_PDS_GLOBAL_PDS_CFG_EN_FS_LATENCY_FIELD_OFFSET         4
#define      TXQ_HAWK_PDS_GLOBAL_PDS_CFG_SEL_RX_FS_LATENCY_FIELD_OFFSET         5
#define      TXQ_HAWK_PDS_GLOBAL_PDS_CFG_EN_LONG_QUEUE_LIMIT_FIELD_OFFSET           6
#define      TXQ_HAWK_PDS_GLOBAL_PDS_CFG_PB_READ_DLY_FIELD_OFFSET           7


#define      TXQ_HAWK_PDS_GLOBAL_PDS_CFG_DIS_FRAG_MEM_RD_BYP_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_GLOBAL_PDS_CFG_DONT_FIX_PB_ECC_ERRORS_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_GLOBAL_PDS_CFG_CANC_RX_FIFO_ST_ACCL_FIELD_SIZE            1
#define      TXQ_HAWK_PDS_GLOBAL_PDS_CFG_CANC_TX_FIFO_ST_ACCL_FIELD_SIZE            1
#define      TXQ_HAWK_PDS_GLOBAL_PDS_CFG_EN_FS_LATENCY_FIELD_SIZE           1
#define      TXQ_HAWK_PDS_GLOBAL_PDS_CFG_SEL_RX_FS_LATENCY_FIELD_SIZE           1
#define      TXQ_HAWK_PDS_GLOBAL_PDS_CFG_EN_LONG_QUEUE_LIMIT_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_GLOBAL_PDS_CFG_PB_READ_DLY_FIELD_SIZE         2

/* end of Global_PDS_CFG*/


/* start of register FIFOs_Limits*/

#define      TXQ_HAWK_PDS_FIFOS_LIMITS_TAIL_ANSWER_FIFO_LIMIT_FIELD_OFFSET          0
#define      TXQ_HAWK_PDS_FIFOS_LIMITS_HEAD_ANSWER_FIFO_LIMIT_FIELD_OFFSET          4


#define      TXQ_HAWK_PDS_FIFOS_LIMITS_TAIL_ANSWER_FIFO_LIMIT_FIELD_SIZE            4
#define      TXQ_HAWK_PDS_FIFOS_LIMITS_HEAD_ANSWER_FIFO_LIMIT_FIELD_SIZE            4

/* end of FIFOs_Limits*/


/* start of register FIFOs_DQ_Disable*/

#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_DATA_READ_ORDER_FIFO_DQ_DISABLE_FIELD_OFFSET         0
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_HEAD_ANSWER_FIFO_DQ_DISABLE_FIELD_OFFSET         1
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_PB_READ_LINE_BUFF_FIFO_DQ_DISABLE_FIELD_OFFSET           2
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_PB_READ_REQ_FIFO_DQ_DISABLE_FIELD_OFFSET         3
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_PB_WRITE_REQ_FIFO_DQ_DISABLE_FIELD_OFFSET            4
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_PB_WRITE_REPLY_FIFO_DQ_DISABLE_FIELD_OFFSET          5
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_PB_READ_FIFO_STATE_RETURN_FIFO_DQ_DISABLE_FIELD_OFFSET           6
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_TAIL_ANS_FIFO_DQ_DISABLE_FIELD_OFFSET            7
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_PB_WR_LINE_BUFF_CTRL_FIFO_DQ_DISABLE_FIELD_OFFSET            8
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_PB_WR_LINE_BUFF_DATA_FIFO_DQ_DISABLE_FIELD_OFFSET            9
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_PB_FIFO_STATE_RETURN_FIFO_DQ_DISABLE_FIELD_OFFSET            10


#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_DATA_READ_ORDER_FIFO_DQ_DISABLE_FIELD_SIZE           1
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_HEAD_ANSWER_FIFO_DQ_DISABLE_FIELD_SIZE           1
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_PB_READ_LINE_BUFF_FIFO_DQ_DISABLE_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_PB_READ_REQ_FIFO_DQ_DISABLE_FIELD_SIZE           1
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_PB_WRITE_REQ_FIFO_DQ_DISABLE_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_PB_WRITE_REPLY_FIFO_DQ_DISABLE_FIELD_SIZE            1
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_PB_READ_FIFO_STATE_RETURN_FIFO_DQ_DISABLE_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_TAIL_ANS_FIFO_DQ_DISABLE_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_PB_WR_LINE_BUFF_CTRL_FIFO_DQ_DISABLE_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_PB_WR_LINE_BUFF_DATA_FIFO_DQ_DISABLE_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_FIFOS_DQ_DISABLE_PB_FIFO_STATE_RETURN_FIFO_DQ_DISABLE_FIELD_SIZE          1

/* end of FIFOs_DQ_Disable*/


/* start of register Max_PDS_size_limit_for_PB*/

#define      TXQ_HAWK_PDS_MAX_PDS_SIZE_LIMIT_FOR_PB_MAX_PDS_SIZE_LIMIT_FOR_PB_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_MAX_PDS_SIZE_LIMIT_FOR_PB_MAX_PDS_SIZE_LIMIT_FOR_PB_FIELD_SIZE            12

/* end of Max_PDS_size_limit_for_PB*/


/* start of register Max_Num_Of_Long_Queues*/

#define      TXQ_HAWK_PDS_MAX_NUM_OF_LONG_QUEUES_MAX_NUM_OF_LONG_QUEUES_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_MAX_NUM_OF_LONG_QUEUES_MAX_NUM_OF_LONG_QUEUES_FIELD_SIZE          9

/* end of Max_Num_Of_Long_Queues*/


/* start of register Tail_Size_for_PB_Wr*/

#define      TXQ_HAWK_PDS_TAIL_SIZE_FOR_PB_WR_TAIL_SIZE_FOR_PB_WR_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_TAIL_SIZE_FOR_PB_WR_TAIL_SIZE_FOR_PB_WR_FIELD_SIZE            12

/* end of Tail_Size_for_PB_Wr*/


/* start of register PID_Empty_Limit_for_PDX*/

#define      TXQ_HAWK_PDS_PID_EMPTY_LIMIT_FOR_PDX_PID_EMPTY_LIMIT_FOR_PDX_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_PID_EMPTY_LIMIT_FOR_PDX_PID_EMPTY_LIMIT_FOR_PDX_FIELD_SIZE            12

/* end of PID_Empty_Limit_for_PDX*/


/* start of register PID_Empty_Limit_for_PB*/

#define      TXQ_HAWK_PDS_PID_EMPTY_LIMIT_FOR_PB_PID_EMPTY_LIMIT_FOR_PB_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_PID_EMPTY_LIMIT_FOR_PB_PID_EMPTY_LIMIT_FOR_PB_FIELD_SIZE          12

/* end of PID_Empty_Limit_for_PB*/


/* start of register NEXT_Empty_Limit_for_PDX*/

#define      TXQ_HAWK_PDS_NEXT_EMPTY_LIMIT_FOR_PDX_NEXT_EMPTY_LIMIT_FOR_PDX_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_NEXT_EMPTY_LIMIT_FOR_PDX_NEXT_EMPTY_LIMIT_FOR_PDX_FIELD_SIZE          11

/* end of NEXT_Empty_Limit_for_PDX*/


/* start of register NEXT_Empty_Limit_for_PB*/

#define      TXQ_HAWK_PDS_NEXT_EMPTY_LIMIT_FOR_PB_NEXT_EMPTY_LIMIT_FOR_PB_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_NEXT_EMPTY_LIMIT_FOR_PB_NEXT_EMPTY_LIMIT_FOR_PB_FIELD_SIZE            11

/* end of NEXT_Empty_Limit_for_PB*/


/* start of register PB_Full_Limit*/

#define      TXQ_HAWK_PDS_PB_FULL_LIMIT_PB_FULL_LIMIT_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_PB_FULL_LIMIT_PB_FULL_LIMIT_FIELD_SIZE            10

/* end of PB_Full_Limit*/


/* start of table entry Queue_Length_Adjust_Profile*/

#define      TXQ_HAWK_PDS_QUEUE_LENGTH_ADJUST_PROFILE_QUEUE_LENGTH_ADJUST_PROFILE_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_QUEUE_LENGTH_ADJUST_PROFILE_QUEUE_LENGTH_ADJUST_PROFILE_FIELD_SIZE            4

/* end of Queue_Length_Adjust_Profile*/


/* start of register PDS_Interrupt_Debug_Cause*/

#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PDS_DEBUG_INTSUM_FIELD_OFFSET           0
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_FIELD_OFFSET           1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_STATE_ORDER_FIFO_FULL_FIELD_OFFSET          2
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_DATA_ORDER_FIFO_FULL_FIELD_OFFSET           3
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_ON_THE_AIR_CNTR_FULL_INT_FIELD_OFFSET          4
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_FIELD_OFFSET            5
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_SAME_WR_ADDR_INT_FIELD_OFFSET         6
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_TOTAL_PDS_CNTR_OVRRUN_INT_FIELD_OFFSET          7
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PDS_CACHE_CNTR_OVRRUN_INT_FIELD_OFFSET          8
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PDS_FULL_INT_FIELD_OFFSET           9
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PB_READ_WITH_NO_DEALLOC_INT_FIELD_OFFSET            10
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_HEAD_RD_MASK_INT_FIELD_OFFSET          11
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_TAIL_RD_MASK_INT_FIELD_OFFSET          12
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_LENGTH_ADJUST_CONSTANT_BIGGER_THAN_THE_PACKETS_BYTE_COUNT_INT_FIELD_OFFSET          13
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_DESC_ILLEGAL_QUEUE_NUMBER_INT_FIELD_OFFSET          14
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PB_WR_NOT_PERFORMED_INT_FIELD_OFFSET            15
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PB_RD_NOT_PERFORMED_INT_FIELD_OFFSET            16


#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PDS_DEBUG_INTSUM_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_STATE_ORDER_FIFO_FULL_FIELD_SIZE            1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_DATA_ORDER_FIFO_FULL_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_ON_THE_AIR_CNTR_FULL_INT_FIELD_SIZE            1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_SAME_WR_ADDR_INT_FIELD_SIZE           1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_TOTAL_PDS_CNTR_OVRRUN_INT_FIELD_SIZE            1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PDS_CACHE_CNTR_OVRRUN_INT_FIELD_SIZE            1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PDS_FULL_INT_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PB_READ_WITH_NO_DEALLOC_INT_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_HEAD_RD_MASK_INT_FIELD_SIZE            1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_TAIL_RD_MASK_INT_FIELD_SIZE            1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_LENGTH_ADJUST_CONSTANT_BIGGER_THAN_THE_PACKETS_BYTE_COUNT_INT_FIELD_SIZE            1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_DESC_ILLEGAL_QUEUE_NUMBER_INT_FIELD_SIZE            1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PB_WR_NOT_PERFORMED_INT_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_CAUSE_PB_RD_NOT_PERFORMED_INT_FIELD_SIZE          1

/* end of PDS_Interrupt_Debug_Cause*/


/* start of register PDS_Interrupt_Debug_Mask*/

#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_0_FIELD_OFFSET            1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_1_FIELD_OFFSET            2
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_2_FIELD_OFFSET            3
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_3_FIELD_OFFSET            4
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_4_FIELD_OFFSET            5
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_5_FIELD_OFFSET            6
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_6_FIELD_OFFSET            7
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_7_FIELD_OFFSET            8
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_8_FIELD_OFFSET            9
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_9_FIELD_OFFSET            10
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_10_FIELD_OFFSET           11
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_11_FIELD_OFFSET           12
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_12_FIELD_OFFSET           13
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_13_FIELD_OFFSET           14
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_14_FIELD_OFFSET           15
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_15_FIELD_OFFSET           16


#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_0_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_1_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_2_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_3_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_4_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_5_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_6_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_7_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_8_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_9_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_10_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_11_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_12_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_13_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_14_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_DEBUG_MASK_PDS_INTERRUPT_DEBUG_CAUSE_MASK_15_FIELD_SIZE         1

/* end of PDS_Interrupt_Debug_Mask*/


/* start of register PDS_Last_Address_Violation*/

#define      TXQ_HAWK_PDS_PDS_LAST_ADDRESS_VIOLATION_LAST_ADDRESS_VIOLATION_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_PDS_LAST_ADDRESS_VIOLATION_LAST_ADDRESS_VIOLATION_FIELD_SIZE          32

/* end of PDS_Last_Address_Violation*/


/* start of register PDS_Interrupt_Summary_Mask*/

#define      TXQ_HAWK_PDS_PDS_INTERRUPT_SUMMARY_MASK_PDS_INTERRUPT_SUMMARY_MASK_1_FIELD_OFFSET          1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_SUMMARY_MASK_PDS_INTERRUPT_SUMMARY_MASK_2_FIELD_OFFSET          2


#define      TXQ_HAWK_PDS_PDS_INTERRUPT_SUMMARY_MASK_PDS_INTERRUPT_SUMMARY_MASK_1_FIELD_SIZE            1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_SUMMARY_MASK_PDS_INTERRUPT_SUMMARY_MASK_2_FIELD_SIZE            1

/* end of PDS_Interrupt_Summary_Mask*/


/* start of register PDS_Interrupt_Summary_Cause*/

#define      TXQ_HAWK_PDS_PDS_INTERRUPT_SUMMARY_CAUSE_PDS_INTSUM_FIELD_OFFSET           0
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_SUMMARY_CAUSE_PDS_FUNCTIONAL_INT_SUM_FIELD_OFFSET           1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_SUMMARY_CAUSE_PDS_DEBUG_INT_SUM_FIELD_OFFSET            2


#define      TXQ_HAWK_PDS_PDS_INTERRUPT_SUMMARY_CAUSE_PDS_INTSUM_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_SUMMARY_CAUSE_PDS_FUNCTIONAL_INT_SUM_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_SUMMARY_CAUSE_PDS_DEBUG_INT_SUM_FIELD_SIZE          1

/* end of PDS_Interrupt_Summary_Cause*/


/* start of register PDS_Interrupt_Functional_Mask*/

#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_MASK_PDS_INTERRUPT_FUNCTIONAL_CAUSE_MASK_0_FIELD_OFFSET          1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_MASK_PDS_INTERRUPT_FUNCTIONAL_CAUSE_MASK_1_FIELD_OFFSET          2
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_MASK_PDS_INTERRUPT_FUNCTIONAL_CAUSE_MASK_2_FIELD_OFFSET          3
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_MASK_PDS_INTERRUPT_FUNCTIONAL_CAUSE_MASK_3_FIELD_OFFSET          4


#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_MASK_PDS_INTERRUPT_FUNCTIONAL_CAUSE_MASK_0_FIELD_SIZE            1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_MASK_PDS_INTERRUPT_FUNCTIONAL_CAUSE_MASK_1_FIELD_SIZE            1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_MASK_PDS_INTERRUPT_FUNCTIONAL_CAUSE_MASK_2_FIELD_SIZE            1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_MASK_PDS_INTERRUPT_FUNCTIONAL_CAUSE_MASK_3_FIELD_SIZE            1

/* end of PDS_Interrupt_Functional_Mask*/


/* start of register PDS_Interrupt_Functional_Cause*/

#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PDS_FUNCTIONAL_INTSUM_FIELD_OFFSET         0
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PDS_BAD_ADDRESS_INT_FIELD_OFFSET           1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_CAUSE_OVERLAP_INT_FIELD_OFFSET           2
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_SINGLE_ERROR_INT_FIELD_OFFSET           3
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_FIELD_OFFSET           4


#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PDS_FUNCTIONAL_INTSUM_FIELD_SIZE           1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PDS_BAD_ADDRESS_INT_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_CAUSE_OVERLAP_INT_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_SINGLE_ERROR_INT_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_FIELD_SIZE         1

/* end of PDS_Interrupt_Functional_Cause*/


/* start of register Total_PDS_Counter*/

#define      TXQ_HAWK_PDS_TOTAL_PDS_COUNTER_TOTAL_PDS_COUNTER_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_TOTAL_PDS_COUNTER_TOTAL_PDS_COUNTER_FIELD_SIZE            20

/* end of Total_PDS_Counter*/


/* start of register PDS_Cache_Counter*/

#define      TXQ_HAWK_PDS_PDS_CACHE_COUNTER_PDS_CACHE_COUNTER_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_PDS_CACHE_COUNTER_PDS_CACHE_COUNTER_FIELD_SIZE            12

/* end of PDS_Cache_Counter*/


/* start of register Idle_Register*/

#define      TXQ_HAWK_PDS_IDLE_REGISTER_PDS_IS_EMPTY_FIELD_OFFSET           0
#define      TXQ_HAWK_PDS_IDLE_REGISTER_CACHE_WR_SM_IS_IDLE_FIELD_OFFSET            1
#define      TXQ_HAWK_PDS_IDLE_REGISTER_CACHE_RD_SM_IS_IDLE_FIELD_OFFSET            2
#define      TXQ_HAWK_PDS_IDLE_REGISTER_FRAG_ON_THE_AIR_COUNTERS_EMPTY_FIELD_OFFSET         3
#define      TXQ_HAWK_PDS_IDLE_REGISTER_ALL_PB_FIFOS_ARE_EMPTY_FIELD_OFFSET         4


#define      TXQ_HAWK_PDS_IDLE_REGISTER_PDS_IS_EMPTY_FIELD_SIZE         1
#define      TXQ_HAWK_PDS_IDLE_REGISTER_CACHE_WR_SM_IS_IDLE_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_IDLE_REGISTER_CACHE_RD_SM_IS_IDLE_FIELD_SIZE          1
#define      TXQ_HAWK_PDS_IDLE_REGISTER_FRAG_ON_THE_AIR_COUNTERS_EMPTY_FIELD_SIZE           1
#define      TXQ_HAWK_PDS_IDLE_REGISTER_ALL_PB_FIFOS_ARE_EMPTY_FIELD_SIZE           1

/* end of Idle_Register*/


/* start of table entry Queue_Fragment_On_The_Air_Counter*/

#define      TXQ_HAWK_PDS_QUEUE_FRAGMENT_ON_THE_AIR_COUNTER_QUEUE_FRAGMENR_ON_THE_AIR_COUNTER_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_QUEUE_FRAGMENT_ON_THE_AIR_COUNTER_QUEUE_FRAGMENR_ON_THE_AIR_COUNTER_FIELD_SIZE            5

/* end of Queue_Fragment_On_The_Air_Counter*/


/* start of register Free_Next_FIFO_Max_Peak*/

#define      TXQ_HAWK_PDS_FREE_NEXT_FIFO_MAX_PEAK_FREE_NEXT_FIFO_MAX_PEAK_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_FREE_NEXT_FIFO_MAX_PEAK_FREE_NEXT_FIFO_MAX_PEAK_FIELD_SIZE            11

/* end of Free_Next_FIFO_Max_Peak*/


/* start of register Free_Next_FIFO_Min_Peak*/

#define      TXQ_HAWK_PDS_FREE_NEXT_FIFO_MIN_PEAK_FREE_NEXT_FIFO_MIN_PEAK_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_FREE_NEXT_FIFO_MIN_PEAK_FREE_NEXT_FIFO_MIN_PEAK_FIELD_SIZE            11

/* end of Free_Next_FIFO_Min_Peak*/


/* start of register Free_PID_FIFO_Max_Peak*/

#define      TXQ_HAWK_PDS_FREE_PID_FIFO_MAX_PEAK_FREE_PID_FIFO_MAX_PEAK_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_FREE_PID_FIFO_MAX_PEAK_FREE_PID_FIFO_MAX_PEAK_FIELD_SIZE          12

/* end of Free_PID_FIFO_Max_Peak*/


/* start of register Free_PID_FIFO_Min_Peak*/

#define      TXQ_HAWK_PDS_FREE_PID_FIFO_MIN_PEAK_FREE_PID_FIFO_MIN_PEAK_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_FREE_PID_FIFO_MIN_PEAK_FREE_PID_FIFO_MIN_PEAK_FIELD_SIZE          12

/* end of Free_PID_FIFO_Min_Peak*/


/* start of register Data_Read_Order_FIFO_Max_Peak*/

#define      TXQ_HAWK_PDS_DATA_READ_ORDER_FIFO_MAX_PEAK_DATA_READ_ORDER_FIFO_MAX_PEAK_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_DATA_READ_ORDER_FIFO_MAX_PEAK_DATA_READ_ORDER_FIFO_MAX_PEAK_FIELD_SIZE            6

/* end of Data_Read_Order_FIFO_Max_Peak*/


/* start of register Head_Answer_FIFO_Max_Peak*/

#define      TXQ_HAWK_PDS_HEAD_ANSWER_FIFO_MAX_PEAK_HEAD_ANSWER_FIFO_MAX_PEAK_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_HEAD_ANSWER_FIFO_MAX_PEAK_HEAD_ANSWER_FIFO_MAX_PEAK_FIELD_SIZE            4

/* end of Head_Answer_FIFO_Max_Peak*/


/* start of register PB_Read_Line_Buff_FIFO_Max_Peak*/

#define      TXQ_HAWK_PDS_PB_READ_LINE_BUFF_FIFO_MAX_PEAK_PB_READ_LINE_BUFF_FIFO_MAX_PEAK_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_PB_READ_LINE_BUFF_FIFO_MAX_PEAK_PB_READ_LINE_BUFF_FIFO_MAX_PEAK_FIELD_SIZE            2

/* end of PB_Read_Line_Buff_FIFO_Max_Peak*/


/* start of register PB_Read_Req_FIFO_Max_Peak*/

#define      TXQ_HAWK_PDS_PB_READ_REQ_FIFO_MAX_PEAK_PB_READ_REQ_FIFO_MAX_PEAK_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_PB_READ_REQ_FIFO_MAX_PEAK_PB_READ_REQ_FIFO_MAX_PEAK_FIELD_SIZE            4

/* end of PB_Read_Req_FIFO_Max_Peak*/


/* start of register PB_Write_Req_FIFO_Max_Peak*/

#define      TXQ_HAWK_PDS_PB_WRITE_REQ_FIFO_MAX_PEAK_PB_WRITE_REQ_FIFO_MAX_PEAK_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_PB_WRITE_REQ_FIFO_MAX_PEAK_PB_WRITE_REQ_FIFO_MAX_PEAK_FIELD_SIZE          4

/* end of PB_Write_Req_FIFO_Max_Peak*/


/* start of register PB_Write_Reply_FIFO_Max_Peak*/

#define      TXQ_HAWK_PDS_PB_WRITE_REPLY_FIFO_MAX_PEAK_PB_WRITE_REPLY_FIFO_MAX_PEAK_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_PB_WRITE_REPLY_FIFO_MAX_PEAK_PB_WRITE_REPLY_FIFO_MAX_PEAK_FIELD_SIZE          6

/* end of PB_Write_Reply_FIFO_Max_Peak*/


/* start of register PB_Read_FIFO_State_Return_FIFO_Max_Peak*/

#define      TXQ_HAWK_PDS_PB_READ_FIFO_STATE_RETURN_FIFO_MAX_PEAK_PB_READ_FIFO_STATE_RETURN_FIFO_MAX_PEAK_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_PB_READ_FIFO_STATE_RETURN_FIFO_MAX_PEAK_PB_READ_FIFO_STATE_RETURN_FIFO_MAX_PEAK_FIELD_SIZE            6

/* end of PB_Read_FIFO_State_Return_FIFO_Max_Peak*/


/* start of register Tail_Ans_FIFO_Max_Peak*/

#define      TXQ_HAWK_PDS_TAIL_ANS_FIFO_MAX_PEAK_TAIL_ANS_FIFO_MAX_PEAK_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_TAIL_ANS_FIFO_MAX_PEAK_TAIL_ANS_FIFO_MAX_PEAK_FIELD_SIZE          4

/* end of Tail_Ans_FIFO_Max_Peak*/


/* start of register PB_Wr_Line_Buff_Ctrl_FIFO_Max_Peak*/

#define      TXQ_HAWK_PDS_PB_WR_LINE_BUFF_CTRL_FIFO_MAX_PEAK_PB_WR_LINE_BUFF_CTRL_FIFO_MAX_PEAK_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_PB_WR_LINE_BUFF_CTRL_FIFO_MAX_PEAK_PB_WR_LINE_BUFF_CTRL_FIFO_MAX_PEAK_FIELD_SIZE          4

/* end of PB_Wr_Line_Buff_Ctrl_FIFO_Max_Peak*/


/* start of register PB_Wr_Line_Buff_Data_FIFO_Max_Peak*/

#define      TXQ_HAWK_PDS_PB_WR_LINE_BUFF_DATA_FIFO_MAX_PEAK_PB_WR_LINE_BUFF_DATA_FIFO_MAX_PEAK_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_PB_WR_LINE_BUFF_DATA_FIFO_MAX_PEAK_PB_WR_LINE_BUFF_DATA_FIFO_MAX_PEAK_FIELD_SIZE          4

/* end of PB_Wr_Line_Buff_Data_FIFO_Max_Peak*/


/* start of register PB_FIFO_State_Read_Order_FIFO_Max_Peak*/

#define      TXQ_HAWK_PDS_PB_FIFO_STATE_READ_ORDER_FIFO_MAX_PEAK_PB_FIFO_STATE_READ_ORDER_FIFO_MAX_PEAK_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_PB_FIFO_STATE_READ_ORDER_FIFO_MAX_PEAK_PB_FIFO_STATE_READ_ORDER_FIFO_MAX_PEAK_FIELD_SIZE          6

/* end of PB_FIFO_State_Read_Order_FIFO_Max_Peak*/


/* start of register Frag_On_The_Air_Cntr_Queue_Status*/

#define      TXQ_HAWK_PDS_FRAG_ON_THE_AIR_CNTR_QUEUE_STATUS_FRAG_ON_THE_AIR_CNTR_QUEUE_STATUS_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_FRAG_ON_THE_AIR_CNTR_QUEUE_STATUS_FRAG_ON_THE_AIR_CNTR_QUEUE_STATUS_FIELD_SIZE            8

/* end of Frag_On_The_Air_Cntr_Queue_Status*/


/* start of register Datas_BMX_Addr_Out_Of_Range_Port*/

#define      TXQ_HAWK_PDS_DATAS_BMX_ADDR_OUT_OF_RANGE_PORT_DATAS_BMX_ADDR_OUT_OF_RANGE_PORT_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_DATAS_BMX_ADDR_OUT_OF_RANGE_PORT_DATAS_BMX_ADDR_OUT_OF_RANGE_PORT_FIELD_SIZE          2

/* end of Datas_BMX_Addr_Out_Of_Range_Port*/


/* start of register PB_Write_FIFO_State_Return_FIFO_Max_Peak*/

#define      TXQ_HAWK_PDS_PB_WRITE_FIFO_STATE_RETURN_FIFO_MAX_PEAK_PB_WRITEFIFO_STATE_RETURN_FIFO_MAX_PEAK_FIELD_OFFSET         0


#define      TXQ_HAWK_PDS_PB_WRITE_FIFO_STATE_RETURN_FIFO_MAX_PEAK_PB_WRITEFIFO_STATE_RETURN_FIFO_MAX_PEAK_FIELD_SIZE           6

/* end of PB_Write_FIFO_State_Return_FIFO_Max_Peak*/


/* start of register PB_Read_Write_Order_FIFO_Max_Peak*/

#define      TXQ_HAWK_PDS_PB_READ_WRITE_ORDER_FIFO_MAX_PEAK_PB_READ_WRITE_ORDER_FIFO_MAX_PEAK_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_PB_READ_WRITE_ORDER_FIFO_MAX_PEAK_PB_READ_WRITE_ORDER_FIFO_MAX_PEAK_FIELD_SIZE            6

/* end of PB_Read_Write_Order_FIFO_Max_Peak*/


/* start of register FIFO_State_Latency_Max_Peak*/

#define      TXQ_HAWK_PDS_FIFO_STATE_LATENCY_MAX_PEAK_FIFO_STATE_LATENCY_MAX_PEAK_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_FIFO_STATE_LATENCY_MAX_PEAK_FIFO_STATE_LATENCY_MAX_PEAK_FIELD_SIZE            10

/* end of FIFO_State_Latency_Max_Peak*/


/* start of register FIFO_State_Latency_Min_Peak*/

#define      TXQ_HAWK_PDS_FIFO_STATE_LATENCY_MIN_PEAK_FIFO_STATE_LATENCY_MIN_PEAK_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_FIFO_STATE_LATENCY_MIN_PEAK_FIFO_STATE_LATENCY_MIN_PEAK_FIELD_SIZE            10

/* end of FIFO_State_Latency_Min_Peak*/


/* start of register Long_Queue_Counter*/

#define      TXQ_HAWK_PDS_LONG_QUEUE_COUNTER_LONG_QUEUE_COUNTER_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_LONG_QUEUE_COUNTER_LONG_QUEUE_COUNTER_FIELD_SIZE          9

/* end of Long_Queue_Counter*/


/* start of register Long_Queue_Count_Max_Peak*/

#define      TXQ_HAWK_PDS_LONG_QUEUE_COUNT_MAX_PEAK_LONG_QUEUE_COUNT_MAX_PEAK_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_LONG_QUEUE_COUNT_MAX_PEAK_LONG_QUEUE_COUNT_MAX_PEAK_FIELD_SIZE            9

/* end of Long_Queue_Count_Max_Peak*/


/* start of register PDS_Cache_Count_Max_Peak*/

#define      TXQ_HAWK_PDS_PDS_CACHE_COUNT_MAX_PEAK_PDS_CACHE_COUNT_MAX_PEAK_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_PDS_CACHE_COUNT_MAX_PEAK_PDS_CACHE_COUNT_MAX_PEAK_FIELD_SIZE          12

/* end of PDS_Cache_Count_Max_Peak*/


/* start of register Total_PDS_Count_Max_Peak*/

#define      TXQ_HAWK_PDS_TOTAL_PDS_COUNT_MAX_PEAK_TOTAL_PDS_COUNT_MAX_PEAK_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_TOTAL_PDS_COUNT_MAX_PEAK_TOTAL_PDS_COUNT_MAX_PEAK_FIELD_SIZE          20

/* end of Total_PDS_Count_Max_Peak*/


/* start of register Active_Queue_Counter*/

#define      TXQ_HAWK_PDS_ACTIVE_QUEUE_COUNTER_ACTIVE_QUEUE_COUNTER_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_ACTIVE_QUEUE_COUNTER_ACTIVE_QUEUE_COUNTER_FIELD_SIZE          9

/* end of Active_Queue_Counter*/


/* start of register Active_Queue_Count_Max_Peak*/

#define      TXQ_HAWK_PDS_ACTIVE_QUEUE_COUNT_MAX_PEAK_ACTIVE_QUEUE_COUNT_MAX_PEAK_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_ACTIVE_QUEUE_COUNT_MAX_PEAK_ACTIVE_QUEUE_COUNT_MAX_PEAK_FIELD_SIZE            9

/* end of Active_Queue_Count_Max_Peak*/


/* start of table entry Profile_Long_Queue_Limit*/

#define      TXQ_HAWK_PDS_PROFILE_LONG_QUEUE_LIMIT_PROFILE_LONG_QUEUE_LIMIT_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_PROFILE_LONG_QUEUE_LIMIT_PROFILE_LONG_QUEUE_LIMIT_FIELD_SIZE          12

/* end of Profile_Long_Queue_Limit*/


/* start of table entry Profile_Head_Empty_Limit*/

#define      TXQ_HAWK_PDS_PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_SIZE          12

/* end of Profile_Head_Empty_Limit*/


/* start of table entry Profile_Dequeue_Length_Adjust*/

#define      TXQ_HAWK_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_ENABLE_PROFILE_FIELD_OFFSET         0
#define      TXQ_HAWK_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_SUB_PROFILE_FIELD_OFFSET         1
#define      TXQ_HAWK_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_OFFSET          2


#define      TXQ_HAWK_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_ENABLE_PROFILE_FIELD_SIZE           1
#define      TXQ_HAWK_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_SUB_PROFILE_FIELD_SIZE           1
#define      TXQ_HAWK_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_SIZE            8

/* end of Profile_Dequeue_Length_Adjust*/


/* start of table entry Profile_Long_Queue_Enable*/

#define      TXQ_HAWK_PDS_PROFILE_LONG_QUEUE_ENABLE_PROFILE_LONG_QUEUE_ENABLE_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_PROFILE_LONG_QUEUE_ENABLE_PROFILE_LONG_QUEUE_ENABLE_FIELD_SIZE            1

/* end of Profile_Long_Queue_Enable*/


/* start of table entry Per_Queue_Counters*/

#define      TXQ_HAWK_PDS_PER_QUEUE_COUNTERS_LONG_FIELD_OFFSET          0
#define      TXQ_HAWK_PDS_PER_QUEUE_COUNTERS_FIDX_FIELD_OFFSET          1
#define      TXQ_HAWK_PDS_PER_QUEUE_COUNTERS_HEAD_COUNTER_FIELD_OFFSET          5
#define      TXQ_HAWK_PDS_PER_QUEUE_COUNTERS_FRAG_COUNTER_FIELD_OFFSET          17
#define      TXQ_HAWK_PDS_PER_QUEUE_COUNTERS_TAIL_COUNTER_FIELD_OFFSET          32


#define      TXQ_HAWK_PDS_PER_QUEUE_COUNTERS_LONG_FIELD_SIZE            1
#define      TXQ_HAWK_PDS_PER_QUEUE_COUNTERS_FIDX_FIELD_SIZE            4
#define      TXQ_HAWK_PDS_PER_QUEUE_COUNTERS_HEAD_COUNTER_FIELD_SIZE            12
#define      TXQ_HAWK_PDS_PER_QUEUE_COUNTERS_FRAG_COUNTER_FIELD_SIZE            15
#define      TXQ_HAWK_PDS_PER_QUEUE_COUNTERS_TAIL_COUNTER_FIELD_SIZE            12

/* end of Per_Queue_Counters*/


/* start of table entry Data_Storage*/

#define      TXQ_HAWK_PDS_DATA_STORAGE_DESCRIPTOR_FIELD_OFFSET          0


#define      TXQ_HAWK_PDS_DATA_STORAGE_DESCRIPTOR_FIELD_SIZE            62

/* end of Data_Storage*/


/* start of table entry PID_Table*/

#define      TXQ_HAWK_PDS_PID_TABLE_PID_FIELD_OFFSET            0


#define      TXQ_HAWK_PDS_PID_TABLE_PID_FIELD_SIZE          12

/* end of PID_Table*/


/* start of table entry NXT_Table*/

#define      TXQ_HAWK_PDS_NXT_TABLE_NEXT_FIELD_OFFSET           0


#define      TXQ_HAWK_PDS_NXT_TABLE_NEXT_FIELD_SIZE         12

/* end of NXT_Table*/


/* start of table entry Tail_Tail*/

#define      TXQ_HAWK_PDS_TAIL_TAIL_NEXT_FIELD_OFFSET           0
#define      TXQ_HAWK_PDS_TAIL_TAIL_CURRENT_FIELD_OFFSET            14


#define      TXQ_HAWK_PDS_TAIL_TAIL_NEXT_FIELD_SIZE         14
#define      TXQ_HAWK_PDS_TAIL_TAIL_CURRENT_FIELD_SIZE          14

/* end of Tail_Tail*/


/* start of table entry Tail_Head*/

#define      TXQ_HAWK_PDS_TAIL_HEAD_NEXT_FIELD_OFFSET           0
#define      TXQ_HAWK_PDS_TAIL_HEAD_CURRENT_FIELD_OFFSET            14


#define      TXQ_HAWK_PDS_TAIL_HEAD_NEXT_FIELD_SIZE         14
#define      TXQ_HAWK_PDS_TAIL_HEAD_CURRENT_FIELD_SIZE          14

/* end of Tail_Head*/


/* start of table entry Head_Tail*/

#define      TXQ_HAWK_PDS_HEAD_TAIL_NEXT_FIELD_OFFSET           0
#define      TXQ_HAWK_PDS_HEAD_TAIL_CURRENT_FIELD_OFFSET            14


#define      TXQ_HAWK_PDS_HEAD_TAIL_NEXT_FIELD_SIZE         14
#define      TXQ_HAWK_PDS_HEAD_TAIL_CURRENT_FIELD_SIZE          14

/* end of Head_Tail*/


/* start of table entry Head_Head*/

#define      TXQ_HAWK_PDS_HEAD_HEAD_NEXT_FIELD_OFFSET           0
#define      TXQ_HAWK_PDS_HEAD_HEAD_CURRENT_FIELD_OFFSET            14


#define      TXQ_HAWK_PDS_HEAD_HEAD_NEXT_FIELD_SIZE         14
#define      TXQ_HAWK_PDS_HEAD_HEAD_CURRENT_FIELD_SIZE          14

/* end of Head_Head*/


/* start of table entry Frag_Tail*/

#define      TXQ_HAWK_PDS_FRAG_TAIL_FIFO_STATE_FIELD_OFFSET         0


#define      TXQ_HAWK_PDS_FRAG_TAIL_FIFO_STATE_FIELD_SIZE           19

/* end of Frag_Tail*/


/* start of table entry Frag_Head*/

#define      TXQ_HAWK_PDS_FRAG_HEAD_FIFO_STATE_FIELD_OFFSET         0


#define      TXQ_HAWK_PDS_FRAG_HEAD_FIFO_STATE_FIELD_SIZE           18

/* end of Frag_Head*/


#endif
