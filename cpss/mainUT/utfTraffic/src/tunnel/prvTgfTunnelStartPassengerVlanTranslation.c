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
* @file prvTgfTunnelStartPassengerVlanTranslation.c
*
* @brief Tunnel Start: Vlan Translation on passenger.
*
* @version   1
********************************************************************************
*/

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>
#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPclGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfCncGen.h>
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelStartMpls.h>
#include <tunnel/prvTgfTunnelTermEtherType.h>
#include <tunnel/prvTgfTunnelStartPassengerVlanTranslation.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default number of packets to send */
static GT_U32        prvTgfBurstCount =   1;

/* default VLAN Id */
#define PRV_TGF_VLANID_1_CNS   5

/* Translate VLAN on passenger */
#define PRV_TGF_VLANID_2_CNS   6

/* evlan for ingress port */
#define PRV_TGF_EVLAN_CNS (3109 % (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(prvTgfDevNum)))

/* default eport1 */
#define PRV_TGF_EPORT1 101

/* default eport2 */
#define PRV_TGF_EPORT2 102

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS      2

/* offset to tcam index */
#define PRV_TGF_TCAM_INDEX_OFFSET 12

/* tunnel start index */
#define PRV_TGF_TUNNEL_START_INDEX 7

/* the TTI Rule index */
#define prvTgfTtiRuleIndex (prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(PRV_TTI_LOOKUP_0, 3)) /* using absolute index */

/* CNC counter block */
#define PRV_TGF_CNC0_BLOCK_NUM_CNS 0

/* CNC Counter index */
#define PRV_TGF_CNC0_COUNTER_NUM_CNS      10

/******************************* Test packets **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x01, 0x01, 0x01, 0x0B},               /* daMac = macGrp */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_1_CNS                          /* pri, cfi, VlanId */
};

/* DATA of packet len = 22 (0x16) */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS /*+ TGF_ETHERTYPE_SIZE_CNS*/ \
    + sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  egressInfo;
    CPSS_TUNNEL_TYPE_ENT                tunnelType;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT   tunnelStartConf;
    CPSS_INTERFACE_INFO_STC eToPhy;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfGetTCAMIndexInfo function
* @endinternal
*
* @brief   Get Tcam Information
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfGetTCAMIndexInfo
(
        IN GT_U32                           tcamIndex,
        OUT GT_U32                          *floorIndex,
        OUT GT_U32                          *floorBankIndex,
        OUT GT_U32                          *floorBlockIndex,
        OUT GT_U32                          *group,
        OUT GT_U32                          *hitNum,
        OUT GT_U32                          *rowInBank,
        OUT GT_BOOL                         *validPtr,
        OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT    *ruleSizePtr
)
{
    GT_STATUS rc;
    GT_U32                              banksPerBlock;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC       floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];

    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
        (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        banksPerBlock = 2;
    }
    else
    {
        banksPerBlock = 6;
    }

    *floorBankIndex = tcamIndex % 12;
    *floorBlockIndex = (*floorBankIndex)/banksPerBlock;
    *floorIndex = tcamIndex/(12*256);

    rc = cpssDxChTcamIndexRangeHitNumAndGroupGet(prvTgfDevNum, *floorIndex, floorInfoArr);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChTcamIndexRangeHitNumAndGroupGet");

    *group = floorInfoArr[*floorBlockIndex].group;
    *hitNum = floorInfoArr[*floorBlockIndex].hitNum;
    *rowInBank = (tcamIndex - *floorBankIndex - *floorIndex*(12*256)) / 12;
    rc = cpssDxChTcamPortGroupRuleValidStatusGet(prvTgfDevNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,tcamIndex, validPtr, ruleSizePtr);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChTcamPortGroupRuleValidStatusGet");

    PRV_UTF_LOG8_MAC("floorBankIndex = %d\nfloorBlockIndex = %d\nfloorIndex = %d\ngroup = %d\nhitNum = %d\nrowInBank = %d\nvalid = %d\n ruleSize = %d\n",
        *floorBankIndex, *floorBlockIndex, *floorIndex, *group, *hitNum, *rowInBank, *validPtr, *ruleSizePtr);

    return GT_OK;
}

