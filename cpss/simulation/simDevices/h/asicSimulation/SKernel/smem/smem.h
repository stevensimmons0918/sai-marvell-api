/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* smem.h
*
* DESCRIPTION:
*       This is a external API definition for SMem module of SKernel.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 39 $
*
*******************************************************************************/
#ifndef __smemh
#define __smemh

#include <os/simTypesBind.h>
#include <asicSimulation/SCIB/scib.h>
#include <asicSimulation/SKernel/smain/smain.h>

/* macro to get name of array and the number of elements */
#define ARRAY_NAME_AND_NUM_ELEMENTS_MAC(arrayName)  arrayName , (sizeof(arrayName) / sizeof(arrayName[0]))

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Global constants definition */
/* use this must be only for device with :    devObjPtr->support_memoryRanges == 0 */
#define SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS               23
#define SMEM_PHY_UNIT_INDEX_CNS                         30

/* Memory access for all types of read (SCIB and SKERNEL) */
#define SMEM_ACCESS_READ_FULL_MAC(type) \
    (IS_WRITE_OPERATION_MAC(type) == 0)

/* Memory access for PCI read/write (SCIB and SKERNEL) */
#define SMEM_ACCESS_PCI_FULL_MAC(type) \
    IS_PCI_OPERATION_MAC(type)

/* Memory access for DFX read/write (SCIB and SKERNEL) */
#define SMEM_ACCESS_DFX_FULL_MAC(type) \
    IS_DFX_OPERATION_MAC(type)

/* Memory access for all types of read/write (SKERNEL) */
#define SMEM_ACCESS_SKERNEL_FULL_MAC(type) \
    IS_SKERNEL_OPERATION_MAC(type)

#define STRING_FOR_UNIT_NAME(unitName)            \
    string_##unitName

#define BUILD_STRING_FOR_UNIT_NAME(unitName)             \
    static GT_CHAR *string_##unitName = STR(unitName)

/* Define the different FDB memory sizes    */
#define SMEM_MAC_TABLE_SIZE_4KB            (0x1000)
#define SMEM_MAC_TABLE_SIZE_8KB            (0x2000)
#define SMEM_MAC_TABLE_SIZE_16KB           (0x4000)
#define SMEM_MAC_TABLE_SIZE_32KB           (0x8000)
#define SMEM_MAC_TABLE_SIZE_64KB           (0x10000)
#define SMEM_MAC_TABLE_SIZE_128KB          (0x20000)
#define SMEM_MAC_TABLE_SIZE_256KB          (0x40000)
#define SMEM_MAC_TABLE_SIZE_384KB          (0x60000)
#define SMEM_MAC_TABLE_SIZE_512KB          (0x80000)

/*******************************************************************************
*  DECLARE_UNIT_MEM_FUNCTION_MAC
*
* DESCRIPTION:
*      find the address in the unit's registers / tables and return the memory
*       pointer.
*
* INPUTS:
*       devObjPtr - device object PTR.
*       address     - Address for ASIC memory.
*       memPtr      - Pointer to the register's memory in the simulation.
*       param       - Registers' specific parameter.
*       inMemPtr    - Pointer to the memory to get register's content.
*
* OUTPUTS:
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*******************************************************************************/
#define DECLARE_UNIT_MEM_FUNCTION_MAC(funcName)  \
    GT_U32 * funcName                            \
    (                                            \
        IN SKERNEL_DEVICE_OBJECT * devObjPtr,    \
        IN SCIB_MEMORY_ACCESS_TYPE accessType,   \
        IN GT_U32 address,                       \
        IN GT_U32 memSize,                       \
        IN GT_UINTPTR param                      \
    )

/*******************************************************************************
*  ACTIVE_WRITE_FUNC_PROTOTYPE_MAC
*
* DESCRIPTION:
*      active Write to register
*
* INPUTS:
*       devObjPtr   - device object PTR.
*       address     - Address for ASIC memory.
*       memSize     - size of the requested memory
*       memPtr      - Pointer to the register's memory in the simulation.
*       param       - Registers' specific parameter.
*       inMemPtr    - Pointer to the memory to get register's content.
*
* OUTPUTS:
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
#define ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(funcName)        \
    void (funcName)(                                     \
        IN         SKERNEL_DEVICE_OBJECT * devObjPtr,    \
        IN         GT_U32   address,                     \
        IN         GT_U32   memSize,                     \
        IN         GT_U32 * memPtr,                      \
        IN         GT_UINTPTR param,                     \
        IN         GT_U32 * inMemPtr                     \
    )

/*******************************************************************************
*  ACTIVE_READ_FUNC_PROTOTYPE_MAC
*
* DESCRIPTION:
*      active read of register
* INPUTS:
*       devObjPtr   - device object PTR.
*       address     - address for ASIC memory.
*       memSize     - memory size to be read.
*       memPtr      - pointer to the register's memory in the simulation.
*       sumBit      - global summary interrupt bit
*
* OUTPUTS:
*       outMemPtr   - Pointer to the memory to copy register's content.
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
#define ACTIVE_READ_FUNC_PROTOTYPE_MAC(funcName)        \
    void (funcName)(                                     \
        IN         SKERNEL_DEVICE_OBJECT * devObjPtr,    \
        IN         GT_U32   address,                     \
        IN         GT_U32   memSize,                     \
        IN         GT_U32 * memPtr,                      \
        IN         GT_UINTPTR sumBit,                    \
        OUT        GT_U32 * outMemPtr                    \
    )

#define SMEM_ADDRESS_NOT_IN_FORMULA_CNS 0xFEFEFEFE

#define SMEM_FORMULA_CELL_NUM_CNS           6

#define END_OF_TABLE    0xFFFFFFFF

#define SMEM_FORMULA_END_CNS    {0, 0}

/* set value to target if target is zero (0) */
#define SMEM_SET_IF_NON_ZERO_MAC(target,source) \
    if((target) == 0) (target) = (source)

/* convert number of bytes to number of words */
#define CONVERT_BYTES_TO_WORDS_MAC(numBytes) (((numBytes)+3)/4)

/* convert number of bits to number of words */
#define CONVERT_BITS_TO_WORDS_MAC(numBits) (((numBits)+31)/32)

/* convert start address and end address to number of words */
#define CONVERT_START_ADDR_END_ADDR_TO_WORDS_MAC(startAddr,endAddr) \
    (((endAddr) - (startAddr))/4 + 1)

/*set SMEM_CHUNK_BASIC_STC entry , by the end address */
#define SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(startAddr,endAddr)  \
    (startAddr),  CONVERT_START_ADDR_END_ADDR_TO_WORDS_MAC(startAddr,endAddr)

/*set SMEM_CHUNK_BASIC_STC entry , by the number of bytes */
#define SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(startAddr,numBytes)\
    (startAddr),   CONVERT_BYTES_TO_WORDS_MAC(numBytes)

/* macro to build the formula section : with single parameter */
#define FORMULA_SINGLE_PARAMETER(numSteps,stepSize) {{numSteps ,stepSize}, SMEM_FORMULA_END_CNS}

/* macro to build the formula section : with 2 parameters */
#define FORMULA_TWO_PARAMETERS(numSteps1,stepSize1,numSteps2,stepSize2) {{numSteps1, stepSize1}, {numSteps2, stepSize2}, SMEM_FORMULA_END_CNS}

/* macro to build the formula section : with 3 parameters */
#define FORMULA_THREE_PARAMETERS(numSteps1,stepSize1,numSteps2,stepSize2,numSteps3,stepSize3) {{numSteps1, stepSize1}, {numSteps2, stepSize2}, {numSteps3, stepSize3}, SMEM_FORMULA_END_CNS}

/* macro to build the formula section : with 4 parameters */
#define FORMULA_FOUR_PARAMETERS(numSteps1,stepSize1,numSteps2,stepSize2,numSteps3,stepSize3,numSteps4,stepSize4) {{numSteps1, stepSize1}, {numSteps2, stepSize2}, {numSteps3, stepSize3}, {numSteps4, stepSize4}, SMEM_FORMULA_END_CNS}

/* set memory entry size and alignment: size - entry size in bits, align - entry alignment in bytes */
#define SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(sizeInBits, alignInBytes) (sizeInBits), (alignInBytes)

#define SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev,tableName,index)\
    smemTableEntryAddressGet(&(dev)->tablesInfo.tableName,index,SMAIN_NOT_VALID_CNS)

#define SMEM_TABLE_ENTRY_2_PARAMS_INDEX_GET_MAC(dev,tableName,index1,index2)\
    ((2 > (dev)->tablesInfo.tableName.commonInfo.multiInstanceInfo.numBaseAddresses) ?      \
      smemTableEntry2ParamsAddressGet(&(dev)->tablesInfo.tableName,index1,index2,SMAIN_NOT_VALID_CNS) : \
      SMEM_TABLE_ENTRY_INDEX_DUP_TBL_GET_MAC(dev,tableName,index1,index2))

#define SMEM_TABLE_ENTRY_3_PARAMS_INDEX_GET_MAC(dev,tableName,index1,index2,index3)\
    smemTableEntry3ParamsAddressGet(&(dev)->tablesInfo.tableName,index1,index2,index3,SMAIN_NOT_VALID_CNS)

#define SMEM_TABLE_ENTRY_4_PARAMS_INDEX_GET_MAC(dev,tableName,index1,index2,index3,index4)\
    smemTableEntry4ParamsAddressGet(&(dev)->tablesInfo.tableName,index1,index2,index3,index4,SMAIN_NOT_VALID_CNS)

/* table with single parameter , but in duplicated unit */
#define SMEM_TABLE_ENTRY_INDEX_DUP_TBL_GET_MAC(dev,tableName,index , instanceIndex)\
    smemTableEntryAddressGet(&(dev)->tablesInfo.tableName,index,instanceIndex)

/* table with 2 parameters , but in duplicated unit */
#define SMEM_TABLE_ENTRY_2_PARAMS_INDEX_DUP_TBL_GET_MAC(dev,tableName,index1,index2 , instanceIndex)\
    smemTableEntry2ParamsAddressGet(&(dev)->tablesInfo.tableName,index1,index2,instanceIndex)

/* table with 3 parameters , but in duplicated unit */
#define SMEM_TABLE_ENTRY_3_PARAMS_INDEX_DUP_TBL_GET_MAC(dev,tableName,index1,index2,index3 , instanceIndex)\
    smemTableEntry3ParamsAddressGet(&(dev)->tablesInfo.tableName,index1,index2,index3,instanceIndex)

/* return the offset in bytes of field from start of the structure that hold it */
#define FIELD_OFFSET_IN_STC_MAC(field,stc)  \
    ((GT_UINTPTR)(GT_U8*)(&(((stc*)NULL)->field)))

/*set next 2 fields in SMEM_CHUNK_BASIC_STC entry : tableOffsetValid, tableOffsetInBytes */
#define SMEM_BIND_TABLE_MAC(tableName)  \
    1/*tableOffsetValid*/,              \
    FIELD_OFFSET_IN_STC_MAC(tableName,SKERNEL_TABLES_INFO_STC),/*tableOffsetInBytes*/ \
    GT_FALSE/*not forbidDirectWriteToChunk*/

