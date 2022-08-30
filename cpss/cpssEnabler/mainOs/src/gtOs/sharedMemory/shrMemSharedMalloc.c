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
* @file shrMemSharedMalloc.c
*
* @brief This file contains realization for functions which allocate memory
* for run-time variables in shared memory.
*
* @version   8
********************************************************************************
*/

/************* Includes *******************************************************/
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsSharedMalloc.h>
#include <gtOs/gtOsSharedData.h>
#include <gtOs/gtOsSharedMemoryRemapper.h>
#include <gtOs/gtOsSharedPp.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>


/***** Global Vars access ********************************************/

#define PRV_SHARED_MAIN_OS_DIR_SHMEM_MALLOC_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_HELPER_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.gtOsShmemMallocSrc._var,_value)

#define PRV_SHARED_MAIN_OS_DIR_SHMEM_MALLOC_SRC_GLOBAL_VAR_GET(_var)\
    PRV_HELPER_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.gtOsShmemMallocSrc._var)


#define MALLOC_SIZE_TOKEN "malloc_size"
#define OSMEM_MAX_MEMORY_SIZE_LIMIT_IN_MB 1024


#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

/************* Locals (static - local to c file) *******************************/
/* Declare pointer to shared memory descriptor structure
    inside shared memory. shrMemSharedInit should accept
    this trick! */
static SHARED_MEM_STC * const malloc_mem =
    (SHARED_MEM_STC*)SHARED_MEMORY_MALLOC_VIRT_ADDR_MAC;
/* Descriptors cannot be shared, so place private descriptor
    here (will be build into libhelper.so - private lib) */
static GT_32 shared_malloc_fd;
static char *dyn_name = "DYNAMIC";

 struct MEMBLOCK_LIST_STCT {
    struct MEMBLOCK_LIST_STCT *next;
    unsigned int size;
};

char* shrAddNameSuffix(const char* name, char* buffer, int bufsize);

/* shared memory cleaner */
GT_STATUS shrMemUnlinkShmObj(GT_VOID);
static void prvShrMem_cleaner(void)
{
    (void)shrMemUnlinkShmObj();
#ifndef ASIC_SIMULATION /* Applicable only for BM */
    fprintf(stderr, "*** prvShrMem_cleaner() called\n");
#endif
}


/************* Functions ******************************************************/
/************* Public Functions ***********************************************/

/**
* @internal shrMemSharedMallocInit function
* @endinternal
*
* @brief   Initialize shared memory buffer for shared malloc call.
*
* @param[in] isFirstClient            - specify if client is the first client (CPSS enabler)
*
* @retval GT_OK                    - Operation succeeded
* @retval GT_FAIL                  - Operation failed
* @retval GT_NO_RESOURCE           - Operaton failed. No resource.
*
* @note Inits shared_malloc_fd descriptor (private for each process).
*
*/
GT_STATUS shrMemSharedMallocInit(IN GT_BOOL isFirstClient)
{
    GT_STATUS retVal;
    char buf[256];
    GT_BOOL           aslrSupport = osNonSharedGlobalVars.osNonVolatileDb.aslrSupport;
    GT_U32 size;

    retVal = shrMemConfigGet(&size);
    if (GT_OK != retVal)
            return retVal;

    if(isFirstClient)
    {
        /* register shared memory content cleaner */
        if (atexit(prvShrMem_cleaner))
        {
            fprintf(stderr, "Cannot register /dev/shr cleaner, error=%d\n", errno);
        }


        /* Create shared buffer file for shared malloc */
        shared_malloc_fd = open(shrAddNameSuffix(SHARED_MEMORY_MALLOC_DATA_CNS,buf,sizeof(buf)), O_RDWR|O_CREAT|O_TRUNC, 0777);
        /* (GT_U32) shm_open("/PSSMDYNAMIC", flags, 0777); */
        if(shared_malloc_fd == -1)
            return GT_FAIL;

        /* First client init shared buffer */

       shmPrintf("Malloc buffer initialization: name=%s, addr=0x%X, size=%X\n", dyn_name,
                   SHARED_MEMORY_MALLOC_VIRT_ADDR_MAC, size);

        retVal = shrMemSharedBufferInit(dyn_name, SHARED_MEMORY_MALLOC_VIRT_ADDR_MAC,
            size, shared_malloc_fd, GT_TRUE, malloc_mem);

        if (GT_OK != retVal)
            return GT_NO_RESOURCE;
    }
    else
    {
        if(GT_TRUE == aslrSupport)
        {

            /* register shared memory content cleaner */
           if (atexit(prvShrMem_cleaner))
           {
               fprintf(stderr, "Cannot register /dev/shr cleaner, error=%d\n", errno);
           }
        }
        /* Open shared buffer file for shared malloc */
        shared_malloc_fd = open(shrAddNameSuffix(SHARED_MEMORY_MALLOC_DATA_CNS,buf,sizeof(buf)), O_RDWR, 0777);
        if (-1 == shared_malloc_fd)
        {
            if(errno == ENOENT)
            {
                fprintf(stderr, "open() : file %s not exist, run Enabler first\n",
                    shrAddNameSuffix(SHARED_MEMORY_MALLOC_DATA_CNS,buf,sizeof(buf)));
            }
            return GT_FAIL;
        }

        /* All non-first clients just map shared buffer */
        shmPrintf("Malloc buffer remapping: name=%s, addr=0x%X, size=%X\n", dyn_name,
                   SHARED_MEMORY_MALLOC_VIRT_ADDR_MAC, size);
        retVal = shrMemMmapMemory
        (
            SHARED_MEMORY_MALLOC_VIRT_ADDR_MAC,
            size,
            0,
            shared_malloc_fd
        );

        if (GT_OK != retVal)
            return GT_FAIL;
    }

    return GT_OK;
}

