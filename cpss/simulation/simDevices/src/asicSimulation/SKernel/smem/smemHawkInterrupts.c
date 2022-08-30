/******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* smemHawkInterrupts.c
*
* DESCRIPTION:
*       Hawk interrupt tree related implementation
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <asicSimulation/SKernel/smem/smemHawk.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>


extern MAC_NUM_INFO_STC hawk100GPortsArr[];
extern MAC_NUM_INFO_STC hawkUsxPortsArr[];

/* Hawk interrupt tree database */
static SKERNEL_INTERRUPT_REG_INFO_STC hawkInterruptsTreeDb[MAX_INTERRUPT_NODES];

static SKERNEL_INTERRUPT_REG_INFO_STC skernel_interrupt_reg_info__last_line__cns[] =
{{{SKERNEL_REGISTERS_DB_TYPE__LAST___E}}};


#define cnm_grp_0_0_intr  (hawk_FuncUnitsInterruptsSummary  [0])
#define cnm_grp_0_1_intr  (hawk_dataPathInterruptSummary    [0])
#define cnm_grp_0_2_intr  (hawk_portsInterruptsSummary      [0])
#define cnm_grp_0_3_intr  (hawk_dfxInterruptsSummary        [0])
#define cnm_grp_0_4_intr  (hawk_dfx1InterruptsSummary       [0])

#define cnm_grp_0_7_intr  (hawk_ports2InterruptsSummary     [0])

#define cnm_grp_0_0_intr_x(x)  (hawk_FuncUnitsInterruptsSummary  [x])


enum{  /*cnm_grp_0_0_intr  - functionalInterruptsSummaryCause/FuncUnitsInterruptsSummary */
     cp_top_em_int_sum                                                      =  1,
     cp_top_epcl_ha_macro_tai_interrupt_cause_int_sum_0                     =  2,
     cp_top_epcl_ha_macro_tai_interrupt_cause_int_sum_1                     =  3,
     cp_top_mt_int_sum                                                      =  4,
     cp_top_pha_macro_tai_interrupt_cause_int_sum_0                         =  5,
     cp_top_pha_macro_tai_interrupt_cause_int_sum_1                         =  6,
     cp_top_pipe0_bma_int_sum                                               =  7,
     cp_top_pipe0_cnc0_int_sum                                              =  8,
     cp_top_pipe0_cnc1_int_sum                                              =  9,
     cp_top_pipe0_cnc2_int_sum                                              = 10,
     cp_top_pipe0_cnc3_int_sum                                              = 11,
     cp_top_pipe0_eft_int_sum                                               = 12,
     cp_top_pipe0_eoam_int_sum                                              = 13,
     cp_top_pipe0_epcl_int_sum                                              = 14,
     cp_top_pipe0_eplr_int_sum                                              = 15,
     cp_top_pipe0_eq_int_sum                                                = 16,
     cp_top_pipe0_erep_int_sum                                              = 17,
     cp_top_pipe0_ermrk_int_sum                                             = 18,
     cp_top_pipe0_ha_int_sum                                                = 19,
     cp_top_pipe0_hbu_int_sum                                               = 20,
     cp_top_pipe0_ioam_int_sum                                              = 21,
     cp_top_pipe0_iplr0_int_sum                                             = 22,
     cp_top_pipe0_iplr1_int_sum                                             = 23,
     cp_top_pipe0_ipvx_int_sum                                              = 24,
     cp_top_pipe0_l2i_int_sum                                               = 25,
     cp_top_pipe0_lpm_int_sum                                               = 26,
     cp_top_pipe0_mll_int_sum                                               = 27,
     cp_top_pipe0_pcl_int_sum                                               = 28,
     cp_top_pipe0_ppu_int_sum                                               = 29,
     cp_top_pipe0_preq_int_sum                                              = 30,
     cp_top_pipe0_qag_int_sum                                               = 31
};


enum{ /*cnm_grp_0_1_intr_ - dataPathInterruptSummary */
    cp_top_pipe0_sht_int_sum                                                =  1,
    cp_top_pipe0_sip_pha_int_sum                                            =  2,
    cp_top_pipe0_tti_int_sum                                                =  3,
    cp_top_shm_int_sum                                                      =  4,
    cp_top_tcam_int_sum                                                     =  5,
    cp_top_tti_macro_tai_interrupt_cause_int_sum_0                          =  6,
    cp_top_tti_macro_tai_interrupt_cause_int_sum_1                          =  7,
    dp_tile_top_ia0_ia_interrupt_0_cause_int_sum                            =  8,
    dp_tile_top_rx0_rxdma_cause_int_sum                                     =  9,
    dp_tile_top_rx1_rxdma_cause_int_sum                                     = 10,
    dp_tile_top_rx2_rxdma_cause_int_sum                                     = 11,
    dp_tile_top_rx3_rxdma_cause_int_sum                                     = 12,
    dp_tile_top_tx0_txd_interrupt_cause_sum                                 = 13,
    dp_tile_top_tx0_txf_interrupt_cause_sum                                 = 14,
    dp_tile_top_tx1_txd_interrupt_cause_sum                                 = 15,
    dp_tile_top_tx1_txf_interrupt_cause_sum                                 = 16,
    dp_tile_top_tx2_txd_interrupt_cause_sum                                 = 17,
    dp_tile_top_tx2_txf_interrupt_cause_sum                                 = 18,
    dp_tile_top_tx3_txd_interrupt_cause_sum                                 = 19,
    dp_tile_top_tx3_txf_interrupt_cause_sum                                 = 20,
    gop_downlink_top_epi0_anp_int_sum_0                                     = 21,
    gop_downlink_top_epi0_anp_int_sum_1                                     = 22,
    gop_downlink_top_epi0_anp_int_sum_2                                     = 23,
    gop_downlink_top_epi0_anp_int_sum_3                                     = 24,
    gop_downlink_top_epi0_anp_int_sum_4                                     = 25,
    gop_downlink_top_epi0_anp_int_sum_5                                     = 26,
    gop_downlink_top_epi0_anp_int_sum_6                                     = 27,
    gop_downlink_top_epi0_hawk_epi_400g_mac_tai_interrupt_cause_int_sum_0   = 28,
    gop_downlink_top_epi0_hawk_epi_400g_mac_tai_interrupt_cause_int_sum_1   = 29,
    gop_downlink_top_epi0_hawk_epi_usx_mac_tai_interrupt_cause_int_sum_0    = 30,
    gop_downlink_top_epi0_hawk_epi_usx_mac_tai_interrupt_cause_int_sum_1    = 31
};

enum{ /*cnm_grp_0_2_intr_ - portsInterruptSummary */
    gop_downlink_top_epi0_leds_int_sum_0                                    =  1,
    gop_downlink_top_epi0_leds_int_sum_1                                    =  2,
    gop_downlink_top_epi0_mac_pcs_int_sum_0                                 =  3,
    gop_downlink_top_epi0_mac_pcs_int_sum_1                                 =  4,
    gop_downlink_top_epi0_mac_pcs_int_sum_2                                 =  5,
    gop_downlink_top_epi0_mac_pcs_int_sum_3                                 =  6,
    gop_downlink_top_epi0_mif_int_sum_0                                     =  7,
    gop_downlink_top_epi0_mif_int_sum_1                                     =  8,
    gop_downlink_top_epi0_mif_int_sum_2                                     =  9,
    gop_downlink_top_epi0_sdw_int_sum_0                                     = 10,
    gop_downlink_top_epi0_sdw_int_sum_1                                     = 11,
    gop_downlink_top_epi1_anp_int_sum_0                                     = 12,
    gop_downlink_top_epi1_anp_int_sum_1                                     = 13,
    gop_downlink_top_epi1_anp_int_sum_2                                     = 14,
    gop_downlink_top_epi1_anp_int_sum_3                                     = 15,
    gop_downlink_top_epi1_anp_int_sum_4                                     = 16,
    gop_downlink_top_epi1_anp_int_sum_5                                     = 17,
    gop_downlink_top_epi1_anp_int_sum_6                                     = 18,
    gop_downlink_top_epi1_anp_int_sum_7                                     = 19,
    gop_downlink_top_epi1_hawk_cpu_port_tai_interrupt_cause_int_sum_0       = 20,
    gop_downlink_top_epi1_hawk_cpu_port_tai_interrupt_cause_int_sum_1       = 21,
    gop_downlink_top_epi1_hawk_epi_400g_mac_tai_interrupt_cause_int_sum_0   = 22,
    gop_downlink_top_epi1_hawk_epi_400g_mac_tai_interrupt_cause_int_sum_1   = 23,
    gop_downlink_top_epi1_hawk_epi_usx_mac_tai_interrupt_cause_int_sum_0    = 24,
    gop_downlink_top_epi1_hawk_epi_usx_mac_tai_interrupt_cause_int_sum_1    = 25,
    gop_downlink_top_epi1_leds_int_sum_0                                    = 26,
    gop_downlink_top_epi1_leds_int_sum_1                                    = 27,
    gop_downlink_top_epi1_leds_int_sum_2                                    = 28,
    gop_downlink_top_epi1_mac_pcs_int_sum_0                                 = 29,
    gop_downlink_top_epi1_mac_pcs_int_sum_1                                 = 30,
    gop_downlink_top_epi1_mac_pcs_int_sum_2                                 = 31
};

