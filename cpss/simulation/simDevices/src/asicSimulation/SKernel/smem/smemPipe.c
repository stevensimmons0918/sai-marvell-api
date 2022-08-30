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
* @file smemPipe.c
*
* @brief Pipe memory mapping implementation
*
* @version   1
********************************************************************************
*/

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/smem/smemPipe.h>
#include <asicSimulation/SKernel/smem/smemLion.h>
#include <asicSimulation/SKernel/smem/smemBobcat2.h>
#include <asicSimulation/SKernel/smem/smemBobcat3.h>
#include <asicSimulation/SKernel/cheetahCommon/sregLion2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregBobcat2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/cheetahCommon/sregPipe.h>

static void onEmulator_smemPipeInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);


#define PIPE_NUM_GOP_PORTS_GIG    16
#define PIPE_NUM_GOP_PORTS_XLG   16
#define PIPE_NUM_SERDESES_CNS    16
/*#define PIPE_NUM_GOP_PORTS_CG_100G   1*/

#define PIPE_NUM_RXDMA_CNS      17
#define PIPE_NUM_TXDMA_CNS      17

#define GOP_MASK_16_PORTS_CNS 0xFFFF0FFF
#define SINGLE_100G_PORT_IS_12_CNS  12
#define SINGLE_100G_OFFSET_CNS  (SINGLE_100G_PORT_IS_12_CNS * 0x1000)/*0xc000*/
#define PIPE_MIB_OFFSET_CNS                  (0x400)
#define SMEM_PIPE_XG_MIB_COUNT_MSK_CNS     0xFF000000 /* all the existing memory 'MIB counters' , so keep only unit ID */


#define PIPE_NUM_UNITS_E    40

#define UNIT_INFO_MAC(baseAddr,unitName) \
     {baseAddr , STR(unitName)}

/* use 2 times the 'PIPE_NUM_UNITS_E' to support that each unit is '8 MSbits' and not '9 MSbits'
*/
static SMEM_UNIT_NAME_AND_INDEX_STC pipeUnitNameAndIndexArr[(2*PIPE_NUM_UNITS_E)+1]=
{
    /* filled in runtime from PIPEunits[] */
    /* must be last */
    {NULL ,                                SMAIN_NOT_VALID_CNS                     }
};


/* the addresses of the units that the Pipe uses */
static SMEM_UNIT_BASE_AND_SIZE_STC   pipeUsedUnitsAddressesArray[(2*PIPE_NUM_UNITS_E)+1]=
{
    {0,0}    /* filled in runtime from PIPEunits[] */
};

typedef struct{
    GT_U32      base_addr;/* base address of unit*/
    GT_CHAR*    nameStr  ;/* name of unit */
}UNIT_INFO_STC;

/* the units of Pipe */
static UNIT_INFO_STC pipeUnits[] =
{
     UNIT_INFO_MAC(0x00000000,UNIT_MG                )
    ,UNIT_INFO_MAC(0x01000000,UNIT_RX_DMA            )
    ,UNIT_INFO_MAC(0x02000000,UNIT_TX_DMA            )
    ,UNIT_INFO_MAC(0x04000000,UNIT_TX_FIFO           )
    ,UNIT_INFO_MAC(0x06000000,UNIT_SBC               )
    ,UNIT_INFO_MAC(0x07000000,UNIT_CNC               )
    ,UNIT_INFO_MAC(0x09000000,UNIT_GOP_LED_0         )
    ,UNIT_INFO_MAC(0x0a000000,UNIT_TAI               )
    ,UNIT_INFO_MAC(0x0b000000,UNIT_GOP_SMI_0         )
    ,UNIT_INFO_MAC(0x0e000000,UNIT_PCP               )
    ,UNIT_INFO_MAC(0x0f000000,UNIT_PHA               )

    ,UNIT_INFO_MAC(0x10000000,UNIT_GOP               )
    ,UNIT_INFO_MAC(0x12000000,UNIT_XG_PORT_MIB       )
    ,UNIT_INFO_MAC(0x13000000,UNIT_SERDES            )

    ,UNIT_INFO_MAC(0x15000000,UNIT_MPPM              )
    ,UNIT_INFO_MAC(0x17000000,UNIT_BM                )
    ,UNIT_INFO_MAC(0x18000000,UNIT_BMA               )

    ,UNIT_INFO_MAC(0x1a000000,UNIT_CPFC              )
    ,UNIT_INFO_MAC(0x1b000000,UNIT_MCFC              )

    ,UNIT_INFO_MAC(0x1e000000,UNIT_TXQ_QUEUE         )
    ,UNIT_INFO_MAC(0x1f000000,UNIT_TXQ_LL            )
    ,UNIT_INFO_MAC(0x20000000,UNIT_TXQ_QCN           )
    ,UNIT_INFO_MAC(0x21000000,UNIT_TXQ_DQ            )
    ,UNIT_INFO_MAC(0x22000000,UNIT_TXQ_DQ_1          )
    ,UNIT_INFO_MAC(0x23000000,UNIT_TXQ_BMX           )

    /* must be last */
    ,{SMAIN_NOT_VALID_CNS,NULL}
};

SMEM_ACTIVE_MEM_ENTRY_STC smemPipeCncActiveTable[] =
{
    {0x00000030, SMEM_FULL_MASK_CNS, NULL, 0, smemPipeActiveWriteFastDumpReg, 0},
    {0x00010000, 0xFFFF0000, smemCht3ActiveCncBlockRead, 0, NULL, 0},
    {0x00001480, 0xFFFFFFC0, smemCht3ActiveCncWrapAroundStatusRead, 0, NULL, 0},

    /* must be last anyway */
    {END_OF_TABLE, SMEM_FULL_MASK_CNS, NULL, 0, NULL, 0}
};

/* NOTE: all units that are duplicated from pipe 0 to pipe 1 are added into this array in runtime !!!
    it is built from PIPEunits[].orig_nameStr*/
static SMEM_UNIT_DUPLICATION_INFO_STC pipe_duplicatedUnits[] =
{
    {STR(UNIT_TXQ_DQ)  ,1}, /* 1 duplication of this unit */
        {STR(UNIT_TXQ_DQ_1)},

    {NULL,0} /* must be last */
};

/* Active memory table -- of the device -- empty , because using DB 'per unit' ...
   the 'per unit' is better because :
   1. lookup of address in faster in time
   2. defined with relative address (and not absolute) ,
       which allow to share it with other devices !!! */
static SMEM_ACTIVE_MEM_ENTRY_STC dummy_ActiveTable[] =
{
    /* must be last anyway */
    SMEM_ACTIVE_MEM_ENTRY_LAST_LINE_CNS
};

static void buildDevUnitAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /* build
        pipeUsedUnitsAddressesArray - the addresses of the units that the bobk uses
        pipeUnitNameAndIndexArr - name of unit and index in pipeUsedUnitsAddressesArray */
    GT_U32  ii,jj;
    GT_U32  index;
    GT_U32  size;
    UNIT_INFO_STC   *unitInfoPtr = &pipeUnits[0];

    index = 0;
    for(ii = 0 ; unitInfoPtr->base_addr != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
    {
        /* the size is in '1<<24' but the arrays are units of '1<<23' so we do
            lopp on : *2 */
        size = 2;
        for(jj = 0 ; jj < size ; jj++ , index++)
        {
            if(index >= (sizeof(pipeUsedUnitsAddressesArray) / sizeof(pipeUsedUnitsAddressesArray[0])))
            {
                skernelFatalError("buildDevUnitAddr : over flow of units (1) \n");
            }

            if(index >= (sizeof(pipeUnitNameAndIndexArr) / sizeof(pipeUnitNameAndIndexArr[0])))
            {
                skernelFatalError("buildDevUnitAddr : over flow of units (2) \n");
            }

            pipeUsedUnitsAddressesArray[index].unitBaseAddr = unitInfoPtr->base_addr ;
            pipeUsedUnitsAddressesArray[index].unitSizeInBytes = 0;
            pipeUnitNameAndIndexArr[index].unitNameIndex = index;
            pipeUnitNameAndIndexArr[index].unitNameStr = unitInfoPtr->nameStr;
        }
    }

    if(index >= (sizeof(pipeUnitNameAndIndexArr) / sizeof(pipeUnitNameAndIndexArr[0])))
    {
        skernelFatalError("buildDevUnitAddr : over flow of units (3) \n");
    }
    /* indication of no more */
    pipeUnitNameAndIndexArr[index].unitNameIndex = SMAIN_NOT_VALID_CNS;
    pipeUnitNameAndIndexArr[index].unitNameStr = NULL;

    devObjPtr->devMemUnitNameAndIndexPtr = pipeUnitNameAndIndexArr;
    devObjPtr->genericUsedUnitsAddressesArray = pipeUsedUnitsAddressesArray;
    devObjPtr->genericNumUnitsAddresses = index+1;
    devObjPtr->devMemUnitPipeOffsetGet = NULL;
}


/**
* @internal smemPipeUnitPcp function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitPcp
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /*PCP Interrupt Cause Register*/
    {0x00005000, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x00005004, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x000000EC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x0000087C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x00000B80)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D30)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001388)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800, 0x00001810)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002B24)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x0000386C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004510)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004614, 0x00004614)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005010, 0x00005010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008000, 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(17,4),SMEM_BIND_TABLE_MAC(pipe_PCP_dstPortMapTable)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00018000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(17,4),SMEM_BIND_TABLE_MAC(pipe_PCP_portFilterTable)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000054,         0x00001001,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000058,         0x00010dbb,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000005c,         0x0001108f,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000060,         0x00011091,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00002b24,         0x00000001,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00004614,         0x0001ffff,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00005010,         0x0000ffff,      1,    0x0}

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};

        {
            SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
            {
                {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D40, 0x00000E3C)}}
               ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001814, 0x00001814)}}
               ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004514, 0x00004610)}}
               ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004618, 0x0000471C)}}
               ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000050F0, 0x000050F0)}}
               ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000, 0x0000600C)}, FORMULA_SINGLE_PARAMETER(7, 0x10)}
               ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000060A0, 0x000060B8)}}
            };

            static SMEM_REGISTER_DEFAULT_VALUE_STC rev1_registersDefaultValueArr[] =
            {
                 {DUMMY_NAME_PTR_CNS,            0x00004714,         0x0001ffff,      1,    0x0}
                ,{DUMMY_NAME_PTR_CNS,            0x000050f0,         0x00000001,      1,    0x0}
                ,{DUMMY_NAME_PTR_CNS,            0x000060A0,         0x00000002,      7,    0x4}

                ,{NULL,            0,         0x00000000,      0,    0x0      }
            };

            static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC rev1List = {rev1_registersDefaultValueArr, &list};

            if(SKERNEL_IS_PIPE_REVISON_A1_DEV(devObjPtr))
            {
                /* Pipe revision A1 */
                GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
                SMEM_UNIT_CHUNKS_STC tmpUnitChunk;

                smemInitMemChunkExt(devObjPtr, chunksMem, numOfChunks, &tmpUnitChunk);

                /*add the tmp unit chunks to the main unit */
                smemInitMemCombineUnitChunks(devObjPtr, unitPtr, &tmpUnitChunk);
                unitPtr->unitDefaultRegistersPtr = &rev1List;

            }
            else
            {
                unitPtr->unitDefaultRegistersPtr = &list;

            }
        }
    }
}

/*ppn%n 0x0F000000 + 0x80000 * m + 0x4000 * p: where p (0-7) represents PPN, where m (0-3) represents PPG*/
#define PPN_REPLICATIONS 4,PPG_OFFSET , 8,PPN_OFFSET

#define PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS FORMULA_TWO_PARAMETERS(4,PPG_OFFSET , 8,PPN_OFFSET)

/*ppg%g 0x0F000000 + 0x80000 * g : where g (0-3) represents PPG*/
#define PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER FORMULA_SINGLE_PARAMETER(4,PPG_OFFSET)

#define PPG_REPLICATION_ADDR(ppg)   \
    (PPG_OFFSET * (ppg))

#define PPN_REPLICATION_ADDR(ppg,ppn)   \
    (PPG_REPLICATION_ADDR(ppg) + (PPN_OFFSET * (ppn)))

#define PPN_REPLICATIONS_ACTIVE_MEM(ppg,ppn) \
    /*PPG Interrupt Cause Register*/         \
    {0x00003040 + PPN_REPLICATION_ADDR(ppg,ppn), SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x00003044 + PPN_REPLICATION_ADDR(ppg,ppn), SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0}

#define PPG_REPLICATIONS_ACTIVE_MEM(ppg) \
    /*PPG Interrupt Cause Register*/     \
    {0x0007FF00 + PPG_REPLICATION_ADDR(ppg), SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x0007FF04 + PPG_REPLICATION_ADDR(ppg), SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},              \
    /*PPG internal error cause Register*/                                                                                              \
    {0x0007FF10 + PPG_REPLICATION_ADDR(ppg), SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x0007FF14 + PPG_REPLICATION_ADDR(ppg), SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},              \
                                        \
    PPN_REPLICATIONS_ACTIVE_MEM(ppg,0), \
    PPN_REPLICATIONS_ACTIVE_MEM(ppg,1), \
    PPN_REPLICATIONS_ACTIVE_MEM(ppg,2), \
    PPN_REPLICATIONS_ACTIVE_MEM(ppg,3), \
    PPN_REPLICATIONS_ACTIVE_MEM(ppg,4), \
    PPN_REPLICATIONS_ACTIVE_MEM(ppg,5), \
    PPN_REPLICATIONS_ACTIVE_MEM(ppg,6), \
    PPN_REPLICATIONS_ACTIVE_MEM(ppg,7)


