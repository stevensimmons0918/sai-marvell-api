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
* @file prvTgfExactMatchManagerTtiPclFullPath.c
*
* @brief Test Exact Match Manager functionality with TTI and PCL configuration
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

/* we force application to give 'num of entries' in steps of 256 */
#define NUM_ENTRIES_STEPS_CNS                       0/* TBD when aging support will be added 256*/
#define PRV_TGF_MAX_TOTAL_ENTRIES                   (PRV_TGF_TOTAL_HW_CAPACITY - NUM_ENTRIES_STEPS_CNS)

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

/* create exact match manager - minimal parameters for tests */
static GT_STATUS createExactMatchManager
(
    IN GT_U32                                          exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                             pktCmd
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

    st = prvTgfExactMatchManagerCreate(exactMatchManagerId,&capacityInfo,&lookupInfo,&entryAttrInfo,&agingInfo);

    return st;
}

/* create exact match manager - minimal parameters for tests
   overide port from reduced action */
static GT_STATUS createExactMatchReducedManager
(
    IN GT_U32                                          exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                             pktCmd
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

/* create exact match manager - minimal parameters for tests
   overide port from reduced action set keyStart!=0 */
static GT_STATUS createExactMatchReducedNonZeroKeyStartManager
(
    IN GT_U32                                          exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                             pktCmd
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

    /* Set Exact Match keyParams.keySize to be != 0
       UDB0 should be removed from TCAM key - and we should get a hit on the Exact Match*/

    lookupInfo.profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.keyStart = 1;

    cpssOsMemSet(pattern.ruleIngrUdbOnly.udb, 0, sizeof(pattern.ruleIngrUdbOnly.udb));
    cpssOsMemSet(mask.ruleIngrUdbOnly.udb, 0xFF, sizeof(mask.ruleIngrUdbOnly.udb));/* full mask for first stage */

    /* pattern */
    /*pattern.ruleIngrUdbOnly.udb[0]=PRV_TGF_PCL_ID_CNS; keyParams.keyStart=1; so this UDB will not be used in EM key*//* {PCL-ID[7:0]}*/
    pattern.ruleIngrUdbOnly.udb[0]=0x80;                /* {UDB Valid,reserved,PCL-ID[9:8]}*/
    pattern.ruleIngrUdbOnly.udb[1]=(PRV_TGF_VLANID_CNS&0xFF);     /* eVLAN LSB [7:0] */
    pattern.ruleIngrUdbOnly.udb[2]=((PRV_TGF_VLANID_CNS>>8)&0x1F);/* eVLAN MSB [12:8]*/
    pattern.ruleIngrUdbOnly.udb[3]=(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]&0xFF);/* Source-ePort[7:0] */
    pattern.ruleIngrUdbOnly.udb[4]=((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]>>8)&0x1F);/* Source-ePort[12:8] */

    /*{ 0x00, 0x00, 0x44, 0x33, 0x22, 0x11 } - daMac */
    pattern.ruleIngrUdbOnly.udb[5]=0;       /* Dst Mac MSB      */
    pattern.ruleIngrUdbOnly.udb[6]=0;       /* Dst Mac byte 1   */
    pattern.ruleIngrUdbOnly.udb[7]=0x44;    /* Dst Mac byte 2   */
    pattern.ruleIngrUdbOnly.udb[8]=0x33;    /* Dst Mac byte 3   */
    pattern.ruleIngrUdbOnly.udb[9]=0x22;    /* Dst Mac byte 4   */
    pattern.ruleIngrUdbOnly.udb[10]=0x11;   /* Dst Mac LSB      */

    mask.ruleIngrUdbOnly.udb[2] = 0x1F;
    mask.ruleIngrUdbOnly.udb[4] = 0x1F;

    cpssOsMemCpy((GT_VOID*) &(lookupInfo.profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.mask[0]), mask.ruleIngrUdbOnly.udb, sizeof(GT_U8)*PRV_TGF_EXACT_MATCH_MAX_KEY_SIZE_CNS);

    st = prvTgfExactMatchManagerCreate(exactMatchManagerId,&capacityInfo,&lookupInfo,&entryAttrInfo,&agingInfo);

    return st;
}

