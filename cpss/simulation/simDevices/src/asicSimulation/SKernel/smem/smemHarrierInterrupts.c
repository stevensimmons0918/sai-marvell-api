/******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* smemHarrierInterrupts.c
*
* DESCRIPTION:
*       Harrier interrupt tree related implementation
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <asicSimulation/SKernel/smem/smemHarrier.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>

/* Harrier interrupt tree database */
static SKERNEL_INTERRUPT_REG_INFO_STC harrierInterruptsTreeDb[MAX_INTERRUPT_NODES];

static SKERNEL_INTERRUPT_REG_INFO_STC skernel_interrupt_reg_info__last_line__cns[] =
{{{SKERNEL_REGISTERS_DB_TYPE__LAST___E}}};


#define cnm_grp_0_0_intr  (harrier_FuncUnitsInterruptsSummary  [0])
#define cnm_grp_0_1_intr  (harrier_dataPathInterruptSummary    [0])
#define cnm_grp_0_2_intr  (harrier_portsInterruptsSummary      [0])
#define cnm_grp_0_3_intr  (harrier_dfxInterruptsSummary        [0])
#define cnm_grp_0_4_intr  (harrier_dfx1InterruptsSummary       [0])

#define cnm_grp_0_7_intr  (harrier_ports2InterruptsSummary     [0])

#define cnm_grp_0_0_intr_x(x)  (harrier_FuncUnitsInterruptsSummary  [x])


enum{  /*cnm_grp_0_0_intr  - functionalInterruptsSummaryCause/FuncUnitsInterruptsSummary */
     cp_top_em_int_sum                                       =  1,
     cp_top_epcl_macro_tai_int_0_sum                         =  2,
     cp_top_epcl_macro_tai_int_1_sum                         =  3,
     cp_top_mt_int_sum                                       =  4,
     cp_top_pha_macro_tai_int_0_sum                          =  5,
     cp_top_pha_macro_tai_int_1_sum                          =  6,
     cp_top_pipe0_bma_int_sum                                =  7,
     cp_top_pipe0_cnc0_int_sum                               =  8,
     cp_top_pipe0_eft_int_sum                                =  9,
     cp_top_pipe0_eoam_int_sum                               = 10,
     cp_top_pipe0_epcl_int_sum                               = 11,
     cp_top_pipe0_eplr_int_sum                               = 12,
     cp_top_pipe0_eq_int_sum                                 = 13,
     cp_top_pipe0_erep_int_sum                               = 14,
     cp_top_pipe0_ermrk_int_sum                              = 15,
     cp_top_pipe0_ha_int_sum                                 = 16,
     cp_top_pipe0_hbu_int_sum                                = 17,
     cp_top_pipe0_ioam_int_sum                               = 18,
     cp_top_pipe0_iplr0_int_sum                              = 19,
     cp_top_pipe0_iplr1_int_sum                              = 20,
     cp_top_pipe0_ipvx_int_sum                               = 21,
     cp_top_pipe0_l2i_int_sum                                = 22,
     cp_top_pipe0_lpm_int_sum                                = 23,
     cp_top_pipe0_mll_int_sum                                = 24,
     cp_top_pipe0_pcl_int_sum                                = 25,
     cp_top_pipe0_ppu_int_sum                                = 26,
     cp_top_pipe0_preq_int_sum                               = 27,
     cp_top_pipe0_qag_int_sum                                = 28,
     cp_top_pipe0_sht_int_sum                                = 29,
     cp_top_pipe0_sip_pha_int_sum                            = 30,
     cp_top_pipe0_tti_int_sum                                = 31
};


