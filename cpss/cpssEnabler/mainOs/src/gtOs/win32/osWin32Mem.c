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
* @file osWin32Mem.c
*
* @brief Win32 Operating System Simulation. Memory manipulation facility.
*
* @version   24
********************************************************************************
*/
/* IMPORTANT : we must remove the declaration of OS_MALLOC_MEMORY_LEAKAGE_DBG */
/* because we not want local calls to osMalloc to be affected by the H file definitions */
#define FORCE_IGNORE_OS_MALLOC_MEMORY_LEAKAGE_DBG

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsExc.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>

/*********  important ****  important  ***  important ****  important **********
NOTE: in this file the OS_MALLOC_MEMORY_LEAKAGE_DBG is NEVER defined
    instead this file must check
    INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG
    instead of
    OS_MALLOC_MEMORY_LEAKAGE_DBG
*********  important ****  important  ***  important ****  important **********/

/* include simulation H files for shared memory issues */
/* next macro defined so the file of <os/simOsProcess.h> will not #error us */
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#include <os/simTypesBind.h>
#include <os/simOsProcess.h>
#include <asicSimulation/SDistributed/sdistributed.h>
#include <asicSimulation/SInit/sinit.h>

/***** External Functions **********************************************/
extern int osPrintf(const char* format, ...);
void *osMemPoolsAlloc(GT_U32 size);
void *osMemPoolsRealloc(void *ptr, GT_U32 size);
void osMemPoolFree(void *memBlock);
void *osStaticMallocFromChunk(GT_U32 size);

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
/***** Static functions ************************************************/

/***** Static variables ************************************************/
/* heap allocation measurement static variables */

GT_BOOL  firstInitialization = GT_TRUE;

/***** Public Functions ************************************************/
/* a number that indicate the 'key' representing the shared memory */
#define SIM_DMA_AS_SHARED_MEM_KEY_CNS   5678

/* id of the shared memory */
static GT_SH_MEM_ID simDmaAsSharedMemId = 0;

static GT_MUTEX simDmaMutexId;
/* pointer to the start of the shared memory */
static void* simDmaAsSharedMemPtr=NULL;

/* size of the shared memory */
static GT_U32 simDmaAsSharedMemSize = 0;

/* offset ued from start of the shared memory  */
static GT_U32 simDmaAsSharedMemOffsetUsed = 0;

/* start protect win32 calles that uses internally semaphore ,
    this to avoid deadlock when a task in inside win32 function that takes
    semaphore and we 'lock' this task from other task that may also call win32 for
    same/other function and will be locked because win32 semaphore is not released */
/*
protect next:
    ClearCommError , ReadFile , WriteFile

*/
#define PROTECT_WIN32_CALL_MAC(win32FuncWithParameters) \
    scibAccessLock();                                   \
    win32FuncWithParameters;                            \
    scibAccessUnlock()


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
static void* dmaAsSharedMemoryMalloc
(
    IN GT_U32 size
)
{
    GT_STATUS   rc;

    rc = simOsSharedMemGet((GT_SH_MEM_KEY)SIM_DMA_AS_SHARED_MEM_KEY_CNS,size,0,&simDmaAsSharedMemId);
    if(rc != GT_OK)
    {
        return NULL;
    }

    /* Now we attach (map) the shared memory to our process memory */
    simDmaAsSharedMemPtr = simOsSharedMemAttach(simDmaAsSharedMemId,
            NULL ,/* NULL means we attach it to no specific memory , we let OS to decide */
            0);/*no specific flags*/

    if(simDmaAsSharedMemPtr)
    {
        simDmaAsSharedMemSize = size;
    }

    return simDmaAsSharedMemPtr;
}

