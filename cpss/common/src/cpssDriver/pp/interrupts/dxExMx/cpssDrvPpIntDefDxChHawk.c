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
* @file cpssDrvPpIntDefDxChHawk.c
*
* @brief This file includes the definition and initialization of the interrupts
* init. parameters tables. -- DxCh Hawk devices
*
* @version   1
********************************************************************************
*/

#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsAlleyCat5P.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxUtilLion.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddr.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* First macsec register id as per Hawk interrupts enum */
#define  PRV_CPSS_DXCH_MACSEC_REGID_FIRST_CNS (PRV_CPSS_AC5P_MSEC_EGR_163_INST_0_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E >> 5)


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

#define  PRV_CPSS_DXCH_UNIT_ANP_400G_1_E    PRV_CPSS_DXCH_UNIT_ANP_400G_0_E
#define  PRV_CPSS_DXCH_UNIT_ANP_400G_2_E    PRV_CPSS_DXCH_UNIT_ANP_400G_0_E + 1
#define  PRV_CPSS_DXCH_UNIT_ANP_400G_3_E    PRV_CPSS_DXCH_UNIT_ANP_400G_0_E + 2
#define  PRV_CPSS_DXCH_UNIT_ANP_400G_4_E    PRV_CPSS_DXCH_UNIT_ANP_400G_0_E + 3

#define  PRV_CPSS_DXCH_UNIT_400G6_TAI0_E    PRV_CPSS_DXCH_UNIT_400G0_TAI0_E
#define  PRV_CPSS_DXCH_UNIT_400G6_TAI1_E    PRV_CPSS_DXCH_UNIT_400G0_TAI1_E
#define  PRV_CPSS_DXCH_UNIT_400G7_TAI0_E    PRV_CPSS_DXCH_UNIT_400G1_TAI0_E
#define  PRV_CPSS_DXCH_UNIT_400G7_TAI1_E    PRV_CPSS_DXCH_UNIT_400G1_TAI1_E
#define  PRV_CPSS_DXCH_UNIT_400G8_TAI0_E    PRV_CPSS_DXCH_UNIT_400G2_TAI0_E
#define  PRV_CPSS_DXCH_UNIT_400G8_TAI1_E    PRV_CPSS_DXCH_UNIT_400G2_TAI1_E
#define  PRV_CPSS_DXCH_UNIT_400G9_TAI0_E    PRV_CPSS_DXCH_UNIT_400G3_TAI0_E
#define  PRV_CPSS_DXCH_UNIT_400G9_TAI1_E    PRV_CPSS_DXCH_UNIT_400G3_TAI1_E

#define  PRV_CPSS_DXCH_UNIT_USX11_TAI0_E    PRV_CPSS_DXCH_UNIT_USX0_TAI0_E
#define  PRV_CPSS_DXCH_UNIT_USX11_TAI1_E    PRV_CPSS_DXCH_UNIT_USX0_TAI1_E
#define  PRV_CPSS_DXCH_UNIT_USX12_TAI0_E    PRV_CPSS_DXCH_UNIT_USX1_TAI0_E
#define  PRV_CPSS_DXCH_UNIT_USX12_TAI1_E    PRV_CPSS_DXCH_UNIT_USX1_TAI1_E

#define  PRV_CPSS_DXCH_UNIT_MIF_400G_1_E    PRV_CPSS_DXCH_UNIT_MIF_400G_0_E
#define  PRV_CPSS_DXCH_UNIT_MIF_400G_2_E    PRV_CPSS_DXCH_UNIT_MIF_400G_0_E + 1
#define  PRV_CPSS_DXCH_UNIT_MIF_400G_3_E    PRV_CPSS_DXCH_UNIT_MIF_400G_0_E + 2
#define  PRV_CPSS_DXCH_UNIT_MIF_400G_4_E    PRV_CPSS_DXCH_UNIT_MIF_400G_0_E + 3

#define  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_INST_0_E    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_0_E
#define  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_INST_0_E    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_0_E
#define  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_INST_0_E    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_0_E
#define  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_INST_0_E    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_0_E
#define  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_INST_1_E    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_1_E
#define  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_INST_1_E    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_1_E
#define  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_INST_1_E    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_1_E
#define  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_INST_1_E    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_1_E
#define  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_INST_2_E    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_2_E
#define  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_INST_2_E    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_2_E
#define  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_INST_2_E    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_2_E
#define  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_INST_2_E    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_2_E
#define  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_INST_3_E    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_3_E
#define  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_INST_3_E    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_3_E
#define  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_INST_3_E    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_3_E
#define  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_INST_3_E    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_3_E

/* convert bit in the enum of the huge interrupts of the device into BIT_0..BIT_31 in a register */
#define DEVICE_INT_TO_BIT_MAC(deviceInterruptId)    \
            (1 << ((deviceInterruptId) & 0x1f))

#define FDB_REG_MASK_CNS    \
    (0xFFFFFFFF & ~                                                             \
        (DEVICE_INT_TO_BIT_MAC(PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AGE_VIA_TRIGGER_ENDED_E) |     \
         DEVICE_INT_TO_BIT_MAC(PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AU_PROC_COMPLETED_INT_E)))

/* unit : PRV_CPSS_DXCH_UNIT_MG_E */
#define MG_UNIT(_mgIndex) \
    (_mgIndex ? (PRV_CPSS_DXCH_UNIT_MG_0_1_E + (_mgIndex-1)) : PRV_CPSS_DXCH_UNIT_MG_E)


#define UNIT_IN_GOP(unit, index) ((unit) + (index))
#define UNIT_ID_GET(unit) ((unit) & 0xFFFF)

#define HAWK_LMU_LATENCY_INTERRUPT_CAUSE_MAC(_threshold)                    \
        ((0x8100 ) + (0x4 * (_threshold)))

#define HAWK_LMU_LATENCY_INTERRUPT_MASK_MAC(_threshold)                     \
        ((0x8180 ) + (0x4 * (_threshold)))

/* USX MAC - cause */
#define PORT_USX_MAC_INTERRUPT_CAUSE_MAC(port)                              \
        (0x38 + ((port) * 0x28))
/* USX MAC - mask */
#define PORT_USX_MAC_INTERRUPT_MASK_MAC(port)                               \
        (0x3C + ((port) * 0x28))

/* 400G MAC - cause */
#define PORT_400G_MAC_INTERRUPT_CAUSE_MAC(port)                             \
        (0x94 + ((port) * 0x18))
/* 400G - mask */
#define PORT_400G_MAC_INTERRUPT_MASK_MAC(port)                              \
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
        PRV_CPSS_AC5P_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_I2C_TIME_OUT_INTERRUPT_E,         \
        PRV_CPSS_AC5P_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_SERDES_ADDRESS_OUT_OF_RANGE_VIOLATION_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                     \
        /* HostCpuDoorbellIntSum - single register per MG accessed from MG Internal Cause0/1/2/3 */ \
        {24, GT_FALSE, MG_UNIT(_mgIndex), NULL,           \
            0x00000518, 0x0000051c,                                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
            PRV_CPSS_AC5P_MG##_mgIndex##_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_0_E,                \
            PRV_CPSS_AC5P_MG##_mgIndex##_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_30_E,               \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 0, NULL, NULL}

#define   mt_NODE_MAC(bit)                                                                          \
    /* MT FDBIntSum - FDB Interrupt Cause */                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_FDB_E, NULL,                                                 \
        0x000001b0, 0x000001b4,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NUM_OF_HOP_EX_P_E,                           \
        PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_ADDRESS_OUT_OF_RANGE_E,                      \
        FILLED_IN_RUNTIME_CNS, 0x0, FDB_REG_MASK_CNS,                                               \
             0, NULL, NULL}

#define   em_NODE_MAC(bit)                                                                          \
    /* EM FDBIntSum - FDB Interrupt Cause */                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EM_E, NULL,                                                  \
        0x00004000, 0x00004004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_EM_EM_INTERRUPT_CAUSE_REGISTER_INDIRECT_ADDRESS_OUT_OF_RANGE_E,               \
        PRV_CPSS_AC5P_EM_EM_INTERRUPT_CAUSE_REGISTER_CHANNEL_1_SHM_ANSWER_FIFO_OVERFLOW_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF,                                                     \
             0, NULL, NULL}

#define HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(bit, _instance, _threshold)                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_LMU_##_instance##_E, NULL,                               \
        HAWK_LMU_LATENCY_INTERRUPT_CAUSE_MAC(_threshold),                                           \
        HAWK_LMU_LATENCY_INTERRUPT_MASK_MAC(_threshold),                                            \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_PCA_MACRO_INST_##_instance##_N_##_threshold##_I_0_LMU_LATENCY_OVER_THRESHOLD_E, \
        PRV_CPSS_AC5P_PCA_MACRO_INST_##_instance##_N_##_threshold##_I_30_LMU_LATENCY_OVER_THRESHOLD_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC(bit, _instance, _threshold)                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_LMU_##_instance##_E, NULL,                               \
        HAWK_LMU_LATENCY_INTERRUPT_CAUSE_MAC(_threshold),                                           \
        HAWK_LMU_LATENCY_INTERRUPT_MASK_MAC(_threshold),                                            \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_N_##_threshold##_I_0_LMU_LATENCY_OVER_THRESHOLD_E,    \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_N_##_threshold##_I_30_LMU_LATENCY_OVER_THRESHOLD_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define lmu_PCA_SUB_TREE_MAC(bit, _instance)                                                        \
    /* LMU Cause Summary */                                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_LMU_##_instance##_E, NULL,                               \
        0x00008230, 0x00008234,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_PCA_MACRO_INST_##_instance##_LMU_CAUSE_SUMMARY_N_0_LMU_LATENCY_OVER_THRESHOLD_SUM_E,  \
        PRV_CPSS_AC5P_PCA_MACRO_INST_##_instance##_LMU_CAUSE_SUMMARY_N_0_LMU_LATENCY_FIFO_FULL_SUM_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 17, NULL, NULL},                                         \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 1, _instance,  0),  \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 2, _instance,  1),  \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 3, _instance,  2),  \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 4, _instance,  3),  \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 5, _instance,  4),  \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 6, _instance,  5),  \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 7, _instance,  6),  \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 8, _instance,  7),  \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC( 9, _instance,  8),  \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(10, _instance,  9),  \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(11, _instance, 10),  \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(12, _instance, 11),  \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(13, _instance, 12),  \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(14, _instance, 13),  \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(15, _instance, 14),  \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(16, _instance, 15),  \
        HAWK_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(17, _instance, 16)

#define lmu_MSEC_SUB_TREE_MAC(bit, _instance)                                                       \
    /* LMU Cause Summary */                                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_LMU_##_instance##_E, NULL,                               \
        0x00008230, 0x00008234,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_LMU_CAUSE_SUMMARY_N_0_LMU_LATENCY_OVER_THRESHOLD_SUM_E,      \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_LMU_CAUSE_SUMMARY_N_0_LMU_LATENCY_FIFO_FULL_SUM_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 17, NULL, NULL},          \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC( 1, _instance,  0),  \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC( 2, _instance,  1),  \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC( 3, _instance,  2),  \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC( 4, _instance,  3),  \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC( 5, _instance,  4),  \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC( 6, _instance,  5),  \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC( 7, _instance,  6),  \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC( 8, _instance,  7),  \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC( 9, _instance,  8),  \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC(10, _instance,  9),  \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC(11, _instance, 10),  \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC(12, _instance, 11),  \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC(13, _instance, 12),  \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC(14, _instance, 13),  \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC(15, _instance, 14),  \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC(16, _instance, 15),  \
        HAWK_LMU_MSEC_LATENCY_OVER_THRESHOLD_INT_MAC(17, _instance, 16)

#define mac_pcs_USX_NODE_MAC(bit, unit, inst, port)                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_USX_##inst##_MAC_##unit##_E, NULL,                   \
        PORT_USX_MAC_INTERRUPT_CAUSE_MAC(port),                                                 \
        PORT_USX_MAC_INTERRUPT_MASK_MAC(port),                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_AC5P_USX_4_AND_8_MAC_MACRO_INST_##unit##_USX_MAC_UNIT_INST_##inst##_PORT_NUM_##port##_LINK_OK_CHANGE_E,   \
        PRV_CPSS_AC5P_USX_4_AND_8_MAC_MACRO_INST_##unit##_USX_MAC_UNIT_INST_##inst##_PORT_NUM_##port##_LPCS_LINK_STATUS_CLEAN_CHANGE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define mac_pcs_USX0_SUB_TREE_MAC(bit)                                                              \
    mac_pcs_USX_SUB_TREE_MAC(bit, 0, 0)

#define mac_pcs_USX1_SUB_TREE_MAC(bit)                                                              \
    mac_pcs_USX_SUB_TREE_MAC(bit, 0, 1)

#define mac_pcs_USX2_SUB_TREE_MAC(bit)                                                              \
    mac_pcs_USX_SUB_TREE_MAC(bit, 0, 2)

#define mac_pcs_USX3_SUB_TREE_MAC(bit)                                                              \
    mac_pcs_USX_SUB_TREE_MAC(bit, 1, 0)

#define mac_pcs_USX4_SUB_TREE_MAC(bit)                                                              \
    mac_pcs_USX_SUB_TREE_MAC(bit, 1, 1)

#define mac_pcs_USX5_SUB_TREE_MAC(bit)                                                              \
    mac_pcs_USX_SUB_TREE_MAC(bit, 1, 2)


#define mac_pcs_USX_SUB_TREE_MAC(bit, unit, inst)                                                   \
    /* USX MAC */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_USX_##inst##_MAC_##unit##_E, NULL,                       \
        0x00000018, 0x0000001c,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_USX_4_AND_8_MAC_MACRO_INST_##unit##_USX_MAC_UNIT_INST_##inst##_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E, \
        PRV_CPSS_AC5P_USX_4_AND_8_MAC_MACRO_INST_##unit##_USX_MAC_UNIT_INST_##inst##_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_7_P_INT_SUM_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8, NULL, NULL},                                     \
            mac_pcs_USX_NODE_MAC(1, unit, inst, 0),                                                 \
            mac_pcs_USX_NODE_MAC(2, unit, inst, 1),                                                 \
            mac_pcs_USX_NODE_MAC(3, unit, inst, 2),                                                 \
            mac_pcs_USX_NODE_MAC(4, unit, inst, 3),                                                 \
            mac_pcs_USX_NODE_MAC(5, unit, inst, 4),                                                 \
            mac_pcs_USX_NODE_MAC(6, unit, inst, 5),                                                 \
            mac_pcs_USX_NODE_MAC(7, unit, inst, 6),                                                 \
            mac_pcs_USX_NODE_MAC(8, unit, inst, 7)

#define anp_CPU_NODE_MAC(bit)                                                                           \
    /* CPU ANP */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_CPU_0_E, NULL,                                               \
        0x00000088, 0x0000008c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_CPU_PORT_INST_0_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                  \
        PRV_CPSS_AC5P_CPU_PORT_INST_0_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define anp_global_400G_NODE_MAC(bit, _instance)                                                        \
    /* 400G ANP */                                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_400G_##_instance##_E, NULL,                                  \
        0x00000088, 0x0000008c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_400G_PCS_INST_##_instance##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,      \
        PRV_CPSS_AC5P_400G_PCS_INST_##_instance##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define anp_400G_port_NODE_MAC(bit, _instance, _port)                                                   \
    /* 400G ANP */                                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_400G_##_instance##_E, NULL,                                  \
        (0x00000100 + _port*0x2a4), (0x00000104 + _port*0x2a4),                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_400G_PCS_INST_##_instance##_PORT_NUM_##_port##_AN_RESTART_E,                      \
        PRV_CPSS_AC5P_400G_PCS_INST_##_instance##_PORT_NUM_##_port##_INT_WAIT_PWRDN_TIME_OUT_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define anp_400G_port2_NODE_MAC(bit, _instance, _port)                                                  \
    /* 400G ANP */                                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_400G_##_instance##_E, NULL,                                  \
        (0x00000108 + _port*0x2a4), (0x0000010c + _port*0x2a4),                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_400G_PCS_INST_##_instance##_PORT_NUM_##_port##_TX_TFIFO_W_ERR_E,                  \
        PRV_CPSS_AC5P_400G_PCS_INST_##_instance##_PORT_NUM_##_port##_RX_TFIFO_EMPTY_E,                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define anp_400G_SUB_TREE_MAC(bit, _instance)                                                           \
    /* 400G ANP */                                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_400G_##_instance##_E, NULL,                                  \
        0x00000094, 0x00000098,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_400G_PCS_INST_##_instance##_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_E,                 \
        PRV_CPSS_AC5P_400G_PCS_INST_##_instance##_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_7_P_INT2_SUM_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 17, NULL, NULL},                                        \
            anp_global_400G_NODE_MAC(1, _instance),                                                     \
            anp_400G_port_NODE_MAC(2, _instance, 0),                                                    \
            anp_400G_port2_NODE_MAC(3, _instance, 0),                                                   \
            anp_400G_port_NODE_MAC(4, _instance, 1),                                                    \
            anp_400G_port2_NODE_MAC(5, _instance, 1),                                                   \
            anp_400G_port_NODE_MAC(6, _instance, 2),                                                    \
            anp_400G_port2_NODE_MAC(7, _instance, 2),                                                   \
            anp_400G_port_NODE_MAC(8, _instance, 3),                                                    \
            anp_400G_port2_NODE_MAC(9, _instance, 3),                                                   \
            anp_400G_port_NODE_MAC(10, _instance, 4),                                                   \
            anp_400G_port2_NODE_MAC(11, _instance, 4),                                                  \
            anp_400G_port_NODE_MAC(12, _instance, 5),                                                   \
            anp_400G_port2_NODE_MAC(13, _instance, 5),                                                  \
            anp_400G_port_NODE_MAC(14, _instance, 6),                                                   \
            anp_400G_port2_NODE_MAC(15, _instance, 6),                                                  \
            anp_400G_port_NODE_MAC(16, _instance, 7),                                                   \
            anp_400G_port2_NODE_MAC(17, _instance, 7)

#define anp_global_USX_NODE_MAC(bit, _unit, _instance)                                                        \
    /* USX ANP */                                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_##_instance##_E, NULL,                                  \
        0x00000088, 0x0000008c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_USX_PCS_UNIT_##_unit##_USX_PCS_INST_##_instance##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,      \
        PRV_CPSS_AC5P_USX_PCS_UNIT_##_unit##_USX_PCS_INST_##_instance##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define anp_USX_port_NODE_MAC(bit, _unit, _instance, _port)                                             \
    /* USX ANP */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_##_instance##_E, NULL,                                   \
        0x00000100, 0x00000104,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_USX_PCS_UNIT_##_unit##_USX_PCS_INST_##_instance##_PORT_NUM_##_port##_AN_RESTART_E,\
        PRV_CPSS_AC5P_USX_PCS_UNIT_##_unit##_USX_PCS_INST_##_instance##_PORT_NUM_##_port##_INT_WAIT_PWRDN_TIME_OUT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define anp_USX_port2_NODE_MAC(bit, _unit, _instance, _port)                                            \
    /* USX ANP */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_400G_##_instance##_E, NULL,                                  \
        0x00000108, 0x0000010c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_USX_PCS_UNIT_##_unit##_USX_PCS_INST_##_instance##_PORT_NUM_##_port##_TX_TFIFO_W_ERR_E,    \
        PRV_CPSS_AC5P_USX_PCS_UNIT_##_unit##_USX_PCS_INST_##_instance##_PORT_NUM_##_port##_RX_TFIFO_EMPTY_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define anp_USX_SUB_TREE_MAC(bit, _unit, _instance)                                                     \
    /* 400G ANP */                                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_##_instance##_E, NULL,                                   \
        0x00000094, 0x00000098,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_USX_PCS_UNIT_##_unit##_USX_PCS_INST_##_instance##_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_E,   \
        PRV_CPSS_AC5P_USX_PCS_UNIT_##_unit##_USX_PCS_INST_##_instance##_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT2_SUM_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 17, NULL, NULL},                                        \
            anp_global_USX_NODE_MAC(1, _unit, _instance),                                                      \
            anp_USX_port_NODE_MAC(2, _unit, _instance, 0),                                                     \
            anp_USX_port2_NODE_MAC(3, _unit, _instance, 0),                                                    \
            anp_USX_port_NODE_MAC(4, _unit, _instance, 1),                                                     \
            anp_USX_port2_NODE_MAC(5, _unit, _instance, 1),                                                    \
            anp_USX_port_NODE_MAC(6, _unit, _instance, 2),                                                     \
            anp_USX_port2_NODE_MAC(7, _unit, _instance, 2),                                                    \
            anp_USX_port_NODE_MAC(8, _unit, _instance, 3),                                                     \
            anp_USX_port2_NODE_MAC(9, _unit, _instance, 3),                                                    \
            anp_USX_port_NODE_MAC(10, _unit, _instance, 4),                                                    \
            anp_USX_port2_NODE_MAC(11, _unit, _instance, 4),                                                   \
            anp_USX_port_NODE_MAC(12, _unit, _instance, 5),                                                    \
            anp_USX_port2_NODE_MAC(13, _unit, _instance, 5),                                                   \
            anp_USX_port_NODE_MAC(14, _unit, _instance, 6),                                                    \
            anp_USX_port2_NODE_MAC(15, _unit, _instance, 6),                                                   \
            anp_USX_port_NODE_MAC(16, _unit, _instance, 7),                                                    \
            anp_USX_port2_NODE_MAC(17, _unit, _instance, 7)

#define pdx_NODE_MAC(bit)                                                                           \
    /* PDX Interrupt Summary */                                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PDX_E, NULL,                                             \
        0x00001408, 0x0000140c,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_TXQ_PR_PDX_FUNCTIONAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,               \
        PRV_CPSS_AC5P_TXQ_PR_PDX_FUNCTIONAL_INTERRUPT_CAUSE_CHANNELS_RANGE_3_DX_DESC_DROP_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pfcc_NODE_MAC(bit)                                                                          \
    /* PFCC Interrupt Summary */                                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E, NULL,                                            \
        0x00000100, 0x00000104,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_TXQ_PR_PFCC_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                         \
        PRV_CPSS_AC5P_TXQ_PR_PFCC_INTERRUPT_CAUSE_SOURCE_PORT_COUNTER_OVERFLOW_E,                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define psi_NODE_MAC(bit)                                                                           \
    /* PSI Interrupt Summary */                                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PSI_E, NULL,                                              \
        0x0000020c, 0x00000210,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_PSI_REGS_PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_E,                           \
        PRV_CPSS_AC5P_PSI_REGS_PSI_INTERRUPT_CAUSE_PDQ_ACCESS_MAP_ERROR_E,                          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define shm_NODE_MAC(bit)                                                                           \
    /* SHM Interrupt Summary */                                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_SHM_E, NULL,                                                 \
        0x000000e0, 0x000000e4,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_SHM_SHM_INTERRUPT_CAUSE_RF_ERR_CAUSE_SUM_E,                                   \
        PRV_CPSS_AC5P_SHM_SHM_INTERRUPT_CAUSE_ENGINE_ERR_CAUSE_SUM_E,                               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define tcam_SUB_TREE_MAC(bit)                                                                      \
    /* TCAM Interrupt Summary */                                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TCAM_E, NULL,                                                \
        0x00501004, 0x00501000,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_TCAM_TCAM_INTERRUPTS_SUMMARY_CAUSE_TCAM_LOGIC_INTERRUPT_SUMMARY_E,            \
        PRV_CPSS_AC5P_TCAM_TCAM_INTERRUPTS_SUMMARY_CAUSE_TCAM_LOGIC_INTERRUPT_SUMMARY_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                     \
        /* TCAM Interrupt */                                                                        \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TCAM_E, NULL,                                              \
            0x0050100C, 0x00501008,                                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
            PRV_CPSS_AC5P_TCAM_TCAM_INTERRUPT_CAUSE_CPU_ADDRESS_OUT_OF_RANGE_E,                     \
            PRV_CPSS_AC5P_TCAM_TCAM_INTERRUPT_CAUSE_TABLE_OVERLAPPING_E,                            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  txqs_tai_NODE_MAC(bit, _tai)                                                    \
    /* TXQ TAI */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQS_TAI##_tai##_E, NULL,                               \
        0x00000000, 0x00000004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_TXQS_INST_0_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,   \
        PRV_CPSS_AC5P_TXQS_INST_0_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  txqs_1_tai_NODE_MAC(bit, _tai)                                                    \
    /* TXQ TAI */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQS1_TAI##_tai##_E, NULL,                               \
        0x00000000, 0x00000004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_TXQS_INST_1_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,   \
        PRV_CPSS_AC5P_TXQS_INST_1_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  tti_tai_NODE_MAC(bit, _instance, _tai)                                                     \
    /* TTI TAI */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TTI_TAI##_tai##_E, NULL,                                     \
        0x00000000, 0x00000004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_MNG_AND_TTI_INST_##_instance##_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,   \
        PRV_CPSS_AC5P_MNG_AND_TTI_INST_##_instance##_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  pca_tai_NODE_MAC(bit, _prefix, _pca, _tai)                                                 \
    /* PCA TAI */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA##_pca##_TAI##_tai##_E, NULL,                             \
        0x00000000, 0x00000004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_PCA_INST_##_prefix##_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,    \
        PRV_CPSS_AC5P_PCA_INST_##_prefix##_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  pca0_tai_NODE_MAC(bit, _tai)                                                               \
        pca_tai_NODE_MAC(bit, 13, 0, _tai)

#define  pca1_tai_NODE_MAC(bit, _tai)                                                               \
        pca_tai_NODE_MAC(bit, 14, 1, _tai)

#define  pca2_tai_NODE_MAC(bit, _tai)                                                               \
        pca_tai_NODE_MAC(bit, 15, 2, _tai)

#define  pca3_tai_NODE_MAC(bit, _tai)                                                               \
    /* PCA TAI */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA3_TAI##_tai##_E, NULL,                                    \
        0x00000000, 0x00000004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_MSEC_INST_16_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,         \
        PRV_CPSS_AC5P_MSEC_INST_16_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   eft_NODE_MAC(bit)                                                                         \
    /* eft  */                                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EGF_EFT_E, NULL,                                             \
        0x000010a0, 0x000010b0,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_EGF_EFT_INTERRUPTS_CAUSE_EGRESS_WRONG_ADDR_INT_E,                             \
        PRV_CPSS_AC5P_EGF_EFT_INTERRUPTS_CAUSE_INC_BUS_IS_TOO_SMALL_INT_E,                          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   eoam_NODE_MAC(bit)                                                                        \
    /* egress OAM  */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EOAM_E, NULL,                                                \
        0x000000f0, 0x000000f4,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_EOAM_OAM_UNIT_INTERRUPT_CAUSE_KEEP_ALIVE_AGING_E,                             \
        PRV_CPSS_AC5P_EOAM_OAM_UNIT_INTERRUPT_CAUSE_EXCESS_KEEPALIVE_E,                             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   epcl_NODE_MAC(bit)                                                                        \
    /* EPCL  */                                                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EPCL_E, NULL,                                            \
        0x00000010, 0x00000014,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_AC5P_EPCL_EPCL_INTERRUPT_CAUSE_REGFILE_ADDRESS_ERROR_E,                        \
        PRV_CPSS_AC5P_EPCL_EPCL_INTERRUPT_CAUSE_TABLE_OVERLAP_ACCESS_ERROR_E,                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eplr_NODE_MAC(bit)                                                                        \
    /* eplr  */                                                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EPLR_E, NULL,                                            \
        0x00000200, 0x00000204,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_AC5P_EPLR_POLICER_INTERRUPT_CAUSE_ILLEGAL_OVERLAPPED_ACCESS_E,                 \
        PRV_CPSS_AC5P_EPLR_POLICER_INTERRUPT_CAUSE_QOS_ATTRIBUTES_TABLE_WRONG_ADDRESS_INTERRUPT_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   ermrk_NODE_MAC(bit)                                                                       \
    /* ERMRK  */                                                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ERMRK_E, NULL,                                           \
        0x00000004, 0x00000008,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_AC5P_ERMRK_ERMRK_INTERRUPT_CAUSE_REGFILE_ADDRESS_ERROR_E,                      \
        PRV_CPSS_AC5P_ERMRK_ERMRK_INTERRUPT_CAUSE_TABLE_OVERLAP_ACCESS_ERROR_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   l2i_NODE_MAC(bit)                                                                         \
    /* L2 Bridge  */                                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_L2I_E,  NULL,                                            \
        0x00002100, 0x00002104,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_AC5P_L2I_BRIDGE_INTERRUPT_CAUSE_ADDRESS_OUT_OF_RANGE_E,                        \
        PRV_CPSS_AC5P_L2I_BRIDGE_INTERRUPT_CAUSE_UPDATE_SECURITY_BREACH_REGISTER_INT_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* node for ingress SCT rate limiters interrupts leaf */
