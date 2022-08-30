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
* @file smemAldrin2.c
*
* @brief Aldrin2 memory mapping implementation
*
* @version   1
********************************************************************************
*/

#include <asicSimulation/SKernel/smem/smemAldrin2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregLion2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregBobcat2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>

/* GOP :
(CPSS) global |  GOP MG  |  local GOP    |      | DP local
  port #      | chain    | (in MG chain) | DP#  | port#
  ==========================================================
     0..11    |    0     |    0..11      |  0   |  0..11
    12..23    |    0     |   12..23      |  1   |  0..11
    24..47    |    1     |    0..23      |  2   |  0..23
    48..71    |    1     |   24..47      |  3   |  0..23
        72    |    1     |       63      |  0   |  24
*/
/* Chain1 GOP offset */
#define CHAIN_1_GOP_OFFSET        0x00400000
/* Chain1 MIB offset */
#define CHAIN_1_MIB_OFFSET_GIG    0x00800000
/* Chain1 serdes offset */
#define CHAIN_1_SERDES_OFFSET     0x00080000

#define GOP_CHAIN_0_NUM_PORTS_CNS   24
#define GOP_CHAIN_1_NUM_PORTS_CNS   48
/* port 24 in DP0 (global 72) is 'CPU PORT' (XG port) that is on 'MG chain 1' port 63 */
#define CPU_PORT_CNS                72
#define GOP_CHAIN_1_CPU_PORT_CNS    63

/* Chain1 cpu port GOP offset */
#define CPU_PORT_CHAIN_1_GOP_OFFSET        (CHAIN_1_GOP_OFFSET    + (GOP_CHAIN_1_CPU_PORT_CNS * 0x1000))
/* Chain1 cpu port serdes offset */
#define CPU_PORT_CHAIN_1_SERDES_OFFSET     (CHAIN_1_SERDES_OFFSET + (GOP_CHAIN_1_CPU_PORT_CNS * 0x1000))
/* Chain1 cpu port MIB offset */
#define CPU_PORT_CHAIN_1_MIB_OFFSET        (CHAIN_1_MIB_OFFSET_GIG + (GOP_CHAIN_1_CPU_PORT_CNS * BOBCAT2_MIB_OFFSET_CNS))


/* memory space : GOP for MG CHAIN 0 ports */
#define FORMULA_SINGLE_PARAMETER_GOP_DUP_MG_CHAIN_0_PORTS_CNS \
    FORMULA_SINGLE_PARAMETER(GOP_CHAIN_0_NUM_PORTS_CNS , 0x1000)

/* memory space : GOP for MG CHAIN 1 ports */
#define FORMULA_SINGLE_PARAMETER_GOP_DUP_MG_CHAIN_1_PORTS_CNS \
    FORMULA_SINGLE_PARAMETER(GOP_CHAIN_1_NUM_PORTS_CNS , 0x1000)

/* the XPCS hold registers one for every 2 ports */
#define FORMULA_SINGLE_PARAMETER_GOP_XPCS_DUP_MG_CHAIN_0_PORTS_CNS \
    FORMULA_SINGLE_PARAMETER(GOP_CHAIN_0_NUM_PORTS_CNS/2 , 0x1000*2)
/* the XPCS hold registers one for every 2 ports */
#define FORMULA_SINGLE_PARAMETER_GOP_XPCS_DUP_MG_CHAIN_1_PORTS_CNS \
    FORMULA_SINGLE_PARAMETER(GOP_CHAIN_1_NUM_PORTS_CNS/2 , 0x1000*2)

/* per lane : the XPCS hold registers one for every 2 ports */
#define FORMULA_TWO_PARAMETERS_GOP_XPCS_LANE_DUP_MG_CHAIN_0_PORTS_CNS \
    FORMULA_TWO_PARAMETERS(6 , 0x44  , GOP_CHAIN_0_NUM_PORTS_CNS/2  , 0x1000*2)

/* per lane : the XPCS hold registers one for every 2 ports */
#define FORMULA_TWO_PARAMETERS_GOP_XPCS_LANE_DUP_MG_CHAIN_1_PORTS_CNS \
    FORMULA_TWO_PARAMETERS(6 , 0x44  , GOP_CHAIN_1_NUM_PORTS_CNS/2  , 0x1000*2)

/* the CG hold registers one for every 4 ports */
#define FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS \
    FORMULA_SINGLE_PARAMETER(GOP_CHAIN_0_NUM_PORTS_CNS/4 , 0x1000*4)

/* default registers : GOP for MG CHAIN 0 ports */
#define DEFAULT_REG_GOP_DUP_MG_CHAIN_0_AND_PORTS_CNS \
    GOP_CHAIN_0_NUM_PORTS_CNS , 0x1000

/* default registers : GOP for MG CHAIN 1 ports */
#define DEFAULT_REG_GOP_DUP_MG_CHAIN_1_AND_PORTS_CNS \
    GOP_CHAIN_1_NUM_PORTS_CNS , 0x1000

/* default registers : GOP CG for MG CHAIN 0 ports */
#define DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS \
    GOP_CHAIN_0_NUM_PORTS_CNS/4 , 0x1000*4

/* SO the SERDESes are like GOP and MIBs */
#define SERDES_CHAIN_0_NUM_PORTS_CNS   GOP_CHAIN_0_NUM_PORTS_CNS
#define SERDES_CHAIN_1_NUM_PORTS_CNS   GOP_CHAIN_1_NUM_PORTS_CNS

/* memory space : SERDES for MG CHAIN 0 ports */
#define FORMULA_SINGLE_PARAMETER_SERDES_DUP_MG_CHAIN_0_PORTS_CNS \
    FORMULA_SINGLE_PARAMETER(SERDES_CHAIN_0_NUM_PORTS_CNS , 0x1000)

/* memory space : SERDES for MG CHAIN 1 ports */
#define FORMULA_SINGLE_PARAMETER_SERDES_DUP_MG_CHAIN_1_PORTS_CNS \
    FORMULA_SINGLE_PARAMETER(SERDES_CHAIN_1_NUM_PORTS_CNS , 0x1000)

/* default registers : SERDES for MG CHAIN 0 ports */
#define DEFAULT_REG_SERDESA_DUP_MG_CHAIN_0_AND_PORTS_CNS \
    SERDES_CHAIN_0_NUM_PORTS_CNS , 0x1000

/* default registers : SERDES for MG CHAIN 1 ports */
#define DEFAULT_REG_SERDESA_DUP_MG_CHAIN_1_AND_PORTS_CNS \
    SERDES_CHAIN_1_NUM_PORTS_CNS , 0x1000


/* memory space : MIB for MG CHAIN 0 ports */
#define FORMULA_SINGLE_PARAMETER_MIB_DUP_MG_CHAIN_0_PORTS_CNS \
    FORMULA_SINGLE_PARAMETER(GOP_CHAIN_0_NUM_PORTS_CNS , BOBCAT2_MIB_OFFSET_CNS)

/* memory space : MIB for MG CHAIN 1 ports */
#define FORMULA_SINGLE_PARAMETER_MIB_DUP_MG_CHAIN_1_PORTS_CNS \
    FORMULA_SINGLE_PARAMETER(GOP_CHAIN_1_NUM_PORTS_CNS , BOBCAT2_MIB_OFFSET_CNS)

#define GOP_PER_63_PORTS_MASK_CNS   0xFFFC0FFF


#define GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(a,b) \
    /*chain 0 ports : 0..23 */ \
 {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (                             a ,                             b)} , FORMULA_SINGLE_PARAMETER_GOP_DUP_MG_CHAIN_0_PORTS_CNS} \
    /*chain 1 ports : 0..47 */ \
,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (CHAIN_1_GOP_OFFSET          +a, CHAIN_1_GOP_OFFSET          +b)} , FORMULA_SINGLE_PARAMETER_GOP_DUP_MG_CHAIN_1_PORTS_CNS} \
    /*chain 1 port : 63 */     \
,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (CPU_PORT_CHAIN_1_GOP_OFFSET +a, CPU_PORT_CHAIN_1_GOP_OFFSET +b)}}

#define GOP____STEP_2_PORTS_SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(a,b) \
    /*chain 0 ports : 0..23 */ \
     {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (                             a,                              b)}, FORMULA_SINGLE_PARAMETER_GOP_XPCS_DUP_MG_CHAIN_0_PORTS_CNS} \
    /*chain 1 ports : 0..47 */ \
    ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CHAIN_1_GOP_OFFSET          +a, CHAIN_1_GOP_OFFSET          +b)}, FORMULA_SINGLE_PARAMETER_GOP_XPCS_DUP_MG_CHAIN_1_PORTS_CNS} \
    /*chain 1 port : 63 */     \
    ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PORT_CHAIN_1_GOP_OFFSET +a, CPU_PORT_CHAIN_1_GOP_OFFSET +b)}}

#define GOP____6_LANES_STEP_2_PORTS_SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(a,b) \
    /*chain 0 ports : 0..23 */ \
     {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (                             a, b)}, FORMULA_TWO_PARAMETERS_GOP_XPCS_LANE_DUP_MG_CHAIN_0_PORTS_CNS} \
    /*chain 1 ports : 0..47 */ \
    ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (CHAIN_1_GOP_OFFSET          +a, b)}, FORMULA_TWO_PARAMETERS_GOP_XPCS_LANE_DUP_MG_CHAIN_1_PORTS_CNS} \
    /*chain 1 port : 63 */     \
    ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (CPU_PORT_CHAIN_1_GOP_OFFSET +a, b)}, FORMULA_SINGLE_PARAMETER(6 , 0x44)}


#define GOP____DEFAULT_REG_MAC(a,b) \
    /*chain 0 ports : 0..23 */ \
     {DUMMY_NAME_PTR_CNS,                              a, b, DEFAULT_REG_GOP_DUP_MG_CHAIN_0_AND_PORTS_CNS } \
    /*chain 1 ports : 0..47 */ \
    ,{DUMMY_NAME_PTR_CNS, CHAIN_1_GOP_OFFSET          +a, b, DEFAULT_REG_GOP_DUP_MG_CHAIN_1_AND_PORTS_CNS } \
    /*chain 1 port : 63 */     \
    ,{DUMMY_NAME_PTR_CNS, CPU_PORT_CHAIN_1_GOP_OFFSET +a, b}

#define GOP____2_PARAM_DEFAULT_REG_MAC(a,b,c,d) \
    /*chain 0 ports : 0..23 */ \
     {DUMMY_NAME_PTR_CNS,                              a,         b,     DEFAULT_REG_GOP_DUP_MG_CHAIN_0_AND_PORTS_CNS      ,c,    d     } \
    /*chain 1 ports : 0..47 */ \
    ,{DUMMY_NAME_PTR_CNS, CHAIN_1_GOP_OFFSET          +a,         b,     DEFAULT_REG_GOP_DUP_MG_CHAIN_1_AND_PORTS_CNS      ,c,    d     } \
    /*chain 1 port : 63 */     \
    ,{DUMMY_NAME_PTR_CNS, CPU_PORT_CHAIN_1_GOP_OFFSET +a,         b,                                                        c,    d     }



/* Aldrin2 interrupt tree database */
static SKERNEL_INTERRUPT_REG_INFO_STC aldrin2InterruptsTreeDb[MAX_INTERRUPT_NODES];

/* there are no more than 88 units */
#define ALDRIN2_NUM_UNITS_E     88

/* not used memory */
#define DUMMY_UNITS_BASE_ADDR_CNS(index)              0x70000000 + UNIT_BASE_ADDR_MAC(2*index)

typedef struct{
    GT_U32      base_addr;/* base address of unit*/
    GT_CHAR*    nameStr  ;/* name of unit */
    GT_U32      size;/* number of units (each unit is (1<<24) bytes size) */
    GT_CHAR*    orig_nameStr  ;/* when orig_nameStr is not NULL than the nameStr
                                  is name of the duplicated unit , and the
                                  orig_nameStr is original unit.

                                  */
}UNIT_INFO_STC;

#define UNIT_INFO_MAC(baseAddr,unitName) \
    UNIT_INFO_LARGE_MAC(baseAddr,unitName,1)

#define UNIT_INFO_LARGE_MAC(baseAddr,unitName,size) \
     {baseAddr , STR(unitName) , size}

#define INIT_LPM_UNIT_MULTI_INSTANCE_TABLES_MAC(tableName)\
    devObjPtr->tablesInfo.tableName.commonInfo.multiInstanceInfo.numBaseAddresses = 2;\
    devObjPtr->tablesInfo.tableName.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.lpm[0];

/* use 2 times the 'ALDRIN2_NUM_UNITS_E' because:
   2 time to support that each unit is '8 MSbits' and not '9 MSbits'
*/
static SMEM_UNIT_NAME_AND_INDEX_STC aldrin2UnitNameAndIndexArr[(2*ALDRIN2_NUM_UNITS_E)+1]=
{
    /* filled in runtime from aldrin2units[] */
    /* must be last */
    {NULL ,                                SMAIN_NOT_VALID_CNS                     }
};


/* the addresses of the units that the bobk uses */
static SMEM_UNIT_BASE_AND_SIZE_STC   aldrin2UsedUnitsAddressesArray[(2*ALDRIN2_NUM_UNITS_E)+1]=
{
    {0,0}    /* filled in runtime from aldrin2units[] */
};

/* the units of Armstron2 */
static UNIT_INFO_STC aldrin2units[(2*ALDRIN2_NUM_UNITS_E)+1] =
{
     UNIT_INFO_MAC(0x00000000,UNIT_MG                )
    ,UNIT_INFO_MAC(0x01000000,UNIT_TTI               )
    ,UNIT_INFO_MAC(0x02000000,UNIT_IPCL              )
    ,UNIT_INFO_MAC(0x03000000,UNIT_L2I               )
    ,UNIT_INFO_MAC(0x04000000,UNIT_IPVX              )
    ,UNIT_INFO_MAC(0x05000000,UNIT_IPLR              )
    ,UNIT_INFO_MAC(0x06000000,UNIT_IPLR1             )
    ,UNIT_INFO_MAC(0x07000000,UNIT_IOAM              )
    ,UNIT_INFO_MAC(0x08000000,UNIT_MLL               )
    ,UNIT_INFO_MAC(0x09000000,UNIT_EQ                )
    ,UNIT_INFO_MAC(0x0a000000,UNIT_EGF_EFT           )

    ,UNIT_INFO_MAC(0x0b000000,UNIT_TXQ_DQ            )
    ,UNIT_INFO_MAC(0x0c000000,UNIT_TXQ_DQ_1          )
    ,UNIT_INFO_MAC(0x0d000000,UNIT_TXQ_DQ_2          )

    ,UNIT_INFO_MAC(0x0e000000,UNIT_CNC               )
    ,UNIT_INFO_MAC(0x0f000000,UNIT_CNC_1             )

    ,UNIT_INFO_MAC(0x10000000,UNIT_GOP               )
    ,UNIT_INFO_MAC(0x12000000,UNIT_XG_PORT_MIB       )
    ,UNIT_INFO_MAC(0x13000000,UNIT_SERDES            )
    ,UNIT_INFO_MAC(0x14000000,UNIT_HA                )
    ,UNIT_INFO_MAC(0x15000000,UNIT_ERMRK             )
    ,UNIT_INFO_MAC(0x16000000,UNIT_EPCL              )
    ,UNIT_INFO_MAC(0x17000000,UNIT_EPLR              )
    ,UNIT_INFO_MAC(0x18000000,UNIT_EOAM              )

    ,UNIT_INFO_MAC(0x19000000,UNIT_RX_DMA            )
    ,UNIT_INFO_MAC(0x1a000000,UNIT_RX_DMA_1          )
    ,UNIT_INFO_MAC(0x1b000000,UNIT_RX_DMA_2          )
    ,UNIT_INFO_MAC(0x1c000000,UNIT_TX_DMA            )
    ,UNIT_INFO_MAC(0x1d000000,UNIT_TX_DMA_1          )
    ,UNIT_INFO_MAC(0x1e000000,UNIT_TX_DMA_2          )
    ,UNIT_INFO_MAC(0x1f000000,UNIT_TX_FIFO           )
    ,UNIT_INFO_MAC(0x20000000,UNIT_TX_FIFO_1         )
    ,UNIT_INFO_MAC(0x21000000,UNIT_TX_FIFO_2         )
    ,UNIT_INFO_MAC(0x22000000,UNIT_IA                )/*Single IA connected to all 4 DP cores.*/
    ,UNIT_INFO_MAC(0x23000000,UNIT_FCU               )
    ,UNIT_INFO_MAC(0x24000000,UNIT_SBC               )
    ,UNIT_INFO_MAC(0x25000000,UNIT_NSEC              )
    ,UNIT_INFO_MAC(0x26000000,UNIT_NSEF              )
    ,UNIT_INFO_MAC(0x27000000,UNIT_GOP_LED_0         )
    ,UNIT_INFO_MAC(0x28000000,UNIT_GOP_LED_1         )
    ,UNIT_INFO_MAC(0x29000000,UNIT_GOP_SMI_0         )
    ,UNIT_INFO_MAC(0x2a000000,UNIT_GOP_SMI_1         )
    ,UNIT_INFO_MAC(0x57000000,UNIT_TAI               )
    ,UNIT_INFO_MAC(0x58000000,UNIT_TAI_1             )

    ,UNIT_INFO_LARGE_MAC(0x30000000,UNIT_EGF_SHT    , 8)

    ,UNIT_INFO_MAC(0x40000000,UNIT_BM                )
    ,UNIT_INFO_MAC(0x41000000,UNIT_BMA               )
    ,UNIT_INFO_MAC(0x42000000,UNIT_CPFC              )
    ,UNIT_INFO_MAC(0x43000000,UNIT_FDB               )
    ,UNIT_INFO_MAC(0x44000000,UNIT_LPM               )
    ,UNIT_INFO_MAC(0x45000000,UNIT_LPM_1             )
    ,UNIT_INFO_MAC(0x50000000,UNIT_EGF_QAG           )
    ,UNIT_INFO_MAC(0x51000000,UNIT_MPPM              )
    ,UNIT_INFO_MAC(0x52000000,UNIT_TCAM              )
    ,UNIT_INFO_MAC(0x53000000,UNIT_TXQ_LL            )
    ,UNIT_INFO_MAC(0x54000000,UNIT_TXQ_QCN           )
    ,UNIT_INFO_MAC(0x55000000,UNIT_TXQ_QUEUE         )
    ,UNIT_INFO_MAC(0x56000000,UNIT_TXQ_BMX           )

    ,UNIT_INFO_MAC(DUMMY_UNITS_BASE_ADDR_CNS(0), UNIT_FDB_TABLE_0 )

    /*********************/
    /* units that were in PIPE 1 in Bobcat3 ... keep their UNIT Address space */
    /*********************/
    ,UNIT_INFO_MAC(0x8b000000,UNIT_TXQ_DQ_3           )  /* in BC3 this is UNIT_TXQ_DQ in pipe 1 */

    ,UNIT_INFO_MAC(0x99000000,UNIT_RX_DMA_3           )  /* in BC3 this is UNIT_RX_DMA in pipe 1 */
    ,UNIT_INFO_MAC(0x9c000000,UNIT_TX_DMA_3           )  /* in BC3 this is UNIT_TXQ_DQ in pipe 1 */
    ,UNIT_INFO_MAC(0x9f000000,UNIT_TX_FIFO_3          )  /* in BC3 this is UNIT_TX_FIFO in pipe 1 */

    ,UNIT_INFO_MAC(0xa4000000,UNIT_SBC_1              )  /* in BC3 this is UNIT_SBC in pipe 1 */

    ,UNIT_INFO_MAC(0xa7000000,UNIT_GOP_LED_2          )  /* in BC3 this is UNIT_GOP_LED_0 in pipe 1 */
    ,UNIT_INFO_MAC(0xa8000000,UNIT_GOP_LED_3          )  /* in BC3 this is UNIT_GOP_LED_1 in pipe 1 */
    ,UNIT_INFO_MAC(0xa9000000,UNIT_GOP_SMI_2          )  /* in BC3 this is UNIT_GOP_SMI_0 in pipe 1 */
    ,UNIT_INFO_MAC(0xaa000000,UNIT_GOP_SMI_3          )  /* in BC3 this is UNIT_GOP_SMI_1 in pipe 1 */



    /* must be last */
    ,{SMAIN_NOT_VALID_CNS,NULL,SMAIN_NOT_VALID_CNS}
};

