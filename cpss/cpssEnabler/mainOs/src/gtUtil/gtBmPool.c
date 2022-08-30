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
* @file gtBmPool.c
*
* @brief Buffer management module.
*
* @version   17
********************************************************************************
*/


/********* include ************************************************************/

#include <gtOs/gtGenTypes.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsStr.h>
#include <gtUtil/gtBmPool.h>
#include <gtOs/gtOsSharedData.h>
/*#include <prestera/tapi/shutDown/shutDown.h>*/

#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>


/***** Global Vars access ********************************************/


#define PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_HELPER_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.gtBmPoolSrc._var,_value)

#define PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(_var)\
    PRV_HELPER_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.gtBmPoolSrc._var)



/************ Internal Typedefs ***********************************************/


/*
 * Typedef: struct GT_POOL_BUF_HEAD
 *
 * Description: Defines the header of a buffer in a pool.
 *
 * Fields:
 *      pNext - Pointer to the next free buffer in the pool.
 *
 */
typedef struct BufHead
{
    struct BufHead  *pNext;

}GT_POOL_FREE_BUF_HEAD;


/*
 * Typedef: struct GT_POOL_BLOCK_NODE
 *
 * Description: Defines the pool block node ptrs in the block's LL.
 *
 * Fields:
 *      pPoolBlock - Pointer to the allocated memory block
 *      pAlignedBlockStart - Pointer to the aligned start address
 *                           of the pool's memory block
 *      pAlignedBlockEnd   - Pointer to the aligned end address
 *                           of the pool's memory block
 *      pNext - Pointer to the next BLOCK node.
 *
 */
typedef struct _buf_block_node
{
    GT_VOID *pBlockPtr;
    GT_VOID *pPoolBlock;

    GT_VOID *pAlignedBlockStart;
    GT_VOID *pAlignedBlockEnd;
    struct _buf_block_node  *pNext;

}GT_POOL_BLOCK_NODE;

/*
 * Typedef: struct gt_pool_id
 *
 * Description: Defines the pool ID
 *
 * Fields:
 *      pListHead - Head of the buffers' list
 *      pPoolBlocksHead - head of the block's list
 *
 *      reqBufferSize - requested Buffer's size
 *      actBufferSize - Actual Buffer's size
 *      numOfBuffers - number of buffers requested to be in that pool
 *      numOfFreeBuffers - number of FREE buffers in that pool
 */
typedef struct gt_pool_head
{

    GT_POOL_BLOCK_NODE      *pPoolBlocksHead;

    GT_U32                  poolAlignment;


    GT_POOL_FREE_BUF_HEAD  *pListHead;

    /* Statistics */
    /**************/
    GT_U32                  memBlocSize;    /* Memory block size */
    GT_U32                  reqBufferSize;  /* requested buffer's size */
    GT_U32                  actBufferSize;  /* Actual buffer's size (after alignment) */

    GT_U32                  numOfBuffers;       /* Number of buffers in the pool */
    GT_U32                  numOfFreeBuffers;   /* number of free buffers in the pool */

    /* Total number of buffers requested from the pool */
    GT_U32                  numOfRequest;

    /* Total number of buffers returned to the pool */
    GT_U32                  numOfReturn;

    /* Is this  semaphore protected pool */
    GT_BOOL                 semProtected;

    /* Protection */
    GT_U32                  magic;
    GT_BOOL                 forDma;  /* is this pool is used for dma (physically contiguous) */
    GT_U32                  dmaWindow;/*select one of the available DMA windows*/

}GT_POOL_HEAD;

#define  GT_POOL_HEAD_MAGIC  0x5A5A5A00
/* indication that the pool is free !!! */
#define  GT_POOL_HEAD_MAGIC_FREE  0


/************ Private Functions ************************************************/

void poolPrintfInfo
(
    IN  GT_POOL_ID poolId
);


/* static GT_STATUS  gtPoolShutDown
 * (
 *   void
 * );
 */

/************ Public Functions ************************************************/