#if __WORDSIZE == 64
static GT_VOID* malloc64_wrapper(unsigned int size)
{
    return malloc((size_t)size);
}
#endif

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
    GT_U32      dmaSize;
    GT_UINTPTR  dmaBase;
    GT_STATUS   rc;

    if (reallocOnDemand == GT_TRUE)
    {
        memAllocMethod = GT_MEMORY_MANAGE_METHOD_REALLOC_ON_DEMAND_E;
    }
    else
    {
        memAllocMethod = GT_MEMORY_MANAGE_METHOD_NO_REALLOC_ON_DEMAND_E;
    }

    /* Create the simDma semaphore */
    if (osMutexCreate("osSimDma", &simDmaMutexId) != GT_OK)
    {
        return GT_FAIL;
    }

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
    simDmaAsSharedMemPtr  = (void*)dmaBase;
    simDmaAsSharedMemSize = dmaSize;
    if(simDmaAsSharedMemPtr == NULL || dmaSize == 0)
    {
        return GT_NO_RESOURCE;
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
            simDistributedRegisterDma((GT_U32)(GT_UINTPTR)simDmaAsSharedMemPtr,/* the address of the pointer in our memory */
                    simDmaAsSharedMemSize,/* the size of shared memory */
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

#if __WORDSIZE == 64
    ret = osMemLibInit(malloc64_wrapper,free,size, memAllocMethod);
#else
    ret = osMemLibInit(malloc,free,size, memAllocMethod);
#endif
    firstInitialization = GT_FALSE;
    return ret;
}

/*******************************************************************************
* osStaticAlloc
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
void *osStaticMalloc
(
    IN GT_U32 size
)
{
    void *ptr;
    if (firstInitialization == GT_TRUE)
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
    IN GT_U32   size
)
{
    if( size == 0) return NULL;

    if (firstInitialization == GT_TRUE)
    {
        /* during first init allocate from the OS */
        OSMEM_NOPOOLS_HDR_STC *hdrPtr;
        hdrPtr = (OSMEM_NOPOOLS_HDR_STC*)malloc(size+sizeof(OSMEM_NOPOOLS_HDR_STC));
        if (hdrPtr == NULL)
        {
            return NULL;
        }
        hdrPtr->magic = (GT_UINTPTR)hdrPtr;
        hdrPtr->size = size;
        hdrPtr++;
        return (GT_VOID*)hdrPtr;
    }

    return osMemPoolsAlloc(size);
}

/*******************************************************************************
*   internalOsRealloc
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
    IN GT_U32    size
)
{
    OSMEM_NOPOOLS_HDR_STC *oldHdr;

    if (ptr == NULL)
    {
        return osMalloc(size);
    }

    oldHdr = (OSMEM_NOPOOLS_HDR_STC*)((GT_UINTPTR)ptr - sizeof(OSMEM_NOPOOLS_HDR_STC));
    if (firstInitialization == GT_TRUE)
    {
        OSMEM_NOPOOLS_HDR_STC *allocPtr;
        if (oldHdr->magic != (GT_UINTPTR)oldHdr)
        {
            osFatalError(OS_FATAL_WARNING, "osRealloc(): bad magic");
        }
        allocPtr = realloc(oldHdr, size+sizeof(OSMEM_NOPOOLS_HDR_STC));
        if (allocPtr == NULL)
        {
            return NULL;
        }
        allocPtr->magic = (GT_UINTPTR)allocPtr;
        allocPtr->size = size;
        return (GT_VOID*)(allocPtr + 1);
    }
    if (oldHdr->magic == (GT_UINTPTR)oldHdr)
    {
        /* realloc memory allocated by malloc() */
        GT_VOID *newPtr = osMalloc(size);
        if (newPtr == NULL)
        {
            return NULL;
        }
        osMemCpy(newPtr, ptr, oldHdr->size < size ? oldHdr->size : size);
