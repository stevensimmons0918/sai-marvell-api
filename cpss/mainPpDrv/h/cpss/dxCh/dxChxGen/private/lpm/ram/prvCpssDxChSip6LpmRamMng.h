/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChSip6LpmRamMng.h
*
* DESCRIPTION:
*       Definitions of the SIP6 LPM algorithm
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvCpssDxChSip6LpmMngh
#define __prvCpssDxChSip6LpmMngh

#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*#define LPM_DEBUG*/
#ifdef PRESTERA_DEBUG
#define LPM_DEBUG
#endif /* PRESTERA_DEBUG */

/* all GONs swap index */
#define PRV_CPSS_DXCH_LPM_RAM_ALL_GONS_SWAP_INDEX_CNS  7

/*
 * Typedef: struct LPM_RAM_COMPRESSED_NODE_DATA_STC
 *
 * Description: Contain data to cacculate node hw address.
 *
 * Fields:
 *      nodePtr              - software node shadow
 *      isMcRangeExist       - if mc range exist in node
 *      mcRanges             - pointers to mc ranges
 *      mcLeavesOrderIdArray - array incorporated order number of mc leave inside gon or embedded compress node
 *      mcLeavesNumber       - number of mc leaves in node
 */
typedef struct
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *nodePtr;
    GT_BOOL                                   isMcRangeExist;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    *mcRanges[PRV_CPSS_DXCH_LPM_MAX_LEAVES_IN_EMBEDDED_3_NODE_E];
    GT_U32                                    mcLeavesOrderIdArray[PRV_CPSS_DXCH_LPM_MAX_LEAVES_IN_EMBEDDED_3_NODE_E];
    GT_U32                                    mcLeavesNumber;
    GT_BOOL                                   fullEmbeddedLeaf;
} LPM_RAM_COMPRESSED_NODE_DATA_STC;

/*
 * Typedef: enum LPM_ROOT_BUCKET_UPDATE_ENT
 *
 * Description:
 *      Indicates what kind of update the root bucket will pass.
 *
 *
 * Fields:
 *      LPM_ROOT_BUCKET_UPDATE_NONE_E  - updates nothing.
 *      LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E     - updates the HW in new mem alloc.
 *      LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_SRC_RESIZE_E - updates the src HW in new mem alloc.
 *      LPM_ROOT_BUCKET_UPDATE_HW_E                    - updates the HW.
 */
typedef enum
{
    LPM_ROOT_BUCKET_UPDATE_NONE_E = 0,
    LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E,
    LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_SRC_RESIZE_E,
    LPM_ROOT_BUCKET_UPDATE_HW_E
}LPM_ROOT_BUCKET_UPDATE_ENT;

/*
 * Typedef: struct LPM_RAM_AFFECTED_BV_LINES_STC
 *
 * Description: shows affected bit vector lines.
 *
 * Fields:
 *      bvLinesId       - bit vector lines array
 */
typedef struct
{
    GT_BOOL bvLinesArray[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];
} LPM_RAM_AFFECTED_BV_LINES_STC;

/**
* @internal updateSwapArea function
* @endinternal
*
* @brief This function fill SWAP area in case of LPM memory shortage
*
* @param[in]      bvLineIndex           - bit vector line index
* @param[in]      hwGroupOffsetHandle   - array of LPM memory handles
* @param[in]      swapSavingType        - describes of data type saving in SWAP
* @param[in]      swapOffset            - given swap memory offset
* @param[in]      shareDevsList         - pointer to device list
* @param[in]      shareDevsListLen      - devices length
*
* @param[out]      newGonsOffsets        - array of GONs offsets in LPM memory
*
* @retval       GT_OK on success, or
* @retval       GT_OUT_OF_CPU_MEM - if failed to allocate CPU memory, or
* @retval       GT_OUT_OF_PP_MEM - if failed to allocate PP memory.
*/
GT_STATUS updateSwapArea
(
    IN GT_U32                                          bvLineIndex,
    IN GT_UINTPTR                                      hwGroupOffsetHandle[],
    IN GT_UINTPTR                                      hwRootNodeOffsetHandle,
    IN GT_U32                                          swapSavingType,
    IN GT_U32                                          swapOffset,
    IN GT_U8                                           *shareDevsList,
    IN GT_U32                                          shareDevsListLen,
    OUT GT_U32                                         newGonsOffsets[]
);

