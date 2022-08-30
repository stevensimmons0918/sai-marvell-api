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
* @file prvTgfExactMatchManagerHaFullCapacity.c
*
* @brief Test Exact Match Manager HA functionality with full capacity
*        exact match entries
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
#include <common/tgfIpGen.h>
#include <exactMatch/prvTgfExactMatchUdbPcl.h>
#include <common/tgfExactMatchManagerGen.h>
#include <exactMatch/prvTgfExactMatchTtiPclFullPath.h>
#include <exactMatchManager/prvTgfExactMatchManagerHaFullCapacity.h>

/* we force application to give 'num of entries' in steps of 256 */
#define NUM_ENTRIES_STEPS_CNS                       0/* TBD when aging support will be added 256*/
#define PRV_TGF_MAX_TOTAL_ENTRIES                   (PRV_TGF_TOTAL_HW_CAPACITY - NUM_ENTRIES_STEPS_CNS)

/* parameter used to check counters validity */
static PRV_TGF_EXACT_MATCH_MANAGER_COUNTERS_STC                counters;
static GT_U32                                                  maxTotalEntriesToDelete;

/* used for reconstrocting back the value of global systemRecoveryInfo;*/
static CPSS_SYSTEM_RECOVERY_INFO_STC    oldSystemRecoveryInfo;

static PRV_TGF_PCL_RULE_FORMAT_UNT                        mask;
static PRV_TGF_PCL_RULE_FORMAT_UNT                        pattern;
static PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_STC              entry;

extern  CPSS_PACKET_CMD_ENT pktCmdUsedForExactMatchEntry;

#define TGF_EXACT_MATCH_PROFILE_ID_CNS  4

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS                   5

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS            0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR1_PORT_IDX_CNS       1
#define PRV_TGF_EGR2_PORT_IDX_CNS       2
#define PRV_TGF_EGR3_PORT_IDX_CNS       3

/* PCL ID */
#define PRV_TGF_PCL_ID_CNS                   0x66

/* cpu code to set */
#define PRV_TGF_EXACT_MATCH_CPU_CODE_CNS    CPSS_NET_FIRST_USER_DEFINED_E + 1

/* use Exact Match size that match the fineTunning */
#define TGF_EXACT_MATCH_SIZE_FULL (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.emNum)
#define TGF_EXACT_MATCH_MAX_BANKS (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchNumOfBanks)

/* we force application to give 'num of entries' in steps of 256 */
#define TGF_EXACT_MATCH_NUM_ENTRIES_STEPS_CNS       256

extern GT_BOOL reducedUsedForExactMatchEntry;

