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
* @file smemBobcat3.c
*
* @brief Bobcat3 memory mapping implementation
*
* @version   1
********************************************************************************
*/

#include <asicSimulation/SKernel/smem/smemBobcat3.h>
#include <asicSimulation/SKernel/cheetahCommon/sregLion2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregBobcat2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <cm3BootChannel.h>


/* port per DP --without-- the CPU port */
#define PORTS_PER_DP_CNS    12

/* Pipe1 GOP offset */
#define BC3_PIPE1_GOP_OFFSET        0x00400000
/* Pipe1 MIB offset */
#define BC3_PIPE1_MIB_OFFSET_GIG    0x00800000
/* Pipe1 serdes offset */
#define BC3_PIPE1_SERDES_OFFSET     0x00080000

#define BC3_NUM_PIPES            2
#define BC3_NUM_GOP_PORTS_GIG   37 /*per pipe*/
#define BC3_NUM_GOP_PORTS_XLG   37 /*per pipe*/
#define BC3_NUM_SERDESES_CNS    37 /*per pipe*/
#define BC3_NUM_GOP_PORTS_CG_100G   9 /* per pipe , steps of 4 ports */

#define GIG_DUP_PIPE_AND_PORTS_CNS \
    BC3_NUM_PIPES, BC3_PIPE1_GOP_OFFSET, BC3_NUM_GOP_PORTS_GIG , 0x1000

#define XLG_DUP_PIPE_AND_PORTS_CNS \
    BC3_NUM_PIPES, BC3_PIPE1_GOP_OFFSET, BC3_NUM_GOP_PORTS_XLG , 0x1000

#define CG_100G_DUP_PIPE_AND_PORTS_CNS \
    BC3_NUM_PIPES, BC3_PIPE1_GOP_OFFSET, BC3_NUM_GOP_PORTS_CG_100G  , (4*0x1000)

#define FORMULA_TWO_PARAMETERS_GIG_DUP_PIPE_AND_PORTS_CNS \
    FORMULA_TWO_PARAMETERS(BC3_NUM_PIPES, BC3_PIPE1_GOP_OFFSET, BC3_NUM_GOP_PORTS_GIG , 0x1000)

#define FORMULA_TWO_PARAMETERS_XLG_DUP_PIPE_AND_PORTS_CNS \
    FORMULA_TWO_PARAMETERS(BC3_NUM_PIPES, BC3_PIPE1_GOP_OFFSET, BC3_NUM_GOP_PORTS_XLG , 0x1000)

#define FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS \
    FORMULA_TWO_PARAMETERS(BC3_NUM_PIPES, BC3_PIPE1_GOP_OFFSET, BC3_NUM_GOP_PORTS_CG_100G  , (4*0x1000))


extern void startSimulationLog(void);

/* Bobcat3 interrupt tree database */
static SKERNEL_INTERRUPT_REG_INFO_STC bobcat3InterruptsTreeDb[MAX_INTERRUPT_NODES];

/* there are no more than 88 units , without pipe 1 units !!! */
#define BOBCAT3_NUM_UNITS_WITHOUT_PIPE_1_E     88

/* indication of unit in pipe 1*/
#define PIPE_1_INDICATION_CNS (SMAIN_NOT_VALID_CNS-1)

/* not used memory */
#define DUMMY_UNITS_BASE_ADDR_CNS(index)              0x70000000 + UNIT_BASE_ADDR_MAC(2*index)

#define SHARED_BETWEEN_PIPE_0_AND_PIPE_1_INDICATION_CNS     DUMMY_NAME_PTR_CNS

typedef struct{
    GT_U32      base_addr;/* base address of unit*/
    GT_CHAR*    nameStr  ;/* name of unit */
    GT_U32      size;/* number of units (each unit is (1<<24) bytes size) */
    GT_CHAR*    orig_nameStr  ;/* when orig_nameStr is not NULL than the nameStr
                                  is name of the duplicated unit , and the
                                  orig_nameStr is original unit.

                                  BUT if SHARED_BETWEEN_PIPE_0_AND_PIPE_1_INDICATION_CNS meaning : shared unit between pipe 0 and pipe 1
                                  */
    GT_U32      pipeOffset;    /* pipe offset - zero indicates pipe 0, non-zero indicaTtes pipe 1 */
}UNIT_INFO_STC;

static GT_U32 unitPipe1OffsetLookUp[256] =
{
    0 /* set in runtime from bobcat3units[] */
};

#define UNIT_INFO_MAC(baseAddr,unitName) \
    UNIT_INFO_LARGE_MAC(baseAddr,unitName,1)

#define UNIT_INFO_LARGE_MAC(baseAddr,unitName,size) \
     {baseAddr , STR(unitName) , size}

/* unit in pipe 1 according to pipe 0 with pipe ofsset */
#define UNIT_INFO_PIPE_1_WITH_OFFSET_MAC(unitName, pipeOffset) \
     {PIPE_1_INDICATION_CNS , ADD_INSTANCE_OF_UNIT_TO_STR(STR(unitName),1) , 1 , STR(unitName), pipeOffset}

/* unit in pipe 1 according to pipe 0 */
#define UNIT_INFO_PIPE_1_MAC(unitName) \
     UNIT_INFO_PIPE_1_WITH_OFFSET_MAC(unitName, SECOND_PIPE_OFFSET_CNS)

#define UNIT_INFO_PIPE_1_LARGE_MAC(unitName,size) \
     {PIPE_1_INDICATION_CNS , ADD_INSTANCE_OF_UNIT_TO_STR(STR(unitName),1) , size , STR(unitName), SECOND_PIPE_OFFSET_CNS}


/* unit shared between pipe 0 and pipe 1 */
#define UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(baseAddr,unitName) \
     UNIT_INFO_LARGE_SHARED_PIPE_0_PIPE_1_MAC(baseAddr,unitName, 1)

/* large unit shared between pipe 0 and pipe 1 */
#define UNIT_INFO_LARGE_SHARED_PIPE_0_PIPE_1_MAC(baseAddr,unitName, size) \
     {baseAddr , STR(unitName) , size , SHARED_BETWEEN_PIPE_0_AND_PIPE_1_INDICATION_CNS, SECOND_PIPE_OFFSET_CNS}


#define INIT_LPM_UNIT_MULTI_INSTANCE_TABLES_MAC(tableName)\
    devObjPtr->tablesInfo.tableName.commonInfo.multiInstanceInfo.numBaseAddresses = 4;\
    devObjPtr->tablesInfo.tableName.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.lpm[0];

/* use 4 times the 'BOBCAT3_NUM_UNITS_WITHOUT_PIPE_1_E' because:
   2 times to support duplicated units from pipe 1
   2 time to support that each unit is '8 MSbits' and not '9 MSbits'
*/
static SMEM_UNIT_NAME_AND_INDEX_STC bobcat3UnitNameAndIndexArr[(4*BOBCAT3_NUM_UNITS_WITHOUT_PIPE_1_E)+1]=
{
    /* filled in runtime from bobcat3units[] */
    /* must be last */
    {NULL ,                                SMAIN_NOT_VALID_CNS                     }
};


/* the addresses of the units that the bobk uses */
static SMEM_UNIT_BASE_AND_SIZE_STC   bobcat3UsedUnitsAddressesArray[(4*BOBCAT3_NUM_UNITS_WITHOUT_PIPE_1_E)+1]=
{
    {0,0}    /* filled in runtime from bobcat3units[] */
};

/* the units of bobcat3 */
static UNIT_INFO_STC bobcat3units[(2*BOBCAT3_NUM_UNITS_WITHOUT_PIPE_1_E)+1] =
{
     UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(0x00000000,UNIT_MG                )
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
    ,UNIT_INFO_MAC(0x22000000,UNIT_IA                )
    ,UNIT_INFO_MAC(0x23000000,UNIT_FCU       )
    ,UNIT_INFO_MAC(0x24000000,UNIT_SBC       )
    ,UNIT_INFO_MAC(0x25000000,UNIT_NSEC       )
    ,UNIT_INFO_MAC(0x26000000,UNIT_NSEF       )
    ,UNIT_INFO_MAC(0x27000000,UNIT_GOP_LED_0           )
    ,UNIT_INFO_MAC(0x28000000,UNIT_GOP_LED_1           )
    ,UNIT_INFO_MAC(0x29000000,UNIT_GOP_SMI_0           )
    ,UNIT_INFO_MAC(0x2a000000,UNIT_GOP_SMI_1           )
    ,UNIT_INFO_MAC(0x57000000,UNIT_TAI                 )


    ,UNIT_INFO_LARGE_MAC(0x30000000,UNIT_EGF_SHT    , 8)    /* 1. was shared between pipes , but now it is 'per pipe'
                                                               2. was in 0x48000000*/

    /* shared units between pipe 0 and pipe 1 */
    ,UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(0x40000000,UNIT_BM                )
    ,UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(0x41000000,UNIT_BMA               )
    ,UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(0x42000000,UNIT_CPFC              )
    ,UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(0x43000000,UNIT_FDB               )
    ,UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(0x44000000,UNIT_LPM               )
    ,UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(0x45000000,UNIT_LPM_1             )
    ,UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(0x46000000,UNIT_LPM_2             )
    ,UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(0x47000000,UNIT_LPM_3             )
    ,UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(0x50000000,UNIT_EGF_QAG           )
    ,UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(0x51000000,UNIT_MPPM              )
    ,UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(0x52000000,UNIT_TCAM              )
    ,UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(0x53000000,UNIT_TXQ_LL            )
    ,UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(0x54000000,UNIT_TXQ_QCN           )
    ,UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(0x55000000,UNIT_TXQ_QUEUE         )
    ,UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(0x56000000,UNIT_TXQ_BMX           )

    ,UNIT_INFO_SHARED_PIPE_0_PIPE_1_MAC(DUMMY_UNITS_BASE_ADDR_CNS(0), UNIT_FDB_TABLE_0 )
    /*********** end of pipe 0 units and shared units **************/

    /*********** start of pipe 1 units **************/

    ,UNIT_INFO_PIPE_1_MAC(UNIT_TTI               )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_IPCL              )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_L2I               )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_IPVX              )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_IPLR              )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_IPLR1             )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_IOAM              )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_MLL               )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_EQ                )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_EGF_EFT           )

    ,UNIT_INFO_PIPE_1_MAC(UNIT_TXQ_DQ            )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_TXQ_DQ_1          )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_TXQ_DQ_2          )

    ,UNIT_INFO_PIPE_1_MAC(UNIT_CNC               )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_CNC_1             )

    ,UNIT_INFO_PIPE_1_WITH_OFFSET_MAC(UNIT_GOP,         BC3_PIPE1_GOP_OFFSET)
    ,UNIT_INFO_PIPE_1_WITH_OFFSET_MAC(UNIT_XG_PORT_MIB, BC3_PIPE1_MIB_OFFSET_GIG)
    ,UNIT_INFO_PIPE_1_WITH_OFFSET_MAC(UNIT_SERDES,      BC3_PIPE1_SERDES_OFFSET)
    ,UNIT_INFO_PIPE_1_MAC(UNIT_HA                )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_ERMRK             )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_EPCL              )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_EPLR              )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_EOAM              )

    ,UNIT_INFO_PIPE_1_MAC(UNIT_RX_DMA            )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_RX_DMA_1          )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_RX_DMA_2          )

    ,UNIT_INFO_PIPE_1_MAC(UNIT_TX_DMA            )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_TX_DMA_1          )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_TX_DMA_2          )

    ,UNIT_INFO_PIPE_1_MAC(UNIT_TX_FIFO           )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_TX_FIFO_1         )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_TX_FIFO_2         )

    ,UNIT_INFO_PIPE_1_MAC(UNIT_IA                )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_FCU       )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_SBC       )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_NSEC       )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_NSEF       )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_GOP_LED_0           )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_GOP_LED_1           )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_GOP_SMI_0           )
    ,UNIT_INFO_PIPE_1_MAC(UNIT_GOP_SMI_1           )
    ,UNIT_INFO_PIPE_1_WITH_OFFSET_MAC(UNIT_TAI,         0x01000000)

    ,UNIT_INFO_PIPE_1_LARGE_MAC(UNIT_EGF_SHT    , 8)


    /* must be last */
    ,{SMAIN_NOT_VALID_CNS,NULL,SMAIN_NOT_VALID_CNS}
};

/* NOTE: all units that are duplicated from pipe 0 to pipe 1 are added into this array in runtime !!!
    it is built from bobcat3units[].orig_nameStr*/
static SMEM_UNIT_DUPLICATION_INFO_STC BOBCAT3_duplicatedUnits[120] =
{
    {STR(UNIT_RX_DMA)  ,2}, /* 2 duplication of this unit */
        {STR(UNIT_RX_DMA_1)},
        {STR(UNIT_RX_DMA_2)},

    {STR(UNIT_TX_FIFO) ,2}, /* 2 duplication of this unit */
        {STR(UNIT_TX_FIFO_1)},
        {STR(UNIT_TX_FIFO_2)},

    {STR(UNIT_TX_DMA)  ,2}, /* 2 duplication of this unit */
        {STR(UNIT_TX_DMA_1)},
        {STR(UNIT_TX_DMA_2)},

    {STR(UNIT_TXQ_DQ)  ,2}, /* 2 duplication of this unit */
        {STR(UNIT_TXQ_DQ_1)},
        {STR(UNIT_TXQ_DQ_2)},

    {STR(UNIT_LPM)  ,3}, /* 3 duplication of this unit */
        {STR(UNIT_LPM_1)},
        {STR(UNIT_LPM_2)},
        {STR(UNIT_LPM_3)},

    /* start duplication on units of instance 0 , to instance 1 !!! */

    /* NOTE: all units that are duplicated from pipe 0 to pipe 1 are added into this array in runtime !!!
        it is built from bobcat3units[].orig_nameStr*/


    {NULL,0} /* must be last */
};

/* DMA : special ports mapping {global,local,DP}*/
static SPECIAL_PORT_MAPPING_CNS BOBCAT3_DMA_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
     {74/*global DMA port*/,12/*local DMA port*/,0/*DP[]*/}
    ,{75/*global DMA port*/,12/*local DMA port*/,1/*DP[]*/}
    ,{72/*global DMA port*/,12/*local DMA port*/,2/*DP[]*/}
    ,{76/*global DMA port*/,12/*local DMA port*/,3/*DP[]*/}
    ,{77/*global DMA port*/,12/*local DMA port*/,4/*DP[]*/}
    ,{73/*global DMA port*/,12/*local DMA port*/,5/*DP[]*/}

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

/* GOP : special ports mapping {global,local,pipe}*/
static SPECIAL_PORT_MAPPING_CNS BOBCAT3_GOP_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
     {72/*global GOP port*/,36/*local GOP port*/,0/*pipe*/}
    ,{73/*global GOP port*/,36/*local GOP port*/,1/*pipe*/}

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

typedef enum{
     UNIT_TYPE_NOT_VALID         = 0
    ,UNIT_TYPE_PIPE_0_ONLY_E
    ,UNIT_TYPE_PIPE_1_ONLY_E
    ,UNIT_TYPE_PIPE_0_AND_PIPE_1_E
}UNIT_TYPE_ENT;

/* array of info about '8 MSB' unit addresses */
static UNIT_TYPE_ENT  bobcat3UnitTypeArr[SMEM_CHT_NUM_UNITS_MAX_CNS / 2] =
{
    0 /* set in runtime from bobcat3units[] */
};


typedef struct{
    GT_BIT        tableOffsetValid;  /* use MACRO SMEM_BIND_TABLE_MAC */
    GT_U32        tableOffsetInBytes;/* use MACRO SMEM_BIND_TABLE_MAC */
    GT_U32        startAddr;/* init in run time : absolute start address of table (with 8 MSBits) */
    GT_U32        lastAddr; /* init in run time : absolute last  address of table (with 8 MSBits) */
    GT_U32        pipeOffset; /* init in run time : the offset of the pipe */
    UNIT_TYPE_ENT table_unitType_deviceAccess;/* for skernel access : indication that the is pipe0/1/shared*/
    UNIT_TYPE_ENT table_unitType_cpu_access;  /* for SCIB(CPU) access : indication that the is pipe0/1/shared*/
}SPECIAL_INSTANCE_TABLES_ARR_STC;

/* address belongs to 'Special tables' that number of instances
   different than the number on instances of the UNIT ! */
static SPECIAL_INSTANCE_TABLES_ARR_STC specialSingleInstanceTablesArr[] =
{ /*tableOffsetValid,tableOffsetInBytes*/     /*startAddr*/ /*lastAddr*/    /*table_unitType_deviceAccess*/  /*table_unitType_cpu_access*/
     {SMEM_BIND_TABLE_MAC(arp)/*tunnelStart*/       ,0           ,0          ,UNIT_TYPE_PIPE_0_AND_PIPE_1_E ,UNIT_TYPE_PIPE_0_AND_PIPE_1_E}
    /* must be last  */
    ,{0,0                                           ,0           ,0          ,0                             ,0}
};

#define BOBCAT3_PIPE_OFFSET_MAC(pipe)               0x80000000 * (pipe)

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
    FuncUnitsInterrupts_Lpm2_Int = 25,
    FuncUnitsInterrupts_Lpm3_Int = 26,
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
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.CNCInterruptSummaryCauseReg) , BOBCAT3_PIPE_OFFSET_MAC(1)},/*SMEM_CHT3_CNC_INTR_CAUSE_REG*/
   /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.CNCInterruptSummaryMaskReg) , BOBCAT3_PIPE_OFFSET_MAC(1)},
   /*myFatherInfo*/{
       /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Cnc0_Int ,
       /*interruptPtr*/&FuncUnitsInterruptsSummary1
       },
   /*isTriggeredByWrite*/ 1
   }
  ,{
   /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[1].globalRegs.CNCInterruptSummaryCauseReg) , BOBCAT3_PIPE_OFFSET_MAC(1)},/*SMEM_CHT3_CNC_INTR_CAUSE_REG*/
   /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[1].globalRegs.CNCInterruptSummaryMaskReg) , BOBCAT3_PIPE_OFFSET_MAC(1)},
   /*myFatherInfo*/{
       /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Cnc1_Int ,
       /*interruptPtr*/&FuncUnitsInterruptsSummary1
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
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EGF_eft.global.EFTInterruptsCause) , BOBCAT3_PIPE_OFFSET_MAC(1)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EGF_eft.global.EFTInterruptsMask) , BOBCAT3_PIPE_OFFSET_MAC(1)},
    /*myFatherInfo*/{
        /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Eft_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary1
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
   ,{
   /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[0].OAMUnitInterruptCause) , BOBCAT3_PIPE_OFFSET_MAC(1)},/*SMEM_LION2_OAM_INTR_CAUSE_REG*/
   /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[0].OAMUnitInterruptMask)  , BOBCAT3_PIPE_OFFSET_MAC(1)},
   /*myFatherInfo*/{
       /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Ioam_Int ,
       /*interruptPtr*/&FuncUnitsInterruptsSummary1
       },
   /*isTriggeredByWrite*/ 1
    }
   ,{
   /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[1].OAMUnitInterruptCause) , BOBCAT3_PIPE_OFFSET_MAC(1)},/*SMEM_LION2_OAM_INTR_CAUSE_REG*/
   /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[1].OAMUnitInterruptMask) , BOBCAT3_PIPE_OFFSET_MAC(1)},
   /*myFatherInfo*/{
       /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Eoam_Int ,
       /*interruptPtr*/&FuncUnitsInterruptsSummary1
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
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCL.PCLUnitInterruptCause) , BOBCAT3_PIPE_OFFSET_MAC(1)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCL.PCLUnitInterruptMask) , BOBCAT3_PIPE_OFFSET_MAC(1)},
    /*myFatherInfo*/{
        /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Pcl_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary1
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
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TTI.TTIEngine.TTIEngineInterruptCause) , BOBCAT3_PIPE_OFFSET_MAC(1)},/*SMEM_LION3_TTI_INTERRUPT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TTI.TTIEngine.TTIEngineInterruptMask) , BOBCAT3_PIPE_OFFSET_MAC(1)},
    /*myFatherInfo*/{
        /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Tti_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary1
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
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(L2I.bridgeEngineInterrupts.bridgeInterruptCause) , BOBCAT3_PIPE_OFFSET_MAC(1)},/*SMEM_CHT_BRIDGE_INT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(L2I.bridgeEngineInterrupts.bridgeInterruptMask) , BOBCAT3_PIPE_OFFSET_MAC(1)},
    /*myFatherInfo*/{
        /*myBitIndex*/Func1UnitsInterrupts_Pipe1_L2I_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary1
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
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(HA.HAInterruptCause) , BOBCAT3_PIPE_OFFSET_MAC(1)},/*SMEM_LION3_HA_INTERRUPT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(HA.HAInterruptMask) , BOBCAT3_PIPE_OFFSET_MAC(1)},
    /*myFatherInfo*/{
        /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Ha_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary1
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
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(ERMRK.ERMRKInterruptCause) , BOBCAT3_PIPE_OFFSET_MAC(1)},/*SMEM_LION3_ERMRK_INTERRUPT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(ERMRK.ERMRKInterruptMask) , BOBCAT3_PIPE_OFFSET_MAC(1)},
    /*myFatherInfo*/{
        /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Ermrk_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary1
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
   ,{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.preEgrInterrupt.preEgrInterruptSummary) , BOBCAT3_PIPE_OFFSET_MAC(1)}, /*SMEM_CHT_EQ_INT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.preEgrInterrupt.preEgrInterruptSummaryMask) , BOBCAT3_PIPE_OFFSET_MAC(1)},  /*SMEM_CHT_EQ_INT_MASK_REG*/
    /*myFatherInfo*/{
        /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Eq_Int ,
        /*interruptPtr*/&FuncUnitsInterruptsSummary1
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
    ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.criticalECCCntrs.BMCriticalECCInterruptCause) , BOBCAT3_PIPE_OFFSET_MAC(1)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.criticalECCCntrs.BMCriticalECCInterruptMask) , BOBCAT3_PIPE_OFFSET_MAC(1)},
        /*myFatherInfo*/{
            /*myBitIndex*/3 ,
            /*interruptPtr*/&preEgrInterruptSummary[1]
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
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptSummary) , BOBCAT3_PIPE_OFFSET_MAC(1)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptSummaryMask) , BOBCAT3_PIPE_OFFSET_MAC(1)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/2 ,
            /*interruptPtr*/&preEgrInterruptSummary[1]
            },
        /*isTriggeredByWrite*/ 1
    }
};