enum{ /*cnm_grp_0_1_intr_ - dataPathInterruptSummary */
    cp_top_tcam_int_sum                       =  1,
    cp_top_tti_macro_tai_int_0_sum            =  2,
    cp_top_tti_macro_tai_int_1_sum            =  3,
    cp_top_l2i_ipvx_macro_tai_int_0_sum       =  4,
    cp_top_l2i_ipvx_macro_tai_int_1_sum       =  5,
    dp_top_ia0_ia_interrupt_0_cause_int_sum   =  6,
    dp_top_pb_int_sum                         =  7,
    dp_top_rx0_rxdma_cause_int_sum            =  8,
    dp_top_rx1_rxdma_cause_int_sum            =  9,
    dp_top_rx2_rxdma_cause_int_sum            = 10,
    dp_top_tx0_txd_interrupt_cause_sum        = 11,
    dp_top_tx0_txf_interrupt_cause_sum        = 12,
    dp_top_tx1_txd_interrupt_cause_sum        = 13,
    dp_top_tx1_txf_interrupt_cause_sum        = 14,
    dp_top_tx2_txd_interrupt_cause_sum        = 15,
    dp_top_tx2_txf_interrupt_cause_sum        = 16,
    gop_downlink_epi0_anp_int_sum                   = 17,
    gop_downlink_epi0_leds_int_sum                  = 18,
    gop_downlink_epi0_mac_pcs_int_sum               = 19,
    gop_downlink_epi0_mif_int_sum                   = 20,
    gop_downlink_epi0_sdw_int_0_sum                 = 21,
    gop_downlink_epi0_sdw_int_1_sum                 = 22,
    gop_downlink_epi1_anp_int_sum                   = 23,
    gop_downlink_epi1_leds_int_sum                  = 24,
    gop_downlink_epi1_mac_pcs_int_sum               = 25,
    gop_downlink_epi1_mif_int_sum                   = 26,
    gop_downlink_epi1_sdw_int_0_sum                 = 27,
    gop_downlink_epi1_sdw_int_1_sum                 = 28,
    gop_downlink_epi2_anp_int_sum                   = 29,
    gop_downlink_epi2_leds_int_sum                  = 30,
    gop_downlink_epi2_mac_pcs_int_sum               = 31
};

enum{ /*cnm_grp_0_2_intr_ - portsInterruptSummary */
    gop_downlink_top_epi2_mif_int_sum                 =  1,
    gop_downlink_top_epi2_sdw_int_sum                 =  2,
    gop_downlink_top_epi_mac_macro_i0_tai_int_0_sum   =  3,
    gop_downlink_top_epi_mac_macro_i0_tai_int_1_sum   =  4,
    gop_downlink_top_epi_mac_macro_i1_tai_int_0_sum   =  5,
    gop_downlink_top_epi_mac_macro_i1_tai_int_1_sum   =  6,
    gop_downlink_top_epi_mac_macro_i2_tai_int_0_sum   =  7,
    gop_downlink_top_epi_mac_macro_i2_tai_int_1_sum   =  8,
    gop_downlink_top_pca0_ctsu_int_sum                =  9,
    gop_downlink_top_pca0_dp2sdb_int_sum              = 10,
    gop_downlink_top_pca0_eip_163e_int_sum            = 11,
    gop_downlink_top_pca0_eip_163i_int_sum            = 12,
    gop_downlink_top_pca0_eip_164e_int_sum            = 13,
    gop_downlink_top_pca0_eip_164i_int_sum            = 14,
    gop_downlink_top_pca0_lmu_int_sum                 = 15,
    gop_downlink_top_pca0_pzarb_rx_int_sum            = 16,
    gop_downlink_top_pca0_pzarb_tx_int_sum            = 17,
    gop_downlink_top_pca0_sff2irq_int_sum             = 18,
    gop_downlink_top_pca1_ctsu_int_sum                = 19,
    gop_downlink_top_pca1_dp2sdb_int_sum              = 20,
    gop_downlink_top_pca1_eip_163e_int_sum            = 21,
    gop_downlink_top_pca1_eip_163i_int_sum            = 22,
    gop_downlink_top_pca1_eip_164e_int_sum            = 23,
    gop_downlink_top_pca1_eip_164i_int_sum            = 24,
    gop_downlink_top_pca1_lmu_int_sum                 = 25,
    gop_downlink_top_pca1_pzarb_rx_int_sum            = 26,
    gop_downlink_top_pca1_pzarb_tx_int_sum            = 27,
    gop_downlink_top_pca1_sff2irq_int_sum             = 28,
    gop_downlink_top_pca2_ctsu_int_sum                = 29,
    gop_downlink_top_pca2_dp2sdb_int_sum              = 30,
    gop_downlink_top_pca2_eip_163e_int_sum            = 31
};

