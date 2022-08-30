/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file prvCpssDxChFlowManager.c
*
* @brief Private CPSS APIs for Flow Manager.
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/flowManager/prvCpssDxChFlowManagerDb.h>
#include <cpss/dxCh/dxChxGen/flowManager/cpssDxChFlowManager.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuLoad.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgFdbHashCrc.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfix.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* Update delta based on wrap around check */
#define FLOW_WRAP_AROUND_CHECK(currentValue, prevReadValue, delta) \
    if (currentValue >= prevReadValue)                              \
    {                                                              \
        delta = currentValue - prevReadValue;                      \
    }                                                              \
    else                                                           \
    {                                                              \
        delta = (2 * BIT_31 + currentValue) - prevReadValue;       \
    }                                                              \

/**
* @internal calcFlowHashAndKey_ipv4 function
* @endinternal
*
* @brief To calculate the flow hash value and its key from the message packeti for ipv4 flows.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in]  flowMngDbPtr   - (pointer to) flow manager db
* @param [in]  flowData       - (pointer to) flow data
* @param [out] flowHashPtr    - (pointer to) flow hash
* @param [out] flowKeyPtr     - (pointer to) flow key
* @param [out] flowKeySizePtr - (pointer to) flow key size
*
* @retval GT_OK                        - on success.
* @retval GT_BAD_PTR                   - on NULL inoput ptr.
*/
static GT_STATUS calcFlowHashAndKey_ipv4
(
    IN  CPSS_DXCH_FLOW_MNG_PTR          flowMngDbPtr,
    IN  CPSS_DXCH_FLOW_MANAGER_FLOW_STC *flowData,
    OUT GT_U16                          *flowHashPtr,
    OUT GT_U8                           *flowKeyPtr,
    OUT GT_U8                           *flowKeySizePtr
)
{
    GT_U32 rc = GT_OK;
    GT_U32 hash_bit[32] = {0};
    GT_U32 initArr[2] = {0, 0};
    GT_U32 numOfDataBits = 96;
    GT_U32 hashData[3] = {0};
    GT_U32 iter;

    /* Store flow key size for use during long and short rules differentiation
     */
    *flowKeySizePtr = flowData->keySize;

    for (iter = 0; iter < flowMngDbPtr->cpssFwCfg.reducedRuleSize; iter++)
    {
        flowKeyPtr[iter] = flowData->keyData[iter];
    }

    /* Initial 2 bytes are reserved for PCL-ID hence not included as part of
     * flow key data so maximum flow key size for hash calculation is 28B only for shor rule size of 30B.
     * Doing xor of the 28B data and storing the result into 12B
     * variable/96bits data for the crc hash calculation. The crc calculation
     * operates on the 96bits and uses crc func to give result of 16bits which
     * is then module with the total hash list size i.e _8K.
     */
    if(flowData->keySize == CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_20B_E)
    {
        for(iter = 0; iter < 3; iter++)
        {
            hashData[0] ^= flowData->keyData[iter] ^ flowData->keyData[iter+3];
            hashData[1] ^= flowData->keyData[iter+6] ^ flowData->keyData[iter+9];
            hashData[2] ^= flowData->keyData[iter+12] ^ flowData->keyData[iter+15];
        }
    }
    else if(flowData->keySize == CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E)
    {
        for(iter = 0; iter < 4; iter++)
        {
            hashData[0] ^= flowData->keyData[iter] ^ flowData->keyData[iter+4];
            hashData[1] ^= flowData->keyData[iter+8] ^ flowData->keyData[iter+12];
            hashData[2] ^= flowData->keyData[iter+16] ^ flowData->keyData[iter+20] ^ flowData->keyData[iter+24];
        }
    }
    else if(flowData->keySize == CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_60B_E)
    {
        for(iter = 0; iter < 8; iter++)
        {
            hashData[0] ^= flowData->keyData[iter] ^ flowData->keyData[iter+8];
            hashData[1] ^= flowData->keyData[iter+16] ^ flowData->keyData[iter+24];
            hashData[2] ^= flowData->keyData[iter+32] ^ flowData->keyData[iter+40] ^ flowData->keyData[iter+48];
        }
        hashData[0] ^= (GT_U32)(flowData->keyData[56]);
        hashData[1] ^= (GT_U32)(flowData->keyData[57]);
    }

    prvCpssDxChBrgFdbSip5CrcCalc(CRC_FUNC_16A_E, numOfDataBits,  initArr, hashData,
                                 hash_bit);
    *flowHashPtr = hash_bit[0] % PRV_NUM_OF_HASH_LISTS_CNS;

    return rc;
}

/**
* @internal prvCpssDxChFlowManagerCalcFlowHashAndKey function
* @endinternal
*
* @brief To calculate the flow hash value and its key from the message packet.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in]  flowMngDbPtr   - (pointer to) flow manager db
* @param [in]  flowData       - (pointer to) flow data
* @param [out] flowHashPtr    - (pointer to) flow hash
* @param [out] flowKeyPtr     - (pointer to) flow key
* @param [out] flowKeySizePtr - (pointer to) flow key size
*
* @retval GT_OK                        - on success.
* @retval GT_BAD_PTR                   - on NULL inoput ptr.
*/
GT_STATUS prvCpssDxChFlowManagerCalcFlowHashAndKey
(
    IN  CPSS_DXCH_FLOW_MNG_PTR          flowMngDbPtr,
    IN  CPSS_DXCH_FLOW_MANAGER_FLOW_STC *flowData,
    OUT GT_U16                          *flowHashPtr,
    OUT GT_U8                           *flowKeyPtr,
    OUT GT_U8                           *flowKeySizePtr
)
{
    GT_U32 rc = GT_OK;

    rc = calcFlowHashAndKey_ipv4(flowMngDbPtr, flowData, flowHashPtr, flowKeyPtr, flowKeySizePtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("calcFlowHashAndKey_ipv4 returned not GT_OK\n");
        return rc;
    }

    return rc;
}

/**
* @internal getCpssFlowDbKey_ipv4 function
* @endinternal
*
* @brief To get flow key of the ipv4 flow based on the flow index.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngDbPtr       - (pointer to) flow manager db
* @param [in] flow_index         - flow index for which flow key has to be determined
* @param [out]flowKeyPtr         - (pointer to) flow key
*
* @retval GT_OK                        - on success.
* @retval GT_BAD_PTR                   - on NULL inoput ptr.
*/
static GT_STATUS getCpssFlowDbKey_ipv4
(
    IN  CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    IN  GT_U32                 flow_index,
    OUT GT_U8                  *flowKeyPtr
)
{
    GT_U32 rc  = GT_OK;
    GT_U32 iter;

    /* get flow db key */
    for (iter = 0; iter < flowMngDbPtr->cpssFwCfg.reducedRuleSize; iter++)
    {
        flowKeyPtr[iter] = flowMngDbPtr->flowDataDb[flow_index].keyData[iter];
    }

    return rc;
}

/**
* @internal prvCpssDxChFlowManagerGetFlowDbKey function
* @endinternal
*
* @brief To get flow key of the flow based on the flow index.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngDbPtr       - (pointer to) flow manager db
* @param [in] flow_index         - flow index for which flow key has to be determined
* @param [out]flowKeyPtr         - (pointer to) flow key
*
* @retval GT_OK                          - on success.
* @retval GT_BAD_PTR                   - on NULL inoput ptr.
*/
GT_STATUS prvCpssDxChFlowManagerGetFlowDbKey
(
    IN  CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    IN  GT_U32                 flow_index,
    OUT GT_U8                  *flowKeyPtr
)
{
    GT_U32 rc = GT_OK;

    if(flow_index > PRV_CPSS_DXCH_FLOW_DB_MAX_CNS)
    {
        CPSS_LOG_INFORMATION_MAC("prvCpssDxChFlowManagerGetFlowDbKey returned flow index not in allowed range\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = getCpssFlowDbKey_ipv4(flowMngDbPtr, flow_index, flowKeyPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("prvCpssDxChFlowManagerGetFlowDbKey returned not GT_OK\n");
        return rc;
    }

    return rc;
}

/**
* @internal compareKeys function
* @endinternal
*
* @brief To get flow index of the existing flow based on flow hash and flow key.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngDbPtr          - (pointer to) flow manager db
* @param [in] new_flow_key          - (pointer to) newly calculated flow key to be matched to
* @param [in] db_flow_key           - (pointer to) flow key existing in the db
*
* @retval GT_TRUE                   - on success.
* @retval GT_FALSE                  - on keys not matching with db stored keys.
*/
static GT_BOOL compareKeys_ipv4
(
    IN  CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    IN GT_U8 *new_flow_key,
    IN GT_U8 *db_flow_key
)
{
    GT_U32 iter;

    for (iter = 0; iter < flowMngDbPtr->cpssFwCfg.reducedRuleSize; iter++)
    {
        if (new_flow_key[iter] != db_flow_key[iter])
        {
            return GT_FALSE;
        }
    }

    return GT_TRUE;
}

/**
* @internal compareKeys function
* @endinternal
*
* @brief To get flow index of the existing flow based on flow hash and flow key.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngDbPtr          - (pointer to) flow manager db
* @param [in] new_flow_key          - (pointer to) newly calculated flow key to be matched to
* @param [in] db_flow_key           - (pointer to) flow key existing in the db
*
* @retval GT_TRUE                   - on success.
* @retval GT_FALSE                  - on keys not matching with db stored keys.
*/
static GT_BOOL compareKeys
(
    IN  CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    IN GT_U8 *new_flow_key,
    IN GT_U8 *db_flow_key
)
{
    GT_BOOL compareResult;
    /* TBD */
    /* Take into account key size for ipv6, thus understand also "key part" */

    compareResult = compareKeys_ipv4(flowMngDbPtr, new_flow_key, db_flow_key);

    return compareResult;
}

/**
* @internal prvCpssDxChFlowManagerGetFlowIndex function
* @endinternal
*
* @brief To get flow index of the existing flow based on flow hash and flow key.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngDbPtr          - (pointer to) flow manager db
* @param [in] flow_hash             - flow hash
* @param [in] flow_key              - (pointer to) flow key
* @param [out]flowIndexPtr          - (pointer to) flow index
*
* @retval GT_OK                         - on success.
* @retval GT_BAD_PTR                   - on NULL inoput ptr.
*/
GT_STATUS prvCpssDxChFlowManagerGetFlowIndex
(
    IN  CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    IN  GT_U16                 flow_hash,
    IN  GT_U8                  *flow_key,
    OUT GT_U32                 *flowIndexPtr
)
{
    PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC *hashEntry;
    GT_U8  flowKey[PRV_CPSS_DXCH_FLOW_KEY_SIZE_CNS];

    if (flow_hash >= PRV_NUM_OF_HASH_LISTS_CNS)
    {
        CPSS_LOG_INFORMATION_MAC("prvCpssDxChFlowManagerGetFlowIndex returned as flow hash is not in acceptable range\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* TBD */
    /* look over the hash index [flow hash] list - if not return DB size */
    hashEntry = flowMngDbPtr->hashList[flow_hash]->nextInHashList;

    while(hashEntry)
    {
        prvCpssDxChFlowManagerGetFlowDbKey(flowMngDbPtr, hashEntry->cpssFlowIndex, &flowKey[0]);
        if(GT_TRUE == compareKeys(flowMngDbPtr, &flowKey[0], flow_key))
            break;
        hashEntry = hashEntry->nextInHashList;
    }

    /* Currently return DB size, i.e. new entry */
    if( hashEntry == NULL )
    {
        *flowIndexPtr = PRV_CPSS_FLOW_MANAGER_INVALID_FLOW_INDEX_CNS;
    }
    else
    {
        *flowIndexPtr = hashEntry->cpssFlowIndex;
    }
    return GT_OK;
}

/**
* @internal setCpssFlowDbKey_ipv4 function
* @endinternal
*
* @brief To set the flow key for ipv4 flows in the cpss db based on the specified flow index and tcam rule size.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngDbPtr        - (pointer to) flow manager db
* @param [in] cpssFlowIndex       - flow index
* @param [in] flowKeyPtr          - (pointer to) flow key
*
* @retval GT_OK                           - on success.
* @retval GT_BAD_PTR                   - on NULL inoput ptr.
*/
static GT_STATUS setCpssFlowDbKey_ipv4
(
    IN CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    IN GT_U32                 cpssFlowIndex,
    IN GT_U8                  *flowKeyPtr
)
{
    GT_U32 rc = GT_OK;
    GT_U32 iter;

    /* set flow db key */
    for (iter = 0; iter < flowMngDbPtr->cpssFwCfg.reducedRuleSize; iter++)
    {
        flowMngDbPtr->flowDataDb[cpssFlowIndex].keyData[iter] = flowKeyPtr[iter];
    }

    return rc;
}

/**
* @internal prvCpssDxChFlowManagerSetFlowDbKey function
* @endinternal
*
* @brief To set the flow key in the cpss db based on the specified flow index and tcam rule size.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngDbPtr        - (pointer to) flow manager db
* @param [in] cpssFlowIndex       - flow index
* @param [in] flowKeyPtr          - (pointer to) flow key
*
* @retval GT_OK                           - on success.
* @retval GT_BAD_PTR                   - on NULL inoput ptr.
*/
GT_STATUS prvCpssDxChFlowManagerSetFlowDbKey
(
    IN CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    IN GT_U32                 cpssFlowIndex,
    IN GT_U8                  *flowKeyPtr
)
{
    GT_U32 rc = GT_OK;

    if(cpssFlowIndex > PRV_CPSS_DXCH_FLOW_DB_MAX_CNS)
    {
        CPSS_LOG_INFORMATION_MAC("prvCpssDxChFlowManagerSetFlowDbKey returned as flow index is not in acceptable range\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* flow_key_size - is used here and in other places to distinguish between short and long rules*/
    rc = setCpssFlowDbKey_ipv4(flowMngDbPtr, cpssFlowIndex, flowKeyPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("prvCpssDxChFlowManagerSetFlowDbKey returned not GT_OK\n");
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChFlowManagerAddFlowToDb function
* @endinternal
*
* @brief To add new flow to cpss db and also to its hash list based on hash index.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngDbPtr          - (pointer to) flow manager db
* @param [in] flow_hash             - flow hash
* @param [in] flowKeyPtr            - (pointer to) flow key
* @param [in] fwFlowId              - flow id
*
* @retval GT_OK                           - on success.
* @retval GT_BAD_PTR                   - on NULL inoput ptr.
*/
GT_STATUS prvCpssDxChFlowManagerAddFlowToDb
(
    IN CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    IN GT_U16                 flow_hash,
    IN GT_U8                  *flowKeyPtr,
    IN GT_U32                 fwFlowId
)
{
    GT_U32 rc = GT_OK;
    PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC *toUse;

    /* Get first free index */
    /* There are 2 linked lists maintained to track the used and free list
     * Initially everything is part of free list and with every arrival of
     * new flow, used list is assigned and free list decremeneted by one thus
     * finally exhausting all the used list and then we may have to remove the
     * oldest entry of the used list going by the LRA algorithm but for now
     * this will be handled by the application and so just for sw sake
     * this is done as below.
     */
    if(flowMngDbPtr->cpssFlowFreeListHeader == NULL)
    {
        /* No free index, release the oldest used one. */
        /* Find the oldest used and then:       */
        /* 1. Age out the flow in CPSS DB to make free index for new incoming flow */
        /* 2. Inform application about removed flow by callback.*/
        /* 3. Remove the entry from hash list as well.*/
        /* TBD application callback to notify aged flows */
        toUse = flowMngDbPtr->cpssFlowUsedListTail;
        if(toUse == flowMngDbPtr->cpssFlowUsedListHeader) {
            /* The only entry in the used list */
            flowMngDbPtr->cpssFlowUsedListHeader = NULL;
        } else {
            /* Remove the entry from the used list */
            flowMngDbPtr->cpssFlowUsedListTail = toUse->prev;
            toUse->prev->next = toUse->next;
        }

        /* Remove the entry from the hash list */
        toUse->prevInHashList->nextInHashList = toUse->nextInHashList;

        if( toUse->nextInHashList != NULL ) {
            toUse->nextInHashList->prevInHashList = toUse->prevInHashList;
        }

        /* Some cleaning */
        toUse->nextInHashList = NULL;
        toUse->prevInHashList = NULL;

        /* Application callback to notify aged flows */
        if(flowMngDbPtr->appCbOldFlowRemovalNotificaion)
        {
            flowMngDbPtr->appCbOldFlowRemovalNotificaion(flowMngDbPtr->flowMngId, toUse->cpssFlowIndex);
            flowMngDbPtr->dbRemovedFlowCount++;
        }
    } else {
        toUse = flowMngDbPtr->cpssFlowFreeListHeader;
        flowMngDbPtr->cpssFlowFreeListHeader = flowMngDbPtr->cpssFlowFreeListHeader->next;
    }

    /* Add new flow to CPSS Flow DB*/
    /* Insert the new flow at the head of the Used list */
    if( flowMngDbPtr->cpssFlowUsedListHeader == NULL) {
        /* The first entry in the used list*/
        toUse->next = NULL;
        /*toUse->prev = NULL;*/
        flowMngDbPtr->cpssFlowUsedListTail = toUse;
    } else {
        toUse->next = flowMngDbPtr->cpssFlowUsedListHeader;
        toUse->next->prev = toUse;
        /*toUse->prev = NULL;*/
    }

    flowMngDbPtr->cpssFlowUsedListHeader = toUse;

    /* Add new flow to its hash list */
    toUse->nextInHashList = flowMngDbPtr->hashList[flow_hash]->nextInHashList;
    toUse->prevInHashList = flowMngDbPtr->hashList[flow_hash];
    flowMngDbPtr->hashList[flow_hash]->nextInHashList = toUse;
    if(toUse->nextInHashList != NULL)
    {
        toUse->nextInHashList->prevInHashList = toUse;
    }

    /* Update FWTOCPSS DB */
    /* Not sure now but this indexing may be needed when there has to be
     * a decision to remove some entries from CPSS DB assuming the entire
     * DB gets full.
     */
    /* Flow Id corresponding to the flows is passed from the FW to
     * the CPSS in the message. This is an internal identifier. At the init
     * all the CPSS DB entries have been init with flow id in sequential
     * fashion. Then for every new flow received from the FW, this
     * fwToCpssFlowDb is updated with the corresponding CPSS Flow ID for
     * every flow id sent from the FW. Please note initially upto the size
     * of FW Db i.e. 8k the CPSS DB and FW DB Flow IDs will be a perfect match
     * i.e. 1-1 and then if a new rule comes to the FW, then initially FW
     * will send remove flow message corresponding to its oldest flow and
     * ipfix entries corresponding to this flow id will be updated in the CPSS DB
     * and then the newly added ipfix flow in thw FW DB would send message corresponding
     * to the previously used FW ID but in CPSS DB this would internally point to the
     * next free location i.e.
     *
     * Case I: Till FW DB is not full (1-1 mapping b/w FW and CPSS Flow Id)
     *
     *   Flow ID FW              Flow ID CPSS
     *       0                         0
     *       1                         1
     *      ...                       ...
     *      8K-1                      8K-1
     *
     * Case II: FW DB full and new rule came not in FW DB, FW replaces flow id 0 to
     *          make entry for this new rule. CPSS does not replace rule 0 but assigns
     *          next free index i.e.8K to this new flow
     *
     *   Flow ID FW              Flow ID CPSS
     *    Not in FW                    0
     *       1                         1
     *      ...                       ...
     *      8K-1                      8K-1
     *       0                         8K
     *
     * Case III: 10 entirely new flows come to FW and informed to CPSS then mapping becomes
     *
     *   Flow ID FW              Flow ID CPSS
     *    Not in FW                    0
     *       11                        1
     *      ...                       ...
     *      8K-1                      8K-1
     *      0-10                      8K+10
     */
    flowMngDbPtr->fwToCpssFlowDb[fwFlowId] = toUse->cpssFlowIndex;
    flowMngDbPtr->flowState[toUse->cpssFlowIndex] = GT_TRUE;

    /* Update CPSS DB other fields: ?? TBD */
    rc  = prvCpssDxChFlowManagerSetFlowDbKey(flowMngDbPtr, toUse->cpssFlowIndex, flowKeyPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("prvCpssDxChFlowManagerSetFlowDbKey returned not GT_OK\n");
        return rc;
    }

    return rc;
}

/**
* @internal prvCpssDxChFlowManagerAddEntryToDb function
* @endinternal
*
* @brief To add/update the ipfix entry for the given flow index in cpss db.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]  devNum                - device number.
* @param [in] flowMngDbPtr          - (pointer to) flow manager db
* @param [in] flowIndex             - flow index value
* @param [in] bufferPtr             - (pointer to) packet buffer
*
* @retval GT_OK                           - on success.
* @retval GT_BAD_PTR                   - on NULL inoput ptr.
*/
GT_STATUS prvCpssDxChFlowManagerAddEntryToDb
(
    IN GT_U8                  devNum,
    IN CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    IN GT_U32                 flowIndex,
    IN GT_U8                  *bufferPtr
)
{
    GT_U32  byteCount; /* store flow byte count */
    GT_U32  packetCount; /* store flow packet count */
    GT_U32  dropCount; /* store flow drop count */
    GT_U32  cpssFlowIndex; /* internal flow index */
    GT_U32  currentValue; /* current value of byte count/ packet count/ drop count as received in flow message */
    GT_U32  delta; /* difference between current and previous byte count/ packet count/ drop count */
    GT_U32  prevReadValue; /* previous value of byte count/ packet count/ drop count as maintained in flow DB */
    GT_U8   *entryPtr; /* message packet pointer */
    GT_U32  firstTsNanoValue; /* 8b nano second timer value in message packet first TS field */
    GT_U32  firstTsSecValue; /* 8b second timer value in message packet first TS field */
    GT_U32  lastTsNanoValue; /* 8b nano second timer value in message packet last TS field */
    GT_U32  lastTsSecValue; /* 8b second timer value in message packet last TS field */
    GT_U32  deltaSec; /* difference between last and first TS nano, second timer value */
    GT_UL64 startFlowTime; /* start time stamp of the flow */
    GT_UL64 lastFlowTime; /* last time stamp of the flow */
    GT_U32  ipfixSec; /* ipfix TOD second timer value LSB 32 bits */
    GT_U32  ipfixSecTemp; /* ipfix TOD second timer value LSB 8 bits */
    GT_U32  ipfixSecMSB; /* ipfix TOD second timer value MSB 24 bits */
    GT_U32  tsSecValueDb; /* second timer value 32 bits as maintained in flow DB */
    CPSS_DXCH_IPFIX_TIMER_STC  ipfixTimer; /* ipfix TOD second, nano second timer value */
    GT_STATUS rc; /* return status */

    cpssOsMemSet(&ipfixTimer, 0 ,sizeof(CPSS_DXCH_IPFIX_TIMER_STC));
    /* Update the flow entry in the CPSS DB corresponding to the
     * flow id received from the FW in the message. Retrieve the flow
     * index in the cpss db corresponding to the fw flow id.
     */
    entryPtr = bufferPtr;
    cpssFlowIndex = flowMngDbPtr->fwToCpssFlowDb[flowIndex];
    flowMngDbPtr->flowCounterDb[cpssFlowIndex].flowId = cpssFlowIndex;

    /* Update flow byte count */
    prevReadValue = (GT_U32)flowMngDbPtr->flowCounterDb[cpssFlowIndex].byteCount;
    cpssOsMemCpy(&byteCount, entryPtr, sizeof(GT_U32));
    byteCount = CPSS_32BIT_LE(byteCount);
    currentValue = byteCount;
    FLOW_WRAP_AROUND_CHECK(currentValue, prevReadValue, delta);
    flowMngDbPtr->flowCounterDb[cpssFlowIndex].byteCount += delta;

    /* Update flow packet count */
    entryPtr += sizeof(GT_U32);
    prevReadValue = (GT_U32)flowMngDbPtr->flowCounterDb[cpssFlowIndex].packetCount;
    cpssOsMemCpy(&packetCount, entryPtr, sizeof(GT_U32));
    packetCount = CPSS_32BIT_LE(packetCount);
    currentValue = packetCount;
    FLOW_WRAP_AROUND_CHECK(currentValue, prevReadValue, delta);
    flowMngDbPtr->flowCounterDb[cpssFlowIndex].packetCount += delta;

    /* Update flow drop count */
    entryPtr += sizeof(GT_U32);
    prevReadValue = (GT_U32)flowMngDbPtr->flowCounterDb[cpssFlowIndex].dropCount;
    cpssOsMemCpy(&dropCount, entryPtr, sizeof(GT_U32));
    dropCount = CPSS_32BIT_LE(dropCount);
    currentValue = dropCount;
    FLOW_WRAP_AROUND_CHECK(currentValue, prevReadValue, delta);
    flowMngDbPtr->flowCounterDb[cpssFlowIndex].dropCount += delta;

    /* The IPFIX timestamp received from FW is in form of timeStampStart(2B)
     * and timeStampEnd(2B). The 2B constitutes sec(1B) and nsec(1B). CPSS
     * handling should take care of the sec wraparound of the timeStampEnd
     * which will happen after every 256sec due to 1B width and should
     * store the timeStampStart and timeStampEnd in alignment with FW in its
     * DB. timeStampStart field will be populated only once either for the
     * first existing or removed ISF.
     */
    /* Update flow timestamp start */
    entryPtr += sizeof(GT_U32);

    firstTsNanoValue = entryPtr[0];
    firstTsSecValue = entryPtr[1];

    rc = cpssDxChIpfixTimerGet(devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, &ipfixTimer);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Time stamp handling for existing flow based on message packet
     * received for the first time. Get the first timestamp from the
     * message packet i.e. 8b second timer and extract LSB 8 bits from the
     * IPFIX TOD second timer and compare to derive the first TS second timer
     * value considering the wraparound. Compare the last TS second timer value
     * with the first TS second timer value based on message packet to
     * calculate delat considering wraparound and update the last TS second
     * timer value in flow db.
     */
    if(flowMngDbPtr->flowCounterDb[cpssFlowIndex].firstTimeStamp == 0)
    {
        ipfixSec = ipfixTimer.secondTimer.l[0];
        /* Extract MSB 24b from IPFIX second timer */
        ipfixSecMSB = ipfixSec >> 8;
        /* Extract LSB 8b from IPFIX second timer */
        ipfixSecTemp = ipfixSec & 0xFF;

        if (ipfixSecTemp < firstTsSecValue)
        {
            ipfixSecMSB--;
        }
        ipfixSec = (ipfixSecMSB << 8) | ipfixSecTemp;
        startFlowTime = ipfixSec;
        startFlowTime = (startFlowTime << 32) | (firstTsNanoValue << 22);
        flowMngDbPtr->flowCounterDb[cpssFlowIndex].firstTimeStamp = startFlowTime;

        /* Update flow timestamp end With Wraparound */
        lastTsNanoValue = entryPtr[2];
        lastTsSecValue = entryPtr[3];

        /* Existing flow ISF message has 2 timer fields denoting first and last time stamp.
         * If flow data request is triggered after last time stamp already
         * wrap-around (8b time stamp field with max value of 256sec) then delta calculation
         * needs to take care of that for once and then its applications job to
         * handle it further.
         */
        if (lastTsSecValue >= firstTsSecValue)
        {
            deltaSec = lastTsSecValue - firstTsSecValue;
        }
        else
        {
            deltaSec = (2 * BIT_7 + lastTsSecValue) - firstTsSecValue;
        }
        ipfixSec += deltaSec;
        lastFlowTime = ipfixSec;
        lastFlowTime = (lastFlowTime << 32) | (lastTsNanoValue << 22);
        flowMngDbPtr->flowCounterDb[cpssFlowIndex].lastTimeStamp = lastFlowTime;
    }
    else
    {
        /* Time stamp handling for existing flow based on message packet
         * received from 2nd time and onwards. Get the last timestamp from the
         * message packet i.e 8b second timer value and extract LSB 8 bits from
         * the last time stamp of the flow db and compare to calculate the delta considering
         * wraparound. Add the delta to the second timer stored in flow db to
         * derive the last time stamp based on last message packet received.
         */
        lastTsNanoValue = entryPtr[2];
        lastTsSecValue = entryPtr[3];
        tsSecValueDb = flowMngDbPtr->flowCounterDb[cpssFlowIndex].lastTimeStamp >> 32;
        ipfixSecTemp = tsSecValueDb & 0xFF;
        if (lastTsSecValue >= ipfixSecTemp)
        {
            deltaSec = lastTsSecValue - ipfixSecTemp;
        }
        else
        {
            deltaSec = (2 * BIT_7 + lastTsSecValue) - ipfixSecTemp;
        }
        tsSecValueDb += deltaSec;
        lastFlowTime = tsSecValueDb;
        lastFlowTime = (lastFlowTime << 32) | (lastTsNanoValue << 22);
        flowMngDbPtr->flowCounterDb[cpssFlowIndex].lastTimeStamp = lastFlowTime;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFlowManagerAddRemovedFlowEntryToDb function
* @endinternal
*
* @brief To add/update the ipfix entry for the given removed flow index in cpss db.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngDbPtr          - (pointer to) flow manager db
* @param [in] flowIndex             - flow index value
*
* @retval GT_OK                           - on success.
* @retval GT_BAD_PTR                   - on NULL inoput ptr.
*/
GT_STATUS prvCpssDxChFlowManagerAddRemovedFlowEntryToDb
(
    IN CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    IN GT_U32                 flowIndex
)
{
    GT_U32  cpssFlowIndex; /* internal flow index */

    cpssFlowIndex = flowMngDbPtr->fwToCpssFlowDb[flowIndex];

    /* Update removed flow byte count */
    flowMngDbPtr->removedFlowCounterDb[cpssFlowIndex].byteCount += flowMngDbPtr->flowCounterDb[cpssFlowIndex].byteCount;
    flowMngDbPtr->flowCounterDb[cpssFlowIndex].byteCount = 0;

    /* Update removed flow packet count */
    flowMngDbPtr->removedFlowCounterDb[cpssFlowIndex].packetCount += flowMngDbPtr->flowCounterDb[cpssFlowIndex].packetCount;
    flowMngDbPtr->flowCounterDb[cpssFlowIndex].packetCount = 0;

    /* Update removed flow drop count */
    flowMngDbPtr->removedFlowCounterDb[cpssFlowIndex].dropCount += flowMngDbPtr->flowCounterDb[cpssFlowIndex].dropCount;
    flowMngDbPtr->flowCounterDb[cpssFlowIndex].dropCount = 0;

    return GT_OK;
}

/**
* @internal prvCpssDxChFlowManagerParseMessagePacketAndAddRule function
* @endinternal
*
* @brief To parse message packet and based on opcode add rule to cpss db or update the db.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]   devNum         - device number.
* @param [in]  flowMngDbPtr   - (pointer to) flow manager db.
* @param [in]  bufferPtr      - (pointer to) packet buffer.
* @param [in]  opcode         - message opcode
* @param [in]  flowId         - flow id
* @param [in]  flowData       - (pointer to) flow data.
*
* @retval GT_OK                        - on success.
* @retval GT_BAD_PTR                   - on NULL inoput ptr.
*/
GT_STATUS prvCpssDxChFlowManagerParseMessagePacketAndAddRule
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_FLOW_MNG_PTR                  flowMngDbPtr,
    IN  GT_U8                                   *bufferPtr,
    IN  GT_U8                                   opcode,
    IN  GT_U32                                  flowId,
    IN  CPSS_DXCH_FLOW_MANAGER_FLOW_STC         *flowData
)
{
    GT_U16 flowHash;
    GT_U32 flowIndex;
    GT_U8  flowKey[PRV_CPSS_DXCH_FLOW_KEY_SIZE_CNS];
    GT_U8  flowKeySize;
    GT_U32 cpssFlowIndex;
    GT_U32 rc = GT_OK;

    if(flowId > MAX_NUM_OF_FW_FLOW_INDEX_CNS)
    {
        CPSS_LOG_INFORMATION_MAC("prvCpssDxChFlowManagerParseMessagePacketAndAddRule fw flow id not in allowed range\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(opcode == PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_NEW_FLOW_E)
    {
        rc = prvCpssDxChFlowManagerCalcFlowHashAndKey(flowMngDbPtr, flowData, &flowHash, &flowKey[0], &flowKeySize);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("prvCpssDxChFlowManagerCalcFlowHashAndKey returned not GT_OK\n");
            return rc;
        }
        /* look if flow already in CPSS FLOW DB */
        rc = prvCpssDxChFlowManagerGetFlowIndex(flowMngDbPtr, flowHash, &flowKey[0], &flowIndex);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("prvCpssDxChFlowManagerGetFlowIndex returned not GT_OK\n");
            return rc;
        }

        if(flowIndex == PRV_CPSS_FLOW_MANAGER_INVALID_FLOW_INDEX_CNS)
        {
            /* Flow not in CPSS FLOW DB */
            /* Add new flow for CPSS FLOW DB */
            rc = prvCpssDxChFlowManagerAddFlowToDb(flowMngDbPtr, flowHash, &flowKey[0], flowId);
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("prvCpssDxChFlowManagerAddFlowToDb for opcode"
                                         "PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_NEW_FLOW_E returned not GT_OK\n");
                return rc;
            }

            /* FW flow id should be internally converted to cpss flow id and
             * stored in cpss db and notified to application for every newly
             * learnt flows which application can use for its own query or flow
             * management purpose.
             */
            cpssFlowIndex = flowMngDbPtr->fwToCpssFlowDb[flowId];
            flowMngDbPtr->dbAddedFlowCount++;

            /* Flow Manager need to notify the application about the newly learnt flows as per DB maintained flows
             * and not as per newly learnt flows sent in message data which is subset of DB maintained flows.
             * Invoking the function callback to achieve that.
             */
            flowData->flowId = flowMngDbPtr->fwToCpssFlowDb[flowId];
            if(flowMngDbPtr->appCbFlowNotificaion && opcode == PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_NEW_FLOW_E)
            {
                flowMngDbPtr->appCbFlowNotificaion(devNum, flowMngDbPtr->flowMngId, flowData);
            }
        }
        else
        {
            /* Newly learnt flow from FW is already existing in the CPSS DB
             * so updating the CPSS DB with the FW flow id for the
             * corresponding cpss flow index. Also mark the flow index as
             * TRUE to signify the flow exists in the FW DB used while querying
             * the flow data from cpss to fw.
             */
            flowMngDbPtr->fwToCpssFlowDb[flowId] = flowIndex;
            flowMngDbPtr->flowState[flowIndex] = GT_TRUE;
        }
    }
    else if(opcode == PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_FLOW_DATA_E)
    {
        rc = prvCpssDxChFlowManagerAddEntryToDb(devNum, flowMngDbPtr, flowId, bufferPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("prvCpssDxChFlowManagerAddEntryToDb for opcode"
                                     "PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_FLOW_DATA_E returned not GT_OK\n");
            return rc;
        }
    }
    else if(opcode == PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_REMOVED_FLOW_E)
    {
        rc = prvCpssDxChFlowManagerAddEntryToDb(devNum, flowMngDbPtr, flowId, bufferPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("prvCpssDxChFlowManagerAddEntryToDb for opcode"
                                     "PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_FLOW_DATA_E returned not GT_OK\n");
            return rc;
        }

        /* FW notified counters are read-only and hence setting aside the flow counters for the removed flow
         * from regular live flows(same flow which was earlier removed from FW DB and came back live again)
         * to avoid ambiguity in terms of maintaining flow based statistics.
         */
        rc = prvCpssDxChFlowManagerAddRemovedFlowEntryToDb(flowMngDbPtr, flowId);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("prvCpssDxChFlowManagerAddRemovedFlowEntryToDb for opcode"
                                     " PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_REMOVED_FLOW_E  returned not GT_OK\n");
            return rc;
        }
        cpssFlowIndex = flowMngDbPtr->fwToCpssFlowDb[flowId];
        /* Mark the flow index as FALSE signifying the flow has been
         * removed from the FW DB to assist with the existing flow
         * information from cpps to fw.
         */
        flowMngDbPtr->flowState[cpssFlowIndex] = GT_FALSE;
    }
    else
    {
        /* Not supported case */
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChFlowManagerConfigGet function
* @endinternal
*
* @brief To get the configuration data sent from the cpss to FW for flow manager service
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId            - unique flow manager id
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_FOUND             - if the Flow Manager DB is not found.
*/
GT_STATUS prvCpssDxChFlowManagerConfigGet
(
    IN GT_U32    flowMngId
)
{
    PRV_CPSS_DXCH_FLOW_MANAGER_STC    *flowMngDbPtr;

    if(flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
    if (flowMngDbPtr == NULL)
    {
        /* can't find the Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    cpssOsPrintf("Rx SDMA queue %d Tx SDMA queue %d message cpu code %d "
                 "FW cpu code start %d TCAM logical start index %d "
                 "Reduced ISF key size %d "
                 "IPFIX start index %d TCAM num of short rules %d "
                 "TCAM short rule size %d FW flow Db size %d "
                 "FW flow short Db size %d\n",
                 flowMngDbPtr->cpssFwCfg.rxSdmaQ, flowMngDbPtr->cpssFwCfg.txSdmaQ,
                 flowMngDbPtr->cpssFwCfg.messagePacketCpuCode, flowMngDbPtr->cpssFwCfg.cpuCodeStart,
                 flowMngDbPtr->cpssFwCfg.tcamStartIndex, flowMngDbPtr->cpssFwCfg.reducedRuleSize,
                 flowMngDbPtr->cpssFwCfg.ipfixStartIndex,
                 flowMngDbPtr->cpssFwCfg.tcamNumOfShortRules, flowMngDbPtr->cpssFwCfg.tcamShortRuleSize,
                 flowMngDbPtr->cpssFwCfg.flowDBSize, flowMngDbPtr->cpssFwCfg.flowShortDBSize);

    return GT_OK;
}


/**
* @internal prvCpssDxChFlowManagerCpssDbDump function
* @endinternal
*
* @brief To dump the entire cpss flow db.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId            - unique flow manager id
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_FOUND             - if the Flow Manager DB is not found.
*/
GT_STATUS prvCpssDxChFlowManagerCpssDbDump
(
    IN GT_U32    flowMngId
)
{
    GT_U32                            iter;
    GT_U32                            keyIndex;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC    *flowMngDbPtr;

    if(flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
    if (flowMngDbPtr == NULL)
    {
        /* can't find the Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    for(iter = 0; iter <= flowMngDbPtr->flowDbSize; iter++)
    {
      cpssOsPrintf("Flow key at index %d\n", iter);
      for (keyIndex = 0; keyIndex < flowMngDbPtr->cpssFwCfg.reducedRuleSize; keyIndex++)
      {
          cpssOsPrintf("%d ",
              flowMngDbPtr->flowDataDb[iter].keyData[keyIndex]);
      }
      cpssOsPrintf("\nFlow entry for index %d : flow id %d  byte count %d packet count %d"
          "drop count %d time stamp %d\n",
          iter, flowMngDbPtr->flowCounterDb[iter].flowId,
          flowMngDbPtr->flowCounterDb[iter].byteCount,
          flowMngDbPtr->flowCounterDb[iter].packetCount,
          flowMngDbPtr->flowCounterDb[iter].dropCount,
          flowMngDbPtr->flowCounterDb[iter].lastTimeStamp);
    }

    return GT_OK;
}

/**
* @internal serviceCpuFlowManagerFwInit function
* @endinternal
*
* @brief Flow Manager Firmware loading and Activate

* @param[in] devNum                - device number
* @param[in] sCpuId                - service CPU ID
* @param[out] fwChannel            - IPC channel
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
*/
GT_STATUS serviceCpuFlowManagerFwInit
(
     IN  GT_U8      devNum,
     IN  GT_U32     sCpuId,
     OUT GT_UINTPTR *fwChannel
)
{
    GT_STATUS rc = GT_OK;

    rc = prvCpssFlowManagerToSrvCpuLoad(devNum, sCpuId, fwChannel);

    return rc;
}

/**
* @internal prvCpssDxChFlowManagerHashListDump function
* @endinternal
*
* @brief To dump the hash list maintained for faster db management.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId            - unique flow manager id
* @param [in] startHash            - start hash index
* @param [in] numOfLists           - number of hash lists to dump
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_FOUND             - if the Flow Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChFlowManagerHashListDump
(
    IN GT_U32    flowMngId,
    IN GT_U32    startHash,
    IN GT_U32    numOfLists
)
{
    GT_STATUS                         rc = GT_OK;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC    *flowMngDbPtr;
    PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC *tmpHash;
    GT_U32                            iter;

    if(flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        /* flowMngId out of range */
        cpssOsPrintf("Flow Manager Id out of acceptable limit\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((numOfLists + startHash) > PRV_NUM_OF_HASH_LISTS_CNS)
    {
        /* startHahs or numofLists out of range */
        cpssOsPrintf("Start hash index or number of lists out of acceptable limit\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);

    if (flowMngDbPtr == NULL)
    {
        /* can't find the Flow Manager DB */
        cpssOsPrintf("Flow Manager DB does not exist\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    for(iter = startHash ; iter < startHash+numOfLists ; iter++) {
        tmpHash = flowMngDbPtr->hashList[iter]->nextInHashList;
        if( tmpHash != NULL) {
            cpssOsPrintf("[H:%d] -> [%d] -> ", iter, tmpHash->cpssFlowIndex);
            tmpHash = tmpHash->nextInHashList;
            while(tmpHash) {
                cpssOsPrintf("[%d] -> ", tmpHash->cpssFlowIndex);
                tmpHash = tmpHash->nextInHashList;
            }
            cpssOsPrintf("NULL\n");
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChFlowManagerFreeListDump function
* @endinternal
*
* @brief To dump the free list based on which entries to new flows are allocated.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId              - unique flow manager id
* @param [in] startIndex             - to dump from this inex onwards
* @param [in] numEntries             - number of entries to print
*
* @retval GT_OK                          - on success.
*/
GT_STATUS prvCpssDxChFlowManagerFreeListDump
(
    IN GT_U32    flowMngId,
    IN GT_U32    startIndex,
    IN GT_U32    numEntries
)
{
    PRV_CPSS_DXCH_FLOW_MANAGER_STC    *flowMngDbPtr;
    PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC *tempPtr;
    GT_STATUS                         rc = GT_OK;
    GT_U32                            iter;

    if (flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        cpssOsPrintf("Flow Manager ID does not exists\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (numEntries == 0 || (startIndex > numEntries))
    {
        cpssOsPrintf("Invalid inputs for startIndex %d or numEntries %d\n", startIndex, numEntries);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
    if (flowMngDbPtr == NULL)
    {
        cpssOsPrintf("Flow Manager DB does not exist\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    tempPtr = flowMngDbPtr->cpssFlowFreeListHeader;
    if(tempPtr == NULL)
    {
        cpssOsPrintf("Free List is empty\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else
    {
        iter = 0;
        while(tempPtr && (iter < startIndex+numEntries))
        {
            if(startIndex <= iter)
            {
                cpssOsPrintf("[%d: CPSS Free Flow Index %d] -> ", iter, tempPtr->cpssFlowIndex);
            }
            tempPtr = tempPtr->next;
            iter++;
            if(tempPtr == NULL)
            {
                cpssOsPrintf("NULL entry \n");
            }
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChFlowManagerUsedListDump function
* @endinternal
*
* @brief To dump the used list maintained in order to track the flows activity.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId              - unique flow manager id
* @param [in] startIndex             - to dump from this inex onwards
* @param [in] numEntries             - number of entries to print
*
* @retval GT_OK                           - on success.
*/
GT_STATUS prvCpssDxChFlowManagerUsedListDump
(
    IN GT_U32    flowMngId,
    IN GT_U32    startIndex,
    IN GT_U32    numEntries
)
{
    PRV_CPSS_DXCH_FLOW_MANAGER_STC    *flowMngDbPtr;
    PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC *tempPtr;
    GT_STATUS                         rc = GT_OK;
    GT_U32                            iter;
    GT_U32                            keyIndex;

    if (flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        cpssOsPrintf("Flow Manager ID does not exists\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (numEntries == 0 || (startIndex > numEntries))
    {
        cpssOsPrintf("Invalid inputs for startIndex %d or numEntries %d\n", startIndex, numEntries);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
    if (flowMngDbPtr == NULL)
    {
        cpssOsPrintf("Flow Manager DB does not exist\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    tempPtr = flowMngDbPtr->cpssFlowUsedListHeader;
    if(tempPtr == NULL ||
       flowMngDbPtr->cpssFlowUsedListTail == NULL)
    {
        cpssOsPrintf("Used List is empty\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else
    {
        iter = 0;
        cpssOsPrintf("Index | State | Flow Key \n");
        cpssOsPrintf("------------------------------------------------------------------------------------ \n");
        while(tempPtr && (iter < startIndex+numEntries))
        {
            if(startIndex <= iter)
            {
                cpssOsPrintf("%5d | ", iter);
                cpssOsPrintf(" %s    | ", (flowMngDbPtr->flowState[iter] == GT_TRUE) ? "A" : "P");
                    for (keyIndex = 0; keyIndex < flowMngDbPtr->cpssFwCfg.reducedRuleSize; keyIndex++)
                    {
                        cpssOsPrintf("%3d ",
                            flowMngDbPtr->flowDataDb[iter].keyData[keyIndex]);
                    }
                    cpssOsPrintf("\n");
            }
            tempPtr = tempPtr->next;
            iter++;
            if(tempPtr == NULL)
            {
                cpssOsPrintf("NULL entry \n");
            }
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChFlowManagerFlowEntryDump function
* @endinternal
*
* @brief To dump the flow entries maintained in order to track the flows activity.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId              - unique flow manager id
* @param [in] startIndex             - to dump from this inex onwards
* @param [in] numEntries             - number of entries to print
*
* @retval GT_OK                           - on success.
*/
GT_STATUS prvCpssDxChFlowManagerFlowEntryDump
(
    IN GT_U32    flowMngId,
    IN GT_U32    startIndex,
    IN GT_U32    numEntries
)
{
    PRV_CPSS_DXCH_FLOW_MANAGER_STC    *flowMngDbPtr;
    GT_STATUS                         rc = GT_OK;
    GT_U32                            iter;

    if (flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        cpssOsPrintf("Flow Manager ID does not exists\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (numEntries == 0 || (startIndex > numEntries))
    {
        cpssOsPrintf("Invalid inputs for startIndex %d or numEntries %d\n", startIndex, numEntries);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
    if (flowMngDbPtr == NULL)
    {
        cpssOsPrintf("Flow Manager DB does not exist\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsPrintf("Index |    Byte Count   |  Packet Count   |   Drop Count    |      Time Stamp Start    |      Time Stamp End       \n");
    cpssOsPrintf("------|-----------------|-----------------|-----------------|--------------------------|-------------------------- \n");
    for(iter = startIndex; iter < (startIndex+numEntries); iter++) {
        cpssOsPrintf("%5d | %15llu | %15llu | %15llu | %8llusec %8llunsec | %8llusec %8llunsec \n",
                 iter,
                 flowMngDbPtr->flowCounterDb[iter].byteCount, flowMngDbPtr->flowCounterDb[iter].packetCount,
                 flowMngDbPtr->flowCounterDb[iter].dropCount,
                 flowMngDbPtr->flowCounterDb[iter].firstTimeStamp >> 32,
                 flowMngDbPtr->flowCounterDb[iter].firstTimeStamp & 0xFFFFFFFF,
                 flowMngDbPtr->flowCounterDb[iter].lastTimeStamp >> 32,
                 flowMngDbPtr->flowCounterDb[iter].lastTimeStamp & 0xFFFFFFFF);
    }
    return rc;
}


/**
* @internal prvCpssDxChFlowManagerFwToCpssIndexDump function
* @endinternal
*
* @brief To dump the fw and cpss flow index mapping.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId              - unique flow manager id
* @param [in] startIndex             - to dump from this index onwards
* @param [in] numEntries             - number of entries to print
*
* @retval GT_OK                          - on success.
*/
GT_STATUS prvCpssDxChFlowManagerFwToCpssIndexDump
(
    IN GT_U32    flowMngId,
    IN GT_U32    startIndex,
    IN GT_U32    numEntries
)
{
    PRV_CPSS_DXCH_FLOW_MANAGER_STC    *flowMngDbPtr;
    GT_STATUS                         rc = GT_OK;
    GT_U32                            iter;

    if (flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        cpssOsPrintf("Flow Manager ID does not exists\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (numEntries == 0 || (startIndex > numEntries))
    {
        cpssOsPrintf("Invalid inputs for startIndex %d or numEntries %d\n", startIndex, numEntries);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
    if (flowMngDbPtr == NULL)
    {
        cpssOsPrintf("Flow Manager DB does not exist\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsPrintf("FW Index | CPSS Index \n");
    cpssOsPrintf("--------------------- \n");
    for(iter = startIndex; iter < (startIndex+numEntries); iter++)
    {
        cpssOsPrintf(" %7d | %10d \n", iter, flowMngDbPtr->fwToCpssFlowDb[iter]);
    }

    return rc;
}

/**
* @internal prvCpssDxChFlowManagerMessageStatisticsClear function
* @endinternal
*
* @brief Debug function to clear message packet statistics.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId              - unique flow manager id
*
* @retval GT_OK                      - on success.
*/
GT_STATUS prvCpssDxChFlowManagerMessageStatisticsClear
(
    IN GT_U32    flowMngId
)
{
    PRV_CPSS_DXCH_FLOW_MANAGER_STC    *flowMngDbPtr;

    if (flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
    if (flowMngDbPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr->messageNewFlowCount = 0;
    flowMngDbPtr->messageRemovedFlowCount = 0;
    flowMngDbPtr->messageFlowDataGetCount = 0;
    flowMngDbPtr->reSyncCount = 0;
    flowMngDbPtr->seqNoCpss = 0;
    flowMngDbPtr->messageNewFlowCountAfterResync = 0;
    flowMngDbPtr->messageRemovedFlowCountAfterResync = 0;
    flowMngDbPtr->messageSequenceNumberForResync = 0;
    flowMngDbPtr->dbAddedFlowCount = 0;
    flowMngDbPtr->dbRemovedFlowCount = 0;

    return GT_OK;
}
