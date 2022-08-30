/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChLpmMc.c
*
* DESCRIPTION:
*       This file includes functions for controlling the LPM Multicast
*       tables and structures.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 14 $
*
*******************************************************************************/

#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamMng.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip6LpmRamMng.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip6LpmRamDeviceSpecific.h>
#include <cpssCommon/private/prvCpssDevMemManager.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamUc.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamMc.h>
#include <cpssCommon/private/prvCpssSkipList.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChTables.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/****************************************************************************
* Debug                                                                     *
****************************************************************************/

extern void * cpssOsLpmMalloc
(
    IN GT_U32 size
);

extern void cpssOsLpmFree
(
    IN void* const memblock
);

/***************************************************************************
* defines
****************************************************************************/

/*
 * Typedef: struct SHADOW_IP_MC_GROUP_INSERT_FUNC_PARAMS_STC
 *
 * Description: A struct that holds the parameters for
 *              PRV_CPSS_DXCH_LPM_RAM_MC_GROUP_INSERT_FUNC_PTR
 *
 * Fields:
 *      see PRV_CPSS_DXCH_LPM_RAM_MC_GROUP_INSERT_FUNC_PTR
 */
typedef struct SHADOW_IP_MC_GROUP_INSERT_FUNC_PARAMS_STCT
{
    GT_U32                                          vrId;
    GT_U8                                           *ipGroup;
    GT_U32                                          ipGroupPrefix;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC         *srcBucketPtr;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                *shadowPtr;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT      insertMode;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT            protocol;
}SHADOW_IP_MC_GROUP_INSERT_FUNC_PARAMS_STC;

/**
* @internal prvCpssDxChLpmRamCheckAvailableMcMem function
* @endinternal
*
* @brief   Check the if there is availble memory for the Mc Group insertion.
*         (Group Search in LPM)
* @param[in] vrId                     - the Vr id to look in.
* @param[in] ipDestPtr                - the ip group address.
* @param[in] ipPrefix                 - the ip Group address prefix length.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] protocol                 - the protocol
* @param[in] insertMode               - how to insert the prefix
*                                      (see PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT)
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
static GT_STATUS prvCpssDxChLpmRamCheckAvailableMcMem
(
    IN GT_U32                                       vrId,
    IN GT_U8                                        *ipDestPtr,
    IN GT_U32                                       ipPrefix,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode,
    IN GT_BOOL                                      defragmentationEnable
)
{
    GT_STATUS   retVal = GT_OK;          /* Function return value.   */
    GT_U8       groupAddr[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS]; /* the group address */
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *rootBucketPtr;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmEngineMemPtr;
    GT_U8       numOfOctets;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC parentWriteFuncInfo;
    PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PARAMS_STC  writeFuncData;

    /* update the info for the parent write function */
    parentWriteFuncInfo.bucketParentWriteFunc =
        prvCpssDxChLpmRamUpdateVrTableFuncWrapper;

    writeFuncData.shadowPtr = shadowPtr;
    writeFuncData.protocol = protocol;
    writeFuncData.vrId = vrId;
    parentWriteFuncInfo.data = (GT_PTR)(&writeFuncData);

    lpmEngineMemPtr = shadowPtr->mcSearchMemArrayPtr[protocol];
    rootBucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol];

    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
    {
        numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
    }
    else
    {
        if(protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
        {
            numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
        }
        else
        {
            numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS;
        }
    }
    cpssOsMemCpy(groupAddr,ipDestPtr,sizeof(GT_U8)*numOfOctets);

    /* Check for group IP allocation. */
    retVal =
        prvCpssDxChLpmRamMngAllocAvailableMemCheck(rootBucketPtr,groupAddr,
                                                   ipPrefix,
                                                   PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E,
                                                   lpmEngineMemPtr,
                                                   shadowPtr,
                                                   protocol,
                                                   insertMode,
                                                   defragmentationEnable,
                                                   &parentWriteFuncInfo);
    return retVal;
}


/**
* @internal prvCpssDxChLpmSip6RamCheckAvailableMcMem function
* @endinternal
*
* @brief   Check the if there is availble memory for the Mc Group insertion.
*         (Group Search in LPM)
* @param[in] vrId                     - the Vr id to look in.
* @param[in] ipDestPtr                - the ip group address.
* @param[in] ipPrefix                 - the ip Group address prefix length.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] protocol                 - the protocol
* @param[in] insertMode               - how to insert the prefix
*                                      (see PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT)
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
* @param[in] parentUpdateParams       - paramaters needed for parent update
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
static GT_STATUS prvCpssDxChLpmSip6RamCheckAvailableMcMem
(
    IN GT_U32                                       vrId,
    IN GT_U8                                        *ipDestPtr,
    IN GT_U32                                       ipPrefix,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode,
    IN GT_BOOL                                      defragmentationEnable,
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC        *parentUpdateParams
)
{
    GT_STATUS   retVal = GT_OK;          /* Function return value.   */
    GT_U8       groupAddr[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS]; /* the group address */
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *rootBucketPtr;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmEngineMemPtr;
    GT_U8       numOfOctets;
    defragmentationEnable = defragmentationEnable;
    insertMode = insertMode;
    /* update the info for the parent write function */
    lpmEngineMemPtr = shadowPtr->mcSearchMemArrayPtr[protocol];
    rootBucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol];

    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
    {
        numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
    }
    else
    {
        if(protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
        {
            numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
        }
        else
        {
            numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS;
        }
    }
    cpssOsMemCpy(groupAddr,ipDestPtr,sizeof(GT_U8)*numOfOctets);

    /* Check for group IP allocation. */
    cpssOsMemSet(parentUpdateParams->swapGonsAdresses, 0xff, sizeof(parentUpdateParams->swapGonsAdresses));

    retVal =
        prvCpssDxChLpmSip6RamMngAllocAvailableMemCheck(rootBucketPtr,
                                                       groupAddr,
                                                       ipPrefix,
                                                       PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E,
                                                       lpmEngineMemPtr,
                                                       shadowPtr,
                                                       protocol,
                                                       PRV_CPSS_DXCH_LPM_ALLOC_MC_GR_TYPE_E,
                                                       insertMode,
                                                       defragmentationEnable,
                                                       parentUpdateParams);

    return retVal;
}


/**
* @internal checkAvailableMcSearchMemory function
* @endinternal
*
* @brief   Check if there is enough available memory in the RAM to insert a new
*         Multicast IP address.
* @param[in] bucketPtr                - The LPM bucket to check on the LPM insert.
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipDestPtr                - If isUnicast == GT_TRUE, holds the unicast IP address to
*                                      be inserted.
*                                      If isUnicast == GT_FALSE, holds the multicast group ip
*                                      address to be inserted.
* @param[in] ipPrefix                 - Holds the prefix length of ipDest.
* @param[in] ipSrcArr[]               - Valid only if isUnicast == GT_FALSE, holds the source ip
*                                      address of the inserted IP-Mc entry.
* @param[in] ipSrcPrefix              - Valid only if isUnicast == GT_FALSE, holds the ip address
*                                      prefix length of ipSrcArr.
* @param[in] rootBucketFlag           - Indicates the way to deal with a root bucket (if it is).
* @param[in] skipMcGroupCheck         - indicates whether to skip the MC group check or not.
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] insertMode               - how to insert the prefix
*                                      (see PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT)
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
* @param[in] parentWriteFuncPtr       a bucket's parent write function in case there is
*                                      a need to update the packet's parent ptr data and
*                                      the parent is not a LPM trie range.
*
* @retval GT_OK                    - If there is enough memory for the insertion.
* @retval GT_OUT_OF_PP_MEM         - otherwise.
*/
static GT_STATUS checkAvailableMcSearchMemory
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC      *bucketPtr,
    IN GT_U32                                       vrId,
    IN GT_U8                                        *ipDestPtr,
    IN GT_U32                                       ipPrefix,
    IN GT_U8                                        ipSrcArr[],
    IN GT_U32                                       ipSrcPrefix,
    IN PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT   rootBucketFlag,
    IN GT_BOOL                                      skipMcGroupCheck,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode,
    IN GT_BOOL                                      defragmentationEnable,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC   *parentWriteFuncPtr
)
{
    GT_STATUS   retVal = GT_OK;             /* Function return value.   */
    GT_STATUS   retVal2 = GT_OK;

    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC                  **lpmEngineMemPtr;
    GT_U32 i;

    lpmEngineMemPtr = shadowPtr->ucSearchMemArrayPtr[protocolStack];
    shadowPtr->neededMemoryListLen = 0;
    shadowPtr->neededMemoryCurIdx = 0;

    /* Check for source IP allocation. */
    retVal = prvCpssDxChLpmRamMngAllocAvailableMemCheck(bucketPtr,ipSrcArr,ipSrcPrefix,
                                                        rootBucketFlag,lpmEngineMemPtr,
                                                        shadowPtr,
                                                        protocolStack,
                                                        insertMode,
                                                        defragmentationEnable,
                                                        parentWriteFuncPtr);

    /* Source IP allocation succeeded, check mc-tree needs.       */
    if((retVal == GT_OK) && (skipMcGroupCheck == GT_FALSE))
    {
        retVal =
            prvCpssDxChLpmRamCheckAvailableMcMem(vrId,ipDestPtr,ipPrefix,
                                                 shadowPtr,
                                                 protocolStack,
                                                 insertMode,
                                                 defragmentationEnable);
        /* if we failed the mc allocation release the uc allocation */
        if (retVal != GT_OK)
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            for (i = 0 ; i < shadowPtr->neededMemoryListLen; i++)
            {
                prvCpssDmmFree(shadowPtr->neededMemoryBlocks[i]);
            }

            /* free the allocated/bound RAM memory */
            retVal2 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFree(shadowPtr,protocolStack);
            if (retVal2!=GT_OK)
            {
                return retVal2;
            }
            /* in case of fail we will need to reconstruct to the status we had before the call to prvCpssDxChLpmRamMngAllocAvailableMemCheck */
            cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                         sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
        }

        /* in order not to loose the retVal in case of out of Group memory the
        Revert of the ListMng will be on retVal2 */

        /* release all the checked memory if there was any allocated*/
        retVal2 = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                                  &shadowPtr->freeMemListDuringUpdate,shadowPtr);

        if(retVal2 != GT_OK)
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            return retVal2;
        }
    }
    else
    {
        if (retVal != GT_OK)
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

        }
    }

    return retVal;
}


/**
* @internal checkSip6AvailableMcSearchMemory function
* @endinternal
*
* @brief   Check if there is enough available memory in the RAM to insert a new
*         Multicast IP address.
* @param[in] bucketPtr                - The LPM bucket to check on the LPM insert.
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipDestPtr                - If isUnicast == GT_TRUE, holds the unicast IP address to
*                                      be inserted.
*                                      If isUnicast == GT_FALSE, holds the multicast group ip
*                                      address to be inserted.
* @param[in] ipPrefix                 - Holds the prefix length of ipDest.
* @param[in] ipSrcArr[]               - Valid only if isUnicast == GT_FALSE, holds the source ip
*                                      address of the inserted IP-Mc entry.
* @param[in] ipSrcPrefix              - Valid only if isUnicast == GT_FALSE, holds the ip address
*                                      prefix length of ipSrcArr.
* @param[in] rootBucketFlag           - Indicates the way to deal with a root bucket (if it is).
* @param[in] skipMcGroupCheck         - indicates whether to skip the MC group check or not.
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] insertMode               - how to insert the prefix
*                                      (see PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT)
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
* @param[in] parentUpdateParams       - paramaters needed for parent update
*
* @retval GT_OK                    - If there is enough memory for the insertion.
* @retval GT_OUT_OF_PP_MEM         - otherwise.
*/
static GT_STATUS checkSip6AvailableMcSearchMemory
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC      *bucketPtr,
    IN GT_U32                                       vrId,
    IN GT_U8                                        *ipDestPtr,
    IN GT_U32                                       ipPrefix,
    IN GT_U8                                        ipSrcArr[],
    IN GT_U32                                       ipSrcPrefix,
    IN PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT   rootBucketFlag,
    IN GT_BOOL                                      skipMcGroupCheck,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode,
    IN GT_BOOL                                      defragmentationEnable,
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC        *parentUpdateParams
)
{
    GT_STATUS   retVal = GT_OK;             /* Function return value.   */
    GT_STATUS   retVal2 = GT_OK;

    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC                  **lpmEngineMemPtr;
    GT_U32 i;

    lpmEngineMemPtr = shadowPtr->ucSearchMemArrayPtr[protocolStack];
    shadowPtr->neededMemoryListLen = 0;
    shadowPtr->neededMemoryCurIdx = 0;

    /* Check for source IP allocation. */
    cpssOsMemSet(parentUpdateParams->swapGonsAdresses, 0xff, sizeof(parentUpdateParams->swapGonsAdresses));

    retVal = prvCpssDxChLpmSip6RamMngAllocAvailableMemCheck(bucketPtr,ipSrcArr,ipSrcPrefix,
                                                            rootBucketFlag,lpmEngineMemPtr,
                                                            shadowPtr,protocolStack,
                                                            PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E,
                                                            insertMode,
                                                            defragmentationEnable,
                                                            parentUpdateParams);
    /* Source IP allocation succeeded, check mc-tree needs.       */
    if((retVal == GT_OK) && (skipMcGroupCheck == GT_FALSE))
    {
        retVal =
            prvCpssDxChLpmSip6RamCheckAvailableMcMem(vrId,ipDestPtr,ipPrefix,
                                                     shadowPtr,
                                                     protocolStack,
                                                     insertMode,
                                                     defragmentationEnable,
                                                     parentUpdateParams);

        /* if we failed the mc allocation release the uc allocation */
        if (retVal != GT_OK)
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            for (i = 0 ; i < shadowPtr->neededMemoryListLen; i++)
            {
                if (shadowPtr->neededMemoryBlocks[i]!=0)
                {
                    prvCpssDmmFree(shadowPtr->neededMemoryBlocks[i]);
                }
            }

            /* free the allocated/bound RAM memory */
            retVal2 = prvCpssDxChLpmSip6RamMngAllocatedAndBoundMemFree(shadowPtr,protocolStack);
            if (retVal2!=GT_OK)
            {
                return retVal2;
            }
            /* in case of fail we will need to reconstruct to the status we had before the call to prvCpssDxChLpmRamMngAllocAvailableMemCheck */
            cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                         sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
        }

        /* in order not to loose the retVal in case of out of Group memory the
        Revert of the ListMng will be on retVal2 */

        /* release all the checked memory if there was any allocated*/
        retVal2 = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                                  &shadowPtr->freeMemListDuringUpdate,shadowPtr);

        if(retVal2 != GT_OK)
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            return retVal2;
        }
    }
    else
    {
        if (retVal != GT_OK)
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

        }
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmSip6RamMcDefRouteSet function
* @endinternal
*
* @brief   sets the default MC route for the given VR router.
*
* @param[in] vrId                     - The  to which this tree belongs to.
* @param[in] defMcRoutePointerPtr     - A pointer to the default mc route
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] insertMode               - the insert mode.
* @param[in] protocol                 - the protocol
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
static GT_STATUS prvCpssDxChLpmSip6RamMcDefRouteSet
(
    IN  GT_U32                                          vrId,
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       *defMcRoutePointerPtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                *shadowPtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT      insertMode,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT            protocol
)
{
    GT_STATUS retVal;

    GT_U8 groupAddr[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];
    GT_U32 ipGroupPrefix;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT rootBucketType; /* the root mc Bucket type */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmEngineMemPtr; /* the MC lpm
                                            Engine Memory array*/
    GT_BOOL                                  updateOldPtr = GT_FALSE;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT defMcRoutePtrType;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *rootBucketPtr;
    PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC  parentUpdateParams;

    rootBucketPtr =
        shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol];
    rootBucketType =
        shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol];

    lpmEngineMemPtr = shadowPtr->mcSearchMemArrayPtr[protocol];

    ipGroupPrefix =
        (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
        PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS :
        PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS;

    groupAddr[0] =
        (GT_U8)((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
        PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS :
        PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV6_MC_ADDRESS_SPACE_CNS);

    PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(defMcRoutePointerPtr->routeEntryMethod,defMcRoutePtrType);
    /* pointer is set to the first gon */
    lpmEngineMemPtr++;

    cpssOsMemSet(&parentUpdateParams,0,sizeof(parentUpdateParams));

    parentUpdateParams.vrId = vrId;
    parentUpdateParams.prefixLength = ipGroupPrefix;
    parentUpdateParams.protocol = protocol;
    parentUpdateParams.groupPtr = groupAddr;

     /* Insert the complete ip address to the lpm structure */
    retVal = prvCpssDxChLpmSip6RamMngInsert(rootBucketPtr, groupAddr,
                                            ipGroupPrefix, defMcRoutePointerPtr,
                                            defMcRoutePtrType,
                                            PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E,
                                            lpmEngineMemPtr,&updateOldPtr,&rootBucketType,
                                            shadowPtr, insertMode,
                                            PRV_CPSS_DXCH_LPM_ALLOC_MC_GR_TYPE_E,protocol,&parentUpdateParams);
    if(retVal == GT_OK)
    {
        /* Now update the ip-mc first table according   */
        /* to the first level of the lpm structure.     */
        shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol] =
            rootBucketType;

        /* Update the default route entry */
        shadowPtr->vrRootBucketArray[vrId].multicastDefault[protocol] =
            defMcRoutePointerPtr;

    }

    return retVal;
}


