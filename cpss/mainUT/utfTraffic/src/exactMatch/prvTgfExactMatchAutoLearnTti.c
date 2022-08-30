/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *2
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvTgfExactMatchAutoLearnTti.c
*
* @brief Configuration for Exact Match Auto Learning
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfPolicerGen.h>
#include <common/tgfIpfixGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCommon.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <exactMatch/prvTgfExactMatchTtiPclFullPath.h>
#include <exactMatch/prvTgfExactMatchAutoLearnTti.h>
#include <tunnel/prvTgfTunnelTermEtherType.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfix.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/******************************* Test packet **********************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS              1

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR1_PORT_IDX_CNS       1
#define PRV_TGF_EGR2_PORT_IDX_CNS       2
#define PRV_TGF_EGR3_PORT_IDX_CNS       3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

#define PRV_TGF_CPU_PORT_IDX_CNS        3

/* profileId number to set */
#define PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS  9
#define PRV_TGF_EXACT_MATCH_EXPANDER_ID_CNS 4

#define PRV_TGF_NUM_OF_UDB_CNS              7

/* max number of VFD to use */
#define PRV_TGF_VFD_NUM_CNS                 2

/* value of Base MacSa */
#define PRV_TGF_MAC_SA_BASE_CNS {0x00, 0xA1, 0xB2, 0xC3, 0xD4, 0x00}

/* cpu code to set */
#define PRV_TGF_EXACT_MATCH_CPU_CODE_CNS    CPSS_NET_FIRST_USER_DEFINED_E + 3

/* default tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS       1

/* default number of packets to send */
static GT_U32       prvTgfBurstCount = 1;

/* PCL ID */
#define PRV_TGF_PCL_ID_CNS                   0x66

extern CPSS_PACKET_CMD_ENT  pktCmdUsedForExactMatchEntry;
extern GT_BOOL              reducedUsedForExactMatchEntry;

static GT_BOOL              firstCall = GT_TRUE;
static GT_U32               savedFlowIdToCheckIpfixCounters;
static GT_U32               savedBaseFlowId=0;
GT_U32                      numberOfPacketsLearned=0;

/* parameters that is needed to be restored */
static struct
{
    GT_U32                              sendPort;
} prvTgfRestoreCfg;

static struct
{
    GT_BOOL                                                 profileIdEn;
    GT_U32                                                  profileId;
    PRV_TGF_EXACT_MATCH_CLIENT_ENT                          firstLookupClientType;
    PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC              keyParams;

    GT_U32                                                  exactMatchEntryIndexArr[16];
    GT_U32                                                  numberOfElementsInExactMatchEntryIndexArr;
    GT_BOOL                                                 exactMatchEntryValid;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT                     exactMatchEntryActionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                          exactMatchEntryAction;
    PRV_TGF_EXACT_MATCH_ENTRY_STC                           exactMatchEntry;
    GT_U32                                                  exactMatchEntryExpandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT                        exactMatchAutoLearnLookupNum;
    GT_BOOL                                                 exactMatchAutoLearnDefaultEnable;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC     exactMatchAutoLearnFlowIdConfiguration;
    PRV_TGF_POLICER_STAGE_METER_MODE_ENT                    stageModeSave;
    PRV_TGF_POLICER_COUNTING_MODE_ENT                       mode;
    PRV_TGF_IPFIX_WRAPAROUND_CFG_STC                        wraparounfConfiguration;

    GT_BOOL                                                 exactMatchActivityEnable;
    GT_U32                                                  ipfixBaseFlowIndex;

} prvTgfExactMatchRestoreCfg;

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x44, 0x33, 0x22, 0x11},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,         /* totalLen   */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

static GT_BOOL prvTgfActionTrapToCpuCheck[] = {
    GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE, GT_FALSE, GT_FALSE
};
/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/****************************************************************************************************************************************************/
/****************************************************************************************************************************************************/

/*Reset the port counters */
static GT_STATUS prvTgfCountersEthReset
(
    GT_VOID
)
{
    GT_U32    portIter;
    GT_STATUS rc, rc1 = GT_OK;

    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfResetCountersEth");
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
};

/**
* @internal prvTgfTunnelTermEtherTypeOtherTestInit function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
* @param[in] sendPortNum              - port number to send packet
* @param[in] vrfId                    - assign VRF ID for packet
*                                      GT_TRUE  - assign VRF ID for packet
*                                      GT_FALSE - not assign VRF ID for packet
* @param[in] vrfId                    - VRF ID to set
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Used 7 first bytes of anchor 'L3 offset - 2'.
*
*/
static GT_VOID prvTgfExactMatchAutoLearnTtiGenericConfigInit
(
    IN  GT_U32                        sendPortNum,
    IN  GT_BOOL                       vrfIdAssign,
    IN  GT_U32                        vrfId
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC    ttiAction ;
    PRV_TGF_TTI_RULE_UNT        ttiPattern;
    PRV_TGF_TTI_RULE_UNT        ttiMask;
    GT_U32 ii;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_UDB_ETHERNET_OTHER_E */
    rc = prvTgfTtiPortLookupEnableSet(sendPortNum,
                                PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);


    /* set the TTI Rule Pattern, Mask and Action for Ether Type Key */

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet((GT_VOID*) &ttiPattern,0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,   0, sizeof(ttiMask));

    /* set TTI Action */
    if (vrfIdAssign == GT_FALSE)
    {
        ttiAction.command                       = CPSS_PACKET_CMD_DROP_HARD_E;
        ttiAction.userDefinedCpuCode            = CPSS_NET_FIRST_USER_DEFINED_E;
        ttiAction.redirectCommand               = PRV_TGF_TTI_NO_REDIRECT_E;
        ttiAction.tunnelTerminate               = GT_TRUE;
        ttiAction.vrfId                         = 0;
    }
    else
    {
        ttiAction.command                       = CPSS_PACKET_CMD_FORWARD_E;
        ttiAction.redirectCommand               = PRV_TGF_TTI_VRF_ID_ASSIGN_E;
        ttiAction.tunnelTerminate               = GT_FALSE;
        ttiAction.vrfId                         = vrfId;
    }

    if (vrfIdAssign == GT_FALSE)
    {
        ttiPattern.eth.common.vid           = PRV_TGF_VLANID_CNS;
        ttiPattern.eth.common.isTagged      = GT_TRUE;
    }
    else
    {
        /* don't care of VLAN in the packet */
        ttiPattern.eth.common.vid           = 0;
        ttiPattern.eth.common.isTagged      = GT_FALSE;
    }

    for(ii = 0; ii < PRV_TGF_NUM_OF_UDB_CNS ; ii++)
    {
        ttiPattern.udbArray.udb[ii] = prvTgfPayloadDataArr[ii+3];
        ttiMask.udbArray.udb[ii] = 0xff;
    }

    PRV_UTF_LOG0_MAC("======= Setting UDB Configuration =======\n");

    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum,
                            PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                            PRV_TGF_TTI_KEY_SIZE_10_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);
    /* AUTODOC: set TTI rule */

    rc = prvTgfTtiRuleUdbSet(PRV_TGF_TTI_INDEX_CNS,PRV_TGF_TTI_RULE_UDB_10_E,
                             &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleUdbSet");

    for(ii = 0; ii < PRV_TGF_NUM_OF_UDB_CNS ; ii++)
    {
        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                        PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, ii,
                        PRV_TGF_TTI_OFFSET_L3_MINUS_2_E,(GT_U8) ii);
    }

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_FALSE);

}

/**
* @internal prvTgfExactMatchAutoLearnTtiGenericConfigSet function
* @endinternal
*
* @brief   Set TTI generic test configuration, not related to Exact Match
*
* @note Used 7 first bytes of anchor 'L3 offset - 2'.
*
*/
GT_VOID prvTgfExactMatchAutoLearnTtiGenericConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    prvTgfRestoreCfg.sendPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
    {
        /* do not modify the 12 ports in prvTgfPortsArray[] !!! */
    }
    else
    {
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] = 16;
    }
    /* init configuration */
    prvTgfExactMatchAutoLearnTtiGenericConfigInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],GT_TRUE, 0);
}

/**
* @internal prvTgfExactMatchAutoLearnTtiConfigSet function
* @endinternal
*
* @brief Set TTI test configuration related to Exact Match Reduced Action
*
*/
GT_VOID prvTgfExactMatchAutoLearnTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    PRV_TGF_EXACT_MATCH_CLIENT_ENT                  firstLookupClientType;
    PRV_TGF_TTI_KEY_TYPE_ENT                        packetType;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT                  lookupNum;

    GT_BOOL                                         enableExactMatchLookup;
    GT_U32                                          profileId;

    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT             actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                  actionData;

    PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  expandedActionOrigin;
    PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC      keyParams;
    PRV_TGF_EXACT_MATCH_ENTRY_STC                   exactMatchEntry;
    GT_U32                                          exactMatchAutoLearnProfileIndex;/* Exact Match Auto Learn profile index*/
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC   exactMatchAutoLearnFlowIdStatusGet;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                            exactMatchAutoLearnDefaulActionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                       exactMatchAutoLearnActionType;

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &actionData, 0, sizeof(actionData));

    /* take all fields from Expanded Action: all fields overwriteExpande=GT_FALSE*/
    cpssOsMemSet((GT_VOID*) &expandedActionOrigin, 0, sizeof(expandedActionOrigin));

    /* auto Learn configuration */
    exactMatchAutoLearnProfileIndex = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    exactMatchAutoLearnActionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    cpssOsMemSet((GT_VOID*) &exactMatchAutoLearnDefaulActionData, 0, sizeof(exactMatchAutoLearnDefaulActionData));
    exactMatchAutoLearnDefaulActionData.ttiAction.tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;/* default value is not 0 */
    exactMatchAutoLearnDefaulActionData.ttiAction.ttPassengerPacketType=CPSS_DXCH_TTI_PASSENGER_IPV4V6_E;/* default value is not 0 */
    exactMatchAutoLearnDefaulActionData.ttiAction.command            = CPSS_PACKET_CMD_TRAP_TO_CPU_E ;
    exactMatchAutoLearnDefaulActionData.ttiAction.userDefinedCpuCode = PRV_TGF_EXACT_MATCH_CPU_CODE_CNS + 2 ; /*505*/
    exactMatchAutoLearnDefaulActionData.ttiAction.ipfixEn = GT_TRUE;
    exactMatchAutoLearnDefaulActionData.ttiAction.flowId = 2;
    exactMatchAutoLearnDefaulActionData.ttiAction.bindToPolicer=GT_TRUE;

    lookupNum                   = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    firstLookupClientType       = PRV_TGF_EXACT_MATCH_CLIENT_TTI_E;
    packetType                  = PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E;

    enableExactMatchLookup      = GT_TRUE;
    profileId                   = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    actionType                  = PRV_TGF_EXACT_MATCH_ACTION_TTI_E ;

    actionData.ttiAction.command            = PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E ;
    actionData.ttiAction.userDefinedCpuCode = PRV_TGF_EXACT_MATCH_CPU_CODE_CNS;

    keyParams.keySize           =CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
    keyParams.keyStart          =0;
    cpssOsMemSet((GT_VOID*) &keyParams.mask[0], 0, sizeof(keyParams.mask));

    exactMatchEntry.key.keySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_33B_E;
    exactMatchEntry.lookupNum   = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    /* here we check exactly the same pattern set for the TTI match in TCAM - all zero */
    cpssOsMemSet((GT_VOID*) &exactMatchEntry.key.pattern[0], 0, sizeof(exactMatchEntry.key.pattern));

    /* AUTODOC: keep flow id configuration, for restore */
    rc = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&prvTgfExactMatchRestoreCfg.exactMatchAutoLearnFlowIdConfiguration);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: keep clientType configured for first lookup , for restore */
    rc = prvTgfExactMatchClientLookupGet(prvTgfDevNum,lookupNum,&prvTgfExactMatchRestoreCfg.firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: keep profileId configured for first lookup , for restore */
    rc = prvTgfExactMatchTtiProfileIdModePacketTypeGet(prvTgfDevNum, packetType, lookupNum ,
                                                       &prvTgfExactMatchRestoreCfg.profileIdEn,
                                                       &prvTgfExactMatchRestoreCfg.profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchTtiProfileIdModePacketTypeGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: save Profile Key Params , for restore */
    rc =  prvTgfExactMatchProfileKeyParamsGet(prvTgfDevNum,profileId ,
                                           &prvTgfExactMatchRestoreCfg.keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set first lookup client type to be PRV_TGF_EXACT_MATCH_CLIENT_TTI_E */
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,lookupNum, firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set profileId= 4  for TTI first lookup */
    rc = prvTgfExactMatchTtiProfileIdModePacketTypeSet(prvTgfDevNum, packetType, lookupNum ,
                                                       enableExactMatchLookup , profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchTtiProfileIdModePacketTypeSet FAILED: %d", prvTgfDevNum);

    /* if auto learning is enabled; flowId is always taken from reduced action*/
    expandedActionOrigin.ttiExpandedActionOrigin.overwriteExpandedActionFlowId = GT_TRUE;
    expandedActionOrigin.ttiExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode = GT_TRUE;
    actionData.ttiAction.ipfixEn = GT_TRUE;
    actionData.ttiAction.flowId = 5;
    actionData.ttiAction.bindToPolicer=GT_TRUE;

    /* AUTODOC: set Expanded Action for TTI lookup, profileId=9, ActionType=TTI, packet Command = TRAP */
    rc = prvTgfExactMatchExpandedActionSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_EXPANDER_ID_CNS,
                                            actionType,&actionData,&expandedActionOrigin);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchExpandedActionSet FAILED: %d", prvTgfDevNum);


    /* set default action for Auto Learn entry */
    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionSet(prvTgfDevNum,
                                                            exactMatchAutoLearnProfileIndex,
                                                            PRV_TGF_EXACT_MATCH_EXPANDER_ID_CNS/*expandedActionIndex*/,
                                                            exactMatchAutoLearnActionType,
                                                            &exactMatchAutoLearnDefaulActionData,
                                                            GT_TRUE/*actionEn*/ );
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnProfileDefaultActionSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum,profileId, &keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    /* check those values for debugging */
    rc = cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                                  &exactMatchAutoLearnFlowIdStatusGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet FAILED: %d", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, exactMatchAutoLearnFlowIdStatusGet.flowIdRangeIsFull,
                                 "cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet FAILED: flowIdRangeIsFull=%d",
                                 exactMatchAutoLearnFlowIdStatusGet.flowIdRangeIsFull);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, exactMatchAutoLearnFlowIdStatusGet.numAllocated,
                                 "cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet FAILED: numAllocated=%d",
                                 exactMatchAutoLearnFlowIdStatusGet.numAllocated);

    return;
}