enum{  /*cnm_grp_0_3_intr  - dfxInterruptsSummaryCause */
    gop_downlink_pca2_eip_163i_int_sum                          =  1,
    gop_downlink_pca2_eip_164e_int_sum                          =  2,
    gop_downlink_pca2_eip_164i_int_sum                          =  3,
    gop_downlink_pca2_lmu_int_sum                               =  4,
    gop_downlink_pca2_pzarb_rx_int_sum                          =  5,
    gop_downlink_pca2_pzarb_tx_int_sum                          =  6,
    gop_downlink_pca2_sff2irq_int_sum                           =  7,
    gop_downlink_pca_163e_macro_i0_tai_int_0_sum                =  8,
    gop_downlink_pca_163e_macro_i0_tai_int_1_sum                =  9,
    gop_downlink_pca_163e_macro_i1_tai_int_0_sum                = 10,
    gop_downlink_pca_163e_macro_i1_tai_int_1_sum                = 11,
    gop_downlink_pca_macro_tai_int_0_sum                        = 12,
    gop_downlink_pca_macro_tai_int_1_sum                        = 13,
    gop_downlink_server_int_sum                                 = 14,
    gop_downlink_pds0_int_sum                                   = 15,
    gop_downlink_pds1_int_sum                                   = 16,
    gop_downlink_pds2_int_sum                                   = 17,
    gop_downlink_pdx_interrupt_sum                              = 18,
    gop_downlink_pfcc_int_sum                                   = 19,
    gop_downlink_psi_int_sum                                    = 20,
    gop_downlink_qfc0_int_sum                                   = 21,
    gop_downlink_qfc1_int_sum                                   = 22,
    gop_downlink_qfc2_int_sum                                   = 23,
    gop_downlink_sdq0_int_sum                                   = 24,
    gop_downlink_sdq1_int_sum                                   = 25,
    gop_downlink_sdq2_int_sum                                   = 26,
    gop_downlink_txqp_macro_tai_int_0_sum                       = 27,
    gop_downlink_txqs_macro_i0_tai_int_0_sum                    = 28,
    gop_downlink_txqs_macro_i1_tai_int_0_sum                    = 29
};

enum{ /*cnm_grp_0_7_intr - ports2InterruptsSummaryCause/ports2InterruptsSummary */
    Mg0_Int_Out0                                                            =  1,
    Mg0_Int_Out1                                                            =  2,
    Mg0_Int_Out2                                                            =  3,
    Mg1_Int_Out0                                                            =  4,
    Mg1_Int_Out1                                                            =  5,
    Mg1_Int_Out2                                                            =  6
};

enum{
     MG_0_INDEX
    ,MG_1_INDEX

    ,NUM_OF_MGS   /*2*/
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
    ,UNSED_MG/* MG 0_1 not used */


/* duplicate the node of MG to MG_x_0 and MG_x_1 (primary + secondary MG units) */
#define DUP_PRIMARY_AND_SECONDARY_MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt) \
     MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_0_INDEX,father_myBitIndex,father_interrupt[MG_0_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_1_INDEX,father_myBitIndex,father_interrupt[MG_1_INDEX])

/* duplicate the node of MG to ALL MG_x_x */
#define DUP_ALL_MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt) \
     MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_0_INDEX,father_myBitIndex,father_interrupt[MG_0_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_1_INDEX,father_myBitIndex,father_interrupt[MG_1_INDEX])

static SKERNEL_INTERRUPT_REG_INFO_STC  harrier_ports2FatherInterruptsSummary[NUM_OF_MGS];

static SKERNEL_INTERRUPT_REG_INFO_STC  harrier_ports2InterruptsSummary    [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(ports2InterruptsSummaryCause     ,ports2InterruptsSummaryMask     ,ports_2_int_sum  ,harrier_ports2FatherInterruptsSummary)  };



static SKERNEL_INTERRUPT_REG_INFO_STC  harrier_GlobalInterruptsSummary    [NUM_OF_MGS] =
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
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask, MG_1_INDEX, Mg1_Int_Out0, harrier_ports2InterruptsSummary[MG_0_INDEX])/* into MG_0_0 (tile 0)*/
};

