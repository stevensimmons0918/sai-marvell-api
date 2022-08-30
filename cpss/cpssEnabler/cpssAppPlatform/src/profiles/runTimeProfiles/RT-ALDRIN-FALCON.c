/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file RT-ALDRIN-FALCON.c
*
* @brief Run time profiles for aldrin Pp.
*
* @version   1
********************************************************************************/

#include <profiles/cpssAppPlatformProfile.h>

static CPSS_UNI_EV_CAUSE_ENT genEventsArr_falcon[] = {

                           CPSS_PP_MISC_ILLEGAL_ADDR_E, CPSS_PP_GPP_E,
                           CPSS_PP_PORT_RX_FIFO_OVERRUN_E, CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,
                           CPSS_PP_PORT_TX_FIFO_OVERRUN_E,
                           CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E, CPSS_PP_EB_NA_FIFO_FULL_E,
                           CPSS_PP_MAC_NUM_OF_HOP_EXP_E,
                           CPSS_PP_MAC_NA_LEARNED_E, CPSS_PP_MAC_NA_NOT_LEARNED_E,
                           CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E, CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,
                           CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E, CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E, CPSS_PP_RX_CNTR_OVERFLOW_E,
                           CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E,
                           CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E, CPSS_PP_POLICER_IPFIX_ALARM_E,
                           CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,
                           CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E, CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,
                           CPSS_PP_PCL_LOOKUP_FIFO_FULL_E, CPSS_PP_CNC_WRAPAROUND_BLOCK_E,
                           CPSS_PP_SCT_RATE_LIMITER_E,
                           CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E, CPSS_PP_TTI_ACCESS_DATA_ERROR_E,
                           CPSS_PP_TX_END_E, CPSS_PP_TX_ERR_QUEUE_E,
                           CPSS_PP_DATA_INTEGRITY_ERROR_E,
                           CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E,
                           CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E, CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E,
                           CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E, CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E,
                           CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E, CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E,
                           CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E, CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E,
                           CPSS_PP_PCL_ACTION_TRIGGERED_E,
                         };

static CPSS_UNI_EV_CAUSE_ENT linkChangeEventsArr_falcon[] = {
                           CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
                           CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
                           CPSS_SRVCPU_PORT_802_3_AP_E,
                           CPSS_PP_PORT_AN_HCD_FOUND_E,
                           CPSS_SRVCPU_PORT_AP_DISABLE_E
                         };

static CPSS_UNI_EV_CAUSE_ENT auEventsArr_falcon[] = {
                           CPSS_PP_EB_AUQ_PENDING_E, CPSS_PP_EB_AUQ_ALMOST_FULL_E,
                           CPSS_PP_EB_AUQ_FULL_E, CPSS_PP_EB_AUQ_OVER_E,
                           CPSS_PP_EB_FUQ_PENDING_E, CPSS_PP_EB_FUQ_FULL_E,
                           CPSS_PP_EB_SECURITY_BREACH_UPDATE_E, CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E, CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E
                         };

static CPSS_UNI_EV_CAUSE_ENT genEventsArr_aldrin[] = {

                           CPSS_PP_MISC_ILLEGAL_ADDR_E,
                           CPSS_PP_PORT_RX_FIFO_OVERRUN_E, CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,
                           CPSS_PP_PORT_TX_FIFO_OVERRUN_E, CPSS_PP_PORT_TX_UNDERRUN_E,
                           CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E, CPSS_PP_EB_NA_FIFO_FULL_E,
                           CPSS_PP_MAC_SFLOW_E, CPSS_PP_MAC_NUM_OF_HOP_EXP_E,
                           CPSS_PP_MAC_NA_LEARNED_E, CPSS_PP_MAC_NA_NOT_LEARNED_E,
                           CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E, CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,
                           CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E, CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E, CPSS_PP_RX_CNTR_OVERFLOW_E,
                           CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E, CPSS_PP_POLICER_DATA_ERR_E,
                           CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E, CPSS_PP_POLICER_IPFIX_ALARM_E,
                           CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E, CPSS_PP_TQ_MISC_E,
                           CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E, CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,
                           CPSS_PP_PCL_LOOKUP_FIFO_FULL_E, CPSS_PP_CNC_WRAPAROUND_BLOCK_E,
                           CPSS_PP_SCT_RATE_LIMITER_E, CPSS_PP_EGRESS_SFLOW_E,
                           CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E, CPSS_PP_TTI_ACCESS_DATA_ERROR_E,
                           CPSS_PP_GTS_GLOBAL_FIFO_FULL_E, CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,
                           CPSS_PP_TX_END_E, CPSS_PP_TX_ERR_QUEUE_E,
                           CPSS_PP_CRITICAL_HW_ERROR_E, CPSS_PP_DATA_INTEGRITY_ERROR_E,
                           CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E, CPSS_PP_PORT_EEE_E,
                           CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E, CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E,
                           CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E, CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E,
                           CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E, CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E,
                           CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E, CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E,
                           CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E, CPSS_PP_PTP_TAI_GENERATION_E,
                           CPSS_PP_BM_MISC_E,
                           CPSS_PP_PORT_PTP_MIB_FRAGMENT_E
                         };