/**
* @internal smemPipeUnitPha function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitPha
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /*PPA Interrupt Cause Register*/
    {0x007EFF10 , SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x007EFF14 , SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},
    /*PPA internal error cause Register*/
    {0x007EFF20 , SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x007EFF24 , SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},

    PPG_REPLICATIONS_ACTIVE_MEM(0),/*include all it's PPN*/
    PPG_REPLICATIONS_ACTIVE_MEM(1),/*include all it's PPN*/
    PPG_REPLICATIONS_ACTIVE_MEM(2),/*include all it's PPN*/
    PPG_REPLICATIONS_ACTIVE_MEM(3),/*include all it's PPN*/

    /*PHA Interrupt Cause Register*/
    {0x007FFF60 , SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x007FFF64 , SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},
    /*PHA internal error cause Register*/
    {0x007FFF48 , SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x007FFF4c , SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},
    /*PHA SER error cause Register*/
    {0x007FFF40 , SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x007FFF44 , SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /*PHA*/
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x007F0000, 17408) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(144,32),SMEM_BIND_TABLE_MAC(pipe_PHA_haTable)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x007F8000, 68)    , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(pipe_PHA_srcPortData)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x007F8400, 136)   , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64,8),SMEM_BIND_TABLE_MAC(pipe_PHA_targetPortData)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x007FFF00, 0x007FFF04)}  }
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x007FFF10, 0x007FFF18)}  }
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x007FFF30, 0x007FFF34)}  }
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x007FFF40, 0x007FFF58)}  }
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x007FFF60, 0x007FFF64)}  }
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x007FFF70, 0x007FFF74)}  }
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x007FFF80, 0x007FFF80)}  }
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x007FFF90, 0x007FFF98)}  }
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x007FFFA0, 0x007FFFAC)}  }

            /* per PPN */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 2048)       }, PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 32)         }, PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002100, 28)         }, PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002200, 148)        }, PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00003000, 0x00003008) }, PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00003010, 0x00003010) }, PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00003020, 0x00003024) }, PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00003040, 0x00003044) }, PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            /* per PPG (cluster of 8 PPN) */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x0007FF00, 0x0007FF04) }, PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x0007FF10, 0x0007FF14) }, PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 16384)      }, PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER}
            /* PPA (cluster of 4 PPG) */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x007EFF00, 0x007EFF00)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x007EFF10, 0x007EFF14)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x007EFF20, 0x007EFF24)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x007EFF30, 0x007EFF30)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x007EFF40, 0x007EFF4C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x007C0000, 8192)}      }
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /*PHA*/
             {DUMMY_NAME_PTR_CNS,            0x007fff00,         0x00000002,      1,    0x0          }
            ,{DUMMY_NAME_PTR_CNS,            0x007fff04,         0x00000001,      2,    0x2c         }
            ,{DUMMY_NAME_PTR_CNS,            0x007fff34,         0x00000001,      1,    0x0          }
            ,{DUMMY_NAME_PTR_CNS,            0x007fff80,         0xffff0000,      1,    0x0          }
            /* PPA */
            ,{DUMMY_NAME_PTR_CNS,            0x007eff00,         0x00000001,      1,    0x0          }
            ,{DUMMY_NAME_PTR_CNS,            0x007eff40,         0xffff0000,      1,    0x0          }

            /* per PPN */
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0x00000001,      PPN_REPLICATIONS   }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}


/**
* @internal smemPipeUnitCnc function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the CNC unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitCnc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr,
    IN GT_U32   cncUnitIndex
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000024, 0x00000024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000030, 0x00000030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000104)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000180, 0x00000184)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000018C, 0x00000190)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001A4, 0x000001A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001B8, 0x000001B8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001CC, 0x000001CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001E0, 0x000001E4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000300)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000310, 0x00000310)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001098, 0x000010D4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001298, 0x000012D4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001498, 0x00001594)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001C98, 0x00001C9C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001CA8, 0x00001CAC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001CB8, 0x00001CBC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001CC8, 0x00001CCC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001CD8, 0x00001CDC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001CE8, 0x00001CEC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001CF8, 0x00001CFC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D08, 0x00001D0C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D18, 0x00001D1C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D28, 0x00001D2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D38, 0x00001D3C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D48, 0x00001D4C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D58, 0x00001D5C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D68, 0x00001D6C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D78, 0x00001D7C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D88, 0x00001D8C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 8192 * 2), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64,8),SMEM_BIND_TABLE_MAC(cncMemory)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        GT_U32  ii;

        if (cncUnitIndex != 0)
        {
            /* make sure that table are bound only to CNC0 (since use of 'multiple instance') */
            for(ii = 0 ; ii < numOfChunks ; ii ++)
            {
                chunksMem[ii].tableOffsetValid = 0;
                chunksMem[ii].tableOffsetInBytes = 0;
            }
        }

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001398, 0x0000139c)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        GT_U32  ii;

        if (cncUnitIndex != 0)
        {
            /* make sure that table are bound only to CNC0 (since use of 'multiple instance') */
            for(ii = 0 ; ii < numOfChunks ; ii ++)
            {
                chunksMem[ii].tableOffsetValid = 0;
                chunksMem[ii].tableOffsetInBytes = 0;
            }
        }

        smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
            ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
    }

    /* Add fast dump register write function */
    unitPtr->unitActiveMemPtr = smemPipeCncActiveTable;
}

/**
* @internal smemPipeUnitMg function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitMg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    ACTIVE_MEM_MG_COMMON_MAC ,

    /* XSMI Management Register */
    {0x00030000, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteXSmii, 0},
    /* XSMI1 Management Register */
    {0x00032000, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteXSmii, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00030000, 8192)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00032000, 8192)}
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
}

/**
* @internal smemPipeUnitMppm function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to common device memory object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitMppm
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /*ECC Interrupt Cause Register*/
    {0x00002000, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x00002004, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},
    /* General Interrupts Cause Register */
    {0x000021c0, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x000021c4, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

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
* @internal smemPipeUnitRxDma function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitRxDma
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000804, 0x00000844)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000950, 0x00000990)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B40)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000E48)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000120C, 0x0000124C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001528, 0x0000152C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001548, 0x0000154C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001704, 0x0000170C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001720, 0x00001744)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001790, 0x00001790)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800, 0x00001800)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001820, 0x00001820)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001830, 0x00001834)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001840, 0x00001840)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001850, 0x00001850)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001864, 0x00001868)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001870, 0x00001870)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001900, 0x00001900)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001930, 0x00001930)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001940, 0x00001940)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001B00, 0x00001B40)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001E00, 0x00001E40)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000020F0, 0x000020F0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x00002140)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002400, 0x00002440)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002A00, 0x00002B58)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002B60, 0x00002B60)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003500, 0x00003540)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003A00, 0x00003A40)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004080, 0x00004084)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004100, 0x00004104)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004200, 0x00004200)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005000)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,         0x00000000,         0xbfffbfff,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00000004,         0x0000bfff,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00000040,         0x0000bfff,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00000084,         0x81008100,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000008c,         0x0001ffe2,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x000000a0,         0x00000003,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00000350,         0x00000800,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00000374,         0x00008600,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00000398,         0x00008100,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000039c,         0x00008a88,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x000003a8,         0x00008847,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x000003ac,         0x00008848,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x000005d0,         0xffff0000,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00000804,         0x0ffff000,     PIPE_NUM_RXDMA_CNS,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000950,         0x00000000,      1,    0x0      }
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
            ,{DUMMY_NAME_PTR_CNS,         0x00000b00,         0x00000010,     PIPE_NUM_RXDMA_CNS,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,         0x00001704,         0x00080001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00001800,         0x007ffdff,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00001840,         0x00000040,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00001850,         0xaaaaaaaa,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00001864,         0x0000bfff,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00001868,         0x00000005,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00001930,         0x0000001f,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00001940,         0x0000000a,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00001b00,         0x00000015,     PIPE_NUM_RXDMA_CNS,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a00,         0x00004049,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a04,         0x00000004,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a08,         0x83828180,     2,    0x124 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a0c,         0x87868584,     2,    0x124 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a10,         0x8b8a8988,     2,    0x124 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a14,         0x8f8e8d8c,     2,    0x124 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a18,         0x93929190,     2,    0x124 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a1c,         0x97969594,     2,    0x124 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a20,         0x9b9a9998,     2,    0x124 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a24,         0x9f9e9d9c,     2,    0x124 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a28,         0xa3a2a1a0,     2,    0x124 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a2c,         0xa7a6a5a4,     2,    0x124 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a30,         0xabaaa9a8,     2,    0x124 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a34,         0xafaeadac,     2,    0x124 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a38,         0xb3b2b1b0,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a3c,         0xb7b6b5b4,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a40,         0xbbbab9b8,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a44,         0xbfbebdbc,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a48,         0xc3c2c1c0,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a4c,         0xc7c6c5c4,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a50,         0x828180c8,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a54,         0x86858483,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a58,         0x8a898887,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a5c,         0x8e8d8c8b,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a60,         0x9291908f,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a64,         0x96959493,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a68,         0x9a999897,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a6c,         0x9e9d9c9b,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a70,         0xa2a1a09f,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a74,         0xa6a5a4a3,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a78,         0xaaa9a8a7,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a7c,         0xaeadacab,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a80,         0xb2b1b0af,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a84,         0xb6b5b4b3,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a88,         0xbab9b8b7,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a8c,         0xbebdbcbb,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a90,         0xc2c1c0bf,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a94,         0xc6c5c4c3,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a98,         0x8180c8c7,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002a9c,         0x85848382,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002aa0,         0x89888786,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002aa4,         0x8d8c8b8a,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002aa8,         0x91908f8e,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002aac,         0x95949392,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ab0,         0x99989796,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ab4,         0x9d9c9b9a,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ab8,         0xa1a09f9e,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002abc,         0xa5a4a3a2,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ac0,         0xa9a8a7a6,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ac4,         0xadacabaa,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ac8,         0xb1b0afae,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002acc,         0xb5b4b3b2,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ad0,         0xb9b8b7b6,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ad4,         0xbdbcbbba,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ad8,         0xc1c0bfbe,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002adc,         0xc5c4c3c2,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ae0,         0x80c8c7c6,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ae4,         0x84838281,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002ae8,         0x88878685,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002aec,         0x8c8b8a89,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002af0,         0x908f8e8d,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002af4,         0x94939291,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002af8,         0x98979695,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002afc,         0x9c9b9a99,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b00,         0xa09f9e9d,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b04,         0xa4a3a2a1,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b08,         0xa8a7a6a5,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b0c,         0xacabaaa9,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b10,         0xb0afaead,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b14,         0xb4b3b2b1,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b18,         0xb8b7b6b5,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b1c,         0xbcbbbab9,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b20,         0xc0bfbebd,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b24,         0xc4c3c2c1,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00002b28,         0xc8c7c6c5,     1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,         0x00004200,         0x00000001,     1,    0x0  }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemPipeUnitTxDma function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitTxDma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x0000003C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000220, 0x00000228)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000504)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001020, 0x00001020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001060, 0x00001060)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002018, 0x0000201C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002038, 0x00002044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002050, 0x00002054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x00002140)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002300, 0x00002340)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x0000321C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003A00, 0x00003A40)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004158)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004160, 0x00004160)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000501C, 0x00005078)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005080, 0x0000508C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005100, 0x00005100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005120, 0x00005120)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005140, 0x00005140)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005160, 0x00005160)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005180, 0x00005180)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000051A0, 0x000051A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000051C0, 0x000051C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005200, 0x00005240)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005500, 0x00005540)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005700, 0x00005740)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005900, 0x00005940)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006100, 0x00006104)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006300, 0x00006304)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006500, 0x00006500)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006520, 0x00006520)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006540, 0x00006540)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006560, 0x00006560)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006580, 0x00006580)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000065A0, 0x000065A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000065C0, 0x000065C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000, 0x00007004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007100, 0x00007140)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007300, 0x00007300)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007500, 0x00007500)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007700, 0x00007700)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007800, 0x00007840)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007930, 0x0000793C)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x00000007,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x000bfd02,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000008,         0x002fffff,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000010,         0x0000ffff,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000014,         0x00000041,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000220,         0x22222222,      3,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00001000,         0x00001fff,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00001004,         0x0000f03f,      2,    0x8 }
            ,{DUMMY_NAME_PTR_CNS,            0x00001008,         0x00000007,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00001010,         0x0000f03f,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00001014,         0x00000005,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00001020,         0x00777777,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00001060,         0x7777777f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0x00000001,     PIPE_NUM_TXDMA_CNS,    0x20}
            ,{DUMMY_NAME_PTR_CNS,            0x00003004,         0x000000b2,     PIPE_NUM_TXDMA_CNS,    0x20}
            ,{DUMMY_NAME_PTR_CNS,            0x00003008,         0x659fe064,     PIPE_NUM_TXDMA_CNS,    0x20}
            ,{DUMMY_NAME_PTR_CNS,            0x0000300c,         0x00000c03,     PIPE_NUM_TXDMA_CNS,    0x20}
            ,{DUMMY_NAME_PTR_CNS,            0x00003010,         0x00020100,     PIPE_NUM_TXDMA_CNS,    0x20}
            ,{DUMMY_NAME_PTR_CNS,            0x00003014,         0x0000000f,     PIPE_NUM_TXDMA_CNS,    0x20}
            ,{DUMMY_NAME_PTR_CNS,            0x00004000,         0x00004011,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00004004,         0x00000004,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00004008,         0x83828180,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000400c,         0x87868584,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x00004010,         0x8b8a8988,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x00004014,         0x8f8e8d8c,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x00004018,         0x82818090,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000401c,         0x86858483,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x00004020,         0x8a898887,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x00004024,         0x8e8d8c8b,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x00004028,         0x8180908f,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000402c,         0x85848382,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x00004030,         0x89888786,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x00004034,         0x8d8c8b8a,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x00004038,         0x80908f8e,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000403c,         0x84838281,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x00004040,         0x88878685,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x00004044,         0x8c8b8a89,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x00004048,         0x908f8e8d,      5,    0x44 }
            ,{DUMMY_NAME_PTR_CNS,            0x00007000,         0x00b7aaa0,      2,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00007100,         0x80000a00,     PIPE_NUM_TXDMA_CNS,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00007300,         0x00000a00,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00007500,         0x8001fc00,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00007700,         0x0000000a,      1,    0x0  }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemPipeUnitTxFifo function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitTxFifo
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
* @internal smemPipeUnitTai function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitTai
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    /* TAI subunit */
    /* Time Counter Function Configuration 0 - Function Trigger */
    {0x10, SMEM_FULL_MASK_CNS, NULL, 0 , smemLion3ActiveWriteTodFuncConfReg, 0},
    /* time Capture Value 0 Frac Low */
    {0x84, SMEM_FULL_MASK_CNS, smemBobcat2ActiveReadTodTimeCaptureValueFracLow, 0 , smemChtActiveWriteToReadOnlyReg, 0},
    /* time Capture Value 1 Frac Low */
    {0xA0, SMEM_FULL_MASK_CNS, smemBobcat2ActiveReadTodTimeCaptureValueFracLow, 1 , smemChtActiveWriteToReadOnlyReg, 0},
    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

     {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000100)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}

