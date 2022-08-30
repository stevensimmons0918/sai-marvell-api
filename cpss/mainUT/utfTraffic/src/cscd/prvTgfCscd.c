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
* @file prvTgfCscd.c
*
* @brief CPSS Cascading
*
* @version   15
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCscdGen.h>
#include <cscd/prvTgfCscd.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  5

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS   1

/* index of local egress port */
#define LOCAL_EGRESS_PORT_INDEX_CNS 2

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* number of packets to send for the test */
#define DEV_MAP_TABLE_TEST_MAX_PKT_CNS             2

/* number of ports used for device map table access based tests */
#define MAX_PORT_NUMBER_DEV_MAP_LOOKUP_TEST 4

/* Cascade link indexes used for device map table entry */
#define CASCADE_LINK_NUMBER1_CNS    1
#define CASCADE_LINK_NUMBER2_CNS    2

#define TARGET_EPORT_NUM_CNS    ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(0x1ACB)

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x56},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};


/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x81, 0x00, 0x05, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketExpectedDsaPart_forward = {
    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E /*GT_TRUE*/ , /*srcIsTagged*/ /* set to GT_TRUE for the copy to CPU , to be with vlan tag 0 */
    0,/*srcHwDev --> filled at runtime*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        0/*portNum --> filled at runtime*/
    },/*source;*/

    0,/*srcId */

    GT_TRUE,/*egrFilterRegistered - the mac DA was found in the FDB */
    GT_FALSE,/*wasRouted*/
    0,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            0,/*devNum*/
            0/*portNum --> set at runtime*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        0,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPhyPortValid*/
    0,/*dstEport --> filled in runtime */
    0,/*tag0TpidIndex  --> filled at runtime*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /* union */
    {
        /*trunkId*/
        0/*portNum --> filled at runtime*/
    },/*origSrcPhy*/
    GT_FALSE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_TRUE /*skipFdbSaLookup*/

};

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketExpectedDsaPart = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_2_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,        /*vpt*/
        0,        /*cfiBit*/
        PRV_TGF_VLANID_CNS, /*vid*/
        GT_FALSE, /*dropOnSource*/
        GT_FALSE  /*packetIsLooped*/
    },/*commonParams*/


    {
        {
            GT_FALSE, /* isEgressPipe */
            GT_FALSE, /* isTagged */
            0,        /* hwDevNum */
            GT_FALSE, /* srcIsTrunk */
            {
                0, /* srcTrunkId */
                0, /* portNum */
                0  /* ePort */
            },        /* interface */
            0,        /* cpuCode */
            GT_FALSE, /* wasTruncated */
            0,        /* originByteCount */
            0,        /* timestamp */
            GT_FALSE, /* packetIsTT */
            {
                0 /* flowId */
            },        /* flowIdTtOffset */
            0
        } /* TGF_DSA_DSA_TO_CPU_STC */

    }/* dsaInfo */
};

/* PARTS of the expected packet that we receive with the DSA tag */
static TGF_PACKET_PART_STC prvTgfPacketPartArray_capturedWithDsa[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_DSA_TAG_E,  &prvTgfPacketExpectedDsaPart},/*DSA tag instead of vlan tag*/
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};
/* the expected packet that we receive with the DSA tag */
static TGF_PACKET_STC prvTgfPacketInfo_capturedWithDsa =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                             /* totalLen */
    sizeof(prvTgfPacketPartArray_capturedWithDsa) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray_capturedWithDsa                                        /* partsArray */
};

/* cascaded link number */
#define CSCD_LINK_IDX         3

/* cascaded link number */
#define CSCD_LINK_IDX_ANOTHER 0

/* target dev/port array size */
#define PRV_TGF_CSCD_TRG_ARRAY_SIZE_CNS 3

/* target device number array*/
static GT_U8 targetDevNumArr[PRV_TGF_CSCD_TRG_ARRAY_SIZE_CNS] = {0/*set in runtime to prvTgfDevNum*/, 13, 31};
static GT_BOOL targetDevNumArrChanged = GT_FALSE;
static GT_U8 origTargetDevNum = 31;

/* target device number array*/
static GT_U8 targetPortNumArr[PRV_TGF_CSCD_TRG_ARRAY_SIZE_CNS] = {2, 26, 56};

/* FDB MAC entry */
static PRV_TGF_BRG_MAC_ENTRY_STC prvTgfMacEntry;


/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_DEV_MAP_LOOKUP_MODE_ENT     lookupMode;
    CPSS_CSCD_LINK_TYPE_STC             cascadeLinkArr[PRV_TGF_CSCD_TRG_ARRAY_SIZE_CNS];
    PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT    srcPortTrunkHashEnArr[PRV_TGF_CSCD_TRG_ARRAY_SIZE_CNS];
    GT_U32                              portsArray[PRV_TGF_MAX_PORTS_NUM_CNS];
} prvTgfRestoreCfg;

/* 'Remote device1' number */
#define REMOTE_DEVICE1_NUMBER_CNS          2

/* 'Remote device2' number */
#define REMOTE_DEVICE2_NUMBER_CNS          4

/* 'Remote device1' port number */
#define REMOTE_DEVICE1_PORT_NUM_CNS       30

