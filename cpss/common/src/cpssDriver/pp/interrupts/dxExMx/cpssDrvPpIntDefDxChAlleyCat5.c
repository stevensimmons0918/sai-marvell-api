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
* @file cpssDrvPpIntDefDxChAlleyCat5.c
*
* @brief This file includes the definition and initialization of the interrupts
* init. parameters tables. -- DxCh alleyCat5 devices
*
* @version   1
********************************************************************************
*/

#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterruptsInit.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsAlleyCat5.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* unit : PRV_CPSS_DXCH_UNIT_MG_E */
#define MG_UNIT(_mgIndex) \
    (_mgIndex ? (PRV_CPSS_DXCH_UNIT_MG_0_1_E + (_mgIndex-1)) : PRV_CPSS_DXCH_UNIT_MG_E)

#define CNM_MG_ADDRESS_MAC(_mgIndex, base)                                                                 \
    PRV_CPSS_AC5_MG0_BASE_ADDRESS_CNS + ((_mgIndex) * 0x100000) + (base)

#define MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, _mgIndex, numChild)                                     \
    /* Global Interrupt Cause */                                                                        \
    {bitIndexInCaller, GT_FALSE,  MG_UNIT(_mgIndex), NULL,                                              \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x00000030),                                                                    \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x00000034),                                                                    \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5_MG_##_mgIndex##_GLOBAL_INTERRUPTS_SUMMARY_CAUSE_PC_IE_INTERRUPT_SUMMARY_E,         \
        PRV_CPSS_AC5_MG_##_mgIndex##_GLOBAL_INTERRUPTS_SUMMARY_CAUSE_MG1_INTERNAL_INTERRUPT_SUMMARY_E,  \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, numChild, NULL, NULL}

#define cnm_grp_0_0_IntsSum_SUB_TREE_MAC(bit, _mgIndex, numChild)                                       \
    /* FuncUnitsIntsSum Interrupt Cause */                                                              \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x000003f8),                                                                    \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x000003fc),                                                                    \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5_MG_##_mgIndex##_FUNCTIONAL_INTERRUPTS_SUMMARY_CAUSE_L2I_INT_SUM_E,                 \
        PRV_CPSS_AC5_MG_##_mgIndex##_FUNCTIONAL_INTERRUPTS_SUMMARY_CAUSE_TCC_UPPER_INT_SUM_E,           \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, numChild, NULL, NULL},                                           \
        /* Bridge Interrupt Cause */                                                                    \
        {1, GT_FALSE, 0, NULL, 0x01040130, 0x01040134,                                                  \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5_BRIDGE_ADDRESS_OUT_OF_RANGE_E,                                                 \
            PRV_CPSS_AC5_BRIDGE_PCR_PARITY_ERROR_E,                                                     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                     \
                                                                                                        \
        {2, GT_FALSE, 0, NULL, 0x14000058, 0x1400005C,                                                  \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5_PRE_EGRESS_SUMMARY_INGRESS_STC_SUM_E,                                          \
            PRV_CPSS_AC5_PRE_EGRESS_SUMMARY_STC_RATE_LIMITERS_SUM_E,                                    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0x0, 2, NULL, NULL},                                            \
                                                                                                        \
            /* Ingress STC Interrupt Cause */                                                           \
            {1, GT_FALSE, 0, NULL, 0x14000020, 0x14000024,                                              \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_AC5_INGRESS_STC_PORT_0_INGRESS_SAMPLE_LOADED_E,                                \
                PRV_CPSS_AC5_INGRESS_STC_PORT_27_INGRESS_SAMPLE_LOADED_E,                               \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
            /* CPU Code Rate Limiters Interrupt Cause */                                                \
            {2, GT_FALSE, 0, NULL, 0x14000060, 0x14000064,                                              \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_AC5_CPU_CODE_RATE_LIMITERS_STC_RATE_LIMITER_1_PKT_DROPED_E,                    \
                PRV_CPSS_AC5_CPU_CODE_RATE_LIMITERS_STC_RATE_LIMITER_31_PKT_DROPED_E,                   \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* CNC1 Interrupt Cause Register */                                                             \
        {4, GT_FALSE, 0, NULL, 0x1F000100, 0x1F000104,                                                  \
         prvCpssDrvHwPpPortGroupIsrRead,                                                                \
         prvCpssDrvHwPpPortGroupIsrWrite,                                                               \
         PRV_CPSS_AC5_CNC1_BLOCK0_WRAPAROUND_E,                                                         \
         PRV_CPSS_AC5_CNC1_DUMP_FINISHED_E,                                                             \
         FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                        \
                                                                                                        \
         XLG_MAC_PORT_ODD_INTERRUPT_SUM_SUBTREE_MAC(5, 27),                                             \
         XLG_MAC_PORT_ODD_INTERRUPT_SUM_SUBTREE_MAC(6, 26),                                             \
         XLG_MAC_PORT_ODD_INTERRUPT_SUM_SUBTREE_MAC(7, 25),                                             \
         XLG_MAC_PORT_ODD_INTERRUPT_SUM_SUBTREE_MAC(8, 24),                                             \
                                                                                                        \
        /* TTI Engine Interrupt Cause */                                                                \
        {9, GT_FALSE, 0, NULL, 0x16000004, 0x16000008,                                                  \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5_TTI_CPU_ADDRESS_OUT_OF_RANGE_E,                                                \
            PRV_CPSS_AC5_TTI_ACCESS_DATA_ERROR_E,                                                       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                     \
                                                                                                        \
        /* IPCL TCC Interrupt Cause */                                                                  \
        {10, GT_FALSE, 0, NULL, 0x1A0001A4, 0x1A0001A8,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5_TCC_LOWER_TCAM_ERROR_DETECTED_E,                                               \
            PRV_CPSS_AC5_TCC_LOWER_TCAM_BIST_FAILED_E,                                                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                     \
                                                                                                        \
        /* Policy Engine Interrupt Cause */                                                             \
        {11, GT_FALSE, 0, NULL, 0x15000004, 0x15000008,                                                 \
             prvCpssDrvHwPpPortGroupIsrRead,                                                            \
             prvCpssDrvHwPpPortGroupIsrWrite,                                                           \
             PRV_CPSS_AC5_POLICY_ENGINE_MG_ADDR_OUT_OF_RANGE_E,                                         \
             PRV_CPSS_AC5_POLICY_ENGINE_PCL_CONFIG_TABLE_1_PARITY_ERROR_E,                              \
             FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                    \
                                                                                                        \
        /* Buffer Memory Main Interrupt Cause Register */                                               \
        {12, GT_FALSE, 0, NULL, 0x0E000090, 0x0E000094,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5_BM_MAIN_REG0_SUM_E,                                                            \
            PRV_CPSS_AC5_BM_MAIN_REG1_SUM_E,                                                            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0x0, 2, NULL, NULL},                                            \
                                                                                                        \
            /* Buffer Memory Interrupt Cause Register0 */                                               \
            {1, GT_FALSE, 0, NULL, 0x0E000024, 0x0E000028,                                              \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_AC5_BM_0_PORT0_TX_FIFO_OVERRUN_E,                                              \
                PRV_CPSS_AC5_BM_0_BURST_FIFO_OVERRUN_E,                                                 \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
            /* Buffer Memory Interrupt Cause Register1 */                                               \
            {2, GT_FALSE, 0, NULL, 0x0E000098, 0x0E00009C,                                              \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_AC5_BM_1_BANK0_ONE_ECC_ERROR_E,                                                \
                PRV_CPSS_AC5_BM_1_BANK1_TWO_OR_MORE_ECC_ERRORS_E,                                       \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        PLR_INTERRUPT_LEAF_MAC(14, 0x06000100, 0x06000104, EPLR),                                       \
                                                                                                        \
        /* CNC Interrupt Cause Register */                                                              \
        {22, GT_FALSE, 0, NULL, 0x10000100, 0x10000104,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5_CNC_BLOCK0_WRAPAROUND_E,                                                       \
            PRV_CPSS_AC5_CNC_DUMP_FINISHED_E,                                                           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                     \
                                                                                                        \
        /* Buffer Management Interrupt Cause1  */                                                       \
        {23, GT_FALSE, 0, NULL, 0x05000048, 0x0500004C,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5_BM_BUFF_LIMIT_REACHED_PORT0_E,                                                 \
            PRV_CPSS_AC5_BM_CPU_PORT_BUFF_LIMIT_REACHED_E,                                              \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                     \
                                                                                                        \
        /* Buffer Management Interrupt Cause0  */                                                       \
        {24, GT_FALSE, 0, NULL, 0x05000040, 0x05000044,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5_BM_EMPTY_CLEAR_E,                                                              \
            PRV_CPSS_AC5_BM_PORT_RX_FULL_E,                                                             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                     \
                                                                                                        \
        /* IPvX TCC Interrupt Cause */                                                                  \
        {31, GT_FALSE, 0, NULL, 0x1B000060, 0x1b000064,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5_TCC_UPPER_TCAM_ERROR_DETECTED_E,                                               \
            PRV_CPSS_AC5_TCC_UPPER_TCAM_BIST_FAILED_E,                                                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define cnm_grp_0_1_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                       \
    /* DataPathIntsSum Interrupt Cause */                                                               \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x000000a4),                                                       \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x000000a8),                                                       \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5_MG_##_mgIndex##_DATA_PATH_INTERRUPTS_SUMMARY_CAUSE_RECEIVER_TCC1_LOWER_INT_SUM_E,  \
        PRV_CPSS_AC5_MG_##_mgIndex##_DATA_PATH_INTERRUPTS_SUMMARY_CAUSE_SERVER_INT_SUM_E,               \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 1, NULL, NULL},                                                  \
        /* Server Interrupt Summary Cause */                                                            \
        {20, GT_FALSE, 0, NULL,                                                                         \
            0x840F8100, 0x840F8104,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5_DFX_SERVER_INTERRUPT_SUMMARY_SERVER_INTERRUPT_SUM_E,                           \
            PRV_CPSS_AC5_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_7_INTERRUPT_SUM_E,                           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define cnm_grp_0_2_IntsSum_SUB_TREE_MAC(bit, _mgIndex, numChild)                                       \
    /* PortsIntsSum Interrupt Cause */                                                                  \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x00000080),                                                                    \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x00000084),                                                                    \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5_MG_##_mgIndex##_PORTS_INTERRUPTS_SUMMARY_CAUSE_PORT_INT_SUM_1_E,                   \
        PRV_CPSS_AC5_MG_##_mgIndex##_PORTS_INTERRUPTS_SUMMARY_CAUSE_PORT_INT_SUM_31_E,                  \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, numChild, NULL, NULL}

#define cnm_grp_0_3_IntsSum_SUB_TREE_MAC(bit, _mgIndex, numChild)                                       \
    /* DFX */                                                                                           \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        0x000000ac, 0x000000b0,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5_MG_##_mgIndex##_DFX_INTERRUPTS_SUMMARY_CAUSE_DFX_INTERRUPT_SUM_1_E,                \
        PRV_CPSS_AC5_MG_##_mgIndex##_DFX_INTERRUPTS_SUMMARY_CAUSE_DFX_INTERRUPT_SUM_31_E,               \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, numChild, NULL, NULL}

#define   mg0InternalIntsSum_SUB_TREE_MAC(bit, _mgIndex, numChild)                                      \
    /* Miscellaneous */                                                                                 \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x00000038),                                                                    \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x0000003C),                                                                    \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_I2C_TIME_OUT_INTERRUPT_E,              \
        PRV_CPSS_AC5_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_SERDES_ADDRESS_OUT_OF_RANGE_VIOLATION_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, numChild, NULL, NULL},                                  \
        /* HostCpuDoorbellIntSum - single register per MG accessed from MG Internal Cause0/1/2/3 */     \
        {24, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                         \
            CNM_MG_ADDRESS_MAC(_mgIndex, 0x00000518),                                                                \
            CNM_MG_ADDRESS_MAC(_mgIndex, 0x0000051c),                                                                \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5_MG##_mgIndex##_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_0_E,                     \
            PRV_CPSS_AC5_MG##_mgIndex##_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_30_E,                    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 0, NULL, NULL}

#define cnm_grp_0_4_IntsSum_SUB_TREE_MAC(bit, _mgIndex, numChild)                                       \
    /* DFX1 */                                                                                          \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x000000b8),                                                                    \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x000000bc),                                                                    \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5_MG_##_mgIndex##_DFX_1_INTERRUPTS_SUMMARY_CAUSE_CCI_ERRIRQ_E,                       \
        PRV_CPSS_AC5_MG_##_mgIndex##_DFX_1_INTERRUPTS_SUMMARY_CAUSE_EXT_MG_INT_OUT2_E,                  \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, numChild, NULL, NULL},                                           \
        MG_LEGACY_INTERRUPTS_MAC(29)

#define cnm_grp_0_5_IntsSum_SUB_TREE_MAC(bit, _mgIndex, numChild)                                       \
    /* FuncUnits1IntsSum */                                                                             \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x000003f4),                                                       \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x000003f0),                                                       \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5_MG_##_mgIndex##_FUNCTIONAL_1_INTERRUPTS_SUMMARY_CAUSE_MG1_CNM_INT0_E,              \
        PRV_CPSS_AC5_MG_##_mgIndex##_FUNCTIONAL_1_INTERRUPTS_SUMMARY_CAUSE_CCSS_GWD_WS1_INT_E,          \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, numChild, NULL, NULL},                                           \
        {4, GT_FALSE, MG_UNIT(1), NULL,                                                                 \
            CNM_MG_ADDRESS_MAC(1, 0x0000030),                                                           \
            CNM_MG_ADDRESS_MAC(1, 0x0000034),                                                           \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5_MG_1_GLOBAL_INTERRUPTS_SUMMARY_CAUSE_GENERAL_INTERRUPT_SUMMARY_E,              \
            PRV_CPSS_AC5_MG_1_GLOBAL_INTERRUPTS_SUMMARY_CAUSE_MG1_INTERNAL_INTERRUPT_SUMMARY_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 2, NULL, NULL},                                     \
            {7, GT_FALSE, MG_UNIT(1), NULL,                                                             \
                CNM_MG_ADDRESS_MAC(1, 0x0000038),                                                       \
                CNM_MG_ADDRESS_MAC(1, 0x000003c),                                                       \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_AC5_MG_1_MG_INTERNAL_INTERRUPT_CAUSE_I2C_TIME_OUT_INTERRUPT_E,                 \
                PRV_CPSS_AC5_MG_1_MG_INTERNAL_INTERRUPT_CAUSE_SERDES_ADDRESS_OUT_OF_RANGE_VIOLATION_E,  \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 0, NULL, NULL},                                 \
            {18, GT_FALSE, MG_UNIT(1), NULL,                                                            \
                CNM_MG_ADDRESS_MAC(1, 0x000009c),                                                       \
                CNM_MG_ADDRESS_MAC(1, 0x00000a0),                                                       \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_AC5_MG_1_MG1_INTERNAL_INTERRUPT_CAUSE_MISCELLANEOUS_INTERRUPT_SUMMARY1_E,      \
                PRV_CPSS_AC5_MG_1_MG1_INTERNAL_INTERRUPT_CAUSE_CM3_SRAM_ADDRESS_OUT_OF_RANGE_INTERRUPT_E,\
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 0, NULL, NULL},                                \
        {7, GT_FALSE, MG_UNIT(1), NULL,                                                                 \
            CNM_MG_ADDRESS_MAC(2, 0x0000030),                                                           \
            CNM_MG_ADDRESS_MAC(2, 0x0000034),                                                           \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_AC5_MG_2_GLOBAL_INTERRUPTS_SUMMARY_CAUSE_GENERAL_INTERRUPT_SUMMARY_E,              \
            PRV_CPSS_AC5_MG_2_GLOBAL_INTERRUPTS_SUMMARY_CAUSE_MG1_INTERNAL_INTERRUPT_SUMMARY_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 2, NULL, NULL},                                     \
            {7, GT_FALSE, MG_UNIT(2), NULL,                                                             \
                CNM_MG_ADDRESS_MAC(2, 0x0000038),                                                       \
                CNM_MG_ADDRESS_MAC(2, 0x000003c),                                                       \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_AC5_MG_2_MG_INTERNAL_INTERRUPT_CAUSE_I2C_TIME_OUT_INTERRUPT_E,                 \
                PRV_CPSS_AC5_MG_2_MG_INTERNAL_INTERRUPT_CAUSE_SERDES_ADDRESS_OUT_OF_RANGE_VIOLATION_E,  \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 0, NULL, NULL},                                 \
            {18, GT_FALSE, MG_UNIT(2), NULL,                                                            \
                CNM_MG_ADDRESS_MAC(2, 0x000009c),                                                       \
                CNM_MG_ADDRESS_MAC(2, 0x00000a0),                                                       \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_AC5_MG_2_MG1_INTERNAL_INTERRUPT_CAUSE_MISCELLANEOUS_INTERRUPT_SUMMARY1_E,      \
                PRV_CPSS_AC5_MG_2_MG1_INTERNAL_INTERRUPT_CAUSE_CM3_SRAM_ADDRESS_OUT_OF_RANGE_INTERRUPT_E,\
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 0, NULL, NULL}

#define cnm_grp_0_6_IntsSum_SUB_TREE_MAC(bit, _mgIndex, numChild)                                       \
    /* Ports1IntSum */                                                                                  \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x00000150),                                                                    \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x00000154),                                                                    \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5_MG_##_mgIndex##_PORTS_1_INTERRUPTS_SUMMARY_CAUSE_G0_INTR_PORT_WAKEUP_E,            \
        PRV_CPSS_AC5_MG_##_mgIndex##_PORTS_1_INTERRUPTS_SUMMARY_CAUSE_IUNIT_INTR_E,                     \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, numChild, NULL, NULL}


#define cnm_grp_0_7_IntsSum_SUB_TREE_MAC(bit, _mgIndex, numChild)                                       \
    /* Ports2ntSum */                                                                                   \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x00000158),                                                                    \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x0000015C),                                                                    \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AC5_MG_##_mgIndex##_PORTS_2_INTERRUPTS_SUMMARY_CAUSE_GPIO_7_0_INTR_E,                  \
        PRV_CPSS_AC5_MG_##_mgIndex##_PORTS_2_INTERRUPTS_SUMMARY_CAUSE_CNM_RFU_INT_SUM_E,                \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, numChild, NULL, NULL}


