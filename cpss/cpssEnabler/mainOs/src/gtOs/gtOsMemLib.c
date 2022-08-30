/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtOsMemLib.c
*
* @brief osMemPool implementation
*
* @version   20
********************************************************************************
*/

/* IMPORTANT : we must remove the declaration of OS_MALLOC_MEMORY_LEAKAGE_DBG */
/* because we not want local calls to osMalloc to be affected by the H file definitions */
#define FORCE_IGNORE_OS_MALLOC_MEMORY_LEAKAGE_DBG
#define OSMEMPOOLS_DEBUG
/***** Includes ********************************************************/
#ifdef _VXWORKS
#include <vxWorks.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsExc.h>
#include <gtOs/gtOsTimer.h>
#include <gtOs/gtOsSharedData.h>
#ifdef SHARED_MEMORY
#include <gtOs/gtOsSharedMalloc.h>
#endif

#ifdef ASIC_SIMULATION
# define CHECK_ADDRESS_ON_FREE
#endif

/*********  important ****  important  ***  important ****  important **********
NOTE: in this file the OS_MALLOC_MEMORY_LEAKAGE_DBG is NEVER defined
    instead this file must check
    INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG
    instead of
    OS_MALLOC_MEMORY_LEAKAGE_DBG
*********  important ****  important  ***  important ****  important **********/

/***** Function Declaration ********************************************/
void *osMemPoolsAlloc(GT_U32 size);
void *osMemPoolsRealloc(void *ptr, GT_U32 size);
void osMemPoolFree(void *memBlock);
void osMemInTestAllocStat(GT_VOID);

extern GT_VOID *internalOsMalloc(IN GT_U32   size);
extern GT_VOID *internalOsRealloc(IN GT_VOID * ptr ,IN GT_U32    size);
extern GT_VOID internalOsFree(IN GT_VOID* const memblock);

static GT_STATUS osMemChunkAllocate(GT_VOID);

static GT_STATUS osMemPoolsInit(GT_U32 blockSizeStep, GT_U32 stepsLimit,
                                GT_U32 poolsLimit);
static OSMEM_POOL_HDR *osMemPoolCreate(GT_U32 elemSize,GT_U32 poolIndex);
static GT_VOID* osMemPoolAllocSize(OSMEM_POOL_HDR_PTR poolHdrPtr,GT_U32 size);
static GT_VOID* osMemPoolAllocVarSize(OSMEM_POOL_HDR_PTR poolHdrPtr,GT_U32 size,GT_U32 *outSizePtr);
static GT_VOID *osMemAllocDynamicFromChunk(GT_U32 size,GT_U32 *chunkIndex);
static GT_STATUS osMemPoolClear(OSMEM_POOL_HDR_PTR poolHdrPtr);

#ifdef INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG
GT_STATUS osMemRegisterPtr(
    IN GT_VOID *    allocPtr,
    IN GT_U32       numOfBytes,
    IN const char*    fileNamePtr,
    IN GT_U32   line
);
void osMemUnregisterPtr(
    IN GT_VOID *    oldPointer,
    IN const char*    fileNamePtr,
    IN GT_U32   line
);
extern GT_VOID *osMalloc_MemoryLeakageDbg
(
    IN GT_U32   size,
    IN const char*    fileNamePtr,
    IN GT_U32   line
);
#else /*!INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG*/
/* define empty macro to reduce management when not needed */
#define osMemRegisterPtr(a,b,c,d)
#define osMemUnregisterPtr(a,b,c)
#endif /*!INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG*/


/***** External Functions **********************************************/
extern GT_STATUS mathLog2(IN  GT_U32  num,OUT GT_U32  *result);

#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>


/***** Global Vars access ********************************************/


#define PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_HELPER_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.gtOsMemLibSrc._var,_value)

#define PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(_var)\
    PRV_HELPER_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.gtOsMemLibSrc._var)

#define PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_HELPER_NON_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.osLinuxMemSrc._var,_value)

#define PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(_var)\
    PRV_HELPER_NON_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.osLinuxMemSrc._var)


/* poolIndex to mark osStaticMalloc */
#define POOL_INDEX_STATIC_CNS   0xFFFFFFFF

/* magic number ot try catch damaged DB */
#define MAGIC_NUMBER_CNS 0xa5a5aF15

#define REPORT_CORRUPTED_MEMORY_MAC     \
    osPrintf("memory corruption detected in file [%s] line [%d] \n" , __FILE__ , __LINE__)

/* allow to 'see' the total DMA allocations */
void   prvOsDmaTotalAllocPrint(void)
{
    GT_U32   lastValue = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(lastValue);
    GT_U32   dmaTotalAlloc = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(dmaTotalAlloc);

    GT_U32  printValue = dmaTotalAlloc;
    osPrintf("dmaTotalAlloc = [0x%x] bytes (%d) ",
        printValue,printValue);
    if(lastValue && (lastValue != dmaTotalAlloc))
    {
        if(dmaTotalAlloc > lastValue)
        {
            printValue = dmaTotalAlloc - lastValue;
            osPrintf(" +[0x%x] bytes (%d) \n",
                printValue,printValue);
        }
        else
        {
            printValue = lastValue - dmaTotalAlloc;
            osPrintf(" -[0x%x] bytes (%d) \n",
                printValue,printValue);
        }
    }
    else
    {
        osPrintf("\n");
    }

    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(lastValue,dmaTotalAlloc);
}
/* allow to 'see' the total DMA allocations */
void   prvOsDmaTotalAllocTraceEnable(IN GT_U32  enable)
{
    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(dmaTotalAlloc_traceEnable,enable);
}

#ifndef SHARED_MEMORY
   /* Workaround for dmaRegisterPtr(), dmaFreeAll(), memRegisterPtr()
    * in case when no osMemLibInit() called
    */
#  define DBG_MALLOC_MAC    malloc
#  define DBG_FREE_MAC      free
#else
#  define DBG_MALLOC_MAC    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_chunks_array.memAllocFuncPtr)
#  define DBG_FREE_MAC      PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_chunks_array.memFreeFuncPtr)
#endif

/* error in implementation of DMA detection ... cause false alarms ... need debug */
#define ERROR_IN_IMPLEMENTATION 1

#if ERROR_IN_IMPLEMENTATION == 0
/* DMA : state the values to be used by the predefined 'overflow detection' region at the
   end of each 'Cached DMA' */
#define  DMA_NUM_PRE_DEFINED_PATTERNS_CNS   4
static GT_U32 dmaPreDefinedPatternsArr[DMA_NUM_PRE_DEFINED_PATTERNS_CNS] __SHARED_DATA_MAINOS =
    { 0xdeadbeaf  , 0xbeafdead , 0xa5a5a5a5 , 0x5a5a5a5a};
#endif /*ERROR_IN_IMPLEMENTATION == 0*/

/* DMA : number of lines with the dmaPreDefinedPatternsArr[] patterns */
#define DMA_NUM_LINES_OF_PRE_DEFINED_PATTERNS   16

/* DMA : number of bytes that used as 'overflow detection' region at the end of each 'Cached DMA' */
#define  DMA_NUM_BYTES_OVERFLOW_DETECTION_REGION_CNS    ((DMA_NUM_PRE_DEFINED_PATTERNS_CNS*4) * DMA_NUM_LINES_OF_PRE_DEFINED_PATTERNS)

/***** Public Functions ************************************************/

/**
* @internal osBzero function
* @endinternal
*
* @brief   Fills the first nbytes characters of the specified buffer with 0
*
* @param[in] start                    -  address of memory block to be zeroed
* @param[in] nbytes                   - size of block to be set
*                                       None
*/
GT_VOID osBzero
(
    IN GT_CHAR * start,
    IN GT_U32 nbytes
)
{
    assert(start != NULL);
#ifdef _VXWORKS
    bzero((GT_CHAR *)start, nbytes);
#else
    memset((GT_VOID *)start, 0,  nbytes);
#endif
    return;
}

/*******************************************************************************
* osMemSet
*
* DESCRIPTION:
*       Stores 'symbol' converted to an unsigned char in each of the elements
*       of the array of unsigned char beginning at 'start', with size 'size'.
*
* INPUTS:
*       start  - start address of memory block for setting
*       symbol - character to store, converted to an unsigned char
*       size   - size of block to be set
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to set memory block
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID * osMemSet
(
    IN GT_VOID * start,
    IN int    symbol,
    IN GT_U32 size
)
{
    if(start == NULL)
        return start;
#ifdef _VXWORKS
    IS_WRAPPER_OPEN_PTR;
#endif
    return memset(start, symbol, size);
}

/*******************************************************************************
* osMemCpy
*
* DESCRIPTION:
*       Copies 'size' characters from the object pointed to by 'source' into
*       the object pointed to by 'destination'. If copying takes place between
*       objects that overlap, the behavior is undefined.
*
* INPUTS:
*       destination - destination of copy
*       source      - source of copy
*       size        - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to destination
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID * osMemCpy
(
    IN GT_VOID *       destination,
    IN const GT_VOID * source,
    IN GT_U32       size
)
{
    if(destination == NULL || source == NULL)
        return destination;

#ifdef _VXWORKS
    IS_WRAPPER_OPEN_PTR;
#endif
    return memcpy(destination, source, size);
}

/*******************************************************************************
* osMemMove
*
* DESCRIPTION:
*       Copies 'size' characters from the object pointed to by 'source' into
*       the object pointed to by 'destination'. The memory areas may overlap:
*       copying takes place as though the bytes in src are first copied into
*       a temporary array that does  not  overlap src or dest, and the bytes
*       are then copied from the temporary array to dest.
*
* INPUTS:
*       destination - destination of copy
*       source      - source of copy
*       size        - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to destination
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID * osMemMove
(
    IN GT_VOID *       destination,
    IN const GT_VOID * source,
    IN GT_U32          size
)
{
    if(destination == NULL || source == NULL)
        return destination;

#ifdef _VXWORKS
    IS_WRAPPER_OPEN_PTR;
#endif
    return memmove(destination, source, size);
}

/**
* @internal osMemCmp function
* @endinternal
*
* @brief   Compare 'size' characters from the object pointed to by 'str1' to
*         the object pointed to by 'str2'.
* @param[in] str1                     - first memory area
* @param[in] str2                     - second memory area
* @param[in] size                     -  of memory to compare
*
* @retval > 0                      - if str1 is alfabetic bigger than str2
* @retval == 0                     - if str1 is equal to str2
* @retval < 0                      - if str1 is alfabetic smaller than str2
*/
GT_32 osMemCmp
(
    IN const GT_VOID  *str1,
    IN const GT_VOID  *str2,
    IN GT_U32       size
)
{
    assert(str1 != NULL);
    assert(str2 != NULL);

    /* this code prevents undefined results of memcmp in case when:
        pointers are the same: str1 == str2, and str1 points to unaligned memory */
    if(str2 == str1)
    {
        osPrintf("osMemCmp warning: Tried to compare the same memory, size is [%d]. \n", size);
        return 0;
    }

#ifdef _VXWORKS
    IS_WRAPPER_OPEN_PTR;
#endif
    return (GT_32) memcmp(str1, str2, size);
}


/**
* @internal osMemLibInit function
* @endinternal
*
* @brief   Initialize the memory management with a user memory function.
*         Can be used to allocate from a dedicated memory pool that remains till
*         HW reset.
* @param[in] memAllocFuncPtr          - the function to be used in memory allocation.
* @param[in] memFreeFuncPtr           - the function to be used in memory allocation.
* @param[in] size                     - The memory  to manage
* @param[in] memAllocMethod           - determines the method of the allocation
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - not enough memory in the system.
*/
GT_STATUS osMemLibInit
(
    IN GT_MEMORY_ALLOC_FUNC memAllocFuncPtr,
    IN GT_MEMORY_FREE_FUNC memFreeFuncPtr,
    IN GT_U32 size,
    IN GT_MEMORY_MANAGE_METHOD_ENT memAllocMethod
)
{
    GT_U32 chunkMaxAmount = 0;
    GT_STATUS retVal = GT_OK;
    OSMEM_CHUNK_ARRAY * osMemChunksArrayPtr;
    GT_U32 wholeMallocSize = OSMEM_MAX_MEMORY_SIZE_USE;

#ifdef SHARED_MEMORY
    retVal = shrMemConfigGet(&wholeMallocSize);
    if (GT_OK != retVal)
            return retVal;
#endif
    osMemChunksArrayPtr = &(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_chunks_array));

    if (PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(firstMemInit) == GT_TRUE)
    {
        if (size == 0)
        {
            return GT_BAD_PARAM;
        }

        chunkMaxAmount = wholeMallocSize / size;
        osPrintf("MALLOC_MEMORY_SIZE_USE : %d\n", wholeMallocSize);
        osMemChunksArrayPtr->chunkArray = memAllocFuncPtr(sizeof(OSMEM_CHUNK_POINTER)
                                                            *chunkMaxAmount);
        osMemChunksArrayPtr->chunkSize = size;
        osMemChunksArrayPtr->lastChunkIndex = 0;
        osMemChunksArrayPtr->staticIndex = 0;
        osMemChunksArrayPtr->memAllocMethod = memAllocMethod;
        osMemChunksArrayPtr->chunkArraySize = chunkMaxAmount;
        osMemChunksArrayPtr->memAllocFuncPtr = memAllocFuncPtr;
        osMemChunksArrayPtr->memFreeFuncPtr = memFreeFuncPtr;


        retVal  = osMemChunkAllocate();
        if (retVal != GT_OK)
        {
            return retVal;
        }

        retVal = osMemPoolsInit(OSMEM_DEFAULT_BLOCK_STEP_SIZE,
                       OSMEM_DEFAULT_STEPS_LIMIT,
                       OSMEM_DEFAULT_POOLS_LIMIT);
        if (retVal != GT_OK)
        {
            memFreeFuncPtr(osMemChunksArrayPtr->chunkArray[0].startAddr);
            osMemChunksArrayPtr->lastChunkIndex = 0;
            memFreeFuncPtr(osMemChunksArrayPtr->chunkArray);
            osMemChunksArrayPtr->chunkArray = NULL;
            return retVal;
        }
#ifdef _VXWORKS
        /*gtShutdownRegister(osMemPoolsClear);*/
#endif
    }

    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(firstMemInit,GT_FALSE);

    return retVal;
}