static SKERNEL_INTERRUPT_REG_INFO_STC  harrier_FuncUnitsInterruptsSummary [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(functionalInterruptsSummaryCause ,functionalInterruptsSummaryMask ,FuncUnitsIntsSum ,harrier_GlobalInterruptsSummary) };
static SKERNEL_INTERRUPT_REG_INFO_STC  harrier_dataPathInterruptSummary   [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(SERInterruptsSummary             ,SERInterruptsMask               ,DataPathIntSum   ,harrier_GlobalInterruptsSummary) };
static SKERNEL_INTERRUPT_REG_INFO_STC  harrier_dfxInterruptsSummary       [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(dfxInterruptsSummaryCause        ,dfxInterruptsSummaryMask         ,dfxIntSum       ,harrier_GlobalInterruptsSummary) };
static SKERNEL_INTERRUPT_REG_INFO_STC  harrier_portsInterruptsSummary     [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(stackPortsInterruptCause     ,stackPortsInterruptMask             ,PortsIntSum      ,harrier_GlobalInterruptsSummary) };

/* this node need support in primary and secondary MG units (to support CNC dump from the FUQ) */
static SKERNEL_INTERRUPT_REG_INFO_STC  harrier_mg_global_misc_interrupts  [NUM_OF_MGS] =
{    DUP_ALL_MG_INTERTTUPT_REG(miscellaneousInterruptCause ,miscellaneousInterruptMask ,MgInternalIntSum ,harrier_GlobalInterruptsSummary) };


#define MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,mgIndex) \
     MG_SDMA_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,mgIndex,father_myBitIndex,father_interrupt[mgIndex])


/* duplicate the node of MG to all MG units */
#define DUP_SDMA_MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt) \
     MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_0_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_1_INDEX)

/* this node need support in ALL MG units (to MG SDMA) */
static SKERNEL_INTERRUPT_REG_INFO_STC  harrier_mg_global_sdma_receiveSDMA_interrupts  [NUM_OF_MGS] =
{    DUP_SDMA_MG_INTERTTUPT_REG(receiveSDMAInterruptCause ,receiveSDMAInterruptMask ,RxSDMASum ,harrier_GlobalInterruptsSummary) };

/* this node need support in ALL MG units (to MG SDMA) */
static SKERNEL_INTERRUPT_REG_INFO_STC  harrier_mg_global_sdma_transmitSDMA_interrupts  [NUM_OF_MGS] =
{    DUP_SDMA_MG_INTERTTUPT_REG(transmitSDMAInterruptCause ,transmitSDMAInterruptMask ,TxSDMASum ,harrier_GlobalInterruptsSummary) };

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
        /*interruptPtr*/&cnm_grp_0_0_intr
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

static SKERNEL_INTERRUPT_REG_INFO_STC lmu_summary_cause_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(LMU[0].summary_cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(LMU[0].summary_mask)},
        /*myFatherInfo*/{
            /*myBitIndex*/gop_downlink_top_pca0_lmu_int_sum ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(LMU[1].summary_cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(LMU[1].summary_mask)},
        /*myFatherInfo*/{
            /*myBitIndex*/gop_downlink_top_pca1_lmu_int_sum,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(LMU[2].summary_cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(LMU[2].summary_mask)},
        /*myFatherInfo*/{
            /*myBitIndex*/gop_downlink_pca2_lmu_int_sum,
            /*interruptPtr*/&harrier_dfxInterruptsSummary[0]
            },
        /*isTriggeredByWrite*/ 1
    }
};

