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
* @file prvTgfCncEgrPacketType.c
*
* @brief CPSS DXCH Centralized counters (CNC) Technology facility implementation.
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
#include <cnc/prvTgfCncEgrPacketType.h>
#include <extUtils/trafficEngine/private/prvTgfTrafficParser.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
static GT_U16 prvTgfDefVlanId;
static GT_U32 isTrapMirror;
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

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfCncTestVidDefConfigurationSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfCncTestVidDefConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: create VLAN 5 with untagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_CNC_TEST_VID);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWrite");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: set PVID 5 for port 1 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], PRV_TGF_CNC_TEST_VID);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* Add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac,
            PRV_TGF_CNC_TEST_VID, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_CNC_RECEIVE_PORT_INDEX_CNS], GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return rc;
};

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
    GT_STATUS rc ,rc1 = GT_OK;

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPvidSet");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_CNC_TEST_VID);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxStartCapture");

    return rc1;
};

static GT_VOID prvTgfCncTestPclConfigurationSet
(
    IN CPSS_PACKET_CMD_ENT packetCmd,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode
)
{
    GT_STATUS                         rc;
    GT_U32                            ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       pattern;
    PRV_TGF_PCL_ACTION_STC            action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ruleFormat;

    /* PCL Configuration */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* init PCL Engine for Egress PCL */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_CNC_RECEIVE_PORT_INDEX_CNS],
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
    action.pktCmd                                = packetCmd;
    action.mirror.cpuCode                        = cpuCode;
    action.egressPolicy                          = GT_TRUE;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);
}

static GT_STATUS prvTgfCncTestPclConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

    rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_STD_E, 0, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");
    prvTgfPclPortsRestore(CPSS_PCL_DIRECTION_EGRESS_E ,CPSS_PCL_LOOKUP_0_E);

    return rc1;
}