#define SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(index)                                              \
    {(index+1), GT_FALSE, PRV_CPSS_DXCH_UNIT_EQ_E, NULL,                                        \
        (0x00010020 + ((index) * 0x10)), (0x00010024 + ((index) * 0x10)),                       \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_AC5P_EQ_REG_1_PORT_0_PKT_DROPED_INT_E   + ((index) * 32),                      \
        PRV_CPSS_AC5P_EQ_REG_1_PORT_15_PKT_DROPED_INT_E  + ((index) * 32),                      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   eqSctRateLimiters_SUB_TREE_MAC(bit)                                                       \
    /* SCT Rate Limiters */                                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EQ_E, NULL,                                              \
        0x00010000, 0x00010004,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_AC5P_EQ_CPU_CODE_RATE_LIMITERS_INTERRUPT_SUMMARY_CAUSE_REG_1_CPU_CODE_RATE_LIMITER_INT_E,         \
        PRV_CPSS_AC5P_EQ_CPU_CODE_RATE_LIMITERS_INTERRUPT_SUMMARY_CAUSE_REG_16_CPU_CODE_RATE_LIMITER_INT_E,        \
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
        PRV_CPSS_AC5P_EQ_PRE_EGRESS_INTERRUPT_SUMMARY_CAUSE_INGRESS_STC_INT_E,                  \
        PRV_CPSS_AC5P_EQ_PRE_EGRESS_INTERRUPT_SUMMARY_CAUSE_PORT_DLB_INTERRUPT_E,               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                 \
        eqSctRateLimiters_SUB_TREE_MAC(2)


#define  ha_NODE_MAC(bit)                                                           \
    /* HA Rate Limiters */                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_HA_E, NULL,                                  \
        0x00000300, 0x00000304,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5P_HA_HA_INTERRUPT_CAUSE_TABLE_OVERLAP_ACCESS_ERROR_E,           \
        PRV_CPSS_AC5P_HA_HA_INTERRUPT_CAUSE_OVERSIZE_TUNNEL_HEADER_E,               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   ia_NODE_MAC(bit)                                                          \
    /* IA */                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IA_E,  NULL,                                 \
        0x00000500, 0x00000504,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5P_IA_IA_INTERRUPT_0_CAUSE_IA_RF_ERR_E,                          \
        PRV_CPSS_AC5P_IA_IA_INTERRUPT_0_CAUSE_IA_LS_FIFO_READ_ERROR_E,              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   ioam_NODE_MAC(bit)                                                        \
    /* ingress OAM  */                                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IOAM_E,  NULL,                               \
        0x000000F0, 0x000000F4,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5P_IOAM_OAM_UNIT_INTERRUPT_CAUSE_KEEP_ALIVE_AGING_E,             \
        PRV_CPSS_AC5P_IOAM_OAM_UNIT_INTERRUPT_CAUSE_RDI_STATUS_E,                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   iplr0_NODE_MAC(bit)                                                       \
    /* iplr0 */                                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IPLR_E,  NULL,                               \
        0x00000200, 0x00000204,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5P_IPLR0_POLICER_INTERRUPT_CAUSE_ILLEGAL_OVERLAPPED_ACCESS_E,    \
        PRV_CPSS_AC5P_IPLR0_POLICER_INTERRUPT_CAUSE_QOS_ATTRIBUTES_TABLE_WRONG_ADDRESS_INTERRUPT_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   iplr1_NODE_MAC(bit)                                                       \
    /* iplr1 */                                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IPLR_1_E,  NULL,                             \
        0x00000200, 0x00000204,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5P_IPLR1_POLICER_INTERRUPT_CAUSE_ILLEGAL_OVERLAPPED_ACCESS_E,    \
        PRV_CPSS_AC5P_IPLR1_POLICER_INTERRUPT_CAUSE_QOS_ATTRIBUTES_TABLE_WRONG_ADDRESS_INTERRUPT_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   router_NODE_MAC(bit)                                                          \
    /* router */                                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IPVX_E,  NULL,                               \
        0x00000970, 0x00000974,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5P_IPVX_ROUTER_INTERRUPT_CAUSE_I_PV_X_BAD_ADDR_E,                \
        PRV_CPSS_AC5P_IPVX_ROUTER_INTERRUPT_CAUSE_ILLEGAL_OVERLAP_ACCESS_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   lpm_NODE_MAC(bit)                                                             \
    /* LPM */                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_LPM_E,  NULL,                                \
        0x00F00120, 0x00F00130,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5P_LPM_LPM_GENERAL_INT_CAUSE_LPM_WRONG_ADDRESS_E,                \
        PRV_CPSS_AC5P_LPM_LPM_GENERAL_INT_CAUSE_TABLE_OVERLAPING_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   mll_NODE_MAC(bit)                                                             \
    /* MLL */                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MLL_E,  NULL,                                \
        0x00000030, 0x00000034,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5P_MLL_MLL_INTERRUPT_CAUSE_REGISTER_INVALID_IP_MLL_ACCESS_INTERRUPT_E,         \
        PRV_CPSS_AC5P_MLL_MLL_INTERRUPT_CAUSE_REGISTER_ILLEGAL_TABLE_OVERLAP_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   pcl_NODE_MAC(bit)                                                             \
    /* PCL */                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCL_E,  NULL,                                \
        0x00000004, 0x00000008,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5P_PCL_PCL_UNIT_INTERRUPT_CAUSE_MG_ADDR_OUTOF_RANGE_E,           \
        PRV_CPSS_AC5P_PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL_ILLEGAL_TABLE_OVERLAP_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   preq_NODE_MAC(bit)                                                        \
    /* PREQ */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PREQ_E,  NULL,                               \
            0x00000600, 0x00000604,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_AC5P_PREQ_PREQ_INTERRUPT_CAUSE_PREQ_REGFILE_ADDRESS_ERROR_E,   \
            PRV_CPSS_AC5P_PREQ_PREQ_INTERRUPT_CAUSE_PREQ_TABLE_ACCESS_OVERLAP_ERROR_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   ppu_NODE_MAC(bit)                                                             \
    /* PPU */                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PPU_E, NULL,                                 \
        0x00000004, 0x00000008,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5P_PPU_PPU_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,             \
        PRV_CPSS_AC5P_PPU_PPU_INTERRUPT_CAUSE_INT_INDEX_7_PPU_ACTION_INT_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   qag_NODE_MAC(bit)                                                             \
    /* qag */                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EGF_QAG_E,  NULL,                            \
        0x000B0000, 0x000B0010,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5P_EGF_QAG_INTERRUPTS_CAUSE_WRONG_ADDRESS_INTERRUPT_E,           \
        PRV_CPSS_AC5P_EGF_QAG_INTERRUPTS_CAUSE_TABLE_OVERLAPING_INTERRUPT_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   sht_NODE_MAC(bit)                                                         \
    /* SHT */                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,  NULL,                            \
        0x00100010, 0x00100020,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5P_EGF_SHT_INTERRUPTS_CAUSE_WRONG_ADDRESS_INTERRUPT_E,           \
        PRV_CPSS_AC5P_EGF_SHT_INTERRUPTS_CAUSE_TABLE_OVERLAPPING_INTERRUPT_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define PPG_OFFSET(ppg)            (0x80000 * (ppg))
#define PHA_PPN_OFFSET(ppg, ppn)   (PPG_OFFSET(ppg) + 0x4000 * (ppn))

#define PHA_PPN_NODE_MAC(bit, ppg, ppn)                                         \
    /* PPN Internal Error Cause */                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E,  NULL,                            \
       (0x00003040 + PHA_PPN_OFFSET(ppg, ppn)), (0x00003044 + PHA_PPN_OFFSET(ppg, ppn)),\
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_AC5P_PPN_##ppn##_PPG_##ppg##_PPN_INTERNAL_ERROR_CAUSE_HOST_UNMAPPED_ACCESS_E, \
        PRV_CPSS_AC5P_PPN_##ppn##_PPG_##ppg##_PPN_INTERNAL_ERROR_CAUSE_DOORBELL_INTERRUPT_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define phaPpg_SUB_TREE_MAC(bit, ppg)                                           \
    /* PPG Internal Error Cause */                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E,  NULL,                            \
        (0x0007ff00 + PPG_OFFSET(ppg)), (0x0007ff04 + PPG_OFFSET(ppg)),         \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_AC5P_PPG_##ppg##_PPG_INTERRUPT_SUM_CAUSE_PPG_FUNCTIONAL_SUMMARY_INTERRUPT_E, \
        PRV_CPSS_AC5P_PPG_##ppg##_PPG_INTERRUPT_SUM_CAUSE_PPN_9_SUMMARY_INTERRUPT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 11, NULL, NULL},                \
        /*  PPG Internal Error Cause */                                         \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E,  NULL,                          \
            (0x0007ff10 + PPG_OFFSET(ppg)), (0x0007ff14 + PPG_OFFSET(ppg)),     \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_AC5P_PPG_##ppg##_PPG_INTERNAL_ERROR_CAUSE_PPG_BAD_ADDRESS_ERROR_E,          \
            PRV_CPSS_AC5P_PPG_##ppg##_PPG_INTERNAL_ERROR_CAUSE_PPG_TABLE_ACCESS_OVERLAP_ERROR_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},             \
        PHA_PPN_NODE_MAC(2, ppg, 0),                                            \
        PHA_PPN_NODE_MAC(3, ppg, 1),                                            \
        PHA_PPN_NODE_MAC(4, ppg, 2),                                            \
        PHA_PPN_NODE_MAC(5, ppg, 3),                                            \
        PHA_PPN_NODE_MAC(6, ppg, 4),                                            \
        PHA_PPN_NODE_MAC(7, ppg, 5),                                            \
        PHA_PPN_NODE_MAC(8, ppg, 6),                                            \
        PHA_PPN_NODE_MAC(9, ppg, 7),                                            \
        PHA_PPN_NODE_MAC(10,ppg, 8),                                            \
        PHA_PPN_NODE_MAC(11,ppg, 9)

#define   phaPpa_SUB_TREE_MAC(bit)                                              \
    /* PHA PPA */                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E,  NULL,                            \
        0x007eff10, 0x007eff14,                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_AC5P_PPA_PPA_INTERRUPT_SUM_CAUSE_PPA_INTERNAL_FUNCTIONAL_SUMMARY_INTERRUPT_E,    \
        PRV_CPSS_AC5P_PPA_PPA_INTERRUPT_SUM_CAUSE_PPG_3_SUMMARY_INTERRUPT_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                 \
        /* PPA Internal Error Cause */                                          \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E,  NULL,                          \
            0x007eff20, 0x007eff24,                                             \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_AC5P_PPA_PPA_INTERNAL_ERROR_CAUSE_PPA_BAD_ADDRESS_ERRROR_E,\
            PRV_CPSS_AC5P_PPA_PPA_INTERNAL_ERROR_CAUSE_PPA_BAD_ADDRESS_ERRROR_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},             \
        phaPpg_SUB_TREE_MAC(2, 0),                                              \
        phaPpg_SUB_TREE_MAC(3, 1),                                              \
        phaPpg_SUB_TREE_MAC(4, 2),                                              \
        phaPpg_SUB_TREE_MAC(5, 3)

#define   phaInternal_NODE_MAC(bit)                                             \
    /* PHA Internal Error Cause */                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E,  NULL,                            \
        0x007fff80, 0x007fff84,                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_AC5P_PHA_PHA_INTERNAL_ERROR_CAUSE_PHA_BAD_ADDRESS_ERROR_E,     \
        PRV_CPSS_AC5P_PHA_PHA_INTERNAL_ERROR_CAUSE_PPA_CLOCK_DOWN_VIOLATION_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   pha_SUB_TREE_MAC(bit)                                                                     \
    /* PHA */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E,  NULL,                                                \
        0x007fff88, 0x007fff8c,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_PHA_PHA_INTERRUPT_SUM_CAUSE_PHA_INTERNAL_FUNCTIONAL_SUMMARY_INTERRUPT_E,      \
        PRV_CPSS_AC5P_PHA_PHA_INTERRUPT_SUM_CAUSE_PPA_SUMMARY_INTERRUPT_E,                          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                     \
        phaInternal_NODE_MAC(1),                                                                    \
        phaPpa_SUB_TREE_MAC(2)

#define   hbu_SUB_TREE_MAC(bit)                                                                          \
        /* HBU */                                                                                        \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_HBU_E, NULL,                                                  \
            0x00000000, 0x00000004,                                                                      \
            prvCpssDrvHwPpPortGroupIsrRead,                                                              \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
            PRV_CPSS_AC5P_HBU_HBU_INTERRUPT_CAUSE_REGISTER_ERROR_FHF_MEM_E,                              \
            PRV_CPSS_AC5P_HBU_HBU_INTERRUPT_CAUSE_REGISTER_HEADER_FIFO_EMPTY_E,                          \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   tti_NODE_MAC(bit)                                                                         \
    /* TTI */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TTI_E,  NULL,                                                \
        0x00000004, 0x00000008,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_TTI_TTI_ENGINE_INTERRUPT_CAUSE_CPU_ADDRESS_OUT_OF_RANGE_E,                    \
        PRV_CPSS_AC5P_TTI_TTI_ENGINE_INTERRUPT_CAUSE_TABLE_OVERLAP_ERROR_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txDma_NODE_MAC(bit, _index)                                                               \
    /* TXD */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##_index##_E,  NULL,                                    \
        0x00006000, 0x00006004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_TXD_INST_##_index##_TXD_INTERRUPT_CAUSE_TXD_RF_ERR_E,                         \
        PRV_CPSS_AC5P_TXD_INST_##_index##_TXD_INTERRUPT_CAUSE_SDQ_BYTES_UNDERFLOW_E,                \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txFifo_NODE_MAC(bit, _index)                                                              \
    /* TXF */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##_index##_E,  NULL,                                  \
        0x00004000, 0x00004004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_TXF_INST_##_index##_TXF_INTERRUPT_CAUSE_TX_FIFO_RF_ERR_E,                     \
        PRV_CPSS_AC5P_TXF_INST_##_index##_TXF_INTERRUPT_CAUSE_DESCRIPTOR_RD_SER_ERROR_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   rxDma_int0_SUB_TREE_MAC(bit, _index, _int)                                                      \
        /* RXD INT0 */                                                                                    \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##_index##_E, NULL,                                       \
            0x00001C80, 0x00001C84,                                                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                                               \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                              \
            PRV_CPSS_AC5P_RXDMA_INST_##_index##_RX_DMA_INTERRUPT_##_int##_CAUSE_RX_DMA_RF_ERR_E,          \
            PRV_CPSS_AC5P_RXDMA_INST_##_index##_RX_DMA_INTERRUPT_##_int##_CAUSE_HIGH_PRIO_DESC_DROP_E,    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   rxDma_int1_SUB_TREE_MAC(bit, _index, _int)                                                         \
        /* RXD INT1 */                                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##_index##_E, NULL,                                          \
            0x00001C88, 0x00001C8C,                                                                          \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
            PRV_CPSS_AC5P_RXDMA_INST_##_index##_RX_DMA_INTERRUPT_##_int##_CAUSE_DESC_ENQUEUE_FIFO_FULL_INT_E,\
            PRV_CPSS_AC5P_RXDMA_INST_##_index##_RX_DMA_INTERRUPT_##_int##_CAUSE_DESC_ENQUEUE_FIFO_FULL_INT_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   rxDma_int2_SUB_TREE_MAC(bit, _index, _int)                                                               \
        /* RXD INT2 */                                                                                             \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##_index##_E, NULL,                                                \
            0x00001C90, 0x00001C94,                                                                                \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                        \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                       \
            PRV_CPSS_AC5P_RXDMA_INST_##_index##_RX_DMA_INTERRUPT_##_int##_CAUSE_PB_TAIL_ID_MEM_SER_INT_E,          \
            PRV_CPSS_AC5P_RXDMA_INST_##_index##_RX_DMA_INTERRUPT_##_int##_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   rxDma_NODE_MAC(bit, _index)                                                               \
    /* RXD */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##_index##_E,  NULL,                                    \
        0x00001CA0, 0x00001CA4,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_RXDMA_INST_##_index##_RX_DMA_INTERRUPT_SUMMARY_CAUSE_RX_DMA_INTERRUPT_0_INT_E,\
        PRV_CPSS_AC5P_RXDMA_INST_##_index##_RX_DMA_INTERRUPT_SUMMARY_CAUSE_RX_DMA_INTERRUPT_2_INT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                     \
        rxDma_int0_SUB_TREE_MAC(1, _index, 0),                                                      \
        rxDma_int1_SUB_TREE_MAC(2, _index, 1),                                                      \
        rxDma_int2_SUB_TREE_MAC(3, _index, 2)