#define CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(n, pipe)                                                                        \
{                                                                                                                              \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptCause[n-1])},    \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptMask[n-1])},     \
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
            /*myBitIndex*/FuncUnitsInterrupts_Pipe0_Mll_Int ,
            /*interruptPtr*/&FuncUnitsInterruptsSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MLL.MLLGlobalCtrl.MLLInterruptCauseReg) , BOBCAT3_PIPE_OFFSET_MAC(1)},/*SMEM_LION3_MLL_INTERRUPT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MLL.MLLGlobalCtrl.MLLInterruptMaskReg) , BOBCAT3_PIPE_OFFSET_MAC(1)},
        /*myFatherInfo*/{
            /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Mll_Int ,
            /*interruptPtr*/&FuncUnitsInterruptsSummary1
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
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[0].policerInterruptCause) , BOBCAT3_PIPE_OFFSET_MAC(1)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[0].policerInterruptMask) , BOBCAT3_PIPE_OFFSET_MAC(1)},
       /*myFatherInfo*/{
           /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Iplr0_Int ,
           /*interruptPtr*/&FuncUnitsInterruptsSummary1
           },
       /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[1].policerInterruptCause) , BOBCAT3_PIPE_OFFSET_MAC(1)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[1].policerInterruptMask) , BOBCAT3_PIPE_OFFSET_MAC(1)},
       /*myFatherInfo*/{
           /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Iplr1_Int ,
           /*interruptPtr*/&FuncUnitsInterruptsSummary1
           },
       /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[2].policerInterruptCause) , BOBCAT3_PIPE_OFFSET_MAC(1)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[2].policerInterruptMask) , BOBCAT3_PIPE_OFFSET_MAC(1)},
       /*myFatherInfo*/{
           /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Eplr_Int ,
           /*interruptPtr*/&FuncUnitsInterruptsSummary1
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
  ,{
   /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[4].global.interrupt.txQDQInterruptSummaryCause)},/*SMEM_CHT_TX_QUEUE_INTERRUPT_CAUSE_REG , SMEM_LION_TXQ_DQ_INTR_SUM_CAUSE_REG*/
   /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[4].global.interrupt.txQDQInterruptSummaryMask)},
   /*myFatherInfo*/{
       /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Txq1_Dq_Int ,
       /*interruptPtr*/&FuncUnitsInterruptsSummary1
       },
   /*isTriggeredByWrite*/ 1
   }
  ,{
   /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[5].global.interrupt.txQDQInterruptSummaryCause)},/*SMEM_CHT_TX_QUEUE_INTERRUPT_CAUSE_REG , SMEM_LION_TXQ_DQ_INTR_SUM_CAUSE_REG*/
   /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(SIP5_TXQ_DQ[5].global.interrupt.txQDQInterruptSummaryMask)},
   /*myFatherInfo*/{
       /*myBitIndex*/Func1UnitsInterrupts_Pipe1_Txq2_Dq_Int ,
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
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(4, 0)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(4, 1)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(4, 2)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(4, 3)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(5, 0)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(5, 1)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(5, 2)
    ,EGR_STC_INTERRUPT_CAUSE_PORTS_MAC(5, 3)
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
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(4, 0)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(4, 1)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(4, 2)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(4, 3)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(5, 0)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(5, 1)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(5, 2)
    ,FLUSH_DONE_INTERRUPT_CAUSE_PORTS_MAC(5, 3)
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
    TXQ_DQ_GENERAL_CAUSE_MAC(3),
    TXQ_DQ_GENERAL_CAUSE_MAC(4),
    TXQ_DQ_GENERAL_CAUSE_MAC(5)
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
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.ingrSTCInterrupt.ingrSTCInterruptSummary) , BOBCAT3_PIPE_OFFSET_MAC(1)}, /*SMEM_LION3_EQ_INGRESS_STC_INT_SUMMARY_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.ingrSTCInterrupt.ingrSTCInterruptSummaryMask) , BOBCAT3_PIPE_OFFSET_MAC(1)},  /*SMEM_LION3_EQ_INGRESS_STC_INT_SUMMARY_MASK_REG*/
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&preEgrInterruptSummary[1]
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC  ingrSTCInterruptSummary1[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.ingrSTCInterrupt.ingrSTCInterruptSummary1)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.ingrSTCInterrupt.ingrSTCInterruptSummary1Mask)},
        /*myFatherInfo*/{
            /*myBitIndex*/5 ,
            /*interruptPtr*/&preEgrInterruptSummary[0]
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.ingrSTCInterrupt.ingrSTCInterruptSummary1), BOBCAT3_PIPE_OFFSET_MAC(1)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.ingrSTCInterrupt.ingrSTCInterruptSummary1Mask), BOBCAT3_PIPE_OFFSET_MAC(1)},
        /*myFatherInfo*/{
            /*myBitIndex*/5 ,
            /*interruptPtr*/&preEgrInterruptSummary[1]
            },
        /*isTriggeredByWrite*/ 1
    }
};

/* node for ingress STC summary of 16 ports */
#define  INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(index, pipe) \
{                                                       \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.ingrSTCInterrupt.ingrSTCInterruptCause[index]) , BOBCAT3_PIPE_OFFSET_MAC(pipe)}, /*SMEM_CHT_INGRESS_STC_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.ingrSTCInterrupt.ingrSTCInterruptMask[index]) , BOBCAT3_PIPE_OFFSET_MAC(pipe)},  /*SMEM_CHT_INGRESS_STC_INT_MASK_REG*/  \
    /*myFatherInfo*/{                                     \
        /*myBitIndex*/((index) % 16+1) ,                               \
            /*interruptPtr*/ ((index) < 16 ? &ingrSTCInterruptSummary[pipe] : &ingrSTCInterruptSummary1[pipe]), \
        },                                                \
    /*isTriggeredByWrite*/ 1                              \
}

static SKERNEL_INTERRUPT_REG_INFO_STC ingr_stc_ports_interrupts[] =
{
    /* ingress stc - support 512 physical ports of EQ */
     INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 0, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 1, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 2, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 3, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 4, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 5, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 6, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 7, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 8, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 9, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(10, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(11, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(12, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(13, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(14, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(15, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(16, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(17, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(18, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(19, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(20, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(21, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(22, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(23, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(24, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(25, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(26, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(27, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(28, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(29, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(30, 0)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(31, 0)

    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 0, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 1, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 2, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 3, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 4, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 5, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 6, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 7, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 8, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC( 9, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(10, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(11, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(12, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(13, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(14, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(15, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(16, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(17, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(18, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(19, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(20, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(21, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(22, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(23, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(24, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(25, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(26, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(27, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(28, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(29, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(30, 1)
    ,INGR_STC_INTERRUPT_CAUSE_FOR_16_PORTS_MAC(31, 1)
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
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[4].interrupts.rxDMAInterruptSummaryCause)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[4].interrupts.rxDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/11 ,
            /*interruptPtr*/&dataPathInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[5].interrupts.rxDMAInterruptSummaryCause)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(rxDMA[5].interrupts.rxDMAInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/12 ,
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
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[4].interrupts.txDMAInterruptSummary)},/**/
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[4].interrupts.txDMAInterruptSummaryMask)},/**/
       /*myFatherInfo*/{
           /*myBitIndex*/14 ,
           /*interruptPtr*/&dataPathInterruptSummary
           },
       /*isTriggeredByWrite*/ 1
    }
   ,{
       /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[5].interrupts.txDMAInterruptSummary)},/**/
       /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXDMA[5].interrupts.txDMAInterruptSummaryMask)},/**/
       /*myFatherInfo*/{
           /*myBitIndex*/15 ,
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
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[4].interrupts.txFIFOInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[4].interrupts.txFIFOInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
           /*myBitIndex*/17 ,
           /*interruptPtr*/&dataPathInterruptSummary
           },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[5].interrupts.txFIFOInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TXFIFO[5].interrupts.txFIFOInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
           /*myBitIndex*/18 ,
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
DEFINE_XLG_SUMMARY_PORT_62_73_INTERRUPT_SUMMARY_MAC(73);


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
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(16)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(17)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(18)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(19)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(20)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(21)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(22)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(23)
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
    /* the ports 0..35 pipe 0 */
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
    /* the port 36 pipe 0 */
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(72)
    /* the port 36 pipe 1 */
    ,XLG_MAC_INTERRUPT_CAUSE_MAC(73)
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

/* define the CG-100G port summary, for port 31..35 in ports_1_int_sum per pipe 0  */
#define  DEFINE_CG_100G_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(port) \
    DEFINE_CG_100G_EXTERNAL_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(port, ((port - 31) + 1)/*bitInMg*/,ports1InterruptsSummary/*regInMg*/)

/* define the CG-100G port summary, for port 26..35 in ports_2_int_sum per pipe 1 */
#define  DEFINE_CG_100G_SUMMARY_PORT_62_71_INTERRUPT_SUMMARY_MAC(port) \
    DEFINE_CG_100G_EXTERNAL_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(port, ((port - 62) + 1)/*bitInMg*/,ports2InterruptsSummary/*regInMg*/)

DEFINE_CG_100G_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC( 0);
DEFINE_CG_100G_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC( 4);
DEFINE_CG_100G_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC( 8);
DEFINE_CG_100G_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC(12);
DEFINE_CG_100G_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC(16);
DEFINE_CG_100G_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC(20);
DEFINE_CG_100G_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC(24);
DEFINE_CG_100G_SUMMARY_PORT_0_30_INTERRUPT_SUMMARY_MAC(28);
DEFINE_CG_100G_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(32);
DEFINE_CG_100G_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(36);
DEFINE_CG_100G_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(40);
DEFINE_CG_100G_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(44);
DEFINE_CG_100G_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(48);
DEFINE_CG_100G_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(52);
DEFINE_CG_100G_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(56);
DEFINE_CG_100G_SUMMARY_PORT_31_61_INTERRUPT_SUMMARY_MAC(60);
DEFINE_CG_100G_SUMMARY_PORT_62_71_INTERRUPT_SUMMARY_MAC(64);
DEFINE_CG_100G_SUMMARY_PORT_62_71_INTERRUPT_SUMMARY_MAC(68);


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
    /* the CG ports 0..35 pipe 0 , steps of 4 */
     CG_MAC_100G_INTERRUPT_CAUSE_MAC( 0)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC( 4)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC( 8)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(12)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(16)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(20)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(24)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(28)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(32)
    /* the CG ports 0..35 pipe 1 , steps of 4 */
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(36)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(40)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(44)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(48)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(52)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(56)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(60)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(64)
    ,CG_MAC_100G_INTERRUPT_CAUSE_MAC(68)
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
    ,GOP_PTP_PORT_XLG_GIGA_MAC_INTERRUPT_CAUSE_MAC(73)
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
    /* the Giga ports 0..71 */
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
    ,GIGA_MAC_INTERRUPT_CAUSE_MAC(73)
};

/**
* @internal smemBobcat3UnitPipeOffsetGet function
* @endinternal
*
* @brief   Get pipe offset for unit register address in data base.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] regAddress               - unit register lookup addres
*
* @param[out] pipeIndexPtr             - (pointer to) the pipe index that hold the 'regAddress'
*                                      ignored if NULL
*
* @retval Pipe offset !== 0        - pipe1 offset
* @retval == 0                     - pipe1 offset not found (should not happen)
*/
static GT_U32 smemBobcat3UnitPipeOffsetGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  regAddress,
    IN GT_U32                      *pipeIndexPtr
)
{
    GT_U32   unitPipe1Offset;
    GT_U32   unitIndex;

    unitIndex = (regAddress >> (SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS + 1));
    unitPipe1Offset = unitPipe1OffsetLookUp[unitIndex];

    if(pipeIndexPtr)
    {
        *pipeIndexPtr = 0;
    }

    if (unitPipe1Offset)
    {
        if(pipeIndexPtr)
        {
            *pipeIndexPtr = (regAddress & unitPipe1Offset) ? 1 : 0;
        }

        return unitPipe1Offset;
    }

    /* support NOT valid address if expected */
    if (regAddress == SMAIN_NOT_VALID_CNS)
    {
        return SECOND_PIPE_OFFSET_CNS;
    }

    skernelFatalError("smemBobcat3UnitPipeOffsetGet: not found pipe offset for register [0x%x]",
        regAddress);

    return 0;
}

static GT_U32 findBaseAddrOfUnit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN UNIT_INFO_STC   *unitsInfoPtr,
    IN GT_CHAR*         unitNamePtr
)
{
    GT_U32  ii;
    UNIT_INFO_STC   *unitInfoPtr = &unitsInfoPtr[0];

    for(ii = 0 ; unitInfoPtr->base_addr != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
    {
        if(0 == strcmp(unitNamePtr,unitInfoPtr->nameStr))
        {
            return unitInfoPtr->base_addr;
        }
    }

    skernelFatalError("findBaseAddrOfUnit : not found unit named [%s]",
        unitNamePtr);

    return 0;
}

static void addDuplicatedUnitIfNotExists
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_CHAR*         origUnitNamePtr,
    IN GT_CHAR*         dupUnitNamePtr
)
{
    GT_U32  ii,jj;
    SMEM_UNIT_DUPLICATION_INFO_STC   *unitInfoPtr_1 = &BOBCAT3_duplicatedUnits[0];
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
    if((ii+2+1) >= (sizeof(BOBCAT3_duplicatedUnits)/sizeof(BOBCAT3_duplicatedUnits[0])))
    {
        skernelFatalError("addDuplicatedUnitIfNotExists : array BOBCAT3_duplicatedUnits[] not large enough \n");
        return;
    }

    BOBCAT3_duplicatedUnits[ii].unitNameStr = origUnitNamePtr;
    BOBCAT3_duplicatedUnits[ii].numOfUnits = 1;

    BOBCAT3_duplicatedUnits[ii+1].unitNameStr = dupUnitNamePtr;
    BOBCAT3_duplicatedUnits[ii+1].numOfUnits = 0;

    BOBCAT3_duplicatedUnits[ii+2].unitNameStr = NULL;
    BOBCAT3_duplicatedUnits[ii+2].numOfUnits = 0;

    return ;
}


static void buildDevUnitAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /* build
        bobcat3UsedUnitsAddressesArray - the addresses of the units that the bobk uses
        bobcat3UnitNameAndIndexArr - name of unit and index in bobcat3UsedUnitsAddressesArray */
    GT_U32  ii,jj;
    GT_U32  index;
    GT_U32  size;
    UNIT_INFO_STC   *unitInfoPtr = &bobcat3units[0];
    GT_U32  unitIndex, unit1Index;
    GT_U32  currentUnit;

    index = 0;
    for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
    {
        if(unitInfoPtr->orig_nameStr == SHARED_BETWEEN_PIPE_0_AND_PIPE_1_INDICATION_CNS)
        {
            unitIndex = unitInfoPtr->base_addr >> (SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS + 1);
            /* set info to pipe0 unit */
            unitPipe1OffsetLookUp[unitIndex] = unitInfoPtr->pipeOffset;
        }
        else
        if(unitInfoPtr->orig_nameStr)
        {
            if(unitInfoPtr->base_addr == PIPE_1_INDICATION_CNS)
            {
                /* need to get address from 'orig_nameStr'
                    and add to it the 'pipe 1' offset
                */
                GT_U32 baseAddr = findBaseAddrOfUnit(devObjPtr,bobcat3units,unitInfoPtr->orig_nameStr);

                /* update the DB */
                unitInfoPtr->base_addr = baseAddr + unitInfoPtr->pipeOffset;

                unitIndex = baseAddr >> (SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS + 1);
                unit1Index = (unitInfoPtr->base_addr >> (SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS + 1));
                for (currentUnit = 0; currentUnit < unitInfoPtr->size; currentUnit++)
                {
                    /* fill in  DB lookup table with unit pipe11 info */
                    unitPipe1OffsetLookUp[unit1Index + currentUnit] = unitInfoPtr->pipeOffset;
                    /* set info to pipe0 unit */
                    unitPipe1OffsetLookUp[unitIndex + currentUnit] = unitInfoPtr->pipeOffset;
                }

                /* check if duplicated unit already in BOBCAT3_duplicatedUnits[] */
            }

            addDuplicatedUnitIfNotExists(devObjPtr, unitInfoPtr->orig_nameStr, unitInfoPtr->nameStr);
        }

        /* the size is in '1<<24' but the arrays are units of '1<<23' so we do
            lopp on : *2 */
        size = unitInfoPtr->size*2;
        for(jj = 0 ; jj < size ; jj++ , index++)
        {
            if(index >= (sizeof(bobcat3UsedUnitsAddressesArray) / sizeof(bobcat3UsedUnitsAddressesArray[0])))
            {
                skernelFatalError("buildDevUnitAddr : over flow of units (1) \n");
            }

            if(index >= (sizeof(bobcat3UnitNameAndIndexArr) / sizeof(bobcat3UnitNameAndIndexArr[0])))
            {
                skernelFatalError("buildDevUnitAddr : over flow of units (2) \n");
            }

            bobcat3UsedUnitsAddressesArray[index].unitBaseAddr = unitInfoPtr->base_addr ;
            bobcat3UsedUnitsAddressesArray[index].unitSizeInBytes = 0;
            bobcat3UnitNameAndIndexArr[index].unitNameIndex = index;
            bobcat3UnitNameAndIndexArr[index].unitNameStr = unitInfoPtr->nameStr;
        }
    }

    if(index >= (sizeof(bobcat3UnitNameAndIndexArr) / sizeof(bobcat3UnitNameAndIndexArr[0])))
    {
        skernelFatalError("buildDevUnitAddr : over flow of units (3) \n");
    }
    /* indication of no more */
    bobcat3UnitNameAndIndexArr[index].unitNameIndex = SMAIN_NOT_VALID_CNS;
    bobcat3UnitNameAndIndexArr[index].unitNameStr = NULL;

    devObjPtr->devMemUnitNameAndIndexPtr = bobcat3UnitNameAndIndexArr;
    devObjPtr->genericUsedUnitsAddressesArray = bobcat3UsedUnitsAddressesArray;
    devObjPtr->genericNumUnitsAddresses = index+1;
    devObjPtr->devMemUnitPipeOffsetGet = smemBobcat3UnitPipeOffsetGet;
}

static GT_VOID smemBobcat3InterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_INTERRUPT_REG_INFO_STC ** currDbPtrPtr
)
{
    GT_U32 index = 0;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr = bobcat3InterruptsTreeDb;

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

    *currDbPtrPtr = bobcat3InterruptsTreeDb;
}

/**
* @internal smemBobcat3ActiveWritePolicerManagementCounters function
* @endinternal
*
* @brief   in BC3 those counters are not accessible to the CPU !!!
*         do not allow to write to it , do not let read from it !!!
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemBobcat3ActiveWritePolicerManagementCounters
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    skernelFatalError("smemBobcat3ActiveWritePolicerManagementCounters : in BC3 management counters are not accessible to the CPU !!! "
        "do not allow to write to it , do not let read from it !!!");
}


/**
* @internal smemBobcat3UnitPolicerUnify function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the any of the 3 Policers:
*         1. iplr 0
*         2. iplr 1
*         3. eplr
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
* @param[in] plrUnit                  - PLR unit
*/
static void smemBobcat3UnitPolicerUnify
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr,
    IN SMEM_SIP5_PP_PLR_UNIT_ENT   plrUnit
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000060, 0x00000068)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x00000070)}
            /*registers -- not table/memory !! -- Policer Table Access Data<%n> */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000074 ,8*4),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerTblAccessData)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000B0, 0x000000B8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000C0, 0x000001BC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x0000020C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000220, 0x00000264)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000270, 0x000002EC)}
            /*Policer Timer Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000300, 36), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerTimer)}
            /*Policer Descriptor Sample Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000400, 96), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4)}
            /*Policer Management Counters Memory --> NOTE: those addresses are NOT accessible by CPU !!! */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000500, 192), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(74,16),SMEM_BIND_TABLE_MAC(policerManagementCounters)}
            /*IPFIX wrap around alert Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000800, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerIpfixWaAlert)}
            /*IPFIX aging alert Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerIpfixAgingAlert)}
            /*registers (not memory) : Port%p and Packet Type Translation Table*/
            /*registers -- not table/memory !! -- Port%p and Packet Type Translation Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800 , 0x00001FFC ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerMeterPointer)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002010, 0x00002014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002020, 0x00002058)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x000026FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003500, 0x00003504)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003510, 0x00003514)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003520, 0x00003524)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003550, 0x00003554)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003560, 0x00003564)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003604, 0x0000360C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003704, 0x0000370C)}
            /*e Attributes Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 262144 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(36,8),SMEM_BIND_TABLE_MAC(policerEPortEVlanTrigger)}
            /*Ingress Policer Re-Marking Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00080000, 8192 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(51,8),SMEM_BIND_TABLE_MAC(policerReMarking)}
            /*Metering Conformance Level Sign Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00400000, 65536 ) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(2,4),SMEM_BIND_TABLE_MAC(policerConformanceLevelSign)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemBobkPolicerTablesSupport(devObjPtr,numOfChunks,chunksMem,plrUnit);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    if(plrUnit == SMEM_SIP5_PP_PLR_UNIT_IPLR_0_E)
    {
        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /*Hierarchical Policing Table*/
                 {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00090000, 65536 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(18,4), SMEM_BIND_TABLE_MAC(policerHierarchicalQos)}
                /*Metering Token Bucket Memory*/
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00100000, 524288 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(211,32),SMEM_BIND_TABLE_MAC(policer)}
                /*Counting Memory*/
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00200000, 524288 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(229,32),SMEM_BIND_TABLE_MAC(policerCounters)}
                /*Metering Configuration Memory*/
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00300000, 131072 ) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(53,8),SMEM_BIND_TABLE_MAC(policerConfig)}
            };
            GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
            SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

            smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

            /*add the tmp unit chunks to the main unit */
            smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
        }
    }


}

static void smemBobcat3UnitIplr0
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

/*policer*/
    /* add common lines of all policers */
    ACTIVE_MEM_POLICER_BC3_COMMON_MAC(SMEM_SIP5_PP_PLR_UNIT_IPLR_0_E),

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    smemBobcat3UnitPolicerUnify(devObjPtr,unitPtr,SMEM_SIP5_PP_PLR_UNIT_IPLR_0_E);
}

static void smemBobcat3UnitIplr1
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

/*policer */
    /* add common lines of all policers */
    ACTIVE_MEM_POLICER_BC3_COMMON_MAC(SMEM_SIP5_PP_PLR_UNIT_IPLR_1_E),

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    smemBobcat3UnitPolicerUnify(devObjPtr,unitPtr,SMEM_SIP5_PP_PLR_UNIT_IPLR_1_E);
}

static void smemBobcat3UnitEplr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

/*policer*/
    /* add common lines of all policers */
    ACTIVE_MEM_POLICER_BC3_COMMON_MAC(SMEM_SIP5_PP_PLR_UNIT_EPLR_E),

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    smemBobcat3UnitPolicerUnify(devObjPtr,unitPtr,SMEM_SIP5_PP_PLR_UNIT_EPLR_E);
}

/**
* @internal smemBobcat3UnitMg function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitMg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    GT_U32  isSip6 = SMEM_CHT_IS_SIP6_GET(devObjPtr);
    GT_U32  isSip6_20 = SMEM_CHT_IS_SIP6_20_GET(devObjPtr);
    SMEM_ACTIVE_MEM_ENTRY_STC * tmp;
    GT_U32 bootChannelAdress;

    if (isSip6 == 0)
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr, unitPtr)

        ACTIVE_MEM_MG_COMMON_MAC ,

        /* XSMI Management Register */
        {0x00030000, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteXSmii, 0},
        /* XSMI1 Management Register */
        {0x00032000, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteXSmii, 0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }
    else
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr, unitPtr)

        ACTIVE_MEM_MG_COMMON_MAC ,

        /* XSMI Management Register */
        {0x00030000, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteXSmii, 0},
        /* XSMI1 Management Register */
        {0x00032000, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteXSmii, 0},

        /* read interrupts tree1 cause registers Misc -- ROC register */
        {0x00000618, SMEM_FULL_MASK_CNS,
            smemChtActiveReadIntrCauseReg, 11, smemChtActiveWriteIntrCauseReg, 1},
        /* Write Interrupt Tree1 Mask MIsc Register */
        {0x0000061c, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteMiscInterruptsMaskReg, 1},

        /* Write Interrupt Tree1 Mask FuncUnitsIntsSum Register */
        {0x00000650, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveGenericWriteInterruptsMaskReg, 0},
        /* Read/Write Interrupt Tree1 cause FuncUnitsIntsSum Register */
        {0x0000064C, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0 , smemChtActiveWriteIntrCauseReg, 0},

        /* Write Interrupt Tree1 Mask FuncUnits1IntsSum Register */
        {0x00000644, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveGenericWriteInterruptsMaskReg, 0},
        /* Read/Write Interrupt Tree1 cause FuncUnits1IntsSum Register */
        {0x00000648, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0 , smemChtActiveWriteIntrCauseReg, 0},

        /* Transmit SDMA Interrupt Tree1 Cause Register */
        {0x00002898, SMEM_FULL_MASK_CNS,
            smemChtActiveReadIntrCauseReg, 21 , smemChtActiveWriteIntrCauseReg,0},
        /* Transmit SDMA Interrupt Tree1 Mask Register */
        {0x000028A8, SMEM_FULL_MASK_CNS,
            NULL, 0, smemChtActiveWriteTransSdmaInterruptsMaskReg, 0},

        /* Receive SDMA Interrupt Cause Tree1 Register (RxSDMAInt) */
        {0x00002890, SMEM_FULL_MASK_CNS,
            smemChtActiveReadIntrCauseReg, 22, smemChtActiveWriteIntrCauseReg, 0},
        /* Receive SDMA Interrupt Tree1 Mask Register */
        {0x000028A0, SMEM_FULL_MASK_CNS,
            NULL, 0, smemChtActiveWriteRcvSdmaInterruptsMaskReg, 0},

         {0x00000500, SMEM_FULL_MASK_CNS,
            NULL, 0, smemFalconActiveWriteConfiProcessorGlobalConfiguration, 0},
         /*Memory used for IPC is last 2K of SRAM . SRAM is at 0x80000 .SRAM size is 0x60000 .
                    IPC start adress =  0x80000+ 0x60000 - 0x800=0xDF800 */
         {0x000DF800, SMEM_FULL_MASK_CNS,
            NULL, 0, smemFalconActiveWriteConfiProcessorIpcMemory, 0},


        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    if (isSip6_20 == 1)
    {
        tmp = unitPtr->unitActiveMemPtr;
        while(tmp->address!=END_OF_TABLE)
        {
            if(tmp->writeFun==smemFalconActiveWriteConfiProcessorIpcMemory)
            {
                bootChannelAdress = cm3SimBootChannelAdressGet(0);
                tmp->address &=0xFFF00000;
                tmp->address |=bootChannelAdress;
                break;
            }
            tmp++;
        }
    }

    if (isSip6 == 0)
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000008, 0x00000008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000010, 0x00000044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000004C, 0x00000068)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x00000084)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000090, 0x00000148)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000150, 0x0000017C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000190, 0x00000198)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001A0, 0x00000288)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000290, 0x00000298)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002A0, 0x000002A8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002B0, 0x000002B8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002C0, 0x000002C8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002D0, 0x00000324)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000330, 0x00000330)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000338, 0x00000338)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000340, 0x00000344)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000360, 24)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000380, 0x00000388)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000390, 0x00000390)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003A0, 0x000003C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003F0, 0x000003FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000480, 0x000004DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000540)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000604)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000610, 0x00000654)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000670, 0x000006B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x000009FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000b64, 0x00000b64)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000b78, 0x00000b7c)}
            /* sdma */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002600, 0x00002684)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000026C0, 0x000026DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002700, 0x00002708)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002710, 0x00002718)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002720, 0x00002728)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002730, 0x00002738)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002740, 0x00002748)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002750, 0x00002758)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002760, 0x00002768)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002770, 0x00002778)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002780, 0x00002780)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002800, 0x00002800)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000280C, 0x00002868)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002870, 0x000028F4)}
            /*xsmi*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00030000, 0x00030000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00030008, 0x00030014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00032000, 0x00032000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00032008, 0x00032014)}
            /* conf processor */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 131072)}
            /*TWSI*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00080000, 0x0008000C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0008001C, 0x0008001C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00091000, 0x00091010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0009101C, 0x0009101C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000910C0, 0x000910E0)}

            /* dummy memory for IPC between CPSS and WM , see smemChtActiveWriteIpcCpssToWm(...) */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000FFF00, 0x000FFFFC)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
    else
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000000)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000008, 0x00000008)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000010, 0x00000044)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000004C, 0x00000068)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x00000084)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000090, 0x00000148)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000150, 0x0000017C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000288)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002D0, 0x00000324)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000330, 0x00000338)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000340, 0x00000358)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000360, 24)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000380, 0x00000388)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000390, 0x00000390)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003A0, 0x000003C0)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003F0, 0x000003FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000400, 24)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000440, 0x0000044C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000480, 0x000004DC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000500)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000518, 0x0000051C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000520, 0x00000520)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000530, 0x00000540)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000604)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000610, 0x00000654)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000670, 0x000006B4)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000800, 512)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002600, 0x00002684)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000026C0, 0x000026DC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002700, 0x00002708)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002710, 0x00002718)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002720, 0x00002728)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002730, 0x00002738)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002740, 0x00002748)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002750, 0x00002758)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002760, 0x00002768)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002770, 0x00002778)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002780, 0x00002780)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002800, 0x00002800)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000280C, 0x00002868)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002870, 0x000028F4)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00030000, 0x00030000)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00030008, 0x00030014)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00032000, 0x00032000)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00032008, 0x00032014)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00080000, 393216)}

        /* dummy memory for IPC between CPSS and WM , see smemChtActiveWriteIpcCpssToWm(...) */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000FFF00, 0x000FFFFC)}
       };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);

        /*simulation defaults*/
      {

        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x00000054,         0xffff0000,      1,    0x4    }/*MG_Metal_Fix*/
            ,{DUMMY_NAME_PTR_CNS,           0x000000c4,         0x00000001,      1,    0x4    }/*NA_Address_Update_Queue_Control*/
            ,{DUMMY_NAME_PTR_CNS,           0x000000cc,         0x00000001,      1,    0x4    }/*FU_Address_Update_Queue_Control*/
            ,{DUMMY_NAME_PTR_CNS,           0x000000d4,         0x000f0f0f,      1,    0x4    }/*General_control*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000044,         0x00000006,      1,    0x4    }/*global_int_cntr_init_val*/
            ,{DUMMY_NAME_PTR_CNS,           0x00002800,         0x81800038,      1,    0x4    }/*SDMA_Configuration*/
            ,{DUMMY_NAME_PTR_CNS,           0x00002780,         0x3fffffff,      1,    0x4    }/*Tx_SDMA_Token_Bucket_Counter*/
            ,{DUMMY_NAME_PTR_CNS,           0x00002860,         0x00000400,      2,    0x4    }/*Receive_SDMA_Resource_Error_Count_And_Mode*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000281c,         0x000000ff,      1,    0x4    }/*Receive_SDMA_Status*/
            ,{DUMMY_NAME_PTR_CNS,           0x00002874,         0x00000024,      1,    0x4    }/*Transmit_SDMA_WRR_Token_Parameters*/
            ,{DUMMY_NAME_PTR_CNS,           0x00002700,         0x3fffffff,      8,    0x10    }/*Tx_SDMA_Token_Bucket_Queue_Counter*/
            ,{DUMMY_NAME_PTR_CNS,           0x000028f0,         0x00000080,      1,    0x4    }/*Tx_SDMA_small_packet_reject*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000050,         0x000011ab,      1,    0x4    }/*Vendor_ID*/
    /*,{DUMMY_NAME_PTR_CNS,           0x0000004c,         0x000e0000,      1,    0x4    }Device_ID*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000390,         0x000fffff,      1,    0x4    }/*CT_global_configuration*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000058,         0x08088002,      1,    0x4    }/*MG_Global_Control*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000005c,         0x20000007,      1,    0x4    }/*Global_Control*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000060,         0x03030302,      1,    0x4    }/*Genxs_Rate_config*/
            ,{DUMMY_NAME_PTR_CNS,           0x000000e4,         0xffffffff,      8,    0x4    }/*GenXs_Existing_Units_Configuration*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000104,         0x0f0f0f0f,      1,    0x4    }/*Genxs_Arbiter_Configuration0*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000108,         0x000f0f0f,      1,    0x4    }/*Genxs_Arbiter_Configuration1*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000010,         0x2c000015,      1,    0x4    }/*TWSI_Global_Configuration*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000014,         0xffffffff,      1,    0x4    }/*TWSI_Last_Address*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000018,         0x80100000,      1,    0x4    }/*TWSI_Timeout_Limit*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000020,         0x00000004,      1,    0x4    }/*TWSI_State_History0*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000518,         0x00000001,      1,    0x4    }/*Host_CPU_Doorbell_Register*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000604,         0x00000004,      1,    0x4    }/*MG_Source_ID*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000274,         0x00000001,      1,    0x4    }/*Regret_control*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000270,         0x0000001f,      1,    0x4    }/*Regret_counter_Initialization_values*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000027c,         0x0000d249,      1,    0x4    }/*Pex_Sync_FIFO_tuning0*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000654,         0x00000006,      1,    0x4    }/*global_int1_cntr_init_val*/
            ,{DUMMY_NAME_PTR_CNS,           0x000006b4,         0x00000006,      1,    0x4    }/*global_int2_cntr_init_val*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000140,         0x0000ffff,      1,    0x4    }/*AddCompCntrl*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000124,         0x00000001,      1,    0x4    }/*AddCompRegion_1*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000012c,         0x00000001,      1,    0x4    }/*AddCompRegion_3*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000134,         0x00000001,      1,    0x4    }/*AddCompRegion_5*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000013c,         0x00000001,      1,    0x4    }/*AddCompRegion_7*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000064,         0x800f0f00,      1,    0x4    }/*Loopback_Control*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000048c,         0x00000007,      6,    0x10    }/*CM3_Ext_Window_Control_Register*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000324,         0x00000006,      1,    0x4    }/*global_int3_cntr_init_val*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000348,         0x00000f0f,      1,    0x4    }/*CM3_TxMBUS_Arbiter_Configuration*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000354,         0x0003d090,      1,    0x4    }/*CM3_STCALIB*/
            ,{DUMMY_NAME_PTR_CNS,           0x00030000,         0x14a00000,      1,    0x4    }/*XSMI_Management*/
            ,{DUMMY_NAME_PTR_CNS,           0x0003000c,         0x0000002c,      1,    0x4    }/*Buffer_Memory_First_XG_Write_Bandwidth_Configuration*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000254,         0x00000007,      6,    0x4    }/*Window_Control_Register*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000204,         0x000000e0,      1,    0x4    }/*Unit_Default_ID*/
            ,{NULL,                         0,                  0x00000000,      0,    0x0  }
        };

        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
      }
    }
}

