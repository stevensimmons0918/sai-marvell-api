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
* @file osLinuxMem.c
*
* @brief Linux Operating System wrapper. Memory manipulation facility.
*
* @version   35
********************************************************************************
*/
/***** Includes ********************************************************/
/* IMPORTANT : we must remove the declaration of OS_MALLOC_MEMORY_LEAKAGE_DBG */
/* because we not want local calls to osMalloc to be affected by the H file definitions */
#define FORCE_IGNORE_OS_MALLOC_MEMORY_LEAKAGE_DBG

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsExc.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <gtOs/gtOsSharedMalloc.h>
#include <gtOs/gtOsSharedData.h>

#if __WORDSIZE == 64
# include <sys/mman.h>
#endif
#if defined(USE_VALGRIND) && !defined(SHARED_MEMORY)
#ifdef ASIC_SIMULATION
#include <valgrind/valgrind.h>
#else
#include </usr/include/valgrind/valgrind.h>
#endif
#endif /* USE_VALGRIND */

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

#ifdef ASIC_SIMULATION
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#include <os/simTypesBind.h>
#include <os/simOsProcess.h>

#include <asicSimulation/SDistributed/sdistributed.h>
#include <asicSimulation/SInit/sinit.h>
#endif
#include <gtExtDrv/drivers/prvExtDrvLinuxMapping.h>


/*********  important ****  important  ***  important ****  important **********
NOTE: in this file the OS_MALLOC_MEMORY_LEAKAGE_DBG is NEVER defined
    instead this file must check
    INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG
    instead of
    OS_MALLOC_MEMORY_LEAKAGE_DBG
*********  important ****  important  ***  important ****  important **********/

extern  char* shrAddNameSuffix(const char* name, char* buffer, int bufsize);


/***** Function Declaration ********************************************/
#ifdef ASIC_SIMULATION
static GT_VOID* dmaAsSharedMemoryMalloc(IN GT_U32 size);
#endif

#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>

/*global variables macros*/

#define PRV_SHARED_MAIN_OS_DIR_OS_LINUX_MEM_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_HELPER_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.osLinuxMemSrc._var,_value)

#define PRV_SHARED_MAIN_OS_DIR_OS_LINUX_MEM_SRC_GLOBAL_VAR_GET(_var)\
    PRV_HELPER_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.osLinuxMemSrc._var)


#define PRV_NON_SHARED_MAIN_OS_DIR_OS_LINUX_MEM_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_HELPER_NON_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.osLinuxMemSrc._var,_value)

#define PRV_NON_SHARED_MAIN_OS_DIR_OS_LINUX_MEM_SRC_GLOBAL_VAR_GET(_var)\
    PRV_HELPER_NON_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.osLinuxMemSrc._var)



/***** External Functions **********************************************/
GT_VOID *osMemPoolsAlloc(GT_U32 size);
GT_VOID *osMemPoolsRealloc(GT_VOID *ptr, GT_U32 size);
GT_VOID osMemPoolFree(GT_VOID *memBlock);
GT_VOID *osStaticMallocFromChunk(GT_U32 size);

extern GT_VOID prvOsCacheDmaMallocOverflowDetectionSupportGet
(
    INOUT GT_U32    *sizePtr
);

extern GT_STATUS osCacheDmaRegisterPtr(
    IN GT_VOID *    allocPtr,
    IN GT_U32       numOfBytes
);
extern void osCacheDmaUnregisterPtr(
    IN GT_VOID *    oldPointer
);
extern GT_STATUS osCacheDmaSystemReset(void);
#ifdef INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG
extern GT_STATUS osMemRegisterPtr(
    IN GT_VOID *    allocPtr,
    IN GT_U32       numOfBytes,
    IN const char*    fileNamePtr,
    IN GT_U32   line
);
extern void osMemUnregisterPtr(
    IN GT_VOID *    oldPointer,
    IN const char*    fileNamePtr,
    IN GT_U32   line
);
#else /*!INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG*/
/* define empty macro to reduce management when not needed */
#define osMemRegisterPtr(a,b,c)
#define osMemUnregisterPtr(a,b)
#endif /*!INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG*/

/***** Global Vars ********************************************/


#ifndef ASIC_SIMULATION
extern GT_32 gtPpFd;
#endif

/* the MAGIC will use only 28 LSbit , and will leave 4 bits for the window (0..15)*/
#define DMA_MEM_MAGIC  0x07654321
#define DMA_MEM_MAGIC_MASK(_val) (_val & 0x0FFFFFFF)

#define DMA_MEM_HIDE_WINDOW_IN_MAGIC(_win) ((_win) << 28)
#define DMA_MEM_GET_WINDOW_FROM_MAGIC(_magic) (((_magic) >> 28) & 0xF)

#define PRV_SHARED_DB osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.osLinuxMemSrc

#define PRV_DMA_WINDOW osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.osLinuxMemSrc.dmaWindowsCurrentWindow
#define PRV_SHARED_DMA_DB osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.osLinuxMemSrc.dmaWindowsArr[PRV_DMA_WINDOW]

#define PRV_SHARED_DMA_window_DB(_win) osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.osLinuxMemSrc.dmaWindowsArr[_win]


#ifdef ASIC_SIMULATION
/* a number that indicate the 'key' representing the shared memory */
#define SIM_DMA_AS_SHARED_MEM_KEY_CNS   5678

#endif /* ASIC_SIMULATION */

extern uint64_t linux_virt_to_phys(uintptr_t vaddr);




/***** Public Functions ************************************************/
#define SHARED_MALLOC32_MAC SHARED_MALLOC_MAC
#define SHARED_FREE32_MAC SHARED_FREE_MAC

#ifndef SHARED_MEMORY
/* shrMemSharedMalloc always allocate in first 2Gig address space */
#if __WORDSIZE == 64
/* 32bit works ok, workarounds for 64 bit */
#undef SHARED_MALLOC32_MAC
#undef SHARED_FREE32_MAC

#if defined(MIPS64_CPU) || defined(INTEL64_CPU)
#define ADV64_CPU
#endif