/* 'Remote device2' port number */
#define REMOTE_DEVICE2_PORT_NUM_CNS       40

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfCscdTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number to send traffic from
* @param[in] packetInfoPtr            - PACKET to send
*                                       None
*/
static GT_VOID prvTgfCscdTestPacketSend
(
    IN GT_U32           portNum,
    IN TGF_PACKET_STC *packetInfoPtr
)
{
    GT_STATUS       rc = GT_OK;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* AUTODOC: send Ethernet packet from port 1 with: */
    /* AUTODOC:   DA=00:00:00:00:00:56, SA=00:00:00:00:00:02, VID=5 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, portNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfCscdSingleTargetDestinationConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set FDB entry with device number 0, port 1, MAC 00:00:00:00: 00:56.
*         -    Set lookup mode to Port for accessing the Device Map table.
*         -    Set the cascade map table
*         - Target device 0, target port 1, link type port, link number 23.
*/
GT_VOID prvTgfCscdSingleTargetDestinationConfigurationSet
(
    GT_VOID
)
{
    CPSS_CSCD_LINK_TYPE_STC   cascadeLink;          /* cascade link */
    GT_STATUS                 rc = GT_OK;

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(prvTgfRestoreCfg.portsArray, prvTgfPortsArray,
                 sizeof(prvTgfRestoreCfg.portsArray));

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   CSCD_LINK_IDX,
                                   CSCD_LINK_IDX_ANOTHER,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    /* AUTODOC: configure TPID profile 7 to not match any etherType and use it for capturing */
    rc = prvTgfCaptureEnableUnmatchedTpidProfile(prvTgfDevNum, GT_TRUE, 7);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCaptureEnableUnmatchedTpidProfile");


    targetDevNumArr[0] = prvTgfDevNum;

    /* reset ethernet counters + link up on the tested ports */
    /* Clear capturing RxPcktTable */
    prvTgfEthCountersReset(prvTgfDevNum);

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    /* clear entry */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    /* Set FDB entry with device number 31, port 27,  MAC 00:00:00:00: 00:56 */
    prvTgfMacEntry.key.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;
    prvTgfMacEntry.isStatic = GT_TRUE;
    prvTgfMacEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    prvTgfMacEntry.dstInterface.devPort.hwDevNum = targetDevNumArr[0];
    prvTgfMacEntry.dstInterface.devPort.portNum = targetPortNumArr[0];
    cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:56, VLAN 5, portNum 2 */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    /* save the current lookup mode */
    rc = prvTgfCscdDevMapLookupModeGet(prvTgfDevNum, &(prvTgfRestoreCfg.lookupMode));
    if (rc == GT_NOT_APPLICABLE_DEVICE)
    {
        /* such device support only TRG_DEV mode */
        prvTgfRestoreCfg.lookupMode = PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_E;
    }
    else
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeGet: %d", prvTgfDevNum);

    /* AUTODOC: set TRG_DEV_TRG_PORT lookup mode for accessing Device Map table */
    rc = prvTgfCscdDevMapLookupModeSet(PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeSet: %d",
                                 PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E);

    /* save the current cascade map table */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, targetDevNumArr[0], targetPortNumArr[0], 0,
                                  &(prvTgfRestoreCfg.cascadeLinkArr[0]), &(prvTgfRestoreCfg.srcPortTrunkHashEnArr[0]));
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet: %d,%d,%d",
                                 prvTgfDevNum, targetDevNumArr[0], targetPortNumArr[0]);

    /* Set the cascade map table */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    cascadeLink.linkNum = prvTgfPortsArray[CSCD_LINK_IDX];

    /* AUTODOC: config CSCD map table with: */
    /* AUTODOC:   trgDevNum= prvTgfDevNum, trgPortNum=2, */
    /* AUTODOC:   cscdLink linkType=PORT, linkNum=3 */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, targetDevNumArr[0], targetPortNumArr[0], 0,
                                  &cascadeLink, 0, GT_FALSE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeSet: %d%d%d%d",
                                 targetDevNumArr[0], targetPortNumArr[0], CPSS_CSCD_LINK_TYPE_PORT_E, prvTgfPortsArray[CSCD_LINK_IDX]);
}

