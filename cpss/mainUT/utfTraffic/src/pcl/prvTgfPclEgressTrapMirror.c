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
* @file prvTgfPclEgressTrapMirror.c
*
* @brief Pcl Egress Mirror/Trap packet cmd tests
*
* @version
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPclGen.h>
#include <pcl/prvTgfPclEgressTrapMirror.h>
#include <tunnel/prvTgfTunnelTermEtherType.h>
#include <common/tgfNetIfGen.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <common/tgfMirror.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_TGF_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E  56
/* port to Send/Receive traffic*/
#define PRV_TGF_INGRESS_PORT_IDX_CNS                            0
#define PRV_TGF_EGRESS_PORT_IDX_CNS                             3
#define EPCL_MATCH_INDEX_CNS                                    prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(6)

/* default number of packets to send */
static GT_U32        prvTgfBurstCount           =   2;
static GT_U32        prvTgfTrapPacketBurstCount =   1;

/* default vlanid */
static GT_U16        prvTgfDefVlanId            =   1;

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};
/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* DATA of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    4,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfTrapDropPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),                 /* dataLength */
    prvTgfPacketPayloadDataArr                          /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfTrapDropPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfTrapDropPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfIpPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,              /* totalLen */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

/* packet to send */
static TGF_PACKET_STC prvTgfIpPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,              /* totalLen */
    (sizeof(prvTgfIpPacketPartArray)
        / sizeof(prvTgfIpPacketPartArray[0])), /* numOfParts */
    prvTgfIpPacketPartArray                    /* partsArray */
};

/* packet to send */
static TGF_PACKET_STC prvTgfTrapDropPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,              /* totalLen */
    (sizeof(prvTgfTrapDropPacketPartArray)
        / sizeof(prvTgfTrapDropPacketPartArray[0])), /* numOfParts */
    prvTgfTrapDropPacketPartArray                    /* partsArray */
};

static GT_BOOL prvTgfActionTrapToCpuCheck[] = {
    GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE, GT_FALSE, GT_FALSE
};

/* PCL UDB configuration */
typedef struct{
    GT_U32                               udbIndex;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType;
    GT_U8                                offset;
    GT_U8                                byteMask;
}prvTgfPclUdb;

static prvTgfPclUdb prvTgfEpclMetadataUdbInfo[] = {

     /* Src Trg/Rx Sniff [0x0 = Ingress] */
     {0 , CPSS_DXCH_PCL_OFFSET_METADATA_E, 53, 0x01},

     /* Egress Mtag Cmd [0x0 = TO_CPU],Bit[1:0]; Egress Marvell Tagged [0x1],Bit[2]; Egress Marvell Tagged Extended [0x3 = 16B eDSA], Bit[4:3] */
     {1 , CPSS_DXCH_PCL_OFFSET_METADATA_E, 64, 0x1F},

     /* must be last */
     {2, CPSS_DXCH_PCL_OFFSET_INVALID_E , 0, 0}
};

static prvTgfPclUdb prvTgfEpclTrapDropMetadataUdbInfo[] = {

     /* Egress Mtag Command 0x3 = FORWARD Bit[1:0] */
     {0 , CPSS_DXCH_PCL_OFFSET_METADATA_E, 64, 0x03},

     /* must be last */
     {1, CPSS_DXCH_PCL_OFFSET_INVALID_E , 0, 0}
};

/* parameters that is needed to be restored */
static struct
{
    GT_U32                                          tcIngressTrapQueueGet;
    GT_U32                                          tcEgressTrapQueueGet;
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC            ingressTrapCpuEntryGet;
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC            egressTrapCpuEntryGet;
    CPSS_DXCH_PHA_FW_IMAGE_ID_ENT                   phaFwImageIdGet;
    GT_BOOL                                         prvTgfSavedCpuEpclEnable;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT    prvTgfSavedCpuEpclAccessMode;
    CPSS_DXCH_MIRROR_ON_DROP_STC                    prvTgfMirrorOnDropEntryGet;
} prvTgfRestoreCfg;

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

GT_STATUS prvTgfFdbDefCfgSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* Create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWrite");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_EPCL_SEND_PORT_INDEX_CNS], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* Set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_EPCL_SEND_PORT_INDEX_CNS], PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* Add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac,
            PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_EPCL_RECEIVE_PORT_INDEX_CNS], GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return rc;
}

GT_STATUS prvTgfEpclReTrapFdbDefCfgSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* Create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWrite");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* Set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* Add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac,
            PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS], GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return rc;
}

GT_STATUS prvTgfFdbDefCfgRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

     /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* Vlan restore */

    /* Restore default PVID on port 1 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_EPCL_SEND_PORT_INDEX_CNS], prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPortVidSet");

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    return rc1;
};

/* packet command for the test*/
static  CPSS_PACKET_CMD_ENT test_pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
/**
* @internal prvTgfPclEgressTrapMirrorConfigurationSet function
* @endinternal
*
* @brief  Configure Egress PCL Rule with:
*         action pkt cmd : CPSS_PACKET_CMD_TRAP_TO_CPU_E
*         cpu code       :
*/
GT_VOID prvTgfPclEgressTrapMirrorCfgSet
(
    GT_VOID
)
{
    GT_STATUS                         rc;
    GT_U32                            ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       pattern;
    PRV_TGF_PCL_ACTION_STC            action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ruleFormat;

    rc = prvTgfFdbDefCfgSet();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFdbDefCfgSet");

    /* PCL Configuration */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* init PCL Engine for Egress PCL */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_EPCL_RECEIVE_PORT_INDEX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* set PCL rule 0 - */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleIndex                                    = 0;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    action.pktCmd                                = test_pktCmd;
    action.mirror.cpuCode                        = CPSS_NET_IP_HDR_ERROR_E;
    action.egressPolicy                          = GT_TRUE;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfEpclDropPacketToErepTrapTest function
* @endinternal
*
* @brief  Configure Egress PCL Rule with:
*         action pkt cmd : CPSS_PACKET_CMD_DROP_HARD_E
*/
GT_VOID prvTgfEpclDropPacketToErepTrapTest
(
    GT_VOID
)
{
    GT_STATUS                         rc;
    GT_U32                            ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       pattern;
    PRV_TGF_PCL_ACTION_STC            action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ruleFormat;
    CPSS_DXCH_MIRROR_ON_DROP_STC      dropConfig;
    GT_U32                            udbSelectidx;
    PRV_TGF_PCL_UDB_SELECT_STC        udbSelect;

    rc = prvTgfFdbDefCfgSet();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFdbDefCfgSet");

    /* init PCL Engine for Egress PCL */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_EPCL_RECEIVE_PORT_INDEX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* set PCL rule 0 - */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        GT_U32  ii;
        for(ii = 0 ; ii < 10; ii++)
        {
            udbSelect.udbSelectArr[ii] = ii;
        }
    }

    while(1)
    {
        if(prvTgfEpclTrapDropMetadataUdbInfo[udbSelectidx].offsetType == CPSS_DXCH_PCL_OFFSET_INVALID_E)
            break;

        /* AUTODOC:   configure User Defined Byte(UDB) */
        /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
        /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E */
        /* AUTODOC:   offsetType CPSS_DXCH_PCL_OFFSET_METADATA_E */
        /* AUTODOC:   offset 64 Egress Mtag Cmd[1:0] */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                        CPSS_PCL_DIRECTION_EGRESS_E,
                                        prvTgfEpclTrapDropMetadataUdbInfo[udbSelectidx].udbIndex,
                                        prvTgfEpclTrapDropMetadataUdbInfo[udbSelectidx].offsetType,
                                        prvTgfEpclTrapDropMetadataUdbInfo[udbSelectidx].offset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

        udbSelect.udbSelectArr[udbSelectidx] = prvTgfEpclTrapDropMetadataUdbInfo[udbSelectidx].udbIndex;
        udbSelectidx++;
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E, PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                           CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);

    /* bit :10 Anchor Offset 64 */
    pattern.ruleEgrUdbOnly.udb[0] = 0x03;
    mask.ruleEgrUdbOnly.udb[0] = prvTgfEpclTrapDropMetadataUdbInfo[0].byteMask;

    ruleIndex                                    = 0;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.pktCmd                                = CPSS_PACKET_CMD_DROP_HARD_E;
    action.mirror.cpuCode                        = CPSS_NET_IP_HDR_ERROR_E;
    action.egressPolicy                          = GT_TRUE;
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);

    dropConfig.mode = CPSS_DXCH_MIRROR_ON_DROP_MODE_TRAP_E;
    dropConfig.analyzerIndex = 0;
    dropConfig.droppedEgressReplicatedPacketMirrorEnable = GT_TRUE;
    rc = prvTgfMirrorOnDropConfigGet(prvTgfDevNum, &prvTgfRestoreCfg.prvTgfMirrorOnDropEntryGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorOnDropConfigGet\n");
    rc = prvTgfMirrorOnDropConfigSet(&dropConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorOnDropConfigSet\n");
}

/**
* @internal prvTgfPclEgressTrapMirrorGenerate function
* @endinternal
*
* @brief  Configure to send packet to port 1
*/
GT_VOID prvTgfPclEgressTrapMirrorTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc  = GT_OK;
    GT_BOOL                         isOk;
    GT_BOOL                         expectTraffic = GT_FALSE;
    GT_U32                          portIter;
    GT_U32                          packetSize = 0;
    GT_U32                          partsCount = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT replicationType;
    GT_U32                          counterVal;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* Reset the port Counter */
    rc = prvTgfCountersEthReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCountersEthReset");

    /* clear the counter before sending packet*/
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        replicationType = CPSS_DXCH_CFG_TRAP_REPLICATION_E;
        rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");

        replicationType = CPSS_DXCH_CFG_OUTGOING_FORWARD_E;
        rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    }

    /* Start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* Generate traffic */

    rc = prvTgfSetTxSetupEth(
            prvTgfDevNum, &prvTgfPacketInfo,
            prvTgfBurstCount, 0 , NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfSetTxSetupEth");

    /* Send packet to port 1 */
    rc = prvTgfStartTransmitingEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EPCL_SEND_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth");

    partsCount    = sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC);

    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);

    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* AUTODOC: verify routed packet on port 1 */
        switch (portIter)
        {
        case PRV_TGF_EPCL_SEND_PORT_INDEX_CNS:
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;
        case PRV_TGF_EPCL_RECEIVE_PORT_INDEX_CNS:
                if (expectTraffic == GT_TRUE)
                {
                    expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                    expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                    expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
                 }
                 else
                 {
                    expectedCntrs.goodOctetsSent.l[0] = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = 0;
                    expectedCntrs.goodPktsSent.l[0]   = 0;
                    expectedCntrs.ucPktsSent.l[0]     = 0;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodPktsRcv.l[0]    = 0;
                    expectedCntrs.ucPktsRcv.l[0]      = 0;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
                 }
                    break;
            default:
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
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
            PRV_UTF_LOG1_MAC("  goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC("    goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("    brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("   goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    if (GT_TRUE == prvTgfActionTrapToCpuCheck[PRV_TGF_CPU_PORT_IDX_CNS])
    {
        GT_U8                           packetBufPtr[64] = {0};
        GT_U32                          packetBufLen = 64;
        GT_U32                          packetLen;
        GT_U8                           devNum;
        GT_U8                           queueCpu;
        TGF_NET_DSA_STC                 rxParams;

        PRV_UTF_LOG0_MAC("check the CPU port capturing:\n");

        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           GT_TRUE, GT_TRUE,
                                           packetBufPtr,
                                           &packetBufLen,
                                           &packetLen,
                                           &devNum,
                                           &queueCpu,
                                           &rxParams);

        if(test_pktCmd == CPSS_PACKET_CMD_DROP_SOFT_E ||
           test_pktCmd == CPSS_PACKET_CMD_DROP_HARD_E)
        {
            /* no packets expected at the CPU */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE,rc);
        }
        else
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
            UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_NET_IP_HDR_ERROR_E,
                                            rxParams.cpuCode);

            PRV_UTF_LOG1_MAC("Source Port :%d\n",rxParams.portNum);
            PRV_UTF_LOG1_MAC("Packet is Trapped to  CPU with CPU Code  :%d\n",rxParams.cpuCode);
        }

    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        replicationType = CPSS_DXCH_CFG_TRAP_REPLICATION_E;
        rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
        PRV_UTF_LOG1_MAC("Number of Replicated packets :%d\n",counterVal);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
        if(test_pktCmd == CPSS_PACKET_CMD_DROP_SOFT_E ||
           test_pktCmd == CPSS_PACKET_CMD_DROP_HARD_E)
        {
            /* no replications to CPU as packet is dropped */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, counterVal, "cpssDxChCfgReplicationCountersGet");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, counterVal, "cpssDxChCfgReplicationCountersGet");
        }

        replicationType = CPSS_DXCH_CFG_OUTGOING_FORWARD_E;
        rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
        if(test_pktCmd == CPSS_PACKET_CMD_DROP_SOFT_E ||
           test_pktCmd == CPSS_PACKET_CMD_DROP_HARD_E)
        {
            /* no replications to CPU as packet is dropped */
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 1), counterVal, "cpssDxChCfgReplicationCountersGet");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 2), counterVal, "cpssDxChCfgReplicationCountersGet");
        }
    }

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfEpclDropPacketToErepTrapTrafficGenerate function
* @endinternal
*
* @brief  Configure to send packet to port 1
*/
GT_VOID prvTgfEpclDropPacketToErepTrapTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc  = GT_OK;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT replicationType;
    GT_U32                          counterVal;
    GT_U8                           packetBufPtr[64] = {0};
    GT_U32                          packetBufLen = 64;
    GT_U32                          packetLen;
    GT_U8                           devNum;
    GT_U8                           queueCpu;
    TGF_NET_DSA_STC                 rxParams;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* Reset the port Counter */
    rc = prvTgfCountersEthReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCountersEthReset");

    /* clear the counter before sending packet*/
    replicationType = CPSS_DXCH_CFG_TRAP_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");

    replicationType = CPSS_DXCH_CFG_OUTGOING_FORWARD_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");

    /* Start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* Generate traffic */

    rc = prvTgfSetTxSetupEth(
            prvTgfDevNum, &prvTgfTrapDropPacketInfo,
            prvTgfBurstCount, 0 , NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfSetTxSetupEth");

    /* Send packet to port 1 */
    rc = prvTgfStartTransmitingEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EPCL_SEND_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth");

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    PRV_UTF_LOG0_MAC("check the CPU port capturing:\n");

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_TRUE, GT_TRUE,
                                       packetBufPtr,
                                       &packetBufLen,
                                       &packetLen,
                                       &devNum,
                                       &queueCpu,
                                       &rxParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
    UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_NET_FIRST_USER_DEFINED_E,
                                    rxParams.cpuCode);

    PRV_UTF_LOG1_MAC("Source Port :%d\n",rxParams.portNum);
    PRV_UTF_LOG1_MAC("Packet is Trapped to  CPU with CPU Code  :%d\n",rxParams.cpuCode);

    replicationType = CPSS_DXCH_CFG_TRAP_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    PRV_UTF_LOG1_MAC("Number of Replicated packets :%d\n",counterVal);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount), counterVal, "cpssDxChCfgReplicationCountersGet");

    replicationType = CPSS_DXCH_CFG_OUTGOING_FORWARD_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 2), counterVal, "cpssDxChCfgReplicationCountersGet");
}