#ifndef ADV64_CPU
#ifdef MAP_32BIT
#if defined(ALL_ALLOCS_32BIT) || defined(ASIC_SIMULATION)
/* workaround: allocate in first 2Gig address space */
static GT_VOID* malloc_32bit(unsigned int size)
{
    size_t *ptr;
    ptr = mmap(NULL, size + sizeof(size_t),
        PROT_READ | PROT_WRITE,
        MAP_32BIT | MAP_ANONYMOUS | MAP_PRIVATE,
        0, 0);
    if (ptr)
    {
        *ptr = size;
        ptr++;
    }
    return ptr;
}
static GT_VOID free_32bit(GT_VOID* ptr)
{
    size_t *sptr = (size_t*)ptr;
    if (sptr)
    {
        sptr--;
        munmap(sptr, *sptr + sizeof(size_t));
    }
}
#endif /* ALL_ALLOCS_32BIT || ASIC_SIMULATION */
#endif /* defined(MAP_32BIT) */
#ifdef ALL_ALLOCS_32BIT
static GT_VOID* realloc_32bit(GT_VOID* ptr, size_t newsize)
{
    size_t *sptr = (size_t*)ptr;
    GT_VOID   *newptr;
    if (ptr == NULL)
        return malloc_32bit(newsize);
    sptr--;
    if (newsize == *sptr)
    {
        return ptr;
    }
    newptr = malloc_32bit(newsize);
    memcpy(newptr, ptr, (*sptr < newsize) ? *sptr : newsize);
    free_32bit(ptr);
    return newptr;
}
#define SHARED_REALLOC32_MAC realloc_32bit
#endif /* ALL_ALLOCS_32BIT */

#define SHARED_MALLOC32_MAC malloc_32bit
#define SHARED_FREE32_MAC free_32bit



#else /* defined(ADV64_CPU) */
/* && !defined SHARED_MEM */
extern int adv64_malloc_32bit;
#include <gtOs/gtOsSharedMemoryRemapper.h>

GT_VOID* malloc_32bit_adv64(unsigned int size)
{
  static GT_BOOL firstTime = GT_TRUE;
  size_t *ptr;
  static GT_UINTPTR addr = (GT_UINTPTR)0;
  static GT_UINTPTR end = (GT_UINTPTR)0;
  void *ptr1;

  if (!adv64_malloc_32bit) /* set indirectly by pssBspApis.c and prestera.c .
                              Default is zero */
  {
    ptr1 = malloc(size);
    if (firstTime)
    {
#if 0
      osPrintf("adv64_malloc_32bit is off. Ret-ptr = 0x%lx\n", ptr1);
#endif
      firstTime = GT_FALSE;
    }
    return ptr1;
  }


  if (addr == (GT_UINTPTR)0)
  {
    /* allocate memory at fixed address */
    void *map;
    map = mmap((void*)SHARED_MEMORY_MALLOC_VIRT_ADDR_MAC, 64*1024*1024,
        PROT_READ | PROT_WRITE,
        MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE,
        0, 0);
    if (MAP_FAILED == map)
    {
        osPrintf("adv64_malloc_32bit mmap failed: %s\n", strerror(errno));
        addr = !((GT_UINTPTR)0);
        end = addr;
    }
    else
    {
        addr = (GT_UINTPTR)map;
        end = addr + 64*1024*1024;
    }
  }

  if ((end - addr) < size)
  {
      return NULL;
  }

  ptr = (size_t*)addr;
  if (ptr)
  {
    *ptr = size;
    ptr++;
  }

  addr = (GT_UINTPTR)ptr + size;


  if (firstTime)
  {
    osPrintf("adv64_malloc_32bit is on. Ret-ptr = 0x%lx\n", ptr);
    firstTime = GT_FALSE;
  }
  return ptr;
}

#if defined(ALL_ALLOCS_32BIT) || defined(ASIC_SIMULATION)
static GT_VOID free_32bit_adv64(GT_VOID* ptr)
{
  if (!adv64_malloc_32bit)
    free(ptr);

  /* else do nothing */
}
#endif

#ifdef ALL_ALLOCS_32BIT
static GT_VOID* realloc_32bit_adv64(GT_VOID* ptr, size_t newsize)
{
  size_t *sptr = (size_t*)ptr;
  GT_VOID   *newptr;
  if (ptr == NULL)
    return malloc_32bit_adv64(newsize);
  sptr--;
  if (newsize == *sptr)
  {
    return ptr;
  }
  newptr = malloc_32bit_adv64(newsize);
  memcpy(newptr, ptr, (*sptr < newsize) ? *sptr : newsize);
  free_32bit_adv64(ptr);
  return newptr;
}
#define SHARED_REALLOC32_MAC realloc_32bit_adv64
#endif /* ALL_ALLOCS_32BIT */

#define SHARED_MALLOC32_MAC malloc_32bit_adv64
#define SHARED_FREE32_MAC free_32bit_adv64
#endif /* defined(ADV64_CPU) */



/* redefine for 64bit */
#ifdef ALL_ALLOCS_32BIT
#  undef SHARED_MALLOC_MAC
#  undef SHARED_FREE_MAC
#  define SHARED_MALLOC_MAC     SHARED_MALLOC32_MAC
#  define SHARED_FREE_MAC       SHARED_FREE32_MAC
#  ifdef SHARED_REALLOC32_MAC
#    define realloc             SHARED_REALLOC32_MAC
#  endif
#endif

#endif /* __WORDSIZE == 64 */
#endif /* !defined(SHARED_MEMORY) */