/* list of duplicated units .
    NOTE:
    the UNIT_GOP_LED_2,UNIT_GOP_LED_3,UNIT_GOP_SMI_2,UNIT_GOP_SMI_3 will be
        explicitly duplicated in BC2 code
*/
static SMEM_UNIT_DUPLICATION_INFO_STC ALDRIN2_duplicatedUnits[30] =
{
    {STR(UNIT_RX_DMA)  ,3}, /* 3 duplication of this unit */
        {STR(UNIT_RX_DMA_1)},
        {STR(UNIT_RX_DMA_2)},
        {STR(UNIT_RX_DMA_3)},

    {STR(UNIT_TX_FIFO) ,3}, /* 3 duplication of this unit */
        {STR(UNIT_TX_FIFO_1)},
        {STR(UNIT_TX_FIFO_2)},
        {STR(UNIT_TX_FIFO_3)},

    {STR(UNIT_TX_DMA)  ,3}, /* 3 duplication of this unit */
        {STR(UNIT_TX_DMA_1)},
        {STR(UNIT_TX_DMA_2)},
        {STR(UNIT_TX_DMA_3)},

    {STR(UNIT_TXQ_DQ)  ,3}, /* 3 duplication of this unit */
        {STR(UNIT_TXQ_DQ_1)},
        {STR(UNIT_TXQ_DQ_2)},
        {STR(UNIT_TXQ_DQ_3)},

    {STR(UNIT_SBC)  ,1},  /* 1 duplication of this unit */
        {STR(UNIT_SBC_1)},

    {STR(UNIT_TAI)  ,1},  /* 1 duplication of this unit */
        {STR(UNIT_TAI_1)},

    {STR(UNIT_LPM)  ,1}, /* 1 duplication of this unit */
        {STR(UNIT_LPM_1)},


    {NULL,0} /* must be last */
};

/* DMA : special ports mapping {global,local,DP}*/
static SPECIAL_PORT_MAPPING_CNS ALDRIN2_DMA_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
     {72/*global DMA port*/,24/*local DMA port*/,0/*DP[]*/}
    /* 73 not exists */
    ,{74/*global DMA port*/,24/*local DMA port*/,3/*DP[]*/}
    ,{75/*global DMA port*/,24/*local DMA port*/,1/*DP[]*/}
    ,{76/*global DMA port*/,24/*local DMA port*/,2/*DP[]*/}
    ,{77/*global DMA port*/,23/*local DMA port*/,0/*DP[]*/}

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

static SKERNEL_INTERRUPT_REG_INFO_STC skernel_interrupt_reg_info__last_line__cns[] =
{{{SKERNEL_REGISTERS_DB_TYPE__LAST___E}}};

enum{
    FuncUnitsInterrupts_Pipe0_Cnc0_Int = 1,
    FuncUnitsInterrupts_Pipe0_Cnc1_Int = 2,
    FuncUnitsInterrupts_Pipe0_Eft_Int = 3,
    FuncUnitsInterrupts_Pipe0_Eoam_Int = 4,
    FuncUnitsInterrupts_Pipe0_Epcl_Int = 5,
    FuncUnitsInterrupts_Pipe0_Eplr_Int = 6,
    FuncUnitsInterrupts_Pipe0_Eq_Int = 7,
    FuncUnitsInterrupts_Pipe0_Ermrk_Int = 8,
    FuncUnitsInterrupts_Pipe0_Ha_Int = 9,
    FuncUnitsInterrupts_Pipe0_Ioam_Int = 10,
    FuncUnitsInterrupts_Pipe0_Iplr0_Int = 11,
    FuncUnitsInterrupts_Pipe0_Iplr1_Int = 12,
    FuncUnitsInterrupts_Pipe0_Ipvx_Int = 13,
    FuncUnitsInterrupts_Pipe0_L2I_Int = 14,
    FuncUnitsInterrupts_Pipe0_Mll_Int = 15,
    FuncUnitsInterrupts_Pipe0_Pcl_Int = 16,
    FuncUnitsInterrupts_Pipe0_Tti_Int = 17,
    FuncUnitsInterrupts_Pipe0_Txq0_Dq_Int = 18,
    FuncUnitsInterrupts_Pipe0_Txq1_Dq_Int = 19,
    FuncUnitsInterrupts_Pipe0_Txq2_Dq_Int = 20,
    FuncUnitsInterrupts_Pipe0_Egf_Sht_Int = 21,
    FuncUnitsInterrupts_Egf_Qag_Int = 22,
    FuncUnitsInterrupts_Lpm0_Int = 23,
    FuncUnitsInterrupts_Lpm1_Int = 24,
    FuncUnitsInterrupts_Mt_Int = 27,
    FuncUnitsInterrupts_Tcam_Int = 28,
    FuncUnitsInterrupts_Mg0_1_Int_Out0 = 29,
    FuncUnitsInterrupts_Mg0_1_Int_Out1 = 30,
    FuncUnitsInterrupts_Mg0_1_Int_Out2 = 31
};

enum{
    Func1UnitsInterrupts_Pipe1_Cnc0_Int            = FuncUnitsInterrupts_Pipe0_Cnc0_Int,
    Func1UnitsInterrupts_Pipe1_Cnc1_Int            = FuncUnitsInterrupts_Pipe0_Cnc1_Int,
    Func1UnitsInterrupts_Pipe1_Eft_Int             = FuncUnitsInterrupts_Pipe0_Eft_Int,
    Func1UnitsInterrupts_Pipe1_Eoam_Int            = FuncUnitsInterrupts_Pipe0_Eoam_Int,
    Func1UnitsInterrupts_Pipe1_Epcl_Int            = FuncUnitsInterrupts_Pipe0_Epcl_Int,
    Func1UnitsInterrupts_Pipe1_Eplr_Int            = FuncUnitsInterrupts_Pipe0_Eplr_Int,
    Func1UnitsInterrupts_Pipe1_Eq_Int              = FuncUnitsInterrupts_Pipe0_Eq_Int,
    Func1UnitsInterrupts_Pipe1_Ermrk_Int           = FuncUnitsInterrupts_Pipe0_Ermrk_Int,
    Func1UnitsInterrupts_Pipe1_Ha_Int              = FuncUnitsInterrupts_Pipe0_Ha_Int,
    Func1UnitsInterrupts_Pipe1_Ioam_Int            = FuncUnitsInterrupts_Pipe0_Ioam_Int,
    Func1UnitsInterrupts_Pipe1_Iplr0_Int           = FuncUnitsInterrupts_Pipe0_Iplr0_Int,
    Func1UnitsInterrupts_Pipe1_Iplr1_Int           = FuncUnitsInterrupts_Pipe0_Iplr1_Int,
    Func1UnitsInterrupts_Pipe1_Ipvx_Int            = FuncUnitsInterrupts_Pipe0_Ipvx_Int,
    Func1UnitsInterrupts_Pipe1_L2I_Int             = FuncUnitsInterrupts_Pipe0_L2I_Int,
    Func1UnitsInterrupts_Pipe1_Mll_Int             = FuncUnitsInterrupts_Pipe0_Mll_Int,
    Func1UnitsInterrupts_Pipe1_Pcl_Int             = FuncUnitsInterrupts_Pipe0_Pcl_Int,
    Func1UnitsInterrupts_Pipe1_Tti_Int             = FuncUnitsInterrupts_Pipe0_Tti_Int,
    Func1UnitsInterrupts_Pipe1_Txq0_Dq_Int         = FuncUnitsInterrupts_Pipe0_Txq0_Dq_Int,
    Func1UnitsInterrupts_Pipe1_Txq1_Dq_Int         = FuncUnitsInterrupts_Pipe0_Txq1_Dq_Int,
    Func1UnitsInterrupts_Pipe1_Txq2_Dq_Int         = FuncUnitsInterrupts_Pipe0_Txq2_Dq_Int,
    Func1UnitsInterrupts_Pipe1_Egf_Sht_Int         = FuncUnitsInterrupts_Pipe0_Egf_Sht_Int,
    Func1UnitsInterrupts_Txq_Ll_Int                = 22,
    Func1UnitsInterrupts_Txq_Qcn_Int               = 23,
    Func1UnitsInterrupts_Txq_Queue_Int             = 24,
    Func1UnitsInterrupts_Txq_Bmx_Int               = 26,
    Func1UnitsInterrupts_Server_Int                = 27
};

enum {
    DataPath_Pipe0_Rxdma0_Rxdma_Int = 1,
    DataPath_Pipe0_Rxdma1_Rxdma_Int = 2,
    DataPath_Pipe0_Rxdma2_Rxdma_Int = 3,
    DataPath_Pipe0_Txdma0_Txdma_Int = 4,
    DataPath_Pipe0_Txdma1_Txdma_Int = 5,
    DataPath_Pipe0_Txdma2_Txdma_Int = 6,
    DataPath_Pipe0_Txfifo0_Txfifo_Int = 7,
    DataPath_Pipe0_Txfifo1_Txfifo_Int = 8,
    DataPath_Pipe0_Txfifo2_Txfifo_Int = 9,
    DataPath_Pipe1_Rxdma0_Rxdma_Int = 10,
    DataPath_Pipe1_Rxdma1_Rxdma_Int = 11,
    DataPath_Pipe1_Rxdma2_Rxdma_Int = 12,
    DataPath_Pipe1_Txdma0_Txdma_Int = 13,
    DataPath_Pipe1_Txdma1_Txdma_Int = 14,
    DataPath_Pipe1_Txdma2_Txdma_Int = 15,
    DataPath_Pipe1_Txfifo0_Txfifo_Int = 16,
    DataPath_Pipe1_Txfifo1_Txfifo_Int = 17,
    DataPath_Pipe1_Txfifo2_Txfifo_Int = 18,
    DataPath_Bm_Int = 19,
    DataPath_Bma_Int = 20,
    DataPath_Mppm_Int = 21,
    DataPath_Mbus_Gpio_7_0_Int = 22,
    DataPath_Mbus_Gpio_15_8_Int = 23,
    DataPath_Mbus_Gpio_23_16_Int = 24,
    DataPath_I2C_Int = 25,
    DataPath_Mg2_Int_Out0 = 26,
    DataPath_Mg2_Int_Out1 = 27,
    DataPath_Mg2_Int_Out2 = 28,
    DataPath_Mg3_Int_Out0 = 29,
    DataPath_Mg3_Int_Out1 = 30,
    DataPath_Mg3_Int_Out2 = 31
};


static SKERNEL_INTERRUPT_REG_INFO_STC cnc_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.CNCInterruptSummaryCauseReg)},/*SMEM_CHT3_CNC_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.CNCInterruptSummaryMaskReg)},
    /*myFatherInfo*/{
        /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Cnc0_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[1].globalRegs.CNCInterruptSummaryCauseReg)},/*SMEM_CHT3_CNC_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[1].globalRegs.CNCInterruptSummaryMaskReg)},
    /*myFatherInfo*/{
        /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Cnc1_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
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
        /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Eft_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
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
        /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Ioam_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[1].OAMUnitInterruptCause)},/*SMEM_LION2_OAM_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[1].OAMUnitInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Eoam_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
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
        /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Pcl_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
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
        /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Tti_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
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
        /*myBitIndex*/FuncUnitsInterrupts_Pipe0_L2I_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC mac_tbl_intrrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(FDB.FDBCore.FDBInterrupt.FDBInterruptCauseReg)},/*SMEM_CHT_MAC_TBL_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(FDB.FDBCore.FDBInterrupt.FDBInterruptMaskReg)},
    /*myFatherInfo*/{
        /*myBitIndex*/FuncUnitsInterrupts_Mt_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC TcamInterruptsSummary =
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TCAM.tcamInterrupts.TCAMInterruptsSummaryCause)},/**/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TCAM.tcamInterrupts.TCAMInterruptsSummaryMask)},/**/
    /*myFatherInfo*/{
        /*myBitIndex*/FuncUnitsInterrupts_Tcam_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
    };

static SKERNEL_INTERRUPT_REG_INFO_STC tcam_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TCAM.tcamInterrupts.TCAMInterruptCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TCAM.tcamInterrupts.TCAMInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&TcamInterruptsSummary
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
        /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Ha_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
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
        /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Ermrk_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
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
        /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Eq_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC eq_critical_ecc_cntrs_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.criticalECCCntrs.BMCriticalECCInterruptCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.criticalECCCntrs.BMCriticalECCInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/3 ,
            /*interruptPtr*/&preEgrInterruptSummary[0]
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
        /*interruptPtr*/&CpuCodeRateLimitersInterruptSummary[0]                                                                \
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
            /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Mll_Int ,
            /*interruptPtr*/&FuncUnitsInterruptsSummary
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
            /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Iplr0_Int ,
            /*interruptPtr*/&FuncUnitsInterruptsSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[1].policerInterruptCause)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[1].policerInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Iplr1_Int ,
            /*interruptPtr*/&FuncUnitsInterruptsSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[2].policerInterruptCause)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[2].policerInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Eplr_Int ,
            /*interruptPtr*/&FuncUnitsInterruptsSummary
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC  TXQInterruptSummary =
{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.queue.global.interrupt.transmitQueueInterruptSummaryCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.queue.global.interrupt.transmitQueueInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/Func1UnitsInterrupts_Txq_Queue_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary1
        },
    /*isTriggeredByWrite*/ 1
};