/**
* @internal prvTgfExactMatchAutoLearnTtiExactMatchPriorityConfig function
* @endinternal
*
* @brief   Set TCAM Priority over Exact Match
* @param[in] exactMatchOverTtiEn - GT_FALSE: TCAM priority over Exact Match
*                                - GT_TRUE : Exact Match priority over TCAM
*/
GT_VOID prvTgfExactMatchAutoLearnTtiExactMatchPriorityConfig
(
    GT_BOOL exactMatchOverTtiEn
)
{
    GT_STATUS                         rc;
    GT_U32                            ii;
    GT_U32                            ruleIndex;
    PRV_TGF_TTI_RULE_UNT              mask;
    PRV_TGF_TTI_RULE_UNT              pattern;
    PRV_TGF_TTI_ACTION_2_STC          action;
    PRV_TGF_TTI_RULE_TYPE_ENT         ruleType;

    /* TTI Configuration */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleIndex                                    = PRV_TGF_TTI_INDEX_CNS;
    ruleType                                     = PRV_TGF_TTI_RULE_UDB_10_E;
    action.command                               = CPSS_PACKET_CMD_FORWARD_E;
    action.redirectCommand                       = PRV_TGF_TTI_VRF_ID_ASSIGN_E;
    action.tunnelTerminate                       = GT_FALSE;
    action.vrfId                                 = 0;
    action.exactMatchOverTtiEn                   = exactMatchOverTtiEn;


    for(ii = 0; ii < PRV_TGF_NUM_OF_UDB_CNS ; ii++)
    {
        pattern.udbArray.udb[ii] = prvTgfPayloadDataArr[ii+3];
        mask.udbArray.udb[ii] = 0xff;
    }
    rc = prvTgfTtiRuleUdbSet(ruleIndex,ruleType,&pattern, &mask, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleUdbSet FAILED: %d, %d, %d", prvTgfDevNum ,ruleIndex ,ruleType );
}

/**
* @internal prvTgfExactMatchAutoLearnTtiTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectForwardTraffic - GT_FALSE: expect no traffic
*                                 - GT_TRUE: expect traffic to be forwarded
* @param[in] expectTrapTraffic    - GT_FALSE: expect no traffic
*                                 - GT_TRUE: expect traffic to be trapped
* @param[in] expectCPUcode        - if expectForwardTraffic set to GT_FALSE
*                                   Compar expectCPUcode to what we got
*/
GT_VOID prvTgfExactMatchAutoLearnTtiTrafficGenerate
(
    GT_BOOL     expectForwardTraffic,
    GT_BOOL     expectTrapTraffic,
    GT_U32      expectCPUcode
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_U32                          ipfixIndex;
    CPSS_DXCH_IPFIX_ENTRY_STC       ipfixEntry;
    GT_U32                          autoLearnOldestFlowId=0;
    GT_U32                          autoLearnOldestExactMatchIndex;


    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic prvTgfTunnelTermEtherTypeTrafficGenerateExpectTraffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    /* Reset the port Counter */
    rc = prvTgfCountersEthReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCountersEthReset");

    /* setup portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;


    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    PRV_UTF_LOG0_MAC("======= set capture on all ports =======\n");
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS];
    /* enable capture on port 1 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];
    /* enable capture on port 2 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS];
    /* enable capture on port 3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=00:00:44:33:22:11, SA=00:00:00:00:00:33, VID=1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* after the entry is learned we should add the proper IPFIX entry */

    /* get the flowId learned */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          &autoLearnOldestFlowId,
                                                          &autoLearnOldestExactMatchIndex);
    if (rc!=GT_OK && rc!=GT_EMPTY)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet");
    }

    /* ======= disable capture on all ports ======= */
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS];
    /* disable capture on port 3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];
    /* disable capture on port 2 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS];
    /* disable capture on port 1 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get no traffic - dropped */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx and Rx counters */
        switch (portIter) {
            case PRV_TGF_SEND_PORT_IDX_CNS:

                /* packetSize is not changed */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;

                break;

        default:
             if (expectForwardTraffic == GT_TRUE)
             {
                /* port has both Rx and Tx counters because of capture */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
             }
             else
             {
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]    = 0;
                expectedCntrs.ucPktsSent.l[0]      = 0;
                expectedCntrs.brdcPktsSent.l[0]    = 0;
                expectedCntrs.mcPktsSent.l[0]      = 0;
                expectedCntrs.goodOctetsRcv.l[0]   = 0;
                expectedCntrs.goodPktsRcv.l[0]     = 0;
                expectedCntrs.ucPktsRcv.l[0]       = 0;
                expectedCntrs.brdcPktsRcv.l[0]     = 0;
                expectedCntrs.mcPktsRcv.l[0]       = 0;
             }
        }

        isOk =
            portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
            portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
            portCntrs.ucPktsSent.l[0]     == expectedCntrs.ucPktsSent.l[0] &&
            portCntrs.brdcPktsSent.l[0]   == expectedCntrs.brdcPktsSent.l[0] &&
            portCntrs.mcPktsSent.l[0]     == expectedCntrs.mcPktsSent.l[0] &&
            portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0] &&
            portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
            portCntrs.ucPktsRcv.l[0]      == expectedCntrs.ucPktsRcv.l[0] &&
            portCntrs.brdcPktsRcv.l[0]    == expectedCntrs.brdcPktsRcv.l[0] &&
            portCntrs.mcPktsRcv.l[0]      == expectedCntrs.mcPktsRcv.l[0];

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

        /* print expected values if bug */
        if (isOk != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");

            PRV_UTF_LOG0_MAC("Current values:\n");
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", portCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", portCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", portCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", portCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", portCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", portCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", portCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", portCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", portCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", portCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    /* get trigger counters */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* ======= check on all ports ======= */
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    if (expectForwardTraffic==GT_TRUE)
    {
        /* check if there are captured packets at all */
        UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers, "the test expected traffic to be forwarded\n");
    }
    else
    {
        /* check if no captured packets at all */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, numTriggers, "the test expected traffic NOT to be forwarded\n");

        if (GT_TRUE == prvTgfActionTrapToCpuCheck[PRV_TGF_CPU_PORT_IDX_CNS])
        {
            GT_U8                           packetBufPtr[64] = {0};
            GT_U32                          packetBufLen = 64;
            GT_U32                          packetLen;
            GT_U8                           devNum;
            GT_U8                           queueCpu;
            TGF_NET_DSA_STC                 rxParams;

            PRV_UTF_LOG0_MAC("CPU port capturing:\n");

            cpssOsBzero((GT_VOID*)&rxParams, sizeof(TGF_NET_DSA_STC));
            rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                               GT_TRUE, GT_TRUE,
                                               packetBufPtr,
                                               &packetBufLen,
                                               &packetLen,
                                               &devNum,
                                               &queueCpu,
                                               &rxParams);

            if(expectTrapTraffic==GT_FALSE)
            {
                /* we want to verify no packet was trapped to CPU
                   the command in the Exact Match Expanded Action was DROP */
                UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
                UTF_VERIFY_EQUAL0_PARAM_MAC(expectCPUcode,rxParams.cpuCode);

                PRV_UTF_LOG1_MAC("Source Port :%d\n",rxParams.portNum);
                PRV_UTF_LOG0_MAC("Packet is NOT Trapped to  CPU. Packet is dropped\n\n");
            }
            else
            {
                 /* we want to verify packet was trapped to CPU */
                 UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
                 UTF_VERIFY_EQUAL0_PARAM_MAC(expectCPUcode,rxParams.cpuCode);

                 PRV_UTF_LOG1_MAC("Source Port :%d\n",rxParams.portNum);
                 PRV_UTF_LOG1_MAC("Packet is Trapped to  CPU with CPU Code  :%d\n\n",rxParams.cpuCode);
            }
        }
    }

    ipfixIndex = autoLearnOldestFlowId+savedBaseFlowId;
    rc = cpssDxChIpfixEntryGet(prvTgfDevNum,
                             CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,
                             ipfixIndex, CPSS_DXCH_IPFIX_RESET_MODE_IPFIX_E, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "cpssDxChIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,
                                 ipfixIndex);
    UTF_VERIFY_EQUAL3_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E, ipfixEntry.lastCpuOrDropCode,
                                 "cpssDxChIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,
                                 ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.packetCount,
                                 "packetCount different then expected");


    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");
}

/**
* @internal prvTgfExactMatchBasicExpandedActionGenericConfigRestore function
* @endinternal
*
* @brief   Restore TTI generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfExactMatchAutoLearnTtiGenericConfigRestore
(
    GT_VOID
)
{

    GT_STATUS   rc = GT_OK;
    GT_U32 ii;

    PRV_UTF_LOG0_MAC("======= Restore Configuration =======\n");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

       /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_UDB_ETHERNET_OTHER_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_FALSE);

    /* AUTODOC: restore send port */
    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] = prvTgfRestoreCfg.sendPort;

    /* AUTODOC: restore UDBs */

    for (ii = 0; ii < PRV_TGF_NUM_OF_UDB_CNS; ii++) {
        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                                         PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, ii,
                                         PRV_TGFH_TTI_OFFSET_INVALID_E , 0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    }

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);
}

/**
* @internal prvTgfExactMatchAutoLearnTtiConfigRestore function
* @endinternal
*
* @brief   Restore TTI test configuration related to Exact Match
*          Reduced Action
*/
GT_VOID prvTgfExactMatchAutoLearnTtiConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    PRV_TGF_TTI_KEY_TYPE_ENT                keyType;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT          ttiLookupNum;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT     actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT          zeroAction;
    GT_PORT_GROUPS_BMP                      portGroupsBmp;
    PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  zeroActionOrigin;
    GT_U32                                  autoLearnOldestFlowId = 0;
    GT_U32                                  autoLearnOldestExactMatchIndex;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FAIL_COUNTERS_STC autoLearnFailCounters;
    CPSS_DXCH_IPFIX_ENTRY_STC               ipfixEntry;
    GT_U32                                  autoLearnFlowId;
    GT_U32                                  autoLearnExactMatchIndex;


    cpssOsMemSet(&zeroAction, 0, sizeof(zeroAction));
    cpssOsMemSet(&zeroActionOrigin, 0, sizeof(zeroActionOrigin));

    /* AUTODOC: Restore profileId configured for first lookup
       need to be restored profileId before restore of client type */
    portGroupsBmp   = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    keyType         = PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E;
    ttiLookupNum    = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    actionType      = PRV_TGF_EXACT_MATCH_ACTION_TTI_E;

    /* AUTODOC: restore Exact Match profileId */
    rc= prvTgfExactMatchTtiProfileIdModePacketTypeSet(prvTgfDevNum,keyType,ttiLookupNum,
                                        prvTgfExactMatchRestoreCfg.profileIdEn,prvTgfExactMatchRestoreCfg.profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchTtiProfileIdModePacketTypeSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: Restore first lookup client type */
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,ttiLookupNum, prvTgfExactMatchRestoreCfg.firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: restore Expanded Action for TTI lookup */
    rc = prvTgfExactMatchExpandedActionSet(prvTgfDevNum, PRV_TGF_EXACT_MATCH_EXPANDER_ID_CNS,
                                           actionType, &zeroAction,
                                           &zeroActionOrigin);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchExpandedActionSet FAILED: %d", prvTgfDevNum);


    /* AUTODOC: restore Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS, &prvTgfExactMatchRestoreCfg.keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    /* expect one entry to be learnd - those are the values of the Auto Learn entry */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          &autoLearnFlowId,
                                                          &autoLearnExactMatchIndex);

    if (rc!=GT_OK && rc!=GT_NO_MORE)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext");
    }

    /* AUTODOC: Gets the oldest entry from the current auto-learned
      Exact Match entries and its assosiated Flow ID. */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          &autoLearnOldestFlowId,
                                                          &autoLearnOldestExactMatchIndex);
    if (rc!=GT_OK && rc!=GT_EMPTY)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet");
    }

    /* clear IPFIX entry */
    rc = cpssDxChIpfixEntryGet(prvTgfDevNum,
                               CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,
                               autoLearnOldestFlowId + savedBaseFlowId, /* baseFlowId set in init phase */
                               CPSS_DXCH_IPFIX_RESET_MODE_IPFIX_E,
                               &ipfixEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");


    /* delete the Auto Learn entry in the Exact Match*/
    rc = prvTgfExactMatchPortGroupEntryInvalidate(prvTgfDevNum,portGroupsBmp,autoLearnOldestExactMatchIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

    /* restore stage mode */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_INGRESS_0_E, prvTgfExactMatchRestoreCfg.stageModeSave);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    /* restore mode */
    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum,PRV_TGF_POLICER_STAGE_INGRESS_0_E,prvTgfExactMatchRestoreCfg.mode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d",PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    /* restore wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E,&prvTgfExactMatchRestoreCfg.wraparounfConfiguration);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    /* AUTODOC: return fail counters for debug */
    rc = cpssDxChExactMatchPortGroupAutoLearnFailCountersGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             &autoLearnFailCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnFailCountersGet");
    UTF_VERIFY_EQUAL1_STRING_MAC(0, autoLearnFailCounters.flowIdFailCounter,
                                 "cpssDxChExactMatchPortGroupAutoLearnFailCountersGet FAILED: flowIdFailCounter=%d",
                                 autoLearnFailCounters.flowIdFailCounter);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, autoLearnFailCounters.indexFailCounter,
                                 "cpssDxChExactMatchPortGroupAutoLearnFailCountersGet FAILED: indexFailCounter=%d",
                                 autoLearnFailCounters.indexFailCounter);
    return;
}