/**
* @internal lpmFalconFillRegularData function
* @endinternal
*
* @brief This function scan bucket shadow and fill regularNodesArray struct.
*
* @param[in] bucketPtr                         - pointer to  bucket
* @param[in] lpmRamTotalBlocksSizeIncludingGap - lpm total block size
* @param[out] regularNodesArrayPtr  - array saving regular nodes info
*
* @retval   GT_OK on success, or
* @retval   GT_FAIL - otherwise
*/
GT_STATUS lpmFalconFillRegularData
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *bucketPtr,
    IN  GT_U32                                      lpmRamTotalBlocksSizeIncludingGap,
    OUT PRV_CPSS_DXCH_LPM_REGULAR_STC               *regularNodesArrayPtr
);

/**
* @internal lpmFalconFillCompressedData function
* @endinternal
*
* @brief This function scan bucket shadow and fill compressedNodesArray struct.
*
* @param[in] bucketPtr                         - pointer to  bucket
* @param[in] lpmRamTotalBlocksSizeIncludingGap - lpm total block size
* @param[out] compressedNodesArrayPtr  - array saving compressed nodes info
* @param[out] compressedNodesData      - MC data collection in order to create
*                                         a match between group and source
*
* @retval   GT_OK on success, or
* @retval   GT_FAIL - otherwise
*
*/
GT_STATUS lpmFalconFillCompressedData
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *bucketPtr,
    IN  GT_U32                                      lpmRamTotalBlocksSizeIncludingGap,
    OUT PRV_CPSS_DXCH_LPM_COMPRESSED_STC            *compressedNodesArrayPtr,
    OUT LPM_RAM_COMPRESSED_NODE_DATA_STC            *compressedNodesData
);

/**
* @internal lpmFalconGetHwNodeOffsetInsideGroupOfNodes funtion
* @endinternal
*
* @brief This function
*        the lpm structures in PP's memory.
*
* @param[in]      bucketPtr             - Pointer to the bucket on which needed range exist.
* @param[in]      rangeAddr             - start address of needed range.
* @param[in]      startSubNodeAddress   - start subnode address. (relevant for regular bucket)
* @param[in]      endSubNodeAddress     - end subnode address. (relevant for regular bucket)
* @param[in]      hwGroupOffset         - hw group offset
*                                         the ranges where the writeRangeInHw is set.
* @param[out]      nodeHwAddrPtr         - The hw offset of needed node inside group.
*
* @retval       GT_OK on success, or
* @retval       GT_FAIL - otherwise
*/
GT_STATUS lpmFalconGetHwNodeOffsetInsideGroupOfNodes
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    IN  GT_U32                                    rangeStartAddr,
    IN  GT_U8                                     startSubNodeAddress,
    IN  GT_U8                                     endSubNodeAddress,
    IN  GT_U32                                    hwGroupOffset,
    OUT GT_U32                                    *nodeHwAddrPtr
);

/**
* @internal getMirrorGroupOfNodesDataAndUpdateRangesAddress function
*
* @brief This function gets a the bucket's shadow data and formats accordingly
*        the bitvector,compressed and the next pointer array.
*
* @param[in]      bucketType            - bucket type
* @param[in]      startAddr             - start subnode address of needed range
* @param[in]      endAddr               - end subnode address of needed range
* @param[in]      rangePtr              - Pointer to the current bucket's first range.
* @param[in]      lpmEngineMemPtrPtr    - points to a an PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                               which holds all the memory information needed
*                               for where and how to allocate search memory.
* @param[in]      newBucketType         - the bucket's new type
* @param[in]      bucketBaseAddress     - the bucket's base address
* @param[in]      gonOffset             - the gon offset
* @param[out]      groupOfNodesPtr       - group of nodes contents
*
* @retval   GT_OK on success, or
* @retval   GT_FAIL - otherwise
*/
GT_STATUS getMirrorGroupOfNodesDataAndUpdateRangesAddress
(
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT             bucketType,
    IN  GT_U8                                           startAddress,
    IN  GT_U8                                           endAddress,
    IN  PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *rangePtr,
    IN  GT_U32                                          lpmRamTotalBlocksSizeIncludingGap,
    IN  GT_U32                                          gonOffset,
    OUT PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC      *groupOfNodesPtr
);

