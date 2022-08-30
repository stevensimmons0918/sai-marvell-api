/*******************************************************************************
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
* @file gmOsMem.h
*
* @brief Operating System CB definitions. Memory manipulation facility.
*
*
* @version   1
********************************************************************************
*/

#ifndef __gmOsMemh
#define __gmOsMemh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/
#include <os/simTypes.h>
/*
#include <extServices/os/gtOs/gtGenTypes.h>
*/

/************* Defines ********************************************************/


/*******************************************************************************
* GM_OS_BZERO_FUNC
*
* DESCRIPTION:
*       Fills the first n-bytes characters of the specified buffer with 0
*
* INPUTS:
*       start  - start address of memory block to be zeroed
*       nbytes - size of block to be set
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef void (*GM_OS_BZERO_FUNC)
(
    IN char * start,
    IN GT_U32 nbytes
);

/*******************************************************************************
* GM_OS_MEM_SET_FUNC
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
typedef void * (*GM_OS_MEM_SET_FUNC)
(
    IN void * start,
    IN int    symbol,
    IN GT_U32 size
);

/*******************************************************************************
* GM_OS_MEM_CPY_FUNC
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
typedef void * (*GM_OS_MEM_CPY_FUNC)
(
    IN void *       destination,
    IN const void * source,
    IN GT_U32       size
);


/*******************************************************************************
* GM_OS_MEM_CMP_FUNC
*
* DESCRIPTION:
*       Compare 'size' characters from the object pointed to by 'str1' to
*       the object pointed to by 'str2'.
*
* INPUTS:
*       str1 - first memory area
*       str2 - second memory area
*       size - size of memory to compare
*
* OUTPUTS:
*       None
*
* RETURNS:
*       > 0  - if str1 is alphabetic bigger than str2
*       == 0 - if str1 is equal to str2
*       < 0  - if str1 is alphabetic smaller than str2
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_32 (*GM_OS_MEM_CMP_FUNC)
(
    IN const void  *str1,
    IN const void  *str2,
    IN GT_U32       size
);



/*******************************************************************************
* GM_OS_STATIC_MALLOC_FUNC
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
*       insufficient memory available
*
* COMMENTS:
*       Usage of this function is only on FIRST initialization.
*
*******************************************************************************/
typedef void * (*GM_OS_STATIC_MALLOC_FUNC)
(
    IN GT_U32 size
);

/*******************************************************************************
* GM_OS_MALLOC_FUNC
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
typedef void * (*GM_OS_MALLOC_FUNC)
(
    IN GT_U32 size
);

/*******************************************************************************
* GM_OS_REALLOC_FUNC
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
typedef void * (*GM_OS_REALLOC_FUNC)
(
    IN void * ptr ,
    IN GT_U32 size
);

/*******************************************************************************
* GM_OS_FREE_FUNC
*
* DESCRIPTION:
*       De-allocates or frees a specified memory block.
*
* INPUTS:
*       memblock - previously allocated memory block to be freed
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       Usage of this function is NEVER during initialization.
*
*******************************************************************************/
typedef void (*GM_OS_FREE_FUNC)
(
    IN void* const memblock
);

/*******************************************************************************
* GM_OS_CACHE_DMA_MALLOC_FUNC
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
typedef void *(*GM_OS_CACHE_DMA_MALLOC_FUNC)
(
    IN GT_U32 size
);

/*******************************************************************************
* GM_OS_CACHE_DMA_FREE_FUNC
*
* DESCRIPTION:
*       Free the buffer acquired with osCacheDmaMalloc()
*
* INPUTS:
*       ptr - pointer to malloc/free buffer
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_STATUS (*GM_OS_CACHE_DMA_FREE_FUNC)
(
    IN void *ptr
);

/*******************************************************************************
* GM_OS_PHY_TO_VIRT_FUNC
*
* DESCRIPTION:
*       Converts physical address to virtual.
*
* INPUTS:
*       phyAddr  - physical address
*
* OUTPUTS:
*       virtAddr - virtual address
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*GM_OS_PHY_TO_VIRT_FUNC)
(
    IN  GT_U32 phyAddr,
    OUT GT_U32 *virtAddr
);

/*******************************************************************************
* GM_OS_VIRT_TO_PHY_FUNC
*
* DESCRIPTION:
*       Converts virtual address to physical.
*
* INPUTS:
*       virtAddr - virtual address
*
* OUTPUTS:
*       phyAddr  - physical address
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*GM_OS_VIRT_TO_PHY_FUNC)
(
    IN  GT_U32 virtAddr,
    OUT GT_U32 *phyAddr
);

/* GM_OS_MEM_BIND_STC -
    structure that hold the "os memory" functions needed be bound to gm.

*/
typedef struct{
    GM_OS_BZERO_FUNC                 osMemBzeroFunc;
    GM_OS_MEM_SET_FUNC               osMemSetFunc;
    GM_OS_MEM_CPY_FUNC               osMemCpyFunc;
    GM_OS_MEM_CMP_FUNC               osMemCmpFunc;
    GM_OS_STATIC_MALLOC_FUNC         osMemStaticMallocFunc;
    GM_OS_MALLOC_FUNC                osMemMallocFunc;
    GM_OS_REALLOC_FUNC               osMemReallocFunc;
    GM_OS_FREE_FUNC                  osMemFreeFunc;
    GM_OS_CACHE_DMA_MALLOC_FUNC      osMemCacheDmaMallocFunc;
    GM_OS_CACHE_DMA_FREE_FUNC        osMemCacheDmaFreeFunc;
    GM_OS_PHY_TO_VIRT_FUNC           osMemPhyToVirtFunc;
    GM_OS_VIRT_TO_PHY_FUNC           osMemVirtToPhyFunc;
}GM_OS_MEM_BIND_STC;

#ifdef __cplusplus
}
#endif

#endif  /* __gmOsMemh */