/**
* @internal smemPipeUnitBm function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitBm
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* BM General Cause Reg1 Register */
    {0x00000300, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    /* Write BM General Mask Reg1 Register */
    {0x00000304, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000400, 0x0000040C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000420, 0x0000042C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000440, 0x0000044C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000460, 0x0000046C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000480, 0x0000048C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000004A0, 0x000004A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000004C0, 0x000004C8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000004D0, 0x000004D8)}
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
* @internal smemPipeUnitBma function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitBma
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

/**
* @internal smemPipeUnitGopLed function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Bobk gop LED unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitGopLed
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000160)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}

/**
* @internal smemPipeUnitGopSmi function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Pipe gop smi unit
*
* @param[in] devObjPtr                - pointer to common device memory object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitGopSmi
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr, unitPtr)

    /* SMI0 Management Register */
    {0x00000000, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteSmi, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr, unitPtr)
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000048)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

}


/**
* @internal smemPipeUnitGop function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the GOP unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitGop
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* Port<n> Auto-Negotiation Configuration Register */
    {0x0000000C                       , GOP_MASK_16_PORTS_CNS, NULL, 0 , smemChtActiveWriteForceLinkDown, 0},
    /* Port MAC Control Register2 */
    {0x00000008                       , GOP_MASK_16_PORTS_CNS, NULL, 0 , smemChtActiveWriteMacGigControl2, 0},

    /* Port MAC Control Register0 */
    {0x000C0000,                        GOP_MASK_16_PORTS_CNS, NULL, 0 , smemChtActiveWriteForceLinkDownXg, 0},

    /* Port MAC Control Register3 */
    {0x000C001C,                        GOP_MASK_16_PORTS_CNS, NULL, 0 , smemXcatActiveWriteMacModeSelect, 0},

    /* Port MAC Control Register4 : SMEM_XCAT_XG_MAC_CONTROL4_REG */
    {0x000C0084,                        GOP_MASK_16_PORTS_CNS, NULL, 0 , smemBobKActiveWriteMacXlgCtrl4, 0},


    /* stack gig ports - Tri-Speed Port<n> Interrupt Mask Register */
    {0x000C0024,                        GOP_MASK_16_PORTS_CNS, NULL, 0 , smemChtActiveWritePortInterruptsMaskReg, 0},

    /* XG Port<n> Interrupt Cause Register  */
    {0x000C0014,                        GOP_MASK_16_PORTS_CNS,  smemChtActiveReadIntrCauseReg, 29, smemChtActiveWriteIntrCauseReg, 0},

    /* XG Port<n> Interrupt Mask Register */
    {0x000C0018,                        GOP_MASK_16_PORTS_CNS, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},

    /* stack gig ports - Port<n> Interrupt Cause Register  */
    {0x00000020,                        GOP_MASK_16_PORTS_CNS,smemChtActiveReadIntrCauseReg, 18, smemChtActiveWriteIntrCauseReg, 0},

    /* stack gig ports - Tri-Speed Port<n> Interrupt Mask Register */
    {0x00000024,                        GOP_MASK_16_PORTS_CNS, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},

    /* CG-mac (100G mac) */
    /* <CG_IP> CG TOP/Units/CG Upper Top Units %a Pipe %t/Converters registers/Control 0 */
    {0x00340000+SINGLE_100G_OFFSET_CNS,  SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWrite100GMacControl0, 0},

    /* <CG_IP> CG TOP/Units/CG Upper Top Units %a Pipe %t/Converters registers/Resets */
    {0x00340010+SINGLE_100G_OFFSET_CNS,  SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWrite100GMacResets, 0},

    /*CG Interrupt mask*/
    /*SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_CONVERTERS.CGMAConvertersCG_Interrupt_mask*/
    {0x00340044+SINGLE_100G_OFFSET_CNS,  SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},

    /*CG Interrupt cause*/
    /*SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_CONVERTERS.CGMAConvertersCG_Interrupt_cause*/
    {0x00340040+SINGLE_100G_OFFSET_CNS,  SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},

    /*STATN_CONFIG */
    /*SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CGPORTMACStatNConfig*/
    {0x00340478+SINGLE_100G_OFFSET_CNS,  SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWrite100GMacMibCtrlReg, 0},

    /* need to cover
       from : 0x10340480 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.aFramesTransmittedOK
       to   : 0x10340624 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.aMACControlFramesReceived

       so we do several 'larger ranges' :
        1. 0x10340400 - 0x103405FC
        2. 0x10340600 - 0x1034063C
    */

    /* 1. 0x10340400 - 0x103405FC */
    {0x00340400+SINGLE_100G_OFFSET_CNS,  0xFFFFFE00, smemBobcat3ActiveRead100GMacMibCounters, 0, NULL, 0},
    /* 2. 0x10340600 - 0x1034063C */
    {0x00340600+SINGLE_100G_OFFSET_CNS,  0xFFFFFFC0, smemBobcat3ActiveRead100GMacMibCounters, 0, NULL, 0},


    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* Gig Ports*/
                /* ports 0..15 */
                 {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000000, 0x00000094)} , FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_GIG , 0x1000)}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000000A0, 0x000000A4)} , FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_GIG , 0x1000)}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000000C0, 0x000000E0)} , FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_GIG , 0x1000)}

            /* XLG */
                /* ports 0..15 */
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000c0000 , 0x000c0024 )}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000c002C , 0x000C0030 )}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000C0038 , 0x000C0060 )}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000C0068 , 0x000C0088 )}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000C0090 , 0x000C0098 )}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}

            /* MPCS */
                /* ports 0..15 */
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180008 , 0x00180018 )}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180020 , 0x00180024 )}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180030 , 0x00180030 )}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0018003C , 0x001800C8 )}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001800D0 , 0x00180120 )}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180128 , 0x0018014C )}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0018015C , 0x0018017C )}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00180200 , 256)}       , FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}

            /* XPCS IP */
                /* ports 0..15 */
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180400, 0x00180424)}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0018042C, 0x0018044C)}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}
                /* 6 lanes */
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00180450, 0x0044)}, FORMULA_TWO_PARAMETERS(6 , 0x44  , PIPE_NUM_GOP_PORTS_XLG , 0x1000)}

            /*FCA*/
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180600, 0x001806A0)}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180704, 0x00180714)}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}
            /*PTP*/
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180800, 0x0018087C)}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , 0x1000)}

            /* Mac-TG Generator - 1 for every 4 ports */
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180C00 ,0x00180CCC)}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG / 4 , 0x1000 * 4)}

            /* CG-mac (100G mac) */
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00340000+SINGLE_100G_OFFSET_CNS, 0x0034003C+SINGLE_100G_OFFSET_CNS)} }
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00340040+SINGLE_100G_OFFSET_CNS, 0x0034004C+SINGLE_100G_OFFSET_CNS)} }
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00340060+SINGLE_100G_OFFSET_CNS, 0x0034009C+SINGLE_100G_OFFSET_CNS)} }
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00340100+SINGLE_100G_OFFSET_CNS, 0x00340108+SINGLE_100G_OFFSET_CNS)} }
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00340400+SINGLE_100G_OFFSET_CNS, 1024)}       }
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00340800+SINGLE_100G_OFFSET_CNS, 1024)}       }
            /* CG-PCS-mac (100G mac) */
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300000+SINGLE_100G_OFFSET_CNS, 0x00300020+SINGLE_100G_OFFSET_CNS)} }
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300038+SINGLE_100G_OFFSET_CNS, 0x0030003C+SINGLE_100G_OFFSET_CNS)} }
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300080+SINGLE_100G_OFFSET_CNS, 0x00300084+SINGLE_100G_OFFSET_CNS)} }
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x003000A8+SINGLE_100G_OFFSET_CNS, 0x003000B4+SINGLE_100G_OFFSET_CNS)} }
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x003000C8+SINGLE_100G_OFFSET_CNS, 0x003000D4+SINGLE_100G_OFFSET_CNS)} }
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300320+SINGLE_100G_OFFSET_CNS, 0x0030036C+SINGLE_100G_OFFSET_CNS)} }
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300640+SINGLE_100G_OFFSET_CNS, 0x0030068C+SINGLE_100G_OFFSET_CNS)} }
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300800+SINGLE_100G_OFFSET_CNS, 0x0030080C+SINGLE_100G_OFFSET_CNS)} }
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300840+SINGLE_100G_OFFSET_CNS, 0x00300840+SINGLE_100G_OFFSET_CNS)} }
                ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300900+SINGLE_100G_OFFSET_CNS, 0x0030099C+SINGLE_100G_OFFSET_CNS)} }

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             /*{DUMMY_NAME_PTR_CNS,            0x000a1000,         0x0000ffff,      1,    0x0         }*/
            /* PTP */
             {DUMMY_NAME_PTR_CNS,           0x00180808,         0x00000001,      PIPE_NUM_GOP_PORTS_XLG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,           0x00180870,         0x000083aa,      PIPE_NUM_GOP_PORTS_XLG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,           0x00180874,         0x00007e5d,      PIPE_NUM_GOP_PORTS_XLG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,           0x00180878,         0x00000040,      PIPE_NUM_GOP_PORTS_XLG , 0x1000 }

        /*Giga*/
            ,{DUMMY_NAME_PTR_CNS,            0x00000000,         0x00008be5,     PIPE_NUM_GOP_PORTS_GIG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x00000003,     PIPE_NUM_GOP_PORTS_GIG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000008,         0x0000c048,     PIPE_NUM_GOP_PORTS_GIG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x0000bae8,     PIPE_NUM_GOP_PORTS_GIG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000001c,         0x00000052,     PIPE_NUM_GOP_PORTS_GIG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000002c,         0x0000000c,     PIPE_NUM_GOP_PORTS_GIG , 0x1000      ,2,    0x18     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000030,         0x0000c815,     PIPE_NUM_GOP_PORTS_GIG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000048,         0x00000300,     PIPE_NUM_GOP_PORTS_GIG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000094,         0x00000001,     PIPE_NUM_GOP_PORTS_GIG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x000000c0,         0x00001004,     PIPE_NUM_GOP_PORTS_GIG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x000000c4,         0x00000100,     PIPE_NUM_GOP_PORTS_GIG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x000000c8,         0x000001fd,     PIPE_NUM_GOP_PORTS_GIG , 0x1000 }

            ,{DUMMY_NAME_PTR_CNS,            0x00000090,         0x0000ff9a,     PIPE_NUM_GOP_PORTS_GIG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000018,         0x00004b4d,     PIPE_NUM_GOP_PORTS_GIG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000014,         0x000008c4,     PIPE_NUM_GOP_PORTS_GIG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x000000d4,         0x000000ff,     PIPE_NUM_GOP_PORTS_GIG , 0x1000 }

        /*XLG*/
                                        /* set ALL ports as XLG */
            ,{DUMMY_NAME_PTR_CNS,            0x000c0004,         0x000002f9,     PIPE_NUM_GOP_PORTS_XLG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x000c001c,         0x00006000,     PIPE_NUM_GOP_PORTS_XLG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x000c0030,         0x000007ec,     PIPE_NUM_GOP_PORTS_XLG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x000c0080,         0x00001000,     PIPE_NUM_GOP_PORTS_XLG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x000c0084,         0x00000210,     PIPE_NUM_GOP_PORTS_XLG , 0x1000 }
        /*FCA*/
            ,{DUMMY_NAME_PTR_CNS,            0x00180000+0x600,   0x00000011,      PIPE_NUM_GOP_PORTS_XLG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00180004+0x600,   0x00002003,      PIPE_NUM_GOP_PORTS_XLG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00180054+0x600,   0x00000001,      PIPE_NUM_GOP_PORTS_XLG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00180058+0x600,   0x0000c200,      PIPE_NUM_GOP_PORTS_XLG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x0018005c+0x600,   0x00000180,      PIPE_NUM_GOP_PORTS_XLG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x0018006c+0x600,   0x00008808,      PIPE_NUM_GOP_PORTS_XLG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00180070+0x600,   0x00000001,      PIPE_NUM_GOP_PORTS_XLG , 0x1000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00180104+0x600,   0x0000ff00,      PIPE_NUM_GOP_PORTS_XLG , 0x1000 }

            /* CG-mac (100G mac) */
            ,{DUMMY_NAME_PTR_CNS,            0x00340000+SINGLE_100G_OFFSET_CNS,         0x10020018,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00340004+SINGLE_100G_OFFSET_CNS,         0x0000003f,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00340008+SINGLE_100G_OFFSET_CNS,         0xff00ff00,       }
            ,{DUMMY_NAME_PTR_CNS,            0x0034000c+SINGLE_100G_OFFSET_CNS,         0x00000190,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00340014+SINGLE_100G_OFFSET_CNS,         0x0a080040,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00340018+SINGLE_100G_OFFSET_CNS,         0x00002f2d,       }
            ,{DUMMY_NAME_PTR_CNS,            0x0034001c+SINGLE_100G_OFFSET_CNS,         0x0000001c,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00340060+SINGLE_100G_OFFSET_CNS,         0x01408000,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00340064+SINGLE_100G_OFFSET_CNS,         0x00477690,       4,    0x10   }
            ,{DUMMY_NAME_PTR_CNS,            0x00340068+SINGLE_100G_OFFSET_CNS,         0x00e6c4f0,       4,    0x10   }
            ,{DUMMY_NAME_PTR_CNS,            0x0034006c+SINGLE_100G_OFFSET_CNS,         0x009b65c5,       4,    0x10   }
            ,{DUMMY_NAME_PTR_CNS,            0x00340070+SINGLE_100G_OFFSET_CNS,         0x003d79a2,       3,    0x10   }
            ,{DUMMY_NAME_PTR_CNS,            0x00340100+SINGLE_100G_OFFSET_CNS,         0x003d79a2,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00340104+SINGLE_100G_OFFSET_CNS,         0x3fff3fff,       2,    0x4   }

            ,{DUMMY_NAME_PTR_CNS,            0x00340400+SINGLE_100G_OFFSET_CNS,         0x00010106,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00340414+SINGLE_100G_OFFSET_CNS,         0x00000600,       }
            ,{DUMMY_NAME_PTR_CNS,            0x0034041c+SINGLE_100G_OFFSET_CNS,         0x00000010,       2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00340430+SINGLE_100G_OFFSET_CNS,         0x00007d00,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00340434+SINGLE_100G_OFFSET_CNS,         0x00000001,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00340444+SINGLE_100G_OFFSET_CNS,         0x0000000c,       }

            ,{DUMMY_NAME_PTR_CNS,            0x00340804+SINGLE_100G_OFFSET_CNS,         0x00008003,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00340a04+SINGLE_100G_OFFSET_CNS,         0x0000f000,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00340a0c+SINGLE_100G_OFFSET_CNS,         0x00000022,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00340a28+SINGLE_100G_OFFSET_CNS,         0x0000002a,       4,    0x4   }

            /* CG-PCS-mac (100G mac) */
            ,{DUMMY_NAME_PTR_CNS,            0x00300000+SINGLE_100G_OFFSET_CNS,         0x00002050,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300010+SINGLE_100G_OFFSET_CNS,         0x00000100,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300014+SINGLE_100G_OFFSET_CNS,         0x00000008,       }
            ,{DUMMY_NAME_PTR_CNS,            0x0030001c+SINGLE_100G_OFFSET_CNS,         0x00000005,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300020+SINGLE_100G_OFFSET_CNS,         0x00008020,       }
            ,{DUMMY_NAME_PTR_CNS,            0x003000b4+SINGLE_100G_OFFSET_CNS,         0x00008000,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300804+SINGLE_100G_OFFSET_CNS,         0x00000001,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300808+SINGLE_100G_OFFSET_CNS,         0x00003fff,       }
            ,{DUMMY_NAME_PTR_CNS,            0x0030080c+SINGLE_100G_OFFSET_CNS,         0x00009999,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300900+SINGLE_100G_OFFSET_CNS,         0x000068c1,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300904+SINGLE_100G_OFFSET_CNS,         0x00000021,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300908+SINGLE_100G_OFFSET_CNS,         0x0000719d,       }
            ,{DUMMY_NAME_PTR_CNS,            0x0030090c+SINGLE_100G_OFFSET_CNS,         0x0000008e,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300910+SINGLE_100G_OFFSET_CNS,         0x00004b59,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300914+SINGLE_100G_OFFSET_CNS,         0x000000e8,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300918+SINGLE_100G_OFFSET_CNS,         0x0000954d,       }
            ,{DUMMY_NAME_PTR_CNS,            0x0030091c+SINGLE_100G_OFFSET_CNS,         0x0000007b,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300920+SINGLE_100G_OFFSET_CNS,         0x000007f5,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300924+SINGLE_100G_OFFSET_CNS,         0x00000009,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300928+SINGLE_100G_OFFSET_CNS,         0x000014dd,       }
            ,{DUMMY_NAME_PTR_CNS,            0x0030092c+SINGLE_100G_OFFSET_CNS,         0x000000c2,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300930+SINGLE_100G_OFFSET_CNS,         0x00004a9a,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300934+SINGLE_100G_OFFSET_CNS,         0x00000026,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300938+SINGLE_100G_OFFSET_CNS,         0x0000457b,       }
            ,{DUMMY_NAME_PTR_CNS,            0x0030093c+SINGLE_100G_OFFSET_CNS,         0x00000066,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300940+SINGLE_100G_OFFSET_CNS,         0x000024a0,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300944+SINGLE_100G_OFFSET_CNS,         0x00000076,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300948+SINGLE_100G_OFFSET_CNS,         0x0000c968,       }
            ,{DUMMY_NAME_PTR_CNS,            0x0030094c+SINGLE_100G_OFFSET_CNS,         0x000000fb,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300950+SINGLE_100G_OFFSET_CNS,         0x00006cfd,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300954+SINGLE_100G_OFFSET_CNS,         0x00000099,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300958+SINGLE_100G_OFFSET_CNS,         0x000091b9,       }
            ,{DUMMY_NAME_PTR_CNS,            0x0030095c+SINGLE_100G_OFFSET_CNS,         0x00000055,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300960+SINGLE_100G_OFFSET_CNS,         0x0000b95c,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300964+SINGLE_100G_OFFSET_CNS,         0x000000b2,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300968+SINGLE_100G_OFFSET_CNS,         0x0000f81a,       }
            ,{DUMMY_NAME_PTR_CNS,            0x0030096c+SINGLE_100G_OFFSET_CNS,         0x000000bd,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300970+SINGLE_100G_OFFSET_CNS,         0x0000c783,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300974+SINGLE_100G_OFFSET_CNS,         0x000000ca,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300978+SINGLE_100G_OFFSET_CNS,         0x00003635,       }
            ,{DUMMY_NAME_PTR_CNS,            0x0030097c+SINGLE_100G_OFFSET_CNS,         0x000000cd,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300980+SINGLE_100G_OFFSET_CNS,         0x000031c4,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300984+SINGLE_100G_OFFSET_CNS,         0x0000004c,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300988+SINGLE_100G_OFFSET_CNS,         0x0000d6ad,       }
            ,{DUMMY_NAME_PTR_CNS,            0x0030098c+SINGLE_100G_OFFSET_CNS,         0x000000b7,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300990+SINGLE_100G_OFFSET_CNS,         0x0000665f,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300994+SINGLE_100G_OFFSET_CNS,         0x0000002a,       }
            ,{DUMMY_NAME_PTR_CNS,            0x00300998+SINGLE_100G_OFFSET_CNS,         0x0000f0c0,       }
            ,{DUMMY_NAME_PTR_CNS,            0x0030099c+SINGLE_100G_OFFSET_CNS,         0x000000e5,       }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemPipeUnitLpSerdes function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the LpSerdes unit