static SKERNEL_INTERRUPT_REG_INFO_STC  txQDQInterruptSummary[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[0].global.interrupt.txQDQInterruptSummaryCause)},/*SMEM_CHT_TX_QUEUE_INTERRUPT_CAUSE_REG , SMEM_LION_TXQ_DQ_INTR_SUM_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[0].global.interrupt.txQDQInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Txq0_Dq_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[1].global.interrupt.txQDQInterruptSummaryCause)},/*SMEM_CHT_TX_QUEUE_INTERRUPT_CAUSE_REG , SMEM_LION_TXQ_DQ_INTR_SUM_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[1].global.interrupt.txQDQInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Txq1_Dq_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[2].global.interrupt.txQDQInterruptSummaryCause)},/*SMEM_CHT_TX_QUEUE_INTERRUPT_CAUSE_REG , SMEM_LION_TXQ_DQ_INTR_SUM_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[2].global.interrupt.txQDQInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Txq2_Dq_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
    }
   ,{
   /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[3].global.interrupt.txQDQInterruptSummaryCause)},/*SMEM_CHT_TX_QUEUE_INTERRUPT_CAUSE_REG , SMEM_LION_TXQ_DQ_INTR_SUM_CAUSE_REG*/
   /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[3].global.interrupt.txQDQInterruptSummaryMask)},
   /*myFatherInfo*/{
       /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Txq0_Dq_Int ,
       /*interruptPtr*/&FuncUnitsInterruptsSummary1
       },
   /*isTriggeredByWrite*/ 1
   }
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
    /* egress stc - support 128 ports per DQ */
     EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(0, 0)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(0, 1)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(0, 2)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(0, 3)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(1, 0)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(1, 1)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(1, 2)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(1, 3)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(2, 0)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(2, 1)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(2, 2)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(2, 3)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(3, 0)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(3, 1)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(3, 2)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(3, 3)
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
    /* flush done - support 128 ports per DQ */
     FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(0, 0)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(0, 1)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(0, 2)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(0, 3)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(1, 0)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(1, 1)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(1, 2)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(1, 3)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(2, 0)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(2, 1)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(2, 2)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(2, 3)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(3, 0)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(3, 1)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(3, 2)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(3, 3)
};

#define  TXQ_DQ_GENERAL_CAUSE_MAC(dq)                                                                                \
{                                                                                                                    \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[dq].global.interrupt.txQDQGeneralInterruptCause)}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[dq].global.interrupt.txQDQGeneralInterruptMask)},  \
/*myFatherInfo*/{                                                                                                    \
    /*myBitIndex*/10,                                                                                                \
    /*interruptPtr*/&txQDQInterruptSummary[dq]                                                                       \
    },                                                                                                               \
/*isTriggeredByWrite*/ 1                                                                                             \
}

static SKERNEL_INTERRUPT_REG_INFO_STC txq_dq_general_interrupt[] =
{
    TXQ_DQ_GENERAL_CAUSE_MAC(0),
    TXQ_DQ_GENERAL_CAUSE_MAC(1),
    TXQ_DQ_GENERAL_CAUSE_MAC(2),
    TXQ_DQ_GENERAL_CAUSE_MAC(3)
};


static SKERNEL_INTERRUPT_REG_INFO_STC txq_queue_global_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.queue.global.interrupt.generalInterrupt1Cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.queue.global.interrupt.generalInterrupt1Mask)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&TXQInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.queue.global.interrupt.portDescFullInterruptCause0)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.queue.global.interrupt.portDescFullInterruptMask0)},
        /*myFatherInfo*/{
            /*myBitIndex*/2 ,
            /*interruptPtr*/&TXQInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.queue.global.interrupt.portDescFullInterruptCause1)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.queue.global.interrupt.portDescFullInterruptMask1)},
        /*myFatherInfo*/{
            /*myBitIndex*/3 ,
            /*interruptPtr*/&TXQInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
     }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.queue.global.interrupt.portDescFullInterruptCause2)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.queue.global.interrupt.portDescFullInterruptMask2)},
        /*myFatherInfo*/{
            /*myBitIndex*/4 ,
            /*interruptPtr*/&TXQInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
     }
    ,{
            /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.qcn.QCNInterruptCause)},
            /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXQ.qcn.QCNInterruptMask)},
            /*myFatherInfo*/{
                /*myBitIndex*/23 ,
                /*interruptPtr*/&FuncUnitsInterruptsSummary1
                },
            /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC  ingrSTCInterruptSummary[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.ingrSTCInterrupt.ingrSTCInterruptSummary)}, /*SMEM_LION3_EQ_INGRESS_STC_INT_SUMMARY_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.ingrSTCInterrupt.ingrSTCInterruptSummaryMask)},  /*SMEM_LION3_EQ_INGRESS_STC_INT_SUMMARY_MASK_REG*/
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&preEgrInterruptSummary[0]
            },
        /*isTriggeredByWrite*/ 1
    }
};

/* node for ingress STC summary of 16 ports */
#define  INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(index) \
{                                                       \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.ingrSTCInterrupt.ingrSTCInterruptCause[index])}, /*SMEM_CHT_INGRESS_STC_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.ingrSTCInterrupt.ingrSTCInterruptMask[index]) },  /*SMEM_CHT_INGRESS_STC_INT_MASK_REG*/  \
    /*myFatherInfo*/{                                     \
        /*myBitIndex*/((index)+1) ,                       \
        /*interruptPtr*/&ingrSTCInterruptSummary[0]       \
        },                                                \
    /*isTriggeredByWrite*/ 1                              \
}

static SKERNEL_INTERRUPT_REG_INFO_STC ingr_stc_ports_interrupts[] =
{
    /* ingress stc - support 256 physical ports of EQ */
     INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 2)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 3)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 4)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 5)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 6)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 7)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 8)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 9)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(10)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(11)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(12)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(13)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(14)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(15)
};

static SKERNEL_INTERRUPT_REG_INFO_STC MppmInterruptsSummary =
{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MPPM.interrupts.MPPMInterruptsSummary)},/**/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MPPM.interrupts.MPPMInterruptsSummaryMask)},/**/
    /*myFatherInfo*/{
        /*myBitIndex*/21 ,
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
        /*myBitIndex*/DataPath_Bm_Int ,
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
            /*myBitIndex*/1 ,
            /*interruptPtr*/&dataPathInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[1].interrupts.rxDMAInterruptSummaryCause)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[1].interrupts.rxDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/2 ,
            /*interruptPtr*/&dataPathInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[2].interrupts.rxDMAInterruptSummaryCause)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[2].interrupts.rxDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/3 ,
            /*interruptPtr*/&dataPathInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[3].interrupts.rxDMAInterruptSummaryCause)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[3].interrupts.rxDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/10 ,
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
};

static SKERNEL_INTERRUPT_REG_INFO_STC TxDmaInterruptsSummary[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[0].interrupts.txDMAInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[0].interrupts.txDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/4 ,
            /*interruptPtr*/&dataPathInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[1].interrupts.txDMAInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[1].interrupts.txDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/5 ,
            /*interruptPtr*/&dataPathInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[2].interrupts.txDMAInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[2].interrupts.txDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/6 ,
            /*interruptPtr*/&dataPathInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[3].interrupts.txDMAInterruptSummary)},/**/
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[3].interrupts.txDMAInterruptSummaryMask)},/**/
       /*myFatherInfo*/{
           /*myBitIndex*/13 ,
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
};

static SKERNEL_INTERRUPT_REG_INFO_STC TxFifoInterruptsSummary[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[0].interrupts.txFIFOInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[0].interrupts.txFIFOInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/7 ,
            /*interruptPtr*/&dataPathInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[1].interrupts.txFIFOInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[1].interrupts.txFIFOInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/8 ,
            /*interruptPtr*/&dataPathInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[2].interrupts.txFIFOInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[2].interrupts.txFIFOInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/9 ,
            /*interruptPtr*/&dataPathInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[3].interrupts.txFIFOInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[3].interrupts.txFIFOInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
           /*myBitIndex*/16 ,
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

/* define the XLG port summary, for port 31..61 in ports_1_int_sum per pipe 0  */
#define  DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(port) \
    DEFINE_XLG_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(port, ((port - 31) + 1)/*bitInMg*/,ports1InterruptsSummary/*regInMg*/);\
    DEFINE_GIGA_SUMMARY_PORT_IN_XLG_INTERRUPT_SUMMARY_MAC(port)

/* define the XLG port summary, for port 26..35 in ports_2_int_sum per pipe 1 */
#define  DEFINE_XLG_SUMMARY_PORT_62_73_INTERRUPT_SUMMARY_MAC(port) \
    DEFINE_XLG_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(port, ((port-62) + 1)/*bitInMg*/,ports2InterruptsSummary/*regInMg*/);\
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
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (16);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (17);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (18);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (19);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (20);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (21);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (22);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (23);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (24);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (25);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (26);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (27);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (28);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (29);
DEFINE_XLG_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC (30);

DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(31);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(32);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(33);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(34);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(35);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(36);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(37);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(38);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(39);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(40);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(41);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(42);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(43);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(44);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(45);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(46);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(47);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(48);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(49);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(50);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(51);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(52);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(53);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(54);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(55);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(56);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(57);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(58);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(59);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(60);
DEFINE_XLG_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(61);

DEFINE_XLG_SUMMARY_PORT_62_73_INTERRUPT_SUMMARY_MAC(62);
DEFINE_XLG_SUMMARY_PORT_62_73_INTERRUPT_SUMMARY_MAC(63);
DEFINE_XLG_SUMMARY_PORT_62_73_INTERRUPT_SUMMARY_MAC(64);
DEFINE_XLG_SUMMARY_PORT_62_73_INTERRUPT_SUMMARY_MAC(65);
DEFINE_XLG_SUMMARY_PORT_62_73_INTERRUPT_SUMMARY_MAC(66);
DEFINE_XLG_SUMMARY_PORT_62_73_INTERRUPT_SUMMARY_MAC(67);
DEFINE_XLG_SUMMARY_PORT_62_73_INTERRUPT_SUMMARY_MAC(68);
DEFINE_XLG_SUMMARY_PORT_62_73_INTERRUPT_SUMMARY_MAC(69);
DEFINE_XLG_SUMMARY_PORT_62_73_INTERRUPT_SUMMARY_MAC(70);
DEFINE_XLG_SUMMARY_PORT_62_73_INTERRUPT_SUMMARY_MAC(71);
DEFINE_XLG_SUMMARY_PORT_62_73_INTERRUPT_SUMMARY_MAC(72);


static SKERNEL_INTERRUPT_REG_INFO_STC xlg_mac_ports_interrupts[] =
{
    /* the ports 0..11 MG CHAIN 0 --> DP[0] ports 0..11 */
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
    /* the ports 12..23 MG CHAIN 0  --> DP[1] ports 0..11 */
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(12)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(13)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(14)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(15)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(16)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(17)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(18)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(19)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(20)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(21)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(22)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(23)
    /* the ports 0..23 MG CHAIN 1  --> DP[2] ports 0..23 */
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(24)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(25)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(26)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(27)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(28)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(29)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(30)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(31)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(32)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(33)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(34)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(35)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(36)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(37)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(38)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(39)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(40)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(41)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(42)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(43)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(44)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(45)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(46)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(47)
    /* the ports 24..47 MG CHAIN 1  --> DP[3] ports 0..23 */
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(48)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(49)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(50)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(51)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(52)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(53)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(54)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(55)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(56)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(57)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(58)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(59)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(60)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(61)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(62)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(63)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(64)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(65)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(66)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(67)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(68)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(69)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(70)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(71)
    /* the port 48 MG CHAIN 1  --> DP[0] ports 0..24 */
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(72)
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

DEFINE_CG_100G_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC( 0);
DEFINE_CG_100G_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC( 4);
DEFINE_CG_100G_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC( 8);
DEFINE_CG_100G_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC(12);
DEFINE_CG_100G_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC(16);
DEFINE_CG_100G_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC(20);

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
    /* the CG ports 0..11 MG CHAIN 0  , steps of 4 --> DP[0] ports 0..11 , steps of 4 */
     CG_MAC_100G_INTERRUPT_CAUSE_MAC( 0)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC( 4)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC( 8)
    /* the CG ports 12..23 MG CHAIN 0 , steps of 4 --> DP[1] ports 0..11 , steps of 4 */
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(12)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(16)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(20)
};


/* node for egress ptp xlg and giga mac ports */
#define  GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(port)               \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(GOP_PTP.PTP[port].PTPInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(GOP_PTP.PTP[port].PTPInterruptMask)},  /**/                            \
    /*myFatherInfo*/{                                     \
        /*myBitIndex*/2,                                 \
        /*interruptPtr*/&gigaInterruptSummary_##port      \
    },                                                    \
    /*isTriggeredByWrite*/ 1,                            \
    /*isSecondFatherExists*/ 1,                          \
    /*myFatherInfo_2*/{                                     \
        /*myBitIndex*/7,                                 \
        /*interruptPtr*/&xlgInterruptSummary_##port      \
    }                                                     \
}


static SKERNEL_INTERRUPT_REG_INFO_STC ptp_mac_ports_interrupts[] =
{
     GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(0)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(1 )
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(2 )
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(3 )
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(4 )
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(5 )
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(6 )
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(7 )
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(8 )
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(9 )
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(10)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(11)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(12)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(13)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(14)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(15)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(16)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(17)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(18)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(19)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(20)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(21)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(22)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(23)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(24)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(25)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(26)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(27)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(28)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(29)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(30)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(31)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(32)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(33)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(34)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(35)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(36)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(37)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(38)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(39)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(40)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(41)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(42)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(43)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(44)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(45)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(46)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(47)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(48)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(49)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(50)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(51)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(52)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(53)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(54)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(55)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(56)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(57)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(58)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(59)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(60)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(61)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(62)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(63)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(64)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(65)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(66)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(67)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(68)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(69)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(70)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(71)
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(72)
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
    /* the Giga ports 0..72 */
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
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(16)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(17)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(18)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(19)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(20)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(21)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(22)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(23)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(24)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(25)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(26)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(27)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(28)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(29)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(30)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(31)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(32)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(33)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(34)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(35)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(36)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(37)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(38)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(39)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(40)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(41)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(42)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(43)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(44)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(45)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(46)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(47)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(48)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(49)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(50)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(51)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(52)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(53)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(54)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(55)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(56)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(57)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(58)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(59)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(60)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(61)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(62)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(63)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(64)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(65)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(66)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(67)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(68)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(69)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(70)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(71)
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(72)
};

static void addDuplicatedUnitIfNotExists
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_CHAR*         origUnitNamePtr,
    IN GT_CHAR*         dupUnitNamePtr
)
{
    GT_U32  ii,jj;
    SMEM_UNIT_DUPLICATION_INFO_STC   *unitInfoPtr_1 = &ALDRIN2_duplicatedUnits[0];
    SMEM_UNIT_DUPLICATION_INFO_STC   *unitInfoPtr_2;
    GT_U32  found_1,found_2;


    for(ii = 0 ; unitInfoPtr_1->unitNameStr != NULL; ii++)
    {
        if(0 == strcmp(origUnitNamePtr,unitInfoPtr_1->unitNameStr))
        {
            found_1 = 1;
        }
        else
        {
            found_1 = 0;
        }

        found_2 = 0;

        unitInfoPtr_2 = unitInfoPtr_1+1;
        for(jj = 0 ; jj < unitInfoPtr_1->numOfUnits ; jj++,unitInfoPtr_2++)
        {
            if(found_1 == 0)
            {
                if(0 == strcmp(origUnitNamePtr,unitInfoPtr_2->unitNameStr))
                {
                    found_1 = 1;
                }
            }

            if(found_2 == 0)
            {
                if(0 == strcmp(dupUnitNamePtr,unitInfoPtr_2->unitNameStr))
                {
                    found_2 = 1;
                }
            }

            if(found_1 && found_2)
            {
                return;
            }
        }

        unitInfoPtr_1 = unitInfoPtr_2;
        ii += jj;
    }

    /* we need to add 2 entries to the DB (and 'NULL' entry) */
    if((ii+2+1) >= (sizeof(ALDRIN2_duplicatedUnits)/sizeof(ALDRIN2_duplicatedUnits[0])))
    {
        skernelFatalError("addDuplicatedUnitIfNotExists : array ALDRIN2_duplicatedUnits[] not large enough \n");
        return;
    }

    ALDRIN2_duplicatedUnits[ii].unitNameStr = origUnitNamePtr;
    ALDRIN2_duplicatedUnits[ii].numOfUnits = 1;

    ALDRIN2_duplicatedUnits[ii+1].unitNameStr = dupUnitNamePtr;
    ALDRIN2_duplicatedUnits[ii+1].numOfUnits = 0;

    ALDRIN2_duplicatedUnits[ii+2].unitNameStr = NULL;
    ALDRIN2_duplicatedUnits[ii+2].numOfUnits = 0;

    return ;
}