#define harrier_LMU_latencyOverThresholdCause(lmuIndex, k)                                                  \
    {                                                                                                       \
        /*causeReg*/ { SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(LMU[lmuIndex].latencyOverThreshold_cause[k])},   \
        /*causeReg*/ { SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(LMU[lmuIndex].latencyOverThreshold_mask[k])},    \
        /*myFatherInfo*/{                                                                                   \
            /*myBitIndex*/1 + k,                                                                            \
            /*interruptPtr*/&lmu_summary_cause_interrupts[lmuIndex]                                         \
        },                                                                                                  \
        /*isTriggeredByWrite*/ 1                                                                            \
    }

#define HARRIER_LMU_LATENCY_OVER_THRESHOLD_MAC(lmuIndex)                                                    \
         harrier_LMU_latencyOverThresholdCause(lmuIndex,  0)                                                \
        ,harrier_LMU_latencyOverThresholdCause(lmuIndex,  1)                                                \
        ,harrier_LMU_latencyOverThresholdCause(lmuIndex,  2)                                                \
        ,harrier_LMU_latencyOverThresholdCause(lmuIndex,  3)                                                \
        ,harrier_LMU_latencyOverThresholdCause(lmuIndex,  4)                                                \
        ,harrier_LMU_latencyOverThresholdCause(lmuIndex,  5)                                                \
        ,harrier_LMU_latencyOverThresholdCause(lmuIndex,  6)                                                \
        ,harrier_LMU_latencyOverThresholdCause(lmuIndex,  7)                                                \
        ,harrier_LMU_latencyOverThresholdCause(lmuIndex,  8)                                                \
        ,harrier_LMU_latencyOverThresholdCause(lmuIndex,  9)                                                \
        ,harrier_LMU_latencyOverThresholdCause(lmuIndex, 10)                                                \
        ,harrier_LMU_latencyOverThresholdCause(lmuIndex, 11)                                                \
        ,harrier_LMU_latencyOverThresholdCause(lmuIndex, 12)                                                \
        ,harrier_LMU_latencyOverThresholdCause(lmuIndex, 13)                                                \
        ,harrier_LMU_latencyOverThresholdCause(lmuIndex, 14)                                                \
        ,harrier_LMU_latencyOverThresholdCause(lmuIndex, 15)                                                \
        ,harrier_LMU_latencyOverThresholdCause(lmuIndex, 16)

static SKERNEL_INTERRUPT_REG_INFO_STC lmu_latency_over_threshold_interrupts[] =
{
     HARRIER_LMU_LATENCY_OVER_THRESHOLD_MAC(0)
    ,HARRIER_LMU_LATENCY_OVER_THRESHOLD_MAC(1)
    ,HARRIER_LMU_LATENCY_OVER_THRESHOLD_MAC(2)
};

#define NUM_PORTS_PER_GROUP 8

/* 400G MAC global interrupt summary */
static SKERNEL_INTERRUPT_REG_INFO_STC ports_400G_global_interruptSummary[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[0].MTI_GLOBAL.globalInterruptSummaryCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[0].MTI_GLOBAL.globalInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/gop_downlink_epi0_mac_pcs_int_sum,
        /*interruptPtr*/&cnm_grp_0_1_intr
        },
    /*isTriggeredByWrite*/ 1
    }
    ,
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[1].MTI_GLOBAL.globalInterruptSummaryCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[1].MTI_GLOBAL.globalInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/gop_downlink_epi1_mac_pcs_int_sum,
        /*interruptPtr*/&cnm_grp_0_1_intr
        },
    /*isTriggeredByWrite*/ 1
    }
    ,
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[2].MTI_GLOBAL.globalInterruptSummaryCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[2].MTI_GLOBAL.globalInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/gop_downlink_epi2_mac_pcs_int_sum,
        /*interruptPtr*/&cnm_grp_0_1_intr
        },
    /*isTriggeredByWrite*/ 1
    }
};


