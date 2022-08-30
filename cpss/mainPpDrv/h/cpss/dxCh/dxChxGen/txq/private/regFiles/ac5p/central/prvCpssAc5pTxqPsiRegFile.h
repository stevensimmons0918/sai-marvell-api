/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,     *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
* prvCpssAc5pTxqPsiRegFile.h
*
*
* DESCRIPTION:
*      Describe TXQ PSI registers (Automaticly generated from CIDER)
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef _TXQ_AC5P_PSI_REGFILE_H_
#define _TXQ_AC5P_PSI_REGFILE_H_

/* start of table entry pdq2sdq_Map*/

#define      TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_OFFSET         0
#define      TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_OFFSET          1
#define      TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_OFFSET           13
#define      TXQ_HAWK_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_OFFSET         21


#define      TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_SIZE           1
#define      TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_SIZE            11
#define      TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_SIZE         5
#define      TXQ_HAWK_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_SIZE           2

/* end of pdq2sdq_Map*/


/* start of register Credit_Value*/

#define      TXQ_HAWK_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_OFFSET            0


#define      TXQ_HAWK_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_SIZE          14

/* end of Credit_Value*/


/* start of register PSI_Interrupt_Cause*/

#define      TXQ_HAWK_PSI_PSI_INTERRUPT_CAUSE_PSI_INTSUM_FIELD_OFFSET           0
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_FIELD_OFFSET          1
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_CAUSE_MG2SNAKE_INTERRUPT_FIELD_OFFSET           2
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_CAUSE_PSI_BAD_ADDRESS_INT_FIELD_OFFSET          3
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_CAUSE_SDQ_ACC_MAP_ERR_FIELD_OFFSET          4
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_CAUSE_PDQ_ACC_MAP_ERR_FIELD_OFFSET          5


#define      TXQ_HAWK_PSI_PSI_INTERRUPT_CAUSE_PSI_INTSUM_FIELD_SIZE         1
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_FIELD_SIZE            1
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_CAUSE_MG2SNAKE_INTERRUPT_FIELD_SIZE         1
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_CAUSE_PSI_BAD_ADDRESS_INT_FIELD_SIZE            1
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_CAUSE_SDQ_ACC_MAP_ERR_FIELD_SIZE            1
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_CAUSE_PDQ_ACC_MAP_ERR_FIELD_SIZE            1

/* end of PSI_Interrupt_Cause*/


/* start of register PSI_Interrupt_Mask*/

#define      TXQ_HAWK_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_1_FIELD_OFFSET          1
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_2_FIELD_OFFSET          2
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_3_FIELD_OFFSET          3
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_4_FIELD_OFFSET          4
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_5_FIELD_OFFSET          5


#define      TXQ_HAWK_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_1_FIELD_SIZE            1
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_2_FIELD_SIZE            1
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_3_FIELD_SIZE            1
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_4_FIELD_SIZE            1
#define      TXQ_HAWK_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_5_FIELD_SIZE            1

/* end of PSI_Interrupt_Mask*/


/* start of register PSI_Last_Address_Violation*/

#define      TXQ_HAWK_PSI_PSI_LAST_ADDRESS_VIOLATION_LAST_ADDRESS_VIOLATION_FIELD_OFFSET            0


#define      TXQ_HAWK_PSI_PSI_LAST_ADDRESS_VIOLATION_LAST_ADDRESS_VIOLATION_FIELD_SIZE          32

/* end of PSI_Last_Address_Violation*/


/* start of register PSI_Metal_Fix_Register*/

#define      TXQ_HAWK_PSI_PSI_METAL_FIX_REGISTER_PSI_METAL_FIX_FIELD_OFFSET         0


#define      TXQ_HAWK_PSI_PSI_METAL_FIX_REGISTER_PSI_METAL_FIX_FIELD_SIZE           32

/* end of PSI_Metal_Fix_Register*/


/* start of register QmrSchPLenBursts*/

#define      TXQ_HAWK_PSI_QMRSCHPLENBURSTS_QMRSCHPLENBURSTS_FIELD_OFFSET            0


#define      TXQ_HAWK_PSI_QMRSCHPLENBURSTS_QMRSCHPLENBURSTS_FIELD_SIZE          10

/* end of QmrSchPLenBursts*/


/* start of register QmrSchBurstsSent*/

#define      TXQ_HAWK_PSI_QMRSCHBURSTSSENT_QMRSCHBURSTSSENT_FIELD_OFFSET            0


#define      TXQ_HAWK_PSI_QMRSCHBURSTSSENT_QMRSCHBURSTSSENT_FIELD_SIZE          6

/* end of QmrSchBurstsSent*/


/* start of register MSG_FIFO_Max_Peak*/

#define      TXQ_HAWK_PSI_MSG_FIFO_MAX_PEAK_MSG_FIFO_MAX_PEAK_FIELD_OFFSET          0


