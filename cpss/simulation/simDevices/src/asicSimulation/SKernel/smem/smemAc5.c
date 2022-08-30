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
* @file smemAc5.c
*
* @brief AC5 memory mapping implementation
*
* @version   1
********************************************************************************
*/
#include <os/simTypes.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/smem/smemAc5.h>
#include <asicSimulation/SKernel/smem/smemLion3.h>
#include <asicSimulation/SLog/simLog.h>

#define BASE_CNC_0_UNIT       0x10000000
#define BASE_PCL_TCC_0_UNIT   0x1a000000
#define END_PCL_TCC_0_UNIT    0x1aFFFFFC


#define BASE_CNC_1_UNIT             0x1F000000
#define BASE_PCL_TCC_1_UNIT         0x20000000
#define BASE_DFX_SERVER_AC5_UNIT    0x84000000
#define BASE_SERDES_AC5_UNIT        0x13000000
#define BASE_MG0_AC5_UNIT           0x7F900000
#define BASE_MPP_AC5_UNIT           0x80000000

#define BASE_MG1_AC5_UNIT           0x7FA00000
#define BASE_MG2_AC5_UNIT           0x7FB00000

/* the 'prefix address' of units that the AC5 uses on top those in AC3 */
static GT_U32   ac5_extra_usedUnitsNumbersArray[SMEM_AC5_UNIT_LAST_E - SMEM_AC5_UNIT_CNC_1_E] =
{
/*SMEM_AC5_UNIT_CNC_1_E    */    BASE_CNC_1_UNIT            >> 23,
/*SMEM_AC5_UNIT_PCL_TCC_1_E*/    BASE_PCL_TCC_1_UNIT        >> 23,
/*SMEM_AC5_UNIT_DFX_SERVER_E*/   BASE_DFX_SERVER_AC5_UNIT   >> 23,
/*SMEM_AC5_UNIT_MG0_E*/          BASE_MG0_AC5_UNIT          >> 23,
/*BASE_MPP_AC5_UNIT*/            BASE_MPP_AC5_UNIT          >> 23
};

/* Return unit chunk pointer by real unit index */
#define AC5_UNIT_CHUNK_PTR_MAC(dev_info, unitBaseAddr) \
    &dev_info->unitMemArr[unitBaseAddr>>23]

/**
* @internal smemAc5UnitCentralizedCounters_unit_1 function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the CNC unit 1
*/
static void smemAc5UnitCnc_unit_1
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC * unitChunkPtr = AC5_UNIT_CHUNK_PTR_MAC(devMemInfoPtr,BASE_CNC_1_UNIT );/*SMEM_AC5_UNIT_CNC_1_E*/
    GT_U32  index;

    /* copied CNC part from smemXCat3ActiveTable[] */
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitChunkPtr)

        /* CNC Fast Dump Trigger Register Register */
        {BASE_CNC_1_UNIT + 0x00000030, SMEM_FULL_MASK_CNS, NULL, 0 , smemCht3ActiveWriteCncFastDumpTrigger, 0},

        /* CPU direct read from the counters */
        {BASE_CNC_1_UNIT + 0x00080000, 0xFFF80000, smemCht3ActiveCncBlockRead, 0, NULL, 0},

        /* CNC Block Wraparound Status Register */
        {BASE_CNC_1_UNIT + 0x000010A0, 0xFFFFF0F0, smemCht3ActiveCncWrapAroundStatusRead, 0, NULL, 0},

        /* read interrupts cause registers CNC -- ROC register */
        {BASE_CNC_1_UNIT + 0x00000100, SMEM_FULL_MASK_CNS,
            smemChtActiveReadIntrCauseReg, 3, smemChtActiveWriteIntrCauseReg, 0},

        /* Write Interrupt Mask CNC Register */
        {BASE_CNC_1_UNIT + 0x00000104, SMEM_FULL_MASK_CNS, NULL, 0 , smemCht3ActiveWriteCncInterruptsMaskReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitChunkPtr)

    index = BASE_CNC_1_UNIT >> 23;
    devMemInfoPtr->common.specFunTbl[index].unitActiveMemPtr = unitChunkPtr->unitActiveMemPtr;


    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_CNC_1_UNIT + 0x00000000, BASE_CNC_1_UNIT + 0x00000000)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_CNC_1_UNIT + 0x00000024, BASE_CNC_1_UNIT + 0x00000024)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_CNC_1_UNIT + 0x00000030, BASE_CNC_1_UNIT + 0x00000030)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_CNC_1_UNIT + 0x00000040, BASE_CNC_1_UNIT + 0x00000044)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_CNC_1_UNIT + 0x00000100, BASE_CNC_1_UNIT + 0x00000104)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_CNC_1_UNIT + 0x00000180, BASE_CNC_1_UNIT + 0x0000018C)},

            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (BASE_CNC_1_UNIT + 0x00080000, 32768),
             SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64, 8)/*,SMEM_BIND_TABLE_MAC(cncMemory)*/}

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitChunkPtr);
    }

    /* chunks with formulas */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(BASE_CNC_1_UNIT + 0x00001080 ,0)}, FORMULA_TWO_PARAMETERS(12, 0x100, 7, 0x4)},
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(BASE_CNC_1_UNIT + 0x0000109C ,0)}, FORMULA_TWO_PARAMETERS(2, 0x100, 1, 0x4)},
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(BASE_CNC_1_UNIT + 0x000010A0 ,0)}, FORMULA_TWO_PARAMETERS(12, 0x100, 4, 0x4)},
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;
        tmpUnitChunk.chunkIndex = unitChunkPtr->chunkIndex;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitChunkPtr,&tmpUnitChunk);
    }
}

/**
* @internal smemAc5UnitIpclTcc_unit_1 function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the IPCL_TCC unit 1
*/
static void smemAc5UnitIpclTcc_unit_1
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC * unitChunkPtr = AC5_UNIT_CHUNK_PTR_MAC(devMemInfoPtr, BASE_PCL_TCC_1_UNIT);/*SMEM_AC5_UNIT_PCL_TCC_1_E*/
    GT_U32  index;


    /* copied lower TCC (PCL TCAM) part from smemXCat3ActiveTable[] */
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitChunkPtr)

        {BASE_PCL_TCC_1_UNIT + 0x00000208, SMEM_FULL_MASK_CNS, NULL, 0 , smemXCatActiveWritePolicyTcamConfig_0,  0},
        {BASE_PCL_TCC_1_UNIT + 0x00000138, SMEM_FULL_MASK_CNS, NULL, 0 , smemCht3ActiveWritePclAction, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitChunkPtr)

    index = BASE_PCL_TCC_1_UNIT >> 23;
    devMemInfoPtr->common.specFunTbl[index].unitActiveMemPtr = unitChunkPtr->unitActiveMemPtr;

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_PCL_TCC_1_UNIT + 0x00000084, BASE_PCL_TCC_1_UNIT + 0x0000009C)},

            /* Action Table and Policy TCAM Access Data */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (BASE_PCL_TCC_1_UNIT + 0x00000100, 48),
             SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4)/*,SMEM_BIND_TABLE_MAC(pclActionTcamData)*/},

            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_PCL_TCC_1_UNIT + 0x00000130, BASE_PCL_TCC_1_UNIT + 0x00000138)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_PCL_TCC_1_UNIT + 0x00000148, BASE_PCL_TCC_1_UNIT + 0x00000148)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_PCL_TCC_1_UNIT + 0x00000168, BASE_PCL_TCC_1_UNIT + 0x00000170)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_PCL_TCC_1_UNIT + 0x00000198, BASE_PCL_TCC_1_UNIT + 0x00000198)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_PCL_TCC_1_UNIT + 0x000001A4, BASE_PCL_TCC_1_UNIT + 0x000001A8)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_PCL_TCC_1_UNIT + 0x00000208, BASE_PCL_TCC_1_UNIT + 0x00000208)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_PCL_TCC_1_UNIT + 0x00000218, BASE_PCL_TCC_1_UNIT + 0x00000218)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_PCL_TCC_1_UNIT + 0x00000220, BASE_PCL_TCC_1_UNIT + 0x00000220)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_PCL_TCC_1_UNIT + 0x00000238, BASE_PCL_TCC_1_UNIT + 0x00000238)},

            /* TCAM Array Compare Enable Table */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (BASE_PCL_TCC_1_UNIT + 0x00002080, 128)},

            /* Policy TCAM Table */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (BASE_PCL_TCC_1_UNIT + 0x00040000, 196608),
             SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(52, 8)/*,SMEM_BIND_TABLE_MAC(pclTcam)*/},

            /* Policy Action Table */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (BASE_PCL_TCC_1_UNIT + 0x000B0000, 49152),
             SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16)/*,SMEM_BIND_TABLE_MAC(pclAction)*/},

            /* Policy ECC X Table */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (BASE_PCL_TCC_1_UNIT + 0x000C0000, 12288)},

            /* Policy ECC Y Table */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (BASE_PCL_TCC_1_UNIT + 0x000D0000, 12288)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitChunkPtr);
    }
}

