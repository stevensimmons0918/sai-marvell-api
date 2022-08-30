/******************************************************************************
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
* @file smemPipeInterrupts.c
*
* @brief Pipe interrupts tree mapping implementation
*
* @version   1
********************************************************************************
*/

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/smem/smemPipe.h>
#include <asicSimulation/SKernel/smem/smemBobcat2.h>
#include <asicSimulation/SKernel/smem/smemBobcat3.h>
#include <asicSimulation/SKernel/cheetahCommon/sregLion2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregBobcat2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/cheetahCommon/sregPipe.h>

enum {
    DataPath_txfifo_int_sum = 1,
    DataPath_rxdma_int_sum  = 2,
    DataPath_bm_int_sum     = 3,
    DataPath_bma_int_sum    = 4,
    DataPath_txdma_int_sum  = 5,
    DataPath_mppm_int_sum   = 6
};

enum{
    functional_units_dip_mg0_int_out0                            = 1,
    functional_units_dip_mg0_int_out1                            = 2,
    functional_units_dip_mg0_int_out2                            = 3,
    functional_units_dip_mg1_int_out0                            = 4,
    functional_units_dip_mg1_int_out1                            = 5,
    functional_units_dip_mg1_int_out2                            = 6,
    functional_units_txq_ll_int_sum                              = 7,
    functional_units_txq_queue_int_sum                           = 8,
    functional_units_txq_qcn_int_sum                             = 9,
    functional_units_txq_dq_int_sum                              = 10,
    functional_units_txq_dq1_int_sum                             = 11,
    functional_units_pfc_interrupt_summary_cause_sum             = 12,
    functional_units_mcfc_interrupt_cause_reg_sum                = 13,
    functional_units_cnc_interrupt_cause_reg_sum                 = 14,
    functional_units_pcp_interrupt_cause_reg_sum                 = 15,
    functional_units_sip_pha_interrupt_cause_sum_reg             = 16,
    functional_units_iunit_interrupts_cause_sum                  = 17,
    functional_units_munit_intr                                  = 18,
    functional_units_gpio_7_0_intr                               = 19,
    functional_units_gpio_15_8_intr                              = 20,
    functional_units_gpio_23_16_intr                             = 21,
    functional_units_gpio_31_24_intr                             = 22,
    functional_units_i2c_intr                                    = 23,
    functional_units_i2c_1_intr                                  = 24,
    functional_units_tai_int                                     = 25,
    functional_units_r2intr_spi_a_rdy                            = 26,
    functional_units_sbc2mg_interrupt_cause_sum                  = 27,
    functional_units_sd2mg_interrupt_cause_sum                   = 28,
    functional_units_server_interrupt_summary_cause_sum          = 29
};


static SKERNEL_INTERRUPT_REG_INFO_STC MppmInterruptsSummary =
{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MPPM.interrupts.MPPMInterruptsSummary)},/**/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MPPM.interrupts.MPPMInterruptsSummaryMask)},/**/
    /*myFatherInfo*/{
        /*myBitIndex*/DataPath_mppm_int_sum ,
        /*interruptPtr*/&dataPathInterruptSummary
        },
    /*isTriggeredByWrite*/ 1,
};