#define SMEM_BIND_TABLE_WITH_LIMITED_WRITE_MAC(tableName)  \
    1/*tableOffsetValid*/,              \
    FIELD_OFFSET_IN_STC_MAC(tableName,SKERNEL_TABLES_INFO_STC),/*tableOffsetInBytes*/ \
    GT_TRUE/*forbidDirectWriteToChunk*/


/* indicator for 'new unit base' */
#define SMAM_SUB_UNIT_AFTER_20_LSB_CNS  0xFFFFFFF0
/* place holder in SMEM_CHUNK_BASIC_STC for the 'new unit base' */
#define SET_SMEM_CHUNK_BASIC_STC_ENTRY_SUB_UNIT_AFTER_20_LSB_MAC(unitAddr23MSBitsAfter20MSBits) \
    unitAddr23MSBitsAfter20MSBits , SMAM_SUB_UNIT_AFTER_20_LSB_CNS


/* init table with 1 param , with array of default value for the entry (all entries the same value) */
#define SMEM_TABLE_1_PARAM_INIT_MAC(_dev,_table, _initValueArr , _numWords , _numEntries)  \
    SMEM_TABLE_ENTRIES_INIT_MAC(_dev,_table, 0 , _initValueArr , _numWords , _numEntries)


/* init table entries with array of default value (duplicate to all entries)*/
#define SMEM_TABLE_ENTRIES_INIT_MAC(_dev,_table, _firstIndex , _initValueArr , _numWords , _numEntries)  \
    {                                                                                      \
        GT_U32  *__memPtr;                                                                 \
        GT_U32  __address;                                                                 \
        GT_U32  __ii;                                                                      \
        GT_U32  __entrySize = (_numWords * (sizeof(GT_U32)));                              \
                                                                                           \
        if(_dev->tablesInfo._table.paramInfo[0].step < __entrySize)                        \
        {                                                                                  \
            skernelFatalError("SMEM_TABLE_1_PARAM_INIT_MAC: entry size oversize \n");      \
        }                                                                                  \
        else                                                                               \
        {                                                                                  \
            for(__ii = _firstIndex ; __ii < _numEntries; __ii++)                                     \
            {                                                                              \
                /* get the address */                                                      \
                __address = SMEM_TABLE_ENTRY_INDEX_GET_MAC(_dev,_table,__ii);              \
                __memPtr = smemMemGet(_dev, __address);                                    \
                /*init the entry according to needed values*/                              \
                memcpy(__memPtr,initValueArr,__entrySize);                                 \
            }                                                                              \
        }                                                                                  \
    }

/* forward declaration */
struct SMEM_CHUNK_STCT;

#define DEBUG_REG_AND_TBL_NAME

typedef struct{
#ifdef DEBUG_REG_AND_TBL_NAME
    GT_CHAR     *tableName;
#endif /*DEBUG_REG_AND_TBL_NAME*/
    GT_U32      baseAddr;
    GT_U32      memType;/*0-internal , 1 - external*/
    GT_U32      entrySize;/* in bits -- number of bits actually used */
    GT_U32      lineAddrAlign;/* in words -- number of words between 2 entries*/
    GT_U32      numOfEntries;
}DEV_TABLE_INFO_STC;

/*******************************************************************************
*   SMEM_FORMULA_CHUNK_FUNC
*
* DESCRIPTION:
*       prototype to function which return the index in the simulation memory chunk
*       that represents the accessed address
*
* INPUTS:
*        devObjPtr  - pointer to device info
*        memCunkPtr - pointer to memory chunk info
*        address    - the accesses address
* OUTPUTS:
*       None
*
* RETURNS:
*       index into memCunkPtr->memPtr[index] that represent 'address'
*       if the return value == SMEM_ADDRESS_NOT_IN_FORMULA_CNS meaning that
*       address not belong to formula
*
* COMMENTS:
*
*
*******************************************************************************/
typedef GT_U32  (*SMEM_FORMULA_CHUNK_FUNC)
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN struct SMEM_CHUNK_STCT* memCunkPtr,
    IN GT_U32                  address
);

/**
* @enum SMEM_INDEX_IN_CHUNK_TYPE_ENT
 *
 * @brief lists the options to calculate the index in the memory chunk that
 * represents an address
*/
typedef enum{

    /** @brief straight access , meaning
     *  index = (address memFirstAddr) / 4;
     */
    SMEM_INDEX_IN_CHUNK_TYPE_STRAIGHT_ACCESS_E,

    /** @brief use mask and shift , meaning
     *  index = ((address memFirstAddr) & mask) >> rightShiftNumBits;
     */
    SMEM_INDEX_IN_CHUNK_TYPE_MASK_AND_SHIFT_E,

    /** @brief use function to calculate the index
     *  index = formulaFuncPtr(devObjPtr,memCunkPtr,address);
     */
    SMEM_INDEX_IN_CHUNK_TYPE_FORMULA_FUNCTION_E,

    /** @brief ignore this entry in the memory !
     *  do not care about the index.
     */
    SMEM_INDEX_IN_CHUNK_TYPE_IGNORED_E,

} SMEM_INDEX_IN_CHUNK_TYPE_ENT;

/**
* @struct SMEM_FORMULA_DIM_STC
 *
 * @brief Describes the memory chunk formula expression
*/
typedef struct{

    /** number of registers */
    GT_U32 size;

    /** memory offset in bytes */
    GT_U32 step;

} SMEM_FORMULA_DIM_STC;

/*
 * Typedef: struct GT_MEM_CHUNK_STC
 *
 * Description:
 *      Describes the memory chunks , that represent the memories of the device
 *
 * Fields:
 *
 */
typedef struct SMEM_CHUNK_STCT{
    GT_U32  memFirstAddr;/* the first address this chunk represents */
    GT_U32  memLastAddr;/* the last address this chunk represents (this address is still in the chunk)*/
    GT_U32  *memPtr;/*pointer to dynamically allocated memory */
    GT_U32  memSize;/*size of dynamically allocated memory in words */

    GT_U32  enrtyNumWordsAlignement;    /* if not zero - memory entry alignment in words(used for direct memory write access by last word) */
    GT_U32  lastEntryWordIndex; /* Last word index in entry (used for direct memory write access by last word) */
    GT_BOOL forbidDirectWriteToChunk;/*Indication that must not write to memory , but allow only write to 'lastEntry'
            and active memory need to handle the write to the table
            needed for tables with read-only fields
             */

    GT_U32  tableOffsetInBytes;/* table offset (in bytes)-- offset from the tables memory place holder.
                           for example : the DXCH devices hold table DB of :
                           devObjPtr->tablesInfo.<table_name>
                           So the offset is from the first table : meaning from
                           (GT_UINTPTR)((GT_U32*)&(devObjPtr->tablesInfo))

                           if value == SMAIN_NOT_VALID_CNS --> not used
                           */

    /* parameters to get index in the memPtr[] */
    SMEM_INDEX_IN_CHUNK_TYPE_ENT calcIndexType;/* how to calculate the index in the memPtr[] */

    /* info for the 'Mask and shift' index calculation */
    GT_U32  memMask;/* mask to set on the address */
    GT_U32  rightShiftNumBits;/* after applying the mask , how many bits to shift to get the index */

    /* info for the 'Formula' index calculation */
    SMEM_FORMULA_CHUNK_FUNC formulaFuncPtr;/* pointer to formula to access the chunk
                                              when NULL --> straight access  */
    SMEM_FORMULA_DIM_STC formulaData[SMEM_FORMULA_CELL_NUM_CNS];   /* memory chunk access formula */
}SMEM_CHUNK_STC;

typedef enum{
     SMEM_UNIT_CHUNK_TYPE_9_MSB_E/*unit support relative address of 23 LSB*/
    ,SMEM_UNIT_CHUNK_TYPE_8_MSB_E/*unit support relative address of 24 LSB*/
    ,SMEM_UNIT_CHUNK_TYPE_23_MSB_AFTER_20_LSB_E/*unit support relative address of 20 LSB in addresses of 43bits */
    ,SMEM_UNIT_CHUNK_TYPE_FLEX_BASE_E/*unit support flex base address and flex size that not derived from 'x' MSBits as prefix */
}SMEM_UNIT_CHUNK_TYPE_ENT;



typedef struct{
    GT_U32  memFirstAddr;/* the first address this chunk represents */
    GT_U32  numOfRegisters;/* the number of registers to hold */
    GT_U32  enrtySizeBits; /* entry size in bits(used for direct memory write access by last word) */
    GT_U32  enrtyNumBytesAlignement;    /* if not zero - memory entry alignment in bytes(used for indirect memory access) */
    GT_BIT  tableOffsetValid;/* is the 'tableOffset' field valid */
    GT_U32  tableOffsetInBytes;/* table offset (in bytes)-- offset from the tables memory place holder.
                           for example : the DXCH devices hold table DB of :
                           devObjPtr->tablesInfo.<table_name>
                           So the offset is from the first table : meaning from
                           (GT_UINTPTR)((GT_U32*)&(devObjPtr->tablesInfo))
                           */
    GT_BOOL forbidDirectWriteToChunk;/*Indication that must not write to memory , but allow only write to 'lastEntry'
            and active memory need to handle the write to the table
            needed for tables with read-only fields
             */
}SMEM_CHUNK_BASIC_STC;

/**
* @struct SMEM_CHUNK_EXTENDED_STC
 *
 * @brief Describes the memory chunks, that represents the memories of the device
 * and access to it - straight and by formula
*/
typedef struct{

    SMEM_CHUNK_BASIC_STC memChunkBasic;

    SMEM_FORMULA_DIM_STC formulaCellArr[SMEM_FORMULA_CELL_NUM_CNS];

} SMEM_CHUNK_EXTENDED_STC;


/* Typedef of SMEM register */
typedef GT_U32 SMEM_REGISTER;

/* Typedef of SMEM PHY register */
typedef GT_U16 SMEM_PHY_REGISTER;

/* Typedef of wide SRAM */
typedef GT_U32 SMEM_WSRAM;

/* Typedef of narrow SRAM */
typedef GT_U32 SMEM_NSRAM;

/* Typedef of flow DRAM */
typedef GT_U32 SMEM_FDRAM;

/* Check memory bounds */
#define CHECK_MEM_BOUNDS(ptr, arraySize, index, size)                           \
    if((index + size) > (arraySize)) {                                          \
        if (skernelUserDebugInfo.disableFatalError == GT_FALSE)                 \
        {                                                                       \
            skernelFatalError("CHECK_MEM_BOUNDS: address[0x%8.8x]\
                                    index or memory size is out of range\n",address);   \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            return 0;                                                           \
        }                                                                       \
    }

/* Return the mask including "numOfBits" bits.          */
#define SMEM_BIT_MASK(numOfBits) \
   (((numOfBits) == 32) ? 0xFFFFFFFF : (~(0xFFFFFFFF << (numOfBits))))

/* Calculate the field mask for a given offset & length */
/* e.g.: BIT_MASK(8,2) = 0xFFFFFCFF                     */
#define SMEM_FIELD_MASK_NOT(offset,len)                      \
        (~(SMEM_BIT_MASK((len)) << (offset)))