enum{  /*cnm_grp_0_3_intr  - dfxInterruptsSummaryCause */
    gop_downlink_top_epi1_mac_pcs_int_sum_3                                 =  1,
    gop_downlink_top_epi1_mac_pcs_int_sum_4                                 =  2,
    gop_downlink_top_epi1_mif_int_sum_0                                     =  3,
    gop_downlink_top_epi1_mif_int_sum_1                                     =  4,
    gop_downlink_top_epi1_mif_int_sum_2                                     =  5,
    gop_downlink_top_epi1_sdw_int_sum_0                                     =  6,
    gop_downlink_top_epi1_sdw_int_sum_1                                     =  7,
    gop_downlink_top_epi1_sdw_int_sum_2                                     =  8,
    gop_downlink_top_pca0_ctsu_interrupt_cause_int_sum                      =  9,
    gop_downlink_top_pca0_dp2sdb_interrupt_cause_int_sum                    = 10,
    gop_downlink_top_pca0_eip_163e_interrupt_cause_int_sum                  = 11,
    gop_downlink_top_pca0_eip_163i_interrupt_cause_int_sum                  = 12,
    gop_downlink_top_pca0_eip_164e_interrupt_cause_int_sum                  = 13,
    gop_downlink_top_pca0_eip_164i_interrupt_cause_int_sum                  = 14,
    gop_downlink_top_pca0_lmu_interrupt_cause_int_sum                       = 15,
    gop_downlink_top_pca0_msec_egr_163_tai_int_sum_0                        = 16,
    gop_downlink_top_pca0_msec_egr_163_tai_int_sum_1                        = 17,
    gop_downlink_top_pca0_pzarb_rx_int_sum                                  = 18,
    gop_downlink_top_pca0_pzarb_tx_int_sum                                  = 19,
    gop_downlink_top_pca0_sff2irq_int_sum                                   = 20,
    gop_downlink_top_pca1_ctsu_interrupt_cause_int_sum                      = 21,
    gop_downlink_top_pca1_dp2sdb_interrupt_cause_int_sum                    = 22,
    gop_downlink_top_pca1_eip_163e_interrupt_cause_int_sum                  = 23,
    gop_downlink_top_pca1_eip_163i_interrupt_cause_int_sum                  = 24,
    gop_downlink_top_pca1_eip_164e_interrupt_cause_int_sum                  = 25,
    gop_downlink_top_pca1_eip_164i_interrupt_cause_int_sum                  = 26,
    gop_downlink_top_pca1_lmu_interrupt_cause_int_sum                       = 27,
    gop_downlink_top_pca1_msec_egr_163_tai_int_sum_0                        = 28,
    gop_downlink_top_pca1_msec_egr_163_tai_int_sum_1                        = 29,
    gop_downlink_top_pca1_pzarb_rx_int_sum                                  = 30,
    gop_downlink_top_pca1_pzarb_tx_int_sum                                  = 31
};

enum{  /*cnm_grp_0_4_intr  - dfx1InterruptsSummaryCause */
    gop_downlink_top_pca1_sff2irq_int_sum                                   =  1,
    gop_uplink_top_epi2_anp_int_sum                                         =  2,
    gop_uplink_top_epi2_hawk_epi_400g_mac_tai_interrupt_cause_int_sum_0     =  3,
    gop_uplink_top_epi2_hawk_epi_400g_mac_tai_interrupt_cause_int_sum_1     =  4,
    gop_uplink_top_epi2_leds_int_sum                                        =  5,
    gop_uplink_top_epi2_mac_pcs_int_sum                                     =  6,
    gop_uplink_top_epi2_mif_int_sum                                         =  7,
    gop_uplink_top_epi2_sdw_int_sum_0                                       =  8,
    gop_uplink_top_epi2_sdw_int_sum_1                                       =  9,
    gop_uplink_top_epi3_anp_int_sum                                         = 10,
    gop_uplink_top_epi3_hawk_epi_400g_mac_tai_interrupt_cause_int_sum_0     = 11,
    gop_uplink_top_epi3_hawk_epi_400g_mac_tai_interrupt_cause_int_sum_1     = 12,
    gop_uplink_top_epi3_leds_int_sum                                        = 13,
    gop_uplink_top_epi3_mac_pcs_int_sum                                     = 14,
    gop_uplink_top_epi3_mif_int_sum                                         = 15,
    gop_uplink_top_epi3_sdw_int_sum_0                                       = 16,
    gop_uplink_top_epi3_sdw_int_sum_1                                       = 17,
    gop_uplink_top_pca2_ctsu_interrupt_cause_int_sum                        = 18,
    gop_uplink_top_pca2_dp2sdb_interrupt_cause_int_sum                      = 19,
    gop_uplink_top_pca2_eip_163e_interrupt_cause_int_sum                    = 20,
    gop_uplink_top_pca2_eip_163i_interrupt_cause_int_sum                    = 21,
    gop_uplink_top_pca2_eip_164e_interrupt_cause_int_sum                    = 22,
    gop_uplink_top_pca2_eip_164i_interrupt_cause_int_sum                    = 23,
    gop_uplink_top_pca2_lmu_interrupt_cause_int_sum                         = 24,
    gop_uplink_top_pca2_pzarb_rx_int_sum                                    = 25,
    gop_uplink_top_pca2_pzarb_tx_int_sum                                    = 26,
    gop_uplink_top_pca2_sff2irq_int_sum                                     = 27,
    gop_uplink_top_pca2_tai_int_sum_0                                       = 28,
    gop_uplink_top_pca2_tai_int_sum_1                                       = 29,
    gop_uplink_top_pca3_ctsu_interrupt_cause_int_sum                        = 30,
    gop_uplink_top_pca3_dp2sdb_interrupt_cause_int_sum                      = 31
};

enum{  /*cnm_grp_0_5_intr  - dfx1InterruptsSummaryCause */
    gop_uplink_top_pca3_eip_163e_interrupt_cause_int_sum                    =  1,
    gop_uplink_top_pca3_eip_163i_interrupt_cause_int_sum                    =  2,
    gop_uplink_top_pca3_eip_164e_interrupt_cause_int_sum                    =  3,
    gop_uplink_top_pca3_eip_164i_interrupt_cause_int_sum                    =  4,
    gop_uplink_top_pca3_lmu_interrupt_cause_int_sum                         =  5,
    gop_uplink_top_pca3_pzarb_rx_int_sum                                    =  6,
    gop_uplink_top_pca3_pzarb_tx_int_sum                                    =  7,
    gop_uplink_top_pca3_sff2irq_int_sum                                     =  8,
    gop_uplink_top_pca3_tai_int_sum_0                                       =  9,
    gop_uplink_top_pca3_tai_int_sum_1                                       = 10,
    pb_tile_top_pb_int_sum                                                  = 11,
    server_macro_server_int_sum                                             = 12,
    txq_top_pds0_int_sum                                                    = 13,
    txq_top_pds1_int_sum                                                    = 14,
    txq_top_pds2_int_sum                                                    = 15,
    txq_top_pds3_int_sum                                                    = 16,
    txq_top_pdx_interrupt_sum                                               = 17,
    txq_top_pfcc_int_sum                                                    = 18,
    txq_top_psi_int_sum                                                     = 19,
    txq_top_qfc0_int_sum                                                    = 20,
    txq_top_qfc1_int_sum                                                    = 21,
    txq_top_qfc2_int_sum                                                    = 22,
    txq_top_qfc3_int_sum                                                    = 23,
    txq_top_sdq0_int_sum                                                    = 24,
    txq_top_sdq1_int_sum                                                    = 25,
    txq_top_sdq2_int_sum                                                    = 26,
    txq_top_sdq3_int_sum                                                    = 27,
    txq_top_txqs0_tai_int_sum_0                                             = 28,
    txq_top_txqs0_tai_int_sum_1                                             = 29,
    txq_top_txqs1_tai_int_sum_0                                             = 30,
    txq_top_txqs1_tai_int_sum_1                                             = 31
};