static SKERNEL_INTERRUPT_REG_INFO_STC mppm_ecc_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MPPM.ECC.interrupts.EccErrorInterruptCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MPPM.ECC.interrupts.EccErrorInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/27 ,
        /*interruptPtr*/&MppmInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC  BMInterruptSummary =
{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(BM.BMInterrupts.summaryInterruptCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(BM.BMInterrupts.summaryInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/DataPath_bm_int_sum ,
        /*interruptPtr*/&dataPathInterruptSummary
        },
    /*isTriggeredByWrite*/ 1
};

static SKERNEL_INTERRUPT_REG_INFO_STC bm_general_cause_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(BM.BMInterrupts.generalInterruptCause1)},/*BM General Cause Reg1*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(BM.BMInterrupts.generalInterruptMask1)},/*BM General Mask Reg1*/
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&BMInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC RxDmaInterruptsSummary[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[0].interrupts.rxDMAInterruptSummaryCause)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[0].interrupts.rxDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/DataPath_rxdma_int_sum ,
            /*interruptPtr*/&dataPathInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC rxdma_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[0].interrupts.rxDMAInterrupt0Cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[0].interrupts.rxDMAInterrupt0Cause)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&RxDmaInterruptsSummary[0]
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC TxDmaInterruptsSummary[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[0].interrupts.txDMAInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[0].interrupts.txDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/DataPath_txdma_int_sum ,
            /*interruptPtr*/&dataPathInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC txdma_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[0].interrupts.txDMAInterruptCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[0].interrupts.txDMAInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/4 ,
            /*interruptPtr*/&TxDmaInterruptsSummary[0]
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC TxFifoInterruptsSummary[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[0].interrupts.txFIFOInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[0].interrupts.txFIFOInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/DataPath_txfifo_int_sum ,
            /*interruptPtr*/&dataPathInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC txfifo_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[0].interrupts.txFIFOInterruptCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[0].interrupts.txFIFOInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&TxFifoInterruptsSummary[0]
            },
        /*isTriggeredByWrite*/ 1
    }
};

/* node for XLG mac ports */
#define  XLG_MAC_INTERRUPT_CAUSE_MAC(port) \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(GOP.ports.XLGIP[port].portInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(GOP.ports.XLGIP[port].portInterruptMask)},  /**/  \
    /*myFatherInfo*/{                                     \
        /*myBitIndex*/1 ,                                 \
        /*interruptPtr*/&xlgInterruptSummary_##port       \
        },                                                \
    /*isTriggeredByWrite*/ 1                              \
}

/* define the XLG port summary directed from MG ports summary */
#define  DEFINE_XLG_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(port, bitInMg, regInMg) \
static SKERNEL_INTERRUPT_REG_INFO_STC  xlgInterruptSummary_##port = \
{                                                  \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(GOP.ports.XLGIP[port].externalUnitsInterruptsCause) }, /**/  \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(GOP.ports.XLGIP[port].externalUnitsInterruptsMask) },  /**/  \
    /*myFatherInfo*/{                              \
        /*myBitIndex*/ bitInMg ,                   \
        /*interruptPtr*/&regInMg                   \
        },                                         \
    /*isTriggeredByWrite*/ 1                       \
}

/* define the Gig port summary directed from XLG summary */
#define  DEFINE_GIGA_SUMMARY_PORT_IN_XLG_INTERRUPT_SUMMARY_MAC(port) \
static SKERNEL_INTERRUPT_REG_INFO_STC  gigaInterruptSummary_##port = \
{                                                  \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(GOP.ports.gigPort[port].portInterruptSummaryCause)}, /**/  \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(GOP.ports.gigPort[port].portInterruptSummaryMask)},  /**/  \
    /*myFatherInfo*/{                               \
        /*myBitIndex*/ 2 ,                          \
        /*interruptPtr*/&xlgInterruptSummary_##port \
        },                                          \
    /*isTriggeredByWrite*/ 1                        \
}

/* define the XLG port summary, for port 0..30 in ports_int_sum per pipe 0  */
#define  DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC(port) \
    DEFINE_XLG_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(port, (port + 1)/*bitInMg*/,portsInterruptsSummary/*regInMg*/);\
    DEFINE_GIGA_SUMMARY_PORT_IN_XLG_INTERRUPT_SUMMARY_MAC(port)

DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC ( 0);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC ( 1);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC ( 2);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC ( 3);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC ( 4);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC ( 5);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC ( 6);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC ( 7);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC ( 8);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC ( 9);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (10);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (11);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (12);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (13);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (14);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (15);


static SKERNEL_INTERRUPT_REG_INFO_STC xlg_mac_ports_interrupts[] =
{
    /* the ports 0..35 pipe 0 */
     XLG_MAC_INTERRUPT_CAUSE_MAC( 0)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC( 1)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC( 2)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC( 3)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC( 4)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC( 5)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC( 6)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC( 7)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC( 8)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC( 9)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(10)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(11)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(12)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(13)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(14)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(15)
};