/**
* @internal smemAc5InitFuncArray function
* @endinternal
*
* @brief   Init specific AC5 functions array - additional units that not in AC3
*
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] commonDevMemInfoPtr      - pointer to common device memory object.
*
*/
static void smemAc5InitFuncArray
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr
)
{
    GT_U32                 unitIndex;
    GT_U32                 unit;
    GT_U32                 relativeUnitIndex;
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;

    /* Bind the units to the specific unit chunk with the generic function */
    for (unit = SMEM_AC5_UNIT_CNC_1_E; unit < SMEM_AC5_UNIT_LAST_E; unit++)
    {
        relativeUnitIndex = unit - SMEM_AC5_UNIT_CNC_1_E;

        unitIndex = ac5_extra_usedUnitsNumbersArray[relativeUnitIndex];
        devMemInfoPtr->unitMemArr[unitIndex].chunkIndex = unitIndex;

        commonDevMemInfoPtr->specFunTbl[unitIndex].specFun = smemDevFindInUnitChunk;

        /* Set the parameter to be (casting of) the pointer to the unit chunk */
        commonDevMemInfoPtr->specFunTbl[unitIndex].specParam  =
            smemConvertChunkIndexToPointerAsParam(devObjPtr, unitIndex);
    }
}


/**
* @internal smemXcat5UnitDfx function
* @endinternal
*
* @brief   Allocate AC5 address type specific memories -- for the DFX unit
*
* @param[in] devObjPtr                - pointer to device memory object.
*                                      unitPtr             - pointer to the unit chunk
*                                      unitBaseAddr        - unit base address
*/
static void smemXcat5UnitDfx
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC * unitChunkPtr = AC5_UNIT_CHUNK_PTR_MAC(devMemInfoPtr, BASE_DFX_SERVER_AC5_UNIT);
    GT_U32  index;

    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitChunkPtr)

        /* SMEM_LION2_DFX_SERVER_RESET_CONTROL_REG() */
        {BASE_DFX_SERVER_AC5_UNIT + 0x000f800c, SMEM_FULL_MASK_CNS, NULL, 0 , smemLion3ActiveWriteDfxServerResetControlReg, 0},
        /* SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl4 */
        {BASE_DFX_SERVER_AC5_UNIT + 0x000f8260, SMEM_FULL_MASK_CNS, NULL, 0 , smemLion3ActiveWriteDfxServerDeviceCtrl4Reg, 0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitChunkPtr)
    }

    index = BASE_DFX_SERVER_AC5_UNIT >> 23;
    devMemInfoPtr->common.specFunTbl[index].unitActiveMemPtr = unitChunkPtr->unitActiveMemPtr;

    /* chunks with flat memory (no formulas) */
    /* chunks with flat memory (no formulas) */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(BASE_DFX_SERVER_AC5_UNIT + 0x00000000, 1015804)},/*0x000F7FFC*/
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8000, BASE_DFX_SERVER_AC5_UNIT + 0x000F8018)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8020, BASE_DFX_SERVER_AC5_UNIT + 0x000F8038)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8040, BASE_DFX_SERVER_AC5_UNIT + 0x000F8050)},
/*            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8058, BASE_DFX_SERVER_AC5_UNIT + 0x000F8058)}, */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8060, BASE_DFX_SERVER_AC5_UNIT + 0x000F806c)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8078, BASE_DFX_SERVER_AC5_UNIT + 0x000F80ac)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F80d0, BASE_DFX_SERVER_AC5_UNIT + 0x000F80dc)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8100, BASE_DFX_SERVER_AC5_UNIT + 0x000F8114)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8120, BASE_DFX_SERVER_AC5_UNIT + 0x000F813C)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8200, BASE_DFX_SERVER_AC5_UNIT + 0x000F8230)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8240, BASE_DFX_SERVER_AC5_UNIT + 0x000F8248)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8250, BASE_DFX_SERVER_AC5_UNIT + 0x000F82ac)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F82D0, BASE_DFX_SERVER_AC5_UNIT + 0x000F82D8)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F82E0, BASE_DFX_SERVER_AC5_UNIT + 0x000F82FC)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8340, BASE_DFX_SERVER_AC5_UNIT + 0x000F8354)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F83C0, BASE_DFX_SERVER_AC5_UNIT + 0x000F83DC)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8450, BASE_DFX_SERVER_AC5_UNIT + 0x000F849c)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8500, BASE_DFX_SERVER_AC5_UNIT + 0x000F851c)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8C80, BASE_DFX_SERVER_AC5_UNIT + 0x000F8C8c)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8D00, BASE_DFX_SERVER_AC5_UNIT + 0x000F8D30)},

            /* DFX memories */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F80B0, 4)},      /*eFuse_Features_Disable_Bypass*/
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F80C0, 4)},      /*eFuse_DevID_Bypass*/
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F8F00, 64)},     /*ID_efuse_Slave*/
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000F9000, 1024)},    /*HD_efuse_Slave*/
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (BASE_DFX_SERVER_AC5_UNIT + 0x000FC000, 12288)},   /*Debug_Unit*/

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitChunkPtr,&tmpUnitChunk);
    }

    devObjPtr->specialUnitsBaseAddr.UNIT_DFX_SERVER = BASE_DFX_SERVER_AC5_UNIT;
    /* the device supports the DFX but not as unique memory space */
    SMEM_CHT_IS_DFX_ON_UNIQUE_MEMORY_SPCAE(devObjPtr) = GT_FALSE;

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC registersDfxDefaultValueArr[] =
        {
            /* empty -- default registers need to come from function smemXcat5UnitDfx_additions */
             {NULL, 0, 0x00000000, 0,    0x0                        }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  element0_DfxDefault_RegistersDefaults =
            {registersDfxDefaultValueArr , NULL};

        devObjPtr->registersDfxDefaultsPtr = &element0_DfxDefault_RegistersDefaults;
    }