/**
* @internal smemBobcat3UnitMppm function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to common device memory object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitMppm
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {   /*bobcat2*/
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x00000124)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000150, 0x00000154)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001F0, 0x000001F0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000378)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000400)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000500)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000510, 0x00000518)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000600)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000610, 0x00000614)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000620, 0x00000624)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000710, 0x00000714)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000804)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000810, 0x00000814)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x00000904)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000910, 0x00000914)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000E04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E30, 0x00000E34)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001050, 0x00001054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x00001104)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001150, 0x00001154)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x00001204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x0000201C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000202C, 0x00002040)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002080, 0x00002094)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000020B0, 0x000020C4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002120, 0x0000214C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002170, 0x0000219C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000021B0, 0x000021B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000021C0, 0x000021C4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003030, 0x00003034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003200, 0x00003204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003230, 0x00003234)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003400, 0x00003408)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003420, 0x00003420)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003480, 0x00003480)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000034E0, 0x000034E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003540, 0x00003540)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003580, 0x00003580)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000035E0, 0x000035E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003640, 0x00003640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004000)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}

/**
* @internal smemBobcat3UnitRxDma function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitRxDma
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000B0, 0x000000B4)}
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000750, 0x00000750)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000760, 0x00000764)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000804, 0x00000924)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000950, 0x00000A70)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000C20)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000F28)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000120C, 0x0000132C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001528, 0x0000154C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001704, 0x0000170C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001720, 0x00001744)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001790, 0x00001790)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800, 0x00001800)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001820, 0x00001820)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001830, 0x00001834)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001840, 0x00001840)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001850, 0x0000185C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001864, 0x00001868)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001870, 0x00001870)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001B00, 0x00001C20)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001E00, 0x00001F20)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000020F0, 0x000020F0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x00002220)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002400, 0x00002520)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002A00, 0x00002B58)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002B60, 0x00002B60)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003500, 0x00003620)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003A00, 0x00003B20)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004080, 0x00004084)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000040A0, 0x000040A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000040C0, 0x000040C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005000)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,         0x00000000,         0x3fff3fff,      8,    0x8      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000040,         0x00003fff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000000b0,         0x3fffffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000000b4,         0x00000001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000350,         0x00000800,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000374,         0x00008600,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000398,         0x00008100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000039c,         0x00008a88,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000003a8,         0x00008847,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000003ac,         0x00008848,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000005d0,         0xffff0000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000804,         0x0ffff000,     73,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000954,         0x00000001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000958,         0x00000002,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000095c,         0x00000003,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000960,         0x00000004,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000964,         0x00000005,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000968,         0x00000006,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000096c,         0x00000007,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000970,         0x00000008,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000974,         0x00000009,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000978,         0x0000000a,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000097c,         0x0000000b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000980,         0x0000000c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000984,         0x0000000d,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000988,         0x0000000e,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000098c,         0x0000000f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000990,         0x00000010,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000994,         0x00000011,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000998,         0x00000012,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000099c,         0x00000013,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009a0,         0x00000014,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009a4,         0x00000015,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009a8,         0x00000016,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009ac,         0x00000017,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009b0,         0x00000018,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009b4,         0x00000019,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009b8,         0x0000001a,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009bc,         0x0000001b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009c0,         0x0000001c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009c4,         0x0000001d,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009c8,         0x0000001e,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009cc,         0x0000001f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009d0,         0x00000020,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009d4,         0x00000021,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009d8,         0x00000022,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009dc,         0x00000023,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009e0,         0x00000024,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009e4,         0x00000025,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009e8,         0x00000026,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009ec,         0x00000027,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009f0,         0x00000028,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009f4,         0x00000029,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009f8,         0x0000002a,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000009fc,         0x0000002b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a00,         0x0000002c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a04,         0x0000002d,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a08,         0x0000002e,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a0c,         0x0000002f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a10,         0x00000030,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a14,         0x00000031,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a18,         0x00000032,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a1c,         0x00000033,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a20,         0x00000034,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a24,         0x00000035,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a28,         0x00000036,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a2c,         0x00000037,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a30,         0x00000038,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a34,         0x00000039,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a38,         0x0000003a,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a3c,         0x0000003b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a40,         0x0000003c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a44,         0x0000003d,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a48,         0x0000003e,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a4c,         0x0000003f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a50,         0x00000040,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a54,         0x00000041,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a58,         0x00000042,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a5c,         0x00000043,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a60,         0x00000044,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a64,         0x00000045,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a68,         0x00000046,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a6c,         0x00000047,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000a70,         0x00000048,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000b00,         0x00000010,     73,    0x4      }
/*            ,{DUMMY_NAME_PTR_CNS,         0x00001700,         0x00008021,      1,    0x0      }*/
            ,{DUMMY_NAME_PTR_CNS,         0x00001704,         0x00080001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00001840,         0x00000080,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00001850,         0xaaaaaaaa,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00001858,         0x0000001f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00001864,         0x00003fff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00001868,         0x00000002,      1,    0x0      }
/*            ,{DUMMY_NAME_PTR_CNS,         0x000019b0,         0x0601e114,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x000019b4,         0x003896cb,      1,    0x0      }*/
            ,{DUMMY_NAME_PTR_CNS,         0x00001b00,         0x00000015,     73,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a00,         0x00004049,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a04,         0x00000004,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a08,         0x83828180,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a0c,         0x87868584,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a10,         0x8b8a8988,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a14,         0x8f8e8d8c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a18,         0x93929190,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a1c,         0x97969594,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a20,         0x9b9a9998,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a24,         0x9f9e9d9c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a28,         0xa3a2a1a0,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a2c,         0xa7a6a5a4,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a30,         0xabaaa9a8,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a34,         0xafaeadac,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a38,         0xb3b2b1b0,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a3c,         0xb7b6b5b4,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a40,         0xbbbab9b8,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a44,         0xbfbebdbc,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a48,         0xc3c2c1c0,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a4c,         0xc7c6c5c4,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a50,         0x828180c8,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a54,         0x86858483,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a58,         0x8a898887,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a5c,         0x8e8d8c8b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a60,         0x9291908f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a64,         0x96959493,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a68,         0x9a999897,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a6c,         0x9e9d9c9b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a70,         0xa2a1a09f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a74,         0xa6a5a4a3,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a78,         0xaaa9a8a7,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a7c,         0xaeadacab,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a80,         0xb2b1b0af,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a84,         0xb6b5b4b3,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a88,         0xbab9b8b7,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a8c,         0xbebdbcbb,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a90,         0xc2c1c0bf,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a94,         0xc6c5c4c3,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a98,         0x8180c8c7,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a9c,         0x85848382,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002aa0,         0x89888786,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002aa4,         0x8d8c8b8a,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002aa8,         0x91908f8e,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002aac,         0x95949392,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ab0,         0x99989796,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ab4,         0x9d9c9b9a,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ab8,         0xa1a09f9e,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002abc,         0xa5a4a3a2,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ac0,         0xa9a8a7a6,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ac4,         0xadacabaa,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ac8,         0xb1b0afae,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002acc,         0xb5b4b3b2,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ad0,         0xb9b8b7b6,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ad4,         0xbdbcbbba,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ad8,         0xc1c0bfbe,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002adc,         0xc5c4c3c2,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ae0,         0x80c8c7c6,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ae4,         0x84838281,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ae8,         0x88878685,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002aec,         0x8c8b8a89,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002af0,         0x908f8e8d,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002af4,         0x94939291,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002af8,         0x98979695,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002afc,         0x9c9b9a99,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b00,         0xa09f9e9d,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b04,         0xa4a3a2a1,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b08,         0xa8a7a6a5,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b0c,         0xacabaaa9,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b10,         0xb0afaead,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b14,         0xb4b3b2b1,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b18,         0xb8b7b6b5,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b1c,         0xbcbbbab9,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b20,         0xc0bfbebd,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b24,         0xc4c3c2c1,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b28,         0xc8c7c6c5,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b2c,         0x83828180,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b30,         0x87868584,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b34,         0x8b8a8988,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b38,         0x8f8e8d8c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b3c,         0x93929190,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b40,         0x97969594,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b44,         0x9b9a9998,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b48,         0x9f9e9d9c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b4c,         0xa3a2a1a0,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b50,         0xa7a6a5a4,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b54,         0xabaaa9a8,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b58,         0xafaeadac,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00003a00,         0x00000001,     73,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,         0x000040c0,         0x00000001,      1,    0x0      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemBobcat3UnitIa function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Ingress Aggregator
*         unit
*/
static void smemBobcat3UnitIa
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000008)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,         0x00000000,         0xffffffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000004,         0xffffffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000008,         0x00000001,      1,    0x0      }

            ,{NULL,                                0,         0x00000000,      0,    0x0      }
        };

        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemBobcat3UnitTxDma function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitTxDma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000244)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000504)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001020, 0x00001034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001060, 0x00001074)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002018, 0x0000201C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002038, 0x00002044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002050, 0x00002054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x00002130)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002300, 0x00002330)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x0000319C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003A00, 0x00003A30)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004158)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004160, 0x00004160)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000501C, 0x00005078)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005080, 0x0000508C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005100, 0x00005114)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005120, 0x00005134)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005140, 0x00005154)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005160, 0x00005174)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005180, 0x00005194)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000051A0, 0x000051B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000051C0, 0x000051D4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005200, 0x00005230)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005500, 0x00005530)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005700, 0x00005730)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005900, 0x00005930)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006100, 0x0000612C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006300, 0x0000632C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006500, 0x00006514)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006520, 0x00006534)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006540, 0x00006554)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006560, 0x00006574)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006580, 0x00006594)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000065A0, 0x000065B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000065C0, 0x000065D4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000, 0x00007004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007100, 0x00007130)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007300, 0x00007300)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007500, 0x00007514)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007700, 0x00007700)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007800, 0x00007830)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007930, 0x0000793C)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,             0x00000000,       0x00000007,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00000004,       0x000BFD02,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00000008,       0x002FFFFF,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00000010,       0x0000FFFF,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00000014,       0x00000041,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00000220,       0x22222222,     10,   0x4    }
            ,{DUMMY_NAME_PTR_CNS,             0x00001000,       0x00001FFF,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00001004,       0x00037CE2,     2,    0x8    }
            ,{DUMMY_NAME_PTR_CNS,             0x00001008,       0x00000007,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00001010,       0x00037CE2,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00001014,       0x00000005,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00001020,       0x00777777,     6,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,             0x00001060,       0x7777777F,     6,    0x4    }

            ,{DUMMY_NAME_PTR_CNS,             0x00003000,       0x00000001,     13,   0x20   }
            ,{DUMMY_NAME_PTR_CNS,             0x00003004,       0x0000018F,     13,   0x20   }
            ,{DUMMY_NAME_PTR_CNS,             0x0000300C,       0x00000C03,     13,   0x20   }
            ,{DUMMY_NAME_PTR_CNS,             0x00003010,       0x00020100,     13,   0x20   }
            ,{DUMMY_NAME_PTR_CNS,             0x00003014,       0x0000000F,     13,   0x20   }
            ,{DUMMY_NAME_PTR_CNS,             0x00003008,       0xC89FE0C8,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00003028,       0x551FE0C8,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00003048,       0x761FE0C8,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00003068,       0x551FE0C8,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00003088,       0xC89FE0C8,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000030A8,       0x551FE0C8,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000030C8,       0x761FE0C8,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000030E8,       0x551FE0C8,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00003108,       0xC89FE0C8,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00003128,       0x551FE0C8,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00003148,       0x761FE0C8,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00003168,       0x551FE0C8,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00003188,       0x449FE0C8,     1,    0x0    }

            ,{DUMMY_NAME_PTR_CNS,             0x00004000,       0x0000400D,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004004,       0x00000004,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004008,       0x83828180,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x0000400C,       0x87868584,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004010,       0x8B8A8988,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004014,       0x8281808C,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004018,       0x86858483,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x0000401C,       0x8A898887,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004020,       0x81808C8B,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004024,       0x85848382,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004028,       0x89888786,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x0000402C,       0x808C8B8A,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004030,       0x84838281,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004034,       0x88878685,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004038,       0x8C8B8A89,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x0000403C,       0x83828180,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004040,       0x87868584,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004044,       0x8B8A8988,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004048,       0x8281808C,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x0000404C,       0x86858483,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004050,       0x8A898887,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004054,       0x81808C8B,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004058,       0x85848382,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x0000405C,       0x89888786,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004060,       0x808C8B8A,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004064,       0x84838281,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004068,       0x88878685,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x0000406C,       0x8C8B8A89,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004070,       0x83828180,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004074,       0x87868584,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004078,       0x8B8A8988,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x0000407C,       0x8281808C,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004080,       0x86858483,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004084,       0x8A898887,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004088,       0x81808C8B,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x0000408C,       0x85848382,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004090,       0x89888786,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004094,       0x808C8B8A,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004098,       0x84838281,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x0000409C,       0x88878685,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040A0,       0x8C8B8A89,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040A4,       0x83828180,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040A8,       0x87868584,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040AC,       0x8B8A8988,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040B0,       0x8281808C,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040B4,       0x86858483,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040B8,       0x8A898887,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040BC,       0x81808C8B,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040C0,       0x85848382,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040C4,       0x89888786,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040C8,       0x808C8B8A,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040CC,       0x84838281,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040D0,       0x88878685,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040D4,       0x8C8B8A89,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040D8,       0x83828180,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040DC,       0x87868584,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040E0,       0x8B8A8988,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040E4,       0x8281808C,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040E8,       0x86858483,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040EC,       0x8A898887,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040F0,       0x81808C8B,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040F4,       0x85848382,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040F8,       0x89888786,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x000040FC,       0x808C8B8A,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004100,       0x84838281,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004104,       0x88878685,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004108,       0x8C8B8A89,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x0000410C,       0x83828180,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004110,       0x87868584,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004114,       0x8B8A8988,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004118,       0x8281808C,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x0000411C,       0x86858483,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004120,       0x8A898887,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004124,       0x81808C8B,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004128,       0x85848382,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x0000412C,       0x89888786,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004130,       0x808C8B8A,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004134,       0x84838281,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004138,       0x88878685,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x0000413C,       0x8C8B8A89,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004140,       0x83828180,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004144,       0x87868584,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004148,       0x8B8A8988,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x0000414C,       0x8281808C,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004150,       0x86858483,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004154,       0x8A898887,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00004158,       0x81808C8B,     1,    0x0    }

            ,{DUMMY_NAME_PTR_CNS,             0x00007000,       0x00B7AAA0,     2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,             0x00007100,       0x80000A00,     13,   0x4    }
            ,{DUMMY_NAME_PTR_CNS,             0x00007300,       0x00000A00,     1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00007500,       0x8001FC00,     6,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,             0x00007700,       0x0000000A,     1,    0x0    }
            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemBobcat3UnitTxFifo function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitTxFifo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000012C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000434)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000440, 0x0000044C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000508, 0x0000050C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000518, 0x0000051C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000724)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000958)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000960, 0x00000960)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x00001224)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x00001424)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x00001624)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001700, 0x00001824)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001900, 0x00001A24)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001B00, 0x00001C24)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003100, 0x00003224)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003300, 0x00003300)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 1648)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00012000, 8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00014000, 0x00014000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015000, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015200, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015400, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015600, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015800, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015A00, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017000, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017200, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017400, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017600, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017800, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017A00, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 2256)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00022000, 4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00024000, 0x00024000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025000, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025200, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025400, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025600, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025800, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025A00, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027000, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027200, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027400, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027600, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027800, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027A00, 296)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemBobcat3UnitTai function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitTai
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
     {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000450)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}

