/******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* smemIronmanInterrupts.c
*
* DESCRIPTION:
*       Ironman interrupt tree related implementation
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <asicSimulation/SKernel/smem/smemIronman.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>

/* Ironman interrupt tree database */
static SKERNEL_INTERRUPT_REG_INFO_STC ironmanInterruptsTreeDb[MAX_INTERRUPT_NODES];

static SKERNEL_INTERRUPT_REG_INFO_STC skernel_interrupt_reg_info__last_line__cns[] =
{{{SKERNEL_REGISTERS_DB_TYPE__LAST___E}}};


#define cnm_grp_0_0_intr  (ironman_FuncUnitsInterruptsSummary  [0])
#define cnm_grp_0_1_intr  (ironman_dataPathInterruptSummary    [0])
#define cnm_grp_0_2_intr  (ironman_portsInterruptsSummary      [0])
#define cnm_grp_0_3_intr  (ironman_dfxInterruptsSummary        [0])
#define cnm_grp_0_4_intr  (ironman_dfx1InterruptsSummary       [0])
#define cnm_grp_0_5_intr  (ironman_FuncUnits1InterruptsSummary  [0])

#define cnm_grp_0_0_intr_x(x)  (ironman_FuncUnitsInterruptsSummary  [x])
#define cnm_grp_0_2_intr_x(x)  (ironman_portsInterruptsSummary  [x])


enum{  /*cnm_grp_0_0_intr  - functionalInterruptsSummaryCause/FuncUnitsInterruptsSummary */

    cnm_grp_0_0_cp_top_epcl_tai0_int_sum          = 2 ,
    cnm_grp_0_0_cp_top_epcl_tai1_int_sum          = 3 ,
    cnm_grp_0_0_cp_top_mt_int_sum                 = 4 ,
    cnm_grp_0_0_cp_top_pipe0_bma_int_sum          = 7 ,
    cnm_grp_0_0_cp_top_pipe0_cnc0_int_sum         = 8 ,
    cnm_grp_0_0_cp_top_pipe0_eft_int_sum          = 9 ,
    cnm_grp_0_0_cp_top_pipe0_epcl_int_sum         = 11,
    cnm_grp_0_0_cp_top_pipe0_eplr_int_sum         = 12,
    cnm_grp_0_0_cp_top_pipe0_eq_int_sum           = 13,
    cnm_grp_0_0_cp_top_pipe0_erep_int_sum         = 14,
    cnm_grp_0_0_cp_top_pipe0_ermrk_int_sum        = 15,
    cnm_grp_0_0_cp_top_pipe0_ha_int_sum           = 16,
    cnm_grp_0_0_cp_top_pipe0_hbu_int_sum          = 17,
    cnm_grp_0_0_cp_top_pipe0_ioam_int_sum         = 18,
    cnm_grp_0_0_cp_top_pipe0_iplr0_int_sum        = 19,
    cnm_grp_0_0_cp_top_pipe0_iplr1_int_sum        = 20,
    cnm_grp_0_0_cp_top_pipe0_ipvx_int_sum         = 21,
    cnm_grp_0_0_cp_top_pipe0_l2i_int_sum          = 22,
    cnm_grp_0_0_cp_top_pipe0_lpm_int_sum          = 23,
    cnm_grp_0_0_cp_top_pipe0_mll_int_sum          = 24,
    cnm_grp_0_0_cp_top_pipe0_pcl_int_sum          = 25,
    cnm_grp_0_0_cp_top_pipe0_ppu_int_sum          = 26,
    cnm_grp_0_0_cp_top_pipe0_preq_int_sum         = 27,
    cnm_grp_0_0_cp_top_pipe0_qag_int_sum          = 28,
    cnm_grp_0_0_cp_top_pipe0_sht_int_sum          = 29,
    cnm_grp_0_0_cp_top_pipe0_tti_int_sum          = 31
};


