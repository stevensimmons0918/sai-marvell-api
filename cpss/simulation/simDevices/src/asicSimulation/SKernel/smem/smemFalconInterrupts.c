/******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* smemFalconInterrupts.c
*
* DESCRIPTION:
*       Falcon interrupt tree related implementation
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <asicSimulation/SKernel/smem/smemFalcon.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>

#define PIPE_MEM_SIZE         0x08000000
#define PIPE_0_START_ADDR     0x08000000/*start of pipe 0*/
#define PIPE_0_END_ADDR       (PIPE_0_START_ADDR + PIPE_MEM_SIZE - 4)/* end of pipe 0 */
#define PIPE_1_START_ADDR     0x10000000/*start of pipe 1*/
#define PIPE_1_END_ADDR       (PIPE_1_START_ADDR + PIPE_MEM_SIZE - 4)/* end of pipe 0 */

#define PIPE_1_FROM_PIPE_0_OFFSET   (PIPE_1_START_ADDR - PIPE_0_START_ADDR)

/* offset between tile 0 to tile 1 to tile 2 to tile 3 */
#define TILE_OFFSET          0x20000000

#define FALCON_PIPE_OFFSET_MAC(pipe)               (PIPE_1_FROM_PIPE_0_OFFSET * (pipe))
#define FALCON_TILE_OFFSET_MAC(tileId)             (TILE_OFFSET * (tileId))

/* Falcon interrupt tree database */
static SKERNEL_INTERRUPT_REG_INFO_STC falconInterruptsTreeDb[MAX_INTERRUPT_NODES];
static SKERNEL_INTERRUPT_REG_INFO_STC falconInterruptsTreeDb_perTile[MAX_INTERRUPT_NODES];

static SKERNEL_INTERRUPT_REG_INFO_STC skernel_interrupt_reg_info__last_line__cns[] =
{{{SKERNEL_REGISTERS_DB_TYPE__LAST___E}}};


#define cnm_grp_0_0_intr  (falcon_FuncUnitsInterruptsSummary  [0])
#define cnm_grp_0_1_intr  (falcon_dataPathInterruptSummary    [0])
#define cnm_grp_0_2_intr  (falcon_portsInterruptsSummary      [0])
#define cnm_grp_0_3_intr  (falcon_dfxInterruptsSummary        [0])
#define cnm_grp_0_4_intr  (falcon_dfx1InterruptsSummary       [0])
#define cnm_grp_0_5_intr  (falcon_FuncUnitsInterruptsSummary1 [0])
#define cnm_grp_0_6_intr  (falcon_ports1InterruptsSummary     [0])
#define cnm_grp_0_7_intr  (falcon_ports2InterruptsSummary     [0])

#define cnm_grp_0_0_intr_x(x)  (falcon_FuncUnitsInterruptsSummary  [x])


enum{  /*cnm_grp_0_0_intr  - functionalInterruptsSummaryCause/FuncUnitsInterruptsSummary */
    cnm_grp_0_0_intr_Raven_0_Int = 1,
    cnm_grp_0_0_intr_Raven_1_Int = 2,
    cnm_grp_0_0_intr_Raven_2_Int = 3,
    cnm_grp_0_0_intr_Raven_3_Int = 4,
    cnm_grp_0_0_intr_Raven_4_Int = 5,     /* point to tile 1 ! */
    cnm_grp_0_0_intr_Raven_5_Int = 6,     /* point to tile 1 ! */
    cnm_grp_0_0_intr_Raven_6_Int = 7,     /* point to tile 1 ! */
    cnm_grp_0_0_intr_Raven_7_Int = 8,     /* point to tile 1 ! */

    cnm_grp_0_0_MG8_int_out0_sum = 17,    /* connecting MG0 of tile 0 to MG0 of tile2 (MG8)*/

    cnm_grp_0_0_intr_Pb_Int       = 22,
    cnm_grp_0_0_intr_Em_Int       = 23,
    cnm_grp_0_0_intr_Mt_Int       = 24,
    cnm_grp_0_0_intr_Packer0_Int  = 25,
    cnm_grp_0_0_intr_Packer1_Int  = 26,
    cnm_grp_0_0_intr_Pdx_Int      = 27,
    cnm_grp_0_0_intr_Pfcc_Int     = 28,
    cnm_grp_0_0_intr_Psi_Int      = 29,
    cnm_grp_0_0_intr_Shm_Int      = 30,
    cnm_grp_0_0_intr_Tcam_Int     = 31
};



enum{ /*cnm_grp_0_1_intr_ - SERInterruptsSummary/dataPathInterruptSummary */
    cnm_grp_0_1_intr_Pipe0_Bma_Int      = 1,
    cnm_grp_0_1_intr_Pipe0_Eft_Int      = 2,
    cnm_grp_0_1_intr_Pipe0_Eoam_Int     = 3,
    cnm_grp_0_1_intr_Pipe0_Epcl_Int     = 4,
    cnm_grp_0_1_intr_Pipe0_Eplr_Int     = 5,
    cnm_grp_0_1_intr_Pipe0_Eq_Int       = 6,
    cnm_grp_0_1_intr_Pipe0_Erep_Int     = 7,
    cnm_grp_0_1_intr_Pipe0_Ha_Int       = 8,
    cnm_grp_0_1_intr_Pipe0_Ia_Int       = 10,
    cnm_grp_0_1_intr_Pipe0_Ioam_Int     = 11,
    cnm_grp_0_1_intr_Pipe0_Iplr0_Int    = 12,
    cnm_grp_0_1_intr_Pipe0_Iplr1_Int    = 13,
    cnm_grp_0_1_intr_Pipe0_Ipvx_Int     = 14,
    cnm_grp_0_1_intr_Pipe0_Lpm_Int      = 15,
    cnm_grp_0_1_intr_Pipe0_Mll_Int      = 16,
    cnm_grp_0_1_intr_Pipe0_Pcl_Int      = 17,
    cnm_grp_0_1_intr_Pipe0_Preq_Int     = 18,
    cnm_grp_0_1_intr_Pipe0_Egf_Qag_Int  = 19,
    cnm_grp_0_1_intr_Pipe0_Egf_Sht_Int  = 20,
    cnm_grp_0_1_intr_Pipe0_Pha_Int      = 21,
    cnm_grp_0_1_intr_Pipe0_Tti_Int      = 22,
    cnm_grp_0_1_intr_Pipe0_Txdma0_Int   = 23,
    cnm_grp_0_1_intr_Pipe0_Txfifo0_Int  = 24,
    cnm_grp_0_1_intr_Pipe0_Txdma1_Int   = 25,
    cnm_grp_0_1_intr_Pipe0_Txfifo1_Int  = 26,
    cnm_grp_0_1_intr_Pipe0_Txdma2_Int   = 27,
    cnm_grp_0_1_intr_Pipe0_Txfifo2_Int  = 28,
    cnm_grp_0_1_intr_Pipe0_Txdma3_Int   = 29,
    cnm_grp_0_1_intr_Pipe0_Txfifo3_Int  = 30
};

enum{ /*cnm_grp_0_2_intr_ - stackPortsInterruptCause/portsInterruptsSummary  */
    cnm_grp_0_2_intr_Pipe1_Bma_Int        = 1,
    cnm_grp_0_2_intr_Pipe1_Eft_Int        = 2,
    cnm_grp_0_2_intr_Pipe1_Eoam_Int       = 3,
    cnm_grp_0_2_intr_Pipe1_Epcl_Int       = 4,
    cnm_grp_0_2_intr_Pipe1_Eplr_Int       = 5,
    cnm_grp_0_2_intr_Pipe1_Eq_Int         = 6,
    cnm_grp_0_2_intr_Pipe1_Erep_Int       = 7,
    cnm_grp_0_2_intr_Pipe1_Ha_Int         = 8,
    cnm_grp_0_2_intr_Pipe1_Ia_Int         = 10,
    cnm_grp_0_2_intr_Pipe1_Ioam_Int       = 11,
    cnm_grp_0_2_intr_Pipe1_Iplr0_Int      = 12,
    cnm_grp_0_2_intr_Pipe1_Iplr1_Int      = 13,
    cnm_grp_0_2_intr_Pipe1_Ipvx_Int       = 14,
    cnm_grp_0_2_intr_Pipe1_Lpm_Int        = 15,
    cnm_grp_0_2_intr_Pipe1_Mll_Int        = 16,
    cnm_grp_0_2_intr_Pipe1_Pcl_Int        = 17,
    cnm_grp_0_2_intr_Pipe1_Preq_Int       = 18,
    cnm_grp_0_2_intr_Pipe1_Egf_Qag_Int    = 19,
    cnm_grp_0_2_intr_Pipe1_Egf_Sht_Int    = 20,
    cnm_grp_0_2_intr_Pipe1_Pha_Int        = 21,
    cnm_grp_0_2_intr_Pipe1_Tti_Int        = 22,
    cnm_grp_0_2_intr_Pipe1_Txdma0_Int     = 23,
    cnm_grp_0_2_intr_Pipe1_Txfifo0_Int    = 24,
    cnm_grp_0_2_intr_Pipe1_Txdma1_Int     = 25,
    cnm_grp_0_2_intr_Pipe1_Txfifo1_Int    = 26,
    cnm_grp_0_2_intr_Pipe1_Txdma2_Int     = 27,
    cnm_grp_0_2_intr_Pipe1_Txfifo2_Int    = 28,
    cnm_grp_0_2_intr_Pipe1_Txdma3_Int     = 29,
    cnm_grp_0_2_intr_Pipe1_Txfifo3_Int    = 30
};

enum{ /*cnm_grp_0_3_intr_ - dfxInterruptsSummaryCause/dfxInterruptsSummary */
    cnm_grp_0_3_intr_tile1_Pipe0_Bma_Int       = 1,       /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Eft_Int       = 2,       /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Eoam_Int      = 3,       /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Epcl_Int      = 4,       /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Eplr_Int      = 5,       /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Eq_Int        = 6,       /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Erep_Int      = 7,       /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Ha_Int        = 8,       /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Ia_Int        = 10,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Ioam_Int      = 11,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Iplr0_Int     = 12,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Iplr1_Int     = 13,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Ipvx_Int      = 14,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Lpm_Int       = 15,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Mll_Int       = 16,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Pcl_Int       = 17,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Preq_Int      = 18,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Egf_Qag_Int   = 19,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Egf_Sht_Int   = 20,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Pha_Int       = 21,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Tti_Int       = 22,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Txdma0_Int    = 23,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Txfifo0_Int   = 24,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Txdma1_Int    = 25,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Txfifo1_Int   = 26,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Txdma2_Int    = 27,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Txfifo2_Int   = 28,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Txdma3_Int    = 29,      /* point to tile 1 ! */
    cnm_grp_0_3_intr_tile1_Pipe0_Txfifo3_Int   = 30       /* point to tile 1 ! */
};

enum{ /*cnm_grp_0_4_intr_ - dfx1InterruptsSummaryCause/dfx1InterruptsSummary */
    cnm_grp_0_4_intr_tile1_Pipe1_Bma_Int       = 1,       /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Eft_Int       = 2,       /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Eoam_Int      = 3,       /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Epcl_Int      = 4,       /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Eplr_Int      = 5,       /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Eq_Int        = 6,       /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Erep_Int      = 7,       /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Ha_Int        = 8,       /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Ia_Int        = 10,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Ioam_Int      = 11,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Iplr0_Int     = 12,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Iplr1_Int     = 13,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Ipvx_Int      = 14,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Lpm_Int       = 15,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Mll_Int       = 16,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Pcl_Int       = 17,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Preq_Int      = 18,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Egf_Qag_Int   = 19,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Egf_Sht_Int   = 20,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Pha_Int       = 21,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Tti_Int       = 22,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Txdma0_Int    = 23,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Txfifo0_Int   = 24,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Txdma1_Int    = 25,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Txfifo1_Int   = 26,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Txdma2_Int    = 27,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Txfifo2_Int   = 28,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Txdma3_Int    = 29,      /* point to tile 1 ! */
    cnm_grp_0_4_intr_tile1_Pipe1_Txfifo3_Int   = 30       /* point to tile 1 ! */
};

enum{  /*cnm_grp_0_5_intr_  - functionalInterruptsSummary1Cause/FuncUnitsInterruptsSummary1 */
    cnm_grp_0_5_intr_Rxdma0_Int  = 0,
    cnm_grp_0_5_intr_Rxdma1_Int  = 1,
    cnm_grp_0_5_intr_Rxdma2_Int  = 2,
    cnm_grp_0_5_intr_Rxdma3_Int  = 3,
    cnm_grp_0_5_intr_Rxdma4_Int  = 4,
    cnm_grp_0_5_intr_Rxdma5_Int  = 5,
    cnm_grp_0_5_intr_Rxdma6_Int  = 6,
    cnm_grp_0_5_intr_Rxdma7_Int  = 7,

    cnm_grp_0_5_intr_tile1_Rxdma0_Int  = 8,    /* point to tile 1 ! */
    cnm_grp_0_5_intr_tile1_Rxdma1_Int  = 9,    /* point to tile 1 ! */
    cnm_grp_0_5_intr_tile1_Rxdma2_Int  = 10,   /* point to tile 1 ! */
    cnm_grp_0_5_intr_tile1_Rxdma3_Int  = 11,   /* point to tile 1 ! */
    cnm_grp_0_5_intr_tile1_Rxdma4_Int  = 12,   /* point to tile 1 ! */
    cnm_grp_0_5_intr_tile1_Rxdma5_Int  = 13,   /* point to tile 1 ! */
    cnm_grp_0_5_intr_tile1_Rxdma6_Int  = 14,   /* point to tile 1 ! */
    cnm_grp_0_5_intr_tile1_Rxdma7_Int  = 15,   /* point to tile 1 ! */

    cnm_grp_0_5_intr_tile1_Pb_Int       = 22,
    cnm_grp_0_5_intr_tile1_Em_Int       = 23,
    cnm_grp_0_5_intr_tile1_Mt_Int       = 24,
    cnm_grp_0_5_intr_tile1_Packer0_Int  = 25,
    cnm_grp_0_5_intr_tile1_Packer1_Int  = 26,
    cnm_grp_0_5_intr_tile1_Pdx_Int      = 27,
    cnm_grp_0_5_intr_tile1_Pfcc_Int     = 28,
    cnm_grp_0_5_intr_tile1_Psi_Int      = 29,
    cnm_grp_0_5_intr_tile1_Shm_Int      = 30,
    cnm_grp_0_5_intr_tile1_Tcam_Int     = 31
};


enum{  /*cnm_grp_0_6_intr_ - ports1InterruptsSummaryCause/ports1InterruptsSummary */
    cnm_grp_0_6_intr_Pipe0_Cnc0_Int  = 23,
    cnm_grp_0_6_intr_Pipe0_Cnc1_Int  = 24,
    cnm_grp_0_6_intr_Pipe1_Cnc0_Int  = 25,
    cnm_grp_0_6_intr_Pipe1_Cnc1_Int  = 26,

    cnm_grp_0_6_intr_tile1_Pipe0_Cnc0_Int  = 27,   /* point to tile 1 ! */
    cnm_grp_0_6_intr_tile1_Pipe0_Cnc1_Int  = 28,   /* point to tile 1 ! */
    cnm_grp_0_6_intr_tile1_Pipe1_Cnc0_Int  = 29,   /* point to tile 1 ! */
    cnm_grp_0_6_intr_tile1_Pipe1_Cnc1_Int  = 30    /* point to tile 1 ! */
};

enum{ /*cnm_grp_0_7_intr - ports2InterruptsSummaryCause/ports2InterruptsSummary */