/**
* @internal gtPoolInit function
* @endinternal
*
* @brief   Used to initialize the pool module
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_VALUE             - In case of bad alignment value
* @retval GT_INIT_ERROR            - In case re-initialization.
* @retval GT_NO_RESOURCE           - Not enough memory or semaphores.
*
* @note Usage of this function is only during FIRST initialization.
*
*/
GT_STATUS  gtPoolInit
(
   IN   GT_U32             maxPools
)
{
    static GT_BOOL    firstPoolInit = GT_TRUE;
    GT_STATUS  status = GT_OK;

    /* Test that the module is initialized only once */
/*    if ( GT_TRUE == isInitialized )
        return(GT_INIT_ERROR);
 */
    if(firstPoolInit)
    {
       /* Allocated pool array and clear it */
       PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_SET(poolArray,osStaticMalloc(maxPools * sizeof(GT_POOL_HEAD)));
       if (NULL == (GT_POOL_HEAD *)PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolArray))
           return(GT_NO_RESOURCE);

       /* No more use of maxPool*/
       PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_SET(poolMaxPools,maxPools);

       status = osMutexCreate("PoolMtx", &(PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolMtx)));
       if(status != GT_OK)
           return(GT_NO_RESOURCE);

       PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_SET(freePoolHeaderIdx,0);

       firstPoolInit = GT_FALSE;
    }
/*
    gtShutdownRegister(gtPoolShutDown, SHUTDOWN_PRIORITY_MEDIUM);
 */
    PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_SET(isInitialized,GT_TRUE);

    return(status);
}