/*******************************************************************************
* osStaticMallocFromChunk
*
* DESCRIPTION:
*       Allocates memory block of specified size, this memory will not be free.
*
* INPUTS:
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available or calling after first init.
*
* COMMENTS:
*       Usage of this function is only on FIRST initialization.
*
*******************************************************************************/
GT_VOID *osStaticMallocFromChunk
(
    IN GT_U32 size
)
{
    OSMEM_POOL_ELEM_PTR retPtr = NULL;
    OSMEM_CHUNK_POINTER_PTR curChunk;
    GT_U32 allocSize;
    OSMEM_CHUNK_ARRAY * osMemChunksArrayPtr;
    GT_MEMORY_ALLOC_FUNC     localMemAllocFuncPtr;

    /* Align size to 4 bytes. */
#ifdef OS_MEM_64_BIT_ALIGNED
    size = (size + 7) & 0xFFFFFFF8;
#else
    size = (size + 3) & 0xFFFFFFFC;
#endif


    osMutexLock(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id));
    osMemChunksArrayPtr = &(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_chunks_array));
    allocSize = size + sizeof (OSMEM_POOL_ELEM);

    localMemAllocFuncPtr = osMemChunksArrayPtr->memAllocFuncPtr;
#ifdef SHARED_MEMORY
    localMemAllocFuncPtr = (GT_MEMORY_ALLOC_FUNC)SHARED_MALLOC_MAC;
#endif

    if (allocSize > osMemChunksArrayPtr->chunkSize)
    {
        osMutexUnlock(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id));
        return localMemAllocFuncPtr(size);
    }
    curChunk =
        &(osMemChunksArrayPtr->chunkArray[osMemChunksArrayPtr->staticIndex]);

    while(osMemChunksArrayPtr->staticIndex < osMemChunksArrayPtr->lastChunkIndex &&
          curChunk->staticAddr - allocSize < curChunk->dynamicAddr)
    {
        osMemChunksArrayPtr->staticIndex++;

        curChunk =
        &(osMemChunksArrayPtr->chunkArray[osMemChunksArrayPtr->staticIndex]);
    }

    if (osMemChunksArrayPtr->staticIndex >= osMemChunksArrayPtr->lastChunkIndex)
    {
        if (osMemChunksArrayPtr->memAllocMethod ==
                GT_MEMORY_MANAGE_METHOD_NO_REALLOC_ON_DEMAND_E)
        {
            osMutexUnlock(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id));

            osPrintf("Out Of Memory!!\n");
            return NULL;
        }
        if (osMemChunkAllocate() != GT_OK)
        {
            osMutexUnlock(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id));
            return NULL;
        }
    }

    curChunk->staticAddr -= allocSize;
    curChunk->freeSize -= allocSize;
    retPtr = (OSMEM_POOL_ELEM_PTR)curChunk->staticAddr;

    retPtr->poolIndex = (GT_UINTPTR)POOL_INDEX_STATIC_CNS;  /* it's more safe to use larger value, it allow more than 255 pools */

    osMutexUnlock(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id));

    retPtr++;
    return (retPtr);
}

/*******************************************************************************
* osMemPoolsRealloc
*
* DESCRIPTION:
*       Reallocate memory block of specified size.
*
* INPUTS:
*       ptr  - pointer to previously allocated buffer
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*
* COMMENTS:
*       Usage of this function is NEVER during initialization.
*       Recommended to avoid usage of this function.
*
*******************************************************************************/
GT_VOID *osMemPoolsRealloc
(
    IN GT_VOID * ptr ,
    IN GT_U32    size
)
{
    GT_VOID *allocPtr = NULL;
    GT_U32  oldSize = size;
    GT_U32  poolIndex;
    OSMEM_VAR_POOL_ELEM_PTR allocVarPtr;
    OSMEM_NOPOOLS_HDR_STC *hdrPtr;
    OSMEM_POOL_HDR_PTR* os_mem_pools_array;

    allocPtr = osMemPoolsAlloc(size);
    if (allocPtr == NULL)
    {
        return NULL;
    }

    hdrPtr = (OSMEM_NOPOOLS_HDR_STC*)((GT_UINTPTR)ptr - sizeof(OSMEM_NOPOOLS_HDR_STC));
    /* resolve size of memory pointed by ptr */
    if (hdrPtr->magic != (GT_UINTPTR)hdrPtr) /* should be always true */
    {
        /* this memory is allocated by pool manager */
        poolIndex = (GT_U32)hdrPtr->magic;
        if (poolIndex <= PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_array_size))
        {
            os_mem_pools_array = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_array);
            oldSize = os_mem_pools_array[poolIndex]->elem_size;

            if (oldSize == 0)
            {
                allocVarPtr = (OSMEM_VAR_POOL_ELEM_PTR)
                              ((GT_VOID*)((GT_UINTPTR)ptr - sizeof(OSMEM_VAR_POOL_ELEM)));
                oldSize = (GT_U32)allocVarPtr->size;
            }
        }
    }
    else
    {
        oldSize = (GT_U32)hdrPtr->size;
    }

    osMemCpy(allocPtr,ptr,size > oldSize ? oldSize : size);
    osMemPoolFree(ptr);

    if (size > oldSize)
    {
        PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(reallocCounter,
                                        PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(reallocCounter)+size);
        PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(tempAllocCounter,
                                        PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(tempAllocCounter)+size);
        if (PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(tempAllocCounter) > PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(maxAllocCounter))
        {
            PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(maxAllocCounter,
                                        PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(tempAllocCounter));
        }
    }

    return allocPtr;
}

/**
* @internal osMemPoolsClear function
* @endinternal
*
* @brief   returns all the allocated memory from all the pools to free.
*
* @retval GT_OK                    - if success.
*/
GT_STATUS osMemPoolsClear()
{
    GT_U32 i,dynSize;
    OSMEM_POOL_HDR_PTR* os_mem_pools_array;
    OSMEM_CHUNK_ARRAY * osMemChunksArrayPtr;

    osMutexLock(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id));

    os_mem_pools_array = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_array);
    osMemChunksArrayPtr = &(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_chunks_array));

    for (i = 0; i < PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_array_size); ++i)
    {
        osMemPoolClear(os_mem_pools_array[i]);
    }

    for (i = 0; i < osMemChunksArrayPtr->lastChunkIndex; ++i)
    {
        dynSize = (GT_U32)(osMemChunksArrayPtr->chunkArray[i].dynamicAddr -
            osMemChunksArrayPtr->chunkArray[i].startAddr);

        osMemChunksArrayPtr->chunkArray[i].freeSize += dynSize;

        osMemChunksArrayPtr->chunkArray[i].dynamicAddr =
            osMemChunksArrayPtr->chunkArray[i].startAddr;

    }

    osMutexUnlock(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id));

    return GT_OK;
}

/**
* @internal osMemBindMemAllocFunc function
* @endinternal
*
* @brief   Binds the the memory allocation function
*
* @param[in] memAllocFuncPtr          - the function to be used in memory allocation
*                                      (refer to osMemLibInit)
*
* @retval GT_OK                    - operation succeeded
* @retval GT_FAIL                  - operation failed
*/
GT_STATUS osMemBindMemAllocFunc
(
    IN   GT_MEMORY_ALLOC_FUNC    memAllocFuncPtr
)
{
    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(os_mem_chunks_array.memAllocFuncPtr,memAllocFuncPtr);

    return GT_OK;
}

/***** Private ********************************************************/

/**
* @internal osMemChunkAllocate function
* @endinternal
*
* @brief   Allocates another chunk to the chunk array.
*
* @retval GT_OK                    - if success.
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS osMemChunkAllocate(GT_VOID)
{
    OSMEM_CHUNK_ARRAY * osMemChunksArrayPtr;
    GT_MEMORY_ALLOC_FUNC localMemAllocFuncPtr;

    osMemChunksArrayPtr = &(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_chunks_array));

    localMemAllocFuncPtr = osMemChunksArrayPtr->memAllocFuncPtr;
#ifdef SHARED_MEMORY
    localMemAllocFuncPtr = (GT_MEMORY_ALLOC_FUNC)SHARED_MALLOC_MAC;
#endif

    if (osMemChunksArrayPtr->lastChunkIndex ==osMemChunksArrayPtr->chunkArraySize)
    {
        return GT_OUT_OF_RANGE;
    }

    osMemChunksArrayPtr->chunkArray[osMemChunksArrayPtr->lastChunkIndex].startAddr
        = localMemAllocFuncPtr(osMemChunksArrayPtr->chunkSize);

    if (osMemChunksArrayPtr->chunkArray[osMemChunksArrayPtr->lastChunkIndex].startAddr
        == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    osMemChunksArrayPtr->chunkArray[osMemChunksArrayPtr->lastChunkIndex].dynamicAddr =
        osMemChunksArrayPtr->chunkArray[osMemChunksArrayPtr->lastChunkIndex].startAddr;

    osMemChunksArrayPtr->chunkArray[osMemChunksArrayPtr->lastChunkIndex].staticAddr =
        osMemChunksArrayPtr->chunkArray[osMemChunksArrayPtr->lastChunkIndex].startAddr +
        osMemChunksArrayPtr->chunkSize;

    osMemChunksArrayPtr->chunkArray[osMemChunksArrayPtr->lastChunkIndex].freeSize =
        osMemChunksArrayPtr->chunkSize;

    (osMemChunksArrayPtr->lastChunkIndex)++;
    return GT_OK;

}

#ifdef  CHECK_ADDRESS_ON_FREE
static GT_VOID checkMemoryAllocatedFromChunks(GT_VOID *ptr)
{
    GT_U32 i;
    GT_UINTPTR C;
    OSMEM_CHUNK_ARRAY * osMemChunksArrayPtr;

    osMemChunksArrayPtr = &(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_chunks_array));

    for (i = 0; i < osMemChunksArrayPtr->lastChunkIndex; i++)
    {
        C = (GT_UINTPTR)(osMemChunksArrayPtr->chunkArray[i].startAddr);
        if (((GT_UINTPTR)ptr >= C) &&
                ((GT_UINTPTR)ptr < (C + osMemChunksArrayPtr->chunkSize)))
        {
            return;
        }
    }
    osFatalError(OS_FATAL_WARNING, "Trying to free memory which is not allocated by osMalloc()");
}
#endif

/**
* @internal osMemPoolsInit function
* @endinternal
*
* @brief   initializes the pools for memory management. it creates pools with sizes
*         starting with blockSizeStep and adding blockSizeStep to the created pool
*         size until it reaches stepsLimit, from then on till poolsLimit it doubles
*         the size every time. (example: if blockSizeStep = 2, stepsLimit = 8,
*         poolsLimit = 32 :the pools that will be created are with element sizes
*         of : 2,4,6,8,16,32)
* @param[in] blockSizeStep            - the step in size that should be added for every
*                                      sequanctual pool created.
* @param[in] stepsLimit               - the limit at which sizes stop being add to but doubled
*                                      (must be a power of 2)
* @param[in] poolsLimit               - the biggest size pool that will be created.
*                                      (must be a power of 2)
*
* @retval GT_OK                    - if success.
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS osMemPoolsInit
(
    IN GT_U32 blockSizeStep,
    IN GT_U32 stepsLimit,
    IN GT_U32 poolsLimit
)
{
    GT_U32 i, j, curSize,log2ofStepsLimit,log2ofPoolsLimit;
    GT_U32 os_mem_pools_array_size;
    OSMEM_POOL_HDR_PTR* os_mem_pools_array;
    OSMEM_CHUNK_ARRAY * osMemChunksArrayPtr;
    GT_MEMORY_ALLOC_FUNC localMemAllocFuncPtr;
    GT_MEMORY_FREE_FUNC  localMemFreeFuncPtr;

    osMemChunksArrayPtr = &(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_chunks_array));

    localMemAllocFuncPtr = osMemChunksArrayPtr->memAllocFuncPtr;
    localMemFreeFuncPtr = osMemChunksArrayPtr->memFreeFuncPtr;
#ifdef SHARED_MEMORY
    localMemAllocFuncPtr = (GT_MEMORY_ALLOC_FUNC)SHARED_MALLOC_MAC;
    localMemFreeFuncPtr = SHARED_FREE_MAC;
#endif

    log2ofStepsLimit = 0;
    log2ofPoolsLimit = 0;


    /* some sanity checks first */
    if (blockSizeStep == 0 ||
        stepsLimit == 0 ||
        poolsLimit == 0 ||
        (poolsLimit < stepsLimit) ||
        ((stepsLimit % blockSizeStep) != 0 ) ||
        (mathLog2(stepsLimit,&log2ofStepsLimit) != GT_OK) ||
        (mathLog2(poolsLimit,&log2ofPoolsLimit) != GT_OK))

    {
        return GT_FAIL;
    }

    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(os_mem_poolsLimit,poolsLimit);

    /* Create the sync semaphore */
    if (osMutexCreate("osMemPools", &((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)))) != GT_OK)
    {
        return GT_FAIL;
    }
    osMutexLock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));

    /* calculate the amount of pools we need */
    os_mem_pools_array_size = (stepsLimit / blockSizeStep) +
        (log2ofPoolsLimit - log2ofStepsLimit) + 1 ;

    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(os_mem_pools_array_size,os_mem_pools_array_size);

    os_mem_pools_array =
        localMemAllocFuncPtr(sizeof(OSMEM_POOL_HDR_PTR) * (os_mem_pools_array_size + 1));

    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(os_mem_pools_array,os_mem_pools_array);

    if (os_mem_pools_array == NULL)
    {
        osMutexUnlock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
        osMutexDelete((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
        PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(os_mem_pools_mtx_id,(GT_MUTEX)0);
        return GT_FAIL;
    }



    curSize = blockSizeStep;
    for (i = 0; i < os_mem_pools_array_size - 1; ++i)
    {
        os_mem_pools_array[i] = osMemPoolCreate(curSize,i);
        if (os_mem_pools_array[i] == NULL)
        {
            for (j = 0; (j < i); j++)
            {
                localMemFreeFuncPtr(os_mem_pools_array[j]);
            }
            localMemFreeFuncPtr(os_mem_pools_array);
            os_mem_pools_array = NULL;
            PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(os_mem_pools_array,os_mem_pools_array);
            osMutexUnlock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
            osMutexDelete((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
            PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(os_mem_pools_mtx_id,(GT_MUTEX)0);
            return GT_FAIL;
        }

        if (curSize >= stepsLimit)
        {
            curSize *= 2;
        }
        else
        {
            curSize += blockSizeStep;
        }
    }

    /* now build the variuos sizes pool */
    os_mem_pools_array[i] = osMemPoolCreate(0,i);
    if (os_mem_pools_array[i] == NULL)
    {
        for (j = 0; (j < i); j++)
        {
            localMemFreeFuncPtr(os_mem_pools_array[j]);
        }
        localMemFreeFuncPtr(os_mem_pools_array);
        os_mem_pools_array = NULL;
        PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(os_mem_pools_array,os_mem_pools_array);
        osMutexUnlock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
        osMutexDelete((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
        PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(os_mem_pools_mtx_id,(GT_MUTEX)0);
        return GT_FAIL;
    }

    osMutexUnlock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));

    return GT_OK;
}

/*******************************************************************************
* osMemPoolCreate
*
* DESCRIPTION:
*      creates a memory pool for malloc managment
*
* INPUTS:
*       elemSize - the size of elements to be allocated from this pool.
*                  in case the size = 0 then this is a various sizes pool.
*       poolIndex - the index of the pool in the pool array.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       the pool id .
*
* COMMENTS:
*       None
*
*******************************************************************************/
static OSMEM_POOL_HDR *osMemPoolCreate
(
    IN GT_U32 elemSize,
    IN GT_U32 poolIndex
)
{
    OSMEM_POOL_HDR *poolHdrPtr;
    OSMEM_CHUNK_ARRAY * osMemChunksArrayPtr;
    GT_MEMORY_ALLOC_FUNC localMemAllocFuncPtr;
    poolHdrPtr = NULL;
    osMemChunksArrayPtr = &(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_chunks_array));


     localMemAllocFuncPtr = osMemChunksArrayPtr->memAllocFuncPtr;
#ifdef SHARED_MEMORY
     localMemAllocFuncPtr = (GT_MEMORY_ALLOC_FUNC)SHARED_MALLOC_MAC;
#endif

    poolHdrPtr = localMemAllocFuncPtr(sizeof(OSMEM_POOL_HDR));

    if (poolHdrPtr == NULL)
    {
        return NULL;
    }

    poolHdrPtr->elem_size = elemSize;
    poolHdrPtr->first_free_PTR = NULL;
    poolHdrPtr->chunkIndex = 0;
    poolHdrPtr->poolIndex = poolIndex;

    poolHdrPtr->free_size = 0;
    poolHdrPtr->size = 0;
    poolHdrPtr->overAlloc = 0;
    poolHdrPtr->alloc_times = 0;
    poolHdrPtr->malloc_used = 0;
    poolHdrPtr->free_times = 0;

    return poolHdrPtr;
}

/**
* @internal osMemPoolsSize2index
* @endinternal
*
* @brief   finds index of pool by size of memory being allocated

* @param[in] size            - size of allocated memory (in bytes).
*
* @retval found pool index or 0xFFFFFFFF on error.
*/
static GT_U32 osMemPoolsSize2index
(
    IN GT_U32 size
)
{
    GT_U32 high, low, middle, midSize;
    OSMEM_POOL_HDR_PTR* os_mem_pools_array;

    /* the last pool dedicated to various sized elements and has elem_size==0 */
    high = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_array_size) - 2;
    low = 0;

    os_mem_pools_array = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_array);

    if (size > os_mem_pools_array[high]->elem_size)
    {
        return 0xFFFFFFFF;
    }
    if (size <= os_mem_pools_array[0]->elem_size)
    {
        return 0;
    }
    while ((high - low) > 1)
    {
        middle = (high + low) / 2;
        midSize = os_mem_pools_array[middle]->elem_size;
        if (size == midSize)
        {
            return middle;
        }
        if (size < midSize)
        {
            high = middle;
        }
        else
        {
            low = middle;
        }
    }
    return high;
}

/*******************************************************************************
* osMemPoolsAlloc
*
* DESCRIPTION:
*       Allocates memory block of specified size.
*
* INPUTS:
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID *osMemPoolsAlloc
(
    IN GT_U32 size
)
{
    GT_VOID *memBlock;
    OSMEM_POOL_HDR_PTR curPool = NULL;
    GT_U32  allocSize;/*actual size that the memory manager gave the pointer */
    GT_U32 os_mem_pools_array_size;
    OSMEM_POOL_HDR_PTR* os_mem_pools_array;
    OSMEM_CHUNK_ARRAY * osMemChunksArrayPtr;
    GT_MEMORY_ALLOC_FUNC localMemAllocFuncPtr;

    osMemChunksArrayPtr = &(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_chunks_array));

    os_mem_pools_array = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_array);

     localMemAllocFuncPtr = osMemChunksArrayPtr->memAllocFuncPtr;
#ifdef SHARED_MEMORY
     localMemAllocFuncPtr = (GT_MEMORY_ALLOC_FUNC)SHARED_MALLOC_MAC;
#endif

    /* search through the pools for the appropriate size pool */
    if (size > PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_poolsLimit))
    {
        if ((size + sizeof (OSMEM_VAR_POOL_ELEM)) > osMemChunksArrayPtr->chunkSize)
        {
            OSMEM_NOPOOLS_HDR_STC *hdrPtr;
            hdrPtr = (OSMEM_NOPOOLS_HDR_STC*)localMemAllocFuncPtr(size+sizeof(OSMEM_NOPOOLS_HDR_STC));
            if (hdrPtr == NULL)
            {
                return NULL;
            }
            hdrPtr->magic = (GT_UINTPTR)hdrPtr;
            hdrPtr->size = size;
            hdrPtr++;
            return (GT_VOID*)hdrPtr;
        }
        os_mem_pools_array_size = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_array_size);
        os_mem_pools_array = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_array);

        curPool = os_mem_pools_array[os_mem_pools_array_size-1];
        /*NOTE: curPool->elem_size == 0 for this type of block */
        allocSize = size;
    }
    else
    {
        curPool = os_mem_pools_array[osMemPoolsSize2index(size)];
        allocSize = curPool->elem_size;
    }

    osMutexLock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));

    /* now allocate the pointer */
    if (curPool->elem_size == 0)
    {
        memBlock = osMemPoolAllocVarSize(curPool,size, &allocSize);
    }
    else
    {
        memBlock = osMemPoolAllocSize(curPool,size);
    }

    /* heap allocation byte counter correction */
    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(heap_bytes_allocated,
                                        PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(heap_bytes_allocated)+allocSize);

    PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(allocCounter,
                                        PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(allocCounter)+allocSize);

    PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(tempAllocCounter,
                                        PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(tempAllocCounter)+allocSize);
    if (PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(tempAllocCounter) > PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(maxAllocCounter))
    {
        PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(maxAllocCounter,
                                        PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(tempAllocCounter));
    }

    osMutexUnlock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));

    return memBlock;
}

/*******************************************************************************
* osMemPoolAllocSize
*
* DESCRIPTION:
*      allocates a pointer from a memory pool (for sizes bigger than
*      poolsLimit - see osMemPoolsInit).
*
* INPUTS:
*       poolId - the pool from which to allocate.
*       size   - the size to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       the pointer to the newly allocated data.
*
* COMMENTS:
*       None
*
*******************************************************************************/
static GT_VOID* osMemPoolAllocSize
(
    IN OSMEM_POOL_HDR_PTR poolHdrPtr,
    IN GT_U32             size
)
{
    OSMEM_POOL_FREE_ELEM_PTR freePtr = NULL;
    OSMEM_POOL_ELEM_PTR allocPtr = NULL;

    poolHdrPtr->alloc_times++;
    /* if there is a free element in the free list take from there */

    if (poolHdrPtr->first_free_PTR != NULL)
    {
        freePtr = poolHdrPtr->first_free_PTR;

        /* now link out the pieces */
        poolHdrPtr->first_free_PTR = freePtr->next_elem;

        poolHdrPtr->free_size  = poolHdrPtr->free_size -
                                 poolHdrPtr->elem_size;
    }


    allocPtr = (OSMEM_POOL_ELEM_PTR)freePtr;
    /* if we could find any elements in the free list -> take a new element
       from the chunk */

    if (allocPtr == NULL)
    {
        poolHdrPtr->overAlloc += poolHdrPtr->elem_size - size;

        allocPtr = osMemAllocDynamicFromChunk(
            sizeof(OSMEM_POOL_ELEM) + poolHdrPtr->elem_size,
            &poolHdrPtr->chunkIndex);

        if (allocPtr == NULL)
        {
            return NULL;
        }

        poolHdrPtr->malloc_used++;

        poolHdrPtr->size += poolHdrPtr->elem_size;
    }

    /* this variable consists the required allocation size  */
    allocPtr->poolIndex = (GT_UINTPTR)(poolHdrPtr->poolIndex);

    /* return a pointer to the data segment only */
    allocPtr++;
    return (allocPtr);

}

/*******************************************************************************
* osMemPoolAllocVarSize
*
* DESCRIPTION:
*      allocates a pointer from a memory pool (for sizes bigger than
*      poolsLimit - see osMemPoolsInit).
*
* INPUTS:
*       poolId - the pool from which to allocate.
*       size   - the size to allocate
*
* OUTPUTS:
*       outSizePtr - (pointer to)aligned size
*
* RETURNS:
*       the pointer to the newly allocated data.
*
* COMMENTS:
*       None
*
*******************************************************************************/
static GT_VOID* osMemPoolAllocVarSize
(
    IN OSMEM_POOL_HDR_PTR poolHdrPtr,
    IN GT_U32             size,
    OUT GT_U32           *outSizePtr
)
{
    OSMEM_POOL_FREE_ELEM_PTR freePtr = NULL;
    OSMEM_POOL_FREE_ELEM_PTR prvFreePtr = NULL;
    OSMEM_POOL_FREE_ELEM_PTR saveFreePtr;
    OSMEM_POOL_FREE_ELEM_PTR savePrvFreePtr;
    OSMEM_POOL_FREE_ELEM_PTR remainderFreePtr = NULL;

    OSMEM_VAR_POOL_ELEM_PTR allocVarPtr = NULL;
    GT_U32 allocSize = 0;
    GT_U32 sizeOfFreeHeader = sizeof(OSMEM_POOL_FREE_ELEM);
    GT_U32 minSplitSize = 16;
    GT_U32 os_mem_poolsLimit;
    OSMEM_CHUNK_ARRAY * osMemChunksArrayPtr;

    osMemChunksArrayPtr = &(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_chunks_array));

    poolHdrPtr->alloc_times++;
    /* if there is a free element in the free list take from there */

#ifdef OS_MEM_64_BIT_ALIGNED
    size = (size + 7) & 0xfffffff8; /* align 8 bytes */
#else
    size = (size + 3) & 0xfffffffc; /* align 4 bytes */