#define cnm_grp_0_8_IntsSum_SUB_TREE_MAC(bit, _mgIndex, numChild)                                       \
    /*  MG1 Internal Interrupt Cause register */                                                        \
    {bit, GT_FALSE, MG_UNIT(_mgIndex), NULL,                                                            \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x0000009c),                                                   \
        CNM_MG_ADDRESS_MAC(_mgIndex, 0x000000a0),                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5_MG_##_mgIndex##_MG1_INTERNAL_INTERRUPT_CAUSE_MISCELLANEOUS_INTERRUPT_SUMMARY1_E,             \
        PRV_CPSS_AC5_MG_##_mgIndex##_MG1_INTERNAL_INTERRUPT_CAUSE_CM3_SRAM_ADDRESS_OUT_OF_RANGE_INTERRUPT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


/* set values that will be updated/filled during runtime (initialization stage)*/
#define FILLED_IN_RUNTIME_CNS       0

#define PLR_INTERRUPT_LEAF_MAC(bit,intCauseAddr,intMaskAddr,stage)                                      \
    /* Policer Interrupt Cause  */                                                                      \
    {bit, GT_FALSE, 0, NULL, intCauseAddr, intMaskAddr,                                                 \
         prvCpssDrvHwPpPortGroupIsrRead,                                                                \
         prvCpssDrvHwPpPortGroupIsrWrite,                                                               \
         PRV_CPSS_AC5_POLICER_##stage##_DATA_ERROR_E,                                                   \
         PRV_CPSS_AC5_POLICER_##stage##_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,                       \
         FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* interrupt cause register for the GIGE MAC port */
#define GIGE_MAC_PORT_INTERRUPT_CAUSE_MAC(port)      (0x12000020  + (port)*0x1000)

/* interrupt mask register for the GIGE MAC port */
#define GIGE_MAC_PORT_INTERRUPT_MASK_MAC(port)       (0x12000024  + (port)*0x1000)

/* interrupt summary cause register for the GIGE MAC port */
#define GIGE_MAC_PORT_INTERRUPT_SUM_CAUSE_MAC(port)  (0x120000A0  + (port)*0x1000)

/* interrupt summary mask register for the GIGE MAC port */
#define GIGE_MAC_PORT_INTERRUPT_SUM_MASK_MAC(port)   (0x120000A4  + (port)*0x1000)

#define  GIGE_MAC_PORT_INTERRUPT_LEAF_MAC(port)                                                         \
     /* Port Interrupt Cause */                                                                         \
     {1, GT_FALSE, 0, NULL,                                                                             \
         GIGE_MAC_PORT_INTERRUPT_CAUSE_MAC(port),                                                       \
         GIGE_MAC_PORT_INTERRUPT_MASK_MAC(port),                                                        \
         prvCpssDrvHwPpPortGroupIsrRead,                                                                \
         prvCpssDrvHwPpPortGroupIsrWrite,                                                               \
         PRV_CPSS_AC5_PORT_##port##_LINK_STATUS_CHANGED_E,                                              \
         PRV_CPSS_AC5_PORT_##port##_MIB_COUNTER_WRAPAROUND_E,                                           \
         FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(port)                                                  \
     /* Port Interrupt Summary Cause */                                                                 \
     {(port)+1, GT_FALSE, 0, NULL,                                                                      \
                GIGE_MAC_PORT_INTERRUPT_SUM_CAUSE_MAC(port),                                            \
                GIGE_MAC_PORT_INTERRUPT_SUM_MASK_MAC(port),                                             \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_AC5_PORT_##port##_SUMMARY_INTERNAL_SUM_E,                                      \
                PRV_CPSS_AC5_PORT_##port##_SUMMARY_INTERNAL_SUM_E,                                      \
                FILLED_IN_RUNTIME_CNS, 0x0, 0x0, 1, NULL, NULL},                                        \
                    GIGE_MAC_PORT_INTERRUPT_LEAF_MAC(port)

/* interrupt cause register for the XLG MAC port */
#define XLG_MAC_PORT_INTERRUPT_CAUSE_MAC(port)      (0x120C0014  + (port)*0x1000)

/* interrupt mask register for the XLG MAC port */
#define XLG_MAC_PORT_INTERRUPT_MASK_MAC(port)       (0x120C0018  + (port)*0x1000)

/* external units interrupt cause register */
#define XLG_MAC_PORT_EXTERNAL_INTERRUPT_CAUSE_MAC(port)     (0x120C0058  + (port)*0x1000)

/* external units interrupt mask register */
#define XLG_MAC_PORT_EXTERNAL_INTERRUPT_MASK_MAC(port)      (0x120C005C  + (port)*0x1000)

/* interrupt cause register for the XLG port MPCS interrupts */
#define XLG_PORT_MPCS_INTERRUPT_CAUSE_MAC(port)     (0x12180008  + (port)*0x1000)

/* interrupt mask register for the XLG port MPCS interrupts */
#define XLG_PORT_MPCS_INTERRUPT_MASK_MAC(port)      (0x1218000C  + (port)*0x1000)

/* interrupt cause register for the XLG port XPCS interrupts */
#define XLG_PORT_XPCS_INTERRUPT_CAUSE_MAC(port)     (0x12180414  + (port)*0x1000)

/* interrupt mask register for the XLG port XPCS interrupts */
#define XLG_PORT_XPCS_INTERRUPT_MASK_MAC(port)      (0x12180418  + (port)*0x1000)

/* interrupt cause register for the XLG port XPCS lane interrupts */
#define XLG_PORT_XPCS_LANE_INTERRUPT_CAUSE_MAC(port,lane)     (0x12180460  + (port)*0x1000 + (lane)*0x44)

/* interrupt mask register for the XLG port XPCS laneinterrupts */
#define XLG_PORT_XPCS_LANE_INTERRUPT_MASK_MAC(port,lane)      (0x12180464  + (port)*0x1000 + (lane)*0x44)

#define  XLG_PORT_MAC_INTERRUPT_LEAF_MAC(port)        \
         /* Port Interrupt Cause */                                \
         { 1, GT_FALSE, 0, NULL,                                            \
              XLG_MAC_PORT_INTERRUPT_CAUSE_MAC(port),                       \
              XLG_MAC_PORT_INTERRUPT_MASK_MAC(port),                        \
              prvCpssDrvHwPpPortGroupIsrRead,                               \
              prvCpssDrvHwPpPortGroupIsrWrite,                              \
              PRV_CPSS_AC5_XLG_PORT_##port##_LINK_STATUS_CHANGED_E,       \
              PRV_CPSS_AC5_XLG_PORT_##port##_PFC_SYNC_FIFO_OVERRUN_E,     \
              FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  MPCS_INTERRUPT_LEAF_MAC(port)          \
         /* PCS40G Common Interrupt Cause */                                \
         { 5, GT_FALSE, 0, NULL,                                            \
              XLG_PORT_MPCS_INTERRUPT_CAUSE_MAC(port),                      \
              XLG_PORT_MPCS_INTERRUPT_MASK_MAC(port),                       \
              prvCpssDrvHwPpPortGroupIsrRead,                               \
              prvCpssDrvHwPpPortGroupIsrWrite,                              \
              PRV_CPSS_AC5_PCS40_G_PORT_##port##_COMMON__ACCESS_ERROR_E,   \
              PRV_CPSS_AC5_PCS40_G_PORT_##port##_COMMON_GB_LOCK_SYNC_CHANGE_E, \
              FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  XPCS_LANE_INTERRUPT_LEAF_MAC(port,lane)        \
         /* Lane Interrupt Cause */                                         \
         {(lane+7), GT_FALSE, 0, NULL,                                      \
                    XLG_PORT_XPCS_LANE_INTERRUPT_CAUSE_MAC(port,lane),      \
                    XLG_PORT_XPCS_LANE_INTERRUPT_MASK_MAC(port,lane),       \
                    prvCpssDrvHwPpPortGroupIsrRead,                         \
                    prvCpssDrvHwPpPortGroupIsrWrite,                        \
                    PRV_CPSS_AC5_XPCS_PORT_##port##_LANE_##lane##_PRBS_ERROR_E,     \
                    PRV_CPSS_AC5_XPCS_PORT_##port##_LANE_##lane##_DETECTED_IIAII_E, \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  XPCS_INTERRUPT_SUBTREE_MAC(port)        \
         /* XPCS Global Interrupt Cause */                                  \
         { 4, GT_FALSE, 0, NULL,                                            \
              XLG_PORT_XPCS_INTERRUPT_CAUSE_MAC(port),                      \
              XLG_PORT_XPCS_INTERRUPT_MASK_MAC(port),                       \
              prvCpssDrvHwPpPortGroupIsrRead,                               \
              prvCpssDrvHwPpPortGroupIsrWrite,                              \
              PRV_CPSS_AC5_XPCS_PORT_##port##_GLOBAL_LINK_STATUS_CHANGED_E,\
              PRV_CPSS_AC5_XPCS_PORT_##port##_GLOBAL_SUM_LANE5_E,          \
              FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6, NULL, NULL},       \
                  XPCS_LANE_INTERRUPT_LEAF_MAC(port,0),                     \
                  XPCS_LANE_INTERRUPT_LEAF_MAC(port,1),                     \
                  XPCS_LANE_INTERRUPT_LEAF_MAC(port,2),                     \
                  XPCS_LANE_INTERRUPT_LEAF_MAC(port,3),                     \
                  XPCS_LANE_INTERRUPT_LEAF_MAC(port,4),                     \
                  XPCS_LANE_INTERRUPT_LEAF_MAC(port,5)

#define  XLG_MAC_PORT_ODD_INTERRUPT_SUM_SUBTREE_MAC(port)                   \
         /* External Units Interrupts cause */                                  \
          {(port)+1, GT_FALSE, 0, NULL,                                          \
                    XLG_MAC_PORT_EXTERNAL_INTERRUPT_CAUSE_MAC(port),            \
                    XLG_MAC_PORT_EXTERNAL_INTERRUPT_MASK_MAC(port),             \
                    prvCpssDrvHwPpPortGroupIsrRead,                             \
                    prvCpssDrvHwPpPortGroupIsrWrite,                            \
                    PRV_CPSS_AC5_EXTERNAL_UNITS_PORT##port##_CAUSE_XLG_PORT_INTERRUPT_SUMMARY_E,  \
                    PRV_CPSS_AC5_EXTERNAL_UNITS_PORT##port##_CAUSE_GB_INTERRUPT_SUMMARY_E,        \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0x0, 3, NULL, NULL},            \
                        XLG_PORT_MAC_INTERRUPT_LEAF_MAC(port),                  \
                        /* Port Interrupt Summary Cause */                      \
                        {2, GT_FALSE, 0, NULL,                                  \
                            GIGE_MAC_PORT_INTERRUPT_SUM_CAUSE_MAC(port),        \
                            GIGE_MAC_PORT_INTERRUPT_SUM_MASK_MAC(port),         \
                            prvCpssDrvHwPpPortGroupIsrRead,                     \
                            prvCpssDrvHwPpPortGroupIsrWrite,                    \
                            PRV_CPSS_AC5_PORT_##port##_SUMMARY_INTERNAL_SUM_E, \
                            PRV_CPSS_AC5_PORT_##port##_SUMMARY_INTERNAL_SUM_E, \
                            FILLED_IN_RUNTIME_CNS, 0x0, 0x0, 1, NULL, NULL},    \
                                GIGE_MAC_PORT_INTERRUPT_LEAF_MAC(port),         \
                        MPCS_INTERRUPT_LEAF_MAC(port)

#define  XLG_MAC_PORT_EVEN_INTERRUPT_SUM_SUBTREE_MAC(port)      \
            /* External Units Interrupts cause */                               \
         {(port)+1, GT_FALSE, 0, NULL,                                          \
                    XLG_MAC_PORT_EXTERNAL_INTERRUPT_CAUSE_MAC(port),            \
                    XLG_MAC_PORT_EXTERNAL_INTERRUPT_MASK_MAC(port),             \
                    prvCpssDrvHwPpPortGroupIsrRead,                             \
                    prvCpssDrvHwPpPortGroupIsrWrite,                            \
                    PRV_CPSS_AC5_EXTERNAL_UNITS_PORT##port##_CAUSE_XLG_PORT_INTERRUPT_SUMMARY_E,  \
                    PRV_CPSS_AC5_EXTERNAL_UNITS_PORT##port##_CAUSE_GB_INTERRUPT_SUMMARY_E,        \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0x0, 4, NULL, NULL},            \
                        XLG_PORT_MAC_INTERRUPT_LEAF_MAC(port),                  \
                        /* Port Interrupt Summary Cause */                      \
                        {2, GT_FALSE, 0, NULL,                                  \
                            GIGE_MAC_PORT_INTERRUPT_SUM_CAUSE_MAC(port),        \
                            GIGE_MAC_PORT_INTERRUPT_SUM_MASK_MAC(port),         \
                            prvCpssDrvHwPpPortGroupIsrRead,                     \
                            prvCpssDrvHwPpPortGroupIsrWrite,                    \
                            PRV_CPSS_AC5_PORT_##port##_SUMMARY_INTERNAL_SUM_E, \
                            PRV_CPSS_AC5_PORT_##port##_SUMMARY_INTERNAL_SUM_E, \
                            FILLED_IN_RUNTIME_CNS, 0x0, 0x0, 1, NULL, NULL},    \
                                GIGE_MAC_PORT_INTERRUPT_LEAF_MAC(port),         \
                        MPCS_INTERRUPT_LEAF_MAC(port),                          \
                        XPCS_INTERRUPT_SUBTREE_MAC(port)

/**
* @internal alleyCat5XgPortMultiplexerHwPpPortGroupIsrRead function
* @endinternal
*
* @brief   Ports 28 and 29 interrupts are muxed in the design with ports 25 and 27,
*         that means that out of 6 MAC units, only 4 interrupts are triggered in
*         MG.
*         Anyway, port 28 will be active only when port 25 is not,
*         and the same for ports 27 and 29.
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] dataPtr                  - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note
*       GalTis:
*       None.
*
*/
static GT_STATUS alleyCat5XgPortMultiplexerHwPpPortGroupIsrRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
)
{
    GT_STATUS rc; /* return code */

    /* read cause register */
    rc = prvCpssDrvHwPpPortGroupIsrRead(devNum, portGroupId, regAddr, dataPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Emulate bit for port 28 according to port 25 bit value  */
    *dataPtr |=
        ((*dataPtr >>
             (PRV_CPSS_AC5_PORTS_S_SUMMARY_XG_PORT25_SUM_E -
                PRV_CPSS_AC5_PORTS_S_SUMMARY_PORTS_SUM_E)) & 0x1) <<
             (PRV_CPSS_AC5_PORTS_S_SUMMARY_PORT_SUM_28_E -
                PRV_CPSS_AC5_PORTS_S_SUMMARY_PORTS_SUM_E);

    /* Emulate bit for port 29 according to port 27 bit value  */
    *dataPtr |=
        ((*dataPtr >>
             (PRV_CPSS_AC5_PORTS_S_SUMMARY_XG_PORT27_SUM_E -
                PRV_CPSS_AC5_PORTS_S_SUMMARY_PORTS_SUM_E)) & 0x1) <<
             (PRV_CPSS_AC5_PORTS_S_SUMMARY_PORT_SUM_29_E -
                PRV_CPSS_AC5_PORTS_S_SUMMARY_PORTS_SUM_E);

    return GT_OK;
}

#define  MG_LEGACY_INTERRUPTS_MAC(bitIndexInCaller)                                                 \
    /* Global Interrupts Summary Cause */                                                           \
    {bitIndexInCaller, GT_FALSE, 0, NULL, 0x00000030, 0x00000034,                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_AC5_GLOBAL_S_SUMMARY_FUNC_UNITSS_SUM_E,                                            \
        PRV_CPSS_AC5_GLOBAL_S_SUMMARY_PORTS_2_SUM_E,                                                \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 11, NULL, NULL},                                             \
                                                                                                    \
        /* Functional Interrupts Summary Cause */                                                   \
        {3, GT_FALSE, 0, NULL, 0x000003F8, 0x000003FC,                                              \
            prvCpssDrvHwPpPortGroupIsrRead,                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
            PRV_CPSS_AC5_FUNCTIONAL_S_SUMMARY_CNC_SUM_E,                                            \
            PRV_CPSS_AC5_FUNCTIONAL_S_SUMMARY_TCC1_LOWER_SUM_E,                                     \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 16, NULL, NULL},                                         \
                                                                                                    \
            /* CNC Interrupt Cause Register */                                                      \
            {1, GT_FALSE, 0, NULL, 0x10000100, 0x10000104,                                          \
                prvCpssDrvHwPpPortGroupIsrRead,                                                     \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_AC5_CNC_BLOCK0_WRAPAROUND_E,                                               \
                PRV_CPSS_AC5_CNC_DUMP_FINISHED_E,                                                   \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                             \
                                                                                                    \
            /* IPvX TCC Interrupt Cause */                                                          \
            {2, GT_FALSE, 0, NULL, 0x1B000060, 0x1b000064,                                          \
                prvCpssDrvHwPpPortGroupIsrRead,                                                     \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_AC5_TCC_UPPER_TCAM_ERROR_DETECTED_E,                                       \
                PRV_CPSS_AC5_TCC_UPPER_TCAM_BIST_FAILED_E,                                          \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                             \
                                                                                                    \
            /* Pre-Egress Interrupt Summary */                                                      \
            {3, GT_FALSE, 0, NULL, 0x14000058, 0x1400005C,                                          \
                prvCpssDrvHwPpPortGroupIsrRead,                                                     \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_AC5_PRE_EGRESS_SUMMARY_INGRESS_STC_SUM_E,                                  \
                PRV_CPSS_AC5_PRE_EGRESS_SUMMARY_STC_RATE_LIMITERS_SUM_E,                            \
                FILLED_IN_RUNTIME_CNS, 0x0, 0x0, 2, NULL, NULL},                                    \
                                                                                                    \
                /* Ingress STC Interrupt Cause */                                                   \
                {1, GT_FALSE, 0, NULL, 0x14000020, 0x14000024,                                      \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_AC5_INGRESS_STC_PORT_0_INGRESS_SAMPLE_LOADED_E,                        \
                    PRV_CPSS_AC5_INGRESS_STC_PORT_27_INGRESS_SAMPLE_LOADED_E,                       \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                         \
                                                                                                    \
                /* CPU Code Rate Limiters Interrupt Cause */                                        \
                {2, GT_FALSE, 0, NULL, 0x14000060, 0x14000064,                                      \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_AC5_CPU_CODE_RATE_LIMITERS_STC_RATE_LIMITER_1_PKT_DROPED_E,            \
                    PRV_CPSS_AC5_CPU_CODE_RATE_LIMITERS_STC_RATE_LIMITER_31_PKT_DROPED_E,           \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                         \
                                                                                                    \
            /* Bridge Interrupt Cause */                                                            \
            {4, GT_FALSE, 0, NULL, 0x01040130, 0x01040134,                                          \
                prvCpssDrvHwPpPortGroupIsrRead,                                                     \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_AC5_BRIDGE_ADDRESS_OUT_OF_RANGE_E,                                         \
                PRV_CPSS_AC5_BRIDGE_PCR_PARITY_ERROR_E,                                             \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                             \
                                                                                                    \
            /* Buffer Memory Main Interrupt Cause Register */                                       \
            {5, GT_FALSE, 0, NULL, 0x0E000090, 0x0E000094,                                          \
                prvCpssDrvHwPpPortGroupIsrRead,                                                     \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_AC5_BM_MAIN_REG0_SUM_E,                                                    \
                PRV_CPSS_AC5_BM_MAIN_REG1_SUM_E,                                                    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0x0, 2, NULL, NULL},                                    \
                                                                                                    \
                /* Buffer Memory Interrupt Cause Register0 */                                       \
                {1, GT_FALSE, 0, NULL, 0x0E000024, 0x0E000028,                                      \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_AC5_BM_0_PORT0_TX_FIFO_OVERRUN_E,                                      \
                    PRV_CPSS_AC5_BM_0_BURST_FIFO_OVERRUN_E,                                         \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                         \
                                                                                                    \
                /* Buffer Memory Interrupt Cause Register1 */                                       \
                {2, GT_FALSE, 0, NULL, 0x0E000098, 0x0E00009C,                                      \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_AC5_BM_1_BANK0_ONE_ECC_ERROR_E,                                        \
                    PRV_CPSS_AC5_BM_1_BANK1_TWO_OR_MORE_ECC_ERRORS_E,                               \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                         \
                                                                                                    \
            PLR_INTERRUPT_LEAF_MAC(6,0x06000100,0x06000104,EPLR),                                   \
                                                                                                    \
            /* FDB Interrupt Cause Register */                                                      \
            {7, GT_FALSE, 0, NULL, 0x0B000018, 0x0B00001C,                                          \
                prvCpssDrvHwPpPortGroupIsrRead,                                                     \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_AC5_FDB_NUM_OF_HOP_EX_P_E,                                                 \
                PRV_CPSS_AC5_FDBAU_FIFO_TO_CPU_IS_FULL_E,                                           \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                             \
                                                                                                    \
            /* TTI Engine Interrupt Cause */                                                        \
            {8, GT_FALSE, 0, NULL, 0x16000004, 0x16000008,                                          \
                prvCpssDrvHwPpPortGroupIsrRead,                                                     \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_AC5_TTI_CPU_ADDRESS_OUT_OF_RANGE_E,                                        \
                PRV_CPSS_AC5_TTI_ACCESS_DATA_ERROR_E,                                               \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                             \
                                                                                                    \
            /* IPCL TCC Interrupt Cause */                                                          \
            {9, GT_FALSE, 0, NULL, 0x1A0001A4, 0x1A0001A8,                                          \
                prvCpssDrvHwPpPortGroupIsrRead,                                                     \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_AC5_TCC_LOWER_TCAM_ERROR_DETECTED_E,                                       \
                PRV_CPSS_AC5_TCC_LOWER_TCAM_BIST_FAILED_E,                                          \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                             \
                                                                                                    \
            /* Policy Engine Interrupt Cause */                                                     \
            {10, GT_FALSE, 0, NULL, 0x15000004, 0x15000008,                                         \
                 prvCpssDrvHwPpPortGroupIsrRead,                                                    \
                 prvCpssDrvHwPpPortGroupIsrWrite,                                                   \
                 PRV_CPSS_AC5_POLICY_ENGINE_MG_ADDR_OUT_OF_RANGE_E,                                 \
                 PRV_CPSS_AC5_POLICY_ENGINE_PCL_CONFIG_TABLE_1_PARITY_ERROR_E,                      \
                 FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                            \
                                                                                                    \
            PLR_INTERRUPT_LEAF_MAC(11,0x17000100,0x17000104,IPLR0),                                 \
                                                                                                    \
            PLR_INTERRUPT_LEAF_MAC(12,0x18000100,0x18000104,IPLR1),                                 \
                                                                                                    \
            /* BCN Interrupt Cause */                                                               \
            {13, GT_FALSE, 0, NULL, 0x03400018, 0x0340001C,                                         \
                 prvCpssDrvHwPpPortGroupIsrRead,                                                    \
                 prvCpssDrvHwPpPortGroupIsrWrite,                                                   \
                 PRV_CPSS_AC5_BCN_COUNTER_WRAP_AROUND_ERR_E,                                        \
                 PRV_CPSS_AC5_BCN_COUNTER_WRAP_AROUND_ERR_E,                                        \
                 FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                            \
                                                                                                    \
            /* Transmit Queue Interrupt Summary Cause */                                            \
            {14, GT_FALSE, 0, NULL, 0x02800100, 0x02800104,                                         \
                 prvCpssDrvHwPpPortGroupIsrRead,                                                    \
                 prvCpssDrvHwPpPortGroupIsrWrite,                                                   \
                 PRV_CPSS_AC5_TXQ_SUMMARY_WATCHDOG_SUM_E,                                           \
                 PRV_CPSS_AC5_TXQ_SUMMARY_TXQ_FULL_SUM_E,                                           \
                 FILLED_IN_RUNTIME_CNS, 0x0, 0x0, 6, NULL, NULL},                                   \
                                                                                                    \
                 /* Transmit Queue WatchDog Interrupt Cause */                                      \
                 {1, GT_FALSE, 0, NULL, 0x02800108, 0x0280010C,                                     \
                     prvCpssDrvHwPpPortGroupIsrRead,                                                \
                     prvCpssDrvHwPpPortGroupIsrWrite,                                               \
                     PRV_CPSS_AC5_TXQ_WATCHDOG_EXP_PORT_0_E,                                        \
                     PRV_CPSS_AC5_TXQ_WATCHDOG_CPU_WATCH_DOG_EXP_E,                                 \
                     FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                        \
                                                                                                    \
                 /* Transmit Queue Flush Done Interrupt Cause */                                    \
                 {2, GT_FALSE, 0, NULL, 0x02800110, 0x02800114,                                     \
                     prvCpssDrvHwPpPortGroupIsrRead,                                                \
                     prvCpssDrvHwPpPortGroupIsrWrite,                                               \
                     PRV_CPSS_AC5_TXQ_FLUSH_DONE_PORT_0_E,                                          \
                     PRV_CPSS_AC5_TXQ_FLUSH_DONE_PORT_31_E,                                         \
                     FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                        \
                                                                                                    \
                 /* Transmit Queue General Interrupt Cause */                                       \
                 {3, GT_FALSE, 0, NULL, 0x02800118, 0x0280011C,                                     \
                     prvCpssDrvHwPpPortGroupIsrRead,                                                \
                     prvCpssDrvHwPpPortGroupIsrWrite,                                               \
                     PRV_CPSS_AC5_TXQ_GENERAL_EGRESS_MIRORR_DESC_DROP_E,                            \
                     PRV_CPSS_AC5_TXQ_GENERAL_VLAN_PARITY_ERROR_E,                                  \
                     FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                        \
                                                                                                    \
                 /* GPP Interrupt Cause Register */                                                 \
                 {4, GT_FALSE, 0, NULL, 0x028001CC, 0x028001C0,                                     \
                     prvCpssDrvHwPpPortGroupIsrRead,                                                \
                     prvCpssDrvHwPpPortGroupIsrWrite,                                               \
                     PRV_CPSS_AC5_GPPGPP_0_E,                                                       \
                     PRV_CPSS_AC5_GPPGPP_11_E,                                                      \
                     FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                        \
                                                                                                    \
                 /* Egress STC Interrupt Cause */                                                   \
                 {5, GT_FALSE, 0, NULL, 0x02800130, 0x02800134,                                     \
                     prvCpssDrvHwPpPortGroupIsrRead,                                                \
                     prvCpssDrvHwPpPortGroupIsrWrite,                                               \
                     PRV_CPSS_AC5_EGRESS_STC_PORT_0_E,                                              \
                     PRV_CPSS_AC5_EGRESS_STC_PORT_27_E,                                             \
                     FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                        \
                                                                                                    \
                 /* Transmit Queue Desc Full Interrupt Summary Cause */                             \
                 {7, GT_FALSE, 0, NULL, 0x0280018C, 0x028001EC,                                     \
                     prvCpssDrvHwPpPortGroupIsrRead,                                                \
                     prvCpssDrvHwPpPortGroupIsrWrite,                                               \
                     PRV_CPSS_AC5_TXQ_DESC_FULL_SUMMARY_DESC_FULL_PORT_0_E,                         \
                     PRV_CPSS_AC5_TXQ_DESC_FULL_SUMMARY_XG_MC_FIFO_FULL_E,                          \
                     FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                        \
            /* CNC1 Interrupt Cause Register */                                                     \
            {15, GT_FALSE, 0, NULL, 0x1F000100, 0x1F000104,                                         \
             prvCpssDrvHwPpPortGroupIsrRead,                                                        \
             prvCpssDrvHwPpPortGroupIsrWrite,                                                       \
             PRV_CPSS_AC5_CNC1_BLOCK0_WRAPAROUND_E,                                                 \
             PRV_CPSS_AC5_CNC1_DUMP_FINISHED_E,                                                     \
             FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                \
                                                                                                    \
            /* PCL TCC1 Interrupt Cause */                                                         \
            {16, GT_FALSE, 0, NULL, 0x200001A4, 0x200001A8,                                         \
             prvCpssDrvHwPpPortGroupIsrRead,                                                        \
             prvCpssDrvHwPpPortGroupIsrWrite,                                                       \
             PRV_CPSS_AC5_TCC1_LOWER_TCAM_ERROR_DETECTED_E,                                         \
             PRV_CPSS_AC5_TCC1_LOWER_TCAM_BIST_FAILED_E,                                            \
             FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                \
                                                                                                    \
        /* Data Path Interrupts Summary Cause */                                                    \
        {4, GT_FALSE, 0, NULL, 0x000000A4, 0x000000A8,                                              \
            prvCpssDrvHwPpPortGroupIsrRead,                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
            PRV_CPSS_AC5_DATA_PATH_S_SUMMARY_LMS_SUM_E,                                             \
            PRV_CPSS_AC5_DATA_PATH_S_SUMMARY_BM_SUM1_E,                                             \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 3, NULL, NULL},                                          \
                                                                                                    \
            /* MIB Counters Interrupt Summary  */                                                   \
            {1, GT_FALSE, 0, NULL, 0x07004010, 0x08005110,                                          \
                prvCpssDrvHwPpPortGroupIsrRead,                                                     \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_AC5_MIB_COUNTERS_SUMMARY_PORT_SUM_E,                                       \
                PRV_CPSS_AC5_MIB_COUNTERS_SUMMARY_PORT_SUM_E,                                       \
                FILLED_IN_RUNTIME_CNS, 0x0, 0x0, 1, NULL, NULL},                                    \
                /* Ports Interrupt Summary */                                                       \
                {5, GT_FALSE,0, NULL, 0x09004010, 0x08005114,                                       \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_AC5_PORTS_SUMMARY_PORT_0_SUM_E,                                        \
                    PRV_CPSS_AC5_PORTS_SUMMARY_CPU_PORT_SUM_E,                                      \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0x0, 25, NULL, NULL},                               \
                                                                                                    \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(0),  /* Port  0 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(1),  /* Port  1 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(2),  /* Port  2 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(3),  /* Port  3 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(4),  /* Port  4 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(5),  /* Port  5 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(6),  /* Port  6 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(7),  /* Port  7 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(8),  /* Port  8 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(9),  /* Port  9 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(10), /* Port 10 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(11), /* Port 11 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(12), /* Port 12 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(13), /* Port 13 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(14), /* Port 14 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(15), /* Port 15 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(16), /* Port 16 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(17), /* Port 17 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(18), /* Port 18 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(19), /* Port 19 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(20), /* Port 20 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(21), /* Port 21 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(22), /* Port 22 */                      \
                    GIGE_MAC_PORT_INTERRUPT_SUM_SUBTREE_MAC(23), /* Port 23 */                      \
                                                                                                    \
                    {24, GT_FALSE,0, NULL,                                                          \
                         XLG_MAC_PORT_EXTERNAL_INTERRUPT_CAUSE_MAC(31),                             \
                         XLG_MAC_PORT_EXTERNAL_INTERRUPT_MASK_MAC(31),                              \
                         prvCpssDrvHwPpPortGroupIsrRead,                                            \
                         prvCpssDrvHwPpPortGroupIsrWrite,                                           \
                         PRV_CPSS_AC5_EXTERNAL_UNITS_PORT_CPU_CAUSE_GIGE_PORT_INTERRUPT_SUMMARY_E,  \
                         PRV_CPSS_AC5_EXTERNAL_UNITS_PORT_CPU_CAUSE_GIGE_PORT_INTERRUPT_SUMMARY_E,  \
                         FILLED_IN_RUNTIME_CNS, 0x0, 0x0, 1, NULL, NULL},                           \
                                                                                                    \
                         {2, GT_FALSE,0, NULL,                                                      \
                             GIGE_MAC_PORT_INTERRUPT_SUM_CAUSE_MAC(31),                             \
                             GIGE_MAC_PORT_INTERRUPT_SUM_MASK_MAC(31),                              \
                             prvCpssDrvHwPpPortGroupIsrRead,                                        \
                             prvCpssDrvHwPpPortGroupIsrWrite,                                       \
                             PRV_CPSS_AC5_PORT_CPU_SUMMARY_INTERNAL_SUM_E,                          \
                             PRV_CPSS_AC5_PORT_CPU_SUMMARY_INTERNAL_SUM_E,                          \
                             FILLED_IN_RUNTIME_CNS, 0x0, 0x0, 1, NULL, NULL},                       \
                                GIGE_MAC_PORT_INTERRUPT_LEAF_MAC(31),                               \
                                                                                                    \
            /* Buffer Management Interrupt Cause0  */                                               \
            {2, GT_FALSE, 0, NULL, 0x05000040, 0x05000044,                                          \
                prvCpssDrvHwPpPortGroupIsrRead,                                                     \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_AC5_BM_EMPTY_CLEAR_E,                                                      \
                PRV_CPSS_AC5_BM_PORT_RX_FULL_E,                                                     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                             \
                                                                                                    \
            /* Buffer Management Interrupt Cause1  */                                               \
            {3, GT_FALSE, 0, NULL, 0x05000048, 0x0500004C,                                          \
                prvCpssDrvHwPpPortGroupIsrRead,                                                     \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_AC5_BM_BUFF_LIMIT_REACHED_PORT0_E,                                         \
                PRV_CPSS_AC5_BM_CPU_PORT_BUFF_LIMIT_REACHED_E,                                      \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                             \
                                                                                                    \
        /* Ports Interrupts Summary Cause */                                                        \
        {5, GT_FALSE, 0, NULL, 0x00000080, 0x00000084,                                              \
            alleyCat5XgPortMultiplexerHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
            PRV_CPSS_AC5_PORTS_S_SUMMARY_XG_PORT24_SUM_E,                                           \
            PRV_CPSS_AC5_PORTS_S_SUMMARY_XG_PORT27_SUM_E,                                           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0x0, 6, NULL, NULL},                                        \
            XLG_MAC_PORT_EVEN_INTERRUPT_SUM_SUBTREE_MAC(24),                                        \
            XLG_MAC_PORT_ODD_INTERRUPT_SUM_SUBTREE_MAC(25),                                         \
            XLG_MAC_PORT_EVEN_INTERRUPT_SUM_SUBTREE_MAC(26),                                        \
            XLG_MAC_PORT_ODD_INTERRUPT_SUM_SUBTREE_MAC(27),                                         \
            XLG_MAC_PORT_ODD_INTERRUPT_SUM_SUBTREE_MAC(28),                                         \
            XLG_MAC_PORT_ODD_INTERRUPT_SUM_SUBTREE_MAC(29),                                         \
                                                                                                    \
        /* DFX Interrupts Summary Cause */                                                          \
        {6, GT_FALSE, 0, NULL, 0x000000AC, 0x000000B0,                                              \
            prvCpssDrvHwPpPortGroupIsrRead,                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
            PRV_CPSS_AC5_DFX_S_SUMMARY_DFX_CLIENT_MT2_MACRO_CORE_CLK_SUM_E,                         \
            PRV_CPSS_AC5_DFX_S_SUMMARY_DFX_INTERRUPT_SUM_31_E,                                      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                    \
        /* MG internal Interrupt Cause */                                                           \
        {7, GT_FALSE, 0, NULL, 0x00000038, 0x0000003C,                                              \
            prvCpssDrvHwPpPortGroupIsrRead,                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
            PRV_CPSS_AC5_MG_INTERNAL_TWSI_TIME_OUT_E,                                               \
            PRV_CPSS_AC5_MG_INTERNAL_READ_BURST_VIOLATION_E,                                        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                    \
        /* Transmit SDMA Interrupt Cause0 */                                                        \
        {8, GT_FALSE, 0, NULL, 0x00002810, 0x00002818,                                              \
            prvCpssDrvHwPpPortGroupIsrRead,                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
            PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_BUFFER_QUEUE_0_E,                                        \
            PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_END_QUEUE_7_E,                                           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                    \
        /* Receive SDMA Interrupt Cause0 */                                                         \
        {9, GT_FALSE, 0, NULL, 0x0000280C, 0x00002814,                                              \
            prvCpssDrvHwPpPortGroupIsrRead,                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
            PRV_CPSS_AC5_RECEIVE_SDMA0_RX_BUFFER_QUEUE_0_E,                                         \
            PRV_CPSS_AC5_RECEIVE_SDMA0_PACKET_CNT_OF_E,                                             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                    \
        /* DFX 1 Interrupts Summary Cause */                                                        \
        {10, GT_FALSE, 0, NULL, 0x000000B8, 0x000000BC,                                             \
             prvCpssDrvHwPpPortGroupIsrRead,                                                        \
             prvCpssDrvHwPpPortGroupIsrWrite,                                                       \
             PRV_CPSS_AC5_DFX_1_S_SUMMARY_DFX_1_INTERRUPT_SUM_1_E,                                  \
             PRV_CPSS_AC5_DFX_1_S_SUMMARY_DFX_1_INTERRUPT_SUM_31_E,                                 \
             FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                \
                                                                                                    \
        /* Functional 1 Interrupts Summary Cause */                                                 \
        {11, GT_FALSE, 0, NULL, 0x000003F4, 0x000003F0,                                             \
             prvCpssDrvHwPpPortGroupIsrRead,                                                        \
             prvCpssDrvHwPpPortGroupIsrWrite,                                                       \
             PRV_CPSS_AC5_MG_IP_FUNCTIONAL_1_INTERRUPTS_SUMMARY_CAUSE_ALL_MG_TERM_TIMEOUT_INT3_E,   \
             PRV_CPSS_AC5_MG_IP_FUNCTIONAL_1_INTERRUPTS_SUMMARY_CAUSE_FUNC_UNITS_1_INT_SUM_31_E,    \
             FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                \
                                                                                                    \
        /* Ports 1 Interrupts Summary Cause */                                                      \
        {16, GT_FALSE, 0, NULL, 0x00000150, 0x00000154,                                             \
             prvCpssDrvHwPpPortGroupIsrRead,                                                        \
             prvCpssDrvHwPpPortGroupIsrWrite,                                                       \
             PRV_CPSS_AC5_MG_IP_PORTS_1_INTERRUPTS_SUMMARY_CAUSE_PORT_1_INT_SUM_1_E,                \
             PRV_CPSS_AC5_MG_IP_PORTS_1_INTERRUPTS_SUMMARY_CAUSE_PORT_1_INT_SUM_31_E,               \
             FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                \
                                                                                                    \
        /* Ports 2 Interrupts Summary Cause */                                                      \
        {17, GT_FALSE, 0, NULL, 0x00000158, 0x0000015C,                                             \
             prvCpssDrvHwPpPortGroupIsrRead,                                                        \
             prvCpssDrvHwPpPortGroupIsrWrite,                                                       \
             PRV_CPSS_AC5_MG_IP_PORTS_2_INTERRUPTS_SUMMARY_CAUSE_PORT_2_INT_SUM_1_E,                \
             PRV_CPSS_AC5_MG_IP_PORTS_2_INTERRUPTS_SUMMARY_CAUSE_PORT_2_INT_SUM_31_E,               \
             FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* number of elements in the array of alleycat5IntrScanArr[] */
#define AC5_NUM_ELEMENTS_IN_SCAN_TREE_CNS \
    (sizeof(alleyCat5IntrScanArr)/sizeof(alleyCat5IntrScanArr[0]))

/* number of mask registers -- according to enum of interrupts */
#define AC5_NUM_MASK_REGISTERS_CNS  (PRV_CPSS_AC5_LAST_INT_E / 32)

#define MG_CNM_INTERRUPTS_MAC(bitIndexInCaller)                                     \
    /* Global Interrupt Cause */                                                    \
    MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, 0 /*_mgIndex*/, 7 /*numChild*/),        \
    /* Data Path Interrupt Summary cause */                                         \
    cnm_grp_0_1_IntsSum_SUB_TREE_MAC(4, 0 /*_mgIndex*/),                            \
    /* MG internal Interrupt Cause */                                               \
    mg0InternalIntsSum_SUB_TREE_MAC( 7,  0 /*_mgIndex*/, 1 /*numChild*/),           \
    /* DFX1 interrupts summary cause */                                             \
    cnm_grp_0_4_IntsSum_SUB_TREE_MAC(10, 0 /*_mgIndex*/, 1 /*numChild*/),           \
    /* FuncUnits1IntsSum */                                                         \
    cnm_grp_0_5_IntsSum_SUB_TREE_MAC(11, 0 /*_mgIndex*/, 2 /*numChild*/),           \
    /* Ports 1 Summary Interrupt Cause */                                           \
    cnm_grp_0_6_IntsSum_SUB_TREE_MAC(16, 0 /*_mgIndex*/, 0 /*numChild*/),           \
    /* Ports 2 Interrupt Summary */                                                 \
    cnm_grp_0_7_IntsSum_SUB_TREE_MAC(17, 0 /*_mgIndex*/, 0 /*numChild*/),           \
    /* MG1 Internal Interrupt Cause register summary bit */                         \
    cnm_grp_0_8_IntsSum_SUB_TREE_MAC(18, 0 /*_mgIndex*/, 0 /*numChild*/)

/*
 * Typedef: struct INTERRUPT_SCAN
 *
 * Description: Basic struct for accessing interrupt register in hierarchy
 *              tree.
 *
 * Fields:
 *      bitNum          - Sum bit num in upper hierarchy, representing this
 *                        register.
 *      isGpp           - Is hierarchy connected to another device.
 *      gppId           - The gpp Id represented by this node (Valid only if
 *                        isGpp is GT_TRUE).
 *      gppFuncPtr      - Pointer to isrFunc to be called if isGPP == GT_TRUE.
 *      devNum          - Device number for isrFunc Call.
 *      causeRegAddr    - Address of the interrupt cause register to scan.
 *      maskRegAddr     - Address of the interrupt mask register to update after
 *                        scanning.
 *      pRegReadFunc    - A pointer to a function to be used for reading the
 *                        interrupt cause register.
 *      pRegWriteFunc   - A pointer to a function to be used for writing the
 *                        interrupt mask register.
 *      startIdx        - The start interrupt index representing the interrupts
 *                        to be scanned.
 *      endIdx          - The end interrupt index representing the interrupts to
 *                        be scanned.
 *      nonSumBitMask   - bit mask of non sum bits in the register
 *      rwBitMask       - bit mask of R/W bits that should be cleared by write.
 *      maskRcvIntrEn   - bit mask of interrupts to be masked after receive.
 *      subIntrListLen  - List length sub interrupts list.
 *      subIntrScan     - pointer to sum bit interrupt register structure.
 *      nextIntrScan    - pointer to next interrupt struct in hierarchy.
 */

static const PRV_CPSS_DRV_INTERRUPT_SCAN_STC alleyCat5IntrScanArr[] =
{
    MG_CNM_INTERRUPTS_MAC(0)
};

#define SET_EVENT_PER_INDEX____1_31___MAC(_prefix,_postFix)    \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     1       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     2       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     3       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     4       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     5       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     6       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     7       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     8       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     9       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     10      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     11      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     12      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     13      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     14      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     15      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     16      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     17      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     18      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     19      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     20      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     21      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     22      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     23      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     24      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     25      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     26      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     27      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     28      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     29      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     30      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     31      )


/* state for GIGE ports 0..27 that event hold extra port number */
#define SET_EVENT_PER_GIGE_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix)   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     0),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     1),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     2),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     3),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     4),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     5),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     6),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     7),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     8),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     9),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     10),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     11),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     12),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     13),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     14),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     15),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     16),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     17),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     18),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     19),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     20),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     21),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     22),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     23),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     24),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     25),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     26),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     27)

/* state for GIGE ports 0..27 and CPU (31) that event hold extra port number */
#define SET_EVENT_PER_GIGE_PORTS_AND_CPU__ON_ALL_PORTS_MAC(_prefix,_postFix)   \
    SET_EVENT_PER_GIGE_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     31)

/* state for GIGE ports 0..29 and CPU (31) that event hold extra port number */
#define SET_EVENT_PER_ALL_GIGE_PORTS_AND_CPU__ON_ALL_PORTS_MAC(_prefix,_postFix)   \
    SET_EVENT_PER_GIGE_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix),   \
    _prefix##_##PORT##_##28##_##_postFix,  MARK_PER_PORT_INT_MAC(25), \
    _prefix##_##PORT##_##29##_##_postFix,  MARK_PER_PORT_INT_MAC(27), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     31)

/* state for even XLG ports 24 & 26 that event hold extra port number */
#define SET_EVENT_PER_XLG_PORTS_EVEN__ON_ALL_PORTS_MAC(_prefix,_postFix)   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     24),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     26)

/* state for odd XLG ports 25, 27, 28 & 29 that event hold extra port number */
#define SET_EVENT_PER_XLG_PORTS_ODD__ON_ALL_PORTS_MAC(_prefix,_postFix)   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     25),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     27),   \
    _prefix##_##PORT##_##28##_##_postFix,  MARK_PER_PORT_INT_MAC(25), \
    _prefix##_##PORT##_##29##_##_postFix,  MARK_PER_PORT_INT_MAC(27)

#define SET_EVENT_PER_XLG_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix)   \
        SET_EVENT_PER_XLG_PORTS_EVEN__ON_ALL_PORTS_MAC(_prefix,_postFix),  \
        SET_EVENT_PER_XLG_PORTS_ODD__ON_ALL_PORTS_MAC(_prefix,_postFix)

/* state for even ports 24 & 26 with lanes 0..5 that event hold extra port number , lane number */
#define SET_EVENT_PER_XLG_PORTS_EVEN_FOR_6_LANES___ON_ALL_PORTS_MAC(_prefix,_postFix) \
    SET_EVENT_PER_PORT_FOR_6_LANES_MAC(_prefix,_postFix,     24),   \
    SET_EVENT_PER_PORT_FOR_6_LANES_MAC(_prefix,_postFix,     26)

/* Interrupt cause to unified event map for alleyCat5
 This Table is for unified event with extended data

  Array structure:
  1. The first element of the array and the elements after MARK_END_OF_UNI_EV_CNS are unified event types.
  2. Elements after unified event type are pairs of interrupt cause and event extended data,
     until MARK_END_OF_UNI_EV_CNS.
*/
static const GT_U32 alleyCat5UniEvMapTableWithExtData[] = {
 /* Per Queue events */
 CPSS_PP_TX_BUFFER_QUEUE_E,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_BUFFER_QUEUE_0_E,  0,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_BUFFER_QUEUE_1_E,  1,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_BUFFER_QUEUE_2_E,  2,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_BUFFER_QUEUE_3_E,  3,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_BUFFER_QUEUE_4_E,  4,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_BUFFER_QUEUE_5_E,  5,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_BUFFER_QUEUE_6_E,  6,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_BUFFER_QUEUE_7_E,  7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TX_ERR_QUEUE_E,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_ERROR_QUEUE_0_E,  0,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_ERROR_QUEUE_1_E,  1,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_ERROR_QUEUE_2_E,  2,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_ERROR_QUEUE_3_E,  3,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_ERROR_QUEUE_4_E,  4,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_ERROR_QUEUE_5_E,  5,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_ERROR_QUEUE_6_E,  6,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_ERROR_QUEUE_7_E,  7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TX_END_E,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_END_QUEUE_0_E,  0,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_END_QUEUE_1_E,  1,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_END_QUEUE_2_E,  2,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_END_QUEUE_3_E,  3,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_END_QUEUE_4_E,  4,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_END_QUEUE_5_E,  5,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_END_QUEUE_6_E,  6,
    PRV_CPSS_AC5_TRANSMIT_SDMA0_TX_END_QUEUE_7_E,  7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE0_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RX_BUFFER_QUEUE_0_E,     0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE1_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RX_BUFFER_QUEUE_1_E,     1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE2_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RX_BUFFER_QUEUE_2_E,     2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE3_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RX_BUFFER_QUEUE_3_E,     3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE4_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RX_BUFFER_QUEUE_4_E,     4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE5_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RX_BUFFER_QUEUE_5_E,     5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE6_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RX_BUFFER_QUEUE_6_E,     6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE7_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RX_BUFFER_QUEUE_7_E,     7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE0_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RX_ERROR_QUEUE_0_E,      0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE1_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RX_ERROR_QUEUE_1_E,      1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE2_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RX_ERROR_QUEUE_2_E,      2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE3_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RX_ERROR_QUEUE_3_E,      3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE4_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RX_ERROR_QUEUE_4_E,      4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE5_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RX_ERROR_QUEUE_5_E,      5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE6_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RX_ERROR_QUEUE_6_E,      6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE7_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RX_ERROR_QUEUE_7_E,      7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MAC_SFLOW_E,
    SET_EVENT_PER_GIGE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_INGRESS_STC,INGRESS_SAMPLE_LOADED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_EGRESS_SFLOW_E,
    SET_EVENT_PER_GIGE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_EGRESS_STC,E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_AN_COMPLETED_E,
    SET_EVENT_PER_ALL_GIGE_PORTS_AND_CPU__ON_ALL_PORTS_MAC(PRV_CPSS_AC5,AUTO_NEG_COMPLETED_ON_PORT_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
    SET_EVENT_PER_ALL_GIGE_PORTS_AND_CPU__ON_ALL_PORTS_MAC(PRV_CPSS_AC5,LINK_STATUS_CHANGED_E),
    SET_EVENT_PER_XLG_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XLG,LINK_STATUS_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_RX_FIFO_OVERRUN_E,
    SET_EVENT_PER_ALL_GIGE_PORTS_AND_CPU__ON_ALL_PORTS_MAC(PRV_CPSS_AC5,RX_FIFO_OVERRUN_E),
    SET_EVENT_PER_XLG_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XLG,RX_FIFO_OVERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_TX_UNDERRUN_E,
    SET_EVENT_PER_ALL_GIGE_PORTS_AND_CPU__ON_ALL_PORTS_MAC(PRV_CPSS_AC5,TX_UNDERRUN_E),
    SET_EVENT_PER_XLG_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XLG,TX_UNDERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_FC_STATUS_CHANGED_E,
    SET_EVENT_PER_XLG_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XLG,FC_STATUS_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_ILLEGAL_SEQUENCE_E,
    SET_EVENT_PER_XLG_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XLG,UNKNOWN_SEQUENCE_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_FAULT_TYPE_CHANGE_E,
    SET_EVENT_PER_XLG_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XLG,FAULT_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

  CPSS_PP_PORT_ADDRESS_OUT_OF_RANGE_E,
    SET_EVENT_PER_XLG_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XLG,ADDRESS_OUT_OF_RANGE_E),
 MARK_END_OF_UNI_EV_CNS,

  CPSS_PP_PORT_NO_BUFF_PACKET_DROP_E,
    SET_EVENT_PER_XLG_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XLG,NO_BUFF_PACKET_DROP_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_COUNT_COPY_DONE_E,
    SET_EVENT_PER_XLG_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XLG,XG_COUNT_COPY_DONE_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_COUNT_EXPIRED_E,
    SET_EVENT_PER_XLG_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XLG,XG_COUNT_EXPIRED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_SYNC_STATUS_CHANGED_E,
    SET_EVENT_PER_ALL_GIGE_PORTS_AND_CPU__ON_ALL_PORTS_MAC(PRV_CPSS_AC5,SYNC_STATUS_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PRBS_ERROR_E,
    SET_EVENT_PER_ALL_GIGE_PORTS_AND_CPU__ON_ALL_PORTS_MAC(PRV_CPSS_AC5,PRBS_ERROR_ON_PORT_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_TX_FIFO_OVERRUN_E,
    SET_EVENT_PER_ALL_GIGE_PORTS_AND_CPU__ON_ALL_PORTS_MAC(PRV_CPSS_AC5,TX_FIFO_OVERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,
    SET_EVENT_PER_ALL_GIGE_PORTS_AND_CPU__ON_ALL_PORTS_MAC(PRV_CPSS_AC5,TX_FIFO_UNDERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TQ_TXQ2_FLUSH_PORT_E,
    SET_EVENT_PER_GIGE_PORTS_AND_CPU__ON_ALL_PORTS_MAC(PRV_CPSS_AC5_TXQ_FLUSH_DONE,E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TQ_PORT_DESC_FULL_E,
    SET_EVENT_PER_GIGE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_TXQ_DESC_FULL_SUMMARY_DESC_FULL,E),
 MARK_END_OF_UNI_EV_CNS,

 /* Per Port XPCS events */
 CPSS_PP_PORT_PCS_LINK_STATUS_CHANGED_E,
    SET_EVENT_PER_XLG_PORTS_EVEN__ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XPCS,GLOBAL_LINK_STATUS_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_DESKEW_TIMEOUT_E,
    SET_EVENT_PER_XLG_PORTS_EVEN__ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XPCS,GLOBAL_DESKEW_TIMEOUT_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_DETECTED_COLUMN_IIAII_E,
    SET_EVENT_PER_XLG_PORTS_EVEN__ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XPCS,GLOBAL_DETECTED_COLUMN_IIAII_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_DESKEW_ERROR_E,
    SET_EVENT_PER_XLG_PORTS_EVEN__ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XPCS,GLOBAL_DESKEW_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_PPM_FIFO_UNDERRUN_E,
    SET_EVENT_PER_XLG_PORTS_EVEN__ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XPCS,GLOBAL_PPM_FIFO_UNDERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_PPM_FIFO_OVERRUN_E,
    SET_EVENT_PER_XLG_PORTS_EVEN__ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XPCS,GLOBAL_PPM_FIFO_OVERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
    SET_EVENT_PER_XLG_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_PCS40_G,COMMON_SIGNAL_DETECT_CHANGE_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
    SET_EVENT_PER_XLG_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_PCS40_G,COMMON_ALIGN_LOCK_LOST_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E,
    SET_EVENT_PER_XLG_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_PCS40_G,COMMON_GB_LOCK_SYNC_CHANGE_E),
 MARK_END_OF_UNI_EV_CNS,

    /* Per Port Per lane XPCS events */
 CPSS_PP_PORT_LANE_PRBS_ERROR_E,
    SET_EVENT_PER_XLG_PORTS_EVEN_FOR_6_LANES___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XPCS,PRBS_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_DISPARITY_ERROR_E,
    SET_EVENT_PER_XLG_PORTS_EVEN_FOR_6_LANES___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XPCS,DISPARITY_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_SYMBOL_ERROR_E,
    SET_EVENT_PER_XLG_PORTS_EVEN_FOR_6_LANES___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XPCS,SYMBOL_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_CJR_PAT_ERROR_E,
    SET_EVENT_PER_XLG_PORTS_EVEN_FOR_6_LANES___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XPCS,CJR_PAT_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,
    SET_EVENT_PER_XLG_PORTS_EVEN_FOR_6_LANES___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XPCS,SIGNAL_DETECT_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_SYNC_STATUS_CHANGED_E,
    SET_EVENT_PER_XLG_PORTS_EVEN_FOR_6_LANES___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XPCS,SYNC_STATUS_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_DETECTED_IIAII_E,
    SET_EVENT_PER_XLG_PORTS_EVEN_FOR_6_LANES___ON_ALL_PORTS_MAC(PRV_CPSS_AC5_XPCS,DETECTED_IIAII_E),
 MARK_END_OF_UNI_EV_CNS,

  /* Per Index events */
 CPSS_PP_GPP_E,
    PRV_CPSS_AC5_GPPGPP_0_E,   0,
    PRV_CPSS_AC5_GPPGPP_1_E,   1,
    PRV_CPSS_AC5_GPPGPP_2_E,   2,
    PRV_CPSS_AC5_GPPGPP_3_E,   3,
    PRV_CPSS_AC5_GPPGPP_4_E,   4,
    PRV_CPSS_AC5_GPPGPP_5_E,   5,
    PRV_CPSS_AC5_GPPGPP_6_E,   6,
    PRV_CPSS_AC5_GPPGPP_7_E,   7,
    PRV_CPSS_AC5_GPPGPP_8_E,   8,
    PRV_CPSS_AC5_GPPGPP_9_E,   9,
    PRV_CPSS_AC5_GPPGPP_10_E, 10,
    PRV_CPSS_AC5_GPPGPP_11_E, 11,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_CNC_WRAPAROUND_BLOCK_E,
    PRV_CPSS_AC5_CNC_BLOCK0_WRAPAROUND_E, 0,
    PRV_CPSS_AC5_CNC_BLOCK1_WRAPAROUND_E, 1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_BM_MISC_E,
    PRV_CPSS_AC5_BM_PORT_RX_FULL_E,           4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_CNTR_OVERFLOW_E,
    PRV_CPSS_AC5_RECEIVE_SDMA0_RESOURCE_ERROR_CNT_OF_E,  0,
    PRV_CPSS_AC5_RECEIVE_SDMA0_BYTE_CNT_OF_E,            1,
    PRV_CPSS_AC5_RECEIVE_SDMA0_PACKET_CNT_OF_E,          2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TQ_MISC_E,
    PRV_CPSS_AC5_TXQ_GENERAL_BAD_ADDR_E,      0,
    PRV_CPSS_AC5_TXQ_GENERAL_DESC_FULL_E,     1,
    PRV_CPSS_AC5_TXQ_GENERAL_BUFFER_FULL_E,   2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_SCT_RATE_LIMITER_E,
    SET_EVENT_PER_INDEX____1_31___MAC(PRV_CPSS_AC5_CPU_CODE_RATE_LIMITERS_STC_RATE_LIMITER,PKT_DROPED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E,
    PRV_CPSS_AC5_POLICER_IPLR0_ADDRESS_OUT_OF_MEMORY_E,    0,
    PRV_CPSS_AC5_POLICER_EPLR_ADDRESS_OUT_OF_MEMORY_E,     1,
    PRV_CPSS_AC5_POLICER_IPLR1_ADDRESS_OUT_OF_MEMORY_E,    2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_DATA_ERR_E,
    /* PRV_CPSS_AC5_POLICER_IPLR0_DATA_ERROR_E,    0, --> bound in CPSS_PP_DATA_INTEGRITY_ERROR_E (like in Lion2) */
    PRV_CPSS_AC5_POLICER_EPLR_DATA_ERROR_E,     1,
    /* PRV_CPSS_AC5_POLICER_IPLR1_DATA_ERROR_E,    2, --> bound in CPSS_PP_DATA_INTEGRITY_ERROR_E (like in Lion2)*/
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E,
    PRV_CPSS_AC5_POLICER_IPLR0_IPFIX_WRAP_AROUND_E,    0,
    PRV_CPSS_AC5_POLICER_EPLR_IPFIX_WRAP_AROUND_E,     1,
    PRV_CPSS_AC5_POLICER_IPLR1_IPFIX_WRAP_AROUND_E,    2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_IPFIX_ALARM_E,
    PRV_CPSS_AC5_POLICER_IPLR0_IPFIX_ALARM_E,    0,
    PRV_CPSS_AC5_POLICER_EPLR_IPFIX_ALARM_E,     1,
    PRV_CPSS_AC5_POLICER_IPLR1_IPFIX_ALARM_E,    2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,
    PRV_CPSS_AC5_POLICER_IPLR0_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,    0,
    PRV_CPSS_AC5_POLICER_EPLR_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,     1,
    PRV_CPSS_AC5_POLICER_IPLR1_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,    2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,
    PRV_CPSS_AC5_POLICY_ENGINE_PCL0_TCAM_ACCESS_DATA_ERROR_E,         0,
    PRV_CPSS_AC5_POLICY_ENGINE_PCL0_ACTION1_TCAM_ACCESS_DATA_ERROR_E, 0,
    PRV_CPSS_AC5_POLICY_ENGINE_PCL0_ACTION2_TCAM_ACCESS_DATA_ERROR_E, 0,
    PRV_CPSS_AC5_POLICY_ENGINE_PCL0_ACTION3_TCAM_ACCESS_DATA_ERROR_E, 0,

    PRV_CPSS_AC5_POLICY_ENGINE_PCL0_1_TCAM_ACCESS_DATA_ERROR_E,       1,
    PRV_CPSS_AC5_POLICY_ENGINE_PCL1_ACTION1_TCAM_ACCESS_DATA_ERROR_E, 1,
    PRV_CPSS_AC5_POLICY_ENGINE_PCL1_ACTION2_TCAM_ACCESS_DATA_ERROR_E, 1,
    PRV_CPSS_AC5_POLICY_ENGINE_PCL1_ACTION3_TCAM_ACCESS_DATA_ERROR_E, 1,

    PRV_CPSS_AC5_POLICY_ENGINE_PCL1_TCAM_ACCESS_DATA_ERROR_E,         2,
    PRV_CPSS_AC5_POLICY_ENGINE_PCL2_ACTION1_TCAM_ACCESS_DATA_ERROR_E, 2,
    PRV_CPSS_AC5_POLICY_ENGINE_PCL2_ACTION2_TCAM_ACCESS_DATA_ERROR_E, 2,
    PRV_CPSS_AC5_POLICY_ENGINE_PCL2_ACTION3_TCAM_ACCESS_DATA_ERROR_E, 2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PCL_LOOKUP_FIFO_FULL_E,
    PRV_CPSS_AC5_POLICY_ENGINE_PCL0_FIFO_FULL_E,   0,
    PRV_CPSS_AC5_POLICY_ENGINE_PCL0_1_FIFO_FULL_E, 1,
    PRV_CPSS_AC5_POLICY_ENGINE_PCL1_FIFO_FULL_E,   2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_CM3_WD_E,
    PRV_CPSS_AC5_MG_0_MG1_INTERNAL_INTERRUPT_CAUSE_CM3_SECOND_TIMER_INTERRUPT_E,  0,
    PRV_CPSS_AC5_MG_1_MG1_INTERNAL_INTERRUPT_CAUSE_CM3_SECOND_TIMER_INTERRUPT_E,  1,
    PRV_CPSS_AC5_MG_2_MG1_INTERNAL_INTERRUPT_CAUSE_CM3_SECOND_TIMER_INTERRUPT_E,  2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_CM3_DOORBELL_E,
    PRV_CPSS_AC5_MG_0_MG_INTERNAL_INTERRUPT_CAUSE_CM3_TO_HOST_DOORBELL_INTERRUPT_E,  0,
    PRV_CPSS_AC5_MG_1_MG_INTERNAL_INTERRUPT_CAUSE_CM3_TO_HOST_DOORBELL_INTERRUPT_E,  1,
    PRV_CPSS_AC5_MG_2_MG_INTERNAL_INTERRUPT_CAUSE_CM3_TO_HOST_DOORBELL_INTERRUPT_E,  2,
 MARK_END_OF_UNI_EV_CNS,

 /* next HW interrupt value will have same index (evExtData) value in CPSS_PP_CRITICAL_HW_ERROR_E */
 CPSS_PP_CRITICAL_HW_ERROR_E,
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_BM_EMPTY_CLEAR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_BM_MC_INC_OVERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_BM_MC_DEC_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_BM_MC_CNT_PARITY_ERR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_DATA_INTEGRITY_ERROR_E,
    /* -- _ECC_ --*/

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_BM_1_BANK0_ONE_ECC_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_BM_1_BANK0_TWO_OR_MORE_ECC_ERRORS_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_BM_1_BANK1_ONE_ECC_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_BM_1_BANK1_TWO_OR_MORE_ECC_ERRORS_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_TXQ_GENERAL_FLL_UNFIXED_ECC_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_TXQ_GENERAL_FLL_FIXED_ECC_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_TXQ_GENERAL_DESC_LL_UNFIXED_ECC_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_TXQ_GENERAL_DESC_LL_FIXED_ECC_ERROR_E),

    /* TCAM */
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_TCC_UPPER_TCAM_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_TCC_LOWER_TCAM_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_TCC1_LOWER_TCAM_ERROR_DETECTED_E),

    /* DFX Interrupts. AC5 has two DFX Pipes. Need to connect to both. */
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_AC5_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E),

 MARK_END_OF_UNI_EV_CNS,
};

/* Interrupt cause to unified event map for alleyCat5 with extended data size */
static const GT_U32 alleyCat5UniEvMapTableWithExtDataSize = (sizeof(alleyCat5UniEvMapTableWithExtData)/(sizeof(GT_U32)));


/* Interrupt cause to unified event map for alleyCat5
 This Table is for unified event without extended data
*/
static const GT_U32 alleyCat5UniEvMapTable[][2] =
{
 {CPSS_PP_MISC_TWSI_TIME_OUT_E,          PRV_CPSS_AC5_MG_INTERNAL_TWSI_TIME_OUT_E},
 {CPSS_PP_MISC_TWSI_STATUS_E,            PRV_CPSS_AC5_MG_INTERNAL_TWSI_STATUS_E},
 {CPSS_PP_MISC_ILLEGAL_ADDR_E,           PRV_CPSS_AC5_MG_INTERNAL_ILLEGAL_ADDR_E},
 {CPSS_PP_BM_AGED_PACKET_E,              PRV_CPSS_AC5_BM_AGED_PACKET_E},
 {CPSS_PP_BM_WRONG_SRC_PORT_E,           PRV_CPSS_AC5_BM_WRONG_SRC_PORT_E},
 {CPSS_PP_TQ_SNIFF_DESC_DROP_E,          PRV_CPSS_AC5_TXQ_GENERAL_EGRESS_MIRORR_DESC_DROP_E},
 {CPSS_PP_EB_SECURITY_BREACH_UPDATE_E,   PRV_CPSS_AC5_BRIDGE_UPDATE_SECURITY_BREACH_E},
 {CPSS_PP_MAC_NUM_OF_HOP_EXP_E,          PRV_CPSS_AC5_FDB_NUM_OF_HOP_EX_P_E},
 {CPSS_PP_MAC_NA_LEARNED_E,              PRV_CPSS_AC5_FDB_NA_LEARNT_E},
 {CPSS_PP_MAC_NA_NOT_LEARNED_E,          PRV_CPSS_AC5_FDB_NA_NOT_LEARNT_E},
 {CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E,   PRV_CPSS_AC5_FDB_AGE_VIA_TRIGGER_ENDED_E},
 {CPSS_PP_MAC_UPDATE_FROM_CPU_DONE_E,    PRV_CPSS_AC5_FDBAU_PROC_COMPLETED_E},
 {CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E,    PRV_CPSS_AC5_FDBAU_MSG_TOCPU_READY_E},
 {CPSS_PP_MAC_NA_SELF_LEARNED_E,         PRV_CPSS_AC5_FDBNA_SELF_LEARNED_E},
 {CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E,     PRV_CPSS_AC5_FDBN_AFROM_CPU_LEARNED_E},
 {CPSS_PP_MAC_NA_FROM_CPU_DROPPED_E,     PRV_CPSS_AC5_FDBN_AFROM_CPU_DROPPED_E},
 {CPSS_PP_MAC_AGED_OUT_E,                PRV_CPSS_AC5_FDB_AGED_OUT_E},
 {CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E,     PRV_CPSS_AC5_FDBAU_FIFO_TO_CPU_IS_FULL_E},
 {CPSS_PP_EB_AUQ_PENDING_E,              PRV_CPSS_AC5_MG_INTERNAL_AUQ_PENDING_E},
 {CPSS_PP_EB_AUQ_FULL_E,                 PRV_CPSS_AC5_MG_INTERNAL_AU_QUEUE_FULL_E},
 {CPSS_PP_EB_AUQ_OVER_E,                 PRV_CPSS_AC5_MG_INTERNAL_AUQ_OVERRUN_E},
 {CPSS_PP_EB_AUQ_ALMOST_FULL_E,          PRV_CPSS_AC5_MG_INTERNAL_AUQ_ALMOST_FULL_E},
 {CPSS_PP_EB_FUQ_PENDING_E,              PRV_CPSS_AC5_MG_INTERNAL_FUQ_PENDING_E},
 {CPSS_PP_EB_FUQ_FULL_E,                 PRV_CPSS_AC5_MG_INTERNAL_FU_QUEUE_FULL_E},
 {CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,    PRV_CPSS_AC5_MG_INTERNAL_GENXS_READ_DMA_DONE_E},
 {CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E,      PRV_CPSS_AC5_MG_INTERNAL_PEX_ADDR_UNMAPPED_E},
 {CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E,     PRV_CPSS_AC5_BRIDGE_ADDRESS_OUT_OF_RANGE_E},
 {CPSS_PP_CNC_DUMP_FINISHED_E,           PRV_CPSS_AC5_CNC_DUMP_FINISHED_E},
 {CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E,PRV_CPSS_AC5_TTI_CPU_ADDRESS_OUT_OF_RANGE_E},
 {CPSS_PP_TTI_ACCESS_DATA_ERROR_E,       PRV_CPSS_AC5_TTI_ACCESS_DATA_ERROR_E},
 {CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E,    PRV_CPSS_AC5_POLICY_ENGINE_MG_ADDR_OUT_OF_RANGE_E},
 {CPSS_PP_PORT_802_3_AP_E,               PRV_CPSS_AC5_MG_INTERNAL_Z80_INTERRUPT_E}
};

/* Interrupt cause to unified event map for Bobcat2 without extended data size */
static const GT_U32 alleyCat5UniEvMapTableSize = (sizeof(alleyCat5UniEvMapTable)/(sizeof(GT_U32)*2));


/**
* @internal setAlleyCat5DedicatedEventsConvertInfo function
* @endinternal
*
* @brief   set info needed by chIntCauseToUniEvConvertDedicatedTables(...) for the
*         alleyCat5 devices.
*/
static void setAlleyCat5DedicatedEventsConvertInfo(IN GT_U8   devNum)
{
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableWithExtDataArr   = alleyCat5UniEvMapTableWithExtData;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableWithExtDataSize  = alleyCat5UniEvMapTableWithExtDataSize;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableArr              = alleyCat5UniEvMapTable;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableSize             = alleyCat5UniEvMapTableSize;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.numReservedPorts              = 24;

    return;
}


/**
* @internal prvCpssDrvPpIntDefDxChAlleyCat5Init function
* @endinternal
*
* @brief   Interrupts initialization for the DxChAlleyCat5 devices.
*
* @param[in] devNum                - the device number
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefDxChAlleyCat5Init
(
    IN  GT_U8   devNum
)
{
    GT_STATUS   rc;
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC *devFamilyPtr;    /* pointer to device interrupts info */
    PRV_CPSS_DRV_INTERRUPTS_INFO_STC devFamilyInterrupstInfo =
    {
        NULL,
        PRV_CPSS_DRV_FAMILY_DXCH_AC5_E,
        AC5_NUM_ELEMENTS_IN_SCAN_TREE_CNS,
        alleyCat5IntrScanArr,
        AC5_NUM_MASK_REGISTERS_CNS,
        NULL, NULL, NULL
    };

    /*set info needed by chIntCauseToUniEvConvertDedicatedTables(...) */
    setAlleyCat5DedicatedEventsConvertInfo(devNum);

    if(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(ac5InitDone) == GT_TRUE)
    {
        return GT_OK;
    }

    rc = prvCpssDrvDxExMxInterruptsMemoryInit(&devFamilyInterrupstInfo, 1);
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    /*
       1. fill the array of mask registers addresses
       2. fill the array of default values for the mask registers
       3. update the bits of nonSumBitMask in the scan tree
    */

    rc = prvCpssDrvExMxDxHwPpMaskRegInfoGet(
            CPSS_PP_FAMILY_DXCH_AC5_E,
            AC5_NUM_ELEMENTS_IN_SCAN_TREE_CNS,
            devFamilyInterrupstInfo.intrScanOutArr,
            AC5_NUM_MASK_REGISTERS_CNS,
            devFamilyInterrupstInfo.maskRegDefaultSummaryArr,
            devFamilyInterrupstInfo.maskRegMapArr);
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    devFamilyPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_AC5_E]);

    devFamilyPtr->numOfInterrupts = PRV_CPSS_AC5_LAST_INT_E;
    devFamilyPtr->maskRegistersAddressesArray = devFamilyInterrupstInfo.maskRegMapArr;
    devFamilyPtr->maskRegistersDefaultValuesArray = devFamilyInterrupstInfo.maskRegDefaultSummaryArr;
    devFamilyPtr->interruptsScanArray = devFamilyInterrupstInfo.intrScanOutArr;
    devFamilyPtr->numOfScanElements = AC5_NUM_ELEMENTS_IN_SCAN_TREE_CNS;
    devFamilyPtr->fdbTrigEndedId = 0;/* don't care !!!! */
    devFamilyPtr->fdbTrigEndedCbPtr = NULL;
    devFamilyPtr->hasFakeInterrupts = GT_FALSE;
    devFamilyPtr->firstFakeInterruptId = 0;/* don't care */
    devFamilyPtr->drvIntCauseToUniEvConvertFunc = &prvCpssDrvPpPortGroupIntCheetahIntCauseToUniEvConvert;
    devFamilyPtr->numOfInterruptRegistersNotAccessibleBeforeStartInit = 0;
    devFamilyPtr->notAccessibleBeforeStartInitPtr=NULL;

    PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(ac5InitDone, GT_TRUE);

    return GT_OK;
}

#ifdef DUMP_DEFAULT_INFO
/**
* @internal prvCpssDrvPpIntDefAlleyCat5Print function
* @endinternal
*
* @brief   print the interrupts arrays info, of alleyCat5 devices
*/
void  prvCpssDrvPpIntDefAlleyCat5Print(
    void
)
{
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC *  interruptsScanArray;

    cpssOsPrintf("AC5 - start : \n");
    devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_AC5_E]);
    interruptsScanArray = devInterruptInfoPtr->interruptsScanArray;

    prvCpssDrvPpIntDefPrint(AC5_NUM_ELEMENTS_IN_SCAN_TREE_CNS, interruptsScanArray,
                            AC5_NUM_MASK_REGISTERS_CNS,
                            devInterruptInfoPtr->maskRegistersAddressesArray,
                            devInterruptInfoPtr->maskRegistersDefaultValuesArray);

    cpssOsPrintf("AC5 - End : \n");

}
#endif/*DUMP_DEFAULT_INFO*/