/**
* @internal internalPoolCreatePool function
* @endinternal
*
* @brief   Create pool of buffers. All buffers have the same size.
*
* @param[in] forDma                   - is physically contigious.
*                                      bufferSize   - Buffer's size
* @param[in] alignment                - Buffers' alignment
* @param[in] numOfBuffers             - number of buffer at the pool
* @param[in] useSem                   - GT_TRUE to use a semaphore for mutual exclusion on
*                                      access to the pool.
*                                      Note: Semaphore protection is one for all pools.
*
* @param[out] pPoolId                  - Return the pool ID
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_VALUE             - Request for zero number of buffers or
*                                       buffer's size is smaller than the size of
*                                       pointer (usually 4 bytes)
* @retval GT_BAD_PTR               - Bad pointer for the pPoolId
* @retval GT_NO_RESOURCE           - No memory is available to create the pool.
* @retval GT_NOT_INITIALIZED       - In case of un-initialized package.
*
*
*/
static GT_STATUS internalPoolCreatePool
(
    IN   GT_U32            reqBufferSize,
    IN   GT_POOL_ALIGNMENT alignment,
    IN   GT_U32            numOfBuffers,
    IN   GT_BOOL           useSem,
    IN   GT_BOOL           forDma,
    IN   GT_U32            dmaWindow,
    OUT  GT_POOL_ID       *pPoolId
)
{
    GT_U32                 actBufferSize, poolSize;
    GT_U32                 indx;
    GT_POOL_FREE_BUF_HEAD *pBuf = NULL;
    GT_U8                 *pBytePtr;
    GT_POOL_HEAD          *pHead;
    GT_U32                ii;/*iterator*/
    GT_BOOL               newIndexUsed = GT_FALSE;/* indication that freePoolHeaderIdx incremented */

    if ( GT_FALSE == PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(isInitialized ))
        return(GT_NOT_INITIALIZED);

    /* Zero buffer in the pool is not allowed.
     * Size smaller than the size of next pointer is not allowed.
     */
    if ( (numOfBuffers == 0) ||
         (reqBufferSize < sizeof(struct BufHead*)) )
        return(GT_BAD_VALUE);

    if (pPoolId == NULL)
        return(GT_BAD_PTR);

    /* Buffer alignment in the Pool. */
    if( alignment != GT_1_BYTE_ALIGNMENT &&
        alignment != GT_4_BYTE_ALIGNMENT &&
        alignment != GT_8_BYTE_ALIGNMENT &&
        alignment != GT_16_BYTE_ALIGNMENT
        )
    {
            return(GT_BAD_VALUE);
    }

    if(PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(freePoolHeaderIdx))
    {
        /* look for empty places to support 'Deleted pools'*/
        for(ii = 0 ; ii < PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(freePoolHeaderIdx); ii++)
        {
            if(((GT_POOL_HEAD *)PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolArray))[ii].magic == GT_POOL_HEAD_MAGIC_FREE)
            {
                /*found empty index*/
                break;
            }
        }
    }
    else /*freePoolHeaderIdx==0*/
    {
        ii = PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(freePoolHeaderIdx);
    }

    if(ii == PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(freePoolHeaderIdx))
    {
        /* not found existing place , so allocate new place */
        if (PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(freePoolHeaderIdx) == PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolMaxPools))
        {
            return(GT_NO_RESOURCE);
        }

        /* Catch pool header */
        pHead = &(((GT_POOL_HEAD *)PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolArray))[PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(freePoolHeaderIdx)]);
        (PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(freePoolHeaderIdx))++;
        newIndexUsed = GT_TRUE;
    }
    else
    {
        pHead = &(((GT_POOL_HEAD *)PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolArray))[ii]);
    }

    /* Calculate the real size of the buffer, depend on the user request,
     * alignment, and buffer header.
     */
    pHead->poolAlignment    = alignment;
    actBufferSize = (reqBufferSize + alignment) & (~alignment);

    /* calculate the memory size needed for the pool */
    poolSize = numOfBuffers * actBufferSize;

    pHead->reqBufferSize    = reqBufferSize;
    pHead->actBufferSize    = actBufferSize;
    pHead->numOfBuffers     = numOfBuffers;
    pHead->numOfFreeBuffers = numOfBuffers;
    pHead->numOfRequest     = 0;
    pHead->numOfReturn      = 0;
    pHead->semProtected     = useSem;
    pHead->magic            = GT_POOL_HEAD_MAGIC;
    pHead->forDma           = forDma;
    pHead->dmaWindow        = dmaWindow;
    /* Allocate memory for the pool.
     * Allocated size is pool's size plus extra
     * bytes that might be needed for alignment.
     */
    if (forDma)
    {
        if(dmaWindow == 0)
        {
            pBytePtr = osCacheDmaMalloc(poolSize + alignment);
        }
        else
        {
            pBytePtr = osCacheDmaMallocByWindow(dmaWindow , poolSize + alignment);
        }
    }
    else
    {
        pBytePtr = osMalloc(poolSize + alignment);
    }
    if (pBytePtr == NULL)
    {
        if(newIndexUsed == GT_TRUE)
        {
            (PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(freePoolHeaderIdx))--;
        }
        return(GT_NO_RESOURCE);
    }

    /* initialize the blocks link list */
    pHead->pPoolBlocksHead = osMalloc(sizeof(GT_POOL_BLOCK_NODE));
    if (pHead->pPoolBlocksHead == NULL)
    {
        if (forDma)
        {
            osCacheDmaFree(pBytePtr);
        }
        else
        {
            osFree(pBytePtr);
        }
        if(newIndexUsed == GT_TRUE)
        {
           (PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(freePoolHeaderIdx))--;
        }
        return(GT_NO_RESOURCE);
    }

    pHead->pPoolBlocksHead->pNext = NULL;

    /* Save the block memory address and its size */
    pHead->pPoolBlocksHead->pPoolBlock = pBytePtr;

    pHead->memBlocSize = poolSize + alignment;

    /* Align the memory block */
    pBytePtr = (GT_U8*)((GT_UINTPTR)(pBytePtr + alignment) & (~((GT_UINTPTR)alignment)));

    /* Store the memory block boundaries*/
    pHead->pPoolBlocksHead->pAlignedBlockStart = pBytePtr;
    pHead->pPoolBlocksHead->pAlignedBlockEnd = pBytePtr + poolSize - 1;

    /* The first buffer is located in the beginning of the memory block */
    pHead->pListHead = (GT_POOL_FREE_BUF_HEAD*)pBytePtr;

     /* Divide the memory block into buffers and make it as a link list */
    for (indx=0; indx < numOfBuffers; indx++)
    {
        pBuf = (GT_POOL_FREE_BUF_HEAD*)pBytePtr;
        pBytePtr = pBytePtr + actBufferSize;
        pBuf->pNext = (GT_POOL_FREE_BUF_HEAD*)pBytePtr;
    }
    /* Last buffer is pointing to NULL */
    pBuf->pNext = NULL;

    /* Set the return pool id. */
    *(GT_POOL_HEAD **)pPoolId = pHead;

    return(GT_OK);
}