/**
* @internal smemBobcat3UnitBm function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitBm
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000000, 0x00000008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000010, 0x00000018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000020, 0x00000020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000030, 0x00000034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000050, 0x00000060)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000200, 0x00000228)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000300, 0x00000314)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000400, 0x00000414)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000420, 0x00000434)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000440, 0x00000454)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000460, 0x00000474)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000480, 0x00000494)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000004A0, 0x000004B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000004BC, 0x000004D8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000004E0, 0x000004E4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000500, 0x0000074C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00100000, 65536)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00120000, 65536)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00200000, 65536)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00220000, 65536)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00300000, 65536)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }


    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,             0x00000000,         0x3fff0000,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00000004,         0x80000000,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00000010,         0xffffffff,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,             0x00000018,         0x0000ffff,      2,    0x8    }
            ,{DUMMY_NAME_PTR_CNS,             0x00000020,         0x0000ffff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,             0x00000400,         0x00000009,      4,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,             0x000004c0,         0x00fc0fc0,      2,    0x10   }

            ,{DUMMY_NAME_PTR_CNS,             0x00000400,         0x00000008,      4,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,             0x000004c0,         0x0000f0f0,      2,    0x10     }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemBobcat3UnitBma function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitBma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 196608)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00059000, 0x0005900C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00059014, 0x00059018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00059020, 0x00059054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00059060, 0x00059074)}
            /*Virtual => Physical source port mapping*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0005A000, 2048),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(7 , 4),SMEM_BIND_TABLE_MAC(bmaPortMapping)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}

extern void smemChtCheckAndSendLinkChange (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   port,
    IN         GT_U32 * memPtr,
    IN         GT_U32 * inMemPtr
);

/**
* @internal smemChtActiveWrite100GMacControl0 function
* @endinternal
*
* @brief   Write Control 0 register in the 100G mac :
*         <CG_IP> CG TOP/Units/CG Upper Top Units %a Pipe %t/Converters registers/Control 0
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWrite100GMacControl0 (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 port;             /* Port number */
    GT_BIT port_cg_enable;

    port = CHT_FROM_ADDRESS_GET_PORT_CNS(devObjPtr, address);

    /* value after the write*/
    port_cg_enable = ((*inMemPtr) >> 16) & 1;

    if(port_cg_enable)
    {
        /* update the CHT_IS_100G_MAC_USED_MAC() indication */
        devObjPtr->portsArr[port].state = SKERNEL_PORT_STATE_CG_100G_E;

    }
    else
    {
        *memPtr = *inMemPtr ;

        /*this will set : devObjPtr->portsArr[port].state as 'XG' or 'GE_STACK_A1' */
        devObjPtr->portsArr[port].state = SKERNEL_PORT_STATE_XG_E;

        /* trigger smemXcatActiveWriteMacModeSelect(...) to be called */
        smemRegUpdateAfterRegFile(devObjPtr,
            SMEM_XCAT_XG_MAC_CONTROL3_REG(devObjPtr, port),
            1);

        return;/* do not impact link status from this function ... it should come from :
            smemChtActiveWriteForceLinkDownXg or
            smemChtActiveWriteForceLinkDown */
    }

    /*    *memPtr = *inMemPtr ; is done inside : smemChtCheckAndSendLinkChange */
    smemChtCheckAndSendLinkChange(devObjPtr,port,memPtr,inMemPtr);

}

/**
* @internal smemChtActiveWrite100GMacResets function
* @endinternal
*
* @brief   Write 'resets' register in the 100G mac :
*         <CG_IP> CG TOP/Units/CG Upper Top Units %a Pipe %t/Converters registers/Resets
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWrite100GMacResets (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 port;             /* Port number */

    port = CHT_FROM_ADDRESS_GET_PORT_CNS(devObjPtr, address);

    if(devObjPtr->portsArr[port].state != SKERNEL_PORT_STATE_CG_100G_E)
    {
        *memPtr = *inMemPtr ;
        return;
    }

    /* allow modification of bit 28 to represent : SNET_CHT_PORT_MAC_FIELDS_PortMACReset_E */
    /* and by this to give link change indication */

    /*    *memPtr = *inMemPtr ; is done inside : smemChtCheckAndSendLinkChange */
    smemChtCheckAndSendLinkChange(devObjPtr,port,memPtr,inMemPtr);
}

/* actual number of counters in <CG_MIBs> */
#define CG_100G_PORT_MIB_ENTRY_SIZE_ALL_64_BITS \
    (2/*64bits*/ * \
        (sizeof(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[0].CG_MIBs) / \
         sizeof(GT_U32)))

/**
* @internal smemChtActiveWrite100GMacMibCtrlReg function
* @endinternal
*
* @brief   Write <STATN_CONFIG> register in the 100G mac :
*         /<CG_IP>CG Top/<CG IP REGS 1.0> CG/Units/CG Port %a Pipe %t/MAC/MAC_RegFile/STATN_CONFIG
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWrite100GMacMibCtrlReg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  port;                   /* port number */
    GT_U32  *internalMemPtr;/*pointer to the 'shadow'*/
    GT_U32  *baseMibPtr;/*base memory of the MIB counters of the port*/
    GT_U32  mibEntrySize = CG_100G_PORT_MIB_ENTRY_SIZE_ALL_64_BITS;
    GT_BIT  clearOnRead,clearAll,captureRx,captureTx;
    GT_U32  regAddr;
    /*
        Configure clear-on-read behavior.
        When set to 1, the counters are cleared after having been transferred
        into the read registers (snapshot captured).
        When set 0 (default) the counters are not modified when read/captured.
        Note: this bit must be configured before any capture command is issued
        (i.e. value must be kept when writing with any of the command bits=1).
    */


    *memPtr = *inMemPtr ;

    clearOnRead = ((*memPtr) >> 1) & 1;
    clearAll    = ((*memPtr) >> 2) & 1;
    captureRx   = ((*memPtr) >> 4) & 1;
    captureTx   = ((*memPtr) >> 5) & 1;


    port = CHT_FROM_ADDRESS_GET_PORT_CNS(devObjPtr, address);
    internalMemPtr = CHT_INTERNAL_MEM_PTR(devObjPtr,CHT_INTERNAL_SIMULATION_USE_MEM_MSM_MIB_COUNTERS_CG_100G_PORT_START_E
                                      + (mibEntrySize * port));
    /* address of the start of counters for this port */
    regAddr = CHT_MAC_PORTS_MIB_COUNTERS_BASE_ADDRESS_CNS(devObjPtr,port);
    baseMibPtr = smemMemGet(devObjPtr,regAddr);

    if(captureRx | captureTx)
    {
        /* copy from the counter to the 'shadow' register */
        memcpy(internalMemPtr,baseMibPtr,mibEntrySize*sizeof(GT_U32));

        if(clearOnRead)
        {
            memset(baseMibPtr,0,mibEntrySize*sizeof(GT_U32));
        }
    }

    if(clearAll)
    {
        memset(baseMibPtr,0,mibEntrySize*sizeof(GT_U32));
    }

    /* clear the 'action bits' */
    *memPtr &= 1 << 1;/*keep only bit '1' <clearOnRead> */
}

/**
* @internal smemBobcat3ActiveRead100GMacMibCounters function
* @endinternal
*
* @brief   GOP MIB counters read of CG MAC (100G port)
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter -
*                                      global interrupt bit number.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*
* @note Performs capturing of the MAC MIB entry to internal memory on
*       first word read. The capture is reseted on last word read.
*
*/
void smemBobcat3ActiveRead100GMacMibCounters
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    GT_U32  port;                   /* port number */
    GT_U32  *internalMemPtr;/*pointer to the 'shadow'*/
    GT_U32  mibEntrySize = CG_100G_PORT_MIB_ENTRY_SIZE_ALL_64_BITS;
    GT_U32  counterAddrLowest   ;
    GT_U32  counterAddrHighiest ;
    GT_U32  offset;

    port = CHT_FROM_ADDRESS_GET_PORT_CNS(devObjPtr, address);

    counterAddrLowest   =     SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[port].CG_MIBs.CG_MEMBER_LOWEST_ADDR;
    counterAddrHighiest = 4 + SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[port].CG_MIBs.CG_MEMBER_HIGH_ADDR;

    if(address < counterAddrLowest ||
       address > counterAddrHighiest)
    {
        /* just read the address ... we are not in the 'counters' zone */
        *outMemPtr = *memPtr;
        return;
    }

    /* so we need to 'read' counter from the 'shadow' */
    offset = (address - counterAddrLowest) / 4;

    internalMemPtr = CHT_INTERNAL_MEM_PTR(devObjPtr,CHT_INTERNAL_SIMULATION_USE_MEM_MSM_MIB_COUNTERS_CG_100G_PORT_START_E
                                      + (mibEntrySize * port));

    *outMemPtr = internalMemPtr[offset];
    return;
}

/**
* @internal smemBobcat3UnitGop function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the GOP unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitGop
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* Port<n> Auto-Negotiation Configuration Register */
    {0x0000000C                       , 0xFFF00FFF, NULL, 0 , smemChtActiveWriteForceLinkDown, 0},
    {0x0000000C + BC3_PIPE1_GOP_OFFSET, 0xFFF00FFF, NULL, 0 , smemChtActiveWriteForceLinkDown, 0},

    /* Port MAC Control Register2 */
    {0x00000008                       , 0xFFF00FFF, NULL, 0 , smemChtActiveWriteMacGigControl2, 0},
    {0x00000008 + BC3_PIPE1_GOP_OFFSET, 0xFFF00FFF, NULL, 0 , smemChtActiveWriteMacGigControl2, 0},

    /* Port MAC Control Register0 */
    {0x000C0000,                        0xFFFC0FFF, NULL, 0 , smemChtActiveWriteForceLinkDownXg, 0},
    {0x000C0000 + BC3_PIPE1_GOP_OFFSET, 0xFFFC0FFF, NULL, 0 , smemChtActiveWriteForceLinkDownXg, 0},

    /* Port MAC Control Register3 */
    {0x000C001C,                        0xFFFF0FFF, NULL, 0 , smemXcatActiveWriteMacModeSelect, 0},
    {0x000C001C + BC3_PIPE1_GOP_OFFSET, 0xFFFF0FFF, NULL, 0 , smemXcatActiveWriteMacModeSelect, 0},

    /* Port MAC Control Register4 : SMEM_XCAT_XG_MAC_CONTROL4_REG */
    {0x000C0084,                        0xFFFF0FFF, NULL, 0 , smemBobKActiveWriteMacXlgCtrl4, 0},
    {0x000C0084 + BC3_PIPE1_GOP_OFFSET, 0xFFFF0FFF, NULL, 0 , smemBobKActiveWriteMacXlgCtrl4, 0},


    /* stack gig ports - Tri-Speed Port<n> Interrupt Mask Register */
    {0x000C0024,                        0xFFFF0FFF, NULL, 0 , smemChtActiveWritePortInterruptsMaskReg, 0},
    {0x000C0024 + BC3_PIPE1_GOP_OFFSET, 0xFFFF0FFF, NULL, 0 , smemChtActiveWritePortInterruptsMaskReg, 0},

    /* XG Port<n> Interrupt Cause Register  */
    {0x000C0014,                        0xFFFC0FFF,  smemChtActiveReadIntrCauseReg, 29, smemChtActiveWriteIntrCauseReg, 0},
    {0x000C0014 + BC3_PIPE1_GOP_OFFSET, 0xFFFC0FFF,  smemChtActiveReadIntrCauseReg, 29, smemChtActiveWriteIntrCauseReg, 0},

    /* XG Port<n> Interrupt Mask Register */
    {0x000C0018,                        0xFFFC0FFF, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},
    {0x000C0018 + BC3_PIPE1_GOP_OFFSET, 0xFFFC0FFF, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},


    /* PTP Port<n> Interrupt Cause Register  */
    {0x00180800,                        0xFFFC0FFF,  smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x00180800 + BC3_PIPE1_GOP_OFFSET, 0xFFFC0FFF,  smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},

    /* PTP Port<n> Interrupt Mask Register */
    {0x00180804,                        0xFFFC0FFF, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},
    {0x00180804 + BC3_PIPE1_GOP_OFFSET, 0xFFFC0FFF, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},




    /* stack gig ports - Port<n> Interrupt Cause Register  */
    {0x00000020,                        0xFFF00FFF,smemChtActiveReadIntrCauseReg, 18, smemChtActiveWriteIntrCauseReg, 0},
    {0x00000020 + BC3_PIPE1_GOP_OFFSET, 0xFFF00FFF,smemChtActiveReadIntrCauseReg, 18, smemChtActiveWriteIntrCauseReg, 0},

    /* stack gig ports - Tri-Speed Port<n> Interrupt Mask Register */
    {0x00000024,                        0xFFF00FFF, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},
    {0x00000024 + BC3_PIPE1_GOP_OFFSET, 0xFFF00FFF, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},

    /* CG-mac (100G mac) */
    /* <CG_IP> CG TOP/Units/CG Upper Top Units %a Pipe %t/Converters registers/Control 0 */
    {0x00340000,                        0xFFFC0FFF, NULL, 0 , smemChtActiveWrite100GMacControl0, 0},
    {0x00340000 + BC3_PIPE1_GOP_OFFSET, 0xFFFC0FFF, NULL, 0 , smemChtActiveWrite100GMacControl0, 0},

    {0x00340010,                        0xFFFC0FFF, NULL, 0 , smemChtActiveWrite100GMacResets, 0},
    {0x00340010 + BC3_PIPE1_GOP_OFFSET, 0xFFFC0FFF, NULL, 0 , smemChtActiveWrite100GMacResets, 0},

    /*CG Interrupt mask*/
    /*SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_CONVERTERS.CGMAConvertersCG_Interrupt_mask*/
    {0x00340044,                        0xFFFC0FFF, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},
    {0x00340044 + BC3_PIPE1_GOP_OFFSET, 0xFFFC0FFF, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},

    /*CG Interrupt cause*/
    /*SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_CONVERTERS.CGMAConvertersCG_Interrupt_cause*/
    {0x00340040,                        0xFFFC0FFF, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x00340040 + BC3_PIPE1_GOP_OFFSET, 0xFFFC0FFF, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},

    /*STATN_CONFIG */
    /*SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CGPORTMACStatNConfig*/
    {0x00340478,                        0xFFFC0FFF, NULL, 0, smemChtActiveWrite100GMacMibCtrlReg, 0},
    {0x00340478 + BC3_PIPE1_GOP_OFFSET, 0xFFFC0FFF, NULL, 0, smemChtActiveWrite100GMacMibCtrlReg, 0},

    /* need to cover
       from : 0x10340480 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.aFramesTransmittedOK
       to   : 0x10340624 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.aMACControlFramesReceived

       so we do several 'larger ranges' :
        1. 0x10340400 - 0x103405FC
        2. 0x10340600 - 0x1034063C
    */

    /* 1. 0x10340400 - 0x103405FC */
    {0x00340400,                        0xFFFC0E00, smemBobcat3ActiveRead100GMacMibCounters, 0, NULL, 0},
    {0x00340400+ BC3_PIPE1_GOP_OFFSET,  0xFFFC0E00, smemBobcat3ActiveRead100GMacMibCounters, 0, NULL, 0},
    /* 2. 0x10340600 - 0x1034063C */
    {0x00340600,                        0xFFFC0FC0, smemBobcat3ActiveRead100GMacMibCounters, 0, NULL, 0},
    {0x00340600+ BC3_PIPE1_GOP_OFFSET,  0xFFFC0FC0, smemBobcat3ActiveRead100GMacMibCounters, 0, NULL, 0},

    /* 3 PTP TX Timestamp Queue0 reg2 */
    {0x00000800 + 0x14, 0xFF000FFF, smemLion3ActiveReadGopPtpTxTsQueueReg2Reg, 0, NULL, 0},
    /* 4 PTP TX Timestamp Queue1 reg2 */
    {0x00000800 + 0x20, 0xFF000FFF, smemLion3ActiveReadGopPtpTxTsQueueReg2Reg, 0, NULL, 0},


    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* Gig Ports*/
                /* ports 0..36 */
                 {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000000, 0x00000094)} , FORMULA_TWO_PARAMETERS_GIG_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000000A0, 0x000000A4)} , FORMULA_TWO_PARAMETERS_GIG_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000000C0, 0x000000E0)} , FORMULA_TWO_PARAMETERS_GIG_DUP_PIPE_AND_PORTS_CNS}

            /* XLG */
                /* ports 0..36 */
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000c0000 , 0x000c0024 )}, FORMULA_TWO_PARAMETERS_XLG_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000c002C , 0x000C0030 )}, FORMULA_TWO_PARAMETERS_XLG_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000C0038 , 0x000C0060 )}, FORMULA_TWO_PARAMETERS_XLG_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000C0068 , 0x000C0088 )}, FORMULA_TWO_PARAMETERS_XLG_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000C0090 , 0x000C0098 )}, FORMULA_TWO_PARAMETERS_XLG_DUP_PIPE_AND_PORTS_CNS}

            /* MPCS */
                /* ports 0..36 */
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180008 , 0x00180018 )}, FORMULA_TWO_PARAMETERS_XLG_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180020 , 0x00180024 )}, FORMULA_TWO_PARAMETERS_XLG_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180030 , 0x00180030 )}, FORMULA_TWO_PARAMETERS_XLG_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0018003C , 0x001800C8 )}, FORMULA_TWO_PARAMETERS_XLG_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001800D0 , 0x00180120 )}, FORMULA_TWO_PARAMETERS_XLG_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180128 , 0x0018014C )}, FORMULA_TWO_PARAMETERS_XLG_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0018015C , 0x0018017C )}, FORMULA_TWO_PARAMETERS_XLG_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00180200 , 256)}       , FORMULA_TWO_PARAMETERS_XLG_DUP_PIPE_AND_PORTS_CNS}

            /* XPCS IP */
                /* ports 0..36 in steps of 2*/
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180400, 0x00180424)}, FORMULA_TWO_PARAMETERS(BC3_NUM_PIPES, BC3_PIPE1_GOP_OFFSET, BC3_NUM_GOP_PORTS_XLG/2  , 0x1000*2)}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0018042C, 0x0018044C)}, FORMULA_TWO_PARAMETERS(BC3_NUM_PIPES, BC3_PIPE1_GOP_OFFSET, BC3_NUM_GOP_PORTS_XLG/2  , 0x1000*2)}
                /* 6 lanes */
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00180450, 0x0044)}, FORMULA_THREE_PARAMETERS(BC3_NUM_PIPES, BC3_PIPE1_GOP_OFFSET, 6 , 0x44  , BC3_NUM_GOP_PORTS_XLG/2  , 0x1000*2)                          }

            /*FCA*/
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180600, 0x001806A0)}, FORMULA_TWO_PARAMETERS_GIG_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180704, 0x00180714)}, FORMULA_TWO_PARAMETERS_GIG_DUP_PIPE_AND_PORTS_CNS}
            /*PTP*/
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180800, 0x0018087C)}, FORMULA_TWO_PARAMETERS_GIG_DUP_PIPE_AND_PORTS_CNS}

            /* Mac-TG Generator */
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180C00 ,0x00180CCC)}, FORMULA_TWO_PARAMETERS_GIG_DUP_PIPE_AND_PORTS_CNS}

            /* CG-mac (100G mac) */
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00340000, 0x00340034)}, FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00340040, 0x0034004C)}, FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00340060, 0x0034009C)}, FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00340100, 0x00340108)}, FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00340400, 1024)}      , FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00340800, 1024)}      , FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS}
            /* CG-PCS-mac (100G mac) */
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300000, 0x00300020)}, FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300038, 0x0030003C)}, FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300080, 0x00300084)}, FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x003000A8, 0x003000B4)}, FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x003000C8, 0x003000D4)}, FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300320, 0x0030036C)}, FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300640, 0x0030068C)}, FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300800, 0x0030080C)}, FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300840, 0x00300840)}, FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300900, 0x0030099C)}, FORMULA_TWO_PARAMETERS_CG_100G_DUP_PIPE_AND_PORTS_CNS}



        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             /*{DUMMY_NAME_PTR_CNS,            0x000a1000,         0x0000ffff,      1,    0x0         }*/
            /* PTP */
             {DUMMY_NAME_PTR_CNS,           0x00180808,         0x00000001,      GIG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,           0x00180870,         0x000083aa,      GIG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,           0x00180874,         0x00007e5d,      GIG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,           0x00180878,         0x00000040,      GIG_DUP_PIPE_AND_PORTS_CNS }

        /*Giga*/
            ,{DUMMY_NAME_PTR_CNS,            0x00000000,         0x00008be5,     GIG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x00000003,     GIG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00000008,         0x0000c048,     GIG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x0000bae8,     GIG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0000001c,         0x00000052,     GIG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0000002c,         0x0000000c,     GIG_DUP_PIPE_AND_PORTS_CNS      ,2,    0x18     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000030,         0x0000c815,     GIG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00000048,         0x00000300,     GIG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00000094,         0x00000001,     GIG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x000000c0,         0x00001004,     GIG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x000000c4,         0x00000100,     GIG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x000000c8,         0x000001fd,     GIG_DUP_PIPE_AND_PORTS_CNS }

            ,{DUMMY_NAME_PTR_CNS,            0x00000090,         0x0000ff9a,     GIG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00000018,         0x00004b4d,     GIG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00000014,         0x000008c4,     GIG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x000000d4,         0x000000ff,     GIG_DUP_PIPE_AND_PORTS_CNS }

        /*XLG*/
                                        /* set ALL ports as XLG */
            ,{DUMMY_NAME_PTR_CNS,            0x000c001c,         0x00006000,     XLG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x000c0030,         0x000007ec,     XLG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x000c0080,         0x00001000,     XLG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x000c0084,         0x00001210,     XLG_DUP_PIPE_AND_PORTS_CNS }
        /*FCA*/
            ,{DUMMY_NAME_PTR_CNS,            0x00180000+0x600,   0x00000011,      XLG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00180004+0x600,   0x00002003,      XLG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00180054+0x600,   0x00000001,      XLG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00180058+0x600,   0x0000c200,      XLG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0018005c+0x600,   0x00000180,      XLG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0018006c+0x600,   0x00008808,      XLG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00180070+0x600,   0x00000001,      XLG_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00180104+0x600,   0x0000ff00,      XLG_DUP_PIPE_AND_PORTS_CNS }

            /* CG-mac (100G mac) */
            ,{DUMMY_NAME_PTR_CNS,            0x00340000,         0x10020018,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340004,         0x0000003f,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340008,         0xff00ff00,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0034000c,         0x00000190,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340014,         0x0a080040,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340018,         0x00002f2d,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0034001c,         0x0000001c,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340060,         0x01408000,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340064,         0x00477690,      CG_100G_DUP_PIPE_AND_PORTS_CNS ,4,    0x10   }
            ,{DUMMY_NAME_PTR_CNS,            0x00340068,         0x00e6c4f0,      CG_100G_DUP_PIPE_AND_PORTS_CNS ,4,    0x10   }
            ,{DUMMY_NAME_PTR_CNS,            0x0034006c,         0x009b65c5,      CG_100G_DUP_PIPE_AND_PORTS_CNS ,4,    0x10   }
            ,{DUMMY_NAME_PTR_CNS,            0x00340070,         0x003d79a2,      CG_100G_DUP_PIPE_AND_PORTS_CNS ,3,    0x10   }
            ,{DUMMY_NAME_PTR_CNS,            0x00340100,         0x003d79a2,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340104,         0x3fff3fff,      CG_100G_DUP_PIPE_AND_PORTS_CNS  ,2,    0x4   }

            ,{DUMMY_NAME_PTR_CNS,            0x00340400,         0x00010106,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340414,         0x00000600,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0034041c,         0x00000010,      CG_100G_DUP_PIPE_AND_PORTS_CNS  ,2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00340430,         0x00007d00,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340434,         0x00000001,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340444,         0x0000000c,      CG_100G_DUP_PIPE_AND_PORTS_CNS }

            ,{DUMMY_NAME_PTR_CNS,            0x00340804,         0x00008003,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340a04,         0x0000f000,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340a0c,         0x00000022,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00340a28,         0x0000002a,      CG_100G_DUP_PIPE_AND_PORTS_CNS  ,4,    0x4   }

            /* CG-PCS-mac (100G mac) */
            ,{DUMMY_NAME_PTR_CNS,            0x00300000,         0x00002050,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300010,         0x00000100,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300014,         0x00000008,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030001c,         0x00000005,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300020,         0x00008020,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x003000b4,         0x00008000,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300804,         0x00000001,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300808,         0x00003fff,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030080c,         0x00009999,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300900,         0x000068c1,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300904,         0x00000021,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300908,         0x0000719d,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030090c,         0x0000008e,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300910,         0x00004b59,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300914,         0x000000e8,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300918,         0x0000954d,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030091c,         0x0000007b,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300920,         0x000007f5,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300924,         0x00000009,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300928,         0x000014dd,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030092c,         0x000000c2,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300930,         0x00004a9a,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300934,         0x00000026,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300938,         0x0000457b,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030093c,         0x00000066,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300940,         0x000024a0,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300944,         0x00000076,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300948,         0x0000c968,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030094c,         0x000000fb,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300950,         0x00006cfd,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300954,         0x00000099,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300958,         0x000091b9,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030095c,         0x00000055,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300960,         0x0000b95c,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300964,         0x000000b2,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300968,         0x0000f81a,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030096c,         0x000000bd,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300970,         0x0000c783,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300974,         0x000000ca,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300978,         0x00003635,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030097c,         0x000000cd,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300980,         0x000031c4,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300984,         0x0000004c,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300988,         0x0000d6ad,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030098c,         0x000000b7,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300990,         0x0000665f,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300994,         0x0000002a,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x00300998,         0x0000f0c0,      CG_100G_DUP_PIPE_AND_PORTS_CNS }
            ,{DUMMY_NAME_PTR_CNS,            0x0030099c,         0x000000e5,      CG_100G_DUP_PIPE_AND_PORTS_CNS }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemBobcat3UnitLpSerdes function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the LpSerdes unit
