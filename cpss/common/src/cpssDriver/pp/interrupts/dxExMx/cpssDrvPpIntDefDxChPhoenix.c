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
* @file cpssDrvPpIntDefDxChPhoenix.c
*
* @brief This file includes the definition and initialization of the interrupts
* init. parameters tables. -- DxCh AC5X (a.k.a. Phoenix) devices
*
* @version   1
********************************************************************************
*/

#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsAlleyCat5X.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxUtilLion.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddr.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* First macsec register id as per Phoenix interrupts enum */
#define  PRV_CPSS_DXCH_AC5X_MACSEC_REGID_FIRST_CNS (PRV_CPSS_AC5X_MSEC_EGR_163_INST_0_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E >> 5)

#define  PRV_CPSS_DXCH_UNIT_TAI0_E     PRV_CPSS_DXCH_UNIT_TAI_E

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

#define PPG_OFFSET(ppg)            (0x80000 * (ppg))
#define PHA_PPN_OFFSET(ppg, ppn)   (PPG_OFFSET(ppg) + 0x4000 * (ppn))

/* convert bit in the enum of the huge interrupts of the device into BIT_0..BIT_31 in a register */
#define DEVICE_INT_TO_BIT_MAC(deviceInterruptId)    \
            (1 << ((deviceInterruptId) & 0x1f))

#define FDB_REG_MASK_CNS        \
    (0xFFFFFFFF & ~                                                             \
        (DEVICE_INT_TO_BIT_MAC(PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AGE_VIA_TRIGGER_ENDED_E) |     \
         DEVICE_INT_TO_BIT_MAC(PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AU_PROC_COMPLETED_INT_E)))

/* unit : PRV_CPSS_DXCH_UNIT_MG_E */
#define MG_UNIT(unit,_mgIndex) \
    (_mgIndex ? (PRV_CPSS_DXCH_UNIT_MG_0_1_E + (_mgIndex-1)) : PRV_CPSS_DXCH_UNIT_MG_E)

#define UNIT_ID_GET(unit) ((unit) & 0xFFFF)

#define LMU_LATENCY_INTERRUPT_CAUSE_MAC(_threshold)            \
        (0x8100 + (0x4 * (_threshold)))

#define LMU_LATENCY_INTERRUPT_MASK_MAC(_threshold)             \
        (0x8180 + (0x4 * (_threshold)))

/* USX MAC - cause */
#define PORT_USX_MAC_INTERRUPT_CAUSE_MAC(port)                        \
        (0x38 + ((port) * 0x28))
/* USX MAC - mask */
#define PORT_USX_MAC_INTERRUPT_MASK_MAC(port)                         \
        (0x3C + ((port) * 0x28))

/* 100G - cause */
#define PORT_100G_MAC_INTERRUPT_CAUSE_MAC(port)                        \
        (0x94 + ((port) * 0x18))
/* 100G - mask */
#define PORT_100G_MAC_INTERRUPT_MASK_MAC(port)                          \
        (0x98 + ((port) * 0x18))

/* CSTU Channel group - cause */
#define CSTU_CHANNEL_GROUP_INTERRUPT_CAUSE_MAC(group)                   \
        (0x3800 + ((group) * 0x4))

/* CSTU Channel - mask */
#define CSTU_CHANNEL_GROUP_INTERRUPT_MASK_MAC(group)                    \
        (0x3820 + ((group) * 0x4))

/* CSTU Channel - cause */
#define CSTU_CHANNEL_INTERRUPT_CAUSE_MAC(channel)                       \
        (0x3000 + ((channel) * 0x4))

/* CSTU Channel - mask */
#define CSTU_CHANNEL_INTERRUPT_MASK_MAC(channel)                        \
        (0x3400 + ((channel) * 0x4))

#define   mg0InternalIntsSum_SUB_TREE_MAC(bit, _mgIndex)                          \
    /* Miscellaneous */                                                                             \
    {bit, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL,                \
        0x00000038, 0x0000003C,                                                           \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5X_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_I2C_TIME_OUT_INTERRUPT_E,                   \
        PRV_CPSS_AC5X_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_SERDES_ADDRESS_OUT_OF_RANGE_VIOLATION_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                     \
        /* HostCpuDoorbellIntSum - single register per MG accessed from MG Internal Cause0/1/2/3 */ \
        {24, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E, _mgIndex), NULL,           \
            0x00000518, 0x0000051c,                                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
            PRV_CPSS_AC5X_MG##_mgIndex##_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_0_E,        \
            PRV_CPSS_AC5X_MG##_mgIndex##_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_30_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 0, NULL, NULL}

#define   mt_SUB_TREE_MAC(bit)                                                  \
    /* MT FDBIntSum - FDB Interrupt Cause */                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_FDB_E, NULL,                \
        0x000001B0, 0x000001B4,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NUM_OF_HOP_EX_P_E,               \
        PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_ADDRESS_OUT_OF_RANGE_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, FDB_REG_MASK_CNS,                                   \
             0, NULL, NULL}

#define   em_SUB_TREE_MAC(bit)                                                          \
    /* EM Interrupt Cause */                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EM_E, NULL,                                      \
        0x00004000, 0x00004004,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AC5X_EM_EM_INTERRUPT_CAUSE_REGISTER_INDIRECT_ADDRESS_OUT_OF_RANGE_E,               \
        PRV_CPSS_AC5X_EM_EM_INTERRUPT_CAUSE_REGISTER_CHANNEL_1_SHM_ANSWER_FIFO_OVERFLOW_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, FDB_REG_MASK_CNS,                          \
             0, NULL, NULL}

#define   epcl_tai_SUB_TREE_MAC(bit, _tai)                                              \
    /* EPCL TAI  */                                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EPCL_HA_TAI##_tai##_E, NULL,                 \
        0x00000000, 0x00000004,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5X_PHA_EPCL_TAI_2_TAI_INST_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,        \
        PRV_CPSS_AC5X_PHA_EPCL_TAI_2_TAI_INST_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   pha_tai_SUB_TREE_MAC(bit, _tai)                                               \
    /* PHA TAI  */                                                                  \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_TAI##_tai##_E, NULL,                     \
        0x00000000, 0x00000004,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5X_PHA_EPCL_TAI_3_TAI_INST_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,        \
        PRV_CPSS_AC5X_PHA_EPCL_TAI_3_TAI_INST_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define  l2i_tai_SUB_TREE_MAC(bit, _tai)                                                                \
    /* L2I TAI */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_L2I_TAI##_tai##_E, NULL,                                         \
        0x00000000, 0x00000004,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_L2I_TAI_1_TAI_INST_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,                 \
        PRV_CPSS_AC5X_L2I_TAI_1_TAI_INST_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   bma_SUB_TREE_MAC(bit)                                                     \
        /* bma  */                                                                  \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_BMA_E, NULL,                             \
            0x00059000, 0x00059004,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_AC5X_BMA_MC_CNT_RMW_CLEAR_FIFO_FULL_E,                         \
            PRV_CPSS_AC5X_BMA_FINAL_CLEAR_UDB_FIFO0_FULL_E,                         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   cnc_SUB_TREE_MAC(bit)                                                                             \
    /* CNC-0  */                                                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_CNC_0_E, NULL,                                                       \
        0x00000100, 0x00000104,                                                                             \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                     \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                    \
        PRV_CPSS_AC5X_CNC_INST_0_CNC_INTERRUPT_SUMMARY_CAUSE_REGISTER_WRAPAROUND_FUNC_INTERRUPT_SUM_E,      \
        PRV_CPSS_AC5X_CNC_INST_0_CNC_INTERRUPT_SUMMARY_CAUSE_REGISTER_MISC_FUNC_INTERRUPT_SUM_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                             \
                                                                                                            \
        /* WraparoundFuncInterruptSum  */                                                                   \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_CNC_0_E, NULL,                                                     \
            0x00000190, 0x000001A4,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5X_CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_0_WRAPAROUND_E,   \
            PRV_CPSS_AC5X_CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_15_WRAPAROUND_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* RateLimitFuncInterruptSum  */                                                                    \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_CNC_0_E, NULL,                                                     \
            0x000001B8, 0x000001CC,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5X_CNC_INST_0_RATE_LIMIT_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_NUMBER_0_RATE_LIMIT_FIFO_DROP_E,   \
            PRV_CPSS_AC5X_CNC_INST_0_RATE_LIMIT_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_NUMBER_15_RATE_LIMIT_FIFO_DROP_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* MiscFuncInterruptSum  */                                                                         \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_CNC_0_E, NULL,                                                     \
            0x000001E0, 0x000001E4,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5X_CNC_INST_0_MISC_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_DUMP_FINISHED_E,              \
            PRV_CPSS_AC5X_CNC_INST_0_MISC_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_CNC_TABLE_OVERLAPPING_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define pdx_SUB_TREE_MAC(bit)                                                   \
    /* PDX Interrupt Summary */                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PDX_E, NULL,            \
        0x00001408, 0x0000140c,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AC5X_TXQ_PR_PDX_FUNCTIONAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                  \
        PRV_CPSS_AC5X_TXQ_PR_PDX_FUNCTIONAL_INTERRUPT_CAUSE_CHANNELS_RANGE_0_DX_DESC_DROP_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pfcc_SUB_TREE_MAC(bit)                                              \
    /* PFCC Interrupt Summary */                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E, NULL,       \
        0x00000100, 0x00000104,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5X_TXQ_PR_PFCC_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,         \
        PRV_CPSS_AC5X_TXQ_PR_PFCC_INTERRUPT_CAUSE_SOURCE_PORT_COUNTER_OVERFLOW_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define psi_SUB_TREE_MAC(bit)                                               \
    /* PSI Interrupt Summary */                                                     \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_PSI_E, NULL,        \
        0x00000020c, 0x00000210,                                                    \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_AC5X_PSI_REGS_PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_E,           \
        PRV_CPSS_AC5X_PSI_REGS_PSI_INTERRUPT_CAUSE_PDQ_ACCESS_MAP_ERROR_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   eft_SUB_TREE_MAC(bit)                                       \
        /* eft  */                                                                    \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EGF_EFT_E, NULL,    \
            0x000010A0, 0x000010B0,                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                           \
            prvCpssDrvHwPpPortGroupIsrWrite,                                          \
            PRV_CPSS_AC5X_EGF_EFT_INTERRUPTS_CAUSE_EGRESS_WRONG_ADDR_INT_E,       \
            PRV_CPSS_AC5X_EGF_EFT_INTERRUPTS_CAUSE_INC_BUS_IS_TOO_SMALL_INT_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   eoam_SUB_TREE_MAC(bit)                                    \
        /* egress OAM  */                                                           \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EOAM_E, NULL,     \
            0x000000F0, 0x000000F4,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_AC5X_EOAM_OAM_UNIT_INTERRUPT_CAUSE_KEEP_ALIVE_AGING_E,      \
            PRV_CPSS_AC5X_EOAM_OAM_UNIT_INTERRUPT_CAUSE_TX_PERIOD_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   epcl_SUB_TREE_MAC(bit)                                        \
        /* EPCL  */                                                                     \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EPCL_E, NULL,         \
            0x00000010, 0x00000014,                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_AC5X_EPCL_EPCL_INTERRUPT_CAUSE_REGFILE_ADDRESS_ERROR_E,        \
            PRV_CPSS_AC5X_EPCL_EPCL_INTERRUPT_CAUSE_TABLE_OVERLAP_ACCESS_ERROR_E,   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eplr_SUB_TREE_MAC(bit)                                        \
        /* eplr  */                                                                     \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EPLR_E, NULL,         \
            0x00000200, 0x00000204, \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_AC5X_EPLR_POLICER_INTERRUPT_CAUSE_ILLEGAL_OVERLAPPED_ACCESS_E,       \
            PRV_CPSS_AC5X_EPLR_POLICER_INTERRUPT_CAUSE_QOS_ATTRIBUTES_TABLE_WRONG_ADDRESS_INTERRUPT_E,    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   ermrk_SUB_TREE_MAC(bit)                                       \
    /* ERMRK  */                                                                        \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ERMRK_E, NULL,        \
            0x00000004, 0x00000008,     \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_AC5X_ERMRK_ERMRK_INTERRUPT_CAUSE_REGFILE_ADDRESS_ERROR_E,          \
            PRV_CPSS_AC5X_ERMRK_ERMRK_INTERRUPT_CAUSE_TABLE_OVERLAP_ACCESS_ERROR_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   l2i_SUB_TREE_MAC(bit)                                             \
        /* L2 Bridge  */                                                                    \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_L2I_E,   NULL,             \
            0x00002100, 0x00002104,                                           \
            prvCpssDrvHwPpPortGroupIsrRead,                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                \
            PRV_CPSS_AC5X_L2I_BRIDGE_INTERRUPT_CAUSE_ADDRESS_OUT_OF_RANGE_E,   \
            PRV_CPSS_AC5X_L2I_BRIDGE_INTERRUPT_CAUSE_UPDATE_SECURITY_BREACH_REGISTER_INT_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* node for ingress SCT rate limiters interrupts leaf */