/**
* @internal gtPoolCreatePool function
* @endinternal
*
* @brief   Create pool of buffers. All buffers have the same size.
*
* @param[out] pPoolId                  - Return the pool ID
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_VALUE             - Request for zero number of buffers or
*                                       buffer's size is smaller than the size of
*                                       pointer (usually 4 bytes)
* @retval GT_BAD_PTR               - Bad pointer for the pPoolId
* @retval GT_NO_RESOURCE           - No memory is available to create the pool.
* @retval GT_NOT_INITIALIZED       - In case of un-initialized package.
*
*
*/
GT_STATUS gtPoolCreatePool
(
    IN   GT_U32            reqBufferSize,
    IN   GT_POOL_ALIGNMENT alignment,
    IN   GT_U32            numOfBuffers,
    IN   GT_BOOL           useSem,
    OUT  GT_POOL_ID       *pPoolId
)
{
    return internalPoolCreatePool(reqBufferSize,alignment,numOfBuffers,useSem,GT_FALSE,0/*dmaWindow*/,pPoolId);
}
/**
* @internal gtPoolCreateDmaPool function
* @endinternal
*
* @brief   Create pool of buffers. All buffers have the same size.
*         The buffers are created in a phsycally contigious area.
*
* @param[out] pPoolId                  - Return the pool ID
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_VALUE             - Request for zero number of buffers or
*                                       buffer's size is smaller than the size of
*                                       pointer (usually 4 bytes)
* @retval GT_BAD_PTR               - Bad pointer for the pPoolId
* @retval GT_NO_RESOURCE           - No memory is available to create the pool.
* @retval GT_NOT_INITIALIZED       - In case of un-initialized package.
*
*
*/
GT_STATUS gtPoolCreateDmaPool
(
    IN   GT_U32            reqBufferSize,
    IN   GT_POOL_ALIGNMENT alignment,
    IN   GT_U32            numOfBuffers,
    IN   GT_BOOL           useSem,
    OUT  GT_POOL_ID       *pPoolId
)
{
    return internalPoolCreatePool(reqBufferSize,alignment,numOfBuffers,useSem,GT_TRUE,0/*dmaWindow*/,pPoolId);
}

/**
* @internal gtPoolCreateDmaPoolByWindow function
* @endinternal
*
* @brief   Create pool of DMA buffers from specific DMA window.
*          All buffers have the same size. The buffers are created in a physically
*          contiguous area.
*          the pool is associated with one of the already existing DMA Windows.
*
* @param[in] windowId              - The DMA window to use
* @param[in] reqBufferSize         - The size of each buffer in the pool
* @param[in] alignment             - The needed buffer's alignment
* @param[in] numOfBuffers          - number of buffers in the pool
* @param[in] useSem                - GT_TRUE to use a semaphore for mutual exclusion on
*                                   access the pool.
*                                   Note: Semaphore protection is one for all pools.
*
* @param[out] pPoolId              - (pointer to) the pool ID
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_VALUE             - Request for zero number of buffers or
*                                       buffer's size is smaller than the size of
*                                       pointer (usually 4 bytes)
* @retval GT_BAD_PTR               - Bad pointer for the pPoolId
* @retval GT_NO_RESOURCE           - No memory is available to create the pool.
* @retval GT_NOT_INITIALIZED       - In case of un-initialized package.
*
* @note Usage of this function is only during FIRST initialization.
*
*/
GT_STATUS gtPoolCreateDmaPoolByWindow
(
    IN   GT_U32            windowId,
    IN   GT_U32            reqBufferSize,
    IN   GT_POOL_ALIGNMENT alignment,
    IN   GT_U32            numOfBuffers,
    IN   GT_BOOL           useSem,
    OUT  GT_POOL_ID       *pPoolId
)
{
    return internalPoolCreatePool(reqBufferSize,alignment,numOfBuffers,useSem,GT_TRUE,windowId/*dmaWindow*/,pPoolId);
}

