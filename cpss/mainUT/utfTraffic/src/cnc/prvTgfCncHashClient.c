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
* @file prvTgfCncHashClient.c
*
* @brief Enhanced UT for CNC hash client
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
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCncGen.h>
#include <common/tgfCosGen.h>
#include <extUtils/trafficEngine/private/prvTgfTrafficParser.h>
#include <common/tgfTrunkGen.h>

#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cnc/prvTgfCncVlanL2L3.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
static GT_U16 prvTgfDefVlanId;
static GT_BOOL originalClearByReadEnable;
static GT_BOOL originalCncHashPerPortMode;
static CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT origCpuOrSrcPortMode;
static GT_BOOL origHashMode;
static PRV_TGF_CNC_BYTE_COUNT_MODE_ENT originalClientByteCountmode;
static GT_U32 originPclId = 0;


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
#define PRV_TGF_TTI_RULE_INDEX_CNS 5
#define PRV_TGF_PCL_RULE_INDEX_CNS 8
/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfCncTestHashClientTriggerTtiConfigSet function
* @endinternal
*
*/
static GT_STATUS prvTgfCncTestHashClientTriggerTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                 rc;
    PRV_TGF_TTI_ACTION_2_STC  ttiAction;
    PRV_TGF_TTI_RULE_UNT      ttiPattern;
    PRV_TGF_TTI_RULE_UNT      ttiMask;
    PRV_TGF_TTI_KEY_TYPE_ENT  tunnelPacketType;
    GT_U32                    pclId;
    GT_U32                    ruleId;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configurations with <triggerHashCncClient> action sets to true  =======\n");

    /* Sets pclId, ruleId and tunnelPacketType values */
    pclId = 1;
    ruleId = PRV_TGF_TTI_RULE_INDEX_CNS;
    tunnelPacketType = PRV_TGF_TTI_KEY_ETH_E;

    /* Sets TTI indexes to be relative to the start of the TTI section in TCAM */
    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: Clears TTI action, mask and pattern structures */
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));

    /* AUTODOC: Sets TTI Actions. cmd:FORWARD, triggerHashCncClient:GT_TRUE */
    ttiAction.command              = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.triggerHashCncClient = GT_TRUE;

    /* AUTODOC: Sets TTI Rule Pattern and Mask */
    /* AUTODOC: Rule is empty and matches all packets */
    ttiMask.eth.common.pclId    = 0x3FF;
    ttiPattern.eth.common.pclId = pclId;

    /* AUTODOC: Enables the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], tunnelPacketType, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: Saves PCL id for TTI restore */
    rc = prvTgfTtiPclIdGet(prvTgfDevNum, tunnelPacketType, &originPclId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiPclIdGet");

    /* AUTODOC: Sets the PCL ID for the specified key type */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum, tunnelPacketType, pclId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiPclIdSet");

    /* AUTODOC: Sets the TTI Rule Pattern, Mask and Action */
    rc = prvTgfTtiRule2Set(ruleId, tunnelPacketType, &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiRule2Set");

    return rc;
}

/**
* @internal prvTgfCncTestHashClientTriggerTtiRestore function
* @endinternal
*
*/
static GT_STATUS prvTgfCncTestHashClientTriggerTtiRestore
(
    GT_VOID
)
{
    GT_STATUS                 rc;
    PRV_TGF_TTI_KEY_TYPE_ENT  tunnelPacketType;
    GT_U32                    ruleId;

    PRV_UTF_LOG0_MAC("======= Resetting TTI Configurations =======\n");

    /* AUTODOC: Sets ruleId and tunnelPacketType to the values that were used */
    ruleId = PRV_TGF_TTI_RULE_INDEX_CNS;
    tunnelPacketType = PRV_TGF_TTI_KEY_ETH_E;

    /* AUTODOC: Restore PCL ID for the specified key type */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum, tunnelPacketType, originPclId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiPclIdSet");

    /* AUTODOC: Invalidates the TTI rule */
    rc = prvTgfTtiRuleValidStatusSet(ruleId,GT_FALSE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: Disables the TTI lookup for port[PRV_TGF_CNC_SEND_PORT_INDEX_CNS] and key PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], tunnelPacketType, GT_FALSE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: Sets TTI indexes to absolute */
    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    return rc;
}

/**
* @internal prvTgfCncTestHashClientTriggerPclConfigSet function
* @endinternal
*
*/
static GT_STATUS prvTgfCncTestHashClientTriggerPclConfigSet
(
    GT_VOID
)
{
    GT_STATUS                        rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    GT_U32                           ruleId;

    PRV_UTF_LOG0_MAC("======= Setting IPCL Configurations with <triggerHashCncClient> action sets to true  =======\n");

    /* AUTODOC: Sets ruleFormat and ruleId values */
    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ruleId = PRV_TGF_PCL_RULE_INDEX_CNS;

    /* AUTODOC: Init PCL Engine */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
                              CPSS_PCL_DIRECTION_INGRESS_E,
                              CPSS_PCL_LOOKUP_0_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclDefPortInit");

    /* AUTODOC: Clear mask, pattern and action structures */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* AUTODOC: Sets IPCL actions. pktCmd:FORWARD, triggerHashCncClient:GT_TRUE */
    action.pktCmd               = CPSS_PACKET_CMD_FORWARD_E;
    action.triggerHashCncClient = GT_TRUE;

    /* AUTODOC: add IPCL rule for any packet from the ingress port */
    rc = prvTgfPclRuleSet(ruleFormat, ruleId, &mask, &pattern, &action);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclRuleSet");

    /* Only for Falcon device need to enable assignment for Trigger Hash CNC Client */
    if ( (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)) && (!PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum)) )
    {
        rc = cpssDxChPclTriggerHashCncClientMaskSet(prvTgfDevNum, CPSS_PCL_LOOKUP_0_E, GT_TRUE);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPclTriggerHashCncClientMaskSet");
    }

    return rc;
}