#define SKIP_INIT_MATRIX_DEFAULT_VALUE 0x0000011F
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,   0x000F8004,         0x02040053}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8008,         0x2FF92830}
            ,{DUMMY_NAME_PTR_CNS,   0x000F800c,         0x002020DE}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8010,         0x0000000F}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8014,         0xFFFFFFFB}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8018,         0x04000000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8020,         SKIP_INIT_MATRIX_DEFAULT_VALUE , 4 , 0x4}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8030,         SKIP_INIT_MATRIX_DEFAULT_VALUE + (1<<9)}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8034,         SKIP_INIT_MATRIX_DEFAULT_VALUE}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8038,         SKIP_INIT_MATRIX_DEFAULT_VALUE}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8040,         SKIP_INIT_MATRIX_DEFAULT_VALUE , 3 , 0x4}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8060,         SKIP_INIT_MATRIX_DEFAULT_VALUE , 4 , 0x4}
            ,{DUMMY_NAME_PTR_CNS,   0x000F807C,         0x80103E14}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8080,         0x30A88019}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8084,         0x33981000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8088,         0x08C01A09}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8090,         SKIP_INIT_MATRIX_DEFAULT_VALUE , 3 , 0x4}
            ,{DUMMY_NAME_PTR_CNS,   0x000F809C,         0xFF8000FF}
            ,{DUMMY_NAME_PTR_CNS,   0x000F80A4,         0x000001F8}
            ,{DUMMY_NAME_PTR_CNS,   0x000F80D0,         0xF0F01032}
            ,{DUMMY_NAME_PTR_CNS,   0x000F80D4,         0x00000781}
            ,{DUMMY_NAME_PTR_CNS,   0x000F80D8,         0x0184E140}
            ,{DUMMY_NAME_PTR_CNS,   0x000F80DC,         0x00030000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8120,         0x0D08007F}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8124,         0x0800806C}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8128,         0xA20081DD}
            ,{DUMMY_NAME_PTR_CNS,   0x000F812C,         0xBDE1FFFF}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8130,         0x00559564}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8248,         0x0000ffff}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8254,         0x00018604}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8258,         0x01000000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8258,         0x01000000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8260,         0x10000000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F826C,         0x00100000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8278,         0x20421500}
            ,{DUMMY_NAME_PTR_CNS,   0x000F827C,         0x04288000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8280,         0xCC000000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8284,         0x80001500}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8288,         0x00800049}
            ,{DUMMY_NAME_PTR_CNS,   0x000F828C,         0xE6410000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8290,         0x03016100}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8294,         0x00050000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8298,         0x00001000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F82A8,         0x02EB81F0}
            ,{DUMMY_NAME_PTR_CNS,   0x000F82AC,         0x00000006}
            ,{DUMMY_NAME_PTR_CNS,   0x000F82AC,         0x00000006}
            ,{DUMMY_NAME_PTR_CNS,   0x000F82D0,         0xFFFFFFFF}
            ,{DUMMY_NAME_PTR_CNS,   0x000F82FC,         0x04288000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8340,         0x00001BFC , 5 , 0x4}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8498,         0x00400000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8500,         0x0000001E}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8504,         0x00000003}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8510,         0x00800000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8514,         0x00000020}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8D00,         0x20000000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8D04,         0x3A000000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8D08,         0x00000100}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8D14,         0xffff0000}
            ,{DUMMY_NAME_PTR_CNS,   0x000F8D28,         0xffff0000}

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitChunkPtr->unitDefaultRegistersPtr = &list;
    }


}
/**
* @internal smemXCat5ActiveWriteToSd1PuPll function
* @endinternal
*
* @brief   Writing to PU PLL and PU_TX/PU_RX sets PLL ready indication
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemXCat5ActiveWriteToSd1PuPll
(
    IN  SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN  GT_U32                  address,
    IN  GT_U32                  memSize,
    IN  GT_U32                  *memPtr,
    IN  GT_UINTPTR              param,
    IN  GT_U32                  *inMemPtr
)
{
    GT_U32  regAddr;

    /* data to be written */
    *memPtr = *inMemPtr;

    /* calculate address */
    regAddr = address + 0x18; /* SERDES External Status 0 */

    smemRegFldSet(devObjPtr, regAddr, 2/* pll_ready_tx*/ ,1, (*inMemPtr & (1<<1)/*pu_pll*/) && (*inMemPtr & (1<<12)/*pu_tx*/));
    smemRegFldSet(devObjPtr, regAddr, 3/* pll_ready_rx*/ ,1, (*inMemPtr & (1<<1)/*pu_pll*/) && (*inMemPtr & (1<<11)/*pu_rx*/));

    return;
}

/**
* @internal smemXCat5ActiveWriteToSd1TxRxTraining function
* @endinternal
*
* @brief   Set Tx/Rx train complete after initiating training
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemXCat5ActiveWriteToSd1TxRxTraining
(
    IN  SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN  GT_U32                  address,
    IN  GT_U32                  memSize,
    IN  GT_U32                  *memPtr,
    IN  GT_UINTPTR              param,
    IN  GT_U32                  *inMemPtr
)
{
    GT_U32  regAddr;

    /* data to be written */
    *memPtr = *inMemPtr;

    /* calculate address */
    regAddr = address + 0x10; /* SERDES External Status 1 */

    smemRegFldSet(devObjPtr, regAddr, 2/* tx_train_complete*/ ,1, (*inMemPtr & (1<<8)) ? 1 : 0/*tx_train_enable*/);
    smemRegFldSet(devObjPtr, regAddr, 0/* rx_train_complete*/ ,1, (*inMemPtr & (1<<7)) ? 1 : 0/*rx_train_enable*/);

    return;
}