    cnm_grp_0_7_intr_falcon_MG1_int_out0_sum = 1,    /* MG1 of the tile */
    cnm_grp_0_7_intr_falcon_MG2_int_out0_sum = 4,    /* MG2 of the tile */
    cnm_grp_0_7_intr_falcon_MG3_int_out0_sum = 7,    /* MG3 of the tile */

    cnm_grp_0_7_intr_falcon_MG5_int_out0_sum = 10,   /* MG5 of the tile */
    cnm_grp_0_7_intr_falcon_MG6_int_out0_sum = 13,   /* MG6 of the tile */
    cnm_grp_0_7_intr_falcon_MG7_int_out0_sum = 16,   /* MG7 of the tile  */

    cnm_grp_0_7_intr_Pipe0_Ermrk_Int         = 19,
    cnm_grp_0_7_intr_Pipe0_L2I_Int           = 20,
    cnm_grp_0_7_intr_Pipe1_Ermrk_Int         = 21,
    cnm_grp_0_7_intr_Pipe1_L2I_Int           = 22,
    cnm_grp_0_7_intr_Tile1_Pipe0_Ermrk_Int   = 23,
    cnm_grp_0_7_intr_Tile1_Pipe0_L2I_Int     = 24,
    cnm_grp_0_7_intr_Tile1_Pipe1_Ermrk_Int   = 25,
    cnm_grp_0_7_intr_Tile1_Pipe1_L2I_Int     = 26,

    cnm_grp_0_7_intr_falcon_MG4_int_out0_sum = 29,   /* MG0 of next tile (for tiles 1,3) */
};

enum{
     MG_0_0_INDEX
    ,MG_0_1_INDEX
    ,MG_0_2_INDEX
    ,MG_0_3_INDEX

    ,MG_1_0_INDEX
    ,MG_1_1_INDEX
    ,MG_1_2_INDEX
    ,MG_1_3_INDEX

    ,MG_2_0_INDEX
    ,MG_2_1_INDEX
    ,MG_2_2_INDEX
    ,MG_2_3_INDEX

    ,MG_3_0_INDEX
    ,MG_3_1_INDEX
    ,MG_3_2_INDEX
    ,MG_3_3_INDEX

    ,NUM_OF_MGS   /*16*/
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
     MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_0_0_INDEX,father_myBitIndex,father_interrupt[MG_0_0_INDEX]) \
    ,UNSED_MG/* MG 0_1 not used */                                                                   \
    ,UNSED_MG/* MG 0_2 not used */                                                                   \
    ,UNSED_MG/* MG 0_3 not used */                                                                   \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_1_0_INDEX,father_myBitIndex,father_interrupt[MG_1_0_INDEX]) \
    ,UNSED_MG/* MG 1_1 not used */                                                                   \
    ,UNSED_MG/* MG 1_2 not used */                                                                   \
    ,UNSED_MG/* MG 1_3 not used */                                                                   \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_2_0_INDEX,father_myBitIndex,father_interrupt[MG_2_0_INDEX]) \
    ,UNSED_MG/* MG 2_1 not used */                                                                   \
    ,UNSED_MG/* MG 2_2 not used */                                                                   \
    ,UNSED_MG/* MG 2_3 not used */                                                                   \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_3_0_INDEX,father_myBitIndex,father_interrupt[MG_3_0_INDEX]) \
    ,UNSED_MG/* MG 3_1 not used */                                                                   \
    ,UNSED_MG/* MG 3_2 not used */                                                                   \
    ,UNSED_MG/* MG 3_3 not used */


/* duplicate the node of MG to MG_x_0 and MG_x_1 (primary + secondary MG units) */
#define DUP_PRIMARY_AND_SECONDARY_MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt) \
     MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_0_0_INDEX,father_myBitIndex,father_interrupt[MG_0_0_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_0_1_INDEX,father_myBitIndex,father_interrupt[MG_0_1_INDEX]) \
    ,UNSED_MG/* MG 0_2 not used */                                                                   \
    ,UNSED_MG/* MG 0_3 not used */                                                                   \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_1_0_INDEX,father_myBitIndex,father_interrupt[MG_1_0_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_1_1_INDEX,father_myBitIndex,father_interrupt[MG_1_1_INDEX]) \
    ,UNSED_MG/* MG 1_2 not used */                                                        \
    ,UNSED_MG/* MG 1_3 not used */                                                        \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_2_0_INDEX,father_myBitIndex,father_interrupt[MG_2_0_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_2_1_INDEX,father_myBitIndex,father_interrupt[MG_2_1_INDEX]) \
    ,UNSED_MG/* MG 2_2 not used */                                                                   \
    ,UNSED_MG/* MG 2_3 not used */                                                                   \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_3_0_INDEX,father_myBitIndex,father_interrupt[MG_3_0_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_3_1_INDEX,father_myBitIndex,father_interrupt[MG_3_1_INDEX]) \
    ,UNSED_MG/* MG 3_2 not used */                                                                   \
    ,UNSED_MG/* MG 3_3 not used */

/* duplicate the node of MG to ALL MG_x_x */
#define DUP_ALL_MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt) \
     MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_0_0_INDEX,father_myBitIndex,father_interrupt[MG_0_0_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_0_1_INDEX,father_myBitIndex,father_interrupt[MG_0_1_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_0_2_INDEX,father_myBitIndex,father_interrupt[MG_0_2_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_0_3_INDEX,father_myBitIndex,father_interrupt[MG_0_3_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_1_0_INDEX,father_myBitIndex,father_interrupt[MG_1_0_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_1_1_INDEX,father_myBitIndex,father_interrupt[MG_1_1_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_1_2_INDEX,father_myBitIndex,father_interrupt[MG_1_2_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_1_3_INDEX,father_myBitIndex,father_interrupt[MG_1_3_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_2_0_INDEX,father_myBitIndex,father_interrupt[MG_2_0_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_2_1_INDEX,father_myBitIndex,father_interrupt[MG_2_1_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_2_2_INDEX,father_myBitIndex,father_interrupt[MG_2_2_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_2_3_INDEX,father_myBitIndex,father_interrupt[MG_2_3_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_3_0_INDEX,father_myBitIndex,father_interrupt[MG_3_0_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_3_1_INDEX,father_myBitIndex,father_interrupt[MG_3_1_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_3_2_INDEX,father_myBitIndex,father_interrupt[MG_3_2_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_3_3_INDEX,father_myBitIndex,father_interrupt[MG_3_3_INDEX])




static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_ports2FatherInterruptsSummary[NUM_OF_MGS];

static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_ports2InterruptsSummary    [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(ports2InterruptsSummaryCause     ,ports2InterruptsSummaryMask     ,ports_2_int_sum  ,falcon_ports2FatherInterruptsSummary)  };

static SKERNEL_INTERRUPT_REG_INFO_STC dummy;

static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_GlobalInterruptsSummary    [NUM_OF_MGS] =
{
    { /* copied from GlobalInterruptsSummary */
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.globalInterruptCause),0,MG_0_0_INDEX | MG_UNIT_USED_CNS},/*SMEM_CHT_GLB_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.globalInterruptMask) ,0,MG_0_0_INDEX | MG_UNIT_USED_CNS},
        /*myFatherInfo*/{
            /*myBitIndex*/0 ,
            /*interruptPtr*/NULL /* no father for me , thank you */
            },
        /*isTriggeredByWrite*/ 1
    }
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_0_1_INDEX,cnm_grp_0_7_intr_falcon_MG1_int_out0_sum,falcon_ports2InterruptsSummary[MG_0_0_INDEX])/* into MG_0_0 (tile 0)*/
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_0_2_INDEX,cnm_grp_0_7_intr_falcon_MG2_int_out0_sum,falcon_ports2InterruptsSummary[MG_0_0_INDEX])
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_0_3_INDEX,cnm_grp_0_7_intr_falcon_MG3_int_out0_sum,falcon_ports2InterruptsSummary[MG_0_0_INDEX])
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_1_0_INDEX,cnm_grp_0_7_intr_falcon_MG4_int_out0_sum,falcon_ports2InterruptsSummary[MG_0_0_INDEX])/* into MG_0_0 (tile 0)*/
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_1_1_INDEX,cnm_grp_0_7_intr_falcon_MG5_int_out0_sum,falcon_ports2InterruptsSummary[MG_1_0_INDEX])/* into MG_1_0 (tile 1)*/
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_1_2_INDEX,cnm_grp_0_7_intr_falcon_MG6_int_out0_sum,falcon_ports2InterruptsSummary[MG_1_0_INDEX])
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_1_3_INDEX,cnm_grp_0_7_intr_falcon_MG7_int_out0_sum,falcon_ports2InterruptsSummary[MG_1_0_INDEX])
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_2_0_INDEX,cnm_grp_0_0_MG8_int_out0_sum            ,dummy/*set in runtime*/)/* into MG_0_0 (tile 0)*/
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_2_1_INDEX,cnm_grp_0_7_intr_falcon_MG1_int_out0_sum,dummy/*set in runtime*/)/* into tree1 in MG_2_0 (tile 2) */
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_2_2_INDEX,cnm_grp_0_7_intr_falcon_MG2_int_out0_sum,dummy/*set in runtime*/)/* into tree1 in MG_2_0 (tile 2) */
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_2_3_INDEX,cnm_grp_0_7_intr_falcon_MG3_int_out0_sum,dummy/*set in runtime*/)/* into tree1 in MG_2_0 (tile 2) */
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_3_0_INDEX,cnm_grp_0_7_intr_falcon_MG4_int_out0_sum,dummy/*set in runtime*/)/* into tree1 in MG_2_0 (tile 2) */
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_3_1_INDEX,cnm_grp_0_7_intr_falcon_MG5_int_out0_sum,falcon_ports2InterruptsSummary[MG_3_0_INDEX])/* into MG_3_0 (tile 3)*/
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_3_2_INDEX,cnm_grp_0_7_intr_falcon_MG6_int_out0_sum,falcon_ports2InterruptsSummary[MG_3_0_INDEX])
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_3_3_INDEX,cnm_grp_0_7_intr_falcon_MG7_int_out0_sum,falcon_ports2InterruptsSummary[MG_3_0_INDEX])
};

static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_FuncUnitsInterruptsSummary [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(functionalInterruptsSummaryCause ,functionalInterruptsSummaryMask ,FuncUnitsIntsSum ,falcon_GlobalInterruptsSummary) };
static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_FuncUnitsInterruptsSummary1[NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(functionalInterruptsSummary1Cause,functionalInterruptsSummary1Mask,FuncUnits1IntsSum,falcon_GlobalInterruptsSummary) };
static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_dataPathInterruptSummary   [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(SERInterruptsSummary             ,SERInterruptsMask               ,DataPathIntSum   ,falcon_GlobalInterruptsSummary) };
static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_ports1InterruptsSummary    [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(ports1InterruptsSummaryCause     ,ports1InterruptsSummaryMask     ,ports_1_int_sum  ,falcon_GlobalInterruptsSummary) };
static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_portsInterruptsSummary     [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(stackPortsInterruptCause         ,stackPortsInterruptMask         ,PortsIntSum      ,falcon_GlobalInterruptsSummary) };

static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_dfxInterruptsSummary [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(dfxInterruptsSummaryCause       ,dfxInterruptsSummaryMask         ,dfxIntSum        ,falcon_GlobalInterruptsSummary) };
static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_dfx1InterruptsSummary [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(dfx1InterruptsSummaryCause      ,dfx1InterruptsSummaryMask        ,dfx1IntSum       ,falcon_GlobalInterruptsSummary) };


/* this node need support in primary and secondary MG units (to support CNC dump from the FUQ) */
static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_mg_global_misc_interrupts  [NUM_OF_MGS] =
{    DUP_ALL_MG_INTERTTUPT_REG(miscellaneousInterruptCause ,miscellaneousInterruptMask ,MgInternalIntSum ,falcon_GlobalInterruptsSummary) };


#define MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,mgIndex) \
     MG_SDMA_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,mgIndex,father_myBitIndex,father_interrupt[mgIndex])


/* duplicate the node of MG to all MG units */
#define DUP_SDMA_MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt) \
     MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_0_0_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_0_1_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_0_2_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_0_3_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_1_0_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_1_1_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_1_2_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_1_3_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_2_0_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_2_1_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_2_2_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_2_3_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_3_0_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_3_1_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_3_2_INDEX) \
    ,MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_3_3_INDEX)

/* this node need support in ALL MG units (to MG SDMA) */
static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_mg_global_sdma_receiveSDMA_interrupts  [NUM_OF_MGS] =
{    DUP_SDMA_MG_INTERTTUPT_REG(receiveSDMAInterruptCause ,receiveSDMAInterruptMask ,RxSDMASum ,falcon_GlobalInterruptsSummary) };

/* this node need support in ALL MG units (to MG SDMA) */
static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_mg_global_sdma_transmitSDMA_interrupts  [NUM_OF_MGS] =
{    DUP_SDMA_MG_INTERTTUPT_REG(transmitSDMAInterruptCause ,transmitSDMAInterruptMask ,TxSDMASum ,falcon_GlobalInterruptsSummary) };

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

#define MG_SDMA_INTERTTUPT_TREE1_REG(mg_causeRegName,mg_maskRegName, father_myBitIndex, father_interrupt) \
{                                                                                              \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.SDMARegs.mg_causeRegName),0, MG_2_0_INDEX | MG_UNIT_USED_CNS}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.SDMARegs.mg_maskRegName ),0, MG_2_0_INDEX | MG_UNIT_USED_CNS}, \
        /*myFatherInfo*/{                                                                      \
            /*myBitIndex*/  father_myBitIndex,                                                 \
            /*interruptPtr*/&father_interrupt                                                  \
            },                                                                                 \
        /*isTriggeredByWrite*/ 1                                                               \
}


static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_GlobalInterruptsSummary_MG8_tree1[] =
{
    { /* copied from GlobalInterruptsSummary */
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt_tree1.globalInterruptCause), 0, MG_2_0_INDEX | MG_UNIT_USED_CNS},/*SMEM_CHT_GLB_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt_tree1.globalInterruptMask) , 0, MG_2_0_INDEX | MG_UNIT_USED_CNS},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_0_MG8_int_out0_sum ,
            /*interruptPtr*/&falcon_FuncUnitsInterruptsSummary[MG_0_0_INDEX]
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_ports2InterruptsSummary_MG8_tree1[] =
{
     MG_INTERTTUPT_TREE1_REG(ports2InterruptsSummaryCause, ports2InterruptsSummaryMask, MG_2_0_INDEX, ports_2_int_sum, falcon_GlobalInterruptsSummary_MG8_tree1[0])
};

static SKERNEL_INTERRUPT_REG_INFO_STC falcon_FuncUnitsInterruptsSummary_MG8_tree1[] =
{
    MG_INTERTTUPT_TREE1_REG(functionalInterruptsSummaryCause, functionalInterruptsSummaryMask, MG_2_0_INDEX, FuncUnitsIntsSum, falcon_GlobalInterruptsSummary_MG8_tree1[0])
};

static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_FuncUnitsInterruptsSummary1_MG8_tree1[] =
{
    MG_INTERTTUPT_TREE1_REG(functionalInterruptsSummary1Cause, functionalInterruptsSummary1Mask, MG_2_0_INDEX, FuncUnits1IntsSum, falcon_GlobalInterruptsSummary_MG8_tree1[0])
};

static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_dataPathInterruptSummary_MG8_tree1[] =
{
    MG_INTERTTUPT_TREE1_REG(SERInterruptsSummary, SERInterruptsMask, MG_2_0_INDEX, DataPathIntSum, falcon_GlobalInterruptsSummary_MG8_tree1[0])
};

