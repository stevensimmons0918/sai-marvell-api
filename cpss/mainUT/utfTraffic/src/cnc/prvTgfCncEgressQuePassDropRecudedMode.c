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
* @file prvTgfCncEgressQuePassDropRecudedMode.c
*
* @brief Centralized Counters (CNC) - test for Egress Queue CNC client in reduced mode with configured Queue ID.
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCncGen.h>
#include <common/tgfCosGen.h>
#include <extUtils/trafficEngine/private/prvTgfTrafficParser.h>


#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cnc/prvTgfCncVlanL2L3.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
static GT_U16 prvTgfDefVlanId;
static GT_BOOL originalClearByReadEnable;
static PRV_TGF_CNC_BYTE_COUNT_MODE_ENT originalClientByteCountmode;
static GT_U32   originalPortBase[4];


#define PRV_TGF_CNC_TEST_VID 5

#define PRV_TGF_CNC_SEND_PORT_INDEX_CNS 1

#define PRV_TGF_CNC_RECEIVE_PORT_INDEX_CNS 2

/* tested block index */
#define PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC() (PRV_TGF_CNC_BLOCKS_NUM_MAC() - 1)

/******************************* Test packet **********************************/

/* common parts */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};

/******************************* IPv4 packet **********************************/

/* ethertype part of packet1 */
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

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,              /* totalLen */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadDataArr)

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/
#define PRV_TGF_VLANID_CNS 10

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfCncTestVidConfigurationSet function
* @endinternal
*
*/
static GT_STATUS prvTgfCncTestSaveConfigurations
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: save original CNC client Byte Count mode */
    rc = prvTgfCncClientByteCountModeGet(
        CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E,
        &originalClientByteCountmode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncClientByteCountModeGet");

    /* AUTODOC: save original clear by read mode */
    rc =prvTgfCncCounterClearByReadEnableGet(&originalClearByReadEnable);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterClearByReadEnableGet");

    return rc;
}

/**
* @internal prvTgfCncTestVidConfigurationSet function
* @endinternal
*
*/
static GT_STATUS prvTgfCncTestVidConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* AUTODOC: get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
                                 &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidGet");

    /* AUTODOC: Set default vlanId */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
                                 PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: create VLAN 5 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, prvTgfPortsArray,
                                           NULL, NULL, prvTgfPortsNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    return rc;
}

/**
* @internal prvTgfCncTestVidConfigurationRestore function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfCncTestVidConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    /* AUTODOC: RESTORE CONFIGURATION: */
    /* AUTODOC:flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC:reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfEthCountersReset");

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPvidSet");

    /* AUTODOC:invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC:clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    return rc;
}

/**
* @internal prvTgfCncBlockConfigure function
* @endinternal
*
*/
static GT_STATUS prvTgfCncBlockConfigure
(
    IN GT_U32                            blockNum,
    IN CPSS_DXCH_CNC_CLIENT_ENT          client,
    IN GT_BOOL                           enable,
    IN GT_U64                            indexRangesBmp,
    IN CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format
)
{
    GT_STATUS   rc;
    GT_U8       devNum  = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* enable block to client */
        rc = cpssDxChCncBlockClientEnableSet(devNum, blockNum, client, enable);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncBlockClientEnableSet");

        /* set block index range */
        rc = cpssDxChCncBlockClientRangesSet(
                      devNum, blockNum, client, &indexRangesBmp);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncBlockClientRangesSet");

        /* set counter format in block */
        rc = cpssDxChCncCounterFormatSet(devNum, blockNum, format);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterFormatSet");
    }

    return GT_OK;
}