/**
* @internal prvTgfExactMatchAutoLearnIpfixtInit function
* @endinternal
*
* @brief  Set test configuration: Enable the Ingress Policy Engine
*/
GT_VOID prvTgfExactMatchAutoLearnIpfixtInit
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          ipfixIndex;
    PRV_TGF_IPFIX_ENTRY_STC         ipfixEntry;
    GT_U32                          autoLearnOldestFlowId=0;
    GT_U32                          autoLearnOldestExactMatchIndex;
    PRV_TGF_IPFIX_WRAPAROUND_CFG_STC wraparounfConf;

    /* Save stage mode */
    rc = prvTgfPolicerStageMeterModeGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E, &prvTgfExactMatchRestoreCfg.stageModeSave);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeGet: %d",PRV_TGF_POLICER_STAGE_INGRESS_0_E);
    /* IPFIX works in Flow mode */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_INGRESS_0_E, PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    /* Save mode */
    rc = prvTgfPolicerCountingModeGet(prvTgfDevNum,PRV_TGF_POLICER_STAGE_INGRESS_0_E,&prvTgfExactMatchRestoreCfg.mode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d",PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum,
                                      PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                      PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                 PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);

    /* save wraparound configuration */
    rc = prvTgfIpfixWraparoundConfGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                      &prvTgfExactMatchRestoreCfg.wraparounfConfiguration);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfGet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    /* save ipfix flowId base configured in the PLR */
    rc = prvTgfIpfixBaseFlowIdGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                  &prvTgfExactMatchRestoreCfg.ipfixBaseFlowIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixBaseFlowIdGet: %d",PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    wraparounfConf.action = PRV_TGF_IPFIX_WRAPAROUND_ACTION_NONE_E;
    wraparounfConf.dropThreshold = 0x3FFFFFFF;
    wraparounfConf.packetThreshold = 0x3FFFFFFF;
    wraparounfConf.byteThreshold.l[0] = 0xFFFFFFFF;
    wraparounfConf.byteThreshold.l[1] = 0xF;

    /* set wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E,&wraparounfConf);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    /* get the oldest flowId learned (or the first free flowId if the table is empty) */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          &autoLearnOldestFlowId,
                                                          &autoLearnOldestExactMatchIndex);
    if (rc!=GT_OK && rc!=GT_EMPTY)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet");
    }

    /* savedBaseFlowId is the baseFlowId set in init phase this is the value of the flowId
       that we be automatically  set in the Exact Match Auto Learn entry */
    ipfixIndex = autoLearnOldestFlowId+savedBaseFlowId;
    cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));
    if(PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
    {
        /* For AC5P, default cpuCode is CPSS_NET_UNDEFINED_CPU_CODE_E and hence need to set
         */
        ipfixEntry.lastCpuOrDropCode  = CPSS_NET_FIRST_USER_DEFINED_E;
    }

    rc = prvTgfIpfixEntrySet(prvTgfDevNum,
                             PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                             ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                 ipfixIndex);

    /* set ipfix flowId base configured for PLR0=0 */
    rc = prvTgfIpfixBaseFlowIdSet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixBaseFlowIdSet: %d",PRV_TGF_POLICER_STAGE_INGRESS_0_E);

}

/**
* @internal prvTgfExactMatchAutoLearnIpfixtRestore function
* @endinternal
*
* [in] numberOfTiggers - number of tiggers to do, each trigger
*                        free one flowId
*
* @brief  Restore test configuration for IPFIX
*/
GT_VOID prvTgfExactMatchAutoLearnIpfixtRestore
(
    GT_U32 numberOfTigger
)
{
    GT_STATUS               rc = GT_OK;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FAIL_COUNTERS_STC autoLearnFailCounters;
    GT_U32                  k=0;

    /* Restore IPFIX stage mode */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_INGRESS_0_E, prvTgfExactMatchRestoreCfg.stageModeSave);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",prvTgfExactMatchRestoreCfg.stageModeSave);

    /* Restore mode */
    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum,PRV_TGF_POLICER_STAGE_INGRESS_0_E,prvTgfExactMatchRestoreCfg.mode);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                 prvTgfExactMatchRestoreCfg.mode);

    /* Restore wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E,&prvTgfExactMatchRestoreCfg.wraparounfConfiguration);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    for( k=0; k<numberOfTigger; k++)
    {
        /* AUTODOC: Release of the oldest FLOW-ID pointer that is currently in use.
           this API should be called after Exact Match Entry is deleted and IPFIX is cleared (cpssDxChIpfixPortGroupEntryGet) */
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        if (rc!=GT_OK && rc!=GT_EMPTY)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");
        }
    }
    /* AUTODOC: return fail counters for debug */
    rc = cpssDxChExactMatchPortGroupAutoLearnFailCountersGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             &autoLearnFailCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnFailCountersGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnFailCountersGet");
    UTF_VERIFY_EQUAL1_STRING_MAC(0, autoLearnFailCounters.flowIdFailCounter,
                                 "cpssDxChExactMatchPortGroupAutoLearnFailCountersGet FAILED: flowIdFailCounter=%d",
                                 autoLearnFailCounters.flowIdFailCounter);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, autoLearnFailCounters.indexFailCounter,
                                 "cpssDxChExactMatchPortGroupAutoLearnFailCountersGet FAILED: indexFailCounter=%d",
                                 autoLearnFailCounters.indexFailCounter);


    /* restore ipfix flowId base configured in the PLR */
    rc = prvTgfIpfixBaseFlowIdSet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                  prvTgfExactMatchRestoreCfg.ipfixBaseFlowIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixBaseFlowIdSet: %d",PRV_TGF_POLICER_STAGE_INGRESS_0_E);
}