static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_ports1InterruptsSummary_MG8_tree1[] =
{
    MG_INTERTTUPT_TREE1_REG(ports1InterruptsSummaryCause, ports1InterruptsSummaryMask, MG_2_0_INDEX, ports_1_int_sum, falcon_GlobalInterruptsSummary_MG8_tree1[0])
};

static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_portsInterruptsSummary_MG8_tree1[] =
{
    MG_INTERTTUPT_TREE1_REG(stackPortsInterruptCause, stackPortsInterruptMask, MG_2_0_INDEX, PortsIntSum, falcon_GlobalInterruptsSummary_MG8_tree1[0])
};

static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_dfxInterruptsSummary_MG8_tree1[] =
{
    MG_INTERTTUPT_TREE1_REG(dfxInterruptsSummaryCause, dfxInterruptsSummaryMask, MG_2_0_INDEX, dfxIntSum, falcon_GlobalInterruptsSummary_MG8_tree1[0])
};

static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_dfx1InterruptsSummary_MG8_tree1[] =
{
    MG_INTERTTUPT_TREE1_REG(dfx1InterruptsSummaryCause, dfx1InterruptsSummaryMask, MG_2_0_INDEX, dfx1IntSum, falcon_GlobalInterruptsSummary_MG8_tree1[0])
};

static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_mg_global_misc_interrupts_MG8_tree1[] =
{
    MG_INTERTTUPT_TREE1_REG(miscellaneousInterruptCause, miscellaneousInterruptMask, MG_2_0_INDEX, MgInternalIntSum, falcon_GlobalInterruptsSummary_MG8_tree1[0])
};

static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_mg_global_sdma_receiveSDMA_interrupts_MG8_tree1[] =
{
    MG_SDMA_INTERTTUPT_TREE1_REG(receiveSDMAInterruptCause_tree1, receiveSDMAInterruptMask_tree1, RxSDMASum, falcon_GlobalInterruptsSummary_MG8_tree1[0])
};

static SKERNEL_INTERRUPT_REG_INFO_STC  falcon_mg_global_sdma_transmitSDMA_interrupts_MG8_tree1[] =
{
    MG_SDMA_INTERTTUPT_TREE1_REG(transmitSDMAInterruptCause_tree1, transmitSDMAInterruptMask_tree1, TxSDMASum, falcon_GlobalInterruptsSummary_MG8_tree1[0])
};

static SKERNEL_INTERRUPT_REG_INFO_STC cnc_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.CNCInterruptSummaryCauseReg)},/*SMEM_CHT3_CNC_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.CNCInterruptSummaryMaskReg)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_6_intr_Pipe0_Cnc0_Int ,
        /*interruptPtr*/&cnm_grp_0_6_intr
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[1].globalRegs.CNCInterruptSummaryCauseReg)},/*SMEM_CHT3_CNC_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[1].globalRegs.CNCInterruptSummaryMaskReg)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_6_intr_Pipe0_Cnc1_Int,
        /*interruptPtr*/&cnm_grp_0_6_intr
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
   /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.CNCInterruptSummaryCauseReg) , FALCON_PIPE_OFFSET_MAC(1)},/*SMEM_CHT3_CNC_INTR_CAUSE_REG*/
   /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.CNCInterruptSummaryMaskReg) , FALCON_PIPE_OFFSET_MAC(1)},
   /*myFatherInfo*/{
       /*myBitIndex*/cnm_grp_0_6_intr_Pipe1_Cnc0_Int,
       /*interruptPtr*/&cnm_grp_0_6_intr
       },
   /*isTriggeredByWrite*/ 1
    }
   ,{
   /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[1].globalRegs.CNCInterruptSummaryCauseReg) , FALCON_PIPE_OFFSET_MAC(1)},/*SMEM_CHT3_CNC_INTR_CAUSE_REG*/
   /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[1].globalRegs.CNCInterruptSummaryMaskReg) , FALCON_PIPE_OFFSET_MAC(1)},
   /*myFatherInfo*/{
       /*myBitIndex*/cnm_grp_0_6_intr_Pipe1_Cnc1_Int,
       /*interruptPtr*/&cnm_grp_0_6_intr
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
        /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Eft_Int,
        /*interruptPtr*/&cnm_grp_0_1_intr
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EGF_eft.global.EFTInterruptsCause) , FALCON_PIPE_OFFSET_MAC(1)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EGF_eft.global.EFTInterruptsMask) , FALCON_PIPE_OFFSET_MAC(1)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Eft_Int,
        /*interruptPtr*/&cnm_grp_0_2_intr
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
        /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Ioam_Int ,
        /*interruptPtr*/&cnm_grp_0_1_intr
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[1].OAMUnitInterruptCause)},/*SMEM_LION2_OAM_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[1].OAMUnitInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Eoam_Int,
        /*interruptPtr*/&cnm_grp_0_1_intr
        },
        /*isTriggeredByWrite*/ 1
    }
   ,{
   /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[0].OAMUnitInterruptCause) , FALCON_PIPE_OFFSET_MAC(1)},/*SMEM_LION2_OAM_INTR_CAUSE_REG*/
   /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[0].OAMUnitInterruptMask)  , FALCON_PIPE_OFFSET_MAC(1)},
   /*myFatherInfo*/{
       /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Ioam_Int,
       /*interruptPtr*/&cnm_grp_0_2_intr
       },
   /*isTriggeredByWrite*/ 1
    }
   ,{
   /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[1].OAMUnitInterruptCause) , FALCON_PIPE_OFFSET_MAC(1)},/*SMEM_LION2_OAM_INTR_CAUSE_REG*/
   /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[1].OAMUnitInterruptMask) , FALCON_PIPE_OFFSET_MAC(1)},
   /*myFatherInfo*/{
       /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Eoam_Int,
       /*interruptPtr*/&cnm_grp_0_2_intr
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
        /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Pcl_Int,
        /*interruptPtr*/&cnm_grp_0_1_intr
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCL.PCLUnitInterruptCause) , FALCON_PIPE_OFFSET_MAC(1)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCL.PCLUnitInterruptMask) , FALCON_PIPE_OFFSET_MAC(1)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Pcl_Int ,
        /*interruptPtr*/&cnm_grp_0_2_intr
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
        /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Tti_Int ,
        /*interruptPtr*/&cnm_grp_0_1_intr
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TTI.TTIEngine.TTIEngineInterruptCause) , FALCON_PIPE_OFFSET_MAC(1)},/*SMEM_LION3_TTI_INTERRUPT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TTI.TTIEngine.TTIEngineInterruptMask) , FALCON_PIPE_OFFSET_MAC(1)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Tti_Int ,
        /*interruptPtr*/&cnm_grp_0_2_intr
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
        /*myBitIndex*/cnm_grp_0_7_intr_Pipe0_L2I_Int ,
        /*interruptPtr*/&cnm_grp_0_7_intr,
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(L2I.bridgeEngineInterrupts.bridgeInterruptCause) , FALCON_PIPE_OFFSET_MAC(1)},/*SMEM_CHT_BRIDGE_INT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(L2I.bridgeEngineInterrupts.bridgeInterruptMask) , FALCON_PIPE_OFFSET_MAC(1)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_7_intr_Pipe1_L2I_Int ,
        /*interruptPtr*/&cnm_grp_0_7_intr,
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
        /*myBitIndex*/cnm_grp_0_0_intr_Mt_Int ,
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
        /*myBitIndex*/cnm_grp_0_0_intr_Tcam_Int ,
        /*interruptPtr*/&cnm_grp_0_0_intr /* tile0 */
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
        /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Ha_Int ,
        /*interruptPtr*/&cnm_grp_0_1_intr
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(HA.HAInterruptCause) , FALCON_PIPE_OFFSET_MAC(1)},/*SMEM_LION3_HA_INTERRUPT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(HA.HAInterruptMask) , FALCON_PIPE_OFFSET_MAC(1)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Ha_Int ,
        /*interruptPtr*/&cnm_grp_0_2_intr
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
        /*myBitIndex*/cnm_grp_0_7_intr_Pipe0_Ermrk_Int ,
        /*interruptPtr*/&cnm_grp_0_7_intr,
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(ERMRK.ERMRKInterruptCause) , FALCON_PIPE_OFFSET_MAC(1)},/*SMEM_LION3_ERMRK_INTERRUPT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(ERMRK.ERMRKInterruptMask) , FALCON_PIPE_OFFSET_MAC(1)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_7_intr_Pipe1_Ermrk_Int ,
        /*interruptPtr*/&cnm_grp_0_7_intr,
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
        /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Eq_Int ,
        /*interruptPtr*/&cnm_grp_0_1_intr
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.preEgrInterrupt.preEgrInterruptSummary) , FALCON_PIPE_OFFSET_MAC(1)}, /*SMEM_CHT_EQ_INT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.preEgrInterrupt.preEgrInterruptSummaryMask) , FALCON_PIPE_OFFSET_MAC(1)},  /*SMEM_CHT_EQ_INT_MASK_REG*/
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Eq_Int ,
        /*interruptPtr*/&cnm_grp_0_2_intr
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
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptSummary) , FALCON_PIPE_OFFSET_MAC(1)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptSummaryMask) , FALCON_PIPE_OFFSET_MAC(1)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/2 ,
            /*interruptPtr*/&preEgrInterruptSummary[1]
            },
        /*isTriggeredByWrite*/ 1
    }
};

#define CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(n, pipe)                                                                        \
{                                                                                                                              \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptCause[n-1]), FALCON_PIPE_OFFSET_MAC(pipe)},    \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptMask[n-1]), FALCON_PIPE_OFFSET_MAC(pipe)},     \
    /*myFatherInfo*/{                                                                                                          \
        /*myBitIndex*/n ,                                                                                                      \
        /*interruptPtr*/&CpuCodeRateLimitersInterruptSummary[pipe]                                                             \
        },                                                                                                                     \
    /*isTriggeredByWrite*/ 1                                                                                                   \
}

static SKERNEL_INTERRUPT_REG_INFO_STC cpu_code_rate_limiters_interrupts[] =
{
     CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 1, 0)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 2, 0)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 3, 0)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 4, 0)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 5, 0)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 6, 0)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 7, 0)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 8, 0)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 9, 0)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(10, 0)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(11, 0)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(12, 0)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(13, 0)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(14, 0)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(15, 0)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(16, 0)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 1, 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 2, 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 3, 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 4, 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 5, 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 6, 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 7, 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 8, 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 9, 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(10, 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(11, 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(12, 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(13, 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(14, 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(15, 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(16, 1)
};

static SKERNEL_INTERRUPT_REG_INFO_STC mll_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MLL.MLLGlobalCtrl.MLLInterruptCauseReg)},/*SMEM_LION3_MLL_INTERRUPT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MLL.MLLGlobalCtrl.MLLInterruptMaskReg)},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Mll_Int ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MLL.MLLGlobalCtrl.MLLInterruptCauseReg) , FALCON_PIPE_OFFSET_MAC(1)},/*SMEM_LION3_MLL_INTERRUPT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MLL.MLLGlobalCtrl.MLLInterruptMaskReg) , FALCON_PIPE_OFFSET_MAC(1)},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Mll_Int ,
            /*interruptPtr*/&cnm_grp_0_2_intr
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
            /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Iplr0_Int ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[1].policerInterruptCause)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[1].policerInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Iplr1_Int ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[2].policerInterruptCause)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[2].policerInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Eplr_Int ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[0].policerInterruptCause) , FALCON_PIPE_OFFSET_MAC(1)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[0].policerInterruptMask) , FALCON_PIPE_OFFSET_MAC(1)},
       /*myFatherInfo*/{
           /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Iplr0_Int ,
           /*interruptPtr*/&cnm_grp_0_2_intr
           },
       /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[1].policerInterruptCause) , FALCON_PIPE_OFFSET_MAC(1)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[1].policerInterruptMask) , FALCON_PIPE_OFFSET_MAC(1)},
       /*myFatherInfo*/{
           /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Iplr1_Int ,
           /*interruptPtr*/&cnm_grp_0_2_intr
           },
       /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[2].policerInterruptCause) , FALCON_PIPE_OFFSET_MAC(1)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[2].policerInterruptMask) , FALCON_PIPE_OFFSET_MAC(1)},
       /*myFatherInfo*/{
           /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Eplr_Int ,
           /*interruptPtr*/&cnm_grp_0_2_intr
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
            /*myBitIndex*/cnm_grp_0_5_intr_Rxdma0_Int ,
            /*interruptPtr*/&cnm_grp_0_5_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[1].interrupts.rxDMAInterruptSummaryCause)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[1].interrupts.rxDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_5_intr_Rxdma1_Int ,
            /*interruptPtr*/&cnm_grp_0_5_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[2].interrupts.rxDMAInterruptSummaryCause)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[2].interrupts.rxDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_5_intr_Rxdma2_Int ,
            /*interruptPtr*/&cnm_grp_0_5_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[3].interrupts.rxDMAInterruptSummaryCause)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[3].interrupts.rxDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_5_intr_Rxdma3_Int ,
            /*interruptPtr*/&cnm_grp_0_5_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[4].interrupts.rxDMAInterruptSummaryCause)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[4].interrupts.rxDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_5_intr_Rxdma4_Int ,
            /*interruptPtr*/&cnm_grp_0_5_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[5].interrupts.rxDMAInterruptSummaryCause)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[5].interrupts.rxDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_5_intr_Rxdma5_Int ,
            /*interruptPtr*/&cnm_grp_0_5_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[6].interrupts.rxDMAInterruptSummaryCause)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[6].interrupts.rxDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_5_intr_Rxdma6_Int ,
            /*interruptPtr*/&cnm_grp_0_5_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[7].interrupts.rxDMAInterruptSummaryCause)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[7].interrupts.rxDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_5_intr_Rxdma7_Int ,
            /*interruptPtr*/&cnm_grp_0_5_intr
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
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[1].interrupts.rxDMAInterrupt0Cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[1].interrupts.rxDMAInterrupt0Cause)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&RxDmaInterruptsSummary[1]
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[2].interrupts.rxDMAInterrupt0Cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[2].interrupts.rxDMAInterrupt0Cause)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&RxDmaInterruptsSummary[2]
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[3].interrupts.rxDMAInterrupt0Cause)},
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[3].interrupts.rxDMAInterrupt0Cause)},
       /*myFatherInfo*/{
           /*myBitIndex*/1 ,
           /*interruptPtr*/&RxDmaInterruptsSummary[3]
           },
       /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[4].interrupts.rxDMAInterrupt0Cause)},
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[4].interrupts.rxDMAInterrupt0Cause)},
       /*myFatherInfo*/{
           /*myBitIndex*/1 ,
           /*interruptPtr*/&RxDmaInterruptsSummary[4]
           },
       /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[5].interrupts.rxDMAInterrupt0Cause)},
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[5].interrupts.rxDMAInterrupt0Cause)},
       /*myFatherInfo*/{
           /*myBitIndex*/1 ,
           /*interruptPtr*/&RxDmaInterruptsSummary[5]
           },
       /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[5].interrupts.rxDMAInterrupt0Cause)},
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[5].interrupts.rxDMAInterrupt0Cause)},
       /*myFatherInfo*/{
           /*myBitIndex*/1 ,
           /*interruptPtr*/&RxDmaInterruptsSummary[6]
           },
       /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[5].interrupts.rxDMAInterrupt0Cause)},
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[5].interrupts.rxDMAInterrupt0Cause)},
       /*myFatherInfo*/{
           /*myBitIndex*/1 ,
           /*interruptPtr*/&RxDmaInterruptsSummary[7]
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
            /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Txdma0_Int ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[1].interrupts.txDMAInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[1].interrupts.txDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Txdma1_Int ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[2].interrupts.txDMAInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[2].interrupts.txDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Txdma2_Int ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[3].interrupts.txDMAInterruptSummary)},/**/
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[3].interrupts.txDMAInterruptSummaryMask)},/**/
       /*myFatherInfo*/{
           /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Txdma3_Int ,
           /*interruptPtr*/&cnm_grp_0_1_intr
           },
       /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[4].interrupts.txDMAInterruptSummary)},/**/
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[4].interrupts.txDMAInterruptSummaryMask)},/**/
       /*myFatherInfo*/{
           /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Txdma0_Int ,
           /*interruptPtr*/&cnm_grp_0_2_intr
           },
       /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[5].interrupts.txDMAInterruptSummary)},/**/
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[5].interrupts.txDMAInterruptSummaryMask)},/**/
       /*myFatherInfo*/{
           /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Txdma1_Int ,
           /*interruptPtr*/&cnm_grp_0_2_intr
           },
       /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[4].interrupts.txDMAInterruptSummary)},/**/
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[4].interrupts.txDMAInterruptSummaryMask)},/**/
       /*myFatherInfo*/{
           /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Txdma2_Int ,
           /*interruptPtr*/&cnm_grp_0_2_intr
           },
       /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[5].interrupts.txDMAInterruptSummary)},/**/
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[5].interrupts.txDMAInterruptSummaryMask)},/**/
       /*myFatherInfo*/{
           /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Txdma3_Int ,
           /*interruptPtr*/&cnm_grp_0_2_intr
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
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[1].interrupts.txDMAInterruptCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[1].interrupts.txDMAInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/4 ,
            /*interruptPtr*/&TxDmaInterruptsSummary[1]
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[2].interrupts.txDMAInterruptCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[2].interrupts.txDMAInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/4 ,
            /*interruptPtr*/&TxDmaInterruptsSummary[2]
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[3].interrupts.txDMAInterruptCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[3].interrupts.txDMAInterruptMask)},
        /*myFatherInfo*/{
           /*myBitIndex*/4 ,
           /*interruptPtr*/&TxDmaInterruptsSummary[3]
           },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[4].interrupts.txDMAInterruptCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[4].interrupts.txDMAInterruptMask)},
        /*myFatherInfo*/{
           /*myBitIndex*/4 ,
           /*interruptPtr*/&TxDmaInterruptsSummary[4]
           },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[5].interrupts.txDMAInterruptCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[5].interrupts.txDMAInterruptMask)},
        /*myFatherInfo*/{
           /*myBitIndex*/4 ,
           /*interruptPtr*/&TxDmaInterruptsSummary[5]
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
            /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Txfifo0_Int ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[1].interrupts.txFIFOInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[1].interrupts.txFIFOInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Txfifo1_Int ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[2].interrupts.txFIFOInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[2].interrupts.txFIFOInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Txfifo2_Int ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[3].interrupts.txFIFOInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[3].interrupts.txFIFOInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
           /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Txfifo3_Int ,
           /*interruptPtr*/&cnm_grp_0_1_intr
           },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[4].interrupts.txFIFOInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[4].interrupts.txFIFOInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
           /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Txfifo0_Int ,
           /*interruptPtr*/&cnm_grp_0_2_intr
           },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[5].interrupts.txFIFOInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[5].interrupts.txFIFOInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
           /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Txfifo1_Int ,
           /*interruptPtr*/&cnm_grp_0_2_intr
           },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[6].interrupts.txFIFOInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[6].interrupts.txFIFOInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
           /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Txfifo3_Int ,
           /*interruptPtr*/&cnm_grp_0_2_intr
           },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[7].interrupts.txFIFOInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[7].interrupts.txFIFOInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
           /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Txfifo3_Int ,
           /*interruptPtr*/&cnm_grp_0_2_intr
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
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[1].interrupts.txFIFOInterruptCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[1].interrupts.txFIFOInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&TxFifoInterruptsSummary[1]
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[2].interrupts.txFIFOInterruptCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[2].interrupts.txFIFOInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&TxFifoInterruptsSummary[2]
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[3].interrupts.txFIFOInterruptCause)},
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[3].interrupts.txFIFOInterruptMask)},
       /*myFatherInfo*/{
           /*myBitIndex*/1 ,
           /*interruptPtr*/&TxFifoInterruptsSummary[3]
           },
       /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[4].interrupts.txFIFOInterruptCause)},
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[4].interrupts.txFIFOInterruptMask)},
       /*myFatherInfo*/{
           /*myBitIndex*/1 ,
           /*interruptPtr*/&TxFifoInterruptsSummary[4]
           },
       /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[5].interrupts.txFIFOInterruptCause)},
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[5].interrupts.txFIFOInterruptMask)},
       /*myFatherInfo*/{
           /*myBitIndex*/1 ,
           /*interruptPtr*/&TxFifoInterruptsSummary[5]
           },
       /*isTriggeredByWrite*/ 1
    }
};