enum{ /*cnm_grp_0_1_intr_ - SERInterruptsSummary/dataPathInterruptSummary */
    cnm_grp_0_1_intr_Tcam_Int           = 3,
    cnm_grp_0_1_intr_Rxdma_Int          = 8,
    cnm_grp_0_1_intr_Txdma_Int          = 9,
    cnm_grp_0_1_intr_Txfifo_Int         = 11,
    cnm_grp_0_1_intr_Anp_Int_sum_0      = 13, /* USX_0*/
    cnm_grp_0_1_intr_Anp_Int_sum_1      = 14, /* USX_1*/
    cnm_grp_0_1_intr_Anp_Int_sum_2      = 15, /* USX_2*/
    cnm_grp_0_1_intr_Anp_Int_sum_3      = 16, /* USX_3*/
    cnm_grp_0_1_intr_Anp_Int_sum_4      = 17, /* USX_4*/
    cnm_grp_0_1_intr_Anp_Int_sum_5      = 18, /* USX_5*/
    cnm_grp_0_1_intr_Anp_Int_sum_6      = 19, /* USX_6*/
};

enum{ /*cnm_grp_0_2_intr_ - stackPortsInterruptCause/portsInterruptsSummary */
    cnm_grp_0_2_intr_usx2_mac_macro_TAI0_Int = 1,
    cnm_grp_0_2_intr_usx2_mac_macro_TAI1_Int = 2,
    cnm_grp_0_2_intr_LED_usx0_Int = 7,
    cnm_grp_0_2_intr_LED_usx1_Int = 8,
    cnm_grp_0_2_intr_usx_mac_pcs_0_Int = 13,
    cnm_grp_0_2_intr_usx_mac_pcs_1_Int = 14,
    cnm_grp_0_2_intr_usx_mac_pcs_2_Int = 15,
    cnm_grp_0_2_intr_usx_mac_pcs_3_Int = 16,
    cnm_grp_0_2_intr_usx_mac_pcs_4_Int = 17,
    cnm_grp_0_2_intr_usx_mac_pcs_5_Int = 18,
    cnm_grp_0_2_intr_usx_mac_pcs_6_Int = 19,
    cnm_grp_0_2_intr_mif_usx0_Int = 22,
    cnm_grp_0_2_intr_mif_usx1_Int = 23,
    cnm_grp_0_2_intr_mif_usx2_Int = 24,
    cnm_grp_0_2_intr_pzarb0_Int = 27,
    cnm_grp_0_2_intr_pzarb1_Int = 28,
    cnm_grp_0_2_intr_sdw0_Int = 29,
    cnm_grp_0_2_intr_sdw1_Int = 30,
    cnm_grp_0_2_intr_sdw2_Int = 31
};

enum{  /*cnm_grp_0_5_intr_  - functionalInterruptsSummary1Cause/FuncUnitsInterruptsSummary1 */
    cnm_grp_5_intr_ironman_MG1_int_out0_sum = 4,    /* MG1 */
    cnm_grp_5_intr_ironman_MG2_int_out0_sum = 7,    /* MG2 */

};

enum{
     MG_0_INDEX
    ,MG_1_INDEX
    ,MG_2_INDEX

    ,NUM_OF_MGS   /*3*/
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
    ,UNSED_MG/* MG 0_2 not used */

/* duplicate the node of MG to ALL MG_x_x */
#define DUP_ALL_MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt) \
     MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_0_INDEX,father_myBitIndex,father_interrupt[MG_0_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_1_INDEX,father_myBitIndex,father_interrupt[MG_1_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_2_INDEX,father_myBitIndex,father_interrupt[MG_2_INDEX])

static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_father_FuncUnits1InterruptsSummary[NUM_OF_MGS];
static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_FuncUnits1InterruptsSummary [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(functionalInterruptsSummary1Cause ,functionalInterruptsSummary1Mask ,FuncUnits1IntsSum ,ironman_father_FuncUnits1InterruptsSummary) };

static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_GlobalInterruptsSummary    [NUM_OF_MGS] =
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
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_1_INDEX,cnm_grp_5_intr_ironman_MG1_int_out0_sum,ironman_FuncUnits1InterruptsSummary[MG_0_INDEX])/* into MG_0_0 (tile 0)*/
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_2_INDEX,cnm_grp_5_intr_ironman_MG2_int_out0_sum,ironman_FuncUnits1InterruptsSummary[MG_0_INDEX])
};

static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_FuncUnitsInterruptsSummary [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(functionalInterruptsSummaryCause ,functionalInterruptsSummaryMask ,FuncUnitsIntsSum ,ironman_GlobalInterruptsSummary) };

