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
* @file shrMemPpInit.c
*
* @brief File contains routines for Packet Processor initialization
* (applicable ONLY for BM).
* Also routine for sections mapping registry is implemented here.
*
*
* @version   9
********************************************************************************
*/

/************* Includes *******************************************************/
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsSharedMemory.h>
#include <gtOs/gtOsSharedMemoryRemapper.h>
#include <gtOs/gtOsSharedMalloc.h>
#include <gtOs/gtOsSharedPp.h>
#include <gtOs/gtOsSharedData.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>


/***** Global Vars access ********************************************/


#define PRV_SHARED_MAIN_OS_DIR_PP_INIT_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_HELPER_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.gtOsShmemPpInitSrc._var,_value)

#define PRV_SHARED_MAIN_OS_DIR_PP_INIT_SRC_GLOBAL_VAR_GET(_var)\
    PRV_HELPER_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.gtOsShmemPpInitSrc._var)



GT_32 gtPpFd = -1;


static char *dma_name = "DMA";

#define PRV_SHARED_DB                  osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.osLinuxMemSrc
#define PRV_SHARED_DMA_window_DB(_win) osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.osLinuxMemSrc.dmaWindowsArr[_win]

/***** External Functions **********************************************/
GT_STATUS extDrvInitDrv(OUT GT_BOOL *isFirstClient);
GT_STATUS extDrvGetDmaBase(OUT GT_UINTPTR * dmaBase);
GT_STATUS extDrvGetDmaVirtBase(OUT GT_UINTPTR *dmaVirtBase);
GT_STATUS extDrvGetDmaSize(OUT GT_U32 * dmaSize);
GT_STATUS shrMemSharedDmaInit
(
    GT_VOID
);

GT_STATUS osPrintDmaPhysicalBase(GT_VOID);



#ifdef SHARED_MEMORY


/************* Functions ******************************************************/
/************* Public Functions ***********************************************/

/**
* @internal shrMemRegisterPpMmapSection function
* @endinternal
*
* @brief   Register section which should be mmaped by all non-first clients.
*
* @param[in] startPtr                 - start address to be mapped
* @param[in] length                   -  of block to be mapped
* @param[in] offset                   -  into file to be mapped
*                                       GT_OK on success
*
* @retval GT_OUT_OF_RANGE          - too many sections for static registry,
*                                       must be increased
*
* @note Operation is relevant only for SHARED_MEMORY=1
*       Also operation is relevant only for undefined ASIC_SIMULATION (BlackMode).
*
*/
GT_STATUS shrMemRegisterPpMmapSection
(
    IN GT_VOID *startPtr,
    IN size_t  length,
    IN off_t   offset
)
{
    PP_MMAP_INFO_STC *infoPtr;
    GT_STATUS         retVal;
    GT_BOOL           aslrSupport = osNonSharedGlobalVars.osNonVolatileDb.aslrSupport;

    /*init global Db here for pp_mmap_registry_index*/
    retVal = gtOsHelperGlobalDbInit(aslrSupport,__func__,__LINE__);
    if(retVal != GT_OK)
    {
        return retVal;
    }

    if(PRV_SHARED_MAIN_OS_DIR_PP_INIT_SRC_GLOBAL_VAR_GET(pp_mmap_registry_index) == PP_REGISTRY_SIZE_CNS)
    {
        fprintf(stderr, "shrMemRegisterPpMmapSection: index out of space, please increase PP_REGISTRY_SIZE_CNS\n");
        return GT_OUT_OF_RANGE;
    }
    infoPtr = &(PRV_SHARED_MAIN_OS_DIR_PP_INIT_SRC_GLOBAL_VAR_GET(pp_mmap_sections_registry[PRV_SHARED_MAIN_OS_DIR_PP_INIT_SRC_GLOBAL_VAR_GET(pp_mmap_registry_index++])));

    /* Store required variables into registry */
    infoPtr->startPtr  = startPtr;
    infoPtr->length    = length;
    infoPtr->offset    = offset;

    return GT_OK;
}
#endif /* SHARED_MEMORY */

/*******************************************************************************
* shrMemSharedPpInit
*
* DESCRIPTION:
*       Performs sharing operations for DMA special device file.
*
* INPUTS:
*       isFirstClient - the-first-client flag
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       Special device file /dev/mvPP is handled by presera kernel module.
*       mmap operation with fixed addresses will be redirected to prestera_mmap
*       to do special operations with DMA, register and config space.
*
*       Operation is relevant only for BlackMode (undefined ASIC_SIMULATION).
*
*******************************************************************************/
#ifndef SHARED_MEMORY
int adv64_malloc_32bit __SHARED_DATA_MAINOS = 0;
#endif