static void buildDevUnitAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /* build
        aldrin2UsedUnitsAddressesArray - the addresses of the units that the bobk uses
        aldrin2UnitNameAndIndexArr - name of unit and index in aldrin2UsedUnitsAddressesArray */
    GT_U32  ii,jj;
    GT_U32  index;
    GT_U32  size;
    UNIT_INFO_STC   *unitInfoPtr = &aldrin2units[0];

    index = 0;
    for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
    {
        if(unitInfoPtr->orig_nameStr)
        {
            addDuplicatedUnitIfNotExists(devObjPtr, unitInfoPtr->orig_nameStr, unitInfoPtr->nameStr);
        }

        /* the size is in '1<<24' but the arrays are units of '1<<23' so we do
            lopp on : *2 */
        size = unitInfoPtr->size*2;
        for(jj = 0 ; jj < size ; jj++ , index++)
        {
            if(index >= (sizeof(aldrin2UsedUnitsAddressesArray) / sizeof(aldrin2UsedUnitsAddressesArray[0])))
            {
                skernelFatalError("buildDevUnitAddr : over flow of units (1) \n");
            }

            if(index >= (sizeof(aldrin2UnitNameAndIndexArr) / sizeof(aldrin2UnitNameAndIndexArr[0])))
            {
                skernelFatalError("buildDevUnitAddr : over flow of units (2) \n");
            }

            aldrin2UsedUnitsAddressesArray[index].unitBaseAddr = unitInfoPtr->base_addr ;
            aldrin2UsedUnitsAddressesArray[index].unitSizeInBytes = 0;
            aldrin2UnitNameAndIndexArr[index].unitNameIndex = index;
            aldrin2UnitNameAndIndexArr[index].unitNameStr = unitInfoPtr->nameStr;
        }
    }

    if(index >= (sizeof(aldrin2UnitNameAndIndexArr) / sizeof(aldrin2UnitNameAndIndexArr[0])))
    {
        skernelFatalError("buildDevUnitAddr : over flow of units (3) \n");
    }
    /* indication of no more */
    aldrin2UnitNameAndIndexArr[index].unitNameIndex = SMAIN_NOT_VALID_CNS;
    aldrin2UnitNameAndIndexArr[index].unitNameStr = NULL;

    devObjPtr->devMemUnitNameAndIndexPtr = aldrin2UnitNameAndIndexArr;
    devObjPtr->genericUsedUnitsAddressesArray = aldrin2UsedUnitsAddressesArray;
    devObjPtr->genericNumUnitsAddresses = index+1;
    devObjPtr->devMemUnitPipeOffsetGet = NULL;/* no pipe offset */
}

static GT_VOID smemAldrin2InterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_INTERRUPT_REG_INFO_STC ** currDbPtrPtr
)
{
    GT_U32 index = 0;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr = aldrin2InterruptsTreeDb;

    smemBobcat2GlobalInterruptTreeInit(devObjPtr, &index, &currDbPtr);

    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, tti_interrupts                    );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, bridge_interrupts                 );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, mac_tbl_intrrupts                 );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, ha_interrupts                     );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, ermrk_interrupts                  );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, bm_general_cause_interrupts       );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, pcl_unit_interrupts               );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, eq_critical_ecc_cntrs_interrupts  );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, tcam_interrupts                   );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, cpu_code_rate_limiters_interrupts );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, ingr_stc_ports_interrupts         );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, mppm_ecc_interrupts               );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, egf_eft_global_interrupts         );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, mll_interrupts                    );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, txq_queue_global_interrupts       );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, policer_ipfix_interrupts          );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, cnc_interrupts                    );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, oam_interrupts                    );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, rxdma_interrupts                  );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, txdma_interrupts                  );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, txfifo_interrupts                 );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, xlg_mac_ports_interrupts          );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, egr_stc_ports_interrupts          );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, flush_done_ports_interrupts       );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, txq_dq_general_interrupt          );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, cg_100g_mac_ports_interrupts      );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, ptp_mac_ports_interrupts          );
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, giga_mac_ports_interrupts         );


    /* End of interrupts - must be last */
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, skernel_interrupt_reg_info__last_line__cns);

    *currDbPtrPtr = aldrin2InterruptsTreeDb;
}

/**
* @internal smemAldrin2UnitGop function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the GOP unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemAldrin2UnitGop
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* Port<n> Auto-Negotiation Configuration Register */
    {0x0000000C                       , GOP_PER_63_PORTS_MASK_CNS, NULL, 0 , smemChtActiveWriteForceLinkDown, 0},
    {0x0000000C + CHAIN_1_GOP_OFFSET  , GOP_PER_63_PORTS_MASK_CNS, NULL, 0 , smemChtActiveWriteForceLinkDown, 0},

    /* Port MAC Control Register2 */
    {0x00000008                       , GOP_PER_63_PORTS_MASK_CNS, NULL, 0 , smemChtActiveWriteMacGigControl2, 0},
    {0x00000008 + CHAIN_1_GOP_OFFSET  , GOP_PER_63_PORTS_MASK_CNS, NULL, 0 , smemChtActiveWriteMacGigControl2, 0},

    /* Port MAC Control Register0 */
    {0x000C0000,                        GOP_PER_63_PORTS_MASK_CNS, NULL, 0 , smemChtActiveWriteForceLinkDownXg, 0},
    {0x000C0000 + CHAIN_1_GOP_OFFSET  , GOP_PER_63_PORTS_MASK_CNS, NULL, 0 , smemChtActiveWriteForceLinkDownXg, 0},

    /* Port MAC Control Register3 */
    {0x000C001C,                        GOP_PER_63_PORTS_MASK_CNS, NULL, 0 , smemXcatActiveWriteMacModeSelect, 0},
    {0x000C001C + CHAIN_1_GOP_OFFSET  , GOP_PER_63_PORTS_MASK_CNS, NULL, 0 , smemXcatActiveWriteMacModeSelect, 0},

    /* Port MAC Control Register4 : SMEM_XCAT_XG_MAC_CONTROL4_REG */
    {0x000C0084,                        GOP_PER_63_PORTS_MASK_CNS, NULL, 0 , smemBobKActiveWriteMacXlgCtrl4, 0},
    {0x000C0084 + CHAIN_1_GOP_OFFSET  , GOP_PER_63_PORTS_MASK_CNS, NULL, 0 , smemBobKActiveWriteMacXlgCtrl4, 0},


    /* stack gig ports - Tri-Speed Port<n> Interrupt Mask Register */
    {0x000C0024,                        GOP_PER_63_PORTS_MASK_CNS, NULL, 0 , smemChtActiveWritePortInterruptsMaskReg, 0},
    {0x000C0024 + CHAIN_1_GOP_OFFSET  , GOP_PER_63_PORTS_MASK_CNS, NULL, 0 , smemChtActiveWritePortInterruptsMaskReg, 0},

    /* XG Port<n> Interrupt Cause Register  */
    {0x000C0014,                        GOP_PER_63_PORTS_MASK_CNS,  smemChtActiveReadIntrCauseReg, 29, smemChtActiveWriteIntrCauseReg, 0},
    {0x000C0014 + CHAIN_1_GOP_OFFSET  , GOP_PER_63_PORTS_MASK_CNS,  smemChtActiveReadIntrCauseReg, 29, smemChtActiveWriteIntrCauseReg, 0},

    /* XG Port<n> Interrupt Mask Register */
    {0x000C0018,                        GOP_PER_63_PORTS_MASK_CNS, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},
    {0x000C0018 + CHAIN_1_GOP_OFFSET  , GOP_PER_63_PORTS_MASK_CNS, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},

     /* PTP Port<n> Interrupt Cause Register  */
    {0x00180800,                      GOP_PER_63_PORTS_MASK_CNS,  smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x00180800 + CHAIN_1_GOP_OFFSET, GOP_PER_63_PORTS_MASK_CNS,  smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},

    /* PTP Port<n> Interrupt Mask Register */
    {0x00180804,                      GOP_PER_63_PORTS_MASK_CNS, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},
    {0x00180804 + CHAIN_1_GOP_OFFSET, GOP_PER_63_PORTS_MASK_CNS, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},

    /* stack gig ports - Port<n> Interrupt Cause Register  */
    {0x00000020,                        GOP_PER_63_PORTS_MASK_CNS,smemChtActiveReadIntrCauseReg, 18, smemChtActiveWriteIntrCauseReg, 0},
    {0x00000020 + CHAIN_1_GOP_OFFSET  , GOP_PER_63_PORTS_MASK_CNS,smemChtActiveReadIntrCauseReg, 18, smemChtActiveWriteIntrCauseReg, 0},

    /* stack gig ports - Tri-Speed Port<n> Interrupt Mask Register */
    {0x00000024,                        GOP_PER_63_PORTS_MASK_CNS, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},
    {0x00000024 + CHAIN_1_GOP_OFFSET  , GOP_PER_63_PORTS_MASK_CNS, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},

    /* CG-mac (100G mac) */
    /* <CG_IP> CG TOP/Units/CG Upper Top Units %a Pipe %t/Converters registers/Control 0 */
    {0x00340000,                        GOP_PER_63_PORTS_MASK_CNS, NULL, 0 , smemChtActiveWrite100GMacControl0, 0},
    {0x00340000 + CHAIN_1_GOP_OFFSET  , GOP_PER_63_PORTS_MASK_CNS, NULL, 0 , smemChtActiveWrite100GMacControl0, 0},

    /* <CG_IP> CG TOP/Units/CG Upper Top Units %a Pipe %t/Converters registers/Resets */
    {0x00340010,                        GOP_PER_63_PORTS_MASK_CNS, NULL, 0 , smemChtActiveWrite100GMacResets, 0},
    {0x00340010 + CHAIN_1_GOP_OFFSET  , GOP_PER_63_PORTS_MASK_CNS, NULL, 0 , smemChtActiveWrite100GMacResets, 0},

    /*CG Interrupt mask*/
    /*SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_CONVERTERS.CGMAConvertersCG_Interrupt_mask*/
    {0x00340044,                        GOP_PER_63_PORTS_MASK_CNS, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},
    {0x00340044 + CHAIN_1_GOP_OFFSET  , GOP_PER_63_PORTS_MASK_CNS, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},

    /*CG Interrupt cause*/
    /*SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_CONVERTERS.CGMAConvertersCG_Interrupt_cause*/
    {0x00340040,                        GOP_PER_63_PORTS_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x00340040 + CHAIN_1_GOP_OFFSET  , GOP_PER_63_PORTS_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},

    /*STATN_CONFIG */
    /*SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CGPORTMACStatNConfig*/
    {0x00340478,                        GOP_PER_63_PORTS_MASK_CNS, NULL, 0, smemChtActiveWrite100GMacMibCtrlReg, 0},
    {0x00340478 + CHAIN_1_GOP_OFFSET  , GOP_PER_63_PORTS_MASK_CNS, NULL, 0, smemChtActiveWrite100GMacMibCtrlReg, 0},

    /* need to cover
       from : 0x10340480 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.aFramesTransmittedOK
       to   : 0x10340624 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.aMACControlFramesReceived

       so we do several 'larger ranges' :
        1. 0x10340400 - 0x103405FC
        2. 0x10340600 - 0x1034063C
    */

    /* 1. 0x10340400 - 0x103405FC */
    {0x00340400,                        0xFFFC0E00, smemBobcat3ActiveRead100GMacMibCounters, 0, NULL, 0},
    {0x00340400+ CHAIN_1_GOP_OFFSET  ,  0xFFFC0E00, smemBobcat3ActiveRead100GMacMibCounters, 0, NULL, 0},
    /* 2. 0x10340600 - 0x1034063C */
    {0x00340600,                        0xFFFC0FC0, smemBobcat3ActiveRead100GMacMibCounters, 0, NULL, 0},
    {0x00340600+ CHAIN_1_GOP_OFFSET  ,  0xFFFC0FC0, smemBobcat3ActiveRead100GMacMibCounters, 0, NULL, 0},


    /* 3 PTP TX Timestamp Queue0 reg2 */
    {0x00000800 + 0x14, 0xFF000FFF, smemLion3ActiveReadGopPtpTxTsQueueReg2Reg, 0, NULL, 0},
    /* 4 PTP TX Timestamp Queue1 reg2 */
    {0x00000800 + 0x20, 0xFF000FFF, smemLion3ActiveReadGopPtpTxTsQueueReg2Reg, 0, NULL, 0},


    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* Gig Ports*/
                /* ports 0..72 */
                 GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000000, 0x00000094)
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000000A0, 0x000000A4)
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000000C0, 0x000000E0)

            /* XLG */
                /* ports 0..72 */
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000c0000 , 0x000c0024 )
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000c002C , 0x000C0030 )
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000C0038 , 0x000C0060 )
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000C0068 , 0x000C0088 )
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000C0090 , 0x000C0098 )

            /* MPCS */
                /* ports 0..72 */
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180008 , 0x00180018 )
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180020 , 0x00180024 )
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180030 , 0x00180030 )
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0018003C , 0x001800C8 )
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001800D0 , 0x00180120 )
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180128 , 0x0018014C )
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0018015C , 0x0018017C )
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180200 , (0x00180200+256))

            /* XPCS IP */
                /* ports 0..72 in steps of 2*/
                ,GOP____STEP_2_PORTS_SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180400, 0x00180424)
                ,GOP____STEP_2_PORTS_SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0018042C, 0x0018044C)
                /* 6 lanes */
                ,GOP____6_LANES_STEP_2_PORTS_SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00180450, 0x0044)

            /*FCA*/
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180600, 0x001806A0)
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180704, 0x00180714)
            /*PTP*/
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180800, 0x0018087C)

            /* Mac-TG Generator */
                ,GOP____SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180C00 ,0x00180CCC)

            /* CG-mac (100G mac) - only ports 0..23 (chain 0) */
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00340000, 0x00340034)}, FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00340040, 0x0034004C)}, FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00340060, 0x0034009C)}, FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00340100, 0x00340108)}, FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00340400, 1024)}      , FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00340800, 1024)}      , FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS}
            /* CG-PCS-mac (100G mac) - only ports 0..23 (chain 0) */
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300000, 0x00300020)}, FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300038, 0x0030003C)}, FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300080, 0x00300084)}, FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x003000A8, 0x003000B4)}, FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x003000C8, 0x003000D4)}, FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300320, 0x0030036C)}, FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300640, 0x0030068C)}, FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300800, 0x0030080C)}, FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300840, 0x00300840)}, FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300900, 0x0030099C)}, FORMULA_SINGLE_PARAMETER_GOP_CG_DUP_MG_CHAIN_0_PORTS_CNS}

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* PTP */
             GOP____DEFAULT_REG_MAC(0x00180808,         0x00000001)
            ,GOP____DEFAULT_REG_MAC(0x00180870,         0x000083aa)
            ,GOP____DEFAULT_REG_MAC(0x00180874,         0x00007e5d)
            ,GOP____DEFAULT_REG_MAC(0x00180878,         0x00000040)

            /*Giga*/
            ,GOP____DEFAULT_REG_MAC(0x00000000,         0x00008be5)
            ,GOP____DEFAULT_REG_MAC(0x00000004,         0x00000003)
            ,GOP____DEFAULT_REG_MAC(0x00000008,         0x0000c048)
            ,GOP____DEFAULT_REG_MAC(0x0000000c,         0x0000bae8)
            ,GOP____DEFAULT_REG_MAC(0x0000001c,         0x00000052)
            ,GOP____2_PARAM_DEFAULT_REG_MAC(0x0000002c, 0x0000000c,     2,    0x18)
            ,GOP____DEFAULT_REG_MAC(0x00000030,         0x0000c815)
            ,GOP____DEFAULT_REG_MAC(0x00000048,         0x00000300)
            ,GOP____DEFAULT_REG_MAC(0x00000094,         0x00000001)
            ,GOP____DEFAULT_REG_MAC(0x000000c0,         0x00001004)
            ,GOP____DEFAULT_REG_MAC(0x000000c4,         0x00000100)
            ,GOP____DEFAULT_REG_MAC(0x000000c8,         0x000001fd)

            ,GOP____DEFAULT_REG_MAC(0x00000090,         0x0000ff9a)
            ,GOP____DEFAULT_REG_MAC(0x00000018,         0x00004b4d)
            ,GOP____DEFAULT_REG_MAC(0x00000014,         0x000008c4)
            ,GOP____DEFAULT_REG_MAC(0x000000d4,         0x000000ff)

            /*XLG*/
            /* set ALL ports as XLG */
            ,GOP____DEFAULT_REG_MAC(0x000c001c,         0x00006000)
            ,GOP____DEFAULT_REG_MAC(0x000c0030,         0x000007ec)
            ,GOP____DEFAULT_REG_MAC(0x000c0080,         0x00001000)
            ,GOP____DEFAULT_REG_MAC(0x000c0084,         0x00001210)
            /*FCA*/
            ,GOP____DEFAULT_REG_MAC(0x00180000+0x600,   0x00000011)
            ,GOP____DEFAULT_REG_MAC(0x00180004+0x600,   0x00002003)
            ,GOP____DEFAULT_REG_MAC(0x00180054+0x600,   0x00000001)
            ,GOP____DEFAULT_REG_MAC(0x00180058+0x600,   0x0000c200)
            ,GOP____DEFAULT_REG_MAC(0x0018005c+0x600,   0x00000180)
            ,GOP____DEFAULT_REG_MAC(0x0018006c+0x600,   0x00008808)
            ,GOP____DEFAULT_REG_MAC(0x00180070+0x600,   0x00000001)
            ,GOP____DEFAULT_REG_MAC(0x00180104+0x600,   0x0000ff00)

            /* CG-mac (100G mac) - only ports 0..23 (chain 0) */
            ,{DUMMY_NAME_PTR_CNS,            0x00340000,         0x10020018,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340004,         0x0000003f,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340008,         0xff00ff00,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0034000c,         0x00000190,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340014,         0x0a080040,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340018,         0x00002f2d,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0034001c,         0x0000001c,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340060,         0x01408000,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340064,         0x00477690,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS ,4,    0x10   }
            ,{DUMMY_NAME_PTR_CNS,            0x00340068,         0x00e6c4f0,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS ,4,    0x10   }
            ,{DUMMY_NAME_PTR_CNS,            0x0034006c,         0x009b65c5,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS ,4,    0x10   }
            ,{DUMMY_NAME_PTR_CNS,            0x00340070,         0x003d79a2,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS ,3,    0x10   }
            ,{DUMMY_NAME_PTR_CNS,            0x00340100,         0x003d79a2,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340104,         0x3fff3fff,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS  ,2,    0x4   }

            ,{DUMMY_NAME_PTR_CNS,            0x00340400,         0x00010106,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340414,         0x00000600,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0034041c,         0x00000010,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS  ,2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00340430,         0x00007d00,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340434,         0x00000001,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340444,         0x0000000c,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }

            ,{DUMMY_NAME_PTR_CNS,            0x00340804,         0x00008003,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340a04,         0x0000f000,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340a0c,         0x00000022,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340a28,         0x0000002a,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS  ,4,    0x4   }

            /* CG-PCS-mac (100G mac) - only ports 0..23 (chain 0) */
            ,{DUMMY_NAME_PTR_CNS,            0x00300000,         0x00002050,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300010,         0x00000100,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300014,         0x00000008,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030001c,         0x00000005,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300020,         0x00008020,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x003000b4,         0x00008000,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300804,         0x00000001,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300808,         0x00003fff,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030080c,         0x00009999,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300900,         0x000068c1,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300904,         0x00000021,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300908,         0x0000719d,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030090c,         0x0000008e,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300910,         0x00004b59,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300914,         0x000000e8,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300918,         0x0000954d,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030091c,         0x0000007b,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300920,         0x000007f5,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300924,         0x00000009,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300928,         0x000014dd,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030092c,         0x000000c2,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300930,         0x00004a9a,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300934,         0x00000026,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300938,         0x0000457b,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030093c,         0x00000066,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300940,         0x000024a0,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300944,         0x00000076,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300948,         0x0000c968,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030094c,         0x000000fb,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300950,         0x00006cfd,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300954,         0x00000099,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300958,         0x000091b9,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030095c,         0x00000055,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300960,         0x0000b95c,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300964,         0x000000b2,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300968,         0x0000f81a,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030096c,         0x000000bd,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300970,         0x0000c783,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300974,         0x000000ca,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300978,         0x00003635,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030097c,         0x000000cd,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300980,         0x000031c4,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300984,         0x0000004c,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300988,         0x0000d6ad,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030098c,         0x000000b7,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300990,         0x0000665f,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300994,         0x0000002a,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300998,         0x0000f0c0,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030099c,         0x000000e5,      DEFAULT_REG_GOP_CG_DUP_MG_CHAIN_0_AND_PORTS_CNS }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemAldrin2UnitLpSerdes function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the LpSerdes unit