*
* @param[in] devObjPtr                - pointer to common device memory object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitLpSerdes
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

/*SERDES*/
    /* took registers of offset 0x200 of Lion2 and convert to offset 0x800 */
    /* COMPHY_H %t Registers/KVCO Calibration Control (0x00000800 + t*0x1000: where t (0-35) represents SERDES) */
    {0x00000808, GOP_MASK_16_PORTS_CNS, NULL, 0, smemLion2ActiveWriteKVCOCalibrationControlReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* serdes external registers SERDESes for ports 0...16 */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 0x44)} , FORMULA_SINGLE_PARAMETER(PIPE_NUM_SERDESES_CNS , 0x1000)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* SERDES registers */
            {DUMMY_NAME_PTR_CNS,         0x00000000,         0x00000800,   PIPE_NUM_SERDESES_CNS  ,   0x1000    }
           ,{DUMMY_NAME_PTR_CNS,         0x00000004,         0x00008801,   PIPE_NUM_SERDESES_CNS  ,   0x1000    }
           ,{DUMMY_NAME_PTR_CNS,         0x00000008,         0x00000100,   PIPE_NUM_SERDESES_CNS  ,   0x1000    }
           /* must be last */
           ,{NULL,            0,         0x00000000,      0,    0x0      }
        };

        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}
/**
* @internal smemPipeUnitXGPortMib function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the XGPortMib unit
*
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitXGPortMib
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* XG port MAC MIB Counters */
    {0x00000000, SMEM_PIPE_XG_MIB_COUNT_MSK_CNS, smemLion2ActiveReadMsmMibCounters, 0, NULL,0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* steps between each port */
    devObjPtr->xgCountersStepPerPort   = PIPE_MIB_OFFSET_CNS;
    devObjPtr->xgCountersStepPerPort_1 = 0;/* not valid */
    /* offset of table xgPortMibCounters_1 */
    devObjPtr->offsetToXgCounters_1 = 0;/* not valid */
    devObjPtr->startPortNumInXgCounters_1 = 0;/* not valid */

    /* chunks with formulas */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* base of 0x00000000 */
            /* ports 0..15 */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000000 , 256)}, FORMULA_SINGLE_PARAMETER(PIPE_NUM_GOP_PORTS_XLG , PIPE_MIB_OFFSET_CNS)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    /* don't point the next 'half' unit on 'me' */
    unitPtr[1].hugeUnitSupportPtr = NULL;
}

/**
* @internal smemPipeUnitTxqDq function
* @endinternal
*
* @brief   Allocate address type specific memories -- TXQ DQ
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitTxqDq
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000028)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000208, 0x00000248)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000260, 0x00000260)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002C0, 0x000002E4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000424)}
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B40)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D40)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001008, 0x00001010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001040, 0x00001064)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001240, 0x0000133C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001380, 0x000013A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x00001500)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001510, 0x00001534)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001650, 0x000017A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000017B0, 0x000017B0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800, 0x00001800)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001904, 0x00001930)}
            /*Scheduler State Variable RAM*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 320), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(168,32),SMEM_BIND_TABLE_MAC(Scheduler_State_Variable)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x0000400C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004100, 0x00004124)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004300, 0x00004324)}
            /*Priority Token Bucket Configuration*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00006000, 640), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(488,64)}
            /*Port Token Bucket Configuration*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000A000, 80), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(61,8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C000, 0x0000C000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C008, 0x0000C044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C080, 0x0000C09C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C100, 0x0000C13C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C180, 0x0000C37C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000CA00, 0x0000CA00)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D000, 0x0000D004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D010, 0x0000D014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D040, 0x0000D048)}
            /*Egress STC Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000D800, 160), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(80,16),SMEM_BIND_TABLE_MAC(egressStc)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00011000, 80)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
/*9 MSBits of 'Addr', 23 LSBits of 'Addr',     val,    repeat,    skip,      block_repeat, block_skip*/
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x001fc140,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000208,         0x28002800,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000020c,         0x00000002,     16,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x000002c4,         0x00000001,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x000002c8,         0x00000002,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x000002cc,         0x00000003,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x000002d0,         0x00000004,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x000002d4,         0x00000005,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x000002d8,         0x00000006,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x000002dc,         0x00000007,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x000002e0,         0x00000008,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x000002e4,         0x00000009,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000a00,         0x0000ffff,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00001000,         0x14e44e15,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00001008,         0x00000002,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00001040,         0x00000003,     10,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00001240,         0x01010101,     16,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00001280,         0x05050505,     16,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00001300,         0x000000ff,     16,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00001500,         0x00001553,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00001650,         0x83828180,      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001654,         0x87868584,      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001658,         0x8b8a8988,      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000165c,         0x8f8e8d8c,      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001660,         0x93929190,      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001664,         0x97969594,      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001668,         0x9b9a9998,      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000166c,         0x9f9e9d9c,      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001670,         0xa3a2a1a0,      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001674,         0xa7a6a5a4,      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001678,         0xabaaa9a8,      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000167c,         0xafaeadac,      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001680,         0xb3b2b1b0,      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001684,         0xb7b6b5b4,      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001688,         0xbbbab9b8,      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000168c,         0xbfbebdbc,      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001690,         0xc3c2c1c0,      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001694,         0xc7c6c5c4,      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001904,         0x05050505,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00001908,         0x00000004,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00004000,         0x82080000,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00004004,         0x00000600,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00004300,         0x000000ff,     10,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x0000c000,         0x00000006,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000c084,         0x00000001,      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000c088,         0x00000002,      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000c08c,         0x00000003,      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000c090,         0x00000004,      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000c094,         0x00000005,      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000c098,         0x00000006,      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000c09c,         0x00000007,      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000d000,         0x00000020,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000d014,         0x00000008,      1,    0x0}

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

    {
        static SMEM_ACTIVE_MEM_ENTRY_STC smemPipeTxqDqActiveTable[] =
        {
            /* Self-clearing of trigger <UpdateSchedVarTrigger> of Scheduler Configuration Register */
            {0x00001000, SMEM_FULL_MASK_CNS, NULL, 0, smemLionActiveWriteSchedulerConfigReg, 0},

            /* !!! FOR TXQ_DQ_1 !!! */
            /* Self-clearing of trigger <UpdateSchedVarTrigger> of Scheduler Configuration Register */
            {0x01001000, SMEM_FULL_MASK_CNS, NULL, 0, smemLionActiveWriteSchedulerConfigReg, 0},

            /* must be last */
            {END_OF_TABLE, SMEM_FULL_MASK_CNS, NULL, 0, NULL, 0}
        };

        unitPtr->unitActiveMemPtr = smemPipeTxqDqActiveTable;
    }
}