/*******************************************************************************
* shrMemSharedMalloc
*
* DESCRIPTION:
*       Allocates memory into shared buffer for shared malloc.
*       Size of required block is specified as parameter.
*
* INPUTS:
*       size    - size of memory which needs to be allocated
*
* OUTPUTS:
*       None
*
* RETURNS:
*       void*   - Operation succeeded. Adress of allocated memory
*       NULL    - Operaton failed
*
* COMMENTS:
*       Uses static pointer malloc_mem to access shared structure.
*       Uses static variable shared_malloc_fd to extend shared file.
*
*******************************************************************************/
void *shrMemSharedMalloc(IN size_t size)
{
    struct MEMBLOCK_LIST_STCT **prev, *blk;
    pthread_mutex_lock(&malloc_mem->mutex);
    prev =(struct MEMBLOCK_LIST_STCT **) &((PRV_SHARED_MAIN_OS_DIR_SHMEM_MALLOC_SRC_GLOBAL_VAR_GET(shmFreeBufferList)));
    blk = ((struct MEMBLOCK_LIST_STCT *)(PRV_SHARED_MAIN_OS_DIR_SHMEM_MALLOC_SRC_GLOBAL_VAR_GET(shmFreeBufferList)));
    while (blk)
    {
        if (blk->size == size)
        {
            *prev = blk->next;
            pthread_mutex_unlock(&malloc_mem->mutex);
            return (GT_VOID*)blk;
        }
        prev = &(blk->next);
        blk = blk->next;
    }
    pthread_mutex_unlock(&malloc_mem->mutex);
    return shrMemSharedMallocIntoBlock(malloc_mem, shared_malloc_fd, size);
}

/**
* @internal shrMemSharedFree function
* @endinternal
*
* @brief   Free memory allocated by shrMemSharedMalloc()
*
* @param[in] ptr                      - memory which needs to be freed
*                                       None
*
* @note Currently does nothing
*
*/
GT_VOID shrMemSharedFree
(
    IN GT_VOID *ptr
)
{
    GT_UNUSED_PARAM(ptr);
}

/**
* @internal shrMemSharedFreeSize function
* @endinternal
*
* @brief   Free memory allocated by shrMemSharedMalloc()
*
* @param[in] ptr                      - memory which needs to be freed
*                                      size - memory block size
*                                       None
*/
GT_VOID shrMemSharedFreeSize
(
    IN GT_VOID *ptr,
    IN unsigned int size
)
{
    struct MEMBLOCK_LIST_STCT *head = (struct MEMBLOCK_LIST_STCT *)ptr;
    head->size = size;
    pthread_mutex_lock(&malloc_mem->mutex);
    head->next = ((struct MEMBLOCK_LIST_STCT *)(PRV_SHARED_MAIN_OS_DIR_SHMEM_MALLOC_SRC_GLOBAL_VAR_GET(shmFreeBufferList)));
    PRV_SHARED_MAIN_OS_DIR_SHMEM_MALLOC_SRC_GLOBAL_VAR_SET(shmFreeBufferList,(GT_VOID *)head);


    pthread_mutex_unlock(&malloc_mem->mutex);
}