/* node for 400G ports interrupt cause */
#define PORT_400G_INTERRUPT_CAUSE_MAC(group, port)      \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[group*16 + port*2].MTI_EXT_PORT.portInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[group*16 + port*2].MTI_EXT_PORT.portInterruptMask)},  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                       \
        /*myBitIndex*/(1+port),    \
        /*interruptPtr*/&ports_400G_global_interruptSummary[group] \
    },                                                      \
    /*isTriggeredByWrite*/ 1                                \
}

/* node for SEG ports interrupt cause */
#define PORT_SEG_INTERRUPT_CAUSE_MAC(group, port)      \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[group*16 + port*8].MTI_EXT_PORT.segPortInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[group*16 + port*8].MTI_EXT_PORT.segPortInterruptMask)},  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                       \
        /*myBitIndex*/(1+8+port),    \
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

/* GOP_2 : only half the MACs : node for 400G ports interrupt cause */
#define GOP_2_PORT_400G_PER_GROUP_INTERRUPT_CAUSE_MAC(group)      \
     PORT_400G_INTERRUPT_CAUSE_MAC(group, 0)                \
    ,PORT_400G_INTERRUPT_CAUSE_MAC(group, 1)                \
    ,PORT_400G_INTERRUPT_CAUSE_MAC(group, 2)                \
    ,PORT_400G_INTERRUPT_CAUSE_MAC(group, 3)

#define GOP_2_PORT_SEG_PER_GROUP_INTERRUPT_CAUSE_MAC(group)       \
     PORT_SEG_INTERRUPT_CAUSE_MAC(group, 0)

static SKERNEL_INTERRUPT_REG_INFO_STC  anp8_global_interruptSummary[] =
{
    {                                               \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip_6_10_ANP[0].ANP.interruptSummaryCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip_6_10_ANP[0].ANP.interruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/ gop_downlink_epi0_anp_int_sum ,
        /*interruptPtr*/&cnm_grp_0_1_intr
        },
    /*isTriggeredByWrite*/ 1
    },
    {                                               \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip_6_10_ANP[1].ANP.interruptSummaryCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip_6_10_ANP[1].ANP.interruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/ gop_downlink_epi1_anp_int_sum ,
        /*interruptPtr*/&cnm_grp_0_1_intr
        },
    /*isTriggeredByWrite*/ 1
    },
    {                                               \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip_6_10_ANP[2].ANP.interruptSummaryCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip_6_10_ANP[2].ANP.interruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/ gop_downlink_epi2_anp_int_sum ,
        /*interruptPtr*/&cnm_grp_0_1_intr
        },
    /*isTriggeredByWrite*/ 1
    },

};

#define ANP8_PORT_INTERRUPT_CAUSE_MAC(group, port)          \
{                                                           \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip_6_10_ANP[group].ANP.portInterruptCause[port])}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip_6_10_ANP[group].ANP.portInterruptMask[port])},  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                       \
        /*myBitIndex*/(2+port*2),                           \
        /*interruptPtr*/&anp8_global_interruptSummary[group]\
    },                                                      \
    /*isTriggeredByWrite*/ 1                                \
}

/* node for 400G ports interrupt cause */
#define ANP8_PER_GROUP_INTERRUPT_CAUSE_MAC(group)           \
     ANP8_PORT_INTERRUPT_CAUSE_MAC(group, 0)                \
    ,ANP8_PORT_INTERRUPT_CAUSE_MAC(group, 1)                \
    ,ANP8_PORT_INTERRUPT_CAUSE_MAC(group, 2)                \
    ,ANP8_PORT_INTERRUPT_CAUSE_MAC(group, 3)                \
    ,ANP8_PORT_INTERRUPT_CAUSE_MAC(group, 4)                \
    ,ANP8_PORT_INTERRUPT_CAUSE_MAC(group, 5)                \
    ,ANP8_PORT_INTERRUPT_CAUSE_MAC(group, 6)                \
    ,ANP8_PORT_INTERRUPT_CAUSE_MAC(group, 7)