/**
* @internal prvTgfCscdSingleTargetDestinationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         No packets are captured.
*         Set FDB entry with device number 13, port 27, MAC 00:00:00:00: 00:56.
*         Set the cascade map table
*         - Target device 13, target port 27, link type port, link number 23.
*         Send to device's port 8 packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 23.
*         Set FDB entry with device number 31, port 58, MAC 00:00:00:00: 00:56.
*         Set the cascade map table
*         - Target device 31, target port 58, link type port, link number 0.
*         Send to device's port 8 packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0.
*/
GT_VOID prvTgfCscdSingleTargetDestinationTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    CPSS_CSCD_LINK_TYPE_STC   cascadeLink;          /* cascade link */
    GT_HW_DEV_NUM               hwDevNum;

    /* get HW device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* send packet */
    prvTgfCscdTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify to get no traffic */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {

            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* No port  received packet */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
    }

    prvTgfMacEntry.dstInterface.devPort.hwDevNum = targetDevNumArr[1];
    prvTgfMacEntry.dstInterface.devPort.portNum = targetPortNumArr[1];

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:56, VLAN 5, portNum 26 */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    /* save the current cascade map table */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, targetDevNumArr[1], targetPortNumArr[1], 0,
                                  &(prvTgfRestoreCfg.cascadeLinkArr[1]), &(prvTgfRestoreCfg.srcPortTrunkHashEnArr[1]));
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet: %d,%d,%d",
                                 prvTgfDevNum, targetDevNumArr[1], targetPortNumArr[1]);

    /* Set the cascade map table */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    cascadeLink.linkNum = prvTgfPortsArray[CSCD_LINK_IDX];

    /* AUTODOC: config CSCD map table with: */
    /* AUTODOC:   trgDevNum=13, trgPortNum=26, */
    /* AUTODOC:   cscdLink linkType=PORT, linkNum=3 */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, targetDevNumArr[1], targetPortNumArr[1], 0,
                                  &cascadeLink, 0, GT_FALSE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeSet: %d%d%d%d",
                                 targetDevNumArr[1], targetPortNumArr[1], CPSS_CSCD_LINK_TYPE_PORT_E, prvTgfPortsArray[CSCD_LINK_IDX]);


    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* send packet */
    prvTgfCscdTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify 1 packet on port 3 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {

            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 23 received 1 packet */
        if (portIter == CSCD_LINK_IDX)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: set TRG_DEV lookup mode for accessing Device Map table */
    rc = prvTgfCscdDevMapLookupModeSet(PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeSet: %d",
                                 PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_E);

    /*the device map table is not accessed for LOCAL DEVICE traffic*/
    if ( (hwDevNum == (targetDevNumArr[2])) || (prvTgfDevNum == targetDevNumArr[2]) )
    {
        targetDevNumArrChanged = GT_TRUE;
        targetDevNumArr[2] -= 1;
    }
    prvTgfMacEntry.dstInterface.devPort.hwDevNum = targetDevNumArr[2];
    prvTgfMacEntry.dstInterface.devPort.portNum = targetPortNumArr[2];

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:56, VLAN 5, portNum 56 */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    /* save the current cascade map table */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, targetDevNumArr[2], targetPortNumArr[2], 0,
                                  &(prvTgfRestoreCfg.cascadeLinkArr[2]), &(prvTgfRestoreCfg.srcPortTrunkHashEnArr[2]));
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet: %d,%d,%d",
                                 prvTgfDevNum, targetDevNumArr[2], targetPortNumArr[2]);

    /* Set the cascade map table */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    cascadeLink.linkNum = prvTgfPortsArray[CSCD_LINK_IDX_ANOTHER];

    /* AUTODOC: config CSCD map table with: */
    /* AUTODOC:   trgDevNum=31, trgPortNum=56, */
    /* AUTODOC:   cscdLink linkType=PORT, linkNum=0 */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, targetDevNumArr[2], targetPortNumArr[2], 0,
                                  &cascadeLink, 0, GT_FALSE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeSet: %d%d%d%d",
                                 targetDevNumArr[2], targetPortNumArr[2],
                                 CPSS_CSCD_LINK_TYPE_PORT_E, prvTgfPortsArray[CSCD_LINK_IDX_ANOTHER]);

    /* send packet */
    prvTgfCscdTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify 1 packet on port 0 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {

            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 0 received 1 packet */
        if (portIter == CSCD_LINK_IDX_ANOTHER)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfCscdSingleTargetDestinationConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCscdSingleTargetDestinationConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: restore default CSCD map table */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, targetDevNumArr[2], targetPortNumArr[2], 0,
                                  &(prvTgfRestoreCfg.cascadeLinkArr[2]),
                                  prvTgfRestoreCfg.srcPortTrunkHashEnArr[2], GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet: %d,%d,%d",
                                 prvTgfDevNum, targetDevNumArr[2], targetPortNumArr[2]);

    /* AUTODOC: restore default targetDevNumArr */
    if (targetDevNumArrChanged)
    {
        targetDevNumArr[2] = origTargetDevNum;
    }

    /* restore cascade map table */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, targetDevNumArr[1], targetPortNumArr[1], 0,
                                  &(prvTgfRestoreCfg.cascadeLinkArr[1]),
                                  prvTgfRestoreCfg.srcPortTrunkHashEnArr[1], GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet: %d,%d,%d",
                                 prvTgfDevNum, targetDevNumArr[1], targetPortNumArr[1]);

    /* restore cascade map table */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, targetDevNumArr[0], targetPortNumArr[0], 0,
                                  &(prvTgfRestoreCfg.cascadeLinkArr[0]),
                                  prvTgfRestoreCfg.srcPortTrunkHashEnArr[0], GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet: %d,%d,%d",
                                 prvTgfDevNum, targetDevNumArr[0], targetPortNumArr[0]);

    /* AUTODOC: restore default lookup mode for accessing Device Map table */
    rc = prvTgfCscdDevMapLookupModeSet(prvTgfRestoreCfg.lookupMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeSet: %d",
                                 prvTgfRestoreCfg.lookupMode);

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfRestoreCfg.portsArray,
                 sizeof(prvTgfRestoreCfg.portsArray));

    /* AUTODOC: restore content of TPID profile 7 */
    rc = prvTgfCaptureEnableUnmatchedTpidProfile(prvTgfDevNum, GT_FALSE, 7);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCaptureEnableUnmatchedTpidProfile");

}

/**
* @internal prvTgfCscdSingleTargetDestinationLocalDeviceConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set FDB entry with device number 0, port 18, MAC 00:00:00:00: 00:56.
*         -    Set lookup mode to Port for accessing the Device Map table.
*         -    Set the cascade map table
*         - Target device 0, target port 0, link type port, link number 23.
*/
GT_VOID prvTgfCscdSingleTargetDestinationLocalDeviceConfigurationSet
(
    GT_VOID
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC prvTgfMacEntry;       /* FDB MAC entry */
    CPSS_CSCD_LINK_TYPE_STC   cascadeLink;          /* cascade link */
    GT_STATUS                 rc = GT_OK;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(prvTgfRestoreCfg.portsArray, prvTgfPortsArray,
                 sizeof(prvTgfRestoreCfg.portsArray));

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfDefPortsArrange(GT_FALSE, CSCD_LINK_IDX, -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    targetDevNumArr[0] = prvTgfDevNum;

    /* clear entry */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    /* Set FDB entry with device number 0, port 18,  MAC 00:00:00:00: 00:56 */
    prvTgfMacEntry.key.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;
    prvTgfMacEntry.isStatic = GT_TRUE;
    prvTgfMacEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    prvTgfMacEntry.dstInterface.hwDevNum = prvTgfDevNum;
    prvTgfMacEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    prvTgfMacEntry.dstInterface.devPort.portNum = prvTgfPortsArray[LOCAL_EGRESS_PORT_INDEX_CNS];
    cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:56, VLAN 5, port 2 */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    /* save the current lookup mode */
    rc = prvTgfCscdDevMapLookupModeGet(prvTgfDevNum, &(prvTgfRestoreCfg.lookupMode));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeGet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: set TRG_DEV_TRG_PORT lookup mode for accessing Device Map table */
    rc = prvTgfCscdDevMapLookupModeSet(PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeSet: %d", PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E);

    /* save the current cascade map table */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, targetDevNumArr[0], prvTgfPortsArray[LOCAL_EGRESS_PORT_INDEX_CNS], 0,
                                  &(prvTgfRestoreCfg.cascadeLinkArr[0]), &(prvTgfRestoreCfg.srcPortTrunkHashEnArr[0]));
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet: %d,%d,%d",
                                 prvTgfDevNum, targetDevNumArr[0], prvTgfPortsArray[LOCAL_EGRESS_PORT_INDEX_CNS]);

    /* Set the cascade map table */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    cascadeLink.linkNum = prvTgfPortsArray[CSCD_LINK_IDX];

    /* AUTODOC: config CSCD map table with: */
    /* AUTODOC:   trgDevNum=0, trgPort=2, */
    /* AUTODOC:   cscdLink linkType=PORT, linkNum=3 */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, targetDevNumArr[0], prvTgfPortsArray[LOCAL_EGRESS_PORT_INDEX_CNS], 0,
                                  &cascadeLink, PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E, GT_FALSE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeSet: %d%d%d%d",
                                 0, prvTgfPortsArray[LOCAL_EGRESS_PORT_INDEX_CNS], CPSS_CSCD_LINK_TYPE_PORT_E, prvTgfPortsArray[CSCD_LINK_IDX]);
}