#define tai_CPU_PORT_NODE_MAC(bit, _tai)                                      \
    /* CPU TAI */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_CPU_PORT_TAI##_tai##_E, NULL,                                \
        0x00000000, 0x00000004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_CPU_AND_400G_MAC_INST_10_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,             \
        PRV_CPSS_AC5P_CPU_AND_400G_MAC_INST_10_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define npm_SUB_TREE_MAC(bit, _index)                                                                       \
    /* NPM-0  */                                                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL,                             \
        0x00000100, 0x00000104,                                                                             \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                     \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                    \
        PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_SUMMARY_CAUSE_INT_SUMMARY_CAUSE_GENERIC_E,           \
        PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_SUMMARY_CAUSE_INT_SUMMARY_CAUSE_LIST_ERR_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 10, NULL, NULL},                                            \
                                                                                                            \
        /* GenericInterruptSum  */                                                                          \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL, \
            0x00000108, 0x0000010C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_GENERIC_CAUSE_INT_GENERIC_CAUSE_ADDR_ERR_E,      \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_GENERIC_CAUSE_INT_GENERIC_CAUSE_RAND_ICRD_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* UintErrorInterruptSum  */                                                                        \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL, \
            0x00000110, 0x00000114,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_0_E,       \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_4_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* ReinsertInterruptSum  */                                                                         \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL, \
            0x00000118, 0x0000011C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_REINSERT_CAUSE_INT_REINSERT_CAUSE_0_0_E,         \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_REINSERT_CAUSE_INT_REINSERT_CAUSE_3_4_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* ReinsertfailInterruptSum  */                                                                     \
        {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL, \
            0x00000120, 0x00000124,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_0_E,    \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_4_E,    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* UnitRefsInterruptSum  */                                                                         \
        {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL, \
            0x00000128, 0x0000012C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_0_E,       \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_4_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* UnitDataInterruptSum  */                                                                         \
        {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL, \
            0x00000130, 0x00000134,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_0_E,       \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_4_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* RefsErrorInterruptSum  */                                                                        \
        {7, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL, \
            0x00000138, 0x0000013C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_0_E,       \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_3_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* AgingErrorInterruptSum  */                                                                       \
        {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL, \
            0x00000140, 0x00000144,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_0_E,     \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_3_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* RefsFifoInterruptSum  */                                                                         \
        {9, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL, \
            0x00000148, 0x0000014C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_0_E,       \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_3_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* ListErrorInterruptSum  */                                                                        \
        {10, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_index##_E, NULL,\
            0x00000150, 0x00000154,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_0_E,         \
            PRV_CPSS_AC5P_PB_NPM_INST_##_index##_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_3_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define smb_SUB_TREE_MAC(bit, _index)                                                                                  \
    /* SMB-MC */                                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E, NULL,   \
        0x00000150, 0x00000154,                                                                                        \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                               \
        PRV_CPSS_AC5P_PB_SMB_INST_##_index##_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                                 \
        PRV_CPSS_AC5P_PB_SMB_INST_##_index##_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_FIFO_E,                                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6,NULL, NULL},                                                         \
                                                                                                                       \
        /* MiscInterruptSum  */                                                                                        \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E, NULL, \
            0x00000100, 0x00000104,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_AC5P_PB_SMB_INST_##_index##_INTERRUPT_MISC_CAUSE_INTERRUPT_RPW_NO_TRUST_E,                        \
            PRV_CPSS_AC5P_PB_SMB_INST_##_index##_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_CONGESTION_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                     \
        /* AgeInterruptSum  */                                                                                         \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E, NULL, \
            0x00000110, 0x00000114,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_AC5P_PB_SMB_INST_##_index##_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_AGE_ERR_E,             \
            PRV_CPSS_AC5P_PB_SMB_INST_##_index##_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_AGE_ERR_E,            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                     \
        /* RbwInterruptSum  */                                                                                         \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E, NULL, \
            0x00000120, 0x00000124,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_AC5P_PB_SMB_INST_##_index##_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_RBW_ERR_E,             \
            PRV_CPSS_AC5P_PB_SMB_INST_##_index##_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_RBW_ERR_E,            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                     \
        /* MemoriesInterruptSum  */                                                                                    \
        {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E, NULL, \
            0x00000130, 0x00000134,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_AC5P_PB_SMB_INST_##_index##_INTERRUPT_MEMORIES_CAUSE_LOGICAL_MEMORY_0_PAYLOAD_SER_ERROR_E,        \
            PRV_CPSS_AC5P_PB_SMB_INST_##_index##_INTERRUPT_MEMORIES_CAUSE_LOGICAL_MEMORY_15_PAYLOAD_SER_ERROR_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                     \
        /* Memories2InterruptSum  */                                                                                   \
        {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E, NULL, \
            0x00000140, 0x00000144,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_AC5P_PB_SMB_INST_##_index##_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_0_REFCNT_SER_ERROR_E,        \
            PRV_CPSS_AC5P_PB_SMB_INST_##_index##_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_15_REFCNT_SER_ERROR_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                     \
        /* FifoInterruptSum  */                                                                                        \
        {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E, NULL, \
            0x00000118, 0x0000011C,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_AC5P_PB_SMB_INST_##_index##_INTERRUPT_FIFO_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_FIFO_ERR_E,           \
            PRV_CPSS_AC5P_PB_SMB_INST_##_index##_INTERRUPT_FIFO_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_FIFO_ERR_E,          \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define gpc_pw_SUB_TREE_MAC(bit, _unit)                                                                                      \
{bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_##_unit##_E, NULL,                                                \
    0x00000200, 0x0000204,                                                                                                   \
    prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
    prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
    PRV_CPSS_AC5P_PACKET_WRITE_INST_##_unit##_INTERRUPT_CAUSE_INT_CAUSE_RDY_NFL_E,                                           \
    PRV_CPSS_AC5P_PACKET_WRITE_INST_##_unit##_INTERRUPT_CAUSE_INT_CAUSE_ADDR_ERR_E,                                          \
    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gpc_pr_chan_SUB_TREE_MAC(bit, _unit, _chan)                                                                          \
{bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_unit##_E, NULL,                                                 \
    0x00001300 + (_chan)*4, 0x00001400 + (_chan)*4,                                                                          \
    prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
    prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
    PRV_CPSS_AC5P_PB_GPR_INST_##_unit##_CHANNEL_INDEX_##_chan##_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E,                   \
    PRV_CPSS_AC5P_PB_GPR_INST_##_unit##_CHANNEL_INDEX_##_chan##_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E,                            \
    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gpc_pr_SUB_TREE_MAC(bit, _unit)                                                                                      \
{bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_unit##_E, NULL,                                                 \
    0x00001530, 0x00001534,                                                                                                  \
    prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
    prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
    PRV_CPSS_AC5P_PB_GPR_INST_##_unit##_INTERRUPT_SUMMARY_CAUSE_INTERRUPT_CHANNELS_SUMMARY_0_E,                              \
    PRV_CPSS_AC5P_PB_GPR_INST_##_unit##_INTERRUPT_SUMMARY_CAUSE_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_SUMMARY_E,             \
    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                                                  \
    {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_unit##_E, NULL,                                                   \
        0x00001520, 0x0001528,                                                                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
        PRV_CPSS_AC5P_PB_GPR_INST_##_unit##_INTERRUPT_CHANNELS_SUMMARY_0_CAUSE_CHANNEL_INDEX_0_INTERRUPT_CHANNEL_SUMMARY_E,      \
        PRV_CPSS_AC5P_PB_GPR_INST_##_unit##_INTERRUPT_CHANNELS_SUMMARY_0_CAUSE_CHANNEL_INDEX_27_INTERRUPT_CHANNEL_SUMMARY_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 28, NULL, NULL},                                                                        \
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
    {11, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_unit##_E, NULL,                                                  \
        0x00001500, 0x0001504,                                                                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
        PRV_CPSS_AC5P_PB_GPR_INST_##_unit##_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E,         \
        PRV_CPSS_AC5P_PB_GPR_INST_##_unit##_INTERRUPT_PACKET_COUNT_CAUSE_NULL_ADDRESS_RECEIVED_PACKET_COUNT_CELL_E,              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                                  \
    {12, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_unit##_E, NULL,                                                  \
        0x00001508, 0x000150C,                                                                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
        PRV_CPSS_AC5P_PB_GPR_INST_##_unit##_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E,                       \
        PRV_CPSS_AC5P_PB_GPR_INST_##_unit##_INTERRUPT_MISC_CAUSE_NULL_ADDRESS_RECEIVED_PACKET_READ_CELL_E,                       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                                  \
    {13, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_unit##_E, NULL,                                                  \
        0x00001510, 0x0001514,                                                                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
        PRV_CPSS_AC5P_PB_GPR_INST_##_unit##_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E,\
        PRV_CPSS_AC5P_PB_GPR_INST_##_unit##_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                                  \
    {14, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_unit##_E, NULL,                                                  \
        0x00001518, 0x000151C,                                                                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
        PRV_CPSS_AC5P_PB_GPR_INST_##_unit##_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E,    \
        PRV_CPSS_AC5P_PB_GPR_INST_##_unit##_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gpc_cr_SUB_TREE_MAC(bit, _unit)                                                                                      \
{bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_##_unit##_E, NULL,                                                   \
    0x000000A0, 0x00000A4,                                                                                                   \
    prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
    prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
    PRV_CPSS_AC5P_GPC_CELL_READ_INST_##_unit##_INTERRUPT_CAUSE_INTERFACE_0_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_E,                    \
    PRV_CPSS_AC5P_GPC_CELL_READ_INST_##_unit##_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                                                    \
    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define packetBuffer_SUB_TREE_MAC(bit)                                                                  \
    /* Packet Buffer */                                                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E,  NULL,                                          \
        0x00001128, 0x0000112c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PB_CENTER_INTERRUPT_SUMMARY_PACKET_BUFFER_CAUSE_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTERS_E, \
        PRV_CPSS_AC5P_PB_CENTER_INTERRUPT_SUMMARY_PACKET_BUFFER_CAUSE_INTERRUPT_SUMMARY_PB_CENTER_E,           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6, NULL, NULL},                                         \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E,  NULL,                                        \
            0x00001108, 0x0000110C,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5P_PB_CENTER_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTER_E, \
            PRV_CPSS_AC5P_PB_CENTER_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTER_CAUSE_INSTANCE_1_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTER_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                     \
        npm_SUB_TREE_MAC(1, 0),                                                                         \
        npm_SUB_TREE_MAC(2, 1),                                                                         \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E,  NULL,                                        \
            0x00001110, 0x00001114,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5P_PB_CENTER_INTERRUPT_SUMMARY_GPC_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_PACKET_WRITE_GPC_E,  \
            PRV_CPSS_AC5P_PB_CENTER_INTERRUPT_SUMMARY_GPC_CAUSE_INSTANCE_3_INTERRUPT_SUMMARY_CELL_READ_GPC_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 12, NULL, NULL},                                       \
        gpc_pw_SUB_TREE_MAC(1, 0),                                                                         \
        gpc_pw_SUB_TREE_MAC(2, 1),                                                                         \
        gpc_pw_SUB_TREE_MAC(3, 2),                                                                         \
        gpc_pw_SUB_TREE_MAC(4, 3),                                                                         \
        gpc_pr_SUB_TREE_MAC(5, 0),                                                                         \
        gpc_pr_SUB_TREE_MAC(6, 1),                                                                         \
        gpc_pr_SUB_TREE_MAC(7, 2),                                                                         \
        gpc_pr_SUB_TREE_MAC(8, 3),                                                                         \
        gpc_cr_SUB_TREE_MAC(9, 0),                                                                         \
        gpc_cr_SUB_TREE_MAC(10, 1),                                                                        \
        gpc_cr_SUB_TREE_MAC(11, 2),                                                                        \
        gpc_cr_SUB_TREE_MAC(12, 3),                                                                        \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E,  NULL,                                        \
            0x00001118, 0x0000111C,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5P_PB_CENTER_INTERRUPT_SUMMARY_SMB_MEMORY_CLUSTER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_SMB_MEMORY_CLUSTER_E,  \
            PRV_CPSS_AC5P_PB_CENTER_INTERRUPT_SUMMARY_SMB_MEMORY_CLUSTER_CAUSE_INSTANCE_2_INTERRUPT_SUMMARY_SMB_MEMORY_CLUSTER_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                     \
            smb_SUB_TREE_MAC(1, 0),                                                                     \
            smb_SUB_TREE_MAC(2, 1),                                                                     \
            smb_SUB_TREE_MAC(3, 2),                                                                     \
        {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E,  NULL,                                        \
            0x00001120, 0x00001124,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5P_PB_CENTER_INTERRUPT_SUMMARY_SMB_WRITE_ARBITER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_SMB_WRITE_ARBITER_E,                          \
            PRV_CPSS_AC5P_PB_CENTER_INTERRUPT_SUMMARY_SMB_WRITE_ARBITER_CAUSE_INSTANCE_3_INTERRUPT_SUMMARY_SMB_WRITE_ARBITER_E,                          \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 4, NULL, NULL},                                     \
            {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E, NULL,                                                               \
                0x00000110, 0x0000114,                                                                                                   \
                prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
                PRV_CPSS_AC5P_WA_INST_0_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                                                        \
                PRV_CPSS_AC5P_WA_INST_0_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                                                        \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                               \
            {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_1_E, NULL,                                                               \
                0x00000110, 0x0000114,                                                                                                   \
                prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
                PRV_CPSS_AC5P_WA_INST_1_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                                                        \
                PRV_CPSS_AC5P_WA_INST_1_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                                                        \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                               \
            {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_2_E, NULL,                                                               \
                0x00000110, 0x0000114,                                                                                                   \
                prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
                PRV_CPSS_AC5P_WA_INST_2_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                                                        \
                PRV_CPSS_AC5P_WA_INST_2_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                                                        \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                               \
            {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_3_E, NULL,                                                               \
                0x00000110, 0x0000114,                                                                                                   \
                prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
                PRV_CPSS_AC5P_WA_INST_3_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                                                        \
                PRV_CPSS_AC5P_WA_INST_3_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                                                        \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                               \
        {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_COUNTER_E, NULL,                                          \
            0x00000190, 0x00000194,                                                                   \
            prvCpssDrvHwPpPortGroupIsrRead,                                                           \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                          \
            PRV_CPSS_AC5P_PB_COUNTER_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                        \
            PRV_CPSS_AC5P_PB_COUNTER_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                   \
        {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E,  NULL,                                      \
          0x00001100, 0x00001104,                                                                     \
          prvCpssDrvHwPpPortGroupIsrRead,                                                             \
          prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
          PRV_CPSS_AC5P_PB_CENTER_INTERRUPT_PB_CENTER_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                 \
          PRV_CPSS_AC5P_PB_CENTER_INTERRUPT_PB_CENTER_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                 \
          FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   erep_NODE_MAC(bit)                                                                            \
    /* EREP Rate Limiters */                                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EREP_E,  NULL,                                                   \
        0x00003000, 0x00003004,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_EREP_EREP_INTERRUPTS_CAUSE_EREP_CPU_ADDRESS_OUT_OF_RANGE_E,                       \
        PRV_CPSS_AC5P_EREP_EREP_INTERRUPTS_CAUSE_MIRROR_REPLICATION_NOT_PERFORMED_E,                    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   bma_NODE_MAC(bit)                                                     \
        /* bma  */                                                                  \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_BMA_E, NULL,                             \
            0x00059000, 0x00059004,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_AC5P_BMA_MC_CNT_RMW_CLEAR_FIFO_FULL_E,                         \
            PRV_CPSS_AC5P_BMA_FINAL_CLEAR_UDB_FIFO0_FULL_E,                         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define   cnc_SUB_TREE_MAC(bit, instance)                                                               \
    /* CNC  */                                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_CNC_##instance##_E,  NULL,                                       \
        0x00000100, 0x00000104,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_CNC_INST_##instance##_CNC_INTERRUPT_SUMMARY_CAUSE_REGISTER_WRAPAROUND_FUNC_INTERRUPT_SUM_E,         \
        PRV_CPSS_AC5P_CNC_INST_##instance##_CNC_INTERRUPT_SUMMARY_CAUSE_REGISTER_MISC_FUNC_INTERRUPT_SUM_E,               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3,NULL, NULL},                                          \
                                                                                                        \
        /* WraparoundFuncInterruptSum  */                                                               \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_CNC_##instance##_E,  NULL,                                     \
            0x00000190, 0x000001a4,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5P_CNC_INST_##instance##_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_0_WRAPAROUND_E,   \
            PRV_CPSS_AC5P_CNC_INST_##instance##_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_15_WRAPAROUND_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                      \
        /* RateLimitFuncInterruptSum  */                                                                \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_CNC_##instance##_E,  NULL,                                     \
            0x000001b8, 0x000001cc,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5P_CNC_INST_##instance##_RATE_LIMIT_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_NUMBER_0_RATE_LIMIT_FIFO_DROP_E,  \
            PRV_CPSS_AC5P_CNC_INST_##instance##_RATE_LIMIT_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_NUMBER_15_RATE_LIMIT_FIFO_DROP_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                      \
        /* MiscFuncInterruptSum  */                                                                     \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_CNC_##instance##_E,  NULL,                                     \
            0x000001e0, 0x000001e4,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5P_CNC_INST_##instance##_MISC_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_DUMP_FINISHED_E,           \
            PRV_CPSS_AC5P_CNC_INST_##instance##_MISC_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_CNC_TABLE_OVERLAPPING_E,   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   epcl_tai_NODE_MAC(bit, _tai)                                              \
    /* EPCL TAI  */                                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EPCL_HA_TAI##_tai##_E, NULL,                 \
        0x00000000, 0x00000004,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5P_PHA_AND_EPCL_HA_INST_4_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,   \
        PRV_CPSS_AC5P_PHA_AND_EPCL_HA_INST_4_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define   pha_tai_NODE_MAC(bit, _tai)                                               \
    /* PHA TAI  */                                                                  \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_TAI##_tai##_E, NULL,                     \
        0x00000000, 0x00000004,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5P_PHA_AND_EPCL_HA_INST_5_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,   \
        PRV_CPSS_AC5P_PHA_AND_EPCL_HA_INST_5_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define pzarb_MSEC_RPZ_INST_NODE_MAC(bit, _instance, _index)                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_##_instance##_E, NULL,                          \
        0x00000B04, 0x00000B08,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_RPZ_INST_##_index##_DUMMY_UNIT_0_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,     \
        PRV_CPSS_AC5P_MSEC_RPZ_INST_##_index##_DUMMY_UNIT_0_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pzarb_MSEC_TPZ_INST_NODE_MAC(bit, _instance, _index)                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_##_instance##_E, NULL,                          \
        0x00000b04, 0x00000b08,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_TPZ_INST_##_index##_DUMMY_UNIT_0_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,     \
        PRV_CPSS_AC5P_MSEC_TPZ_INST_##_index##_DUMMY_UNIT_0_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pzarb_PCA2I_MACRO_INST_NODE_MAC(bit, _instance)                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_##_instance##_E, NULL,                          \
        0x00000B04, 0x00000B08,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_MACRO_INST_2_T_AND_R_PZ_UNIT_INST_##_instance##_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,     \
        PRV_CPSS_AC5P_PCA_MACRO_INST_2_T_AND_R_PZ_UNIT_INST_##_instance##_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pzarb_PCA2E_MACRO_INST_NODE_MAC(bit, _instance)                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_##_instance##_E, NULL,                          \
        0x00000B04, 0x00000B08,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_MACRO_INST_2_T_AND_R_PZ_UNIT_INST_##_instance##_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,     \
        PRV_CPSS_AC5P_PCA_MACRO_INST_2_T_AND_R_PZ_UNIT_INST_##_instance##_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pzarb_PCA3I_MACRO_INST_NODE_MAC(bit, _instance)                                                  \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_##_instance##_E, NULL,                          \
        0x00000B04, 0x00000B08,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_MACRO_INST_3_T_AND_R_PZ_UNIT_INST_##_instance##_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,     \
        PRV_CPSS_AC5P_PCA_MACRO_INST_3_T_AND_R_PZ_UNIT_INST_##_instance##_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pzarb_PCA3E_MACRO_INST_NODE_MAC(bit, _instance)                                                  \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_##_instance##_E, NULL,                          \
        0x00000B04, 0x00000B08,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_MACRO_INST_3_T_AND_R_PZ_UNIT_INST_##_instance##_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,     \
        PRV_CPSS_AC5P_PCA_MACRO_INST_3_T_AND_R_PZ_UNIT_INST_##_instance##_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define dfx_server_SUB_TREE_MAC(bit)                                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_DFX_SERVER_E, NULL,                                           \
        0x000F8108, 0x000F810C,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5P_DFX_SERVER_INTERRUPT_WRONG_ADDRESS_FROM_PIPE_0_E,                              \
        PRV_CPSS_AC5P_DFX_SERVER_INTERRUPT_EXTERNAL_LOW_TEMPERATURE_THRESHOLD_E,                     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define dfx_server_sum_SUB_TREE_MAC(bit)                                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_DFX_SERVER_E, NULL,                                           \
        0x000F8100, 0x000F8104,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5P_DFX_SERVER_INTERRUPT_SUMMARY_SERVER_INTERRUPT_SUM_E,                           \
        PRV_CPSS_AC5P_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_7_INTERRUPT_SUM_E,                           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                      \
        dfx_server_SUB_TREE_MAC(1)

#define pds_SUB_TREE_MAC(bit, _unit, _index, _pds)                                      \
    /* PDS Interrupt Summary */                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS##_pds##_E, NULL,                   \
        0x00042000, 0x00042004,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_PDS_INTERRUPT_SUMMARY_CAUSE_PDS_FUNCTIONAL_INT_SUM_E, \
        PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_PDS_INTERRUPT_SUMMARY_CAUSE_PDS_DEBUG_INT_SUM_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                                     \
        /* PDS Functional Interrupt Summary */                                                                      \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS##_pds##_E, NULL,                                             \
            0x00042008, 0x0004200C,                                                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                        \
            PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PDS_BAD_ADDRESS_INT_E,    \
            PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                 \
        /* PDS Debug Interrupt Summary */                                                                           \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS##_pds##_E, NULL,                                             \
            0x00042010, 0x00042014,                                                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                        \
            PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_E, \
            PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_PDS_INTERRUPT_DEBUG_CAUSE_PB_RD_NOT_PERFORMED_INT_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pds0_SUB_TREE_MAC(bit)                                                          \
        pds_SUB_TREE_MAC(bit, 0, 0, 0)

#define pds1_SUB_TREE_MAC(bit)                                                          \
        pds_SUB_TREE_MAC(bit, 0, 1, 1)

#define pds2_SUB_TREE_MAC(bit)                                                          \
        pds_SUB_TREE_MAC(bit, 1, 0, 2)

#define pds3_SUB_TREE_MAC(bit)                                                          \
        pds_SUB_TREE_MAC(bit, 1, 1, 3)

#define qfc_FUNC_INT_NODE_MAC(bit, _unit, _index, _qfc)                                 \
    /* QFC Interrupt Summary */                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC##_qfc##_E, NULL,                   \
        0x00000200, 0x00000204,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E, \
        PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define qfc_DBG_INT_NODE_MAC(bit, _unit, _index, _qfc)                                                              \
    /* QFC Debug Interrupt Summary */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC##_qfc##_E, NULL,                                               \
        0x00000208, 0x0000020C,                                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
        PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_QFC_INTERRUPT_DEBUG_CAUSE_PFC_MESSAGES_DROP_COUNTER_WRAPAROUND_INT_E,                 \
        PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_QFC_INTERRUPT_DEBUG_CAUSE_AGGREGATION_BUFFER_COUNT_OVERFLOW_E,                        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define QFC_HR_INT_OFFSET(_unit, _index, _regIndex)                                                                 \
        (((_unit) * 2 * 9 * 32) + ((_index) * 9 * 32) + ((_regIndex) * 32))

#define qfc_HR_INT_NODE_MAC(bit, _unit, _index, _qfc, _regIndex)                                                    \
    /* QFC HR Crossed Threshold Interrupt Summary */                                                                \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC##_qfc##_E, NULL,                                               \
        (0x0000021C+(_regIndex*0x8)), (0x00000220+(_regIndex*0x8)),                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
        PRV_CPSS_AC5P_TXQS_INST_0_UNIT_INST_0_REGISTER_INDEX_0_PORT_0_TC_0_CROSSED_HR_THRESHOLD_E + QFC_HR_INT_OFFSET(_unit, _index, _regIndex),   \
        PRV_CPSS_AC5P_TXQS_INST_0_UNIT_INST_0_REGISTER_INDEX_0_PORT_2_TC_7_CROSSED_HR_THRESHOLD_E + QFC_HR_INT_OFFSET(_unit, _index, _regIndex),   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define qfc_SUB_TREE_MAC(bit, _unit, _index, _qfc)                                      \
    /* QFC Interrupt Summary */                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC##_qfc##_E, NULL,                   \
        0x00000210, 0x00000214,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_QFC_INTERRUPT_SUMMARY_CAUSE_QFC_FUNCTIONAL_INT_SUM_E,    \
        PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_QFC_INTERRUPT_SUMMARY_CAUSE_REGISTER_INDEX_8_INT_SUM_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 11, NULL, NULL},                        \
        qfc_FUNC_INT_NODE_MAC( 1, _unit, _index, _qfc),                                 \
        qfc_DBG_INT_NODE_MAC ( 2, _unit, _index, _qfc),                                 \
        qfc_HR_INT_NODE_MAC  ( 3, _unit, _index, _qfc,  0),                             \
        qfc_HR_INT_NODE_MAC  ( 4, _unit, _index, _qfc,  1),                             \
        qfc_HR_INT_NODE_MAC  ( 5, _unit, _index, _qfc,  2),                             \
        qfc_HR_INT_NODE_MAC  ( 6, _unit, _index, _qfc,  3),                             \
        qfc_HR_INT_NODE_MAC  ( 7, _unit, _index, _qfc,  4),                             \
        qfc_HR_INT_NODE_MAC  ( 8, _unit, _index, _qfc,  5),                             \
        qfc_HR_INT_NODE_MAC  ( 9, _unit, _index, _qfc,  6),                             \
        qfc_HR_INT_NODE_MAC  (10, _unit, _index, _qfc,  7),                             \
        qfc_HR_INT_NODE_MAC  (11, _unit, _index, _qfc,  8)

#define qfc0_SUB_TREE_MAC(bit)                                                          \
        qfc_SUB_TREE_MAC(bit, 0, 0, 0)

#define qfc1_SUB_TREE_MAC(bit)                                                          \
        qfc_SUB_TREE_MAC(bit, 0, 1, 1)

#define qfc2_SUB_TREE_MAC(bit)                                                          \
        qfc_SUB_TREE_MAC(bit, 1, 0, 2)

#define qfc3_SUB_TREE_MAC(bit)                                                          \
        qfc_SUB_TREE_MAC(bit, 1, 1, 3)


#define sdq_SUB_TREE_MAC(bit, _unit, _index, _sdq)                                                  \
    /* SDQ Interrupt Summary */                                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ##_sdq##_E, NULL,                               \
        0x00000200, 0x00000204,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_SDQ_INTERRUPT_SUMMARY_CAUSE_SDQ_FUNCTIONAL_INTERRUPT_SUMMARY_E, \
        PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_SDQ_INTERRUPT_SUMMARY_CAUSE_SDQ_DEBUG_INTERRUPT_SUMMARY_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                                    \
        /* SDQ Functional Interrupt Summary */                                                                     \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ##_sdq##_E, NULL,                                            \
            0x00000208, 0x0000020C,                                                                                \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                        \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                       \
            PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_ILLEGAL_ADDRESS_ACCESS_E, \
            PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_LIST_PTRS_CONFIG_PORT_NOT_EMPTY_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                \
        /* SDQ Debug Interrupt Summary */                                                                          \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ##_sdq##_E, NULL,                                            \
            0x00000210, 0x00000214,                                                                                \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                        \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                       \
            PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_SDQ_INTERRUPT_DEBUG_CAUSE_QCN_FIFO_OVERRUN_E,   \
            PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_SDQ_INTERRUPT_DEBUG_CAUSE_QCN_FIFO_OVERRUN_E,   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define sdq0_SUB_TREE_MAC(bit)                                                                      \
        sdq_SUB_TREE_MAC(bit, 0, 0, 0)

#define sdq1_SUB_TREE_MAC(bit)                                                                      \
        sdq_SUB_TREE_MAC(bit, 0, 1, 1)

#define sdq2_SUB_TREE_MAC(bit)                                                                      \
        sdq_SUB_TREE_MAC(bit, 1, 0, 2)

#define sdq3_SUB_TREE_MAC(bit)                                                                      \
        sdq_SUB_TREE_MAC(bit, 1, 1, 3)

#define tai_CPU_NODE_MAC(bit, _instance)                                                            \
    /* CPU TAI */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_CPU_PORT_TAI##_instance__E, NULL,                            \
        0x00000000, 0x00000004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_CPU_AND_400G_MAC_INST_##_unit##_UNIT_INST_##_instance##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,      \
        PRV_CPSS_AC5P_CPU_AND_400G_MAC_INST_##_unit##_UNIT_INST_##_instance##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define tai_400G_NODE_MAC(bit, _unit, _instance)                                                    \
    /* 400G TAI */                                                                                  \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_400G##_unit##_TAI##_instance##_E, NULL,                      \
        0x00000000, 0x00000004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_CPU_AND_400G_MAC_INST_##_unit##_UNIT_INST_##_instance##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,      \
        PRV_CPSS_AC5P_CPU_AND_400G_MAC_INST_##_unit##_UNIT_INST_##_instance##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define tai_USX_NODE_MAC(bit, _unit, _instance)                                                     \
    /* USX TAI */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_USX##_unit##_TAI##_instance##_E, NULL,                       \
        0x00000000, 0x00000004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_USX_MAC_MACRO_INST_##_unit##_UNIT_INST_##_instance##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,      \
        PRV_CPSS_AC5P_USX_MAC_MACRO_INST_##_unit##_UNIT_INST_##_instance##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  tai_master_SUB_TREE_MAC(bit, _tai)                                                          \
    /* TAI master */                                                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TAI##_tai##_E, NULL,                                          \
        0x00000000, 0x00000004,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5P_TAI_MASTER_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,            \
        PRV_CPSS_AC5P_TAI_MASTER_UNIT_INST_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define led_CPU_NODE_MAC(bit)                                                              \
    /* CPU LED */                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_LED_CPU_0_E, NULL,                                  \
        0x00000200, 0x00000204,                                                            \
        prvCpssDrvHwPpPortGroupIsrRead,                                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                   \
        PRV_CPSS_AC5P_CPU_PORT_0_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,             \
        PRV_CPSS_AC5P_CPU_PORT_0_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define led_400G_NODE_MAC(bit, _instance)                                                  \
    /* 400G LED */                                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_LED_0_E + _instance, NULL,                          \
        0x00000200, 0x00000204,                                                            \
        prvCpssDrvHwPpPortGroupIsrRead,                                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                   \
        PRV_CPSS_AC5P_400G_MAC_INST_1_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E + (32 * _instance),             \
        PRV_CPSS_AC5P_400G_MAC_INST_1_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E + (32 * _instance),             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define led_USX_NODE_MAC(bit, _instance)                                                   \
    /* USX LED */                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_LED_4_E + _instance, NULL,                          \
        0x00000200, 0x00000204,                                                            \
        prvCpssDrvHwPpPortGroupIsrRead,                                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                   \
        PRV_CPSS_AC5P_USX_MAC_INST_5_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E + (32 * _instance),             \
        PRV_CPSS_AC5P_USX_MAC_INST_5_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E + (32 * _instance),             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define mac_pcs_400G_NODE_MAC(bit, _instance, _port)                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MAC_400G_##_instance##_E, NULL,                           \
        PORT_400G_MAC_INTERRUPT_CAUSE_MAC(_port),                                                \
        PORT_400G_MAC_INTERRUPT_MASK_MAC(_port),                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
        PRV_CPSS_AC5P_400G_MAC_INST_##_instance##_PORT_NUM_##_port##_LINK_OK_CHANGE_E,           \
        PRV_CPSS_AC5P_400G_MAC_INST_##_instance##_PORT_NUM_##_port##_TSD_NON_ACCURATE_PTP_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define mac_pcs_SEG_NODE_MAC(bit, _instance, _port)                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MAC_400G_##_instance##_E, NULL,                           \
        PORT_SEG_MAC_INTERRUPT_CAUSE_MAC(_port),                                                 \
        PORT_SEG_MAC_INTERRUPT_MASK_MAC(_port),                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
        PRV_CPSS_AC5P_400G_MAC_INST_##_instance##_SEG_PORT_NUM_##_port##_LINK_OK_CHANGE_E,       \
        PRV_CPSS_AC5P_400G_MAC_INST_##_instance##_SEG_PORT_NUM_##_port##_TSD_NON_ACCURATE_PTP_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define mac_pcs_400G_SUB_TREE_MAC(bit, _instance)                                                    \
    /* 400G MAC */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MAC_400G_##_instance##_E, NULL,                               \
        0x0000004C, 0x00000050,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5P_400G_MAC_INST_##_instance##_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,         \
        PRV_CPSS_AC5P_400G_MAC_INST_##_instance##_GLOBAL_INTERRUPT_SUMMARY_CAUSE_FEC_NCE_INT_SUM_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 10, NULL, NULL},                                     \
            mac_pcs_400G_NODE_MAC(1, _instance,  0),                                                 \
            mac_pcs_400G_NODE_MAC(2, _instance,  1),                                                 \
            mac_pcs_400G_NODE_MAC(3, _instance,  2),                                                 \
            mac_pcs_400G_NODE_MAC(4, _instance,  3),                                                 \
            mac_pcs_400G_NODE_MAC(5, _instance,  4),                                                 \
            mac_pcs_400G_NODE_MAC(6, _instance,  5),                                                 \
            mac_pcs_400G_NODE_MAC(7, _instance,  6),                                                 \
            mac_pcs_400G_NODE_MAC(8, _instance,  7),                                                 \
            mac_pcs_SEG_NODE_MAC (9, _instance,  0),                                                 \
            mac_pcs_SEG_NODE_MAC(10, _instance,  1)

#define mac_pcs_CPU_NODE_MAC(bit)                                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MAC_CPU_0_E, NULL,                                            \
        0x0000000C, 0x00000010,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5P_CPU_MAC_PORT_INTERRUPT_CAUSE_LINK_OK_CHANGED_E,                                \
        PRV_CPSS_AC5P_CPU_MAC_PORT_INTERRUPT_CAUSE_TSD_NON_ACCURATE_PTP_E,                           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define mif_CPU_NODE_MAC(bit)                                                                        \
    /* CPU MIF */                                                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MIF_CPU_0_E, NULL,                                            \
        0x00000120, 0x00000124,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5P_DUMMY_ONLY_PLACE_HOLDER_6_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_TYPE_8_TX_PROTOCOL_VIOLATION_INTERRUPT_E, \
        PRV_CPSS_AC5P_DUMMY_ONLY_PLACE_HOLDER_6_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_BAD_ACCESS_SUM_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define mif_USX_NODE_MAC(bit, _instance)                                                              \
    /* USX MIF */                                                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MIF_USX_##_instance##_E, NULL,                                 \
        0x00000120, 0x00000124,                                                                       \
        prvCpssDrvHwPpPortGroupIsrRead,                                                               \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                              \
        PRV_CPSS_AC5P_USX_MAC_INST_0_5_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_TYPE_8_TX_PROTOCOL_VIOLATION_INTERRUPT_E + (32 * (_instance)), \
        PRV_CPSS_AC5P_USX_MAC_INST_0_5_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_BAD_ACCESS_SUM_E + (32 * (_instance)),       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define mif_400G_NODE_MAC(bit, _instance)                                                             \
    /* 400G MIF */                                                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MIF_400G_##_instance##_E, NULL,                                \
        0x00000120, 0x00000124,                                                                       \
        prvCpssDrvHwPpPortGroupIsrRead,                                                               \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                              \
        PRV_CPSS_AC5P_400G_MAC_INST_##_instance##_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_TYPE_8_TX_PROTOCOL_VIOLATION_INTERRUPT_E, \
        PRV_CPSS_AC5P_400G_MAC_INST_##_instance##_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_BAD_ACCESS_SUM_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_CH0_NODE_MAC(bit, _prefix, _unit, _index)                                              \
    /* MacSec Channel Interrupts 0 Cause */                                                             \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x00000338, 0x0000033C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_ING_##_prefix##_UNIT_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_0_CHANNEL_INTERRUPT_E, \
        PRV_CPSS_AC5P_MSEC_ING_##_prefix##_UNIT_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_15_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_CH1_NODE_MAC(bit, _prefix, _unit, _index)                                              \
    /* MacSec Channel Interrupts 1 Cause */                                                             \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x00000340, 0x00000344,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_ING_##_prefix##_UNIT_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_16_CHANNEL_INTERRUPT_E, \
        PRV_CPSS_AC5P_MSEC_ING_##_prefix##_UNIT_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_31_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_EIP_163_GLOBAL_NODE_MAC(bit, _unit, _index)                                            \
    /* Classifier Engine(EIP-163) Egress Interrupts Cause */                                            \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x0000FC2C, 0x0000FC08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_EGR_163_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E,    \
        PRV_CPSS_AC5P_MSEC_EGR_163_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_CHANNEL_IRQ_15_31_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_EIP_163_GLOBAL_NODE_MAC(bit, _unit, _index)                                            \
    /* Classifier Engine(EIP-163) Ingress Interrupts Cause */                                           \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x0000FC2C, 0x0000FC08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_ING_163_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E, \
        PRV_CPSS_AC5P_MSEC_ING_163_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_CHANNEL_IRQ_15_31_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_EIP_66_NODE_MAC(bit, _unit, _index)                                                    \
    /* MACSec Transform Crypto Engine(EIP-66) Egress Interrupts Cause */                                \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x00005C2C, 0x00005C08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_EGR_66_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_0_E, \
        PRV_CPSS_AC5P_MSEC_EGR_66_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SEQ_NUM_ROLLOVER_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_EIP_66_NODE_MAC(bit, _unit, _index)                                                    \
    /* MACSec Transform Crypto Engine(EIP-66) Ingress Interrupts Cause */                               \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x00005C2C, 0x00005C08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_ING_66_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_0_E, \
        PRV_CPSS_AC5P_MSEC_ING_66_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_14_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_EIP_164_GLOBAL_NODE_MAC(bit, _unit, _index)                                            \
    /* Transformer Engine(EIP-164) Egress Interrupts Cause */                                           \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x0000FC2C, 0x0000FC08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_EGR_164_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_DROP_CLASS_E, \
        PRV_CPSS_AC5P_MSEC_EGR_164_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_CHANNEL_IRQ_15_31_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                             \
        msec_EGR_EIP_66_NODE_MAC(3, _unit, _index)

#define msec_ING_EIP_164_GLOBAL_NODE_MAC(bit, _unit, _index)                                            \
    /* Transformer Engine(EIP-164) Ingress Interrupts Cause */                                          \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x0000FC2C, 0x0000FC08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_ING_164_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_DROP_CLASS_E, \
        PRV_CPSS_AC5P_MSEC_ING_164_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_CHANNEL_IRQ_15_31_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                               \
        msec_ING_EIP_66_NODE_MAC(3, _unit, _index)

#define msec_EGR_CH0_NODE_MAC(bit, _prefix, _unit, _index)                                              \
    /* MacSec Channel Interrupts 0 Cause */                                                             \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x00000338, 0x0000033c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_EGR_##_prefix##_UNIT_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_0_CHANNEL_INTERRUPT_E, \
        PRV_CPSS_AC5P_MSEC_EGR_##_prefix##_UNIT_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_15_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_CH1_NODE_MAC(bit, _prefix, _unit, _index)                                              \
    /* MacSec Channel Interrupts 1 Cause */                                                             \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x00000340, 0x00000344,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_EGR_##_prefix##_UNIT_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_16_CHANNEL_INTERRUPT_E, \
        PRV_CPSS_AC5P_MSEC_EGR_##_prefix##_UNIT_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_31_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_163_SUB_TREE_MAC(bit, _index, _unit)                                                   \
    /* MSEC Egress 163 */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_##_unit##_E,  NULL,                         \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_EGR_163_INST_0_UNIT_##_index##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E, \
        PRV_CPSS_AC5P_MSEC_EGR_163_INST_0_UNIT_##_index##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_1_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                            \
        msec_EGR_EIP_163_GLOBAL_NODE_MAC(1, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_##_unit##_E, _index), \
        msec_EGR_CH0_NODE_MAC(4, 163_INST_0, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_##_unit##_E, _index), \
        msec_EGR_CH1_NODE_MAC(5, 163_INST_0, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_##_unit##_E, _index)

#define msec_ING_163_SUB_TREE_MAC(bit, _index, _unit)                                                   \
    /* MSEC Ingress 163 */                                                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_##_unit##_E,  NULL,                         \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_ING_163_INST_1_UNIT_##_index##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E, \
        PRV_CPSS_AC5P_MSEC_ING_163_INST_1_UNIT_##_index##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_1_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                         \
        msec_ING_EIP_163_GLOBAL_NODE_MAC(1, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_##_unit##_E, _index), \
        msec_ING_CH0_NODE_MAC(4, 163_INST_1, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_##_unit##_E, _index), \
        msec_ING_CH1_NODE_MAC(5, 163_INST_1, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_##_unit##_E, _index)

#define msec_EGR_164_SUB_TREE_MAC(bit, _index, _unit)                                                   \
    /* MSEC Egress 164 */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_##_unit##_E,  NULL,                         \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_EGR_164_INST_2_UNIT_##_index##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E, \
        PRV_CPSS_AC5P_MSEC_EGR_164_INST_2_UNIT_##_index##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_1_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                         \
        msec_EGR_EIP_164_GLOBAL_NODE_MAC(1, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_##_unit##_E, _index), \
        msec_EGR_CH0_NODE_MAC(4, 164_INST_2, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_##_unit##_E, _index), \
        msec_EGR_CH1_NODE_MAC(5, 164_INST_2, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_##_unit##_E, _index)

#define msec_ING_164_SUB_TREE_MAC(bit, _index, _unit)                                                   \
    /* MSEC Egress 164 */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_##_unit##_E,  NULL,                         \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_ING_164_INST_3_UNIT_##_index##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E, \
        PRV_CPSS_AC5P_MSEC_ING_164_INST_3_UNIT_##_index##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_1_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                         \
        msec_ING_EIP_164_GLOBAL_NODE_MAC(1, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_##_unit##_E, _index), \
        msec_ING_CH0_NODE_MAC(4, 164_INST_3, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_##_unit##_E, _index), \
        msec_ING_CH1_NODE_MAC(5, 164_INST_3, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_##_unit##_E, _index)

#define pca_MSEC_CH0_NODE_MAC(bit, _prefix, _unit, _index)                                              \
    /* PCA MacSec Channel Interrupts 0 Cause */                                                         \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x00000338, 0x0000033C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_MACRO_##_prefix##_PCA_UNIT_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_0_CHANNEL_INTERRUPT_E, \
        PRV_CPSS_AC5P_PCA_MACRO_##_prefix##_PCA_UNIT_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_15_CHANNEL_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pca_MSEC_CH1_NODE_MAC(bit, _prefix, _unit, _index)                                              \
    /* PCA MacSec Channel Interrupts 1 Cause */                                                         \
    {bit, GT_FALSE, _unit,  NULL,                                                                       \
        0x00000340, 0x00000344,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_MACRO_##_prefix##_PCA_UNIT_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_16_CHANNEL_INTERRUPT_E, \
        PRV_CPSS_AC5P_PCA_MACRO_##_prefix##_PCA_UNIT_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_31_CHANNEL_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pca_MSEC2_EGR_SUB_TREE_MAC(bit, _index, _unit)                                                  \
    /* PCA MSEC */                                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_##_unit##_E,  NULL,                         \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_MACRO_INST_2_PCA_UNIT_INST_##_index##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E, \
        PRV_CPSS_AC5P_PCA_MACRO_INST_2_PCA_UNIT_INST_##_index##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_1_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
        pca_MSEC_CH0_NODE_MAC(4, INST_2, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_##_unit##_E, _index),  \
        pca_MSEC_CH1_NODE_MAC(5, INST_2, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_##_unit##_E, _index)

#define pca_MSEC2_ING_SUB_TREE_MAC(bit, _index, _unit)                                                  \
    /* PCA MSEC */                                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_##_unit##_E,  NULL,                         \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_MACRO_INST_2_PCA_UNIT_INST_##_index##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E, \
        PRV_CPSS_AC5P_PCA_MACRO_INST_2_PCA_UNIT_INST_##_index##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_1_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
        pca_MSEC_CH0_NODE_MAC(4, INST_2, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_##_unit##_E, _index),  \
        pca_MSEC_CH1_NODE_MAC(5, INST_2, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_##_unit##_E, _index)

#define pca_MSEC3_EGR_SUB_TREE_MAC(bit, _index, _unit)                                              \
    /* PCA MSEC */                                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_##_unit##_E,  NULL,                         \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_MACRO_INST_3_PCA_UNIT_INST_##_index##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E, \
        PRV_CPSS_AC5P_PCA_MACRO_INST_3_PCA_UNIT_INST_##_index##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_1_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
        pca_MSEC_CH0_NODE_MAC(4, INST_3, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_##_unit##_E, _index),          \
        pca_MSEC_CH1_NODE_MAC(5, INST_3, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_##_unit##_E, _index)

#define pca_MSEC3_ING_SUB_TREE_MAC(bit, _index, _unit)                                              \
    /* PCA MSEC */                                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_##_unit##_E,  NULL,                         \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_MACRO_INST_3_PCA_UNIT_INST_##_index##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E, \
        PRV_CPSS_AC5P_PCA_MACRO_INST_3_PCA_UNIT_INST_##_index##_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_1_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
        pca_MSEC_CH0_NODE_MAC(4, INST_3, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_##_unit##_E, _index),          \
        pca_MSEC_CH1_NODE_MAC(5, INST_3, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_##_unit##_E, _index)

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
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CTSU_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,     \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CTSU_GLOBAL_INTERRUPT_CAUSE_TIME_STAMPING_COMMAND_PARSING_ERROR_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define ctsu_channel_group0_PCA_SUB_TREE_MAC(bit, _instance)                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E, NULL,                                  \
        0x00003800, 0x00003820,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,  \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_0_CHANNEL_CTSU_16_PCA_NODES_MAC(_instance)

#define ctsu_channel_group1_PCA_SUB_TREE_MAC(bit, _instance)                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E, NULL,                                  \
        0x00003804, 0x00003824,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,  \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_1_CHANNEL_CTSU_16_PCA_NODES_MAC(_instance)

#define ctsu_channel_PCA_NODE_MAC(bit, _instance, channel)                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E, NULL,                                  \
        CTSU_CHANNEL_PCA_INTERRUPT_CAUSE_MAC(channel),                                                  \
        CTSU_CHANNEL_PCA_INTERRUPT_MASK_MAC(channel),                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CHANNEL_##channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_0_NEW_ENTRY_INTERRUPT_E,     \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CHANNEL_##channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_1_OVERFLOW_INTERRUPT_E,      \
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
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CTSU_INTERRUPT_CAUSE_SUMMARY_GLOBAL_INTERRUPT_CAUSE_SUMMARY_E, \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CTSU_INTERRUPT_CAUSE_SUMMARY_CHANNEL_GROUP_1_INTERRUPT_CAUSE_SUMMARY_E, \
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
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CTSU_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,     \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CTSU_GLOBAL_INTERRUPT_CAUSE_TIME_STAMPING_COMMAND_PARSING_ERROR_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define ctsu_channel_group0_MSEC_SUB_TREE_MAC(bit, _instance)                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E, NULL,                                  \
        0x00003800, 0x00003820,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,  \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_0_CHANNEL_CTSU_16_MSEC_NODES_MAC(_instance)

#define ctsu_channel_group1_MSEC_SUB_TREE_MAC(bit, _instance)                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E, NULL,                                  \
        0x00003804, 0x00003824,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,  \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_1_CHANNEL_CTSU_16_MSEC_NODES_MAC(_instance)

#define ctsu_channel_MSEC_NODE_MAC(bit, _instance, channel)                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E, NULL,                                  \
        CTSU_CHANNEL_MSEC_INTERRUPT_CAUSE_MAC(channel),                                                  \
        CTSU_CHANNEL_MSEC_INTERRUPT_MASK_MAC(channel),                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CHANNEL_##channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_0_NEW_ENTRY_INTERRUPT_E,     \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CHANNEL_##channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_1_OVERFLOW_INTERRUPT_E,      \
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
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CTSU_INTERRUPT_CAUSE_SUMMARY_GLOBAL_INTERRUPT_CAUSE_SUMMARY_E, \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CTSU_INTERRUPT_CAUSE_SUMMARY_CHANNEL_GROUP_1_INTERRUPT_CAUSE_SUMMARY_E, \
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
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CHID_15_0_##channel##_CH_OCCUP_CNT_OF_INTERRUPT_CAUSE_E,  \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CHID_15_0_##channel##_CH_CRT_TOK0_ACCU_OF_INTERRUPT_CAUSE_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define sff_channel1_PCA_NODE_MAC(bit, _instance, channel)                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        SFF_GR1_CHANNEL_INTERRUPT_CAUSE_MAC((channel % 16)),                                            \
        SFF_GR1_CHANNEL_INTERRUPT_MASK_MAC((channel % 16)),                                             \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CHID_31_16_##channel##_CH_OCCUP_CNT_OF_INTERRUPT_CAUSE_E,  \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CHID_31_16_##channel##_CH_CRT_TOK0_ACCU_OF_INTERRUPT_CAUSE_E, \
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
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CH_GRP0_SUMMARY_INTERRUPT_CAUSE_CHID_0_CH_GRP0_SUM_INTERRUPT_CAUSE_E,  \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CH_GRP0_SUMMARY_INTERRUPT_CAUSE_CHID_15_CH_GRP0_SUM_INTERRUPT_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_0_CHANNEL_SFF_16_PCA_NODES_MAC(_instance)

#define sff_channel_group1_PCA_SUB_TREE_MAC(bit, _instance)                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        0x00000030, 0x00000034,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CH_GRP1_SUMMARY_INTERRUPT_CAUSE_CHID_16_CH_GRP1_SUM_INTERRUPT_CAUSE_E,  \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CH_GRP1_SUMMARY_INTERRUPT_CAUSE_CHID_31_CH_GRP1_SUM_INTERRUPT_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_1_CHANNEL_SFF_16_PCA_NODES_MAC(_instance)

#define sff_PCA_GLOBAL_NODE_MAC(bit, _instance)                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        0x00000010, 0x00000014,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_GLOBAL_INTERRUPT_CAUSE_SOP_MISMATCH_INTERRUPT_CAUSE_E,     \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_GLOBAL_INTERRUPT_CAUSE_BAD_ADDRESS_INTERRUPT_CAUSE_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define sff_PCA_SUB_TREE_MAC(bit, _instance)                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        0x00000020, 0x00000024,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_SFF_UNIT_SUMMARY_CAUSE_GLOB_INTERRUPT_SUMMARY_CAUSE_E,     \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_SFF_UNIT_SUMMARY_CAUSE_CH_GROUP1_INTERRUPT_SUMMARY_CAUSE_E, \
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
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CHID_15_0_##channel##_CH_OCCUP_CNT_OF_INTERRUPT_CAUSE_E,  \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CHID_15_0_##channel##_CH_CRT_TOK0_ACCU_OF_INTERRUPT_CAUSE_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define sff_channel1_MSEC_NODE_MAC(bit, _instance, channel)                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        SFF_GR1_CHANNEL_INTERRUPT_CAUSE_MAC((channel % 16)),                                            \
        SFF_GR1_CHANNEL_INTERRUPT_MASK_MAC((channel % 16)),                                             \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CHID_31_16_##channel##_CH_OCCUP_CNT_OF_INTERRUPT_CAUSE_E, \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CHID_31_16_##channel##_CH_CRT_TOK0_ACCU_OF_INTERRUPT_CAUSE_E,      \
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
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CH_GRP0_SUMMARY_INTERRUPT_CAUSE_CHID_0_CH_GRP0_SUM_INTERRUPT_CAUSE_E,  \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CH_GRP0_SUMMARY_INTERRUPT_CAUSE_CHID_15_CH_GRP0_SUM_INTERRUPT_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_0_CHANNEL_SFF_16_MSEC_NODES_MAC(_instance)

#define sff_channel_group1_MSEC_SUB_TREE_MAC(bit, _instance)                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        0x00000030, 0x00000034,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CH_GRP1_SUMMARY_INTERRUPT_CAUSE_CHID_16_CH_GRP1_SUM_INTERRUPT_CAUSE_E, \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CH_GRP1_SUMMARY_INTERRUPT_CAUSE_CHID_31_CH_GRP1_SUM_INTERRUPT_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_1_CHANNEL_SFF_16_MSEC_NODES_MAC(_instance)

#define sff_MSEC_GLOBAL_NODE_MAC(bit, _instance)                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        0x00000010, 0x00000014,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_GLOBAL_INTERRUPT_CAUSE_SOP_MISMATCH_INTERRUPT_CAUSE_E,    \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_GLOBAL_INTERRUPT_CAUSE_BAD_ADDRESS_INTERRUPT_CAUSE_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define sff_MSEC_SUB_TREE_MAC(bit, _instance)                                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_SFF_##_instance##_E, NULL,                                   \
        0x00000020, 0x00000024,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_SFF_UNIT_SUMMARY_CAUSE_GLOB_INTERRUPT_SUMMARY_CAUSE_E,    \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_SFF_UNIT_SUMMARY_CAUSE_CH_GROUP1_INTERRUPT_SUMMARY_CAUSE_E, \
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
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CHANNEL_##_channel##_RX_SDB_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CHANNEL_##_channel##_RX_SDB_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_rx_global_MSEC_NODE_MAC(bit, _instance)                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00000750, 0x00000754,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_RX_GLOBAL_INTERRUPT_CAUSE_RX_ROTATED_FIFO_OVERRUN_INTERRUPT_E,\
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_RX_GLOBAL_INTERRUPT_CAUSE_RX_SDB_CHANNEL_ID_ERROR_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_rx_channel_group0_MSEC_SUB_TREE_MAC(bit, _instance)                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00000600, 0x00000620,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_RX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,\
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_RX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_RX_CHANNEL_GROUP_0_MSEC_16_NODES_MAC(_instance)

#define brg_rx_channel_group1_MSEC_SUB_TREE_MAC(bit, _instance)                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00000604, 0x00000624,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_RX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,\
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_RX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
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
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CHANNEL_##_channel##_TX_SLICE_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,   \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CHANNEL_##_channel##_TX_SDB_CREDIT_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_tx_global_MSEC_NODE_MAC(bit, _instance)                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00001a64, 0x00001a68,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_TX_GLOBAL_INTERRUPT_CAUSE_ACCUM_MEMORY_FULL_E,            \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_TX_GLOBAL_INTERRUPT_CAUSE_TX_CREDIT_FIFO_OVERRUN_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_tx_channel_group0_MSEC_SUB_TREE_MAC(bit, _instance)                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00001900, 0x00001920,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_TX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,\
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_TX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_TX_CHANNEL_GROUP_0_MSEC_16_NODES_MAC(_instance)

#define brg_tx_channel_group1_MSEC_SUB_TREE_MAC(bit, _instance)                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00001904, 0x00001924,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_TX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,\
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_TX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_TX_CHANNEL_GROUP_1_MSEC_16_NODES_MAC(_instance)

#define brg_MSEC_NODE_MAC(bit, _instance)                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00000004, 0x00000008,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_BRG_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,             \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_BRG_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_MSEC_SUB_TREE_MAC(bit, _instance)                                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x0000000c, 0x00000010,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_BRG_INTERRUPT_CAUSE_SUMMARY_RX_GLOBAL_INTERRUPT_SUMMARY_E,\
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_BRG_INTERRUPT_CAUSE_SUMMARY_BRIDGE_CAUSE_INTERRUPT_SUMMARY_E, \
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
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CHANNEL_##_channel##_RX_SDB_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CHANNEL_##_channel##_RX_SDB_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_rx_global_PCA_NODE_MAC(bit, _instance)                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00000750, 0x00000754,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_RX_GLOBAL_INTERRUPT_CAUSE_RX_ROTATED_FIFO_OVERRUN_INTERRUPT_E,\
        PRV_CPSS_AC5P_PCA_INST_##_instance##_RX_GLOBAL_INTERRUPT_CAUSE_RX_SDB_CHANNEL_ID_ERROR_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_rx_channel_group0_PCA_SUB_TREE_MAC(bit, _instance)                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00000600, 0x00000620,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_RX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,\
        PRV_CPSS_AC5P_PCA_INST_##_instance##_RX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_RX_CHANNEL_GROUP_0_PCA_16_NODES_MAC(_instance)

#define brg_rx_channel_group1_PCA_SUB_TREE_MAC(bit, _instance)                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00000604, 0x00000624,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_RX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,\
        PRV_CPSS_AC5P_PCA_INST_##_instance##_RX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
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
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CHANNEL_##_channel##_TX_SLICE_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,   \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CHANNEL_##_channel##_TX_SDB_CREDIT_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_tx_global_PCA_NODE_MAC(bit, _instance)                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00001a64, 0x00001a68,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_TX_GLOBAL_INTERRUPT_CAUSE_ACCUM_MEMORY_FULL_E,             \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_TX_GLOBAL_INTERRUPT_CAUSE_TX_CREDIT_FIFO_OVERRUN_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_tx_channel_group0_PCA_SUB_TREE_MAC(bit, _instance)                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00001900, 0x00001920,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_TX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,\
        PRV_CPSS_AC5P_PCA_INST_##_instance##_TX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_TX_CHANNEL_GROUP_0_PCA_16_NODES_MAC(_instance)

#define brg_tx_channel_group1_PCA_SUB_TREE_MAC(bit, _instance)                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00001904, 0x00001924,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_TX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,\
        PRV_CPSS_AC5P_PCA_INST_##_instance##_TX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_TX_CHANNEL_GROUP_1_PCA_16_NODES_MAC(_instance)

#define brg_PCA_NODE_MAC(bit, _instance)                                                                \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x00000004, 0x00000008,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_BRG_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,              \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_BRG_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_PCA_SUB_TREE_MAC(bit, _instance)                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E, NULL,                                   \
        0x0000000c, 0x00000010,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_BRG_INTERRUPT_CAUSE_SUMMARY_RX_GLOBAL_INTERRUPT_SUMMARY_E, \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_BRG_INTERRUPT_CAUSE_SUMMARY_BRIDGE_CAUSE_INTERRUPT_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 7, NULL, NULL},                                         \
        brg_rx_global_PCA_NODE_MAC(1, _instance),                                                       \
        brg_rx_channel_group0_PCA_SUB_TREE_MAC(2, _instance),                                           \
        brg_rx_channel_group1_PCA_SUB_TREE_MAC(3, _instance),                                           \
        brg_tx_global_PCA_NODE_MAC(6, _instance),                                                       \
        brg_tx_channel_group0_PCA_SUB_TREE_MAC(7, _instance),                                           \
        brg_tx_channel_group1_PCA_SUB_TREE_MAC(8, _instance),                                           \
        brg_PCA_NODE_MAC(9, _instance)

#define sdw_SDW_INST_NODE_MAC(bit, _instance)                                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_SDW0_0_E + _instance, NULL,                                      \
        0x00000300, 0x00000304,                                                                         \
        prvCpssDrvHwPpSerdesIsrRead,                                                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_SDW_INST_##_instance##_SDW_INTERRUPT_CAUSE_SER_DES_INTERRUPT_E,                   \
        PRV_CPSS_AC5P_SDW_INST_##_instance##_SDW_INTERRUPT_CAUSE_SQ_DETECT_3_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define sdw_28G_SDW_INST_NODE_MAC(bit)                                                                  \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_SERDES_1_E, NULL,                                                \
        0x00000300, 0x00000304,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_SDW_28G_SDW_INTERRUPT_CAUSE_SER_DES_INTERRUPT_E,                                  \
        PRV_CPSS_AC5P_SDW_28G_SDW_INTERRUPT_CAUSE_SQ_DETECT_3_E,                                        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define cnm_grp_0_0_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                 \
    /* FuncUnitsIntsSum Interrupt Cause */                                              \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                            \
        0x000003f8, 0x000003fc,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AC5P_MG_##_mgIndex##_FUNCTIONAL_INTERRUPTS_SUMMARY_CAUSE_CP_TOP_EM_INT_SUM_E,    \
        PRV_CPSS_AC5P_MG_##_mgIndex##_FUNCTIONAL_INTERRUPTS_SUMMARY_CAUSE_CP_TOP_PIPE0_QAG_INT_SUM_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 31, NULL, NULL},                                 \
        em_NODE_MAC(1),                                                                 \
        epcl_tai_NODE_MAC(2, 0),                                                        \
        epcl_tai_NODE_MAC(3, 1),                                                        \
        mt_NODE_MAC(4),                                                                 \
        pha_tai_NODE_MAC(5, 0),                                                         \
        pha_tai_NODE_MAC(6, 1),                                                         \
        bma_NODE_MAC(7),                                                                \
        cnc_SUB_TREE_MAC( 8, 0),                                                        \
        cnc_SUB_TREE_MAC( 9, 1),                                                        \
        cnc_SUB_TREE_MAC(10, 2),                                                        \
        cnc_SUB_TREE_MAC(11, 3),                                                        \
        eft_NODE_MAC(12),                                                               \
        eoam_NODE_MAC(13),                                                              \
        epcl_NODE_MAC(14),                                                              \
        eplr_NODE_MAC(15),                                                              \
        eq_SUB_TREE_MAC(16),                                                            \
        erep_NODE_MAC(17),                                                              \
        ermrk_NODE_MAC(18),                                                             \
        ha_NODE_MAC(19),                                                                \
        hbu_SUB_TREE_MAC(20),                                                           \
        ioam_NODE_MAC(21),                                                              \
        iplr0_NODE_MAC(22),                                                             \
        iplr1_NODE_MAC(23),                                                             \
        router_NODE_MAC(24),                                                            \
        l2i_NODE_MAC(25),                                                               \
        lpm_NODE_MAC(26),                                                               \
        mll_NODE_MAC(27),                                                               \
        pcl_NODE_MAC(28),                                                               \
        ppu_NODE_MAC(29),                                                               \
        preq_NODE_MAC(30),                                                              \
        qag_NODE_MAC(31)

#define cnm_grp_0_1_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                             \
    /* DataPathIntsSum Interrupt Cause */                                                           \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                        \
        0x000000a4, 0x000000a8,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5P_MG_##_mgIndex##_DATA_PATH_INTERRUPTS_SUMMARY_CAUSE_CP_TOP_PIPE0_SHT_INT_SUM_E,\
        PRV_CPSS_AC5P_MG_##_mgIndex##_DATA_PATH_INTERRUPTS_SUMMARY_CAUSE_GOP_DOWNLINK_TOP_EPI0_ANP_INT_SUM0_E,  \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 31, NULL, NULL},                                             \
        sht_NODE_MAC(1),                                                                            \
        pha_SUB_TREE_MAC(2),                                                                        \
        tti_NODE_MAC(3),                                                                            \
        shm_NODE_MAC(4),                                                                            \
        tcam_SUB_TREE_MAC(5),                                                                       \
        tti_tai_NODE_MAC(6, 2, 0),                                                                  \
        tti_tai_NODE_MAC(7, 2, 1),                                                                  \
        ia_NODE_MAC(8),                                                                             \
        rxDma_NODE_MAC( 9, 0),                                                                      \
        rxDma_NODE_MAC(10, 1),                                                                      \
        rxDma_NODE_MAC(11, 2),                                                                      \
        rxDma_NODE_MAC(12, 3),                                                                      \
        txDma_NODE_MAC(13, 0),                                                                      \
        txFifo_NODE_MAC(14, 0),                                                                     \
        txDma_NODE_MAC(15, 1),                                                                      \
        txFifo_NODE_MAC(16, 1),                                                                     \
        txDma_NODE_MAC(17, 2),                                                                      \
        txFifo_NODE_MAC(18, 2),                                                                     \
        txDma_NODE_MAC(19, 3),                                                                      \
        txFifo_NODE_MAC(20, 3),                                                                     \
        anp_400G_SUB_TREE_MAC(21, 1),                                                               \
        anp_USX_SUB_TREE_MAC(22, 5, 0),                                                             \
        anp_USX_SUB_TREE_MAC(23, 6, 0),                                                             \
        anp_USX_SUB_TREE_MAC(24, 7, 0),                                                             \
        anp_USX_SUB_TREE_MAC(25, 8, 0),                                                             \
        anp_USX_SUB_TREE_MAC(26, 9, 0),                                                             \
        anp_USX_SUB_TREE_MAC(27, 10, 0),                                                            \
        tai_400G_NODE_MAC(28, 6, 0),                                                                \
        tai_400G_NODE_MAC(29, 6, 1),                                                                \
        tai_USX_NODE_MAC(30, 11, 0),                                                                \
        tai_USX_NODE_MAC(31, 11, 1)

#define cnm_grp_0_2_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                 \
    /* PortsIntsSum Interrupt Cause */                                                                  \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        0x00000080, 0x00000084,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MG_##_mgIndex##_PORTS_INTERRUPTS_SUMMARY_CAUSE_GOP_DOWNLINK_TOP_EPI0_ANP_INT_SUM1_E, \
        PRV_CPSS_AC5P_MG_##_mgIndex##_PORTS_INTERRUPTS_SUMMARY_CAUSE_GOP_DOWNLINK_TOP_EPI1_ANP_INT_SUM5_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 30, NULL, NULL},                                                 \
        led_400G_NODE_MAC(1, 0),                                                                        \
        led_USX_NODE_MAC(2, 0),                                                                         \
        mac_pcs_400G_SUB_TREE_MAC(3, 0),                                                                \
        mac_pcs_USX0_SUB_TREE_MAC(4),                                                                   \
        mac_pcs_USX1_SUB_TREE_MAC(5),                                                                   \
        mac_pcs_USX2_SUB_TREE_MAC(6),                                                                   \
        mif_400G_NODE_MAC(7, 1),                                                                        \
        mif_USX_NODE_MAC(8, 0),                                                                         \
        sdw_SDW_INST_NODE_MAC(10, 0),                                                                   \
        sdw_SDW_INST_NODE_MAC(11, 1),                                                                   \
        anp_400G_SUB_TREE_MAC(12, 2),                                                                   \
        anp_USX_SUB_TREE_MAC(13, 5, 1),                                                                 \
        anp_USX_SUB_TREE_MAC(14, 6, 1),                                                                 \
        anp_USX_SUB_TREE_MAC(15, 7, 1),                                                                 \
        anp_USX_SUB_TREE_MAC(16, 8, 1),                                                                 \
        anp_USX_SUB_TREE_MAC(17, 9, 1),                                                                 \
        anp_USX_SUB_TREE_MAC(18, 10, 1),                                                                \
        anp_CPU_NODE_MAC(19),                                                                           \
        tai_CPU_PORT_NODE_MAC(20, 0),                                                                   \
        tai_CPU_PORT_NODE_MAC(21, 1),                                                                   \
        tai_400G_NODE_MAC(22, 7, 0),                                                                    \
        tai_400G_NODE_MAC(23, 7, 1),                                                                    \
        tai_USX_NODE_MAC(24, 12, 0),                                                                    \
        tai_USX_NODE_MAC(25, 12, 1),                                                                    \
        led_400G_NODE_MAC(26, 1),                                                                       \
        led_USX_NODE_MAC(27, 1),                                                                        \
        led_CPU_NODE_MAC(28),                                                                           \
        mac_pcs_400G_SUB_TREE_MAC(29, 1),                                                               \
        mac_pcs_USX3_SUB_TREE_MAC(30),                                                                  \
        mac_pcs_USX4_SUB_TREE_MAC(31)

#define cnm_grp_0_3_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                 \
    /* DFX */                                                                                           \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        0x000000ac, 0x000000b0,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MG_##_mgIndex##_DFX_INTERRUPTS_SUMMARY_CAUSE_GOP_DOWNLINK_TOP_EPI1_ANP_INT_SUM6_E,\
        PRV_CPSS_AC5P_MG_##_mgIndex##_DFX_INTERRUPTS_SUMMARY_CAUSE_GOP_DOWNLINK_TOP_PCA0_PZARB_TX_INT_SUM_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 31, NULL, NULL},                                                 \
        mac_pcs_USX5_SUB_TREE_MAC(1),                                                                   \
        mac_pcs_CPU_NODE_MAC(2),                                                                        \
        mif_400G_NODE_MAC(3, 2),                                                                        \
        mif_USX_NODE_MAC(4, 1),                                                                         \
        mif_CPU_NODE_MAC(5),                                                                            \
        sdw_SDW_INST_NODE_MAC(6, 2),                                                                    \
        sdw_SDW_INST_NODE_MAC(7, 3),                                                                    \
        sdw_28G_SDW_INST_NODE_MAC(8),                                                                   \
        ctsu_MSEC_SUB_TREE_MAC(9, 0),                                                                   \
        brg_MSEC_SUB_TREE_MAC(10, 0),                                                                   \
        msec_EGR_163_SUB_TREE_MAC(11, 0, 0),                                                            \
        msec_ING_163_SUB_TREE_MAC(12, 0, 0),                                                            \
        msec_EGR_164_SUB_TREE_MAC(13, 0, 0),                                                            \
        msec_ING_164_SUB_TREE_MAC(14, 0, 0),                                                            \
        lmu_MSEC_SUB_TREE_MAC(15, 0),                                                                   \
        pca0_tai_NODE_MAC(16, 0),                                                                       \
        pca0_tai_NODE_MAC(17, 1),                                                                       \
        pzarb_MSEC_RPZ_INST_NODE_MAC(18, 0, 0),                                                         \
        pzarb_MSEC_TPZ_INST_NODE_MAC(19, 0, 0),                                                         \
        sff_MSEC_SUB_TREE_MAC(20, 0),                                                                   \
        ctsu_MSEC_SUB_TREE_MAC(21, 1),                                                                  \
        brg_MSEC_SUB_TREE_MAC(22, 1),                                                                   \
        msec_EGR_163_SUB_TREE_MAC(23, 1, 1),                                                            \
        msec_ING_163_SUB_TREE_MAC(24, 1, 1),                                                            \
        msec_EGR_164_SUB_TREE_MAC(25, 1, 1),                                                            \
        msec_ING_164_SUB_TREE_MAC(26, 1, 1),                                                            \
        lmu_MSEC_SUB_TREE_MAC(27, 1),                                                                   \
        pca1_tai_NODE_MAC(28, 0),                                                                       \
        pca1_tai_NODE_MAC(29, 1),                                                                       \
        pzarb_MSEC_RPZ_INST_NODE_MAC(30, 1, 1),                                                         \
        pzarb_MSEC_TPZ_INST_NODE_MAC(31, 1, 1)

#define cnm_grp_0_4_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                 \
    /* DFX1 */                                                                                          \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        0x000000b8, 0x000000bc,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MG_##_mgIndex##_DFX_1_INTERRUPTS_SUMMARY_CAUSE_GOP_DOWNLINK_TOP_PCA0_SFF2IRQ_INT_SUM_E,\
        PRV_CPSS_AC5P_MG_##_mgIndex##_DFX_1_INTERRUPTS_SUMMARY_CAUSE_GOP_UPLINK_TOP_PCA2_DP2SDB_INTERRUPT_CAUSE_INT_SUM_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 31, NULL, NULL},                                                 \
        sff_MSEC_SUB_TREE_MAC(1,  1),                                                                   \
        anp_400G_SUB_TREE_MAC(2,  3),                                                                   \
        tai_400G_NODE_MAC(3, 8, 0),                                                                     \
        tai_400G_NODE_MAC(4, 8, 1),                                                                     \
        led_400G_NODE_MAC(5, 2),                                                                        \
        mac_pcs_400G_SUB_TREE_MAC(6, 2),                                                                \
        mif_400G_NODE_MAC(7, 3),                                                                        \
        sdw_SDW_INST_NODE_MAC(8, 4),                                                                    \
        sdw_SDW_INST_NODE_MAC(9, 5),                                                                    \
        anp_400G_SUB_TREE_MAC(10, 4),                                                                   \
        tai_400G_NODE_MAC(11, 9, 0),                                                                    \
        tai_400G_NODE_MAC(12, 9, 1),                                                                    \
        led_400G_NODE_MAC(13, 3),                                                                       \
        mac_pcs_400G_SUB_TREE_MAC(14, 3),                                                               \
        mif_400G_NODE_MAC(15, 4),                                                                       \
        sdw_SDW_INST_NODE_MAC(16, 6),                                                                   \
        sdw_SDW_INST_NODE_MAC(17, 7),                                                                   \
        ctsu_PCA_SUB_TREE_MAC(18, 2),                                                                   \
        brg_PCA_SUB_TREE_MAC(19, 2),                                                                    \
        pca_MSEC2_EGR_SUB_TREE_MAC(20, 0, 2),                                                           \
        pca_MSEC2_ING_SUB_TREE_MAC(21, 1, 2),                                                           \
        pca_MSEC3_EGR_SUB_TREE_MAC(20, 0, 2),                                                           \
        pca_MSEC3_ING_SUB_TREE_MAC(21, 1, 2),                                                           \
        lmu_PCA_SUB_TREE_MAC(24, 2),                                                                    \
        pzarb_PCA2I_MACRO_INST_NODE_MAC(25, 0),                                                         \
        pzarb_PCA2E_MACRO_INST_NODE_MAC(26, 1),                                                         \
        sff_PCA_SUB_TREE_MAC(27,  2),                                                                   \
        pca2_tai_NODE_MAC(28, 0),                                                                       \
        pca2_tai_NODE_MAC(29, 1),                                                                       \
        ctsu_PCA_SUB_TREE_MAC(30, 3),                                                                   \
        brg_PCA_SUB_TREE_MAC(31, 3)

#define cnm_grp_0_5_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                 \
    /* FuncUnits1IntsSum */                                                                             \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        0x000003f4, 0x000003f0,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MG_##_mgIndex##_FUNCTIONAL_1_INTERRUPTS_SUMMARY_CAUSE_GOP_UPLINK_TOP_PCA2_EIP_163E_INTERRUPT_CAUSE_INT_SUM_E,\
        PRV_CPSS_AC5P_MG_##_mgIndex##_FUNCTIONAL_1_INTERRUPTS_SUMMARY_CAUSE_TXQ_TOP_QFC0_INT_SUM_E,     \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 31, NULL, NULL},                                                 \
        pca_MSEC2_EGR_SUB_TREE_MAC(20, 2, 3),                                                           \
        pca_MSEC2_ING_SUB_TREE_MAC(21, 3, 3),                                                           \
        pca_MSEC3_EGR_SUB_TREE_MAC(20, 2, 3),                                                           \
        pca_MSEC3_ING_SUB_TREE_MAC(21, 3, 3),                                                           \
        lmu_PCA_SUB_TREE_MAC(5, 3),                                                                     \
        pzarb_PCA3I_MACRO_INST_NODE_MAC(6, 0),                                                          \
        pzarb_PCA3E_MACRO_INST_NODE_MAC(7, 1),                                                          \
        sff_PCA_SUB_TREE_MAC(8,  3),                                                                    \
        pca3_tai_NODE_MAC(9, 0),                                                                        \
        pca3_tai_NODE_MAC(10, 1),                                                                       \
        packetBuffer_SUB_TREE_MAC(11),                                                                  \
        dfx_server_sum_SUB_TREE_MAC(12),                                                                \
        pds0_SUB_TREE_MAC(13),                                                                          \
        pds1_SUB_TREE_MAC(14),                                                                          \
        pds2_SUB_TREE_MAC(15),                                                                          \
        pds3_SUB_TREE_MAC(16),                                                                          \
        pdx_NODE_MAC(17),                                                                               \
        pfcc_NODE_MAC(18),                                                                              \
        psi_NODE_MAC(19),                                                                               \
        qfc0_SUB_TREE_MAC(20),                                                                          \
        qfc1_SUB_TREE_MAC(21),                                                                          \
        qfc2_SUB_TREE_MAC(22),                                                                          \
        qfc3_SUB_TREE_MAC(23),                                                                          \
        sdq0_SUB_TREE_MAC(24),                                                                          \
        sdq1_SUB_TREE_MAC(25),                                                                          \
        sdq2_SUB_TREE_MAC(26),                                                                          \
        sdq3_SUB_TREE_MAC(27),                                                                          \
        txqs_tai_NODE_MAC(28, 0),                                                                    \
        txqs_tai_NODE_MAC(29, 1),                                                                    \
        txqs_1_tai_NODE_MAC(30, 0),                                                                    \
        txqs_1_tai_NODE_MAC(31, 1)

/*  cnm_grp_0_6_IntsSum_SUB_TREE_MAC */
#define cnm_grp_0_6_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                 \
    /* Ports1IntSum */                                                                                  \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        0x00000150, 0x00000154,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MG_##_mgIndex##_PORTS_1_INTERRUPTS_SUMMARY_CAUSE_GPIO_7_0_INTR_E,                 \
        PRV_CPSS_AC5P_MG_##_mgIndex##_PORTS_1_INTERRUPTS_SUMMARY_CAUSE_CNM_XOR2_IRQ_MEM1_E,             \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 6, NULL, NULL},                                                  \
        /* 0..7, GPIO_0_31_Interrupt Cause 0 */                                                         \
        {1, GT_TRUE, 0, prvCpssDrvDxExMxRunitGppIsrCall,                                                \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_AC5P_GPIO_REGFILE_GPIO_NUM_0_GPIO_0_INT_E,                                     \
                PRV_CPSS_AC5P_GPIO_REGFILE_GPIO_NUM_0_GPIO_7_INT_E,                                     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 8..15, GPIO_0_31_Interrupt Cause 0 */                                                        \
        {2, GT_TRUE, 8, prvCpssDrvDxExMxRunitGppIsrCall,                                                \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_AC5P_GPIO_REGFILE_GPIO_NUM_0_GPIO_8_INT_E,                                     \
                PRV_CPSS_AC5P_GPIO_REGFILE_GPIO_NUM_0_GPIO_15_INT_E,                                    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 16..23, GPIO_0_31_Interrupt Cause 0 */                                                       \
        {3, GT_TRUE, 16, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_AC5P_GPIO_REGFILE_GPIO_NUM_0_GPIO_16_INT_E,                                    \
                PRV_CPSS_AC5P_GPIO_REGFILE_GPIO_NUM_0_GPIO_23_INT_E,                                    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 24..31, GPIO_0_31_Interrupt Cause 0 */                                                       \
        {4, GT_TRUE, 24, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_AC5P_GPIO_REGFILE_GPIO_NUM_0_GPIO_24_INT_E,                                    \
                PRV_CPSS_AC5P_GPIO_REGFILE_GPIO_NUM_0_GPIO_31_INT_E,                                    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 32..39, GPIO_0_31_Interrupt Cause 1 */                                                       \
        {5, GT_TRUE, 32, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018154,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018158),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_AC5P_GPIO_REGFILE_GPIO_NUM_1_GPIO_0_INT_E,                                     \
                PRV_CPSS_AC5P_GPIO_REGFILE_GPIO_NUM_1_GPIO_7_INT_E,                                     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 40..47, GPIO_0_31_Interrupt Cause 1 */                                                       \
        {6, GT_TRUE, 40, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018154,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018158),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_AC5P_GPIO_REGFILE_GPIO_NUM_1_GPIO_8_INT_E,                                     \
                PRV_CPSS_AC5P_GPIO_REGFILE_GPIO_NUM_1_GPIO_15_INT_E,                                    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* hold 5 'native' sons  */
/*  cnm_grp_0_7_IntsSum_single_tile_SUB_TREE_MAC */
#define cnm_grp_0_7_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                 \
    /* Ports2ntSum */                                                                                   \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        0x00000158, 0x0000015C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MG_##_mgIndex##_PORTS_2_INTERRUPTS_SUMMARY_CAUSE_MG0_INT_OUT0_E,                  \
        PRV_CPSS_AC5P_MG_##_mgIndex##_PORTS_2_INTERRUPTS_SUMMARY_CAUSE_MG3_INT_OUT2_E,                  \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 5, NULL, NULL},                                                  \
        /*MG1*/                                                                                         \
        HAWK_INTERRUPTS_MG_1_MAC(hawk_MG1_int_sum, 1/*mgUnit*/),                                        \
        /*MG2*/                                                                                         \
        HAWK_INTERRUPTS_MG_x_MAC(hawk_MG2_int_sum, 2/*mgUnit*/),                                        \
        /*MG3*/                                                                                         \
        HAWK_INTERRUPTS_MG_x_MAC(hawk_MG3_int_sum, 3/*mgUnit*/),                                        \
        tai_master_SUB_TREE_MAC(30, 0),                                                                 \
        tai_master_SUB_TREE_MAC(31, 1)

#define HAWK_MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, numChild, _mgIndex)                                \
    /* Global Interrupt Cause */                                                                        \
    {bitIndexInCaller, GT_FALSE,  MG_UNIT(_mgIndex), NULL,                                              \
        0x00000030, 0x00000034,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5P_MG_##_mgIndex##_GLOBAL_INTERRUPTS_SUMMARY_CAUSE_PC_IE_INTERRUPT_SUMMARY_E,        \
        PRV_CPSS_AC5P_MG_##_mgIndex##_GLOBAL_INTERRUPTS_SUMMARY_CAUSE_MG1_INTERNAL_INTERRUPT_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, numChild, NULL, NULL}


#define HAWK_MG_Tx_SDMA_SUMMARY_MAC(bitIndexInCaller, _mgIndex)             \
        /* Tx SDMA  */                                                      \
        {bitIndexInCaller, GT_FALSE, MG_UNIT(_mgIndex), NULL,               \
            0x00002810, 0x00002818,                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                \
            PRV_CPSS_AC5P_MG_##_mgIndex##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_TX_BUFFER_QUEUE_0_E, \
            PRV_CPSS_AC5P_MG_##_mgIndex##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_TX_REJECT_0_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define HAWK_MG_Rx_SDMA_SUMMARY_MAC(bitIndexInCaller, _mgIndex)             \
        /* Rx SDMA  */                                                      \
        {bitIndexInCaller, GT_FALSE, MG_UNIT(_mgIndex), NULL,               \
            0x0000280C, 0x00002814,                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                \
            PRV_CPSS_AC5P_MG_##_mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_RX_BUFFER_QUEUE_0_E, \
            PRV_CPSS_AC5P_MG_##_mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_PACKET_CNT_OF_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define  HAWK_INTERRUPTS_MG_1_MAC(bitIndexInCaller, _mgId)                          \
    /* Global Interrupt Cause */                                                    \
    HAWK_MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, 3 /*numChild*/,_mgId/*mgId*/),     \
        /* MG internal Interrupt Cause */                                           \
        mg0InternalIntsSum_SUB_TREE_MAC(7, _mgId /*_mgIndex*/),                     \
        /* Tx SDMA  */                                                              \
        HAWK_MG_Tx_SDMA_SUMMARY_MAC(8, _mgId /*_mgIndex*/),                         \
        /* Rx SDMA  */                                                              \
        HAWK_MG_Rx_SDMA_SUMMARY_MAC(9, _mgId /*_mgIndex*/)


#define  HAWK_INTERRUPTS_MG_x_MAC(bitIndexInCaller, _mgId)                          \
    /* Global Interrupt Cause */                                                    \
    HAWK_MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, 3 /*numChild*/,_mgId/*mgId*/),     \
        /* MG internal Interrupt Cause */                                           \
        mg0InternalIntsSum_SUB_TREE_MAC(7, _mgId /*_mgIndex*/),                     \
        /* Tx SDMA  */                                                              \
        HAWK_MG_Tx_SDMA_SUMMARY_MAC(8, _mgId /*_mgIndex*/),                         \
        /* Rx SDMA  */                                                              \
        HAWK_MG_Rx_SDMA_SUMMARY_MAC(9, _mgId /*_mgIndex*/)

#define hawk_MG1_int_sum 4
#define hawk_MG2_int_sum 7
#define hawk_MG3_int_sum 10

static const PRV_CPSS_DRV_INTERRUPT_SCAN_STC hawkIntrScanArr[] =
{
    /* Global Interrupt Cause */
    HAWK_MG_GLOBAL_SUMMARY_MAC(0, 10 /*numChild*/, 0 /*_mgIndex*/),
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
        HAWK_MG_Tx_SDMA_SUMMARY_MAC(8, 0 /*_mgIndex*/),
        /* Rx SDMA  */
        HAWK_MG_Rx_SDMA_SUMMARY_MAC(9, 0 /*_mgIndex*/),
        /* DFX1 interrupts summary cause */
        cnm_grp_0_4_IntsSum_SUB_TREE_MAC(10, 0 /*_mgIndex*/),
        /* FuncUnits1IntsSum */
        cnm_grp_0_5_IntsSum_SUB_TREE_MAC(11, 0 /*_mgIndex*/),
#if 0
        /* Ports 1 Summary Interrupt Cause */
        cnm_grp_0_6_IntsSum_SUB_TREE_MAC(16, 0/*_mgIndex*/),
#endif
        /* Ports 2 Interrupt Summary */
        cnm_grp_0_7_IntsSum_SUB_TREE_MAC(17, 0 /*_mgIndex*/)
};

/* number of elements in the array of hawkIntrScanArr[] */
#define HAWK_NUM_ELEMENTS_IN_SCAN_TREE_CNS \
    NUM_ELEMENTS_IN_ARR_MAC(hawkIntrScanArr)


#define AC5P_NUM_MASK_REGISTERS_CNS  (PRV_CPSS_AC5P_LAST_INT_E / 32)

/* mark extData as per pipe per tile port interrupt */
/* _tile    --> the tileId 0, 2  */
/* _raven   --> local raven 0..3 */
/* _port    --> local port in the pipe 0..15 */
/* Cider ports 0..7 are mapped to 'global ports' :*/

#define MTI_PORT                0x1
#define MTI_SEG_PORT            0x2
#define USX_PORT                0x4
#define PORT_TYPE_OFFSET_BIT    16

#define HAWK_SET_EVENT_PER_400G_PORT_MAC(_instance, _postFix, _portNum)   \
    PRV_CPSS_AC5P_400G_MAC_INST_##_instance##_PORT_NUM_##_portNum##_##_postFix,      MTI_PORT << PORT_TYPE_OFFSET_BIT | ((_instance) * 8 + (_portNum))

#define HAWK_SET_EVENT_PER_SEG_PORT_MAC(_instance, _postFix, _portNum)   \
    PRV_CPSS_AC5P_400G_MAC_INST_##_instance##_SEG_PORT_NUM_##_portNum##_##_postFix,  MTI_SEG_PORT << PORT_TYPE_OFFSET_BIT | ((_instance) * 8 + (_portNum))

#define HAWK_SET_EVENT_PER_USX_PORT_MAC(_unit, _instance, _postFix, _portNum)   \
    PRV_CPSS_AC5P_USX_4_AND_8_MAC_MACRO_INST_##_unit##_USX_MAC_UNIT_INST_##_instance##_PORT_NUM_##_portNum##_   ##_postFix, USX_PORT << PORT_TYPE_OFFSET_BIT | ((_unit * 24) + (_instance) * 8 + (_portNum))

/* _postFix - must include the "_E" */
#define SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, _index)             \
    PRV_CPSS_AC5P_PCA_MACRO_INST_##_inst##_N_##_unit##_I_##_index##_##_postFix,  (((_inst) << 16) | ((31*(_unit)) + (_index)))

#define SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, _index)             \
    PRV_CPSS_AC5P_MSEC_INST_##_inst##_N_##_unit##_I_##_index##_##_postFix,  (((_inst) << 16) | ((31*(_unit)) + (_index)))

#define HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst, _unit)                     \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  0),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  1),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  2),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  3),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  4),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  5),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  6),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  7),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  8),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  9),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 10),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 11),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 12),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 13),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 14),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 15),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 16),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 17),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 18),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 19),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 20),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 21),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 22),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 23),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 24),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 25),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 26),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 27),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 28),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 29),                        \
    SET_PCA_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 30)

#define HAWK_PCA_LMU_EVENT_PER_INST_MAC(_postFix, _inst)                                 \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  0),                           \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  1),                           \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  2),                           \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  3),                           \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  4),                           \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  5),                           \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  6),                           \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  7),                           \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  8),                           \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  9),                           \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst, 10),                           \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst, 11),                           \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst, 12),                           \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst, 13),                           \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst, 14),                           \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst, 15),                           \
    HAWK_PCA_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst, 16)

#define HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst, _unit)                     \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  0),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  1),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  2),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  3),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  4),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  5),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  6),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  7),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  8),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit,  9),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 10),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 11),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 12),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 13),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 14),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 15),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 16),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 17),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 18),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 19),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 20),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 21),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 22),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 23),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 24),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 25),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 26),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 27),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 28),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 29),                        \
    SET_MSEC_LMU_EVENT_WITH_INDEX_MAC(_postFix, _inst, _unit, 30)

#define HAWK_MSEC_LMU_EVENT_PER_INST_MAC(_postFix, _inst)                                 \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  0),                           \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  1),                           \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  2),                           \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  3),                           \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  4),                           \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  5),                           \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  6),                           \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  7),                           \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  8),                           \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst,  9),                           \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst, 10),                           \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst, 11),                           \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst, 12),                           \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst, 13),                           \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst, 14),                           \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst, 15),                           \
    HAWK_MSEC_LMU_EVENT_PER_INST_UNIT_MAC(_postFix, _inst, 16)

/* _postFix - must include the "_E" */
#define HAWK_SET_EVENT_WITH_INDEX_MAC(_postFix, _index)             \
    PRV_CPSS_AC5P_##_postFix,                   _index

/* _postFix - must include the "_E" */
#define HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_postFix)                         \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5P_##_postFix)

/* _postFix - must include the "_E" */
#define HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_preFix, _index, _port, _postFix)     \
    PRV_CPSS_AC5P_##_preFix##_REG_##_index##_PORT_##_port##_##_postFix,   (_index * 16) + _port

/* _postFix - must include the "_E" */
#define SET_EVENT_INDEX_PER_SOURCE_PER_DIRECTION_PER_DP_MAC(_postFix, _prefix, _direction, _dp, _source)   \
    PRV_CPSS_AC5P_MSEC_##_prefix##_##_dp##_##_postFix,            (_source << 8 |(((_dp) * 2) + _direction))

#define SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, _prefix, _direction, _source)           \
    SET_EVENT_INDEX_PER_SOURCE_PER_DIRECTION_PER_DP_MAC(_postFix, _prefix, _direction, 0, _source), \
    SET_EVENT_INDEX_PER_SOURCE_PER_DIRECTION_PER_DP_MAC(_postFix, _prefix, _direction, 1, _source)

/* _postFix - must include the "_E" */
#define HAWK_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(_postFix, _source)  \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_164_INST, 0, _source),     \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, ING_164_INST, 1, _source)

/* _postFix - must include the "_E" */
#define HAWK_SET_EIP_163_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(_postFix, _source)  \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_163_INST, 0, _source),     \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, ING_163_INST, 1, _source)

/* _postFix - must include the "_E" */
#define SET_EVENT_INDEX_PER_DIRECTION_PER_DP_MAC(_postFix, _prefix, _direction, _dp)   \
    PRV_CPSS_AC5P_MSEC_##_prefix##_##_dp##_##_postFix,           (((_dp) << 1) + _direction)

#define SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, _prefix, _direction)       \
    SET_EVENT_INDEX_PER_DIRECTION_PER_DP_MAC(_postFix, _prefix, _direction, 0), \
    SET_EVENT_INDEX_PER_DIRECTION_PER_DP_MAC(_postFix, _prefix, _direction, 1)

/* _postFix - must include the "_E" */
#define HAWK_SET_EIP_164_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_164_INST, 0)

/* _postFix - must include the "_E" */
#define HAWK_SET_EIP_164_EVENT_SOURCE_PER_INGRESS_DIRECTION_ALL_DP_MAC(_postFix, _source)  \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, ING_164_INST, 1, _source)

/* _postFix - must include the "_E" */
#define HAWK_SET_EIP_66_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_66_INST, 0)

/* _postFix - must include the "_E" */
#define HAWK_SET_EIP_164_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_164_INST, 0),      \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, ING_164_INST, 1)

/* _postFix - must include the "_E" */
#define HAWK_SET_EIP_163_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_163_INST, 0),      \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, ING_163_INST, 1)

/* _postFix - must include the "_E" */
#define HAWK_SET_EIP_66_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_66_INST, 0),      \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, ING_66_INST, 1)

#define SET_EVENT_ALL_DIRECTIONS_PER_DP_MAC(_prefix, _index, _direction, _dp)   \
    PRV_CPSS_AC5P_MSEC_##_prefix##_##_dp##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_##_index##_E,  (_index << 8 |(((_dp) * 2) + _direction))

#define SET_EVENT_ALL_DIRECTIONS_ALL_DP_MAC(_prefix, _index, _direction)            \
    SET_EVENT_ALL_DIRECTIONS_PER_DP_MAC(_prefix, _index, _direction, 0),             \
    SET_EVENT_ALL_DIRECTIONS_PER_DP_MAC(_prefix, _index, _direction, 1)

#define HAWK_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_index)    \
    SET_EVENT_ALL_DIRECTIONS_ALL_DP_MAC(EGR_66_INST, _index, 0),             \
    SET_EVENT_ALL_DIRECTIONS_ALL_DP_MAC(ING_66_INST, _index, 1)

#define HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_index, _postFix)          \
    HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  0, _postFix),            \
    HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  1, _postFix),            \
    HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  2, _postFix),            \
    HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  3, _postFix),            \
    HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  4, _postFix),            \
    HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  5, _postFix),            \
    HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  6, _postFix),            \
    HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  7, _postFix),            \
    HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  8, _postFix),            \
    HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index,  9, _postFix),            \
    HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index, 10, _postFix),            \
    HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index, 11, _postFix),            \
    HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index, 12, _postFix),            \
    HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index, 13, _postFix),            \
    HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index, 14, _postFix),            \
    HAWK_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(EQ, _index, 15, _postFix)


#define HAWK_SET_EVENT_PER_ALL_PORTS_ALL_REGISTERS_MAC(_postFix)                             \
    HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 1, _postFix),                           \
    HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 2, _postFix),                           \
    HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 3, _postFix),                           \
    HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 4, _postFix),                           \
    HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 5, _postFix),                           \
    HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 6, _postFix),                           \
    HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 7, _postFix),                           \
    HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 8, _postFix),                           \
    HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC( 9, _postFix),                           \
    HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(10, _postFix),                           \
    HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(11, _postFix),                           \
    HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(12, _postFix),                           \
    HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(13, _postFix),                           \
    HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(14, _postFix),                           \
    HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(15, _postFix),                           \
    HAWK_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(16, _postFix)

#define HAWK_SET_EVENT_PER_UNIT_PER_INSTANCE_USX_PORTS_MAC(unit, instance, _postFix)            \
    HAWK_SET_EVENT_PER_USX_PORT_MAC(unit, instance, _postFix, 0),                               \
    HAWK_SET_EVENT_PER_USX_PORT_MAC(unit, instance, _postFix, 1),                               \
    HAWK_SET_EVENT_PER_USX_PORT_MAC(unit, instance, _postFix, 2),                               \
    HAWK_SET_EVENT_PER_USX_PORT_MAC(unit, instance, _postFix, 3),                               \
    HAWK_SET_EVENT_PER_USX_PORT_MAC(unit, instance, _postFix, 4),                               \
    HAWK_SET_EVENT_PER_USX_PORT_MAC(unit, instance, _postFix, 5),                               \
    HAWK_SET_EVENT_PER_USX_PORT_MAC(unit, instance, _postFix, 6),                               \
    HAWK_SET_EVENT_PER_USX_PORT_MAC(unit, instance, _postFix, 7)


#define HAWK_SET_INDEX_EQUAL_EVENT_PER_USX_PORTS(unit,_postFix)                                 \
    HAWK_SET_EVENT_PER_UNIT_PER_INSTANCE_USX_PORTS_MAC(unit, 0, _postFix),                      \
    HAWK_SET_EVENT_PER_UNIT_PER_INSTANCE_USX_PORTS_MAC(unit, 1, _postFix),                      \
    HAWK_SET_EVENT_PER_UNIT_PER_INSTANCE_USX_PORTS_MAC(unit, 2, _postFix)

#define HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(index,_postFix)                            \
    HAWK_SET_EVENT_PER_400G_PORT_MAC(index, _postFix,      0),                              \
    HAWK_SET_EVENT_PER_400G_PORT_MAC(index, _postFix,      1),                              \
    HAWK_SET_EVENT_PER_400G_PORT_MAC(index, _postFix,      2),                              \
    HAWK_SET_EVENT_PER_400G_PORT_MAC(index, _postFix,      3),                              \
    HAWK_SET_EVENT_PER_400G_PORT_MAC(index, _postFix,      4),                              \
    HAWK_SET_EVENT_PER_400G_PORT_MAC(index, _postFix,      5),                              \
    HAWK_SET_EVENT_PER_400G_PORT_MAC(index, _postFix,      6),                              \
    HAWK_SET_EVENT_PER_400G_PORT_MAC(index, _postFix,      7)

#define HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(index,_postFix)                        \
    HAWK_SET_EVENT_PER_SEG_PORT_MAC(index,_postFix,  0),                                \
    HAWK_SET_EVENT_PER_SEG_PORT_MAC(index,_postFix,  1)

#define SINGLE_MG_EVENT_MAC(_postFix, _mgUnitId, _index)                                        \
    PRV_CPSS_AC5X_MG_##_mgUnitId##_MG_INTERNAL_INTERRUPT_CAUSE_##_postFix, (((_mgUnitId)*8) + _index)

#define ALL_MG_EVENTS_MAC(_postFix, _index)                                                   \
    SINGLE_MG_EVENT_MAC(_postFix, 0, _index),                                                 \
    SINGLE_MG_EVENT_MAC(_postFix, 1, _index),                                                 \
    SINGLE_MG_EVENT_MAC(_postFix, 2, _index)

#define SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, _localQueue)  \
    PRV_CPSS_AC5P_MG_##_mgUnitId##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_##type##_##_localQueue##_E,  (_mgUnitId*8 + _localQueue)

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
    SINGLE_MG_TX_SDMA_QUEUES_MAC(type, 1/*mg1*/),                      \
    SINGLE_MG_TX_SDMA_QUEUES_MAC(type, 2/*mg0*/),                      \
    SINGLE_MG_TX_SDMA_QUEUES_MAC(type, 3/*mg1*/)


/* connect unified event of 'RX SDMA' to their interrupt */
/* specific unified event to specific interrupt */
#define SINGLE_RX_SDMA_INT_MAC(type, local_mgUnitId, globalQueue, localQueue) \
    CPSS_PP_##type##_QUEUE##globalQueue##_E,                              \
        PRV_CPSS_AC5P_MG_##local_mgUnitId##_RECEIVE_SDMA_INTERRUPT_CAUSE0_##type##_QUEUE_##localQueue##_E, globalQueue, \
    MARK_END_OF_UNI_EV_CNS



#define SINGLE_MG_RX_SDMA_MAC(mgIndex) \
    PRV_CPSS_AC5P_MG_##mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_RESOURCE_ERROR_CNT_OF_E,     ((mgIndex)*6)+0, \
    PRV_CPSS_AC5P_MG_##mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_BYTE_CNT_OF_E,               ((mgIndex)*6)+1, \
    PRV_CPSS_AC5P_MG_##mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_PACKET_CNT_OF_E,             ((mgIndex)*6)+2


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
    SINGLE_RX_SDMA_INT_MAC(type,  1/*local mg*/,  8/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  1/*local mg*/,  9/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  1/*local mg*/, 10/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  1/*local mg*/, 11/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  1/*local mg*/, 12/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  1/*local mg*/, 13/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  1/*local mg*/, 14/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  1/*local mg*/, 15/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
/* global MG 2 */
#define MG2_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,  2/*local mg*/, 16/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  2/*local mg*/, 17/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  2/*local mg*/, 18/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  2/*local mg*/, 19/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  2/*local mg*/, 20/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  2/*local mg*/, 21/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  2/*local mg*/, 22/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  2/*local mg*/, 23/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
/* global MG 3 */
#define MG3_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,  3/*local mg*/, 24/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  3/*local mg*/, 25/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  3/*local mg*/, 26/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  3/*local mg*/, 27/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  3/*local mg*/, 28/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  3/*local mg*/, 29/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  3/*local mg*/, 30/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  3/*local mg*/, 31/*globalQueue*/,7/*localQueue*/)


/* connect unified event of 'RX SDMA' to their interrupt */
/* ALL MGs [0..15] */
#define ALL_MGS_RX_SDMA_INT_MAC(type)    \
    MG0_RX_SDMA_INT_MAC(type),           \
    MG1_RX_SDMA_INT_MAC(type),           \
    MG2_RX_SDMA_INT_MAC(type),           \
    MG3_RX_SDMA_INT_MAC(type)

/* Maximal PPN events */
#define PPN_EVENT_TOTAL    40

#define SET_PPN_EVENT_MAC(_ppg, _ppn)   \
    PRV_CPSS_AC5P_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_HOST_UNMAPPED_ACCESS_E             , (12 + (PPN_EVENT_TOTAL * 0) + (_ppg * 10) + (_ppn)),  \
    PRV_CPSS_AC5P_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_CORE_UNMAPPED_ACCESS_E             , (12 + (PPN_EVENT_TOTAL * 1) + (_ppg * 10) + (_ppn)),  \
    PRV_CPSS_AC5P_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_NEAR_EDGE_IMEM_ACCESS_E            , (12 + (PPN_EVENT_TOTAL * 2) + (_ppg * 10) + (_ppn)),  \
    PRV_CPSS_AC5P_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_DOORBELL_INTERRUPT_E               , (12 + (PPN_EVENT_TOTAL * 3) + (_ppg * 10) + (_ppn))

#define SET_PPG_EVENT_PER_10_PPN_MAC(_ppg)\
    SET_PPN_EVENT_MAC(_ppg, 0),          \
    SET_PPN_EVENT_MAC(_ppg, 1),          \
    SET_PPN_EVENT_MAC(_ppg, 2),          \
    SET_PPN_EVENT_MAC(_ppg, 3),          \
    SET_PPN_EVENT_MAC(_ppg, 4),          \
    SET_PPN_EVENT_MAC(_ppg, 5),          \
    SET_PPN_EVENT_MAC(_ppg, 6),          \
    SET_PPN_EVENT_MAC(_ppg, 7),          \
    SET_PPN_EVENT_MAC(_ppg, 8),          \
    SET_PPN_EVENT_MAC(_ppg, 9)


#define SET_PHA_EVENTS_MAC  \
   /* PHA/PPA */                         \
   PRV_CPSS_AC5P_PHA_PHA_INTERNAL_ERROR_CAUSE_PHA_BAD_ADDRESS_ERROR_E,                                      0,  \
   PRV_CPSS_AC5P_PHA_PHA_INTERNAL_ERROR_CAUSE_PHA_TABLE_ACCESS_OVERLAP_ERROR_E,                             1,  \
   PRV_CPSS_AC5P_PHA_PHA_INTERNAL_ERROR_CAUSE_HEADER_SIZE_VIOLATION_E,                                      2,  \
   PRV_CPSS_AC5P_PHA_PHA_INTERNAL_ERROR_CAUSE_PPA_CLOCK_DOWN_VIOLATION_E,                                   3,  \
   /* PPGs */                                                                                                   \
   PRV_CPSS_AC5P_PPG_0_PPG_INTERNAL_ERROR_CAUSE_PPG_BAD_ADDRESS_ERROR_E,                                    4,  \
   PRV_CPSS_AC5P_PPG_1_PPG_INTERNAL_ERROR_CAUSE_PPG_BAD_ADDRESS_ERROR_E,                                    5,  \
   PRV_CPSS_AC5P_PPG_2_PPG_INTERNAL_ERROR_CAUSE_PPG_BAD_ADDRESS_ERROR_E,                                    6,  \
   PRV_CPSS_AC5P_PPG_3_PPG_INTERNAL_ERROR_CAUSE_PPG_BAD_ADDRESS_ERROR_E,                                    7,  \
                                                                                                                \
   PRV_CPSS_AC5P_PPG_0_PPG_INTERNAL_ERROR_CAUSE_PPG_TABLE_ACCESS_OVERLAP_ERROR_E,                           8,  \
   PRV_CPSS_AC5P_PPG_1_PPG_INTERNAL_ERROR_CAUSE_PPG_TABLE_ACCESS_OVERLAP_ERROR_E,                           9,  \
   PRV_CPSS_AC5P_PPG_2_PPG_INTERNAL_ERROR_CAUSE_PPG_TABLE_ACCESS_OVERLAP_ERROR_E,                           10, \
   PRV_CPSS_AC5P_PPG_3_PPG_INTERNAL_ERROR_CAUSE_PPG_TABLE_ACCESS_OVERLAP_ERROR_E,                           11, \
   /* PPNs */                                                                                                   \
   SET_PPG_EVENT_PER_10_PPN_MAC(0),                                                                             \
   SET_PPG_EVENT_PER_10_PPN_MAC(1),                                                                             \
   SET_PPG_EVENT_PER_10_PPN_MAC(2),                                                                             \
   SET_PPG_EVENT_PER_10_PPN_MAC(3)

#define SET_HR_TRSH_EVENT_MAC(_unit, _index, _regIndex, _port, _tc)             \
    PRV_CPSS_AC5P_TXQS_INST_##_unit##_UNIT_INST_##_index##_REGISTER_INDEX_##_regIndex##_PORT_##_port##_TC_##_tc##_CROSSED_HR_THRESHOLD_E, (_port * 8) + _tc

#define SET_HR_TRSH_EVENT_ALL_TC_MAC(_unit, _index, _regIndex, _port)           \
    SET_HR_TRSH_EVENT_MAC(_unit, _index, _regIndex, _port, 0),                  \
    SET_HR_TRSH_EVENT_MAC(_unit, _index, _regIndex, _port, 1),                  \
    SET_HR_TRSH_EVENT_MAC(_unit, _index, _regIndex, _port, 2),                  \
    SET_HR_TRSH_EVENT_MAC(_unit, _index, _regIndex, _port, 3),                  \
    SET_HR_TRSH_EVENT_MAC(_unit, _index, _regIndex, _port, 4),                  \
    SET_HR_TRSH_EVENT_MAC(_unit, _index, _regIndex, _port, 5),                  \
    SET_HR_TRSH_EVENT_MAC(_unit, _index, _regIndex, _port, 6),                  \
    SET_HR_TRSH_EVENT_MAC(_unit, _index, _regIndex, _port, 7)

#define HAWK_SET_HR_TRSH_EVENT_ALL_PORTS_MAC                                    \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 0,   0),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 0,   1),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 0,   2),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 1,   3),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 1,   4),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 1,   5),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 2,   6),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 2,   7),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 2,   8),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 3,   9),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 3,  10),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 3,  11),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 4,  12),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 4,  13),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 4,  14),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 5,  15),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 5,  16),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 5,  17),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 6,  18),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 6,  19),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 6,  20),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 7,  21),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 7,  22),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 7,  23),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 8,  24),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 8,  25),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 0, 8,  26),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 0,  27),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 0,  28),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 0,  29),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 1,  30),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 1,  31),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 1,  32),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 2,  33),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 2,  34),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 2,  35),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 3,  36),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 3,  37),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 3,  38),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 4,  39),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 4,  40),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 4,  41),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 5,  42),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 5,  43),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 5,  44),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 6,  45),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 6,  46),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 6,  47),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 7,  48),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 7,  49),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 7,  50),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 8,  51),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 8,  52),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(0, 1, 8,  53),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 0,  54),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 0,  55),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 0,  56),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 1,  57),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 1,  58),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 1,  59),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 2,  60),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 2,  61),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 2,  62),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 3,  63),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 3,  64),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 3,  65),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 4,  66),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 4,  67),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 4,  68),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 5,  69),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 5,  70),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 5,  71),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 6,  72),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 6,  73),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 6,  74),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 7,  75),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 7,  76),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 7,  77),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 8,  78),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 8,  79),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 0, 8,  80),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 0,  81),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 0,  82),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 0,  83),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 1,  84),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 1,  85),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 1,  86),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 2,  87),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 2,  88),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 2,  89),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 3,  90),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 3,  91),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 3,  92),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 4,  93),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 4,  94),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 4,  95),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 5,  96),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 5,  97),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 5,  98),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 6,  99),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 6, 100),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 6, 101),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 7, 102),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 7, 103),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 7, 104),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 8, 105),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 8, 106),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(1, 1, 8, 107)

/* _postFix - must include the "_E" */
#define SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(_postFix, _unit, _instance, _index)            \
    PRV_CPSS_AC5P_##_unit##_UNIT_INST_##_instance##_##_postFix,  _index

/* _postFix - must include the "_E" */
#define SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(_unit, _index)            \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E, _unit, 0, (_index)),    \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E, _unit, 1, (_index+1))

/* _postFix - must include the "_E" */
#define SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(_unit, _index)            \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_GENERATION_INT_E, _unit, 0, (_index)),    \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_GENERATION_INT_E, _unit, 1, (_index+1))

#define CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, _channel, _extData, _port)                          \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CHANNEL_##_channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_0_##_postfix, (((_port) << 8) | (_extData))

#define CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, _channel, _extData, _port)                          \
        PRV_CPSS_AC5P_PCA_INST_##_instance##_CHANNEL_##_channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_1_##_postfix, (((_port) << 8) | (_extData))

#define CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, _channel, _extData, _port)                          \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CHANNEL_##_channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_0_##_postfix, (((_port) << 8) | (_extData))

#define CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, _channel, _extData, _port)                          \
        PRV_CPSS_AC5P_MSEC_INST_##_instance##_CHANNEL_##_channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_1_##_postfix, (((_port) << 8) | (_extData))

#define SET_EVENT_GROUP_0_CHANNEL_CTSU_16_PCA_MAC(_postfix, _instance, _extData)                                                                \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  0, _extData, ( 0 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  1, _extData, ( 1 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  2, _extData, ( 2 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  3, _extData, ( 3 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  4, _extData, ( 4 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  5, _extData, ( 5 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  6, _extData, ( 6 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  7, _extData, ( 7 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  8, _extData, ( 8 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  9, _extData, ( 9 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 10, _extData, (10 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 11, _extData, (11 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 12, _extData, (12 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 13, _extData, (13 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 14, _extData, (14 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 15, _extData, (15 + (_instance * 16)))

#define SET_EVENT_GROUP_0_INSTANCE_CTSU_32_PCA_MAC(_postfix, _extData)                                                                          \
        SET_EVENT_GROUP_0_CHANNEL_CTSU_16_PCA_MAC(_postfix, 2, _extData),                                                                       \
        SET_EVENT_GROUP_0_CHANNEL_CTSU_16_PCA_MAC(_postfix, 3, _extData)

#define SET_EVENT_GROUP_1_CHANNEL_CTSU_16_PCA_MAC(_postfix, _instance, _extData)                                                                \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 16, _extData, (16 + (_instance * 16))),              \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 17, _extData, (17 + (_instance * 16))),              \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 18, _extData, (18 + (_instance * 16))),              \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 19, _extData, (19 + (_instance * 16))),              \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 20, _extData, (20 + (_instance * 16))),              \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 21, _extData, (21 + (_instance * 16))),              \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 22, _extData, (22 + (_instance * 16))),              \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 23, _extData, (23 + (_instance * 16))),              \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 24, _extData, (24 + (_instance * 16))),              \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 25, _extData, (25 + (_instance * 16))),              \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 26, _extData, (26 + (_instance * 16))),              \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 27, _extData, (27 + (_instance * 16))),              \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 28, _extData, (28 + (_instance * 16))),              \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 29, _extData, (29 + (_instance * 16))),              \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 30, _extData, (30 + (_instance * 16))),              \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 31, _extData, (31 + (_instance * 16)))

#define SET_EVENT_GROUP_1_INSTANCE_CTSU_32_PCA_MAC(_postfix, _extData)                                                                          \
        SET_EVENT_GROUP_1_CHANNEL_CTSU_16_PCA_MAC(_postfix, 2, _extData),                                                                       \
        SET_EVENT_GROUP_1_CHANNEL_CTSU_16_PCA_MAC(_postfix, 3, _extData)

#define SET_EVENT_CTSU_PCA_MAC(_postfix)                                                                                                        \
        SET_EVENT_GROUP_0_INSTANCE_CTSU_32_PCA_MAC(_postfix, GTS_INT_EXT_PARAM_MAC(1,0,1)),                                                     \
        SET_EVENT_GROUP_1_INSTANCE_CTSU_32_PCA_MAC(_postfix, GTS_INT_EXT_PARAM_MAC(1,1,1))

#define SET_EVENT_GROUP_0_CHANNEL_CTSU_16_MSEC_MAC(_postfix, _instance, _extData)                                                               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  0, _extData, ( 0 + (_instance * 16))),            \
        CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  1, _extData, ( 1 + (_instance * 16))),            \
        CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  2, _extData, ( 2 + (_instance * 16))),            \
        CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  3, _extData, ( 3 + (_instance * 16))),            \
        CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  4, _extData, ( 4 + (_instance * 16))),            \
        CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  5, _extData, ( 5 + (_instance * 16))),            \
        CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  6, _extData, ( 6 + (_instance * 16))),            \
        CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  7, _extData, ( 7 + (_instance * 16))),            \
        CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  8, _extData, ( 8 + (_instance * 16))),            \
        CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance,  9, _extData, ( 9 + (_instance * 16))),            \
        CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 10, _extData, (10 + (_instance * 16))),            \
        CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 11, _extData, (11 + (_instance * 16))),            \
        CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 12, _extData, (12 + (_instance * 16))),            \
        CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 13, _extData, (13 + (_instance * 16))),            \
        CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 14, _extData, (14 + (_instance * 16))),            \
        CTSU_INSTANCE_CHANNEL_QUEUE0_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _instance, 15, _extData, (15 + (_instance * 16)))

#define SET_EVENT_GROUP_0_INSTANCE_CTSU_32_MSEC_MAC(_postfix, _extData)                                                                         \
        SET_EVENT_GROUP_0_CHANNEL_CTSU_16_MSEC_MAC(_postfix, 0, _extData),                                                                      \
        SET_EVENT_GROUP_0_CHANNEL_CTSU_16_MSEC_MAC(_postfix, 1, _extData)

#define SET_EVENT_GROUP_1_CHANNEL_CTSU_16_MSEC_MAC(_postfix, _instance, _extData)                                                               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 16, _extData, (16 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 17, _extData, (17 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 18, _extData, (18 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 19, _extData, (19 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 20, _extData, (20 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 21, _extData, (21 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 22, _extData, (22 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 23, _extData, (23 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 24, _extData, (24 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 25, _extData, (25 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 26, _extData, (26 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 27, _extData, (27 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 28, _extData, (28 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 29, _extData, (29 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 30, _extData, (30 + (_instance * 16))),             \
        CTSU_INSTANCE_CHANNEL_QUEUE1_MSEC_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,_instance, 31, _extData, (31 + (_instance * 16)))

#define SET_EVENT_GROUP_1_INSTANCE_CTSU_32_MSEC_MAC(_postfix, _extData)                                                                          \
        SET_EVENT_GROUP_1_CHANNEL_CTSU_16_MSEC_MAC(_postfix, 0, _extData),                                                                       \
        SET_EVENT_GROUP_1_CHANNEL_CTSU_16_MSEC_MAC(_postfix, 1, _extData)

#define SET_EVENT_CTSU_MSEC_MAC(_postfix)                                                                               \
    SET_EVENT_GROUP_0_INSTANCE_CTSU_32_MSEC_MAC(_postfix, GTS_INT_EXT_PARAM_MAC(1,0,1)),                                \
    SET_EVENT_GROUP_1_INSTANCE_CTSU_32_MSEC_MAC(_postfix, GTS_INT_EXT_PARAM_MAC(1,1,1))


#define HAWK_COMMON_EVENTS                                                                                              \
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
    PRV_CPSS_AC5P_ERMRK_ERMRK_INTERRUPT_CAUSE_INGRESS_TIMESTAMP_QUEUE0_FULL_E,  0,                                      \
    PRV_CPSS_AC5P_ERMRK_ERMRK_INTERRUPT_CAUSE_EGRESS_TIMESTAMP_QUEUE0_FULL_E,   1,                                      \
    PRV_CPSS_AC5P_ERMRK_ERMRK_INTERRUPT_CAUSE_INGRESS_TIMESTAMP_QUEUE1_FULL_E,  2,                                      \
    PRV_CPSS_AC5P_ERMRK_ERMRK_INTERRUPT_CAUSE_EGRESS_TIMESTAMP_QUEUE1_FULL_E,   3,                                      \
    SET_EVENT_CTSU_MSEC_MAC(OVERFLOW_INTERRUPT_E),                                                                      \
    SET_EVENT_CTSU_PCA_MAC(OVERFLOW_INTERRUPT_E),                                                                       \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,                                                                               \
    PRV_CPSS_AC5P_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_INGRESS_TIMESTAMP_Q0_E,  0,                                           \
    PRV_CPSS_AC5P_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_EGRESS_TIMESTAMP_Q0_E,   1,                                           \
    PRV_CPSS_AC5P_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_INGRESS_TIMESTAMP_Q1_E,  2,                                           \
    PRV_CPSS_AC5P_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_EGRESS_TIMESTAMP_Q1_E,   3,                                           \
    SET_EVENT_CTSU_MSEC_MAC(NEW_ENTRY_INTERRUPT_E),                                                                     \
    SET_EVENT_CTSU_PCA_MAC(NEW_ENTRY_INTERRUPT_E),                                                                      \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E,                                                                                    \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(TAI_MASTER,                    0),                                     \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(MSEC_INST_16,                  2),                                     \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(PCA_INST_13,                   4),                                     \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(PCA_INST_14,                   6),                                     \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(PCA_INST_15,                   8),                                     \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(USX_MAC_MACRO_INST_11,         10),                                    \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(USX_MAC_MACRO_INST_12,         12),                                    \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(CPU_AND_400G_MAC_INST_6,       14),                                    \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(CPU_AND_400G_MAC_INST_7,       16),                                    \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(CPU_AND_400G_MAC_INST_8,       18),                                    \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(CPU_AND_400G_MAC_INST_9,       20),                                    \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(CPU_AND_400G_MAC_INST_10,      22),                                    \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(PHA_AND_EPCL_HA_INST_4,        24),                                    \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(PHA_AND_EPCL_HA_INST_5,        26),                                    \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(MNG_AND_TTI_INST_2,            28),                                    \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(MNG_AND_TTI_INST_3,            30),                                    \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(TXQS_INST_0,                   32),                                    \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(TXQS_INST_1,                   34),                                    \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PTP_TAI_GENERATION_E,                                                                                          \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(TAI_MASTER,                         0),                                     \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(MSEC_INST_16,                       2),                                     \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(PCA_INST_13,                        4),                                     \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(PCA_INST_14,                        6),                                     \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(PCA_INST_15,                        8),                                     \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(USX_MAC_MACRO_INST_11,              10),                                    \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(USX_MAC_MACRO_INST_12,              12),                                    \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(CPU_AND_400G_MAC_INST_6,            14),                                    \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(CPU_AND_400G_MAC_INST_7,            16),                                    \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(CPU_AND_400G_MAC_INST_8,            18),                                    \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(CPU_AND_400G_MAC_INST_9,            20),                                    \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(CPU_AND_400G_MAC_INST_10,           22),                                    \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(PHA_AND_EPCL_HA_INST_4,             24),                                    \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(PHA_AND_EPCL_HA_INST_5,             26),                                    \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(MNG_AND_TTI_INST_2,                 28),                                    \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(MNG_AND_TTI_INST_3,                 30),                                    \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(TXQS_INST_0,                        32),                                    \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(TXQS_INST_1,                        34),                                    \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 /* 128 uniEvents for 128 queues , extData = queueId */                                                                 \
 ALL_MGS_RX_SDMA_INT_MAC(RX_BUFFER),                                                                                    \
 /* 128 uniEvents for 128 queues , extData = queueId */                                                                 \
 ALL_MGS_RX_SDMA_INT_MAC(RX_ERROR),                                                                                     \
                                                                                                                        \
 CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,                                                                                       \
    /* USX ports */                                                                                                     \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_USX_PORTS(0, MAC_TX_UNDERFLOW_E),                                                    \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_USX_PORTS(1, MAC_TX_UNDERFLOW_E),                                                    \
    /* MTI total ports 0..255 */                                                                                        \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(0, MAC_TX_UNDERFLOW_E),                                                    \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(1, MAC_TX_UNDERFLOW_E),                                                    \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(2, MAC_TX_UNDERFLOW_E),                                                    \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(3, MAC_TX_UNDERFLOW_E),                                                    \
    /* seg ports*/                                                                                                      \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(0, MAC_TX_UNDERFLOW_E),                                                    \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(1, MAC_TX_UNDERFLOW_E),                                                    \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(2, MAC_TX_UNDERFLOW_E),                                                    \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(3, MAC_TX_UNDERFLOW_E),                                                    \
                                                                                                                        \
    /* CPU ports */                                                                                                     \
    PRV_CPSS_AC5P_CPU_MAC_PORT_INTERRUPT_CAUSE_TX_UNDERFLOW_E,    105,                                                  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PORT_TX_FIFO_OVERRUN_E,                                                                                        \
    /* USX ports */                                                                                                     \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_USX_PORTS(0, MAC_TX_OVR_ERR_E),                                                      \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_USX_PORTS(1, MAC_TX_OVR_ERR_E),                                                      \
    /* MTI total ports 0..255 */                                                                                        \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(0, EMAC_TX_OVR_ERR_E),                                                     \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(1, EMAC_TX_OVR_ERR_E),                                                     \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(2, EMAC_TX_OVR_ERR_E),                                                     \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(3, EMAC_TX_OVR_ERR_E),                                                     \
    /* seg ports*/                                                                                                      \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(0, MAC_TX_OVR_ERR_E),                                                      \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(1, MAC_TX_OVR_ERR_E),                                                      \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(2, MAC_TX_OVR_ERR_E),                                                      \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(3, MAC_TX_OVR_ERR_E),                                                      \
    /* CPU ports */                                                                                                     \
    PRV_CPSS_AC5P_CPU_MAC_PORT_INTERRUPT_CAUSE_TX_OVR_ERR_E,    105,                                                    \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PORT_RX_FIFO_OVERRUN_E,                                                                                        \
    /* USX ports */                                                                                                     \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_USX_PORTS(0, RX_OVERRUN_E),                                                          \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_USX_PORTS(1, RX_OVERRUN_E),                                                          \
    /* MTI total ports 0..255 */                                                                                        \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(0, EMAC_RX_OVERRUN_E),                                                     \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(1, EMAC_RX_OVERRUN_E),                                                     \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(2, EMAC_RX_OVERRUN_E),                                                     \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(3, EMAC_RX_OVERRUN_E),                                                     \
    /* seg ports*/                                                                                                      \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(0, RX_OVERRUN_E),                                                          \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(1, RX_OVERRUN_E),                                                          \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(2, RX_OVERRUN_E),                                                          \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(3, RX_OVERRUN_E),                                                          \
    /* CPU ports */                                                                                                     \
    PRV_CPSS_AC5P_CPU_MAC_PORT_INTERRUPT_CAUSE_RX_OVERRUN_E,    105,                                                    \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_LMU_LATENCY_EXCEED_THRESHOLD_E,                                                                                \
    HAWK_MSEC_LMU_EVENT_PER_INST_MAC(LMU_LATENCY_OVER_THRESHOLD_E, 0),                                                  \
    HAWK_MSEC_LMU_EVENT_PER_INST_MAC(LMU_LATENCY_OVER_THRESHOLD_E, 1),                                                  \
    HAWK_PCA_LMU_EVENT_PER_INST_MAC(LMU_LATENCY_OVER_THRESHOLD_E, 2),                                                   \
    HAWK_PCA_LMU_EVENT_PER_INST_MAC(LMU_LATENCY_OVER_THRESHOLD_E, 3),                                                   \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_CNC_WRAPAROUND_BLOCK_E,                                                                                        \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_0_WRAPAROUND_E,  (16*0 +  0)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_1_WRAPAROUND_E,  (16*0 +  1)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_2_WRAPAROUND_E,  (16*0 +  2)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_3_WRAPAROUND_E,  (16*0 +  3)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_4_WRAPAROUND_E,  (16*0 +  4)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_5_WRAPAROUND_E,  (16*0 +  5)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_6_WRAPAROUND_E,  (16*0 +  6)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_7_WRAPAROUND_E,  (16*0 +  7)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_8_WRAPAROUND_E,  (16*0 +  8)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_9_WRAPAROUND_E,  (16*0 +  9)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_10_WRAPAROUND_E, (16*0 + 10)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_11_WRAPAROUND_E, (16*0 + 11)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_12_WRAPAROUND_E, (16*0 + 12)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_13_WRAPAROUND_E, (16*0 + 13)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_14_WRAPAROUND_E, (16*0 + 14)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_15_WRAPAROUND_E, (16*0 + 15)),  \
                                                                                                                        \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_1_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_0_WRAPAROUND_E,  (16*1 +  0)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_1_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_1_WRAPAROUND_E,  (16*1 +  1)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_1_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_2_WRAPAROUND_E,  (16*1 +  2)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_1_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_3_WRAPAROUND_E,  (16*1 +  3)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_1_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_4_WRAPAROUND_E,  (16*1 +  4)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_1_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_5_WRAPAROUND_E,  (16*1 +  5)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_1_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_6_WRAPAROUND_E,  (16*1 +  6)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_1_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_7_WRAPAROUND_E,  (16*1 +  7)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_1_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_8_WRAPAROUND_E,  (16*1 +  8)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_1_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_9_WRAPAROUND_E,  (16*1 +  9)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_1_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_10_WRAPAROUND_E, (16*1 + 10)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_1_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_11_WRAPAROUND_E, (16*1 + 11)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_1_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_12_WRAPAROUND_E, (16*1 + 12)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_1_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_13_WRAPAROUND_E, (16*1 + 13)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_1_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_14_WRAPAROUND_E, (16*1 + 14)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_1_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_15_WRAPAROUND_E, (16*1 + 15)),  \
\
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_2_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_0_WRAPAROUND_E,  (16*2 +  0)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_2_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_1_WRAPAROUND_E,  (16*2 +  1)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_2_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_2_WRAPAROUND_E,  (16*2 +  2)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_2_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_3_WRAPAROUND_E,  (16*2 +  3)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_2_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_4_WRAPAROUND_E,  (16*2 +  4)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_2_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_5_WRAPAROUND_E,  (16*2 +  5)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_2_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_6_WRAPAROUND_E,  (16*2 +  6)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_2_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_7_WRAPAROUND_E,  (16*2 +  7)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_2_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_8_WRAPAROUND_E,  (16*2 +  8)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_2_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_9_WRAPAROUND_E,  (16*2 +  9)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_2_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_10_WRAPAROUND_E, (16*2 + 10)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_2_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_11_WRAPAROUND_E, (16*2 + 11)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_2_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_12_WRAPAROUND_E, (16*2 + 12)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_2_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_13_WRAPAROUND_E, (16*2 + 13)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_2_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_14_WRAPAROUND_E, (16*2 + 14)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_2_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_15_WRAPAROUND_E, (16*2 + 15)),  \
\
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_3_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_0_WRAPAROUND_E,  (16*3 +  0)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_3_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_1_WRAPAROUND_E,  (16*3 +  1)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_3_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_2_WRAPAROUND_E,  (16*3 +  2)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_3_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_3_WRAPAROUND_E,  (16*3 +  3)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_3_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_4_WRAPAROUND_E,  (16*3 +  4)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_3_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_5_WRAPAROUND_E,  (16*3 +  5)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_3_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_6_WRAPAROUND_E,  (16*3 +  6)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_3_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_7_WRAPAROUND_E,  (16*3 +  7)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_3_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_8_WRAPAROUND_E,  (16*3 +  8)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_3_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_9_WRAPAROUND_E,  (16*3 +  9)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_3_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_10_WRAPAROUND_E, (16*3 + 10)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_3_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_11_WRAPAROUND_E, (16*3 + 11)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_3_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_12_WRAPAROUND_E, (16*3 + 12)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_3_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_13_WRAPAROUND_E, (16*3 + 13)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_3_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_14_WRAPAROUND_E, (16*3 + 14)),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(CNC_INST_3_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_15_WRAPAROUND_E, (16*3 + 15)),  \
                                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_RX_CNTR_OVERFLOW_E,                                                                                      \
    SINGLE_MG_RX_SDMA_MAC(0),                                                                                     \
    SINGLE_MG_RX_SDMA_MAC(1),                                                                                     \
    SINGLE_MG_RX_SDMA_MAC(2),                                                                                     \
    SINGLE_MG_RX_SDMA_MAC(3),                                                                                     \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_SCT_RATE_LIMITER_E,                                                                                      \
    HAWK_SET_EVENT_PER_ALL_PORTS_ALL_REGISTERS_MAC(PKT_DROPED_INT_E),                                             \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E,                                                                            \
    HAWK_SET_EVENT_WITH_INDEX_MAC(IPLR0_POLICER_INTERRUPT_CAUSE_ILLEGAL_ACCESS_E,       0),                       \
    HAWK_SET_EVENT_WITH_INDEX_MAC(IPLR1_POLICER_INTERRUPT_CAUSE_ILLEGAL_ACCESS_E,       1),                       \
    HAWK_SET_EVENT_WITH_INDEX_MAC(EPLR_POLICER_INTERRUPT_CAUSE_ILLEGAL_ACCESS_E,        2),                       \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E,                                                                             \
    HAWK_SET_EVENT_WITH_INDEX_MAC(IPLR0_POLICER_INTERRUPT_CAUSE_IPFIX_WRAPAROUND_E,            0 ),               \
    HAWK_SET_EVENT_WITH_INDEX_MAC(IPLR1_POLICER_INTERRUPT_CAUSE_IPFIX_WRAPAROUND_E,            1 ),               \
    HAWK_SET_EVENT_WITH_INDEX_MAC(EPLR_POLICER_INTERRUPT_CAUSE_IPFIX_WRAPAROUND_E,             2 ),               \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_POLICER_IPFIX_ALARM_E,                                                                                   \
    HAWK_SET_EVENT_WITH_INDEX_MAC(IPLR0_POLICER_INTERRUPT_CAUSE_IPFIX_ALARM_E,                 0 ),               \
    HAWK_SET_EVENT_WITH_INDEX_MAC(IPLR1_POLICER_INTERRUPT_CAUSE_IPFIX_ALARM_E,                 1 ),               \
    HAWK_SET_EVENT_WITH_INDEX_MAC(EPLR_POLICER_INTERRUPT_CAUSE_IPFIX_ALARM_E,                  2 ),               \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,                                                        \
    HAWK_SET_EVENT_WITH_INDEX_MAC(IPLR0_POLICER_INTERRUPT_CAUSE_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    0 ),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(IPLR1_POLICER_INTERRUPT_CAUSE_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    1 ),  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(EPLR_POLICER_INTERRUPT_CAUSE_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,     2 ),  \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,                                                                                 \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    0),      \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    0),      \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    0),      \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    0),      \
                                                                                                                  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    1),      \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    1),      \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    1),      \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    1),      \
                                                                                                                  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    2),      \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    2),      \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    2),      \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    2),      \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_PCL_LOOKUP_FIFO_FULL_E,                                                                                  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_LOOKUP0_FIFO_FULL_E,                      0),      \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_LOOKUP1_FIFO_FULL_E,                      1),      \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_LOOKUP2_FIFO_FULL_E,                      2),      \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_PCL_ACTION_TRIGGERED_E,                                                                                  \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_IPCL0_TCAM_TRIGGERED_INTERRUPT_E,         0),      \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_IPCL1_TCAM_TRIGGERED_INTERRUPT_E,         1),      \
    HAWK_SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_IPCL2_TCAM_TRIGGERED_INTERRUPT_E,         2),      \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_EB_NA_FIFO_FULL_E,                                                                                             \
    HAWK_SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_0_NA_FIFO_FULL_E,         0),                 \
    HAWK_SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_1_NA_FIFO_FULL_E,         1),                 \
    HAWK_SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_2_NA_FIFO_FULL_E,         2),                 \
    HAWK_SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_3_NA_FIFO_FULL_E,         3),                 \
    HAWK_SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_4_NA_FIFO_FULL_E,         4),                 \
    HAWK_SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_5_NA_FIFO_FULL_E,         5),                 \
    HAWK_SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_6_NA_FIFO_FULL_E,         6),                 \
    HAWK_SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_7_NA_FIFO_FULL_E,         7),                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_TTI_ACCESS_DATA_ERROR_E,                                                                                       \
    HAWK_SET_EVENT_WITH_INDEX_MAC(TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_0_ACCESS_DATA_ERROR_E, 0),                   \
    HAWK_SET_EVENT_WITH_INDEX_MAC(TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_1_ACCESS_DATA_ERROR_E, 1),                   \
    HAWK_SET_EVENT_WITH_INDEX_MAC(TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_2_ACCESS_DATA_ERROR_E, 2),                   \
    HAWK_SET_EVENT_WITH_INDEX_MAC(TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_3_ACCESS_DATA_ERROR_E, 3),                   \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 /* next HW interrupt value will have same index (evExtData) value in CPSS_PP_CRITICAL_HW_ERROR_E */                    \
 CPSS_PP_CRITICAL_HW_ERROR_E,                                                                                           \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E), \