*
* @param[in] devObjPtr                - pointer to common device memory object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemAldrin2UnitLpSerdes
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

/*SERDES*/
    /* took registers of offset 0x200 of Lion2 and convert to offset 0x800 */
    /* COMPHY_H %t Registers/KVCO Calibration Control (0x00000800 + t*0x1000: where t (0-35) represents SERDES) */
            /**************************************/
            /*  handle memory space of MG CHAIN 0 */
            /**************************************/
            /* serdes external registers SERDESes for ports 0...23 */
    {0x00000808                        , 0xFFF00FFF, NULL, 0, smemLion2ActiveWriteKVCOCalibrationControlReg, 0},
            /**************************************/
            /*  handle memory space of MG CHAIN 1 */
            /**************************************/
            /* serdes external registers SERDESes for ports 24...72 */
    {0x00000808 + CHAIN_1_SERDES_OFFSET, 0xFFF00FFF, NULL, 0, smemLion2ActiveWriteKVCOCalibrationControlReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /**************************************/
            /*  handle memory space of MG CHAIN 0 */
            /**************************************/
            /* serdes external registers SERDESes for ports 0...23 */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 0x44)} , FORMULA_SINGLE_PARAMETER_SERDES_DUP_MG_CHAIN_0_PORTS_CNS}

            /**************************************/
            /*  handle memory space of MG CHAIN 1 */
            /**************************************/
            /* serdes external registers SERDESes for ports 24...71 */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (CHAIN_1_SERDES_OFFSET          +0x00000000, 0x44)} , FORMULA_SINGLE_PARAMETER_SERDES_DUP_MG_CHAIN_1_PORTS_CNS}

            /* serdes external registers SERDESes for port 72 */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (CPU_PORT_CHAIN_1_SERDES_OFFSET +0x00000000, 0x44)} }

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* SERDES registers */
            /***********************************************/
            /*  handle default registers of SERDES CHAIN 0 */
            /***********************************************/
            /* ports 0..23 */
            {DUMMY_NAME_PTR_CNS,         0x00000000,         0x00000800,   DEFAULT_REG_SERDESA_DUP_MG_CHAIN_0_AND_PORTS_CNS}
           ,{DUMMY_NAME_PTR_CNS,         0x00000004,         0x00008801,   DEFAULT_REG_SERDESA_DUP_MG_CHAIN_0_AND_PORTS_CNS}
           ,{DUMMY_NAME_PTR_CNS,         0x00000008,         0x00000100,   DEFAULT_REG_SERDESA_DUP_MG_CHAIN_0_AND_PORTS_CNS}

            /***********************************************/
            /*  handle default registers of SERDES CHAIN 1 */
            /***********************************************/
            /* ports 24..71 */
           ,{DUMMY_NAME_PTR_CNS,         CHAIN_1_SERDES_OFFSET          +0x00000000,         0x00000800,   DEFAULT_REG_SERDESA_DUP_MG_CHAIN_1_AND_PORTS_CNS}
           ,{DUMMY_NAME_PTR_CNS,         CHAIN_1_SERDES_OFFSET          +0x00000004,         0x00008801,   DEFAULT_REG_SERDESA_DUP_MG_CHAIN_1_AND_PORTS_CNS}
           ,{DUMMY_NAME_PTR_CNS,         CHAIN_1_SERDES_OFFSET          +0x00000008,         0x00000100,   DEFAULT_REG_SERDESA_DUP_MG_CHAIN_1_AND_PORTS_CNS}

            /* port 72 */
           ,{DUMMY_NAME_PTR_CNS,         CPU_PORT_CHAIN_1_SERDES_OFFSET +0x00000000,         0x00000800}
           ,{DUMMY_NAME_PTR_CNS,         CPU_PORT_CHAIN_1_SERDES_OFFSET +0x00000004,         0x00008801}
           ,{DUMMY_NAME_PTR_CNS,         CPU_PORT_CHAIN_1_SERDES_OFFSET +0x00000008,         0x00000100}

           /* must be last */
           ,{NULL,            0,         0x00000000,      0,    0x0      }
        };

        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}
/**
* @internal smemAldrin2UnitXGPortMib function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the XGPortMib unit
*
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemAldrin2UnitXGPortMib
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* XG port MAC MIB Counters */
            /**************************************/
            /*  handle memory space of MG CHAIN 0 */
            /**************************************/
    {0x00000000,                            SMEM_BOBCAT2_XG_MIB_COUNT_MSK_CNS, smemLion2ActiveReadMsmMibCounters, 0, NULL,0},
            /**************************************/
            /*  handle memory space of MG CHAIN 1 */
            /**************************************/
    {0x00000000 + CHAIN_1_MIB_OFFSET_GIG  , SMEM_BOBCAT2_XG_MIB_COUNT_MSK_CNS, smemLion2ActiveReadMsmMibCounters, 0, NULL,0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* steps between each port */
    devObjPtr->xgCountersStepPerPort   = BOBCAT2_MIB_OFFSET_CNS;
    devObjPtr->xgCountersStepPerPort_1 = 0;/* not valid */
    /* offset of table xgPortMibCounters_1 */
    devObjPtr->offsetToXgCounters_1 = 0;/* not valid */
    devObjPtr->startPortNumInXgCounters_1 = 0;/* not valid */

    /* chunks with formulas */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* base of 0x00000000 */
            /**************************************/
            /*  handle memory space of MG CHAIN 0 */
            /**************************************/
            /* ports 0..23 */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000000 , 256)}, FORMULA_SINGLE_PARAMETER_MIB_DUP_MG_CHAIN_0_PORTS_CNS}
            /**************************************/
            /*  handle memory space of MG CHAIN 0 */
            /**************************************/
            /* ports 24..71 */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(CHAIN_1_MIB_OFFSET_GIG      + 0x00000000 , 256)}, FORMULA_SINGLE_PARAMETER_MIB_DUP_MG_CHAIN_1_PORTS_CNS}

            /* port 72 */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(CPU_PORT_CHAIN_1_MIB_OFFSET + 0x00000000 , 256)}}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    /* don't point the next 'half' unit on 'me' */
    unitPtr[1].hugeUnitSupportPtr = NULL;
}