static CPSS_UNI_EV_CAUSE_ENT genEventsArr[] = {

                           CPSS_PP_MISC_ILLEGAL_ADDR_E, CPSS_PP_GPP_E,
                           CPSS_PP_PORT_SYNC_STATUS_CHANGED_E,
                           CPSS_PP_PORT_RX_FIFO_OVERRUN_E, CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,
                           CPSS_PP_PORT_TX_FIFO_OVERRUN_E, CPSS_PP_PORT_TX_UNDERRUN_E,
                           CPSS_PP_TQ_TXQ2_FLUSH_PORT_E, CPSS_PP_TQ_SNIFF_DESC_DROP_E,
                           CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E, CPSS_PP_EB_NA_FIFO_FULL_E,
                           CPSS_PP_MAC_SFLOW_E, CPSS_PP_MAC_NUM_OF_HOP_EXP_E,
                           CPSS_PP_MAC_NA_LEARNED_E, CPSS_PP_MAC_NA_NOT_LEARNED_E,
                           CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E, CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,
                           CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E, CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E, CPSS_PP_RX_CNTR_OVERFLOW_E,
                           CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E, CPSS_PP_POLICER_DATA_ERR_E,
                           CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E, CPSS_PP_POLICER_IPFIX_ALARM_E,
                           CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E, CPSS_PP_TQ_MISC_E,
                           CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E, CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,
                           CPSS_PP_PCL_LOOKUP_FIFO_FULL_E, CPSS_PP_CNC_WRAPAROUND_BLOCK_E,
                           CPSS_PP_SCT_RATE_LIMITER_E, CPSS_PP_EGRESS_SFLOW_E,
                           CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E, CPSS_PP_TTI_ACCESS_DATA_ERROR_E,
                           CPSS_PP_GTS_GLOBAL_FIFO_FULL_E, CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,
                           CPSS_PP_TX_END_E, CPSS_PP_TX_ERR_QUEUE_E,
                           CPSS_PP_CRITICAL_HW_ERROR_E, CPSS_PP_DATA_INTEGRITY_ERROR_E,
                           CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E, CPSS_PP_PORT_EEE_E,
                           CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E, CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E,
                           CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E, CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E,
                           CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E, CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E,
                           CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E, CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E,
                           CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E, CPSS_PP_PTP_TAI_GENERATION_E,
                           CPSS_PP_BM_MISC_E,
                           CPSS_PP_PORT_PTP_MIB_FRAGMENT_E
                         };

static CPSS_UNI_EV_CAUSE_ENT genEventsArr_ap[] = {

                           CPSS_PP_MISC_ILLEGAL_ADDR_E, CPSS_PP_GPP_E,
                           CPSS_PP_PORT_RX_FIFO_OVERRUN_E, CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,
                           CPSS_PP_PORT_TX_FIFO_OVERRUN_E, CPSS_PP_PORT_TX_UNDERRUN_E,
                           CPSS_PP_TQ_TXQ2_FLUSH_PORT_E, CPSS_PP_TQ_SNIFF_DESC_DROP_E,
                           CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E, CPSS_PP_EB_NA_FIFO_FULL_E,
                           CPSS_PP_MAC_SFLOW_E, CPSS_PP_MAC_NUM_OF_HOP_EXP_E,
                           CPSS_PP_MAC_NA_LEARNED_E, CPSS_PP_MAC_NA_NOT_LEARNED_E,
                           CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E, CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,
                           CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E, CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E, CPSS_PP_RX_CNTR_OVERFLOW_E,
                           CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E, CPSS_PP_POLICER_DATA_ERR_E,
                           CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E, CPSS_PP_POLICER_IPFIX_ALARM_E,
                           CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E, CPSS_PP_TQ_MISC_E,
                           CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E, CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,
                           CPSS_PP_PCL_LOOKUP_FIFO_FULL_E, CPSS_PP_CNC_WRAPAROUND_BLOCK_E,
                           CPSS_PP_SCT_RATE_LIMITER_E, CPSS_PP_EGRESS_SFLOW_E,
                           CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E, CPSS_PP_TTI_ACCESS_DATA_ERROR_E,
                           CPSS_PP_GTS_GLOBAL_FIFO_FULL_E, CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,
                           CPSS_PP_TX_END_E, CPSS_PP_TX_ERR_QUEUE_E,
                           CPSS_PP_CRITICAL_HW_ERROR_E, CPSS_PP_DATA_INTEGRITY_ERROR_E,
                           CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E, CPSS_PP_PORT_EEE_E,
                           CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E, CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E,
                           CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E, CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E,
                           CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E, CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E,
                           CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E, CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E,
                           CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E,
                           CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E, CPSS_PP_PTP_TAI_GENERATION_E,
                           CPSS_PP_BM_MISC_E,
                           CPSS_PP_PORT_PTP_MIB_FRAGMENT_E
                         };

