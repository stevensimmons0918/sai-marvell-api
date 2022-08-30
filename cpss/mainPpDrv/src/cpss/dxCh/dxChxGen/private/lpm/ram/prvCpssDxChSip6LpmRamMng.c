/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChSip6LpmRamMng.c
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
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*
 * Typedef: enum LPM_BUCKET_UPDATE_MODE_ENT
 *
 * Description:
 *      Indicates what kind of update the bucket will go.
 *
 *
 * Fields:
 *      LPM_BUCKET_UPDATE_NONE_E  - updates nothing.
 *      LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E - updates the HW and mem alloc new
 *                                            memory if needed.
 *      LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E - updates the HW and resize
 *                                             bucket's memory if needed
 *                                            (bulk delete operation).
 *      LPM_BUCKET_UPDATE_MEM_ONLY_E - updates mem alloc only.
 *      LPM_BUCKET_UPDATE_SHADOW_ONLY_E - updates the bucket tree shadow only
 *                                       without memory alloc or HW.
 *      LPM_BUCKET_UPDATE_ROOT_BUCKET_SHADOW_ONLY_E - this is an update for
 *                                     root bucket , it's a shodow update
 *                                     only, no touching of the HW.
 *
 */
typedef enum
{
    LPM_BUCKET_UPDATE_NONE_E = 0,
    LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E,
    LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E,
    LPM_BUCKET_UPDATE_MEM_ONLY_E,
    LPM_BUCKET_UPDATE_SHADOW_ONLY_E,
    LPM_BUCKET_UPDATE_ROOT_BUCKET_SHADOW_ONLY_E,
    LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E,

}LPM_BUCKET_UPDATE_MODE_ENT;

static GT_STATUS updateCurrentAndParentGons
(
    IN CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT          newNodeType,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC      *currentBucketPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC        *parentNodeDataPtr,
    IN PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC   *groupOfNodesPtr,
    IN GT_U32                                        subnodeId
);


extern void * cpssOsLpmMalloc
(
    IN GT_U32 size
);

extern void cpssOsLpmFree
(
    IN void* const memblock
);

static const GT_U8                                          startSubNodeAddress[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] =
    {START_ADDR_OF_SUBNODE_0_IN_GROUP_OF_NODES,START_ADDR_OF_SUBNODE_1_IN_GROUP_OF_NODES,
     START_ADDR_OF_SUBNODE_2_IN_GROUP_OF_NODES,START_ADDR_OF_SUBNODE_3_IN_GROUP_OF_NODES,
     START_ADDR_OF_SUBNODE_4_IN_GROUP_OF_NODES,START_ADDR_OF_SUBNODE_5_IN_GROUP_OF_NODES};
static const GT_U8                                           endSubNodeAddress[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] =
    {END_ADDR_OF_SUBNODE_0_IN_GROUP_OF_NODES,END_ADDR_OF_SUBNODE_1_IN_GROUP_OF_NODES,
     END_ADDR_OF_SUBNODE_2_IN_GROUP_OF_NODES,END_ADDR_OF_SUBNODE_3_IN_GROUP_OF_NODES,
     END_ADDR_OF_SUBNODE_4_IN_GROUP_OF_NODES,END_ADDR_OF_SUBNODE_5_IN_GROUP_OF_NODES};

/***************************************************************************
* local defines
****************************************************************************/
/* Defines */

#define PRV_CPSS_DXCH_AAC_USED_FOR_LPM_CNS 1

/* illegal swap index */
#define PRV_CPSS_DXCH_LPM_RAM_ILLEGAL_SWAP_INDEX_CNS  100

/* This macro checks if the given range is the last range   */
/* in this LPM level.                                       */
#define LAST_RANGE_MAC(range)            (range->next == NULL)

/* This macro converts a prefix length to a mask            */
/* representation.                                          */
#define PREFIX_2_MASK_MAC(prefixLen,maxPrefix) \
        (((prefixLen > maxPrefix)|| (prefixLen == 0))? 0 : (1 << (prefixLen-1)))

/* The following macro returns the maximal prefix in the    */
/* next level bucket.                                       */
#define NEXT_BUCKET_PREFIX_MAC(prefix,levelPrefix)  \
        ((prefix <= levelPrefix) ? 0 : (prefix - levelPrefix))

/* The following macro returns the maximal prefix in the    */
/* given level bucket.                                       */
#define BUCKET_PREFIX_AT_LEVEL_MAC(prefix,levelPrefix,level)  \
        ((prefix <= (level * levelPrefix)) ? \
        0 : (prefix - (level * levelPrefix)))

/*global variables macros*/
#define PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.ipLpmDir.sip6IpLpmRamSrc._var)

/*
 * Typedef: struct LPM_RAM_REGULAR_NODE_DATA_STC
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
} LPM_RAM_REGULAR_NODE_DATA_STC;


/**
* @internal mask2PrefixLength function
* @endinternal
*
* @brief This function returns the index of the most significant set bit (1-8),
*        in the given mask.
*
* @param[in] mask            - The prefix mask to operate on.
* @param[in] levelMaxPrefix  - The maximum prefix that can be hold by this mask.
*
* @retval 1 - levelMaxPrefix if (mask != 0),
* @retval 0 - otherwise.
*
*/
GT_U32 mask2PrefixLength
(
    IN GT_U32 mask,
    IN GT_U8 levelMaxPrefix
)
{
    GT_U8 i;

    for(i = 0; i < levelMaxPrefix; i++)
    {
        if((mask & (1 << (levelMaxPrefix - 1 - i))) != 0)
            return (levelMaxPrefix - i);
    }
    return 0;
}

/**
* @internal calcStartEndAddr function
* @endinternal
*
* @brief This function calcules the start & end address of a prefix.
*
* @param[in]  addr          - the address octet.
* @param[in]  prefixLen     - the address prefix length
* @param[out] startAddrPtr  - the calculated start address
* @param[out] endAddrPtr    - the calculated end address
*
*
* @retval GT_OK   - on success
* @retval GT_FAIL - on failure
*
*/
static GT_STATUS calcStartEndAddr
(
    IN  GT_U8  addr,
    IN  GT_U32 prefixLen,
    OUT GT_U8  *startAddrPtr,
    OUT GT_U8  *endAddrPtr
)
{
    GT_U8  prefixComp;          /* temp var. for calculating startAddr  */
                                /* and endAddr.                         */

    prefixComp = (GT_U8)(((prefixLen > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS) ?
                  0 : (PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS - prefixLen)));

    *startAddrPtr = (GT_U8)(addr &
                 (BIT_MASK_MAC(PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS) << prefixComp));

    if (endAddrPtr != NULL)
        *endAddrPtr = (GT_U8)(*startAddrPtr | ~(0xFF << prefixComp));

    return GT_OK;
}

/**
* @internal lpmFalconCheckIfLastLevelBucketEmbdLeaves function
* @endinternal
*
* @brief Check if this embedded leaves bucket from the last level .
*
* @param[in] bucketPtr         - Pointer to the  bucket.
*
* @retval GT_TRUE/GT_FALSE
*
*/
GT_BOOL lpmFalconCheckIfLastLevelBucketEmbdLeaves
(
  IN    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC       *bucketPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *rangePtr = NULL;
    GT_U32  i = 0;
    GT_BOOL lastLevelBucketEmbLeaves = GT_FALSE;
    rangePtr = bucketPtr->rangeList;

    while (rangePtr != NULL)
    {
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )
        {
            i++;
        }
        else
        {
            /* it is not last level */
            return GT_FALSE;
        }
        rangePtr = rangePtr->next;
    }
    if (i <= 3)
    {
        lastLevelBucketEmbLeaves = GT_TRUE;
    }
    return lastLevelBucketEmbLeaves;
}


/**
* @internal find1stOverlap function
* @endinternal
*
* @brief This function traverses the ranges linked list from the low address and
*        stopping at the first overlapping range with the prefix.
*
* @param[in] bucketPtr   - pointer to bucket, in CPU's memory.
* @param[in] startAddr   - prefix start address, lower address covered by the prefix.
* @param[in] pPrevPtr    - A pointer to the node before the ovelapping node.
*
* @retval A pointer to the first overlapping range.
*
*/
static PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *find1stOverlap
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U8                                     startAddr,
    OUT PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    **pPrevPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *rangePtr;/* Current range pointer.   */

    if(pPrevPtr != NULL)
        *pPrevPtr = NULL;

    /* if the address we need is after the cashed range, start with the cashed
       range,
       or if the address we need is the same as the cashed ranged, use it only
       if we don't need to provide a prev range */
    if ((bucketPtr->rangeCash != NULL) &&
        ((startAddr > bucketPtr->rangeCash->startAddr) ||
         ((startAddr == bucketPtr->rangeCash->startAddr) &&
          (pPrevPtr == NULL))))
    {
        rangePtr = bucketPtr->rangeCash;
    }
    else
    {
        rangePtr = bucketPtr->rangeList;
    }

    while((rangePtr->next != NULL) &&
          (startAddr >= rangePtr->next->startAddr))
    {
        bucketPtr->rangeCash = rangePtr;/* the range cash always saves the prev*/
        if(pPrevPtr != NULL)
            *pPrevPtr = rangePtr;

        rangePtr = rangePtr->next;
    }

    return rangePtr;
}


/**
* @internal insert2Trie function
* @endinternal
*
* @brief Inserts a next hop entry to the trie structure.
*
* @param[in] bucketPtr   - Bucket to which the given entry is associated.
* @param[in] startAddr   - Start Addr of the address associated with the given
*                          entry.
* @param[in] prefix      - The address prefix.
* @param[in] trieDepth   - The maximum depth of the trie.
* @param[in] nextPtr     - A pointer to the next hop/next_lpm_trie entry to be
*                         inserted to the trie.
*
* @retval       GT_OK on success,
* @retval       GT_FAIL otherwise.
*/
static GT_STATUS insert2Trie
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    IN GT_U8                                    startAddr,
    IN GT_U32                                   prefix,
    IN GT_U8                                    trieDepth,
    IN GT_PTR                                   nextPtr
)
{
    GT_U8 addr[1];               /* prefix in GT_U8 representation   */

    addr[0] = (GT_U8)(startAddr & (BIT_MASK_MAC(trieDepth) << (trieDepth - prefix)));

    return prvCpssDxChLpmTrieInsert(&(bucketPtr->trieRoot),
                                    addr,prefix,trieDepth,nextPtr);
}

/**
* @internal createNewSip6Bucket function
* @endinternal
*
* @brief This function creates a new bucket with a given default next hop route
*        entry.
*
* @param[in] nextHopPtr - The default route next hop data.
* @param[in] trieDepth   - The maximum depth of the trie.
* @param[in] pointerType - range pointer type.
*
* @retval A pointer to the new created bucket if succeeded, NULL otherwise.
*
*/
static PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC * createNewSip6Bucket
(
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopPtr,
    IN GT_U8                                     trieDepth,
    IN CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT       pointerType
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *pBucket;     /* The bucket to be created.    */
    GT_STATUS                                rc;

    if((pBucket = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC)))
       == NULL)
        return NULL;

    if((pBucket->rangeList = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC)))
       == NULL)
    {
        cpssOsLpmFree(pBucket);
        pBucket = NULL;

        return NULL;
    }

    /* Initialize the bucket's fields.          */
     if ( ((pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)&&
         (nextHopPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_MULTIPATH_E)) ||
         ((pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)&&
          (nextHopPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E)) )
    {
        cpssOsPrintf(" BAD STATE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }
    pBucket->rangeList->pointerType              = pointerType;
    pBucket->rangeList->lowerLpmPtr.nextHopEntry = nextHopPtr;

    pBucket->rangeList->startAddr   = 0;
    pBucket->rangeList->mask        = 0;
    pBucket->rangeList->next        = NULL;
    pBucket->bucketType             = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
    pBucket->fifthAddress           = 0;
    pBucket->hwBucketOffsetHandle   = 0;
    cpssOsMemSet(pBucket->hwGroupOffsetHandle,0,sizeof(pBucket->hwGroupOffsetHandle));
    pBucket->pointingRangeMemAddr   = 0xffffffff;

    pBucket->rangeCash = NULL;

    pBucket->numOfRanges = 1;
    pBucket->bucketHwUpdateStat = PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E;


    /* Insert the default prefix into the       */
    /* Trie of the newly create LPM bucket.     */
    pBucket->trieRoot.pData = NULL;
    rc = insert2Trie(pBucket,0,0,trieDepth,nextHopPtr);
    if (rc != GT_OK)
    {
        /* trie was not inserted successfully*/
        cpssOsLpmFree(pBucket->rangeList);
        cpssOsLpmFree(pBucket);
        return NULL;
    }

    pBucket->trieRoot.father = pBucket->trieRoot.leftSon = NULL;
    pBucket->trieRoot.rightSon = NULL;

    return pBucket;
}

/**
* @internal sip6SplitRange function
* @endinternal
*
* @brief This function splits a range. According to one of the following possible splits:
*           PRV_CPSS_DXCH_LPM_RAM_SPLIT_OVERWRITE_E, PRV_CPSS_DXCH_LPM_RAM_SPLIT_LOW_SPLIT_E,
*           PRV_CPSS_DXCH_LPM_RAM_SPLIT_HIGH_SPLIT_E and PRV_CPSS_DXCH_LPM_RAM_SPLIT_MID_SPLIT_E
*
*
* @param[in] rangePtrPtr     - Pointer to the range to be splitted.
* @param[in] startAddr       - The low address of the new range.
* @param[in] endAddr         - The high address of the new range.
* @param[in] prefixLength    - The length of the dominant prefix of the new range.
* @param[in] rangeInfoPtr    - the range information to use for the new range
*                              (or overwrite an old one with this new info).
* @param[in] levelPrefix     - The current lpm level prefix.
* @param[in] updateOldPtr    - (GT_TRUE) this is an update for an already existing entry.
*  @param[out] rangePtrPtr         - A pointer to the next range to be checked for split.
*  @param[out] numOfNewRangesPtr   - Number of new created ranges as a result of the split.
*  @param[out] pointerTypePtrPtr   - the added/replaced range's pointerType field pointer
*
* @retval   GT_OK on success, or
* @retval   GT_OUT_OF_CPU_MEM on lack of cpu memory.
*
*/
static GT_STATUS sip6SplitRange
(
    INOUT PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    **rangePtrPtr,
    IN GT_U8                                        startAddr,
    IN GT_U8                                        endAddr,
    IN GT_U32                                       prefixLength,
    IN PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC       *rangeInfoPtr,
    IN GT_U8                                        levelPrefix,
    IN GT_BOOL                                      *updateOldPtr,
    IN CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT          *bucketTypePtr,
    OUT GT_U32                                      *numOfNewRangesPtr,
    OUT GT_U32                                      *numOfNewHwRangesPtr,
    OUT CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT         **pointerTypePtrPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *newRangePtr;  /* Points to the new create */
                            /* range, if such creation is needed.                      */

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *curRangePtr = (*rangePtrPtr);    /* Points to rangePtrPtr, for  */
                                            /* easy access.             */
    GT_U8 mask;                /* The mask represented by prefixLength */
                                /* and levelPrefix.                     */
    GT_U8 endRange;            /* Index by which this range ends.      */
    PRV_CPSS_DXCH_LPM_RAM_SPLIT_METHOD_ENT splitMethod;     /* The method by which to split the     */
                                                            /* rangePtrPtr, if needed.              */
    GT_BOOL splittedRangeNeedsHwUpdate = GT_FALSE;
    GT_U8 groupSubNodeId;

    mask = (GT_U8)PREFIX_2_MASK_MAC(prefixLength,levelPrefix);

    endRange = (GT_U8)(LAST_RANGE_MAC(curRangePtr) ? ((1 << levelPrefix) - 1) :
        ((curRangePtr->next->startAddr) - 1));

    splitMethod = (((startAddr > curRangePtr->startAddr) ? 1 : 0) +
                   ((endAddr < endRange) ? 2 : 0));

    if((*rangePtrPtr)->updateRangeInHw == GT_TRUE)
    {
        /* the range is going to be splitted needs update in hw     */
        /* so all ranges created after splitting should be marked as*/
        /* as needed update in hw                                   */
        /* such situation could be occured during bulk operation    */
        splittedRangeNeedsHwUpdate = GT_TRUE;
    }

    switch (splitMethod)
    {
    case PRV_CPSS_DXCH_LPM_RAM_SPLIT_OVERWRITE_E:
        newRangePtr = curRangePtr;
        newRangePtr->mask |= mask;
        if(*updateOldPtr == GT_TRUE)       /*   Check This  */
        {
            /*osStatFree(newRangePtr->nextPointer.nextPtr.nextHopEntry);*/
            *updateOldPtr = GT_FALSE;
        }

        newRangePtr->pointerType = rangeInfoPtr->pointerType;
        newRangePtr->lowerLpmPtr = rangeInfoPtr->lowerLpmPtr;
        newRangePtr->updateRangeInHw = GT_TRUE;
        *numOfNewRangesPtr = 0;
        *numOfNewHwRangesPtr = 0;
        /* record the pointer type ptr */
        *pointerTypePtrPtr = &(newRangePtr->pointerType);
        break;

    case PRV_CPSS_DXCH_LPM_RAM_SPLIT_LOW_SPLIT_E:     /* A new node should be added before curRangePtr    */
        if((newRangePtr = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC)))
           == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

        /* Insert newRangePtr after curRangePtr     */
        newRangePtr->next = curRangePtr->next;
        curRangePtr->next = newRangePtr;

        /* newRangePtr represents the old Range.    */
        newRangePtr->startAddr  = (GT_U8)(endAddr + 1);
        newRangePtr->mask       = curRangePtr->mask;

        newRangePtr->pointerType = curRangePtr->pointerType;
        newRangePtr->lowerLpmPtr = curRangePtr->lowerLpmPtr;
        if (splittedRangeNeedsHwUpdate == GT_TRUE)
        {
            newRangePtr->updateRangeInHw = GT_TRUE;
        }
        else
        {
            /* no new pointer in the old range */
            newRangePtr->updateRangeInHw = GT_FALSE;
        }


        if (splittedRangeNeedsHwUpdate == GT_TRUE)
        {
            newRangePtr->updateRangeInHw = GT_TRUE;
        }
        else
        {
            /* no new pointer in the old range */
            newRangePtr->updateRangeInHw = GT_FALSE;
        }
        /* curRangePtr represents the new Range.    */
        curRangePtr->mask       |= mask;
        curRangePtr->pointerType = rangeInfoPtr->pointerType;
        curRangePtr->lowerLpmPtr = rangeInfoPtr->lowerLpmPtr;

        /* new pointer so we need to update in the hardware */
        curRangePtr->updateRangeInHw = GT_TRUE;

        *rangePtrPtr       = newRangePtr;
        *numOfNewRangesPtr = 1;
        *numOfNewHwRangesPtr = 1;
        if (*bucketTypePtr == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            if (startAddr != 0)
            {
               groupSubNodeId = endAddr/NUMBER_OF_RANGES_IN_SUBNODE;
               if (endAddr == endSubNodeAddress[groupSubNodeId])
               {
                   *numOfNewHwRangesPtr = 0;
               }
            }
        }

        /* record the pointer type ptr */
        *pointerTypePtrPtr = &(curRangePtr->pointerType);
        break;

    case PRV_CPSS_DXCH_LPM_RAM_SPLIT_HIGH_SPLIT_E:    /* A new node should be added after curRangePtr    */
        if((newRangePtr = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC)))
           == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

        /* Insert newRangePtr after curRangePtr    */
        newRangePtr->next = curRangePtr->next;
        curRangePtr->next = newRangePtr;
        if (splittedRangeNeedsHwUpdate == GT_TRUE)
        {
            curRangePtr->updateRangeInHw = GT_TRUE;
        }
        else
        {
            /* no new pointer in the old range */
            curRangePtr->updateRangeInHw = GT_FALSE;
        }


        /* Update the fields of newRangePtr     */
        newRangePtr->startAddr  = startAddr;
        newRangePtr->mask       = (GT_U8)(curRangePtr->mask | mask);
        newRangePtr->pointerType = rangeInfoPtr->pointerType;
        newRangePtr->lowerLpmPtr = rangeInfoPtr->lowerLpmPtr;
        /* new pointer so we need to update in the hardware */
        newRangePtr->updateRangeInHw = GT_TRUE;

        *rangePtrPtr       = newRangePtr;
        *numOfNewRangesPtr = 1;
        *numOfNewHwRangesPtr = 1;
        if (*bucketTypePtr == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            if (endAddr != 255)
            {
               groupSubNodeId = startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
               if (startAddr == startSubNodeAddress[groupSubNodeId])
               {
                   *numOfNewHwRangesPtr = 0;
               }
            }
        }
        /* record the pointer type ptr */
        *pointerTypePtrPtr = &(newRangePtr->pointerType);
        break;

    case PRV_CPSS_DXCH_LPM_RAM_SPLIT_MID_SPLIT_E:   /* Two new node should be added, one for the    */
                                                    /* second part of curRangePtr, and one for the  */
                                                    /* range created by the new inserted prefix.    */
        if((newRangePtr = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC)))
           == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);


        if((newRangePtr->next = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC)))
           == NULL)
        {
            cpssOsLpmFree(newRangePtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

        }


        /* Insert the two new ranges after curRangePtr     */
        newRangePtr->next->next = curRangePtr->next;
        curRangePtr->next = newRangePtr;
        if (splittedRangeNeedsHwUpdate == GT_TRUE)
        {
            curRangePtr->updateRangeInHw = GT_TRUE;
        }
        else
        {
            /* no new pointer in the old range */
            curRangePtr->updateRangeInHw = GT_FALSE;
        }


        /* Update the fields of the first added range   */
        newRangePtr->startAddr  = startAddr;
        newRangePtr->mask       = (GT_U8)(curRangePtr->mask | mask);

        newRangePtr->pointerType = rangeInfoPtr->pointerType;
        newRangePtr->lowerLpmPtr = rangeInfoPtr->lowerLpmPtr;
        /* new pointer so we need to update in the hardware */
        newRangePtr->updateRangeInHw = GT_TRUE;

        /* record the pointer type ptr */
        *pointerTypePtrPtr = &(newRangePtr->pointerType);

        /* Update the fields of the second added range  */
        newRangePtr             = newRangePtr->next;
        newRangePtr->startAddr  = (GT_U8)(endAddr + 1);
        newRangePtr->mask       = curRangePtr->mask;

        newRangePtr->pointerType = curRangePtr->pointerType;
        newRangePtr->lowerLpmPtr = curRangePtr->lowerLpmPtr;

        if (splittedRangeNeedsHwUpdate == GT_TRUE)
        {
            newRangePtr->updateRangeInHw = GT_TRUE;
        }
        else
        {
            /* the new pointer is the not new just copied, no need to update in Hw*/
            newRangePtr->updateRangeInHw = GT_FALSE;
        }

        *rangePtrPtr = newRangePtr;
        *numOfNewRangesPtr = 2;
        *numOfNewHwRangesPtr = 2;
        if (*bucketTypePtr == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
           groupSubNodeId = startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
           if (startAddr == startSubNodeAddress[groupSubNodeId])
           {
               (*numOfNewHwRangesPtr)--;
           }
           groupSubNodeId = endAddr/NUMBER_OF_RANGES_IN_SUBNODE;
           if (endAddr == endSubNodeAddress[groupSubNodeId])
           {
               (*numOfNewHwRangesPtr)--;
           }
        }

        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal lpmFalconFillLeafData function
* @endinternal
*
* @brief This function fill leafsNodesArray struct.
*
* @param[in] nextHopPtr             - pointer to nexthop data
* @param[in] entryType              - next hop pointer type
* @param[out] leafNodesArrayPtr     - array saving leaf nodes info
*
* @retval GT_OK on success, or
* @retval GT_FAIL - otherwise
*
*/
static GT_STATUS lpmFalconFillLeafData
(
    IN  PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC         *rangePtr,
    OUT PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC               *leafNodesArrayPtr
)
{
    GT_UINTPTR hwBucketOffsetHandle = 0;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT            entryType;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC      *nextHopPtr = NULL;

    entryType = rangePtr->pointerType;
    if ( (entryType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
         (entryType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  &&
         (entryType != 0xff) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "wrong node type");
    }
    if (entryType == 0xff) /*leaf points to src trie*/
    {
        hwBucketOffsetHandle = rangePtr->lowerLpmPtr.nextBucket->hwBucketOffsetHandle;
        /* It points on src tree */
        if (hwBucketOffsetHandle != 0)
        {
            leafNodesArrayPtr->pointToSip = GT_TRUE;
            leafNodesArrayPtr->pointerToSip = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(hwBucketOffsetHandle);
            leafNodesArrayPtr->nextNodeType = rangePtr->lowerLpmPtr.nextBucket->bucketType;
            leafNodesArrayPtr->priority = 0;
            if ((leafNodesArrayPtr->nextNodeType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                (leafNodesArrayPtr->nextNodeType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) )
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "wrong node type");
            }
            leafNodesArrayPtr->entryType = PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E;
            return GT_OK;
        }
        else
        {
            /* we have only shadow src tree */
            entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        }
    }
    if (rangePtr->pointerType == 0xff)
    {
        nextHopPtr = rangePtr->lowerLpmPtr.nextBucket->rangeList->lowerLpmPtr.nextHopEntry;
    }
    else
    {
        nextHopPtr = rangePtr->lowerLpmPtr.nextHopEntry;
    }

    leafNodesArrayPtr->priority                 = nextHopPtr->priority;
    leafNodesArrayPtr->entryType                = entryType;
    leafNodesArrayPtr->index                    = nextHopPtr->routeEntryBaseMemAddr;
    leafNodesArrayPtr->ipv6MCGroupScopeLevel    = nextHopPtr->ipv6McGroupScopeLevel;
    leafNodesArrayPtr->ucRPFCheckEnable         = nextHopPtr->ucRpfCheckEnable;
    leafNodesArrayPtr->sipSaCheckMismatchEnable = nextHopPtr->srcAddrCheckMismatchEnable;
    leafNodesArrayPtr->applyPbr                 = nextHopPtr->applyPbr ;
    return GT_OK;
}


/**
* @internal lpmFalconConvertLpmOffset function
* @endinternal
*
* @brief This function convert absolute lpm offset to bunk number and offset within the bank.
*
* @param[in] bucketType                    - type of bucket
* @param[in] groupOffseHandletPtr          - pointer to  group of nodes offsets handle array
* @param[in] startSubnode                  - first affected LPM line in regular node
* @param[in] endSubnode                    - last affected LPM line in regular node
* @param[in] allSubnodes                   - all lines in regular node
* @param[out] relativeOffsetPtr            - pointer to  group of nodes offsets array {bank,offset_in_bank}
*
* @retval   GT_OK on success, or
* @retval   GT_FAIL on failure
*
*/
static GT_STATUS lpmFalconConvertLpmOffset
(
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT bucketType,
    IN  GT_UINTPTR                          *groupOffseHandlePtr,
    IN  GT_U32                              lpmRamTotalBlocksSizeIncludingGap,
    IN  GT_U32                              startSubnode,
    IN  GT_U32                              endSubnode,
    IN  GT_BOOL                             allSubnodes,
    OUT GT_UINTPTR                          *relativeOffsetPtr
)
{
    GT_U32 bankNumber;
    GT_U32 offsetInBank;
    GT_U32 groupOffset;
    GT_U32 i;
    GT_U32 maxNumOfSubnodes, startNumOfSubnodes;
    if ( (bucketType != CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         &&
         (bucketType != CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   &&
         (bucketType != CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) &&
         (bucketType != CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E)       &&
         (bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
        if (allSubnodes == GT_TRUE)
        {
            startNumOfSubnodes = 0;
            maxNumOfSubnodes = PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS;
        }
        else
        {
            startNumOfSubnodes = startSubnode;
            maxNumOfSubnodes = endSubnode;
        }
    }
    else
    {
        startNumOfSubnodes = 0;
        maxNumOfSubnodes = 1;
    }
    for (i = startNumOfSubnodes; i < maxNumOfSubnodes; i++)
    {
        if (groupOffseHandlePtr[i] != 0)
        {
            groupOffset = (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(groupOffseHandlePtr[i]));
            bankNumber = groupOffset/lpmRamTotalBlocksSizeIncludingGap;
            offsetInBank = groupOffset%lpmRamTotalBlocksSizeIncludingGap;
            *relativeOffsetPtr = (bankNumber << 15 | offsetInBank)&0xfffff;
        }
        else
        {
            *relativeOffsetPtr = 0;
        }
        relativeOffsetPtr++;
    }
   return GT_OK;
}


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
)
{
    GT_STATUS rc = GT_OK;
    GT_32 i = 0;
    GT_U32 mcLeavesCounter = 0;
    GT_U8 numOfLeaves = 0;
    GT_U8 maxNumOfLeaves = 0;
    GT_U8 maxOffsetsNum = 0;
    GT_U8 lastOffset = 0;
    GT_U8 lastChildNodeTypes = 0;
    GT_U8 offsetBeforeLast = 0;
    GT_U8 childTypeBeforeLast;
    GT_BOOL lastLevelBucketEmbLeaves = GT_FALSE;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC              *rangePtr = NULL;
    switch (bucketPtr->bucketType)
    {
    case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
        maxNumOfLeaves = 0;
        maxOffsetsNum = 9;
        break;
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
        maxNumOfLeaves = 1;
        maxOffsetsNum = 6;
        break;
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
        maxNumOfLeaves = 2;
        maxOffsetsNum = 4;
        break;
    case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
        maxNumOfLeaves = 3;
        maxOffsetsNum = 2;
        break;
   /* case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
    case CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E:
    case PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E:
        break;*/
    default:
        cpssOsPrintf(" BAD STATE: wrong bucket type\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    if (compressedNodesData != NULL)
    {
        compressedNodesData->fullEmbeddedLeaf = GT_FALSE;
        lastLevelBucketEmbLeaves = lpmFalconCheckIfLastLevelBucketEmbdLeaves(bucketPtr);
    }
    compressedNodesArrayPtr->compressedType = bucketPtr->bucketType;
    rangePtr = bucketPtr->rangeList;

    while (rangePtr != NULL)
    {
        if (i == 0)
        {
            /* first range : its start is 0*/
            compressedNodesArrayPtr->ranges1_9[i]=0;
        }
        else
            compressedNodesArrayPtr->ranges1_9[i]= rangePtr->startAddr;
        /* check subnodes ths range belongs to */
        if (rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            compressedNodesArrayPtr->childNodeTypes0_9[i] = 0x2;
        }

        if( (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
        {
            compressedNodesArrayPtr->childNodeTypes0_9[i] = 0x3;
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )
        {
            compressedNodesArrayPtr->childNodeTypes0_9[i] = 0x1;
            if( (bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
                (bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
                (bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
            {
                /* embedded leaves bucket */
                if (numOfLeaves < maxNumOfLeaves)
                {
                    rc =  lpmFalconFillLeafData(rangePtr,
                                                &compressedNodesArrayPtr->embLeavesArray[numOfLeaves]);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    if((((bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E)&&
                         (bucketPtr->numOfRanges==2)) ||
                        (bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E))&&
                        (rangePtr->pointerType == 0xff))
                    {
                        /* set in src hw line that points on src */
                        rangePtr->lowerLpmPtr.nextBucket->pointingRangeMemAddr = bucketPtr->nodeMemAddr;
                        /* set in src offset of pointer inside of line*/
                        rangePtr->lowerLpmPtr.nextBucket->fifthAddress =  (89 - numOfLeaves*23)+2;
                    }
                }
                else
                    cpssOsPrintf(" BAD STATE: number of leaves doesn't match compress bucket type\n");
            }

            if ( (compressedNodesData != NULL) &&
                 (rangePtr->pointerType == 0xff) &&
                 (lastLevelBucketEmbLeaves == GT_TRUE) )
            {
                compressedNodesData->fullEmbeddedLeaf = GT_TRUE;
                compressedNodesData->isMcRangeExist = GT_TRUE;
                compressedNodesData->mcLeavesOrderIdArray[mcLeavesCounter] = numOfLeaves;
                compressedNodesData->mcRanges[mcLeavesCounter] = rangePtr;
                mcLeavesCounter++;
            }
            numOfLeaves++;
        }
        rangePtr = rangePtr->next;
        i++;
    }
    if (compressedNodesData != NULL)
    {
        compressedNodesData->mcLeavesNumber = mcLeavesCounter;
    }

    if (i==0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "\n ERROR unexpected i==0, zero ranges \n");
    }
    if (i - 1 < maxOffsetsNum)
    {
        lastOffset = compressedNodesArrayPtr->ranges1_9[i - 1];
        lastChildNodeTypes = compressedNodesArrayPtr->childNodeTypes0_9[i-1];
        offsetBeforeLast = (i-2 > 0)? compressedNodesArrayPtr->ranges1_9[i-2] : lastOffset;
        childTypeBeforeLast = (i-2 > 0)? compressedNodesArrayPtr->childNodeTypes0_9[i-2] :
                               i<=2    ? 0 : lastChildNodeTypes;
        if((i-2) > 0)
        {
            compressedNodesArrayPtr->childNodeTypes0_9[i-2] = 0;
        }

        for (i = i-1; i <= maxOffsetsNum; i++)
        {
            if (i == maxOffsetsNum)
            {
                compressedNodesArrayPtr->ranges1_9[i] = lastOffset;
                compressedNodesArrayPtr->childNodeTypes0_9[i] = lastChildNodeTypes;
            }
            else
            if ((i+1) == maxOffsetsNum)
            {
                compressedNodesArrayPtr->ranges1_9[i] = offsetBeforeLast;
                compressedNodesArrayPtr->childNodeTypes0_9[i] = childTypeBeforeLast;
            }
            else
            {
                compressedNodesArrayPtr->ranges1_9[i] = offsetBeforeLast;
                compressedNodesArrayPtr->childNodeTypes0_9[i] = 0;
            }
        }
    }

    /* now add ofsets to hw data: */
    if (bucketPtr->hwGroupOffsetHandle[0]!= 0)
    {
         rc =  lpmFalconConvertLpmOffset(bucketPtr->bucketType,
                                    bucketPtr->hwGroupOffsetHandle,
                                    lpmRamTotalBlocksSizeIncludingGap,
                                    0,0,0,
                                    &compressedNodesArrayPtr->lpmOffset);
    }
    else
    {
        /* possible to check for node with embedded leaves. Otherwisw - error*/
        /* it may be last node with embedded leaves */
        compressedNodesArrayPtr->lpmOffset = 0;
    }
    compressedNodesArrayPtr->numberOfLeaves = numOfLeaves;
    return rc;
}



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
)
{
    GT_U32 startNodeIndex, endNodeIndex,i;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC              *rangePtr = NULL;
    GT_U8 endRangeAddress = 0;
    rangePtr = bucketPtr->rangeList;
    /* In regular node we have 6 different subnodes */
    while (rangePtr != NULL)
    {
        startNodeIndex = rangePtr->startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
        if (rangePtr->next == NULL)
        {
            endRangeAddress = 255; /* last range end addr */
        }
        else
        {
            endRangeAddress = rangePtr->next->startAddr - 1;
        }
        endNodeIndex =  endRangeAddress / NUMBER_OF_RANGES_IN_SUBNODE;
        /* check subnodes ths range belongs to */
        if (rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            regularNodesArrayPtr->childNodeTypes0_255[rangePtr->startAddr] = 0x2;
            for (i = startNodeIndex; i <= endNodeIndex; i++ )
            {
                if (i > startNodeIndex)
                {
                    /* catch all subnodes crosses in spectrum */
                    regularNodesArrayPtr->childNodeTypes0_255[startSubNodeAddress[i]] = 0x2;
                }
            }
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
        {
            regularNodesArrayPtr->childNodeTypes0_255[rangePtr->startAddr] = 0x3;
            for (i = startNodeIndex; i <= endNodeIndex; i++ )
            {
                if (i > startNodeIndex)
                {
                    /* catch all subnodes crosses in spectrum */
                    regularNodesArrayPtr->childNodeTypes0_255[startSubNodeAddress[i]] = 0x3;
                }
            }
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )
        {
            regularNodesArrayPtr->childNodeTypes0_255[rangePtr->startAddr] = 0x1;

            for (i = startNodeIndex; i <= endNodeIndex; i++ )
            {
                if (i > startNodeIndex)
                {
                    /* catch all subnodes crosses in spectrum */
                    regularNodesArrayPtr->childNodeTypes0_255[startSubNodeAddress[i]] = 1;
                }
            }
        }
        rangePtr = rangePtr->next;
    }
    /* now add ofsets to hw data: */
    lpmFalconConvertLpmOffset(bucketPtr->bucketType,
                              bucketPtr->hwGroupOffsetHandle,
                              lpmRamTotalBlocksSizeIncludingGap,
                              0,0,GT_TRUE,
                              regularNodesArrayPtr->lpmOffsets);
    return GT_OK;
}


#if 0
/*******************************************************************************
* lpmFalconUpdateRegularDataForGivenSubnodes
*
* DESCRIPTION:
*       This function scan bucket shadow and fill regularNodesArray struct for
*       given subnode.
*
* INPUTS:
*       bucketPtr                         - pointer to  bucket
*       lpmRamTotalBlocksSizeIncludingGap - lpm total block size
*       startSubNodeAddress               - start subnode address
*       endSubNodeAddress                 - end subnode address
*
* OUTPUTS:
*       regularNodesArrayPtr  - array saving regular nodes info
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL - otherwise
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS lpmFalconUpdateRegularDataForGivenSubnodes
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *bucketPtr,
    IN  GT_U32                                      lpmRamTotalBlocksSizeIncludingGap,
    IN  GT_U32                                      startSubNodeAddr,
    IN  GT_U32                                      endSubNodeAddr,
    OUT CPSS_DXCH_LPM_REGULAR_STC                   *regularNodesArrayPtr
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC              *rangePtr = NULL;
    GT_U32 endRangeAddr = 0;
    GT_U32 startSubNodeIndex,endSubNodeIndex;
    startSubNodeIndex = startSubNodeAddr/NUMBER_OF_RANGES_IN_SUBNODE;
    endSubNodeIndex = endSubNodeAddr/NUMBER_OF_RANGES_IN_SUBNODE;
    rangePtr = bucketPtr->rangeList;

    /* choose needed range for subnode */
    while (rangePtr !=NULL)
    {
        if (rangePtr->startAddr >= startSubNodeAddr)
        {
            break;
        }
        else
        {
            /* check if the range incorporate needed range */
            if (rangePtr->next != NULL)
            {
                endRangeAddr = rangePtr->next->startAddr - 1;
            }
            else
            {
                endRangeAddr = 255; /* last range end address must be 255*/
            }
            if (endRangeAddr > startSubNodeAddr)
            {
                /* It must be start from this range */
                break;
            }
        }
        rangePtr = rangePtr->next;
    }

    while (rangePtr != NULL)
    {
        if (rangePtr->next == NULL)
        {
            endRangeAddr = 255; /* last range end addr */
        }
        else
        {
            endRangeAddr = rangePtr->next->startAddr - 1;
        }
        if (rangePtr->startAddr > endSubNodeAddr)
        {
            break;
        }

        /* check subnodes ths range belongs to */
        if (rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            regularNodesArrayPtr->childNodeTypes0_255[rangePtr->startAddr] = 0x2;
            for (i = startSubNodeIndex; i <= endSubNodeIndex; i++ )
            {
                if (i > startSubNodeIndex)
                {
                    /* catch all subnodes crosses in spectrum */
                    regularNodesArrayPtr->childNodeTypes0_255[startSubNodeAddress[i]] = 0x2;
                }
            }
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
        {
            regularNodesArrayPtr->childNodeTypes0_255[rangePtr->startAddr] = 0x3;
            for (i = startSubNodeIndex; i <= endSubNodeIndex; i++ )
            {
                if (i > startSubNodeIndex)
                {
                    /* catch all subnodes crosses in spectrum */
                    regularNodesArrayPtr->childNodeTypes0_255[startSubNodeAddress[i]] = 0x3;

                }
            }
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) )
        {
            if (rangePtr->startAddr < startSubNodeAddr)
            {
                regularNodesArrayPtr->childNodeTypes0_255[startSubNodeAddr] = 1;
            }
            else
            {
                regularNodesArrayPtr->childNodeTypes0_255[rangePtr->startAddr] = 1;
            }

            for (i = startSubNodeIndex; i <= endSubNodeIndex; i++ )
            {
                if (i > startSubNodeIndex)
                {
                    /* catch all subnodes crosses in spectrum */
                    regularNodesArrayPtr->childNodeTypes0_255[startSubNodeAddress[i]] = 1;
                }
            }
        }
        rangePtr = rangePtr->next;
    }
    /* now add ofsets to hw data: */
    lpmFalconConvertLpmOffset(bucketPtr->bucketType,
                              bucketPtr->hwGroupOffsetHandle,
                              lpmRamTotalBlocksSizeIncludingGap,
                              startSubNodeIndex,endSubNodeIndex,GT_TRUE,
                              regularNodesArrayPtr->lpmOffsets);
    return GT_OK;
}

#endif
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
)
{
    GT_STATUS rc;
    GT_U8 regularCounter = 0;
    GT_U8 compressedCounter = 0;
    GT_U8 leafCounter = 0;
    GT_U8 endRangeAddr = 0;
    LPM_RAM_COMPRESSED_NODE_DATA_STC  *compressedNodesData=NULL;
    LPM_RAM_REGULAR_NODE_DATA_STC     *regularNodesData=NULL;
    GT_U32 i,j;
    GT_U32 leafLineOffset = 0;
    GT_U8 indexInLine = 0;
    GT_U32 compressedOffset = 0;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    *mcRange = NULL;

    compressedNodesData =  (LPM_RAM_COMPRESSED_NODE_DATA_STC *)cpssOsMalloc(NUMBER_OF_RANGES_IN_SUBNODE*sizeof(LPM_RAM_COMPRESSED_NODE_DATA_STC));
    if (compressedNodesData == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(compressedNodesData,0,NUMBER_OF_RANGES_IN_SUBNODE*sizeof(LPM_RAM_COMPRESSED_NODE_DATA_STC));

    regularNodesData =  (LPM_RAM_REGULAR_NODE_DATA_STC *)cpssOsMalloc(NUMBER_OF_RANGES_IN_SUBNODE*sizeof(LPM_RAM_REGULAR_NODE_DATA_STC));
    if (regularNodesData == NULL)
    {
        cpssOsFree(compressedNodesData);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(regularNodesData,0,NUMBER_OF_RANGES_IN_SUBNODE*sizeof(LPM_RAM_REGULAR_NODE_DATA_STC));


    if (bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
        /* choose needed range for subnode */
        while (rangePtr !=NULL)
        {
            if (rangePtr->startAddr >= startAddress)
            {
                break;
            }
            else
            {
                /* check if the range incorporate needed range */
                if (rangePtr->next != NULL)
                {
                    endRangeAddr = rangePtr->next->startAddr - 1;
                }
                else
                {
                    endRangeAddr = 255; /* last range end address must be 255*/
                }
                if (endRangeAddr >= startAddress)
                {
                    /* It must be start from this range */
                    break;
                }
            }
            rangePtr = rangePtr->next;
        }
    }
    while (rangePtr != NULL)
    {
        if (bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            if (rangePtr->startAddr > endAddress)
            {
                break;
            }
        }
        if(rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            rc =  lpmFalconFillRegularData(rangePtr->lowerLpmPtr.nextBucket,
                                           lpmRamTotalBlocksSizeIncludingGap,
                                           &groupOfNodesPtr->regularNodesArray[regularCounter]);
            if (rc != GT_OK)
            {
                cpssOsFree(compressedNodesData);
                cpssOsFree(regularNodesData);
                return rc;
            }
            regularNodesData[regularCounter].nodePtr = rangePtr->lowerLpmPtr.nextBucket;
            regularCounter++;

        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
        {
            rc =  lpmFalconFillCompressedData(rangePtr->lowerLpmPtr.nextBucket,
                                              lpmRamTotalBlocksSizeIncludingGap,
                                              &groupOfNodesPtr->compressedNodesArray[compressedCounter],
                                              &compressedNodesData[compressedCounter]);
            if (rc != GT_OK)
            {
                cpssOsFree(compressedNodesData);
                cpssOsFree(regularNodesData);
                return rc;
            }
            compressedNodesData[compressedCounter].nodePtr = rangePtr->lowerLpmPtr.nextBucket;
            compressedCounter++;
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )
        {
            /* if bucket type is compressed_with_embedded leaves then all leaves are inside*/
            /* compressed node itself. It is not included into group of nodes current bucket pointed on*/
            /* These leaves relate to the next group of nodes together with embedded compressed: 1 level up*/
            if( (bucketType != CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   &&
                (bucketType != CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) &&
                (bucketType != CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
            {
                rc =  lpmFalconFillLeafData(rangePtr,
                                            &groupOfNodesPtr->leafNodesArray[leafCounter]);
                if (rc != GT_OK)
                {
                    cpssOsFree(compressedNodesData);
                    cpssOsFree(regularNodesData);
                    return rc;
                }
                if (rangePtr->pointerType == 0xff)
                {
                    /* we are in group node. Taking in account that only exact match group prefix supported*/
                    /* we have only leaves in this node*/
                    leafLineOffset = gonOffset + leafCounter/MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;
                    indexInLine    = leafCounter%MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;
                    /* set in src hw line that points on src */
                    rangePtr->lowerLpmPtr.nextBucket->pointingRangeMemAddr = leafLineOffset;
                    /* set in src offset of pointer inside of line*/
                    rangePtr->lowerLpmPtr.nextBucket->fifthAddress = (92-23*indexInLine)+2;
                }
                leafCounter++;
            }
        }
        rangePtr->updateRangeInHw = GT_FALSE;
        rangePtr = rangePtr->next;
    }

    /* update hw addresses for all shadow nodes inside of gon */
    for (i = 0; i < regularCounter; i++)
    {
        regularNodesData[i].nodePtr->nodeMemAddr = gonOffset +i*6;;
    }
    compressedOffset = gonOffset + regularCounter*6;
    for (i = 0; i < compressedCounter; i++)
    {
        compressedNodesData[i].nodePtr->nodeMemAddr = compressedOffset +i;
        if ( (compressedNodesData[i].isMcRangeExist == GT_TRUE) &&
             (compressedNodesData[i].fullEmbeddedLeaf == GT_TRUE) )
        {
            for (j = 0; j < compressedNodesData[i].mcLeavesNumber; j++)
            {
                leafLineOffset = gonOffset + leafCounter/MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;

                indexInLine = compressedNodesData[i].mcLeavesOrderIdArray[j]%MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;
                /* set in src hw line that points on src */
                mcRange = compressedNodesData[i].mcRanges[j];
                if ( !((mcRange->lowerLpmPtr.nextBucket->hwBucketOffsetHandle == 0)||
                     (mcRange->lowerLpmPtr.nextBucket->numOfRanges == 1)) )
                {
                    mcRange->lowerLpmPtr.nextBucket->pointingRangeMemAddr = compressedOffset + i;
                    /* set in src offset of pointer inside of line*/
                    mcRange->lowerLpmPtr.nextBucket->fifthAddress = (89 - indexInLine*23)+2;
                }
            }
        }
    }
    cpssOsFree(compressedNodesData);
    cpssOsFree(regularNodesData);
    return GT_OK;
}


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
)
{
    GT_U32 leafCounter = 0;
    GT_U32 compressedCounter = 0;
    GT_U32 regularCounter = 0;
    GT_U32 numberOfCompressedNodesBeforeRange = 0;

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *rangePtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *updatedRangePtr = NULL;
    rangePtr = bucketPtr->rangeList;
    switch (bucketPtr->bucketType)
    {
    case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        break;
    case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
        /* choose needed range for subnode */
        while (rangePtr !=NULL)
        {
            if (rangePtr->startAddr >= startSubNodeAddress)
            {
                break;
            }
            else
            {
                /* check if the range incorporate needed range */
                if (rangePtr->next != NULL)
                {
                    if ((rangePtr->next->startAddr - 1) >= startSubNodeAddress)
                    {
                        /* It must be start from this range */
                        break;
                    }
                }
            }
            rangePtr = rangePtr->next;
        }
    }
    while (rangePtr != NULL)
    {
        if (bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            if (rangePtr->startAddr > endSubNodeAddress)
            {
                break;
            }
        }
        if (rangePtr->startAddr == rangeStartAddr)
        {
            updatedRangePtr = rangePtr;

            if (rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                /* for this case the only number of regular before this range is interesting*/
                break;
            }
            else
                if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                    (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
                    (rangePtr->pointerType == 0xff) )
                {
                    /* this is illegal case */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "rangePtr->pointerType : Range poining to leaf is not relevant object! ");
                }
                else
                {
                    /* in case of compressed all number of regular and number of compressed before the range*/
                    numberOfCompressedNodesBeforeRange = compressedCounter;
                }
        }
        if (rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
           regularCounter++;
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
        {
            compressedCounter++;
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )
        {
            leafCounter++;
        }
        rangePtr = rangePtr->next;
    }
    if (updatedRangePtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "updatedRangePtr : Can't find updated range! ");

    }
    if (updatedRangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
        *nodeHwAddrPtr = hwGroupOffset + 6*regularCounter;
    }
    else
    {
        *nodeHwAddrPtr = hwGroupOffset+6*regularCounter+numberOfCompressedNodesBeforeRange;
    }
    return GT_OK;
}

/**
* @internal lpmFalconUpdateRangeInHw function
* @endinternal
*
* @brief This function creates a mirrored group of nodes in CPU's memory, and updates
*        the lpm structures in PP's memory.
*
* @param[in]      bucketPtr             - pointer to the bucket to create a mirror from.
* @param[in]      pRange                - pointer to range list
* @param[in]      shadowPtr             - lpm shadow ptr
*
* @retval   GT_OK on success, or
* @retval   GT_OUT_OF_CPU_MEM - if failed to allocate CPU memory, or
* @retval   GT_OUT_OF_PP_MEM - if failed to allocate PP memory.
*/
static GT_STATUS lpmFalconUpdateRangeInHw
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC         *bucketPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *pRange,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                *shadowPtr
)
{
    GT_U32 tempAddr;
    GT_U32 j = 0;
    GT_STATUS retVal;
    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
    GT_U32 shareDevListLen;
    /*GT_U8 startSubnodeId = 0;
     GT_U8 endSubnodeId = 0;*/
    GT_U8 subNodeId = 0;
    GT_UINTPTR groupStartAddrHandle = 0;
    GT_U32     groupOfNodesLines;

    cpssOsMemSet(&(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_2)), 0,
                 sizeof(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_2)));
    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;


    while (pRange != NULL)
    {
        /* only if the range needs updating , update it */
        if (pRange->updateRangeInHw == GT_TRUE)
        {
            /* calculate group of nodes start address*/
            switch (bucketPtr->bucketType)
            {
            case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
            case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
            case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
            case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
                subNodeId = 0;
                groupStartAddrHandle = bucketPtr->hwGroupOffsetHandle[0];
                break;
            case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
                subNodeId = pRange->startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
                groupStartAddrHandle = bucketPtr->hwGroupOffsetHandle[subNodeId];
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
            }

            /* there is a leaf case*/
            if( (pRange->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)||
                (pRange->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) )
            {
                if (bucketPtr->hwGroupOffsetHandle[subNodeId]==0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected bucketPtr->hwGroupOffsetHandle[subNodeId]=0\n");
                }

                tempAddr = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[subNodeId]);
                retVal = getMirrorGroupOfNodesDataAndUpdateRangesAddress(bucketPtr->bucketType,
                                                                         startSubNodeAddress[subNodeId],
                                                                         endSubNodeAddress[subNodeId],
                                                                         bucketPtr->rangeList,
                                                                         shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                                         tempAddr,
                                                                         &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_2)));
                if (retVal != GT_OK)
                {
                    return retVal;
                }
            }
            else
            {
                switch (pRange->lowerLpmPtr.nextBucket->bucketType)
                {
                case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
                case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
                case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
                case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
                    retVal =  lpmFalconFillCompressedData(pRange->lowerLpmPtr.nextBucket,
                                                          shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                          &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_2).compressedNodesArray[0]),
                                                          NULL);
                    if (retVal != GT_OK)
                    {
                        return retVal;
                    }
                    break;
                case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
                    retVal =  lpmFalconFillRegularData(pRange->lowerLpmPtr.nextBucket,
                                                       shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                       &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_2).regularNodesArray[0]));
                    if (retVal != GT_OK)
                    {
                        return retVal;
                    }
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
                }

                if (groupStartAddrHandle==0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected groupStartAddrHandle=0\n");
                }

                /* now calculate offset inside of group of nodes for node this range pointed on */
                retVal = lpmFalconGetHwNodeOffsetInsideGroupOfNodes(bucketPtr,
                                                                    pRange->startAddr,
                                                                    startSubNodeAddress[subNodeId],
                                                                    endSubNodeAddress[subNodeId],
                                                                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(groupStartAddrHandle),
                                                                    &tempAddr);
                if (retVal != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                pRange->lowerLpmPtr.nextBucket->nodeMemAddr = tempAddr;
            }

            if (groupStartAddrHandle==0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected groupStartAddrHandle=0\n");
            }

            /* tempAddr shows where this node starts in group of nodes*/
            for (j = 0; j < shareDevListLen; j++)
            {
                retVal =  prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[j],
                                                          tempAddr,
                                                          &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_2)),
                                                          &groupOfNodesLines);
                if (retVal != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
            }
            /*  remove the flag */
            pRange->updateRangeInHw = GT_FALSE;
            cpssOsMemSet(&(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_2)), 0,
                         sizeof(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_2)));
        }
        pRange = pRange->next;
    }
    return GT_OK;
}

/**
 * @internal lpmFalconGetNumOfLeaves funciton
 * @endinternal
 *
 * @param[in] bucketPtr - pointer to the shadow bucket.
 *
 * @retval number of leaves value
 */
GT_U32 lpmFalconGetNumOfLeaves
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr
)
{
    GT_U32 numOfLeaves = 0;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC      *rangePtr = NULL;
    /* calculate leaves number*/

    rangePtr = bucketPtr->rangeList;
    while (rangePtr != NULL)
    {
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )
        {
            numOfLeaves++;
        }
        rangePtr = rangePtr->next;
    }

    return numOfLeaves;
}

/**
* @internal lpmFalconGetCompressedBucketType function
* @endinternal
*
* @brief Get exact type of compressed bucket
*
* @param[in]       bucketPtr                     - Pointer to the bucket to create a mirror from.
* @param[out]       newCompressedBucBucketTypePtr - The new compressed type of the mirrored bucket.
*/
GT_VOID lpmFalconGetCompressedBucketType
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    OUT CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     *newCompressedBucBucketTypePtr
)
{
    GT_U32 numOfRanges = bucketPtr->numOfRanges;
    GT_U32 numOfLeaves = lpmFalconGetNumOfLeaves(bucketPtr);

    if ((numOfRanges <=7) && (numOfLeaves == 1))
    {
        *newCompressedBucBucketTypePtr = CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E;
    }
    else if ((numOfRanges <=5) && (numOfLeaves == 2))
    {
        *newCompressedBucBucketTypePtr = CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E;
    }
    else if ((numOfRanges ==3) && (numOfLeaves == 3))
    {
        *newCompressedBucBucketTypePtr = CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E;
    }
    else
    {
        *newCompressedBucBucketTypePtr = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;
    }
}



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
)
{
    GT_U32 i,j = 0;
    GT_STATUS rc = GT_OK;
    GT_U32 *hwGonDataArr=NULL;
    GT_U32 gonReadOffset;
    GT_U32 lpmGonWriteOffset;
    GT_U32 gonSize;
    GT_U32 numberOfGons;
    GT_U32 gonStartIndex;
    GT_UINTPTR gonNodePtr;
    if (newGonsOffsets == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "LPM parentNodeData bad state");
    }

    if ( (hwGroupOffsetHandle == NULL) && (hwRootNodeOffsetHandle == 0xffffffff))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "updateSwapArea bad state");
    }

    hwGonDataArr =  (GT_U32 *)cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_GROUP_OF_NODES_SIZE_IN_WORDS_CNS*sizeof(GT_U32));
    if (hwGonDataArr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(hwGonDataArr,0,PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_GROUP_OF_NODES_SIZE_IN_WORDS_CNS*sizeof(GT_U32));


    if (swapSavingType == PRV_CPSS_DXCH_LPM_RAM_ALL_GONS_SWAP_INDEX_CNS)
    {
        numberOfGons = PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS;
        gonStartIndex = 0;
    }
    else
    {
        gonStartIndex = bvLineIndex;
        numberOfGons = gonStartIndex+1;
    }
    lpmGonWriteOffset = swapOffset;
    /* we are going to take all GONs in single SWAP*/
    for (i = gonStartIndex; i < numberOfGons; i++)
    {
        /* read data from LPM memory*/

        if (hwGroupOffsetHandle != NULL)
        {
            gonNodePtr = hwGroupOffsetHandle[i];
        }
        else
        {
            gonNodePtr = hwRootNodeOffsetHandle;
        }
        if (gonNodePtr==0)
        {
            cpssOsFree(hwGonDataArr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected gonNodePtr=0\n");
        }

        gonReadOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(gonNodePtr);
        gonSize = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(gonNodePtr);

        for (j = 0; j < shareDevsListLen; j++)
        {

            rc = prvCpssDxChReadTableMultiEntry(shareDevsList[j],
                                                 CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                 gonReadOffset,
                                                 gonSize,
                                                 hwGonDataArr);
            if (rc != GT_OK)
            {
                cpssOsFree(hwGonDataArr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "LPM read fails");
            }

            /* Write the node */
            rc = prvCpssDxChSip6RamMngWriteMultiEntry(shareDevsList[j],
                                             CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                             lpmGonWriteOffset,
                                             gonSize,
                                             hwGonDataArr);
            if (rc != GT_OK)
            {
                cpssOsFree(hwGonDataArr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "LPM write fails");
            }
        }
        newGonsOffsets[i] = lpmGonWriteOffset;
        lpmGonWriteOffset = lpmGonWriteOffset + gonSize;
    }

    cpssOsFree(hwGonDataArr);
    return rc;
}

/**
 * @internal updateMirrorGroupOfNodes function
 * @endinternal
 *
 * @brief This function creates a mirrored group of nodes in CPU's memory, and updates
 *        the lpm structures in PP's memory.
 *
 * @param[in] bucketPtr             - Pointer to the bucket to create a mirror from.
 * @param[in] bucketUpdateMode      - Indicates whether to write the table to the device's
 *                                    Ram, update the memory alloc , or do nothing.
 * @param[in] isDestTreeRootBucket  - Indicates whether the bucket is the root of the
 *                                    destination address tree
 * @param[in] isSrcTreeRootBucket   - Indicates whether the bucket is the root SRC of the
 *                                    destination address tree
 * @param[in] resizeBucket          - Indicates whether the bucket was resized or not
 *                                    during the insertion / deletion process.
 * @param[in] forceWriteWholeBucket - force writing of the whole bucket and not just
 *                                    the ranges where the writeRangeInHw is set.
 * @param[in] indicateSiblingUpdate - whether to update siblings of buckets (used in
 *                                    bulk mode)
 * @param[in] lpmEngineMemPtrPtr    - points to a an PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
 *                                    which holds all the memory information needed
 *                                    for where and how to allocate search memory.
 * @param[inout] bucketTypePtr      - The bucket type of the mirrored bucket.
 * @param[in] shadowPtr             - the shadow relevant for the devices asked to act on.
 * @param[in] parentWriteFuncPtr    - the bucket's parent write function in case there is
 *                                    a need to update the packet's parent ptr data and
 *                                    the parent is not a LPM trie range.
 *                                    (relevant only in delete operations)
 * @param[in] subnodesSizesPtr       - pointer to subnodes gon sizes
 * @param[in] subnodesIndexesPtr     - pointer to subnodes array showing subnodes going to change.
 *
 * @retval  GT_OK on success, or
 * @retval  GT_OUT_OF_CPU_MEM - if failed to allocate CPU memory, or
 * @retval  GT_OUT_OF_PP_MEM  - if failed to allocate PP memory.
 *
*/
static GT_STATUS updateMirrorGroupOfNodes
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    IN LPM_BUCKET_UPDATE_MODE_ENT               bucketUpdateMode,
    IN GT_BOOL                                  isDestTreeRootBucket,
    IN GT_BOOL                                  isSrctTreeRootBucket,
    IN LPM_ROOT_BUCKET_UPDATE_ENT               rootBucketState,
    IN GT_BOOL                                  resizeGroup,
    IN GT_BOOL                                  numOfRangesIsChanged,
    IN GT_BOOL                                  overwriteGroup,
    IN GT_BOOL                                  forceWriteWholeBucket,
    IN GT_BOOL                                  indicateSiblingUpdate,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC       **lpmEngineMemPtrPtr,
    INOUT CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT   *bucketTypePtr,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC         *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC    *parentNodeData,
    IN GT_BOOL                                  *subnodesIndexesPtr,
    IN GT_U32                                   *subnodesSizesPtr
)
{
    GT_STATUS retVal = GT_OK;
    GT_STATUS retVal1 = GT_OK;
    GT_U32 i,j,k;
    GT_UINTPTR oldMemPool[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS]= {0};  /* The memory pool from which the old buckets   */

    GT_UINTPTR oldMemPoolSelected=0;

    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
    GT_U32 shareDevListLen;
    GT_UINTPTR tempHwRootAddrHandle = 0;
    GT_UINTPTR tempHwAddrHandle[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};/* Will hold the allocated area in PP's RAM to  */
                                /* which the buckets will be written.           */
    GT_UINTPTR tempHwAddrHandleToBeFreed[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};/* Will hold the allocated area in PP's RAM */
                                                                                                  /* that should be freed                     */

    GT_BOOL needToFreeAllocationInCaseOfFail[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};

    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempNextMemInfoPtr = NULL;/* use for going over the list of blocks per octet */
    GT_UINTPTR                           tmpStructsMemPool = 0;/* use for going over the list of blocks per octet */

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRange = NULL;
    GT_U32 tempAddr[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};     /* Temporary address                */

    PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC     **freeMemListPtr;
    GT_BOOL swapMemInUse = GT_FALSE;
    GT_BOOL secondSwapMemInUse = GT_FALSE;
    GT_U32 swapFirstIndex = PRV_CPSS_DXCH_LPM_RAM_ILLEGAL_SWAP_INDEX_CNS;
    GT_U32 swapSecondIndex = PRV_CPSS_DXCH_LPM_RAM_ILLEGAL_SWAP_INDEX_CNS;
    GT_U32 bvLineIndex=0;

    GT_BOOL swapMemInUseForAdd[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0}; /* indicate that the memory allocated for ADD operation was taken from the swap memory */
    GT_U32  swapMemIndexInUseForAdd[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0}; /* swap memory used: swap1/swap2/swap3*/
    GT_UINTPTR swapMemToUseOldHandleAddr[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS]={0};/* to old memory we should move to the swap */
    GT_U32 swapMemToUseOldHandleGonIndex[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS]={0};/* to old memory gon index we should move to the swap */
    GT_BOOL swapMemInUseForAddGlobalFlag=GT_FALSE;
    GT_BOOL oneOldWhenComToReg=GT_FALSE;/* when compress become regular we only have one old handle*/

    GT_BOOL freeOldMem = GT_TRUE;
    GT_BOOL groopsAllocWasDone = GT_FALSE;
    GT_BOOL srcTreeRootNodeIsHandledbySwapFunc = GT_FALSE;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT newBucketType;
    GT_UINTPTR oldHwAddrHandle[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];
    GT_U32 groupOfNodesNum = 1;
    GT_BOOL updatePtrToNewAllocatedAfterSwap = GT_FALSE;
    GT_BOOL convertFromRegularToCompress = GT_FALSE;
    GT_BOOL convertFromCompressToRegular = GT_FALSE;
    GT_U32  parentStartRangeBitVectorLocation=0;

    GT_U32      blockIndex=0; /* calculated according to the memory offset devided by block size including gap */
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT oldBucBucketType;

    GT_BOOL     freeBlockCanBeFoundRegToCompSwapUse = GT_FALSE;
    GT_BOOL     freeBlockCanBeFound = GT_FALSE;
    GT_U32      freeBlockGonIndex;
    GT_U32      bankIndexForShrink=0xFFFF;
    GT_BOOL     swapUsedForShrink = GT_FALSE;
    GT_U32      groupOfNodesLines;

    indicateSiblingUpdate = indicateSiblingUpdate; /* prevent warning */
    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;
    if (parentNodeData != NULL)
    {
        cpssOsMemSet(parentNodeData->swapGonsAdresses, 0xff, sizeof(parentNodeData->swapGonsAdresses));
    }

    oldBucBucketType = bucketPtr->bucketType;

    /* Determine the bucket type of the new bucket. */
    if ((bucketPtr->numOfRanges == 1) && (isDestTreeRootBucket == GT_FALSE))
    {
        newBucketType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
    }
    else
        if((bucketPtr->numOfRanges <= MAX_NUMBER_OF_COMPRESSED_RANGES_CNS)&&    /*  compressed bucket */
            (!((parentNodeData->addOperation == GT_TRUE)&& (bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E))) )
    {
        /* check exact compressed type */
        lpmFalconGetCompressedBucketType(bucketPtr,&newBucketType);
    }
    else                                    /* Regular bucket           */
    {
        newBucketType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
        groupOfNodesNum = PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS;
    }

    parentNodeData->addOperation = GT_FALSE;
    if ( (bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E) &&
         ((newBucketType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                        ||
          (newBucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)  ||
          (newBucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E)||
          (newBucketType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E)      ||
          (newBucketType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)))
    {
        convertFromRegularToCompress = GT_TRUE;
    }
    if ( (bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E) &&
         (newBucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E) )
    {
        convertFromCompressToRegular = GT_TRUE;
    }

    if ( (bucketUpdateMode == LPM_BUCKET_UPDATE_SHADOW_ONLY_E) ||
         (bucketUpdateMode == LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E) )
    {
        /* this means we are in a stage where we just update the shadow
           the only released memory here is that of empty buckets */
        freeMemListPtr = &shadowPtr->freeMemListEndOfUpdate;
    }
    else
    {
        /* check what kind of memory this bucket occupies according to it HW update
           status */
        freeMemListPtr = (bucketPtr->bucketHwUpdateStat ==
                          PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_WAS_RESIZED_AND_NEEDS_REWRITING_E)?
            &shadowPtr->freeMemListDuringUpdate:
            &shadowPtr->freeMemListEndOfUpdate;
    }
    if ( bucketUpdateMode == LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E )
    {
        /* This is delete operation and bucket going to be compressed with embedded leaves:
                   3 ranges 3 leaves (last level). It means we need to free memory and updates shadow */
        for (i = 0; i <  PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
        {
            if (resizeGroup == GT_TRUE)
            {

                if (bucketPtr->hwGroupOffsetHandle[i] == 0)
                {
                    cpssOsPrintf("BAD STATE:Last level, resize = TRUE,bucketPtr->hwGroupOffsetHandle[0] =0\n");
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "updateMirrorGroupOfNodes");
                }
                /* swap memory is a pre-allocated memory that should never be freed */
                if ((bucketPtr->hwGroupOffsetHandle[i]!=shadowPtr->swapMemoryAddr)&&
                    (bucketPtr->hwGroupOffsetHandle[i]!=shadowPtr->secondSwapMemoryAddr)&&
                    (bucketPtr->hwGroupOffsetHandle[i]!=shadowPtr->thirdSwapMemoryAddr))
                {
                    retVal = prvCpssDxChLpmRamMemFreeListMng(bucketPtr->hwGroupOffsetHandle[i],
                                                             PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E,
                                                             freeMemListPtr,shadowPtr);
                    if (retVal != GT_OK)
                    {
                       CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "prvCpssDxChLpmRamMemFreeListMng failed");
                    }
                }

            }
            if (convertFromRegularToCompress == GT_FALSE)
            {
                break;
            }
        }
        resizeGroup = GT_FALSE;
    }

    /* If there is only one range (next hop type), then delete the
       whole bucket */
    if(newBucketType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
    {
        if (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
        {
            for (i = 0; i <  PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
            {
                /* in some cases bucket is not pointing on GON, so there is no need to free */
                if (bucketPtr->hwGroupOffsetHandle[i] != 0)
                {
                     /* swap memory is a pre-allocated memory that should never be freed */
                    if ((bucketPtr->hwGroupOffsetHandle[i]!=shadowPtr->swapMemoryAddr)&&
                        (bucketPtr->hwGroupOffsetHandle[i]!=shadowPtr->secondSwapMemoryAddr)&&
                        (bucketPtr->hwGroupOffsetHandle[i]!=shadowPtr->thirdSwapMemoryAddr))
                    {
                        retVal = prvCpssDxChLpmRamMemFreeListMng(bucketPtr->hwGroupOffsetHandle[i],
                                                                 PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E,
                                                                 freeMemListPtr,shadowPtr);
                        if (retVal != GT_OK)
                        {
                             CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "prvCpssDxChLpmRamMemFreeListMng failed");
                        }
                    }
                }
                if (convertFromRegularToCompress == GT_FALSE)
                {
                    break;
                }
            }
        }

        bucketPtr->bucketType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        /* in case of PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E pointer type
           shouldn't be updated because it represents that it points to a src
           trie (and not the actuall type of the bucket) */
        if ((GT_U32)*bucketTypePtr != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)
            *bucketTypePtr = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        if (isSrctTreeRootBucket == GT_TRUE)
        {
            /* src tree root bucket*/
            if (bucketPtr->hwBucketOffsetHandle != 0)
            {
                /* swap memory is a pre-allocated memory that should never be freed */
                if ((bucketPtr->hwBucketOffsetHandle!=shadowPtr->swapMemoryAddr)&&
                    (bucketPtr->hwBucketOffsetHandle!=shadowPtr->secondSwapMemoryAddr)&&
                    (bucketPtr->hwBucketOffsetHandle!=shadowPtr->thirdSwapMemoryAddr))
                {
                    retVal = prvCpssDxChLpmRamMemFreeListMng(bucketPtr->hwBucketOffsetHandle,
                                                             PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E,
                                                             freeMemListPtr,shadowPtr);
                    if (retVal != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "prvCpssDxChLpmRamMemFreeListMng failed");
                    }
                }
            }
        }
        bucketPtr->hwBucketOffsetHandle = 0;
        cpssOsMemSet(bucketPtr->hwGroupOffsetHandle,0,sizeof(bucketPtr->hwGroupOffsetHandle));
        return GT_OK;
    }


    /* check the case of no hw update */
    if((bucketUpdateMode == LPM_BUCKET_UPDATE_ROOT_BUCKET_SHADOW_ONLY_E)  ||
       (bucketUpdateMode == LPM_BUCKET_UPDATE_NONE_E))
    {
        /* all buckets that are visited during the shadow update stage of
           the bulk operation should be scanned during the hardware update stage;
           therefore don't leave buckets with PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E
           mark */
        if ((indicateSiblingUpdate == GT_TRUE) &&
            (bucketPtr->bucketHwUpdateStat == PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E))
        {
            /*indicate we need to go through this bucket in bulk update in order
              to reach it's siblings for update. */
            bucketPtr->bucketHwUpdateStat =
                PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_UPDATE_SIBLINGS_E;
        }

        if (bucketUpdateMode == LPM_BUCKET_UPDATE_ROOT_BUCKET_SHADOW_ONLY_E)
        {
            /* for root bucket type we need to update the bucket type */
            *bucketTypePtr = newBucketType;
        }

        /* update the bucket type and exit */
        bucketPtr->bucketType = *bucketTypePtr;
        return GT_OK;
    }

    if ((bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E) &&
        (bucketPtr->bucketHwUpdateStat ==
         PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_WAS_RESIZED_AND_NEEDS_REWRITING_E))
    {
        /* we are in a bulk operation update (delete) and this bucket
           was already found to be resized , so we refer to it as need to be
           resized since it's memory hasn't been resized yet*/
        resizeGroup = GT_TRUE;
        rootBucketState = LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E;
    }

    /* check if any allocation was done for group of nodes */
    for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
    {
        if ((bucketPtr->hwGroupOffsetHandle[i] != 0) && (bucketPtr->hwGroupOffsetHandle[i] != DMM_BLOCK_NOT_FOUND))
        {
            groopsAllocWasDone = GT_TRUE;
            break;
        }
        if (bucketPtr->bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            break;
        }
    }

    if(((bucketUpdateMode != LPM_BUCKET_UPDATE_SHADOW_ONLY_E) && (bucketUpdateMode != LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E)) &&
       (( resizeGroup == GT_TRUE) || (groopsAllocWasDone == GT_FALSE)))
    {
        /* first check if memory was already allocated for these changes */
        /* if yes , use only that memory! */
        if (shadowPtr->neededMemoryListLen > 0)
        {
            i = shadowPtr->neededMemoryCurIdx;
            if (i >= shadowPtr->neededMemoryListLen)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* should not happen */
            /* Add another check for size : new regular, new compressed,*/
            cpssOsMemCpy(tempHwAddrHandle,shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks,
                         sizeof(tempHwAddrHandle));
            cpssOsMemCpy(swapMemInUseForAdd,shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksSwapUsedForAdd,
                         sizeof(swapMemInUseForAdd));
            cpssOsMemCpy(swapMemIndexInUseForAdd,shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksSwapIndexUsedForAdd,
                         sizeof(swapMemIndexInUseForAdd));
            cpssOsMemCpy(swapMemToUseOldHandleAddr,shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksOldHandleAddr,
                         sizeof(swapMemToUseOldHandleAddr));

            cpssOsMemCpy(swapMemToUseOldHandleGonIndex,shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksOldHandleGonIndex,
                         sizeof(swapMemToUseOldHandleGonIndex));


            /* if swap memory should be used for add we need to update parameters:
               IN GT_BOOL                                  *subnodesIndexesPtr,
               IN GT_U32                                   *subnodesSizesPtr
               in case of ADD those parameters are not initialized in prvCpssDxChLpmSip6RamMngInsert */
            for (k = 0; k < groupOfNodesNum; k++)
            {
                if (swapMemInUseForAdd[k] == GT_TRUE)
                {
                   swapMemInUseForAddGlobalFlag = GT_TRUE;/* set a flag that swap is used for add in one of the GONs*/
                   switch (swapMemIndexInUseForAdd[k])
                    {
                    case 1:
                        /* tempHwAddrHandle has already the value of the swapMemoryAddr,
                           given in the prvCpssDxChLpmSip6RamMngAllocAvailableMemCheck
                           tempHwAddrHandle[k] = shadowPtr->swapMemoryAddr;*/

                        /*in case we have a change of a bucket from compress to regular,
                          then for 6 new GONs we have a single old bucket.
                          so we found in previous stages who is the GON out of the 6
                          that will have an old bucket pointer to be used by the swap.
                          this old is always located as the first element in the array,
                          so even if k!=0 in the loop its old index is 0*/
                        if (shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksOldHandleGonIndex[k]==0xFFFE)
                        {
                            swapFirstIndex=0;
                            bvLineIndex = 0;
                        }
                        else
                        {
                            swapFirstIndex = k;
                        }
                        subnodesIndexesPtr[k]=GT_TRUE;
                        subnodesSizesPtr[k]=shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksSizes[k];
                        break;
                    case 2:
                     /* tempHwAddrHandle has already the value of the swapMemoryAddr,
                           given in the prvCpssDxChLpmSip6RamMngAllocAvailableMemCheck
                           tempHwAddrHandle[k] = shadowPtr->secondSwapMemoryAddr;*/

                        /*in case we have a change of a bucket from compress to regular,
                          then for 6 new GONs we have a single old bucket.
                          so we found in previous stages who is the GON out of the 6
                          that will have an old bucket pointer to be used by the swap.
                          this old is always located as the first element in the array,
                          so even if k!=0 in the loop its old index is 0*/
                        if (shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksOldHandleGonIndex[k]==0xFFFE)
                        {
                            swapSecondIndex=0;
                            bvLineIndex=0;
                        }
                        else
                        {
                            swapSecondIndex = k;
                        }
                        subnodesIndexesPtr[k]=GT_TRUE;
                        subnodesSizesPtr[k]=shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksSizes[k];
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "no free swap memory");
                    }
                }
            }

            shadowPtr->neededMemoryCurIdx++;
        }
        else
        {
            tempNextMemInfoPtr = lpmEngineMemPtrPtr[0];
            tmpStructsMemPool = lpmEngineMemPtrPtr[0]->structsMemPool;
            /*  this can only happen in lpm Delete, since in an insert the memory is preallocated.
               if we here gon must be reallocated */
            /* calculate new gon size : take relevant from subnodesIndexesPtr, subnodesSizesPtr*/

            for (i = 0; i < groupOfNodesNum; i++)
            {
                if (newBucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                {
                    if (subnodesIndexesPtr[i] == GT_FALSE)
                    {
                        continue;
                    }
                }
                if (lpmEngineMemPtrPtr[0]->structsMemPool == 0)
                {
                    tempHwAddrHandle[i] = DMM_BLOCK_NOT_FOUND;
                    cpssOsPrintf(" \n !!!!!!!There is no bank list for given octet\n");
                }
                else
                {
                    do
                    {
                        tempHwAddrHandle[i] = prvCpssDmmAllocate(tmpStructsMemPool,
                                                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * subnodesSizesPtr[i],
                                                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);
                        if (tempNextMemInfoPtr->nextMemInfoPtr == NULL)
                        {
                            break;
                        }
                        tempNextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                        tmpStructsMemPool = tempNextMemInfoPtr->structsMemPool;

                    } while (tempHwAddrHandle[i]==DMM_BLOCK_NOT_FOUND);
                }
                if ((tempHwAddrHandle[i] == DMM_BLOCK_NOT_FOUND)||(tempHwAddrHandle[i]==0))
                {
                    /* we reached a max fragmentation in the PP's memory*/
                    /* this can only happen in lpm Delete, since in an insert */
                    /* the memory is preallocated. */
                    /* this forces use to use the swap memory , which we will*/
                    /* swap back after using */

                    if ( ( (newBucketType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
                           (newBucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
                           (newBucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
                           (newBucketType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E))      &&
                           (parentNodeData->isBvCompConvertion == GT_TRUE) )
                    {
                        retVal = prvCpssDxChLpmSip6RamMngFindIfSwapAreaCanBeUsefulToFindFreeBlockForRegToCompConversion(bucketPtr,
                                                                                                                        subnodesSizesPtr[i],
                                                                                                                        &freeBlockCanBeFoundRegToCompSwapUse,
                                                                                                                        &freeBlockGonIndex);
                        if (retVal != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM, " swap will not help - no more memory \n");
                        }

                        if (freeBlockCanBeFoundRegToCompSwapUse == GT_TRUE)
                        {
                           /* all gons will be moved later to the SWAP area.
                              freeBlockGonIndex will be used to allocate the needed memory in the appropriate bank*/
                            swapFirstIndex = PRV_CPSS_DXCH_LPM_RAM_ALL_GONS_SWAP_INDEX_CNS;
                        }
                        else
                        {
                            /* check if shrink migth help to find new space for compress */
                            retVal = prvCpssDxChLpmSip6RamMngCheckIfShrinkOperationUsefulForDefragForRegToCompConversion(shadowPtr,
                                                                                                                         lpmEngineMemPtrPtr[0],/* the first element allready fit to the correct level*/
                                                                                                                         parentNodeData->protocol,
                                                                                                                         bucketPtr->hwGroupOffsetHandle,
                                                                                                                         subnodesSizesPtr[i],
                                                                                                                         &freeBlockCanBeFound,
                                                                                                                         &bankIndexForShrink,
                                                                                                                         &swapUsedForShrink);
                            if (retVal != GT_OK)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, " shrink will not help - fatal error fail \n");
                            }
                            if (freeBlockCanBeFound==GT_TRUE)
                            {
                                /* shrink the bank */
                                retVal = prvCpssDxChLpmSip6RamMngShrinkAllGonsRegToCompressBank(shadowPtr,
                                                        bucketPtr->hwGroupOffsetHandle,
                                                        bankIndexForShrink,
                                                        subnodesSizesPtr[i]*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS,/* the total size needed from a specific bank for all GONs in this bank */
                                                        swapUsedForShrink,
                                                        parentNodeData,
                                                        needToFreeAllocationInCaseOfFail,
                                                        tempHwAddrHandleToBeFreed);
                                if (retVal!=GT_OK)
                                {
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"error in prvCpssDxChLpmSip6RamMngShrinkAllGonsRegToCompressBank \n");
                                }
                                /* reset fields used in prvCpssDxChLpmSip6RamMngShrinkAllGonsRegToCompressBank*/
                                parentNodeData->funcCallCounter=0;

                                if(swapUsedForShrink==GT_TRUE)
                                {
                                    /* swap will help to find new block after we did shrink */
                                    tempHwAddrHandle[i] = shadowPtr->swapMemoryAddr;
                                    swapFirstIndex = PRV_CPSS_DXCH_LPM_RAM_ALL_GONS_SWAP_INDEX_CNS;
                                }
                                else
                                {
                                   /* memory can be found without using the swap
                                      try to allocate the memory - should pass  */
                                    tempHwAddrHandle[i] = prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[bankIndexForShrink],
                                                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * subnodesSizesPtr[i],
                                                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);

                                    if ((tempHwAddrHandle[i] == DMM_BLOCK_NOT_FOUND)||(tempHwAddrHandle[i]==0))
                                    {
                                        if(tempHwAddrHandle[i] == DMM_BLOCK_NOT_FOUND)
                                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM, "no more free space - fail after shrink");
                                        else/*(tempHwAddrHandle[i]==0)*/
                                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "bad allocation - fail after shrink");
                                    }

                                    needToFreeAllocationInCaseOfFail[i] = GT_TRUE;
                                    tempHwAddrHandleToBeFreed[i] = tempHwAddrHandle[i];

                                    /*  set pending flag for future need */
                                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[i])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                    retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                                    if (retVal1!= GT_OK)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                                    }
                                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
                                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                                        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandle[i]);

                                    continue;/* no need to get to the swap code*/
                                }
                            }
                            else
                            {
                                /* if we can not find free block for the compress then we should leave the regular
                                   delete operation should not fail in this case */
                                parentNodeData->funcCallCounter = 3;
                                retVal = updateHwRangeDataAndGonPtr(parentNodeData);
                                if (retVal != GT_OK)
                                {
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "bad swap update");
                                }
                                parentNodeData->funcCallCounter = 0;
                                parentNodeData->isBvCompConvertion = GT_FALSE;
                                return GT_OK;
                            }
                        }
                    }

                     /* swap will help to find new block */
                    if ((swapMemInUse == GT_TRUE) || (secondSwapMemInUse == GT_TRUE))
                    {
                        if (secondSwapMemInUse == GT_FALSE)
                        {
                            /* first swap is already occupied */
                            tempHwAddrHandle[i] = shadowPtr->secondSwapMemoryAddr;
                            secondSwapMemInUse = GT_TRUE;
                            swapSecondIndex = i;
                        }
                        else
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "no free swap memory");
                        }
                    }
                    else
                    {
                        tempHwAddrHandle[i] = shadowPtr->swapMemoryAddr;
                        swapMemInUse = GT_TRUE;
                        if ( ( (newBucketType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
                               (newBucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
                               (newBucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
                               (newBucketType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E))      &&
                               (parentNodeData->isBvCompConvertion == GT_TRUE) )
                        {
                            swapFirstIndex = PRV_CPSS_DXCH_LPM_RAM_ALL_GONS_SWAP_INDEX_CNS;
                        }
                        else
                        {
                            swapFirstIndex = i;
                        }
                    }
                    if (tempHwAddrHandle[i] == 0)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(((GT_STATUS)GT_ERROR), LOG_ERROR_NO_MSG);
                    }
                }
                else
                {
                    /* we had a CPU Memory allocation error */
                    if(tempHwAddrHandle[i] == DMM_MALLOC_FAIL)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }

                    needToFreeAllocationInCaseOfFail[i] = GT_TRUE;
                    tempHwAddrHandleToBeFreed[i] = tempHwAddrHandle[i];

                    /*  set pending flag for future need */
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[i])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                    if (retVal1 != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                    }
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandle[i]);
                }
            }
        }
    }
    else
    {
        cpssOsMemCpy(tempHwAddrHandle,bucketPtr->hwGroupOffsetHandle,sizeof(tempHwAddrHandle));
    }
    if ((isDestTreeRootBucket == GT_TRUE)||(isSrctTreeRootBucket == GT_TRUE))
    {
        /* root bucket */
        tempHwRootAddrHandle = bucketPtr->hwBucketOffsetHandle;
    }
    /* update the bucket's hw status */
    if (resizeGroup /*resizeBucket*/ == GT_TRUE)
    {
        bucketPtr->bucketHwUpdateStat =
            PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_WAS_RESIZED_AND_NEEDS_REWRITING_E;
    }
    else
    {
        if ((bucketPtr->bucketHwUpdateStat == PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E) ||
            (bucketPtr->bucketHwUpdateStat == PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_UPDATE_SIBLINGS_E))
        {
            /* the bucket wasn't resized , but we reached here so it needs
               re-writing in the HW (some pointers were changed in it) */
            if (bucketUpdateMode != LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E)
            {
                bucketPtr->bucketHwUpdateStat = PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_NEEDS_UPDATE_E;
            }
        }
    }

    /* Write the tables to PP's RAM for each device that share this LPM table. */

    if ((bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E) ||
        (bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E)||
        ( (bucketUpdateMode == LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E) &&
          ((isSrctTreeRootBucket == GT_TRUE) || (isDestTreeRootBucket == GT_TRUE)) &&
          (rootBucketState != LPM_ROOT_BUCKET_UPDATE_NONE_E)))
    {
        pRange = bucketPtr->rangeList;
        /* Preparation of HW words.     */

        if (bucketUpdateMode != LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E)
        {
            if ((resizeGroup /*resizeBucket*/ == GT_TRUE) ||
                (forceWriteWholeBucket == GT_TRUE) ||
                (bucketPtr->bucketHwUpdateStat ==
                 PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_WAS_RESIZED_AND_NEEDS_REWRITING_E) ||
                (numOfRangesIsChanged == GT_TRUE) || (overwriteGroup == GT_TRUE))
                /*it could be that this bucket was resized but haven't been written
                  the the HW*/
            {
                /* first get the bucket base addr for sibling buckets parent
                   range mem address calculation */
                /* how to undersatnd in case of regular in which line change occured ????*/
                if ( (swapMemInUse == GT_TRUE) || (secondSwapMemInUse == GT_TRUE) || (swapMemInUseForAddGlobalFlag==GT_TRUE))
                {
                    /* if we're using the swap we will not record it but record the
                       bucket's old memory address */
                    for (i=0; i<PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS;i++ )
                    {
                        /* check if swap area is used for add, if so need to record the old bucket's memory address */
                        if (swapMemInUseForAddGlobalFlag==GT_TRUE)
                        {
                            if (swapMemInUseForAdd[i] == GT_TRUE)
                            {
                                if ((swapMemToUseOldHandleAddr[i] == 0) || (swapMemToUseOldHandleAddr[i] == DMM_BLOCK_NOT_FOUND))
                                {
                                    /* cant happen since if flag is true we need to have an old address */
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(((GT_STATUS)GT_ERROR), "swapMemInUseForAddGlobalFlag is true but there is no old memory\n");
                                }
                                tempAddr[i] = (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(swapMemToUseOldHandleAddr[i]));
                            }
                            else
                            {
                                if ((tempHwAddrHandle[i] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[i] == 0))
                                {
                                    continue;
                                }
                                tempAddr[i] = (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[i]));
                            }
                        }
                        else
                        {
                            if ((bucketPtr->hwGroupOffsetHandle[i] == 0) || (bucketPtr->hwGroupOffsetHandle[i] == DMM_BLOCK_NOT_FOUND))
                            {
                                continue;
                            }
                            tempAddr[i] = (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[i]));
                        }
                    }
                }
                else
                {
                    for (i=0; i<groupOfNodesNum;i++ )
                    {
                        if ((tempHwAddrHandle[i] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[i] == 0))
                        {
                            continue;
                        }
                        tempAddr[i] = (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[i]));
                    }
                }
                cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_1), 0,
                             sizeof(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_1)));
                for (i = 0; i < groupOfNodesNum; i++)
                {
                    /* build hw data for affected group subnodes */
                    if (subnodesIndexesPtr[i] == GT_FALSE)
                    {
                        continue;
                    }
                    if ((tempHwAddrHandle[i] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[i] ==0))
                    {

                        if (bucketPtr->hwGroupOffsetHandle[i]==0)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected bucketPtr->hwGroupOffsetHandle[j]=0\n");
                        }
                        tempAddr[i] = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[i]);
                    }
                    else
                    {
                       tempAddr[i] = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[i]);
                    }

                    /*if ((swapFirstIndex != PRV_CPSS_DXCH_LPM_RAM_ALL_GONS_SWAP_INDEX_CNS) && ((i != swapFirstIndex) && (i != swapSecondIndex)) )*/
                    {
                        retVal = getMirrorGroupOfNodesDataAndUpdateRangesAddress(newBucketType,
                                                                                 startSubNodeAddress[i],
                                                                                 endSubNodeAddress[i],
                                                                                 pRange,
                                                                                 shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                                                 tempAddr[i],
                                                                                 &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_1)[i]));
                        if (retVal != GT_OK)
                        {
                            return retVal;
                        }
                    }
                }
                /* Writing to RAM.*/
                do
                {
                    for (j = 0; j < groupOfNodesNum; j++)
                    {
                        if (subnodesIndexesPtr[j] == GT_FALSE)
                        {
                            continue;
                        }
                        /* we have here 2 cases: one for using swap in delete and one for using swap in add
                           in both cases we should not enter the if case */
                        if (((swapMemInUseForAddGlobalFlag==GT_FALSE)&&(j != swapFirstIndex) && (j != swapSecondIndex) && (swapFirstIndex != PRV_CPSS_DXCH_LPM_RAM_ALL_GONS_SWAP_INDEX_CNS))||
                            ((swapMemInUseForAddGlobalFlag==GT_TRUE) && (swapMemInUseForAdd[j]==GT_FALSE)))
                        {
                            /* build hw data for affected group subnodes */
                            if ((tempHwAddrHandle[j] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[j] ==0))
                            {

                                if (bucketPtr->hwGroupOffsetHandle[j]==0)
                                {
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected bucketPtr->hwGroupOffsetHandle[j]=0\n");
                                }
                                tempAddr[j] = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[j]);
                            }
                            else
                            {
                               tempAddr[j] = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[j]);
                            }

                            for (i = 0; i < shareDevListLen; i++)
                            {

                                if((resizeGroup == GT_FALSE) &&
                                   ((numOfRangesIsChanged == GT_TRUE) || (overwriteGroup == GT_TRUE)))
                                {
                                    retVal = updateCurrentAndParentGons(newBucketType,
                                                                        bucketPtr,
                                                                        parentNodeData,
                                                                        &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_1)[j]),
                                                                        j);
                                }
                                else
                                {
                                    retVal =  prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[i],
                                                                              tempAddr[j],
                                                                              &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_1)[j]),
                                                                              &groupOfNodesLines);
                                }
                                if (retVal != GT_OK)
                                {
                                     /* this is a fail that do not support any rollback operation,
                                       because if we get here it is after we did all calculations
                                       needed and we verified that the data can be written to the HW.
                                       if we get a fail it means a fatal error that should not be
                                       happen and the behavior is unpredicted */

                                    /* just need to free any allocations done in previous stage */
                                    if((needToFreeAllocationInCaseOfFail[j] == GT_TRUE)&&
                                       (tempHwAddrHandleToBeFreed[j] != 0)&&
                                       (tempHwAddrHandleToBeFreed[j] != DMM_BLOCK_NOT_FOUND) &&
                                       (tempHwAddrHandleToBeFreed[j] != DMM_MALLOC_FAIL))
                                    {
                                        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandleToBeFreed[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                        retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                                        if (retVal1 != GT_OK)
                                        {
                                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                                        }
                                        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                                        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandleToBeFreed[j]);

                                        prvCpssDmmFree(tempHwAddrHandleToBeFreed[j]);
                                    }
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "gon update failed");
                                }
                            }
                        }
                        else
                        {
                            GT_U32 swapOffset = 0xffffffff;
                            /* this is a special case when the old handle is not located in the same gon Index as the new gon
                               the case when compress become regular */
                            if(swapMemToUseOldHandleGonIndex[j]==0xFFFE && swapMemIndexInUseForAdd[j]==GT_TRUE)
                            {
                                if (swapFirstIndex==0)
                                {
                                    if (shadowPtr->swapMemoryAddr==0)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected shadowPtr->swapMemoryAddr=0\n");
                                    }
                                    swapOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->swapMemoryAddr);
                                    bvLineIndex=0;
                                }
                                else
                                {
                                    if (swapSecondIndex==0)
                                    {
                                        if (shadowPtr->secondSwapMemoryAddr==0)
                                        {
                                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected shadowPtr->secondSwapMemoryAddr=0\n");
                                        }
                                        swapOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->secondSwapMemoryAddr);
                                        bvLineIndex=0;
                                    }
                                    else
                                    {
                                       CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "updateSwapArea failed");
                                    }
                                }
                            }
                            else
                            {
                                if (swapFirstIndex == PRV_CPSS_DXCH_LPM_RAM_ALL_GONS_SWAP_INDEX_CNS)
                                {
                                    if(shadowPtr->swapMemoryAddr==0)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected shadowPtr->swapMemoryAddr=0\n");
                                    }
                                    swapOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->swapMemoryAddr);
                                    bvLineIndex=j;
                                }
                                else
                                {
                                    if (j == swapFirstIndex)
                                    {
                                        if (shadowPtr->swapMemoryAddr==0)
                                        {
                                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected shadowPtr->swapMemoryAddr=0\n");
                                        }
                                        swapOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->swapMemoryAddr);
                                        bvLineIndex=j;
                                    }
                                    else
                                    {
                                        if (shadowPtr->secondSwapMemoryAddr==0)
                                        {
                                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected shadowPtr->secondSwapMemoryAddr=0\n");
                                        }
                                        swapOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->secondSwapMemoryAddr);
                                        bvLineIndex=j;
                                    }
                                }
                            }
                            /* copy separate GON into appropriate swap area */
                            retVal = updateSwapArea(bvLineIndex,bucketPtr->hwGroupOffsetHandle,0xffffffff,swapFirstIndex,
                                                    swapOffset,shareDevsList,shareDevListLen,parentNodeData->swapGonsAdresses);
                            if (retVal != GT_OK)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "updateSwapArea failed");
                            }
                        }
                    }
                    if ((bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E) ||
                        (swapMemInUse == GT_TRUE)||(secondSwapMemInUse == GT_TRUE)||(swapMemInUseForAddGlobalFlag==GT_TRUE))
                    {
                        cpssOsMemCpy(oldHwAddrHandle,bucketPtr->hwGroupOffsetHandle,sizeof(oldHwAddrHandle));
                         /* ok since the group of nodes bucket pointed on is updated in the HW, we can now
                            update the shadow for given bucket: group of nodes current bucket belongs will
                            be updated on higher level */
                        if (bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E)
                        {
                            bucketPtr->bucketType = (GT_U8)newBucketType;

                            /* in case of PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E we
                               don't update the pointer type since it represents that it
                               points to a src trie. (and not the actuall type of the
                               bucket it points to)*/
                            if ((GT_U32)*bucketTypePtr != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)
                                *bucketTypePtr = (GT_U8)newBucketType;
                            for (j = 0; j < groupOfNodesNum; j++)
                            {
                                /* build hw data for affected group subnodes */
                                if ((tempHwAddrHandle[j] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[j] ==0))
                                {
                                    continue;
                                }
                                bucketPtr->hwGroupOffsetHandle[j] = tempHwAddrHandle[j];
                            }
                        }
                        /* gon was written in swap area. The old connection is still working. In order to activate new connection,
                        the previous gon must be updated with swap pointer */
                        if (bucketUpdateMode != LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E)
                        {
                            if((isDestTreeRootBucket == GT_TRUE) || (isSrctTreeRootBucket == GT_TRUE))
                            {
                                bucketPtr->hwBucketOffsetHandle = tempHwRootAddrHandle;
                                parentNodeData->isRootNode = GT_TRUE;
                            }
                            parentNodeData->funcCallCounter++; /* funcCallCounter == 1 */

                            retVal = updateHwRangeDataAndGonPtr(parentNodeData);
                            if (retVal != GT_OK)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "bad swap update");
                            }
                        }


                        /* now it's ok to free this bucket's old memory */
                        /* in multicast may be additional type is compareded like leaf_triger */
                        if ((freeOldMem == GT_TRUE) &&
                            (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
                            (bucketPtr->bucketType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) )
                        {
                            if ( (swapMemInUse == GT_TRUE ) && (parentNodeData->isBvCompConvertion == GT_TRUE) )
                            {
                                /*there is special case when under delete operation node transforms from*/
                                /* bit vector to compressed. In this case we must free all bit vector's Gons*/
                                for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                                {
                                    if ((oldHwAddrHandle[j] != DMM_BLOCK_NOT_FOUND) && (oldHwAddrHandle[j] != DMM_MALLOC_FAIL))
                                    {
                                        oldMemPool[j] = DMM_GET_PARTITION(oldHwAddrHandle[j]);

                                        /*  set pending flag for future need */
                                        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(oldHwAddrHandle[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                        retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                                        if (retVal1 != GT_OK)
                                        {
                                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                                        }
                                        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                                        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(oldHwAddrHandle[j]);

                                        prvCpssDmmFree(oldHwAddrHandle[j]);
                                    }
                                    else
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "oldHwAddrHandle[j] should not be NULL ");
                                    }
                                }
                            }
                            else if (bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E)
                            {
                                /* bulk insertion */
                                for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                                {
                                    if ((oldHwAddrHandle[j] != DMM_BLOCK_NOT_FOUND) && (oldHwAddrHandle[j] != DMM_MALLOC_FAIL))
                                    {
                                        oldMemPool[j] = DMM_GET_PARTITION(oldHwAddrHandle[j]);

                                        /*  set pending flag for future need */
                                        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(oldHwAddrHandle[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                        retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                                        if (retVal1 != GT_OK)
                                        {
                                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                                        }
                                        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                                        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(oldHwAddrHandle[j]);

                                        prvCpssDmmFree(oldHwAddrHandle[j]);
                                    }

                                    if ( (bucketPtr->bucketType !=CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E) && (j != 0) )
                                    {
                                        bucketPtr->hwGroupOffsetHandle[j] = 0;
                                    }
                                }
                            }
                            else
                            {
                                /* in case of special case that compress turn to regular
                                   there is only one old - need to be treated differently */
                                for (j = 0; j < groupOfNodesNum; j++)
                                {
                                    if ((swapMemInUseForAddGlobalFlag==GT_TRUE)&&
                                        (swapMemInUseForAdd[j] == GT_TRUE)&&
                                        (swapMemToUseOldHandleGonIndex[j]==0xFFFE))
                                        oneOldWhenComToReg=GT_TRUE;
                                }

                                for (j = 0; j < groupOfNodesNum; j++)
                                {
                                    /* build hw data for affected group subnodes */
                                    if ((tempHwAddrHandle[j] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[j] ==0))
                                    {
                                        continue;
                                    }
                                    if (swapMemInUseForAddGlobalFlag==GT_TRUE)
                                    {
                                        /* in case of special case that compress turn to regular
                                           the old need to be treated differently */
                                        if (swapMemInUseForAdd[j] == GT_TRUE)
                                        {
                                            if(swapMemToUseOldHandleGonIndex[j]==0xFFFE)
                                            {
                                                 /* double check - should always be true */
                                                if (!((swapMemToUseOldHandleAddr[j]==oldHwAddrHandle[0])&&
                                                     (oldHwAddrHandle[0] != DMM_BLOCK_NOT_FOUND) &&
                                                     (oldHwAddrHandle[0] != DMM_MALLOC_FAIL)))
                                                {
                                                      /* illegal - since the old in a move from compress to regular is always first */
                                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "swapMemToUseOldHandleAddr[j]!=oldHwAddrHandle[0])");
                                                }
                                            }
                                            else
                                            {
                                                if (!((swapMemToUseOldHandleAddr[j]==oldHwAddrHandle[swapMemToUseOldHandleGonIndex[j]])&&
                                                      (oldHwAddrHandle[swapMemToUseOldHandleGonIndex[j]] != DMM_BLOCK_NOT_FOUND) &&
                                                      (oldHwAddrHandle[swapMemToUseOldHandleGonIndex[j]] != DMM_MALLOC_FAIL)))
                                                {
                                                      /* illegal - since the old in a move from compress to regular is always first */
                                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "swapMemToUseOldHandleAddr[j]!=oldHwAddrHandle[swapMemToUseOldHandleGonIndex[j]])");
                                                }
                                            }

                                            oldMemPool[j] = DMM_GET_PARTITION(swapMemToUseOldHandleAddr[j]);

                                            if (swapMemToUseOldHandleAddr[j]==0)
                                            {
                                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected swapMemToUseOldHandleAddr[j]=0\n");
                                            }
                                            /*  set pending flag for future need */
                                            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(swapMemToUseOldHandleAddr[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                            retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                                            if (retVal1 != GT_OK)
                                            {
                                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                                            }
                                            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                                            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(swapMemToUseOldHandleAddr[j]);

                                            prvCpssDmmFree(swapMemToUseOldHandleAddr[j]);

                                            /* in case of special case that compress turn to regular
                                            there is no need to continue the loop since there is only one old for compress */
                                            if(swapMemToUseOldHandleGonIndex[j]==0xFFFE)
                                            {
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            if(oneOldWhenComToReg==GT_FALSE)
                                            {
                                                if ((oldHwAddrHandle[j] != DMM_BLOCK_NOT_FOUND) && (oldHwAddrHandle[j] != DMM_MALLOC_FAIL))
                                                {
                                                    oldMemPool[j] = DMM_GET_PARTITION(oldHwAddrHandle[j]);

                                                    /*  set pending flag for future need */
                                                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(oldHwAddrHandle[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                                    retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                                                    if (retVal1 != GT_OK)
                                                    {
                                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                                                    }
                                                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                                                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(oldHwAddrHandle[j]);

                                                    prvCpssDmmFree(oldHwAddrHandle[j]);
                                                }
                                                else
                                                {
                                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "oldHwAddrHandle[j] should not be NULL ");
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if ((oldHwAddrHandle[j] != DMM_BLOCK_NOT_FOUND) && (oldHwAddrHandle[j] != DMM_MALLOC_FAIL))
                                        {
                                            oldMemPool[j] = DMM_GET_PARTITION(oldHwAddrHandle[j]);

                                            /*  set pending flag for future need */
                                            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(oldHwAddrHandle[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                            retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                                            if (retVal1 != GT_OK)
                                            {
                                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                                            }
                                            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                                            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(oldHwAddrHandle[j]);

                                            prvCpssDmmFree(oldHwAddrHandle[j]);
                                        }
                                        else
                                        {
                                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "oldHwAddrHandle[j] should not be NULL ");
                                        }
                                    }
                                }
                            }
                            freeOldMem = GT_FALSE;
                        }
                    }

                    if ((swapMemInUse == GT_FALSE)&&(secondSwapMemInUse == GT_FALSE)&&(swapMemInUseForAddGlobalFlag==GT_FALSE))
                        break;


                    for (j = 0; j < groupOfNodesNum; j++)
                    {
                        /* build hw data for affected group subnodes */
                        if ((tempHwAddrHandle[j] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[j] ==0))
                        {
                            continue;
                        }

                        /* if this is a case of swap use in add then the alloction need to be done only if the flag is up
                           since it can be that (tempHwAddrHandle[j] != DMM_BLOCK_NOT_FOUND) && (tempHwAddrHandle[j] !=0)
                           but no new allocation should be done
                           in case of use of swap in delete then if
                           (tempHwAddrHandle[j] != DMM_BLOCK_NOT_FOUND) && (tempHwAddrHandle[j] !=0)
                           it mean new allocation should be done */

                        if((swapMemInUseForAdd[j] == GT_TRUE)||
                           ((swapMemInUse == GT_TRUE) &&
                            ((swapFirstIndex == j)||(swapFirstIndex == PRV_CPSS_DXCH_LPM_RAM_ALL_GONS_SWAP_INDEX_CNS)))||
                           ((secondSwapMemInUse == GT_TRUE) &&
                            ((swapSecondIndex == j)||(swapSecondIndex == PRV_CPSS_DXCH_LPM_RAM_ALL_GONS_SWAP_INDEX_CNS))))
                        {
                            if(freeBlockCanBeFoundRegToCompSwapUse==GT_TRUE)
                            {
                                /* we know were should we allocate the new GON */
                                if ((oldHwAddrHandle[freeBlockGonIndex] != DMM_BLOCK_NOT_FOUND) && (oldHwAddrHandle[freeBlockGonIndex] != DMM_MALLOC_FAIL))
                                {
                                    oldMemPoolSelected = oldMemPool[freeBlockGonIndex];
                                    if (oldMemPoolSelected == 0)
                                    {
                                        /*  should never happen since we just freed this memory */
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "oldMemPoolSelected should not be NULL,should never happen since we just freed this memory ");
                                    }
                                }
                                else
                                {
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "oldHwAddrHandle[freeBlockGonIndex] should not be NULL ");
                                }
                            }
                            else
                            {
                                if (swapUsedForShrink==GT_TRUE)
                                {
                                    /* we know were should we allocate the new GON - in the shrinked blocked */
                                    oldMemPoolSelected = shadowPtr->lpmRamStructsMemPoolPtr[bankIndexForShrink];
                                }
                                else
                                {
                                    /* if we reached here the swap memory is in use , and there is
                                    a need to move and rewrite the bucket back where it was */
                                    if (oldMemPool[j] == 0)
                                    {
                                        /*  should never happen since we just freed this memory */
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "oldMemPool[j] should not be NULL,  should never happen since we just freed this memory ");
                                    }
                                    oldMemPoolSelected = oldMemPool[j];
                                }
                            }
                            tempHwAddrHandle[j] = prvCpssDmmAllocate(oldMemPoolSelected,
                                                              DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * subnodesSizesPtr[j],
                                                              DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);
                            if ((tempHwAddrHandle[j] == DMM_BLOCK_NOT_FOUND)||(tempHwAddrHandle[j] == DMM_MALLOC_FAIL))
                            {
                                /* should never happen since we just freed this mem*/
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " can't allocate memory - defrag");
                            }
                            else
                            {
                                /* build gon with new allocated address*/
                                retVal = getMirrorGroupOfNodesDataAndUpdateRangesAddress(newBucketType,
                                                                                         startSubNodeAddress[j],
                                                                                         endSubNodeAddress[j],
                                                                                         pRange,
                                                                                         shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                                                         PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[j]),
                                                                                         &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_1)[j]));
                                if (retVal != GT_OK)
                                {
                                    return retVal;
                                }


                                /* keep new tempHwAddrHandle[j] -- in case of a fail it should be freed */
                                needToFreeAllocationInCaseOfFail[j] = GT_TRUE;
                                tempHwAddrHandleToBeFreed[j] = tempHwAddrHandle[j];

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
                                if(swapMemInUseForAdd[j] == GT_TRUE)
                                {
                                    /* if we have a special case that compress turn
                                       to regular the the new address should be kept
                                       in first same as the old handle was */
                                    if (swapMemToUseOldHandleGonIndex[j]==0xFFFE)
                                    {
                                        parentNodeData->swapGonsAdresses[0] = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[j]);
                                    }
                                    else
                                    {
                                        parentNodeData->swapGonsAdresses[j] = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[j]);
                                    }
                                }
                                else
                                {
                                    parentNodeData->swapGonsAdresses[j] = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[j]);
                                }
                            }
                        }
                    }

                    /*we have new allocation and must rewrite pointer to current GON*/
                    updatePtrToNewAllocatedAfterSwap = GT_TRUE;

                     /* indicate we're not using the swap memory anymore*/
                    swapMemInUse = GT_FALSE;
                    secondSwapMemInUse = GT_FALSE;
                    swapFirstIndex = PRV_CPSS_DXCH_LPM_RAM_ILLEGAL_SWAP_INDEX_CNS;
                    swapSecondIndex = PRV_CPSS_DXCH_LPM_RAM_ILLEGAL_SWAP_INDEX_CNS;
                    cpssOsMemSet(swapMemInUseForAdd,0,sizeof(swapMemInUseForAdd));
                    cpssOsMemSet(swapMemIndexInUseForAdd,0,sizeof(swapMemInUseForAdd));
                    cpssOsMemSet(swapMemToUseOldHandleAddr,0,sizeof(swapMemInUseForAdd));
                    cpssOsMemSet(swapMemToUseOldHandleGonIndex,0,sizeof(swapMemInUseForAdd));
                    swapMemInUseForAddGlobalFlag = GT_FALSE;

                } while (GT_TRUE);
                /* check if it is root bucket we should update root node itself*/
            }
            else
            {
                /* there was no resize in octet group of nodes, but may be group of nodes content
                   was changed and update is needed */
                retVal = lpmFalconUpdateRangeInHw(bucketPtr,pRange,
                                                  shadowPtr);
                if (retVal != GT_OK)
                {
                    return retVal;
                }
            }
        }

        if ( (isDestTreeRootBucket == GT_TRUE) || (isSrctTreeRootBucket == GT_TRUE) )
        {
            /* main loop is over and now is handling root nodes*/
            if ((isDestTreeRootBucket == GT_TRUE))
            {
                if ((convertFromCompressToRegular==GT_TRUE) || (convertFromRegularToCompress == GT_TRUE) )
                {
                    /* the update was done before */
                    updatePtrToNewAllocatedAfterSwap = GT_FALSE;
                }
            }
            if(((convertFromRegularToCompress == GT_TRUE)||(convertFromCompressToRegular==GT_TRUE)) &&
               (updatePtrToNewAllocatedAfterSwap == GT_TRUE) )
            {

                if (convertFromRegularToCompress==GT_TRUE)
                {
                    bucketPtr->hwGroupOffsetHandle[0] = tempHwAddrHandle[0];
                }
                else
                {
                    cpssOsMemCpy(bucketPtr->hwGroupOffsetHandle,tempHwAddrHandle,sizeof(tempHwAddrHandle));
                }

                parentNodeData->funcCallCounter++;/* funcCallCounter==2 */
                parentNodeData->lpmEngineMemPtrPtr = lpmEngineMemPtrPtr-1;

                if (convertFromCompressToRegular==GT_TRUE)
                {
                    parentNodeData->addOperation=GT_TRUE;
                    /* the value of the needed memory must be in this case one of the swap areas*/
                    parentNodeData->neededMemoryBlocksInfoPtr= &shadowPtr->neededMemoryBlocksInfo[shadowPtr->neededMemoryCurIdx];
                    parentStartRangeBitVectorLocation=0;
                    /* we want the parent we need to look at shadowPtr->neededMemoryCurIdx */
                    if(shadowPtr->neededMemoryBlocksInfo[shadowPtr->neededMemoryCurIdx].neededMemoryBlocksSwapUsedForAdd[parentStartRangeBitVectorLocation]==GT_TRUE)
                    {
                        /* in the case the not only the gons changes from 1 to 6
                           but also the bucket is changed from 1 line compress to 6 lines regular,
                           this happen in the parent level*/
                        parentNodeData->swapAreaAlsoUsedForAddOnParentLevel=GT_TRUE;
                    }
                    else
                    {
                        parentNodeData->swapAreaAlsoUsedForAddOnParentLevel=GT_FALSE;
                    }
                }
                else
                {
                    /* we are interested only in special case that compress become regular
                       all the rest will not be treated in the code */
                    parentNodeData->addOperation=GT_FALSE;
                    parentNodeData->swapAreaAlsoUsedForAddOnParentLevel=GT_FALSE;
                }
                retVal = updateHwRangeDataAndGonPtr(parentNodeData);
                if (retVal != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "bad after swap update");
                }
                updatePtrToNewAllocatedAfterSwap = GT_FALSE;
                parentNodeData->funcCallCounter = 0;
            }
            else
                /* destination root bucket or src root bucket is going to be updated*/
                if ((rootBucketState == LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E) ||
                    (rootBucketState == LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_SRC_RESIZE_E) ||
                    (rootBucketState == LPM_ROOT_BUCKET_UPDATE_HW_E) )
                {
                     GT_UINTPTR oldHwGroupOffsetHandle[6];
                     cpssOsMemCpy(oldHwGroupOffsetHandle,bucketPtr->hwGroupOffsetHandle,sizeof(oldHwGroupOffsetHandle));
                        /* it means new ranges were added into root node*/
                    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_1),0,
                                 sizeof(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_1)));

                    if ((rootBucketState == LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E) ||
                        (rootBucketState == LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_SRC_RESIZE_E))
                    {
                        /* root bucket add : from compressed to regular*/
                        /* new alloc for regular in octet 0 memory and free old memory*/
                        if(convertFromCompressToRegular == GT_TRUE)
                        {
                            /* memory is preallocated in add */
                            if (shadowPtr->neededMemoryListLen > 0)
                            {
                                PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmOctet0MemPtr;
                                lpmOctet0MemPtr = lpmEngineMemPtrPtr-1;

                                i = shadowPtr->neededMemoryCurIdx;
                                if (i >= shadowPtr->neededMemoryListLen)
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* should not happen */

                                /* memory is preallocated or swap is in use */
                                if (shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksSwapUsedForAdd[0] == GT_TRUE)
                                {
                                    for (j=0;j<groupOfNodesNum;j++)
                                    {
                                        /* i+1 is the place were the gons are located */
                                        bucketPtr->hwGroupOffsetHandle[j]= tempHwAddrHandle[j];
                                    }
                                    bucketPtr->bucketType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
                                    parentNodeData->funcCallCounter = 2;/* funcCallCounter==2 */
                                    parentNodeData->lpmEngineMemPtrPtr = lpmOctet0MemPtr;
                                    parentNodeData->isRootNode = isSrctTreeRootBucket;

                                    /* compress become regular - add operation */
                                    parentNodeData->addOperation=GT_TRUE;
                                    parentNodeData->swapAreaAlsoUsedForAddOnParentLevel=GT_TRUE;
                                    parentNodeData->neededMemoryBlocksInfoPtr = &shadowPtr->neededMemoryBlocksInfo[i];

                                    retVal = updateHwRangeDataAndGonPtr(parentNodeData);
                                    if (retVal != GT_OK)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "bad after swap update");
                                    }

                                    /* now need to free old GON memory */
                                    for (j=0;j<groupOfNodesNum;j++)
                                    {
                                        if ((oldHwGroupOffsetHandle[j]!=0)&&(oldHwGroupOffsetHandle[j]!=DMM_BLOCK_NOT_FOUND))
                                        {
                                            retVal = prvCpssDxChLpmRamMemFreeListMng(oldHwGroupOffsetHandle[j],
                                                                                     PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E,
                                                                                     freeMemListPtr,shadowPtr);
                                            if (retVal != GT_OK)
                                            {
                                                return retVal;
                                            }
                                        }
                                    }
                                    /* the bucket's HW update was done. no need to further update it */
                                    bucketPtr->bucketHwUpdateStat = PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E;
                                    return GT_OK;
                                }
                                else
                                {
                                    tempHwRootAddrHandle = shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0];
                                    swapMemInUseForAdd[0]= shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksSwapUsedForAdd[0];
                                    swapMemIndexInUseForAdd[0] = shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksSwapIndexUsedForAdd[0]; /* swap memory used: swap1/swap2/swap3*/
                                    swapMemToUseOldHandleAddr[0]=shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksOldHandleAddr[0];/* to old memory we should move to the swap */
                                    swapMemToUseOldHandleGonIndex[0]=shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksOldHandleGonIndex[0];/* to old memory gon index we should move to the swap */

                                    shadowPtr->neededMemoryCurIdx++;
                                }
                            }
                        }
                        else if (shadowPtr->neededMemoryListLen > 0)
                            {
                                i = shadowPtr->neededMemoryCurIdx;
                                if (i >= shadowPtr->neededMemoryListLen)
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* should not happen */

                                tempHwRootAddrHandle = shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0];
                                swapMemInUseForAdd[0]= shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksSwapUsedForAdd[0];
                                swapMemIndexInUseForAdd[0] = shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksSwapIndexUsedForAdd[0]; /* swap memory used: swap1/swap2/swap3*/
                                swapMemToUseOldHandleAddr[0]=shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksOldHandleAddr[0];/* to old memory we should move to the swap */
                                swapMemToUseOldHandleGonIndex[0]=shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksOldHandleGonIndex[0];/* to old memory gon index we should move to the swap */

                                shadowPtr->neededMemoryCurIdx++;
                            }
                            else if (convertFromRegularToCompress == GT_TRUE)
                            {
                                /* delete : from regular to compressed*/
                                /* new alloc for compressed in octet 0 memory and free old memory*/
                                PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmOctet0MemPtr;
                                lpmOctet0MemPtr = lpmEngineMemPtrPtr-1;
                                tempNextMemInfoPtr = lpmOctet0MemPtr[0];
                                tmpStructsMemPool = lpmOctet0MemPtr[0]->structsMemPool;
                                /* allocate memory */
                                do
                                {
                                    tempHwRootAddrHandle = prvCpssDmmAllocate(tmpStructsMemPool,
                                                                             DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * 1,
                                                                             DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);
                                    if (tempNextMemInfoPtr->nextMemInfoPtr == NULL)
                                    {
                                        break;
                                    }
                                    tempNextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                                    tmpStructsMemPool = tempNextMemInfoPtr->structsMemPool;

                                } while (tempHwRootAddrHandle==DMM_BLOCK_NOT_FOUND);

                                if (tempHwRootAddrHandle == DMM_BLOCK_NOT_FOUND)
                                {
                                    /* there is no free place */
                                    if ( (swapMemInUse == GT_TRUE) && (secondSwapMemInUse == GT_TRUE) )
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM, LOG_ERROR_NO_MSG);
                                    }
                                    else
                                    {
                                        bucketPtr->hwGroupOffsetHandle[0]= tempHwAddrHandle[0];
                                        bucketPtr->bucketType = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;
                                        parentNodeData->funcCallCounter = 2; /* funcCallCounter==2 */
                                        parentNodeData->lpmEngineMemPtrPtr = lpmOctet0MemPtr;
                                        parentNodeData->isRootNode = isSrctTreeRootBucket;

                                        /* regular become compress - delete operation */
                                        parentNodeData->addOperation=GT_FALSE;
                                        parentNodeData->swapAreaAlsoUsedForAddOnParentLevel=GT_FALSE;

                                        retVal = updateHwRangeDataAndGonPtr(parentNodeData);
                                        if (retVal != GT_OK)
                                        {
                                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "bad after swap update");
                                        }
                                        tempHwRootAddrHandle = bucketPtr->hwBucketOffsetHandle;
                                        bucketPtr->bucketType = oldBucBucketType;
                                        updatePtrToNewAllocatedAfterSwap = GT_FALSE;
                                        parentNodeData->funcCallCounter = 0;
                                        srcTreeRootNodeIsHandledbySwapFunc = GT_TRUE;
                                    }
                                }
                                else
                                {
                                    if (tempHwRootAddrHandle==0)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected tempHwRootAddrHandle=0\n");
                                    }
                                    /*  set pending flag for future need */
                                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwRootAddrHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                    retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                                    if (retVal1 != GT_OK)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                                    }
                                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
                                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                                        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwRootAddrHandle);
                                }
                            }
                            else
                            {
                                cpssOsPrintf(" Bad data state: contradiction between Bucket data and delete logic\n");
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Bad data state: contradiction between Bucket data and delete logic"); /* should not happen */
                            }
                        }
                        else
                        {
                            tempHwRootAddrHandle = bucketPtr->hwBucketOffsetHandle;
                        }

                    /* We are doing update of root node due to some of its GONs were changed.
                       Update GON's offset information */
                    for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                    {
                        if ((tempHwAddrHandle[i] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[i] ==0))
                        {
                          continue;
                        }
                        bucketPtr->hwGroupOffsetHandle[i] = tempHwAddrHandle[i];
                    }

                    if (srcTreeRootNodeIsHandledbySwapFunc == GT_FALSE)
                    {
                        bucketPtr->bucketType = newBucketType;
                        if(newBucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                        {
                            retVal = lpmFalconFillRegularData(bucketPtr,
                                                              shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                              &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_1)[0].regularNodesArray[0]));
                        }
                        else
                        {
                            retVal = lpmFalconFillCompressedData(bucketPtr,
                                                                 shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                                 &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_1)[0].compressedNodesArray[0]),
                                                                 NULL);
                        }
                        bucketPtr->bucketType = oldBucBucketType;
                        if (isSrctTreeRootBucket == GT_TRUE)
                        {
                            if (tempHwRootAddrHandle==0)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected tempHwRootAddrHandle=0\n");
                            }
                            bucketPtr->nodeMemAddr = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwRootAddrHandle);
                        }
                    }

                    if (retVal != GT_OK)
                    {
                        return retVal;
                    }
                    if (srcTreeRootNodeIsHandledbySwapFunc == GT_FALSE)
                    {
                        for (i = 0; i < shareDevListLen; i++)
                        {
                            if (tempHwRootAddrHandle==0)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected tempHwRootAddrHandle=0\n");
                            }
                            /* write root node */
                            retVal = prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[i],
                                                                     PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwRootAddrHandle),
                                                                     &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_1)[0]),
                                                                     &groupOfNodesLines);
                            if (retVal != GT_OK)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                            }
                        }
                    }
                    cpssOsMemCpy(bucketPtr->hwGroupOffsetHandle,oldHwGroupOffsetHandle,sizeof(oldHwGroupOffsetHandle));
                    /* bucket was resized */
                    if (((rootBucketState == LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E) ||
                         (rootBucketState == LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_SRC_RESIZE_E)) &&
                        (bucketPtr->hwBucketOffsetHandle != 0))
                    {
                        /* swap memory is a pre-allocated memory that should never be freed */
                        if ((bucketPtr->hwBucketOffsetHandle!=shadowPtr->swapMemoryAddr)&&
                            (bucketPtr->hwBucketOffsetHandle!=shadowPtr->secondSwapMemoryAddr)&&
                            (bucketPtr->hwBucketOffsetHandle!=shadowPtr->thirdSwapMemoryAddr))
                        {
                            if (srcTreeRootNodeIsHandledbySwapFunc == GT_FALSE)
                            {
                                /* Free the old group of nodes */
                                retVal = prvCpssDxChLpmRamMemFreeListMng(bucketPtr->hwBucketOffsetHandle,
                                                                         PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E,
                                                                         freeMemListPtr,shadowPtr);
                                if (retVal != GT_OK)
                                {
                                    return retVal;
                                }
                            }
                        }
                    }
                    bucketPtr->hwBucketOffsetHandle = tempHwRootAddrHandle;
                }
        }
        /* the bucket's HW update was done. no need to further update it */
        bucketPtr->bucketHwUpdateStat = PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E;
    }
    else if (bucketUpdateMode == LPM_BUCKET_UPDATE_MEM_ONLY_E)
    {
        /* it is bulk insertion */
        if ((isDestTreeRootBucket == GT_TRUE) && (convertFromCompressToRegular == GT_TRUE))
        {
            if ( (rootBucketState == LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E) &&
                 (bucketPtr->hwBucketOffsetHandle != 0) )
            {

                if (shadowPtr->neededMemoryListLen > 0)
                {
                    i = shadowPtr->neededMemoryCurIdx;
                    if (i >= shadowPtr->neededMemoryListLen)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* should not happen */
                    /* Free the old group of nodes */
                    retVal = prvCpssDxChLpmRamMemFreeListMng(bucketPtr->hwBucketOffsetHandle,
                                                             PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E,
                                                             freeMemListPtr,shadowPtr);
                    if (retVal != GT_OK)
                    {
                        return retVal;
                    }
                    bucketPtr->hwBucketOffsetHandle = shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0];
                    shadowPtr->neededMemoryCurIdx++;
                }
            }
        }
    }

    if ((bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E) ||
        (bucketUpdateMode == LPM_BUCKET_UPDATE_MEM_ONLY_E))
    {
        /* in cases we deal with memory allocation, check if we need to free the
           old bucket */
        /* in multicast we have new leaf type leaf_triger */
        freeOldMem = ((freeOldMem) &&
                      (/*resizeBucket*/resizeGroup == GT_TRUE) &&
                      (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
                      (bucketPtr->bucketType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ); /* not a new bucket */

        if (freeOldMem)
        {
         /* if we are in delete and regular turned to compressed all 6 group of nodes must be freed*/
            for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
            {
                if (convertFromRegularToCompress == GT_TRUE)
                {
                    if (bucketPtr->hwGroupOffsetHandle[i] == 0)
                    {
                        /* must not happend !!!!*/
                        cpssOsPrintf(" FAIL!!! regular node with no group offsets!!!!\n");
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }
                }
                else
                {
                    if ((tempHwAddrHandle[i] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[i] ==0) ||
                        (bucketPtr->hwGroupOffsetHandle[i] == 0))
                    {
                        continue;
                    }
                }
                /* swap memory is a pre-allocated memory that should never be freed */
                if ((bucketPtr->hwGroupOffsetHandle[i]!=shadowPtr->swapMemoryAddr)&&
                    (bucketPtr->hwGroupOffsetHandle[i]!=shadowPtr->secondSwapMemoryAddr)&&
                    (bucketPtr->hwGroupOffsetHandle[i]!=shadowPtr->thirdSwapMemoryAddr))
                {
                    /* Free the old group of nodes */
                    retVal = prvCpssDxChLpmRamMemFreeListMng(bucketPtr->hwGroupOffsetHandle[i],
                                                             PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E,
                                                             freeMemListPtr,shadowPtr);
                    if (retVal != GT_OK)
                    {
                        return retVal;
                    }
                }
                if (bucketPtr->bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                {
                    break;
                }
            }
        }

        for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
        {
            if (convertFromRegularToCompress == GT_TRUE)
            {
                bucketPtr->hwGroupOffsetHandle[i] = 0;
            }

            if ((tempHwAddrHandle[i] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[i] == 0))
            {
              continue;
            }
            bucketPtr->hwGroupOffsetHandle[i] = tempHwAddrHandle[i];
        }

        if (updatePtrToNewAllocatedAfterSwap == GT_TRUE)
        {
            parentNodeData->funcCallCounter++; /* funcCallCounter==2 */
            parentNodeData->lpmEngineMemPtrPtr = lpmEngineMemPtrPtr-1;

            bucketPtr->bucketType = (GT_U8)newBucketType;

            /* in case of PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E we
               don't update the pointer type since it represents that it
               points to a src trie. (and not the actuall type of the
               bucket it points to)*/
            if ((GT_U32)*bucketTypePtr != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)
                *bucketTypePtr = (GT_U8)newBucketType;
            for (j = 0; j < groupOfNodesNum; j++)
            {
                /* build hw data for affected group subnodes */
                if ((tempHwAddrHandle[j] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[j] ==0))
                {
                    continue;
                }
                bucketPtr->hwGroupOffsetHandle[j] = tempHwAddrHandle[j];
            }

            if (convertFromCompressToRegular==GT_TRUE)
            {
                parentNodeData->addOperation=GT_TRUE;
                parentStartRangeBitVectorLocation = parentNodeData->rangePtrArray[(parentNodeData->level-1)]->startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
                /* we want the parent we need to look at shadowPtr->neededMemoryCurIdx */
                if(shadowPtr->neededMemoryBlocksInfo[shadowPtr->neededMemoryCurIdx].neededMemoryBlocksSwapUsedForAdd[parentStartRangeBitVectorLocation]==GT_TRUE)
                {
                    /* in the case the not only the gons changes from 1 to 6
                       but also the bucket is changed from 1 line compress to 6 lines regular,
                       this happen in the parent level*/
                    parentNodeData->swapAreaAlsoUsedForAddOnParentLevel=GT_TRUE;
                    /* the value of the needed memory must be in this case one of the swap areas*/
                    parentNodeData->neededMemoryBlocksInfoPtr= &shadowPtr->neededMemoryBlocksInfo[shadowPtr->neededMemoryCurIdx];
                }
                else
                {
                    parentNodeData->swapAreaAlsoUsedForAddOnParentLevel=GT_FALSE;
                    /* memory was preallocated, and located in index 0 (compress has one line) */
                    parentNodeData->neededMemoryBlocksInfoPtr= &shadowPtr->neededMemoryBlocksInfo[shadowPtr->neededMemoryCurIdx];
                }
            }
            else
            {
                /* we are interested only in special case that compress become regular
                   all the rest will not be treated in the code */
                parentNodeData->addOperation=GT_FALSE;
                parentNodeData->swapAreaAlsoUsedForAddOnParentLevel=GT_FALSE;
            }
            retVal = updateHwRangeDataAndGonPtr(parentNodeData);
            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "bad after swap update");
            }
            updatePtrToNewAllocatedAfterSwap = GT_FALSE;
            parentNodeData->funcCallCounter = 0;
        }
    }

    /* in LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E mode , this shadow update part
       already been done */
    if ((bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E) ||
        (bucketUpdateMode == LPM_BUCKET_UPDATE_MEM_ONLY_E) ||
        (bucketUpdateMode == LPM_BUCKET_UPDATE_SHADOW_ONLY_E)||
        (bucketUpdateMode == LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E))
    {
        /* update the bucket's info info */
        bucketPtr->bucketType = newBucketType;

        /* update the range lower Level bucket type */
        /* in case of PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E we don't update
           the lower Level bucket type (pointer type) since it represents that
           it points to a src trie. (and not the actuall type of the bucket it
           points to)*/
        if ((GT_U32)*bucketTypePtr != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)
            *bucketTypePtr = newBucketType;
        if (bucketUpdateMode == LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E)
        {
            cpssOsMemSet(bucketPtr->hwGroupOffsetHandle, 0, sizeof(bucketPtr->hwGroupOffsetHandle));
        }
    }

    return GT_OK;
}

/**
* @internal checkGroupOfNodesAllocationForOctet function
* @endinternal
*
* @brief Check if allocation was done for given octet
*
* @param[in] octetIndex          - octet number
* @param[in] shadowPtr           - lpm shadow
* @param[in] ucMcType            - describe allocation type
*
* @param[out] isAllocatedPtr   - GT_TRUE: allocation is done
*                                GT_FALSE: otherwise
*
* @retval GT_OK - on success
* @retval GT_FAIL - otherwise
*/
GT_STATUS checkGroupOfNodesAllocationForOctet
(
    IN  GT_U32                               octetIndex,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    IN  PRV_CPSS_DXCH_LPM_ALLOC_TYPE_ENT     ucMcType,
    OUT GT_BOOL                              *isAllocatedPtr
)
{

    GT_U32 i,j;
    *isAllocatedPtr = GT_FALSE;
    for (i = 0; i <shadowPtr->neededMemoryListLen; i++ )
    {
        if (shadowPtr->neededMemoryBlocksInfo[i].octetId == octetIndex)
        {
            if (shadowPtr->neededMemoryBlocksInfo[i].ipAllocType == ucMcType)
            {
                for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                {
                   if( shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j] != DMM_BLOCK_NOT_FOUND)
                   {
                       *isAllocatedPtr = GT_TRUE;
                       return GT_OK;
                   }
                }
            }
        }
    }
    return GT_OK;
}

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
)
{

    GT_U8  startAddr;           /* The startAddr and endAddr of the     */
    GT_U8  endAddr;             /* given prefix in the current level.   */
    GT_U32 rangePrefixLength;   /* The prefix length represented by the */
                                /* current range's mask field.          */
    GT_U32 newPrefixLength;     /* The new prefix length that the       */
                                /* current range represents.            */
    GT_U32 numOfNewRanges = 0;  /* Number of new created ranges as a    */
                                /* result of insertion.                 */
    GT_U32 numOfNewHwRanges = 0;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC newRange;/* the new created range from the insert */
    GT_STATUS retVal= GT_OK;           /* functions returned values.           */

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC **pRange;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *parentRangeToUpdateDueToLowerLevelResizePtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *parentRangeToUpdateDueToSecondLowerLevelResizePtr = NULL;
    GT_U32  parentRangeLevel = 99; /* initializing with and invalid level*/
    GT_U32  parentSecondRangeLevel = 99; /* initializing with and invalid level*/

    GT_BOOL indicateSiblingUpdate = GT_FALSE;
    GT_U32  level = 0;
    GT_BOOL goDown;
    GT_BOOL isDestTreeRootBucket = GT_FALSE;
    GT_BOOL isSrctTreeRootBucket = GT_FALSE;

    LPM_BUCKET_UPDATE_MODE_ENT bucketUpdateMode;

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  **currBucketPtr = PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArry);
    GT_U8                                    *currAddrByte = addrCurBytePtr;
    GT_U32                                   currPrefix = prefix;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC       **currLpmEngineMemPtr = lpmEngineMemPtrPtr;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT      **currBucketPtrType = PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArray);
    LPM_ROOT_BUCKET_UPDATE_ENT               *rootBucketUpdateStatePtr = PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(rootBucketUpdateState);
    LPM_RAM_AFFECTED_BV_LINES_STC            *bvLinePtr = &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pAffectedBvLineArray)[0]);
    GT_BOOL                                  *resizeGroupPtr = PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(resizeGroup);
    GT_BOOL                                  *newRangeIsAddedPtr = PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(newRangeIsAdded);
    GT_BOOL                                  *overriteDonePtr = PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(overriteIsDone);
    GT_BOOL                                  *doHwUpdatePtr = PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(doHwUpdate);
    GT_BOOL                                  *bitVectorCompConvertionPtr = PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(bitVectorCompConvertion);
    GT_BOOL                                  isAllocated = GT_FALSE;
    GT_U32                                   i = 0;
    GT_U32                                   j = 0;
    GT_U32                                   k = 0;
    #if 0
    GT_UINTPTR                               rollBackHwBucketOffsetHandle = 0;
    LPM_BUCKET_UPDATE_MODE_ENT               rollBackBucketUpdateMode = LPM_BUCKET_UPDATE_NONE_E;
    GT_BOOL                                  rollBackResizeBucket = GT_FALSE;
    #endif
    GT_BOOL                                  lastLevelBucketEmbLeaves = GT_FALSE;
    GT_U32 subnodesSizes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_BOOL subnodesIndexes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U8 l = 0;
    GT_U32 loopCounter=0;

    cpssOsMemSet(&newRange,0,sizeof(newRange));

    /* zero the arrays */
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(rootBucketUpdateState),LPM_ROOT_BUCKET_UPDATE_NONE_E,sizeof(GT_BOOL)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(resizeGroup),GT_FALSE,sizeof(GT_BOOL)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(doHwUpdate),GT_FALSE,sizeof(GT_BOOL)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArry),0,
                 sizeof(PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC*)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(newRangeIsAdded),GT_FALSE,sizeof(GT_BOOL)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(overriteIsDone),GT_FALSE,sizeof(GT_BOOL)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(bitVectorCompConvertion),GT_FALSE,sizeof(GT_BOOL)*MAX_LPM_LEVELS_CNS);

    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pRangeArr),0xFF, /*0xFFFFFFFF indicates untouched level*/
                 sizeof(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pRangeArr)));
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pAffectedBvLineArray),0,sizeof(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pAffectedBvLineArray)));


    /* set the initial values.*/
    PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArry)[0] = bucketPtr;
    PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArray)[0] = bucketTypePtr;
    pRange = PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pRangeArr);

    /* first do the lpm tree shadow update part */
    while (currBucketPtr >= PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArry))
    {
        parentUpdateParams->bucketPtrArray[level] = *currBucketPtr;

        /* we start with the assumption we'll go up the bucket trie */
        goDown = GT_FALSE;
        lastLevelBucketEmbLeaves = GT_FALSE;

        /* if the range pointer reached NULL means that we finished with this level */
        if (*pRange != NULL)
        {
            calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,&endAddr);
            cpssOsMemSet(subnodesIndexes,0,sizeof(subnodesIndexes));
            if ((*currBucketPtr)->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                for (l = startAddr / NUMBER_OF_RANGES_IN_SUBNODE; l <= endAddr / NUMBER_OF_RANGES_IN_SUBNODE; l++)
                {
                    bvLinePtr->bvLinesArray[l] = GT_TRUE;
                }
            }
            else
            {
                bvLinePtr->bvLinesArray[0] = GT_TRUE;
            }
            /* check if we need to continue this level or this a brand new level
               (0xFFFFFFFF) */
            if (*pRange == (PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *)(~((GT_UINTPTR)0)))
            {
                *pRange = find1stOverlap(*currBucketPtr,startAddr,NULL);

                parentUpdateParams->rangePtrArray[level] = *pRange;

                /* If belongs to this level, insert it into trie.    */
                if (currPrefix <= PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
                {
                    if (!(((*pRange)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                        ((*pRange)->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
                        ((GT_U32)((*pRange)->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)))
                    {
                        /* it is not the end level. So nextHop overwrite should be done on the next level */
                         *doHwUpdatePtr = GT_TRUE;
                         *overriteDonePtr = GT_TRUE;
                    }
                    retVal = insert2Trie(*currBucketPtr, startAddr,
                                         currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,nextPtr);
                    if (retVal != GT_OK)
                    {
                        /* failed on last level*/
                        for (i=1; i <= level; i++)
                        {
                            if (*(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArray)[i]) == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
                            {
                                *(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArray)[i]) = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;
                            }
                        }
                        if (shadowPtr->neededMemoryListLen > 0)
                        {
                            for (i = shadowPtr->neededMemoryCurIdx; i < shadowPtr->neededMemoryListLen; i++)
                            {
                                if (shadowPtr->neededMemoryBlocksInfo[i].regularNode == GT_TRUE)
                                {
                                    for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                                    {
                                        prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);
                                    }
                                }
                                else
                                    prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]);
                            }
                        }
                        /* create mask for current range */
                        (*pRange)->mask = (GT_U8)PREFIX_2_MASK_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                        return retVal;
                    }

                }
            }

            do
            {
                rangePrefixLength = mask2PrefixLength((*pRange)->mask,
                                                      PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                if (currPrefix >= rangePrefixLength)
                {
                    if (((*pRange)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                        ((*pRange)->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
                        ((GT_U32)((*pRange)->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                    {
                        /* Insert the given addr. to the next level.    */
                        if (currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
                        {
                            /* Create a new bucket with the dominant next_pointer*/
                            /* value of the upper level as the default route.    */
                            newRange.lowerLpmPtr.nextBucket =
                                createNewSip6Bucket((*pRange)->lowerLpmPtr.nextHopEntry,
                                                    PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,
                                                    (*pRange)->pointerType);
                            if (newRange.lowerLpmPtr.nextBucket == NULL)
                            {
                                for (i=1; i <= level; i++)
                                {
                                    if (*(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArray)[i]) == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
                                    {
                                        *(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArray)[i]) = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
                                    }
                                }
                                if (shadowPtr->neededMemoryListLen > 0)
                                {
                                    for (i = shadowPtr->neededMemoryCurIdx; i < shadowPtr->neededMemoryListLen; i++)
                                    {
                                        if (shadowPtr->neededMemoryBlocksInfo[i].regularNode == GT_TRUE)
                                        {
                                            for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                                            {
                                                prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);
                                            }
                                        }
                                        else
                                            prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]);
                                    }
                                }
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                            }

                            /* the new range prefix len is as of what was the prefixlen*/
                            newPrefixLength = rangePrefixLength;

                            /* we set the new range pointer type to next hop
                               since we still don't know what kind of bucket it will
                               be (as in createNewBucket) */
                            newRange.pointerType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;

                            /* Insert the given entry to the next level.     */
                            currBucketPtr[1] = newRange.lowerLpmPtr.nextBucket;

                            /* indicate we need to proceed down the bucket trie*/
                            goDown = GT_TRUE;
                        }
                        else
                        {
                            /* this is the end level. record in the new range
                               the given next ptr */
                            newRange.pointerType = (GT_U8)nextPtrType;
                            if ((nextPtrType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                                (nextPtrType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) )
                            {
                                newRange.lowerLpmPtr.nextHopEntry = nextPtr;
                            }
                            else
                            {
                                /* nextPtrType ==
                                   PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E */
                                newRange.lowerLpmPtr.nextBucket = nextPtr;
                            }

                            /* the new range prefix len is as of what's left of
                               the prefix length */
                            newPrefixLength = currPrefix;
                        }
                        /* Create new ranges if needed, as a result of inserting    */
                        /* the new range on a pre-existing range.                   */
                        retVal = sip6SplitRange(pRange,startAddr,endAddr,
                                                newPrefixLength,&newRange,
                                                PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,updateOldPtr,
                                                PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArray)[level],&numOfNewRanges,&numOfNewHwRanges,
                                                &(currBucketPtrType[1]));
                        if (retVal != GT_OK)
                        {
                            if (currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
                            {
                                /* delete new bucket that was created just before fail */
                                prvCpssDxChLpmRamMngBucketDelete(newRange.lowerLpmPtr.nextBucket,
                                                                 PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, NULL);
                            }
                            else
                            {
                                /* create mask for current range */
                                (*pRange)->mask = (GT_U8)PREFIX_2_MASK_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                            }
                            for (i=1; i <= level; i++)
                            {
                                if (*(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArray)[i]) == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
                                {
                                    *(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArray)[i]) = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
                                }
                            }

                            if (shadowPtr->neededMemoryListLen > 0)
                            {
                                for (i = shadowPtr->neededMemoryCurIdx; i < shadowPtr->neededMemoryListLen; i++)
                                {
                                    if (shadowPtr->neededMemoryBlocksInfo[i].regularNode == GT_TRUE)
                                    {
                                        for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                                        {
                                            prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);
                                        }
                                    }
                                    else
                                        prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]);
                                }
                            }
                            return retVal;
                        }

                        /* a change happend to the structure of the bucket
                           clear the cash*/
                        (*currBucketPtr)->rangeCash = NULL;
                        /* check if root bucket also should be resized */
                        if ((currBucketPtr == PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArry))     &&
                            (rootBucketFlag !=PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E) )
                        {
                            /* check if resized*/
                            retVal = checkGroupOfNodesAllocationForOctet(level ,shadowPtr,ucMcType,&isAllocated);
                            if (retVal != GT_OK)
                            {
                                return retVal;
                            }

                            if ((isAllocated == GT_TRUE) && (ucMcType != PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E))
                            {
                                /* for root bucket */
                                *rootBucketUpdateStatePtr = LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E;
                            }
                            else if (isAllocated == GT_TRUE)
                            {
                                /* mc source root */
                                *rootBucketUpdateStatePtr = LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_SRC_RESIZE_E;
                            }
                            else
                            {
                                *rootBucketUpdateStatePtr = LPM_ROOT_BUCKET_UPDATE_HW_E;
                            }
                        }

                        if (/*numOfNewRanges*/numOfNewHwRanges != 0)
                        {
                            *newRangeIsAddedPtr = GT_TRUE;
                        }
                        if ((((*currBucketPtr)->numOfRanges == 1) && (protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E) &&
                               ((*currBucketPtr)->bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E))||
                             ( ((*currBucketPtr)->numOfRanges <= MAX_NUMBER_OF_COMPRESSED_RANGES_CNS) &&
                               (((*currBucketPtr)->numOfRanges + numOfNewRanges) > MAX_NUMBER_OF_COMPRESSED_RANGES_CNS) &&
                                  ((*currBucketPtr)->bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)) )
                        {
                                if ((*currBucketPtr)->numOfRanges == 1)
                                {
                                   /* new bucket - compressed */
                                   cpssOsMemSet(bvLinePtr->bvLinesArray,0,sizeof(bvLinePtr->bvLinesArray));
                                   bvLinePtr->bvLinesArray[0] = GT_TRUE;
                                }
                                else
                                {
                                    /* new turned regular*/
                                   for(k=0;k<PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS;k++)
                                        bvLinePtr->bvLinesArray[k]=GT_TRUE;

                                   /* compress become regular - need to update parent parameters for future use*/
                                   *bitVectorCompConvertionPtr = GT_TRUE;
                                }
                                /* new bucket on this level or bucket turn to regular from compressed.
                                    This is clear that  groop of nodes new/turned bucket pointed on
                                    must be resized.
                                    Also the group of nodes size this bucket belongs to is changed.
                                    The previous level pointed on this group of nodes must be aware
                                    that size of its group are changed and groop needs a resize*/
                                if (((*currBucketPtr)->numOfRanges == 1) &&
                                (currPrefix <= PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS))
                                {
                                /* new bucket, last level --> compressed with embedded leaves*/
                                 *resizeGroupPtr = GT_FALSE;
                                }
                                else
                                {
                                 *resizeGroupPtr = GT_TRUE;
                                }
                                 if (!((currBucketPtr == PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArry))     &&
                                     (rootBucketFlag !=PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E) ))
                                 {
                                    /* we are not in root */
                                    /* must to check if previous level really need resize */
                                    retVal = checkGroupOfNodesAllocationForOctet(level,shadowPtr,ucMcType,&isAllocated);
                                    if (retVal != GT_OK)
                                    {
                                        return retVal;
                                    }
                                    if (isAllocated == GT_TRUE)
                                    {
                                        *(resizeGroupPtr - 1) = GT_TRUE;
                                        /* if previous level is root*/
                                        if ( (level-1 == 0) &&
                                             (rootBucketFlag !=PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E) )
                                        {
                                            /* for root bucket */
                                            if ( *(rootBucketUpdateStatePtr - 1) == LPM_ROOT_BUCKET_UPDATE_NONE_E)
                                            {
                                                *(rootBucketUpdateStatePtr - 1) = LPM_ROOT_BUCKET_UPDATE_HW_E;
                                            }
                                        }
                                    }
                                }
                        }
                        else
                        {
                            if ( (((*currBucketPtr)->numOfRanges+numOfNewRanges) <= MAX_NUMBER_OF_COMPRESSED_RANGES_CNS) &&
                                ((*currBucketPtr)->bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E) )
                            {
                                /*compressed */
                                 if (!((currBucketPtr == PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArry))&&
                                     (rootBucketFlag !=PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E)&&
                                     (ucMcType != PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E) ))
                                 {
                                    /* we are not in dst root */
                                     cpssOsMemSet(bvLinePtr,0,sizeof(bvLinePtr->bvLinesArray));
                                     bvLinePtr->bvLinesArray[0] = GT_TRUE;
                                 }
                            }
                            /* in order to know whether group of nodes this bucket pointed on will be resized
                               check allocation info for given level */
                            retVal = checkGroupOfNodesAllocationForOctet(level+1,shadowPtr,ucMcType,&isAllocated);
                            if (retVal != GT_OK)
                            {
                                return retVal;
                            }
                            if (isAllocated == GT_TRUE)
                            {
                                *resizeGroupPtr = GT_TRUE;
                            }
                            else
                            {
                                if (numOfNewHwRanges == 0)
                                {
                                    /* owerrite is done: group of nodes must be updated */
                                    *overriteDonePtr = GT_TRUE;
                                }
                            }
                        }
                        *doHwUpdatePtr = GT_TRUE;
                        /* Add to the amount of ranges */
                        (*currBucketPtr)->numOfRanges = (GT_U16)((*currBucketPtr)->numOfRanges + numOfNewRanges);

                        /* and update the pointing range it needs to be
                           rewritten (if indeed this is the level after
                           that recorded pointing range) */
                        if ((parentRangeToUpdateDueToLowerLevelResizePtr != NULL) &&
                            ((parentRangeLevel +1) == level))
                        {
                            parentRangeToUpdateDueToLowerLevelResizePtr->updateRangeInHw =
                                GT_TRUE;
                            /* indicate to the upper level that it needs
                               a HW update*/
                            *(doHwUpdatePtr - 1) = GT_TRUE;
                        }
                        if ((parentRangeToUpdateDueToSecondLowerLevelResizePtr != NULL) &&
                            ((parentSecondRangeLevel +2) == level))
                        {
                            if ( *(resizeGroupPtr - 1) == GT_TRUE)
                            {
                                parentRangeToUpdateDueToSecondLowerLevelResizePtr->updateRangeInHw =
                                    GT_TRUE;
                                /* indicate to the upper level that it needs
                                   a HW update*/
                                *(doHwUpdatePtr - 2) = GT_TRUE;
                            }
                        }

                    }
                    else
                    {
                        /* Insert the addr. to the existing next bucket. */
                        currBucketPtr[1] = (*pRange)->lowerLpmPtr.nextBucket;

                        currBucketPtrType[1] = &((*pRange)->pointerType);

                        /* indicate we need to proceed down the bucket trie*/
                        goDown = GT_TRUE;

                        if (currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
                        {
                            /* in the case the prefix ends at lower level it
                               may cause the a resize in that lower level which
                               will cause this pointed bucket to change
                               location. so we record the range so the lower
                               level could indicate to this level that in needs
                               to rewrite only pointer and not the whole
                               bucket.
                               Note! if the prefix ends in this level
                               or it ended in higher level, there could be a
                               resize in lower leves! */
                            parentSecondRangeLevel = parentRangeLevel;
                            parentRangeToUpdateDueToSecondLowerLevelResizePtr =
                                parentRangeToUpdateDueToLowerLevelResizePtr;
                            parentRangeToUpdateDueToLowerLevelResizePtr = *pRange;
                            parentRangeLevel = level;
                        }

                        (*pRange)->mask |=
                            PREFIX_2_MASK_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                    }
                }
                else
                {
                    (*pRange)->mask |=
                        PREFIX_2_MASK_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                }

                *pRange = (*pRange)->next;

            }while (((*pRange) != NULL) && (endAddr >= (*pRange)->startAddr) &&
                    (goDown == GT_FALSE));

            /* check if we finished with the ranges we need to go over */
            if (((*pRange) != NULL) && (endAddr < (*pRange)->startAddr))
                (*pRange) = NULL;
        }

        /* check which direction we go in the tree */
        if (goDown == GT_TRUE)
        {
            /* we're going down. so advance in the pointer arrays */
            currBucketPtr++;
            currBucketPtrType++;
            doHwUpdatePtr++;
            rootBucketUpdateStatePtr++;
            resizeGroupPtr++;
            newRangeIsAddedPtr++;
            overriteDonePtr++;
            pRange++;
            bvLinePtr++;
            bitVectorCompConvertionPtr++;

            /* and advance in the prefix information */
            currAddrByte++;

            loopCounter++;
            if (((ucMcType == PRV_CPSS_DXCH_LPM_ALLOC_UC_TYPE_E) &&
                 (loopCounter < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS)) ||
                ((ucMcType != PRV_CPSS_DXCH_LPM_ALLOC_UC_TYPE_E) &&
                 (loopCounter < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_MC_PROTOCOL_CNS)))
            {
                currLpmEngineMemPtr++;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Shouldn't happen: lpmEngineMemPtrPtr exceeds limit");
            }

            level++;
            currPrefix = BUCKET_PREFIX_AT_LEVEL_MAC(prefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,
                                                  level);
        }
        else
        {
            /* we're going up, meaning we finished with the current level.
               write it to the HW and move back the pointers in the arrays*/

            /* here additional check for last level node must be done : if this node is compressed */
            /* and number of ranges no more than 3*/
            if ( currPrefix <= PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
            {
                /* it is last level.*/
                if ((*currBucketPtr)->numOfRanges <= 3)
                {
                    lastLevelBucketEmbLeaves = lpmFalconCheckIfLastLevelBucketEmbdLeaves(*currBucketPtr);
                    /* so there must be comressed with 2 or 3  embedded leaves leaves */
                    if (lastLevelBucketEmbLeaves == GT_TRUE)
                    {
                        PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *llRange = NULL;
                        /* such node is not pointing on GON */
                        *resizeGroupPtr = GT_FALSE;
                        /* in case of override must signal to previous node */
                        if (*overriteDonePtr == GT_TRUE)
                        {
                            if (level != 0)
                            {
                                *(overriteDonePtr - 1) = GT_TRUE;
                                *(doHwUpdatePtr-1) = GT_TRUE;
                            }
                        }
                        else
                        {
                            *doHwUpdatePtr = GT_FALSE;
                        }
                        /* clear all update hw flags in this bucket*/
                        llRange = (*currBucketPtr)->rangeList;
                        while (llRange != NULL)
                        {
                            llRange->updateRangeInHw = GT_FALSE;
                            llRange = llRange->next;
                        }
                        if ((*currBucketPtr)->hwGroupOffsetHandle[0] != 0)
                        {
                            cpssOsPrintf(" BAD STATE !!!!, hwGroupOffsetHandle[0] != 0 on last level\n ");
                        }
                    }
                }
            }

            /* if the bucket need resizing always update the HW */
            if ((*rootBucketUpdateStatePtr == LPM_ROOT_BUCKET_UPDATE_HW_E) ||
                (*rootBucketUpdateStatePtr == LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E) )
            {
                *doHwUpdatePtr = GT_TRUE;
            }
            if ((currBucketPtr == PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArry))     &&
                (rootBucketFlag !=PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E) )
            {
                if (ucMcType != PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E)
                {
                    isDestTreeRootBucket = GT_TRUE;
                    isSrctTreeRootBucket = GT_FALSE;
                }
                else
                {
                    isSrctTreeRootBucket = GT_TRUE;
                    isDestTreeRootBucket = GT_FALSE;
                }

                parentUpdateParams->bucketPtr = *currBucketPtr;
                parentUpdateParams->pRange = NULL;

            }
            else
            {
                /* both isSrctTreeRootBucket and isDestTreeRootBucket are false */
                 /* level can be equal to zero only in case of updates nexthops in the tree */
                if (level != 0)
                {
                    parentUpdateParams->bucketPtr =  PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArry)[level - 1];
                    parentUpdateParams->pRange = parentUpdateParams->rangePtrArray[level-1];
                }
                else
                {
                    if (rootBucketFlag !=PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E)
                    {
                        cpssOsPrintf("there is a problem here, level = 0,rootBucketFlag =%d\n",rootBucketFlag);
                    }
                }
            }

            if (parentUpdateParams->nextLevelDoneBySwapFunc == GT_TRUE)
            {
                *doHwUpdatePtr=GT_FALSE;
                *resizeGroupPtr = GT_FALSE;
                *newRangeIsAddedPtr = GT_FALSE;
                *overriteDonePtr = GT_FALSE;

                parentUpdateParams->nextLevelDoneBySwapFunc = GT_FALSE;
            }

            /* during hot sync we shouldn't touch the Hw or the allocation
               it will be done in the end  */
            if (isDestTreeRootBucket &&
                (rootBucketFlag == PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_RAM_UPDATE_E))
            {
                bucketUpdateMode = LPM_BUCKET_UPDATE_ROOT_BUCKET_SHADOW_ONLY_E;
                indicateSiblingUpdate =
                    (insertMode == PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E)?
                    GT_TRUE : GT_FALSE;
            }
            else switch (insertMode)
            {
                 case PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E:
                     if (lastLevelBucketEmbLeaves == GT_TRUE)
                     {
                         bucketUpdateMode = LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E;
                     }
                     else
                     {
                         bucketUpdateMode = LPM_BUCKET_UPDATE_SHADOW_ONLY_E;
                     }
                     break;
                case PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E:
                    if (*doHwUpdatePtr == GT_TRUE)
                    {
                        bucketUpdateMode = LPM_BUCKET_UPDATE_MEM_ONLY_E;
                    }
                    else
                    {
                        bucketUpdateMode = LPM_BUCKET_UPDATE_NONE_E;
                        indicateSiblingUpdate = GT_TRUE;
                    }
                    if (lastLevelBucketEmbLeaves == GT_TRUE)
                    {
                        bucketUpdateMode = LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E;
                    }
                    break;
                case PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E:
                    bucketUpdateMode = (*doHwUpdatePtr == GT_TRUE) ?
                                       LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E:
                                       LPM_BUCKET_UPDATE_NONE_E;
                    if (lastLevelBucketEmbLeaves == GT_TRUE)
                    {
                        bucketUpdateMode = LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E;
                    }
                    break;
                 default:
                     for (i=1; i <= level; i++)
                     {
                         if (*(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArray)[i]) == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
                         {
                             *(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArray)[i]) = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
                         }
                     }

                     if (shadowPtr->neededMemoryListLen > 0)
                     {
                         for (i = shadowPtr->neededMemoryCurIdx; i < shadowPtr->neededMemoryListLen; i++)
                         {
                             if (shadowPtr->neededMemoryBlocksInfo[i].regularNode == GT_TRUE)
                             {
                                 for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                                 {
                                     prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);
                                 }
                             }
                             else
                                 prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]);
                         }
                     }
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            /* save rollback information */
            #if 0
            rollBackHwBucketOffsetHandle = (*currBucketPtr)->hwBucketOffsetHandle;
            rollBackBucketUpdateMode = bucketUpdateMode;
            rollBackResizeBucket = *resizeBucketPtr;
           #endif
            if (bucketUpdateMode == LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E)
            {
                /* there wouldn't be hw update */
                cpssOsMemSet(subnodesIndexes, GT_FALSE, sizeof(subnodesIndexes));
            }
            else
            {
                cpssOsMemCpy(subnodesIndexes, bvLinePtr->bvLinesArray, sizeof(subnodesIndexes));
            }

            parentUpdateParams->isRootNode = isDestTreeRootBucket;
            parentUpdateParams->shadowPtr = shadowPtr;
            parentUpdateParams->level = level;
            parentUpdateParams->isBvCompConvertion= PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(bitVectorCompConvertion)[level];
            parentUpdateParams->addOperation = GT_TRUE;/* will be overwritten later by defrag operation */
            if (currPrefix == 0)
            {
                parentUpdateParams->nhUpdateOnly = GT_TRUE;
            }
            else
            {
                parentUpdateParams->nhUpdateOnly = GT_FALSE;
            }

           /* The shadow structures to the RAM.    */
           cpssOsMemSet(subnodesSizes,0,sizeof(subnodesSizes));
           if ((bucketUpdateMode==LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E)&&
               ((isDestTreeRootBucket == GT_TRUE)||(isSrctTreeRootBucket == GT_TRUE))&&
               (insertMode==PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E)&&
               (*overriteDonePtr==GT_TRUE))
           {
               bucketUpdateMode=LPM_BUCKET_UPDATE_SHADOW_ONLY_E;
           }
           retVal = updateMirrorGroupOfNodes(*currBucketPtr, bucketUpdateMode, 
                                              isDestTreeRootBucket,isSrctTreeRootBucket,
                                              *rootBucketUpdateStatePtr,*resizeGroupPtr,*newRangeIsAddedPtr,
                                              *overriteDonePtr,GT_FALSE,
                                              indicateSiblingUpdate,
                                              currLpmEngineMemPtr,
                                              *currBucketPtrType,shadowPtr,parentUpdateParams,
                                              &subnodesIndexes[0],&subnodesSizes[0]);

            if (retVal != GT_OK)
            {
                /* TBD Roolback is not implemented yet for SIP6
                   When we implement it, the following code should be
                   investigated and counters should be updtaed properly */
#if 0
 /* need to check this code */
                (*currBucketPtr)->bucketHwUpdateStat = PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E;
                for (i=1; i <= level; i++)
                {
                    if (*(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArray)[i]) == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
                    {
                        *(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArray)[i]) = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
                    }
                }

                if((rollBackBucketUpdateMode != LPM_BUCKET_UPDATE_SHADOW_ONLY_E) &&
                   ((rollBackResizeBucket == GT_TRUE) || (rollBackHwBucketOffsetHandle == 0)))
                {
                    /* incase we have a fail in the updateMirrorBucket and we are using the
                        swap area inorder to do resize, this means that in the PP memory
                        estimation stage we didn't do any memory allocation  - so nothing to free */
                    if (((*currBucketPtr)->hwBucketOffsetHandle == shadowPtr->swapMemoryAddr) ||
                        /* hw update was not successful */
                        (rollBackHwBucketOffsetHandle == (*currBucketPtr)->hwBucketOffsetHandle))
                    {
                        /* remove memory allocation that was done for this bucket on */
                        /* PP memory estimation stage */

                        if (shadowPtr->neededMemoryListLen > 0)
                        {
                            if (shadowPtr->neededMemoryCurIdx > 0)
                            {
                                for (i = shadowPtr->neededMemoryCurIdx-1; i < shadowPtr->neededMemoryListLen; i++)
                                {
                                    if (shadowPtr->neededMemoryBlocksInfo[i].regularNode == GT_TRUE)
                                    {
                                        for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                                        {
                                            /*  set pending flag for future need */
                                            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                            blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                                            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                                            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                                PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);

                                            if((shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]!=shadowPtr->swapMemoryAddr)&&
                                                (shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]!=shadowPtr->secondSwapMemoryAddr)&&
                                                (shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]!=shadowPtr->thirdSwapMemoryAddr))
                                            {
                                                prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);
                                            }
                                            else
                                            {
                                                /* nothing to free */
                                            }
                                        }
                                     }
                                     else
                                     {
                                       /*  set pending flag for future need */
                                        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                        blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                                        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                                        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]);

                                        if((shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]!=shadowPtr->swapMemoryAddr)&&
                                            (shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]!=shadowPtr->secondSwapMemoryAddr)&&
                                            (shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]!=shadowPtr->thirdSwapMemoryAddr))
                                        {
                                            prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]);
                                        }
                                        else
                                        {
                                            /* nothing to free */
                                        }
                                    }
                                }
                            }
                        }
                   }
                }
                else
                {
                    if(rollBackResizeBucket == GT_FALSE)
                    {
                        /* it was not successfull update of existing bucket */
                        /* free all preallocated memory memory */
                        if (shadowPtr->neededMemoryListLen > 0)
                        {
                                for (i = shadowPtr->neededMemoryCurIdx; i < shadowPtr->neededMemoryListLen; i++)
                                {
                                    if (shadowPtr->neededMemoryBlocksInfo[i].regularNode == GT_TRUE)
                                    {
                                        for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                                        {
                                            /*  set pending flag for future need */
                                            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                            blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                                            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                                            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                                PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);

                                            if((shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]!=shadowPtr->swapMemoryAddr)&&
                                                (shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]!=shadowPtr->secondSwapMemoryAddr)&&
                                                (shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]!=shadowPtr->thirdSwapMemoryAddr))
                                            {
                                                prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);
                                            }
                                            else
                                            {
                                                /* nothing to free */
                                            }
                                        }
                                     }
                                     else
                                     {
                                       /*  set pending flag for future need */
                                            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                                            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                                PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]);

                                            if((shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]!=shadowPtr->swapMemoryAddr)&&
                                                (shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]!=shadowPtr->secondSwapMemoryAddr)&&
                                                (shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]!=shadowPtr->thirdSwapMemoryAddr))
                                            {
                                                prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]);
                                            }
                                            else
                                            {
                                                /* nothing to free */
                                            }
                                     }
                                }
                            }
                        }
                    }
#endif
                return retVal;
            }

            /* reset isBvToCompConvertion for next level use */
            parentUpdateParams->isBvCompConvertion = GT_FALSE;
            parentUpdateParams->isRootNode = GT_FALSE;

            /* update partition block */
            if ((*currBucketPtr)->hwBucketOffsetHandle!=0)
            {
                SET_DMM_BLOCK_PROTOCOL((*currBucketPtr)->hwBucketOffsetHandle, protocol);
                SET_DMM_BUCKET_SW_ADDRESS((*currBucketPtr)->hwBucketOffsetHandle, (*currBucketPtr));
            }

            for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
            {
                if ((*currBucketPtr)->hwGroupOffsetHandle[j]!=0)
                {
                    SET_DMM_BLOCK_PROTOCOL((*currBucketPtr)->hwGroupOffsetHandle[j], protocol);
                    SET_DMM_BUCKET_SW_ADDRESS((*currBucketPtr)->hwGroupOffsetHandle[j], (*currBucketPtr));
                }
            }

            /* move back in the arrays*/
            currBucketPtr--;
            currBucketPtrType--;
            doHwUpdatePtr--;
            rootBucketUpdateStatePtr--;
            resizeGroupPtr--;
            newRangeIsAddedPtr--;
            overriteDonePtr--;
            bvLinePtr--;
            bitVectorCompConvertionPtr--;
            /* before we go up , indicate this level is finished, and ready
               for a fresh level if needed in the future */
            *pRange = (PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *)(~((GT_UINTPTR)0));
            pRange--;

            /* and move back in the prefix information */
            currAddrByte--;
            if (loopCounter>0)
            {
                loopCounter--;
                currLpmEngineMemPtr--;
            }
            level--;
            currPrefix = BUCKET_PREFIX_AT_LEVEL_MAC(prefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,level);
        }
    }

    return GT_OK;
}


/**
* @internal lpmFalconGetSplitMethod function
* @endinternal
*
* @brief Return the split method of a given range in the lpm tree.
*
* @param[in]  bucketType            - type of bucket
* @param[in]  numOfBucketRangesPtr  - Number of bucket ranges before the split.
* @param[in]  rangePtr              - Pointer to the range to be splitted.
* @param[in]  startAddr             - The low address of the new range.
* @param[in]  endAddr               - The high address of the new range.
* @param[in]  prefixLength          - The length of the dominant prefix of the new range
* @param[in]  levelPrefix           - The current lpm level prefix.
*
* @param[out] numOfBucketRangesPtr   - Number of bucket ranges after the split.
* @param[out] numOfBitVectorRangesPtr -
* @param[out] splitMethodPtr         - The mode according to which the range should be split.
*
* @retval  GT_OK     - on success.
* @retval  GT_fAIL   - otherwise
*/
static GT_STATUS lpmFalconGetSplitMethod
(
    IN      CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT    bucketType,
    INOUT   GT_U32                                 *numOfBucketRangesPtr,
    IN      PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *rangePtr,
    IN      GT_U8                                  startAddr,
    IN      GT_U8                                  endAddr,
    IN      GT_U8                                  levelPrefix,
    OUT     PRV_CPSS_DXCH_LPM_RAM_SPLIT_METHOD_ENT *splitMethodPtr,
    OUT     GT_U32                                 *numOfBitVectorLineRangesPtr
)
{
    GT_U8  endRange, realEndRange;           /* Index by which this range ends.      */
    GT_U8  rangeStatrtAddr;
    GT_U8 groupSubNodeId;
    GT_U8 startSubNodeId;
    GT_U8 endSubNodeId;
    PRV_CPSS_DXCH_LPM_RAM_SPLIT_METHOD_ENT bvSplitMethod;
    GT_U32 oldNumOfRanges = *numOfBucketRangesPtr;
    rangeStatrtAddr = rangePtr->startAddr;
    endRange = (GT_U8)(LAST_RANGE_MAC(rangePtr)? ((1 << levelPrefix) - 1) :
    ((rangePtr->next->startAddr) - 1));
    realEndRange = endRange;

    *splitMethodPtr = (((startAddr > rangeStatrtAddr) ? 1 : 0) +
                       ((endAddr < endRange) ? 2 : 0));

    switch (*splitMethodPtr)
    {
    case (PRV_CPSS_DXCH_LPM_RAM_SPLIT_HIGH_SPLIT_E):
    case (PRV_CPSS_DXCH_LPM_RAM_SPLIT_LOW_SPLIT_E):
        *numOfBucketRangesPtr += 1;
        break;

    case (PRV_CPSS_DXCH_LPM_RAM_SPLIT_MID_SPLIT_E):
        *numOfBucketRangesPtr += 2;
        break;

    case (PRV_CPSS_DXCH_LPM_RAM_SPLIT_OVERWRITE_E):
        *numOfBucketRangesPtr += 0;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    if ((bucketType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E) ||
        (bucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E) ||
        (bucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
        (bucketType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
    {
        if (((oldNumOfRanges < MIN_NUMBER_OF_BIT_VECTOR_RANGES_CNS)&& (*numOfBucketRangesPtr >= MIN_NUMBER_OF_BIT_VECTOR_RANGES_CNS)) ||
            ((startAddr != endAddr)&& (oldNumOfRanges >= MAX_NUMBER_OF_COMPRESSED_RANGES_CNS - 1 )))
        {
            /* node is going to be regular */
            bucketType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
        }
        else
        {
            return GT_OK;
        }
    }

    if (bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
        startSubNodeId = startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
        endSubNodeId = endAddr/NUMBER_OF_RANGES_IN_SUBNODE;
        for (groupSubNodeId = startSubNodeId; groupSubNodeId <= endSubNodeId; groupSubNodeId++ )
        {
            if (startSubNodeAddress[groupSubNodeId] > realEndRange)
            {
                /* we are checking how giving leaf range impact on current range
                   and moving on subnodes that leaf range includes*/
                /* In case when start subnode address bigger then end of current range zone of impact is over*/
                break;
            }
            if (endSubNodeAddress[groupSubNodeId] < rangePtr->startAddr)
            {
                continue;
            }
            if (startAddr <= startSubNodeAddress[groupSubNodeId])
            {
                rangeStatrtAddr =startSubNodeAddress[groupSubNodeId];
            }
            if ( (LAST_RANGE_MAC(rangePtr) == GT_TRUE)||
                 (((rangePtr->next->startAddr) - 1) > (GT_U8)endSubNodeAddress[groupSubNodeId]) )
            {
                endRange = (GT_U8)endSubNodeAddress[groupSubNodeId];
            }
            else
            {
                endRange = (rangePtr->next->startAddr) - 1;
            }
            bvSplitMethod = (((startAddr > rangeStatrtAddr) ? 1 : 0) +
                          ((endAddr < endRange) ? 2 : 0));
            switch (bvSplitMethod)
            {
                case (PRV_CPSS_DXCH_LPM_RAM_SPLIT_HIGH_SPLIT_E):
                case (PRV_CPSS_DXCH_LPM_RAM_SPLIT_LOW_SPLIT_E):
                    numOfBitVectorLineRangesPtr[groupSubNodeId] += 1;
                    break;

                case (PRV_CPSS_DXCH_LPM_RAM_SPLIT_MID_SPLIT_E):
                    numOfBitVectorLineRangesPtr[groupSubNodeId] += 2;
                    break;

                case (PRV_CPSS_DXCH_LPM_RAM_SPLIT_OVERWRITE_E):
                    numOfBitVectorLineRangesPtr[groupSubNodeId] += 0;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }

    }

    return GT_OK;
}


/**
* @internal lpmFalconGetGroupOfNodesItemsForCompressed function
* @endinternal
*
* @brief Gets number group of nodes components for compressed node.
*
* @param[in] rangeListPtr      - Pointer to the bucket ranges .
*
* @param[out] regularNodesNumberPtr     - pointer to regular nodes number
* @param[out] compressedNodesNumberPtr  - pointer to compressed nodes number
* @param[out] leafNodesNumberPtr        - pointer to leaf nodes number
*
* @retval GT_OK               - on success,
* @retval GT_FAIL             - Otherwise.
*/
static GT_STATUS lpmFalconGetGroupOfNodesItemsForCompressed
(
    IN    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC        *rangeListPtr,
    OUT   GT_U32                                        *regularNodesNumberPtr,
    OUT   GT_U32                                        *compressedNodesNumberPtr,
    OUT   GT_U32                                        *leafNodesNumberPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC              *rangePtr = NULL;
    rangePtr = rangeListPtr;
    while (rangePtr != NULL)
    {
        if(rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            regularNodesNumberPtr[0]++;
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
        {
            compressedNodesNumberPtr[0]++;
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )
        {
            leafNodesNumberPtr[0]++;
        }
        rangePtr = rangePtr->next;
    }

    return GT_OK;
}

/**
* @internal lpmFalconGetGroupOfNodesItemsForRegular function
* @endinternal
*
* @brief Gets number group of nodes components for regular node.
*
* @param[in] rangeListPtr      - Pointer to the bucket ranges .
* @param[in] startAddr         - new range start address
* @param[in] endAddr           - new range end address
* @param[out] isStartAddrMatchRegionPtr - if start of new range already points on lpm node
* @param[out] isEndAddrMatchRegionPtr   - if end of new range already points on lpm node
* @param[out] regularNodesNumberPtr     - pointer to regular nodes number
* @param[out] compressedNodesNumberPtr  - pointer to compressed nodes number
* @param[out] leafNodesNumberPtr        - pointer to leaf nodes number
*
* @retval  GT_OK               - on success,
* @retval  GT_FAIL             - Otherwise.
*/
static GT_STATUS lpmFalconGetGroupOfNodesItemsForRegular
(
    IN    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC        *rangeListPtr,
    IN    GT_U32                                        startAddr,
    IN    GT_U32                                        endAddr,
    OUT   GT_BOOL                                       *isStartAddrMatchRegionPtr,
    OUT   GT_BOOL                                       *isEndAddrMatchRegionPtr,
    OUT   GT_U32                                        *regularNodesNumberPtr,
    OUT   GT_U32                                        *compressedNodesNumberPtr,
    OUT   GT_U32                                        *leafNodesNumberPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC              *rangePtr = NULL;
    GT_U32 startNodeIndex, endNodeIndex,endRangeAddr,i;
    rangePtr = rangeListPtr;
    /* In regular node we have 6 different subnodes */
    while (rangePtr != NULL)
    {
        if (rangePtr->startAddr == startAddr)
        {
            *isStartAddrMatchRegionPtr = GT_TRUE;
        }
        startNodeIndex = rangePtr->startAddr / NUMBER_OF_RANGES_IN_SUBNODE;
        if (rangePtr->next == NULL)
        {
            endRangeAddr = 255;

        }
        else
        {
            endRangeAddr = rangePtr->next->startAddr - 1;
        }
        endNodeIndex = endRangeAddr/NUMBER_OF_RANGES_IN_SUBNODE;
        if (endRangeAddr == endAddr)
        {
            *isEndAddrMatchRegionPtr = GT_TRUE;
        }
        /* check subnodes ths range belongs to */
        if (rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            for (i = startNodeIndex; i <= endNodeIndex; i++ )
            {
                regularNodesNumberPtr[i]++;
            }
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
        {
            for (i = startNodeIndex; i <= endNodeIndex; i++ )
            {
                compressedNodesNumberPtr[i]++;
            }
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )
        {
            for (i = startNodeIndex; i <= endNodeIndex; i++ )
            {
                leafNodesNumberPtr[i]++;
            }
        }
        rangePtr = rangePtr->next;
    }

    return GT_OK;
}

/**
* @internal lpmFalconCheckIfCompressedWithEmbeddedLeaves function
* @endinternal
*
* @brief Check if compressed node with embedded leaves.
*
* @param[in] numOfRanges      - number of ranges in current compressed node
* @param[in] numOfLeaves      - number of leaves in current compressed node
* @param[out] isCompressedWithEmbLeavesPtr     - pointer to type of compressed node
*/
GT_VOID lpmFalconCheckIfCompressedWithEmbeddedLeaves
(
    IN    GT_U32 numOfRanges,
    IN    GT_U32 numOfLeaves,
    OUT   GT_BOOL *isCompressedWithEmbLeavesPtr,
    OUT   CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT *compressNodeTypePtr
)
{
    *isCompressedWithEmbLeavesPtr = GT_FALSE;
    if ((numOfRanges <=7) && (numOfLeaves == 1))
    {
        *isCompressedWithEmbLeavesPtr = GT_TRUE;
        *compressNodeTypePtr = CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E;
    }else
        if ((numOfRanges <=5) && (numOfLeaves == 2))
        {
            *isCompressedWithEmbLeavesPtr = GT_TRUE;
            *compressNodeTypePtr = CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E;
        }else
            if ((numOfRanges ==3) && (numOfLeaves == 3))
            {
                *isCompressedWithEmbLeavesPtr = GT_TRUE;
                *compressNodeTypePtr = CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E;
            }else
            {
                *compressNodeTypePtr = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;
            }
}

/**
* @internal lpmFalconGetGroupOfNodesItems function
* @endinternal
*
* @brief Gets number of components assembled group of nodes.
*
* @param[in] rangeListPtr      - Pointer to the bucket ranges .
* @param[in] bucketType        - bucket type
* @param[in] startAddr         - new range start address
* @param[in] endAddr           - new range end address
* @param[out] isStartAddrMatchRegionPtr - if start of new range already points on lpm node
* @param[out] isEndAddrMatchRegionPtr   - if end of new range already points on lpm node
* @param[out] regularNodesNumberPtr     - pointer to regular nodes number
* @param[out] compressedNodesNumberPtr  - pointer to compressed nodes number
* @param[out] leafNodesNumberPtr        - pointer to leaf nodes number
*
* @retval GT_OK               - on success,
* @retval GT_FAIL             - Otherwise.
*/
static GT_STATUS lpmFalconGetGroupOfNodesItems
(
    IN    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC        *rangeListPtr,
    IN    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT           bucketType,
    IN    GT_U32                                        startAddr,
    IN    GT_U32                                        endAddr,
    OUT   GT_BOOL                                       *isStartAddrMatchRegionPtr,
    OUT   GT_BOOL                                       *isEndAddrMatchRegionPtr,
    OUT   GT_U32                                        *regularNodesNumberPtr,
    OUT   GT_U32                                        *compressedNodesNumberPtr,
    OUT   GT_U32                                        *leafNodesNumberPtr
)
{

    switch (bucketType)
    {
    case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
        lpmFalconGetGroupOfNodesItemsForCompressed(rangeListPtr, regularNodesNumberPtr,
                                                   compressedNodesNumberPtr,leafNodesNumberPtr);
        break;
    case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        lpmFalconGetGroupOfNodesItemsForRegular(rangeListPtr,startAddr,endAddr,isStartAddrMatchRegionPtr,
                                                isEndAddrMatchRegionPtr,regularNodesNumberPtr,
                                                compressedNodesNumberPtr,leafNodesNumberPtr);
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal lpmFalconGetGroupOfNodesSizes function
* @endinternal
*
* @brief Calc the ammount of needed Pp memory for given group of nodes.
*
* @param[in] bucketPtr         - Pointer to the given bucket.
* @param[in] bucketType        - given bucket type
* @param[in] curAddrByte       - The current address byte
* @param[in] prefix            - The address prefix.
* @param[in] numberOfNewRanges - number of new ranges after insertion
* @param[in] newTurnedRegular  - GT_TRUE: bucket going to turn into regular under current insertion
*                                GT_FALSE: bucket was regular before current insertion
* @param[in] newLevelPlaned    - GT_TRUE: new level will be opened under current insertion
*                                GT_FALSE: new level will not be opened under current insertion
* @param[out] groupOfNodesIndex - group of node index size calculated for (if 0xf - fro all 6 indexes)
* @param[out] memPoolList       - List of memory pools from which the blocks should be
*                                 allocated from.
* @param[out] memListLenPtr     - Number of memory blocks in neededMemList.
* @param[out] groupOfNodesSizes - group of nodes sizes array
*
* @retval   GT_OK               - on success,
* @retval   GT_FAIL             - Otherwise.
*/
static GT_STATUS lpmFalconGetGroupOfNodesSizes
(
    IN    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC       *bucketPtr,
    IN    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT           bucketType,
    IN    GT_U8                                         curAddrByte,
    IN    GT_U32                                        prefix,
    IN    GT_U32                                        numberOfNewRanges,
    IN    GT_BOOL                                       newTurnedRegular,
    IN    GT_BOOL                                       newLevelPlaned,
    IN    GT_U32                                        *numberOfBvLineRanges,
    OUT   GT_BOOL                                       *groupOfNodesIndexes,
    OUT   GT_U32                                        *groupOfNodesSizes
)
{
    GT_STATUS                                        rc = GT_OK;
    GT_U32                                           regularNodesNumber[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U32                                           compressedNodesNumber[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U32                                           leafNodesNumber[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U32                                           leafNodesSize[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U8                                            startAddr = 0;
    GT_U8                                            endAddr = 0;
    GT_U8                                            startSubNodeIndex = 0;
    GT_U8                                            endSubNodeIndex = 0;
    GT_U8                                            gonIndex = 0;
    GT_U32                                           i = 0;
    GT_U32                                           maxNodesNumber = 0;
    GT_BOOL isCompressedWithEmbLeaves = GT_FALSE;
    GT_BOOL isStartAddrMatchRegion = GT_FALSE;
    GT_BOOL isEndAddrMatchRegion = GT_FALSE;
    GT_BOOL isOverwrite = GT_FALSE;
    GT_U8   counter = 0xff;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT compressedNodeType = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;
    switch (bucketType)
    {
    case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:

        maxNodesNumber = 1;
        rc = lpmFalconGetGroupOfNodesItems(bucketPtr->rangeList,CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E,
                                           startAddr,endAddr,&isStartAddrMatchRegion,&isEndAddrMatchRegion,
                                           &regularNodesNumber[0],&compressedNodesNumber[0],
                                           &leafNodesNumber[0]);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (newLevelPlaned == GT_TRUE)
        {
            /* in this bucket new range is opened. It points on new bucket*/
            compressedNodesNumber[0]++;
            leafNodesNumber[0] = leafNodesNumber[0] + numberOfNewRanges - bucketPtr->numOfRanges - 1;
        }
        else
        {
            leafNodesNumber[0] = leafNodesNumber[0] + numberOfNewRanges - bucketPtr->numOfRanges;
        }
        /* verify do we need special place for leaves or it is embedded leaves*/
        /* in order to do that check if current node after adding new regions is node with embedded leaves*/
        lpmFalconCheckIfCompressedWithEmbeddedLeaves(numberOfNewRanges,leafNodesNumber[0],
                                                     &isCompressedWithEmbLeaves, &compressedNodeType);
        if (isCompressedWithEmbLeaves == GT_TRUE)
        {
            /* all leaves are embedded.*/
            leafNodesNumber[0] = 0;
        }
        break;
    case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        maxNodesNumber = PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS;
        /* Some ranges are added: need to know in which subnodes */
        calcStartEndAddr(curAddrByte,prefix,&startAddr,&endAddr);
        startSubNodeIndex = startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
        endSubNodeIndex = endAddr/NUMBER_OF_RANGES_IN_SUBNODE;

        rc = lpmFalconGetGroupOfNodesItems(bucketPtr->rangeList,bucketType,startAddr,endAddr,
                                           &isStartAddrMatchRegion,&isEndAddrMatchRegion, &regularNodesNumber[0],
                                           &compressedNodesNumber[0],&leafNodesNumber[0]);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* the number of hw bit vector ranges can be differ from shadow  */
        /* it is possible that "changed" bit vector has no additionals ranges */
        if (numberOfBvLineRanges != NULL)
        {
            if (numberOfNewRanges > bucketPtr->numOfRanges)
            {
                counter = 0;
                for (gonIndex = startSubNodeIndex; gonIndex <= endSubNodeIndex; gonIndex++)
                {
                    if (numberOfBvLineRanges[gonIndex] > 0)
                    {
                        counter++;
                    }
                }
            }
        }
        if ((numberOfNewRanges == bucketPtr->numOfRanges) || (counter == 0) )
        {
            isOverwrite = GT_TRUE;
        }
        /* check overwrite */
        if (isOverwrite == GT_TRUE)
        {
            /* overwrite take place */
            /* overwrite for single range and for leaf spectrum lead to same result */
            /* the new level doesn't open in case of leaf stream */
             if (newLevelPlaned == GT_TRUE)
             {
                 /* leaf is going to be changed to compressed */
                 compressedNodesNumber[startSubNodeIndex]++;
                 leafNodesNumber[startSubNodeIndex]--;

             }
        }
        else /* no overwrite*/
        {
            if (numberOfBvLineRanges==NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "\n ERROR unexpected numberOfBvLineRanges==NULL \n");
            }
            if (startAddr == endAddr)
            {
                /* startSubNodeIndex = endSubNodeIndex*/
                /* it is not leaf spectrum*/

                if (newLevelPlaned == GT_TRUE)
                {
                    compressedNodesNumber[startSubNodeIndex]++;
                    leafNodesNumber[startSubNodeIndex] =
                        leafNodesNumber[startSubNodeIndex] + numberOfBvLineRanges[startSubNodeIndex] - 1;
                }
                else
                {
                    leafNodesNumber[startSubNodeIndex] =
                        leafNodesNumber[startSubNodeIndex] + numberOfBvLineRanges[startSubNodeIndex];
                }
            }
            else
            {
                /* leaf spectrum */
                /* max number of new ranges is 2 */
                for (gonIndex = startSubNodeIndex; gonIndex <= endSubNodeIndex; gonIndex++)
                {
                   leafNodesNumber[gonIndex]+= numberOfBvLineRanges[gonIndex];
                }
            }
        }
       break;
    default:
        break;
    }
    if (bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
         groupOfNodesIndexes[0] = GT_TRUE;
    }
    for (i = 0; i < maxNodesNumber; i++)
    {
        if (bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            if (newTurnedRegular == GT_FALSE)
            {
                if ( (i < startSubNodeIndex) || (i > endSubNodeIndex) )
                {
                    continue;
                }
                groupOfNodesIndexes[i] = GT_TRUE;
            }
            else
            {
                groupOfNodesIndexes[i] = GT_TRUE;
            }
        }

        /* calculate place for leaves */
        leafNodesSize[i] = leafNodesNumber[i]/MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;
        if ( (leafNodesNumber[i] % MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS) != 0 )
            leafNodesSize[i]++;
        groupOfNodesSizes[i] =
            regularNodesNumber[i]*PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS +
             compressedNodesNumber[i]*PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS + leafNodesSize[i];
    }
    return GT_OK;
}



/**
* @internal lpmFalconCalcNeededMemory function
* @endinternal
*
* @brief Calc the ammount of needed Pp memory for unicast address insertion.
*
* @param[in]  bucketPtr         - Pointer to the root bucket.
* @param[in]  addrCurBytePtr    - The Byte Array represnting the Address associated
*                                 with the entry.
* @param[in]  prefix            - The address prefix.
* @param[in]  rootBucketFlag    - Indicates the way to deal with a root bucket (if it is).
* @param[in]  lpmEngineMemPtrPtr- points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                 which holds all the memory information needed for where and
*                                 how to allocate search memory for each of the lpm levels
* @param[in]  neededMemList     - List holding the memory needed for inserting the given
*                                 address.
* @param[in]  memPoolList       - List of memory pools from which the blocks should be
*                                 allocated from.
* @param[in]  memListLenPtr     - Nuumber of memory blocks in neededMemList.
* @param[in]  parentUpdateParams- parameters needed for parent update
* @param[out] neededMemList     - List holding the memory needed for inserting the given
*                                 address (in LPM lines)
* @param[out] memPoolList       - List of memory pools from which the blocks should be
*                                 allocated from.
* @param[out] memListLenPtr     - Number of memory blocks in neededMemList.
* @param[out] memListOctetIndexList - List holding the Octet Index of each memory needed for
*                                     inserting the given address
* @param[out] memoryBlockTakenArr - 1:  the block was taken for the ADD operation
*                                   0: the block was NOT taken for the ADD operation
*                                   2,3: the block was taken in 3 octets per block mode
* @retval   GT_OK               - on success,
* @retval   GT_FAIL             - Otherwise.
*/
static GT_STATUS lpmFalconCalcNeededMemory
(
    IN    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC       *bucketPtr,
    IN    GT_U8                                         *addrCurBytePtr,
    IN    GT_U32                                        prefix,
    IN    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT    rootBucketFlag,
    IN    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC            **lpmEngineMemPtrPtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT    insertMode,
    IN    GT_BOOL                                       defragmentationEnable,
    INOUT PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC     *neededMemList[],
    INOUT GT_UINTPTR                                    *memPoolList[],
    INOUT GT_U32                                        *memListLenPtr,
    INOUT GT_U32                                        *memListOctetIndexList[],
    IN    PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC         *parentUpdateParams,
    OUT   GT_U32                                         memoryBlockTakenArr[]
)
{
    GT_U8  startAddr;           /* The startAddr and endAddr of the     */
    GT_U8  endAddr;             /* given prefix in the current level.   */

    GT_U32 rangePrefixLength;   /* The prefix length represented by the */
                                /* current range's mask field.          */


    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRange;         /* The first range that overlaps with   */
                                /* the given address * prefix.               */
    PRV_CPSS_DXCH_LPM_RAM_SPLIT_METHOD_ENT    splitMethod;  /* The method according to which the    */
                                                              /* range should be split.               */
    GT_U32          numOfBucketRanges;  /* Number of ranges in the      */
    GT_U32          numberOfBvLineRanges[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0}; /* number of ranges in bv line */
                                /* current's level's bucket.            */
    GT_U32          currNumOfBucketRanges;  /* Number of ranges in the      */
                                /* current's level's bucket.            */
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *currBucketPtr = bucketPtr;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *copyOfCurrentBucketPtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *prevLevelBucketPtr = NULL;
    GT_U8 *currAddrByte = addrCurBytePtr;
    GT_U32 currPrefix = prefix;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC **currLpmEngineMemPtr = lpmEngineMemPtrPtr;
    GT_U32 octetIndex;
    GT_BOOL planedNewBucket[17];
    GT_U32 grOfNodesSizes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_BOOL grOfNodesIndexes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U32 prevLevelGroupOfNodesSize = 0;
    GT_U32 prevLevelGroupOfNodesIndex = 0;
    GT_U32 i = 0;
    GT_U32 tempPrefixLength = 0;
    GT_U32 prevStartAddr = 0;
    GT_STATUS rc = GT_OK;
    GT_BOOL allocIsNeeded = GT_FALSE;
    GT_BOOL allocIsNeededPerGroup[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS]= {0};
    GT_UINTPTR  prevOctetstructsMemPool = 0;
    GT_U32      prevOctetstructsRamIndex = 0;
    GT_U32 level = 0;

    GT_BOOL     freeBlockCanBeFound;

    GT_U32      numberOfOldHandles=0;
    GT_U32      numberOfNewGons=0;
    GT_UINTPTR  hwGroupOffsetHandleToUse=0;
    GT_U32      hwGroupOffsetHandleToUseGonIndex=0;
    GT_U32      grOfNodesSizesToUse=0;
    GT_U32      counterSizeOftheBiggestGon=0;
    GT_U32      counterIndexOftheBiggestGon=0;
    GT_BOOL     oldHandleForOneOfSixGonsFound=GT_FALSE;

    /* All allocations are calculated for group of nodes */
    /* First group is starting from octet 1*/
    octetIndex=1;
    currLpmEngineMemPtr++;

    cpssOsMemSet(planedNewBucket, 0, sizeof(planedNewBucket));
    while(currPrefix > 0)
    {
        parentUpdateParams->bucketPtrArray[level] = currBucketPtr;
        freeBlockCanBeFound=GT_FALSE;

        copyOfCurrentBucketPtr = currBucketPtr;
        splitMethod = PRV_CPSS_DXCH_LPM_RAM_SPLIT_LAST_E;
        calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,&endAddr);
        if (currBucketPtr == NULL)
        {
            /* This is a new tree level, a new bucket should be created, */
            /* This new bucket will point to new group of nodes*/
            tempPrefixLength = NEXT_BUCKET_PREFIX_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
            if (tempPrefixLength > 0)
            {
                /* The new groups of nodes can be 1 compressed + 1 leaf  */
                /* or 1 compressed +2 leaves*/
                /* new compressed nodes types with 1,2,3 leaves takes 1 line*/
                /* it consists leaves inside of compressed node itself, so new group of nodes */
                /* would be only 1 compressed without leaves */
                (*neededMemList)->neededMemoryBlocksSizes[0] = 1;
            }
            else
            {
                /* This is last level . The new group of nodes is 2 or 3 leaves */
                /* In this case new compressed type with 2 or 3 leaves is used */
                /* This compressed incorporates leaves inside and point on them*/
                /* So there is no new group of nodes */
                (*neededMemList)->neededMemoryBlocksSizes[0] = 0;

            }
            if((*neededMemList)->neededMemoryBlocksSizes[0]>0)
            {
                (*neededMemList)->neededMemoryBlocksSwapUsedForAdd[0] = GT_FALSE;
                (*neededMemList)->neededMemoryBlocksSwapIndexUsedForAdd[0]=0;/* no use */
                (*neededMemList)->neededMemoryBlocksOldHandleAddr[0] = 0;/* no old pointer */
                (*neededMemList)->neededMemoryBlocksOldHandleGonIndex[0] = 0;/* no old pointer */

                (*neededMemList)->octetId = octetIndex;
                **memPoolList = currLpmEngineMemPtr[0]->structsMemPool;
                if (currLpmEngineMemPtr[0]->structsMemPool!=0)
                {
                    memoryBlockTakenArr[currLpmEngineMemPtr[0]->ramIndex]++;
                }
                (*memListLenPtr)++;
                **memListOctetIndexList=octetIndex;
                (*memPoolList)--;
                (*neededMemList)--;
                (*memListOctetIndexList)--;
            }

            /* next level doesn't exists as well. so set it to null        */
            currBucketPtr = NULL;
        }
        else
        {
            pRange = find1stOverlap(currBucketPtr,startAddr,NULL);
            parentUpdateParams->rangePtrArray[level] = pRange;

            numOfBucketRanges = currNumOfBucketRanges = currBucketPtr->numOfRanges;

            /* Go over all levels until the first level to be written to HW.    */
            if (rootBucketFlag == PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_RAM_UPDATE_E)
            {
                /* Calculate the needed memory of the next level bucket.        */
                /* in multicast may be will use leaf_triger type*/
                if ((pRange->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                    (pRange->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
                    ((GT_U32)(pRange->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                {
                    currBucketPtr = NULL;
                }
                else
                {
                    currBucketPtr = pRange->lowerLpmPtr.nextBucket;
                }
            }
            else do
            {
                rangePrefixLength = mask2PrefixLength(pRange->mask,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                if (currPrefix >= rangePrefixLength)
                {
                    /* the inserted prefix covers all or part of this range */
                    /* in multicast may be will use leaf_triger type */
                    if ((pRange->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                        (pRange->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
                        ((GT_U32)(pRange->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                    {
                        /* the range points to an end of tree (next-hop or
                           src bucket */

                        lpmFalconGetSplitMethod(copyOfCurrentBucketPtr->bucketType,
                                                &numOfBucketRanges,
                                                pRange,startAddr,
                                                endAddr,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,
                                                &splitMethod, &numberOfBvLineRanges[0]);

                        if (currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
                        {
                            /* the prefix needs to be inserted deeper thus in
                            this case we Need to create new tree levels, all of
                            them of type "1-line bucket" including 2 or 3 lines.*/

                            /* (will reach here only once per do-while loop) */
                            currBucketPtr = NULL;
                            planedNewBucket[octetIndex] = GT_TRUE;
                        }

                    }
                    else
                    {
                        /* the next pointer points to a bucket  */
                        /* (will reach here only once per do-while loop, in
                            the case we need to go deeper. in other words: if
                            the insertion will cause lower levels to be
                            resized - this is the lower level bucket. if we
                            are on the last level to be resized then it can
                            reach this line several times, but it will have no
                            effect) */
                       /* prevLevelBucketPtr = pRange->lowerLpmPtr.nextBucket;*/
                        currBucketPtr = pRange->lowerLpmPtr.nextBucket;
                    }
                }

                pRange = pRange->next;
            }while ((pRange != NULL) && (endAddr >= pRange->startAddr));

            /* check if a resize is needed, if not return */
            if ( (numOfBucketRanges != currNumOfBucketRanges) ||
                 (splitMethod == PRV_CPSS_DXCH_LPM_RAM_SPLIT_OVERWRITE_E) )
            {
                /* The bucket size is changed, group of nodes this bucket pointed on */
                /* also changed */

                if ((numOfBucketRanges <= MAX_NUMBER_OF_COMPRESSED_RANGES_CNS) &&
                    (copyOfCurrentBucketPtr->bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E))
                {
                    /* check if we have new mc src bucket  */
                    if ( (copyOfCurrentBucketPtr->hwBucketOffsetHandle == 0) &&
                         (currNumOfBucketRanges == 1) && (level == 0) )
                    {
                        /* this is new mc source root which has no hw allocation yet */
                        /* This node at this moment can be compressed : 1 line */
                        (*neededMemList)->neededMemoryBlocksSwapUsedForAdd[0] = GT_FALSE;
                        (*neededMemList)->neededMemoryBlocksSwapIndexUsedForAdd[0]=0;/* no use */
                        (*neededMemList)->neededMemoryBlocksOldHandleAddr[0] = 0;/* no old pointer */
                        (*neededMemList)->neededMemoryBlocksOldHandleGonIndex[0] = 0;/* no old pointer */

                        (*neededMemList)->neededMemoryBlocksSizes[0] = 1;
                        (*memListLenPtr)++;
                        (*neededMemList)->octetId = octetIndex-1;
                        (*neededMemList)--;
                        currLpmEngineMemPtr--;
                        **memPoolList = currLpmEngineMemPtr[0]->structsMemPool;
                        if (currLpmEngineMemPtr[0]->structsMemPool!=0)
                        {
                            memoryBlockTakenArr[currLpmEngineMemPtr[0]->ramIndex]++;
                        }
                        /* return pool ptr on correct octet */
                        currLpmEngineMemPtr++;
                        (*memPoolList)--;
                        **memListOctetIndexList=octetIndex-1;
                        (*memListOctetIndexList)--;
                    }
                    /* there is one case when bucket is regular but number of ranges like in compressed:*/
                    /* root bucket is always regular */
                    /* the bucket is changed. It means that whole group of nodes this bucket belongs to */
                    /* is also changed. So it is needed recalculate memory for this group of nodes */
                    /* calculate group of nodes size in lpm lines this bucket pointed on */
                    cpssOsMemSet(grOfNodesIndexes,0,sizeof(grOfNodesIndexes));
                    cpssOsMemSet(grOfNodesSizes,0,sizeof(grOfNodesSizes));

                    rc = lpmFalconGetGroupOfNodesSizes(copyOfCurrentBucketPtr,
                                                       CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E,
                                                       *currAddrByte,currPrefix,numOfBucketRanges,
                                                       GT_FALSE,planedNewBucket[octetIndex], &numberOfBvLineRanges[0],
                                                       &grOfNodesIndexes[0], &grOfNodesSizes[0]);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    cpssOsMemSet(numberOfBvLineRanges,0,sizeof(numberOfBvLineRanges));
                    /* check if previous group size is differ  */
                    if (copyOfCurrentBucketPtr->hwGroupOffsetHandle[0] != 0)
                    {
                        if ( (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(copyOfCurrentBucketPtr->hwGroupOffsetHandle[0]) != grOfNodesSizes[0]) ||
                            ((PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(copyOfCurrentBucketPtr->hwGroupOffsetHandle[0]) == grOfNodesSizes[0]) &&
                              ( (splitMethod != PRV_CPSS_DXCH_LPM_RAM_SPLIT_LAST_E) && (splitMethod != PRV_CPSS_DXCH_LPM_RAM_SPLIT_OVERWRITE_E))))
                        {
                            allocIsNeeded = GT_TRUE;
                            (*neededMemList)->neededMemoryBlocksSizes[0] = grOfNodesSizes[0];
                            (*neededMemList)->octetId = octetIndex;
                            (*neededMemList)->regularNode = GT_FALSE;

                                /*  Check if incase we use a swap area, the memory can be added.
                                this means that there is free block near the used block and
                                we can use them to create a new bigger block.
                                in case of add-bulk operation we will not support defrag
                                and the use of swap area*/
                            if ((defragmentationEnable==GT_TRUE)&&
                                (copyOfCurrentBucketPtr->hwGroupOffsetHandle[0]!=0)&&
                                (insertMode!=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E)&&
                                ((copyOfCurrentBucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)||
                                (copyOfCurrentBucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)||
                                (copyOfCurrentBucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E)||
                                (copyOfCurrentBucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E)))
                            {
                                rc = prvCpssDxChLpmSip6RamMngFindIfSwapAreaCanBeUsefulToFindFreeBlock(copyOfCurrentBucketPtr->hwGroupOffsetHandle[0],
                                                                                                      grOfNodesSizes[0],
                                                                                                      &freeBlockCanBeFound);
                                if (rc!=GT_OK)
                                {
                                    return rc;
                                }
                            }

                            if (freeBlockCanBeFound==GT_TRUE)
                            {
                                (*neededMemList)->neededMemoryBlocksSwapUsedForAdd[0] = GT_TRUE;
                                (*neededMemList)->neededMemoryBlocksSwapIndexUsedForAdd[0]=0;/* no use yet */
                                SET_DMM_BLOCK_WAS_MOVED(copyOfCurrentBucketPtr->hwGroupOffsetHandle[0],0);/* reset moved bit */
                                (*neededMemList)->neededMemoryBlocksOldHandleAddr[0] = copyOfCurrentBucketPtr->hwGroupOffsetHandle[0];
                                (*neededMemList)->neededMemoryBlocksOldShadowBucket[0] = copyOfCurrentBucketPtr;
                                (*neededMemList)->neededMemoryBlocksOldHandleGonIndex[0] = 0;
                                freeBlockCanBeFound=GT_FALSE;
                            }
                            else
                            {
                                (*neededMemList)->neededMemoryBlocksSwapUsedForAdd[0] = GT_FALSE;
                                (*neededMemList)->neededMemoryBlocksSwapIndexUsedForAdd[0]=0;/* no use yet */
                                if (copyOfCurrentBucketPtr->hwGroupOffsetHandle[0]!=0)
                                {
                                    SET_DMM_BLOCK_WAS_MOVED(copyOfCurrentBucketPtr->hwGroupOffsetHandle[0], 0); /* reset moved bit */
                                }
                                (*neededMemList)->neededMemoryBlocksOldHandleAddr[0] = copyOfCurrentBucketPtr->hwGroupOffsetHandle[0];
                                (*neededMemList)->neededMemoryBlocksOldShadowBucket[0] = copyOfCurrentBucketPtr;
                                (*neededMemList)->neededMemoryBlocksOldHandleGonIndex[0] = 0;
                            }
                        }
                    }
                    else
                    {
                        /* since if (copyOfCurrentBucketPtr->hwGroupOffsetHandle[0] == 0) there is no need to call
                           prvCpssDxChLpmSip6RamMngFindIfSwapAreaCanBeUsefulToFindFreeBlock because there is
                           no old memory allocated */

                        /* it can be when on last level is compressed node with 3 ranges and 3 leaves or
                           compressed with 2 ranges and 2 leaves*/
                        if (grOfNodesSizes[0]>0)
                        {
                            allocIsNeeded = GT_TRUE;
                            (*neededMemList)->neededMemoryBlocksSwapUsedForAdd[0] = GT_FALSE;
                            (*neededMemList)->neededMemoryBlocksSwapIndexUsedForAdd[0]=0;/* no use */
                            (*neededMemList)->neededMemoryBlocksSizes[0] = grOfNodesSizes[0];
                            (*neededMemList)->octetId = octetIndex;
                            (*neededMemList)->regularNode = GT_FALSE;
                        }
                    }
                }
                else
                {
                    /* Regular bucket: 6 groups of nodes*/
                        /*  Check if node was regular before or it is result of new
                           range adding under current operation */
                    if ((currNumOfBucketRanges > MAX_NUMBER_OF_COMPRESSED_RANGES_CNS) ||
                        (copyOfCurrentBucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E) )
                    {
                        /* node was regular before current adding or destination root  */
                        /* need find index of changed group of nodes and calculate new size */
                        cpssOsMemSet(grOfNodesIndexes,0,sizeof(grOfNodesIndexes));
                        cpssOsMemSet(grOfNodesSizes,0,sizeof(grOfNodesSizes));

                        rc = lpmFalconGetGroupOfNodesSizes(copyOfCurrentBucketPtr,
                                                           CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E,
                                                           *currAddrByte,currPrefix,numOfBucketRanges,
                                                           GT_FALSE,planedNewBucket[octetIndex],&numberOfBvLineRanges[0],
                                                           &grOfNodesIndexes[0],&grOfNodesSizes[0]);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                        cpssOsMemSet(numberOfBvLineRanges,0,sizeof(numberOfBvLineRanges));
                        /* check if we really need new allocation */
                        for (i =0; i <PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                        {
                            if (grOfNodesIndexes[i] == GT_TRUE)
                            {
                                if(copyOfCurrentBucketPtr->hwGroupOffsetHandle[i]==0)
                                {
                                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected copyOfCurrentBucketPtr->hwGroupOffsetHandle[i]=0\n");
                                }
                                if ( (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(copyOfCurrentBucketPtr->hwGroupOffsetHandle[i]) != grOfNodesSizes[i]) ||
                                    ((PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(copyOfCurrentBucketPtr->hwGroupOffsetHandle[i]) == grOfNodesSizes[i]) &&
                                     ( (splitMethod != PRV_CPSS_DXCH_LPM_RAM_SPLIT_LAST_E) && (splitMethod != PRV_CPSS_DXCH_LPM_RAM_SPLIT_OVERWRITE_E))))
                                {
                                    allocIsNeeded = GT_TRUE;
                                    allocIsNeededPerGroup[i] = GT_TRUE;

                                    if ((defragmentationEnable==GT_TRUE)&&
                                        (copyOfCurrentBucketPtr->hwGroupOffsetHandle[i]!=0)&&
                                        (insertMode!=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E))
                                    {
                                        rc = prvCpssDxChLpmSip6RamMngFindIfSwapAreaCanBeUsefulToFindFreeBlock(copyOfCurrentBucketPtr->hwGroupOffsetHandle[i],
                                                                                                              grOfNodesSizes[i],
                                                                                                              &freeBlockCanBeFound);
                                        if (rc!=GT_OK)
                                        {
                                            return rc;
                                        }
                                    }

                                    if (freeBlockCanBeFound==GT_TRUE)
                                    {
                                        /* update the field that indicate that swap memory can be useful in case of memory full
                                           this do not mean it will actually be used */
                                        (*neededMemList)->neededMemoryBlocksSwapUsedForAdd[i] = GT_TRUE;
                                        (*neededMemList)->neededMemoryBlocksSwapIndexUsedForAdd[i]=0;/* no use yet*/
                                        SET_DMM_BLOCK_WAS_MOVED(copyOfCurrentBucketPtr->hwGroupOffsetHandle[i],0);/* reset moved bit */
                                        (*neededMemList)->neededMemoryBlocksOldHandleAddr[i] = copyOfCurrentBucketPtr->hwGroupOffsetHandle[i];
                                        (*neededMemList)->neededMemoryBlocksOldShadowBucket[i] = copyOfCurrentBucketPtr;
                                        (*neededMemList)->neededMemoryBlocksOldHandleGonIndex[i] = i;
                                        freeBlockCanBeFound=GT_FALSE;
                                    }
                                    else
                                    {
                                        (*neededMemList)->neededMemoryBlocksSwapUsedForAdd[i] = GT_FALSE;
                                        (*neededMemList)->neededMemoryBlocksSwapIndexUsedForAdd[i]=0;/* no use */
                                        if (copyOfCurrentBucketPtr->hwGroupOffsetHandle[i]!=0)
                                        {
                                            SET_DMM_BLOCK_WAS_MOVED(copyOfCurrentBucketPtr->hwGroupOffsetHandle[i], 0); /* reset moved bit */
                                        }
                                        (*neededMemList)->neededMemoryBlocksOldHandleAddr[i] = copyOfCurrentBucketPtr->hwGroupOffsetHandle[i];
                                        (*neededMemList)->neededMemoryBlocksOldShadowBucket[i] = copyOfCurrentBucketPtr;
                                        (*neededMemList)->neededMemoryBlocksOldHandleGonIndex[i] = i;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        /* As a result of range adding bucket turned into regular */
                        /* there is possibility that node is mc source root*/
                        /* if it is so, new allocation for regular node is done*/

                        if ((level == 0) && (copyOfCurrentBucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E))
                        {
                            /*  Check if incase we use a swap area, the memory can be added.
                                this means that there is free block near the used block and
                                we can use them to create a new bigger block.
                                in case of add-bulk operation we will not support defrag
                                and the use of swap area*/
                            if ((defragmentationEnable==GT_TRUE)&&
                                (copyOfCurrentBucketPtr->hwBucketOffsetHandle!=0)&&
                                (insertMode!=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E))
                            {
                                rc = prvCpssDxChLpmSip6RamMngFindIfSwapAreaCanBeUsefulToFindFreeBlock(copyOfCurrentBucketPtr->hwBucketOffsetHandle,
                                                                                                      PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS,
                                                                                                      &freeBlockCanBeFound);
                                if (rc!=GT_OK)
                                {
                                    return rc;
                                }
                            }

                            if (freeBlockCanBeFound==GT_TRUE)
                            {
                                (*neededMemList)->neededMemoryBlocksSwapUsedForAdd[0] = GT_TRUE;
                                (*neededMemList)->neededMemoryBlocksSwapIndexUsedForAdd[0]=0;/* no use yet */
                                SET_DMM_BLOCK_WAS_MOVED(copyOfCurrentBucketPtr->hwBucketOffsetHandle,0);/* reset moved bit */
                                (*neededMemList)->neededMemoryBlocksOldHandleAddr[0] = copyOfCurrentBucketPtr->hwBucketOffsetHandle;
                                (*neededMemList)->neededMemoryBlocksOldShadowBucket[0] = copyOfCurrentBucketPtr;
                                (*neededMemList)->neededMemoryBlocksOldHandleGonIndex[0] = 0xFFFF;/* 0xFFFF will tell us that we are looking at the
                                                                                                     hwBucketOffsetHandle and not at hwGroupOffsetHandle */
                                freeBlockCanBeFound=GT_FALSE;
                            }
                            else
                            {
                                (*neededMemList)->neededMemoryBlocksSwapUsedForAdd[0] = GT_FALSE;
                                (*neededMemList)->neededMemoryBlocksSwapIndexUsedForAdd[0]=0;/* no use */
                                if (copyOfCurrentBucketPtr->hwBucketOffsetHandle!=0)
                                {
                                    SET_DMM_BLOCK_WAS_MOVED(copyOfCurrentBucketPtr->hwBucketOffsetHandle, 0); /* reset moved bit */
                                }
                                (*neededMemList)->neededMemoryBlocksOldHandleAddr[0] = copyOfCurrentBucketPtr->hwBucketOffsetHandle;
                                (*neededMemList)->neededMemoryBlocksOldShadowBucket[0] = copyOfCurrentBucketPtr;
                                (*neededMemList)->neededMemoryBlocksOldHandleGonIndex[0] = 0xFFFF;/* 0xFFFF will tell us that we are looking at the
                                                                                                     hwBucketOffsetHandle and not at hwGroupOffsetHandle */
                            }

                            /* it is mc source root tree going to be regular*/
                            (*neededMemList)->neededMemoryBlocksSizes[0] = 6;
                            (*memListLenPtr)++;
                            (*neededMemList)->octetId = octetIndex-1;
                            (*neededMemList)->regularNode = GT_TRUE;
                            (*neededMemList)--;
                            currLpmEngineMemPtr--;
                            **memPoolList = currLpmEngineMemPtr[0]->structsMemPool;
                            if (currLpmEngineMemPtr[0]->structsMemPool!=0)
                            {
                                memoryBlockTakenArr[currLpmEngineMemPtr[0]->ramIndex]++;
                            }
                            /* return pool ptr on correct octet */
                            currLpmEngineMemPtr++;
                            (*memPoolList)--;
                            **memListOctetIndexList=octetIndex-1;
                            (*memListOctetIndexList)--;
                        }
                        else
                        {
                            /* As a result of range adding bucket turned into regular. So its group of nodes */
                            /* size was changed. It is needed to go back and update group of nodes size in */
                            /* previous octet*/
                            prevLevelGroupOfNodesIndex = 0; /* for compressed */
                            if (prevLevelBucketPtr==NULL)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "\n ERROR unexpected prevLevelBucketPtr==NULL \n");
                            }
                            if (prevLevelBucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                            {
                                prevLevelGroupOfNodesIndex = prevStartAddr/NUMBER_OF_RANGES_IN_SUBNODE;
                                (*neededMemList)->regularNode = GT_TRUE;
                            }
                            else
                            {
                                (*neededMemList)->regularNode = GT_FALSE;
                            }

                            if(prevLevelBucketPtr->hwGroupOffsetHandle[prevLevelGroupOfNodesIndex]==0)
                            {
                                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected prevLevelBucketPtr->hwGroupOffsetHandle[prevLevelGroupOfNodesIndex]=0\n");
                            }
                            prevLevelGroupOfNodesSize =
                                PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(prevLevelBucketPtr->hwGroupOffsetHandle[prevLevelGroupOfNodesIndex]);
                            if (prevLevelGroupOfNodesSize > 264)
                            {
                                cpssOsPrintf(" The GON size too big!!!!!! ERROR!!!!\n");
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                            }
                            prevLevelGroupOfNodesSize = prevLevelGroupOfNodesSize -
                                PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS + PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;
                            if (prevLevelGroupOfNodesSize > 264)
                            {
                                cpssOsPrintf(" The GON size too big!!!!!! ERROR!!!!\n");
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                            }

                            /* here we have clear allocation case for previous level */

                            /*  Check if incase we use a swap area, the memory can be added.
                                this means that there is free block near the used block and
                                we can use them to create a new bigger block.
                                in case of add-bulk operation we will not support defrag
                                and the use of swap area*/
                            if ((defragmentationEnable==GT_TRUE)&&
                                (prevLevelBucketPtr->hwGroupOffsetHandle[prevLevelGroupOfNodesIndex]!=0)&&
                                (insertMode!=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E))
                            {
                                rc = prvCpssDxChLpmSip6RamMngFindIfSwapAreaCanBeUsefulToFindFreeBlock(prevLevelBucketPtr->hwGroupOffsetHandle[prevLevelGroupOfNodesIndex],
                                                                                                      prevLevelGroupOfNodesSize,
                                                                                                      &freeBlockCanBeFound);
                                if (rc!=GT_OK)
                                {
                                    return rc;
                                }
                            }

                            if (freeBlockCanBeFound==GT_TRUE)
                            {
                                (*neededMemList)->neededMemoryBlocksSwapUsedForAdd[prevLevelGroupOfNodesIndex] = GT_TRUE;
                                (*neededMemList)->neededMemoryBlocksSwapIndexUsedForAdd[prevLevelGroupOfNodesIndex]=0;/* no use yet */
                                SET_DMM_BLOCK_WAS_MOVED(prevLevelBucketPtr->hwGroupOffsetHandle[prevLevelGroupOfNodesIndex],0);/* reset moved bit */
                                (*neededMemList)->neededMemoryBlocksOldHandleAddr[prevLevelGroupOfNodesIndex] = prevLevelBucketPtr->hwGroupOffsetHandle[prevLevelGroupOfNodesIndex];
                                (*neededMemList)->neededMemoryBlocksOldShadowBucket[prevLevelGroupOfNodesIndex] = prevLevelBucketPtr;
                                (*neededMemList)->neededMemoryBlocksOldHandleGonIndex[prevLevelGroupOfNodesIndex] = prevLevelGroupOfNodesIndex;
                                freeBlockCanBeFound=GT_FALSE;
                            }
                            else
                            {
                                (*neededMemList)->neededMemoryBlocksSwapUsedForAdd[prevLevelGroupOfNodesIndex] = GT_FALSE;
                                (*neededMemList)->neededMemoryBlocksSwapIndexUsedForAdd[prevLevelGroupOfNodesIndex]=0;/* no use */
                                if (prevLevelBucketPtr->hwGroupOffsetHandle[prevLevelGroupOfNodesIndex]!=0)
                                {
                                    SET_DMM_BLOCK_WAS_MOVED(prevLevelBucketPtr->hwGroupOffsetHandle[prevLevelGroupOfNodesIndex], 0); /* reset moved bit */
                                }
                                (*neededMemList)->neededMemoryBlocksOldHandleAddr[prevLevelGroupOfNodesIndex] = prevLevelBucketPtr->hwGroupOffsetHandle[prevLevelGroupOfNodesIndex];
                                (*neededMemList)->neededMemoryBlocksOldShadowBucket[prevLevelGroupOfNodesIndex] = prevLevelBucketPtr;
                                (*neededMemList)->neededMemoryBlocksOldHandleGonIndex[prevLevelGroupOfNodesIndex] = prevLevelGroupOfNodesIndex;
                            }

                            (*neededMemList)->neededMemoryBlocksSizes[prevLevelGroupOfNodesIndex] =
                                prevLevelGroupOfNodesSize;
                            (*memListLenPtr)++;
                            (*neededMemList)->octetId = octetIndex-1;
                            (*neededMemList)--;
                            **memPoolList = prevOctetstructsMemPool;
                            if (prevOctetstructsMemPool!=0)
                            {
                                memoryBlockTakenArr[prevOctetstructsRamIndex]++;
                            }
                            (*memPoolList)--;
                            **memListOctetIndexList=octetIndex-1;
                            (*memListOctetIndexList)--;

                        }
                        /* new 6 group of nodes would be created */
                        cpssOsMemSet(grOfNodesIndexes,0,sizeof(grOfNodesIndexes));
                        cpssOsMemSet(grOfNodesSizes,0,sizeof(grOfNodesSizes));

                        rc = lpmFalconGetGroupOfNodesSizes(copyOfCurrentBucketPtr,
                                                           CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E,
                                                           *currAddrByte,currPrefix,numOfBucketRanges,
                                                           GT_TRUE,planedNewBucket[octetIndex],&numberOfBvLineRanges[0],
                                                           &grOfNodesIndexes[0],&grOfNodesSizes[0]);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                        cpssOsMemSet(numberOfBvLineRanges,0,sizeof(numberOfBvLineRanges));
                        allocIsNeeded = GT_TRUE;
                        for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                        {
                            allocIsNeededPerGroup[i] = GT_TRUE;
                        }
                    }
                    if (allocIsNeeded == GT_TRUE)
                    {
                        /* if we realy need new allocation and we move from one GON (compressed) to
                           6 GONs (regular) only the biggest gon out of the 6 GONs can have an old handle
                           we need to chose the biggest GON out of the 6 ones that will
                           hold the old handle address */
                        /* first check that only one handle address is legal in the old bucket */
                        for (i =0; i <PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                        {
                            if ((copyOfCurrentBucketPtr->hwGroupOffsetHandle[i] != 0) &&
                                (copyOfCurrentBucketPtr->hwGroupOffsetHandle[i] != DMM_BLOCK_NOT_FOUND))
                            {
                                numberOfOldHandles++;
                            }

                            /* find number of GONs */
                            if (grOfNodesSizes[i]!=0)
                            {
                                numberOfNewGons++;
                            }

                            /* find the biggest GON out of the 6 */
                            if (grOfNodesSizes[i]>counterSizeOftheBiggestGon)
                            {
                                counterSizeOftheBiggestGon = grOfNodesSizes[i];
                                counterIndexOftheBiggestGon = i;
                            }
                        }
                        if ((numberOfOldHandles==1)&&(numberOfNewGons==6))/* moving from compress to regular */
                        {
                           oldHandleForOneOfSixGonsFound = GT_TRUE;
                        }
                        else
                        {
                            oldHandleForOneOfSixGonsFound = GT_FALSE;
                        }

                        for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                        {
                            if (grOfNodesIndexes[i] == GT_TRUE)
                            {
                                if (allocIsNeededPerGroup[i] == GT_TRUE)
                                {
                                    (*neededMemList)->neededMemoryBlocksSizes[i] = grOfNodesSizes[i];
                                    (*neededMemList)->regularNode = GT_TRUE;
                                    (*neededMemList)->octetId = octetIndex;

                                    if(numberOfOldHandles==1 && numberOfNewGons==6)/* comp -->reg */
                                    {
                                        if(counterIndexOftheBiggestGon==i)
                                        {
                                            /* the old handle is always the first in the list */
                                            hwGroupOffsetHandleToUse=copyOfCurrentBucketPtr->hwGroupOffsetHandle[0];
                                            grOfNodesSizesToUse = counterSizeOftheBiggestGon;
                                            hwGroupOffsetHandleToUseGonIndex = 0xFFFE;/* tell us to look at hwGroupOffsetHandle[0] and not hwGroupOffsetHandle[i]*/
                                        }
                                        else
                                        {
                                            /* all the rest of the handles is 0, swap should not be an option  */
                                            hwGroupOffsetHandleToUse=0;
                                            grOfNodesSizesToUse = 0;/* will not be used since hwGroupOffsetHandleToUse=0 so will not get into relevant code of swap */
                                            hwGroupOffsetHandleToUseGonIndex=0;
                                        }

                                    }
                                    else
                                    {
                                        hwGroupOffsetHandleToUse = copyOfCurrentBucketPtr->hwGroupOffsetHandle[i];
                                        grOfNodesSizesToUse = grOfNodesSizes[i];
                                        hwGroupOffsetHandleToUseGonIndex=i;
                                    }

                                   /*  Check if incase we use a swap area, the memory can be added.
                                        this means that there is free block near the used block and
                                        we can use them to create a new bigger block.
                                        in case of add-bulk operation we will not support defrag
                                        and the use of swap area*/
                                    if ((defragmentationEnable==GT_TRUE)&&
                                        (hwGroupOffsetHandleToUse!=0)&&
                                        ((oldHandleForOneOfSixGonsFound==GT_TRUE && counterIndexOftheBiggestGon==i)||
                                         (oldHandleForOneOfSixGonsFound==GT_FALSE))&&
                                        (insertMode!=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E))
                                    {
                                        rc = prvCpssDxChLpmSip6RamMngFindIfSwapAreaCanBeUsefulToFindFreeBlock(hwGroupOffsetHandleToUse,
                                                                                                              grOfNodesSizesToUse,
                                                                                                              &freeBlockCanBeFound);
                                        if (rc!=GT_OK)
                                        {
                                            return rc;
                                        }
                                    }

                                    if (freeBlockCanBeFound==GT_TRUE)
                                    {
                                         /* update the field that indicate that swap memory can be useful in case of memory full
                                           this do not mean it will actually be used */
                                        (*neededMemList)->neededMemoryBlocksSwapUsedForAdd[i] = GT_TRUE;
                                        (*neededMemList)->neededMemoryBlocksSwapIndexUsedForAdd[i]=0;/* no use yet */
                                        SET_DMM_BLOCK_WAS_MOVED(hwGroupOffsetHandleToUse,0);/* reset moved bit */
                                        (*neededMemList)->neededMemoryBlocksOldHandleAddr[i] = hwGroupOffsetHandleToUse;
                                        (*neededMemList)->neededMemoryBlocksOldShadowBucket[i] = copyOfCurrentBucketPtr;
                                        (*neededMemList)->neededMemoryBlocksOldHandleGonIndex[i] = hwGroupOffsetHandleToUseGonIndex;
                                        freeBlockCanBeFound = GT_FALSE;
                                    }
                                    else
                                    {
                                        (*neededMemList)->neededMemoryBlocksSwapUsedForAdd[i] = GT_FALSE;
                                        (*neededMemList)->neededMemoryBlocksSwapIndexUsedForAdd[i]=0;/* no use */
                                        if (hwGroupOffsetHandleToUse!=0)
                                        {
                                            /* in this case - old handle is not used in swap code but migth be used in the shrink
                                               we must check the handle is valid */
                                            SET_DMM_BLOCK_WAS_MOVED(hwGroupOffsetHandleToUse, 0); /* reset moved bit */
                                        }
                                        (*neededMemList)->neededMemoryBlocksOldHandleAddr[i] = hwGroupOffsetHandleToUse;
                                        (*neededMemList)->neededMemoryBlocksOldShadowBucket[i] = copyOfCurrentBucketPtr;
                                        (*neededMemList)->neededMemoryBlocksOldHandleGonIndex[i] = hwGroupOffsetHandleToUseGonIndex;
                                    }
                                }
                            }
                        }
                    }
                }
                if (allocIsNeeded == GT_TRUE)
                {
                    **memPoolList = currLpmEngineMemPtr[0]->structsMemPool;
                    if (currLpmEngineMemPtr[0]->structsMemPool!=0)
                    {
                        memoryBlockTakenArr[currLpmEngineMemPtr[0]->ramIndex]++;
                    }
                    (*memListLenPtr)++;
                    (*memPoolList)--;
                    (*neededMemList)--;
                    **memListOctetIndexList=octetIndex;
                    (*memListOctetIndexList)--;
                    allocIsNeeded = GT_FALSE;
                    cpssOsMemSet(allocIsNeededPerGroup,GT_FALSE,sizeof(allocIsNeededPerGroup));
                }
            }
        }

        prevStartAddr = startAddr;
        prevLevelBucketPtr = copyOfCurrentBucketPtr;
        currAddrByte++;
        currPrefix = NEXT_BUCKET_PREFIX_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
        rootBucketFlag = PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E;
        prevOctetstructsMemPool = currLpmEngineMemPtr[0]->structsMemPool;
        prevOctetstructsRamIndex = currLpmEngineMemPtr[0]->ramIndex;
        currLpmEngineMemPtr++;
        octetIndex++;
        level++;
    }

    return rc;
}



/**
* @internal mergeCheckSip6 function
* @endinternal
*
* @brief   Returns 1 if the two input ranges can be merged to one range.
*
* @param[in] leftRangePtr             - The first range to check for merging.
* @param[in] rightRangePtr            - The second range to check for merging.
* @param[in] levelPrefix              - The current lpm level prefix.
*                                       1 if the two input ranges can be merged to one range.
*/
GT_U8 mergeCheckSip6
(
    IN PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *leftRangePtr,
    IN PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *rightRangePtr,
    IN GT_U8                                    levelPrefix
)
{
    GT_U32  rightStartAddr;      /* The start address of the right range */
                                 /* after taking away the unneeded LSBs  */
                                 /* according to the range's mask.       */
    GT_U32  rightPrefix;         /* Prefix length of the right range.    */
    GT_U32  leftPrefix;          /* Prefix length of the left range.     */

    if ((leftRangePtr == NULL) || (rightRangePtr == NULL))
        return 0;

    if (((leftRangePtr->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
         (leftRangePtr->pointerType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) &&
         ((GT_U32)(leftRangePtr->pointerType) != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)) ||
        ((rightRangePtr->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
         (rightRangePtr->pointerType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) &&
         ((GT_U32)(rightRangePtr->pointerType) != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)))
        return 0;

    rightPrefix = mask2PrefixLength(rightRangePtr->mask,levelPrefix);
    leftPrefix  = mask2PrefixLength(leftRangePtr->mask,levelPrefix);

    rightStartAddr = (rightRangePtr->startAddr & (0xFFFFFFFF <<
                                               (levelPrefix - rightPrefix)));

    if((leftRangePtr->startAddr >= rightStartAddr) && (leftPrefix == rightPrefix))
        return 1;

    return 0;
}







/**
* @internal mergeRangeSip6 function
* @endinternal
*
* @brief This function merges a range. According to one of the following possible merges:
*           PRV_CPSS_DXCH_LPM_RAM_MERGE_OVERWRITE_E, PRV_CPSS_DXCH_LPM_RAM_MERGE_LOW_E,
*           PRV_CPSS_DXCH_LPM_RAM_MERGE_HIGH_E and PRV_CPSS_DXCH_LPM_RAM_MERGE_MID_E
*
* @param[in] rangePtr          - Pointer to the range to be merged.
* @param[in] startAddr         - The low address of the range to be merged.
* @param[in] prefixLength      - The length of the dominant prefix of the range.
* @param[in] prevRangePtr      - A pointer to the lower address neighbor of the range to be
*                                merged. NULL if rangePtr is the 1st in the list.
* @param[in] levelPrefix       - The current lpm level prefix.
* @param[out] rangePtr          - A pointer to the next node in the list.
* @param[out] numOfNewRangesPtr - Number of new created ranges as a result of the split.
*
* @retval   GT_OK on success,
* @retval   GT_FAIL otherwise.
*/
static GT_STATUS mergeRangeSip6
(
    IN    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT       bucketType,
    INOUT PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    **rangePtr,
    IN    GT_U32                                    startAddr,
    IN    GT_U32                                    endAddr,
    IN    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    *prevRangePtr,
    IN    GT_U8                                     levelPrefix,
    OUT   GT_32                                     *numOfNewRangesPtr,
    OUT   GT_32                                     *numOfNewHwRangesPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *curRangePtr = (*rangePtr);/* Temp vars*/
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *tempNextRangePtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *tempPrevRangePtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_MERGE_METHOD_ENT mergeMethod;     /* The method by which too perform the  */
                                                            /* ranges merge.                        */
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *tmpPtr; /* Temporary range pointer to be used   */
                                /* in range operation.                  */
    GT_U32                                           startSubNodeIndex = 0;
    GT_U32                                           endSubNodeIndex = 0;

    /* warnings fix */
    *numOfNewHwRangesPtr = 0;
    /* calculate number of real ranges in bit vector */
    if (bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
        startSubNodeIndex = startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
        endSubNodeIndex =   endAddr/NUMBER_OF_RANGES_IN_SUBNODE;
        if (startSubNodeIndex == endSubNodeIndex)
        {
            if (endAddr == endSubNodeAddress[endSubNodeIndex])
            {
                tempNextRangePtr = NULL;
            }
            else
            {
                tempNextRangePtr = curRangePtr->next;
            }
            if (startAddr == startSubNodeAddress[startSubNodeIndex])
            {
                tempPrevRangePtr = NULL;
            }
            else
            {
                tempPrevRangePtr = prevRangePtr;
            }
            mergeMethod = ((mergeCheckSip6(curRangePtr,tempNextRangePtr,
                                       levelPrefix) ? 1 : 0) +
                           (mergeCheckSip6(tempPrevRangePtr,curRangePtr,levelPrefix) ? 2 : 0));
            switch (mergeMethod)
            {
            case PRV_CPSS_DXCH_LPM_RAM_MERGE_OVERWRITE_E: /* The old range fell       */
                *numOfNewHwRangesPtr = 0;
                break;
            case PRV_CPSS_DXCH_LPM_RAM_MERGE_HIGH_E:  /* Merge the upper part of the range */
                *numOfNewHwRangesPtr = -1;
                break;
            case PRV_CPSS_DXCH_LPM_RAM_MERGE_LOW_E: /* Merge the lower part of the range */
                *numOfNewHwRangesPtr = -1;
                break;
            case PRV_CPSS_DXCH_LPM_RAM_MERGE_MID_E: /* Merge the ranges from both sides of the range.*/
                *numOfNewHwRangesPtr = -2;
                break;
            default:
                break;
            }
        }
    }
    mergeMethod = ((mergeCheckSip6(curRangePtr,curRangePtr->next,
                               levelPrefix) ? 1 : 0) +
                   (mergeCheckSip6(prevRangePtr,curRangePtr,levelPrefix) ? 2 : 0));

    switch (mergeMethod)
    {
    case PRV_CPSS_DXCH_LPM_RAM_MERGE_OVERWRITE_E: /* The old range fell       */
                                                  /* exactly on a older range */
        *numOfNewRangesPtr = 0;

        curRangePtr->updateRangeInHw = GT_TRUE;
        break;

    case PRV_CPSS_DXCH_LPM_RAM_MERGE_HIGH_E:  /* Merge the upper part of the range */
        curRangePtr->mask = curRangePtr->next->mask;

        curRangePtr->lowerLpmPtr = curRangePtr->next->lowerLpmPtr;
        curRangePtr->pointerType = curRangePtr->next->pointerType;

        tmpPtr = curRangePtr->next;
        curRangePtr->next = curRangePtr->next->next;
        (*rangePtr) = curRangePtr;
        cpssOsLpmFree(tmpPtr);
        tmpPtr = NULL;
        *numOfNewRangesPtr = -1;

        curRangePtr->updateRangeInHw = GT_TRUE;

        break;

    case PRV_CPSS_DXCH_LPM_RAM_MERGE_LOW_E: /* Merge the lower part of the range */

        prevRangePtr->next = curRangePtr->next;

        cpssOsLpmFree(curRangePtr);
        curRangePtr = NULL;
        (*rangePtr) = prevRangePtr;
        *numOfNewRangesPtr = -1;
        prevRangePtr->updateRangeInHw = GT_TRUE;
        break;

    case PRV_CPSS_DXCH_LPM_RAM_MERGE_MID_E: /* Merge the ranges from both */
                                            /* sides of the range.        */

        prevRangePtr->next = curRangePtr->next->next;

        cpssOsLpmFree(curRangePtr->next);
        curRangePtr->next = NULL;

        cpssOsLpmFree(curRangePtr);
        curRangePtr = NULL;

        (*rangePtr) = prevRangePtr;
        *numOfNewRangesPtr = -2;

        prevRangePtr->updateRangeInHw = GT_TRUE;
        break;

    default:
        break;
    }

    if (bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
        *numOfNewHwRangesPtr = *numOfNewRangesPtr;
    }
    else
        if (startSubNodeIndex != endSubNodeIndex)
        {
            *numOfNewHwRangesPtr = *numOfNewRangesPtr;
        }
    return GT_OK;
}
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
)
{
    GT_U8 startAddr;           /* The startAddr and endAddr of the     */
    GT_U8 endAddr;             /* given prefix in the current level.   */

    GT_U32 rangePrefixLength;   /* The prefix length represented by the */
                                /* current range's mask field.          */
    GT_U32 newPrefixLength;     /* The new prefix length that the       */
                                /* current range represents.            */

    GT_U8  newStartAddr;        /* The startAddr of the range that      */
                                /* should replace the deleted one.      */

   /* Was the bucket resized as a result of deletion.                  */
    LPM_ROOT_BUCKET_UPDATE_ENT               rootBucketUpdateState = LPM_ROOT_BUCKET_UPDATE_NONE_E;
    GT_BOOL resizeGroup;       /* Was the group resized as a  */
                                /* result of deletion.                  */

    GT_BOOL doHwUpdate;         /* is it needed to update the HW */
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *newNextHopPtr;/* Next hop representing
                                                           the prefix that
                                                           replaces the deleted
                                                           one. */
    GT_32 numOfNewRanges;       /* Number of new ranges as a result of  */
                                /* range merging.                       */
    GT_32 numberOfNewHwRanges;   /* Number of new hw ranges as a result of  */
                                /* range merging.(relates to bitvector)     */
    LPM_BUCKET_UPDATE_MODE_ENT bucketUpdateMode;/* indicates whether to write
                                                   the bucket to the device's
                                                   RAM.*/
    GT_BOOL indicateSiblingUpdate;
    GT_PTR  nextPtr;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT  insertMode;

    GT_U32 level = 0;
    GT_U32 j=0;
    GT_U32 oldNumOfRanges = 0;
    GT_BOOL updateOld,updateUpperLevel = GT_FALSE;
    GT_STATUS retVal = GT_OK;

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC **pPrevRange;/* The first range that overlaps with   */
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC **pRange;    /* address & prefix, and a pointer to  */
                                                        /* the range that comes before it.      */
    PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT              **currLpmPtr;
    GT_U8                                                 *currAddrByte;
    GT_U32                                                currPrefix;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT                   **currBucketPtrType;
    GT_BOOL                                               isDestTreeRootBucket = GT_FALSE;
    GT_BOOL                                               isSrcTreeRootBucket = GT_FALSE;
    GT_BOOL                                               regularCompressedConversion = GT_FALSE;
    GT_BOOL                                               rangeIsRemoved = GT_FALSE;
    GT_BOOL                                               overwriteIsDone = GT_FALSE;
    GT_BOOL                                               lastLevelBucketEmbLeaves = GT_FALSE;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT                   newNextHopPtrType;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT                   updatedBucketType;
    GT_U32 subnodesSizes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_BOOL subnodesIndexes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U32 i;
    GT_U8                                                 startSubNodeIndex = 0;
    GT_U8                                                 endSubNodeIndex = 0;
    PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC                 tempParentUpdateParams;
    GT_U32                                                loopCounter = 0;

    currAddrByte = addrCurBytePtr;
    currPrefix = prefix;

    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(lpmPtrArray),0,sizeof(PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT*)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pRangeArray),0,sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC*)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pPrvRangeArray),0,sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC*)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrTypeArray),0,sizeof(GT_U8*)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(&tempParentUpdateParams,0,sizeof(tempParentUpdateParams));
    cpssOsMemSet(parentUpdateParams->nodeWasCrearedInNextLevel, 0, sizeof(parentUpdateParams->nodeWasCrearedInNextLevel));

    PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrTypeArray)[0] = bucketTypePtr;
    PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(lpmPtrArray)[0] = lpmPtr;
    parentUpdateParams->bucketPtrArray[0] = lpmPtr->nextBucket;
    currLpmPtr = PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(lpmPtrArray);
    pPrevRange = PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pPrvRangeArray);
    pRange = PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pRangeArray);
    currBucketPtrType = PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrTypeArray);

    loopCounter = 0;
    /* lpmEngineMemPtrPtr points on memory pool for octet 0*/
    /* gon for bucket related to octet 0 can't be in banks associated with octet 0 */
    lpmEngineMemPtrPtr++;
    loopCounter++;

    /* first run and record all the neccesary info down the bucket tree */
    for(;;)
    {
        if (*currLpmPtr== NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG); /* the prefix is not in the tree */

        calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,&endAddr);

        *pRange = find1stOverlap((*currLpmPtr)->nextBucket,startAddr,pPrevRange);

        if(level >= MAX_LPM_LEVELS_CNS) {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "\n ERROR in prvCpssDxChLpmSip6RamMngEntryDelete,(level >= MAX_LPM_LEVELS_CNS) \n");
        }
        parentUpdateParams->rangePtrArray[level] = *pRange;
       /* The deleted prefix ends in a deeper level,   */
        /* call the delete function recursively.        */
        /* in multicast may be use leaf_tigger type */
        if ((currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS) &&
            ((*pRange)->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
            ((*pRange)->pointerType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  &&
            ((GT_U32)((*pRange)->pointerType) != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
        {
            currLpmPtr[1] = &((*pRange)->lowerLpmPtr);
            if(level+1 >= MAX_LPM_LEVELS_CNS) {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "\n ERROR in prvCpssDxChLpmSip6RamMngEntryDelete,(level >= MAX_LPM_LEVELS_CNS) \n");
            }
            parentUpdateParams->bucketPtrArray[level+1] = (*pRange)->lowerLpmPtr.nextBucket;
            currBucketPtrType[1] = &((*pRange)->pointerType);
            currAddrByte++;
            loopCounter++;
            if (((ucMcType == PRV_CPSS_DXCH_LPM_ALLOC_UC_TYPE_E) &&
                 (loopCounter < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS)) ||
                ((ucMcType != PRV_CPSS_DXCH_LPM_ALLOC_UC_TYPE_E) &&
                 (loopCounter < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_MC_PROTOCOL_CNS)))
            {
                lpmEngineMemPtrPtr++;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Shouldn't happen: lpmEngineMemPtrPtr exceeds limit");
            }

            currPrefix = NEXT_BUCKET_PREFIX_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

            /* advance the array pointers */
            currLpmPtr++;
            pRange++;
            pPrevRange++;
            currBucketPtrType++;
            level++;
        }
        else
            break;
    }

    /* now we are on the lowest level for this prefix, delete the from trie */
    retVal = delFromTrie((*currLpmPtr)->nextBucket,startAddr,currPrefix,
                         PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,delEntryPtr);
    if (retVal != GT_OK)
    {
        if (rollBack == GT_FALSE)
        {
            return retVal;
        }
    }

    while (currLpmPtr >= PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(lpmPtrArray))
    {
        /* reset the flags */
        rootBucketUpdateState    = LPM_ROOT_BUCKET_UPDATE_NONE_E;
        resizeGroup              = GT_FALSE;
        doHwUpdate               = GT_FALSE;
        indicateSiblingUpdate    = GT_FALSE;
        rangeIsRemoved           = GT_FALSE;
        overwriteIsDone          = GT_FALSE;
        lastLevelBucketEmbLeaves = GT_FALSE;
        updatedBucketType = (*currLpmPtr)->nextBucket->bucketType;

        if ((currLpmPtr == PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(lpmPtrArray)) &&
            (rootBucketFlag !=PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E))
        {
            if (ucMcType != PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E)
            {
                isDestTreeRootBucket = GT_TRUE;
                isSrcTreeRootBucket = GT_FALSE;
            }
            else
            {
                isSrcTreeRootBucket = GT_TRUE;
                isDestTreeRootBucket = GT_FALSE;
            }
        }

        calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,&endAddr);
        do
        {
            /* The deleted prefix ends in a deeper level,   */
            if (currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
            {
                /* check if the bucket was resized , thus moved in memory */
                if (updateUpperLevel == GT_TRUE)
                {
                    if (parentUpdateParams->nextLevelDoneBySwapFunc == GT_FALSE)
                    {
                        (*pRange)->updateRangeInHw = GT_TRUE;
                        doHwUpdate = GT_TRUE;
                        if (regularCompressedConversion == GT_TRUE)
                        {
                            /* The bucket on next deeper level turn to compressed */
                            /* The size of current GON is changed */
                            cpssOsMemSet(subnodesIndexes,0,sizeof(subnodesIndexes));
                            cpssOsMemSet(subnodesSizes,0,sizeof(subnodesSizes));
                            retVal = lpmFalconGetGroupOfNodesSizes((*currLpmPtr)->nextBucket,
                                                                   updatedBucketType,
                                                                    *currAddrByte,currPrefix,
                                                                   (*currLpmPtr)->nextBucket->numOfRanges,
                                                                   GT_FALSE,GT_FALSE,NULL,
                                                                   &subnodesIndexes[0],&subnodesSizes[0]);
                            if (retVal != GT_OK)
                            {
                                return retVal;
                            }

                            resizeGroup = GT_TRUE;
                            regularCompressedConversion = GT_FALSE;
                        }
                    }
                    parentUpdateParams->nextLevelDoneBySwapFunc = GT_FALSE;

                    /* the upper level has been marked for update , remove
                       the signal */
                    if (resizeGroup == GT_FALSE)
                        updateUpperLevel = GT_FALSE;
                }

                /* Check the type of the next bucket after deletion */
                if (((*pRange)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                    ((*pRange)->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E))
                {
                    /* if it's type "route entry pointer" it means there is no
                       need for that pointed bucket (and it actually been
                       deleted in the lower level) so merge the range */
                    mergeRangeSip6((*currLpmPtr)->nextBucket->bucketType,pRange,startAddr,endAddr,
                                   *pPrevRange,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,&numOfNewRanges,
                                   &numberOfNewHwRanges);
                    if ((*currLpmPtr)->nextBucket->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                    {
                        if ((numberOfNewHwRanges == 0) && (numOfNewRanges == -1))
                        {
                            if ((GT_U16)((*currLpmPtr)->nextBucket->numOfRanges + numOfNewRanges) >= 11)
                            {
                                /* An overwrite merge was done (no change in the amount
                                   of ranges) extract the appropriate prefix from the
                                   trie, and insert it in the same place.*/
                                rangePrefixLength =
                                    mask2PrefixLength((*pRange)->mask,
                                                      PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

                                nextPtr = getFromTrie((*currLpmPtr)->nextBucket,
                                                      (*pRange)->startAddr,
                                                      rangePrefixLength,
                                                      PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                                if (nextPtr==NULL)
                                {
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "\n ERROR got nextPtr==NULL from getFromTrie \n");
                                }
                                PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(((PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC*)nextPtr)->routeEntryMethod,
                                                                                 (*pRange)->pointerType);
                                (*pRange)->lowerLpmPtr.nextHopEntry = nextPtr;

                                doHwUpdate = GT_TRUE;

                            }
                        }
                    }
                    if (numOfNewRanges == 0)
                    {
                        /* An overwrite merge was done (no change in the amount
                           of ranges) extract the appropriate prefix from the
                           trie, and insert it in the same place.*/
                        /* The override can impact group of nodes size. The GON size must be
                           recalculated to understand if resize is needed */
                        cpssOsMemSet(subnodesIndexes,0,sizeof(subnodesIndexes));
                        cpssOsMemSet(subnodesSizes,0,sizeof(subnodesSizes));
                        retVal = lpmFalconGetGroupOfNodesSizes((*currLpmPtr)->nextBucket,
                                                               updatedBucketType,
                                                                *currAddrByte,currPrefix,
                                                               (*currLpmPtr)->nextBucket->numOfRanges,
                                                               GT_FALSE,GT_FALSE,NULL,
                                                               &subnodesIndexes[0],&subnodesSizes[0]);
                        if (retVal != GT_OK)
                        {
                            return retVal;
                        }
                        for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                        {
                            if (subnodesIndexes[i] == GT_TRUE)
                            {
                                if (subnodesSizes[i] == 0)
                                {
                                    /* it is possible only if it become  Last level  with bucket became 3-3,2-2 */
                                    /* as a result of delettion deeper level */
                                    lastLevelBucketEmbLeaves = GT_TRUE;

                                    if ((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[0] != 0)
                                    {
                                        resizeGroup = GT_TRUE;
                                        /* the memory must be freed */
                                    }
                                    else
                                    {
                                        resizeGroup = GT_FALSE;
                                    }
                                    break;
                                }
                                if ((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[i]==0)
                                {
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "\n ERROR in prvCpssDxChLpmSip6RamMngEntryDelete,(*currLpmPtr)->nextBucket->hwGroupOffsetHandle[i]==0 \n");
                                }
                                if (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[i]) != subnodesSizes[i])
                                {
                                    resizeGroup = GT_TRUE;
                                    break;
                                }
                            }

                        }
                        rangePrefixLength =
                            mask2PrefixLength((*pRange)->mask,
                                              PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

                        nextPtr = getFromTrie((*currLpmPtr)->nextBucket,
                                              (*pRange)->startAddr,
                                              rangePrefixLength,
                                              PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                        if (nextPtr==NULL)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "\n ERROR got nextPtr==NULL from getFromTrie \n");
                        }
                        PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(((PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC*)nextPtr)->routeEntryMethod,
                                                                         (*pRange)->pointerType);
                        (*pRange)->lowerLpmPtr.nextHopEntry = nextPtr;

                        doHwUpdate = GT_TRUE;
                        overwriteIsDone = GT_TRUE;
                    }
                    else
                    {
                        /* update the number of ranges */
                        oldNumOfRanges = (*currLpmPtr)->nextBucket->numOfRanges;
                        (*currLpmPtr)->nextBucket->numOfRanges =
                            (GT_U16)((*currLpmPtr)->nextBucket->numOfRanges + numOfNewRanges);

                        /* check if range deletion cause to group resize */
                        /* indicate a resize has happen */
                        if ( ((*currLpmPtr)->nextBucket->numOfRanges == 1) &&
                             ((*currLpmPtr)->nextBucket->bucketType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
                        {
                            resizeGroup = GT_FALSE;
                        }
                        else
                            if (((*currLpmPtr)->nextBucket->numOfRanges == 1) &&
                                ((level != 0) || (protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)))
                            {
                                /* this bucket is going to be deleted fully*/
                                resizeGroup = GT_TRUE;
                            }

                        if  ( ( (oldNumOfRanges >= 11) && ((*currLpmPtr)->nextBucket->numOfRanges < 11)) ||
                               (((*currLpmPtr)->nextBucket->numOfRanges <= MAX_NUMBER_OF_COMPRESSED_RANGES_CNS) &&
                                ((*currLpmPtr)->nextBucket->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E) ))
                        {
                                resizeGroup = GT_TRUE;
                                regularCompressedConversion = GT_TRUE;
                                parentUpdateParams->isBvCompConvertion = GT_TRUE;
                                updatedBucketType = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;
                        }

                        /* if ((*currLpmPtr)->nextBucket->numOfRanges > 1) */
                        if (((*currLpmPtr)->nextBucket->numOfRanges > 1) ||
                            (((*currLpmPtr)->nextBucket->numOfRanges == 1) && (level == 0) &&
                             (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)))
                        {
                            /* in this case 1 or 2 leaves removed.*/
                            /* calculate group of nodes size */
                            cpssOsMemSet(subnodesIndexes,0,sizeof(subnodesIndexes));
                            cpssOsMemSet(subnodesSizes,0,sizeof(subnodesSizes));
                            retVal = lpmFalconGetGroupOfNodesSizes((*currLpmPtr)->nextBucket,
                                                                   updatedBucketType,
                                                                   *currAddrByte,currPrefix,
                                                                   (*currLpmPtr)->nextBucket->numOfRanges,
                                                                   GT_FALSE,GT_FALSE,NULL,
                                                                   &subnodesIndexes[0],&subnodesSizes[0]);
                            if (retVal != GT_OK)
                            {
                                return retVal;
                            }
                            for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                            {
                                if ( subnodesIndexes[i] == GT_TRUE)
                                {
                                    if (subnodesSizes[i] == 0)
                                    {
                                        /* it is possible only if it become  Last level  with bucket became 3-3,2-2 */
                                        /* as a result of delettion deeper level */
                                        lastLevelBucketEmbLeaves = GT_TRUE;

                                        if ((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[0] != 0)
                                        {
                                            resizeGroup = GT_TRUE;
                                            /* the memory must be freed */
                                        }
                                        else
                                        {
                                            resizeGroup = GT_FALSE;
                                        }
                                        break;
                                    }
                                    if ((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[i]==0)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "\n ERROR in prvCpssDxChLpmSip6RamMngEntryDelete,(*currLpmPtr)->nextBucket->hwGroupOffsetHandle[i]==0 \n");
                                    }
                                    if (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[i]) != subnodesSizes[i])
                                    {
                                        resizeGroup = GT_TRUE;
                                        break;
                                    }
                                }
                            }
                            if (resizeGroup == GT_FALSE)
                            {
                                /* There is no resize, but number of ranges was removed. The correspondent
                                   GON must be updated */
                                rangeIsRemoved = GT_TRUE;
                                doHwUpdate = GT_TRUE;
                            }
                        }
                        /* a change happend to the structure of the bucket
                        (removed ranges) clear the cash, and signal the upper
                        level */
                        (*currLpmPtr)->nextBucket->rangeCash = NULL;
                    }
                    updateUpperLevel = GT_TRUE;
                }
                else
                {
                    /* check if we are in a case that the number of ranges is not changed and
                       their value<11( fit to compress), but the bucket type is regular.
                       this mean we that even that the num of ranges is nor changes we should
                       recalculate the needed for delete. we migth need to move the regular
                       GON to a compress GON*/
                   if  (((*currLpmPtr)->nextBucket->numOfRanges <= MAX_NUMBER_OF_COMPRESSED_RANGES_CNS) &&
                         ((*currLpmPtr)->nextBucket->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E))
                   {
                       resizeGroup = GT_TRUE;
                       regularCompressedConversion = GT_TRUE;
                       parentUpdateParams->isBvCompConvertion = GT_TRUE;
                       updatedBucketType = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;

                       /* An overwrite merge was done (no change in the amount of ranges)
                          extract the appropriate prefix from the trie, and insert it in the same place.*/
                        /* The override can impact group of nodes size. The GON size must be
                           recalculated to understand if resize is needed */
                        cpssOsMemSet(subnodesIndexes,0,sizeof(subnodesIndexes));
                        cpssOsMemSet(subnodesSizes,0,sizeof(subnodesSizes));
                        retVal = lpmFalconGetGroupOfNodesSizes((*currLpmPtr)->nextBucket,
                                                           updatedBucketType,
                                                            *currAddrByte,currPrefix,
                                                           (*currLpmPtr)->nextBucket->numOfRanges,
                                                           GT_FALSE,GT_FALSE,NULL,
                                                           &subnodesIndexes[0],&subnodesSizes[0]);
                        if (retVal != GT_OK)
                        {
                            return retVal;
                        }

                        (*currLpmPtr)->nextBucket->rangeCash = NULL;
                        updateUpperLevel = GT_TRUE;
                   }
                }
            }
            else
            {
                /* The deleted prefix ends in this level.   */
                rangePrefixLength =
                    mask2PrefixLength((*pRange)->mask, PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

                if (currPrefix > rangePrefixLength)
                    /* should never happen */
                    if (rollBack == GT_FALSE)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

                (*pRange)->mask &= ~(PREFIX_2_MASK_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS));

                if (currPrefix == rangePrefixLength)
                {
                    /* The current range represents the deleted prefix. */
                    /* in multicast may be use leaf_trigger type */
                    if (((*pRange)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                        ((*pRange)->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
                        ((GT_U32)((*pRange)->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                    {
                        /* if the range's type "route entry pointer" it means
                           there is no need for this range, merge it */

                        mergeRangeSip6((*currLpmPtr)->nextBucket->bucketType, pRange,startAddr,endAddr,
                                       *pPrevRange,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,&numOfNewRanges,
                                       &numberOfNewHwRanges);

                        if ((*currLpmPtr)->nextBucket->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                        {
                            if ((numberOfNewHwRanges == 0) && (numOfNewRanges == -1))
                            {
                                if ((GT_U16)((*currLpmPtr)->nextBucket->numOfRanges + numOfNewRanges) >= 11)
                                {
                                    /* An overwrite merge was done (no change in the amount
                                       of ranges) extract the appropriate prefix from the
                                       trie, and insert it in the same place.*/
                                    rangePrefixLength =
                                        mask2PrefixLength((*pRange)->mask,
                                                          PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

                                    nextPtr = getFromTrie((*currLpmPtr)->nextBucket,
                                                          (*pRange)->startAddr,
                                                          rangePrefixLength,
                                                          PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                                    if (nextPtr==NULL)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "\n ERROR got nextPtr==NULL from getFromTrie \n");
                                    }
                                    PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(((PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC*)nextPtr)->routeEntryMethod,
                                                                                     (*pRange)->pointerType);
                                    (*pRange)->lowerLpmPtr.nextHopEntry = nextPtr;

                                    doHwUpdate = GT_TRUE;

                                }
                            }
                        }
                        if (numOfNewRanges == 0)
                        {
                           /* An overwrite merge was done (no change in the
                              amount of ranges) extract the appropriate prefix
                              from the trie, and insert it in the same
                              place.*/
                            cpssOsMemSet(subnodesIndexes,0,sizeof(subnodesIndexes));
                            cpssOsMemSet(subnodesSizes,0,sizeof(subnodesSizes));

                            retVal = lpmFalconGetGroupOfNodesSizes((*currLpmPtr)->nextBucket,
                                                                   updatedBucketType,
                                                                    *currAddrByte,currPrefix,
                                                                   (*currLpmPtr)->nextBucket->numOfRanges,
                                                                   GT_FALSE,GT_FALSE,NULL,
                                                                   &subnodesIndexes[0],&subnodesSizes[0]);
                            if (retVal != GT_OK)
                            {
                                return retVal;
                            }
                            for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                            {
                                if (subnodesIndexes[i] == GT_TRUE)
                                {
                                    if (subnodesSizes[i] == 0)
                                    {
                                        /* it is possible only if it become  Last level  with bucket became 3-3,2-2 */
                                        /* as a result of delettion deeper level */
                                        lastLevelBucketEmbLeaves = GT_TRUE;

                                        if ((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[0] != 0)
                                        {
                                            resizeGroup = GT_TRUE;
                                            /* the memory must be freed */
                                        }
                                        else
                                        {
                                            resizeGroup = GT_FALSE;
                                        }
                                        break;
                                    }
                                    if ((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[i]==0)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "\n ERROR in prvCpssDxChLpmSip6RamMngEntryDelete,(*currLpmPtr)->nextBucket->hwGroupOffsetHandle[i]==0 \n");
                                    }
                                    if (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[i]) != subnodesSizes[i])
                                    {
                                        resizeGroup = GT_TRUE;
                                        break;
                                    }
                                }
                            }

                            rangePrefixLength =
                                mask2PrefixLength((*pRange)->mask,
                                                  PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                            nextPtr =
                                getFromTrie((*currLpmPtr)->nextBucket,
                                            (*pRange)->startAddr,
                                            rangePrefixLength,
                                            PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                            if (nextPtr==NULL)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "\n ERROR got nextPtr==NULL from getFromTrie \n");
                            }
                            PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(((PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC*)nextPtr)->routeEntryMethod,
                                                                             (*pRange)->pointerType);
                            (*pRange)->lowerLpmPtr.nextHopEntry = nextPtr;
                            if ( (resizeGroup == GT_FALSE) && (lastLevelBucketEmbLeaves == GT_FALSE) )
                            {
                                /* There is no resize, but range was overwrited. The correspondent
                                   GON must be updated */
                                overwriteIsDone = GT_TRUE;
                                doHwUpdate = GT_TRUE;
                            }
                        }
                        else
                        {

                            /* update the amount of ranges */
                            oldNumOfRanges = (*currLpmPtr)->nextBucket->numOfRanges;
                            (*currLpmPtr)->nextBucket->numOfRanges =
                                (GT_U16)((*currLpmPtr)->nextBucket->numOfRanges + numOfNewRanges);
                            /* indicate a resize has happen */

                            if ( ((*currLpmPtr)->nextBucket->numOfRanges == 1) &&
                                 ( ((*currLpmPtr)->nextBucket->bucketType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) ||
                                    ((*currLpmPtr)->nextBucket->bucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E)) )
                            {
                                /* this is the deepest level: if node was 2 ranges 2 leaves or 3 ranges 3 leaves
                                   it doesn't points on GON: leaves are inside of node structure */
                              /*  lastLevelBucketEmbLeaves = GT_TRUE;*/
                                resizeGroup = GT_FALSE;
                            }
                            else
                                if ((*currLpmPtr)->nextBucket->numOfRanges == 1)
                                {
                                    /* this bucket is going to be deleted fully*/
                                    resizeGroup = GT_TRUE;
                                }


                            if (( (oldNumOfRanges >= 11) && ((*currLpmPtr)->nextBucket->numOfRanges < 11))||
                                (((*currLpmPtr)->nextBucket->numOfRanges <= MAX_NUMBER_OF_COMPRESSED_RANGES_CNS) &&
                                ((*currLpmPtr)->nextBucket->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E) ))
                            {
                                resizeGroup = GT_TRUE;
                                regularCompressedConversion = GT_TRUE;
                                parentUpdateParams->isBvCompConvertion = GT_TRUE;
                                updatedBucketType = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;
                            }
                            if ((*currLpmPtr)->nextBucket->numOfRanges > 1)
                            {
                                /* in this case 1 or 2 leaves removed.*/
                                /* calculate group of nodes size */
                                cpssOsMemSet(subnodesIndexes,0,sizeof(subnodesIndexes));
                                cpssOsMemSet(subnodesSizes,0,sizeof(subnodesSizes));

                                retVal = lpmFalconGetGroupOfNodesSizes((*currLpmPtr)->nextBucket,
                                                                       updatedBucketType,
                                                                       *currAddrByte,currPrefix,
                                                                       (*currLpmPtr)->nextBucket->numOfRanges,
                                                                       GT_FALSE,GT_FALSE,NULL,
                                                                       &subnodesIndexes[0],&subnodesSizes[0]);
                                if (retVal != GT_OK)
                                {
                                    return retVal;
                                }
                                for (i =0; i <PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                                {
                                    if (subnodesIndexes[i] == GT_TRUE)
                                    {
                                        if (subnodesSizes[i] == 0)
                                        {
                                            /* it is possible only if it become  Last level  with bucket became 3-3,2-2 */
                                            /* as a result of delettion deeper level */
                                            lastLevelBucketEmbLeaves = GT_TRUE;
                                            if ((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[0] != 0)
                                            {
                                                resizeGroup = GT_TRUE;
                                                /* the memory must be freed */
                                            }
                                            else
                                            {
                                                resizeGroup = GT_FALSE;
                                            }
                                            break;
                                        }
                                        if ((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[i]==0)
                                        {
                                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "\n ERROR in prvCpssDxChLpmSip6RamMngEntryDelete,(*currLpmPtr)->nextBucket->hwGroupOffsetHandle[i]==0 \n");
                                        }
                                        if (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[i]) != subnodesSizes[i])
                                        {
                                            resizeGroup = GT_TRUE;
                                            break;
                                        }
                                    }
                                }
                                if ( (resizeGroup == GT_FALSE) && (lastLevelBucketEmbLeaves == GT_FALSE) )
                                {
                                    /* There is no resize, but number of ranges was removed. The correspondent
                                       GON must be updated */
                                    rangeIsRemoved = GT_TRUE;
                                    doHwUpdate = GT_TRUE;
                                }
                            }

                            /* a change happend to the structure of the bucket
                               (removed ranges) clear the cash, and signal the
                               upper level */
                            (*currLpmPtr)->nextBucket->rangeCash = NULL;
                        }
                        updateUpperLevel = GT_TRUE;
                    }
                    else
                    {
                        /* check if we are in a case that the number of ranges is not changed and
                           their value<11( fit to compress), but the bucket type is regular.
                           this mean we that even that the num of ranges is not changes we should
                           recalculate the needed for delete. we migth need to move the regular
                           GON to a compress GON*/
                       if  (((*currLpmPtr)->nextBucket->numOfRanges <= MAX_NUMBER_OF_COMPRESSED_RANGES_CNS) &&
                             ((*currLpmPtr)->nextBucket->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E))
                       {
                           resizeGroup = GT_TRUE;
                           regularCompressedConversion = GT_TRUE;
                           parentUpdateParams->isBvCompConvertion = GT_TRUE;
                           updatedBucketType = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;

                           /* An overwrite merge was done (no change in the amount of ranges)
                              extract the appropriate prefix from the trie, and insert it in the same place.*/
                            /* The override can impact group of nodes size. The GON size must be
                               recalculated to understand if resize is needed */
                            cpssOsMemSet(subnodesIndexes,0,sizeof(subnodesIndexes));
                            cpssOsMemSet(subnodesSizes,0,sizeof(subnodesSizes));
                            retVal = lpmFalconGetGroupOfNodesSizes((*currLpmPtr)->nextBucket,
                                                               updatedBucketType,
                                                                *currAddrByte,currPrefix,
                                                               (*currLpmPtr)->nextBucket->numOfRanges,
                                                               GT_FALSE,GT_FALSE,NULL,
                                                               &subnodesIndexes[0],&subnodesSizes[0]);
                            if (retVal != GT_OK)
                            {
                                return retVal;
                            }

                            (*currLpmPtr)->nextBucket->rangeCash = NULL;
                            updateUpperLevel = GT_TRUE;
                       }

                        /* The current range was extended as a result of    */
                        /* previous insertions of larger prefixes.          */
                        newPrefixLength =
                            mask2PrefixLength((*pRange)->mask,
                                              PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                        newStartAddr = (GT_U8)(startAddr &
                            (0xFF << (PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS - newPrefixLength)));

                        /* Get the prefix that should replace the   */
                        /* deleted prefix.                          */
                        newNextHopPtr = getFromTrie((*currLpmPtr)->nextBucket,
                                                    newStartAddr,
                                                    newPrefixLength,
                                                    PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                        if (newNextHopPtr==NULL)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "\n ERROR got newNextHopPtr==NULL from getFromTrie \n");
                        }
                        /* in case of no HW update it's the same as in hot sync,
                           shadow update only */
                        insertMode = (updateHwAndMem == GT_TRUE)?
                                     PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E:
                                     PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E;

                        /* Insert the replacing prefix to the next level. this
                           insertion cannot resize any lower level bucket , it
                           simply overwrites using the replacing prefix the
                           ranges the old prefix dominated. thus there is no
                           need to check if the lower levels have been resized
                           for a current range HW update */
                        updateOld = GT_FALSE;
                        (*pRange)->updateRangeInHw = GT_TRUE;
                        PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(newNextHopPtr->routeEntryMethod,newNextHopPtrType);
                        retVal =
                            prvCpssDxChLpmSip6RamMngInsert((*pRange)->lowerLpmPtr.nextBucket,
                                                           currAddrByte + 1,/* actually not relevant*/
                                                           0,newNextHopPtr,
                                                           newNextHopPtrType,
                                                           PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E,
                                                           lpmEngineMemPtrPtr + 1,
                                                           &updateOld,
                                                           &((*pRange)->pointerType),
                                                           shadowPtr,insertMode,
                                                           ucMcType,
                                                           protocol,&tempParentUpdateParams);
                        if (retVal != GT_OK)
                        {
                            return retVal;
                        }
                        /* the GON on which pointed (*currLpmPtr)->nextBucket must be updated*/
                        doHwUpdate = GT_TRUE;
                        overwriteIsDone = GT_TRUE;
                        /* where override is needed */

                        startSubNodeIndex = startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
                        endSubNodeIndex = endAddr/NUMBER_OF_RANGES_IN_SUBNODE;
                        if ((*currLpmPtr)->nextBucket->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                        {
                            for (i = startSubNodeIndex; i <= endSubNodeIndex; i++)
                            {
                                subnodesIndexes[i] = GT_TRUE;
                            }
                        }
                        else
                        {
                            subnodesIndexes[0] = GT_TRUE;
                        }
                    }
                }
            }

            *pPrevRange = (*pRange);
            (*pRange) = (*pRange)->next;

        }while (((*pRange) != NULL) && (endAddr >= (*pRange)->startAddr));

        /* if the bucket need resizing always update the HW */
        if (resizeGroup == GT_TRUE)
            doHwUpdate = GT_TRUE;

        if ((isSrcTreeRootBucket == GT_TRUE) || (isDestTreeRootBucket == GT_TRUE))
        {
            if (regularCompressedConversion == GT_TRUE)
            {
                if(isDestTreeRootBucket == GT_TRUE)
                {
                    rootBucketUpdateState = LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E;
                }
                else
                {
                    rootBucketUpdateState = LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_SRC_RESIZE_E;
                }
            }
            else if ( (resizeGroup == GT_TRUE) || (rangeIsRemoved == GT_TRUE) )
            {
                /* some ranges were deleted from root, need update */
                /* here some problem may be seen: i need know resize or not*/
                rootBucketUpdateState = LPM_ROOT_BUCKET_UPDATE_HW_E;
            }
            else if ((resizeGroup == GT_FALSE) && (lastLevelBucketEmbLeaves == GT_TRUE))
            {
                rootBucketUpdateState = LPM_ROOT_BUCKET_UPDATE_HW_E;
            }
        }

        if (isDestTreeRootBucket &&
            (rootBucketFlag == PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_RAM_UPDATE_E))
        {
            bucketUpdateMode = LPM_BUCKET_UPDATE_ROOT_BUCKET_SHADOW_ONLY_E;
            indicateSiblingUpdate = (updateHwAndMem == GT_FALSE) ?
                                    GT_TRUE:GT_FALSE;
        }
        else if (doHwUpdate == GT_FALSE)
        {
            bucketUpdateMode = LPM_BUCKET_UPDATE_NONE_E;
            indicateSiblingUpdate =
            (updateHwAndMem == GT_FALSE)? GT_TRUE:GT_FALSE;
        }
        else if (updateHwAndMem == GT_FALSE)
        {
            bucketUpdateMode = LPM_BUCKET_UPDATE_SHADOW_ONLY_E;
        }
        else
        {
            bucketUpdateMode = LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E;
        }
        if (lastLevelBucketEmbLeaves == GT_TRUE)
        {
            bucketUpdateMode = LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E;
        }
        if ( (isDestTreeRootBucket == GT_FALSE) && (isSrcTreeRootBucket == GT_FALSE) )
        {
            parentUpdateParams->bucketPtr = PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(lpmPtrArray)[level - 1]->nextBucket;
            parentUpdateParams->pRange = PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pRangeArray)[level-1];
        }
        else
        {
            parentUpdateParams->bucketPtr = (*currLpmPtr)->nextBucket;
            parentUpdateParams->pRange = NULL;
        }
        parentUpdateParams->isRootNode = isDestTreeRootBucket;
        parentUpdateParams->shadowPtr = shadowPtr;
        parentUpdateParams->level = level;

        /* Write the prepared shadow bucket to the device's RAM. */
        retVal = updateMirrorGroupOfNodes((*currLpmPtr)->nextBucket,bucketUpdateMode,
                                          isDestTreeRootBucket,isSrcTreeRootBucket,rootBucketUpdateState,
                                          resizeGroup,rangeIsRemoved,overwriteIsDone,GT_FALSE,
                                          indicateSiblingUpdate,lpmEngineMemPtrPtr,
                                          *currBucketPtrType,shadowPtr,parentUpdateParams,
                                          &subnodesIndexes[0],&subnodesSizes[0]);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        if(updateHwAndMem == GT_TRUE)
        {
            if(level >= MAX_LPM_LEVELS_CNS) {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "\n ERROR in prvCpssDxChLpmSip6RamMngEntryDelete,(level >= MAX_LPM_LEVELS_CNS) \n");
            }
            PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pPrvRangeArray)[level]->updateRangeInHw = GT_FALSE;
        }
        /* reset isBvToCompConvertion for next level use */
        parentUpdateParams->isBvCompConvertion = GT_FALSE;
        parentUpdateParams->isRootNode = GT_FALSE;


        /* update partition block */
        /* if hwBucketOffsetHandle==0 it means that the
           bucket was deleted in the updateMirrorGroupOfNodes stage */
        if (((*currLpmPtr)->nextBucket)->hwBucketOffsetHandle!=0)
        {
            SET_DMM_BLOCK_PROTOCOL((*currLpmPtr)->nextBucket->hwBucketOffsetHandle, protocol);
            SET_DMM_BUCKET_SW_ADDRESS((*currLpmPtr)->nextBucket->hwBucketOffsetHandle, ((*currLpmPtr)->nextBucket));
        }
        for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
        {
            if (((*currLpmPtr)->nextBucket)->hwGroupOffsetHandle[j]!=0)
            {
                SET_DMM_BLOCK_PROTOCOL((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[j], protocol);
                SET_DMM_BUCKET_SW_ADDRESS((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[j], ((*currLpmPtr)->nextBucket));
            }
        }

        /* If this level is not a root bucket then delete it if its empty.*/
        /* in multicast may be use another type leaf_trigger */
        if (((currLpmPtr != PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(lpmPtrArray)) ||
             (rootBucketFlag == PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E)) &&
            ((**currBucketPtrType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
             (**currBucketPtrType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)))
        {
            if(level >= MAX_LPM_LEVELS_CNS) {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "\n ERROR in prvCpssDxChLpmSip6RamMngEntryDelete,(level >= MAX_LPM_LEVELS_CNS) \n");
            }
            parentUpdateParams->nodeWasCrearedInNextLevel[level] = GT_TRUE;
            newNextHopPtr =
                (*currLpmPtr)->nextBucket->rangeList->lowerLpmPtr.nextHopEntry;
            retVal = prvCpssDxChLpmRamMngBucketDelete((*currLpmPtr)->nextBucket,
                                                      PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, NULL);
            if (retVal != GT_OK)
            {
                return retVal;
            }
            (*currLpmPtr)->nextHopEntry = newNextHopPtr;
        }

        /* step back with the arrays */
        currLpmPtr--;
        pRange--;
        pPrevRange--;
        currBucketPtrType--;
        currAddrByte--;
        if (loopCounter > 0)
        {
            loopCounter --;
            lpmEngineMemPtrPtr--;
        }
        level--;
        currPrefix = BUCKET_PREFIX_AT_LEVEL_MAC(prefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,level);
    }

    return retVal;
}



/**
* @internal prvCpssDxChLpmSip6RamMngAllocatedAndBoundMemFree function
* @endinternal
*
* @brief  Free memory that was pre allocated or bound in
*         prvCpssDxChLpmSip6RamMngAllocAvailableMemCheck function.
*         used in case of error in the insertion.
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
)
{
    GT_U32                                  octetIndex;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      *headOfListToFreePtr; /* head of the list of memories we want to free */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      *tempElemToFreePtr;   /* temp pointer used for free operation */
    GT_U32                                  maxNumOfOctets=0;

    switch (protocolStack)
    {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_FCOE_PROTOCOL_CNS;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* free the allocated/bound RAM memory */
    for (octetIndex=0; octetIndex<maxNumOfOctets; octetIndex++)
    {
        /* the first element in the list is the father of the first new allocation,
           we need to free all elements after the father */
        headOfListToFreePtr = shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex];

        if (headOfListToFreePtr!=NULL)/* A new alloction was done for this octet */
        {
            if(headOfListToFreePtr->nextMemInfoPtr==NULL)/* this is an allocation of the root */
            {
                /* make sure this is the root - should always be true */
                if (headOfListToFreePtr==(&(shadowPtr->lpmMemInfoArray[protocolStack][octetIndex])))
                {
                    /* just reset the values */
                    headOfListToFreePtr->ramIndex=0;
                    headOfListToFreePtr->structsBase=0;
                    headOfListToFreePtr->structsMemPool=0;
                }
                else
                {
                    /* should never happen or it is a scenario we haven't thought about */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
            }
            else
            {
                /* this is an allocation of a new element in the linked list
                   need to free the elemen and remove it from the linked list (update the father pointers) */
                while (headOfListToFreePtr->nextMemInfoPtr != NULL)
                {
                    /* in case of a merge we can have a case that a octet got a potential bank and
                       allocated form it prior to the merge, then the merge partially happened and
                       ended with an error. in case of an error the first allocation should be freed,
                       but the bank should get a permanent association with the octet due to the merge operation */

                    tempElemToFreePtr = headOfListToFreePtr->nextMemInfoPtr;
                    if (((protocolStack==PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)&&
                            (shadowPtr->protocolCountersPerBlockArr[tempElemToFreePtr->ramIndex].sumOfIpv4Counters==0))||
                        ((protocolStack==PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)&&
                            (shadowPtr->protocolCountersPerBlockArr[tempElemToFreePtr->ramIndex].sumOfIpv6Counters==0))||
                        ((protocolStack==PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)&&
                            (shadowPtr->protocolCountersPerBlockArr[tempElemToFreePtr->ramIndex].sumOfFcoeCounters==0)))
                    {
                        headOfListToFreePtr->nextMemInfoPtr = headOfListToFreePtr->nextMemInfoPtr->nextMemInfoPtr;
                        cpssOsFree(tempElemToFreePtr);
                    }
                    else
                    {
                        /* the memory was passed by merge and we need to permanent update the assosiation to the bank
                           in the tempLpmRamOctetsToBlockMappingUsedForReconstractPtr used in case of a fail */
                         cpssOsMemCpy(&(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr[tempElemToFreePtr->ramIndex]),
                                      &(shadowPtr->lpmRamOctetsToBlockMappingPtr[tempElemToFreePtr->ramIndex]),
                                      sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC));

                         /* Set next element as head (will not be freed) */
                         headOfListToFreePtr = headOfListToFreePtr->nextMemInfoPtr;
                    }
                }
            }
        }
    }

    cpssOsMemSet(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr,0,sizeof(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr));

    return GT_OK;
}

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
    IN      GT_U32                                        memoryBlockTakenArr[]
)
{
    GT_STATUS retVal=GT_OK;
    GT_U32 blockStart=0;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *potentialNewBlockPtr=NULL;

    /* avoid warnings */
    newStructsMemPool=newStructsMemPool;

    /* go over all blocks until the memory of the octet can be allocated in the block found */

    while (blockStart < shadowPtr->numOfLpmMemories)
    {
        /* find a new free block and bind it to the octet and protocol */
        retVal = prvCpssDxChLpmRamMngFindValidMemoryBlockForProtocol(shadowPtr,
                                                                     protocol,
                                                                     octetIndex,
                                                                     blockStart,
                                                                     memoryBlockTakenArr,
                                                                     (DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * neededMemoryBlocksSizes),
                                                                     newFreeBlockIndexPtr);
        if (retVal != GT_OK)
        {
            break;
        }

        /* allocate the memory needed from the new structsMemPool bound */
        *neededMemoryBlocksPtr =
            prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[*newFreeBlockIndexPtr],
                               DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * (neededMemoryBlocksSizes),
                               DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);

        /* neededMemoryBlocksPtr is not 0 nor 0xFFFFFFFF --> we did succeed in allocating the
        needed memory from the valid block we found  */
        if ((*neededMemoryBlocksPtr != DMM_BLOCK_NOT_FOUND)&&
            (*neededMemoryBlocksPtr != DMM_MALLOC_FAIL))
        {
            if(*justBindAndDontAllocateFreePoolMemPtr==GT_TRUE)
            {
                lastMemInfoPtr->ramIndex = *newFreeBlockIndexPtr;
                lastMemInfoPtr->structsBase=0;
                /* bind the new block */
                lastMemInfoPtr->structsMemPool=shadowPtr->lpmRamStructsMemPoolPtr[*newFreeBlockIndexPtr];
                newStructsMemPool = shadowPtr->lpmRamStructsMemPoolPtr[*newFreeBlockIndexPtr];
                lastMemInfoPtr->nextMemInfoPtr=NULL;
                /* first element in linked list of the blocks that
                   need to be freed in case of an error is the root */
                firstMemInfoInListToFreePtr = lastMemInfoPtr;

                *justBindAndDontAllocateFreePoolMemPtr=GT_FALSE;
            }
            else
            {
                /* allocate a new elemenet block to the list */
                potentialNewBlockPtr =  (PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC));
                if (potentialNewBlockPtr == NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }
                potentialNewBlockPtr->ramIndex = *newFreeBlockIndexPtr;
                potentialNewBlockPtr->structsBase = 0;
                potentialNewBlockPtr->structsMemPool = 0;
                potentialNewBlockPtr->nextMemInfoPtr = NULL;

                /* bind the new block */
                potentialNewBlockPtr->structsMemPool=shadowPtr->lpmRamStructsMemPoolPtr[*newFreeBlockIndexPtr];
                newStructsMemPool = shadowPtr->lpmRamStructsMemPoolPtr[*newFreeBlockIndexPtr];
                lastMemInfoPtr->nextMemInfoPtr = potentialNewBlockPtr;

                 /* first element in linked list of the blocks that
                   need to be freed in case of an error is the father
                   of the new block added to the list
                  (father of potentialNewBlockPtr is lastMemInfoPtr) */
                firstMemInfoInListToFreePtr = lastMemInfoPtr;
            }

            /* mark the block as used */
            shadowPtr->lpmRamOctetsToBlockMappingPtr[*newFreeBlockIndexPtr].isBlockUsed=GT_TRUE;
            /* set the block as taken */
            memoryBlockTakenArr[*newFreeBlockIndexPtr]++;
            /*  set the block to be used by the specific octet and protocol*/
            PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_SET_MAC(shadowPtr,protocol,octetIndex,*newFreeBlockIndexPtr);

             /*  set pending flag for future need */
            shadowPtr->pendingBlockToUpdateArr[*newFreeBlockIndexPtr].updateInc=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[*newFreeBlockIndexPtr].numOfIncUpdates +=
            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(*neededMemoryBlocksPtr);

            /* We only save the first element allocated or bound per octet */
            if ((allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex]==NULL)||
                (allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex]->structsMemPool==0))
            {
                /* keep the head of the list we need to free in case of an error -
                first element is the father of the first element that should be freed */
                allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex] = firstMemInfoInListToFreePtr;
            }
            break;
        }
        /* if the neededMemoryBlocksPtr is 0 or 0xFFFFFFFF --> then we did not succeed in allocating the
         needed memory from the valid block we found, look for another valid block or
         return an error incase of CPU memory allocation fail  */
        else
        {
            if (*neededMemoryBlocksPtr != DMM_MALLOC_FAIL)
            {
                blockStart = *newFreeBlockIndexPtr + 1;
            }
            else
            {
                retVal = GT_OUT_OF_CPU_MEM;
                break;
            }
        }
    }
    /* if we get here with GT_OK but we still did not find a free space to allocate,
       this can happen when we finish to go over all blocks in a sharing mode */
    if(*neededMemoryBlocksPtr == DMM_BLOCK_NOT_FOUND)
    {
        retVal = GT_OUT_OF_PP_MEM;
    }
    else
    {
        if(*neededMemoryBlocksPtr == DMM_MALLOC_FAIL)
        {
            retVal = GT_OUT_OF_CPU_MEM;
        }
    }

    return retVal;
}

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
)
{
    PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC  neededMemoryBlocksSizes[MAX_LPM_LEVELS_CNS];  /* Holds memory allocation  */
    GT_UINTPTR  memoryPoolList[MAX_LPM_LEVELS_CNS];            /* needs.                   */
    GT_U32      neededMemoryBlocksOctetsIndexs[MAX_LPM_LEVELS_CNS];
    PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC  *neededMemoryBlocksPtr;
    GT_U32      i,j,k;
    GT_STATUS   retVal = GT_OK,retVal1=GT_OK,retVal2=GT_OK;             /* Function return value.   */
    GT_U32      startIdx;
    PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC *neededMemoryBlocksSizesPtr;
    GT_U32      *neededMemoryBlocksOctetsIndexsPtr;
    GT_UINTPTR  *memoryPoolListPtr;
    GT_U32      newFreeBlockIndex; /* index of a new free block */
    GT_UINTPTR  tempNewStructsMemPool=0;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempNextMemInfoPtr;/* use for going over the list of blocks per octet */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  **allNewNextMemInfoAllocatedPerOctetArrayPtr;/*(pointer to) an array that holds for each octet
                                                                                      the allocted new element that need to be freed.
                                                                                      Size of the array is 16 for case of IPV6 */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *firstMemInfoInListToFreePtr=NULL;/* use for going over the list of blocks per octet */

    GT_U32      octetIndex; /* the octet we are working on */
    GT_BOOL     justBindAndDontAllocateFreePoolMem = GT_FALSE;
    GT_U32      blockIndex=0xFFFFFFFF; /* calculated according to the memory offset devided by block size including gap */

    GT_U32      neededMemoryListLenTreatedSuccessfully=0;
    GT_U32      sizeOfOptionalUsedAndFreeBlock;

    GT_BOOL     firstSwapUsed  = GT_FALSE;
    GT_BOOL     secondSwapUsed = GT_FALSE;
    GT_BOOL     thirdSwapUsed  = GT_FALSE;

    GT_U32 moved=0;/* to be used in order to find out if a memory we are working
                      on was moved to a different location due to defrag */
    GT_U32 tempGlobalMemoryBlockTakenArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    /* in bulk style lpm insertion we should work with the same globalMemoryBlockTakenArr untill the end of the bulk operation */
    if(insertMode!=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E)
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));

    cpssOsMemSet(neededMemoryBlocksSizes, 0, sizeof(neededMemoryBlocksSizes));
    neededMemoryBlocksPtr  = shadowPtr->neededMemoryBlocksInfo;

    allNewNextMemInfoAllocatedPerOctetArrayPtr = shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr;

    /* remember the current list len, to go over only the added ones*/
    startIdx = shadowPtr->neededMemoryListLen;
    neededMemoryListLenTreatedSuccessfully = startIdx;

    neededMemoryBlocksSizesPtr = &neededMemoryBlocksSizes[MAX_LPM_LEVELS_CNS-1];
    neededMemoryBlocksOctetsIndexsPtr = &neededMemoryBlocksOctetsIndexs[MAX_LPM_LEVELS_CNS-1];
    memoryPoolListPtr = &memoryPoolList[MAX_LPM_LEVELS_CNS-1];

    /* Get needed memory for LPM search insertion.  */
    retVal = lpmFalconCalcNeededMemory(bucketPtr,destArr,prefix,
                                       rootBucketFlag,lpmEngineMemPtrPtr,
                                       insertMode,defragmentationEnable,
                                       &neededMemoryBlocksSizesPtr,
                                       &memoryPoolListPtr,
                                       &shadowPtr->neededMemoryListLen,
                                       &neededMemoryBlocksOctetsIndexsPtr,
                                       parentUpdateParams,
                                       shadowPtr->globalMemoryBlockTakenArr);
    if(retVal != GT_OK)
    {
        return retVal;
    }

    /* keep values in case reconstruct is needed */
    cpssOsMemCpy(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,shadowPtr->lpmRamOctetsToBlockMappingPtr,
                 sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

   /* reset neededMemoryBlocks before new allocations */
   /* cpssOsMemSet(neededMemoryBlocks+startIdx, (int)DMM_BLOCK_NOT_FOUND, (sizeof(GT_UINTPTR) * (shadowPtr->neededMemoryListLen-startIdx)));*/
    for (i = startIdx; i <shadowPtr->neededMemoryListLen; i++ )
    {
        for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
        {
            neededMemoryBlocksPtr[i].neededMemoryBlocks[j] = DMM_BLOCK_NOT_FOUND;
            neededMemoryBlocksPtr[i].neededMemoryBlocksSizes[j] = 0;

            neededMemoryBlocksPtr[i].neededMemoryBlocksSwapUsedForAdd[j]=GT_FALSE;
            neededMemoryBlocksPtr[i].neededMemoryBlocksSwapIndexUsedForAdd[j]=0;/* no use*/
            neededMemoryBlocksPtr[i].neededMemoryBlocksOldHandleAddr[j]=0;
            neededMemoryBlocksPtr[i].neededMemoryBlocksOldHandleGonIndex[j]=0;
            neededMemoryBlocksPtr[i].shrinkOperationUsefulForDefragArr[j]=GT_FALSE;
            neededMemoryBlocksPtr[i].bankIndexForShrinkArr[j]=0;
            neededMemoryBlocksPtr[i].swapUsedForShrinkArr[j]=GT_FALSE;
            neededMemoryBlocksPtr[i].mergeOperationUsefulForDefragArr[j] = GT_FALSE;
            neededMemoryBlocksPtr[i].octetIndexForMergeArr[j] = 0;
            neededMemoryBlocksPtr[i].bankIndexForMergeArr[j] = 0;
        }
        neededMemoryBlocksPtr[i].shrinkOperationUsefulForDefragGlobalFlag=GT_FALSE;
        neededMemoryBlocksPtr[i].mergeOperationUsefulForDefragGlobalFlag = GT_FALSE;
        neededMemoryBlocksPtr[i].regularNode = GT_FALSE;
        neededMemoryBlocksPtr[i].octetId = 0xff;
        neededMemoryBlocksPtr[i].ipAllocType = PRV_CPSS_DXCH_LPM_ALLOC_UC_TYPE_E;
    }
    /* Try to allocate all needed memory.               */
    for(i = startIdx; i < shadowPtr->neededMemoryListLen; i++)
    {
        memoryPoolListPtr++;
        neededMemoryBlocksSizesPtr++;
        neededMemoryBlocksOctetsIndexsPtr++;
        octetIndex = (*neededMemoryBlocksOctetsIndexsPtr);
        if (*memoryPoolListPtr!=0)
        {
            for (k = 0; k < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; k++)
            {
                if (neededMemoryBlocksSizesPtr->neededMemoryBlocksSizes[k] > 0)
                {
                    neededMemoryBlocksPtr[i].neededMemoryBlocks[k] =
                    prvCpssDmmAllocate(*memoryPoolListPtr,
                                       DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * (neededMemoryBlocksSizesPtr->neededMemoryBlocksSizes[k]),
                                       DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);
                }
                if (neededMemoryBlocksSizesPtr->regularNode == GT_FALSE)
                {
                    break;
                }
            }
        }
        else
        {
            /* no memory pool is bound to the root of the octet list,
               in this case no need to allocate a memory pool struct just
               to bind it to a free pool */
            justBindAndDontAllocateFreePoolMem = GT_TRUE;

        }
        for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
        {
            shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksSizes[j] =
            neededMemoryBlocksSizesPtr->neededMemoryBlocksSizes[j];

            shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksOldHandleAddr[j] =
            neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleAddr[j];

            shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksOldHandleGonIndex[j] =
            neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleGonIndex[j];

            shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksOldShadowBucket[j] =
            neededMemoryBlocksSizesPtr->neededMemoryBlocksOldShadowBucket[j];
        }
        shadowPtr->neededMemoryBlocksInfo[i].octetId = neededMemoryBlocksSizesPtr->octetId;
        shadowPtr->neededMemoryBlocksInfo[i].regularNode = neededMemoryBlocksSizesPtr->regularNode;
        shadowPtr->neededMemoryBlocksInfo[i].ipAllocType = ipAllocType;

        for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
        {
            /* in falcon we are checking for all group of nodes */
            if (neededMemoryBlocksSizesPtr->neededMemoryBlocksSizes[j] == 0)
            {
                continue;
            }
            if(neededMemoryBlocksPtr[i].neededMemoryBlocks[j] == DMM_BLOCK_NOT_FOUND)
            {
                /* check if the octet is bound to more blocks,
                  if yes then try to allocte the memory again */

                 /* shadowPtr->neededMemoryListLen is the number of blocks
                    (just the first in the list of each octet) in neededMemoryBlocks
                    --> meaninng number of octets we want to add in the currect prefix */

                /* the search is from the last octet to the first one since
                   memoryPoolListPtr was initialized backwards in lpmCalcNeededMemory */
                tempNextMemInfoPtr = lpmEngineMemPtrPtr[octetIndex];

                while((tempNextMemInfoPtr->nextMemInfoPtr!= NULL)&&
                      (neededMemoryBlocksPtr[i].neededMemoryBlocks[j]==DMM_BLOCK_NOT_FOUND))
                {
                    neededMemoryBlocksPtr[i].neededMemoryBlocks[j] =
                        prvCpssDmmAllocate(tempNextMemInfoPtr->nextMemInfoPtr->structsMemPool,
                                           DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * (neededMemoryBlocksSizesPtr->neededMemoryBlocksSizes[j]),
                                           DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);
                    tempNextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                }

                /* could not find an empty space in the current bound blocks - continue looking */
                if ((tempNextMemInfoPtr->nextMemInfoPtr==NULL)&&(neededMemoryBlocksPtr[i].neededMemoryBlocks[j]==DMM_BLOCK_NOT_FOUND))
                {

                    if(neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleAddr[j]!=0)
                    {
                        moved = GET_DMM_BLOCK_WAS_MOVED(neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleAddr[j]);
                        /* if the old was moved then we need to assign the new handle given instead of the old one
                           this is updated automatically in neededMemoryBlocksOldShadowBucketPtr */
                        if (moved!=0)
                        {
                            /* reset moved bit */
                            SET_DMM_BLOCK_WAS_MOVED(neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleAddr[j],0);

                            /* set the new location of the old handle */
                            if (neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleGonIndex[j]==0xFFFE)
                            {
                                 neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleAddr[j] =
                                     (neededMemoryBlocksSizesPtr->neededMemoryBlocksOldShadowBucket[j])->hwGroupOffsetHandle[0];
                            }
                            else
                            {
                                if (neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleGonIndex[j]==0xFFFF)
                                {
                                    neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleAddr[j] =
                                        (neededMemoryBlocksSizesPtr->neededMemoryBlocksOldShadowBucket[j])->hwBucketOffsetHandle;
                                }
                                else
                                {
                                    neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleAddr[j] =
                                        (neededMemoryBlocksSizesPtr->neededMemoryBlocksOldShadowBucket[j])->hwGroupOffsetHandle[neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleGonIndex[j]];
                                }
                            }

                            /* now double check if swap area should be used in case of new memory allocation */
                            sizeOfOptionalUsedAndFreeBlock =prvCpssDmmCheckResizeAvailableWithSameMemory(neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleAddr[j],
                                                                                                         ((neededMemoryBlocksSizesPtr->neededMemoryBlocksSizes[j])*
                                                                                                          PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS));
                            if ((sizeOfOptionalUsedAndFreeBlock>0)&&(sizeOfOptionalUsedAndFreeBlock!=DMM_BLOCK_NOT_FOUND))
                            {
                                 neededMemoryBlocksSizesPtr->neededMemoryBlocksSwapUsedForAdd[j] = GT_TRUE;
                            }
                            else
                            {
                                 neededMemoryBlocksSizesPtr->neededMemoryBlocksSwapUsedForAdd[j] = GT_FALSE;
                            }
                        }
                    }

                    /* first we try to allocate the space using the swap area
                       when neededMemoryBlocksSizesPtr.neededMemoryBlocksSwapUsedForAdd[j] is GT_TRUE it means that
                       swap area migth be usefull, it does not mean we will use it for sure */
                    if ((defragmentationEnable==GT_TRUE)&&(neededMemoryBlocksSizesPtr->neededMemoryBlocksSwapUsedForAdd[j]==GT_TRUE)&&
                        (neededMemoryBlocksSizesPtr->neededMemoryBlocksOldHandleAddr[j]!=0))
                    {
                        prvCpssDxChLpmSip6RamMngShrinkCheckIfSwapAreaIsUseful(lpmEngineMemPtrPtr, shadowPtr,
                                                                              neededMemoryBlocksSizesPtr, octetIndex, i, j,
                                                                              &firstSwapUsed, &secondSwapUsed,
                                                                              &thirdSwapUsed);
                    }

                    /* if using swap area do not help then find an available new free block that
                       could be bound to the octet */
                    if (neededMemoryBlocksPtr[i].neededMemoryBlocks[j]==DMM_BLOCK_NOT_FOUND)
                    {
                        retVal = findAndBindValidMemoryBlock(shadowPtr, protocol, octetIndex,
                                                             &neededMemoryBlocksPtr[i].neededMemoryBlocks[j],
                                                             neededMemoryBlocksSizesPtr->neededMemoryBlocksSizes[j],
                                                             tempNextMemInfoPtr,
                                                             firstMemInfoInListToFreePtr,
                                                             &justBindAndDontAllocateFreePoolMem,
                                                             allNewNextMemInfoAllocatedPerOctetArrayPtr,
                                                             tempNewStructsMemPool,
                                                             &newFreeBlockIndex,
                                                             shadowPtr->globalMemoryBlockTakenArr);
                        if (retVal != GT_OK)
                        {
                            if ((defragmentationEnable==GT_TRUE)&&
                                (shadowPtr->lpmRamBlocksAllocationMethod!=PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E))
                            {
                               /* if we did not find a free block to bind we
                               should continue and try to shrink the blocks,
                               this will happen only after going over all the GONs
                               Sharing mode do not support shrink*/
                               retVal = GT_OK;
                               continue;
                            }
                            else
                            {
                                retVal = GT_OUT_OF_PP_MEM;
                                break;
                            }
                        }
                    }
                    /* keep temp for recostruct incase of a fail */
                    cpssOsMemCpy(tempGlobalMemoryBlockTakenArr,shadowPtr->globalMemoryBlockTakenArr,sizeof(shadowPtr->globalMemoryBlockTakenArr));
                }
                else
                {
                    /* keep temp for recostruct incase of a fail */
                    cpssOsMemCpy(tempGlobalMemoryBlockTakenArr,shadowPtr->globalMemoryBlockTakenArr,sizeof(shadowPtr->globalMemoryBlockTakenArr));

                    /* we had a CPU Memory allocation error */
                    if(neededMemoryBlocksPtr[i].neededMemoryBlocks[j] == DMM_MALLOC_FAIL)
                    {
                        retVal = GT_OUT_OF_CPU_MEM;
                        break;
                    }
                    /* if we are using swap area then no allocation was done */
                    if (neededMemoryBlocksPtr[i].neededMemoryBlocksSwapUsedForAdd[j] == GT_FALSE)
                    {
                        /* mark the block as used */
                        /* set pending flag for future need */
                        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocksPtr[i].neededMemoryBlocks[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                        retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                        if (retVal != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                        }
                        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
                        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(neededMemoryBlocksPtr[i].neededMemoryBlocks[j]);
                        /* in case of bulk operation */
                        if(insertMode==PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E)
                        {
                            /* in case the globalMemoryBlockTakenArr was not set as taken, we should set it.
                               this can happen in bulk operation when we already have a block assign to the
                               octet and we allocate new space, but the lines alloctaed will not be updated
                               untill the end of the bulk operation, and we must set this block as used,
                               so it will not be taken by other octet */
                            if (shadowPtr->globalMemoryBlockTakenArr[blockIndex]==0)
                            {
                                shadowPtr->globalMemoryBlockTakenArr[blockIndex]++;
                            }
                        }
                    }
                }
            }
            else
            {
                /* we had a CPU Memory allocation error */
                if(neededMemoryBlocksPtr[i].neededMemoryBlocks[j] == DMM_MALLOC_FAIL)
                {
                    retVal = GT_OUT_OF_CPU_MEM;
                    break;
                }
                else
                {
                    /* keep temp for recostruct incase of a fail */
                    cpssOsMemCpy(tempGlobalMemoryBlockTakenArr,shadowPtr->globalMemoryBlockTakenArr,sizeof(shadowPtr->globalMemoryBlockTakenArr));

                    /* if we are using swap area then no allocation was done */
                    if (neededMemoryBlocksPtr[i].neededMemoryBlocksSwapUsedForAdd[j] == GT_FALSE)
                    {
                        /*We succeed in allocating the memory*/
                        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocksPtr[i].neededMemoryBlocks[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                        retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                        if (retVal1 != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                        }
                        /* set pending flag for future need */
                        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;

                        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                                PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(neededMemoryBlocksPtr[i].neededMemoryBlocks[j]);
                        /* in case of bulk operation */
                        if(insertMode==PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E)
                        {
                            /* in case the globalMemoryBlockTakenArr was not set as taken, we should set it.
                               this can happen in bulk operation when we already have a block assign to the
                               octet and we allocate new space, but the lines alloctaed will not be updated
                               untill the end of the bulk operation, and we must set this block as used,
                               so it will not be taken by other octet */
                            if (shadowPtr->globalMemoryBlockTakenArr[blockIndex]==0)
                            {
                                shadowPtr->globalMemoryBlockTakenArr[blockIndex]++;
                            }
                        }
                    }
                }
            }
        }
        if (retVal != GT_OK)
        {
            break;
        }

        if(neededMemoryBlocksPtr[i].regularNode==GT_TRUE)
        {
            tempNewStructsMemPool = 0;
            retVal2 = GT_OK;
            for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
            {
                /* if we have a regular GON then gons can share the same block,
                   this can lead to taking space assuming to be used for swap operation
                   need to double check that swap still can be used */
                if ((defragmentationEnable==GT_TRUE)&&(neededMemoryBlocksPtr[i].neededMemoryBlocksSwapUsedForAdd[j]==GT_TRUE)&&
                    (neededMemoryBlocksPtr[i].neededMemoryBlocksOldHandleAddr[j]!=0))
                {
                    retVal = prvCpssDxChLpmSip6RamMngShrinkAllocRegularGonCheck(shadowPtr, lpmEngineMemPtrPtr, protocol,
                                                                    octetIndex, neededMemoryBlocksSizesPtr,
                                                                    tempNewStructsMemPool, firstMemInfoInListToFreePtr,
                                                                    &justBindAndDontAllocateFreePoolMem,i,j,
                                                                    &firstSwapUsed, &secondSwapUsed,
                                                                    &thirdSwapUsed);
                    if (retVal != GT_OK)
                    {
                        if (retVal == GT_OUT_OF_PP_MEM)
                        {
                            retVal2 = GT_OUT_OF_PP_MEM;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
            if (retVal2 != GT_OK)
            {
                for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                {
                    /* Swap failed. Free allocated memory*/
                    if ((defragmentationEnable==GT_TRUE) &&
                        (neededMemoryBlocksPtr[i].neededMemoryBlocksSwapUsedForAdd[j]==GT_FALSE) &&
                        (neededMemoryBlocksPtr[i].neededMemoryBlocksOldHandleAddr[j] == 0) &&
                        (neededMemoryBlocksPtr[i].neededMemoryBlocks[j] != 0) &&
                        (neededMemoryBlocksPtr[i].neededMemoryBlocks[j] != DMM_BLOCK_NOT_FOUND))
                    {
                        prvCpssDmmFree(neededMemoryBlocksPtr[i].neededMemoryBlocks[j]);
                    }
                }
                return retVal2;
            }
        }
        if (retVal!=GT_OK)
        {
            break;
        }
        else
        {
            justBindAndDontAllocateFreePoolMem=GT_FALSE;
            neededMemoryListLenTreatedSuccessfully++;
        }
    }

    /* in case we did not succeed in allocated all memory needed, we try to shrink the blocks */
    if((retVal==GT_OK)&&(defragmentationEnable==GT_TRUE)&&
       (shadowPtr->lpmRamBlocksAllocationMethod!=PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E))
    {
        retVal = prvCpssDxChLpmSip6RamMngDefragCheckAndApply(shadowPtr, lpmEngineMemPtrPtr, protocol, startIdx,
                                                             parentUpdateParams, &neededMemoryListLenTreatedSuccessfully,
                                                             &firstSwapUsed, &secondSwapUsed, &thirdSwapUsed);
    }

    if(((neededMemoryListLenTreatedSuccessfully != shadowPtr->neededMemoryListLen))||
        (retVal!=GT_OK))
    {
#if 0
        /* if we get to this point due to CPU error then leave the
           retVal we got in previous stage else return error due to PP */
        if (retVal!=GT_OUT_OF_CPU_MEM)
        {
            retVal = GT_OUT_OF_PP_MEM;
        }
#endif
        i = 0;
        /* Allocation failed, free all allocated memory. */
        while(i < shadowPtr->neededMemoryListLen)
        {
            for (j= 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
            {
                if ((neededMemoryBlocksPtr[i].neededMemoryBlocks[j] != DMM_BLOCK_NOT_FOUND) &&
                    (neededMemoryBlocksPtr[i].neededMemoryBlocks[j] != DMM_MALLOC_FAIL)&&
                    (neededMemoryBlocksPtr[i].neededMemoryBlocksSizes[j] != 0))
                {
                    if(neededMemoryBlocksPtr[i].neededMemoryBlocksSwapUsedForAdd[j]==GT_FALSE)
                    {
                        /*We fail in allocating the memory*/
                        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocksPtr[i].neededMemoryBlocks[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                        retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                        if (retVal1 != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                        }
                        /* set pending flag for future need */
                        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_FALSE;
                        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates=0;
                        /* in case of bulk operation */
                        if(insertMode==PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E)
                        {
                            /* reset globalMemoryBlockTakenArr to its old value */
                            cpssOsMemCpy(shadowPtr->globalMemoryBlockTakenArr,tempGlobalMemoryBlockTakenArr,sizeof(shadowPtr->globalMemoryBlockTakenArr));
                        }
                        prvCpssDmmFree(neededMemoryBlocksPtr[i].neededMemoryBlocks[j]);
                    }
                }
                if(neededMemoryBlocksPtr[i].regularNode == GT_FALSE)
                {
                    break;
                }
            }
            i++;
        }

        shadowPtr->neededMemoryListLen = 0;

        /* free the allocated/bound RAM memory */
        retVal2 = prvCpssDxChLpmSip6RamMngAllocatedAndBoundMemFree(shadowPtr,protocol);
        if (retVal2!=GT_OK)
        {
            return retVal2;
        }
        /* in case of fail we will need to reconstruct to the status we had after the call to prvCpssDxChLpmRamMngFindValidMemoryBlockForProtocol */
        cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
               sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

        return retVal;
    }
    return retVal;
}





/**
* @internal prvCpssDxChLpmSip6RamMngHandleRootNodeAllocation function
* @endinternal
*
* @brief allocate lpm hw memory for root node
*
* @param[in] tempNextMemInfoPtr          - pointer used for going over the list of blocks per octet
* @param[in] memSize                     - memory size in lines
* @param[in] minAllocationSizeInBytes    - minimum allocation size in bytes
* @param[in] protocol                    - the protocol
* @param[in] octetIndex                  - can be 0 for the root regular bit vector or
*                                          1 for the GON of the root
* @param[in] shadowPtr                   - pointer to LPM shadow
* @param[out] hwBucketOffsetHandlePtr     - pointer to hw bucket offset handle.
*
* @retval   GT_OK                    - If there is enough memory for the insertion.
* @retval   GT_OUT_OF_PP_MEM         - otherwise.
* @retval   GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChLpmSip6RamMngHandleRootNodeAllocation
(
    IN  PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC          *tempNextMemInfoPtr,
    IN  GT_U32                                      memSize,
    IN  GT_U32                                      minAllocationSizeInBytes,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT        protocol,
    IN  GT_U32                                      octetIndex,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    OUT GT_UINTPTR                                  *hwBucketOffsetHandlePtr
)
{
    GT_UINTPTR hwBucketOffsetHandle = DMM_BLOCK_NOT_FOUND;
    GT_U32 blockStart = 0;
    GT_U32 newFreeBlockIndex; /* index of a new free block */
    GT_STATUS retVal = GT_OK;
    GT_STATUS retVal1 = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *potentialNewBlockPtr=NULL;
    GT_U32      blockIndex=0; /* calculated according to the memory offset devided by block size including gap */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  **allNewNextMemInfoAllocatedPerOctetArrayPtr;/*(pointer to) an array that holds for each octet
                                                                                        the allocted new element that need to be freed.
                                                                                        Size of the array is 16 for case of IPV6 */
    GT_U32     memoryBlockTakenArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    cpssOsMemSet(memoryBlockTakenArr, 0, sizeof(memoryBlockTakenArr));
    allNewNextMemInfoAllocatedPerOctetArrayPtr = shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr;

    hwBucketOffsetHandle = prvCpssDmmAllocate(tempNextMemInfoPtr->structsMemPool,
                                              minAllocationSizeInBytes * memSize,
                                              minAllocationSizeInBytes);
    if ( (hwBucketOffsetHandle != DMM_MALLOC_FAIL) && (hwBucketOffsetHandle != DMM_BLOCK_NOT_FOUND) )
    {
        *hwBucketOffsetHandlePtr = hwBucketOffsetHandle;

        /*  set pending flag for future need */
        shadowPtr->pendingBlockToUpdateArr[tempNextMemInfoPtr->ramIndex].updateInc=GT_TRUE;
        shadowPtr->pendingBlockToUpdateArr[tempNextMemInfoPtr->ramIndex].numOfIncUpdates += memSize;

        return GT_OK;
    }
    if (hwBucketOffsetHandle == DMM_MALLOC_FAIL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    /* go over all blocks bound to the octet and try to allocate */
    while((tempNextMemInfoPtr->nextMemInfoPtr != NULL) && (hwBucketOffsetHandle == DMM_BLOCK_NOT_FOUND))
    {
        hwBucketOffsetHandle = prvCpssDmmAllocate(tempNextMemInfoPtr->nextMemInfoPtr->structsMemPool,
                                                       minAllocationSizeInBytes * memSize,
                                                       minAllocationSizeInBytes);
        blockIndex = tempNextMemInfoPtr->nextMemInfoPtr->ramIndex;
        tempNextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
    }

    /* could not find an empty space in the current bound blocks - continue looking */
    if ((tempNextMemInfoPtr->nextMemInfoPtr == NULL) && (hwBucketOffsetHandle == DMM_BLOCK_NOT_FOUND))
    {
        /* go over all blocks until the memory of the octet can be allocated in the block found */
        while (blockStart < shadowPtr->numOfLpmMemories)
        {
            /* find a new free block and bind it to the octet and protocol */
            retVal = prvCpssDxChLpmRamMngFindValidMemoryBlockForProtocol(shadowPtr,
                                                                         protocol,
                                                                         octetIndex,
                                                                         blockStart,
                                                                         memoryBlockTakenArr,
                                                                         (minAllocationSizeInBytes*memSize),
                                                                         &newFreeBlockIndex);
            if (retVal != GT_OK)
            {
                break;
            }

            /* allocate the memory needed from the new structsMemPool bound */
             hwBucketOffsetHandle = prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[newFreeBlockIndex],
                                   minAllocationSizeInBytes * memSize,
                                   minAllocationSizeInBytes);

            /* neededMemoryBlocks is not 0 nor 0xFFFFFFFF --> we did succeed in allocating the
               needed memory from the valid block we found */
            if ((hwBucketOffsetHandle != DMM_BLOCK_NOT_FOUND) && (hwBucketOffsetHandle != DMM_MALLOC_FAIL))
            {
                /* allocate a new elemenet block to the list */
                potentialNewBlockPtr =  (PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC));
                if (potentialNewBlockPtr == NULL)
                {
                    /*goto delete_ranges*/
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }
                potentialNewBlockPtr->ramIndex = newFreeBlockIndex;
                potentialNewBlockPtr->structsBase = 0;
                potentialNewBlockPtr->structsMemPool = 0;
                potentialNewBlockPtr->nextMemInfoPtr = NULL;

                /* bind the new block */
                potentialNewBlockPtr->structsMemPool=shadowPtr->lpmRamStructsMemPoolPtr[newFreeBlockIndex];
                tempNextMemInfoPtr->nextMemInfoPtr = potentialNewBlockPtr;

                /* mark the block as used */
                shadowPtr->lpmRamOctetsToBlockMappingPtr[newFreeBlockIndex].isBlockUsed = GT_TRUE;
                /* set the block as taken */
                memoryBlockTakenArr[newFreeBlockIndex]++;
                /*  set the block to be used by the specific octet and protocol*/
                PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_SET_MAC(shadowPtr,protocol,octetIndex,newFreeBlockIndex);

                /*  set pending flag for future need */
                shadowPtr->pendingBlockToUpdateArr[newFreeBlockIndex].updateInc=GT_TRUE;
                shadowPtr->pendingBlockToUpdateArr[newFreeBlockIndex].numOfIncUpdates +=
                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(hwBucketOffsetHandle);

                /* We only save the first element allocated or bound per octet
                   we are dealing now with the root so we will get here twice
                   first time for the regular but vector located at bank0 (octet 0)
                   second time for the GONs of this bit vector located at bank 1 (octet 1)
                   for SIP 6 we look at the octets like we have 5 octets for ipv4 and 17 octets for ipv6 */
                if ((allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex]==NULL)||
                    (allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex]->structsMemPool==0))
                {
                    /* keep the head of the list we need to free incase of an error */
                    allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex] = tempNextMemInfoPtr;
                }

                break;
            }
            /* if the neededMemoryBlocks is 0 or 0xFFFFFFFF --> then we did not succeed in allocating the
                needed memory from the valid block we found, look for another valid block or
                return an error incase of CPU memory allocation fail  */
            else
            {
                if (hwBucketOffsetHandle != DMM_MALLOC_FAIL)
                {
                    blockStart = newFreeBlockIndex + 1;
                }
                else
                {
                    break;
                }
            }
        }
        /* if the neededMemoryBlocks is 0 or 0xFFFFFFFF --> then we did not succeed in allocating the needed memory  */
        if ((hwBucketOffsetHandle == DMM_BLOCK_NOT_FOUND) || (hwBucketOffsetHandle == DMM_MALLOC_FAIL))
        {
            if (hwBucketOffsetHandle == DMM_BLOCK_NOT_FOUND)
            {
                retVal = GT_OUT_OF_PP_MEM;
            }
            else
            {
                retVal = GT_OUT_OF_CPU_MEM;
            }
        }
    }
    else
    {
        /* we had a CPU Memory allocation error */
        if(hwBucketOffsetHandle == DMM_MALLOC_FAIL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        else
        {
            /*We succeed in allocating the memory*/
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(hwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
            if (retVal1 != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
            }
            /*  set pending flag for future need */
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates += memSize;

        }
    }
    *hwBucketOffsetHandlePtr = hwBucketOffsetHandle;
    return retVal;
}

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
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC         *newRootBucket;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC              **lpmEngineMemPtrPtr;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *rangeListPtr;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *secondRangePtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *thirdRangePtr = NULL;
    GT_U8                                           *shareDevsList;
    GT_U8                                           shareDevListLen, devNum;
    GT_U32                                          memSize;
    GT_U8                                           prefixLength;
    GT_STATUS                                       retVal = GT_OK,retVal1 = GT_OK, retVal2 = GT_OK;
    GT_PTR                                          nextHopEntry = NULL;
    GT_U32                                          secondRangePrefixLength = 0;
    GT_U32                                          minAllocationSizeInBytes = DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS;
    GT_U32                                          groupOfNodesLines;

    GT_U32      blockIndex=0; /* calculated according to the memory offset devided by block size including gap */

    if (protocol > PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E) && (defMcNextHopEntryPtr != NULL))
    {
        /* FCoE is unicast only */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ((defUcNextHopEntryPtr == NULL) && (defMcNextHopEntryPtr == NULL))
    {
        /* there must be at least one default */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
    {
        /* we need to add a nexthop to both UC and MC range. So if one range is
           invalid it will get a nexthop anyway. This is only a dummy nexthop
           that will never be hit. */
        if (defUcNextHopEntryPtr == NULL)
        {
            defUcNextHopEntryPtr = defMcNextHopEntryPtr;
            if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                defReservedNextHopEntryPtr = defMcNextHopEntryPtr;
            }
        }
        if (defMcNextHopEntryPtr == NULL)
        {
            defMcNextHopEntryPtr = defUcNextHopEntryPtr;
        }
    }

    /* keep values in case reconstruct is needed */
    cpssOsMemCpy(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,shadowPtr->lpmRamOctetsToBlockMappingPtr,
                 sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

    /* Create the root bucket. It's created with startAddr 0, which is the start
       of the UC address space */
    newRootBucket = prvCpssDxChLpmSip6RamMngCreateNew(defUcNextHopEntryPtr,
                                                      PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
    if (newRootBucket == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    rangeListPtr = newRootBucket->rangeList;
    rangeListPtr->updateRangeInHw = GT_FALSE;

    PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(defUcNextHopEntryPtr->routeEntryMethod,rangeListPtr->pointerType);

    if (protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
    {
        /* Add a range for MC */
        secondRangePtr = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC));
        if (secondRangePtr == NULL)
        {
            cpssOsLpmFree(rangeListPtr);
            cpssOsLpmFree(newRootBucket);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        rangeListPtr->next = secondRangePtr;
        if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
        {
            newRootBucket->numOfRanges = 3;
            secondRangePtr->startAddr = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS;
            secondRangePrefixLength = PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS;
        }
        else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
        {
            newRootBucket->numOfRanges = 2;
            secondRangePtr->startAddr = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            secondRangePrefixLength = PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS;
        }
        retVal = prvCpssMathPowerOf2((GT_U8)(secondRangePrefixLength - 1), &secondRangePtr->mask);
        if (retVal != GT_OK)
        {
            cpssOsLpmFree(secondRangePtr);
            cpssOsLpmFree(rangeListPtr);
            cpssOsLpmFree(newRootBucket);
            return retVal;
        }
        PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(defMcNextHopEntryPtr->routeEntryMethod,secondRangePtr->pointerType);
        secondRangePtr->lowerLpmPtr.nextHopEntry = defMcNextHopEntryPtr;
        if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
        {
            secondRangePtr->next = NULL;
        }
        secondRangePtr->updateRangeInHw = GT_FALSE;

        /* update the trie */
        retVal = insert2Trie(newRootBucket, secondRangePtr->startAddr, secondRangePrefixLength,
                             PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,
                             defMcNextHopEntryPtr);
        if (retVal != GT_OK)
        {
            cpssOsLpmFree(secondRangePtr);
            cpssOsLpmFree(rangeListPtr);
            cpssOsLpmFree(newRootBucket);
            return retVal;
        }
        if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
        {
            /* Add a range for the reserved address space */
            thirdRangePtr = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC));
            if (thirdRangePtr == NULL)
            {
                cpssOsLpmFree(secondRangePtr);
                cpssOsLpmFree(rangeListPtr);
                cpssOsLpmFree(newRootBucket);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }
            secondRangePtr->next = thirdRangePtr;
            thirdRangePtr->startAddr = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
            prefixLength = PRV_CPSS_DXCH_LPM_RAM_IPV4_RESERVED_ADDRESS_SPACE_PREFIX_CNS;
            retVal = prvCpssMathPowerOf2((GT_U8)(prefixLength - 1), &thirdRangePtr->mask);
            if (retVal != GT_OK)
            {
                /* delete the second range (the one that represents MC) */
                delFromTrie(newRootBucket, secondRangePtr->startAddr, secondRangePrefixLength,
                            PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &nextHopEntry);

                cpssOsLpmFree(thirdRangePtr);
                cpssOsLpmFree(secondRangePtr);
                cpssOsLpmFree(rangeListPtr);
                cpssOsLpmFree(newRootBucket);
                return retVal;
            }
            PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(defReservedNextHopEntryPtr->routeEntryMethod,thirdRangePtr->pointerType);
            thirdRangePtr->lowerLpmPtr.nextHopEntry = defReservedNextHopEntryPtr;
            thirdRangePtr->next = NULL;
            thirdRangePtr->updateRangeInHw = GT_FALSE;

            /* update the trie */
            retVal = insert2Trie(newRootBucket, thirdRangePtr->startAddr, prefixLength,
                                 PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,
                                 defReservedNextHopEntryPtr);
            if (retVal != GT_OK)
            {
                /* delete the second range (the one that represents MC) */
                delFromTrie(newRootBucket, secondRangePtr->startAddr, secondRangePrefixLength,
                            PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &nextHopEntry);
                cpssOsLpmFree(thirdRangePtr);
                cpssOsLpmFree(secondRangePtr);
                cpssOsLpmFree(rangeListPtr);
                cpssOsLpmFree(newRootBucket);
                return retVal;
            }

        }
    }
    /* new node type for SIP6 architecture */
    lpmFalconGetCompressedBucketType(newRootBucket, &(newRootBucket->bucketType));

    if (updateHw == GT_TRUE)
    {
        /* both unicast and multicast use ucSearchMemArrayPtr */
        lpmEngineMemPtrPtr = shadowPtr->ucSearchMemArrayPtr[protocol];
        memSize = PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS;
        shareDevsList = shadowPtr->workDevListPtr->shareDevs;
        shareDevListLen = (GT_U8)shadowPtr->workDevListPtr->shareDevNum;

        /* allocate memory for root bucket */
        retVal = prvCpssDxChLpmSip6RamMngHandleRootNodeAllocation(lpmEngineMemPtrPtr[0], memSize,
                                                              minAllocationSizeInBytes,protocol,0,shadowPtr,
                                                              &newRootBucket->hwBucketOffsetHandle);
        if (retVal != GT_OK)
        {
            if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                /* delete the third range (the one that represents reserved) */
                delFromTrie(newRootBucket, thirdRangePtr->startAddr, PRV_CPSS_DXCH_LPM_RAM_IPV4_RESERVED_ADDRESS_SPACE_PREFIX_CNS,
                            PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &nextHopEntry);

                cpssOsLpmFree(thirdRangePtr);
            }
            if (protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
            {
                /* delete the second range (the one that represents MC) */
                delFromTrie(newRootBucket, secondRangePtr->startAddr, secondRangePrefixLength,
                            PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &nextHopEntry);
                cpssOsLpmFree(secondRangePtr);
            }
            cpssOsLpmFree(rangeListPtr);
            cpssOsLpmFree(newRootBucket);

            /* free the allocated/bound RAM memory */
            retVal2 = prvCpssDxChLpmSip6RamMngAllocatedAndBoundMemFree(shadowPtr,protocol);
            if (retVal2!=GT_OK)
            {
                return retVal2;
            }
            /* in case of fail we will need to reconstruct to the status we had before the call to prvCpssDxChLpmRamMngFindValidMemoryBlockForProtocol */
            cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
            return retVal;
        }

        /* prepare the same for root node */
        cpssOsMemSet(&(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(rootGroupOfNodes)),0,
                     sizeof(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(rootGroupOfNodes)));

        retVal =  lpmFalconFillCompressedData(newRootBucket,
                                              shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                              &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(rootGroupOfNodes).compressedNodesArray[0]),
                                              NULL /*compressedNodesData*/);
        if (retVal != GT_OK)
        {
            /* just need to free any allocations done in previous stage */

            if ((newRootBucket->hwBucketOffsetHandle != DMM_BLOCK_NOT_FOUND) && (newRootBucket->hwBucketOffsetHandle != DMM_MALLOC_FAIL))
            {
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                if (retVal1 != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                }
                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle);

                prvCpssDmmFree(newRootBucket->hwBucketOffsetHandle);
            }
            return retVal;
        }

        for (devNum = 0; devNum < shareDevListLen; devNum++)
        {
            if (newRootBucket->hwBucketOffsetHandle==0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected newRootBucket->hwBucketOffsetHandle=0\n");
            }
            /* write compressed root node */
            retVal = prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[devNum],
                                                     PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle),
                                                     &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(rootGroupOfNodes)),
                                                     &groupOfNodesLines);
            if (retVal != GT_OK)
            {
                if ((newRootBucket->hwBucketOffsetHandle != DMM_BLOCK_NOT_FOUND) && (newRootBucket->hwBucketOffsetHandle != DMM_MALLOC_FAIL))
                {
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    retVal1 = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                    if (retVal1 != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                    }
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle);

                    prvCpssDmmFree(newRootBucket->hwBucketOffsetHandle);
                }

                if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
                {
                    /* delete the third range (the one that represents reserved) */
                    delFromTrie(newRootBucket, thirdRangePtr->startAddr, PRV_CPSS_DXCH_LPM_RAM_IPV4_RESERVED_ADDRESS_SPACE_PREFIX_CNS,
                                PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &nextHopEntry);

                    cpssOsLpmFree(thirdRangePtr);
                }
                if (protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
                {
                    /* delete the second range (the one that represents MC) */
                    delFromTrie(newRootBucket, secondRangePtr->startAddr, secondRangePrefixLength,
                                PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &nextHopEntry);
                    cpssOsLpmFree(secondRangePtr);
                }
                cpssOsLpmFree(rangeListPtr);
                cpssOsLpmFree(newRootBucket);

                /* free the allocated/bound RAM memory */
                retVal2 = prvCpssDxChLpmSip6RamMngAllocatedAndBoundMemFree(shadowPtr,protocol);
                if (retVal2!=GT_OK)
                {
                    return retVal2;
                }
                /* in case of fail we will need to reconstruct to the status we had before the call to prvCpssDxChLpmRamMngFindValidMemoryBlockForProtocol */
                cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
                return retVal;
            }
        }

        /* allocation passed - update the protocolCountersPerBlockArr according to the pending array */
        retVal = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                           shadowPtr->pendingBlockToUpdateArr,
                                                           shadowPtr->protocolCountersPerBlockArr,
                                                           shadowPtr->pendingBlockToUpdateArr,
                                                           protocol,
                                                           shadowPtr->numOfLpmMemories);
        if (retVal!=GT_OK)
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            return retVal;
        }

        /* update partition block */
        if (newRootBucket->hwBucketOffsetHandle!=0)
        {
            SET_DMM_BLOCK_PROTOCOL(newRootBucket->hwBucketOffsetHandle, protocol);
            SET_DMM_BUCKET_SW_ADDRESS(newRootBucket->hwBucketOffsetHandle, newRootBucket);
        }
    }
    newRootBucket->nodeMemAddr = vrId; /* in case of Root this field is used for holding VR id instead of the hw node address */
    shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol] = newRootBucket;
    shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol] =
                                newRootBucket->bucketType;
    shadowPtr->vrRootBucketArray[vrId].valid = GT_TRUE;
    shadowPtr->vrRootBucketArray[vrId].needsHwUpdate = GT_FALSE;
    if (protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
    {
        shadowPtr->vrRootBucketArray[vrId].multicastDefault[protocol] =
            defMcNextHopEntryPtr;
    }

    return GT_OK;
}

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
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *pBucket; /* The first level bucket */

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRange;

    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT      pointerType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
    switch (nextHopPtr->routeEntryMethod)
    {
    case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_MULTIPATH_E:
        pointerType = CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E;
        break;
    case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E:
        pointerType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        break;
    default:
        break;
    }

    pBucket = createNewSip6Bucket(nextHopPtr,firstLevelPrefixLen,pointerType);
    if(pBucket == NULL)
        return NULL;

    pRange = pBucket->rangeList;

    /* beacuse it's a new bucket there is a need to update it in the HW */
    pRange->updateRangeInHw = GT_TRUE;

    return pBucket;
}



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
)
{
    GT_STATUS   rc;
    GT_U32      i,j,l,leavesCounter;
    int k;
    GT_U32      *groupOfNodesArrayPtr;
    GT_U32      *tempGonArrayPtr;
    GT_U32      bankNumber = 0;
    GT_U32      offsetInBank = 0;
    GT_U32      lpmLinesNumber = 0;
    GT_U32      value = 0;
    GT_BOOL     startNewGonLeaves = GT_FALSE;
    GT_U32      *groupOfNodesArray=NULL;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E |  CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(groupOfNodesPtr);

    groupOfNodesArray =  (GT_U32 *)cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_GROUP_OF_NODES_SIZE_IN_WORDS_CNS*sizeof(GT_U32));
    if (groupOfNodesArray == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(groupOfNodesArray,0,PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_GROUP_OF_NODES_SIZE_IN_WORDS_CNS*sizeof(GT_U32));

    groupOfNodesArrayPtr = groupOfNodesArray;

    /* start processing from regular*/
    for (j=0; j<NUMBER_OF_RANGES_IN_SUBNODE; j++)
    {
        if (groupOfNodesPtr->regularNodesArray[j].childNodeTypes0_255[0] == 0)
        {
            break;
        }

        /* take in account 4 reserved, for bit vector start k =3 */
        /* process regular line */
        for (l = 0; l < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; l++)
        {
            tempGonArrayPtr = groupOfNodesArrayPtr;
            groupOfNodesArrayPtr = groupOfNodesArrayPtr +3; /* starting from the last word of BV line */
            /* write number of leaves in first 2 bits: for implemented case it is alwase 0*/
            U32_SET_FIELD_MASKED_MAC(*groupOfNodesArrayPtr, 16, 2, 0);
            /* bits from 111 .... 108 are reserved, so in each bit vector line range info is started */
            /* from bit number 106 ->10 bit in word -> it means k = 5 */
            k = 5;
            for (i = startSubNodeAddress[l]; i <= endSubNodeAddress[l]; i++,k--)
            {
                U32_SET_FIELD_MASKED_MAC(*groupOfNodesArrayPtr, k *2, 2,
                                          groupOfNodesPtr->regularNodesArray[j].childNodeTypes0_255[i]);
                if (k==0)
                {
                    k = 16;
                    groupOfNodesArrayPtr--;
                }
                if (i == 255)
                {
                    /* This is last range, go bank and offset word */
                    groupOfNodesArrayPtr--;
                }
            }

            /* add pointer bits 19 -0: 19-15 bank number, 14-0 offset in bank */
            offsetInBank = groupOfNodesPtr->regularNodesArray[j].lpmOffsets[l] & 0x7fff;
            bankNumber = (groupOfNodesPtr->regularNodesArray[j].lpmOffsets[l] >> 15) & 0x1f;
            U32_SET_FIELD_MASKED_MAC(*groupOfNodesArrayPtr,15,5,bankNumber & 0x1f);
            U32_SET_FIELD_MASKED_MAC(*groupOfNodesArrayPtr,0,15,offsetInBank & 0x7fff);
            if (groupOfNodesArrayPtr != tempGonArrayPtr)
            {
                cpssOsPrintf("prvCpssDxChLpmGroupOfNodesWrite: wrong GON ptr\n ");
            }
            /* Now bit vector line is ready. Move pointer to next bv line */
            groupOfNodesArrayPtr = groupOfNodesArrayPtr + 4;
            lpmLinesNumber++;
        }
    }

    /* start processing compressed */
    for (j=0; j<NUMBER_OF_RANGES_IN_SUBNODE; j++)
    {
        if (groupOfNodesPtr->compressedNodesArray[j].childNodeTypes0_9[0] == 0)
        {
            /* Check FCOE root bucket case - one range one leaf */
            if (!((groupOfNodesPtr->compressedNodesArray[j].childNodeTypes0_9[1] == 0) &&
                  (groupOfNodesPtr->compressedNodesArray[j].childNodeTypes0_9[2] == 0) &&
                  (groupOfNodesPtr->compressedNodesArray[j].childNodeTypes0_9[3] == 0) &&
                  (groupOfNodesPtr->compressedNodesArray[j].childNodeTypes0_9[4] == 0) &&
                  (groupOfNodesPtr->compressedNodesArray[j].childNodeTypes0_9[5] == 0) &&
                  (groupOfNodesPtr->compressedNodesArray[j].childNodeTypes0_9[6] != 0) &&
                  (groupOfNodesPtr->compressedNodesArray[j].numberOfLeaves == 1)))
            {
                break;
            }
        }
        /*check compress type*/
        rc = prvCpssDxChLpmFalconCompressedNodeDataBuild(devNum,
                                                         &groupOfNodesPtr->compressedNodesArray[j],
                                                         &groupOfNodesArrayPtr);
        if (rc != GT_OK)
        {
            cpssOsFree(groupOfNodesArray);
            return rc;
        }
        lpmLinesNumber++;
    }
    /* start processing leaves*/
    /* There is 2 types of leaves unicast and multicast*/

    leavesCounter = 0; /* leaves counter */
    k = 0; /* offset inside word */
    startNewGonLeaves = GT_TRUE;
    for (j=0; j<NUMBER_OF_RANGES_IN_SUBNODE; j++)
    {
        value = 0;
        if (groupOfNodesPtr->leafNodesArray[j].entryType == 0)
        {
            break;
        }
        /* in one lpm entry- 4 words-  we have 5 leaves*/
        rc = prvCpssDxChLpmFalconLeafNodeDataBuild(devNum,
                                                   &groupOfNodesPtr->leafNodesArray[j],
                                                   leavesCounter,
                                                   startNewGonLeaves,
                                                   &groupOfNodesArrayPtr);
        if (rc != GT_OK)
        {
            cpssOsFree(groupOfNodesArray);
            return rc;
        }
        startNewGonLeaves = GT_FALSE;
        leavesCounter++;
    }
    value = leavesCounter /MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;
    if ((leavesCounter%MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS) != 0)
    {
        value++;
    }
    lpmLinesNumber = lpmLinesNumber + value;
    *groupOfNodesLinesPtr=lpmLinesNumber;

    /* Write the node */
    rc = prvCpssDxChSip6RamMngWriteMultiEntry(devNum,
                                         CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                         lpmGroupLineOffset,
                                         lpmLinesNumber,
                                         &groupOfNodesArray[0]);
    cpssOsFree(groupOfNodesArray);
    return rc;

}

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
)
{
    GT_STATUS   rc=GT_OK;
    GT_U32      lpmGroupLineOffset; /*group of nodes offset from the LPM base in LPM Lines*/
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT          protocol;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC       *bucketShadowPtr=NULL;
    GT_U32      hwBankIndex=0;
    GT_UINTPTR  tempHwAddrHandle=0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E |  CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(groupOfNodesLinesPtr);

    if (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(*hwGroupOffsetHandlePtr) > *groupOfNodesLinesPtr)
    {
        /* need to free the block in the partition and  allocate it again with the correct size */
        /* keep parameters to be updated in the new block allocted */

        protocol = GET_DMM_BLOCK_PROTOCOL(*hwGroupOffsetHandlePtr);
        bucketShadowPtr = (PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *)GET_DMM_BUCKET_SW_ADDRESS(*hwGroupOffsetHandlePtr);

        hwBankIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(*hwGroupOffsetHandlePtr)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
        rc = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&hwBankIndex);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
        }

        lpmGroupLineOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(*hwGroupOffsetHandlePtr);

        shadowPtr->pendingBlockToUpdateArr[hwBankIndex].updateDec=GT_TRUE;
        shadowPtr->pendingBlockToUpdateArr[hwBankIndex].numOfDecUpdates +=
            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(*hwGroupOffsetHandlePtr);

        prvCpssDmmFree(*hwGroupOffsetHandlePtr);

        rc = prvCpssDmmAllocateByPtr(shadowPtr->lpmRamStructsMemPoolPtr[hwBankIndex],
                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * lpmGroupLineOffset,
                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * (*groupOfNodesLinesPtr),
                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS,
                                 &tempHwAddrHandle);

        if ((tempHwAddrHandle == DMM_BLOCK_NOT_FOUND)||(tempHwAddrHandle == DMM_MALLOC_FAIL))
        {
            /* should never happen since we just freed this mem when calling prvCpssDmmFree */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmGroupOfNodesCheckAndUpdateNewLinesNeededInThePartition - GT_FAIL \n");
        }

        *hwGroupOffsetHandlePtr = tempHwAddrHandle;
        SET_DMM_BLOCK_PROTOCOL(*hwGroupOffsetHandlePtr, protocol);
        SET_DMM_BUCKET_SW_ADDRESS(*hwGroupOffsetHandlePtr, bucketShadowPtr);

        shadowPtr->pendingBlockToUpdateArr[hwBankIndex].updateInc=GT_TRUE;
        shadowPtr->pendingBlockToUpdateArr[hwBankIndex].numOfIncUpdates +=
            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandle);
    }

    return rc;
}

/**
* @internal prvCpssDxChLpmSip6RamMngGetLeafOffsetAndIndexInLeafLine function
* @endinternal
*
* @brief This function the lpm structures in PP's memory.
*
* @param[in] bucketPtr             - Pointer to the bucket on which needed range exist.
* @param[in] rangeAddr             - start address of needed range.
* @param[in] startSubNodeAddress   - start subnode address. (relevant for regular bucket)
* @param[in] endSubNodeAddress     - end subnode address. (relevant for regular bucket)
* @param[in] hwGroupOffset         - hw group offset
*                                    the ranges where the writeRangeInHw is set.
* @param[out] leafLineNumber         - The hw offset of needed leaf line.
* @param[out] indexInLine            - leaf position in line
*
* @retval   GT_OK on success, or
* @retval   GT_FAIL - otherwise
*/
static GT_STATUS prvCpssDxChLpmSip6RamMngGetLeafOffsetAndIndexInLeafLine
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    IN  GT_U32                                    rangeStartAddr,
    IN  GT_U8                                     startSubNodeAddress,
    IN  GT_U8                                     endSubNodeAddress,
    IN  GT_U32                                    hwGroupOffset,
    OUT GT_U32                                    *leafLineNumber,
    OUT GT_U32                                    *indexInLine
)
{
    GT_U32 leafCounter = 0;
    GT_U32 compressedCounter = 0;
    GT_U32 regularCounter = 0;
    GT_U32 numberOfLeavesNodesBeforeRange = 0;


    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *rangePtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *updatedRangePtr = NULL;
    rangePtr = bucketPtr->rangeList;
    switch (bucketPtr->bucketType)
    {
    case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        break;
    case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
        /* choose needed range for subnode */
        while (rangePtr !=NULL)
        {
            if (rangePtr->startAddr >= startSubNodeAddress)
            {
                break;
            }
            else
            {
                /* check if the range incorporate needed range */
                if (rangePtr->next != NULL)
                {
                    if ((rangePtr->next->startAddr - 1) >= startSubNodeAddress)
                    {
                        /* It must be start from this range */
                        break;
                    }
                }
            }
            rangePtr = rangePtr->next;
        }
    }
    while (rangePtr != NULL)
    {
        if (bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            if (rangePtr->startAddr > endSubNodeAddress)
            {
                break;
            }
        }
        if (rangePtr->startAddr == rangeStartAddr)
        {
            updatedRangePtr = rangePtr;

            if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
                (rangePtr->pointerType == 0xff) )
            {
                /* it is needed leaf */
                numberOfLeavesNodesBeforeRange = leafCounter;

            }
        }
        if (rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
           regularCounter++;
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
        {
            compressedCounter++;
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )
        {
            leafCounter++;
        }
        rangePtr = rangePtr->next;
    }
    if (updatedRangePtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "updatedRangePtr : Can't find updated range! ");

    }
    if( (updatedRangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
        (updatedRangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
        (updatedRangePtr->pointerType == 0xff) )
    {
        *leafLineNumber = hwGroupOffset+PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS*regularCounter +
             PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS*compressedCounter + numberOfLeavesNodesBeforeRange/MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;
        *indexInLine    = numberOfLeavesNodesBeforeRange%MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "updatedRangePtr : Can't find leaf range! ");
    }

    return GT_OK;
}






/**
* @internal prvCpssDxChLpmSip6RamMngSearchLeafOffset function
* @endinternal
*
* @brief   Search for the existence of a given (address,Prefix) in the given LPM
*         structure and return associated hw bucket offset and pointer to the bucket.
* @param[in] bucketPtr                - A pointer to the bucket to search in.
* @param[in] addrCurBytePtr           - The Byte Array represnting the Address to search for.
* @param[in] prefix                   - address  length.
*
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
)
{
    GT_U8                                       startAddr, prevStartAddr;
    GT_U8                                       endAddr;
    GT_U32                                      currPrefix      = prefix;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *currBucketPtr  = bucketPtr;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *prevBucketPtr = NULL;
    GT_U8                                       *currAddrByte   = addrCurBytePtr;
    GT_PTR                                      *nextHopPtr;
    GT_U8                                       subNodeId = 0;
    GT_UINTPTR                                  groupStartAddrHandle = 0;

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC      *pRange;
    GT_U32 hwGonOffset = 0;
    GT_U32 bvLeafId[2] = {0};
    GT_STATUS rc = GT_OK;
    GT_U32 leafLineNumber,i,j,indexInLine;
    GT_BOOL lastLevelBucketEmbLeaves = GT_FALSE;

    if(bucketPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,&endAddr);
    prevStartAddr = startAddr;
    while (currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
    {
        pRange = find1stOverlap(currBucketPtr,startAddr,NULL);

        if((pRange->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
           (pRange->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
           ((GT_U32)(pRange->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        /* search in the next level.    */
        prevBucketPtr = currBucketPtr;
        prevStartAddr = startAddr;
        currBucketPtr = pRange->lowerLpmPtr.nextBucket;
        currAddrByte += 1;
        currPrefix = NEXT_BUCKET_PREFIX_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

        calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,&endAddr);
    }

    nextHopPtr = getFromTrie(currBucketPtr,startAddr,currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

    if(nextHopPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* check whether the bucket type is embedded compressed */
    if ( (currBucketPtr->numOfRanges <=3) && (currBucketPtr->bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E) )
    {
        lastLevelBucketEmbLeaves = lpmFalconCheckIfLastLevelBucketEmbdLeaves(currBucketPtr);
    }
    /* check the bucket type is compressed*/
    else if (currBucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)
    {
        if (currBucketPtr->hwGroupOffsetHandle[0]==0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected currBucketPtr->hwGroupOffsetHandle[0]=0\n");
        }

        hwGonOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(currBucketPtr->hwGroupOffsetHandle[0]);

        rc = prvCpssDxChLpmSip6RamMngGetLeafOffsetAndIndexInLeafLine(currBucketPtr,
                                                                     startAddr,
                                                                     startSubNodeAddress[0],
                                                                     endSubNodeAddress[0],
                                                                     hwGonOffset,
                                                                     &leafLineNumber,
                                                                     &indexInLine);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        leafLineOffsetPtr[0] = leafLineNumber;
        leafOffsetInLinePtr[0] = indexInLine;
        *leavesNumberPtr = 1;
        return rc;
    }

    /* now we have needed node shadow*/
    /* we need to get to leaf line and place of given leaf in line */
    bvLeafId[0] = startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
    bvLeafId[1] = endAddr/NUMBER_OF_RANGES_IN_SUBNODE;
    /* max number of involved leaves is 4 */
    *leavesNumberPtr = bvLeafId[1] - bvLeafId[0] + 1;
    for (i = bvLeafId[0], j = 0; (i <= bvLeafId[1]) && (j < *leavesNumberPtr) ; i++,j++ )
    {
        if (lastLevelBucketEmbLeaves == GT_TRUE)
        {
            subNodeId = prevStartAddr/NUMBER_OF_RANGES_IN_SUBNODE;
            if(prevBucketPtr==NULL)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "\n ERROR unexpected prevBucketPtr==NULL \n");
            }
            if (prevBucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                groupStartAddrHandle = prevBucketPtr->hwGroupOffsetHandle[subNodeId];
            }
            else
            {
                groupStartAddrHandle = prevBucketPtr->hwGroupOffsetHandle[0];
            }

            if (groupStartAddrHandle==0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected groupStartAddrHandle=0\n");
            }
            /* calculate offset of embedded leaves bucket inside of GON */
            rc = lpmFalconGetHwNodeOffsetInsideGroupOfNodes(prevBucketPtr,
                                                            prevStartAddr,
                                                            startSubNodeAddress[subNodeId],
                                                            endSubNodeAddress[subNodeId],
                                                            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(groupStartAddrHandle),
                                                            &hwGonOffset);

           /* hwGonOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(prevBucketPtr->hwGroupOffsetHandle[i]);*/
        }
        else
        {
            if (currBucketPtr->hwGroupOffsetHandle[i]==0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected currBucketPtr->hwGroupOffsetHandle[i]=0\n");
            }
            hwGonOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(currBucketPtr->hwGroupOffsetHandle[i]);
        }
        rc = prvCpssDxChLpmSip6RamMngGetLeafOffsetAndIndexInLeafLine(currBucketPtr,
                                                                     startAddr,
                                                                     startSubNodeAddress[i],
                                                                     endSubNodeAddress[i],
                                                                     hwGonOffset,
                                                                     &leafLineNumber,
                                                                     &indexInLine);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        leafLineOffsetPtr[j] = leafLineNumber;
        leafOffsetInLinePtr[j] = indexInLine;
    }

    return rc;
}

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
)
{
    GT_U8                                       startAddr, prevStartAddr;
    GT_U8                                       endAddr;
    GT_U32                                      currPrefix      = prefix;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *currBucketPtr  = bucketPtr;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *prevBucketPtr = NULL;
    GT_U8                                       *currAddrByte   = addrCurBytePtr;
    GT_PTR                                      *nextHopPtr;
    GT_U8                                       subNodeId = 0;
    GT_UINTPTR                                  groupStartAddrHandle = 0;

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC      *pRange;
    GT_U32 hwGonOffset = 0;
    GT_STATUS rc = GT_OK;
    GT_U32 leafLineNumber,indexInLine;
    GT_BOOL lastLevelBucketEmbLeaves = GT_FALSE;

    if(bucketPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,&endAddr);
    prevStartAddr = startAddr;
    while (currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
    {
        pRange = find1stOverlap(currBucketPtr,startAddr,NULL);

        if((pRange->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
           (pRange->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
           ((GT_U32)(pRange->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        /* search in the next level.    */
        prevBucketPtr = currBucketPtr;
        prevStartAddr = startAddr;
        currBucketPtr = pRange->lowerLpmPtr.nextBucket;
        currAddrByte += 1;
        currPrefix = NEXT_BUCKET_PREFIX_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

        calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,&endAddr);
    }
    nextHopPtr = getFromTrie(currBucketPtr,startAddr,currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

    if(nextHopPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    else
    {
        *lastGroupbucketTypePtr = currBucketPtr->bucketType;
        /* check the bucket type */
        if ( (currBucketPtr->numOfRanges <=3) && (currBucketPtr->bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E) )
        {
            lastLevelBucketEmbLeaves = lpmFalconCheckIfLastLevelBucketEmbdLeaves(currBucketPtr);
        }
        *lastLevelBucketEmbLeavesPtr = lastLevelBucketEmbLeaves;
        if (lastLevelBucketEmbLeaves == GT_TRUE)
        {
            subNodeId = prevStartAddr/NUMBER_OF_RANGES_IN_SUBNODE;
            if(prevBucketPtr==NULL)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "\n ERROR unexpected prevBucketPtr==NULL \n");
            }
            if (prevBucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                groupStartAddrHandle = prevBucketPtr->hwGroupOffsetHandle[subNodeId];
            }
            else
            {
                groupStartAddrHandle = prevBucketPtr->hwGroupOffsetHandle[0];
            }
            if (groupStartAddrHandle==0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected groupStartAddrHandle=0\n");
            }
            /* calculate offset of embedded leaves bucket inside of GON */
            rc = lpmFalconGetHwNodeOffsetInsideGroupOfNodes(prevBucketPtr,
                                                            prevStartAddr,
                                                            startSubNodeAddress[subNodeId],
                                                            endSubNodeAddress[subNodeId],
                                                            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(groupStartAddrHandle),
                                                            &hwGonOffset);

           /* hwGonOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(prevBucketPtr->hwGroupOffsetHandle[i]);*/
        }
        else
        {
            if (bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                subNodeId = startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
            }
            else
            {
                subNodeId = 0;
            }

            if (currBucketPtr->hwGroupOffsetHandle[subNodeId]==0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected currBucketPtr->hwGroupOffsetHandle[subNodeId]=0\n");
            }
            hwGonOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(currBucketPtr->hwGroupOffsetHandle[subNodeId]);
        }
        rc = prvCpssDxChLpmSip6RamMngGetLeafOffsetAndIndexInLeafLine(currBucketPtr,
                                                                     startAddr,
                                                                     startSubNodeAddress[subNodeId],
                                                                     endSubNodeAddress[subNodeId],
                                                                     hwGonOffset,
                                                                     &leafLineNumber,
                                                                     &indexInLine);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        *leafLineOffsetPtr = leafLineNumber;
        *leafOffsetInLinePtr = indexInLine;
    }
    return rc;
}

/**
* @internal lpmFalconGetGroupOfNodesSizes function
* @endinternal
*
* @brief Calc the ammount of needed Pp memory for given group of nodes.
*
* @param[in]  bucketPtr         - Pointer to the given bucket.
* @param[out] groupOfNodesSizes - group of nodes sizes array
*
* @retval   GT_OK               - on success,
* @retval   GT_FAIL             - Otherwise.
*/
static GT_STATUS lpmFalconGetStaticGonsSizes
(
    IN    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC       *bucketPtr,
    OUT   GT_U32                                        *groupOfNodesSizes
)
{
    GT_STATUS                                        rc = GT_OK;
    GT_U32                                           regularNodesNumber[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U32                                           compressedNodesNumber[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U32                                           leafNodesNumber[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U32                                           leafNodesSize[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U8                                            startAddr = 0;
    GT_U8                                            endAddr = 255;
    GT_U32                                           i = 0;
    GT_U32                                           maxNodesNumber = 0;
    GT_BOOL isCompressedWithEmbLeaves = GT_FALSE;
    GT_BOOL isStartAddrMatchRegion = GT_FALSE;
    GT_BOOL isEndAddrMatchRegion = GT_FALSE;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT compressedNodeType = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;

    switch (bucketPtr->bucketType)
    {
    case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:

        maxNodesNumber = 1;
        rc = lpmFalconGetGroupOfNodesItems(bucketPtr->rangeList,CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E,
                                           startAddr,endAddr,&isStartAddrMatchRegion,&isEndAddrMatchRegion,
                                           &regularNodesNumber[0],&compressedNodesNumber[0],
                                           &leafNodesNumber[0]);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* verify do we need special place for leaves or it is embedded leaves*/
        /* in order to do that check if current node after adding new regions is node with embedded leaves*/
        lpmFalconCheckIfCompressedWithEmbeddedLeaves(bucketPtr->numOfRanges,leafNodesNumber[0],
                                                     &isCompressedWithEmbLeaves, &compressedNodeType);
        if (isCompressedWithEmbLeaves == GT_TRUE)
        {
            /* all leaves are embedded.*/
            leafNodesNumber[0] = 0;
        }
        break;
    case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        maxNodesNumber = PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS;

        rc = lpmFalconGetGroupOfNodesItems(bucketPtr->rangeList,CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E,startAddr,endAddr,
                                           &isStartAddrMatchRegion,&isEndAddrMatchRegion, &regularNodesNumber[0],
                                           &compressedNodesNumber[0],&leafNodesNumber[0]);
        if (rc != GT_OK)
        {
            return rc;
        }
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

        for (i = 0; i < maxNodesNumber; i++)
        {
            /* calculate place for leaves */
            leafNodesSize[i] = leafNodesNumber[i]/MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;
            if ( (leafNodesNumber[i] % MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS) != 0 )
                leafNodesSize[i]++;
            groupOfNodesSizes[i] =
                regularNodesNumber[i]*PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS +
                 compressedNodesNumber[i]*PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS + leafNodesSize[i];
        }
        return GT_OK;
}

/**
* @internal handleDeleteLackOfMemory function
* @endinternal
*
* @brief This function updates root node under swap operation mode
*
* @param[in] parentNodeDataPtr     - pointer to the bucket to parent node parameters.
*
* @retval   GT_OK on success, or
* @retval   GT_OUT_OF_CPU_MEM - if failed to allocate CPU memory, or
* @retval   GT_OUT_OF_PP_MEM - if failed to allocate PP memory.
*/
static GT_STATUS handleDeleteLackOfMemory
(
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC      *parentNodeDataPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8 subNodeId1 = 0;
    GT_U32 i,j,level,tempAddr, gonOffset;
    GT_U32 shareDevsListLen;
    GT_U8  *shareDevsList;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr = NULL;
    PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC   *groupOfNodes=NULL;
    GT_U32 groupOfNodesLines;

    groupOfNodes =  (PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC *)cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS*sizeof(PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC));
    if (groupOfNodes == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(groupOfNodes,0,PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS*sizeof(PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC));

    shadowPtr = parentNodeDataPtr->shadowPtr;
    level = parentNodeDataPtr->level;
    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevsListLen = shadowPtr->workDevListPtr->shareDevNum;

    subNodeId1 = parentNodeDataPtr->rangePtrArray[level]->startAddr / NUMBER_OF_RANGES_IN_SUBNODE;
    /* build needed gons*/
    for (i = subNodeId1; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i ++ )
    {
        if (parentNodeDataPtr->bucketPtrArray[level]->hwGroupOffsetHandle[i]==0)
        {
            cpssOsFree(groupOfNodes);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected parentNodeDataPtr->bucketPtrArray[level]->hwGroupOffsetHandle[i]=0\n");
        }
        rc = getMirrorGroupOfNodesDataAndUpdateRangesAddress(parentNodeDataPtr->bucketPtrArray[level]->bucketType,
                                                             startSubNodeAddress[i],
                                                             endSubNodeAddress[i],
                                                             parentNodeDataPtr->bucketPtrArray[level]->rangeList,
                                                             parentNodeDataPtr->shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                             PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(parentNodeDataPtr->bucketPtrArray[level]->hwGroupOffsetHandle[i]),
                                                             &groupOfNodes[i]);
        if (rc != GT_OK)
        {
            cpssOsFree(groupOfNodes);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "getMirrorGroupOfNodesDataAndUpdateRangesAddress fails");
        }
    }

    /* now update affected GONs */
    for (j = subNodeId1; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j ++ )
    {
        for (i = 0; i < shareDevsListLen; i++)
        {
            if (parentNodeDataPtr->bucketPtrArray[level]->hwGroupOffsetHandle[j]==0)
            {
                cpssOsFree(groupOfNodes);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected parentNodeDataPtr->bucketPtrArray[level]->hwGroupOffsetHandle[j]=0\n");
            }

            rc =  prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[i],
                                                  PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(parentNodeDataPtr->bucketPtrArray[level]->hwGroupOffsetHandle[j]),
                                                  &groupOfNodes[j],
                                                  &groupOfNodesLines);
            if (rc != GT_OK)
            {
                cpssOsFree(groupOfNodes);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChLpmGroupOfNodesWrite");
            }

            if (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC((parentNodeDataPtr->bucketPtrArray[level]->hwGroupOffsetHandle[j])) > groupOfNodesLines)
            {
                /* if the number of lines we updated in the HW is smaller than the number
                   of lines previusly occupied by the GON then we will have a hole after the rewrite,
                   so we need to set this hole as free in the partition
                   need to free the old space and allocate the new space using prvCpssDmmAllocateByPtr */
                rc = prvCpssDxChLpmGroupOfNodesCheckAndUpdateNewLinesNeededInThePartition(shareDevsList[i],shadowPtr,
                                                         &(parentNodeDataPtr->bucketPtrArray[level]->hwGroupOffsetHandle[j]),
                                                         &groupOfNodesLines);
                if (rc != GT_OK)
                {
                    cpssOsFree(groupOfNodes);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChLpmGroupOfNodesWrite");
                }
            }
        }
    }
    /* remove range from node points on updated above gon */
    /* find address of needed node in gon */
    if (level != 0)
    {
        if (parentNodeDataPtr->bucketPtrArray[level - 1]->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            subNodeId1 = parentNodeDataPtr->rangePtrArray[level - 1]->startAddr / NUMBER_OF_RANGES_IN_SUBNODE;
        }
        else
        {
            subNodeId1 = 0;
        }
        if (parentNodeDataPtr->bucketPtrArray[level -1]->hwGroupOffsetHandle[subNodeId1]==0)
        {
            cpssOsFree(groupOfNodes);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected parentNodeDataPtr->bucketPtrArray[level -1]->hwGroupOffsetHandle[subNodeId1]");
        }
        gonOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(parentNodeDataPtr->bucketPtrArray[level -1]->hwGroupOffsetHandle[subNodeId1]);
         /* now calculate offset inside of group of nodes for node this range pointed on */
        rc = lpmFalconGetHwNodeOffsetInsideGroupOfNodes(parentNodeDataPtr->bucketPtrArray[level -1],
                                                        parentNodeDataPtr->rangePtrArray[level-1]->startAddr,
                                                        startSubNodeAddress[subNodeId1],
                                                        endSubNodeAddress[subNodeId1],
                                                        gonOffset,
                                                        &tempAddr);
        if (rc != GT_OK)
        {
            cpssOsFree(groupOfNodes);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if (parentNodeDataPtr->bucketPtrArray[level]->hwBucketOffsetHandle==0)
        {
            cpssOsFree(groupOfNodes);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected parentNodeDataPtr->bucketPtrArray[level]->hwBucketOffsetHandle");
        }
        tempAddr = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(parentNodeDataPtr->bucketPtrArray[level]->hwBucketOffsetHandle);
    }

    cpssOsMemSet(groupOfNodes,0,PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS*sizeof(PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC));
    rc = lpmFalconFillRegularData(parentNodeDataPtr->bucketPtrArray[level],
                                  shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                  &groupOfNodes[0].regularNodesArray[0]);
    if (rc != GT_OK)
    {
        cpssOsFree(groupOfNodes);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    for (i = 0; i < shareDevsListLen; i++)
    {
        rc =  prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[i],
                                              tempAddr,
                                              &groupOfNodes[0],
                                              &groupOfNodesLines);
        if (rc != GT_OK)
        {
            cpssOsFree(groupOfNodes);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChLpmGroupOfNodesWrite");
        }
    }

    parentNodeDataPtr->nextLevelDoneBySwapFunc = GT_TRUE;
    cpssOsFree(groupOfNodes);
    return GT_OK;


}

/**
* @internal handleSrcRootNode function
* @endinternal
*
* @brief This function updates root node under swap operation mode
*
* @param[in]  parentNodeDataPtr     - pointer to the bucket to parent node parameters.
*
* @retval   GT_OK on success, or
* @retval   GT_OUT_OF_CPU_MEM - if failed to allocate CPU memory, or
* @retval   GT_OUT_OF_PP_MEM - if failed to allocate PP memory.
*/
static GT_STATUS handleSrcRootNode
(
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC    *parentNodeDataPtr,
    IN GT_U8                                    *shareDevsList,
    IN GT_U32                                    shareDevsListLen,
    IN GT_U32                                    swapAddr

)
{
    GT_STATUS   rc = GT_OK;
    PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC   groupOfNodes;
    GT_U32 groupOfNodesLines;
    GT_U8 subNodeId = 0;
    GT_UINTPTR tempAddr = DMM_BLOCK_NOT_FOUND;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *bucketPtr = NULL;
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
    GT_U32 blockIndex=0;
    GT_U32 newGonSize = 0;

    bucketPtr = parentNodeDataPtr->bucketPtr;
    shadowPtr = parentNodeDataPtr->shadowPtr;

    rootBucketPtr = shadowPtr->vrRootBucketArray[parentNodeDataPtr->vrId].rootBucket[parentNodeDataPtr->protocol];

    cpssOsMemSet(&groupOfNodes, 0, sizeof(groupOfNodes));

    /* if we have a case of insert then all memory is preallocated
       need to check if we have a case of using swap area or not
       in case of delete we must enter to this if code */
    if (((parentNodeDataPtr->addOperation==GT_TRUE) && (parentNodeDataPtr->swapAreaAlsoUsedForAddOnParentLevel==GT_TRUE))||
         (parentNodeDataPtr->addOperation==GT_FALSE))
    {
        rc = updateSwapArea(subNodeId,NULL,bucketPtr->hwBucketOffsetHandle,0,
                            swapAddr,shareDevsList,shareDevsListLen,parentNodeDataPtr->swapGonsAdresses);

        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "updateSwapArea failed");
        }

        /* before free src root node cause group ptr to point on swap2 */
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

            /* now change ptr in neaded leaf */
            rc = prvCpssDxChReadTableMultiEntry(shareDevsList[i],
                                                 CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                 leafLineOffset,
                                                 1,
                                                 gonDataArray);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "LPM read fails");
            }

            U32_SET_FIELD_IN_ENTRY_MAC(gonDataArray,startBitOfLeafInLine+sipPtrOffset,sipPtrLength,swapAddr);

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
        if (bucketPtr->hwBucketOffsetHandle==0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected bucketPtr->hwBucketOffsetHandle");
        }
        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
        rc = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
        }
        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle);

        /* free src root node */
        prvCpssDmmFree(bucketPtr->hwBucketOffsetHandle);

        if(parentNodeDataPtr->addOperation==GT_FALSE)
        {
            /* allocate new node : it is compressed node  - memory was freed so must pass new allocation */
            newGonSize = 1;
            bucketPtr->bucketType=CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;
        }
        else
        {
            /* allocate new node : it is regular node */
            newGonSize = 6;
            bucketPtr->bucketType=CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
        }

        tempAddr = prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[blockIndex],
                                      DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * newGonSize,
                                      DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);

        if (tempAddr == DMM_BLOCK_NOT_FOUND)
        {
            /* it can't be possible */
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "prvCpssDmmAllocate fails");
        }

        if (tempAddr==0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected tempAddr=0\n");
        }
        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempAddr)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
        rc = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
        }
        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempAddr);

        bucketPtr->hwBucketOffsetHandle = tempAddr;

        if (parentNodeDataPtr->addOperation==GT_FALSE)
        {
            /* create new */
            rc = lpmFalconFillCompressedData(bucketPtr,
                                             shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                         &groupOfNodes.compressedNodesArray[0],
                                         NULL);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "lpmFalconFillCompressedData fails");
            }
        }
        else
        {
            rc = lpmFalconFillRegularData(bucketPtr,
                                      shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                      &groupOfNodes.regularNodesArray[0]);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "lpmFalconFillRegularData fails");
            }
        }
    }
    else
    {
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

        /* preallocated memory */

        tempAddr = parentNodeDataPtr->neededMemoryBlocksInfoPtr->neededMemoryBlocks[0];

        /* create new */
        bucketPtr->bucketType=CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
        rc = lpmFalconFillRegularData(bucketPtr,
                                      shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                      &groupOfNodes.regularNodesArray[0]);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "lpmFalconFillCompressedData fails");
        }

        bucketPtr->hwBucketOffsetHandle = tempAddr;
    }

    if (tempAddr==0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected tempAddr=0\n");
    }
    bucketPtr->nodeMemAddr = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempAddr);
    /* write it into new memory */
    for (i = 0; i < shareDevsListLen; i++)
    {
        rc =  prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[i],
                                              PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempAddr),
                                              &groupOfNodes,
                                              &groupOfNodesLines);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChLpmGroupOfNodesWrite");
        }
    }
    /* change pointer */
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

        U32_SET_FIELD_IN_ENTRY_MAC(gonDataArray,startBitOfLeafInLine+sipPtrOffset,sipPtrLength,PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempAddr));
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

    if(parentNodeDataPtr->addOperation==GT_TRUE)
    {
        if(parentNodeDataPtr->swapAreaAlsoUsedForAddOnParentLevel==GT_FALSE)
        {
            if (bucketPtr->hwBucketOffsetHandle==0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected bucketPtr->hwBucketOffsetHandle=0\n");
            }
            /* now it is ok to free old SRC memory */
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            rc = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
            }
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle);

            /* free src root node */
            prvCpssDmmFree(bucketPtr->hwBucketOffsetHandle);
        }

        /* parent was updated - need to increment counter */
        shadowPtr->neededMemoryCurIdx++;
    }

    parentNodeDataPtr->nextLevelDoneBySwapFunc = GT_TRUE;

    return rc;

}

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
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8 subNodeId = 0;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *bucketPtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr = NULL;
    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
    GT_U32 shareDevListLen;
    GT_U32 tempAddr;
    GT_U32 tempAddr1;
    GT_UINTPTR tempAddr2;
    GT_U32 i,j = 0;
    GT_U32 gonSize = 0;
    GT_U32 newGonSize = 0;
    GT_U32 gonDataArray[4] = {0};
    GT_U32 level  = 0;
    GT_U32 blockIndex=0;
    GT_U32 swap_addr = 0xffffffff;
    GT_UINTPTR oldHwAddrHandle = 0;
    GT_U32 groupOfNodesLines;

    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_3), 0,
                 sizeof(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_3)));
    bucketPtr = parentNodeDataPtr->bucketPtr;
    shadowPtr = parentNodeDataPtr->shadowPtr;
    level = parentNodeDataPtr->level;
    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;


    /* there are 2 cases update range in root node and other nodes */
    /* in case of regular no more than 2 nodes can be updated simultaneously */

    /* calculate group of nodes start address*/
    if ( (parentNodeDataPtr->isBvCompConvertion == GT_TRUE) &&
         (parentNodeDataPtr->funcCallCounter == 2)  )
    {
        /* it means that on last level all gons are written into swap area, old gons memory are freed*/
        /* pointers of regular node are points on swap gons. The problem is that regular must be     */
        /* converted into compressed  */
        /* move GON to swap2 */
        cpssOsMemSet(parentNodeDataPtr->swapGonsAdresses, 0xff,sizeof(parentNodeDataPtr->swapGonsAdresses));
        if (shadowPtr->secondSwapMemoryAddr==0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected shadowPtr->secondSwapMemoryAddr=0\n");
        }
        swap_addr = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->secondSwapMemoryAddr);
        if (parentNodeDataPtr->isRootNode == GT_TRUE)
        {
            /* if we are in add operation the swap memory is already assigned, so override the sawp_addr value */
            if (parentNodeDataPtr->addOperation==GT_TRUE)
            {
                if (parentNodeDataPtr->neededMemoryBlocksInfoPtr->neededMemoryBlocks[0]==0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected parentNodeDataPtr->neededMemoryBlocksInfoPtr->neededMemoryBlocks[0]=0\n");
                }
                swap_addr = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(parentNodeDataPtr->neededMemoryBlocksInfoPtr->neededMemoryBlocks[0]);
            }

            return handleSrcRootNode(parentNodeDataPtr,shareDevsList,shareDevListLen,swap_addr);
        }

        /* if we have a case of insert then all memory is preallocated
           need to check if we have a case of using swap area or not
           in case of delete we must enter to this if code */
        if (((parentNodeDataPtr->addOperation==GT_TRUE) && (parentNodeDataPtr->swapAreaAlsoUsedForAddOnParentLevel==GT_TRUE))||
            (parentNodeDataPtr->addOperation==GT_FALSE))
        {
            if (parentNodeDataPtr->bucketPtrArray[level - 1]->bucketType==CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                subNodeId = parentNodeDataPtr->rangePtrArray[level - 1]->startAddr / NUMBER_OF_RANGES_IN_SUBNODE;
            }
            else
            {
                subNodeId = 0;
            }
            rc = updateSwapArea(subNodeId,bucketPtr->hwGroupOffsetHandle,0xffffffff,0,
                                swap_addr,shareDevsList,shareDevListLen,parentNodeDataPtr->swapGonsAdresses);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "updateSwapArea failed");
            }
            /* change pointer of current node to point on swap */
            /* need prange for level -2*/
            if (level -1 == 0)
            {
                if (parentNodeDataPtr->bucketPtrArray[level -1]->hwBucketOffsetHandle==0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected parentNodeDataPtr->bucketPtrArray[level -1]->hwBucketOffsetHandle=0\n");
                }
                /* need to change pointer in root node */
                tempAddr1 = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(parentNodeDataPtr->bucketPtrArray[level -1]->hwBucketOffsetHandle);
            }
            else
            {
                /* now calculate offset inside of group of nodes for node this range pointed on */
                tempAddr1 = parentNodeDataPtr->rangePtrArray[level-2]->lowerLpmPtr.nextBucket->nodeMemAddr;

            }
            for (i = 0; i < shareDevListLen; i++)
            {

                rc = prvCpssDxChReadTableMultiEntry(shareDevsList[i],
                                                     CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                     tempAddr1+subNodeId,
                                                     1,
                                                     gonDataArray);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "LPM read fails");
                }
                /* change ptr : taken from swapGonsAdresses */
                /* check node types: 3-3, 2-2 with ptr =0 can't be */

                U32_SET_FIELD_IN_ENTRY_MAC(gonDataArray,0,20,swap_addr);
                /* Write the node */
                rc = prvCpssDxChSip6RamMngWriteMultiEntry(shareDevsList[j],
                                                     CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                     tempAddr1+subNodeId,
                                                     1,
                                                     gonDataArray);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "LPM write fails");
                }
            }
            /* free memory : */
            if (parentNodeDataPtr->bucketPtrArray[level - 1]->bucketType==CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                subNodeId = parentNodeDataPtr->rangePtrArray[level-1]->startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
            }
            else
            {
                subNodeId = 0;
            }
            if (parentNodeDataPtr->bucketPtrArray[level - 1]->hwGroupOffsetHandle[subNodeId]==0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected parentNodeDataPtr->bucketPtrArray[level - 1]->hwGroupOffsetHandle[subNodeId]=0\n");
            }
            gonSize = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(parentNodeDataPtr->bucketPtrArray[level - 1]->hwGroupOffsetHandle[subNodeId]);

            oldHwAddrHandle = parentNodeDataPtr->bucketPtrArray[level - 1]->hwGroupOffsetHandle[subNodeId];
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(oldHwAddrHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            rc = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
            }
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(oldHwAddrHandle);

            prvCpssDmmFree(oldHwAddrHandle);

            if(parentNodeDataPtr->addOperation==GT_FALSE)
            {
                /* regular to compress*/
                newGonSize = gonSize - 5;
            }
            else
            {
                /* compress to regular */
                newGonSize = gonSize + 5;
            }

            /* we just freed space so allocation must pass  */
            tempAddr2 = prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[blockIndex],
                               DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * newGonSize,
                               DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);

            if (tempAddr2 == DMM_BLOCK_NOT_FOUND)
            {
                /* it can't be possible */
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "prvCpssDmmAllocate fails");
            }
            if ((GT_UINTPTR)tempAddr2==0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected (GT_UINTPTR)tempAddr2=0\n");
            }
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC((GT_UINTPTR)tempAddr2)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            rc = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
            }
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC((GT_UINTPTR)tempAddr2);

            parentNodeDataPtr->bucketPtrArray[level - 1]->hwGroupOffsetHandle[subNodeId] = tempAddr2;
        }
        else
        {
             /* this mean parentNodeDataPtr->swapAreaAlsoUsedForAddOnParentLevel==GT_FALSE
               need to use the preallocated memory - insert case */
            if (parentNodeDataPtr->bucketPtrArray[level - 1]->bucketType==CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                subNodeId = parentNodeDataPtr->rangePtrArray[level - 1]->startAddr / NUMBER_OF_RANGES_IN_SUBNODE;
            }
            else
            {
                subNodeId = 0;
            }

            /* keep pointer of current node */
            /* need prange for level -2*/
            if (level -1 == 0)
            {
                if (parentNodeDataPtr->bucketPtrArray[level -1]->hwBucketOffsetHandle==0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected parentNodeDataPtr->bucketPtrArray[level -1]->hwBucketOffsetHandle=0\n");
                }
                /* need to change pointer in root node */
                tempAddr1 = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(parentNodeDataPtr->bucketPtrArray[level -1]->hwBucketOffsetHandle);
            }
            else
            {
                /* now calculate offset inside of group of nodes for node this range pointed on */
                tempAddr1 = parentNodeDataPtr->rangePtrArray[level-2]->lowerLpmPtr.nextBucket->nodeMemAddr;
            }

            /* preallocated memory */
            tempAddr2 = parentNodeDataPtr->neededMemoryBlocksInfoPtr->neededMemoryBlocks[subNodeId];
            oldHwAddrHandle = parentNodeDataPtr->bucketPtrArray[level - 1]->hwGroupOffsetHandle[subNodeId];
            parentNodeDataPtr->bucketPtrArray[level - 1]->hwGroupOffsetHandle[subNodeId] = tempAddr2;
        }

        if ((GT_UINTPTR)tempAddr2==0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected (GT_UINTPTR)tempAddr2=0\n");
        }
        /* create new gon*/
        rc = getMirrorGroupOfNodesDataAndUpdateRangesAddress(parentNodeDataPtr->bucketPtrArray[level - 1]->bucketType,
                                                             startSubNodeAddress[subNodeId],
                                                             endSubNodeAddress[subNodeId],
                                                             parentNodeDataPtr->bucketPtrArray[level - 1]->rangeList,
                                                             parentNodeDataPtr->shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                             PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC((GT_UINTPTR)tempAddr2),
                                                             &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_3)[0]));
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "getMirrorGroupOfNodesDataAndUpdateRangesAddress fails");
        }

         /* now write gon into new allocated memory*/
         for (i = 0; i < shareDevListLen; i++)
         {
             rc =  prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[i],
                                                   PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC((GT_UINTPTR)tempAddr2),
                                                   &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_3)[0]),
                                                   &groupOfNodesLines);
             if (rc != GT_OK)
             {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChLpmGroupOfNodesWrite");
             }
         }
         cpssOsMemSet(&(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_3)), 0,
                      sizeof(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_3)));
         /* change pointer on node  level -1 to point on new allocated gon */
         for (i = 0; i < shareDevListLen; i++)
         {
                 /* update ptr from swap to new allocated gon*/
             if ((parentNodeDataPtr->isRootNode == GT_FALSE)&&
                 ((level -1) != 0))
             {
                switch (parentNodeDataPtr->rangePtrArray[level - 2]->pointerType)
                {
                case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
                case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
                case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
                case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
                    rc =  lpmFalconFillCompressedData(parentNodeDataPtr->rangePtrArray[level - 2]->lowerLpmPtr.nextBucket,
                                                      shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                      &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_3)[0].compressedNodesArray[0]),
                                                      NULL);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "lpmFalconFillCompressedData fails");
                    }
                    break;
                case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
                    rc =  lpmFalconFillRegularData(parentNodeDataPtr->rangePtrArray[level - 2]->lowerLpmPtr.nextBucket,
                                                   shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                   &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_3)[0].regularNodesArray[0]));
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "lpmFalconFillRegularData fails");
                    }
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
                }

             }
             else
             {
                /* root node*/
                if (bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                {
                    rc =  lpmFalconFillRegularData(bucketPtr,
                                                   shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                   &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_3)[0].regularNodesArray[0]));
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "lpmFalconFillRegularData fails");
                    }
                }
                else
                {
                    rc =  lpmFalconFillCompressedData(bucketPtr,
                                                      shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                      &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_3)[0].compressedNodesArray[0]),
                                                      NULL);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "lpmFalconFillCompressedData fails");
                    }
                }
             }

             /* now write gon into new allocated memory*/
             for (i = 0; i < shareDevListLen; i++)
             {
                rc =  prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[i],
                                                      tempAddr1,
                                                      &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_3)[0]),
                                                      &groupOfNodesLines);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChLpmGroupOfNodesWrite");
                }
             }
        }
        parentNodeDataPtr->nextLevelDoneBySwapFunc = GT_TRUE;

        if(parentNodeDataPtr->addOperation==GT_TRUE)
        {
            if(parentNodeDataPtr->swapAreaAlsoUsedForAddOnParentLevel==GT_FALSE)
            {
                /* now after using the preallocated memory,
                   and updating the pointers to the new memory,
                   we can free the old memory */
                if (parentNodeDataPtr->bucketPtrArray[level - 1]->bucketType==CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                {
                    subNodeId = parentNodeDataPtr->rangePtrArray[level-1]->startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
                }
                else
                {
                    subNodeId=0;
                }

                if (oldHwAddrHandle==0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected oldHwAddrHandle=0\n");
                }
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(oldHwAddrHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                rc = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                }
                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(oldHwAddrHandle);

                prvCpssDmmFree(oldHwAddrHandle);

                parentNodeDataPtr->bucketPtrArray[level - 1]->hwGroupOffsetHandle[subNodeId] = tempAddr2;
            }

            /* parent was updated - need to increment counter */
            shadowPtr->neededMemoryCurIdx++;
        }

        return rc;
    }
    if ( (parentNodeDataPtr->isBvCompConvertion == GT_TRUE) &&
         (parentNodeDataPtr->funcCallCounter == 3)  )
    {
        /* in this case swap can't help. Delete must be finished */
        return handleDeleteLackOfMemory(parentNodeDataPtr);
    }

    if (parentNodeDataPtr->isRootNode == GT_FALSE)
    {
        /* now calculate offset inside of group of nodes for node this range pointed on */
       tempAddr = parentNodeDataPtr->bucketPtrArray[level]->nodeMemAddr;
    }
    else
    {
        if (bucketPtr->hwBucketOffsetHandle==0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected bucketPtr->hwBucketOffsetHandle=0\n");
        }
        tempAddr = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle);
    }
    cpssOsMemSet(&(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_3)), 0,
                 sizeof(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_3)));
    /* tempAddr shows where this node starts in group of nodes*/
    for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
    {
        if (parentNodeDataPtr->swapGonsAdresses[j] != 0xffffffff)
        {
            /* this bit vector is written to swap. Now read needed bit vector line*/

            if (parentNodeDataPtr->funcCallCounter == 2)
            {
                /* update ptr from swap to new allocated gon*/
                switch (parentNodeDataPtr->bucketPtrArray[level]->bucketType)
                {
                case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
                case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
                case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
                case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
                    rc =  lpmFalconFillCompressedData(parentNodeDataPtr->bucketPtrArray[level],
                                                      shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                      &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_3)[0].compressedNodesArray[0]),
                                                      NULL);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "lpmFalconFillCompressedData fails");
                    }
                    break;
                case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
                    rc =  lpmFalconFillRegularData(parentNodeDataPtr->bucketPtrArray[level],
                                                   shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                   &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_3)[0].regularNodesArray[0]));
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "lpmFalconFillRegularData fails");
                    }
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
                }

                /* now write gon into new allocated memory*/
                for (i = 0; i < shareDevListLen; i++)
                {
                    rc =  prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[i],
                                                          tempAddr,
                                                          &(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(groupOfNodes_3)[0]),
                                                          &groupOfNodesLines);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChLpmGroupOfNodesWrite");
                    }
                 }

            }
            else if (parentNodeDataPtr->funcCallCounter == 1)
            {
                for (i = 0; i < shareDevListLen; i++)
                {

                    rc = prvCpssDxChReadTableMultiEntry(shareDevsList[i],
                                                         CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                         tempAddr+j,
                                                         1,
                                                         gonDataArray);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "LPM read fails");
                    }
                    /* change ptr : taken from swapGonsAdresses */
                    /* check node types: 3-3, 2-2 with ptr =0 can't be */

                    U32_SET_FIELD_IN_ENTRY_MAC(gonDataArray,0,20,parentNodeDataPtr->swapGonsAdresses[j]);
                    /* Write the node */
                    rc = prvCpssDxChSip6RamMngWriteMultiEntry(shareDevsList[i],
                                                         CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                         tempAddr+j,
                                                         1,
                                                         gonDataArray);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "LPM write fails");
                    }
                }
            }
        }
    }

    return GT_OK;
}


/**
* @internal updateCurrentAndParentGons function
* @endinternal
*
* @brief This function updates lpm gons
*
* @param[in]  parentNodeDataPtr     - pointer to the bucket to parent node parameters.
*
* @retval   GT_OK on success, or
* @retval   GT_OUT_OF_CPU_MEM - if failed to allocate CPU memory, or
* @retval   GT_OUT_OF_PP_MEM - if failed to allocate PP memory.
*/
static GT_STATUS updateCurrentAndParentGons
(
    IN CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT          newNodeType,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC      *currentBucketPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC        *parentNodeDataPtr,
    IN PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC   *groupOfNodesPtr,
    IN GT_U32                                        subnodeId
)
{
     GT_STATUS   rc = GT_OK;
     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr = NULL;
     GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
     GT_U32 shareDevListLen;
     GT_U32 i = 0;
     GT_U32 gonOffset = 0;
     GT_U32 level  = 0;
     GT_U32 swapOffset = 0xffffffff;
     GT_U32 funcCallCounter = 0;
     GT_BOOL addOperation = GT_FALSE;
     GT_BOOL isBvCompConvertion = GT_FALSE;
     CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT oldNodeType;
     GT_U32 groupOfNodesLines;

     shadowPtr = parentNodeDataPtr->shadowPtr;
     level = parentNodeDataPtr->level;
     shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
     shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;
     oldNodeType = currentBucketPtr->bucketType;
     currentBucketPtr->bucketType = newNodeType;
     /* put current GON to SWAP3*/
     if (parentNodeDataPtr->bucketPtrArray[level]->hwGroupOffsetHandle[subnodeId]==0)
     {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected parentNodeDataPtr->bucketPtrArray[level]->hwGroupOffsetHandle[subnodeId]=0\n");
     }
     gonOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(parentNodeDataPtr->bucketPtrArray[level]->hwGroupOffsetHandle[subnodeId]);
     if (shadowPtr->thirdSwapMemoryAddr==0)
     {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected shadowPtr->thirdSwapMemoryAddr=0\n");
     }
     swapOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->thirdSwapMemoryAddr);
     if (parentNodeDataPtr->nhUpdateOnly == GT_FALSE)
     {
         rc = updateSwapArea(subnodeId, currentBucketPtr->hwGroupOffsetHandle,0xffffffff,0,
                             swapOffset,shareDevsList,shareDevListLen,parentNodeDataPtr->swapGonsAdresses);
         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "updateSwapArea failed");
         }
         /* change ptr to SWAP */
         funcCallCounter = parentNodeDataPtr->funcCallCounter;
         addOperation = parentNodeDataPtr->addOperation;
         isBvCompConvertion = parentNodeDataPtr->isBvCompConvertion;
         parentNodeDataPtr->addOperation = GT_FALSE;
         parentNodeDataPtr->funcCallCounter=1;
         parentNodeDataPtr->isBvCompConvertion = GT_FALSE;
         rc = updateHwRangeDataAndGonPtr(parentNodeDataPtr);
         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "updateHwRangeDataAndGonPtr failed");
         }
     }
     /* update original*/
     for (i = 0; i < shareDevListLen; i++)
     {
         rc =  prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[i],
                                               gonOffset,
                                               groupOfNodesPtr,
                                               &groupOfNodesLines);
         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChLpmGroupOfNodesWrite failed");
         }

     }
     if (parentNodeDataPtr->nhUpdateOnly == GT_TRUE)
     {
         return rc;
     }
     /* update parent and change ptr from swap to updated node */
     parentNodeDataPtr->funcCallCounter=2;
     rc = updateHwRangeDataAndGonPtr(parentNodeDataPtr);
     if (rc != GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "updateHwRangeDataAndGonPtr failed");
     }
     parentNodeDataPtr->funcCallCounter = funcCallCounter;
     parentNodeDataPtr->addOperation = addOperation;
     parentNodeDataPtr->isBvCompConvertion = isBvCompConvertion;
     currentBucketPtr->bucketType = oldNodeType;
     return rc;
}




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
)
{
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC       **currRange,*rangeMem[MAX_LPM_LEVELS_CNS*2];
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC **currLpmEnginePtr;
    GT_BOOL forceWriteWholeBucket,lastLevelBucketEmbLeaves;
    LPM_BUCKET_UPDATE_MODE_ENT bucketUpdateMode;
    GT_BOOL isDestTreeRootBucket;
    PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC  parentUpdateParams;
    GT_U32 i = 0;
    GT_BOOL subnodesIndexes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];
    GT_U32  subnodesSizes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_BOOL isSrcTreeRootBucket = GT_FALSE;
    LPM_ROOT_BUCKET_UPDATE_ENT  rootBucketUpdateState = LPM_ROOT_BUCKET_UPDATE_NONE_E;

    cpssOsMemSet(&parentUpdateParams,0,sizeof(parentUpdateParams));
    parentUpdateParams.vrId = vrId;
    parentUpdateParams.protocol = protocolStack;
    parentUpdateParams.shadowPtr = shadowPtr;

    /* intialize the range memory */
    rangeMem[0] = rootRangePtr;

    /* start with the first level */
    lpmEngineMemPtrPtr++;
    currLpmEnginePtr = lpmEngineMemPtrPtr;
    currRange = rangeMem;

    forceWriteWholeBucket =
        (updateType == PRV_CPSS_DXCH_LPM_RAM_TRIE_WRITE_HW_AND_ALLOC_MEM_E)?
        GT_TRUE : GT_FALSE;

    bucketUpdateMode =
        (updateType == PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_HW_AND_ALLOC_MEM_E)?
        LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E:
        LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E;

    for ( i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
    {
        subnodesIndexes[i] = GT_TRUE;
    }
    cpssOsMemSet(subnodesSizes,0,sizeof(subnodesSizes));

    while(currRange >= rangeMem)
    {
        if ((*currRange) == NULL)
        {
            /* this means that we finished with this level - move back up a level*/
            currRange--;
            currLpmEnginePtr--;
            if (currRange >= rangeMem)
            {

                if (((updateType == PRV_CPSS_DXCH_LPM_RAM_TRIE_WRITE_HW_AND_ALLOC_MEM_E) ||
                     /* in update mode, write only if this bucket needs update */
                     ((updateType != PRV_CPSS_DXCH_LPM_RAM_TRIE_WRITE_HW_AND_ALLOC_MEM_E) &&
                      (((*currRange)->lowerLpmPtr.nextBucket->bucketHwUpdateStat
                        != PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E) ||
                       ((*currRange)->lowerLpmPtr.nextBucket->bucketHwUpdateStat
                        != PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_UPDATE_SIBLINGS_E)))))
                {
                    isSrcTreeRootBucket = GT_FALSE;
                    isDestTreeRootBucket = GT_FALSE;
                    rootBucketUpdateState = LPM_ROOT_BUCKET_UPDATE_NONE_E;
                    lastLevelBucketEmbLeaves = GT_FALSE;
                    isDestTreeRootBucket = (currLpmEnginePtr == lpmEngineMemPtrPtr) ? GT_TRUE : GT_FALSE;
                    if (isDestTreeRootBucket == GT_FALSE)
                    {
                        if ((*currRange)->lowerLpmPtr.nextBucket->hwBucketOffsetHandle !=0 )
                        {
                            isSrcTreeRootBucket = GT_TRUE;
                        }
                    }
                    if ((isDestTreeRootBucket == GT_TRUE) || (isSrcTreeRootBucket == GT_TRUE))
                    {
                        rootBucketUpdateState = LPM_ROOT_BUCKET_UPDATE_HW_E;
                    }
                    /* now that we finished the lower levels , write this bucket */
                    if ((*currRange)->lowerLpmPtr.nextBucket->bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                    {
                        if ((*currRange)->lowerLpmPtr.nextBucket->numOfRanges <= 3)
                        {
                            lastLevelBucketEmbLeaves = lpmFalconCheckIfLastLevelBucketEmbdLeaves((*currRange)->lowerLpmPtr.nextBucket);
                        }
                    }
                    if (lastLevelBucketEmbLeaves == GT_TRUE)
                    {
                        bucketUpdateMode = LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E;
                    }
                    else
                    {
                        bucketUpdateMode =
                            (updateType == PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_HW_AND_ALLOC_MEM_E)?
                            LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E:
                            LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E;

                    }
                    if (forceWriteWholeBucket == GT_FALSE)
                    {
                        /*we are going to deal with memory allocation and deallocation*/
                        retVal = lpmFalconGetStaticGonsSizes((*currRange)->lowerLpmPtr.nextBucket,
                                                             &subnodesSizes[0]);
                        if (retVal != GT_OK)
                        {
                            return retVal;
                        }
                    }
                    if ((*currRange)->lowerLpmPtr.nextBucket->numOfRanges > 1)
                        retVal = updateMirrorGroupOfNodes((*currRange)->lowerLpmPtr.nextBucket,
                                                           bucketUpdateMode,
                                                           isDestTreeRootBucket,
                                                           isSrcTreeRootBucket,
                                                           rootBucketUpdateState,
                                                           GT_FALSE,
                                                           GT_FALSE,
                                                           GT_FALSE,
                                                           forceWriteWholeBucket,
                                                           GT_FALSE,
                                                           currLpmEnginePtr,
                                                           &(*currRange)->pointerType,
                                                           shadowPtr,
                                                           &parentUpdateParams,
                                                           &subnodesIndexes[0],
                                                           &subnodesSizes[0]);

                    if (retVal != GT_OK)
                    {
                        return retVal;
                    }
                    else
                    {
                        /* update partition block */
                        if ((*currRange)->lowerLpmPtr.nextBucket->hwBucketOffsetHandle!=0)
                        {
                            SET_DMM_BLOCK_PROTOCOL((*currRange)->lowerLpmPtr.nextBucket->hwBucketOffsetHandle, protocolStack);
                            SET_DMM_BUCKET_SW_ADDRESS((*currRange)->lowerLpmPtr.nextBucket->hwBucketOffsetHandle, (*currRange)->lowerLpmPtr.nextBucket);
                        }
                    }
                }
                else if ((updateType != PRV_CPSS_DXCH_LPM_RAM_TRIE_WRITE_HW_AND_ALLOC_MEM_E) ||
                         (currRange == rangeMem))
                {
                    /* it could be that it's BUCKET_HW_UPDATE_SIBLINGS we need
                       to reset it */
                    (*currRange)->lowerLpmPtr.nextBucket->bucketHwUpdateStat
                        = PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E;
                }

                (*currRange) = (*currRange)->next;
            }
        }
         else if (((*currRange)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                 ((*currRange)->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
                 /* if we're in "update only" mode then check if this next bucket
                    or it's siblings need update */
                 ((updateType != PRV_CPSS_DXCH_LPM_RAM_TRIE_WRITE_HW_AND_ALLOC_MEM_E) &&
                  ((*currRange)->lowerLpmPtr.nextBucket->bucketHwUpdateStat ==
                   PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E)))
        {
            (*currRange) = (*currRange)->next;
        }
        else
        {
            /* this means this range has lower levels. go and explore */
            currRange[1] = (*currRange)->lowerLpmPtr.nextBucket->rangeList;

            currRange++;
            currLpmEnginePtr++;
        }
    }

    return retVal;
}

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
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      numOfWords, currWord;
    GT_U32      regData;

    /* 1 or 2 AAC channels are used for LPM, each 32 words */
    if(numWordsToWrite > PRV_CPSS_DXCH_AAC_USED_FOR_LPM_CNS*PRV_CPSS_DXCH_AAC_MAX_NUM_OF_TRANSACTION_WORDS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* It is enough to check first */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEnginesStatus;
    rc = prvCpssPortGroupBusyWait(devNum, 0xFFFFFFFF, regAddr,
                                  PRV_CPSS_DXCH_AAC_CHANNEL_LPM_MANAGER_1_E, GT_FALSE/*busyWait*/);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* AAC Engine <<%n>> Data for channel LPM */
    regAddr  = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineData[PRV_CPSS_DXCH_AAC_CHANNEL_LPM_MANAGER_1_E];
    numOfWords = (numWordsToWrite >= PRV_CPSS_DXCH_AAC_MAX_NUM_OF_TRANSACTION_WORDS) ? PRV_CPSS_DXCH_AAC_MAX_NUM_OF_TRANSACTION_WORDS : numWordsToWrite;
    for (currWord = 0; currWord < numOfWords; currWord++)
    {
        regData = entryValuePtr[currWord];
        /* Write words of data */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* AAC Engine <<%n>> Address for channel LPM */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineAddress[PRV_CPSS_DXCH_AAC_CHANNEL_LPM_MANAGER_1_E];
    regData = startAddress;

    /* Write address of LPM direct accsess data register */
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(numWordsToWrite <= PRV_CPSS_DXCH_AAC_MAX_NUM_OF_TRANSACTION_WORDS) {
        return GT_OK;
    }

    if(PRV_CPSS_DXCH_AAC_USED_FOR_LPM_CNS == 1) {
        return GT_OK;
    }

    /* AAC Engine <<%n>> Data for channel LPM */
    regAddr  = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineData[PRV_CPSS_DXCH_AAC_CHANNEL_LPM_MANAGER_2_E];
    numOfWords = numWordsToWrite - PRV_CPSS_DXCH_AAC_MAX_NUM_OF_TRANSACTION_WORDS;
    for (currWord = 0; currWord < numOfWords; currWord++)
    {
        regData = entryValuePtr[currWord + PRV_CPSS_DXCH_AAC_MAX_NUM_OF_TRANSACTION_WORDS];
        /* Write words of data */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* AAC Engine <<%n>> Address for channel LPM */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineAddress[PRV_CPSS_DXCH_AAC_CHANNEL_LPM_MANAGER_2_E];
    regData = startAddress + PRV_CPSS_DXCH_AAC_MAX_NUM_OF_TRANSACTION_WORDS * 4;

    /* Write address of LPM direct accsess data register */
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

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
)
{
    GT_STATUS                                   rc;
    GT_U32                                      regAddr;
    GT_U32                                      regVal;

    if (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 0)
    {
        /* AAC not used for devices with single tile */
        return GT_OK;
    }

    /* AAC global control */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACControl;
    /* AAC Enable */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, 1);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*  AAC Engine <<%n>> Control for channel LPM */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineControl[PRV_CPSS_DXCH_AAC_CHANNEL_LPM_MANAGER_1_E];
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &regVal);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* AAC address mode - direct */
    U32_SET_FIELD_MAC(regVal, 0, 1, 0);
    /* AAC multicast enable */
    U32_SET_FIELD_MAC(regVal, 2, 1, 1);

    rc = prvCpssHwPpWriteRegister(devNum, regAddr, regVal);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* AAC Engine <<%n>> MC External Offset for channel LPM */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineMCExternalOffset[PRV_CPSS_DXCH_AAC_CHANNEL_LPM_MANAGER_1_E];
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, FALCON_TILE_OFFSET_CNS);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*  AAC Engine <<%n>> MC Control for channel LPM */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineMCControl[PRV_CPSS_DXCH_AAC_CHANNEL_LPM_MANAGER_1_E];
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 10, PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*  AAC Engine <<%n>> Control for channel LPM */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineControl[PRV_CPSS_DXCH_AAC_CHANNEL_LPM_MANAGER_2_E];
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &regVal);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* AAC address mode - direct */
    U32_SET_FIELD_MAC(regVal, 0, 1, 0);
    /* AAC multicast enable */
    U32_SET_FIELD_MAC(regVal, 2, 1, 1);

    rc = prvCpssHwPpWriteRegister(devNum, regAddr, regVal);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* AAC Engine <<%n>> MC External Offset for channel LPM */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineMCExternalOffset[PRV_CPSS_DXCH_AAC_CHANNEL_LPM_MANAGER_2_E];
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, FALCON_TILE_OFFSET_CNS);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*  AAC Engine <<%n>> MC Control for channel LPM */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineMCControl[PRV_CPSS_DXCH_AAC_CHANNEL_LPM_MANAGER_2_E];
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 10, PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

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
)
{
    GT_U32  ii;         /* loop iterator            */
    GT_U32  entrySize;  /* entry size in words      */
    GT_U32  rc;         /* return code              */
    GT_U32  numWordsToWrite = 0;
    GT_U32  startAddress = 0;
    GT_U32 maxEntriesToWrite = 0;

    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *tablePtr;

    CPSS_NULL_PTR_CHECK_MAC(entryValueArrayPtr);
    if( 0 == numOfEntries )
        return GT_OK;

    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);
    tablePtr =(PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->writeTablePtr);

    /* calculate entry size in words*/
    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);
    entrySize = tableInfoPtr->entrySize;
    startAddress = tablePtr->baseAddress;

    maxEntriesToWrite = (PRV_CPSS_DXCH_AAC_USED_FOR_LPM_CNS*PRV_CPSS_DXCH_AAC_MAX_NUM_OF_TRANSACTION_WORDS)/entrySize;

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) && (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles > 1))
    {
        /* In case entry X+1 is in incremental memory to X use AAC to write up to 4
           entries using 2 AAC channels */
        if(tablePtr->step != 4 * entrySize) {
            maxEntriesToWrite = 1;
        }
        for( ii = startIndex ; ii < startIndex + numOfEntries ;  )
        {

            numWordsToWrite = (startIndex + numOfEntries - ii >= maxEntriesToWrite) ? maxEntriesToWrite : (startIndex + numOfEntries - ii);
            numWordsToWrite *= entrySize;

            startAddress = tablePtr->baseAddress + ii * tablePtr->step;
            rc = prvCpssDxChSip6LpmAacHwWriteEntry(devNum,entryValueArrayPtr, numWordsToWrite, startAddress);
            if (rc != GT_OK)
            {
                return rc;
            }

            entryValueArrayPtr+=(numWordsToWrite);

            ii += (numWordsToWrite/entrySize);
        }
    }
    else
    {
        for( ii = startIndex ; ii < startIndex + numOfEntries ; ii++ )
        {

            rc = prvCpssDxChWriteTableEntry(devNum,tableType,ii,entryValueArrayPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            entryValueArrayPtr+=entrySize;
        }
    }

    return GT_OK;
}