/**
* @internal gtPoolReCreatePool function
* @endinternal
*
* @brief   Create pool of buffers. All buffers have the same size.
*
* @param[in] poolId                   - Id of the pool to be re-created.
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_VALUE             - Bad poolId
* @retval GT_NOT_INITIALIZED       - In case of un-initialized package.
*
* @note Usage of this function is only during FIRST initialization.
*
*/
GT_STATUS gtPoolReCreatePool
(
    IN  GT_POOL_ID  poolId
)
{
    GT_U32                 numOfBuffers;
    GT_U32                 indx;
    GT_POOL_FREE_BUF_HEAD *pBuf = NULL;
    GT_U8                 *pBytePtr;
    GT_POOL_HEAD          *pHead;
    GT_POOL_BLOCK_NODE    *pBlockNode;

    if ( GT_FALSE == PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(isInitialized ) )
        return(GT_NOT_INITIALIZED);

    pHead = (GT_POOL_HEAD *)poolId;
    if(pHead == NULL)
      return GT_BAD_VALUE;

    if(pHead->magic != GT_POOL_HEAD_MAGIC)
      return GT_BAD_VALUE;


    numOfBuffers = pHead->numOfBuffers ;
    if (numOfBuffers == 0)
    {
        return GT_BAD_VALUE;
    }

    pHead->numOfFreeBuffers = numOfBuffers;
    pHead->numOfRequest     = 0;
    pHead->numOfReturn      = 0;

    pBlockNode = pHead->pPoolBlocksHead;
    /* Store the memory block boundaries*/
    pBytePtr = pBlockNode->pAlignedBlockStart;

    /* The first buffer is located in the beginning of the memory block */
    pHead->pListHead = (GT_POOL_FREE_BUF_HEAD*)pBytePtr;

     /* Divide the memory block into buffers and make it as a link list */
    for (indx=0; indx < numOfBuffers; indx++)
    {
        pBuf = (GT_POOL_FREE_BUF_HEAD*)pBytePtr;
        pBytePtr = pBytePtr + pHead->actBufferSize;
        if ((pBytePtr > (GT_U8*)pBlockNode->pAlignedBlockEnd)
            && (indx < (numOfBuffers-1)))
        {
            /* boundry check*/
            if (pBlockNode->pNext == NULL)
            {
                /*shouldn't happen*/
                return GT_FAIL;
            }
            pBlockNode = pBlockNode->pNext;
            pBytePtr = pBlockNode->pAlignedBlockStart;
        }
        pBuf->pNext = (GT_POOL_FREE_BUF_HEAD*)pBytePtr;
    }
    /* Last buffer is pointing to NULL */
    pBuf->pNext = NULL;

    return(GT_OK);
}

/**
* @internal gtPoolDeletePool function
* @endinternal
*
* @brief   Delete a pool and free all the memory occupied by that pool.
*         The operation is failed if not all the buffers belong to that pool
*         have been freed.
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_STATE             - Not all buffers belong to that pool have
*                                       been freed.
*/
GT_STATUS gtPoolDeletePool
(
    IN  GT_POOL_ID  poolId
)
{
    GT_POOL_HEAD  *pHead = (GT_POOL_HEAD *)poolId;
    GT_POOL_BLOCK_NODE       *ptr;



    if ( GT_FALSE == PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(isInitialized ) )
        return(GT_NOT_INITIALIZED);


    if ( (pHead <  &(((GT_POOL_HEAD *)PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolArray))[0])) ||
         (pHead >= &(((GT_POOL_HEAD *)PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolArray))[PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolMaxPools)]) ))
        return(GT_BAD_PTR);


    /* If not all the buffers have been freed, return with error */
    if (pHead->numOfFreeBuffers != pHead->numOfBuffers)
        return(GT_BAD_STATE);

    /* free pool's memory block */
    ptr = pHead->pPoolBlocksHead;
    while (ptr != NULL)
    {
        if (pHead->forDma)
        {

            /* was allocated by osCacheDmaMalloc */
            osCacheDmaFree(ptr->pPoolBlock);
        }
        else
        {
            /* was allocated by osMalloc */
            osFree(ptr->pPoolBlock);
        }
        ptr = ptr->pNext;
    }

    /* free the blocks header .. was allocated by osMalloc */
    osFree(pHead->pPoolBlocksHead);

    /* free pool ID */
    /* indication that the pool is free !!! */
    pHead->magic = GT_POOL_HEAD_MAGIC_FREE;
    /*  do not call osFree(...) because the header is not allocated by it !!!
        osFree(pHead);*/

    return(GT_OK);
}