/* prepare the parameters to be used when creating a manager */
static GT_STATUS prepareManagerConfiguration
(
    PRV_TGF_EXACT_MATCH_MANAGER_CAPACITY_STC           *capacityInfoPtr,
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC   *entryAttrInfoPtr,
    PRV_TGF_EXACT_MATCH_MANAGER_LOOKUP_STC             *lookupInfoPtr,
    PRV_TGF_EXACT_MATCH_MANAGER_AGING_STC              *agingInfoPtr,
    CPSS_PACKET_CMD_ENT                                 pktCmd
)
{
      /* capacity info configuration */
    capacityInfoPtr->hwCapacity.numOfHwIndexes = TGF_EXACT_MATCH_SIZE_FULL;
    capacityInfoPtr->hwCapacity.numOfHashes    = TGF_EXACT_MATCH_MAX_BANKS;
    capacityInfoPtr->maxTotalEntries           = TGF_EXACT_MATCH_SIZE_FULL;
    capacityInfoPtr->maxEntriesPerAgingScan    = TGF_EXACT_MATCH_NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfoPtr->maxEntriesPerDeleteScan   = TGF_EXACT_MATCH_NUM_ENTRIES_STEPS_CNS; /*256*/

    /* lookup configuration */
    lookupInfoPtr->lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupEnable=GT_TRUE;
    lookupInfoPtr->lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClient=PRV_TGF_EXACT_MATCH_CLIENT_IPCL_0_E;
    lookupInfoPtr->lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsNum=1;
    lookupInfoPtr->lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].pclMappingElem.packetType=PRV_TGF_PCL_PACKET_TYPE_UDE6_E;
    lookupInfoPtr->lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].pclMappingElem.subProfileId = 0;
    lookupInfoPtr->lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].pclMappingElem.enableExactMatchLookup=GT_TRUE;
    lookupInfoPtr->lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].pclMappingElem.profileId=TGF_EXACT_MATCH_PROFILE_ID_CNS;
    lookupInfoPtr->profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.keySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;
    lookupInfoPtr->profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.keyStart = 0;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    cpssOsMemSet(pattern.ruleIngrUdbOnly.udb, 0, sizeof(pattern.ruleIngrUdbOnly.udb));
    cpssOsMemSet(mask.ruleIngrUdbOnly.udb, 0xFF, sizeof(mask.ruleIngrUdbOnly.udb));/* full mask for first stage */

     /* pattern */
    pattern.ruleIngrUdbOnly.udb[0]=PRV_TGF_PCL_ID_CNS;  /* {PCL-ID[7:0]}                             */
    pattern.ruleIngrUdbOnly.udb[1]=0x80;                /* {UDB Valid,reserved,PCL-ID[9:8]}          */
    pattern.ruleIngrUdbOnly.udb[2]=(PRV_TGF_VLANID_CNS&0xFF);     /* eVLAN LSB [7:0] */
    pattern.ruleIngrUdbOnly.udb[3]=((PRV_TGF_VLANID_CNS>>8)&0x1F);/* eVLAN MSB [12:8]*/
    pattern.ruleIngrUdbOnly.udb[4]=(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]&0xFF);/* Source-ePort[7:0] */
    pattern.ruleIngrUdbOnly.udb[5]=((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]>>8)&0x1F);/* Source-ePort[12:8] */

    /*{ 0x00, 0x00, 0x44, 0x33, 0x22, 0x11 } - daMac */
    pattern.ruleIngrUdbOnly.udb[6]=0;       /* Dst Mac MSB      */
    pattern.ruleIngrUdbOnly.udb[7]=0;       /* Dst Mac byte 1   */
    pattern.ruleIngrUdbOnly.udb[8]=0x44;    /* Dst Mac byte 2   */
    pattern.ruleIngrUdbOnly.udb[9]=0x33;    /* Dst Mac byte 3   */
    pattern.ruleIngrUdbOnly.udb[10]=0x22;   /* Dst Mac byte 4   */
    pattern.ruleIngrUdbOnly.udb[11]=0x11;   /* Dst Mac LSB      */

    mask.ruleIngrUdbOnly.udb[3] = 0x1F;
    mask.ruleIngrUdbOnly.udb[5] = 0x1F;

    cpssOsMemCpy((GT_VOID*) &(lookupInfoPtr->profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.mask[0]), mask.ruleIngrUdbOnly.udb, sizeof(GT_U8)*PRV_TGF_EXACT_MATCH_MAX_KEY_SIZE_CNS);

    switch (pktCmd)
    {
    case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
        /* entry configuration; take all fields from Expanded Action */
        entryAttrInfoPtr->expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.pclAction.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        /* For SIP6 and above for the egress control pipe the CPU code is also 'drop code' so relevant for drops too. */
        entryAttrInfoPtr->expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.pclAction.mirror.cpuCode = PRV_TGF_EXACT_MATCH_CPU_CODE_CNS;
        pktCmdUsedForExactMatchEntry = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        break;

    case CPSS_PACKET_CMD_FORWARD_E:
        /* entry configuration; take all fields from Expanded Action */
        entryAttrInfoPtr->expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
        /* For SIP6 and above for the egress control pipe the CPU code is also 'drop code' so relevant for drops too. */
        entryAttrInfoPtr->expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.pclAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E+5;
        entryAttrInfoPtr->expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.pclAction.bypassBridge = GT_TRUE;
        entryAttrInfoPtr->expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.pclAction.bypassIngressPipe = GT_TRUE;
        entryAttrInfoPtr->expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.pclAction.redirect.redirectCmd=PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
        entryAttrInfoPtr->expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_PORT_E;
        entryAttrInfoPtr->expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum=prvTgfDevNum;
        entryAttrInfoPtr->expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS];
        pktCmdUsedForExactMatchEntry = CPSS_PACKET_CMD_FORWARD_E;
        break;

    default:
        return GT_NOT_SUPPORTED;
    }

    entryAttrInfoPtr->expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].exactMatchExpandedEntryValid = GT_TRUE;
    entryAttrInfoPtr->expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedActionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;
    entryAttrInfoPtr->expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.pclAction.egressPolicy = GT_FALSE;

    agingInfoPtr->agingRefreshEnable = GT_TRUE;

    return GT_OK;
}