static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_dataPathInterruptSummary   [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(SERInterruptsSummary             ,SERInterruptsMask               ,DataPathIntSum   ,ironman_GlobalInterruptsSummary) };

static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_portsInterruptsSummary     [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(stackPortsInterruptCause     ,stackPortsInterruptMask     ,PortsIntSum  ,ironman_GlobalInterruptsSummary) };


/* this node need support in primary and secondary MG units (to support CNC dump from the FUQ) */
static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_mg_global_misc_interrupts  [NUM_OF_MGS] =
{    DUP_ALL_MG_INTERTTUPT_REG(miscellaneousInterruptCause ,miscellaneousInterruptMask ,MgInternalIntSum ,ironman_GlobalInterruptsSummary) };


#define MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,mgIndex) \
     MG_SDMA_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,mgIndex,father_myBitIndex,father_interrupt[mgIndex])


/* duplicate the node of MG to all MG units */
#define DUP_SDMA_MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt) \
     MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_0_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_1_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_2_INDEX)

/* this node need support in ALL MG units (to MG SDMA) */
static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_mg_global_sdma_receiveSDMA_interrupts  [NUM_OF_MGS] =
{    DUP_SDMA_MG_INTERTTUPT_REG(receiveSDMAInterruptCause ,receiveSDMAInterruptMask ,RxSDMASum ,ironman_GlobalInterruptsSummary) };

/* this node need support in ALL MG units (to MG SDMA) */
static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_mg_global_sdma_transmitSDMA_interrupts  [NUM_OF_MGS] =
{    DUP_SDMA_MG_INTERTTUPT_REG(transmitSDMAInterruptCause ,transmitSDMAInterruptMask ,TxSDMASum ,ironman_GlobalInterruptsSummary) };

static SKERNEL_INTERRUPT_REG_INFO_STC cnc_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.CNCInterruptSummaryCauseReg)},/*SMEM_CHT3_CNC_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.CNCInterruptSummaryMaskReg)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_cnc0_int_sum ,
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
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_eft_int_sum,
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
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_ioam_int_sum ,
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
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_pcl_int_sum,
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
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_tti_int_sum ,
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
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_l2i_int_sum ,
        /*interruptPtr*/&cnm_grp_0_0_intr,
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC preq_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PREQ.preqInterrupts.preqInterruptCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PREQ.preqInterrupts.preqInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_preq_int_sum ,
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
        /*myBitIndex*/cnm_grp_0_0_cp_top_mt_int_sum ,
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
        /*myBitIndex*/cnm_grp_0_1_intr_Tcam_Int ,
        /*interruptPtr*/&cnm_grp_0_1_intr /* tile0 */
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
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_ha_int_sum ,
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
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_ermrk_int_sum ,
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
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_eq_int_sum ,
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

#define CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(n)                                                                        \
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
            /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_mll_int_sum ,
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
            /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_iplr0_int_sum ,
            /*interruptPtr*/&cnm_grp_0_0_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[1].policerInterruptCause)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[1].policerInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_iplr1_int_sum ,
            /*interruptPtr*/&cnm_grp_0_0_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[2].policerInterruptCause)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[2].policerInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_eplr_int_sum ,
            /*interruptPtr*/&cnm_grp_0_0_intr
            },
        /*isTriggeredByWrite*/ 1
    }
};

#define NUM_USX_PORTS_PER_GROUP 8

/* define the USX MAC global interrupt summary */
#define  USX_SUMMARY_GLOBAL_INTERRUPT_SUMMARY_MAC(group, bitInMg, regInMg) \
static SKERNEL_INTERRUPT_REG_INFO_STC  usx_global_interruptSummary_group##group = \
{                                                  \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[group].MTI_GLOBAL.globalInterruptSummaryCause)}, /**/  \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[group].MTI_GLOBAL.globalInterruptSummaryMask)},  /**/  \
    /*myFatherInfo*/{                              \
        /*myBitIndex*/ bitInMg ,                   \
        /*interruptPtr*/&regInMg                   \
        },                                         \
    /*isTriggeredByWrite*/ 1                       \
}