/* init DMA related memory */
GT_STATUS shrDmaByWindowInit(IN GT_BOOL isFirstClient,IN GT_U32 dmaWindowId)
{
    GT_STATUS retVal;

    if(dmaWindowId >= GT_MEMORY_DMA_CONFIG_WINDOWS_CNS)
    {
        return GT_BAD_PARAM;
    }

    if(dmaWindowId >= PRV_SHARED_DB.dmaWindowsNumOfActiveWindows)
    {
        /* update the DB to the hold the needed window */
        PRV_SHARED_DB.dmaWindowsNumOfActiveWindows = dmaWindowId + 1;
    }

    if(isFirstClient)
    {
        /* this is first time that extDrvGetDmaBase() is called , and this function
           is expected to do the actual mmup of DMA with the mvDmaDrv.ko or with
           the 'HUGE_PAGES' */

        /* Get physical address of the dma area need for Virt2Phys and Phys2Virt */
        extDrvGetDmaBase(&(PRV_SHARED_DMA_window_DB(dmaWindowId).dmaBaseAddr));
        extDrvGetDmaVirtBase(&(PRV_SHARED_DMA_window_DB(dmaWindowId).dmaBaseVirtAddr));
        /* Map shared buffer and init management structure. */
        extDrvGetDmaSize(&(PRV_SHARED_DMA_window_DB(dmaWindowId).dmaWindowSize));
        if (PRV_SHARED_DMA_window_DB(dmaWindowId).dmaWindowSize & 0x80000000)
        {
            PRV_SHARED_DMA_window_DB(dmaWindowId).dmaWindowSize &= ~0x80000000;
#ifndef SHARED_MEMORY
            adv64_malloc_32bit = 1;
#endif
        }
        shmPrintf("DMA buffer initialization: name=%s, addr=0x%p, size=%X\n", dma_name,
                  (void *)(PRV_SHARED_DMA_window_DB(dmaWindowId).dmaBaseVirtAddr), PRV_SHARED_DMA_window_DB(dmaWindowId).dmaWindowSize);
        retVal = shrMemSharedBufferInit(dma_name, PRV_SHARED_DMA_window_DB(dmaWindowId).dmaBaseVirtAddr,
                                        PRV_SHARED_DMA_window_DB(dmaWindowId).dmaWindowSize, -1,
                                        GT_FALSE, &(PRV_SHARED_DMA_window_DB(dmaWindowId).cdma_mem));
        if (GT_OK != retVal)
            return GT_NO_RESOURCE;
    }
    else
    {
#ifdef SHARED_MEMORY

#else
        /* Without shared memory approach only SINGLE client is accessible.
         * So it should be the only first one.*/
        return GT_BAD_VALUE;
#endif
    }

    return GT_OK;
}

GT_STATUS shrMemSharedPpInit(IN GT_BOOL isFirstClient)
{
    GT_STATUS retVal;
    GT_BOOL           aslrSupport = osNonSharedGlobalVars.osNonVolatileDb.aslrSupport;
#ifndef SHARED_MEMORY
    GT_BOOL first;

    retVal = extDrvInitDrv(&first);
    if (retVal == GT_OK)
    {
        if (first == GT_TRUE)
            isFirstClient = GT_TRUE;
    }
    else
    {
        if (retVal != GT_ALREADY_EXIST)
            return retVal;
    }
#endif

    if(isFirstClient)
    {
        /*init global Db here for the first client since we need to remember DMA base*/
        retVal = gtOsHelperGlobalDbInit(aslrSupport,__func__,__LINE__);
        if(retVal != GT_OK)
        {
            return retVal;
        }
        retVal = shrMemSharedDmaInit();
        if(retVal != GT_OK)
        {
            return retVal;
        }

        retVal = shrDmaByWindowInit(isFirstClient,0/*DMA window*/);
        if(retVal != GT_OK)
        {
            return retVal;
        }

    }
    else
    {
#ifdef SHARED_MEMORY
        int i=0;

        /*init global Db here for pp_mmap_registry_index*/
        retVal = gtOsHelperGlobalDbInit(aslrSupport,__func__,__LINE__);
        if(retVal != GT_OK)
        {
            return retVal;
        }
        /* All other clients just map sections from registry */
        while(i<PRV_SHARED_MAIN_OS_DIR_PP_INIT_SRC_GLOBAL_VAR_GET(pp_mmap_registry_index))
        {
            /* Map registry */
            shmPrintf("DMA buffer remapping: name=%s, addr=0x%X, size=%X, offset=%X\n", dma_name,
                  PRV_SHARED_MAIN_OS_DIR_PP_INIT_SRC_GLOBAL_VAR_GET(pp_mmap_sections_registry[i].startPtr),
                  PRV_SHARED_MAIN_OS_DIR_PP_INIT_SRC_GLOBAL_VAR_GET(pp_mmap_sections_registry[i].length),
                  PRV_SHARED_MAIN_OS_DIR_PP_INIT_SRC_GLOBAL_VAR_GET(pp_mmap_sections_registry[i].offset));
            retVal = shrMemMmapMemory((GT_UINTPTR)PRV_SHARED_MAIN_OS_DIR_PP_INIT_SRC_GLOBAL_VAR_GET(pp_mmap_sections_registry[i].startPtr),
                                      PRV_SHARED_MAIN_OS_DIR_PP_INIT_SRC_GLOBAL_VAR_GET(pp_mmap_sections_registry[i].length),
                                      PRV_SHARED_MAIN_OS_DIR_PP_INIT_SRC_GLOBAL_VAR_GET(pp_mmap_sections_registry[i].offset),
                                      gtPpFd);
            if (GT_OK != retVal)
                return retVal;

            i++;
        }
#else
        /* Without shared memory approach only SINGLE client is accessible.
         * So it should be the only first one.*/
        return GT_BAD_VALUE;
#endif
    }

    return GT_OK;
}



