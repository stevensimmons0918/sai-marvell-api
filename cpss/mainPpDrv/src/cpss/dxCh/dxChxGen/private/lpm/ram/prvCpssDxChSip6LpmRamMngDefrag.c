/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChSip6LpmRamMngDefrag.c
*
* DESCRIPTION:
*       Implementation of the LPM algorithm, for the use of UC and MC engines.
*
* DEPENDENCIES:
*       None
*
* FILE REVISION NUMBER:
*       $Revision: 24 $
*
*******************************************************************************/

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/private/prvCpssDevMemManager.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamMng.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip6LpmRamMng.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip6LpmRamMngDefrag.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChIpLpmDbg.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamDbg.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

static const GT_U8                                          startSubNodeAddress[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] =
    {START_ADDR_OF_SUBNODE_0_IN_GROUP_OF_NODES,START_ADDR_OF_SUBNODE_1_IN_GROUP_OF_NODES,
     START_ADDR_OF_SUBNODE_2_IN_GROUP_OF_NODES,START_ADDR_OF_SUBNODE_3_IN_GROUP_OF_NODES,
     START_ADDR_OF_SUBNODE_4_IN_GROUP_OF_NODES,START_ADDR_OF_SUBNODE_5_IN_GROUP_OF_NODES};
static const GT_U8                                           endSubNodeAddress[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] =
    {END_ADDR_OF_SUBNODE_0_IN_GROUP_OF_NODES,END_ADDR_OF_SUBNODE_1_IN_GROUP_OF_NODES,
     END_ADDR_OF_SUBNODE_2_IN_GROUP_OF_NODES,END_ADDR_OF_SUBNODE_3_IN_GROUP_OF_NODES,
     END_ADDR_OF_SUBNODE_4_IN_GROUP_OF_NODES,END_ADDR_OF_SUBNODE_5_IN_GROUP_OF_NODES};

/*global variables macros*/
#define PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.ipLpmDir.sip6IpLpmRamDefragSrc._var)

/**
* @internal prvCpssDxChLpmSip6RamMngFindIfSwapAreaCanBeUsefulToFindFreeBlock
*           function
* @endinternal
*
* @brief   Check if in case we use a swap area, new memory can be added.
*         this means that there is free block near the used block and
*         we can use them to create a new bigger block
*
* @param[in] oldHwBucketOffsetHandle  - the old handle we want to extend
* @param[in] newBucketSize            - the size (in lines) of the new bucket
*
* @param[out] freeBlockCanBeFoundPtr   - (pointer to)
*                                      GT_TRUE: empty block can be found if we use swap area
*                                      GT_FALSE: empty block can NOT be found even if we use swap area
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmSip6RamMngFindIfSwapAreaCanBeUsefulToFindFreeBlock
(
    IN  GT_UINTPTR                              oldHwBucketOffsetHandle,
    IN  GT_U32                                  newBucketSize,
    OUT GT_BOOL                                 *freeBlockCanBeFoundPtr
)
{
    GT_DMM_BLOCK        *bucketBlock;
    GT_U32              sizeOfOptionalUsedAndFreeBlock;
    GT_U32              newBucketSizeInWords;

    /* for a given oldHwBucketOffsetHandle we check if there is free memory near it
      (on top or below) that can be merged with the current oldHwBucketOffsetHandle
      memory space to create a bigger block */

    bucketBlock = (GT_DMM_BLOCK *)oldHwBucketOffsetHandle;

    if( (oldHwBucketOffsetHandle == 0)||
        (DMM_BLOCK_STATUS(bucketBlock)!= DMM_BLOCK_ALLOCATED)||
        (DMM_BLOCK_STAMP(bucketBlock) != DMM_STAMP_USED) )
    {
        cpssOsPrintf("oldHwBucketOffsetHandle was not allocated\n");
        *freeBlockCanBeFoundPtr=GT_FALSE;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngFindIfSwapAreaCanBeUsefulToFindFreeBlock - GT_FAIL \n");
    }

    newBucketSizeInWords = newBucketSize * PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS;

    sizeOfOptionalUsedAndFreeBlock = prvCpssDmmCheckResizeAvailableWithSameMemory(oldHwBucketOffsetHandle,newBucketSizeInWords);
    if ((sizeOfOptionalUsedAndFreeBlock>0)&&(sizeOfOptionalUsedAndFreeBlock!=DMM_BLOCK_NOT_FOUND))
    {
        *freeBlockCanBeFoundPtr=GT_TRUE;
    }
    else
    {
        *freeBlockCanBeFoundPtr=GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal
*           prvCpssDxChLpmSip6RamMngFindIfSwapAreaCanBeUsefulToFindFreeBlockForRegToCompConversion
*           function
* @endinternal
*
* @brief  Check if incase we use a swap area, new memory can be added in delete
*         operation. this means that there is free block near the used block
*         and we can use them to create a new bigger block
*         the check is done on 6 Gons becoming one Gon - case of delete
*
* @param[in] bucketPtr                - bucket holding the the old handles we
*                                       want to extend
* @param[in] newBucketSize            - the size (in lines) of the new bucket
*
* @param[out] freeBlockCanBeFoundPtr  - (pointer to)
*                                      GT_TRUE: empty block can be found if we use swap area
*                                      GT_FALSE: empty block can NOT be found even if we use swap area
* @param[out] freeBlockGonIndexPtr    - index of the GON were the swap migth
*                                       help to find the needed memory
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmSip6RamMngFindIfSwapAreaCanBeUsefulToFindFreeBlockForRegToCompConversion
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    IN  GT_U32                                  newBucketSize,
    OUT GT_BOOL                                 *freeBlockCanBeFoundPtr,
    OUT GT_U32                                  *freeBlockGonIndexPtr
)
{
    GT_STATUS           retVal=GT_OK;
    GT_U32              j=0;

    for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
    {
        if (bucketPtr->hwGroupOffsetHandle[j]!=0)
        {
             /* for a given hwGroupOffsetHandle[j] we check if there is free memory near it
                (on top or below) that can be merged with the current hwGroupOffsetHandle[j]
                memory space to create a bigger block */
            retVal = prvCpssDxChLpmSip6RamMngFindIfSwapAreaCanBeUsefulToFindFreeBlock(bucketPtr->hwGroupOffsetHandle[j],
                                                                                      newBucketSize,
                                                                                      freeBlockCanBeFoundPtr);
            if (retVal!=GT_OK)
            {
                *freeBlockCanBeFoundPtr=GT_FALSE;
                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "error in prvCpssDxChLpmSip6RamMngFindIfSwapAreaCanBeUsefulToFindFreeBlock\n");
            }
            if (*freeBlockCanBeFoundPtr==GT_TRUE)
            {
                *freeBlockGonIndexPtr=j;
                break;
            }
            if(bucketPtr->bucketType!=CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                break;
            }
        }
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmSip6RamMngCheckIfShrinkOperationUsefulForDefrag function
* @endinternal
*
* @brief   Check if shrink operation may help for defrag
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] lpmMemInfoPtr            - (pointer to) the lpm memory linked list holding
*                                      all blocked mapped to this octetIndex
* @param[in] protocol                 - the protocol
* @param[in] neededMemoryBlocksPtr    - (pointer to) memory data needed for
*                                       shrink as:
*                                       1.the old bucket handle, if the value is
*                                         0, it means this is a new bucket, no
*                                         resize
*                                       2.the size (in lines) of the new bucket
*                                       3.the index of the bank to shrink
*                                       4.indication if the swap are is in use
*                                         GT_TRUE-swapArea will be used when
*                                         doing shrink
*                                         GT_FALSE - swapArea will NOT be used
*                                         when doing shrink
* @param[out] freeBlockCanBeFoundPtr   - (pointer to)
*                                      GT_TRUE: empty block can be found if we shrink memory
*                                      GT_FALSE: empty block can NOT be found even if we shrink memory
* @param[out] freeSpaceTakenFromBlockIndexArray - array holding the number of
*                                       lines needed from each block to do
*                                       defrag and to find place for all needed
*                                       sizes in all GONs
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmSip6RamMngCheckIfShrinkOperationUsefulForDefrag
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC          *lpmMemInfoPtr,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT        protocol,
    IN  PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC   *neededMemoryBlocksPtr,
    OUT GT_BOOL                                     *freeBlockCanBeFoundPtr,
    OUT GT_U32                                      freeSpaceTakenFromBlockIndexArray[]
)
{
    GT_STATUS     retVal=GT_OK;
    GT_U32        oldBlockIndex=0;
    GT_U32        sumOfFreeBlockAfterShrink=0;
    GT_BOOL       useSwapArea=GT_FALSE;
    GT_DMM_BLOCK  *usedBlock;
    GT_U32        oldBlockSize=0, newBucketSize=0;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempMemInfoPtr;/* use for going over the list of blocks per octet */
    GT_U32        i=0,j=0;

    GT_DMM_PARTITION    *partition;
    GT_U32              totalNewBucketSize=0;/* counter to sum the size of all the GONs*/
    GT_U32              totalOldBlockSize=0;/* counter to sum the size of all old GONs*/
    GT_U32              sumOfTotalFreeBlockInAllBanksAfterShrink=0;/* counter to sum all free space in all allocated
                                                                    & unallocated banks*/
    GT_BOOL             continueToNextGon = GT_FALSE;
    GT_U32              oldSpaceUsedFromBlockIndexArray[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS]={0}; /* in words not in lines */

    /*  Optimized checks prior modifications
        As in Falcon multiple GONs may be created or extended even at specific octet as result of single prefix -
        we should check in advance there will be enough free space for all new / extended GONs,
        prior any manipulation to one of the GON.
        By summing all new / extend buckets sizes, deducting their current sizes &
        comparing that with the total free space in all allocated & unallocated banks. */

    /* first we calculate the total size needed in all GONs - we treat it a a single GON */
    for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
    {
        if (((neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_BLOCK_NOT_FOUND)||
             (neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_MALLOC_FAIL))&&
             (neededMemoryBlocksPtr->neededMemoryBlocksSizes[j]!=0))
        {
            totalNewBucketSize += neededMemoryBlocksPtr->neededMemoryBlocksSizes[j];

            /* in case there is an old bucket we check its size */
            if (neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[j]!=0)
            {
                usedBlock = (GT_DMM_BLOCK *)neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[j];
                oldBlockSize = SIZE_IN_WORDS(usedBlock);
                totalOldBlockSize += oldBlockSize;
            }
            if (neededMemoryBlocksPtr->regularNode==GT_FALSE)
            {
                break;
            }
        }
    }

    if (totalNewBucketSize != 0)
    {
        /* calculate all free space in all allocated & unallocated banks*/
        /*all calculations in partition are in words */
        totalNewBucketSize = totalNewBucketSize*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS;

        if (totalOldBlockSize >= totalNewBucketSize)
        {
            /* all new allocation needed will fit into the old memory that will be freed */
            /* nothing to do at the moment if later we will find that there is actualy space
               then we will set *freeBlockCanBeFoundPtr=GT_TRUE; */
        }
        else
        {
            /* allocated banks */
            tempMemInfoPtr = lpmMemInfoPtr;/* parameter to the function is already for the specific octet we are working on */
            while((tempMemInfoPtr!= NULL)&&(tempMemInfoPtr->structsMemPool!=0))
            {
                partition = (GT_DMM_PARTITION *)tempMemInfoPtr->structsMemPool;
                if( (partition == 0) ||
                    (partition == (GT_DMM_PARTITION*)((GT_UINTPTR)0xFFFFFFFE)))
                {
                    /* should never happen */
                    *freeBlockCanBeFoundPtr=GT_FALSE;
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngCheckIfShrinkOperationUsefulForDefrag, "
                                                          "wrong partition bind to the octet - GT_FAIL \n");
                }
                sumOfFreeBlockAfterShrink = prvCpssDmmGetEmptyMemoryLeft(partition);
                sumOfTotalFreeBlockInAllBanksAfterShrink += sumOfFreeBlockAfterShrink;

                tempMemInfoPtr = tempMemInfoPtr->nextMemInfoPtr;
            }

            /*  summing all new / extend buckets sizes, deducting their current sizes &
                comparing that with the total free space in all allocated

                COMMENT - only in add is correct to do totalNewBucketSize - totalOldBlockSize
                in delete we do totalOldBlockSize-totalNewBucketSize in
                prvCpssDxChLpmSip6RamMngCheckIfShrinkOperationUsefulForDefragForRegToCompConversion*/
            if ((totalNewBucketSize - totalOldBlockSize) <= sumOfTotalFreeBlockInAllBanksAfterShrink)
            {
                 /* all new allocation needed migth fit into the old memory that will be freed plus the shrinked one */
                 /* nothing to do at the moment if later we will find that there is actualy space
                    then we will set *freeBlockCanBeFoundPtr=GT_TRUE; */
            }
            else
            {
                /* unallocated banks */
                for (i = 0; i < shadowPtr->numOfLpmMemories; i++)
                {
                    /* if the block is used BUT not by the specific input protocol - not violate full wire speed */
                    if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,protocol,i)==GT_FALSE)
                    {
                        partition = (GT_DMM_PARTITION *)shadowPtr->lpmRamStructsMemPoolPtr[i];
                        if( (partition == 0) ||
                            (partition == (GT_DMM_PARTITION*)((GT_UINTPTR)0xFFFFFFFE)))
                        {
                            /* should never happen */
                            *freeBlockCanBeFoundPtr=GT_FALSE;
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngCheckIfShrinkOperationUsefulForDefrag, "
                                                              "wrong partition bind to the octet - GT_FAIL \n");
                        }
                        sumOfFreeBlockAfterShrink = prvCpssDmmGetEmptyMemoryLeft(partition);
                        sumOfTotalFreeBlockInAllBanksAfterShrink += sumOfFreeBlockAfterShrink;
                    }
                }

                /*  summing all new / extend buckets sizes, deducting their current sizes &
                    comparing that with the total free space in all allocated & unallocated banks.

                    COMMENT - only in add is correct to do totalNewBucketSize - totalOldBlockSize
                    in delete we do totalOldBlockSize-totalNewBucketSize in
                    prvCpssDxChLpmSip6RamMngCheckIfShrinkOperationUsefulForDefragForRegToCompConversion*/

                if ((totalNewBucketSize - totalOldBlockSize) > sumOfTotalFreeBlockInAllBanksAfterShrink)
                {
                    for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                    {
                        if ((neededMemoryBlocksPtr->neededMemoryBlocksSizes[j] > 0) &&
                            (neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_BLOCK_NOT_FOUND))
                        {
                            /* merge should check if new bank can be allocated */
                            neededMemoryBlocksPtr->mergeOperationUsefulForDefragArr[j] = GT_TRUE;
                            neededMemoryBlocksPtr->octetIndexForMergeArr[j] = PRV_CPSS_DXCH_SIP6_LPM_RAM_MERGE_NOT_CHECKED;
                            neededMemoryBlocksPtr->bankIndexForMergeArr[j] = PRV_CPSS_DXCH_SIP6_LPM_RAM_MERGE_NOT_CHECKED;
                        }
                    }
                    *freeBlockCanBeFoundPtr=GT_FALSE;
                    return GT_OK;
                }
            }
        }
    }
    else
    {
        /* should never happen since we get to this function because we need new memory */
        *freeBlockCanBeFoundPtr=GT_FALSE;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngCheckIfShrinkOperationUsefulForDefrag - GT_FAIL \n");
    }
    /* Now after we did optimized checks prior modifications we can find the banks we want to shrink */

    /* first we try to allocate all GONs in the same bank of the old bucket*/

    for (j= 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
    {
        continueToNextGon = GT_FALSE;
        if ((neededMemoryBlocksPtr->neededMemoryBlocksSizes[j]==0)||
            ((neededMemoryBlocksPtr->neededMemoryBlocksSizes[j]!=0)&&
             (neededMemoryBlocksPtr->neededMemoryBlocks[j] != DMM_BLOCK_NOT_FOUND)&&
             (neededMemoryBlocksPtr->neededMemoryBlocks[j] != DMM_MALLOC_FAIL)&&
             (neededMemoryBlocksPtr->neededMemoryBlocks[j] != 0)))
         {
             /* the block was alocated or no need to allocate it */
             continue;
         }

        *freeBlockCanBeFoundPtr = GT_FALSE;

        /* Phase 1 */
        /* Go over the partition were the old bucket is defined
           check if shrink can be useful when looking at the
           allocated block as optional to be freed using the swap_area_1 */
        if (neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[j] != 0)
        {
            oldBlockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[j]) /
                                                                                    (shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&oldBlockIndex);
            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
            }

            useSwapArea = GT_TRUE; /* we can use swap area if needed*/
            usedBlock = (GT_DMM_BLOCK *)(neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[j]);
            oldBlockSize = SIZE_IN_WORDS(usedBlock);
            newBucketSize = neededMemoryBlocksPtr->neededMemoryBlocksSizes[j];
            sumOfFreeBlockAfterShrink = prvCpssDmmCheckIfShrinkOperationUsefulForDefrag(shadowPtr->lpmRamStructsMemPoolPtr[oldBlockIndex],
                                                                                        newBucketSize * PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS,
                                                                                        oldBlockSize,
                                                                                        &useSwapArea);
            if(sumOfFreeBlockAfterShrink!=0)
            {
                if (useSwapArea==GT_TRUE)/* flag is still true --> old space should be used */
                {
                    oldSpaceUsedFromBlockIndexArray[oldBlockIndex]+=oldBlockSize;
                    sumOfFreeBlockAfterShrink -= oldBlockSize;
                }
                if (((sumOfFreeBlockAfterShrink+oldSpaceUsedFromBlockIndexArray[oldBlockIndex])-
                     (freeSpaceTakenFromBlockIndexArray[oldBlockIndex]))>=
                     (newBucketSize*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS))
                {
                    neededMemoryBlocksPtr->shrinkOperationUsefulForDefragArr[j]=GT_TRUE;
                    neededMemoryBlocksPtr->shrinkOperationUsefulForDefragGlobalFlag = GT_TRUE;
                    neededMemoryBlocksPtr->bankIndexForShrinkArr[j] = oldBlockIndex;
                    neededMemoryBlocksPtr->swapUsedForShrinkArr[j] = useSwapArea;
                    /* keep what was already taken */
                    freeSpaceTakenFromBlockIndexArray[oldBlockIndex]+= (newBucketSize * PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS);

                    *freeBlockCanBeFoundPtr = GT_TRUE;
                    continue; /* continue to next GON*/
                }
            }
        }

        /* Phase 2 - continue looking */
        /* Go over all the list of partition related to the octet*/
        tempMemInfoPtr = lpmMemInfoPtr;

        while((tempMemInfoPtr!= NULL)&&(tempMemInfoPtr->structsMemPool!=0))
        {
            useSwapArea = GT_FALSE;
            oldBlockSize = 0; /* using the old block in previous stage did not help */
            newBucketSize = neededMemoryBlocksPtr->neededMemoryBlocksSizes[j];
            sumOfFreeBlockAfterShrink = prvCpssDmmCheckIfShrinkOperationUsefulForDefrag(tempMemInfoPtr->structsMemPool,
                                                                                        newBucketSize * PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS,
                                                                                        oldBlockSize,
                                                                                        &useSwapArea);

            if(sumOfFreeBlockAfterShrink!=0)
            {
                if (((sumOfFreeBlockAfterShrink + oldSpaceUsedFromBlockIndexArray[tempMemInfoPtr->ramIndex])-
                     (freeSpaceTakenFromBlockIndexArray[tempMemInfoPtr->ramIndex]))>=
                     (newBucketSize*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS))
                {
                    neededMemoryBlocksPtr->shrinkOperationUsefulForDefragArr[j]=GT_TRUE;
                    neededMemoryBlocksPtr->shrinkOperationUsefulForDefragGlobalFlag = GT_TRUE;
                    neededMemoryBlocksPtr->bankIndexForShrinkArr[j] = tempMemInfoPtr->ramIndex;
                    neededMemoryBlocksPtr->swapUsedForShrinkArr[j] = useSwapArea;

                    /* keep what was already taken */
                    freeSpaceTakenFromBlockIndexArray[tempMemInfoPtr->ramIndex]+= (newBucketSize*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS);

                    *freeBlockCanBeFoundPtr=GT_TRUE;
                    continueToNextGon = GT_TRUE;
                    break;/* break the while and continue to next GON*/
                }
            }

            tempMemInfoPtr = tempMemInfoPtr->nextMemInfoPtr;
        }
        if (continueToNextGon==GT_TRUE)
        {
            continue;
        }

        /* Phase 3 */
        /* if we get here it means we did not find place to shrink in the bounded memory,
           try to see if shrink memory can be useful in unbounded blocks */
        for (i = 0; i < shadowPtr->numOfLpmMemories; i++)
        {
            /* if the block is used BUT not by the specific input protocol - not violate full wire speed */
            if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,protocol,i)==GT_FALSE)
            {
                /* check if shrink is an option */
                useSwapArea = GT_FALSE;
                oldBlockSize = 0; /* using the old block in previous stage did not help */
                newBucketSize = neededMemoryBlocksPtr->neededMemoryBlocksSizes[j];
                sumOfFreeBlockAfterShrink = prvCpssDmmCheckIfShrinkOperationUsefulForDefrag(shadowPtr->lpmRamStructsMemPoolPtr[i],
                                                                                           newBucketSize * PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS,
                                                                                           oldBlockSize,
                                                                                           &useSwapArea);

                if ((sumOfFreeBlockAfterShrink!=0)&&
                    (((sumOfFreeBlockAfterShrink + oldSpaceUsedFromBlockIndexArray[i])-
                      (freeSpaceTakenFromBlockIndexArray[i]))>=
                      (newBucketSize*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS)))
                {
                    neededMemoryBlocksPtr->shrinkOperationUsefulForDefragArr[j]=GT_TRUE;
                    neededMemoryBlocksPtr->shrinkOperationUsefulForDefragGlobalFlag = GT_TRUE;
                    neededMemoryBlocksPtr->bankIndexForShrinkArr[j] = i;
                    neededMemoryBlocksPtr->swapUsedForShrinkArr[j] = useSwapArea;

                    /* keep what was already taken */
                    freeSpaceTakenFromBlockIndexArray[i]+=(newBucketSize*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS);

                    *freeBlockCanBeFoundPtr=GT_TRUE;
                    continue;
                }
                else
                {
                    /* all phases did not find a free block -->shrink will not help */
                    *freeBlockCanBeFoundPtr = GT_FALSE;
                    if (shadowPtr->defragSip6MergeEnable == GT_TRUE)
                    {
                        /* merge should check if new bank can be allocated */
                        neededMemoryBlocksPtr->mergeOperationUsefulForDefragArr[j] = GT_TRUE;
                        neededMemoryBlocksPtr->octetIndexForMergeArr[j] = PRV_CPSS_DXCH_SIP6_LPM_RAM_MERGE_NOT_CHECKED;
                        neededMemoryBlocksPtr->bankIndexForMergeArr[j] = PRV_CPSS_DXCH_SIP6_LPM_RAM_MERGE_NOT_CHECKED;
                        continue;
                    }
                    else
                    {
                        /* undo all assignments done */
                        cpssOsMemSet(neededMemoryBlocksPtr->shrinkOperationUsefulForDefragArr, 0,
                                     sizeof(neededMemoryBlocksPtr->shrinkOperationUsefulForDefragArr));
                        cpssOsMemSet(neededMemoryBlocksPtr->bankIndexForShrinkArr, 0,
                                     sizeof(neededMemoryBlocksPtr->bankIndexForShrinkArr));
                        cpssOsMemSet(neededMemoryBlocksPtr->swapUsedForShrinkArr, 0,
                                     sizeof(neededMemoryBlocksPtr->swapUsedForShrinkArr));
                        neededMemoryBlocksPtr->shrinkOperationUsefulForDefragGlobalFlag = GT_FALSE;
                        return GT_OK;
                    }
                }
            }
        }

        /* all phases did not find a free block -->shrink will not help */
        if (*freeBlockCanBeFoundPtr == GT_FALSE)
        {
            if (shadowPtr->defragSip6MergeEnable == GT_TRUE)
            {
                /* merge should check if new bank can be allocated */
                neededMemoryBlocksPtr->mergeOperationUsefulForDefragArr[j] = GT_TRUE;
                neededMemoryBlocksPtr->octetIndexForMergeArr[j] = PRV_CPSS_DXCH_SIP6_LPM_RAM_MERGE_NOT_CHECKED;
                neededMemoryBlocksPtr->bankIndexForMergeArr[j] = PRV_CPSS_DXCH_SIP6_LPM_RAM_MERGE_NOT_CHECKED;
            }
            else
            {
                /* undo all assignments done */
                cpssOsMemSet(neededMemoryBlocksPtr->shrinkOperationUsefulForDefragArr, 0,
                             sizeof(neededMemoryBlocksPtr->shrinkOperationUsefulForDefragArr));
                cpssOsMemSet(neededMemoryBlocksPtr->bankIndexForShrinkArr, 0,
                             sizeof(neededMemoryBlocksPtr->bankIndexForShrinkArr));
                cpssOsMemSet(neededMemoryBlocksPtr->swapUsedForShrinkArr, 0,
                             sizeof(neededMemoryBlocksPtr->swapUsedForShrinkArr));
                neededMemoryBlocksPtr->shrinkOperationUsefulForDefragGlobalFlag = GT_FALSE;
                return GT_OK;
            }
        }
    }

    /* if we get here it means all needed memory can be allocated */
    return GT_OK;
}