/* Calculate the field mask for a given offset & length */
/* e.g.: BIT_MASK(8,2) = 0x00000300                     */
#define SMEM_FIELD_MASK(offset,len)                      \
        ( (SMEM_BIT_MASK((len)) << (offset)) )

/*
    NOTE: see also function snetFieldValueGet
*/
/* Returns the info located at the specified offset & length in data.   */
#define SMEM_U32_GET_FIELD(data,offset,length)           \
        (((data) >> (offset)) & SMEM_BIT_MASK(length))

/*
    NOTE: see also function snetFieldValueSet
*/
/* Sets the field located at the specified offset & length in data.     */
#define SMEM_U32_SET_FIELD(data,offset,length,val)           \
(data) = (((data) & SMEM_FIELD_MASK_NOT((offset),(length))) | \
                  (((val) & SMEM_BIT_MASK(length)) << (offset)))

/* all bits mask */
#define SMEM_FULL_MASK_CNS 0xFFFFFFFF

/* macro to determine if a read/write need to be done in the Skernel level */
#define SMEM_SKIP_LOCAL_READ_WRITE \
    ((sasicgSimulationRoleIsDevices == GT_TRUE || \
      sasicgSimulationRole == SASICG_SIMULATION_ROLE_NON_DISTRIBUTED_E) ? 0 : 1)

/*******************************************************************************
*  SMEM_ACTIVE_MEM_READ_FUN
*
* DESCRIPTION:
*      Definition of the Active register read function.
* INPUTS:
*       deviceObjPtr - device object PTR.
*       address     - Address for ASIC memory.
*       memPtr      - Pointer to the register's memory in the simulation.
*       param       - Registers' specific parameter.
*
* OUTPUTS:
*       outMemPtr   - Pointer to the memory to copy register's content.
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
typedef void (* SMEM_ACTIVE_MEM_READ_FUN ) (
                              IN         SKERNEL_DEVICE_OBJECT * deviceObjPtr,
                              IN         GT_U32   address,
                              IN         GT_U32   memSize,
                              IN         GT_U32 * memPtr,
                              IN         GT_UINTPTR param,
                              OUT        GT_U32 * outMemPtr);
/*******************************************************************************
*  SMEM_ACTIVE_MEM_WRITE_FUN
*
* DESCRIPTION:
*      Definition of the Active register write function.
* INPUTS:
*       deviceObjPtr - device object PTR.
*       address     - Address for ASIC memory.
*       memPtr      - Pointer to the register's memory in the simulation.
*       param       - Registers' specific parameter.
*       inMemPtr    - Pointer to the memory to get register's content.
*
* OUTPUTS:
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
typedef void (* SMEM_ACTIVE_MEM_WRITE_FUN ) (
                              IN         SKERNEL_DEVICE_OBJECT * deviceObjPtr,
                              IN         GT_U32   address,
                              IN         GT_U32   memSize,
                              IN         GT_U32 * memPtr,
                              IN         GT_UINTPTR param,
                              IN         GT_U32 * inMemPtr);

/*
 * Typedef: struct SMEM_EXT_ACTIVE_MEM_ENTRY_STC
 *
 * Description:
 *      Describes the extended data in the active memory table.
 *
 * Fields:
 *      addressPtr          - (pointer to) base address in array
 *      addressNum          - number of adressess in array
 *      addressOffset       - offset of the next address in array in words
 *      addressNum1         - number of adressess in array1
 *      addressOffset1      - offset of the next address in array1 in words
 *
 * Comments:
*/
typedef struct {
    GT_U32 * addressPtr;
    GT_U32   addressNum;
    GT_U32   addressOffset;
    GT_U32   addressNum1;
    GT_U32   addressOffset1;
}SMEM_EXT_ACTIVE_MEM_ENTRY_STC;

/*******************************************************************************
*  SMEM_EXT_ACTIVE_MEM_ADDRESS_FIND_FUN
*
* DESCRIPTION:
*      Fuction finds memory active address in register DB.
*
* INPUTS:
*       deviceObjPtr    - device object PTR.
*       address         - address to find in register's DB
*       param           - specific parameter
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_TRUE     - the address found in register DB
*       GT_FALSE    - the address not found in register DB
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_BOOL (* SMEM_EXT_ACTIVE_MEM_ADDRESS_FIND_FUN) (
                              IN        SKERNEL_DEVICE_OBJECT * deviceObjPtr,
                              IN        GT_U32 address,
                              IN        GT_UINTPTR param);

/*
 * Typedef: struct SMEM_ACTIVE_MEM_ENTRY_STC
 *
 * Description:
 *      Describe the entry in the active memory table.
 *
 * Fields:
 *      address         : Address of the register.
 *      mask            : Mask to apply to an address.
 *      readFun         : Entry point of read active memory function.
 *      readFunParam    : Parameter for readFun function.
 *      writeFun        : Entry point of write active memory function.
 *      writeFunParam   : Parameter for writeFun function.
 *      extMemFindFun   : Extended memory find. Used to find address in registers DB.
 *
 * Comments:
 */

typedef struct SMEM_ACTIVE_MEM_ENTRY_T {
    GT_U32                    address;
    GT_U32                    mask;
    SMEM_ACTIVE_MEM_READ_FUN  readFun;
    GT_U32                    readFunParam;
    SMEM_ACTIVE_MEM_WRITE_FUN writeFun;
    GT_U32                    writeFunParam;
    SMEM_EXT_ACTIVE_MEM_ADDRESS_FIND_FUN  extMemAddressFindFun;

} SMEM_ACTIVE_MEM_ENTRY_STC;

SMEM_ACTIVE_MEM_ENTRY_STC* smemMallocAndCopyActiveTable(
    IN SKERNEL_DEVICE_OBJECT        * devObjPtr,
    IN const SMEM_ACTIVE_MEM_ENTRY_STC * orgActiveTablePtr,
    IN GT_U32                       numOfEntries);

/* last line in every active memory table */
#define  SMEM_ACTIVE_MEM_ENTRY_LAST_LINE_CNS    \
        /* must be last anyway */               \
        {END_OF_TABLE, SMEM_FULL_MASK_CNS, NULL,0,NULL,0}

#define START_BIND_UNIT_ACTIVE_MEM_MAC(_devObjPtr,_unitPtr) \
    static const SMEM_ACTIVE_MEM_ENTRY_STC unitActiveTable[] =    \
    {

#define END_BIND_UNIT_ACTIVE_MEM_MAC(_devObjPtr,_unitPtr) \
        /* last line in every active memory table */      \
        SMEM_ACTIVE_MEM_ENTRY_LAST_LINE_CNS               \
    };/*end of unitActiveTable */                         \
    /* malloc your own copy */                        \
    _unitPtr->unitActiveMemPtr = smemMallocAndCopyActiveTable(_devObjPtr,    \
        unitActiveTable,sizeof(unitActiveTable)/sizeof(unitActiveTable[0])); \


/* get offset of second pipe units by unit register address */
#define SMEM_UNIT_PIPE_OFFSET_GET_MAC(_devObjPtr, _regAddress , _pipeIdPtr) \
    ((_devObjPtr->devMemUnitPipeOffsetGet) ? \
        _devObjPtr->devMemUnitPipeOffsetGet(_devObjPtr, _regAddress , _pipeIdPtr) : 0)

/* duplicate the active memory for 2 pipes */
#define SMEM_ACTIVE_MEM_FOR_2_PIPES_MAC(pipeOffset,address,mask,readFun,readFunParam,writeFun,writeFunParam) \
    {address            ,mask,readFun,readFunParam,writeFun,writeFunParam},                                  \
    {address+pipeOffset ,mask,readFun,readFunParam,writeFun,writeFunParam}

/* global active memory for :
    no active memory to the unit.
   and no need to look all over the active memory of the device */
extern SMEM_ACTIVE_MEM_ENTRY_STC smemEmptyActiveMemoryArr[];

/* chunks of memory for a unit */
typedef struct SMEM_UNIT_CHUNKS_STCT{
    GT_U32          chunkIndex; /* chunk unit base index (chunkBase << 23 is base address) */
    SMEM_CHUNK_STC *chunksArray;/* array of chunks */
    GT_U32          numOfChunks;/* number of chunks in the array */

    struct SMEM_UNIT_CHUNKS_STCT *hugeUnitSupportPtr;/* (when not NULL) pointer to a huge sub unit that also includes this sub unit ,
                                                        and is already allocated all it's registers/memories */
    void*           otherPortGroupDevObjPtr;/*(when not NULL) pointer to device object of the actual device that hold the memory
                                            supporting 'shared memory' between port groups */

    SMEM_UNIT_CHUNK_TYPE_ENT    chunkType;
    GT_U32                      numOfUnits;/*number of Consecutive units (for huge units)*/
    SMEM_ACTIVE_MEM_ENTRY_STC   *unitActiveMemPtr;/* active memory dedicated to the unit */

    SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC   *unitDefaultRegistersPtr;/* default register values for the unit */
}SMEM_UNIT_CHUNKS_STC;

/* get the base address of the unit chunk
    unitInfoPtr of type : (SMEM_UNIT_CHUNKS_STC *) */
#define UNIT_CHUNK_BASE_ADDR_GET_MAC(dev,unitInfoPtr) \
    smemGenericUnitBaseAddressesGet(dev,unitInfoPtr)


/**
* @struct SMEM_ACTIVE_MEM_ENTRY_REPLACEMENT_STC
 *
 * @brief Describe the address in the active memory that should be replaced with
 * another entry info.
*/
typedef struct{

    GT_U32 oldAddress;

    /** @brief : info about the replacing entry
     *  Comments:
     *  this used for example for xCat that wants to use most info from ch3 but
     *  still to 'override' some of it
     */
    SMEM_ACTIVE_MEM_ENTRY_STC newEntry;

} SMEM_ACTIVE_MEM_ENTRY_REPLACEMENT_STC;

/*******************************************************************************
*  SMEM_SPEC_MEMORY_FIND_FUN
*
* DESCRIPTION:
*      Definition of memory address type specific search function of SKERNEL.
* INPUTS:
*       deviceObjPtr - device object PTR.
*       address     - Address for ASIC memory.
*       memSize     - Size of ASIC memory to read or write.
*       param       - Function specific parameter.
*
* OUTPUTS:
*       None
* RETURNS:
*                pointer to the memory location in the database
*       NULL - if address not exist, or memSize > than existed size
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_U32 * (* SMEM_SPEC_MEMORY_FIND_FUN ) (
                        IN SKERNEL_DEVICE_OBJECT *  deviceObjPtr,
                        IN SCIB_MEMORY_ACCESS_TYPE  accessType,
                        IN GT_U32                   address,
                        IN GT_U32                   memSize,
                        IN GT_UINTPTR               param);

/*******************************************************************************
*  SMEM_DEV_MEMORY_FIND_FUN
*
* DESCRIPTION:
*      Definition of memory address type device specific search function
* INPUTS:
*       deviceObjPtr    - device object PTR.
*       address         - Address for ASIC memory.
        accessType      - Memory access type
*       memSize         - Size of ASIC memory to read or write.
*
* OUTPUTS:
*       activeMemPtrPtr - pointer to the active memory entry or NULL if not
*                         exist.
* RETURNS:
*       pointer to the memory location in the database
*       NULL - if address not exist, or memSize > than existed size
*
* COMMENTS:
*
*******************************************************************************/
typedef void * (* SMEM_DEV_MEMORY_FIND_FUN ) (
                    IN struct SKERNEL_DEVICE_OBJECT_T *     deviceObjPtr,
                    IN SCIB_MEMORY_ACCESS_TYPE              accessType,
                    IN GT_U32                               address,
                    IN GT_U32                               memSize,
                    OUT struct SMEM_ACTIVE_MEM_ENTRY_T **   activeMemPtrPtr);