/*#ifdef OSMEMPOOLS_DEBUG
        osPrintf("Warning:Trying to Free a pointer allocated by malloc after osMemInit !\n");
#endif OSMEMPOOLS_DEBUG bothers CLI*/
        free(oldHdr);
        return newPtr;
    }
    return osMemPoolsRealloc(ptr, size);
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
    if (firstInitialization == GT_TRUE)
    {
        if (oldHdr->magic != (GT_UINTPTR)oldHdr)
        {
            osFatalError(OS_FATAL_WARNING, "osFree(): bad magic");
        }
        free(oldHdr);
    }
    else
    {
        /* check if this free is trying to free something that wasn't
           allocated by owr pools manager */
        if (oldHdr->magic == (GT_UINTPTR)oldHdr)
        {
/*#ifdef OSMEMPOOLS_DEBUG
            osPrintf("Warning:Trying to Free a pointer allocated by malloc after osMemInit !\n");
#endif OSMEMPOOLS_DEBUG bothers CLI*/
            free(oldHdr);
        }
        else
        {
            osMemPoolFree(memblock);
        }
    }
    return;
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
*       None
*
*******************************************************************************/
void *osMalloc
(
    IN GT_U32 size
)
{
    void *ptr;
    ptr = internalOsMalloc(size);

    osMemRegisterPtr(ptr,size,LOCAL_DEBUG_INFO);
    return ptr;
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
*       None
*
*******************************************************************************/
void *osRealloc
(
    IN void * ptr ,
    IN GT_U32 size
)
{
    void *newPtr;
    newPtr = internalOsRealloc(ptr,size);

    /* unregister old ptr */
    osMemUnregisterPtr(ptr,LOCAL_DEBUG_INFO);
    /* register new ptr */
    osMemRegisterPtr(newPtr,size,LOCAL_DEBUG_INFO);
    return newPtr;
}

/**
* @internal osFree function
* @endinternal
*
* @brief   Deallocates or frees a specified memory block.
*/
void osFree
(
    IN void* const memblock
)
{
    internalOsFree(memblock);
    osMemUnregisterPtr(memblock,LOCAL_DEBUG_INFO);
}

/*******************************************************************************
* osCacheDmaMalloc
*
* DESCRIPTION:
*       Allocate a cache-safe buffer  of specified size for DMA devices
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
void *osCacheDmaMalloc
(
    IN GT_U32 size
)
{
    void * ptr;

    if (size == 0)
        return NULL;

    /* check if need to get modified size to hold support for 'overflow detection' */
    prvOsCacheDmaMallocOverflowDetectionSupportGet(&size);

    if(simDmaAsSharedMemSize) /* cpssHwDriverGenWmInPexModeGet() also get here */
    {
        char* tmpPtr; /* tmp pointer to start of memory to return */
        /* take sem */
        osMutexLock(simDmaMutexId);

        /* check if we have enough space for the new memory request */
        if((simDmaAsSharedMemOffsetUsed + size) > simDmaAsSharedMemSize)
        {
            /* free sem */
            osMutexUnlock(simDmaMutexId);
            return NULL;
        }

        tmpPtr = (char*)simDmaAsSharedMemPtr + simDmaAsSharedMemOffsetUsed;

        simDmaAsSharedMemOffsetUsed += size;

        /* free sem */
        osMutexUnlock(simDmaMutexId);

        ptr = tmpPtr;
    }
    else
    {
        ptr =  malloc(size); /*TODO: os_mem_chunks_array.memAllocFuncPtr(size); */
    }

    /* register the dma pointer for system reset */
    osCacheDmaRegisterPtr(ptr,size);

    return ptr;
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
    IN void *ptr
)
{
    /* unregister the ptr */
    (void)osCacheDmaUnregisterPtr(ptr);

    if(simDmaAsSharedMemSize) /* cpssHwDriverGenWmInPexModeGet() also get here */
    {
        /* not supporting free of this memory */
        return GT_OK;
    }


    free (ptr);
    return GT_OK;
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
    GT_STATUS rc;

   /* let the manager clear the DB one by one (will be calling osCacheDmaFree(...))*/
    rc = osCacheDmaSystemReset();

    if(simDmaAsSharedMemSize) /* cpssHwDriverGenWmInPexModeGet() also get here */
    {
        /* take sem */
        osMutexLock(simDmaMutexId);
        /* like on HW , we need to forget about all the previous allocations */
        simDmaAsSharedMemOffsetUsed = 0;
        /* release sem */
        osMutexUnlock(simDmaMutexId);
    }

    return rc;
}

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
#if __WORDSIZE == 64
    if(simDmaAsSharedMemSize) /* cpssHwDriverGenWmInPexModeGet() also get here */
    {
        /* we assume that the caller calls this function only for 'real' cases
           that relate to DMA memory
        */

        if(phyAddr & 0xFFFFFFFF00000000L)
        {
            printf("osPhy2Virt : phyAddr [0x %p] must fit in 32 bit \n",phyAddr);
        }
        *virtAddr = ((GT_UINTPTR)simDmaAsSharedMemPtr & 0xFFFFFFFF00000000L) |
                    (phyAddr                          & 0x00000000FFFFFFFFL);
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
#if __WORDSIZE == 64
    if(simDmaAsSharedMemSize) /* cpssHwDriverGenWmInPexModeGet() also get here */
    {
        /* we assume that the caller calls this function only for 'real' cases
           that relate to DMA memory
        */

        if((virtAddr & 0xFFFFFFFF00000000L) != ((GT_UINTPTR)simDmaAsSharedMemPtr & 0xFFFFFFFF00000000L))
        {
            printf("osVirt2Phy : virtAddr [0x %p] must match range with 'simDmaAsSharedMemPtr' [0x %p] \n",
                virtAddr,(GT_UINTPTR)simDmaAsSharedMemPtr);
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
}


/**
* @internal osGlobalDbDmaActiveWindowByPcieParamsSet function
* @endinternal
*
* @brief   function to set the current window Id for DMA chunk allocation to be
*          the one that should be associated with the PCIe device.
*          the function sets a new current window Id and returns this window to application.
*
* INPUTS:
*        pciDomain  - the PCI domain
*        pciBus     - the PCI bus
*        pciDev     - the PCI device
*        pciFunc    - the PCI function
*
* OUTPUTS:
*       windowPtr   - (pointer to) the window Id that associated with the PCIe device
*                   ignored if NULL
*
* @retval  GT_OK                    - if the PCIe parameters found to be for device
*                                   that was seen during PCIe scan of the extDrv
*          GT_NOT_FOUND             - if not found
*/
GT_STATUS osGlobalDbDmaActiveWindowByPcieParamsSet(/* STUBS - supporting only single window */
    IN GT_U32  pciDomain,
    IN GT_U32  pciBus,
    IN GT_U32  pciDev,
    IN GT_U32  pciFunc,
    OUT GT_U32  *windowPtr/* ignored if NULL*/
)
{
    GT_UNUSED_PARAM(pciDomain);
    GT_UNUSED_PARAM(pciBus);
    GT_UNUSED_PARAM(pciDev);
    GT_UNUSED_PARAM(pciFunc);
    *windowPtr = 0;
    return GT_OK;
}

/**
* @internal osGlobalDbDmaActiveWindowSet function
* @endinternal
*
* @brief   function to set the current window for DMA chunk allocation.
*
* INPUTS:
*       window      - the window Id to used as active.
*
* @retval  GT_OK                    - if window in valid range
*          GT_BAD_PARAM             - if window not in valid range
*/
GT_STATUS osGlobalDbDmaActiveWindowSet(/* STUBS - supporting only single window */
    IN GT_U32  window
)
{
    if(window != 0)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return GT_OK;
}

/**
* @internal osGlobalDbDmaActiveWindowGet function
* @endinternal
*
* @brief   function to get the current window used for DMA chunk allocation.
*
* OUTPUTS:
*       windowPtr   - (pointer to) the window Id
*
*
* @retval  GT_OK                  - on success
*          GT_BAD_PTR             - if windowPtr is NULL pointer
*/
GT_STATUS osGlobalDbDmaActiveWindowGet(/* STUBS - supporting only single window */
    OUT GT_U32  *windowPtr
)
{
    *windowPtr = 0;
    return GT_OK;
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
GT_VOID *osCacheDmaMallocByWindow /* STUBS - supporting only single window */
(
    IN GT_U32 windowId,
    IN GT_U32 size
)
{
    if(windowId != 0)
    {
        return NULL;
    }

    return osCacheDmaMalloc(size);
}

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