/* create exact match manager - minimal parameters for tests
   overide port from reduced action set keySize=19B */
static GT_STATUS createExactMatchReducedUdb19Manager
(
    IN GT_U32                                          exactMatchManagerId,
    CPSS_PACKET_CMD_ENT                                pktCmd
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

    st = prepareManagerConfiguration(&capacityInfo,&entryAttrInfo,&lookupInfo,&agingInfo,pktCmd);
    if (st!=GT_OK)
    {
        return st;
    }

    /* the outInterface should be taken from reduced entry */
    entryAttrInfo.expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedActionOrigin.pclExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;

    st = prvTgfExactMatchManagerCreate(exactMatchManagerId,&capacityInfo,&lookupInfo,&entryAttrInfo,&agingInfo);

    return st;
}
/*
* @internal prvTgfExactMatchManagerHaFullCapacityReducedUdb19ManagerCreate function
* @endinternal
*
* @brief   Create Exact Match Manager with device and set reduce
*          entry to be used, with UDB 19B key size
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityReducedUdb19ManagerCreate
(
    IN GT_U32                               exactMatchManagerId,
    CPSS_PACKET_CMD_ENT                     pktCmd
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC     pairListArr[1];
    GT_U32                                                  numOfPairs=1;

    pairListArr[0].devNum=prvTgfDevNum;
    pairListArr[0].portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* create single valid manager */
    st = createExactMatchReducedUdb19Manager(exactMatchManagerId,pktCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "createExactMatchReducedUdb19Manager: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* add device to it */
    st = prvTgfExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",exactMatchManagerId);
}