/**
* @struct SMEM_SPEC_FIND_FUN_ENTRY_STC
 *
 * @brief Describe the entry in the address type specific R/W functions table.
*/
typedef struct{

    /** : Entry point of the function. */
    SMEM_SPEC_MEMORY_FIND_FUN specFun;

    /** @brief : Additional address type specific parameter specFun .
     *  unitActiveMemPtr: pointer to active memory of the unit.
     *  when != NULL , using ONLY this active memory.
     *  when NULL , using the active memory of the device.
     *  Comments:
     */
    GT_UINTPTR specParam;

    SMEM_ACTIVE_MEM_ENTRY_STC *unitActiveMemPtr;

} SMEM_SPEC_FIND_FUN_ENTRY_STC;

/**
* @enum SMEM_UNIT_PCI_BUS_ENT
 *
 * @brief Units that can be used for lookup on PCI (bus) or other external memory space
 * before accessing the memory of the device
*/
typedef enum{

    /** MBUS external memory */
    SMEM_UNIT_PCI_BUS_MBUS_E,

    /** DFX server external memory */
    SMEM_UNIT_PCI_BUS_DFX_E,

    SMEM_UNIT_PCI_BUS_UNIT_LAST_E

} SMEM_UNIT_PCI_BUS_ENT;

/*******************************************************************************
 * Typedef: struct SMEM_UNIT_CHUNK_BASE_ADDRESS_STC
 *
 * Description:
 *      Unit chunks memory structure
 *
 *******************************************************************************/
typedef struct{
    SMEM_UNIT_CHUNKS_STC            unitMem;
    GT_U32                          unitBaseAddr;
} SMEM_UNIT_CHUNK_BASE_ADDRESS_STC;


/**
* @enum SMEM_GENERIC_HIDDEN_MEM_ENT
 *
 * @brief types of hidden memories that the CPU can not direct access to , and
 *  hold not 'CIDER' address
*/
typedef enum{
     SMEM_GENERIC_HIDDEN_MEM_FDB_E
    ,SMEM_GENERIC_HIDDEN_MEM_FDB_TILE1_E
    ,SMEM_GENERIC_HIDDEN_MEM_FDB_TILE2_E
    ,SMEM_GENERIC_HIDDEN_MEM_FDB_TILE3_E

    ,SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_E
    ,SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_TILE1_E
    ,SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_TILE2_E
    ,SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_TILE3_E

    ,SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL0_E
    ,SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL1_E
    ,SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL2_E
    ,SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL3_E
    ,SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL4_E
    ,SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL5_E
    ,SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL6_E
    ,SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL7_E

    ,SMEM_GENERIC_HIDDEN_MEM___LAST___E

}SMEM_GENERIC_HIDDEN_MEM_ENT;

/**
* @struct SMEM_GENERIC_HIDDEN_MEM_STC
 *
 * @brief Describe the info per hidden table
*/
typedef struct{
    GT_U32  *startOfMemPtr;               /*the start of the allocated memory*/
    GT_U32  alignmentOfEntryInWords;      /*the number of words for alignment (and allocation) per entry */
    GT_U32  numOfEntries;                 /*the number of entries*/
    SMEM_GENERIC_HIDDEN_MEM_ENT tableType;/* currently not used */
    GT_U32  currentWordIndex;             /* current word index */
}SMEM_GENERIC_HIDDEN_MEM_STC;


/**
* @internal smemInit function
* @endinternal
*
* @brief   Init memory module for a device.
*
* @param[in] deviceObj                - pointer to device object.
*/
void smemInit
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);


/**
* @internal smemInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] deviceObj                - pointer to device object.
*/
void smemInit2
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemRegSet function
* @endinternal
*
* @brief   Write data to the register.
*
* @param[in] deviceObj                - pointer to device object.
* @param[in] address                  -  of register.
* @param[in] data                     - new register's value.
*
* @note The function aborts application if address not exist.
*
*/
void smemRegSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj,
    IN GT_U32                  address,
    IN GT_U32                  data
);
/**
* @internal smemRegGet function
* @endinternal
*
* @brief   Read data from the register.
*
* @param[out] data                     - pointer to register's value.
*
* @note The function aborts application if address not exist.
*
*/
void smemRegGet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    OUT GT_U32                * data
);

/*******************************************************************************
*   smemMemGet
*
* DESCRIPTION:
*       Return pointer to the register's or tables's memory.
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       address     - address of memory(register or table).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*
* COMMENTS:
*      The function aborts application if address not exist.
*
*******************************************************************************/
void * smemMemGet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address
);

/**
* @internal smemMemSet function
* @endinternal
*
* @brief   Write data to a register or table.
*
* @param[in] deviceObj                - pointer to device object.
* @param[in] address                  -  of memory(register or table).
* @param[in] dataPtr                  - new data location for memory .
* @param[in] dataSize                 - size of memory location to be updated.
*
* @note The function aborts application if address not exist.
*
*/
void smemMemSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj,
    IN GT_U32                  address,
    IN GT_U32        *         dataPtr,
    IN GT_U32                  dataSize
);

/**
* @internal smemRegFldGet function
* @endinternal
*
* @brief   Read data from the register's field
*
* @note The function aborts application if address not exist.
*
*/
void smemRegFldGet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  fldFirstBit,
    IN GT_U32                  fldSize,
    OUT GT_U32               * data
);
/**
* @internal smemPciRegFldGet function
* @endinternal
*
* @brief   Read data from the PCI register's field
*
* @note The function aborts application if address not exist.
*
*/
void smemPciRegFldGet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  fldFirstBit,
    IN GT_U32                  fldSize,
    OUT GT_U32               * data
);
/**
* @internal smemRegFldSet function
* @endinternal
*
* @brief   Write data to the register's field
*
* @note The function aborts application if address not exist.
*
*/
void smemRegFldSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  fldFirstBit,
    IN GT_U32                  fldSize,
    IN GT_U32                  data
);

/**
* @internal smemPciRegFldSet function
* @endinternal
*
* @brief   Write data to the PCI register's field
*
* @note The function aborts application if address not exist.
*
*/
void smemPciRegFldSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  fldFirstBit,
    IN GT_U32                  fldSize,
    IN GT_U32                  data
);

/**
* @internal smemPciRegSet function
* @endinternal
*
* @brief   Write data to the PCI register's
*
* @note The function aborts application if address not exist.
*
*/
void smemPciRegSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  data
);

/**
* @internal smemPciRegGet function
* @endinternal
*
* @brief   Read data from PCI register's
*
* @param[out] dataPtr                  - pointer to register's value.
*
* @note The function aborts application if address not exist.
*
*/
void smemPciRegGet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    OUT GT_U32                  *dataPtr
);

/**
* @internal smemRmuReadWriteMem function
* @endinternal
*
* @brief   Read/Write data to/from a register or table.
*
* @param[in] accessType               - define access operation Read or Write.
*                                      deviceObj   - pointer to device object.
* @param[in] address                  -  of memory(register or table).
*                                      dataPtr     - new data location for memory .
*                                      dataSize    - size of memory location to be updated.
* @param[in,out] memPtr                   - for Write this pointer to application memory,which
*                                      will be copied to the ASIC memory .
*
* @note The function aborts application if address not exist.
*
*/
void  smemRmuReadWriteMem
(
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN void * deviceObjPtr,
    IN GT_U32 address,
    IN GT_U32 memSize,
    INOUT GT_U32 * memPtr
);

/**
* @internal smemReadWriteMem function
* @endinternal
*
* @brief   Read/Write data to/from a register or table.
*
* @param[in] accessType               - define access operation Read or Write.
*                                      deviceObj   - pointer to device object.
* @param[in] address                  -  of memory(register or table).
*                                      dataPtr     - new data location for memory .
*                                      dataSize    - size of memory location to be updated.
* @param[in,out] memPtr                   - for Write this pointer to application memory,which
*                                      will be copied to the ASIC memory .
*
* @note The function aborts application if address not exist.
*
*/
void  smemReadWriteMem
(
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN void * deviceObjPtr,
    IN GT_U32 address,
    IN GT_U32 memSize,
    INOUT GT_U32 * memPtr
);


/**
* @internal smemInitMemChunk__internal function
* @endinternal
*
* @brief   init unit memory chunk
*
* @param[out] unitChunksPtr            - (pointer to) the unit memories chunks
*                                       none
*/
void smemInitMemChunk__internal
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SMEM_CHUNK_BASIC_STC  basicMemChunksArr[],
    IN GT_U32           numOfChunks,
    OUT SMEM_UNIT_CHUNKS_STC  *unitChunksPtr,
    IN const char*     fileNamePtr,
    IN GT_U32          line
);

#define smemInitMemChunk(devObjPtr,basicMemChunksArr,numOfChunks,unitChunksPtr) \
        smemInitMemChunk__internal(devObjPtr,basicMemChunksArr,numOfChunks,unitChunksPtr,__FILE__,__LINE__)

/**
* @internal smemInitMemChunkExt__internal function
* @endinternal
*
* @brief   Allocates and init unit memory at flat model and using formula
*
* @param[in] chunksMemArrExt[]        - array of extended memory chunks info
* @param[in] numOfChunks              - number of extended chunks
* @param[in] fileNamePtr              -  file name that called the allocation
* @param[in] line                     -   in the file that called the allocation
*
* @param[out] unitChunksPtr            - (pointer to) the unit memories chunks
*                                       none
*/
void smemInitMemChunkExt__internal
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SMEM_CHUNK_EXTENDED_STC  chunksMemArrExt[],
    IN GT_U32                   numOfChunks,
    OUT SMEM_UNIT_CHUNKS_STC    *unitChunksPtr,
    IN const char*     fileNamePtr,
    IN GT_U32          line
);
#define smemInitMemChunkExt(devObjPtr,chunksMemArrExt,numOfChunks,unitChunksPtr)    \
        smemInitMemChunkExt__internal(devObjPtr,chunksMemArrExt,numOfChunks,unitChunksPtr,__FILE__,__LINE__)

/**
* @internal smemInitMemCombineUnitChunks__internal function
* @endinternal
*
* @brief   takes 2 unit chunks and combine them into first unit.
*         NOTEs:
*         1. the function will re-allocate memories for the combined unit
*         2. all the memories of the second unit chunks will be free/reused by
*         first unit after the action done. so don't use those memories afterwards.
*         3. the 'second' memory will have priority over the memory of the 'first' memory.
*         meaning that if address is searched and it may be found in 'first' and
*         in 'second' , then the 'second' memory is searched first !
*         this is to allow 'override' of memories from the 'first' by memories
*         of the 'second'
* @param[in] firstUnitChunks          - (pointer to) the first unit memories chunks
* @param[in] secondUnitChunks         - (pointer to) the second unit memories chunks
* @param[in] fileNamePtr              -  file name that called the allocation
* @param[in] line                     -   in the file that called the allocation
*                                       none
*/
void smemInitMemCombineUnitChunks__internal
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC     *firstUnitChunks,
    IN SMEM_UNIT_CHUNKS_STC     *secondUnitChunks,
    IN const char*     fileNamePtr,
    IN GT_U32          line
);

