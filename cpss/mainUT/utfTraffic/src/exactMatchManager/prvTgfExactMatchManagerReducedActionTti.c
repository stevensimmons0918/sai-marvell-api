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
* @file prvTgfExactMatchReducedActionTti.c
*
* @brief Test Exact Match functionality with TTI Action Type and Reduced entry
*        configuration
*
* @version 1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfBridgeGen.h>
#include <exactMatchManager/prvTgfExactMatchManagerReducedActionTti.h>

#include <common/tgfExactMatchManagerGen.h>

/* use Exact Match size that match the fineTunning */
#define TGF_EXACT_MATCH_SIZE_FULL (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.emNum)
#define TGF_EXACT_MATCH_MAX_BANKS (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchNumOfBanks)

#define TGF_EXACT_MATCH_PROFILE_ID_CNS              9

/* we force application to give 'num of entries' in steps of 256 */
#define TGF_EXACT_MATCH_NUM_ENTRIES_STEPS_CNS       256

/* cpu code to set */
#define PRV_TGF_EXACT_MATCH_CPU_CODE_CNS    CPSS_NET_FIRST_USER_DEFINED_E + 3

/* create exact match manager - minimal parameters for tests */
static GT_STATUS createExactMatchManager
(
    IN GT_U32           exactMatchManagerId,
    IN GT_BOOL          isReduced
)
{
    GT_STATUS   st=GT_OK;
    PRV_TGF_EXACT_MATCH_MANAGER_CAPACITY_STC           capacityInfo;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC   entryAttrInfo;
    PRV_TGF_EXACT_MATCH_MANAGER_LOOKUP_STC             lookupInfo;
    PRV_TGF_EXACT_MATCH_MANAGER_AGING_STC              agingInfo;

    /* reset params */
    cpssOsMemSet(&capacityInfo,0,sizeof(PRV_TGF_EXACT_MATCH_MANAGER_CAPACITY_STC));
    cpssOsMemSet(&entryAttrInfo,0,sizeof(PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC));
    cpssOsMemSet(&lookupInfo,0,sizeof(PRV_TGF_EXACT_MATCH_MANAGER_LOOKUP_STC));
    cpssOsMemSet(&agingInfo,0,sizeof(PRV_TGF_EXACT_MATCH_MANAGER_AGING_STC));

    /* capacity info configuration */
    capacityInfo.hwCapacity.numOfHwIndexes = TGF_EXACT_MATCH_SIZE_FULL;
    capacityInfo.hwCapacity.numOfHashes    = TGF_EXACT_MATCH_MAX_BANKS;
    capacityInfo.maxTotalEntries           = TGF_EXACT_MATCH_SIZE_FULL;
    capacityInfo.maxEntriesPerAgingScan    = TGF_EXACT_MATCH_NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerDeleteScan   = TGF_EXACT_MATCH_NUM_ENTRIES_STEPS_CNS; /*256*/
    
    /* lookup configuration */
    lookupInfo.lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupEnable=GT_TRUE;
    lookupInfo.lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClient=PRV_TGF_EXACT_MATCH_CLIENT_TTI_E;
    lookupInfo.lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsNum=1;
    lookupInfo.lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.keyType=PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E;
    lookupInfo.lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.enableExactMatchLookup=GT_TRUE;
    lookupInfo.lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.profileId=TGF_EXACT_MATCH_PROFILE_ID_CNS;
    lookupInfo.profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
    lookupInfo.profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.keyStart = 0;
    cpssOsMemSet((GT_VOID*) &(lookupInfo.profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.mask[0]), 0, sizeof(GT_U8)*PRV_TGF_EXACT_MATCH_MAX_KEY_SIZE_CNS);

    /* entry configuration; take all fields from Expanded Action */
    entryAttrInfo.expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].exactMatchExpandedEntryValid = GT_TRUE;
    entryAttrInfo.expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedActionType = PRV_TGF_EXACT_MATCH_ACTION_TTI_E;
    if (isReduced==GT_TRUE)
    {
        entryAttrInfo.expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.ttiAction.command = CPSS_PACKET_CMD_FORWARD_E; 
        entryAttrInfo.expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedActionOrigin.ttiExpandedActionOrigin.overwriteExpandedActionCommand=GT_TRUE;
        entryAttrInfo.expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.ttiAction.userDefinedCpuCode = PRV_TGF_EXACT_MATCH_CPU_CODE_CNS;
        entryAttrInfo.expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedActionOrigin.ttiExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode = GT_TRUE;
    }
    else
    {
        entryAttrInfo.expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.ttiAction.command = PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E; 
        entryAttrInfo.expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedActionOrigin.ttiExpandedActionOrigin.overwriteExpandedActionCommand=GT_FALSE;
        entryAttrInfo.expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.ttiAction.userDefinedCpuCode = PRV_TGF_EXACT_MATCH_CPU_CODE_CNS;
        entryAttrInfo.expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedActionOrigin.ttiExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode = GT_FALSE;
    }
    
    st = prvTgfExactMatchManagerCreate(exactMatchManagerId,&capacityInfo,&lookupInfo,&entryAttrInfo,&agingInfo);

    return st;
}