/*******************************************************************************
* gtPoolGetBuf
*
* DESCRIPTION:
*           Return pointer to a buffer from the requested pool
*
* INPUTS:
*           PoolId - The pool ID as returned by the create function.
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_VOID* - Pointer to the new allocated buffer. NULL is returned in case
*               no buffer is not available.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID *gtPoolGetBuf
(
    IN  GT_POOL_ID poolId
)
{

    GT_POOL_FREE_BUF_HEAD *pBuf;
    GT_POOL_HEAD          *pHead = (GT_POOL_HEAD *)poolId;


    if ( GT_FALSE == PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(isInitialized ) )
        return(NULL);

    if ( (pHead <  &(((GT_POOL_HEAD *)PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolArray))[0])) ||
         (pHead >= &(((GT_POOL_HEAD *)PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolArray))[PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolMaxPools)])))
        return(NULL);


    if ( pHead->semProtected )
        osMutexLock(PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolMtx));

    /* Take the buffer from the head of the list */
    pBuf = pHead->pListHead;
    if (pBuf != NULL) /* if the list is not empty, update the header */
    {
        pHead->pListHead = pBuf->pNext;

        /* Update number of free buffers */
        pHead->numOfFreeBuffers--;
    }

    if ( pHead->semProtected )
        osMutexUnlock(PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolMtx));

    return(pBuf);
}

/**
* @internal gtPoolFreeBuf function
* @endinternal
*
* @brief   Free a buffer back to its pool.
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_PTR               - The returned buffer is not belongs to that pool
* @retval GT_NOT_INITIALIZED       - In case of un-initialized package.
*/
GT_STATUS gtPoolFreeBuf
(
    IN  GT_POOL_ID poolId,
    IN  GT_VOID   *pBuf
)
{
    GT_POOL_FREE_BUF_HEAD   *pRetbuf;
    GT_U8                   *pBlocStart, *pBlockEnd;
    GT_POOL_HEAD            *pHead = (GT_POOL_HEAD *)poolId;
    GT_POOL_BLOCK_NODE      *pBlockNode;


    if ( GT_FALSE == PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(isInitialized ) )
        return(GT_NOT_INITIALIZED);


    if ( (pHead <  &(((GT_POOL_HEAD *)PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolArray))[0])) ||
         (pHead >= &(((GT_POOL_HEAD *)PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolArray))[PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolMaxPools)])))
        return(GT_BAD_PTR);

    if ( pHead->semProtected )
        osMutexLock(PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolMtx));

    pBlockNode = pHead->pPoolBlocksHead;

    while (pBlockNode != NULL)
    {
        pBlocStart = pBlockNode->pAlignedBlockStart;
        pBlockEnd = pBlockNode->pAlignedBlockEnd;


        /* Test if the returned buffer belongs to that pool */
        if ( ( (GT_U8*)pBuf >= pBlocStart ) &&           /* in the pool boundaries */
             ( (GT_U8*)pBuf <= pBlockEnd )  &&
             ( ((GT_UINTPTR)pBuf & pHead->poolAlignment) == 0 )/* ...and aligned  */
           )
            break;

        pBlockNode = pBlockNode->pNext;
    }

    if (pBlockNode == NULL)
    {
        if ( pHead->semProtected )
            osMutexUnlock(PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolMtx));

        return GT_BAD_PTR;
    }

    /* Insert the returned buffer to the head of the list */
    pRetbuf = pBuf;
    pRetbuf->pNext = pHead->pListHead;
    pHead->pListHead = pRetbuf;

    /* Update number of free buffers */
    pHead->numOfFreeBuffers++;

    if ( pHead->semProtected )
        osMutexUnlock(PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolMtx));

    return(GT_OK);
}