static CPSS_UNI_EV_CAUSE_ENT linkChangeEventsArr[] = {
                           CPSS_PP_PORT_LINK_STATUS_CHANGED_E, CPSS_PP_PORT_AN_COMPLETED_E,
                           CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E, CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E,
                           CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E
                         };

static CPSS_UNI_EV_CAUSE_ENT auEventsArr[] = {
                           CPSS_PP_EB_AUQ_PENDING_E, CPSS_PP_EB_AUQ_ALMOST_FULL_E,
                           CPSS_PP_EB_AUQ_FULL_E, CPSS_PP_EB_AUQ_OVER_E,
                           CPSS_PP_EB_FUQ_PENDING_E, CPSS_PP_EB_FUQ_FULL_E,
                           CPSS_PP_EB_SECURITY_BREACH_UPDATE_E, CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E, CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E
                         };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr1[] = { CPSS_PP_RX_BUFFER_QUEUE0_E, CPSS_PP_RX_ERR_QUEUE0_E,
                                                CPSS_PP_RX_BUFFER_QUEUE1_E, CPSS_PP_RX_ERR_QUEUE1_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr2[] = { CPSS_PP_RX_BUFFER_QUEUE2_E, CPSS_PP_RX_ERR_QUEUE2_E,
                                                CPSS_PP_RX_BUFFER_QUEUE3_E, CPSS_PP_RX_ERR_QUEUE3_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr3[] = { CPSS_PP_RX_BUFFER_QUEUE4_E, CPSS_PP_RX_ERR_QUEUE4_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr4[] = { CPSS_PP_RX_BUFFER_QUEUE5_E, CPSS_PP_RX_ERR_QUEUE5_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr5[] = { CPSS_PP_RX_BUFFER_QUEUE6_E, CPSS_PP_RX_ERR_QUEUE6_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr6[] = { CPSS_PP_RX_BUFFER_QUEUE7_E,     CPSS_PP_RX_ERR_QUEUE7_E,
                                              /*MG1*/
                                              CPSS_PP_MG1_RX_BUFFER_QUEUE0_E
                                              ,CPSS_PP_MG1_RX_BUFFER_QUEUE1_E
                                              ,CPSS_PP_MG1_RX_BUFFER_QUEUE2_E
                                              ,CPSS_PP_MG1_RX_BUFFER_QUEUE3_E
                                              ,CPSS_PP_MG1_RX_BUFFER_QUEUE4_E
                                              ,CPSS_PP_MG1_RX_BUFFER_QUEUE5_E
                                              ,CPSS_PP_MG1_RX_BUFFER_QUEUE6_E
                                              ,CPSS_PP_MG1_RX_BUFFER_QUEUE7_E
                                              ,CPSS_PP_MG1_RX_ERR_QUEUE0_E
                                              ,CPSS_PP_MG1_RX_ERR_QUEUE1_E
                                              ,CPSS_PP_MG1_RX_ERR_QUEUE2_E
                                              ,CPSS_PP_MG1_RX_ERR_QUEUE3_E
                                              ,CPSS_PP_MG1_RX_ERR_QUEUE4_E
                                              ,CPSS_PP_MG1_RX_ERR_QUEUE5_E
                                              ,CPSS_PP_MG1_RX_ERR_QUEUE6_E
                                              ,CPSS_PP_MG1_RX_ERR_QUEUE7_E
                                              /*MG2*/
                                              ,CPSS_PP_MG2_RX_BUFFER_QUEUE0_E
                                              ,CPSS_PP_MG2_RX_BUFFER_QUEUE1_E
                                              ,CPSS_PP_MG2_RX_BUFFER_QUEUE2_E
                                              ,CPSS_PP_MG2_RX_BUFFER_QUEUE3_E
                                              ,CPSS_PP_MG2_RX_BUFFER_QUEUE4_E
                                              ,CPSS_PP_MG2_RX_BUFFER_QUEUE5_E
                                              ,CPSS_PP_MG2_RX_BUFFER_QUEUE6_E
                                              ,CPSS_PP_MG2_RX_BUFFER_QUEUE7_E
                                              ,CPSS_PP_MG2_RX_ERR_QUEUE0_E
                                              ,CPSS_PP_MG2_RX_ERR_QUEUE1_E
                                              ,CPSS_PP_MG2_RX_ERR_QUEUE2_E
                                              ,CPSS_PP_MG2_RX_ERR_QUEUE3_E
                                              ,CPSS_PP_MG2_RX_ERR_QUEUE4_E
                                              ,CPSS_PP_MG2_RX_ERR_QUEUE5_E
                                              ,CPSS_PP_MG2_RX_ERR_QUEUE6_E
                                              ,CPSS_PP_MG2_RX_ERR_QUEUE7_E
                                              /*MG3*/
                                              ,CPSS_PP_MG3_RX_BUFFER_QUEUE0_E
                                              ,CPSS_PP_MG3_RX_BUFFER_QUEUE1_E
                                              ,CPSS_PP_MG3_RX_BUFFER_QUEUE2_E
                                              ,CPSS_PP_MG3_RX_BUFFER_QUEUE3_E
                                              ,CPSS_PP_MG3_RX_BUFFER_QUEUE4_E
                                              ,CPSS_PP_MG3_RX_BUFFER_QUEUE5_E
                                              ,CPSS_PP_MG3_RX_BUFFER_QUEUE6_E
                                              ,CPSS_PP_MG3_RX_BUFFER_QUEUE7_E
                                              ,CPSS_PP_MG3_RX_ERR_QUEUE0_E
                                              ,CPSS_PP_MG3_RX_ERR_QUEUE1_E
                                              ,CPSS_PP_MG3_RX_ERR_QUEUE2_E
                                              ,CPSS_PP_MG3_RX_ERR_QUEUE3_E
                                              ,CPSS_PP_MG3_RX_ERR_QUEUE4_E
                                              ,CPSS_PP_MG3_RX_ERR_QUEUE5_E
                                              ,CPSS_PP_MG3_RX_ERR_QUEUE6_E
                                              ,CPSS_PP_MG3_RX_ERR_QUEUE7_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr6_aldrin[] = { CPSS_PP_RX_BUFFER_QUEUE7_E,     CPSS_PP_RX_ERR_QUEUE7_E};


static CPSS_UNI_EV_CAUSE_ENT rxEventsArr6_12_8_Falcon[] = { CPSS_PP_RX_BUFFER_QUEUE7_E,     CPSS_PP_RX_ERR_QUEUE7_E,
                                              /*MG1*/
                                               CPSS_PP_MG1_RX_BUFFER_QUEUE0_E
                                              ,CPSS_PP_MG1_RX_BUFFER_QUEUE1_E
                                              ,CPSS_PP_MG1_RX_BUFFER_QUEUE2_E
                                              ,CPSS_PP_MG1_RX_BUFFER_QUEUE3_E
                                              ,CPSS_PP_MG1_RX_BUFFER_QUEUE4_E
                                              ,CPSS_PP_MG1_RX_BUFFER_QUEUE5_E
                                              ,CPSS_PP_MG1_RX_BUFFER_QUEUE6_E
                                              ,CPSS_PP_MG1_RX_BUFFER_QUEUE7_E
                                              ,CPSS_PP_MG1_RX_ERR_QUEUE0_E
                                              ,CPSS_PP_MG1_RX_ERR_QUEUE1_E
                                              ,CPSS_PP_MG1_RX_ERR_QUEUE2_E
                                              ,CPSS_PP_MG1_RX_ERR_QUEUE3_E
                                              ,CPSS_PP_MG1_RX_ERR_QUEUE4_E
                                              ,CPSS_PP_MG1_RX_ERR_QUEUE5_E
                                              ,CPSS_PP_MG1_RX_ERR_QUEUE6_E
                                              ,CPSS_PP_MG1_RX_ERR_QUEUE7_E
                                              /*MG2*/
                                              ,CPSS_PP_MG2_RX_BUFFER_QUEUE0_E
                                              ,CPSS_PP_MG2_RX_BUFFER_QUEUE1_E
                                              ,CPSS_PP_MG2_RX_BUFFER_QUEUE2_E
                                              ,CPSS_PP_MG2_RX_BUFFER_QUEUE3_E
                                              ,CPSS_PP_MG2_RX_BUFFER_QUEUE4_E
                                              ,CPSS_PP_MG2_RX_BUFFER_QUEUE5_E
                                              ,CPSS_PP_MG2_RX_BUFFER_QUEUE6_E
                                              ,CPSS_PP_MG2_RX_BUFFER_QUEUE7_E
                                              ,CPSS_PP_MG2_RX_ERR_QUEUE0_E
                                              ,CPSS_PP_MG2_RX_ERR_QUEUE1_E
                                              ,CPSS_PP_MG2_RX_ERR_QUEUE2_E
                                              ,CPSS_PP_MG2_RX_ERR_QUEUE3_E
                                              ,CPSS_PP_MG2_RX_ERR_QUEUE4_E
                                              ,CPSS_PP_MG2_RX_ERR_QUEUE5_E
                                              ,CPSS_PP_MG2_RX_ERR_QUEUE6_E
                                              ,CPSS_PP_MG2_RX_ERR_QUEUE7_E
                                              /*MG3*/
                                              ,CPSS_PP_MG3_RX_BUFFER_QUEUE0_E
                                              ,CPSS_PP_MG3_RX_BUFFER_QUEUE1_E
                                              ,CPSS_PP_MG3_RX_BUFFER_QUEUE2_E
                                              ,CPSS_PP_MG3_RX_BUFFER_QUEUE3_E
                                              ,CPSS_PP_MG3_RX_BUFFER_QUEUE4_E
                                              ,CPSS_PP_MG3_RX_BUFFER_QUEUE5_E
                                              ,CPSS_PP_MG3_RX_BUFFER_QUEUE6_E
                                              ,CPSS_PP_MG3_RX_BUFFER_QUEUE7_E
                                              ,CPSS_PP_MG3_RX_ERR_QUEUE0_E
                                              ,CPSS_PP_MG3_RX_ERR_QUEUE1_E
                                              ,CPSS_PP_MG3_RX_ERR_QUEUE2_E
                                              ,CPSS_PP_MG3_RX_ERR_QUEUE3_E
                                              ,CPSS_PP_MG3_RX_ERR_QUEUE4_E
                                              ,CPSS_PP_MG3_RX_ERR_QUEUE5_E
                                              ,CPSS_PP_MG3_RX_ERR_QUEUE6_E
                                              ,CPSS_PP_MG3_RX_ERR_QUEUE7_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE32_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE33_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE34_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE35_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE36_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE37_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE38_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE39_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE40_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE41_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE42_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE43_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE44_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE45_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE46_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE47_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE48_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE49_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE50_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE51_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE52_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE53_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE54_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE55_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE56_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE57_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE58_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE59_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE60_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE61_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE62_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE63_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE64_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE65_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE66_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE67_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE68_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE69_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE70_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE71_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE72_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE73_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE74_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE75_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE76_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE77_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE78_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE79_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE80_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE81_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE82_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE83_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE84_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE85_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE86_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE87_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE88_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE89_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE90_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE91_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE92_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE93_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE94_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE95_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE96_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE97_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE98_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE99_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE100_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE101_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE102_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE103_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE104_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE105_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE106_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE107_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE108_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE109_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE110_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE111_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE112_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE113_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE114_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE115_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE116_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE117_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE118_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE119_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE120_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE121_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE122_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE123_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE124_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE125_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE126_E
                                              ,CPSS_PP_RX_BUFFER_QUEUE127_E
                                              };


/* Rx Event call back function */
GT_STATUS cpssAppRefRxEventHandle
(
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
);

/* Generic Event call back function */
GT_STATUS cpssAppRefGenEventHandle
(
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
);

/* AU Event call back function */
GT_STATUS cpssAppRefAuEventHandle
(
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
);

/* Link change Event call back function */
GT_STATUS cpssAppRefLinkChangeEvHandle
(
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
);

/* Gen Event handler */
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  gen_event_handle =
{
    _SM_(eventListPtr) genEventsArr,
    _SM_(numOfEvents) sizeof(genEventsArr)/sizeof(genEventsArr[0]),
    _SM_(callbackFuncPtr) cpssAppRefGenEventHandle,
    _SM_(taskPriority) 200
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  gen_event_handle_ap =
{
    _SM_(eventListPtr) genEventsArr_ap,
    _SM_(numOfEvents) sizeof(genEventsArr_ap)/sizeof(genEventsArr_ap[0]),
    _SM_(callbackFuncPtr) cpssAppRefGenEventHandle,
    _SM_(taskPriority) 200
};

/* Gen Event handler - Aldrin*/
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  gen_event_handle_aldrin =
{
    _SM_(eventListPtr) genEventsArr_aldrin,
    _SM_(numOfEvents) sizeof(genEventsArr_aldrin)/sizeof(genEventsArr_aldrin[0]),
    _SM_(callbackFuncPtr) cpssAppRefGenEventHandle,
    _SM_(taskPriority) 200
};

/* Gen Event handler - Falcon*/
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  gen_event_handle_falcon =
{
    _SM_(eventListPtr) genEventsArr_falcon,
    _SM_(numOfEvents) sizeof(genEventsArr_falcon)/sizeof(genEventsArr_falcon[0]),
    _SM_(callbackFuncPtr) cpssAppRefGenEventHandle,
    _SM_(taskPriority) 200
};

/* RX Event handler */
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle1 =
{
    _SM_(eventListPtr) rxEventsArr1,
    _SM_(numOfEvents) sizeof(rxEventsArr1)/sizeof(rxEventsArr1[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 199
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle2 =
{
    _SM_(eventListPtr) rxEventsArr2,
    _SM_(numOfEvents) sizeof(rxEventsArr2)/sizeof(rxEventsArr2[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 198
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle3 =
{
    _SM_(eventListPtr) rxEventsArr3,
    _SM_(numOfEvents) sizeof(rxEventsArr3)/sizeof(rxEventsArr3[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 197
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle4 =
{
    _SM_(eventListPtr) rxEventsArr4,
    _SM_(numOfEvents) sizeof(rxEventsArr4)/sizeof(rxEventsArr4[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 196
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle5 =
{
    _SM_(eventListPtr) rxEventsArr5,
    _SM_(numOfEvents) sizeof(rxEventsArr5)/sizeof(rxEventsArr5[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 195
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle6 =
{
    _SM_(eventListPtr) rxEventsArr6,
    _SM_(numOfEvents) sizeof(rxEventsArr6)/sizeof(rxEventsArr6[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 194
};


CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle6_12_8=
{
    _SM_(eventListPtr) rxEventsArr6_12_8_Falcon,
    _SM_(numOfEvents) sizeof(rxEventsArr6_12_8_Falcon)/sizeof(rxEventsArr6_12_8_Falcon[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 194
};


CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle6_aldrin =
{
    _SM_(eventListPtr) rxEventsArr6_aldrin,
    _SM_(numOfEvents) sizeof(rxEventsArr6_aldrin)/sizeof(rxEventsArr6_aldrin[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 194
};

/* AU Event handler */
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  au_event_handle =
{
    _SM_(eventListPtr) auEventsArr,
    _SM_(numOfEvents) sizeof(auEventsArr)/sizeof(auEventsArr[0]),
    _SM_(callbackFuncPtr) cpssAppRefAuEventHandle,
    _SM_(taskPriority) 193
};

/* Link Change Event handler */
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  link_change_event_handle =
{
    _SM_(eventListPtr) linkChangeEventsArr,
    _SM_(numOfEvents) sizeof(linkChangeEventsArr)/sizeof(linkChangeEventsArr[0]),
    _SM_(callbackFuncPtr) cpssAppRefLinkChangeEvHandle,
    _SM_(taskPriority) 191
};

/* AU Event handler - falcon */
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  au_event_handle_falcon =
{
    _SM_(eventListPtr) auEventsArr_falcon,
    _SM_(numOfEvents) sizeof(auEventsArr_falcon)/sizeof(auEventsArr_falcon[0]),
    _SM_(callbackFuncPtr) cpssAppRefAuEventHandle,
    _SM_(taskPriority) 193
};

/* Link Change Event handler - falcon */
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  link_change_event_handle_falcon =
{
    _SM_(eventListPtr) linkChangeEventsArr_falcon,
    _SM_(numOfEvents) sizeof(linkChangeEventsArr_falcon)/sizeof(linkChangeEventsArr_falcon[0]),
    _SM_(callbackFuncPtr) cpssAppRefLinkChangeEvHandle,
    _SM_(taskPriority) 191
};

/*------------------*
 * Aldrin           *
 *------------------*/
static CPSS_APP_PLATFORM_PORT_CONFIG_STC portInitlist_Aldrin[] =
{
     { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {0, 31, 1,      APP_INV_PORT_CNS},  CPSS_PORT_INTERFACE_MODE_SR_LR_E, CPSS_PORT_SPEED_10000_E,CPSS_PORT_FEC_MODE_DISABLED_E }
    ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_LIST_E,      {32,            APP_INV_PORT_CNS},  CPSS_PORT_INTERFACE_MODE_SGMII_E, CPSS_PORT_SPEED_1000_E,CPSS_PORT_FEC_MODE_DISABLED_E  }
    ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E,     {               APP_INV_PORT_CNS},  CPSS_PORT_INTERFACE_MODE_NA_E,  CPSS_PORT_SPEED_NA_E ,CPSS_PORT_FEC_MODE_DISABLED_E     }
};

static CPSS_APP_PLATFORM_PORT_CONFIG_STC portInitlist_aldrin2[] =
{
   { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {0, 20, 4, APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_KR4_E, CPSS_PORT_SPEED_100G_E,CPSS_PORT_RS_FEC_MODE_ENABLED_E }
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {24, 58, 1, APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E,CPSS_PORT_FEC_MODE_DISABLED_E }
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {64, 77, 1, APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E,CPSS_PORT_FEC_MODE_DISABLED_E }
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E,     {APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_QSGMII_E, CPSS_PORT_SPEED_NA_E,CPSS_PORT_FEC_MODE_DISABLED_E }
};

#if 0
static CPSS_APP_PLATFORM_PORT_CONFIG_STC falcon_12_8_portInitlist_512_port_mode[] =
{

   { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {0, 58, 1, APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_50000_E }
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {64, 260, 1, APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_50000_E }
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {261, 261, 1, APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E }
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E,     {APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_QSGMII_E, CPSS_PORT_SPEED_NA_E }
};
#endif
static CPSS_APP_PLATFORM_PORT_CONFIG_STC falcon_12_8_portInitlist_128_port_mode[] =
{

   { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {0, 119, 1, APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_50000_E,CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E }
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E,     {APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_QSGMII_E, CPSS_PORT_SPEED_NA_E ,CPSS_PORT_FEC_MODE_DISABLED_E}
};

static CPSS_APP_PLATFORM_PORT_CONFIG_STC falcon_12_8_portInitlist_128_port_mode_no_ports[] =
{

   { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {0, 119, 1, APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E,CPSS_PORT_FEC_MODE_DISABLED_E }
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E,     {APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_QSGMII_E, CPSS_PORT_SPEED_NA_E,CPSS_PORT_FEC_MODE_DISABLED_E }
};

static CPSS_APP_PLATFORM_PORT_CONFIG_STC falcon_6_4_portInitlist_256_port_mode[] =
{

   { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {0, 15, 1, APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_25000_E,CPSS_PORT_FEC_MODE_DISABLED_E }
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E,     {APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_QSGMII_E, CPSS_PORT_SPEED_NA_E,CPSS_PORT_FEC_MODE_DISABLED_E }
};

static CPSS_APP_PLATFORM_PORT_CONFIG_STC falcon_12_8_portInitlist_512_port_mode[] =
{

   { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {0,   60,  1, APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_50000_E, CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E }
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {64,  258, 1, APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_50000_E, CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E}
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {259, 260, 1, APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E, CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E}
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E,     { APP_INV_PORT_CNS }            , CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E   , CPSS_PORT_FEC_MODE_DISABLED_E}
};

static CPSS_APP_PLATFORM_PORT_CONFIG_STC falcon_12_8_portInitlist_512_port_mode_no_ports[] =
{

   { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {0, 60, 1, APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E,CPSS_PORT_FEC_MODE_DISABLED_E }
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {64, 258, 1, APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E,CPSS_PORT_FEC_MODE_DISABLED_E }
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {259, 259, 1, APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E ,CPSS_PORT_FEC_MODE_DISABLED_E}
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E,     {APP_INV_PORT_CNS }, CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E,CPSS_PORT_FEC_MODE_DISABLED_E }
};

CPSS_APP_PLATFORM_PORT_MANAGER_HANDLE_STC portManager_ports_param_set =
{
    _SM_(taskPriority) 500,
    _SM_(portManagerFunc) NULL
};

CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC traffic_enable_info =
{
    _SM_(devNum) 0,
    _SM_(portTypeListPtr) portInitlist_aldrin2,
    _SM_(phyMapListPtr) NULL,
    _SM_(phyMapListPtrSize) 0
};

CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC traffic_enable_info_aldrin =
{
    _SM_(devNum) 0,
    _SM_(portTypeListPtr) portInitlist_Aldrin,
    _SM_(phyMapListPtr) NULL,
    _SM_(phyMapListPtrSize) 0
};

CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC traffic_enable_info_falcon =
{
    _SM_(devNum) 0,
    _SM_(portTypeListPtr) falcon_12_8_portInitlist_128_port_mode,
    _SM_(phyMapListPtr) NULL,
    _SM_(phyMapListPtrSize) 0
};

CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC traffic_enable_info_falcon_no_ports =
{
    _SM_(devNum) 0,
    _SM_(portTypeListPtr) falcon_12_8_portInitlist_128_port_mode_no_ports,
    _SM_(phyMapListPtr) NULL,
    _SM_(phyMapListPtrSize) 0
};

CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC traffic_enable_info_falcon_belly2belly =
{
    _SM_(devNum) 0,
    _SM_(portTypeListPtr) falcon_12_8_portInitlist_512_port_mode,
    _SM_(phyMapListPtr) NULL,
    _SM_(phyMapListPtrSize) 0
};

CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC traffic_enable_info_falcon_belly2belly_no_ports =
{
    _SM_(devNum) 0,
    _SM_(portTypeListPtr) falcon_12_8_portInitlist_512_port_mode_no_ports,
    _SM_(phyMapListPtr) NULL,
    _SM_(phyMapListPtrSize) 0
};

CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC traffic_enable_info_falcon_6_4 =
{
    _SM_(devNum) 0,
    _SM_(portTypeListPtr) falcon_6_4_portInitlist_256_port_mode,
    _SM_(phyMapListPtr) NULL,
    _SM_(phyMapListPtrSize) 0
};

/*Run Time profile for generic event Handler (without AP events) */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_DB_98EX5520_GEN_EVENT_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &gen_event_handle}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/*Run Time profile for generic event Handler (with AP events) */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_DB_98EX5520_GEN_EVENT_AP_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &gen_event_handle_ap}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/*Run Time profile for generic event Handler (without AP events) - aldrin */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_DB_ALDRIN_GEN_EVENT_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &gen_event_handle_aldrin}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/*Run Time profile for generic event Handler (without AP events) -falcon*/
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_WM_FALCON_GEN_EVENT_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &gen_event_handle_falcon}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/*Run Time profile for port manager */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_PORT_MANAGER_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_PORT_MANAGER_HANDLE_E,
#ifdef ANSI_PROFILES
      {_SM_(portManagerHandlePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&portManager_ports_param_set}
#else
      {_SM_(portManagerHandlePtr) &portManager_ports_param_set}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/*Aldrin2 Application run Time profile */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_DB_98EX5520_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &au_event_handle}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &link_change_event_handle}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle1}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle2}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle3}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle4}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle5}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle6}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E,
#ifdef ANSI_PROFILES
      {_SM_(trafficEnablePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&traffic_enable_info}
#else
      {_SM_(trafficEnablePtr) &traffic_enable_info}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/* Falcon Application run Time profile */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_WM_FALCON_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &au_event_handle_falcon}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &link_change_event_handle_falcon}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle1}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle2}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle3}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle4}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle5}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle6_12_8}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E,
#ifdef ANSI_PROFILES
      {_SM_(trafficEnablePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&traffic_enable_info_falcon}
#else
      {_SM_(trafficEnablePtr) &traffic_enable_info_falcon}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/* Falcon Application run Time profile with no ports initialized */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_WM_NOPORTS_FALCON_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &au_event_handle_falcon}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &link_change_event_handle_falcon}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle1}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle2}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle3}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle4}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle5}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle6_12_8}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E,
#ifdef ANSI_PROFILES
      {_SM_(trafficEnablePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&traffic_enable_info_falcon_no_ports}
#else
      {_SM_(trafficEnablePtr) &traffic_enable_info_falcon_no_ports}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/* Falcon Application run Time profile */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_WM_FALCON_BELLY2BELLY_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &au_event_handle_falcon}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &link_change_event_handle_falcon}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle1}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle2}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle3}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle4}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle5}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle6_12_8}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E,