enum{  /*cnm_grp_0_6_intr_ - ports1InterruptsSummaryCause */
    Gpio_7_0_Intr                                                           =  1,
    Gpio_15_8_Intr                                                          =  2,
    Gpio_23_16_Intr                                                         =  3,
    Gpio_31_24_Intr                                                         =  4,
    Gpio_39_32_Intr                                                         =  5,
    Gpio_47_40_Intr                                                         =  6,
    Uart_Intr_0                                                             =  7,
    Uart_Intr_1                                                             =  8,
    I2C_Intr_0                                                              =  9,
    I2C_Intr_1                                                              = 10,
    R2Intr_Spi_A_Rdy                                                        = 11,
    Iunit_Intr                                                              = 12,
    Pcie_Err_Int                                                            = 13,
    Pcie_Int                                                                = 14,
    Aac_Int                                                                 = 15,
    Cnm_Rfu_Int                                                             = 16,
    PIN_PHY_INT_OUT                                                         = 17,
    R2Intr_Spi_B_Rdy                                                        = 18,
    Cnm_Amb3_M2A_Intr                                                       = 19,
    Cnm_Amb2_M2A_Intr                                                       = 20,
    Cnm_Ambd_M2A_Intr                                                       = 21,
    Cnm_Xor2Irq_Sys_0                                                       = 22,
    Cnm_Xor2Irq_Mem_0                                                       = 23,
    Cnm_Xor2Irq_Sys_1                                                       = 24,
    Cnm_Xor2Irq_Mem_1                                                       = 25
};

enum{ /*cnm_grp_0_7_intr - ports2InterruptsSummaryCause/ports2InterruptsSummary */
    Mg0_Int_Out0                                                            =  1,
    Mg0_Int_Out1                                                            =  2,
    Mg0_Int_Out2                                                            =  3,
    Mg1_Int_Out0                                                            =  4,
    Mg1_Int_Out1                                                            =  5,
    Mg1_Int_Out2                                                            =  6,
    Mg2_Int_Out0                                                            =  7,
    Mg2_Int_Out1                                                            =  8,
    Mg2_Int_Out2                                                            =  9,
    Mg3_Int_Out0                                                            = 10,
    Mg3_Int_Out1                                                            = 11,
    Mg3_Int_Out2                                                            = 12,
    dfx_client_hawk_mng_macro_tai_clk_int_sum                               = 27,
    dfx_client_hawk_mng_macro_cm3_clk_int_sum                               = 28,
    dfx_client_hawk_mng_macro_mng_clk_int_sum                               = 29,
    tai_interrupt_cause_int_sum_0                                           = 30,
    tai_interrupt_cause_int_sum_1                                           = 31
};

enum{
     MG_0_INDEX
    ,MG_1_INDEX
    ,MG_2_INDEX
    ,MG_3_INDEX

    ,NUM_OF_MGS   /*4*/
};


#define MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,mgIndex,father_myBitIndex,father_interrupt) \
{                                                                                              \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.mg_causeRegName),0,mgIndex | MG_UNIT_USED_CNS}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.mg_maskRegName ),0,mgIndex | MG_UNIT_USED_CNS}, \
        /*myFatherInfo*/{                                                                      \
            /*myBitIndex*/  father_myBitIndex ,                                                \
            /*interruptPtr*/&father_interrupt                                                  \
            },                                                                                 \
        /*isTriggeredByWrite*/ 1                                                               \
}

#define MG_SDMA_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,mgIndex,father_myBitIndex,father_interrupt) \
{                                                                                              \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.SDMARegs.mg_causeRegName),0,mgIndex | MG_UNIT_USED_CNS}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.SDMARegs.mg_maskRegName ),0,mgIndex | MG_UNIT_USED_CNS}, \
        /*myFatherInfo*/{                                                                      \
            /*myBitIndex*/  father_myBitIndex ,                                                \
            /*interruptPtr*/&father_interrupt                                                  \
            },                                                                                 \
        /*isTriggeredByWrite*/ 1                                                               \
}

#define UNSED_MG {{0}}

/* duplicate the node of MG only to MG_x_0 (primary MG units) */
#define DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt) \
     MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_0_INDEX,father_myBitIndex,father_interrupt[MG_0_INDEX]) \
    ,UNSED_MG/* MG 0_1 not used */                                                                   \
    ,UNSED_MG/* MG 0_2 not used */                                                                   \
    ,UNSED_MG/* MG 0_3 not used */


/* duplicate the node of MG to MG_x_0 and MG_x_1 (primary + secondary MG units) */
#define DUP_PRIMARY_AND_SECONDARY_MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt) \
     MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_0_INDEX,father_myBitIndex,father_interrupt[MG_0_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_1_INDEX,father_myBitIndex,father_interrupt[MG_1_INDEX]) \
    ,UNSED_MG/* MG 0_2 not used */                                                                   \
    ,UNSED_MG/* MG 0_3 not used */

/* duplicate the node of MG to ALL MG_x_x */
#define DUP_ALL_MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt) \
     MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_0_INDEX,father_myBitIndex,father_interrupt[MG_0_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_1_INDEX,father_myBitIndex,father_interrupt[MG_1_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_2_INDEX,father_myBitIndex,father_interrupt[MG_2_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_3_INDEX,father_myBitIndex,father_interrupt[MG_3_INDEX])

static SKERNEL_INTERRUPT_REG_INFO_STC  hawk_ports2FatherInterruptsSummary[NUM_OF_MGS];

static SKERNEL_INTERRUPT_REG_INFO_STC  hawk_ports2InterruptsSummary    [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(ports2InterruptsSummaryCause     ,ports2InterruptsSummaryMask     ,ports_2_int_sum  ,hawk_ports2FatherInterruptsSummary)  };



static SKERNEL_INTERRUPT_REG_INFO_STC  hawk_GlobalInterruptsSummary    [NUM_OF_MGS] =
{
    { /* copied from GlobalInterruptsSummary */
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.globalInterruptCause),0,MG_0_INDEX | MG_UNIT_USED_CNS},/*SMEM_CHT_GLB_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.globalInterruptMask) ,0,MG_0_INDEX | MG_UNIT_USED_CNS},
        /*myFatherInfo*/{
            /*myBitIndex*/0 ,
            /*interruptPtr*/NULL /* no father for me , thank you */
            },
        /*isTriggeredByWrite*/ 1
    }
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask, MG_1_INDEX,Mg1_Int_Out0, hawk_ports2InterruptsSummary[MG_0_INDEX])/* into MG_0_0 (tile 0)*/
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask, MG_2_INDEX,Mg2_Int_Out0, hawk_ports2InterruptsSummary[MG_0_INDEX])
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask, MG_3_INDEX,Mg3_Int_Out0, hawk_ports2InterruptsSummary[MG_0_INDEX])
};