/**
* @internal prvTgfExactMatchManagerReducedActionTtiManagerCreate function
* @endinternal
*
* @brief   Create Exact Match Manager with device
* @param[in] exactMatchManagerId - manager Id 
* @param[in] isReduced - GT_FALSE:  take CPU code and command
*                                   from Expander configuration
*                        GT_TRUE:   take CPU code and command
*                                   from Reduced entry configuration
*/
GT_VOID prvTgfExactMatchManagerReducedActionTtiManagerCreate
(
    IN GT_U32                               exactMatchManagerId,
    IN GT_BOOL                              isReduced
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC     pairListArr[1];
    GT_U32                                                  numOfPairs=1;

    pairListArr[0].devNum=prvTgfDevNum;
    pairListArr[0].portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* create single valid manager */
    st = createExactMatchManager(exactMatchManagerId,isReduced);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "createExactMatchManager: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* add device to it */
    st = prvTgfExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",exactMatchManagerId);
}

/**
* @internal prvTgfExactMatchReducedActionTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test configuration related to Exact Match 
*          Expanded Action
* @param[in] exactMatchManagerId - manager Id 
* @param[in] isReduced - GT_FALSE:  take CPU code and command
*                                   from Expander configuration
*                        GT_TRUE:   take CPU code and command
*                                   from Reduced entry configuration  
*
*/
GT_VOID prvTgfExactMatchManagerReducedActionTtiConfigSet
(
    IN GT_U32                               exactMatchManagerId,
    IN GT_BOOL                              isReduced
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_STC                   entry;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC        params;

    /* reset params */
    cpssOsMemSet(&entry,0,sizeof(PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_STC));
    cpssOsMemSet(&params,0,sizeof(PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC));

    entry.exactMatchEntry.key.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_33B_E;

    /* here we check exactly the same pattern set for the TTI match in TCAM - all zero */
    cpssOsMemSet((GT_VOID*) &entry.exactMatchEntry.key.pattern[0], 0, sizeof(entry.exactMatchEntry.key.pattern));
    /* same as configured in Expanded Entry */
    entry.exactMatchEntry.lookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    entry.expandedActionIndex = TGF_EXACT_MATCH_PROFILE_ID_CNS;
    entry.exactMatchActionType = PRV_TGF_EXACT_MATCH_ACTION_TTI_E;
    entry.exactMatchAction.ttiAction.command = PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E;
    if(isReduced==GT_TRUE)
    {
        entry.exactMatchAction.ttiAction.userDefinedCpuCode = PRV_TGF_EXACT_MATCH_CPU_CODE_CNS+ 2 ; /*505*/
    }
    else
    {
        entry.exactMatchAction.ttiAction.userDefinedCpuCode = PRV_TGF_EXACT_MATCH_CPU_CODE_CNS;/* 503 */
    }
    
    params.rehashEnable = GT_FALSE;

    /* add entry to manager */
    st = prvTgfExactMatchManagerEntryAdd(exactMatchManagerId,&entry,&params);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerEntryAdd: expected to GT_OK on manager [%d]",exactMatchManagerId);
}

/**
* @internal prvTgfExactMatchManagerReducedActionTtiManagerDelete function
* @endinternal
*
* @brief   Delete Exact Match Manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerReducedActionTtiManagerDelete
(
    IN GT_U32                               exactMatchManagerId
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC     pairListArr[1];
    GT_U32                                                  numOfPairs=1;

    pairListArr[0].devNum=prvTgfDevNum;
    pairListArr[0].portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* remove device from manager */
    st = prvTgfExactMatchManagerDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerDevListRemove: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* delete manager */
    st = prvTgfExactMatchManagerDelete(exactMatchManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerDelete: expected to GT_OK on manager [%d]",exactMatchManagerId);
}

/**
* @internal prvTgfExactMatcManagerhReducedActionTtiDeleteEmEntry function
* @endinternal
*
* @brief   Delete Exact Match Entry from manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatcManagerhReducedActionTtiDeleteEmEntry
(
    IN GT_U32                               exactMatchManagerId
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_STC                   entry;

    /* reset params */
    cpssOsMemSet(&entry,0,sizeof(PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_STC));

    entry.exactMatchEntry.key.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E;
    /* here we check exactly the same pattern set for the TTI match in TCAM - all zero */
    cpssOsMemSet((GT_VOID*) &entry.exactMatchEntry.key.pattern[0], 0, sizeof(entry.exactMatchEntry.key.pattern));
    /* same as configured in Expanded Entry */
    entry.exactMatchEntry.lookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    entry.expandedActionIndex = TGF_EXACT_MATCH_PROFILE_ID_CNS;
    entry.exactMatchActionType = PRV_TGF_EXACT_MATCH_ACTION_TTI_E;
    entry.exactMatchAction.ttiAction.command = PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E;
    entry.exactMatchAction.ttiAction.userDefinedCpuCode = PRV_TGF_EXACT_MATCH_CPU_CODE_CNS;

    /* add entry to manager */
    st = prvTgfExactMatchManagerEntryDelete(exactMatchManagerId,&entry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerEntryDelete: expected to GT_OK on manager [%d]",exactMatchManagerId);
}
