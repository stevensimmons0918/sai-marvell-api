/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssDrvPpIntDefDxChHarrier.c
*
* @brief This file includes the definition and initialization of the interrupts
* init. parameters tables. -- DxCh Harrier devices
*
* @version   1
********************************************************************************
*/

#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsHarrier.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxUtilLion.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddr.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/* First macsec register id as per Harrier interrupts enum */
#define  PRV_CPSS_DXCH_HARRIER_MACSEC_REGID_FIRST_CNS (PRV_CPSS_HARRIER_EIP163E_MACSEC_TYPE_1_EIP163E_MACSEC_INSTANCE_0_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E >> 5)
#define  PRV_CPSS_DXCH_UNIT_TAI0_E     PRV_CPSS_DXCH_UNIT_TAI_E
#define  PRV_CPSS_DXCH_UNIT_RXDMA0_E   PRV_CPSS_DXCH_UNIT_RXDMA_E
#define  PRV_CPSS_DXCH_UNIT_TXDMA0_E   PRV_CPSS_DXCH_UNIT_TXDMA_E
#define  PRV_CPSS_DXCH_UNIT_TX_FIFO0_E PRV_CPSS_DXCH_UNIT_TX_FIFO_E

#define  CPSS_PP_RX_ERROR_QUEUE0_E     CPSS_PP_RX_ERR_QUEUE0_E
#define  CPSS_PP_RX_ERROR_QUEUE1_E     CPSS_PP_RX_ERR_QUEUE1_E
#define  CPSS_PP_RX_ERROR_QUEUE2_E     CPSS_PP_RX_ERR_QUEUE2_E
#define  CPSS_PP_RX_ERROR_QUEUE3_E     CPSS_PP_RX_ERR_QUEUE3_E
#define  CPSS_PP_RX_ERROR_QUEUE4_E     CPSS_PP_RX_ERR_QUEUE4_E
#define  CPSS_PP_RX_ERROR_QUEUE5_E     CPSS_PP_RX_ERR_QUEUE5_E
#define  CPSS_PP_RX_ERROR_QUEUE6_E     CPSS_PP_RX_ERR_QUEUE6_E
#define  CPSS_PP_RX_ERROR_QUEUE7_E     CPSS_PP_RX_ERR_QUEUE7_E
#define  CPSS_PP_RX_ERROR_QUEUE8_E     CPSS_PP_RX_ERR_QUEUE8_E
#define  CPSS_PP_RX_ERROR_QUEUE9_E     CPSS_PP_RX_ERR_QUEUE9_E
#define  CPSS_PP_RX_ERROR_QUEUE10_E     CPSS_PP_RX_ERR_QUEUE10_E
#define  CPSS_PP_RX_ERROR_QUEUE11_E     CPSS_PP_RX_ERR_QUEUE11_E
#define  CPSS_PP_RX_ERROR_QUEUE12_E     CPSS_PP_RX_ERR_QUEUE12_E
#define  CPSS_PP_RX_ERROR_QUEUE13_E     CPSS_PP_RX_ERR_QUEUE13_E
#define  CPSS_PP_RX_ERROR_QUEUE14_E     CPSS_PP_RX_ERR_QUEUE14_E
#define  CPSS_PP_RX_ERROR_QUEUE15_E     CPSS_PP_RX_ERR_QUEUE15_E
#define  CPSS_PP_RX_ERROR_QUEUE16_E     CPSS_PP_RX_ERR_QUEUE16_E
#define  CPSS_PP_RX_ERROR_QUEUE17_E     CPSS_PP_RX_ERR_QUEUE17_E
#define  CPSS_PP_RX_ERROR_QUEUE18_E     CPSS_PP_RX_ERR_QUEUE18_E
#define  CPSS_PP_RX_ERROR_QUEUE19_E     CPSS_PP_RX_ERR_QUEUE19_E
#define  CPSS_PP_RX_ERROR_QUEUE20_E     CPSS_PP_RX_ERR_QUEUE20_E
#define  CPSS_PP_RX_ERROR_QUEUE21_E     CPSS_PP_RX_ERR_QUEUE21_E
#define  CPSS_PP_RX_ERROR_QUEUE22_E     CPSS_PP_RX_ERR_QUEUE22_E
#define  CPSS_PP_RX_ERROR_QUEUE23_E     CPSS_PP_RX_ERR_QUEUE23_E
#define  CPSS_PP_RX_ERROR_QUEUE24_E     CPSS_PP_RX_ERR_QUEUE24_E
#define  CPSS_PP_RX_ERROR_QUEUE25_E     CPSS_PP_RX_ERR_QUEUE25_E
#define  CPSS_PP_RX_ERROR_QUEUE26_E     CPSS_PP_RX_ERR_QUEUE26_E
#define  CPSS_PP_RX_ERROR_QUEUE27_E     CPSS_PP_RX_ERR_QUEUE27_E
#define  CPSS_PP_RX_ERROR_QUEUE28_E     CPSS_PP_RX_ERR_QUEUE28_E
#define  CPSS_PP_RX_ERROR_QUEUE29_E     CPSS_PP_RX_ERR_QUEUE29_E
#define  CPSS_PP_RX_ERROR_QUEUE30_E     CPSS_PP_RX_ERR_QUEUE30_E
#define  CPSS_PP_RX_ERROR_QUEUE31_E     CPSS_PP_RX_ERR_QUEUE31_E

#define  PRV_CPSS_DXCH_UNIT_TXQ0_TAI0_E  PRV_CPSS_DXCH_UNIT_TXQS_TAI0_E
#define  PRV_CPSS_DXCH_UNIT_TXQ0_TAI1_E  PRV_CPSS_DXCH_UNIT_TXQS_TAI1_E
#define  PRV_CPSS_DXCH_UNIT_TXQ1_TAI0_E  PRV_CPSS_DXCH_UNIT_TXQS1_TAI0_E
#define  PRV_CPSS_DXCH_UNIT_TXQ1_TAI1_E  PRV_CPSS_DXCH_UNIT_TXQS1_TAI1_E

#define  PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_0_E  PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E

/* convert bit in the enum of the huge interrupts of the device into BIT_0..BIT_31 in a register */
#define DEVICE_INT_TO_BIT_MAC(deviceInterruptId)    \
            (1 << ((deviceInterruptId) & 0x1f))

#define FDB_REG_MASK_CNS    \
    (0xFFFFFFFF & ~                                                             \
        (DEVICE_INT_TO_BIT_MAC(PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AGE_VIA_TRIGGER_ENDED_E) |     \
         DEVICE_INT_TO_BIT_MAC(PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AU_PROC_COMPLETED_INT_E)))

/* unit : PRV_CPSS_DXCH_UNIT_MG_E */
#define MG_UNIT(_mgIndex) \
    (_mgIndex ? (PRV_CPSS_DXCH_UNIT_MG_0_1_E + (_mgIndex-1)) : PRV_CPSS_DXCH_UNIT_MG_E)


#define UNIT_IN_GOP(unit, index) ((unit) + (index))
#define UNIT_ID_GET(unit) ((unit) & 0xFFFF)

#define HARRIER_LMU_LATENCY_INTERRUPT_CAUSE_MAC(_threshold)                    \
        ((0x8100 ) + (0x4 * (_threshold)))

#define HARRIER_LMU_LATENCY_INTERRUPT_MASK_MAC(_threshold)                     \
        ((0x8180 ) + (0x4 * (_threshold)))


/* MAC - cause */
#define PORT_MAC_INTERRUPT_CAUSE_MAC(port)                             \
        (0x94 + ((port) * 0x18))
/* MAC - mask */
#define PORT_MAC_INTERRUPT_MASK_MAC(port)                              \
        (0x98 + ((port) * 0x18))

/* SEG MAC - cause */
#define PORT_SEG_MAC_INTERRUPT_CAUSE_MAC(port)                              \
        (0x68 + ((port) * 0x14))
/* SEG - mask */
#define PORT_SEG_MAC_INTERRUPT_MASK_MAC(port)                                 \
        (0x6c + ((port) * 0x14))

#define   mg0InternalIntsSum_SUB_TREE_MAC(bit, _mgIndex)                                            \
    /* Miscellaneous */                                                                             \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                        \
        0x00000038, 0x0000003C,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_I2C_TIME_OUT_INTERRUPT_E,         \
        PRV_CPSS_HARRIER_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_SERDES_ADDRESS_OUT_OF_RANGE_VIOLATION_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                     \
        /* HostCpuDoorbellIntSum - single register per MG accessed from MG Internal Cause0/1/2/3 */ \
        {24, GT_FALSE, MG_UNIT(_mgIndex), NULL,           \
            0x00000518, 0x0000051c,                                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
            PRV_CPSS_HARRIER_MG##_mgIndex##_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_0_E,                \
            PRV_CPSS_HARRIER_MG##_mgIndex##_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_30_E,               \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 0, NULL, NULL}

#define   mt_NODE_MAC(bit)                                                                          \
    /* MT FDBIntSum - FDB Interrupt Cause */                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_FDB_E, NULL,                                                 \
        0x000001b0, 0x000001b4,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NUM_OF_HOP_EX_P_E,                           \
        PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_ADDRESS_OUT_OF_RANGE_E,                      \
        FILLED_IN_RUNTIME_CNS, 0x0, FDB_REG_MASK_CNS,                                               \
             0, NULL, NULL}

#define   em_NODE_MAC(bit)                                                                          \
    /* EM FDBIntSum - FDB Interrupt Cause */                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EM_E, NULL,                                                  \
        0x00004000, 0x00004004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_EM_EM_INTERRUPT_CAUSE_REGISTER_INDIRECT_ADDRESS_OUT_OF_RANGE_E,               \
        PRV_CPSS_HARRIER_EM_EM_INTERRUPT_CAUSE_REGISTER_CHANNEL_1_SHM_ANSWER_FIFO_OVERFLOW_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF,                                                     \
             0, NULL, NULL}

#define HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(bit, _instance, _threshold)                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_LMU_##_instance##_E, NULL,                               \
        HARRIER_LMU_LATENCY_INTERRUPT_CAUSE_MAC(_threshold),                                           \
        HARRIER_LMU_LATENCY_INTERRUPT_MASK_MAC(_threshold),                                            \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_LMU_INSTANCE_##_instance##_N_##_threshold##_I_0_LMU_LATENCY_OVER_THRESHOLD_E, \
        PRV_CPSS_HARRIER_LMU_INSTANCE_##_instance##_N_##_threshold##_I_30_LMU_LATENCY_OVER_THRESHOLD_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define lmu_PCA_SUB_TREE_MAC(bit, _instance)                                                        \
    /* LMU Cause Summary */                                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_LMU_##_instance##_E, NULL,                               \
        0x00008230, 0x00008234,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_LMU_INSTANCE_##_instance##_LMU_CAUSE_SUMMARY_N_0_LMU_LATENCY_OVER_THRESHOLD_SUM_E,  \
        PRV_CPSS_HARRIER_LMU_INSTANCE_##_instance##_LMU_CAUSE_SUMMARY_N_16_LMU_LATENCY_OVER_THRESHOLD_SUM_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 17, NULL, NULL},                                         \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 1, _instance,  0),  \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 2, _instance,  1),  \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 3, _instance,  2),  \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 4, _instance,  3),  \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 5, _instance,  4),  \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 6, _instance,  5),  \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 7, _instance,  6),  \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 8, _instance,  7),  \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 9, _instance,  8),  \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(10, _instance,  9),  \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(11, _instance, 10),  \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(12, _instance, 11),  \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(13, _instance, 12),  \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(14, _instance, 13),  \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(15, _instance, 14),  \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(16, _instance, 15),  \
        HARRIER_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(17, _instance, 16)

#define anp_global_NODE_MAC(bit, _instance)                                              \
    /* ANP */                                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_400G_##_instance##_E, NULL,                                  \
        0x00000088, 0x0000008c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_ANP_INSTANCE_##_instance##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,      \
        PRV_CPSS_HARRIER_ANP_INSTANCE_##_instance##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define anp_port_NODE_MAC(bit, _instance, _port)                                                      \
    /* ANP */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_400G_##_instance##_E, NULL,                                  \
        0x00000100 + (_port * 0x2a4), 0x00000104 + (_port * 0x2a4),                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_ANP_INSTANCE_##_instance##_PORT_NUM_##_port##_AN_RESTART_E,                             \
        PRV_CPSS_HARRIER_ANP_INSTANCE_##_instance##_PORT_NUM_##_port##_INT_WAIT_PWRDN_TIME_OUT_E,               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define anp_port2_NODE_MAC(bit, _instance, _port)                                                  \
    /*  ANP */                                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_400G_##_instance##_E, NULL,                                  \
        0x00000108 + (_port * 0x2a4), 0x0000010c + (_port * 0x2a4),                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_ANP_INSTANCE_##_instance##_PORT_NUM_##_port##_TX_TFIFO_W_ERR_E,                \
        PRV_CPSS_HARRIER_ANP_INSTANCE_##_instance##_PORT_NUM_##_port##_RX_TFIFO_EMPTY_E,                \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define anp_SUB_TREE_MAC(bit, _instance)                                                       \
    /* ANP */                                                                                  \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_400G_##_instance##_E, NULL,                         \
        0x00000094, 0x00000098,                                                                \
        prvCpssDrvHwPpPortGroupIsrRead,                                                        \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                       \
        PRV_CPSS_HARRIER_ANP_INSTANCE_##_instance##_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_E,      \
        PRV_CPSS_HARRIER_ANP_INSTANCE_##_instance##_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_7_P_INT2_SUM_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 17, NULL, NULL},                               \
        anp_global_NODE_MAC(1, _instance),                                                     \
        anp_port_NODE_MAC(2, _instance, 0),                                                    \
        anp_port2_NODE_MAC(3, _instance, 0),                                                   \
        anp_port_NODE_MAC(4, _instance, 1),                                                    \
        anp_port2_NODE_MAC(5, _instance, 1),                                                   \
        anp_port_NODE_MAC(6, _instance, 2),                                                    \
        anp_port2_NODE_MAC(7, _instance, 2),                                                   \
        anp_port_NODE_MAC(8, _instance, 3),                                                    \
        anp_port2_NODE_MAC(9, _instance, 3),                                                   \
        anp_port_NODE_MAC(10, _instance, 4),                                                   \
        anp_port2_NODE_MAC(11, _instance, 4),                                                  \
        anp_port_NODE_MAC(12, _instance, 5),                                                   \
        anp_port2_NODE_MAC(13, _instance, 5),                                                  \
        anp_port_NODE_MAC(14, _instance, 6),                                                   \
        anp_port2_NODE_MAC(15, _instance, 6),                                                  \
        anp_port_NODE_MAC(16, _instance, 7),                                                   \
        anp_port2_NODE_MAC(17, _instance, 7)

#define anp_half_SUB_TREE_MAC(bit, _instance)                                                       \
    /* ANP */                                                                                  \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_400G_##_instance##_E, NULL,                         \
        0x00000094, 0x00000098,                                                                \
        prvCpssDrvHwPpPortGroupIsrRead,                                                        \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                       \
        PRV_CPSS_HARRIER_ANP_INSTANCE_##_instance##_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_E,      \
        PRV_CPSS_HARRIER_ANP_INSTANCE_##_instance##_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_7_P_INT2_SUM_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 9, NULL, NULL},                                \
        anp_global_NODE_MAC(1, _instance),                                                     \
        anp_port_NODE_MAC(2, _instance, 0),                                                    \
        anp_port2_NODE_MAC(3, _instance, 0),                                                   \
        anp_port_NODE_MAC(4, _instance, 1),                                                    \
        anp_port2_NODE_MAC(5, _instance, 1),                                                   \
        anp_port_NODE_MAC(6, _instance, 2),                                                    \
        anp_port2_NODE_MAC(7, _instance, 2),                                                   \
        anp_port_NODE_MAC(8, _instance, 3),                                                    \
        anp_port2_NODE_MAC(9, _instance, 3)


#define pdx_NODE_MAC(bit)                                                                           \
    /* PDX Interrupt Summary */                                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PDX_E, NULL,                                             \
        0x00001408, 0x0000140c,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_TXQ_PR_PDX_FUNCTIONAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,            \
        PRV_CPSS_HARRIER_TXQ_PR_PDX_FUNCTIONAL_INTERRUPT_CAUSE_CHANNELS_RANGE_2_DX_DESC_DROP_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pfcc_NODE_MAC(bit)                                                                          \
    /* PFCC Interrupt Summary */                                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E, NULL,                                            \
        0x00000100, 0x00000104,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_TXQ_PR_PFCC_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                         \
        PRV_CPSS_HARRIER_TXQ_PR_PFCC_INTERRUPT_CAUSE_SOURCE_PORT_COUNTER_OVERFLOW_E,                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define psi_NODE_MAC(bit)                                                                           \
    /* PSI Interrupt Summary */                                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PSI_E, NULL,                                              \
        0x0000020c, 0x00000210,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_PSI_REGS_PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_E,                           \
        PRV_CPSS_HARRIER_PSI_REGS_PSI_INTERRUPT_CAUSE_PDQ_ACCESS_MAP_ERROR_E,                          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define tcam_SUB_TREE_MAC(bit)                                                                      \
    /* TCAM Interrupt Summary */                                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TCAM_E, NULL,                                                \
        0x00501004, 0x00501000,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_TCAM_TCAM_INTERRUPTS_SUMMARY_CAUSE_TCAM_LOGIC_INTERRUPT_SUMMARY_E,         \
        PRV_CPSS_HARRIER_TCAM_TCAM_INTERRUPTS_SUMMARY_CAUSE_TCAM_LOGIC_INTERRUPT_SUMMARY_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                     \
        /* TCAM Interrupt */                                                                        \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TCAM_E, NULL,                                              \
            0x0050100C, 0x00501008,                                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
            PRV_CPSS_HARRIER_TCAM_TCAM_INTERRUPT_CAUSE_CPU_ADDRESS_OUT_OF_RANGE_E,                  \
            PRV_CPSS_HARRIER_TCAM_TCAM_INTERRUPT_CAUSE_TABLE_OVERLAPPING_E,                         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  txq_tai_NODE_MAC(bit, _txq, _tai, _instance)                                               \
    /* TXQ TAI */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ##_txq##_TAI##_tai##_E, NULL,                             \
        0x00000000, 0x00000004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_QFC_TAI_SLAVE_##_instance##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          \
        PRV_CPSS_HARRIER_QFC_TAI_SLAVE_##_instance##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  tti_tai_NODE_MAC(bit, _tai)                                                     \
    /* TTI TAI */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TTI_TAI##_tai##_E, NULL,                                     \
        0x00000000, 0x00000004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_TTI_TAI_SLAVE_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,               \
        PRV_CPSS_HARRIER_TTI_TAI_SLAVE_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  pca_tai_NODE_MAC(bit, _pca, _tai, _index)                                                  \
    /* PCA TAI */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA##_pca##_TAI##_tai##_E, NULL,                             \
        0x00000000, 0x00000004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_PCA_TAI_SLAVE_##_index##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,             \
        PRV_CPSS_HARRIER_PCA_TAI_SLAVE_##_index##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  mac_tai_NODE_MAC(bit, _instance, _tai, _index)                                              \
    /* TAI */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_400G##_instance##_TAI##_tai##_E, NULL,                        \
        0x00000000, 0x00000004,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_HARRIER_MAC_TAI_SLAVE_##_index##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,              \
        PRV_CPSS_HARRIER_MAC_TAI_SLAVE_##_index##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   eft_NODE_MAC(bit)                                                                         \
    /* eft  */                                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EGF_EFT_E, NULL,                                             \
        0x000010a0, 0x000010b0,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_EGF_EFT_INTERRUPTS_CAUSE_EGRESS_WRONG_ADDR_INT_E,                             \
        PRV_CPSS_HARRIER_EGF_EFT_INTERRUPTS_CAUSE_INC_BUS_IS_TOO_SMALL_INT_E,                          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   eoam_NODE_MAC(bit)                                                                        \
    /* egress OAM  */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EOAM_E, NULL,                                                \
        0x000000f0, 0x000000f4,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_EOAM_OAM_UNIT_INTERRUPT_CAUSE_KEEP_ALIVE_AGING_E,                             \
        PRV_CPSS_HARRIER_EOAM_OAM_UNIT_INTERRUPT_CAUSE_EXCESS_KEEPALIVE_E,                             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   epcl_NODE_MAC(bit)                                                                        \
    /* EPCL  */                                                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EPCL_E, NULL,                                            \
        0x00000010, 0x00000014,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_HARRIER_EPCL_EPCL_INTERRUPT_CAUSE_REGFILE_ADDRESS_ERROR_E,                        \
        PRV_CPSS_HARRIER_EPCL_EPCL_INTERRUPT_CAUSE_TABLE_OVERLAP_ACCESS_ERROR_E,                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eplr_NODE_MAC(bit)                                                                        \
    /* eplr  */                                                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EPLR_E, NULL,                                            \
        0x00000200, 0x00000204,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_HARRIER_EPLR_POLICER_INTERRUPT_CAUSE_ILLEGAL_OVERLAPPED_ACCESS_E,                 \
        PRV_CPSS_HARRIER_EPLR_POLICER_INTERRUPT_CAUSE_QOS_ATTRIBUTES_TABLE_WRONG_ADDRESS_INTERRUPT_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   ermrk_NODE_MAC(bit)                                                                       \
    /* ERMRK  */                                                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ERMRK_E, NULL,                                           \
        0x00000004, 0x00000008,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_HARRIER_ERMRK_ERMRK_INTERRUPT_CAUSE_REGFILE_ADDRESS_ERROR_E,                      \
        PRV_CPSS_HARRIER_ERMRK_ERMRK_INTERRUPT_CAUSE_TABLE_OVERLAP_ACCESS_ERROR_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   l2i_NODE_MAC(bit)                                                                         \
    /* L2 Bridge  */                                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_L2I_E,  NULL,                                            \
        0x00002100, 0x00002104,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_HARRIER_L2I_BRIDGE_INTERRUPT_CAUSE_ADDRESS_OUT_OF_RANGE_E,                     \
        PRV_CPSS_HARRIER_L2I_BRIDGE_INTERRUPT_CAUSE_UPDATE_SECURITY_BREACH_REGISTER_INT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* node for ingress SCT rate limiters interrupts leaf */
#define SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(index)                                          \
    {(index+1), GT_FALSE, PRV_CPSS_DXCH_UNIT_EQ_E, NULL,                                        \
        (0x00010020 + ((index) * 0x10)), (0x00010024 + ((index) * 0x10)),                       \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_HARRIER_EQ_REG_1_PORT_0_PKT_DROPED_INT_E   + ((index) * 32),                   \
        PRV_CPSS_HARRIER_EQ_REG_1_PORT_15_PKT_DROPED_INT_E  + ((index) * 32),                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   eqSctRateLimiters_SUB_TREE_MAC(bit)                                                   \
    /* SCT Rate Limiters */                                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EQ_E, NULL,                                              \
        0x00010000, 0x00010004,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_HARRIER_EQ_CPU_CODE_RATE_LIMITERS_INTERRUPT_SUMMARY_CAUSE_REG_1_CPU_CODE_RATE_LIMITER_INT_E,         \
        PRV_CPSS_HARRIER_EQ_CPU_CODE_RATE_LIMITERS_INTERRUPT_SUMMARY_CAUSE_REG_16_CPU_CODE_RATE_LIMITER_INT_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(0),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(1),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(2),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(3),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(4),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(5),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(6),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(7),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(8),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(9),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(10),                                            \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(11),                                            \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(12),                                            \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(13),                                            \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(14),                                            \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(15)

#define   eq_SUB_TREE_MAC(bit)                                                                          \
    /* Pre-Egress Interrupt Summary Cause */                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EQ_E, NULL,                                              \
        0x00000058, 0x0000005C,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_HARRIER_EQ_PRE_EGRESS_INTERRUPT_SUMMARY_CAUSE_INGRESS_STC_INT_E,                  \
        PRV_CPSS_HARRIER_EQ_PRE_EGRESS_INTERRUPT_SUMMARY_CAUSE_PORT_DLB_INTERRUPT_E,               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                 \
        eqSctRateLimiters_SUB_TREE_MAC(2)

#define   ia_NODE_MAC(bit)                                                          \
    /* IA */                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IA_E,  NULL,                                 \
        0x00000500, 0x00000504,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_HARRIER_IA_IA_INTERRUPT_0_CAUSE_IA_RF_ERR_E,                          \
        PRV_CPSS_HARRIER_IA_IA_INTERRUPT_0_CAUSE_IA_LS_FIFO_READ_ERROR_E,              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   ioam_NODE_MAC(bit)                                                        \
    /* ingress OAM  */                                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IOAM_E,  NULL,                               \
        0x000000F0, 0x000000F4,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_HARRIER_IOAM_OAM_UNIT_INTERRUPT_CAUSE_KEEP_ALIVE_AGING_E,             \
        PRV_CPSS_HARRIER_IOAM_OAM_UNIT_INTERRUPT_CAUSE_RDI_STATUS_E,                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   iplr0_NODE_MAC(bit)                                                       \
    /* iplr0 */                                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IPLR_E,  NULL,                               \
        0x00000200, 0x00000204,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_HARRIER_IPLR0_POLICER_INTERRUPT_CAUSE_ILLEGAL_OVERLAPPED_ACCESS_E,    \
        PRV_CPSS_HARRIER_IPLR0_POLICER_INTERRUPT_CAUSE_QOS_ATTRIBUTES_TABLE_WRONG_ADDRESS_INTERRUPT_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   iplr1_NODE_MAC(bit)                                                       \
    /* iplr1 */                                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IPLR_1_E,  NULL,                             \
        0x00000200, 0x00000204,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_HARRIER_IPLR1_POLICER_INTERRUPT_CAUSE_ILLEGAL_OVERLAPPED_ACCESS_E,    \
        PRV_CPSS_HARRIER_IPLR1_POLICER_INTERRUPT_CAUSE_QOS_ATTRIBUTES_TABLE_WRONG_ADDRESS_INTERRUPT_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   router_NODE_MAC(bit)                                                          \
    /* router */                                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IPVX_E,  NULL,                               \
        0x00000970, 0x00000974,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_HARRIER_IPVX_ROUTER_INTERRUPT_CAUSE_I_PV_X_BAD_ADDR_E,                \
        PRV_CPSS_HARRIER_IPVX_ROUTER_INTERRUPT_CAUSE_ILLEGAL_OVERLAP_ACCESS_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   lpm_NODE_MAC(bit)                                                             \
    /* LPM */                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_LPM_E,  NULL,                                \
        0x00F00120, 0x00F00130,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_HARRIER_LPM_LPM_GENERAL_INT_CAUSE_LPM_WRONG_ADDRESS_E,                \
        PRV_CPSS_HARRIER_LPM_LPM_GENERAL_INT_CAUSE_TABLE_OVERLAPING_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   mll_NODE_MAC(bit)                                                             \
    /* MLL */                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MLL_E,  NULL,                                \
        0x00000030, 0x00000034,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_HARRIER_MLL_MLL_INTERRUPT_CAUSE_REGISTER_INVALID_IP_MLL_ACCESS_INTERRUPT_E,         \
        PRV_CPSS_HARRIER_MLL_MLL_INTERRUPT_CAUSE_REGISTER_ILLEGAL_TABLE_OVERLAP_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   pcl_NODE_MAC(bit)                                                             \
    /* PCL */                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCL_E,  NULL,                                \
        0x00000004, 0x00000008,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_HARRIER_PCL_PCL_UNIT_INTERRUPT_CAUSE_MG_ADDR_OUTOF_RANGE_E,           \
        PRV_CPSS_HARRIER_PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL_ILLEGAL_TABLE_OVERLAP_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   preq_NODE_MAC(bit)                                                        \
    /* PREQ */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PREQ_E,  NULL,                               \
        0x00000600, 0x00000604,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_HARRIER_PREQ_PREQ_INTERRUPT_CAUSE_PREQ_REGFILE_ADDRESS_ERROR_E,    \
        PRV_CPSS_HARRIER_PREQ_PREQ_INTERRUPT_CAUSE_PREQ_TABLE_ACCESS_OVERLAP_ERROR_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   ppu_NODE_MAC(bit)                                                         \
    /* PPU */                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PPU_E, NULL,                                 \
        0x00000004, 0x00000008,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_HARRIER_PPU_PPU_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,          \
        PRV_CPSS_HARRIER_PPU_PPU_INTERRUPT_CAUSE_INT_INDEX_7_PPU_ACTION_INT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   qag_NODE_MAC(bit)                                                         \
    /* qag */                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EGF_QAG_E,  NULL,                            \
        0x000B0000, 0x000B0010,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_HARRIER_EGF_QAG_INTERRUPTS_CAUSE_WRONG_ADDRESS_INTERRUPT_E,        \
        PRV_CPSS_HARRIER_EGF_QAG_INTERRUPTS_CAUSE_TABLE_OVERLAPING_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   sht_NODE_MAC(bit)                                                         \
    /* SHT */                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,  NULL,                            \
        0x00100010, 0x00100020,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_HARRIER_EGF_SHT_INTERRUPTS_CAUSE_WRONG_ADDRESS_INTERRUPT_E,           \
        PRV_CPSS_HARRIER_EGF_SHT_INTERRUPTS_CAUSE_TABLE_OVERLAPPING_INTERRUPT_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define PPG_OFFSET(ppg)            (0x80000 * (ppg))
#define PHA_PPN_OFFSET(ppg, ppn)   (PPG_OFFSET(ppg) + 0x4000 * (ppn))

#define PHA_PPN_NODE_MAC(bit, ppg, ppn)                                         \
    /* PPN Internal Error Cause */                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E,  NULL,                            \
       (0x00003040 + PHA_PPN_OFFSET(ppg, ppn)), (0x00003044 + PHA_PPN_OFFSET(ppg, ppn)),\
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_HARRIER_PPN_##ppn##_PPG_##ppg##_PPN_INTERNAL_ERROR_CAUSE_HOST_UNMAPPED_ACCESS_E, \
        PRV_CPSS_HARRIER_PPN_##ppn##_PPG_##ppg##_PPN_INTERNAL_ERROR_CAUSE_DOORBELL_INTERRUPT_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define phaPpg_SUB_TREE_MAC(bit, ppg)                                           \
    /* PPG Internal Error Cause */                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E,  NULL,                            \
        (0x0007ff00 + PPG_OFFSET(ppg)), (0x0007ff04 + PPG_OFFSET(ppg)),         \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_HARRIER_PPG_##ppg##_PPG_INTERRUPT_SUM_CAUSE_PPG_FUNCTIONAL_SUMMARY_INTERRUPT_E, \
        PRV_CPSS_HARRIER_PPG_##ppg##_PPG_INTERRUPT_SUM_CAUSE_PPN_7_SUMMARY_INTERRUPT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 9, NULL, NULL},                \
        /*  PPG Internal Error Cause */                                         \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E,  NULL,                          \
            (0x0007ff10 + PPG_OFFSET(ppg)), (0x0007ff14 + PPG_OFFSET(ppg)),     \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_HARRIER_PPG_##ppg##_PPG_INTERNAL_ERROR_CAUSE_PPG_BAD_ADDRESS_ERROR_E,          \
            PRV_CPSS_HARRIER_PPG_##ppg##_PPG_INTERNAL_ERROR_CAUSE_PPG_TABLE_ACCESS_OVERLAP_ERROR_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},             \
        PHA_PPN_NODE_MAC(2, ppg, 0),                                            \
        PHA_PPN_NODE_MAC(3, ppg, 1),                                            \
        PHA_PPN_NODE_MAC(4, ppg, 2),                                            \
        PHA_PPN_NODE_MAC(5, ppg, 3),                                            \
        PHA_PPN_NODE_MAC(6, ppg, 4),                                            \
        PHA_PPN_NODE_MAC(7, ppg, 5),                                            \
        PHA_PPN_NODE_MAC(8, ppg, 6),                                            \
        PHA_PPN_NODE_MAC(9, ppg, 7)

#define   phaPpa_SUB_TREE_MAC(bit)                                              \
    /* PHA PPA */                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E,  NULL,                            \
        0x007eff10, 0x007eff14,                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_HARRIER_PPA_PPA_INTERRUPT_SUM_CAUSE_PPA_INTERNAL_FUNCTIONAL_SUMMARY_INTERRUPT_E,    \
        PRV_CPSS_HARRIER_PPA_PPA_INTERRUPT_SUM_CAUSE_PPG_1_SUMMARY_INTERRUPT_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                 \
        /* PPA Internal Error Cause */                                          \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E,  NULL,                          \
            0x007eff20, 0x007eff24,                                             \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_HARRIER_PPA_PPA_INTERNAL_ERROR_CAUSE_PPA_BAD_ADDRESS_ERRROR_E,\
            PRV_CPSS_HARRIER_PPA_PPA_INTERNAL_ERROR_CAUSE_PPA_BAD_ADDRESS_ERRROR_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},             \
        phaPpg_SUB_TREE_MAC(2, 0),                                              \
        phaPpg_SUB_TREE_MAC(3, 1)