/**
* @internal smemPipeUnitTxqLl function
* @endinternal
*
* @brief   Allocate address type specific memories -- TXQ ll (link list)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitTxqLl
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008000, 640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00018000, 640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00028000, 640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00030000, 640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00038000, 640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00048000, 640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00050000, 640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00058000, 640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000, 640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00068000, 640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00110000, 0x00110000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00112008, 0x0011200C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00112020, 0x00112030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00112100, 0x00112100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00113000, 0x00113000)}

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00110000,         0x00000008,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00113000,         0x0000ffff,      1,    0x0 }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemPipeUnitTxqQcn function
* @endinternal
*
* @brief   Allocate address type specific memories -- TXQ Qcn
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitTxqQcn
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B3C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C3C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000E1C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002018)}
            /*CN Sample Intervals Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 1280), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(40 ,8),SMEM_BIND_TABLE_MAC(CN_Sample_Intervals)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000200,         0x00000100,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000210,         0xffff0000,      1,    0x0 }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemPipeUnitTxqQueue function
* @endinternal
*
* @brief   Allocate address type specific memories -- TXQ queue
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitTxqQueue
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090040, 0x00090044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090060, 0x00090064)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090078, 0x00090084)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000900B0, 0x000900B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090180, 0x00090184)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00090400, 0x00090410)}
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0050, 0x000A009C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0200, 0x000A024C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0360, 0x000A03BC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A03D0, 0x000A040C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0440, 0x000A0450)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0500, 0x000A063C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0700, 0x000A0710)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A0720, 0x000A0728)}
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A4010, 0x000A405C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A4130, 0x000A417C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000A4250, 0x000A426C)}
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000AA400, 24), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64,8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000AA800, 24), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64,8)}

            /*Q Main Buff*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000C0000, 640), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(16,4)}
            /*Q Main MC Buff*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000D0000, 640), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(20,4)}

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }



    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            {DUMMY_NAME_PTR_CNS,            0x00090a80,         0x0000ffff,      1,    0x0  }
           ,{DUMMY_NAME_PTR_CNS,            0x00090a94,         0xffbf7f3f,      1,    0x0  }
           ,{DUMMY_NAME_PTR_CNS,            0x00093000,         0x0009f599,      1,    0x0  }
           ,{DUMMY_NAME_PTR_CNS,            0x00093020,         0x00001011,      1,    0x0  }
           ,{DUMMY_NAME_PTR_CNS,            0x000936a0,         0x0001e000,      1,    0x0  }
           ,{DUMMY_NAME_PTR_CNS,            0x000a0000,         0x0002c190,      1,    0x0  }
           ,{DUMMY_NAME_PTR_CNS,            0x000a0004,         0x00000004,     16,    0x4  }
           ,{DUMMY_NAME_PTR_CNS,            0x000a0044,         0x00000002,      1,    0x0  }
           ,{DUMMY_NAME_PTR_CNS,            0x000a0050,         0x000000ff,     20,    0x4  }
           ,{DUMMY_NAME_PTR_CNS,            0x000a03a0,         0xffffffff,      8,    0x4  }
           ,{DUMMY_NAME_PTR_CNS,            0x000a0500,         0xffffffff,     16,    0x4,      2,    0x80}
           ,{DUMMY_NAME_PTR_CNS,            0x000a0800,         0x00001fb0,      1,    0x0  }
           ,{DUMMY_NAME_PTR_CNS,            0x000a0810,         0x00002000,      1,    0x0  }
           ,{DUMMY_NAME_PTR_CNS,            0x000a0820,         0x00000019,     16,    0x4  }
           ,{DUMMY_NAME_PTR_CNS,            0x000a0900,         0x00000028,     16,    0x4  }
           ,{DUMMY_NAME_PTR_CNS,            0x000a0a00,         0x00000300,      8,    0x4  }
           ,{DUMMY_NAME_PTR_CNS,            0x000a0a20,         0x000002bc,      8,    0x4  }
           ,{DUMMY_NAME_PTR_CNS,            0x000a4250,         0x00000080,      8,    0x4  }
           ,{DUMMY_NAME_PTR_CNS,            0x000a9000,         0x000a000a,      1,    0x0  }
           ,{DUMMY_NAME_PTR_CNS,            0x000a9004,         0x0000000a,      2,    0xc  }
           ,{DUMMY_NAME_PTR_CNS,            0x000a9020,         0x00000800,      2,    0x10 }
           ,{DUMMY_NAME_PTR_CNS,            0x000aa004,         0x00000800,     16,    0x4  }
           ,{DUMMY_NAME_PTR_CNS,            0x000aa050,         0x00000400,     16,    0x4,      2,    0x40}
           ,{DUMMY_NAME_PTR_CNS,            0x000aa0d0,         0x00000100,     16,    0x4  }
           ,{DUMMY_NAME_PTR_CNS,            0x000aa110,         0x00001000,      8,    0x4  }
           ,{DUMMY_NAME_PTR_CNS,            0x000aa1b0,         0x00000400,      8,    0x4  }
           ,{DUMMY_NAME_PTR_CNS,            0x000aa210,         0x00002800,      1,    0x0  }
           ,{DUMMY_NAME_PTR_CNS,            0x000aa230,         0x00000100,      1,    0x0  }

           ,{NULL,                                   0,         0x00000000,      0,    0x0}
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}


/**
* @internal smemPipeUnitTxqBmx function
* @endinternal
*
* @brief   Allocate address type specific memories -- TXQ bmx
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitTxqBmx
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
            {DUMMY_NAME_PTR_CNS,            0x00000004,         0x01007802,      1,    0x0                       }
           ,{DUMMY_NAME_PTR_CNS,            0x00000030,         0x00000005,      1,    0x0                       }
           ,{NULL,                                   0,         0x00000000,      0,    0x0                       }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}


/**
* @internal smemPipeUnit__dummy function
* @endinternal
*
* @brief   Allocate address type specific memories -- dummy
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnit__dummy
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
* @internal smemPipeUnitTxqPfc function
* @endinternal
*
* @brief   Allocate address type specific memories -- TXQ pfc
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitTxqPfc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000030, 0x00000030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000040)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000050, 0x00000050)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000060, 0x00000060)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000D0, 0x000000D0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000F0, 0x000000FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002F0, 0x00000300)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000006F0, 0x000006F0)}
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001C00, 0x00001C7C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003104)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003200, 0x00003240)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008000, 544)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00014000, 0x0001401C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00014200, 0x0001421C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00014400, 0x0001441C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00015400, 0x00015408)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00015500, 0x00015500)}

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* UNIT_CPFC*/
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x00000001,      1,    0x0                  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001100,         0xffff0000,      1,    0x0                  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001110,         0x00000287,      1,    0x0                  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001800,         0x0007ffff,     64,    0x4                  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c04,         0x00000401,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c08,         0x00000802,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c0c,         0x00000c03,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c10,         0x00001004,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c14,         0x00001405,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c18,         0x00001806,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c1c,         0x00001c07,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c20,         0x00002008,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c24,         0x00002409,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c28,         0x0000280a,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c2c,         0x00002c0b,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c30,         0x0000300c,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c34,         0x0000340d,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c38,         0x0000380e,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c3c,         0x00003c0f,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c40,         0x00004010,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c44,         0x00004411,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c48,         0x00004812,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c4c,         0x00004c13,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c50,         0x00005014,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c54,         0x00005415,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c58,         0x00005816,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c5c,         0x00005c17,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c60,         0x00006018,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c64,         0x00006419,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c68,         0x0000681a,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c6c,         0x00006c1b,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c70,         0x0000701c,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c74,         0x0000741d,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c78,         0x0000781e,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c7c,         0x00007c1f,                       }
            ,{DUMMY_NAME_PTR_CNS,            0x00014000,         0x0007ffff,      1,    0x20,      7,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00014400,         0x0007ffff,      1,    0x20,      7,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00015400,         0x00000001,      1,    0x0                  }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemPipeUnitMcfc function
* @endinternal
*
* @brief   Allocate address type specific memories -- MCFC pfc
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPipeUnitMcfc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x000000B8)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

}

/**
* @internal smemPipeSpecificDeviceUnitAlloc_DP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DP units -- (non sip units)
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemPipeSpecificDeviceUnitAlloc_DP_units
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
        smemPipeUnitMg(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitGop(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_SERDES)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitLpSerdes(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_XG_PORT_MIB)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitXGPortMib(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_MPPM)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitMppm(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_RX_DMA)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitRxDma(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_DMA);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitTxDma(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_FIFO);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitTxFifo(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TAI)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitTai(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_BM);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitBm(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_BMA);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitBma(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_DQ)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitTxqDq(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_LL)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitTxqLl(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_QCN)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitTxqQcn(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_QUEUE)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitTxqQueue(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_BMX)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitTxqBmx(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_SBC);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnit__dummy(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_CPFC);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitTxqPfc(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_MCFC);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitMcfc(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP_LED_0);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitGopLed(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP_SMI_0);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitGopSmi(devObjPtr,currUnitChunkPtr);
    }


}


/**
* @internal smemPipeSpecificDeviceUnitAlloc_SIP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         SIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemPipeSpecificDeviceUnitAlloc_SIP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    GT_U32  unitIndex;

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_PCP);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitPcp(devObjPtr,currUnitChunkPtr);
    }


    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_PHA);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitPha(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_CNC);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemPipeUnitCnc(devObjPtr,currUnitChunkPtr,0);
    }


}

/**
* @internal smemPipeSpecificDeviceUnitAlloc_DFX_PEX_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DFX,PEX units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemPipeSpecificDeviceUnitAlloc_DFX_PEX_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;

    currUnitChunkPtr = &devMemInfoPtr->PEX_UnitMem;
    /* generate memory space for PEX server */
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemLion2UnitPex(devObjPtr,&devMemInfoPtr->PEX_UnitMem, devMemInfoPtr->common.pciUnitBaseAddr);
    }

    currUnitChunkPtr = &devMemInfoPtr->common.pciExtMemArr[SMEM_UNIT_PCI_BUS_DFX_E].unitMem;
    /* generate memory space for DFX server */
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemLion3UnitDfx(devObjPtr,
                         &devMemInfoPtr->common.pciExtMemArr[SMEM_UNIT_PCI_BUS_DFX_E].unitMem,
                         devMemInfoPtr->common.pciExtMemArr[SMEM_UNIT_PCI_BUS_DFX_E].unitBaseAddr);
    }
}



/**
* @internal smemPipeSpecificDeviceUnitAlloc function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemPipeSpecificDeviceUnitAlloc
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
        currUnitChunkPtr->numOfUnits = 1;

        /* point the next 'half' unit on 'me' */
        currUnitChunkPtr[1].hugeUnitSupportPtr = currUnitChunkPtr;
        currUnitChunkPtr[1].numOfUnits = 1;
        currUnitChunkPtr[1].chunkType = SMEM_UNIT_CHUNK_TYPE_9_MSB_E;
    }

    smemPipeSpecificDeviceUnitAlloc_DFX_PEX_units(devObjPtr);

    smemPipeSpecificDeviceUnitAlloc_DP_units(devObjPtr);

    smemPipeSpecificDeviceUnitAlloc_SIP_units(devObjPtr);



}

/**
* @internal pipeRegsInfoSet_part1 function
* @endinternal
*
* @brief   align the addresses of registers according to addresses of corresponding
*         units
* @param[in] devObjPtr                - pointer to device object.
*/
static void pipeRegsInfoSet_part1
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  unitBaseAddress;

    /*fill the DB with 'not valid addresses' */
    smemChtRegDbPreInit_part1(devObjPtr);
    smemChtRegDbPreInit_part2(devObjPtr);

    unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_RX_DMA);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,rxDMA[0]           ,unitBaseAddress);

    unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_GOP);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,GOP                ,unitBaseAddress);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,GOP_PTP            ,unitBaseAddress);

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_TAI))
    {
        /* TAI is NOT part of the GOP */
        unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TAI);
    }
    else
    {
        /* TAI is part of the GOP */
        unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_GOP);
    }
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,TAI                ,unitBaseAddress);


    unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_CNC);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,CNC[0]              ,unitBaseAddress);

    unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TXQ_QUEUE);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,TXQ.queue           ,unitBaseAddress);

    unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TXQ_LL);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,TXQ.ll              ,unitBaseAddress);

    unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TXQ_DQ);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,TXQ.dq[0]           ,unitBaseAddress);

    unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TXQ_QCN);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,TXQ.qcn              ,unitBaseAddress);

    unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_CPFC);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,TXQ.pfc             ,unitBaseAddress);


    unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_BM);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,BM     ,unitBaseAddress);

    unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TX_FIFO);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,TXFIFO[0]     ,unitBaseAddress);

    unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TX_DMA);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,TXDMA[0]     ,unitBaseAddress);

    unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_MPPM);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,MPPM     ,unitBaseAddress);

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_TXQ_DQ_1))
    {
        unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TXQ_DQ_1);
        SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,TXQ.dq[1]           ,unitBaseAddress);
    }

    unitBaseAddress =  UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_PCP);
    SMEM_PIPE_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,PCP     ,unitBaseAddress);

    unitBaseAddress =  UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_PHA);
    SMEM_PIPE_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,PHA     ,unitBaseAddress);

    unitBaseAddress =  UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_MCFC);
    SMEM_PIPE_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,MCFC     ,unitBaseAddress);


    unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_MG);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,MG       ,unitBaseAddress);
    unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_GOP);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,GOP.ports          ,unitBaseAddress);
    unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_GOP);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,GOP.packGenConfig       ,unitBaseAddress);
    unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_SERDES);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,GOP.SERDESes       ,unitBaseAddress);

}

/**
* @internal pipeRegsInfoSet_part2 function
* @endinternal
*
* @brief   align the addresses of registers according to addresses of corresponding
*         units
* @param[in] devObjPtr                - pointer to device object.
*/
static void pipeRegsInfoSet_part2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    SMEM_SIP5_PP_REGS_ADDR_STC      *regAddrDbSip5Ptr;
    SMEM_CHT_PP_REGS_ADDR_STC      *regAddrDbPtr;
    SMEM_PIPE_PP_REGS_ADDR_STC     *regAddrDbPtr_pipe;

    /* regDB : version SIP5 */
    regAddrDbSip5Ptr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr);

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP)];
    smemGenericRegistersArrayAlignToUnit(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->GOP),
        currUnitChunkPtr);

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP)];
    smemGenericRegistersArrayAlignToUnit(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->GOP_PTP),
        currUnitChunkPtr);

    currUnitChunkPtr = NULL;
    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_TAI))
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TAI)];
    }

    if(currUnitChunkPtr && currUnitChunkPtr->numOfChunks)
    {
        smemGenericRegistersArrayAlignForceUnitReset(GT_TRUE);

        /* the TAI is not part of the GOP base addresses */
        smemGenericRegistersArrayAlignToUnit(devObjPtr,
            REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->TAI),
            currUnitChunkPtr);

        smemGenericRegistersArrayAlignForceUnitReset(GT_FALSE);
    }
    else
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP)];

        /* the TAI is part of the GOP base addresses */
        smemGenericRegistersArrayAlignToUnit(devObjPtr,
            REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->TAI),
            currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_RX_DMA)];
    smemGenericRegistersArrayAlignToUnit(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->rxDMA[0]),
        currUnitChunkPtr);

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_CNC)];
    smemGenericRegistersArrayAlignToUnit1(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->CNC[0]),
        currUnitChunkPtr,
        GT_TRUE);/* addresses are '0' based and need += of the 'base unit address' */

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_QUEUE)];
    smemGenericRegistersArrayAlignToUnit1(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->TXQ.queue),
        currUnitChunkPtr,
        GT_TRUE);/* addresses are '0' based and need += of the 'base unit address' */

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_LL)];
    smemGenericRegistersArrayAlignToUnit1(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->TXQ.ll),
        currUnitChunkPtr,
        GT_TRUE);/* addresses are '0' based and need += of the 'base unit address' */

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_DQ)];
    smemGenericRegistersArrayAlignToUnit1(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->TXQ.dq[0]),
        currUnitChunkPtr,
        GT_TRUE);/* addresses are '0' based and need += of the 'base unit address' */

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_CPFC)];
    smemGenericRegistersArrayAlignToUnit1(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->TXQ.pfc),
        currUnitChunkPtr,
        GT_TRUE);/* addresses are '0' based and need += of the 'base unit address' */

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_QCN)];
    smemGenericRegistersArrayAlignToUnit1(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->TXQ.qcn),
        currUnitChunkPtr,
        GT_TRUE);/* addresses are '0' based and need += of the 'base unit address' */

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_BM)];
    smemGenericRegistersArrayAlignToUnit(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->BM),
        currUnitChunkPtr);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_FIFO)];
    smemGenericRegistersArrayAlignToUnit(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->TXFIFO[0]),
        currUnitChunkPtr);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_DMA)];
    smemGenericRegistersArrayAlignToUnit(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->TXDMA[0]),
        currUnitChunkPtr);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_MPPM)];
    smemGenericRegistersArrayAlignToUnit(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->MPPM),
        currUnitChunkPtr);

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_TXQ_DQ_1))
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_DQ_1)];
        smemGenericRegistersArrayAlignToUnit(devObjPtr,
            REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->TXQ.dq[1]),
            currUnitChunkPtr);
    }

    /* regDB : version 1 */
    regAddrDbPtr = SMEM_CHT_MAC_REG_DB_GET(devObjPtr);

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_MG)];
    smemGenericRegistersArrayAlignToUnit(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbPtr->MG),
        currUnitChunkPtr);

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP)];
    smemGenericRegistersArrayAlignToUnit(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbPtr->GOP.ports),
        currUnitChunkPtr);

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP)];
    smemGenericRegistersArrayAlignToUnit(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbPtr->GOP.packGenConfig),
        currUnitChunkPtr);


    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_SERDES)];
    smemGenericRegistersArrayAlignToUnit(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbPtr->GOP.SERDESes),
        currUnitChunkPtr);

    regAddrDbPtr_pipe = SMEM_PIPE_MAC_REG_DB_GET(devObjPtr);

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_PCP)];
    smemGenericRegistersArrayAlignToUnit(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbPtr_pipe->PCP),
        currUnitChunkPtr);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_PHA)];
    smemGenericRegistersArrayAlignToUnit(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbPtr_pipe->PHA),
        currUnitChunkPtr);

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_MCFC)];
    smemGenericRegistersArrayAlignToUnit(devObjPtr,
        REG_PTR_AND_SIZE_MAC(regAddrDbPtr_pipe->MCFC),
        currUnitChunkPtr);

}