/**
* @internal osMemInit function
* @endinternal
*
* @brief   Initialize the memory management.
*
* @param[in] size                     - memory  to manage.
* @param[in] reallocOnDemand          - defines whether in the case of memory chunk depleation
*                                      a new chunk will be allocated or an out of memory will
*                                      be reported.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - not enough memory in the system.
*/
GT_STATUS osMemInit
(
    IN GT_U32 size,
    IN GT_BOOL reallocOnDemand
)
{
    GT_MEMORY_MANAGE_METHOD_ENT memAllocMethod;
    GT_STATUS ret;

    if (reallocOnDemand == GT_TRUE)
    {
        memAllocMethod = GT_MEMORY_MANAGE_METHOD_REALLOC_ON_DEMAND_E;
    }
    else
    {
        memAllocMethod = GT_MEMORY_MANAGE_METHOD_NO_REALLOC_ON_DEMAND_E;
    }

    /* Create the freeDma semaphore */
    if (osMutexCreate("osFreeDmaPool", &PRV_SHARED_DB.freeDmaPoolMtx) != GT_OK)
    {
        return GT_FAIL;
    }
#ifdef ASIC_SIMULATION
    /* Create the simDma semaphore */
    if (osMutexCreate("osSimDma", &PRV_SHARED_DB.simDmaMutexId) != GT_OK)
    {
        return GT_FAIL;
    }

    {
        GT_U32      dmaSize;
        GT_UINTPTR  dmaBase;
        GT_STATUS   rc;

        /* get the DMA block base address (physical address !!!) */
        /* NOTE: in WM the 'physical address' and th e'virtual address' are the same (1:1) */
        rc = extDrvGetDmaBase(&dmaBase);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* get the DMA block size */
        rc = extDrvGetDmaSize(&dmaSize);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* NOTE : we use the memory allocated by 'extDrvGetDmaBase()' that hold 'alignment' of _1M */
        PRV_SHARED_DB.simDmaAsSharedMemPtr  = (void*)dmaBase;
        PRV_SHARED_DB.simDmaAsSharedMemSize = dmaSize;
        if(PRV_SHARED_DB.simDmaAsSharedMemPtr == NULL || dmaSize == 0)
        {
            return GT_NO_RESOURCE;
        }
    }

    if(sasicgSimulationRole == SASICG_SIMULATION_ROLE_DISTRIBUTED_APPLICATION_SIDE_VIA_BROKER_E)
    {
        /* the application side that work with broker may decide how the broker
           will handle DMA read/write requests:
            1. use shared memory and do read/write by itself.
            2. use socket to send to application the request from read/write DMA
        */

        if(brokerDmaMode == BROKER_NOT_USE_SOCKET)
        {
            /* steal 1/2 of memory for DMA */
            if(NULL == dmaAsSharedMemoryMalloc(size/2))
            {
                return GT_NO_RESOURCE;
            }

            /* send DMA registration info */
            simDistributedRegisterDma(
                    (GT_U32)((GT_UINTPTR)PRV_SHARED_DB.simDmaAsSharedMemPtr),/* the address of the pointer in our memory */
                    PRV_SHARED_DB.simDmaAsSharedMemSize,/* the size of shared memory */
                    SIM_DMA_AS_SHARED_MEM_KEY_CNS,/* the key to retrieve the shared memory */
                    BROKER_DMA_MODE_SHARED_MEMORY);

            /* use other 1/2 of memory for non-DMA */
            size = size / 2;
        }
        else if(brokerDmaMode == BROKER_USE_SOCKET)
        {
            /* send DMA registration info -- we work in socket mode */
            simDistributedRegisterDma(0,/* don't care */
                                      0,/* don't care */
                                      0,/* don't care */
                                      BROKER_DMA_MODE_SOCKET);
        }
        else
        {
            return GT_BAD_STATE;
        }
    }
#endif
    ret = osMemLibInit(
            (GT_MEMORY_ALLOC_FUNC)SHARED_MALLOC_MAC,
            SHARED_FREE_MAC,size,
            memAllocMethod);

    PRV_SHARED_MAIN_OS_DIR_OS_LINUX_MEM_SRC_GLOBAL_VAR_SET(firstInitialization,GT_FALSE);

    return ret;
}

/*******************************************************************************
* osStaticMalloc
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
GT_VOID *osStaticMalloc
(
    IN GT_U32 size
)
{
    void *ptr;
    if (PRV_SHARED_MAIN_OS_DIR_OS_LINUX_MEM_SRC_GLOBAL_VAR_GET(firstInitialization) == GT_TRUE)
    {
        return osMalloc(size);
    }
    ptr = osStaticMallocFromChunk(size);
    osMemRegisterPtr(ptr,size,LOCAL_DEBUG_INFO);
    return ptr;
}

/*******************************************************************************
* internalOsMalloc
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
*       insufficient memory available
*
* COMMENTS:
*       Usage of this function is NEVER during initialization.
*
*******************************************************************************/
GT_VOID *internalOsMalloc
(
    IN GT_U32 size
)
{
    IS_WRAPPER_OPEN_PTR;

    if( size == 0) return NULL;

    if (PRV_SHARED_MAIN_OS_DIR_OS_LINUX_MEM_SRC_GLOBAL_VAR_GET(firstInitialization) == GT_TRUE)
    {
        OSMEM_NOPOOLS_HDR_STC *hdrPtr;
        hdrPtr = (OSMEM_NOPOOLS_HDR_STC*)SHARED_MALLOC_MAC(size+sizeof(OSMEM_NOPOOLS_HDR_STC));
        if (hdrPtr == NULL)
        {
            return NULL;
        }
        hdrPtr->magic = (GT_UINTPTR)hdrPtr;
        hdrPtr->size = size;
        hdrPtr++;
        return (GT_VOID*)hdrPtr;
    }
    else
    {
        return osMemPoolsAlloc(size);
    }
}

/*******************************************************************************
* osMalloc
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
*       insufficient memory available
*
* COMMENTS:
*       Usage of this function is NEVER during initialization.
*
*******************************************************************************/
GT_VOID *osMalloc
(
    IN GT_U32 size
)
{
    void *ptr;
#if defined(USE_VALGRIND) && !defined(SHARED_MEMORY)
    if (RUNNING_ON_VALGRIND)
    {
        return malloc(size);
    }
#endif
    ptr = internalOsMalloc(size);

    osMemRegisterPtr(ptr,size,LOCAL_DEBUG_INFO);
    return ptr;
}
/*******************************************************************************
* internalOsRealloc
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
GT_VOID *internalOsRealloc
(
    IN GT_VOID * ptr ,
    IN GT_U32 size
)
{
    OSMEM_NOPOOLS_HDR_STC *oldHdr;
    IS_WRAPPER_OPEN_PTR;

    if (ptr == NULL)
    {
        return osMalloc(size);
    }

    oldHdr = (OSMEM_NOPOOLS_HDR_STC*)((GT_UINTPTR)ptr - sizeof(OSMEM_NOPOOLS_HDR_STC));
    if (PRV_SHARED_MAIN_OS_DIR_OS_LINUX_MEM_SRC_GLOBAL_VAR_GET(firstInitialization) == GT_TRUE)
    {
        OSMEM_NOPOOLS_HDR_STC *allocPtr;
#if defined(SHARED_MEMORY) || !defined(ASIC_SIMULATION)
        if (oldHdr->magic != (GT_UINTPTR)oldHdr)
        {
            osFatalError(OS_FATAL_WARNING, "osRealloc(): trying to realloc wrong memory\n");
            return NULL;
        }

#if 0
        osPrintf("Warning: osRealloc before initialization\n");
#endif

        allocPtr = (OSMEM_NOPOOLS_HDR_STC*)SHARED_MALLOC_MAC(size+sizeof(OSMEM_NOPOOLS_HDR_STC));
        if (allocPtr == NULL)
        {
            return NULL;
        }
        osMemCpy(allocPtr+1,ptr,oldHdr->size < size ? oldHdr->size : size);
#ifdef SHARED_MEMORY
        shrMemSharedFreeSize(oldHdr, oldHdr->size+sizeof(OSMEM_NOPOOLS_HDR_STC));
#else
        SHARED_FREE_MAC(oldHdr);
#endif
#else
        allocPtr = realloc(oldHdr, size+sizeof(OSMEM_NOPOOLS_HDR_STC));
        if (allocPtr == NULL)
        {
            return NULL;
        }
#endif
        allocPtr->magic = (GT_UINTPTR)allocPtr;
        allocPtr->size = size;
        return (GT_VOID*)(allocPtr + 1);

    }
    else
    {
        if (oldHdr->magic == (GT_UINTPTR)oldHdr)
        {
            /* realloc memory allocated by malloc() */
            GT_VOID *newPtr = osMalloc(size);
            if (newPtr == NULL)
            {
                return NULL;
            }
            osMemCpy(newPtr, ptr, oldHdr->size < size ? oldHdr->size : size);