static SKERNEL_INTERRUPT_REG_INFO_STC  hawk_FuncUnitsInterruptsSummary [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(functionalInterruptsSummaryCause ,functionalInterruptsSummaryMask ,FuncUnitsIntsSum ,hawk_GlobalInterruptsSummary) };
static SKERNEL_INTERRUPT_REG_INFO_STC  hawk_dataPathInterruptSummary   [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(SERInterruptsSummary             ,SERInterruptsMask               ,DataPathIntSum   ,hawk_GlobalInterruptsSummary) };
static SKERNEL_INTERRUPT_REG_INFO_STC  hawk_portsInterruptsSummary     [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(stackPortsInterruptCause     ,stackPortsInterruptMask             ,PortsIntSum      ,hawk_GlobalInterruptsSummary) };
static SKERNEL_INTERRUPT_REG_INFO_STC  hawk_dfxInterruptsSummary     [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(dfxInterruptsSummaryCause     ,dfxInterruptsSummaryMask           ,dfxIntSum        ,hawk_GlobalInterruptsSummary) };
static SKERNEL_INTERRUPT_REG_INFO_STC  hawk_dfx1InterruptsSummary     [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(dfx1InterruptsSummaryCause    ,dfx1InterruptsSummaryMask          ,dfx1IntSum       ,hawk_GlobalInterruptsSummary) };



/* this node need support in primary and secondary MG units (to support CNC dump from the FUQ) */
static SKERNEL_INTERRUPT_REG_INFO_STC  hawk_mg_global_misc_interrupts  [NUM_OF_MGS] =
{    DUP_ALL_MG_INTERTTUPT_REG(miscellaneousInterruptCause ,miscellaneousInterruptMask ,MgInternalIntSum ,hawk_GlobalInterruptsSummary) };


#define MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,mgIndex) \
     MG_SDMA_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,mgIndex,father_myBitIndex,father_interrupt[mgIndex])


/* duplicate the node of MG to all MG units */
#define DUP_SDMA_MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt) \
     MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_0_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_1_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_2_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_3_INDEX)

/* this node need support in ALL MG units (to MG SDMA) */
static SKERNEL_INTERRUPT_REG_INFO_STC  hawk_mg_global_sdma_receiveSDMA_interrupts  [NUM_OF_MGS] =
{    DUP_SDMA_MG_INTERTTUPT_REG(receiveSDMAInterruptCause ,receiveSDMAInterruptMask ,RxSDMASum ,hawk_GlobalInterruptsSummary) };

/* this node need support in ALL MG units (to MG SDMA) */
static SKERNEL_INTERRUPT_REG_INFO_STC  hawk_mg_global_sdma_transmitSDMA_interrupts  [NUM_OF_MGS] =
{    DUP_SDMA_MG_INTERTTUPT_REG(transmitSDMAInterruptCause ,transmitSDMAInterruptMask ,TxSDMASum ,hawk_GlobalInterruptsSummary) };

#define MG_INTERTTUPT_TREE1_REG(mg_causeRegName, mg_maskRegName, mgIndex, father_myBitIndex, father_interrupt) \
{                                                                                                               \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt_tree1.mg_causeRegName), 0, mgIndex | MG_UNIT_USED_CNS}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt_tree1.mg_maskRegName ), 0, mgIndex | MG_UNIT_USED_CNS}, \
        /*myFatherInfo*/{                                                                      \
            /*myBitIndex*/  father_myBitIndex ,                                                \
            /*interruptPtr*/&father_interrupt                                                  \
            },                                                                                 \
        /*isTriggeredByWrite*/ 1                                                               \
}

static SKERNEL_INTERRUPT_REG_INFO_STC cnc_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.CNCInterruptSummaryCauseReg)},/*SMEM_CHT3_CNC_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.CNCInterruptSummaryMaskReg)},
    /*myFatherInfo*/{
        /*myBitIndex*/cp_top_pipe0_cnc0_int_sum ,
        /*interruptPtr*/&cnm_grp_0_0_intr
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[1].globalRegs.CNCInterruptSummaryCauseReg)},/*SMEM_CHT3_CNC_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[1].globalRegs.CNCInterruptSummaryMaskReg)},
    /*myFatherInfo*/{
        /*myBitIndex*/cp_top_pipe0_cnc1_int_sum,
        /*interruptPtr*/&cnm_grp_0_0_intr
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC egf_eft_global_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EGF_eft.global.EFTInterruptsCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EGF_eft.global.EFTInterruptsMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cp_top_pipe0_eft_int_sum,
        /*interruptPtr*/&cnm_grp_0_0_intr
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC oam_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[0].OAMUnitInterruptCause)},/*SMEM_LION2_OAM_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[0].OAMUnitInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cp_top_pipe0_ioam_int_sum ,
        /*interruptPtr*/&cnm_grp_0_0_intr
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[1].OAMUnitInterruptCause)},/*SMEM_LION2_OAM_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[1].OAMUnitInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cp_top_pipe0_eoam_int_sum,
        /*interruptPtr*/&cnm_grp_0_0_intr
        },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC pcl_unit_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCL.PCLUnitInterruptCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCL.PCLUnitInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cp_top_pipe0_pcl_int_sum,
        /*interruptPtr*/&cnm_grp_0_0_intr
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC tti_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TTI.TTIEngine.TTIEngineInterruptCause)},/*SMEM_LION3_TTI_INTERRUPT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TTI.TTIEngine.TTIEngineInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cp_top_pipe0_tti_int_sum ,
        /*interruptPtr*/&cnm_grp_0_1_intr
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC bridge_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(L2I.bridgeEngineInterrupts.bridgeInterruptCause)},/*SMEM_CHT_BRIDGE_INT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(L2I.bridgeEngineInterrupts.bridgeInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cp_top_pipe0_l2i_int_sum ,
        /*interruptPtr*/&cnm_grp_0_0_intr,
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC mac_tbl_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(FDB.FDBCore.FDBInterrupt.FDBInterruptCauseReg)},/*SMEM_CHT_MAC_TBL_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(FDB.FDBCore.FDBInterrupt.FDBInterruptMaskReg)},
    /*myFatherInfo*/{
        /*myBitIndex*/cp_top_mt_int_sum ,
        /*interruptPtr*/&cnm_grp_0_0_intr,
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC TcamInterruptsSummary[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TCAM.tcamInterrupts.TCAMInterruptsSummaryCause)},/**/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TCAM.tcamInterrupts.TCAMInterruptsSummaryMask)},/**/
    /*myFatherInfo*/{
        /*myBitIndex*/cp_top_tcam_int_sum ,
        /*interruptPtr*/&cnm_grp_0_1_intr
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC tcam_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TCAM.tcamInterrupts.TCAMInterruptCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TCAM.tcamInterrupts.TCAMInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&TcamInterruptsSummary[0],
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC ha_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(HA.HAInterruptCause)},/*SMEM_LION3_HA_INTERRUPT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(HA.HAInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cp_top_pipe0_ha_int_sum ,
        /*interruptPtr*/&cnm_grp_0_0_intr
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC ermrk_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(ERMRK.ERMRKInterruptCause)},/*SMEM_LION3_ERMRK_INTERRUPT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(ERMRK.ERMRKInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cp_top_pipe0_ermrk_int_sum,
        /*interruptPtr*/&cnm_grp_0_0_intr,
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC  preEgrInterruptSummary[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.preEgrInterrupt.preEgrInterruptSummary)}, /*SMEM_CHT_EQ_INT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.preEgrInterrupt.preEgrInterruptSummaryMask)},  /*SMEM_CHT_EQ_INT_MASK_REG*/
    /*myFatherInfo*/{
        /*myBitIndex*/cp_top_pipe0_eq_int_sum,
        /*interruptPtr*/&cnm_grp_0_0_intr
        },
    /*isTriggeredByWrite*/ 1
    }
};


static SKERNEL_INTERRUPT_REG_INFO_STC CpuCodeRateLimitersInterruptSummary[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/2 ,
            /*interruptPtr*/&preEgrInterruptSummary[0]
            },
        /*isTriggeredByWrite*/ 1
    }
};

#define CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(n)                                                                          \
{                                                                                                                              \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptCause[n-1])},    \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptMask[n-1])},     \
    /*myFatherInfo*/{                                                                                                          \
        /*myBitIndex*/n ,                                                                                                      \
        /*interruptPtr*/&CpuCodeRateLimitersInterruptSummary[0]                                                             \
        },                                                                                                                     \
    /*isTriggeredByWrite*/ 1                                                                                                   \
}

static SKERNEL_INTERRUPT_REG_INFO_STC cpu_code_rate_limiters_interrupts[] =
{
     CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 2)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 3)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 4)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 5)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 6)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 7)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 8)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 9)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(10)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(11)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(12)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(13)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(14)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(15)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(16)
};