#define smemInitMemCombineUnitChunks(devObjPtr,firstUnitChunks,secondUnitChunks) \
        smemInitMemCombineUnitChunks__internal(devObjPtr,firstUnitChunks,secondUnitChunks,__FILE__,__LINE__)

/**
* @internal smemInitMemUpdateUnitChunks__internal function
* @endinternal
*
* @brief   takes 2 unit chunks and combine them into first unit.
*         NOTEs:
*         1. Find all duplicated chunks in destination chunks array and reallocate
*         them according to appended chunk info.
*         2. The function will re-allocate memories for the combined unit
*         3. All the memories of the "append" unit chunks will be free/reused by
*         first unit after the action done. so don't use those memories afterwards.
* @param[in] unitChunksAppendToPtr    - (pointer to) the destination unit memories chunks
* @param[in] unitChunksAppendPtr      - (pointer to) the unit memories chunks to be appended
* @param[in] fileNamePtr              -  file name that called the allocation
* @param[in] line                     -   in the file that called the allocation
*                                       none
*/
void smemInitMemUpdateUnitChunks__internal
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC     *unitChunksAppendToPtr,
    IN SMEM_UNIT_CHUNKS_STC     *unitChunksAppendPtr,
    IN const char*     fileNamePtr,
    IN GT_U32          line
);
#define smemInitMemUpdateUnitChunks(devObjPtr,unitChunksAppendToPtr,unitChunksAppendPtr) \
        smemInitMemUpdateUnitChunks__internal(devObjPtr,unitChunksAppendToPtr,unitChunksAppendPtr,__FILE__,__LINE__)


/**
* @internal smemUnitChunkAddBasicChunk__internal function
* @endinternal
*
* @brief   add basic chunk to unit chunk.
*         NOTEs:
* @param[in] unitChunkPtr             - (pointer to) unit memories chunk
* @param[in] basicChunkPtr            - (pointer to) basic chunk
* @param[in] numBasicElements         - number of elements in the basic chunk
* @param[in] fileNamePtr              -  file name that called the allocation
* @param[in] line                     -   in the file that called the allocation
*                                       none
*/
void smemUnitChunkAddBasicChunk__internal
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC     *unitChunkPtr,
    IN SMEM_CHUNK_BASIC_STC     *basicChunkPtr,
    IN GT_U32                   numBasicElements,
    IN const char*     fileNamePtr,
    IN GT_U32          line
);

#define smemUnitChunkAddBasicChunk(devObjPtr,unitChunkPtr,basicChunkPtr_and_numBasicElements)    \
        smemUnitChunkAddBasicChunk__internal(devObjPtr,unitChunkPtr,basicChunkPtr_and_numBasicElements,__FILE__,__LINE__)


/*******************************************************************************
* smemTestDeviceIdToDevPtrConvert
*
* DESCRIPTION:
*        test utility -- convert device Id to device pointer
*
* INPUTS:
*       deviceId       - device id to refer to
*
* OUTPUTS:
*
* RETURN:
*       the device pointer
* COMMENTS:
*   if not found --> fatal error
*
*******************************************************************************/
SKERNEL_DEVICE_OBJECT* smemTestDeviceIdToDevPtrConvert
(
    IN  GT_U32                      deviceId
);

/**
* @internal genericTablesTestValidity function
* @endinternal
*
* @brief   test utility -- check that all the general table base addresses exists
*         in the device memory
* @param[in] devNum                   - the device index
*/
GT_U32  genericTablesTestValidity(
    IN GT_U32 devNum
);

/**
* @internal smemTestDevice function
* @endinternal
*
* @brief   test utility -- function to test memory of device
*
* @param[in] devNum                   - the device index
* @param[in] registersAddrPtr         - array of addresses of registers
* @param[in] numRegisters             - number of registers
* @param[in] tablesInfoPtr            - array of tables info
* @param[in] numTables                - number of tables
* @param[in] testActiveMemory         - to test (or not) the addresses of the active memory to
*                                      see if exists
*/
void   smemTestDevice
(
    IN GT_U32               devNum,
    IN GT_U32               *registersAddrPtr,
    IN GT_U32               numRegisters,
    IN DEV_TABLE_INFO_STC   *tablesInfoPtr,
    IN GT_U32               numTables,
    IN GT_BOOL              testActiveMemory
);

/**
* @internal smemDebugModeByPassFatalErrorOnMemNotExists function
* @endinternal
*
* @brief   debug utill - set a flag to allow to by pass the fatal error on accessing to
*         non-exists memory. this allow to use test of memory without the need to stop
*         on every memory .
*         we can get a list of all registers at once.
*
* @param[out] byPassFatalErrorOnMemNotExists - bypass or no the fatal error on accessing
*                                      to non exists memory
*                                       pointer to the memory location
*
* @retval NULL                     - if memory not exist
*/
void smemDebugModeByPassFatalErrorOnMemNotExists(
    IN GT_U32 byPassFatalErrorOnMemNotExists
);


/**
* @internal smemTableEntryAddressGet function
* @endinternal
*
* @brief   function return the address of the entry in the table.
*         NOTE: no checking on the out of range violation
* @param[in] tablePtr                 - pointer to table info
* @param[in] index                    -  of the entry in the table
* @param[in] instanceIndex            - index of instance. for multi-instance support.
*                                      ignored when == SMAIN_NOT_VALID_CNS
*                                       the address of the specified entry in the specified table
*/
GT_U32  smemTableEntryAddressGet(
    IN  SKERNEL_TABLE_INFO_STC    *tablePtr,
    IN  GT_U32                    index,
    IN  GT_U32                    instanceIndex
);

/**
* @internal smemTableEntry2ParamsAddressGet function
* @endinternal
*
* @brief   function return the address of the entry in the table.
*         NOTE: no checking on the out of range violation
* @param[in] tablePtr                 - pointer to table info
* @param[in] index1                   -  of the entry in the table
* @param[in] index2                   -  of the entry in the table
* @param[in] instanceIndex            - index of instance. for multi-instance support.
*                                      ignored when == SMAIN_NOT_VALID_CNS
*                                       the address of the specified entry in the specified table
*/
GT_U32  smemTableEntry2ParamsAddressGet(
    IN  SKERNEL_TABLE_2_PARAMETERS_INFO_STC    *tablePtr,
    IN  GT_U32                    index1,
    IN  GT_U32                    index2,
    IN  GT_U32                    instanceIndex
);

/**
* @internal smemTableEntry3ParamsAddressGet function
* @endinternal
*
* @brief   function return the address of the entry in the table.
*         NOTE: no checking on the out of range violation
* @param[in] tablePtr                 - pointer to table info
* @param[in] index1                   -  of the entry in the table
* @param[in] index2                   -  of the entry in the table
* @param[in] index3                   -  of the entry in the table
* @param[in] instanceIndex            - index of instance. for multi-instance support.
*                                      ignored when == SMAIN_NOT_VALID_CNS
*                                       the address of the specified entry in the specified table
*/
GT_U32  smemTableEntry3ParamsAddressGet(
    IN  SKERNEL_TABLE_3_PARAMETERS_INFO_STC    *tablePtr,
    IN  GT_U32                    index1,
    IN  GT_U32                    index2,
    IN  GT_U32                    index3,
    IN  GT_U32                    instanceIndex
);

/**
* @internal smemTableEntry4ParamsAddressGet function
* @endinternal
*
* @brief   function return the address of the entry in the table.
*         NOTE: no checking on the out of range violation
* @param[in] tablePtr                 - pointer to table info
* @param[in] index1                   -  of the entry in the table
* @param[in] index2                   -  of the entry in the table
* @param[in] index3                   -  of the entry in the table
* @param[in] index4                   -  of the entry in the table
* @param[in] instanceIndex            - index of instance. for multi-instance support.
*                                      ignored when == SMAIN_NOT_VALID_CNS
*                                       the address of the specified entry in the specified table
*/
GT_U32  smemTableEntry4ParamsAddressGet(
    IN  SKERNEL_TABLE_4_PARAMETERS_INFO_STC    *tablePtr,
    IN  GT_U32                    index1,
    IN  GT_U32                    index2,
    IN  GT_U32                    index3,
    IN  GT_U32                    index4,
    IN  GT_U32                    instanceIndex
);

/**
* @internal smemUnitChunksReset function
* @endinternal
*
* @brief   Reset memory chunks by zeroes for Cheetah3 and above devices.
*
* @param[in] devObjPtr                - pointer to device object
*/
void smemUnitChunksReset
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr
);

/*******************************************************************************
*   smemDevMemoryCalloc
*
* DESCRIPTION:
*       Allocate device memory and store reference to allocations into internal array.
*
* INPUTS:
*       devObjPtr       - pointer to device object.
*       sizeObjCount    - count of objects to be allocated
*       sizeObjSizeof   - size of object to be allocated
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Pointer to allocated memory
*
* COMMENTS:
*       The function calls standard calloc function.
*       Internal data base will hold reference for all allocations
*       for future use in SW reset
*
*******************************************************************************/
void * smemDevMemoryCalloc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 sizeObjCount,
    IN GT_U32 sizeObjSizeof
);

/*******************************************************************************
*   smemDevFindInUnitChunk
*
* DESCRIPTION:
*       find the memory in the specified unit chunk
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       unitChunksPtr - pointer to the unit chunk
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter --> used as pointer to the memory unit chunk
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
GT_U32 *  smemDevFindInUnitChunk
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
);

/*******************************************************************************
*   smemFindCommonMemory
*
* DESCRIPTION:
*       Return pointer to the register's common memory.
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*
* OUTPUTS:
*       activeMemPtrPtr - (pointer to) the active memory entry or NULL if not
*                         exist.
*                         (if activeMemPtrPtr == NULL ignored)
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*******************************************************************************/
void * smemFindCommonMemory
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  memSize,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    OUT struct SMEM_ACTIVE_MEM_ENTRY_T **   activeMemPtrPtr
);

/**
* @internal smemRegUpdateAfterRegFile function
* @endinternal
*
* @brief   The function read the register and write it back , so active memory can
*         be activated on this address after loading register file.
*/
void smemRegUpdateAfterRegFile
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  dataSize
);


/**
* @internal smemBindTablesToMemories function
* @endinternal
*
* @brief   bind tables from devObjPtr->tablesInfo.<tableName> to memories specified
*         in chunksPtr
*/
void smemBindTablesToMemories(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC     *chunksPtr,
    IN GT_U32                   numOfUnits
);