GT_VOID prvTgfPclEgressTrapMirrorCfgRestore
(
    GT_VOID
)
{
    GT_STATUS                         rc,rc1 = GT_OK;;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disables egress policy on port 2 */
    rc = prvTgfPclEgressPclPacketTypesSet(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_EPCL_RECEIVE_PORT_INDEX_CNS],
        PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_STD_E, PRV_TGF_EPCL_RULE_INDEX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    rc = prvTgfFdbDefCfgRestore();
    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}

GT_VOID prvTgfEpclDropPacketToErepTrapTestRestore
(
    GT_VOID
)
{
    GT_STATUS                         rc,rc1 = GT_OK;;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_10_BYTES_E, PRV_TGF_EPCL_RULE_INDEX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    rc = prvTgfMirrorOnDropConfigSet(&prvTgfRestoreCfg.prvTgfMirrorOnDropEntryGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorOnDropConfigSet\n");

    prvTgfPclPortsRestoreAll();

    rc = prvTgfFdbDefCfgRestore();
    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}

/**
* @internal prvTgfPclEgressDropCfgSet function
* @endinternal
*
* @brief  Configure Egress PCL Rule with:
*         action pkt cmd : CPSS_PACKET_CMD_DROP_SOFT_E
*         cpu code       : CPSS_NET_IP_HDR_ERROR_E
*/
GT_VOID prvTgfPclEgressDropCfgSet(
    IN CPSS_PACKET_CMD_ENT packetCommand
)
{
    test_pktCmd = packetCommand;
    prvTgfPclEgressTrapMirrorCfgSet();
}
/**
* @internal prvTgfPclEgressTrapMirrorGenerate function
* @endinternal
*
* @brief  Configure to send packet to port 1
*/
GT_VOID prvTgfPclEgressDropTrafficGenerate
(
    GT_VOID
)
{
    /* check drop */
    prvTgfPclEgressTrapMirrorTrafficGenerate();

}
GT_VOID prvTgfPclEgressDropCfgRestore
(
    GT_VOID
)
{
    prvTgfPclEgressTrapMirrorCfgRestore();
    /* restore to default before the next test */
    test_pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
}

/**
* @internal prvTgfEpclTrapIngressTrapPacketConfig function
* @endinternal
*
* @brief Configuration for EPCL Trap Ingress Trap packet test
*
*/
static GT_VOID prvTgfIEpclConfig
(
    GT_VOID
)
{
    GT_STATUS                               rc;
    GT_U32                                  ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT             mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT             pattern;
    PRV_TGF_PCL_ACTION_STC                  action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT        ruleFormat;
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC    cpuCodeEntry;
    CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC      rxCounters;
    PRV_TGF_PCL_UDB_SELECT_STC              udbSelect;
    GT_U32                                  udbSelectidx;
    PRV_TGF_PCL_LOOKUP_CFG_STC              lookupCfg; /* PCL Configuration Table Entry */
    CPSS_INTERFACE_INFO_STC                 interfaceInfo;

    rc = prvTgfEpclReTrapFdbDefCfgSet();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEpclReTrapFdbDefCfgSet");

    /* PCL Configuration */

    /* init PCL Engine for port 0 */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_NUMBER_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* set PCL rule 0 - */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));

    ruleIndex                                                = 0;
    ruleFormat                                               = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    action.pktCmd                                            = test_pktCmd;
    action.mirror.cpuCode                                    = CPSS_NET_IP_HDR_ERROR_E;
    action.redirect.redirectCmd                              = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type             = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS];

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);

    /* save input config parametrs for traffic test and restore */
    prvTgfRestoreCfg.tcIngressTrapQueueGet = 2;

    /* save CPU code table entry for cpu code CPSS_NET_IP_HDR_ERROR_E */
    rc = prvTgfNetIfCpuCodeTableGet(prvTgfDevNum, CPSS_NET_IP_HDR_ERROR_E, &prvTgfRestoreCfg.ingressTrapCpuEntryGet);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("prvTgfNetIfCpuCodeTableGet failed, device %d cpuCode %d\n", prvTgfDevNum, CPSS_NET_IP_HDR_ERROR_E);
    }

    /* set tcQueue for cpuCode CPSS_NET_IP_HDR_ERROR_E  */
    cpssOsMemSet(&cpuCodeEntry, 0, sizeof(cpuCodeEntry));
    cpuCodeEntry.tc = 2;
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum, CPSS_NET_IP_HDR_ERROR_E, &cpuCodeEntry);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("prvTgfNetIfCpuCodeTableSet failed, device %d cpuCode %d\n", prvTgfDevNum, CPSS_NET_IP_HDR_ERROR_E);
    }

    /* clear the queue counters */
    rc = cpssDxChNetIfSdmaRxCountersGet(prvTgfDevNum, cpuCodeEntry.tc, &rxCounters);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("cpssDxChNetIfSdmaRxCountersGet failed, device %d tcQueue %d\n", prvTgfDevNum, cpuCodeEntry.tc);
    }

     /* AUTODOC: Enables egress policy per device */
    rc = prvTgfPclEgressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPolicyEnable");

    /* AUTODOC: save CPU's EPCL settings */
    rc = prvTgfPclEgressPclPacketTypesGet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS,
                    PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E, &prvTgfRestoreCfg.prvTgfSavedCpuEpclEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesGet");

    /* AUTODOC: Enables egress policy on port per packet type*/
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS,
                                          CPSS_DXCH_PCL_EGRESS_PKT_TO_CPU_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Save access mode for EPCL lookup for restore */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(CPSS_CPU_PORT_NUM_CNS,
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           &prvTgfRestoreCfg.prvTgfSavedCpuEpclAccessMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclPortLookupCfgTabAccessModeGet");
    /* write an entry into the EPCL configuration table */
    interfaceInfo.type = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum = prvTgfDevNum;
    interfaceInfo.devPort.portNum = CPSS_CPU_PORT_NUM_CNS;

    /* AUTODOC: setup EPCL configuration table entries. */
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = EPCL_MATCH_INDEX_CNS;
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;

    rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
                            CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(CPSS_CPU_PORT_NUM_CNS,
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    cpssOsMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    while(1)
    {
        if(prvTgfEpclMetadataUdbInfo[udbSelectidx].offsetType == CPSS_DXCH_PCL_OFFSET_INVALID_E)
            break;

        /* AUTODOC:   configure User Defined Byte(UDB) */
        /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
        /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E */
        /* AUTODOC:   offsetType CPSS_DXCH_PCL_OFFSET_METADATA_E */
        /* AUTODOC:   offset 53 Rx Sniff[0] */
        /* AUTODOC:   offset 64 Egress Mtag Cmd[1:0] */
        /* AUTODOC:   offset 64 Egress Marvell Tagged[2] */
        /* AUTODOC:   offset 64 Egress Marvell Tagged Extended[4:3] */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                        CPSS_PCL_DIRECTION_EGRESS_E,
                                        prvTgfEpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                        prvTgfEpclMetadataUdbInfo[udbSelectidx].offsetType,
                                        prvTgfEpclMetadataUdbInfo[udbSelectidx].offset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

        udbSelect.udbSelectArr[udbSelectidx] = prvTgfEpclMetadataUdbInfo[udbSelectidx].udbIndex;
        udbSelectidx++;
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E, PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                           CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* bit 0 Anchor Offset 53 */
    pattern.ruleEgrUdbOnly.udb[0] = 0x0 /* Src Trg/Rx Sniff [0x0 = Ingress Trapped] */;
    mask.ruleEgrUdbOnly.udb[0] = prvTgfEpclMetadataUdbInfo[0].byteMask;

    /* bits 4:0 Anchor Offset 64 */
    pattern.ruleEgrUdbOnly.udb[1] =  0x0        /* Egress Mtag Cmd [0x0 = TO_CPU] */ |
                                    (0x1 << 2)  /* Egress Marvell Tagged */  |
                                    (0x3  << 3) /* Egress Marvell Tagged Extended [0x3 = 16B eDSA] */;
    mask.ruleEgrUdbOnly.udb[1] = prvTgfEpclMetadataUdbInfo[1].byteMask;

    /* AUTODOC: EPCL action - trap the packet */
    action.egressPolicy                          = GT_TRUE;
    action.pktCmd                                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    action.mirror.cpuCode                        = CPSS_NET_USER_DEFINED_0_E;
    action.epclPhaInfo.phaThreadIdAssignmentMode = CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadId               = PRV_TGF_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E;

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            EPCL_MATCH_INDEX_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 EPCL_MATCH_INDEX_CNS);

    /* set tcQueue for cpuCode CPSS_NET_INTERVENTION_PORT_LOCK_E to validate packet exception based on new MAC SA */
    cpssOsMemSet(&cpuCodeEntry, 0, sizeof(cpuCodeEntry));
    cpuCodeEntry.tc = 4;
    prvTgfRestoreCfg.tcEgressTrapQueueGet = 4;

    rc = prvTgfNetIfCpuCodeTableGet(prvTgfDevNum, CPSS_NET_USER_DEFINED_0_E, &prvTgfRestoreCfg.egressTrapCpuEntryGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNetIfCpuCodeTableSet failed, device %d cpuCode %d\n", prvTgfDevNum, CPSS_NET_FIRST_USER_DEFINED_E);

    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum, CPSS_NET_USER_DEFINED_0_E, &cpuCodeEntry);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNetIfCpuCodeTableSet failed, device %d cpuCode %d\n", prvTgfDevNum, CPSS_NET_FIRST_USER_DEFINED_E);

    /* clear the queue counters */
    rc = cpssDxChNetIfSdmaRxCountersGet(prvTgfDevNum, cpuCodeEntry.tc, &rxCounters);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChNetIfSdmaRxCountersGet failed, device %d tcQueue %d\n", prvTgfDevNum, cpuCodeEntry.tc);
}

static GT_VOID prvTgfPhaConfig
(
    GT_VOID
)
{
    CPSS_DXCH_PHA_THREAD_INFO_UNT           extInfo;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC    commonInfo;
    GT_STATUS                               rc = GT_OK;

    /* GM does not support PHA */
    if(GT_TRUE == prvUtfIsGmCompilation())
        return;

    /* AUTODOC: Assign PHA firmware image ID 02 */
    rc = cpssDxChPhaFwImageIdGet(prvTgfDevNum, &(prvTgfRestoreCfg.phaFwImageIdGet));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaFwImageIdGet FAILED with rc = [%d]", rc);

    rc = cpssDxChPhaInit(prvTgfDevNum, GT_FALSE, CPSS_DXCH_PHA_FW_IMAGE_ID_02_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaInit FAILED for phaFwImageId = [%d], rc = [%d]", CPSS_DXCH_PHA_FW_IMAGE_ID_02_E, rc);

    cpssOsMemSet(&extInfo, 0, sizeof(extInfo)); /* No template */

    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    commonInfo.statisticalProcessingFactor  = 0;
    commonInfo.busyStallMode                = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode                = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    /* AUTODOC: Set the thread entry */
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum, PRV_TGF_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E, &commonInfo,
                                     CPSS_DXCH_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E, &extInfo);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,"[TGF]: cpssDxChPhaThreadIdEntrySet FAILED, phaThreadId = [%d], extType = [%d], rc = [%d]",
    PRV_TGF_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E, CPSS_DXCH_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E, rc);

}