/**
* @internal prvTgfCscdSingleTargetDestinationLocalDeviceTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 18.
*         Enable the local target port for device map lookup for local device.
*         Send to device's port 8 packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 23.
*/
GT_VOID prvTgfCscdSingleTargetDestinationLocalDeviceTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* send packet */
    prvTgfCscdTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic on port 2 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {

            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 18 received 1 packet */
        if (portIter == LOCAL_EGRESS_PORT_INDEX_CNS)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: enable local ingress trg port 1 for device map lookup */
    rc = prvTgfCscdPortLocalDevMapLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                  CPSS_DIRECTION_INGRESS_E,
                                                  GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdPortLocalDevMapLookupEnableSet: %d%d%d",
                                                  prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                  CPSS_DIRECTION_INGRESS_E,
                                                  GT_TRUE);

    /* AUTODOC: enable local egress trg port 2 for device map lookup */
    rc = prvTgfCscdPortLocalDevMapLookupEnableSet(prvTgfPortsArray[LOCAL_EGRESS_PORT_INDEX_CNS],
                                                  CPSS_DIRECTION_EGRESS_E,
                                                  GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdPortLocalDevMapLookupEnableSet: %d%d%d",
                                                  prvTgfPortsArray[LOCAL_EGRESS_PORT_INDEX_CNS],
                                                  CPSS_DIRECTION_EGRESS_E,
                                                  GT_TRUE);

    /* send packet */
    prvTgfCscdTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic on port 3 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {

            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 23 received 1 packet */
        if (portIter == CSCD_LINK_IDX)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

}

/**
* @internal prvTgfCscdSingleTargetDestinationLocalDeviceConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCscdSingleTargetDestinationLocalDeviceConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disable local ingress trg port 1 for device map lookup */
    rc = prvTgfCscdPortLocalDevMapLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                  CPSS_DIRECTION_INGRESS_E,
                                                  GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdPortLocalDevMapLookupEnableSet: %d%d%d",
                                                  prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                  CPSS_DIRECTION_INGRESS_E,
                                                  GT_FALSE);

    /* AUTODOC: disable local egress trg port 2 for device map lookup */
    rc = prvTgfCscdPortLocalDevMapLookupEnableSet(prvTgfPortsArray[LOCAL_EGRESS_PORT_INDEX_CNS],
                                                  CPSS_DIRECTION_EGRESS_E,
                                                  GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdPortLocalDevMapLookupEnableSet: %d%d%d",
                                                  prvTgfPortsArray[LOCAL_EGRESS_PORT_INDEX_CNS],
                                                  CPSS_DIRECTION_EGRESS_E,
                                                  GT_FALSE);

    /* AUTODOC: restore default CSCD map table */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, targetDevNumArr[0],
                                  prvTgfPortsArray[LOCAL_EGRESS_PORT_INDEX_CNS], 0,
                                  &(prvTgfRestoreCfg.cascadeLinkArr[0]),
                                  prvTgfRestoreCfg.srcPortTrunkHashEnArr[0],
                                  GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet: %d,%d,%d",
                                 prvTgfDevNum, targetDevNumArr[0], prvTgfPortsArray[LOCAL_EGRESS_PORT_INDEX_CNS]);

    /* AUTODOC: restore default lookup mode for accessing Device Map table */
    rc = prvTgfCscdDevMapLookupModeSet(prvTgfRestoreCfg.lookupMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeSet: %d",
                                 prvTgfRestoreCfg.lookupMode);

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfRestoreCfg.portsArray,
                 sizeof(prvTgfRestoreCfg.portsArray));
}