/**
* @internal prvTgfExactMatchAutoLearnConfigSet function
* @endinternal
*
* @brief  Set test configuration for Auto Learn
*
* @param[in] flowIdMaxNum        - maximum number of flow-IDs
*
*/
GT_VOID prvTgfExactMatchAutoLearnConfigSet
(
    GT_U32  flowIdMaxNum
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC   exactMatchAutoLearnFlowIdStatusGet;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC         exactMatchAutoLearnFlowIdConfig;
    GT_U32                      exactMatchAutoLearnProfileIndex;/* Exact Match Auto Learn profile index*/

    exactMatchAutoLearnProfileIndex = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;/* set the autoLearn profile id to be the same as the expanded profile index */

    /*save config */
    /* check those values debugging */
    rc = cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                                  &exactMatchAutoLearnFlowIdStatusGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet FAILED: %d", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, exactMatchAutoLearnFlowIdStatusGet.flowIdRangeIsFull,
                                 "cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet FAILED: flowIdRangeIsFull=%d",
                                 exactMatchAutoLearnFlowIdStatusGet.flowIdRangeIsFull);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, exactMatchAutoLearnFlowIdStatusGet.numAllocated,
                                 "cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet FAILED: numAllocated=%d",
                                 exactMatchAutoLearnFlowIdStatusGet.numAllocated);

    /* AUTODOC: keep AutoLearnLookupNum configured, for restore */
    rc = cpssDxChExactMatchAutoLearnLookupGet(prvTgfDevNum,&prvTgfExactMatchRestoreCfg.exactMatchAutoLearnLookupNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnLookupGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: keep flow id configuration, for restore */
    rc = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                            &prvTgfExactMatchRestoreCfg.exactMatchAutoLearnFlowIdConfiguration);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: keep AutoLearn default action enable status, for restore */
    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet(prvTgfDevNum,
                                                                  exactMatchAutoLearnProfileIndex,
                                                                  &prvTgfExactMatchRestoreCfg.exactMatchAutoLearnDefaultEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet FAILED: %d", prvTgfDevNum);


    /*set */

    /* AUTODOC: Set Exact Match Auto Learn Lookup Num */
    rc = cpssDxChExactMatchAutoLearnLookupSet(prvTgfDevNum,CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnLookupSet FAILED: %d", prvTgfDevNum);

    /* Application MUST disable Exact Match Auto Learning per profile ID before changing Flow ID allocation configuration */
    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(prvTgfDevNum,
                                                                  exactMatchAutoLearnProfileIndex,
                                                                  GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet FAILED: %d", prvTgfDevNum);

    /* the IPFIX table is shared between PLRs.
       Here we assume each PLR get the same size,
       so we set the EM base to be in the half of the first PLR0 */
    savedBaseFlowId = (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->policer.countingMemSize[PRV_TGF_POLICER_STAGE_INGRESS_0_E])/2;

    exactMatchAutoLearnFlowIdConfig.baseFlowId  = savedBaseFlowId;
    exactMatchAutoLearnFlowIdConfig.maxNum      = flowIdMaxNum;
    exactMatchAutoLearnFlowIdConfig.threshold   = flowIdMaxNum/2;

    rc = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                            &exactMatchAutoLearnFlowIdConfig);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet FAILED: %d, "
                                            "baseFlowId=%d,maxNum=%d,threshold=%d",
                                 prvTgfDevNum,
                                 exactMatchAutoLearnFlowIdConfig.baseFlowId,
                                 exactMatchAutoLearnFlowIdConfig.maxNum,
                                 exactMatchAutoLearnFlowIdConfig.threshold);

    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(prvTgfDevNum,
                                                                  exactMatchAutoLearnProfileIndex,
                                                                  GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet FAILED: %d", prvTgfDevNum);

    return;
}
/**
* @internal prvTgfExactMatchAutoLearnConfigSet function
* @endinternal
*
* @brief  Set test configuration for Auto Learn
*
* @param[in] flowIdMaxNum        - maximum number of flow-IDs
*
*/
GT_VOID prvTgfExactMatchAutoLearnMaxConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC   exactMatchAutoLearnFlowIdStatusGet;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC         exactMatchAutoLearnFlowIdConfig;
    GT_U32                      exactMatchAutoLearnProfileIndex;/* Exact Match Auto Learn profile index*/
    GT_U32                      flowIdMaxNum=0;

    exactMatchAutoLearnProfileIndex = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;/* set the autoLearn profile id to be the same as the expanded profile index */

    /*save config */
    /* check those values debugging */
    rc = cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                                  &exactMatchAutoLearnFlowIdStatusGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet FAILED: %d", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, exactMatchAutoLearnFlowIdStatusGet.flowIdRangeIsFull,
                                 "cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet FAILED: flowIdRangeIsFull=%d",
                                 exactMatchAutoLearnFlowIdStatusGet.flowIdRangeIsFull);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, exactMatchAutoLearnFlowIdStatusGet.numAllocated,
                                 "cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet FAILED: numAllocated=%d",
                                 exactMatchAutoLearnFlowIdStatusGet.numAllocated);

    /* AUTODOC: keep AutoLearnLookupNum configured, for restore */
    rc = cpssDxChExactMatchAutoLearnLookupGet(prvTgfDevNum,&prvTgfExactMatchRestoreCfg.exactMatchAutoLearnLookupNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnLookupGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: keep flow id configuration, for restore */
    rc = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                            &prvTgfExactMatchRestoreCfg.exactMatchAutoLearnFlowIdConfiguration);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: keep AutoLearn default action enable status, for restore */
    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet(prvTgfDevNum,
                                                                  exactMatchAutoLearnProfileIndex,
                                                                  &prvTgfExactMatchRestoreCfg.exactMatchAutoLearnDefaultEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet FAILED: %d", prvTgfDevNum);


    /*set */

    /* AUTODOC: Set Exact Match Auto Learn Lookup Num */
    rc = cpssDxChExactMatchAutoLearnLookupSet(prvTgfDevNum,CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnLookupSet FAILED: %d", prvTgfDevNum);

    /* Application MUST disable Exact Match Auto Learning per profile ID before changing Flow ID allocation configuration */
    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(prvTgfDevNum,
                                                                  exactMatchAutoLearnProfileIndex,
                                                                  GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet FAILED: %d", prvTgfDevNum);


    /* the IPFIX table is shared between PLRs.
       Here we assume each PLR get the same size,
       so we set the EM base to be in the half of the first PLR0 */
    savedBaseFlowId = 0;
    flowIdMaxNum = PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.plrInfo.countingEntriesNum-1;

    exactMatchAutoLearnFlowIdConfig.baseFlowId  = savedBaseFlowId;
    exactMatchAutoLearnFlowIdConfig.maxNum      = flowIdMaxNum;
    exactMatchAutoLearnFlowIdConfig.threshold   = flowIdMaxNum/2;

    rc = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                            &exactMatchAutoLearnFlowIdConfig);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet FAILED: %d, "
                                            "baseFlowId=%d,maxNum=%d,threshold=%d",
                                 prvTgfDevNum,
                                 exactMatchAutoLearnFlowIdConfig.baseFlowId,
                                 exactMatchAutoLearnFlowIdConfig.maxNum,
                                 exactMatchAutoLearnFlowIdConfig.threshold);

    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(prvTgfDevNum,
                                                                  exactMatchAutoLearnProfileIndex,
                                                                  GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet FAILED: %d", prvTgfDevNum);

    return;
}
/**
* @internal prvTgfExactMatchAutoLearnConfigRestore function
* @endinternal
*
* @brief  Restore test configuration for Auto Learn
*/
GT_VOID prvTgfExactMatchAutoLearnConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                                                   rc = GT_OK;
    GT_U32                                                      exactMatchAutoLearnProfileIndex;/* Exact Match Auto Learn profile index*/


    exactMatchAutoLearnProfileIndex = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;/* set the autoLearn profile id to be the same as the expanded profile index */
    /* AUTODOC: Restore Exact Match Auto Learn Lookup Num */
    rc = cpssDxChExactMatchAutoLearnLookupSet(prvTgfDevNum,prvTgfExactMatchRestoreCfg.exactMatchAutoLearnLookupNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnLookupSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: Restore flow id configuration */
    /* Application MUST disable Exact Match Auto Learning per profile ID before changing Flow ID allocation configuration */
    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(prvTgfDevNum,
                                                                  exactMatchAutoLearnProfileIndex,
                                                                  GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet FAILED: %d", prvTgfDevNum);

    rc = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                            &prvTgfExactMatchRestoreCfg.exactMatchAutoLearnFlowIdConfiguration);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: Restore AutoLearn default action enable status*/
    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(prvTgfDevNum,exactMatchAutoLearnProfileIndex,
                                                                  prvTgfExactMatchRestoreCfg.exactMatchAutoLearnDefaultEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet FAILED: %d", prvTgfDevNum);

    /* reset value */
    firstCall=GT_TRUE;
    return;
}

/**
* @internal GT_VOID prvTgfExactMatchAutoLearnTtiPclFullPathReducedConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Expanded Action
*          and Reduced Action
*
* @param[in] pktCmd   - packet command to assign in Exact Match Action
* @param[in] keySize  - keySize for profile configuration
*
*/
GT_VOID prvTgfExactMatchAutoLearnTtiPclFullPathReducedConfigSet
(
     CPSS_PACKET_CMD_ENT                pktCmd,
     PRV_TGF_EXACT_MATCH_KEY_SIZE_ENT   keySize
)
{
    GT_STATUS                                       rc=GT_OK;

    PRV_TGF_EXACT_MATCH_CLIENT_ENT                  firstLookupClientType;
    CPSS_PCL_DIRECTION_ENT                          direction;
    PRV_TGF_PCL_PACKET_TYPE_ENT                     pclPacketType;
    GT_U32                                          subProfileId;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT                  pclLookupNum;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT                  exactMatchLookupNum;

    GT_BOOL                                         enableExactMatchLookup;
    GT_U32                                          profileId;

    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT             actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                  actionData;

    PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  expandedActionOrigin;
    PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC      keyParams;

    PRV_TGF_EXACT_MATCH_ENTRY_STC                   exactMatchEntry;

    PRV_TGF_PCL_RULE_FORMAT_UNT                     mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT                     pattern;

    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                exactMatchAutoLearnDefaulActionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT           exactMatchAutoLearnActionType;
    GT_U32                                          exactMatchAutoLearnProfileIndex;/* Exact Match Auto Learn profile index*/
    GT_HW_DEV_NUM                                   hwDevNumVal;

    /* AUTODOC: keep exactMatchActivityEnable configured - for restore */
    rc =  prvTgfExactMatchActivityBitEnableGet(prvTgfDevNum,&prvTgfExactMatchRestoreCfg.exactMatchActivityEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchActivityBitEnableGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set exactMatchActivityEnable */
    rc =  prvTgfExactMatchActivityBitEnableSet(prvTgfDevNum,GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchActivityBitEnableSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: keep clientType configured for first lookup , for restore */
    exactMatchLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    rc = prvTgfExactMatchClientLookupGet(prvTgfDevNum,exactMatchLookupNum,&prvTgfExactMatchRestoreCfg.firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set first lookup client type to be PRV_TGF_EXACT_MATCH_CLIENT_IPCL_0_E */
    firstLookupClientType = PRV_TGF_EXACT_MATCH_CLIENT_IPCL_0_E;
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,exactMatchLookupNum, firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: keep profileId configured for first lookup , for restore */
    pclLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    direction = CPSS_PCL_DIRECTION_INGRESS_E;
    pclPacketType = PRV_TGF_PCL_PACKET_TYPE_UDE6_E;
    subProfileId = 0;/* by default if not configured this field is 0 */
    rc = prvTgfExactMatchPclProfileIdGet(prvTgfDevNum, direction, pclPacketType, subProfileId, pclLookupNum,
                                         &prvTgfExactMatchRestoreCfg.profileIdEn , &prvTgfExactMatchRestoreCfg.profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPclProfileIdGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set profileId=9 for PCL first lookup */
    enableExactMatchLookup = GT_TRUE;
    profileId = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    rc = prvTgfExactMatchPclProfileIdSet(prvTgfDevNum, direction, pclPacketType, subProfileId,
                                         pclLookupNum , enableExactMatchLookup , profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPclProfileIdSet FAILED: %d", prvTgfDevNum);

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &actionData, 0, sizeof(actionData));

    /* take all fields from Expanded Action: all fields overwriteExpande=GT_FALSE*/
    cpssOsMemSet((GT_VOID*) &expandedActionOrigin, 0, sizeof(expandedActionOrigin));

    switch(pktCmd)
    {
    case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
        actionData.pclAction.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        /* For SIP6 and above for the egress control pipe the CPU code is also 'drop code'
           so relevant for drops too. */
        actionData.pclAction.mirror.cpuCode= PRV_TGF_EXACT_MATCH_CPU_CODE_CNS;
        pktCmdUsedForExactMatchEntry = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        reducedUsedForExactMatchEntry=GT_FALSE;
        break;
    case CPSS_PACKET_CMD_FORWARD_E:
        actionData.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
        actionData.pclAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E+5;
        actionData.pclAction.bypassBridge = GT_TRUE;
        actionData.pclAction.bypassIngressPipe = GT_TRUE;
        actionData.pclAction.redirect.redirectCmd=PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
        actionData.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_PORT_E;
        actionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum=prvTgfDevNum;
        actionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS];

        actionData.pclAction.policer.policerEnable = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;

        pktCmdUsedForExactMatchEntry = CPSS_PACKET_CMD_FORWARD_E;
        reducedUsedForExactMatchEntry=GT_FALSE;

        /* the outInterface should be taken from reduced entry */
        expandedActionOrigin.pclExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;

         /* if auto learning is enabled; flowId is always taken from reduced action*/
        expandedActionOrigin.pclExpandedActionOrigin.overwriteExpandedActionFlowId = GT_TRUE;
        expandedActionOrigin.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode = GT_TRUE;
        actionData.pclAction.ipfixEnable = GT_TRUE;
        actionData.pclAction.flowId = 5;

        break;
    default:
        rc=GT_NOT_SUPPORTED;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchTtiPclFullPathConfigSet FAILED pktCmd not supported in the test : %d", pktCmd);
        break;
    }

    actionData.pclAction.egressPolicy = GT_FALSE;
    actionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;

    /* AUTODOC: set Expanded Action for PCL lookup, profileId=4, ActionType=PCL, packet Command = DROP */
    rc = prvTgfExactMatchExpandedActionSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_EXPANDER_ID_CNS,
                                            actionType,&actionData,&expandedActionOrigin);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchExpandedActionSet FAILED: %d", prvTgfDevNum);

    keyParams.keySize=keySize;
    keyParams.keyStart=0;

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

    cpssOsMemCpy((GT_VOID*) &keyParams.mask[0], mask.ruleIngrUdbOnly.udb, sizeof(keyParams.mask));
    /* AUTODOC: save Profile Key Params , for restore */
    rc =  prvTgfExactMatchProfileKeyParamsGet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS ,
                                           &prvTgfExactMatchRestoreCfg.keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS, &keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: calculate index */

    exactMatchEntry.key.keySize=keySize;

    /* here we check exactly the same pattern set for the PCL match in TCAM */

    cpssOsMemCpy((GT_VOID*) &exactMatchEntry.key.pattern[0], pattern.ruleIngrUdbOnly.udb, sizeof(exactMatchEntry.key.pattern));
    exactMatchEntry.lookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;

    actionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;

    /* AUTODOC: Auto Learn configuration */
    exactMatchAutoLearnProfileIndex = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;/* set the autoLearn profile id to be the same as the expanded profile index */
    exactMatchAutoLearnActionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;
    cpssOsMemSet((GT_VOID*) &exactMatchAutoLearnDefaulActionData, 0, sizeof(exactMatchAutoLearnDefaulActionData));

    exactMatchAutoLearnDefaulActionData.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    exactMatchAutoLearnDefaulActionData.pclAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E+5;
    exactMatchAutoLearnDefaulActionData.pclAction.bypassBridge = GT_TRUE;
    exactMatchAutoLearnDefaulActionData.pclAction.bypassIngressPipe = GT_TRUE;
    exactMatchAutoLearnDefaulActionData.pclAction.redirect.redirectCmd=PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    exactMatchAutoLearnDefaulActionData.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_PORT_E;
    exactMatchAutoLearnDefaulActionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum=prvTgfDevNum;
    exactMatchAutoLearnDefaulActionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS];
    exactMatchAutoLearnDefaulActionData.pclAction.ipfixEnable = GT_TRUE;
    exactMatchAutoLearnDefaulActionData.pclAction.flowId = 2;
    pktCmdUsedForExactMatchEntry = CPSS_PACKET_CMD_FORWARD_E;
    reducedUsedForExactMatchEntry=GT_FALSE;

    /* AUTODOC: set Exact Match Entry - Reduced entry set forward port to be 2 */
    if(pktCmd==CPSS_PACKET_CMD_FORWARD_E)
    {
        reducedUsedForExactMatchEntry=GT_TRUE;
        exactMatchAutoLearnDefaulActionData.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_PORT_E;
        rc = prvUtfHwDeviceNumberGet((GT_U8)prvTgfDevNum,&hwDevNumVal);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet FAILED: %d", prvTgfDevNum);
        exactMatchAutoLearnDefaulActionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum=hwDevNumVal;
        exactMatchAutoLearnDefaulActionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];
        exactMatchAutoLearnDefaulActionData.pclAction.policer.policerEnable = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
    }

    /* set default action for Auto Learn entry */
    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionSet(prvTgfDevNum,
                                                            exactMatchAutoLearnProfileIndex,
                                                            PRV_TGF_EXACT_MATCH_EXPANDER_ID_CNS/*expandedActionIndex*/,
                                                            exactMatchAutoLearnActionType,
                                                            &exactMatchAutoLearnDefaulActionData,
                                                            GT_TRUE/*actionEn*/ );
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnProfileDefaultActionSet FAILED: %d", prvTgfDevNum);

    return;
}

/**
* @internal GT_VOID prvTgfExactMatchAutoLearnTtiPclFullPathConfigRestore function
* @endinternal
*
* @brief   Restore PCL test configuration related to Exact Match Expanded Action
*/
GT_VOID prvTgfExactMatchAutoLearnTtiPclFullPathConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                           rc=GT_OK;

    CPSS_PCL_DIRECTION_ENT              direction;
    PRV_TGF_PCL_PACKET_TYPE_ENT         pclPacketType;
    GT_U32                              subProfileId;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT      pclLookupNum;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT      exactMatchLookupNum;
    PRV_TGF_EXACT_MATCH_ACTION_UNT      zeroAction;

    cpssOsMemSet(&zeroAction, 0, sizeof(zeroAction));

    /* AUTODOC: Restore exactMatchActivityEnable */
    rc =  prvTgfExactMatchActivityBitEnableSet(prvTgfDevNum,prvTgfExactMatchRestoreCfg.exactMatchActivityEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchActivityBitEnableSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: Restore profileId configured for first lookup
       need to be restored profileId before restore of client type */
    pclLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    direction = CPSS_PCL_DIRECTION_INGRESS_E;
    pclPacketType = PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E;
    subProfileId = 0;/* by default if not configured this field is 0 */
    rc = prvTgfExactMatchPclProfileIdSet(prvTgfDevNum, direction, pclPacketType, subProfileId, pclLookupNum,
                                         prvTgfExactMatchRestoreCfg.profileIdEn , prvTgfExactMatchRestoreCfg.profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPclProfileIdSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: Restore first lookup client type */
    exactMatchLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,exactMatchLookupNum, prvTgfExactMatchRestoreCfg.firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: restore Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS, &prvTgfExactMatchRestoreCfg.keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    return;
}

/**
* @internal prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficSend function
*
* @endinternal
*
* @brief
*      Sent traffic and check IPFIX counters
*
* @param[in] expectedIpfixPacketCounterValue - the expected
*       value of the ipfix packet counter
*/
GT_VOID prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficSend
(
    GT_U32 expectedIpfixPacketCounterValue
)
{
    GT_STATUS                       rc=GT_OK;
    GT_U32                          ipfixIndex;
    CPSS_DXCH_IPFIX_ENTRY_STC       ipfixEntry;
    GT_U32                          autoLearnOldestFlowId=0;
    GT_U32                          autoLearnOldestExactMatchIndex;

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* Generate traffic - check no traffic, no match in TCAM,
       get match on Exact Match Auto Learn reduce action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* after the entry is learned we should add the proper IPFIX entry */
    /* get the flowId learned */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          &autoLearnOldestFlowId,
                                                          &autoLearnOldestExactMatchIndex);
    if (rc!=GT_OK && rc!=GT_EMPTY)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet");
    }

    /* savedBaseFlowId is the baseFlowId set in init phase this is the value of the flowId
       that we be automatically  set in the Exact Match Auto Learn entry */
    ipfixIndex = autoLearnOldestFlowId+savedBaseFlowId;
    rc = cpssDxChIpfixEntryGet(prvTgfDevNum,
                               CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,
                               ipfixIndex,
                               CPSS_DXCH_IPFIX_RESET_MODE_IPFIX_E,
                               &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                 ipfixIndex);
    UTF_VERIFY_EQUAL3_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E, ipfixEntry.lastCpuOrDropCode,
                                 "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                 ipfixIndex);

    /* check IPFIX counters */
    if (expectedIpfixPacketCounterValue==0)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[0],"btyeCount.l[0] different then expected");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(88, ipfixEntry.byteCount.l[0],"btyeCount.l[0] different then expected");
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedIpfixPacketCounterValue, ipfixEntry.packetCount,
                                 "packetCount different then expected");
}

/**
* @internal prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend function
*
* @endinternal
*
* @brief
*      Sent traffic and check IPFIX counters
*
* @param[in] expectedIpfixPacketCounterValue - the expected
*       value of the ipfix packet counter
* @param[in] numberOfPacketsToSend - number of packets to send
*/
GT_VOID prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend
(
    GT_U32  expectedIpfixPacketCounterValue,
    GT_U32  numberOfPacketsToSend
)
{
    GT_STATUS                       rc=GT_OK;
    GT_U32                          ipfixIndex;
    CPSS_DXCH_IPFIX_ENTRY_STC       ipfixEntry;
    GT_U32                          autoLearnOldestFlowId;
    GT_U32                          autoLearnOldestExactMatchIndex;
    GT_U32                          exactMatchAutoLearnProfileIndex;/* Exact Match Auto Learn profile index*/

    exactMatchAutoLearnProfileIndex = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;/* set the autoLearn profile id to be the same as the expanded profile index */

    /* enable Exact Match Auto Learning per profile ID */
    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(prvTgfDevNum,
                                                                  exactMatchAutoLearnProfileIndex,
                                                                  GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet FAILED: %d", prvTgfDevNum);

    /* return to test default value */
    pktCmdUsedForExactMatchEntry=CPSS_PACKET_CMD_FORWARD_E;

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* Generate traffic - check no traffic, no match in TCAM,
       get match on Exact Match Auto Learn reduce action */
    prvTgfExactMatchTtiPclFullPathMultipleTrafficGenerate(GT_FALSE,GT_TRUE,numberOfPacketsToSend);

    /* after the entry is learned we should add the proper IPFIX entry */
    /* get the flowId learned */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          &autoLearnOldestFlowId,
                                                          &autoLearnOldestExactMatchIndex);
    if (rc!=GT_OK && rc!=GT_EMPTY)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet");
    }
    if (firstCall == GT_TRUE)
    {
        /* this is the flow id we will check ipfix counters on,
           this is the learned place of packet with MAC 0x00, 0x00, 0x44, 0x33, 0x22, 0x11 */
        savedFlowIdToCheckIpfixCounters = autoLearnOldestFlowId;
        firstCall=GT_FALSE;
    }
    /* savedBaseFlowId is the baseFlowId set in init phase this is the value of the flowId
       that we be automatically  set in the Exact Match Auto Learn entry */
    ipfixIndex = savedFlowIdToCheckIpfixCounters+savedBaseFlowId;
    rc = cpssDxChIpfixEntryGet(prvTgfDevNum,
                           CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,
                           ipfixIndex,
                           CPSS_DXCH_IPFIX_RESET_MODE_IPFIX_E,
                           &ipfixEntry);

    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "cpssDxChIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                 ipfixIndex);
    UTF_VERIFY_EQUAL3_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E, ipfixEntry.lastCpuOrDropCode,
                                 "cpssDxChIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                 ipfixIndex);

    /* check IPFIX counters */
    if (expectedIpfixPacketCounterValue==0)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[0],"btyeCount.l[0] different then expected");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(88, ipfixEntry.byteCount.l[0],"btyeCount.l[0] different then expected");
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedIpfixPacketCounterValue, ipfixEntry.packetCount,
                                 "packetCount different then expected");
}

/**
* @internal prvTgfExactMatchAutoLearnTtiPclFullPathInvalidateEmEntry function
* @endinternal
*
* @brief   Invalidate the Auto Learned Exact Match Entry
*
* @param[in] numberOfEntriesToInvalidate - number of EM entries to delete
*/
GT_VOID prvTgfExactMatchAutoLearnTtiPclFullPathInvalidateEmEntry
(
    GT_U32 numberOfEntriesToInvalidate
)
{
    GT_STATUS           rc=GT_OK;
    GT_PORT_GROUPS_BMP  portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32              autoLearnOldestFlowId=0;
    GT_U32              autoLearnOldestExactMatchIndex;
    GT_U32              exactMatchAutoLearnProfileIndex = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    GT_U32              k=0;
    PRV_TGF_IPFIX_ENTRY_STC    ipfixEntry;
    GT_U32                     ipfixIndex,freeFlowIdForNextRun=0;


    if(numberOfEntriesToInvalidate==0xFFFFFFFF)
    {
        numberOfEntriesToInvalidate = numberOfPacketsLearned;
    }
    for (k=0;k<numberOfEntriesToInvalidate;k++)
    {

        /* AUTODOC: Gets the oldest entry from the current auto-learned
          Exact Match entries and its assosiated Flow ID. */
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                              &autoLearnOldestFlowId,
                                                              &autoLearnOldestExactMatchIndex);
        if (rc!=GT_OK)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext");
        }

        /* delete the Auto Learn entry in the Exact Match*/
        rc = prvTgfExactMatchPortGroupEntryInvalidate(prvTgfDevNum,portGroupsBmp,autoLearnOldestExactMatchIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

        if ((rc==GT_OK)&&(numberOfPacketsLearned>0))
        {
           numberOfPacketsLearned--;
        }
        if (numberOfEntriesToInvalidate == 1)
        {
            /* autoLearnOldestFlowId will be recycle so the nextFlowId assigned is the
               value of the autoLearnOldestFlowId plus 1 this is needed for the test that
               send only one packet to prevent from it to fail if running multiple times
               - since the fisrt free flowId is always moving in a cyclic way*/
            freeFlowIdForNextRun = autoLearnOldestFlowId+1;
        }
        /* AUTODOC: Release of the oldest FLOW-ID pointer that is currently in use.
           this API should be called after Exact Match Entry is deleted and IPFIX is cleared (cpssDxChIpfixPortGroupEntryGet) */
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        if (rc!=GT_OK && rc!=GT_EMPTY)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");
        }
    }

    if (numberOfEntriesToInvalidate==1)
    {
        /* set the ipfixEntry for next autoLearn */

        /* savedBaseFlowId is the baseFlowId set in init phase this is the value of the flowId
           that will be automatically  set in the Exact Match Auto Learn entry */
        ipfixIndex = freeFlowIdForNextRun+savedBaseFlowId;
        cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
        {
            /* For AC5P, default cpuCode is CPSS_NET_UNDEFINED_CPU_CODE_E and hence need to set
             */
            ipfixEntry.lastCpuOrDropCode  = CPSS_NET_FIRST_USER_DEFINED_E;
        }

        rc = prvTgfIpfixEntrySet(prvTgfDevNum,
                                 PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                 ipfixIndex, &ipfixEntry);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                     prvTgfDevNum,
                                     PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                     ipfixIndex);
    }

    /* return to test default value */
    pktCmdUsedForExactMatchEntry=CPSS_PACKET_CMD_DROP_HARD_E;

    /* disable Exact Match Auto Learning per profile ID before changing Flow ID allocation configuration */
    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(prvTgfDevNum,
                                                                  exactMatchAutoLearnProfileIndex,
                                                                  GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet FAILED: %d", prvTgfDevNum);
}

/**
* @internal tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios function
* @endinternal
*
* @param[in] maxNumber - max number of EM AutoLearn entries in DB
*
*/
GT_VOID tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios
(
    GT_U32 maxNumber
)
{
    GT_STATUS           rc=GT_OK;
    GT_U32              k,portGroupId;
    GT_U32              regAddr;
    GT_U32              firstFlowId,nextFlowId;
    GT_U32              flowIdIndex[10],exactMatchIndex[10];
    GT_U32              numAllocated,baseFlowId,autoLearnOldestExactMatchIndex;
    GT_U32              exactMatchAutoLearnProfileIndex = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;

    portGroupId = PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.firstActivePortGroup;

    /* test senario:
       0. get first free flowId; this will be baseFlowId.
       1. learn max entries; firstFlowId == nextFlowId == (baseFlowId % maxNumber).
       2. getNext 3 times; flowIdPointer == ((baseFlowId+2) % maxNumber).
       3. recycle first entry; firstFlowId == ((baseFlowId+1) % maxNumber).
       4. getNext should return flowIdPointer == ((baseFlowId+3) % maxNumber).
       5. getNext + recycle all remaining entries
       6. getNext should return GT_NO_MORE.
       7. learn max entries; firstFlowId == nextFlowId == flowIdPointer == (baseFlowId % maxNumber).
       8. getNext + recycle 5 times; firstFlowId == flowIdPointer == ((baseFlowId+5) % maxNumber), nextFlowId == (baseFlowId % maxNumber).
       9. getNext 4 times should return flowIdPointer == (baseFlowId % maxNumber).
       10. recycle all remaining entries.
       11. getNext should return GT_NO_MORE. */

    /* get the oldest flowId learned (or the first free flowId if the table is empty) */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             &baseFlowId,
                                                             &autoLearnOldestExactMatchIndex);
    if (rc!=GT_OK && rc!=GT_EMPTY)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet");
    }

    /* STEP 1: Generate max packets traffic - expect Exact Match hit on an autoLearn Entry
               traffic forwarded to port 2 from auto learn reduced action - auto learn is enabled */
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend(1,maxNumber);

    /* STEP 2: GetNext 3 times */
    for (k=0;k<3;k++)
    {
       /* Gets the oldest entry from the current auto-learned
          Exact Match entries and its assosiated Flow ID. */
       rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             &(flowIdIndex[k]),
                                                             &(exactMatchIndex[k]));
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);
    }

    /* STEP 3: Delete Auto Learn entry from the Exact Match table */
    rc = cpssDxChExactMatchPortGroupEntryInvalidate(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,exactMatchIndex[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

    /* Recycle Entry */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
    if (rc!=GT_OK && rc!=GT_EMPTY)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");
    }

    /* STEP 4: Gets the oldest entry from the current auto-learned
           Exact Match entries and its assosiated Flow ID. */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          &(flowIdIndex[3]),
                                                          &(exactMatchIndex[3]));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);

    /* Read the value of the 'oldest' Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus2;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&firstFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of the next Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus3;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&nextFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of allocated FlowIDs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus1;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,1,17,&numAllocated);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);


    /* AUTODEOC: STEP 4: check pointers values */
    if (!((firstFlowId == ((baseFlowId+1) % maxNumber)) && (nextFlowId == ((baseFlowId+0) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].flowIdPointer == ((baseFlowId+3) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].isFirstPointer == GT_FALSE) && numAllocated == 9))
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FAIL, rc, "tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios FAILED: %d", prvTgfDevNum);
    }

    /* STEP 5: Delete all DB */
    for (k=1; k<maxNumber; k++)
    {
        /* entries 1..3 were read already */
        if (k>=4)
        {
            /* Gets the oldest entry from the current auto-learned
               Exact Match entries and its assosiated Flow ID. */
           rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                                 &(flowIdIndex[k]),
                                                                 &(exactMatchIndex[k]));
           UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);
        }

        /* Delete Auto Learn entry from the Exact Match table */
        rc = cpssDxChExactMatchPortGroupEntryInvalidate(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,exactMatchIndex[k]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

        /* Recycle entry */
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        if (rc!=GT_OK && rc!=GT_EMPTY)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");
        }
    }

    /* AUTODOC: Read the value of the 'oldest' Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus2;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&firstFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* AUTODOC: Read the value of the next Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus3;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&nextFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of allocated FlowIDs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus1;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,1,17,&numAllocated);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* STEP 5: check pointers values; DB is empty */
    if (!((firstFlowId == ((baseFlowId+0) % maxNumber)) && (nextFlowId == ((baseFlowId+0) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].flowIdPointer == ((baseFlowId+9) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].isFirstPointer == GT_TRUE) &&
        (numAllocated == 0)))
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FAIL, rc, "tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios FAILED: %d", prvTgfDevNum);
    }

    /* STEP 6: Gets the oldest entry from the current auto-learned
               Exact Match entries and its assosiated Flow ID. */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                         &(flowIdIndex[0]),
                                                         &(exactMatchIndex[0]));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);

    /* STEP 7: Generate max packets traffic - expect Exact Match hit on an autoLearn Entry
               traffic forwarded to port 2 from auto learn reduced action - auto learn is enabled */
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend(1,maxNumber);

    /* STEP 8: Delete half of DB */
    for (k=0; k<maxNumber/2; k++)
    {

        /* Gets the oldest entry from the current auto-learned
           Exact Match entries and its assosiated Flow ID. */
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                              &(flowIdIndex[k]),
                                                              &(exactMatchIndex[k]));
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);

        /* Delete Auto Learn entry from the Exact Match table */
        rc = cpssDxChExactMatchPortGroupEntryInvalidate(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,exactMatchIndex[k]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

        /* Recycle entry */
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        if (rc!=GT_OK && rc!=GT_EMPTY)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");
        }
    }

    /* Read the value of the 'oldest' Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus2;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&firstFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of the next Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus3;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&nextFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);

    /* Read the value of allocated FlowIDs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus1;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,1,17,&numAllocated);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* STEP 8: check pointers values */
    if (!((firstFlowId == ((baseFlowId+5) % maxNumber)) && (nextFlowId ==((baseFlowId+0) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].flowIdPointer == ((baseFlowId+4) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].isFirstPointer == GT_TRUE) &&
        (numAllocated == 5)))
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FAIL, rc, "tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios FAILED: %d", prvTgfDevNum);
    }

    /* STEP 9: GetNext 3 times */
    for (k=maxNumber/2; k<maxNumber-1; k++)
    {
        /* Gets the oldest entry from the current auto-learned
           Exact Match entries and its assosiated Flow ID. */
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                              &(flowIdIndex[k]),
                                                              &(exactMatchIndex[k]));
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);
     }

    /* Read the value of the 'oldest' Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus2;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&firstFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of the next Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus3;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&nextFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);

    /* Read the value of allocated FlowIDs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus1;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,1,17,&numAllocated);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* STEP 9: check pointers values */
    if (!((firstFlowId == ((baseFlowId+5) % maxNumber)) && (nextFlowId == ((baseFlowId+0) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].flowIdPointer == ((baseFlowId+8) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].isFirstPointer == GT_FALSE) &&
        (numAllocated == 5)))
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FAIL, rc, "tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios FAILED: %d", prvTgfDevNum);
    }

    /* STEP 10: delete 4 entries  */
    for (k=maxNumber/2; k<maxNumber-1; k++)
    {
        /* Delete Auto Learn entry from the Exact Match table */
        rc = cpssDxChExactMatchPortGroupEntryInvalidate(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,exactMatchIndex[k]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

        /* Recycle entry */
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        if (rc!=GT_OK && rc!=GT_EMPTY)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");
        }
    }

    /* STEP 11: delete last entry  */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          &(flowIdIndex[0]),
                                                          &(exactMatchIndex[0]));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);

    /* Delete Auto Learn entry from the Exact Match table */
    rc = cpssDxChExactMatchPortGroupEntryInvalidate(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,exactMatchIndex[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

    /* Recycle entry */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
    if (rc!=GT_OK && rc!=GT_EMPTY)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");
    }

    /* STEP 12: Gets the oldest entry from the current auto-learned
                Exact Match entries and its assosiated Flow ID. */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          &(flowIdIndex[0]),
                                                          &(exactMatchIndex[0]));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);

    numberOfPacketsLearned = 0;

    /* return to test default value */
    pktCmdUsedForExactMatchEntry=CPSS_PACKET_CMD_DROP_HARD_E;

    /* disable Exact Match Auto Learning per profile ID before changing Flow ID allocation configuration */
    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(prvTgfDevNum,
                                                                  exactMatchAutoLearnProfileIndex,
                                                                  GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet FAILED: %d", prvTgfDevNum);
}

/**
* @internal
*           tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios2
*           function
* @endinternal
*
* @param[in] maxNumber - max number of EM AutoLearn entries in DB
*
*/
GT_VOID tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios2
(
    GT_U32 maxNumber
)
{
    GT_STATUS           rc=GT_OK;
    GT_U32              k,portGroupId;
    GT_U32              regAddr;
    GT_U32              firstFlowId,nextFlowId;
    GT_U32              flowIdIndex[20],exactMatchIndex[20];
    GT_U32              numAllocated,baseFlowId,autoLearnOldestExactMatchIndex;
    GT_U32              exactMatchAutoLearnProfileIndex = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;

    portGroupId = PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.firstActivePortGroup;

    /* test senario:
       0. get first free flowId; this will be baseFlowId.
       1. learn max entries; firstFlowId == nextFlowId == (baseFlowId % maxNumber).
       2. getNext max times; flowIdPointer == ((baseFlowId+19) % maxNumber).
       3. getNext should return GT_NO_MORE.
       4. recycle all entries.
       5. learn max entries; firstFlowId == nextFlowId == (baseFlowId % maxNumber).
       6. recycle 1 entry. firstFlowId == ((baseFlowId+1) % maxNumber).
       7. getNext (max-1) times; flowIdPointer == ((baseFlowId+19) % maxNumber).
       8. getNext should return GT_NO_MORE.
       9. recycle all entries.
       10. learn max/2 entries; flowIdPointer == ((baseFlowId+19) % maxNumber).
       11. getNext (max/2) times; flowIdPointer == ((baseFlowId+9) % maxNumber).
       12. getNext should return GT_NO_MORE.
       13. recycle all entries.
       14. learn max/2 entries; flowIdPointer == ((baseFlowId+9) % maxNumber).
       15. getNext (max/2) times; flowIdPointer == ((baseFlowId+19) % maxNumber).
       16. getNext should return GT_NO_MORE.
       17. recycle all entries.
       */

    /* get the oldest flowId learned (or the first free flowId if the table is empty) */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             &baseFlowId,
                                                             &autoLearnOldestExactMatchIndex);
    if (rc!=GT_OK && rc!=GT_EMPTY)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet");
    }

    /* STEP 1: Generate max packets traffic - expect Exact Match hit on an autoLearn Entry
               traffic forwarded to port 2 from auto learn reduced action - auto learn is enabled */
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend(1,maxNumber);

    /* STEP 2: GetNext maxNumber times */
    for (k=0;k<maxNumber;k++)
    {
       /* Gets the oldest entry from the current auto-learned
          Exact Match entries and its assosiated Flow ID. */
       rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             &(flowIdIndex[k]),
                                                             &(exactMatchIndex[k]));
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);
    }

    /* STEP 3: Gets the oldest entry from the current auto-learned
          Exact Match entries and its assosiated Flow ID. */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          &(flowIdIndex[k]),
                                                          &(exactMatchIndex[k]));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);

    /* STEP 4: recycle all entries */
    for (k=0;k<maxNumber;k++)
    {
        /* Delete Auto Learn entry from the Exact Match table */
        rc = cpssDxChExactMatchPortGroupEntryInvalidate(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,exactMatchIndex[k]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

        /* Recycle Entry */
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        if (rc!=GT_OK && rc!=GT_EMPTY)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");
        }
    }

    /* Read the value of the 'oldest' Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus2;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&firstFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of the next Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus3;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&nextFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of allocated FlowIDs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus1;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,1,17,&numAllocated);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* AUTODEOC: STEP 4: check pointers values */
    if (!((firstFlowId == ((baseFlowId+0) % maxNumber)) && (nextFlowId == ((baseFlowId+0) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].flowIdPointer == ((baseFlowId+19) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].isFirstPointer == GT_TRUE) && numAllocated == 0))
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FAIL, rc, "tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios FAILED: %d", prvTgfDevNum);
    }

    /* STEP 5: Generate max packets traffic - expect Exact Match hit on an autoLearn Entry
               traffic forwarded to port 2 from auto learn reduced action - auto learn is enabled */
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend(1,maxNumber);

    /* STEP 6: Gets the oldest entry from the current auto-learned
          Exact Match entries and its assosiated Flow ID. */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          &(flowIdIndex[0]),
                                                          &(exactMatchIndex[0]));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);

    /* Delete Auto Learn entry from the Exact Match table */
    rc = cpssDxChExactMatchPortGroupEntryInvalidate(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,exactMatchIndex[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

    /* Recycle Entry */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
    if (rc!=GT_OK && rc!=GT_EMPTY)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");
    }

    /* Read the value of the 'oldest' Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus2;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&firstFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of the next Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus3;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&nextFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of allocated FlowIDs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus1;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,1,17,&numAllocated);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* AUTODEOC: STEP 6: check pointers values */
    if (!((firstFlowId == ((baseFlowId+1) % maxNumber)) && (nextFlowId == ((baseFlowId+0) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].flowIdPointer == ((baseFlowId+0) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].isFirstPointer == GT_TRUE) && numAllocated == 19))
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FAIL, rc, "tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios FAILED: %d", prvTgfDevNum);
    }

    /* STEP 7: GetNext maxNumber-1 times */
    for (k=0;k<(maxNumber-1);k++)
    {
       /* Gets the oldest entry from the current auto-learned
          Exact Match entries and its assosiated Flow ID. */
       rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             &(flowIdIndex[k]),
                                                             &(exactMatchIndex[k]));
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);
    }

    /* Read the value of the 'oldest' Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus2;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&firstFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of the next Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus3;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&nextFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of allocated FlowIDs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus1;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,1,17,&numAllocated);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* STEP 8: Gets the oldest entry from the current auto-learned
          Exact Match entries and its assosiated Flow ID. */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          &(flowIdIndex[k+1]),
                                                          &(exactMatchIndex[k+1]));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);

    /* STEP 9: recycle all entries */
    for (k=0;k<(maxNumber-1);k++)
    {
        /* Delete Auto Learn entry from the Exact Match table */
        rc = cpssDxChExactMatchPortGroupEntryInvalidate(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,exactMatchIndex[k]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

        /* Recycle Entry */
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        if (rc!=GT_OK && rc!=GT_EMPTY)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");
        }
    }

    /* Read the value of the 'oldest' Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus2;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&firstFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of the next Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus3;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&nextFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of allocated FlowIDs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus1;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,1,17,&numAllocated);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* AUTODEOC: STEP 9: check pointers values */
    if (!((firstFlowId == ((baseFlowId+0) % maxNumber)) && (nextFlowId == ((baseFlowId+0) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].flowIdPointer == ((baseFlowId+19) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].isFirstPointer == GT_TRUE) && numAllocated == 0))
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FAIL, rc, "tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios FAILED: %d", prvTgfDevNum);
    }

    /* STEP 10: Generate max packets traffic - expect Exact Match hit on an autoLearn Entry
               traffic forwarded to port 2 from auto learn reduced action - auto learn is enabled */
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend(1,maxNumber/2);

    /* STEP 11: GetNext maxNumber/2 times */
    for (k=0;k<maxNumber/2;k++)
    {
       /* Gets the oldest entry from the current auto-learned
          Exact Match entries and its assosiated Flow ID. */
       rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             &(flowIdIndex[k]),
                                                             &(exactMatchIndex[k]));
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);
    }

    /* STEP 12: Gets the oldest entry from the current auto-learned
          Exact Match entries and its assosiated Flow ID. */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          &(flowIdIndex[k]),
                                                          &(exactMatchIndex[k]));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);

    /* STEP 13: recycle all entries */
    for (k=0;k<maxNumber/2;k++)
    {
        /* Delete Auto Learn entry from the Exact Match table */
        rc = cpssDxChExactMatchPortGroupEntryInvalidate(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,exactMatchIndex[k]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

        /* Recycle Entry */
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        if (rc!=GT_OK && rc!=GT_EMPTY)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");
        }
    }

    /* Read the value of the 'oldest' Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus2;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&firstFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of the next Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus3;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&nextFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of allocated FlowIDs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus1;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,1,17,&numAllocated);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* AUTODEOC: STEP 13: check pointers values */
    if (!((firstFlowId == ((baseFlowId+10) % maxNumber)) && (nextFlowId == ((baseFlowId+10) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].flowIdPointer == ((baseFlowId+9) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].isFirstPointer == GT_TRUE) && numAllocated == 0))
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FAIL, rc, "tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios FAILED: %d", prvTgfDevNum);
    }

    /* get the oldest flowId learned (or the first free flowId if the table is empty) */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             &baseFlowId,
                                                             &autoLearnOldestExactMatchIndex);
    if (rc!=GT_OK && rc!=GT_EMPTY)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet");
    }

    /* STEP 14: Generate max packets traffic - expect Exact Match hit on an autoLearn Entry
               traffic forwarded to port 2 from auto learn reduced action - auto learn is enabled */
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend(0,maxNumber/2);

    /* STEP 15: GetNext maxNumber/2 times */
    for (k=0;k<maxNumber/2;k++)
    {
       /* Gets the oldest entry from the current auto-learned
          Exact Match entries and its assosiated Flow ID. */
       rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             &(flowIdIndex[k]),
                                                             &(exactMatchIndex[k]));
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);
    }

    /* STEP 16: Gets the oldest entry from the current auto-learned
          Exact Match entries and its assosiated Flow ID. */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          &(flowIdIndex[k]),
                                                          &(exactMatchIndex[k]));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);

    /* STEP 17: recycle all entries */
    for (k=0;k<maxNumber/2;k++)
    {
        /* Delete Auto Learn entry from the Exact Match table */
        rc = cpssDxChExactMatchPortGroupEntryInvalidate(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,exactMatchIndex[k]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

        /* Recycle Entry */
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        if (rc!=GT_OK && rc!=GT_EMPTY)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");
        }
    }

    /* Read the value of the 'oldest' Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus2;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&firstFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of the next Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus3;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&nextFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of allocated FlowIDs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus1;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,1,17,&numAllocated);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* AUTODEOC: STEP 17: check pointers values */
    if (!((firstFlowId == ((baseFlowId+10) % maxNumber)) && (nextFlowId == ((baseFlowId+10) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].flowIdPointer == ((baseFlowId+9) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].isFirstPointer == GT_TRUE) && numAllocated == 0))
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FAIL, rc, "tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios FAILED: %d", prvTgfDevNum);
    }

    numberOfPacketsLearned = 0;

    /* return to test default value */
    pktCmdUsedForExactMatchEntry=CPSS_PACKET_CMD_DROP_HARD_E;

    /* disable Exact Match Auto Learning per profile ID before changing Flow ID allocation configuration */
    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(prvTgfDevNum,
                                                                  exactMatchAutoLearnProfileIndex,
                                                                  GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet FAILED: %d", prvTgfDevNum);
}