/**
* @internal prvCpssDxChLpmRamMcDefRouteSet function
* @endinternal
*
* @brief   sets the default MC route for the given VR router.
*
* @param[in] vrId                     - The  to which this tree belongs to.
* @param[in] defMcRoutePointerPtr     - A pointer to the default mc route
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] insertMode               - the insert mode.
* @param[in] protocol                 - the protocol
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
static GT_STATUS prvCpssDxChLpmRamMcDefRouteSet
(
    IN  GT_U32                                          vrId,
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       *defMcRoutePointerPtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                *shadowPtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT      insertMode,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT            protocol
)
{
    GT_STATUS retVal;

    GT_U8 groupAddr[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];
    GT_U32 ipGroupPrefix;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT rootBucketType; /* the root mc Bucket type */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmEngineMemPtr; /* the MC lpm
                                            Engine Memory array*/
    GT_BOOL                                  updateOldPtr = GT_FALSE;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT defMcRoutePtrType;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *rootBucketPtr;

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC parentWriteFuncInfo;
    PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PARAMS_STC  writeFuncData;
    CPSS_SYSTEM_RECOVERY_INFO_STC                           tempSystemRecovery_Info;
    GT_BOOL                                                 managerHwWriteBlock;

    /* update the info for the parent write function */
    parentWriteFuncInfo.bucketParentWriteFunc =
        prvCpssDxChLpmRamUpdateVrTableFuncWrapper;

    writeFuncData.shadowPtr = shadowPtr;
    writeFuncData.protocol = protocol;
    writeFuncData.vrId = vrId;
    parentWriteFuncInfo.data = (GT_PTR)(&writeFuncData);

    rootBucketPtr =
        shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol];
    rootBucketType =
        shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol];

    lpmEngineMemPtr = shadowPtr->mcSearchMemArrayPtr[protocol];

    ipGroupPrefix =
        (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
        PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS :
        PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS;

    groupAddr[0] =
        (GT_U8)((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
        PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS :
        PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV6_MC_ADDRESS_SPACE_CNS);

    PRV_CPSS_DXCH_LPM_NEXT_PTR_TYPE_CONVERT_MAC(defMcRoutePointerPtr->routeEntryMethod,defMcRoutePtrType);
     /* Insert the complete ip address to the lpm structure */
    retVal = prvCpssDxChLpmRamMngInsert(rootBucketPtr, groupAddr,
                                        ipGroupPrefix, defMcRoutePointerPtr,
                                        defMcRoutePtrType,
                                        PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E,
                                        lpmEngineMemPtr,&updateOldPtr,&rootBucketType,
                                        shadowPtr, insertMode, GT_FALSE,
                                        &parentWriteFuncInfo,protocol,PRV_CPSS_DXCH_LPM_ALLOC_MC_GR_TYPE_E);
    if(retVal == GT_OK)
    {
        /* Now update the ip-mc first table according   */
        /* to the first level of the lpm structure.     */
        shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol] =
            rootBucketType;

        /* Update the default route entry */
        shadowPtr->vrRootBucketArray[vrId].multicastDefault[protocol] =
            defMcRoutePointerPtr;

        retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        /* in case of shadow update don't touch the hw */
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if ((insertMode != PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E)&&
            (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
            (managerHwWriteBlock == GT_FALSE) )
        {
            retVal = prvCpssDxChLpmRamMngVrfEntryUpdate(vrId ,protocol, shadowPtr);
        }
    }

    return retVal;
}


/**
* @internal prvCpssDxChLpmRamMcGroupSearch function
* @endinternal
*
* @brief   searches the MC tree for the Given Mc Ip Group address and returns
*         the bucket pointer of the lpm stucture attached to it.
*         (Group Search in LPM)
* @param[in] vrId                     - The  to which this tree belongs to.
* @param[in] ipGroupPtr               - The ip Address to search for.
* @param[in] ipGroupPrefix            - The prefix length of ipGroupPtr.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] protocol                 - the protocol
*
* @param[out] srcBucketPtrPtr          - A pointer to the shadow bucket including the src. IP
*                                      prefixes.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
static GT_STATUS prvCpssDxChLpmRamMcGroupSearch
(
    IN  GT_U32                                    vrId,
    IN  GT_U8                                     *ipGroupPtr,
    IN  GT_U32                                    ipGroupPrefix,
    OUT PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   **srcBucketPtrPtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol
)
{
    GT_STATUS retVal;
    GT_U8 groupAddr[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS], numOfOctets;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *rootBucketPtr;
    rootBucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol];

    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
    {
        numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
    }
    else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
    {
        numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
    }
    cpssOsMemCpy(groupAddr,ipGroupPtr,sizeof(GT_U8)*numOfOctets);

    retVal = prvCpssDxChLpmRamMngSearch(rootBucketPtr,groupAddr,
                                        ipGroupPrefix,(GT_PTR*)srcBucketPtrPtr);
    return retVal;
}

/**
* @internal cpssDxChLpmRamMcGroupGetNext function
* @endinternal
*
* @brief   This function searchs the trie for the next valid group node.
*         The search is started from the given ipGroupPtr & ipGroupPrefixPtr.
*         (Group Search in LPM)
* @param[in] vrId                     - The  to which this table belongs to.
* @param[in] force                    - Indicates whether to find the next valid entry even
*                                      if the given ipGroupPtr and ipGroupPrefixPtr are not
*                                      valid.
* @param[in,out] ipGroupPtr               - IP address associated with the search start point.
* @param[in,out] ipGroupPrefixPtr         - prefix of ipGroupPtr.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] protocol                 - the protocol
* @param[in,out] ipGroupPtr               - The ip address associated with the next node.
* @param[in,out] ipGroupPrefixPtr         - The prefix associated with ipGroupPtr.
*
* @param[out] srcBucketPtrPtr          - A pointer to the src lpm base if found, NULL otherwise.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*
* @note 1. If the given node is the last node in the trie then the values of
*       ipGroupPtr and ipGroupPrefixPtr will not be changed, and
*       srcBucketPtrPtr will get NULL.
*       2. If force == GT_TRUE and (ipGroupPtr,ipGroupPrefixPtr) is invalid,
*       then this function will find the first valid node.
*
*/
static GT_STATUS cpssDxChLpmRamMcGroupGetNext
(
    IN    GT_U32                                   vrId,
    IN    GT_BOOL                                  force,
    INOUT GT_U8                                    *ipGroupPtr,
    INOUT GT_U32                                   *ipGroupPrefixPtr,
    OUT   PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  **srcBucketPtrPtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC         *shadowPtr,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT     protocol
)
{
    GT_STATUS retVal;

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *bucketPtr;/* Pointer to the LPM structure */
    GT_U8 groupAddr[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS]; /* this is the group address */
    GT_U8 numOfOctets, i;
    GT_U8 mask;
    GT_U8* groupAddrPtr = (GT_U8*)groupAddr;
    GT_U8 remainder;

    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
    {
        numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
    }
    else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
    {
        numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
    }
    cpssOsMemCpy(groupAddr,ipGroupPtr,sizeof(GT_U8)*numOfOctets);

    /* truncate only the prefix part out of the ip address */
    remainder = (GT_U8)*ipGroupPrefixPtr;
    for (i = 0; i < numOfOctets; i++)
    {
        if (*ipGroupPrefixPtr == 0)
        {
            mask = 0;
        }
        else if (remainder >= 8)
        {
            mask = 0xFF;
        }
        else
        {
            mask = (GT_U8)(0xFF << (8 - remainder));
        }
        *groupAddrPtr = (GT_U8) (*groupAddrPtr & mask);
        groupAddrPtr++;
        if (remainder >= 8)
        {
            remainder = (GT_U8)(*ipGroupPrefixPtr - 8);
        }
        else
        {
            remainder = 0;
        }
    }

    bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol];

    if (force == GT_FALSE)
    {
        /* in this case the ip has to be valid */
        retVal = prvCpssDxChLpmRamMngSearch(bucketPtr,groupAddr,
                                            *ipGroupPrefixPtr,
                                            (GT_PTR*)srcBucketPtrPtr);
        if (retVal != GT_OK)
        {
            return retVal;
        }
    }

    retVal = prvCpssDxChLpmRamMngEntryGet(bucketPtr,protocol,groupAddr,
                                          ipGroupPrefixPtr,(GT_PTR*)srcBucketPtrPtr);

    if ((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) &&
        (groupAddr[0] >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS))
    {
        return /* do not log this error */ GT_NOT_FOUND;
    }

    cpssOsMemCpy(ipGroupPtr,groupAddr,sizeof(GT_U8)*numOfOctets);

    return (retVal);
}

/**
* @internal prvCpssDxChLpmSip6RamMcGroupInsert function
* @endinternal
*
* @brief   Insert an Ip Group Address to the ipv4 mc Tree.
*         (Group Search in LPM)
* @param[in] vrId                     - The  to which this table belongs to.
* @param[in] ipGroupPtr               - The ip group Address associated with the new inserted prefix.
* @param[in] ipGroupPrefix            - The prefix length of the new inserted prefix.
* @param[in] srcBucketPtr             - A pointer to the shadow bucket including the src. IP prefixes.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] insertMode               - the insert mode
* @param[in] protocol                 - the protocol
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
static GT_STATUS prvCpssDxChLpmSip6RamMcGroupInsert
(
    IN GT_U32                                           vrId,
    IN GT_U8                                            *ipGroupPtr,
    IN GT_U32                                           ipGroupPrefix,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC          *srcBucketPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                 *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT       insertMode,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT            protocol
)
{
    GT_STATUS retVal;

    GT_U8 groupAddr[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS]; /* this is the group address */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      **lpmEngineMemPtr; /* the MC lpm Engine Memory array*/
    PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC   parentUpdateParams;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *rootBucketPtr;       /* the root Mc bucket */
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     rootBucketType;  /* the root mc Bucket type */
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     origRootBucketType;
    GT_U32                                  origRootHwBucketOffset = 0xffffffff;
    GT_BOOL                                 updateOldPtr = GT_FALSE;
    GT_U8                                   numOfOctets;
    CPSS_SYSTEM_RECOVERY_INFO_STC           tempSystemRecovery_Info;
    GT_BOOL                                 managerHwWriteBlock;

    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
    {
        numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
    }
    else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
    {
        numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
    }
    cpssOsMemCpy(groupAddr,ipGroupPtr,sizeof(GT_U8)*numOfOctets);

    rootBucketPtr          = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol];
    rootBucketType         = shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol];
    origRootBucketType     = shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol];
    if (rootBucketPtr->hwBucketOffsetHandle != 0)
    {
        origRootHwBucketOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(rootBucketPtr->hwBucketOffsetHandle);
    }
    lpmEngineMemPtr        = shadowPtr->mcSearchMemArrayPtr[protocol];

    /* pointer is set to the first gon */
    lpmEngineMemPtr++;

    cpssOsMemSet(&parentUpdateParams,0,sizeof(parentUpdateParams));

    parentUpdateParams.vrId = vrId;
    parentUpdateParams.prefixLength = ipGroupPrefix;
    parentUpdateParams.protocol = protocol;
    parentUpdateParams.groupPtr = ipGroupPtr;

     /* Insert the complete ip address to the lpm structure */
    retVal = prvCpssDxChLpmSip6RamMngInsert(rootBucketPtr, groupAddr,
                                            ipGroupPrefix, srcBucketPtr,
                                            PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E,
                                            PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E,
                                            lpmEngineMemPtr,&updateOldPtr,&rootBucketType,
                                            shadowPtr, insertMode, PRV_CPSS_DXCH_LPM_ALLOC_MC_GR_TYPE_E,
                                            protocol,&parentUpdateParams);

    if(retVal == GT_OK)
    {
        /* Now update the ip-mc first table according   */
        /* to the first level of the lpm structure.     */
        shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol] =
            rootBucketType;

        retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        /* in case of shadow update don't touch the hw */
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if ((insertMode != PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E) &&
            (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
            (managerHwWriteBlock == GT_FALSE) &&
            (((shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol]->hwBucketOffsetHandle!=0)&&
              (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(
                 shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol]->hwBucketOffsetHandle) != origRootHwBucketOffset)) ||
             (origRootBucketType != rootBucketType)))
        {
            retVal = prvCpssDxChLpmRamMngVrfEntryUpdate(vrId ,protocol, shadowPtr);
        }
    }
    return retVal;
}


/**
* @internal prvCpssDxChLpmRamMcGroupInsert function
* @endinternal
*
* @brief   Insert an Ip Group Address to the ipv4 mc Tree.
*         (Group Search in LPM)
* @param[in] vrId                     - The  to which this table belongs to.
* @param[in] ipGroupPtr               - The ip group Address associated with the new inserted prefix.
* @param[in] ipGroupPrefix            - The prefix length of the new inserted prefix.
* @param[in] srcBucketPtr             - A pointer to the shadow bucket including the src. IP prefixes.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] insertMode               - the insert mode
* @param[in] protocol                 - the protocol
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
static GT_STATUS prvCpssDxChLpmRamMcGroupInsert
(
    IN GT_U32                                           vrId,
    IN GT_U8                                            *ipGroupPtr,
    IN GT_U32                                           ipGroupPrefix,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC          *srcBucketPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                 *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT       insertMode,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT            protocol
)
{
    GT_STATUS retVal;

    GT_U8 groupAddr[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS]; /* this is the group address */
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *rootBucketPtr;       /* the root Mc bucket */
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT rootBucketType;  /* the root mc Bucket type */
    GT_BOOL                                  updateOldPtr = GT_FALSE;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmEngineMemPtr; /* the MC lpm Engine Memory
                                                     array*/
    GT_U8 numOfOctets;

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC parentWriteFuncInfo;
    PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PARAMS_STC writeFuncData;
    CPSS_SYSTEM_RECOVERY_INFO_STC                           tempSystemRecovery_Info;
    GT_BOOL                                                 managerHwWriteBlock;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT                     origRootBucketType;
    GT_U32                                                  origRootHwBucketOffset = 0xffffffff;

    /* update the info for the parent write function */
    parentWriteFuncInfo.bucketParentWriteFunc =
        prvCpssDxChLpmRamUpdateVrTableFuncWrapper;

    writeFuncData.shadowPtr = shadowPtr;
    writeFuncData.protocol = protocol;
    writeFuncData.vrId = vrId;
    parentWriteFuncInfo.data = (GT_PTR)(&writeFuncData);

    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
    {
        numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
    }
    else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
    {
        numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
    }
    cpssOsMemCpy(groupAddr,ipGroupPtr,sizeof(GT_U8)*numOfOctets);

    lpmEngineMemPtr = shadowPtr->mcSearchMemArrayPtr[protocol];

    rootBucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol];
    rootBucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol];
    origRootBucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol];
    if (rootBucketPtr->hwBucketOffsetHandle != 0)
    {
        origRootHwBucketOffset = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(rootBucketPtr->hwBucketOffsetHandle);
    }

     /* Insert the complete ip address to the lpm structure */
    retVal = prvCpssDxChLpmRamMngInsert(rootBucketPtr, groupAddr,
                                        ipGroupPrefix, srcBucketPtr,
                                        PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E,
                                        PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E,
                                        lpmEngineMemPtr,&updateOldPtr,&rootBucketType,
                                        shadowPtr, insertMode, GT_FALSE,
                                        &parentWriteFuncInfo,protocol,PRV_CPSS_DXCH_LPM_ALLOC_MC_GR_TYPE_E);
    if(retVal == GT_OK)
    {
        /* Now update the ip-mc first table according   */
        /* to the first level of the lpm structure.     */
        shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol] =
            rootBucketType;

        retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal != GT_OK)
        {
            return retVal;
        }

        /* in case of shadow update don't touch the hw */
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if ((insertMode != PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E) &&
            (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
            (managerHwWriteBlock == GT_FALSE) &&
            (((shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol]->hwBucketOffsetHandle!=0)&&
             (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(
                 shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol]->hwBucketOffsetHandle) != origRootHwBucketOffset)) ||
             (origRootBucketType != rootBucketType)))
        {
            retVal = prvCpssDxChLpmRamMngVrfEntryUpdate(vrId, protocol, shadowPtr);
        }
        else
        {
            /* hwBucketOffsetHandle can be 0 only incase of HA process */
            if  ((insertMode != PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E) &&
                 (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
                 (managerHwWriteBlock == GT_FALSE) &&
                 (shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol]->hwBucketOffsetHandle==0))

            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Unexpected case:shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol]->hwBucketOffsetHandle==0,"
                                                        "for porotocol=%d, vrId=%d \n",protocol,vrId);
            }
        }
    }
    return retVal;
}