#define SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(index)                                  \
        {(index+1), GT_FALSE, PRV_CPSS_DXCH_UNIT_EQ_E, NULL,                            \
            0x00010020 + ((index) * 0x10), 0x00010024 + ((index) * 0x10),               \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_AC5X_EQ_REG_1_PORT_0_PKT_DROPED_INT_E   + ((index) * 32),          \
            PRV_CPSS_AC5X_EQ_REG_1_PORT_15_PKT_DROPED_INT_E  + ((index) * 32),          \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   eqSctRateLimiters_SUB_TREE_MAC(bit)                           \
        /* SCT Rate Limiters */                                                         \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EQ_E, NULL,           \
            0x00010000, 0x00010004,                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_AC5X_EQ_CPU_CODE_RATE_LIMITERS_INTERRUPT_SUMMARY_CAUSE_REG_1_CPU_CODE_RATE_LIMITER_INT_E,         \
            PRV_CPSS_AC5X_EQ_CPU_CODE_RATE_LIMITERS_INTERRUPT_SUMMARY_CAUSE_REG_16_CPU_CODE_RATE_LIMITER_INT_E,        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                    \
                                                                                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(0),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(1),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(2),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(3),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(4),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(5),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(6),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(7),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(8),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(9),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(10),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(11),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(12),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(13),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(14),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(15)

#define   eq_SUB_TREE_MAC(bit)                                          \
        /* Pre-Egress Interrupt Summary Cause */                                        \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EQ_E, NULL,                                  \
            0x00000058, 0x0000005C,     \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_AC5X_EQ_PRE_EGRESS_INTERRUPT_SUMMARY_CAUSE_INGRESS_STC_INT_E,      \
            PRV_CPSS_AC5X_EQ_PRE_EGRESS_INTERRUPT_SUMMARY_CAUSE_PORT_DLB_INTERRUPT_E,   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                     \
            eqSctRateLimiters_SUB_TREE_MAC(2)


#define  ha_SUB_TREE_MAC(bit)                                                           \
        /* HA Rate Limiters */                                                          \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_HA_E, NULL,                                    \
            0x00000300, 0x00000304,                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_AC5X_HA_HA_INTERRUPT_CAUSE_TABLE_OVERLAP_ACCESS_ERROR_E,           \
            PRV_CPSS_AC5X_HA_HA_INTERRUPT_CAUSE_OVERSIZE_TUNNEL_HEADER_E,               \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   ioam_SUB_TREE_MAC(bit)                                        \
        /* ingress OAM  */                                                              \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IOAM_E, NULL,         \
            0x000000F0, 0x000000F4, \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_AC5X_IOAM_OAM_UNIT_INTERRUPT_CAUSE_KEEP_ALIVE_AGING_E, \
            PRV_CPSS_AC5X_IOAM_OAM_UNIT_INTERRUPT_CAUSE_TX_PERIOD_E,        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   iplr0_SUB_TREE_MAC(bit)                                       \
        /* iplr0 */                                                                     \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IPLR_E, NULL,                                \
            0x00000200, 0x00000204,                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_AC5X_IPLR0_POLICER_INTERRUPT_CAUSE_ILLEGAL_OVERLAPPED_ACCESS_E,              \
            PRV_CPSS_AC5X_IPLR0_POLICER_INTERRUPT_CAUSE_QOS_ATTRIBUTES_TABLE_WRONG_ADDRESS_INTERRUPT_E,            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   iplr1_SUB_TREE_MAC(bit)                                   \
        /* iplr1 */                                                                 \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IPLR_1_E, NULL,   \
            0x00000200, 0x00000204,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_AC5X_IPLR1_POLICER_INTERRUPT_CAUSE_ILLEGAL_OVERLAPPED_ACCESS_E,              \
            PRV_CPSS_AC5X_IPLR1_POLICER_INTERRUPT_CAUSE_QOS_ATTRIBUTES_TABLE_WRONG_ADDRESS_INTERRUPT_E,            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   router_SUB_TREE_MAC(bit)                                         \
        /* router */                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IPVX_E, NULL,            \
            0x00000970, 0x00000974,\
            prvCpssDrvHwPpPortGroupIsrRead,                                                \
            prvCpssDrvHwPpPortGroupIsrWrite,                                               \
            PRV_CPSS_AC5X_IPVX_ROUTER_INTERRUPT_CAUSE_I_PV_X_BAD_ADDR_E,                   \
            PRV_CPSS_AC5X_IPVX_ROUTER_INTERRUPT_CAUSE_ILLEGAL_OVERLAP_ACCESS_E,            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   lpm_SUB_TREE_MAC(bit)                                         \
        /* LPM */                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_LPM_E, NULL,          \
            0x00F00120, 0x00F00130, \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_AC5X_LPM_LPM_GENERAL_INT_CAUSE_LPM_WRONG_ADDRESS_E,         \
            PRV_CPSS_AC5X_LPM_LPM_GENERAL_INT_CAUSE_MG_TO_LPM_FORBIDDEN_WRITE_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   mll_SUB_TREE_MAC(bit)                                         \
        /* MLL */                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MLL_E, NULL,                                 \
            0x00000030, 0x00000034, \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_AC5X_MLL_MLL_INTERRUPT_CAUSE_REGISTER_INVALID_IP_MLL_ACCESS_INTERRUPT_E,         \
            PRV_CPSS_AC5X_MLL_MLL_INTERRUPT_CAUSE_REGISTER_ILLEGAL_TABLE_OVERLAP_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   pcl_SUB_TREE_MAC(bit)                                                         \
        /* PCL */                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCL_E, NULL,          \
            0x00000004, 0x00000008, \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_AC5X_PCL_PCL_UNIT_INTERRUPT_CAUSE_MG_ADDR_OUTOF_RANGE_E,         \
            PRV_CPSS_AC5X_PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL_ILLEGAL_TABLE_OVERLAP_E,    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   ppu_SUB_TREE_MAC(bit)                                                         \
        /* PPU */                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PPU_E, NULL,                                 \
            0x00000004, 0x00000008,                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_AC5X_PPU_PPU_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,             \
            PRV_CPSS_AC5X_PPU_PPU_INTERRUPT_CAUSE_INT_INDEX_7_PPU_ACTION_INT_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   preq_SUB_TREE_MAC(bit)                                                 \
        /* PREQ */                                                                               \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PREQ_E, NULL,                  \
                0x00000600, 0x00000604, \
                prvCpssDrvHwPpPortGroupIsrRead,                                                  \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                 \
                PRV_CPSS_AC5X_PREQ_PREQ_INTERRUPT_CAUSE_PREQ_REGFILE_ADDRESS_ERROR_E,    \
                PRV_CPSS_AC5X_PREQ_PREQ_INTERRUPT_CAUSE_PREQ_TABLE_ACCESS_OVERLAP_ERROR_E,       \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   qag_SUB_TREE_MAC(bit)                                         \
        /* qag */                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EGF_QAG_E, NULL,      \
            0x000B0000, 0x000B0010, \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_AC5X_EGF_QAG_INTERRUPTS_CAUSE_WRONG_ADDRESS_INTERRUPT_E,                \
            PRV_CPSS_AC5X_EGF_QAG_INTERRUPTS_CAUSE_TABLE_OVERLAPING_INTERRUPT_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   sht_SUB_TREE_MAC(bit)                                                             \
        /* SHT */                                                                           \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EGF_SHT_E, NULL,                                 \
            0x00100010, 0x00100020,                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                \
            PRV_CPSS_AC5X_EGF_SHT_INTERRUPTS_CAUSE_WRONG_ADDRESS_INTERRUPT_E,               \
            PRV_CPSS_AC5X_EGF_SHT_INTERRUPTS_CAUSE_TABLE_OVERLAPPING_INTERRUPT_E,           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define PHA_PPN_NODE_MAC(bit, _ppg, _ppn)                                       \
    /* PPN Internal Error Cause */                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E, NULL,                             \
        0x00003040 + PHA_PPN_OFFSET(_ppg, _ppn), 0x00003044 + PHA_PPN_OFFSET(_ppg, _ppn), \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_AC5X_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_HOST_UNMAPPED_ACCESS_E, \
        PRV_CPSS_AC5X_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_DOORBELL_INTERRUPT_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define phaPpg_SUB_TREE_MAC(bit, _ppg)                                          \
    /* PPG Internal Error Cause */                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E, NULL,                             \
        0x0007FF00 + PPG_OFFSET(_ppg), 0x0007FF04 + PPG_OFFSET(_ppg),           \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_AC5X_PPG_##_ppg##_PPG_INTERRUPT_SUM_CAUSE_PPG_FUNCTIONAL_SUMMARY_INTERRUPT_E, \
        PRV_CPSS_AC5X_PPG_##_ppg##_PPG_INTERRUPT_SUM_CAUSE_PPN_7_SUMMARY_INTERRUPT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 9, NULL, NULL},                 \
        /*  PPG Internal Error Cause */                                         \
            {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E, NULL,                       \
            0x0007FF10 + PPG_OFFSET(_ppg), 0x0007FF14 + PPG_OFFSET(_ppg),       \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_AC5X_PPG_##_ppg##_PPG_INTERNAL_ERROR_CAUSE_PPG_BAD_ADDRESS_ERROR_E,          \
            PRV_CPSS_AC5X_PPG_##_ppg##_PPG_INTERNAL_ERROR_CAUSE_PPG_TABLE_ACCESS_OVERLAP_ERROR_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},             \
            PHA_PPN_NODE_MAC(2, _ppg, 0),                                       \
            PHA_PPN_NODE_MAC(3, _ppg, 1),                                       \
            PHA_PPN_NODE_MAC(4, _ppg, 2),                                       \
            PHA_PPN_NODE_MAC(5, _ppg, 3),                                       \
            PHA_PPN_NODE_MAC(6, _ppg, 4),                                       \
            PHA_PPN_NODE_MAC(7, _ppg, 5),                                       \
            PHA_PPN_NODE_MAC(8, _ppg, 6),                                       \
            PHA_PPN_NODE_MAC(9, _ppg, 7)

#define   phaPpa_SUB_TREE_MAC(bit)                                              \
        /* PHA PPA */                                                               \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E, NULL,                             \
            0x007EFF10, 0x007EFF14,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_AC5X_PPA_PPA_INTERRUPT_SUM_CAUSE_PPA_INTERNAL_FUNCTIONAL_SUMMARY_INTERRUPT_E,    \
            PRV_CPSS_AC5X_PPA_PPA_INTERRUPT_SUM_CAUSE_PPG_1_SUMMARY_INTERRUPT_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                 \
            /* PPA Internal Error Cause */                                          \
            {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E, NULL,                           \
                0x007EFF20, 0x007EFF24,                                             \
                prvCpssDrvHwPpPortGroupIsrRead,                                     \
                prvCpssDrvHwPpPortGroupIsrWrite,                                    \
                PRV_CPSS_AC5X_PPA_PPA_INTERNAL_ERROR_CAUSE_PPA_BAD_ADDRESS_ERRROR_E,\
                PRV_CPSS_AC5X_PPA_PPA_INTERNAL_ERROR_CAUSE_PPA_BAD_ADDRESS_ERRROR_E,\
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},             \
            phaPpg_SUB_TREE_MAC(2, 0),                                              \
            phaPpg_SUB_TREE_MAC(3, 1)

#define   phaInternal_NODE_MAC(bit)                                             \
        /* PHA Internal Error Cause */                                              \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E, NULL,                             \
            0x007FFF80, 0x007FFF84,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_AC5X_PHA_PHA_INTERNAL_ERROR_CAUSE_PHA_BAD_ADDRESS_ERROR_E,     \
            PRV_CPSS_AC5X_PHA_PHA_INTERNAL_ERROR_CAUSE_PPA_CLOCK_DOWN_VIOLATION_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   pha_SUB_TREE_MAC(bit)                                                                         \
        /* PHA */                                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PHA_E, NULL,                                                 \
            0x007FFF88, 0x007FFF8C,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5X_PHA_PHA_INTERRUPT_SUM_CAUSE_PHA_INTERNAL_FUNCTIONAL_SUMMARY_INTERRUPT_E,      \
            PRV_CPSS_AC5X_PHA_PHA_INTERRUPT_SUM_CAUSE_PPA_SUMMARY_INTERRUPT_E,                          \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                     \
            phaInternal_NODE_MAC(1),                                                                    \
            phaPpa_SUB_TREE_MAC(2)

#define   tti_SUB_TREE_MAC(bit)                                                                         \
        /* TTI */                                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TTI_E, NULL,                                                 \
            0x00000004, 0x00000008,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5X_TTI_TTI_ENGINE_INTERRUPT_CAUSE_CPU_ADDRESS_OUT_OF_RANGE_E,                    \
            PRV_CPSS_AC5X_TTI_TTI_ENGINE_INTERRUPT_CAUSE_TABLE_OVERLAP_ERROR_E,                         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   hbu_SUB_TREE_MAC(bit)                                                                          \
        /* HBU */                                                                                        \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_HBU_E, NULL,                                                  \
            0x00000000, 0x00000004,                                                                      \
            prvCpssDrvHwPpPortGroupIsrRead,                                                              \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
            PRV_CPSS_AC5X_HBU_HBU_INTERRUPT_CAUSE_REGISTER_ERROR_FHF_MEM_E,                              \
            PRV_CPSS_AC5X_HBU_HBU_INTERRUPT_CAUSE_REGISTER_HEADER_FIFO_EMPTY_E,                          \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txq_tai_SUB_TREE_MAC(bit)                                                                     \
        /* TAI */                                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQS_TAI0_E, NULL,                                                \
            0x00000000, 0x00000004,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5X_TXQ_TAI_4_TAI_INST_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,                    \
            PRV_CPSS_AC5X_TXQ_TAI_4_TAI_INST_0_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,                 \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   erep_SUB_TREE_MAC(bit)                                                                        \
        /* EREP Rate Limiters */                                                                        \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EREP_E, NULL,                                                \
            0x00003000, 0x00003004,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5X_EREP_EREP_INTERRUPTS_CAUSE_EREP_CPU_ADDRESS_OUT_OF_RANGE_E,                   \
            PRV_CPSS_AC5X_EREP_EREP_INTERRUPTS_CAUSE_MIRROR_REPLICATION_NOT_PERFORMED_E,                \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* 6 elements */
#define gop_tai_units                                                                                   \
        gop_usx2_1_mac_macro_tai_SUB_TREE_MAC(1, 0),                                                    \
        gop_usx2_1_mac_macro_tai_SUB_TREE_MAC(2, 1),                                                    \
        gop_usx2_mac_cpu_macro_cpu_tai_SUB_TREE_MAC(3, 0),                                              \
        gop_usx2_mac_cpu_macro_cpu_tai_SUB_TREE_MAC(4, 1),                                              \
        gop_usx2_mac_cpu_macro_mac_tai_SUB_TREE_MAC(5, 0),                                              \
        gop_usx2_mac_cpu_macro_mac_tai_SUB_TREE_MAC(6, 1)

/* 5 elements */
#define gop_led_units                                                                                   \
        gop_led_usx_SUB_TREE_MAC(7, 0),                                                                 \
        gop_led_usx_SUB_TREE_MAC(8, 1),                                                                 \
        gop_led_cpu_SUB_TREE_MAC(9, 2),                                                                 \
        gop_led_cpu_SUB_TREE_MAC(10, 3),                                                                \
        gop_led_100G_SUB_TREE_MAC(11, 4)

/* 1 element */
#define gop_lmu_units                                                                                   \
        gop_lmu_SUB_TREE_MAC(12)

/* 9 elements */
#define gop_mac_pcs                                                                                  \
        gop_usx_mac_pcs_0_SUB_TREE_MAC(13),                                                          \
        gop_usx_mac_pcs_1_SUB_TREE_MAC(14),                                                          \
        gop_usx_mac_pcs_2_SUB_TREE_MAC(15),                                                          \
        gop_usx_mac_pcs_3_SUB_TREE_MAC(16),                                                          \
        gop_usx_mac_pcs_4_SUB_TREE_MAC(17),                                                          \
        gop_usx_mac_pcs_5_SUB_TREE_MAC(18),                                                          \
        gop_mac_pcs_cpu_SUB_TREE_MAC(19, 0),                                                         \
        gop_mac_pcs_cpu_SUB_TREE_MAC(20, 1),                                                         \
        gop_mac_pcs_100G_SUB_TREE_MAC(21)

/* 5 elements */
#define gop_mif                                                                                 \
        gop_mif_usx_SUB_TREE_MAC(22, 0),                                                             \
        gop_mif_usx_SUB_TREE_MAC(23, 1),                                                             \
        gop_mif_usx2_SUB_TREE_MAC(24, 2),                                                            \
        gop_mif_usx_cpu_SUB_TREE_MAC(25),                                                         \
        gop_mif_usx_100G_SUB_TREE_MAC(26)
/* 2 elements */
#define gop_pzarb                                                                               \
        gop_pzarb_SUB_TREE_MAC(27, 0),                                                               \
        gop_pzarb_SUB_TREE_MAC(28, 1)
/* 3 elements */
#define gop_sdw                                                                                 \
        gop_sdw_SUB_TREE_MAC(29, 0),                                                                 \
        gop_sdw_SUB_TREE_MAC(30, 1),                                                                 \
        gop_sdw_SUB_TREE_MAC(31, 2)

#define gop_mif_usx_SUB_TREE_MAC(bit, _index)                                                        \
    /* MIF */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MIF_USX_##_index##_E, NULL,                                   \
        0x00000120, 0x00000124,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5X_USX_MIF_0_1_##_index##_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_TYPE_8_TX_PROTOCOL_VIOLATION_INTERRUPT_E, \
        PRV_CPSS_AC5X_USX_MIF_0_1_##_index##_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_BAD_ACCESS_SUM_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_mif_usx_cpu_SUB_TREE_MAC(bit)                                                        \
    /* MIF */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MIF_CPU_0_E, NULL,                                   \
        0x00000120, 0x00000124,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5X_CPU_MIF_3_3_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_TYPE_8_TX_PROTOCOL_VIOLATION_INTERRUPT_E, \
        PRV_CPSS_AC5X_CPU_MIF_3_3_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_BAD_ACCESS_SUM_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_mif_usx_100G_SUB_TREE_MAC(bit)                                                        \
    /* MIF */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MIF_400G_0_E, NULL,                                   \
        0x00000120, 0x00000124,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5X_100G_MIF_4_4_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_TYPE_8_TX_PROTOCOL_VIOLATION_INTERRUPT_E, \
        PRV_CPSS_AC5X_100G_MIF_4_4_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_BAD_ACCESS_SUM_E,           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_mif_usx2_SUB_TREE_MAC(bit, _index)                                                        \
    /* MIF */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MIF_USX_##_index##_E, NULL,                                   \
        0x00000120, 0x00000124,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5X_USX_MIF_2_2_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_TYPE_8_TX_PROTOCOL_VIOLATION_INTERRUPT_E, \
        PRV_CPSS_AC5X_USX_MIF_2_2_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_BAD_ACCESS_SUM_E,           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define LMU_LATENCY_OVER_THRESHOLD_INT_MAC(bit, _threshold)                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_LMU_0_E, NULL,                                           \
        LMU_LATENCY_INTERRUPT_CAUSE_MAC(_threshold),                                                \
        LMU_LATENCY_INTERRUPT_MASK_MAC(_threshold),                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5X_LMU_N_##_threshold##_I_0_LMU_LATENCY_OVER_THRESHOLD_E,                        \
        PRV_CPSS_AC5X_LMU_N_##_threshold##_I_30_LMU_LATENCY_OVER_THRESHOLD_E,                       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_lmu_SUB_TREE_MAC(bit)                                                                   \
    /* LMU */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_LMU_0_E, NULL,                                           \
        0x00008230, 0x00008234,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5X_LMU_LMU_CAUSE_SUMMARY_N_0_LMU_LATENCY_OVER_THRESHOLD_SUM_E,                   \
        PRV_CPSS_AC5X_LMU_LMU_CAUSE_SUMMARY_N_0_LMU_LATENCY_FIFO_FULL_SUM_E,                        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 17, NULL, NULL},                                    \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC( 1,  0),                                             \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC( 2,  1),                                             \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC( 3,  2),                                             \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC( 4,  3),                                             \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC( 5,  4),                                             \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC( 6,  5),                                             \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC( 7,  6),                                             \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC( 8,  7),                                             \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC( 9,  8),                                             \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC(10,  9),                                             \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC(11, 10),                                             \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC(12, 11),                                             \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC(13, 12),                                             \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC(14, 13),                                             \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC(15, 14),                                             \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC(16, 15),                                             \
            LMU_LATENCY_OVER_THRESHOLD_INT_MAC(17, 16)

#define gop_led_usx_SUB_TREE_MAC(bit, _index)                                                        \
    /* LED */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_LED_##_index##_E, NULL,                                       \
        0x00000200, 0x00000204,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5X_LED_0_1_##_index##_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,               \
        PRV_CPSS_AC5X_LED_0_1_##_index##_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_led_cpu_SUB_TREE_MAC(bit, _index)                                                        \
    /* LED */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_LED_##_index##_E, NULL,                                       \
        0x00000200, 0x00000204,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5X_LED_2_3_##_index##_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,              \
        PRV_CPSS_AC5X_LED_2_3_##_index##_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_led_100G_SUB_TREE_MAC(bit, _index)                                                       \
    /* LED */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_LED_##_index##_E, NULL,                                       \
        0x00000200, 0x00000204,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5X_LED_4_5_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                         \
        PRV_CPSS_AC5X_LED_4_5_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_100G_macro_tai_SUB_TREE_MAC(bit, _index)                                                 \
    /* TAI */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_100G_TAI##_index##_E, NULL,                                   \
        0x00000000, 0x00000004,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5X_100G_TAI_10_TAI_INST_##_index##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          \
        PRV_CPSS_AC5X_100G_TAI_10_TAI_INST_##_index##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_PCA_mac_macro_tai_SUB_TREE_MAC(bit, _index)                                              \
    /* TAI */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA0_TAI##_index##_E, NULL,                                   \
        0x00000000, 0x00000004,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5X_PCA_USX2_0_TAI_5_TAI_INST_##_index##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,     \
        PRV_CPSS_AC5X_PCA_USX2_0_TAI_5_TAI_INST_##_index##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_usx2_0_mac_macro_tai_SUB_TREE_MAC(bit, _index)                                           \
    /* TAI */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_USX2_0_TAI##_index##_E, NULL,                                 \
        0x00000000, 0x00000004,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5X_PCA_USX2_0_TAI_6_TAI_INST_##_index##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,     \
        PRV_CPSS_AC5X_PCA_USX2_0_TAI_6_TAI_INST_##_index##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_usx2_1_mac_macro_tai_SUB_TREE_MAC(bit, _index)                                           \
    /* TAI */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_USX2_1_TAI##_index##_E, NULL,                                 \
        0x00000000, 0x00000004,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5X_USX2_1_TAI_7_TAI_INST_##_index##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,         \
        PRV_CPSS_AC5X_USX2_1_TAI_7_TAI_INST_##_index##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_usx2_mac_cpu_macro_cpu_tai_SUB_TREE_MAC(bit, _tai)                                      \
    /* TAI */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MAC_CPUC_TAI##_tai##_E, NULL,                                \
        0x00000000, 0x00000004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5X_CPU_TAI_8_TAI_INST_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,             \
        PRV_CPSS_AC5X_CPU_TAI_8_TAI_INST_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_usx2_mac_cpu_macro_mac_tai_SUB_TREE_MAC(bit, _tai)                                      \
    /* USX2 TAI */                                                                                  \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MAC_CPUM_TAI##_tai##_E, NULL,                                \
        0x00000000, 0x00000004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5X_CPU_TAI_9_TAI_INST_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,             \
        PRV_CPSS_AC5X_CPU_TAI_9_TAI_INST_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_mac_pcs_100G_SUB_TREE_MAC(bit)                                                           \
    /* 100G MAC */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MAC_400G_0_E, NULL,                                           \
        0x0000004C, 0x00000050,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5X_100_MAC_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                       \
        PRV_CPSS_AC5X_100_MAC_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 4, NULL, NULL},                                      \
            PORT_100G_MAC_MAC(1,   0),                                                               \
            PORT_100G_MAC_MAC(2,   1),                                                               \
            PORT_100G_MAC_MAC(3,   2),                                                               \
            PORT_100G_MAC_MAC(4,   3)


#define gop_mac_pcs_cpu_SUB_TREE_MAC(bit, _index)                                                    \
    /* CPU MAC */                                                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MAC_CPU_##_index##_E, NULL,                                   \
        0x0000000c, 0x00000010,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5X_MAC_6_7_##_index##_PORT_INTERRUPT_CAUSE_LINK_OK_CHANGED_E,                \
        PRV_CPSS_AC5X_MAC_6_7_##_index##_PORT_INTERRUPT_CAUSE_TSD_NON_ACCURATE_PTP_E,           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define port_usx_mac_pcs_0_SUB_TREE_MAC(bit, port)                                                \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_0_E, NULL,                                                                     \
            PORT_USX_MAC_INTERRUPT_CAUSE_MAC(port),                                               \
            PORT_USX_MAC_INTERRUPT_MASK_MAC(port),                                                \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5X_MAC_0_1_0_PORT_NUM_##port##_LINK_OK_CHANGE_E,                                 \
            PRV_CPSS_AC5X_MAC_0_1_0_PORT_NUM_##port##_LPCS_LINK_STATUS_CLEAN_CHANGE_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_usx_mac_pcs_1_SUB_TREE_MAC(bit, port)                                                \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_USX_1_MAC_0_E, NULL,                                                                     \
            PORT_USX_MAC_INTERRUPT_CAUSE_MAC(port),                                               \
            PORT_USX_MAC_INTERRUPT_MASK_MAC(port),                                                \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5X_MAC_0_1_1_PORT_NUM_##port##_LINK_OK_CHANGE_E,                                 \
            PRV_CPSS_AC5X_MAC_0_1_1_PORT_NUM_##port##_LPCS_LINK_STATUS_CLEAN_CHANGE_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define PORT_100G_MAC_MAC(bit, port)                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MAC_400G_0_E, NULL,                                                                     \
            PORT_100G_MAC_INTERRUPT_CAUSE_MAC(port),                                               \
            PORT_100G_MAC_INTERRUPT_MASK_MAC(port),                                                \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5X_100_MAC_PORT_NUM_##port##_LINK_OK_CHANGE_E,                                 \
            PRV_CPSS_AC5X_100_MAC_PORT_NUM_##port##_TSD_NON_ACCURATE_PTP_E,                         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_usx_mac_pcs_2_SUB_TREE_MAC(bit, port)                                                \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_USX_2_MAC_0_E, NULL,                                                                     \
            PORT_USX_MAC_INTERRUPT_CAUSE_MAC(port),                                               \
            PORT_USX_MAC_INTERRUPT_MASK_MAC(port),                                                \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5X_MAC_2_3_0_PORT_NUM_##port##_LINK_OK_CHANGE_E,                                 \
            PRV_CPSS_AC5X_MAC_2_3_0_PORT_NUM_##port##_LPCS_LINK_STATUS_CLEAN_CHANGE_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_usx_mac_pcs_3_SUB_TREE_MAC(bit, port)                                                \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_1_E, NULL,                                                                     \
            PORT_USX_MAC_INTERRUPT_CAUSE_MAC(port),                                               \
            PORT_USX_MAC_INTERRUPT_MASK_MAC(port),                                                \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5X_MAC_2_3_1_PORT_NUM_##port##_LINK_OK_CHANGE_E,                                 \
            PRV_CPSS_AC5X_MAC_2_3_1_PORT_NUM_##port##_LPCS_LINK_STATUS_CLEAN_CHANGE_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_usx_mac_pcs_4_SUB_TREE_MAC(bit, port)                                                \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_USX_1_MAC_1_E, NULL,                                                                     \
            PORT_USX_MAC_INTERRUPT_CAUSE_MAC(port),                                               \
            PORT_USX_MAC_INTERRUPT_MASK_MAC(port),                                                \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5X_MAC_4_5_0_PORT_NUM_##port##_LINK_OK_CHANGE_E,                                 \
            PRV_CPSS_AC5X_MAC_4_5_0_PORT_NUM_##port##_LPCS_LINK_STATUS_CLEAN_CHANGE_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_usx_mac_pcs_5_SUB_TREE_MAC(bit, port)                                                \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_USX_2_MAC_1_E, NULL,                                                                     \
            PORT_USX_MAC_INTERRUPT_CAUSE_MAC(port),                                               \
            PORT_USX_MAC_INTERRUPT_MASK_MAC(port),                                                \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5X_MAC_4_5_1_PORT_NUM_##port##_LINK_OK_CHANGE_E,                                 \
            PRV_CPSS_AC5X_MAC_4_5_1_PORT_NUM_##port##_LPCS_LINK_STATUS_CLEAN_CHANGE_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_usx_mac_pcs_0_SUB_TREE_MAC(bit)                                                             \
    /* USX MAC */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_0_E, NULL,                                         \
        0x00000018, 0x0000001c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_MAC_0_1_0_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,                  \
        PRV_CPSS_AC5X_MAC_0_1_0_GLOBAL_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_SUM_E,                        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8, NULL, NULL},                                         \
            port_usx_mac_pcs_0_SUB_TREE_MAC(1,   0),                                                    \
            port_usx_mac_pcs_0_SUB_TREE_MAC(2,   1),                                                    \
            port_usx_mac_pcs_0_SUB_TREE_MAC(3,   2),                                                    \
            port_usx_mac_pcs_0_SUB_TREE_MAC(4,   3),                                                    \
            port_usx_mac_pcs_0_SUB_TREE_MAC(5,   4),                                                    \
            port_usx_mac_pcs_0_SUB_TREE_MAC(6,   5),                                                    \
            port_usx_mac_pcs_0_SUB_TREE_MAC(7,   6),                                                    \
            port_usx_mac_pcs_0_SUB_TREE_MAC(8,   7)

#define gop_usx_mac_pcs_1_SUB_TREE_MAC(bit)                                                    \
    /* USX MAC */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_USX_1_MAC_0_E, NULL,                                 \
        0x00000018, 0x0000001c,                                                                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_AC5X_MAC_0_1_1_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,          \
        PRV_CPSS_AC5X_MAC_0_1_1_GLOBAL_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_SUM_E,                \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8, NULL, NULL},                                         \
            port_usx_mac_pcs_1_SUB_TREE_MAC(1,   0),                                                    \
            port_usx_mac_pcs_1_SUB_TREE_MAC(2,   1),                                                    \
            port_usx_mac_pcs_1_SUB_TREE_MAC(3,   2),                                                    \
            port_usx_mac_pcs_1_SUB_TREE_MAC(4,   3),                                                    \
            port_usx_mac_pcs_1_SUB_TREE_MAC(5,   4),                                                    \
            port_usx_mac_pcs_1_SUB_TREE_MAC(6,   5),                                                    \
            port_usx_mac_pcs_1_SUB_TREE_MAC(7,   6),                                                    \
            port_usx_mac_pcs_1_SUB_TREE_MAC(8,   7)

#define gop_usx_mac_pcs_2_SUB_TREE_MAC(bit)                                                    \
    /* USX MAC */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_USX_2_MAC_0_E, NULL,                                 \
        0x00000018, 0x0000001c,                                                                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_AC5X_MAC_2_3_0_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,          \
        PRV_CPSS_AC5X_MAC_2_3_0_GLOBAL_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_SUM_E,                \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8, NULL, NULL},                                     \
            port_usx_mac_pcs_2_SUB_TREE_MAC(1,   0),                                                    \
            port_usx_mac_pcs_2_SUB_TREE_MAC(2,   1),                                                    \
            port_usx_mac_pcs_2_SUB_TREE_MAC(3,   2),                                                    \
            port_usx_mac_pcs_2_SUB_TREE_MAC(4,   3),                                                    \
            port_usx_mac_pcs_2_SUB_TREE_MAC(5,   4),                                                    \
            port_usx_mac_pcs_2_SUB_TREE_MAC(6,   5),                                                    \
            port_usx_mac_pcs_2_SUB_TREE_MAC(7,   6),                                                    \
            port_usx_mac_pcs_2_SUB_TREE_MAC(8,   7)