#define  raven_x_globalInterruptsSummary(ravenIndex) \
    {                                                \
        /*causeReg*/{ SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MG_RAVEN[ravenIndex].globalInterrupt.globalInterruptCause) }, /**/ \
        /*maskReg*/{ SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MG_RAVEN[ravenIndex].globalInterrupt.globalInterruptMask) }, /**/   \
        /*myFatherInfo*/{                                                     \
            /*myBitIndex*/cnm_grp_0_0_intr_Raven_0_Int + (ravenIndex%8),      \
            /*interruptPtr*/&cnm_grp_0_0_intr_x(ravenIndex & 8)/*0 or 8*/     \
        },                                                                    \
        /*isTriggeredByWrite*/ 1                                              \
    }


/* node in Raven MG that aggregates ports 0..15 */   \
static SKERNEL_INTERRUPT_REG_INFO_STC  raven_globalInterruptsSummary[] =
{
     raven_x_globalInterruptsSummary(0)
    ,raven_x_globalInterruptsSummary(1)
    ,raven_x_globalInterruptsSummary(2)
    ,raven_x_globalInterruptsSummary(3)
    ,raven_x_globalInterruptsSummary(4)
    ,raven_x_globalInterruptsSummary(5)
    ,raven_x_globalInterruptsSummary(6)
    ,raven_x_globalInterruptsSummary(7)
    ,raven_x_globalInterruptsSummary(8)
    ,raven_x_globalInterruptsSummary(9)
    ,raven_x_globalInterruptsSummary(10)
    ,raven_x_globalInterruptsSummary(11)
    ,raven_x_globalInterruptsSummary(12)
    ,raven_x_globalInterruptsSummary(13)
    ,raven_x_globalInterruptsSummary(14)
    ,raven_x_globalInterruptsSummary(15)
};

#define  raven_x_portsInterruptsSummary(ravenIndex)  \
    {                                                \
        /*causeReg*/{ SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MG_RAVEN[ravenIndex].globalInterrupt.portsInterruptCause) }, /**/\
        /*maskReg*/{ SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MG_RAVEN[ravenIndex].globalInterrupt.portsInterruptMask) }, /**/  \
        /*myFatherInfo*/{                                                \
            /*myBitIndex*/PortsIntSum,                                   \
            /*interruptPtr*/&raven_globalInterruptsSummary[ravenIndex]   \
        },                                                               \
        /*isTriggeredByWrite*/ 1                                         \
    }

#define  raven_x_ports1InterruptsSummary(ravenIndex)  \
    {                                                \
        /*causeReg*/{ SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MG_RAVEN[ravenIndex].globalInterrupt.ports1InterruptsSummaryCause) }, /**/\
        /*maskReg*/{ SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MG_RAVEN[ravenIndex].globalInterrupt.ports1InterruptsSummaryMask) }, /**/  \
        /*myFatherInfo*/{                                                \
            /*myBitIndex*/ports_1_int_sum,                                   \
            /*interruptPtr*/&raven_globalInterruptsSummary[ravenIndex]   \
        },                                                               \
        /*isTriggeredByWrite*/ 1                                         \
    }


/* node in Raven MG that aggregates ports 0..15 */   \
static SKERNEL_INTERRUPT_REG_INFO_STC  raven_portsInterruptsSummary[] =
{
     raven_x_portsInterruptsSummary(0)
    ,raven_x_portsInterruptsSummary(1)
    ,raven_x_portsInterruptsSummary(2)
    ,raven_x_portsInterruptsSummary(3)
    ,raven_x_portsInterruptsSummary(4)
    ,raven_x_portsInterruptsSummary(5)
    ,raven_x_portsInterruptsSummary(6)
    ,raven_x_portsInterruptsSummary(7)
    ,raven_x_portsInterruptsSummary(8)
    ,raven_x_portsInterruptsSummary(9)
    ,raven_x_portsInterruptsSummary(10)
    ,raven_x_portsInterruptsSummary(11)
    ,raven_x_portsInterruptsSummary(12)
    ,raven_x_portsInterruptsSummary(13)
    ,raven_x_portsInterruptsSummary(14)
    ,raven_x_portsInterruptsSummary(15)
};

static SKERNEL_INTERRUPT_REG_INFO_STC  raven_ports1InterruptsSummary[] =
{
     raven_x_ports1InterruptsSummary(0)
    ,raven_x_ports1InterruptsSummary(1)
    ,raven_x_ports1InterruptsSummary(2)
    ,raven_x_ports1InterruptsSummary(3)
    ,raven_x_ports1InterruptsSummary(4)
    ,raven_x_ports1InterruptsSummary(5)
    ,raven_x_ports1InterruptsSummary(6)
    ,raven_x_ports1InterruptsSummary(7)
    ,raven_x_ports1InterruptsSummary(8)
    ,raven_x_ports1InterruptsSummary(9)
    ,raven_x_ports1InterruptsSummary(10)
    ,raven_x_ports1InterruptsSummary(11)
    ,raven_x_ports1InterruptsSummary(12)
    ,raven_x_ports1InterruptsSummary(13)
    ,raven_x_ports1InterruptsSummary(14)
    ,raven_x_ports1InterruptsSummary(15)
};

#define RAVEN_OFFSET_CNS         0x01000000
#define RAVEN_OFFSET(globalRavenIndex/*0..15*/) \
    (((((globalRavenIndex)/4) & 1) == 0) ?/* is mirrored tile or not */ \
     ((((globalRavenIndex)/4)*TILE_OFFSET) + ((   (globalRavenIndex)%4 )*RAVEN_OFFSET_CNS) ) : /* not mirrored */ \
     ((((globalRavenIndex)/4)*TILE_OFFSET) + ((3-((globalRavenIndex)%4))*RAVEN_OFFSET_CNS) )) /* mirrored tile */

#define _raven_LMU_x_causeInterruptsSummary(bit, ravenIndex/*0..15*/ , lmuIndex/*0,1*/)                       \
    {                                                                                       \
        /*causeReg*/ { SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(LMU[lmuIndex].summary_cause) , RAVEN_OFFSET(ravenIndex)},  \
        /*causeReg*/ { SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(LMU[lmuIndex].summary_mask)  , RAVEN_OFFSET(ravenIndex)},   \
        /*myFatherInfo*/{                                                                   \
            /*myBitIndex*/bit,                                                              \
            /*interruptPtr*/&raven_ports1InterruptsSummary[ravenIndex]                      \
        },                                                                                  \
        /*isTriggeredByWrite*/ 1                                                            \
    }

#define raven_LMU_0_causeInterruptsSummary(bit, ravenIndex/*0..15*/) \
    _raven_LMU_x_causeInterruptsSummary(bit,ravenIndex,0)
#define raven_LMU_1_causeInterruptsSummary(bit, ravenIndex/*0..15*/) \
    _raven_LMU_x_causeInterruptsSummary(bit,ravenIndex,1)

static SKERNEL_INTERRUPT_REG_INFO_STC  raven_LMU_causeInterruptsSummary[] =
{
     /* LMU0 of Ravens 0..15 */
     raven_LMU_0_causeInterruptsSummary(6,  0)
    ,raven_LMU_0_causeInterruptsSummary(6,  1)
    ,raven_LMU_0_causeInterruptsSummary(6,  2)
    ,raven_LMU_0_causeInterruptsSummary(6,  3)
    ,raven_LMU_0_causeInterruptsSummary(6,  4)
    ,raven_LMU_0_causeInterruptsSummary(6,  5)
    ,raven_LMU_0_causeInterruptsSummary(6,  6)
    ,raven_LMU_0_causeInterruptsSummary(6,  7)
    ,raven_LMU_0_causeInterruptsSummary(6,  8)
    ,raven_LMU_0_causeInterruptsSummary(6,  9)
    ,raven_LMU_0_causeInterruptsSummary(6, 10)
    ,raven_LMU_0_causeInterruptsSummary(6, 11)
    ,raven_LMU_0_causeInterruptsSummary(6, 12)
    ,raven_LMU_0_causeInterruptsSummary(6, 13)
    ,raven_LMU_0_causeInterruptsSummary(6, 14)
    ,raven_LMU_0_causeInterruptsSummary(6, 15)
    /* LMU1 of Ravens 0..15 */
    ,raven_LMU_1_causeInterruptsSummary(7,  0)
    ,raven_LMU_1_causeInterruptsSummary(7,  1)
    ,raven_LMU_1_causeInterruptsSummary(7,  2)
    ,raven_LMU_1_causeInterruptsSummary(7,  3)
    ,raven_LMU_1_causeInterruptsSummary(7,  4)
    ,raven_LMU_1_causeInterruptsSummary(7,  5)
    ,raven_LMU_1_causeInterruptsSummary(7,  6)
    ,raven_LMU_1_causeInterruptsSummary(7,  7)
    ,raven_LMU_1_causeInterruptsSummary(7,  8)
    ,raven_LMU_1_causeInterruptsSummary(7,  9)
    ,raven_LMU_1_causeInterruptsSummary(7, 10)
    ,raven_LMU_1_causeInterruptsSummary(7, 11)
    ,raven_LMU_1_causeInterruptsSummary(7, 12)
    ,raven_LMU_1_causeInterruptsSummary(7, 13)
    ,raven_LMU_1_causeInterruptsSummary(7, 14)
    ,raven_LMU_1_causeInterruptsSummary(7, 15)
};

#define raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex, k)                                                    \
    {                                                                                                       \
        /*causeReg*/ { SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(LMU[lmuIndex].latencyOverThreshold_cause[k]) , RAVEN_OFFSET(ravenIndex)},  \
        /*causeReg*/ { SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(LMU[lmuIndex].latencyOverThreshold_mask[k])  , RAVEN_OFFSET(ravenIndex)},   \
        /*myFatherInfo*/{                                                                                   \
            /*myBitIndex*/1 + k,                                                                            \
            /*interruptPtr*/&raven_LMU_causeInterruptsSummary[(16*(lmuIndex))+(ravenIndex)]                 \
        },                                                                                                  \
        /*isTriggeredByWrite*/ 1                                                                            \
    }

#define RAVEN_LMU_LATENCY_OVER_THRESHOLD_MAC(lmuIndex,ravenIndex)          \
         raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex,  0)                 \
        ,raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex,  1)                 \
        ,raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex,  2)                 \
        ,raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex,  3)                 \
        ,raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex,  4)                 \
        ,raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex,  5)                 \
        ,raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex,  6)                 \
        ,raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex,  7)                 \
        ,raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex,  8)                 \
        ,raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex,  9)                 \
        ,raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex, 10)                 \
        ,raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex, 11)                 \
        ,raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex, 12)                 \
        ,raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex, 13)                 \
        ,raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex, 14)                 \
        ,raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex, 15)                 \
        ,raven_LMU_latencyOverThresholdCause(lmuIndex,ravenIndex, 16)                 \