#ifdef ANSI_PROFILES
      {_SM_(trafficEnablePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&traffic_enable_info_falcon_belly2belly}
#else
      {_SM_(trafficEnablePtr) &traffic_enable_info_falcon_belly2belly}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/* Falcon Application run Time profile with no ports initialized */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_WM_NOPORTS_FALCON_BELLY2BELLY_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &au_event_handle_falcon}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &link_change_event_handle_falcon}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle1}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle2}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle3}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle4}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle5}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle6_12_8}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E,
#ifdef ANSI_PROFILES
      {_SM_(trafficEnablePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&traffic_enable_info_falcon_belly2belly_no_ports}
#else
      {_SM_(trafficEnablePtr) &traffic_enable_info_falcon_belly2belly_no_ports}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/*Aldrin Application run Time profile */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_DB_ALDRIN_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &au_event_handle}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &link_change_event_handle}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle1}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle2}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle3}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle4}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle5}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle6_aldrin}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E,
#ifdef ANSI_PROFILES
      {_SM_(trafficEnablePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&traffic_enable_info_aldrin}
#else
      {_SM_(trafficEnablePtr) &traffic_enable_info_aldrin}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/* Falcon 6.4 Application link change events profile */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC LINK_WM_FALCON_6_4_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &link_change_event_handle_falcon}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/* Falcon 6.4 Application rx events profile */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RX_WM_FALCON_6_4_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle1}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle2}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle3}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle4}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle5}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle6}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/* Falcon 6.4 Application run Time profile */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_WM_FALCON_6_4_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E,
#ifdef ANSI_PROFILES
      {_SM_(trafficEnablePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&traffic_enable_info_falcon_6_4}
#else
      {_SM_(trafficEnablePtr) &traffic_enable_info_falcon_6_4}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};