/**
* @internal findAndBindValidMemoryBlock function
* @endinternal
*
* @brief This function find a valid memory block and bind it to the list
*
* @param[in]  shadowPtr               - the shadow relevant for the devices asked to act on.
* @param[in]  protocol                - type of ip protocol stack to work on.
* @param[in]  octetIndex              - the octet we are working on
* @param[in]  neededMemoryBlocksPtr   - (pointer to)memory handler needed for octets allocation.
* @param[in]  neededMemoryBlocksSizes - memory sizes in lpm lines
* @param[in]  lastMemInfoPtr          - last pointer in list of blocks per octet to be used
* @param[in]  firstMemInfoInListToFreePtr-(pointer to) used for going over the list of blocks per octet
* @param[in]  justBindAndDontAllocateFreePoolMemPtr -(pointer to) GT_TRUE: no memory pool is bound to the root of the octet list,
*                                                    in this case no need to allocate a memory pool struct just
*                                                    to bind it to a free pool
*                                                    GT_FALSE:allocate a new elemenet block to the list
* @param[in]  allNewNextMemInfoAllocatedPerOctetArrayPtr - (pointer to) an array that holds for each octet
*                                                          the allocted new element that need to be freed.
*                                                          Size of the array is 17 for case of IPV6*
* @param[in]  memoryBlockTakenArr - 1:  the block was taken for the ADD operation
*                                   0: the block was NOT taken for the ADD operation
*                                   2,3: the block was taken in 3 octets per block mode
*
* @param[out] newStructsMemPool       - the new pool allocated
* @param[out] newFreeBlockIndexPtr    - (pointer to) index of a new free block bind to the list
*
* @retval   GT_OK on success, or
* @retval   GT_OUT_OF_CPU_MEM - if failed to allocate CPU memory, or
* @retval   GT_OUT_OF_PP_MEM - if failed to allocate PP memory.
*/
GT_STATUS findAndBindValidMemoryBlock
(
    IN      PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN      PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol,
    IN      GT_U32                                       octetIndex,
    INOUT   GT_UINTPTR                                   *neededMemoryBlocksPtr,
    IN      GT_U32                                       neededMemoryBlocksSizes,
    IN      PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           *lastMemInfoPtr,
    INOUT   PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           *firstMemInfoInListToFreePtr,
    IN      GT_BOOL                                      *justBindAndDontAllocateFreePoolMemPtr,
    INOUT   PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           **allNewNextMemInfoAllocatedPerOctetArrayPtr,
    OUT     GT_UINTPTR                                   newStructsMemPool,
    OUT     GT_U32                                       *newFreeBlockIndexPtr,
    IN      GT_U32                                       memoryBlockTakenArr[]
);

/**
* @internal updateHwRangeDataAndGonPtr function
* @endinternal
*
* @brief This function updates lpm node
*
* @param[in]  parentNodeDataPtr     - pointer to the bucket to parent node parameters.
*
* @retval   GT_OK on success, or
* @retval   GT_OUT_OF_CPU_MEM - if failed to allocate CPU memory, or
* @retval   GT_OUT_OF_PP_MEM - if failed to allocate PP memory.
*/
GT_STATUS updateHwRangeDataAndGonPtr
(
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC    *parentNodeDataPtr
);