#ifdef SHARED_MEMORY
            shrMemSharedFreeSize(oldHdr, oldHdr->size+sizeof(OSMEM_NOPOOLS_HDR_STC));
#else
            SHARED_FREE_MAC(oldHdr);
#endif
            return newPtr;

        }
        return osMemPoolsRealloc(ptr, size);
    }
}
/*******************************************************************************
* osRealloc
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
GT_VOID *osRealloc
(
    IN GT_VOID * ptr ,
    IN GT_U32 size
)
{
    void *newPtr;
#if defined(USE_VALGRIND) && !defined(SHARED_MEMORY)
    if (RUNNING_ON_VALGRIND)
    {
        return realloc(ptr,size);
    }
#endif
    newPtr = internalOsRealloc(ptr,size);

    /* unregister old ptr */
    osMemUnregisterPtr(ptr,LOCAL_DEBUG_INFO);
    /* register new ptr */
    osMemRegisterPtr(newPtr,size,LOCAL_DEBUG_INFO);
    return newPtr;
}

/**
* @internal internalOsFree function
* @endinternal
*
* @brief   Deallocates or frees a specified memory block.
*
* @note Usage of this function is NEVER during initialization.
*
*/
GT_VOID internalOsFree
(
    IN GT_VOID* const memblock
)
{
    OSMEM_NOPOOLS_HDR_STC *oldHdr = NULL;
    assert(memblock != NULL);
    oldHdr = (OSMEM_NOPOOLS_HDR_STC*)((GT_UINTPTR)memblock - sizeof(OSMEM_NOPOOLS_HDR_STC));
    if (PRV_SHARED_MAIN_OS_DIR_OS_LINUX_MEM_SRC_GLOBAL_VAR_GET(firstInitialization) == GT_TRUE)
    {
        if (oldHdr->magic != (GT_UINTPTR)oldHdr)
        {
            osFatalError(OS_FATAL_WARNING, "osFree(): bad magic");
        }
        else
        {
#ifdef SHARED_MEMORY
            shrMemSharedFreeSize(oldHdr, oldHdr->size+sizeof(OSMEM_NOPOOLS_HDR_STC));
#else
            SHARED_FREE_MAC(oldHdr);
#endif
        }
    }
    else
    {
        /* check if this free is trying to free something that wasn't
           allocated by owr pools manager */
        if (oldHdr->magic == (GT_UINTPTR)oldHdr)
        {
/*#ifdef OSMEMPOOLS_DEBUG
            osPrintf("Warning:Trying to Free a pointer allocated by malloc after osMemInit !\n");
#endif bothers CLI */

#ifdef SHARED_MEMORY
            shrMemSharedFreeSize(oldHdr, oldHdr->size+sizeof(OSMEM_NOPOOLS_HDR_STC));
#else
            SHARED_FREE_MAC(oldHdr);
#endif
        }
        else
        {
            osMemPoolFree(memblock);
        }
    }
    return;
}

/**
* @internal CPSS_osFree function
* @endinternal
*
* @brief   Deallocates or frees a specified memory block.
*
* @note Usage of this function is NEVER during initialization.
*
*/
GT_VOID CPSS_osFree
(
    IN GT_VOID* const memblock
)
{
#if defined(USE_VALGRIND) && !defined(SHARED_MEMORY)
    if (RUNNING_ON_VALGRIND)
    {
        free(memblock);
        return;
    }
#endif
    internalOsFree(memblock);
    osMemUnregisterPtr(memblock,LOCAL_DEBUG_INFO);
}

/*******************************************************************************
* osCacheDmaMallocByWindow
*
* DESCRIPTION:
*       Allocate a cache-safe buffer of specified size for DMA devices
*       and drivers
*
* INPUTS:
*       windowId - the DMA window to use.
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       A pointer to the cache-safe buffer, or NULL
*
* COMMENTS:
*       None
*
*******************************************************************************/
static GT_VOID *internal_osCacheDmaMallocByWindow
(
    IN GT_U32 windowId,
    IN GT_U32 size
)
{
    GT_VOID *ptr;
    IS_WRAPPER_OPEN_PTR;

    if (size == 0)
        return NULL;

    /* check if need to get modified size to hold support for 'overflow detection' */
    prvOsCacheDmaMallocOverflowDetectionSupportGet(&size);

#ifdef ASIC_SIMULATION
    if(PRV_SHARED_DB.simDmaAsSharedMemSize == 0)
    {
        ptr = SHARED_MALLOC32_MAC(size);
    }
    else /* cpssHwDriverGenWmInPexModeGet() also get here */
#endif  /* ASIC_SIMULATION */
    {
        OSMEM_POOL_FREE_ELEM_PTR freePtr;
        OSMEM_POOL_FREE_ELEM_PTR *prev;
        OSMEM_NOPOOLS_HDR_STC *hdrPtr = NULL;

        osMutexLock(PRV_SHARED_DB.freeDmaPoolMtx);
        freePtr = PRV_SHARED_DMA_window_DB(windowId).freeDmaPool;
        prev = &PRV_SHARED_DMA_window_DB(windowId).freeDmaPool;
        while (freePtr != NULL)
        {
            if (freePtr->size == size)
                break;
            prev = &(freePtr->next_elem);
            freePtr = freePtr->next_elem;
        }
        if (freePtr)
        {
fprintf(stderr,"Reuse from 'free' chain : Allocating %d bytes for DMA\n", size);
            /* remove from chain */
            *prev = freePtr->next_elem;
            hdrPtr = (OSMEM_NOPOOLS_HDR_STC*)freePtr;
        }
        osMutexUnlock(PRV_SHARED_DB.freeDmaPoolMtx);

        if (!freePtr)
        {
#ifndef ASIC_SIMULATION
/*fprintf(stderr,"Allocating %d bytes for DMA\n", size);*/
            hdrPtr = (OSMEM_NOPOOLS_HDR_STC*)shrMemSharedMallocIntoBlock(
                    &(PRV_SHARED_DMA_window_DB(windowId).cdma_mem), gtPpFd, size+sizeof(OSMEM_NOPOOLS_HDR_STC));
#else   /* ASIC_SIMULATION */
            /* take sem */
            osMutexLock(PRV_SHARED_DB.simDmaMutexId);

            /* check if we have enough space for the new memory request */
            if ((PRV_SHARED_DB.simDmaAsSharedMemOffsetUsed + size+sizeof(*hdrPtr)) <=
                PRV_SHARED_DB.simDmaAsSharedMemSize)
            {
                hdrPtr = (OSMEM_NOPOOLS_HDR_STC*)((GT_CHAR*)PRV_SHARED_DB.simDmaAsSharedMemPtr +
                          PRV_SHARED_DB.simDmaAsSharedMemOffsetUsed);
                PRV_SHARED_DB.simDmaAsSharedMemOffsetUsed += size+sizeof(*hdrPtr);
            }

            /* free sem */
            osMutexUnlock(PRV_SHARED_DB.simDmaMutexId);

#endif  /* ASIC_SIMULATION */

        }
        if (hdrPtr == NULL)
        {
fprintf(stderr,"Failed to allocate %d bytes, exiting\n", size);
exit(1);
            return NULL;
        }

        /* save the 'windowId' within the magic field */
        hdrPtr->magic = DMA_MEM_MAGIC | DMA_MEM_HIDE_WINDOW_IN_MAGIC(windowId);
        hdrPtr->size = size;
        hdrPtr++;
        ptr = hdrPtr;
    }

    /* register the dma pointer for system reset */
    osCacheDmaRegisterPtr(ptr,size);

    return ptr;
}

