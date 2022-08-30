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
* @file prvTgfExactMatchManagerExpandedActionUdbPcl.c
*
* @brief Test Exact Match Manager functionality with PCL Action Type
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


#include <exactMatch/prvTgfExactMatchUdbPcl.h>
#include <common/tgfExactMatchManagerGen.h>

/* we force application to give 'num of entries' in steps of 256 */
#define NUM_ENTRIES_STEPS_CNS                       0/* TBD when aging support will be added 256*/
#define PRV_TGF_MAX_TOTAL_ENTRIES                   (PRV_TGF_TOTAL_HW_CAPACITY - NUM_ENTRIES_STEPS_CNS)

static GT_U8 udbPattern[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];
static GT_U8 udbMask[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

/* original VLAN Id */
#define PRV_TGF_PASSENGER_TAG0_VLAN_ID_CNS   0x10

/* original VLAN Id */
#define PRV_TGF_PASSENGER_TAG1_VLAN_ID_CNS   0x11

/* original VLAN Id */
#define PRV_TGF_TUNNEL_TAG0_VLAN_ID_CNS      0x21

/* original VLAN Id */
#define PRV_TGF_TUNNEL_TAG1_VLAN_ID_CNS      0x22

/* lookup1 VLAN Id */
#define PRV_TGF_PCL_NEW_VLAN_ID_CNS          0x31

/* Action Flow Id  */
#define PRV_TGF_TTI_ACTION_FLOW_ID_CNS       0x25

/* Action Flow Id  */
#define PRV_TGF_PCL_ACTION_FLOW_ID_CNS       0x26

/* Router Source MAC_SA index  */
#define PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS  0x12

/* TCP Sorce port  */
#define PRV_TGF_PCL_TCP_SRC_PORT_CNS         0x7654

/* TCP Destination port  */
#define PRV_TGF_PCL_TCP_DST_PORT_CNS         0xFECB

/* UDP Sorce port  */
#define PRV_TGF_PCL_UDP_SRC_PORT_CNS         0x4567

/* UDP Destination port  */
#define PRV_TGF_PCL_UDP_DST_PORT_CNS         0xBCEF

/* IPV6 Sorce IP  */
#define PRV_TGF_PCL_IPV6_SRC_IP_CNS          {0x23, 0x45, 0x67, 0x89, 0x87, 0x65, 0x43, 0x21}

/* IPV6 Destination IP  */
#define PRV_TGF_PCL_IPV6_DST_IP_CNS          {0x01, 0x2E, 0x3D, 0x4C, 0x5B, 0x6A, 0x79, 0x88}


/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketPassengerL2Part =
{
    { 0x00, 0x12, 0x34, 0x56, 0x78, 0x9A },                /* daMac */
    { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 }                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketPassengerVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,              /* etherType */
    0, 0, PRV_TGF_PASSENGER_TAG0_VLAN_ID_CNS      /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketPassengerVlanTag1Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,              /* etherType */
    0, 0, PRV_TGF_PASSENGER_TAG1_VLAN_ID_CNS      /* pri, cfi, VlanId */
};

/* Ethernet Type part */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthOth1EthertypePart =
{
    0x3456
};

/* Payload of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),            /* dataLength */
    prvTgfPacketPayloadDataArr                     /* dataPtr */
};

/* Double Tagged Ethernet Other Packet Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketNotTunneled2tagsEthOthPartsArray[] = {
    { TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part },  /* type, partPtr */
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part },
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part },
    { TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOth1EthertypePart },
    { TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart }
};

/* Double Tagged Ethernet Other Packet Info */
static TGF_PACKET_STC prvTgfPacketNotTunneled2tagsEthOthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(prvTgfPacketPayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacketNotTunneled2tagsEthOthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketNotTunneled2tagsEthOthPartsArray                                        /* partsArray */
};

/* use Exact Match size that match the fineTunning */
#define TGF_EXACT_MATCH_SIZE_FULL (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.emNum)
#define TGF_EXACT_MATCH_MAX_BANKS (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchNumOfBanks)

#define TGF_EXACT_MATCH_PROFILE_ID_CNS              5

/* we force application to give 'num of entries' in steps of 256 */
#define TGF_EXACT_MATCH_NUM_ENTRIES_STEPS_CNS       256

/* create exact match manager - minimal parameters for tests */
static GT_STATUS createExactMatchManager
(
    IN GT_U32                                          exactMatchManagerId
)
{
    GT_STATUS   st=GT_OK;
    PRV_TGF_EXACT_MATCH_MANAGER_CAPACITY_STC           capacityInfo;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC   entryAttrInfo;
    PRV_TGF_EXACT_MATCH_MANAGER_LOOKUP_STC             lookupInfo;
    PRV_TGF_EXACT_MATCH_MANAGER_AGING_STC              agingInfo;

    PRV_TGF_PCL_RULE_FORMAT_UNT                        mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT                        pattern;
    GT_U32                                             udbAmount;

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
    lookupInfo.lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClient=PRV_TGF_EXACT_MATCH_CLIENT_IPCL_0_E;
    lookupInfo.lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsNum=1;
    lookupInfo.lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].pclMappingElem.packetType=PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E;
    lookupInfo.lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].pclMappingElem.subProfileId = 0;
    lookupInfo.lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].pclMappingElem.enableExactMatchLookup=GT_TRUE;
    lookupInfo.lookupsArray[PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].pclMappingElem.profileId=TGF_EXACT_MATCH_PROFILE_ID_CNS;
    lookupInfo.profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
    lookupInfo.profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.keyStart = 0;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    udbAmount = 33;
    prvTgfExactMatchPclUdbMaskPatternBuild(udbAmount,
                                           &prvTgfPacketNotTunneled2tagsEthOthInfo,/* packetPtr      */
                                           0,                                      /* offsetInPacket */
                                           udbMask,
                                           udbPattern);

    cpssOsMemCpy((GT_VOID*) &(lookupInfo.profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.mask[0]), &udbMask, sizeof(GT_U8)*PRV_TGF_EXACT_MATCH_MAX_KEY_SIZE_CNS);

    /* entry configuration; take all fields from Expanded Action */
    entryAttrInfo.expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].exactMatchExpandedEntryValid = GT_TRUE;
    entryAttrInfo.expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedActionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;
    entryAttrInfo.expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.pclAction.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    /* For SIP6 and above for the egress control pipe the CPU code is also 'drop code' so relevant for drops too. */
    entryAttrInfo.expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.pclAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E+1;
    entryAttrInfo.expandedArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].expandedAction.pclAction.egressPolicy = GT_FALSE;

    st = prvTgfExactMatchManagerCreate(exactMatchManagerId,&capacityInfo,&lookupInfo,&entryAttrInfo,&agingInfo);

    return st;
}