#endif
    *outSizePtr = size;
    if (poolHdrPtr->first_free_PTR != NULL)
    {
        /* look for element with size exact as required */
        freePtr = poolHdrPtr->first_free_PTR;
        prvFreePtr = NULL;
        saveFreePtr = NULL;
        savePrvFreePtr = NULL;
        while ((freePtr != NULL) && (freePtr->size != size))
        {
            if (freePtr->size > size)
            {
                if ((saveFreePtr == NULL) || (saveFreePtr->size > freePtr->size))
                {
                    saveFreePtr = freePtr;
                    savePrvFreePtr = prvFreePtr;
                }
            }
            prvFreePtr = freePtr;
            freePtr = freePtr->next_elem;
        }

        if (freePtr != NULL)
        {
            if (prvFreePtr == NULL)
            {
                poolHdrPtr->first_free_PTR = freePtr->next_elem;
            }
            else
            {
                prvFreePtr->next_elem = freePtr->next_elem;
            }

            poolHdrPtr->free_size  = poolHdrPtr->free_size - size;
        }
        else
        {
            /* look for element with size not less than required */
            freePtr = saveFreePtr;
            prvFreePtr = savePrvFreePtr;
            if (freePtr != NULL)
            {
                if (freePtr->size > (sizeOfFreeHeader + size + minSplitSize))
                {
                    /* element has place for additional header to split memory */
                    /* split it here to updated freePtr and remainderFreePtr   */
                    remainderFreePtr =
                        (OSMEM_POOL_FREE_ELEM_PTR)(((char*)freePtr) + sizeOfFreeHeader + size);
                    remainderFreePtr->next_elem = freePtr->next_elem;
                    /* less allocated size and additional free-element header */
                    remainderFreePtr->size = freePtr->size - (sizeOfFreeHeader + size);
                    freePtr->size = size;
                    freePtr->next_elem = remainderFreePtr;
                    poolHdrPtr->free_size -= sizeOfFreeHeader;
                }
                if (prvFreePtr == NULL)
                {
                    poolHdrPtr->first_free_PTR = freePtr->next_elem;
                }
                else
                {
                    prvFreePtr->next_elem = freePtr->next_elem;
                }
                poolHdrPtr->free_size  -= freePtr->size;
            }
        }
    }


    allocVarPtr = (OSMEM_VAR_POOL_ELEM_PTR)freePtr;
    /* if we couldn't find any elements in the free list -> take a new element
       from the chunk */

    if (allocVarPtr == NULL)
    {
        GT_U32 chunkIndex;
        allocSize = sizeof(OSMEM_VAR_POOL_ELEM) + size;

        chunkIndex = poolHdrPtr->chunkIndex;
        allocVarPtr = osMemAllocDynamicFromChunk(allocSize,
                                                 &chunkIndex);

        if (allocVarPtr == NULL)
        {
            return NULL;
        }
        os_mem_poolsLimit = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_poolsLimit);
        /* and as a precheck that the next element can or cannot be allocated from
           this chunk */
        while (poolHdrPtr->chunkIndex < osMemChunksArrayPtr->lastChunkIndex &&
            osMemChunksArrayPtr->chunkArray[poolHdrPtr->chunkIndex].freeSize <= os_mem_poolsLimit)
        {
            poolHdrPtr->chunkIndex++;
        }

        allocVarPtr->size = size;

        poolHdrPtr->malloc_used++;

        poolHdrPtr->size += size;
    }
    else
    {
        size = freePtr->size;
        allocVarPtr->size = size;
        *outSizePtr = size;
    }

    /* this variable consists the required allocation size  */
    /* allocVarPtr->poolIndex = size;  - it seems to be an error */
    /* last poolIndex - var size pool */
    allocVarPtr->poolIndex = (GT_UINTPTR)(poolHdrPtr->poolIndex);

    /* return a pointer to the data segment only */
    allocVarPtr++;
    return(allocVarPtr);

}

/*******************************************************************************
* osMemAllocDynamicFromChunk
*
* DESCRIPTION:
*       Allocates dynamic alloc memory from the given chunk.
*
* INPUTS:
*       size - the size to allocate.
*       chunkIndex - the index of the chunk from which to allocate.
*
* OUTPUTS:
*       chunkIndex - if the memory was allocated from a different chunk then the
*                    given one , then that chunk index is returned.
*
* RETURNS:
*       a pointer to the allocated memory.
*
* COMMENTS:
*       None
*
*******************************************************************************/
static GT_VOID *osMemAllocDynamicFromChunk
(
    IN    GT_U32 size,
    INOUT GT_U32 *chunkIndex
)
{
    GT_VOID *retPtr = NULL;
    OSMEM_CHUNK_POINTER_PTR curChunk;
    OSMEM_CHUNK_ARRAY * osMemChunksArrayPtr;

    osMemChunksArrayPtr = &(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_chunks_array));

#ifdef OS_MEM_64_BIT_ALIGNED
    size = (size + 7) & 0xfffffff8;
#endif
    /* start by using the given chunk */
    curChunk = &(osMemChunksArrayPtr->chunkArray[*chunkIndex]);

    /* now look for the first allocated chunk with enough memory (could be the
       given one) */
    while (*chunkIndex < osMemChunksArrayPtr->lastChunkIndex &&
           curChunk->dynamicAddr + size > curChunk->staticAddr)
    {
        (*chunkIndex)++;
        curChunk = &(osMemChunksArrayPtr->chunkArray[*chunkIndex]);
    }

    /* if none of the allocated ones hasn't got enough memory allocated a new
       one (if possible) */
    if (*chunkIndex >= osMemChunksArrayPtr->lastChunkIndex)
    {
        /* check if allowed to allocate more? */
        if (osMemChunksArrayPtr->memAllocMethod ==
                    GT_MEMORY_MANAGE_METHOD_NO_REALLOC_ON_DEMAND_E)
        {
            /* we reached out of mem situation! */
            return NULL;
        }
        /* allocate another chunk */
        if (osMemChunkAllocate() != GT_OK)
        {
            return NULL;
        }
    }
    /* now take the dynamicAddr for the return ptr */
    retPtr = curChunk->dynamicAddr;
    /* and update the chunk */
    curChunk->dynamicAddr += size;
    curChunk->freeSize -= size;

    /* and as a precheck that the next element can or cannot be allocated from
       this chunk */
    if (curChunk->dynamicAddr + size > curChunk->staticAddr)
    {
        (*chunkIndex)++;
    }

    return retPtr;
}

/**
* @internal osMemPoolFree function
* @endinternal
*
* @brief   frees a pointer from a memory pool
*
* @param[in] memBlock                 - the pointer which was allocated.
*                                       none
*/
GT_VOID osMemPoolFree
(
    IN GT_VOID    *memBlock
)
{
    OSMEM_POOL_HDR_PTR poolHdrPtr;
    OSMEM_POOL_ELEM_PTR allocPtr;
    OSMEM_VAR_POOL_ELEM_PTR allocVarPtr;
    OSMEM_POOL_FREE_ELEM_PTR freePtr;
    OSMEM_POOL_FREE_ELEM_PTR continueFreePtr;
    OSMEM_POOL_FREE_ELEM_PTR prevFreePtr;
    OSMEM_POOL_FREE_ELEM_PTR prefixMergedPtr;
    OSMEM_POOL_FREE_ELEM_PTR postfixPtr;
    OSMEM_POOL_FREE_ELEM_PTR postfixStartPtr;
    OSMEM_POOL_FREE_ELEM_PTR postfixPrevPtr;
    OSMEM_POOL_FREE_ELEM_PTR workFreePtr;
    GT_U32 sizeOfFreeHeader = sizeof(OSMEM_POOL_FREE_ELEM);
    GT_U32 freeSize = 0;
    GT_U32 os_mem_pools_array_size;
    OSMEM_POOL_HDR_PTR* os_mem_pools_array;
    OSMEM_CHUNK_ARRAY * osMemChunksArrayPtr;
    GT_MEMORY_FREE_FUNC  localMemFreeFuncPtr;

    osMemChunksArrayPtr = &(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_chunks_array));

    localMemFreeFuncPtr = osMemChunksArrayPtr->memFreeFuncPtr;
#ifdef SHARED_MEMORY
    localMemFreeFuncPtr = SHARED_FREE_MAC;
#endif

    osMutexLock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
#ifdef  CHECK_ADDRESS_ON_FREE
    checkMemoryAllocatedFromChunks(memBlock);
#endif

    /* Note: the structure in memory just before *memBlock can be:
        1) OSMEM_POOL_ELEM_PTR      (4 bytes:              GT_U32 poolIndex)
        2) OSMEM_VAR_POOL_ELEM_PTR  (8 bytes: GT_U32 size, GT_U32 poolIndex) */
    allocPtr = (OSMEM_POOL_ELEM_PTR)((GT_VOID*)((GT_UINTPTR) memBlock - sizeof(OSMEM_POOL_ELEM)));

    os_mem_pools_array_size = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_array_size);
    if (allocPtr->poolIndex > (GT_UINTPTR)os_mem_pools_array_size)
    {   /* out of range poolIndex */
        if (allocPtr->poolIndex == (GT_UINTPTR)POOL_INDEX_STATIC_CNS)
        {
            osPrintf("Warning:Trying to free a osStaticMalloc pointer !!!\n");

            osMutexUnlock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
            return;
        }
        else
        {
            if(localMemFreeFuncPtr== NULL)
            {
                osPrintf("Warning: osMemPoolFree - invalid poolIndex, memory lost !!!\n");
            }
            else
            {
                /* it was allocated in osMemPoolsAlloc(...) for huge size by
                   os_mem_chunks_array.memAllocFuncPtr(...)
                */
                localMemFreeFuncPtr(memBlock);
            }

            osMutexUnlock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
            return;
        }
    }
    os_mem_pools_array = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_array);
    poolHdrPtr = os_mem_pools_array[allocPtr->poolIndex];

    if (poolHdrPtr->elem_size == 0)
    {
        /* various size pool */
        allocVarPtr = (OSMEM_VAR_POOL_ELEM_PTR)
                      ((GT_VOID*)((GT_UINTPTR)memBlock - sizeof(OSMEM_VAR_POOL_ELEM)));
        freeSize = (GT_U32)allocVarPtr->size;
        freePtr = (OSMEM_POOL_FREE_ELEM_PTR)allocVarPtr;
        freePtr->size = freeSize;
    }
    else
    {
        freePtr = (OSMEM_POOL_FREE_ELEM_PTR)allocPtr;
        freeSize = poolHdrPtr->elem_size;
    }

    if(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(heap_bytes_allocated) < freeSize)
    {
        osFatalError(OS_FATAL_WARNING, "osMemPoolFree(): bad management cause negative heap_bytes_allocated \n");
    }
    else
    {
        /* heap allocation byte counter correction */
        PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(heap_bytes_allocated,
            PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(heap_bytes_allocated)-freeSize);
    }

    poolHdrPtr->free_times++;

    /* now deal with the free list ***** */

    /* used only for variable sized pool, but relates to last pool structure update*/
    prefixMergedPtr = NULL;

    if (poolHdrPtr->elem_size == 0)
    {
        /* attempt to merge being free element to the element ends at the free element origin address */
        workFreePtr = poolHdrPtr->first_free_PTR;
        prevFreePtr = NULL;
        prefixMergedPtr = NULL;
        postfixPtr = NULL;
        postfixPrevPtr = NULL;
        postfixStartPtr =
            (OSMEM_POOL_FREE_ELEM_PTR)(((char *)freePtr) + sizeOfFreeHeader + freePtr->size);
        while (workFreePtr != NULL)
        {
            if (prefixMergedPtr == NULL)
            {
                continueFreePtr =
                    (OSMEM_POOL_FREE_ELEM_PTR)(((char *)workFreePtr) + sizeOfFreeHeader + workFreePtr->size);
                if (continueFreePtr == freePtr)
                {
                    prefixMergedPtr = workFreePtr;
                    workFreePtr->size += sizeOfFreeHeader + freePtr->size;
                    poolHdrPtr->free_size += sizeOfFreeHeader;
                }
            }
            if (workFreePtr == postfixStartPtr)
            {
                postfixPtr = workFreePtr;
                postfixPrevPtr = prevFreePtr;
            }
            if ((prefixMergedPtr != NULL) && (postfixPtr != NULL))
            {
                break;
            }
            prevFreePtr = workFreePtr;
            workFreePtr = workFreePtr->next_elem;
        }

        /* attempt to merge being free element to the element continuing it */
        if (prefixMergedPtr != NULL)
        {
            freePtr = prefixMergedPtr;
        }
        /* merge postfix to freePtr */
        if (postfixPtr != NULL)
        {
            if (poolHdrPtr->first_free_PTR == postfixPtr)
            {
                poolHdrPtr->first_free_PTR = postfixPtr->next_elem;
            }
            if (postfixPrevPtr != NULL)
            {
                postfixPrevPtr->next_elem = postfixPtr->next_elem;
            }
            freePtr->size += (sizeOfFreeHeader + postfixPtr->size);
            poolHdrPtr->free_size += sizeOfFreeHeader;
        }
    }

    if (prefixMergedPtr == NULL)
    {
        /* only if the freed memory not added to prefix memory */
        freePtr->next_elem = poolHdrPtr->first_free_PTR;
        poolHdrPtr->first_free_PTR = freePtr;
    }

    /* just freeSize, not freePtr->size, because freePtr can be updated to prefixMergedPtr */
    poolHdrPtr->free_size = poolHdrPtr->free_size + freeSize;
    osMutexUnlock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));

    PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(freeCounter,
                                                PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(freeCounter)+freeSize);

    PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(tempAllocCounter,
                                                PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(tempAllocCounter)-freeSize);
    return;
}

/**
* @internal osMemPoolClear function
* @endinternal
*
* @brief   returns all the allocated memory from this pool to free.
*
* @retval GT_OK                    - if success.
*/
static GT_STATUS osMemPoolClear
(
    IN OSMEM_POOL_HDR_PTR poolHdrPtr
)
{
    poolHdrPtr->first_free_PTR = NULL;
    poolHdrPtr->free_size = 0;
    poolHdrPtr->size = 0;
    poolHdrPtr->chunkIndex = 0;

    return GT_OK;
}

static GT_8 formatUnit(GT_32 num)
{
    if (num >=  1000000000) {
        return 'G';
    }
    if (num >= 1000000) {
        return 'M';
    }
    if (num >= 1000) {
        return 'K';
    }
    return ' ';
}

static GT_FLOAT32 formatNumber(GT_32 num)
{
    GT_FLOAT32 floatNumber = num;

    if (num >=  1000000000) {
        return (floatNumber / 1073741824);
    }
    if (num >= 1000000) {
        return (floatNumber / 1048576);
    }
    if (num >= 1000) {
        return (floatNumber / 1024);
    }
    return num;
}