/**
* @internal
*           tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios3
*           function
* @endinternal
*
* @param[in] maxNumber - max number of EM AutoLearn entries in DB
*
*/
GT_VOID tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios3
(
    GT_U32 maxNumber
)
{
    GT_STATUS           rc=GT_OK;
    GT_U32              k,portGroupId;
    GT_U32              regAddr;
    GT_U32              firstFlowId,nextFlowId;
    GT_U32              flowIdIndex[10],exactMatchIndex[10];
    GT_U32              numAllocated,baseFlowId,autoLearnOldestExactMatchIndex;
    GT_U32              exactMatchAutoLearnProfileIndex = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;

    portGroupId = PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.firstActivePortGroup;

    /* test senario:
       0. get first free flowId; this will be baseFlowId.
       1. learn 5 entries                                                         .
       2. recycle all entries.                                                                        .
       3. learn 8 entries; firstFlowId == ((baseFlowId+5) % maxNumber); nextFlowId == ((baseFlowId+3) % maxNumber).                                                                                               .
       4. getNext 7 times; flowIdPointer == ((baseFlowId+1) % maxNumber).                                                                                                                                         .
       5. recycle 1 entry. firstFlowId == ((baseFlowId+6) % maxNumber).                                                                                                                                                                                         	  .
       6. getNext 1 time; flowIdPointer == ((baseFlowId+2) % maxNumber).                                                                                                                                                                                        																	  .
       7. getNext should return GT_NO_MORE.
       8. recycle all entries; firstFlowId == nextFlowId == ((baseFlowId+3) % maxNumber).
       9. learn 7 entries.
       10. recycle all entries; firstFlowId == nextFlowId == ((baseFlowId+0) % maxNumber).
       */

    /* get the oldest flowId learned (or the first free flowId if the table is empty) */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             &baseFlowId,
                                                             &autoLearnOldestExactMatchIndex);
    if (rc!=GT_OK && rc!=GT_EMPTY)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet");
    }

    /* STEP 1: Generate 5 packets traffic - expect Exact Match hit on an autoLearn Entry
               traffic forwarded to port 2 from auto learn reduced action - auto learn is enabled */
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend(1,5);

    /* STEP 2: recycle all entries */
    for (k=0;k<5;k++)
    {
        /* Gets the oldest entry from the current auto-learned
          Exact Match entries and its assosiated Flow ID. */
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                              &(flowIdIndex[k]),
                                                              &(exactMatchIndex[k]));
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);

        /* Delete Auto Learn entry from the Exact Match table */
        rc = cpssDxChExactMatchPortGroupEntryInvalidate(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,exactMatchIndex[k]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

        /* Recycle Entry */
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        if (rc!=GT_OK && rc!=GT_EMPTY)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");
        }
    }

    /* STEP 3: Generate 8 packets traffic - expect Exact Match hit on an autoLearn Entry
               traffic forwarded to port 2 from auto learn reduced action - auto learn is enabled */
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend(1,8);

    /* Read the value of the 'oldest' Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus2;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&firstFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of the next Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus3;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&nextFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of allocated FlowIDs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus1;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,1,17,&numAllocated);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* STEP 3: check pointers values */
    if (!((firstFlowId == ((baseFlowId+5) % maxNumber)) && (nextFlowId == ((baseFlowId+3) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].flowIdPointer == ((baseFlowId+4) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].isFirstPointer == GT_TRUE) && numAllocated == 8))
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FAIL, rc, "tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios FAILED: %d", prvTgfDevNum);
    }

    /* STEP 4: GetNext 7 times */
    for (k=0;k<7;k++)
    {
       /* Gets the oldest entry from the current auto-learned
          Exact Match entries and its assosiated Flow ID. */
       rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             &(flowIdIndex[k]),
                                                             &(exactMatchIndex[k]));
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);
    }

    /* STEP5: Delete Auto Learn entry from the Exact Match table */
    rc = cpssDxChExactMatchPortGroupEntryInvalidate(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,exactMatchIndex[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

    /* Recycle Entry */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
    if (rc!=GT_OK && rc!=GT_EMPTY)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");
    }

    /* STEP 6: GetNext 1 time */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             &(flowIdIndex[7]),
                                                             &(exactMatchIndex[7]));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);

    /* Read the value of the 'oldest' Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus2;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&firstFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of the next Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus3;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&nextFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of allocated FlowIDs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus1;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,1,17,&numAllocated);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* STEP 6: check pointers values */
    if (!((firstFlowId == ((baseFlowId+6) % maxNumber)) && (nextFlowId == ((baseFlowId+3) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].flowIdPointer == ((baseFlowId+2) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].isFirstPointer == GT_FALSE) && numAllocated == 7 &&
        (flowIdIndex[7] == ((baseFlowId+2) % maxNumber))))
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FAIL, rc, "tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios FAILED: %d", prvTgfDevNum);
    }

    /* STEP 7: GetNext 1 time */
    rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             &(flowIdIndex[8]),
                                                             &(exactMatchIndex[8]));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);

    /* STEP 8: recycle all entries */
    for (k=1;k<8;k++)
    {
        /* Delete Auto Learn entry from the Exact Match table */
        rc = cpssDxChExactMatchPortGroupEntryInvalidate(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,exactMatchIndex[k]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

        /* Recycle Entry */
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        if (rc!=GT_OK && rc!=GT_EMPTY)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");
        }
    }

    /* Read the value of the 'oldest' Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus2;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&firstFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of the next Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus3;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&nextFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of allocated FlowIDs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus1;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,1,17,&numAllocated);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* STEP 8: check pointers values */
    if (!((firstFlowId == ((baseFlowId+3) % maxNumber)) && (nextFlowId == ((baseFlowId+3) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].flowIdPointer == ((baseFlowId+2) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].isFirstPointer == GT_TRUE)) && numAllocated == 0)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FAIL, rc, "tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios FAILED: %d", prvTgfDevNum);
    }

    /* STEP 9: Generate max packets traffic - expect Exact Match hit on an autoLearn Entry
               traffic forwarded to port 2 from auto learn reduced action - auto learn is enabled */
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend(0,7);

    /* STEP 10: recycle all entries */
    for (k=0;k<7;k++)
    {
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          &(flowIdIndex[k]),
                                                          &(exactMatchIndex[k]));
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryGetNext FAILED: %d", prvTgfDevNum);

        /* Delete Auto Learn entry from the Exact Match table */
        rc = cpssDxChExactMatchPortGroupEntryInvalidate(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,exactMatchIndex[k]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

        /* Recycle Entry */
        rc = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        if (rc!=GT_OK && rc!=GT_EMPTY)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger");
        }
    }

    /* Read the value of the 'oldest' Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus2;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&firstFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of the next Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus3;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&nextFlowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* Read the value of allocated FlowIDs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus1;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,1,17,&numAllocated);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssHwPpPortGroupGetRegField FAILED: %d", prvTgfDevNum);

    /* AUTODEOC: STEP 10: check pointers values */
    if (!((firstFlowId == ((baseFlowId+0) % maxNumber)) && (nextFlowId == ((baseFlowId+0) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].flowIdPointer == ((baseFlowId+9) % maxNumber)) &&
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId].isFirstPointer == GT_TRUE)) && numAllocated == 0)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FAIL, rc, "tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios FAILED: %d", prvTgfDevNum);
    }

    numberOfPacketsLearned = 0;

    /* return to test default value */
    pktCmdUsedForExactMatchEntry=CPSS_PACKET_CMD_DROP_HARD_E;

    /* disable Exact Match Auto Learning per profile ID before changing Flow ID allocation configuration */
    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(prvTgfDevNum,
                                                                  exactMatchAutoLearnProfileIndex,
                                                                  GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet FAILED: %d", prvTgfDevNum);
}

/**
*   @internal prvTgfExactMatchAutoLearnTtiPclFullPathCheckFailCounter function
*
* @endinternal
*
* @brief   Counts the number of flows EM didn't learn due to
*          lack of availables flowID's
*
* @param[in] expectedFlowIdFailCounterValue - number of EM entries didn't learn due to lack of flowIDs
* @param[in] expectedIndexFailCounterValue - number of EM entries didn't learn due to lack of free entries
*/
GT_VOID prvTgfExactMatchAutoLearnTtiPclFullPathCheckFailCounter
(
    GT_U32 expectedFlowIdFailCounterValue,
    GT_U32 expectedIndexFailCounterValue
)
{
    GT_STATUS rc=GT_OK;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FAIL_COUNTERS_STC          failCounter;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC   flowIdAllocStatus;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC         flowIdConfiguration;

    rc = cpssDxChExactMatchPortGroupAutoLearnFailCountersGet(prvTgfDevNum,
                                                             CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             &failCounter);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnFailCountersGet FAILED: %d", prvTgfDevNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(expectedFlowIdFailCounterValue, failCounter.flowIdFailCounter,
                                 "cpssDxChExactMatchPortGroupAutoLearnFailCountersGet FAILED: expectedCounter=%d not as currentCounter=%d",
                                 expectedFlowIdFailCounterValue,failCounter.flowIdFailCounter);
    UTF_VERIFY_EQUAL2_STRING_MAC(expectedIndexFailCounterValue, failCounter.indexFailCounter,
                                 "cpssDxChExactMatchPortGroupAutoLearnFailCountersGet FAILED: expectedCounter=%d not as currentCounter=%d",
                                 expectedIndexFailCounterValue,failCounter.flowIdFailCounter);

    /* if we expect flowIdFail, it means the range is full */
    if (expectedFlowIdFailCounterValue!=0)
    {
        rc = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet(prvTgfDevNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, &flowIdConfiguration);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet FAILED: %d", prvTgfDevNum);

        rc = cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet(prvTgfDevNum,
                                                                       CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                                       &flowIdAllocStatus);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet FAILED: %d", prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, flowIdAllocStatus.flowIdRangeIsFull,
                                      "cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet FAILED: flowIdRangeIsFull=%d is not as expected",
                                      flowIdAllocStatus.flowIdRangeIsFull);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowIdConfiguration.maxNum, flowIdAllocStatus.numAllocated,
                                      "cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet FAILED: numAllocated=%d is not as expected=%d",
                                      flowIdAllocStatus.numAllocated,flowIdConfiguration.maxNum);
    }
}

/**
*   @internal prvTgfExactMatchAutoLearnTtiPclFullPathCheckCollision function
*
* @endinternal
*
* @brief   Gets the Exact Match Auto Learn collision key parameters
*
*/
GT_VOID prvTgfExactMatchAutoLearnTtiPclFullPathCheckCollision
(
    GT_VOID
)
{
    GT_STATUS                       rc=GT_OK;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC keyParams;
#if 0
    PRV_TGF_PCL_RULE_FORMAT_UNT     pattern;
    PRV_TGF_EXACT_MATCH_ENTRY_STC   exactMatchEntry;
    GT_U32                          vlanId;
#endif
    rc = cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet(prvTgfDevNum,
                                                             CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             &keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet FAILED: %d", prvTgfDevNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E, keyParams.lookupNum,
                                 "cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet FAILED: expectedLookup%d not as currentLookup=%d",
                                 CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E,keyParams.lookupNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E, keyParams.key.keySize,
                                 "cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet FAILED: expectedKeySize=%d not as currentKeySize=%d",
                                 CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E,keyParams.key.keySize);

#if 0
/* we have no way of knowing what is the pattern that will cause the collision
   in case we do we would compare as described bellow */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(pattern.ruleIngrUdbOnly.udb, 0, sizeof(pattern.ruleIngrUdbOnly.udb));

    /* pattern */
    vlanId = 0x5;/* fit to the packet that will be learn - we need to have a collision */
    pattern.ruleIngrUdbOnly.udb[0]=PRV_TGF_PCL_ID_CNS;  /* {PCL-ID[7:0]}                             */
    pattern.ruleIngrUdbOnly.udb[1]=0x80;                /* {UDB Valid,reserved,PCL-ID[9:8]}          */
    pattern.ruleIngrUdbOnly.udb[2]=(vlanId&0xFF);     /* eVLAN LSB [7:0] */
    pattern.ruleIngrUdbOnly.udb[3]=((vlanId>>8)&0x1F);/* eVLAN MSB [12:8]*/
    pattern.ruleIngrUdbOnly.udb[4]=(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]&0xFF);/* Source-ePort[7:0] */
    pattern.ruleIngrUdbOnly.udb[5]=((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]>>8)&0x1F);/* Source-ePort[12:8] */

    /*{ 0x00, 0x00, 0x44, 0x33, 0x22, 0x11 } - daMac */
    pattern.ruleIngrUdbOnly.udb[6]=0;       /* Dst Mac MSB      */
    pattern.ruleIngrUdbOnly.udb[7]=0;       /* Dst Mac byte 1   */
    pattern.ruleIngrUdbOnly.udb[8]=0x44;    /* Dst Mac byte 2   */
    pattern.ruleIngrUdbOnly.udb[9]=0x33;    /* Dst Mac byte 3   */
    pattern.ruleIngrUdbOnly.udb[10]=0x22;   /* Dst Mac byte 4   */
    pattern.ruleIngrUdbOnly.udb[11]=0x11;   /* Dst Mac LSB      */

    if(0!=cpssOsMemCmp((GT_VOID*) &keyParams.key.pattern[0], pattern.ruleIngrUdbOnly.udb, sizeof(exactMatchEntry.key.pattern)))
    {
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_FAIL, GT_FAIL,
                                     "cpssDxChExactMatchPortGroupAutoLearnFailCountersGet FAILED: keyParams.key.pattern not as expected %d",
                                     prvTgfDevNum);
    }
#endif
}