\
    /*PSI_REG*/                                                                                                         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PSI_REGS_PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_E),                          \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PSI_REGS_PSI_INTERRUPT_CAUSE_MG2SNAKE_INTERRUPT_E),                           \
\
    /*TXQ_QFC*/                                                                                                         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PORT_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E),\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E),\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PORT_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E),\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E),\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PORT_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E),\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E),\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PORT_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E),\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E),\
\
    /*TXQ_PFCC*/                                                                                                        \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_PR_PFCC_INTERRUPT_CAUSE_POOL_0_COUNTER_OVERFLOW_E),                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_PR_PFCC_INTERRUPT_CAUSE_SOURCE_PORT_COUNTER_OVERFLOW_E),                  \
\
    /*HBU*/                                                                                                             \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HBU_HBU_INTERRUPT_CAUSE_REGISTER_ERROR_FHF_MEM_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HBU_HBU_INTERRUPT_CAUSE_REGISTER_ERROR_CMT1_MEM_E),                           \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HBU_HBU_INTERRUPT_CAUSE_REGISTER_ERROR_CMT2_MEM_E),                           \
\
    /*TXQ_SDQ*/                                                                                                         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_CREDIT_E),\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_ENQ_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_DEQ_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_QCN_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_CREDIT_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_ENQ_E),    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_DEQ_E),    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_QCN_E),    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_PFC_E),    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_PORT_OUTOFRANGE_E),\
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_CREDIT_E),\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_ENQ_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_DEQ_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_QCN_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_CREDIT_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_ENQ_E),    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_DEQ_E),    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_QCN_E),    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_PFC_E),    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_PORT_OUTOFRANGE_E),\
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_CREDIT_E),\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_ENQ_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_DEQ_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_QCN_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_CREDIT_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_ENQ_E),    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_DEQ_E),    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_QCN_E),    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_PFC_E),    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_PORT_OUTOFRANGE_E),\
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_CREDIT_E),\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_ENQ_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_DEQ_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_QCN_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_CREDIT_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_ENQ_E),    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_DEQ_E),    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_QCN_E),    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_PFC_E),    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_PORT_OUTOFRANGE_E),\
\
    /*TXQ_PDS*/                                                                                                                              \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_STATE_ORDER_FIFO_FULL_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_DATA_ORDER_FIFO_FULL_E),                 \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_ON_THE_AIR_CNTR_FULL_INT_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_E),              \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_SAME_WR_ADDR_INT_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_PDS_INTERRUPT_DEBUG_CAUSE_TOTAL_PDS_CNTR_OVRRUN_INT_E),                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_PDS_INTERRUPT_DEBUG_CAUSE_PDS_CACHE_CNTR_OVRRUN_INT_E),                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_0_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_E),                 \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_STATE_ORDER_FIFO_FULL_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_DATA_ORDER_FIFO_FULL_E),                 \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_ON_THE_AIR_CNTR_FULL_INT_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_E),              \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_SAME_WR_ADDR_INT_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_PDS_INTERRUPT_DEBUG_CAUSE_TOTAL_PDS_CNTR_OVRRUN_INT_E),                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_PDS_INTERRUPT_DEBUG_CAUSE_PDS_CACHE_CNTR_OVRRUN_INT_E),                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_0_UNIT_INST_1_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_E),                 \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_STATE_ORDER_FIFO_FULL_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_DATA_ORDER_FIFO_FULL_E),                 \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_ON_THE_AIR_CNTR_FULL_INT_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_E),              \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_SAME_WR_ADDR_INT_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_PDS_INTERRUPT_DEBUG_CAUSE_TOTAL_PDS_CNTR_OVRRUN_INT_E),                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_PDS_INTERRUPT_DEBUG_CAUSE_PDS_CACHE_CNTR_OVRRUN_INT_E),                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_0_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_E),                 \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_STATE_ORDER_FIFO_FULL_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_DATA_ORDER_FIFO_FULL_E),                 \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_ON_THE_AIR_CNTR_FULL_INT_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_E),              \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_SAME_WR_ADDR_INT_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_PDS_INTERRUPT_DEBUG_CAUSE_TOTAL_PDS_CNTR_OVRRUN_INT_E),                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_PDS_INTERRUPT_DEBUG_CAUSE_PDS_CACHE_CNTR_OVRRUN_INT_E),                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQS_INST_1_UNIT_INST_1_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_E),                 \