void osMemInTestAllocStat(GT_VOID)
{
    osPrintf("\nosMemInTestAlloc status\n");
    osPrintf("-------------------------\n");
    osPrintf("Maximum Allocation in a row in test: %.2f %cB\tTemporary Alloc in test: %.2f %cB \nTotal: Malloc: %.2f %cB  Realloc: %.2f %cB  Free: %.2f %cB\n\n",
    formatNumber(PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(maxAllocCounter)), formatUnit(PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(maxAllocCounter)),
    formatNumber(PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(tempAllocCounter)), formatUnit(PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(tempAllocCounter)),
    formatNumber(PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(allocCounter)), formatUnit(PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(allocCounter)),
    formatNumber(PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(reallocCounter)), formatUnit(PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(reallocCounter)),
    formatNumber(PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(freeCounter)), formatUnit(PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(freeCounter)));
    PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(maxAllocCounter,0);
    PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(tempAllocCounter,0);
    PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(allocCounter,0);
    PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(reallocCounter,0);
    PRV_NON_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(freeCounter,0);

}
/***** Debug ********************************************************/
#ifdef OSMEMPOOLS_DEBUG

/**
* @internal osMemPrintMemPoolsStat function
* @endinternal
*
* @brief   print statistics of all pools
*/
GT_VOID osMemPrintMemPoolsStat
(
    GT_VOID
)
{
    GT_U32 i;
    GT_U32 totalFree, totalAlloc, totalAllocTimes, totalFreeTimes;
    GT_U32 totalMallocUsed, totalOverhead, totalOverAlloc, dynamicSize, staticSize;
    GT_U32 totalStatic, totalDynamic,os_mem_pools_array_size;
    OSMEM_POOL_HDR_PTR* os_mem_pools_array;
    OSMEM_CHUNK_ARRAY * osMemChunksArrayPtr;

    osMemChunksArrayPtr = &(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_chunks_array));

    os_mem_pools_array = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_array);

    totalFree = 0;
    totalAlloc = 0;
    totalAllocTimes = 0;
    totalFreeTimes = 0;
    totalMallocUsed = 0;
    totalOverhead = 0;
    totalOverAlloc = 0;
    totalStatic = 0;
    totalDynamic = 0;

    osPrintf("\n\tosMemPools Status:\n");
    osPrintf("-----------------------------------------\n");
    osPrintf("Pool \t  Free\t      Used\t  Size\t    OverAlloc    OverHead   # of Allocs  # of Malloc  # of Frees  Reusage\n");
    osPrintf("=================================================================================================================\n");
    os_mem_pools_array_size = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_array_size);
    for (i = 0; i < os_mem_pools_array_size; ++i)
    {
        if (os_mem_pools_array[i]->size > 0)
        {
            osPrintf("%-7d%7.2f %cB  %7.2f %cB  %7.2f %cB  %7.2f %cB  %7.2f %cB   %-12d %-12d %-12d %d%\n",
                     os_mem_pools_array[i]->elem_size,
                     formatNumber(os_mem_pools_array[i]->free_size),
                     formatUnit(os_mem_pools_array[i]->free_size),
                     formatNumber((os_mem_pools_array[i]->size -
                     os_mem_pools_array[i]->free_size)),
                     formatUnit((os_mem_pools_array[i]->size -
                     os_mem_pools_array[i]->free_size)),
                     formatNumber(os_mem_pools_array[i]->size),
                     formatUnit(os_mem_pools_array[i]->size),
                     formatNumber(os_mem_pools_array[i]->overAlloc),
                     formatUnit(os_mem_pools_array[i]->overAlloc),
                     formatNumber(os_mem_pools_array[i]->malloc_used*
                     sizeof(OSMEM_POOL_ELEM)),
                     formatUnit(os_mem_pools_array[i]->malloc_used*
                     sizeof(OSMEM_POOL_ELEM)),
                     os_mem_pools_array[i]->alloc_times,
                     os_mem_pools_array[i]->malloc_used,
                     os_mem_pools_array[i]->free_times,
                     ((os_mem_pools_array[i]->alloc_times -
                      os_mem_pools_array[i]->malloc_used)*100) /
                     os_mem_pools_array[i]->alloc_times);
            totalFree += os_mem_pools_array[i]->free_size;
            totalAlloc += os_mem_pools_array[i]->size;
            totalAllocTimes += os_mem_pools_array[i]->alloc_times;
            totalFreeTimes += os_mem_pools_array[i]->free_times;
            totalMallocUsed += os_mem_pools_array[i]->malloc_used;
            totalOverhead += os_mem_pools_array[i]->malloc_used*
                     sizeof(OSMEM_POOL_ELEM);
            totalOverAlloc += os_mem_pools_array[i]->overAlloc;
        }
    }

    osPrintf("------------------------------------------------------------------------------------------------------------------\n");
    osPrintf("Total  %7.2f %cB  %7.2f %cB  %7.2f %cB  %7.2f %cB  %7.2f %cB   %-12d %-12d %-12d %d%\n",
            formatNumber(totalFree),formatUnit(totalFree),formatNumber(totalAlloc - totalFree),
            formatUnit(totalAlloc - totalFree), formatNumber(totalAlloc), formatUnit(totalAlloc),
            formatNumber(totalOverAlloc), formatUnit(totalOverAlloc),
            formatNumber(totalOverhead),formatUnit(totalOverhead),
            totalAllocTimes,totalMallocUsed, totalFreeTimes,
            ((totalAllocTimes - totalMallocUsed)*100) / totalAllocTimes);

    osPrintf("\nTable fields legend:\n");
    osPrintf("\tPool - Size of elements in pool.\n");
    osPrintf("\t       Example: Pool - 4 will be used for allocation requests <= 4\n");
    osPrintf("\t(Pool 0 - Variable size pool for large allocation > 65 KB)\n");
    osPrintf("\tFree - Number of elements that have been freed and not reclaimed.\n");
    osPrintf("\tUsed - Total memory occupied by used elements in pool (including overhead)\n");
    osPrintf("\tSize - Total memory occupied by all elements (including free elements + overhead)\n");
    osPrintf("\tOverAlloc - Summary of all diffs (element size - requested size) in a pool.\n");
    osPrintf("\t            Example: requested: 6, Pool element: 8, OverAlloc is 2.\n");
    osPrintf("\tOverHead - Ammount of memory used by element header (header size is 2 x ptr size)\n");
    osPrintf("\t# of Allocs - Number of calls for osMalloc().\n");
    osPrintf("\t# of Malloc - Number of currently allocated elements.\n");
    osPrintf("\t# of Frees - Number of calls to osFree().\n");
    osPrintf("\tReusage - (# of reclaimed elements) / # of Allocs => indication of the pool utilization by the application.\n");

    totalFree = 0;
    totalAlloc = 0;
    for (i = 0; i < osMemChunksArrayPtr->lastChunkIndex; ++i)
    {
        dynamicSize = (GT_U32)(osMemChunksArrayPtr->chunkArray[i].dynamicAddr -
                      osMemChunksArrayPtr->chunkArray[i].startAddr);
        totalDynamic += dynamicSize;
        totalFree += osMemChunksArrayPtr->chunkArray[i].freeSize;
        totalAlloc += osMemChunksArrayPtr->chunkSize;
        staticSize = osMemChunksArrayPtr->chunkSize -
                     (GT_U32)(osMemChunksArrayPtr->chunkArray[i].staticAddr -
        osMemChunksArrayPtr->chunkArray[i].startAddr);
        totalStatic += staticSize;

    }
        osPrintf("\n  Total Statistics for Chunks:\n");
        osPrintf("--------------------------------\n");
        osPrintf("Size of every Chunk: %.2f %cB   Number of Chunks: %d   Dynamic: %.2f %cB   Free: %.2f %cB   Static: %.2f %cB\n\n",
                  formatNumber(osMemChunksArrayPtr->chunkSize), formatUnit(osMemChunksArrayPtr->chunkSize),
                  osMemChunksArrayPtr->lastChunkIndex,
                  formatNumber(totalDynamic),formatUnit(totalDynamic),
                  formatNumber(totalFree), formatUnit(totalFree),
                  formatNumber(totalStatic), formatUnit(totalStatic));

    return;
}

/**
* @internal osMemPrintMemDelta function
* @endinternal
*
* @brief   print memory delta
*/
GT_VOID osMemPrintMemDelta
(
    GT_U32* dyn,
    GT_U32 *dynOver,
    GT_U32 *staticAlloc
)
{
    GT_U32 i;
    GT_U32 totalAlloc;
    GT_U32 totalOverhead, staticSize;
    GT_U32 totalStatic,os_mem_pools_array_size;
    OSMEM_POOL_HDR_PTR* os_mem_pools_array;
    OSMEM_CHUNK_ARRAY * osMemChunksArrayPtr;

    osMemChunksArrayPtr = &(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_chunks_array));

    totalAlloc = 0;
    totalOverhead = 0;
    totalStatic = 0;

    os_mem_pools_array_size = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_array_size);
    os_mem_pools_array = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_array);

    for (i = 0; i < os_mem_pools_array_size; ++i)
    {
        if (os_mem_pools_array[i]->size > 0)
        {
            totalAlloc += os_mem_pools_array[i]->size - os_mem_pools_array[i]->free_size;
            totalOverhead += os_mem_pools_array[i]->malloc_used*
                     sizeof(OSMEM_POOL_ELEM);
        }
    }

   for (i = 0; i < osMemChunksArrayPtr->lastChunkIndex; ++i)
   {
       staticSize = osMemChunksArrayPtr->chunkSize -
                    (GT_U32)(osMemChunksArrayPtr->chunkArray[i].staticAddr -
                     osMemChunksArrayPtr->chunkArray[i].startAddr);
       totalStatic += staticSize;
   }
   /*osPrintf("\nDelta: Dynamic : %d dynamic+overhead : %d Static: %d \n",
            totalAlloc - countDynamic,
            totalAlloc+totalOverhead - countDynamicOver,
            totalStatic - countStatic);*/

    if ((dyn != NULL) &&
        (dynOver != NULL) &&
        (staticAlloc != NULL))
    {
        *dyn += totalAlloc - PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(countDynamic);
        *dynOver += totalAlloc+totalOverhead - PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(countDynamicOver);
        *staticAlloc += totalStatic - PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(countStatic);
    }

   PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(countDynamic,totalAlloc);
   PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(countStatic,totalStatic);
   PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(countDynamicOver,totalAlloc+totalOverhead);

    return;
}

/**
* @internal osMemGetHeapBytesAllocated function
* @endinternal
*
* @brief   returns the number of bytes allocated in heap
*/
GT_U32 osMemGetHeapBytesAllocated
(
    GT_VOID
)
{
    return PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(heap_bytes_allocated);
}

/**
* @internal osMemStartHeapAllocationCounter function
* @endinternal
*
* @brief   set the current value of heap allocated bytes to the allocation counter
*/
GT_VOID osMemStartHeapAllocationCounter
(
    GT_VOID
)
{
    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(heap_start_counter_value,PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(heap_bytes_allocated));
}

/**
* @internal osMemGetHeapAllocationCounter function
* @endinternal
*
* @brief   returns the delta of current allocated bytes number and the value of allocation counter set by
*         preveous startHeapAllocationCounter() function
*/
GT_U32 osMemGetHeapAllocationCounter
(
    GT_VOID
)
{
    return PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(heap_bytes_allocated) - PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(heap_start_counter_value);
}


/*******************************************************************************
* osDbgMallocWithDebugInfo
*
* DESCRIPTION:
*       osDbgMallocWithDebugInfo replace osDbgMalloc to get debug info
*
* INPUTS:
*       size - bytes to allocate
*       fileNamePtr - (pointer to)the file name (or full path name) of the source code calling
*                   this function. usually used as __FILE__
*                  when NULL - ignored
*       line    - line number in the source code file. usually used as __LINE__
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*
* COMMENTS:
*       None
*
*******************************************************************************/
void *osDbgMallocWithDebugInfo
(
    IN GT_U32 size,
    IN const char*    fileNamePtr,
    IN GT_U32   line
)
{
    void *ptr;

    GT_UNUSED_PARAM(fileNamePtr);
    GT_UNUSED_PARAM(line);

    if (PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(mallocFailEnable) == GT_TRUE)
    {
        PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(mallocCounter,
                                                                PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(mallocCounter)+1);
        if (PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(mallocFailCounterLimit) > 0)
        {
            if (PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(mallocCounter) >= PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(mallocFailCounterLimit))
            {
                PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(mallocCounter,0);
                PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(mallocFailCounterLimit,0);
                return NULL;
            }
        }

    }

#ifdef INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG
    ptr = osMalloc_MemoryLeakageDbg(size,fileNamePtr,line);
#else /*INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG*/
    ptr = osMalloc(size);
#endif /*INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG*/
    return ptr;
}

/*******************************************************************************
* osDbgMalloc
*
* DESCRIPTION:
*       Allocates memory block of specified size or fail allocation
*       in accordance with checked flags
*
* INPUTS:
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*
* COMMENTS:
*       None
*
*******************************************************************************/
void *osDbgMalloc
(
    IN GT_U32 size
)
{
    return osDbgMallocWithDebugInfo(size,LOCAL_DEBUG_INFO);
}



