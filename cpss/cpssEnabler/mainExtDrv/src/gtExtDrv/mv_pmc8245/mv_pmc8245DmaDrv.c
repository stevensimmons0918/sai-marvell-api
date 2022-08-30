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
#include <gtExtDrv/drivers/gtMpc107Dma.h>


/**
* @internal extDrvDmaWrite function
* @endinternal
*
* @brief   Write a given buffer to the given address using the Dma.
*
* @param[in] address                  - The destination  to write to.
* @param[in] buffer                   - The  to be written.
* @param[in] length                   - Length of buffer in words.
* @param[in] burstLimit               - Number of words to be written on each burst.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*
* @note 1. The given buffer is allways 4 bytes aligned, any further allignment
*       requirements should be handled internally by this function.
*       2. The given buffer may be allocated from an uncached memory space, and
*       it's to the function to handle the cache flushing.
*       3. The Prestera Driver assumes that the implementation of the DMA is
*       blocking, otherwise the Driver functionality might be damaged.
*
*/
GT_STATUS extDrvDmaWrite
(
    IN  GT_UINTPTR  address,
    IN  GT_U32      *buffer,
    IN  GT_U32      length,
    IN  GT_U32      burstLimit
)
{
    GT_U32  sourceAddr;     /* The source address to begin the transfer from*/
    GT_U32  destAddr;       /* Destination address for the data to be       */
                            /* transfered.                                  */
    DMA_ENGINE  dmaEngine;  /* The Dma engine to perform the burst through. */
    DMA_STATUS  dmaStatus;

    GT_U32  numOfBursts = 0;

    /* Set the dma function parameters.     */
    dmaEngine   = DMA_ENG_0;
    sourceAddr  = (GT_U32)buffer;
    destAddr    = (GT_U32)address;

    /*osPrintf("src = 0x%x, dst = 0x%x, length = %d.\n",sourceAddr,destAddr,
             length);*/

    /* Wait until the Dma is Idle.          */
    while(mpc107DmaIsEngineActive(dmaEngine) != DMA_ENGINE_IDLE);

    /* Flush the buffer data    */
    extDrvMgmtCacheFlush(GT_MGMT_DATA_CACHE_E,buffer,length * sizeof(GT_U32));
                       
    numOfBursts = length / burstLimit;

    while ( numOfBursts )
    {
      dmaStatus = mpc107DmaTransfer(dmaEngine,sourceAddr,destAddr,burstLimit * 4);
      if(dmaStatus != DMA_OK)
      {
          return GT_FAIL;
      }

      sourceAddr += (burstLimit * 4);
      destAddr += (burstLimit * 4);
      numOfBursts--;

      /* Wait until the Dma is Idle.          */
      while(mpc107DmaIsEngineActive(dmaEngine) != DMA_ENGINE_IDLE);
    }

    return GT_OK;
}



/**
* @internal extDrvDmaRead function
* @endinternal
*
* @brief   Read a memory block from a given address.
*
* @param[in] address                  - The  to read from.
* @param[in] length                   - Length of the memory block to read (in words).
* @param[in] burstLimit               - Number of words to be read on each burst.
*
* @param[out] buffer                   - The read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*
* @note 1. The given buffer is allways 4 bytes aligned, any further allignment
*       requirements should be handled internally by this function.
*       2. The given buffer may be allocated from an uncached memory space, and
*       it's to the function to handle the cache flushing.
*       3. The Prestera Driver assumes that the implementation of the DMA is
*       blocking, otherwise the Driver functionality might be damaged.
*
*/
GT_STATUS extDrvDmaRead
(
    IN  GT_UINTPTR  address,
    IN  GT_U32      length,
    IN  GT_U32      burstLimit,
    OUT GT_U32      *buffer
)
{
    GT_U32  sourceAddr;     /* The source address to begin the transfer from*/
    GT_U32  destAddr;       /* Destination address for the data to be       */
                            /* transfered.                                  */
    DMA_ENGINE  dmaEngine;  /* The Dma engine to perform the burst through. */
    DMA_STATUS  dmaStatus;

    GT_U32  numOfBursts = 0;
        
    /* Set the dma function parameters.     */
    dmaEngine   = DMA_ENG_1;
    sourceAddr  = (GT_U32)address;
    destAddr    = (GT_U32)buffer;

    /*osPrintf("src = 0x%x, dst = 0x%x, length = %d.\n",sourceAddr,destAddr,
             length);*/

    /* Wait until the Dma is Idle.          */
    while(mpc107DmaIsEngineActive(dmaEngine) != DMA_ENGINE_IDLE);

    extDrvMgmtCacheFlush(GT_MGMT_DATA_CACHE_E,buffer,sizeof(GT_U32) * length);
    extDrvMgmtCacheInvalidate(GT_MGMT_DATA_CACHE_E,buffer,sizeof(GT_U32) * length);

    numOfBursts = length / burstLimit;

    while ( numOfBursts )
    { 
      dmaStatus = mpc107DmaTransfer(dmaEngine,sourceAddr,destAddr,burstLimit * 4);
      if(dmaStatus != DMA_OK)
      {
          return GT_FAIL;
      }

      sourceAddr += (burstLimit * 4);
      destAddr += (burstLimit * 4);
      numOfBursts--;  

      /* Wait until the Dma is Idle.          */
      while(mpc107DmaIsEngineActive(dmaEngine) != DMA_ENGINE_IDLE);
    }

    return GT_OK;
}