/**
* @internal smemXCat5UnitSerdes function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Serdes (SD) unit
*/
static void smemXCat5UnitSerdes
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC * unitChunkPtr = AC5_UNIT_CHUNK_PTR_MAC(devMemInfoPtr, BASE_SERDES_AC5_UNIT);
    GT_U32  index;

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitChunkPtr)

    {0x13000000, 0xFFF00FFF, NULL, 0 ,smemXCat5ActiveWriteToSd1PuPll,0},
    {0x1300000C, 0xFFF00FFF, NULL, 0 ,smemXCat5ActiveWriteToSd1TxRxTraining,0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitChunkPtr)

    index = BASE_SERDES_AC5_UNIT >> 23;
    devMemInfoPtr->common.specFunTbl[index].unitActiveMemPtr = unitChunkPtr->unitActiveMemPtr;


    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x13000000, 0x1300000C)}, FORMULA_SINGLE_PARAMETER(12, 0x2000)},
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x13000014, 0x13000038)}, FORMULA_SINGLE_PARAMETER(12, 0x2000)},
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x13000800+0x800,      4096 )}, FORMULA_SINGLE_PARAMETER(12, 0x2000)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitChunkPtr);
    }
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {    /* External SD registers */
             {DUMMY_NAME_PTR_CNS,         0x00000004,   0x00000001 , 12 , 0x2000}
            ,{DUMMY_NAME_PTR_CNS,         0x0000000c,   0x00000020 , 12 , 0x2000}
            ,{DUMMY_NAME_PTR_CNS,         0x00000014,   0x00FF0100 , 12 , 0x2000}
             /* Internal SD registers */
            ,{DUMMY_NAME_PTR_CNS,         0x00001000, 0x00005200 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001004, 0x00004a20 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001008, 0x00000020 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001010, 0x000020a1 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001014, 0x00004000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000104c, 0x00000200 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001058, 0x00000100 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001084, 0x00000303 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001088, 0x00000004 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001094, 0x00000003 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000010a0, 0x00000010 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000010ac, 0x00000001 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000010b0, 0x00001800 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000010c0, 0x00000002 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000010c4, 0x000000a0 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000010c8, 0x00000c0b , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000010cc, 0x00008000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000010d8, 0x0000a010 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001100, 0x000000f0 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001104, 0x0000ffff , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001108, 0x00007743 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001110, 0x00001919 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001118, 0x00002727 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000111c, 0x00000100 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001120, 0x0000c924 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001124, 0x00002442 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001128, 0x00005442 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000112c, 0x0000c4ed , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001130, 0x0000072d , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001134, 0x00001ba7 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001138, 0x00003c64 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001140, 0x00008060 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001144, 0x0000ff75 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001148, 0x0000001b , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000114c, 0x00000ed1 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001150, 0x0000af11 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001154, 0x00000ec8 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001158, 0x00000e00 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000115c, 0x0000056d , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001160, 0x00004800 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001168, 0x0000cec8 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000116c, 0x0000cec8 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001170, 0x00000ec8 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001174, 0x00009888 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001178, 0x00000492 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000117c, 0x00000379 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001180, 0x000000aa , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001184, 0x00002815 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001188, 0x00000038 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001194, 0x00001900 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000119c, 0x0000e000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000011c0, 0x00000249 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000011c4, 0x00001000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000011c8, 0x00000200 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000011cc, 0x00003010 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001200, 0x0000ff00 , 12 , 0x2000  ,2, 0x108}
            ,{DUMMY_NAME_PTR_CNS,         0x00001204, 0x0000ff00 , 12 , 0x2000  ,2, 0x108}
            ,{DUMMY_NAME_PTR_CNS,         0x00001208, 0x000000ff , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001210, 0x00002b6a , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001214, 0x00001240 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001218, 0x000090f3 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000121c, 0x00004888 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001220, 0x000054d4 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001224, 0x0000145e , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001240, 0x00000138 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001248, 0x00000004 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000124c, 0x0000000f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001250, 0x000005cf , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001254, 0x0000ae33 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001258, 0x000001c0 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000125c, 0x0000002a , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001260, 0x000027d4 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001264, 0x00003000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001268, 0x00003500 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001280, 0x0000000f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001284, 0x00001008 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001288, 0x00004080 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000128c, 0x00004a00 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001290, 0x00003f00 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001294, 0x00000bbc , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001298, 0x00000453 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000012c0, 0x00000227 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000012c4, 0x00002000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000012cc, 0x0000064f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000012d0, 0x00004000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000012d4, 0x00000220 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000012d8, 0x00001000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000012e0, 0x00000bea , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000012e4, 0x00000013 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001310, 0x0000000f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001314, 0x00000683 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001318, 0x000017ff , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000131c, 0x000099f0 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001324, 0x00000e66 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001328, 0x00000007 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000132c, 0x0000d6c0 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001334, 0x000015b7 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001400, 0x00000053 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001404, 0x00000041 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001408, 0x00000040 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000140c, 0x00000080 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001410, 0x000000ff , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001414, 0x00000fc4 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001418, 0x00000040 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000142c, 0x00002222 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001430, 0x00006646 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001434, 0x00008868 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001438, 0x000091a6 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001480, 0x00000060 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001484, 0x00004288 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001488, 0x00000055 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000148c, 0x0000100c , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001490, 0x00000804 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001494, 0x0000fff0 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001498, 0x0000a050 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000149c, 0x00000083 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000014a8, 0x00000050 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001500, 0x00000001 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000150c, 0x00002020 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001540, 0x00001850 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001544, 0x00000293 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000154c, 0x00002200 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001550, 0x00008000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001554, 0x00000011 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001558, 0x00000040 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000155c, 0x0000903f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001560, 0x0000901e , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001564, 0x00000025 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000156c, 0x0000fd00 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001570, 0x00006188 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001574, 0x00009408 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001578, 0x00001116 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000157c, 0x00007e00 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001584, 0x0000921f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001588, 0x0000817c , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000158c, 0x00001800 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001590, 0x00006000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001594, 0x00000002 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001598, 0x00002504 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000159c, 0x00002800 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000015a0, 0x00004c01 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000015a8, 0x00002101 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000015ac, 0x00000003 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000015b4, 0x00005000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000015c0, 0x0000c000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000015c4, 0x00006a80 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000015c8, 0x0000081f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000015cc, 0x00004900 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000015d4, 0x000003c8 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001600, 0x00001000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001604, 0x00001000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001608, 0x00001000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000160c, 0x000000ce , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001610, 0x00000f1f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001614, 0x00002f3f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001618, 0x00004f5f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000161c, 0x00006f7f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001620, 0x00000727 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001624, 0x00004767 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001628, 0x00000121 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000162c, 0x00004161 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001630, 0x00004006 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001634, 0x0000010f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001638, 0x00001000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000163c, 0x00001000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001644, 0x00001000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001648, 0x0000fee8 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000164c, 0x0000fee0 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001650, 0x00000101 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001654, 0x0000f083 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001658, 0x00000001 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000165c, 0x00000ffe , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001664, 0x00003c00 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001694, 0x00008802 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001698, 0x00006b93 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016ac, 0x00001e01 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016b0, 0x0000ffc0 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016b4, 0x0000ffc0 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016b8, 0x00008888 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016bc, 0x00008965 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016c0, 0x00008e8b , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016c4, 0x0000801c , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016c8, 0x0000a765 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016cc, 0x00001234 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016d0, 0x00000003 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016d4, 0x0000ec80 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016d8, 0x00000104 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016dc, 0x00004f4f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016e0, 0x00004f4f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016e4, 0x00004f4f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016e8, 0x00004f4f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016ec, 0x00005555 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000016f0, 0x00005555 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001700, 0x0000203f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001750, 0x00000500 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001758, 0x0000001e , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000175c, 0x000007c1 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001760, 0x0000783e , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001764, 0x000003c1 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001768, 0x00001e1e , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000176c, 0x00005055 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001770, 0x00005550 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001774, 0x00005555 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001778, 0x00005555 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000177c, 0x00005555 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001784, 0x00008180 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001788, 0x00000011 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001794, 0x0000190b , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001798, 0x000040a7 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000179c, 0x00002588 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000017a0, 0x00003086 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000017b4, 0x0000005f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000017bc, 0x00000020 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000017c4, 0x00000123 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000017c8, 0x000057af , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000017cc, 0x0000ffff , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000017dc, 0x0000259f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000017e0, 0x00004321 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000017f0, 0x00007654 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x000017f4, 0x00003210 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001904, 0x00000840 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001918, 0x00000c70 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001928, 0x00002511 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000192c, 0x00006417 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001930, 0x00000103 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001934, 0x00000013 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001940, 0x00000001 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001948, 0x00000442 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000194c, 0x0000ed6d , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001950, 0x0000002d , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001954, 0x000000ee , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001958, 0x00004800 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001960, 0x00000026 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001964, 0x00001bb7 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001968, 0x00002364 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000196c, 0x00002d00 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001970, 0x00002760 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001974, 0x00002d00 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001978, 0x00002805 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000197c, 0x00002706 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001980, 0x00001f95 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001984, 0x00002166 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001988, 0x00002508 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000198c, 0x00001fe0 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a00, 0x00000002 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a04, 0x00000cc2 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a0c, 0x00006441 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a10, 0x00006404 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a20, 0x0000ce15 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a24, 0x00002000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a28, 0x00004e15 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a2c, 0x00008000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a40, 0x0000141c , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a44, 0x00008808 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a48, 0x00000400 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a4c, 0x00000400 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a54, 0x00003bec , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a58, 0x00004c00 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a5c, 0x00000923 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a60, 0x00001400 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a64, 0x0000002c , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a68, 0x00004021 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a6c, 0x00000c00 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a70, 0x00008000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a84, 0x00004000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001a8c, 0x00000400 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001aa0, 0x00000231 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001aa4, 0x00000291 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001aa8, 0x00000231 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ab8, 0x00008000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ac0, 0x00001fc1 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ac4, 0x00000190 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ac8, 0x00000087 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ae4, 0x00001329 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ae8, 0x0000496e , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001aec, 0x00000053 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001b24, 0x0000002a , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001b28, 0x00000820 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001b2c, 0x00000820 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001b30, 0x00000820 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001b34, 0x00000820 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001b38, 0x00000820 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001b3c, 0x00000820 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001b4c, 0x00006318 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001b50, 0x00007000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ba0, 0x00000041 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001bc0, 0x00003800 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001bc4, 0x00001007 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001bc8, 0x00005e10 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001bcc, 0x00000200 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001be0, 0x00007213 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001be4, 0x00006213 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001bf0, 0x00003000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001bf8, 0x00007193 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001c00, 0x00000708 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001c14, 0x00000048 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001c20, 0x0000000c , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001c24, 0x00000304 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001c40, 0x00002238 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001c44, 0x0000cb98 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001c48, 0x00001002 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001c5c, 0x0000dcba , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001c60, 0x000000fe , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001e04, 0x00002041 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001e14, 0x00004a81 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001e18, 0x00000601 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001e1c, 0x00001f08 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001e20, 0x00000013 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001e24, 0x00000084 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001e2c, 0x00006814 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001e34, 0x000000ec , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001e84, 0x00000cf8 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001e88, 0x00000b68 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001e9c, 0x000083ed , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ea0, 0x00005041 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ea4, 0x00000201 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ea8, 0x00000962 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001eac, 0x000009e4 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001eb0, 0x00000a2d , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001eb4, 0x000007e7 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001eb8, 0x000009a1 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ebc, 0x0000001e , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ec0, 0x000002c0 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ec4, 0x00000200 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ec8, 0x00000240 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ecc, 0x00000180 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ed4, 0x00000005 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ed8, 0x00000006 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001edc, 0x00000245 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ee0, 0x00000186 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ee4, 0x00000007 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ee8, 0x000003c0 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ef0, 0x00004b4f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ef4, 0x0000058f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001ef8, 0x00000100 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f00, 0x00001000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f04, 0x00000029 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f0c, 0x00004011 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f10, 0x00000087 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f18, 0x000020c1 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f1c, 0x00001628 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f20, 0x00001000 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f24, 0x00000100 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f28, 0x00000818 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f2c, 0x0000101f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f30, 0x00001f1f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f34, 0x00000202 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f38, 0x0000fcfa , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f3c, 0x00000300 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f40, 0x0000011e , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f44, 0x00003014 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f54, 0x00000001 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f5c, 0x00001930 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f60, 0x00000100 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f64, 0x00000818 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f68, 0x0000101f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f6c, 0x0000301f , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f70, 0x00000205 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f80, 0x00000022 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f94, 0x00000001 , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f98, 0x0000ffff , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001f9c, 0x0000ffff , 12 , 0x2000  }
            ,{DUMMY_NAME_PTR_CNS,         0x00001fc0, 0x00000020 , 12 , 0x2000  }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };

        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitChunkPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemXCat5UnitMg function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the MG units