/* create exact match manager - minimal parameters for tests
   overide port from reduced action set mask!=full */
static GT_STATUS createExactMatchReducedNonFullKeyMaskManager
(
    IN GT_U32                                          exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                             pktCmd
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

    cpssOsMemSet(mask.ruleIngrUdbOnly.udb, 0xFF, sizeof(mask.ruleIngrUdbOnly.udb));/* full mask for first stage */

    /* set mask different form full 0xFF */
    mask.ruleIngrUdbOnly.udb[0] = PRV_TGF_PCL_ID_CNS;
    mask.ruleIngrUdbOnly.udb[1] = 0x80;
    mask.ruleIngrUdbOnly.udb[2] = (PRV_TGF_VLANID_CNS&0xFF);
    mask.ruleIngrUdbOnly.udb[3] = ((PRV_TGF_VLANID_CNS>>8)&0x1F);
    mask.ruleIngrUdbOnly.udb[4] = (prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]&0xFF);
    mask.ruleIngrUdbOnly.udb[5] = ((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]>>8)&0x1F);
    mask.ruleIngrUdbOnly.udb[6] = 0;
    mask.ruleIngrUdbOnly.udb[7] = 0;
    mask.ruleIngrUdbOnly.udb[8] = 0x44;
    mask.ruleIngrUdbOnly.udb[9] = 0x33;
    mask.ruleIngrUdbOnly.udb[10] = 0x22;
    mask.ruleIngrUdbOnly.udb[11] = 0x11;

    cpssOsMemCpy((GT_VOID*) &(lookupInfo.profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.mask[0]), mask.ruleIngrUdbOnly.udb, sizeof(GT_U8)*PRV_TGF_EXACT_MATCH_MAX_KEY_SIZE_CNS);

    st = prvTgfExactMatchManagerCreate(exactMatchManagerId,&capacityInfo,&lookupInfo,&entryAttrInfo,&agingInfo);

    return st;
}

/* create exact match manager - minimal parameters for tests
   overide port from reduced action set keySize=47B */