/************* Typedefs ********************************************************/
/*
* Typedef: struct PRV_SHR_STATIC_BLOCK_STC
*
* Description:
*       The linked list of statically allocated data
*
* Fields:
*       next            - next item ptr or NULL
*       memBlockName    - name of shared memory
*       size            - size of allocated memory
*       ptr             - allocated memory
*
*******************************************************************************/
typedef struct PRV_SHR_STATIC_BLOCK_STCT {
    struct PRV_SHR_STATIC_BLOCK_STCT *next;
    char        memBlockName[32];
    unsigned    int size;
    GT_VOID*    ptr;
} PRV_SHR_STATIC_BLOCK_STC;


/*******************************************************************************
* shrMemAllocStatic
*
* DESCRIPTION:
*       Allocates memory into shared buffer with a given name
*       If memory with such name already allocated then return
*       pointer to such object
*
* INPUTS:
*       memBlockName - the name for memory block
*       size         - size of memory which needs to be allocated
*
* OUTPUTS:
*       None
*
* RETURNS:
*       void*   - Operation succeeded. Adress of allocated memory
*       NULL    - Operaton failed
*
* COMMENTS:
*       Uses shrMemSharedMalloc()
*
*******************************************************************************/
GT_VOID *shrMemAllocStatic(
  IN const char*    memBlockName,
  IN unsigned int   size
)
{
    PRV_SHR_STATIC_BLOCK_STC *blk;
    /* search if block was previosly allocated */
    for (blk = (PRV_SHR_STATIC_BLOCK_STC *)PRV_SHARED_MAIN_OS_DIR_SHMEM_MALLOC_SRC_GLOBAL_VAR_GET(prvShrMemBlockList); blk != NULL; blk = blk->next)
    {
        if (strcmp(blk->memBlockName, memBlockName) == 0)
        {
            /* memblock name match */
            if (blk->size == size)
                return blk->ptr;
            return NULL; /* size doesn't match */
        }
    }
    /* not found, create new */
    /*TODO: assume no race condition here */
    blk = (PRV_SHR_STATIC_BLOCK_STC*)shrMemSharedMalloc(sizeof(*blk));
    if (blk == NULL)
        return NULL;
    strncpy(blk->memBlockName, memBlockName, sizeof(blk->memBlockName));
    blk->memBlockName[sizeof(blk->memBlockName)-1] = 0;
    blk->size = size;
    blk->ptr = shrMemSharedMalloc(size);
    if (blk->ptr != NULL)
        memset(blk->ptr, 0, size);
    /* insert into list */
    blk->next = ((PRV_SHR_STATIC_BLOCK_STC *)PRV_SHARED_MAIN_OS_DIR_SHMEM_MALLOC_SRC_GLOBAL_VAR_GET(prvShrMemBlockList));
    PRV_SHARED_MAIN_OS_DIR_SHMEM_MALLOC_SRC_GLOBAL_VAR_SET(prvShrMemBlockList,((GT_VOID *)blk));

    return blk->ptr;
}

GT_STATUS  shrMemConfigGet
(
  OUT GT_U32          *sizePtr
)
{
    FILE * config;
    GT_CHAR buffer[256];
    GT_CHAR * tmp;
    GT_U32 size;

    if(NULL == sizePtr)
    {
        return GT_BAD_PTR;
    }

    *sizePtr = OSMEM_MAX_MEMORY_SIZE_USE;

    if((config = fopen("/dev/shm/config", "r")) != NULL)
    {
       while(fscanf(config,"%[^\n]",buffer)!=EOF)
       {
          fgetc (config);
          tmp = strstr(buffer,MALLOC_SIZE_TOKEN);

          if(NULL!=tmp)
          {
            sscanf(tmp+strlen(MALLOC_SIZE_TOKEN),"%d",&size);

            if(size<=OSMEM_MAX_MEMORY_SIZE_LIMIT_IN_MB)
            {
                *sizePtr = size*1024*1024;
            }
          }
       }
       fclose(config);
    }

    return GT_OK;
}