*/
static void smemXCat5UnitMg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC * unitChunkPtr;

    /* use space of MG 0 "unit" for all MG0,1,2 units */
    unitChunkPtr = AC5_UNIT_CHUNK_PTR_MAC(devMemInfoPtr, BASE_MG0_AC5_UNIT);

    devObjPtr->supportAnyAddress = GT_TRUE;

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG0_AC5_UNIT + 0x00000030, BASE_MG0_AC5_UNIT + 0x00000044)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG0_AC5_UNIT + 0x0000004C, BASE_MG0_AC5_UNIT + 0x00000084)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG0_AC5_UNIT + 0x00000090, BASE_MG0_AC5_UNIT + 0x00000100)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG0_AC5_UNIT + 0x00000150, BASE_MG0_AC5_UNIT + 0x0000017C)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG0_AC5_UNIT + 0x000003F0, BASE_MG0_AC5_UNIT + 0x000003FC)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG0_AC5_UNIT + 0x00000500, BASE_MG0_AC5_UNIT + 0x0000051C)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG0_AC5_UNIT + 0x00030000 ,BASE_MG0_AC5_UNIT + 0x00030014)}, /* XSMI */

            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG1_AC5_UNIT + 0x00000030, BASE_MG1_AC5_UNIT + 0x00000044)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG1_AC5_UNIT + 0x0000004C, BASE_MG1_AC5_UNIT + 0x00000084)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG1_AC5_UNIT + 0x00000090, BASE_MG1_AC5_UNIT + 0x00000100)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG1_AC5_UNIT + 0x00000150, BASE_MG1_AC5_UNIT + 0x0000017C)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG1_AC5_UNIT + 0x000003F0, BASE_MG1_AC5_UNIT + 0x000003FC)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG1_AC5_UNIT + 0x00000500, BASE_MG1_AC5_UNIT + 0x0000051C)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG1_AC5_UNIT + 0x00030000 ,BASE_MG1_AC5_UNIT + 0x00030014)}, /* XSMI */

            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG2_AC5_UNIT + 0x00000030, BASE_MG2_AC5_UNIT + 0x00000044)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG2_AC5_UNIT + 0x0000004C, BASE_MG2_AC5_UNIT + 0x00000084)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG2_AC5_UNIT + 0x00000090, BASE_MG2_AC5_UNIT + 0x00000100)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG2_AC5_UNIT + 0x00000150, BASE_MG2_AC5_UNIT + 0x0000017C)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG2_AC5_UNIT + 0x000003F0, BASE_MG2_AC5_UNIT + 0x000003FC)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG2_AC5_UNIT + 0x00000500, BASE_MG2_AC5_UNIT + 0x0000051C)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MG2_AC5_UNIT + 0x00030000 ,BASE_MG2_AC5_UNIT + 0x00030014)}  /* XSMI */

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitChunkPtr);
    }
}

/**
* @internal smemXCat5UnitMpp function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the MPP unit
*/
static void smemXCat5UnitMpp
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC * unitChunkPtr = AC5_UNIT_CHUNK_PTR_MAC(devMemInfoPtr, BASE_MPP_AC5_UNIT);

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (BASE_MPP_AC5_UNIT + 0x00020100, BASE_MPP_AC5_UNIT + 0x0002011C)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitChunkPtr);
    }
}

/**
* @internal smemAc5AllocSpecMemory_additions function
* @endinternal
*
* @brief   Allocate addition AC5 address type specific memories , that are not in AC3
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemAc5AllocSpecMemory_additions
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_CHT_DEV_COMMON_MEM_INFO * commonDevMemInfoPtr = &devMemInfoPtr->common;

    /* init specific functions array */
    smemAc5InitFuncArray(devObjPtr, commonDevMemInfoPtr);

    /* second CNC unit */
    smemAc5UnitCnc_unit_1(devObjPtr);
    /* second PCL TCAM unit */
    smemAc5UnitIpclTcc_unit_1(devObjPtr);

    /* DFX allocation */
    smemXcat5UnitDfx(devObjPtr);

    /* SD registers */
    smemXCat5UnitSerdes(devObjPtr);

    /* MG0-2 registers */
    smemXCat5UnitMg(devObjPtr);

    /* MPP registers */
    smemXCat5UnitMpp(devObjPtr);

    devObjPtr->cncNumOfUnits = 2;
    devObjPtr->memUnitBaseAddrInfo.CNC[0] = BASE_CNC_0_UNIT;
    devObjPtr->memUnitBaseAddrInfo.CNC[1] = BASE_CNC_1_UNIT;

    devObjPtr->tablesInfo.cncMemory.commonInfo.multiInstanceInfo.numBaseAddresses = devObjPtr->cncNumOfUnits;
    devObjPtr->tablesInfo.cncMemory.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.CNC[0];

}


static void copyRegsFixAddr(
    IN void*    srcPtr,
    IN void*    dstPtr,
    IN GT_U32   srcBaseAddr,
    IN GT_U32   dstBaseAddr,
    IN GT_U32   numRegisters
)
{
    SMEM_CHT_PP_REGS_UNIT_START_INFO_STC    *jumpPtr;
    GT_U32  *srcU32Ptr,*dstU32Ptr;
    GT_U32  new_numRegisters;
    GT_U32  ii;

    jumpPtr = (SMEM_CHT_PP_REGS_UNIT_START_INFO_STC*)srcPtr + 1;
    srcU32Ptr = (GT_U32*)(void*)jumpPtr;

    jumpPtr = (SMEM_CHT_PP_REGS_UNIT_START_INFO_STC*)dstPtr + 1;
    dstU32Ptr = (GT_U32*)(void*)jumpPtr;

    new_numRegisters = numRegisters - (sizeof(SMEM_CHT_PP_REGS_UNIT_START_INFO_STC) / sizeof(GT_U32));

    for(ii = 0 ; ii < new_numRegisters ; ii++)
    {
        if(srcU32Ptr[ii] == 0xFFFFFFFF)
        {
            continue;
        }

        dstU32Ptr[ii] = srcU32Ptr[ii] - srcBaseAddr + dstBaseAddr;
    }

    return;
}

/**
* @internal smemAc5DfxMemUpdates function
* @endinternal
*
* @brief   AC5 DFX updates to add/override AC3 registers/tables
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void smemAc5DfxMemUpdates
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_DFX_SERVER_PP_REGS_ADDR_STC    *regAddrExternalDfxPtr;
    GT_U32  *U32Ptr;
    GT_U32  numRegisters;
    GT_U32  ii;

    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl24 = 0x000F8D00;/*0x840F8D00;*/

    regAddrExternalDfxPtr = SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr);

    /* align the registers to proper unit base address */

    U32Ptr = (GT_U32*)(void*)regAddrExternalDfxPtr;

    numRegisters = sizeof(SMEM_DFX_SERVER_PP_REGS_ADDR_STC) / sizeof(GT_U32);

    for(ii = 0 ; ii < numRegisters ; ii++)
    {
        if(U32Ptr[ii] == 0xFFFFFFFF)
        {
            continue;
        }

        U32Ptr[ii] += BASE_DFX_SERVER_AC5_UNIT;
    }

}