#define gop_usx_mac_pcs_3_SUB_TREE_MAC(bit)                                                    \
    /* USX MAC */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_1_E, NULL,                                 \
        0x00000018, 0x0000001c,                                                                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_AC5X_MAC_2_3_1_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,          \
        PRV_CPSS_AC5X_MAC_2_3_1_GLOBAL_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_SUM_E,                \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8, NULL, NULL},                                         \
            port_usx_mac_pcs_3_SUB_TREE_MAC(1,   0),                                                    \
            port_usx_mac_pcs_3_SUB_TREE_MAC(2,   1),                                                    \
            port_usx_mac_pcs_3_SUB_TREE_MAC(3,   2),                                                    \
            port_usx_mac_pcs_3_SUB_TREE_MAC(4,   3),                                                    \
            port_usx_mac_pcs_3_SUB_TREE_MAC(5,   4),                                                    \
            port_usx_mac_pcs_3_SUB_TREE_MAC(6,   5),                                                    \
            port_usx_mac_pcs_3_SUB_TREE_MAC(7,   6),                                                    \
            port_usx_mac_pcs_3_SUB_TREE_MAC(8,   7)


#define gop_usx_mac_pcs_4_SUB_TREE_MAC(bit)                                                    \
    /* USX MAC */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_USX_1_MAC_1_E, NULL,                                 \
        0x00000018, 0x0000001c,                                                                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_AC5X_MAC_4_5_0_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,          \
        PRV_CPSS_AC5X_MAC_4_5_0_GLOBAL_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_SUM_E,                \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8, NULL, NULL},                                 \
            port_usx_mac_pcs_4_SUB_TREE_MAC(1,   0),                                                    \
            port_usx_mac_pcs_4_SUB_TREE_MAC(2,   1),                                                    \
            port_usx_mac_pcs_4_SUB_TREE_MAC(3,   2),                                                    \
            port_usx_mac_pcs_4_SUB_TREE_MAC(4,   3),                                                    \
            port_usx_mac_pcs_4_SUB_TREE_MAC(5,   4),                                                    \
            port_usx_mac_pcs_4_SUB_TREE_MAC(6,   5),                                                    \
            port_usx_mac_pcs_4_SUB_TREE_MAC(7,   6),                                                    \
            port_usx_mac_pcs_4_SUB_TREE_MAC(8,   7)


#define gop_usx_mac_pcs_5_SUB_TREE_MAC(bit)                                                    \
    /* USX MAC */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_USX_2_MAC_1_E, NULL,                                 \
        0x00000018, 0x0000001c,                                                                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_AC5X_MAC_4_5_1_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,          \
        PRV_CPSS_AC5X_MAC_4_5_1_GLOBAL_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_SUM_E,                \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8, NULL, NULL},                                 \
            port_usx_mac_pcs_5_SUB_TREE_MAC(1,   0),                                                    \
            port_usx_mac_pcs_5_SUB_TREE_MAC(2,   1),                                                    \
            port_usx_mac_pcs_5_SUB_TREE_MAC(3,   2),                                                    \
            port_usx_mac_pcs_5_SUB_TREE_MAC(4,   3),                                                    \
            port_usx_mac_pcs_5_SUB_TREE_MAC(5,   4),                                                    \
            port_usx_mac_pcs_5_SUB_TREE_MAC(6,   5),                                                    \
            port_usx_mac_pcs_5_SUB_TREE_MAC(7,   6),                                                    \
            port_usx_mac_pcs_5_SUB_TREE_MAC(8,   7)

#define global_port_usx_anp_0_3_SUB_TREE_MAC(bit, unit)                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_##unit##_E, NULL,                                        \
        0x00000088, 0x0000008C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_PCS_ANP0_3_##unit##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,              \
        PRV_CPSS_AC5X_PCS_ANP0_3_##unit##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_usx_anp_0_3_SUB_TREE_MAC(bit, unit)                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_##unit##_E, NULL,                                        \
        0x00000100, 0x00000104,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_PCS_ANP0_3_##unit##_PORT_NUM_0_AN_RESTART_E,                                      \
        PRV_CPSS_AC5X_PCS_ANP0_3_##unit##_PORT_NUM_0_INT_TX_TRAIN_TIME_OUT_E,                           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_usx_anp_0_3_SUB_TREE_MAC(bit, unit)                                                         \
    /* USX ANP */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_##unit##_E, NULL,                                        \
        0x00000094, 0x00000098,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_PCS_ANP0_3_##unit##_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_E,                         \
        PRV_CPSS_AC5X_PCS_ANP0_3_##unit##_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT2_SUM_E,              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
            global_port_usx_anp_0_3_SUB_TREE_MAC(1, unit),                                              \
            port_usx_anp_0_3_SUB_TREE_MAC(2, unit)

#define global_port_usx_anp_4_5_SUB_TREE_MAC(bit, unit)                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_##unit##_E, NULL,                                        \
        0x00000088, 0x0000008C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_PCS_ANP4_5_##unit##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,              \
        PRV_CPSS_AC5X_PCS_ANP4_5_##unit##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_usx_anp_4_5_SUB_TREE_MAC(bit, unit)                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_##unit##_E, NULL,                                        \
        0x00000100, 0x00000104,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_PCS_ANP4_5_##unit##_PORT_NUM_0_AN_RESTART_E,                                      \
        PRV_CPSS_AC5X_PCS_ANP4_5_##unit##_PORT_NUM_0_INT_TX_TRAIN_TIME_OUT_E,                           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_usx_anp_4_5_SUB_TREE_MAC(bit, unit)                                                         \
    /* USX ANP */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_##unit##_E, NULL,                                        \
        0x00000094, 0x00000098,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_PCS_ANP4_5_##unit##_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_E,                         \
        PRV_CPSS_AC5X_PCS_ANP4_5_##unit##_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT2_SUM_E,              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
            global_port_usx_anp_4_5_SUB_TREE_MAC(1, unit),                                              \
            port_usx_anp_4_5_SUB_TREE_MAC(2,unit)

#define global_port_cpu_anp_SUB_TREE_MAC(bit, cpu, unit)                                                \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_CPU_##cpu##_E, NULL,                                         \
        0x00000088, 0x0000008C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_PCS_ANP6_7_##unit##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,              \
        PRV_CPSS_AC5X_PCS_ANP6_7_##unit##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_cpu_anp_SUB_TREE_MAC(bit, cpu, unit)                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_CPU_##cpu##_E, NULL,                                         \
        0x00000100, 0x00000104,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_PCS_ANP6_7_##unit##_PORT_NUM_0_AN_RESTART_E,                                      \
        PRV_CPSS_AC5X_PCS_ANP6_7_##unit##_PORT_NUM_0_INT_TX_TRAIN_TIME_OUT_E,                           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_cpu_anp_SUB_TREE_MAC(bit, cpu, unit)                                                        \
    /* CPU ANP */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_CPU_##cpu##_E, NULL,                                         \
        0x00000094, 0x00000098,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_PCS_ANP6_7_##unit##_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_E,                         \
        PRV_CPSS_AC5X_PCS_ANP6_7_##unit##_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT2_SUM_E,              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
            global_port_cpu_anp_SUB_TREE_MAC(1, cpu, unit),                                             \
            port_cpu_anp_SUB_TREE_MAC(2, cpu, unit)