/**
* @internal prvCpssDxChLpmSip6RamMngInsert function
* @endinternal
*
* @brief Insert a new entry to the LPM tables.
*
* @param[in]      bucketPtr         - Pointer to the root bucket.
* @param[in]      addrCurBytePtr    - The Byte Array represnting the Address associated
*                                     with the entry.
* @param[in]      prefix            - The address prefix.
* @param[in]      nextPtr           - A pointer to a next hop/next bucket entry.
* @param[in]      nextPtrType       - The type of nextPtr (can be next hop, ECMP/QoS entry
*                                     or bucket)
* @param[in]      rootBucketFlag    - Indicates the way to deal with a root bucket (if it is).
* @param[in]      lpmEngineMemPtrPtr- points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                     which holds all the memory information needed for where and
*                                     how to allocate search memory for each of the lpm levels
* @param[in]      updateOldPtr      - (GT_TRUE) this is an update for an already existing entry.
* @param[in]      bucketTypePtr     - The bucket type of the bucket inserted on the root.
*                                     Possible values:
*                                     REGULAR_BUCKET, COMPRESSED_1, or COMPRESSED_2.
* @param[in]      shadowPtr         - the shadow relevant for the devices asked to act on.
* @param[in]      insertMode        - is the function called in the course of Hot Sync,bulk
*                                     operation or regular
* @param[in]      ucMcType          - indicates whether bucketPtr is the uc, mc src tree, mc group bucket
* @param[in]      protocol          - the protocol
* @param[in]      parentUpdateParams- parameters needed for parent update
* @param[out]     updateOldPtr      - (GT_TRUE) this is an update for an already existing entry.
*                 bucketTypePtr     - The bucket type of the bucket inserted on the root.
*                                     Possible values:
*                                     REGULAR_BUCKET, COMPRESSED_1, or COMPRESSED_2.
*
* @retval   GT_OK on success, or
* @retval   GT_OUT_OF_CPU_MEM - if failed to allocate CPU memory, or
* @retval   GT_OUT_OF_PP_MEM - if failed to allocate PP memory.
*/
GT_STATUS prvCpssDxChLpmSip6RamMngInsert
(
    IN    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC       *bucketPtr,
    IN    GT_U8                                         *addrCurBytePtr,
    IN    GT_U32                                        prefix,
    IN    GT_PTR                                        nextPtr,
    IN    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT           nextPtrType,
    IN    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT    rootBucketFlag,
    IN    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC            **lpmEngineMemPtrPtr,
    INOUT GT_BOOL                                       *updateOldPtr,
    INOUT CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT           *bucketTypePtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC              *shadowPtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT    insertMode,
    IN    PRV_CPSS_DXCH_LPM_ALLOC_TYPE_ENT              ucMcType,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT          protocol,
    IN    PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC         *parentUpdateParams
);