/* define the USX MAC global interrupt summary per group of USX ports 0..7 */
#define USX_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(group) \
    USX_SUMMARY_GLOBAL_INTERRUPT_SUMMARY_MAC(group, (cnm_grp_0_2_intr_usx_mac_pcs_0_Int + group)/*bitInMg*/, cnm_grp_0_2_intr/*regInMg*/)

USX_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(0);
USX_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(1);
USX_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(2);
USX_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(3);
USX_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(4);
USX_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(5);
USX_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(6);


/* node for USX mac ports interrupt cause */
#define USX_PORT_INTERRUPT_CAUSE_MAC(group, port)      \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[group * NUM_USX_PORTS_PER_GROUP + port].MTI_EXT_PORT.portInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[group * NUM_USX_PORTS_PER_GROUP + port].MTI_EXT_PORT.portInterruptMask) },  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
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

/* define the ANP1 global interrupt summary */
#define  ANP1_SUMMARY_GLOBAL_INTERRUPT_SUMMARY_MAC(group, bitInMg, regInMg) \
static SKERNEL_INTERRUPT_REG_INFO_STC  anp1_global_interruptSummary_group##group = \
{                                                  \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip_6_10_ANP[group].ANP.interruptSummaryCause)}, /**/  \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip_6_10_ANP[group].ANP.interruptSummaryMask)},  /**/  \
    /*myFatherInfo*/{                              \
        /*myBitIndex*/ bitInMg ,                   \
        /*interruptPtr*/&regInMg                   \
        },                                         \
    /*isTriggeredByWrite*/ 1                       \
}

/* define the USX MAC global interrupt summary per group of USX ports 0..7 */
#define ANP1_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(group) \
    ANP1_SUMMARY_GLOBAL_INTERRUPT_SUMMARY_MAC(group, (cnm_grp_0_1_intr_Anp_Int_sum_0 + group)/*bitInMg*/, cnm_grp_0_1_intr/*regInMg*/)

ANP1_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(0);
ANP1_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(1);
ANP1_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(2);
ANP1_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(3);
ANP1_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(4);
ANP1_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(5);
ANP1_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(6);


#define ANP1_PORT_INTERRUPT_CAUSE_MAC(group)      \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip_6_10_ANP[group].ANP.portInterruptCause[0])}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip_6_10_ANP[group].ANP.portInterruptMask[0])},  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                       \
        /*myBitIndex*/(2),    \
        /*interruptPtr*/&anp1_global_interruptSummary_group##group \
    },                                                      \
    /*isTriggeredByWrite*/ 1                                \
}

static SKERNEL_INTERRUPT_REG_INFO_STC ironman_mac_ports_interrupts[] =
{
     USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(0)
    ,USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(1)
    ,USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(2)
    ,USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(3)
    ,USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(4)
    ,USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(5)
    ,USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(6)

    ,ANP1_PORT_INTERRUPT_CAUSE_MAC(0)  /* ANP1 USX0 */
    ,ANP1_PORT_INTERRUPT_CAUSE_MAC(1)  /* ANP1 USX1 */
    ,ANP1_PORT_INTERRUPT_CAUSE_MAC(2)  /* ANP1 USX2 */
    ,ANP1_PORT_INTERRUPT_CAUSE_MAC(3)  /* ANP1 USX3 */
    ,ANP1_PORT_INTERRUPT_CAUSE_MAC(4)  /* ANP1 USX4 */
    ,ANP1_PORT_INTERRUPT_CAUSE_MAC(5)  /* ANP1 USX5 */
    ,ANP1_PORT_INTERRUPT_CAUSE_MAC(6)  /* ANP1 USX6 */
};

static SKERNEL_INTERRUPT_REG_INFO_STC RxDmaInterruptsSummary[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_rxDMA[0].interrupts.rxdma_interrupt_summary_cause)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_rxDMA[0].interrupts.rxdma_interrupt_summary_mask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_1_intr_Rxdma_Int ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
};