/**
* @internal prvTgfCncTestHashClientTriggerPclRestore function
* @endinternal
*
*/
static GT_STATUS prvTgfCncTestHashClientTriggerPclRestore
(
    GT_VOID
)
{
    GT_STATUS                 rc;
    GT_U32                    ruleId;

    PRV_UTF_LOG0_MAC("======= Resetting PCL Configurations =======\n");

    /* AUTODOC: Sets same ruleId that was used */
    ruleId = PRV_TGF_PCL_RULE_INDEX_CNS;

    /* AUTODOC: Invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, ruleId, GT_FALSE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclRuleValidStatusSet");

    /* AUTODOC : Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclRuleValidStatusSet");

    /* AUTODOC: Disables all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* Only for Falcon device. Disable assignment for Trigger Hash CNC Client */
    if ( (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)) && (!PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum)) )
    {
        rc = cpssDxChPclTriggerHashCncClientMaskSet(prvTgfDevNum, CPSS_PCL_LOOKUP_0_E, GT_FALSE);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPclTriggerHashCncClientMaskSet");
    }

    return rc;
}

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
    /* AUTODOC: save originial dropOrPass mode of Source Port */
    rc = cpssDxChCncPacketTypePassDropToCpuModeGet(prvTgfDevNum,&origCpuOrSrcPortMode);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChCncPacketTypePassDropToCpuModeGet");

    /* AUTODOC: save original Hash mode */
    rc = cpssDxChCncIngressPacketTypeClientHashModeEnableGet(prvTgfDevNum, &origHashMode);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChCncIngressPacketTypeClientHashModeEnableSet");

    /* AUTODOC: save original CNC client Byte Count mode */
    rc = prvTgfCncClientByteCountModeGet(
        CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E,
        &originalClientByteCountmode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncClientByteCountModeGet");

    /* AUTODOC: save original clear by read mode */
    rc =prvTgfCncCounterClearByReadEnableGet(&originalClearByReadEnable);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterClearByReadEnableGet");

    /* AUTODOC: Save original CNC Hash counting mode of sender port */
    rc = cpssDxChCncPortHashClientEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], &originalCncHashPerPortMode);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChCncPortHashClientEnableGet");

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
    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
                                 &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidGet");


    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
                                 PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: create VLAN 5 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, prvTgfPortsArray,
                                           NULL, NULL, prvTgfPortsNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWithPortsSet");

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
};