/**
* @internal smemGenericTablesSectionsGet function
* @endinternal
*
* @brief   get the number of tables from each section and the pointer to the sections:
*         section of '1 parameter' tables
*         section of '2 parameters' tables
*         section of '3 parameters' tables
*         section of '4 parameters' tables
*
* @param[out] tablesPtr                - pointer to tables with single parameter
* @param[out] numTablesPtr             -  number of tables with single parameter
* @param[out] tables2ParamsPtr         - pointer to tables with 2 parameters
* @param[out] numTables2ParamsPtr      -number of tables with 2 parameters
* @param[out] tables3ParamsPtr         - pointer to tables with 3 parameters
* @param[out] numTables3ParamsPtr      - number of tables with 3 parameters
* @param[out] tables4ParamsPtr         - pointer to tables with 4 parameters
* @param[out] numTables4ParamsPtr      - number of tables with 4 parameters
*                                      RETURN:
*                                      COMMENTS:
*/
void  smemGenericTablesSectionsGet(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    OUT SKERNEL_TABLE_INFO_STC  **tablesPtr,
    OUT GT_U32  *numTablesPtr,
    OUT SKERNEL_TABLE_2_PARAMETERS_INFO_STC  **tables2ParamsPtr,
    OUT GT_U32  *numTables2ParamsPtr,
    OUT SKERNEL_TABLE_3_PARAMETERS_INFO_STC  **tables3ParamsPtr,
    OUT GT_U32  *numTables3ParamsPtr,
    OUT SKERNEL_TABLE_4_PARAMETERS_INFO_STC  **tables4ParamsPtr,
    OUT GT_U32  *numTables4ParamsPtr
);

/**
* @internal smemGenericUnitAddressesAlignToBaseAddress function
* @endinternal
*
* @brief   align all addresses of the unit according to base address of the unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitInfoPtr              - pointer to the relevant unit - before alignment of addresses.
* @param[in,out] unitInfoPtr              - pointer to the relevant unit - after alignment of addresses.
*/
void smemGenericUnitAddressesAlignToBaseAddress(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC *unitInfoPtr
);

/**
* @internal smemGenericUnitAddressesAlignToBaseAddress1 function
* @endinternal
*
* @brief   align all addresses of the unit according to base address of the unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitInfoPtr              - pointer to the relevant unit - before alignment of addresses.
* @param[in] isZeroBased              - GT_FALSE - the same as function smemGenericUnitAddressesAlignToBaseAddress
*                                      GT_TRUE  - the addresses in unitInfoPtr are '0' based and
*                                      need to add to all of then the unit base address from unitInfoPtr->chunkIndex
* @param[in,out] unitInfoPtr              - pointer to the relevant unit - after alignment of addresses.
*/
void smemGenericUnitAddressesAlignToBaseAddress1(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC *unitInfoPtr,
    IN    GT_BOOL               isZeroBased
);
/**
* @internal smemGenericRegistersArrayAlignForceUnitReset function
* @endinternal
*
* @brief   indication that addresses will be overriding the previous address without
*         checking that address belong to unit.
* @param[in] forceReset               - indication to force override
*/
void smemGenericRegistersArrayAlignForceUnitReset(
    IN    GT_BOOL               forceReset
);

/**
* @internal smemGenericRegistersArrayAlignToUnit function
* @endinternal
*
* @brief   align all addresses of the array of registers according to
*         to base address of the unit
* @param[in] registersArr[]           - array of registers
* @param[in] numOfRegisters           - number of registers in registersArr
* @param[in] unitInfoPtr              - pointer to the relevant unit .
*/
void smemGenericRegistersArrayAlignToUnit(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32               registersArr[],
    IN GT_U32               numOfRegisters,
    IN SMEM_UNIT_CHUNKS_STC *unitInfoPtr
);

/**
* @internal smemGenericRegistersArrayAlignToUnit1 function
* @endinternal
*
* @brief   align all addresses of the array of registers according to
*         to base address of the unit
* @param[in] registersArr[]           - array of registers
* @param[in] numOfRegisters           - number of registers in registersArr
* @param[in] unitInfoPtr              - pointer to the relevant unit .
* @param[in] isZeroBased              - GT_FALSE - the same as function smemGenericRegistersArrayAlignToUnit
*                                      GT_TRUE  - the addresses in unitInfoPtr are '0' based and
*                                      need to add to all of then the unit base address from unitInfoPtr->chunkIndex
*/
void smemGenericRegistersArrayAlignToUnit1(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32               registersArr[],
    IN GT_U32               numOfRegisters,
    IN SMEM_UNIT_CHUNKS_STC *unitInfoPtr,
    IN    GT_BOOL               isZeroBased
);

/*******************************************************************************
*   smemGenericFindMem
*
* DESCRIPTION:
*       Return pointer to the register's or tables's memory.
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       accessType  - Read/Write operation
*       address     - address of memory(register or table).
*       memsize     - size of memory
*
* OUTPUTS:
*       activeMemPtrPtr - pointer to the active memory entry or NULL if not exist.
*
* RETURNS:
*       pointer to the memory location
*       NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
void * smemGenericFindMem
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32                  address,
    IN GT_U32                  memSize,
    OUT SMEM_ACTIVE_MEM_ENTRY_STC ** activeMemPtrPtr
);

/**
* @internal smemIsDeviceMemoryOwner function
* @endinternal
*
* @brief   Return indication that the device is the owner of the memory.
*         relevant to multi port groups where there is 'shared memory' between port groups.
*
* @retval GT_TRUE                  - the device is   the owner of the memory.
* @retval GT_FALSE                 - the device is NOT the owner of the memory.
*/
GT_BOOL smemIsDeviceMemoryOwner
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  address
);

/**
* @internal smemDfxRegFldGet function
* @endinternal
*
* @brief   Read data from the DFX register's field
*
* @note The function aborts application if address not exist.
*
*/
void smemDfxRegFldGet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  fldFirstBit,
    IN GT_U32                  fldSize,
    OUT GT_U32               * data
);

/**
* @internal smemDfxRegFldSet function
* @endinternal
*
* @brief   Write data to the DFX register's field
*
* @note The function aborts application if address not exist.
*
*/
void smemDfxRegFldSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  fldFirstBit,
    IN GT_U32                  fldSize,
    IN GT_U32                  data
);

/**
* @internal smemDfxRegSet function
* @endinternal
*
* @brief   Write data to the DFX register's
*
* @note The function aborts application if address not exist.
*
*/
void smemDfxRegSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  data
);

/**
* @internal smemDfxRegGet function
* @endinternal
*
* @brief   Read data from the DFX register's
*
* @note The function aborts application if address not exist.
*
*/
void smemDfxRegGet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                * dataPtr
);

/**
* @internal smemBar0RegSet function
* @endinternal
*
* @brief   Write data to the BAR0 register's (without active memory)
*
* @note The function aborts application if address not exist.
*
*/
void smemBar0RegSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  data
);

/**
* @internal smemDeviceObjMemoryAllAllocationsFree function
* @endinternal
*
* @brief   free all memory allocations that the device ever done.
*         optionally to including 'own' pointer !!!
*         this device must do 'FULL init sequence' if need to be used.
* @param[in] devObjPtr                - pointer to device object
* @param[in] freeMyObj                - indication to free the memory of 'device itself'
*                                      GT_TRUE  - free devObjPtr
*                                      GT_FALSE - do not free devObjPtr
*/
void smemDeviceObjMemoryAllAllocationsFree
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN GT_BOOL                  freeMyObj
);

/**
* @internal smemDeviceObjMemoryAlloc__internal function
* @endinternal
*
* @brief   allocate memory (calloc style) that is registered with the device object.
*         this call instead of direct call to calloc/malloc needed for 'soft reset'
*         where all the dynamic memory of the device should be free/cleared.
* @param[in] devObjPtr                - pointer to device object (ignored if NULL)
* @param[in] nitems                   - alloc for number of elements (calloc style)
* @param[in] size                     -  of each element         (calloc style)
* @param[in] fileNamePtr              -  file name that called the allocation
* @param[in] line                     -   in the file that called the allocation
*                                       pointer to allocated memory
*/
GT_PTR smemDeviceObjMemoryAlloc__internal
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN size_t   nitems,
    IN size_t   size,
    IN const char*              fileNamePtr,
    IN GT_U32                   line
);
#define smemDeviceObjMemoryAlloc(devObjPtr,nitems,size) \
        smemDeviceObjMemoryAlloc__internal(devObjPtr,nitems,size,__FILE__,__LINE__)

/**
* @internal smemDeviceObjMemoryRealloc__internal function
* @endinternal
*
* @brief   return new memory allocation after 'realloc' old memory to new size.
*         the oldPointer should be pointer that smemDeviceObjMemoryAlloc(..) returned.
*         this call instead of direct call to realloc needed for 'soft reset'
*         where all the dynamic memory of the device should be free/cleared.
* @param[in] devObjPtr                - pointer to device object (ignored if NULL)
* @param[in] oldPointer               - pointer to the 'old' element (realloc style)
* @param[in] size                     - number of bytes for new segment (realloc style)
* @param[in] fileNamePtr              -  file name that called the re-allocation
* @param[in] line                     -   in the file that called the re-allocation
*                                       pointer to reallocated memory
*/
GT_PTR smemDeviceObjMemoryRealloc__internal
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN GT_PTR                   oldPointer,
    IN size_t                   size,
    IN const char*              fileNamePtr,
    IN GT_U32                   line
);
#define smemDeviceObjMemoryRealloc(devObjPtr,oldPointer,size) \
        smemDeviceObjMemoryRealloc__internal(devObjPtr,oldPointer,size,__FILE__,__LINE__)

/**
* @internal smemDeviceObjMemoryPtrFree__internal function
* @endinternal
*
* @brief   free memory that was allocated by smemDeviceObjMemoryAlloc or
*         smemDeviceObjMemoryRealloc
*         this call instead of direct call to free needed for 'soft reset'
*         where all the dynamic memory of the device should be free/cleared.
* @param[in] devObjPtr                - pointer to device object (ignored if NULL)
* @param[in] oldPointer               - pointer to the 'old' pointer that need to be free
* @param[in] fileNamePtr              -  file name that called the free
* @param[in] line                     -   in the file that called the free
*                                       none.
*/
void smemDeviceObjMemoryPtrFree__internal
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN GT_PTR                   oldPointer,
    IN const char*              fileNamePtr,
    IN GT_U32                   line
);
#define smemDeviceObjMemoryPtrFree(devObjPtr,oldPointer) \
        smemDeviceObjMemoryPtrFree__internal(devObjPtr,oldPointer,__FILE__,__LINE__)

/**
* @internal smemDeviceObjMemoryPtrMemSetZero function
* @endinternal
*
* @brief   'memset 0' to memory that was allocated by smemDeviceObjMemoryAlloc or
*         smemDeviceObjMemoryRealloc
* @param[in] devObjPtr                - pointer to device object
* @param[in] memPointer               - pointer to the memory that need to be set to 0
*                                       none.
*/
void smemDeviceObjMemoryPtrMemSetZero
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN GT_PTR                   memPointer
);

/**
* @internal smemDeviceObjMemoryAllAllocationsSum function
* @endinternal
*
* @brief   debug function to print the sum of all allocations done for a device
*         allocations that done by smemDeviceObjMemoryAlloc or
*         smemDeviceObjMemoryRealloc
* @param[in] devObjPtr                - pointer to device object
*                                       none.
*/
void smemDeviceObjMemoryAllAllocationsSum
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr
);