static GT_VOID prepareTtiPclFullPathConfigSet
(
    CPSS_PACKET_CMD_ENT                                 pktCmd,
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC    *paramsPtr
)
{
    /* reset params */
    cpssOsMemSet(&entry,0,sizeof(PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_STC));

    entry.exactMatchEntry.key.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;
    /* here we check exactly the same pattern set for the PCL match in TCAM */
    cpssOsMemCpy((GT_VOID*) &entry.exactMatchEntry.key.pattern[0], &pattern.ruleIngrUdbOnly.udb, sizeof(entry.exactMatchEntry.key.pattern));
    /* same as configured in Expanded Entry */
    entry.exactMatchEntry.lookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    entry.expandedActionIndex = TGF_EXACT_MATCH_PROFILE_ID_CNS;
    entry.exactMatchActionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;
    entry.exactMatchAction.pclAction.egressPolicy = GT_FALSE;

    switch (pktCmd)
    {
    case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
        reducedUsedForExactMatchEntry=GT_FALSE;
        entry.exactMatchAction.pclAction.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        /* For SIP6 and above for the egress control pipe the CPU code is also 'drop code' so relevant for drops too. */
        entry.exactMatchAction.pclAction.mirror.cpuCode = PRV_TGF_EXACT_MATCH_CPU_CODE_CNS;
        break;

    case CPSS_PACKET_CMD_FORWARD_E:
        reducedUsedForExactMatchEntry=GT_FALSE;
        entry.exactMatchAction.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
        /* For SIP6 and above for the egress control pipe the CPU code is also 'drop code' so relevant for drops too. */
        entry.exactMatchAction.pclAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E+5;
        entry.exactMatchAction.pclAction.bypassBridge = GT_TRUE;
        entry.exactMatchAction.pclAction.bypassIngressPipe = GT_TRUE;
        entry.exactMatchAction.pclAction.redirect.redirectCmd=PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
        entry.exactMatchAction.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_PORT_E;
        entry.exactMatchAction.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum=prvTgfDevNum;
        entry.exactMatchAction.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS];
        break;
    default:
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_NOT_SUPPORTED, "prvTgfExactMatchManagerTtiPclFullPathConfigSet FAILED pktCmd not supported in the test : %d", pktCmd);
        break;

    }
    paramsPtr->rehashEnable = GT_FALSE;
    return;
}
/**
* @internal prvTgfExactMatchManagerHaFullCapacityReducedUdb19ConfigSet function
* @endinternal
*
* @brief   Set PCL/TTI test configuration related to Exact Match Expanded Action
*          and Reduced Action with keySize=19B
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityReducedUdb19ConfigSet
(
    IN GT_U32                               exactMatchManagerId,
    CPSS_PACKET_CMD_ENT                     pktCmd
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC        params;

    prepareTtiPclFullPathConfigSet(pktCmd,&params);

    switch (pktCmd)
    {
    case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
       /* nothig to do
          all was set during the call to prepareTtiPclFullPathConfigSet */
        break;

    case CPSS_PACKET_CMD_FORWARD_E:
       /* AUTODOC: set Exact Match Entry - Reduced entry set forward port to be 2 */
        reducedUsedForExactMatchEntry=GT_TRUE;
        entry.exactMatchAction.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];
        break;

    default:
        st=GT_NOT_SUPPORTED;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerTtiPclFullPathConfigSet FAILED pktCmd not supported in the test : %d", pktCmd);
        break;
    }

    /* add entry to manager */
    entry.exactMatchEntry.key.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;
    st = prvTgfExactMatchManagerEntryAdd(exactMatchManagerId,&entry,&params);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerEntryAdd: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* Verify Counter - After entry addition */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    counters.ipcl0ClientKeySizeEntriesArray[PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E] += 1;
    counters.usedEntriesIndexes     += 2; /* 19B hold 2 index */
    counters.freeEntriesIndexes     = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntriesIndexes;

    prvTgfExactMatchManagerCounterVerify(exactMatchManagerId, &counters, GT_FALSE);
}