/**
* @internal gtPoolGetBufSize function
* @endinternal
*
* @brief   Get the buffer size.
*
* @retval GT_U32                   - the buffer size.
*/
GT_U32 gtPoolGetBufSize
(
    IN        GT_POOL_ID  poolId
)
{
    GT_POOL_HEAD    *pHead = (GT_POOL_HEAD *)poolId;
    return pHead->reqBufferSize;
}

/**
* @internal gtPoolExpandPool function
* @endinternal
*
* @brief   Expand a pool of buffers. All buffers have the same size.
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_VALUE             - Request for zero number of buffers or
*                                       buffer's size is smaller than the size of
*                                       pointer (usually 4 bytes)
* @retval GT_BAD_PTR               - Bad pointer for the pPoolId
* @retval GT_NO_RESOURCE           - No memory is available to create the pool.
* @retval GT_NOT_INITIALIZED       - In case of un-initialized package.
*
* @note not used for Dma buffers!
*
*/
GT_STATUS gtPoolExpandPool
(
    IN   GT_POOL_ID        poolId,
    IN   GT_U32            numOfBuffers
)
{
    GT_U32                 actBufferSize, addedPoolSize;
    GT_U32                 indx;
    GT_POOL_FREE_BUF_HEAD *pBuf = NULL;
    GT_U8                 *pBytePtr;
    GT_POOL_HEAD          *pHead = (GT_POOL_HEAD *)poolId;
    GT_POOL_ALIGNMENT     alignment;
    GT_POOL_BLOCK_NODE    *pBlockNode;
    GT_POOL_FREE_BUF_HEAD *lastFirstBuf;

    if ( GT_FALSE == PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(isInitialized ) )
        return(GT_NOT_INITIALIZED);

    if ( (pHead <  &(((GT_POOL_HEAD *)PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolArray))[0])) ||
         (pHead >= &(((GT_POOL_HEAD *)PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolArray))[PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolMaxPools)])))
        return(GT_BAD_PTR);

    if (pHead->forDma == GT_TRUE)
        return (GT_BAD_PARAM);


    /* Zero buffer in the pool is not allowed.
     * Size smaller than the size of next pointer is not allowed.
     */
    if (numOfBuffers == 0)
        return(GT_BAD_VALUE);

    if ( pHead->semProtected )
        osMutexLock(PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolMtx));

    /* Calculate the real size of the buffer, depend on the user request,
     * alignment, and buffer header.
     */
    alignment = pHead->poolAlignment;
    actBufferSize = pHead->actBufferSize;

    /* calculate the memory size needed for the pool */
    addedPoolSize = numOfBuffers * pHead->actBufferSize;


    /* Allocate memory for the pool.
     * Allocated size is pool's size plus extra
     * bytes that might be needed for alignment.
     */
    pBytePtr = osMalloc(addedPoolSize + alignment);
    if (pBytePtr == NULL)
    {
        if ( pHead->semProtected )
            osMutexUnlock(PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolMtx));
        return(GT_NO_RESOURCE);
    }

    pHead->numOfBuffers     = numOfBuffers + pHead->numOfBuffers;
    pHead->numOfFreeBuffers = numOfBuffers + pHead->numOfFreeBuffers;

    /* add to the blocks link list */
    pBlockNode = pHead->pPoolBlocksHead;
    pHead->pPoolBlocksHead = osMalloc(sizeof(GT_POOL_BLOCK_NODE));
    if (pHead->pPoolBlocksHead == NULL)
    {
        osFree(pBytePtr);
        if ( pHead->semProtected )
            osMutexUnlock(PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolMtx));
        return(GT_NO_RESOURCE);
    }

    pHead->pPoolBlocksHead->pNext = pBlockNode;

    /* Save the block memory address and its size */
    pHead->pPoolBlocksHead->pPoolBlock = pBytePtr;

    /* Align the memory block */
    pBytePtr = (GT_U8*)((GT_UINTPTR)(pBytePtr + alignment) & (~((GT_UINTPTR)alignment)));

    /* Store the memory block boundaries*/
    pHead->pPoolBlocksHead->pAlignedBlockStart = pBytePtr;
    pHead->pPoolBlocksHead->pAlignedBlockEnd = pBytePtr + addedPoolSize - 1;

    /* The first buffer is located in the beginning of the memory block */
    lastFirstBuf = pHead->pListHead;
    pHead->pListHead = (GT_POOL_FREE_BUF_HEAD*)pBytePtr;

     /* Divide the memory block into buffers and make it as a link list */
    for (indx=0; indx < numOfBuffers; indx++)
    {
        pBuf = (GT_POOL_FREE_BUF_HEAD*)pBytePtr;
        pBytePtr = pBytePtr + actBufferSize;
        pBuf->pNext = (GT_POOL_FREE_BUF_HEAD*)pBytePtr;
    }
    /* Last buffer is pointing to last first buffer */
    pBuf->pNext = lastFirstBuf;

    if ( pHead->semProtected )
        osMutexUnlock(PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolMtx));

    return(GT_OK);
}