/**
* @internal smemConvertGlobalPortToCurrentPipeId function
* @endinternal
*
* @brief   Convert 'global port' of DMA/GOP local port and pipe-id.
*         NOTE: the pipe-id is saved as 'currentPipeId' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task'
*         needed for multi-pipe device.
*         NOTE: if the 'global port' found as 'GOP port' the convert is to 'local GOP port'
*         if the 'global port' found as 'DMA port' the convert is to 'local DMA port' in it's DP[]
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalPortNum            - global port number of the device.
*                                       None
*
* @note function MUST be called when packet ingress the device.
*
*/
void smemConvertGlobalPortToCurrentPipeId
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  globalPortNum,
    OUT GT_U32                 *localPortNumberPtr
);

/*Gets 'cookiePtr' of the thread*/
extern GT_PTR simOsTaskOwnTaskCookieGet(GT_VOID);

/**
* @internal smemConvertCurrentPipeIdAndLocalPortToGlobal function
* @endinternal
*
* @brief   Convert (currentPipeId,port) of local port on pipe to (dev,port) of 'global'.
*         NOTE: the pipe-id is taken from 'currentPipeId' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task'
*         needed for multi-pipe device.
* @param[in] devObjPtr                - pointer to device object of the pipe.
* @param[in] localPortNum             - local port number of the pipe.
* @param[in] allowNotValidPort        - allow not valid port.
*                                      when GT_TRUE - if port not valid --> no fatal error.
*                                      globalPortNum converted to 'SMAIN_NOT_VALID_CNS'
*                                      when GT_FALSE - if port not valid --> fatal error.
*                                       None
*
* @note function MUST be called before packet egress the device.
*
*/
void smemConvertCurrentPipeIdAndLocalPortToGlobal
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  localPortNum,
    IN GT_BOOL                 allowNotValidPort,
    OUT GT_U32                 *globalPortNumPtr
);

/**
* @internal smemSetCurrentPipeId function
* @endinternal
*
* @brief   set current pipe-id (saved as 'currentPipeId' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task')
*         needed for multi-pipe device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] currentPipeId            - the current pipeId to set.
*                                       None
*/
void smemSetCurrentPipeId
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  currentPipeId
);

/**
* @internal smemGetCurrentPipeId function
* @endinternal
*
* @brief   get current pipe-id (saved as 'currentPipeId' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task')
*         needed for multi-pipe device.
* @param[in] devObjPtr                - pointer to device object.
*
* @retval currentPipeId            - the current pipeId to set.
*/
GT_U32 smemGetCurrentPipeId
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemSetCurrentMgUnitIndex function
* @endinternal
*
* @brief   set current MG unit Index (saved as 'currentMgUnitIndex' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task')
*         needed for multi-pipe device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] currentMgUnitIndex       - the current MG unit to set.
* @retval  -  None
*/
void smemSetCurrentMgUnitIndex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  currentMgUnitIndex
);

/**
* @internal smemGetCurrentMgUnitIndex function
* @endinternal
*
* @brief   get current MG unit Index (saved as 'currentMgUnitIndex' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task')
*         needed for multi-MG units device.
* @param[in] devObjPtr                - pointer to device object.
*
* @retval currentMgUnitIndex          - the current MG unit to set.
*/
GT_U32 smemGetCurrentMgUnitIndex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemSetCurrentTaskExtParamValue function
* @endinternal
*
* @brief   set value at Index (saved as 'extParamArr[index]=value' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task')
*         needed for multi-unit device.
* @param[in] devObjPtr  - pointer to device object.
* @param[in] index      - the index in which to set the value.
* @param[in] value      - the value to set. (at the index)
* @retval  -  None
*/
void smemSetCurrentTaskExtParamValue
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  index,
    IN GT_U32                  value
);

/**
* @internal smemGetCurrentTaskExtParamValue function
* @endinternal
*
* @brief   get value at Index (was saved as 'extParamArr[index]=value' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task')
*         needed for multi-unit device.
* @param[in] devObjPtr  - pointer to device object.
* @param[in] index      - the index in which to set the value.
* @retval  -  value (that was set by smemSetCurrentTaskExtParamValue)
*/
GT_U32 smemGetCurrentTaskExtParamValue
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  index
);

/**
* @internal smemRestCurrentTaskExtParamValues function
* @endinternal
*
* @brief   reset all values at all Indexes (saved as 'extParamArr[index]=value' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task')
*         needed for multi-unit device.
* @param[in] devObjPtr  - pointer to device object.
* @retval  -  None
*/
void smemRestCurrentTaskExtParamValues(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemSetCurrentMgUnitIndex_withRelativeMgToTile function
* @endinternal
*
* @brief   set current MG unit Index (saved as 'currentMgUnitIndex' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task')
*         needed for multi-pipe device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] currentMgUnitIndex_relativeToTile  - the current MG unit to set (relative to 'current tile').
* @retval  -  None
*/
void smemSetCurrentMgUnitIndex_withRelativeMgToTile
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  currentMgUnitIndex_relativeToTile
);

/**
* @internal smemSetCurrentMgUnitIndex_byRegAddr function
* @endinternal
*
* @brief   set current MG unit Index according to globalRegAddr
*         needed for multi-pipe device.
* @param[in] devObjPtr      - pointer to device object.
* @param[in] globalRegAddr  - the address of register.(relevant only if the register is one of MG register)
* @retval  -  None
*/
void smemSetCurrentMgUnitIndex_byRegAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  globalRegAddr
);


/**
* @internal smemConvertGlobalPipeIdToTileAndLocalPipeIdInTile function
* @endinternal
*
* @brief   convert global pipe-id to (tile-Id , local pipeIdInTile
*         needed for multi-tile device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalPipeId             - the global pipeId to convert.
*
* @param[out] tileIdPtr                - the Tile Id.
*                                      (ignored if NULL).
*
* @retval localPipeIdInTile        - the local pipeId in the Tile.
*/
GT_U32 smemConvertGlobalPipeIdToTileAndLocalPipeIdInTile
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                globalPipeId,
    OUT GT_U32               *tileIdPtr
);

/**
* @internal smemConvertTileAndLocalPipeIdInTileToGlobalPipeId function
* @endinternal
*
* @brief   convert (tile-Id , local pipeIdInTile) to global pipe-id .
*         needed for multi-tile device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] tileId                   - the Tile Id.
* @param[in] localPipeIdInTile        - the local pipeId in the Tile.
*
* @retval globalPipeId             - the global pipeId to convert.
*/
GT_U32 smemConvertTileAndLocalPipeIdInTileToGlobalPipeId
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                tileId,
    IN GT_U32                localPipeIdInTile
);

/**
* @internal smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex function
* @endinternal
*
* @brief   Convert (PipeId,dpIndex,localPort) of local port on DP to (dev,port) of 'global'.
*         needed for multi-pipe device.
* @param[in] devObjPtr                - pointer to device object of the pipe.
* @param[in] pipeIndex                - pipeIndex
* @param[in] dpIndex                  -  (relative to pipe)
* @param[in] localPortNum             - local port number of the DP.
* @param[in] isCpuSdma                - indication that the local port is 'CPU SDMA' or 'CPU networt'
*                                       relevant only to DMAs that hold 'mux' for the 'CPU port'
*  None
*/
void smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  pipeIndex,
    IN GT_U32                  dpIndex,
    IN GT_U32                  localPortNum,
    IN GT_BOOL                 isCpuSdma,
    OUT GT_U32                 *globalPortNumPtr
);

/**
* @internal smemMultiDpUnitIndexGet function
* @endinternal
*
* @brief   Get DP unit according to the port number.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] portNum                  - the port number
*                                       return the index of the DP unit
*/
GT_U32 smemMultiDpUnitIndexGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   portNum
);

/**
* @internal smemMultiDpUnitRelativePortGet function
* @endinternal
*
* @brief   Get DP local port number according to the global port number.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] portNum                  - the global port number (in the Pipe)
*                                       return the local port number
*/
GT_U32 smemMultiDpUnitRelativePortGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   portNum
);

/**
* @internal smemMultiDpUnitIsTxFifoEnableSdmaCpuPortGet function
* @endinternal
*
* @brief   check if the TxFifo unit enabled for 'CPU SDMA' or for 'CPU network port'
*         that connected to the DP unit (of this CPU SDMA port).
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalDpIndex            - the global TxFifo unit number
*
* return GT_TRUE  - the TxFifo unit in mode :     'CPU SDMA'
*        GT_FALSE - the TxFifo unit in mode : NOT 'CPU SDMA'
*/
GT_BOOL smemMultiDpUnitIsTxFifoEnableSdmaCpuPortGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   globalDpIndex
);

/**
* @internal smemMultiDpUnitIsCpuPortGet function
* @endinternal
*
* @brief   check if the global DMA port is 'CPU SDMA' and return the MG unit Id
*         that connected to the DP unit (of this CPU SDMA port).
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalDmaNum             - the global DMA port number
* @param[out] mgUnitIndexPtr          - (pointer to) the global MG unit Id that connected to the DP unit of this CPU SDMA port
*                                       ignored if NULL
*
* return GT_TRUE  - the global DMA port is     'CPU SDMA'
*        GT_FALSE - the global DMA port is NOT 'CPU SDMA'
*/
GT_BOOL smemMultiDpUnitIsCpuSdmaPortGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   globalDmaNum,
    OUT GT_U32  *mgUnitIndexPtr
);
/**
* @internal smemMultiDpUnitIsCpuSdmaPort_notConnected_Get function
* @endinternal
*
* @brief   check if the global DMA port is 'CPU SDMA' that is NOT connected.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalDmaNum             - the global DMA port number
* @param[out] mgUnitIndexPtr          - (pointer to) the global MG unit Id that connected to the DP unit of this CPU SDMA port
*                                       ignored if NULL
*
* return GT_TRUE  - the global DMA port is     'CPU SDMA' but is not connected.
*        GT_FALSE - the global DMA port is NOT 'CPU SDMA' or is connected !
*/
GT_BOOL smemMultiDpUnitIsCpuSdmaPort_notConnected_Get
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   globalDmaNum
);

/**
* @internal smemMultiDpUnitIsCpuPortGet function
* @endinternal
*
* @brief   check if the global DMA port is 'CPU network' or 'CPU SDMA'.
*         that connected to the DP unit (of this CPU SDMA port).
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalDmaNum             - the global DMA port number.
*
* return GT_TRUE  - the global DMA port is     'CPU network' or 'CPU SDMA'
*        GT_FALSE - the global DMA port is NOT 'CPU network' or 'CPU SDMA'
*/
GT_BOOL smemMultiDpUnitIsCpuPortGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   globalDmaNum
);