#define   phaInternal_NODE_MAC(bit)                                             \
    /* PHA Internal Error Cause */                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E,  NULL,                            \
        0x007fff80, 0x007fff84,                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_HARRIER_PHA_PHA_INTERNAL_ERROR_CAUSE_PHA_BAD_ADDRESS_ERROR_E,     \
        PRV_CPSS_HARRIER_PHA_PHA_INTERNAL_ERROR_CAUSE_PPA_CLOCK_DOWN_VIOLATION_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  ha_NODE_MAC(bit)                                                           \
        /* HA Rate Limiters */                                                      \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_HA_E, NULL,                              \
            0x00000300, 0x00000304,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_HARRIER_HA_HA_INTERRUPT_CAUSE_TABLE_OVERLAP_ACCESS_ERROR_E,    \
            PRV_CPSS_HARRIER_HA_HA_INTERRUPT_CAUSE_OVERSIZE_TUNNEL_HEADER_E,        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   pha_SUB_TREE_MAC(bit)                                                                     \
    /* PHA */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E,  NULL,                                                \
        0x007fff88, 0x007fff8c,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_PHA_PHA_INTERRUPT_SUM_CAUSE_PHA_INTERNAL_FUNCTIONAL_SUMMARY_INTERRUPT_E,      \
        PRV_CPSS_HARRIER_PHA_PHA_INTERRUPT_SUM_CAUSE_PPA_SUMMARY_INTERRUPT_E,                          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                     \
        phaInternal_NODE_MAC(1),                                                                    \
        phaPpa_SUB_TREE_MAC(2)


#define   tti_NODE_MAC(bit)                                                                         \
    /* TTI */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TTI_E,  NULL,                                                \
        0x00000004, 0x00000008,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_TTI_TTI_ENGINE_INTERRUPT_CAUSE_CPU_ADDRESS_OUT_OF_RANGE_E,                    \
        PRV_CPSS_HARRIER_TTI_TTI_ENGINE_INTERRUPT_CAUSE_TABLE_OVERLAP_ERROR_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txDma_NODE_MAC(bit, _index)                                                               \
    /* TXD */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##_index##_E,  NULL,                                    \
        0x00006000, 0x00006004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_TXD_INSTANCE_##_index##_TXD_INTERRUPT_CAUSE_TXD_RF_ERR_E,                  \
        PRV_CPSS_HARRIER_TXD_INSTANCE_##_index##_TXD_INTERRUPT_CAUSE_SDQ_BYTES_UNDERFLOW_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txFifo_NODE_MAC(bit, _index)                                                              \
    /* TXF */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##_index##_E,  NULL,                                  \
        0x00004000, 0x00004004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_TXF_INSTANCE_##_index##_TXF_INTERRUPT_CAUSE_TX_FIFO_RF_ERR_E,              \
        PRV_CPSS_HARRIER_TXF_INSTANCE_##_index##_TXF_INTERRUPT_CAUSE_DESCRIPTOR_RD_SER_ERROR_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   rxDma_int0_SUB_TREE_MAC(bit, _index)                                                            \
        /* RXD INT0 */                                                                                    \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##_index##_E, NULL,                                       \
            0x00001C80, 0x00001C84,                                                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                                               \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                              \
            PRV_CPSS_HARRIER_RX_INSTANCE_##_index##_RX_DMA_INTERRUPT_0_CAUSE_RX_DMA_RF_ERR_E,             \
            PRV_CPSS_HARRIER_RX_INSTANCE_##_index##_RX_DMA_INTERRUPT_0_CAUSE_HIGH_PRIO_DESC_DROP_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   rxDma_int1_SUB_TREE_MAC(bit, _index)                                                               \
        /* RXD INT1 */                                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##_index##_E, NULL,                                          \
            0x00001C88, 0x00001C8C,                                                                          \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
            PRV_CPSS_HARRIER_RX_INSTANCE_##_index##_RX_DMA_INTERRUPT_1_CAUSE_DESC_ENQUEUE_FIFO_FULL_INT_E,   \
            PRV_CPSS_HARRIER_RX_INSTANCE_##_index##_RX_DMA_INTERRUPT_1_CAUSE_DESC_ENQUEUE_FIFO_FULL_INT_E,   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   rxDma_int2_SUB_TREE_MAC(bit, _index)                                                                     \
        /* RXD INT2 */                                                                                             \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##_index##_E, NULL,                                                \
            0x00001C90, 0x00001C94,                                                                                \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                        \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                       \
            PRV_CPSS_HARRIER_RX_INSTANCE_##_index##_RX_DMA_INTERRUPT_2_CAUSE_PB_TAIL_ID_MEM_SER_INT_E,             \
            PRV_CPSS_HARRIER_RX_INSTANCE_##_index##_RX_DMA_INTERRUPT_2_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E,   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   rxDma_NODE_MAC(bit, _index)                                                                        \
    /* RXD */                                                                                                \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##_index##_E,  NULL,                                             \
        0x00001CA0, 0x00001CA4,                                                                              \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                      \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                     \
        PRV_CPSS_HARRIER_RX_INSTANCE_##_index##_RX_DMA_INTERRUPT_SUMMARY_CAUSE_RX_DMA_INTERRUPT_0_INT_E,     \
        PRV_CPSS_HARRIER_RX_INSTANCE_##_index##_RX_DMA_INTERRUPT_SUMMARY_CAUSE_RX_DMA_INTERRUPT_2_INT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                              \
        rxDma_int0_SUB_TREE_MAC(1, _index),                                                                  \
        rxDma_int1_SUB_TREE_MAC(2, _index),                                                                  \
        rxDma_int2_SUB_TREE_MAC(3, _index)

#define npm_SUB_TREE_MAC(bit, _index)                                                                       \
    /* NPM-0  */                                                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL,                             \
        0x00000100, 0x00000104,                                                                             \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                     \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                    \
        PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_SUMMARY_CAUSE_INT_SUMMARY_CAUSE_GENERIC_E,    \
        PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_SUMMARY_CAUSE_INT_SUMMARY_CAUSE_LIST_ERR_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 10, NULL, NULL},                                            \
        /* GenericInterruptSum  */                                                                          \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL,                           \
            0x00000108, 0x0000010C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_GENERIC_CAUSE_INT_GENERIC_CAUSE_ADDR_ERR_E,      \
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_GENERIC_CAUSE_INT_GENERIC_CAUSE_RAND_ICRD_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* UintErrorInterruptSum  */                                                                        \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL,                           \
            0x00000110, 0x00000114,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_0_E,\
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_4_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* ReinsertInterruptSum  */                                                                         \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL,                           \
            0x00000118, 0x0000011C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_REINSERT_CAUSE_INT_REINSERT_CAUSE_0_0_E,  \
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_REINSERT_CAUSE_INT_REINSERT_CAUSE_3_4_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* ReinsertfailInterruptSum  */                                                                     \
        {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL,                           \
            0x00000120, 0x00000124,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_0_E,    \
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_4_E,    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* UnitRefsInterruptSum  */                                                                         \
        {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL,                           \
            0x00000128, 0x0000012C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_0_E,\
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_4_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* UnitDataInterruptSum  */                                                                         \
        {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL,                           \
            0x00000130, 0x00000134,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_0_E,\
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_4_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* RefsErrorInterruptSum  */                                                                        \
        {7, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL,                           \
            0x00000138, 0x0000013C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_0_E,\
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_3_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* AgingErrorInterruptSum  */                                                                       \
        {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL,                           \
            0x00000140, 0x00000144,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_0_E,     \
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_3_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* RefsFifoInterruptSum  */                                                                         \
        {9, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL,                           \
            0x00000148, 0x0000014C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_0_E,\
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_3_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* ListErrorInterruptSum  */                                                                        \
        {10, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL,                          \
            0x00000150, 0x00000154,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_0_E,  \
            PRV_CPSS_HARRIER_NPM_MC_INSTANCE_##_index##_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_3_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define smb_SUB_TREE_MAC(bit, _index)                                                                                   \
    /* SMB-MC */                                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E, NULL,                                         \
        0x00000150, 0x00000154,                                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                \
        PRV_CPSS_HARRIER_SMB_MC_INSTANCE_##_index##_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                           \
        PRV_CPSS_HARRIER_SMB_MC_INSTANCE_##_index##_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_FIFO_E,                           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6,NULL, NULL},                                                          \
        /* MiscInterruptSum  */                                                                                         \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E, NULL,                                       \
            0x00000100, 0x00000104,                                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            PRV_CPSS_HARRIER_SMB_MC_INSTANCE_##_index##_INTERRUPT_MISC_CAUSE_INTERRUPT_RPW_NO_TRUST_E,                  \
            PRV_CPSS_HARRIER_SMB_MC_INSTANCE_##_index##_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_CONGESTION_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                      \
        /* AgeInterruptSum  */                                                                                          \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E, NULL,                                       \
            0x00000110, 0x00000114,                                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            PRV_CPSS_HARRIER_SMB_MC_INSTANCE_##_index##_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_AGE_ERR_E,       \
            PRV_CPSS_HARRIER_SMB_MC_INSTANCE_##_index##_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_AGE_ERR_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                      \
        /* RbwInterruptSum  */                                                                                          \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E, NULL,                                       \
            0x00000120, 0x00000124,                                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            PRV_CPSS_HARRIER_SMB_MC_INSTANCE_##_index##_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_RBW_ERR_E,       \
            PRV_CPSS_HARRIER_SMB_MC_INSTANCE_##_index##_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_RBW_ERR_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                      \
        /* MemoriesInterruptSum  */                                                                                     \
        {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E, NULL,                                       \
            0x00000130, 0x00000134,                                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            PRV_CPSS_HARRIER_SMB_MC_INSTANCE_##_index##_INTERRUPT_MEMORIES_CAUSE_LOGICAL_MEMORY_0_PAYLOAD_SER_ERROR_E,  \
            PRV_CPSS_HARRIER_SMB_MC_INSTANCE_##_index##_INTERRUPT_MEMORIES_CAUSE_LOGICAL_MEMORY_15_PAYLOAD_SER_ERROR_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                      \
        /* Memories2InterruptSum  */                                                                                    \
        {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E, NULL,                                       \
            0x00000140, 0x00000144,                                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            PRV_CPSS_HARRIER_SMB_MC_INSTANCE_##_index##_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_0_REFCNT_SER_ERROR_E,  \
            PRV_CPSS_HARRIER_SMB_MC_INSTANCE_##_index##_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_15_REFCNT_SER_ERROR_E,    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                      \
        /* FifoInterruptSum  */                                                                                         \
        {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E, NULL,                                       \
            0x00000118, 0x0000011C,                                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            PRV_CPSS_HARRIER_SMB_MC_INSTANCE_##_index##_INTERRUPT_FIFO_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_FIFO_ERR_E,     \
            PRV_CPSS_HARRIER_SMB_MC_INSTANCE_##_index##_INTERRUPT_FIFO_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_FIFO_ERR_E,    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define gpc_pw_SUB_TREE_MAC(bit, _unit)                                                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_##_unit##_E, NULL,                                       \
        0x00000200, 0x0000204,                                                                                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                \
        PRV_CPSS_HARRIER_GPC_PACKET_WRITE_INSTANCE_##_unit##_INTERRUPT_CAUSE_INT_CAUSE_RDY_NFL_E,                       \
        PRV_CPSS_HARRIER_GPC_PACKET_WRITE_INSTANCE_##_unit##_INTERRUPT_CAUSE_INT_CAUSE_ADDR_ERR_E,                      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gpc_pr_chan_SUB_TREE_MAC(bit, _unit, _chan)                                                                          \
{bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_unit##_E, NULL,                                                 \
    0x00001300 + (_chan)*4, 0x00001400 + (_chan)*4,                                                                          \
    prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
    prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
    PRV_CPSS_HARRIER_GPC_PACKET_READ_INSTANCE_##_unit##_CHANNEL_INDEX_##_chan##_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E,   \
    PRV_CPSS_HARRIER_GPC_PACKET_READ_INSTANCE_##_unit##_CHANNEL_INDEX_##_chan##_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E,            \
    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gpc_pr_SUB_TREE_MAC(bit, _unit)                                                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_unit##_E, NULL,                                        \
        0x00001530, 0x00001534,                                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                \
        PRV_CPSS_HARRIER_GPC_PACKET_READ_INSTANCE_##_unit##_INTERRUPT_SUMMARY_CAUSE_INTERRUPT_CHANNELS_SUMMARY_0_E,     \
        PRV_CPSS_HARRIER_GPC_PACKET_READ_INSTANCE_##_unit##_INTERRUPT_SUMMARY_CAUSE_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_SUMMARY_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                                         \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_unit##_E, NULL,                                      \
            0x00001520, 0x0001528,                                                                                      \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            PRV_CPSS_HARRIER_GPC_PACKET_READ_INSTANCE_##_unit##_INTERRUPT_CHANNELS_SUMMARY_0_CAUSE_CHANNEL_INDEX_0_INTERRUPT_CHANNEL_SUMMARY_E,      \
            PRV_CPSS_HARRIER_GPC_PACKET_READ_INSTANCE_##_unit##_INTERRUPT_CHANNELS_SUMMARY_0_CAUSE_CHANNEL_INDEX_27_INTERRUPT_CHANNEL_SUMMARY_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 28, NULL, NULL},                                                    \
        gpc_pr_chan_SUB_TREE_MAC(1, _unit, 0),                                                                                          \
        gpc_pr_chan_SUB_TREE_MAC(2, _unit, 1),                                                                                          \
        gpc_pr_chan_SUB_TREE_MAC(3, _unit, 2),                                                                                          \
        gpc_pr_chan_SUB_TREE_MAC(4, _unit, 3),                                                                                          \
        gpc_pr_chan_SUB_TREE_MAC(5, _unit, 4),                                                                                          \
        gpc_pr_chan_SUB_TREE_MAC(6, _unit, 5),                                                                                          \
        gpc_pr_chan_SUB_TREE_MAC(7, _unit, 6),                                                                                          \
        gpc_pr_chan_SUB_TREE_MAC(8, _unit, 7),                                                                                          \
        gpc_pr_chan_SUB_TREE_MAC(9, _unit, 8),                                                                                          \
        gpc_pr_chan_SUB_TREE_MAC(10, _unit, 9),                                                                                         \
        gpc_pr_chan_SUB_TREE_MAC(11, _unit, 10),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(12, _unit, 11),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(13, _unit, 12),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(14, _unit, 13),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(15, _unit, 14),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(16, _unit, 15),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(17, _unit, 16),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(18, _unit, 17),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(19, _unit, 18),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(20, _unit, 19),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(21, _unit, 20),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(22, _unit, 21),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(23, _unit, 22),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(24, _unit, 23),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(25, _unit, 24),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(26, _unit, 25),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(27, _unit, 26),                                                                                        \
        gpc_pr_chan_SUB_TREE_MAC(28, _unit, 27),                                                                                        \
        {11, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_unit##_E, NULL,                                     \
            0x00001500, 0x0001504,                                                                                      \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            PRV_CPSS_HARRIER_GPC_PACKET_READ_INSTANCE_##_unit##_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E,         \
            PRV_CPSS_HARRIER_GPC_PACKET_READ_INSTANCE_##_unit##_INTERRUPT_PACKET_COUNT_CAUSE_NULL_ADDRESS_RECEIVED_PACKET_COUNT_CELL_E,              \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                     \
        {12, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_unit##_E, NULL,                                     \
            0x00001508, 0x000150C,                                                                                      \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            PRV_CPSS_HARRIER_GPC_PACKET_READ_INSTANCE_##_unit##_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E,                       \
            PRV_CPSS_HARRIER_GPC_PACKET_READ_INSTANCE_##_unit##_INTERRUPT_MISC_CAUSE_NULL_ADDRESS_RECEIVED_PACKET_READ_CELL_E,                       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                     \
        {13, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_unit##_E, NULL,                                     \
            0x00001510, 0x0001514,                                                                                      \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            PRV_CPSS_HARRIER_GPC_PACKET_READ_INSTANCE_##_unit##_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E,\
            PRV_CPSS_HARRIER_GPC_PACKET_READ_INSTANCE_##_unit##_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                     \
        {14, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_unit##_E, NULL,                                     \
            0x00001518, 0x000151C,                                                                                      \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            PRV_CPSS_HARRIER_GPC_PACKET_READ_INSTANCE_##_unit##_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E,    \
            PRV_CPSS_HARRIER_GPC_PACKET_READ_INSTANCE_##_unit##_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gpc_cr_SUB_TREE_MAC(bit, _unit)                                                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_##_unit##_E, NULL,                                          \
        0x000000A0, 0x00000A4,                                                                                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                \
        PRV_CPSS_HARRIER_GPC_CELL_READ_INSTANCE_##_unit##_INTERRUPT_CAUSE_INTERFACE_0_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_E,\
        PRV_CPSS_HARRIER_GPC_CELL_READ_INSTANCE_##_unit##_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define smb_write_arb_SUB_TREE_MAC(bit, _unit)                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_##_unit##_E, NULL,                                          \
        0x00000110, 0x0000114,                                                                                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                \
        PRV_CPSS_HARRIER_SMB_WRITE_ARBITER_INSTANCE_##_unit##_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                 \
        PRV_CPSS_HARRIER_SMB_WRITE_ARBITER_INSTANCE_##_unit##_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pb_SUB_TREE_MAC(bit)                                                                        \
    /* Packet Buffer */                                                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E,  NULL,                                      \
        0x00001128, 0x0000112c,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_PB_CENTER_INTERRUPT_SUMMARY_PACKET_BUFFER_CAUSE_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTERS_E, \
        PRV_CPSS_HARRIER_PB_CENTER_INTERRUPT_SUMMARY_PACKET_BUFFER_CAUSE_INTERRUPT_SUMMARY_PB_CENTER_E,           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6, NULL, NULL},                                      \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E,  NULL,                                     \
            0x00001108, 0x0000110C,                                                                  \
            prvCpssDrvHwPpPortGroupIsrRead,                                                          \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
            PRV_CPSS_HARRIER_PB_CENTER_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTER_E, \
            PRV_CPSS_HARRIER_PB_CENTER_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTER_CAUSE_INSTANCE_1_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTER_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                   \
            npm_SUB_TREE_MAC(1, 0),                                                                   \
            npm_SUB_TREE_MAC(2, 1),                                                                   \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E,  NULL,                                      \
            0x00001110, 0x00001114,                                                                   \
            prvCpssDrvHwPpPortGroupIsrRead,                                                           \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                          \
            PRV_CPSS_HARRIER_PB_CENTER_INTERRUPT_SUMMARY_GPC_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_PACKET_WRITE_GPC_E,  \
            PRV_CPSS_HARRIER_PB_CENTER_INTERRUPT_SUMMARY_GPC_CAUSE_INSTANCE_3_INTERRUPT_SUMMARY_CELL_READ_GPC_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 9, NULL, NULL},                                   \
            gpc_pw_SUB_TREE_MAC(1,  0),                                                               \
            gpc_pw_SUB_TREE_MAC(2,  1),                                                               \
            gpc_pw_SUB_TREE_MAC(3,  2),                                                               \
            gpc_pr_SUB_TREE_MAC(5,  0),                                                               \
            gpc_pr_SUB_TREE_MAC(6,  1),                                                               \
            gpc_pr_SUB_TREE_MAC(7,  2),                                                               \
            gpc_cr_SUB_TREE_MAC(9,  0),                                                               \
            gpc_cr_SUB_TREE_MAC(10, 1),                                                               \
            gpc_cr_SUB_TREE_MAC(11, 2),                                                               \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E,  NULL,                                      \
            0x00001118, 0x0000111C,                                                                   \
            prvCpssDrvHwPpPortGroupIsrRead,                                                           \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                          \
            PRV_CPSS_HARRIER_PB_CENTER_INTERRUPT_SUMMARY_SMB_MEMORY_CLUSTER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_SMB_MEMORY_CLUSTER_E,  \
            PRV_CPSS_HARRIER_PB_CENTER_INTERRUPT_SUMMARY_SMB_MEMORY_CLUSTER_CAUSE_INSTANCE_2_INTERRUPT_SUMMARY_SMB_MEMORY_CLUSTER_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                   \
            smb_SUB_TREE_MAC(1, 0),                                                                   \
            smb_SUB_TREE_MAC(2, 1),                                                                   \
            smb_SUB_TREE_MAC(3, 2),                                                                   \
        {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E,  NULL,                                      \
            0x00001120, 0x00001124,                                                                   \
            prvCpssDrvHwPpPortGroupIsrRead,                                                           \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                          \
            PRV_CPSS_HARRIER_PB_CENTER_INTERRUPT_SUMMARY_SMB_WRITE_ARBITER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_SMB_WRITE_ARBITER_E,                          \
            PRV_CPSS_HARRIER_PB_CENTER_INTERRUPT_SUMMARY_SMB_WRITE_ARBITER_CAUSE_INSTANCE_3_INTERRUPT_SUMMARY_SMB_WRITE_ARBITER_E,                          \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                   \
            smb_write_arb_SUB_TREE_MAC(1, 0),                                                         \
            smb_write_arb_SUB_TREE_MAC(2, 1),                                                         \
            smb_write_arb_SUB_TREE_MAC(3, 2),                                                         \
        {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_COUNTER_E, NULL,                                          \
            0x00000190, 0x00000194,                                                                   \
            prvCpssDrvHwPpPortGroupIsrRead,                                                           \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                          \
            PRV_CPSS_HARRIER_PB_COUNTER_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                     \
            PRV_CPSS_HARRIER_PB_COUNTER_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                   \
        {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E,  NULL,                                      \
            0x00001100, 0x00001104,                                                                   \
            prvCpssDrvHwPpPortGroupIsrRead,                                                           \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                          \
            PRV_CPSS_HARRIER_PB_CENTER_INTERRUPT_PB_CENTER_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,            \
            PRV_CPSS_HARRIER_PB_CENTER_INTERRUPT_PB_CENTER_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   erep_NODE_MAC(bit)                                                                            \
    /* EREP Rate Limiters */                                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EREP_E,  NULL,                                                   \
        0x00003000, 0x00003004,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_EREP_EREP_INTERRUPTS_CAUSE_EREP_CPU_ADDRESS_OUT_OF_RANGE_E,                       \
        PRV_CPSS_HARRIER_EREP_EREP_INTERRUPTS_CAUSE_MIRROR_REPLICATION_NOT_PERFORMED_E,                    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   cnc_SUB_TREE_MAC(bit)                                                                         \
    /* CNC  */                                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_CNC_0_E,  NULL,                                                  \
        0x00000100, 0x00000104,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_CNC_CNC_INTERRUPT_SUMMARY_CAUSE_REGISTER_WRAPAROUND_FUNC_INTERRUPT_SUM_E,         \
        PRV_CPSS_HARRIER_CNC_CNC_INTERRUPT_SUMMARY_CAUSE_REGISTER_MISC_FUNC_INTERRUPT_SUM_E,               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3,NULL, NULL},                                          \
                                                                                                        \
        /* WraparoundFuncInterruptSum  */                                                               \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_CNC_0_E,  NULL,                                     \
            0x00000190, 0x000001a4,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_HARRIER_CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_0_WRAPAROUND_E,   \
            PRV_CPSS_HARRIER_CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_15_WRAPAROUND_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                      \
        /* RateLimitFuncInterruptSum  */                                                                \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_CNC_0_E,  NULL,                                     \
            0x000001b8, 0x000001cc,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_HARRIER_CNC_RATE_LIMIT_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_NUMBER_0_RATE_LIMIT_FIFO_DROP_E,  \
            PRV_CPSS_HARRIER_CNC_RATE_LIMIT_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_NUMBER_15_RATE_LIMIT_FIFO_DROP_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                      \
        /* MiscFuncInterruptSum  */                                                                     \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_CNC_0_E,  NULL,                                     \
            0x000001e0, 0x000001e4,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_HARRIER_CNC_MISC_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_DUMP_FINISHED_E,           \
            PRV_CPSS_HARRIER_CNC_MISC_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_CNC_TABLE_OVERLAPPING_E,   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   epcl_tai_NODE_MAC(bit, _tai)                                              \
    /* EPCL TAI  */                                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EPCL_HA_TAI##_tai##_E, NULL,                 \
        0x00000000, 0x00000004,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_HARRIER_ERMRK_TAI_SLAVE_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,        \
        PRV_CPSS_HARRIER_ERMRK_TAI_SLAVE_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define  l2i_tai_SUB_TREE_MAC(bit, _tai)                                                                \
    /* L2I TAI */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TAI##_tai##_E, NULL,                                             \
        0x00000000, 0x00000004,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_TAI_MASTER_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,                      \
        PRV_CPSS_HARRIER_TAI_MASTER_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   pha_tai_NODE_MAC(bit, _tai)                                               \
    /* PHA TAI  */                                                                  \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_TAI##_tai##_E, NULL,                     \
        0x00000000, 0x00000004,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_HARRIER_PHA_TAI_SLAVE_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,   \
        PRV_CPSS_HARRIER_PHA_TAI_SLAVE_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define pzarb_PCA_RPZ_NODE_MAC(bit, _instance)                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_##_instance##_E, NULL,                          \
        0x00000B04, 0x00000B08,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_RPZ_ARB_TYPE_1_RPZ_ARB_INSTANCE_##_instance##_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,     \
        PRV_CPSS_HARRIER_RPZ_ARB_TYPE_1_RPZ_ARB_INSTANCE_##_instance##_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pzarb_PCA_TPZ_NODE_MAC(bit, _instance)                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_##_instance##_E, NULL,                          \
        0x00000b04, 0x00000b08,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_TPZ_ARB_TYPE_0_TPZ_ARB_INSTANCE_##_instance##_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,     \
        PRV_CPSS_HARRIER_TPZ_ARB_TYPE_0_TPZ_ARB_INSTANCE_##_instance##_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pds_SUB_TREE_MAC(bit, _pds)                                                                                 \
    /* PDS Interrupt Summary */                                                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS##_pds##_E, NULL,                                               \
        0x00042000, 0x00042004,                                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
        PRV_CPSS_HARRIER_PDS_UNIT_##_pds##_PDS_INTERRUPT_SUMMARY_CAUSE_PDS_FUNCTIONAL_INT_SUM_E,                    \
        PRV_CPSS_HARRIER_PDS_UNIT_##_pds##_PDS_INTERRUPT_SUMMARY_CAUSE_PDS_DEBUG_INT_SUM_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                                     \
        /* PDS Functional Interrupt Summary */                                                                      \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS##_pds##_E, NULL,                                             \
            0x00042008, 0x0004200C,                                                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                        \
            PRV_CPSS_HARRIER_PDS_UNIT_##_pds##_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PDS_BAD_ADDRESS_INT_E,                \
            PRV_CPSS_HARRIER_PDS_UNIT_##_pds##_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_E,            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                 \
        /* PDS Debug Interrupt Summary */                                                                           \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS##_pds##_E, NULL,                                             \
            0x00042010, 0x00042014,                                                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                        \
            PRV_CPSS_HARRIER_PDS_UNIT_##_pds##_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_E, \
            PRV_CPSS_HARRIER_PDS_UNIT_##_pds##_PDS_INTERRUPT_DEBUG_CAUSE_PB_RD_NOT_PERFORMED_INT_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define qfc_FUNC_INT_NODE_MAC(bit, _unit)                                                        \
    /* QFC Interrupt Summary */                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC##_unit##_E, NULL,                   \
        0x00000200, 0x00000204,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_HARRIER_QFC_UNIT_##_unit##_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E, \
        PRV_CPSS_HARRIER_QFC_UNIT_##_unit##_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define qfc_DBG_INT_NODE_MAC(bit, _unit)                                                                            \
    /* QFC Debug Interrupt Summary */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC##_unit##_E, NULL,                                              \
        0x00000208, 0x0000020C,                                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
        PRV_CPSS_HARRIER_QFC_UNIT_##_unit##_QFC_INTERRUPT_DEBUG_CAUSE_PFC_MESSAGES_DROP_COUNTER_WRAPAROUND_INT_E,   \
        PRV_CPSS_HARRIER_QFC_UNIT_##_unit##_QFC_INTERRUPT_DEBUG_CAUSE_AGGREGATION_BUFFER_COUNT_OVERFLOW_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define QFC_HR_INT_OFFSET(_unit, _regIndex)                                                                         \
        (((_unit) * 9 * 32) + ((_regIndex) * 32))

#define qfc_HR_INT_NODE_MAC(bit, _unit, _regIndex)                                                                  \
    /* QFC HR Crossed Threshold Interrupt Summary */                                                                \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC##_unit##_E, NULL,                                              \
        (0x0000021C+(_regIndex*0x8)), (0x00000220+(_regIndex*0x8)),                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
        PRV_CPSS_HARRIER_QFC_UNIT_0_REGISTER_INDEX_0_PORT_0_TC_0_CROSSED_HR_THRESHOLD_E + QFC_HR_INT_OFFSET(_unit, _regIndex),   \
        PRV_CPSS_HARRIER_QFC_UNIT_0_REGISTER_INDEX_0_PORT_2_TC_7_CROSSED_HR_THRESHOLD_E + QFC_HR_INT_OFFSET(_unit, _regIndex),   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define qfc_SUB_TREE_MAC(bit, _unit)                                                    \
    /* QFC Interrupt Summary */                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC##_unit##_E, NULL,                  \
        0x00000210, 0x00000214,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_HARRIER_QFC_UNIT_##_unit##_QFC_INTERRUPT_SUMMARY_CAUSE_QFC_FUNCTIONAL_INT_SUM_E,    \
        PRV_CPSS_HARRIER_QFC_UNIT_##_unit##_QFC_INTERRUPT_SUMMARY_CAUSE_REGISTER_INDEX_8_INT_SUM_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 11, NULL, NULL},                        \
        qfc_FUNC_INT_NODE_MAC( 1, _unit),                                               \
        qfc_DBG_INT_NODE_MAC ( 2, _unit),                                               \
        qfc_HR_INT_NODE_MAC  ( 3, _unit, 0),                                            \
        qfc_HR_INT_NODE_MAC  ( 4, _unit, 1),                                            \
        qfc_HR_INT_NODE_MAC  ( 5, _unit, 2),                                            \
        qfc_HR_INT_NODE_MAC  ( 6, _unit, 3),                                            \
        qfc_HR_INT_NODE_MAC  ( 7, _unit, 4),                                            \
        qfc_HR_INT_NODE_MAC  ( 8, _unit, 5),                                            \
        qfc_HR_INT_NODE_MAC  ( 9, _unit, 6),                                            \
        qfc_HR_INT_NODE_MAC  (10, _unit, 7),                                            \
        qfc_HR_INT_NODE_MAC  (11, _unit, 8)

#define sdq_NODE_MAC(bit, _sdq)                                                                     \
    /* SDQ Interrupt Summary */                                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ##_sdq##_E, NULL,                               \
        0x00000200, 0x00000204,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_SDQ_INST_##_sdq##_SDQ_INTERRUPT_SUMMARY_CAUSE_SDQ_FUNCTIONAL_INTERRUPT_SUMMARY_E, \
        PRV_CPSS_HARRIER_SDQ_INST_##_sdq##_SDQ_INTERRUPT_SUMMARY_CAUSE_SDQ_DEBUG_INTERRUPT_SUMMARY_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define tai_master_NODE_MAC(bit, _instance)                                                    \
    /* TAI */                                                                                  \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_CPU_PORT_TAI##_instance##_E, NULL,                      \
        0x00000000, 0x00000004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_TAI_MASTER_##_instance##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,      \
        PRV_CPSS_HARRIER_TAI_MASTER_##_instance##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define led_NODE_MAC(bit, _instance)                                                  \
    /*  LED */                                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_LED_##_instance##_E, NULL,                          \
        0x00000200, 0x00000204,                                                            \
        prvCpssDrvHwPpPortGroupIsrRead,                                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                   \
        PRV_CPSS_HARRIER_LED_INSTANCE_##_instance##_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,             \
        PRV_CPSS_HARRIER_LED_INSTANCE_##_instance##_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define mac_pcs_NODE_MAC(bit, _instance, _port)                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MAC_400G_##_instance##_E, NULL,                          \
        PORT_MAC_INTERRUPT_CAUSE_MAC(_port),                                                    \
        PORT_MAC_INTERRUPT_MASK_MAC(_port),                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_HARRIER_MAC_INSTANCE_##_instance##_PORT_NUM_##_port##_LINK_OK_CHANGE_E,        \
        PRV_CPSS_HARRIER_MAC_INSTANCE_##_instance##_PORT_NUM_##_port##_TSD_NON_ACCURATE_PTP_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define mac_pcs_SEG_NODE_MAC(bit, _instance, _port)                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MAC_400G_##_instance##_E, NULL,                           \
        PORT_SEG_MAC_INTERRUPT_CAUSE_MAC(_port),                                                 \
        PORT_SEG_MAC_INTERRUPT_MASK_MAC(_port),                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
        PRV_CPSS_HARRIER_MAC_INSTANCE_##_instance##_SEG_PORT_NUM_##_port##_LINK_OK_CHANGE_E,             \
        PRV_CPSS_HARRIER_MAC_INSTANCE_##_instance##_SEG_PORT_NUM_##_port##_FRAME_DROP_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define mac_pcs_SUB_TREE_MAC(bit, _instance)                                                    \
    /*  400 MAC */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MAC_400G_##_instance##_E, NULL,                               \
        0x0000004C, 0x00000050,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_HARRIER_MAC_INSTANCE_##_instance##_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,         \
        PRV_CPSS_HARRIER_MAC_INSTANCE_##_instance##_GLOBAL_INTERRUPT_SUMMARY_CAUSE_FEC_NCE_INT_SUM_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 10, NULL, NULL},                                     \
            mac_pcs_NODE_MAC(1, _instance,  0),                                                 \
            mac_pcs_NODE_MAC(2, _instance,  1),                                                 \
            mac_pcs_NODE_MAC(3, _instance,  2),                                                 \
            mac_pcs_NODE_MAC(4, _instance,  3),                                                 \
            mac_pcs_NODE_MAC(5, _instance,  4),                                                 \
            mac_pcs_NODE_MAC(6, _instance,  5),                                                 \
            mac_pcs_NODE_MAC(7, _instance,  6),                                                 \
            mac_pcs_NODE_MAC(8, _instance,  7),                                                 \
            mac_pcs_SEG_NODE_MAC (9, _instance,  0),                                            \
            mac_pcs_SEG_NODE_MAC(10, _instance,  1)

#define mac_pcs_GOP2_SUB_TREE_MAC(bit)                                                          \
    /* 400G MAC */                                                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MAC_400G_2_E, NULL,                                      \
        0x0000004C, 0x00000050,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_HARRIER_MAC_INSTANCE_2_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,  \
        PRV_CPSS_HARRIER_MAC_INSTANCE_2_GLOBAL_INTERRUPT_SUMMARY_CAUSE_FEC_NCE_INT_SUM_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                 \
            mac_pcs_NODE_MAC(1, 2,  0),                                                         \
            mac_pcs_NODE_MAC(2, 2,  1),                                                         \
            mac_pcs_NODE_MAC(3, 2,  2),                                                         \
            mac_pcs_NODE_MAC(4, 2,  3),                                                         \
            mac_pcs_SEG_NODE_MAC (9, 2,  0)

#define mif_NODE_MAC(bit, _instance)                                                                \
    /*  MIF */                                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MIF_400G_##_instance##_E, NULL,                              \
        0x00000120, 0x00000124,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_MIF_INSTACE_##_instance##_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_TYPE_8_TX_PROTOCOL_VIOLATION_INTERRUPT_E, \
        PRV_CPSS_HARRIER_MIF_INSTACE_##_instance##_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_BAD_ACCESS_SUM_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* CTSU RX Channel - cause */
#define CTSU_CHANNEL_PCA_INTERRUPT_CAUSE_MAC(channel)                                                   \
        (0x3000 + ((channel) * 0x4))

/* CTSU RX Channel - mask */
#define CTSU_CHANNEL_PCA_INTERRUPT_MASK_MAC(channel)                        \
        (0x3400 + ((channel) * 0x4))

#define ctsu_PCA_GLOBAL_NODE_MAC(bit, _instance)                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E, NULL,                                  \
        0x00000080, 0x00000084,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_CTSU_INSTANCE_##_instance##_CTSU_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,     \
        PRV_CPSS_HARRIER_CTSU_INSTANCE_##_instance##_CTSU_GLOBAL_INTERRUPT_CAUSE_TIME_STAMPING_COMMAND_PARSING_ERROR_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define ctsu_channel_group0_PCA_SUB_TREE_MAC(bit, _instance)                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E, NULL,                                  \
        0x00003800, 0x00003820,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_CTSU_INSTANCE_##_instance##_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,  \
        PRV_CPSS_HARRIER_CTSU_INSTANCE_##_instance##_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_0_CHANNEL_CTSU_16_PCA_NODES_MAC(_instance)

#define ctsu_channel_group1_PCA_SUB_TREE_MAC(bit, _instance)                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E, NULL,                                  \
        0x00003804, 0x00003824,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_CTSU_INSTANCE_##_instance##_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,  \
        PRV_CPSS_HARRIER_CTSU_INSTANCE_##_instance##_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_1_CHANNEL_CTSU_16_PCA_NODES_MAC(_instance)

#define ctsu_channel_PCA_NODE_MAC(bit, _instance, channel)                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E, NULL,                                  \
        CTSU_CHANNEL_PCA_INTERRUPT_CAUSE_MAC(channel),                                                  \
        CTSU_CHANNEL_PCA_INTERRUPT_MASK_MAC(channel),                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_CTSU_INSTANCE_##_instance##_CHANNEL_##channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_0_NEW_ENTRY_INTERRUPT_E,     \
        PRV_CPSS_HARRIER_CTSU_INSTANCE_##_instance##_CHANNEL_##channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_1_OVERFLOW_INTERRUPT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define GROUP_0_CHANNEL_CTSU_16_PCA_NODES_MAC(_instance)                                                \
        ctsu_channel_PCA_NODE_MAC( 1, _instance,  0),                                                   \
        ctsu_channel_PCA_NODE_MAC( 2, _instance,  1),                                                   \
        ctsu_channel_PCA_NODE_MAC( 3, _instance,  2),                                                   \
        ctsu_channel_PCA_NODE_MAC( 4, _instance,  3),                                                   \
        ctsu_channel_PCA_NODE_MAC( 5, _instance,  4),                                                   \
        ctsu_channel_PCA_NODE_MAC( 6, _instance,  5),                                                   \
        ctsu_channel_PCA_NODE_MAC( 7, _instance,  6),                                                   \
        ctsu_channel_PCA_NODE_MAC( 8, _instance,  7),                                                   \
        ctsu_channel_PCA_NODE_MAC( 9, _instance,  8),                                                   \
        ctsu_channel_PCA_NODE_MAC(10, _instance,  9),                                                   \
        ctsu_channel_PCA_NODE_MAC(11, _instance, 10),                                                   \
        ctsu_channel_PCA_NODE_MAC(12, _instance, 11),                                                   \
        ctsu_channel_PCA_NODE_MAC(13, _instance, 12),                                                   \
        ctsu_channel_PCA_NODE_MAC(14, _instance, 13),                                                   \
        ctsu_channel_PCA_NODE_MAC(15, _instance, 14),                                                   \
        ctsu_channel_PCA_NODE_MAC(16, _instance, 15)

#define GROUP_1_CHANNEL_CTSU_16_PCA_NODES_MAC(_instance)                                                \
        ctsu_channel_PCA_NODE_MAC( 1, _instance, 16),                                                   \
        ctsu_channel_PCA_NODE_MAC( 2, _instance, 17),                                                   \
        ctsu_channel_PCA_NODE_MAC( 3, _instance, 18),                                                   \
        ctsu_channel_PCA_NODE_MAC( 4, _instance, 19),                                                   \
        ctsu_channel_PCA_NODE_MAC( 5, _instance, 20),                                                   \
        ctsu_channel_PCA_NODE_MAC( 6, _instance, 21),                                                   \
        ctsu_channel_PCA_NODE_MAC( 7, _instance, 22),                                                   \
        ctsu_channel_PCA_NODE_MAC( 8, _instance, 23),                                                   \
        ctsu_channel_PCA_NODE_MAC( 9, _instance, 24),                                                   \
        ctsu_channel_PCA_NODE_MAC(10, _instance, 25),                                                   \
        ctsu_channel_PCA_NODE_MAC(11, _instance, 26),                                                   \
        ctsu_channel_PCA_NODE_MAC(12, _instance, 27),                                                   \
        ctsu_channel_PCA_NODE_MAC(13, _instance, 28),                                                   \
        ctsu_channel_PCA_NODE_MAC(14, _instance, 29),                                                   \
        ctsu_channel_PCA_NODE_MAC(15, _instance, 30),                                                   \
        ctsu_channel_PCA_NODE_MAC(16, _instance, 31)

#define ctsu_PCA_SUB_TREE_MAC(bit, _instance)                                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E, NULL,                                  \
        0x00000088, 0x0000008c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_CTSU_INSTANCE_##_instance##_CTSU_INTERRUPT_CAUSE_SUMMARY_GLOBAL_INTERRUPT_CAUSE_SUMMARY_E, \
        PRV_CPSS_HARRIER_CTSU_INSTANCE_##_instance##_CTSU_INTERRUPT_CAUSE_SUMMARY_CHANNEL_GROUP_1_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                         \
        ctsu_PCA_GLOBAL_NODE_MAC(1, _instance),                                                         \
        ctsu_channel_group0_PCA_SUB_TREE_MAC(2, _instance),                                             \
        ctsu_channel_group1_PCA_SUB_TREE_MAC(3, _instance)

/* CTSU RX Channel - cause */
#define CTSU_CHANNEL_MSEC_INTERRUPT_CAUSE_MAC(channel)                                                   \
        (0x3000 + ((channel) * 0x4))

/* CTSU RX Channel - mask */
#define CTSU_CHANNEL_MSEC_INTERRUPT_MASK_MAC(channel)                        \
        (0x3400 + ((channel) * 0x4))

#define ctsu_MSEC_GLOBAL_NODE_MAC(bit, _instance)                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E, NULL,                                  \
        0x00000080, 0x00000084,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CTSU_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,     \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CTSU_GLOBAL_INTERRUPT_CAUSE_TIME_STAMPING_COMMAND_PARSING_ERROR_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define ctsu_channel_group0_MSEC_SUB_TREE_MAC(bit, _instance)                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E, NULL,                                  \
        0x00003800, 0x00003820,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,  \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_0_CHANNEL_CTSU_16_MSEC_NODES_MAC(_instance)

#define ctsu_channel_group1_MSEC_SUB_TREE_MAC(bit, _instance)                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E, NULL,                                  \
        0x00003804, 0x00003824,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,  \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_1_CHANNEL_CTSU_16_MSEC_NODES_MAC(_instance)

#define ctsu_channel_MSEC_NODE_MAC(bit, _instance, channel)                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E, NULL,                                  \
        CTSU_CHANNEL_MSEC_INTERRUPT_CAUSE_MAC(channel),                                                  \
        CTSU_CHANNEL_MSEC_INTERRUPT_MASK_MAC(channel),                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_CTSU_INSTANCE__##_instance##_CHANNEL_##channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_0_NEW_ENTRY_INTERRUPT_E,     \
        PRV_CPSS_HARRIER_CTSU_INSTANCE__##_instance##_CHANNEL_##channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_1_OVERFLOW_INTERRUPT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define GROUP_0_CHANNEL_CTSU_16_MSEC_NODES_MAC(_instance)                                                \
        ctsu_channel_MSEC_NODE_MAC( 1, _instance,  0),                                                   \
        ctsu_channel_MSEC_NODE_MAC( 2, _instance,  1),                                                   \
        ctsu_channel_MSEC_NODE_MAC( 3, _instance,  2),                                                   \
        ctsu_channel_MSEC_NODE_MAC( 4, _instance,  3),                                                   \
        ctsu_channel_MSEC_NODE_MAC( 5, _instance,  4),                                                   \
        ctsu_channel_MSEC_NODE_MAC( 6, _instance,  5),                                                   \
        ctsu_channel_MSEC_NODE_MAC( 7, _instance,  6),                                                   \
        ctsu_channel_MSEC_NODE_MAC( 8, _instance,  7),                                                   \
        ctsu_channel_MSEC_NODE_MAC( 9, _instance,  8),                                                   \
        ctsu_channel_MSEC_NODE_MAC(10, _instance,  9),                                                   \
        ctsu_channel_MSEC_NODE_MAC(11, _instance, 10),                                                   \
        ctsu_channel_MSEC_NODE_MAC(12, _instance, 11),                                                   \
        ctsu_channel_MSEC_NODE_MAC(13, _instance, 12),                                                   \
        ctsu_channel_MSEC_NODE_MAC(14, _instance, 13),                                                   \
        ctsu_channel_MSEC_NODE_MAC(15, _instance, 14),                                                   \
        ctsu_channel_MSEC_NODE_MAC(16, _instance, 15)

#define GROUP_1_CHANNEL_CTSU_16_MSEC_NODES_MAC(_instance)                                                \
        ctsu_channel_MSEC_NODE_MAC( 1, _instance, 16),                                                   \
        ctsu_channel_MSEC_NODE_MAC( 2, _instance, 17),                                                   \
        ctsu_channel_MSEC_NODE_MAC( 3, _instance, 18),                                                   \
        ctsu_channel_MSEC_NODE_MAC( 4, _instance, 19),                                                   \
        ctsu_channel_MSEC_NODE_MAC( 5, _instance, 20),                                                   \
        ctsu_channel_MSEC_NODE_MAC( 6, _instance, 21),                                                   \
        ctsu_channel_MSEC_NODE_MAC( 7, _instance, 22),                                                   \
        ctsu_channel_MSEC_NODE_MAC( 8, _instance, 23),                                                   \
        ctsu_channel_MSEC_NODE_MAC( 9, _instance, 24),                                                   \
        ctsu_channel_MSEC_NODE_MAC(10, _instance, 25),                                                   \
        ctsu_channel_MSEC_NODE_MAC(11, _instance, 26),                                                   \
        ctsu_channel_MSEC_NODE_MAC(12, _instance, 27),                                                   \
        ctsu_channel_MSEC_NODE_MAC(13, _instance, 28),                                                   \
        ctsu_channel_MSEC_NODE_MAC(14, _instance, 29),                                                   \
        ctsu_channel_MSEC_NODE_MAC(15, _instance, 30),                                                   \
        ctsu_channel_MSEC_NODE_MAC(16, _instance, 31)

#define ctsu_MSEC_SUB_TREE_MAC(bit, _instance)                                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E, NULL,                                  \
        0x00000088, 0x0000008c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CTSU_INTERRUPT_CAUSE_SUMMARY_GLOBAL_INTERRUPT_CAUSE_SUMMARY_E, \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CTSU_INTERRUPT_CAUSE_SUMMARY_CHANNEL_GROUP_1_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                         \
        ctsu_MSEC_GLOBAL_NODE_MAC(1, _instance),                                                         \
        ctsu_channel_group0_MSEC_SUB_TREE_MAC(2, _instance),                                             \
        ctsu_channel_group1_MSEC_SUB_TREE_MAC(3, _instance)

#define sff_channel0_PCA_NODE_MAC(bit, _instance, channel)                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                  \
        SFF_GR0_CHANNEL_INTERRUPT_CAUSE_MAC(channel),                                                  \
        SFF_GR0_CHANNEL_INTERRUPT_MASK_MAC(channel),                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                               \
        PRV_CPSS_HARRIER_SFF_INSTANCE_##_instance##_CHID_15_0_##channel##_CH_OCCUP_CNT_OF_INTERRUPT_CAUSE_E,  \
        PRV_CPSS_HARRIER_SFF_INSTANCE_##_instance##_CHID_15_0_##channel##_CH_CRT_TOK0_ACCU_OF_INTERRUPT_CAUSE_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define sff_channel1_PCA_NODE_MAC(bit, _instance, channel)                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        SFF_GR1_CHANNEL_INTERRUPT_CAUSE_MAC((channel % 16)),                                            \
        SFF_GR1_CHANNEL_INTERRUPT_MASK_MAC((channel % 16)),                                             \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_SFF_INSTANCE_##_instance##_CHID_31_16_##channel##_CH_OCCUP_CNT_OF_INTERRUPT_CAUSE_E,  \
        PRV_CPSS_HARRIER_SFF_INSTANCE_##_instance##_CHID_31_16_##channel##_CH_CRT_TOK0_ACCU_OF_INTERRUPT_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define GROUP_0_CHANNEL_SFF_16_PCA_NODES_MAC(_instance)                                                \
        sff_channel0_PCA_NODE_MAC( 1, _instance,  0),                                                  \
        sff_channel0_PCA_NODE_MAC( 2, _instance,  1),                                                  \
        sff_channel0_PCA_NODE_MAC( 3, _instance,  2),                                                  \
        sff_channel0_PCA_NODE_MAC( 4, _instance,  3),                                                  \
        sff_channel0_PCA_NODE_MAC( 5, _instance,  4),                                                  \
        sff_channel0_PCA_NODE_MAC( 6, _instance,  5),                                                  \
        sff_channel0_PCA_NODE_MAC( 7, _instance,  6),                                                  \
        sff_channel0_PCA_NODE_MAC( 8, _instance,  7),                                                  \
        sff_channel0_PCA_NODE_MAC( 9, _instance,  8),                                                  \
        sff_channel0_PCA_NODE_MAC(10, _instance,  9),                                                  \
        sff_channel0_PCA_NODE_MAC(11, _instance, 10),                                                  \
        sff_channel0_PCA_NODE_MAC(12, _instance, 11),                                                  \
        sff_channel0_PCA_NODE_MAC(13, _instance, 12),                                                  \
        sff_channel0_PCA_NODE_MAC(14, _instance, 13),                                                  \
        sff_channel0_PCA_NODE_MAC(15, _instance, 14),                                                  \
        sff_channel0_PCA_NODE_MAC(16, _instance, 15)

#define GROUP_1_CHANNEL_SFF_16_PCA_NODES_MAC(_instance)                                                \
        sff_channel1_PCA_NODE_MAC( 1, _instance, 16),                                                  \
        sff_channel1_PCA_NODE_MAC( 2, _instance, 17),                                                  \
        sff_channel1_PCA_NODE_MAC( 3, _instance, 18),                                                  \
        sff_channel1_PCA_NODE_MAC( 4, _instance, 19),                                                  \
        sff_channel1_PCA_NODE_MAC( 5, _instance, 20),                                                  \
        sff_channel1_PCA_NODE_MAC( 6, _instance, 21),                                                  \
        sff_channel1_PCA_NODE_MAC( 7, _instance, 22),                                                  \
        sff_channel1_PCA_NODE_MAC( 8, _instance, 23),                                                  \
        sff_channel1_PCA_NODE_MAC( 9, _instance, 24),                                                  \
        sff_channel1_PCA_NODE_MAC(10, _instance, 25),                                                  \
        sff_channel1_PCA_NODE_MAC(11, _instance, 26),                                                  \
        sff_channel1_PCA_NODE_MAC(12, _instance, 27),                                                  \
        sff_channel1_PCA_NODE_MAC(13, _instance, 28),                                                  \
        sff_channel1_PCA_NODE_MAC(14, _instance, 29),                                                  \
        sff_channel1_PCA_NODE_MAC(15, _instance, 30),                                                  \
        sff_channel1_PCA_NODE_MAC(16, _instance, 31)

#define sff_channel_group0_PCA_SUB_TREE_MAC(bit, _instance)                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        0x00000030, 0x00000034,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_SFF_INSTANCE_##_instance##_CH_GRP0_SUMMARY_INTERRUPT_CAUSE_CHID_0_CH_GRP0_SUM_INTERRUPT_CAUSE_E,  \
        PRV_CPSS_HARRIER_SFF_INSTANCE_##_instance##_CH_GRP0_SUMMARY_INTERRUPT_CAUSE_CHID_15_CH_GRP0_SUM_INTERRUPT_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_0_CHANNEL_SFF_16_PCA_NODES_MAC(_instance)

#define sff_channel_group1_PCA_SUB_TREE_MAC(bit, _instance)                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        0x00000030, 0x00000034,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_SFF_INSTANCE_##_instance##_CH_GRP1_SUMMARY_INTERRUPT_CAUSE_CHID_16_CH_GRP1_SUM_INTERRUPT_CAUSE_E,  \
        PRV_CPSS_HARRIER_SFF_INSTANCE_##_instance##_CH_GRP1_SUMMARY_INTERRUPT_CAUSE_CHID_31_CH_GRP1_SUM_INTERRUPT_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_1_CHANNEL_SFF_16_PCA_NODES_MAC(_instance)

#define sff_PCA_GLOBAL_NODE_MAC(bit, _instance)                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        0x00000010, 0x00000014,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_SFF_INSTANCE_##_instance##_GLOBAL_INTERRUPT_CAUSE_SOP_MISMATCH_INTERRUPT_CAUSE_E,     \
        PRV_CPSS_HARRIER_SFF_INSTANCE_##_instance##_GLOBAL_INTERRUPT_CAUSE_BAD_ADDRESS_INTERRUPT_CAUSE_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define sff_PCA_SUB_TREE_MAC(bit, _instance)                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        0x00000020, 0x00000024,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_SFF_INSTANCE_##_instance##_SFF_UNIT_SUMMARY_CAUSE_GLOB_INTERRUPT_SUMMARY_CAUSE_E,     \
        PRV_CPSS_HARRIER_SFF_INSTANCE_##_instance##_SFF_UNIT_SUMMARY_CAUSE_CH_GROUP1_INTERRUPT_SUMMARY_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                         \
        sff_PCA_GLOBAL_NODE_MAC(1, _instance),                                                          \
        sff_channel_group0_PCA_SUB_TREE_MAC(2, _instance),                                              \
        sff_channel_group1_PCA_SUB_TREE_MAC(3, _instance)

/* SFF Group 0 Channel - cause */
#define SFF_GR0_CHANNEL_INTERRUPT_CAUSE_MAC(_channel)                                             \
        (0x0200 + ((_channel) * 0x4))

/* SFF Group 0 Channel - mask */
#define SFF_GR0_CHANNEL_INTERRUPT_MASK_MAC(_channel)                                              \
        (0x0400 + ((_channel) * 0x4))

/* SFF Group 1 Channel - cause */
#define SFF_GR1_CHANNEL_INTERRUPT_CAUSE_MAC(_channel)                                             \
        (0x0280 + ((_channel) * 0x4))

/* SFF Group 1 Channel - mask */
#define SFF_GR1_CHANNEL_INTERRUPT_MASK_MAC(_channel)                                                    \
        (0x0480 + ((_channel) * 0x4))

#define sff_channel0_MSEC_NODE_MAC(bit, _instance, channel)                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        SFF_GR0_CHANNEL_INTERRUPT_CAUSE_MAC(channel),                                                   \
        SFF_GR0_CHANNEL_INTERRUPT_MASK_MAC(channel),                                                    \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CHID_15_0_##channel##_CH_OCCUP_CNT_OF_INTERRUPT_CAUSE_E,  \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CHID_15_0_##channel##_CH_CRT_TOK0_ACCU_OF_INTERRUPT_CAUSE_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define sff_channel1_MSEC_NODE_MAC(bit, _instance, channel)                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        SFF_GR1_CHANNEL_INTERRUPT_CAUSE_MAC((channel % 16)),                                            \
        SFF_GR1_CHANNEL_INTERRUPT_MASK_MAC((channel % 16)),                                             \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CHID_31_16_##channel##_CH_OCCUP_CNT_OF_INTERRUPT_CAUSE_E, \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CHID_31_16_##channel##_CH_CRT_TOK0_ACCU_OF_INTERRUPT_CAUSE_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define GROUP_0_CHANNEL_SFF_16_MSEC_NODES_MAC(_instance)                                                 \
        sff_channel0_MSEC_NODE_MAC( 1, _instance,  0),                                                   \
        sff_channel0_MSEC_NODE_MAC( 2, _instance,  1),                                                   \
        sff_channel0_MSEC_NODE_MAC( 3, _instance,  2),                                                   \
        sff_channel0_MSEC_NODE_MAC( 4, _instance,  3),                                                   \
        sff_channel0_MSEC_NODE_MAC( 5, _instance,  4),                                                   \
        sff_channel0_MSEC_NODE_MAC( 6, _instance,  5),                                                   \
        sff_channel0_MSEC_NODE_MAC( 7, _instance,  6),                                                   \
        sff_channel0_MSEC_NODE_MAC( 8, _instance,  7),                                                   \
        sff_channel0_MSEC_NODE_MAC( 9, _instance,  8),                                                   \
        sff_channel0_MSEC_NODE_MAC(10, _instance,  9),                                                   \
        sff_channel0_MSEC_NODE_MAC(11, _instance, 10),                                                   \
        sff_channel0_MSEC_NODE_MAC(12, _instance, 11),                                                   \
        sff_channel0_MSEC_NODE_MAC(13, _instance, 12),                                                   \
        sff_channel0_MSEC_NODE_MAC(14, _instance, 13),                                                   \
        sff_channel0_MSEC_NODE_MAC(15, _instance, 14),                                                   \
        sff_channel0_MSEC_NODE_MAC(16, _instance, 15)

#define GROUP_1_CHANNEL_SFF_16_MSEC_NODES_MAC(_instance)                                                 \
        sff_channel1_MSEC_NODE_MAC( 1, _instance, 16),                                                   \
        sff_channel1_MSEC_NODE_MAC( 2, _instance, 17),                                                   \
        sff_channel1_MSEC_NODE_MAC( 3, _instance, 18),                                                   \
        sff_channel1_MSEC_NODE_MAC( 4, _instance, 19),                                                   \
        sff_channel1_MSEC_NODE_MAC( 5, _instance, 20),                                                   \
        sff_channel1_MSEC_NODE_MAC( 6, _instance, 21),                                                   \
        sff_channel1_MSEC_NODE_MAC( 7, _instance, 22),                                                   \
        sff_channel1_MSEC_NODE_MAC( 8, _instance, 23),                                                   \
        sff_channel1_MSEC_NODE_MAC( 9, _instance, 24),                                                   \
        sff_channel1_MSEC_NODE_MAC(10, _instance, 25),                                                   \
        sff_channel1_MSEC_NODE_MAC(11, _instance, 26),                                                   \
        sff_channel1_MSEC_NODE_MAC(12, _instance, 27),                                                   \
        sff_channel1_MSEC_NODE_MAC(13, _instance, 28),                                                   \
        sff_channel1_MSEC_NODE_MAC(14, _instance, 29),                                                   \
        sff_channel1_MSEC_NODE_MAC(15, _instance, 30),                                                   \
        sff_channel1_MSEC_NODE_MAC(16, _instance, 31)

#define sff_channel_group0_MSEC_SUB_TREE_MAC(bit, _instance)                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        0x00000030, 0x00000034,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CH_GRP0_SUMMARY_INTERRUPT_CAUSE_CHID_0_CH_GRP0_SUM_INTERRUPT_CAUSE_E,  \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CH_GRP0_SUMMARY_INTERRUPT_CAUSE_CHID_15_CH_GRP0_SUM_INTERRUPT_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_0_CHANNEL_SFF_16_MSEC_NODES_MAC(_instance)

#define sff_channel_group1_MSEC_SUB_TREE_MAC(bit, _instance)                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        0x00000030, 0x00000034,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CH_GRP1_SUMMARY_INTERRUPT_CAUSE_CHID_16_CH_GRP1_SUM_INTERRUPT_CAUSE_E, \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CH_GRP1_SUMMARY_INTERRUPT_CAUSE_CHID_31_CH_GRP1_SUM_INTERRUPT_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_1_CHANNEL_SFF_16_MSEC_NODES_MAC(_instance)

#define sff_MSEC_GLOBAL_NODE_MAC(bit, _instance)                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        0x00000010, 0x00000014,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_GLOBAL_INTERRUPT_CAUSE_SOP_MISMATCH_INTERRUPT_CAUSE_E,    \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_GLOBAL_INTERRUPT_CAUSE_BAD_ADDRESS_INTERRUPT_CAUSE_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define sff_MSEC_SUB_TREE_MAC(bit, _instance)                                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        0x00000020, 0x00000024,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_SFF_UNIT_SUMMARY_CAUSE_GLOB_INTERRUPT_SUMMARY_CAUSE_E,    \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_SFF_UNIT_SUMMARY_CAUSE_CH_GROUP1_INTERRUPT_SUMMARY_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                         \
        sff_MSEC_GLOBAL_NODE_MAC(1, _instance),                                                         \
        sff_channel_group0_MSEC_SUB_TREE_MAC(2, _instance),                                             \
        sff_channel_group1_MSEC_SUB_TREE_MAC(3, _instance)

/* BRG TX Channel - cause */
#define BRG_TX_CHANNEL_INTERRUPT_CAUSE_MAC(_channel, _group)                                     \
        (0x1500 + ((_group) * 0x40) + ((_channel) * 0x4))

/* BRG TX Channel - mask */
#define BRG_TX_CHANNEL_INTERRUPT_MASK_MAC(_channel, _group)                                      \
        (0x1700 + ((_group) * 0x40) + ((_channel) * 0x4))


/* BRG RX Channel - cause */
#define BRG_RX_CHANNEL_INTERRUPT_CAUSE_MAC(_channel, _group)                                     \
        (0x200 + ((_group) * 0x80) + ((_channel) * 0x4))

/* BRG RX Channel - mask */
#define BRG_RX_CHANNEL_INTERRUPT_MASK_MAC(_channel, _group)                                      \
        (0x400 + ((_group) * 0x80) + ((_channel) * 0x4))

#define BRG_RX_CHANNEL_GROUP_0_MSEC_16_NODES_MAC(_instance)                                             \
        brg_rx_channel_MSEC_NODE_MAC( 1, _instance,  0, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC( 2, _instance,  1, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC( 3, _instance,  2, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC( 4, _instance,  3, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC( 5, _instance,  4, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC( 6, _instance,  5, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC( 7, _instance,  6, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC( 8, _instance,  7, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC( 9, _instance,  8, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(10, _instance,  9, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(11, _instance, 10, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(12, _instance, 11, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(13, _instance, 12, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(14, _instance, 13, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(15, _instance, 14, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(16, _instance, 15, 0)

#define BRG_RX_CHANNEL_GROUP_1_MSEC_16_NODES_MAC(_instance)                                             \
        brg_rx_channel_MSEC_NODE_MAC( 1, _instance, 16, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC( 2, _instance, 17, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC( 3, _instance, 18, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC( 4, _instance, 19, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC( 5, _instance, 20, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC( 6, _instance, 21, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC( 7, _instance, 22, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC( 8, _instance, 23, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC( 9, _instance, 24, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(10, _instance, 25, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(11, _instance, 26, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(12, _instance, 27, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(13, _instance, 28, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(14, _instance, 29, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(15, _instance, 30, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(16, _instance, 31, 1)

#define brg_rx_channel_MSEC_NODE_MAC(bit, _instance, _channel, _group)                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        BRG_RX_CHANNEL_INTERRUPT_CAUSE_MAC(_channel, _group),                                              \
        BRG_RX_CHANNEL_INTERRUPT_MASK_MAC(_channel, _group),                                               \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CHANNEL_##_channel##_RX_SDB_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CHANNEL_##_channel##_RX_SDB_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_rx_global_MSEC_NODE_MAC(bit, _instance)                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00000750, 0x00000754,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_RX_GLOBAL_INTERRUPT_CAUSE_RX_ROTATED_FIFO_OVERRUN_INTERRUPT_E,\
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_RX_GLOBAL_INTERRUPT_CAUSE_RX_SDB_CHANNEL_ID_ERROR_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_rx_channel_group0_MSEC_SUB_TREE_MAC(bit, _instance)                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00000600, 0x00000620,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_RX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,\
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_RX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_RX_CHANNEL_GROUP_0_MSEC_16_NODES_MAC(_instance)

#define brg_rx_channel_group1_MSEC_SUB_TREE_MAC(bit, _instance)                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00000604, 0x00000624,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_RX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,\
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_RX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_RX_CHANNEL_GROUP_1_MSEC_16_NODES_MAC(_instance)

#define BRG_TX_CHANNEL_GROUP_0_MSEC_16_NODES_MAC(_instance)                                             \
        brg_tx_channel_MSEC_NODE_MAC( 1, _instance,  0, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC( 2, _instance,  1, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC( 3, _instance,  2, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC( 4, _instance,  3, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC( 5, _instance,  4, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC( 6, _instance,  5, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC( 7, _instance,  6, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC( 8, _instance,  7, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC( 9, _instance,  8, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(10, _instance,  9, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(11, _instance, 10, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(12, _instance, 11, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(13, _instance, 12, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(14, _instance, 13, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(15, _instance, 14, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(16, _instance, 15, 0)

#define BRG_TX_CHANNEL_GROUP_1_MSEC_16_NODES_MAC(_instance)                                             \
        brg_tx_channel_MSEC_NODE_MAC( 1, _instance, 16, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC( 2, _instance, 17, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC( 3, _instance, 18, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC( 4, _instance, 19, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC( 5, _instance, 20, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC( 6, _instance, 21, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC( 7, _instance, 22, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC( 8, _instance, 23, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC( 9, _instance, 24, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(10, _instance, 25, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(11, _instance, 26, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(12, _instance, 27, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(13, _instance, 28, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(14, _instance, 29, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(15, _instance, 30, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(16, _instance, 31, 1)

#define brg_tx_channel_MSEC_NODE_MAC(bit, _instance, _channel, _group)                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        BRG_TX_CHANNEL_INTERRUPT_CAUSE_MAC(_channel, _group),                                               \
        BRG_TX_CHANNEL_INTERRUPT_MASK_MAC(_channel, _group),                                                \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CHANNEL_##_channel##_TX_SLICE_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,   \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_CHANNEL_##_channel##_TX_SDB_CREDIT_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_tx_global_MSEC_NODE_MAC(bit, _instance)                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00001a64, 0x00001a68,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_TX_GLOBAL_INTERRUPT_CAUSE_ACCUM_MEMORY_FULL_E,            \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_TX_GLOBAL_INTERRUPT_CAUSE_TX_CREDIT_FIFO_OVERRUN_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_tx_channel_group0_MSEC_SUB_TREE_MAC(bit, _instance)                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00001900, 0x00001920,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_TX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,\
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_TX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_TX_CHANNEL_GROUP_0_MSEC_16_NODES_MAC(_instance)

#define brg_tx_channel_group1_MSEC_SUB_TREE_MAC(bit, _instance)                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00001904, 0x00001924,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_TX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,\
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_TX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_TX_CHANNEL_GROUP_1_MSEC_16_NODES_MAC(_instance)

#define brg_MSEC_NODE_MAC(bit, _instance)                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00000004, 0x00000008,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_BRG_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,             \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_BRG_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_MSEC_SUB_TREE_MAC(bit, _instance)                                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x0000000c, 0x00000010,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_BRG_INTERRUPT_CAUSE_SUMMARY_RX_GLOBAL_INTERRUPT_SUMMARY_E,\
        PRV_CPSS_HARRIER_MSEC_INST_##_instance##_BRG_INTERRUPT_CAUSE_SUMMARY_BRIDGE_CAUSE_INTERRUPT_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 7, NULL, NULL},                                         \
        brg_rx_global_MSEC_NODE_MAC(1, _instance),                                                      \
        brg_rx_channel_group0_MSEC_SUB_TREE_MAC(2, _instance),                                          \
        brg_rx_channel_group1_MSEC_SUB_TREE_MAC(3, _instance),                                          \
        brg_tx_global_MSEC_NODE_MAC(6, _instance),                                                      \
        brg_tx_channel_group0_MSEC_SUB_TREE_MAC(7, _instance),                                          \
        brg_tx_channel_group1_MSEC_SUB_TREE_MAC(8, _instance),                                          \
        brg_MSEC_NODE_MAC(9, _instance)

#define BRG_RX_CHANNEL_GROUP_0_PCA_16_NODES_MAC(_instance)                                             \
        brg_rx_channel_PCA_NODE_MAC( 1, _instance,  0, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC( 2, _instance,  1, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC( 3, _instance,  2, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC( 4, _instance,  3, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC( 5, _instance,  4, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC( 6, _instance,  5, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC( 7, _instance,  6, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC( 8, _instance,  7, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC( 9, _instance,  8, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(10, _instance,  9, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(11, _instance, 10, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(12, _instance, 11, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(13, _instance, 12, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(14, _instance, 13, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(15, _instance, 14, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(16, _instance, 15, 0)

#define BRG_RX_CHANNEL_GROUP_1_PCA_16_NODES_MAC(_instance)                                             \
        brg_rx_channel_PCA_NODE_MAC( 1, _instance, 16, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC( 2, _instance, 17, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC( 3, _instance, 18, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC( 4, _instance, 19, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC( 5, _instance, 20, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC( 6, _instance, 21, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC( 7, _instance, 22, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC( 8, _instance, 23, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC( 9, _instance, 24, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(10, _instance, 25, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(11, _instance, 26, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(12, _instance, 27, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(13, _instance, 28, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(14, _instance, 29, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(15, _instance, 30, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(16, _instance, 31, 1)

#define brg_rx_channel_PCA_NODE_MAC(bit, _instance, _channel, _group)                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        BRG_RX_CHANNEL_INTERRUPT_CAUSE_MAC(_channel, _group),                                    \
        BRG_RX_CHANNEL_INTERRUPT_MASK_MAC(_channel, _group),                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_CHANNEL_##_channel##_RX_SDB_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_CHANNEL_##_channel##_RX_SDB_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_rx_global_PCA_NODE_MAC(bit, _instance)                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00000750, 0x00000754,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_RX_GLOBAL_INTERRUPT_CAUSE_RX_ROTATED_FIFO_OVERRUN_INTERRUPT_E,\
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_RX_GLOBAL_INTERRUPT_CAUSE_RX_SDB_CHANNEL_ID_ERROR_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_rx_channel_group0_PCA_SUB_TREE_MAC(bit, _instance)                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00000600, 0x00000620,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_RX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,\
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_RX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_RX_CHANNEL_GROUP_0_PCA_16_NODES_MAC(_instance)

#define brg_rx_channel_group1_PCA_SUB_TREE_MAC(bit, _instance)                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00000604, 0x00000624,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_RX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,\
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_RX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_RX_CHANNEL_GROUP_1_PCA_16_NODES_MAC(_instance)

#define BRG_TX_CHANNEL_GROUP_0_PCA_16_NODES_MAC(_instance)                                             \
        brg_tx_channel_PCA_NODE_MAC( 1, _instance,  0, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC( 2, _instance,  1, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC( 3, _instance,  2, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC( 4, _instance,  3, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC( 5, _instance,  4, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC( 6, _instance,  5, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC( 7, _instance,  6, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC( 8, _instance,  7, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC( 9, _instance,  8, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(10, _instance,  9, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(11, _instance, 10, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(12, _instance, 11, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(13, _instance, 12, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(14, _instance, 13, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(15, _instance, 14, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(16, _instance, 15, 0)

#define BRG_TX_CHANNEL_GROUP_1_PCA_16_NODES_MAC(_instance)                                             \
        brg_tx_channel_PCA_NODE_MAC( 1, _instance, 16, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC( 2, _instance, 17, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC( 3, _instance, 18, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC( 4, _instance, 19, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC( 5, _instance, 20, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC( 6, _instance, 21, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC( 7, _instance, 22, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC( 8, _instance, 23, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC( 9, _instance, 24, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(10, _instance, 25, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(11, _instance, 26, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(12, _instance, 27, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(13, _instance, 28, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(14, _instance, 29, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(15, _instance, 30, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(16, _instance, 31, 1)

#define brg_tx_channel_PCA_NODE_MAC(bit, _instance, _channel, _group)                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                  \
        BRG_TX_CHANNEL_INTERRUPT_CAUSE_MAC(_channel, _group),                                   \
        BRG_TX_CHANNEL_INTERRUPT_MASK_MAC(_channel, _group),                                    \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                               \
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_CHANNEL_##_channel##_TX_SLICE_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,   \
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_CHANNEL_##_channel##_TX_SDB_CREDIT_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_tx_global_PCA_NODE_MAC(bit, _instance)                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00001a64, 0x00001a68,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_TX_GLOBAL_INTERRUPT_CAUSE_ACCUM_MEMORY_FULL_E,             \
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_TX_GLOBAL_INTERRUPT_CAUSE_TX_CREDIT_FIFO_OVERRUN_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_tx_channel_group0_PCA_SUB_TREE_MAC(bit, _instance)                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00001900, 0x00001920,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_TX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,\
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_TX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_TX_CHANNEL_GROUP_0_PCA_16_NODES_MAC(_instance)

#define brg_tx_channel_group1_PCA_SUB_TREE_MAC(bit, _instance)                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00001904, 0x00001924,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_TX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,\
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_TX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_TX_CHANNEL_GROUP_1_PCA_16_NODES_MAC(_instance)

#define brg_PCA_NODE_MAC(bit, _instance)                                                                \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00000004, 0x00000008,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_BRG_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,              \
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_BRG_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_PCA_SUB_TREE_MAC(bit, _instance)                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x0000000c, 0x00000010,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_BRG_INTERRUPT_CAUSE_SUMMARY_RX_GLOBAL_INTERRUPT_SUMMARY_E, \
        PRV_CPSS_HARRIER_PCA_INST_##_instance##_BRG_INTERRUPT_CAUSE_SUMMARY_BRIDGE_CAUSE_INTERRUPT_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 7, NULL, NULL},                                         \
        brg_rx_global_PCA_NODE_MAC(1, _instance),                                                       \
        brg_rx_channel_group0_PCA_SUB_TREE_MAC(2, _instance),                                           \
        brg_rx_channel_group1_PCA_SUB_TREE_MAC(3, _instance),                                           \
        brg_tx_global_PCA_NODE_MAC(6, _instance),                                                       \
        brg_tx_channel_group0_PCA_SUB_TREE_MAC(7, _instance),                                           \
        brg_tx_channel_group1_PCA_SUB_TREE_MAC(8, _instance),                                           \
        brg_PCA_NODE_MAC(9, _instance)

#define sdw_NODE_MAC(bit, _instance)                                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_SDW0_E + _instance, NULL,                                      \
        0x00000300, 0x00000304,                                                                         \
        prvCpssDrvHwPpHarrierSerdesIsrRead, /* use dedicated callback */                                \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_SDW_INSTANCE_##_instance##_SDW_INTERRUPT_CAUSE_SER_DES_INTERRUPT_E,            \
        PRV_CPSS_HARRIER_SDW_INSTANCE_##_instance##_SDW_INTERRUPT_CAUSE_SQ_DETECT_3_E,                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_EIP_163_GLOBAL_NODE_MAC(bit, _unit, _prefix, _type, _instance)                                            \
    /* Classifier Engine(EIP-163) Egress Interrupts Cause */                                            \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x0000FC2C, 0x0000FC08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E,    \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_CHANNEL_IRQ_15_31_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_EIP_163_GLOBAL_NODE_MAC(bit, _unit, _prefix, _type, _instance)                                            \
    /* Classifier Engine(EIP-163) Ingress Interrupts Cause */                                           \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x0000FC2C, 0x0000FC08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E, \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_CHANNEL_IRQ_15_31_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_EIP_66_NODE_MAC(bit, _unit, _prefix, _type, _instance)                                                    \
    /* MACSec Transform Crypto Engine(EIP-66) Egress Interrupts Cause */                                \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x00005C2C, 0x00005C08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_0_E, \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SEQ_NUM_ROLLOVER_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_EIP_66_NODE_MAC(bit, _unit, _prefix, _type, _instance)                                                    \
    /* MACSec Transform Crypto Engine(EIP-66) Ingress Interrupts Cause */                               \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x00005C2C, 0x00005C08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_0_E, \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_14_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_EIP_164_GLOBAL_NODE_MAC(bit, _unit, _prefix, _type, _instance)                                            \
    /* Transformer Engine(EIP-164) Egress Interrupts Cause */                                           \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x0000FC2C, 0x0000FC08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_DROP_CLASS_E, \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_CHANNEL_IRQ_15_31_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                             \
        msec_EGR_EIP_66_NODE_MAC(3, _unit, EIP66E, _type, _instance)

#define msec_ING_EIP_164_GLOBAL_NODE_MAC(bit, _unit, _prefix, _type, _instance)                                            \
    /* Transformer Engine(EIP-164) Ingress Interrupts Cause */                                          \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x0000FC2C, 0x0000FC08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_DROP_CLASS_E, \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_CHANNEL_IRQ_15_31_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                               \
        msec_ING_EIP_66_NODE_MAC(3, _unit, EIP66I, _type, _instance)

#define msec_ING_CH0_NODE_MAC(bit, _unit, _prefix, _type, _instance)                                    \
    /* MacSec Channel Interrupts 0 Cause */                                                             \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x00000338, 0x0000033C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_0_CHANNEL_INTERRUPT_E, \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_15_CHANNEL_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_CH1_NODE_MAC(bit, _unit, _prefix, _type, _instance)                                    \
    /* MacSec Channel Interrupts 1 Cause */                                                             \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x00000340, 0x00000344,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_16_CHANNEL_INTERRUPT_E, \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_31_CHANNEL_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_CH0_NODE_MAC(bit, _unit, _prefix, _type, _instance)                                    \
    /* MacSec Channel Interrupts 0 Cause */                                                             \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x00000338, 0x0000033c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_0_CHANNEL_INTERRUPT_E, \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_15_CHANNEL_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_CH1_NODE_MAC(bit, _unit, _prefix, _type, _instance)                                    \
    /* MacSec Channel Interrupts 1 Cause */                                                             \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x00000340, 0x00000344,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_16_CHANNEL_INTERRUPT_E, \
        PRV_CPSS_HARRIER_##_prefix##_MACSEC_TYPE_##_type##_##_prefix##_MACSEC_INSTANCE_##_instance##_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_31_CHANNEL_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_163_SUB_TREE_MAC(bit, _unit, _instance)                                                \
    /* MSEC Egress 163 */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_##_unit##_E,  NULL,                         \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_EIP163E_MACSEC_TYPE_1_EIP163E_MACSEC_INSTANCE_##_instance##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E,          \
        PRV_CPSS_HARRIER_EIP163E_MACSEC_TYPE_1_EIP163E_MACSEC_INSTANCE_##_instance##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_1_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                         \
        msec_EGR_EIP_163_GLOBAL_NODE_MAC(1, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_##_unit##_E, EIP163E, 1, _instance), \
        msec_EGR_CH0_NODE_MAC(4, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_##_unit##_E, EIP163E, 1, _instance), \
        msec_EGR_CH1_NODE_MAC(5, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_##_unit##_E, EIP163E, 1, _instance)

#define msec_ING_163_SUB_TREE_MAC(bit, _unit, _instance)                                                \
    /* MSEC Ingress 163 */                                                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_##_unit##_E,  NULL,                         \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_EIP163I_MACSEC_TYPE_0_EIP163I_MACSEC_INSTANCE_##_instance##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E, \
        PRV_CPSS_HARRIER_EIP163I_MACSEC_TYPE_0_EIP163I_MACSEC_INSTANCE_##_instance##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_1_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                         \
        msec_ING_EIP_163_GLOBAL_NODE_MAC(1, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_##_unit##_E, EIP163I, 0, _instance), \
        msec_ING_CH0_NODE_MAC(4, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_##_unit##_E, EIP163I, 0, _instance), \
        msec_ING_CH1_NODE_MAC(5, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_##_unit##_E, EIP163I, 0, _instance)

#define msec_EGR_164_SUB_TREE_MAC(bit, _unit, _instance)                                                \
    /* MSEC Egress 164 */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_##_unit##_E,  NULL,                         \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_EIP164E_MACSEC_TYPE_3_EIP164E_MACSEC_INSTANCE_##_instance##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E, \
        PRV_CPSS_HARRIER_EIP164E_MACSEC_TYPE_3_EIP164E_MACSEC_INSTANCE_##_instance##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_1_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                         \
        msec_EGR_EIP_164_GLOBAL_NODE_MAC(1, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_##_unit##_E, EIP164E, 3, _instance), \
        msec_EGR_CH0_NODE_MAC(4, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_##_unit##_E, EIP164E, 3, _instance), \
        msec_EGR_CH1_NODE_MAC(5, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_##_unit##_E, EIP164E, 3, _instance)

#define msec_ING_164_SUB_TREE_MAC(bit, _unit, _instance)                                                \
    /* MSEC Egress 164 */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_##_unit##_E,  NULL,                         \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_EIP164I_MACSEC_TYPE_2_EIP164I_MACSEC_INSTANCE_##_instance##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E, \
        PRV_CPSS_HARRIER_EIP164I_MACSEC_TYPE_2_EIP164I_MACSEC_INSTANCE_##_instance##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_1_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                         \
        msec_ING_EIP_164_GLOBAL_NODE_MAC(1, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_##_unit##_E, EIP164I, 2, _instance), \
        msec_ING_CH0_NODE_MAC(4, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_##_unit##_E, EIP164I, 2, _instance), \
        msec_ING_CH1_NODE_MAC(5, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_##_unit##_E, EIP164I, 2, _instance)

#define no_msec_EGR_163_SUB_TREE_MAC(bit, _unit, _instance)                                             \
    /* MSEC Egress 163 */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_##_unit##_E,  NULL,                         \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_EIP163E_MACSEC_TYPE_1_EIP163E_MACSEC_INSTANCE_##_instance##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E,          \
        PRV_CPSS_HARRIER_EIP163E_MACSEC_TYPE_1_EIP163E_MACSEC_INSTANCE_##_instance##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_1_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
        msec_EGR_CH0_NODE_MAC(4, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_##_unit##_E, EIP163E, 1, _instance), \
        msec_EGR_CH1_NODE_MAC(5, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_##_unit##_E, EIP163E, 1, _instance)

#define no_msec_ING_163_SUB_TREE_MAC(bit, _unit, _instance)                                             \
    /* MSEC Ingress 163 */                                                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_##_unit##_E,  NULL,                         \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_EIP163I_MACSEC_TYPE_0_EIP163I_MACSEC_INSTANCE_##_instance##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E, \
        PRV_CPSS_HARRIER_EIP163I_MACSEC_TYPE_0_EIP163I_MACSEC_INSTANCE_##_instance##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_1_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
        msec_ING_CH0_NODE_MAC(4, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_##_unit##_E, EIP163I, 0, _instance), \
        msec_ING_CH1_NODE_MAC(5, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_##_unit##_E, EIP163I, 0, _instance)

#define no_msec_EGR_164_SUB_TREE_MAC(bit, _unit, _instance)                                             \
    /* MSEC Egress 164 */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_##_unit##_E,  NULL,                         \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_EIP164E_MACSEC_TYPE_3_EIP164E_MACSEC_INSTANCE_##_instance##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E, \
        PRV_CPSS_HARRIER_EIP164E_MACSEC_TYPE_3_EIP164E_MACSEC_INSTANCE_##_instance##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_1_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
        msec_EGR_CH0_NODE_MAC(4, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_##_unit##_E, EIP164E, 3, _instance), \
        msec_EGR_CH1_NODE_MAC(5, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_##_unit##_E, EIP164E, 3, _instance)

#define no_msec_ING_164_SUB_TREE_MAC(bit, _unit, _instance)                                             \
    /* MSEC Egress 164 */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_##_unit##_E,  NULL,                         \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_EIP164I_MACSEC_TYPE_2_EIP164I_MACSEC_INSTANCE_##_instance##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E, \
        PRV_CPSS_HARRIER_EIP164I_MACSEC_TYPE_2_EIP164I_MACSEC_INSTANCE_##_instance##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_1_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
        msec_ING_CH0_NODE_MAC(4, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_##_unit##_E, EIP164I, 2, _instance), \
        msec_ING_CH1_NODE_MAC(5, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_##_unit##_E, EIP164I, 2, _instance)

#define   hbu_NODE_MAC(bit)                                                                             \
    /* HBU */                                                                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_HBU_E, NULL,                                                     \
        0x00000000, 0x00000004,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_HBU_HBU_INTERRUPT_CAUSE_REGISTER_ERROR_FHF_MEM_E,                              \
        PRV_CPSS_HARRIER_HBU_HBU_INTERRUPT_CAUSE_REGISTER_HEADER_FIFO_EMPTY_E,                          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define cnm_grp_0_0_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                 \
    /* FuncUnitsIntsSum Interrupt Cause */                                              \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                            \
        0x000003f8, 0x000003fc,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_HARRIER_MG_##_mgIndex##_FUNCTIONAL_INTERRUPTS_SUMMARY_CAUSE_CP_TOP_EM_INT_SUM_E,    \
        PRV_CPSS_HARRIER_MG_##_mgIndex##_FUNCTIONAL_INTERRUPTS_SUMMARY_CAUSE_CP_TOP_PIPE0_QAG_INT_SUM_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 29, NULL, NULL},                                 \
        em_NODE_MAC(1),                                                                 \
        epcl_tai_NODE_MAC(2, 0),                                                        \
        epcl_tai_NODE_MAC(3, 1),                                                        \
        mt_NODE_MAC(4),                                                                 \
        pha_tai_NODE_MAC(5, 0),                                                         \
        pha_tai_NODE_MAC(6, 1),                                                         \
        /* bma_SUB_TREE_MAC(7), */                                                      \
        cnc_SUB_TREE_MAC(8),                                                            \
        eft_NODE_MAC(9),                                                                \
        eoam_NODE_MAC(10),                                                              \
        epcl_NODE_MAC(11),                                                              \
        eplr_NODE_MAC(12),                                                              \
        eq_SUB_TREE_MAC(13),                                                            \
        erep_NODE_MAC(14),                                                              \
        ermrk_NODE_MAC(15),                                                             \
        ha_NODE_MAC(16),                                                                \
        /* hbu_SUB_TREE_MAC(17), */                                                     \
        ioam_NODE_MAC(18),                                                              \
        iplr0_NODE_MAC(19),                                                             \
        iplr1_NODE_MAC(20),                                                             \
        router_NODE_MAC(21),                                                            \
        l2i_NODE_MAC(22),                                                               \
        lpm_NODE_MAC(23),                                                               \
        mll_NODE_MAC(24),                                                               \
        pcl_NODE_MAC(25),                                                               \
        ppu_NODE_MAC(26),                                                               \
        preq_NODE_MAC(27),                                                              \
        qag_NODE_MAC(28),                                                               \
        sht_NODE_MAC(29),                                                               \
        pha_SUB_TREE_MAC(30),                                                           \
        tti_NODE_MAC(31)


#define cnm_grp_0_1_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                             \
    /* DataPathIntsSum Interrupt Cause */                                                           \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                        \
        0x000000a4, 0x000000a8,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_MG_##_mgIndex##_DATA_PATH_INTERRUPTS_SUMMARY_CAUSE_CP_TOP_PIPE0_SHT_INT_SUM_E, \
        PRV_CPSS_HARRIER_MG_##_mgIndex##_DATA_PATH_INTERRUPTS_SUMMARY_CAUSE_GOP_DOWNLINK_TOP_EPI0_HAWK_EPI_USX_MAC_TAI_INTERRUPT_CAUSE_INT_SUM1_E,  \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 31, NULL, NULL},                                             \
        tcam_SUB_TREE_MAC(1),                                                                       \
        tti_tai_NODE_MAC(2, 0),                                                                     \
        tti_tai_NODE_MAC(3, 1),                                                                     \
        l2i_tai_SUB_TREE_MAC(4, 0),                                                                 \
        l2i_tai_SUB_TREE_MAC(5, 1),                                                                 \
        ia_NODE_MAC(6),                                                                             \
        pb_SUB_TREE_MAC(7),                                                                         \
        rxDma_NODE_MAC(8,  0),                                                                      \
        rxDma_NODE_MAC(9,  1),                                                                      \
        rxDma_NODE_MAC(10, 2),                                                                      \
        txDma_NODE_MAC(11, 0),                                                                      \
        txFifo_NODE_MAC(12, 0),                                                                     \
        txDma_NODE_MAC(13, 1),                                                                      \
        txFifo_NODE_MAC(14, 1),                                                                     \
        txDma_NODE_MAC(15, 2),                                                                      \
        txFifo_NODE_MAC(16, 2),                                                                     \
        anp_SUB_TREE_MAC(17, 0),                                                                    \
        led_NODE_MAC(18, 0),                                                                        \
        mac_pcs_SUB_TREE_MAC(19, 0),                                                                \
        mif_NODE_MAC(20, 0),                                                                        \
        sdw_NODE_MAC(21, 0),                                                                        \
        sdw_NODE_MAC(22, 1),                                                                        \
        anp_SUB_TREE_MAC(23, 1),                                                                    \
        led_NODE_MAC(24, 1),                                                                        \
        mac_pcs_SUB_TREE_MAC(25, 1),                                                                \
        mif_NODE_MAC(26, 1),                                                                        \
        sdw_NODE_MAC(27, 3),                                                                        \
        sdw_NODE_MAC(28, 2),                                                                        \
        anp_half_SUB_TREE_MAC(29, 2),                                                                    \
        led_NODE_MAC(30, 2),                                                                        \
        mac_pcs_GOP2_SUB_TREE_MAC(31)


#define cnm_grp_0_2_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                             \
    /* PortsIntsSum Interrupt Cause */                                                              \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                        \
        0x00000080, 0x00000084,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_HARRIER_MG_##_mgIndex##_PORTS_INTERRUPTS_SUMMARY_CAUSE_GOP_DOWNLINK_TOP_EPI0_LEDS_INT_SUM0_E, \
        PRV_CPSS_HARRIER_MG_##_mgIndex##_PORTS_INTERRUPTS_SUMMARY_CAUSE_GOP_DOWNLINK_TOP_EPI1_MAC_PCS_INT_SUM2_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 28, NULL, NULL},                                             \
        mif_NODE_MAC(1, 2),                                                                         \
        sdw_NODE_MAC(2, 4),                                                                         \
        mac_tai_NODE_MAC(3, 0, 0, 0),                                                               \
        mac_tai_NODE_MAC(4, 0, 1, 1),                                                               \
        mac_tai_NODE_MAC(5, 1, 0, 2),                                                               \
        mac_tai_NODE_MAC(6, 1, 1, 3),                                                               \
        mac_tai_NODE_MAC(7, 2, 0, 4),                                                               \
        mac_tai_NODE_MAC(8, 2, 1, 5),                                                               \
        ctsu_PCA_SUB_TREE_MAC(9, 0),                                                                \
        msec_EGR_163_SUB_TREE_MAC(11, 0, 0),                                                        \
        msec_ING_163_SUB_TREE_MAC(12, 0, 0),                                                        \
        msec_EGR_164_SUB_TREE_MAC(13, 0, 0),                                                        \
        msec_ING_164_SUB_TREE_MAC(14, 0, 0),                                                        \
        lmu_PCA_SUB_TREE_MAC(15, 0),                                                                \
        pzarb_PCA_RPZ_NODE_MAC(16, 0),                                                              \
        pzarb_PCA_TPZ_NODE_MAC(17, 0),                                                              \
        sff_PCA_SUB_TREE_MAC(18, 0),                                                                \
        ctsu_PCA_SUB_TREE_MAC(19, 1),                                                               \
        msec_EGR_163_SUB_TREE_MAC(21, 1, 1),                                                        \
        msec_ING_163_SUB_TREE_MAC(22, 1, 1),                                                        \
        msec_EGR_164_SUB_TREE_MAC(23, 1, 1),                                                        \
        msec_ING_164_SUB_TREE_MAC(24, 1, 1),                                                        \
        lmu_PCA_SUB_TREE_MAC(25, 1),                                                                \
        pzarb_PCA_RPZ_NODE_MAC(26, 1),                                                              \
        pzarb_PCA_TPZ_NODE_MAC(27, 1),                                                              \
        sff_PCA_SUB_TREE_MAC(28, 1),                                                                \
        ctsu_PCA_SUB_TREE_MAC(29, 2),                                                               \
        no_msec_EGR_163_SUB_TREE_MAC(31, 2, 2)

#define cnm_grp_0_3_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                 \
    /* DFX */                                                                                           \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        0x000000ac, 0x000000b0,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MG_##_mgIndex##_DFX_INTERRUPTS_SUMMARY_CAUSE_GOP_DOWNLINK_TOP_EPI1_MAC_PCS_INT_SUM3_E,\
        PRV_CPSS_HARRIER_MG_##_mgIndex##_DFX_INTERRUPTS_SUMMARY_CAUSE_GOP_DOWNLINK_TOP_PCA1_PZARB_TX_INT_SUM_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 29, NULL, NULL},                                                 \
        no_msec_ING_163_SUB_TREE_MAC(1, 2, 2),                                                             \
        no_msec_EGR_164_SUB_TREE_MAC(2, 2, 2),                                                             \
        no_msec_ING_164_SUB_TREE_MAC(3, 2, 2),                                                             \
        lmu_PCA_SUB_TREE_MAC(4, 2),                                                                     \
        pzarb_PCA_RPZ_NODE_MAC(5, 2),                                                             \
        pzarb_PCA_TPZ_NODE_MAC(6, 2),                                                             \
        sff_PCA_SUB_TREE_MAC(7, 2),                                                                     \
        pca_tai_NODE_MAC(8, 2, 0, 0),                                                                   \
        pca_tai_NODE_MAC(9, 2, 1, 1),                                                                   \
        pca_tai_NODE_MAC(10, 0, 0, 2),                                                                  \
        pca_tai_NODE_MAC(11, 0, 1, 3),                                                                  \
        pca_tai_NODE_MAC(12, 1, 0, 4),                                                                  \
        pca_tai_NODE_MAC(13, 1, 1, 5),                                                                  \
        dfx_server_sum_SUB_TREE_MAC(14),                                                                \
        pds_SUB_TREE_MAC(15, 0),                                                                        \
        pds_SUB_TREE_MAC(16, 1),                                                                        \
        pds_SUB_TREE_MAC(17, 2),                                                                        \
        pdx_NODE_MAC(18),                                                                               \
        pfcc_NODE_MAC(19),                                                                              \
        psi_NODE_MAC(20),                                                                               \
        qfc_SUB_TREE_MAC(21, 0),                                                                        \
        qfc_SUB_TREE_MAC(22, 1),                                                                        \
        qfc_SUB_TREE_MAC(23, 2),                                                                        \
        sdq_NODE_MAC(24, 0),                                                                            \
        sdq_NODE_MAC(25, 1),                                                                            \
        sdq_NODE_MAC(26, 2),                                                                            \
        txq_tai_NODE_MAC(27, 1, 0, 2),                                                                  \
        txq_tai_NODE_MAC(28, 0, 0, 0),                                                                  \
        txq_tai_NODE_MAC(29, 0, 1, 1)

#define cnm_grp_0_4_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                 \
    /* DFX1 */                                                                                          \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        0x000000b8, 0x000000bc,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MG_##_mgIndex##_DFX_1_INTERRUPTS_SUMMARY_CAUSE_GOP_DOWNLINK_TOP_PCA1_SFF2IRQ_INT_SUM_E,\
        PRV_CPSS_HARRIER_MG_##_mgIndex##_DFX_1_INTERRUPTS_SUMMARY_CAUSE_GOP_UPLINK_TOP_PCA3_DP2SDB_INTERRUPT_CAUSE_INT_SUM_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

/*  cnm_grp_0_6_IntsSum_SUB_TREE_MAC */
#define cnm_grp_0_6_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                 \
    /* Ports1IntSum */                                                                                  \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        0x00000150, 0x00000154,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MG_##_mgIndex##_PORTS_1_INTERRUPTS_SUMMARY_CAUSE_GPIO_7_0_INTR_E,              \
        PRV_CPSS_HARRIER_MG_##_mgIndex##_PORTS_1_INTERRUPTS_SUMMARY_CAUSE_CNM_XOR2_IRQ_MEM1_E,          \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL},                                                  \
        /* 0..7, GPIO_0_31_Interrupt Cause 0 */                                                         \
        {1, GT_TRUE, 0, prvCpssDrvDxExMxRunitGppIsrCall,                                                \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_HARRIER_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_0_RESERVED_1_E                       \
                PRV_CPSS_HARRIER_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_0_RESERVED_7_E,                      \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 8..15, GPIO_0_31_Interrupt Cause 0 */                                                        \
        {2, GT_TRUE, 8, prvCpssDrvDxExMxRunitGppIsrCall,                                                \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_HARRIER_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_0_RESERVED_8_E,                      \
                PRV_CPSS_HARRIER_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_0_RESERVED_15_E,                     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 16..23, GPIO_0_31_Interrupt Cause 0 */                                                       \
        {3, GT_TRUE, 16, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_HARRIER_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_0_RESERVED_16_E,                     \
                PRV_CPSS_HARRIER_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_0_RESERVED_23_E,                     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 24..31, GPIO_0_31_Interrupt Cause 0 */                                                       \
        {4, GT_TRUE, 24, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_HARRIER_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_0_RESERVED_24_E,                     \
                PRV_CPSS_HARRIER_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_0_RESERVED_31_E,                     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 32..39, GPIO_0_31_Interrupt Cause 1 */                                                       \
        {5, GT_TRUE, 32, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018154,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018158),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_HARRIER_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_1_RESERVED_1_E,                      \
                PRV_CPSS_HARRIER_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_1_RESERVED_7_E,                      \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 40..47, GPIO_0_31_Interrupt Cause 1 */                                                       \
        {6, GT_TRUE, 40, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018154,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018158),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_HARRIER_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_1_RESERVED_8_E,                      \
                PRV_CPSS_HARRIER_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_1_RESERVED_15_E,                     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* hold 5 'native' sons  */
/*  cnm_grp_0_7_IntsSum_single_tile_SUB_TREE_MAC */
#define cnm_grp_0_7_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                 \
    /* Ports2ntSum */                                                                                   \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        0x00000158, 0x0000015C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MG_##_mgIndex##_PORTS_2_INTERRUPTS_SUMMARY_CAUSE_MG0_INT_OUT0_E,                 \
        PRV_CPSS_HARRIER_MG_##_mgIndex##_PORTS_2_INTERRUPTS_SUMMARY_CAUSE_TAI_INTERRUPT_CAUSE_INT_SUM1_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 1, NULL, NULL},                                                  \
        /*MG1*/                                                                                         \
        HARRIER_INTERRUPTS_MG_1_MAC(harrier_MG1_int_sum, 1/*mgUnit*/)

#define HARRIER_MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, numChild, _mgIndex)                             \
    /* Global Interrupt Cause */                                                                        \
    {bitIndexInCaller, GT_FALSE,  MG_UNIT(_mgIndex), NULL,                                              \
        0x00000030, 0x00000034,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_HARRIER_MG_##_mgIndex##_GLOBAL_INTERRUPTS_SUMMARY_CAUSE_PC_IE_INTERRUPT_SUMMARY_E,        \
        PRV_CPSS_HARRIER_MG_##_mgIndex##_GLOBAL_INTERRUPTS_SUMMARY_CAUSE_MG1_INTERNAL_INTERRUPT_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, numChild, NULL, NULL}


#define HARRIER_MG_Tx_SDMA_SUMMARY_MAC(bitIndexInCaller, _mgIndex)             \
        /* Tx SDMA  */                                                      \
        {bitIndexInCaller, GT_FALSE, MG_UNIT(_mgIndex), NULL,               \
            0x00002810, 0x00002818,                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                \
            PRV_CPSS_HARRIER_MG_##_mgIndex##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_TX_BUFFER_QUEUE_0_E, \
            PRV_CPSS_HARRIER_MG_##_mgIndex##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_TX_REJECT_0_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define HARRIER_MG_Rx_SDMA_SUMMARY_MAC(bitIndexInCaller, _mgIndex)             \
        /* Rx SDMA  */                                                      \
        {bitIndexInCaller, GT_FALSE, MG_UNIT(_mgIndex), NULL,               \
            0x0000280C, 0x00002814,                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                \
            PRV_CPSS_HARRIER_MG_##_mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_RX_BUFFER_QUEUE_0_E, \
            PRV_CPSS_HARRIER_MG_##_mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_PACKET_CNT_OF_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define  HARRIER_INTERRUPTS_MG_1_MAC(bitIndexInCaller, _mgId)                          \
    /* Global Interrupt Cause */                                                    \
    HARRIER_MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, 3 /*numChild*/,_mgId/*mgId*/),     \
        /* MG internal Interrupt Cause */                                           \
        mg0InternalIntsSum_SUB_TREE_MAC(7, _mgId /*_mgIndex*/),                     \
        /* Tx SDMA  */                                                              \
        HARRIER_MG_Tx_SDMA_SUMMARY_MAC(8, _mgId /*_mgIndex*/),                         \
        /* Rx SDMA  */                                                              \
        HARRIER_MG_Rx_SDMA_SUMMARY_MAC(9, _mgId /*_mgIndex*/)

#define dfx_server_SUB_TREE_MAC(bit)                                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_DFX_SERVER_E, NULL,                                           \
        0x000F8108, 0x000F810C,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_HARRIER_DFX_SERVER_INTERRUPT_WRONG_ADDRESS_FROM_PIPE_0_E,                           \
        PRV_CPSS_HARRIER_DFX_SERVER_INTERRUPT_EXTERNAL_LOW_TEMPERATURE_THRESHOLD_E,                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define dfx_server_sum_SUB_TREE_MAC(bit)                                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_DFX_SERVER_E, NULL,                                           \
        0x000F8100, 0x000F8104,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_HARRIER_DFX_SERVER_INTERRUPT_SUMMARY_SERVER_INTERRUPT_SUM_E,                        \
        PRV_CPSS_HARRIER_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_7_INTERRUPT_SUM_E,                        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                      \
        dfx_server_SUB_TREE_MAC(1)


#define harrier_MG1_int_sum 4

static const PRV_CPSS_DRV_INTERRUPT_SCAN_STC harrierIntrScanArr[] =
{
    /* Global Interrupt Cause */
    HARRIER_MG_GLOBAL_SUMMARY_MAC(0, 8 /*numChild*/, 0 /*_mgIndex*/),
        /* FuncUnitsIntsSum Interrupt Cause */
        cnm_grp_0_0_IntsSum_SUB_TREE_MAC(3, 0 /*_mgIndex*/),
        /* Data Path Interrupt Summay */
        cnm_grp_0_1_IntsSum_SUB_TREE_MAC(4, 0/*_mgIndex*/),
        /* Ports Summary Interrupt Cause */
        cnm_grp_0_2_IntsSum_SUB_TREE_MAC(5, 0 /*_mgIndex */),
        /* DFX interrupts summary cause */
        cnm_grp_0_3_IntsSum_SUB_TREE_MAC(6, 0 /*_mgIndex*/),
        /* MG internal Interrupt Cause */
        mg0InternalIntsSum_SUB_TREE_MAC(7, 0 /*_mgIndex*/),
        /* Tx SDMA  */
        HARRIER_MG_Tx_SDMA_SUMMARY_MAC(8, 0 /*_mgIndex*/),
        /* Rx SDMA  */
        HARRIER_MG_Rx_SDMA_SUMMARY_MAC(9, 0 /*_mgIndex*/),
        /* Ports 2 Interrupt Summary */
        cnm_grp_0_7_IntsSum_SUB_TREE_MAC(17, 0 /*_mgIndex*/)
};

/* number of elements in the array of harrierIntrScanArr[] */
#define HARRIER_NUM_ELEMENTS_IN_SCAN_TREE_CNS \
    NUM_ELEMENTS_IN_ARR_MAC(harrierIntrScanArr)


#define AC5P_NUM_MASK_REGISTERS_CNS  (PRV_CPSS_HARRIER_LAST_INT_E / 32)

/* mark extData as per pipe per tile port interrupt */
/* _tile    --> the tileId 0, 2  */
/* _raven   --> local raven 0..3 */
/* _port    --> local port in the pipe 0..15 */
/* Cider ports 0..7 are mapped to 'global ports' :*/

#define MTI_PORT                0x1
#define MTI_SEG_PORT            0x2
#define PORT_TYPE_OFFSET_BIT    16

#define HARRIER_SET_EVENT_PER_PORT_MAC(_instance, _postFix, _portNum)   \
    PRV_CPSS_HARRIER_MAC_INSTANCE_##_instance##_PORT_NUM_##_portNum##_##_postFix,      MTI_PORT << PORT_TYPE_OFFSET_BIT | ((_instance) * 8 + (_portNum))

#define HARRIER_SET_EVENT_PER_SEG_PORT_MAC(_instance, _postFix, _portNum)   \
    PRV_CPSS_HARRIER_MAC_INSTANCE_##_instance##_SEG_PORT_NUM_##_portNum##_##_postFix,  MTI_SEG_PORT << PORT_TYPE_OFFSET_BIT | ((_instance) * 8 + (_portNum))

/* _postFix - must include the "_E" */
#define HARRIER_SET_EVENT_WITH_INDEX_MAC(_postFix, _index)             \
    PRV_CPSS_HARRIER_##_postFix,                   _index

/* _postFix - must include the "_E" */
#define HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_postFix)                         \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_HARRIER_##_postFix)

/* _postFix - must include the "_E" */
#define HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_preFix, _index, _port, _postFix)     \
    PRV_CPSS_HARRIER_##_preFix##_REG_##_index##_PORT_##_port##_##_postFix,    (_index * 16) + _port

/* _postFix - must include the "_E" */
#define SET_EVENT_INDEX_PER_SOURCE_PER_DIRECTION_PER_DP_MAC(_postFix, _type, _prefix, _direction, _dp, _source)   \
    PRV_CPSS_HARRIER_##_type##_##_prefix##_##_dp##_##_postFix,            (_source << 8 |(((_dp) * 2) + _direction))

#define SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, _type, _prefix, _direction, _source)           \
    SET_EVENT_INDEX_PER_SOURCE_PER_DIRECTION_PER_DP_MAC(_postFix, _type, _prefix, _direction, 0, _source), \
    SET_EVENT_INDEX_PER_SOURCE_PER_DIRECTION_PER_DP_MAC(_postFix, _type, _prefix, _direction, 1, _source)

/* _postFix - must include the "_E" */
#define HARRIER_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(_postFix, _source)  \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, EIP164E_MACSEC_TYPE_3, EIP164E_MACSEC_INSTANCE, 0, _source),     \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, EIP164I_MACSEC_TYPE_2, EIP164I_MACSEC_INSTANCE, 1, _source)

/* _postFix - must include the "_E" */
#define HARRIER_SET_EIP_163_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(_postFix, _source)  \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, EIP163E_MACSEC_TYPE_1, EIP163E_MACSEC_INSTANCE, 0, _source),     \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, EIP163I_MACSEC_TYPE_0, EIP163I_MACSEC_INSTANCE, 1, _source)

/* _postFix - must include the "_E" */
#define SET_EVENT_INDEX_PER_DIRECTION_PER_DP_MAC(_postFix, _type, _prefix, _direction, _dp)   \
    PRV_CPSS_HARRIER_##_type##_##_prefix##_##_dp##_##_postFix,           (((_dp) << 1) + _direction)

#define SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, _type, _prefix, _direction)       \
    SET_EVENT_INDEX_PER_DIRECTION_PER_DP_MAC(_postFix, _type, _prefix, _direction, 0), \
    SET_EVENT_INDEX_PER_DIRECTION_PER_DP_MAC(_postFix, _type, _prefix, _direction, 1)

/* _postFix - must include the "_E" */
#define HARRIER_SET_EIP_164_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, EIP164E_MACSEC_TYPE_3, EIP164E_MACSEC_INSTANCE, 0)

/* _postFix - must include the "_E" */
#define HARRIER_SET_EIP_164_EVENT_SOURCE_PER_INGRESS_DIRECTION_ALL_DP_MAC(_postFix, _source)  \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, EIP164I_MACSEC_TYPE_2, EIP164I_MACSEC_INSTANCE, 1, _source)

/* _postFix - must include the "_E" */
#define HARRIER_SET_EIP_66_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, EIP66E_MACSEC_TYPE_3, EIP66E_MACSEC_INSTANCE, 0)

/* _postFix - must include the "_E" */
#define HARRIER_SET_EIP_164_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, EIP164E_MACSEC_TYPE_3, EIP164E_MACSEC_INSTANCE, 0),      \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, EIP164I_MACSEC_TYPE_2, EIP164I_MACSEC_INSTANCE, 1)

#define SET_EVENT_ALL_DIRECTIONS_PER_DP_MAC(_type, _prefix, _index, _direction, _dp)   \
    PRV_CPSS_HARRIER_##_type##_##_prefix##_##_dp##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_##_index##_E,  (_index << 8 |(((_dp) * 2) + _direction))

#define SET_EVENT_ALL_DIRECTIONS_ALL_DP_MAC(_type, _prefix, _index, _direction)            \
    SET_EVENT_ALL_DIRECTIONS_PER_DP_MAC(_type, _prefix, _index, _direction, 0),             \
    SET_EVENT_ALL_DIRECTIONS_PER_DP_MAC(_type, _prefix, _index, _direction, 1)

#define HARRIER_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_index)    \
    SET_EVENT_ALL_DIRECTIONS_ALL_DP_MAC(EIP66E_MACSEC_TYPE_3, EIP66E_MACSEC_INSTANCE, _index, 0),             \
    SET_EVENT_ALL_DIRECTIONS_ALL_DP_MAC(EIP66I_MACSEC_TYPE_2, EIP66I_MACSEC_INSTANCE, _index, 1)

#define HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_index, _postFix)          \
    HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  0, _postFix),            \
    HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  1, _postFix),            \
    HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  2, _postFix),            \
    HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  3, _postFix),            \
    HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  4, _postFix),            \
    HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  5, _postFix),            \
    HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  6, _postFix),            \
    HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  7, _postFix),            \
    HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  8, _postFix),            \
    HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  9, _postFix),            \
    HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index, 10, _postFix),            \
    HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index, 11, _postFix),            \
    HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index, 12, _postFix),            \
    HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index, 13, _postFix),            \
    HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index, 14, _postFix),            \
    HARRIER_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index, 15, _postFix)


#define HARRIER_SET_EVENT_PER_ALL_PORTS_ALL_REGISTERS_MAC(_postFix)                             \
    HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 1, _postFix),                           \
    HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 2, _postFix),                           \
    HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 3, _postFix),                           \
    HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 4, _postFix),                           \
    HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 5, _postFix),                           \
    HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 6, _postFix),                           \
    HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 7, _postFix),                           \
    HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 8, _postFix),                           \
    HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 9, _postFix),                           \
    HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(10, _postFix),                           \
    HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(11, _postFix),                           \
    HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(12, _postFix),                           \
    HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(13, _postFix),                           \
    HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(14, _postFix),                           \
    HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(15, _postFix),                           \
    HARRIER_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(16, _postFix)


#define HARRIER_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(index,_postFix)                             \
    HARRIER_SET_EVENT_PER_PORT_MAC(index, _postFix,      0),                                    \
    HARRIER_SET_EVENT_PER_PORT_MAC(index, _postFix,      1),                                    \
    HARRIER_SET_EVENT_PER_PORT_MAC(index, _postFix,      2),                                    \
    HARRIER_SET_EVENT_PER_PORT_MAC(index, _postFix,      3),                                    \
    HARRIER_SET_EVENT_PER_PORT_MAC(index, _postFix,      4),                                    \
    HARRIER_SET_EVENT_PER_PORT_MAC(index, _postFix,      5),                                    \
    HARRIER_SET_EVENT_PER_PORT_MAC(index, _postFix,      6),                                    \
    HARRIER_SET_EVENT_PER_PORT_MAC(index, _postFix,      7)

#define HARRIER_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(index,_postFix)                     \
    HARRIER_SET_EVENT_PER_PORT_MAC(index, _postFix,      0),                                    \
    HARRIER_SET_EVENT_PER_PORT_MAC(index, _postFix,      1),                                    \
    HARRIER_SET_EVENT_PER_PORT_MAC(index, _postFix,      2),                                    \
    HARRIER_SET_EVENT_PER_PORT_MAC(index, _postFix,      3)

#define HARRIER_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(index,_postFix)                             \
    HARRIER_SET_EVENT_PER_SEG_PORT_MAC(index,_postFix,  0),                                     \
    HARRIER_SET_EVENT_PER_SEG_PORT_MAC(index,_postFix,  1)

#define HARRIER_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(index,_postFix)                     \
    HARRIER_SET_EVENT_PER_SEG_PORT_MAC(index,_postFix,  0)

#define SINGLE_MG_EVENT_MAC(_postFix, _mgUnitId, _index)                                        \
    PRV_CPSS_AC5X_MG_##_mgUnitId##_MG_INTERNAL_INTERRUPT_CAUSE_##_postFix, (((_mgUnitId)*8) + _index)

#define ALL_MG_EVENTS_MAC(_postFix, _index)                                                     \
    SINGLE_MG_EVENT_MAC(_postFix, 0, _index),                                                   \
    SINGLE_MG_EVENT_MAC(_postFix, 1, _index),                                                   \
    SINGLE_MG_EVENT_MAC(_postFix, 2, _index)

#define SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, _localQueue)  \
    PRV_CPSS_HARRIER_MG_##_mgUnitId##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_##type##_##_localQueue##_E,  (_mgUnitId*8 + _localQueue)

#define SINGLE_MG_TX_SDMA_QUEUES_MAC(type, _mgUnitId)                  \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 0/*local queue 0*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 1/*local queue 1*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 2/*local queue 2*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 3/*local queue 3*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 4/*local queue 4*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 5/*local queue 5*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 6/*local queue 6*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 7/*local queue 7*/)

#define ALL_MG_TX_SDMA_QUEUES(type)                                    \
    SINGLE_MG_TX_SDMA_QUEUES_MAC(type, 0/*mg0*/),                      \
    SINGLE_MG_TX_SDMA_QUEUES_MAC(type, 1/*mg1*/)


/* connect unified event of 'RX SDMA' to their interrupt */
/* specific unified event to specific interrupt */
#define SINGLE_RX_SDMA_INT_MAC(type, local_mgUnitId, globalQueue, localQueue) \
    CPSS_PP_##type##_QUEUE##globalQueue##_E,                              \
        PRV_CPSS_HARRIER_MG_##local_mgUnitId##_RECEIVE_SDMA_INTERRUPT_CAUSE0_##type##_QUEUE_##localQueue##_E, globalQueue, \
    MARK_END_OF_UNI_EV_CNS



#define SINGLE_MG_RX_SDMA_MAC(mgIndex) \
    PRV_CPSS_HARRIER_MG_##mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_RESOURCE_ERROR_CNT_OF_E,     ((mgIndex)*6)+0, \
    PRV_CPSS_HARRIER_MG_##mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_BYTE_CNT_OF_E,               ((mgIndex)*6)+1, \
    PRV_CPSS_HARRIER_MG_##mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_PACKET_CNT_OF_E,             ((mgIndex)*6)+2


/* connect unified event of 'RX SDMA' to their interrupt */
/* global MG 0 */
#define MG0_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,  0/*local mg*/,  0/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  0/*local mg*/,  1/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  0/*local mg*/,  2/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  0/*local mg*/,  3/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  0/*local mg*/,  4/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  0/*local mg*/,  5/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  0/*local mg*/,  6/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  0/*local mg*/,  7/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
/* global MG 1 */
#define MG1_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,  1/*local mg*/,   8/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  1/*local mg*/,   9/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  1/*local mg*/,  10/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  1/*local mg*/,  11/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  1/*local mg*/,  12/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  1/*local mg*/,  13/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  1/*local mg*/,  14/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  1/*local mg*/,  15/*globalQueue*/,7/*localQueue*/)


/* connect unified event of 'RX SDMA' to their interrupt */
/* ALL MGs [0..15] */
#define ALL_MGS_RX_SDMA_INT_MAC(type)    \
    MG0_RX_SDMA_INT_MAC(type),           \
    MG1_RX_SDMA_INT_MAC(type)

/* Maximal PPN events */
#define PPN_EVENT_TOTAL    16

#define SET_PPN_EVENT_MAC(_ppg, _ppn)   \
    PRV_CPSS_HARRIER_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_HOST_UNMAPPED_ACCESS_E             , ((8 + (PPN_EVENT_TOTAL * 0) + (_ppg * 8)) + (_ppn)),  \
    PRV_CPSS_HARRIER_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_CORE_UNMAPPED_ACCESS_E             , ((8 + (PPN_EVENT_TOTAL * 1) + (_ppg * 8)) + (_ppn)),  \
    PRV_CPSS_HARRIER_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_NEAR_EDGE_IMEM_ACCESS_E            , ((8 + (PPN_EVENT_TOTAL * 2) + (_ppg * 8)) + (_ppn)),  \
    PRV_CPSS_HARRIER_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_DOORBELL_INTERRUPT_E               , ((8 + (PPN_EVENT_TOTAL * 3) + (_ppg * 8)) + (_ppn))

#define SET_PPG_EVENT_PER_8_PPN_MAC(_ppg)\
    SET_PPN_EVENT_MAC(_ppg, 0),          \
    SET_PPN_EVENT_MAC(_ppg, 1),          \
    SET_PPN_EVENT_MAC(_ppg, 2),          \
    SET_PPN_EVENT_MAC(_ppg, 3),          \
    SET_PPN_EVENT_MAC(_ppg, 4),          \
    SET_PPN_EVENT_MAC(_ppg, 5),          \
    SET_PPN_EVENT_MAC(_ppg, 6),          \
    SET_PPN_EVENT_MAC(_ppg, 7)


#define SET_PHA_EVENTS_MAC  \
   /* PHA/PPA */                         \
   PRV_CPSS_HARRIER_PHA_PHA_INTERNAL_ERROR_CAUSE_PHA_BAD_ADDRESS_ERROR_E,                                      0,   \
   PRV_CPSS_HARRIER_PHA_PHA_INTERNAL_ERROR_CAUSE_PHA_TABLE_ACCESS_OVERLAP_ERROR_E,                             512, \
   PRV_CPSS_HARRIER_PHA_PHA_INTERNAL_ERROR_CAUSE_HEADER_SIZE_VIOLATION_E,                                      513, \
   PRV_CPSS_HARRIER_PHA_PHA_INTERNAL_ERROR_CAUSE_PPA_CLOCK_DOWN_VIOLATION_E,                                   514, \
   /* PPGs */                                                                                                    \
   PRV_CPSS_HARRIER_PPG_0_PPG_INTERNAL_ERROR_CAUSE_PPG_BAD_ADDRESS_ERROR_E,                                    4,   \
   PRV_CPSS_HARRIER_PPG_1_PPG_INTERNAL_ERROR_CAUSE_PPG_BAD_ADDRESS_ERROR_E,                                    5,   \
                                                                                                                 \
   PRV_CPSS_HARRIER_PPG_0_PPG_INTERNAL_ERROR_CAUSE_PPG_TABLE_ACCESS_OVERLAP_ERROR_E,                           515, \
   PRV_CPSS_HARRIER_PPG_1_PPG_INTERNAL_ERROR_CAUSE_PPG_TABLE_ACCESS_OVERLAP_ERROR_E,                           516, \
   /* PPNs */                                                                                                    \
   SET_PPG_EVENT_PER_8_PPN_MAC(0),                                                                              \
   SET_PPG_EVENT_PER_8_PPN_MAC(1)

/* _postFix - must include the "_E" */
#define SET_LMU_EVENT_WITH_INDEX_MAC(_postFix, _instance, _threshold, _index)                                   \
    PRV_CPSS_HARRIER_LMU_INSTANCE_##_instance##_N_##_threshold##_I_##_index##_##_postFix, ((_instance << 16) | ((31*_threshold) +(_index)))

#define LMU_LATENCY_OVER_THRESHOLD_MAC(_instance, _threshold)                                   \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   0),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   1),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   2),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   3),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   4),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   5),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   6),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   7),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   8),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   9),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  10),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  11),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  12),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  13),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  14),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  15),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  16),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  17),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  18),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  19),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  20),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  21),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  22),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  23),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  24),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  25),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  26),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  27),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  28),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  29),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  30)

#define HARRIER_PER_INSTANCE_LMU_LATENCY_OVER_THRESHOLD_MAC(_instance)                          \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance,  0),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance,  1),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance,  2),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance,  3),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance,  4),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance,  5),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance,  6),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance,  7),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance,  8),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance,  9),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance, 10),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance, 11),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance, 12),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance, 13),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance, 14),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance, 15),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_instance, 16)

#define SET_HR_TRSH_EVENT_MAC(_unit, _regIndex, _port, _tc)             \
    PRV_CPSS_HARRIER_QFC_UNIT_##_unit##_REGISTER_INDEX_##_regIndex##_PORT_##_port##_TC_##_tc##_CROSSED_HR_THRESHOLD_E, (_port * 8) + _tc

#define SET_HR_TRSH_EVENT_ALL_TC_MAC(_unit, _regIndex, _port)           \
    SET_HR_TRSH_EVENT_MAC(_unit, _regIndex, _port, 0),                  \
    SET_HR_TRSH_EVENT_MAC(_unit, _regIndex, _port, 1),                  \
    SET_HR_TRSH_EVENT_MAC(_unit, _regIndex, _port, 2),                  \
    SET_HR_TRSH_EVENT_MAC(_unit, _regIndex, _port, 3),                  \
    SET_HR_TRSH_EVENT_MAC(_unit, _regIndex, _port, 4),                  \
    SET_HR_TRSH_EVENT_MAC(_unit, _regIndex, _port, 5),                  \
    SET_HR_TRSH_EVENT_MAC(_unit, _regIndex, _port, 6),                  \
    SET_HR_TRSH_EVENT_MAC(_unit, _regIndex, _port, 7)

#define HARRIER_SET_HR_TRSH_EVENT_ALL_PORTS_MAC                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0,   0),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0,   1),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0,   2),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1,   3),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1,   4),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1,   5),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 2,   6),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 2,   7),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 2,   8),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 3,   9),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 3,  10),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 3,  11),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 4,  12),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 4,  13),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 4,  14),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 5,  15),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 5,  16),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 5,  17),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 6,  18),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 6,  19),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 6,  20),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 7,  21),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 7,  22),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 7,  23),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 8,  24),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 8,  25),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 8,  26),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0,  27),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0,  28),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0,  29),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1,  30),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1,  31),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1,  32),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 2,  33),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 2,  34),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 2,  35),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 3,  36),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 3,  37),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 3,  38),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 4,  39),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 4,  40),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 4,  41),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 5,  42),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 5,  43),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 5,  44),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 6,  45),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 6,  46),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 6,  47),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 7,  48),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 7,  49),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 7,  50),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 8,  51),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 8,  52),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 8,  53),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 0,  54),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 0,  55),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 0,  56),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 1,  57),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 1,  58),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 1,  59),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 2,  60),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 2,  61),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 2,  62),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 3,  63),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 3,  64),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 3,  65),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 4,  66),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 4,  67),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 4,  68),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 5,  69),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 5,  70),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 5,  71),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 6,  72),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 6,  73),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 6,  74),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 7,  75),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 7,  76),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 7,  77),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 8,  78),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 8,  79),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(2, 8,  80)

#define CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, _channel, _extData, _port)                          \
        PRV_CPSS_HARRIER_CTSU_INSTANCE_##_instance##_CHANNEL_##_channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_0_##_postfix, (((_port) << 8) | (_extData))

#define CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, _channel, _extData, _port)                          \
        PRV_CPSS_HARRIER_CTSU_INSTANCE_##_instance##_CHANNEL_##_channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_1_##_postfix, (((_port) << 8) | (_extData))

#define SET_EVENT_GROUP_0_CHANNEL_CTSU_16_PCA_MAC(_postfix, _instance, _extData)                                                                 \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  0, _extData, ( 0 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  1, _extData, ( 1 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  2, _extData, ( 2 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  3, _extData, ( 3 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  4, _extData, ( 4 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  5, _extData, ( 5 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  6, _extData, ( 6 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  7, _extData, ( 7 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  8, _extData, ( 8 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  9, _extData, ( 9 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 10, _extData, (10 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 11, _extData, (11 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 12, _extData, (12 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 13, _extData, (13 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 14, _extData, (14 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 15, _extData, (15 + (_instance * 16)))

#define SET_EVENT_GROUP_0_INSTANCE_CTSU_16_PCA_MAC(_postfix, _extData)                                                                          \
        SET_EVENT_GROUP_0_CHANNEL_CTSU_16_PCA_MAC(_postfix, 0, _extData),                                                                       \
        SET_EVENT_GROUP_0_CHANNEL_CTSU_16_PCA_MAC(_postfix, 1, _extData),                                                                       \
        SET_EVENT_GROUP_0_CHANNEL_CTSU_16_PCA_MAC(_postfix, 2, _extData)

#define SET_EVENT_GROUP_1_CHANNEL_CTSU_16_PCA_MAC(_postfix, _instance, _extData)                                                                 \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance,  0, _extData, (16 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance,  1, _extData, (17 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance,  2, _extData, (18 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance,  3, _extData, (19 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance,  4, _extData, (20 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance,  5, _extData, (21 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance,  6, _extData, (22 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance,  7, _extData, (23 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance,  8, _extData, (24 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance,  9, _extData, (25 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 10, _extData, (26 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 11, _extData, (27 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 12, _extData, (28 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 13, _extData, (29 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 14, _extData, (30 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 15, _extData, (31 + (_instance * 16)))

#define SET_EVENT_GROUP_1_INSTANCE_CTSU_16_PCA_MAC(_postfix, _extData)                                                                          \
        SET_EVENT_GROUP_1_CHANNEL_CTSU_16_PCA_MAC(_postfix, 0, _extData),                                                                       \
        SET_EVENT_GROUP_1_CHANNEL_CTSU_16_PCA_MAC(_postfix, 1, _extData),                                                                       \
        SET_EVENT_GROUP_1_CHANNEL_CTSU_16_PCA_MAC(_postfix, 2, _extData)                                                                        

#define HARRIER_COMMON_EVENTS                                                                                           \
 /* single uniEvent for all 128 queues , extData = queueId */                                                           \
 CPSS_PP_TX_BUFFER_QUEUE_E,                                                                                             \
     ALL_MG_TX_SDMA_QUEUES(TX_BUFFER_QUEUE),                                                                            \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 /* single uniEvent for all 128 queues , extData = queueId */                                                           \
 CPSS_PP_TX_ERR_QUEUE_E,                                                                                                \
     ALL_MG_TX_SDMA_QUEUES(TX_ERROR_QUEUE),                                                                             \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 /* single uniEvent for all 128 queues , extData = queueId */                                                           \
 CPSS_PP_TX_END_E,                                                                                                      \
     ALL_MG_TX_SDMA_QUEUES(TX_END_QUEUE),                                                                               \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_GTS_GLOBAL_FIFO_FULL_E,                                                                                        \
    PRV_CPSS_HARRIER_ERMRK_ERMRK_INTERRUPT_CAUSE_INGRESS_TIMESTAMP_QUEUE0_FULL_E,  0,                                   \
    PRV_CPSS_HARRIER_ERMRK_ERMRK_INTERRUPT_CAUSE_EGRESS_TIMESTAMP_QUEUE0_FULL_E,   1,                                   \
    PRV_CPSS_HARRIER_ERMRK_ERMRK_INTERRUPT_CAUSE_INGRESS_TIMESTAMP_QUEUE1_FULL_E,  2,                                   \
    PRV_CPSS_HARRIER_ERMRK_ERMRK_INTERRUPT_CAUSE_EGRESS_TIMESTAMP_QUEUE1_FULL_E,   3,                                   \
    SET_EVENT_GROUP_0_INSTANCE_CTSU_16_PCA_MAC(OVERFLOW_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,0,1)),                     \
    SET_EVENT_GROUP_0_INSTANCE_CTSU_16_PCA_MAC(OVERFLOW_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,1,1)),                     \
    SET_EVENT_GROUP_1_INSTANCE_CTSU_16_PCA_MAC(OVERFLOW_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,0,1)),                     \
    SET_EVENT_GROUP_1_INSTANCE_CTSU_16_PCA_MAC(OVERFLOW_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,1,1)),                     \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,                                                                               \
    PRV_CPSS_HARRIER_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_INGRESS_TIMESTAMP_Q0_E,  0,                                        \
    PRV_CPSS_HARRIER_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_EGRESS_TIMESTAMP_Q0_E,   1,                                        \
    PRV_CPSS_HARRIER_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_INGRESS_TIMESTAMP_Q1_E,  2,                                        \
    PRV_CPSS_HARRIER_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_EGRESS_TIMESTAMP_Q1_E,   3,                                        \
    SET_EVENT_GROUP_0_INSTANCE_CTSU_16_PCA_MAC(NEW_ENTRY_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,0,1)),                    \
    SET_EVENT_GROUP_0_INSTANCE_CTSU_16_PCA_MAC(NEW_ENTRY_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,1,1)),                    \
    SET_EVENT_GROUP_1_INSTANCE_CTSU_16_PCA_MAC(NEW_ENTRY_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,0,1)),                    \
    SET_EVENT_GROUP_1_INSTANCE_CTSU_16_PCA_MAC(NEW_ENTRY_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,1,1)),                    \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E,                                                                                    \
    PRV_CPSS_HARRIER_TAI_MASTER_0_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,       0,                                  \
    PRV_CPSS_HARRIER_TAI_MASTER_1_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,       1,                                  \
    PRV_CPSS_HARRIER_MAC_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    2,                                  \
    PRV_CPSS_HARRIER_MAC_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    3,                                  \
    PRV_CPSS_HARRIER_MAC_TAI_SLAVE_2_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    4,                                  \
    PRV_CPSS_HARRIER_MAC_TAI_SLAVE_3_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    5,                                  \
    PRV_CPSS_HARRIER_MAC_TAI_SLAVE_4_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    6,                                  \
    PRV_CPSS_HARRIER_MAC_TAI_SLAVE_5_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    7,                                  \
    PRV_CPSS_HARRIER_QFC_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    8,                                  \
    PRV_CPSS_HARRIER_QFC_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    9,                                  \
    PRV_CPSS_HARRIER_QFC_TAI_SLAVE_2_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    10,                                 \
    PRV_CPSS_HARRIER_ERMRK_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,  11,                                 \
    PRV_CPSS_HARRIER_ERMRK_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,  12,                                 \
    PRV_CPSS_HARRIER_PHA_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    13,                                 \
    PRV_CPSS_HARRIER_PHA_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    14,                                 \
    PRV_CPSS_HARRIER_TTI_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    15,                                 \
    PRV_CPSS_HARRIER_TTI_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    16,                                 \
    PRV_CPSS_HARRIER_PCA_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    17,                                 \
    PRV_CPSS_HARRIER_PCA_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    18,                                 \
    PRV_CPSS_HARRIER_PCA_TAI_SLAVE_2_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    19,                                 \
    PRV_CPSS_HARRIER_PCA_TAI_SLAVE_3_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    20,                                 \
    PRV_CPSS_HARRIER_PCA_TAI_SLAVE_4_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    21,                                 \
    PRV_CPSS_HARRIER_PCA_TAI_SLAVE_5_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    22,                                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PTP_TAI_GENERATION_E,                                                                                          \
    PRV_CPSS_HARRIER_TAI_MASTER_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,             0,                                  \
    PRV_CPSS_HARRIER_TAI_MASTER_1_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,             1,                                  \
    PRV_CPSS_HARRIER_MAC_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          2,                                  \
    PRV_CPSS_HARRIER_MAC_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          3,                                  \
    PRV_CPSS_HARRIER_MAC_TAI_SLAVE_2_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          4,                                  \
    PRV_CPSS_HARRIER_MAC_TAI_SLAVE_3_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          5,                                  \
    PRV_CPSS_HARRIER_MAC_TAI_SLAVE_4_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          6,                                  \
    PRV_CPSS_HARRIER_MAC_TAI_SLAVE_5_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          7,                                  \
    PRV_CPSS_HARRIER_QFC_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          8,                                  \
    PRV_CPSS_HARRIER_QFC_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          9,                                  \
    PRV_CPSS_HARRIER_QFC_TAI_SLAVE_2_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          10,                                 \
    PRV_CPSS_HARRIER_ERMRK_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,        11,                                 \
    PRV_CPSS_HARRIER_ERMRK_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,        12,                                 \
    PRV_CPSS_HARRIER_PHA_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          13,                                 \
    PRV_CPSS_HARRIER_PHA_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          14,                                 \
    PRV_CPSS_HARRIER_TTI_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          15,                                 \
    PRV_CPSS_HARRIER_TTI_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          16,                                 \
    PRV_CPSS_HARRIER_PCA_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          17,                                 \
    PRV_CPSS_HARRIER_PCA_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          18,                                 \
    PRV_CPSS_HARRIER_PCA_TAI_SLAVE_2_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          19,                                 \
    PRV_CPSS_HARRIER_PCA_TAI_SLAVE_3_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          20,                                 \
    PRV_CPSS_HARRIER_PCA_TAI_SLAVE_4_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          21,                                 \
    PRV_CPSS_HARRIER_PCA_TAI_SLAVE_5_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          22,                                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 /* 128 uniEvents for 128 queues , extData = queueId */                                                                 \
 ALL_MGS_RX_SDMA_INT_MAC(RX_BUFFER),                                                                                    \
 /* 128 uniEvents for 128 queues , extData = queueId */                                                                 \
 ALL_MGS_RX_SDMA_INT_MAC(RX_ERROR),                                                                                     \
                                                                                                                        \
 CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,                                                                                       \
    /* MTI total ports 0..255 */                                                                                        \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(0, MAC_TX_UNDERFLOW_E),                                                 \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(1, MAC_TX_UNDERFLOW_E),                                                 \
    HARRIER_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(2, MAC_TX_UNDERFLOW_E),                                         \
    /* seg ports*/                                                                                                      \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(0, MAC_TX_UNDERFLOW_E),                                                 \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(1, MAC_TX_UNDERFLOW_E),                                                 \
    HARRIER_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(2, MAC_TX_UNDERFLOW_E),                                         \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PORT_TX_FIFO_OVERRUN_E,                                                                                        \
    /* MTI total ports 0..255 */                                                                                        \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(0, EMAC_TX_OVR_ERR_E),                                                  \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(1, EMAC_TX_OVR_ERR_E),                                                  \
    HARRIER_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(2, EMAC_TX_OVR_ERR_E),                                          \
    /* seg ports*/                                                                                                      \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(0, MAC_TX_OVR_ERR_E),                                                   \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(1, MAC_TX_OVR_ERR_E),                                                 \
    HARRIER_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(2, MAC_TX_OVR_ERR_E),                                           \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PORT_RX_FIFO_OVERRUN_E,                                                                                        \
    /* MTI total ports 0..255 */                                                                                        \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(0, EMAC_RX_OVERRUN_E),                                                  \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(1, EMAC_RX_OVERRUN_E),                                                  \
    HARRIER_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(2, EMAC_RX_OVERRUN_E),                                          \
    /* seg ports*/                                                                                                      \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(0, RX_OVERRUN_E),                                                       \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(1, RX_OVERRUN_E),                                                 \
    HARRIER_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(2, RX_OVERRUN_E),                                               \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_CNC_WRAPAROUND_BLOCK_E,                                                                                        \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_0_WRAPAROUND_E,  (16*0 +  0)),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_1_WRAPAROUND_E,  (16*0 +  1)),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_2_WRAPAROUND_E,  (16*0 +  2)),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_3_WRAPAROUND_E,  (16*0 +  3)),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_4_WRAPAROUND_E,  (16*0 +  4)),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_5_WRAPAROUND_E,  (16*0 +  5)),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_6_WRAPAROUND_E,  (16*0 +  6)),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_7_WRAPAROUND_E,  (16*0 +  7)),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_8_WRAPAROUND_E,  (16*0 +  8)),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_9_WRAPAROUND_E,  (16*0 +  9)),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_10_WRAPAROUND_E, (16*0 + 10)),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_11_WRAPAROUND_E, (16*0 + 11)),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_12_WRAPAROUND_E, (16*0 + 12)),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_13_WRAPAROUND_E, (16*0 + 13)),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_14_WRAPAROUND_E, (16*0 + 14)),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_15_WRAPAROUND_E, (16*0 + 15)),  \
                                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_RX_CNTR_OVERFLOW_E,                                                                                      \
    SINGLE_MG_RX_SDMA_MAC(0),                                                                                     \
    SINGLE_MG_RX_SDMA_MAC(1),                                                                                     \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_SCT_RATE_LIMITER_E,                                                                                      \
    HARRIER_SET_EVENT_PER_ALL_PORTS_ALL_REGISTERS_MAC(PKT_DROPED_INT_E),                                          \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E,                                                                            \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(IPLR0_POLICER_INTERRUPT_CAUSE_ILLEGAL_ACCESS_E,       0),                    \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(IPLR1_POLICER_INTERRUPT_CAUSE_ILLEGAL_ACCESS_E,       1),                    \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(EPLR_POLICER_INTERRUPT_CAUSE_ILLEGAL_ACCESS_E,        2),                    \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E,                                                                             \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(IPLR0_POLICER_INTERRUPT_CAUSE_IPFIX_WRAPAROUND_E,            0 ),            \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(IPLR1_POLICER_INTERRUPT_CAUSE_IPFIX_WRAPAROUND_E,            1 ),            \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(EPLR_POLICER_INTERRUPT_CAUSE_IPFIX_WRAPAROUND_E,             2 ),            \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_POLICER_IPFIX_ALARM_E,                                                                                   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(IPLR0_POLICER_INTERRUPT_CAUSE_IPFIX_ALARM_E,                 0 ),            \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(IPLR1_POLICER_INTERRUPT_CAUSE_IPFIX_ALARM_E,                 1 ),            \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(EPLR_POLICER_INTERRUPT_CAUSE_IPFIX_ALARM_E,                  2 ),            \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,                                                        \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(IPLR0_POLICER_INTERRUPT_CAUSE_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    0 ),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(IPLR1_POLICER_INTERRUPT_CAUSE_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    1 ),  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(EPLR_POLICER_INTERRUPT_CAUSE_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,     2 ),  \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,                                                                                 \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    0),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    0),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    0),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    0),   \
                                                                                                                  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    1),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    1),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    1),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    1),   \
                                                                                                                  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    2),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    2),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    2),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    2),   \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_PCL_LOOKUP_FIFO_FULL_E,                                                                                  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_LOOKUP0_FIFO_FULL_E,                      0),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_LOOKUP1_FIFO_FULL_E,                      1),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_LOOKUP2_FIFO_FULL_E,                      2),   \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_PCL_ACTION_TRIGGERED_E,                                                                                  \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_IPCL0_TCAM_TRIGGERED_INTERRUPT_E,         0),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_IPCL1_TCAM_TRIGGERED_INTERRUPT_E,         1),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_IPCL2_TCAM_TRIGGERED_INTERRUPT_E,         2),   \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_EB_NA_FIFO_FULL_E,                                                                                       \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_0_NA_FIFO_FULL_E,         0),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_1_NA_FIFO_FULL_E,         1),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_2_NA_FIFO_FULL_E,         2),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_3_NA_FIFO_FULL_E,         3),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_4_NA_FIFO_FULL_E,         4),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_5_NA_FIFO_FULL_E,         5),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_6_NA_FIFO_FULL_E,         6),   \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_7_NA_FIFO_FULL_E,         7),   \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_TTI_ACCESS_DATA_ERROR_E,                                                                                 \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_0_ACCESS_DATA_ERROR_E, 0),          \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_1_ACCESS_DATA_ERROR_E, 1),          \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_2_ACCESS_DATA_ERROR_E, 2),          \
    HARRIER_SET_EVENT_WITH_INDEX_MAC(TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_3_ACCESS_DATA_ERROR_E, 3),          \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 /* next HW interrupt value will have same index (evExtData) value in CPSS_PP_CRITICAL_HW_ERROR_E */                    \
 CPSS_PP_CRITICAL_HW_ERROR_E,                                                                                           \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E), \
     /*PSI_REG*/                                                                                                         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PSI_REGS_PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_E),                          \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PSI_REGS_PSI_INTERRUPT_CAUSE_MG2SNAKE_INTERRUPT_E),                           \
\
    /*TXQ_QFC*/                                                                                                         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(QFC_UNIT_0_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PORT_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(QFC_UNIT_0_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(QFC_UNIT_1_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PORT_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(QFC_UNIT_1_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(QFC_UNIT_2_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PORT_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(QFC_UNIT_2_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E),\
\
    /*TXQ_PFCC*/                                                                                                        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_PR_PFCC_INTERRUPT_CAUSE_POOL_0_COUNTER_OVERFLOW_E),                       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_PR_PFCC_INTERRUPT_CAUSE_SOURCE_PORT_COUNTER_OVERFLOW_E),                  \
\
    /*HBU*/                                                                                                             \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HBU_HBU_INTERRUPT_CAUSE_REGISTER_ERROR_FHF_MEM_E),                            \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HBU_HBU_INTERRUPT_CAUSE_REGISTER_ERROR_CMT1_MEM_E),                           \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HBU_HBU_INTERRUPT_CAUSE_REGISTER_ERROR_CMT2_MEM_E),                           \
\
    /*TXQ_SDQ*/                                                                                                         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_CREDIT_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_ENQ_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_DEQ_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_QCN_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_CREDIT_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_ENQ_E),    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_DEQ_E),    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_QCN_E),    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_PFC_E),    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_PORT_OUTOFRANGE_E),\
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_CREDIT_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_ENQ_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_DEQ_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_QCN_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_CREDIT_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_ENQ_E),    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_DEQ_E),    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_QCN_E),    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_PFC_E),    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_PORT_OUTOFRANGE_E),\
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_CREDIT_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_ENQ_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_DEQ_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_QCN_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_CREDIT_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_ENQ_E),    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_DEQ_E),    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_QCN_E),    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_PFC_E),    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_PORT_OUTOFRANGE_E),\
\
    /*TXQ_PDS*/                                                                                                                              \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_0_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_E),                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_0_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_STATE_ORDER_FIFO_FULL_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_0_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_DATA_ORDER_FIFO_FULL_E),                 \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_0_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_ON_THE_AIR_CNTR_FULL_INT_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_0_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_E),              \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_0_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_SAME_WR_ADDR_INT_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_0_PDS_INTERRUPT_DEBUG_CAUSE_TOTAL_PDS_CNTR_OVRRUN_INT_E),                    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_0_PDS_INTERRUPT_DEBUG_CAUSE_PDS_CACHE_CNTR_OVRRUN_INT_E),                    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_0_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_E),                 \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_1_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_E),                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_1_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_STATE_ORDER_FIFO_FULL_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_1_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_DATA_ORDER_FIFO_FULL_E),                 \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_1_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_ON_THE_AIR_CNTR_FULL_INT_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_1_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_E),              \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_1_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_SAME_WR_ADDR_INT_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_1_PDS_INTERRUPT_DEBUG_CAUSE_TOTAL_PDS_CNTR_OVRRUN_INT_E),                    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_1_PDS_INTERRUPT_DEBUG_CAUSE_PDS_CACHE_CNTR_OVRRUN_INT_E),                    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_1_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_E),                 \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_2_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_E),                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_2_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_STATE_ORDER_FIFO_FULL_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_2_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_DATA_ORDER_FIFO_FULL_E),                 \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_2_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_ON_THE_AIR_CNTR_FULL_INT_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_2_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_E),              \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_2_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_SAME_WR_ADDR_INT_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_2_PDS_INTERRUPT_DEBUG_CAUSE_TOTAL_PDS_CNTR_OVRRUN_INT_E),                    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_2_PDS_INTERRUPT_DEBUG_CAUSE_PDS_CACHE_CNTR_OVRRUN_INT_E),                    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PDS_UNIT_2_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_E),                 \
\
    /*GPC_CELL_READ*/                                                                                                                    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_0_INTERRUPT_CAUSE_INTERFACE_0_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_0_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_0_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E),                    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_0_INTERRUPT_CAUSE_CAM_MULTIPLE_MATCHES_E),                                  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_0_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E),                    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_0_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_OVERFLOW_E),                     \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_1_INTERRUPT_CAUSE_INTERFACE_0_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_1_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_1_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E),                    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_1_INTERRUPT_CAUSE_CAM_MULTIPLE_MATCHES_E),                                  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_1_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E),                    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_1_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_OVERFLOW_E),                     \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_2_INTERRUPT_CAUSE_INTERFACE_0_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_2_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_2_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E),                    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_2_INTERRUPT_CAUSE_CAM_MULTIPLE_MATCHES_E),                                  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_2_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E),                    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INSTANCE_2_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_OVERFLOW_E),                     \
\
    /*NPM_MC*/                                                                                                                           \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_4_E),                      \
    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_4_E),                      \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_0_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_1_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_2_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_3_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_4_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_0_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_1_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_2_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_3_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_4_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_0_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_1_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_2_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_3_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_4_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_0_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_1_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_2_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_3_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_4_E),                   \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_0_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_1_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_2_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_3_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_4_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_0_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_1_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_2_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_3_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_4_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_0_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_1_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_2_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_3_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_4_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_0_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_1_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_2_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_3_E),                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_4_E),                   \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_4_E),                      \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_4_E),                      \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_4_E),                      \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_3_E),                      \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_3_E),                      \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_3_E),                      \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_0_E),                           \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_1_E),                           \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_2_E),                           \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_0_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_3_E),                           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_4_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_4_E),                      \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_3_E),                      \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_3_E),                      \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_3_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_0_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_1_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_2_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_3_E),                      \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_0_E),                           \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_1_E),                           \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_2_E),                           \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INSTANCE_1_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_3_E),                           \
\
    /*SMB_MC*/                                                                                                                           \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E),                  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_0_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_1_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_2_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_3_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_4_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_5_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_6_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_7_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_8_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_9_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_10_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_11_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_12_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_13_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_14_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_15_REFCNT_SER_ERROR_E),                                   \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E),                  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_0_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_1_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_2_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_3_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_4_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_5_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_6_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_7_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_8_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_9_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_10_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_11_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_12_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_13_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_14_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_15_REFCNT_SER_ERROR_E),                                   \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E),                  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_AGE_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_RBW_ERR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_0_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_1_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_2_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_3_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_4_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_5_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_6_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_7_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_8_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_9_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_10_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_11_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_12_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_13_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_14_REFCNT_SER_ERROR_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_15_REFCNT_SER_ERROR_E),                                   \
\
    /*SMB_WRITE_ARBITER*/                                                                                                                       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_WRITE_ARBITER_INSTANCE_0_INTERRUPT_MISC_CAUSE_INTERRUPT_MC_CA_FIFO_OVERFLOW_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_WRITE_ARBITER_INSTANCE_1_INTERRUPT_MISC_CAUSE_INTERRUPT_MC_CA_FIFO_OVERFLOW_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_WRITE_ARBITER_INSTANCE_2_INTERRUPT_MISC_CAUSE_INTERRUPT_MC_CA_FIFO_OVERFLOW_E),                \
\
    /*GPC_PACKET_WRITE[0x0B400200]*/                                                                                                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_0_E),                                       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_1_E),                                       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_2_E),                                       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_3_E),                                       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_0_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_0_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_1_INTERRUPT_CAUSE_INT_CAUSE_FIFO_0_E),                                       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_1_INTERRUPT_CAUSE_INT_CAUSE_FIFO_1_E),                                       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_1_INTERRUPT_CAUSE_INT_CAUSE_FIFO_2_E),                                       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_1_INTERRUPT_CAUSE_INT_CAUSE_FIFO_3_E),                                       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_1_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_1_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_2_INTERRUPT_CAUSE_INT_CAUSE_FIFO_0_E),                                       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_2_INTERRUPT_CAUSE_INT_CAUSE_FIFO_1_E),                                       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_2_INTERRUPT_CAUSE_INT_CAUSE_FIFO_2_E),                                       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_2_INTERRUPT_CAUSE_INT_CAUSE_FIFO_3_E),                                       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_2_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INSTANCE_2_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
\
    /*GPC_PACKET_READ[0x0AD01300]*/                                                                                                                 \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_0_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                 \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_0_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_0_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_0_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_0_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_0_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_0_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_0_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_0_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_0_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_0_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_0_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_1_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_1_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_1_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_1_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_1_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_1_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_1_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_1_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_1_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_1_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_1_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_1_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_2_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_2_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_2_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_2_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_2_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_2_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_2_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_2_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_2_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_2_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_2_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_2_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_3_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_3_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_3_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_3_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_3_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_3_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_3_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_3_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_3_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_3_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_3_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_3_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_4_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_4_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_4_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_4_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_4_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_4_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_4_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_4_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_4_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_4_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_4_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_4_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_5_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_5_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_5_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_5_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_5_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_5_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_5_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_5_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_5_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_5_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_5_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_5_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_6_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_6_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_6_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_6_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_6_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_6_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_6_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_6_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_6_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_6_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_6_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_6_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_7_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_7_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_7_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_7_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_7_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_7_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_7_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_7_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_7_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_7_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_7_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_7_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_8_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_8_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_8_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_8_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_8_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_8_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_8_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_8_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_8_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_8_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_8_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_8_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_9_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_9_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_9_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_9_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_9_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_9_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_9_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_9_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_9_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_9_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_9_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_9_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_10_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_10_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_10_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_10_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_10_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_10_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_10_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_10_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_10_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_10_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_10_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_10_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_11_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_11_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_11_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_11_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_11_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_11_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_11_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_11_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_11_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_11_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_11_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_11_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_12_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_12_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_12_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_12_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_12_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_12_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_12_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_12_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_12_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_12_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_12_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_12_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_13_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_13_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_13_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_13_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_13_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_13_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_13_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_13_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_13_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_13_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_13_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_13_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_14_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_14_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_14_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_14_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_14_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_14_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_14_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_14_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_14_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_14_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_14_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_14_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_15_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_15_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_15_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_15_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_15_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_15_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_15_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_15_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_15_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_15_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_15_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_15_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_16_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_16_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_16_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_16_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_16_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_16_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_16_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_16_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_16_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_16_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_16_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_16_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_17_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_17_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_17_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_17_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_17_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_17_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_17_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_17_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_17_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_17_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_17_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_17_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_18_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_18_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_18_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_18_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_18_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_18_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_18_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_18_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_18_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_18_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_18_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_18_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_19_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_19_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_19_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_19_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_19_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_19_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_19_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_19_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_19_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_19_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_19_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_19_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_20_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_20_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_20_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_20_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_20_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_20_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_20_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_20_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_20_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_20_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_20_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_20_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_21_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_21_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_21_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_21_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_21_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_21_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_21_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_21_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_21_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_21_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_21_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_21_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_22_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_22_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_22_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_22_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_22_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_22_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_22_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_22_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_22_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_22_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_22_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_22_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_23_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_23_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_23_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_23_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_23_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_23_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_23_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_23_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_23_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_23_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_23_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_23_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_24_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_24_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_24_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_24_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_24_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_24_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_24_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_24_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_24_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_24_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_24_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_24_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_25_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_25_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_25_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_25_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_25_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_25_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_25_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_25_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_25_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_25_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_25_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_25_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_26_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_26_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_26_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_26_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_26_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_26_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_26_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_26_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_26_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_26_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_26_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_26_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_27_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_27_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_27_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_27_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_27_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_27_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_27_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_27_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_27_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_27_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_27_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_27_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    /*GPC_PACKET_READ_MISC[0x01C01508]*/                                                                                                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_OVERFLOW_E),            \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_UNDERFLOW_E),           \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_MISC_CAUSE_CAM_MULTIPLE_MATCHES_E),                  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_MISC_CAUSE_DESCRIPTOR_HEADER_SIZE_ERROR_E),          \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_MISC_CAUSE_DESCRIPTOR_TAIL_SIZE_ERROR_E),            \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_MISC_CAUSE_PACKET_COUNT_PACKET_SIZE_ERROR_E),        \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_OVERFLOW_E),            \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_UNDERFLOW_E),           \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_MISC_CAUSE_CAM_MULTIPLE_MATCHES_E),                  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_MISC_CAUSE_DESCRIPTOR_HEADER_SIZE_ERROR_E),          \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_MISC_CAUSE_DESCRIPTOR_TAIL_SIZE_ERROR_E),            \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_MISC_CAUSE_PACKET_COUNT_PACKET_SIZE_ERROR_E),        \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_OVERFLOW_E),            \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_UNDERFLOW_E),           \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_MISC_CAUSE_CAM_MULTIPLE_MATCHES_E),                  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_MISC_CAUSE_DESCRIPTOR_HEADER_SIZE_ERROR_E),          \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_MISC_CAUSE_DESCRIPTOR_TAIL_SIZE_ERROR_E),            \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_MISC_CAUSE_PACKET_COUNT_PACKET_SIZE_ERROR_E),        \
\
    /*GPC_PACKET_READ_PACKET_COUNT[0x0AD01518]*/                                                                                          \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CAUSE_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_PACKET_COUNT_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CAUSE_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_PACKET_COUNT_E),\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CAUSE_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_PACKET_COUNT_E),\
\
    /*GPC_PACKET_READ_PACKET_CREDIT_COUNTER[0x0AD01510]*/                                                                                 \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_HT_OVERFLOW_E),           \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_HT_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_HT_OVERFLOW_E),     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E),                  \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_HT_OVERFLOW_E),           \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_HT_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_HT_OVERFLOW_E),     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E),                  \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_HT_OVERFLOW_E),           \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_HT_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_HT_OVERFLOW_E),     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E),                  \
\
    /*GPC_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER[0x01C01518]*/                                                         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E),          \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E),          \
\
    /*DFX_SERVER_INTERRUPT_CAUSE[0x0B8F8108]*/                                                                                                                    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_BIST_FAIL_E),                                                    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_EXTERNAL_TEMPERATURE_THRESHOLD_E),                               \
\
    /*RXDMA_INTERRUPT2_CAUSE[0x08001C90]*/                                                                                         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_PB_TAIL_ID_MEM_SER_INT_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_0_MEM_SER_INT_E),                 \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_1_MEM_SER_INT_E),                 \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_REPLY_TAIL_PB_ID_MEM_SER_INT_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_0_PB_RD_DATA_ERR_INT_E),               \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_1_PB_RD_DATA_ERR_INT_E),               \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_0_PB_RD_DATA_ERR_INT_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_1_PB_RD_DATA_ERR_INT_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_0_RD_SER_INT_E),               \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_1_RD_SER_INT_E),               \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_ENQUEUE_REQUESTS_DESCRIPTOR_FIFO_RD_SER_INT_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_HEAD_PACKET_COUNT_DATA_FIFO_MEM_RD_SER_INT_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E),            \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_PB_TAIL_ID_MEM_SER_INT_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_0_MEM_SER_INT_E),                 \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_1_MEM_SER_INT_E),                 \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_REPLY_TAIL_PB_ID_MEM_SER_INT_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_0_PB_RD_DATA_ERR_INT_E),               \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_1_PB_RD_DATA_ERR_INT_E),               \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_0_PB_RD_DATA_ERR_INT_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_1_PB_RD_DATA_ERR_INT_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_0_RD_SER_INT_E),               \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_1_RD_SER_INT_E),               \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_ENQUEUE_REQUESTS_DESCRIPTOR_FIFO_RD_SER_INT_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_HEAD_PACKET_COUNT_DATA_FIFO_MEM_RD_SER_INT_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E),            \
\
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_PB_TAIL_ID_MEM_SER_INT_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_0_MEM_SER_INT_E),                 \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_1_MEM_SER_INT_E),                 \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_REPLY_TAIL_PB_ID_MEM_SER_INT_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_0_PB_RD_DATA_ERR_INT_E),               \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_1_PB_RD_DATA_ERR_INT_E),               \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_0_PB_RD_DATA_ERR_INT_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_1_PB_RD_DATA_ERR_INT_E),                \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_0_RD_SER_INT_E),               \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_1_RD_SER_INT_E),               \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_ENQUEUE_REQUESTS_DESCRIPTOR_FIFO_RD_SER_INT_E), \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_HEAD_PACKET_COUNT_DATA_FIFO_MEM_RD_SER_INT_E),  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E),            \
\
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_DATA_INTEGRITY_ERROR_E,                                                                                        \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HA_HA_INTERRUPT_CAUSE_ECC_SINGLE_ERROR_E),                                 \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HA_HA_INTERRUPT_CAUSE_ECC_DOUBLE_ERROR_E),                                 \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TCAM_TCAM_INTERRUPT_CAUSE_TCAM_ARRAY_PARITY_ERROR_E),                      \
    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INSTANCE_0_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INSTANCE_0_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INSTANCE_1_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INSTANCE_1_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INSTANCE_2_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INSTANCE_2_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INSTANCE_3_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INSTANCE_3_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INSTANCE_4_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INSTANCE_4_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                     \
    \
    /* DFX Interrupts. Harrier has 6 DFX Pipes. Need to connect to all. */                                              \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_3_INTERRUPT_SUM_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_4_INTERRUPT_SUM_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_5_INTERRUPT_SUM_E),                      \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E,                                                                               \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_KEEP_ALIVE_AGING_E),                         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_KEEP_ALIVE_AGING_E),                         \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E,                                                                              \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_EXCESS_KEEPALIVE_E),                            \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_EXCESS_KEEPALIVE_E),                            \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E,                                                                             \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_INVALID_KEEPALIVE_HASH_E),                      \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_INVALID_KEEPALIVE_HASH_E),                      \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E,                                                                                    \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_RDI_STATUS_E),                                  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_RDI_STATUS_E),                                  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E,                                                                                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_TX_PERIOD_E),                                   \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_TX_PERIOD_E),                                   \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E,                                                                                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_MEG_LEVEL_EXCEPTION_E),                         \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_MEG_LEVEL_EXCEPTION_E),                         \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E,                                                                              \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_SOURCE_INTERFACE_EXCEPTION_E),               \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_SOURCE_INTERFACE_EXCEPTION_E),               \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E,                                                                                     \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_ILLEGAL_OAM_ENTRY_INDEX_E),                  \
    HARRIER_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_ILLEGAL_OAM_ENTRY_INDEX_E),                  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
                                                                                                                        \
 CPSS_PP_PHA_E,                                                                                                         \
    SET_PHA_EVENTS_MAC,                                                                                                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_TQ_PORT_HR_CROSSED_THRESHOLD_E,                                                                                \
    HARRIER_SET_HR_TRSH_EVENT_ALL_PORTS_MAC,                                                                            \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