#define RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC(RavenIndex)    \
     RAVEN_LMU_LATENCY_OVER_THRESHOLD_MAC(0, RavenIndex)        \
    ,RAVEN_LMU_LATENCY_OVER_THRESHOLD_MAC(1, RavenIndex)


static SKERNEL_INTERRUPT_REG_INFO_STC lmu_latency_over_threshold_interrupts[] =
{
     RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC( 0)
    ,RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC( 1)
    ,RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC( 2)
    ,RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC( 3)
    ,RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC( 4)
    ,RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC( 5)
    ,RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC( 6)
    ,RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC( 7)
    ,RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC( 8)
    ,RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC( 9)
    ,RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC(10)
    ,RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC(11)
    ,RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC(12)
    ,RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC(13)
    ,RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC(14)
    ,RAVEN_LMU_0_1_LATENCY_OVER_THRESHOLD_MAC(15)
};

#define GLOBAL_PORT_NUMBER(ravenIndex,localPort)    (16*(ravenIndex) + (localPort))


/* define the MTI port summary directed from MG ports summary */
#define  DEFINE_MTI_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(ravenIndex,localPort, bitInMg, regInMg) \
static SKERNEL_INTERRUPT_REG_INFO_STC  mti_interruptSummary_Raven##ravenIndex##_port##localPort = \
{                                                  \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[GLOBAL_PORT_NUMBER(ravenIndex,localPort) / 8].MTI_GLOBAL.globalInterruptSummaryCause)}, /**/  \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[GLOBAL_PORT_NUMBER(ravenIndex,localPort) / 8].MTI_GLOBAL.globalInterruptSummaryMask)},  /**/  \
    /*myFatherInfo*/{                              \
        /*myBitIndex*/ bitInMg ,                   \
        /*interruptPtr*/&regInMg                   \
        },                                         \
    /*isTriggeredByWrite*/ 1                       \
}

/* define the MTI MAC port summary, for port 0..63 in ports_int_sum per tile  */
#define  DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex,localPort) \
    DEFINE_MTI_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(ravenIndex,localPort, (25+((localPort/8)%2))/*bitInMg*/,raven_portsInterruptsSummary[ravenIndex]/*regInMg*/)


#define mti_interruptSummary_RAVEN_16_PORTS(ravenIndex)                                         \
    DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex , 0);   \
    DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex , 1);   \
    DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex , 2);   \
    DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex , 3);   \
    DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex , 4);   \
    DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex , 5);   \
    DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex , 6);   \
    DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex , 7);   \
    DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex , 8);   \
    DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex , 9);   \
    DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex ,10);   \
    DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex ,11);   \
    DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex ,12);   \
    DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex ,13);   \
    DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex ,14);   \
    DEFINE_MTI_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(ravenIndex ,15)

mti_interruptSummary_RAVEN_16_PORTS(0);
mti_interruptSummary_RAVEN_16_PORTS(1);
mti_interruptSummary_RAVEN_16_PORTS(2);
mti_interruptSummary_RAVEN_16_PORTS(3);
mti_interruptSummary_RAVEN_16_PORTS(4);
mti_interruptSummary_RAVEN_16_PORTS(5);
mti_interruptSummary_RAVEN_16_PORTS(6);
mti_interruptSummary_RAVEN_16_PORTS(7);
mti_interruptSummary_RAVEN_16_PORTS(8);
mti_interruptSummary_RAVEN_16_PORTS(9);
mti_interruptSummary_RAVEN_16_PORTS(10);
mti_interruptSummary_RAVEN_16_PORTS(11);
mti_interruptSummary_RAVEN_16_PORTS(12);
mti_interruptSummary_RAVEN_16_PORTS(13);
mti_interruptSummary_RAVEN_16_PORTS(14);
mti_interruptSummary_RAVEN_16_PORTS(15);

/* define the MTI port summary directed from MG ports summary */
#define MTI_CPU_PORT_INTERRUPT_CAUSE_MAC(ravenIndex)    \
{                                                       \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_cpu_port[ravenIndex].MTI_EXT_PORT.portInterruptCause)}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_cpu_port[ravenIndex].MTI_EXT_PORT.portInterruptMask)},  \
    /*myFatherInfo*/{                              \
        /*myBitIndex*/ 24,                   \
        /*interruptPtr*/&raven_portsInterruptsSummary[ravenIndex]                   \
        },                                         \
    /*isTriggeredByWrite*/ 1                       \
}


/* node for MTI64 mac ports */
#define  MTI_64_INTERRUPT_CAUSE_MAC(ravenIndex,localPort)      \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[GLOBAL_PORT_NUMBER(ravenIndex,localPort)].MTI_EXT_PORT.portInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[GLOBAL_PORT_NUMBER(ravenIndex,localPort)].MTI_EXT_PORT.portInterruptMask) },  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                     \
        /*myBitIndex*/(1+(localPort % 8)) ,                    \
        /*interruptPtr*/&mti_interruptSummary_Raven##ravenIndex##_port##localPort      \
    },                                                    \
    /*isTriggeredByWrite*/ 1                              \
}

/* node for MTI400 mac ports */
#define  MTI_400_INTERRUPT_CAUSE_MAC(ravenIndex,localPort)                \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[GLOBAL_PORT_NUMBER(ravenIndex,localPort)].MTI_EXT_PORT.segPortInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[GLOBAL_PORT_NUMBER(ravenIndex,localPort)].MTI_EXT_PORT.segPortInterruptMask) },  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                     \
        /*myBitIndex*/(9+((localPort/8) % 2)) ,                \
        /*interruptPtr*/&mti_interruptSummary_Raven##ravenIndex##_port##localPort      \
    },                                                    \
    /*isTriggeredByWrite*/ 1                              \
}

#define FEC_CE_INTERRUPT_CAUSE_MAC(ravenIndex,localPort)                \
{                                                  \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[GLOBAL_PORT_NUMBER(ravenIndex,localPort) / 8].MTI_GLOBAL.globalFecCeInterruptCause)}, /**/  \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[GLOBAL_PORT_NUMBER(ravenIndex,localPort) / 8].MTI_GLOBAL.globalFecCeInterruptMask)},  /**/  \
    /*myFatherInfo*/{                               \
        /*myBitIndex*/12 ,                          \
        /*interruptPtr*/&mti_interruptSummary_Raven##ravenIndex##_port##localPort      \
        },                                          \
    /*isTriggeredByWrite*/ 1                        \
}

#define FEC_NCE_INTERRUPT_CAUSE_MAC(ravenIndex,localPort)                \
{                                                  \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[GLOBAL_PORT_NUMBER(ravenIndex,localPort) / 8].MTI_GLOBAL.globalFecNceInterruptCause)}, /**/  \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[GLOBAL_PORT_NUMBER(ravenIndex,localPort) / 8].MTI_GLOBAL.globalFecNceInterruptMask)},  /**/  \
    /*myFatherInfo*/{                               \
        /*myBitIndex*/13 ,                          \
        /*interruptPtr*/&mti_interruptSummary_Raven##ravenIndex##_port##localPort      \
        },                                          \
    /*isTriggeredByWrite*/ 1                        \
}

#define MTI_COMMON_INTERRUPT_CAUSE_MAC(ravenIndex,localPort) \
    MTI_64_INTERRUPT_CAUSE_MAC(ravenIndex,localPort), MTI_400_INTERRUPT_CAUSE_MAC(ravenIndex,localPort)

#define MTI_SINGLE_INTERRUPT_CAUSE_MAC(ravenIndex,localPort) \
    MTI_64_INTERRUPT_CAUSE_MAC(ravenIndex,localPort)

#define MTI_COMMON_WITH_GLOBAL_INTERRUPT_CAUSE_MAC(ravenIndex,localPort) \
    MTI_COMMON_INTERRUPT_CAUSE_MAC(ravenIndex,localPort), FEC_CE_INTERRUPT_CAUSE_MAC(ravenIndex,localPort), FEC_NCE_INTERRUPT_CAUSE_MAC(ravenIndex,localPort)


#define RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC(ravenIndex)      \
     MTI_COMMON_WITH_GLOBAL_INTERRUPT_CAUSE_MAC(ravenIndex , 0)    \
    ,MTI_SINGLE_INTERRUPT_CAUSE_MAC(ravenIndex , 1)    \
    ,MTI_SINGLE_INTERRUPT_CAUSE_MAC(ravenIndex , 2)    \
    ,MTI_SINGLE_INTERRUPT_CAUSE_MAC(ravenIndex , 3)    \
    ,MTI_COMMON_INTERRUPT_CAUSE_MAC(ravenIndex , 4)    \
    ,MTI_SINGLE_INTERRUPT_CAUSE_MAC(ravenIndex , 5)    \
    ,MTI_SINGLE_INTERRUPT_CAUSE_MAC(ravenIndex , 6)    \
    ,MTI_SINGLE_INTERRUPT_CAUSE_MAC(ravenIndex , 7)    \
    ,MTI_COMMON_WITH_GLOBAL_INTERRUPT_CAUSE_MAC(ravenIndex , 8)    \
    ,MTI_SINGLE_INTERRUPT_CAUSE_MAC(ravenIndex , 9)    \
    ,MTI_SINGLE_INTERRUPT_CAUSE_MAC(ravenIndex ,10)    \
    ,MTI_SINGLE_INTERRUPT_CAUSE_MAC(ravenIndex ,11)    \
    ,MTI_COMMON_INTERRUPT_CAUSE_MAC(ravenIndex ,12)    \
    ,MTI_SINGLE_INTERRUPT_CAUSE_MAC(ravenIndex ,13)    \
    ,MTI_SINGLE_INTERRUPT_CAUSE_MAC(ravenIndex ,14)    \
    ,MTI_SINGLE_INTERRUPT_CAUSE_MAC(ravenIndex ,15)    \
    ,MTI_CPU_PORT_INTERRUPT_CAUSE_MAC(ravenIndex)

static SKERNEL_INTERRUPT_REG_INFO_STC mti_mac_ports_interrupts[] =
{
     RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC( 0)
    ,RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC( 1)
    ,RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC( 2)
    ,RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC( 3)
    ,RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC( 4)
    ,RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC( 5)
    ,RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC( 6)
    ,RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC( 7)
    ,RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC( 8)
    ,RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC( 9)
    ,RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC(10)
    ,RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC(11)
    ,RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC(12)
    ,RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC(13)
    ,RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC(14)
    ,RAVEN_ALL_PORTS_INTERRUPT_CAUSE_MAC(15)
};

static SKERNEL_INTERRUPT_REG_INFO_STC PHA_summary_Interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.pha_regs.errorsAndInterrupts.PHAInterruptSumCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.pha_regs.errorsAndInterrupts.PHAInterruptSumMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_1_intr_Pipe0_Pha_Int ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
    ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.pha_regs.errorsAndInterrupts.PHAInterruptSumCause), FALCON_PIPE_OFFSET_MAC(1)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.pha_regs.errorsAndInterrupts.PHAInterruptSumMask) , FALCON_PIPE_OFFSET_MAC(1)},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_2_intr_Pipe1_Pha_Int ,
            /*interruptPtr*/&cnm_grp_0_2_intr
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
    ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPA.ppa_regs.PPAInterruptSumCause),FALCON_PIPE_OFFSET_MAC(1)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPA.ppa_regs.PPAInterruptSumMask) ,FALCON_PIPE_OFFSET_MAC(1)},
        /*myFatherInfo*/{
            /*myBitIndex*/ 2 ,
            /*interruptPtr*/&PHA_summary_Interrupts[1]
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
    }                                                                                                 \
    ,{                                                                                                \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPG[ppgId].ppg_regs.PPGInterruptSumCause),FALCON_PIPE_OFFSET_MAC(1)}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPG[ppgId].ppg_regs.PPGInterruptSumMask) ,FALCON_PIPE_OFFSET_MAC(1)}, \
    /*myFatherInfo*/{                                                                                 \
        /*myBitIndex*/ (ppgId)+2 ,                                                                    \
        /*interruptPtr*/&PPA_summary_Interrupts[1]                                                    \
        },                                                                                            \
    /*isTriggeredByWrite*/ 1                                                                          \
    }

static SKERNEL_INTERRUPT_REG_INFO_STC PPG_summary_Interrupts[6] =
{
     PPG_INTERRUPT_SUM_CAUSE_NODE(0)
    ,PPG_INTERRUPT_SUM_CAUSE_NODE(1)
    ,PPG_INTERRUPT_SUM_CAUSE_NODE(2)
};

#define PPN_IN_PPG_INTERRUPT_CAUSE_NODE(ppgId,ppnId)   \
    {                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPG[ppgId].PPN[ppnId].ppn_regs.PPNInternalErrorCause)}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPG[ppgId].PPN[ppnId].ppn_regs.PPNInternalErrorMask)},  \
    /*myFatherInfo*/{                                                                                 \
        /*myBitIndex*/ (ppnId)+2 ,                                                                    \
        /*interruptPtr*/&PPG_summary_Interrupts[2*ppgId + 0]                                          \
        },                                                                                            \
    /*isTriggeredByWrite*/ 1                                                                          \
    }                                                                                                 \
    ,{                                                                                                \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPG[ppgId].PPN[ppnId].ppn_regs.PPNInternalErrorCause),FALCON_PIPE_OFFSET_MAC(1)}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPG[ppgId].PPN[ppnId].ppn_regs.PPNInternalErrorMask) ,FALCON_PIPE_OFFSET_MAC(1)},  \
    /*myFatherInfo*/{                                                                                 \
        /*myBitIndex*/ (ppnId)+2 ,                                                                    \
        /*interruptPtr*/&PPG_summary_Interrupts[2*ppgId + 1]                                          \
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
    ,PPN_IN_PPG_INTERRUPT_CAUSE_NODE(ppgId,8)


#define PPG_CAUSE_NODE(ppgId)                   \
    {                                           \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPG[ppgId].ppg_regs.PPGInternalErrorCause)}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPG[ppgId].ppg_regs.PPGInternalErrorMask)},  \
    /*myFatherInfo*/{                                                                                 \
        /*myBitIndex*/ 1 ,                                                                            \
        /*interruptPtr*/&PPG_summary_Interrupts[2*ppgId + 0]                                          \
        },                                                                                            \
    /*isTriggeredByWrite*/ 1                                                                          \
    }                                                                                                 \
    ,{                                                                                                \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPG[ppgId].ppg_regs.PPGInternalErrorCause),FALCON_PIPE_OFFSET_MAC(1)}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPG[ppgId].ppg_regs.PPGInternalErrorMask) ,FALCON_PIPE_OFFSET_MAC(1)}, \
    /*myFatherInfo*/{                                                                                 \
        /*myBitIndex*/ 1 ,                                                                            \
        /*interruptPtr*/&PPG_summary_Interrupts[2*ppgId + 1]                                          \
        },                                                                                            \
    /*isTriggeredByWrite*/ 1                                                                          \
    },                                                                                                \
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

    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.pha_regs.errorsAndInterrupts.PHAInternalErrorCause),FALCON_PIPE_OFFSET_MAC(1)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.pha_regs.errorsAndInterrupts.PHAInternalErrorMask) ,FALCON_PIPE_OFFSET_MAC(1)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&PHA_summary_Interrupts[1]
            },
        /*isTriggeredByWrite*/ 1
    },

    /*********************/

    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPA.ppa_regs.PPAInternalErrorCause),FALCON_PIPE_OFFSET_MAC(1)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PHA.PPA.ppa_regs.PPAInternalErrorMask) ,FALCON_PIPE_OFFSET_MAC(1)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&PPA_summary_Interrupts[1]
            },
        /*isTriggeredByWrite*/ 1
    },

    PPG_CAUSE_NODE(0),
    PPG_CAUSE_NODE(1),
    PPG_CAUSE_NODE(2)

};