/* define the XLG port summary directed from MG ports summary */
#define  DEFINE_CG_100G_EXTERNAL_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(port, bitInMg, regInMg) \
static SKERNEL_INTERRUPT_REG_INFO_STC  cg100gInterruptExternalSummary_##port = \
{                                                  \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(GOP.ports_100G_CG_MAC[port].CG_CONVERTERS.CGMAConvertersExternal_Units_Interrupts_cause)}, /**/  \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(GOP.ports_100G_CG_MAC[port].CG_CONVERTERS.CGMAConvertersExternal_Units_Interrupts_mask)},  /**/  \
    /*myFatherInfo*/{                              \
        /*myBitIndex*/ bitInMg ,                   \
        /*interruptPtr*/&regInMg                   \
        },                                         \
    /*isTriggeredByWrite*/ 1                       \
}

/* define the CG-100G port summary, for port 0..30 in ports_int_sum per pipe 0  */
#define  DEFINE_CG_100G_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC(port) \
    DEFINE_CG_100G_EXTERNAL_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(port, (port + 1)/*bitInMg*/,portsInterruptsSummary/*regInMg*/)

DEFINE_CG_100G_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC(12);

/* node for CG mac ports */
#define  CG_MAC_100G_INTERRUPT_CAUSE_MAC(port)               \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(GOP.ports_100G_CG_MAC[port].CG_CONVERTERS.CGMAConvertersCG_Interrupt_cause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(GOP.ports_100G_CG_MAC[port].CG_CONVERTERS.CGMAConvertersCG_Interrupt_mask) },  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                     \
        /*myBitIndex*/1 ,                                 \
        /*interruptPtr*/&cg100gInterruptExternalSummary_##port  \
    },                                                    \
    /*isTriggeredByWrite*/ 1                              \
}

static SKERNEL_INTERRUPT_REG_INFO_STC cg_100g_mac_ports_interrupts[] =
{
     CG_MAC_100G_INTERRUPT_CAUSE_MAC(12)
};

/* node for Gig mac ports */
#define  GIGA_MAC_INTERRUPT_CAUSE_MAC(port)               \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(GOP.ports.gigPort[port].portInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(GOP.ports.gigPort[port].portInterruptMask)},  /**/                            \
    /*myFatherInfo*/{                                     \
        /*myBitIndex*/1 ,                                 \
        /*interruptPtr*/&gigaInterruptSummary_##port      \
    },                                                    \
    /*isTriggeredByWrite*/ 1                              \
}

static SKERNEL_INTERRUPT_REG_INFO_STC giga_mac_ports_interrupts[] =
{
    /* the Giga ports 0..15 */
     GIGA_MAC_INTERRUPT_CAUSE_MAC(0 )
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(1 )
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(2 )
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(3 )
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(4 )
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(5 )
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(6 )
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(7 )
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(8 )
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(9 )
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(10)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(11)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(12)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(13)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(14)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(15)
};

static SKERNEL_INTERRUPT_REG_INFO_STC  TXQ_queue_InterruptSummary =
{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.queue.global.interrupt.transmitQueueInterruptSummaryCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.queue.global.interrupt.transmitQueueInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/functional_units_txq_queue_int_sum ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
};