/**
* @internal osDbgMallocFailSet function
* @endinternal
*
* @brief   Allocates memory block of specified size or fail allocation
*         in accordance with checked flags
*
* @retval GT_OK                    - on success
*/
GT_STATUS osDbgMallocFailSet
(
    IN GT_BOOL                  failEnable,
    IN GT_U32                   failCounter
)
{
    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(mallocCounter,0);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(mallocFailEnable,failEnable);
    if (failEnable == GT_TRUE)
    {
        PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(mallocFailCounterLimit,failCounter);
    }
    else
    {
        PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(mallocFailCounterLimit,0);
    }
    return GT_OK;
}


/**
* @internal osMemGetMallocAllocationCounter function
* @endinternal
*
* @brief   Get malloc allocation counter
*
* @param[in] allocationNumberCounterPtr - pointer to number of malloc allocations
*
* @retval GT_OK                    - on success
*/
GT_STATUS osMemGetMallocAllocationCounter
(
    IN GT_U32 *allocationNumberCounterPtr
)
{
    *allocationNumberCounterPtr = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(mallocCounter);
    return GT_OK;
}

#endif /*OSMEMPOOLS_DEBUG*/

/**
* @internal dmaRegisterPtr function
* @endinternal
*
* @brief   register pointer that was returned by osCacheDmaMalloc(...)
*
* @param[in] allocPtr                 - allocated pointer by osCacheDmaMalloc
* @param[in] numOfBytes               - number of bytes to allocated by osCacheDmaMalloc
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on CPU memory run out (malloc)
*
* @note function under locking of os_mem_pools_mtx_id
*
*/
static GT_STATUS dmaRegisterPtr(
    IN GT_VOID *    allocPtr,
    IN GT_U32       numOfBytes
)
{
    DMA_ALLOCATION_STC *nextElementPtr = NULL;

    DMA_ALLOCATION_STC *currentPtr;

    /* find empty element */
    {
        currentPtr = (PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalDmaAllocationsFirstPtr));

        while(currentPtr)
        {
            if(currentPtr->myMemoryPtr == NULL)
            {
                /* empty place */
                nextElementPtr = currentPtr;
                break;
            }

            /* update the 'current' to be 'next' */
            currentPtr = currentPtr->nextElementPtr;
        }

    }


    if(nextElementPtr == NULL)
    {
        /* new allocation so need new 'element' */
        nextElementPtr = DBG_MALLOC_MAC(sizeof(*nextElementPtr));
        if(nextElementPtr == NULL)
        {
            return GT_OUT_OF_CPU_MEM;
        }

        if(NULL == PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalDmaAllocationsLastPtr))
        {
             PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalDmaAllocationsLastPtr)= nextElementPtr;
             PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalDmaAllocationsFirstPtr) = nextElementPtr;
        }
        else
        {

            /* bind the previous element to the new element */
            PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalDmaAllocationsLastPtr)->nextElementPtr = nextElementPtr;


            /* update last element pointer to the newly allocated element*/
            PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalDmaAllocationsLastPtr) = nextElementPtr;
         }
         /* terminate the link list */
         nextElementPtr->nextElementPtr = NULL;
    }

    nextElementPtr->myMagic = MAGIC_NUMBER_CNS;
    nextElementPtr->myMemoryPtr    = allocPtr;  /*save the DMA pointer */
    nextElementPtr->myMemoryNumOfBytes = numOfBytes;

#if ERROR_IN_IMPLEMENTATION == 0
    if(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(dmaOverflowDetectionEnabled) == GT_TRUE)
    {
        GT_U32  ii,jj;
        GT_U32  *startOfOverflowDetectionRegionPtr;

        if(numOfBytes <= DMA_NUM_BYTES_OVERFLOW_DETECTION_REGION_CNS)
        {
            /* should not happen */
            return GT_ERROR;
        }

        /* the pointer and numOfBytes holds the 'DMA_NUM_BYTES_OVERFLOW_DETECTION_REGION_CNS'
           for the 'overflow detection' region at the end of each 'Cached DMA' .

            we need to initialize it with the pre-defined values.
        */

        startOfOverflowDetectionRegionPtr = (GT_U32*)((GT_CHAR*)allocPtr + (numOfBytes - DMA_NUM_BYTES_OVERFLOW_DETECTION_REGION_CNS));

        for(ii = 0 ; ii < DMA_NUM_LINES_OF_PRE_DEFINED_PATTERNS ; ii++)
        {
            for(jj = 0 ; jj < DMA_NUM_PRE_DEFINED_PATTERNS_CNS; jj++)
            {
                (*startOfOverflowDetectionRegionPtr)= dmaPreDefinedPatternsArr[jj];
                startOfOverflowDetectionRegionPtr ++;
            }
        }
    }
#endif /*ERROR_IN_IMPLEMENTATION == 0*/


    return GT_OK;
}

/**
* @internal osCacheDmaRegisterPtr function
* @endinternal
*
* @brief   register pointer that was returned by osCacheDmaMalloc(...)
*
* @param[in] allocPtr                 - allocated pointer by osCacheDmaMalloc
* @param[in] numOfBytes               - number of bytes to allocated by osCacheDmaMalloc
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on CPU memory run out (malloc)
*/
GT_STATUS osCacheDmaRegisterPtr(
    IN GT_VOID *    allocPtr,
    IN GT_U32       numOfBytes
)
{
    GT_STATUS   rc;
    osMutexLock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
    rc = dmaRegisterPtr(allocPtr,numOfBytes);
    if(rc == GT_OK)
    {
        PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(dmaTotalAlloc,
                                                                PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(dmaTotalAlloc)+numOfBytes);
        if(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(dmaTotalAlloc_traceEnable))
        {
            prvOsDmaTotalAllocPrint();
        }
    }
    osMutexUnlock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));

    return rc;
}

/*******************************************************************************
* dmaMemoryFind
*
* DESCRIPTION:
*        find element in DB according to it's DMA pointer
*
* INPUTS:
*       searchForPtr - DMA pointer to look for
*       numOfBytes - number of bytes to allocated by osCacheDmaMalloc
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the element (NULL - means 'not found')
* COMMENTS:
*       function under locking of os_mem_pools_mtx_id
*******************************************************************************/
static DMA_ALLOCATION_STC* dmaMemoryFind
(
    IN GT_PTR                   searchForPtr
)
{
    DMA_ALLOCATION_STC *currentPtr;

    currentPtr = (PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalDmaAllocationsFirstPtr));

    while(currentPtr)
    {
        if(currentPtr->myMemoryPtr == searchForPtr)
        {
            /* got match */
            return currentPtr;
        }

        /* update the 'current' to be 'next' */
        currentPtr = currentPtr->nextElementPtr;
    }

    return NULL;
}

/**
* @internal dmaUnregisterPtr function
* @endinternal
*
* @brief   unregister pointer that is free by osCacheDmaFree(...)
*
* @param[in] oldPointer               - pointer to unregister
*                                       None
*
* @note function under locking of os_mem_pools_mtx_id
*
*/
static void dmaUnregisterPtr(
    IN GT_VOID *    oldPointer
)
{
    DMA_ALLOCATION_STC *elementPtr = dmaMemoryFind(oldPointer);

    if(elementPtr == NULL)
    {
        /* not found the PTR to free */
        return ;
    }

    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(dmaTotalAlloc,
                            (PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(dmaTotalAlloc)-elementPtr->myMemoryNumOfBytes));
    if(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(dmaTotalAlloc_traceEnable))
    {
        prvOsDmaTotalAllocPrint();
    }

    /* NOTE that : elementPtr->myMemoryPtr == oldPointer !!! */

    /* oldPointer is free by osCacheDmaFree(...) */

    elementPtr->myMemoryPtr = NULL;/* indicate that DMA was free , but element
                                      still exists */
    elementPtr->myMemoryNumOfBytes = 0;

    if(elementPtr->myMagic != MAGIC_NUMBER_CNS)
    {
        /* ERROR ! */
        REPORT_CORRUPTED_MEMORY_MAC;
    }


    return;
}

/**
* @internal osCacheDmaUnregisterPtr function
* @endinternal
*
* @brief   unregister pointer that is free by osCacheDmaFree(...)
*
* @param[in] oldPointer               - pointer to unregister
*                                       None
*/
void osCacheDmaUnregisterPtr(
    IN GT_VOID *    oldPointer
)
{

    osMutexLock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
    if(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(underSystemReset))
    {
        /* we not want handling call from osCacheDmaSystemReset(...) to
            osCacheDmaFree(...) and to
            osCacheDmaUnregisterPtr(...)

            during changing the DB via osCacheDmaSystemReset
        */
    }
    else
    {
        dmaUnregisterPtr(oldPointer);
    }
    osMutexUnlock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
}


/**
* @internal dmaFreeAll function
* @endinternal
*
* @brief   free all DMA elements in the list and their DMA memory
*
* @note function under locking of os_mem_pools_mtx_id
*
*/
static GT_VOID dmaFreeAll(GT_VOID)
{
    DMA_ALLOCATION_STC *currentPtr;
    DMA_ALLOCATION_STC *nextPtr;

    currentPtr = (PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalDmaAllocationsFirstPtr));

    while(currentPtr)
    {
        nextPtr = currentPtr->nextElementPtr;
        /* free the DMA allocation needed by the device */
        if(currentPtr->myMemoryPtr)/* free only if not NULL , because maybe already released */
        {
            osCacheDmaFree(currentPtr->myMemoryPtr);
        }

        if(currentPtr->myMagic != MAGIC_NUMBER_CNS)
        {
            /* error detected !!! */
            REPORT_CORRUPTED_MEMORY_MAC;

            /* we CAN'T go on the link list !!! it is corrupted !!! */
            break;
        }

        /* free the element itself */
#ifdef SHARED_MEMORY
        if(DBG_FREE_MAC != NULL)
#endif
            DBG_FREE_MAC(currentPtr);

            if(currentPtr==(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalDmaAllocationsFirstPtr)))
            {
                PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(globalDmaAllocationsFirstPtr,NULL);
            }
            if(currentPtr==(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalDmaAllocationsLastPtr)))
            {
               PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(globalDmaAllocationsLastPtr,NULL);
            }


        /* update the 'current' to be 'next' */
        currentPtr = nextPtr;
    }

    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(dmaTotalAlloc,0);
    return ;
}

/**
* @internal osCacheDmaSystemReset function
* @endinternal
*
* @brief   free all DMA elements in the list and their DMA memory
*/
GT_STATUS osCacheDmaSystemReset(void)
{
    osMutexLock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(underSystemReset,1);
    dmaFreeAll();
    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(underSystemReset,0);
    osMutexUnlock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));

    return GT_OK;
}

/*******************************************************************************
* osDbgMalloc_MemoryLeakageDbg
*
* DESCRIPTION:
*       osDbgMalloc_MemoryLeakageDbg replace osDbgMalloc to get debug info
*
* INPUTS:
*       size - bytes to allocate
*       fileNamePtr - (pointer to)the file name (or full path name) of the source code calling
*                   this function. usually used as __FILE__
*                  when NULL - ignored
*       line    - line number in the source code file. usually used as __LINE__
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*
* COMMENTS:
*       None
*
*******************************************************************************/
void *osDbgMalloc_MemoryLeakageDbg
(
    IN GT_U32 size,
    IN const char*    fileNamePtr,
    IN GT_U32   line
)
{
    return osDbgMallocWithDebugInfo(size,fileNamePtr,line);
}

#ifdef INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG
/**
* @internal memRegisterPtr function
* @endinternal
*
* @brief   register pointer that was returned by osMalloc(...)/osRealloc(...)
*
* @param[in] allocPtr                 - allocated pointer by osMalloc(...)/osRealloc(...)
* @param[in] numOfBytes               - number of bytes to allocated by osMalloc(...)/osRealloc(...)
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on CPU memory run out (malloc)
*
* @note function under locking of os_mem_pools_mtx_id
*
*/
static GT_STATUS memRegisterPtr(
    IN GT_VOID *    allocPtr,
    IN GT_U32       numOfBytes,
    IN const char*    fileNamePtr,
    IN GT_U32   line
)
{
    MEM_ALLOCATION_STC *nextElementPtr = NULL;
    MEM_ALLOCATION_STC *currentPtr;

    /* find empty element */
    {
        currentPtr = &PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalMemAllocations);
        do
        {
            if(currentPtr->myMemoryPtr == NULL)
            {
                /* empty place */
                nextElementPtr = currentPtr;
                break;
            }

            /* update the 'current' to be 'next' */
            currentPtr = currentPtr->nextElementPtr;
        }while(currentPtr);
    }


    if(nextElementPtr == NULL)
    {
        /* new allocation so need new 'element' */
        nextElementPtr = DBG_MALLOC_MAC(sizeof(*nextElementPtr));
        if(allocPtr == NULL)
        {
            return GT_OUT_OF_CPU_MEM;
        }

        /* bind the previous element to the new element */
        PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalMemAllocationsLastPtr)->nextElementPtr = nextElementPtr;

        /* update last element pointer to the newly allocated element*/
        PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(globalMemAllocationsLastPtr,nextElementPtr);

        /* terminate the link list */
        nextElementPtr->nextElementPtr = NULL;
    }

    nextElementPtr->myMagic = MAGIC_NUMBER_CNS;
    nextElementPtr->myMemoryPtr    = allocPtr;  /*save the MEM pointer */
    nextElementPtr->myMemoryNumOfBytes = numOfBytes;
    /* register debug info about the caller */
    nextElementPtr->memoryLeakageDbgInfo.fileNamePtr = fileNamePtr;
    nextElementPtr->memoryLeakageDbgInfo.line = line;
    nextElementPtr->memoryLeakageDbgInfo.memoryLeakageState = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalMemoryLeakageState);

    return GT_OK;
}