/**
* @internal prvTgfEport2PhySetMPLS function
* @endinternal
*
* @brief   Set MPLS tunnel start configuration with eport
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfEport2PhySetMPLS
(
        IN GT_PORT_NUM portNum,
        IN GT_PORT_NUM eport
)
{
    CPSS_INTERFACE_INFO_STC eToPhy;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  egressInfo;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT   tunnelStartConf;
    GT_STATUS rc;
    GT_U32 tunnelStartInd = PRV_TGF_TUNNEL_START_INDEX;

    /* get default value for restore */
    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum, eport, &prvTgfRestoreCfg.eToPhy);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet");

    /* set eport to physical port mapping */
    cpssOsMemSet(&eToPhy,0,sizeof(CPSS_INTERFACE_INFO_STC));
    eToPhy.type = CPSS_INTERFACE_PORT_E;
    eToPhy.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    eToPhy.devPort.portNum = portNum;
    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum, eport, &eToPhy);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet");

    /* get default value for restore */
    rc = cpssDxChTunnelStartEntryGet(prvTgfDevNum, tunnelStartInd, &prvTgfRestoreCfg.tunnelType, &prvTgfRestoreCfg.tunnelStartConf);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChTunnelStartEntryGet");

    /* set tunnel start entry */
    cpssOsMemSet(&tunnelStartConf,0,sizeof(tunnelStartConf));
    tunnelStartConf.mplsCfg.macDa.arEther[0] = 0;
    tunnelStartConf.mplsCfg.macDa.arEther[1] = 0;
    tunnelStartConf.mplsCfg.macDa.arEther[2] = 1;
    tunnelStartConf.mplsCfg.macDa.arEther[3] = 1;
    tunnelStartConf.mplsCfg.macDa.arEther[4] = 1;
    tunnelStartConf.mplsCfg.macDa.arEther[5] = (GT_U8)tunnelStartInd;
    tunnelStartConf.mplsCfg.numLabels       = 3;
    tunnelStartConf.mplsCfg.label1          = 53+tunnelStartInd;
    tunnelStartConf.mplsCfg.label2          = 34;
    tunnelStartConf.mplsCfg.label3          = 35;
    tunnelStartConf.mplsCfg.ttl             = 73;
    tunnelStartConf.mplsCfg.tagEnable       = GT_TRUE;
    tunnelStartConf.mplsCfg.vlanId          = PRV_TGF_VLANID_1_CNS;

    if(tunnelStartInd != 0)
    {
        rc = cpssDxChTunnelStartEntrySet(prvTgfDevNum, tunnelStartInd, CPSS_TUNNEL_X_OVER_MPLS_E, &tunnelStartConf);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChTunnelStartEntrySet");
    }

    /* get default value for restore */
    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum, eport, &prvTgfRestoreCfg.egressInfo);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChBrgEportToPhysicalPortEgressPortInfoGet");

    /* set physical port info for eport*/
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo.tsPassengerPacketType    = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
    if(tunnelStartInd != 0)
    {
        egressInfo.tunnelStart              = GT_TRUE;
        egressInfo.tunnelStartPtr           = tunnelStartInd;
    }
    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum, eport, &egressInfo);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet");

    return GT_OK;
}