static GT_STATUS createExactMatchReducedUdb47Manager
(
    IN GT_U32                                          exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                             pktCmd
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

    /* Set Exact Match keyParams
       keyParams.keySize = 47 bytes
       keyParams.keyStart = 1
       keyParams.mask != 0xff
       UDB0 should be removed from TCAM key and we should get a hit on the Exact Match */
    lookupInfo.profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E;
    lookupInfo.profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.keyStart = 1;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    cpssOsMemSet(pattern.ruleIngrUdbOnly.udb, 0, sizeof(pattern.ruleIngrUdbOnly.udb));
    cpssOsMemSet(mask.ruleIngrUdbOnly.udb, 0xFF, sizeof(mask.ruleIngrUdbOnly.udb));/* full mask for first stage */

    /* pattern */
    /*pattern.ruleIngrUdbOnly.udb[0]=PRV_TGF_PCL_ID_CNS;*//* {PCL-ID[7:0]}*/
    pattern.ruleIngrUdbOnly.udb[0]=0x80;                /* {UDB Valid,reserved,PCL-ID[9:8]}*/
    pattern.ruleIngrUdbOnly.udb[1]=(PRV_TGF_VLANID_CNS&0xFF);     /* eVLAN LSB [7:0] */
    pattern.ruleIngrUdbOnly.udb[2]=((PRV_TGF_VLANID_CNS>>8)&0x1F);/* eVLAN MSB [12:8]*/
    pattern.ruleIngrUdbOnly.udb[3]=(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]&0xFF);/* Source-ePort[7:0] */
    pattern.ruleIngrUdbOnly.udb[4]=((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]>>8)&0x1F);/* Source-ePort[12:8] */

    /*{ 0x00, 0x00, 0x44, 0x33, 0x22, 0x11 } - daMac */
    pattern.ruleIngrUdbOnly.udb[5]=0;       /* Dst Mac MSB      */
    pattern.ruleIngrUdbOnly.udb[6]=0;       /* Dst Mac byte 1   */
    pattern.ruleIngrUdbOnly.udb[7]=0x44;    /* Dst Mac byte 2   */
    pattern.ruleIngrUdbOnly.udb[8]=0x33;    /* Dst Mac byte 3   */
    pattern.ruleIngrUdbOnly.udb[9]=0x22;    /* Dst Mac byte 4   */
    pattern.ruleIngrUdbOnly.udb[10]=0x11;   /* Dst Mac LSB      */

    mask.ruleIngrUdbOnly.udb[0] = 0x80;
    mask.ruleIngrUdbOnly.udb[1] = (PRV_TGF_VLANID_CNS&0xFF);
    mask.ruleIngrUdbOnly.udb[2] = ((PRV_TGF_VLANID_CNS>>8)&0x1F);
    mask.ruleIngrUdbOnly.udb[3] = (prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]&0xFF);
    mask.ruleIngrUdbOnly.udb[4] = ((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]>>8)&0x1F);
    mask.ruleIngrUdbOnly.udb[5] = 0;
    mask.ruleIngrUdbOnly.udb[6] = 0;
    mask.ruleIngrUdbOnly.udb[7] = 0x44;
    mask.ruleIngrUdbOnly.udb[8] = 0x33;
    mask.ruleIngrUdbOnly.udb[9] = 0x22;
    mask.ruleIngrUdbOnly.udb[10] = 0x11;

    cpssOsMemCpy((GT_VOID*) &(lookupInfo.profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.mask[0]), mask.ruleIngrUdbOnly.udb, sizeof(GT_U8)*PRV_TGF_EXACT_MATCH_MAX_KEY_SIZE_CNS);

    st = prvTgfExactMatchManagerCreate(exactMatchManagerId,&capacityInfo,&lookupInfo,&entryAttrInfo,&agingInfo);

    return st;
}

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathManagerCreate function
* @endinternal
*
* @brief   Create Exact Match Manager with device
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathManagerCreate
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC     pairListArr[1];
    GT_U32                                                  numOfPairs=1;

    pairListArr[0].devNum=prvTgfDevNum;
    pairListArr[0].portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* create single valid manager */
    st = createExactMatchManager(exactMatchManagerId,pktCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "createExactMatchManager: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* add device to it */
    st = prvTgfExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",exactMatchManagerId);
}

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathReducedManagerCreate function
* @endinternal
*
* @brief   Create Exact Match Manager with device and set reduce entry to be used
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathReducedManagerCreate
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC     pairListArr[1];
    GT_U32                                                  numOfPairs=1;

    pairListArr[0].devNum=prvTgfDevNum;
    pairListArr[0].portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* create single valid manager */
    st = createExactMatchReducedManager(exactMatchManagerId,pktCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "createExactMatchReducedManager: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* add device to it */
    st = prvTgfExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",exactMatchManagerId);
}

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathReducedNonZeroKeyStartManagerCreate function
* @endinternal
*
* @brief   Create Exact Match Manager with device and set reduce entry to be used, with non zero key start
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathReducedNonZeroKeyStartManagerCreate
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC     pairListArr[1];
    GT_U32                                                  numOfPairs=1;

    pairListArr[0].devNum=prvTgfDevNum;
    pairListArr[0].portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* create single valid manager */
    st = createExactMatchReducedNonZeroKeyStartManager(exactMatchManagerId,pktCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "createExactMatchReducedNonZeroKeyStartManager: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* add device to it */
    st = prvTgfExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",exactMatchManagerId);
}