/**
* @internal prvTgfCncConfigure function
* @endinternal
*
* @brief   This function configures Cnc Egress Queue client in Reduced mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
static GT_STATUS prvTgfCncConfigure
(
    IN GT_U32                   counterIndex
)
{
    GT_STATUS                      rc;
    GT_U64                         indexRangesBmp;
    PRV_TGF_CNC_COUNTER_STC        counter;

    /* get block size in the current device */
    GT_U32 blockSize = prvTgfCncFineTuningBlockSizeGet();

    indexRangesBmp.l[0] =(1 << (counterIndex / blockSize));
    indexRangesBmp.l[1] = 0x0;

    counter.byteCount.l[0] = 0;
    counter.byteCount.l[1] = 0;
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;


    /* configure Cnc block */
    rc = prvTgfCncBlockConfigure(
        0 /* Binds block 0 to the client */,
        CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E,
        GT_TRUE /*enable*/,
        indexRangesBmp,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);

    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncBlockConfigure");

    /* AUTODOC: Set client Reduced mode for Egress Queue client */
    rc = cpssDxChCncEgressQueueClientModeSet(prvTgfDevNum, CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChCncEgressQueueClientModeSet");

    /* AUTODOC: enable clear by read mode of CNC counters read operation */
    rc = prvTgfCncCounterClearByReadEnableSet(GT_TRUE /*enable*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterClearByReadEnableSet");

    /* AUTODOC: for FORMAT_MODE_0 set counter clear value 0 */
    rc = prvTgfCncCounterClearByReadValueSet(
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterClearByReadValueSet");

    /* AUTODOC: for L2L3_INGRESS_VLAN CNC client set BYTE_COUNT_MODE_L2 mode */
    rc = prvTgfCncClientByteCountModeSet(
        CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E,
        PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncClientByteCountModeSet");

    return GT_OK;
}

/**
* @internal prvTgfCncTestRestore function
* @endinternal
*
* @brief  Restore Configuration.
*/
static GT_STATUS prvTgfCncTestRestore
(
    GT_VOID
)
{
    GT_U64      indexRangesBmp;
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter = 0;
    indexRangesBmp.l[0] = 0;
    indexRangesBmp.l[1] = 0;

    /* Disable Cnc client on block */
    rc = prvTgfCncBlockConfigure(
        0 /* Binds block 0 to the client */,
        CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E,
        GT_FALSE /*disable*/,
        indexRangesBmp,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncBlockConfigure");

    /* restore CNC counter clear by read */
    rc = prvTgfCncCounterClearByReadEnableSet(
        originalClearByReadEnable);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterClearByReadEnableSet");

    /* restore original CNC client Byte Count mode */
    rc = prvTgfCncClientByteCountModeSet(
        CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E,
        originalClientByteCountmode);
    PRV_UTF_VERIFY_GT_OK(rc,"prvTgfCncClientByteCountModeGet");

    /* restore original per port queue id*/
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = cpssDxChCncPortQueueGroupBaseSet(prvTgfDevNum, prvTgfPortsArray[portIter], CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E, originalPortBase[portIter]);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChCncPortQueueGroupBaseSet");
    }

    return rc;
}

/**
* @internal prvTgfCncEgressReducedModeTestSendPacketAndCheck function
* @endinternal
*
*/
static GT_VOID prvTgfCncEgressReducedModeTestSendPacketAndCheck
(
    IN GT_U32            burstCount,
    IN GT_U32            cncCounterPerClient,
    IN GT_U32            counterIdx,
    IN GT_U32            cpuCounterIdx,
    IN TGF_PACKET_STC   *packetInfoPtr
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_DXCH_CNC_COUNTER_STC   counter;
    GT_U32                      portIter;
    GT_U32                      blockSize;
    GT_U32                      blockNum=0;

    blockSize = prvTgfCncFineTuningBlockSizeGet();

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");
    /* AURODOC: clear counter */
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;
    counter.byteCount.l[0] = 0;
    counter.byteCount.l[1] = 0;

    /* AURODOC: reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* AURODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum,packetInfoPtr,
                             burstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d",
                                 prvTgfDevNum);
    /* AURODOC: clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AURODOC: start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS]);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS]);

   /* AURODOC: read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: verify traffic is dropped */
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            /*expected Rx*/((PRV_TGF_CNC_SEND_PORT_INDEX_CNS  == portIter)  ? 1 : 0),
            /*expected Tx*/1,
            0,
            burstCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AURODOC: stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* Delay 100 milliseconds - give to sent packet to pass all pilelines in PP */
    cpssOsTimerWkAfter(100);

    /* AURODOC: Get cnc counters from Rx ports*/
    rc = cpssDxChCncCounterGet(prvTgfDevNum, blockNum, (counterIdx % blockSize),
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

    PRV_UTF_LOG4_MAC("CncBlock[%d] Index[%d]: packets[%d] bytes[%d]\n",
                    blockNum, counterIdx, counter.packetCount.l[0], counter.byteCount.l[0]);

    UTF_VERIFY_EQUAL2_STRING_MAC((3  *  cncCounterPerClient) , counter.packetCount.l[0],
        "packet counter blockNum: %d counterIdx %d", blockNum, counterIdx);

    /* AURODOC: Get cnc counters from CPU packet*/
    rc = cpssDxChCncCounterGet(prvTgfDevNum, blockNum, (cpuCounterIdx % blockSize),
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

    PRV_UTF_LOG4_MAC("CncBlock[%d] Index[%d]: packets[%d] bytes[%d]\n",
                    blockNum, cpuCounterIdx, counter.packetCount.l[0], counter.byteCount.l[0]);

    UTF_VERIFY_EQUAL2_STRING_MAC(cncCounterPerClient , counter.packetCount.l[0],
        "packet counter blockNum: %d counterIdx %d", blockNum, cpuCounterIdx);
}

/**
* @internal prvTgfCncEgressPassDropRecudedModeQueIdTest function
* @endinternal
*
* @brief    Configure Queue group base for specific port and CNC client.
*           Generate traffic: Send to device's port given packet: Check
*           counters.
*/
GT_VOID prvTgfCncEgressPassDropRecudedModeQueIdTest
(
    GT_VOID
)
{
    GT_U32      burstCount = 1;
    GT_STATUS   rc;
    GT_U32 portBase = 20;
    GT_U32 cpuPortBase = 27 /* 20+7 from tc offset*/;
    GT_U32 cncCounterPerClient=1;
    GT_U32 portIter = 0;

    /* AUTODOC: Save CONFIGURATION: */
    rc = prvTgfCncTestSaveConfigurations();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestVidDefConfigurationSet");

    /* AUTODOC: SETUP VLAN CONFIGURATION: */
    rc = prvTgfCncTestVidConfigurationSet();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestVidDefConfigurationSet");

    /* AUTODOC: Test Cnc */
    /* AUTODOC: Save original and Set per port Queue ID */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
            rc = cpssDxChCncPortQueueGroupBaseGet(prvTgfDevNum, prvTgfPortsArray[portIter], CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E, &originalPortBase[portIter]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncPortQueueGroupBaseGet");
            /* AUTODOC: Configure Cnc Client */
            rc = prvTgfCncConfigure(portBase<<1); /*index of cnc port base <<1 + pass/drop = 0 */
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncConfigure");
            rc = cpssDxChCncPortQueueGroupBaseSet(prvTgfDevNum, prvTgfPortsArray[portIter], CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E,portBase);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncPortQueueGroupBaseSet");
    }

    prvTgfCncEgressReducedModeTestSendPacketAndCheck(burstCount,cncCounterPerClient,(portBase<<1), (cpuPortBase<<1), &prvTgfPacketInfo); /*index calculated QueId[14:1] Pass/Drop[0]*/

    /* AUTODOC: Restore VLAN configurations */
    rc = prvTgfCncTestVidConfigurationRestore();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestVidConfigurationRestore");

    /* AUTODOC: RESTORE CNC CONFIGURATION: */
    rc = prvTgfCncTestRestore();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestRestore");
}