/**
* @internal
*           prvCpssDxChLpmSip6RamMngCheckIfShrinkOperationUsefulForDefragForRegToCompConversion
*           function
* @endinternal
*
* @brief   Check if shrink operation may help for defrag, the check is done on 6
*          Gons becoming one Gon - case of delete
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] lpmMemInfoPtr            - (pointer to) the lpm memory linked list holding
*                                        all blocked mapped to this octetIndex
* @param[in] protocol                - the protocol
* @param[in] hwGroupOffsetHandle   - (pointer to)old bucket regular GONs handles
* @param[in] newBucketSize            - the size of the new compress GON
* @param[out] freeBlockCanBeFoundPtr  - (pointer to)
*                                      GT_TRUE: empty block can be found if we shrink memory
*                                      GT_FALSE: empty block can NOT be found even if we shrink memory
* @param[out] bankIndexForShrinkPtr - (pointer to) the bank index we should shrink
* @param[out] swapUsedForShrinkPtr  - (pointer to) GT_TRUE: swap is used to
*                                       hold old GONs when shrinking the bank
*                                       GT_FALSE: swap is not used to hold
*                                       old GONs when shrinking the bank
*
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmSip6RamMngCheckIfShrinkOperationUsefulForDefragForRegToCompConversion
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC          *lpmMemInfoPtr,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT        protocol,
    IN  GT_UINTPTR                                  *hwGroupOffsetHandle,
    IN  GT_U32                                      newBucketSize,
    OUT GT_BOOL                                     *freeBlockCanBeFoundPtr,
    OUT GT_U32                                      *bankIndexForShrinkPtr,
    OUT GT_BOOL                                     *swapUsedForShrinkPtr
)
{
    GT_STATUS     retVal=GT_OK;
    GT_U32        oldBlockIndex=0;
    GT_U32        sumOfFreeBlockAfterShrink=0;
    GT_BOOL       useSwapArea=GT_FALSE;
    GT_DMM_BLOCK  *usedBlock;
    GT_U32        oldBlockSize=0;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempMemInfoPtr;/* use for going over the list of blocks per octet */
    GT_U32        i=0,j=0;

    GT_DMM_PARTITION    *partition;
    GT_U32              totalNewBucketSize=0;/* counter to sum the size of all the GONs*/
    GT_U32              totalOldBlockSize=0;/* counter to sum the size of all old GONs*/
    GT_U32              sumOfTotalFreeBlockInAllBanksAfterShrink=0;/* counter to sum all free space in all allocated
                                                                    & unallocated banks*/
    GT_U32              oldSpaceUsedFromBlockIndexArray[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS]={0}; /* in words not in lines */

    /*  Optimized checks prior modifications
        As in Falcon multiple GONs may be created or extended even at specific octet as result of single prefix -
        we should check in advance there will be enough free space for all new / extended GONs,
        prior any manipulation to one of the GON.
        By summing all new / extend buckets sizes, deducting their current sizes &
        comparing that with the total free space in all allocated & unallocated banks. */


    /* all calculations in partition are in words */
    totalNewBucketSize = newBucketSize*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS;

    /* first we calculate the total size to be freed in all GONs - we treat it a a single GON */
    for (j= 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
    {
        if (hwGroupOffsetHandle[j] != 0)
        {
            usedBlock = (GT_DMM_BLOCK *)(hwGroupOffsetHandle[j]);
            oldBlockSize = SIZE_IN_WORDS(usedBlock);
            totalOldBlockSize += oldBlockSize;
        }
    }

    if (totalNewBucketSize != 0)
    {
        /* calculate all free space in all allocated & unallocated banks*/

        /* allocated banks */
        tempMemInfoPtr = lpmMemInfoPtr;/* parameter to the function is already for the specific octet we are working on */
        while((tempMemInfoPtr!= NULL)&&(tempMemInfoPtr->structsMemPool!=0))
        {
            partition = (GT_DMM_PARTITION *)tempMemInfoPtr->structsMemPool;
            if( (partition == 0) ||
                (partition == (GT_DMM_PARTITION*)((GT_UINTPTR)0xFFFFFFFE)))
            {
                /* should never happen */
                *freeBlockCanBeFoundPtr=GT_FALSE;
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngCheckIfShrinkOperationUsefulForDefrag, "
                                                      "wrong partition bind to the octet - GT_FAIL \n");
            }
            sumOfFreeBlockAfterShrink = prvCpssDmmGetEmptyMemoryLeft(partition);
            sumOfTotalFreeBlockInAllBanksAfterShrink += sumOfFreeBlockAfterShrink;

            tempMemInfoPtr = tempMemInfoPtr->nextMemInfoPtr;
        }

        /* unallocated banks */
        for (i = 0; i < shadowPtr->numOfLpmMemories; i++)
        {
            /* if the block is used BUT not by the specific input protocol - not violate full wire speed */
            if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,protocol,i)==GT_FALSE)
            {
                partition = (GT_DMM_PARTITION *)shadowPtr->lpmRamStructsMemPoolPtr[i];
                if( (partition == 0) ||
                    (partition == (GT_DMM_PARTITION*)((GT_UINTPTR)0xFFFFFFFE)))
                {
                    /* should never happen */
                    *freeBlockCanBeFoundPtr=GT_FALSE;
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngCheckIfShrinkOperationUsefulForDefrag, "
                                                      "wrong partition bind to the octet - GT_FAIL \n");
                }
                sumOfFreeBlockAfterShrink = prvCpssDmmGetEmptyMemoryLeft(partition);
                sumOfTotalFreeBlockInAllBanksAfterShrink += sumOfFreeBlockAfterShrink;
            }
        }

        /*  summing all old buckets sizes, deducting their new size &
            comparing that with the total free space in all allocated & unallocated banks.*/
        if((totalOldBlockSize-totalNewBucketSize)>sumOfTotalFreeBlockInAllBanksAfterShrink)
        {
            *freeBlockCanBeFoundPtr=GT_FALSE;
            return GT_OK;
        }
    }
    else
    {
        /* should never happen since we get to this function because we need new memory */
        *freeBlockCanBeFoundPtr=GT_FALSE;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngCheckIfShrinkOperationUsefulForDefrag - GT_FAIL \n");
    }
    /* Now after we did optimized checks prior modifications we can find the banks we want to shrink */

    /* we try to allocate the Gon */
    /* sum all the memory used in all the banks related to the octet,
       6 gons can be located in different banks or in same banks */
    for (j= 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
    {
        if (hwGroupOffsetHandle[j]==0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected hwGroupOffsetHandle[j]=0\n");
        }
        oldBlockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(hwGroupOffsetHandle[j])/
                                                                        (shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
        retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&oldBlockIndex);
        if (retVal != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
        }
        usedBlock = (GT_DMM_BLOCK *)(hwGroupOffsetHandle[j]);
        oldBlockSize = SIZE_IN_WORDS(usedBlock);
        oldSpaceUsedFromBlockIndexArray[oldBlockIndex]+=oldBlockSize;
    }

    /* Phase 1 */
    /* Go over the partition were the old buckets are defined
       check if shrink can be useful when looking at the
       allocated block as optional to be freed using the swap_area_1 */

    /* try to find memory for needed size in a specific bank when taking
       into consideration the space that are occupied by the Gon in the specific bank */
    for (oldBlockIndex= 0; oldBlockIndex < shadowPtr->numOfLpmMemories; oldBlockIndex++)
    {
        if (oldSpaceUsedFromBlockIndexArray[oldBlockIndex]!=0)
        {
            useSwapArea = GT_TRUE; /* we can use swap area if needed*/
            oldBlockSize = oldSpaceUsedFromBlockIndexArray[oldBlockIndex];
            sumOfFreeBlockAfterShrink = prvCpssDmmCheckIfShrinkOperationUsefulForDefrag(shadowPtr->lpmRamStructsMemPoolPtr[oldBlockIndex],
                                                                                        newBucketSize * PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS,
                                                                                        oldBlockSize,
                                                                                        &useSwapArea);
            if (sumOfFreeBlockAfterShrink>0)
            {
                *bankIndexForShrinkPtr = oldBlockIndex;
                *swapUsedForShrinkPtr = useSwapArea;
                *freeBlockCanBeFoundPtr = GT_TRUE;
                return GT_OK;
            }
        }
    }

    /* Phase 2 - continue looking */
    /* Go over all the list of partition related to the octet*/
    tempMemInfoPtr = lpmMemInfoPtr;

    while((tempMemInfoPtr!= NULL)&&(tempMemInfoPtr->structsMemPool!=0))
    {
        useSwapArea = GT_FALSE;
        oldBlockSize = 0; /* using the old block in previous stage did not help */
        sumOfFreeBlockAfterShrink = prvCpssDmmCheckIfShrinkOperationUsefulForDefrag(tempMemInfoPtr->structsMemPool,
                                                                                    newBucketSize * PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS,
                                                                                    oldBlockSize,
                                                                                    &useSwapArea);
        if (sumOfFreeBlockAfterShrink>0)
        {
            *bankIndexForShrinkPtr = tempMemInfoPtr->ramIndex;
            *swapUsedForShrinkPtr = useSwapArea;
            *freeBlockCanBeFoundPtr = GT_TRUE;
            return GT_OK;
        }
        tempMemInfoPtr = tempMemInfoPtr->nextMemInfoPtr;
    }

    /* Phase 3 */
    /* if we get here it means we did not find place to shrink in the bounded memory,
       try to see if shrink memory can be useful in unbounded blocks */
    for (i = 0; i < shadowPtr->numOfLpmMemories; i++)
    {
        /* if the block is used BUT not by the specific input protocol - not violate full wire speed */
        if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,protocol,i)==GT_FALSE)
        {
            /* check if shrink is an option */
            useSwapArea = GT_FALSE;
            oldBlockSize = 0; /* using the old block in previous stage did not help */
            sumOfFreeBlockAfterShrink = prvCpssDmmCheckIfShrinkOperationUsefulForDefrag(shadowPtr->lpmRamStructsMemPoolPtr[i],
                                                                                       newBucketSize * PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS,
                                                                                       oldBlockSize,
                                                                                       &useSwapArea);
            if (sumOfFreeBlockAfterShrink>0)
            {
                *bankIndexForShrinkPtr = i;
                *swapUsedForShrinkPtr = useSwapArea;
                *freeBlockCanBeFoundPtr = GT_TRUE;
                return GT_OK;
            }
        }
    }

    /* if we get here it means all phases did not find a free block -->shrink will not help */
    *freeBlockCanBeFoundPtr=GT_FALSE;
    return GT_OK;
}
/*******************************************************************************
* handleMcSrcRootNodeForShrink
*
* DESCRIPTION:
*       This function updates Src root node in shrink operation
*
* INPUTS:
*       parentNodeDataPtr     - pointer to the bucket to parent node parameters.
*       shareDevsList         - pointer to device list
*       shareDevsListLen      - devices length
*       newAddr               - group should point to new SRC Root address
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK on success, or
*       GT_OUT_OF_CPU_MEM - if failed to allocate CPU memory, or
*       GT_OUT_OF_PP_MEM - if failed to allocate PP memory.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS handleMcSrcRootNodeForShrink
(
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC    *parentNodeDataPtr,
    IN GT_U8                                    *shareDevsList,
    IN GT_U32                                    shareDevsListLen,
    IN GT_U32                                    newAddr
)
{
    GT_STATUS   rc = GT_OK;
    PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC   groupOfNodes;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *rootBucketPtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr = NULL;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     lastGroupbucketType;
    GT_BOOL lastLevelBucketEmbLeaves        = GT_FALSE;
    GT_U32 leafLineOffset;
    GT_U32 leafOffsetInLine;
    GT_U32 sipPtrOffset = 2;
    GT_U32 sipPtrLength = 20;
    GT_U32 startBitOfLeafInLine;
    GT_U32 gonDataArray[4] = {0};

    shadowPtr = parentNodeDataPtr->shadowPtr;

    rootBucketPtr = shadowPtr->vrRootBucketArray[parentNodeDataPtr->vrId].rootBucket[parentNodeDataPtr->protocol];

    cpssOsMemSet(&groupOfNodes, 0, sizeof(groupOfNodes));

    /* this mean parentNodeDataPtr->swapAreaAlsoUsedForAddOnParentLevel==GT_FALSE
      need to use the preallocated memory - insert case */

    /* get line offset and leaf position in line */
    rc = prvCpssDxChLpmSip6RamMngGetLastGonNodeOffsetAndLeafPosition(rootBucketPtr,
                                                                     parentNodeDataPtr->groupPtr,
                                                                     parentNodeDataPtr->prefixLength,
                                                                     &leafLineOffset,
                                                                     &leafOffsetInLine,
                                                                     &lastGroupbucketType,
                                                                     &lastLevelBucketEmbLeaves);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChLpmSip6RamMngGetLastGonNodeOffsetAndLeafPosition failed");
    }
    if (lastLevelBucketEmbLeaves == GT_TRUE)
    {
        switch (lastGroupbucketType)
        {
            case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
            case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
                    startBitOfLeafInLine = 89 - leafOffsetInLine*23;
            break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "wrong node type");
        }
    }
    else
    {
        /* we support only exact match group, so GON's pointed by last bucket
           incorporate only leaves */
        startBitOfLeafInLine = 92-23*leafOffsetInLine;
    }

    for (i = 0; i < shareDevsListLen; i++)
    {

        /* now change ptr in needed leaf */
        rc = prvCpssDxChReadTableMultiEntry(shareDevsList[i],
                                             CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                             leafLineOffset,
                                             1,
                                             gonDataArray);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "LPM read fails");
        }

        U32_SET_FIELD_IN_ENTRY_MAC(gonDataArray,startBitOfLeafInLine+sipPtrOffset,sipPtrLength,newAddr);
        /* Write the leaf line */
        rc = prvCpssDxChSip6RamMngWriteMultiEntry(shareDevsList[i],
                                             CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                             leafLineOffset,
                                             1,
                                             gonDataArray);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "LPM write fails");
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChLpmSip6RamMngShrinkHwWriteAndPointersUpdatePhase1Phase2 function
* @endinternal
*
* @brief   Build the HW representation of the block according to the Shadow,
*         write the data to the HW and update the pointers.
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] bucketBaseAddress        - the bucket's base address
* @param[in] oldBucketShadowPtr       - the old SW bucket handle
* @param[in] gonIndex                 - the index of the gon we are working on,
*                                       related to the oldBucketShadowPtr
* @param[in] needToFreeAllocationInCaseOfFail - GT_TRUE: need to free memory
*                                      GT_FALSE: NO need to free memory
* @param[in] tempHwAddrHandleToBeFreed - Hw handle to free in case of fail
* @param[in] parentUpdateParams        - parameters needed for parent update
* @param[in] phaseFlag                 - GT_TRUE: phase1 update
*                                        GT_FALSE: phase2 update
* @param[in] ucMcType                 - indicates whether bucketPtr is the uc,
*                                       mc src tree, mc group bucket
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssDxChLpmSip6RamMngShrinkHwWriteAndPointersUpdatePhase1Phase2
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                             *shadowPtr,
    IN GT_U32                                                       bucketBaseAddress,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC                      *oldBucketShadowPtr,
    IN GT_U32                                                       gonIndex,
    IN GT_BOOL                                                      needToFreeAllocationInCaseOfFail,
    IN GT_UINTPTR                                                   tempHwAddrHandleToBeFreed,
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC                        *parentUpdateParams,
    IN PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE_ENT       phaseFlag,
    IN PRV_CPSS_DXCH_LPM_ALLOC_TYPE_ENT                             ucMcType
)
{
    GT_STATUS                                retVal=GT_OK;

    /* the fields for writing the bucket to the HW */
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRange;
    GT_U32                                   blockIndex=0;
    GT_U32                                   i=0;

    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
    GT_U32 shareDevListLen;
    GT_U32 groupOfNodesLines;

    /* oldBucketShadowPtr will be used to build the HW from SW shadow and for
       updated pointers when we move the HW bucket location */
    pRange              = oldBucketShadowPtr->rangeList;

    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;

    if (phaseFlag == PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE1_E)
    {
        if (gonIndex==0xFFFF)
        {
            /* prepare the root node */
            cpssOsMemSet(&(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_VAR_GET(rootGroupOfNodes)),0,
                         sizeof(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_VAR_GET(rootGroupOfNodes)));

            if(oldBucketShadowPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                retVal = lpmFalconFillRegularData(oldBucketShadowPtr,
                                          shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                          &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_VAR_GET(rootGroupOfNodes).regularNodesArray[0]));
            }
            else
            {
                retVal = lpmFalconFillCompressedData(oldBucketShadowPtr,
                                          shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                          &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_VAR_GET(rootGroupOfNodes).compressedNodesArray[0]),
                                          NULL);
            }
            if(ucMcType!=PRV_CPSS_DXCH_LPM_ALLOC_UC_TYPE_E)
            {
                /* if this is a SRC Root nodeMemAddr should not be overwrite
                   overwrite nodeMemAddr is holding the VR id and should not be changed */
                oldBucketShadowPtr->nodeMemAddr =  bucketBaseAddress;
            }
        }
        else
        {
            /* create the bucket data, and update the ranges hw address */
            cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_VAR_GET(tempGroupOfNodes), 0,
                         sizeof(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_VAR_GET(tempGroupOfNodes)));

            /* set in lpmFalconCalcNeededMemory to specify we are moving from compress to regular,
               and the old block is located in index 0 despite the different gonIndex that can be 0-5 */
            if(gonIndex==0xFFFE)
            {
                /* build needed gon*/
                retVal = getMirrorGroupOfNodesDataAndUpdateRangesAddress(oldBucketShadowPtr->bucketType,
                                                                     startSubNodeAddress[0],
                                                                     endSubNodeAddress[0],
                                                                     pRange,
                                                                     shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                                     bucketBaseAddress,
                                                                     &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_VAR_GET(tempGroupOfNodes)[0]));
            }
            else
            {
                /* build needed gon*/
                retVal = getMirrorGroupOfNodesDataAndUpdateRangesAddress(oldBucketShadowPtr->bucketType,
                                                                     startSubNodeAddress[gonIndex],
                                                                     endSubNodeAddress[gonIndex],
                                                                     pRange,
                                                                     shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                                     bucketBaseAddress,
                                                                     &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_VAR_GET(tempGroupOfNodes)[gonIndex]));
            }
        }
        if (retVal != GT_OK)
        {
            /* this is a fail that do not support any rollback operation,
               because if we get here it is after we did all calculations
               needed and we verified that the data can be written to the HW.
               if we get a fail it means a fatal error that should not be
               happen and the behavior is unpredicted */

            /* just need to free any allocations done in previous stage */
            if(needToFreeAllocationInCaseOfFail == GT_TRUE)
            {
                if (tempHwAddrHandleToBeFreed==0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected tempHwAddrHandleToBeFreed=0\n");
                }
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandleToBeFreed)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                if (retVal != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                }
                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandleToBeFreed);
                prvCpssDmmFree(tempHwAddrHandleToBeFreed);
            }

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkHwWriteAndPointersUpdatePhase1Phase2 - GT_FAIL \n");
        }


        for (i = 0; i < shareDevListLen; i++)
        {
            if (gonIndex==0xFFFF)
            {
                /* write root node */
                retVal = prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[i],
                                                         bucketBaseAddress,
                                                         &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_VAR_GET(rootGroupOfNodes)),
                                                         &groupOfNodesLines);
            }
            else
            {
                /* set in lpmFalconCalcNeededMemory to specify we are moving from compress to regular,
                   and the old block is located in index 0 despite the different gonIndex that can be 0-5 */
                if(gonIndex==0xFFFE)
                {
                    retVal =  prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[i],
                                                              bucketBaseAddress,
                                                              &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_VAR_GET(tempGroupOfNodes)[0]),
                                                              &groupOfNodesLines);
                }
                else
                {
                    retVal =  prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[i],
                                                              bucketBaseAddress,
                                                              &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_VAR_GET(tempGroupOfNodes)[gonIndex]),
                                                              &groupOfNodesLines);
                }
            }
            if (retVal != GT_OK)
            {
                 /* this is a fail that do not support any rollback operation,
                   because if we get here it is after we did all calculations
                   needed and we verified that the data can be written to the HW.
                   if we get a fail it means a fatal error that should not be
                   happen and the behavior is unpredicted */

                /* just need to free any allocations done in previous stage */
                if((needToFreeAllocationInCaseOfFail == GT_TRUE)&&
                   (tempHwAddrHandleToBeFreed != 0)&&
                   (tempHwAddrHandleToBeFreed != DMM_BLOCK_NOT_FOUND) &&
                   (tempHwAddrHandleToBeFreed != DMM_MALLOC_FAIL))
                {
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandleToBeFreed)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                    if (retVal != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                    }
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandleToBeFreed);

                    prvCpssDmmFree(tempHwAddrHandleToBeFreed);
                }
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "error in prvCpssDxChLpmRamMngShrinkUpdatePointersPhase1Phase2 - GT_FAIL \n");
            }
        }
    }
    else /* PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE2_E */
    {
        parentUpdateParams->addOperation=GT_FALSE;
        parentUpdateParams->swapAreaAlsoUsedForAddOnParentLevel=GT_FALSE;
        parentUpdateParams->neededMemoryBlocksInfoPtr = NULL;

        parentUpdateParams->shadowPtr = shadowPtr;

        parentUpdateParams->funcCallCounter=1;

        /* if we are working on the Root or MC SRC Root */
        switch (parentUpdateParams->level)
        {
        case 0:/* this is the root */
            parentUpdateParams->isRootNode=GT_TRUE;

            if(ucMcType == PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E)
            {
               /* in case of Root SRC - need to update the MC group pointer */
               retVal = handleMcSrcRootNodeForShrink(parentUpdateParams,
                                                     shareDevsList,
                                                     shareDevListLen,
                                                     bucketBaseAddress);

            }
            else
            {
                 /* in case of Root move need to update the VR table */
                retVal = prvCpssDxChLpmRamMngVrfEntryUpdate(parentUpdateParams->vrId,
                                                            parentUpdateParams->protocol,
                                                            parentUpdateParams->shadowPtr);
            }

            break;
        case 1:
            /* if the level is 1 then need to update the Root pointer-hwBucketOffsetHandle*/
            parentUpdateParams->isRootNode=GT_TRUE;
            parentUpdateParams->bucketPtr = parentUpdateParams->bucketPtrArray[0];
            parentUpdateParams->pRange = NULL;

            retVal = updateHwRangeDataAndGonPtr(parentUpdateParams);
            break;
        default:
            /* in all other cases need to update level-1 pointers , so we need to read level-2 data
               pointing to level-1 */
            parentUpdateParams->isRootNode=GT_FALSE;

            parentUpdateParams->bucketPtr =  parentUpdateParams->bucketPtrArray[(parentUpdateParams->level)-2];
            parentUpdateParams->pRange = parentUpdateParams->rangePtrArray[(parentUpdateParams->level)-2];
            /* we need to go into updateHwRangeDataAndGonPtr with the level of the node and not the level of the GONs */
            parentUpdateParams->level--;
            retVal = updateHwRangeDataAndGonPtr(parentUpdateParams);
            /* assign back the level of the GONs */
            parentUpdateParams->level++;
            break;
        }

        if (retVal != GT_OK)
        {
            /* this is a fail that do not support any rollback operation,
                   because if we get here it is after we did all calculations
                   needed and we verified that the data can be written to the HW.
                   if we get a fail it means a fatal error that should not be
                   happen and the behavior is unpredicted */

                /* just need to free any allocations done in previous stage */
                if((needToFreeAllocationInCaseOfFail == GT_TRUE)&&
                   (tempHwAddrHandleToBeFreed != 0)&&
                   (tempHwAddrHandleToBeFreed != DMM_BLOCK_NOT_FOUND) &&
                   (tempHwAddrHandleToBeFreed != DMM_MALLOC_FAIL))
                {
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandleToBeFreed)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                    if (retVal != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                    }
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandleToBeFreed);

                    prvCpssDmmFree(tempHwAddrHandleToBeFreed);
                }
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "error in prvCpssDxChLpmRamMngShrinkUpdatePointersPhase1Phase2 - GT_FAIL \n");
        }
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmSip6RamMngShrinkPointersUpdate function
* @endinternal
*
* @brief   update the pointers for a moved block according to the Shadow
*          parameters.
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] bucketBaseAddress        - the bucket's base address we moved
* @param[in] oldBucketShadowPtr       - the old SW bucket handle
* @param[in] gonIndex                 - the index of the gon we are working on,
*                                       related to the oldBucketShadowPtr
* @param[in] needToFreeAllocationInCaseOfFail - GT_TRUE: need to free memory
*                                               GT_FALSE: NO need to free memory
* @param[in] tempHwAddrHandleToBeFreed - Hw handle to free in case of fail
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssDxChLpmSip6RamMngShrinkPointersUpdate
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN GT_U32                                       bucketBaseAddress,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC      *oldBucketShadowPtr,
    IN GT_U32                                       gonIndex,
    IN GT_BOOL                                      needToFreeAllocationInCaseOfFail,
    IN GT_UINTPTR                                   tempHwAddrHandleToBeFreed
)
{
    GT_STATUS                                retVal=GT_OK;
    GT_U32                                   blockIndex=0;
    GT_U32                                   parentNodeAdress;  /* address of the parent pointing to the blocked we moved */
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT     protocol;          /* the protocol of the block we moved */
    GT_U32                                   offset;            /* offset of the pointer value in the data */
    GT_U32                                   length=20;         /* length of the address field = 20 bits */
    GT_U32                                   i=0;
    GT_BOOL                                  isRoot=GT_FALSE;
    GT_U32                                   vrId;              /* used for updating the VR in case Root node was moved */
    GT_U32                                   gonDataArray[4] = {0};/* 1 line / 4 words of data */
    GT_U8                                    *shareDevsList;    /* List of devices sharing this LPM structure   */
    GT_U32                                   shareDevsListLen;

    shareDevsList    = shadowPtr->workDevListPtr->shareDevs;
    shareDevsListLen = shadowPtr->workDevListPtr->shareDevNum;

    /* oldBucketShadowPtr will be used to for updated pointers when we move the HW bucket location */
    if (gonIndex==0xFFFF)
    {
        if (oldBucketShadowPtr->pointingRangeMemAddr!=0xFFFFFFFF)/* the value is different then 0xFFFFFFFF only in srcRoot case */
        {
            parentNodeAdress = oldBucketShadowPtr->pointingRangeMemAddr;
            offset = oldBucketShadowPtr->fifthAddress;/* fifthAddress is used to hold the offset of the address inside the leaf */
        }
        else
        {
            isRoot = GT_TRUE;
            vrId = oldBucketShadowPtr->nodeMemAddr; /* nodeMemAddr is used in this case to hold the VR id in this case */

            /* offset and length are not used
               we need to update the VR table*/
        }
        protocol   = GET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwBucketOffsetHandle);
    }
    else
    {
        if (oldBucketShadowPtr->hwBucketOffsetHandle!=0)
        {
            /* we are dealing with the GON of the root */
            parentNodeAdress = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(oldBucketShadowPtr->hwBucketOffsetHandle) + gonIndex;
        }
        else
        {
            parentNodeAdress = oldBucketShadowPtr->nodeMemAddr + gonIndex;
        }
        protocol    = GET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex]);
        offset      = 0;
    }

    /* if we are working on the Root */
    if (isRoot==GT_TRUE)
    {
         /* in case of Root move need to update the VR table */

         /* the bucket shadow is already updated with the new address
            the block was moved to - done before we enter this function,
            so the prvCpssDxChLpmRamMngVrfEntryUpdate API will build the
            correct data to be updated to the VR table according to the shadow */
         retVal = prvCpssDxChLpmRamMngVrfEntryUpdate(vrId,
                                                     protocol,
                                                     shadowPtr);
    }
    else
    {
       /* in all other blocks move we need to use the parentNodeAdress we got from the shadow data */
        for (i = 0; i < shareDevsListLen; i++)
        {
            /* change ptr in needed leaf/node */
            retVal = prvCpssDxChReadTableMultiEntry(shareDevsList[i],
                                                 CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                 parentNodeAdress,
                                                 1,
                                                 gonDataArray);
            if (retVal != GT_OK)
            {
                break;
            }

            /* update the new address of the block moved */
            U32_SET_FIELD_IN_ENTRY_MAC(gonDataArray,offset,length,bucketBaseAddress);

            /* Write the leaf/node line */
            retVal = prvCpssDxChSip6RamMngWriteMultiEntry(shareDevsList[i],
                                             CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                             parentNodeAdress,
                                             1,
                                             gonDataArray);
            if (retVal != GT_OK)
            {
                break;
            }
        }
    }

    if (retVal != GT_OK)
    {
        /* this is a fail that do not support any rollback operation,
           because if we get here it is after we did all calculations
           needed and we verified that the data can be written to the HW.
           if we get a fail it means a fatal error that should not be
           happen and the behavior is unpredicted */

        /* just need to free any allocations done in previous stage */
        if((needToFreeAllocationInCaseOfFail == GT_TRUE)&&
           (tempHwAddrHandleToBeFreed != 0)&&
           (tempHwAddrHandleToBeFreed != DMM_BLOCK_NOT_FOUND) &&
           (tempHwAddrHandleToBeFreed != DMM_MALLOC_FAIL))
        {
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandleToBeFreed)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
            }
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandleToBeFreed);

            prvCpssDmmFree(tempHwAddrHandleToBeFreed);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "error in prvCpssDxChLpmSip6RamMngShrinkPointersUpdate - GT_FAIL \n");
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmSip6RamMngShrinkFreeOldBucket function
* @endinternal
*
* @brief   Free old bucket HW data.
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] bankIndex                - the index of the bank to shrink
* @param[in] oldHwAddrHandle          - the bucket's old HW address
* @param[in] oldBucketShadowPtr       - the old SW bucket handle
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssDxChLpmSip6RamMngShrinkFreeOldBucket
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr,
    IN GT_U32                                  bankIndex,
    IN GT_UINTPTR                              oldHwAddrHandle,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *oldBucketShadowPtr
)
{
    GT_STATUS   retVal=GT_OK;
    GT_U32      blockIndex=0;

    /* free this bucket's old memory */
    if ((oldBucketShadowPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
        (oldBucketShadowPtr->bucketType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E))
    {
        /*  set pending flag for future need */
        if (oldHwAddrHandle==0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected oldHwAddrHandle=0\n");
        }
        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(oldHwAddrHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
        retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
        if (retVal != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
        }
        if (blockIndex!=bankIndex)
        {
            /* we should be working on the bankIndex we got as parameter to the function */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkFreeOldBucket - GT_FAIL \n");
        }
        if(oldHwAddrHandle==0)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected oldHwAddrHandle=0\n");
        }
        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(oldHwAddrHandle);

        prvCpssDmmFree(oldHwAddrHandle);
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmSip6RamMngShrinkCopyToOrFromSwapArea1and2 function
* @endinternal
*
* @brief   Write old buckets to and from swap area 1/2
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] bankIndex                - the index of the bank to shrink
* @param[in] oldBucketShadowPtr       - the old SW bucket handle
* @param[in] oldMemSize               - size of the old bucket in lines
* @param[in] gonIndex                 - the index of the gon we are working on,
*                                       related to the oldBucketShadowPtr
* @param[in] copyToFromSwapArea       - The direction of the copy operation,
*                                      TO or FROM the swap area
* @param[in]  parentUpdateParams      - parameters needed for parent update
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmSip6RamMngShrinkCopyToOrFromSwapArea1and2
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                             *shadowPtr,
    IN GT_U32                                                       bankIndex,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC                      *oldBucketShadowPtr,
    IN GT_U32                                                       oldMemSize,
    IN GT_U32                                                       gonIndex,
    IN PRV_CPSS_DXCH_LPM_RAM_COPY_TO_FROM_FIRST_SWAP_AREA_ENT       copyToFromSwapArea,
    IN GT_U32                                                       swapAreaIndex,
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC                        *parentUpdateParams
)
{
    GT_STATUS                                retVal=GT_OK;
    GT_DMM_PARTITION                         *partition;
    GT_UINTPTR                                oldHwAddrHandle=0;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      oldHwProtocol;/* the protocol of the bucket we are moving */
    GT_UINTPTR                                tempHwAddrHandle=0;

    GT_U32                                   tempAddr = 0;     /* Temporary address                */
    GT_U32                                   blockIndex=0;
    PRV_CPSS_DXCH_LPM_ALLOC_TYPE_ENT         ucMcType=PRV_CPSS_DXCH_LPM_ALLOC_UC_TYPE_E;/* indicates whether bucketPtr is the uc,
                                                                                           mc src tree, mc group bucket */

    GT_BOOL     needToFreeAllocationInCaseOfFail = GT_FALSE;
    GT_UINTPTR  tempHwAddrHandleToBeFreed = 0;

    partition = (GT_DMM_PARTITION *)shadowPtr->lpmRamStructsMemPoolPtr[bankIndex];
    /* if the partition is empty --> the block is empty */
    if(partition->allocatedBytes == 0)
    {
        /* nothing to shrink
           should never happen since this function should be called after
           checking shrink operation may be useful for defrag */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkCopyToOrFromSwapArea1and2 - GT_FAIL \n");
    }

    /* oldBucketShadowPtr will be used to build the HW from SW shadow and for
       updated pointers when we move the HW bucket location */
    if (gonIndex==0xFFFF)
    {
        oldHwAddrHandle     = oldBucketShadowPtr->hwBucketOffsetHandle;
        oldHwProtocol       = GET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwBucketOffsetHandle);
        if (oldBucketShadowPtr->pointingRangeMemAddr!=0xFFFFFFFF)
        {
            /* this is SRC Root
               all other cases are treated same as UC bucket */
            ucMcType = PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E;
        }
    }
    else
    {
        /* set in lpmFalconCalcNeededMemory to specify we are moving from compress to regular,
           and the old block is located in index 0 despite the different gonIndex that can be 0-5 */
        if(gonIndex==0xFFFE)
        {
            oldHwAddrHandle     = oldBucketShadowPtr->hwGroupOffsetHandle[0];
            oldHwProtocol       = GET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwGroupOffsetHandle[0]);
        }
        else
        {
            oldHwAddrHandle     = oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex];
            oldHwProtocol       = GET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex]);
        }
    }

    if(copyToFromSwapArea==PRV_CPSS_DXCH_LPM_RAM_COPY_TO_FIRST_SWAP_AREA_E)
    {
        /* create the HW data to be writen to swap_area_1/2 according to the oldBucketPtr */
        switch(swapAreaIndex)
        {
        case 1:
            if(shadowPtr->swapMemoryAddr==0)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected shadowPtr->swapMemoryAddr=0\n");
            }
            tempAddr = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->swapMemoryAddr);
            break;
        case 2:
            if(shadowPtr->secondSwapMemoryAddr==0)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected shadowPtr->secondSwapMemoryAddr=0\n");
            }
            tempAddr = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->secondSwapMemoryAddr);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkCopyToOrFromSwapArea1and2 wronf swapAreaIndex- GT_FAIL \n");
        }

        /* reset swapGonsAdresses values */
        cpssOsMemSet(parentUpdateParams->swapGonsAdresses, 0xff, sizeof(parentUpdateParams->swapGonsAdresses));

        if (gonIndex==0xFFFF)
        {
            parentUpdateParams->swapGonsAdresses[0]=tempAddr; /* pointers will be updated to point to swap area */
        }
        else
        {
            /* set in lpmFalconCalcNeededMemory to specify we are moving from compress to regular,
               and the old block is located in index 0 despite the different gonIndex that can be 0-5 */
            if(gonIndex==0xFFFE)
            {
                parentUpdateParams->swapGonsAdresses[0]=tempAddr; /* pointers will be updated to point to swap area */
            }
            else
            {
                parentUpdateParams->swapGonsAdresses[gonIndex]=tempAddr; /* pointers will be updated to point to swap area */
            }
        }
    }
    else
    {
        /* Swap memory is in use , and there is a need to move and rewrite the bucket back to the partition it was */
        tempHwAddrHandle = prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[bankIndex],
                                              DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * oldMemSize,
                                              DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);
        if ((tempHwAddrHandle == DMM_BLOCK_NOT_FOUND)||(tempHwAddrHandle == DMM_MALLOC_FAIL))
        {
            /* should never happen since we just freed this mem when calling
               prvCpssDxChLpmSip6RamMngShrinkBankCopyToSwapArea1AndUpdatePointers*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkCopyToOrFromSwapArea1and2 - GT_FAIL \n");
        }
        else
        {
             /* keep new tempHwAddrHandle -- in case of a fail should be freed */
            needToFreeAllocationInCaseOfFail = GT_TRUE;
            tempHwAddrHandleToBeFreed = tempHwAddrHandle;

           /*  set pending flag for future need */
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
            }
            if (blockIndex!=bankIndex)
            {
                /* we should be working on the bankIndex we got as parameter to the function */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkCopyToOrFromSwapArea1and2 - GT_FAIL \n");
            }
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc = GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandle);
        }

        tempAddr = (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle));

        /* reset swapGonsAdresses values */
        cpssOsMemSet(parentUpdateParams->swapGonsAdresses, 0xff, sizeof(parentUpdateParams->swapGonsAdresses));

        if (gonIndex==0xFFFF)
        {
            parentUpdateParams->swapGonsAdresses[0]=tempAddr; /* pointers will be updated to point to new allocated memory area */
        }
        else
        {
            /* set in lpmFalconCalcNeededMemory to specify we are moving from compress to regular,
               and the old block is located in index 0 despite the different gonIndex that can be 0-5 */
            if(gonIndex==0xFFFE)
            {
                parentUpdateParams->swapGonsAdresses[0]=tempAddr; /* pointers will be updated to point to new allocated memory area */
            }
            else
            {
                parentUpdateParams->swapGonsAdresses[gonIndex]=tempAddr; /* pointers will be updated to point to new allocated memory area */
            }
        }
   }

    retVal = prvCpssDxChLpmSip6RamMngShrinkHwWriteAndPointersUpdatePhase1Phase2(shadowPtr,
                                                                            tempAddr,/* swap area or an allocated one */
                                                                            oldBucketShadowPtr,
                                                                            gonIndex,
                                                                            needToFreeAllocationInCaseOfFail,
                                                                            tempHwAddrHandleToBeFreed,
                                                                            parentUpdateParams,
                                                                            PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE1_E,
                                                                            ucMcType);

    if (retVal!=GT_OK)
    {
       /* in case of a fail any allocations done in previous stages was already
          free in prvCpssDxChLpmSip6RamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkCopyToOrFromSwapArea1and2 - GT_FAIL \n");
    }
    if(copyToFromSwapArea==PRV_CPSS_DXCH_LPM_RAM_COPY_TO_FIRST_SWAP_AREA_E)
    {
        /* ok since the bucket is updated in the HW, we can now update the shadow */
        /* olds code oldBucketShadowPtr->hwBucketOffsetHandle = shadowPtr->swapMemoryAddr;*/
        if (gonIndex==0xFFFF)
        {
            /* first we need to set the old block as a blocked that was moved to a new location */
            SET_DMM_BLOCK_WAS_MOVED(oldBucketShadowPtr->hwBucketOffsetHandle, 1);

            switch(swapAreaIndex)
            {
            case 1:
                oldBucketShadowPtr->hwBucketOffsetHandle = shadowPtr->swapMemoryAddr;
                break;
            case 2:
                oldBucketShadowPtr->hwBucketOffsetHandle = shadowPtr->secondSwapMemoryAddr;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkCopyToOrFromSwapArea1and2 wronf swapAreaIndex- GT_FAIL \n");
            }
        }
        else
        {
            /* set in lpmFalconCalcNeededMemory to specify we are moving from compress to regular,
               and the old block is located in index 0 despite the different gonIndex that can be 0-5 */
            if(gonIndex==0xFFFE)
            {
               /* first we need to set the old block as a blocked that was moved to a new location */
                SET_DMM_BLOCK_WAS_MOVED(oldBucketShadowPtr->hwGroupOffsetHandle[0], 1);

                switch(swapAreaIndex)
                {
                case 1:
                    oldBucketShadowPtr->hwGroupOffsetHandle[0] = shadowPtr->swapMemoryAddr;
                    break;
                case 2:
                    oldBucketShadowPtr->hwGroupOffsetHandle[0] = shadowPtr->secondSwapMemoryAddr;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkCopyToOrFromSwapArea1and2 wronf swapAreaIndex- GT_FAIL \n");
                }
            }
            else
            {
                /* first we need to set the old block as a blocked that was moved to a new location */
                SET_DMM_BLOCK_WAS_MOVED(oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex], 1);

                switch(swapAreaIndex)
                {
                case 1:
                    oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex] = shadowPtr->swapMemoryAddr;
                    break;
                case 2:
                    oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex] = shadowPtr->secondSwapMemoryAddr;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkCopyToOrFromSwapArea1and2 wronf swapAreaIndex- GT_FAIL \n");
                }
            }
        }
    }
    else
    {
        /* ok since the bucket is updated in the HW, we can now update the shadow */
        if (gonIndex==0xFFFF)
        {
            /* first we need to set the old block as a blocked that was moved to a new location */
            SET_DMM_BLOCK_WAS_MOVED(oldBucketShadowPtr->hwBucketOffsetHandle, 1);

            oldBucketShadowPtr->hwBucketOffsetHandle= tempHwAddrHandle;
            SET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwBucketOffsetHandle, oldHwProtocol);
            SET_DMM_BUCKET_SW_ADDRESS(oldBucketShadowPtr->hwBucketOffsetHandle, oldBucketShadowPtr);
        }
        else
        {
            /* set in lpmFalconCalcNeededMemory to specify we are moving from compress to regular,
               and the old block is located in index 0 despite the different gonIndex that can be 0-5 */
            if(gonIndex==0xFFFE)
            {
                /* first we need to set the old block as a blocked that was moved to a new location */
                SET_DMM_BLOCK_WAS_MOVED(oldBucketShadowPtr->hwGroupOffsetHandle[0], 1);

                oldBucketShadowPtr->hwGroupOffsetHandle[0] = tempHwAddrHandle;
                SET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwGroupOffsetHandle[0], oldHwProtocol);
                SET_DMM_BUCKET_SW_ADDRESS(oldBucketShadowPtr->hwGroupOffsetHandle[0], oldBucketShadowPtr);
            }
            else
            {
               /* first we need to set the old block as a blocked that was moved to a new location */
                SET_DMM_BLOCK_WAS_MOVED( oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex], 1);

                oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex] = tempHwAddrHandle;
                SET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex], oldHwProtocol);
                SET_DMM_BUCKET_SW_ADDRESS(oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex], oldBucketShadowPtr);
            }
        }
    }

    retVal = prvCpssDxChLpmSip6RamMngShrinkHwWriteAndPointersUpdatePhase1Phase2(shadowPtr,
                                                                            tempAddr,
                                                                            oldBucketShadowPtr,
                                                                            gonIndex,
                                                                            needToFreeAllocationInCaseOfFail,
                                                                            tempHwAddrHandleToBeFreed,
                                                                            parentUpdateParams,
                                                                            PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE2_E,
                                                                            ucMcType);

    if (retVal!=GT_OK)
    {
       /* in case of a fail any allocations done in previous stages was already
          free in prvCpssDxChLpmSip6RamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkCopyToOrFromSwapArea1and2 - GT_FAIL \n");
    }
    if(copyToFromSwapArea==PRV_CPSS_DXCH_LPM_RAM_COPY_TO_FIRST_SWAP_AREA_E)
    {
       /* now it's ok to free this bucket's old memory */
        retVal = prvCpssDxChLpmSip6RamMngShrinkFreeOldBucket(shadowPtr,
                                                         bankIndex,
                                                         oldHwAddrHandle,
                                                         oldBucketShadowPtr);
        if (retVal!=GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkCopyToOrFromSwapArea1and2 - GT_FAIL \n");
        }
    }
    return GT_OK;
}