/**
* @internal prvTgfExactMatchManagerTtiPclFullPathReducedNonFullKeyMaskManagerCreate function
* @endinternal
*
* @brief   Create Exact Match Manager with device and set reduce
*          entry to be used, with non full key mask
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathReducedNonFullKeyMaskManagerCreate
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC     pairListArr[1];
    GT_U32                                                  numOfPairs=1;

    pairListArr[0].devNum=prvTgfDevNum;
    pairListArr[0].portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* create single valid manager */
    st = createExactMatchReducedNonFullKeyMaskManager(exactMatchManagerId,pktCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "createExactMatchReducedNonFullKeyMaskManager: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* add device to it */
    st = prvTgfExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",exactMatchManagerId);
}

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathReducedUdb47ManagerCreate function
* @endinternal
*
* @brief   Create Exact Match Manager with device and set reduce
*          entry to be used, with UDB 47B key size
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathReducedUdb47ManagerCreate
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC     pairListArr[1];
    GT_U32                                                  numOfPairs=1;

    pairListArr[0].devNum=prvTgfDevNum;
    pairListArr[0].portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* create single valid manager */
    st = createExactMatchReducedUdb47Manager(exactMatchManagerId,pktCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "createExactMatchReducedUdb47Manager: expected to GT_OK on manager [%d]",exactMatchManagerId);

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
* @internal prvTgfExactMatchManagerTtiPclFullPathConfigSet function
* @endinternal
*
* @brief   Set PCL/TTI test configuration related to Exact Match Expanded Action
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathConfigSet
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC        params;
    PRV_TGF_EXACT_MATCH_MANAGER_COUNTERS_STC                counters;

    prepareTtiPclFullPathConfigSet(pktCmd,&params);

    /* add entry to manager */
    st = prvTgfExactMatchManagerEntryAdd(exactMatchManagerId,&entry,&params);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerEntryAdd: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* Verify Counter - After entry addition */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    counters.ipcl0ClientKeySizeEntriesArray[PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E] += 1;
    counters.usedEntriesIndexes     += 2; /* 19B hold 2 index */
    counters.freeEntriesIndexes     = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntriesIndexes;
    counters.bankCounters[0]        +=1;
    counters.bankCounters[1]        +=1;
    prvTgfExactMatchManagerCounterVerify(exactMatchManagerId, &counters, GT_TRUE);
}

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathReducedConfigSet function
* @endinternal
*
* @brief   Set PCL/TTI test configuration related to Exact Match Expanded Action
*          and Reduced Action
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathReducedConfigSet
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC        params;
    PRV_TGF_EXACT_MATCH_MANAGER_COUNTERS_STC                counters;

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
    st = prvTgfExactMatchManagerEntryAdd(exactMatchManagerId,&entry,&params);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerEntryAdd: expected to GT_OK on manager [%d]",exactMatchManagerId);
    /* Verify Counter - After entry addition */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    counters.ipcl0ClientKeySizeEntriesArray[PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E] += 1;
    counters.usedEntriesIndexes     += 2; /* 19B hold 2 index */
    counters.freeEntriesIndexes     = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntriesIndexes;
    counters.bankCounters[0]        +=1;
    counters.bankCounters[1]        +=1;
    prvTgfExactMatchManagerCounterVerify(exactMatchManagerId, &counters, GT_TRUE);
}

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathReducedUdb47ConfigSet function
* @endinternal
*
* @brief   Set PCL/TTI test configuration related to Exact Match Expanded Action
*          and Reduced Action with keySize=47B
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathReducedUdb47ConfigSet
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC        params;
    PRV_TGF_EXACT_MATCH_MANAGER_COUNTERS_STC                counters;

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
    entry.exactMatchEntry.key.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E;
    st = prvTgfExactMatchManagerEntryAdd(exactMatchManagerId,&entry,&params);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerEntryAdd: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* Verify Counter - After entry addition */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    counters.ipcl0ClientKeySizeEntriesArray[PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E] += 1;
    counters.usedEntriesIndexes     += 4; /* 47B hold 4 index */
    counters.freeEntriesIndexes     = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntriesIndexes;
    counters.bankCounters[0]        +=1;
    counters.bankCounters[1]        +=1;
    counters.bankCounters[2]        +=1;
    counters.bankCounters[3]        +=1;
    prvTgfExactMatchManagerCounterVerify(exactMatchManagerId, &counters, GT_TRUE);
}

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathManagerDelete function
* @endinternal
*
* @brief   Delete Exact Match Manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathManagerDelete
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
* @internal prvTgfExactMatchManagerTtiPclFullPathInvalidateEmEntry function
* @endinternal
*
* @brief   Delete Exact Match Entry from manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathInvalidateEmEntry
(
    IN GT_U32                               exactMatchManagerId
)
{
    GT_STATUS                                               st;

    /* delete entry from manager */
    st = prvTgfExactMatchManagerEntryDelete(exactMatchManagerId,&entry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerEntryDelete: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* return to test default value */
    pktCmdUsedForExactMatchEntry=CPSS_PACKET_CMD_DROP_HARD_E;
}

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathHaReplayInvalidEntry function
* @endinternal
*
* @brief   Add invalid entries and expect replay to fail.
*          Add a valid entry that will not be replayed and
*          expect this entry to be deleted in the complition stage.
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
* @param[in] firstCall           - GT_TRUE: calling this API for the first time
*                                  GT_FALSE: calling this API for the second time
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaReplayInvalidEntry
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd,
    IN GT_BOOL                              firstCall
)
{
    GT_STATUS                                               st;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC        params;
    PRV_TGF_EXACT_MATCH_MANAGER_COUNTERS_STC                counters;
    PRV_TGF_EXACT_MATCH_MANAGER_STATISTICS_STC              statistics;

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

    if (firstCall == GT_TRUE)
    {
        /*  Add an entry that will not be replayed in the HA process,
            this entry will be invalidated from the HW in the complition stage */
        entry.exactMatchEntry.key.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E;
        /* change the key */
        entry.exactMatchEntry.key.pattern[0]=+5;
        entry.exactMatchEntry.key.pattern[1]=+5;
        st = prvTgfExactMatchManagerEntryAdd(exactMatchManagerId,&entry,&params);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerEntryAdd: expected to GT_OK on manager [%d]",exactMatchManagerId);

        /* Verify Counter - we have one entry from priveous replay of valid entry
           so check counters for 2 valid entries */
        cpssOsMemSet(&counters, 0, sizeof(counters));
        counters.ipcl0ClientKeySizeEntriesArray[PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E] += 2;
        counters.usedEntriesIndexes     += 8; /* 47B hold 4 index */
        counters.freeEntriesIndexes     = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntriesIndexes;
        counters.bankCounters[0]        +=2;
        counters.bankCounters[1]        +=2;
        counters.bankCounters[2]        +=2;
        counters.bankCounters[3]        +=2;
        prvTgfExactMatchManagerCounterVerify(exactMatchManagerId, &counters, GT_TRUE);
    }
    else
    {
        /* add entry to manager -
           expect fail since this entry was not added before the HA process */
        entry.exactMatchEntry.key.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E;
        /* change the key expect fail due to key mismatch */
        entry.exactMatchEntry.key.pattern[0]++;
        entry.exactMatchEntry.key.pattern[1]++;
        st = prvTgfExactMatchManagerEntryAdd(exactMatchManagerId,&entry,&params);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st, "prvTgfExactMatchManagerEntryAdd: expected NON GT_OK on manager [%d]",exactMatchManagerId);

        st = prvTgfExactMatchManagerStatisticsGet(exactMatchManagerId, &statistics);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,st, "cpssDxChExactMatchManagerStatisticsGet: GT_OK");
        UTF_VERIFY_EQUAL0_STRING_MAC(1, statistics.entryAddErrorReplayEntryNotFound,"ERROR: cpssDxChExactMatchManagerStatisticsGet");

        entry.exactMatchEntry.key.pattern[0]--;
        entry.exactMatchEntry.key.pattern[1]--;
        /* change the action expect fail due to action mismatch*/
        entry.exactMatchAction.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS];
        st = prvTgfExactMatchManagerEntryAdd(exactMatchManagerId,&entry,&params);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st, "prvTgfExactMatchManagerEntryAdd: expected NON GT_OK on manager [%d]",exactMatchManagerId);

        st = prvTgfExactMatchManagerStatisticsGet(exactMatchManagerId, &statistics);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,st, "cpssDxChExactMatchManagerStatisticsGet: GT_OK");
        UTF_VERIFY_EQUAL0_STRING_MAC(2, statistics.entryAddErrorReplayEntryNotFound,"ERROR: cpssDxChExactMatchManagerStatisticsGet");

        /* Verify Counter - we have one entry from priveous replay of valid entry
           second entry that was added is not replayed and because of that we only
           expect one entry to be counted in the counters */
        cpssOsMemSet(&counters, 0, sizeof(counters));
        counters.ipcl0ClientKeySizeEntriesArray[PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E] += 1;
        counters.usedEntriesIndexes     += 4; /* 47B hold 4 index */
        counters.freeEntriesIndexes     = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntriesIndexes;
        counters.bankCounters[0]        +=1;
        counters.bankCounters[1]        +=1;
        counters.bankCounters[2]        +=1;
        counters.bankCounters[3]        +=1;
        prvTgfExactMatchManagerCounterVerify(exactMatchManagerId, &counters, GT_TRUE);
    }
}

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathHaReplayInvalidManager function
* @endinternal
*
* @brief   Add invalid manager and expect replay to fail.
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaReplayInvalidManager
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
)
{

    GT_STATUS                                               st=GT_OK;
    PRV_TGF_EXACT_MATCH_MANAGER_CAPACITY_STC                capacityInfo;
    PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC        entryAttrInfo;
    PRV_TGF_EXACT_MATCH_MANAGER_LOOKUP_STC                  lookupInfo;
    PRV_TGF_EXACT_MATCH_MANAGER_AGING_STC                   agingInfo;
    PRV_TGF_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC     pairListArr[1];
    GT_U32                                                  numOfPairs=1;

    pairListArr[0].devNum=prvTgfDevNum;
    pairListArr[0].portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* reset params */
    cpssOsMemSet(&capacityInfo,0,sizeof(PRV_TGF_EXACT_MATCH_MANAGER_CAPACITY_STC));
    cpssOsMemSet(&entryAttrInfo,0,sizeof(PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC));
    cpssOsMemSet(&lookupInfo,0,sizeof(PRV_TGF_EXACT_MATCH_MANAGER_LOOKUP_STC));
    cpssOsMemSet(&agingInfo,0,sizeof(PRV_TGF_EXACT_MATCH_MANAGER_AGING_STC));

    st = prepareManagerConfiguration(&capacityInfo,&entryAttrInfo,&lookupInfo,&agingInfo,pktCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prepareManagerConfiguration: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* create a single manager not equal to the one configured in createExactMatchReducedUdb47Manager
       replay should fail */
    st = prvTgfExactMatchManagerCreate(exactMatchManagerId,&capacityInfo,&lookupInfo,&entryAttrInfo,&agingInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "createExactMatchReducedUdb47Manager: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* add device to it - will get a fail in sync HW/SW check.
       the fail is due to keySize and keyStart not equal to what is configured in the HW
       SW keySize=19 , keyStart=0
       HW keySize=47 , keyStart=1 */
    st = prvTgfExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerDevListAdd: expected NON GT_OK on manager [%d]",exactMatchManagerId);

    /* remove the incorrect manager */
    st = prvTgfExactMatchManagerDelete(exactMatchManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "createExactMatchReducedUdb47Manager: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* create a single manager not equal to the one configured in createExactMatchReducedUdb47Manager
       replay should fail */
    lookupInfo.profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E;
    lookupInfo.profileEntryParamsArray[TGF_EXACT_MATCH_PROFILE_ID_CNS].keyParams.keyStart = 1;
    st = prvTgfExactMatchManagerCreate(exactMatchManagerId,&capacityInfo,&lookupInfo,&entryAttrInfo,&agingInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "createExactMatchReducedUdb47Manager: expected to GT_OK on manager [%d]",exactMatchManagerId);

   /* add device to it - will get a fail in sync HW/SW check.
       the fail is due key pattern mismatch */
    st = prvTgfExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfExactMatchManagerDevListAdd: expected NON GT_OK on manager [%d]",exactMatchManagerId);

    /* remove the incorrect manager */
    st = prvTgfExactMatchManagerDelete(exactMatchManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "createExactMatchReducedUdb47Manager: expected to GT_OK on manager [%d]",exactMatchManagerId);

}

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathHaAddBasicConfigSingleEntry function
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
GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaAddBasicConfigSingleEntry
(
    IN GT_U32   exactMatchManagerId,
    IN GT_BOOL  firstCall
)
{
    if (firstCall==GT_FALSE)
    {
        prvTgfExactMatchManagerTtiPclFullPathHaReplayInvalidManager(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E);
    }

    /* Create manager with device */
    prvTgfExactMatchManagerTtiPclFullPathReducedUdb47ManagerCreate(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E);

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(firstCall);

    /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Exact Match Manager Expanded Action PCL configuration with keySize=47B */
    prvTgfExactMatchManagerTtiPclFullPathReducedUdb47ConfigSet(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E);

    /* configure exact match entries extra checks  */
    prvTgfExactMatchManagerTtiPclFullPathHaReplayInvalidEntry(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E,firstCall);
}

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathHaValidityCheckAndTrafficSendSingleEntry function
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
GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaValidityCheckAndTrafficSendSingleEntry
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
* @internal prvTgfExactMatchManagerTtiPclFullPathHaSystemRecoveryStateSet function
* @endinternal
*
* @brief  Keep Exact Match Manager DB values for reconstruct and
*         set flag for HA process.
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaSystemRecoveryStateSet()
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
* @internal prvTgfExactMatchManagerTtiPclFullPathHaDelBasicConfigSingleEntry function
* @endinternal
*
* @brief  Delete all Exact Match configuration done
*
* @param[in] exactMatchManagerId - manager Id
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaDelBasicConfigSingleEntry
(
    IN GT_U32   exactMatchManagerId
)
{
    GT_STATUS   rc=GT_OK;

    /* delete the single ExactMatch entry configured
       entry and params are already with the correct entry to delete
       since we are in HA the delete is only from SW and not from HW */
    rc= prvTgfExactMatchManagerEntryDelete(exactMatchManagerId,&entry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchManagerEntryDelete: expected to GT_OK on manager [%d]",exactMatchManagerId);

    /* Delete manager and device */
    prvTgfExactMatchManagerTtiPclFullPathManagerDelete(exactMatchManagerId);
}

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathHaSyncSwHwSingleEntry function
* @endinternal
*
* @brief  do any suncronization left between HW and Shadow
*         set complition state
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaSyncSwHwSingleEntry()
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
* @internal prvTgfExactMatchManagerTtiPclFullPathHaRestoreBasicConfigSingleEntry function
* @endinternal
*
* @brief  restore old configuration and delete exact match entry and manager
*
* @param[in] exactMatchManagerId - manager Id
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaRestoreBasicConfigSingleEntry
(
    IN GT_U32   exactMatchManagerId
)
{
    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Delete manager and device */
    prvTgfExactMatchManagerTtiPclFullPathManagerDelete(exactMatchManagerId);
}