/**
* @internal prvTgfCncHashTestCncBlockConfigure function
* @endinternal
*
*/
static GT_STATUS prvTgfCncHashTestCncBlockConfigure
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
* @internal prvTgfCncPacketTypeClientConfigure function
* @endinternal
*
* @brief   This function configures Cnc Ingress Packet Type 
*          Pass/Drop tests
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
static GT_STATUS prvTgfCncPacketTypeClientConfigure
(
    IN GT_U32                   counterIndex
)
{
    GT_STATUS                      rc;
    GT_U32                         blockSize;
    GT_U64                         indexRangesBmp;
    PRV_TGF_CNC_COUNTER_STC        counter;

    /* get block size in the current device */
    blockSize = prvTgfCncFineTuningBlockSizeGet();

    indexRangesBmp.l[0] =  (1 << (counterIndex / blockSize));
    indexRangesBmp.l[1] = 0x0;

    counter.byteCount.l[0] = 0;
    counter.byteCount.l[1] = 0;
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;


    /* configure Cnc block */
    rc = prvTgfCncHashTestCncBlockConfigure(
        0 /* Binds block 0 to the client */,
        CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E,
        GT_TRUE /*enable*/,
        indexRangesBmp,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);

    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncHashTestCncBlockConfigure");

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
        CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E,
        PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncClientByteCountModeSet");

    return GT_OK;
}

/**
* @internal prvTgfCncTestIndexSet function
* @endinternal
*
* @brief   This function configures Cnc Index
*
*/
static GT_STATUS prvTgfCncTestIndexSet
(
    IN GT_U32                            blockNum,
    IN CPSS_DXCH_CNC_CLIENT_ENT          client,
    IN GT_U32                            counterIndex
)
{
    GT_STATUS   rc = GT_OK;
    GT_STATUS   rc1 = GT_OK;
    GT_U8       devNum  = 0;
    GT_U64      indexRangesBmp;
    GT_U32      blockSize;

    /* get block size in the current device */
    blockSize = prvTgfCncFineTuningBlockSizeGet();

    indexRangesBmp.l[0] =  (1 << (counterIndex / blockSize));
    indexRangesBmp.l[1] = 0x0;
    

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* set block index range */
        rc = cpssDxChCncBlockClientRangesSet(
                      devNum, blockNum, client, &indexRangesBmp);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncBlockClientRangesSet");

        rc1 = rc;
    }

    return rc1;
}


/**
* @internal prvTgfCncTestHashClientTypeRestore function
* @endinternal
*
* @brief  Restore Configuration.
*/
static GT_STATUS prvTgfCncTestHashClientTypeRestore
(
    GT_VOID
)
{
    GT_U64 indexRangesBmp;
    GT_STATUS rc = GT_OK;

    indexRangesBmp.l[0] = 0;
    indexRangesBmp.l[1] = 0;

    /* AUTODOC: restore original dropOrPass mode of Source Port */
    rc = cpssDxChCncPacketTypePassDropToCpuModeSet(prvTgfDevNum,origCpuOrSrcPortMode);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChCncPacketTypePassDropToCpuModeSet");

    /* AUTODOC: restore original Hash mode */
    rc = cpssDxChCncIngressPacketTypeClientHashModeEnableSet(prvTgfDevNum, origHashMode);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChCncIngressPacketTypeClientHashModeEnableGet");

    /* AUTODOC: restore CNC Hash counting mode for sender port */
    rc = cpssDxChCncPortHashClientEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], originalCncHashPerPortMode);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChCncPortHashClientEnableSet");

    rc = prvTgfCncHashTestCncBlockConfigure(
        0 /* Binds block 0 to the client */,
        CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E,
        GT_FALSE /*disable*/,
        indexRangesBmp,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncHashTestCncBlockConfigure");

    /* restore CNC counter clear by read */
    rc = prvTgfCncCounterClearByReadEnableSet(
        originalClearByReadEnable);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterClearByReadEnableSet");

    /* restore original CNC client Byte Count mode */
    rc = prvTgfCncClientByteCountModeSet(
        CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E,
        originalClientByteCountmode);
    PRV_UTF_VERIFY_GT_OK(rc,"prvTgfCncClientByteCountModeGet");

    return rc;
}