/**
* @internal osMemRegisterPtr function
* @endinternal
*
* @brief   register pointer that was returned by osMalloc(...)/osRealloc(...)(...)
*
* @param[in] allocPtr                 - allocated pointer by osMemMalloc
* @param[in] numOfBytes               - number of bytes to allocated by osMalloc(...)/osRealloc(...)
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on CPU memory run out (malloc)
*/
GT_STATUS osMemRegisterPtr(
    IN GT_VOID *    allocPtr,
    IN GT_U32       numOfBytes,
    IN const char*    fileNamePtr,
    IN GT_U32   line
)
{
    GT_STATUS   rc;
    osMutexLock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
    rc = memRegisterPtr(allocPtr,numOfBytes,fileNamePtr,line);
    osMutexUnlock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));

    return rc;
}

/*******************************************************************************
* memMemoryFind
*
* DESCRIPTION:
*        find element in DB according to it's MEM pointer
*
* INPUTS:
*       searchForPtr - MEM pointer to look for
*       numOfBytes - number of bytes to allocated by osMalloc(...)/osRealloc(...)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the element (NULL - means 'not found')
* COMMENTS:
*       function under locking of os_mem_pools_mtx_id
*******************************************************************************/
static MEM_ALLOCATION_STC* memMemoryFind
(
    IN GT_PTR                   searchForPtr
)
{
    MEM_ALLOCATION_STC *currentPtr;

    currentPtr = &PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalMemAllocations);

    do
    {
        if(currentPtr->myMemoryPtr == searchForPtr)
        {
            /* got match */
            return currentPtr;
        }

        /* update the 'current' to be 'next' */
        currentPtr = currentPtr->nextElementPtr;
    }while(currentPtr);

    return NULL;
}

/**
* @internal memUnregisterPtr function
* @endinternal
*
* @brief   unregister pointer that is free by osMalloc(...)/osRealloc(...)
*
* @param[in] oldPointer               - pointer to unregister
*                                       None
*
* @note function under locking of os_mem_pools_mtx_id
*
*/
static void memUnregisterPtr(
    IN GT_VOID *    oldPointer,
    IN const char*    fileNamePtr,
    IN GT_U32   line
)
{
    OSMEM_POOL_ELEM_PTR allocPtr;
    MEM_ALLOCATION_STC *elementPtr = memMemoryFind(oldPointer);
    GT_U32 os_mem_pools_array_size;
    OSMEM_CHUNK_ARRAY * osMemChunksArrayPtr;
    GT_MEMORY_FREE_FUNC  localMemFreeFuncPtr;

    osMemChunksArrayPtr = &(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_chunks_array));

    if(oldPointer == NULL)
    {
        osPrintf("memUnregisterPtr : try to free 'NULL' pointer ?! called from file[%s] line[%d] \n",
            fileNamePtr,line);
        return ;
    }
    else
    if(elementPtr == NULL)
    {
        /* not found the PTR to free */
        osPrintf("memUnregisterPtr : try to free pointer that not allocated ?! called from file[%s] line[%d] \n",
            fileNamePtr,line);
        return ;
    }

    localMemFreeFuncPtr = osMemChunksArrayPtr->memFreeFuncPtr;
#ifdef SHARED_MEMORY
    localMemFreeFuncPtr = SHARED_FREE_MAC;
#endif

    /* logic from osMemPoolFree(...) about 'static' malloc */
    if(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(firstMemInit) == GT_FALSE)/* indication that osMemLibInit(...) was called */
    {
        allocPtr = (OSMEM_POOL_ELEM_PTR)((GT_VOID*)((GT_UINTPTR)oldPointer - sizeof(OSMEM_POOL_ELEM)));
        os_mem_pools_array_size = PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_array_size);
        if (allocPtr->poolIndex > os_mem_pools_array_size)
        {
            if (allocPtr->poolIndex == POOL_INDEX_STATIC_CNS)
            {
                /* function osMemPoolFree(...) did not free the memory */
                if(elementPtr->memoryLeakageDbgInfo.fileNamePtr)
                {
                    osPrintf("memUnregisterPtr : try to free [%d] bytes of 'static pointer' ?! \n"
                        "called from file[%s] line[%d] and allocated from file[%s] line[%d] \n",
                        elementPtr->myMemoryNumOfBytes,
                        fileNamePtr,line,
                        elementPtr->memoryLeakageDbgInfo.fileNamePtr,
                        elementPtr->memoryLeakageDbgInfo.line);
                }
                else
                {
                    osPrintf("memUnregisterPtr : try to free [%d] bytes of 'static pointer' ?! \n",
                        elementPtr->myMemoryNumOfBytes);
                }
            }
            else
            if(localMemFreeFuncPtr == NULL)
            {
                /* function osMemPoolFree(...) did not free the memory */
                if(elementPtr->memoryLeakageDbgInfo.fileNamePtr)
                {
                    osPrintf("memUnregisterPtr : try to free [%d] bytes of pointer without callback function?! \n"
                        "called from file[%s] line[%d] and allocated from file[%s] line[%d] \n",
                        elementPtr->myMemoryNumOfBytes,
                        fileNamePtr,line,
                        elementPtr->memoryLeakageDbgInfo.fileNamePtr,
                        elementPtr->memoryLeakageDbgInfo.line);
                }
                else
                {
                    osPrintf("memUnregisterPtr : try to free [%d] bytes of 'static pointer' ?! \n",
                        elementPtr->myMemoryNumOfBytes);
                }
            }
        }
    }

    if(elementPtr->myMagic != MAGIC_NUMBER_CNS)
    {
        /* error */
        REPORT_CORRUPTED_MEMORY_MAC;
    }

    /* NOTE that : elementPtr->myMemoryPtr == oldPointer !!! */

    /* oldPointer is free by osMemFree(...) */

    elementPtr->myMemoryPtr = NULL;/* indicate that MEM was free , but element
                                      still exists */
    elementPtr->myMemoryNumOfBytes = 0;

    elementPtr->memoryLeakageDbgInfo.fileNamePtr = NULL;
    elementPtr->memoryLeakageDbgInfo.line = 0;
    elementPtr->memoryLeakageDbgInfo.memoryLeakageState = OSMEM_MEMORY_LEAKAGE_STATE_DBG_OFF_E;

    return;
}

/**
* @internal osMemUnregisterPtr function
* @endinternal
*
* @brief   unregister pointer that is free by osMalloc(...)/osRealloc(...)
*
* @param[in] oldPointer               - pointer to unregister
*                                       None
*/
void osMemUnregisterPtr(
    IN GT_VOID *    oldPointer,
    IN const char*    fileNamePtr,
    IN GT_U32   line
)
{

    osMutexLock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
    memUnregisterPtr(oldPointer,fileNamePtr,line);
    osMutexUnlock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
}
#endif /*INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG*/

typedef enum{
    MEM_REGISTERED_ACTION_DUMP_STATUS_ON_E      =   (1<<0),         /* dump those with 'on'             */
    MEM_REGISTERED_ACTION_DUMP_STATUS_OFF_E     =   (1<<1),        /* dump those with 'off'            */
    MEM_REGISTERED_ACTION_DUMP_STATUS_ANY_E     =   (MEM_REGISTERED_ACTION_DUMP_STATUS_ON_E |
                                                     MEM_REGISTERED_ACTION_DUMP_STATUS_OFF_E),  /* dump all                         */
    MEM_REGISTERED_ACTION_SET_ALL_ON_TO_OFF_E   =   (1<<2),      /* change all with 'on' to 'off'    */

    MEM_REGISTERED_ACTION___LAST__E/* must be last --- not to be used */
}MEM_REGISTERED_ACTION_ENT;

/**
* @internal memRegisteredPtrAction function
* @endinternal
*
* @brief   actions on the pointers that not free by osFree
*
* @param[in] action                   - the type of action
*                                      dump those with 'on'
*                                      dump those with 'off'
*                                      dump all
*                                      change all with 'on' to 'off'
*                                       None
*
* @note function under locking of os_mem_pools_mtx_id
*
*/
GT_VOID memRegisteredPtrAction(
    IN MEM_REGISTERED_ACTION_ENT action
)
{
    MEM_ALLOCATION_STC *currentPtr;

    OSMEM_MEMORY_LEAKAGE_DBG_INFO *memoryLeakageDbgInfoPtr;
    GT_U32      sumNumBytes = 0,sumNumElements = 0;
    GT_U32      sumNumElementsWithoutFileName = 0;
    MEM_ALLOCATION_STC  *globalMemAllocationsPtr = &PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalMemAllocations);

    currentPtr = &PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalMemAllocations);

    if(globalMemAllocationsPtr->myMagic == 0 &&
       globalMemAllocationsPtr->myMemoryPtr == NULL &&
       globalMemAllocationsPtr->myMemoryNumOfBytes == 0)
    {
        globalMemAllocationsPtr->myMagic = MAGIC_NUMBER_CNS;
    }

    do
    {
        memoryLeakageDbgInfoPtr = &currentPtr->memoryLeakageDbgInfo;

        if(currentPtr->myMagic != MAGIC_NUMBER_CNS)
        {
            REPORT_CORRUPTED_MEMORY_MAC;
            /* we CAN'T go on the link list !!! it is corrupted !!! */
            break;
        }
        else
        if(currentPtr->myMemoryPtr && currentPtr->myMemoryNumOfBytes)
        {
            switch(action)
            {
                case MEM_REGISTERED_ACTION_DUMP_STATUS_ON_E:         /* dump those with 'on'             */
                case MEM_REGISTERED_ACTION_DUMP_STATUS_OFF_E:        /* dump those with 'off'            */
                case MEM_REGISTERED_ACTION_DUMP_STATUS_ANY_E:        /* dump all                         */
                    if(
                       (
                        /* print status on */
                        ((action & MEM_REGISTERED_ACTION_DUMP_STATUS_ON_E) &&
                        (memoryLeakageDbgInfoPtr->memoryLeakageState == OSMEM_MEMORY_LEAKAGE_STATE_DBG_ON_E))

                        ||
                        /* print status off */
                        ((action & MEM_REGISTERED_ACTION_DUMP_STATUS_OFF_E) &&
                        (memoryLeakageDbgInfoPtr->memoryLeakageState == OSMEM_MEMORY_LEAKAGE_STATE_DBG_OFF_E))
                        ))
                    {
                        sumNumBytes += currentPtr->myMemoryNumOfBytes;
                        sumNumElements ++;

                        if(memoryLeakageDbgInfoPtr->fileNamePtr)
                        {
                            osPrintf("not free memory that was alloc/realloc of [%d] bytes from file[%s] line[%d] \n"
                                ,currentPtr->myMemoryNumOfBytes
                                ,memoryLeakageDbgInfoPtr->fileNamePtr
                                ,memoryLeakageDbgInfoPtr->line
                                );

                            if((sumNumElements % 50))
                            {
                                /* let time for WIN32 to be able to print all without loss */
                                osTimerWkAfter(50);
                            }
                        }
                        else
                        {
                            /* no file name ?! */
                            sumNumElementsWithoutFileName++;
                        }
                    }

                break;
                case MEM_REGISTERED_ACTION_SET_ALL_ON_TO_OFF_E:      /* change all with 'on' to 'off'    */
                    memoryLeakageDbgInfoPtr->memoryLeakageState = OSMEM_MEMORY_LEAKAGE_STATE_DBG_OFF_E;
                break;
                default:
                    break;
            }
        }

        /* update the 'current' to be 'next' */
        currentPtr = currentPtr->nextElementPtr;
    }while(currentPtr);

    switch(action)
    {
        case MEM_REGISTERED_ACTION_DUMP_STATUS_ON_E:         /* dump those with 'on'             */
        case MEM_REGISTERED_ACTION_DUMP_STATUS_OFF_E:        /* dump those with 'off'            */
        case MEM_REGISTERED_ACTION_DUMP_STATUS_ANY_E:        /* dump all                         */
            if(sumNumBytes)
            {
                osPrintf("not free memory that was alloc/realloc total of [%d] bytes in [%d] elements \n",
                    sumNumBytes,sumNumElements);
                if(sumNumElementsWithoutFileName)
                {
                    osPrintf("[%d] elements without file name \n",
                        sumNumElementsWithoutFileName);
                }
            }
            break;
        default:
            break;
    }



    return ;
}

/**
* @internal osMallocMemoryLeakageDbgAction function
* @endinternal
*
* @brief   debug tool relate to 'Memory leakage detection'
*         relevant only when compilation with : OS_MALLOC_MEMORY_LEAKAGE_DBG
*         Set state of debug for memory leakage: start/stop.
* @param[in] memoryLeakageState       - state of debug for memory leakage:
*                                      start/stop.
*
* @retval GT_OK                    - on success
*
* @note use function osMallocMemoryLeakageDbgPrintStandingAllocated to dump all the
*       allocated pointer that are still allocated (not free) that were allocated
*       between last 'start' and the consecutive 'stop'
*
*/
GT_STATUS osMallocMemoryLeakageDbgAction
(
    IN OSMEM_MEMORY_LEAKAGE_STATE_DBG_ENT  memoryLeakageState
)
{
    if(memoryLeakageState != OSMEM_MEMORY_LEAKAGE_STATE_DBG_OFF_E &&
       memoryLeakageState != OSMEM_MEMORY_LEAKAGE_STATE_DBG_ON_E)
    {
        return GT_BAD_PARAM;
    }

    if(memoryLeakageState == OSMEM_MEMORY_LEAKAGE_STATE_DBG_ON_E &&
       PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalMemoryLeakageState) != memoryLeakageState)
    {
        /* we start new session of recording , so clear previous recording */
        osMutexLock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
        memRegisteredPtrAction(MEM_REGISTERED_ACTION_SET_ALL_ON_TO_OFF_E);
        osMutexUnlock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
    }

    /* set global mode */
    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(globalMemoryLeakageState,memoryLeakageState);

    return GT_OK;
}