#define gop_100G_anp_SUB_TREE_MAC(bit)                                                                  \
    /* 100G ANP */                                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_400G_0_E, NULL,                                              \
        0x00000094, 0x00000098,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_ANP4_IP_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_E,                                     \
        PRV_CPSS_AC5X_ANP4_IP_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_3_P_INT2_SUM_E,                          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                         \
            {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_400G_0_E, NULL,                                        \
                0x00000088, 0x0000008C,                                                                 \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_AC5X_ANP4_IP_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                  \
                PRV_CPSS_AC5X_ANP4_IP_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                  \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
            {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_400G_0_E, NULL,                                        \
                0x00000100, 0x00000104,                                                                 \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_0_AN_RESTART_E,                                          \
                PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_0_INT_TX_TRAIN_TIME_OUT_E,                               \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
            {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_400G_0_E, NULL,                                        \
                0x000003A4, 0x000003A8,                                                                 \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_1_AN_RESTART_E,                                          \
                PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_1_INT_TX_TRAIN_TIME_OUT_E,                               \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
            {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_400G_0_E, NULL,                                        \
                0x00000648, 0x0000064C,                                                                 \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_2_AN_RESTART_E,                                          \
                PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_2_INT_TX_TRAIN_TIME_OUT_E,                               \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
            {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_400G_0_E, NULL,                                        \
                0x000008EC, 0x000008F0,                                                                 \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_3_AN_RESTART_E,                                          \
                PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_3_INT_TX_TRAIN_TIME_OUT_E,                               \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define gop_pzarb_SUB_TREE_MAC(bit, _index)
        /* Not implemented */
#define gop_sdw_SUB_TREE_MAC(bit, _index)
        /* Not implemented */

#define cnm_grp_0_0_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                 \
    /* FuncUnitsIntsSum Interrupt Cause */                                                              \
    {bit, GT_FALSE,MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL,                                     \
        0x000003F8, 0x000003FC,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_MG_##_mgIndex##_FUNCTIONAL_INTERRUPTS_SUMMARY_CAUSE_FUNC_UNITS_INT_SUM_1_E,       \
        PRV_CPSS_AC5X_MG_##_mgIndex##_FUNCTIONAL_INTERRUPTS_SUMMARY_CAUSE_FUNC_UNITS_INT_SUM_31_E,      \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 31, NULL, NULL},                                                 \
        em_SUB_TREE_MAC(1),                                                                             \
        epcl_tai_SUB_TREE_MAC(2, 0),                                                                    \
        epcl_tai_SUB_TREE_MAC(3, 1),                                                                    \
        mt_SUB_TREE_MAC(4),                                                                             \
        pha_tai_SUB_TREE_MAC(5, 0),                                                                     \
        pha_tai_SUB_TREE_MAC(6, 1),                                                                     \
        bma_SUB_TREE_MAC(7),                                                                            \
        cnc_SUB_TREE_MAC(8),                                                                            \
        eft_SUB_TREE_MAC(9),                                                                            \
        eoam_SUB_TREE_MAC(10),                                                                          \
        epcl_SUB_TREE_MAC(11),                                                                          \
        eplr_SUB_TREE_MAC(12),                                                                          \
        eq_SUB_TREE_MAC(13),                                                                            \
        erep_SUB_TREE_MAC(14),                                                                          \
        ermrk_SUB_TREE_MAC(15),                                                                         \
        ha_SUB_TREE_MAC(16),                                                                            \
        hbu_SUB_TREE_MAC(17),                                                                           \
        ioam_SUB_TREE_MAC(18),                                                                          \
        iplr0_SUB_TREE_MAC(19),                                                                         \
        iplr1_SUB_TREE_MAC(20),                                                                         \
        router_SUB_TREE_MAC(21),                                                                        \
        l2i_SUB_TREE_MAC(22),                                                                           \
        lpm_SUB_TREE_MAC(23),                                                                           \
        mll_SUB_TREE_MAC(24),                                                                           \
        pcl_SUB_TREE_MAC(25),                                                                           \
        ppu_SUB_TREE_MAC(26),                                                                           \
        preq_SUB_TREE_MAC(27),                                                                          \
        qag_SUB_TREE_MAC(28),                                                                           \
        sht_SUB_TREE_MAC(29),                                                                           \
        pha_SUB_TREE_MAC(30),                                                                           \
        tti_SUB_TREE_MAC(31)

#define  tcam_SUB_TREE_MAC(bit)                                                                         \
    /* TCAM Interrupt Summary */                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TCAM_E, NULL,                                                    \
        0x00501004, 0x00501000,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_TCAM_TCAM_INTERRUPTS_SUMMARY_CAUSE_TCAM_LOGIC_INTERRUPT_SUMMARY_E,                \
        PRV_CPSS_AC5X_TCAM_TCAM_INTERRUPTS_SUMMARY_CAUSE_TCAM_LOGIC_INTERRUPT_SUMMARY_E,                \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        /* TCAM Interrupt */                                                                            \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TCAM_E, NULL,                                                  \
            0x0050100C, 0x00501008,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5X_TCAM_TCAM_INTERRUPT_CAUSE_CPU_ADDRESS_OUT_OF_RANGE_E,                         \
            PRV_CPSS_AC5X_TCAM_TCAM_INTERRUPT_CAUSE_TABLE_OVERLAPPING_E,                                \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  tti_tai_SUB_TREE_MAC(bit, _tai)                                                                \
    /* TTI TAI */                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TTI_TAI##_tai##_E, NULL,                                         \
        0x00000000, 0x00000004,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_TTI_TAI_0_TAI_INST_##_tai##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,                 \
        PRV_CPSS_AC5X_TTI_TAI_0_TAI_INST_##_tai##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   ia_SUB_TREE_MAC(bit)                                                                          \
    /* IA */                                                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IA_E, NULL,                                                      \
        0x00000500, 0x00000504,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_IA_IA_INTERRUPT_0_CAUSE_IA_RF_ERR_E,                                              \
        PRV_CPSS_AC5X_IA_IA_INTERRUPT_0_CAUSE_IA_LS_FIFO_READ_ERROR_E,                                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define npm_SUB_TREE_MAC(bit)                                                            \
    /* NPM-0  */                                                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E, NULL,                                      \
        0x00000100, 0x00000104,                                                                             \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                     \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                    \
        PRV_CPSS_AC5X_NPM_MC_INTERRUPT_SUMMARY_CAUSE_INT_SUMMARY_CAUSE_GENERIC_E,                           \
        PRV_CPSS_AC5X_NPM_MC_INTERRUPT_SUMMARY_CAUSE_INT_SUMMARY_CAUSE_LIST_ERR_E,                          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 10, NULL, NULL},                                            \
                                                                                                            \
        /* GenericInterruptSum  */                                                                          \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E, NULL, \
            0x00000108, 0x0000010C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_GENERIC_CAUSE_INT_GENERIC_CAUSE_ADDR_ERR_E,                      \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_GENERIC_CAUSE_INT_GENERIC_CAUSE_ADDR_ERR_E,                      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* UintErrorInterruptSum  */                                                                        \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E, NULL, \
            0x00000110, 0x00000114,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_0_E,                       \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_4_E,                       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* ReinsertInterruptSum  */                                                                         \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E, NULL, \
            0x00000118, 0x0000011C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_REINSERT_CAUSE_INT_REINSERT_CAUSE_0_0_E,                         \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_REINSERT_CAUSE_INT_REINSERT_CAUSE_0_4_E,                         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* ReinsertfailInterruptSum  */                                                                     \
        {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E, NULL, \
            0x00000120, 0x00000124,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_0_E,                    \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_4_E,                    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* UnitRefsInterruptSum  */                                                                         \
        {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E, NULL, \
            0x00000128, 0x0000012C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_0_E,                       \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_4_E,                       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* UnitDataInterruptSum  */                                                                         \
        {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E, NULL, \
            0x00000130, 0x00000134,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_0_E,                       \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_4_E,                       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* RefsErrorInterruptSum  */                                                                        \
        {7, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E, NULL, \
            0x00000138, 0x0000013C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_0_E,                       \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_3_E,                       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* AgingErrorInterruptSum  */                                                                       \
        {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E, NULL, \
            0x00000140, 0x00000144,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_0_E,                     \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_3_E,                     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* RefsFifoInterruptSum  */                                                                         \
        {9, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E, NULL, \
            0x00000148, 0x0000014C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_0_E,                       \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_3_E,                       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* ListErrorInterruptSum  */                                                                        \
        {10, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E, NULL,\
            0x00000150, 0x00000154,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_0_E,                         \
            PRV_CPSS_AC5X_NPM_MC_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_0_E,                         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define smb_SUB_TREE_MAC(bit)                                                               \
    /* SMB-MC */                                                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E, NULL,   \
        0x00000150, 0x00000154,                                                                                        \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                               \
        PRV_CPSS_AC5X_SMB_MC_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                                                 \
        PRV_CPSS_AC5X_SMB_MC_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_FIFO_E,                                                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6,NULL, NULL},                                                         \
                                                                                                                       \
        /* MiscInterruptSum  */                                                                                        \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E, NULL, \
            0x00000100, 0x00000104,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_AC5X_SMB_MC_INTERRUPT_MISC_CAUSE_INTERRUPT_RPW_NO_TRUST_E,                                        \
            PRV_CPSS_AC5X_SMB_MC_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_CONGESTION_E,                      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                     \
        /* AgeInterruptSum  */                                                                                         \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E, NULL, \
            0x00000110, 0x00000114,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_AC5X_SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_AGE_ERR_E,                             \
            PRV_CPSS_AC5X_SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_AGE_ERR_E,                             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                     \
        /* RbwInterruptSum  */                                                                                         \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E, NULL, \
            0x00000120, 0x00000124,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_AC5X_SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_RBW_ERR_E,                             \
            PRV_CPSS_AC5X_SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_RBW_ERR_E,                             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                     \
        /* MemoriesInterruptSum  */                                                                                    \
        {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E, NULL, \
            0x00000130, 0x00000134,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_AC5X_SMB_MC_INTERRUPT_MEMORIES_CAUSE_LOGICAL_MEMORY_0_PAYLOAD_SER_ERROR_E,                        \
            PRV_CPSS_AC5X_SMB_MC_INTERRUPT_MEMORIES_CAUSE_LOGICAL_MEMORY_7_PAYLOAD_SER_ERROR_E,                        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                     \
        /* Memories2InterruptSum  */                                                                                   \
        {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E, NULL, \
            0x00000140, 0x00000144,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_AC5X_SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_0_REFCNT_SER_ERROR_E,                        \
            PRV_CPSS_AC5X_SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_1_LIST_SER_ERROR_E,                          \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                     \
        /* FifoInterruptSum  */                                                                                        \
        {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E, NULL, \
            0x00000118, 0x0000011C,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_AC5X_SMB_MC_INTERRUPT_FIFO_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_FIFO_ERR_E,                           \
            PRV_CPSS_AC5X_SMB_MC_INTERRUPT_FIFO_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_FIFO_ERR_E,                           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define gpc_pr_chan_sum0_SUB_TREE_MAC(bit, _chan)                                                                            \
{bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E, NULL,                                                         \
    0x00001300 + (_chan)*4, 0x00001400 + (_chan)*4,                                                                          \
    prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
    prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
    PRV_CPSS_AC5X_GPC_PACKET_READ_CHANNEL_INDEX_##_chan##_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E,                         \
    PRV_CPSS_AC5X_GPC_PACKET_READ_CHANNEL_INDEX_##_chan##_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E,                                  \
    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gpc_pr_chan_sum1_SUB_TREE_MAC(bit, _chan)                                                                            \
{bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E, NULL,                                                         \
    0x00001300 + (_chan)*4, 0x00001400 + (_chan)*4,                                                                          \
    prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
    prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
    PRV_CPSS_AC5X_GPC_PACKET_READ_CHANNEL_INDEX_##_chan##_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E,                         \
    PRV_CPSS_AC5X_GPC_PACKET_READ_CHANNEL_INDEX_##_chan##_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E,                                  \
    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   pb_SUB_TREE_MAC(bit)                                                                          \
    /* PB */                                                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, NULL,                                           \
        0x00001128, 0x0000112C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_PB_CENTER_INTERRUPT_SUMMARY_PACKET_BUFFER_CAUSE_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTERS_E,\
        PRV_CPSS_AC5X_PB_CENTER_INTERRUPT_SUMMARY_PACKET_BUFFER_CAUSE_INTERRUPT_SUMMARY_PB_CENTER_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6, NULL, NULL},                                          \
            {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, NULL,                                      \
                0x00001108, 0x0000110C,                                                                  \
                prvCpssDrvHwPpPortGroupIsrRead,                                                          \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
                PRV_CPSS_AC5X_PB_CENTER_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTER_E,\
                PRV_CPSS_AC5X_PB_CENTER_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTER_E,\
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                  \
            npm_SUB_TREE_MAC(1),                                                                         \
            {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, NULL,                                      \
                0x00001110, 0x00001114,                                                                  \
                prvCpssDrvHwPpPortGroupIsrRead,                                                          \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
                PRV_CPSS_AC5X_PB_CENTER_INTERRUPT_SUMMARY_GPC_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_PACKET_WRITE_GPC_E,                                      \
                PRV_CPSS_AC5X_PB_CENTER_INTERRUPT_SUMMARY_GPC_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_CELL_READ_GPC_E,                                         \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                                                                   \
                {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E, NULL,                                                                       \
                    0x00000200, 0x0000204,                                                                                                                \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                                                                       \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                                                                      \
                    PRV_CPSS_AC5X_GPC_PACKET_WRITE_INTERRUPT_CAUSE_INT_CAUSE_RDY_NFL_E,                                                                   \
                    PRV_CPSS_AC5X_GPC_PACKET_WRITE_INTERRUPT_CAUSE_INT_CAUSE_ADDR_ERR_E,                                                                  \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                                               \
                {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E, NULL,                                                                        \
                    0x00001530, 0x00001534,                                                                                                               \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                                                                       \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                                                                      \
                    PRV_CPSS_AC5X_GPC_PACKET_READ_INTERRUPT_SUMMARY_CAUSE_INTERRUPT_CHANNELS_SUMMARY_0_E,                                                 \
                    PRV_CPSS_AC5X_GPC_PACKET_READ_INTERRUPT_SUMMARY_CAUSE_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_SUMMARY_E,                                \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6, NULL, NULL},                                                                               \
                    {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E, NULL,                                                                    \
                        0x00001520, 0x0001528,                                                                                                            \
                        prvCpssDrvHwPpPortGroupIsrRead,                                                                                                   \
                        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                                  \
                        PRV_CPSS_AC5X_GPC_PACKET_READ_INTERRUPT_CHANNELS_SUMMARY_0_CAUSE_CHANNEL_INDEX_0_INTERRUPT_CHANNEL_SUMMARY_E,                     \
                        PRV_CPSS_AC5X_GPC_PACKET_READ_INTERRUPT_CHANNELS_SUMMARY_0_CAUSE_CHANNEL_INDEX_30_INTERRUPT_CHANNEL_SUMMARY_E,                    \
                        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 31, NULL, NULL},                                                                          \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(1, 0),                                                                                          \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(2, 1),                                                                                          \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(3, 2),                                                                                          \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(4, 3),                                                                                          \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(5, 4),                                                                                          \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(6, 5),                                                                                          \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(7, 6),                                                                                          \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(8, 7),                                                                                          \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(9, 8),                                                                                          \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(10, 9),                                                                                         \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(11, 10),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(12, 11),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(13, 12),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(14, 13),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(15, 14),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(16, 15),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(17, 16),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(18, 17),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(19, 18),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(20, 19),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(21, 20),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(22, 21),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(23, 22),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(24, 23),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(25, 24),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(26, 25),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(27, 26),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(28, 27),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(29, 28),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(30, 29),                                                                                        \
                        gpc_pr_chan_sum0_SUB_TREE_MAC(31, 30),                                                                                        \
                    {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E, NULL,                                                                \
                        0x00001524, 0x000152C,                                                                                                        \
                        prvCpssDrvHwPpPortGroupIsrRead,                                                                                               \
                        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                              \
                        PRV_CPSS_AC5X_GPC_PACKET_READ_INTERRUPT_CHANNELS_SUMMARY_1_CAUSE_CHANNEL_INDEX_31_INTERRUPT_CHANNEL_SUMMARY_E,                \
                        PRV_CPSS_AC5X_GPC_PACKET_READ_INTERRUPT_CHANNELS_SUMMARY_1_CAUSE_CHANNEL_INDEX_56_INTERRUPT_CHANNEL_SUMMARY_E,                \
                        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 26, NULL, NULL},                                                                       \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(1, 31),                                                                                          \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(2, 32),                                                                                          \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(3, 33),                                                                                          \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(4, 34),                                                                                          \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(5, 35),                                                                                          \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(6, 36),                                                                                          \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(7, 37),                                                                                          \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(8, 38),                                                                                          \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(9, 39),                                                                                          \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(10, 40),                                                                                         \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(11, 41),                                                                                        \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(12, 42),                                                                                        \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(13, 43),                                                                                        \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(14, 44),                                                                                        \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(15, 45),                                                                                        \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(16, 46),                                                                                        \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(17, 47),                                                                                        \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(18, 48),                                                                                        \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(19, 49),                                                                                        \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(20, 50),                                                                                        \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(21, 51),                                                                                        \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(22, 52),                                                                                        \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(23, 53),                                                                                        \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(24, 54),                                                                                        \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(25, 55),                                                                                        \
                        gpc_pr_chan_sum1_SUB_TREE_MAC(26, 56),                                                                                        \
                    {11, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E, NULL,                                                                   \
                        0x00001500, 0x0001504,                                                                                                            \
                        prvCpssDrvHwPpPortGroupIsrRead,                                                                                                   \
                        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                                  \
                        PRV_CPSS_AC5X_GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E,                        \
                        PRV_CPSS_AC5X_GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CAUSE_NULL_ADDRESS_RECEIVED_PACKET_COUNT_CELL_E,                             \
                        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                                           \
                    {12, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E, NULL,                                                                   \
                        0x00001508, 0x000150C,                                                                                                            \
                        prvCpssDrvHwPpPortGroupIsrRead,                                                                                                   \
                        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                                  \
                        PRV_CPSS_AC5X_GPC_PACKET_READ_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E,                                      \
                        PRV_CPSS_AC5X_GPC_PACKET_READ_INTERRUPT_MISC_CAUSE_NULL_ADDRESS_RECEIVED_PACKET_READ_CELL_E,                                      \
                        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                                           \
                    {13, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E, NULL,                                                                   \
                        0x00001510, 0x0001514,                                                                                                            \
                        prvCpssDrvHwPpPortGroupIsrRead,                                                                                                   \
                        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                                  \
                        PRV_CPSS_AC5X_GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E,      \
                        PRV_CPSS_AC5X_GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E,               \
                        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                                           \
                    {14, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E, NULL,                                                                   \
                        0x00001518, 0x000151C,                                                                                                            \
                        prvCpssDrvHwPpPortGroupIsrRead,                                                                                                   \
                        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                                  \
                        PRV_CPSS_AC5X_GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E,    \
                        PRV_CPSS_AC5X_GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E,     \
                        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                                           \
                {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E, NULL,                                                             \
                    0x000000A0, 0x00000A4,                                                                                                   \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
                    PRV_CPSS_AC5X_GPC_CELL_READ_INTERRUPT_CAUSE_INTERFACE_0_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_E,                    \
                    PRV_CPSS_AC5X_GPC_CELL_READ_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                                                    \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                              \
            {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, NULL,                                      \
                0x00001118, 0x0000111C,                                                                  \
                prvCpssDrvHwPpPortGroupIsrRead,                                                          \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
                PRV_CPSS_AC5X_PB_CENTER_INTERRUPT_SUMMARY_SMB_MEMORY_CLUSTER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_SMB_MEMORY_CLUSTER_E,\
                PRV_CPSS_AC5X_PB_CENTER_INTERRUPT_SUMMARY_SMB_MEMORY_CLUSTER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_SMB_MEMORY_CLUSTER_E,\
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                  \
            smb_SUB_TREE_MAC(1),                                                                         \
            {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, NULL,               \
                0x00001120, 0x00001124,                                                                  \
                prvCpssDrvHwPpPortGroupIsrRead,                                                          \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
                PRV_CPSS_AC5X_PB_CENTER_INTERRUPT_SUMMARY_SMB_WRITE_ARBITER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_SMB_WRITE_ARBITER_E,          \
                PRV_CPSS_AC5X_PB_CENTER_INTERRUPT_SUMMARY_SMB_WRITE_ARBITER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_SMB_WRITE_ARBITER_E,          \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                                                      \
                {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E, NULL,                                                               \
                    0x00000110, 0x0000114,                                                                                                   \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
                    PRV_CPSS_AC5X_SMB_WRITE_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,      \
                    PRV_CPSS_AC5X_SMB_WRITE_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,      \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                              \
                    {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E, NULL,                                                               \
                        0x00000100, 0x0000104,                                                                                                   \
                        prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
                        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
                        PRV_CPSS_AC5X_SMB_WRITE_INTERRUPT_MISC_CAUSE_INTERRUPT_ILLEGAL_ADDRESS_E,        \
                        PRV_CPSS_AC5X_SMB_WRITE_INTERRUPT_MISC_CAUSE_INTERRUPT_MC_CA_FIFO_OVERFLOW_E,    \
                        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                          \
            {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_COUNTER_E, NULL,                                         \
                0x00000190, 0x00000194,                                                                  \
                prvCpssDrvHwPpPortGroupIsrRead,                                                          \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
                PRV_CPSS_AC5X_PB_COUNTER_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                       \
                PRV_CPSS_AC5X_PB_COUNTER_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                       \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                  \
            {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, NULL,                                      \
                0x00001100, 0x00001104,                                                                  \
                prvCpssDrvHwPpPortGroupIsrRead,                                                          \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
                PRV_CPSS_AC5X_PB_CENTER_INTERRUPT_PB_CENTER_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,              \
                PRV_CPSS_AC5X_PB_CENTER_INTERRUPT_PB_CENTER_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,              \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  rxDma0_NODE_MAC(bit)                                                                           \
    /* RXD0 */                                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA_E, NULL,                                                   \
        0x00001C80, 0x00001C84,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_RXDMA_RX_DMA_INTERRUPT_0_CAUSE_RX_DMA_RF_ERR_E,                                   \
        PRV_CPSS_AC5X_RXDMA_RX_DMA_INTERRUPT_0_CAUSE_HIGH_PRIO_DESC_DROP_E,                             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  rxDma1_NODE_MAC(bit)                                                                           \
    /* RXD1 */                                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA_E, NULL,                                                   \
        0x00001C88, 0x00001C8C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_RXDMA_RX_DMA_INTERRUPT_1_CAUSE_DESC_ENQUEUE_FIFO_FULL_INT_E,                      \
        PRV_CPSS_AC5X_RXDMA_RX_DMA_INTERRUPT_1_CAUSE_DESC_ENQUEUE_FIFO_FULL_INT_E,                      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  rxDma2_NODE_MAC(bit)                                                                           \
    /* RXD2 */                                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA_E, NULL,                                                   \
        0x00001C90, 0x00001C94,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_RXDMA_RX_DMA_INTERRUPT_2_CAUSE_PB_TAIL_ID_MEM_SER_INT_E,                          \
        PRV_CPSS_AC5X_RXDMA_RX_DMA_INTERRUPT_2_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E,                \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  rxDma_SUB_TREE_MAC(bit)                                                                        \
    /* RXD */                                                                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA_E, NULL,                                                   \
        0x00001CA0, 0x00001CA4,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_RXDMA_RX_DMA_INTERRUPT_SUMMARY_CAUSE_RX_DMA_INTERRUPT_0_INT_E,                    \
        PRV_CPSS_AC5X_RXDMA_RX_DMA_INTERRUPT_SUMMARY_CAUSE_RX_DMA_INTERRUPT_2_INT_E,                    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                         \
            rxDma0_NODE_MAC(1),                                                                         \
            rxDma1_NODE_MAC(2),                                                                         \
            rxDma2_NODE_MAC(3)

#define   txDma_SUB_TREE_MAC(bit)                                                                       \
    /* TXD */                                                                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA_E, NULL,                                                   \
        0x00006000, 0x00006004,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_TXD_TXD_INTERRUPT_CAUSE_TXD_RF_ERR_E,                                             \
        PRV_CPSS_AC5X_TXD_TXD_INTERRUPT_CAUSE_PB2TX_FIF_OSTATE_SYNC_OVERFLOW_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txFifo_SUB_TREE_MAC(bit)                                                                      \
    /* TXF */                                                                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO_E, NULL,                                                 \
        0x00004000, 0x00004004,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_TXF_TXF_INTERRUPT_CAUSE_TX_FIFO_RF_ERR_E,                                  \
        PRV_CPSS_AC5X_TXF_TXF_INTERRUPT_CAUSE_DESCRIPTOR_RD_SER_ERROR_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define global_cstu_NODE_MAC(bit)                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E, NULL,                                              \
        0x00000080, 0x00000084,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_CTSU_CTSU_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                        \
        PRV_CPSS_AC5X_CTSU_CTSU_GLOBAL_INTERRUPT_CAUSE_TIME_STAMPING_COMMAND_PARSING_ERROR_E,           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define channel_cstu_NODE_MAC(bit, channel)                                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E, NULL,                                              \
        CSTU_CHANNEL_INTERRUPT_CAUSE_MAC(channel),                                                      \
        CSTU_CHANNEL_INTERRUPT_MASK_MAC(channel),                                                       \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_CTSU_CHANNEL_##channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_0_NEW_ENTRY_INTERRUPT_E,  \
        PRV_CPSS_AC5X_CTSU_CHANNEL_##channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_1_OVERFLOW_INTERRUPT_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define GROUP_0_CHANNEL_CSTU_16_NODES_MAC                                                               \
        channel_cstu_NODE_MAC( 1,  0),                                                                  \
        channel_cstu_NODE_MAC( 2,  1),                                                                  \
        channel_cstu_NODE_MAC( 3,  2),                                                                  \
        channel_cstu_NODE_MAC( 4,  3),                                                                  \
        channel_cstu_NODE_MAC( 5,  4),                                                                  \
        channel_cstu_NODE_MAC( 6,  5),                                                                  \
        channel_cstu_NODE_MAC( 7,  6),                                                                  \
        channel_cstu_NODE_MAC( 8,  7),                                                                  \
        channel_cstu_NODE_MAC( 9,  8),                                                                  \
        channel_cstu_NODE_MAC(10,  9),                                                                  \
        channel_cstu_NODE_MAC(11, 10),                                                                  \
        channel_cstu_NODE_MAC(12, 11),                                                                  \
        channel_cstu_NODE_MAC(13, 12),                                                                  \
        channel_cstu_NODE_MAC(14, 13),                                                                  \
        channel_cstu_NODE_MAC(15, 14),                                                                  \
        channel_cstu_NODE_MAC(16, 15)

#define GROUP_1_CHANNEL_CSTU_16_NODES_MAC                                                               \
        channel_cstu_NODE_MAC( 1, 16),                                                                  \
        channel_cstu_NODE_MAC( 2, 17),                                                                  \
        channel_cstu_NODE_MAC( 3, 18),                                                                  \
        channel_cstu_NODE_MAC( 4, 19),                                                                  \
        channel_cstu_NODE_MAC( 5, 20),                                                                  \
        channel_cstu_NODE_MAC( 6, 21),                                                                  \
        channel_cstu_NODE_MAC( 7, 22),                                                                  \
        channel_cstu_NODE_MAC( 8, 23),                                                                  \
        channel_cstu_NODE_MAC( 9, 24),                                                                  \
        channel_cstu_NODE_MAC(10, 25),                                                                  \
        channel_cstu_NODE_MAC(11, 26),                                                                  \
        channel_cstu_NODE_MAC(12, 27),                                                                  \
        channel_cstu_NODE_MAC(13, 28),                                                                  \
        channel_cstu_NODE_MAC(14, 29),                                                                  \
        channel_cstu_NODE_MAC(15, 30),                                                                  \
        channel_cstu_NODE_MAC(16, 31)

#define GROUP_2_CHANNEL_CSTU_16_NODES_MAC                                                               \
        channel_cstu_NODE_MAC( 1, 32),                                                                  \
        channel_cstu_NODE_MAC( 2, 33),                                                                  \
        channel_cstu_NODE_MAC( 3, 34),                                                                  \
        channel_cstu_NODE_MAC( 4, 35),                                                                  \
        channel_cstu_NODE_MAC( 5, 36),                                                                  \
        channel_cstu_NODE_MAC( 6, 37),                                                                  \
        channel_cstu_NODE_MAC( 7, 38),                                                                  \
        channel_cstu_NODE_MAC( 8, 39),                                                                  \
        channel_cstu_NODE_MAC( 9, 40),                                                                  \
        channel_cstu_NODE_MAC(10, 41),                                                                  \
        channel_cstu_NODE_MAC(11, 42),                                                                  \
        channel_cstu_NODE_MAC(12, 43),                                                                  \
        channel_cstu_NODE_MAC(13, 44),                                                                  \
        channel_cstu_NODE_MAC(14, 45),                                                                  \
        channel_cstu_NODE_MAC(15, 46),                                                                  \
        channel_cstu_NODE_MAC(16, 47)

#define GROUP_3_CHANNEL_CSTU_16_NODES_MAC                                                               \
        channel_cstu_NODE_MAC( 1, 48),                                                                  \
        channel_cstu_NODE_MAC( 2, 49),                                                                  \
        channel_cstu_NODE_MAC( 3, 50),                                                                  \
        channel_cstu_NODE_MAC( 4, 51),                                                                  \
        channel_cstu_NODE_MAC( 5, 52),                                                                  \
        channel_cstu_NODE_MAC( 6, 53),                                                                  \
        channel_cstu_NODE_MAC( 7, 54),                                                                  \
        channel_cstu_NODE_MAC( 8, 55),                                                                  \
        channel_cstu_NODE_MAC( 9, 56),                                                                  \
        channel_cstu_NODE_MAC(10, 57),                                                                  \
        channel_cstu_NODE_MAC(11, 58),                                                                  \
        channel_cstu_NODE_MAC(12, 59),                                                                  \
        channel_cstu_NODE_MAC(13, 60),                                                                  \
        channel_cstu_NODE_MAC(14, 61),                                                                  \
        channel_cstu_NODE_MAC(15, 62),                                                                  \
        channel_cstu_NODE_MAC(16, 63)

#define channel_group_cstu_SUB_TREE_MAC(bit, group)                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E, NULL,                                              \
        CSTU_CHANNEL_GROUP_INTERRUPT_CAUSE_MAC(group),                                                  \
        CSTU_CHANNEL_GROUP_INTERRUPT_MASK_MAC(group),                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_CTSU_CH_GROUP_##group##_INTERRUPT_CAUSE_SUMMARY_CH_GROUP_##group##_INTERRUPT_SUMMARY_E + 1,  \
        PRV_CPSS_AC5X_CTSU_CH_GROUP_##group##_INTERRUPT_CAUSE_SUMMARY_RESERVED_17_E - 1,                \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
            GROUP_##group##_CHANNEL_CSTU_16_NODES_MAC

#define msec_ING_CH0_NODE_MAC(bit, _unit, _index)                                                                \
    /* MacSec Channel 0 Interrupts Cause */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_##_unit##_0_E,  NULL,                                    \
        0x00000338, 0x0000033c,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_AC5X_MSEC_ING_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_0_CHANNEL_INTERRUPT_E, \
        PRV_CPSS_AC5X_MSEC_ING_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_15_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_CH1_NODE_MAC(bit, _unit, _index)                                                                \
    /* MacSec Channel 1 Interrupts Cause */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_##_unit##_0_E,  NULL,                                    \
        0x00000340, 0x00000344,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_AC5X_MSEC_ING_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_16_CHANNEL_INTERRUPT_E,\
        PRV_CPSS_AC5X_MSEC_ING_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_31_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_CH2_NODE_MAC(bit, _unit, _index)                                                                \
    /* MacSec Channel 2 Interrupts Cause */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_##_unit##_0_E,  NULL,                                    \
        0x00000348, 0x0000034c,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_AC5X_MSEC_ING_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_2_CAUSE_CHANNEL_32_CHANNEL_INTERRUPT_E,\
        PRV_CPSS_AC5X_MSEC_ING_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_2_CAUSE_CHANNEL_47_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_CH3_NODE_MAC(bit, _unit, _index)                                                                \
    /* MacSec Channel 3 Interrupts Cause */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_##_unit##_0_E,  NULL,                                    \
        0x00000350, 0x00000354,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_AC5X_MSEC_ING_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_3_CAUSE_CHANNEL_48_CHANNEL_INTERRUPT_E,\
        PRV_CPSS_AC5X_MSEC_ING_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_3_CAUSE_CHANNEL_63_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_CH0_NODE_MAC(bit, _unit, _index)                                                                \
    /* MacSec Channel 0 Interrupts Cause */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_##_unit##_0_E,  NULL,                                    \
        0x00000338, 0x0000033c,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_AC5X_MSEC_EGR_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_0_CHANNEL_INTERRUPT_E, \
        PRV_CPSS_AC5X_MSEC_EGR_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_15_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_CH1_NODE_MAC(bit, _unit, _index)                                                                \
    /* MacSec Channel 1 Interrupts Cause */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_##_unit##_0_E,  NULL,                                    \
        0x00000340, 0x00000344,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_AC5X_MSEC_EGR_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_16_CHANNEL_INTERRUPT_E,\
        PRV_CPSS_AC5X_MSEC_EGR_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_31_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_CH2_NODE_MAC(bit, _unit, _index)                                                                \
    /* MacSec Channel 2 Interrupts Cause */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_##_unit##_0_E,  NULL,                                    \
        0x00000348, 0x0000034c,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_AC5X_MSEC_EGR_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_2_CAUSE_CHANNEL_32_CHANNEL_INTERRUPT_E,\
        PRV_CPSS_AC5X_MSEC_EGR_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_2_CAUSE_CHANNEL_47_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_CH3_NODE_MAC(bit, _unit, _index)                                                                \
    /* MacSec Channel 3 Interrupts Cause */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_##_unit##_0_E,  NULL,                                    \
        0x00000350, 0x00000354,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_AC5X_MSEC_EGR_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_3_CAUSE_CHANNEL_48_CHANNEL_INTERRUPT_E,\
        PRV_CPSS_AC5X_MSEC_EGR_INST_##_index##_MAC_SEC_CHANNEL_INTERRUPTS_3_CAUSE_CHANNEL_63_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_EIP_163_GLOBAL_NODE_MAC(bit, _unit, _index)                                            \
    /* Classifier Engine(EIP-163) Egress Interrupts Cause */                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_##_unit##_E_0_E,  NULL,                           \
        0x0000FC2C, 0x0000FC08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_MSEC_EGR_163_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E,      \
        PRV_CPSS_AC5X_MSEC_EGR_163_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_CHANNEL_IRQ_15_31_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_EIP_163_GLOBAL_NODE_MAC(bit, _unit, _index)                                            \
    /* Classifier Engine(EIP-163) Ingress Interrupts Cause */                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_##_unit##_I_0_E,  NULL,                           \
        0x0000FC2C, 0x0000FC08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_MSEC_ING_163_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E, \
        PRV_CPSS_AC5X_MSEC_ING_163_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_CHANNEL_IRQ_15_31_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_EIP_66_NODE_MAC(bit, _unit, _index)                                                    \
    /* MACSec Transform Crypto Engine(EIP-66) Egress Interrupts Cause */                                \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_##_unit##_E_0_E,  NULL,                           \
        0x00005C2C, 0x00005C08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_MSEC_EGR_66_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_0_E, \
        PRV_CPSS_AC5X_MSEC_EGR_66_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SEQ_NUM_ROLLOVER_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_EIP_66_NODE_MAC(bit, _unit, _index)                                                    \
    /* MACSec Transform Crypto Engine(EIP-66) Ingress Interrupts Cause */                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_##_unit##_I_0_E,  NULL,                           \
        0x00005C2C, 0x00005C08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_MSEC_ING_66_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_0_E, \
        PRV_CPSS_AC5X_MSEC_ING_66_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_14_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_EIP_164_GLOBAL_NODE_MAC(bit, _unit, _index)                                            \
    /* Transformer Engine(EIP-164) Egress Interrupts Cause */                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_##_unit##_E_0_E,  NULL,                           \
        0x0000FC2C, 0x0000FC08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_MSEC_EGR_164_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_DROP_CLASS_E, \
        PRV_CPSS_AC5X_MSEC_EGR_164_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_CHANNEL_IRQ_15_31_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        msec_EGR_EIP_66_NODE_MAC(3, _unit, _index)

#define msec_ING_EIP_164_GLOBAL_NODE_MAC(bit, _unit, _index)                                            \
    /* Transformer Engine(EIP-164) Ingress Interrupts Cause */                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_##_unit##_I_0_E,  NULL,                           \
        0x0000FC2C, 0x0000FC08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_MSEC_ING_164_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_DROP_CLASS_E, \
        PRV_CPSS_AC5X_MSEC_ING_164_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_CHANNEL_IRQ_15_31_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        msec_ING_EIP_66_NODE_MAC(3, _unit, _index)

#define gop_eip_163i_SUB_TREE_MAC(bit)                                                                  \
    /* MSEC Ingress 163 */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_0_E,  NULL,                                 \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_MSEC_ING_INST_1_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E,                     \
        PRV_CPSS_AC5X_MSEC_ING_INST_1_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_3_INTERRUPT_E,           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                         \
        msec_ING_EIP_163_GLOBAL_NODE_MAC(1, 163, 1),                                                    \
        msec_ING_CH0_NODE_MAC(4, 163, 1),                                                               \
        msec_ING_CH1_NODE_MAC(5, 163, 1),                                                               \
        msec_ING_CH2_NODE_MAC(6, 163, 1),                                                               \
        msec_ING_CH3_NODE_MAC(7, 163, 1)

#define gop_eip_164i_SUB_TREE_MAC(bit)                                                                  \
    /* MSEC Ingress 164 */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_0_E,  NULL,                                 \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_MSEC_ING_INST_2_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E,                     \
        PRV_CPSS_AC5X_MSEC_ING_INST_2_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_3_INTERRUPT_E,           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                         \
        msec_ING_EIP_164_GLOBAL_NODE_MAC(1, 164, 2),                                                    \
        msec_ING_CH0_NODE_MAC(4, 164, 2),                                                               \
        msec_ING_CH1_NODE_MAC(5, 164, 2),                                                               \
        msec_ING_CH2_NODE_MAC(6, 164, 2),                                                               \
        msec_ING_CH3_NODE_MAC(7, 164, 2)

#define gop_eip_163e_SUB_TREE_MAC(bit)                                                                  \
    /* MSEC Egress 163 */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_0_E,  NULL,                                 \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_MSEC_EGR_INST_0_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E,                     \
        PRV_CPSS_AC5X_MSEC_EGR_INST_0_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_3_INTERRUPT_E,           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                         \
        msec_EGR_EIP_163_GLOBAL_NODE_MAC(1, 163, 0),                                                    \
        msec_EGR_CH0_NODE_MAC(4, 163, 0),                                                               \
        msec_EGR_CH1_NODE_MAC(5, 163, 0),                                                               \
        msec_EGR_CH2_NODE_MAC(6, 163, 0),                                                               \
        msec_EGR_CH3_NODE_MAC(7, 163, 0)

#define gop_eip_164e_SUB_TREE_MAC(bit)                                                                  \
    /* MSEC Egress 164 */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_0_E,  NULL,                                 \
        0x00000330, 0x00000334,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_MSEC_EGR_INST_1_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_GLOBAL_IRQ_E,                     \
        PRV_CPSS_AC5X_MSEC_EGR_INST_1_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_CHANNELS_3_INTERRUPT_E,           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                         \
        msec_EGR_EIP_164_GLOBAL_NODE_MAC(1, 164, 1),                                                    \
        msec_EGR_CH0_NODE_MAC(4, 164, 1),                                                               \
        msec_EGR_CH1_NODE_MAC(5, 164, 1),                                                               \
        msec_EGR_CH2_NODE_MAC(6, 164, 1),                                                               \
        msec_EGR_CH3_NODE_MAC(7, 164, 1)

#define gop_ctsu_SUB_TREE_MAC(bit)                                                                      \
    /* CTSU */                                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E, NULL,                                              \
        0x00000088, 0x0000008C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_CTSU_CTSU_INTERRUPT_CAUSE_SUMMARY_GLOBAL_INTERRUPT_CAUSE_SUMMARY_E,               \
        PRV_CPSS_AC5X_CTSU_CTSU_INTERRUPT_CAUSE_SUMMARY_CHANNEL_GROUP_3_INTERRUPT_CAUSE_SUMMARY_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                         \
            global_cstu_NODE_MAC(1),                                                                    \
            channel_group_cstu_SUB_TREE_MAC(2, 0),                                                      \
            channel_group_cstu_SUB_TREE_MAC(3, 1),                                                      \
            channel_group_cstu_SUB_TREE_MAC(4, 2),                                                      \
            channel_group_cstu_SUB_TREE_MAC(5, 3)

#define cnm_grp_0_1_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                 \
    /* DataPathIntsSum Interrupt Cause */                                                               \
    {bit, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL,                                    \
        0x000000A4, 0x000000A8,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_MG_##_mgIndex##_DATA_PATH_INTERRUPTS_SUMMARY_CAUSE_DATA_PATH_INT_SUM1_E,          \
        PRV_CPSS_AC5X_MG_##_mgIndex##_DATA_PATH_INTERRUPTS_SUMMARY_CAUSE_DATA_PATH_INT_SUM31_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0x0, 28, NULL, NULL},                                               \
            l2i_tai_SUB_TREE_MAC(1, 0),                                                                 \
            l2i_tai_SUB_TREE_MAC(2, 1),                                                                 \
            tcam_SUB_TREE_MAC(3),                                                                       \
            tti_tai_SUB_TREE_MAC(4, 0),                                                                 \
            tti_tai_SUB_TREE_MAC(5, 1),                                                                 \
            ia_SUB_TREE_MAC(6),                                                                         \
            pb_SUB_TREE_MAC(7),                                                                         \
            rxDma_SUB_TREE_MAC(8),                                                                      \
            txDma_SUB_TREE_MAC(9),                                                                      \
            txFifo_SUB_TREE_MAC(11),                                                                    \
            gop_usx_anp_0_3_SUB_TREE_MAC(13, 0),                                                        \
            gop_usx_anp_0_3_SUB_TREE_MAC(14, 1),                                                        \
            gop_usx_anp_0_3_SUB_TREE_MAC(15, 2),                                                        \
            gop_usx_anp_0_3_SUB_TREE_MAC(16, 3),                                                        \
            gop_usx_anp_4_5_SUB_TREE_MAC(17, 4),                                                        \
            gop_usx_anp_4_5_SUB_TREE_MAC(18, 5),                                                        \
            gop_cpu_anp_SUB_TREE_MAC(19, 0 /*CPU-0*/, 6),                                               \
            gop_cpu_anp_SUB_TREE_MAC(20, 1 /*CPU-1*/, 7),                                               \
            gop_100G_anp_SUB_TREE_MAC(21),                                                              \
            gop_ctsu_SUB_TREE_MAC(22),                                                                  \
            gop_eip_163e_SUB_TREE_MAC(24),                                                              \
            gop_eip_163i_SUB_TREE_MAC(25),                                                              \
            gop_eip_164e_SUB_TREE_MAC(26),                                                              \
            gop_eip_164i_SUB_TREE_MAC(27),                                                              \
            gop_100G_macro_tai_SUB_TREE_MAC(28, 0),                                                     \
            gop_100G_macro_tai_SUB_TREE_MAC(29, 1),                                                     \
            gop_usx2_0_mac_macro_tai_SUB_TREE_MAC(30, 0),                                               \
            gop_usx2_0_mac_macro_tai_SUB_TREE_MAC(31, 1)

#define pds_SUB_TREE_MAC(bit)                                                           \
    /* PDS Interrupt Summary */                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E, NULL,                          \
        0x00043000, 0x000043004,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AC5X_TXQ_SCN_PDS_INTERRUPT_SUMMARY_CAUSE_PDS_FUNCTIONAL_INT_SUM_E,     \
        PRV_CPSS_AC5X_TXQ_SCN_PDS_INTERRUPT_SUMMARY_CAUSE_PDS_DEBUG_INT_SUM_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                         \
        /* PDS Functional Interrupt Summary */                                                                      \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E, NULL,                                                    \
            0x00043008, 0x0004300C,                                                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                        \
            PRV_CPSS_AC5X_TXQ_SCN_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PDS_BAD_ADDRESS_INT_E,                             \
            PRV_CPSS_AC5X_TXQ_SCN_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_E,                         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                 \
        /* PDS Debug Interrupt Summary */                                                                                                                  \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E, NULL,                                                                                           \
            0x00043010, 0x00043014,                                                                                                                        \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                                                                \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                                                               \
            PRV_CPSS_AC5X_TXQ_SCN_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_E,                                                                    \
            PRV_CPSS_AC5X_TXQ_SCN_PDS_INTERRUPT_DEBUG_CAUSE_PB_RD_NOT_PERFORMED_INT_E,                                                                     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define qfc_FUNC_INT_NODE_MAC(bit)                                                                                  \
    /* QFC Functional Interrupt Summary */                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E, NULL,                                                      \
        0x00000200, 0x00000204,                                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
        PRV_CPSS_AC5X_TXQ_SCN_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E,                                    \
        PRV_CPSS_AC5X_TXQ_SCN_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define qfc_DBG_INT_NODE_MAC(bit)                                                                                   \
    /* QFC Debug Interrupt Summary */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E, NULL,                                                      \
        0x00000208, 0x0000020C,                                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
        PRV_CPSS_AC5X_TXQ_SCN_QFC_INTERRUPT_DEBUG_CAUSE_PFC_MESSAGES_DROP_COUNTER_WRAPAROUND_INT_E,                 \
        PRV_CPSS_AC5X_TXQ_SCN_QFC_INTERRUPT_DEBUG_CAUSE_AGGREGATION_BUFFER_COUNT_OVERFLOW_E,                        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define QFC_HR_INT_OFFSET(_regIndex)        ((_regIndex) * 32)                                                      \

#define qfc_HR_INT_NODE_MAC(bit, _regIndex)                                                             \
    /* QFC HR Crossed Threshold Interrupt Summary */                                                                \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E, NULL,                                                      \
        (0x0000021C+(_regIndex*0x4)), (0x000002B4+(_regIndex*0x4)),                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
        PRV_CPSS_AC5X_TXQ_SCN_REGISTER_INDEX_0_PORT_0_TC_0_CROSSED_HR_THRESHOLD_E + QFC_HR_INT_OFFSET(_regIndex),   \
        PRV_CPSS_AC5X_TXQ_SCN_REGISTER_INDEX_0_PORT_2_TC_7_CROSSED_HR_THRESHOLD_E + QFC_HR_INT_OFFSET(_regIndex),   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define qfc_SUB_TREE_MAC(bit)                                                                                       \
    /* QFC Interrupt Summary */                                                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E, NULL,                                                      \
        0x00000210, 0x00000214,                                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
        PRV_CPSS_AC5X_TXQ_SCN_QFC_INTERRUPT_SUMMARY_CAUSE_QFC_FUNCTIONAL_INT_SUM_E,                                 \
        PRV_CPSS_AC5X_TXQ_SCN_QFC_INTERRUPT_SUMMARY_CAUSE_REGISTER_INDEX_18_INT_SUM_E,                              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 21, NULL, NULL},                                                    \
        qfc_FUNC_INT_NODE_MAC(1),                                                                                   \
        qfc_DBG_INT_NODE_MAC(2),                                                                                    \
        qfc_HR_INT_NODE_MAC( 3,  0),                                                                                \
        qfc_HR_INT_NODE_MAC( 4,  1),                                                                                \
        qfc_HR_INT_NODE_MAC( 5,  2),                                                                                \
        qfc_HR_INT_NODE_MAC( 6,  3),                                                                                \
        qfc_HR_INT_NODE_MAC( 7,  4),                                                                                \
        qfc_HR_INT_NODE_MAC( 8,  5),                                                                                \
        qfc_HR_INT_NODE_MAC( 9,  6),                                                                                \
        qfc_HR_INT_NODE_MAC(10,  7),                                                                                \
        qfc_HR_INT_NODE_MAC(11,  8),                                                                                \
        qfc_HR_INT_NODE_MAC(12,  9),                                                                                \
        qfc_HR_INT_NODE_MAC(13, 10),                                                                                \
        qfc_HR_INT_NODE_MAC(14, 11),                                                                                \
        qfc_HR_INT_NODE_MAC(15, 12),                                                                                \
        qfc_HR_INT_NODE_MAC(16, 13),                                                                                \
        qfc_HR_INT_NODE_MAC(17, 14),                                                                                \
        qfc_HR_INT_NODE_MAC(18, 15),                                                                                \
        qfc_HR_INT_NODE_MAC(19, 16),                                                                                \
        qfc_HR_INT_NODE_MAC(20, 17),                                                                                \
        qfc_HR_INT_NODE_MAC(21, 18)


#define sdq_SUB_TREE_MAC(bit)                                                               \
    /* SDQ Interrupt Summary */                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E, NULL,                                           \
        0x00000200, 0x00000204,                                                                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_AC5X_TXQ_SCN_SDQ_INTERRUPT_SUMMARY_CAUSE_SDQ_FUNCTIONAL_INTERRUPT_SUMMARY_E,            \
        PRV_CPSS_AC5X_TXQ_SCN_SDQ_INTERRUPT_SUMMARY_CAUSE_SDQ_DEBUG_INTERRUPT_SUMMARY_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                          \
        /* SDQ Functional Interrupt Summary */                                                                     \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E, NULL,                                                   \
            0x00000208, 0x0000020C,                                                                                \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                        \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                       \
            PRV_CPSS_AC5X_TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                         \
            PRV_CPSS_AC5X_TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_LIST_PTRS_CONFIG_PORT_NOT_EMPTY_E,            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                \
        /* SDQ Debug Interrupt Summary */                                                                          \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E, NULL,                                                   \
            0x00000210, 0x00000214,                                                                                \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                        \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                       \
            PRV_CPSS_AC5X_TXQ_SCN_SDQ_INTERRUPT_DEBUG_CAUSE_QCN_FIFO_OVERRUN_E,                                    \
            PRV_CPSS_AC5X_TXQ_SCN_SDQ_INTERRUPT_DEBUG_CAUSE_QCN_FIFO_OVERRUN_E,                                    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define dfx_server_SUB_TREE_MAC(bit)                                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_DFX_SERVER_E, NULL,                                           \
        0x000F8108, 0x000F810C,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5X_DFX_SERVER_INTERRUPT_WRONG_ADDRESS_FROM_PIPE_0_E,                              \
        PRV_CPSS_AC5X_DFX_SERVER_INTERRUPT_EXTERNAL_TEMPERATURE_THRESHOLD_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define dfx_server_sum_SUB_TREE_MAC(bit)                                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_DFX_SERVER_E, NULL,                                           \
        0x000F8100, 0x000F8104,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_AC5X_DFX_SERVER_INTERRUPT_SUMMARY_SERVER_INTERRUPT_SUM_E,                           \
        PRV_CPSS_AC5X_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_7_INTERRUPT_SUM_E,                           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                      \
        dfx_server_SUB_TREE_MAC(1)

#define cnm_grp_0_2_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                 \
    /* PortsIntsSum Interrupt Cause */                                                                  \
    {bit, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E, _mgIndex), NULL,                                   \
        0x00000080, 0x00000084,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_MG_##_mgIndex##_PORTS_INTERRUPTS_SUMMARY_CAUSE_PORT_INT_SUM_1_E,                  \
        PRV_CPSS_AC5X_MG_##_mgIndex##_PORTS_INTERRUPTS_SUMMARY_CAUSE_PORT_INT_SUM_31_E,                 \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 26, NULL, NULL},                                                 \
        gop_tai_units /* 6 elements */,                                                                 \
        gop_led_units /* 5 elements */,                                                                 \
        gop_lmu_units /* 1 element */,                                                                  \
        gop_mac_pcs /* 9 elements */,                                                                   \
        gop_mif /* 5 elements */
        /* gop_pzarb(27),*/
        /* gop_sdw(29) */

#define cnm_grp_0_3_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                 \
    /* DFX */                                                                                           \
    {bit, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E, _mgIndex), NULL,                                   \
        0x000000AC, 0x000000B0,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_MG_##_mgIndex##_DFX_INTERRUPTS_SUMMARY_CAUSE_DFX_INTERRUPT_SUM_1_E,               \
        PRV_CPSS_AC5X_MG_##_mgIndex##_DFX_INTERRUPTS_SUMMARY_CAUSE_DFX_INTERRUPT_SUM_31_E,              \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 10, NULL, NULL},                                                 \
        gop_PCA_mac_macro_tai_SUB_TREE_MAC(5, 0),                                                       \
        gop_PCA_mac_macro_tai_SUB_TREE_MAC(6, 1),                                                       \
        dfx_server_sum_SUB_TREE_MAC(7),                                                                 \
        pds_SUB_TREE_MAC(8),                                                                            \
        pdx_SUB_TREE_MAC(9),                                                                            \
        pfcc_SUB_TREE_MAC(10),                                                                          \
        psi_SUB_TREE_MAC(11),                                                                           \
        qfc_SUB_TREE_MAC(12),                                                                           \
        sdq_SUB_TREE_MAC(13),                                                                           \
        txq_tai_SUB_TREE_MAC(14)

#define cnm_grp_0_5_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                 \
    /* FuncUnits1IntsSum */                                                                             \
    {bit, GT_FALSE,MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E, _mgIndex), NULL,                                    \
        0x000003F4, 0x000003F0,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5X_MG_##_mgIndex##_FUNCTIONAL_1_INTERRUPTS_SUMMARY_CAUSE_FUNC_UNITS_1_INT_SUM_1_E,   \
        PRV_CPSS_AC5X_MG_##_mgIndex##_FUNCTIONAL_1_INTERRUPTS_SUMMARY_CAUSE_FUNC_UNITS_1_INT_SUM_31_E,  \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 2, NULL, NULL},                                                  \
        /*MG1*/                                                                                         \
        AC5X_MG_INTERRUPTS_MG_1_MAC(phoenix_MG1_int_sum),                                               \
        /*MG1*/                                                                                         \
        AC5X_MG_INTERRUPTS_MG_2_MAC(phoenix_MG2_int_sum)

#define MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, numChild, _mgIndex)  \
    /* AC5X Global Interrupt Cause */                                            \
    {bitIndexInCaller, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL,     \
        0x00000030, 0x00000034,                                             \
        prvCpssDrvHwPpPortGroupIsrRead,                                     \
        prvCpssDrvHwPpPortGroupIsrWrite,                                    \
        PRV_CPSS_AC5X_MG_##_mgIndex##_GLOBAL_INTERRUPTS_SUMMARY_CAUSE_FUNCTIONAL_UNITS_INTERRUPT_SUMMARY_E,   \
        PRV_CPSS_AC5X_MG_##_mgIndex##_GLOBAL_INTERRUPTS_SUMMARY_CAUSE_MG1_INTERNAL_INTERRUPT_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, numChild, NULL, NULL}


#define AC5X_MG_Tx_SDMA_SUMMARY_MAC(bitIndexInCaller, _mgIndex)          \
        /* Tx SDMA  */                                                    \
        {bitIndexInCaller, GT_FALSE,MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL, \
            0x00002810, 0x00002818,                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                               \
            prvCpssDrvHwPpPortGroupIsrWrite,                              \
            PRV_CPSS_AC5X_MG_##_mgIndex##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_TX_BUFFER_QUEUE_0_E, \
            PRV_CPSS_AC5X_MG_##_mgIndex##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_TX_REJECT_0_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define AC5X_MG_Rx_SDMA_SUMMARY_MAC(bitIndexInCaller, _mgIndex) \
        /* Rx SDMA  */                                                    \
        {bitIndexInCaller, GT_FALSE,MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL, \
            0x0000280C, 0x00002814,                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                               \
            prvCpssDrvHwPpPortGroupIsrWrite,                              \
            PRV_CPSS_AC5X_MG_##_mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_RX_BUFFER_QUEUE_0_E, \
            PRV_CPSS_AC5X_MG_##_mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_PACKET_CNT_OF_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define  AC5X_MG_INTERRUPTS_MG_1_MAC(bitIndexInCaller)                       \
    /* Global Interrupt Cause */                                                    \
    MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, 3 /*numChild*/, 1/*mgId*/), \
        /* MG internal Interrupt Cause */                                           \
        mg0InternalIntsSum_SUB_TREE_MAC(7, 1 /*_mgIndex*/),               \
        /* Tx SDMA  */                                                              \
        AC5X_MG_Tx_SDMA_SUMMARY_MAC(8, 1 /*_mgIndex*/),                 \
        /* Rx SDMA  */                                                              \
        AC5X_MG_Rx_SDMA_SUMMARY_MAC(9, 1 /*_mgIndex*/)

#define  AC5X_MG_INTERRUPTS_MG_2_MAC(bitIndexInCaller)                       \
    /* Global Interrupt Cause */                                                    \
    MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, 1 /*numChild*/, 2/*mgId*/), \
        /* MG internal Interrupt Cause */                                           \
        mg0InternalIntsSum_SUB_TREE_MAC(7, 2 /*_mgIndex*/)

#define phoenix_MG1_int_sum 4
#define phoenix_MG2_int_sum 7

static const PRV_CPSS_DRV_INTERRUPT_SCAN_STC phoenixIntrScanArr[] =
{
    /* Global Interrupt Cause */
    MG_GLOBAL_SUMMARY_MAC(0, 8/*numChild*/, 0 /*_mgIndex*/),
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
        AC5X_MG_Tx_SDMA_SUMMARY_MAC(8, 0 /*_mgIndex*/),
        /* Rx SDMA  */
        AC5X_MG_Rx_SDMA_SUMMARY_MAC(9, 0 /*_mgIndex*/),
        /* Func1UnitsIntsSum */
        cnm_grp_0_5_IntsSum_SUB_TREE_MAC(11, 0 /*_mgIndex*/)
};

#define AC5X_NUM_MASK_REGISTERS_CNS  (PRV_CPSS_AC5X_LAST_INT_E / 32)

/* _postFix - must include the "_E" */
#define SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, _port)     \
    PRV_CPSS_AC5X_##_preFix##_##_regIndex##_PORT_##_port##_##_postFix, (((_regIndex) * 16) + _port)

/* _postFix - must include the "_E" */
#define SET_EVENT_PER_REG_PORTS_MAC(_preFix, _regIndex, _postFix)                 \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 0),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 1),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 2),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 3),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 4),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 5),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 6),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 7),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 8),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 9),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 10),       \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 11),       \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 12),       \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 13),       \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 14),       \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 15)


#define SET_EVENT_PER_ALL_PORTS_MAC(_preFix, _postFix)                            \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     1,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     2,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     3,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     4,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     5,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     6,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     7,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     8,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     9,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,    10,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,    11,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,    12,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,    13,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,    14,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,    15,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,    16,  _postFix)

/* _postFix - must include the "_E" */
#define SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_postFix)                                                 \
    PRV_CPSS_AC5X_##_postFix, PRV_CPSS_AC5X_##_postFix

/* _postFix - must include the "_E" */
#define SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(_postFix, _unit, _instance, _index)            \
    PRV_CPSS_AC5X_##_unit##_TAI_INST_##_instance##_##_postFix,  _index

/* _postFix - must include the "_E" */
#define SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(_unit, _index)            \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E, _unit, 0, (_index)),    \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E, _unit, 1, (_index+1))

/* _postFix - must include the "_E" */
#define SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(_unit, _index)            \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_GENERATION_INT_E, _unit, 0, (_index)),    \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_GENERATION_INT_E, _unit, 1, (_index+1))

/* _postFix - must include the "_E" */
#define SET_EVENT_WITH_INDEX_MAC(_postFix, _index)                \
    PRV_CPSS_AC5X_##_postFix,    _index

/* _postFix - must include the "_E" */
#define SET_LMU_EVENT_WITH_INDEX_MAC(_postFix, _threshold, _index)             \
    PRV_CPSS_AC5X_LMU_N_##_threshold##_I_##_index##_##_postFix,  ((31*(_threshold)) +_index)

/* _postFix - must include the "_E" */
#define SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, _prefix, _direction, _source, _inst)           \
    PRV_CPSS_AC5X_MSEC_##_prefix##_##_inst##_##_postFix,            (_source << 8 |(_direction))

/* _postFix - must include the "_E" */
#define PHOENIX_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(_postFix, _source)  \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_164_INST, 0, _source, 1),     \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, ING_164_INST, 1, _source, 2)

/* _postFix - must include the "_E" */
#define PHOENIX_SET_EIP_163_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(_postFix, _source)  \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_163_INST, 0, _source, 0),     \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, ING_163_INST, 1, _source, 1)

/* _postFix - must include the "_E" */
#define SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, _prefix, _direction, _inst)       \
    PRV_CPSS_AC5X_MSEC_##_prefix##_##_inst##_##_postFix,           (_direction)

/* _postFix - must include the "_E" */
#define PHOENIX_SET_EIP_164_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_164_INST, 0, 1)

/* _postFix - must include the "_E" */
#define PHOENIX_SET_EIP_164_EVENT_SOURCE_PER_INGRESS_DIRECTION_ALL_DP_MAC(_postFix, _source)  \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, ING_164_INST, 1, _source, 2)

/* _postFix - must include the "_E" */
#define PHOENIX_SET_EIP_66_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_66_INST, 0, 1)

/* _postFix - must include the "_E" */
#define PHOENIX_SET_EIP_164_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_164_INST, 0, 1),      \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, ING_164_INST, 1, 2)

#define SET_EVENT_ALL_DIRECTIONS_ALL_DP_MAC(_prefix, _index, _direction, _inst)            \
    PRV_CPSS_AC5X_MSEC_##_prefix##_##_inst##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_##_index##_E,  (_index << 8 |(_direction))

#define PHOENIX_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_index)    \
    SET_EVENT_ALL_DIRECTIONS_ALL_DP_MAC(EGR_66_INST, _index, 0, 1),             \
    SET_EVENT_ALL_DIRECTIONS_ALL_DP_MAC(ING_66_INST, _index, 1, 2)

/* _postFix - must include the "_E" */
#define SET_INDEX_EVENT_PER_PER_16_USX_PORTS_MAC(_postFix, _usxFrom, _usxTo)             \
    PRV_CPSS_AC5X_MAC_##_usxFrom##_##_usxTo##_0_PORT_NUM_0_##_postFix, ((_usxFrom/2) * 16) +  0, \
    PRV_CPSS_AC5X_MAC_##_usxFrom##_##_usxTo##_0_PORT_NUM_1_##_postFix, ((_usxFrom/2) * 16) +  1, \
    PRV_CPSS_AC5X_MAC_##_usxFrom##_##_usxTo##_0_PORT_NUM_2_##_postFix, ((_usxFrom/2) * 16) +  2, \
    PRV_CPSS_AC5X_MAC_##_usxFrom##_##_usxTo##_0_PORT_NUM_3_##_postFix, ((_usxFrom/2) * 16) +  3, \
    PRV_CPSS_AC5X_MAC_##_usxFrom##_##_usxTo##_0_PORT_NUM_4_##_postFix, ((_usxFrom/2) * 16) +  4, \
    PRV_CPSS_AC5X_MAC_##_usxFrom##_##_usxTo##_0_PORT_NUM_5_##_postFix, ((_usxFrom/2) * 16) +  5, \
    PRV_CPSS_AC5X_MAC_##_usxFrom##_##_usxTo##_0_PORT_NUM_6_##_postFix, ((_usxFrom/2) * 16) +  6, \
    PRV_CPSS_AC5X_MAC_##_usxFrom##_##_usxTo##_0_PORT_NUM_7_##_postFix, ((_usxFrom/2) * 16) +  7, \
    PRV_CPSS_AC5X_MAC_##_usxFrom##_##_usxTo##_1_PORT_NUM_0_##_postFix, ((_usxFrom/2) * 16) +  8, \
    PRV_CPSS_AC5X_MAC_##_usxFrom##_##_usxTo##_1_PORT_NUM_1_##_postFix, ((_usxFrom/2) * 16) +  9, \
    PRV_CPSS_AC5X_MAC_##_usxFrom##_##_usxTo##_1_PORT_NUM_2_##_postFix, ((_usxFrom/2) * 16) + 10, \
    PRV_CPSS_AC5X_MAC_##_usxFrom##_##_usxTo##_1_PORT_NUM_3_##_postFix, ((_usxFrom/2) * 16) + 11, \
    PRV_CPSS_AC5X_MAC_##_usxFrom##_##_usxTo##_1_PORT_NUM_4_##_postFix, ((_usxFrom/2) * 16) + 12, \
    PRV_CPSS_AC5X_MAC_##_usxFrom##_##_usxTo##_1_PORT_NUM_5_##_postFix, ((_usxFrom/2) * 16) + 13, \
    PRV_CPSS_AC5X_MAC_##_usxFrom##_##_usxTo##_1_PORT_NUM_6_##_postFix, ((_usxFrom/2) * 16) + 14, \
    PRV_CPSS_AC5X_MAC_##_usxFrom##_##_usxTo##_1_PORT_NUM_7_##_postFix, ((_usxFrom/2) * 16) + 15

/* _postFix - must include the "_E" */
#define SET_INDEX_EVENT_PER_100G_PORTS_MAC(_postFix)                    \
    PRV_CPSS_AC5X_100_MAC_PORT_NUM_0_##_postFix,  50,                    \
    PRV_CPSS_AC5X_100_MAC_PORT_NUM_1_##_postFix,  51,                    \
    PRV_CPSS_AC5X_100_MAC_PORT_NUM_2_##_postFix,  52,                    \
    PRV_CPSS_AC5X_100_MAC_PORT_NUM_3_##_postFix,  53


/* state for ports 0..47 that event hold extra port number */
#define SET_INDEX_EQUAL_EVENT_PER_48_USX_PORTS_MAC(_postFix)            \
    SET_INDEX_EVENT_PER_PER_16_USX_PORTS_MAC(_postFix, 0, 1),           \
    SET_INDEX_EVENT_PER_PER_16_USX_PORTS_MAC(_postFix, 2, 3),           \
    SET_INDEX_EVENT_PER_PER_16_USX_PORTS_MAC(_postFix, 4, 5)


#define PORTS_LINK_STATUS_CHANGE_EVENTS \
    /* USX total ports 0..47 */                                                             \
    SET_INDEX_EQUAL_EVENT_PER_48_USX_PORTS_MAC(LINK_STATUS_CHANGE_E),                       \
    SET_INDEX_EQUAL_EVENT_PER_48_USX_PORTS_MAC(LPCS_LINK_STATUS_CHANGE_E),                  \
    /* 100G ports */                                                                        \
    SET_INDEX_EVENT_PER_100G_PORTS_MAC(LINK_STATUS_CHANGE_E),                               \
    SET_INDEX_EVENT_PER_100G_PORTS_MAC(LPCS_LINK_STATUS_CHANGE_E)

#define PORTS_LINK_OK_CHANGE_EVENTS \
    /* USX total ports 0..47 */                                                             \
    SET_INDEX_EQUAL_EVENT_PER_48_USX_PORTS_MAC(LINK_OK_CHANGE_E),                           \
    /* 100G ports */                                                                        \
    SET_INDEX_EVENT_PER_100G_PORTS_MAC(LINK_OK_CHANGE_E)

#define AC5X_PORT_AN_HCD_RESOLUTION_DONE_EVENTS                                 \
    PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_0_HCD_FOUND_E,                           50, \
    PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_1_HCD_FOUND_E,                           51, \
    PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_2_HCD_FOUND_E,                           52, \
    PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_3_HCD_FOUND_E,                           53, \
    PRV_CPSS_AC5X_PCS_ANP6_7_6_PORT_NUM_0_HCD_FOUND_E,                      48, \
    PRV_CPSS_AC5X_PCS_ANP6_7_7_PORT_NUM_0_HCD_FOUND_E,                      49, \
    PRV_CPSS_AC5X_PCS_ANP4_5_4_PORT_NUM_0_HCD_FOUND_E,                      32, \
    PRV_CPSS_AC5X_PCS_ANP4_5_5_PORT_NUM_0_HCD_FOUND_E,                      40, \
    PRV_CPSS_AC5X_PCS_ANP0_3_0_PORT_NUM_0_HCD_FOUND_E,                      0,  \
    PRV_CPSS_AC5X_PCS_ANP0_3_1_PORT_NUM_0_HCD_FOUND_E,                      8,  \
    PRV_CPSS_AC5X_PCS_ANP0_3_2_PORT_NUM_0_HCD_FOUND_E,                      16, \
    PRV_CPSS_AC5X_PCS_ANP0_3_3_PORT_NUM_0_HCD_FOUND_E,                      24

#define AC5X_PORT_AN_RESTART_DONE_EVENTS                                 \
    PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_0_AN_RESTART_E,                           50, \
    PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_1_AN_RESTART_E,                           51, \
    PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_2_AN_RESTART_E,                           52, \
    PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_3_AN_RESTART_E,                           53, \
    PRV_CPSS_AC5X_PCS_ANP6_7_6_PORT_NUM_0_AN_RESTART_E,                      48, \
    PRV_CPSS_AC5X_PCS_ANP6_7_7_PORT_NUM_0_AN_RESTART_E,                      49, \
    PRV_CPSS_AC5X_PCS_ANP4_5_4_PORT_NUM_0_AN_RESTART_E,                      32, \
    PRV_CPSS_AC5X_PCS_ANP4_5_5_PORT_NUM_0_AN_RESTART_E,                      40, \
    PRV_CPSS_AC5X_PCS_ANP0_3_0_PORT_NUM_0_AN_RESTART_E,                      0,  \
    PRV_CPSS_AC5X_PCS_ANP0_3_1_PORT_NUM_0_AN_RESTART_E,                      8,  \
    PRV_CPSS_AC5X_PCS_ANP0_3_2_PORT_NUM_0_AN_RESTART_E,                      16, \
    PRV_CPSS_AC5X_PCS_ANP0_3_3_PORT_NUM_0_AN_RESTART_E,                      24

#define AC5X_PORT_INT_PM_PCS_LINK_TIMER_OUT_EVENTS                                 \
    PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,                           50, \
    PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_1_INT_PM_PCS_LINK_TIMER_OUT_E,                           51, \
    PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_2_INT_PM_PCS_LINK_TIMER_OUT_E,                           52, \
    PRV_CPSS_AC5X_ANP4_IP_PORT_NUM_3_INT_PM_PCS_LINK_TIMER_OUT_E,                           53, \
    PRV_CPSS_AC5X_PCS_ANP6_7_6_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,                      48, \
    PRV_CPSS_AC5X_PCS_ANP6_7_7_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,                      49, \
    PRV_CPSS_AC5X_PCS_ANP4_5_4_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,                      32, \
    PRV_CPSS_AC5X_PCS_ANP4_5_5_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,                      40, \
    PRV_CPSS_AC5X_PCS_ANP0_3_0_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,                      0,  \
    PRV_CPSS_AC5X_PCS_ANP0_3_1_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,                      8,  \
    PRV_CPSS_AC5X_PCS_ANP0_3_2_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,                      16, \
    PRV_CPSS_AC5X_PCS_ANP0_3_3_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,                      24

#define AC5X_CPU_PORT_LINK_STATUS_CHANGE_EVENTS \
    /* CPU ports */                                                                         \
    PRV_CPSS_AC5X_MAC_6_7_0_PORT_INTERRUPT_CAUSE_LINK_STATUS_CHANGED_E,    48,         \
    PRV_CPSS_AC5X_MAC_6_7_1_PORT_INTERRUPT_CAUSE_LINK_STATUS_CHANGED_E,    49

#define AC5X_CPU_PORT_LINK_OK_CHANGED_EVENTS \
    /* CPU ports */                                                                         \
    PRV_CPSS_AC5X_MAC_6_7_0_PORT_INTERRUPT_CAUSE_LINK_OK_CHANGED_E,        48,         \
    PRV_CPSS_AC5X_MAC_6_7_1_PORT_INTERRUPT_CAUSE_LINK_OK_CHANGED_E,        49


#define SINGLE_MG_EVENT_MAC(_postFix, _mgUnitId, _index)                                        \
    PRV_CPSS_AC5X_MG_##_mgUnitId##_MG_INTERNAL_INTERRUPT_CAUSE_##_postFix, (((_mgUnitId)*8) + _index)

#define ALL_MG_EVENTS_MAC(_postFix, _index)                                                   \
    SINGLE_MG_EVENT_MAC(_postFix, 0, _index),                                                 \
    SINGLE_MG_EVENT_MAC(_postFix, 1, _index),                                                 \
    SINGLE_MG_EVENT_MAC(_postFix, 2, _index)

#define SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, _localQueue)  \
    PRV_CPSS_AC5X_MG_##_mgUnitId##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_##type##_##_localQueue##_E,  (_mgUnitId*8 + _localQueue)

#define SINGLE_MG_TX_SDMA_QUEUES_MAC(type, _mgUnitId)                  \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 0/*local queue 0*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 1/*local queue 1*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 2/*local queue 2*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 3/*local queue 3*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 4/*local queue 4*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 5/*local queue 5*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 6/*local queue 6*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 7/*local queue 7*/)

#define ALL_MG_TX_SDMA_QUEUES(type)         \
    SINGLE_MG_TX_SDMA_QUEUES_MAC(type, 0/*mg0*/),  \
    SINGLE_MG_TX_SDMA_QUEUES_MAC(type, 1/*mg1*/)


/* connect unified event of 'RX SDMA' to their interrupt */
/* specific unified event to specific interrupt */
#define SINGLE_RX_SDMA_INT_MAC(type, local_mgUnitId, globalQueue, localQueue) \
    CPSS_PP_##type##_QUEUE##globalQueue##_E,                              \
        PRV_CPSS_AC5X_MG_##local_mgUnitId##_RECEIVE_SDMA_INTERRUPT_CAUSE0_##type##_QUEUE_##localQueue##_E, globalQueue, \
    MARK_END_OF_UNI_EV_CNS



#define SINGLE_MG_RX_SDMA_MAC(mgIndex) \
    PRV_CPSS_AC5X_MG_##mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_RESOURCE_ERROR_CNT_OF_E,     ((mgIndex)*6)+0, \
    PRV_CPSS_AC5X_MG_##mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_BYTE_CNT_OF_E,               ((mgIndex)*6)+1, \
    PRV_CPSS_AC5X_MG_##mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_PACKET_CNT_OF_E,             ((mgIndex)*6)+2


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
    SINGLE_RX_SDMA_INT_MAC(type, 1/*local mg*/,  8/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type, 1/*local mg*/,  9/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type, 1/*local mg*/, 10/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type, 1/*local mg*/, 11/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type, 1/*local mg*/, 12/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type, 1/*local mg*/, 13/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type, 1/*local mg*/, 14/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type, 1/*local mg*/, 15/*globalQueue*/,7/*localQueue*/)


/* connect unified event of 'RX SDMA' to their interrupt */
/* ALL MGs [0..15] */
#define ALL_MGS_RX_SDMA_INT_MAC(type)    \
    MG0_RX_SDMA_INT_MAC(type),           \
    MG1_RX_SDMA_INT_MAC(type)

/* support for 'multi PHA units' (we have one per pipe = 2 per tile)
    add the 'portGroupId' as : (portGroupId) << 16 */
#define PHA_INSTANCE_OFFSET(_tileId,_pipe) (((_tileId*2)+_pipe) << 16)

/* Maximal PPN events */
#define PPN_EVENT_TOTAL    16

#define SET_PPN_EVENT_MAC(_ppg, _ppn)   \
    PRV_CPSS_AC5X_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_HOST_UNMAPPED_ACCESS_E             , ((8 + (PPN_EVENT_TOTAL * 0) + (_ppg * 8)) + (_ppn)),  \
    PRV_CPSS_AC5X_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_CORE_UNMAPPED_ACCESS_E             , ((8 + (PPN_EVENT_TOTAL * 1) + (_ppg * 8)) + (_ppn)),  \
    PRV_CPSS_AC5X_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_NEAR_EDGE_IMEM_ACCESS_E            , ((8 + (PPN_EVENT_TOTAL * 2) + (_ppg * 8)) + (_ppn)),  \
    PRV_CPSS_AC5X_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_DOORBELL_INTERRUPT_E               , ((8 + (PPN_EVENT_TOTAL * 3) + (_ppg * 8)) + (_ppn))

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
   PRV_CPSS_AC5X_PHA_PHA_INTERNAL_ERROR_CAUSE_PHA_BAD_ADDRESS_ERROR_E,                                      0,   \
   PRV_CPSS_AC5X_PHA_PHA_INTERNAL_ERROR_CAUSE_PHA_TABLE_ACCESS_OVERLAP_ERROR_E,                             512, \
   PRV_CPSS_AC5X_PHA_PHA_INTERNAL_ERROR_CAUSE_HEADER_SIZE_VIOLATION_E,                                      513, \
   PRV_CPSS_AC5X_PHA_PHA_INTERNAL_ERROR_CAUSE_PPA_CLOCK_DOWN_VIOLATION_E,                                   514, \
   /* PPGs */                                                                                                    \
   PRV_CPSS_AC5X_PPG_0_PPG_INTERNAL_ERROR_CAUSE_PPG_BAD_ADDRESS_ERROR_E,                                    4,   \
   PRV_CPSS_AC5X_PPG_1_PPG_INTERNAL_ERROR_CAUSE_PPG_BAD_ADDRESS_ERROR_E,                                    5,   \
                                                                                                                 \
   PRV_CPSS_AC5X_PPG_0_PPG_INTERNAL_ERROR_CAUSE_PPG_TABLE_ACCESS_OVERLAP_ERROR_E,                           515, \
   PRV_CPSS_AC5X_PPG_1_PPG_INTERNAL_ERROR_CAUSE_PPG_TABLE_ACCESS_OVERLAP_ERROR_E,                           516, \
   /* PPNs */                                                                                                    \
   SET_PPG_EVENT_PER_8_PPN_MAC(0),                                                                               \
   SET_PPG_EVENT_PER_8_PPN_MAC(1)

#define LMU_LATENCY_OVER_THRESHOLD_MAC(_threshold)   \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   0),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   1),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   2),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   3),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   4),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   5),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   6),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   7),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   8),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   9),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  10),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  11),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  12),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  13),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  14),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  15),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  16),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  17),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  18),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  19),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  20),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  21),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  22),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  23),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  24),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  25),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  26),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  27),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  28),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  29),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  30)

#define SET_HR_TRSH_EVENT_MAC(_regIndex,_port,_tc) \
    PRV_CPSS_AC5X_TXQ_SCN_REGISTER_INDEX_##_regIndex##_PORT_##_port##_TC_##_tc##_CROSSED_HR_THRESHOLD_E, (((_port) * 8) + (_tc))

#define SET_HR_TRSH_EVENT_ALL_TC_MAC(_regIndex, _port)                          \
    SET_HR_TRSH_EVENT_MAC(_regIndex, _port, 0),                                 \
    SET_HR_TRSH_EVENT_MAC(_regIndex, _port, 1),                                 \
    SET_HR_TRSH_EVENT_MAC(_regIndex, _port, 2),                                 \
    SET_HR_TRSH_EVENT_MAC(_regIndex, _port, 3),                                 \
    SET_HR_TRSH_EVENT_MAC(_regIndex, _port, 4),                                 \
    SET_HR_TRSH_EVENT_MAC(_regIndex, _port, 5),                                 \
    SET_HR_TRSH_EVENT_MAC(_regIndex, _port, 6),                                 \
    SET_HR_TRSH_EVENT_MAC(_regIndex, _port, 7)

#define PHOENIX_SET_HR_TRSH_EVENT_ALL_PORTS_MAC       \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 0,  0),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 0,  1),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 0,  2),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 1,  3),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 1,  4),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 1,  5),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 2,  6),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 2,  7),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 2,  8),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 3,  9),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 3, 10),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 3, 11),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 4, 12),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 4, 13),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 4, 14),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 5, 15),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 5, 16),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 5, 17),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 6, 18),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 6, 19),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 6, 20),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 7, 21),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 7, 22),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 7, 23),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 8, 24),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 8, 25),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 8, 26),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 9, 27),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 9, 28),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC( 9, 29),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(10, 30),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(10, 31),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(10, 32),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(11, 33),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(11, 34),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(11, 35),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(12, 36),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(12, 37),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(12, 38),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(13, 39),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(13, 40),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(13, 41),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(14, 42),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(14, 43),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(14, 44),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(15, 45),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(15, 46),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(15, 47),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(16, 48),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(16, 49),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(16, 50),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(17, 51),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(17, 52),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(17, 53),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(18, 54),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(18, 55),             \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(18, 56)

#define CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _channel,  _extData, _port)                        \
        PRV_CPSS_AC5X_CTSU_CHANNEL_##_channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_0_##_postfix, (((_port) << 8) | (_extData))

#define CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _channel, _extData, _port)                         \
        PRV_CPSS_AC5X_CTSU_CHANNEL_##_channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_1_##_postfix, (((_port) << 8) | (_extData))


#define SET_EVENT_GROUP_CHANNEL_CTSU_64_PCA_MAC(_postfix, _extData)                                               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  0, _extData,  0),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  1, _extData,  1),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  2, _extData,  2),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  3, _extData,  3),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  4, _extData,  4),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  5, _extData,  5),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  6, _extData,  6),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  7, _extData,  7),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  8, _extData,  8),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  9, _extData,  9),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 10, _extData, 10),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 11, _extData, 11),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 12, _extData, 12),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 13, _extData, 13),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 14, _extData, 14),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 15, _extData, 15),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 16, _extData, 16),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 17, _extData, 17),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 18, _extData, 18),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 19, _extData, 19),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 20, _extData, 20),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 21, _extData, 21),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 22, _extData, 22),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 23, _extData, 23),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 24, _extData, 24),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 25, _extData, 25),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 26, _extData, 26),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 27, _extData, 27),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 28, _extData, 28),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 29, _extData, 29),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 30, _extData, 30),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 31, _extData, 31),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 32, _extData, 32),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 33, _extData, 33),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 34, _extData, 34),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 35, _extData, 35),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 36, _extData, 36),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 37, _extData, 37),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 38, _extData, 38),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 39, _extData, 39),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 40, _extData, 40),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 41, _extData, 41),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 42, _extData, 42),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 43, _extData, 43),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 44, _extData, 44),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 45, _extData, 45),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 46, _extData, 46),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 47, _extData, 47),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 48, _extData, 48),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 49, _extData, 49),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 50, _extData, 50),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 51, _extData, 51),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 52, _extData, 52),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 53, _extData, 53),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 54, _extData, 54),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 55, _extData, 55),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 56, _extData, 56),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 57, _extData, 57),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 58, _extData, 58),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 59, _extData, 59),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 60, _extData, 60),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 61, _extData, 61),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 62, _extData, 62),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 63, _extData, 63)

#define PHOENIX_COMMON_EVENTS                                                                                           \
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
CPSS_PP_GTS_GLOBAL_FIFO_FULL_E,                                                                                         \
    PRV_CPSS_AC5X_ERMRK_ERMRK_INTERRUPT_CAUSE_INGRESS_TIMESTAMP_QUEUE0_FULL_E,  0,                                      \
    PRV_CPSS_AC5X_ERMRK_ERMRK_INTERRUPT_CAUSE_EGRESS_TIMESTAMP_QUEUE0_FULL_E,   1,                                      \
    PRV_CPSS_AC5X_ERMRK_ERMRK_INTERRUPT_CAUSE_INGRESS_TIMESTAMP_QUEUE1_FULL_E,  2,                                      \
    PRV_CPSS_AC5X_ERMRK_ERMRK_INTERRUPT_CAUSE_EGRESS_TIMESTAMP_QUEUE1_FULL_E,   3,                                      \
    SET_EVENT_GROUP_CHANNEL_CTSU_64_PCA_MAC(OVERFLOW_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,0,1)),                        \
    SET_EVENT_GROUP_CHANNEL_CTSU_64_PCA_MAC(OVERFLOW_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,1,1)),                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,                                                                               \
    PRV_CPSS_AC5X_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_INGRESS_TIMESTAMP_Q0_E,  0,                                           \
    PRV_CPSS_AC5X_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_EGRESS_TIMESTAMP_Q0_E,   1,                                           \
    PRV_CPSS_AC5X_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_INGRESS_TIMESTAMP_Q1_E,  2,                                           \
    PRV_CPSS_AC5X_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_EGRESS_TIMESTAMP_Q1_E,   3,                                           \
    SET_EVENT_GROUP_CHANNEL_CTSU_64_PCA_MAC(NEW_ENTRY_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,0,1)),                       \
    SET_EVENT_GROUP_CHANNEL_CTSU_64_PCA_MAC(NEW_ENTRY_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,1,1)),                       \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E,                                                                                     \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(L2I_TAI_1,         0),                                                 \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(100G_TAI_10,       2),                                                 \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(CPU_TAI_8,         4),                                                 \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(CPU_TAI_9,         6),                                                 \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(USX2_1_TAI_7,      8),                                                 \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(USX2_1_TAI_7,      10),                                                \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(PCA_USX2_0_TAI_5,  12),                                                \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(PCA_USX2_0_TAI_6,  14),                                                \
    PRV_CPSS_AC5X_TXQ_TAI_4_TAI_INST_0_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E, 16,                                  \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(PHA_EPCL_TAI_2,    17),                                                \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(PHA_EPCL_TAI_3,    19),                                                \
    SET_TAI_INCOMING_TRIGER_EVENT_WITH_INDEX_MAC(TTI_TAI_0,         21),                                                \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PTP_TAI_GENERATION_E,                                                                                          \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(L2I_TAI_1,         0),                                                      \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(100G_TAI_10,       2),                                                      \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(CPU_TAI_8,         4),                                                      \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(CPU_TAI_9,         6),                                                      \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(USX2_1_TAI_7,      8),                                                      \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(USX2_1_TAI_7,      10),                                                     \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(PCA_USX2_0_TAI_5,  12),                                                     \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(PCA_USX2_0_TAI_6,  14),                                                     \
    PRV_CPSS_AC5X_TXQ_TAI_4_TAI_INST_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E, 16,                                        \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(PHA_EPCL_TAI_2,    17),                                                     \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(PHA_EPCL_TAI_3,    19),                                                     \
    SET_TAI_GENERATION_EVENT_WITH_INDEX_MAC(TTI_TAI_0,         21),                                                     \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 ALL_MGS_RX_SDMA_INT_MAC(RX_BUFFER),                                                                                    \
 ALL_MGS_RX_SDMA_INT_MAC(RX_ERROR),                                                                                     \
                                                                                                                        \
 CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,                                                                                       \
    /* USX total ports 0..47 */                                                                                         \
    SET_INDEX_EQUAL_EVENT_PER_48_USX_PORTS_MAC(MAC_TX_UNDERFLOW_E),                                                     \
    SET_INDEX_EVENT_PER_100G_PORTS_MAC(MAC_TX_UNDERFLOW_E),                                                             \
    /* CPU ports */                                                                                                     \
    PRV_CPSS_AC5X_MAC_6_7_0_PORT_INTERRUPT_CAUSE_TX_UNDERFLOW_E,            48,                                         \
    PRV_CPSS_AC5X_MAC_6_7_1_PORT_INTERRUPT_CAUSE_TX_UNDERFLOW_E,            49,                                         \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PORT_TX_FIFO_OVERRUN_E,                                                                                        \
    /* USX total ports 0..47 */                                                                                         \
    SET_INDEX_EQUAL_EVENT_PER_48_USX_PORTS_MAC(MAC_TX_OVR_ERR_E),                                                       \
    SET_INDEX_EVENT_PER_100G_PORTS_MAC(MAC_TX_OVR_ERR_E),                                                               \
    /* CPU ports */                                                                                                     \
    PRV_CPSS_AC5X_MAC_6_7_0_PORT_INTERRUPT_CAUSE_TX_OVR_ERR_E,            48,                                           \
    PRV_CPSS_AC5X_MAC_6_7_1_PORT_INTERRUPT_CAUSE_TX_OVR_ERR_E,            49,                                           \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PORT_RX_FIFO_OVERRUN_E,                                                                                        \
    /* USX total ports 0..47 */                                                                                         \
    SET_INDEX_EQUAL_EVENT_PER_48_USX_PORTS_MAC(RX_OVERRUN_E),                                                           \
    SET_INDEX_EVENT_PER_100G_PORTS_MAC(RX_OVERRUN_E),                                                                   \
    /* CPU ports */                                                                                                     \
    PRV_CPSS_AC5X_MAC_6_7_0_PORT_INTERRUPT_CAUSE_RX_OVERRUN_E,            48,                                           \
    PRV_CPSS_AC5X_MAC_6_7_1_PORT_INTERRUPT_CAUSE_RX_OVERRUN_E,            49,                                           \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
  /* Per Index events */                                                                                                \
 CPSS_PP_GPP_E,                                                                                                         \
    ALL_MG_EVENTS_MAC(GPP0_INTERRUPT_CAUSE_E, 0),                                                                 \
    ALL_MG_EVENTS_MAC(GPP1_INTERRUPT_CAUSE_E, 1),                                                                 \
    ALL_MG_EVENTS_MAC(GPP2_INTERRUPT_CAUSE_E, 2),                                                                 \
    ALL_MG_EVENTS_MAC(GPP3_INTERRUPT_CAUSE_E, 3),                                                                 \
    ALL_MG_EVENTS_MAC(GPP4_INTERRUPT_CAUSE_E, 4),                                                                 \
    ALL_MG_EVENTS_MAC(GPP5_INTERRUPT_CAUSE_E, 5),                                                                 \
    ALL_MG_EVENTS_MAC(GPP6_INTERRUPT_CAUSE_E, 6),                                                                 \
    ALL_MG_EVENTS_MAC(GPP7_INTERRUPT_CAUSE_E, 7),                                                                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_LMU_LATENCY_EXCEED_THRESHOLD_E,                                                                                \
    LMU_LATENCY_OVER_THRESHOLD_MAC(0),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(1),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(2),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(3),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(4),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(5),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(6),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(7),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(8),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(9),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(10),                                                                                 \
    LMU_LATENCY_OVER_THRESHOLD_MAC(11),                                                                                 \
    LMU_LATENCY_OVER_THRESHOLD_MAC(12),                                                                                 \
    LMU_LATENCY_OVER_THRESHOLD_MAC(13),                                                                                 \
    LMU_LATENCY_OVER_THRESHOLD_MAC(14),                                                                                 \
    LMU_LATENCY_OVER_THRESHOLD_MAC(15),                                                                                 \
    LMU_LATENCY_OVER_THRESHOLD_MAC(16),                                                                                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_CNC_WRAPAROUND_BLOCK_E,                                                                                        \
    SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_0_WRAPAROUND_E,  0),       \
    SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_1_WRAPAROUND_E,  1),       \
    SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_2_WRAPAROUND_E,  2),       \
    SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_3_WRAPAROUND_E,  3),       \
    SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_4_WRAPAROUND_E,  4),       \
    SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_5_WRAPAROUND_E,  5),       \
    SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_6_WRAPAROUND_E,  6),       \
    SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_7_WRAPAROUND_E,  7),       \
    SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_8_WRAPAROUND_E,  8),       \
    SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_9_WRAPAROUND_E,  9),       \
    SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_10_WRAPAROUND_E, 10),      \
    SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_11_WRAPAROUND_E, 11),      \
    SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_12_WRAPAROUND_E, 12),      \
    SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_13_WRAPAROUND_E, 13),      \
    SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_14_WRAPAROUND_E, 14),      \
    SET_EVENT_WITH_INDEX_MAC(CNC_INST_0_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_15_WRAPAROUND_E, 15),      \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_RX_CNTR_OVERFLOW_E,                                                                                            \
    SINGLE_MG_RX_SDMA_MAC(0),                                                                                           \
    SINGLE_MG_RX_SDMA_MAC(1),                                                                                           \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_SCT_RATE_LIMITER_E,                                                                                            \
    SET_EVENT_PER_ALL_PORTS_MAC(EQ_REG, PKT_DROPED_INT_E),                                                              \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E,                                                                                  \
    SET_EVENT_WITH_INDEX_MAC(IPLR0_POLICER_INTERRUPT_CAUSE_ILLEGAL_ACCESS_E,       0),                                  \
    SET_EVENT_WITH_INDEX_MAC(IPLR1_POLICER_INTERRUPT_CAUSE_ILLEGAL_ACCESS_E,       1),                                  \
    SET_EVENT_WITH_INDEX_MAC(EPLR_POLICER_INTERRUPT_CAUSE_ILLEGAL_ACCESS_E,        2),                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E,                                                                                   \
    SET_EVENT_WITH_INDEX_MAC(IPLR0_POLICER_INTERRUPT_CAUSE_IPFIX_WRAPAROUND_E,            0),                           \
    SET_EVENT_WITH_INDEX_MAC(IPLR1_POLICER_INTERRUPT_CAUSE_IPFIX_WRAPAROUND_E,            1),                           \
    SET_EVENT_WITH_INDEX_MAC(EPLR_POLICER_INTERRUPT_CAUSE_IPFIX_WRAPAROUND_E,             2),                           \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_POLICER_IPFIX_ALARM_E,                                                                                         \
    SET_EVENT_WITH_INDEX_MAC(IPLR0_POLICER_INTERRUPT_CAUSE_IPFIX_ALARM_E,                 0),                           \
    SET_EVENT_WITH_INDEX_MAC(IPLR1_POLICER_INTERRUPT_CAUSE_IPFIX_ALARM_E,                 1),                           \
    SET_EVENT_WITH_INDEX_MAC(EPLR_POLICER_INTERRUPT_CAUSE_IPFIX_ALARM_E,                  2),                           \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,                                                              \
    SET_EVENT_WITH_INDEX_MAC(IPLR0_POLICER_INTERRUPT_CAUSE_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    0),              \
    SET_EVENT_WITH_INDEX_MAC(IPLR1_POLICER_INTERRUPT_CAUSE_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    1),              \
    SET_EVENT_WITH_INDEX_MAC(EPLR_POLICER_INTERRUPT_CAUSE_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,     2),              \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,                                                                                       \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    0),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    0),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    0),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    0),                 \
                                                                                                                        \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    1),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    1),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    1),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    1),                 \
                                                                                                                        \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    2),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    2),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    2),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    2),                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PCL_LOOKUP_FIFO_FULL_E,                                                                                        \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_LOOKUP0_FIFO_FULL_E,                      0),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_LOOKUP1_FIFO_FULL_E,                      1),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_LOOKUP2_FIFO_FULL_E,                      2),                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PCL_ACTION_TRIGGERED_E,                                                                                        \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_IPCL0_TCAM_TRIGGERED_INTERRUPT_E,         0),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_IPCL1_TCAM_TRIGGERED_INTERRUPT_E,         1),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_IPCL2_TCAM_TRIGGERED_INTERRUPT_E,         2),                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_EB_NA_FIFO_FULL_E,                                                                                             \
    SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_0_NA_FIFO_FULL_E,         0),                 \
    SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_1_NA_FIFO_FULL_E,         1),                 \
    SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_2_NA_FIFO_FULL_E,         2),                 \
    SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_3_NA_FIFO_FULL_E,         3),                 \
    SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_4_NA_FIFO_FULL_E,         4),                 \
    SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_5_NA_FIFO_FULL_E,         5),                 \
    SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_6_NA_FIFO_FULL_E,         6),                 \
    SET_EVENT_WITH_INDEX_MAC(FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_7_NA_FIFO_FULL_E,         7),                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_TTI_ACCESS_DATA_ERROR_E,                                                                                       \
    SET_EVENT_WITH_INDEX_MAC(TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_0_ACCESS_DATA_ERROR_E, 0),                        \
    SET_EVENT_WITH_INDEX_MAC(TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_1_ACCESS_DATA_ERROR_E, 1),                        \
    SET_EVENT_WITH_INDEX_MAC(TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_2_ACCESS_DATA_ERROR_E, 2),                        \
    SET_EVENT_WITH_INDEX_MAC(TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_3_ACCESS_DATA_ERROR_E, 3),                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