\
    /*GPC_CELL_READ*/                                                                                                                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_0_INTERRUPT_CAUSE_INTERFACE_0_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_E),\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_0_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_0_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E),                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_0_INTERRUPT_CAUSE_CAM_MULTIPLE_MATCHES_E),                                  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_0_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E),                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_0_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_OVERFLOW_E),                     \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_1_INTERRUPT_CAUSE_INTERFACE_0_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_E),\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_1_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_1_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E),                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_1_INTERRUPT_CAUSE_CAM_MULTIPLE_MATCHES_E),                                  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_1_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E),                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_1_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_OVERFLOW_E),                     \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_2_INTERRUPT_CAUSE_INTERFACE_0_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_E),\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_2_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_2_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E),                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_2_INTERRUPT_CAUSE_CAM_MULTIPLE_MATCHES_E),                                  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_2_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E),                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_2_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_OVERFLOW_E),                     \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_3_INTERRUPT_CAUSE_INTERFACE_0_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_E),\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_3_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_3_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E),                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_3_INTERRUPT_CAUSE_CAM_MULTIPLE_MATCHES_E),                                  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_3_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E),                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INST_3_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_OVERFLOW_E),                     \
\
    /*NPM_MC*/                                                                                                                           \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_4_E),                      \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_0_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_1_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_2_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_3_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_4_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_0_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_1_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_2_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_3_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_4_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_0_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_1_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_2_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_3_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_4_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_0_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_1_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_2_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_3_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_4_E),                   \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_4_E),                      \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_4_E),                      \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_3_E),                      \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_3_E),                      \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_3_E),                      \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_0_E),                           \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_1_E),                           \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_2_E),                           \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_0_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_3_E),                           \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_4_E),                      \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_0_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_1_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_2_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_3_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_4_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_0_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_1_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_2_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_3_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_4_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_0_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_1_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_2_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_3_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_4_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_0_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_1_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_2_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_3_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_4_E),                   \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_4_E),                      \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_4_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_4_E),                      \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_3_E),                      \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_3_E),                      \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_3_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_0_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_1_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_2_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_3_E),                      \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_0_E),                           \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_1_E),                           \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_2_E),                           \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_NPM_INST_1_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_3_E),                           \