#define      TXQ_HAWK_PSI_MSG_FIFO_MAX_PEAK_MSG_FIFO_MAX_PEAK_FIELD_SIZE            12

/* end of MSG_FIFO_Max_Peak*/


/* start of register MSG_FIFO_Min_Peak*/

#define      TXQ_HAWK_PSI_MSG_FIFO_MIN_PEAK_MSG_FIFO_MIN_PEAK_FIELD_OFFSET          0


#define      TXQ_HAWK_PSI_MSG_FIFO_MIN_PEAK_MSG_FIFO_MIN_PEAK_FIELD_SIZE            12

/* end of MSG_FIFO_Min_Peak*/


/* start of register MSG_FIFO_Fill_Level*/

#define      TXQ_HAWK_PSI_MSG_FIFO_FILL_LEVEL_MSG_FIFO_FILL_LEVEL_FIELD_OFFSET          0


#define      TXQ_HAWK_PSI_MSG_FIFO_FILL_LEVEL_MSG_FIFO_FILL_LEVEL_FIELD_SIZE            12

/* end of MSG_FIFO_Fill_Level*/


/* start of register Queue_Status_Read_Request*/

#define      TXQ_HAWK_PSI_QUEUE_STATUS_READ_REQUEST_QUEUE_STATUS_READ_REQUEST_FIELD_OFFSET          0


#define      TXQ_HAWK_PSI_QUEUE_STATUS_READ_REQUEST_QUEUE_STATUS_READ_REQUEST_FIELD_SIZE            11

/* end of Queue_Status_Read_Request*/


/* start of register Queue_Status_Read_Reply*/

#define      TXQ_HAWK_PSI_QUEUE_STATUS_READ_REPLY_QUEUE_STATUS_FIELD_OFFSET         0


#define      TXQ_HAWK_PSI_QUEUE_STATUS_READ_REPLY_QUEUE_STATUS_FIELD_SIZE           1

/* end of Queue_Status_Read_Reply*/


/* start of register Debug_CFG_Register*/

#define      TXQ_HAWK_PSI_DEBUG_CFG_REGISTER_DISABLE_DEQUEUE_OF_MSG_FIFO_FIELD_OFFSET           0


#define      TXQ_HAWK_PSI_DEBUG_CFG_REGISTER_DISABLE_DEQUEUE_OF_MSG_FIFO_FIELD_SIZE         1

/* end of Debug_CFG_Register*/


/* start of register SDQ_Map_Error_Capture*/

#define      TXQ_HAWK_PSI_SDQ_MAP_ERROR_CAPTURE_SDQ_ACC_ERROR_VALID_FIELD_OFFSET            0
#define      TXQ_HAWK_PSI_SDQ_MAP_ERROR_CAPTURE_SDQ_ERROR_QUEUE_FIELD_OFFSET            1
#define      TXQ_HAWK_PSI_SDQ_MAP_ERROR_CAPTURE_SDQ_ERROR_PORT_FIELD_OFFSET         9
#define      TXQ_HAWK_PSI_SDQ_MAP_ERROR_CAPTURE_SDQ_ERROR_ID_FIELD_OFFSET           14


#define      TXQ_HAWK_PSI_SDQ_MAP_ERROR_CAPTURE_SDQ_ACC_ERROR_VALID_FIELD_SIZE          1
#define      TXQ_HAWK_PSI_SDQ_MAP_ERROR_CAPTURE_SDQ_ERROR_QUEUE_FIELD_SIZE          8
#define      TXQ_HAWK_PSI_SDQ_MAP_ERROR_CAPTURE_SDQ_ERROR_PORT_FIELD_SIZE           5
#define      TXQ_HAWK_PSI_SDQ_MAP_ERROR_CAPTURE_SDQ_ERROR_ID_FIELD_SIZE         2

/* end of SDQ_Map_Error_Capture*/


/* start of register PDQ_Map_Error_Capture*/

#define      TXQ_HAWK_PSI_PDQ_MAP_ERROR_CAPTURE_PDQ_ACC_ERROR_VALID_FIELD_OFFSET            0
#define      TXQ_HAWK_PSI_PDQ_MAP_ERROR_CAPTURE_PDQ_ERROR_QUEUE_FIELD_OFFSET            1
#define      TXQ_HAWK_PSI_PDQ_MAP_ERROR_CAPTURE_PDQ_ERROR_PORT_FIELD_OFFSET         12


#define      TXQ_HAWK_PSI_PDQ_MAP_ERROR_CAPTURE_PDQ_ACC_ERROR_VALID_FIELD_SIZE          1
#define      TXQ_HAWK_PSI_PDQ_MAP_ERROR_CAPTURE_PDQ_ERROR_QUEUE_FIELD_SIZE          11
#define      TXQ_HAWK_PSI_PDQ_MAP_ERROR_CAPTURE_PDQ_ERROR_PORT_FIELD_SIZE           7

/* end of PDQ_Map_Error_Capture*/


#endif