/**
* @internal prvTgfExactMatchManagerHaFullCapacityEntriesAdd function
* @endinternal
*
* @brief   Add entries to the exact match table with rehashEnable=GT_TRUE
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
* @param[in] firstCall           - GT_TRUE: calling this API for the first time
*                                  GT_FALSE: calling this API for the second time
*
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityEntriesAdd
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd,
    IN GT_BOOL                              firstCall
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC        params;
    GT_U32                                                  ii;
    GT_U32                                                  rand32Bits; /* random value in 32 bits (random 30 bits +
                                                                           2 bits from iterationNum) */
    GT_U32                                                  maxTotalEntries;
    GT_U32                                                  numEntriesAdded=0;  /* total number of entries added to the DB */
    GT_U32                                                  numIndexesAdded;    /* number of entries added to each bank;
                                                                                if one entry is added with keysize=19 then:
                                                                                numEntriesAdded=1 and numIndexesAdded=2 */
    numEntriesAdded = 0;
    numIndexesAdded = 0;
    if (firstCall==GT_TRUE)
    {
         maxTotalEntries = EXACT_MATCH_SIZE_FULL;
    }
    else
    {
        /* in replay case we need to add only the exactly amound added and deleted*/
         maxTotalEntries = maxTotalEntriesToDelete;
    }


    prepareTtiPclFullPathConfigSet(pktCmd,&params);

    switch (pktCmd)
    {
    case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
       /* nothig to do
          all was set during the call to prepareTtiPclFullPathConfigSet */
        break;

    case CPSS_PACKET_CMD_FORWARD_E:
       /* AUTODOC: set Exact Match Entry - Reduced entry set forward port to be 1
          we already set an entry to be forwarded to port 2  -
          so we want all the rest of the entries to be forwarded to port 1*/
        reducedUsedForExactMatchEntry=GT_TRUE;
        entry.exactMatchAction.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS];
        break;

    default:
        st=GT_NOT_SUPPORTED;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerTtiPclFullPathConfigSet FAILED pktCmd not supported in the test : %d", pktCmd);
        break;
    }

    /* add entries to manager */
    entry.exactMatchEntry.key.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;
    params.rehashEnable = GT_TRUE;
    /* set specific seed for random generator */
    cpssOsSrand(exactMatchManagerId); /* use exactMatchManagerId as seed value */
    for(ii = 0 ; ii < maxTotalEntries; /*update according to entry size*/)
    {
        rand32Bits = (cpssOsRand() & 0x7FFF) << 15 |
                     (cpssOsRand() & 0x7FFF)       |
                     ii << 30;
       
        entry.exactMatchUserDefined = rand32Bits & 0xFFFF;/* 16 bits */

        entry.exactMatchEntry.key.pattern[0]=(GT_U8)(rand32Bits >> 24);
        entry.exactMatchEntry.key.pattern[1]=(GT_U8)(rand32Bits >> 16);
        entry.exactMatchEntry.key.pattern[2]=(GT_U8)(rand32Bits >>  8);
        entry.exactMatchEntry.key.pattern[3]=(GT_U8)(rand32Bits >>  0);

        entry.exactMatchEntry.key.pattern[15]=(GT_U8)(rand32Bits >> 24);
        entry.exactMatchEntry.key.pattern[16]=(GT_U8)(rand32Bits >> 16);
        entry.exactMatchEntry.key.pattern[17]=(GT_U8)(rand32Bits >>  8);
        entry.exactMatchEntry.key.pattern[18]=(GT_U8)(rand32Bits >>  0);

        st = prvTgfExactMatchManagerEntryAdd(exactMatchManagerId,&entry,&params);

        if(st==GT_OK)
        {
            numEntriesAdded++;
            numIndexesAdded+=2;/* adding 19B entries */
            ii+=2;

            counters.ipcl0ClientKeySizeEntriesArray[PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E] += 1;
            counters.usedEntriesIndexes     += 2; /* 19B hold 2 index */
            counters.freeEntriesIndexes     = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntriesIndexes;
            /* We can not update the bank counters in case of randon addition since we do not know
               at this point what banks were selected as index to be used */
        }
        else
        {
            if (st==GT_ALREADY_EXIST)
            {
                ii++;
            }
            else
            {
                if (st==GT_FULL)
                {
                    st=GT_OK;
                    break;
                }
                else
                {
                    /* illegal return value */
                     UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerEntryAdd: expected to GT_OK on manager [%d]",exactMatchManagerId);
                     break;
                }
            }
        }
    }
    maxTotalEntriesToDelete=ii;

    if (maxTotalEntries!=0)
    {
        PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n", 
            maxTotalEntries,numIndexesAdded,
                (100*numIndexesAdded)/maxTotalEntries);
    }
    else
    {
        PRV_UTF_LOG0_MAC("ended - no entries to add, maxTotalEntries=0 \n");
    }
    /* verify counters after all additions are done*/
    prvTgfExactMatchManagerCounterVerify(exactMatchManagerId, &counters, GT_FALSE);
}

/**
* @internal prvTgfExactMatchManagerHaFullCapacityEntriesDelete function
* @endinternal
*
* @brief   Delete entries added to the exact match table
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityEntriesDelete
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC        params;
    GT_U32                                                  ii;
    GT_U32                                                  rand32Bits; /* random value in 32 bits (random 30 bits +
                                                                           2 bits from iterationNum) */
    GT_U32                                                  numEntriesDeleted=0;/* total number of entries added to the DB */
    GT_U32                                                  numIndexesDeleted;  /* number of entries deleted from each bank;
                                                                                if one entry is deleted with keysize=19 then:
                                                                                numEntriesDeleted=1 and numIndexesDeleted=2 */
    /* counters that cound the number of indexes we try to delete for the second time */
    GT_U32                                                  numEntriesAlreadyDeleted=0;
    GT_U32                                                  numIndexesAlreadyDeleted=0;

    numEntriesDeleted = 0;
    numIndexesDeleted = 0;

    prepareTtiPclFullPathConfigSet(pktCmd,&params);

    switch (pktCmd)
    {
    case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
       /* nothig to do
          all was set during the call to prepareTtiPclFullPathConfigSet */
        break;

    case CPSS_PACKET_CMD_FORWARD_E:
       /* AUTODOC: set Exact Match Entry - Reduced entry set forward port to be 1
          we already set an entry to be forwarded to port 2  -
          so we want all the rest of the entries to be forwarded to port 1*/
        reducedUsedForExactMatchEntry=GT_TRUE;
        entry.exactMatchAction.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS];
        break;

    default:
        st=GT_NOT_SUPPORTED;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerTtiPclFullPathConfigSet FAILED pktCmd not supported in the test : %d", pktCmd);
        break;
    }

    /* add entries to manager */
    entry.exactMatchEntry.key.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;
    /* set specific seed for random generator */
    cpssOsSrand(exactMatchManagerId); /* use exactMatchManagerId as seed value */
    for(ii = 0 ; ii < maxTotalEntriesToDelete; /*update according to entry size*/)
    {
        rand32Bits = (cpssOsRand() & 0x7FFF) << 15 |
                     (cpssOsRand() & 0x7FFF)       |
                     ii << 30;

        entry.exactMatchUserDefined = rand32Bits & 0xFFFF;/* 16 bits */

        entry.exactMatchEntry.key.pattern[0]=(GT_U8)(rand32Bits >> 24);
        entry.exactMatchEntry.key.pattern[1]=(GT_U8)(rand32Bits >> 16);
        entry.exactMatchEntry.key.pattern[2]=(GT_U8)(rand32Bits >>  8);
        entry.exactMatchEntry.key.pattern[3]=(GT_U8)(rand32Bits >>  0);

        entry.exactMatchEntry.key.pattern[15]=(GT_U8)(rand32Bits >> 24);
        entry.exactMatchEntry.key.pattern[16]=(GT_U8)(rand32Bits >> 16);
        entry.exactMatchEntry.key.pattern[17]=(GT_U8)(rand32Bits >>  8);
        entry.exactMatchEntry.key.pattern[18]=(GT_U8)(rand32Bits >>  0);

        st = prvTgfExactMatchManagerEntryDelete(exactMatchManagerId,&entry);

        if(st==GT_OK)
        {
            numEntriesDeleted++;
            numIndexesDeleted+=2;/* deleting 19B entries */
            ii+=2;

            counters.ipcl0ClientKeySizeEntriesArray[PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E] -= 1;
            counters.usedEntriesIndexes     -= 2; /* 19B hold 2 index */
            counters.freeEntriesIndexes     = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntriesIndexes;
            /* We can not update the bank counters in case of randon addition since we do not know
               at this point what banks were selected as index to be used
               counters.bankCounters[0]        +=1;
               counters.bankCounters[1]        +=1;*/
        }
        else
        {
            if (st==GT_NOT_FOUND)
            {
                numEntriesAlreadyDeleted++;
                numIndexesAlreadyDeleted++;
                ii++;
            }
            else
            {
               /* illegal return value */
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerEntryAdd: expected to GT_OK on manager [%d]",exactMatchManagerId);
               break;
            }
        }
    }
    if (maxTotalEntriesToDelete!=0)
    {
        PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n", 
        maxTotalEntriesToDelete,(numIndexesDeleted+numEntriesAlreadyDeleted),
            (100*(numIndexesDeleted+numIndexesAlreadyDeleted))/maxTotalEntriesToDelete);
    }
    else
    {
        PRV_UTF_LOG0_MAC("ended - no entries to delete \n");
    }

    /* verify counters after all additions are done*/
    prvTgfExactMatchManagerCounterVerify(exactMatchManagerId, &counters, GT_FALSE);
}