/**
* @internal prvTgfCncTestCncBlockConfigure function
* @endinternal
*
* @brief   This function sets configuration of Cnc Block
*
* @param[in] blockNum                 - CNC block number
* @param[in] client                   - CNC client
*                                      valid range see in datasheet of specific device.
* @param[in] enable                   - the client  to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more than one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
* @param[in] indexRangesBmp[]         - the counter index ranges bitmap
*                                      DxCh3 and DxChXcat devices have 8 ranges of counter indexes,
*                                      each 2048 indexes (0..(2K-1), 2K..(4K-1), ..., 14K..(16K-1))
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      The Lion devices has 64 ranges (512 indexes each).
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more than one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
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
static GT_STATUS prvTgfCncTestCncBlockConfigure
(
    IN GT_U32                            blockNum,
    IN CPSS_DXCH_CNC_CLIENT_ENT          client,
    IN GT_BOOL                           enable,
    IN GT_U64                            indexRangesBmp,
    IN CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format
)
{
    GT_STATUS                   rc;
    GT_U8                    devNum  = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChCncBlockClientEnableSet(devNum, blockNum,
                                     client, enable);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncBlockClientEnableSet");

        rc = cpssDxChCncBlockClientRangesSet(
                      devNum, blockNum, client, &indexRangesBmp);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncBlockClientRangesSet");

        rc = cpssDxChCncCounterFormatSet( devNum,
                     blockNum, format);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterFormatSet");
    }

    return GT_OK;
}

/**
* @internal prvTgfCncTestNotZeroCountersDump function
* @endinternal
*
* @brief   This function Dumps all non zero counters
*/
GT_STATUS prvTgfCncTestNotZeroCountersDump
(
    GT_VOID
)
{
    GT_STATUS                   rc;
    GT_U32                      blockIdx;
    GT_U32                      blockAmount;
    GT_U32                      blockSize;
    GT_U32                      counterIdx;
    PRV_TGF_CNC_COUNTER_STC     counter;

    if (prvUtfIsGmCompilation() || cpssDeviceRunCheck_onEmulator())
    {
        /* skip this debug log for GM because of low performance */
        return GT_OK;
    }

    blockAmount = PRV_TGF_CNC_BLOCKS_NUM_MAC();
    blockSize   = PRV_TGF_CNC_BLOCK_ENTRIES_NUM_MAC();

    for (blockIdx = 0; (blockIdx < blockAmount); blockIdx++)
    {
        for (counterIdx = 0; (counterIdx < blockSize); counterIdx ++)
        {
                /* AUTODOC: reset CNC counters for all index and blocks */
            rc = prvTgfCncCounterGet(
                blockIdx, counterIdx,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterGet");

            if ((counter.packetCount.l[0] != 0) || (counter.byteCount.l[0] != 0))
            {
                PRV_UTF_LOG4_MAC(
                    "CncBlock[%d] Index[%d]: packets[%d] bytes[%d]\n",
                    blockIdx, counterIdx, counter.packetCount.l[0], counter.byteCount.l[0]);
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfCncTestEgrPacketTypeConfigure function
* @endinternal
*
* @brief   This function configures Cnc Egress Packet Type Pass/Drop tests
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
static GT_STATUS prvTgfCncTestEgrPacketTypeConfigure
(
    IN CPSS_PACKET_CMD_ENT      packetCmd,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    IN GT_U32                   counterIndex
)
{
    GT_STATUS                      rc;
    GT_U32                         blockSize;
    GT_U64                         indexRangesBmp;
    PRV_TGF_CNC_COUNTER_STC        counter;

    /* AUTODOC: SETUP CONFIGURATION: */
    prvTgfCncTestVidDefConfigurationSet();
    prvTgfCncTestPclConfigurationSet(packetCmd, cpuCode);

    blockSize = prvTgfCncFineTuningBlockSizeGet();
    indexRangesBmp.l[0] =  (1 << (counterIndex / blockSize));
    indexRangesBmp.l[1] = 0x0;

    counter.byteCount.l[0] = 0;
    counter.byteCount.l[1] = 0;
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;

    rc = prvTgfCncTestCncBlockConfigure(
        PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC(),
        CPSS_DXCH_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E,
        GT_TRUE /*enable*/,
        indexRangesBmp,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);

    if (rc != GT_OK)
    {
        return rc;
    }

    /* AUTODOC: enable clear by read mode of CNC counters read operation */
    rc = prvTgfCncCounterClearByReadEnableSet(
        GT_TRUE /*enable*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterClearByReadEnableSet");

    /* AUTODOC: for FORMAT_MODE_0 set counter clear value 0 */
    rc = prvTgfCncCounterClearByReadValueSet(
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterClearByReadValueSet");

    /* AUTODOC: for L2L3_INGRESS_VLAN CNC client set BYTE_COUNT_MODE_L2 mode */
    rc = prvTgfCncClientByteCountModeSet(
        PRV_TGF_CNC_CLIENT_L2L3_INGRESS_VLAN_E,
        PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncClientByteCountModeSet");

    return GT_OK;
}

/**
* @internal prvTgfCncTestSendPacketAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port /given packet:
*         Check that counter's packet count is equail to burstCount
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               -  burst size
* @param[in] blockNum                 -  CNC block number
* @param[in] counterIdx               -  index of counter to check
* @param[in] byteCountPattern         -  expecteded byte counter value
*                                      if 0xFFFFFFFF - the check skipped
* @param[in] cncIncrPerPacket         - number of CNC increments per packet
*                                       None
*/
static GT_VOID prvTgfCncTestSendPacketAndCheck
(
    IN TGF_PACKET_STC   *packetInfoPtr,
    IN GT_U32            burstCount,
    IN GT_U32            blockNum,
    IN GT_U32            counterIdx,
    IN GT_U32            byteCountPattern,
    IN GT_U32            cncIncrPerPacket
)
{
    GT_STATUS                       rc           = GT_OK;
    PRV_TGF_CNC_COUNTER_STC         counter;
    GT_U32          portsCount   = prvTgfPortsNum;
    GT_U32          portIter;
    GT_U32          dsaTagByte   = 16;

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear counter */
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;
    counter.byteCount.l[0] = 0;
    counter.byteCount.l[1] = 0;

    rc = prvTgfCncCounterSet(
        blockNum, counterIdx,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    if (GT_OK != rc)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: prvTgfCncCounterSet FAILED, rc = [%d]",
                                     rc);
        return;
    }

    /* reset counters and force links UP */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

        /* setup Packet */
    rc = prvTgfSetTxSetupEth(
            prvTgfDevsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
            packetInfoPtr, burstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(
        GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
        prvTgfDevsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], burstCount, 0, 0);

        /* send Packet from port 0 */
    rc = prvTgfStartTransmitingEth(
            prvTgfDevsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
            prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth: %d %d",
            prvTgfDevsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
            prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS]);
    /* Delay 100 milliseconds - give to sent packet to pass all pilelines in PP */
    cpssOsTimerWkAfter(100);

    if (isTrapMirror)
    {
        byteCountPattern = dsaTagByte + byteCountPattern*2;
        burstCount       = burstCount +1;
    }

    rc = prvTgfCncCounterGet(
        blockNum, counterIdx,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

    UTF_VERIFY_EQUAL2_STRING_MAC(
        (burstCount * cncIncrPerPacket) , counter.packetCount.l[0],
        "packet counter blockNum: %d counterIdx %d",
        blockNum, counterIdx);

    if (byteCountPattern != 0xFFFFFFFF)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(
            (byteCountPattern  * cncIncrPerPacket) , counter.byteCount.l[0],
            "byte counter blockNum: %d counterIdx %d",
            blockNum, counterIdx);
    }
}

/**
* @internal prvTgfCncTestEgrPacketTypeRestore function
* @endinternal
*
* @brief  Restore Configuration.
*/
static GT_VOID prvTgfCncTestEgrPacketTypeRestore
(
    GT_VOID
)
{
    GT_U64 indexRangesBmp;
    GT_STATUS rc = GT_OK;

    rc = prvTgfCncTestVidConfigurationRestore();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestVidConfigurationRestore");
    rc = prvTgfCncTestPclConfigurationRestore();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestPclConfigurationRestore");

    indexRangesBmp.l[0] = 0;
    indexRangesBmp.l[1] = 0;

    rc = prvTgfCncTestCncBlockConfigure(
        PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC(),
        CPSS_DXCH_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E,
        GT_FALSE /*enable*/,
        indexRangesBmp,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestCncBlockConfigure");

    rc = prvTgfCncCounterClearByReadEnableSet(
        GT_TRUE /*enable*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterClearByReadEnableSet");
}

static GT_VOID prvTgfCncTestEgrPacketTypeConfigTest
(
    IN GT_U32 burstCount,
    IN CPSS_PACKET_CMD_ENT packetCmd,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode
)
{
    GT_U32 index = 0;
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;
    GT_U32      cncIncrPerPacket;

    if(packetCmd == CPSS_PACKET_CMD_FORWARD_E)
    {
         index |= (packetCmd << 8);
         /* FROM_CPU packet and packet of the test increment counter */
         cncIncrPerPacket = 2;
    }
    else
    {
        rc = prvCpssDxChNetIfCpuToDsaCode(cpuCode, &dsaCpuCode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssDxChNetIfCpuToDsaCode");

        index |= ((packetCmd << 8) | (dsaCpuCode & 0xff));
        cncIncrPerPacket = 1;
    }

    PRV_UTF_LOG1_MAC("index = %d\n", index);
    prvTgfCncTestEgrPacketTypeConfigure(packetCmd, cpuCode, index);
    index = index % 1024;
    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:55 */
    prvTgfCncTestSendPacketAndCheck(
        &prvTgfPacketInfo,
        burstCount,
        PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC(),
        index,
        /* counting all packet bytes with CRC */
        ((PRV_TGF_PACKET_LEN_CNS + 4) * burstCount), cncIncrPerPacket);

    prvTgfCncTestNotZeroCountersDump();
    prvTgfCncTestEgrPacketTypeRestore();
}

/**
* @internal prvTgfCncTestVlanL2L3TrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfCncTestEgrPacketTypeTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_U32                          burstCount;
    CPSS_PACKET_CMD_ENT packetCmd;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;
    /* 1. With packet cmd - CPSS_PACKET_CMD_FORWARD_E */
    burstCount = 1;

    packetCmd  = CPSS_PACKET_CMD_FORWARD_E;
    cpuCode    = 0;
    isTrapMirror  = 0;

    prvTgfCncTestEgrPacketTypeConfigTest(burstCount, packetCmd, cpuCode);

    /* 2. With packet cmd - CPSS_PACKET_CMD_TRAP_TO_CPU_E */
    packetCmd  = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    cpuCode    = CPSS_NET_IP_HDR_ERROR_E;
    isTrapMirror  = 1;

    prvTgfCncTestEgrPacketTypeConfigTest(burstCount, packetCmd, cpuCode);

     /* 3. With packet cmd - CPSS_PACKET_CMD_MIRROR_TO_CPU_E */
    packetCmd  = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    cpuCode    = CPSS_NET_IP_HDR_ERROR_E;
    isTrapMirror  = 1;

    prvTgfCncTestEgrPacketTypeConfigTest(burstCount, packetCmd, cpuCode);

     /* 4. With packet cmd - CPSS_PACKET_CMD_DROP_HARD_E */
    packetCmd  = CPSS_PACKET_CMD_DROP_HARD_E;
    cpuCode    = CPSS_NET_FIRST_USER_DEFINED_E;
    isTrapMirror  = 0;

    prvTgfCncTestEgrPacketTypeConfigTest(burstCount, packetCmd, cpuCode);

     /* 5. With packet cmd - CPSS_PACKET_CMD_DROP_SOFT_E */
    packetCmd  = CPSS_PACKET_CMD_DROP_SOFT_E;
    cpuCode    = CPSS_NET_FIRST_USER_DEFINED_E;
    isTrapMirror  = 0;

    prvTgfCncTestEgrPacketTypeConfigTest(burstCount, packetCmd, cpuCode);
}