/**
* @internal prvTgfCncHashTestSendPacketAndCheck function
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
* @param[in] cncCounterPerClient         - number of CNC increments per packet
*                                       None
*/
static GT_VOID prvTgfCncHashTestSendPacketAndCheck
(
    IN GT_U32            burstCount,
    IN GT_U32            cncCounterPerClient,
    IN GT_U32            counterIdx,
    IN TGF_PACKET_STC   *packetInfoPtr
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_DXCH_CNC_COUNTER_STC   counter;
    GT_U32                      portIter;
    GT_U32                      blockSize;
    GT_U32                      blockNum=0;
    /*GT_U32                      byteCountPattern = ((PRV_TGF_PACKET_LEN_CNS + 4) * burstCount);  counting all packet bytes with CRC */

    blockSize = prvTgfCncFineTuningBlockSizeGet();

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");
    /* clear counter */
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;
    counter.byteCount.l[0] = 0;
    counter.byteCount.l[1] = 0;

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum,packetInfoPtr,
                             burstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d",
                                 prvTgfDevNum);
    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS]);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS]);

   /* read and check ethernet counters */
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

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* Delay 100 milliseconds - give to sent packet to pass all pilelines in PP */
    cpssOsTimerWkAfter(100);

    /* AURODOC: Get cnc counters */
    rc = cpssDxChCncCounterGet(prvTgfDevNum, blockNum, (counterIdx % blockSize),
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

    PRV_UTF_LOG4_MAC("CncBlock[%d] Index[%d]: packets[%d] bytes[%d]\n",
                    blockNum, counterIdx, counter.packetCount.l[0], counter.byteCount.l[0]);

    UTF_VERIFY_EQUAL2_STRING_MAC((burstCount  *  cncCounterPerClient) , counter.packetCount.l[0],
        "packet counter blockNum: %d counterIdx %d", blockNum, counterIdx);
}

static GT_VOID prvTgfCncTestIngPacketTypeIndexCalc
(
    IN CPSS_PACKET_CMD_ENT packetCmd,
    IN GT_U32 sourcePort,
    IN GT_U32 dropOrPass,
    OUT GT_U32 *indexPtr
)
{
    *indexPtr = 0;
    *indexPtr |= (packetCmd << 11);
    *indexPtr |= sourcePort & 0x3FF;
    *indexPtr |= (dropOrPass << 10);
}