/**
* @internal localEPortMapToRemotePhyPortAndDsaTagCheck_config function
* @endinternal
*
* @brief   test relevant only to E_ARCH devices
*         1. set fdb entry on eport (TARGET_EPORT_NUM_CNS) on local device
*         2. map by the e2Phy this eport to remote (dev,port) physical interface.
*         a. remote dev = targetDevNumArr[2] , remote port = targetPortNumArr[2]
*         3. Set the egress port as 'egress cascade port' with DSA of 2 words
*         (but as ingress 'not cascade port')
*/
static GT_VOID localEPortMapToRemotePhyPortAndDsaTagCheck_config(GT_VOID)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_CSCD_LINK_TYPE_STC   cascadeLink;          /* cascade link */
    GT_HW_DEV_NUM               hwDevNum;

    /* get HW device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet: %d",
                                 prvTgfDevNum);
    /*start with generic config */
    prvTgfCscdSingleTargetDestinationConfigurationSet();

    /*the device map table is not accessed for LOCAL DEVICE traffic*/
    if ( (hwDevNum == targetDevNumArr[2]) || (prvTgfDevNum == targetDevNumArr[2]) )
    {
        targetDevNumArrChanged = GT_TRUE;
        targetDevNumArr[2] -= 1;
    }

    /* the expected info on the DSA */
    prvTgfPacketExpectedDsaPart_forward.srcHwDev         = prvTgfDevNum;
    prvTgfPacketExpectedDsaPart_forward.source.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    prvTgfPacketExpectedDsaPart_forward.dstInterface.devPort.hwDevNum  = targetDevNumArr [2];
    prvTgfPacketExpectedDsaPart_forward.dstInterface.devPort.portNum = targetPortNumArr[2];

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup target portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[CSCD_LINK_IDX];

    /* enable capture on target port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);

    prvTgfMacEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    prvTgfMacEntry.dstInterface.devPort.portNum = TARGET_EPORT_NUM_CNS;

    /* AUTODOC: set FDB entry with MAC 00:00:00:00:00:56, VLAN 5, local device , eport TARGET_EPORT_NUM_CNS */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    /* AUTODOC: set the egress port (the cascade port) to allow capture the info with DSA */
    /* AUTODOC: since the port will be in loopback we set it only for egress to be cascade , but on ingress as 'non cascade' */
    rc = tgfTrafficGeneratorEgressCscdPortEnableSet(prvTgfDevNum,prvTgfPortsArray[CSCD_LINK_IDX],
            GT_TRUE,
            CPSS_CSCD_PORT_DSA_MODE_EXTEND_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    /* AUTODOC: set the e2phy for eport to remote physical device,port */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfPacketExpectedDsaPart_forward.dstInterface.devPort.hwDevNum ;
    portInterface.devPort.portNum = prvTgfPacketExpectedDsaPart_forward.dstInterface.devPort.portNum;
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           TARGET_EPORT_NUM_CNS,
                                                           &portInterface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d", prvTgfDevNum);


    /* AUTODOC: set TRG_DEV_TRG_PORT lookup mode for accessing Device Map table */
    rc = prvTgfCscdDevMapLookupModeSet(PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeSet: %d",
                                 PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E);


    /* save the current cascade map table */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, targetDevNumArr[2], targetPortNumArr[2], 0,
                                  &(prvTgfRestoreCfg.cascadeLinkArr[2]), &(prvTgfRestoreCfg.srcPortTrunkHashEnArr[2]));
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet: %d,%d,%d",
                                 prvTgfDevNum, targetDevNumArr[2], targetPortNumArr[2]);

    /* Set the cascade map table */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    cascadeLink.linkNum = prvTgfPortsArray[CSCD_LINK_IDX];

    /* AUTODOC: config CSCD map table with: */
    /* AUTODOC:   trgDevNum=31, trgPortNum= eport TARGET_EPORT_NUM_CNS, */
    /* AUTODOC:   cscdLink linkType=PORT, linkNum=0 */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, targetDevNumArr[2], TARGET_EPORT_NUM_CNS/*targetPortNumArr[2]*/, 0,
                                  &cascadeLink, 0, GT_FALSE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeSet: %d%d%d%d",
                                 targetDevNumArr[2], TARGET_EPORT_NUM_CNS/*targetPortNumArr[2]*/,
                                 CPSS_CSCD_LINK_TYPE_PORT_E, cascadeLink.linkNum);

}

/**
* @internal localEPortMapToRemotePhyPortAndDsaTagCheck_restore function
* @endinternal
*
* @brief   test relevant only to E_ARCH devices : restore
*/
static GT_VOID localEPortMapToRemotePhyPortAndDsaTagCheck_restore(GT_VOID)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;

    /* AUTODOC: restore default CSCD map table */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, targetDevNumArr[2], TARGET_EPORT_NUM_CNS/*targetPortNumArr[2]*/, 0,
                                  &(prvTgfRestoreCfg.cascadeLinkArr[2]),
                                  prvTgfRestoreCfg.srcPortTrunkHashEnArr[2], GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet: %d,%d,%d",
                                 prvTgfDevNum, targetDevNumArr[2], targetPortNumArr[2]);

    /* AUTODOC: restore default targetDevNumArr */
    if (targetDevNumArrChanged)
    {
        targetDevNumArr[2] = origTargetDevNum;
    }

    /* set the port as 'non cascade'*/
    rc = tgfTrafficGeneratorEgressCscdPortEnableSet(prvTgfDevNum,prvTgfPortsArray[CSCD_LINK_IDX],
            GT_FALSE,
            CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    /* setup target portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[CSCD_LINK_IDX];

    /* disable capture on target port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* do generic restore */
    prvTgfCscdSingleTargetDestinationConfigurationRestore();

}

/**
* @internal localEPortMapToRemotePhyPortAndDsaTagCheck_send function
* @endinternal
*
* @brief   test relevant only to E_ARCH devices : send traffic and check
*         4. send the traffic
*         5. trap the traffic that egress the port
*         6. make sure that trgPort in the DSA is targetPortNumArr[2] (and not eport)
*/
static GT_VOID localEPortMapToRemotePhyPortAndDsaTagCheck_send(GT_VOID)
{
    GT_STATUS    rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32       actualCapturedNumOfPackets;
    GT_U32       portIter;

    /* send packet */
    prvTgfCscdTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* do this right before the compare of expected packet */
    /* bind the DSA tag FORWARD part (since union is used .. can't do it in compilation time) */
    prvTgfPacketExpectedDsaPart.dsaInfo.forward = prvTgfPacketExpectedDsaPart_forward;

    /* AUTODOC: check the captured DSA tag */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[CSCD_LINK_IDX];

    /* AUTODOC: check the captured DSA tag within the packet */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            &prvTgfPacketInfo_capturedWithDsa,
            prvTgfBurstCount,/*numOfPackets*/
            0/*vfdNum*/,
            NULL /*vfdArray*/,
            NULL, /* bytesNum's skip list */
            0,    /* length of skip list */
            &actualCapturedNumOfPackets,
            NULL/*onFirstPacketNumTriggersBmpPtr*/);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);
}

/**
* @internal localEPortMapToRemotePhyPortAndDsaTagCheck function
* @endinternal
*
* @brief   test relevant only to E_ARCH devices
*         1. set fdb entry on eport (TARGET_EPORT_NUM_CNS) on local device
*         2. map by the e2Phy this eport to remote (dev,port) physical interface.
*         a. remote dev = targetDevNumArr[2] , remote port = targetPortNumArr[2]
*         3. Set the egress port as 'egress cascade port' with DSA of 2 words
*         (but as ingress 'not cascade port')
*         4. send the traffic
*         5. trap the traffic that egress the port
*         6. make sure that trgPort in the DSA is targetPortNumArr[2] (and not eport)
*/
GT_VOID localEPortMapToRemotePhyPortAndDsaTagCheck(GT_VOID)
{
    localEPortMapToRemotePhyPortAndDsaTagCheck_config();
    localEPortMapToRemotePhyPortAndDsaTagCheck_send();
    localEPortMapToRemotePhyPortAndDsaTagCheck_restore();
}