/**
* @internal prvCpssDxChLpmSip6RamMngEntryDelete function
* @endinternal
*
* @brief  Delete an entry from the LPM tables.
*
* @param[in]  lpmPtr            - Pointer to the root bucket.
* @param[in]  addrCurBytePtr    - The Byte Array represnting the Address associated
*                                 with the entry.
* @param[in]  prefix            - The address prefix.
* @param[in]  rootBucketFlag    - Indicates the way to deal with a root bucket (if it is)
* @param[in]  updateHwAndMem    - whether an HW update and memory alloc should take place
* @param[in]  lpmEngineMemPtrPtr- points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                 which holds all the memory information needed for where and
*                                 how to allocate search memory for each of the lpm levels
* @param[in]  bucketTypePtr     - The bucket type of the root bucket after deletion.
*                                 Possible values:
*                                 REGULAR_BUCKET, COMPRESSED_1, COMPRESSED_2, or NEXT_HOP
* @param[in]  shadowPtr         - the shadow relevant for the devices asked to act on.
* @param[in]  ucMcType          - indicates whether bucketPtr is the uc, mc src or mc group.
* @param[in]  rollBack          - GT_TRUE: rollback is taking place
*                                 GT_FALSE: otherwise
* @param[out] lpmPtr            - Pointer to the root bucket.
* @param[out] bucketTypePtr     - The bucket type of the root bucket after deletion.
*                                 Possible values:
*                                 REGULAR_BUCKET, COMPRESSED_1, COMPRESSED_2, or NEXT_HOP
* @param[out] delEntryPtr       - The next_pointer structure of the entry associated with
*                                 the deleted (address,prefix).
* @param[out] parentUpdateParams - parameters needed for parent update
*
* @retval   GT_OK on success, or
* @retval   GT_NOT_FOUND             - If (address,prefix) not found.
* @retval   GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmSip6RamMngEntryDelete
(
    INOUT PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT                  *lpmPtr,
    IN    GT_U8                                                     *addrCurBytePtr,
    IN    GT_U32                                                    prefix,
    IN    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT                rootBucketFlag,
    IN    GT_BOOL                                                   updateHwAndMem,
    IN    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC                        **lpmEngineMemPtrPtr,
    INOUT CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT                       *bucketTypePtr,
    OUT   GT_PTR                                                    *delEntryPtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                          *shadowPtr,
    IN    PRV_CPSS_DXCH_LPM_ALLOC_TYPE_ENT                          ucMcType,
    IN    GT_BOOL                                                   rollBack,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT                      protocol,
    IN    PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC                     *parentUpdateParams
);


/**
* @internal prvCpssDxChLpmSip6RamMngAllocAvailableMemCheck function
* @endinternal
*
* @brief Check if there is enough available memory to insert a new
*        Unicast or Multicast address and if there is allocate it
*        for further use in the insertion.
*
* @param[in] bucketPtr       - The LPM bucket to check on the LPM insert.
* @param[in] destArr         - If holds the unicast address to be inserted.
* @param[in] prefix          - Holds the prefix length of destArr.
* @param[in] rootBucketFlag  - Indicates the way to deal with a root bucket (if it is).
* @param[in] lpmEngineMemPtrPtr - points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                 which holds all the memory information needed for where and
*                                 how to allocate search memory for each of the lpm levels
* @param[in] shadowPtr       - the shadow relevant for the devices asked to act on.
* @param[in] protocol        - type of ip protocol stack to work on.
* @param[in] ipAllocType     - describes if allocation is for uc or mc entry
* @param[in] insertMode      - how to insert the prefix
*                              (see PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT)
* @param[in] defragmentationEnable  - whether to enable performance costing
*                                     de-fragmentation process in the case that there
*                                     is no place to insert the prefix. To point of the
*                                     process is just to make space for this prefix.*
* @param[in] parentUpdateParams     - parameters needed for parent update
*
* @retval   GT_OK                    - If there is enough memory for the insertion.
* @retval   GT_OUT_OF_PP_MEM         - otherwise.
* @retval   GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Check available memory is done for group of nodes per octet
*/
GT_STATUS prvCpssDxChLpmSip6RamMngAllocAvailableMemCheck
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC      *bucketPtr,
    IN GT_U8                                        *destArr,
    IN GT_U32                                       prefix,
    IN PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT   rootBucketFlag,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           **lpmEngineMemPtrPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol,
    IN PRV_CPSS_DXCH_LPM_ALLOC_TYPE_ENT             ipAllocType,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode,
    IN GT_BOOL                                      defragmentationEnable,
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC        *parentUpdateParams
);


/**
* @internal prvCpssDxChLpmSip6RamMngRootBucketCreate function
* @endinternal
*
* @brief Create a shadow root bucket for a specific virtual router/forwarder Id
*        and protocol, and write it to the HW.
*
* @param[in] shadowPtr                   - the shadow to work on
* @param[in] vrId                        - The virtual router/forwarder ID
* @param[in] protocol                    - the protocol
* @param[in] defUcNextHopEntryPtr        - the default unicast nexthop
* @param[in] defReservedNextHopEntryPtr  - the default reserved range nexthop
* @param[in] defMcNextHopEntryPtr        - the default multicast nexthop
* @param[in] updateHw                    - whether to update the HW
*
* @retval   GT_OK                   - on success
* @retval   GT_BAD_PARAM            - on bad input parameters
* @retval   GT_OUT_OF_CPU_MEM       - no memory
* @retval   GT_FAIL                 - on other failure
*/
GT_STATUS prvCpssDxChLpmSip6RamMngRootBucketCreate
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN GT_U32                                       vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *defUcNextHopEntryPtr,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *defReservedNextHopEntryPtr,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *defMcNextHopEntryPtr,
    IN GT_BOOL                                      updateHw
);

/**
* @internal prvCpssDxChLpmSip6RamMngCreateNew function
* @endinternal
*
* @brief This function creates a new LPM structure, with the pair (0,0) as the
*        default route entry.
*
* @param[in] nextHopPtr          - A pointer to the next hop entry to be set in the
*                                  nextHopEntry field.
* @param[in] firstLevelPrefixLen - The first lpm level prefix.
*
* @retval   A pointer to the new created Bucket, or NULL if allocation failed.
*/
PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC* prvCpssDxChLpmSip6RamMngCreateNew
(
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopPtr,
    IN GT_U8 firstLevelPrefixLen
);