/**
* @internal gtPoolGetBufFreeCnt function
* @endinternal
*
* @brief   Get the free buffer count.
*
* @retval GT_U32                   - the free buffer count.
*/
GT_U32 gtPoolGetBufFreeCnt
(
    IN        GT_POOL_ID  poolId
)
{
    GT_POOL_HEAD    *pHead = (GT_POOL_HEAD *)poolId;
    return pHead->numOfFreeBuffers;
}

/************ Private Functions ************************************************/


/*******************************************************************************
* gtPoolShutDown
*
* DESCRIPTION:
*           Used to initialize the pool module
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK          - For successful operation.
*       GT_BAD_VALUE   - In case of bad alignment value
*       GT_INIT_ERROR  - In case re-initialization
*       GT_NO_RESOURCE - Not enough memory.
*       GT_NOT_INITIALIZED - In case of un-initialized package.
*
* COMMENTS:
*       None
*
*******************************************************************************/
/*static GT_STATUS  gtPoolShutDown
(
  void
)
{

    if ( GT_FALSE == isInitialized )
        return(GT_NOT_INITIALIZED);

    isInitialized = GT_FALSE;

    return(GT_OK);

}
*/

/**
* @internal gtPoolPrintStats function
* @endinternal
*
* @brief   Print pool's statistics for the requested pool, or for all
*         the pools in case poolID is GT_POOL_NULL_ID.
*/
void gtPoolPrintStats
(
    IN  GT_POOL_ID poolId
)
{
    GT_U32  indx;

    if (poolId == GT_POOL_NULL_ID)
    {
       for (indx = 0 ; indx < PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolMaxPools); indx++)
       {
          poolPrintfInfo(&(((GT_POOL_HEAD *)PRV_SHARED_MAIN_OS_DIR_GT_BM_POOL_SRC_GLOBAL_VAR_GET(poolArray))[indx]));
       }
    }
    else
    {
        poolPrintfInfo(poolId);
    }
}


/**
* @internal poolPrintfInfo function
* @endinternal
*
* @brief   Helper function for gtPoolPrintStats
*/
void poolPrintfInfo
(
    IN  GT_POOL_ID poolId
)
{
    GT_POOL_HEAD*    pHead = (GT_POOL_HEAD*)poolId;
    GT_POOL_BLOCK_NODE* pBlockNode;


    pBlockNode = pHead->pPoolBlocksHead;
    while (pBlockNode != NULL)
    {

        osPrintf("\nPool's ID          : 0x%08X\n", poolId);
        osPrintf("Pool's Boundaries  : %p - %p (%d Bytes)\n",
                 pBlockNode->pPoolBlock,
                 (void*)((GT_UINTPTR)(pBlockNode->pPoolBlock) + pHead->memBlocSize - 1),
                 pHead->memBlocSize);

        osPrintf("Aligned Boundaries : %p - %p (%d Bytes)\n",
                 pBlockNode->pAlignedBlockStart,
                 pBlockNode->pAlignedBlockEnd,
                 (pHead->numOfBuffers * pHead->actBufferSize));
        pBlockNode = pBlockNode->pNext;
    }

    osPrintf("Requested size     : %d\n", pHead->reqBufferSize);
    osPrintf("Aligned size       : %d\n", pHead->actBufferSize);
    osPrintf("Number of buffers  : %d (%d buffers free)\n",
              pHead->numOfBuffers, pHead->numOfFreeBuffers);
}