static SKERNEL_INTERRUPT_REG_INFO_STC harrier_mac_ports_interrupts[] =
{
     PORT_400G_PER_GROUP_INTERRUPT_CAUSE_MAC(0)
    ,PORT_400G_PER_GROUP_INTERRUPT_CAUSE_MAC(1)
    ,GOP_2_PORT_400G_PER_GROUP_INTERRUPT_CAUSE_MAC(2)
    ,PORT_SEG_PER_GROUP_INTERRUPT_CAUSE_MAC(0)
    ,PORT_SEG_PER_GROUP_INTERRUPT_CAUSE_MAC(1)
    ,GOP_2_PORT_SEG_PER_GROUP_INTERRUPT_CAUSE_MAC(2)

    ,ANP8_PER_GROUP_INTERRUPT_CAUSE_MAC(0)/* ANP8 400 */
    ,ANP8_PER_GROUP_INTERRUPT_CAUSE_MAC(1)/* ANP8 400 */
    ,ANP8_PER_GROUP_INTERRUPT_CAUSE_MAC(2)/* ANP8 400 */

};

static SKERNEL_INTERRUPT_REG_INFO_STC PHA_summary_Interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.pha_regs.errorsAndInterrupts.PHAInterruptSumCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.pha_regs.errorsAndInterrupts.PHAInterruptSumMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cp_top_pipe0_sip_pha_int_sum,
            /*interruptPtr*/&cnm_grp_0_0_intr
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
    ,PPN_IN_PPG_INTERRUPT_CAUSE_NODE(ppgId,7)


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
    PPG_CAUSE_NODE(1)

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


#define HARRIER_TSU_GLOBAL_UNIT_CHANNEL_INTERRUPT_SUMMARY_MAC(bit, unit)                                                                            \
{                                                                                                                                               \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].ctsuGlobalInterruptSummaryCause)},                                         \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].ctsuGlobalInterrupSummarytMask)},                                          \
    /*myFatherInfo*/{                                                                                                                           \
        /*myBitIndex*/ bit,                                                                                                                     \
        /*interruptPtr*/&cnm_grp_0_2_intr                                                                                                       \
        },                                                                                                                                      \
    /*isTriggeredByWrite*/ 1                                                                                                                    \
}

static SKERNEL_INTERRUPT_REG_INFO_STC  harrierTsuInterruptSummary[] =
{ 
    HARRIER_TSU_GLOBAL_UNIT_CHANNEL_INTERRUPT_SUMMARY_MAC( 9, 0),
    HARRIER_TSU_GLOBAL_UNIT_CHANNEL_INTERRUPT_SUMMARY_MAC(19, 1),
    HARRIER_TSU_GLOBAL_UNIT_CHANNEL_INTERRUPT_SUMMARY_MAC(29, 2)
};

#define HARRIER_TSU_UNIT_CHANNEL_0_15_INTERRUPT_SUMMARY_MAC(unit)                                                                           \
{                                                                                                                                           \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].channel_0_15_InterruptSummaryCause)},                                  \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].channel_0_15_InterruptSummaryMask)},                                   \
    /*myFatherInfo*/{                                                                                                                       \
        /*myBitIndex*/2 ,                                                                                                                   \
        /*interruptPtr*/&harrierTsuInterruptSummary[unit]                                                                                   \
        },                                                                                                                                  \
    /*isTriggeredByWrite*/ 1                                                                                                                \
}

#define HARRIER_TSU_UNIT_CHANNEL_16_31_INTERRUPT_SUMMARY_MAC(unit)                                                                          \
{                                                                                                                                           \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].channel_16_31_InterruptSummaryCause)},                                 \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].channel_16_31_InterruptSummaryMask)},                                  \
    /*myFatherInfo*/{                                                                                                                       \
        /*myBitIndex*/3 ,                                                                                                                   \
        /*interruptPtr*/&harrierTsuInterruptSummary[unit]                                                                                   \
        },                                                                                                                                  \
    /*isTriggeredByWrite*/ 1                                                                                                                \
}