/*******************************************************************************
* osCacheDmaMalloc
*
* DESCRIPTION:
*       Allocate a cache-safe buffer of specified size for DMA devices
*       and drivers
*
* INPUTS:
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       A pointer to the cache-safe buffer, or NULL
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID *osCacheDmaMalloc
(
    IN GT_U32 size
)
{
    return internal_osCacheDmaMallocByWindow(0,size);
}
/*******************************************************************************
* osCacheDmaMallocByWindow
*
* DESCRIPTION:
*       Allocate a cache-safe buffer of specified size for DMA devices
*       and drivers
*
* INPUTS:
*       windowId - the DMA window to use.
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       A pointer to the cache-safe buffer, or NULL
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID *osCacheDmaMallocByWindow
(
    IN GT_U32 windowId,
    IN GT_U32 size
)
{
    IS_WRAPPER_OPEN_PTR;
    if(windowId >= PRV_SHARED_DB.dmaWindowsNumOfActiveWindows)
    {
        /* ERROR window is out of supported range */
        return NULL;
    }
    return internal_osCacheDmaMallocByWindow(windowId,size);
}
/**
* @internal osCacheDmaFree function
* @endinternal
*
* @brief   Free the buffer acquired with osCacheDmaMalloc()
*
* @param[in] ptr                      - pointer to malloc/free buffer
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osCacheDmaFree
(
    IN GT_VOID *ptr
)
{
    IS_WRAPPER_OPEN_STATUS;

    /* unregister the ptr */
    (void)osCacheDmaUnregisterPtr(ptr);

#ifdef ASIC_SIMULATION
    if(PRV_SHARED_DB.simDmaAsSharedMemSize == 0)
    {
        SHARED_FREE32_MAC(ptr);
    }
    else /* cpssHwDriverGenWmInPexModeGet() also get here */
#endif  /* ASIC_SIMULATION */
    {
        OSMEM_NOPOOLS_HDR_STC *oldHdr;
        OSMEM_POOL_FREE_ELEM_PTR freePtr;
        GT_U32                  windowId;

        if (ptr == NULL)
        {
            return GT_FAIL;
        }

        oldHdr = (OSMEM_NOPOOLS_HDR_STC*)((GT_UINTPTR)ptr - sizeof(OSMEM_NOPOOLS_HDR_STC));
        if (DMA_MEM_MAGIC_MASK(oldHdr->magic) != DMA_MEM_MAGIC)
        {
            fprintf(stderr,"ERROR: failed to free bytes to DMA (magic number)\n");
            return GT_FAIL;
        }

#ifdef ASIC_SIMULATION
        /* take sem */
        osMutexLock(PRV_SHARED_DB.simDmaMutexId);

        /* check if freed space can return to the previous state */
        if ((((GT_CHAR*)(oldHdr+1)) + oldHdr->size) ==
            ((GT_CHAR*)PRV_SHARED_DB.simDmaAsSharedMemPtr + PRV_SHARED_DB.simDmaAsSharedMemOffsetUsed))
        {
            PRV_SHARED_DB.simDmaAsSharedMemOffsetUsed -= oldHdr->size+sizeof(*oldHdr);

            /* free sem */
            osMutexUnlock(PRV_SHARED_DB.simDmaMutexId);

            return GT_OK;
        }

        /* free sem */
        osMutexUnlock(PRV_SHARED_DB.simDmaMutexId);

#endif  /* ASIC_SIMULATION */
        freePtr = (OSMEM_POOL_FREE_ELEM_PTR)oldHdr;
        freePtr->size = oldHdr->size;
        /* fprintf(stderr,"free %d bytes to DMA\n", oldHdr->size); */
        /* put to chain */
        osMutexLock(PRV_SHARED_DB.freeDmaPoolMtx);

        /* use specific window */
        windowId = DMA_MEM_GET_WINDOW_FROM_MAGIC(oldHdr->magic);

        freePtr->next_elem = PRV_SHARED_DMA_window_DB(windowId).freeDmaPool;
        PRV_SHARED_DMA_window_DB(windowId).freeDmaPool = freePtr;

        osMutexUnlock(PRV_SHARED_DB.freeDmaPoolMtx);
    }

  return GT_OK;
}

/**
* @internal osCacheDmaFreeAllByWindow function
* @endinternal
*
* @brief   Free ALL DMA memory
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osCacheDmaFreeAllByWindow(IN GT_U32   windowId)
{
    GT_STATUS rc;
    IS_WRAPPER_OPEN_STATUS;

    /* let the manager clear the DB one by one (will be calling osCacheDmaFree(...))*/
    rc = osCacheDmaSystemReset();

#ifndef ASIC_SIMULATION
    /* reset full DMA in one swipe */
    PRV_SHARED_DMA_window_DB(windowId).cdma_mem.curr = PRV_SHARED_DMA_window_DB(windowId).cdma_mem.base;
#else
    if(PRV_SHARED_DB.simDmaAsSharedMemSize) /* cpssHwDriverGenWmInPexModeGet() also get here */
    {
        /* take sem */
        osMutexLock(PRV_SHARED_DB.simDmaMutexId);
        /* like on HW , we need to forget about all the previous allocations */
        PRV_SHARED_DB.simDmaAsSharedMemOffsetUsed = 0;
        /* release sem */
        osMutexUnlock(PRV_SHARED_DB.simDmaMutexId);
    }