static SKERNEL_INTERRUPT_REG_INFO_STC txq_queue_global_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.queue.global.interrupt.generalInterrupt1Cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.queue.global.interrupt.generalInterrupt1Mask)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&TXQ_queue_InterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.queue.global.interrupt.portDescFullInterruptCause0)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.queue.global.interrupt.portDescFullInterruptMask0)},
        /*myFatherInfo*/{
            /*myBitIndex*/2 ,
            /*interruptPtr*/&TXQ_queue_InterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
    ,{
            /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.qcn.QCNInterruptCause)},
            /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.qcn.QCNInterruptMask)},
            /*myFatherInfo*/{
                /*myBitIndex*/functional_units_txq_qcn_int_sum ,
                /*interruptPtr*/&FuncUnitsInterruptsSummary
                },
            /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC cnc_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.CNCInterruptSummaryCauseReg)},/*SMEM_CHT3_CNC_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.CNCInterruptSummaryMaskReg)},
    /*myFatherInfo*/{
        /*myBitIndex*/functional_units_cnc_interrupt_cause_reg_sum ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC  txQDQInterruptSummary[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[0].global.interrupt.txQDQInterruptSummaryCause)},/*SMEM_CHT_TX_QUEUE_INTERRUPT_CAUSE_REG , SMEM_LION_TXQ_DQ_INTR_SUM_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[0].global.interrupt.txQDQInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/functional_units_txq_dq_int_sum ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[1].global.interrupt.txQDQInterruptSummaryCause)},/*SMEM_CHT_TX_QUEUE_INTERRUPT_CAUSE_REG , SMEM_LION_TXQ_DQ_INTR_SUM_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[1].global.interrupt.txQDQInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/functional_units_txq_dq1_int_sum ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
    }
};

/* node for Flush Done <%p> Interrupt Cause */
#define  FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(dq, index)  \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[dq].global.interrupt.flushDoneInterruptCause[index])}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[dq].global.interrupt.flushDoneInterruptMask[index])},  \
    /*myFatherInfo*/{                                     \
        /*myBitIndex*/((index)+1) ,                       \
        /*interruptPtr*/&txQDQInterruptSummary[dq]        \
        },                                                \
    /*isTriggeredByWrite*/ 1                              \
}

static SKERNEL_INTERRUPT_REG_INFO_STC flush_done_ports_interrupts[] =
{
    /* flush done - support 10 ports per DQ */
     FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(0, 0) /* DQ[0] */
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(1, 0) /* DQ[1] */
};

/* node for ingress STC summary */
#define  EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(dq, index)  \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[dq].global.interrupt.egrSTCInterruptCause[index])}, /*SMEM_CHT_STC_INT_CAUSE_REG , SMEM_LION_TXQ_EGR_STC_INTR_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[dq].global.interrupt.egrSTCInterruptMask[index])},  /*SMEM_CHT_STC_INT_MASK_REG , SMEM_LION_TXQ_EGR_STC_INTR_MASK_REG*/  \
    /*myFatherInfo*/{                                     \
        /*myBitIndex*/((index)+6) ,                       \
        /*interruptPtr*/&txQDQInterruptSummary[dq]        \
        },                                                \
    /*isTriggeredByWrite*/ 1                              \
}

static SKERNEL_INTERRUPT_REG_INFO_STC egr_stc_ports_interrupts[] =
{
    /* egress stc - support 10 ports per DQ */
      EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(0, 0) /* DQ[0] */
     ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(1, 0) /* DQ[1] */
};


static SKERNEL_INTERRUPT_REG_INFO_STC PCP_Interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PCP.general.PCPInterruptsCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PCP.general.PCPInterruptsMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/functional_units_pcp_interrupt_cause_reg_sum ,
            /*interruptPtr*/&FuncUnitsInterruptsSummary
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC PHA_summary_Interrupts =
{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PHA.pha_regs.PHAInterruptSumCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PHA.pha_regs.PHAInterruptSumMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/functional_units_sip_pha_interrupt_cause_sum_reg ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
};

static SKERNEL_INTERRUPT_REG_INFO_STC PPA_summary_Interrupts =
{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PHA.PPA.ppa_regs.PPAInterruptSumCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PHA.PPA.ppa_regs.PPAInterruptSumMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/ 3 ,
        /*interruptPtr*/&PHA_summary_Interrupts
        },
    /*isTriggeredByWrite*/ 1
};

#define PPG_INTERRUPT_SUM_CAUSE_NODE(ppgId)   \
    {                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PHA.PPG[ppgId].ppg_regs.PPGInterruptSumCause)}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PHA.PPG[ppgId].ppg_regs.PPGInterruptSumMask)},  \
    /*myFatherInfo*/{                                                                                 \
        /*myBitIndex*/ (ppgId)+2 ,                                                                    \
        /*interruptPtr*/&PPA_summary_Interrupts                                                       \
        },                                                                                            \
    /*isTriggeredByWrite*/ 1                                                                          \
    }