\
    /*SMB_MC*/                                                                                                                           \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E),                  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_0_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_1_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_2_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_3_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_4_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_5_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_6_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_7_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_8_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_9_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_10_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_11_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_12_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_13_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_14_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_15_REFCNT_SER_ERROR_E),                                   \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E),                  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E),                  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_0_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_1_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_2_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_3_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_4_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_5_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_6_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_7_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_8_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_9_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_10_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_11_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_12_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_13_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_14_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_15_REFCNT_SER_ERROR_E),                                   \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E),                  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_AGE_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_RBW_ERR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_0_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_1_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_2_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_3_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_4_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_5_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_6_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_7_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_8_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_9_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_10_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_11_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_12_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_13_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_14_REFCNT_SER_ERROR_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_SMB_INST_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_15_REFCNT_SER_ERROR_E),                                   \
\
    /*SMB_WRITE_ARBITER*/                                                                                                                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(WA_INST_0_INTERRUPT_MISC_CAUSE_INTERRUPT_MC_CA_FIFO_OVERFLOW_E),                               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(WA_INST_1_INTERRUPT_MISC_CAUSE_INTERRUPT_MC_CA_FIFO_OVERFLOW_E),                               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(WA_INST_2_INTERRUPT_MISC_CAUSE_INTERRUPT_MC_CA_FIFO_OVERFLOW_E),                               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(WA_INST_3_INTERRUPT_MISC_CAUSE_INTERRUPT_MC_CA_FIFO_OVERFLOW_E),                               \