/**
* @internal smemAldrin2UnitRxDma function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemAldrin2UnitRxDma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000040)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000084, 0x0000008C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000C4, 0x000000D0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000E4, 0x00000160)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000174, 0x00000180)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000198, 0x00000294)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002F0, 0x000002FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000320, 0x0000032C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000350, 0x00000350)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000374, 0x00000374)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000398, 0x000003AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003C8, 0x000003D4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003E8, 0x000003F4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000408, 0x00000414)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000428, 0x00000434)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000043C, 0x00000448)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000450, 0x0000045C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000004D0, 0x000005E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000630, 0x0000063C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000710, 0x00000714)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000720, 0x00000724)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000730, 0x00000738)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000740, 0x00000740)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000760, 0x00000764)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000804, 0x00000864)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000950, 0x000009B0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B60)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000E68)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000120C, 0x0000126C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001528, 0x0000152C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001548, 0x0000154C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001704, 0x0000170C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001720, 0x0000175C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001790, 0x00001790)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800, 0x00001800)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001820, 0x00001820)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001830, 0x00001834)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001840, 0x00001840)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001850, 0x00001850)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001864, 0x00001868)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001870, 0x00001870)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001900, 0x0000190C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001930, 0x00001930)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001940, 0x00001940)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001B00, 0x00001B60)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001E00, 0x00001E60)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000020F0, 0x000020F0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x00002160)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002400, 0x00002460)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002A00, 0x00002B58)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002B60, 0x00002B60)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003500, 0x00003560)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003A00, 0x00003A60)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004080, 0x0000409C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004100, 0x0000411C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004200, 0x00004200)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005000)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
              {DUMMY_NAME_PTR_CNS,             0x00000000 ,       0xbfffbfff,     8,    0x8    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000004 ,       0x0000bfff,     8,    0x8    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000040 ,       0x0000bfff,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000084 ,       0x81008100,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x0000008C ,       0x0001FFE2,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x000000A0 ,       0x00000003,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000350 ,       0x00000800,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000374 ,       0x00008600,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000398 ,       0x00008100,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x0000039C ,       0x00008A88,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x000003A8 ,       0x00008847,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x000003AC ,       0x00008848,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x000005D0 ,       0xFFFF0000,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000804 ,       0x0FFFF000,     25,   0x4    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000954 ,       0x00000001,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000958 ,       0x00000002,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x0000095C ,       0x00000003,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000960 ,       0x00000004,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000964 ,       0x00000005,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000968 ,       0x00000006,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x0000096C ,       0x00000007,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000970 ,       0x00000008,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000974 ,       0x00000009,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000978 ,       0x0000000A,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x0000097C ,       0x0000000B,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000980 ,       0x0000000C,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000984 ,       0x0000000D,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000988 ,       0x0000000E,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x0000098C ,       0x0000000F,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000990 ,       0x00000010,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000994 ,       0x00000011,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000998 ,       0x00000012,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x0000099C ,       0x00000013,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x000009A0 ,       0x00000014,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x000009A4 ,       0x00000015,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x000009A8 ,       0x00000016,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x000009AC ,       0x00000017,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x000009B0 ,       0x00000018,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00000B00 ,       0x00000002,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00001704 ,       0x00780001,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00001800 ,       0x007FFDFF,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00001840 ,       0x00000040,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00001850 ,       0xAAAAAAAA,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00001864 ,       0x0000BFFF,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00001868 ,       0x00000005,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00001930 ,       0x0000001F,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00001940 ,       0x0000000A,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00001B00 ,       0x00000015,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A00 ,       0x00004049,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A04 ,       0x00000004,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A08 ,       0x83828180,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A0C ,       0x87868584,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A10 ,       0x8B8A8988,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A14 ,       0x8F8E8D8C,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A18 ,       0x93929190,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A1C ,       0x97969594,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A20 ,       0x9B9A9998,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A24 ,       0x9F9E9D9C,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A28 ,       0xA3A2A1A0,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A2C ,       0xA7A6A5A4,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A30 ,       0xABAAA9A8,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A34 ,       0xAFAEADAC,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A38 ,       0xB3B2B1B0,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A3C ,       0xB7B6B5B4,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A40 ,       0xBBBAB9B8,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A44 ,       0xBFBEBDBC,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A48 ,       0xC3C2C1C0,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A4C ,       0xC7C6C5C4,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A50 ,       0x828180C8,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A54 ,       0x86858483,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A58 ,       0x8A898887,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A5C ,       0x8E8D8C8B,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A60 ,       0x9291908F,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A64 ,       0x96959493,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A68 ,       0x9A999897,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A6C ,       0x9E9D9C9B,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A70 ,       0xA2A1A09F,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A74 ,       0xA6A5A4A3,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A78 ,       0xAAA9A8A7,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A7C ,       0xAEADACAB,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A80 ,       0xB2B1B0AF,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A84 ,       0xB6B5B4B3,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A88 ,       0xBAB9B8B7,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A8C ,       0xBEBDBCBB,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A90 ,       0xC2C1C0BF,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A94 ,       0xC6C5C4C3,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A98 ,       0x8180C8C7,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002A9C ,       0x85848382,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AA0 ,       0x89888786,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AA4 ,       0x8D8C8B8A,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AA8 ,       0x91908F8E,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AAC ,       0x95949392,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AB0 ,       0x99989796,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AB4 ,       0x9D9C9B9A,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AB8 ,       0xA1A09F9E,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002ABC ,       0xA5A4A3A2,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AC0 ,       0xA9A8A7A6,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AC4 ,       0xADACABAA,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AC8 ,       0xB1B0AFAE,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002ACC ,       0xB5B4B3B2,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AD0 ,       0xB9B8B7B6,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AD4 ,       0xBDBCBBBA,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AD8 ,       0xC1C0BFBE,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002ADC ,       0xC5C4C3C2,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AE0 ,       0x80C8C7C6,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AE4 ,       0x84838281,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AE8 ,       0x88878685,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AEC ,       0x8C8B8A89,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AF0 ,       0x908F8E8D,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AF4 ,       0x94939291,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AF8 ,       0x98979695,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002AFC ,       0x9C9B9A99,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B00 ,       0xA09F9E9D,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B04 ,       0xA4A3A2A1,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B08 ,       0xA8A7A6A5,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B0C ,       0xACABAAA9,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B10 ,       0xB0AFAEAD,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B14 ,       0xB4B3B2B1,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B18 ,       0xB8B7B6B5,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B1C ,       0xBCBBBAB9,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B20 ,       0xC0BFBEBD,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B24 ,       0xC4C3C2C1,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B28 ,       0xC8C7C6C5,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B2C ,       0x83828180,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B30 ,       0x87868584,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B34 ,       0x8B8A8988,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B38 ,       0x8F8E8D8C,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B3C ,       0x93929190,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B40 ,       0x97969594,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B44 ,       0x9B9A9998,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B48 ,       0x9F9E9D9C,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B4C ,       0xA3A2A1A0,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B50 ,       0xA7A6A5A4,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B54 ,       0xABAAA9A8,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00002B58 ,       0xAFAEADAC,     1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,             0x00003A00 ,       0x00000001,    25,   0x4      }
             ,{DUMMY_NAME_PTR_CNS,             0x00004200 ,       0x00000001,     1,    0x0    }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemAldrin2UnitTxDma function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemAldrin2UnitTxDma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000250)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000504)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001020, 0x0000102C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001060, 0x0000106C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002018, 0x0000201C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002038, 0x00002044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002050, 0x00002054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x00002160)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002300, 0x00002360)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x0000331C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003A00, 0x00003A60)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004158)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004160, 0x00004160)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000501C, 0x00005078)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005080, 0x0000508C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005100, 0x0000510C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005120, 0x0000512C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005140, 0x0000514C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005160, 0x0000516C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005180, 0x0000518C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000051A0, 0x000051AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000051C0, 0x000051CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005200, 0x00005260)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005500, 0x00005560)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005700, 0x00005760)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005900, 0x00005960)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006100, 0x0000611C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006300, 0x0000631C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006500, 0x0000650C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006520, 0x0000652C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006540, 0x0000654C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006560, 0x0000656C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006580, 0x0000658C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000065A0, 0x000065AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000065C0, 0x000065CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000, 0x00007004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007100, 0x00007160)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007300, 0x00007300)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007500, 0x0000750C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007700, 0x00007700)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007800, 0x00007860)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007930, 0x0000793C)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {

              {DUMMY_NAME_PTR_CNS,             0x00000000,         0x00000007,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000004,         0x000bfd02,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000008,         0x002fffff,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000010,         0x0000ffff,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000014,         0x00000041,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000220,         0x22222222,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00001000,         0x00001fff,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00001004,         0x000390e7,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00001008,         0x00000007,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000100c,         0x000390e7,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00001010,         0x000390e7,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00001014,         0x00000005,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00001020,         0x00777777,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00001060,         0x7777777f,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00003000,         0x00000001,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00003004,         0x00000126,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00003008,         0xc89fe0c8,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000300c,         0x00000c03,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00003010,         0x00020100,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00003014,         0x0000000f,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00003028,         0x551fe0c8,      6,    0x40}
             ,{DUMMY_NAME_PTR_CNS,             0x00003048,         0x761fe0c8,      3,    0x80,      2,    0x140}
             ,{DUMMY_NAME_PTR_CNS,             0x00003088,         0xc89fe0c8,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00003108,         0xc89fe0c8,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000031a8,         0x449fe0c8,      6,    0x40}
             ,{DUMMY_NAME_PTR_CNS,             0x000031c8,         0x551fe0c8,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00003248,         0x551fe0c8,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000032c8,         0x551fe0c8,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00003308,         0x449fe0c8,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004000,         0x00004019,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004004,         0x00000004,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004008,         0x83828180,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x0000400c,         0x87868584,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x00004010,         0x8b8a8988,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x00004014,         0x8f8e8d8c,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x00004018,         0x93929190,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x0000401c,         0x97969594,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x00004020,         0x82818098,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x00004024,         0x86858483,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x00004028,         0x8a898887,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x0000402c,         0x8e8d8c8b,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x00004030,         0x9291908f,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004034,         0x96959493,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004038,         0x81809897,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000403c,         0x85848382,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004040,         0x89888786,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004044,         0x8d8c8b8a,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004048,         0x91908f8e,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000404c,         0x95949392,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004050,         0x80989796,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004054,         0x84838281,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004058,         0x88878685,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000405c,         0x8c8b8a89,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004060,         0x908f8e8d,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004064,         0x94939291,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004068,         0x98979695,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004094,         0x9291908f,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004098,         0x96959493,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000409c,         0x81809897,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000040a0,         0x85848382,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000040a4,         0x89888786,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000040a8,         0x8d8c8b8a,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000040ac,         0x91908f8e,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000040b0,         0x95949392,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000040b4,         0x80989796,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000040b8,         0x84838281,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000040bc,         0x88878685,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000040c0,         0x8c8b8a89,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000040c4,         0x908f8e8d,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000040c8,         0x94939291,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000040cc,         0x98979695,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000040f8,         0x9291908f,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000040fc,         0x96959493,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004100,         0x81809897,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004104,         0x85848382,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004108,         0x89888786,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000410c,         0x8d8c8b8a,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004110,         0x91908f8e,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004114,         0x95949392,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004118,         0x80989796,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000411c,         0x84838281,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004120,         0x88878685,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004124,         0x8c8b8a89,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004128,         0x908f8e8d,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000412c,         0x94939291,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004130,         0x98979695,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00007000,         0x00b7aaa0,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00007004,         0x00b7aaa0,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00007100,         0x80000a00,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00007300,         0x00000a00,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00007500,         0x8001fc00,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00007700,         0x0000000a,      1,    0x0 }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemAldrin2UnitTxFifo function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemAldrin2UnitTxFifo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000068)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x0000040C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000420, 0x00000424)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000440, 0x0000044C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000508, 0x0000050C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000518, 0x0000051C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000660)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000958)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000960, 0x00000960)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x00001160)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x00001360)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x00001560)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001700, 0x00001760)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001900, 0x00001960)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001B00, 0x00001B60)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003100, 0x00003160)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003300, 0x00003300)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 600)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00012000, 8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00014000, 0x00014000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015000, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015200, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015400, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015600, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015800, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015A00, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017000, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017200, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017400, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017600, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017800, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017A00, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 680)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00022000, 4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00024000, 0x00024000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025000, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025200, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025400, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025600, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025800, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025A00, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027000, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027200, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027400, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027600, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027800, 100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027A00, 100)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {

              {DUMMY_NAME_PTR_CNS,             0x00000000,         0x80001ddc,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000004,         0x0000ffff,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000008,         0x00000001,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000600,         0x00000036,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000604,         0x0000002b,      6,    0x8 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000608,         0x0000002d,      3,    0x10,      2,    0x28 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000610,         0x00000036,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000620,         0x00000036,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000634,         0x00000023,      6,    0x8 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000638,         0x0000002b,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000648,         0x0000002b,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000658,         0x0000002b,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000660,         0x00000023,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000800,         0x00004019,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000804,         0x00000004,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000808,         0x83828180,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x0000080c,         0x87868584,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x00000810,         0x8b8a8988,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x00000814,         0x8f8e8d8c,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x00000818,         0x93929190,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x0000081c,         0x97969594,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x00000820,         0x82818098,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x00000824,         0x86858483,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x00000828,         0x8a898887,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x0000082c,         0x8e8d8c8b,      4,    0x64}
             ,{DUMMY_NAME_PTR_CNS,             0x00000830,         0x9291908f,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000834,         0x96959493,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000838,         0x81809897,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000083c,         0x85848382,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000840,         0x89888786,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000844,         0x8d8c8b8a,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000848,         0x91908f8e,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000084c,         0x95949392,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000850,         0x80989796,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000854,         0x84838281,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000858,         0x88878685,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000085c,         0x8c8b8a89,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000860,         0x908f8e8d,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000864,         0x94939291,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000868,         0x98979695,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000894,         0x9291908f,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000898,         0x96959493,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000089c,         0x81809897,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000008a0,         0x85848382,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000008a4,         0x89888786,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000008a8,         0x8d8c8b8a,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000008ac,         0x91908f8e,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000008b0,         0x95949392,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000008b4,         0x80989796,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000008b8,         0x84838281,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000008bc,         0x88878685,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000008c0,         0x8c8b8a89,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000008c4,         0x908f8e8d,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000008c8,         0x94939291,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000008cc,         0x98979695,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000008f8,         0x9291908f,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000008fc,         0x96959493,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000900,         0x81809897,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000904,         0x85848382,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000908,         0x89888786,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000090c,         0x8d8c8b8a,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000910,         0x91908f8e,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000914,         0x95949392,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000918,         0x80989796,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000091c,         0x84838281,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000920,         0x88878685,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000924,         0x8c8b8a89,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000928,         0x908f8e8d,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000092c,         0x94939291,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000930,         0x98979695,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00003000,         0x00000bac,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00003004,         0x00000bac,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00003100,         0x80025540,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00003300,         0x00025540,      1,    0x0 }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemAldrin2UnitTxqDq function
* @endinternal
*
* @brief   Allocate address type specific memories -- TXQ DQ
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemAldrin2UnitTxqDq
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000064)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000208, 0x00000248)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000260, 0x00000260)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002C0, 0x00000320)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000460)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000604)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000610, 0x00000610)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000620, 0x00000620)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000630, 0x00000630)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000640, 0x00000640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000650, 0x00000650)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000660, 0x00000660)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000670, 0x00000670)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000680, 0x00000680)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000690, 0x00000690)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000800)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000808, 0x00000808)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x00000904)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000920, 0x00000924)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A00)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B60)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D60)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001008, 0x00001010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001040, 0x000010A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001240, 0x0000133C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001380, 0x000013E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x00001500)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001510, 0x00001570)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001650, 0x000017A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000017B0, 0x000017B0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800, 0x00001800)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001904, 0x0000196C)}
            /*Scheduler State Variable RAM*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 800), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(168,32),SMEM_BIND_TABLE_MAC(Scheduler_State_Variable)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x0000400C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004100, 0x00004160)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004300, 0x00004360)}
            /*Priority Token Bucket Configuration*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00006000, 1600), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(488,64)}
            /*Port Token Bucket Configuration*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000A000, 200), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(61,8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C000, 0x0000C000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C008, 0x0000C044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C080, 0x0000C09C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C100, 0x0000C13C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C180, 0x0000C37C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C9FC, 0x0000CA00)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D000, 0x0000D004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D010, 0x0000D014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D040, 0x0000D088)}
            /*Egress STC Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000D800, 400), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(80,16),SMEM_BIND_TABLE_MAC(egressStc)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00011000, 512)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {

              {DUMMY_NAME_PTR_CNS,             0x00000000,         0x001fc140,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000208,         0x28002800,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000020c,         0x00000002,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000002c4,         0x00000001,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000002c8,         0x00000002,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000002cc,         0x00000003,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000002d0,         0x00000004,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000002d4,         0x00000005,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000002d8,         0x00000006,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000002dc,         0x00000007,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000002e0,         0x00000008,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000002e4,         0x00000009,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000002e8,         0x0000000a,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000002ec,         0x0000000b,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000002f0,         0x0000000c,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000002f4,         0x0000000d,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000002f8,         0x0000000e,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x000002fc,         0x0000000f,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000300,         0x00000010,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000304,         0x00000011,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000030c,         0x00000001,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000310,         0x00000002,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000314,         0x00000003,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000318,         0x00000004,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000031c,         0x00000005,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000320,         0x00000006,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00000a00,         0x0000ffff,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00001000,         0x14e44e15,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00001008,         0x00000002,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00001040,         0x00000003,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00001240,         0x01010101,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00001280,         0x05050505,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00001300,         0x000000ff,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00001500,         0x00001553,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00001650,         0x83828180,      5,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x00001654,         0x87868584,      5,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x00001658,         0x8b8a8988,      5,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x0000165c,         0x8f8e8d8c,      5,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x00001660,         0x93929190,      5,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x00001664,         0x97969594,      5,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x00001668,         0x9b9a9998,      5,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x0000166c,         0x9f9e9d9c,      5,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x00001670,         0xa3a2a1a0,      5,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x00001674,         0xa7a6a5a4,      5,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x00001678,         0xabaaa9a8,      5,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x0000167c,         0xafaeadac,      5,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x00001680,         0xb3b2b1b0,      5,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x00001684,         0xb7b6b5b4,      4,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x00001688,         0xbbbab9b8,      4,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x0000168c,         0xbfbebdbc,      4,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x00001690,         0xc3c2c1c0,      4,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x00001694,         0xc7c6c5c4,      4,    0x48}
             ,{DUMMY_NAME_PTR_CNS,             0x00001904,         0x05050505,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00001908,         0x00000004,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004000,         0x82080000,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004004,         0x00000600,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004008,         0x00000600,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x00004300,         0x000000ff,     25,    0x4  }
             ,{DUMMY_NAME_PTR_CNS,             0x0000c000,         0x00000006,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000c084,         0x00000001,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000c088,         0x00000002,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000c08c,         0x00000003,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000c090,         0x00000004,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000c094,         0x00000005,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000c098,         0x00000006,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000c09c,         0x00000007,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000d000,         0x00000020,      1,    0x0 }
             ,{DUMMY_NAME_PTR_CNS,             0x0000d014,         0x00000008,      1,    0x0 }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemAldrin2UnitTxqQueue function
* @endinternal
*
* @brief Allocate address type specific memories -- TXQ Queue
*
* @param[in] devObjPtr   - pointer to device object.
* @param[in] unitPtr     - pointer to the unit chunk
*/
static void smemAldrin2UnitTxqQueue
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        TXQ_QUEQUE_ACTIVE_MEM_MAC,
        TXQ_QUEQUE_EGR_PACKET_COUNTERS_CLEAR_ON_READ_ACTIVE_MEM_MAC ,
    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090000, 0x00090014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090040, 0x00090054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090060, 0x0009019C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090280, 0x0009039C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090400, 0x00090430)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090480, 0x0009059C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090680, 0x0009079C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090880, 0x000908AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000908C0, 0x00090904)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000909A0, 0x000909E4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000909F0, 0x000909F0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090A00, 0x00090A0C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090A40, 0x00090A40)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090A80, 0x00090A80)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090A90, 0x00090A94)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090B00, 0x00090B48)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090C00, 0x00090C08)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00093000, 0x00093010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00093020, 0x00093020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00093200, 0x00093214)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00093220, 0x00093224)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00093230, 0x00093234)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00093240, 0x00093244)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00093250, 0x00093254)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00093260, 0x00093264)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00093270, 0x00093274)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00093280, 0x00093284)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00093290, 0x00093294)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000936A0, 0x000936A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0000, 0x000A0044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0050, 0x000A016C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0200, 0x000A031C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0360, 0x000A03BC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A03D0, 0x000A040C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0440, 0x000A0450)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0500, 0x000A063C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0800, 0x000A0800)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0810, 0x000A0810)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0820, 0x000A085C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0900, 0x000A093C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0A00, 0x000A0A3C)}
            /*Maximum Queue Limits*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000A1000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(34,8),SMEM_BIND_TABLE_MAC(Shared_Queue_Maximum_Queue_Limits)}
            /*Queue Limits DP0 - Enqueue*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000A1800, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(48,8),SMEM_BIND_TABLE_MAC(Queue_Limits_DP0_Enqueue)}
            /*Queue Buffer Limits - Dequeue*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000A2000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(34,8),SMEM_BIND_TABLE_MAC(Queue_Buffer_Limits_Dequeue)}
            /*Queue Descriptor Limits - Dequeue*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000A2800, 512), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(14,4),SMEM_BIND_TABLE_MAC(Queue_Descriptor_Limits_Dequeue)}
            /*Queue Limits DP12 - Enqueue*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000A3000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(48,8),SMEM_BIND_TABLE_MAC(Queue_Limits_DP12_Enqueue)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A4000, 0x000A4004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A4010, 0x000A426C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A4290, 0x000A42AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A9000, 0x000A9004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A9010, 0x000A9010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A9020, 0x000A9020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A9030, 0x000A9030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A9200, 0x000A9210)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A9500, 0x000A9504)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A9510, 0x000A951C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000AA000, 0x000AA040)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000AA050, 0x000AA12C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000AA1B0, 0x000AA1CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000AA210, 0x000AA210)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000AA230, 0x000AA230)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000AA400, 576), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64,8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000AA800, 576), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64,8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000B0000, 0x000B0034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000B0040, 0x000B0074)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000B0080, 0x000B00B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000B00C0, 0x000B00F4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000B0100, 0x000B010C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000B1120, 0x000B18FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000B1920, 0x000B20FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000B2120, 0x000B28FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000B2920, 0x000B30FC)}
            /*Q Main Buff*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000C0000, 18432), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(16,4)}
            /*Q Main MC Buff*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000D0000, 18432), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(20,4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000E0000, 0x000E0190)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000E8000, 0x000EB200)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             /*{DUMMY_NAME_PTR_CNS,            0x000a1000,         0x0000ffff,      1,    0x0         }*/

            {DUMMY_NAME_PTR_CNS,            0x000A0000,         0x0000C1A0,      1,    0x0                       }
           ,{DUMMY_NAME_PTR_CNS,            0x000A0450,         0x00009060,      1,    0x0                       }
           ,{DUMMY_NAME_PTR_CNS,            0x000A0800,         0x0000BFB0,      1,    0x0                       }
           ,{DUMMY_NAME_PTR_CNS,            0x000A0A00,         0x00000300,      8,    0x4                       }
           ,{DUMMY_NAME_PTR_CNS,            0x000A0A20,         0x000002BC,      8,    0x4                       }
           ,{NULL,                                   0,         0x00000000,      0,    0x0                       }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}


/**
* @internal smemAldrin2UnitHa function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the HA unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemAldrin2UnitHa
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            {DUMMY_NAME_PTR_CNS, 0x00000000, 0x00109010, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000004, 0x03FDD003, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000008, 0x00000004, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000070, 0x00040000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000080, 0xFF000000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000084, 0x00000001, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x0000008c, 0xFF020000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000090, 0xFFFFFFFF, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000094, 0xFFFFFFFF, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000098, 0xFFFFFFFF, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x0000009c, 0xFFFFFFFF, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x000003D0, 0x00000008, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000408, 0x81000000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x0000040c, 0x81000000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000410, 0x00010000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000424, 0x00110000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000500, 0x00008100, 8,     0x4}
           ,{DUMMY_NAME_PTR_CNS, 0x00000550, 0x88488847, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000554, 0x00008100, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000558, 0x000022F3, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x0000055C, 0x00006558, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000710, 0x0000FFFF, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000714, 0x0000003F, 1,     0x0}
           ,{NULL,               0,          0x00000000, 0,     0x0}
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemAldrin2SpecificDeviceUnitAlloc_DP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DP units -- (non sip units)
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemAldrin2SpecificDeviceUnitAlloc_DP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemAldrin2UnitGop(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_SERDES)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemAldrin2UnitLpSerdes(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_XG_PORT_MIB)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemAldrin2UnitXGPortMib(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_DMA)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemAldrin2UnitTxDma(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_RX_DMA)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemAldrin2UnitRxDma(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_FIFO)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemAldrin2UnitTxFifo(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_DQ)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemAldrin2UnitTxqDq(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_QUEUE)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemAldrin2UnitTxqQueue(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_HA)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemAldrin2UnitHa(devObjPtr,currUnitChunkPtr);
    }

    /* call BC3 to do the reset */
    smemBobcat3SpecificDeviceUnitAlloc_DP_units(devObjPtr);
}