#endif
    osMutexLock(PRV_SHARED_DB.freeDmaPoolMtx);
    PRV_SHARED_DMA_window_DB(windowId).freeDmaPool = NULL;
    osMutexUnlock(PRV_SHARED_DB.freeDmaPoolMtx);

    return rc;
}
/**
* @internal osCacheDmaFreeAll function
* @endinternal
*
* @brief   Free ALL DMA memory
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osCacheDmaFreeAll(GT_VOID)
{
    GT_STATUS   rc,rcFinal = GT_OK;
    GT_U32  windowId;
    GT_U32  numWin;

    osMutexLock(PRV_SHARED_DB.freeDmaPoolMtx);

    numWin = PRV_SHARED_DB.dmaWindowsNumOfActiveWindows ? PRV_SHARED_DB.dmaWindowsNumOfActiveWindows : 1/*support WM*/;

    for(windowId = 0 ; windowId < numWin; windowId++)
    {
        rc = osCacheDmaFreeAllByWindow(windowId);
        if(rc != GT_OK)
        {
            rcFinal = rc;/*save last error*/
        }
    }

    PRV_SHARED_DB.dmaWindowsNumOfActiveWindows = 0;
    PRV_SHARED_DB.dmaWindowsCurrentWindow      = 0;

    osMutexUnlock(PRV_SHARED_DB.freeDmaPoolMtx);

    return rcFinal;
}

#ifndef ASIC_SIMULATION
/* find DMA window by a unique physical address (DMA address) */
static GT_STATUS findWindowByPhyAddr(IN  GT_UINTPTR phyAddr , OUT GT_U32 *windowIdPtr)
{
    GT_U32  windowId;

    for(windowId = 0 ; windowId < PRV_SHARED_DB.dmaWindowsNumOfActiveWindows; windowId++)
    {
        if (PRV_SHARED_DMA_window_DB(windowId).dmaBaseAddr == 0xffffffff)
        {
            continue;
        }
#if __WORDSIZE == 64
        if((PRV_SHARED_DMA_window_DB(windowId).dmaBaseAddr >> 32) &&    /* the actual physical hold 'prefix' */
           (0 == (phyAddr >> 32)))                 /* the physical when read from HW is 32 bits */
        {
            phyAddr += (PRV_SHARED_DMA_window_DB(windowId).dmaBaseAddr >> 32) << 32;
        }
#endif

        if (phyAddr >= PRV_SHARED_DMA_window_DB(windowId).dmaBaseAddr &&
            phyAddr < PRV_SHARED_DMA_window_DB(windowId).dmaBaseAddr +
                      PRV_SHARED_DMA_window_DB(windowId).dmaWindowSize)
        {
            *windowIdPtr = windowId;
            return GT_OK;
        }
    }

    *windowIdPtr = 0;
    return GT_NOT_FOUND;
}

/* find DMA window by a unique virtual address */
static GT_STATUS findWindowByVirtAddr(IN  GT_UINTPTR virtAddr , OUT GT_U32 *windowIdPtr)
{
    GT_U32  windowId;

    for(windowId = 0 ; windowId < PRV_SHARED_DB.dmaWindowsNumOfActiveWindows; windowId++)
    {
        if (PRV_SHARED_DMA_window_DB(windowId).dmaBaseAddr == 0xffffffff)
        {
            continue;
        }

        if (virtAddr >= PRV_SHARED_DMA_window_DB(windowId).dmaBaseVirtAddr &&
            virtAddr < PRV_SHARED_DMA_window_DB(windowId).dmaBaseVirtAddr +
                      PRV_SHARED_DMA_window_DB(windowId).dmaWindowSize)
        {
            *windowIdPtr = windowId;
            return GT_OK;
        }
    }

    *windowIdPtr = 0;
    return GT_NOT_FOUND;
}
#endif /*!ASIC_SIMULATION*/

/**
* @internal osPhy2Virt function
* @endinternal
*
* @brief   Converts physical address to virtual.
*
* @param[in] phyAddr                  - physical address
*
* @param[out] virtAddr                 - virtual address
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osPhy2Virt
(
    IN  GT_UINTPTR phyAddr,
    OUT GT_UINTPTR *virtAddr
)
{
#ifndef ASIC_SIMULATION
    GT_U32  windowId = 0;
#endif
    IS_WRAPPER_OPEN_STATUS;

#ifndef ASIC_SIMULATION
    (void)findWindowByPhyAddr(phyAddr,&windowId);

    if (PRV_SHARED_DMA_window_DB(windowId).dmaBaseAddr == 0xffffffff)
    {
        osPrintf("osPhy2Virt:ERROR,baseAddr(0x%0x)\n", (int)PRV_SHARED_DMA_window_DB(windowId).dmaBaseAddr);
        return GT_FAIL;
    }

#if __WORDSIZE == 64
    if((PRV_SHARED_DMA_window_DB(windowId).dmaBaseAddr >> 32) &&    /* the actual physical hold 'prefix' */
       (0 == (phyAddr >> 32)))/* the physical when read from HW is 32 bits */
    {
        phyAddr += (PRV_SHARED_DMA_window_DB(windowId).dmaBaseAddr >> 32) << 32;
    }
#endif

    if (phyAddr >= PRV_SHARED_DMA_window_DB(windowId).dmaBaseAddr && phyAddr < PRV_SHARED_DMA_window_DB(windowId).dmaBaseAddr + PRV_SHARED_DMA_window_DB(windowId).dmaWindowSize)
    {
        *virtAddr = PRV_SHARED_DMA_window_DB(windowId).dmaBaseVirtAddr + (phyAddr - PRV_SHARED_DMA_window_DB(windowId).dmaBaseAddr);
        return GT_OK;
    }
    else
    {
        fprintf(stderr, "osPhy2Virt:ERROR,phyAddr(0x%0x)\n", (int)phyAddr);
        *virtAddr = 0xffffffff;
        osPrintf("osPhy2Virt:ERROR\n");
        return GT_FAIL;
    }
#else
#if __WORDSIZE == 64
    if(PRV_SHARED_DB.simDmaAsSharedMemSize) /* cpssHwDriverGenWmInPexModeGet() also get here */
    {
        /* we assume that the caller calls this function only for 'real' cases
           that relate to DMA memory
        */

        if(phyAddr & 0xFFFFFFFF00000000L)
        {
            printf("osPhy2Virt : phyAddr [0x %p] must fit in 32 bit \n",phyAddr);
        }
        *virtAddr = ((GT_UINTPTR)PRV_SHARED_DB.simDmaAsSharedMemPtr & 0xFFFFFFFF00000000L) |
                    (phyAddr                                        & 0x00000000FFFFFFFFL);
        return GT_OK;
    }
    else
    {
        /* we don't have ability to 'convert' to specific base as logic gives
           free running 'malloc' every time that could be from different areas
        */
    }
#endif
    *virtAddr = phyAddr;
    return GT_OK;
#endif
}