/**
* @internal prvTgfTtiMplsEportConfigSet function
* @endinternal
*
* @brief   Set MPLS TTI configuration with Eport
*
* @retval GT_OK                    - on success
*
* @note  1. Set VLAN configuration
*        2. Set TTI configuration
*        3. Configure MPLS tunnet start
*        4. Set VLAN translation configuration
*
*/
static GT_STATUS prvTgfTtiMplsEportConfigSet
(
        GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 tcamIndex = prvTgfTtiRuleIndex;
    CPSS_DXCH_TTI_RULE_UNT      pattern;
    CPSS_DXCH_TTI_RULE_UNT      mask;
    CPSS_DXCH_TTI_ACTION_STC    action;
    CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType;
    CPSS_DXCH_TTI_RULE_TYPE_ENT ruleType;
    GT_U16                      ingressVlanId = PRV_TGF_VLANID_1_CNS;
    GT_U16                      vlanId = PRV_TGF_EVLAN_CNS; /* eVlan */
    GT_U32                      eport1 = PRV_TGF_EPORT1, eport2 = PRV_TGF_EPORT2;
    GT_PORT_NUM                 port1 =  prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]; /* ingress port */

    CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_PORTS_BMP_STC           portsMembersPtr;
    CPSS_PORTS_BMP_STC           portsTaggingPtr;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC    portsTaggingCmd;
    /*GT_ETHERADDR                          srcMac; */
    GT_U32      floorIndex;
    GT_U32      floorBankIndex;
    GT_U32      floorBlockIndex;
    GT_U32      group;
    GT_U32      hitNum;
    GT_U32      rawInBank;
    GT_BOOL     valid;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT ruleSize;

    /* get tcam information for given tcamIndex */
    rc = prvTgfGetTCAMIndexInfo(tcamIndex, &floorBankIndex, &floorBlockIndex, &floorIndex, &group, &hitNum, &rawInBank, &valid, &ruleSize);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfGetTCAMIndexInfo");

    if (hitNum != 0)
    {
        PRV_UTF_LOG0_MAC("Hit number not 0\n");
        return GT_BAD_PARAM;
    }

    /* -------------------------------------------------------------------------
     * 1. Set VLAN configuration
     */

    /* Create EVLAN */
    cpssOsMemSet(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));
    cpssOsMemSet(&portsMembersPtr, 0, sizeof(CPSS_PORTS_BMP_STC));
    cpssOsMemSet(&portsTaggingPtr, 0, sizeof(CPSS_PORTS_BMP_STC));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    cpssVlanInfo.naMsgToCpuEn           = GT_TRUE;
    cpssVlanInfo.autoLearnDisable       = GT_TRUE; /* Disable auto learn on VLAN */
    cpssVlanInfo.ucastLocalSwitchingEn  = GT_TRUE;
    cpssVlanInfo.mcastLocalSwitchingEn  = GT_TRUE;
    cpssVlanInfo.fidValue               = vlanId; /* this is a must for routing!, new for BC2*/
    /* Assign this L2MLL group to Evlan (3109) as a flood id=5000  */

    cpssVlanInfo.floodVidx              = 5000;
    cpssVlanInfo.unregIpmEVidx          = 5000;
    cpssVlanInfo.floodVidxMode          = CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;

    rc = cpssDxChBrgVlanEntryWrite(prvTgfDevNum,vlanId,&portsMembersPtr,&portsTaggingPtr,
                                   &cpssVlanInfo,
                                   &portsTaggingCmd);
    if (rc != GT_OK)
        return rc;

    rc = cpssDxChBrgVlanMemberAdd(prvTgfDevNum,vlanId,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],GT_TRUE,CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E);
    PRV_UTF_VERIFY_GT_OK(rc,"cpssDxChBrgVlanMemberAdd");

    rc = cpssDxChBrgVlanMemberAdd(prvTgfDevNum,vlanId,prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],GT_TRUE,CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E);
    PRV_UTF_VERIFY_GT_OK(rc,"cpssDxChBrgVlanMemberAdd");

     /* -------------------------------------------------------------------------
      * 2. Set TTI configuration
      */

    keyType = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;
    ruleType = CPSS_DXCH_TTI_RULE_UDB_10_E;

    rc = cpssDxChTtiPacketTypeKeySizeSet(prvTgfDevNum, keyType, CPSS_DXCH_TTI_KEY_SIZE_10_B_E);
    PRV_UTF_VERIFY_GT_OK(rc,"cpssDxChTtiPacketTypeKeySizeSet");

    /* UDB for source ePort*/
    rc = cpssDxChTtiUserDefinedByteSet(prvTgfDevNum, keyType, 0,
                                       CPSS_DXCH_TTI_OFFSET_METADATA_E, 26);
    PRV_UTF_VERIFY_GT_OK(rc,"cpssDxChTtiUserDefinedByteSet");

    /* UDB for Tag0 VLAN ID*/
    rc = cpssDxChTtiUserDefinedByteSet(prvTgfDevNum, keyType, 1,
                                       CPSS_DXCH_TTI_OFFSET_METADATA_E, 24);
    PRV_UTF_VERIFY_GT_OK(rc,"cpssDxChTtiUserDefinedByteSet");

    rc = cpssDxChTtiPortLookupEnableSet(prvTgfDevNum, port1, keyType, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc,"cpssDxChTtiPortLookupEnableSet");

    /* For ingress port: Create TTI rule */
    cpssOsMemSet(&(mask), 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cpssOsMemSet(&(pattern), 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));

    pattern.udbArray.udb[0] = (GT_U8)port1;
    mask.udbArray.udb[0] = 0xFF;

    /*if vlan is vlanId, do ePort assignment*/
    pattern.udbArray.udb[1] = (GT_U8)ingressVlanId;
    mask.udbArray.udb[1] = 0xFF;

    /*  action: 1) assigned source eport (example PRV_TGF_EPORT1) 2) assign evlan id 3) forwarding */
    cpssOsMemSet(&action, 0, sizeof(CPSS_DXCH_TTI_ACTION_STC));
    action.command = CPSS_PACKET_CMD_FORWARD_E ;
    action.tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    action.userDefinedCpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 1;
    action.sourceEPortAssignmentEnable = GT_TRUE;
    action.sourceEPort = eport1;
    action.tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
    action.tag0VlanId = vlanId;
    /* For debug the flow - bind the flow to CNC Counter */
    action.bindToCentralCounter         = GT_TRUE                               ;
    action.centralCounterIndex          = PRV_TGF_CNC0_COUNTER_NUM_CNS;

    action.egressInterface.type = CPSS_INTERFACE_PORT_E;
    action.egressInterface.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    action.egressInterface.devPort.portNum = eport2;

    action.redirectCommand  = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;

    action.bridgeBypass       = GT_TRUE;
    action.ingressPipeBypass  = GT_FALSE;
    action.actionStop         = GT_TRUE;

    rc = cpssDxChTtiRuleSet(prvTgfDevNum, tcamIndex+PRV_TGF_TCAM_INDEX_OFFSET, ruleType, &pattern, &mask, &action);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChTtiRuleSet");

    /* -------------------------------------------------------------------------
     * 3. Configure MPLS tunnel start
     */
    rc = prvTgfEport2PhySetMPLS(prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], eport2);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfEport2PhySetMPLS");

    /* -------------------------------------------------------------------------
     * 4. Set Vlan Translation
     */

    /* enable vlan on tunnel passenger */
    rc = cpssDxChTunnelStartPassengerVlanTranslationEnableSet(prvTgfDevNum, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChTunnelStartPassengerVlanTranslationEnableSet");

    /* enable vlan translation on egress */
    rc = cpssDxChBrgVlanPortTranslationEnableSet(prvTgfDevNum, eport1, CPSS_DIRECTION_EGRESS_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChBrgVlanPortTranslationEnableSet");

    /* egress filtering on VLAN before translation */
    rc = cpssDxChBrgVlanTranslationEntryWrite(prvTgfDevNum, vlanId, CPSS_DIRECTION_EGRESS_E, PRV_TGF_VLANID_2_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChBrgVlanTranslationEntryWrite");

    return GT_OK;

}

/**
* @internal prvTgfTcamTtiCountersGet function
* @endinternal
*
* @brief   Get tti counter value
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfTcamTtiCountersGet
(
        OUT GT_U32 *tti_hits
)
{
    GT_STATUS rc;
    GT_U32 totalTTICounter = 0;

    CPSS_DXCH_CNC_COUNTER_STC counter;
    rc = cpssDxChCncCounterGet(prvTgfDevNum, PRV_TGF_CNC0_BLOCK_NUM_CNS, PRV_TGF_CNC0_COUNTER_NUM_CNS, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChCncCounterGet");

    if(counter.packetCount.l[0] != 0)
    {
        totalTTICounter = counter.packetCount.l[0] + totalTTICounter;
        PRV_UTF_LOG2_MAC("TTI rule counter %d hit %X times\n", PRV_TGF_CNC0_BLOCK_NUM_CNS*1024, counter.packetCount.l[0] );
    }

    PRV_UTF_LOG1_MAC("%d packets passed through TTI\n" , totalTTICounter);
    *tti_hits = totalTTICounter;

    return GT_OK;
}

/**
* @internal prvTgfTcamTtiCountersSet function
* @endinternal
*
* @brief   Set tti counter configuration
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfTcamTtiCountersSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       devNum = prvTgfDevNum;
    GT_U64      indexRangesBmp;

    rc = cpssDxChCncCountingEnableSet(devNum, CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_TTI_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChCncCountingEnableSet");

    rc = cpssDxChCncBlockClientEnableSet(prvTgfDevNum, PRV_TGF_CNC0_BLOCK_NUM_CNS, CPSS_DXCH_CNC_CLIENT_TTI_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChCncBlockClientEnableSet");

    indexRangesBmp.l[0] = 1 << PRV_TGF_CNC0_BLOCK_NUM_CNS;
    indexRangesBmp.l[1] = 0;
    rc = cpssDxChCncBlockClientRangesSet(prvTgfDevNum, PRV_TGF_CNC0_BLOCK_NUM_CNS, CPSS_DXCH_CNC_CLIENT_TTI_E, &indexRangesBmp);
    PRV_UTF_VERIFY_GT_OK(rc,"cpssDxChCncBlockClientRangesSet");

    return GT_OK;
}

/**
* @internal prvTgfTunnelStartPassengerVlanTranslationConfigSet function
* @endinternal
*
* @brief   Set tcam tti configuration
*/
GT_VOID prvTgfTunnelStartPassengerVlanTranslationConfigSet
(
        GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: configure tcam tti counters */
    rc = prvTgfTcamTtiCountersSet();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTcamTtiCountersSet failed");

    /* AUTODOC: set MPLS TTI configuration with Eport */
    rc = prvTgfTtiMplsEportConfigSet();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMplsEportConfigSet failed");

}

/**
* @internal prvTgfTunnelStartPassengerVlanTranslationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelStartPassengerVlanTranslationTrafficGenerate
(
        GT_VOID
)
{
    GT_STATUS               rc          = GT_OK;
    TGF_VFD_INFO_STC        vfdArray[1];
    CPSS_INTERFACE_INFO_STC portInt;
    GT_U8                   packetBufPtr[64] = {0};
    GT_U32                  packetBufLen = 64;
    GT_U32                  packetLen;
    GT_U8                   devNum = prvTgfDevNum;
    GT_U8                   queueCpu;
    TGF_NET_DSA_STC         rxParams;
    GT_PORT_NUM             capturePort = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    GT_U16                  expVlan = PRV_TGF_VLANID_2_CNS;
    GT_U32                  expTtiHits = prvTgfBurstCount;
    GT_U16                  obsVlan = 0;
    GT_U32                  obsTtiHits = 0;

    /* AUTODOC: GENERATE TRAFFIC: */
    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset failed\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* enable capture on a MC subscriber port */
    portInt.type            = CPSS_INTERFACE_PORT_E;
    portInt.devPort.hwDevNum  = prvTgfDevNum;
    portInt.devPort.portNum = capturePort;

    /* start capture on egress port*/
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet failed");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear failed");

    /* setup packet 1*/
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth failed");

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth failed");

    cpssOsTimerWkAfter(100);

    /* capture packet by trapping to CPU*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                       GT_TRUE, GT_TRUE,
                                       packetBufPtr,
                                       &packetBufLen,
                                       &packetLen,
                                       &devNum,
                                       &queueCpu,
                                       &rxParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"tgfTrafficGeneratorRxInCpuGet failed");

    /* get observed tti hits */
    rc = prvTgfTcamTtiCountersGet(&obsTtiHits);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTcamTtiCountersGet failed");

    /* extract passenger vlan id from captured packet */
    obsVlan = (GT_U16)((packetBufPtr[44] << 8 | packetBufPtr[45]) & 0xFFF);

    /* Compare result */
    UTF_VERIFY_EQUAL0_STRING_MAC(expTtiHits, obsTtiHits, "TTI hits does not match");
    UTF_VERIFY_EQUAL0_STRING_MAC(expVlan, obsVlan, "Passenger vlan id does not match");

    /* restore port capture setting */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
            prvTgfDevNum, portInt.devPort.portNum);

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture failed");

    /* clear rx packet table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear failed");

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset failed\n");

}

/**
* @internal prvTgfTunnelStartPassengerVlanTranslationConfigRestore function
* @endinternal
*
* @brief   Restore Configuration
*/
GT_VOID prvTgfTunnelStartPassengerVlanTranslationConfigRestore
(
        GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U8 devNum = prvTgfDevNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType;
    GT_U32                      eport1 = PRV_TGF_EPORT1, eport2 = PRV_TGF_EPORT2;
    GT_PORT_NUM                 port1 =  prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_U64      indexRangesBmp;

    /* Restore Vlan configuration */
    rc = cpssDxChBrgVlanEntryInvalidate(devNum, PRV_TGF_EVLAN_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanEntryInvalidate failed. vlan=0x%X",
            PRV_TGF_EVLAN_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush failed");

    /* Restore TTI configuration */
    keyType = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;

    /* restore UDB bytes */
    rc = cpssDxChTtiUserDefinedByteSet(prvTgfDevNum, keyType, 0, CPSS_DXCH_TTI_OFFSET_INVALID_E, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiUserDefinedByteSet failed");

    rc = cpssDxChTtiUserDefinedByteSet(prvTgfDevNum, keyType, 1, CPSS_DXCH_TTI_OFFSET_INVALID_E, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiUserDefinedByteSet failed");

    /* disable tti lookup on ingress port */
    rc = cpssDxChTtiPortLookupEnableSet(prvTgfDevNum, port1, keyType, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiPortLookupEnableSet failed");

    /* restore tunnel start entry */
    rc = cpssDxChTunnelStartEntrySet(prvTgfDevNum, PRV_TGF_TUNNEL_START_INDEX, prvTgfRestoreCfg.tunnelType, &prvTgfRestoreCfg.tunnelStartConf);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTunnelStartEntrySet failed");

    /* remove tti rule */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex+PRV_TGF_TCAM_INDEX_OFFSET, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet failed");

    /* disable vlan Translation on tunnel passenger*/
    rc = cpssDxChTunnelStartPassengerVlanTranslationEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTunnelStartPassengerVlanTranslationEnableSet failed");

    /* disable vlan translation on egress port. */
    rc = cpssDxChBrgVlanPortTranslationEnableSet(prvTgfDevNum, eport1, CPSS_DIRECTION_EGRESS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortTranslationEnableSet failed");

    /* restore eport mapping */
    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum, eport2, &prvTgfRestoreCfg.eToPhy);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet failed");

    /* restore port info */
    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum, eport2, &prvTgfRestoreCfg.egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet failed");

    /* restore tti counter settings */
    rc = cpssDxChCncBlockClientEnableSet(prvTgfDevNum, PRV_TGF_CNC0_BLOCK_NUM_CNS, CPSS_DXCH_CNC_CLIENT_TTI_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncBlockClientEnableSet failed");

    indexRangesBmp.l[0] = 0;
    indexRangesBmp.l[1] = 0;
    rc = cpssDxChCncBlockClientRangesSet(prvTgfDevNum, PRV_TGF_CNC0_BLOCK_NUM_CNS, CPSS_DXCH_CNC_CLIENT_TTI_E, &indexRangesBmp);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChCncBlockClientRangesSet failed");

}