static SKERNEL_INTERRUPT_REG_INFO_STC PPG_summary_Interrupts[4] =
{
     PPG_INTERRUPT_SUM_CAUSE_NODE(0)
    ,PPG_INTERRUPT_SUM_CAUSE_NODE(1)
    ,PPG_INTERRUPT_SUM_CAUSE_NODE(2)
    ,PPG_INTERRUPT_SUM_CAUSE_NODE(3)
};

#define PPN_IN_PPG_INTERRUPT_CAUSE_NODE(ppgId,ppnId)   \
    {                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PHA.PPN[ppnId][ppgId].ppn_regs.PPNInternalErrorCause)}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PHA.PPN[ppnId][ppgId].ppn_regs.PPNInternalErrorMask)},  \
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


static SKERNEL_INTERRUPT_REG_INFO_STC PHA_Interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PHA.pha_regs.PHAInternalErrorCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PHA.pha_regs.PHAInternalErrorMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&PHA_summary_Interrupts
            },
        /*isTriggeredByWrite*/ 1
    },
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PHA.pha_regs.PHASERErrorCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PHA.pha_regs.PHASERErrorMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/2 ,
            /*interruptPtr*/&PHA_summary_Interrupts
            },
        /*isTriggeredByWrite*/ 1
    },
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PHA.pha_regs.PHASERErrorCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PHA.pha_regs.PHASERErrorMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/2 ,
            /*interruptPtr*/&PHA_summary_Interrupts
            },
        /*isTriggeredByWrite*/ 1
    },

    /*********************/

    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PHA.PPA.ppa_regs.PPAInternalErrorCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(PHA.PPA.ppa_regs.PPAInternalErrorMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&PPA_summary_Interrupts
            },
        /*isTriggeredByWrite*/ 1
    },

    ALL_PPN_IN_PPG_CAUSE_NODES(0),
    ALL_PPN_IN_PPG_CAUSE_NODES(1),
    ALL_PPN_IN_PPG_CAUSE_NODES(2),
    ALL_PPN_IN_PPG_CAUSE_NODES(3)

};

static SKERNEL_INTERRUPT_REG_INFO_STC MCFC_Interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(MCFC.interrupts.MCFCInterruptsCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_PIPE_MAC(MCFC.interrupts.MCFCInterruptsMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/functional_units_mcfc_interrupt_cause_reg_sum ,
            /*interruptPtr*/&FuncUnitsInterruptsSummary
            },
        /*isTriggeredByWrite*/ 1
    }
};


static SKERNEL_INTERRUPT_REG_INFO_STC skernel_interrupt_reg_info__last_line__cns[] =
{{{SKERNEL_REGISTERS_DB_TYPE__LAST___E}}};

/* PIPE interrupt tree database */
static SKERNEL_INTERRUPT_REG_INFO_STC pipeInterruptsTreeDb[MAX_INTERRUPT_NODES];

/**
* @internal smemPipeInterruptTreeInit function
* @endinternal
*
* @brief   Init the interrupts tree for the PIPE device
*
* @param[in] devObjPtr                - pointer to device object.
*/
GT_VOID smemPipeInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 index = 0;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr;

    devObjPtr->myInterruptsDbPtr = pipeInterruptsTreeDb;

    currDbPtr = devObjPtr->myInterruptsDbPtr;

    smemBobcat2GlobalInterruptTreeInit(devObjPtr, &index, &currDbPtr);

    /* common units of the PIPE device (that also used in other DXCH devices) */
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, bm_general_cause_interrupts       );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, mppm_ecc_interrupts               );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, txq_queue_global_interrupts       );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, cnc_interrupts                    );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, rxdma_interrupts                  );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, txdma_interrupts                  );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, txfifo_interrupts                 );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, xlg_mac_ports_interrupts          );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, flush_done_ports_interrupts       );/* txq-dq */
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, egr_stc_ports_interrupts          );/* txq-dq */
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, cg_100g_mac_ports_interrupts      );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, giga_mac_ports_interrupts         );

    /* special units of the PIPE device (that not exists in DXCH devices) */
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, PCP_Interrupts         );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, PHA_Interrupts         );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, MCFC_Interrupts        );

    /* End of interrupts - must be last */
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, skernel_interrupt_reg_info__last_line__cns);
}