/**
* @internal osVirt2Phy function
* @endinternal
*
* @brief   Converts virtual address to physical.
*
* @param[in] virtAddr                 - virtual address
*
* @param[out] phyAddr                  - physical address
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osVirt2Phy
(
    IN  GT_UINTPTR virtAddr,
    OUT GT_UINTPTR *phyAddr
)
{
#ifndef ASIC_SIMULATION
    GT_U32  windowId = 0;
#endif
    IS_WRAPPER_OPEN_STATUS;

#ifndef ASIC_SIMULATION
    (void)findWindowByVirtAddr(virtAddr,&windowId);

    if (PRV_SHARED_DMA_window_DB(windowId).dmaBaseAddr == 0xffffffff)
    {
        /*ofPrintf(stderr, "osVirt2Phy:ERROR,baseAddr(0x%0x)\n", (int)baseAddr);*/
        return GT_FAIL;
    }

    if(virtAddr >= PRV_SHARED_DMA_window_DB(windowId).dmaBaseVirtAddr &&
       virtAddr < (PRV_SHARED_DMA_window_DB(windowId).dmaBaseVirtAddr+PRV_SHARED_DMA_window_DB(windowId).dmaWindowSize))
    {
        /* the mask 0xFFFFFFFF to support 'high' PHYSICALs that the switch support
           'high address' mapping to it */
        *phyAddr = (PRV_SHARED_DMA_window_DB(windowId).dmaBaseAddr & 0xFFFFFFFF) + (virtAddr - PRV_SHARED_DMA_window_DB(windowId).dmaBaseVirtAddr);
        /*osPrintf("\nosVirt2Phy: *phyAddr 0x%X\n",*phyAddr);*/
        return GT_OK;
    }
    else
    {
        fprintf(stderr, "osVirt2Phy:ERROR,virtAddr(0x%0x)\n", (int)virtAddr);
        *phyAddr = 0xffffffff;
        /*osPrintf("osVirt2Phy:ERROR2\n");*/
        return GT_FAIL;
    }
#else
#if __WORDSIZE == 64
    if(PRV_SHARED_DB.simDmaAsSharedMemSize) /* cpssHwDriverGenWmInPexModeGet() also get here */
    {
        /* we assume that the caller calls this function only for 'real' cases
           that relate to DMA memory
        */

        if((virtAddr & 0xFFFFFFFF00000000L) !=
           ((GT_UINTPTR)PRV_SHARED_DB.simDmaAsSharedMemPtr & 0xFFFFFFFF00000000L))
        {
            printf("osVirt2Phy : virtAddr [0x %p] must match range with 'simDmaAsSharedMemPtr' [0x %p] \n",
                virtAddr,(GT_UINTPTR)PRV_SHARED_DB.simDmaAsSharedMemPtr);
        }
        *phyAddr  = virtAddr & 0x00000000FFFFFFFFL;
        return GT_OK;
    }
    else
    {
        /* we don't have ability to 'convert' to specific base as logic gives
           free running 'malloc' every time that could be from different areas
        */
    }
#endif
    *phyAddr = virtAddr;
    return GT_OK;
#endif
}

/***** Private ********************************************************/

#ifdef ASIC_SIMULATION
/*******************************************************************************
* dmaAsSharedMemoryMalloc
*
* DESCRIPTION:
*       Allocates shared memory block of specified size, this shared memory will
*       be used for 'DMA memory'
*
* INPUTS:
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       pointer to the shared memory (on NULL on fail)
*
* COMMENTS:
*       Usage of this function is only on FIRST initialization.
*       used only when distributed simulation of application side is working
*       with broker , in mode of DMA as shared memory(shared between the
*       broker and the application).
*
*******************************************************************************/
static GT_VOID* dmaAsSharedMemoryMalloc
(
    IN GT_U32 size
)
{
    GT_STATUS   rc;

    rc = simOsSharedMemGet((GT_SH_MEM_KEY)SIM_DMA_AS_SHARED_MEM_KEY_CNS, size, 0,
                           &PRV_SHARED_DB.simDmaAsSharedMemId);
    if(rc != GT_OK)
    {
        return NULL;
    }

    /* Now we attach (map) the shared memory to our process memory */
    PRV_SHARED_DB.simDmaAsSharedMemPtr = simOsSharedMemAttach(PRV_SHARED_DB.simDmaAsSharedMemId,
            NULL ,/* NULL means we attach it to no specific memory , we let OS to decide */
            0);/*no specific flags*/

    if(PRV_SHARED_DB.simDmaAsSharedMemPtr)
    {
        PRV_SHARED_DB.simDmaAsSharedMemSize = size;
    }

    return PRV_SHARED_DB.simDmaAsSharedMemPtr;
}
#endif /* ASIC_SIMULATION */