/**
* @internal prvTgfExactMatchManagerExpandedActionUdbPclManagerCreate function
* @endinternal
*
* @brief   Create Exact Match Manager with device
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerExpandedActionUdbPclManagerCreate
(
    IN GT_U32                               exactMatchManagerId
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC     pairListArr[1];
    GT_U32                                                  numOfPairs=1;

    pairListArr[0].devNum=prvTgfDevNum;
    pairListArr[0].portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* create single valid manager */
    st = createExactMatchManager(exactMatchManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "createExactMatchManager: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* add device to it */
    st = prvTgfExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",exactMatchManagerId);
}

/**
* @internal prvTgfExactMatchManagerExpandedActionUdbPclConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Expanded Action
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerExpandedActionUdbPclConfigSet
(
    IN GT_U32                               exactMatchManagerId
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_STC                   entry;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC        params;
    PRV_TGF_EXACT_MATCH_MANAGER_COUNTERS_STC                counters;

    /* reset params */
    cpssOsMemSet(&entry,0,sizeof(PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_STC));
    cpssOsMemSet(&params,0,sizeof(PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC));

    entry.exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
    /* here we check exactly the same pattern set for the PCL match in TCAM */
    cpssOsMemCpy((GT_VOID*) &entry.exactMatchEntry.key.pattern[0], &udbPattern, sizeof(entry.exactMatchEntry.key.pattern));
    /* same as configured in Expanded Entry */
    entry.exactMatchEntry.lookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    entry.expandedActionIndex = TGF_EXACT_MATCH_PROFILE_ID_CNS;
    entry.exactMatchActionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;
    entry.exactMatchAction.pclAction.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    /* For SIP6 and above for the egress control pipe the CPU code is also 'drop code' so relevant for drops too. */
    entry.exactMatchAction.pclAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E+1;
    entry.exactMatchAction.pclAction.egressPolicy = GT_FALSE;

    params.rehashEnable = GT_FALSE;

    /* add entry to manager */
    st = prvTgfExactMatchManagerEntryAdd(exactMatchManagerId,&entry,&params);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerEntryAdd: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* Verify Counter - After entry addition */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    counters.ipcl0ClientKeySizeEntriesArray[PRV_TGF_EXACT_MATCH_KEY_SIZE_33B_E] += 1;
    counters.usedEntriesIndexes     += 3; /* 33B hold 3 index */
    counters.freeEntriesIndexes     = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntriesIndexes;
    counters.bankCounters[0]        +=1;
    counters.bankCounters[1]        +=1;
    counters.bankCounters[2]        +=1;
    prvTgfExactMatchManagerCounterVerify(exactMatchManagerId, &counters, GT_TRUE);
}

/**
* @internal prvTgfExactMatchManagerExpandedActionUdbPclManagerDelete function
* @endinternal
*
* @brief   Delete Exact Match Manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerExpandedActionUdbPclManagerDelete
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
* @internal prvTgfExactMatchManagerExpandedActionUdbPclInvalidateEmEntry function
* @endinternal
*
* @brief   Delete Exact Match Entry from manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerExpandedActionUdbPclInvalidateEmEntry
(
    IN GT_U32                               exactMatchManagerId
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_STC                   entry;

    /* reset params */
    cpssOsMemSet(&entry,0,sizeof(PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_STC));

    entry.exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
    /* here we check exactly the same pattern set for the PCL match in TCAM */
    cpssOsMemCpy((GT_VOID*) &entry.exactMatchEntry.key.pattern[0], &udbPattern, sizeof(entry.exactMatchEntry.key.pattern));
    /* same as configured in Expanded Entry */
    entry.exactMatchEntry.lookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    entry.expandedActionIndex = TGF_EXACT_MATCH_PROFILE_ID_CNS;
    entry.exactMatchActionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;
    entry.exactMatchAction.pclAction.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    /* For SIP6 and above for the egress control pipe the CPU code is also 'drop code' so relevant for drops too. */
    entry.exactMatchAction.pclAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E+1;
    entry.exactMatchAction.pclAction.egressPolicy = GT_FALSE;

    /* add entry to manager */
    st = prvTgfExactMatchManagerEntryDelete(exactMatchManagerId,&entry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerEntryDelete: expected to GT_OK on manager [%d]",exactMatchManagerId);
}