/*******************************************************************************
* updateParentSwapGonsAdresses
*
* DESCRIPTION:
*       update parentUpdateParams with the address to copy from/to
*
* INPUTS:
* @param[in] parentUpdateParams      - parameters needed for parent update
* @param[in] address                 - the address we are working on
* @param[in] gonIndex                - the index of the gon we are working on
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK on success, or
*       GT_OUT_OF_CPU_MEM - if failed to allocate CPU memory, or
*       GT_FAIL - otherwise
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS updateParentSwapGonsAdresses
(
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC     *parentUpdateParams,
    IN GT_U32                                    address,
    IN GT_U32                                    gonIndex
)
{
    /* reset swapGonsAdresses values */
    cpssOsMemSet(parentUpdateParams->swapGonsAdresses, 0xff, sizeof(parentUpdateParams->swapGonsAdresses));

    if (gonIndex == 0xFFFF)
    {
        parentUpdateParams->swapGonsAdresses[0]=address;
    }
    else
    {
        /* set in lpmFalconCalcNeededMemory to specify we are moving from compress to regular,
           and the old block is located in index 0 despite the different gonIndex that can be 0-5 */
        if(gonIndex==0xFFFE)
        {
            parentUpdateParams->swapGonsAdresses[0]=address;
        }
        else
        {
            parentUpdateParams->swapGonsAdresses[gonIndex]=address;
        }
    }
    return GT_OK;
}
/**
* @internal prvCpssDxChLpmSip6RamMngShrinkOneBlockUsingSwapArea3 function
* @endinternal
*
* @brief   Shrink one block using swap area 3
*
* @param[in] shadowPtr                - (pointer to) the shadow information
*                                      lpmMemInfoPtr   - (pointer to) the lpm memory linked list holding
*                                      all blocked mapped to this octetIndex
* @param[in] bankIndex                - the index of the bank to shrink
* @param[in] oldBucketShadowPtr       - the old SW bucket handle
* @param[in] oldMemSize               - size of the old bucket in lines
* @param[in] gonIndex                 - the index of the gon we are working on,
*                                       related to the oldBucketShadowPtr
* @param[in]  parentUpdateParams      - parameters needed for parent update
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*
* @note oldMemSize may be 0 in cases we do not have a resize
*       operation but a new bucket in this case no real use of the
*       oldMemSize parameter
*
*/
GT_STATUS prvCpssDxChLpmSip6RamMngShrinkOneBlockUsingSwapArea3
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                             *shadowPtr,
    IN GT_U32                                                       bankIndex,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC                      *oldBucketShadowPtr,
    IN GT_U32                                                       oldMemSize,
    IN GT_U32                                                       gonIndex,
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC                        *parentUpdateParams
)
{
    GT_STATUS                                retVal=GT_OK;
    GT_DMM_PARTITION                         *partition;
    GT_UINTPTR                               oldHwAddrHandle=0;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT     oldHwProtocol;/* the protocol of the bucket we are moving */
    GT_UINTPTR                               tempHwAddrHandle=0;
    GT_BOOL                                  copyToSwapArea3=GT_TRUE;

    GT_U32                                   tempAddr = 0;    /* Temporary address                */
    GT_U32                                   blockIndex=0;

    GT_BOOL     needToFreeAllocationInCaseOfFail = GT_FALSE;
    GT_UINTPTR  tempHwAddrHandleToBeFreed = 0;
    PRV_CPSS_DXCH_LPM_ALLOC_TYPE_ENT         ucMcType=PRV_CPSS_DXCH_LPM_ALLOC_UC_TYPE_E;/* indicates whether bucketPtr is the uc,
                                                                                           mc src tree, mc group bucket */

    partition = (GT_DMM_PARTITION *)shadowPtr->lpmRamStructsMemPoolPtr[bankIndex];
    /* if the partition is empty --> the block is empty */
    if(partition->allocatedBytes == 0)
    {
        /* nothing to shrink
           should never happen since this function should be called after
           checking shrink operation may be useful for defrag */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkOneBlockUsingSwapArea3 - GT_FAIL \n");
    }

    /* oldBucketShadowPtr will be used to build the HW from SW shadow and for
       updated pointers when we move the HW bucket location */
    if (gonIndex==0xFFFF)
    {
        oldHwAddrHandle     = oldBucketShadowPtr->hwBucketOffsetHandle;
        oldHwProtocol       = GET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwBucketOffsetHandle);
        if (oldBucketShadowPtr->pointingRangeMemAddr!=0xFFFFFFFF)
        {
             /* this is SRC Root
               all other cases are treated same as UC bucket */
            ucMcType = PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E;
        }
    }
    else
    {
        /* set in lpmFalconCalcNeededMemory to specify we are moving from compress to regular,
           and the old block is located in index 0 despite the different gonIndex that can be 0-5 */
        if(gonIndex==0xFFFE)
        {
            oldHwAddrHandle     = oldBucketShadowPtr->hwGroupOffsetHandle[0];
            oldHwProtocol       = GET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwGroupOffsetHandle[0]);
        }
        else
        {
            oldHwAddrHandle     = oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex];
            oldHwProtocol       = GET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex]);
        }
    }

    /* we will loop the code twice
       one for copying the bucket to swap_area_3 and one for copying it back to the partition */
    while (GT_TRUE)
    {
        if (copyToSwapArea3 == GT_TRUE)
        {
            if(shadowPtr->thirdSwapMemoryAddr==0)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected shadowPtr->thirdSwapMemoryAddr=0\n");
            }
            /* create the HW data to be writen to swap_area_3 according to the oldBucketPtr */
            tempAddr = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->thirdSwapMemoryAddr);

            retVal = updateParentSwapGonsAdresses(parentUpdateParams,tempAddr,gonIndex);/* pointers will be updated to point to swap area */
            if (retVal!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"error in prvCpssDxChLpmRamMngShrinkOneBlockUsingSwapArea3 - updateSwapGonsAdresses - GT_FAIL \n");
            }
        }
        else
        {
            /* Swap memory is in use , and there is a need to move and rewrite the bucket back to the partition it was */
            tempHwAddrHandle = prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[bankIndex],
                                                  DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * oldMemSize,
                                                  DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);
            if ((tempHwAddrHandle == DMM_BLOCK_NOT_FOUND)||(tempHwAddrHandle == DMM_MALLOC_FAIL))
            {
                /* should never happen since we just freed this mem when calling
                   prvCpssDxChLpmRamMngShrinkOneBlockUsingSwapArea3 */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkOneBlockUsingSwapArea3 - GT_FAIL \n");
            }
            else
            {
                /* keep new tempHwAddrHandle -- in case of a fail should be freed */
                needToFreeAllocationInCaseOfFail = GT_TRUE;
                tempHwAddrHandleToBeFreed = tempHwAddrHandle;

               /*  set pending flag for future need */
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                if (retVal != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                }
                if (blockIndex!=bankIndex)
                {
                    /* we should be working on the bankIndex we got as parameter to the function */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkOneBlockUsingSwapArea2 - GT_FAIL \n");
                }
                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc = GT_TRUE;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandle);
            }

            tempAddr = (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle));

            retVal = updateParentSwapGonsAdresses(parentUpdateParams,tempAddr,gonIndex);/* pointers will be updated to point to new allocated memory area */
            if (retVal!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"error in prvCpssDxChLpmRamMngShrinkOneBlockUsingSwapArea3 - updateSwapGonsAdresses - GT_FAIL \n");
            }
       }

       retVal = prvCpssDxChLpmSip6RamMngShrinkHwWriteAndPointersUpdatePhase1Phase2(shadowPtr,
                                                                            tempAddr,/* third swap area or an allocated one */
                                                                            oldBucketShadowPtr,
                                                                            gonIndex,
                                                                            needToFreeAllocationInCaseOfFail,
                                                                            tempHwAddrHandleToBeFreed,
                                                                            parentUpdateParams,
                                                                            PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE1_E,
                                                                            ucMcType);

        if (retVal!=GT_OK)
        {
           /* in case of a fail any allocations done in previous stages was already
              free in prvCpssDxChLpmSip6RamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkOneBlockUsingSwapArea3 - GT_FAIL \n");
        }
        if(copyToSwapArea3==GT_TRUE)
        {
            /* ok since the bucket is updated in the HW, we can now update the shadow */
            if (gonIndex==0xFFFF)
            {
                /* first we need to set the old block as a blocked that was moved to a new location */
                SET_DMM_BLOCK_WAS_MOVED(oldBucketShadowPtr->hwBucketOffsetHandle, 1);

                oldBucketShadowPtr->hwBucketOffsetHandle = shadowPtr->thirdSwapMemoryAddr;
            }
            else
            {
                 /* set in lpmFalconCalcNeededMemory to specify we are moving from compress to regular,
                   and the old block is located in index 0 despite the different gonIndex that can be 0-5 */
                if(gonIndex==0xFFFE)
                {
                    /* first we need to set the old block as a blocked that was moved to a new location */
                    SET_DMM_BLOCK_WAS_MOVED(oldBucketShadowPtr->hwGroupOffsetHandle[0], 1);

                    oldBucketShadowPtr->hwGroupOffsetHandle[0] = shadowPtr->thirdSwapMemoryAddr;
                }
                else
                {
                    /* first we need to set the old block as a blocked that was moved to a new location */
                    SET_DMM_BLOCK_WAS_MOVED(oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex], 1);

                    oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex] = shadowPtr->thirdSwapMemoryAddr;
                }
            }
        }
        else
        {
           /* ok since the bucket is updated in the HW, we can now update the shadow */
            if (gonIndex==0xFFFF)
            {
                /* first we need to set the old block as a blocked that was moved to a new location */
                SET_DMM_BLOCK_WAS_MOVED(oldBucketShadowPtr->hwBucketOffsetHandle, 1);

                oldBucketShadowPtr->hwBucketOffsetHandle = tempHwAddrHandle;
                SET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwBucketOffsetHandle, oldHwProtocol);
                SET_DMM_BUCKET_SW_ADDRESS(oldBucketShadowPtr->hwBucketOffsetHandle, oldBucketShadowPtr);
            }
            else
            {
                /* set in lpmFalconCalcNeededMemory to specify we are moving from compress to regular,
                   and the old block is located in index 0 despite the different gonIndex that can be 0-5 */
                if(gonIndex==0xFFFE)
                {
                    /* first we need to set the old block as a blocked that was moved to a new location */
                    SET_DMM_BLOCK_WAS_MOVED(oldBucketShadowPtr->hwGroupOffsetHandle[0], 1);

                    oldBucketShadowPtr->hwGroupOffsetHandle[0] = tempHwAddrHandle;
                    SET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwGroupOffsetHandle[0], oldHwProtocol);
                    SET_DMM_BUCKET_SW_ADDRESS(oldBucketShadowPtr->hwGroupOffsetHandle[0], oldBucketShadowPtr);
                }
                else
                {
                    /* first we need to set the old block as a blocked that was moved to a new location */
                    SET_DMM_BLOCK_WAS_MOVED(oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex], 1);

                    oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex] = tempHwAddrHandle;
                    SET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex], oldHwProtocol);
                    SET_DMM_BUCKET_SW_ADDRESS(oldBucketShadowPtr->hwGroupOffsetHandle[gonIndex], oldBucketShadowPtr);
                }
            }
        }

        retVal = prvCpssDxChLpmSip6RamMngShrinkPointersUpdate(shadowPtr,
                                                              tempAddr,/* third swap area or an allocated one */
                                                              oldBucketShadowPtr,
                                                              gonIndex,
                                                              needToFreeAllocationInCaseOfFail,
                                                              tempHwAddrHandleToBeFreed);
        if (retVal!=GT_OK)
        {
           /* in case of a fail any allocations done in previous stages was already
              free in prvCpssDxChLpmSip6RamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkOneBlockUsingSwapArea3 - GT_FAIL \n");
        }
        if(copyToSwapArea3==GT_TRUE)
        {
            /* now it's ok to free this bucket's old memory */
            retVal = prvCpssDxChLpmSip6RamMngShrinkFreeOldBucket(shadowPtr,
                                                             bankIndex,
                                                             oldHwAddrHandle,
                                                             oldBucketShadowPtr);
            if (retVal!=GT_OK)
            {
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkOneBlockUsingSwapArea3 - GT_FAIL \n");
            }
        }

        if (copyToSwapArea3==GT_TRUE)
        {
            copyToSwapArea3 = GT_FALSE;
        }
        else
        {
            /* finish to shrink one block element */
            break;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmSip6RamMngShrinkOneBlock function
* @endinternal
*
* @brief   Shrink one block element in the bank
*
* @param[in] shadowPtr                - (pointer to) the shadow information
*                                      lpmMemInfoPtr   - (pointer to) the lpm memory linked list holding
*                                      all blocked mapped to this octetIndex
* @param[in] bankIndex                - the index of the bank to shrink
* @param[in] oldBucketShadowPtr       - the old SW bucket handle
* @param[in] oldMemSize               - size of the old bucket in lines
* @param[in] oldMemGonIndex           - the GON index we need to move 0-5 or
*                                       0xFFFF in case of Root
* @param[in] parentUpdateParams       - parameters needed for parent update
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmSip6RamMngShrinkOneBlock
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                             *shadowPtr,
    IN GT_U32                                                       bankIndex,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC                      *oldBucketShadowPtr,
    IN GT_U32                                                       oldMemSize,
    IN GT_U32                                                       oldMemGonIndex,
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC                        *parentUpdateParams
)
{
    GT_STATUS                                retVal=GT_OK;
    GT_DMM_PARTITION                         *partition;
    GT_UINTPTR                               oldHwAddrHandle=0;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT     oldHwProtocol;/* the protocol of the bucket we are moving */
    GT_UINTPTR                               tempHwAddrHandle=0;

    GT_U32                                   tempAddr = 0;     /* Temporary address                */
    GT_U32                                   blockIndex=0;
    GT_U32                                   newLocationBlockSize;

    GT_BOOL     needToFreeAllocationInCaseOfFail = GT_FALSE;
    GT_UINTPTR  tempHwAddrHandleToBeFreed = 0;

    PRV_CPSS_DXCH_LPM_ALLOC_TYPE_ENT         ucMcType=PRV_CPSS_DXCH_LPM_ALLOC_UC_TYPE_E;/* indicates whether bucketPtr is the uc,
                                                                                           mc src tree, mc group bucket */

    partition = (GT_DMM_PARTITION *)shadowPtr->lpmRamStructsMemPoolPtr[bankIndex];
    /* if the partition is empty --> the block is empty */
    if(partition->allocatedBytes == 0)
    {
        /* nothing to shrink
           should never happen since this function should be called after
           checking shrink operation may be useful for defrag */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkOneBlock - GT_FAIL \n");
    }

    /* oldBucketPtr will be used to build the HW from SW shadow and for
       updated pointers when we move the HW bucket location */
    if (oldMemGonIndex==0xFFFF)
    {
        oldHwAddrHandle     = oldBucketShadowPtr->hwBucketOffsetHandle;
        oldHwProtocol       = GET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwBucketOffsetHandle);

        if (oldBucketShadowPtr->pointingRangeMemAddr!=0xFFFFFFFF)
        {
             /* this is SRC Root
               all other cases are treated same as UC bucket */
            ucMcType = PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E;
        }
    }
    else
    {
        oldHwAddrHandle     = oldBucketShadowPtr->hwGroupOffsetHandle[oldMemGonIndex];
        oldHwProtocol       = GET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwGroupOffsetHandle[oldMemGonIndex]);
    }

    /* double check that the element we want to move have a free space it will fit
       before its current location */
    if (((GT_DMM_BLOCK *)oldHwAddrHandle)->prevByAddr != NULL)
    {
        newLocationBlockSize =  SIZE_IN_WORDS(((GT_DMM_BLOCK *)oldHwAddrHandle)->prevByAddr);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkOneBlock - GT_FAIL \n");
    }

    if((DMM_BLOCK_STATUS(((GT_DMM_BLOCK *)oldHwAddrHandle)->prevByAddr) == DMM_BLOCK_FREE) &&
       ((OFFSET_IN_WORDS(((GT_DMM_BLOCK *)oldHwAddrHandle)->prevByAddr) + newLocationBlockSize) <=
         OFFSET_IN_WORDS(oldHwAddrHandle)) &&
       (oldMemSize<=newLocationBlockSize))
    {

        retVal = prvCpssDmmAllocateByPtr(shadowPtr->lpmRamStructsMemPoolPtr[bankIndex],
                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS*(OFFSET_IN_WORDS(((GT_DMM_BLOCK *)oldHwAddrHandle)->prevByAddr)),
                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS* oldMemSize,
                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS,
                                 &tempHwAddrHandle);

        if ((tempHwAddrHandle == DMM_BLOCK_NOT_FOUND)||(tempHwAddrHandle == DMM_MALLOC_FAIL))
        {
            /* should never happen since we just freed this mem when calling
               prvCpssDxChLpmSip6RamMngShrinkCopyToOrFromSwapArea1and2 */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkOneBlock - GT_FAIL \n");
        }
        else
        {
            /* keep new tempHwAddrHandle -- in case of a fail should be freed */
            needToFreeAllocationInCaseOfFail = GT_TRUE;
            tempHwAddrHandleToBeFreed = tempHwAddrHandle;

           /*  set pending flag for future need */
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
            }
            if (blockIndex!=bankIndex)
            {
                /* we should be working on the bankIndex we got as parameter to the function */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkOneBlock - GT_FAIL \n");
            }
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc = GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandle);
        }

        tempAddr = (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle));

        retVal = updateParentSwapGonsAdresses(parentUpdateParams,tempAddr,oldMemGonIndex);/* pointers will be updated to point to new allocated memory area */
        if (retVal!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"error in prvCpssDxChLpmRamMngShrinkOneBlockUsingSwapArea3 - updateSwapGonsAdresses - GT_FAIL \n");
        }

        retVal = prvCpssDxChLpmSip6RamMngShrinkHwWriteAndPointersUpdatePhase1Phase2(shadowPtr,
                                                                                    tempAddr,
                                                                                    oldBucketShadowPtr,
                                                                                    oldMemGonIndex,
                                                                                    needToFreeAllocationInCaseOfFail,
                                                                                    tempHwAddrHandleToBeFreed,
                                                                                    parentUpdateParams,
                                                                                    PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE1_E,
                                                                                    ucMcType);

        if (retVal!=GT_OK)
        {
           /* in case of a fail any allocations done in previous stages was already
              free in prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkOneBlock - GT_FAIL \n");
        }

        /* ok since the bucket is updated in the HW, we can now update the shadow */
        if (oldMemGonIndex==0xFFFF)
        {
            /* first we need to set the old block as a blocked that was moved to a new location */
            SET_DMM_BLOCK_WAS_MOVED(oldBucketShadowPtr->hwBucketOffsetHandle, 1);
            oldBucketShadowPtr->hwBucketOffsetHandle = tempHwAddrHandle;

            SET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwBucketOffsetHandle, oldHwProtocol);
            SET_DMM_BUCKET_SW_ADDRESS(oldBucketShadowPtr->hwBucketOffsetHandle, oldBucketShadowPtr);
        }
        else
        {
            /* first we need to set the old block as a blocked that was moved to a new location */
            SET_DMM_BLOCK_WAS_MOVED(oldBucketShadowPtr->hwGroupOffsetHandle[oldMemGonIndex], 1);

            oldBucketShadowPtr->hwGroupOffsetHandle[oldMemGonIndex] = tempHwAddrHandle;

            SET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwGroupOffsetHandle[oldMemGonIndex], oldHwProtocol);
            SET_DMM_BUCKET_SW_ADDRESS(oldBucketShadowPtr->hwGroupOffsetHandle[oldMemGonIndex], oldBucketShadowPtr);
        }

        retVal = prvCpssDxChLpmSip6RamMngShrinkPointersUpdate(shadowPtr,
                                                              tempAddr,
                                                              oldBucketShadowPtr,
                                                              oldMemGonIndex,
                                                              needToFreeAllocationInCaseOfFail,
                                                              tempHwAddrHandleToBeFreed);
        if (retVal!=GT_OK)
        {
           /* in case of a fail any allocations done in previous stages was already
              free in prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkOneBlock - GT_FAIL \n");
        }

        /* now it's ok to free this bucket's old memory */
        retVal = prvCpssDxChLpmSip6RamMngShrinkFreeOldBucket(shadowPtr,
                                                         bankIndex,
                                                         oldHwAddrHandle,
                                                         oldBucketShadowPtr);
        if (retVal!=GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkOneBlock - GT_FAIL \n");
        }
    }
    else
    {
        /* nothing to shrink
           should never happen since this function should be called after
           checking shrink operation may be useful for defrag */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkOneBlock - GT_FAIL \n");
    }

    return GT_OK;
}