/* get the MG index from the currMgInfoPtr->causeReg.mgUnitIndex */
#define   GET_MG_INDEX(currMgInfoPtr)    (currMgInfoPtr->causeReg.mgUnitIndex & (~MG_UNIT_USED_CNS))
/* check if the interrupt node is one of MG interrupts */
static GT_BOOL isMgIntReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_INTERRUPT_REG_INFO_STC *currDbPtr
)
{
    GT_U32  start_MG_registers = FIELD_OFFSET_IN_STC_MAC(MG,SMEM_CHT_PP_REGS_ADDR_STC);
    GT_U32  end_MG_registers   = start_MG_registers +
                    sizeof(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->MG);

    if(currDbPtr->causeReg.registersDbType == SKERNEL_REGISTERS_DB_TYPE_LEGACY_E &&
       (currDbPtr->causeReg.registerOffsetInDb >= start_MG_registers &&
        currDbPtr->causeReg.registerOffsetInDb <  end_MG_registers))
    {
        return GT_TRUE;
    }

    return GT_FALSE;
}


/* state how the tile 0 interrupts connected to MG[0] and
         how the tile 1 interrupts connected to MG[0] (same MG[0] as for tile 0) */
typedef struct{
    GT_U32                          bitAtFatherForTile0Interrupt;
    SKERNEL_INTERRUPT_REG_INFO_STC  *fatherForTile0InterruptPtr;
    GT_U32                          bitAtFatherForTile1Interrupt;
    SKERNEL_INTERRUPT_REG_INFO_STC  *fatherForTile1InterruptPtr;
}FULL_CHIP_TO_MG0_CONNECTIONS;

static  FULL_CHIP_TO_MG0_CONNECTIONS fullChipTile1ToTile0Connections[] =
{
#if 0 /* The Ravens already explicitly connected */
     {   cnm_grp_0_0_intr_Raven_0_Int         , &cnm_grp_0_0_intr   , cnm_grp_0_0_intr_Raven_4_Int               , &cnm_grp_0_0_intr}
    ,{   cnm_grp_0_0_intr_Raven_1_Int         , &cnm_grp_0_0_intr   , cnm_grp_0_0_intr_Raven_5_Int               , &cnm_grp_0_0_intr}
    ,{   cnm_grp_0_0_intr_Raven_2_Int         , &cnm_grp_0_0_intr   , cnm_grp_0_0_intr_Raven_6_Int               , &cnm_grp_0_0_intr}
    ,{   cnm_grp_0_0_intr_Raven_3_Int         , &cnm_grp_0_0_intr   , cnm_grp_0_0_intr_Raven_7_Int               , &cnm_grp_0_0_intr}
#endif /*0*/
     {   cnm_grp_0_0_intr_Pb_Int              , &cnm_grp_0_0_intr   , cnm_grp_0_5_intr_tile1_Pb_Int              , &cnm_grp_0_5_intr}
    ,{   cnm_grp_0_0_intr_Em_Int              , &cnm_grp_0_0_intr   , cnm_grp_0_5_intr_tile1_Em_Int              , &cnm_grp_0_5_intr}
    ,{   cnm_grp_0_0_intr_Mt_Int              , &cnm_grp_0_0_intr   , cnm_grp_0_5_intr_tile1_Mt_Int              , &cnm_grp_0_5_intr}
    ,{   cnm_grp_0_0_intr_Packer0_Int         , &cnm_grp_0_0_intr   , cnm_grp_0_5_intr_tile1_Packer0_Int         , &cnm_grp_0_5_intr}
    ,{   cnm_grp_0_0_intr_Packer1_Int         , &cnm_grp_0_0_intr   , cnm_grp_0_5_intr_tile1_Packer1_Int         , &cnm_grp_0_5_intr}
    ,{   cnm_grp_0_0_intr_Pdx_Int             , &cnm_grp_0_0_intr   , cnm_grp_0_5_intr_tile1_Pdx_Int             , &cnm_grp_0_5_intr}
    ,{   cnm_grp_0_0_intr_Pfcc_Int            , &cnm_grp_0_0_intr   , cnm_grp_0_5_intr_tile1_Pfcc_Int            , &cnm_grp_0_5_intr}
    ,{   cnm_grp_0_0_intr_Psi_Int             , &cnm_grp_0_0_intr   , cnm_grp_0_5_intr_tile1_Psi_Int             , &cnm_grp_0_5_intr}
    ,{   cnm_grp_0_0_intr_Shm_Int             , &cnm_grp_0_0_intr   , cnm_grp_0_5_intr_tile1_Shm_Int             , &cnm_grp_0_5_intr}
    ,{   cnm_grp_0_0_intr_Tcam_Int            , &cnm_grp_0_0_intr   , cnm_grp_0_5_intr_tile1_Tcam_Int            , &cnm_grp_0_5_intr}

    ,{   cnm_grp_0_1_intr_Pipe0_Bma_Int       , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Bma_Int       , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Eft_Int       , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Eft_Int       , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Eoam_Int      , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Eoam_Int      , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Epcl_Int      , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Epcl_Int      , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Eplr_Int      , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Eplr_Int      , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Eq_Int        , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Eq_Int        , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Erep_Int      , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Erep_Int      , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Ha_Int        , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Ha_Int        , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Ia_Int        , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Ia_Int        , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Ioam_Int      , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Ioam_Int      , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Iplr0_Int     , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Iplr0_Int     , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Iplr1_Int     , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Iplr1_Int     , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Ipvx_Int      , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Ipvx_Int      , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Lpm_Int       , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Lpm_Int       , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Mll_Int       , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Mll_Int       , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Pcl_Int       , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Pcl_Int       , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Preq_Int      , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Preq_Int      , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Egf_Qag_Int   , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Egf_Qag_Int   , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Egf_Sht_Int   , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Egf_Sht_Int   , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Pha_Int       , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Pha_Int       , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Tti_Int       , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Tti_Int       , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Txdma0_Int    , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Txdma0_Int    , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Txfifo0_Int   , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Txfifo0_Int   , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Txdma1_Int    , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Txdma1_Int    , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Txfifo1_Int   , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Txfifo1_Int   , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Txdma2_Int    , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Txdma2_Int    , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Txfifo2_Int   , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Txfifo2_Int   , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Txdma3_Int    , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Txdma3_Int    , &cnm_grp_0_3_intr}
    ,{   cnm_grp_0_1_intr_Pipe0_Txfifo3_Int   , &cnm_grp_0_1_intr   , cnm_grp_0_3_intr_tile1_Pipe0_Txfifo3_Int   , &cnm_grp_0_3_intr}

    ,{   cnm_grp_0_2_intr_Pipe1_Bma_Int       , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Bma_Int       , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Eft_Int       , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Eft_Int       , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Eoam_Int      , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Eoam_Int      , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Epcl_Int      , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Epcl_Int      , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Eplr_Int      , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Eplr_Int      , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Eq_Int        , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Eq_Int        , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Erep_Int      , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Erep_Int      , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Ha_Int        , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Ha_Int        , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Ia_Int        , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Ia_Int        , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Ioam_Int      , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Ioam_Int      , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Iplr0_Int     , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Iplr0_Int     , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Iplr1_Int     , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Iplr1_Int     , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Ipvx_Int      , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Ipvx_Int      , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Lpm_Int       , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Lpm_Int       , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Mll_Int       , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Mll_Int       , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Pcl_Int       , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Pcl_Int       , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Preq_Int      , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Preq_Int      , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Egf_Qag_Int   , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Egf_Qag_Int   , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Egf_Sht_Int   , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Egf_Sht_Int   , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Pha_Int       , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Pha_Int       , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Tti_Int       , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Tti_Int       , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Txdma0_Int    , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Txdma0_Int    , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Txfifo0_Int   , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Txfifo0_Int   , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Txdma1_Int    , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Txdma1_Int    , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Txfifo1_Int   , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Txfifo1_Int   , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Txdma2_Int    , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Txdma2_Int    , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Txfifo2_Int   , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Txfifo2_Int   , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Txdma3_Int    , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Txdma3_Int    , &cnm_grp_0_4_intr}
    ,{   cnm_grp_0_2_intr_Pipe1_Txfifo3_Int   , &cnm_grp_0_2_intr   , cnm_grp_0_4_intr_tile1_Pipe1_Txfifo3_Int   , &cnm_grp_0_4_intr}

    ,{   cnm_grp_0_5_intr_Rxdma0_Int          , &cnm_grp_0_5_intr   , cnm_grp_0_5_intr_tile1_Rxdma0_Int          , &cnm_grp_0_5_intr}
    ,{   cnm_grp_0_5_intr_Rxdma1_Int          , &cnm_grp_0_5_intr   , cnm_grp_0_5_intr_tile1_Rxdma1_Int          , &cnm_grp_0_5_intr}
    ,{   cnm_grp_0_5_intr_Rxdma2_Int          , &cnm_grp_0_5_intr   , cnm_grp_0_5_intr_tile1_Rxdma2_Int          , &cnm_grp_0_5_intr}
    ,{   cnm_grp_0_5_intr_Rxdma3_Int          , &cnm_grp_0_5_intr   , cnm_grp_0_5_intr_tile1_Rxdma3_Int          , &cnm_grp_0_5_intr}
    ,{   cnm_grp_0_5_intr_Rxdma4_Int          , &cnm_grp_0_5_intr   , cnm_grp_0_5_intr_tile1_Rxdma4_Int          , &cnm_grp_0_5_intr}
    ,{   cnm_grp_0_5_intr_Rxdma5_Int          , &cnm_grp_0_5_intr   , cnm_grp_0_5_intr_tile1_Rxdma5_Int          , &cnm_grp_0_5_intr}
    ,{   cnm_grp_0_5_intr_Rxdma6_Int          , &cnm_grp_0_5_intr   , cnm_grp_0_5_intr_tile1_Rxdma6_Int          , &cnm_grp_0_5_intr}
    ,{   cnm_grp_0_5_intr_Rxdma7_Int          , &cnm_grp_0_5_intr   , cnm_grp_0_5_intr_tile1_Rxdma7_Int          , &cnm_grp_0_5_intr}

    ,{   cnm_grp_0_6_intr_Pipe0_Cnc0_Int      , &cnm_grp_0_6_intr   , cnm_grp_0_6_intr_tile1_Pipe0_Cnc0_Int      , &cnm_grp_0_6_intr}
    ,{   cnm_grp_0_6_intr_Pipe0_Cnc1_Int      , &cnm_grp_0_6_intr   , cnm_grp_0_6_intr_tile1_Pipe0_Cnc1_Int      , &cnm_grp_0_6_intr}
    ,{   cnm_grp_0_6_intr_Pipe1_Cnc0_Int      , &cnm_grp_0_6_intr   , cnm_grp_0_6_intr_tile1_Pipe1_Cnc0_Int      , &cnm_grp_0_6_intr}
    ,{   cnm_grp_0_6_intr_Pipe1_Cnc1_Int      , &cnm_grp_0_6_intr   , cnm_grp_0_6_intr_tile1_Pipe1_Cnc1_Int      , &cnm_grp_0_6_intr}

    ,{   cnm_grp_0_7_intr_Pipe0_Ermrk_Int     , &cnm_grp_0_7_intr   , cnm_grp_0_7_intr_Tile1_Pipe0_Ermrk_Int     , &cnm_grp_0_7_intr}
    ,{   cnm_grp_0_7_intr_Pipe1_Ermrk_Int     , &cnm_grp_0_7_intr   , cnm_grp_0_7_intr_Tile1_Pipe1_Ermrk_Int     , &cnm_grp_0_7_intr}
    ,{   cnm_grp_0_7_intr_Pipe0_L2I_Int       , &cnm_grp_0_7_intr   , cnm_grp_0_7_intr_Tile1_Pipe0_L2I_Int       , &cnm_grp_0_7_intr}
    ,{   cnm_grp_0_7_intr_Pipe1_L2I_Int       , &cnm_grp_0_7_intr   , cnm_grp_0_7_intr_Tile1_Pipe1_L2I_Int       , &cnm_grp_0_7_intr}

    /* must be last */
    ,{  0                                     ,NULL                 , 0                                          ,NULL              }
};


/**
* @internal tile1ToMg0DeepCheckFatherAndUpdate function
* @endinternal
*
* @brief   check if this sub-tree hold node that need to be updated.
*          with father in MG[0]
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void tile1ToMg0DeepCheckFatherAndUpdate(
    IN    SKERNEL_DEVICE_OBJECT         * devObjPtr,
    IN    SKERNEL_INTERRUPT_REG_INFO_STC *currDbPtr,
    IN    FULL_CHIP_TO_MG0_CONNECTIONS    *currInfoPtr
)
{
    GT_U32  offset;

    if(currDbPtr == NULL)
    {
        return;
    }

    if(currDbPtr->myFatherInfo.interruptPtr == currInfoPtr->fatherForTile0InterruptPtr &&
       currDbPtr->myFatherInfo.myBitIndex   == currInfoPtr->bitAtFatherForTile0Interrupt )
    {
        /* we found interrupt of tile 1 in MG0 that we need to update it's connection info */
        /* update pointer ,bit to 'father' */
        currDbPtr->myFatherInfo.interruptPtr = currInfoPtr->fatherForTile1InterruptPtr;
        currDbPtr->myFatherInfo.myBitIndex   = currInfoPtr->bitAtFatherForTile1Interrupt;

        offset = FALCON_TILE_OFFSET_MAC(1);
        currDbPtr->causeReg.registerAddrOffset |= offset;
        currDbPtr->maskReg .registerAddrOffset |= offset;

        return;
    }

    if(isMgIntReg(devObjPtr,currDbPtr->myFatherInfo.interruptPtr))
    {
        /* do not go deeper ... you may find nodes that not relevant for this change */
        return;
    }

    /***********************/
    /* try check my father */
    /***********************/
    tile1ToMg0DeepCheckFatherAndUpdate(devObjPtr,
        currDbPtr->myFatherInfo.interruptPtr,
        currInfoPtr);
}