CPSS_PP_MACSEC_SA_EXPIRED_E,\
    HARRIER_SET_EIP_164_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SA_EXPIRED_E),               \
MARK_END_OF_UNI_EV_CNS,                                                                                                 \
                                                                                                                        \
CPSS_PP_MACSEC_SA_PN_FULL_E,                                                                                            \
    HARRIER_SET_EIP_164_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SA_PN_THR_E),              \
MARK_END_OF_UNI_EV_CNS,                                                                                                 \
                                                                                                                        \
CPSS_PP_MACSEC_STATISTICS_SUMMARY_E,                                                                                    \
    HARRIER_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_SA_THR_E,       0),  \
    HARRIER_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_SECY_THR_E,     1),  \
    HARRIER_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_IFC0_THR_E,     2),  \
    HARRIER_SET_EIP_164_EVENT_SOURCE_PER_INGRESS_DIRECTION_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_IFC1_THR_E,  3),  \
    HARRIER_SET_EIP_164_EVENT_SOURCE_PER_INGRESS_DIRECTION_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_RXCAM_THR_E, 4),  \
                                                                                                                         \
    HARRIER_SET_EIP_163_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E,     5),  \
    HARRIER_SET_EIP_163_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_CHANNEL_THR_E,  6),  \
MARK_END_OF_UNI_EV_CNS,                                                                                                  \
                                                                                                                         \
CPSS_PP_MACSEC_TRANSFORM_ENGINE_ERR_E,                                                                                   \
    HARRIER_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(0),       \
    HARRIER_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(1),       \
    HARRIER_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(2),       \
    HARRIER_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(3),       \
    HARRIER_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(4),       \
    HARRIER_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(5),       \
    HARRIER_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(6),       \
    HARRIER_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(7),       \
    HARRIER_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(8),       \
    HARRIER_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(9),       \
    HARRIER_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(10),      \
    HARRIER_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(11),      \
    HARRIER_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(12),      \
    HARRIER_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(13),      \
    HARRIER_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(14),      \
MARK_END_OF_UNI_EV_CNS,                                                                                                            \
                                                                                                                                   \
CPSS_PP_MACSEC_EGRESS_SEQ_NUM_ROLLOVER_E,                                                                                          \
    HARRIER_SET_EIP_66_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SEQ_NUM_ROLLOVER_E),                      \
MARK_END_OF_UNI_EV_CNS,                                                     \
                                                                            \
CPSS_PP_LMU_LATENCY_EXCEED_THRESHOLD_E,                                     \
    HARRIER_PER_INSTANCE_LMU_LATENCY_OVER_THRESHOLD_MAC(0),                 \
    HARRIER_PER_INSTANCE_LMU_LATENCY_OVER_THRESHOLD_MAC(1),                 \
    HARRIER_PER_INSTANCE_LMU_LATENCY_OVER_THRESHOLD_MAC(2),                 \
MARK_END_OF_UNI_EV_CNS

#define PORTS_LINK_STATUS_CHANGE_EVENTS \
    /* MTI total ports 0..255 */                                                                \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(0,LINK_STATUS_CHANGE_E),                        \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(1,LINK_STATUS_CHANGE_E),                        \
    HARRIER_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(2,LINK_STATUS_CHANGE_E),                \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(0,LPCS_LINK_STATUS_CHANGE_E),                   \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(1,LPCS_LINK_STATUS_CHANGE_E),                   \
    HARRIER_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(2,LPCS_LINK_STATUS_CHANGE_E),           \
/* seg ports*/                                                                                  \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(0,LINK_STATUS_CHANGE_E),                        \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(1,LINK_STATUS_CHANGE_E),                        \
    HARRIER_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(2,LINK_STATUS_CHANGE_E)

#define PORTS_LINK_OK_CHANGE_EVENTS \
                                                                                                \
    /* MTI total ports 0..255 */                                                                \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(0, LINK_OK_CHANGE_E),                           \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(1, LINK_OK_CHANGE_E),                           \
    HARRIER_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(2, LINK_OK_CHANGE_E),                   \
/* seg ports*/                                                                                  \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(0, LINK_OK_CHANGE_E),                           \
    HARRIER_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(1, LINK_OK_CHANGE_E),                           \
    HARRIER_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(2, LINK_OK_CHANGE_E)

#define HARRIER_PORT_AN_HCD_RESOLUTION_DONE_EVENTS                \
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_0_HCD_FOUND_E,     0,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_1_HCD_FOUND_E,     2,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_2_HCD_FOUND_E,     4,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_3_HCD_FOUND_E,     6,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_4_HCD_FOUND_E,     8,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_5_HCD_FOUND_E,    10,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_6_HCD_FOUND_E,    12,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_7_HCD_FOUND_E,    14,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_0_HCD_FOUND_E,    16,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_1_HCD_FOUND_E,    18,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_2_HCD_FOUND_E,    20,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_3_HCD_FOUND_E,    22,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_4_HCD_FOUND_E,    24,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_5_HCD_FOUND_E,    26,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_6_HCD_FOUND_E,    28,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_7_HCD_FOUND_E,    30,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_2_PORT_NUM_0_HCD_FOUND_E,    32,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_2_PORT_NUM_1_HCD_FOUND_E,    34,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_2_PORT_NUM_2_HCD_FOUND_E,    36,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_2_PORT_NUM_3_HCD_FOUND_E,    38