/**
* @internal pipeUnitMCFCRegsInfoSet function
* @endinternal
*
* @brief   Init memory unit MCFC in DB .
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void pipeUnitMCFCRegsInfoSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    {/*start of unit MCFC */
        {/*start of unit interrupts */
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.interrupts.MCFCInterruptsCause = 0x0000005c;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.interrupts.MCFCInterruptsMask = 0x00000060;

        }/*end of unit interrupts */


        {/*start of unit cntrs */
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.cntrs.PFCReceivedCntr = 0x00000010;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.cntrs.MCReceivedCntr = 0x00000014;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.cntrs.UCReceivedCntr = 0x00000018;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.cntrs.QCNReceivedCntr = 0x0000001c;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.cntrs.MCQCNReceivedCntr = 0x00000020;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.cntrs.outgoingQCNPktsCntr = 0x00000040;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.cntrs.outgoingMcPktsCntr = 0x00000038;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.cntrs.outgoingQCN2PFCMsgsCntr = 0x0000003c;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.cntrs.outgoingMCQCNPktsCntr = 0x0000004c;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.cntrs.outgoingPFCPktsCntr = 0x00000044;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.cntrs.outgoingUcPktsCntr = 0x00000048;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.cntrs.ingrDropCntr = 0x00000024;

        }/*end of unit cntrs */


        {/*start of unit config */
            {/*1b000064+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 16 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.config.QCNMCPortmapForPort[n] =
                        0x00000064+0x4*n;
                }/* end of loop n */
            }/*1b000064+0x4*n*/
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.config.PFCConfig = 0x00000000;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.config.QCNGlobalConfig = 0x00000004;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.config.portsQCN2PFCEnableConfig = 0x00000008;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.config.portsQCNEnableConfig = 0x0000000c;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.config.UCMCArbiterConfig = 0x00000050;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->MCFC.config.ingrTrafficTrigQCNArbiterConfig = 0x00000054;

        }/*end of unit config */


    }/*end of unit MCFC */

}


/**
* @internal pipeUnitPCP_A1_RegsInfoSet function
* @endinternal
*
* @brief   Init memory unit PCP in DB .
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void pipeUnitPCP_A1_RegsInfoSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    {/*start of unit PCP */
        {/*start of unit PRS */
            {/*e000d40+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPacketTypeKeyTableIP2MEMatchIndexEntry[n] =
                        0x00000d40+0x4*n;
                }/* end of loop n */
            }/*e000d40+0x4*n*/
            {/*e000dc0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPacketTypeKeyTableIP2MEMatchIndexEntryMask[n] =
                        0x00000dc0+0x4*n;
                }/* end of loop n */
            }/*e000dc0+0x4*n*/

        }/*end of unit PRS */
        {/*start of unit PORTMAP */
            {/*e004314+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 255; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PORTMAP.PCPLagDesignatedPortEntry[n] =
                        0x00004314+0x4*n;
                }/* end of loop n */
            }/*e004314+0x4*n*/
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PORTMAP.PCPPortsEnableConfig = 0x00004714;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PORTMAP.PCPPacketTypeEnableEgressPortFiltering = 0x00004718;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PORTMAP.PCPPacketTypeEnableIngressPortFiltering = 0x0000471c;

        }/*end of unit PORTMAP */
        {/*start of unit HASH  */
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.HASH.PCPPacketTypeLagTableNumber = 0x00001814;

        }/*end of unit HASH  */
        {/*start of unit general */
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.general.PCPGlobalConfiguration = 0x000050F0;

        }/*end of unit general */

        {/*start of unit IP2ME */
            {/*e006000+0x4*p+0x16*n*/
                GT_U32    p,n;
                for(p = 0 ; p <= 3 ; p++) {
                    for(n = 0 ; n <= 6 ; n++) {
                        SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.IP2ME.PCPIP2MEIpPart[p][n] =
                            0x00006000+0x4*p+0x10*n;
                    }/* end of loop n */
                }/* end of loop p    */
            }/*e006000+0x4*p+0x16*n*/
            {/*e0060a0+0x4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 6 ; p++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.IP2ME.PCPIP2MEControlBitsEntry[p] =
                        0x000060a0+0x4*p;
                }/* end of loop p    */
            }/*e0060a0+0x4*p*/

        }/*end of unit IP2ME*/
    }/*end of unit PCP */
}

/**
* @internal pipeUnitPCPRegsInfoSet function
* @endinternal
*
* @brief   Init memory unit PCP in DB .
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void pipeUnitPCPRegsInfoSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    {/*start of unit PCP */
        {/*start of unit PTP */
            {/*e002620+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PTP.PCPPTPTypeKeyTableUDPAndPort_profileEntry[n] =
                        0x00002620+0x4*n;
                }/* end of loop n */
            }/*e002620+0x4*n*/
            {/*e002000+0x4*m+ 0x8*n*/
                GT_U32    m,n;
                for(m = 0 ; m <= 1 ; m++) {
                    for(n = 0 ; n <= 16 ; n++) {
                        SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PTP.PCPPortPTPTypeKeyUDBPConfigPort[m][n] =
                            0x00002000+0x4*m+ 0x8*n;
                    }/* end of loop n */
                }/* end of loop m */
            }/*e002000+0x4*m+ 0x8*n*/
            {/*e002088+0x4*(m-2) + 0x18*n*/
                GT_U32    m,n;
                for(m = 2 ; m <= 7 ; m++) {
                    for(n = 0 ; n <= 16 ; n++) {
                        SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PTP.PCPPortPTPTypeKeyUDBConfigPort[m-2][n] =
                            0x00002088+0x4*(m-2) + 0x18*n;
                    }/* end of loop n */
                }/* end of loop m */
            }/*e002088+0x4*(m-2) + 0x18*n*/
            {/*e002aa0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PTP.PCPPTPTypeKeyTableUDPAndPortProfileEntryMask[n] =
                        0x00002aa0+0x4*n;
                }/* end of loop n */
            }/*e002aa0+0x4*n*/
            {/*e0026a0+0x4*m +0x8*n*/
                GT_U32    m,n;
                for(m = 0 ; m <= 1 ; m++) {
                    for(n = 0 ; n <= 31 ; n++) {
                        SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PTP.PCPPTPTypeKeyTableUDBPEntryMask[m][n] =
                            0x000026a0+0x4*m +0x8*n;
                    }/* end of loop n */
                }/* end of loop m */
            }/*e0026a0+0x4*m +0x8*n*/
            {/*e002220+0x4*m + 0x8*n*/
                GT_U32    m,n;
                for(m = 0 ; m <= 1 ; m++) {
                    for(n = 0 ; n <= 31 ; n++) {
                        SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PTP.PCPPTPTypeKeyTableUDBPEntry[m][n] =
                            0x00002220+0x4*m + 0x8*n;
                    }/* end of loop n */
                }/* end of loop m */
            }/*e002220+0x4*m + 0x8*n*/
            {/*e0027a0+0x4*(m-2) + 0x18*n*/
                GT_U32    m,n;
                for(m = 2 ; m <= 7 ; m++) {
                    for(n = 0 ; n <= 31 ; n++) {
                        SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PTP.PCPPTPTypeKeyTableUDBEntryMask[m-2][n] =
                            0x000027a0+0x4*(m-2) + 0x18*n;
                    }/* end of loop n */
                }/* end of loop m */
            }/*e0027a0+0x4*(m-2) + 0x18*n*/
            {/*e002320+0x4*(m-2) +0x18*n*/
                GT_U32    m,n;
                for(m = 2 ; m <= 7 ; m++) {
                    for(n = 0 ; n <= 31 ; n++) {
                        SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PTP.PCPPTPTypeKeyTableUDBEntry[m-2][n] =
                            0x00002320+0x4*(m-2) +0x18*n;
                    }/* end of loop n */
                }/* end of loop m */
            }/*e002320+0x4*(m-2) +0x18*n*/
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PTP.PCPPTPTypeKeyTableEntriesEnable = 0x00002b20;

        }/*end of unit PTP */


        {/*start of unit PRS */
            {/*e000044+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPTPIDTable[n] =
                        0x00000044+0x4*n;
                }/* end of loop n */
            }/*e000044+0x4*n*/
            {/*e0000ac+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 16 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPortPktTypeKeyTableExt[n] =
                        0x000000ac+0x4*n;
                }/* end of loop n */
            }/*e0000ac+0x4*n*/
            {/*e000068+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 16 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPortPktTypeKeyTable[n] =
                        0x00000068+0x4*n;
                }/* end of loop n */
            }/*e000068+0x4*n*/
            {/*e000000+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 16 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPortDaOffsetConfig[n] =
                        0x00000000+0x4*n;
                }/* end of loop n */
            }/*e000000+0x4*n*/
            {/*e000d00+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyUDBPMissValue[n] =
                        0x00000d00+0x4*n;
                }/* end of loop n */
            }/*e000d00+0x4*n*/
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyTPIDEtherTypeMissValue = 0x00000d10;
            {/*e000680+0x4*m + 0x10*n*/
                GT_U32    n,m;
                for(n = 0 ; n <= 31 ; n++) {
                    for(m = 0 ; m <= 3 ; m++) {
                        SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyTableUDBPEntryMask[n][m] =
                            0x00000680+0x4*m + 0x10*n;
                    }/* end of loop m */
                }/* end of loop n */
            }/*e000680+0x4*m + 0x10*n*/
            {/*e000200+0x4*m + 0x10*n*/
                GT_U32    n,m;
                for(n = 0 ; n <= 31 ; n++) {
                    for(m = 0 ; m <= 3 ; m++) {
                        SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyTableUDBPEntry[n][m] =
                            0x00000200+0x4*m + 0x10*n;
                    }/* end of loop m */
                }/* end of loop n */
            }/*e000200+0x4*m + 0x10*n*/
            {/*e000900+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyTableTPIDEtherTypeEntryMask[n] =
                        0x00000900+0x4*n;
                }/* end of loop n */
            }/*e000900+0x4*n*/
            {/*e000400+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyTableTPIDEtherTypeEntry[n] =
                        0x00000400+0x4*n;
                }/* end of loop n */
            }/*e000400+0x4*n*/
            {/*e000580+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyTableSRCPortProfileEntry[n] =
                        0x00000580+0x4*n;
                }/* end of loop n */
            }/*e000580+0x4*n*/
            {/*e000980+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyTableMACDA4MSBEntryMask[n] =
                        0x00000980+0x4*n;
                }/* end of loop n */
            }/*e000980+0x4*n*/
            {/*e000480+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyTableMACDA4MSBEntry[n] =
                        0x00000480+0x4*n;
                }/* end of loop n */
            }/*e000480+0x4*n*/
            {/*e000a00+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyTableMACDA2LSBEntryMask[n] =
                        0x00000a00+0x4*n;
                }/* end of loop n */
            }/*e000a00+0x4*n*/
            {/*e000500+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyTableMACDA2LSBEntry[n] =
                        0x00000500+0x4*n;
                }/* end of loop n */
            }/*e000500+0x4*n*/
            {/*e000600+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyTableIsLLCNonSnapEntry[n] =
                        0x00000600+0x4*n;
                }/* end of loop n */
            }/*e000600+0x4*n*/
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyTableEntriesEnable = 0x00000b80;
            {/*e000a80+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyTableSRCPortProfileEntryMask[n] =
                        0x00000a80+0x4*n;
                }/* end of loop n */
            }/*e000a80+0x4*n*/
            {/*e000b00+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyTableIsLLCNonSnapEntryMask[n] =
                        0x00000b00+0x4*n;
                }/* end of loop n */
            }/*e000b00+0x4*n*/
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeySRCPortProfileMissValue = 0x00000d1c;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyMACDA4MSBMissValue = 0x00000d14;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyMACDA2LSBMissValue = 0x00000d18;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyLookUpMissDetected = 0x00000d24;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyLookUpMissCntr = 0x00000d28;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPPktTypeKeyIsLLCNonSnapMissValue = 0x00000d20;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPGlobalEtherTypeConfig4 = 0x00000064;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPGlobalEtherTypeConfig3 = 0x00000060;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPGlobalEtherTypeConfig2 = 0x0000005c;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPGlobalEtherTypeConfig1 = 0x00000058;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPGlobalEtherTypeConfig0 = 0x00000054;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PRS.PCPECNEnableConfig = 0x00000d2c;

        }/*end of unit PRS */


        {/*start of unit PORTMAP */
            {/*e004000+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PORTMAP.PCPPktTypeDestinationFormat0Entry[n] =
                        0x00004000+0x4*n;
                }/* end of loop n */
            }/*e004000+0x4*n*/
            {/*e004080+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PORTMAP.PCPPktTypeDestinationFormat1Entry[n] =
                        0x00004080+0x4*n;
                }/* end of loop n */
            }/*e004080+0x4*n*/
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PORTMAP.PCPPortsEnableConfig = 0x00004614;
            {/*e004280+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PORTMAP.PCPPktTypePortFilterFormat2Entry[n] =
                        0x00004280+0x4*n;
                }/* end of loop n */
            }/*e004280+0x4*n*/
            {/*e004200+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PORTMAP.PCPPktTypePortFilterFormat1Entry[n] =
                        0x00004200+0x4*n;
                }/* end of loop n */
            }/*e004200+0x4*n*/
            {/*e004180+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PORTMAP.PCPPktTypePortFilterFormat0Entry[n] =
                        0x00004180+0x4*n;
                }/* end of loop n */
            }/*e004180+0x4*n*/
            {/*e004100+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PORTMAP.PCPPktTypeDestinationFormat2Entry[n] =
                        0x00004100+0x4*n;
                }/* end of loop n */
            }/*e004100+0x4*n*/
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PORTMAP.PCPDstIdxExceptionCntr = 0x00004300;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PORTMAP.PCPPortFilterIdxExceptionCntr = 0x00004304;
            {/*e004314+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PORTMAP.PCPLagDesignatedPortEntry[n] =
                        0x00004314+0x4*n;
                }/* end of loop n */
            }/*e004314+0x4*n*/
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PORTMAP.PCPPortFilterExceptionForwardingPortMap = 0x00004310;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.PORTMAP.PCPDSTExceptionForwardingPortMap = 0x0000430c;

        }/*end of unit PORTMAP */


        {/*start of unit HASH */
            {/*e001050+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 8 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.HASH.PCPPktTypeHashMode2Entry[n] =
                        0x00001050+0x4*n;
                }/* end of loop n */
            }/*e001050+0x4*n*/
            {/*e001804+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.HASH.PCPLFSRSeed[n] =
                        0x00001804+4*n;
                }/* end of loop n */
            }/*e001804+4*n*/
            {/*e00102c+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 8 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.HASH.PCPPktTypeHashMode1Entry[n] =
                        0x0000102c+0x4*n;
                }/* end of loop n */
            }/*e00102c+0x4*n*/
            {/*e001008+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 8 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.HASH.PCPPktTypeHashMode0Entry[n] =
                        0x00001008+0x4*n;
                }/* end of loop n */
            }/*e001008+0x4*n*/
            {/*e001098+0x4*m + 0x54*n*/
                GT_U32    m,n;
                for(m = 0 ; m <= 20 ; m++) {
                    for(n = 0 ; n <= 8 ; n++) {
                        SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.HASH.PCPPktTypeHashConfigUDBPEntry[m][n] =
                            0x00001098+0x4*m + 0x54*n;
                    }/* end of loop n */
                }/* end of loop m */
            }/*e001098+0x4*m + 0x54*n*/
            {/*e001074+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 8 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.HASH.PCPPktTypeHashConfigIngrPort[n] =
                        0x00001074+0x4*n;
                }/* end of loop n */
            }/*e001074+0x4*n*/
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.HASH.PCPHashPktTypeUDE1EtherType = 0x00001000;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.HASH.PCPHashPktTypeUDE2EtherType = 0x00001004;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.HASH.PCPHashCRC32Seed = 0x00001800;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.HASH.PCPLagTableIndexMode = 0x0000180c;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.HASH.PCPPrngCtrl = 0x00001810;

        }/*end of unit HASH  */


        {/*start of unit general */
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.general.PCPInterruptsCause = 0x00005000;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.general.PCPInterruptsMask = 0x00005004;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.general.PCPDebug = 0x00005008;

        }/*end of unit general */


        {/*start of unit COS */
            {/*e003430+0x40*p+0x4*n*/
                GT_U32    p,n;
                for(p = 0 ; p <= 16 ; p++) {
                    for(n = 0 ; n <= 15 ; n++) {
                        SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.COS.PCPPortL2CoSMapEntry[p][n] =
                            0x00003430+0x40*p+0x4*n;
                    }/* end of loop n */
                }/* end of loop p    */
            }/*e003430+0x40*p+0x4*n*/
            {/*e0033e0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 16 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.COS.PCPPortCoSAttributes[n] =
                        0x000033e0+0x4*n;
                }/* end of loop n */
            }/*e0033e0+0x4*n*/
            {/*e0033c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.COS.PCPMPLSCoSMapEntry[n] =
                        0x000033c0+0x4*n;
                }/* end of loop n */
            }/*e0033c0+0x4*n*/
            {/*e0032c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 63 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.COS.PCPL3CoSMapEntry[n] =
                        0x000032c0+0x4*n;
                }/* end of loop n */
            }/*e0032c0+0x4*n*/
            {/*e0030c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.COS.PCPDsaCoSMapEntry[n] =
                        0x000030c0+0x4*n;
                }/* end of loop n */
            }/*e0030c0+0x4*n*/
            {/*e003000+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PCP.COS.PCPCoSFormatTableEntry[n] =
                        0x00003000+0x4*n;
                }/* end of loop n */
            }/*e003000+0x4*n*/

        }/*end of unit COS */


    }/*end of unit PCP */

    if(SKERNEL_IS_PIPE_REVISON_A1_DEV(devObjPtr))
    {
        /* Pipe revision A1 */
        pipeUnitPCP_A1_RegsInfoSet(devObjPtr);
    }
}