/**
* @internal prvTgfCncTestHashClient function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfCncTestHashClient
(
    GT_VOID
)
{
    GT_U32      burstCount = 1;
    GT_STATUS   rc;
    GT_U32 index = 0;
    GT_U32 cncCounterPerClient=1;
    PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT globalHashModeGet;

    /* AUTODOC: Save CONFIGURATION: */
    rc = prvTgfCncTestSaveConfigurations();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestVidDefConfigurationSet");

    /* AUTODOC: SETUP VLAN CONFIGURATION: */
    rc = prvTgfCncTestVidConfigurationSet();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestVidDefConfigurationSet");

    /* AUTODOC: Test Cnc with Packet type drop or pass client */
    /* AUTODOC: calculate index */
    prvTgfCncTestIngPacketTypeIndexCalc(3 /*FLOW*/ , prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
                                        0,&index);

    /* AUTODOC: set dropOrPass to Source Port */
    rc = cpssDxChCncPacketTypePassDropToCpuModeSet(prvTgfDevNum,
                                                   CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_SRC_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncPacketTypePassDropToCpuModeSet");

    /* AUTODOC: Configure Packet Type Client */
    rc = prvTgfCncPacketTypeClientConfigure(index);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncPacketTypeClientConfigure");

    /* AUTODOC: send traffic and check counters */
    prvTgfCncHashTestSendPacketAndCheck(burstCount,cncCounterPerClient,index, &prvTgfPacketInfo);
    /*UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncClientTest");*/

    /* AUTODOC: Test Cnc with Ingress Hash client */
    /* AUTODOC: configure Drop Client Mode register */
    rc = cpssDxChCncIngressPacketTypeClientHashModeEnableSet(prvTgfDevNum,GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncIngressPacketTypeClientHashModeEnableSet");

    /* AUTODOC: configure coarse grain register */
    rc = cpssDxChTrunkHashNumBitsSet(prvTgfDevNum,CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_CNC_E,0,6);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTrunkHashNumBitsSet");

    /* AUTODOC: Enable CNC Hash counting for sender port */
    rc = cpssDxChCncPortHashClientEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncPortHashClientEnableSet");

    /* AUTODOC: Configure Hash Client index */
    rc = prvTgfCncTestIndexSet(0/*block number*/,CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E, 0x17);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestIndexSet");

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* Get the general hashing mode of trunk hash generation */
        rc =  prvTgfTrunkHashGlobalModeGet(prvTgfDevNum,&globalHashModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeGet: %d", prvTgfDevNum);

        /* Set the general hashing mode of trunk hash generation based on packet data */
        rc =  prvTgfTrunkHashGlobalModeSet(CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeSet: %d", prvTgfDevNum);
    }

    /* AUTODOC: send traffic and check counters */
    prvTgfCncHashTestSendPacketAndCheck(burstCount,cncCounterPerClient,0x17, &prvTgfPacketInfo);
    /*UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncClientTest");*/

    /* Checks triggering of CNC Hash Client via TTI/IPCL units
       - Configures TTI with <triggerHashCncClient> action
       - Sends packet and checks counter
       - Restore TTI configurations
       - Configures IPCL with <triggerHashCncClient> action
       - Sends packet and checks counter
       - Restore PCL configurations
       Applicable for Falcon, AC5P and AC5X devices */
    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Disables CNC Hash counting via port */
        rc = cpssDxChCncPortHashClientEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncPortHashClientEnableSet");

        PRV_UTF_LOG0_MAC("======= Trigger Hash CNC Client via TTI  =======\n");

        /* AUTODOC: Configures TTI to trigger Hash CNC Client */
        rc = prvTgfCncTestHashClientTriggerTtiConfigSet();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestHashClientTriggerTtiConfigSet");

        /* AUTODOC: Sends traffic and check counters */
        prvTgfCncHashTestSendPacketAndCheck(burstCount,cncCounterPerClient,0x17, &prvTgfPacketInfo);

        /* AUTODOC: Restore TTI */
        rc = prvTgfCncTestHashClientTriggerTtiRestore();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestHashClientTriggerTtiRestore");

        PRV_UTF_LOG0_MAC("======= Trigger Hash CNC Client via IPCL  =======\n");

        /* AUTODOC: Configures PCL to trigger Hash CNC Client */
        rc = prvTgfCncTestHashClientTriggerPclConfigSet();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestHashClientTriggerPclConfigSet");

        /* AUTODOC: Sends traffic and check counters */
        prvTgfCncHashTestSendPacketAndCheck(burstCount,cncCounterPerClient,0x17, &prvTgfPacketInfo);

        /* AUTODOC: Restore PCL */
        rc = prvTgfCncTestHashClientTriggerPclRestore();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestHashClientTriggerPclRestore");

    }

    /* Restore VLAN configurations */
    rc = prvTgfCncTestVidConfigurationRestore();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestVidConfigurationRestore");

    /* AUTODOC: RESTORE CNC CONFIGURATION: */
    rc = prvTgfCncTestHashClientTypeRestore();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestHashClientTypeRestore");

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: restore to enhanced crc hash data */
        /* restore global hash mode */
        rc =  prvTgfTrunkHashGlobalModeSet(globalHashModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeSet: %d", prvTgfDevNum);
    }
}