/*******************************************************************************
* findBlockSizeAndIndexToMove
*
* DESCRIPTION:
*       for a given block return the size and the gon index
*
* INPUTS:
*       blockToMove              - (pointer to) a block memory to move
*
* OUTPUTS:
*       oldMemSizeToMovePtr      - (pointer to) size of the block to move
*       oldMemGonIndexToMovePtr  - (pointer to) gon Index of the block to move
*
* RETURNS:
*       GT_OK on success, or
*       GT_OUT_OF_CPU_MEM - if failed to allocate CPU memory, or
*       GT_FAIL - otherwise
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS findBlockSizeAndIndexToMove
(
    IN  GT_DMM_BLOCK                             *blockToMove,
    OUT GT_U32                                   *oldMemSizeToMovePtr,
    OUT GT_U32                                   *oldMemGonIndexToMovePtr
)
{
    GT_U32 j=0;

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *oldBucketShadowPtr=NULL;

    if (blockToMove==0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected blockToMove=0\n");
    }

    oldBucketShadowPtr = (PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *)GET_DMM_BUCKET_SW_ADDRESS(blockToMove); /* old bucket representation*/

    /* in SIP6 we can have 6 GONS and a node that will give us the same SW bucket representation
    so we need to find what is the specific GON we are about to move */
    if (oldBucketShadowPtr!=0)
    {
        if (oldBucketShadowPtr->hwBucketOffsetHandle!=0)/* we might be moving a Root or a SRC Root node*/
        {
            if (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(oldBucketShadowPtr->hwBucketOffsetHandle)==
                PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(blockToMove))
            {
                /* we are moving the root */
                *oldMemSizeToMovePtr         = SIZE_IN_WORDS(blockToMove);
                *oldMemGonIndexToMovePtr     = 0xFFFF;
            }
        }
        if (*oldMemSizeToMovePtr==0)/* continue looking */
        {
            /* find the correct GON to move */
            for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
            {
                if ((blockToMove==0)||(oldBucketShadowPtr->hwGroupOffsetHandle[j]==0))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected blockToMove=0 or oldBucketShadowPtr->hwGroupOffsetHandle[j]=0\n");
                }
                if (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(oldBucketShadowPtr->hwGroupOffsetHandle[j])==
                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(blockToMove))
                {
                    *oldMemSizeToMovePtr         = SIZE_IN_WORDS(blockToMove);
                    *oldMemGonIndexToMovePtr     = j;
                    /* we found the correct GON address */
                    break;
                }
                if (oldBucketShadowPtr->bucketType!=CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                {
                    /* this is not a regular node -> only one Gon index */
                    break;
                }
            }
        }
        if (*oldMemSizeToMovePtr==0)
        {
            /* we should never get here - specify some logic error */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in findBlockSizeAndIndexToMove oldMemSizeToMove==0 - GT_FAIL \n");
        }
    }
    else
    {
        /* this case can happen in case we are working on group prefix
           stage 1 - source prefix memory allocation was done but not updated in HW yet
           stage 2 - group prefix memory allocation need to move the source prefix from stage 1,
           this case should be prevented - no move should be done

           another case can happen when working with 6 gons in regular bucket.
           first gon is allocated correctly from block x,
           then for the second gon we need to merge block y into block x,
           but we first need to shrink block x. in this case if we try to move
           the allocted space taken by the first gon from block x - we should fail.
           this is a case that should be prevented */
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM,"error in findBlockSizeAndIndexToMove - GT_OUT_OF_PP_MEM \n");
    }

    return GT_OK;
}