/**
* @internal prvTgfExactMatchManagerHaFullCapacityManagerDelete function
* @endinternal
*
* @brief   Delete Exact Match Manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityManagerDelete
(
    IN GT_U32                               exactMatchManagerId
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC     pairListArr[1];
    GT_U32                                                  numOfPairs=1;

    /* clean counters */
    cpssOsMemSet(&counters, 0, sizeof(counters));

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
* @internal prvTgfExactMatchManagerHaFullCapacityBasicConfigAdd function
* @endinternal
*
* @brief
*       configure TTI , PCL and Excat Match Manager in such way that
*       we can send a packet that will get a match in the Exact Match
*       Entry. This is a basic configuration for a HA test using a
*       single Exact Match Entry.
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] firstCall           - GT_TRUE: calling this API for the first time
*                                  GT_FALSE: calling this API for the second time
*
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityBasicConfigAdd
(
    IN GT_U32   exactMatchManagerId,
    IN GT_BOOL  firstCall
)
{
    /* Create manager with device */
    prvTgfExactMatchManagerHaFullCapacityReducedUdb19ManagerCreate(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E);

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(firstCall);

    /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Exact Match Manager Expanded Action PCL configuration with keySize=19B */
    prvTgfExactMatchManagerHaFullCapacityReducedUdb19ConfigSet(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E);

    /* add as many as possible entries to the exact match use rehashEnable=GT_TRUE */
    prvTgfExactMatchManagerHaFullCapacityEntriesAdd(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E,firstCall);
}

/**
* @internal prvTgfExactMatchManagerHaFullCapacityValidityCheckAndTrafficSend function
* @endinternal
*
* @brief
*      call validity check and sent traffic
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] firstCall           - GT_TRUE: calling this API for the first time
*                                  GT_FALSE: calling this API for the second time
*
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityValidityCheckAndTrafficSend
(
    IN GT_U32   exactMatchManagerId
)
{
    /* check validity */
    PRV_TGF_EXACT_MATCH_MANAGER_DB_CHECKS_STC               dbChecks;
    PRV_TGF_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT        dbResultArray[PRV_TGF_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNumber;

    dbChecks.globalCheckEnable = GT_TRUE;
    dbChecks.countersCheckEnable = GT_TRUE;
    dbChecks.dbFreeListCheckEnable = GT_TRUE;
    dbChecks.dbUsedListCheckEnable = GT_TRUE;
    dbChecks.dbIndexPointerCheckEnable = GT_TRUE;
    dbChecks.dbAgingBinCheckEnable = GT_TRUE;
    dbChecks.dbAgingBinUsageMatrixCheckEnable = GT_TRUE;
    dbChecks.hwUsedListCheckEnable = GT_TRUE;
    dbChecks.cuckooDbCheckEnable = GT_TRUE;

    prvTgfExactMatchManagerDatabaseCheck(exactMatchManagerId,&dbChecks,dbResultArray,&errorNumber);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, errorNumber, "prvTgfExactMatchManagerDatabaseCheck: expected to get errorNumber=0 on manager [%d]",exactMatchManagerId);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);
}