/**
* @internal smemAldrin2UnitTcam function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the TCAM unit
*/
static void smemAldrin2UnitTcam
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x0000000f,  49152/2,    0x20     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000010,         0x0000000f,  49152/2,    0x20     }
            ,{DUMMY_NAME_PTR_CNS,            0x00500000,         0x000FFFFF,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00500004,         0x00000000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00500008,         0x00000f00,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0050000c,         0x0000f000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00500010,         0x000f0000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00500100,         0x00000000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00500108,         0x00000030,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00500110,         0x00000019,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00500118,         0x00000039,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00500120,         0x00000012,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00500128,         0x00000032,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00500130,         0x0000001b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00500138,         0x0000003b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00500140,         0x00000014,      2,    0x10     }
            ,{DUMMY_NAME_PTR_CNS,            0x00500148,         0x00000034,      2,    0x10     }
            ,{DUMMY_NAME_PTR_CNS,            0x00502304,         0x000000ff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00503000,         0x00000042,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00503008,         0x00001ff7,      2,    0xc      }
            ,{DUMMY_NAME_PTR_CNS,            0x0050300c,         0x00000001,      2,    0xc      }
            ,{DUMMY_NAME_PTR_CNS,            0x0050301c,         0x00000001,      2,    0xc      }
            ,{DUMMY_NAME_PTR_CNS,            0x00504010,         0x00000006,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00505004,         0x00000615,     20,    0x8      }
            ,{DUMMY_NAME_PTR_CNS,            0x00506000,         0x00000008,     20,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00506050,         0x03000000,      3,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x0050606c,         0x03000000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00506074,         0x0000eeee,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00507000,         0x24010302,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00507004,         0x00000202,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00507008,         0x0000000b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0050700C,         0x00000015,      1,    0x0      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}


/**
* @internal smemAldrin2UnitEgfSht function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EGF-SHT unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemAldrin2UnitEgfSht
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {

        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* L2 Port Isolation Table */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000000 ,34816), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128,16),SMEM_BIND_TABLE_MAC(l2PortIsolation)}
            /* Egress Spanning Tree State Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x02040000 , 65536), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128,16),SMEM_BIND_TABLE_MAC(egressStp)}
            /* Non Trunk Members 2 Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x02060000 , 65536 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128,16),SMEM_BIND_TABLE_MAC(nonTrunkMembers2)}
            /* Source ID Members Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x02080000 , 65536), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128,16),SMEM_BIND_TABLE_MAC(sst)}
            /* Eport EVlan Filter*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x020A0000, 131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64,8),SMEM_BIND_TABLE_MAC(egfShtEportEVlanFilter)}
            /* Multicast Groups Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x020C0000 , 65536), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128,16),SMEM_BIND_TABLE_MAC(mcast)}
             /* Device Map Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x02100000 ,  16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(16, 4),SMEM_BIND_TABLE_MAC(deviceMapTable)}
             /* Vid Mapper Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x02110000 ,  32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(egfShtVidMapper)}
             /* Designated Port Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x02120000 ,   2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128,16),SMEM_BIND_TABLE_MAC(designatedPorts)}
            /* Egress EPort table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x02200000 , 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(29, 4),SMEM_BIND_TABLE_MAC(egfShtEgressEPort)}
            /* Non Trunk Members Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x02400000 , 4096 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128,16),SMEM_BIND_TABLE_MAC(nonTrunkMembers)}
            /* Egress vlan table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x03000000 ,131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128,16),SMEM_BIND_TABLE_MAC(egressVlan)}
            /* EVlan Attribute table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x03400000 , 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(6, 4),SMEM_BIND_TABLE_MAC(egfShtEVlanAttribute)}
            /* EVlan Spanning table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x03440000 , 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(egfShtEVlanSpanning)}
            /* L3 Port Isolation Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x04000000 ,34816), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128,16),SMEM_BIND_TABLE_MAC(l3PortIsolation)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020000, 0x06020010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020020, 0x06020020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020030, 0x06020030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020040, 0x0602004C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020080, 0x0602008C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x060200C0, 0x060200CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020100, 0x0602010C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020140, 0x0602014C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020180, 0x0602018C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x060201C0, 0x060201CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020200, 0x0602021C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020280, 0x0602028C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x060202C0, 0x060202CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020300, 0x0602033C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020400, 0x060204FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020800, 0x0602080C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020880, 0x0602088C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x060208C0, 0x060208CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020900, 0x06020900)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x06020000,         0x00004007,         1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,           0x06020004,         0x00000001,         1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,           0x06020008,         0x0000000f,         1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,           0x06020030,         0xffff0000                      }
            ,{DUMMY_NAME_PTR_CNS,           0x06020040,         0xffffffff,         4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,           0x06020140,         0xffffffff,         4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,           0x060201c0,         0xffffffff,         4,    0x4   }
            /* Default value of 'UplinkIsTrunk - 0x1 = Trunk;' like in legacy devices */
            ,{DUMMY_NAME_PTR_CNS,           0x02100000,         0x00000002,   16384/4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,           0x02120000,         0xffffffff,    2048/4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,           0x020a0000,         0xffffffff,  131072/4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,           0x02080000,         0xffffffff,   65536/4,    0x4   }
            /* vidx 0xfff - 128 members */
            ,{DUMMY_NAME_PTR_CNS, 0x020C0000 + (0xfff * 0x10),  0xffffffff,         4,    0x4   }

            /* vlan 1 members */
            ,{DUMMY_NAME_PTR_CNS, 0x03000000 + (1 * 0x10),      0xffffffff,         4,    0x4   }

            ,{NULL,                                  0,         0x00000000,         0,    0x0   }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemAldrin2UnitEgfQag function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Aldrin2 EGF-QAG unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemAldrin2UnitEgfQag
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 524288),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(48, 8),SMEM_BIND_TABLE_MAC(egfQagEVlanDescriptorAssignmentAttributes)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00800000, 65536), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(44, 8),SMEM_BIND_TABLE_MAC(egfQagEgressEPort)}
            /* TC_DP_Mapper_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00900000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(8, 4),SMEM_BIND_TABLE_MAC(egfQagTcDpMapper)}
            /* VOQ_Mapper_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00910000, 8192), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(6, 4)}
            /* Cpu_Code_To_Loopback_Mapper_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00920000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(11, 4), SMEM_BIND_TABLE_MAC(egfQagCpuCodeToLbMapper)}
            /* Port_Target_Attributes_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00921000, 512), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(17, 4),SMEM_BIND_TABLE_MAC(egfQagPortTargetAttribute)}
            /* Port_Enq_Attributes_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00922000, 512), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(10, 4),SMEM_BIND_TABLE_MAC(egfQagTargetPortMapper)}
            /* Port_Source_Attributes_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00923000, 512), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(4, 4) ,SMEM_BIND_TABLE_MAC(egfQagPortSourceAttribute)}
            /* EVIDX_Activity_Status_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00924000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00E00000, 0x00E00008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00E00010, 0x00E00018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00E00020, 0x00E00020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00E00170, 0x00E0019C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00E00A00, 0x00E00A00)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00010, 0x00F00010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00020, 0x00F00020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00030, 0x00F00030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00100, 0x00F00114)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00200, 0x00F00200)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /*TC Loopback Configuration,*/
            {DUMMY_NAME_PTR_CNS,            0x00E00004,         0x00000006}
            /*VoQ assignment general configuration 1*/
            ,{DUMMY_NAME_PTR_CNS,           0x00E00020,         0x003AA0A0}
            /*QAG Metal Fix,*/
            ,{DUMMY_NAME_PTR_CNS,           0x00F00030,         0xFFFF0000}
            /*Unicast-Multicast Control,*/
            ,{DUMMY_NAME_PTR_CNS,           0x00F00100,         0x00041011}
            /*Unicast Weights,*/
            ,{DUMMY_NAME_PTR_CNS,           0x00F00104,         0x00008888}
            /*Multicast Weights,*/
            ,{DUMMY_NAME_PTR_CNS,           0x00F00108,         0x00008888}

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemAldrin2UnitIpvx function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the ipvx unit
*/
static void smemAldrin2UnitIpvx
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000010)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000028)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000104)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000204)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000250, 0x0000026C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000278, 0x00000294)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000360, 0x00000364)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000380, 0x00000380)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x00000924)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000940, 0x00000948)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000950, 0x00000954)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000968, 0x00000978)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000980, 0x00000984)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A24)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A80, 0x00000A84)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B24)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B80, 0x00000B84)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C24)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C80, 0x00000C84)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000E2C)}
        /* Router QoS Profile Offsets Table */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(ipvxQoSProfileOffsets)}
        /* Router Next Hop Table Age Bits */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00020000, 3072),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(routeNextHopAgeBits)}
        /* Router Acces Matrix Table */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000, 3072),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(ipvxAccessMatrix)}
        /* Router EVlan Table */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00100000, 131072),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(66, 16),SMEM_BIND_TABLE_MAC(ipvxIngressEVlan)}
        /* Router EPort Table */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00200000, 32768),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(112, 16),SMEM_BIND_TABLE_MAC(ipvxIngressEPort)}
        /* Router Next Hop Table */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00400000, 393216),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(106, 16),SMEM_BIND_TABLE_MAC(ipvxNextHop)}
        /* Router ECMP pointer Table */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00300000, 98304),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(15, 4),SMEM_BIND_TABLE_MAC(ipvxEcmpPointer)}
    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x00000000,         0x00000047,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000004,         0x017705dc,      4,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000020,         0x0380001c,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000024,         0x00000fff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000100,         0x1b79b01b,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000104,         0x001b665b,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000200,         0x1b79b01b,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000204,         0x0000661B,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000250,         0xffc0fe80,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000254,         0xfe00fc00,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000264,         0x00000001,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000268,         0x00000003,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000360,         0x00006140,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000364,         0xffffffff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000380,         0x001b9360,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000968,         0x0000ffff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000978,         0x99abadad,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e00,         0x88878685,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e04,         0x8c8b8a89,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e08,         0x9f8f8e8d,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e0c,         0xa3a2a1a0,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e10,         0xa7a6a5a4,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e14,         0xabaaa9a8,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e18,         0xafaeadac,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e1c,         0x90b6b1b0,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e20,         0x91b5b4b3,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e24,         0xcbcac9c8,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e28,         0x9392cdcc,      1,    0x0    }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemAldrin2UnitMll function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the MLL unit
*/
static void smemAldrin2UnitMll
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000030, 0x00000034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000040)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000050, 0x00000058)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000060, 0x00000064)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x00000080)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000210, 0x00000220)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x0000030C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000404)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000804)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x00000900)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000980, 0x00000984)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A00)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A80, 0x00000A84)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B04, 0x00000B04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C00)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C80, 0x00000C84)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D00)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D80, 0x00000D84)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000E08)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E20, 0x00000E28)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 65536), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(20, 4),SMEM_BIND_TABLE_MAC(l2MllLtt)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00100000, 524288), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(162, 32) , SMEM_BIND_TABLE_MAC(mll)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

}

/**
* @internal smemAldrin2SpecificDeviceUnitAlloc_SIP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         SIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemAldrin2SpecificDeviceUnitAlloc_SIP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    GT_U32  unitIndex;

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TCAM);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemAldrin2UnitTcam(devObjPtr, currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_EGF_SHT);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemAldrin2UnitEgfSht(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_EGF_QAG);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemAldrin2UnitEgfQag(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_IPVX);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemAldrin2UnitIpvx(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_MLL);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemAldrin2UnitMll(devObjPtr,currUnitChunkPtr);
    }

    /* call BC3 to do the reset */
    smemBobcat3SpecificDeviceUnitAlloc_SIP_units(devObjPtr);
}

/**
* @internal smemAldrin2SpecificDeviceUnitAlloc function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemAldrin2SpecificDeviceUnitAlloc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    GT_U32  unitIndex;

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[0];

    for (unitIndex = 0 ; unitIndex < SMEM_CHT_NUM_UNITS_MAX_CNS;
        unitIndex+=2,currUnitChunkPtr += 2)
    {
        currUnitChunkPtr->chunkType = SMEM_UNIT_CHUNK_TYPE_8_MSB_E;

        /* point the next 'half' unit on 'me' */
        currUnitChunkPtr[1].hugeUnitSupportPtr = currUnitChunkPtr;
        currUnitChunkPtr[1].numOfUnits = 1;
        currUnitChunkPtr[1].chunkType = SMEM_UNIT_CHUNK_TYPE_9_MSB_E;
    }

    {
        GT_U32  ii,jj;
        UNIT_INFO_STC   *unitInfoPtr = &aldrin2units[0];

        for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
        {
            jj = unitInfoPtr->base_addr >> SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS;

            currUnitChunkPtr = &devMemInfoPtr->unitMemArr[jj];

            if(currUnitChunkPtr->chunkIndex != jj)
            {
                skernelFatalError("smemAldrin2SpecificDeviceUnitAlloc : not matched index");
            }
            currUnitChunkPtr->numOfUnits = unitInfoPtr->size;
        }
    }

    /* allocate the specific units that we NOT want the bc2_init , lion3_init , lion2_init
       to allocate. */

    smemAldrin2SpecificDeviceUnitAlloc_DP_units(devObjPtr);

    smemAldrin2SpecificDeviceUnitAlloc_SIP_units(devObjPtr);
}

static void initPtpPort0(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPInterruptCause = 0x00180800;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPInterruptMask = 0x00180804;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPGeneralCtrl = 0x00180808;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPTXTimestampQueue0Reg0 = 0x0018080c;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPTXTimestampQueue0Reg1 = 0x00180810;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPTXTimestampQueue0Reg2 = 0x00180814;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPTXTimestampQueue1Reg0 = 0x00180818;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPTXTimestampQueue1Reg1 = 0x0018081c;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPTXTimestampQueue1Reg2 = 0x00180820;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].totalPTPPktsCntr = 0x00180824;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPv1PktCntr = 0x00180828;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPv2PktCntr = 0x0018082c;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].Y1731PktCntr = 0x00180830;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].NTPTsPktCntr = 0x00180834;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].NTPReceivePktCntr = 0x00180838;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].NTPTransmitPktCntr = 0x0018083c;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].WAMPPktCntr = 0x00180840;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].addCorrectedTimeActionPktCntr = 0x00180858;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].NTPPTPOffsetHigh = 0x00180870;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].noneActionPktCntr = 0x00180844;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].forwardActionPktCntr = 0x00180848;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].dropActionPktCntr = 0x0018084c;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].captureIngrTimeActionPktCntr = 0x0018086c;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].captureAddTimeActionPktCntr = 0x0018085c;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].captureAddIngrTimeActionPktCntr = 0x00180868;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].captureAddCorrectedTimeActionPktCntr = 0x00180860;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].captureActionPktCntr = 0x00180850;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].addTimeActionPktCntr = 0x00180854;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].addIngrTimeActionPktCntr = 0x00180864;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].NTPPTPOffsetLow = 0x00180874;
}
/**
* @internal smemAldrin2GopRegDbInit function
* @endinternal
*
* @brief   Init GOP regDB registers for Aldrin2.
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void smemAldrin2GopRegDbInit(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_SIP5_PP_REGS_ADDR_STC      *regAddrDbSip5Ptr;
    SMEM_CHT_PP_REGS_ADDR_STC        * regAddrDbPtr;
    GT_U32    offset;

    regAddrDbPtr     = SMEM_CHT_MAC_REG_DB_GET(devObjPtr);
    regAddrDbSip5Ptr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr);

    devObjPtr->portMacOffset = 0x1000;
    devObjPtr->portMacMask   = 0x7F;

    {/*start of unit SERDES */
        GT_U32    s,minus_S,globalSerdes;
        /******************************************/
        /*  handle memory space of SERDES CHAIN 0 */
        /******************************************/
        offset = 0;
        minus_S = 0;
        globalSerdes = minus_S;
        for(s = 0 ; s < SERDES_CHAIN_0_NUM_PORTS_CNS ; s++,globalSerdes++) {

            smemLion2RegsInfoSet_GOP_SERDES(devObjPtr, regAddrDbPtr, globalSerdes,minus_S, offset, GT_TRUE/*isGopVer1*/);

        }/* end of loop s */

        /******************************************/
        /*  handle memory space of SERDES CHAIN 1 */
        /******************************************/
        offset = CHAIN_1_SERDES_OFFSET;
        minus_S = SERDES_CHAIN_0_NUM_PORTS_CNS;
        globalSerdes = minus_S;
        for(s = 0 ; s < SERDES_CHAIN_1_NUM_PORTS_CNS ; s++,globalSerdes++) {

            smemLion2RegsInfoSet_GOP_SERDES(devObjPtr, regAddrDbPtr, globalSerdes,minus_S, offset, GT_TRUE/*isGopVer1*/);

        }/* end of loop s */
    }/*end of unit SERDES */

    {/*start of GOP units */
        GT_U32    p,minus_P,globalPort;

        /* must be called before smemLion2RegsInfoSet_GOP_PTP() */
        initPtpPort0(devObjPtr);

        /**************************************/
        /*  handle memory space of MG CHAIN 0 */
        /**************************************/
        offset = 0;
        minus_P = 0;
        globalPort = minus_P;
        for(p = 0 ; p < GOP_CHAIN_0_NUM_PORTS_CNS ; p++,globalPort++) {

            smemLion2RegsInfoSet_GOP_gigPort(devObjPtr,regAddrDbPtr,globalPort,minus_P ,offset,GT_TRUE/*isGopVer1*/);

            smemLion2RegsInfoSet_GOP_XLGIP(devObjPtr,regAddrDbPtr,globalPort,minus_P ,offset);

            smemLion2RegsInfoSet_GOP_MPCSIP(devObjPtr,regAddrDbPtr,globalPort,minus_P ,offset,GT_TRUE/*isGopVer1*/);

            smemLion2RegsInfoSet_GOP_PTP(devObjPtr,regAddrDbSip5Ptr,globalPort,minus_P ,offset);

        }
        /**************************************/
        /*  handle memory space of MG CHAIN 1 */
        /**************************************/
        offset = CHAIN_1_GOP_OFFSET;
        minus_P = GOP_CHAIN_0_NUM_PORTS_CNS;
        globalPort = minus_P;
        for(p = 0 ; p < GOP_CHAIN_1_NUM_PORTS_CNS ; p++,globalPort++) {

            smemLion2RegsInfoSet_GOP_gigPort(devObjPtr,regAddrDbPtr,globalPort,minus_P ,offset,GT_TRUE/*isGopVer1*/);

            smemLion2RegsInfoSet_GOP_XLGIP(devObjPtr,regAddrDbPtr,globalPort,minus_P ,offset);

            smemLion2RegsInfoSet_GOP_MPCSIP(devObjPtr,regAddrDbPtr,globalPort,minus_P ,offset,GT_TRUE/*isGopVer1*/);

            smemLion2RegsInfoSet_GOP_PTP(devObjPtr,regAddrDbSip5Ptr,globalPort,minus_P ,offset);
        }
        /************************************/
        /*  handle memory space of CPU port */
        /************************************/
        offset = CHAIN_1_GOP_OFFSET;
        /* we need (globalPort-minus_p) = 63 */
        minus_P = (CPU_PORT_CNS-GOP_CHAIN_1_CPU_PORT_CNS);
        globalPort = CPU_PORT_CNS;
        {
            smemLion2RegsInfoSet_GOP_gigPort(devObjPtr,regAddrDbPtr,globalPort,minus_P ,offset,GT_TRUE/*isGopVer1*/);

            smemLion2RegsInfoSet_GOP_XLGIP(devObjPtr,regAddrDbPtr,globalPort,minus_P ,offset);

            smemLion2RegsInfoSet_GOP_MPCSIP(devObjPtr,regAddrDbPtr,globalPort,minus_P ,offset,GT_TRUE/*isGopVer1*/);

            smemLion2RegsInfoSet_GOP_PTP(devObjPtr,regAddrDbSip5Ptr,globalPort,minus_P ,offset);
        }
    }/*end of unit GOP units */
}