static SKERNEL_INTERRUPT_REG_INFO_STC mll_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MLL.MLLGlobalCtrl.MLLInterruptCauseReg)},/*SMEM_LION3_MLL_INTERRUPT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MLL.MLLGlobalCtrl.MLLInterruptMaskReg)},
        /*myFatherInfo*/{
            /*myBitIndex*/cp_top_pipe0_mll_int_sum,
            /*interruptPtr*/&cnm_grp_0_0_intr
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC policer_ipfix_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[0].policerInterruptCause)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[0].policerInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cp_top_pipe0_iplr0_int_sum ,
            /*interruptPtr*/&cnm_grp_0_0_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[1].policerInterruptCause)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[1].policerInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cp_top_pipe0_iplr1_int_sum ,
            /*interruptPtr*/&cnm_grp_0_0_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[2].policerInterruptCause)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[2].policerInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cp_top_pipe0_eplr_int_sum ,
            /*interruptPtr*/&cnm_grp_0_0_intr
            },
        /*isTriggeredByWrite*/ 1
    }
};

#define NUM_PORTS_PER_GROUP 8

/* define the USX MAC global interrupt summary */
#define  USX_SUMMARY_GLOBAL_INTERRUPT_SUMMARY_MAC(group, bitInMg, regInMg) \
static SKERNEL_INTERRUPT_REG_INFO_STC  usx_global_interruptSummary_group##group = \
{                                                  \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[group+4].USX_GLOBAL.globalInterruptSummaryCause)}, /**/  \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[group+4].USX_GLOBAL.globalInterruptSummaryMask)},  /**/  \
    /*myFatherInfo*/{                              \
        /*myBitIndex*/ bitInMg ,                   \
        /*interruptPtr*/&regInMg                   \
        },                                         \
    /*isTriggeredByWrite*/ 1                       \
}

/* define the USX MAC global interrupt summary per group of USX ports 0..7 */
#define USX_SUMMARY_PORT_GROUP_0_2_INTERRUPT_SUMMARY_MAC(group) \
    USX_SUMMARY_GLOBAL_INTERRUPT_SUMMARY_MAC(group, (gop_downlink_top_epi0_mac_pcs_int_sum_1 + group)/*bitInMg*/, cnm_grp_0_2_intr/*regInMg*/)

#define USX_SUMMARY_PORT_GROUP_3_4_INTERRUPT_SUMMARY_MAC(group) \
    USX_SUMMARY_GLOBAL_INTERRUPT_SUMMARY_MAC(group, (gop_downlink_top_epi1_mac_pcs_int_sum_1 + group % 3)/*bitInMg*/, cnm_grp_0_2_intr/*regInMg*/)

#define USX_SUMMARY_PORT_GROUP_5_5_INTERRUPT_SUMMARY_MAC(group) \
    USX_SUMMARY_GLOBAL_INTERRUPT_SUMMARY_MAC(group, (gop_downlink_top_epi1_mac_pcs_int_sum_3)/*bitInMg*/, cnm_grp_0_3_intr/*regInMg*/)

USX_SUMMARY_PORT_GROUP_0_2_INTERRUPT_SUMMARY_MAC(0);
USX_SUMMARY_PORT_GROUP_0_2_INTERRUPT_SUMMARY_MAC(1);
USX_SUMMARY_PORT_GROUP_0_2_INTERRUPT_SUMMARY_MAC(2);
USX_SUMMARY_PORT_GROUP_3_4_INTERRUPT_SUMMARY_MAC(3);
USX_SUMMARY_PORT_GROUP_3_4_INTERRUPT_SUMMARY_MAC(4);
USX_SUMMARY_PORT_GROUP_5_5_INTERRUPT_SUMMARY_MAC(5);

/* node for USX mac ports interrupt cause */
#define USX_PORT_INTERRUPT_CAUSE_MAC(group, port)      \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[((group) * NUM_PORTS_PER_GROUP) + port].USX_MAC_PORT.portInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[((group) * NUM_PORTS_PER_GROUP) + port].USX_MAC_PORT.portInterruptMask) },  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                       \
        /*myBitIndex*/(1+port),                             \
        /*interruptPtr*/&usx_global_interruptSummary_group##group      \
    },                                                    \
    /*isTriggeredByWrite*/ 1                              \
}

/* define USX ports interrupt summary per group */
#define USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(group)      \
    USX_PORT_INTERRUPT_CAUSE_MAC(group, 0),                 \
    USX_PORT_INTERRUPT_CAUSE_MAC(group, 1),                 \
    USX_PORT_INTERRUPT_CAUSE_MAC(group, 2),                 \
    USX_PORT_INTERRUPT_CAUSE_MAC(group, 3),                 \
    USX_PORT_INTERRUPT_CAUSE_MAC(group, 4),                 \
    USX_PORT_INTERRUPT_CAUSE_MAC(group, 5),                 \
    USX_PORT_INTERRUPT_CAUSE_MAC(group, 6),                 \
    USX_PORT_INTERRUPT_CAUSE_MAC(group, 7)

/* node for CPU ports interrupt cause */
#define CPU_PORT_INTERRUPT_CAUSE_MAC(unitIndex)      \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_cpu_port[unitIndex].MTI_EXT_PORT.portInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_cpu_port[unitIndex].MTI_EXT_PORT.portInterruptMask)},  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                       \
        /*myBitIndex*/(gop_downlink_top_epi1_mac_pcs_int_sum_4 + unitIndex),\
        /*interruptPtr*/&cnm_grp_0_3_intr                   \
    },                                                      \
    /*isTriggeredByWrite*/ 1                                \
}

/* 400G MAC global interrupt summary */
static SKERNEL_INTERRUPT_REG_INFO_STC ports_400G_global_interruptSummary[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[0].MTI_GLOBAL.globalInterruptSummaryCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[0].MTI_GLOBAL.globalInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/gop_downlink_top_epi0_mac_pcs_int_sum_0,
        /*interruptPtr*/&cnm_grp_0_2_intr
        },
    /*isTriggeredByWrite*/ 1
    }
    ,
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[1].MTI_GLOBAL.globalInterruptSummaryCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[1].MTI_GLOBAL.globalInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/gop_downlink_top_epi1_mac_pcs_int_sum_0,
        /*interruptPtr*/&cnm_grp_0_2_intr
        },
    /*isTriggeredByWrite*/ 1
    }
    ,
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[2].MTI_GLOBAL.globalInterruptSummaryCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[2].MTI_GLOBAL.globalInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/gop_uplink_top_epi2_mac_pcs_int_sum,
        /*interruptPtr*/&cnm_grp_0_4_intr
        },
    /*isTriggeredByWrite*/ 1
    }
    ,
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[3].MTI_GLOBAL.globalInterruptSummaryCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[3].MTI_GLOBAL.globalInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/gop_uplink_top_epi3_mac_pcs_int_sum,
        /*interruptPtr*/&cnm_grp_0_4_intr
        },
    /*isTriggeredByWrite*/ 1
    }
};


/* node for 400G ports interrupt cause */
#define PORT_400G_INTERRUPT_CAUSE_MAC(group, port)      \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[port].MTI_EXT_PORT.portInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[port].MTI_EXT_PORT.portInterruptMask)},  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                       \
        /*myBitIndex*/(1+port),    \
        /*interruptPtr*/&ports_400G_global_interruptSummary[group] \
    },                                                      \
    /*isTriggeredByWrite*/ 1                                \
}

/* node for SEG ports interrupt cause */
#define PORT_SEG_INTERRUPT_CAUSE_MAC(group, port)      \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[port].MTI_EXT_PORT.segPortInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[port].MTI_EXT_PORT.segPortInterruptMask)},  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                       \
        /*myBitIndex*/(9+port),    \
        /*interruptPtr*/&ports_400G_global_interruptSummary[group] \
    },                                                      \
    /*isTriggeredByWrite*/ 1                                \
}