/**
* @internal smemAc5MemUpdates function
* @endinternal
*
* @brief   AC5 updates to add/override AC3 registers/tables
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemAc5MemUpdates
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_PP_REGS_ADDR_STC        * regAddrDbPtr;

    regAddrDbPtr = SMEM_CHT_MAC_REG_DB_GET(devObjPtr);

    regAddrDbPtr->centralizedCntrs[0].globalRegs.CNCMetalFixRegister = 0x10000180;


    /* copy registers and set proper base addr */
    copyRegsFixAddr(&regAddrDbPtr->centralizedCntrs[0],
                    &regAddrDbPtr->centralizedCntrs[1],
                    BASE_CNC_0_UNIT,
                    BASE_CNC_1_UNIT,
                    sizeof(regAddrDbPtr->centralizedCntrs[0])/sizeof(GT_U32));

    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,centralizedCntrs[1]   ,BASE_CNC_1_UNIT);


    /* copy registers and set proper base addr */
    copyRegsFixAddr(&regAddrDbPtr->TCCLowerIPCL[0],
                    &regAddrDbPtr->TCCLowerIPCL[1],
                    BASE_PCL_TCC_0_UNIT,
                    BASE_PCL_TCC_1_UNIT,
                    sizeof(regAddrDbPtr->TCCLowerIPCL[0])/sizeof(GT_U32));

    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,TCCLowerIPCL[1]   ,BASE_PCL_TCC_1_UNIT);

    /* add/update the DFX */
    smemAc5DfxMemUpdates(devObjPtr);
}

/**
* @internal smemAc5PclTcamClientContextSet function
* @endinternal
*
* @brief   AC5 : state that the PCL-TCAM client (IPCL 0/1/2 or EPCL) is starting
            it's access . this needed for memory access to 'SMEM_XCAT3_UNIT_TCC_LOWER_E'
*           address to access proper pcl_tcam unit (can be changed to SMEM_AC5_UNIT_PCL_TCC_1_E)
* @param[in] devObjPtr                - pointer to device object.
* @param[in] tcamClient              -  one of: IPCL 0/1/2 / EPCL , LAST last means 'out of context'
*
* return - None
*
* @note function MUST be called before the client start to access the TCAM UNIT
*       memory/registers
*
*/
void smemAc5PclTcamClientContextSet(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SIP5_TCAM_CLIENT_ENT    tcamClient/* one of: IPCL 0/1/2 / EPCL , LAST
                                            last means 'out of context' */
)
{
    DECLARE_FUNC_NAME(smemAc5PclTcamClientContextSet);
    GT_U32  bitIndex;
    GT_U32  value;
    char*   name;

    switch(tcamClient)
    {
        case SIP5_TCAM_CLIENT_IPCL0_E :
            bitIndex = 10 ;
            name = "SIP5_TCAM_CLIENT_IPCL0_E";
            break;
        case SIP5_TCAM_CLIENT_IPCL1_E :
            bitIndex = 11 ;
            name = "SIP5_TCAM_CLIENT_IPCL1_E";
            break;
        case SIP5_TCAM_CLIENT_IPCL2_E :
            bitIndex = 12 ;
            name = "SIP5_TCAM_CLIENT_IPCL2_E";
            break;
        case SIP5_TCAM_CLIENT_EPCL_E  :
            bitIndex = 13 ;
            name = "SIP5_TCAM_CLIENT_EPCL_E" ;
            break;
        case SIP5_TCAM_CLIENT_LAST_E  :
            __LOG(("State to the 'Memory Engine' to 'reset' PCL-TCAM context (since we out of client) \n"));
            smemSetCurrentTaskExtParamValue(devObjPtr,TASK_EXT_PARAM_INDEX_AC5_TCAM,0/* reset the context to default */);
            return;

        default:
            skernelFatalError("smemAc5PclTcamClientContextSet: not supported TCAM client [%d]\n",tcamClient);
            return;
    }

    smemDfxRegFldGet(devObjPtr,
        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl24,
        bitIndex, 1, &value);

    __LOG(("State to the 'Memory Engine' to access the PCL-TCAM[%d] for client[%s]\n",
        value,
        name));

    /* state to the memory engine to access the TCAM[0] or to TCAM[1] */
    smemSetCurrentTaskExtParamValue(devObjPtr,TASK_EXT_PARAM_INDEX_AC5_TCAM,value);

    return;
}

/**
* @internal smemAc5PclTcamByAddrContextSet function
* @endinternal
*
* @brief   AC5 : state that the PCL-TCAM is accessed from the CPU by address in one of the TCAM[0/1]
*           this needed for memory access to 'SMEM_XCAT3_UNIT_TCC_LOWER_E'
*           address to access proper pcl_tcam unit (can be changed to SMEM_AC5_UNIT_PCL_TCC_1_E)
* @param[in] devObjPtr            - pointer to device object.
* @param[in] address              - address in TCAM[0] or in TCAM[1]
*
* return - None
*
* @note function MUST be called before the client start to access the TCAM UNIT
*       memory/registers
*       function is called from the 'active memory'
*
*       NOTE: to 'end' the context , call smemAc5PclTcamClientContextSet with  SIP5_TCAM_CLIENT_LAST_E
*/
void smemAc5PclTcamByAddrContextSet(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32    address
)
{
    GT_U32  value;
    if(address >= BASE_PCL_TCC_0_UNIT &&
       address <= END_PCL_TCC_0_UNIT)
    {
        value = 0;
    }
    else
    {
        value = 1;
    }

    smemSetCurrentTaskExtParamValue(devObjPtr,TASK_EXT_PARAM_INDEX_AC5_TCAM,value);
}


/**
* @internal ac5ConvertDevAndAddrToNewDevAndAddr function
* @endinternal
*
* @brief   AC5 : Convert (dev,address) to new (dev,address).
*           needed for memory access to 'SMEM_XCAT3_UNIT_TCC_LOWER_E' address to access
*           proper pcl_tcam unit (can be changed to SMEM_AC5_UNIT_PCL_TCC_1_E)
* @param[in] devObjPtr                - pointer to device object.
* @param[in] address                  -  of memory(register or table).
* @param[in] accessType               - the access type
*                                       None
*
* @note function MUST be called before calling smemFindMemory()
*
*/
static void ac5ConvertDevAndAddrToNewDevAndAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  address,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    OUT SKERNEL_DEVICE_OBJECT * *newDevObjPtrPtr,
    OUT GT_U32                  *newAddressPtr
)
{
    *newDevObjPtrPtr = devObjPtr;

    *newAddressPtr = address;
    if(0 == IS_SKERNEL_OPERATION_MAC(accessType))
    {
        /* the CPU access 'pipe 0/1' explicitly */
        /* so no address modifications */
        return;
    }

    if(address >= BASE_PCL_TCC_0_UNIT &&
       address <= END_PCL_TCC_0_UNIT)
    {
        if(smemGetCurrentTaskExtParamValue(devObjPtr,TASK_EXT_PARAM_INDEX_AC5_TCAM))
        {
            /* the 'address' is of TCC0 but we need TCC1 */
            *newAddressPtr = (address - BASE_PCL_TCC_0_UNIT) + BASE_PCL_TCC_1_UNIT;
        }
    }

    return;
}

/**
* @internal ac5SupportMultiTcamRecognition function
* @endinternal
*
* @brief   prepare multi PCL-TCAM units recognition
*
* @param[in] devObjPtr                - pointer to device object of pipe 0
*/
static void ac5SupportMultiTcamRecognition
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr;

    /* NOTE: the function ac5ConvertDevAndAddrToNewDevAndAddr will help
       any memory access to 'SMEM_XCAT3_UNIT_TCC_LOWER_E' address to access
       proper pcl_tcam unit (can be changed to SMEM_AC5_UNIT_PCL_TCC_1_E) */
    commonDevMemInfoPtr = devObjPtr->deviceMemory;
    commonDevMemInfoPtr->smemConvertDevAndAddrToNewDevAndAddrFunc =
        ac5ConvertDevAndAddrToNewDevAndAddr;
}

enum{
    FuncUnitsInterrupts_CNCInterruptSummaryCauseReg = 1,
    FuncUnitsInterrupts_eqInterruptSummary = 3,
    FuncUnitsInterrupts_bridgeInterruptCause = 4,
    FuncUnitsInterrupts_policerInterruptCause_2 = 6, /*EPLR*/
    FuncUnitsInterrupts_FDBInterruptCauseReg = 7,
    FuncUnitsInterrupts_TTIEngineInterruptCause = 8,
    FuncUnitsInterrupts_policerInterruptCause_0 = 11, /*IPLR0*/
    FuncUnitsInterrupts_policerInterruptCause_1 = 12, /*IPLR1*/
    FuncUnitsInterrupts_txQInterruptSummary = 14,
    FuncUnitsInterrupts_CNC1InterruptSummaryCauseReg = 15
};