/*******************************************************************************
* moveDataFromSwapAreaBackToTheOriginalBlock
*
* DESCRIPTION:
*       move back blocks put in swap memory to their original block
*
* INPUTS:
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] octetIndex               - the octet we are working on
* @param[in] bankIndex                - the index of the bank to shrink
* @param[in] oldBucketShadowPtr       - the old SW bucket handle
* @param[in] oldMemSizeArr            - size of the old bucket in lines
* @param[in] oldHwHandleAddrArr       - the old HW bucket handle, if the
*                                      value is 0, it is a new bucket, no
*                                      resize, up to 2 handles for a single add
*                                      prefix
* @param[in] oldHwHandleGonIndexArr   - the index of the gon we are working on,
*                                       related to the oldBucketShadowPtr
* @param[in] oldBucketShouldBeMovedArr- flag to specify if we need to move the old bucket
* @param[in] useSwapAreaArr           - GT_TRUE  - swapArea will be used when
*                                      doing shrink
*                                      GT_FALSE - swapArea will NOT be used when
*                                      doing shrink
*                                      up to 2 swap areas used for a single add
*                                      prefix
* @param[in] parentUpdateParams       - parameters needed for parent update
*
* OUTPUTS:

*
* RETURNS:
*       GT_OK on success, or
*       GT_OUT_OF_CPU_MEM - if failed to allocate CPU memory, or
*       GT_FAIL - otherwise
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS moveDataFromSwapAreaBackToTheOriginalBlock
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                             *shadowPtr,
    IN GT_U32                                                       bankIndex,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC                      *oldBucketShadowPtr[],
    IN GT_U32                                                       oldMemSizeArr[],
    IN GT_UINTPTR                                                   oldHwHandleAddrArr[],
    IN GT_U32                                                       oldHwHandleGonIndexArr[],
    IN GT_BOOL                                                      oldBucketShouldBeMovedArr[],
    IN GT_BOOL                                                      useSwapAreaArr[],
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC                        *parentUpdateParams
)
{
    GT_STATUS retVal= GT_OK;
    GT_U32 i=0;

    /* up to 2 swap areas used */
    for (i=0;i<2;i++)
    {
        /* if useSwapArea is GT_TRUE this means we need to copy old bucket
           from swap_area_1/swap_area_2 to its original partition */
        if ((useSwapAreaArr[i]==GT_TRUE)&&(oldBucketShouldBeMovedArr[i]==GT_TRUE))
        {
            retVal = prvCpssDxChLpmSip6RamMngShrinkCopyToOrFromSwapArea1and2(shadowPtr,
                                                                    bankIndex,
                                                                    oldBucketShadowPtr[i],
                                                                    oldMemSizeArr[i],
                                                                    oldHwHandleGonIndexArr[i],
                                                                    PRV_CPSS_DXCH_LPM_RAM_COPY_FROM_FIRST_SWAP_AREA_E,
                                                                    (i+1),/* swap area index, 1 or 2 */
                                                                    parentUpdateParams);
            if (retVal!=GT_OK)
            {
                /* should never happen since swap area is a free memory that can be used */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in moveDataFromSwapAreaBackToTheOriginalBlock - GT_FAIL \n");
            }
            /* the old memory was moved to a new location, need to update arrays to be used when getting out of this function */
            if (oldHwHandleGonIndexArr[i]==0xFFFF)
            {
                oldHwHandleAddrArr[i] = oldBucketShadowPtr[i]->hwBucketOffsetHandle;
            }
            else
            {
                 /* set in lpmFalconCalcNeededMemory to specify we are moving from compress to regular,
                   and the old block is located in index 0 despite the different gonIndex that can be 0-5 */
                if(oldHwHandleGonIndexArr[i]==0xFFFE)
                {
                    oldHwHandleAddrArr[i] = oldBucketShadowPtr[i]->hwGroupOffsetHandle[0];
                }
                else
                {
                    oldHwHandleAddrArr[i] = oldBucketShadowPtr[i]->hwGroupOffsetHandle[oldHwHandleGonIndexArr[i]];
                }
            }
        }
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmSip6RamMngShrinkBank function
* @endinternal
*
* @brief   Shrink the partition
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] bankIndex                - the index of the bank to shrink
* @param[in] oldHwHandleAddrArr       - the old HW bucket handle, if the
*                                      value is 0, it is a new bucket, no
*                                      resize, up to 2 handles for a single add
*                                      prefix
* @param[in] oldHwHandleGonIndexArr   - the gonIndex of the element in
*                                       oldHwHandleAddrArr
* @param[in] newBucketSize            - the size (in lines) of the new bucket
* @param[in] useSwapAreaArr           - GT_TRUE  - swapArea will be used when
*                                      doing shrink
*                                      GT_FALSE - swapArea will NOT be used when
*                                      doing shrink
*                                      up to 2 swap areas used for a single add
*                                      prefix
* @param[in]  parentUpdateParams      - parameters needed for parent update
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmSip6RamMngShrinkBank
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                             *shadowPtr,
    IN GT_U32                                                       bankIndex,
    IN GT_UINTPTR                                                   oldHwHandleAddrArr[],
    IN GT_U32                                                       oldHwHandleGonIndexArr[],
    IN GT_U32                                                       newBucketSize,
    IN GT_BOOL                                                      useSwapAreaArr[],
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC                        *parentUpdateParams
)
{
    GT_STATUS           retVal = GT_OK, retVal2 = GT_OK;
    GT_U32              i=0;
    GT_DMM_PARTITION    *partition;

    GT_DMM_BLOCK        *firstFreeBlock;
    GT_DMM_BLOCK        *firstOccupiedBlockAfterFirstFreeBlock;

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *oldBucketShadowPtr[2]={NULL};
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *oldBucketShadowToMovePtr=NULL;
    GT_U32                                   oldMemSizeArr[2]={0};
    GT_BOOL                                  oldBucketShouldBeMovedArr[2]={GT_FALSE};/* flag to specify if we need to move the old bucket */

    GT_U32 oldMemSizeToMove=0;     /* the size of the GON we need to move */
    GT_U32 oldMemGonIndexToMove=0; /* the index of the GON we need to move: 0-5 or 0xFFFF in case of Root*/

    /* we can have up to 2 old handles to be moved in the shrink operation */
    /* oldBucketShadowPtr will be used to build the HW from SW shadow and for
       updated pointers when we move the HW bucket location */
    for (i = 0; i < 2; i++)
    {
        if (oldHwHandleAddrArr[i]!=0)
        {
            oldBucketShadowPtr[i] = (PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *)GET_DMM_BUCKET_SW_ADDRESS(oldHwHandleAddrArr[i]);/* old bucket representation*/
            if(oldHwHandleGonIndexArr[i]==0xFFFF)/* set in lpmFalconCalcNeededMemory to specify the old  block is of a SRC root*/
            {
                oldMemSizeArr[i]         = SIZE_IN_WORDS(oldBucketShadowPtr[i]->hwBucketOffsetHandle)
                                                                    /DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS;
            }
            else
            {
                /* set in lpmFalconCalcNeededMemory to specify we are moving from compress to regular,
                   and the old block is located in index 0 despite the different gonIndex that can be 0-5 */
                if(oldHwHandleGonIndexArr[i]==0xFFFE)
                {
                    oldMemSizeArr[i]         = SIZE_IN_WORDS(oldBucketShadowPtr[i]->hwGroupOffsetHandle[0])
                                                                    /DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS;
                }
                else
                {
                    oldMemSizeArr[i]         = SIZE_IN_WORDS(oldBucketShadowPtr[i]->hwGroupOffsetHandle[oldHwHandleGonIndexArr[i]])
                                                                    /DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS;
                }
            }

            oldBucketShouldBeMovedArr[i]=GT_TRUE;
        }
        else
        {
            /* we need to do shrink for a new block, so no old pointers to update
               and no old memory location to be moved */
            oldBucketShouldBeMovedArr[i]=GT_FALSE;
        }
    }

    partition = (GT_DMM_PARTITION *)shadowPtr->lpmRamStructsMemPoolPtr[bankIndex];
    /* if the partition is empty --> the block is empty */
    if(partition->allocatedBytes == 0)
    {
        /* nothing to shrink
           should never happen since this function should be called after
           checking shrink operation may be useful for defrag */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkBank - GT_FAIL \n");
    }

    /* up to 2 swap areas used */
    for (i=0;i<2;i++)
    {
        /* if useSwapArea is GT_TRUE this means we need to copy old bucket
           to swap_area_1/swap_area_2, since we need its space for the shrink operation */
        if ((useSwapAreaArr[i]==GT_TRUE)&&(oldBucketShouldBeMovedArr[i]==GT_TRUE))
        {
            /* 1. copy oldHwHandleAddr[i] to the swap_area_1/swap_area_2 */
            /* 2. update pointers */
            /* 3. free space of oldHwHandleAddr[i] */

            retVal = prvCpssDxChLpmSip6RamMngShrinkCopyToOrFromSwapArea1and2(shadowPtr,
                                                                  bankIndex,
                                                                  oldBucketShadowPtr[i],
                                                                  oldMemSizeArr[i],
                                                                  oldHwHandleGonIndexArr[i],
                                                                  PRV_CPSS_DXCH_LPM_RAM_COPY_TO_FIRST_SWAP_AREA_E,
                                                                  (i+1),/* swap index 1 or 2 */
                                                                  parentUpdateParams);

            if (retVal!=GT_OK)
            {
                /* should never happen since swap area is a free memory that can be used */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkBank - GT_FAIL \n");
            }

            /* 4. continue to next steps of shrink */
        }
    }

    /* find the first free block in the partition */
    firstFreeBlock = partition->pointedFirstFreeBlock;
    if((firstFreeBlock == NULL)||(DMM_BLOCK_STATUS(firstFreeBlock)!=DMM_BLOCK_FREE))
    {
         /* nothing to shrink
            should never happen since this function should be called after
            checking shrink operation may be useful for defrag */

        /* revert what was done above

           if useSwapArea is GT_TRUE this means we need to copy old bucket
           from swap_area_1/swap_area_2 to its original partition */

       retVal = moveDataFromSwapAreaBackToTheOriginalBlock(shadowPtr,
                                                       bankIndex,
                                                       oldBucketShadowPtr,
                                                       oldMemSizeArr,
                                                       oldHwHandleAddrArr,
                                                       oldHwHandleGonIndexArr,
                                                       oldBucketShouldBeMovedArr,
                                                       useSwapAreaArr,
                                                       parentUpdateParams);
        if (retVal!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"error in moveDataFromSwapAreaBackToTheOriginalBlock - GT_FAIL \n");
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkBank - GT_FAIL \n");
        }
    }

    /* go over the list and copy allocated block to the free block
       this will shrink all occupied blocks down and all free blocks up */
    while(GT_TRUE)
    {
        /* check if now we have big enough free space to add new bucket */
        if (newBucketSize<=SIZE_IN_WORDS(firstFreeBlock))
        {
            /* we found a free space to fit newBucketSize -- stop the shrink operation */

            /* we shrink the bank but the new allocation of space should be done
               when getting out of the function
               1. write new bucket to head of free space
               2. update pointers
               3. update free list */
            break;
        }

        /* after a free block we will always have an occupied block,
           otherwise there is a merged between 2 free blocks */
        firstOccupiedBlockAfterFirstFreeBlock = firstFreeBlock->nextByAddr;
        if (firstOccupiedBlockAfterFirstFreeBlock==NULL)
        {
            /* if no next bucket, bank is fully shrinked and failed to add a bucket.
               this is considered error as we previously verified bank has enough space */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkBank - GT_FAIL \n");
        }

        /* reset OUT parameters */
        oldMemSizeToMove = 0;
        oldMemGonIndexToMove=0;

        /* if the size of the block we want to move is smaller or equal to the free block we have,
           then there is no problem to move the block and update the pointers */
        if (SIZE_IN_WORDS(firstOccupiedBlockAfterFirstFreeBlock)<=SIZE_IN_WORDS(firstFreeBlock))
        {
            retVal = findBlockSizeAndIndexToMove(firstOccupiedBlockAfterFirstFreeBlock,&oldMemSizeToMove,&oldMemGonIndexToMove);
            if (retVal!=GT_OK)
            {
                retVal2 = retVal;
                /* this case can happen in case we are working on group prefix
                   stage 1 - source prefix memory allocation was done but not updated in HW yet
                   stage 2 - group prefix memory allocation need to move the source prefix from stage 1,
                   this case should be prevented - no move should be done
                   need to return the data put in the swap back to its old block */
                if (retVal2==GT_OUT_OF_PP_MEM)
                {
                    retVal = moveDataFromSwapAreaBackToTheOriginalBlock(shadowPtr,
                                                                    bankIndex,
                                                                    oldBucketShadowPtr,
                                                                    oldMemSizeArr,
                                                                    oldHwHandleAddrArr,
                                                                    oldHwHandleGonIndexArr,
                                                                    oldBucketShouldBeMovedArr,
                                                                    useSwapAreaArr,
                                                                    parentUpdateParams);

                    if (retVal!=GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"error in moveDataFromSwapAreaBackToTheOriginalBlock - GT_FAIL \n");
                    }
                    else
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal2,"error in prvCpssDxChLpmSip6RamMngShrinkBank call to findGonSizeAndIndexToMove - GT_FAIL \n");
                    }
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(retVal2, "error in prvCpssDxChLpmSip6RamMngShrinkBank call to findGonSizeAndIndexToMove - GT_FAIL \n");
                }
            }

            oldBucketShadowToMovePtr = (PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *)GET_DMM_BUCKET_SW_ADDRESS(firstOccupiedBlockAfterFirstFreeBlock);
            retVal = prvCpssDxChLpmSip6RamMngShrinkOneBlock(shadowPtr,
                                                          bankIndex,
                                                          oldBucketShadowToMovePtr,
                                                          oldMemSizeToMove,
                                                          oldMemGonIndexToMove,
                                                          parentUpdateParams);

            if (retVal!=GT_OK)
            {
                /* should never happen since swap area is a free memory that can be used */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkBank - GT_FAIL \n");
            }
        }
        else
        {
            /* we need to copy the firstOccupiedBlock to swap_area_3
               to prevent the bucket from overwrite itself  */

            /* 1. copy firstOccupiedBlockAfterFirstFreeBlock to swap_area_3 */
            /* 2. update pointers to swap_area_3 */
            /* 3. free firstOccupiedBlockAfterFirstFreeBlock original location,
                  the new freed memory will be merged with firstFreeBlock  */
            /* 4. write firstOccupiedBlockAfterFirstFreeBlock to firstFreeBlock */
            /* 5. update the pointers to the new location */

            /* reset OUT parameters */
            oldMemSizeToMove = 0;
            oldMemGonIndexToMove=0;
            retVal = findBlockSizeAndIndexToMove(firstOccupiedBlockAfterFirstFreeBlock,&oldMemSizeToMove,&oldMemGonIndexToMove);
            if (retVal!=GT_OK)
            {
                retVal2 = retVal;
                /* this case can happen in case we are working on group prefix
                   stage 1 - source prefix memory allocation was done but not updated in HW yet
                   stage 2 - group prefix memory allocation need to move the source prefix from stage 1,
                   this case should be prevented - no move should be done
                   need to return the data put in the swap back to its old block */
                if (retVal==GT_OUT_OF_PP_MEM)
                {
                    retVal = moveDataFromSwapAreaBackToTheOriginalBlock(shadowPtr,
                                                                    bankIndex,
                                                                    oldBucketShadowPtr,
                                                                    oldMemSizeArr,
                                                                    oldHwHandleAddrArr,
                                                                    oldHwHandleGonIndexArr,
                                                                    oldBucketShouldBeMovedArr,
                                                                    useSwapAreaArr,
                                                                    parentUpdateParams);
                    if (retVal!=GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"error in moveDataFromSwapAreaBackToTheOriginalBlock - GT_FAIL \n");
                    }
                    else
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal2,"error in prvCpssDxChLpmSip6RamMngShrinkBank call to findGonSizeAndIndexToMove - GT_FAIL \n");
                    }
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(retVal2,"error in prvCpssDxChLpmSip6RamMngShrinkBank call to findGonSizeAndIndexToMove - GT_FAIL \n");
                }
            }

            oldBucketShadowToMovePtr = (PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *)GET_DMM_BUCKET_SW_ADDRESS(firstOccupiedBlockAfterFirstFreeBlock);
            retVal = prvCpssDxChLpmSip6RamMngShrinkOneBlockUsingSwapArea3(shadowPtr,
                                                                          bankIndex,
                                                                          oldBucketShadowToMovePtr,
                                                                          oldMemSizeToMove,
                                                                          oldMemGonIndexToMove,
                                                                          parentUpdateParams);
            if (retVal!=GT_OK)
            {
                /* should never happen since swap area is a free memory that can be used */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkBank - GT_FAIL \n");
            }
        }

         /* find the new first free block in the partition */
         firstFreeBlock = partition->pointedFirstFreeBlock;
    }

    /* up to 2 swap areas used  --> move it back to its original block */
    retVal = moveDataFromSwapAreaBackToTheOriginalBlock(shadowPtr,
                                                    bankIndex,
                                                    oldBucketShadowPtr,
                                                    oldMemSizeArr,
                                                    oldHwHandleAddrArr,
                                                    oldHwHandleGonIndexArr,
                                                    oldBucketShouldBeMovedArr,
                                                    useSwapAreaArr,
                                                    parentUpdateParams);

    if (retVal!=GT_OK)
    {
        /* should never happen since we just shrink the bank */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkBank - GT_FAIL \n");
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmSip6RamMngShrinkAllGonBanks function
* @endinternal
*
* @brief   Shrink all GONs partitions
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] neededMemoryBlocksPtr    - (pointer to) memory data needed for
*                                       shrink as:
*                                       1.the old bucket handle, if the value is
*                                         0, it means this is a new bucket, no
*                                         resize
*                                       2.the size (in lines) of the new bucket
*                                       3.the index of the bank to shrink
*                                       4.indication if the swap are is in use
*                                         GT_TRUE-swapArea will be used when
*                                         doing shrink
*                                         GT_FALSE - swapArea will NOT be used
*                                         when doing shrink
* @param[in] freeSpaceTakenFromBlockIndexArray - array holding the number of
*                                       lines needed from each block to do
*                                       defrag and to find place for all needed
*                                       sizes in all GONs
* @param[in]  parentUpdateParams- parameters needed for parent update
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmSip6RamMngShrinkAllGonBanks
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                     *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC            *neededMemoryBlocksPtr,
    IN GT_U32                                               freeSpaceTakenFromBlockIndexArray[],
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC                *parentUpdateParams
)
{
    GT_STATUS rc = GT_OK;
    GT_U32  j,k;
    GT_UINTPTR  oldHandleAddrUsedForShrinkArr[2]={0}; /* up to 2 swaps area used for old handle in srink operation */
    GT_U32      oldHwHandleGonIndexArr[2]={0};
    GT_BOOL     swapUsefullUsedForShrinkArr[2]={GT_FALSE};   /* up to 2 swaps area used for old handle in srink operation */
    GT_U32      newTotalSizeNeeded=0; /* in lines */
    GT_U32      tempNewTotalSizeNeeded=0;/* in lines */
    GT_U32      tempIndex=0;
    GT_U32      oldBlockIndex;

    /* we get all GONs of a specific level */
    for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
    {
       if(neededMemoryBlocksPtr->shrinkOperationUsefulForDefragArr[j]==GT_TRUE)
       {
           cpssOsMemSet(oldHandleAddrUsedForShrinkArr,0,sizeof(oldHandleAddrUsedForShrinkArr));
           cpssOsMemSet(oldHwHandleGonIndexArr,0,sizeof(oldHwHandleGonIndexArr));
           cpssOsMemSet(swapUsefullUsedForShrinkArr,0,sizeof(swapUsefullUsedForShrinkArr));
           tempIndex=0;
           tempNewTotalSizeNeeded=0;
           newTotalSizeNeeded=0;

           if(freeSpaceTakenFromBlockIndexArray[neededMemoryBlocksPtr->bankIndexForShrinkArr[j]]!=0)
           {
               newTotalSizeNeeded = (freeSpaceTakenFromBlockIndexArray[neededMemoryBlocksPtr->bankIndexForShrinkArr[j]])/
                                            PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS;
           }
           else
           {
               /* this bank was already shrinked */
               continue;
           }

            /* now go and gather information acording to newTotalSizeNeeded ....
               we need to deal together with all space needed in the same bank */
            for (k= 0; k < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; k++)
            {
               if((neededMemoryBlocksPtr->shrinkOperationUsefulForDefragArr[k]==GT_TRUE)&&
                  (neededMemoryBlocksPtr->bankIndexForShrinkArr[j]==neededMemoryBlocksPtr->bankIndexForShrinkArr[k]))
               {
                   tempNewTotalSizeNeeded += neededMemoryBlocksPtr->neededMemoryBlocksSizes[k];
                   if (tempNewTotalSizeNeeded<=newTotalSizeNeeded)
                   {
                       /* keep values */
                       if ((neededMemoryBlocksPtr->swapUsedForShrinkArr[k]==GT_TRUE)&&
                           (neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[k]!=0))
                       {
                           if(tempIndex<2)
                           {
                               swapUsefullUsedForShrinkArr[tempIndex] = neededMemoryBlocksPtr->swapUsedForShrinkArr[k];/* GT_TRUE */
                               oldHandleAddrUsedForShrinkArr[tempIndex] = neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[k];
                               oldHwHandleGonIndexArr[tempIndex] = neededMemoryBlocksPtr->neededMemoryBlocksOldHandleGonIndex[k];
                               tempIndex++;
                           }
                           else
                           {
                               /* error can not have more than 2 old hanle using swap area in shrink operation */
                               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkAllGonBanks "
                                                                     "error can not have more than 2 old hanle using swap area in shrink operation \n");
                           }
                       }
                   }
               }
               if (neededMemoryBlocksPtr->regularNode == GT_FALSE)
               {
                   break;
               }
           }
           /* now go and find if we have a gonIndex that is marked to use swapArea in add operation,
               and its oldHandle is located in the bank we are about to shrink.
               in this case while shrinking the bank we can change the location of the old handle,
               so we need to update the new location later in neededMemoryBlocksPtr */
            for (k= 0; k < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; k++)
            {
                if((neededMemoryBlocksPtr->neededMemoryBlocksSwapUsedForAdd[k]==GT_TRUE)&&
                   (neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[k]!=0)&&
                   (neededMemoryBlocksPtr->neededMemoryBlocksOldHandleGonIndex[k]==0xFFFE))
                {
                    /* check if the old located in the block we are about to shrink */
                    oldBlockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[k]) /
                                                                                    (shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    rc = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&oldBlockIndex);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                    }
                    if (oldBlockIndex==neededMemoryBlocksPtr->bankIndexForShrinkArr[j])
                    {
                        if(tempIndex<2)
                        {
                           oldHwHandleGonIndexArr[tempIndex] = neededMemoryBlocksPtr->neededMemoryBlocksOldHandleGonIndex[k];

                           /* no need to keep value in oldHandleAddrUsedForAddArr, since it will not be used in
                              prvCpssDxChLpmSip6RamMngShrinkBank if swapUsefullUsedForShrinkArr=false
                              we will use the value of
                              neededMemoryBlocksPtr->neededMemoryBlocksOldShadowBucket[k]->hwGroupOffsetHandle[0];
                              to update the pointer, since this is update inside the prvCpssDxChLpmSip6RamMngShrinkBank
                              once a block is moved */

                           /* since the gonIndex is 0xFFFE it means the old address is located in the first element */
                           /* double check the values */
                           if (neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[k]!=
                               neededMemoryBlocksPtr->neededMemoryBlocksOldShadowBucket[k]->hwGroupOffsetHandle[0])
                           {
                               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "ERROR:something went wrong - illegal oldAddress \n");
                           }
                           tempIndex++;
                        }
                        else
                        {
                           /* error can not have more than 2 old hanle using swap area in shrink operation */
                           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkAllGonBanks "
                                                                 "error can not have more than 2 old hanle using swap area in shrink operation \n");
                        }
                    }
                }
            }

            rc = prvCpssDxChLpmSip6RamMngShrinkBank(shadowPtr,
                                                    neededMemoryBlocksPtr->bankIndexForShrinkArr[j],
                                                    oldHandleAddrUsedForShrinkArr,/* array of 2 elements, can hold up to 2 oldHandels*/
                                                    oldHwHandleGonIndexArr,/* array of 2 elements, can hold up to 2 gon index */
                                                    newTotalSizeNeeded*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS,/* the total size needed from a specific bank for all GONs in this bank */
                                                    swapUsefullUsedForShrinkArr, /* array of 2 elements, can hold up to 2 flags for use of swap area */
                                                    parentUpdateParams);

            if (rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChLpmSip6RamMngShrinkAllGonBanks \n");
            }
            else
            {
                /* we reset the values in the array since we already treated with all shrink related to this bank */
                freeSpaceTakenFromBlockIndexArray[neededMemoryBlocksPtr->bankIndexForShrinkArr[j]] = 0;

                /* if the node loaction was changed due to use of swap area,
                   then we need to update the new location of the old Node/Gon
                   to be used in the updateMirrorGroupOfNodes */
                tempIndex=0;
                for (k= 0; k < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; k++)
                {
                    /* verify if this is the bank we shrinked.
                       in case we have 2 gons that are chaged in different banks and both use swap area for the shrink,
                       we do  not want to reset the values of one gon while working on the other gon */
                    if (neededMemoryBlocksPtr->bankIndexForShrinkArr[j]==neededMemoryBlocksPtr->bankIndexForShrinkArr[k])
                    {
                        if((neededMemoryBlocksPtr->swapUsedForShrinkArr[k]==GT_TRUE)&&
                           (neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[k]!=0))
                        {
                            if(tempIndex<2)
                            {
                                /* set new oldHandle */
                                neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[k]=oldHandleAddrUsedForShrinkArr[tempIndex];
                                tempIndex++;
                            }
                            else
                            {
                               /* error can not have more than 2 old hanle using swap area in shrink operation */
                               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkAllGonBanks "
                                                                     "error can not have more than 2 old hanle using swap area in shrink operation \n");
                            }
                        }
                    }
                }

                /* now go and find if we have a gonIndex that is marked to use swapArea in add operation,
                   and its oldHandle is located in the bank we shrinked.
                   in this case while shrinking the bank we migth have changed the location of the old handle,
                   so we need to update the location in neededMemoryBlocksPtr */
                for (k= 0; k < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; k++)
                {
                    if((neededMemoryBlocksPtr->neededMemoryBlocksSwapUsedForAdd[k]==GT_TRUE)&&
                       (neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[k]!=0)&&
                       (neededMemoryBlocksPtr->neededMemoryBlocksOldHandleGonIndex[k]==0xFFFE))
                    {
                        /* check if the old is located in the block we shrinked */
                        oldBlockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[k]) /
                                                                                        (shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                        rc = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&oldBlockIndex);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                        }
                        if (oldBlockIndex==neededMemoryBlocksPtr->bankIndexForShrinkArr[j])
                        {
                            if(tempIndex<2)
                            {
                                /* set new oldHandle, the value of hwGroupOffsetHandle[0] is updated once the block is moved  */
                                neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[k]=neededMemoryBlocksPtr->neededMemoryBlocksOldShadowBucket[k]->hwGroupOffsetHandle[0];
                                tempIndex++;
                            }
                            else
                            {
                               /* error can not have more than 2 old hanle using swap area in shrink operation */
                               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkAllGonBanks "
                                                                     "error can not have more than 2 old hanle using swap area in shrink operation \n");
                            }
                        }
                    }
                }
            }
        }
        if (neededMemoryBlocksPtr->regularNode == GT_FALSE)
        {
            break;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmSip6RamMngShrinkAllGonsRegToCompressBank function
* @endinternal
*
* @brief   Shrink the partition while puting all gons in swap area if needed
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] hwGroupOffsetHandle      - (pointer to) the old bucket regular GONs
* @param[in] bankIndex                - the index of the bank to shrink
* @param[in] newBucketSize            - the size (in lines) of the new bucket
* @param[in] useSwapAreaArr           - GT_TRUE  - swapArea will be used when
*                                      doing shrink
*                                      GT_FALSE - swapArea will NOT be used when
*                                      doing shrink
* @param[in] parentUpdateParams      - parameters needed for parent update
* @param[in] needToFreeAllocationInCaseOfFailPtr - (pointer to)
*                                       GT_TRUE: need to free memory
*                                       GT_FALSE: NO need to free memory
* @param[in] tempHwAddrHandleToBeFreedPtr - (pointer to)Hw handle to free in
*                                      case of fail
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmSip6RamMngShrinkAllGonsRegToCompressBank
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                             *shadowPtr,
    IN GT_UINTPTR                                                   *hwGroupOffsetHandle,
    IN GT_U32                                                       bankIndex,
    IN GT_U32                                                       newBucketSize,
    IN GT_BOOL                                                      useSwapArea,
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC                        *parentUpdateParams,
    IN GT_BOOL                                                      *needToFreeAllocationInCaseOfFailPtr,
    IN GT_UINTPTR                                                   *tempHwAddrHandleToBeFreedPtr
)
{
    GT_STATUS retVal=GT_OK;
    GT_STATUS retVal1=GT_OK;
    GT_U32    i,j,k;

    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
    GT_U32 shareDevListLen;

    GT_U32          bvLineIndex = 0;
    GT_UINTPTR      oldHwHandleAddrArr[2]={0};    /* empty arrays to be passed to prvCpssDxChLpmSip6RamMngShrinkBank - no use of swap area in this case */
    GT_U32          oldHwHandleGonIndexArr[2]={0};/* empty arrays to be passed to prvCpssDxChLpmSip6RamMngShrinkBank - no use of swap area in this case */
    GT_BOOL         swapUsedForShrinkArr[2]={0};  /* empty arrays to be passed to prvCpssDxChLpmSip6RamMngShrinkBank - no use of swap area in this case */
    GT_UINTPTR      tempOldHwAddrHandle[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];/* 6 Gons handles moved to swap area */
    GT_UINTPTR      tempOldHwAddrMemPool[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];/* 6 Gons memory pool from which the old buckets where allocated */
    GT_U32          tempOldHwAddrBlockSize[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];/* 6 Gons block size moved to swap area */
    GT_U32          tempNewSwapAddrOffset[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];/* 6 Gons offset moved to swap area */
    GT_UINTPTR      tempHwAddrHandle[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];/* new allocations done when moving back blocks to their original banks */
    GT_U32          memoryNeededFromEachBlock[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS]={0};
    GT_U32          swapOffset;/* offset of swap area1 to be used */
    GT_U32          hwGonDataArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_GROUP_OF_NODES_SIZE_IN_WORDS_CNS] = {0};
    GT_U32          blockIndex=0; /* calculated according to the memory offset devided by block size including gap */

    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;

    /* if a swap area flag is true then we need to put all 6 GONS of the regular bucket in the swap area1 */
    if (useSwapArea==GT_TRUE)
    {
        if(shadowPtr->swapMemoryAddr==0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected shadowPtr->swapMemoryAddr=0\n");
        }
        /* put all gons to swap area1 */
        swapOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->swapMemoryAddr);

        /* copy GONs into appropriate swap area:
           PRV_CPSS_DXCH_LPM_RAM_ALL_GONS_SWAP_INDEX_CNS is the flag that specify that 6 gons should be copied
           hwGroupOffsetHandle: adress to copy from
           swapOffset:          adress to copy to
           parentNodeData->swapGonsAdresses: output - here we will have the new offsets of the gons in the swap area1
           bvLineIndex: parameter not used in this case */
        retVal = updateSwapArea(bvLineIndex,hwGroupOffsetHandle,0xffffffff,PRV_CPSS_DXCH_LPM_RAM_ALL_GONS_SWAP_INDEX_CNS,
                                swapOffset,shareDevsList,shareDevListLen,parentUpdateParams->swapGonsAdresses);
        if (retVal != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "updateSwapArea failed");
        }

        /* keep the old hanles before we overwrite them with the new swap offset addresses */
         cpssOsMemCpy(tempOldHwAddrHandle,hwGroupOffsetHandle,sizeof(tempOldHwAddrHandle));

         /* keep the new swap area GON offset before we overwrite them with the new swap offset addresses */
         cpssOsMemCpy(tempNewSwapAddrOffset,parentUpdateParams->swapGonsAdresses,sizeof(tempNewSwapAddrOffset));


         /* the group of nodes bucket pointed on is updated in the HW,
            since the pointers are in swap area we do not have the handles to update in
            the shadow for given bucket hwGroupOffsetHandle[j], we only have the offsets.

            since this is  a temporary case we will update the shadow pointers only when
            puting the data back from swap to the new memory allocated
            */

        /* update pointers */
        parentUpdateParams->addOperation=GT_FALSE;
        parentUpdateParams->swapAreaAlsoUsedForAddOnParentLevel=GT_FALSE;
        parentUpdateParams->neededMemoryBlocksInfoPtr = NULL;

        parentUpdateParams->shadowPtr = shadowPtr;

        parentUpdateParams->funcCallCounter=1;

        if (parentUpdateParams->level==0)
        {
            parentUpdateParams->isRootNode = GT_TRUE;
            parentUpdateParams->bucketPtr = parentUpdateParams->bucketPtrArray[0];
            parentUpdateParams->pRange = NULL;
        }
        else
        {
            parentUpdateParams->isRootNode = GT_FALSE;
            parentUpdateParams->bucketPtr = parentUpdateParams->bucketPtrArray[(parentUpdateParams->level) - 1];
            parentUpdateParams->pRange = parentUpdateParams->rangePtrArray[(parentUpdateParams->level)-1];
        }
        retVal = updateHwRangeDataAndGonPtr(parentUpdateParams);

        /* free old Gons memory */
        for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
        {
            if ((tempOldHwAddrHandle[j] != DMM_BLOCK_NOT_FOUND) && (tempOldHwAddrHandle[j] != DMM_MALLOC_FAIL))
            {
                /* save old block index to be used latter */
                tempOldHwAddrMemPool[j] = DMM_GET_PARTITION(tempOldHwAddrHandle[j]);
                tempOldHwAddrBlockSize[j]=PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempOldHwAddrHandle[j]);

                /*  set pending flag for future need */
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempOldHwAddrHandle[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                if (retVal1 != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                }
                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempOldHwAddrHandle[j]);

                memoryNeededFromEachBlock[blockIndex]+=tempOldHwAddrBlockSize[j];
                prvCpssDmmFree(tempOldHwAddrHandle[j]);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "tempOldHwAddrHandle[j] should not be NULL ");
            }
        }

        /* now we shrink the block needed for the compress, according to the calculation done above
           we need space to put back the original gons copied from the swap area
           if the size needed for the new compress is bigger then the size used currently by
           the regular gon in the specific bank, we will shrink according to the bigger value */
        if (newBucketSize>(memoryNeededFromEachBlock[bankIndex]*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS))
        {
            /* shrink the bank - no use of swap area1 or 2 */
            retVal = prvCpssDxChLpmSip6RamMngShrinkBank(shadowPtr,
                                                bankIndex,/* bank index */
                                                oldHwHandleAddrArr,/* no use of old mem */
                                                oldHwHandleGonIndexArr,/* no use of gon index */
                                                newBucketSize,/* the total size needed from a specific bank for all GONs in this bank */
                                                swapUsedForShrinkArr, /* no use of swap */
                                                parentUpdateParams);
        }
        else
        {
            /* shrink the bank - no use of swap area1 or 2 */
            retVal = prvCpssDxChLpmSip6RamMngShrinkBank(shadowPtr,
                                                    bankIndex,/* bank index */
                                                    oldHwHandleAddrArr,/* no use of old mem */
                                                    oldHwHandleGonIndexArr,/* no use of gon index */
                                                    memoryNeededFromEachBlock[bankIndex]*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS,/* the total size needed from a specific bank for all GONs in this bank */
                                                    swapUsedForShrinkArr, /* no use of swap */
                                                    parentUpdateParams);
        }

        if (retVal!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"error in prvCpssDxChLpmSip6RamMngShrinkAllGonsRegToCompressBank \n");
        }

        /* now we shrink all the blocks used by the gons according to the calculation done above
           we need space to put back the original gons copied from the swap area */
        for (k=0;k<shadowPtr->numOfLpmMemories;k++)
        {
            if (memoryNeededFromEachBlock[k]!=0)
            {
                /* if the size needed for the new compress is bigger then the size used currently by
                   the regular gon in the specific bank, we will shrink according to the bigger value */
                if ((k==bankIndex)&&
                    (newBucketSize>(memoryNeededFromEachBlock[k]*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS)))
                {
                    /* shrink the bank - no use of swap area1 or 2 */
                    retVal = prvCpssDxChLpmSip6RamMngShrinkBank(shadowPtr,
                                                        k,/* bank index */
                                                        oldHwHandleAddrArr,/* no use of old mem */
                                                        oldHwHandleGonIndexArr,/* no use of gon index */
                                                        newBucketSize,/* the total size needed from a specific bank for all GONs in this bank */
                                                        swapUsedForShrinkArr, /* no use of swap */
                                                        parentUpdateParams);
                }
                else
                {
                    /* shrink the bank - no use of swap area1 or 2 */
                    retVal = prvCpssDxChLpmSip6RamMngShrinkBank(shadowPtr,
                                                            k,/* bank index */
                                                            oldHwHandleAddrArr,/* no use of old mem */
                                                            oldHwHandleGonIndexArr,/* no use of gon index */
                                                            memoryNeededFromEachBlock[k]*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS,/* the total size needed from a specific bank for all GONs in this bank */
                                                            swapUsedForShrinkArr, /* no use of swap */
                                                            parentUpdateParams);
                }

                if (retVal!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"error in prvCpssDxChLpmSip6RamMngShrinkAllGonsRegToCompressBank \n");
                }
            }
        }
    }
    else
    {
        /* in case swap area is not used then we only need to shrink the bank according to the
           newBucketSize given as a parameter to the function - this is the new size of the compress */

        /* shrink the bank - no use of swap area1 or 2 */
        retVal = prvCpssDxChLpmSip6RamMngShrinkBank(shadowPtr,
                                                bankIndex,
                                                oldHwHandleAddrArr,/* no use of old mem */
                                                oldHwHandleGonIndexArr,/* no use of gon index */
                                                newBucketSize,/* the total size needed from a specific bank for all GONs in this bank */
                                                swapUsedForShrinkArr, /* no use of swap */
                                                parentUpdateParams);

        if (retVal!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"error in prvCpssDxChLpmSip6RamMngShrinkAllGonsRegToCompressBank \n");
        }
    }
    if (useSwapArea==GT_TRUE)
    {
         /* reset swapGonsAdresses values */
         cpssOsMemSet(parentUpdateParams->swapGonsAdresses, 0xff, sizeof(parentUpdateParams->swapGonsAdresses));

         /* put all gons back to memory */
         for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
         {
             /* allocate space for the gons */
            if ((tempOldHwAddrHandle[j] != DMM_BLOCK_NOT_FOUND) && (tempOldHwAddrHandle[j] != DMM_MALLOC_FAIL))
            {
                tempHwAddrHandle[j] = prvCpssDmmAllocate(tempOldHwAddrMemPool[j],
                                                         DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * tempOldHwAddrBlockSize[j],
                                                         DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);
                if ((tempHwAddrHandle[j] == DMM_BLOCK_NOT_FOUND)||(tempHwAddrHandle[j] == DMM_MALLOC_FAIL))
                {
                   /* should never happen since we just freed this mem */
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " can't allocate memory - defrag");
                }
                else
                {
                    /* keep new tempHwAddrHandle[j] -- in case of a fail it should be freed */
                    needToFreeAllocationInCaseOfFailPtr[j] = GT_TRUE;
                    tempHwAddrHandleToBeFreedPtr[j] = tempHwAddrHandle[j];

                    /*  set pending flag for future need */
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                    if (retVal1 != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                    }
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandle[j]);


                    /* read data from swap and write it to new location */
                    for (i = 0; i < shareDevListLen; i++)
                    {

                        /* read GON from swap */
                        retVal = prvCpssDxChReadTableMultiEntry(shareDevsList[i],
                                                            CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                            tempNewSwapAddrOffset[j],
                                                            tempOldHwAddrBlockSize[j],
                                                            hwGonDataArr);
                        if (retVal != GT_OK)
                        {
                            /*  free allocated memory */
                            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                            retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                            if (retVal1 != GT_OK)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                            }
                            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandle[j]);

                            needToFreeAllocationInCaseOfFailPtr[j] = GT_FALSE;

                            prvCpssDmmFree(tempHwAddrHandle[j]);

                            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "LPM read fails");
                        }
                        /* Write the GON to new place  */
                        retVal = prvCpssDxChSip6RamMngWriteMultiEntry(shareDevsList[i],
                                                             CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                             PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[j]),
                                                             tempOldHwAddrBlockSize[j],
                                                             hwGonDataArr);
                        if (retVal != GT_OK)
                        {
                            /*  free allocated memory */
                            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                            retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                            if (retVal1 != GT_OK)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                            }
                            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandle[j]);

                            needToFreeAllocationInCaseOfFailPtr[j] = GT_FALSE;

                            prvCpssDmmFree(tempHwAddrHandle[j]);

                            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "LPM write fails");
                        }
                    }

                    /* ok since the group of nodes bucket pointed on is updated in the HW, we can now
                    update the shadow for given bucket
                    swapGonsAdresses will be used for the pointers update in updateHwRangeDataAndGonPtr */
                    hwGroupOffsetHandle[j]=tempHwAddrHandle[j];
                    parentUpdateParams->swapGonsAdresses[j] = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[j]);
                }
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "tempOldHwAddrHandle[j] should not be NULL ");
            }
         }

        /* update pointers */
        parentUpdateParams->addOperation=GT_FALSE;
        parentUpdateParams->swapAreaAlsoUsedForAddOnParentLevel=GT_FALSE;
        parentUpdateParams->neededMemoryBlocksInfoPtr = NULL;

        parentUpdateParams->shadowPtr = shadowPtr;

        parentUpdateParams->funcCallCounter=1;

        if (parentUpdateParams->level==0)
        {
            parentUpdateParams->isRootNode = GT_TRUE;
            parentUpdateParams->bucketPtr =  parentUpdateParams->bucketPtrArray[0];
            parentUpdateParams->pRange = NULL;
        }
        else
        {
            parentUpdateParams->isRootNode = GT_FALSE;
            parentUpdateParams->bucketPtr =  parentUpdateParams->bucketPtrArray[(parentUpdateParams->level)-1];
            parentUpdateParams->pRange = parentUpdateParams->rangePtrArray[(parentUpdateParams->level)-1];
        }

        retVal = updateHwRangeDataAndGonPtr(parentUpdateParams);
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmSip6RamMngShrinkCheckIfSwapAreaIsUseful function
* @endinternal
*
* @brief   Check if swap area can be used for space allocation.
*          First we try to allocate the space using the swap area
*          when neededMemoryBlocksSizesPtr.neededMemoryBlocksSwapUsedForAdd[j]
*          is GT_TRUE it means that swap area migth be usefull, it does not
*          mean we will use it for sure.
*
* @param[in] lpmEngineMemPtrPtr           - (pointer to) the lpm memory linked list holding
*                                           all blocked mapped
* @param[in] shadowPtr                    - (pointer to) the shadow information
* @param[inout] neededMemoryBlockSizesPtr - (pointer to) needed memory blocks data array
* @param[in] octetIndex                   - index to octet under work.
* @param[in] levelIndex                   - index to level under work.
* @param[in] bvLineIndex                  - index to bit vector line under work.
* @param[inout] firstSwapUsedPtr          - (pointer to) first swap area used info.
* @param[inout] secondSwapUsedPtr         - (pointer to) second swap area used info.
* @param[inout] thirdSwapUsedPtr          - (pointer to) third swap area used info.
*
*/
GT_VOID prvCpssDxChLpmSip6RamMngShrinkCheckIfSwapAreaIsUseful
(
    IN    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           **lpmEngineMemPtrPtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    INOUT PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC    *neededMemoryBlocksSizesPtr,
    IN    GT_U32                                       octetIndex,
    IN    GT_U32                                       levelIndex,
    IN    GT_U32                                       bvLineIndex,
    IN    GT_BOOL                                      *firstSwapUsedPtr,
    IN    GT_BOOL                                      *secondSwapUsedPtr,
    IN    GT_BOOL                                      *thirdSwapUsedPtr
)
{
    GT_STATUS   retVal=GT_OK;
    GT_U32      oldBlockIndex       = 0xFFFFFFFE; /* calculated according to the memory offset devided by block size including gap */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempNextMemInfoForSwapUsePtr;/* use for going over the list of blocks per octet */
    GT_U32      sizeOfOptionalUsedAndFreeBlock;
    GT_BOOL     firstSwapUsed  = *firstSwapUsedPtr;
    GT_BOOL     secondSwapUsed = *secondSwapUsedPtr;
    GT_BOOL     thirdSwapUsed  = *thirdSwapUsedPtr;
    PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC  *neededMemoryBlocksPtr;

    neededMemoryBlocksPtr  = shadowPtr->neededMemoryBlocksInfo;

    if(neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleAddr[bvLineIndex]==0)
    {
        cpssOsPrintf("Error: unexpected neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleAddr[bvLineIndex]=0\n");
        return;
    }
    oldBlockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleAddr[bvLineIndex])/
                                                                        (shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
    retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&oldBlockIndex);
    if (retVal != GT_OK)
    {
        CPSS_LOG_ERROR_MAC("ERROR:illegal blockIndex - fall in holes \n");
    }
    tempNextMemInfoForSwapUsePtr = lpmEngineMemPtrPtr[octetIndex];
    /* go over again on the list of blocks bound to the octet and look
       for the block with a potential to swap are use */
    while((tempNextMemInfoForSwapUsePtr!= NULL)&&(neededMemoryBlocksPtr[levelIndex].neededMemoryBlocks[bvLineIndex]==DMM_BLOCK_NOT_FOUND))
    {
        /* if the current memory block is the memory block of the oldHwBucketOffsetHandle
           we can achieve free space by using the swap area.*/
        if (tempNextMemInfoForSwapUsePtr->ramIndex==oldBlockIndex)
        {
            /* in this stage we must check again that if we use
               the swap area a reuse of the current memory can be done.
               in some cases (in shared mode) we can have the memory near the used one
               already been taken by another octet in case of sharing mode */

            /* another case that we need to double check is if we have a regular GON,
               in this case gons can share the same block, can lead to taking space
               assuming to be used for swap operation */
            if((shadowPtr->lpmRamBlocksAllocationMethod==PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E)||
               ((shadowPtr->lpmRamBlocksAllocationMethod!=PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E)&&
                (neededMemoryBlocksSizesPtr->regularNode==GT_TRUE)))
            {
                 sizeOfOptionalUsedAndFreeBlock =
                   prvCpssDmmCheckResizeAvailableWithSameMemory(neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleAddr[bvLineIndex],
                                                                ((neededMemoryBlocksSizesPtr->neededMemoryBlocksSizes[bvLineIndex])*
                                                                        PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS));
                if ((sizeOfOptionalUsedAndFreeBlock > 0) && (sizeOfOptionalUsedAndFreeBlock != DMM_BLOCK_NOT_FOUND))
                {
                    if (firstSwapUsed==GT_FALSE)
                    {
                        neededMemoryBlocksPtr[levelIndex].neededMemoryBlocks[bvLineIndex] = shadowPtr->swapMemoryAddr;
                        neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapUsedForAdd[bvLineIndex]=GT_TRUE; /* now the potential use of swap became a real use of swap1*/
                        neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapIndexUsedForAdd[bvLineIndex]=1;
                        firstSwapUsed=GT_TRUE;
                        break;
                    }
                    else
                    {
                        if (secondSwapUsed==GT_FALSE)
                        {
                            neededMemoryBlocksPtr[levelIndex].neededMemoryBlocks[bvLineIndex] = shadowPtr->secondSwapMemoryAddr;
                            neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapUsedForAdd[bvLineIndex]=GT_TRUE;/* now the potential use of swap became a real use of swap2*/
                            neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapIndexUsedForAdd[bvLineIndex]=2;
                            secondSwapUsed = GT_TRUE;
                            break;
                        }
                        else
                        {
                            if (thirdSwapUsed==GT_FALSE)
                            {
                                neededMemoryBlocksPtr[levelIndex].neededMemoryBlocks[bvLineIndex] = shadowPtr->thirdSwapMemoryAddr;
                                neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapUsedForAdd[bvLineIndex]=GT_TRUE;/* now the potential use of swap became a real use of swap3*/
                                neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapIndexUsedForAdd[bvLineIndex]=3;
                                thirdSwapUsed = GT_TRUE;
                                break;
                            }
                            else
                            {
                                /* no free swaps to use */
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                if (firstSwapUsed==GT_FALSE)
                {
                    neededMemoryBlocksPtr[levelIndex].neededMemoryBlocks[bvLineIndex] = shadowPtr->swapMemoryAddr;
                    neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapUsedForAdd[bvLineIndex]=GT_TRUE;/* now the potential use of swap became a real use of swap1*/
                    neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapIndexUsedForAdd[bvLineIndex]=1;
                    firstSwapUsed=GT_TRUE;
                    break;
                }
                else
                {
                    if (secondSwapUsed==GT_FALSE)
                    {
                        neededMemoryBlocksPtr[levelIndex].neededMemoryBlocks[bvLineIndex] = shadowPtr->secondSwapMemoryAddr;
                        neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapUsedForAdd[bvLineIndex]=GT_TRUE;/* now the potential use of swap became a real use of swap2*/
                        neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapIndexUsedForAdd[bvLineIndex]=2;
                        secondSwapUsed = GT_TRUE;
                        break;
                    }
                    else
                    {
                        if (thirdSwapUsed==GT_FALSE)
                        {
                            neededMemoryBlocksPtr[levelIndex].neededMemoryBlocks[bvLineIndex] = shadowPtr->thirdSwapMemoryAddr;
                            neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapUsedForAdd[bvLineIndex]=GT_TRUE;/* now the potential use of swap became a real use of swap3*/
                            neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapIndexUsedForAdd[bvLineIndex]=3;
                            thirdSwapUsed = GT_TRUE;
                            break;
                        }
                        else
                        {
                            /* no free swaps to use */
                            break;
                        }
                    }
                }
            }
        }
        tempNextMemInfoForSwapUsePtr = tempNextMemInfoForSwapUsePtr->nextMemInfoPtr;
    }

    *firstSwapUsedPtr  = firstSwapUsed;
    *secondSwapUsedPtr = secondSwapUsed;
    *thirdSwapUsedPtr  = thirdSwapUsed;
}

/**
* @internal prvCpssDxChLpmSip6RamMngShrinkAllocRegularGonCheck function
* @endinternal
*
* @brief   Check shrink for allocating regular GON.
*          if we have a regular GON then gons can share the same block,
*          this can lead to taking space assuming to be used for swap
*          operation need to double check that swap still can be used.
*
* @param[in] shadowPtr                  - (pointer to) the shadow information
* @param[in] lpmEngineMemPtrPtr         - (pointer to) the lpm memory linked list holding
*                                         all blocked mapped
* @param[in] protocol                   - type of ip protocol stack to work on.
* @param[in] octetIndex                 - index to the octet under work
* @param[in] neededMemoryBlocksSizesPtr - (pointer to) needed memory blocks sizes array
* @param[in] tempNewStructsMemPool      - new structs memory pool
* @param[inout] firstMemInfoInListToFreePtr  - (pointer to) use for going over the
*                                              list of blocks per octet
* @param[in] firstMemInfoInListToFreePtr -(pointer to) used for going over the list of blocks per octet
* @param[in] justBindAndDontAllocateFreePoolMemPtr -(pointer to) GT_TRUE: no memory pool is bound to the root of the octet list,
*                                                    in this case no need to allocate a memory pool struct just
*                                                    to bind it to a free pool
*                                                    GT_FALSE:allocate a new elemenet block to the list
* @param[in] levelIndex                  - index to the level under work
* @param[in] bvLineIndex                 - index to the bit vector line under work
* @param[inout] firstSwapUsedPtr         - (pointer to) first swap area used info.
* @param[inout] secondSwapUsedPtr        - (pointer to) second swap area used info.
* @param[inout] thirdSwapUsedPtr         - (pointer to) third swap area used info.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmSip6RamMngShrinkAllocRegularGonCheck
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           **lpmEngineMemPtrPtr,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol,
    IN    GT_U32                                       octetIndex,
    INOUT PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC    *neededMemoryBlocksSizesPtr,
    IN    GT_UINTPTR                                   tempNewStructsMemPool,
    INOUT PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           *firstMemInfoInListToFreePtr,
    INOUT GT_BOOL                                      *justBindAndDontAllocateFreePoolMemPtr,
    IN    GT_U32                                       levelIndex,
    IN    GT_U32                                       bvLineIndex,
    INOUT GT_BOOL                                      *firstSwapUsedPtr,
    INOUT GT_BOOL                                      *secondSwapUsedPtr,
    INOUT GT_BOOL                                      *thirdSwapUsedPtr
)
{
    GT_STATUS   retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempNextMemInfoPtr;/* use for going over the list of blocks per octet */
    GT_U32      sizeOfOptionalUsedAndFreeBlock;
    GT_BOOL     firstSwapUsed  = *firstSwapUsedPtr;
    GT_BOOL     secondSwapUsed = *secondSwapUsedPtr;
    GT_BOOL     thirdSwapUsed  = *thirdSwapUsedPtr;
    GT_U32      newFreeBlockIndex; /* index of a new free block */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  **allNewNextMemInfoAllocatedPerOctetArrayPtr;/*(pointer to) an array that holds for each octet
                                                                                      the allocted new element that need to be freed.
                                                                                      Size of the array is 16 for case of IPV6 */

    GT_BOOL     defragmentationEnable = GT_TRUE;
    PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC  *neededMemoryBlocksPtr;

    allNewNextMemInfoAllocatedPerOctetArrayPtr = shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr;

    neededMemoryBlocksPtr  = shadowPtr->neededMemoryBlocksInfo;

    sizeOfOptionalUsedAndFreeBlock =
      prvCpssDmmCheckResizeAvailableWithSameMemory(neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksOldHandleAddr[bvLineIndex],
                                                   ((neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSizes[bvLineIndex])*
                                                           PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS));
   if ((sizeOfOptionalUsedAndFreeBlock>0)&&(sizeOfOptionalUsedAndFreeBlock!=DMM_BLOCK_NOT_FOUND))
   {
    /* all is good, parameters are set correctly - continue */
   }
   else
   {
       /* reset swap parameters */
       if (neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapIndexUsedForAdd[bvLineIndex]==1)/* swapMemoryAddr */
       {
           neededMemoryBlocksPtr[levelIndex].neededMemoryBlocks[bvLineIndex] = DMM_BLOCK_NOT_FOUND;
           neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapUsedForAdd[bvLineIndex]=GT_FALSE;
           neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapIndexUsedForAdd[bvLineIndex]=0;
           firstSwapUsed=GT_FALSE;
       }
       else
       {
           if (neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapIndexUsedForAdd[bvLineIndex]==2)/* secondSwapMemoryAddr */
           {
               neededMemoryBlocksPtr[levelIndex].neededMemoryBlocks[bvLineIndex] = DMM_BLOCK_NOT_FOUND;
               neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapUsedForAdd[bvLineIndex]=GT_FALSE;
               neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapIndexUsedForAdd[bvLineIndex]=0;
               secondSwapUsed = GT_FALSE;
           }
           else
           {
               if (neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapIndexUsedForAdd[bvLineIndex]==3)/* thirdSwapMemoryAddr */
               {
                   neededMemoryBlocksPtr[levelIndex].neededMemoryBlocks[bvLineIndex] = DMM_BLOCK_NOT_FOUND;
                   neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapUsedForAdd[bvLineIndex]=GT_FALSE;
                   neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSwapIndexUsedForAdd[bvLineIndex]=0;
                   thirdSwapUsed = GT_FALSE;
               }
               else
               {
                   /* illegal index - can not happen */
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM, "illegal index for swap area - can not happen");
               }
           }
       }

       *firstSwapUsedPtr  = firstSwapUsed;
       *secondSwapUsedPtr = secondSwapUsed;
       *thirdSwapUsedPtr  = thirdSwapUsed;

       if ((neededMemoryBlocksPtr[levelIndex].neededMemoryBlocks[bvLineIndex]==DMM_BLOCK_NOT_FOUND)&&(tempNewStructsMemPool!=0))
       {
          /* in case we have 2 gons that wanted to use the swap areas but then they couldn't
             use it due to lack of space, the first one will get a new block but the second
             should use the new block given to the one before, and only then if there is no space
             to allocate a new block if needed */
           neededMemoryBlocksPtr[levelIndex].neededMemoryBlocks[bvLineIndex] =
                                       prvCpssDmmAllocate(tempNewStructsMemPool,
                                                          DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * (neededMemoryBlocksPtr[levelIndex].neededMemoryBlocksSizes[bvLineIndex]),
                                                          DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);
       }

       /* get to the last pointer in the list */
       tempNextMemInfoPtr = lpmEngineMemPtrPtr[octetIndex];
       while(tempNextMemInfoPtr->nextMemInfoPtr!= NULL)
       {
           tempNextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
       }

       /* if using swap area do not help then find an available new free block that
           could be bound to the octet */
       if (neededMemoryBlocksPtr[levelIndex].neededMemoryBlocks[bvLineIndex]==DMM_BLOCK_NOT_FOUND)
       {
           retVal = findAndBindValidMemoryBlock(shadowPtr, protocol, octetIndex,
                                            &neededMemoryBlocksPtr[levelIndex].neededMemoryBlocks[bvLineIndex],
                                            neededMemoryBlocksSizesPtr->neededMemoryBlocksSizes[bvLineIndex],
                                            tempNextMemInfoPtr,
                                            firstMemInfoInListToFreePtr,
                                            justBindAndDontAllocateFreePoolMemPtr,
                                            allNewNextMemInfoAllocatedPerOctetArrayPtr,
                                            tempNewStructsMemPool,
                                            &newFreeBlockIndex,
                                            shadowPtr->globalMemoryBlockTakenArr);
           if (retVal != GT_OK)
           {
               if ((retVal == GT_OUT_OF_PP_MEM)&&(defragmentationEnable==GT_TRUE)&&
                   (shadowPtr->lpmRamBlocksAllocationMethod!=PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E))
               {
                  /* if we did not find a free block to bind we
                  should continue and try to shrink the blocks,
                  this will happen only after going over all the GONs
                  Sharing mode do not support shrink */
                  retVal = GT_OK;
                  return retVal;
               }
               else
               {
                   return retVal;
               }
           }
       }
   }

    return retVal;
}

/**
* @internal prvCpssDxChLpmSip6RamRetryAllocNeededMemoryAfterShrink function
* @endinternal
*
* @brief   Retry needed memory allocation after shrink operation is done.
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] lpmEngineMemPtrPtr       - (pointer to) the lpm memory linked list holding
*                                       all blocked mapped
* @param[in] protocol                 - type of ip protocol stack to work on.
* @param[in] neededMemoryBlocksPtr    - (pointer to) memory data needed for
*                                       shrink as:
*                                       1.the old bucket handle, if the value is
*                                         0, it means this is a new bucket, no
*                                         resize
*                                       2.the size (in lines) of the new bucket
*                                       3.the index of the bank to shrink
*                                       4.indication if the swap are is in use
*                                         GT_TRUE-swapArea will be used when
*                                         doing shrink
*                                         GT_FALSE - swapArea will NOT be used
*                                         when doing shrink
* @param[inout] firstSwapUsedPtr      - (pointer to) first swap area used info.
* @param[inout] secondSwapUsedPtr     - (pointer to) second swap area used info.
* @param[inout] thirdSwapUsedPtr      - (pointer to) third swap area used info.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
static GT_STATUS prvCpssDxChLpmSip6RamMngRetryAllocNeededMemoryAfterShrink
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           **lpmEngineMemPtrPtr,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol,
    INOUT PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC    *neededMemoryBlocksPtr,
    INOUT GT_BOOL                                      *firstSwapUsedPtr,
    INOUT GT_BOOL                                      *secondSwapUsedPtr,
    INOUT GT_BOOL                                      *thirdSwapUsedPtr
)
{
    GT_STATUS   retVal = GT_OK;
    GT_BOOL     firstSwapUsed  = *firstSwapUsedPtr;
    GT_BOOL     secondSwapUsed = *secondSwapUsedPtr;
    GT_BOOL     thirdSwapUsed  = *thirdSwapUsedPtr;
    GT_U32      blockIndex;
    GT_U32      j;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *tempNextMemInfoPtr;/* use for going over the list of blocks per octet */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *potentialNewBlockPtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *firstMemInfoInListToFreePtr = NULL;/* use for going over the list of blocks per octet */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC **allNewNextMemInfoAllocatedPerOctetArrayPtr;/*(pointer to) an array that holds for each octet
                                                                                      the allocted new element that need to be freed.
                                                                                      Size of the array is 16 for case of IPV6 */
    GT_UINTPTR      oldHandleToCheck[2] = {0,0};
    GT_U32          newSize[2]={0,0};
    GT_U32          numberOfLinesToMark=0,sizeOfMarkedArea=0,sizeOfMarkedArea0=0,sizeOfMarkedArea1=0;
    GT_DMM_BLOCK    *nextByAddr,*prevByAddr;
    GT_U32          i=0, bankIndex = 0;
    GT_U32          lpmGroupLineOffset = 0;
    GT_UINTPTR      tempHwAddrHandle[2] = {0,0};



    /* find if we need to mark the area near the oldMemory as taken */
    for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
    {
        if(((neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_BLOCK_NOT_FOUND) ||
            (neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_MALLOC_FAIL) ||
            (neededMemoryBlocksPtr->neededMemoryBlocks[j] == 0))&&
            (neededMemoryBlocksPtr->neededMemoryBlocksSizes[j] != 0))
        {

            if (neededMemoryBlocksPtr->swapUsedForShrinkArr[j] == GT_TRUE)
            {
                 /* in case we used the swap_area_1 for the shrink operation
                   need to pass this information to the update mirror bucket,
                   so it will do the logic code using the swap */
                if (firstSwapUsed == GT_FALSE)
                {
                    neededMemoryBlocksPtr->neededMemoryBlocks[j] = shadowPtr->swapMemoryAddr;
                    neededMemoryBlocksPtr->neededMemoryBlocksSwapIndexUsedForAdd[j] = 1; /* parameter used is case of Add or Shrink*/
                    neededMemoryBlocksPtr->neededMemoryBlocksSwapUsedForAdd[j] = neededMemoryBlocksPtr->swapUsedForShrinkArr[j];/* GT_TRUE */
                    firstSwapUsed = GT_TRUE;
                    oldHandleToCheck[0] = neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[j];
                    newSize[0] = neededMemoryBlocksPtr->neededMemoryBlocksSizes[j];

                    if (newSize[0]>=PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(oldHandleToCheck[0]))
                    {
                        numberOfLinesToMark = newSize[0] - PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(oldHandleToCheck[0]);

                        nextByAddr = ((GT_DMM_BLOCK *)oldHandleToCheck[0])->nextByAddr;
                        prevByAddr = ((GT_DMM_BLOCK *)oldHandleToCheck[0])->prevByAddr;
                        if (nextByAddr != NULL)
                        {
                            if (DMM_BLOCK_STATUS(nextByAddr) == DMM_BLOCK_FREE)
                            {
                                sizeOfMarkedArea=PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(nextByAddr);

                                if (sizeOfMarkedArea>=numberOfLinesToMark)
                                {
                                    sizeOfMarkedArea0 = sizeOfMarkedArea;
                                    /* set as occupied */
                                }
                            }
                        }
                        else
                        {
                            sizeOfMarkedArea0 = 0;
                        }
                        if (prevByAddr != NULL)
                        {
                            if (DMM_BLOCK_STATUS(prevByAddr)==DMM_BLOCK_FREE)
                            {
                                sizeOfMarkedArea=PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(prevByAddr);

                                if (sizeOfMarkedArea>=numberOfLinesToMark)
                                {
                                    /* set as occupied */
                                    sizeOfMarkedArea1 = sizeOfMarkedArea;
                                }
                            }
                        }
                        else
                        {
                            sizeOfMarkedArea1 = 0;
                        }
                        if ( (sizeOfMarkedArea0 == 0) && (sizeOfMarkedArea1 == 0))
                        {
                             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "ERROR:not enough free space after shrink \n");
                        }
                        else if (sizeOfMarkedArea0 == 0)
                        {
                            /* alocation would be done by prevByAddr */
                            lpmGroupLineOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(prevByAddr);
                        }
                        else if (sizeOfMarkedArea1 == 0)
                        {
                            /* alocation would be done by nextByAddr */
                            lpmGroupLineOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(nextByAddr);
                        }
                        else
                        {
                            /* allocation can be done from both of sides */
                            /* let's find area with minimum free lines */
                            if (sizeOfMarkedArea0 < sizeOfMarkedArea1)
                            {
                                /* alocation would be done by nextByAddr */
                                lpmGroupLineOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(nextByAddr);
                            }
                            else
                            {
                                /* alocation would be done by prevByAddr */
                                lpmGroupLineOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(prevByAddr);
                            }
                        }

                        /* do fake allocation */
                        bankIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(oldHandleToCheck[0]) /
                                                                                    (shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                        retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&bankIndex);
                        if (retVal != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                        }
                        retVal = prvCpssDmmAllocateByPtr(shadowPtr->lpmRamStructsMemPoolPtr[bankIndex],
                                                         DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS* lpmGroupLineOffset,
                                                         DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS* numberOfLinesToMark,
                                                         DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS,
                                                         &tempHwAddrHandle[0]);
                        if ((tempHwAddrHandle[0] == DMM_BLOCK_NOT_FOUND)||(tempHwAddrHandle[0] == DMM_MALLOC_FAIL))
                        {
                            /* should never happen since we are allocating DMM acording to the existing HW */
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in swPointerAllocateAccordingtoHwAddress - GT_FAIL \n");
                        }

                    }
                    else
                    {
                        /* something is wrong*/
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "ERROR:can not set blocks as free \n");
                    }

                }
                else
                {
                    if (secondSwapUsed == GT_FALSE)
                    {
                        neededMemoryBlocksPtr->neededMemoryBlocks[j] = shadowPtr->secondSwapMemoryAddr;
                        neededMemoryBlocksPtr->neededMemoryBlocksSwapIndexUsedForAdd[j] = 2;/* parameter used is case of Add or Shrink*/
                        neededMemoryBlocksPtr->neededMemoryBlocksSwapUsedForAdd[j] = neededMemoryBlocksPtr->swapUsedForShrinkArr[j];/* GT_TRUE */
                        secondSwapUsed = GT_TRUE;
                        oldHandleToCheck[1] = neededMemoryBlocksPtr->neededMemoryBlocksOldHandleAddr[j];
                        newSize[1] = neededMemoryBlocksPtr->neededMemoryBlocksSizes[j];
                        if (newSize[1]>=PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(oldHandleToCheck[0]))
                        {
                            numberOfLinesToMark = newSize[1] - PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(oldHandleToCheck[1]);

                            nextByAddr = ((GT_DMM_BLOCK *)oldHandleToCheck[1])->nextByAddr;
                            prevByAddr = ((GT_DMM_BLOCK *)oldHandleToCheck[1])->prevByAddr;
                            if (nextByAddr != NULL)
                            {
                                if (DMM_BLOCK_STATUS(nextByAddr)==DMM_BLOCK_FREE)
                                {
                                    sizeOfMarkedArea=PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(nextByAddr);

                                    if (sizeOfMarkedArea>=numberOfLinesToMark)
                                    {
                                        sizeOfMarkedArea0 = sizeOfMarkedArea;
                                        /* set as occupied*/
                                    }
                                }
                            }
                            else
                            {
                                sizeOfMarkedArea0 = 0;
                            }
                            if (prevByAddr != NULL)
                            {
                                if (DMM_BLOCK_STATUS(prevByAddr)==DMM_BLOCK_FREE)
                                {
                                   sizeOfMarkedArea=PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(prevByAddr);

                                   if (sizeOfMarkedArea>=numberOfLinesToMark)
                                   {
                                       /* set as occupied */
                                       sizeOfMarkedArea1 = sizeOfMarkedArea;
                                   }
                                }
                            }
                            else
                            {
                                sizeOfMarkedArea1 = 0;
                            }
                            if ( (sizeOfMarkedArea0 == 0) && (sizeOfMarkedArea1 == 0))
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "ERROR:not enough free space after shrink \n");
                            }
                            else if (sizeOfMarkedArea0 == 0)
                            {
                               /* alocation would be done by prevByAddr */
                                lpmGroupLineOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(prevByAddr);
                            }
                            else if (sizeOfMarkedArea1 == 0)
                            {
                               /* alocation would be done by nextByAddr */
                               lpmGroupLineOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(nextByAddr);
                            }
                            else
                            {
                               /* allocation can be done from both of sides */
                               /* let's find area with minimum free lines */
                               if (sizeOfMarkedArea0 < sizeOfMarkedArea1)
                               {
                                   /* alocation would be done by nextByAddr */
                                   lpmGroupLineOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(nextByAddr);
                               }
                               else
                               {
                                   /* alocation would be done by prevByAddr */
                                   lpmGroupLineOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(prevByAddr);
                               }
                            }

                               /* do fake allocation */
                            bankIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(oldHandleToCheck[1]) /
                                                                                           (shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                            retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&bankIndex);
                            if (retVal != GT_OK)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                            }
                            retVal = prvCpssDmmAllocateByPtr(shadowPtr->lpmRamStructsMemPoolPtr[bankIndex],
                                                             DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS* lpmGroupLineOffset,
                                                             DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS* numberOfLinesToMark,
                                                             DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS,
                                                             &tempHwAddrHandle[1]);
                            if ((tempHwAddrHandle[1] == DMM_BLOCK_NOT_FOUND)||(tempHwAddrHandle[1] == DMM_MALLOC_FAIL))
                            {
                                /* should never happen since we are allocating DMM acording to the existing HW */
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in swPointerAllocateAccordingtoHwAddress - GT_FAIL \n");
                            }
                        }
                        else
                        {
                               /* something is wrong*/
                               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "ERROR:can not set blocks as free \n");
                        }
                    }
                    else
                    {
                        /* should never happen, third swap is only used for shrink operation
                           any case of more than 2 swap areas used is not legal */
                        if(neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_MALLOC_FAIL)
                            retVal = GT_OUT_OF_CPU_MEM;
                        else
                            retVal = GT_OUT_OF_PP_MEM;
                        break;
                    }
                }
            }
        }
    }
    allNewNextMemInfoAllocatedPerOctetArrayPtr = shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr;

    for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
    {
        /* for blocks that was not allocated yet and was marked as part of shrink operation
           we try to allocate a new memory */
        if (neededMemoryBlocksPtr->shrinkOperationUsefulForDefragArr[j] == GT_TRUE)
        {
            if(((neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_BLOCK_NOT_FOUND) ||
                (neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_MALLOC_FAIL) ||
                (neededMemoryBlocksPtr->neededMemoryBlocks[j] == 0))&&
                (neededMemoryBlocksPtr->neededMemoryBlocksSizes[j] != 0))
            {

            if (neededMemoryBlocksPtr->swapUsedForShrinkArr[j] == GT_TRUE)
            {
                 /* in case we used the swap_area_1 for the shrink operation
                   need to pass this information to the update mirror bucket,
                   so it will do the logic code using the swap */
                if (firstSwapUsed == GT_FALSE)
                {
                    neededMemoryBlocksPtr->neededMemoryBlocks[j] = shadowPtr->swapMemoryAddr;
                    neededMemoryBlocksPtr->neededMemoryBlocksSwapIndexUsedForAdd[j] = 1; /* parameter used is case of Add or Shrink*/
                    neededMemoryBlocksPtr->neededMemoryBlocksSwapUsedForAdd[j] = neededMemoryBlocksPtr->swapUsedForShrinkArr[j];/* GT_TRUE */
                    firstSwapUsed = GT_TRUE;
                }
                else
                {
                    if (secondSwapUsed == GT_FALSE)
                    {
                        neededMemoryBlocksPtr->neededMemoryBlocks[j] = shadowPtr->secondSwapMemoryAddr;
                        neededMemoryBlocksPtr->neededMemoryBlocksSwapIndexUsedForAdd[j] = 2;/* parameter used is case of Add or Shrink*/
                        neededMemoryBlocksPtr->neededMemoryBlocksSwapUsedForAdd[j] = neededMemoryBlocksPtr->swapUsedForShrinkArr[j];/* GT_TRUE */
                        secondSwapUsed = GT_TRUE;

                    }
                    else
                    {
                        /* should never happen, third swap is only used for shrink operation
                           any case of more than 2 swap areas used is not legal */
                        if(neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_MALLOC_FAIL)
                            retVal = GT_OUT_OF_CPU_MEM;
                        else
                            retVal = GT_OUT_OF_PP_MEM;
                        break;
                    }
                }
            }
            else
            {
                /* try to allocate again the memory needed - operation should pass */
                neededMemoryBlocksPtr->neededMemoryBlocks[j] =
                            prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[neededMemoryBlocksPtr->bankIndexForShrinkArr[j]],
                                DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * (neededMemoryBlocksPtr->neededMemoryBlocksSizes[j]),
                                DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);
            }

            /* if the neededMemoryBlocks is 0 or 0xFFFFFFFF --> then we did not succeed in allocating the needed memory  */
            if ((neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_BLOCK_NOT_FOUND)||
                (neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_MALLOC_FAIL))
            {
                /* should not happen since we just shrinked the memory */
                if(neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_MALLOC_FAIL)
                    retVal = GT_OUT_OF_CPU_MEM;
                else
                    retVal = GT_OUT_OF_PP_MEM;
                break;
            }

            /* if we are using swap area then no allocation was done */
            if (neededMemoryBlocksPtr->swapUsedForShrinkArr[j] == GT_FALSE)
            {
                /* need to check if the block we shrink is an unbounded block,
                   in this case we need to bound the block to the list */
                if(PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_OCTET_IN_PROTOCOL_GET_MAC(shadowPtr,protocol,
                                                                                    neededMemoryBlocksPtr->octetId,
                                                                                    neededMemoryBlocksPtr->bankIndexForShrinkArr[j]) == GT_FALSE)
                {
                    tempNextMemInfoPtr = lpmEngineMemPtrPtr[neededMemoryBlocksPtr->octetId];
                    /* if this is the first element in the list, just need to bind */
                    if(tempNextMemInfoPtr->structsMemPool == 0)
                    {
                        tempNextMemInfoPtr->ramIndex = neededMemoryBlocksPtr->bankIndexForShrinkArr[j];
                        tempNextMemInfoPtr->structsBase = 0;
                        /* bind the new block */
                        tempNextMemInfoPtr->structsMemPool = shadowPtr->lpmRamStructsMemPoolPtr[neededMemoryBlocksPtr->bankIndexForShrinkArr[j]];
                        tempNextMemInfoPtr->nextMemInfoPtr = NULL;
                        /* first element in linked list of the blocks that
                           need to be freed in case of an error is the root */
                        firstMemInfoInListToFreePtr = tempNextMemInfoPtr;
                    }
                    else
                    {
                         /* get to the last pointer in the list */
                        while(tempNextMemInfoPtr->nextMemInfoPtr != NULL)
                        {
                            tempNextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                        }
                        /* allocate a new elemenet block to the list */
                        potentialNewBlockPtr =  (PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC));
                        if (potentialNewBlockPtr == NULL)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                        }
                        potentialNewBlockPtr->ramIndex       = neededMemoryBlocksPtr->bankIndexForShrinkArr[j];
                        potentialNewBlockPtr->structsBase    = 0;
                        potentialNewBlockPtr->structsMemPool = 0;
                        potentialNewBlockPtr->nextMemInfoPtr = NULL;

                        /* bind the new block */
                        potentialNewBlockPtr->structsMemPool = shadowPtr->lpmRamStructsMemPoolPtr[neededMemoryBlocksPtr->bankIndexForShrinkArr[j]];
                        tempNextMemInfoPtr->nextMemInfoPtr = potentialNewBlockPtr;

                        /* first element in linked list of the blocks that
                           need to be freed in case of an error is the father
                           of the new block added to the list
                          (father of potentialNewBlockPtr is tempNextMemInfoPtr) */
                        firstMemInfoInListToFreePtr = tempNextMemInfoPtr;
                    }

                    /* mark the block as used */
                    shadowPtr->lpmRamOctetsToBlockMappingPtr[neededMemoryBlocksPtr->bankIndexForShrinkArr[j]].isBlockUsed = GT_TRUE;
                    /* set the block as taken */
                    shadowPtr->globalMemoryBlockTakenArr[neededMemoryBlocksPtr->bankIndexForShrinkArr[j]] ++;
                    /*  set the block to be used by the specific octet and protocol*/
                    PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_SET_MAC(shadowPtr,protocol,
                                                                         neededMemoryBlocksPtr->octetId,
                                                                         neededMemoryBlocksPtr->bankIndexForShrinkArr[j]);

                    /* We only save the first element allocated or bound per octet */
                    if ((allNewNextMemInfoAllocatedPerOctetArrayPtr[neededMemoryBlocksPtr->octetId] == NULL) ||
                        (allNewNextMemInfoAllocatedPerOctetArrayPtr[neededMemoryBlocksPtr->octetId]->structsMemPool == 0))
                    {
                        /* keep the head of the list we need to free in case of an error -
                        first element is the father of the first element that should be freed */
                        allNewNextMemInfoAllocatedPerOctetArrayPtr[neededMemoryBlocksPtr->octetId] = firstMemInfoInListToFreePtr;
                    }
                }

                    /* set pending flag for future need */
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocksPtr->neededMemoryBlocks[j]) /
                                        (shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                    if (retVal != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                    }
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc = GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(neededMemoryBlocksPtr->neededMemoryBlocks[j]);
                }
            }
        }
    }

    /* If we had a case where swap area was in use and we marked some lines next to the oldBlock area
       as occupied, whne actual allocation of the GONs is finished, we need to undo the mark so the
       lines will be considered as free.
       The final allocation of those lines will be done in updateMirrorGroupOfNodes */
    for (i=0;i<2;i++)
    {
        if (tempHwAddrHandle[i] != 0)
        {
            prvCpssDmmFree(tempHwAddrHandle[i]);
        }
    }

    *firstSwapUsedPtr  = firstSwapUsed;
    *secondSwapUsedPtr = secondSwapUsed;
    *thirdSwapUsedPtr  = thirdSwapUsed;

    return retVal;
}