/* node for 400G ports interrupt cause */
#define PORT_400G_PER_GROUP_INTERRUPT_CAUSE_MAC(group)      \
     PORT_400G_INTERRUPT_CAUSE_MAC(group, 0)                \
    ,PORT_400G_INTERRUPT_CAUSE_MAC(group, 1)                \
    ,PORT_400G_INTERRUPT_CAUSE_MAC(group, 2)                \
    ,PORT_400G_INTERRUPT_CAUSE_MAC(group, 3)                \
    ,PORT_400G_INTERRUPT_CAUSE_MAC(group, 4)                \
    ,PORT_400G_INTERRUPT_CAUSE_MAC(group, 5)                \
    ,PORT_400G_INTERRUPT_CAUSE_MAC(group, 6)                \
    ,PORT_400G_INTERRUPT_CAUSE_MAC(group, 7)

#define PORT_SEG_PER_GROUP_INTERRUPT_CAUSE_MAC(group)       \
     PORT_SEG_INTERRUPT_CAUSE_MAC(group, 0)                 \
    ,PORT_SEG_INTERRUPT_CAUSE_MAC(group, 1)

static SKERNEL_INTERRUPT_REG_INFO_STC hawk_mac_ports_interrupts[] =
{
     USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(0)
    ,USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(1)
    ,USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(2)
    ,USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(3)
    ,USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(4)
    ,USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(5)
    ,PORT_400G_PER_GROUP_INTERRUPT_CAUSE_MAC(0)
    ,PORT_400G_PER_GROUP_INTERRUPT_CAUSE_MAC(1)
    ,PORT_400G_PER_GROUP_INTERRUPT_CAUSE_MAC(2)
    ,PORT_400G_PER_GROUP_INTERRUPT_CAUSE_MAC(3)
    ,PORT_SEG_PER_GROUP_INTERRUPT_CAUSE_MAC(0)
    ,PORT_SEG_PER_GROUP_INTERRUPT_CAUSE_MAC(1)
    ,PORT_SEG_PER_GROUP_INTERRUPT_CAUSE_MAC(2)
    ,PORT_SEG_PER_GROUP_INTERRUPT_CAUSE_MAC(3)
    ,CPU_PORT_INTERRUPT_CAUSE_MAC(0)
};

#define HAWK_TSU_PCA_GLOBAL_UNIT_CHANNEL_INTERRUPT_SUMMARY_MAC(bit, unit)                                                                            \
{                                                                                                                                               \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].ctsuGlobalInterruptSummaryCause)},                                         \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].ctsuGlobalInterrupSummarytMask)},                                          \
    /*myFatherInfo*/{                                                                                                                           \
        /*myBitIndex*/ bit,                                                                                                                     \
        /*interruptPtr*/&cnm_grp_0_4_intr                                                                                                       \
        },                                                                                                                                      \
    /*isTriggeredByWrite*/ 1                                                                                                                    \
}

#define HAWK_TSU_MSEC_GLOBAL_UNIT_CHANNEL_INTERRUPT_SUMMARY_MAC(bit, unit)                                                                      \
{                                                                                                                                               \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].ctsuGlobalInterruptSummaryCause)},                                         \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].ctsuGlobalInterrupSummarytMask)},                                          \
    /*myFatherInfo*/{                                                                                                                           \
        /*myBitIndex*/ bit,                                                                                                                     \
        /*interruptPtr*/&cnm_grp_0_3_intr                                                                                                       \
        },                                                                                                                                      \
    /*isTriggeredByWrite*/ 1                                                                                                                    \
}

static SKERNEL_INTERRUPT_REG_INFO_STC  hawkTsuInterruptSummary[] =
{ 
    HAWK_TSU_MSEC_GLOBAL_UNIT_CHANNEL_INTERRUPT_SUMMARY_MAC(9, 0),
    HAWK_TSU_MSEC_GLOBAL_UNIT_CHANNEL_INTERRUPT_SUMMARY_MAC(21, 1),
    HAWK_TSU_PCA_GLOBAL_UNIT_CHANNEL_INTERRUPT_SUMMARY_MAC (18, 2),
    HAWK_TSU_PCA_GLOBAL_UNIT_CHANNEL_INTERRUPT_SUMMARY_MAC (30, 3)
};

#define HAWK_TSU_UNIT_CHANNEL_0_15_INTERRUPT_SUMMARY_MAC(unit)                                                                                  \
{                                                                                                                                           \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].channel_0_15_InterruptSummaryCause)},                                  \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].channel_0_15_InterruptSummaryMask)},                                   \
    /*myFatherInfo*/{                                                                                                                       \
        /*myBitIndex*/2 ,                                                                                                                   \
        /*interruptPtr*/&hawkTsuInterruptSummary[unit]                                                                                      \
        },                                                                                                                                  \
    /*isTriggeredByWrite*/ 1                                                                                                                \
}                                                                                                                                           \

#define HAWK_TSU_UNIT_CHANNEL_16_31_INTERRUPT_SUMMARY_MAC(unit)                                                                                 \
{                                                                                                                                               \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].channel_16_31_InterruptSummaryCause)},                                 \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].channel_16_31_InterruptSummaryMask)},                                  \
    /*myFatherInfo*/{                                                                                                                       \
        /*myBitIndex*/3 ,                                                                                                                   \
        /*interruptPtr*/&hawkTsuInterruptSummary[unit]                                                                                      \
        },                                                                                                                                  \
    /*isTriggeredByWrite*/ 1                                                                                                                \
}

static SKERNEL_INTERRUPT_REG_INFO_STC hawkTsuChannelInterrupt_0_15_Summary[] =
{
    HAWK_TSU_UNIT_CHANNEL_0_15_INTERRUPT_SUMMARY_MAC(0)
   ,HAWK_TSU_UNIT_CHANNEL_0_15_INTERRUPT_SUMMARY_MAC(1)
   ,HAWK_TSU_UNIT_CHANNEL_0_15_INTERRUPT_SUMMARY_MAC(2)
   ,HAWK_TSU_UNIT_CHANNEL_0_15_INTERRUPT_SUMMARY_MAC(3)
};

static SKERNEL_INTERRUPT_REG_INFO_STC hawkTsuChannelInterrupt_16_31_Summary[] =
{
    HAWK_TSU_UNIT_CHANNEL_16_31_INTERRUPT_SUMMARY_MAC(0)
   ,HAWK_TSU_UNIT_CHANNEL_16_31_INTERRUPT_SUMMARY_MAC(1)
   ,HAWK_TSU_UNIT_CHANNEL_16_31_INTERRUPT_SUMMARY_MAC(2)
   ,HAWK_TSU_UNIT_CHANNEL_16_31_INTERRUPT_SUMMARY_MAC(3)
};

/* node for TSU channel interrupt cause registers */
#define  GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, index)                                                                                         \
{                                                                                                                                               \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].channelInterruptCause[(index)])}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/          \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].channelInterruptMask[(index)])},  /*SMEM_CHT_PORT_INT_CAUSE_MASK_REG*/     \
    /*myFatherInfo*/{                                                                                                                           \
        /*myBitIndex*/(((index)%16)+1),                                                                                                         \
        /*interruptPtr*/(((((index)/16) % 2) == 0) ? &hawkTsuChannelInterrupt_0_15_Summary[unit] : &hawkTsuChannelInterrupt_16_31_Summary[unit])   \
    },                                                                                                                                          \
    /*isTriggeredByWrite*/ 1                                                                                                                    \
}

#define GROUP_TSU_UNIT_CHANNEL_INTERRUPT_CAUSE_MAC(unit)       \
     GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit,  0)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit,  1)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit,  2)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit,  3)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit,  4)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit,  5)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit,  6)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit,  7)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit,  8)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit,  9)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 10)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 11)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 12)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 13)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 14)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 15)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 16)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 17)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 18)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 19)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 20)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 21)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 22)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 23)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 24)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 25)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 26)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 27)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 28)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 29)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 30)           \
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, 31)

static SKERNEL_INTERRUPT_REG_INFO_STC pca_tsu_interrupts[] =
{
     GROUP_TSU_UNIT_CHANNEL_INTERRUPT_CAUSE_MAC(0)
    ,GROUP_TSU_UNIT_CHANNEL_INTERRUPT_CAUSE_MAC(1)
    ,GROUP_TSU_UNIT_CHANNEL_INTERRUPT_CAUSE_MAC(2)
    ,GROUP_TSU_UNIT_CHANNEL_INTERRUPT_CAUSE_MAC(3)
};

