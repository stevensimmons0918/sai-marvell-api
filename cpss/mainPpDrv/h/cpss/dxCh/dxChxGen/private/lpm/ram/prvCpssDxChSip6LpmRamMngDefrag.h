/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChSip6LpmRamMngDefrag.h
*
* DESCRIPTION:
*       Definitions of the SIP6 LPM Defrag algorithm
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __prvCpssDxChSip6LpmMngDefragh
#define __prvCpssDxChSip6LpmMngDefragh

#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*#define LPM_DEBUG*/
#ifdef PRESTERA_DEBUG
#define LPM_DEBUG
#endif /* PRESTERA_DEBUG */

#define PRV_CPSS_DXCH_SIP6_LPM_RAM_MERGE_NOT_CHECKED 0xFFFFFFFF


typedef struct PRV_CPSS_SIP6_LPM_MERGE_RESERVATIONS_STCT
{
    GT_U8   octetIdx;
    GT_U8   reassignedOctetIndex;
    GT_BOOL isMarkedForMerge;
    GT_U32  sourceLines;
    GT_U32  freeLines;
} PRV_CPSS_SIP6_LPM_MERGE_RESERVATIONS_STC;

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
    OUT GT_U32                                       freeSpaceTakenFromBlockIndexArray[]
);
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
);
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
* @param[in] parentUpdateParams       - parameters needed for parent update
* @param[in] phaseFlag                - GT_TRUE: phase1 update
*                                      GT_FALSE: phase2 update
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

);
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
);

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
);

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
* @param[in] parentUpdateParams      - parameters needed for parent update
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
);

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
);

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
* @param[in]  parentUpdateParams      - parameters needed for parent update
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
);

/**
* @internal prvCpssDxChLpmSip6RamMngShrinkBank function
* @endinternal
*
* @brief   Shrink the partition
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] lpmMemInfoPtrPtr         (pointer to) the lpm memory linked list holding
*                                      all blocked mapped
* @param[in] bankIndex                - the index of the bank to shrink
* @param[in] oldHwHandleAddr          - the old HW bucket handle, if the value is 0,
*                                      it is a new bucket, no resize
* @param[in] newBucketSize            - the size (in lines) of the new bucket
* @param[in] useSwapArea              - GT_TRUE  - swapArea will be used when doing shrink
*                                      GT_FALSE - swapArea will NOT be used when doing shrink
* @param[in] firstFreeBlockArr        - array of all free blocks found by shrink
* @param[in] parentUpdateParams       - parameters needed for parent update
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
);

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
    IN  PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC           *neededMemoryBlocksPtr,
    IN GT_U32                                               freeSpaceTakenFromBlockIndexArray[],
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC                *parentUpdateParams
);

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
    IN  GT_UINTPTR                                                  *hwGroupOffsetHandle,
    IN GT_U32                                                       bankIndex,
    IN GT_U32                                                       newBucketSize,
    IN GT_BOOL                                                      useSwapArea,
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC                        *parentUpdateParams,
    IN GT_BOOL                                                      *needToFreeAllocationInCaseOfFailPtr,
    IN GT_UINTPTR                                                   *tempHwAddrHandleToBeFreedPtr
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChSip6LpmMngDefragh */