/* next HW interrupt value will have same index (evExtData) value in CPSS_PP_CRITICAL_HW_ERROR_E */                     \
 CPSS_PP_CRITICAL_HW_ERROR_E,                                                                                           \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E),    \
\
    /*PSI_REG*/                                                                                                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PSI_REGS_PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_E),                               \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PSI_REGS_PSI_INTERRUPT_CAUSE_MG2SNAKE_INTERRUPT_E),                                \
\
    /*TXQ_QFC*/                                                                                                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_QFC_INTERRUPT_FUNCTIONAL_CAUSE_PORT_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E),\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E),\
\
    /*TXQ_PFCC*/                                                                                                        \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_PR_PFCC_INTERRUPT_CAUSE_POOL_0_COUNTER_OVERFLOW_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_PR_PFCC_INTERRUPT_CAUSE_SOURCE_PORT_COUNTER_OVERFLOW_E),                       \
\
    /*HBU*/                                                                                                             \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HBU_HBU_INTERRUPT_CAUSE_REGISTER_ERROR_FHF_MEM_E),                                 \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HBU_HBU_INTERRUPT_CAUSE_REGISTER_ERROR_CMT1_MEM_E),                                \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HBU_HBU_INTERRUPT_CAUSE_REGISTER_ERROR_CMT2_MEM_E),                                \