/**
* @internal setTile1ConnectionsInTile0Mg0 function
* @endinternal
*
* @brief   the tree in tile0 MG0 already hold connections.
*          need to set those needed for tile1 that connected to MG0 of tile0.
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void setTile1ConnectionsInTile0Mg0(
    IN    SKERNEL_DEVICE_OBJECT         * devObjPtr,
    IN    SKERNEL_INTERRUPT_REG_INFO_STC *Tile1ElementsDbPtr,
    IN    GT_U32                        numTile1Elements
)
{
    GT_U32  ii,kk;
    FULL_CHIP_TO_MG0_CONNECTIONS    *currInfoPtr;
    SKERNEL_INTERRUPT_REG_INFO_STC  *currDbPtr;
    GT_U32  kkMax = numTile1Elements;

    currInfoPtr = &fullChipTile1ToTile0Connections[0];

    for(ii = 0 ; currInfoPtr->fatherForTile0InterruptPtr ; ii++,currInfoPtr++)
    {
        currDbPtr = Tile1ElementsDbPtr;

        for(kk = 0; kk < kkMax; kk++ , currDbPtr++)
        {
            tile1ToMg0DeepCheckFatherAndUpdate(devObjPtr,
                currDbPtr,
                currInfoPtr);
        }
    }
}

/**
* @internal smemFalconInterruptTreeInit_perTile function
* @endinternal
*
* @brief   Init the interrupts tree for the Falcon device
*
* @param[in] devObjPtr                - pointer to device object.
*/
static GT_VOID smemFalconInterruptTreeInit_perTile
(
    IN    SKERNEL_DEVICE_OBJECT         * devObjPtr,
    IN      GT_U32                      tileId,
    INOUT    GT_U32                    *indexPtr,
    INOUT SKERNEL_INTERRUPT_REG_INFO_STC ** currDbPtrPtr
)
{
    GT_U32 index = *indexPtr;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr = *currDbPtrPtr;
    GT_U32  mgMax;
    GT_U32  mgIndex;

    {/* MG unit related interrupts */
        if(tileId == 0)
        {
            smemBobcat2GlobalInterruptTreeInit(devObjPtr, &index, &currDbPtr);
        }

        /* for MG0 of the tile */
        mgIndex = 4*tileId;
        mgMax = mgIndex + 4;
        INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &falcon_ports2InterruptsSummary[mgIndex]);

        if((tileId % 2) == 0)
        {
            /* (skip MG[0]) add support for MG1,2,3 */
            mgIndex++;/* 1 */
        }
        else
        {
            /* add support for MG4,5,6,7 */
        }

        for(/* continue*/; mgIndex < mgMax ; mgIndex++)
        {
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &falcon_GlobalInterruptsSummary  [mgIndex]         );
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &falcon_mg_global_misc_interrupts[mgIndex]         );
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &falcon_mg_global_sdma_receiveSDMA_interrupts [mgIndex]);
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &falcon_mg_global_sdma_transmitSDMA_interrupts[mgIndex]);
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
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, rxdma_interrupts                  );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, txdma_interrupts                  );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, txfifo_interrupts                 );
        {/* PHA interrupts */
            INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, PHA_Interrupts                );
            INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, PHA_summary_Interrupts        );
            INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, PPA_summary_Interrupts        );
            INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, PPG_summary_Interrupts        );
        }
    }

    *indexPtr = index;
    *currDbPtrPtr = currDbPtr;
}
/*******************************************************************************
*   smemFalconInterruptTreeInit_tile0
*
* DESCRIPTION:
*       tile 0 : Init the interrupts tree for the Falcon device
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
static GT_VOID smemFalconInterruptTreeInit_tile0
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN    GT_U32 *indexPtr,
    INOUT SKERNEL_INTERRUPT_REG_INFO_STC ** currDbPtrPtr
)
{
    smemFalconInterruptTreeInit_perTile(devObjPtr,0/*tileId*/,indexPtr,currDbPtrPtr);
}

#define MG_REG_OFFSET_IN_DB(regName)     \
    FIELD_OFFSET_IN_STC_MAC(MG.globalInterrupt.regName,SMEM_CHT_PP_REGS_ADDR_STC)

#define SDMA_MG_REG_OFFSET_IN_DB(regName)     \
    FIELD_OFFSET_IN_STC_MAC(MG.SDMARegs.regName,SMEM_CHT_PP_REGS_ADDR_STC)

#define MG_REG_TREE1_OFFSET_IN_DB(regName)     \
    FIELD_OFFSET_IN_STC_MAC(MG.globalInterrupt_tree1.regName,SMEM_CHT_PP_REGS_ADDR_STC)

#define SDMA_MG_TREE1_REG_OFFSET_IN_DB(regName)     \
    FIELD_OFFSET_IN_STC_MAC(MG.SDMARegs.regName##_tree1,SMEM_CHT_PP_REGS_ADDR_STC)

typedef struct {
    GT_U32                              mgRegOffsetInDB;
    SKERNEL_INTERRUPT_REG_INFO_STC      *perMgArrPtr;/* array of MG units for this register */
}CONVERT_INTERRUPT_MG_REGS_STC;

static CONVERT_INTERRUPT_MG_REGS_STC convertInterruptMgRegs[] =
{
    {MG_REG_OFFSET_IN_DB(globalInterruptCause)              , falcon_GlobalInterruptsSummary        }
   ,{MG_REG_OFFSET_IN_DB(functionalInterruptsSummaryCause)  , falcon_FuncUnitsInterruptsSummary     }
   ,{MG_REG_OFFSET_IN_DB(functionalInterruptsSummary1Cause) , falcon_FuncUnitsInterruptsSummary1    }
   ,{MG_REG_OFFSET_IN_DB(SERInterruptsSummary)              , falcon_dataPathInterruptSummary       }
   ,{MG_REG_OFFSET_IN_DB(stackPortsInterruptCause)          , falcon_portsInterruptsSummary         }
   ,{MG_REG_OFFSET_IN_DB(ports1InterruptsSummaryCause)      , falcon_ports1InterruptsSummary         }
   ,{MG_REG_OFFSET_IN_DB(ports2InterruptsSummaryCause)      , falcon_ports2InterruptsSummary        }
   ,{MG_REG_OFFSET_IN_DB(miscellaneousInterruptCause)       , falcon_mg_global_misc_interrupts      }
   ,{SDMA_MG_REG_OFFSET_IN_DB(receiveSDMAInterruptCause)    , falcon_mg_global_sdma_receiveSDMA_interrupts      }
   ,{SDMA_MG_REG_OFFSET_IN_DB(transmitSDMAInterruptCause)   , falcon_mg_global_sdma_transmitSDMA_interrupts     }
   ,{MG_REG_OFFSET_IN_DB(dfxInterruptsSummaryCause)         , falcon_dfxInterruptsSummary         }
   ,{MG_REG_OFFSET_IN_DB(dfx1InterruptsSummaryCause)        , falcon_dfx1InterruptsSummary        }

   /* must be last*/
  ,{SMAIN_NOT_VALID_CNS,NULL}

   /* must be last*/
   ,{SMAIN_NOT_VALID_CNS,NULL}
};

static CONVERT_INTERRUPT_MG_REGS_STC convertInterruptMgRegs_MG8_tree1[] =
{
    {MG_REG_OFFSET_IN_DB(globalInterruptCause)              , falcon_GlobalInterruptsSummary_MG8_tree1        }
   ,{MG_REG_OFFSET_IN_DB(functionalInterruptsSummaryCause)  , falcon_FuncUnitsInterruptsSummary_MG8_tree1     }
   ,{MG_REG_OFFSET_IN_DB(functionalInterruptsSummary1Cause) , falcon_FuncUnitsInterruptsSummary1_MG8_tree1    }
   ,{MG_REG_OFFSET_IN_DB(SERInterruptsSummary)              , falcon_dataPathInterruptSummary_MG8_tree1       }
   ,{MG_REG_OFFSET_IN_DB(stackPortsInterruptCause)          , falcon_portsInterruptsSummary_MG8_tree1         }
   ,{MG_REG_OFFSET_IN_DB(ports1InterruptsSummaryCause)      , falcon_ports1InterruptsSummary_MG8_tree1         }
   ,{MG_REG_OFFSET_IN_DB(ports2InterruptsSummaryCause)      , falcon_ports2InterruptsSummary_MG8_tree1        }
   ,{MG_REG_OFFSET_IN_DB(miscellaneousInterruptCause)       , falcon_mg_global_misc_interrupts_MG8_tree1      }
   ,{SDMA_MG_REG_OFFSET_IN_DB(receiveSDMAInterruptCause)    , falcon_mg_global_sdma_receiveSDMA_interrupts_MG8_tree1      }
   ,{SDMA_MG_REG_OFFSET_IN_DB(transmitSDMAInterruptCause)   , falcon_mg_global_sdma_transmitSDMA_interrupts_MG8_tree1     }
   ,{MG_REG_OFFSET_IN_DB(dfxInterruptsSummaryCause)         , falcon_dfxInterruptsSummary_MG8_tree1         }
   ,{MG_REG_OFFSET_IN_DB(dfx1InterruptsSummaryCause)        , falcon_dfx1InterruptsSummary_MG8_tree1        }

   /* must be last*/
   ,{SMAIN_NOT_VALID_CNS,NULL}
};


/* find MG interrupts node to represent the register in the specific MG unit index */
static  SKERNEL_INTERRUPT_REG_INFO_STC* convertInterruptMgRegsFunc(
    IN GT_U32                  regOffsetInDB,
    IN GT_U32                  mgUnitIndex
)
{
    GT_U32  ii,jj;
    SKERNEL_INTERRUPT_REG_INFO_STC *currMgInfoPtr;
    CONVERT_INTERRUPT_MG_REGS_STC *convertInterruptMgRegsPtr;
    GT_U32 maxIterations;

    if (mgUnitIndex == MG_2_0_INDEX)
    {
        convertInterruptMgRegsPtr = convertInterruptMgRegs_MG8_tree1;
        maxIterations = 1;
    }
    else
    {
        convertInterruptMgRegsPtr = convertInterruptMgRegs;
        maxIterations = NUM_OF_MGS;
    }

    for(ii = 0 ; convertInterruptMgRegsPtr[ii].mgRegOffsetInDB != SMAIN_NOT_VALID_CNS ;ii++)
    {
        if(regOffsetInDB != convertInterruptMgRegsPtr[ii].mgRegOffsetInDB)
        {
            continue;
        }

        currMgInfoPtr = &convertInterruptMgRegsPtr[ii].perMgArrPtr[0];
        for(jj = 0; jj < maxIterations ; jj++ , currMgInfoPtr++)
        {
            if(currMgInfoPtr->causeReg.registerOffsetInDb == 0)
            {
                /* not used entry */
                continue;
            }

            if(GET_MG_INDEX(currMgInfoPtr) == mgUnitIndex)
            {
                return currMgInfoPtr;
            }
        }
    }

    /* the register not MG register (or not need convert) */
    return NULL;
}

static void print_curr(
    IN SKERNEL_DEVICE_OBJECT* devObjPtr ,
    IN SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr ,
    IN int    ii);

static GT_BIT   allowMgChanges = 1;

static GT_U32   getRegOffsetInIntTree(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   tileId,
    IN SKERNEL_INTERRUPT_REG_INFO_STC *currDbPtr,
    OUT SKERNEL_INTERRUPT_REG_INFO_STC **preDefinedInfoPtr
)
{
    GT_U32 offset;
    GT_U32  mgUnitIndex;

    *preDefinedInfoPtr = NULL;

    if(isMgIntReg(devObjPtr,currDbPtr))
    {
        /*******************************************/
        /* MG register that need special treatment */
        /*******************************************/
        if(allowMgChanges)
        {
            mgUnitIndex  = GET_MG_INDEX(currDbPtr);
            if(tileId >= 2)
            {
                mgUnitIndex += MG_2_0_INDEX;
            }
            else
            {
                mgUnitIndex = MG_1_0_INDEX * tileId;
            }

            /* tiles 0,1 CnM at tile 0 */
            /* tile 1 use MG[4] and not MG[0] in the same CnM as tile 0 ! */
            /* tiles 2,3 CnM at tile 2 */
            /* tile 3 use MG[4] and not MG[0] in the same CnM as tile 2 ! */
            *preDefinedInfoPtr = convertInterruptMgRegsFunc(currDbPtr->causeReg.registerOffsetInDb,
                mgUnitIndex);
        }
        else
        {
            /* keep this MG node unmodified */
            *preDefinedInfoPtr = currDbPtr;
        }

        /*************************************************/
        /* the 'preDefinedInfoPtr' not need extra offset */
        /*************************************************/
        offset = 0;
    }
    else
    {
        if(tileId == 3)
        {
            /* it is copy values from tile 1 , so the offset is only of 2 tiles ! */
            offset = FALCON_TILE_OFFSET_MAC(2);
        }
        else
        {
            offset = FALCON_TILE_OFFSET_MAC(tileId);
        }
    }

    return offset;
}


typedef struct{
    GT_U32                          tileId;
    SKERNEL_INTERRUPT_REG_INFO_STC  *alreadyExistsFatherPtr;
    GT_U32                          replaceFatherBit;
    SKERNEL_INTERRUPT_REG_INFO_STC  replaceFatherInfo;
}NEXT_TILE_INFO_STC;
#define MAX_TILE_INFO_CNS (500*4)
static NEXT_TILE_INFO_STC tileInfo[MAX_TILE_INFO_CNS];
/* look for existing entry in tileInfo[] or for new place to set it */
static NEXT_TILE_INFO_STC* findIntFatherNode
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   tileId,
    IN SKERNEL_INTERRUPT_REG_MY_FATHER_INFO *fatherDbPtr
)
{
    GT_U32      ii;
    NEXT_TILE_INFO_STC *tmpPtr = &tileInfo[0];


    if(fatherDbPtr->interruptPtr == NULL)
    {
        return NULL;
    }

    for(ii = 0; ii < MAX_TILE_INFO_CNS; ii++ , tmpPtr++)
    {
        if(tmpPtr->tileId == 0)
        {
            /* empty ... first free ! */
            return &tileInfo[ii];
        }

        if(tileId      == tmpPtr->tileId &&
           fatherDbPtr->interruptPtr == tmpPtr->alreadyExistsFatherPtr &&
           fatherDbPtr->myBitIndex == tmpPtr->replaceFatherBit)
        {
            /* existing entry found */
            return &tileInfo[ii];
        }
    }

    skernelFatalError("findIntFatherNode : Not enough indexes at 'tileInfo' hold [%d] indexes \n",
        MAX_TILE_INFO_CNS);
    return &tileInfo[0];/*dummy for compilation*/
}