/**
* @internal prvCpssDxChLpmSip6RamMngRetryAllocNeededMemoryAfterMerge function
* @endinternal
*
* @brief   Retry needed memory allocation after merge operation is done.
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] lpmEngineMemPtrPtr       - (pointer to) the lpm memory linked list holding
*                                       all blocked mapped
* @param[in] protocol                 - type of ip protocol stack to work on.
* @param[in] neededMemoryBlocksPtr    - (pointer to) memory data needed for
*                                       shrink as:
*                                       1.the old bucket handle, if the value is
*                                         0, it means this is a new bucket, no
*                                         resize
*                                       2.the size (in lines) of the new bucket
*                                       3.the index of the bank to shrink
*                                       4.indication if the swap are is in use
*                                         GT_TRUE-swapArea will be used when
*                                         doing shrink
*                                         GT_FALSE - swapArea will NOT be used
*                                         when doing shrink
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
static GT_STATUS prvCpssDxChLpmSip6RamMngRetryAllocNeededMemoryAfterMerge
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           **lpmEngineMemPtrPtr,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol,
    INOUT PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC    *neededMemoryBlocksPtr
)
{
    GT_STATUS   retVal = GT_OK;
    GT_U32      blockIndex;
    GT_U32      j;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *tempNextMemInfoPtr;/* use for going over the list of blocks per octet */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *potentialNewBlockPtr=NULL;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *firstMemInfoInListToFreePtr=NULL;/* use for going over the list of blocks per octet */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC **allNewNextMemInfoAllocatedPerOctetArrayPtr;/*(pointer to) an array that holds for each octet
                                                                                      the allocted new element that need to be freed.
                                                                                      Size of the array is 16 for case of IPV6 */

    allNewNextMemInfoAllocatedPerOctetArrayPtr = shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr;

    for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
    {
        if(((neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_BLOCK_NOT_FOUND) ||
            (neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_MALLOC_FAIL) ||
            (neededMemoryBlocksPtr->neededMemoryBlocks[j] == 0))&&
            (neededMemoryBlocksPtr->neededMemoryBlocksSizes[j] != 0))
        {
            if (neededMemoryBlocksPtr->mergeOperationUsefulForDefragArr[j] == GT_TRUE)
            {
                /* try to allocate again the memory needed - operation should pass */
                neededMemoryBlocksPtr->neededMemoryBlocks[j] =
                            prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[neededMemoryBlocksPtr->bankIndexForMergeArr[j]],
                                DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * (neededMemoryBlocksPtr->neededMemoryBlocksSizes[j]),
                                DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);
            }

            /* if the neededMemoryBlocks is 0 or 0xFFFFFFFF --> then we did not succeed in allocating the needed memory  */
            if ((neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_BLOCK_NOT_FOUND)||
                (neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_MALLOC_FAIL))
            {
                /* should not happen since we just shrinked the memory */
                if(neededMemoryBlocksPtr->neededMemoryBlocks[j] == DMM_MALLOC_FAIL)
                    retVal = GT_OUT_OF_CPU_MEM;
                else
                    retVal = GT_OUT_OF_PP_MEM;
                break;
            }

            /* need to check if the block we shrink is an unbounded block,
               in this case we need to bound the block to the list */
            if(PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_OCTET_IN_PROTOCOL_GET_MAC(shadowPtr,protocol,
                                                                                neededMemoryBlocksPtr->octetId,
                                                                                neededMemoryBlocksPtr->bankIndexForMergeArr[j])==GT_FALSE)
            {
                tempNextMemInfoPtr = lpmEngineMemPtrPtr[neededMemoryBlocksPtr->octetId];
                /* if this is the first element in the list, just need to bind */
                if(tempNextMemInfoPtr->structsMemPool==0)
                {
                    tempNextMemInfoPtr->ramIndex = neededMemoryBlocksPtr->bankIndexForMergeArr[j];
                    tempNextMemInfoPtr->structsBase=0;
                    /* bind the new block */
                    tempNextMemInfoPtr->structsMemPool=shadowPtr->lpmRamStructsMemPoolPtr[neededMemoryBlocksPtr->bankIndexForMergeArr[j]];
                    tempNextMemInfoPtr->nextMemInfoPtr=NULL;
                    /* first element in linked list of the blocks that
                       need to be freed in case of an error is the root */
                    firstMemInfoInListToFreePtr = tempNextMemInfoPtr;
                }
                else
                {
                     /* get to the last pointer in the list */
                    while(tempNextMemInfoPtr->nextMemInfoPtr!= NULL)
                    {
                        tempNextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                    }
                    /* allocate a new elemenet block to the list */
                    potentialNewBlockPtr =  (PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC));
                    if (potentialNewBlockPtr == NULL)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }
                    potentialNewBlockPtr->ramIndex = neededMemoryBlocksPtr->bankIndexForMergeArr[j];
                    potentialNewBlockPtr->structsBase = 0;
                    potentialNewBlockPtr->structsMemPool = 0;
                    potentialNewBlockPtr->nextMemInfoPtr = NULL;

                    /* bind the new block */
                    potentialNewBlockPtr->structsMemPool=shadowPtr->lpmRamStructsMemPoolPtr[neededMemoryBlocksPtr->bankIndexForMergeArr[j]];
                    tempNextMemInfoPtr->nextMemInfoPtr = potentialNewBlockPtr;

                    /* first element in linked list of the blocks that
                       need to be freed in case of an error is the father
                       of the new block added to the list
                      (father of potentialNewBlockPtr is tempNextMemInfoPtr) */
                    firstMemInfoInListToFreePtr = tempNextMemInfoPtr;
                }

                /* mark the block as used */
                shadowPtr->lpmRamOctetsToBlockMappingPtr[neededMemoryBlocksPtr->bankIndexForMergeArr[j]].isBlockUsed=GT_TRUE;
                /* set the block as taken */
                shadowPtr->globalMemoryBlockTakenArr[neededMemoryBlocksPtr->bankIndexForMergeArr[j]]++;
                /*  set the block to be used by the specific octet and protocol*/
                PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_SET_MAC(shadowPtr,protocol,
                                                                     neededMemoryBlocksPtr->octetId,
                                                                     neededMemoryBlocksPtr->bankIndexForMergeArr[j]);

                /* We only save the first element allocated or bound per octet */
                if ((allNewNextMemInfoAllocatedPerOctetArrayPtr[neededMemoryBlocksPtr->octetId]==NULL)||
                    (allNewNextMemInfoAllocatedPerOctetArrayPtr[neededMemoryBlocksPtr->octetId]->structsMemPool==0))
                {
                    /* keep the head of the list we need to free in case of an error -
                    first element is the father of the first element that should be freed */
                    allNewNextMemInfoAllocatedPerOctetArrayPtr[neededMemoryBlocksPtr->octetId] = firstMemInfoInListToFreePtr;
                }
            }

            /* set pending flag for future need */
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocksPtr->neededMemoryBlocks[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
            }
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(neededMemoryBlocksPtr->neededMemoryBlocks[j]);

            neededMemoryBlocksPtr->mergeOperationUsefulForDefragArr[j] = GT_FALSE;
        }
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmSip6RamMngDefragCheckAndApply function
* @endinternal
*
* @brief   Apply defragmentation and allocate needed memory.
*          In case we did not succeed in allocated all memory needed,
*          we try to shrink the blocks.
*
* @param[in] shadowPtr                   - (pointer to) the shadow information
* @param[in] lpmEngineMemPtrPtr          - (pointer to) the lpm memory linked list holding
*                                          all blocked mapped
* @param[in] protocol                    - type of ip protocol stack to work on.
* @param[in] startIdx                    - start index for needed memory list.
* @param[in] parentUpdateParams          - parameters needed for parent update
* @param[inout] neededMemoryListLenTreatedSuccessfullyPtr - (pointer to) flg to check whether
*                                                           needed memory list is fully handled.
* @param[inout] firstSwapUsedPtr         - (pointer to) first swap area used info.
* @param[inout] secondSwapUsedPtr        - (pointer to) second swap area used info.
* @param[inout] thirdSwapUsedPtr         - (pointer to) third swap area used info.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmSip6RamMngDefragCheckAndApply
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           **lpmEngineMemPtrPtr,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol,
    IN    GT_U32                                       startIdx,
    IN    PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC        *parentUpdateParams,
    INOUT GT_U32                                       *neededMemoryListLenTreatedSuccessfullyPtr,
    IN    GT_BOOL                                      *firstSwapUsedPtr,
    IN    GT_BOOL                                      *secondSwapUsedPtr,
    IN    GT_BOOL                                      *thirdSwapUsedPtr
)
{
    /* At the moment we do not support shrink in sharing mode  TBD */

    /* for all unallocated octets we try to find a free memory by using defrag mechanism
       we will look at the following elements values from the stages above
       neededMemoryBlocksInfo[i].neededMemoryBlocks[j]
       neededMemoryBlocksInfo[i].neededMemoryBlocksSizes[j]
       neededMemoryBlocksInfo[i].neededMemoryBlocksSwapUsedForAdd[j]
       neededMemoryBlocksInfo[i].octetId
       */

    GT_STATUS   retVal = GT_OK;
    GT_BOOL     checkShrink = GT_FALSE;
    GT_BOOL     freeBlockCanBeFoundForAllNeededBlocks = GT_TRUE; /* flag will be changed if defrag is not possible */
    GT_U32      freeSpaceTakenFromBlockIndexArray[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS]={0}; /* in words not in lines */
    GT_BOOL     shrinkWasCheckedSuccessfully = GT_FALSE;/* flag for checking shrink */
    GT_BOOL     mergeWasCheckedSuccessfully = GT_FALSE;/* flag for checking shrink */
    GT_U32      i,j;
    GT_U32      blockIndex;
    GT_U32      oldBlockIndex       = 0xFFFFFFFE; /* calculated according to the memory offset devided by block size including gap */
    GT_BOOL     mergeBankCanBeFound = GT_FALSE;
    GT_BOOL     freeBlockCanBeFound = GT_FALSE;
    PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC  *neededMemoryBlocksInfo;
    PRV_CPSS_SIP6_LPM_MERGE_RESERVATIONS_STC   mergeReservationsArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];
    GT_BOOL                                    isMarkedForShrinkArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    neededMemoryBlocksInfo  = shadowPtr->neededMemoryBlocksInfo;

    for(i = startIdx; i < shadowPtr->neededMemoryListLen; i++)
    {
        checkShrink=GT_FALSE;

        if(freeBlockCanBeFoundForAllNeededBlocks==GT_TRUE)
        {
            /* for all memory not allocated we check that shrink will help for all octet allocation
               only after we get GT_TRUE for all shrinks we will start shrinking */
            for (j= 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
            {
                if(((neededMemoryBlocksInfo[i].neededMemoryBlocks[j] == DMM_BLOCK_NOT_FOUND)||
                    (neededMemoryBlocksInfo[i].neededMemoryBlocks[j] == DMM_MALLOC_FAIL)||
                    (neededMemoryBlocksInfo[i].neededMemoryBlocks[j] ==0))&&
                    (neededMemoryBlocksInfo[i].neededMemoryBlocksSizes[j]!=0))
                {
                    /* in the firt instant we find that shrink is needed we will call
                       the function that will deal with all GONs in one place*/
                    checkShrink = GT_TRUE;
                    break;
                }
                else
                {
                    /* allocation was done */
                    if (neededMemoryBlocksInfo[i].regularNode==GT_FALSE)
                    {
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            if (checkShrink == GT_TRUE)
            {
                shrinkWasCheckedSuccessfully = GT_FALSE;
                retVal = prvCpssDxChLpmSip6RamMngCheckIfShrinkOperationUsefulForDefrag(shadowPtr,
                                                                                  lpmEngineMemPtrPtr[neededMemoryBlocksInfo[i].octetId],
                                                                                  protocol,
                                                                                  &neededMemoryBlocksInfo[i],
                                                                                  &freeBlockCanBeFound,
                                                                                  freeSpaceTakenFromBlockIndexArray);
                if (retVal!=GT_OK)
                {
                    break;
                }

                if (freeBlockCanBeFound == GT_FALSE)
                {
                    if (shadowPtr->defragSip6MergeEnable == GT_TRUE)
                    {
                        /* This is initialization to do check for whether merge operation is useful.
                           It will be then set to GT_FALSE if it is not useful */
                        neededMemoryBlocksInfo[i].mergeOperationUsefulForDefragGlobalFlag = GT_TRUE;
                    }
                    else
                    {
                        /* first time we get a GT_FAIL we concludes that defrag will not help */
                        freeBlockCanBeFoundForAllNeededBlocks = GT_FALSE;
                        retVal = GT_OUT_OF_PP_MEM;
                        break;
                    }
                }
                else
                {
                    /* if freeBlockCanBeFound=GT_TRUE then
                    all the following fields were already updated for future use
                    neededMemoryBlocksInfo[i].shrinkOperationUsefulForDefragArr[j]=GT_TRUE;
                    neededMemoryBlocksInfo[i].bankIndexForShrinkArr[j]=bankIndexForShrink;
                    neededMemoryBlocksInfo[i].swapUsedForShrinkArr[j]=useSwapArea;*/

                    /* set a flag that shrink should be done */
                    shrinkWasCheckedSuccessfully = GT_TRUE;
                }
            }
        }
    }

    cpssOsMemSet(mergeReservationsArr, 0, sizeof(mergeReservationsArr));
    cpssOsMemSet(isMarkedForShrinkArr, 0, sizeof(isMarkedForShrinkArr));

    /* Check if any banks are marked for shrink. If used, exclude them for merge */
    for(i = startIdx; i < shadowPtr->neededMemoryListLen; i++)
    {
        for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
        {
            if (shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksSwapUsedForAdd[j] == GT_TRUE)
            {
                if(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksOldHandleAddr[j]==0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksOldHandleAddr[j]=0\n");
                }
                oldBlockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksOldHandleAddr[j])/
                                                                    (shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&oldBlockIndex);
                if (retVal != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                }

                if (oldBlockIndex >= PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "illegal octet index");
                }
                isMarkedForShrinkArr[oldBlockIndex] = GT_TRUE;
            }
            if(shadowPtr->neededMemoryBlocksInfo[i].shrinkOperationUsefulForDefragArr[j]==GT_TRUE)
            {
                /* prevent the block index we want to shrink to be part of the merge logic */
                oldBlockIndex = shadowPtr->neededMemoryBlocksInfo[i].bankIndexForShrinkArr[j];
                if (oldBlockIndex >= PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "illegal octet index");
                }
                isMarkedForShrinkArr[oldBlockIndex] = GT_TRUE;
            }
        }
    }

    for(i = startIdx; i < shadowPtr->neededMemoryListLen; i++)
    {
        if ((shadowPtr->defragSip6MergeEnable == GT_TRUE) &&
            (neededMemoryBlocksInfo[i].mergeOperationUsefulForDefragGlobalFlag == GT_TRUE))
        {
            mergeWasCheckedSuccessfully = GT_FALSE;
            neededMemoryBlocksInfo[i].mergeOperationUsefulForDefragGlobalFlag = GT_FALSE;

            retVal = prvCpssDxChLpmSip6RamMngCheckIfMergeBankOperationUsefulForDefrag(shadowPtr,
                                                                                      lpmEngineMemPtrPtr,
                                                                                      protocol,
                                                                                      i,
                                                                                      isMarkedForShrinkArr,
                                                                                      mergeReservationsArr,
                                                                                      &mergeBankCanBeFound);
            if (retVal != GT_OK)
            {
                break;
            }

            if (mergeBankCanBeFound == GT_FALSE)
            {
                /* first time we get a GT_FAIL we concludes that defrag will not help */
                freeBlockCanBeFoundForAllNeededBlocks = GT_FALSE;
                neededMemoryBlocksInfo[i].mergeOperationUsefulForDefragGlobalFlag = GT_FALSE;
                retVal = GT_OUT_OF_PP_MEM;
                break;
            }
            else
            {
                mergeWasCheckedSuccessfully = GT_TRUE;
            }
         }
    }

    if((retVal==GT_OK)&&
       (freeBlockCanBeFoundForAllNeededBlocks==GT_TRUE)&&
       (shrinkWasCheckedSuccessfully==GT_TRUE))/* if we get here it means all needed memory can be found using shrink */
    {
        /* it meeans that defrag will help, start shrinking and try to allocate again */
        for(i = startIdx; i < shadowPtr->neededMemoryListLen; i++)
        {
            if (neededMemoryBlocksInfo[i].shrinkOperationUsefulForDefragGlobalFlag==GT_TRUE)
            {
                /* sometimes we can have a special case when we have 6 GONs, and one of the GONs don't have enough memory,
                   but the rest was allocated successfully. for this single GON we need to do shrink, BUT we cannot move
                   a block that was not written in the HW, so we might get a fail although we do have place to allocate
                   all memory needed. In this case we will temporary remove the allocated memory for the GONs,
                   and we will shrink the bank taking into consideration the size of those allocated memory */
                for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                {
                    /* temporary free the memory allocated */
                    if(((neededMemoryBlocksInfo[i].neededMemoryBlocks[j] != DMM_BLOCK_NOT_FOUND)&&
                        (neededMemoryBlocksInfo[i].neededMemoryBlocks[j] != DMM_MALLOC_FAIL)&&
                        (neededMemoryBlocksInfo[i].neededMemoryBlocks[j] != 0))&&
                        (neededMemoryBlocksInfo[i].neededMemoryBlocksSizes[j]!=0))
                    {
                        /* if we are using swap area then no allocation was done */
                        if (neededMemoryBlocksInfo[i].neededMemoryBlocksSwapUsedForAdd[j] == GT_FALSE)
                        {
                            /* check if this memory was allocated from the same block we are about to shrink */
                            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocksInfo[i].neededMemoryBlocks[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                            retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                            if (retVal != GT_OK)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                            }
                            if (freeSpaceTakenFromBlockIndexArray[blockIndex]!=0)
                            {
                                /* this block is about to be shrinked,add the amount of memory
                                   we are about to delete into the total size for shrink operation */
                                freeSpaceTakenFromBlockIndexArray[blockIndex]+= (neededMemoryBlocksInfo[i].neededMemoryBlocksSizes[j]
                                                                                 * PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS);

                                /* set flag that this memory should be taken from the bank we are about to shrink */
                                neededMemoryBlocksInfo[i].shrinkOperationUsefulForDefragArr[j] = GT_TRUE;
                                neededMemoryBlocksInfo[i].bankIndexForShrinkArr[j] = blockIndex;

                                /* set pending flag for future need */
                                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;

                                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);

                                prvCpssDmmFree(neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);

                                /* reset value */
                                neededMemoryBlocksInfo[i].neededMemoryBlocks[j]=DMM_BLOCK_NOT_FOUND;
                            }
                        }
                    }
                }

                parentUpdateParams->level = neededMemoryBlocksInfo[i].octetId;

                /* shrink the Banks we found in previous phase */
                retVal = prvCpssDxChLpmSip6RamMngShrinkAllGonBanks(shadowPtr,
                                                            &neededMemoryBlocksInfo[i],
                                                            freeSpaceTakenFromBlockIndexArray,
                                                            parentUpdateParams);
                if (retVal != GT_OK)
                {
                    break;
                }

                /* banks were shrinked try to allocate again */
                retVal = prvCpssDxChLpmSip6RamMngRetryAllocNeededMemoryAfterShrink(shadowPtr,
                                                            lpmEngineMemPtrPtr,
                                                            protocol,
                                                            &neededMemoryBlocksInfo[i],
                                                            firstSwapUsedPtr,
                                                            secondSwapUsedPtr,
                                                            thirdSwapUsedPtr);
                if (retVal != GT_OK)
                {
                    break;
                }
            }
            else
            {
                /* neededMemoryBlocksInfo[i] was already treated and memory was allocated,
                   no need to shrink for this i */
                continue;
            }
            if ((retVal == GT_OUT_OF_CPU_MEM)||(retVal == GT_OUT_OF_PP_MEM))
            {
                break;
            }
        }
        *neededMemoryListLenTreatedSuccessfullyPtr=i;
    }

    if ((retVal == GT_OK) &&
        (shadowPtr->defragSip6MergeEnable == GT_TRUE) &&
        (mergeWasCheckedSuccessfully == GT_TRUE))
    {
        for(i = startIdx; i < shadowPtr->neededMemoryListLen; i++)
        {
            if (neededMemoryBlocksInfo[i].mergeOperationUsefulForDefragGlobalFlag == GT_FALSE)
            {
                continue;
            }

            parentUpdateParams->level = neededMemoryBlocksInfo[i].octetId;

            /* Merge banks */
            retVal = prvCpssDxChLpmSip6RamMngMergeAllGonBanks(shadowPtr,
                                                              lpmEngineMemPtrPtr,
                                                              protocol,
                                                              &neededMemoryBlocksInfo[i],
                                                              isMarkedForShrinkArr,
                                                              parentUpdateParams);
            if (retVal != GT_OK)
            {
                break;
            }

            /* banks were shrinked try to allocate again */
            retVal = prvCpssDxChLpmSip6RamMngRetryAllocNeededMemoryAfterMerge(shadowPtr,
                                                                              lpmEngineMemPtrPtr,
                                                                              protocol,
                                                                              &neededMemoryBlocksInfo[i]);
            if (retVal != GT_OK)
            {
                break;
            }
        }
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamMngCheckIfMergeBankOperationUsefulForDefrag function
* @endinternal
*
* @brief   Check if merge bank operation may help for defrag
*
* @param[in] shadowPtr                - (pointer to) the shadow information
*                                      lpmEngineMemPtrPtr - points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                      which holds all the memory information needed for where and
*                                      how to allocate search memory for each of the lpm levels
* @param[in] lpmEngineMemPtrPtr       - (pointer to) the lpm memory linked list holding
*                                       all blocked mapped
* @param[in] protocol                 - the protocol
* @param[in] neededMemoryIndex         - needed memory index of the bucket we are working on
* @param[in] isMarkedForShrinkArr     - Array to check if bank should be ignored for merge
*                                       as it is used for shrink.
* @param[in] mergeReservationsArr      - store modifications related to merge operation.
* @param[out] mergeBankCanBeFoundPtr   - (pointer to)
*                                      GT_TRUE: empty block can be found if we merge memory
*                                      GT_FALSE: empty block can NOT be found even if we merge memory
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - a potential bank for merge was not found
*/
GT_STATUS prvCpssDxChLpmSip6RamMngCheckIfMergeBankOperationUsefulForDefrag
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC          **lpmMemInfoPtrPtr,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT        protocol,
    IN    GT_U32                                      neededMemoryIndex,
    IN    GT_BOOL                                     *isMarkedForShrinkArr,
    INOUT PRV_CPSS_SIP6_LPM_MERGE_RESERVATIONS_STC    *mergeReservationsArr,
    OUT   GT_BOOL                                     *mergeBankCanBeFoundPtr
)
{
    GT_STATUS retVal=GT_OK;
    GT_BOOL mergeBank        = GT_FALSE; /* did we found a bank to merge */
    GT_U32  mergeSourceOctet = 0;        /* the octet mapped to the bank we are going to merge */
    GT_U32  mergeSourceBank  = 0;        /* the bank index we are going to merge */
    GT_U32  extraLines       = 0;        /* number of extra lines not used by any
                                            of the banks related to mergeSourceOctet */
    GT_U32  maxNumOfOctets   = 0;        /* max number of octets according to the protocol */
    GT_U32  octetInd;                    /* index to run over all octets */
    GT_U32  sumOfSourceLines;            /* calculation of total lines that will have relocated -
                                            only for the specific protocol buckets */
    GT_U32  bestSumOfSourceLines=0;      /* best result calculation of total lines that will have relocated -
                                            only for the specific protocol buckets */
    GT_U32  sumOfFreeLinesAtTargetBanks=0; /* calculation of total free lines in the bank we want to merge */
    GT_U32  temp;
    GT_U32  blockIndex = 0;      /* the block index of a given bank we are working on */
    GT_BOOL allUpdatedInHwFlag;
    GT_U32  i,j;
    GT_U32  octetReservedLines[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_MAX_CNS];
    GT_U32  newBucketSize = 0;
    GT_U8   bankReserveIndexes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *tempLpmMemInfoPtr; /* use for going over the list of
                                                                   blocks mapped to a specific octet */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *secondTempLpmMemInfoPtr;
    PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC  *neededMemoryBlocksPtr;

    neededMemoryBlocksPtr  = &shadowPtr->neededMemoryBlocksInfo[neededMemoryIndex];

    cpssOsMemSet(octetReservedLines, 0, sizeof(octetReservedLines));
    cpssOsMemSet(bankReserveIndexes, 0xFF, sizeof(bankReserveIndexes));

    switch (protocol)
    {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* We try to allocate all GONs to a new bank */
    for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
    {
        if ((neededMemoryBlocksPtr->neededMemoryBlocksSizes[i] == 0) ||
            ((neededMemoryBlocksPtr->neededMemoryBlocksSizes[i] != 0) &&
             (neededMemoryBlocksPtr->neededMemoryBlocks[i] != DMM_BLOCK_NOT_FOUND) &&
             (neededMemoryBlocksPtr->neededMemoryBlocks[i] != DMM_MALLOC_FAIL) &&
             (neededMemoryBlocksPtr->neededMemoryBlocks[i] != 0)))
        {
            /* the block was allocated or no need to allocate it */
            continue;
        }

        if ((neededMemoryBlocksPtr->mergeOperationUsefulForDefragArr[i] == GT_FALSE) ||
            (neededMemoryBlocksPtr->octetIndexForMergeArr[i] != PRV_CPSS_DXCH_SIP6_LPM_RAM_MERGE_NOT_CHECKED) ||
            (neededMemoryBlocksPtr->bankIndexForMergeArr[i] != PRV_CPSS_DXCH_SIP6_LPM_RAM_MERGE_NOT_CHECKED))
        {
            /* No need to check for merge for this GON.
               It is already allocated by previous merge or shrink or normal allocation */
            continue;
        }

        mergeSourceOctet = PRV_CPSS_DXCH_SIP6_LPM_RAM_MERGE_NOT_CHECKED;
        mergeSourceBank  = PRV_CPSS_DXCH_SIP6_LPM_RAM_MERGE_NOT_CHECKED;
        neededMemoryBlocksPtr->mergeOperationUsefulForDefragArr[i] = GT_FALSE;
        neededMemoryBlocksPtr->octetIndexForMergeArr[i] = 0;
        neededMemoryBlocksPtr->bankIndexForMergeArr[i] = 0;

        /* Check whether Merge is already done for one of the GONS for the current octet.
           If yes, check if the new bank assigned can be used to store current GON */
        for(j = 0; j < i ; j++)
        {
            if (mergeReservationsArr[bankReserveIndexes[j]].freeLines >= neededMemoryBlocksPtr->neededMemoryBlocksSizes[i])
            {
                neededMemoryBlocksPtr->mergeOperationUsefulForDefragArr[i]     = GT_TRUE;
                neededMemoryBlocksPtr->octetIndexForMergeArr[i]                = mergeReservationsArr[bankReserveIndexes[j]].octetIdx;
                neededMemoryBlocksPtr->bankIndexForMergeArr[i]                 = bankReserveIndexes[j];
                neededMemoryBlocksPtr->mergeOperationUsefulForDefragGlobalFlag = GT_TRUE;

                mergeReservationsArr[bankReserveIndexes[j]].freeLines -= neededMemoryBlocksPtr->neededMemoryBlocksSizes[i];
                break;
            }
        }
        /* If space is available in previously allocated block just use it. No need for additional merge operation */
        if (neededMemoryBlocksPtr->mergeOperationUsefulForDefragArr[i] == GT_TRUE)
        {
            continue;
        }

        /* Scan all other octets that are not the octetIndex we got as parameter to this function
           Try to find the best single bank to release.
           We scan all banks from all octets no matter what - we want to verify we take the best.
           The "best" is considered the one that has most extra free lines at its target banks
           - on top the ones required to hold its source lines.
           Note that doesn't necessarily the one with the least lines to move. */

        /* scan all other octets of the protocol */
        for (octetInd = 0; octetInd < maxNumOfOctets; octetInd++)
        {
            if (octetInd == neededMemoryBlocksPtr->octetId)
            {
                /* no need to check the octetIndex we got as parameter to this function */
                continue;
            }
            /* skip octets with less than 2 banks - no potential to merge */
            if (lpmMemInfoPtrPtr[octetInd]->nextMemInfoPtr == NULL)
            {
                continue;
            }
            tempLpmMemInfoPtr = lpmMemInfoPtrPtr[octetInd];

            /* try to find the bank with the best chance to succeed in releasing */
            while (tempLpmMemInfoPtr != NULL)
            {
                /* bank 0 can not be merged to other banks since it is a special bank holding the swap areas */
                if ((tempLpmMemInfoPtr->ramIndex == 0))
                {
                    tempLpmMemInfoPtr = tempLpmMemInfoPtr->nextMemInfoPtr;
                    continue;
                }

                /* reset values each new loop */
                sumOfFreeLinesAtTargetBanks = 0;
                blockIndex = tempLpmMemInfoPtr->ramIndex;

                /* First check that the potential bank to merge do not hold blocks that were not
                   yet updated in HW, such in case that we are looking for a place for MC
                   group but the source was not yet updated in HW */
                retVal = prvCpssDmmAllPartitionIsUpdatedInHw(tempLpmMemInfoPtr->structsMemPool,&allUpdatedInHwFlag);
                if (retVal!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"error in prvCpssDmmAllPartitionIsUpdatedInHw - GT_FAIL \n");
                }

                if (allUpdatedInHwFlag == GT_FALSE)
                {
                    tempLpmMemInfoPtr = tempLpmMemInfoPtr->nextMemInfoPtr;
                    continue;
                }

                /* Do not consider the bank if it is already maarked for shrink */
                if (isMarkedForShrinkArr[tempLpmMemInfoPtr->ramIndex] == GT_TRUE)
                {
                    tempLpmMemInfoPtr = tempLpmMemInfoPtr->nextMemInfoPtr;
                    continue;
                }

                /* Do not consider the bank if it is already marked for merge */
                if (mergeReservationsArr[tempLpmMemInfoPtr->ramIndex].isMarkedForMerge == GT_TRUE)
                {
                    /* Cannot use this bank as it is already marked to be freed */
                    octetReservedLines[octetInd] += mergeReservationsArr[tempLpmMemInfoPtr->ramIndex].sourceLines;
                    tempLpmMemInfoPtr = tempLpmMemInfoPtr->nextMemInfoPtr;
                    continue;
                }

                /* total lines that will have relocated - only specific protocol buckets

                   at this point the counters we have in protocolCountersPerBlockArr do
                   not show the correct picture.
                   In some cases prvCpssDxChLpmRamUpdateBlockUsageCounters was not
                   called yet so we need to take into consideration also the pending counters */
                switch (protocol)
                {
                case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
                    sumOfSourceLines = (shadowPtr->protocolCountersPerBlockArr[tempLpmMemInfoPtr->ramIndex].sumOfIpv4Counters+
                                        shadowPtr->pendingBlockToUpdateArr[tempLpmMemInfoPtr->ramIndex].numOfIncUpdates-
                                        shadowPtr->pendingBlockToUpdateArr[tempLpmMemInfoPtr->ramIndex].numOfDecUpdates);
                    break;
                case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
                    sumOfSourceLines = (shadowPtr->protocolCountersPerBlockArr[tempLpmMemInfoPtr->ramIndex].sumOfIpv6Counters+
                                     shadowPtr->pendingBlockToUpdateArr[tempLpmMemInfoPtr->ramIndex].numOfIncUpdates-
                                     shadowPtr->pendingBlockToUpdateArr[tempLpmMemInfoPtr->ramIndex].numOfDecUpdates);
                    break;
                case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
                    sumOfSourceLines = (shadowPtr->protocolCountersPerBlockArr[tempLpmMemInfoPtr->ramIndex].sumOfFcoeCounters+
                                     shadowPtr->pendingBlockToUpdateArr[tempLpmMemInfoPtr->ramIndex].numOfIncUpdates-
                                     shadowPtr->pendingBlockToUpdateArr[tempLpmMemInfoPtr->ramIndex].numOfDecUpdates);
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"error in illegal protocol - GT_BAD_PARAM \n");

                }

                mergeReservationsArr[tempLpmMemInfoPtr->ramIndex].sourceLines          = sumOfSourceLines;

                newBucketSize = neededMemoryBlocksPtr->neededMemoryBlocksSizes[i];
                /* check that relocating the buckets will allow to add the new one instead */
                if (sumOfSourceLines < newBucketSize)
                {
                    tempLpmMemInfoPtr = tempLpmMemInfoPtr->nextMemInfoPtr;
                    continue;
                }

                if(mergeBank == GT_FALSE)/* we still did not find a potential bank to merge */
                {
                    bestSumOfSourceLines = sumOfSourceLines;
                }

                /* after getting a potential bank to be merged, we check that there is enough
                   free space in the rest of the banks to merge it to.

                   total free lines at target banks */
                secondTempLpmMemInfoPtr = lpmMemInfoPtrPtr[octetInd];
                while (secondTempLpmMemInfoPtr != NULL)
                {
                    /* skip the bank we found as potential for merge */
                    if (secondTempLpmMemInfoPtr->structsMemPool == tempLpmMemInfoPtr->structsMemPool)
                    {
                        secondTempLpmMemInfoPtr=secondTempLpmMemInfoPtr->nextMemInfoPtr;
                        continue;
                    }

                    if (mergeReservationsArr[secondTempLpmMemInfoPtr->ramIndex].isMarkedForMerge == GT_TRUE)
                    {
                        /* Cannot use this bank as it is already marked to be freed */
                        secondTempLpmMemInfoPtr=secondTempLpmMemInfoPtr->nextMemInfoPtr;
                        continue;
                    }

                    /* total free lines in the target banks

                       at this point the counters we have in protocolCountersPerBlockArr do
                       not show the correct picture.
                       In some cases prvCpssDxChLpmRamUpdateBlockUsageCounters was not
                       called yet so we need to take into consideration also the pending counters
                       example: add new prefix -
                       octet 0 was taken from bank3 -> numOfIncUpdates=4
                       octet 1 has no place so we go and check if a merge can be done
                       the merge checks bank3 for allocated and free space.
                       if we use the counters we will get a wrong value since numOfIncUpdates=4 was
                       not update in protocolCountersPerBlockArr.
                       the update is done in the end of the add operation only after all went well */
                    temp = shadowPtr->lpmRamBlocksSizeArrayPtr[secondTempLpmMemInfoPtr->ramIndex]-
                                    (shadowPtr->protocolCountersPerBlockArr[secondTempLpmMemInfoPtr->ramIndex].sumOfIpv4Counters+
                                     shadowPtr->protocolCountersPerBlockArr[secondTempLpmMemInfoPtr->ramIndex].sumOfIpv6Counters+
                                     shadowPtr->protocolCountersPerBlockArr[secondTempLpmMemInfoPtr->ramIndex].sumOfFcoeCounters+
                                     shadowPtr->pendingBlockToUpdateArr[secondTempLpmMemInfoPtr->ramIndex].numOfIncUpdates-
                                     shadowPtr->pendingBlockToUpdateArr[secondTempLpmMemInfoPtr->ramIndex].numOfDecUpdates);

                    sumOfFreeLinesAtTargetBanks += temp;

                    mergeReservationsArr[secondTempLpmMemInfoPtr->ramIndex].freeLines            = temp;

                    secondTempLpmMemInfoPtr=secondTempLpmMemInfoPtr->nextMemInfoPtr;
                }

                if (sumOfFreeLinesAtTargetBanks < octetReservedLines[octetInd])
                {
                    tempLpmMemInfoPtr = tempLpmMemInfoPtr->nextMemInfoPtr;
                    continue;
                }

                if (sumOfFreeLinesAtTargetBanks >= octetReservedLines[octetInd])
                {
                    /* substract number of relocated lines due to other merge operation */
                    sumOfFreeLinesAtTargetBanks -= octetReservedLines[octetInd];
                }
                else
                {
                    sumOfFreeLinesAtTargetBanks = 0;
                }

                /* check enough free space */
                if (sumOfFreeLinesAtTargetBanks < sumOfSourceLines)
                {
                    tempLpmMemInfoPtr = tempLpmMemInfoPtr->nextMemInfoPtr;
                    continue;
                }

                /* check if this is the best bank to relocate so far or first one we found */
                if (((sumOfFreeLinesAtTargetBanks-sumOfSourceLines)>extraLines)||
                    (mergeBank==GT_FALSE)/* first potential bank found */ ||
                    (((sumOfFreeLinesAtTargetBanks-sumOfSourceLines)==extraLines) &&
                     (sumOfSourceLines < bestSumOfSourceLines)))
                {
                    mergeBank=GT_TRUE;           /* we found a bank to merge */
                    mergeSourceOctet = octetInd; /* the octet using the bank we are going to merge is the current octet */
                    mergeSourceBank = blockIndex;/* the bank index we are going to merge is the current bank */
                    /* number of extra lines not used by any of the banks related to mergeSourceOctet */
                    extraLines = sumOfFreeLinesAtTargetBanks-sumOfSourceLines;
                    bestSumOfSourceLines = sumOfSourceLines;
                }
                tempLpmMemInfoPtr = tempLpmMemInfoPtr->nextMemInfoPtr;
            }
        }

        if (mergeSourceBank != PRV_CPSS_DXCH_SIP6_LPM_RAM_MERGE_NOT_CHECKED)
        {
            mergeReservationsArr[mergeSourceBank].octetIdx             = mergeSourceOctet;
            mergeReservationsArr[mergeSourceBank].reassignedOctetIndex = neededMemoryBlocksPtr->octetId;
            mergeReservationsArr[mergeSourceBank].isMarkedForMerge     = GT_TRUE;

            octetReservedLines[mergeSourceOctet] = bestSumOfSourceLines;

            neededMemoryBlocksPtr->bankIndexForMergeArr[i]                 = mergeSourceBank;
            neededMemoryBlocksPtr->octetIndexForMergeArr[i]                = mergeSourceOctet;
            neededMemoryBlocksPtr->mergeOperationUsefulForDefragArr[i]     = mergeBank;
            neededMemoryBlocksPtr->mergeOperationUsefulForDefragGlobalFlag = GT_TRUE;
            bankReserveIndexes[i] = mergeSourceBank;

        }
        else
        {
            *mergeBankCanBeFoundPtr = GT_FALSE;
            neededMemoryBlocksPtr->mergeOperationUsefulForDefragGlobalFlag = GT_TRUE;
            neededMemoryBlocksPtr->mergeOperationUsefulForDefragArr[i] = GT_TRUE;
            neededMemoryBlocksPtr->octetIndexForMergeArr[i] = PRV_CPSS_DXCH_SIP6_LPM_RAM_MERGE_NOT_CHECKED;
            neededMemoryBlocksPtr->bankIndexForMergeArr[i] = PRV_CPSS_DXCH_SIP6_LPM_RAM_MERGE_NOT_CHECKED;

#if 0
            /* Debug prints */
            prvCpssDxChIpLpmDbgLastNeededMemInfoPrint(shadowPtr);
            cpssDxChIpLpmDbgHwBlockInfoPrint(0);
#endif
            return GT_OK;
        }
    }

    if (neededMemoryBlocksPtr->mergeOperationUsefulForDefragGlobalFlag == GT_TRUE)
    {
        *mergeBankCanBeFoundPtr = GT_TRUE;
    }
    else
    {
#if 0
        /* Debug prints */
        prvCpssDxChIpLpmDbgLastNeededMemInfoPrint(shadowPtr);
        cpssDxChIpLpmDbgHwBlockInfoPrint(0);
#endif
        *mergeBankCanBeFoundPtr = GT_FALSE;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmSip6RamMngMoveBlockToNewLocation function
* @endinternal
*
* @brief   Move one block from the bank we need to release to one of the other
*         banks mapped to a given octet
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] lpmMemInfoPtrPtr         - (pointer to) the lpm memory linked list holding
*                                      all blocked mapped to this octetIndex
* @param[in] octetIndexForMerge       - octet index of the bucket we are working on
* @param[in] bankIndexForMerge        - the index of the bank to merge
* @param[in] isMarkedForShrinkArr     - Array to check if bank should be ignored for merge
*                                       as it is used for shrink.
* @param[in] blockHandleAddrForMerge  - the block address to be moved
* @param[in] parentUpdateParams       - parameters needed for parent update
* @param[out] bucketRelocatedPtr       - (pointer to) GT_TRUE: the bucket was relocated
*                                      GT_FALSE: the bucket was NOT relocated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssDxChLpmSip6RamMngMoveBlockToNewLocation
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      **lpmMemInfoPtrPtr,
    IN GT_U32                                  octetIndexForMerge,
    IN GT_U32                                  bankIndexForMerge,
    IN GT_BOOL                                 *isMarkedForShrinkArr,
    IN GT_UINTPTR                              blockHandleAddrForMerge,
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC   *parentUpdateParams,
    OUT GT_BOOL                                *bucketRelocatedPtr
)
{
    GT_STATUS                               retVal=GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      *tempLpmMemInfoPtr;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *bucketShadowToMovePtr;/* bucket to be merged in SW representation*/
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    bucketShadowToMoveProtocol;/* the protocol of the bucket we are moving */
    GT_U32                                  memSizeToMove;         /* bucket size to be merged*/
    GT_U32                                  memBlockBase=0;
    GT_UINTPTR                              newHwBucketOffsetHandle = 0;
    GT_BOOL                                 needToFreeAllocationInCaseOfFail = GT_FALSE;
    GT_UINTPTR                              tempHwAddrHandleToBeFreed = 0;
    GT_U32                                  blockIndex = 0;
    GT_U32                                  bankIndex = 0;
    GT_U32                                  tempAddr = 0;     /* Temporary address */
    PRV_CPSS_DXCH_LPM_ALLOC_TYPE_ENT        ucMcType=PRV_CPSS_DXCH_LPM_ALLOC_UC_TYPE_E;/* indicates whether bucketPtr is the uc,
                                                                                          mc src tree, mc group bucket */
    GT_U32                                  oldMemSizeToMove = 0;
    GT_U32                                  oldMemGonIndexToMove = 0;
    GT_U32                                  i;

    GT_BOOL                                 useShrink = GT_FALSE;
    GT_BOOL                                 useSwapArea = GT_FALSE;
    GT_U32                                  bankToShrink=0;
    GT_U32                                  sumOfFreeBlockAfterShrink=0;
    GT_UINTPTR                              oldHandleAddrUsedForShrinkArr[2]={0}; /* up to 2 swaps area used for old handle in srink operation */
    GT_U32                                  oldHwHandleGonIndexArr[2]={0};
    GT_BOOL                                 swapUsefullUsedForShrinkArr[2]={GT_FALSE};   /* up to 2 swaps area used for old handle in srink operation */

    bucketShadowToMovePtr = (PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *)GET_DMM_BUCKET_SW_ADDRESS(blockHandleAddrForMerge);
    memSizeToMove         = SIZE_IN_WORDS(blockHandleAddrForMerge);
    bucketShadowToMoveProtocol   = GET_DMM_BLOCK_PROTOCOL(blockHandleAddrForMerge);

    *bucketRelocatedPtr = GT_FALSE;
    useShrink = GT_FALSE;

    /* Check if the block can be relocated in naive method in first try.
       If it doesn't work, use shrink and try relocating in the second try*/
    for (i = 0; i < 2; i++)
    {
        tempLpmMemInfoPtr = lpmMemInfoPtrPtr[octetIndexForMerge];

        /* try to relocate the naive way - scan the list by the order it appears */
        while ((tempLpmMemInfoPtr!=NULL) && (newHwBucketOffsetHandle==0))
        {
            /* skip the bank that we are going to release and bank marked for shrink */
            if ((tempLpmMemInfoPtr->structsMemPool == shadowPtr->lpmRamStructsMemPoolPtr[bankIndexForMerge]) ||
                (isMarkedForShrinkArr[tempLpmMemInfoPtr->ramIndex] == GT_TRUE))
            {
                tempLpmMemInfoPtr = tempLpmMemInfoPtr->nextMemInfoPtr;
                continue;/* try next bank */
            }

            if (useShrink == GT_TRUE)
            {
                /* look for the smallest (which is big enough) free space
                   in bank to hold relocated bucket - it is done by the DMM allocation algorithm
                   in the call to prvCpssDxChLpmRamMngMoveBlockToNewLocation we already found that a simple allocation
                   will not succeed so now we need to shrink the bank before allocation */

                /* check if shrink can be usefull */
                sumOfFreeBlockAfterShrink = prvCpssDmmCheckIfShrinkOperationUsefulForDefrag(tempLpmMemInfoPtr->structsMemPool,
                                                                                            memSizeToMove,
                                                                                            0, /* oldBlockSize = 0, we do not have any old block to move */
                                                                                            &useSwapArea);

                if (sumOfFreeBlockAfterShrink>0)
                {
                    /* shrink the Bank we found in previous phase */
                    bankToShrink = tempLpmMemInfoPtr->ramIndex;

                    retVal = prvCpssDxChLpmSip6RamMngShrinkBank(shadowPtr,
                                                                bankToShrink,
                                                                oldHandleAddrUsedForShrinkArr,
                                                                oldHwHandleGonIndexArr,
                                                                memSizeToMove,
                                                                swapUsefullUsedForShrinkArr,
                                                                parentUpdateParams);
                    if (retVal!=GT_OK)
                    {
                        break;
                    }
                }
                else
                {
                    tempLpmMemInfoPtr = tempLpmMemInfoPtr->nextMemInfoPtr;
                    continue;
                }
            }
            /* look for the smallest (which is big enough) free space
               in bank to hold relocated bucket - it is done by the DMM allocation algorithm */
             memBlockBase = tempLpmMemInfoPtr->structsBase;
             bankIndex = tempLpmMemInfoPtr->ramIndex;
             newHwBucketOffsetHandle = prvCpssDmmAllocate(tempLpmMemInfoPtr->structsMemPool,
                                                    DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * memSizeToMove,
                                                    DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);
             if ((newHwBucketOffsetHandle == DMM_BLOCK_NOT_FOUND)||(newHwBucketOffsetHandle == DMM_MALLOC_FAIL))
             {
                 if (useShrink == GT_TRUE)
                 {
                     /* This case should never happen */
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "error in prvCpssDxChLpmSip6RamMngMoveBlockToNewLocation (useShrink = GT_TRUE)\n");
                 }
                 /* continue looking in next bank - we did not find what we needed in current bank */
                 newHwBucketOffsetHandle = 0;
             }
             tempLpmMemInfoPtr = tempLpmMemInfoPtr->nextMemInfoPtr;
        }
        if (newHwBucketOffsetHandle != 0)
        {
            break;
        }
        else
        {
            useShrink = GT_TRUE;
        }
    }

    /* if free space was found then we move the bucket to
       its new location and update all related pointers */
    if(newHwBucketOffsetHandle != 0)
    {
        /* keep new tempHwAddrHandle -- in case of a fail should be freed */
        needToFreeAllocationInCaseOfFail = GT_TRUE;
        tempHwAddrHandleToBeFreed = newHwBucketOffsetHandle;

        /*  set pending flag for future need */
        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(newHwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
        retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
        if (retVal != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
        }
        if (blockIndex != bankIndex)
        {
            /* we should be working on the bankIndex we got as parameter to the function */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngMoveBlockToNewLocation - GT_FAIL \n");
        }

        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc = GT_TRUE;
        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(newHwBucketOffsetHandle);

        tempAddr = (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(newHwBucketOffsetHandle)) + memBlockBase;

        /* reset OUT parameters */
        oldMemSizeToMove = 0;
        oldMemGonIndexToMove=0;

        retVal = findBlockSizeAndIndexToMove((GT_DMM_BLOCK *)blockHandleAddrForMerge, &oldMemSizeToMove, &oldMemGonIndexToMove);
        if (retVal!=GT_OK)
        {
            /* This case should never happen */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in findBlockSizeAndIndexToMove - GT_FAIL \n");
        }

        if (!((oldMemGonIndexToMove < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS) || (oldMemGonIndexToMove == 0xFFFF)))
        {
            /* This shouldn't happen */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in gonIndex - GT_FAIL \n");
        }

        retVal = updateParentSwapGonsAdresses(parentUpdateParams, tempAddr, oldMemGonIndexToMove);/* pointers will be updated to point to new allocated memory area */
        if (retVal!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"error in prvCpssDxChLpmRamMngMoveBlockToNewLocation - updateSwapGonsAdresses - GT_FAIL \n");
        }

        /* oldBucketShadowPtr will be used to build the HW from SW shadow and for
           updated pointers when we move the HW bucket location */
        if ((oldMemGonIndexToMove == 0xFFFF) && (bucketShadowToMovePtr->pointingRangeMemAddr!=0xFFFFFFFF))
        {
            /* this is SRC Root
               all other cases are treated same as UC bucket */
            ucMcType = PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E;
        }

        retVal = prvCpssDxChLpmSip6RamMngShrinkHwWriteAndPointersUpdatePhase1Phase2(shadowPtr,
                                                    tempAddr,
                                                    bucketShadowToMovePtr,
                                                    oldMemGonIndexToMove,
                                                    needToFreeAllocationInCaseOfFail,
                                                    tempHwAddrHandleToBeFreed,
                                                    parentUpdateParams,
                                                    PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE1_E,
                                                    ucMcType);
        if (retVal!=GT_OK)
        {
           /* in case of a fail any allocations done in previous stages was already
              free in prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkHwWriteAndPointersUpdatePhase1Phase2 - GT_FAIL \n");
        }

        if (oldMemGonIndexToMove < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS)
        {
            /* ok since the bucket is updated in the HW, we can now update the shadow
               but first we need to set the old block as a blocked that was moved to a new location */
            SET_DMM_BLOCK_WAS_MOVED(bucketShadowToMovePtr->hwGroupOffsetHandle[oldMemGonIndexToMove], 1);
            bucketShadowToMovePtr->hwGroupOffsetHandle[oldMemGonIndexToMove] = newHwBucketOffsetHandle;

           SET_DMM_BLOCK_PROTOCOL(bucketShadowToMovePtr->hwGroupOffsetHandle[oldMemGonIndexToMove], bucketShadowToMoveProtocol);
           SET_DMM_BUCKET_SW_ADDRESS(bucketShadowToMovePtr->hwGroupOffsetHandle[oldMemGonIndexToMove], bucketShadowToMovePtr);
        }
        else if (oldMemGonIndexToMove == 0xFFFF)
        {
            /* This is root bucket */
            if (bucketShadowToMovePtr->hwBucketOffsetHandle == 0)
            {
                /* This shouldn't happen */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in root bucket offset handle - GT_FAIL \n");
            }
            SET_DMM_BLOCK_WAS_MOVED(bucketShadowToMovePtr->hwBucketOffsetHandle, 1);
            bucketShadowToMovePtr->hwBucketOffsetHandle = newHwBucketOffsetHandle;

            SET_DMM_BLOCK_PROTOCOL(bucketShadowToMovePtr->hwBucketOffsetHandle, bucketShadowToMoveProtocol);
            SET_DMM_BUCKET_SW_ADDRESS(bucketShadowToMovePtr->hwBucketOffsetHandle, bucketShadowToMovePtr);
        }
        else
        {
            /* This shouldn't happen */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in gonIndex - GT_FAIL \n");
        }

        retVal = prvCpssDxChLpmSip6RamMngShrinkPointersUpdate(shadowPtr,
                                                              tempAddr,
                                                              bucketShadowToMovePtr,
                                                              oldMemGonIndexToMove,
                                                              needToFreeAllocationInCaseOfFail,
                                                              tempHwAddrHandleToBeFreed);
        if (retVal!=GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmSip6RamMngShrinkPointersUpdate - GT_FAIL \n");
        }

        /* now it's ok to free this bucket's old memory */
        retVal = prvCpssDxChLpmSip6RamMngShrinkFreeOldBucket(shadowPtr,
                                                             bankIndexForMerge,
                                                             blockHandleAddrForMerge,
                                                             bucketShadowToMovePtr);
        if (retVal!=GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngMoveBlockToNewLocation - GT_FAIL \n");
        }

        *bucketRelocatedPtr = GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmSip6RamMngMergeBank function
* @endinternal
*
* @brief   Merge one bank to the rest of the banks mapped to a given octet
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] lpmMemInfoPtrPtr         - (pointer to) the lpm memory linked list holding
*                                       all blocked mapped to this octetIndex
* @param[in] protocol                 - the protocol
* @param[in] octetIndexForMerge       - octet index of the bucket we are working on
* @param[in] bankIndexForMerge        - the index of the bank to merge
* @param[in] isMarkedForShrinkArr     - Array to check if bank should be ignored for merge
*                                       as it is used for shrink.
* @param[in] parentUpdateParams       - parameters needed for parent update
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - merge operation fail
*/
GT_STATUS prvCpssDxChLpmSip6RamMngMergeBank
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           **lpmMemInfoPtrPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol,
    IN GT_U32                                       octetIndexForMerge,
    IN GT_U32                                       bankIndexForMerge,
    IN GT_BOOL                                      *isMarkedForShrinkArr,
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC        *parentUpdateParams
)
{
    GT_STATUS                          retVal = GT_OK;
    GT_STATUS                          retVal1 = GT_OK;
    GT_DMM_PARTITION                   *partition;
    GT_UINTPTR                         partitionId;
    GT_U32                             relocatedBucketSize=0;  /* the size of the buket we are going to move */
    GT_DMM_SORTED_PARTITION_ELEMENT    *tempElemList;          /* temporary list to be used when running over
                                                                  a list of buckets to be moved */
    GT_BOOL                            bucketRelocated = GT_FALSE;/* flag specify success of relocate operation */
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempNextMemInfoPtr;/* use for going over the list of blocks per octet */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempElemToFreePtr;   /* temp pointer used for free operation */

   /* Create a sorted list of all buckets at the bank we need to release (only of relevant protocol),
      motivation is that later on when we will try to add them to the other banks one by one starting
      from the biggest to smallest - this will increase chances to success.*/
    partitionId = shadowPtr->lpmRamStructsMemPoolPtr[bankIndexForMerge];
    partition = (GT_DMM_PARTITION *)shadowPtr->lpmRamStructsMemPoolPtr[bankIndexForMerge];

    retVal = prvCpssDmmSortPartition(partitionId,(GT_U32)protocol);
    if (retVal!=GT_OK)
    {
        prvCpssDmmFreeSortedPartitionArray(partitionId);
        return retVal;
    }

    /* save values pendingBlockToUpdateArr before the merge */
    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));
    cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
    /* reset shadowPtr->pendingBlockToUpdateArr */
    cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

    /* Run all over the buckets that need to be relocated from biggest to smallest */
    for (relocatedBucketSize = DMM_MAXIMAL_BLOCK_SIZE_ALLOWED; relocatedBucketSize > 0; relocatedBucketSize--)
    {
        /*check if we have a bank of relocatedBucketSize to be moved */
        tempElemList = partition->tableOfSortedUsedBlocksPointers[relocatedBucketSize];

        /* go over all element of relocatedBucketSize and move them */
        while (tempElemList != NULL)
        {
            /* move the element to its new location */
            retVal = prvCpssDxChLpmSip6RamMngMoveBlockToNewLocation(shadowPtr,
                                                                    lpmMemInfoPtrPtr,
                                                                    octetIndexForMerge,
                                                                    bankIndexForMerge,
                                                                    isMarkedForShrinkArr,
                                                                    (GT_UINTPTR)tempElemList->blockElementPtr,/* the block to be moved */
                                                                    parentUpdateParams,
                                                                    &bucketRelocated);
            if (retVal!=GT_OK)
            {
                prvCpssDmmFreeSortedPartitionArray(partitionId);
                /* in case of merge we should instantly update the block usage counters
                  since it is a permanent operation that will not be reverted in case
                  of a fail.
                  For example in MC case we need the counters to be updated corectly after
                  the Src addition and not after Src+Grp finish succesfuly */
                prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                   shadowPtr->pendingBlockToUpdateArr,
                                                                   shadowPtr->protocolCountersPerBlockArr,
                                                                   shadowPtr->pendingBlockToUpdateArr,
                                                                   protocol,
                                                                   shadowPtr->numOfLpmMemories);

                /* put back the values kept in pendingBlockToUpdateArr before the merge */
                cpssOsMemCpy(shadowPtr->pendingBlockToUpdateArr,tempPendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"error in prvCpssDxChLpmRamMngMergeBank - GT_FAIL \n");
            }

            if (bucketRelocated==GT_TRUE)
            {
                /* in case of merge we should instantly update the block usage counters
                  since it is a permanent operation that will not be reverted in case
                  of a fail.*/
                retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                   shadowPtr->pendingBlockToUpdateArr,
                                                                   shadowPtr->protocolCountersPerBlockArr,
                                                                   shadowPtr->pendingBlockToUpdateArr,
                                                                   protocol,
                                                                   shadowPtr->numOfLpmMemories);
                if (retVal1!=GT_OK)
                {
                    /* put back the values kept in pendingBlockToUpdateArr before the merge */
                    cpssOsMemCpy(shadowPtr->pendingBlockToUpdateArr,tempPendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
                    CPSS_LOG_ERROR_AND_RETURN_MAC(retVal1,"error in prvCpssDxChLpmRamUpdateBlockUsageCounters - GT_FAIL \n");
                }

                /* go to next element with (size = relocatedBucketSize) to be moved */
                tempElemList = tempElemList->nextSortedElemPtr;
            }
            else/* bucketRelocated==GT_FALSE*/
            {
                /* even with shrink we do not succeed in relocating the block - this is a fail */
                /* if cannot relocate bucket even with shrink it means the merge failed */
                prvCpssDmmFreeSortedPartitionArray(partitionId);

                /* put back the values kept in pendingBlockToUpdateArr before the merge */
                cpssOsMemCpy(shadowPtr->pendingBlockToUpdateArr,tempPendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM, "error in prvCpssDxChLpmRamMngMergeBank - GT_OUT_OF_PP_MEM \n");
            }
        }
    }

    /* if we reached here it means the selected bank was merged to other
       banks related to the selected octet.
       Now the released bank can be used by another octet
       We should mark this bank as not used anymore by the given protocol */
    /*  clear block mapping for the given protocol */
    PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_CLEAR_MAC(shadowPtr, protocol, octetIndexForMerge, bankIndexForMerge);
    /* the blocks that was freed should stay free even if we get an error in the next phases so we update
       tempLpmRamOctetsToBlockMappingUsedForReconstractPtr to be used in case reconstruct is needed,*/
    PRV_CPSS_DXCH_TEMP_LPM_RAM_OCTET_TO_BLOCK_MAPPING_CLEAR_MAC(shadowPtr, protocol, octetIndexForMerge, bankIndexForMerge);

    /* if the block is not used by any protocol set it as not used */
    if(((PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,bankIndexForMerge)==GT_FALSE))&&
        ((PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,bankIndexForMerge)==GT_FALSE))&&
        ((PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,bankIndexForMerge)==GT_FALSE)))
    {
        shadowPtr->lpmRamOctetsToBlockMappingPtr[bankIndexForMerge].isBlockUsed = GT_FALSE;
        shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr[bankIndexForMerge].isBlockUsed = GT_FALSE;
    }


    /* need to remove the bankIndexForMerge from its current location in the linked list */
    tempNextMemInfoPtr = lpmMemInfoPtrPtr[octetIndexForMerge];

    if (tempNextMemInfoPtr!=NULL)/* the list is not empty */
    {
        if(tempNextMemInfoPtr->nextMemInfoPtr==NULL)/* this is an allocation of the root */
        {
            if (tempNextMemInfoPtr->ramIndex==bankIndexForMerge)
            {
                /* just reset the values */
                tempNextMemInfoPtr->ramIndex=0;
                tempNextMemInfoPtr->structsBase=0;
                tempNextMemInfoPtr->structsMemPool=0;
            }
            else
            {
                /* should never happen or it is a scenario we haven't thought about */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            /* the bankIndexForMerge is located as the first element in the linked list */
            if (tempNextMemInfoPtr->ramIndex==bankIndexForMerge)
            {
                /* copy the values from the next element to the root  */
                tempElemToFreePtr = tempNextMemInfoPtr->nextMemInfoPtr;

                tempNextMemInfoPtr->ramIndex=tempNextMemInfoPtr->nextMemInfoPtr->ramIndex;
                tempNextMemInfoPtr->structsBase=tempNextMemInfoPtr->nextMemInfoPtr->structsBase;
                tempNextMemInfoPtr->structsMemPool=tempNextMemInfoPtr->nextMemInfoPtr->structsMemPool;

                tempNextMemInfoPtr->nextMemInfoPtr=tempNextMemInfoPtr->nextMemInfoPtr->nextMemInfoPtr;
                cpssOsFree(tempElemToFreePtr);
            }
            else
            {
                /* this is an allocation of an element in the linked list
                   need to free the elemen and remove it from the linked list
                   (update the father pointers) */
                while(tempNextMemInfoPtr->nextMemInfoPtr != NULL)
                {
                    if (tempNextMemInfoPtr->nextMemInfoPtr->ramIndex==bankIndexForMerge)
                    {
                        /* free the element */
                         tempElemToFreePtr = tempNextMemInfoPtr->nextMemInfoPtr;

                         tempNextMemInfoPtr->nextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr->nextMemInfoPtr;
                         cpssOsFree(tempElemToFreePtr);
                         break;
                    }
                    tempNextMemInfoPtr=tempNextMemInfoPtr->nextMemInfoPtr;
                }
            }
        }
    }
    else
    {
        /* should never happen or it is a scenario we haven’t thought about */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* reset sorted partition list */
    prvCpssDmmFreeSortedPartitionArray(partitionId);

    /* in case of merge we should instantly update the block usage counters
       since it is a permanent operation that will not be reverted in case
       of a fail.
       For example in MC case we need the counters to be updated corectly after
       the Src addition and not after Src+Grp finish succesfuly */
     retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                        shadowPtr->pendingBlockToUpdateArr,
                                                        shadowPtr->protocolCountersPerBlockArr,
                                                        shadowPtr->pendingBlockToUpdateArr,
                                                        protocol,
                                                        shadowPtr->numOfLpmMemories);
     if (retVal1!=GT_OK)
     {
         /* put back the values kept in pendingBlockToUpdateArr before the merge */
         cpssOsMemCpy(shadowPtr->pendingBlockToUpdateArr,tempPendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
         CPSS_LOG_ERROR_AND_RETURN_MAC(retVal1,"error in prvCpssDxChLpmRamMngMergeBank - GT_FAIL \n");
     }

     if (retVal1!=GT_OK)
     {
         /* put back the values kept in pendingBlockToUpdateArr before the merge */
         cpssOsMemCpy(shadowPtr->pendingBlockToUpdateArr,tempPendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
         CPSS_LOG_ERROR_AND_RETURN_MAC(retVal1,"error in prvCpssDxChLpmRamMngMergeBank - GT_FAIL \n");
     }

     /* put back the values kept in pendingBlockToUpdateArr before the merge */
    cpssOsMemCpy(shadowPtr->pendingBlockToUpdateArr,tempPendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmSip6RamMngMergeAllGonBanks function
* @endinternal
*
* @brief   Merge one bank to the rest of the banks mapped to a given octet
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] lpmMemInfoPtrPtr         - (pointer to) the lpm memory linked list holding
*                                      all blocked mapped to this octetIndex
* @param[in] protocol                 - the protocol
* @param[in] neededMemoryBlocksPtr    - (pointer to) memory data needed for
*                                       shrink as:
*                                       1.the old bucket handle, if the value is
*                                         0, it means this is a new bucket, no
*                                         resize
*                                       2.the size (in lines) of the new bucket
*                                       3.the index of the bank to shrink
*                                       4.indication if the swap are is in use
*                                         GT_TRUE-swapArea will be used when
*                                         doing shrink
*                                         GT_FALSE - swapArea will NOT be used
*                                         when doing shrink
* @param[in] isMarkedForShrinkArr     - Array to check if bank should be ignored for merge
*                                       as it is used for shrink.
* @param[in] parentUpdateParams       - parameters needed for parent update
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - merge operation fail
*/
GT_STATUS prvCpssDxChLpmSip6RamMngMergeAllGonBanks
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC        **lpmMemInfoPtrPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol,
    IN PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC *neededMemoryBlocksPtr,
    IN GT_BOOL                                   *isMarkedForShrinkArr,
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC     *parentUpdateParams
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i;
    GT_BOOL   isBlockMerged[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS] = {GT_FALSE};

    if (neededMemoryBlocksPtr->mergeOperationUsefulForDefragGlobalFlag == GT_FALSE)
    {
        return GT_OK;
    }

    for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
    {
        if ((neededMemoryBlocksPtr->octetIndexForMergeArr[i] >= PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_MAX_CNS) ||
            (neededMemoryBlocksPtr->bankIndexForMergeArr[i] >= PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS))
        {
            /* This should not happen. Possible bug */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Octet or Bank out of range\n");
        }

        if ((neededMemoryBlocksPtr->mergeOperationUsefulForDefragArr[i] == GT_TRUE) &&
            (isBlockMerged[neededMemoryBlocksPtr->bankIndexForMergeArr[i]] == GT_FALSE))
        {
            rc = prvCpssDxChLpmSip6RamMngMergeBank(shadowPtr, lpmMemInfoPtrPtr, protocol,
                                                   neededMemoryBlocksPtr->octetIndexForMergeArr[i],
                                                   neededMemoryBlocksPtr->bankIndexForMergeArr[i],
                                                   isMarkedForShrinkArr,
                                                   parentUpdateParams);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChLpmSip6RamMngMergeBank - GT_FAIL \n");
            }
            isBlockMerged[neededMemoryBlocksPtr->bankIndexForMergeArr[i]] = GT_TRUE;
        }
    }
    neededMemoryBlocksPtr->mergeOperationUsefulForDefragGlobalFlag = GT_FALSE;
    return rc;
}