\
    /*TXQ_SDQ*/                                                                                                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_CREDIT_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_ENQ_E),                        \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_DEQ_E),                        \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_QCN_E),                        \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_CREDIT_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_ENQ_E),                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_DEQ_E),                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_QCN_E),                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_PFC_E),                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_PORT_OUTOFRANGE_E),                     \
\
    /*TXQ_PDS*/                                                                                                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_STATE_ORDER_FIFO_FULL_E),                \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_DATA_ORDER_FIFO_FULL_E),                 \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_ON_THE_AIR_CNTR_FULL_INT_E),                \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_E),              \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_SAME_WR_ADDR_INT_E),                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_PDS_INTERRUPT_DEBUG_CAUSE_TOTAL_PDS_CNTR_OVRRUN_INT_E),                    \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_PDS_INTERRUPT_DEBUG_CAUSE_PDS_CACHE_CNTR_OVRRUN_INT_E),                    \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_E),                 \
\
    /*GPC_CELL_READ*/                                                                                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INTERRUPT_CAUSE_INTERFACE_0_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_E/*bit1*/ ),\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E),              \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E),               \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INTERRUPT_CAUSE_CAM_MULTIPLE_MATCHES_E),                             \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E),               \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_CELL_READ_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_OVERFLOW_E),                \