#define HARRIER_PORT_AN_RESTART_DONE_EVENTS                \
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_0_AN_RESTART_E,     0,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_1_AN_RESTART_E,     2,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_2_AN_RESTART_E,     4,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_3_AN_RESTART_E,     6,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_4_AN_RESTART_E,     8,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_5_AN_RESTART_E,    10,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_6_AN_RESTART_E,    12,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_7_AN_RESTART_E,    14,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_0_AN_RESTART_E,    16,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_1_AN_RESTART_E,    18,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_2_AN_RESTART_E,    20,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_3_AN_RESTART_E,    22,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_4_AN_RESTART_E,    24,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_5_AN_RESTART_E,    26,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_6_AN_RESTART_E,    28,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_7_AN_RESTART_E,    30,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_2_PORT_NUM_0_AN_RESTART_E,    32,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_2_PORT_NUM_1_AN_RESTART_E,    34,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_2_PORT_NUM_2_AN_RESTART_E,    36,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_2_PORT_NUM_3_AN_RESTART_E,    38

#define HARRIER_PORT_INT_PM_PCS_LINK_TIMER_OUT_EVENTS                \
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,     0,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_1_INT_PM_PCS_LINK_TIMER_OUT_E,     2,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_2_INT_PM_PCS_LINK_TIMER_OUT_E,     4,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_3_INT_PM_PCS_LINK_TIMER_OUT_E,     6,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_4_INT_PM_PCS_LINK_TIMER_OUT_E,     8,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_5_INT_PM_PCS_LINK_TIMER_OUT_E,    10,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_6_INT_PM_PCS_LINK_TIMER_OUT_E,    12,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_0_PORT_NUM_7_INT_PM_PCS_LINK_TIMER_OUT_E,    14,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,    16,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_1_INT_PM_PCS_LINK_TIMER_OUT_E,    18,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_2_INT_PM_PCS_LINK_TIMER_OUT_E,    20,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_3_INT_PM_PCS_LINK_TIMER_OUT_E,    22,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_4_INT_PM_PCS_LINK_TIMER_OUT_E,    24,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_5_INT_PM_PCS_LINK_TIMER_OUT_E,    26,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_6_INT_PM_PCS_LINK_TIMER_OUT_E,    28,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_1_PORT_NUM_7_INT_PM_PCS_LINK_TIMER_OUT_E,    30,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_2_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,    32,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_2_PORT_NUM_1_INT_PM_PCS_LINK_TIMER_OUT_E,    34,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_2_PORT_NUM_2_INT_PM_PCS_LINK_TIMER_OUT_E,    36,\
    PRV_CPSS_HARRIER_ANP_INSTANCE_2_PORT_NUM_3_INT_PM_PCS_LINK_TIMER_OUT_E,    38

/* Interrupt cause to unified event map for harrier
 This Table is for unified event with extended data

  Array structure:
  1. The first element of the array and the elements after MARK_END_OF_UNI_EV_CNS are unified event types.
  2. Elements after unified event type are pairs of interrupt cause and event extended data,
     until MARK_END_OF_UNI_EV_CNS.
*/
static const GT_U32 harrierUniEvMapTableWithExtData[] = {
    CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
        PORTS_LINK_STATUS_CHANGE_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
        PORTS_LINK_OK_CHANGE_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_AN_HCD_FOUND_E,
        HARRIER_PORT_AN_HCD_RESOLUTION_DONE_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_AN_RESTART_E,
        HARRIER_PORT_AN_RESTART_DONE_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_AN_PARALLEL_DETECT_E,
        HARRIER_PORT_INT_PM_PCS_LINK_TIMER_OUT_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    HARRIER_COMMON_EVENTS
};

#define harrierUniEvMapTableUburst  \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        PRV_CPSS_HARRIER_QFC_UNIT_0_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E},    \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        PRV_CPSS_HARRIER_QFC_UNIT_1_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E},    \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        PRV_CPSS_HARRIER_QFC_UNIT_2_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E}     \


#define harrierUniEvMapTableMg_x(_mgIndex)  \
    {CPSS_PP_EB_AUQ_PENDING_E,              PRV_CPSS_HARRIER_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_AU_QUEUE_PENDING_E},                  \
    {CPSS_PP_EB_AUQ_FULL_E,                 PRV_CPSS_HARRIER_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_AU_QUEUE_FULL_E},                     \
    {CPSS_PP_EB_AUQ_OVER_E,                 PRV_CPSS_HARRIER_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_AU_QUEUE_OVERRUN_E},                  \
    {CPSS_PP_EB_AUQ_ALMOST_FULL_E,          PRV_CPSS_HARRIER_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_AU_QUEUE_ALMOST_FULL_E},              \
    {CPSS_PP_EB_FUQ_PENDING_E,              PRV_CPSS_HARRIER_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_FU_QUEUE_PENDING_E},                  \
    {CPSS_PP_EB_FUQ_FULL_E,                 PRV_CPSS_HARRIER_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_FU_QUEUE_FULL_E},                     \
    {CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,    PRV_CPSS_HARRIER_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_MG_READ_DMA_DONE_E},                  \
    {CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E,      PRV_CPSS_HARRIER_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_PC_IE_ADDRESS_UNMAPPED_E},            \
    {CPSS_PP_MISC_TWSI_TIME_OUT_E,          PRV_CPSS_HARRIER_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_I2C_TIME_OUT_INTERRUPT_E},            \
    {CPSS_PP_MISC_TWSI_STATUS_E,            PRV_CPSS_HARRIER_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_I2C_STATUS_INTERRUPT_E},              \
    {CPSS_PP_MISC_ILLEGAL_ADDR_E,           PRV_CPSS_HARRIER_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_INTERRUPT_E},         \
    {CPSS_PP_PORT_802_3_AP_E,               PRV_CPSS_HARRIER_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_Z80_INTERRUPT_E},                     \
    {CPSS_SRVCPU_IPC_E,                     PRV_CPSS_HARRIER_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_CM3_TO_HOST_DOORBELL_INTERRUPT_E}     \

/* Interrupt cause to unified event map for harrier
 This Table is for unified event without extended data
*/
static const GT_U32 harrierUniEvMapTable[][2] =
{
    {CPSS_PP_EB_SECURITY_BREACH_UPDATE_E,    PRV_CPSS_HARRIER_L2I_BRIDGE_INTERRUPT_CAUSE_UPDATE_SECURITY_BREACH_REGISTER_INT_E         },
    {CPSS_PP_MAC_NUM_OF_HOP_EXP_E,           PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NUM_OF_HOP_EX_P_E                       },
    {CPSS_PP_MAC_NA_LEARNED_E,               PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NA_LEARNT_E                             },
    {CPSS_PP_MAC_NA_NOT_LEARNED_E,           PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NA_NOT_LEARNT_E                         },
    {CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E,    PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AGE_VIA_TRIGGER_ENDED_E                 },
    {CPSS_PP_MAC_UPDATE_FROM_CPU_DONE_E,     PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AU_PROC_COMPLETED_INT_E                 },
    {CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E,     PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AU_MSG_TOCPU_READY_E                    },
    {CPSS_PP_MAC_NA_SELF_LEARNED_E,          PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NA_SELF_LEARNED_E                       },
    {CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E,      PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_N_AFROM_CPU_LEARNED_E                   },
    {CPSS_PP_MAC_NA_FROM_CPU_DROPPED_E,      PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_N_AFROM_CPU_DROPPED_E                   },
    {CPSS_PP_MAC_AGED_OUT_E,                 PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AGED_OUT_E                              },
    {CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E,      PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AU_FIFO_TO_CPU_IS_FULL_E                },
    {CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E,      PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_ADDRESS_OUT_OF_RANGE_E                  },
    {CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E, PRV_CPSS_HARRIER_TTI_TTI_ENGINE_INTERRUPT_CAUSE_CPU_ADDRESS_OUT_OF_RANGE_E                },
    {CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E,     PRV_CPSS_HARRIER_PCL_PCL_UNIT_INTERRUPT_CAUSE_MG_ADDR_OUTOF_RANGE_E                       },
    {CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E, PRV_CPSS_HARRIER_FDB_FDB_INTERRUPT_CAUSE_REGISTER_BLC_OVERFLOW_E                      },
    {CPSS_PP_EM_AUTO_LEARN_INDIRECT_ADDRESS_OUT_OF_RANGE_EVENT_E,       PRV_CPSS_HARRIER_EM_EM_INTERRUPT_CAUSE_REGISTER_INDIRECT_ADDRESS_OUT_OF_RANGE_E        },
    {CPSS_PP_EM_AUTO_LEARN_FLOW_ID_ALLOCATION_FAILED_EVENT_E,           PRV_CPSS_HARRIER_EM_EM_INTERRUPT_CAUSE_REGISTER_FLOW_ID_ALLOCATION_FAILED_E            },
    {CPSS_PP_EM_AUTO_LEARN_FLOW_ID_RANGE_FULL_EVENT_E,                  PRV_CPSS_HARRIER_EM_EM_INTERRUPT_CAUSE_REGISTER_FLOW_ID_RANGE_FULL_E                   },
    {CPSS_PP_EM_AUTO_LEARN_FLOW_ID_THRESHOLD_CROSSED_EVENT_E,           PRV_CPSS_HARRIER_EM_EM_INTERRUPT_CAUSE_REGISTER_FLOW_ID_THRESHOLD_CROSSED_E            },
    {CPSS_PP_EM_AUTO_LEARN_FLOW_ID_EMPTY_EVENT_E,                       PRV_CPSS_HARRIER_EM_EM_INTERRUPT_CAUSE_REGISTER_FLOW_ID_EMPTY_E                        },
    {CPSS_PP_EM_AUTO_LEARN_COLLISION_EVENT_E,                           PRV_CPSS_HARRIER_EM_EM_INTERRUPT_CAUSE_REGISTER_AUTO_LEARNING_COLLISION_E              },
    {CPSS_PP_EM_AUTO_LEARN_ILLEGAL_ADDRESS_ACCESS_EVENT_E,              PRV_CPSS_HARRIER_EM_EM_INTERRUPT_CAUSE_REGISTER_ILLEGAL_ADDRESS_ACCESS_E               },
    {CPSS_PP_EM_AUTO_LEARN_ILLEGAL_TABLE_OVERLAP_EVENT_E,               PRV_CPSS_HARRIER_EM_EM_INTERRUPT_CAUSE_REGISTER_ILLEGAL_TABLE_OVERLAP_E                },
    {CPSS_PP_EM_AUTO_LEARN_CHANNEL_0_SHM_ANSWER_FIFO_OVERFLOW_EVENT_E,  PRV_CPSS_HARRIER_EM_EM_INTERRUPT_CAUSE_REGISTER_CHANNEL_0_SHM_ANSWER_FIFO_OVERFLOW_E   },
    {CPSS_PP_EM_AUTO_LEARN_CHANNEL_1_SHM_ANSWER_FIFO_OVERFLOW_EVENT_E,  PRV_CPSS_HARRIER_EM_EM_INTERRUPT_CAUSE_REGISTER_CHANNEL_1_SHM_ANSWER_FIFO_OVERFLOW_E   },
    harrierUniEvMapTableMg_x(0)                                                                                             ,
    harrierUniEvMapTableMg_x(1)                                                                                             ,
    harrierUniEvMapTableUburst
};

/* Interrupt cause to unified event map for Harrier without extended data size */
static const GT_U32 harrierUniEvMapTableSize = (sizeof(harrierUniEvMapTable)/(sizeof(GT_U32)*2));

/**
* @internal prvCpssDrvHwPpHarrierSerdesIsrRead function
* @endinternal
*
* @brief Reads Serdes interrupt register value, clears and restores the ECC bits of Serdes General Control 0 register.
*
* @note   APPLICABLE DEVICES:      Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum      - The PP to read from.
* @param[in] portGroupId - The port group id. relevant only to 'multi-port-groups'
*                          devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr     - The register's address to read from.
* @param[in] dataPtr     - (pointer to) the register value.
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on hardware error
* @retval GT_NOT_INITIALIZED - if the driver was not initialized
* @retval GT_HW_ERROR        - on hardware error
*
*/
GT_STATUS prvCpssDrvHwPpHarrierSerdesIsrRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
)
{
    GT_STATUS rc = GT_OK; /* return code */
    GT_U32    eccSet; /* variable for ECC interrupt bits settings */
    GT_U32    data; /* register data */

    /* Read SERDES interrupt cause register */
    rc = prvCpssDrvHwPpPortGroupIsrRead(devNum, portGroupId, regAddr, dataPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* SERDES interrupt cause register bits [11:10] denote SRAM DOUBLE/SINGLE ECC error */
    eccSet = ((*dataPtr) >> 10) & 0x3;

    if(eccSet)
    {
        /* SERDES General Control 0 register is at offset 0x300 below to SERDES interrupt cause register */
        regAddr -= 0x300;

        /* Read SERDES General Control 0 register */
        rc = prvCpssDrvHwPpPortGroupIsrRead(devNum, portGroupId, regAddr, &data);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* reset SRAM DOUBLE/SINGLE ECC bits of General Control 0 register based on interrupt cause register */
        data |= eccSet << 16;
        /* Write back SERDES General Control 0 register to clear the ECC bits */
        rc = prvCpssDrvHwPpPortGroupIsrWrite(devNum, portGroupId, regAddr, data);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* unset SRAM DOUBLE/SINGLE ECC bits of General Control 0 register */
        data &= 0xFFFCFFFF;
        /* Write back SERDES General Control 0 register to clear the ECC bits */
        rc = prvCpssDrvHwPpPortGroupIsrWrite(devNum, portGroupId, regAddr, data);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChHarrierPortEventPortMapConvert function
* @endinternal
*
* @brief   Converts port unified event port from extended data to MAC/PHY port.
* @param[in] evConvertType          - event convert type
* @param[inout] portNumPtr          - (pointer to) port from event extended data
*
* @retval GT_BAD_PTR                - port parameter is NULL pointer
* @retval GT_BAD_PARAM              - wrong port type
* @retval GT_OUT_OF_RANGE           - port index is out of range
*/
static GT_STATUS prvCpssDxChHarrierPortEventPortMapConvert
(
    IN GT_SW_DEV_NUM                    devNum,
    IN PRV_CPSS_EV_CONVERT_DIRECTION_ENT evConvertType,
    INOUT GT_U32                *portNumPtr
)
{

    GT_U32      portNum;
    GT_U32      portIndex;
    PRV_CPSS_PORT_TYPE_ENT  portMacType;

    CPSS_NULL_PTR_CHECK_MAC(portNumPtr);

    portNum = *portNumPtr;

    if (evConvertType == PRV_CPSS_EV_CONVERT_DIRECTION_HW_INTERRUPT_TO_UNI_EVENT_E)
    {
        switch (portNum >> PORT_TYPE_OFFSET_BIT)
        {
            case MTI_PORT:
                portIndex = portNum & 0xFF; /* portIndex = 0..7 , 8..15 , 16..19 */
                *portNumPtr = portIndex << 1;/* x2 */ /*0,2,4,6...38 */
                break;
            case MTI_SEG_PORT:
                portIndex = portNum & 0xFF;
                if ((portIndex % 2) == 0) /* portIndex = 0, 8 , 16 */
                {
                    *portNumPtr = portIndex << 1;/* x2 */ /* 0,16,32 */
                }
                else /* portIndex = 1,9 */
                {
                    *portNumPtr = (4 + (portIndex-1)) << 1;/* x2 */ /* 8,24 */
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "portNum[%d] type[%d] is not supported",
                    portNum & 0xFF,
                    portNum >> 16);
        }
    }
    else /* PRV_CPSS_EV_CONVERT_DIRECTION_UNI_EVENT_TO_HW_INTERRUPT_E */
    {
        /* MAC port */
        portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portNum);

        portIndex = portNum >> 1;/* /2 */

        switch (portMacType)
        {
            case PRV_CPSS_PORT_MTI_100_E:
                *portNumPtr = (MTI_PORT << PORT_TYPE_OFFSET_BIT) | portIndex;
                break;
            case PRV_CPSS_PORT_MTI_400_E:
                *portNumPtr = (MTI_SEG_PORT << PORT_TYPE_OFFSET_BIT);
                if(4 == (portIndex & 0x7))
                {
                    *portNumPtr |= (portIndex+1) - 4;
                }
                else /* should be 0 == (portIndex & 0x7) */
                {
                    *portNumPtr |= portIndex;
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "portNum[%d] wrong type[%d]", portNum,portMacType);
        }
    }

    return GT_OK;
}
/**
* @internal prvCpssHarrierDedicatedEventsNonSharedConvertInfoSet function
* @endinternal
*
* @brief   set non-shared callbacks  needed by chIntCauseToUniEvConvertDedicatedTables(...) for the
*          Harrier  devices.
*/

void prvCpssHarrierDedicatedEventsNonSharedConvertInfoSet(IN GT_U8   devNum)
{
    PRV_INTERRUPT_CTRL_GET(devNum).eventHwIndexToMacNumConvertFunc = prvCpssDxChHarrierPortEventPortMapConvert;
    return;
}

/**
* @internal harrierDedicatedEventsConvertInfoSet function
* @endinternal
*
* @brief   set info needed by chIntCauseToUniEvConvertDedicatedTables(...) for the
*          Harrier devices.
*/
static void harrierDedicatedEventsConvertInfoSet(IN GT_U8   devNum)
{
    /* Interrupt cause to unified event map for Harrier with extended data size */
    GT_U32 harrierUniEvMapTableWithExtDataSize;

    harrierUniEvMapTableWithExtDataSize = (sizeof(harrierUniEvMapTableWithExtData)/(sizeof(GT_U32)));
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableWithExtDataArr   = harrierUniEvMapTableWithExtData;

    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableWithExtDataSize  = harrierUniEvMapTableWithExtDataSize;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableArr              = harrierUniEvMapTable;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableSize             = harrierUniEvMapTableSize;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.numReservedPorts              = 0;
    prvCpssHarrierDedicatedEventsNonSharedConvertInfoSet(devNum);

    return;
}

static void harrierUpdateIntScanTreeInfo(
    IN GT_U8   devNum,
    IN PRV_CPSS_DRV_INTERRUPT_SCAN_STC *intrScanArr,
    IN GT_U32  numOfElements
)
{
    GT_U32  ii;
    GT_U32  baseAddr;
    PRV_CPSS_DXCH_UNIT_ENT  unitId;
    GT_BOOL didError;

    for(ii = 0 ; ii < numOfElements ; ii++)
    {
        unitId = UNIT_ID_GET ((PRV_CPSS_DXCH_UNIT_ENT)intrScanArr[ii].gppId);

        didError = GT_FALSE;

        baseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum, unitId, &didError);

        if(didError == GT_TRUE)
        {

            /*support MG units */
            intrScanArr[ii].maskRegAddr  = CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS;

            /* NOTE: this macro not 'return' */
            CPSS_LOG_ERROR_MAC("Not supported unit [%d] in Harrier", unitId);
        }
        else
        {
            if(baseAddr > 0 && intrScanArr[ii].causeRegAddr > baseAddr)
            {
                /* ERROR : all addresses MUST be 0 based !!! */
                /* ERROR : all addresses MUST be 0 based !!! */
                /* ERROR : all addresses MUST be 0 based !!! */

               /* because we can't mask the address !!! */
               cpssOsPrintf("ERROR : Harrier interrupts tree : NODE hold causeRegAddr [0x%8.8x] but must be relative to the start of the unit [0x%8.8x] \n",
                intrScanArr[ii].causeRegAddr,
                baseAddr);

                continue;/* do not ADD the base --- ERROR */
            }

            intrScanArr[ii].causeRegAddr += baseAddr;
            intrScanArr[ii].maskRegAddr  += baseAddr;
        }
    }
}

extern GT_STATUS prvCpssDxChUnitBaseTableInit(IN CPSS_PP_FAMILY_TYPE_ENT devFamily);

/**
* @internal prvCpssDrvPpIntDefDxChHarrierMacSecRegIdInit function
* @endinternal
*
* @brief   Init macsec register ids array.
*
* */
static GT_STATUS prvCpssDrvPpIntDefDxChHarrierMacSecRegIdInit
(
    GT_VOID
)
{
    GT_U32 iter;

    /* check that Global DB size is in synch with required one */
    if (PRV_CPSS_GLOBAL_DB_HARRIER_MACSEC_REGID_ARRAY_SIZE_CNS != PRV_CPSS_DXCH_HARRIER_MACSEC_REGID_ARRAY_SIZE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    for(iter = 0; iter < PRV_CPSS_DXCH_HARRIER_MACSEC_REGID_ARRAY_SIZE_CNS - 1; iter++)
    {
        PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(macSecRegIdNotAccessibleBeforeMacSecInitHarrier[iter],
                                                                               PRV_CPSS_DXCH_HARRIER_MACSEC_REGID_FIRST_CNS + iter);
    }
    /* end identifier for the array */
    PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(macSecRegIdNotAccessibleBeforeMacSecInitHarrier[iter],
                                                                           PRV_CPSS_MACSEC_REGID_LAST_CNS);
    return GT_OK;
}

/**
* @internal prvCpssDrvPpIntDefDxChHarrierInit function
* @endinternal
*
* @brief   Interrupts initialization for the Harrier devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefDxChHarrierInit
(
    IN GT_U8   devNum ,
    IN GT_U32   ppRevision
)
{
    GT_STATUS   rc;         /* return code */
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
                            /* pointer to device interrupt info */
    PRV_CPSS_DRV_INTERRUPTS_INFO_STC devFamilyInterrupstInfo =
    {
        NULL,
        PRV_CPSS_DRV_FAMILY_DXCH_HARRIER_E,
        HARRIER_NUM_ELEMENTS_IN_SCAN_TREE_CNS ,
        harrierIntrScanArr,
        PRV_CPSS_HARRIER_LAST_INT_E/32,
        NULL, NULL, NULL
    };

    ppRevision = ppRevision;

    /*set info needed by chIntCauseToUniEvConvertDedicatedTables(...) */
    harrierDedicatedEventsConvertInfoSet(devNum);

    /* init macsec register ids */
    rc = prvCpssDrvPpIntDefDxChHarrierMacSecRegIdInit();
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    if(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(harrierInitDone) == GT_TRUE)
    {
        return GT_OK;
    }

    rc = prvCpssDrvDxExMxInterruptsMemoryInit(&devFamilyInterrupstInfo, 1);
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    (void)prvCpssDxChUnitBaseTableInit(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devFamily);/*needed for calling prvCpssDxChHwUnitBaseAddrGet(...) */

    /* 1. Align base addresses for mask registers */
    harrierUpdateIntScanTreeInfo(devNum,
        devFamilyInterrupstInfo.intrScanOutArr ,
        devFamilyInterrupstInfo.numScanElements);

    /*
       1. fill the array of mask registers addresses
       2. fill the array of default values for the mask registers
       3. update the bits of nonSumBitMask in the scan tree
    */
    rc = prvCpssDrvExMxDxHwPpMaskRegInfoGet(
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devFamily,
            devFamilyInterrupstInfo.numScanElements          ,
            devFamilyInterrupstInfo.intrScanOutArr              ,
            devFamilyInterrupstInfo.numMaskRegisters         ,
            devFamilyInterrupstInfo.maskRegDefaultSummaryArr ,
            devFamilyInterrupstInfo.maskRegMapArr            );
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    rc = prvCpssDrvPpIntUnMappedMgSetSkip(devNum, devFamilyInterrupstInfo.maskRegMapArr, devFamilyInterrupstInfo.numMaskRegisters);
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_HARRIER_E]);
    devInterruptInfoPtr->numOfScanElements               = devFamilyInterrupstInfo.numScanElements         ;
    devInterruptInfoPtr->interruptsScanArray             = devFamilyInterrupstInfo.intrScanOutArr             ;
    devInterruptInfoPtr->maskRegistersDefaultValuesArray = devFamilyInterrupstInfo.maskRegDefaultSummaryArr;
    devInterruptInfoPtr->maskRegistersAddressesArray     = devFamilyInterrupstInfo.maskRegMapArr           ;

    devInterruptInfoPtr->numOfInterrupts = devFamilyInterrupstInfo.numMaskRegisters * 32;
    devInterruptInfoPtr->fdbTrigEndedId = 0;/* don't care */
    devInterruptInfoPtr->fdbTrigEndedCbPtr = NULL;
    devInterruptInfoPtr->hasFakeInterrupts = GT_FALSE;
    devInterruptInfoPtr->firstFakeInterruptId = 0;/* don't care */
    devInterruptInfoPtr->drvIntCauseToUniEvConvertFunc = &prvCpssDrvPpPortGroupIntCheetahIntCauseToUniEvConvert;
    devInterruptInfoPtr->numOfInterruptRegistersNotAccessibleBeforeStartInit = 0;
    devInterruptInfoPtr->notAccessibleBeforeStartInitPtr = NULL;
    PRV_INTERRUPT_CTRL_GET(devNum).intMaskSetFptr = NULL;
    devInterruptInfoPtr->macSecRegIdArray =
        PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(macSecRegIdNotAccessibleBeforeMacSecInitHarrier);

    PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(harrierInitDone, GT_TRUE);

    return GT_OK;
}




#ifdef DUMP_DEFAULT_INFO

/**
* @internal prvCpssDrvPpIntDefHarrierPrint function
* @endinternal
*
* @brief   print the interrupts arrays info of Harrier devices
*/
void  prvCpssDrvPpIntDefHarrierPrint(
    void
)
{
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC *  interruptsScanArray;

    cpssOsPrintf("Harrier - start : \n");
    devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_HARRIER_E]);

    /* remove the 'const' from the array */
    interruptsScanArray = devInterruptInfoPtr->interruptsScanArray;
    /* port group 0 */
    prvCpssDrvPpIntDefPrint(devInterruptInfoPtr->numOfScanElements,
                            interruptsScanArray,
                            AC5P_NUM_MASK_REGISTERS_CNS,
                            devInterruptInfoPtr->maskRegistersAddressesArray,
                            devInterruptInfoPtr->maskRegistersDefaultValuesArray);
    cpssOsPrintf("Harrier - End : \n");

}
#endif/*DUMP_DEFAULT_INFO*/

/**
* @internal prvCpssDrvPpIntDefHarrierPrint_regInfoByInterruptIndex function
* @endinternal
*
* @brief   print for Harrier devices the register info according to value in
*         PRV_CPSS_HARRIER_LAST_INT_E (assuming that this register is part of
*         the interrupts tree)
*/
void  prvCpssDrvPpIntDefHarrierPrint_regInfoByInterruptIndex(
    IN PRV_CPSS_HARRIER_INT_CAUSE_ENT   interruptId
)
{
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
    GT_CHAR*    deviceNamePtr = "Harrier interrupts";
    GT_U32      numMaskRegisters = PRV_CPSS_HARRIER_LAST_INT_E/32;

    cpssOsPrintf("Harrier - start regInfoByInterruptIndex : \n");

    if(interruptId >= PRV_CPSS_HARRIER_LAST_INT_E)
    {
        cpssOsPrintf("interruptId[%d] >= maximum(%d) \n" ,
            interruptId, PRV_CPSS_HARRIER_LAST_INT_E);
    }
    else
    {
        if(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(harrierInitDone) == GT_FALSE)
        {
            cpssOsPrintf("Harrier - ERROR : the 'IntrScanArr' was not initialized with 'base addresses' yet End \n");
            return;
        }

        if((GT_U32)interruptId >= (numMaskRegisters << 5))
        {
            cpssOsPrintf("[%s] skipped : interruptId[%d] >= maximum(%d) \n" ,
                deviceNamePtr , interruptId, (numMaskRegisters << 5));
        }

        devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_HARRIER_E]);

        cpssOsPrintf("start [%s]: \n", deviceNamePtr);
        prvCpssDrvPpIntDefPrint_regInfoByInterruptIndex(devInterruptInfoPtr->numOfScanElements, devInterruptInfoPtr->interruptsScanArray, interruptId, GT_FALSE);
        cpssOsPrintf("ended [%s] \n\n", deviceNamePtr);
    }
    cpssOsPrintf("Harrier - End regInfoByInterruptIndex : \n");
}