/**
* @internal prvCpssDxChLpmSip6RamMngAllocatedAndBoundMemFree function
* @endinternal
*
* @brief   Free memory that was pre allocated or bound in prvCpssDxChLpmRamMngAllocAvailableMemCheck
*         function. used in case of error in the insertion.
*
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] protocolStack            - protocol stack to work on.
*
* @retval GT_OK                    - operation finish successfully
* @retval GT_BAD_PARAM             - Bad protocolStack input parameter
*/
GT_STATUS prvCpssDxChLpmSip6RamMngAllocatedAndBoundMemFree
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack
);

/**
* @internal prvCpssDxChLpmGroupOfNodesWrite function
* @endinternal
*
* @brief Write an LPM Group Of nodes to the HW.
*
* @note APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2
*
* @param[in] devNum              - the device number
* @param[in] lpmGroupLineOffset  - group of nodes offset from the LPM base in LPM lines
* @param[in] groupOfNodesPtr     - group of nodes content
* @param[out] groupOfNodesLinesPtr - group of nodes number of Lines
*
* @retval   GT_OK                    - on success
* @retval   GT_BAD_PARAM             - one of the parameters with bad value
* @retval   GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval   GT_HW_ERROR              - on hardware error
* @retval   GT_FAIL                  - on error
* @retval   GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS prvCpssDxChLpmGroupOfNodesWrite
(
    IN GT_U8                                               devNum,
    IN GT_U32                                              lpmGroupLineOffset,
    IN OUT PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC      *groupOfNodesPtr,
    OUT GT_U32                                             *groupOfNodesLinesPtr
);

/**
* @internal prvCpssDxChLpmGroupOfNodesCheckAndUpdateNewLinesNeededInThePartition function
* @endinternal
*
* @brief get the number of lines needed for the new LPM Group Of
*        nodes and update the block in the partition.
*
* @note APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2
*
* @param[in] devNum              - the device number
* @param[in] hwGroupOffsetHandle - group of nodes offset handle
* @param[out] groupOfNodesLinesPtr - group of nodes number of Lines
*
* @retval   GT_OK                    - on success
* @retval   GT_BAD_PARAM             - one of the parameters with bad value
* @retval   GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval   GT_HW_ERROR              - on hardware error
* @retval   GT_FAIL                  - on error
* @retval   GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS prvCpssDxChLpmGroupOfNodesCheckAndUpdateNewLinesNeededInThePartition
(
    IN GT_U8                                               devNum,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                    *shadowPtr,
    IN GT_UINTPTR                                          *hwGroupOffsetHandlePtr,
    OUT GT_U32                                             *groupOfNodesLinesPtr
);

/**
* @internal prvCpssDxChLpmSip6RamMngSearchLeafOffset function
* @endinternal
*
* @brief   Search for the existence of a given (address,Prefix) in the given LPM
*         structure and return associated hw bucket offset and pointer to the bucket.
*
* @param[in] bucketPtr                - A pointer to the bucket to search in.
* @param[in] addrCurBytePtr           - The Byte Array represnting the Address to search for.
* @param[in] prefix                   - address  length.
* @param[out] leafLineOffsetPtr        - pointer to hw offset of line contained needed leaf
* @param[out] leafOffsetInLinePtr      - pointer to the position of leaf in line
* @param[out] leafOffsetInLinePtr      - pointer to the position of leaf in line
*
* @retval GT_OK                    - if leaf is found
* @retval GT_NOT_FOUND             - if not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmSip6RamMngSearchLeafOffset
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    IN  GT_U8                                    *addrCurBytePtr,
    IN  GT_U32                                   prefix,
    OUT GT_U32                                   *leafLineOffsetPtr,
    OUT GT_U32                                   *leafOffsetInLinePtr,
    OUT GT_U32                                   *leavesNumberPtr
);

/**
* @internal prvCpssDxChLpmSip6RamMngFindIfSwapAreaCanBeUsefulToFindFreeBlock function
* @endinternal
*
* @brief   Check if in case we use a swap area, new memory can be added.
*         this means that there is free block near the used block and
*         we can use them to create a new bigger block
*
* @param[in] oldHwBucketOffsetHandle  - the old handle we want to extend
* @param[in] newBucketSize            - the size (in lines) of the new bucket
*
* @param[out]freeBlockCanBeFoundPtr   - (pointer to)
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
);

/**
* @internal prvCpssDxChLpmSip6RamMngFindIfSwapAreaCanBeUsefulToFindFreeBlockForRegToCompConversion
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
);

/**
* @internal prvCpssDxChLpmSip6RamMngBucketTreeWrite function
* @endinternal
*
* @brief   write an lpm bucket tree to the HW, and if neccessary allocate memory
*         for it - assuming there is enough memory
*
* @param[in] rootRangePtr             - the range which holds the root bucket of the lpm.
* @param[in] lpmEngineMemPtrPtr       - points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                      which holds all the memory information needed for where and
*                                      how to allocate search memory for each of the lpm levels
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] updateType               - is this an update only for the trie, overwrite of
*                                      the whole tree , or update and allocate memory.
* @param[in] protocolStack            - the protocol Stack (relvant only if updateType ==
*                                      PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_HW_AND_ALLOC_MEM_E)
* @param[in] vrId                     - VR Id (relvant only if updateType ==
*                                      PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_HW_AND_ALLOC_MEM_E)
*
* @retval GT_OK                    - If there is enough memory for the insertion.
* @retval GT_OUT_OF_PP_MEM         - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmSip6RamMngBucketTreeWrite
(
    IN PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC       *rootRangePtr,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           **lpmEngineMemPtrPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_TYPE_ENT   updateType,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack,
    IN GT_U32                                       vrId
);

/**
* @internal prvCpssDxChSip6LpmAacHwWriteEntry function
* @endinternal
*
* @brief   Write a whole LPM entry to the HW using AAC method.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
* @param[in] numWordsToWrite          - number of words to write
* @param[in] startAddress             - Address to start the write
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
*
*/
GT_STATUS prvCpssDxChSip6LpmAacHwWriteEntry
(
    IN GT_U8                   devNum,
    IN GT_U32                  *entryValuePtr,
    IN GT_U32                  numWordsToWrite,
    IN GT_U32                  startAddress
);
/**
* @internal prvCpssDxChSip6LpmAccParamSet function
* @endinternal
*
* @brief   This function sets the ACC related parameters to the HW.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChSip6LpmAccParamSet
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChSip6RamMngWriteMultiEntry function
* @endinternal
*
* @brief   Write number of entries to the table in consecutive indexes.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] startIndex               - index to the first table entry
* @param[in] numOfEntries             - the number of consecutive entries to write
* @param[in] entryValueArrayPtr       - (pointer to) the data that will be written to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSip6RamMngWriteMultiEntry
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_TABLE_ENT              tableType,
    IN GT_U32                           startIndex,
    IN GT_U32                           numOfEntries,
    IN GT_U32                           *entryValueArrayPtr
);

/**
* @internal prvCpssDxChLpmSip6RamMngGetLastGonNodeOffsetAndLeafPosition function
* @endinternal
*
* @brief   Search by given (address,Prefix) in the given LPM
*         structure and return last node pointed to Gon.
* @param[in] bucketPtr                -  root bucket.
* @param[in] addrCurBytePtr           - The Byte Array represnting the Address to search for.
* @param[in] prefix                   - prefix length.
*
* @param[out] leafLineOffsetPtr        - pointer to hw offset of line contained needed leaf
* @param[out] leafOffsetInLinePtr      - pointer to the position of leaf in line
*
* @retval GT_OK                    - if leaf is found
* @retval GT_NOT_FOUND             - if not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmSip6RamMngGetLastGonNodeOffsetAndLeafPosition
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    IN  GT_U8                                    *addrCurBytePtr,
    IN  GT_U32                                   prefix,
    OUT GT_U32                                   *leafLineOffsetPtr,
    OUT GT_U32                                   *leafOffsetInLinePtr,
    OUT CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT      *lastGroupbucketTypePtr,
    OUT GT_BOOL                                  *lastLevelBucketEmbLeavesPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChSip6LpmMngh */