\
    /*GPC_PACKET_WRITE[0x00C00200]*/                                                                                                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_0_E),                                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_1_E),                                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_2_E),                                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_3_E),                                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_0_E),                                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_1_E),                                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_2_E),                                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_3_E),                                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_0_E),                                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_1_E),                                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_2_E),                                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_3_E),                                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_0_E),                                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_1_E),                                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_2_E),                                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_3_E),                                       \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PACKET_WRITE_INST_0_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
\
    /*GPC_PACKET_READ[0x01C01300]*/                                                                                                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_0_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_0_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_0_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_0_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_0_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_0_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_0_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_0_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_0_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_0_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_0_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_0_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_0_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_0_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_0_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_0_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_1_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_1_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_1_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_1_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_1_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_1_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_1_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_1_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_1_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_1_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_1_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_1_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_1_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_1_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_1_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_1_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_2_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_2_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_2_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_2_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_2_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_2_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_2_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_2_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_2_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_2_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_2_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_2_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_2_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_2_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_2_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_2_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_3_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_3_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_3_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_3_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_3_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_3_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_3_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_3_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_3_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_3_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_3_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_3_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_3_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_3_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_3_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_3_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_4_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_4_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_4_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_4_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_4_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_4_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_4_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_4_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_4_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_4_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_4_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_4_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_4_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_4_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_4_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_4_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_5_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_5_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_5_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_5_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_5_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_5_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_5_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_5_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_5_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_5_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_5_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_5_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_5_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_5_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_5_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_5_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_6_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_6_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_6_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_6_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_6_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_6_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_6_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_6_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_6_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_6_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_6_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_6_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_6_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_6_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_6_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_6_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_7_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_7_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_7_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_7_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_7_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_7_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_7_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_7_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_7_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_7_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_7_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_7_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_7_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_7_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_7_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_7_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_8_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_8_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_8_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_8_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_8_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_8_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_8_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_8_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_8_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_8_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_8_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_8_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_8_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_8_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_8_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_8_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_9_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_9_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_9_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_9_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_9_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_9_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_9_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_9_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_9_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_9_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_9_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_9_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_9_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_9_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_9_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_9_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_10_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_10_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_10_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_10_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_10_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_10_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_10_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_10_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_10_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_10_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_10_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_10_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_10_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_10_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_10_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_10_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_11_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_11_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_11_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_11_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_11_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_11_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_11_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_11_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_11_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_11_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_11_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_11_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_11_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_11_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_11_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_11_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_12_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_12_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_12_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_12_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_12_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_12_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_12_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_12_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_12_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_12_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_12_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_12_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_12_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_12_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_12_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_12_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_13_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_13_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_13_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_13_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_13_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_13_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_13_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_13_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_13_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_13_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_13_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_13_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_13_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_13_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_13_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_13_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_14_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_14_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_14_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_14_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_14_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_14_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_14_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_14_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_14_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_14_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_14_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_14_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_14_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_14_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_14_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_14_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_15_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_15_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_15_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_15_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_15_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_15_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_15_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_15_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_15_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_15_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_15_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_15_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_15_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_15_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_15_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_15_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_16_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_16_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_16_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_16_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_16_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_16_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_16_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_16_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_16_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_16_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_16_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_16_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_16_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_16_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_16_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_16_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_17_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_17_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_17_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_17_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_17_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_17_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_17_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_17_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_17_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_17_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_17_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_17_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_17_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_17_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_17_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_17_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_18_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_18_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_18_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_18_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_18_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_18_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_18_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_18_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_18_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_18_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_18_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_18_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_18_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_18_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_18_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_18_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_19_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_19_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_19_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_19_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_19_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_19_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_19_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_19_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_19_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_19_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_19_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_19_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_19_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_19_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_19_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_19_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_20_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_20_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_20_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_20_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_20_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_20_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_20_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_20_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_20_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_20_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_20_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_20_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_20_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_20_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_20_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_20_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_21_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_21_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_21_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_21_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_21_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_21_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_21_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_21_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_21_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_21_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_21_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_21_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_21_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_21_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_21_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_21_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_22_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_22_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_22_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_22_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_22_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_22_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_22_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_22_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_22_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_22_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_22_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_22_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_22_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_22_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_22_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_22_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_23_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_23_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_23_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_23_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_23_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_23_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_23_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_23_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_23_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_23_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_23_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_23_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_23_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_23_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_23_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_23_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_24_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_24_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_24_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_24_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_24_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_24_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_24_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_24_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_24_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_24_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_24_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_24_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_24_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_24_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_24_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_24_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_25_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_25_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_25_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_25_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_25_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_25_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_25_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_25_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_25_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_25_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_25_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_25_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_25_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_25_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_25_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_25_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_26_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_26_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_26_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_26_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_26_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_26_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_26_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_26_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_26_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_26_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_26_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_26_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_26_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_26_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_26_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_26_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_27_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_27_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_27_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_CHANNEL_INDEX_27_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_27_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_27_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_27_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_CHANNEL_INDEX_27_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_27_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_27_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_27_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_CHANNEL_INDEX_27_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_27_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_27_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_27_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_CHANNEL_INDEX_27_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    /*GPC_PACKET_READ_MISC[0x01C01508]*/                                                                                                  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_OVERFLOW_E),                               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_UNDERFLOW_E),                              \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_MISC_CAUSE_CAM_MULTIPLE_MATCHES_E),                                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_MISC_CAUSE_DESCRIPTOR_HEADER_SIZE_ERROR_E),                             \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_MISC_CAUSE_DESCRIPTOR_TAIL_SIZE_ERROR_E),                               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_MISC_CAUSE_PACKET_COUNT_PACKET_SIZE_ERROR_E),                           \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_OVERFLOW_E),                               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_UNDERFLOW_E),                              \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_MISC_CAUSE_CAM_MULTIPLE_MATCHES_E),                                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_MISC_CAUSE_DESCRIPTOR_HEADER_SIZE_ERROR_E),                             \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_MISC_CAUSE_DESCRIPTOR_TAIL_SIZE_ERROR_E),                               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_MISC_CAUSE_PACKET_COUNT_PACKET_SIZE_ERROR_E),                           \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_OVERFLOW_E),                               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_UNDERFLOW_E),                              \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_MISC_CAUSE_CAM_MULTIPLE_MATCHES_E),                                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_MISC_CAUSE_DESCRIPTOR_HEADER_SIZE_ERROR_E),                             \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_MISC_CAUSE_DESCRIPTOR_TAIL_SIZE_ERROR_E),                               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_MISC_CAUSE_PACKET_COUNT_PACKET_SIZE_ERROR_E),                           \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E),                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_OVERFLOW_E),                               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_UNDERFLOW_E),                              \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_MISC_CAUSE_CAM_MULTIPLE_MATCHES_E),                                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_MISC_CAUSE_DESCRIPTOR_HEADER_SIZE_ERROR_E),                             \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_MISC_CAUSE_DESCRIPTOR_TAIL_SIZE_ERROR_E),                               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_MISC_CAUSE_PACKET_COUNT_PACKET_SIZE_ERROR_E),                           \
\
    /*GPC_PACKET_READ_PACKET_COUNT[0x01C01500]*/                                                                                          \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E),     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_COUNT_CAUSE_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_PACKET_COUNT_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E),     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_COUNT_CAUSE_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_PACKET_COUNT_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E),     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_COUNT_CAUSE_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_PACKET_COUNT_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E),     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_COUNT_CAUSE_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_PACKET_COUNT_E),   \
\
    /*GPC_PACKET_READ_PACKET_CREDIT_COUNTER[0x01C01510]*/                                                                                 \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_HT_OVERFLOW_E),           \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_HT_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_HT_OVERFLOW_E),     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E),                  \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_HT_OVERFLOW_E),           \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_HT_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_HT_OVERFLOW_E),     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E),                  \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_HT_OVERFLOW_E),           \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_HT_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_HT_OVERFLOW_E),     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E),                  \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_HT_OVERFLOW_E),           \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_HT_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_HT_OVERFLOW_E),     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E),                  \
\
    /*GPC_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER[0x01C01518]*/                                                         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),        \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E),          \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),        \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E),          \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),        \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E),          \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),        \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PB_GPR_INST_3_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E),          \
\
    /*DFX_SERVER_INTERRUPT_CAUSE[0x0B8F8108]*/                                                                                                                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_BIST_FAIL_E),                                                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_EXTERNAL_TEMPERATURE_THRESHOLD_E),                               \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(BMA_MC_CNT_COUNTER_FLOW_E                                 ),  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(BMA_MC_CNT_PARITY_ERROR_E                                 ),  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(BMA_SHIFTER_LATENCY_FIFO_OVERFLOW_E                       ),  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(BMA_SHIFTER_LATENCY_FIFO_UNDERFLOW_E                      ),  \
\
    /*RXDMA_INTERRUPT2_CAUSE[0x08001C90]*/                                                                                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_0_RX_DMA_INTERRUPT_2_CAUSE_PB_TAIL_ID_MEM_SER_INT_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_0_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_0_MEM_SER_INT_E),                 \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_0_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_1_MEM_SER_INT_E),                 \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_0_RX_DMA_INTERRUPT_2_CAUSE_REPLY_TAIL_PB_ID_MEM_SER_INT_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_0_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_0_PB_RD_DATA_ERR_INT_E),               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_0_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_1_PB_RD_DATA_ERR_INT_E),               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_0_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_0_PB_RD_DATA_ERR_INT_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_0_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_1_PB_RD_DATA_ERR_INT_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_0_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_0_RD_SER_INT_E),               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_0_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_1_RD_SER_INT_E),               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_0_RX_DMA_INTERRUPT_2_CAUSE_ENQUEUE_REQUESTS_DESCRIPTOR_FIFO_RD_SER_INT_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_0_RX_DMA_INTERRUPT_2_CAUSE_HEAD_PACKET_COUNT_DATA_FIFO_MEM_RD_SER_INT_E),  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_0_RX_DMA_INTERRUPT_2_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E),            \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_1_RX_DMA_INTERRUPT_2_CAUSE_PB_TAIL_ID_MEM_SER_INT_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_1_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_0_MEM_SER_INT_E),                 \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_1_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_1_MEM_SER_INT_E),                 \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_1_RX_DMA_INTERRUPT_2_CAUSE_REPLY_TAIL_PB_ID_MEM_SER_INT_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_1_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_0_PB_RD_DATA_ERR_INT_E),               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_1_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_1_PB_RD_DATA_ERR_INT_E),               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_1_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_0_PB_RD_DATA_ERR_INT_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_1_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_1_PB_RD_DATA_ERR_INT_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_1_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_0_RD_SER_INT_E),               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_1_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_1_RD_SER_INT_E),               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_1_RX_DMA_INTERRUPT_2_CAUSE_ENQUEUE_REQUESTS_DESCRIPTOR_FIFO_RD_SER_INT_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_1_RX_DMA_INTERRUPT_2_CAUSE_HEAD_PACKET_COUNT_DATA_FIFO_MEM_RD_SER_INT_E),  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_1_RX_DMA_INTERRUPT_2_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E),            \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_2_RX_DMA_INTERRUPT_2_CAUSE_PB_TAIL_ID_MEM_SER_INT_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_2_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_0_MEM_SER_INT_E),                 \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_2_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_1_MEM_SER_INT_E),                 \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_2_RX_DMA_INTERRUPT_2_CAUSE_REPLY_TAIL_PB_ID_MEM_SER_INT_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_2_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_0_PB_RD_DATA_ERR_INT_E),               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_2_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_1_PB_RD_DATA_ERR_INT_E),               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_2_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_0_PB_RD_DATA_ERR_INT_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_2_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_1_PB_RD_DATA_ERR_INT_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_2_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_0_RD_SER_INT_E),               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_2_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_1_RD_SER_INT_E),               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_2_RX_DMA_INTERRUPT_2_CAUSE_ENQUEUE_REQUESTS_DESCRIPTOR_FIFO_RD_SER_INT_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_2_RX_DMA_INTERRUPT_2_CAUSE_HEAD_PACKET_COUNT_DATA_FIFO_MEM_RD_SER_INT_E),  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_2_RX_DMA_INTERRUPT_2_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E),            \
\
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_3_RX_DMA_INTERRUPT_2_CAUSE_PB_TAIL_ID_MEM_SER_INT_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_3_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_0_MEM_SER_INT_E),                 \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_3_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_1_MEM_SER_INT_E),                 \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_3_RX_DMA_INTERRUPT_2_CAUSE_REPLY_TAIL_PB_ID_MEM_SER_INT_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_3_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_0_PB_RD_DATA_ERR_INT_E),               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_3_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_1_PB_RD_DATA_ERR_INT_E),               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_3_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_0_PB_RD_DATA_ERR_INT_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_3_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_1_PB_RD_DATA_ERR_INT_E),                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_3_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_0_RD_SER_INT_E),               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_3_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_1_RD_SER_INT_E),               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_3_RX_DMA_INTERRUPT_2_CAUSE_ENQUEUE_REQUESTS_DESCRIPTOR_FIFO_RD_SER_INT_E), \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_3_RX_DMA_INTERRUPT_2_CAUSE_HEAD_PACKET_COUNT_DATA_FIFO_MEM_RD_SER_INT_E),  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_INST_3_RX_DMA_INTERRUPT_2_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E),            \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_DATA_INTEGRITY_ERROR_E,                                                                                        \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HA_HA_INTERRUPT_CAUSE_ECC_SINGLE_ERROR_E),                                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HA_HA_INTERRUPT_CAUSE_ECC_DOUBLE_ERROR_E),                                    \
                                                                                                                        \
    /* -- _PARITY_ -- */                                                                                                \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TCAM_TCAM_INTERRUPT_CAUSE_TCAM_ARRAY_PARITY_ERROR_E),                         \
                                                                                                                        \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INST_0_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INST_0_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INST_1_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INST_1_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INST_2_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INST_2_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INST_3_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INST_3_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INST_4_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INST_4_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INST_5_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INST_5_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INST_6_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INST_6_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INST_7_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SDW_INST_7_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                            \
                                                                                                                        \
    /* DFX Interrupts. AC5P has 8 DFX Pipes. Need to connect to all. */                                                 \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E),                         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E),                         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E),                         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_3_INTERRUPT_SUM_E),                         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_4_INTERRUPT_SUM_E),                         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_5_INTERRUPT_SUM_E),                         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_6_INTERRUPT_SUM_E),                         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_7_INTERRUPT_SUM_E),                         \
\
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E,                                                                               \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_INVALID_KEEPALIVE_HASH_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_INVALID_KEEPALIVE_HASH_E),                      \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E,                                                                              \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_EXCESS_KEEPALIVE_E),                            \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_EXCESS_KEEPALIVE_E),                            \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E,                                                                             \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_INVALID_KEEPALIVE_HASH_E),                      \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_INVALID_KEEPALIVE_HASH_E),                      \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E,                                                                                    \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_RDI_STATUS_E),                                  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_RDI_STATUS_E),                                  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E,                                                                                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_TX_PERIOD_E),                                   \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_TX_PERIOD_E),                                   \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E,                                                                                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_MEG_LEVEL_EXCEPTION_E),                         \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_MEG_LEVEL_EXCEPTION_E),                         \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E,                                                                              \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_SOURCE_INTERFACE_EXCEPTION_E),                  \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_SOURCE_INTERFACE_EXCEPTION_E),                  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E,                                                                                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_ILLEGAL_OAM_ENTRY_INDEX_E),                     \
    HAWK_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_ILLEGAL_OAM_ENTRY_INDEX_E),                     \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
                                                                                                                        \
 CPSS_PP_PHA_E,                                                                                                         \
    SET_PHA_EVENTS_MAC,                                                                                                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_TQ_PORT_HR_CROSSED_THRESHOLD_E,                                                                                \
    HAWK_SET_HR_TRSH_EVENT_ALL_PORTS_MAC,                                                                               \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
CPSS_PP_MACSEC_SA_EXPIRED_E,\
    HAWK_SET_EIP_164_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SA_EXPIRED_E),                  \
MARK_END_OF_UNI_EV_CNS,                                                                                                 \
                                                                                                                        \
CPSS_PP_MACSEC_SA_PN_FULL_E,                                                                                            \
    HAWK_SET_EIP_164_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SA_PN_THR_E),                 \
MARK_END_OF_UNI_EV_CNS,                                                                                                 \
                                                                                                                        \
CPSS_PP_MACSEC_STATISTICS_SUMMARY_E,                                                                                    \
    HAWK_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_SA_THR_E,       0),  \
    HAWK_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_SECY_THR_E,     1),  \
    HAWK_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_IFC0_THR_E,     2),  \
    HAWK_SET_EIP_164_EVENT_SOURCE_PER_INGRESS_DIRECTION_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_IFC1_THR_E,  3),  \
    HAWK_SET_EIP_164_EVENT_SOURCE_PER_INGRESS_DIRECTION_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_RXCAM_THR_E, 4),  \
                                                                                                                         \
    HAWK_SET_EIP_163_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E,     5),  \
    HAWK_SET_EIP_163_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_CHANNEL_THR_E,  6),  \
MARK_END_OF_UNI_EV_CNS,                                                                                                  \
                                                                                                                         \
CPSS_PP_MACSEC_TRANSFORM_ENGINE_ERR_E,                                                                                   \
    HAWK_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(0),       \
    HAWK_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(1),       \
    HAWK_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(2),       \
    HAWK_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(3),       \
    HAWK_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(4),       \
    HAWK_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(5),       \
    HAWK_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(6),       \
    HAWK_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(7),       \
    HAWK_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(8),       \
    HAWK_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(9),       \
    HAWK_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(10),      \
    HAWK_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(11),      \
    HAWK_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(12),      \
    HAWK_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(13),      \
    HAWK_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(14),      \
MARK_END_OF_UNI_EV_CNS,                                                                                                            \
                                                                                                                                   \
CPSS_PP_MACSEC_EGRESS_SEQ_NUM_ROLLOVER_E,                                                                                          \
    HAWK_SET_EIP_66_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SEQ_NUM_ROLLOVER_E),                      \
MARK_END_OF_UNI_EV_CNS,                                                                                                            \

#define PORTS_LINK_STATUS_CHANGE_EVENTS \
    /* USX ports */                                                                         \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_USX_PORTS(0, LINK_STATUS_CHANGE_E),                      \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_USX_PORTS(1, LINK_STATUS_CHANGE_E),                      \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_USX_PORTS(0, LPCS_LINK_STATUS_CHANGE_E),                 \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_USX_PORTS(1, LPCS_LINK_STATUS_CHANGE_E),                 \
    /* MTI total ports 0..255 */                                                            \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(0,LINK_STATUS_CHANGE_E),                       \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(1,LINK_STATUS_CHANGE_E),                       \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(2,LINK_STATUS_CHANGE_E),                       \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(3,LINK_STATUS_CHANGE_E),                       \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(0,LPCS_LINK_STATUS_CHANGE_E),                  \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(1,LPCS_LINK_STATUS_CHANGE_E),                  \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(2,LPCS_LINK_STATUS_CHANGE_E),                  \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(3,LPCS_LINK_STATUS_CHANGE_E),                  \
/* seg ports*/                                                                              \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(0,LINK_STATUS_CHANGE_E),                       \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(1,LINK_STATUS_CHANGE_E),                       \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(2,LINK_STATUS_CHANGE_E),                       \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(3,LINK_STATUS_CHANGE_E)

#define PORTS_LINK_OK_CHANGE_EVENTS \
                                                                                            \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_USX_PORTS(0, LINK_OK_CHANGE_E),                          \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_USX_PORTS(1, LINK_OK_CHANGE_E),                          \
    /* MTI total ports 0..255 */                                                            \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(0, LINK_OK_CHANGE_E),                          \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(1, LINK_OK_CHANGE_E),                          \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(2, LINK_OK_CHANGE_E),                          \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(3, LINK_OK_CHANGE_E),                          \
/* seg ports*/                                                                              \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(0, LINK_OK_CHANGE_E),                          \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(1, LINK_OK_CHANGE_E),                          \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(2, LINK_OK_CHANGE_E),                          \
    HAWK_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(3, LINK_OK_CHANGE_E)

#define HAWK_CPU_PORT_LINK_STATUS_CHANGE_EVENTS \
    /* CPU ports */                                                                         \
    PRV_CPSS_AC5P_CPU_MAC_PORT_INTERRUPT_CAUSE_LINK_STATUS_CHANGED_E,    105

#define HAWK_CPU_PORT_LINK_OK_CHANGED_EVENTS \
    /* CPU ports */                                                                         \
    PRV_CPSS_AC5P_CPU_MAC_PORT_INTERRUPT_CAUSE_LINK_OK_CHANGED_E,        105

#define AC5P_PORT_AN_HCD_RESOLUTION_DONE_EVENTS                                 \
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_0_HCD_FOUND_E,                 0,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_1_HCD_FOUND_E,                 1,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_2_HCD_FOUND_E,                 2,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_3_HCD_FOUND_E,                 6,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_4_HCD_FOUND_E,                10,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_5_HCD_FOUND_E,                14,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_6_HCD_FOUND_E,                18,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_7_HCD_FOUND_E,                22,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_0_HCD_FOUND_E,                26,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_1_HCD_FOUND_E,                27,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_2_HCD_FOUND_E,                28,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_3_HCD_FOUND_E,                32,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_4_HCD_FOUND_E,                36,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_5_HCD_FOUND_E,                40,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_6_HCD_FOUND_E,                44,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_7_HCD_FOUND_E,                48,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_0_HCD_FOUND_E,                52,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_1_HCD_FOUND_E,                53,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_2_HCD_FOUND_E,                54,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_3_HCD_FOUND_E,                58,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_4_HCD_FOUND_E,                62,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_5_HCD_FOUND_E,                66,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_6_HCD_FOUND_E,                70,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_7_HCD_FOUND_E,                74,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_0_HCD_FOUND_E,                78,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_1_HCD_FOUND_E,                79,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_2_HCD_FOUND_E,                80,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_3_HCD_FOUND_E,                84,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_4_HCD_FOUND_E,                88,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_5_HCD_FOUND_E,                92,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_6_HCD_FOUND_E,                96,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_7_HCD_FOUND_E,               100,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_5_USX_PCS_INST_0_PORT_NUM_0_HCD_FOUND_E,   2,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_5_USX_PCS_INST_1_PORT_NUM_0_HCD_FOUND_E,   6,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_6_USX_PCS_INST_0_PORT_NUM_0_HCD_FOUND_E,  10,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_6_USX_PCS_INST_1_PORT_NUM_0_HCD_FOUND_E,  14,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_7_USX_PCS_INST_0_PORT_NUM_0_HCD_FOUND_E,  18,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_7_USX_PCS_INST_1_PORT_NUM_0_HCD_FOUND_E,  22,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_8_USX_PCS_INST_0_PORT_NUM_0_HCD_FOUND_E,  28,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_8_USX_PCS_INST_1_PORT_NUM_0_HCD_FOUND_E,  32,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_9_USX_PCS_INST_0_PORT_NUM_0_HCD_FOUND_E,  36,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_9_USX_PCS_INST_1_PORT_NUM_0_HCD_FOUND_E,  40,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_10_USX_PCS_INST_0_PORT_NUM_0_HCD_FOUND_E, 44,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_10_USX_PCS_INST_1_PORT_NUM_0_HCD_FOUND_E, 48

#define AC5P_PORT_AN_RESTART_DONE_EVENTS                                 \
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_0_AN_RESTART_E,                 0,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_1_AN_RESTART_E,                 1,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_2_AN_RESTART_E,                 2,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_3_AN_RESTART_E,                 6,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_4_AN_RESTART_E,                10,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_5_AN_RESTART_E,                14,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_6_AN_RESTART_E,                18,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_7_AN_RESTART_E,                22,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_0_AN_RESTART_E,                26,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_1_AN_RESTART_E,                27,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_2_AN_RESTART_E,                28,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_3_AN_RESTART_E,                32,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_4_AN_RESTART_E,                36,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_5_AN_RESTART_E,                40,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_6_AN_RESTART_E,                44,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_7_AN_RESTART_E,                48,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_0_AN_RESTART_E,                52,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_1_AN_RESTART_E,                53,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_2_AN_RESTART_E,                54,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_3_AN_RESTART_E,                58,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_4_AN_RESTART_E,                62,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_5_AN_RESTART_E,                66,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_6_AN_RESTART_E,                70,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_7_AN_RESTART_E,                74,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_0_AN_RESTART_E,                78,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_1_AN_RESTART_E,                79,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_2_AN_RESTART_E,                80,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_3_AN_RESTART_E,                84,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_4_AN_RESTART_E,                88,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_5_AN_RESTART_E,                92,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_6_AN_RESTART_E,                96,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_7_AN_RESTART_E,               100,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_5_USX_PCS_INST_0_PORT_NUM_0_AN_RESTART_E,   2,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_5_USX_PCS_INST_1_PORT_NUM_0_AN_RESTART_E,   6,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_6_USX_PCS_INST_0_PORT_NUM_0_AN_RESTART_E,  10,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_6_USX_PCS_INST_1_PORT_NUM_0_AN_RESTART_E,  14,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_7_USX_PCS_INST_0_PORT_NUM_0_AN_RESTART_E,  18,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_7_USX_PCS_INST_1_PORT_NUM_0_AN_RESTART_E,  22,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_8_USX_PCS_INST_0_PORT_NUM_0_AN_RESTART_E,  28,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_8_USX_PCS_INST_1_PORT_NUM_0_AN_RESTART_E,  32,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_9_USX_PCS_INST_0_PORT_NUM_0_AN_RESTART_E,  36,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_9_USX_PCS_INST_1_PORT_NUM_0_AN_RESTART_E,  40,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_10_USX_PCS_INST_0_PORT_NUM_0_AN_RESTART_E, 44,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_10_USX_PCS_INST_1_PORT_NUM_0_AN_RESTART_E, 48

#define AC5P_PORT_INT_PM_PCS_LINK_TIMER_OUT_EVENTS                                 \
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,                 0,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_1_INT_PM_PCS_LINK_TIMER_OUT_E,                 1,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_2_INT_PM_PCS_LINK_TIMER_OUT_E,                 2,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_3_INT_PM_PCS_LINK_TIMER_OUT_E,                 6,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_4_INT_PM_PCS_LINK_TIMER_OUT_E,                10,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_5_INT_PM_PCS_LINK_TIMER_OUT_E,                14,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_6_INT_PM_PCS_LINK_TIMER_OUT_E,                18,\
    PRV_CPSS_AC5P_400G_PCS_INST_1_PORT_NUM_7_INT_PM_PCS_LINK_TIMER_OUT_E,                22,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,                26,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_1_INT_PM_PCS_LINK_TIMER_OUT_E,                27,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_2_INT_PM_PCS_LINK_TIMER_OUT_E,                28,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_3_INT_PM_PCS_LINK_TIMER_OUT_E,                32,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_4_INT_PM_PCS_LINK_TIMER_OUT_E,                36,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_5_INT_PM_PCS_LINK_TIMER_OUT_E,                40,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_6_INT_PM_PCS_LINK_TIMER_OUT_E,                44,\
    PRV_CPSS_AC5P_400G_PCS_INST_2_PORT_NUM_7_INT_PM_PCS_LINK_TIMER_OUT_E,                48,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,                52,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_1_INT_PM_PCS_LINK_TIMER_OUT_E,                53,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_2_INT_PM_PCS_LINK_TIMER_OUT_E,                54,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_3_INT_PM_PCS_LINK_TIMER_OUT_E,                58,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_4_INT_PM_PCS_LINK_TIMER_OUT_E,                62,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_5_INT_PM_PCS_LINK_TIMER_OUT_E,                66,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_6_INT_PM_PCS_LINK_TIMER_OUT_E,                70,\
    PRV_CPSS_AC5P_400G_PCS_INST_3_PORT_NUM_7_INT_PM_PCS_LINK_TIMER_OUT_E,                74,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,                78,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_1_INT_PM_PCS_LINK_TIMER_OUT_E,                79,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_2_INT_PM_PCS_LINK_TIMER_OUT_E,                80,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_3_INT_PM_PCS_LINK_TIMER_OUT_E,                84,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_4_INT_PM_PCS_LINK_TIMER_OUT_E,                88,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_5_INT_PM_PCS_LINK_TIMER_OUT_E,                92,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_6_INT_PM_PCS_LINK_TIMER_OUT_E,                96,\
    PRV_CPSS_AC5P_400G_PCS_INST_4_PORT_NUM_7_INT_PM_PCS_LINK_TIMER_OUT_E,               100,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_5_USX_PCS_INST_0_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,   2,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_5_USX_PCS_INST_1_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,   6,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_6_USX_PCS_INST_0_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,  10,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_6_USX_PCS_INST_1_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,  14,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_7_USX_PCS_INST_0_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,  18,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_7_USX_PCS_INST_1_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,  22,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_8_USX_PCS_INST_0_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,  28,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_8_USX_PCS_INST_1_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,  32,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_9_USX_PCS_INST_0_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,  36,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_9_USX_PCS_INST_1_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,  40,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_10_USX_PCS_INST_0_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E, 44,\
    PRV_CPSS_AC5P_USX_PCS_UNIT_10_USX_PCS_INST_1_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E, 48

/* Interrupt cause to unified event map for hawk
 This Table is for unified event with extended data

  Array structure:
  1. The first element of the array and the elements after MARK_END_OF_UNI_EV_CNS are unified event types.
  2. Elements after unified event type are pairs of interrupt cause and event extended data,
     until MARK_END_OF_UNI_EV_CNS.
*/
static const GT_U32 hawkUniEvMapTableWithExtData[] = {
    CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
        PORTS_LINK_STATUS_CHANGE_EVENTS,
        HAWK_CPU_PORT_LINK_STATUS_CHANGE_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
        PORTS_LINK_OK_CHANGE_EVENTS,
        HAWK_CPU_PORT_LINK_OK_CHANGED_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_AN_HCD_FOUND_E,
        AC5P_PORT_AN_HCD_RESOLUTION_DONE_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_AN_RESTART_E,
        AC5P_PORT_AN_RESTART_DONE_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_AN_PARALLEL_DETECT_E,
        AC5P_PORT_INT_PM_PCS_LINK_TIMER_OUT_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    HAWK_COMMON_EVENTS
};