enum{  /*cnm_grp_0_4_intr  - dfx1InterruptsSummaryCause */
    cci_errirq                                                          = 19,
    adec_amb2_int_sum                                                   = 20,
    adec_amb3_int_sum                                                   = 21,
    adec_cs_trace_int_sum                                               = 22,
    adec_inic_int_sum                                                   = 23,
    adec_pcie_int_sum                                                   = 24,
    adec_xor0_int_sum                                                   = 25,
    adec_xor1_int_sum                                                   = 26,
    adec_gic_int_sum                                                    = 27,
    adec_cpu_int_sum                                                    = 28,
    ext_mg_int_out0                                                     = 29,
    ext_mg_int_out1                                                     = 30,
    ext_mg_int_out2                                                     = 31
};

static SKERNEL_INTERRUPT_REG_INFO_STC  ac5_mg0_globalInterruptsSummary =
{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.globalAc5CnmInterruptCause)},/*SMEM_CHT_GLB_INT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.globalAc5CnmInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/0 ,
        /*interruptPtr*/NULL /* no father for me , thank you */
        },
    /*isTriggeredByWrite*/ 1
};

static SKERNEL_INTERRUPT_REG_INFO_STC  ac5_mg0_dfx1InterruptsSummary =
{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.dfx1Ac5CnmInterruptsSummaryCause)},/*SMEM_CHT_GLB_INT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.dfx1Ac5CnmInterruptsSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/dfx1IntSum ,
        /*interruptPtr*/&ac5_mg0_globalInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
};

SKERNEL_INTERRUPT_REG_INFO_STC ac5_GlobalInterruptsSummary =
{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.globalInterruptCause)},/*SMEM_CHT_GLB_INT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.globalInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/ext_mg_int_out0,
        /*interruptPtr*/&ac5_mg0_dfx1InterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
};

SKERNEL_INTERRUPT_REG_INFO_STC ac5_FunctionalInterruptsSummary =
{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.functionalInterruptsSummaryCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.functionalInterruptsSummaryMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/FuncUnitsIntsSum,
            /*interruptPtr*/&ac5_GlobalInterruptsSummary
            },
        /*isTriggeredByWrite*/ 1
};

/* node in MG that aggregates ports 0..30 */
SKERNEL_INTERRUPT_REG_INFO_STC  ac5_portsInterruptsSummary =
{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.stackPortsInterruptCause)},/*SMEM_CHT3_XG_PORTS_INTERRUPT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.stackPortsInterruptMask)},/*SMEM_CHT3_XG_PORTS_INTERRUPT_MASK_REG*/
    /*myFatherInfo*/{
        /*myBitIndex*/PortsIntSum ,
        /*interruptPtr*/&ac5_GlobalInterruptsSummary
        },
    /*isTriggeredByWrite*/ 1
};

static SKERNEL_INTERRUPT_REG_INFO_STC  eqInterruptSummary =
{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(EQ.preEgrInterrupt.preEgrInterruptSummary)}, /*SMEM_CHT_EQ_INT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(EQ.preEgrInterrupt.preEgrInterruptSummaryMask)},  /*SMEM_CHT_EQ_INT_MASK_REG*/
    /*myFatherInfo*/{
        /*myBitIndex*/FuncUnitsInterrupts_eqInterruptSummary ,
        /*interruptPtr*/&ac5_FunctionalInterruptsSummary
        }
};

static SKERNEL_INTERRUPT_REG_INFO_STC  txQInterruptSummary =
{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(egrAndTxqVer0.transmitQueueInterrupt.transmitQueueInterruptSummaryCause)},/*SMEM_CHT_TX_QUEUE_INTERRUPT_CAUSE_REG , SMEM_LION_TXQ_DQ_INTR_SUM_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(egrAndTxqVer0.transmitQueueInterrupt.transmitQueueInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/FuncUnitsInterrupts_txQInterruptSummary ,
        /*interruptPtr*/&ac5_FunctionalInterruptsSummary
        }
};

static SKERNEL_INTERRUPT_REG_INFO_STC  dataPathInterruptsSummary =
{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.SERInterruptsSummary)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.SERInterruptsMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/DataPathIntSum ,
        /*interruptPtr*/&ac5_GlobalInterruptsSummary
        }
};

static SKERNEL_INTERRUPT_REG_INFO_STC  mibCountersInterruptsSummary =
{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(LMS.LMS0.MIBCntrsInterruptSummary)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(LMS.LMS0LMS0Group0.portsMIBCntrsInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/1 ,
        /*interruptPtr*/&dataPathInterruptsSummary
        }
};

static SKERNEL_INTERRUPT_REG_INFO_STC  lmsPortsInterruptsSummary =
{
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(LMS.LMS1.portsInterruptSummary)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(LMS.LMS0LMS0Group0.portsInterruptSummaryMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/5 ,
        /*interruptPtr*/&mibCountersInterruptsSummary
        }
};

/* Gige port summary */
#define  GIGE_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(port,bitInReg,regIn) \
static SKERNEL_INTERRUPT_REG_INFO_STC  gigaInterruptSummary_##port =                                            \
{                                                                                                               \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(GOP.ports.gigPort[port].portInterruptSummaryCause)},    \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(GOP.ports.gigPort[port].portInterruptSummaryMask)},     \
    /*myFatherInfo*/{                                                                                           \
        /*myBitIndex*/bitInReg ,                                                                                \
        /*interruptPtr*/&regIn                                                                                  \
        },                                                                                                      \
}

/* Gige port summary directed LMS ports summary */
#define GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(port) \
        GIGE_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(port,(port+1),lmsPortsInterruptsSummary)

/* Gige port summary directed from XLG summary */
#define GIGE_SUMMARY_PORT_IN_XLG_INTERRUPT_SUMMARY_MAC(port) \
        GIGE_SUMMARY_PORT_INTERRUPT_SUMMARY_MAC(port,2,xlgInterruptSummary_##port)

/* Gige mac port leaf */
#define  GIGE_MAC_INTERRUPT_CAUSE_MAC(port) \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(GOP.ports.gigPort[port].portInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(GOP.ports.gigPort[port].portInterruptMask)},  /**/  \
    /*myFatherInfo*/{                                     \
        /*myBitIndex*/1 ,                                 \
        /*interruptPtr*/&gigaInterruptSummary_##port      \
        }                                                 \
}

/* XLG port summary directed from MG ports summary */
#define  XLG_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(port,bitInMg,regInMg) \
static SKERNEL_INTERRUPT_REG_INFO_STC  xlgInterruptSummary_##port =  \
{                                                  \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(GOP.ports.XLGIP[port].externalUnitsInterruptsCause)}, /*SMEM_LION_XG_PORTS_SUMMARY_INTERRUPT_CAUSE_REG*/  \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(GOP.ports.XLGIP[port].externalUnitsInterruptsMask)},  /*SMEM_LION2_XG_PORTS_SUMMARY_INTERRUPT_MASK_REG*/  \
    /*myFatherInfo*/{                              \
        /*myBitIndex*/ bitInMg ,                   \
        /*interruptPtr*/&regInMg                   \
        },                                         \
}

/* XLG mac ports leaf */
#define  XLG_MAC_INTERRUPT_CAUSE_MAC(port) \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(GOP.ports.XLGIP[port].portInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(GOP.ports.XLGIP[port].portInterruptMask)},  /**/  \
    /*myFatherInfo*/{                                     \
        /*myBitIndex*/1 ,                                 \
        /*interruptPtr*/&xlgInterruptSummary_##port      \
        }                                                 \
}

GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(0);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(1);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(2);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(3);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(4);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(5);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(6);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(7);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(8);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(9);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(10);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(11);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(12);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(13);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(14);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(15);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(16);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(17);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(18);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(19);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(20);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(21);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(22);
GIGE_SUMMARY_PORT_IN_LMS_INTERRUPT_SUMMARY_MAC(23);