*
* @param[in] devObjPtr                - pointer to common device memory object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitLpSerdes
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

/*SERDES*/
    /* took registers of offset 0x200 of Lion2 and convert to offset 0x800 */
    /* COMPHY_H %t Registers/KVCO Calibration Control (0x00000800 + t*0x1000: where t (0-35) represents SERDES) */
    {0x00000808, 0xFFF00FFF, NULL, 0, smemLion2ActiveWriteKVCOCalibrationControlReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* serdes external registers SERDESes for ports 0...36 */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 0x44)} , FORMULA_TWO_PARAMETERS(BC3_NUM_SERDESES_CNS , 0x1000, BC3_NUM_PIPES, BC3_PIPE1_SERDES_OFFSET)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* SERDES registers */
            {DUMMY_NAME_PTR_CNS,         0x00000000,         0x00000800,   BC3_NUM_SERDESES_CNS  ,   0x1000,    BC3_NUM_PIPES, BC3_PIPE1_SERDES_OFFSET}
           ,{DUMMY_NAME_PTR_CNS,         0x00000004,         0x00008801,   BC3_NUM_SERDESES_CNS  ,   0x1000,    BC3_NUM_PIPES, BC3_PIPE1_SERDES_OFFSET}
           ,{DUMMY_NAME_PTR_CNS,         0x00000008,         0x00000100,   BC3_NUM_SERDESES_CNS  ,   0x1000,    BC3_NUM_PIPES, BC3_PIPE1_SERDES_OFFSET}
           /* must be last */
           ,{NULL,            0,         0x00000000,      0,    0x0      }
        };

        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}
/**
* @internal smemBobcat3UnitXGPortMib function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the XGPortMib unit
*
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitXGPortMib
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* XG port MAC MIB Counters */
    {0x00000000,                            SMEM_BOBCAT2_XG_MIB_COUNT_MSK_CNS, smemLion2ActiveReadMsmMibCounters, 0, NULL,0},
    {0x00000000 + BC3_PIPE1_MIB_OFFSET_GIG, SMEM_BOBCAT2_XG_MIB_COUNT_MSK_CNS, smemLion2ActiveReadMsmMibCounters, 0, NULL,0},

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
            /* ports 0..36 */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000000 , 256)}, FORMULA_TWO_PARAMETERS(BC3_NUM_GOP_PORTS_GIG , BOBCAT2_MIB_OFFSET_CNS, BC3_NUM_PIPES, BC3_PIPE1_MIB_OFFSET_GIG)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    /*if(devObjPtr->support_memoryRanges == 0)*/
    {
        /* don't point the next 'half' unit on 'me' */
        unitPtr[1].hugeUnitSupportPtr = NULL;
    }
}

/**
* @internal smemBobcat3UnitTxqDq function
* @endinternal
*
* @brief   Allocate address type specific memories -- TXQ DQ
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitTxqDq
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000120)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000208, 0x00000248)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000260, 0x00000268)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002C0, 0x000003DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x0000051C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000604)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000610, 0x00000650)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000660, 0x00000660)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000670, 0x00000670)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000680, 0x00000680)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000690, 0x00000690)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000800)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000808, 0x00000808)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x00000904)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000920, 0x00000924)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A00)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B30)}
            /* JIRA : TXQ-1265 - registers addresses % 4 are not 0 (1,2,3) - TXQ_IP_dq/Global/Credit Counters/TxDMA Port [n] Baseline. */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D30)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001008, 0x00001010)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001040, 0x0000115C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001240, 0x0000133C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001380, 0x0000149C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x00001500)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001510, 0x0000162C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001650, 0x000017A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000017B0, 0x000017B0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800, 0x00001800)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001904, 0x00001A28)}
            /*Scheduler State Variable RAM*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 3072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(168,32),SMEM_BIND_TABLE_MAC(Scheduler_State_Variable)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x0000400C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004100, 0x0000421C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004300, 0x0000441C)}
            /*Priority Token Bucket Configuration*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00006000, 6144), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(488,64)}
            /*Port Token Bucket Configuration*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000A000, 768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(61,8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C000, 0x0000C000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C008, 0x0000C044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C080, 0x0000C09C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C100, 0x0000C13C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C180, 0x0000C37C)}
            /*,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C500, 0x0000C8FC)}*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000CA00, 0x0000CA08)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D000, 0x0000D004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D010, 0x0000D014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D040, 0x0000D15C)}
            /*Egress STC Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000D800, 1536), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(80,16),SMEM_BIND_TABLE_MAC(egressStc)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E120, 0x0000E17C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E220, 0x0000E27C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E520, 0x0000E57C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F120, 0x0000F17C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F220, 0x0000F27C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F320, 0x0000F37C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F420, 0x0000F47C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00010120, 0x0001017C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00010220, 0x0001027C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00011000, 512)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}


/**
* @internal smemBobcat3UnitTxqLl function
* @endinternal
*
* @brief   Allocate address type specific memories -- TXQ ll (link list)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitTxqLl
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 18432)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008000, 18432)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 18432)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 18432)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00028000, 18432)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00030000, 18432)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00038000, 18432)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00110000, 0x00110000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00112008, 0x00112030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00112100, 0x00112144)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00113000, 0x00113000)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             /*{DUMMY_NAME_PTR_CNS,            0x000a1000,         0x0000ffff,      1,    0x0         }*/

            {NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemBobcat3UnitTxqQcn function
* @endinternal
*
* @brief   Allocate address type specific memories -- TXQ Qcn
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitTxqQcn
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000000C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000040)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000050, 0x00000050)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000060, 0x00000060)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000090, 0x000000AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000D0, 0x000000D0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000E0, 0x000000E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000F0, 0x000000F0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000110, 0x00000110)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x00000128)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000130, 0x00000130)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000140, 0x00000140)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000200)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000210, 0x00000210)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x000004FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000C5C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000E1C)}
            /*CN Sample Intervals Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 36864), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(40 ,8),SMEM_BIND_TABLE_MAC(CN_Sample_Intervals)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

}

/**
* @internal smemBobcat3UnitTxqQueue function
* @endinternal
*
* @brief   Allocate address type specific memories -- TXQ queue
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitTxqQueue
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    if(devObjPtr->errata.txqEgressMibCountersNotClearOnRead)
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
            TXQ_QUEQUE_ACTIVE_MEM_MAC,
        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }
    else
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
            TXQ_QUEQUE_ACTIVE_MEM_MAC,
            TXQ_QUEQUE_EGR_PACKET_COUNTERS_CLEAR_ON_READ_ACTIVE_MEM_MAC ,
       END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0440, 0x000A044C)}
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

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             /*{DUMMY_NAME_PTR_CNS,            0x000a1000,         0x0000ffff,      1,    0x0         }*/

            {DUMMY_NAME_PTR_CNS,            0x000A0000,         0x0000C1A0,      1,    0x0                       }
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
* @internal smemBobcat3UnitTxqBmx function
* @endinternal
*
* @brief   Allocate address type specific memories -- TXQ bmx
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitTxqBmx
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000030)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            {DUMMY_NAME_PTR_CNS,            0x00000004,         0x006318C6,      1,    0x0                       }
           ,{DUMMY_NAME_PTR_CNS,            0x00000030,         0x00000005,      1,    0x0                       }
           ,{NULL,                                   0,         0x00000000,      0,    0x0                       }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemBobcat3UnitFcu function
* @endinternal
*
* @brief   Allocate address type specific memories -- FCU
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitFcu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000068)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000015C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x0000025C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000, 2048)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}

/**
* @internal smemBobcat3Unit__dummy function
* @endinternal
*
* @brief   Allocate address type specific memories -- dummy
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3Unit__dummy
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000000)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}

/**
* @internal smemBobcat3UnitTxqPfc function
* @endinternal
*
* @brief   Allocate address type specific memories -- TXQ pfc
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitTxqPfc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    GT_U32  isSip5_25    = SMEM_CHT_IS_SIP5_25_GET(devObjPtr);

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000030, 0x00000030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000040)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000050, 0x00000050)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000060, 0x00000060)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x00000070)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000D0, 0x000000D0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000F0, 0x000001EC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002F0, 0x0000052C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000006F0, 0x0000072C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D1C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x00001100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001110, 0x00001110)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001120, 0x00001120)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001140, 0x00001140)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001170, 0x00001170)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x00001304)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001310, 0x00001314)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001320, 0x0000132C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001370, 0x00001370)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001390, 0x00001390)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000013B0, 0x000013BC)}
            /*FC Mode Profile TC XOff Thresholds*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001800, 256), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(29,4),SMEM_BIND_TABLE_MAC(FC_Mode_Profile_TC_XOff_Thresholds)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001A00, 0x00001AFC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001C00, 0x000023FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008000, 2496)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00014000, 0x0001401C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00014200, 0x0001421C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00014400, 0x0001441C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00015400, 0x00015408)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00015500, 0x00015500)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    if (isSip5_25)
    {
        SMEM_CHUNK_BASIC_STC  sip5_25ChunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003104)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003200, 0x00003260)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC ((0x00008000+2496), 704)} /* table has 3200 bytes = 2496 (BC3) + 704 */
        };
        smemUnitChunkAddBasicChunk(devObjPtr, unitPtr, ARRAY_NAME_AND_NUM_ELEMENTS_MAC(sip5_25ChunksMem));
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* UNIT_CPFC*/
/*9 MSBits of 'Addr', 23 LSBits of 'Addr',     val,    repeat,    skip,      block_repeat, block_skip*/
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x00000001,      1,    0x0                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001100,         0xffff0000,      1,    0x0                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001110,         0x00000287,      1,    0x0                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001800,         0x0007ffff,     64,    0x4                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c04,         0x00000401,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c08,         0x00000802,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c0c,         0x00000c03,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c10,         0x00001004,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c14,         0x00001405,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c18,         0x00001806,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c1c,         0x00001c07,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c20,         0x00002008,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c24,         0x00002409,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c28,         0x0000280a,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c2c,         0x00002c0b,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c30,         0x0000300c,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c34,         0x0000340d,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c38,         0x0000380e,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c3c,         0x00003c0f,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c40,         0x00004010,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c44,         0x00004411,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c48,         0x00004812,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c4c,         0x00004c13,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c50,         0x00005014,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c54,         0x00005415,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c58,         0x00005816,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c5c,         0x00005c17,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c60,         0x00006018,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c64,         0x00006419,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c68,         0x0000681a,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c6c,         0x00006c1b,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c70,         0x0000701c,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c74,         0x0000741d,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c78,         0x0000781e,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c7c,         0x00007c1f,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c80,         0x00008020,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c84,         0x00008421,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c88,         0x00008822,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c8c,         0x00008c23,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c90,         0x00009024,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c94,         0x00009425,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c98,         0x00009826,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c9c,         0x00009c27,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001ca0,         0x0000a028,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001ca4,         0x0000a429,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001ca8,         0x0000a82a,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cac,         0x0000ac2b,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cb0,         0x0000b02c,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cb4,         0x0000b42d,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cb8,         0x0000b82e,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cbc,         0x0000bc2f,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cc0,         0x0000c030,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cc4,         0x0000c431,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cc8,         0x0000c832,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001ccc,         0x0000cc33,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cd0,         0x0000d034,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cd4,         0x0000d435,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cd8,         0x0000d836,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cdc,         0x0000dc37,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001ce0,         0x0000e038,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001ce4,         0x0000e439,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001ce8,         0x0000e83a,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cec,         0x0000ec3b,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cf0,         0x0000f03c,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cf4,         0x0000f43d,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cf8,         0x0000f83e,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001cfc,         0x0000fc3f,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d00,         0x00010040,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d04,         0x00010441,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d08,         0x00010842,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d0c,         0x00010c43,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d10,         0x00011044,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d14,         0x00011445,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d18,         0x00011846,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d1c,         0x00011c47,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d20,         0x00012048,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d24,         0x00012449,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d28,         0x0001284a,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d2c,         0x00012c4b,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d30,         0x0001304c,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d34,         0x0001344d,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00014000,         0x0007ffff,      1,    0x20,      7,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00014400,         0x0007ffff,      1,    0x20,      7,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00015400,         0x00000001,      1,    0x0                       }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}
/**
* @internal smemBobcat3SpecificDeviceUnitAlloc_DP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DP units -- (non sip units)
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemBobcat3SpecificDeviceUnitAlloc_DP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    GT_U32  unitIndex;

    /* need to know some of Errata during unit allocations and active memories */
    smemBobcat2ErrataCleanUp(devObjPtr);


    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_MG)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitMg(devObjPtr,currUnitChunkPtr);
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_GOP))
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP)];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3UnitGop(devObjPtr,currUnitChunkPtr);
        }
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_SERDES))
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_SERDES)];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3UnitLpSerdes(devObjPtr,currUnitChunkPtr);
        }
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_XG_PORT_MIB))
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_XG_PORT_MIB)];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3UnitXGPortMib(devObjPtr,currUnitChunkPtr);
        }
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_MPPM))
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_MPPM)];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3UnitMppm(devObjPtr,currUnitChunkPtr);
        }
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_RX_DMA)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitRxDma(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_DMA);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitTxDma(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_FIFO);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitTxFifo(devObjPtr,currUnitChunkPtr);
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_IA))
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_IA)];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3UnitIa(devObjPtr,currUnitChunkPtr);
        }
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TAI)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitTai(devObjPtr,currUnitChunkPtr);
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_BM))
    {
        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_BM);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3UnitBm(devObjPtr,currUnitChunkPtr);
        }
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_BMA);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitBma(devObjPtr,currUnitChunkPtr);
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_TXQ_DQ))
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_DQ)];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3UnitTxqDq(devObjPtr,currUnitChunkPtr);
        }
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_TXQ_LL))
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_LL)];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3UnitTxqLl(devObjPtr,currUnitChunkPtr);
        }
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_TXQ_QCN))
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_QCN)];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3UnitTxqQcn(devObjPtr,currUnitChunkPtr);
        }
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_TXQ_QUEUE))
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_QUEUE)];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3UnitTxqQueue(devObjPtr,currUnitChunkPtr);
        }
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_TXQ_BMX))
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_BMX)];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3UnitTxqBmx(devObjPtr,currUnitChunkPtr);
        }
    }


    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_FCU))
    {
        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_FCU);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3UnitFcu(devObjPtr,currUnitChunkPtr);
        }
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_SBC))
    {
        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_SBC);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3Unit__dummy(devObjPtr,currUnitChunkPtr);
        }
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_NSEC))
    {
        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_NSEC);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3Unit__dummy(devObjPtr,currUnitChunkPtr);
        }
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_NSEF))
    {
        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_NSEF);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3Unit__dummy(devObjPtr,currUnitChunkPtr);
        }
    }


    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_CPFC))
    {
        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_CPFC);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];

        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3UnitTxqPfc(devObjPtr,currUnitChunkPtr);
        }
    }

}

/**
* @internal smemBobcat3UnitEgfQag function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Lion3 EGF-QAG unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitEgfQag
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 1048576),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(48, 8),SMEM_BIND_TABLE_MAC(egfQagEVlanDescriptorAssignmentAttributes)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00800000, 131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(44, 8),SMEM_BIND_TABLE_MAC(egfQagEgressEPort)}
            /* TC_DP_Mapper_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00900000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(8, 4),SMEM_BIND_TABLE_MAC(egfQagTcDpMapper)}
            /* VOQ_Mapper_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00910000, 8192), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(6, 4)}
            /* Cpu_Code_To_Loopback_Mapper_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00920000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(11, 4), SMEM_BIND_TABLE_MAC(egfQagCpuCodeToLbMapper)}
            /* Port_Target_Attributes_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00921000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(17, 4),SMEM_BIND_TABLE_MAC(egfQagPortTargetAttribute)}
            /* Port_Enq_Attributes_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00922000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(10, 4),SMEM_BIND_TABLE_MAC(egfQagTargetPortMapper)}
            /* Port_Source_Attributes_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00923000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(4, 4) ,SMEM_BIND_TABLE_MAC(egfQagPortSourceAttribute)}
            /* EVIDX_Activity_Status_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00924000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00E00000, 0x00E00004)}
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
}

/**
* @internal smemBobcat3UnitTcam function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the TCAM unit
*/
static void smemBobcat3UnitTcam
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    GT_U32  tcamNumOfFloors = devObjPtr->tcamNumOfFloors;
    GT_U32  isSip6    = SMEM_CHT_IS_SIP6_GET(devObjPtr);

    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 1572864),
         SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(84, 16),SMEM_BIND_TABLE_MAC(tcamMemory)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00200000, 1572864),
         SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(242, 32),SMEM_BIND_TABLE_MAC(globalActionTable)},

        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00400000, 294912)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00500000, 0x00500010)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00500100, 0x00500100)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00500108, 0x00500108)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00500110, 0x00500110)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00500118, 0x00500118)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00500120, 0x00500120)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00500128, 0x00500128)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00500130, 0x00500130)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00500138, 0x00500138)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00500140, 0x00500140)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00500148, 0x00500148)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00500150, 0x00500150)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00500158, 0x00500158)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00501000, 0x00501010)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00502000, 0x005021C8)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x005021D0, 0x005021DC)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x005021E4, 0x005021F0)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x005021F8, 0x00502204)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0050220C, 0x00502218)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00502300, 0x0050230C)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00503000, 0x00503030)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0050303C, 0x0050303C)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00504010, 0x00504014)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00505000, 0x0050509C)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00506000, 0x00506058)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0050606C, 0x00506078)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00507000, 0x0050700C)}
    };
    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    /*<tcamMemory>*/
    chunksMem[0].numOfRegisters = (tcamNumOfFloors * 256 * 16/*banks*/ * 16/* line alignment*/ * 2/* X/Y*/)/4;
    /*<globalActionTable>*/
    chunksMem[1].numOfRegisters = (tcamNumOfFloors * 256 * 16/*banks*/ * 32/* line alignment*/)/4;

    if (isSip6)
    {
        chunksMem[1].enrtySizeBits = 247;
    }
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    if (isSip6)
    {
        SMEM_CHUNK_BASIC_STC  sip6ChunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00507080, 0x005077DC)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00508000, 0x005087F8)}
        };
        smemUnitChunkAddBasicChunk(devObjPtr, unitPtr, ARRAY_NAME_AND_NUM_ELEMENTS_MAC(sip6ChunksMem));
    }
}