static SKERNEL_INTERRUPT_REG_INFO_STC harrierTsuChannelInterrupt_0_15_Summary[] =
{
    HARRIER_TSU_UNIT_CHANNEL_0_15_INTERRUPT_SUMMARY_MAC(0)
   ,HARRIER_TSU_UNIT_CHANNEL_0_15_INTERRUPT_SUMMARY_MAC(1)
   ,HARRIER_TSU_UNIT_CHANNEL_0_15_INTERRUPT_SUMMARY_MAC(2)
};

static SKERNEL_INTERRUPT_REG_INFO_STC harrierTsuChannelInterrupt_16_31_Summary[] =
{
    HARRIER_TSU_UNIT_CHANNEL_16_31_INTERRUPT_SUMMARY_MAC(0)
   ,HARRIER_TSU_UNIT_CHANNEL_16_31_INTERRUPT_SUMMARY_MAC(1)
   ,HARRIER_TSU_UNIT_CHANNEL_16_31_INTERRUPT_SUMMARY_MAC(2)
};

/* node for TSU channel interrupt cause registers */
#define  GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(unit, index)                                                                                         \
{                                                                                                                                               \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].channelInterruptCause[(index)])}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/          \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[unit].channelInterruptMask[(index)])},  /*SMEM_CHT_PORT_INT_CAUSE_MASK_REG*/     \
    /*myFatherInfo*/{                                                                                                                           \
        /*myBitIndex*/(((index)%16)+1),                                                                                                         \
        /*interruptPtr*/(((((index)/16) % 2) == 0) ? &harrierTsuChannelInterrupt_0_15_Summary[unit] : &harrierTsuChannelInterrupt_16_31_Summary[unit])   \
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
};


/**
* @internal internal_smemHarrierInterruptTreeInit function
* @endinternal
*
* @brief   Init the interrupts tree for the Harrier device
*
* @param[in] devObjPtr                - pointer to device object.
*/
static GT_VOID internal_smemHarrierInterruptTreeInit
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



    { /* MG unit related interrupts */
        smemBobcat2GlobalInterruptTreeInit(devObjPtr, &index, &currDbPtr);

        /* for MG0 of the tile */
        mgIndex = 0;
        mgMax = NUM_OF_MGS;
        INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &harrier_ports2InterruptsSummary[mgIndex]);

        /* (skip MG[0]) add support for MG1, */
        mgIndex++;/* 1 */

        for(/* continue*/; mgIndex < mgMax ; mgIndex++)
        {
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &harrier_GlobalInterruptsSummary  [mgIndex]         );
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &harrier_mg_global_misc_interrupts[mgIndex]         );
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &harrier_mg_global_sdma_receiveSDMA_interrupts [mgIndex]);
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &harrier_mg_global_sdma_transmitSDMA_interrupts[mgIndex]);
        }
    }

    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, harrier_mac_ports_interrupts);
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, lmu_latency_over_threshold_interrupts);

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
*   smemHarrierInterruptTreeInit
*
* DESCRIPTION:
*       Init the interrupts tree for the Harrier device
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
GT_VOID smemHarrierInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 index = 0;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr;

    /* add here additions to 'compilation' info , that since 2 arrays point to
       each other we need to set it in runtime */

    /* Initialize Ports2 father Summary interrupts for primary MG units in all tiles */
    harrier_ports2InterruptsSummary[MG_0_INDEX].myFatherInfo.interruptPtr = &harrier_GlobalInterruptsSummary[MG_0_INDEX];

    devObjPtr->myInterruptsDbPtr = harrierInterruptsTreeDb;

    currDbPtr = devObjPtr->myInterruptsDbPtr;

    internal_smemHarrierInterruptTreeInit(devObjPtr,&index,&currDbPtr);

    /* End of interrupts - must be last */
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, skernel_interrupt_reg_info__last_line__cns);
}

void print_harrierInterruptsTreeDb_mgInterrupts(void)
{
    GT_U32  ii=0;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr = harrierInterruptsTreeDb;
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

void print_harrierInterruptsTreeDb_all_Interrupts(void)
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

void print_harrierInterruptsTreeDb_childs_Interrupts(void)
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