/* CPU port */
XLG_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(31,25,lmsPortsInterruptsSummary);
GIGE_SUMMARY_PORT_IN_XLG_INTERRUPT_SUMMARY_MAC(31);

XLG_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(24,25,ac5_portsInterruptsSummary);
XLG_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(25,26,ac5_portsInterruptsSummary);
XLG_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(26,27,ac5_portsInterruptsSummary);
XLG_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(27,28,ac5_portsInterruptsSummary);
XLG_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(28,26,ac5_portsInterruptsSummary);
XLG_SUMMARY_PORT_IN_MG_INTERRUPT_SUMMARY_MAC(29,28,ac5_portsInterruptsSummary);

GIGE_SUMMARY_PORT_IN_XLG_INTERRUPT_SUMMARY_MAC(24);
GIGE_SUMMARY_PORT_IN_XLG_INTERRUPT_SUMMARY_MAC(25);
GIGE_SUMMARY_PORT_IN_XLG_INTERRUPT_SUMMARY_MAC(26);
GIGE_SUMMARY_PORT_IN_XLG_INTERRUPT_SUMMARY_MAC(27);
GIGE_SUMMARY_PORT_IN_XLG_INTERRUPT_SUMMARY_MAC(28);
GIGE_SUMMARY_PORT_IN_XLG_INTERRUPT_SUMMARY_MAC(29);

/* define the 'leafs' of the interrupts of Ac5 device */
static SKERNEL_INTERRUPT_REG_INFO_STC ac5InterruptsTreeDb[] = {
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.SDMARegs.receiveSDMAInterruptCause)}, /*SMEM_CHT_SDMA_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.SDMARegs.receiveSDMAInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/RxSDMASum ,
            /*interruptPtr*/&ac5_GlobalInterruptsSummary
            }
    }
    ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.SDMARegs.transmitSDMAInterruptCause)},/*SMEM_CHT_TX_SDMA_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.SDMARegs.transmitSDMAInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/TxSDMASum ,
            /*interruptPtr*/&ac5_GlobalInterruptsSummary
            }
    }
    ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(TTI.TTIEngineInterruptCause)},/*SMEM_LION3_TTI_INTERRUPT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(TTI.TTIEngineInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/FuncUnitsInterrupts_TTIEngineInterruptCause ,
            /*interruptPtr*/&ac5_FunctionalInterruptsSummary
            }
    }
    ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(L2I.bridgeEngineInterrupts.bridgeInterruptCause)},/*SMEM_CHT_BRIDGE_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(L2I.bridgeEngineInterrupts.bridgeInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/FuncUnitsInterrupts_bridgeInterruptCause ,
            /*interruptPtr*/&ac5_FunctionalInterruptsSummary
            }
    }
    ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(FDB.FDBInterrupt.FDBInterruptCauseReg)},/*SMEM_CHT_MAC_TBL_INTR_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(FDB.FDBInterrupt.FDBInterruptMaskReg)},
        /*myFatherInfo*/{
            /*myBitIndex*/FuncUnitsInterrupts_FDBInterruptCauseReg ,
            /*interruptPtr*/&ac5_FunctionalInterruptsSummary
            }
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(centralizedCntrs[0].globalRegs.CNCInterruptSummaryCauseReg)},/*SMEM_CHT3_CNC_INTR_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(centralizedCntrs[0].globalRegs.CNCInterruptSummaryMaskReg)},
        /*myFatherInfo*/{
            /*myBitIndex*/FuncUnitsInterrupts_CNCInterruptSummaryCauseReg ,
            /*interruptPtr*/&ac5_FunctionalInterruptsSummary
            }
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(centralizedCntrs[1].globalRegs.CNCInterruptSummaryCauseReg)},/*SMEM_CHT3_CNC_INTR_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(centralizedCntrs[1].globalRegs.CNCInterruptSummaryMaskReg)},
        /*myFatherInfo*/{
            /*myBitIndex*/FuncUnitsInterrupts_CNC1InterruptSummaryCauseReg ,
            /*interruptPtr*/&ac5_FunctionalInterruptsSummary
            }
    }
    ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(PLR[0].policerInterruptCause)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(PLR[0].policerInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/FuncUnitsInterrupts_policerInterruptCause_0 ,
            /*interruptPtr*/&ac5_FunctionalInterruptsSummary
            }
    }
    ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(PLR[1].policerInterruptCause)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(PLR[1].policerInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/FuncUnitsInterrupts_policerInterruptCause_1 ,
            /*interruptPtr*/&ac5_FunctionalInterruptsSummary
            }
    }
    ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(PLR[2].policerInterruptCause)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(PLR[2].policerInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/FuncUnitsInterrupts_policerInterruptCause_2 ,
            /*interruptPtr*/&ac5_FunctionalInterruptsSummary
            }
    }
    ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.miscellaneousInterruptCause)},/*SMEM_CHT_MISC_INTR_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt.miscellaneousInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/MgInternalIntSum ,
            /*interruptPtr*/&ac5_GlobalInterruptsSummary
            }
    }
    ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(EQ.ingrSTCInterrupt.ingrSTCInterruptCause)}, /*SMEM_CHT_INGRESS_STC_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(EQ.ingrSTCInterrupt.ingrSTCInterruptMask)},  /*SMEM_CHT_INGRESS_STC_INT_MASK_REG*/
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&eqInterruptSummary
            }
    }
    ,{                                                         \
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(egrAndTxqVer0.transmitQueueInterrupt.egrSTCInterruptCause)}, /*SMEM_CHT_STC_INT_CAUSE_REG , SMEM_LION_TXQ_EGR_STC_INTR_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(egrAndTxqVer0.transmitQueueInterrupt.egrSTCInterruptMask)},  /*SMEM_CHT_STC_INT_MASK_REG , SMEM_LION_TXQ_EGR_STC_INTR_MASK_REG*/
        /*myFatherInfo*/{
        /*myBitIndex*/5 ,
        /*interruptPtr*/&txQInterruptSummary
        }
    }

    /* the Gige ports 0..23 */
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(0 )
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(1 )
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(2 )
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(3 )
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(4 )
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(5 )
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(6 )
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(7 )
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(8 )
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(9 )
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(10)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(11)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(12)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(13)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(14)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(15)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(16)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(17)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(18)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(19)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(20)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(21)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(22)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(23)

    /* CPU port*/
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(31)

    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(24)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(25)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(26)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(27)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(28)
    ,GIGE_MAC_INTERRUPT_CAUSE_MAC(29)

    /* the XLG ports 24..27 */
    ,XLG_MAC_INTERRUPT_CAUSE_MAC (24)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC (25)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC (26)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC (27)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC (28)
    ,XLG_MAC_INTERRUPT_CAUSE_MAC (29)

    ,SKERNEL_INTERRUPT_REG_INFO__LAST_LINE__CNS /* must be last */
};

/**
* @internal smemAc5Init function
* @endinternal
*
* @brief   Init memory module for the AC5 device.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemAc5Init
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_PP_REGS_ADDR_STC        * regAddrDbPtr;

    /* call the 'common' AC3 init */
    smemXCat3Init(devObjPtr);

    ac5SupportMultiTcamRecognition(devObjPtr);

    regAddrDbPtr = SMEM_CHT_MAC_REG_DB_GET(devObjPtr);

    /* Initialize AC5 specific interrupt cause registers */
    regAddrDbPtr->MG.globalInterrupt.globalAc5CnmInterruptCause = 0x7F900030;
    regAddrDbPtr->MG.globalInterrupt.globalAc5CnmInterruptMask = 0x7F900034;
    regAddrDbPtr->MG.globalInterrupt.dfx1Ac5CnmInterruptsSummaryCause = 0x7F9000B8;
    regAddrDbPtr->MG.globalInterrupt.dfx1Ac5CnmInterruptsSummaryMask = 0x7F9000BC;

    devObjPtr->myInterruptsDbPtr = ac5InterruptsTreeDb;
    /* 2. need to support CNC upload to MG */


}

/**
* @internal smemAc5Init2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr             - pointer to device object.
*/
void smemAc5Init2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /* call the 'common' AC3 init */
    smemXCat3Init2(devObjPtr);
}