/*******************************************************************************
* osMemGlobalDbShmemInit
*
* DESCRIPTION:
*      Initialize process interface to shared lib:
*     Create a shared memory file, if one does not yet exist
*   Map shared memory to process address space
*
* INPUTS:
*       size - bytes to allocate
*       name - shared lib name
*
* OUTPUTS:
*       initDataSegmentPtr - whether init data
*      sharedGlobalVarsPtrPtr - pointer to shared memory
*
*
* RETURNS:
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* COMMENTS: none
*
*
*******************************************************************************/
GT_STATUS osMemGlobalDbShmemInit
(
    IN  GT_U32       size,
    IN  GT_CHAR_PTR  name,
    OUT GT_BOOL      * initDataSegmentPtr,
    OUT GT_VOID      **sharedGlobalVarsPtrPtr
)
{
    GT_32 shm_fd = -1;
    GT_BOOL initialize = GT_TRUE;

    GT_VOID_PTR sharedMem;
    GT_U32 initializerPid;
    struct stat st;
    GT_BOOL sizeMatch = GT_FALSE;
    GT_U32  numberOfActiveClients;
    GT_STATUS rc;
    GT_CHAR buff[256];
    GT_CHAR fullPathBuff[256];
#ifndef ASIC_SIMULATION
    uint64_t   physAddr;
#endif
    if(NULL == initDataSegmentPtr ||
       NULL ==sharedGlobalVarsPtrPtr||
       NULL ==name)
    {
        return GT_BAD_PTR;
    }

    if (size == 0 || size > 0x20000000)
    {
        osPrintf("%s: Error - size out of range\n", __func__);
        return GT_OUT_OF_RANGE;
    }

#if defined SHARED_MEMORY
    shrAddNameSuffix(name,buff,sizeof(buff));
#endif
    osSprintf(fullPathBuff, "/dev/shm/%s",buff);

    /*It is not enoght that file exist.
            The size should match.*/
    if (stat(fullPathBuff, &st) == 0)
    {
        if ((GT_U32)st.st_size == size)
        {
            /* Check if file already exists */
            shm_fd = shm_open(buff, O_RDWR, 0);
            sizeMatch = GT_TRUE;
        }
    }

    if (GT_TRUE==sizeMatch&&shm_fd >= 0)
    {
        /* If already exists, read PID of creator and check if alive */
        sharedMem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (sharedMem == MAP_FAILED) {
            osPrintf("%s: Error %d - cannot create shared mem file %s\n", __func__, errno,  name);
            close(shm_fd);
            return GT_FAIL;
        }
        *sharedGlobalVarsPtrPtr = sharedMem;

        rc = prvOsHelperGlobalDbInitParamsGet(&numberOfActiveClients,&initializerPid);
        if(rc!=GT_OK)
        {
           return rc;
        }
        if (numberOfActiveClients > 0) {
            /* There are alive clients */
            initialize = GT_FALSE;
            SHM_PRINT("Shared lib already initialized by proc %d. Number of active procs %d\n", initializerPid,numberOfActiveClients);
        }
        close(shm_fd);
    }


    if (initialize) {
        *initDataSegmentPtr = GT_TRUE;
        /* Couldn't find valid shared memory file, need to (re)create one */
        shm_fd = shm_open(buff, O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (shm_fd <0) {
            osPrintf("%s: Error %d - cannot create shared mem file %s\n", __func__, errno,  buff);
            return GT_FAIL;
        }
        SHM_PRINT("Initializing shared lib\n");
        /* Set the file to requested size */
        if(ftruncate(shm_fd, size)) {
            osPrintf("%s: Failed to truncate shared mem file %s to size 0x%x\n", __func__, buff, size);
            close(shm_fd);
            return -1;
        }
        /* Map the shared memory file */
        sharedMem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (sharedMem == MAP_FAILED) {
            osPrintf("%s: Failed to map shared mem\n", __func__);
            close(shm_fd);
            return -1;
        }

        *sharedGlobalVarsPtrPtr = sharedMem;
        /* Update the creator PID to self */
         *((GT_U32 *)sharedMem) = (GT_U32)getpid();
    }

#ifndef ASIC_SIMULATION

    /*
            Lock a shared-memory segment into physical memory to eliminate paging.
            The mlock  system call  tell the system to lock to a specified memory range, and to not allow that memory to be paged.
            This means that once the physical page has been allocated to the page table entry, references to that page will always be fast.
    */
    if(mlock(sharedMem,size) != 0)
    {
      perror("mlock failure");
      close(shm_fd);
      return -1;
    }
    physAddr = linux_virt_to_phys((uintptr_t)sharedMem);
    SHM_PRINT("Shared memory mapped at physical address  h = 0x%08x l= 0x%08x \n",
            (((physAddr)>>32)& 0x00000000ffffffffL),physAddr& 0x00000000ffffffffL);
#endif

    SHM_PRINT("Shared memory mapped at virtual address  %p.[Originally created by process %d ].\n", sharedMem,*((pid_t *)sharedMem));

    return GT_OK;
}

/*


*/
/*******************************************************************************
 * osMemGlobalDbShmemUnlink
 *
 * DESCRIPTION:
 *    Deletes a name from the file system.
 *    If that name was the last link to a file and no processes have the file open the file is deleted and the space it was using is made available for reuse.
*    If the name was the last link to a file but any processes still have the file open the file will remain in existence until the last file descriptor referring to it is closed.
 *
 * INPUTS:
 *       name - name of shared library
 *
 * OUTPUTS:
 *       initDataSegmentPtr - whether init data
 *      sharedGlobalVarsPtrPtr - pointer to shared memory
 *
 *
 * RETURNS:
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 *
 * COMMENTS: none
 *
 *
 *******************************************************************************/

GT_STATUS osMemGlobalDbShmemUnlink
(
    IN  GT_CHAR_PTR  name
)
{
    GT_CHAR   buff[256];
    GT_STATUS rc = GT_OK;
    if(NULL ==name)
    {
        return GT_BAD_PTR;
    }

#if defined SHARED_MEMORY
    shrAddNameSuffix(name,buff,sizeof(buff));
#endif
    osPrintf ("Shared memory [%s]  ", buff);

    if(unlink(buff) < 0)
    {
        if(errno == EBUSY)
        {
           osPrintf ("cannot be unlinked because it is being used by the system or another proces\n");
        }
        else if(errno == ENOENT)
        {
           osPrintf ("No such file or directory\n");
        }
        else if(0 != errno)
        {
            osPrintf ("unmap error  %s\n", strerror(errno));
            rc = GT_FAIL;
        }
    }
    else
    {
        osPrintf("unmapped\n");
    }

    return rc;
}

GT_STATUS shrMemSharedDmaInit
(
    GT_VOID
)
{
    if(NULL==osSharedGlobalVarsPtr)
    {
        return GT_BAD_PTR;
    }
    PRV_SHARED_DMA_DB.dmaWindowSize   = 2 * (1024 * 1024); /*default value*/
    PRV_SHARED_DMA_DB.dmaBaseVirtAddr = LINUX_VMA_DMABASE + PRV_DMA_WINDOW * PRV_SHARED_DMA_DB.dmaWindowSize;

    return GT_OK;
}

GT_STATUS osPrintDmaPhysicalBase(GT_VOID)
{
    GT_UINTPTR pbase,vbase,windowId;
    GT_STATUS rc,rcFinal = GT_OK;
    GT_U32  numWin;

    numWin = PRV_SHARED_DB.dmaWindowsNumOfActiveWindows ? PRV_SHARED_DB.dmaWindowsNumOfActiveWindows : 1/*support WM*/;

    osPrintf ("dmaWindowsNumOfActiveWindows[%d]\n", PRV_SHARED_DB.dmaWindowsNumOfActiveWindows);

    for(windowId = 0 ; windowId < numWin; windowId++)
    {
        vbase = PRV_SHARED_DMA_window_DB(windowId).dmaBaseVirtAddr;
        rc = osVirt2Phy(vbase,&pbase);

        if(GT_OK==rc && vbase)
        {
            osPrintf ("DMA address phys %p ,virtual %p window[%d]\n", pbase,vbase,windowId);
        }
        else if (GT_OK != rc && vbase)
        {
            rcFinal = rc;
        }
    }

    return rcFinal;
}


/* STUB for WM */
#ifdef ASIC_SIMULATION
/**
* @internal shrDmaByWindowInit function
* @endinternal
*
* @brief   synch the mainOs info about DMA on specific window , after the DMA was
*          already allocated by the call to mvDmaDrv.ko or to 'huge page' allocation .
*
* @note
*
*/
GT_STATUS shrDmaByWindowInit(IN GT_BOOL isFirstClient,IN GT_U32 dmaWindowId) /* STUBS - supporting only single window */
{
    if(dmaWindowId != 0 || isFirstClient != GT_TRUE)
    {
        return GT_BAD_PARAM;
    }

    return GT_OK;
}
#endif /*ASIC_SIMULATION*/