/**
* @internal prvTgfEpclReTrapPacketConfig function
* @endinternal
*
* @brief Configuration for EPCL ReTrap packet test
*
*/
GT_VOID prvTgfEpclReTrapPacketConfig
(
    GT_VOID
)
{
    /* Bridge/I/E PCL Config */
    prvTgfIEpclConfig();

    /* PHA - Attach PHA Thread as per EPCL action */
    prvTgfPhaConfig();

}

/**
* @internal prvTgfEpclReTrapPacketTrafficGenerate function
* @endinternal
*
* @brief Traffic generate and verify for EPCL ReTrap test
*
*/
GT_VOID prvTgfEpclReTrapPacketTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                                   rc  = GT_OK;
    TGF_VFD_INFO_STC                            vfdArray[1];
    CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT replicationType;
    GT_U32                                      counterVal;
    GT_U8                                       packetBufPtr[64] = {0};
    GT_U32                                      packetBufLen = 64;
    GT_U32                                      packetLen;
    GT_U8                                       devNum;
    GT_U8                                       queueCpu;
    TGF_NET_DSA_STC                             rxParams;
    CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC          rxCounters;
    GT_U32                                      trapPktCount = 0;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* Reset the port Counter */
    rc = prvTgfCountersEthReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCountersEthReset");

    /* clear the counter before sending packet*/
    replicationType = CPSS_DXCH_CFG_TRAP_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");

    replicationType = CPSS_DXCH_CFG_OUTGOING_FORWARD_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");

    /* Start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* Generate traffic */
    rc = prvTgfSetTxSetupEth(
            prvTgfDevNum, &prvTgfIpPacketInfo,
            prvTgfTrapPacketBurstCount, 0 , NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfSetTxSetupEth");

    /* Send packet to port 0 */
    rc = prvTgfStartTransmitingEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth");

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    PRV_UTF_LOG0_MAC("check the CPU port capturing:\n");

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_TRUE, GT_TRUE,
                                       packetBufPtr,
                                       &packetBufLen,
                                       &packetLen,
                                       &devNum,
                                       &queueCpu,
                                       &rxParams);

    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
    UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_NET_USER_DEFINED_0_E,
                                rxParams.cpuCode);

    replicationType = CPSS_DXCH_CFG_TRAP_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfTrapPacketBurstCount, counterVal, "cpssDxChCfgReplicationCountersGet");

    replicationType = CPSS_DXCH_CFG_OUTGOING_FORWARD_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfTrapPacketBurstCount * 2), counterVal, "cpssDxChCfgReplicationCountersGet");

    /* Verify packet should reach in case of enabled queue*/
    rc = cpssDxChNetIfSdmaRxCountersGet(prvTgfDevNum, prvTgfRestoreCfg.tcIngressTrapQueueGet, &rxCounters);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("cpssDxChNetIfSdmaRxCountersGet failed, device %d tcQueue %d\n", prvTgfDevNum, prvTgfRestoreCfg.tcIngressTrapQueueGet);
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(trapPktCount, rxCounters.rxInPkts, "cpssDxChCfgReplicationCountersGet");

    rc = cpssDxChNetIfSdmaRxCountersGet(prvTgfDevNum, prvTgfRestoreCfg.tcEgressTrapQueueGet, &rxCounters);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("cpssDxChNetIfSdmaRxCountersGet failed, device %d tcQueue %d\n", prvTgfDevNum, prvTgfRestoreCfg.tcEgressTrapQueueGet);
    }
    trapPktCount = prvTgfTrapPacketBurstCount;
    UTF_VERIFY_EQUAL0_STRING_MAC(trapPktCount, rxCounters.rxInPkts, "cpssDxChCfgReplicationCountersGet");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfEpclReTrapPacketRestore function
* @endinternal
*
* @brief Restore for EPCL ReTrap test
*
*/
GT_VOID prvTgfEpclReTrapPacketRestore
(
    GT_VOID
)
{
    GT_STATUS                         rc = GT_OK;
    GT_U32                            portIter;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("tgfTrafficTableRxPcktTblClear failed for device %d\n", prvTgfDevNum);
    }

    /* AUTODOC: Disable RX capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCommonAllCntrsReset");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    /* Vlan restore */
    /* Restore default PVID on port 1 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS], prvTgfDefVlanId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* restore CPU code table entry for cpu code CPSS_NET_IP_HDR_ERROR_E */
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum, CPSS_NET_IP_HDR_ERROR_E, &prvTgfRestoreCfg.ingressTrapCpuEntryGet);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("prvTgfNetIfCpuCodeTableSet failed, device %d cpuCode %d\n", prvTgfDevNum, CPSS_NET_IP_HDR_ERROR_E);
    }
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum, CPSS_NET_USER_DEFINED_0_E, &prvTgfRestoreCfg.egressTrapCpuEntryGet);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("prvTgfNetIfCpuCodeTableSet failed, device %d cpuCode %d\n", prvTgfDevNum, CPSS_NET_USER_DEFINED_0_E);
    }

    /* AUTODOC: Restore PHA Configuration */
    rc = cpssDxChPhaInit(prvTgfDevNum, GT_FALSE, prvTgfRestoreCfg.phaFwImageIdGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaInit FAILED for phaFwImageId = [%d], rc = [%d]", prvTgfRestoreCfg.phaFwImageIdGet, rc);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfBrgFdbFlush failed for device %d port %d\n", prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    /* AUTODOC: invalidate I/EPCL rule */
    rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_10_BYTES_E, EPCL_MATCH_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet");

    rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_STD_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet");

    /* AUTODOC: Restore egress policy on port per packet type*/
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS,
                                          CPSS_DXCH_PCL_EGRESS_PKT_TO_CPU_E, prvTgfRestoreCfg.prvTgfSavedCpuEpclEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Restore access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(CPSS_CPU_PORT_NUM_CNS,
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           prvTgfRestoreCfg.prvTgfSavedCpuEpclAccessMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: Disables egress policy per device */
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPolicyEnable");

    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable");

}