/**
* @internal prvTgfExactMatchManagerHaFullCapacitySystemRecoveryStateSet function
* @endinternal
*
* @brief  Keep Exact Match Manager DB values for reconstruct and
*         set flag for HA process.
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacitySystemRecoveryStateSet()
{
    CPSS_SYSTEM_RECOVERY_INFO_STC newSystemRecoveryInfo;
    cpssOsMemSet(&oldSystemRecoveryInfo, 0, sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));
    cpssSystemRecoveryStateGet(&oldSystemRecoveryInfo);

    newSystemRecoveryInfo = oldSystemRecoveryInfo ;
    newSystemRecoveryInfo.systemRecoveryProcess=CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
    newSystemRecoveryInfo.systemRecoveryState=CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E;
    prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);
}

/**
* @internal prvTgfExactMatchManagerHaFullCapacityDelBasicConfig function
* @endinternal
*
* @brief  Delete all Exact Match configuration done
*
* @param[in] exactMatchManagerId - manager Id
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityDelBasicConfig
(
    IN GT_U32   exactMatchManagerId
)
{
    GT_STATUS   rc=GT_OK;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC        params;

    /* delete the single ExactMatch entry previously configured */
    prepareTtiPclFullPathConfigSet(CPSS_PACKET_CMD_FORWARD_E,&params);
    reducedUsedForExactMatchEntry=GT_TRUE;
    entry.exactMatchAction.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];
    entry.exactMatchEntry.key.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;

    rc= prvTgfExactMatchManagerEntryDelete(exactMatchManagerId,&entry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchManagerEntryDelete: expected to GT_OK on manager [%d]",exactMatchManagerId);

    if (rc==GT_OK)
    {
        counters.ipcl0ClientKeySizeEntriesArray[PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E] -= 1;
        counters.usedEntriesIndexes     -= 2; /* 19B hold 2 index */
        counters.freeEntriesIndexes     = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntriesIndexes;
        /* verify counters after all additions are done*/
        prvTgfExactMatchManagerCounterVerify(exactMatchManagerId, &counters, GT_FALSE);
    }

    /* delete all entries randomly added */
    prvTgfExactMatchManagerHaFullCapacityEntriesDelete(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E);

    /* Delete manager and device */
    prvTgfExactMatchManagerHaFullCapacityManagerDelete(exactMatchManagerId);
}

/**
* @internal prvTgfExactMatchManagerHaFullCapacitySyncSwHw function
* @endinternal
*
* @brief  do any suncronization left between HW and Shadow
*         set complition state
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacitySyncSwHw()
{
    GT_STATUS   rc=GT_OK;
    CPSS_SYSTEM_RECOVERY_INFO_STC newSystemRecoveryInfo;

    /* set complete flag */
    newSystemRecoveryInfo = oldSystemRecoveryInfo;
    newSystemRecoveryInfo.systemRecoveryState=CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);

    /* call function to sync shadow and HW
       invalidate not needed entries in all Exact Match Managers
       Delete temporary database */
    rc = prvTgfExactMatchManagerCompletionForHa();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfLpmRamSyncSwHwForHa\n");

    /* return to the values we had before EMM HA process */
    prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
}

/**
* @internal prvTgfExactMatchManagerHaFullCapacityBasicConfigRestore function
* @endinternal
*
* @brief  restore old configuration and delete exact match entry and manager
*
* @param[in] exactMatchManagerId - manager Id
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityBasicConfigRestore
(
    IN GT_U32   exactMatchManagerId
)
{
    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Delete manager and device */
    prvTgfExactMatchManagerHaFullCapacityManagerDelete(exactMatchManagerId);
}