/**
* @internal smemBobcat3UnitTti function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the TTI unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitTti
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000068)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x00000168)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000170, 0x00000180)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000190, 0x000001BC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001CC, 0x000001E4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000214)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000320)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000340, 0x00000340)}
            /*DSCP To DSCP Map - one entry per translation profile
              Each entry holds 64 DSCP values, one per original DSCP value. */
#define  lion3_dscpToDscpMap_size  64
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000400, 768),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(384 , lion3_dscpToDscpMap_size),SMEM_BIND_TABLE_MAC(dscpToDscpMap)}
            /*EXP To QosProfile Map - Each entry represents Qos translation profile
              Each entry holds QoS profile for each of 8 EXP values*/
#define  lion3_expToQoSProfile_size  16
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000700, 192),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(80, lion3_expToQoSProfile_size),SMEM_BIND_TABLE_MAC(expToQoSProfile)}
            /*DSCP To QosProfile Map - holds 12 profiles, each defining a 10bit QoSProfile per each of the 64 DSCP values : 0x400 + 0x40*p: where p (0-11) represents profile*/
#define  lion3_dscpToQoSProfile_size  128
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000800, 1536),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(640 , lion3_dscpToQoSProfile_size),SMEM_BIND_TABLE_MAC(dscpToQoSProfile)}
            /*UP_CFI To QosProfile Map - Each entry holds a QoS profile per each value of {CFI,UP[2:0]} */
#define  lion3_upToQoSProfile_size  32
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000E00, 384),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(160, lion3_upToQoSProfile_size),SMEM_BIND_TABLE_MAC(upToQoSProfile)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000013B0, 0x000013DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000013F8, 0x0000148C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x00001540)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000300C, 0x0000300C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003030, 0x00003034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003040, 0x0000307C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003090, 0x000030AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000, 0x000065FC)}

            /* Default Port Protocol eVID and QoS Configuration Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 65536), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(portProtocolVidQoSConf)}
            /* PCL User Defined Bytes Configuration Memory -- 70 udb's in each entry */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(770, 128),SMEM_BIND_TABLE_MAC(ipclUserDefinedBytesConf)}
            /* TTI User Defined Bytes Configuration Memory -- TTI keys based on UDB's : 8 entries support 8 keys*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 1280), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(330, 64),SMEM_BIND_TABLE_MAC(ttiUserDefinedBytesConf)}
            /* VLAN Translation Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00050000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(13, 4),SMEM_BIND_TABLE_MAC(ingressVlanTranslation)}
             /*Physical Port Attributes*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00100000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(58, 8),SMEM_BIND_TABLE_MAC(ttiPhysicalPortAttribute)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00110000, 0x00110400)}
             /*Physical Port Attributes 2*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00160000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(251, 32),SMEM_BIND_TABLE_MAC(ttiPhysicalPort2Attribute)}
             /* QCN to Pause Timer Map*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00300000, 256), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(16, 4),SMEM_BIND_TABLE_MAC(ttiQcnToPauseTimerMap)}
            /*Default ePort Attributes (pre-tti lookup eport table)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00210000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(123, 16),SMEM_BIND_TABLE_MAC(ttiPreTtiLookupIngressEPort)}
            /*ePort Attributes (post-tti lookup eport table)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00240000, 131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(17, 4),SMEM_BIND_TABLE_MAC(ttiPostTtiLookupIngressEPort)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);

    }

    { /* sip5: chunks with formulas */
        {
            SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
            {
                /* MAC2ME Registers */ /* registers not table -- SMEM_BIND_TABLE_MAC(macToMe)*/
                 {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00001600,0)}, FORMULA_TWO_PARAMETERS(6 , 0x4 , 128, 0x20)}
            };

            GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
            SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

            smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

            /*add the tmp unit chunks to the main unit */
            smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
        }
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000100,         0xffffffff,      4,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x000000f0,         0xffffffff,      4,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000018,         0x65586558,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000028,         0x000088e7,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000030,         0x88488847,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000034,         0x65586558,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000038,         0x00003232,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000014c,         0x00ffffff,      1,    0x0      }
/*            ,{DUMMY_NAME_PTR_CNS,            0x000001c8,         0x0000ffff,      1,    0x0      }*/
            ,{DUMMY_NAME_PTR_CNS,            0x00000300,         0x81008100,      4,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000310,         0xffffffff,      4,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000000,         0x30002503,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x00000020,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000068,         0x1b6db81b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000110,         0x0000004b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000118,         0x00001320,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000011c,         0x0000001b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000084,         0x000fff00,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000088,         0x3fffffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000008c,         0x3fffffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000200,         0x030022f3,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000204,         0x00400000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000208,         0x12492492,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000020c,         0x00092492,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000210,         0x0180c200,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0x88f788f7,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00003004,         0x013f013f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000300c,         0x00000570,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00003094,         0x88b588b5,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00003098,         0x00000001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001500,         0x00602492,      1,    0x0      }
/*            ,{DUMMY_NAME_PTR_CNS,            0x00001000,         0x3fffffff,     43,    0x4      }*/
            ,{DUMMY_NAME_PTR_CNS,            0x00001608,         0xffff0fff,    128,    0x20     }
            ,{DUMMY_NAME_PTR_CNS,            0x00001600,         0x00000fff,    128,    0x20     }
            ,{DUMMY_NAME_PTR_CNS,            0x0000160c,         0xffffffff,    128,    0x20     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000040,         0xff000000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000044,         0x00000001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000004c,         0xff020000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000050,         0xff000000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000054,         0xffffffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000058,         0xffffffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000005c,         0xffffffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000070,         0x00008906,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000150,         0x0000301f,      1,    0x0      }

            ,{DUMMY_NAME_PTR_CNS,            0x0000001c,         0x0000000e,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000003c,         0x0000000d,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000140,         0x20A6C01B,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000144,         0x24924924,      2,    0x4      }

            ,{DUMMY_NAME_PTR_CNS,            0x00000170,         0x0fffffff,      4,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x000013fc,         0x000001da,      1,    0x0      }

                ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemBobcat3UnitIpcl function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the IPCL unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitIpcl
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* PCL Unit Interrupt Cause */
        {0x00000004, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg,0},
        {0x00000008, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000000, 0x0000003C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000044, 0x00000048)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x0000005C, 0x0000005C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000070, 0x00000078)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000080, 0x000000BC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000005C0, 0x000005FC),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(pearsonHash)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000730, 0x0000073C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000744, 0x0000077C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000800, 0x00000850)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000C00, 512),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(74, 16),SMEM_BIND_TABLE_MAC(crcHashMask)}
            /* next are set below as formula of 3 tables
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 18432)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 18432)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00030000, 18432)}*/

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 7168),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(506, 64),SMEM_BIND_TABLE_MAC( ipcl0UdbSelect)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00042000, 7168),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(506, 64),SMEM_BIND_TABLE_MAC(ipcl1UdbSelect)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00044000, 7168),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(506, 64),SMEM_BIND_TABLE_MAC(ipcl2UdbSelect)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);

    }

    /* chunks with formulas */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* IPCL0,IPCL1,IPCL2 Configuration Table */
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00010000 ,18432), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(27, 4),SMEM_BIND_TABLE_MAC(pclConfig)}, FORMULA_SINGLE_PARAMETER(3 , 0x00010000)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;
        GT_U32  numMoreEntries = 512 ;/*BC3,Aldrin2*/

        /* number of entries : 4K + numMoreEntries . keep alignment and use for memory size */
        chunksMem[0].memChunkBasic.numOfRegisters = ((4*1024) + numMoreEntries) * (chunksMem[0].memChunkBasic.enrtyNumBytesAlignement / 4);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }


    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x00000001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x02801000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000018,         0x0000ffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000001c,         0x00000028,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000020,         0x00000042,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000034,         0x00000fff,      3,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000044,         0x3fffffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000048,         0x0000ffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000730,         0x00ff0080,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000738,         0x00080008,      1,    0x0      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };



        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemBobcat3UnitL2i function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Lion3 L2i unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitL2i
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x0000002C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000048)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x00000120)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000130, 0x00000130)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000140, 0x0000014C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000160, 0x00000164)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000170, 0x00000170)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000180, 0x0000019C)}

            /*,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x0000063C)} --> see ieeeRsrvMcCpuIndex */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x0000071C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x0000093C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C3C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D1C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000E08)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x0000112C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x0000120C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001220, 0x00001224)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x00001308)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001400, 0x0000140C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x00001508)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001510, 0x00001514)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001520, 0x00001528)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001530, 0x00001544)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001550, 0x00001564)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001600, 0x0000163C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002010, 0x00002010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002020, 0x00002020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x00002104)}
            /* Source Trunk Attribute Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00202000 ,4096 ),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(40, 8),SMEM_BIND_TABLE_MAC(bridgeIngressTrunk)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }

    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* IEEE Reserved Multicast Configuration register */ /* register and not table SMEM_BIND_TABLE_MAC(ieeeRsrvMcConfTable) */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000200 ,0),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(ieeeRsrvMcConfTable)}, FORMULA_TWO_PARAMETERS(16 , 0x4 , 8 ,0x80)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000600 ,0),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(ieeeRsrvMcCpuIndex)}, FORMULA_SINGLE_PARAMETER(16 , 0x4)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }
    {
        SMEM_CHUNK_BASIC_STC  chunksMemBc2A0[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x0000081C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A1C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B1C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001240, 0x000012BC)}
        };

        SMEM_CHUNK_BASIC_STC  chunksMemBc2B0[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000880)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A80)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B80)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001240, 0x000012DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002120, 0x00002120)}
        };

        if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr) == 0)
        {
            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMemBc2A0));
        }
        else
        {
            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMemBc2B0));
        }
    }

    /* to support bobk :
       add tables that depend on number of vlans and number of eport and number
       of physical ports */
    {
        GT_U32  numPhyPorts = devObjPtr->limitedResources.phyPort;
        GT_U32  numEPorts = devObjPtr->limitedResources.ePort;
        GT_U32  numEVlans = devObjPtr->limitedResources.eVid;
        GT_U32  numStg = 1 << devObjPtr->flexFieldNumBitsSupport.stgId;

        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /*Ingress Bridge physical Port Table*/
                 {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00200000 ,1024 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(29, 4),SMEM_BIND_TABLE_MAC(bridgePhysicalPortEntry)}
                /*Ingress Bridge physical Port Rate Limit Counters Table*/
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00201000 ,1024 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(22, 4),SMEM_BIND_TABLE_MAC(bridgePhysicalPortRateLimitCountersEntry)}
            };

            /* number of entries : numPhyPorts . keep alignment and use for memory size */
            chunksMem[0].numOfRegisters = numPhyPorts * (chunksMem[0].enrtyNumBytesAlignement / 4);
            /* number of entries : numPhyPorts . keep alignment and use for memory size*/
            chunksMem[1].numOfRegisters = numPhyPorts * (chunksMem[1].enrtyNumBytesAlignement / 4);

            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
        }

        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /* ingress EPort learn prio Table */
                {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00210000 ,8192 ),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(40, 8),SMEM_BIND_TABLE_MAC(bridgeIngressEPortLearnPrio)}
            };

            /* number of entries : numEPorts/8 . keep alignment and use for memory size */
            chunksMem[0].numOfRegisters = (numEPorts / 8) * (chunksMem[0].enrtyNumBytesAlignement / 4);

            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
        }

        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /* Ingress Spanning Tree State Table */
                {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00240000 , 262144 ),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(1024, 128),SMEM_BIND_TABLE_MAC(stp)}
            };
            GT_U32  numLines =
                (devObjPtr->support_remotePhysicalPortsTableMode) ?
                    numStg / 2 :  /* there are 2 'entries' per line */
                    numStg;

            /* each entry : 2 bits per physical port */
            chunksMem[0].enrtySizeBits = (2 * numPhyPorts);


            /* alignment is according to entry size */
            chunksMem[0].enrtyNumBytesAlignement = ((chunksMem[0].enrtySizeBits + 31)/32)*4;
            /* number of entries : numStg . use calculated alignment and use for memory size */
            chunksMem[0].numOfRegisters = (numLines) * (chunksMem[0].enrtyNumBytesAlignement / 4);

            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
        }

        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /* Ingress Port Membership Table */
                {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00280000 , 262144), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(512 , 64),SMEM_BIND_TABLE_MAC(bridgeIngressPortMembership)}
            };

            GT_U32  numLines =
                (devObjPtr->support_remotePhysicalPortsTableMode) ?
                    numEVlans / 2 : /* there are 2 'entries' per line */
                    numEVlans;

            /* each entry : 1 bit per physical port */
            chunksMem[0].enrtySizeBits = numPhyPorts;
            /* alignment is according to entry size */
            chunksMem[0].enrtyNumBytesAlignement = ((chunksMem[0].enrtySizeBits + 31)/32)*4;
            /* number of entries : numEVlans . use calculated alignment and use for memory size */
            chunksMem[0].numOfRegisters = (numLines ) * (chunksMem[0].enrtyNumBytesAlignement / 4) ;

            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
        }

        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /* Ingress Vlan Table */
                {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x002C0000 , 131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(113 , 16),SMEM_BIND_TABLE_MAC(vlan)}
            };

            /* number of entries : numEVlans . keep alignment and use for memory size */
            chunksMem[0].numOfRegisters = (numEVlans ) * (chunksMem[0].enrtyNumBytesAlignement / 4) ;

            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
        }

        {
            SMEM_CHUNK_BASIC_STC  chunksMemBc3[]=
            {
                /*Bridge Ingress ePort Table*/
                 {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00300000 ,131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(80 ,16),SMEM_BIND_TABLE_MAC(bridgeIngressEPort)}
                /* Ingress Span State Group Index Table */
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00218000 , 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12 , 4),SMEM_BIND_TABLE_MAC(ingressSpanStateGroupIndex)}
            };
            /* number of entries : numEPorts . keep alignment and use for memory size */
            chunksMemBc3[0].numOfRegisters = (numEPorts ) * (chunksMemBc3[0].enrtyNumBytesAlignement / 4) ;
            /* number of entries : numEVlans . keep alignment and use for memory size */
            chunksMemBc3[1].numOfRegisters = (numEVlans ) * (chunksMemBc3[1].enrtyNumBytesAlignement / 4) ;
            /* number of bits : stgId */
            chunksMemBc3[1].enrtySizeBits = devObjPtr->flexFieldNumBitsSupport.stgId;
            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMemBc3));
        }

    }


    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x06806004,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x00002240,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000020,         0x22023924,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000024,         0x00000000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000048,         0x0007e03f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000120,         0x52103210,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000130,         0x00001ffe,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000140,         0x05f205f2,      4,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000160,         0x00001fff,      2,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000164,         0x00000000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001000,         0x000007e7,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001200,         0x04d85f41,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001204,         0x18027027,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001208,         0x00019019,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000120c,         0x0000ffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001600,         0xffffffff,      8,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002000,         0x0000007f,      1,    0x0      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemBobcat3ActiveReadHaArpTsTable function
* @endinternal
*
* @brief   in BC3 this table shared between the 2 pipes
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemBobcat3ActiveReadHaArpTsTable
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    if(address & SECOND_PIPE_OFFSET_CNS)
    {
        /* smemMemGet(...) will get to specialSingleInstanceAddressInfoGet()*/
        /* and will return address of pipe 0 */
        GT_U32 * actualMemPtr = smemMemGet(devObjPtr,address);
        *outMemPtr = *actualMemPtr;
    }
    else
    {
        *outMemPtr = *memPtr;
    }

}

/**
* @internal smemBobcat3ActiveWriteHaArpTsTable function
* @endinternal
*
* @brief   in BC3 this table shared between the 2 pipes
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - memory size to be read.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to set register's content.
*/
void smemBobcat3ActiveWriteHaArpTsTable
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    if(address & SECOND_PIPE_OFFSET_CNS)
    {
        /* ignore : (memPtr) */

        /* for tables we need to go through the 'non-last word' operations */
        scibWriteMemory(devObjPtr->deviceId,
            (address & (~SECOND_PIPE_OFFSET_CNS)),
            1,
            inMemPtr);
    }
    else
    {
        *memPtr = *inMemPtr;
    }
}



/**
* @internal smemBobcat3UnitHa function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the HA unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitHa
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        HA_ACTIVE_MEM_MAC,

        /*SMEM_BIND_TABLE_MAC(arp)-- tunnelStart*/
        /*range = 0x00200000 from 0x00400000 : so need 2 ranges of active memory*/
        /* active memory for 0x00400000 till 0x004ffffc */
        /* active memory for 0x00500000 till 0x005ffffc */
        {0x00400000, 0xfff00000, smemBobcat3ActiveReadHaArpTsTable , 0 , smemBobcat3ActiveWriteHaArpTsTable , 0},
        {0x00500000, 0xfff00000, smemBobcat3ActiveReadHaArpTsTable , 0 , smemBobcat3ActiveWriteHaArpTsTable , 0},


        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000058)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000060, 0x00000070)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x0000009C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000144)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000150, 0x0000016C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000314)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003C0, 0x000003C4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003D0, 0x000003D0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000408, 0x00000410)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000420, 0x00000424)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000430, 0x00000434)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x0000051C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000550, 0x00000560)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x0000063C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000710, 0x00000714)}
            /*HA Physical Port Table 1*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(27, 4) , SMEM_BIND_TABLE_MAC(haEgressPhyPort1)}
            /*HA Physical Port Table 2*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 8192), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(81, 16) , SMEM_BIND_TABLE_MAC(haEgressPhyPort2)}
            /*HA QoS Profile to EXP Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004000 , 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(3, 4) , SMEM_BIND_TABLE_MAC(haQosProfileToExp)}
            /*HA Global MAC SA Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00005000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(48, 8) , SMEM_BIND_TABLE_MAC(haGlobalMacSa)}
            /*EVLAN Table (was 'vlan translation' in legacy device)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 65536), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(36, 8) , SMEM_BIND_TABLE_MAC(egressVlanTranslation)}
            /*VLAN MAC SA Table (was 'VLAN/Port MAC SA Table' in legacy device)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(8, 4) , SMEM_BIND_TABLE_MAC(vlanPortMacSa)}
            /*PTP Domain table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 20480) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(35, 8) , SMEM_BIND_TABLE_MAC(haPtpDomain)}
            /*Generic TS Profile table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00050000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(537,128), SMEM_BIND_TABLE_MAC(tunnelStartGenericIpProfile) }
            /*EPCL User Defined Bytes Configuration Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(600,128) , SMEM_BIND_TABLE_MAC(haEpclUserDefinedBytesConfig)}
            /*HA Egress ePort Attribute Table 1*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00100000, 262144), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(88,16) , SMEM_BIND_TABLE_MAC(haEgressEPortAttr1)}
            /*HA Egress ePort Attribute Table 2*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00200000, 65536), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(25, 4) , SMEM_BIND_TABLE_MAC(haEgressEPortAttr2)}
            /*Router ARP DA and Tunnel Start Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00400000, 2097152), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(384,64) , SMEM_BIND_TABLE_MAC(arp)/*tunnelStart*/}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

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
           ,{NULL,               0,          0x00000000, 0,     0x0}

        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemBobcat3UnitEq function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Eq unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitEq
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        /*ingress STC interrupt register*/
        /* need to support 0x00900620 - 32 registers in steps of 0x10 */
        /*   and Summary Cause Register in  0x09000600                */
        /*                                                            */
        /*   so 0x09000600 .. 0x09000810                              */
        /* 0x0B000600 .. 0x0B0006F0 , steps of 0x10 */
        {0x00000600, 0xFFFFFF0F, smemChtActiveReadIntrCauseReg, 24 , smemChtActiveWriteIntrCauseReg,0},
        /* 0x0B000700 .. 0x0B0007F0 , steps of 0x10 */
        {0x00000700, 0xFFFFFF0F, smemChtActiveReadIntrCauseReg, 24 , smemChtActiveWriteIntrCauseReg,0},
        /* 0x0B000800 .. 0x0B000810 , steps of 0x10 */
        {0x00000800, 0xFFFFFFEF, smemChtActiveReadIntrCauseReg, 24 , smemChtActiveWriteIntrCauseReg,0},

        /* rest of memories */
        ACTIVE_MEM_EQ_WITHOUT_INT_STC_INT_COMMON_MAC,

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000001C, 0x00000020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000003C, 0x00000040)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000058, 0x0000007C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000088, 0x0000008C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x0000060C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000110, 0x0000011C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005010, 0x00005018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005020, 0x00005024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005030, 0x00005030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005050, 0x00005054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000, 0x0000703C),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(tcpUdpDstPortRangeCpuCode)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007800, 0x0000783C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007C00, 0x00007C10),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(ipProtCpuCode)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A000, 0x0000A000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A008, 0x0000A008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A010, 0x0000A010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A020, 0x0000A034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A100, 0x0000A13C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000AA00, 0x0000AA08)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000AB00, 0x0000AB5C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000AC00, 0x0000AC08)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000AD00, 0x0000AD70)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000AF00, 0x0000AF08)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000AF10, 0x0000AF10)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000AF30, 0x0000AF30)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000B000, 0x0000B008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000B200, 0x0000B218)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x0000B300, 256),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(eqPhysicalPortIngressMirrorIndexTable)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D000, 0x0000D004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D010, 0x0000D010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00010000, 0x0001000C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002000C, 0x00020010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 8192)}/*SMEM_BIND_TABLE_MAC(ingrStc)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00080000, 0x000803F8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00100000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(statisticalRateLimit)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00110000, 4096),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(14, 4),SMEM_BIND_TABLE_MAC(qosProfile)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00120000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(cpuCode)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00130000, 1024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00160000, 16384),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(28, 4),SMEM_BIND_TABLE_MAC(eqTrunkLtt)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00700000, 16384),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(eqIngressEPort)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00800000, 65536),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(28, 4),SMEM_BIND_TABLE_MAC(eqL2EcmpLtt)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00900000, 65536),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(24, 4),SMEM_BIND_TABLE_MAC(eqL2Ecmp)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00A00000, 2048),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(txProtectionSwitchingTable)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00A80000, 32768),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(26, 4),SMEM_BIND_TABLE_MAC(ePortToLocMappingTable)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00B00000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(oamProtectionLocStatusTable)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00B10000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(oamTxProtectionLocStatusTable)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00C00000, 65536),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(23, 4),SMEM_BIND_TABLE_MAC(eqE2Phy)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
    /* chunks with formulas */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000620 ,0)}, FORMULA_TWO_PARAMETERS(2,0x4, 32,0x10)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00010020 ,0)}, FORMULA_TWO_PARAMETERS(2,0x4, 32,0x10)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[]  =
        {
             {DUMMY_NAME_PTR_CNS,         0x0000001c,         0x00000498,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00005000,         0x00000004,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00005020,         0x00000020,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00005024,         0xffffffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00005030,         0x000001e0,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00005054,         0x0000003f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000a000,         0x3ef084e2,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000a010,         0x00000160,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000a020,         0x000007e0,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000a100,         0xffffffff,      16,    0x4     }
            ,{DUMMY_NAME_PTR_CNS,         0x0000af00,         0x0000211f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000af04,         0x00004201,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000af08,         0x00007fff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000af10,         0x0000421f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000af30,         0x0000ffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000b000,         0x00004e01,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000b004,         0x00000801,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000b200,         0x02000000,      7,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000d000,         0x00000020,      1,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000d004,         0xffffffff,      1,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000d010,         0x000001E0,      1,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,         0x00020010,         0x00000063,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00080000,         0x0000ffff,    255,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,         0x00100000,         0xffffffff,    256,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,         0x00130000,         0x0fffffff,    256,    0x4      }
            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}