static SKERNEL_INTERRUPT_REG_INFO_STC rxdma_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_rxDMA[0].interrupts.rxdma_interrupt_0_cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_rxDMA[0].interrupts.rxdma_interrupt_0_mask)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&RxDmaInterruptsSummary[0]
            },
        /*isTriggeredByWrite*/ 1
    },
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_rxDMA[0].interrupts.rxdma_interrupt_1_cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_rxDMA[0].interrupts.rxdma_interrupt_1_mask)},
        /*myFatherInfo*/{
            /*myBitIndex*/2 ,
            /*interruptPtr*/&RxDmaInterruptsSummary[0]
            },
        /*isTriggeredByWrite*/ 1
    },
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_rxDMA[0].interrupts.rxdma_interrupt_2_cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_rxDMA[0].interrupts.rxdma_interrupt_2_mask)},
        /*myFatherInfo*/{
            /*myBitIndex*/3 ,
            /*interruptPtr*/&RxDmaInterruptsSummary[0]
            },
        /*isTriggeredByWrite*/ 1
    },
};

static SKERNEL_INTERRUPT_REG_INFO_STC txdma_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_txDMA[0].interrupts.txd_interrupt_cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_txDMA[0].interrupts.txd_interrupt_mask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_1_intr_Txdma_Int ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC txfifo_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_txFIFO[0].interrupts.txf_interrupt_cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_txFIFO[0].interrupts.txf_interrupt_mask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_1_intr_Txfifo_Int ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
};

/* get the MG index from the currMgInfoPtr->causeReg.mgUnitIndex */
#define   GET_MG_INDEX(currMgInfoPtr)    (currMgInfoPtr->causeReg.mgUnitIndex & (~MG_UNIT_USED_CNS))


/**
* @internal internal_smemIronmanInterruptTreeInit function
* @endinternal
*
* @brief   Init the interrupts tree for the Ironman device
*
* @param[in] devObjPtr                - pointer to device object.
*/
static GT_VOID internal_smemIronmanInterruptTreeInit
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

    {/* MG unit related interrupts */
        smemBobcat2GlobalInterruptTreeInit(devObjPtr, &index, &currDbPtr);

        /* for MG0 of the tile */
        mgIndex = 0;
        mgMax = NUM_OF_MGS;
        INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_FuncUnits1InterruptsSummary[mgIndex]);
/*        INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_portsInterruptsSummary[mgIndex]); */

        /* (skip MG[0]) add support for MG : 1,2 */
        mgIndex++;/* 1 */

        for(/* continue*/; mgIndex < mgMax ; mgIndex++)
        {
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_GlobalInterruptsSummary  [mgIndex]         );
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_mg_global_misc_interrupts[mgIndex]         );
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_mg_global_sdma_receiveSDMA_interrupts [mgIndex]);
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_mg_global_sdma_transmitSDMA_interrupts[mgIndex]);
        }
    }

    {   /* per pipe / tile interrupts */
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, tti_interrupts                    );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, bridge_interrupts                 );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, preq_interrupts                   );
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
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, rxdma_interrupts                  );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, txdma_interrupts                  );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, txfifo_interrupts                 );
    }

    *indexPtr = index;
    *currDbPtrPtr = currDbPtr;
}

/*******************************************************************************
*   smemIronmanInterruptTreeInit
*
* DESCRIPTION:
*       Init the interrupts tree for the Ironman device
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
GT_VOID smemIronmanInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 index = 0;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr;

    /* add here additions to 'compilation' info , that since 2 arrays point to
       each other we need to set it in runtime */

    /* Initialize Ports2 father Summary interrupts for primary MG units in all tiles */
    ironman_FuncUnits1InterruptsSummary[MG_0_INDEX].myFatherInfo.interruptPtr = &ironman_GlobalInterruptsSummary[MG_0_INDEX];

    devObjPtr->myInterruptsDbPtr = ironmanInterruptsTreeDb;

    currDbPtr = devObjPtr->myInterruptsDbPtr;

    internal_smemIronmanInterruptTreeInit(devObjPtr,&index,&currDbPtr);

    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, ironman_mac_ports_interrupts);

    /* End of interrupts - must be last */
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, skernel_interrupt_reg_info__last_line__cns);
}

void print_ironmanInterruptsTreeDb_mgInterrupts(void)
{
    GT_U32  ii=0;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr = ironmanInterruptsTreeDb;
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

void print_ironmanInterruptsTreeDb_all_Interrupts(void)
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

void print_ironmanInterruptsTreeDb_childs_Interrupts(void)
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