static SKERNEL_INTERRUPT_REG_INFO_STC PHA_summary_Interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.pha_regs.errorsAndInterrupts.PHAInterruptSumCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.pha_regs.errorsAndInterrupts.PHAInterruptSumMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cp_top_pipe0_sip_pha_int_sum,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC PPA_summary_Interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPA.ppa_regs.PPAInterruptSumCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPA.ppa_regs.PPAInterruptSumMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/ 2 ,
            /*interruptPtr*/&PHA_summary_Interrupts[0]
            },
        /*isTriggeredByWrite*/ 1
    }
};

#define PPG_INTERRUPT_SUM_CAUSE_NODE(ppgId)   \
    {                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPG[ppgId].ppg_regs.PPGInterruptSumCause)}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPG[ppgId].ppg_regs.PPGInterruptSumMask)},  \
    /*myFatherInfo*/{                                                                                 \
        /*myBitIndex*/ (ppgId)+2 ,                                                                    \
        /*interruptPtr*/&PPA_summary_Interrupts[0]                                                    \
        },                                                                                            \
    /*isTriggeredByWrite*/ 1                                                                          \
    }

static SKERNEL_INTERRUPT_REG_INFO_STC PPG_summary_Interrupts[] =
{
     PPG_INTERRUPT_SUM_CAUSE_NODE(0)
    ,PPG_INTERRUPT_SUM_CAUSE_NODE(1)
    ,PPG_INTERRUPT_SUM_CAUSE_NODE(2)
    ,PPG_INTERRUPT_SUM_CAUSE_NODE(3)
};

#define PPN_IN_PPG_INTERRUPT_CAUSE_NODE(ppgId,ppnId)   \
    {                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPG[ppgId].PPN[ppnId].ppn_regs.PPNInternalErrorCause)}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPG[ppgId].PPN[ppnId].ppn_regs.PPNInternalErrorMask)},  \
    /*myFatherInfo*/{                                                                                 \
        /*myBitIndex*/ (ppnId)+2 ,                                                                    \
        /*interruptPtr*/&PPG_summary_Interrupts[ppgId]                                                \
        },                                                                                            \
    /*isTriggeredByWrite*/ 1                                                                          \
    }

#define ALL_PPN_IN_PPG_CAUSE_NODES(ppgId) \
     PPN_IN_PPG_INTERRUPT_CAUSE_NODE(ppgId,0)  \
    ,PPN_IN_PPG_INTERRUPT_CAUSE_NODE(ppgId,1)  \
    ,PPN_IN_PPG_INTERRUPT_CAUSE_NODE(ppgId,2)  \
    ,PPN_IN_PPG_INTERRUPT_CAUSE_NODE(ppgId,3)  \
    ,PPN_IN_PPG_INTERRUPT_CAUSE_NODE(ppgId,4)  \
    ,PPN_IN_PPG_INTERRUPT_CAUSE_NODE(ppgId,5)  \
    ,PPN_IN_PPG_INTERRUPT_CAUSE_NODE(ppgId,6)  \
    ,PPN_IN_PPG_INTERRUPT_CAUSE_NODE(ppgId,7)  \
    ,PPN_IN_PPG_INTERRUPT_CAUSE_NODE(ppgId,8)  \
    ,PPN_IN_PPG_INTERRUPT_CAUSE_NODE(ppgId,9)


#define PPG_CAUSE_NODE(ppgId)                   \
    {                                           \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPG[ppgId].ppg_regs.PPGInternalErrorCause)}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPG[ppgId].ppg_regs.PPGInternalErrorMask)},  \
    /*myFatherInfo*/{                                                                                 \
        /*myBitIndex*/ 1 ,                                                                            \
        /*interruptPtr*/&PPG_summary_Interrupts[ppgId]                                                \
        },                                                                                            \
    /*isTriggeredByWrite*/ 1                                                                          \
    }                                                                                                 \
    ,                                                                                                 \
    ALL_PPN_IN_PPG_CAUSE_NODES(ppgId)



static SKERNEL_INTERRUPT_REG_INFO_STC PHA_Interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.pha_regs.errorsAndInterrupts.PHAInternalErrorCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.pha_regs.errorsAndInterrupts.PHAInternalErrorMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&PHA_summary_Interrupts[0]
            },
        /*isTriggeredByWrite*/ 1
    },

    /*********************/

    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPA.ppa_regs.PPAInternalErrorCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPA.ppa_regs.PPAInternalErrorMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&PPA_summary_Interrupts[0]
            },
        /*isTriggeredByWrite*/ 1
    },

    PPG_CAUSE_NODE(0),
    PPG_CAUSE_NODE(1),
    PPG_CAUSE_NODE(2),
    PPG_CAUSE_NODE(3)

};

static SKERNEL_INTERRUPT_REG_INFO_STC em_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EXACT_MATCH.emInterrupt.emInterruptCauseReg)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EXACT_MATCH.emInterrupt.emInterruptMaskReg)},
    /*myFatherInfo*/{
        /*myBitIndex*/cp_top_em_int_sum,
        /*interruptPtr*/&cnm_grp_0_0_intr
        },
    /*isTriggeredByWrite*/ 1
    }
};

/* get the MG index from the currMgInfoPtr->causeReg.mgUnitIndex */
#define   GET_MG_INDEX(currMgInfoPtr)    (currMgInfoPtr->causeReg.mgUnitIndex & (~MG_UNIT_USED_CNS))


/**
* @internal internal_smemHawkInterruptTreeInit function
* @endinternal
*
* @brief   Init the interrupts tree for the Hawk device
*
* @param[in] devObjPtr                - pointer to device object.
*/
static GT_VOID internal_smemHawkInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT         * devObjPtr,
    INOUT    GT_U32                    *indexPtr,
    INOUT SKERNEL_INTERRUPT_REG_INFO_STC ** currDbPtrPtr
)
{
    GT_U32 index = *indexPtr;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr = *currDbPtrPtr;
    GT_U32  mgMax;
    GT_U32  mgIndex;

    GT_U32 group;
    GT_U32 port, globalMac;
    GT_U32 ii;
    SKERNEL_INTERRUPT_REG_INFO_STC * portEntryPtr;

    portEntryPtr = &hawk_mac_ports_interrupts[0];

    /* USX ports address alignment */
    for (group = 0; group < 6; group++)
    {
        for (port = 0; port < 8; port++)
        {
            ii = group * NUM_PORTS_PER_GROUP + port;
            globalMac = hawkUsxPortsArr[ii].globalMacNum;
            portEntryPtr->causeReg.registerOffsetInDb =
                FIELD_OFFSET_IN_STC_MAC(sip6_MTI[globalMac].USX_MAC_PORT.portInterruptCause, SMEM_SIP5_PP_REGS_ADDR_STC);
            portEntryPtr->maskReg.registerOffsetInDb =
                FIELD_OFFSET_IN_STC_MAC(sip6_MTI[globalMac].USX_MAC_PORT.portInterruptMask, SMEM_SIP5_PP_REGS_ADDR_STC);
            portEntryPtr++;
        }
    }

    /* 400G ports address alignment */
    for (group = 0; group < 4; group++)
    {
        for (port = 0; port < 8; port++)
        {
            ii =  group * NUM_PORTS_PER_GROUP + port;
            globalMac = hawk100GPortsArr[ii].globalMacNum;
            portEntryPtr->causeReg.registerOffsetInDb =
                FIELD_OFFSET_IN_STC_MAC(sip6_MTI[globalMac].MTI_EXT_PORT.portInterruptCause, SMEM_SIP5_PP_REGS_ADDR_STC);
            portEntryPtr->maskReg.registerOffsetInDb =
                FIELD_OFFSET_IN_STC_MAC(sip6_MTI[globalMac].MTI_EXT_PORT.portInterruptMask, SMEM_SIP5_PP_REGS_ADDR_STC);
            portEntryPtr++;
        }
    }

    /* SEG ports address alignment */
    for (group = 0; group < 4; group++)
    {
        for (port = 0; port < 2; port++)
        {
            ii =  group * NUM_PORTS_PER_GROUP + (port * 4);
            globalMac = hawk100GPortsArr[ii].globalMacNum;
            portEntryPtr->causeReg.registerOffsetInDb =
                FIELD_OFFSET_IN_STC_MAC(sip6_MTI[globalMac].MTI_EXT_PORT.segPortInterruptCause, SMEM_SIP5_PP_REGS_ADDR_STC);
            portEntryPtr->maskReg.registerOffsetInDb =
                FIELD_OFFSET_IN_STC_MAC(sip6_MTI[globalMac].MTI_EXT_PORT.segPortInterruptMask, SMEM_SIP5_PP_REGS_ADDR_STC);
            portEntryPtr++;
        }
    }

    { /* MG unit related interrupts */
        smemBobcat2GlobalInterruptTreeInit(devObjPtr, &index, &currDbPtr);

        /* for MG0 of the tile */
        mgIndex = 0;
        mgMax = 4;
        INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &hawk_ports2InterruptsSummary[mgIndex]);

        /* (skip MG[0]) add support for MG1,2,3 */
        mgIndex++;/* 1 */

        for(/* continue*/; mgIndex < mgMax ; mgIndex++)
        {
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &hawk_GlobalInterruptsSummary  [mgIndex]         );
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &hawk_mg_global_misc_interrupts[mgIndex]         );
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &hawk_mg_global_sdma_receiveSDMA_interrupts [mgIndex]);
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &hawk_mg_global_sdma_transmitSDMA_interrupts[mgIndex]);
        }
    }

    {   /* per pipe / tile interrupts */
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, tti_interrupts                    );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, bridge_interrupts                 );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, mac_tbl_interrupts                );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, ha_interrupts                     );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, ermrk_interrupts                  );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, pcl_unit_interrupts               );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, tcam_interrupts                   );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, cpu_code_rate_limiters_interrupts );
/*        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, preEgrInterruptSummary            );*/
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, egf_eft_global_interrupts         );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, mll_interrupts                    );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, policer_ipfix_interrupts          );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, cnc_interrupts                    );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, oam_interrupts                    );
#if 0 /* need to use sip6_xxx registers not xxx registers */
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, rxdma_interrupts                  );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, txdma_interrupts                  );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, txfifo_interrupts                 );
#endif
        {/* PHA interrupts */
            INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, PHA_Interrupts                );
            INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, PHA_summary_Interrupts        );
            INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, PPA_summary_Interrupts        );
            INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, PPG_summary_Interrupts        );
        }

        /* Exact Match interrupts*/
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, em_interrupts                     );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, pca_tsu_interrupts                );
    }

    *indexPtr = index;
    *currDbPtrPtr = currDbPtr;
}