/**
* @internal smemBobcat3UnitEgfEft function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EGF-EFT unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitEgfEft
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
                /* Secondary Target Port Table */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(8, 4) , SMEM_BIND_TABLE_MAC(secondTargetPort)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001020, 0x00001020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001030, 0x00001030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001040, 0x00001040)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000010A0, 0x000010A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000010B0, 0x000010B0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001110, 0x00001110)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001130, 0x00001130)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001140, 0x00001140)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001150, 0x0000138C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x00002100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002108, 0x00002108)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000021D0, 0x000021D0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002200, 0x000022BC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x0000309C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003100, 0x000031FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004010, 0x0000402C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00010000, 0x00010004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00010040, 0x000101FC)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);


        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00001000,         0x000007e7,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001020,         0x08e00800 ,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001030,         0x0000000e,      1,    0x0      }
/*            ,{DUMMY_NAME_PTR_CNS,            0x00001040,         0x3f3f3f3f,      4,    0x4      }*/
            ,{DUMMY_NAME_PTR_CNS,            0x00001110,         0xffff0000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001150,         0x00007E3F ,     128,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002108,         0x08080808,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x000021d0,         0x0870e1c3,      1,    0x0      }
/*            ,{DUMMY_NAME_PTR_CNS,            0x000021e0,         0x000002c3,      1,    0x0      }*/
/*            ,{DUMMY_NAME_PTR_CNS,            0x000021f4,         0x55555555,      4,    0x10     }
            ,{DUMMY_NAME_PTR_CNS,            0x000021f8,         0xaaaaaaaa,      4,    0x10     }
            ,{DUMMY_NAME_PTR_CNS,            0x000021fc,         0xffffffff,      1,    0x0      }*/
            ,{DUMMY_NAME_PTR_CNS,            0x0000220c,         0xffffffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000221c,         0xffffffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000222c,         0xffffffff,      2,    0x24     }
            ,{DUMMY_NAME_PTR_CNS,            0x00002254,         0xffffffff,      3,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0x00000010,      8,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00010000,         0x00000801,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00010004,         0x000fff39,      1,    0x0      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemBobcat3UnitEgfSht function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EGF-SHT unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitEgfSht
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {

        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* L2 Port Isolation Table */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000000 ,69632), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(256, 32),SMEM_BIND_TABLE_MAC(l2PortIsolation)}
            /* Egress Spanning Tree State Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x02040000 , 131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(256, 32),SMEM_BIND_TABLE_MAC(egressStp)}
            /* Non Trunk Members 2 Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x02060000 , 131072 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(256, 32),SMEM_BIND_TABLE_MAC(nonTrunkMembers2)}
            /* Source ID Members Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x02080000 , 131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(256, 32),SMEM_BIND_TABLE_MAC(sst)}
            /* Eport EVlan Filter*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x020A0000, 131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64, 8),SMEM_BIND_TABLE_MAC(egfShtEportEVlanFilter)}
            /* Multicast Groups Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x020C0000 , 262144), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(512, 64),SMEM_BIND_TABLE_MAC(mcast)}
             /* Device Map Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x02100000 ,  16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(16, 4),SMEM_BIND_TABLE_MAC(deviceMapTable)}
             /* Vid Mapper Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x02110000 ,  32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(egfShtVidMapper)}
             /* Designated Port Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x02120000 ,   8192), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(512, 64),SMEM_BIND_TABLE_MAC(designatedPorts)}
            /* Egress EPort table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x02200000 , 65536), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(15, 4),SMEM_BIND_TABLE_MAC(egfShtEgressEPort)}
            /* Non Trunk Members Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x02400000 , 16384 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(512, 64),SMEM_BIND_TABLE_MAC(nonTrunkMembers)}
            /* Egress vlan table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x03000000 ,262144), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(256, 32),SMEM_BIND_TABLE_MAC(egressVlan)}
            /* EVlan Attribute table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x03400000 , 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(6, 4),SMEM_BIND_TABLE_MAC(egfShtEVlanAttribute)}
            /* EVlan Spanning table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x03440000 , 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(egfShtEVlanSpanning)}
            /* L3 Port Isolation Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x04000000 ,69632), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(256, 32),SMEM_BIND_TABLE_MAC(l3PortIsolation)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020000, 0x06020010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020020, 0x06020020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020030, 0x06020030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020040, 0x0602083C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x06020880, 0x06020900)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x06020000,               0x00004007, 1,     0x0}
            ,{DUMMY_NAME_PTR_CNS,           0x06020004,               0x00000001, 1,     0x0}
            ,{DUMMY_NAME_PTR_CNS,           0x06020008,               0x0000000f, 1,     0x0}
            ,{DUMMY_NAME_PTR_CNS,           0x06020040,               0xffffffff, 16,    0x4}
            ,{DUMMY_NAME_PTR_CNS,           0x06020140,               0xffffffff, 16,    0x4}
            ,{DUMMY_NAME_PTR_CNS,           0x060201c0,               0xffffffff, 16,    0x4}
             /* Default value of 'UplinkIsTrunk - 0x1 = Trunk;' like in legacy devices */
            ,{DUMMY_NAME_PTR_CNS,           0x02100000,               0x00000002, 4096,  0x4}
            ,{DUMMY_NAME_PTR_CNS,           0x02120000,               0xffffffff, 2048,  0x4}
            ,{DUMMY_NAME_PTR_CNS,           0x020a0000,               0xffffffff, 32768, 0x4}
             /* non trunk members */
            ,{DUMMY_NAME_PTR_CNS,           0x02400000,               0xffffffff, 4096,  0x4}
            ,{DUMMY_NAME_PTR_CNS,           0x02080000,               0xffffffff, 32768, 0x4}
             /* vidx 0xfff - 512 members */
            ,{DUMMY_NAME_PTR_CNS,           0x020C0000 +(0xfff*0x40), 0xffffffff, 16,    0x4}
            /* vlan 1 members */
            ,{DUMMY_NAME_PTR_CNS,           0x03000000 + (1*0x20),    0xffffffff, 8,     0x4}

            ,{NULL, 0, 0x00000000, 0, 0x0}
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }



}

/**
* @internal smemBobcat3UnitOamUnify function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the any of the 2 OAMs:
*         1. ioam
*         2. eoam
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
* @param[in] bindTable                - bind table with memory chunks
*/
static void smemBobcat3UnitOamUnify
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr,
    IN GT_BOOL bindTable
)
{

    /* chunks with flat memory (no formulas) */
    {
        /* start with tables */
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000014, 0x0000003C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000050, 0x0000005C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x0000007C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000084, 0x00000088)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000090, 0x000000E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000F0, 0x00000148)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000160, 0x00000160)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x0000021C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x0000031C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x0000041C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x0000051C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x0000061C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x0000071C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x0000081C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x0000091C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A1C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A40, 0x00000A44)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A50, 0x00000A58)}
            /* Aging Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00007000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4), SMEM_BIND_TABLE_MAC(oamAgingTable)}
            /* Meg Exception Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4), SMEM_BIND_TABLE_MAC(oamMegExceptionTable)}
            /* Source Interface Exception Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00018000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4), SMEM_BIND_TABLE_MAC(oamSrcInterfaceExceptionTable)}
            /* Invalid Keepalive Hash Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4), SMEM_BIND_TABLE_MAC(oamInvalidKeepAliveHashTable)}
            /* Excess Keepalive Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00028000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4), SMEM_BIND_TABLE_MAC(oamExcessKeepAliveTable)}
            /* OAM Exception Summary Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00030000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4), SMEM_BIND_TABLE_MAC(oamExceptionSummaryTable)}
            /* RDI Status Change Exception Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00038000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4), SMEM_BIND_TABLE_MAC(oamRdiStatusChangeExceptionTable)}
            /* Tx Period Exception Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4), SMEM_BIND_TABLE_MAC(oamTxPeriodExceptionTable)}
            /* OAM Opcode Packet Command Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(40, 8), SMEM_BIND_TABLE_MAC(oamOpCodePacketCommandTable)}
            /* OAM Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00080000, 262144), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(104, 16), SMEM_BIND_TABLE_MAC(oamTable)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);


        GT_U32  ii;

        for(ii = 0 ; ii < numOfChunks; ii++)
        {
            if (bindTable == GT_FALSE)
            {
                /* make sure that table are not bound to eoam(only to ioam) */
                chunksMem[ii].tableOffsetValid = 0;
                chunksMem[ii].tableOffsetInBytes = 0;
            }
        }

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}

/**
* @internal smemBobcat3UnitIOam function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the IOAM
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitIOam
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    smemBobcat3UnitOamUnify(devObjPtr, unitPtr, GT_TRUE);
}
/**
* @internal smemBobcat3UnitEOam function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EOAM
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobcat3UnitEOam
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    smemBobcat3UnitOamUnify(devObjPtr, unitPtr, GT_FALSE);
}

/**
* @internal smemBobcat3UnitIpvx function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the ipvx unit
*/
static void smemBobcat3UnitIpvx
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000010)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000024)}
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
* @internal smemBobcat3UnitMll function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the MLL unit
*/
static void smemBobcat3UnitMll
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(20, 4),SMEM_BIND_TABLE_MAC(l2MllLtt)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00100000, 1048576), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(162, 32) , SMEM_BIND_TABLE_MAC(mll)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

}

/**
* @internal smemBobcat3SpecificDeviceUnitAlloc_SIP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         SIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemBobcat3SpecificDeviceUnitAlloc_SIP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    GT_U32  unitIndex;

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_IPLR);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitIplr0(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_IPLR1);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitIplr1(devObjPtr, currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_EPLR);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitEplr(devObjPtr,currUnitChunkPtr);
    }


    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_EGF_QAG);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitEgfQag(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TTI);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitTti(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TCAM);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitTcam(devObjPtr, currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_IPCL);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitIpcl(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_L2I);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitL2i(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_HA);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitHa(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_EQ);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitEq(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_EGF_EFT);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitEgfEft(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_EGF_SHT);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitEgfSht(devObjPtr,currUnitChunkPtr);
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_IOAM))
    {
        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_IOAM);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3UnitIOam(devObjPtr,currUnitChunkPtr);
        }
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_EOAM))
    {
        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_EOAM);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobcat3UnitEOam(devObjPtr,currUnitChunkPtr);
        }
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_IPVX);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitIpvx(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_MLL);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobcat3UnitMll(devObjPtr,currUnitChunkPtr);
    }
}

/**
* @internal smemBobcat3SpecificDeviceUnitAlloc function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemBobcat3SpecificDeviceUnitAlloc
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
        UNIT_INFO_STC   *unitInfoPtr = &bobcat3units[0];

        for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
        {
            jj = unitInfoPtr->base_addr >> SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS;

            currUnitChunkPtr = &devMemInfoPtr->unitMemArr[jj];

            if(currUnitChunkPtr->chunkIndex != jj)
            {
                skernelFatalError("smemBobcat3SpecificDeviceUnitAlloc : not matched index");
            }
            currUnitChunkPtr->numOfUnits = unitInfoPtr->size;
        }
    }

    /* allocate the specific units that we NOT want the bc2_init , lion3_init , lion2_init
       to allocate. */

    smemBobcat3SpecificDeviceUnitAlloc_DP_units(devObjPtr);

    smemBobcat3SpecificDeviceUnitAlloc_SIP_units(devObjPtr);



}

/* bind the GOP unit to it's active mem */
static void bindUnitTaiActiveMem(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    /* TAI subunit */
    /* Time Counter Function Configuration 0 - Function Trigger */
    {0x10, 0xFFFFFFFF, NULL, 0 , smemLion3ActiveWriteTodFuncConfReg, 0},
    /* time Capture Value 0 Frac Low */
    {0x84, 0xFFFFFFFF, smemBobcat2ActiveReadTodTimeCaptureValueFracLow, 0 , smemChtActiveWriteToReadOnlyReg, 0},
    /* time Capture Value 1 Frac Low */
    {0xA0, 0xFFFFFFFF, smemBobcat2ActiveReadTodTimeCaptureValueFracLow, 1 , smemChtActiveWriteToReadOnlyReg, 0},
    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
}

/* NOTE: the cnc0 and the CNC1 are split , because can't share the same
    'static DB' because hold different base addresses !!!! */

/* bind the CNC0 unit to it's active mem */
static void bindUnitCnc0ActiveMem(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
/*CNC_0 ,CNC_1 */
    ACTIVE_MEM_CNC_BC3_COMMON_MAC,

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
}

/* bind the CNC1 unit to it's active mem */
static void bindUnitCnc1ActiveMem(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
/*CNC_0 ,CNC_1 */
    ACTIVE_MEM_CNC_BC3_COMMON_MAC,

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
}


/**
* @internal smemBobcat3SpecificDeviceMemInitPart2 function
* @endinternal
*
* @brief   specific part 2 of initialization that called from init 1 of Bobcat3
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemBobcat3SpecificDeviceMemInitPart2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{

    SMEM_SIP5_PP_REGS_ADDR_STC      *regAddrDbSip5Ptr;
    SMEM_UNIT_CHUNKS_STC            *currUnitChunkPtr;
    SMEM_CHT_GENERIC_DEV_MEM_INFO   *devMemInfoPtr = devObjPtr->deviceMemory;

    /* bind active memories to units ... need to be before
        smemBobcat2SpecificDeviceMemInitPart2 ... so bc2 will not override it */
/*    bindActiveMemoriesOnUnits(devObjPtr);*/

    /* call bobcat2 */
    smemBobcat2SpecificDeviceMemInitPart2(devObjPtr);

    /* Align IA unit addresses */
    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_IA))
    {
        SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,
                ingressAggregator[0], UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_IA));
        currUnitChunkPtr = &devMemInfoPtr->
            unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr, UNIT_IA)];
        regAddrDbSip5Ptr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr);
        smemGenericRegistersArrayAlignToUnit(devObjPtr,
            REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->ingressAggregator[0]),
            currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TAI)];
    if(IS_VALID_AND_EMPTY_ACTIVE_MEM_ARR_FOR_CHUNK_MAC(currUnitChunkPtr))
    {
        bindUnitTaiActiveMem(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_CNC)];
    if(IS_VALID_AND_EMPTY_ACTIVE_MEM_ARR_FOR_CHUNK_MAC(currUnitChunkPtr))
    {
        bindUnitCnc0ActiveMem(devObjPtr,currUnitChunkPtr);
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_CNC_1))
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_CNC_1)];
        if(IS_VALID_AND_EMPTY_ACTIVE_MEM_ARR_FOR_CHUNK_MAC(currUnitChunkPtr))
        {
            bindUnitCnc1ActiveMem(devObjPtr,currUnitChunkPtr);
        }
    }
}


/**
* @internal specialSingleInstanceAddressInfoGet function
* @endinternal
*
* @brief   the function check if the address belongs to special memory space.
*         NOTE: currently implemented only for tables because the ARP/TS table is
*         shared between 2 units (although the HA unit is duplicated to 2 pipes)
*         if not --> return value of UNIT_TYPE_NOT_VALID
*         if yes -> return the 'instance type' (pipe 0/1/shared)
* @param[in] devObjPtr                - pointer to device object.
* @param[in] address                  - the  to access to.
* @param[in] accessFromCpu            - indication that the CPU is accessing the memory.
*/
static UNIT_TYPE_ENT  specialSingleInstanceAddressInfoGet(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  address,
    IN GT_BOOL                 accessFromCpu,
    OUT SKERNEL_DEVICE_OBJECT ** newDevObjPtrPtr,
    IN GT_U32                  *newAddressPtr
)
{
    GT_U32  ii;
    SPECIAL_INSTANCE_TABLES_ARR_STC *specialTableInfoPtr;
    GT_U32  newAddr;
    GT_U32  pipeIteration;

    specialTableInfoPtr = &specialSingleInstanceTablesArr[0];
    if(newDevObjPtrPtr)
    {
        (*newDevObjPtrPtr) = devObjPtr;
    }

    for(ii = 0 ; specialTableInfoPtr->tableOffsetValid != 0; ii++,specialTableInfoPtr++)
    {
        for(pipeIteration = 0 ; pipeIteration < 2/*devObjPtr->numOfPipes*/ ; pipeIteration++)
        {
            if(pipeIteration == 0)
            {
                /* remove the pipeId */
                newAddr = address & (~specialTableInfoPtr->pipeOffset);
            }
            else
            {
                /* add the pipeId */
                newAddr = address | specialTableInfoPtr->pipeOffset;
            }

            if(newAddr >= specialTableInfoPtr->startAddr &&
               newAddr <= specialTableInfoPtr->lastAddr)
            {
                *newAddressPtr = newAddr;

                /* address belongs to 'Special tables' that number of instances
                   that can be different than the number on instances of the UNIT ! */
                if(accessFromCpu == GT_TRUE)
                {
                    return  specialTableInfoPtr->table_unitType_cpu_access;
                }
                else
                {
                    return  specialTableInfoPtr->table_unitType_deviceAccess;
                }
            }
        }/*pipeIteration*/
    }/*ii*/

    return UNIT_TYPE_NOT_VALID;
}


/**
* @internal smemBobcat3ConvertDevAndAddrToNewDevAndAddr function
* @endinternal
*
* @brief   Bobcat3 : Convert (dev,address) to new (dev,address).
*         needed for multi-pipe device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] address                  -  of memory(register or table).
* @param[in] accessType               - the access type
*                                       None
*
* @note function MUST be called before calling smemFindMemory()
*
*/
static void smemBobcat3ConvertDevAndAddrToNewDevAndAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  address,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    OUT SKERNEL_DEVICE_OBJECT * *newDevObjPtrPtr,
    OUT GT_U32                  *newAddressPtr
)
{

    GT_U32  unitIndex;
    GT_U32  newAddress;
    GT_U32 currentPipeId;
    GT_U32 pipeOffset , pipeId = 0;
    UNIT_TYPE_ENT unitType;
    UNIT_TYPE_ENT specialUnitType;

    *newDevObjPtrPtr = devObjPtr;

    specialUnitType = specialSingleInstanceAddressInfoGet(devObjPtr,address,
        (0 == IS_SKERNEL_OPERATION_MAC(accessType)) ?
        GT_TRUE :   /* access from CPU */
        GT_FALSE,   /* access from DEVICE */
        NULL,
        &newAddress);
    if(specialUnitType != UNIT_TYPE_NOT_VALID)
    {
        /* already got new address */
        *newAddressPtr = newAddress;
        return;
    }

    if(0 == IS_SKERNEL_OPERATION_MAC(accessType)||
       IS_DFX_OPERATION_MAC(accessType))
    {
        /* the CPU access 'pipe 0/1' explicitly */
        /* the DFX is only on 'pipe 0' */
        /* so no address modifications */
        *newAddressPtr = address;
        return;
    }
    /*smemBobcat3UnitPipeOffsetGet(...)*/
    pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(devObjPtr, address , &pipeId);
    if(pipeId != 0)
    {
        /* explicit access to pipe 1 , do not modify */
        *newAddressPtr = address;
        return;
    }

    currentPipeId = smemGetCurrentPipeId(devObjPtr);

    if(currentPipeId == 0)
    {
        /* no address update needed for pipe 0 accessing */
        *newAddressPtr = address;
        return;
    }

    /* now need to check how to convert the address */

    /* bobcat3UnitTypeArr[] indexed by '8 MSB' index */
    unitIndex = address >> (1 + SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS);

    unitType = bobcat3UnitTypeArr[unitIndex];

    switch(unitType)
    {
        case UNIT_TYPE_PIPE_0_AND_PIPE_1_E:
            /* no change in address as it is shared between the pipes */
            newAddress = address;
            break;
        case UNIT_TYPE_PIPE_0_ONLY_E:
            /* we access pipe 0 units , but we mean to access the pipe 1 units ! */
            newAddress = address | pipeOffset;
            break;
        default:
        case UNIT_TYPE_NOT_VALID:
            /* no change in address as it is not valid unit ...
               it will cause fatal error , somewhere next ... */
            newAddress = address;
            break;
        case UNIT_TYPE_PIPE_1_ONLY_E:
            /* we are in 'pipe 1 aware mode' ?! */

            /* should not happen */
            skernelFatalError("smemBobcat3ConvertDevAndAddrToNewDevAndAddr: 'pipe 1 device' not designed for 'aware mode' \n");

            newAddress = address;
            break;
    }

    *newAddressPtr = newAddress;

    return;
}