/**
* @internal prvCpssDxChLpmSip6RamMcGroupDelete function
* @endinternal
*
* @brief   Deletes an Ip Group Address from the ipv4 mc Tree.
*         (Group Search in LPM)
* @param[in] vrId                     - The  to which this table belongs to.
* @param[in] ipGroupPtr               - The ip group Address associated with the new inserted prefix.
* @param[in] ipGroupPrefix            - The prefix length of the new inserted prefix.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] protocol                 - the protocol.
*                                      roollBack                 - GT_TRUE: rollback is taking place.
*                                      GT_FALSE: otherwise.
*
* @param[out] srcBucketPtrPtr          - A pointer to the shadow bucket including the src. IP
*                                      prefixes.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
static GT_STATUS prvCpssDxChLpmSip6RamMcGroupDelete
(
    IN GT_U32                                     vrId,
    IN GT_U8                                      *ipGroupPtr,
    IN GT_U32                                     ipGroupPrefix,
    OUT PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   **srcBucketPtrPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC           *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT       protocol,
    IN GT_BOOL                                    rollBack
)
{
    GT_STATUS                                           retVal,retVal2;
    PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT            lpmPtr; /* Pointer to the LPM structure to delete from.*/
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT                 rootBucketType; /* the root mc Bucket type */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC                  **lpmEngineMemPtr; /* the MC lpm Engine Memory  array*/
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];
    PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC               parentUpdateParams;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT                 origRootBucketType;
    GT_U32                                              origRootHwBucketOffset = 0xffffffff;
    GT_U8                                               numOfOctets;
    GT_U8 groupAddr[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS]; /* this is the group addres */

    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));
    /* update the info for the parent write function */
    lpmEngineMemPtr = shadowPtr->mcSearchMemArrayPtr[protocol];

    lpmPtr.nextBucket      = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol];
    origRootBucketType     = shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol];
    if (lpmPtr.nextBucket->hwBucketOffsetHandle != 0)
    {
        origRootHwBucketOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(lpmPtr.nextBucket->hwBucketOffsetHandle);
    }

    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
    {
        numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
    }
    else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
    {
        numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
    }
    cpssOsMemCpy(groupAddr, ipGroupPtr, sizeof(GT_U8)* numOfOctets);
    cpssOsMemSet(&parentUpdateParams,0,sizeof(parentUpdateParams));

    parentUpdateParams.vrId = vrId;
    parentUpdateParams.prefixLength = ipGroupPrefix;
    parentUpdateParams.protocol = protocol;
    parentUpdateParams.groupPtr = ipGroupPtr;

    rootBucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol];
    retVal = prvCpssDxChLpmSip6RamMngEntryDelete(&lpmPtr,groupAddr,ipGroupPrefix,
                                                 PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E,GT_TRUE,
                                                 lpmEngineMemPtr,&rootBucketType,
                                                 (GT_PTR*)srcBucketPtrPtr,shadowPtr,
                                                 PRV_CPSS_DXCH_LPM_ALLOC_MC_GR_TYPE_E,
                                                 rollBack,protocol,&parentUpdateParams);
    if(retVal == GT_OK)
    {
        shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol] = rootBucketType;

        if (shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol]->hwBucketOffsetHandle==0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol]->hwBucketOffsetHandle=0\n");
        }
        if ((PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(
                 shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol]->hwBucketOffsetHandle) != origRootHwBucketOffset) ||
             (origRootBucketType != rootBucketType))
        {
            retVal = prvCpssDxChLpmRamMngVrfEntryUpdate(vrId, protocol, shadowPtr);
        }

    }

    /* keep the pendingBlockToUpdateArr. in case group release fail need to return to previuos prnding list updated for src  */
    cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,
                 sizeof(shadowPtr->pendingBlockToUpdateArr));

    retVal2 = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                              &shadowPtr->freeMemListEndOfUpdate,shadowPtr);

    if (retVal2!=GT_OK)
    {
        cpssOsMemCpy(shadowPtr->pendingBlockToUpdateArr,tempPendingBlockToUpdateArr,
                     sizeof(shadowPtr->pendingBlockToUpdateArr));
    }

    if(retVal==GT_OK)
    {
        return retVal2;
    }

    return retVal;
}



/**
* @internal prvCpssDxChLpmRamMcGroupDelete function
* @endinternal
*
* @brief   Deletes an Ip Group Address from the ipv4 mc Tree.
*         (Group Search in LPM)
* @param[in] vrId                     - The  to which this table belongs to.
* @param[in] ipGroupPtr               - The ip group Address associated with the new inserted prefix.
* @param[in] ipGroupPrefix            - The prefix length of the new inserted prefix.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] protocol                 - the protocol.
*                                      roollBack                 - GT_TRUE: rollback is taking place.
*                                      GT_FALSE: otherwise.
*
* @param[out] srcBucketPtrPtr          - A pointer to the shadow bucket including the src. IP
*                                      prefixes.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
static GT_STATUS prvCpssDxChLpmRamMcGroupDelete
(
    IN GT_U32                                     vrId,
    IN GT_U8                                      *ipGroupPtr,
    IN GT_U32                                     ipGroupPrefix,
    OUT PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   **srcBucketPtrPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC           *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT       protocol,
    IN GT_BOOL                                    rollBack
)
{
    GT_STATUS retVal,retVal2;
    GT_U8     numOfOctets;

    PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT lpmPtr; /* Pointer to the LPM structure to */
                                                     /* delete from.                    */

    GT_U8 groupAddr[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS]; /* this is the group addres */
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT rootBucketType; /* the root mc Bucket type */

    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmEngineMemPtr; /* the MC lpm Engine Memory
                                                     array*/
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC parentWriteFuncInfo;

    PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PARAMS_STC writeFuncData;

    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));

    /* update the info for the parent write function */
    parentWriteFuncInfo.bucketParentWriteFunc =
        prvCpssDxChLpmRamUpdateVrTableFuncWrapper;

    writeFuncData.shadowPtr = shadowPtr;
    writeFuncData.protocol = protocol;
    writeFuncData.vrId = vrId;

    parentWriteFuncInfo.data = (GT_PTR)(&writeFuncData);

    lpmEngineMemPtr = shadowPtr->mcSearchMemArrayPtr[protocol];

    lpmPtr.nextBucket = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol];

    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
    {
        numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
    }
    else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
    {
        numOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
    }
    cpssOsMemCpy(groupAddr, ipGroupPtr, sizeof(GT_U8)* numOfOctets);

    rootBucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol];
    retVal = prvCpssDxChLpmRamMngEntryDelete(&lpmPtr,groupAddr,ipGroupPrefix,
                                             PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E,GT_TRUE,
                                             lpmEngineMemPtr,&rootBucketType,
                                             (GT_PTR*)srcBucketPtrPtr,shadowPtr,
                                             &parentWriteFuncInfo, GT_FALSE,rollBack,protocol,PRV_CPSS_DXCH_LPM_ALLOC_MC_GR_TYPE_E);
    if(retVal == GT_OK)
    {
        shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol] = rootBucketType;
        retVal = prvCpssDxChLpmRamMngVrfEntryUpdate(vrId, protocol, shadowPtr);
    }

    /* keep the pendingBlockToUpdateArr. in case group release fail need to return to previuos prnding list updated for src  */
    cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,
                 sizeof(shadowPtr->pendingBlockToUpdateArr));

    retVal2 = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                              &shadowPtr->freeMemListEndOfUpdate,shadowPtr);

    if (retVal2!=GT_OK)
    {
        cpssOsMemCpy(shadowPtr->pendingBlockToUpdateArr,tempPendingBlockToUpdateArr,
                     sizeof(shadowPtr->pendingBlockToUpdateArr));
    }

    if(retVal==GT_OK)
    {
        return retVal2;
    }

    return retVal;
}