/**
* @internal pipeUnitPHARegsInfoSet function
* @endinternal
*
* @brief   Init memory unit PHA in DB .
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void pipeUnitPHARegsInfoSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{

    {/*start of unit units */
        {/*start of unit pha_regs */
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.pha_regs.PHACtrl = 0x007fff00;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.pha_regs.PHAStatus = 0x007fff04;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.pha_regs.portInvalidCRCMode = 0x007fff10;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.pha_regs.egrTimestampConfig = 0x007fff14;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.pha_regs.PHAGeneralConfig = 0x007fff18;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.pha_regs.PHATablesSERCtrl = 0x007fff30;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.pha_regs.PHAInterruptSumCause = 0x007fff60;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.pha_regs.PHASERErrorCause = 0x007fff40;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.pha_regs.PHASERErrorMask = 0x007fff44;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.pha_regs.PHAInternalErrorCause = 0x007fff48;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.pha_regs.PHAInternalErrorMask = 0x007fff4c;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.pha_regs.headerSERCtrl = 0x007fff34;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.pha_regs.PHAInterruptSumMask = 0x007fff64;

        }/*end of unit pha_regs */


    }/*end of unit units */

    {/*start of unit PPA */
        {/*start of unit ppa_regs */
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPA.ppa_regs.PPACtrl = 0x007eff00;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPA.ppa_regs.PPAInterruptSumCause = 0x007eff10;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPA.ppa_regs.PPAInterruptSumMask = 0x007eff14;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPA.ppa_regs.PPAInternalErrorCause = 0x007eff20;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPA.ppa_regs.PPAInternalErrorMask = 0x007eff24;

        }/*end of unit ppa_regs */


    }/*end of unit PPA */

    {/*start of unit PPG */
        {/*start of unit ppg_regs */
                SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPG[0].ppg_regs.PPGInterruptSumMask  = 0x0007FF04;
                SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPG[0].ppg_regs.PPGInterruptSumCause = 0x0007FF00;
                SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPG[0].ppg_regs.PPGInternalErrorMask = 0x0007FF14;
                SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPG[0].ppg_regs.PPGInternalErrorCause = 0x0007FF10;

        }/*end of unit ppg_regs */


        {/*ffffffff+m*0x80000*/
            GT_U32    m;
            for(m = 0 ; m <= 3 ; m++) {
                GT_U32        unitArrayIndex;
                GT_U32        *unitArray_0_ptr,*currU32Ptr;
                currU32Ptr = (void*)&SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPG[m] ;
                unitArray_0_ptr = (void*)&(SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPG[0]);
                /* loop on all elements in the unit , and set/update the address of register/table */
                for(unitArrayIndex = 0 ;
                    unitArrayIndex < NUM_ELEMENTS_IN_UNIT_MAC(SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPG[0],GT_U32) ;
                    unitArrayIndex++ , currU32Ptr++){
                    (*currU32Ptr) = unitArray_0_ptr[unitArrayIndex] + 0* 0xffffffff+m*0x80000;
                } /*loop on unitArrayIndex */
            }/* end of loop m */
        }/*ffffffff+m*0x80000*/
    }/*end of unit PPG[4] */

    {/*start of unit PPN */
        {/*start of unit ppn_regs */
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPN[0][0].ppn_regs.PPNDoorbell = 0x00003010;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPN[0][0].ppn_regs.pktHeaderAccessOffset = 0x00003024;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPN[0][0].ppn_regs.PPNInternalErrorCause = 0x00003040;
            SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPN[0][0].ppn_regs.PPNInternalErrorMask = 0x00003044;

        }/*end of unit ppn_regs */


        {/*0+0x80000 * m + 0x4000 * p*/
            GT_U32    p,m;
            for(p = 0 ; p <= 7 ; p++) {
                for(m = 0 ; m <= 3 ; m++) {
                    GT_U32        unitArrayIndex;
                    GT_U32        *unitArray_0_ptr,*currU32Ptr;
                    currU32Ptr = (void*)&SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPN[p][m] ;
                    unitArray_0_ptr = (void*)&(SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPN[0][0]);
                    /* loop on all elements in the unit , and set/update the address of register/table */
                    for(unitArrayIndex = 0 ;
                        unitArrayIndex < NUM_ELEMENTS_IN_UNIT_MAC(SMEM_PIPE_MAC_REG_DB_GET(devObjPtr)->PHA.PPN[0][0],GT_U32) ;
                        unitArrayIndex++ , currU32Ptr++){
                        (*currU32Ptr) = unitArray_0_ptr[unitArrayIndex] + 0* 0x0+0x80000 * m + 0x4000 * p;
                    } /*loop on unitArrayIndex */
                }/* end of loop m */
            }/* end of loop p */
        }/*0+0x80000 * m + 0x4000 * p*/
    }/*end of unit PPN[8][4] */

}

/**
* @internal pipeRegsInfoSet function
* @endinternal
*
* @brief   Init memory module for PIPE device.
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void pipeRegsInfoSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    pipeRegsInfoSet_part1(devObjPtr);

    /* need to be called before pipeRegsInfoSet_part2 */
    smemLion2MgRegDbInit(devObjPtr);
    smemLion2GopRegDbInit(devObjPtr);
    smemSip5RegsInfoSet(devObjPtr);
    pipeUnitMCFCRegsInfoSet(devObjPtr);
    pipeUnitPCPRegsInfoSet(devObjPtr);
    pipeUnitPHARegsInfoSet(devObjPtr);

    smemDfxServerRegsInfoSet(devObjPtr);
    smemInternalPciRegsInfoSet(devObjPtr);

    pipeRegsInfoSet_part2(devObjPtr);
}

/**
* @internal pipeTableInfoSet function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*
* @param[in] devObjPtr                - device object PTR.
*/
static void pipeTableInfoSet
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    smemChtTableNamesSet(devObjPtr);
    /* nothing to change */
    return ;
}

/**
* @internal initBaseAddressOfUnitActiveMemory function
* @endinternal
*
* @brief   set base address to all entries of the active memory of the unit.
*/
static void initBaseAddressOfUnitActiveMemory(
    SMEM_UNIT_CHUNKS_STC        *unitChunksPtr
)
{
    GT_U32              ii;
    GT_U32              baseAddr = unitChunksPtr->chunkIndex << SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS;
    SMEM_ACTIVE_MEM_ENTRY_STC   *unitActiveMemPtr;

    unitActiveMemPtr = unitChunksPtr->unitActiveMemPtr;

    if(unitActiveMemPtr == NULL)
    {
        unitActiveMemPtr = unitChunksPtr->unitActiveMemPtr = smemEmptyActiveMemoryArr;
    }

    /* first of all need to check if this active memory was already initialized.
        by other unit (huge unit support)
        or
        by other device (multi devices)
        or
        before system reset
    */
    for (ii = 0; unitActiveMemPtr[ii].address != END_OF_TABLE; ii++)
    {
        /* do nothing .. we look for the index of the last entry */
    }

    if(ii == 0)
    {
        /* one dummy entry ... no need to set indications */
        return;
    }

    /* we found the last entry -- the 'termination' entry */
    if(unitActiveMemPtr[ii].mask == SMEM_FULL_MASK_CNS)
    {
        /* indication that DB was already added base address */
        unitActiveMemPtr[ii].mask = baseAddr;
    }
    else
    {
        /* DB was already added the base address ... must not do it again */
        return;
    }

    /* add the address */
    for (ii = 0; unitActiveMemPtr[ii].address != END_OF_TABLE; ii++)
    {
        /* update the address */
        unitActiveMemPtr[ii].address += baseAddr;
    }
}

/**
* @internal pipeBindPerUnitActiveMemToSpecFunTbl function
* @endinternal
*
* @brief   bind per unit active memory to 'specFunTbl'
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] commonDevMemInfoPtr      - pointer to common device memory object.
*/
static void pipeBindPerUnitActiveMemToSpecFunTbl
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr
)
{
    GT_U32              jj;
    SMEM_UNIT_CHUNKS_STC        *unitChunksPtr;

    /* bind the units to the specific unit chunk with the generic function */
    for (jj = 0; jj < SMEM_CHT_NUM_UNITS_MAX_CNS ; jj++)
    {
        /* get the unit info from the 'specParam' */
        unitChunksPtr = (void*)(GT_UINTPTR)commonDevMemInfoPtr->specFunTbl[jj].specParam;
        if(unitChunksPtr == NULL)
        {
            continue;
        }

        commonDevMemInfoPtr->specFunTbl[jj].unitActiveMemPtr =
            unitChunksPtr->unitActiveMemPtr;

        initBaseAddressOfUnitActiveMemory(unitChunksPtr);
    }
}


/**
* @internal pipeTableInfoSetPart2 function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*         AFTER the bound of memories (after calling smemBindTablesToMemories)
* @param[in] devObjPtr                - device object PTR.
*/
static void pipeTableInfoSetPart2
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    devObjPtr->tablesInfo.xgPortMibCounters.commonInfo.baseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_XG_PORT_MIB) + 0;
    devObjPtr->tablesInfo.xgPortMibCounters.paramInfo[0].step = devObjPtr->xgCountersStepPerPort;
    devObjPtr->tablesInfo.cncMemory.paramInfo[0].step = 8*_1K;/* block --> 1K entries of 2 words (8 bytes)*/
    devObjPtr->tablesInfo.cncMemory.paramInfo[1].step = 8;/*index (2 counters per entry --> 8 bytes)*/
    smemLion3MemoriesInitTxq(devObjPtr);
}