/* check if father register already duplicated (with it's sons) */
/* if not ... need duplications */
/* returning pointer to updated father */
static SKERNEL_INTERRUPT_REG_INFO_STC * doRecursiveDupOfFatherInt
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   tileId,
    IN SKERNEL_INTERRUPT_REG_MY_FATHER_INFO *fatherDbPtr
)
{
    NEXT_TILE_INFO_STC                     *dupINFatherGenInfoPtr;
    SKERNEL_INTERRUPT_REG_INFO_STC         *currDbPtr;
    SKERNEL_INTERRUPT_REG_INFO_STC         *updatedMyFatherPtr;
    GT_U32  offset;
    SKERNEL_INTERRUPT_REG_INFO_STC         *specialDbPtr;
    SKERNEL_INTERRUPT_REG_INFO_STC*        fatherInterruptPtr;

    if(fatherDbPtr->interruptPtr &&
       isMgIntReg(devObjPtr,fatherDbPtr->interruptPtr) &&
       (allowMgChanges == 0 ||
       (MG_2_0_INDEX <= GET_MG_INDEX(fatherDbPtr->interruptPtr))))
    {
        /* MG duplications NOT needed */
        return fatherDbPtr->interruptPtr;
    }


    dupINFatherGenInfoPtr = findIntFatherNode(devObjPtr,tileId,fatherDbPtr);
    if(dupINFatherGenInfoPtr == NULL)
    {
        /* no duplication needed */
        return NULL;
    }
    else
    if(dupINFatherGenInfoPtr->tileId != 0)
    {
        /* no more to do ... already duplicated */
        return &dupINFatherGenInfoPtr->replaceFatherInfo;
    }

    if(fatherDbPtr->interruptPtr == NULL)
    {
        /* should not get here ... dupINFatherGenInfoPtr == NULL */
        skernelFatalError("doRecursiveDupOfFatherInt: NULL pointer");
        return &dupINFatherGenInfoPtr->replaceFatherInfo;
    }

    fatherInterruptPtr = fatherDbPtr->interruptPtr;
    if (fatherDbPtr->interruptFatherReplace.interruptPtr != NULL)
    {
        if (tileId % 2)
        {
            /* Tile 1 and 3 father replacement */
            fatherDbPtr->myBitIndex = (fatherDbPtr->interruptFatherReplace).myBitIndex;
            fatherInterruptPtr = (fatherDbPtr->interruptFatherReplace).interruptPtr;
        }
    }
    /* we got empty space , that requires duplication */
    /* copy the info from the father */
    dupINFatherGenInfoPtr->replaceFatherInfo = *fatherInterruptPtr;
    /* state that now the 'father' is known */
    dupINFatherGenInfoPtr->tileId           = tileId;
    dupINFatherGenInfoPtr->alreadyExistsFatherPtr = fatherInterruptPtr;
    dupINFatherGenInfoPtr->replaceFatherBit = fatherDbPtr->myBitIndex;

    /* update the duplicated father with needed info */
    currDbPtr = &dupINFatherGenInfoPtr->replaceFatherInfo;

    if(currDbPtr->causeReg.registerOffsetInDb == 0 &&
       currDbPtr->maskReg .registerOffsetInDb == 0)
    {
        /* info not valid ! */
        offset = 0;
    }
    else
    {
        offset = getRegOffsetInIntTree(devObjPtr,tileId,currDbPtr,&specialDbPtr);
        if(specialDbPtr == currDbPtr)
        {
            /* no need to copy/modify ... same pointer ! */
        }
        else
        if(specialDbPtr)
        {
            /* need to update the 'current' */
            *currDbPtr = *specialDbPtr;
        }
        else
        {
            /**************************************/
            /* recursive call to current function */
            /**************************************/
            updatedMyFatherPtr = doRecursiveDupOfFatherInt(devObjPtr,tileId,&currDbPtr->myFatherInfo);
            currDbPtr->myFatherInfo.interruptPtr = updatedMyFatherPtr;

            if(currDbPtr->isSecondFatherExists)
            {
                /***********************************/
                /* treat also the secondary father */
                /***********************************/

                /**************************************/
                /* recursive call to current function */
                /**************************************/
                updatedMyFatherPtr = doRecursiveDupOfFatherInt(devObjPtr,tileId,&currDbPtr->myFatherInfo_2);
                currDbPtr->myFatherInfo_2.interruptPtr = updatedMyFatherPtr;
            }
        }
    }

    /* at this point my father (and second father) are updated */
    /* need to update 'self' */

    currDbPtr->causeReg.registerAddrOffset += offset;
    currDbPtr->maskReg .registerAddrOffset += offset;

    return currDbPtr;
}



/*******************************************************************************
*   smemFalconInterruptTreeUpdateForTile
*
* DESCRIPTION:
*       update the info from tile 0 with info of this tile
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       tileId      - the tile Id of the updated interrupts tree addresses
*       lastIndex   - last index in the tree (array)
*       startOfDbPtr   - pointer to start of the tree (array)
* OUTPUTS:
*       None.
*
* RETURNS:
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_VOID smemFalconInterruptTreeUpdateForTile
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN    GT_U32  tileId,
    IN    GT_U32  lastIndex,
    INOUT SKERNEL_INTERRUPT_REG_INFO_STC *startOfDbPtr
)
{
    GT_U32 ii;
    SKERNEL_INTERRUPT_REG_INFO_STC *currDbPtr = startOfDbPtr;
    SKERNEL_INTERRUPT_REG_INFO_STC *fatherInfoPtr;
    SKERNEL_INTERRUPT_REG_INFO_STC *specialDbPtr;
    GT_U32 offset;

    for(ii = 0 ; ii < lastIndex ; ii++, currDbPtr++)
    {
        offset = getRegOffsetInIntTree(devObjPtr,tileId,currDbPtr,&specialDbPtr);
        if(specialDbPtr == currDbPtr)
        {
            /* no need to copy/modify ... same pointer ! */
            continue;
        }
        else
        if(specialDbPtr)
        {
            /* need to update the 'current' */
            *currDbPtr = *specialDbPtr;
        }

        currDbPtr->causeReg.registerAddrOffset += offset;
        currDbPtr->maskReg .registerAddrOffset += offset;

        /* we got a leaf node ... need to see if it's fathers exist in current tile */
        fatherInfoPtr  = doRecursiveDupOfFatherInt(devObjPtr,tileId,&currDbPtr->myFatherInfo);
        currDbPtr->myFatherInfo.interruptPtr = fatherInfoPtr;

        if(currDbPtr->isSecondFatherExists)
        {
            /* treat also the secondary father */
            fatherInfoPtr = doRecursiveDupOfFatherInt(devObjPtr,tileId,&currDbPtr->myFatherInfo_2);
            currDbPtr->myFatherInfo_2.interruptPtr = fatherInfoPtr;
        }
    }

    return;
}

/*******************************************************************************
*   smemFalconInterruptTreeUpdateForTile_noMgChanges
*
* DESCRIPTION:
*       update the info from tile 0 with info of this tile
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       tileId      - the tile Id of the updated interrupts tree addresses
*       lastIndex   - last index in the tree (array)
*       startOfDbPtr   - pointer to start of the tree (array)
* OUTPUTS:
*       None.
*
* RETURNS:
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_VOID smemFalconInterruptTreeUpdateForTile_noMgChanges
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN    GT_U32  tileId,
    IN    GT_U32  lastIndex,
    INOUT SKERNEL_INTERRUPT_REG_INFO_STC *startOfDbPtr
)
{
    allowMgChanges = 0;
    smemFalconInterruptTreeUpdateForTile(devObjPtr,tileId,lastIndex,startOfDbPtr);
    allowMgChanges = 1;
}

/*******************************************************************************
*   smemFalconInterruptTreeInit_tile1
*
* DESCRIPTION:
*       tile 1 : Init the interrupts tree for the Falcon device
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
static GT_VOID smemFalconInterruptTreeInit_tile1
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN    GT_U32 *indexPtr,
    INOUT SKERNEL_INTERRUPT_REG_INFO_STC ** currDbPtrPtr
)
{
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr_endOfTile0 = *currDbPtrPtr;
    GT_U32  index_endOfTile0 = *indexPtr;
    GT_U32  tileId = 1;
    GT_U32  numNewElements;

    /*****************************/
    /* add the tile 1 interrupts */
    /*****************************/
    smemFalconInterruptTreeInit_perTile(devObjPtr,tileId,indexPtr,currDbPtrPtr);

    numNewElements = *indexPtr - index_endOfTile0;
    /*********************************************************/
    /* update the info of tile 1 with info proper for tile 1 */
    /*********************************************************/
    currDbPtr = currDbPtr_endOfTile0;/* was saved from input param (end of tile 0) */
    smemFalconInterruptTreeUpdateForTile_noMgChanges(devObjPtr,tileId,numNewElements,currDbPtr);

    numNewElements = *indexPtr - index_endOfTile0;

    /* the tree in tile0 MG0 already hold connections.
       need to set those needed for tile1 that connected to MG0 of tile0.

      No new elements ... only update the connections that relate tile 1 into MG[0]
    */
    setTile1ConnectionsInTile0Mg0(devObjPtr,currDbPtr_endOfTile0,numNewElements);

}

/*******************************************************************************
*   smemFalconInterruptTreeInit_tile2_3
*
* DESCRIPTION:
*       tiles 2,3 : Init the interrupts tree for the Falcon device
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
static GT_VOID smemFalconInterruptTreeInit_tile2_3
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN    GT_U32 *indexPtr,
    INOUT SKERNEL_INTERRUPT_REG_INFO_STC ** currDbPtrPtr,
    IN    GT_U32    numIntTile0,
    IN    GT_U32    numIntTile1
)
{
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr_endOfTile1 = *currDbPtrPtr;
    GT_U32  index_endOfTile1 = *indexPtr;
    GT_U32  tileId;
    GT_U32  numInt,totalNumInt=0;

    /**************************************/
    /* duplicate the info from tile 0,1 ! */
    /**************************************/
    currDbPtr = falconInterruptsTreeDb_perTile;
    smemBobcat2InterruptTreeAddNode(&currDbPtr, devObjPtr->myInterruptsDbPtr, index_endOfTile1);

    /*********************************************************/
    /* update the info from tiles 0,1 with info of tiles 2,3 */
    /*********************************************************/
    currDbPtr = falconInterruptsTreeDb_perTile;
    for(tileId = 2 ; tileId < 4; tileId++)
    {
        numInt =  tileId == 2 ? numIntTile0 : numIntTile1;
        smemFalconInterruptTreeUpdateForTile(devObjPtr,tileId,numInt,currDbPtr);
        currDbPtr   += numInt;
        totalNumInt += numInt;

        if(MAX_INTERRUPT_NODES <= totalNumInt)
        {
            skernelFatalError("(tile[%d])Not enough indexes at 'falconInterruptsTreeDb' hold[%d] but need at least[%d]",
                tileId,
                MAX_INTERRUPT_NODES,
                (totalNumInt + 1));
            return;
        }
    }

    /**********************************************************/
    /* add the updated info from tiles 2,3 to the device info */
    /**********************************************************/
    currDbPtr = currDbPtr_endOfTile1;
    smemBobcat2InterruptTreeAddNode(&currDbPtr, falconInterruptsTreeDb_perTile, index_endOfTile1);

    * indexPtr     += totalNumInt;
    * currDbPtrPtr = currDbPtr;
}

/*******************************************************************************
*   smemFalconInterruptTreeInit
*
* DESCRIPTION:
*       Init the interrupts tree for the Falcon device
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
GT_VOID smemFalconInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 index = 0;
    GT_U32 numIntTile0,numIntTile1;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr;
    GT_U32  numOfRavens;

    /* add here additions to 'compilation' info , that since 2 arrays point to
       each other we need to set it in runtime */

    /* Initialize Ports2 father Summary interrupts for primary MG units in all tiles */
    falcon_ports2InterruptsSummary[MG_0_0_INDEX].myFatherInfo.interruptPtr = &falcon_GlobalInterruptsSummary[MG_0_0_INDEX];
    falcon_ports2InterruptsSummary[MG_1_0_INDEX].myFatherInfo.interruptPtr = &falcon_GlobalInterruptsSummary[MG_1_0_INDEX];
    falcon_ports2InterruptsSummary[MG_2_0_INDEX].myFatherInfo.interruptPtr = &falcon_GlobalInterruptsSummary[MG_2_0_INDEX];
    falcon_ports2InterruptsSummary[MG_3_0_INDEX].myFatherInfo.interruptPtr = &falcon_GlobalInterruptsSummary[MG_3_0_INDEX];
    /* Initialize global of MG[8] to connect to 'falcon_FuncUnitsInterruptsSummary' of MG[0] */
    falcon_GlobalInterruptsSummary[MG_2_0_INDEX].myFatherInfo.interruptPtr = &falcon_FuncUnitsInterruptsSummary[MG_0_0_INDEX];

    devObjPtr->myInterruptsDbPtr = falconInterruptsTreeDb;

    currDbPtr = devObjPtr->myInterruptsDbPtr;

    /***********************************/
    /* start with interrupts of tile 0 */
    /***********************************/
    smemFalconInterruptTreeInit_tile0(devObjPtr,&index,&currDbPtr);

    if(devObjPtr->numOfTiles > 1)
    {
        numIntTile0 = index;
        /************************/
        /* interrupts of tile 1 */
        /************************/
        smemFalconInterruptTreeInit_tile1(devObjPtr,&index,&currDbPtr);
        numIntTile1 = index - numIntTile0;

        if(devObjPtr->numOfTiles > 2)
        {

            /* bind 'tree0' of MG9,10,11,12 to 'tree1' of MG8 */
            falcon_GlobalInterruptsSummary[MG_2_1_INDEX].myFatherInfo.interruptPtr = &falcon_ports2InterruptsSummary_MG8_tree1[0];
            falcon_GlobalInterruptsSummary[MG_2_2_INDEX].myFatherInfo.interruptPtr = &falcon_ports2InterruptsSummary_MG8_tree1[0];
            falcon_GlobalInterruptsSummary[MG_2_3_INDEX].myFatherInfo.interruptPtr = &falcon_ports2InterruptsSummary_MG8_tree1[0];
            falcon_GlobalInterruptsSummary[MG_3_0_INDEX].myFatherInfo.interruptPtr = &falcon_ports2InterruptsSummary_MG8_tree1[0];

            /* Initialize Ravens father Summary interrupts for MG units in tiles 2,3  */
            raven_globalInterruptsSummary[8].myFatherInfo.interruptPtr = &falcon_FuncUnitsInterruptsSummary_MG8_tree1[0];
            raven_globalInterruptsSummary[9].myFatherInfo.interruptPtr = &falcon_FuncUnitsInterruptsSummary_MG8_tree1[0];
            raven_globalInterruptsSummary[10].myFatherInfo.interruptPtr = &falcon_FuncUnitsInterruptsSummary_MG8_tree1[0];
            raven_globalInterruptsSummary[11].myFatherInfo.interruptPtr = &falcon_FuncUnitsInterruptsSummary_MG8_tree1[0];
            raven_globalInterruptsSummary[12].myFatherInfo.interruptPtr = &falcon_FuncUnitsInterruptsSummary_MG8_tree1[0];
            raven_globalInterruptsSummary[13].myFatherInfo.interruptPtr = &falcon_FuncUnitsInterruptsSummary_MG8_tree1[0];
            raven_globalInterruptsSummary[14].myFatherInfo.interruptPtr = &falcon_FuncUnitsInterruptsSummary_MG8_tree1[0];
            raven_globalInterruptsSummary[15].myFatherInfo.interruptPtr = &falcon_FuncUnitsInterruptsSummary_MG8_tree1[0];

            /***************************/
            /* interrupts of tiles 2,3 */
            /***************************/
            smemFalconInterruptTreeInit_tile2_3(devObjPtr,&index,&currDbPtr,numIntTile0,numIntTile1);
        }
    }

    /* add all ravens at once at the end */
    {/* Raven interrupts */
        numOfRavens =  4*devObjPtr->numOfTiles;

        INTERRUPT_TREE_ADD_X_NODES_WITH_CHECK(index, &currDbPtr, raven_portsInterruptsSummary   , (numOfRavens * 16));
        INTERRUPT_TREE_ADD_X_NODES_WITH_CHECK(index, &currDbPtr, raven_ports1InterruptsSummary, numOfRavens);
        INTERRUPT_TREE_ADD_X_NODES_WITH_CHECK(index, &currDbPtr, raven_globalInterruptsSummary  , numOfRavens  );
        INTERRUPT_TREE_ADD_X_NODES_WITH_CHECK(index, &currDbPtr, mti_mac_ports_interrupts       , (numOfRavens * (16 /*MTI64*/ + 4 /*MTI400*/ + 2 /* CPU port*/ + 4 /* FEC */)));
        INTERRUPT_TREE_ADD_X_NODES_WITH_CHECK(index, &currDbPtr, raven_LMU_causeInterruptsSummary, numOfRavens * 2/*LMU units*/ * 17/*LMU latency Thresholds*/);
        INTERRUPT_TREE_ADD_X_NODES_WITH_CHECK(index, &currDbPtr, lmu_latency_over_threshold_interrupts, numOfRavens * 2/*LMU units*/ * 17/*LMU latency Thresholds*/);
    }

    /* End of interrupts - must be last */
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, skernel_interrupt_reg_info__last_line__cns);
}

void print_falconInterruptsTreeDb_mgInterrupts(void)
{
    GT_U32  ii=0;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr = falconInterruptsTreeDb;
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

void print_falconInterruptsTreeDb_all_Interrupts(void)
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

void print_falconInterruptsTreeDb_childs_Interrupts(void)
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