/**
* @internal osMallocMemoryLeakageDbgPrintStandingAllocated function
* @endinternal
*
* @brief   debug tool relate to 'Memory leakage detection'
*         relevant only when compilation with : OS_MALLOC_MEMORY_LEAKAGE_DBG
*         print the info about the allocated pointers that were not free in the
*         time between 'start' and 'Stop'
*
* @retval GT_OK                    - on success
*/
GT_STATUS osMallocMemoryLeakageDbgPrintStandingAllocated
(
    GT_VOID
)
{
    osMutexLock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));
    memRegisteredPtrAction(MEM_REGISTERED_ACTION_DUMP_STATUS_ON_E);
    osMutexUnlock((PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(os_mem_pools_mtx_id)));

    return GT_OK;
}

#ifdef INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG
/*******************************************************************************
* osMalloc_MemoryLeakageDbg
*
* DESCRIPTION:
*       osMalloc_MemoryLeakageDbg replace osMalloc to get debug info
*
* INPUTS:
*       size - bytes to allocate
*       fileNamePtr - (pointer to)the file name (or full path name) of the source code calling
*                   this function. usually used as __FILE__
*                  when NULL - ignored
*       line    - line number in the source code file. usually used as __LINE__
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*
* COMMENTS:
*       Usage of this function is NEVER during initialization.
*
*******************************************************************************/
GT_VOID *osMalloc_MemoryLeakageDbg
(
    IN GT_U32   size,
    IN const char*    fileNamePtr,
    IN GT_U32   line
)
{
    void *ptr;
    ptr = internalOsMalloc(size);

    osMemRegisterPtr(ptr,size,fileNamePtr,line);
    return ptr;
}

/*******************************************************************************
*   osRealloc_MemoryLeakageDbg
*
* DESCRIPTION:
*       osRealloc_MemoryLeakageDbg replace osRealloc to get debug info
*
* INPUTS:
*       ptr  - pointer to previously allocated buffer
*       size - bytes to allocate
*       fileNamePtr - (pointer to)the file name (or full path name) of the source code calling
*                   this function. usually used as __FILE__
*                  when NULL - ignored
*       line    - line number in the source code file. usually used as __LINE__
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*
* COMMENTS:
*       Usage of this function is NEVER during initialization.
*       Recommended to avoid usage of this function.
*
*******************************************************************************/
GT_VOID *osRealloc_MemoryLeakageDbg
(
    IN GT_VOID * ptr ,
    IN GT_U32    size,
    IN const char*    fileNamePtr,
    IN GT_U32   line
)
{
    void *newPtr;
    newPtr = internalOsRealloc(ptr,size);

    /* unregister old ptr */
    osMemUnregisterPtr(ptr,fileNamePtr,line);
    /* register new ptr */
    osMemRegisterPtr(newPtr,size,fileNamePtr,line);
    return newPtr;
}
/**
* @internal osFree_MemoryLeakageDbg function
* @endinternal
*
* @brief   osFree_MemoryLeakageDbg replace osFree to get debug info
*
* @note Usage of this function is NEVER during initialization.
*
*/
GT_VOID osFree_MemoryLeakageDbg
(
    IN GT_VOID* const memblock,
    IN const char*    fileNamePtr,
    IN GT_U32   line
)
{
    internalOsFree(memblock);
    osMemUnregisterPtr(memblock,fileNamePtr,line);
}
#endif /*INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG*/


/**
* @internal dmaOverflowDetectionGet function
* @endinternal
*
* @brief   check if the DMA memory detected with overflow error.
*         this function allow the application to check via function
*         osCacheDmaOverflowDetectionErrorGet(...) to check if any of the 'Cached DMA'
*         caused 'overflow'
* @param[in] allocPtr                 - allocated pointer by osCacheDmaMalloc
* @param[in] numOfBytes               - number of bytes to allocated by osCacheDmaMalloc
*
* @retval GT_TRUE                  - on error
* @retval GT_FALSE                 - no error
*/
static GT_BOOL dmaOverflowDetectionGet
(
    IN GT_PTR   allocPtr ,
    IN GT_U32   numOfBytes
)
{
#if ERROR_IN_IMPLEMENTATION == 0
    GT_U32  ii,jj;
    GT_U32  *startOfOverflowDetectionRegionPtr;

    if(numOfBytes <= DMA_NUM_BYTES_OVERFLOW_DETECTION_REGION_CNS)
    {
        osPrintf("ERROR : dmaOverflowDetectionGet : numOfBytes[%d] <= [%d] \n" ,
            numOfBytes , DMA_NUM_BYTES_OVERFLOW_DETECTION_REGION_CNS);

        /* should not happen */
        return GT_TRUE;
    }

    /* the pointer and numOfBytes holds the 'DMA_NUM_BYTES_OVERFLOW_DETECTION_REGION_CNS'
       for the 'overflow detection' region at the end of each 'Cached DMA' .

        we need to initialize it with the pre-defined values.
    */

    startOfOverflowDetectionRegionPtr = (GT_U32*)((GT_CHAR*)allocPtr + (numOfBytes - DMA_NUM_BYTES_OVERFLOW_DETECTION_REGION_CNS));

    for(ii = 0 ; ii < DMA_NUM_LINES_OF_PRE_DEFINED_PATTERNS ; ii++)
    {
        for(jj = 0 ; jj < DMA_NUM_PRE_DEFINED_PATTERNS_CNS; jj++)
        {
            if((*startOfOverflowDetectionRegionPtr) != dmaPreDefinedPatternsArr[jj])
            {
                osPrintf("ERROR : dmaOverflowDetectionGet : ii=[%d] ,  jj=[%d] , read value[0x%8.8x] , expected [0x%8.8x] \n" ,
                    ii , jj , (*startOfOverflowDetectionRegionPtr) , dmaPreDefinedPatternsArr[jj]);


                osPrintf("dump start and end of memory \n");
                osPrintf("dump start memory \n");
                startOfOverflowDetectionRegionPtr = (GT_U32*)((GT_CHAR*)allocPtr);
                for(ii = 0 ; ii < 10 ; ii++)
                {
                    osPrintf("%2.2d : [0x%8.8x] [0x%8.8x] [0x%8.8x] [0x%8.8x] \n",
                        ii ,
                        startOfOverflowDetectionRegionPtr[0],
                        startOfOverflowDetectionRegionPtr[1],
                        startOfOverflowDetectionRegionPtr[2],
                        startOfOverflowDetectionRegionPtr[3]);

                    startOfOverflowDetectionRegionPtr += 4;
                }
                osPrintf("dump start memory -- ended \n");

                osPrintf("dump end memory \n");
                startOfOverflowDetectionRegionPtr = (GT_U32*)((GT_CHAR*)allocPtr + (numOfBytes - (16*10)));
                for(ii = 0 ; ii < 10 ; ii++)
                {
                    osPrintf("%2.2d : [0x%8.8x] [0x%8.8x] [0x%8.8x] [0x%8.8x] \n",
                        (numOfBytes / 16) + ii ,
                        startOfOverflowDetectionRegionPtr[0],
                        startOfOverflowDetectionRegionPtr[1],
                        startOfOverflowDetectionRegionPtr[2],
                        startOfOverflowDetectionRegionPtr[3]);

                    startOfOverflowDetectionRegionPtr += 4;
                }
                osPrintf("dump end memory -- ended \n");

                /* error detected */
                return GT_TRUE;
            }

            startOfOverflowDetectionRegionPtr++;
        }
    }
#else
    GT_UNUSED_PARAM(allocPtr);
    GT_UNUSED_PARAM(numOfBytes);
#endif /*ERROR_IN_IMPLEMENTATION == 0*/
    return GT_FALSE;
}

/**
* @internal prvOsCacheDmaMallocOverflowDetectionSupportGet function
* @endinternal
*
* @brief   the function checks if need to update the 'size' with number of bytes
*         used for 'overflow detection' region at the end of each 'Cached DMA'
* @param[in,out] sizePtr                  - (pointer to) the size of allocation given by the application.
* @param[in,out] sizePtr                  - (pointer to) the update size of needed allocation in case that
*                                      needed extra bytes for 'overflow detection' region.
*                                       None.
*/
GT_VOID prvOsCacheDmaMallocOverflowDetectionSupportGet
(
    INOUT GT_U32    *sizePtr
)
{
#if ERROR_IN_IMPLEMENTATION == 0
    if(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(dmaOverflowDetectionEnabled) == GT_TRUE)
    {
        (*sizePtr) += DMA_NUM_BYTES_OVERFLOW_DETECTION_REGION_CNS;
    }
#else
    GT_UNUSED_PARAM(sizePtr);
#endif /*ERROR_IN_IMPLEMENTATION == 0*/
    return;
}


/**
* @internal osCacheDmaOverflowDetectionEnableSet function
* @endinternal
*
* @brief   enable the 'Cache DMA' manager to add 'overflow detection' region at
*         the end of each 'Cached DMA' (on top of the actual needed size) and to
*         initialize this region with 'predefined pattern' words.
*         this function allow the application to check via function
*         osCacheDmaOverflowDetectionErrorGet(...) to check if any of the 'Cached DMA'
*         caused 'overflow'
* @param[in] enable                   - GT_TRUE -  the detection.
*                                      GT_FALSE - disable the detection.
*
* @retval GT_OK                    - on success
*/
GT_STATUS osCacheDmaOverflowDetectionEnableSet
(
    IN GT_BOOL  enable
)
{
#if ERROR_IN_IMPLEMENTATION == 0
    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_SET(dmaOverflowDetectionEnabled,enable);
#else
    GT_UNUSED_PARAM(enable);
#endif /*ERROR_IN_IMPLEMENTATION == 0*/
    return GT_OK;
}

/**
* @internal osCacheDmaOverflowDetectionErrorGet function
* @endinternal
*
* @brief   the function checks ALL if any of the 'Cached DMA' memories caused 'overflow'
*         and the function will return the pointers to the 'Cached DMA' memories with errors.
* @param[in] maxNumErrorsToGet        - max number of errors that dmaMemoryWithErrorsArr[] can hold.
*                                      NOTE: value 0 means that application not care about which
*                                      memory caused the problem.
* @param[in,out] dmaMemoryWithErrorsArr[] - array of pointers to get the 'Cached DMA' memories with errors.
*                                      NOTE : 1. this array can be NULL when maxNumErrorsToGet == 0
*                                      2. this array must hold space for maxNumErrorsToGet 'Cached DMA' memories.
*                                      3. if maxNumErrorsToGet is smaller then actual number of memories with errors,
*                                      then the rest of the memories are not filled into this array.
*
* @param[out] actualNumErrorsPtr       - (pointer to) the actual number of 'Cached DMA' memories
*                                      that caused 'overflow'.
*                                      can be NULL if application not care about the number of errors.
* @param[in,out] dmaMemoryWithErrorsArr[] - array of pointers to the 'Cached DMA' memories with errors.
*
* @retval GT_OK                    - no memory caused error.
* @retval GT_ERROR                 - at least one of the 'Cached DMA' memories with errors.
*/
GT_STATUS osCacheDmaOverflowDetectionErrorGet
(
    IN    GT_U32  maxNumErrorsToGet,
    OUT   GT_U32  *actualNumErrorsPtr,
    INOUT   void*   dmaMemoryWithErrorsArr[]
)
{
    GT_STATUS   rc = GT_OK;
    GT_BOOL     gotError;
    DMA_ALLOCATION_STC *currentPtr;
    DMA_ALLOCATION_STC *nextPtr;
    GT_U32      currentErrorIndex = 0;

    if(PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(dmaOverflowDetectionEnabled) == GT_FALSE)
    {
        if(actualNumErrorsPtr)
        {
            *actualNumErrorsPtr = 0;
        }
        /* no errors can be detected ! */
        return GT_OK;
    }

    currentPtr = (PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(globalDmaAllocationsFirstPtr));

    while(currentPtr)
    {
        nextPtr = currentPtr->nextElementPtr;
        gotError = GT_FALSE;

        if(currentPtr->myMagic != MAGIC_NUMBER_CNS)
        {
            /* error */
            gotError = GT_TRUE;
            REPORT_CORRUPTED_MEMORY_MAC;
        }
        else
        if(currentPtr->myMemoryPtr)
        {
            /* check if this 'Cached DMA' memory caused 'overflow' */
            gotError = dmaOverflowDetectionGet(currentPtr->myMemoryPtr,currentPtr->myMemoryNumOfBytes);
        }

        if(gotError == GT_TRUE)
        {
            rc = GT_ERROR;

            /* check if application allocated space to get the memory indication */
            if((currentErrorIndex < maxNumErrorsToGet) && dmaMemoryWithErrorsArr)
            {
                /* let application info */
                dmaMemoryWithErrorsArr[currentErrorIndex] = currentPtr->myMemoryPtr;
            }

            /* increment the number of errors */
            currentErrorIndex++;

            if(currentPtr->myMagic != MAGIC_NUMBER_CNS)
            {
                /* we CAN'T go on the link list !!! it is corrupted !!! */
                break;
            }
        }

        /* update the 'current' to be 'next' */
        currentPtr = nextPtr;
    }

    if(actualNumErrorsPtr)
    {
        *actualNumErrorsPtr = currentErrorIndex;
    }

    return rc;
}