/**
* @internal smemBobcat3PreparePipe1Recognition function
* @endinternal
*
* @brief   prepare pipe 1 recognition
*
* @param[in] devObjPtr                - pointer to device object of pipe 0
*/
static void smemBobcat3PreparePipe1Recognition
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr;
    UNIT_INFO_STC   *unitInfoPtr = &bobcat3units[0];
    GT_U32  unitIndex;
    UNIT_TYPE_ENT   unitType;
    GT_U32  ii,jj;
    SPECIAL_INSTANCE_TABLES_ARR_STC *specialTableInfoPtr;
    SKERNEL_TABLE_4_PARAMETERS_INFO_STC *tableInfoPtr;
    SMEM_CHUNK_STC    *memChunkPtr;

    /* point pipe1 to look directly to pipe 0 memory */
    /* NOTE: the function smemBobcat3ConvertDevAndAddrToNewDevAndAddr will help
       any memory access via pipe1 to access proper unit in the 'pipe 0 device' */
    commonDevMemInfoPtr = devObjPtr->deviceMemory;
    commonDevMemInfoPtr->smemConvertDevAndAddrToNewDevAndAddrFunc =
        smemBobcat3ConvertDevAndAddrToNewDevAndAddr;


    /* NOTE: at this stage the units of pipe 1 are already allocated by 'pipe 0 device' */

    /* set 'pipe1' to point on the 'high' units as if they are in the 'low' units */

    for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
    {
        if(unitInfoPtr->orig_nameStr ==
            SHARED_BETWEEN_PIPE_0_AND_PIPE_1_INDICATION_CNS)
        {
            /* this unit used both by pipe 0 and pip 1 */
            unitType = UNIT_TYPE_PIPE_0_AND_PIPE_1_E;
        }
        else
        if(unitInfoPtr->pipeOffset)
        {
            /* this unit belongs to pip 1 */
            unitType = UNIT_TYPE_PIPE_1_ONLY_E;
        }
        else
        {
            /* this unit belongs to pip 0 */
            unitType = UNIT_TYPE_PIPE_0_ONLY_E;
        }

        /* bobcat3UnitTypeArr[] indexed by '8 MSB' index */
        unitIndex = unitInfoPtr->base_addr >> (1 + SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS);

        for(jj = 0 ; jj < unitInfoPtr->size ; jj++)
        {
            /*  Don't override unit type - if already defined for pipe 0 */
            if (bobcat3UnitTypeArr[unitIndex + jj] == UNIT_TYPE_NOT_VALID)
            {
                bobcat3UnitTypeArr[unitIndex + jj] = unitType;
            }
        }
    }

    /* init the <startAddr> , <lastAddr> in the 'special tables' */
    specialTableInfoPtr = &specialSingleInstanceTablesArr[0];
    for(ii = 0 ; specialTableInfoPtr->tableOffsetValid != 0; ii++,specialTableInfoPtr++)
    {
        tableInfoPtr = (SKERNEL_TABLE_4_PARAMETERS_INFO_STC*)
            ((GT_U8*)(void*)(&devObjPtr->tablesInfo)+ specialTableInfoPtr->tableOffsetInBytes);

        memChunkPtr = tableInfoPtr->commonInfo.memChunkPtr;

        specialTableInfoPtr->startAddr = memChunkPtr->memFirstAddr;
        specialTableInfoPtr->lastAddr  = memChunkPtr->memLastAddr;
        /*smemBobcat3UnitPipeOffsetGet*/
        specialTableInfoPtr->pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(devObjPtr, specialTableInfoPtr->startAddr , NULL);
    }

}

#define UNIT_IN_PIPE_1_MAC(orig_unitName) \
    orig_unitName##_INSTANCE_1

#define ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(orig_unitName , regDbUnitName, dpIndex) \
    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_IN_PIPE_1_MAC(orig_unitName)))           \
    {                                                                                                                                             \
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_IN_PIPE_1_MAC(orig_unitName))]; \
        smemGenericRegistersArrayAlignToUnit(devObjPtr,                                                                                                     \
            REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->regDbUnitName[dpIndex]),                                                                       \
            currUnitChunkPtr);                                                                                                                    \
                                                                                                                                                  \
        unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_IN_PIPE_1_MAC(orig_unitName));                                    \
        SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,regDbUnitName[dpIndex]           ,unitBaseAddress);                               \
    }


/**
* @internal smemBobcat3InitRegDbPipe1 function
* @endinternal
*
* @brief   Init RegDb of pipe 1.
*/
static void smemBobcat3InitRegDbPipe1
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    SMEM_SIP5_PP_REGS_ADDR_STC      *regAddrDbSip5Ptr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr);
    GT_U32  unitBaseAddress;

    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_RX_DMA    , rxDMA       ,3);/*DP[3]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_RX_DMA_1  , rxDMA       ,4);/*DP[4]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_RX_DMA_2  , rxDMA       ,5);/*DP[5]*/

    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_IA        , ingressAggregator ,1);

    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TX_DMA    , TXDMA       ,3);/*DP[3]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TX_DMA_1  , TXDMA       ,4);/*DP[4]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TX_DMA_2  , TXDMA       ,5);/*DP[5]*/

    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TX_FIFO   , TXFIFO      ,3);/*DP[3]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TX_FIFO_1 , TXFIFO      ,4);/*DP[4]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TX_FIFO_2 , TXFIFO      ,5);/*DP[5]*/

    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TXQ_DQ    , SIP5_TXQ_DQ ,3);/*DP[3]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TXQ_DQ_1  , SIP5_TXQ_DQ ,4);/*DP[4]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TXQ_DQ_2  , SIP5_TXQ_DQ ,5);/*DP[5]*/

    /* TXQ DQ */
    devObjPtr->tablesInfo.Scheduler_State_Variable.commonInfo.multiInstanceInfo.numBaseAddresses = 6;
    devObjPtr->tablesInfo.Scheduler_State_Variable.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.txqDq[0];

    devObjPtr->tablesInfo.egressStc.commonInfo.multiInstanceInfo.numBaseAddresses = 6;
    devObjPtr->tablesInfo.egressStc.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.txqDq[0];

    INIT_LPM_UNIT_MULTI_INSTANCE_TABLES_MAC(lpmEcmp);
    INIT_LPM_UNIT_MULTI_INSTANCE_TABLES_MAC(lpmIpv4VrfId);
    INIT_LPM_UNIT_MULTI_INSTANCE_TABLES_MAC(lpmIpv6VrfId);
    INIT_LPM_UNIT_MULTI_INSTANCE_TABLES_MAC(lpmFcoeVrfId);

}


/**
* @internal smemBobcat3IsDeviceMemoryOwner function
* @endinternal
*
* @brief   Return indication that the device is the owner of the memory.
*         relevant to multi pipe where there is 'shared memory' between pipes.
*
* @retval GT_TRUE                  - the device is   the owner of the memory.
* @retval GT_FALSE                 - the device is NOT the owner of the memory.
*/
static GT_BOOL smemBobcat3IsDeviceMemoryOwner
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  address
)
{
    GT_U32  unitIndex = address >> (1 + SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS);

    return bobcat3UnitTypeArr[unitIndex] == UNIT_TYPE_NOT_VALID ?
            GT_FALSE : GT_TRUE;
}

/**
* @internal smemBobcat3Init function
* @endinternal
*
* @brief   Init memory module for a Bobcat3 device.
*/
void smemBobcat3Init
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_BOOL isBobcat3 = GT_FALSE;
    GT_U32 pipeOffset;
    GT_U32 unitBaseAddress;

    /* state the supported features */
    SMEM_CHT_IS_SIP5_16_GET(devObjPtr) = 1;
    SMEM_CHT_IS_SIP5_20_GET(devObjPtr) = 1;

    if(devObjPtr->devMemUnitNameAndIndexPtr == NULL)
    {
        buildDevUnitAddr(devObjPtr);

        isBobcat3 = GT_TRUE;
    }

    if(devObjPtr->registersDefaultsPtr == NULL)
    {
        /*devObjPtr->registersDefaultsPtr = &linkListElementsBobcat3_RegistersDefaults;*/
    }

    if(devObjPtr->registersDefaultsPtr_unitsDuplications == NULL)
    {
        devObjPtr->registersDefaultsPtr_unitsDuplications = BOBCAT3_duplicatedUnits;
        devObjPtr->unitsDuplicationsPtr = BOBCAT3_duplicatedUnits;
    }

    if (isBobcat3 == GT_TRUE)
    {
        devObjPtr->devIsOwnerMemFunPtr = smemBobcat3IsDeviceMemoryOwner;

        devObjPtr->tcamHardWiredInfo.isValid = 1;
        devObjPtr->tcamHardWiredInfo.groupsArr[0] = 0;/*TTI*/
        devObjPtr->tcamHardWiredInfo.groupsArr[1] = 1;/*IPCL0*/
        devObjPtr->tcamHardWiredInfo.groupsArr[2] = 2;/*IPCL1*/
        devObjPtr->tcamHardWiredInfo.groupsArr[3] = 3;/*IPCL2*/
        devObjPtr->tcamHardWiredInfo.groupsArr[4] = 4;/*EPCL*/

        /* state 'data path' structure */
        devObjPtr->multiDataPath.supportMultiDataPath =  1;
        devObjPtr->multiDataPath.maxDp = 3;/* 6 DP units for the device */
        /* there is TXQ,dq per 'data path' */
        devObjPtr->multiDataPath.numTxqDq           = devObjPtr->multiDataPath.maxDp;
        devObjPtr->multiDataPath.txqDqNumPortsPerDp = 96;

        devObjPtr->multiDataPath.supportRelativePortNum = 1;

        devObjPtr->supportTrafficManager_notAllowed = 1;

        devObjPtr->dmaNumOfCpuPort = 74;/* 'global' port in the egress RXDMA/TXDMA units */

        devObjPtr->numOfPipes = 2;
        devObjPtr->numOfPortsPerPipe =
            devObjPtr->multiDataPath.maxDp*PORTS_PER_DP_CNS;

        devObjPtr->txqNumPorts =
            devObjPtr->multiDataPath.txqDqNumPortsPerDp *
            devObjPtr->multiDataPath.numTxqDq *
            devObjPtr->numOfPipes;/*the TXQ of Bc3 support 576 ports */

        devObjPtr->multiDataPath.maxIa = devObjPtr->numOfPipes;/* single IA per pipe */

        {
            GT_U32  index;
            for(index = 0 ; index < devObjPtr->multiDataPath.numTxqDq ; index ++)
            {
                devObjPtr->multiDataPath.dqInfo[index].use_egressDpIndex = 1;
                /* in BC3  : DQ[0..5] to TXDMA[0..5]  (one to one) */
                devObjPtr->multiDataPath.dqInfo[index].egressDpIndex = index;
            }
        }

        {
            GT_U32  index;
            for(index = 0 ; index < devObjPtr->multiDataPath.maxDp ; index++)
            {
                /* each DP supports 12 ports + 1 CPU port (index 12) */
                devObjPtr->multiDataPath.info[index].dataPathFirstPort  = PORTS_PER_DP_CNS*index;
                devObjPtr->multiDataPath.info[index].dataPathNumOfPorts = PORTS_PER_DP_CNS;
            }
            devObjPtr->multiDataPath.info[0].cpuPortDmaNum = PORTS_PER_DP_CNS;

            devObjPtr->memUnitBaseAddrInfo.txqDq[0] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TXQ_DQ);
            devObjPtr->memUnitBaseAddrInfo.txqDq[1] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TXQ_DQ_1);
            devObjPtr->memUnitBaseAddrInfo.txqDq[2] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TXQ_DQ_2);

            /*smemBobcat3UnitPipeOffsetGet(...)*/
            unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_IN_PIPE_1_MAC(UNIT_TXQ_DQ));
            pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(devObjPtr, unitBaseAddress , NULL);
            devObjPtr->memUnitBaseAddrInfo.txqDq[3] = devObjPtr->memUnitBaseAddrInfo.txqDq[0] | pipeOffset;
            devObjPtr->memUnitBaseAddrInfo.txqDq[4] = devObjPtr->memUnitBaseAddrInfo.txqDq[1] | pipeOffset;
            devObjPtr->memUnitBaseAddrInfo.txqDq[5] = devObjPtr->memUnitBaseAddrInfo.txqDq[2] | pipeOffset;


            devObjPtr->memUnitBaseAddrInfo.lpm[0] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_LPM);
            devObjPtr->memUnitBaseAddrInfo.lpm[1] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_LPM_1);
            devObjPtr->memUnitBaseAddrInfo.lpm[2] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_LPM_2);
            devObjPtr->memUnitBaseAddrInfo.lpm[3] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_LPM_3);

        }

        devObjPtr->dma_specialPortMappingArr = BOBCAT3_DMA_specialPortMappingArr;
        devObjPtr->gop_specialPortMappingArr = BOBCAT3_GOP_specialPortMappingArr;

        devObjPtr->tcam_numBanksForHitNumGranularity = 2;
        devObjPtr->tcamNumOfFloors   = 12;
        devObjPtr->portMacSecondBase = 0;
        devObjPtr->portMacSecondBaseFirstPort = 0;

        devObjPtr->support_remotePhysicalPortsTableMode = 1;
    }

    SET_IF_ZERO_MAC(devObjPtr->lpmRam.numOfLpmRams , 20);
    SET_IF_ZERO_MAC(devObjPtr->lpmRam.numOfEntriesBetweenRams , 16*1024);

    devObjPtr->portMibCounters64Bits = 1;

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
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.tunnelstartPtr , 16);/*64K*/
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.arpPtr ,  4 * devObjPtr->flexFieldNumBitsSupport.tunnelstartPtr);
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.fid , 13);


        SET_IF_ZERO_MAC(devObjPtr->limitedResources.eVid,1<<devObjPtr->flexFieldNumBitsSupport.eVid);
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.ePort,1<<devObjPtr->flexFieldNumBitsSupport.ePort);
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.phyPort,1<<devObjPtr->flexFieldNumBitsSupport.phyPort);
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.l2Ecmp,16*1024);
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.nextHop,24*1024);

        SET_IF_ZERO_MAC(devObjPtr->limitedResources.mllPairs,32*1024);
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.l2LttMll,32*1024);
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.l3LttMll,16*1024);/*routing ECMP table*/


        SET_IF_ZERO_MAC(devObjPtr->fdbMaxNumEntries , SMEM_MAC_TABLE_SIZE_256KB);
        SET_IF_ZERO_MAC(devObjPtr->lpmRam.perRamNumEntries , 16*1024);

        SET_IF_ZERO_MAC(devObjPtr->policerSupport.iplrTableSize   , (16 * _1K));

        devObjPtr->policerSupport.eplrTableSize   = 0 ;/* not to be used !!!*/
        devObjPtr->policerSupport.iplrMemoriesSize[0] = 0;/* not to be used !!!*/
        devObjPtr->policerSupport.iplrMemoriesSize[1] = 0;/* not to be used !!!*/
        devObjPtr->policerSupport.iplrMemoriesSize[2] = 0;/* not to be used !!!*/

        devObjPtr->policerSupport.iplr0TableSize = 0;/* not to be used !!!*/
        devObjPtr->policerSupport.iplr1TableSize = 0;/* not to be used !!!*/

        devObjPtr->policerSupport.supportCountingEntryFormatSelect = 0;/* Sip5.20 not support 'Compressed mode' ! */

        SET_IF_ZERO_MAC(devObjPtr->oamNumEntries,8*_1K);

        SET_IF_ZERO_MAC(devObjPtr->cncClientSupportBitmap,SNET_SIP5_20_CNC_CLIENTS_BMP_ALL_CNS);

        devObjPtr->supportCutThroughFastToSlow = 1;

        devObjPtr->errata.plrIndirectReadCountersOnDisabledMode = 1;
        devObjPtr->errata.plrIndirectReadAndResetIpFixCounters  = 1;
    }

    /* function will be called from inside smemLion2AllocSpecMemory(...) */
    if(devObjPtr->devMemSpecificDeviceUnitAlloc == NULL)
    {
        devObjPtr->devMemSpecificDeviceUnitAlloc = smemBobcat3SpecificDeviceUnitAlloc;
    }

    /* function will be called from inside smemLion3Init(...) */
    if(devObjPtr->devMemSpecificDeviceMemInitPart2 == NULL)
    {
        devObjPtr->devMemSpecificDeviceMemInitPart2 = smemBobcat3SpecificDeviceMemInitPart2;
    }

    if(devObjPtr->devMemInterruptTreeInit == NULL)
    {
        devObjPtr->devMemInterruptTreeInit = smemBobcat3InterruptTreeInit;
    }

    smemBobkInit(devObjPtr);

    if (isBobcat3 == GT_TRUE)
    {
        /* init regDb of pipe1 DP units */
        smemBobcat3InitRegDbPipe1(devObjPtr);

    /*    smemBobcat3InitPostBobk(devObjPtr);*/

        /* Init the bobk interrupts */
    /*    smemBobcat3InitInterrupts(devObjPtr);*/

        /* prepare pipe 1 recognition */
        smemBobcat3PreparePipe1Recognition(devObjPtr);
    }
}

/**
* @internal smemBobcat3Init2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemBobcat3Init2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    static int my_dummy = 0;
    GT_U32 pipeOffset;

    smemBobkInit2(devObjPtr);

    if(my_dummy)
    {
        GT_U32  pipeId;
        GT_U32  globalPortNum;
        /* <DeviceEn>*/
        smemDfxRegFldSet(devObjPtr, SMEM_LION2_DFX_SERVER_RESET_CONTROL_REG(devObjPtr), 0, 1, 1);

        globalPortNum = 0;
        for(pipeId = 0 ; pipeId < devObjPtr->numOfPipes ; pipeId++)
        {
            GT_U32  ii;
            GT_U32  regAddress;
            GT_U32  offset;
            GT_U32  value;
            GT_U32  dpUnitInPipe;
            GT_U32  portInPipe = 0;

            for(dpUnitInPipe = 0 ; dpUnitInPipe < devObjPtr->multiDataPath.maxDp; dpUnitInPipe ++)
            {
                offset = 0;

                portInPipe = PORTS_PER_DP_CNS*dpUnitInPipe;
                regAddress = SMEM_LION3_RXDMA_SCDMA_CONFIG_1_REG(devObjPtr,portInPipe);
                /*smemBobcat3UnitPipeOffsetGet(...)*/
                pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(devObjPtr, regAddress , NULL);
                regAddress += pipeId * pipeOffset;

                /* port per register */
                for(ii = 0 ; ii < (PORTS_PER_DP_CNS+1) ; ii++ , offset+=4)
                {
                    if(ii == PORTS_PER_DP_CNS)
                    {
                        value = SNET_CHT_CPU_PORT_CNS;
                    }
                    else
                    {
                        value = globalPortNum;

                        globalPortNum ++;
                    }

                    /*perform RXDMA mapping from local port to 'virual' port on the field of:
                      localDevSrcPort */
                    smemRegFldSet(devObjPtr,regAddress + offset,
                        0, 9, value);
                }
            }

        }

        {
            GT_U32  ii;
            GT_U32  regAddress = SMEM_BOBCAT2_BMA_PORT_MAPPING_TBL_MEM(devObjPtr,0);
            GT_U32  value;

            /* /Cider/EBU/Bobcat3/Bobcat3 {Current}/Switching Core/BMA_IP/Tables/<BMA_IP> BMA_IP Features/Port Mapping/Port Mapping entry */
            /* set mapping 1:1 */

            /* port per register */
            for(ii = 0 ; ii < 512 ; ii++ , regAddress+=4)
            {
                value = ii & 0x7f;/* 7 bits */
                smemRegSet(devObjPtr,regAddress,value);
            }
        }

        for(pipeId = 0 ; pipeId < devObjPtr->numOfPipes ; pipeId++)
        {
            GT_U32  dpUnitInPipe;

            for(dpUnitInPipe = 0 ; dpUnitInPipe < devObjPtr->multiDataPath.maxDp; dpUnitInPipe ++)
            {
                GT_U32  ii;
                GT_U32  regAddress = SMEM_LION3_TXQ_DQ_TXQ_PORT_TO_TX_DMA_MAP_REG(devObjPtr,0,dpUnitInPipe);
                GT_U32  value;
                /*smemBobcat3UnitPipeOffsetGet(...)*/
                GT_U32  pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(devObjPtr, regAddress , NULL);

                regAddress += pipeId * pipeOffset;

                /* /Cider/EBU-IP/TXQ_IP/SIP6.0/TXQ_IP {Current}/TXQ_DQ/Units/TXQ_IP_dq/Global/Global DQ Config/Port <%n> To DMA Map Table */
                /* set mapping 1:1 */

                /* port per register */
                for(ii = 0 ; ii < 74 ; ii++ , regAddress+=4)
                {
                    value = ii;
                    smemRegSet(devObjPtr,regAddress,value);
                }
            }
        }

        {
            GT_U32  ii;
            GT_U32  regAddress = SMEM_CHT_MAC_CONTROL0_REG(devObjPtr, 0);
            /*smemBobcat3UnitPipeOffsetGet(...)*/
            GT_U32  pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(devObjPtr, regAddress , NULL);

            for(pipeId = 0 ; pipeId < devObjPtr->numOfPipes ; pipeId++)
            {
                for(ii = 0 ; ii < 36 ; ii++)
                {
                    regAddress = SMEM_CHT_MAC_CONTROL0_REG(devObjPtr, ii);
                    smemRegFldSet(devObjPtr, regAddress + pipeId * pipeOffset, 0, 1, 1);

                    regAddress = SMEM_CHT_MAC_CONTROL_REG(devObjPtr, ii);
                    smemRegFldSet(devObjPtr, regAddress + pipeId * pipeOffset, 0, 12, 0x2f9);
                }
            }
        }
#if     0
        {
            GT_U32  initValueArr[MAX_INIT_WORDS_CNS];
            GT_U32  numWords;
            GT_U32  numEntries;
            GT_U32  startIndex;

            startIndex = 0xFFF / 2;
            numEntries = 1;
            numWords = 0;
            initValueArr[numWords++] = 0xFFFFFFFF;
            initValueArr[numWords++] = 0xFFFFFFFF;
            initValueArr[numWords++] = 0xFFFFFFFF;
            initValueArr[numWords++] = 0xFFFFFFFF;
            initValueArr[numWords++] = 0xFFFFFFFF;
            initValueArr[numWords++] = 0xFFFFFFFF;
            initValueArr[numWords++] = 0xFFFFFFFF;
            initValueArr[numWords++] = 0xFFFFFFFF;
            /* set vidx entry for legacy mode 0xfff */
            SMEM_TABLE_ENTRIES_INIT_MAC(devObjPtr,mcast,startIndex,initValueArr,numWords,numEntries);
        }
        /*startSimulationLog();*/
#endif
    }
}

/**
* @internal smemSip5_20_GlobalTxQPortForDqRegisterGet function
* @endinternal
*
* @brief   Get Global TxQ port from register address and local TxQ
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for TxQ DQ register.
* @param[in] localPortNum             - Local TxQ port number
*/
GT_U32 smemSip5_20_GlobalTxQPortForDqRegisterGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   address,
    IN GT_U32   localPortNum
)
{
    /* device has multiple DQ units */
    GT_U32 unitIdx, firstDqIdx, regAddr, dqIdx;
    GT_U32 addressInPipe;
    GT_U32 pipeId = 0;
    GT_U32 pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(devObjPtr, address , &pipeId);

    if((pipeId == 0) && (address >= pipeOffset))
    {
        /* this is DQ#3 of Aldrin 2 */
        dqIdx = 3;
    }
    else
    {
        /* calculate relative address in pipe */
        addressInPipe = (address >= pipeOffset) ? (address - pipeOffset) : address;

        /* get local Unit ID in pipe  */
        unitIdx = UNIT_INDEX_FROM_ADDR_GET_MAC(devObjPtr, addressInPipe);

        /* get address and Unit ID of TxQ port 0 of first DQ unit */
        regAddr = SMEM_LION_DEQUEUE_ENABLE_REG(devObjPtr, 0, 0);
        firstDqIdx = UNIT_INDEX_FROM_ADDR_GET_MAC(devObjPtr, regAddr);

        /* calculate global DQ index from PIPE and local DQ unit.
           DQ units are sequential. The UNIT_INDEX_FROM_ADDR_GET_MAC returns SIP_4 unit indexes.
           Need to divide to 2 to get SIP_5 values. */
        dqIdx = (pipeId * devObjPtr->multiDataPath.numTxqDq) + (unitIdx - firstDqIdx) / 2;
    }

    return localPortNum + dqIdx * (devObjPtr->multiDataPath.txqDqNumPortsPerDp);
}