/**
* @internal smemMultiDpUnitFromCurrentMgGet function
* @endinternal
*
* @brief   get the DP index of the 'current MG' that is connected to.
*
* @param[in] devObjPtr                - pointer to device object.
*
* return the DP index of the 'current MG' that is connected to
*/
GT_U32 smemMultiDpUnitFromCurrentMgGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemMultiDpGlobalCpuSdmaNumFromCurrentMgGet function
* @endinternal
*
* @brief   get the CPU SDMA port number from the 'current MG' that is connected to.
*
* @param[in] devObjPtr                - pointer to device object.
*
* return the CPU SDMA port number from the 'current MG' that is connected to
*/
GT_U32 smemMultiDpGlobalCpuSdmaNumFromCurrentMgGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemRavenGlobalPortConvertGet function
* @endinternal
*
* @brief   convert global MAC number to Global Raven number , local MAC number in Raven....
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalPortNum            - the global MAC number
*
* @param[out] globalRavenNumPtr        - (pointer to) the global Raven number.
* @param[out] relativeRavenNumInPipePtr- (pointer to) the relative Raven number in Pipe.
* @param[out] localMacNumPtr           - (pointer to) the local MAC number in Raven.
* @param[out] localChannelIdInGroupPtr - (pointer to) the local channel Id in the channel group.
* @param[out] channelGroupPtr          - (pointer to) the local channel group.
* @param[out] isCpuPortPtr             - (pointer to) indication that the port is 'Cpu Port'
*
* @return none
*/
void smemRavenGlobalPortConvertGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32               globalPortNum,
    OUT GT_U32              *globalRavenNumPtr,
    OUT GT_U32              *relativeRavenNumInPipePtr,
    OUT GT_U32              *localMacNumPtr,
    OUT GT_U32              *localChannelIdInGroupPtr,
    OUT GT_U32              *localChannelGroupPtr,
    OUT GT_U32              *isCpuPortPtr
);

/**
* @internal smemRavenGlobalPortToRavenIndexGet function
* @endinternal
*
* @brief   Get Raven index according to the port number.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalPortNum            - the global port number
*
* return the Raven index
*/
GT_U32 smemRavenGlobalPortToRavenIndexGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   globalPortNum
);

/**
* @internal smemRavenGlobalPortToChannelGroupGet function
* @endinternal
*
* @brief   Get Raven local channel group according to the port number.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalPortNum            - the global port number
*
* return the Raven local channel group
*/
GT_U32 smemRavenGlobalPortToChannelGroupGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   globalPortNum
);

/**
* @internal smemRavenMtiPortInterruptCauseRegisterGet function
* @endinternal
*
* @brief   Get Port<n> Interrupt Cause Register for sip6 devices
*
* @param[in] devObjPtr           - device object PTR.
* @param[in] portNum             - port number
*/
GT_U32 smemRavenMtiPortInterruptCauseRegisterGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum
);

/**
* @internal resetRegDb function
* @endinternal
*
* @brief   Get DP local port number according to the global port number.
*
* @param[in] startUnitPtr             - pointer to memory of start unit
* @param[in] numRegisters             - the number of registers from startUnitPtr
* @param[in] stepOverStartUnit        - indication to skip start of unit that hold 'delemiter'
*                                      of SMEM_CHT_PP_REGS_UNIT_START_INFO_STC
*                                       None
*/
void resetRegDb(
    IN GT_U32                *startUnitPtr,
    IN GT_U32                numRegisters,
    IN GT_BOOL               stepOverStartUnit
);

/**
* @internal smemGenericUnitBaseAddressesGet function
* @endinternal
*
* @brief   get the base address of the unit chunk
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitInfoPtr              - pointer to the unit chunk.
*/
GT_U32  smemGenericUnitBaseAddressesGet(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC *unitInfoPtr
);

/*******************************************************************************
*   SMEM_DEVICE_UNIT_ALLOCATION_FUNC
*
* DESCRIPTION:
*       Allocate address type specific memories
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       unitPtr - pointer to the unit chunk
* OUTPUTS:
*       None.
*
* RETURNS:
*
* COMMENTS:
*
*
*******************************************************************************/
typedef void (*SMEM_DEVICE_UNIT_ALLOCATION_FUNC)
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
);
/* struct to allow generic calling to CB function of the unit (with common checking) */
typedef struct{
    GT_CHAR*                            unitNameStr;
    SMEM_DEVICE_UNIT_ALLOCATION_FUNC    allocUnitFunc;
}SMEM_DEVICE_UNIT_ALLOCATION_STC;

/**
* @internal smemGenericUnitAlloc function
* @endinternal
*
* @brief   function to alloc units in the unitsArr[]
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] unitsArr[]               - array of units info to alloc
*                                       None.
*/
void smemGenericUnitAlloc(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  SMEM_DEVICE_UNIT_ALLOCATION_STC unitsArr[]
);

typedef struct{
    GT_U32      base_addr;/* base address of unit*/
    GT_CHAR*    nameStr  ;/* name of unit */
    GT_U32      size;/* in bytes of the unit */
    GT_CHAR*    orig_nameStr  ;/* when orig_nameStr is not NULL then the nameStr
                                  is name of the duplicated unit , and the
                                  orig_nameStr is original unit.

                                  if PER_TILE_INDICATION_CNS    meaning : shared unit per tile    : between pipe 0,1 (or 2,3 or 4,5 or 6,7)
                                  if PER_2_TILES_INDICATION_CNS meaning : shared unit per 2 tiles : between pipe 0..3 (or 4..7)
                                  */
    GT_U32      pipeOffset;    /* pipe offset - zero indicates pipe 0, non-zero indicates pipe 1 */
}SMEM_GEN_UNIT_INFO_STC;

/**
* @internal smemGenericUnitSizeCheck function
* @endinternal
*
* @brief   check that no unit exceed the size of it's unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] unitInfoArr[]            - array of units info
*/
void smemGenericUnitSizeCheck (
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_GEN_UNIT_INFO_STC   unitInfoArr[]
);

/**
* @internal smemConvertDevAndAddrToNewDevAndAddr function
* @endinternal
*
* @brief   Convert (dev,address) to new (dev,address).
*         needed for multi-pipe device.
*
* @note function MUST be called before calling smemFindMemory()
*
*/
void smemConvertDevAndAddrToNewDevAndAddr  (
    IN SKERNEL_DEVICE_OBJECT * deviceObj,
    IN GT_U32                  address,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    OUT SKERNEL_DEVICE_OBJECT * *newDeviceObjPtrPtr,
    OUT GT_U32                  *newAddressPtr
);

/**
* @internal smemGenericHiddenFindMem function
* @endinternal
*
* @brief   Get pointer to the proper entry index in the specified hidden memory
* @param[in] devObjPtr                - pointer to device object.
* @param[in] memType                  - the memory type of hidden memory.
* @param[in] entryIndex               - the index in the hidden memory.
* @param[out] instanceIdPtr           - (pointer to)the instance ID (ignored if NULL).
*
*/
GT_U32 * smemGenericHiddenFindMem
(
    IN SKERNEL_DEVICE_OBJECT        * devObjPtr,
    IN SMEM_GENERIC_HIDDEN_MEM_ENT  memType,
    IN GT_U32                       entryIndex,
    OUT GT_U32                      *instanceIdPtr
);

/**
* @internal smemGenericHiddenMemSet function
* @endinternal
*
* @brief   Write data to a register or table.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] memType                  - the memory type of hidden memory.
* @param[in] entryIndex               - the index in the hidden memory.
* @param[in] dataPtr                  - new data location for memory .
* @param[in] dataSize                 - size of memory location to be updated.
*
* @note The function aborts application if memory/index not exist.
*
*/
void smemGenericHiddenMemSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_GENERIC_HIDDEN_MEM_ENT  memType,
    IN GT_U32                       entryIndex,
    IN GT_U32        *         dataPtr,
    IN GT_U32                  dataSize
);

/*****************************************/
/* start definitions for PHA FW support  */
/*****************************************/
typedef GT_U8      uint8_t;
typedef GT_U16     uint16_t;
typedef GT_U32       uint32_t;

/* the active device that is in the Firmware */
/* used by the FW for 'LOG info' */
extern SKERNEL_DEVICE_OBJECT * current_fw_devObjPtr;
extern GT_U32                  current_fw_ThreadId;

/* memory range info */
typedef struct MEMORY_RANGE_{
    uint32_t startAddr ;
    uint32_t numOfBytes ;
    char* namePtr;
    uint32_t *actualMemoryPtr;
}MEMORY_RANGE;

/* active memory prototype: same prototype for read and for write */
typedef enum {READ_OPER , WRITE_OPER}READ_OR_WRITE;
/* active memory function type */
typedef uint32_t (ACTIVE_MEM_FUNC)(uint32_t addr, uint32_t *memPtr , uint32_t writeValue/*relevant for write only*/);

/* active memory info */
typedef struct ACTIVE_MEMORY_RANGE_{
    uint32_t startAddr ;
    uint32_t numOfRegisters;
    uint32_t stepsBetweenRegisters;
    char* namePtr;
    ACTIVE_MEM_FUNC *writeFuncPtr;/*can be NULL*/
    ACTIVE_MEM_FUNC *readFuncPtr;/*can be NULL*/
}ACTIVE_MEMORY_RANGE;
/* accelerator info */
typedef struct ACCEL_INFO_STC_{
    char* operationNamePtr;
    uint32_t registerOffset;
    uint32_t threadId;/* ignored if 0 .(to support pipe)
                         in Falcon the registers uploaded in run
                         time so threads uses same addresses */
}ACCEL_INFO_STC;

/* memory access functions */
uint32_t* pha_findMem(uint32_t addr);

void ppaFwSpWrite(uint32_t addr,uint32_t data);
void ppaFwSpShortWrite(uint32_t addr,uint16_t data);
void ppaFwSpByteWrite(uint32_t addr,uint8_t data);
uint32_t ppaFwSpRead(uint32_t addr);
uint16_t ppaFwSpShortRead(uint32_t addr);
uint8_t ppaFwSpByteRead(uint32_t addr);

/* init functions */
void simulationPipeFirmwareInit(IN SKERNEL_DEVICE_OBJECT * devObjPtr);
void simulationFalconFirmwareInit(IN SKERNEL_DEVICE_OBJECT * devObjPtr);
void simulationAc5pFirmwareInit(IN SKERNEL_DEVICE_OBJECT * devObjPtr);
/* accelerator function */
uint32_t pha_fw_activeMem_write_ACCEL_CMDS_TRIG_BASE_ADDR(uint32_t addr, uint32_t *memPtr , uint32_t writeValue);
/* processing packets */
void actual_firmware_processing(IN SKERNEL_DEVICE_OBJECT * devObjPtr,IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,IN GT_U32 egressPort);

/*****************************************/
/* end definitions for PHA FW support    */
/*****************************************/


/**
* @internal smemChtAnswerToIpcCpssToWm function
* @endinternal
*
* @brief   The CPSS triggered 'string' to operate on the WM.
*/
void smemChtAnswerToIpcCpssToWm (
    IN GT_U32   wmDevNum,
    IN GT_U32   answer1,
    IN GT_U32   answer2,
    IN GT_U32   answer3
);

/**
* @internal smemDevSoftResetCommonStart function
* @endinternal
*
* @brief   common operations for sip5,sip6,AC5 on triggering soft reset
*          called in context of 'active memory'
*          (called from :
*           smemFalconActiveWriteCnmRfuSystemSoftResetReg ,
*           smemLion3ActiveWriteDfxServerResetControlReg
*
*/
void smemDevSoftResetCommonStart(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  globalControlRegisterContent
);

/*******************************************************************************
*   smemTableInternalDataPtrGet
*
* DESCRIPTION:
*       find the pointer to internal data of a table
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       address     - address in memory of table.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory of the internal data
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
GT_U32 *  smemTableInternalDataPtrGet
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32 address
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemh */



