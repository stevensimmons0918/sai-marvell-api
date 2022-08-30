/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#include <gtExtDrv/drivers/gtCacheMng.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <asicSimulation/SCIB/scib.h>

void*  prvExtDrvDmaPtr = NULL;     /*used only for print */
GT_UINTPTR prvExtDrvDmaPhys = 0;   /*used only for print */
uint64_t prvExtDrvDmaPhys64 = 0;   /*used by logics in : noKmDrvUtils.c */
GT_U32 prvExtDrvDmaLen = 0;        /*used only for print */
static GT_VOID *actualMallocOfDma = NULL;/* hold actual malloc of DMA , this malloc may hold 'spare' for alignment purposes */

static GT_VOID *actualMallocOfDdr = NULL;/* hold actual malloc of DDR , this malloc may hold 'spare' for alignment purposes */
GT_UINTPTR prvExtDrvDdrPhys = 0;   /*used only for print */
uint64_t prvExtDrvDdrPhys64 = 0;   /*used only for print */
GT_U32 prvExtDrvDdrLen = 0;        /*used only for print */






/**
* @internal extDrvMgmtCacheFlush function
* @endinternal
*
* @brief   Flush to RAM content of cache
*
* @param[in] type                     -  of cache memory data/interaction
* @param[in] address_PTR              - starting address of memory block to flush
* @param[in] size                     -  of memory block
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/

GT_STATUS extDrvMgmtCacheFlush
(
    IN GT_MGMT_CACHE_TYPE_ENT   type,
    IN void                     *address_PTR,
    IN size_t                   size
)
{
    /*empty ... like in ..\linuxNoKernelModule\noKmDrvCacheMng.c */
    (void) type;
    (void) address_PTR;
    (void) size;
    return GT_OK;
}


/**
* @internal extDrvMgmtCacheInvalidate function
* @endinternal
*
* @brief   Invalidate current content of cache
*
* @param[in] type                     -  of cache memory data/interaction
* @param[in] address_PTR              - starting address of memory block to flush
* @param[in] size                     -  of memory block
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/

GT_STATUS extDrvMgmtCacheInvalidate
(
    IN GT_MGMT_CACHE_TYPE_ENT   type,
    IN void                     *address_PTR,
    IN size_t                   size
)
{
    /*empty ... like in ..\linuxNoKernelModule\noKmDrvCacheMng.c */
    (void) type;
    (void) address_PTR;
    (void) size;
    return GT_OK;
}

/* same default value used in ..\linuxNoKernelModule\noKmDrvCacheMng.c */
static GT_U32   dmaLen  = _2M;
/* this value comes from internal_prvNoKmDrv_configure_dma_per_devNum()
   that force 'dma base' to be 1M aligned */
static GT_U32   dmaMask = (_1M-1);
#define DMA_LEN                 dmaLen
#define ALIGNMENT_MASK          dmaMask

#define DDR_LEN                 _8M
#define DDR_ALIGNMENT_MASK          (_4M-1)

/**
* @internal extDrvSetDmaSize function
* @endinternal
*
* @brief   Set the size of the DMA area , and alignment.
*          NOTE: 1. default is _2MB size on _1M alignment
*                2. this function relevant only before first time that one of
*                   next functions is called.
*
* @param[in] dmaSize                  - The size of the DMA area.(must be '64K*y'  value )
* @param[in] dmaAlignment             - The size of the DMA alignment. (must be '2^x' value)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_ALREADY_EXIST         - the DMA size already been set and used.
* @retval GT_BAD_PARAM             - the DMA alignment is not '2^x' value.
*                                    or the DMA size is not '64K*y' value.
*/
GT_STATUS extDrvSetDmaSize
(
    IN GT_U32 dmaSize,
    IN GT_U32 dmaAlignment
)
{
    GT_U32  ii,counter=0;

    if(prvExtDrvDmaLen != 0)
    {
        return GT_ALREADY_EXIST;
    }

    if(dmaSize & 0xFFFF)
    {
        /*DMA size is not '64K*y' value*/
        return GT_BAD_PARAM;
    }

    /*make sure dmaAlignment is single bit value*/
    for(ii = 0 ; ii < 32 ; ii++)
    {
        if((1<<ii) & dmaAlignment)
        {
            counter ++;
        }
    }

    if(counter != 1)
    {
        return GT_BAD_PARAM;
    }

    dmaLen  = dmaSize;
    dmaMask = dmaAlignment-1;

    return GT_OK;
}

/**
* @internal extDrvGetDmaSize function
* @endinternal
*
* @brief   Get the size of the DMA area
*
* @param[out] dmaSize                  - The size of the DMA area.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvGetDmaSize
(
    OUT GT_U32 * dmaSize
)
{
    if(prvExtDrvDmaLen == 0) /* was not initialized yet ! */
    {
        GT_UINTPTR dmaBase;
        /* this will allocate the first memory */
        extDrvGetDmaBase(&dmaBase);
    }

    *dmaSize = prvExtDrvDmaLen;
    return GT_OK;
}
/**
* @internal extDrvGetDmaBase function
* @endinternal
*
* @brief   Get the base address of the DMA area need for Virt2Phys and Phys2Virt
*         translaton
*
* @param[out] dmaBase                  -the base address of the DMA area.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvGetDmaBase
(
    OUT GT_UINTPTR * dmaBase
)
{
    GT_UINTPTR  alignOffset;

    if(actualMallocOfDma == NULL)
    {

        /* allocate more than needed to ensure 'spcae' for alignment */
        actualMallocOfDma = osMalloc(DMA_LEN + ALIGNMENT_MASK);

        /* calc the offset to make the memory to be aligned */
        alignOffset = (GT_UINTPTR)actualMallocOfDma & ALIGNMENT_MASK;
        alignOffset = (ALIGNMENT_MASK+1) - alignOffset;
        alignOffset &= ALIGNMENT_MASK;

        /* save values according to the needed alignment */
        prvExtDrvDmaPhys   = ((GT_UINTPTR)actualMallocOfDma) + alignOffset;
        prvExtDrvDmaPhys64 = prvExtDrvDmaPhys;/*on 64 bit CPU both are 64bits */

        prvExtDrvDmaLen    = DMA_LEN;/* use the exact needed size and not according to allocated size */

#if __WORDSIZE == 64
        /* notify simulation about the 'upper 32 bits' of address */
        scibDmaUpper32BitsSet(prvExtDrvDmaPhys64 >> 32);
#else
        /* notify simulation about the 'upper 32 bits' of address */
        scibDmaUpper32BitsSet(0);
#endif
    }

#if __WORDSIZE == 64
    * dmaBase  = prvExtDrvDmaPhys64;
#else
    * dmaBase  = prvExtDrvDmaPhys;
#endif

    return GT_OK;
}

/**
* @internal extDrvGetDdrBase function
* @endinternal
*
* @brief   Get the base address of the DDR area need for CM3 simulation
*         translaton
*
* @param[out] dmaBase                  - the base address of the DMA area.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvDdrAllocate
(
    IN GT_U32        size,
    OUT GT_VOID_PTR   *ddrBase
)
{
    GT_UINTPTR  alignOffset;

    if(size>DDR_LEN)
    {
        return GT_OUT_OF_RANGE;
    }

    if(actualMallocOfDdr == NULL)
    {

        /* allocate more than needed to ensure 'spcae' for alignment */
        actualMallocOfDdr = osMalloc(DDR_LEN + DDR_ALIGNMENT_MASK);

        /* calc the offset to make the memory to be aligned */
        alignOffset = (GT_UINTPTR)actualMallocOfDdr & DDR_ALIGNMENT_MASK;
        alignOffset = (DDR_ALIGNMENT_MASK+1) - alignOffset;
        alignOffset &= DDR_ALIGNMENT_MASK;

        /* save values according to the needed alignment */
        prvExtDrvDdrPhys   = ((GT_UINTPTR)actualMallocOfDdr) + alignOffset;
        prvExtDrvDdrPhys64 = prvExtDrvDdrPhys;/*on 64 bit CPU both are 64bits */

        prvExtDrvDdrLen    = DDR_LEN;/* use the exact needed size and not according to allocated size */
    }

    *ddrBase  = (GT_VOID_PTR)prvExtDrvDdrPhys;

    return GT_OK;
}