/*******************************************************************************
*   smemHawkInterruptTreeInit
*
* DESCRIPTION:
*       Init the interrupts tree for the Hawk device
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*
* COMMENTS:
*
*
*******************************************************************************/
GT_VOID smemHawkInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 index = 0;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr;

    /* add here additions to 'compilation' info , that since 2 arrays point to
       each other we need to set it in runtime */

    /* Initialize Ports2 father Summary interrupts for primary MG units in all tiles */
    hawk_ports2InterruptsSummary[MG_0_INDEX].myFatherInfo.interruptPtr = &hawk_GlobalInterruptsSummary[MG_0_INDEX];

    devObjPtr->myInterruptsDbPtr = hawkInterruptsTreeDb;

    currDbPtr = devObjPtr->myInterruptsDbPtr;

    internal_smemHawkInterruptTreeInit(devObjPtr,&index,&currDbPtr);

    INTERRUPT_TREE_ADD_X_NODES_WITH_CHECK(index, &currDbPtr, hawk_mac_ports_interrupts, 48/*USX ports*/ + 32/*400G ports*/ + 8 /* SEG ports */ + 1/*CPU ports*/);

    /* End of interrupts - must be last */
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, skernel_interrupt_reg_info__last_line__cns);
}

void print_hawkInterruptsTreeDb_mgInterrupts(void)
{
    GT_U32  ii=0;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr = hawkInterruptsTreeDb;
    GT_U32  mgUnitIndex;

    while(currDbPtr->causeReg.registersDbType != SKERNEL_REGISTERS_DB_TYPE__LAST___E)
    {
        if(0 == strncmp(currDbPtr->causeReg.registerName,"MG.",3))
        {
            mgUnitIndex = GET_MG_INDEX(currDbPtr);

            printf("index [%3.3d] in MG[%d] hold [%s] \n",
                ii,
                mgUnitIndex,
                currDbPtr->causeReg.registerName);
        }

        ii ++;
        currDbPtr++;
    }

    printf("last index [%d] \n",
        ii-1);

}

extern GT_U32 getInterruptRegAddr
(
    IN SKERNEL_DEVICE_OBJECT *         devObjPtr,
    IN SKERNEL_INTERRUPT_REG_INFO_STC *currDbPtr,
    IN GT_BOOL                         getCause
);


static void print_curr(
    IN SKERNEL_DEVICE_OBJECT* devObjPtr ,
    IN SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr ,
    IN int    ii)
{
    GT_U32  mgUnitIndex;

    if(0 == strncmp(currDbPtr->causeReg.registerName,"MG.",3))
    {
        mgUnitIndex = GET_MG_INDEX(currDbPtr);

        printf("index [%3.3d] in MG[%d] hold [%s]  offset[0x%x] cause[0x%8.8x]mask[0x%8.8x]\n",
            ii,
            mgUnitIndex,
            currDbPtr->causeReg.registerName,
            currDbPtr->causeReg.registerAddrOffset,
            getInterruptRegAddr(devObjPtr,currDbPtr,GT_TRUE),/*cause*/
            getInterruptRegAddr(devObjPtr,currDbPtr,GT_FALSE)/*mask*/
            );
    }
    else
    {
        printf("index [%3.3d] hold [%s] offset[0x%x] cause[0x%8.8x]mask[0x%8.8x]\n",
            ii,
            currDbPtr->causeReg.registerName,
            currDbPtr->causeReg.registerAddrOffset,
            getInterruptRegAddr(devObjPtr,currDbPtr,GT_TRUE),/*cause*/
            getInterruptRegAddr(devObjPtr,currDbPtr,GT_FALSE)/*mask*/
            );
    }
}

void print_hawkInterruptsTreeDb_all_Interrupts(void)
{
    GT_U32  ii=0;
    SKERNEL_DEVICE_OBJECT* devObjPtr = smemTestDeviceIdToDevPtrConvert(0);
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr = devObjPtr->myInterruptsDbPtr;

    while(currDbPtr->causeReg.registersDbType != SKERNEL_REGISTERS_DB_TYPE__LAST___E)
    {
        print_curr(devObjPtr,currDbPtr,ii);
        ii ++;
        currDbPtr++;
    }

    printf("last index [%d] \n",
        ii-1);

}

static void print_child (IN SKERNEL_DEVICE_OBJECT* devObjPtr,IN SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr);
static void print_father(IN SKERNEL_DEVICE_OBJECT* devObjPtr,IN SKERNEL_INTERRUPT_REG_MY_FATHER_INFO * fatherDbPtr)
{
    if(fatherDbPtr->interruptPtr == NULL)
    {
        return;
    }
    printf("bit[%d] at --> ",fatherDbPtr->myBitIndex);
    print_child(devObjPtr,fatherDbPtr->interruptPtr);
}

static void print_child(IN SKERNEL_DEVICE_OBJECT* devObjPtr,IN SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr)
{
    print_curr(devObjPtr,currDbPtr,(-1));
    print_father(devObjPtr,&currDbPtr->myFatherInfo);
}

void print_hawkInterruptsTreeDb_childs_Interrupts(void)
{
    SKERNEL_DEVICE_OBJECT* devObjPtr = smemTestDeviceIdToDevPtrConvert(0);
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr = devObjPtr->myInterruptsDbPtr;
    GT_U32  ii = 0;

    while(currDbPtr->causeReg.registersDbType != SKERNEL_REGISTERS_DB_TYPE__LAST___E)
    {
        printf("*****************************\n");
        printf("index[%d] : ");
        print_child(devObjPtr,currDbPtr);
        ii++;
        currDbPtr++;
    }

    printf("*****************************\n");
    return;
}