\
    /*NPM_MC*/                                                                                                          \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_0_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_1_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_2_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_3_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_4_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_0_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_1_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_2_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_3_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_4_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_0_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_1_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_2_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_3_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_4_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_0_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_1_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_2_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_3_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_4_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_0_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_1_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_2_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_3_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_0_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_1_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_2_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_3_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_0_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_1_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_2_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_3_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(NPM_MC_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_0_E),                      \
\
    /*SMB_MC*/                                                                                                          \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E),             \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_AGE_ERR_E),                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_AGE_ERR_E),                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_AGE_ERR_E),                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_AGE_ERR_E),                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_AGE_ERR_E),                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_AGE_ERR_E),                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_AGE_ERR_E),                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_AGE_ERR_E),                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_RBW_ERR_E),                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_RBW_ERR_E),                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_RBW_ERR_E),                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_RBW_ERR_E),                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_RBW_ERR_E),                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_RBW_ERR_E),                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_RBW_ERR_E),                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_RBW_ERR_E),                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_0_REFCNT_SER_ERROR_E),                          \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_1_REFCNT_SER_ERROR_E),                          \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_2_REFCNT_SER_ERROR_E),                          \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_3_REFCNT_SER_ERROR_E),                          \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_4_REFCNT_SER_ERROR_E),                          \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_5_REFCNT_SER_ERROR_E),                          \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_6_REFCNT_SER_ERROR_E),                          \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_7_REFCNT_SER_ERROR_E),                          \
\
    /*SMB_WRITE_ARBITER*/                                                                                               \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_WRITE_INTERRUPT_MISC_CAUSE_INTERRUPT_MC_CA_FIFO_OVERFLOW_E),                   \
\
    /*GPC_PACKET_WRITE[0x93800200]*/                                                                                                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INTERRUPT_CAUSE_INT_CAUSE_FIFO_0_E),                                       \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INTERRUPT_CAUSE_INT_CAUSE_FIFO_1_E),                                       \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INTERRUPT_CAUSE_INT_CAUSE_FIFO_2_E),                                       \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INTERRUPT_CAUSE_INT_CAUSE_FIFO_3_E),                                       \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
\
    /*GPC_PACKET_READ[0x93001300]*/                                                                                                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_0_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_0_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_0_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_0_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_1_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_1_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_1_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_1_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_2_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_2_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_2_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_2_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_3_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_3_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_3_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_3_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_4_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_4_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_4_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_4_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_5_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_5_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_5_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_5_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_6_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_6_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_6_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_6_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_7_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_7_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_7_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_7_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_8_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_8_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_8_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_8_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_9_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_9_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_9_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_9_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_10_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_10_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_10_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_10_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_11_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_11_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_11_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_11_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_12_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_12_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_12_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_12_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_13_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_13_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_13_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_13_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_14_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_14_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_14_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_14_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_15_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_15_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_15_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_15_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_16_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_16_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_16_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_16_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_17_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_17_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_17_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_17_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_18_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_18_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_18_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_18_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_19_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_19_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_19_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_19_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_20_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_20_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_20_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_20_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_21_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_21_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_21_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_21_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_22_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_22_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_22_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_22_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_23_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_23_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_23_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_23_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_24_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_24_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_24_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_24_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_25_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_25_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_25_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_25_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_26_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_26_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_26_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_26_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_27_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_27_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_27_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_27_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_28_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_28_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_28_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_28_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_29_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_29_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_29_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_29_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_30_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_30_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_30_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_30_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_31_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_31_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_31_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_31_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_32_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_32_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_32_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_32_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_33_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_33_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_33_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_33_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_34_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_34_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_34_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_34_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_35_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_35_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_35_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_35_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_36_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_36_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_36_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_36_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_37_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_37_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_37_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_37_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_38_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_38_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_38_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_38_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_39_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_39_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_39_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_39_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_40_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_40_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_40_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_40_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_41_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_41_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_41_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_41_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_42_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_42_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_42_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_42_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_43_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_43_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_43_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_43_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_44_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_44_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_44_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_44_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_45_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_45_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_45_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_45_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_46_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_46_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_46_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_46_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_47_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_47_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_47_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_47_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_48_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_48_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_48_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_48_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_49_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_49_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_49_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_49_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_50_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_50_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_50_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_50_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_51_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_51_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_51_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_51_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_52_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_52_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_52_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_52_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_53_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_53_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_53_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_53_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_54_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_54_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_54_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_54_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_55_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_55_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_55_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_55_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_56_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_56_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_56_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_CHANNEL_INDEX_56_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                               \
\
    /*GPC_PACKET_READ_MISC[0x93001508]*/                                                                                                    \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E),                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_OVERFLOW_E),                               \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_UNDERFLOW_E),                              \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_MISC_CAUSE_CAM_MULTIPLE_MATCHES_E),                                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_MISC_CAUSE_DESCRIPTOR_HEADER_SIZE_ERROR_E),                             \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_MISC_CAUSE_DESCRIPTOR_TAIL_SIZE_ERROR_E),                               \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_MISC_CAUSE_PACKET_COUNT_PACKET_SIZE_ERROR_E),                           \
\
    /*GPC_PACKET_READ_PACKET_COUNT[0x93001500]*/                                                                                          \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E),     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CAUSE_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_PACKET_COUNT_E),   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E),     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CAUSE_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_PACKET_COUNT_E),   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E),     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CAUSE_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_PACKET_COUNT_E),   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E),     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CAUSE_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_PACKET_COUNT_E),   \
\
    /*GPC_PACKET_READ_PACKET_CREDIT_COUNTER[0x93001510]*/                                                                                 \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_HT_OVERFLOW_E),           \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_HT_OVERFLOW_E),   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_HT_OVERFLOW_E),     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E),                  \
\
    /*GPC_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER[0x93001518]*/                                                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),        \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E),          \
\
    /*DFX_SERVER_INTERRUPT_CAUSE[0x944F8108]*/                                                                                                                    \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_BIST_FAIL_E),                                                    \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_EXTERNAL_TEMPERATURE_THRESHOLD_E),                               \