/**
* @internal coreIpMcGroupInsertFuncWrap function
* @endinternal
*
* @brief   wrapper function to CORE_IP_MC_GROUP_INSERT_FUNC_PTR
*
* @param[in] data                     - parameters to CORE_IP_MC_GROUP_INSERT_FUNC_PTR
*
* @retval GT_OK                    - on success.
* @retval GT_NO_SUCH               - if ipGroup was not found, when updateOld is GT_TRUE
* @retval GT_OUT_OF_PP_MEM         - no more ipm Groups can be added
* @retval GT_FAIL                  - on error.
*/
static GT_STATUS coreIpMcGroupInsertFuncWrap
(
    IN GT_PTR data
)
{
    SHADOW_IP_MC_GROUP_INSERT_FUNC_PARAMS_STC *params;

    params = (SHADOW_IP_MC_GROUP_INSERT_FUNC_PARAMS_STC*)data;

    if (params->protocol > PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return (prvCpssDxChLpmRamMcGroupInsert(params->vrId,
                                           params->ipGroup,
                                           params->ipGroupPrefix,
                                           params->srcBucketPtr,
                                           params->shadowPtr,
                                           params->insertMode,
                                           params->protocol));
}

/**
* @internal prvCpssDxChLpmRamMcDefRollBack function
* @endinternal
*
* @brief   restore original the default MC route for the given VR router.
*
* @param[in] vrId                     - The  to which this tree belongs to.
* @param[in] defMcRoutePointerPtr     - A pointer to the default mc route
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] insertMode               - the insert mode.
* @param[in] protocolStack            - the protocol stack.
* @param[in] srcBucketPtr             - pointer to the root bucket of LPM structure that holds the  IP-Mc src prefixes.
* @param[in] ipSrcArr[]               - the root address for source base multi tree protocol.
* @param[in] rootBucketType           - the bucket type of the root bucket
* @param[in] alreadyExists            - indicates if the given Src prefix already exists in the LPM structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
static GT_STATUS prvCpssDxChLpmRamMcDefRollBack
(
    IN    GT_U32                                          vrId,
    IN    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       *defMcRoutePointerPtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                *shadowPtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT      insertMode,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT            protocolStack,
    IN    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC         *srcBucketPtr,
    IN    GT_U8                                           ipSrcArr[],
    IN    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT             rootBucketType,
    IN    GT_BOOL                                         alreadyExists
)
{
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT    rootBucketFlag;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC            **lpmEngineMemPtr;
    GT_U32                                        ipGroupPrefix;
    GT_U8                                         ipGroup[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC parentWriteFuncInfo;
    SHADOW_IP_MC_GROUP_INSERT_FUNC_PARAMS_STC writeFuncData;

    ipGroup[0] =
        (GT_U8)((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
        PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS :
        PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS);
    ipGroupPrefix =
        (protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
        PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS :
        PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS;

     /* set the default route */
    retVal = prvCpssDxChLpmRamMcDefRouteSet(vrId,defMcRoutePointerPtr,
                                            shadowPtr,insertMode,protocolStack);
    if (retVal != GT_OK)
    {
        return retVal;
    }

     while(1)
     {
        /* get next group */
        retVal = cpssDxChLpmRamMcGroupGetNext(vrId, GT_TRUE, ipGroup,
                                              &ipGroupPrefix,&srcBucketPtr,
                                              shadowPtr,protocolStack);
        /* check if there are no more groups */
        if (retVal == GT_NOT_FOUND)
        {
            retVal = GT_OK;
            break;
        }

        if (retVal != GT_OK)
            return retVal;

        if (srcBucketPtr == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

        /* check if the (G,*) is explicit */
        if (srcBucketPtr->isImplicitGroupDefault == GT_FALSE)
        {
            continue; /* to the next group */
        }

        /* update the new group default in its source trie */

        /* the root type for the src trie can be regular, compressed-1 or compressed-2 */
        rootBucketFlag = PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITH_RAM_UPDATE_E;

        /* src lpm search is done in the same way as UC lpm */
        lpmEngineMemPtr = shadowPtr->ucSearchMemArrayPtr[protocolStack];

        /* update the info for the parent write function */
        parentWriteFuncInfo.bucketParentWriteFunc =
             coreIpMcGroupInsertFuncWrap;

        writeFuncData.vrId              = vrId;
        writeFuncData.ipGroup           = ipGroup;
        writeFuncData.ipGroupPrefix     = ipGroupPrefix;
        writeFuncData.srcBucketPtr      = srcBucketPtr;
        writeFuncData.shadowPtr         = shadowPtr;
        writeFuncData.insertMode        = PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E;
        writeFuncData.protocol          = protocolStack;
        parentWriteFuncInfo.data        = (GT_PTR)(&writeFuncData);

        /* Insert the complete ip address to the lpm structure of PP */
        retVal = prvCpssDxChLpmRamMngInsert(srcBucketPtr, ipSrcArr, 0, defMcRoutePointerPtr,
                                            CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
                                            rootBucketFlag, lpmEngineMemPtr,
                                            &alreadyExists,&rootBucketType,
                                            shadowPtr, PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                            GT_TRUE,
                                            &parentWriteFuncInfo,protocolStack,PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E);
        if (retVal != GT_OK)
            return retVal;
    }
     return GT_OK;
}



/**
* @internal prvCpssDxChLpmSip6RamMcEntryAdd function
* @endinternal
*
* @brief   To add the multicast routing information for IP datagrams from
*         a particular source and addressed to a particular IP multicast
*         group address.
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroupPtr               - The IP multicast group address.
* @param[in] ipGroupPrefix            - The number of bits that are actual valid in,
*                                      the ipGroup. (CIDR like)
* @param[in] ipSrcArr[]               - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefix              - The number of bits that are actual valid in,
*                                      the ipSrcArr. (CIDR like)
* @param[in] mcRoutePointerPtr        - the mc route entry pointer.
* @param[in] override                 - update or  an existing entry, or create a new one.
* @param[in] insertMode               - how to insert the new entry.
* @param[in] protocolStack            - protocol to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - if inserted group is already exist and new ipGroupRuleIndex
*                                       is not equal to current ipGroupRuleIndex, or
* @retval GT_ERROR                 - if the virtual router does not exist,
*                                       or ipGroupPrefix == 0,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmSip6RamMcEntryAdd
(
    IN GT_U32                                       vrId,
    IN GT_U8                                        *ipGroupPtr,
    IN GT_U32                                       ipGroupPrefix,
    IN GT_U8                                        ipSrcArr[],
    IN GT_U32                                       ipSrcPrefix,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *mcRoutePointerPtr,
    IN GT_BOOL                                      override,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN GT_BOOL                                      defragmentationEnable
)
{
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT rootBucketType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E; /* The bucket type of the root bucket*/
                                        /* of the LPM structure that holds  */
                                        /* the IP-Mc src prefixes.          */

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *srcBucket = NULL;/* A pointer to the root bucket of  */
                                        /* LPM structure that holds the     */
                                        /* IP-Mc src prefixes.              */
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *defMcRoutePointer;/* A pointer to the
                                           default route */
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopEntry = NULL;          /* Temporary vars. that hold the new*/

                                        /* entries to be inserted to the LPM*/
                                        /* structure.                       */
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT nextHopEntryType;
    GT_BOOL updateOld;                  /* The updateOld param. to be sent  */
                                        /* the mcTreeInsert() function.     */

    GT_BOOL alreadyExists = GT_FALSE;   /* Indicates if the given Src prefix*/
                                        /* already exists in the LPM        */
                                        /* structure.                       */
    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT    rootBucketFlag;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmEngineMemPtr;
    GT_STATUS           retVal,retVal2;
    GT_U8                                   searchIpGroup[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];
    GT_U32                                  searchIpGroupPrefix,i,maxMemSize;
    GT_BOOL                                 tmpIsImplicitGroupDefault = GT_FALSE;
    GT_BOOL                                 isFullGroupMask = GT_FALSE;
    GT_BOOL                                 addNewSrcTree = GT_FALSE;
    PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC   parentUpdateParams;
    GT_U32 shareDevListLen;
    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
    CPSS_SYSTEM_RECOVERY_INFO_STC           tempSystemRecovery_Info;
    GT_BOOL                                 managerHwWriteBlock;

    cpssOsMemSet(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));
    /* rollback settings */
    /* first check if the protocol was initialized */
    if (shadowPtr->isProtocolInitialized[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "protocol is not initialized");
    }

    if((vrId >= shadowPtr->vrfTblSize) ||
       (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "BAD vrId");
    }

    /* verify that multicast is supported for this protocol in the VR */
    if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "multicast is not supported for this protocol in the VR");
    }

    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;

    for (i = 0; i < shareDevListLen; i++)
    {
        /*Lock the access to per device data base in order to avoid corruption*/
        CPSS_API_LOCK_MAC(shareDevsList[i],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        if (mcRoutePointerPtr->routeEntryMethod == PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E)
        {
            maxMemSize = PRV_CPSS_DXCH_PP_MAC(shareDevsList[i])->fineTuning.tableSize.routerNextHop;
        }
        else /* PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E or PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E */
        {
            maxMemSize = PRV_CPSS_DXCH_PP_MAC(shareDevsList[i])->fineTuning.tableSize.ecmpQos;
        }
        if (mcRoutePointerPtr->routeEntryBaseMemAddr >= maxMemSize)
        {
           /*Unlock the access to per device data base*/
           CPSS_API_UNLOCK_MAC(shareDevsList[i],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /*Unlock the access to per device data base*/
        CPSS_API_UNLOCK_MAC(shareDevsList[i],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }


    retVal = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
                                             &shadowPtr->freeMemListEndOfUpdate,shadowPtr);
    if (retVal != GT_OK)
        return (retVal);

    /* get the default route */
    retVal = prvCpssDxChLpmRamMcDefRouteGet(vrId,&defMcRoutePointer,shadowPtr,protocolStack);
    if (retVal != GT_OK)
    {
        /* shouldn't happen, there should always be a default */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if ((((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) &&
         (ipGroupPrefix <= PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS)) ||
        ((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) &&
         (ipGroupPrefix <= PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS))))
    {
        isFullGroupMask = GT_TRUE;
    }

    if ((isFullGroupMask == GT_TRUE) && (ipSrcPrefix == 0))
    {
        /* default can be only overwritten */
        if (override == GT_FALSE)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
        /* copy out the def mc pointer */
        cpssOsMemCpy(defMcRoutePointer ,mcRoutePointerPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

        /* set the default route */
        retVal = prvCpssDxChLpmSip6RamMcDefRouteSet(vrId,defMcRoutePointer,
                                                    shadowPtr,insertMode,protocolStack);
        if (retVal != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "prvCpssDxChLpmRamMcDefRouteSet is failed");
        }

        /* we overwritten the default MC, now we also have to update all
           the implicit (G,*) that points to the default MC */
        searchIpGroup[0] =
            (GT_U8)((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
            PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS :
            PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS);
        searchIpGroupPrefix =
            (protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
            PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS :
            PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS;

        while(1)
        {
            /* get next group */
            retVal = cpssDxChLpmRamMcGroupGetNext(vrId, GT_TRUE, searchIpGroup,
                                                  &searchIpGroupPrefix,
                                                  &srcBucket,shadowPtr,protocolStack);
            /* check if there are no more groups */
            if (retVal == GT_NOT_FOUND)
            {
                retVal = GT_OK;
                break;
            }

            if (retVal != GT_OK)
                return retVal;

            if (srcBucket == NULL)
            {
                /*return the def mc pointer */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

            }

            /* check if the (G,*) is explicit */
            if (srcBucket->isImplicitGroupDefault == GT_FALSE)
            {
                continue; /* to the next group */
            }

            /* update the new group default in its source trie */

            /* the root type for the src trie can be regular, compressed-1 or compressed-2 */
            rootBucketFlag = PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITH_RAM_UPDATE_E;

            /* src lpm search is done in the same way as UC lpm */
            lpmEngineMemPtr = shadowPtr->ucSearchMemArrayPtr[protocolStack];
            /* pointer is set to the first gon */
            lpmEngineMemPtr++;

            cpssOsMemSet(&parentUpdateParams,0,sizeof(parentUpdateParams));

            parentUpdateParams.vrId = vrId;
            parentUpdateParams.prefixLength = ipGroupPrefix;
            parentUpdateParams.protocol = protocolStack;
            parentUpdateParams.groupPtr = ipGroupPtr;

            /* Insert the complete ip address to the lpm structure of PP */
            retVal = prvCpssDxChLpmSip6RamMngInsert(srcBucket, ipSrcArr, 0, defMcRoutePointer,
                                                    CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
                                                    rootBucketFlag, lpmEngineMemPtr,
                                                    &alreadyExists,&rootBucketType,
                                                    shadowPtr, PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                    PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E,
                                                    protocolStack,&parentUpdateParams);
            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "prvCpssDxChLpmRamMngInsert src is failed");
            }
        }

        return retVal;
    }

    if ((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) &&
        (ipGroupPrefix < PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS))
    {
        ipGroupPrefix = PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS;
    }
    else if ((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) &&
             (ipGroupPrefix < PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS))
    {
        ipGroupPrefix = PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS;
    }

    retVal = prvCpssDxChLpmRamMcGroupSearch(vrId,ipGroupPtr,ipGroupPrefix,
                                           &srcBucket,shadowPtr,protocolStack);
    if ((GT_PTR*)srcBucket == (GT_PTR*)defMcRoutePointer)
    {
        /* if srcBucket and defMcRoutePointer are equal it means that we are
           adding (G,*) with group prefix of 0 and source prefix > 0 */

        if ((GT_PTR*)srcBucket != (GT_PTR*)shadowPtr->vrRootBucketArray[vrId].multicastDefault[protocolStack])
        {
            /* the group root bucket points to another bucket */
            updateOld = GT_TRUE;
            addNewSrcTree = GT_FALSE;
            tmpIsImplicitGroupDefault = GT_FALSE;
        }
        else
        {
            /* there is no next bucket for the group root bucket */
            updateOld = GT_FALSE;
            addNewSrcTree = GT_TRUE;
            tmpIsImplicitGroupDefault = GT_TRUE;
        }
    }
    else if(retVal == GT_OK) /* if the group already exists */
    {
        updateOld = GT_TRUE;
        if (ipSrcPrefix == 0)
            /* we add (G,*) to an existing group, therefore the (G,*) will be explicit */
            tmpIsImplicitGroupDefault = GT_FALSE;
        else
            /* we add source to an existing group, therefore the (G,*) implicit state doesn't change */
            tmpIsImplicitGroupDefault = srcBucket->isImplicitGroupDefault;
    }
    else /* the group doesn't exists */
    {
        updateOld = GT_FALSE;
    }

    /* There is no initialized LPM structure for this ip-group, initialize  */
    /* an LPM structure, with the default mc entry as the LPM default entry.*/
    if(updateOld == GT_FALSE)
    {
        srcBucket =
            prvCpssDxChLpmSip6RamMngCreateNew(defMcRoutePointer,
                                              PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
        if(srcBucket == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        if (addNewSrcTree == GT_FALSE)
        {
            if (ipSrcPrefix == 0)
            {
                /* We create a new group and add the group default, therefore the
                   group WILL be explicit default. The (G,*) is currently marked as
                   implicit because it was created implicitly when creating srcBucket
                   but wasn't explicity added yet */
                srcBucket->isImplicitGroupDefault = GT_TRUE;
                tmpIsImplicitGroupDefault = GT_FALSE;
            }
            else
            {
                /* We add source to a new group, therefore an implicit group
                   default will be added */
                srcBucket->isImplicitGroupDefault = GT_TRUE;
                tmpIsImplicitGroupDefault = GT_TRUE;
            }
        }
    }

    /* Check if an entry with the same (ipSrcArr,ipSrcPrefix) already exists;
       if yes, delete the old one and create a new entry. */
    if (addNewSrcTree == GT_FALSE)
    {
        retVal = prvCpssDxChLpmRamMngSearch(srcBucket,ipSrcArr,ipSrcPrefix,
                                            (GT_PTR*)(GT_UINTPTR)&nextHopEntry);
    }
    else
    {
        retVal = GT_NOT_FOUND;
    }
    /* check if the MC entry exists and was explicitly added */
    if (((retVal == GT_OK) && (ipSrcPrefix != 0)) ||
        ((retVal == GT_OK) && (ipSrcPrefix == 0) && (srcBucket->isImplicitGroupDefault == GT_FALSE)))
    {
        if (override == GT_FALSE)
        {
            if (updateOld == GT_FALSE)
                prvCpssDxChLpmRamMngBucketDelete(srcBucket,8,NULL);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
        }

        alreadyExists = GT_TRUE;
    }
    else
    {
        /* new entry allocate space */
        nextHopEntry = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
        if (nextHopEntry == NULL)
        {
            if (updateOld == GT_FALSE)
                prvCpssDxChLpmRamMngBucketDelete(srcBucket,8,NULL);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
    }

    cpssOsMemSet(&parentUpdateParams,0,sizeof(parentUpdateParams));
    parentUpdateParams.vrId = vrId;
    parentUpdateParams.prefixLength = ipGroupPrefix;
    parentUpdateParams.protocol = protocolStack;
    parentUpdateParams.groupPtr = ipGroupPtr;


    /* the root type for the src trie can be regular, compressed-1 or compressed-2 */
    rootBucketFlag = PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITH_RAM_UPDATE_E;

    retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (retVal != GT_OK)
    {
        return retVal;
    }

    /* in case of hot sync no need to check and allocate , it's done in the end*/
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
    if ((insertMode != PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E)&&
        (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
        (managerHwWriteBlock == GT_FALSE) )
    {
        /* Check memory availability. */
        retVal = checkSip6AvailableMcSearchMemory(srcBucket,vrId,ipGroupPtr,
                                                  ipGroupPrefix,ipSrcArr,ipSrcPrefix,
                                                  rootBucketFlag,updateOld,
                                                  protocolStack,shadowPtr,
                                                  insertMode,defragmentationEnable,
                                                  &parentUpdateParams);
        if (retVal != GT_OK)
        {
            if (updateOld == GT_FALSE)
                prvCpssDxChLpmRamMngBucketDelete(srcBucket,8,NULL);
            if (alreadyExists == GT_FALSE)
                cpssOsLpmFree(nextHopEntry);
            return (retVal);
        }
    }

    if (srcBucket == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, "Shouldn't happen: srcBucket is NULL");
    }

	/* copy out the mc pointer */
    cpssOsMemCpy(nextHopEntry ,mcRoutePointerPtr,
                 sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
    rootBucketType = srcBucket->bucketType;

    /* set src lpm search */
    /* src lpm search is done in the same way as UC lpm */
    lpmEngineMemPtr = shadowPtr->ucSearchMemArrayPtr[protocolStack];
    /* pointer is set to the first gon */
    lpmEngineMemPtr++;
    PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(nextHopEntry->routeEntryMethod,nextHopEntryType);

    cpssOsMemSet(&parentUpdateParams,0,sizeof(parentUpdateParams));
    parentUpdateParams.vrId = vrId;
    parentUpdateParams.prefixLength = ipGroupPrefix;
    parentUpdateParams.protocol = protocolStack;
    parentUpdateParams.groupPtr = ipGroupPtr;

    /* Insert the complete ip address to the lpm structure of PP */
    /*writeFuncData.srcBucketPtr = srcBucket;*/
    retVal = prvCpssDxChLpmSip6RamMngInsert(srcBucket, ipSrcArr, ipSrcPrefix, nextHopEntry,
                                            nextHopEntryType,
                                            rootBucketFlag, lpmEngineMemPtr,
                                            &alreadyExists, &rootBucketType,
                                            shadowPtr, insertMode,
                                            PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E,
                                            protocolStack,&parentUpdateParams);


    if(retVal != GT_OK)
    {

        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "mc src tree insert fail");
    }

    /* Insert the MC Group ip address to the lpm structure of PP */
    retVal = prvCpssDxChLpmSip6RamMcGroupInsert(vrId, ipGroupPtr, ipGroupPrefix,
                                                srcBucket, shadowPtr,
                                                insertMode, protocolStack);

    if (retVal != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "mc group insert fail");
    }

    retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (retVal != GT_OK)
    {
        return retVal;
    }
    /* in case of shadow update only no need to check since we didn't allocate
       nothing*/
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
    if ((insertMode != PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E)&&
        (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
        (managerHwWriteBlock == GT_FALSE))
    {
        /* check that all the pre-allocated memory was used */
        if (shadowPtr->neededMemoryCurIdx != shadowPtr->neededMemoryListLen)
        {
            /* should never! - meaning we didn't use all preallcoated memory
               FATAL ERROR*/

            /* free the allocated/bound RAM memory */
            retVal2 = prvCpssDxChLpmSip6RamMngAllocatedAndBoundMemFree(shadowPtr,protocolStack);
            if (retVal2!=GT_OK)
            {
                return retVal2;
            }
            /* in case of fail we will need to recondtruct to the status we had before the call to prvCpssDxChLpmRamMngAllocAvailableMemCheck */
            cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                         sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    /* MC entry was added successfully, set group default implicit state */
    srcBucket->isImplicitGroupDefault = tmpIsImplicitGroupDefault;

    shadowPtr->neededMemoryListLen = 0;

    /* the data was written successfully to HW, we can reset the information regarding the new memoryPool allocations done.
       next call to ADD will set this array with new values of allocated/bound blocks */
    cpssOsMemSet(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr,0,sizeof(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr));

    cpssOsMemSet(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));

    retVal2 = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                              &shadowPtr->freeMemListEndOfUpdate,shadowPtr);
    if(retVal==GT_OK)
    {
        if(retVal2==GT_OK)
        {
            /* update counters for MC allocation */
            retVal2 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                shadowPtr->protocolCountersPerBlockArr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                protocolStack,
                                                                shadowPtr->numOfLpmMemories);
            if (retVal2!=GT_OK)
            {
                /* reset pending array for future use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

                return retVal2;
            }
        }
        else
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
        }
        return retVal2;
    }
    else
    {
        /* reset pending array for future use */
        cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
    }

    return retVal;
}




/**
* @internal prvCpssDxChLpmRamMcEntryAdd function
* @endinternal
*
* @brief   To add the multicast routing information for IP datagrams from
*         a particular source and addressed to a particular IP multicast
*         group address.
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroupPtr               - The IP multicast group address.
* @param[in] ipGroupPrefix            - The number of bits that are actual valid in,
*                                      the ipGroup. (CIDR like)
* @param[in] ipSrcArr[]               - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefix              - The number of bits that are actual valid in,
*                                      the ipSrcArr. (CIDR like)
* @param[in] mcRoutePointerPtr        - the mc route entry pointer.
* @param[in] override                 - update or  an existing entry, or create a new one.
* @param[in] insertMode               - how to insert the new entry.
* @param[in] protocolStack            - protocol to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - if inserted group is already exist and new ipGroupRuleIndex
*                                       is not equal to current ipGroupRuleIndex, or
* @retval GT_ERROR                 - if the virtual router does not exist,
*                                       or ipGroupPrefix == 0,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamMcEntryAdd
(
    IN GT_U32                                       vrId,
    IN GT_U8                                        *ipGroupPtr,
    IN GT_U32                                       ipGroupPrefix,
    IN GT_U8                                        ipSrcArr[],
    IN GT_U32                                       ipSrcPrefix,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *mcRoutePointerPtr,
    IN GT_BOOL                                      override,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN GT_BOOL                                      defragmentationEnable
)
{
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT rootBucketType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E; /* The bucket type of the root bucket*/
                                        /* of the LPM structure that holds  */
                                        /* the IP-Mc src prefixes.          */

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *srcBucket = NULL;/* A pointer to the root bucket of  */
                                        /* LPM structure that holds the     */
                                        /* IP-Mc src prefixes.              */
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *defMcRoutePointer;/* A pointer to the
                                           default route */
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopEntry = NULL;          /* Temporary vars. that hold the new*/

                                        /* entries to be inserted to the LPM*/
                                        /* structure.                       */
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT nextHopEntryType;
    GT_BOOL updateOld;                  /* The updateOld param. to be sent  */
                                        /* the mcTreeInsert() function.     */

    GT_BOOL alreadyExists = GT_FALSE;   /* Indicates if the given Src prefix*/
                                        /* already exists in the LPM        */
                                        /* structure.                       */
    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT    rootBucketFlag;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmEngineMemPtr;
    GT_STATUS           retVal,retVal2;
    GT_U8                                   searchIpGroup[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];
    GT_U32                                  searchIpGroupPrefix,i,maxMemSize;
    GT_BOOL                                 tmpIsImplicitGroupDefault = GT_FALSE;
    GT_BOOL                                 isFullGroupMask = GT_FALSE;
    GT_BOOL                                 addNewSrcTree = GT_FALSE;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC rollBackDefMcRoutePointer;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *rollBackNextHopEntryPtr = NULL;
    GT_U32 shareDevListLen;
    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
    CPSS_SYSTEM_RECOVERY_INFO_STC   tempSystemRecovery_Info;
    GT_BOOL                         managerHwWriteBlock;

    /* rollback definitions */
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC parentWriteFuncInfo;
    SHADOW_IP_MC_GROUP_INSERT_FUNC_PARAMS_STC writeFuncData;

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC parentWriteFuncInfoVrf;
    PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PARAMS_STC writeFuncDataVrf;

    PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT lpmPtr;    /* Will hold the pointer to the IP-src LPM  */
                                                        /* structure for delete operations.         */
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT newBucketType;  /* Holds the LPM first bucket type after    */
                                                        /* delete operations.                       */
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *delEntry; /* A pointer to the data saved in the trie, */
                                                         /* for IP-src LPM history purposes. And     */
                                                         /* should be deleted.                       */
    cpssOsMemSet(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));
    cpssOsMemSet(&rollBackDefMcRoutePointer,0,sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
    /* rollback settings */
    parentWriteFuncInfo.bucketParentWriteFunc =
        coreIpMcGroupInsertFuncWrap;

    writeFuncData.vrId              = vrId;
    writeFuncData.ipGroup           = ipGroupPtr;
    writeFuncData.ipGroupPrefix     = ipGroupPrefix;
    writeFuncData.shadowPtr         = shadowPtr;
    writeFuncData.insertMode        = PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E;
    writeFuncData.protocol          = protocolStack;

    parentWriteFuncInfo.data = (GT_PTR)(&writeFuncData);

    /* first check if the protocol was initialized */
    if (shadowPtr->isProtocolInitialized[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if((vrId >= shadowPtr->vrfTblSize) ||
       (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* verify that multicast is supported for this protocol in the VR */
    if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;

    for (i = 0; i < shareDevListLen; i++)
    {
        /*Lock the access to per device data base in order to avoid corruption*/
        CPSS_API_LOCK_MAC(shareDevsList[i],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        if (mcRoutePointerPtr->routeEntryMethod == PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E)
        {
            maxMemSize = PRV_CPSS_DXCH_PP_MAC(shareDevsList[i])->fineTuning.tableSize.routerNextHop;
        }
        else /* PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E or PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E */
        {
            maxMemSize = PRV_CPSS_DXCH_PP_MAC(shareDevsList[i])->fineTuning.tableSize.ecmpQos;
        }
        if (mcRoutePointerPtr->routeEntryBaseMemAddr >= maxMemSize)
        {
           /*Unlock the access to per device data base*/
           CPSS_API_UNLOCK_MAC(shareDevsList[i],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /*Unlock the access to per device data base*/
        CPSS_API_UNLOCK_MAC(shareDevsList[i],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }


    retVal = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
                                             &shadowPtr->freeMemListEndOfUpdate,shadowPtr);
    if (retVal != GT_OK)
        return (retVal);

    /* get the default route */
    retVal = prvCpssDxChLpmRamMcDefRouteGet(vrId,&defMcRoutePointer,shadowPtr,protocolStack);
    if (retVal != GT_OK)
    {
        /* shouldn't happen, there should always be a default */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if ((((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) &&
         (ipGroupPrefix <= PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS)) ||
        ((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) &&
         (ipGroupPrefix <= PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS))))
    {
        isFullGroupMask = GT_TRUE;
    }

    if ((isFullGroupMask == GT_TRUE) && (ipSrcPrefix == 0))
    {
        /* default can be only overwritten */
        if (override == GT_FALSE)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);

        /* save original def mc pointer */
        cpssOsMemCpy(&rollBackDefMcRoutePointer ,defMcRoutePointer,
                     sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

        /* copy out the def mc pointer */
        cpssOsMemCpy(defMcRoutePointer ,mcRoutePointerPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

        /* set the default route */
        retVal = prvCpssDxChLpmRamMcDefRouteSet(vrId,defMcRoutePointer,
                                                shadowPtr,insertMode,protocolStack);
        if (retVal != GT_OK)
        {
            /*return the def mc pointer */
            cpssOsMemCpy(defMcRoutePointer ,&rollBackDefMcRoutePointer,
                         sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

            /* new default set is failed */
            /* return original default   */
            prvCpssDxChLpmRamMcDefRouteSet(vrId,defMcRoutePointer,
                                           shadowPtr,insertMode,protocolStack);
            return retVal;
        }

        /* we overwritten the default MC, now we also have to update all
           the implicit (G,*) that points to the default MC */
        searchIpGroup[0] =
            (GT_U8)((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
            PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS :
            PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS);
        searchIpGroupPrefix =
            (protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
            PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS :
            PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS;

        while(1)
        {
            /* get next group */
            retVal = cpssDxChLpmRamMcGroupGetNext(vrId, GT_TRUE, searchIpGroup,
                                                  &searchIpGroupPrefix,
                                                  &srcBucket,shadowPtr,protocolStack);
            /* check if there are no more groups */
            if (retVal == GT_NOT_FOUND)
            {
                retVal = GT_OK;
                break;
            }

            if (retVal != GT_OK)
                return retVal;

            if (srcBucket == NULL)
            {
                /*return the def mc pointer */
                cpssOsMemCpy(defMcRoutePointer ,&rollBackDefMcRoutePointer,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

                /* src default update failed, activate rollback */
                prvCpssDxChLpmRamMcDefRollBack(vrId,defMcRoutePointer,shadowPtr,insertMode,
                                               protocolStack,srcBucket,ipSrcArr,rootBucketType,
                                               alreadyExists);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

            }

            /* check if the (G,*) is explicit */
            if (srcBucket->isImplicitGroupDefault == GT_FALSE)
            {
                continue; /* to the next group */
            }

            /* update the new group default in its source trie */

            /* the root type for the src trie can be regular, compressed-1 or compressed-2 */
            rootBucketFlag = PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITH_RAM_UPDATE_E;

            /* src lpm search is done in the same way as UC lpm */
            lpmEngineMemPtr = shadowPtr->ucSearchMemArrayPtr[protocolStack];

            /* Insert the complete ip address to the lpm structure of PP */
            writeFuncData.srcBucketPtr = srcBucket;
            retVal = prvCpssDxChLpmRamMngInsert(srcBucket, ipSrcArr, 0, defMcRoutePointer,
                                                CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
                                                rootBucketFlag, lpmEngineMemPtr,
                                                &alreadyExists,&rootBucketType,
                                                shadowPtr, PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                GT_TRUE,
                                                &parentWriteFuncInfo,protocolStack,PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E);
            if (retVal != GT_OK)
            {
                /*return the def mc pointer */
                cpssOsMemCpy(defMcRoutePointer ,&rollBackDefMcRoutePointer,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

                /* src default update failed, activate rollback */
                prvCpssDxChLpmRamMcDefRollBack(vrId,defMcRoutePointer,shadowPtr,insertMode,
                                               protocolStack,srcBucket,ipSrcArr,rootBucketType,
                                               alreadyExists);
                return retVal;

            }
        }

        return retVal;
    }

    if ((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) &&
        (ipGroupPrefix < PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS))
    {
        ipGroupPrefix = PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS;
    }
    else if ((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) &&
             (ipGroupPrefix < PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS))
    {
        ipGroupPrefix = PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS;
    }

    retVal = prvCpssDxChLpmRamMcGroupSearch(vrId,ipGroupPtr,ipGroupPrefix,
                                           &srcBucket,shadowPtr,protocolStack);
    if ((GT_PTR*)srcBucket == (GT_PTR*)defMcRoutePointer)
    {
        /* if srcBucket and defMcRoutePointer are equal it means that we are
           adding (G,*) with group prefix of 0 and source prefix > 0 */

        if ((GT_PTR*)srcBucket != (GT_PTR*)shadowPtr->vrRootBucketArray[vrId].multicastDefault[protocolStack])
        {
            /* the group root bucket points to another bucket */
            updateOld = GT_TRUE;
            addNewSrcTree = GT_FALSE;
            tmpIsImplicitGroupDefault = GT_FALSE;
        }
        else
        {
            /* there is no next bucket for the group root bucket */
            updateOld = GT_FALSE;
            addNewSrcTree = GT_TRUE;
            tmpIsImplicitGroupDefault = GT_TRUE;
        }
    }
    else if(retVal == GT_OK) /* if the group already exists */
    {
        updateOld = GT_TRUE;
        if (ipSrcPrefix == 0)
            /* we add (G,*) to an existing group, therefore the (G,*) will be explicit */
            tmpIsImplicitGroupDefault = GT_FALSE;
        else
            /* we add source to an existing group, therefore the (G,*) implicit state doesn't change */
            tmpIsImplicitGroupDefault = srcBucket->isImplicitGroupDefault;
    }
    else /* the group doesn't exists */
    {
        updateOld = GT_FALSE;
    }

    /* There is no initialized LPM structure for this ip-group, initialize  */
    /* an LPM structure, with the default mc entry as the LPM default entry.*/
    if(updateOld == GT_FALSE)
    {
        srcBucket =
            prvCpssDxChLpmRamMngCreateNew(defMcRoutePointer,
                                          PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
        if(srcBucket == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        if (addNewSrcTree == GT_FALSE)
        {
            if (ipSrcPrefix == 0)
            {
                /* We create a new group and add the group default, therefore the
                   group WILL be explicit default. The (G,*) is currently marked as
                   implicit because it was created implicitly when creating srcBucket
                   but wasn't explicity added yet */
                srcBucket->isImplicitGroupDefault = GT_TRUE;
                tmpIsImplicitGroupDefault = GT_FALSE;
            }
            else
            {
                /* We add source to a new group, therefore an implicit group
                   default will be added */
                srcBucket->isImplicitGroupDefault = GT_TRUE;
                tmpIsImplicitGroupDefault = GT_TRUE;
            }
        }
    }

    /* Check if an entry with the same (ipSrcArr,ipSrcPrefix) already exists;
       if yes, delete the old one and create a new entry. */
    if (addNewSrcTree == GT_FALSE)
    {
        retVal = prvCpssDxChLpmRamMngSearch(srcBucket,ipSrcArr,ipSrcPrefix,
                                            (GT_PTR*)(GT_UINTPTR)&nextHopEntry);
    }
    else
    {
        retVal = GT_NOT_FOUND;
    }

    if (retVal == GT_OK)
    {
        /* save original nextHopEntry  */
        cpssOsMemCpy(&rollBackDefMcRoutePointer ,nextHopEntry,
                     sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
    }

    /* check if the MC entry exists and was explicitly added */
    if (((retVal == GT_OK) && (ipSrcPrefix != 0)) ||
        ((retVal == GT_OK) && (ipSrcPrefix == 0) && (srcBucket->isImplicitGroupDefault == GT_FALSE)))
    {
        if (override == GT_FALSE)
        {
            if (updateOld == GT_FALSE)
                prvCpssDxChLpmRamMngBucketDelete(srcBucket,8,NULL);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
        }

        alreadyExists = GT_TRUE;
    }
    else
    {
        /* new entry allocate space */
        nextHopEntry = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
        if (nextHopEntry == NULL)
        {
            if (updateOld == GT_FALSE)
                prvCpssDxChLpmRamMngBucketDelete(srcBucket,8,NULL);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
    }

    /* copy out the mc pointer */
    cpssOsMemCpy(nextHopEntry ,mcRoutePointerPtr,
                 sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

    /* the root type for the src trie can be regular, compressed-1 or compressed-2 */
    rootBucketFlag = PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITH_RAM_UPDATE_E;

    /* update the info for the parent write function */
    parentWriteFuncInfoVrf.bucketParentWriteFunc =
        prvCpssDxChLpmRamUpdateVrTableFuncWrapper;

    writeFuncDataVrf.shadowPtr = shadowPtr;
    writeFuncDataVrf.protocol = protocolStack;
    writeFuncDataVrf.vrId = vrId;
    parentWriteFuncInfoVrf.data = (GT_PTR)(&writeFuncDataVrf);

    retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (retVal != GT_OK)
    {
        return retVal;
    }
    /* in case of hot sync no need to check and allocate , it's done in the end*/
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
    if ((insertMode != PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E)&&
        (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
        (managerHwWriteBlock == GT_FALSE))
    {
        /* Check memory availability. */
        retVal = checkAvailableMcSearchMemory(srcBucket,vrId,ipGroupPtr,
                                              ipGroupPrefix,ipSrcArr,ipSrcPrefix,
                                              rootBucketFlag,updateOld,
                                              protocolStack,shadowPtr,
                                              insertMode,defragmentationEnable,
                                              &parentWriteFuncInfoVrf);
        if (retVal != GT_OK)
        {
            if (updateOld == GT_FALSE)
                prvCpssDxChLpmRamMngBucketDelete(srcBucket,8,NULL);
            if (alreadyExists == GT_FALSE)
                cpssOsLpmFree(nextHopEntry);
            return (retVal);
        }
    }

    if (srcBucket == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, "Shouldn't happen: srcBucket is NULL");
    }
    rootBucketType = srcBucket->bucketType;

    /* set src lpm search */
    /* src lpm search is done in the same way as UC lpm */
    lpmEngineMemPtr = shadowPtr->ucSearchMemArrayPtr[protocolStack];

    PRV_CPSS_DXCH_LPM_NEXT_PTR_TYPE_CONVERT_MAC(nextHopEntry->routeEntryMethod,nextHopEntryType);
    /* Insert the complete ip address to the lpm structure of PP */
    writeFuncData.srcBucketPtr = srcBucket;
    retVal = prvCpssDxChLpmRamMngInsert(srcBucket, ipSrcArr, ipSrcPrefix, nextHopEntry,
                                        nextHopEntryType,
                                        rootBucketFlag, lpmEngineMemPtr,
                                        &alreadyExists, &rootBucketType,
                                        shadowPtr, insertMode, GT_TRUE,
                                        &parentWriteFuncInfo,protocolStack,PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E);
    if(retVal != GT_OK)
    {
        shadowPtr->neededMemoryListLen = 0;
        prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                        &shadowPtr->freeMemListEndOfUpdate,shadowPtr);
        /* update the info for the parent write function */
        /* get the old bucket type */
        newBucketType = srcBucket->bucketType;
        lpmPtr.nextBucket = srcBucket;
        writeFuncData.srcBucketPtr = srcBucket;
        if ((updateOld == GT_TRUE) && (ipSrcPrefix == 0))
        {
            /* It was G,* insertion */
            /* return previous G,* */
            if (srcBucket->isImplicitGroupDefault == GT_TRUE)
            {
                /* G,* wasn't explicit before, restore default */
                rollBackNextHopEntryPtr = defMcRoutePointer;
            }
            else
            {
                /* restore original nextHopEntry  */
                cpssOsMemCpy(nextHopEntry,&rollBackDefMcRoutePointer,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                rollBackNextHopEntryPtr = nextHopEntry;
            }


            prvCpssDxChLpmRamMngInsert(srcBucket, ipSrcArr, ipSrcPrefix, rollBackNextHopEntryPtr,
                                        CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
                                        rootBucketFlag, lpmEngineMemPtr,
                                        &alreadyExists,&rootBucketType,
                                        shadowPtr, PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                        GT_TRUE,
                                        &parentWriteFuncInfo,protocolStack,PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E);
            if ((updateOld == GT_TRUE) && (ipSrcPrefix == 0) && (srcBucket->isImplicitGroupDefault == GT_FALSE))
            {
                alreadyExists = GT_TRUE;
            }
        }
        else
        /* Delete the given ip-src from the LPM structure   */
            prvCpssDxChLpmRamMngEntryDelete(&lpmPtr,ipSrcArr,ipSrcPrefix,
                                            PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E,
                                            GT_TRUE,lpmEngineMemPtr,&newBucketType,
                                            (GT_PTR*)(GT_UINTPTR)&delEntry,shadowPtr,
                                            &parentWriteFuncInfo,GT_TRUE,GT_TRUE,protocolStack,PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E);
        if (updateOld == GT_TRUE)
        {
            prvCpssDxChLpmRamMcGroupInsert(vrId,ipGroupPtr,ipGroupPrefix,
                                           srcBucket,shadowPtr,
                                           PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                           protocolStack);
        }

        prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                        &shadowPtr->freeMemListEndOfUpdate,shadowPtr);

        if (updateOld == GT_FALSE)
            prvCpssDxChLpmRamMngBucketDelete(srcBucket,8,NULL);
        if (alreadyExists == GT_FALSE)
            cpssOsLpmFree(nextHopEntry);

        retVal2 = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal2 != GT_OK)
        {
            return retVal2;
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if ((insertMode != PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E)&&
            (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
            (managerHwWriteBlock == GT_FALSE))
        {
            /* free the allocated/bound RAM memory */
            retVal2 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFree(shadowPtr,protocolStack);
            if (retVal2!=GT_OK)
            {
                /* need to clear all configurations done for pendingBlockToUpdateArr in the call to prvCpssDxChLpmRamMemFreeListMng */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

                return retVal2;
            }
            /* in case of fail we will need to recondtruct to the status we had before the call to prvCpssDxChLpmRamMngAllocAvailableMemCheck */
            cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                         sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
        }

        /* need to clear all configurations done for pendingBlockToUpdateArr in the call to prvCpssDxChLpmRamMemFreeListMng */
        /* reset pending array for future use */
        cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

        return retVal;
    }

    /* Insert the MC Group ip address to the lpm structure of PP */
    retVal = prvCpssDxChLpmRamMcGroupInsert(vrId, ipGroupPtr, ipGroupPrefix,
                                            srcBucket, shadowPtr,
                                            insertMode, protocolStack);

    if (retVal != GT_OK)
    {
        shadowPtr->neededMemoryListLen = 0;
        prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                        &shadowPtr->freeMemListEndOfUpdate,shadowPtr);
        /* insert group is failed, activate rollBack */
        prvCpssDxChLpmRamMcEntryDelete(vrId,ipGroupPtr,ipGroupPrefix,ipSrcArr,
                                       ipSrcPrefix,protocolStack,shadowPtr,GT_TRUE,srcBucket);
        retVal2 = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal2 != GT_OK)
        {
            return retVal2;
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if ((insertMode != PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E)&&
            (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
            (managerHwWriteBlock == GT_FALSE))
        {
            /* free the allocated/bound RAM memory */
            retVal2 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFree(shadowPtr,protocolStack);
            if (retVal2!=GT_OK)
            {
                /* need to clear all configurations done for pendingBlockToUpdateArr in the call to prvCpssDxChLpmRamMemFreeListMng */
                /* reset pending array for future use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

                return retVal2;
            }
            /* in case of fail we will need to recondtruct to the status we had before the call to prvCpssDxChLpmRamMngAllocAvailableMemCheck */
            cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                         sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
        }
        /* need to clear all configurations done for pendingBlockToUpdateArr in the call to prvCpssDxChLpmRamMemFreeListMng */
        /* reset pending array for future use */
        cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

        return retVal;
    }

    retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (retVal != GT_OK)
    {
        return retVal;
    }
    /* in case of shadow update only no need to check since we didn't allocate
       nothing*/
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
    if ((insertMode != PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E)&&
        (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
        (managerHwWriteBlock == GT_FALSE))
    {
        /* check that all the pre-allocated memory was used */
        if (shadowPtr->neededMemoryCurIdx != shadowPtr->neededMemoryListLen)
        {
            /* should never! - meaning we didn't use all preallcoated memory
               FATAL ERROR*/

            /* free the allocated/bound RAM memory */
            retVal2 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFree(shadowPtr,protocolStack);
            if (retVal2!=GT_OK)
            {
                return retVal2;
            }
            /* in case of fail we will need to recondtruct to the status we had before the call to prvCpssDxChLpmRamMngAllocAvailableMemCheck */
            cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                         sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    /* MC entry was added successfully, set group default implicit state */
    srcBucket->isImplicitGroupDefault = tmpIsImplicitGroupDefault;

    shadowPtr->neededMemoryListLen = 0;

    /* the data was written successfully to HW, we can reset the information regarding the new memoryPool allocations done.
       next call to ADD will set this array with new values of allocated/bound blocks */
    cpssOsMemSet(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr,0,sizeof(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr));

    cpssOsMemSet(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));

    retVal2 = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                              &shadowPtr->freeMemListEndOfUpdate,shadowPtr);
    if(retVal==GT_OK)
    {
        if(retVal2==GT_OK)
        {
            /* update counters for MC allocation */
            retVal2 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                shadowPtr->protocolCountersPerBlockArr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                protocolStack,
                                                                shadowPtr->numOfLpmMemories);
            if (retVal2!=GT_OK)
            {
                /* reset pending array for future use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

                return retVal2;
            }
        }
        else
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
        }
        return retVal2;
    }
    else
    {
        /* reset pending array for future use */
        cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
    }

    return retVal;
}



/**
* @internal prvCpssDxChLpmSip6RamMcEntryDelete function
* @endinternal
*
* @brief   To delete a particular mc route
*
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroupPtr               - The IP multicast group address.
* @param[in] ipGroupPrefix            - The number of bits that are actual valid in,
*                                      the ipGroup. (CIDR like)
* @param[in] ipSrcArr[]               - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefix              - The number of bits that are actual valid in,
*                                      the ipSrcArr. (CIDR like)
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*                                      roollBack                - GT_TRUE: rollback is taking place.
*                                      GT_FALSE: otherwise.
* @param[in] rollBackSrcBucketPtr     - pointer to src bucket.(used only for rollback)
*
* @retval GT_OK                    - on success, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In order to delete the all the multicast entries, call this function
*       with ipSrcArr[0] = ipSrcPrefix = 0.
*
*/
GT_STATUS prvCpssDxChLpmSip6RamMcEntryDelete
(
    IN GT_U32                                   vrId,
    IN GT_U8                                    *ipGroupPtr,
    IN GT_U32                                   ipGroupPrefix,
    IN GT_U8                                    ipSrcArr[],
    IN GT_U32                                   ipSrcPrefix,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT     protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC         *shadowPtr,
    IN GT_BOOL                                  rollBack,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *rollBackSrcBucketPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *srcBucket;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *defMcRoutePointer;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT         defMcRoutePointerType;

    PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT lpmPtr; /* Will hold the pointer to the IP-src LPM  */
                                                     /* structure for delete operations.         */

    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT newBucketType;  /* Holds the LPM first bucket type after    */
                                /* delete operations.                       */

    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *delEntry; /* A pointer to the data saved in the trie, */
                                /* for IP-src LPM history purposes. And     */
                                /* should be deleted.                       */

    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmEngineMemPtr;
    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT rootBucketFlag;
    GT_STATUS retVal;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT rootBucketType;  /* The bucket type of the root bucket*/
                                        /* of the LPM structure that holds  */
                                        /* the IP-Mc src prefixes.          */
    GT_BOOL alreadyExists = GT_FALSE;   /* Indicates if the given Src prefix*/
                                        /* already exists in the LPM        */
                                        /* structure.                       */

    GT_BOOL     deleteEmptyGroup = GT_FALSE;   /* if group is empty and need to be deleted */
    GT_BOOL     isFullGroupMask = GT_FALSE;

    PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC  parentUpdateParams;


    /* first check the ip version was initialized */
    if (shadowPtr->isProtocolInitialized[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if ((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) &&
        (ipGroupPrefix <= PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS))
    {
        isFullGroupMask = GT_TRUE;
        ipGroupPrefix = PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS;
    }
    else if ((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) &&
             (ipGroupPrefix <= PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS))
    {
        isFullGroupMask = GT_TRUE;
        ipGroupPrefix = PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS;
    }

    /* can't delete the default route. only overwrite it */
    if ((isFullGroupMask == GT_TRUE) && (ipSrcPrefix == 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((vrId >= shadowPtr->vrfTblSize) ||
       (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(((GT_STATUS)GT_ERROR), LOG_ERROR_NO_MSG);
    }

    /* verify that multicast is supported for this protocol in the VR */
    if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(&parentUpdateParams,0,sizeof(parentUpdateParams));

    parentUpdateParams.vrId = vrId;
    parentUpdateParams.prefixLength = ipGroupPrefix;
    parentUpdateParams.protocol = protocolStack;
    parentUpdateParams.groupPtr = ipGroupPtr;


    /* we can't use compressed 2 as a root type for the src trie */
    rootBucketFlag = PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E;

     /* src lpm search is done in the same way as UC lpm */
    lpmEngineMemPtr = shadowPtr->ucSearchMemArrayPtr[protocolStack];

    retVal = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
                                             &shadowPtr->freeMemListEndOfUpdate,shadowPtr);
    if (retVal != GT_OK)
        return (retVal);

    if (rollBack == GT_FALSE)
    {
        retVal = prvCpssDxChLpmRamMcGroupSearch(vrId,ipGroupPtr,ipGroupPrefix,
                                                &srcBucket,shadowPtr,protocolStack);
    }
    else
    {
        srcBucket = rollBackSrcBucketPtr;
    }

    /* if the group wasn't found */
    if(retVal != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* get the default route */
    retVal = prvCpssDxChLpmRamMcDefRouteGet(vrId,&defMcRoutePointer,
                                            shadowPtr,protocolStack);
    if (retVal != GT_OK)
    {
        /* shouldn't happen, there should always be a default */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* if we delete (G,*) */
    if(ipSrcPrefix == 0)
    {
        /* if the (G,*) is implicit, it can't be deleted explicitly */
        if (srcBucket->isImplicitGroupDefault == GT_TRUE)
        {
            if (rollBack == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
            }
        }

        /* if there are no sources attached to the group, delete the group */
        if (srcBucket->bucketType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
        {
            /* mark the group as need to be deleted */
            deleteEmptyGroup = GT_TRUE;
        }
        /* there are still sources attached to the group, therefore just */
        /* set the group default next hop to the MC default hext hop     */
        else
        {
            /* get next hop attached to (G,*) */
            retVal = prvCpssDxChLpmRamMngSearch(srcBucket,ipSrcArr,ipSrcPrefix,(GT_PTR*)(GT_UINTPTR)&delEntry);
            if (retVal != GT_OK)
            {
                /* shouldn't happen */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            /* Note we already know the (G,*) is explicit.
               Replace the next hop with MC default next hop and
               mark the (G,*) as implicit */

            /* set root bucket type */
            rootBucketType = srcBucket->bucketType;

            /* src lpm search is done in the same way as UC lpm */
            lpmEngineMemPtr = shadowPtr->ucSearchMemArrayPtr[protocolStack];

            PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(defMcRoutePointer->routeEntryMethod,defMcRoutePointerType);
            /* Insert the complete ip address to the lpm structure of PP */
            retVal = prvCpssDxChLpmSip6RamMngInsert(srcBucket, ipSrcArr,ipSrcPrefix, defMcRoutePointer,
                                                    defMcRoutePointerType,
                                                    rootBucketFlag,lpmEngineMemPtr,
                                                    &alreadyExists,&rootBucketType,
                                                    shadowPtr, PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                    PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E,protocolStack,&parentUpdateParams);
            if (retVal != GT_OK)
                return retVal;

            retVal = prvCpssDxChLpmSip6RamMcGroupInsert(vrId, ipGroupPtr, ipGroupPrefix,
                                                        srcBucket, shadowPtr,
                                                        PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                        protocolStack);
            if (retVal != GT_OK)
                return retVal;

            /* free old next hop */
            cpssOsLpmFree(delEntry);

            /* mark the (G,*) as implicit */
            srcBucket->isImplicitGroupDefault = GT_TRUE;
        }
    }
    /* we delete (G,S) and not (G,*) */
    else
    {

        /* get the old bucket type */
        newBucketType = srcBucket->bucketType;

        lpmPtr.nextBucket = srcBucket;

        /* Delete the given ip-src from the LPM structure   */
        retVal = prvCpssDxChLpmSip6RamMngEntryDelete(&lpmPtr,ipSrcArr,ipSrcPrefix,
                                                     rootBucketFlag,
                                                     GT_TRUE,lpmEngineMemPtr,
                                                     &newBucketType,(GT_PTR*)(GT_UINTPTR)&delEntry,
                                                     shadowPtr,
                                                     PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E,rollBack,protocolStack,
                                                     &parentUpdateParams);
        if(retVal != GT_OK)
        {
            prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                            &shadowPtr->freeMemListEndOfUpdate,shadowPtr);
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            return retVal;
        }

        if(retVal == GT_OK)
        {
            cpssOsLpmFree(delEntry);
        }
        if (rollBack == GT_FALSE)
        {
            retVal = prvCpssDxChLpmSip6RamMcGroupInsert(vrId, ipGroupPtr, ipGroupPrefix,
                                                        srcBucket, shadowPtr,
                                                        PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                        protocolStack);

            if (retVal != GT_OK)
            {
                return retVal;
            }
        }
        else
        {
            if (!((srcBucket->bucketType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
            (srcBucket->isImplicitGroupDefault == GT_TRUE)))
            {
                retVal = prvCpssDxChLpmSip6RamMcGroupInsert(vrId,ipGroupPtr,ipGroupPrefix,
                                                            srcBucket,shadowPtr,
                                                            PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                            protocolStack);
            }
        }

        /* if there are no more sources attached to the group and the (G,*) is
           implicit, then delete the group */
        if ((srcBucket->bucketType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
            (srcBucket->isImplicitGroupDefault == GT_TRUE))
        {
            /* mark the group as need to be deleted */
            deleteEmptyGroup = GT_TRUE;
        }
    }

    /* if the group was marked as need to be deleted, delete it */
    if (deleteEmptyGroup == GT_TRUE)
    {
        retVal = prvCpssDxChLpmSip6RamMcGroupDelete(vrId,ipGroupPtr,ipGroupPrefix,
                                                    &srcBucket,shadowPtr,protocolStack,rollBack);
        if(retVal != GT_OK)
        {
            prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                            &shadowPtr->freeMemListEndOfUpdate,shadowPtr);

            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            return retVal;
        }

        retVal = prvCpssDxChLpmRamMngBucketDelete(srcBucket,8,(GT_PTR*)(GT_UINTPTR)&delEntry);

        if(retVal != GT_OK)
        {
            /*No need to check the return value of prvCpssDxChLpmMemFreeListMng
              because in this scenario we will return the error from
              prvCpssExMxPmLpmDeleteBucket */
            prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                            &shadowPtr->freeMemListEndOfUpdate,shadowPtr);

            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            return retVal;
        }

        if ((isFullGroupMask == GT_TRUE) && (srcBucket->isImplicitGroupDefault == GT_TRUE))
        {
            retVal = prvCpssDxChLpmSip6RamMcDefRouteSet(vrId, defMcRoutePointer,
                                                        shadowPtr,
                                                        PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                        protocolStack);
            if (retVal != GT_OK)
            {
                 /* reset pending array for future use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

                return retVal;
            }
        }

        /* check if this is the default next hop , if not free it */
        if(delEntry != defMcRoutePointer)
        {
            cpssOsLpmFree(delEntry);
        }
    }

    retVal = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                             &shadowPtr->freeMemListEndOfUpdate,shadowPtr);

    /* in case of rollBack no need to update counters in the "delete" operation
       since the counters were not updated in the "add" operation.
       counters are updated at the end of operation in case of success */
    if ((retVal==GT_OK)&&(rollBack == GT_FALSE))
    {
        /* update counters for MC allocation */
        retVal = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                            shadowPtr->pendingBlockToUpdateArr,
                                                            shadowPtr->protocolCountersPerBlockArr,
                                                            shadowPtr->pendingBlockToUpdateArr,
                                                            protocolStack,
                                                            shadowPtr->numOfLpmMemories);
        if (retVal!=GT_OK)
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            return retVal;
        }
    }
    else
    {
        /* reset pending array for future use */
        cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
    }

    return retVal;
}



/**
* @internal prvCpssDxChLpmRamMcEntryDelete function
* @endinternal
*
* @brief   To delete a particular mc route
*
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroupPtr               - The IP multicast group address.
* @param[in] ipGroupPrefix            - The number of bits that are actual valid in,
*                                      the ipGroup. (CIDR like)
* @param[in] ipSrcArr[]               - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefix              - The number of bits that are actual valid in,
*                                      the ipSrcArr. (CIDR like)
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*                                      roollBack                - GT_TRUE: rollback is taking place.
*                                      GT_FALSE: otherwise.
* @param[in] rollBackSrcBucketPtr     - pointer to src bucket.(used only for rollback)
*
* @retval GT_OK                    - on success, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In order to delete the all the multicast entries, call this function
*       with ipSrcArr[0] = ipSrcPrefix = 0.
*
*/
GT_STATUS prvCpssDxChLpmRamMcEntryDelete
(
    IN GT_U32                                   vrId,
    IN GT_U8                                    *ipGroupPtr,
    IN GT_U32                                   ipGroupPrefix,
    IN GT_U8                                    ipSrcArr[],
    IN GT_U32                                   ipSrcPrefix,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT     protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC         *shadowPtr,
    IN GT_BOOL                                  rollBack,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *rollBackSrcBucketPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *srcBucket;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *defMcRoutePointer;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT         defMcRoutePointerType;

    PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT lpmPtr; /* Will hold the pointer to the IP-src LPM  */
                                                     /* structure for delete operations.         */

    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT newBucketType;  /* Holds the LPM first bucket type after    */
                                /* delete operations.                       */

    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *delEntry; /* A pointer to the data saved in the trie, */
                                /* for IP-src LPM history purposes. And     */
                                /* should be deleted.                       */

    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmEngineMemPtr;
    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT rootBucketFlag;
    GT_STATUS retVal;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT rootBucketType;  /* The bucket type of the root bucket*/
                                        /* of the LPM structure that holds  */
                                        /* the IP-Mc src prefixes.          */
    GT_BOOL alreadyExists = GT_FALSE;   /* Indicates if the given Src prefix*/
                                        /* already exists in the LPM        */
                                        /* structure.                       */
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC parentWriteFuncInfo;

    SHADOW_IP_MC_GROUP_INSERT_FUNC_PARAMS_STC writeFuncData;
    GT_BOOL     deleteEmptyGroup = GT_FALSE;   /* if group is empty and need to be deleted */
    GT_BOOL     isFullGroupMask = GT_FALSE;

    /* first check the ip version was initialized */
    if (shadowPtr->isProtocolInitialized[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if ((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) &&
        (ipGroupPrefix <= PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS))
    {
        isFullGroupMask = GT_TRUE;
        ipGroupPrefix = PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS;
    }
    else if ((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) &&
             (ipGroupPrefix <= PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS))
    {
        isFullGroupMask = GT_TRUE;
        ipGroupPrefix = PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS;
    }

    /* can't delete the default route. only overwrite it */
    if ((isFullGroupMask == GT_TRUE) && (ipSrcPrefix == 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((vrId >= shadowPtr->vrfTblSize) ||
       (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(((GT_STATUS)GT_ERROR), LOG_ERROR_NO_MSG);
    }

    /* verify that multicast is supported for this protocol in the VR */
    if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* we can't use compressed 2 as a root type for the src trie */
    rootBucketFlag = PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E;

     /* src lpm search is done in the same way as UC lpm */
    lpmEngineMemPtr = shadowPtr->ucSearchMemArrayPtr[protocolStack];

    retVal = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
                                             &shadowPtr->freeMemListEndOfUpdate,shadowPtr);
    if (retVal != GT_OK)
        return (retVal);

    if (rollBack == GT_FALSE)
    {
        retVal = prvCpssDxChLpmRamMcGroupSearch(vrId,ipGroupPtr,ipGroupPrefix,
                                                &srcBucket,shadowPtr,protocolStack);
    }
    else
    {
        srcBucket = rollBackSrcBucketPtr;
    }

    /* if the group wasn't found */
    if(retVal != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* get the default route */
    retVal = prvCpssDxChLpmRamMcDefRouteGet(vrId,&defMcRoutePointer,
                                            shadowPtr,protocolStack);
    if (retVal != GT_OK)
    {
        /* shouldn't happen, there should always be a default */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* if we delete (G,*) */
    if(ipSrcPrefix == 0)
    {
        /* if the (G,*) is implicit, it can't be deleted explicitly */
        if (srcBucket->isImplicitGroupDefault == GT_TRUE)
        {
            if (rollBack == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
            }
        }

        /* if there are no sources attached to the group, delete the group */
        if (srcBucket->bucketType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
        {
            /* mark the group as need to be deleted */
            deleteEmptyGroup = GT_TRUE;
        }
        /* there are still sources attached to the group, therefore just */
        /* set the group default next hop to the MC default hext hop     */
        else
        {
            /* get next hop attached to (G,*) */
            retVal = prvCpssDxChLpmRamMngSearch(srcBucket,ipSrcArr,ipSrcPrefix,(GT_PTR*)(GT_UINTPTR)&delEntry);
            if (retVal != GT_OK)
            {
                /* shouldn't happen */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            /* Note we already know the (G,*) is explicit.
               Replace the next hop with MC default next hop and
               mark the (G,*) as implicit */

            /* set root bucket type */
            rootBucketType = srcBucket->bucketType;

            /* src lpm search is done in the same way as UC lpm */
            lpmEngineMemPtr = shadowPtr->ucSearchMemArrayPtr[protocolStack];

            PRV_CPSS_DXCH_LPM_NEXT_PTR_TYPE_CONVERT_MAC(defMcRoutePointer->routeEntryMethod,defMcRoutePointerType);
            /* Insert the complete ip address to the lpm structure of PP */
            retVal = prvCpssDxChLpmRamMngInsert(srcBucket, ipSrcArr, ipSrcPrefix, defMcRoutePointer,
                                                defMcRoutePointerType,
                                                rootBucketFlag, lpmEngineMemPtr,
                                                &alreadyExists,&rootBucketType,
                                                shadowPtr, PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                GT_TRUE,
                                                &parentWriteFuncInfo,protocolStack,PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E);
            if (retVal != GT_OK)
                return retVal;

            retVal = prvCpssDxChLpmRamMcGroupInsert(vrId,ipGroupPtr,ipGroupPrefix,
                                                    srcBucket,shadowPtr,
                                                    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                    protocolStack);
            if (retVal != GT_OK)
                return retVal;

            /* free old next hop */
            cpssOsLpmFree(delEntry);

            /* mark the (G,*) as implicit */
            srcBucket->isImplicitGroupDefault = GT_TRUE;
        }
    }
    /* we delete (G,S) and not (G,*) */
    else
    {
        /* update the info for the parent write function */
        parentWriteFuncInfo.bucketParentWriteFunc =
            coreIpMcGroupInsertFuncWrap;

        writeFuncData.vrId         = vrId;
        writeFuncData.ipGroup     = ipGroupPtr;
        writeFuncData.ipGroupPrefix = ipGroupPrefix;
        writeFuncData.srcBucketPtr = srcBucket;
        writeFuncData.shadowPtr = shadowPtr;
        writeFuncData.insertMode = PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E;
        writeFuncData.protocol   = protocolStack;

        parentWriteFuncInfo.data = (GT_PTR)(&writeFuncData);

        /* get the old bucket type */
        newBucketType = srcBucket->bucketType;

        lpmPtr.nextBucket = srcBucket;

        /* Delete the given ip-src from the LPM structure   */
        retVal = prvCpssDxChLpmRamMngEntryDelete(&lpmPtr,ipSrcArr,ipSrcPrefix,rootBucketFlag,
                                                 GT_TRUE,lpmEngineMemPtr,&newBucketType,
                                                 (GT_PTR*)(GT_UINTPTR)&delEntry,shadowPtr,
                                                 &parentWriteFuncInfo,GT_TRUE,rollBack,protocolStack,
                                                 PRV_CPSS_DXCH_LPM_ALLOC_MC_SRC_TYPE_E);
        if(retVal != GT_OK)
        {
            prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                            &shadowPtr->freeMemListEndOfUpdate,shadowPtr);
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            return retVal;
        }

        if(retVal == GT_OK)
        {
            cpssOsLpmFree(delEntry);
        }
        if (rollBack == GT_FALSE)
        {
            retVal = prvCpssDxChLpmRamMcGroupInsert(vrId,ipGroupPtr,ipGroupPrefix,
                                                    srcBucket,shadowPtr,
                                                    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                    protocolStack);
            if (retVal != GT_OK)
            {
                return retVal;
            }
        }
        else
        {
            if (!((srcBucket->bucketType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
            (srcBucket->isImplicitGroupDefault == GT_TRUE)))
            {
                retVal = prvCpssDxChLpmRamMcGroupInsert(vrId,ipGroupPtr,ipGroupPrefix,
                                                        srcBucket,shadowPtr,
                                                        PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                        protocolStack);
            }
        }

        /* if there are no more sources attached to the group and the (G,*) is
           implicit, then delete the group */
        if ((srcBucket->bucketType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
            (srcBucket->isImplicitGroupDefault == GT_TRUE))
        {
            /* mark the group as need to be deleted */
            deleteEmptyGroup = GT_TRUE;
        }
    }

    /* if the group was marked as need to be deleted, delete it */
    if (deleteEmptyGroup == GT_TRUE)
    {
        retVal = prvCpssDxChLpmRamMcGroupDelete(vrId,ipGroupPtr,ipGroupPrefix,
                                                &srcBucket,shadowPtr,protocolStack,rollBack);
        if(retVal != GT_OK)
        {
            prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                            &shadowPtr->freeMemListEndOfUpdate,shadowPtr);

            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            return retVal;
        }

        retVal = prvCpssDxChLpmRamMngBucketDelete(srcBucket,8,(GT_PTR*)(GT_UINTPTR)&delEntry);

        if(retVal != GT_OK)
        {
            /*No need to check the return value of prvCpssDxChLpmMemFreeListMng
              because in this scenario we will return the error from
              prvCpssExMxPmLpmDeleteBucket */
            prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                            &shadowPtr->freeMemListEndOfUpdate,shadowPtr);

            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            return retVal;
        }

        if ((isFullGroupMask == GT_TRUE) && (srcBucket->isImplicitGroupDefault == GT_TRUE))
        {
            retVal = prvCpssDxChLpmRamMcDefRouteSet(vrId, defMcRoutePointer,
                                                    shadowPtr,
                                                    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                    protocolStack);
            if (retVal != GT_OK)
            {
                 /* reset pending array for future use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

                return retVal;
            }
        }

        /* check if this is the default next hop , if not free it */
        if(delEntry != defMcRoutePointer)
        {
            cpssOsLpmFree(delEntry);
        }
    }

    retVal = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                             &shadowPtr->freeMemListEndOfUpdate,shadowPtr);

    /* in case of rollBack no need to update counters in the "delete" operation
       since the counters were not updated in the "add" operation.
       counters are updated at the end of operation in case of success */
    if ((retVal==GT_OK)&&(rollBack == GT_FALSE))
    {
        /* update counters for MC allocation */
        retVal = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                            shadowPtr->pendingBlockToUpdateArr,
                                                            shadowPtr->protocolCountersPerBlockArr,
                                                            shadowPtr->pendingBlockToUpdateArr,
                                                            protocolStack,
                                                            shadowPtr->numOfLpmMemories);
        if (retVal!=GT_OK)
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            return retVal;
        }
    }
    else
    {
        /* reset pending array for future use */
        cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamMcEntriesFlush function
* @endinternal
*
* @brief   flushes the multicast IP Routing table and stays with the default entry
*         only.
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] protocolStack            - protocol to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamMcEntriesFlush
(
    IN GT_U32                               vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr
)
{
    GT_U8 ipGroup[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS]; /* The Multicast group IP address to be deleted at each loop */
    GT_U32 ipGroupPrefix;   /* The Multicast group IP address prefix to be deleted. */
    GT_U8 ipSrc[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];   /* The source IP address to be deleted */
    GT_U32 ipSrcPrefix;     /* The source IP address prefix to be deleted */
    GT_U32 bankIndex=0,firstBlockIndex=0,secondBlockIndex=0;

    GT_STATUS retVal = GT_OK;

    /* first check the ip version was initialized */
    if (shadowPtr->isProtocolInitialized[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if((vrId >= shadowPtr->vrfTblSize) ||
       (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(((GT_STATUS)GT_ERROR), LOG_ERROR_NO_MSG);
    }

    while(1)
    {
        if (protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
        {
            ipGroup[0] = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS;
            ipGroupPrefix = PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS;
        }
        else
        {
            ipGroup[0] = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            ipGroupPrefix = PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS;
        }
        cpssOsMemSet(&ipGroup[1], 0, sizeof(GT_U8) * (PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS - 1));
        cpssOsMemSet(ipSrc, 0, sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);
        ipSrcPrefix = 0;

        /* get next entry */
        retVal = prvCpssDxChLpmRamMcEntryGetNext(vrId,ipGroup,&ipGroupPrefix,
                                                 ipSrc,&ipSrcPrefix,NULL,
                                                 protocolStack,shadowPtr);

        /* check if there are no more entries */
        if (retVal == GT_NOT_FOUND)
        {
            retVal = GT_OK;
            break;
        }

        if (retVal != GT_OK)
            return retVal;

        /* delete the enrty found */
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmSip6RamMcEntryDelete(vrId, ipGroup, ipGroupPrefix, ipSrc,
                                                        ipSrcPrefix,protocolStack,shadowPtr,GT_FALSE,NULL);

        }
        else
        {
            retVal = prvCpssDxChLpmRamMcEntryDelete(vrId, ipGroup, ipGroupPrefix, ipSrc,
                                                    ipSrcPrefix,protocolStack,shadowPtr,GT_FALSE,NULL);
        }

        if (retVal != GT_OK)
            return retVal;
    }

    /* release banks from its mapping */
       /* if the bank is marked as used we need to check if it is empty,
       if it is we need to unbind it from current "octet to bank mapping" and reuse it
       bank0 should never be released. For all the protocols octet 0 is mapped to bank 0.
       In Sip6 the bank related to octet1 should never be released */

    /* octet0 mapping*/
    firstBlockIndex = shadowPtr->lpmMemInfoArray[protocolStack][0].ramIndex;
    /* octet1 mapping*/
    secondBlockIndex = shadowPtr->lpmMemInfoArray[protocolStack][1].ramIndex;

    for (bankIndex=0; bankIndex < shadowPtr->numOfLpmMemories; bankIndex++)
    {
        if (((shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)&&
            ((bankIndex==firstBlockIndex)||(bankIndex==secondBlockIndex)))||
            ((shadowPtr->shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)&&
             (bankIndex==firstBlockIndex)))
        {
            /* do not release the mapping */
            continue;
        }

        if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,protocolStack,bankIndex)==GT_TRUE)
        {
            retVal = prvCpssDxChLpmRamMngUnbindBlock(shadowPtr, bankIndex);
            if (retVal!=GT_OK)
            {
                return retVal;
            }
        }
    }

    return retVal;
}


/**
* @internal prvCpssDxChLpmRamMcEntryGetNext function
* @endinternal
*
* @brief   This function returns the next muticast (ipSrc,ipGroup) entry.
*
* @param[in] vrId                     - The virtual router Id.
* @param[in,out] ipGroupPtr               - The ip Group address to get the next entry for.
* @param[in,out] ipGroupPrefixPtr         - ipGroup prefix length.
* @param[in,out] ipSrcPtr                 - The ip Source address to get the next entry for.
* @param[in,out] ipSrcPrefixPtr           - ipSrc prefix length.
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in,out] ipGroupPtr               - The next ip Group address.
* @param[in,out] ipGroupPrefixPtr         - ipGroup prefix length.
* @param[in,out] ipSrcPtr                 - The next ip Source address. (must be 16 x GT_U8!)
* @param[in,out] ipSrcPrefixPtr           - ipSrc prefix length.
*
* @param[out] mcRoutePointerPtr        the mc route entry (NH) pointer.
*                                       GT_OK if found, or
*
* @retval GT_NOT_FOUND             - If the given address is the last one on the IP-Mc table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The values of (ipGroup,ipGroupPrefix) must be a valid values, it
*       means that they exist in the IP-Mc Table, unless this is the first
*       call to this function, then the value of (ipGroupPrefix) is (0).
*
*/
GT_STATUS prvCpssDxChLpmRamMcEntryGetNext
(
    IN    GT_U32                                    vrId,
    INOUT GT_U8                                     *ipGroupPtr,
    INOUT GT_U32                                    *ipGroupPrefixPtr,
    INOUT GT_U8                                     *ipSrcPtr,
    INOUT GT_U32                                    *ipSrcPrefixPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *mcRoutePointerPtr,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocolStack,
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *bucketPtr;   /* Holds a pointer to the shadow root*/
                                /* bucket of the ip-src LPM structure.      */
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopEntry = NULL;

    GT_BOOL newGroup;           /* Indicates whether this is the first time */
                                /* a certain group is searched.             */

    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *defMcRoutePointer;
    GT_STATUS retVal;

    /* first check the ip version was initialized */
    if (shadowPtr->isProtocolInitialized[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    newGroup        = GT_FALSE;

    if ((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) && (*ipGroupPrefixPtr == 0))
    {
        ipGroupPtr[0] = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS;
    }

    if ((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) && (*ipGroupPrefixPtr == 0))
    {
        ipGroupPtr[0] = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS;
    }

    if ((((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) &&
          (ipGroupPtr[0] == PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS) &&
          (*ipGroupPrefixPtr <= PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS)) ||
         ((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) &&
          (ipGroupPtr[0] == PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS) &&
          (*ipGroupPrefixPtr <= PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS))) &&
        (*ipSrcPrefixPtr == 0))
    {
        *ipGroupPrefixPtr =
            (protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
            PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS :
            PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS;

        /* search for 224 (IPv4) or 255 (IPv6) */
        retVal = prvCpssDxChLpmRamMcGroupSearch(vrId,ipGroupPtr,*ipGroupPrefixPtr,
                                                &bucketPtr,shadowPtr,protocolStack);
        if ((retVal != GT_OK) || (bucketPtr == NULL))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }

        if ((GT_PTR*)bucketPtr == (GT_PTR*)shadowPtr->vrRootBucketArray[vrId].multicastDefault[protocolStack])
        {
            /* if we reached here than we don't have a 224/4 (IPv4) or 255/8 (IPv6) + src.
               For 224/4 (IPv4) or 255/8 (IPv6) + src prvCpssDxChLpmRamMcGroupSearch
               set bucketPtr to be different than the default nexthop */
            retVal = cpssDxChLpmRamMcGroupGetNext(vrId, GT_TRUE, ipGroupPtr,
                                              ipGroupPrefixPtr,
                                              &bucketPtr,
                                              shadowPtr,protocolStack);
            if ((retVal != GT_OK) || (bucketPtr == NULL))
            {
                return /* do not log this error */ GT_NOT_FOUND;
            }
        }
        newGroup = GT_TRUE;

        if (protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
        {
            cpssOsMemSet(ipSrcPtr,0,sizeof(GT_U8)*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS);
        }
        else
        {
            cpssOsMemSet(ipSrcPtr,0,sizeof(GT_U8)*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS);
        }

        (*ipSrcPrefixPtr)  = 0;
    }
    else
    {
        retVal = prvCpssDxChLpmRamMcGroupSearch(vrId,ipGroupPtr,*ipGroupPrefixPtr,
                                                &bucketPtr,shadowPtr,protocolStack);
        if(retVal != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
    }

    /* Search for the IP-src part of the address.   */
    while(1)
    {
        /* return the entry associated with IP-src (0,0)   */
        if(newGroup == GT_TRUE)
        {
            prvCpssDxChLpmRamMngSearch(bucketPtr, ipSrcPtr, 0, (GT_PTR*)(GT_UINTPTR)&nextHopEntry);
            retVal = GT_OK;
        }

        /* get the default group for comparison  */
        if (prvCpssDxChLpmRamMcDefRouteGet(vrId,&defMcRoutePointer,
                                           shadowPtr,protocolStack) != GT_OK)
            /* can't happen - it means we have no default */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

        /* incase we recived in the src (0,0) the default entry, it means this
           is not a valid G,* , which means we need to treat as looking for G,S*/
        if ((newGroup == GT_FALSE) ||
            ((nextHopEntry != NULL) &&
             (nextHopEntry == defMcRoutePointer)))
        {
            retVal = prvCpssDxChLpmRamMngEntryGet(bucketPtr, protocolStack, ipSrcPtr, ipSrcPrefixPtr,
                                                  (GT_PTR*)(GT_UINTPTR)&nextHopEntry);
        }

        /* No ip-src addresses left for this group, move to the next group. */
        if(retVal != GT_OK)
        {
            retVal =
                cpssDxChLpmRamMcGroupGetNext(vrId,GT_FALSE, ipGroupPtr,
                                             ipGroupPrefixPtr,
                                             &bucketPtr,
                                             shadowPtr,protocolStack);
            if ((retVal != GT_OK) || (bucketPtr == NULL))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
            }

            newGroup        = GT_TRUE;

            if (protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                cpssOsMemSet(ipSrcPtr,0,sizeof(GT_U8)*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS);
            }
            else
            {
                cpssOsMemSet(ipSrcPtr,0,sizeof(GT_U8)*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS);
            }

            (*ipSrcPrefixPtr)  = 0;
            newGroup        = GT_TRUE;
        }
        else    /* A new entry was found.   */
        {
            /* if needed copy the mcRoute (NH) */
            if (mcRoutePointerPtr != NULL)
            {
                cpssOsMemCpy(mcRoutePointerPtr,nextHopEntry,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
            }
            return GT_OK;
        }
    }
}

/**
* @internal prvCpssDxChLpmRamMcEntrySearch function
* @endinternal
*
* @brief   This function searches for the route entries associates with a given
*         (S,G), prefixes.
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipGroupPtr               - The ip Group address to get the next entry for.
* @param[in] ipGroupPrefix            - ipGroupPtr prefix length.
* @param[in] ipSrcArr[]               - The ip Source address to get the next entry for.
* @param[in] ipSrcPrefix              - ipSrcArr prefix length.
* @param[in] protocolStack            - the protocol to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] ipNextHopEntryPtr        - A pointer to a next hop struct including the HW
*                                      pointer
*                                       GT_OK if successful, or
*                                       GT_NOT_FOUND if the given prefix was not found.
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Calling this function with ipGroupPrefix = 0, will return the default
*       Mc route entries.
*
*/
GT_STATUS prvCpssDxChLpmRamMcEntrySearch
(
    IN GT_U32                                       vrId,
    IN GT_U8                                        *ipGroupPtr,
    IN GT_U32                                       ipGroupPrefix,
    IN GT_U8                                        ipSrcArr[],
    IN GT_U32                                       ipSrcPrefix,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *ipNextHopEntryPtr,
    IN CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *srcBucket;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopEntry;/* Next hop entry stored in the */
                                                          /* Mc-src lpm structure.        */
    /* first check the ip version was initialized */
    if (shadowPtr->isProtocolInitialized[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* verify that multicast is supported for this protocol in the VR */
    if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if (ipNextHopEntryPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (((ipGroupPrefix == PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS) &&
         (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)) ||
        ((ipGroupPrefix == PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS) &&
         (protocolStack == CPSS_IP_PROTOCOL_IPV6_E)))
    {
        if (GT_OK !=
            prvCpssDxChLpmRamMcDefRouteGet(vrId,&nextHopEntry,
                                           shadowPtr,protocolStack))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if (prvCpssDxChLpmRamMcGroupSearch(vrId,ipGroupPtr,ipGroupPrefix,
                                           &srcBucket,shadowPtr,protocolStack)
            != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }

        if (prvCpssDxChLpmRamMngSearch(srcBucket,ipSrcArr,ipSrcPrefix,(GT_PTR*)(GT_UINTPTR)&nextHopEntry) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
    }

    cpssOsMemCpy(ipNextHopEntryPtr,nextHopEntry,
                 sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamMcDefRouteGet function
* @endinternal
*
* @brief   retrieves the default MC route for the given VR router.
*
* @param[in] vrId                     - The  to which this tree belongs to.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] protocol                 - the protocol
*
* @param[out] defMcRoutePointerPtr     - A pointer to the default mc route
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssDxChLpmRamMcDefRouteGet
(
    IN  GT_U32                                    vrId,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC **defMcRoutePointerPtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol
)
{
    *defMcRoutePointerPtr =
        shadowPtr->vrRootBucketArray[vrId].multicastDefault[protocol];

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamMcTraverse function
* @endinternal
*
* @brief   a tree traverse function to set/retrive/count all the memory allocations
*         in this mc , and it's lpm src tree. it is done in DFS fashion.
* @param[in] traverseOp               - the traverse operation needed.
* @param[in] memAllocArrayPtr         - the array that holds the information of memory
*                                      allocations.
* @param[in] memAllocArrayIndexPtr    - the index in the array this function should start
*                                      using.
* @param[in] memAllocArraySize        - the size of the above array.
* @param[in] vrId                     - The  to which this table belongs to.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] protocol                 - the protocol
*
* @param[out] stopPointIterPtr         - an iterator that is returned to point where we stopped
*                                      in the lpm tree, so we can continue from there in the
*                                      next call.
*
* @retval GT_OK                    - If there is enough memory for the insertion.
* @retval GT_OUT_OF_PP_MEM         - otherwise.
*
* @note if stopPointIter ==0 we start at the begining.
*       and if the returned stopPointIter==0 the we finished with this tree.
*
*/
GT_STATUS prvCpssDxChLpmRamMcTraverse
(
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_OP_ENT    traverseOp,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC     *memAllocArrayPtr,
    IN GT_U32                                       *memAllocArrayIndexPtr,
    IN GT_U32                                       memAllocArraySize,
    IN GT_U32                                       vrId,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    OUT GT_UINTPTR                                  *stopPointIterPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol
)
{
    GT_STATUS retVal;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC tmpRange;

    tmpRange.lowerLpmPtr.nextBucket =
        shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol];
    tmpRange.pointerType =
        shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol];
    tmpRange.next = NULL;
    retVal = prvCpssDxChLpmRamMngMemTraverse(traverseOp,memAllocArrayPtr,
                                             memAllocArrayIndexPtr,
                                             memAllocArraySize,&tmpRange,GT_FALSE,
                                             shadowPtr->mcSearchMemArrayPtr[protocol],
                                             stopPointIterPtr);
    return retVal;
}

/**
* @internal prvCpssDxChIpLpmRamMcEntryActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific MC entry
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in] protocol                 - type of  stack to work on.
* @param[in] ipGroupPtr               - Pointer to the ip Group address to get the entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrcPtr                 - Pointer to the ip Source address to get the entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
* @param[in] clearActivity            - Indicates to clear activity status.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDxChIpLpmRamMcEntryActivityStatusGet
(
    IN  GT_U32                              vrId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT          protocol,
    IN  GT_U8                               *ipGroupPtr,
    IN  GT_U32                              ipGroupPrefixLen,
    IN  GT_U8                               *ipSrcPtr,
    IN  GT_U32                              ipSrcPrefixLen,
    IN  GT_BOOL                             clearActivity,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
)
{
    GT_STATUS   rc;
    GT_U32      entryIndex;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *rootBucketPtr;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *srcBucketPtr;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *bucketPtr;
    CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT rangeSelectSec;
    GT_U32 numOfRanges;
    CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC nextPointerArray[256];
    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
    GT_U32 shareDevListLen;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC  *startRange;
    GT_U8  lastIpPtr;
    GT_U8  startAddr;
    GT_U8  endAddr;

    CPSS_NULL_PTR_CHECK_MAC(activityStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(ipGroupPtr);
    CPSS_NULL_PTR_CHECK_MAC(ipSrcPtr);

    rootBucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol];
    rc = prvCpssDxChLpmRamMngSearch(rootBucketPtr, ipGroupPtr,
            ipGroupPrefixLen, (GT_PTR)&srcBucketPtr);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    /* Look for offset/index */
    rc = prvCpssDxChLpmRamMngSearchOffset(srcBucketPtr, ipSrcPtr, ipSrcPrefixLen, &entryIndex, &bucketPtr);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;

    /* Read node from the hw by index. It's enough to read only first device  */
    rc = cpssDxChLpmNodeRead(shareDevsList[0], entryIndex, bucketPtr->bucketType, &rangeSelectSec, &numOfRanges, (CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC *)&nextPointerArray);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    /* From printLpm debugging:
     * 1.1.1.2 / 32
     * rangeStart[0] = 0 - always 0
     * rangeStart[1] = 0x2 rangeStart[2] = 0x3 rangeStart[3] =  0 rangeStart[4] =  0
     * rangeStart[3] = 0 - end of ranges if 0
     * So:
     * 0..1     range[i+1] - 1 == 2-1 == 1
     * 2..2     range[i+1] - 1 == 3-1 == 2
     * 3..255   range[i+1] - 1 == 0-1 == 255
     */

    /* Last address byte index depends on protocol */
    if(protocol == CPSS_IP_PROTOCOL_IPV4_E)
    {
        lastIpPtr = 3;
    }
    else
    if(protocol == CPSS_IP_PROTOCOL_IPV6_E)
    {
        lastIpPtr = 15;
    }
    else
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* Look for appropriate range in the range list.
     * Indexes in the range list and in the nextPointerArray[] are same.
     */
    startRange = bucketPtr->rangeList;
    for(i = 0; i< numOfRanges; i++){
        startAddr = startRange->startAddr;
        endAddr = 0;
        if(startRange->next)
            endAddr = startRange->next->startAddr;
        /* check last address byte within range */
        if(startAddr <= ipSrcPtr[lastIpPtr] && ipSrcPtr[lastIpPtr]<= (GT_U8)(endAddr-1)) break;
        /* next range */
        startRange = startRange->next;
    }
    /* That's why we are here */
    *activityStatusPtr = nextPointerArray[i].pointerData.nextHopOrEcmpPointer.activityState;

    /* if status set and we need to clear it */
    if(*activityStatusPtr && clearActivity)
    {
        nextPointerArray[i].pointerData.nextHopOrEcmpPointer.activityState = GT_FALSE;
        /* Write to all devices in the dev list */
        for(i = 0; i < shareDevListLen; i++)
        {
            /* write node to the hw */
            rc = cpssDxChLpmNodeWrite(shareDevsList[i], entryIndex, bucketPtr->bucketType, &rangeSelectSec, numOfRanges, (CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC *)&nextPointerArray);
        }
        if (rc != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChIpLpmSip6RamMcEntryActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific MC entry
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2, Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in] protocol                 - type of  stack to work on.
* @param[in] ipGroupPtr               - Pointer to the ip Group address to get the entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrcPtr                 - Pointer to the ip Source address to get the entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
* @param[in] clearActivity            - Indicates to clear activity status.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDxChIpLpmSip6RamMcEntryActivityStatusGet
(
    IN  GT_U32                              vrId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT          protocol,
    IN  GT_U8                               *ipGroupPtr,
    IN  GT_U32                              ipGroupPrefixLen,
    IN  GT_U8                               *ipSrcPtr,
    IN  GT_U32                              ipSrcPrefixLen,
    IN  GT_BOOL                             clearActivity,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
)
{
    GT_STATUS   rc;
    GT_U32 entryIndex[4] = {0};
    GT_U32 indexInLeafLine[4] = {0};
    GT_U32 leavesNumber = 0;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *rootBucketPtr;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *srcBucketPtr;
    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
    GT_U32 shareDevListLen;
    GT_U32  i,j,k;
    GT_U32 agingData = 0,tmpAgingData = 0;
    GT_U32 agingBitNumber = 0;
    GT_U32 agingBitMask = 0;
    GT_U32 agingEntryIndex = 0;
    GT_U32 bankNumber;
    GT_U32 offsetInBank;
    GT_U32 isActive = 0;
    GT_U32 numOfTiles = 0;
    GT_U32 hwBankSize = 0;

    CPSS_NULL_PTR_CHECK_MAC(activityStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(ipGroupPtr);
    CPSS_NULL_PTR_CHECK_MAC(ipSrcPtr);

    rootBucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol];
    rc = prvCpssDxChLpmRamMngSearch(rootBucketPtr, ipGroupPtr,
            ipGroupPrefixLen, (GT_PTR)&srcBucketPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* Look for offset/index */
    rc = prvCpssDxChLpmSip6RamMngSearchLeafOffset(srcBucketPtr, ipSrcPtr, ipSrcPrefixLen, entryIndex,
                                                  indexInLeafLine, &leavesNumber);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChLpmSip6RamMngSearchLeafOffset is failed");
    }
    if (leavesNumber > 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "wrong leaves number");
    }

    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;

    for (i =0; i < leavesNumber; i++)
    {
         bankNumber = entryIndex[i]/shadowPtr->lpmRamTotalBlocksSizeIncludingGap;
         if (bankNumber > 29)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "wrong leaves number");
         }

         for(j = 0; j < shareDevListLen; j++)
         {
             offsetInBank = entryIndex[i]%shadowPtr->lpmRamTotalBlocksSizeIncludingGap;
             hwBankSize = prvCpssDxChIpLpmSip6LpmRamDeviceSpecificBankSizeGet(shareDevsList[j]);
             if (bankNumber <= 28)
             {
                 agingBitNumber = MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS * (bankNumber * hwBankSize + offsetInBank) + indexInLeafLine[i];
             }
             else
             {
                 /* bank 29 */
                 agingBitNumber = MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS *(28 * hwBankSize + 640 + offsetInBank) + indexInLeafLine[i];
             }
             agingEntryIndex = agingBitNumber/32;

             numOfTiles = PRV_CPSS_PP_MAC(shareDevsList[j])->multiPipe.numOfTiles;
             numOfTiles = numOfTiles ? numOfTiles : 1;
             agingData = 0;
             for (k =0; k < numOfTiles; k++)
             {
                 tmpAgingData = 0;
                 rc = prvCpssDxChPortGroupReadTableEntry(shareDevsList[j], k*2,
                                               CPSS_DXCH_SIP6_TABLE_LPM_AGING_E,
                                               agingEntryIndex,
                                               &tmpAgingData);
                 if (rc != GT_OK)
                 {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "LPM Aging entry read is failed");
                 }
                 agingData = agingData | tmpAgingData;
             }
         }

         /* check if aging bit is set. In this case if clearActivity is set, need to clear it */
         agingBitMask = FIELD_MASK_MAC(agingBitNumber%32,1);
         isActive = agingBitMask & agingData;
         *activityStatusPtr = (isActive == 0 )? GT_FALSE : GT_TRUE;
         if ((*activityStatusPtr == GT_TRUE) && (clearActivity == GT_TRUE))
         {
             agingData = agingData & (~agingBitMask);
             /* clear the bit */
              for(j = 0; j < shareDevListLen; j++)
              {
                  rc = prvCpssDxChWriteTableEntry(shareDevsList[j],
                                                 CPSS_DXCH_SIP6_TABLE_LPM_AGING_E,
                                                 agingEntryIndex,
                                                 &agingData);
                  if (rc != GT_OK)
                  {
                      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "LPM Aging entry write is failed");
                  }
              }
         }
    }

    return GT_OK;
}