/**
* @internal tgfTrunkDeviceMapTableTargetPortModeLookupTest function
* @endinternal
*
* @brief   Target port based device map table lookup test
*/
GT_VOID tgfTrunkDeviceMapTableTargetPortModeLookupTest
(
    GT_VOID
)
{
    GT_STATUS                                     rc;/*return code*/
    GT_U32                                        ii, jj;
    GT_U8                                         portIter;
    GT_U8                                         pktIter;
    CPSS_CSCD_LINK_TYPE_STC                       cascadeLink; /* cascade link info */
    PRV_TGF_BRG_MAC_ENTRY_STC                     macEntry1 = PRV_TGF_BRG_FDB_MAC_ENTRY_DEFAULT_MAC;
    PRV_TGF_BRG_MAC_ENTRY_STC                     macEntry2 = PRV_TGF_BRG_FDB_MAC_ENTRY_DEFAULT_MAC;
    static GT_U32                                 devTableArr[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS];
    CPSS_PORT_MAC_COUNTER_SET_STC                 portCntrs;
    CPSS_PORTS_BMP_STC                            portsVlanMembers = {{0, 0}};
    GT_U32                                        portCounters[MAX_PORT_NUMBER_DEV_MAP_LOOKUP_TEST];

    cpssOsMemSet(&portCounters, 0, sizeof(portCounters));
    /* add the remote device to the FDB device table , so FDB entries on current
       device which are registered on the remote device will not be deleted by
       the aging daemon */
    rc = prvTgfBrgFdbDeviceTableGet(devTableArr);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    devTableArr[0] |= (1 << REMOTE_DEVICE1_NUMBER_CNS);

    rc = prvTgfBrgFdbDeviceTableSet(devTableArr);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    devTableArr[0] |= (1 << REMOTE_DEVICE2_NUMBER_CNS);
    rc = prvTgfBrgFdbDeviceTableSet(devTableArr);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc =  prvTgfCscdDevMapLookupModeGet(prvTgfDevNum,&prvTgfRestoreCfg.lookupMode);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    rc = prvTgfCscdDevMapLookupModeSet(PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_PORT_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* next tests need the FDB with 'known UC' */
    /* testType == TRUNK_TEST_TYPE_CRC_HASH_E */
    /* set mac entries on 'Remote device' so we can point this device via the
       cascade port */
    macEntry1.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry1.dstInterface.devPort.hwDevNum = REMOTE_DEVICE1_NUMBER_CNS;
    macEntry1.dstInterface.devPort.portNum = REMOTE_DEVICE1_PORT_NUM_CNS;/* dummy port number */

    macEntry2.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry2.dstInterface.devPort.hwDevNum = REMOTE_DEVICE2_NUMBER_CNS;
    macEntry2.dstInterface.devPort.portNum = REMOTE_DEVICE2_PORT_NUM_CNS;/* dummy port number */

    /* cascade link info */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;

    /* save the current device map table */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum,REMOTE_DEVICE1_NUMBER_CNS,
            REMOTE_DEVICE1_PORT_NUM_CNS,0,
            &(prvTgfRestoreCfg.cascadeLinkArr[0]),&(prvTgfRestoreCfg.srcPortTrunkHashEnArr[0]));
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum,REMOTE_DEVICE2_NUMBER_CNS,
            REMOTE_DEVICE2_PORT_NUM_CNS,0,
            &(prvTgfRestoreCfg.cascadeLinkArr[1]),&(prvTgfRestoreCfg.srcPortTrunkHashEnArr[1]));
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* configure device map table */
    cascadeLink.linkNum = prvTgfPortsArray[CASCADE_LINK_NUMBER1_CNS];
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,REMOTE_DEVICE1_NUMBER_CNS,
            REMOTE_DEVICE1_PORT_NUM_CNS,0,&cascadeLink,
            GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    cascadeLink.linkNum = prvTgfPortsArray[CASCADE_LINK_NUMBER2_CNS];
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,REMOTE_DEVICE2_NUMBER_CNS,
            REMOTE_DEVICE2_PORT_NUM_CNS,0,&cascadeLink,
            GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* make sure that ALL the mac DA that will be sent during the test will
       be known in the FDB on the needed vlans */
    macEntry1.key.key.macVlan.vlanId = 5;
    for(jj = 0; jj < 6; jj++)
    {
        macEntry1.key.key.macVlan.macAddr.arEther[jj] = prvTgfPacketL2Part.daMac[jj];
        macEntry2.key.key.macVlan.macAddr.arEther[jj] = prvTgfPacketL2Part.daMac[jj];
    }

    macEntry2.key.key.macVlan.vlanId = 5;
    macEntry2.key.key.macVlan.macAddr.arEther[4] =  prvTgfPacketL2Part.daMac[4] + 1;
    macEntry2.key.key.macVlan.macAddr.arEther[5] =  prvTgfPacketL2Part.daMac[5] + 1;

    rc = prvTgfBrgFdbMacEntrySet(&macEntry1);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    rc = prvTgfBrgFdbMacEntrySet(&macEntry2);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsVlanMembers);

    /* ports 0, 8, 18, 23 are VLAN Members */
    for (ii = 0; ii < prvTgfPortsNum; ii++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsVlanMembers,prvTgfPortsArray[ii]);
    }

    /* create VLAN with untagged ports  */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: Send packets to DA MAC1 residing on remote device #1 */
    for(pktIter = 0; pktIter < DEV_MAP_TABLE_TEST_MAX_PKT_CNS; pktIter++)
    {
        prvTgfCscdTestPacketSend(prvTgfPortsArray[0], &prvTgfPacketInfo);
    }

    /* AUTODOC: Send packets to DA MAC2 residing on remote device #2 */
    prvTgfPacketL2Part.daMac[4] = macEntry2.key.key.macVlan.macAddr.arEther[4];
    prvTgfPacketL2Part.daMac[5] = macEntry2.key.key.macVlan.macAddr.arEther[5];
    for(pktIter = 0; pktIter < DEV_MAP_TABLE_TEST_MAX_PKT_CNS; pktIter++)
    {
        prvTgfCscdTestPacketSend(prvTgfPortsArray[0], &prvTgfPacketInfo);
    }

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevsArray[portIter], prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevsArray[portIter], prvTgfPortsArray[portIter]);

        if (prvTgfPortsArray[3] == prvTgfPortsArray[portIter])
        {
            /* check that fourth port did not send or receive any packets */
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
            continue;
        }

        /* Port numbers which are part of cascade links will get even share of
         * packets because of hash based device map lookup mode configurations.
         */
        if ((prvTgfPortsArray[CASCADE_LINK_NUMBER2_CNS] == prvTgfPortsArray[portIter]) || (prvTgfPortsArray[CASCADE_LINK_NUMBER1_CNS] == prvTgfPortsArray[portIter]))
        {
            /* check Tx counters */
            if(prvTgfPortsArray[CASCADE_LINK_NUMBER2_CNS] == prvTgfPortsArray[portIter])
            {
                portCounters[CASCADE_LINK_NUMBER2_CNS] = portCntrs.goodPktsSent.l[0];
            }
            else
            {
                portCounters[CASCADE_LINK_NUMBER1_CNS] = portCntrs.goodPktsSent.l[0];
            }
            /* validate port counters sent from cascade link ports is half of
             * total number of packets sent.
             */
            UTF_VERIFY_EQUAL0_STRING_MAC(DEV_MAP_TABLE_TEST_MAX_PKT_CNS, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(DEV_MAP_TABLE_TEST_MAX_PKT_CNS*2, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }
    portCounters[CASCADE_LINK_NUMBER2_CNS + 1] = portCounters[CASCADE_LINK_NUMBER1_CNS] + portCounters[CASCADE_LINK_NUMBER2_CNS];
    /* check sum of counters of cascade link ports is equal to total number of
     * packets sent.
     */
    UTF_VERIFY_EQUAL0_STRING_MAC(DEV_MAP_TABLE_TEST_MAX_PKT_CNS*2, portCounters[CASCADE_LINK_NUMBER2_CNS + 1],
                                 "get another totalPktsSent counter than expected");

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal tgfTrunkDeviceMapTableTargetPortModeLookupRestore function
* @endinternal
*
* @brief   Restore test configurations
*/
GT_VOID tgfTrunkDeviceMapTableTargetPortModeLookupRestore
(
    GT_VOID
)
{
    GT_STATUS                    rc;
    static GT_U32                devTableArr[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS];

    /* remove the remote device from the FDB device table */
    /* do it before FDB flush that is done in prvTgfTrunkGenericRestore(...)
       otherwise the 'Remote device entries' will not be deleted */
    rc = prvTgfBrgFdbDeviceTableGet(devTableArr);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    devTableArr[0] &= ~(1 << REMOTE_DEVICE1_NUMBER_CNS);

    rc = prvTgfBrgFdbDeviceTableSet(devTableArr);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    devTableArr[0] &= ~(1 << REMOTE_DEVICE2_NUMBER_CNS);

    rc = prvTgfBrgFdbDeviceTableSet(devTableArr);

    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

    /* restore cascade device map lookup mode */
    rc = prvTgfCscdDevMapLookupModeSet(prvTgfRestoreCfg.lookupMode);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* restore the current cascade map table */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,REMOTE_DEVICE1_NUMBER_CNS,
            REMOTE_DEVICE1_PORT_NUM_CNS,0,
            &(prvTgfRestoreCfg.cascadeLinkArr[0]),
            prvTgfRestoreCfg.srcPortTrunkHashEnArr[0],
            GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,REMOTE_DEVICE2_NUMBER_CNS,
            REMOTE_DEVICE2_PORT_NUM_CNS,0,
            &(prvTgfRestoreCfg.cascadeLinkArr[1]),
            prvTgfRestoreCfg.srcPortTrunkHashEnArr[1],
            GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
}

/**
* @internal tgfTrunkDeviceMapTableTargetDeviceSourcePortModeLookupTest function
* @endinternal
*
* @brief   Target port based device map table lookup test
*/
GT_VOID tgfTrunkDeviceMapTableTargetDeviceSourcePortModeLookupTest
(
    GT_VOID
)
{
    GT_STATUS                                     rc;/*return code*/
    GT_U32                                        ii, jj;
    GT_U8                                         portIter;
    GT_U8                                         pktIter;
    CPSS_CSCD_LINK_TYPE_STC                       cascadeLink; /* cascade link info */
    PRV_TGF_BRG_MAC_ENTRY_STC                     macEntry1 = PRV_TGF_BRG_FDB_MAC_ENTRY_DEFAULT_MAC;
    static GT_U32                                 devTableArr[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS];
    CPSS_PORT_MAC_COUNTER_SET_STC                 portCntrs;
    CPSS_PORTS_BMP_STC                            portsVlanMembers = {{0, 0}};
    GT_U32                                        portCounters[MAX_PORT_NUMBER_DEV_MAP_LOOKUP_TEST];

    cpssOsMemSet(&portCounters, 0, sizeof(portCounters));
    /* add the remote device to the FDB device table , so FDB entries on current
       device which are registered on the remote device will not be deleted by
       the aging daemon */
    rc = prvTgfBrgFdbDeviceTableGet(devTableArr);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    devTableArr[0] |= (1 << REMOTE_DEVICE1_NUMBER_CNS);

    rc = prvTgfBrgFdbDeviceTableSet(devTableArr);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc =  prvTgfCscdDevMapLookupModeGet(prvTgfDevNum,&prvTgfRestoreCfg.lookupMode);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    rc = prvTgfCscdDevMapLookupModeSet(PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* next tests need the FDB with 'known UC' */
    /* testType == TRUNK_TEST_TYPE_CRC_HASH_E */
    /* set mac entries on 'Remote device' so we can point this device via the
       cascade port */
    macEntry1.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry1.dstInterface.devPort.hwDevNum = REMOTE_DEVICE1_NUMBER_CNS;
    macEntry1.dstInterface.devPort.portNum = REMOTE_DEVICE1_PORT_NUM_CNS;/* dummy port number */

    /* cascade link info */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;

    /* save the current device map table */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum,REMOTE_DEVICE1_NUMBER_CNS,
            prvTgfPortsArray[0],0,
            &(prvTgfRestoreCfg.cascadeLinkArr[0]),&(prvTgfRestoreCfg.srcPortTrunkHashEnArr[0]));
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum,REMOTE_DEVICE1_NUMBER_CNS,
            prvTgfPortsArray[1],0,
            &(prvTgfRestoreCfg.cascadeLinkArr[1]),&(prvTgfRestoreCfg.srcPortTrunkHashEnArr[1]));
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* configure device map table */
    cascadeLink.linkNum = prvTgfPortsArray[CASCADE_LINK_NUMBER1_CNS + 1];
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,REMOTE_DEVICE1_NUMBER_CNS,
            prvTgfPortsArray[0],0,&cascadeLink,
            GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    cascadeLink.linkNum = prvTgfPortsArray[CASCADE_LINK_NUMBER2_CNS + 1];
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,REMOTE_DEVICE1_NUMBER_CNS,
            prvTgfPortsArray[1],0,&cascadeLink,
            GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* make sure that ALL the mac DA that will be sent during the test will
       be known in the FDB on the needed vlans */
    macEntry1.key.key.macVlan.vlanId = 5;
    for(jj = 0; jj < 6; jj++)
    {
        macEntry1.key.key.macVlan.macAddr.arEther[jj] = prvTgfPacketL2Part.daMac[jj];
    }

    rc = prvTgfBrgFdbMacEntrySet(&macEntry1);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsVlanMembers);

    /* ports 0, 8, 18, 23 are VLAN Members */
    for (ii = 0; ii < prvTgfPortsNum; ii++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsVlanMembers,prvTgfPortsArray[ii]);
    }

    /* create VLAN with untagged ports  */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: Send packets to DA MAC1 from local port 0 residing on remote device #1 */
    for(pktIter = 0; pktIter < DEV_MAP_TABLE_TEST_MAX_PKT_CNS; pktIter++)
    {
        prvTgfCscdTestPacketSend(prvTgfPortsArray[0], &prvTgfPacketInfo);
    }
    for(pktIter = 0; pktIter < DEV_MAP_TABLE_TEST_MAX_PKT_CNS; pktIter++)
    {
        prvTgfCscdTestPacketSend(prvTgfPortsArray[1], &prvTgfPacketInfo);
    }

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevsArray[portIter], prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevsArray[portIter], prvTgfPortsArray[portIter]);

        /* Port numbers which are part of cascade links will get even share of
         * packets because of hash based device map lookup mode configurations.
         */
        if ((prvTgfPortsArray[CASCADE_LINK_NUMBER2_CNS + 1] == prvTgfPortsArray[portIter]) ||
            (prvTgfPortsArray[CASCADE_LINK_NUMBER1_CNS + 1] == prvTgfPortsArray[portIter]))
        {
            /* check Tx counters */
            if(prvTgfPortsArray[CASCADE_LINK_NUMBER2_CNS + 1] == prvTgfPortsArray[portIter])
            {
                /* one of the cascade links receiving 31 packets instead of 32
                 * so tolerance of 1 is acceptable
                 */
                portCounters[CASCADE_LINK_NUMBER2_CNS] = portCntrs.goodPktsSent.l[0];
            }
            else
            {
                portCounters[CASCADE_LINK_NUMBER1_CNS] = portCntrs.goodPktsSent.l[0];
            }
            /* validate port counters sent from cascade link ports is half of
             * total number of packets sent.
             */
            UTF_VERIFY_EQUAL0_STRING_MAC(DEV_MAP_TABLE_TEST_MAX_PKT_CNS, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(DEV_MAP_TABLE_TEST_MAX_PKT_CNS, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }
    portCounters[CASCADE_LINK_NUMBER2_CNS + 1] = portCounters[CASCADE_LINK_NUMBER1_CNS] + portCounters[CASCADE_LINK_NUMBER2_CNS];
    /* check sum of counters of cascade link ports is equal to total number of
     * packets sent.
     */
    UTF_VERIFY_EQUAL0_STRING_MAC(DEV_MAP_TABLE_TEST_MAX_PKT_CNS*2, portCounters[CASCADE_LINK_NUMBER2_CNS + 1],
                                 "get another totalPktsSent counter than expected");

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal tgfTrunkDeviceMapTableTargetDeviceSourcePortModeLookupRestore function
* @endinternal
*
* @brief   Restore test configurations
*/
GT_VOID tgfTrunkDeviceMapTableTargetDeviceSourcePortModeLookupRestore
(
    GT_VOID
)
{
    GT_STATUS                    rc;
    static GT_U32                devTableArr[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS];

    /* remove the remote device from the FDB device table */
    /* do it before FDB flush that is done in prvTgfTrunkGenericRestore(...)
       otherwise the 'Remote device entries' will not be deleted */
    rc = prvTgfBrgFdbDeviceTableGet(devTableArr);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    devTableArr[0] &= ~(1 << REMOTE_DEVICE1_NUMBER_CNS);

    rc = prvTgfBrgFdbDeviceTableSet(devTableArr);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc = prvTgfBrgFdbDeviceTableSet(devTableArr);

    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_VLANID_CNS);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

    /* restore cascade device map lookup mode */
    rc = prvTgfCscdDevMapLookupModeSet(prvTgfRestoreCfg.lookupMode);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* restore the current cascade map table */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,REMOTE_DEVICE1_NUMBER_CNS,
            prvTgfPortsArray[0],0,
            &(prvTgfRestoreCfg.cascadeLinkArr[0]),
            prvTgfRestoreCfg.srcPortTrunkHashEnArr[0],
            GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,REMOTE_DEVICE1_NUMBER_CNS,
            prvTgfPortsArray[1],0,
            &(prvTgfRestoreCfg.cascadeLinkArr[1]),
            prvTgfRestoreCfg.srcPortTrunkHashEnArr[1],
            GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
}