/**
* @internal smemAldrin2GopPortByAddrGet function
* @endinternal
*
* @brief   get global MAC port id according to associated address (in GOP unit).
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] address                  - the  in the  space of the port
*
* @note the port id
*
*/
static GT_U32 smemAldrin2GopPortByAddrGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   address
)
{
    GT_U32 gopPortNum;

    gopPortNum = (address >> 12) & 0x3F;/* 'local to MG chain' port number is 0..63 */

    if(address & CHAIN_1_GOP_OFFSET)
    {
        /* MG chain 1*/
        if(gopPortNum == GOP_CHAIN_1_CPU_PORT_CNS)
        {
            gopPortNum = CPU_PORT_CNS;
        }
        else
        {
            gopPortNum += GOP_CHAIN_0_NUM_PORTS_CNS;
        }
    }

    return gopPortNum;
}

/**
* @internal smemAldrin2MibPortByAddrGet function
* @endinternal
*
* @brief   get global MIB port id according to associated address (in MSM unit).
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] address                  - the  in the MIB  space of the port
*
* @note the port id
*
*/
static GT_U32 smemAldrin2MibPortByAddrGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   address
)
{
    GT_U32 mibPortNum;

    mibPortNum = (address >> 10) & 0x3F;/* 'local to MG chain' port number is 0..63 */

    if(address & CHAIN_1_MIB_OFFSET_GIG)
    {
        /* MG chain 1*/
        if(mibPortNum == GOP_CHAIN_1_CPU_PORT_CNS)
        {
            mibPortNum = CPU_PORT_CNS;
        }
        else
        {
            mibPortNum += GOP_CHAIN_0_NUM_PORTS_CNS;
        }
    }

    return mibPortNum;
}

static void onEmulator_smemAldrin2Init
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemAldrin2Init function
* @endinternal
*
* @brief   Init memory module for a Aldrin2 device.
*/
void smemAldrin2Init
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  index,portNum,mibBaseAddr;
    GT_BOOL isAldrin2 = GT_FALSE;

    /* state the supported features */
    SMEM_CHT_IS_SIP5_25_GET(devObjPtr) = 1;

    if(devObjPtr->devMemUnitNameAndIndexPtr == NULL)
    {
        buildDevUnitAddr(devObjPtr);

        isAldrin2 = GT_TRUE;
    }

    if(devObjPtr->registersDefaultsPtr == NULL)
    {
        /*empty*/
    }

    if(devObjPtr->registersDefaultsPtr_unitsDuplications == NULL)
    {
        devObjPtr->registersDefaultsPtr_unitsDuplications = ALDRIN2_duplicatedUnits;
        devObjPtr->unitsDuplicationsPtr = ALDRIN2_duplicatedUnits;
    }

    if (isAldrin2 == GT_TRUE)
    {
        devObjPtr->devMemGopRegDbInitFuncPtr = smemAldrin2GopRegDbInit;

        devObjPtr->devIsOwnerMemFunPtr = NULL;/* all addresses */

        devObjPtr->tcamHardWiredInfo.isValid = 0;/*Client to Group connectivity flexible (in BC3 we limited the connectivity for BE reasons)*/

        /* state 'data path' structure */
        devObjPtr->multiDataPath.supportMultiDataPath =  1;
        devObjPtr->multiDataPath.maxDp     = 4;/* 4 DP units for the device */
        /* there is TXQ,dq per 'data path' */
        devObjPtr->multiDataPath.numTxqDq  = 4;
        devObjPtr->multiDataPath.txqDqNumPortsPerDp = 25;

        devObjPtr->multiDataPath.supportRelativePortNum = 1;

        devObjPtr->supportTrafficManager_notAllowed = 1;

        devObjPtr->dmaNumOfCpuPort = 74;/* (port on MG[0]) 'global' port in the egress RXDMA/TXDMA units (BC3 was 74)*/

        devObjPtr->numOfPipes = 0;/* must be 0 */

        devObjPtr->txqNumPorts =
            devObjPtr->multiDataPath.txqDqNumPortsPerDp *
            devObjPtr->multiDataPath.numTxqDq;/*the TXQ of ALDRIN2 support 100 ports */

        devObjPtr->multiDataPath.maxIa = 1;/* single IA */

        for(index = 0 ; index < devObjPtr->multiDataPath.numTxqDq ; index ++)
        {
            devObjPtr->multiDataPath.dqInfo[index].use_egressDpIndex = 1;
            /* in ALDRIN2  : DQ[0..3] to TXDMA[0..3]  (one to one) */
            devObjPtr->multiDataPath.dqInfo[index].egressDpIndex = index;
        }

        index = 0;
        devObjPtr->multiDataPath.info[index].dataPathFirstPort  = 0;
        devObjPtr->multiDataPath.info[index].dataPathNumOfPorts = 12;
        devObjPtr->multiDataPath.info[index].cpuPortDmaNum      = 23;/*local DP port number (23 !!! unlike DP[1..3])*/
        index++;/*1*/
        devObjPtr->multiDataPath.info[index].dataPathFirstPort  = 12;
        devObjPtr->multiDataPath.info[index].dataPathNumOfPorts = 12;
        devObjPtr->multiDataPath.info[index].cpuPortDmaNum      = 24;/*local DP port number*/
        index++;/*2*/
        devObjPtr->multiDataPath.info[index].dataPathFirstPort  = 24;
        devObjPtr->multiDataPath.info[index].dataPathNumOfPorts = 24;
        devObjPtr->multiDataPath.info[index].cpuPortDmaNum      = 24;/*local DP port number*/
        index++;/*3*/
        devObjPtr->multiDataPath.info[index].dataPathFirstPort  = 48;
        devObjPtr->multiDataPath.info[index].dataPathNumOfPorts = 24;
        devObjPtr->multiDataPath.info[index].cpuPortDmaNum      = 24;/*local DP port number*/
        index++;/*4*/


        devObjPtr->memUnitBaseAddrInfo.txqDq[0] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TXQ_DQ);
        devObjPtr->memUnitBaseAddrInfo.txqDq[1] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TXQ_DQ_1);
        devObjPtr->memUnitBaseAddrInfo.txqDq[2] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TXQ_DQ_2);
        devObjPtr->memUnitBaseAddrInfo.txqDq[3] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TXQ_DQ_3);

        devObjPtr->memUnitBaseAddrInfo.lpm[0] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_LPM);
        devObjPtr->memUnitBaseAddrInfo.lpm[1] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_LPM_1);

        /* TXQ DQ */
        devObjPtr->tablesInfo.Scheduler_State_Variable.commonInfo.multiInstanceInfo.numBaseAddresses = 4;
        devObjPtr->tablesInfo.Scheduler_State_Variable.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.txqDq[0];

        devObjPtr->tablesInfo.egressStc.commonInfo.multiInstanceInfo.numBaseAddresses = 4;
        devObjPtr->tablesInfo.egressStc.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.txqDq[0];

        INIT_LPM_UNIT_MULTI_INSTANCE_TABLES_MAC(lpmEcmp);
        INIT_LPM_UNIT_MULTI_INSTANCE_TABLES_MAC(lpmIpv4VrfId);
        INIT_LPM_UNIT_MULTI_INSTANCE_TABLES_MAC(lpmIpv6VrfId);
        INIT_LPM_UNIT_MULTI_INSTANCE_TABLES_MAC(lpmFcoeVrfId);

        devObjPtr->dma_specialPortMappingArr = ALDRIN2_DMA_specialPortMappingArr;
        devObjPtr->gop_specialPortMappingArr = NULL;/* not multi-pipe device */

        devObjPtr->tcam_numBanksForHitNumGranularity = 6; /* BC3 was 2  */
        devObjPtr->tcamNumOfFloors   = 6;                 /* BC3 was 12 */
        devObjPtr->portMacSecondBase = 0;
        devObjPtr->portMacSecondBaseFirstPort = 0;

        mibBaseAddr = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_XG_PORT_MIB);
        index = 0;
        for(portNum = 0 ; portNum < GOP_CHAIN_0_NUM_PORTS_CNS; portNum++,index++)
        {
            devObjPtr->portsArr[portNum].mibBaseAddr = mibBaseAddr + index*BOBCAT2_MIB_OFFSET_CNS;
        }

        mibBaseAddr += CHAIN_1_MIB_OFFSET_GIG;
        index = 0;
        for(/*continue portNum*/ ; portNum < (GOP_CHAIN_0_NUM_PORTS_CNS + GOP_CHAIN_1_NUM_PORTS_CNS); portNum++,index++)
        {
            devObjPtr->portsArr[portNum].mibBaseAddr = mibBaseAddr + index*BOBCAT2_MIB_OFFSET_CNS;
        }

        /************************************/
        /*  handle memory space of CPU port */
        /************************************/
        portNum = CPU_PORT_CNS;
        index = GOP_CHAIN_1_CPU_PORT_CNS;
        devObjPtr->portsArr[portNum].mibBaseAddr = mibBaseAddr + index*BOBCAT2_MIB_OFFSET_CNS;


        devObjPtr->devMemGopPortByAddrGetPtr = smemAldrin2GopPortByAddrGet;
        devObjPtr->devMemMibPortByAddrGetPtr = smemAldrin2MibPortByAddrGet;
    }

    {

        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.stgId,12); /* 4K*/
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.tunnelstartPtr , 16);/*64K*/

        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.phyPort , 9);
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.trunkId , 12);
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.ePort , 14);/*16K*/
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.eVid , 13);/*8K*/
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.eVidx , 14);/*16K*/
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.hwDevNum , 10);/*1K*/
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.sstId , 12 );/*4K*/
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.arpPtr ,  4 * devObjPtr->flexFieldNumBitsSupport.tunnelstartPtr);
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.fid , 13);


        SET_IF_ZERO_MAC(devObjPtr->limitedResources.eVid,1<<devObjPtr->flexFieldNumBitsSupport.eVid);
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.ePort,8*1024);/*16K in BC3*/
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.phyPort,128);/*512 in BC3*/
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.l2Ecmp,16*1024);/*same as BC3*/
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.nextHop,24*1024);/*same as BC3*/

        SET_IF_ZERO_MAC(devObjPtr->limitedResources.mllPairs,16*1024);/*32K in BC3*/
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.l2LttMll,32*1024);
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.l3LttMll,12*1024);/*routing ECMP table. 16K in BC3*/


        SET_IF_ZERO_MAC(devObjPtr->fdbMaxNumEntries , SMEM_MAC_TABLE_SIZE_128KB);/*256K in BC3*/
        SET_IF_ZERO_MAC(devObjPtr->lpmRam.perRamNumEntries , 8*1024);/*16K in BC3*/

        SET_IF_ZERO_MAC(devObjPtr->policerSupport.iplrTableSize   , (8 * _1K));/*16K in BC3*/

        SET_IF_ZERO_MAC(devObjPtr->oamNumEntries,8*_1K);

        SET_IF_ZERO_MAC(devObjPtr->cncClientSupportBitmap,SNET_SIP5_20_CNC_CLIENTS_BMP_ALL_CNS);

        SET_IF_ZERO_MAC(devObjPtr->ipvxEcmpIndirectMaxNumEntries , 24*1024);
    }

    /* function will be called from inside smemLion2AllocSpecMemory(...) */
    if(devObjPtr->devMemSpecificDeviceUnitAlloc == NULL)
    {
        devObjPtr->devMemSpecificDeviceUnitAlloc = smemAldrin2SpecificDeviceUnitAlloc;
    }

    /* function will be called from inside smemLion3Init(...) */
    if(devObjPtr->devMemSpecificDeviceMemInitPart2 == NULL)
    {
        devObjPtr->devMemSpecificDeviceMemInitPart2 = NULL;/* let BC3 set it's function */
    }

    if(devObjPtr->devMemInterruptTreeInit == NULL)
    {
        devObjPtr->devMemInterruptTreeInit = smemAldrin2InterruptTreeInit;
    }


    smemBobcat3Init(devObjPtr);

    if(simulationCheck_onEmulator() && isAldrin2 == GT_TRUE)
    {
        /* remove units/memories that not exists */
        onEmulator_smemAldrin2Init(devObjPtr);
    }

}

/**
* @internal smemAldrin2Init2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemAldrin2Init2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    smemBobcat3Init2(devObjPtr);
}

/**
* @internal onEmulator_smemAldrin2Init function
* @endinternal
*
* @brief   Aldrin2 'on emulator' - remove the units that not exists in the Emulator.
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void onEmulator_smemAldrin2Init
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    GT_U32  unitIndex;
#if 0
    GT_U32  sizePerPort;
#endif /*0*/
    static SMEM_REGISTER_DEFAULT_VALUE_STC nonExistsUnit_registersDefaultValueArr[] =
    {
         {NULL,            0,         0x00000000,      0,    0x0      }
    };
    static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC nonExistsUnit_defaults = {nonExistsUnit_registersDefaultValueArr,NULL};

    /*****************************************/
    /* remote units/memories that not exists */
    /*****************************************/
    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP_SMI_0);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    currUnitChunkPtr->numOfChunks = 0;
    currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP_SMI_1);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    currUnitChunkPtr->numOfChunks = 0;
    currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP_SMI_2);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    currUnitChunkPtr->numOfChunks = 0;
    currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP_SMI_3);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    currUnitChunkPtr->numOfChunks = 0;
    currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;

#if 0 /*allow to access : GOP/<SD_WRAPPER> SD_IP Units<<%s>>*/
    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_SERDES);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;
    currUnitChunkPtr->numOfChunks = 0;
    currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;
    /*onEmulator_UnitMemoryBindToChunk(devObjPtr,currUnitChunkPtr);*/

    sizePerPort = sizeof(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.SERDESes.SERDES[0]);
    sizePerPort /= 4;
    /* reset ALL ports (73 ports) */
    resetRegDb((GT_U32*)&(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.SERDESes.SERDES[0]),
        sizePerPort*(CPU_PORT_CNS + 1) , GT_FALSE);
#endif /*0*/

}