#define hawkUniEvMapTableUburst  \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        PRV_CPSS_AC5P_TXQS_INST_0_UNIT_INST_0_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E},    \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        PRV_CPSS_AC5P_TXQS_INST_0_UNIT_INST_1_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E},    \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        PRV_CPSS_AC5P_TXQS_INST_1_UNIT_INST_0_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E},    \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        PRV_CPSS_AC5P_TXQS_INST_1_UNIT_INST_1_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E}


#define hawkUniEvMapTableMg_x(_mgIndex)  \
    {CPSS_PP_EB_AUQ_PENDING_E,              PRV_CPSS_AC5P_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_AU_QUEUE_PENDING_E},                  \
    {CPSS_PP_EB_AUQ_FULL_E,                 PRV_CPSS_AC5P_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_AU_QUEUE_FULL_E},                     \
    {CPSS_PP_EB_AUQ_OVER_E,                 PRV_CPSS_AC5P_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_AU_QUEUE_OVERRUN_E},                  \
    {CPSS_PP_EB_AUQ_ALMOST_FULL_E,          PRV_CPSS_AC5P_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_AU_QUEUE_ALMOST_FULL_E},              \
    {CPSS_PP_EB_FUQ_PENDING_E,              PRV_CPSS_AC5P_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_FU_QUEUE_PENDING_E},                  \
    {CPSS_PP_EB_FUQ_FULL_E,                 PRV_CPSS_AC5P_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_FU_QUEUE_FULL_E},                     \
    {CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,    PRV_CPSS_AC5P_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_MG_READ_DMA_DONE_E},                  \
    {CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E,      PRV_CPSS_AC5P_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_PC_IE_ADDRESS_UNMAPPED_E},            \
    {CPSS_PP_MISC_TWSI_TIME_OUT_E,          PRV_CPSS_AC5P_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_I2C_TIME_OUT_INTERRUPT_E},            \
    {CPSS_PP_MISC_TWSI_STATUS_E,            PRV_CPSS_AC5P_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_I2C_STATUS_INTERRUPT_E},              \
    {CPSS_PP_MISC_ILLEGAL_ADDR_E,           PRV_CPSS_AC5P_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_INTERRUPT_E},         \
    {CPSS_PP_PORT_802_3_AP_E,               PRV_CPSS_AC5P_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_Z80_INTERRUPT_E},                     \
    {CPSS_SRVCPU_IPC_E,                     PRV_CPSS_AC5P_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_CM3_TO_HOST_DOORBELL_INTERRUPT_E}     \

/* Interrupt cause to unified event map for hawk
 This Table is for unified event without extended data
*/
static const GT_U32 hawkUniEvMapTable[][2] =
{
    {CPSS_PP_EB_SECURITY_BREACH_UPDATE_E,    PRV_CPSS_AC5P_L2I_BRIDGE_INTERRUPT_CAUSE_UPDATE_SECURITY_BREACH_REGISTER_INT_E         },
    {CPSS_PP_MAC_NUM_OF_HOP_EXP_E,           PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NUM_OF_HOP_EX_P_E                       },
    {CPSS_PP_MAC_NA_LEARNED_E,               PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NA_LEARNT_E                             },
    {CPSS_PP_MAC_NA_NOT_LEARNED_E,           PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NA_NOT_LEARNT_E                         },
    {CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E,    PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AGE_VIA_TRIGGER_ENDED_E                 },
    {CPSS_PP_MAC_UPDATE_FROM_CPU_DONE_E,     PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AU_PROC_COMPLETED_INT_E                 },
    {CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E,     PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AU_MSG_TOCPU_READY_E                    },
    {CPSS_PP_MAC_NA_SELF_LEARNED_E,          PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NA_SELF_LEARNED_E                       },
    {CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E,      PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_N_AFROM_CPU_LEARNED_E                   },
    {CPSS_PP_MAC_NA_FROM_CPU_DROPPED_E,      PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_N_AFROM_CPU_DROPPED_E                   },
    {CPSS_PP_MAC_AGED_OUT_E,                 PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AGED_OUT_E                              },
    {CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E,      PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AU_FIFO_TO_CPU_IS_FULL_E                },
    {CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E,      PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_ADDRESS_OUT_OF_RANGE_E                  },
    {CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E, PRV_CPSS_AC5P_TTI_TTI_ENGINE_INTERRUPT_CAUSE_CPU_ADDRESS_OUT_OF_RANGE_E                },
    {CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E,     PRV_CPSS_AC5P_PCL_PCL_UNIT_INTERRUPT_CAUSE_MG_ADDR_OUTOF_RANGE_E                       },
    {CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E, PRV_CPSS_AC5P_FDB_FDB_INTERRUPT_CAUSE_REGISTER_BLC_OVERFLOW_E                      },
    {CPSS_PP_EM_AUTO_LEARN_INDIRECT_ADDRESS_OUT_OF_RANGE_EVENT_E,       PRV_CPSS_AC5P_EM_EM_INTERRUPT_CAUSE_REGISTER_INDIRECT_ADDRESS_OUT_OF_RANGE_E        },
    {CPSS_PP_EM_AUTO_LEARN_FLOW_ID_ALLOCATION_FAILED_EVENT_E,           PRV_CPSS_AC5P_EM_EM_INTERRUPT_CAUSE_REGISTER_FLOW_ID_ALLOCATION_FAILED_E            },
    {CPSS_PP_EM_AUTO_LEARN_FLOW_ID_RANGE_FULL_EVENT_E,                  PRV_CPSS_AC5P_EM_EM_INTERRUPT_CAUSE_REGISTER_FLOW_ID_RANGE_FULL_E                   },
    {CPSS_PP_EM_AUTO_LEARN_FLOW_ID_THRESHOLD_CROSSED_EVENT_E,           PRV_CPSS_AC5P_EM_EM_INTERRUPT_CAUSE_REGISTER_FLOW_ID_THRESHOLD_CROSSED_E            },
    {CPSS_PP_EM_AUTO_LEARN_FLOW_ID_EMPTY_EVENT_E,                       PRV_CPSS_AC5P_EM_EM_INTERRUPT_CAUSE_REGISTER_FLOW_ID_EMPTY_E                        },
    {CPSS_PP_EM_AUTO_LEARN_COLLISION_EVENT_E,                           PRV_CPSS_AC5P_EM_EM_INTERRUPT_CAUSE_REGISTER_AUTO_LEARNING_COLLISION_E              },
    {CPSS_PP_EM_AUTO_LEARN_ILLEGAL_ADDRESS_ACCESS_EVENT_E,              PRV_CPSS_AC5P_EM_EM_INTERRUPT_CAUSE_REGISTER_ILLEGAL_ADDRESS_ACCESS_E               },
    {CPSS_PP_EM_AUTO_LEARN_ILLEGAL_TABLE_OVERLAP_EVENT_E,               PRV_CPSS_AC5P_EM_EM_INTERRUPT_CAUSE_REGISTER_ILLEGAL_TABLE_OVERLAP_E                },
    {CPSS_PP_EM_AUTO_LEARN_CHANNEL_0_SHM_ANSWER_FIFO_OVERFLOW_EVENT_E,  PRV_CPSS_AC5P_EM_EM_INTERRUPT_CAUSE_REGISTER_CHANNEL_0_SHM_ANSWER_FIFO_OVERFLOW_E   },
    {CPSS_PP_EM_AUTO_LEARN_CHANNEL_1_SHM_ANSWER_FIFO_OVERFLOW_EVENT_E,  PRV_CPSS_AC5P_EM_EM_INTERRUPT_CAUSE_REGISTER_CHANNEL_1_SHM_ANSWER_FIFO_OVERFLOW_E   },
    hawkUniEvMapTableMg_x(0)                                                                                             ,
    hawkUniEvMapTableMg_x(1)                                                                                             ,
    hawkUniEvMapTableMg_x(2)                                                                                             ,
    hawkUniEvMapTableMg_x(3)                                                                                             ,
    hawkUniEvMapTableUburst
};

/* Interrupt cause to unified event map for Hawk without extended data size */
static const GT_U32 hawkUniEvMapTableSize = (sizeof(hawkUniEvMapTable)/(sizeof(GT_U32)*2));

/**
* @internal prvCpssDxChHawkPortEventPortMapConvert function
* @endinternal
*
* @brief   Converts port unified event port from extended data to MAC/PHY port.
* @param[in] devNum                 - device number
* @param[in] evConvertType          - event convert type
* @param[inout] portNumPtr          - (pointer to) port from event extended data
*
* @retval GT_BAD_PTR                - port parameter is NULL pointer
* @retval GT_BAD_PARAM              - wrong port type
* @retval GT_OUT_OF_RANGE           - port index is out of range
*/
static GT_STATUS prvCpssDxChHawkPortEventPortMapConvert
(
    IN GT_SW_DEV_NUM            devNum,
    IN PRV_CPSS_EV_CONVERT_DIRECTION_ENT evConvertType,
    INOUT GT_U32                *portNumPtr
)
{

    GT_U32      portNum;
    GT_U32      portIndex;
    PRV_CPSS_PORT_TYPE_ENT  portMacType;
    const GT_U32   *portMacArrayPtr = NULL;
    GT_U32    portMacArraySize = 0;
    GT_U32    ii;

    #define CPU_PORT                105

    /* Converts MTI global port index to it's MAC numbers.
       Global port index represents extended data in unified events for MTI ports.
       Formula: global_port_index = port_group * 8 + port_index (where port_group(0-3) and port_index(0-7))
    */
    static const GT_U32 hawkMtiMacPortsArr[] = {
        /* global MAC ports */
           0,  1,  2,  6, 10, 14, 18, 22
        , 26, 27, 28, 32, 36, 40, 44, 48
        , 52, 53, 54, 58, 62, 66, 70, 74
        , 78, 79, 80, 84, 88, 92, 96, 100
    };

    static const GT_U32 hawkMtiMacPortsArrSize = sizeof(hawkMtiMacPortsArr)/sizeof(hawkMtiMacPortsArr[0]);

    static const GT_U32 hawkMtiSegMacPortsArr[] = {
          /* global MAC ports */
             0, 10,0xff,0xff,0xff,0xff,0xff,0xff
          , 26, 36,0xff,0xff,0xff,0xff,0xff,0xff
          , 52, 62,0xff,0xff,0xff,0xff,0xff,0xff
          , 78, 88
      };

    static const GT_U32 hawkMtiSegMacPortsArrSize = sizeof(hawkMtiSegMacPortsArr)/sizeof(hawkMtiSegMacPortsArr[0]);


    /* Converts USX global port index to it's MAC numbers.
       Global port index represents extended data in unified events for USX ports.
       Formula: global_port_index = port_group * 8 + port_index (where port_group(0-5) and port_index(0-7))
    */
    static const GT_U32 hawkUsxMacPortsArr[] = {
        /* global MAC ports */
           2,  4,  3,  5,  6,  8,  7,  9
        , 10, 12, 11, 13, 14, 16, 15, 17
        , 18, 20, 19, 21, 22, 24, 23, 25
        , 28, 30, 29, 31, 32, 34, 33, 35
        , 36, 38, 37, 39, 40, 42, 41, 43
        , 44, 46, 45, 47, 48, 50, 49, 51
    };

    static const GT_U32 hawkUsxMacPortsArrSize = sizeof(hawkUsxMacPortsArr)/sizeof(hawkUsxMacPortsArr[0]);

    CPSS_NULL_PTR_CHECK_MAC(portNumPtr);

    portNum = *portNumPtr;

    if (evConvertType == PRV_CPSS_EV_CONVERT_DIRECTION_HW_INTERRUPT_TO_UNI_EVENT_E)
    {
        switch (portNum >> PORT_TYPE_OFFSET_BIT)
        {
            case MTI_PORT:
                portIndex = portNum & 0xFF; /* 0...31 */
                if (portIndex >= hawkMtiMacPortsArrSize)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "MTI portIndex[%d] is out of range",
                                                  portIndex);
                }
                *portNumPtr = hawkMtiMacPortsArr[portIndex];
                break;
            case MTI_SEG_PORT:
                portIndex = portNum & 0xFF;
                if (portIndex >= hawkMtiMacPortsArrSize)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "MTI SEG portIndex[%d] is out of range",
                                                  portIndex);
                }
                if (portIndex % 2 == 0) /* 0, 8, 16, 20 */
                {
                    *portNumPtr = hawkMtiMacPortsArr[portIndex];
                }
                else                    /* 1, 9, 17, 21 */
                {
                    portIndex = portIndex + 3;

                    if (portIndex >= hawkMtiMacPortsArrSize)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "MTI SEG portIndex[%d] is out of range",
                                                      portIndex);
                    }

                    *portNumPtr = hawkMtiMacPortsArr[portIndex];
                }
                break;
            case USX_PORT:
                portIndex = portNum & 0xFF; /* 0...47 */
                if (portIndex >= hawkUsxMacPortsArrSize)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "USX portIndex[%d] is out of range",
                                                  portIndex);
                }
                *portNumPtr = hawkUsxMacPortsArr[portIndex];
                break;
            default:
                if (portNum == CPU_PORT)
                {
                    /* No need conversion */
                    break;
                }
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "portNum[%d] type[%d] is not supported",
                    portNum & 0xFF,
                    portNum >> 16);
        }
    }
    else
    {
        /* MAC port */
        portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portNum);
        switch (portMacType)
        {
            case PRV_CPSS_PORT_MTI_100_E:
                *portNumPtr = MTI_PORT << PORT_TYPE_OFFSET_BIT;
                portMacArrayPtr = hawkMtiMacPortsArr;
                portMacArraySize = hawkMtiMacPortsArrSize;
                break;
            case PRV_CPSS_PORT_MTI_400_E:
                *portNumPtr = MTI_SEG_PORT << PORT_TYPE_OFFSET_BIT;
                portMacArrayPtr = hawkMtiSegMacPortsArr;
                portMacArraySize = hawkMtiSegMacPortsArrSize;
                break;
            case PRV_CPSS_PORT_MTI_USX_E:
                *portNumPtr = USX_PORT << PORT_TYPE_OFFSET_BIT;
                portMacArrayPtr = hawkUsxMacPortsArr;
                portMacArraySize = hawkUsxMacPortsArrSize;
                break;
            case PRV_CPSS_PORT_MTI_CPU_E:
                /* No need conversion */
                return GT_OK;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "portNum[%d] wrong type", portNum);
        }

        for (ii = 0; ii < portMacArraySize; ii++)
        {
            if (portNum == portMacArrayPtr[ii])
            {
                *portNumPtr |= ii;
                break;
            }
        }

        if (ii == portMacArraySize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "portNum[%d] wrong MAC port", portNum);
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssHawkDedicatedEventsNonSharedConvertInfoSet function
* @endinternal
*
* @brief   set non-shared callbacks  needed by chIntCauseToUniEvConvertDedicatedTables(...) for the
*          Hawk devices.
*/
void prvCpssHawkDedicatedEventsNonSharedConvertInfoSet(IN GT_U8   devNum)
{
    PRV_INTERRUPT_CTRL_GET(devNum).eventHwIndexToMacNumConvertFunc = prvCpssDxChHawkPortEventPortMapConvert;
    return;
}

/**
* @internal hawkDedicatedEventsConvertInfoSet function
* @endinternal
*
* @brief   set info needed by chIntCauseToUniEvConvertDedicatedTables(...) for the
*          Hawk devices.
*/
static void hawkDedicatedEventsConvertInfoSet(IN GT_U8   devNum)
{
    /* Interrupt cause to unified event map for Hawk with extended data size */
    GT_U32 hawkUniEvMapTableWithExtDataSize;

    hawkUniEvMapTableWithExtDataSize = (sizeof(hawkUniEvMapTableWithExtData)/(sizeof(GT_U32)));
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableWithExtDataArr   = hawkUniEvMapTableWithExtData;

    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableWithExtDataSize  = hawkUniEvMapTableWithExtDataSize;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableArr              = hawkUniEvMapTable;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableSize             = hawkUniEvMapTableSize;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.numReservedPorts              = 0;
    prvCpssHawkDedicatedEventsNonSharedConvertInfoSet(devNum);

    return;
}

static void hawkUpdateIntScanTreeInfo(
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
            CPSS_LOG_ERROR_MAC("Not supported unit [%d] in Hawk", unitId);
        }
        else
        {
            if(baseAddr > 0 && intrScanArr[ii].causeRegAddr > baseAddr)
            {
                /* ERROR : all addresses MUST be 0 based !!! */
                /* ERROR : all addresses MUST be 0 based !!! */
                /* ERROR : all addresses MUST be 0 based !!! */

               /* because we can't mask the address !!! */
               cpssOsPrintf("ERROR : Hawk interrupts tree : NODE hold causeRegAddr [0x%8.8x] but must be relative to the start of the unit [0x%8.8x] \n",
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
* @internal prvCpssDrvPpIntDefDxChHawkMacSecRegIdInit function
* @endinternal
*
* @brief   Init macsec register ids array.
*
* */
static GT_STATUS prvCpssDrvPpIntDefDxChHawkMacSecRegIdInit
(
    GT_VOID
)
{
    GT_U32 iter;

    /* check that Global DB size is in synch with required one */
    if (PRV_CPSS_DXCH_MACSEC_REGID_ARRAY_SIZE_CNS != PRV_CPSS_GLOBAL_DB_AC5P_MACSEC_REGID_ARRAY_SIZE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    for(iter = 0; iter < PRV_CPSS_DXCH_MACSEC_REGID_ARRAY_SIZE_CNS - 1; iter++)
    {
        PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(macSecRegIdNotAccessibleBeforeMacSecInitAc5p[iter],
                                                                            PRV_CPSS_DXCH_MACSEC_REGID_FIRST_CNS + iter);
    }
    /* end identifier for the array */
    PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(macSecRegIdNotAccessibleBeforeMacSecInitAc5p[iter],
                                                                        PRV_CPSS_MACSEC_REGID_LAST_CNS);
    return GT_OK;
}

/**
* @internal prvCpssDrvPpIntDefDxChHawkInit function
* @endinternal
*
* @brief   Interrupts initialization for the Hawk devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefDxChHawkInit
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
        PRV_CPSS_DRV_FAMILY_DXCH_AC5P_E,
        HAWK_NUM_ELEMENTS_IN_SCAN_TREE_CNS,
        hawkIntrScanArr,
        PRV_CPSS_AC5P_LAST_INT_E/32,
        NULL, NULL, NULL
    };

    ppRevision = ppRevision;

    /*set info needed by chIntCauseToUniEvConvertDedicatedTables(...) */
    hawkDedicatedEventsConvertInfoSet(devNum);

    /* init macsec register ids */
    rc = prvCpssDrvPpIntDefDxChHawkMacSecRegIdInit();
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    if(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(hawkInitDone) == GT_TRUE)
    {
        return GT_OK;
    }

    rc = prvCpssDrvDxExMxInterruptsMemoryInit(&devFamilyInterrupstInfo, 1);
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    (void)prvCpssDxChUnitBaseTableInit(CPSS_PP_FAMILY_DXCH_AC5P_E);/*needed for calling prvCpssDxChHwUnitBaseAddrGet(...) */

    /* 1. Align base addresses for mask registers */
    hawkUpdateIntScanTreeInfo(devNum,
        devFamilyInterrupstInfo.intrScanOutArr ,
        devFamilyInterrupstInfo.numScanElements);

    /*
       1. fill the array of mask registers addresses
       2. fill the array of default values for the mask registers
       3. update the bits of nonSumBitMask in the scan tree
    */
    rc = prvCpssDrvExMxDxHwPpMaskRegInfoGet(
            CPSS_PP_FAMILY_DXCH_AC5P_E           ,
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

    devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[devFamilyInterrupstInfo.intFamily]);
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
        PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(macSecRegIdNotAccessibleBeforeMacSecInitAc5p);

    PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(hawkInitDone, GT_TRUE);

    return GT_OK;
}



#ifdef DUMP_DEFAULT_INFO

/**
* @internal prvCpssDrvPpIntDefHawkPrint function
* @endinternal
*
* @brief   print the interrupts arrays info of Hawk devices
*/
void  prvCpssDrvPpIntDefHawkPrint(
    void
)
{
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC *  interruptsScanArray;

    cpssOsPrintf("Hawk - start : \n");
    devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_AC5P_E]);
    interruptsScanArray = devInterruptInfoPtr->interruptsScanArray;
    /* port group 0 */
    prvCpssDrvPpIntDefPrint(devInterruptInfoPtr->numOfScanElements,
                            interruptsScanArray,
                            AC5P_NUM_MASK_REGISTERS_CNS,
                            devInterruptInfoPtr->maskRegistersAddressesArray,
                            devInterruptInfoPtr->maskRegistersDefaultValuesArray);
    cpssOsPrintf("Hawk - End : \n");

}
#endif/*DUMP_DEFAULT_INFO*/

/**
* @internal prvCpssDrvPpIntDefHawkPrint_regInfoByInterruptIndex function
* @endinternal
*
* @brief   print for Hawk devices the register info according to value in
*         PRV_CPSS_AC5P_LAST_INT_E (assuming that this register is part of
*         the interrupts tree)
*/
void  prvCpssDrvPpIntDefHawkPrint_regInfoByInterruptIndex(
    IN PRV_CPSS_AC5P_INT_CAUSE_ENT   interruptId
)
{
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
    GT_CHAR*    deviceNamePtr = "Hawk interrupts";
    GT_U32      numMaskRegisters = PRV_CPSS_AC5P_LAST_INT_E/32;

    cpssOsPrintf("Hawk - start regInfoByInterruptIndex : \n");

    if(interruptId >= PRV_CPSS_AC5P_LAST_INT_E)
    {
        cpssOsPrintf("interruptId[%d] >= maximum(%d) \n" ,
            interruptId, PRV_CPSS_AC5P_LAST_INT_E);
    }
    else
    {
        if(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(hawkInitDone) == GT_FALSE)
        {
            cpssOsPrintf("Hawk - ERROR : the 'IntrScanArr' was not initialized with 'base addresses' yet End \n");
            return;
        }

        if((GT_U32)interruptId >= (numMaskRegisters << 5))
        {
            cpssOsPrintf("[%s] skipped : interruptId[%d] >= maximum(%d) \n" ,
                deviceNamePtr , interruptId, (numMaskRegisters << 5));
        }

        devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_AC5P_E]);

        cpssOsPrintf("start [%s]: \n", deviceNamePtr);
        prvCpssDrvPpIntDefPrint_regInfoByInterruptIndex(devInterruptInfoPtr->numOfScanElements, devInterruptInfoPtr->interruptsScanArray, interruptId, GT_FALSE);
        cpssOsPrintf("ended [%s] \n\n", deviceNamePtr);
    }
    cpssOsPrintf("Hawk - End regInfoByInterruptIndex : \n");
}

/**
* @internal prvCpssDrvHwPpPortGroupAicIsrRead function
* @endinternal
*
* @brief Reads a AIC(EIP-163/164/66) register value using special interrupt address
*        completion region in the specific port group in the device and clears of the interrupt bits.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS prvCpssDrvHwPpPortGroupAicIsrRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
)
{
    GT_STATUS rc = GT_OK;

    /* Read AIC_ENABLE_CTRL cause register */
    rc = prvCpssDrvHwPpPortGroupIsrRead(devNum, portGroupId,regAddr, dataPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* AIC_ACK register is at offset 0x1C below to AIC_RAW_STAT cause register */
    regAddr -= 0x1C;

    /* Write back AIC_ACK register to acknowledge interrupt signal and clear the status bit */
    rc = prvCpssDrvHwPpPortGroupIsrWrite(devNum, portGroupId, regAddr, *dataPtr);

    return rc;

}

/**
* @internal prvCpssDrvHwPpSerdesIsrRead function
* @endinternal
*
* @brief Reads Serdes interrupt register value, clears and restores the ECC bits of Serdes General Control 0 register.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
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
GT_STATUS prvCpssDrvHwPpSerdesIsrRead
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

    /* SERDES interrupt cause register bits [10:9] denote SRAM DOUBLE/SINGLE ECC error */
    eccSet = ((*dataPtr) >> 9) & 0x3;

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
* @internal prvCpssPpDrvIntDefDxChHawkSdmaInterruptsArraysGet function
* @endinternal
*
* @brief   Get CpuPort SDMA Interrupts arrays
*
* @param[out] interruptRxBufQueArrPtr   - (pointer to) HW interrupt IDs RX BUF_QUEUE tile.mg.queue ordered.
* @param[out] interruptRxErrQueArrPtr   - (pointer to) HW interrupt IDs RX ERR tile.mg.queue ordered.
* @param[out] interruptRxCntrQueArrPtr  - (pointer to) HW interrupt IDs RX CNTR tile.mg ordered.
* @param[out] interruptTxBufQueArrPtr   - (pointer to) HW interrupt IDs TX BUFFER tile.mg ordered.
* @param[out] interruptTxErrQueArrPtr   - (pointer to) HW interrupt IDs TX ERROR tile.mg ordered.
* @param[out] interruptTxEndQueArrPtr   - (pointer to) HW interrupt IDs TX_END tile.mg ordered.
*/
GT_STATUS prvCpssPpDrvIntDefDxChHawkSdmaInterruptsArraysGet
(
    OUT const GT_U32 ** interruptRxBufQueArrPtr,
    OUT const GT_U32 ** interruptRxErrQueArrPtr,
    OUT const GT_U32 ** interruptTxBufQueArrPtr,
    OUT const GT_U32 ** interruptTxErrQueArrPtr,
    OUT const GT_U32 ** interruptTxEndQueArrPtr
)
{
    #define AC5P_INT_NAME_RX_SDMA_QUEUE(type, local_mgUnitId, localQueue) \
        PRV_CPSS_AC5P_MG_##local_mgUnitId##_RECEIVE_SDMA_INTERRUPT_CAUSE0_##type##_QUEUE_##localQueue##_E

    #define AC5P_INT_NAME_RX_SDMA_MG(type, local_mgUnitId)      \
        AC5P_INT_NAME_RX_SDMA_QUEUE(type, local_mgUnitId, 0),   \
        AC5P_INT_NAME_RX_SDMA_QUEUE(type, local_mgUnitId, 1),   \
        AC5P_INT_NAME_RX_SDMA_QUEUE(type, local_mgUnitId, 2),   \
        AC5P_INT_NAME_RX_SDMA_QUEUE(type, local_mgUnitId, 3),   \
        AC5P_INT_NAME_RX_SDMA_QUEUE(type, local_mgUnitId, 4),   \
        AC5P_INT_NAME_RX_SDMA_QUEUE(type, local_mgUnitId, 5),   \
        AC5P_INT_NAME_RX_SDMA_QUEUE(type, local_mgUnitId, 6),   \
        AC5P_INT_NAME_RX_SDMA_QUEUE(type, local_mgUnitId, 7)

    #define AC5P_INT_NAME_RX_SDMA(type)                         \
        AC5P_INT_NAME_RX_SDMA_MG(type, 0),                      \
        AC5P_INT_NAME_RX_SDMA_MG(type, 1),                      \
        AC5P_INT_NAME_RX_SDMA_MG(type, 2),                      \
        AC5P_INT_NAME_RX_SDMA_MG(type, 3)

    #define AC5P_INT_NAME_SINGLE_MG_SINGLE_TYPE(mgIndex,type)                           \
        PRV_CPSS_AC5P_MG_##mgIndex##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_##type##_0_E,       \
        PRV_CPSS_AC5P_MG_##mgIndex##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_##type##_1_E,       \
        PRV_CPSS_AC5P_MG_##mgIndex##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_##type##_2_E,       \
        PRV_CPSS_AC5P_MG_##mgIndex##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_##type##_3_E,       \
        PRV_CPSS_AC5P_MG_##mgIndex##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_##type##_4_E,       \
        PRV_CPSS_AC5P_MG_##mgIndex##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_##type##_5_E,       \
        PRV_CPSS_AC5P_MG_##mgIndex##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_##type##_6_E,       \
        PRV_CPSS_AC5P_MG_##mgIndex##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_##type##_7_E

    #define AC5P_INT_NAME_TX_SDMA(type)                                                 \
        AC5P_INT_NAME_SINGLE_MG_SINGLE_TYPE(0, type),                                   \
        AC5P_INT_NAME_SINGLE_MG_SINGLE_TYPE(1, type),                                   \
        AC5P_INT_NAME_SINGLE_MG_SINGLE_TYPE(2, type),                                   \
        AC5P_INT_NAME_SINGLE_MG_SINGLE_TYPE(3, type)

    /* HW interrupt IDs RX BUF_QUEUE tile.mg.queue ordered */
    static const GT_U32 hawkInterruptRxBufQueArr[] =
    {
        AC5P_INT_NAME_RX_SDMA(RX_BUFFER)                                                \
    };

    /* HW interrupt IDs RX ERR tile.mg.queue ordered */
    static const GT_U32 hawkInterruptRxErrQueArr[] =
    {
        AC5P_INT_NAME_RX_SDMA(RX_ERROR)                                                 \
    };

    /* HW interrupt IDs TX BUFFER tile.mg ordered */
    static const GT_U32 hawkInterruptTxBufQueArr[] =
    {
        AC5P_INT_NAME_TX_SDMA(TX_BUFFER_QUEUE)                                          \
    };

    /* HW interrupt IDs TX ERROR tile.mg ordered */
    static const GT_U32 hawkInterruptTxErrQueArr[] =
    {
        AC5P_INT_NAME_TX_SDMA(TX_ERROR_QUEUE)                                           \
    };

    /* HW interrupt IDs TX_END tile.mg ordered */
    static const GT_U32 hawkInterruptTxEndQueArr[] =
    {
        AC5P_INT_NAME_TX_SDMA(TX_END_QUEUE)                                             \
    };

    *interruptRxBufQueArrPtr  = hawkInterruptRxBufQueArr;
    *interruptRxErrQueArrPtr  = hawkInterruptRxErrQueArr;
    *interruptTxBufQueArrPtr  = hawkInterruptTxBufQueArr;
    *interruptTxErrQueArrPtr  = hawkInterruptTxErrQueArr;
    *interruptTxEndQueArrPtr  = hawkInterruptTxEndQueArr;


    #undef AC5P_INT_NAME_RX_SDMA_QUEUE
    #undef AC5P_INT_NAME_RX_SDMA_MG
    #undef AC5P_INT_NAME_RX_SDMA
    #undef AC5P_INT_NAME_SINGLE_MG_RX_SDMA_MAC
    #undef AC5P_INT_NAME_TX_SDMA
    #undef AC5P_INT_NAME_SINGLE_MG_SINGLE_TYPE

    return GT_OK;
}