\
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(BMA_MC_CNT_COUNTER_FLOW_E                                 ),  \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(BMA_MC_CNT_PARITY_ERROR_E                                 ),  \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(BMA_SHIFTER_LATENCY_FIFO_OVERFLOW_E                       ),  \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(BMA_SHIFTER_LATENCY_FIFO_UNDERFLOW_E                      ),  \
\
    /*RXDMA_INTERRUPT2_CAUSE[0x91C01C90]*/                                                                                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_PB_TAIL_ID_MEM_SER_INT_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_0_MEM_SER_INT_E),                 \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_1_MEM_SER_INT_E),                 \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_REPLY_TAIL_PB_ID_MEM_SER_INT_E),                \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_0_PB_RD_DATA_ERR_INT_E),               \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_1_PB_RD_DATA_ERR_INT_E),               \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_0_PB_RD_DATA_ERR_INT_E),                \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_1_PB_RD_DATA_ERR_INT_E),                \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_0_RD_SER_INT_E),               \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_1_RD_SER_INT_E),               \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_ENQUEUE_REQUESTS_DESCRIPTOR_FIFO_RD_SER_INT_E), \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_HEAD_PACKET_COUNT_DATA_FIFO_MEM_RD_SER_INT_E),  \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E),            \
\
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_DATA_INTEGRITY_ERROR_E,                                                                                        \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HA_HA_INTERRUPT_CAUSE_ECC_SINGLE_ERROR_E),                                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HA_HA_INTERRUPT_CAUSE_ECC_DOUBLE_ERROR_E),                                     \
                                                                                                                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TCAM_TCAM_INTERRUPT_CAUSE_TCAM_ARRAY_PARITY_ERROR_E),                          \
\
    /* DFX Interrupts. AC5X has 5 DFX Pipes. Need to connect to all. */                  \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E), \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E), \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E), \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_3_INTERRUPT_SUM_E), \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_4_INTERRUPT_SUM_E), \
\
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E,                                                                               \
        SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_KEEP_ALIVE_AGING_E),                             \
        SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_KEEP_ALIVE_AGING_E),                             \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E,                                                                              \
        SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_EXCESS_KEEPALIVE_E),                             \
        SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_EXCESS_KEEPALIVE_E),                             \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E,                                                                             \
        SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_INVALID_KEEPALIVE_HASH_E),                       \
        SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_INVALID_KEEPALIVE_HASH_E),                       \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E,                                                                                    \
        SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_RDI_STATUS_E),                                   \
        SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_RDI_STATUS_E),                                   \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E,                                                                                     \
        SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_TX_PERIOD_E),                                    \
        SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_TX_PERIOD_E),                                    \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E,                                                                                     \
        SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_MEG_LEVEL_EXCEPTION_E),                          \
        SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_MEG_LEVEL_EXCEPTION_E),                          \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E,                                                                              \
        SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_SOURCE_INTERFACE_EXCEPTION_E),                   \
        SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_SOURCE_INTERFACE_EXCEPTION_E),                   \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E,                                                                                     \
        SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_ILLEGAL_OAM_ENTRY_INDEX_E),                      \
        SET_EQUAL_EVENT_WITH_POSTFIX_MAC(EOAM_OAM_UNIT_INTERRUPT_CAUSE_ILLEGAL_OAM_ENTRY_INDEX_E),                      \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PHA_E,                                                                                                         \
        SET_PHA_EVENTS_MAC,                                                                                             \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
\
CPSS_PP_TQ_PORT_HR_CROSSED_THRESHOLD_E,                                                                                 \
    PHOENIX_SET_HR_TRSH_EVENT_ALL_PORTS_MAC,                                                                            \
MARK_END_OF_UNI_EV_CNS,                                                                                                 \
\
CPSS_PP_MACSEC_SA_EXPIRED_E,\
    PHOENIX_SET_EIP_164_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SA_EXPIRED_E),               \
MARK_END_OF_UNI_EV_CNS,                                                                                                 \
                                                                                                                        \
CPSS_PP_MACSEC_SA_PN_FULL_E,                                                                                            \
    PHOENIX_SET_EIP_164_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SA_PN_THR_E),              \
MARK_END_OF_UNI_EV_CNS,                                                                                                 \
                                                                                                                        \
CPSS_PP_MACSEC_STATISTICS_SUMMARY_E,                                                                                    \
    PHOENIX_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_SA_THR_E,       0),  \
    PHOENIX_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_SECY_THR_E,     1),  \
    PHOENIX_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_IFC0_THR_E,     2),  \
    PHOENIX_SET_EIP_164_EVENT_SOURCE_PER_INGRESS_DIRECTION_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_IFC1_THR_E,  3),  \
    PHOENIX_SET_EIP_164_EVENT_SOURCE_PER_INGRESS_DIRECTION_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_RXCAM_THR_E, 4),  \
                                                                                                                         \
    PHOENIX_SET_EIP_163_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E,     5),  \
    PHOENIX_SET_EIP_163_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_CHANNEL_THR_E,  6),  \
MARK_END_OF_UNI_EV_CNS,                                                                                                  \
                                                                                                                         \
CPSS_PP_MACSEC_TRANSFORM_ENGINE_ERR_E,                                                                                   \
    PHOENIX_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(0),       \
    PHOENIX_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(1),       \
    PHOENIX_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(2),       \
    PHOENIX_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(3),       \
    PHOENIX_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(4),       \
    PHOENIX_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(5),       \
    PHOENIX_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(6),       \
    PHOENIX_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(7),       \
    PHOENIX_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(8),       \
    PHOENIX_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(9),       \
    PHOENIX_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(10),      \
    PHOENIX_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(11),      \
    PHOENIX_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(12),      \
    PHOENIX_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(13),      \
    PHOENIX_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(14),      \
MARK_END_OF_UNI_EV_CNS,                                                                                                            \
                                                                                                                                   \
CPSS_PP_MACSEC_EGRESS_SEQ_NUM_ROLLOVER_E,                                                                                          \
    PHOENIX_SET_EIP_66_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SEQ_NUM_ROLLOVER_E),                   \
MARK_END_OF_UNI_EV_CNS




/* Interrupt cause to unified event map for AC5X
 This Table is for unified event with extended data

  Array structure:
  1. The first element of the array and the elements after MARK_END_OF_UNI_EV_CNS are unified event types.
  2. Elements after unified event type are pairs of interrupt cause and event extended data,
     until MARK_END_OF_UNI_EV_CNS.
*/
static const GT_U32 AC5X_UniEvMapTableWithExtData[] = {
    CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
        PORTS_LINK_STATUS_CHANGE_EVENTS,
        AC5X_CPU_PORT_LINK_STATUS_CHANGE_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
        PORTS_LINK_OK_CHANGE_EVENTS,
        AC5X_CPU_PORT_LINK_OK_CHANGED_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_AN_HCD_FOUND_E,
        AC5X_PORT_AN_HCD_RESOLUTION_DONE_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_AN_RESTART_E,
        AC5X_PORT_AN_RESTART_DONE_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_AN_PARALLEL_DETECT_E,
        AC5X_PORT_INT_PM_PCS_LINK_TIMER_OUT_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    PHOENIX_COMMON_EVENTS,
};


#define ac5xUniEvMapTable_MG_n(_mgIndex)  \
    {CPSS_PP_EB_AUQ_PENDING_E,              PRV_CPSS_AC5X_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_AU_QUEUE_PENDING_E},                  \
    {CPSS_PP_EB_AUQ_FULL_E,                 PRV_CPSS_AC5X_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_AU_QUEUE_FULL_E},                     \
    {CPSS_PP_EB_AUQ_OVER_E,                 PRV_CPSS_AC5X_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_AU_QUEUE_OVERRUN_E},                  \
    {CPSS_PP_EB_AUQ_ALMOST_FULL_E,          PRV_CPSS_AC5X_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_AU_QUEUE_ALMOST_FULL_E},              \
    {CPSS_PP_EB_FUQ_PENDING_E,              PRV_CPSS_AC5X_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_FU_QUEUE_PENDING_E},                  \
    {CPSS_PP_EB_FUQ_FULL_E,                 PRV_CPSS_AC5X_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_FU_QUEUE_FULL_E},                     \
    {CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,    PRV_CPSS_AC5X_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_MG_READ_DMA_DONE_E},                  \
    {CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E,      PRV_CPSS_AC5X_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_PC_IE_ADDRESS_UNMAPPED_E},            \
    {CPSS_PP_MISC_TWSI_TIME_OUT_E,          PRV_CPSS_AC5X_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_I2C_TIME_OUT_INTERRUPT_E},            \
    {CPSS_PP_MISC_TWSI_STATUS_E,            PRV_CPSS_AC5X_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_I2C_STATUS_INTERRUPT_E},              \
    {CPSS_PP_MISC_ILLEGAL_ADDR_E,           PRV_CPSS_AC5X_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_INTERRUPT_E},         \
    {CPSS_PP_PORT_802_3_AP_E,               PRV_CPSS_AC5X_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_Z80_INTERRUPT_E},                     \
    {CPSS_SRVCPU_IPC_E,                     PRV_CPSS_AC5X_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_CM3_TO_HOST_DOORBELL_INTERRUPT_E}     \

/* Interrupt cause to unified event map for AC5X
 This Table is for unified event without extended data
*/
static const GT_U32 ac5xUniEvMapTable[][2] =
{
    {CPSS_PP_EB_SECURITY_BREACH_UPDATE_E,    PRV_CPSS_AC5X_L2I_BRIDGE_INTERRUPT_CAUSE_UPDATE_SECURITY_BREACH_REGISTER_INT_E         },
    {CPSS_PP_MAC_NUM_OF_HOP_EXP_E,           PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NUM_OF_HOP_EX_P_E                       },
    {CPSS_PP_MAC_NA_LEARNED_E,               PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NA_LEARNT_E                             },
    {CPSS_PP_MAC_NA_NOT_LEARNED_E,           PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NA_NOT_LEARNT_E                         },
    {CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E,    PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AGE_VIA_TRIGGER_ENDED_E                 },
    {CPSS_PP_MAC_UPDATE_FROM_CPU_DONE_E,     PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AU_PROC_COMPLETED_INT_E                 },
    {CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E,     PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AU_MSG_TOCPU_READY_E                    },
    {CPSS_PP_MAC_NA_SELF_LEARNED_E,          PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NA_SELF_LEARNED_E                       },
    {CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E,      PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_N_AFROM_CPU_LEARNED_E                   },
    {CPSS_PP_MAC_NA_FROM_CPU_DROPPED_E,      PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_N_AFROM_CPU_DROPPED_E                   },
    {CPSS_PP_MAC_AGED_OUT_E,                 PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AGED_OUT_E                              },
    {CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E,      PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AU_FIFO_TO_CPU_IS_FULL_E                },
    {CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E,      PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_ADDRESS_OUT_OF_RANGE_E                  },
    {CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E, PRV_CPSS_AC5X_TTI_TTI_ENGINE_INTERRUPT_CAUSE_CPU_ADDRESS_OUT_OF_RANGE_E                },
    {CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E,     PRV_CPSS_AC5X_PCL_PCL_UNIT_INTERRUPT_CAUSE_MG_ADDR_OUTOF_RANGE_E                       },
    {CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E, PRV_CPSS_AC5X_FDB_FDB_INTERRUPT_CAUSE_REGISTER_BLC_OVERFLOW_E                      },
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,         PRV_CPSS_AC5X_TXQ_SCN_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E                },
    {CPSS_PP_EM_AUTO_LEARN_INDIRECT_ADDRESS_OUT_OF_RANGE_EVENT_E,       PRV_CPSS_AC5X_EM_EM_INTERRUPT_CAUSE_REGISTER_INDIRECT_ADDRESS_OUT_OF_RANGE_E        },
    {CPSS_PP_EM_AUTO_LEARN_FLOW_ID_ALLOCATION_FAILED_EVENT_E,           PRV_CPSS_AC5X_EM_EM_INTERRUPT_CAUSE_REGISTER_FLOW_ID_ALLOCATION_FAILED_E            },
    {CPSS_PP_EM_AUTO_LEARN_FLOW_ID_RANGE_FULL_EVENT_E,                  PRV_CPSS_AC5X_EM_EM_INTERRUPT_CAUSE_REGISTER_FLOW_ID_RANGE_FULL_E                   },
    {CPSS_PP_EM_AUTO_LEARN_FLOW_ID_THRESHOLD_CROSSED_EVENT_E,           PRV_CPSS_AC5X_EM_EM_INTERRUPT_CAUSE_REGISTER_FLOW_ID_THRESHOLD_CROSSED_E            },
    {CPSS_PP_EM_AUTO_LEARN_FLOW_ID_EMPTY_EVENT_E,                       PRV_CPSS_AC5X_EM_EM_INTERRUPT_CAUSE_REGISTER_FLOW_ID_EMPTY_E                        },
    {CPSS_PP_EM_AUTO_LEARN_COLLISION_EVENT_E,                           PRV_CPSS_AC5X_EM_EM_INTERRUPT_CAUSE_REGISTER_AUTO_LEARNING_COLLISION_E              },
    {CPSS_PP_EM_AUTO_LEARN_ILLEGAL_ADDRESS_ACCESS_EVENT_E,              PRV_CPSS_AC5X_EM_EM_INTERRUPT_CAUSE_REGISTER_ILLEGAL_ADDRESS_ACCESS_E               },
    {CPSS_PP_EM_AUTO_LEARN_ILLEGAL_TABLE_OVERLAP_EVENT_E,               PRV_CPSS_AC5X_EM_EM_INTERRUPT_CAUSE_REGISTER_ILLEGAL_TABLE_OVERLAP_E                },
    {CPSS_PP_EM_AUTO_LEARN_CHANNEL_0_SHM_ANSWER_FIFO_OVERFLOW_EVENT_E,  PRV_CPSS_AC5X_EM_EM_INTERRUPT_CAUSE_REGISTER_CHANNEL_0_SHM_ANSWER_FIFO_OVERFLOW_E   },
    {CPSS_PP_EM_AUTO_LEARN_CHANNEL_1_SHM_ANSWER_FIFO_OVERFLOW_EVENT_E,  PRV_CPSS_AC5X_EM_EM_INTERRUPT_CAUSE_REGISTER_CHANNEL_1_SHM_ANSWER_FIFO_OVERFLOW_E   },
    ac5xUniEvMapTable_MG_n(0)                                                                                                        ,
    ac5xUniEvMapTable_MG_n(1)                                                                                                        ,

};

/* Interrupt cause to unified event map for AC5X without extended data size */
static const GT_U32 ac5xUniEvMapTableSize = (sizeof(ac5xUniEvMapTable)/(sizeof(GT_U32)*2));

/**
* @internal setAc5xDedicatedEventsConvertInfo function
* @endinternal
*
* @brief   set info needed by chIntCauseToUniEvConvertDedicatedTables(...) for the
*          AC5X devices.
*/
static void setAc5xDedicatedEventsConvertInfo(IN GT_U8   devNum)
{
    /* Interrupt cause to unified event map for AC5X with extended data size */
    GT_U32 ac5xUniEvMapTableWithExtDataSize;

    ac5xUniEvMapTableWithExtDataSize = (sizeof(AC5X_UniEvMapTableWithExtData)/(sizeof(GT_U32)));
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableWithExtDataArr   = AC5X_UniEvMapTableWithExtData;

    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableWithExtDataSize  = ac5xUniEvMapTableWithExtDataSize;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableArr              = ac5xUniEvMapTable;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableSize             = ac5xUniEvMapTableSize;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.numReservedPorts              = 0;

    return;
}

static void phoenixUpdateIntScanTreeInfo(
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
        unitId       = UNIT_ID_GET ((PRV_CPSS_DXCH_UNIT_ENT)intrScanArr[ii].gppId);

        didError = GT_FALSE;

        baseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,&didError);

        if(didError == GT_TRUE)
        {

            /*support MG units */
            intrScanArr[ii].maskRegAddr  = CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS;

            /* NOTE: this macro not 'return' */
            CPSS_LOG_ERROR_MAC("not supported unit [%d] in AC5X",
                unitId);
        }
        else
        {
            if(baseAddr > 0 && intrScanArr[ii].causeRegAddr > baseAddr)
            {
                /* ERROR : all addresses MUST be 0 based !!! */
                /* ERROR : all addresses MUST be 0 based !!! */
                /* ERROR : all addresses MUST be 0 based !!! */

               /* because we can't mask the address !!! */
               cpssOsPrintf("ERROR : AC5X interrupts tree : NODE hold causeRegAddr [0x%8.8x] but must be relative to the start of the unit [0x%8.8x] \n",
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
* @internal prvCpssDrvPpIntDefDxChPhoenixMacSecRegIdInit function
* @endinternal
*
* @brief   Init macsec register ids array.
*
* */
static GT_STATUS prvCpssDrvPpIntDefDxChPhoenixMacSecRegIdInit
(
    GT_VOID
)
{
    GT_U32 iter;

    /* check that Global DB size is in synch with required one */
    if (PRV_CPSS_GLOBAL_DB_AC5X_MACSEC_REGID_ARRAY_SIZE_CNS != PRV_CPSS_DXCH_AC5X_MACSEC_REGID_ARRAY_SIZE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    for(iter = 0; iter < PRV_CPSS_DXCH_AC5X_MACSEC_REGID_ARRAY_SIZE_CNS - 1; iter++)
    {
        PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(macSecRegIdNotAccessibleBeforeMacSecInitAc5x[iter],
                                                                               PRV_CPSS_DXCH_AC5X_MACSEC_REGID_FIRST_CNS + iter);
    }
    /* end identifier for the array */
    PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(macSecRegIdNotAccessibleBeforeMacSecInitAc5x[iter],
                                                                           PRV_CPSS_MACSEC_REGID_LAST_CNS);
    return GT_OK;
}

/**
* @internal prvCpssDrvPpIntDefDxChPhoenixInit function
* @endinternal
*
* @brief   Interrupts initialization for the AC5X devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefDxChPhoenixInit
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
        PRV_CPSS_DRV_FAMILY_DXCH_AC5X_E,
        NUM_ELEMENTS_IN_ARR_MAC(phoenixIntrScanArr),
        phoenixIntrScanArr,
        PRV_CPSS_AC5X_LAST_INT_E/32,
        NULL, NULL, NULL
    };

    ppRevision = ppRevision;

    /*set info needed by chIntCauseToUniEvConvertDedicatedTables(...) */
    setAc5xDedicatedEventsConvertInfo(devNum);

    /* init macsec register ids */
    rc = prvCpssDrvPpIntDefDxChPhoenixMacSecRegIdInit();
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    if(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(phoenixInitDone) == GT_TRUE)
    {
        return GT_OK;
    }

    rc = prvCpssDrvDxExMxInterruptsMemoryInit(&devFamilyInterrupstInfo, 1);
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }
    (void)prvCpssDxChUnitBaseTableInit(CPSS_PP_FAMILY_DXCH_AC5X_E);/*needed for calling prvCpssDxChHwUnitBaseAddrGet(...) */

    /* 1. Align base addresses for mask registers */
    phoenixUpdateIntScanTreeInfo(devNum,
        devFamilyInterrupstInfo.intrScanOutArr ,
        devFamilyInterrupstInfo.numScanElements);

    /*
       1. fill the array of mask registers addresses
       2. fill the array of default values for the mask registers
       3. update the bits of nonSumBitMask in the scan tree
    */
    rc = prvCpssDrvExMxDxHwPpMaskRegInfoGet(
            CPSS_PP_FAMILY_DXCH_AC5X_E       ,
            devFamilyInterrupstInfo.numScanElements          ,
            devFamilyInterrupstInfo.intrScanOutArr           ,
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
        PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(macSecRegIdNotAccessibleBeforeMacSecInitAc5x);

    PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(phoenixInitDone, GT_TRUE);

    return GT_OK;
}


#ifdef DUMP_DEFAULT_INFO

/**
* @internal prvCpssDrvPpIntDefPhoenixPrint function
* @endinternal
*
* @brief   print the interrupts arrays info of AC5X devices
*/
void  prvCpssDrvPpIntDefPhoenixPrint(
    void
)
{
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC *  interruptsScanArray;

    cpssOsPrintf("AC5X - start : \n");
    devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_AC5X_E]);

    interruptsScanArray = devInterruptInfoPtr->interruptsScanArray;
    /* port group 0 */
    prvCpssDrvPpIntDefPrint(devInterruptInfoPtr->numOfScanElements,
                            interruptsScanArray,
                            AC5X_NUM_MASK_REGISTERS_CNS,
                            devInterruptInfoPtr->maskRegistersAddressesArray,
                            devInterruptInfoPtr->maskRegistersDefaultValuesArray);
    cpssOsPrintf("AC5X - End : \n");

}
#endif/*DUMP_DEFAULT_INFO*/

/**
* @internal prvCpssDrvPpIntDefPhoenixPrint_regInfoByInterruptIndex function
* @endinternal
*
* @brief   print for AC5X devices the register info according to value in
*         PRV_CPSS_AC5X_LAST_INT_E (assuming that this register is part of
*         the interrupts tree)
*/
void  prvCpssDrvPpIntDefPhoenixPrint_regInfoByInterruptIndex(
    IN PRV_CPSS_AC5X_INT_CAUSE_ENT   interruptId
)
{
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
    GT_CHAR*    deviceNamePtr = "AC5X interrupts";
    GT_U32      numMaskRegisters = PRV_CPSS_AC5X_LAST_INT_E/32;

    cpssOsPrintf("AC5X - start regInfoByInterruptIndex : \n");

    if(interruptId >= PRV_CPSS_AC5X_LAST_INT_E)
    {
        cpssOsPrintf("interruptId[%d] >= maximum(%d) \n" ,
            interruptId, PRV_CPSS_AC5X_LAST_INT_E);
    }
    else
    {
        if(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(phoenixInitDone) == GT_FALSE)
        {
            cpssOsPrintf("AC5X - ERROR : the 'IntrScanArr' was not initialized with 'base addresses' yet End \n");
            return;
        }

        if((GT_U32)interruptId >= (numMaskRegisters << 5))
        {
            cpssOsPrintf("[%s] skipped : interruptId[%d] >= maximum(%d) \n" ,
                deviceNamePtr , interruptId, (numMaskRegisters << 5));
        }

        devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_AC5X_E]);

        cpssOsPrintf("start [%s]: \n", deviceNamePtr);
        prvCpssDrvPpIntDefPrint_regInfoByInterruptIndex(devInterruptInfoPtr->numOfScanElements, devInterruptInfoPtr->interruptsScanArray, interruptId, GT_FALSE);
        cpssOsPrintf("ended [%s] \n\n", deviceNamePtr);
    }

    cpssOsPrintf("AC5X - End regInfoByInterruptIndex : \n");
}