/**
*   @internal prvTgfExactMatchAutoLearnAddStaticEntry function
*
* @endinternal
*
* @brief  Set Exact Match static entry
*
*/
GT_VOID prvTgfExactMatchAutoLearnAddStaticEntry
(
    GT_VOID
)
{
    GT_STATUS                                       rc=GT_OK;
    GT_U32                                          index=0;
    GT_U32                                          numberOfElemInCrcMultiHashArr;
    GT_U32                                          exactMatchCrcMultiHashArr[16];
    PRV_TGF_PCL_RULE_FORMAT_UNT                     pattern;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT             actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                  actionData;
    PRV_TGF_EXACT_MATCH_ENTRY_STC                   exactMatchEntry;
    GT_U32                                          expandedActionIndex;
    GT_U32                                          vlanId;
    GT_U32                                          bank;

    /* set illegal value in index array*/
    cpssOsMemSet(prvTgfExactMatchRestoreCfg.exactMatchEntryIndexArr, 0xFFFFFFFF, sizeof(prvTgfExactMatchRestoreCfg.exactMatchEntryIndexArr));
    prvTgfExactMatchRestoreCfg.numberOfElementsInExactMatchEntryIndexArr=0;

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &actionData, 0, sizeof(actionData));

    actionData.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    actionData.pclAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E+6;/* this is a different value then the one configured for autoLearn */
    actionData.pclAction.bypassBridge = GT_TRUE;
    actionData.pclAction.bypassIngressPipe = GT_TRUE;
    actionData.pclAction.redirect.redirectCmd=PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    actionData.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_PORT_E;
    actionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum=prvTgfDevNum;
    actionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];
    actionData.pclAction.egressPolicy = GT_FALSE;
    actionData.pclAction.policer.policerEnable = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
    actionData.pclAction.ipfixEnable = GT_TRUE;

    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(pattern.ruleIngrUdbOnly.udb, 0, sizeof(pattern.ruleIngrUdbOnly.udb));

    /* pattern */
    vlanId = 0x5;/* fit to the packet that will be learn - we need to have a collision */
    pattern.ruleIngrUdbOnly.udb[0]=PRV_TGF_PCL_ID_CNS;  /* {PCL-ID[7:0]}                             */
    pattern.ruleIngrUdbOnly.udb[1]=0x80;                /* {UDB Valid,reserved,PCL-ID[9:8]}          */
    pattern.ruleIngrUdbOnly.udb[2]=(vlanId&0xFF);     /* eVLAN LSB [7:0] */
    pattern.ruleIngrUdbOnly.udb[3]=((vlanId>>8)&0x1F);/* eVLAN MSB [12:8]*/
    pattern.ruleIngrUdbOnly.udb[4]=(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]&0xFF);/* Source-ePort[7:0] */
    pattern.ruleIngrUdbOnly.udb[5]=((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]>>8)&0x1F);/* Source-ePort[12:8] */

    /*{ 0x00, 0x00, 0x44, 0x33, 0x22, 0x11 } - daMac */
    pattern.ruleIngrUdbOnly.udb[6]=0;       /* Dst Mac MSB      */
    pattern.ruleIngrUdbOnly.udb[7]=0;       /* Dst Mac byte 1   */
    pattern.ruleIngrUdbOnly.udb[8]=0x44;    /* Dst Mac byte 2   */
    pattern.ruleIngrUdbOnly.udb[9]=0x33;    /* Dst Mac byte 3   */
    pattern.ruleIngrUdbOnly.udb[10]=0x22;   /* Dst Mac byte 4   */
    pattern.ruleIngrUdbOnly.udb[11]=0x11;   /* Dst Mac LSB      */

    /* AUTODOC: calculate index */
    exactMatchEntry.key.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_33B_E;

    expandedActionIndex = PRV_TGF_EXACT_MATCH_EXPANDER_ID_CNS;
    actionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;

    /* here we check exactly the same pattern set for the PCL match in TCAM */
    cpssOsMemCpy((GT_VOID*) &exactMatchEntry.key.pattern[0], pattern.ruleIngrUdbOnly.udb, sizeof(exactMatchEntry.key.pattern));
    exactMatchEntry.lookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;

    rc =  prvTgfExactMatchHashCrcMultiResultsByParamsCalc(prvTgfDevNum,
                                                          &exactMatchEntry.key,
                                                          &numberOfElemInCrcMultiHashArr,
                                                          exactMatchCrcMultiHashArr);


    if (numberOfElemInCrcMultiHashArr!=0)
    {
        prvTgfExactMatchRestoreCfg.numberOfElementsInExactMatchEntryIndexArr=numberOfElemInCrcMultiHashArr;

        /* add the same entry to all possible indexes,
        we need all indexes to be taken inorder to get a colllision */
        for (bank = 0;
            ((bank < numberOfElemInCrcMultiHashArr)&&((bank+exactMatchEntry.key.keySize)<numberOfElemInCrcMultiHashArr));
             bank +=(exactMatchEntry.key.keySize+1))
        {
            /* we check that we have all relative bank values and this is
               not the end of the array that hold irrelevant indexes */
            index = exactMatchCrcMultiHashArr[bank];
            prvTgfExactMatchRestoreCfg.exactMatchEntryIndexArr[bank]=index;

            /* AUTODOC: set Exact Match Entry */
            rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                 index,
                                                 expandedActionIndex,
                                                 &exactMatchEntry,
                                                 actionType,/* same as configured in Extpanded Entry */
                                                 &actionData);/* same as configured in Extpanded Entry */
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);
        }
    }
    else
    {
        rc = GT_BAD_VALUE;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "numberOfElemInCrcMultiHashArr is 0  - illegal - FAILED");
    }

   /* expect the next packet that will be send will auto learn -->
       so need to change the command from forward to drop so the expected values will fit */
    pktCmdUsedForExactMatchEntry=CPSS_PACKET_CMD_DROP_HARD_E;
}
/**
* @internal prvTgfExactMatchAutoLearnAddStaticEntryInvalidate function
* @endinternal
*
* @brief   Invalidate Exact Match Entry
*/
GT_VOID prvTgfExactMatchAutoLearnAddStaticEntryInvalidate
(
    GT_VOID
)
{
    GT_STATUS           rc=GT_OK;
    GT_PORT_GROUPS_BMP  portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32              bank;

    /* invalidate all entries added */
    for (bank = 0;bank < prvTgfExactMatchRestoreCfg.numberOfElementsInExactMatchEntryIndexArr ;bank++)
   {
        /* find a real index and invalidate it */
        if (prvTgfExactMatchRestoreCfg.exactMatchEntryIndexArr[bank]!=0xFFFFFFFF)
        {
            /* invalidate the entries */
            rc = prvTgfExactMatchPortGroupEntryInvalidate(prvTgfDevNum,portGroupsBmp,prvTgfExactMatchRestoreCfg.exactMatchEntryIndexArr[bank]);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);
        }
    }

    /* set illegal value in index array*/
    cpssOsMemSet(prvTgfExactMatchRestoreCfg.exactMatchEntryIndexArr, 0xFFFFFFFF, sizeof(prvTgfExactMatchRestoreCfg.exactMatchEntryIndexArr));
    prvTgfExactMatchRestoreCfg.numberOfElementsInExactMatchEntryIndexArr=0;

    /* return to test default value */
    pktCmdUsedForExactMatchEntry=CPSS_PACKET_CMD_DROP_HARD_E;
}

/**
* @internal prvTgfExactMatchAutoLearnTtiPclFullPathCheckEventInterrupt function
* @endinternal
*
* @brief   Check/Clear Auto Learn interrupts
*
* @param[in] clearInterrupts - GT_TRUE: clear the interrupts, do not check counter
*                              GT_FALSE: Read the interrupts and check counter
* @param[in] expectedFlowAllocationFailEvents - number of "flow allocation fail" events expected
* @param[in] expectedRangeFullEvents - number of "range full" events expected
* @param[in] expectedThresholdCrossedEvents - number of "threshold crossed" events expected
* @param[in] expectedFlowIdEmptyEvents - number of "flow Id empty" events expected
* @param[in] expectedCollisionEvents - number of "auto learn Collision" events expected
*
*/
GT_VOID prvTgfExactMatchAutoLearnTtiPclFullPathCheckEventInterrupt
(
    GT_BOOL     clearInterrupts,
    GT_U32      expectedFlowAllocationFailEvents,
    GT_U32      expectedRangeFullEvents,
    GT_U32      expectedThresholdCrossedEvents,
    GT_U32      expectedFlowIdEmptyEvents,
    GT_U32      expectedCollisionEvents
)
{
    GT_STATUS       rc=GT_OK;
    GT_U32          numOfEvents;    /*number of cpss events         */

    if(GT_FALSE==PRV_CPSS_PP_MAC(prvTgfDevNum)->isGmDevice)
    {
        /* check for interrupt */
        rc = utfGenEventCounterGet(prvTgfDevNum,
                                  CPSS_PP_EM_AUTO_LEARN_FLOW_ID_ALLOCATION_FAILED_EVENT_E,GT_TRUE,&numOfEvents);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpGenGetEvent , utfGenEventCounterGet - CPSS_PP_EM_AUTO_LEARN_FLOW_ID_ALLOCATION_FAILED_EVENT_E");
        if(clearInterrupts==GT_FALSE)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedFlowAllocationFailEvents, numOfEvents, "prvTgfPtpGenGetEvent , utfGenEventCounterGet - CPSS_PP_EM_AUTO_LEARN_FLOW_ID_ALLOCATION_FAILED_EVENT_E");
        }

        rc = utfGenEventCounterGet(prvTgfDevNum,
                                  CPSS_PP_EM_AUTO_LEARN_FLOW_ID_RANGE_FULL_EVENT_E,GT_TRUE,&numOfEvents);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpGenGetEvent , utfGenEventCounterGet - CPSS_PP_EM_AUTO_LEARN_FLOW_ID_RANGE_FULL_EVENT_E");
        if(clearInterrupts==GT_FALSE)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedRangeFullEvents, numOfEvents, "prvTgfPtpGenGetEvent , utfGenEventCounterGet - CPSS_PP_EM_AUTO_LEARN_FLOW_ID_RANGE_FULL_EVENT_E");
        }

        /* if expectedThresholdCrossedEvents is 0xFFFFFFFF
           then ignore the threshold check */
        if(expectedThresholdCrossedEvents!=0XFFFFFFFF)
        {
            rc = utfGenEventCounterGet(prvTgfDevNum,
                                      CPSS_PP_EM_AUTO_LEARN_FLOW_ID_THRESHOLD_CROSSED_EVENT_E,GT_TRUE,&numOfEvents);
            if((numOfEvents > expectedThresholdCrossedEvents + 3) || (numOfEvents < expectedThresholdCrossedEvents))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpGenGetEvent , utfGenEventCounterGet - CPSS_PP_EM_AUTO_LEARN_FLOW_ID_THRESHOLD_CROSSED_EVENT_E");
                if(clearInterrupts==GT_FALSE)
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC(expectedThresholdCrossedEvents, numOfEvents, "prvTgfPtpGenGetEvent , utfGenEventCounterGet - CPSS_PP_EM_AUTO_LEARN_FLOW_ID_THRESHOLD_CROSSED_EVENT_E");
                }
            }
        }

        rc = utfGenEventCounterGet(prvTgfDevNum,
                                  CPSS_PP_EM_AUTO_LEARN_FLOW_ID_EMPTY_EVENT_E,GT_TRUE,&numOfEvents);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpGenGetEvent , utfGenEventCounterGet - CPSS_PP_EM_AUTO_LEARN_FLOW_ID_EMPTY_EVENT_E");
        if(clearInterrupts==GT_FALSE)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedFlowIdEmptyEvents, numOfEvents, "prvTgfPtpGenGetEvent , utfGenEventCounterGet - CPSS_PP_EM_AUTO_LEARN_FLOW_ID_EMPTY_EVENT_E");
        }

        rc = utfGenEventCounterGet(prvTgfDevNum,
                                  CPSS_PP_EM_AUTO_LEARN_COLLISION_EVENT_E,GT_TRUE,&numOfEvents);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpGenGetEvent , utfGenEventCounterGet - CPSS_PP_EM_AUTO_LEARN_COLLISION_EVENT_E");
        if(clearInterrupts==GT_FALSE)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedCollisionEvents, numOfEvents, "prvTgfPtpGenGetEvent , utfGenEventCounterGet - CPSS_PP_EM_AUTO_LEARN_COLLISION_EVENT_E");
        }
    }
}

/**
*   @internal prvTgfExactMatchAutoLearnDebugPrint function
*
* @endinternal
*
* @brief  Print Exact Match Debug Info
*
*/
GT_STATUS prvTgfExactMatchAutoLearnDebugPrint(GT_VOID)
{
    GT_STATUS rc;
    GT_U32    regAddr, portGroupId;
    GT_U32    numAllocated,maxNumFlowIds;
    GT_U32    firstFlowId,nextFlowId;
    PRV_CPSS_DXCH_EXACT_MATCH_DB_INFO_PER_PORT_GROUP_STC  *exactMatchInfoPtr;

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(prvTgfDevNum, 0xFFFFFFFF, portGroupId);

    /* Read the number of the allocated Flow IDs to verify that there are Flow-IDs that were learned. */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus1;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,1,17,&numAllocated);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Read the value of the 'oldest' Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus2;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&firstFlowId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Read the value of the 'next' Flow-ID (Relative to BaseFlowID). */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationStatus3;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,0,16,&nextFlowId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Read max Allocatable Flow IDs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(prvTgfDevNum)->EM.EMAutoLearning.EMFlowIdAllocationConfiguration2;
    rc = prvCpssHwPpPortGroupGetRegField(prvTgfDevNum,portGroupId,regAddr,16,16,&maxNumFlowIds);
    if (rc != GT_OK)
    {
        return rc;
    }

    exactMatchInfoPtr = &PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchInfoPerPortGroup[portGroupId];

    cpssOsPrintf("numAllocated [%d] firstFlowId [%d] nextFlowId [%d] maxNumFlowIds [%d] isFirstPointer [%d] flowIdPointer [%d]\n",
                 numAllocated,firstFlowId,nextFlowId,maxNumFlowIds,exactMatchInfoPtr->isFirstPointer,exactMatchInfoPtr->flowIdPointer);

    return GT_OK;
}