/**
* @internal pipeMallocMem function
* @endinternal
*
* @brief   melloc PIPE device memory.
*/
static void pipeMallocMem
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr;

    devMemInfoPtr = smemDeviceObjMemoryAlloc(devObjPtr,1, sizeof(SMEM_CHT_GENERIC_DEV_MEM_INFO));
    if (devMemInfoPtr == 0)
    {
        skernelFatalError("pipeMallocMem: allocation error\n");
    }

    devObjPtr->deviceMemory = devMemInfoPtr;
    commonDevMemInfoPtr= &devMemInfoPtr->common;
    commonDevMemInfoPtr->isPartOfGeneric = 1;

    devObjPtr->supportRegistersDb = 1;
    devObjPtr->designatedPortVersion = 1;
    devObjPtr->numOfTxqUnits = 1;

    /* support SIP5 */
    devObjPtr->unitEArchEnable.txq = 1;/* the TQX unit hold no 'enable bit' */

    devObjPtr->devMemUnitMemoryGetPtr = NULL;
    devObjPtr->supportEArch = 1;

    devObjPtr->cncNumOfUnits = 1;

    devObjPtr->trafficGeneratorSupport.tgSupport = 1;
    devObjPtr->memUnitBaseAddrInfo.CNC[0] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_CNC);

    devObjPtr->cncBlocksNum = 2;
    devObjPtr->cncBlockMaxRangeIndex = _1K;

    devObjPtr->supportTxQGlobalPorts = 1;

    devObjPtr->txqRevision = 1;
    devObjPtr->supportXgMacMibCountersControllOnAllPorts = 1;
    devObjPtr->supportCutThrough = 1;
    devObjPtr->supportResourceHistogram = 1;
    devObjPtr->supportPtp = 1;

    devObjPtr->supportForwardFcPackets = 1;

    devObjPtr->isMsmMibOnAllPorts = 1;
    devObjPtr->isMsmGigPortOnAllPorts = 1;
    devObjPtr->support1024to1518MibCounter = 1;

    SMEM_CHT_IS_DFX_SERVER(devObjPtr) = GT_TRUE;
    SMEM_CHT_IS_DFX_ON_UNIQUE_MEMORY_SPCAE(devObjPtr) = GT_TRUE;


    devObjPtr->portMibCounters64Bits = 1;
    devObjPtr->supportSingleTai = 1;
    SET_IF_ZERO_MAC(devObjPtr->numOfTaiUnits ,1);

    {
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.phyPort , 9);
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.trunkId , 12);
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.ePort , 14);/*16K*/
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.eVid , 13);/*8K*/
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.eVidx , 14);/*16K*/
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.hwDevNum , 10);/*1K*/

        SET_IF_ZERO_MAC(devObjPtr->cncClientSupportBitmap,SNET_PIPE_CNC_CLIENTS_BMP_ALL_CNS);

        devObjPtr->supportCutThroughFastToSlow = 1;
    }

    devObjPtr->devMemInterruptTreeInit = NULL;

    /* support active memory per unit */
    devObjPtr->supportActiveMemPerUnit = 1;

    devObjPtr->cpuPortNoSpecialMac = 1;/* not have special mac for CPU port (when work in SGMII) */

    devObjPtr->unitBaseAddrAlignmentNumBits = 24;

    devObjPtr->portMacSecondBase = 0;
    devObjPtr->portMacSecondBaseFirstPort = 0;

    devObjPtr->errata.txqEgressMibCountersNotClearOnRead = 0;/* issue fixed in bobk*//*[JIRA] (TXQ-1147) */

    devObjPtr->activeMemPtr = dummy_ActiveTable;

    /* PCI Express base address conflict with the MG address space.                     */
    /* Please remember that access to PCI Express registers (through CPSS) must be done */
    /* with the dedicated PCI/PEX APIs!!!                                               */
    commonDevMemInfoPtr->pciUnitBaseAddrMask = 0x000f0000;
    commonDevMemInfoPtr->pciUnitBaseAddr = 0x00040000;
    commonDevMemInfoPtr->accessPexMemorySpaceOnlyOnExplicitAction = 1;

    commonDevMemInfoPtr->dfxMemBaseAddr = 0x0;
    commonDevMemInfoPtr->dfxMemBaseAddrMask = 0x0;

    devObjPtr->devFindMemFunPtr = (void *)smemGenericFindMem;

    /* init specific functions array */
    smemLion2InitFuncArray(devObjPtr,commonDevMemInfoPtr);

    /* init the internal memory */
    smemChtInitInternalSimMemory(devObjPtr,commonDevMemInfoPtr);

    /* Alloc the unit of the PIPE */
    smemPipeSpecificDeviceUnitAlloc(devObjPtr);

    pipeRegsInfoSet(devObjPtr);

    pipeTableInfoSet(devObjPtr);

    if(devObjPtr->devMemSpecificDeviceMemInitPart1)
    {
        devObjPtr->devMemSpecificDeviceMemInitPart1(devObjPtr);
    }

    if(devObjPtr->unitsDuplicationsPtr)
    {
        /* do duplications of the units */
        smemCheetahDuplicateUnits(devObjPtr);
    }

    /* no need in bobcat2 to do explicit call to
       smemGenericUnitAddressesAlignToBaseAddress1 for the'extra units' ,
       because smemLion3UnitMemoryBindToChunk will do it. */
    smemLion2UnitMemoryBindToChunk(devObjPtr);

    /* bind now tables to memories --
       MUST be done after calling smemLion2TableInfoSet(...)
       so the tables can override previous settings */
    smemBindTablesToMemories(devObjPtr,
        /* array of x units !!! (the units are not set as the first units only , but can be spread)*/
        devMemInfoPtr->unitMemArr,
        SMEM_CHT_NUM_UNITS_MAX_CNS);

    pipeTableInfoSetPart2(devObjPtr);


    /*
        bind 'non PP' per unit active memory to the unit.
        the memory spaces are : 'PEX' and 'DFX' (and 'MBUS')
    */
    smemLion3BindNonPpPerUnitActiveMem(devObjPtr,devMemInfoPtr);

    if(devObjPtr->supportActiveMemPerUnit)
    {
        pipeBindPerUnitActiveMemToSpecFunTbl(devObjPtr,commonDevMemInfoPtr);
    }
}

/**
* @internal smemPipeInit function
* @endinternal
*
* @brief   Init memory module for a PIPE device.
*/
void smemPipeInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_BOOL isPipe = GT_FALSE;

    /* state the supported features */
    SMEM_CHT_IS_SIP5_GET(devObjPtr) = 1;
    SMEM_CHT_IS_SIP5_10_GET(devObjPtr) = 1;
    SMEM_CHT_IS_SIP5_15_GET(devObjPtr) = 1;
    SMEM_CHT_IS_SIP5_16_GET(devObjPtr) = 1;
    SMEM_CHT_IS_SIP5_20_GET(devObjPtr) = 1;

    if(devObjPtr->devMemUnitNameAndIndexPtr == NULL)
    {
        buildDevUnitAddr(devObjPtr);

        isPipe = GT_TRUE;
    }

    if(devObjPtr->registersDefaultsPtr == NULL)
    {
        /* not need 'per device' default registers */
    }

    if(devObjPtr->registersDefaultsPtr_unitsDuplications == NULL)
    {
        devObjPtr->registersDefaultsPtr_unitsDuplications = pipe_duplicatedUnits;
        devObjPtr->unitsDuplicationsPtr = pipe_duplicatedUnits;
    }

    if (isPipe == GT_TRUE)
    {
        devObjPtr->devIsOwnerMemFunPtr = NULL;

        /* state 'data path' structure */
        devObjPtr->multiDataPath.supportMultiDataPath =  0;
        devObjPtr->multiDataPath.maxDp = 1;/* 1 DP units for the device */
        /* there are 2 DQs although single 'data path' */
        devObjPtr->multiDataPath.numTxqDq           = 2;
        devObjPtr->multiDataPath.txqDqNumPortsPerDp = 10;

        devObjPtr->multiDataPath.supportRelativePortNum = 0;

        devObjPtr->supportTrafficManager_notAllowed = 1;

        devObjPtr->dmaNumOfCpuPort = 16;/* 'global' port in the egress RXDMA/TXDMA units */

        devObjPtr->numOfPipes = 0;

        devObjPtr->txqNumPorts =
            devObjPtr->multiDataPath.txqDqNumPortsPerDp *
            devObjPtr->multiDataPath.numTxqDq;

        {
            GT_U32  index;
            for(index = 0 ; index < devObjPtr->multiDataPath.numTxqDq ; index ++)
            {
                devObjPtr->multiDataPath.dqInfo[index].use_egressDpIndex = 1;
                /* in PIPE : DQ[0] and DQ[1] are both mapped to TXDMA[0] */
                devObjPtr->multiDataPath.dqInfo[index].egressDpIndex = 0;
            }
        }

        {

            /* SINGLE DP supports 16 ports + 1 CPU port (index 16) */
            devObjPtr->multiDataPath.info[0].dataPathFirstPort  = 0;
            devObjPtr->multiDataPath.info[0].dataPathNumOfPorts = 17;

            devObjPtr->multiDataPath.info[0].cpuPortDmaNum = 16;

            devObjPtr->memUnitBaseAddrInfo.txqDq[0] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TXQ_DQ);
            devObjPtr->memUnitBaseAddrInfo.txqDq[1] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TXQ_DQ_1);
        }
    }

    pipeMallocMem(devObjPtr);

    /* bind the interrupts tree to the device */
    smemPipeInterruptTreeInit(devObjPtr);

    if(simulationCheck_onEmulator())
    {
        /* remove units/memories that not exists */
        onEmulator_smemPipeInit(devObjPtr);
    }

    /* init the FIRMAWARE */
    simulationPipeFirmwareInit(devObjPtr);
}

/**
* @internal smemPipeInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemPipeInit2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  ii;
    GT_U32  regAddr;

    for( ii = 0 ; ii < devObjPtr->portsNumber ; ii++)
    {
        if(!IS_CHT_VALID_PORT(devObjPtr,ii))
        {
            continue;
        }

        /* check the port registers to see what ports uses GE/XG registers */
        if(IS_CHT_HYPER_GIGA_PORT(devObjPtr,ii))
        {
            regAddr = SMEM_XCAT_XG_MAC_CONTROL3_REG(devObjPtr, ii);
            /* update the register back , using the SCIB interface so the
              'Active memory' will be called --> meaning that function
               smemXcatActiveWriteMacModeSelect(...) will be called */
            smemRegUpdateAfterRegFile(devObjPtr,regAddr,1);
        }

        devObjPtr->portsArr[ii].timestampEgressQueue[0].actualSize =
            devObjPtr->portsArr[ii].timestampEgressQueue[1].actualSize = 4;
        devObjPtr->portsArr[ii].timestampEgressQueue[0].isEmpty =
            devObjPtr->portsArr[ii].timestampEgressQueue[1].isEmpty = 1;

        devObjPtr->portsArr[ii].timestampMacEgressQueue.actualSize = 8;
        devObjPtr->portsArr[ii].timestampMacEgressQueue.isEmpty = 1;

        /* overwrite enabling */
        regAddr = SMEM_LION3_GOP_PTP_GENERAL_CTRL_REG(devObjPtr,ii);
        if(regAddr != SMAIN_NOT_VALID_CNS)
        {
            smemRegUpdateAfterRegFile(devObjPtr,regAddr,1);
        }
    }

    smemChtInit2(devObjPtr);

    /* set queue "is empty" flag */
    for (ii = 0 ; ii< SIM_MAX_PIPES_CNS; ii++)
    {
           devObjPtr->timestampQueue[SMAIN_DIRECTION_INGRESS_E][0][ii].isEmpty = 1;
           devObjPtr->timestampQueue[SMAIN_DIRECTION_INGRESS_E][1][ii].isEmpty = 1;
           devObjPtr->timestampQueue[SMAIN_DIRECTION_EGRESS_E][0][ii].isEmpty = 1;
           devObjPtr->timestampQueue[SMAIN_DIRECTION_EGRESS_E][1][ii].isEmpty = 1;
       }
}

/**
* @internal smemPipeActiveWriteFastDumpReg function
* @endinternal
*
* @brief   The application changed the value of the DFX CNC Fast dump trigger register.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
*                                      param      - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemPipeActiveWriteFastDumpReg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   writeMask,
    IN         GT_U32 * inMemPtr
)
{
    SBUF_BUF_ID bufferId;               /* Message vuffer ID */
    GT_U8  * dataPtr;                   /* Pointer to the data in the buffer */
    GT_U32 dataSize;                    /* Data size */
    GT_U32 cncUnitIndex = 0;            /* CNC unit index */
    GT_U32 pipeId = 0;                  /* pipe index */

    *memPtr = *inMemPtr;

    bufferId = sbufAlloc(devObjPtr->bufPool, 2 * sizeof(GT_U32));

    if (bufferId == NULL)
    {
        simWarningPrintf("smemPipeActiveWriteFastDumpReg: "
                            "no buffers to trigger block upload \n");
        return;
    }

    /* Get actual data pointer */
    sbufDataGet(bufferId, &dataPtr, &dataSize);

    /* Copy CNC Fast Dump Trigger Register to buffer */
    memcpy(dataPtr, memPtr, sizeof(GT_U32));

    /* Copy cncUnitIndex to buffer (not relevant for PIPE) */
    memcpy(dataPtr + sizeof(GT_U32), &cncUnitIndex, sizeof(GT_U32));

    /* Set source type of buffer */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    /* Set message type of buffer */
    bufferId->dataType = SMAIN_MSG_TYPE_CNC_FAST_DUMP_E;

    bufferId->pipeId = pipeId; /* Not relevant for PIPE */

    /* Put buffer to queue */
    squeBufPut(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));
}

/* init for 'Emulator Mode' (remove not supported units/registers) */
static void onEmulator_smemPipeInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    GT_U32  unitIndex;
    GT_U32  sizePerPort;
    static SMEM_REGISTER_DEFAULT_VALUE_STC nonExistsUnit_registersDefaultValueArr[] =
    {
         {NULL,            0,         0x00000000,      0,    0x0      }
    };
    static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC nonExistsUnit_defaults = {nonExistsUnit_registersDefaultValueArr,NULL};

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_SERDES);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    currUnitChunkPtr->numOfChunks = 0;
    currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;

    sizePerPort = sizeof(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.SERDESes.SERDES[0]);
    sizePerPort /= 4;
    /* reset all ports */
    resetRegDb((GT_U32*)&(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.SERDESes.SERDES[0]),
        sizePerPort*PIPE_NUM_SERDESES_CNS , GT_FALSE);
}

