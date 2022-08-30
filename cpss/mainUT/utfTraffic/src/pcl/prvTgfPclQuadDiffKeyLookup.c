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
* @file prvTgfPclQuadDiffKeyLookup.c
*
* @brief PCL Quad lookup advanced UTs for SIP6_10 devices supporting different keys parallel sublookups.
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTcamGen.h>
#include <common/tgfCosGen.h>

#include <pcl/prvTgfPclQuadDiffKeyLookup.h>


#define DEBUG_DROP 0

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* the rules match 2 Low MAC_DA bytes of the packet                          */
/* sent 4 packets different only by 16 low bits of MAC_DA                    */
/* The packets are tagged - source VID modified once                         */
/* Packets sent to prvTgfPortsArray[0] and received from prvTgfPortsArray[1] */
/* Each rule modifies VLAN ID to different values                            */

/**
* @struct PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC
 *
 * @brief Configuration for tests 4 parallel sublookups with different keys.
 * 
*/
typedef struct
{
    /** device number */
    GT_U8           devNum;

    /* packet ingress port */
    GT_U32          srcPort;

    /* packet egress port */
    GT_U32          dstPort;

    /** packet info to send - The first part L2 - modified before send */
    TGF_PACKET_STC *prvTgfPacketInfoPtr;

    /** PCL Packet type */
    CPSS_DXCH_PCL_PACKET_TYPE_ENT        pclPacketType;
    
    /* bitmap of sublookups used in test */
    GT_U32          subLookupsBmp;

    /* low 16 bits of MAC_DA for each sublookup */
    GT_U16         macDaBits_15_0_arr[4];

    /** Source packet VLAN */
    GT_U16         srcVid;

    /** VID assigned by Action */
    GT_U16         newVidArr[4];

    /** Pcl Direction */
    CPSS_PCL_DIRECTION_ENT           pclDirection;

    /** Pcl Lookup Number */
    CPSS_PCL_LOOKUP_NUMBER_ENT       pclLookupNum;

    /** PCL Configuration PCL id */
    GT_U32         pclCfgPclId;

    /** PCL Configuration EM profile id */
    GT_U32         pclCfgEmProfileId;

    /** PCL UDB indexes for MAC_DA[15:0] */
    GT_U32         pclUdbBaseIndex;

    /** TTI packet type */
    CPSS_DXCH_TTI_KEY_TYPE_ENT ttiPacketType;

    /** TTI UDB indexes for MAC_DA[15:0] */
    GT_U32         ttiUdbBaseIndex;

    /** TTI TCAM Profile Id mode */
    CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_ENT  ttiTcamProfileIdMode;

    /** TTI TCAM  Profile Id per Port */
    GT_U32 ttiTcamProfileIdPerPort;

    /** MAC_DA[15:0] offset in the key */
    GT_U32         superKeyFieldOffset;

    /** MAC_DA[15:0] offset in the 3 PCL rules for sublookups 1-3 */
    GT_U32         ruleFieldOffsetArr[3];

    /** indexes of rules for 4 hits in 10-byte units */
    GT_U32         ruleIndexArr[4];

    /** sizes of rules for 4 hits in 10-byte units */
    GT_U32         ruleSizesArr[4];

    /** TCAM Client */
    CPSS_DXCH_TCAM_CLIENT_ENT       tcamClient;

    /** TCAM profile id */
    GT_U32         tcamProfileId;

    /** tcam Profile configuration - Mux Table Line Indexes */
    GT_U32         tcamMuxTableLineIndexesArr[3];

    /** tcam Profile configuration - Mux Table Line Offsets in 10 bytes resolution */
    GT_U32         tcamMuxTableLine10BOffsetsArr[3];

    /* tcam MUX table entries for sublookups 1-3 */
    CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC  tcamLookupMuxTableLineCfg[3];

} PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC;

/**
* @struct PRV_TGF_TCAM_DIFF_KEYS_TEST_SAVE_CFG_STC
 *
 * @brief Saved/Restored Configuration for tests 4 parallel sublookups with different keys.
 * 
*/
typedef struct
{
    /** mapping of floor 0 to lookups */
    CPSS_DXCH_TCAM_BLOCK_INFO_STC    floor0InfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];

    /** mapping of floor 1 to lookups */
    CPSS_DXCH_TCAM_BLOCK_INFO_STC    floor1InfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];

} PRV_TGF_TCAM_DIFF_KEYS_TEST_SAVE_CFG_STC;

/**
* @internal prvTgfPclQuadDiffKeySendAndCheckPacket function
* @endinternal
*
* @brief   Send packet and check VID in egress packet.
*
* @param[in] prvTgfPacketInfoPtr  -  pattern packet to send
* @param[in] macDaBits_15_0       -  MAC DA bits[15:0] to modify before send
* @param[in] newVid               -  VID to check in egress packet VLAN tag
*
*/
GT_VOID prvTgfPclQuadDiffKeySendAndCheckPacket
(
    IN  TGF_PACKET_STC   *prvTgfPacketInfoPtr,
    IN  GT_U16           macDaBits_15_0,
    IN  GT_U16           newVid
)
{
    TGF_PACKET_STC              prvTgfPacketInfoCopy;
    TGF_PACKET_PART_STC         partsCopyArr[16];
    TGF_PACKET_L2_STC           prvTgfPacketL2PartCopy;
    TGF_PACKET_L2_STC           *prvTgfPacketL2PartPtr;
    GT_U32                      i;
    GT_U32                      firstL2Part;

    if  (prvTgfPacketInfoPtr->numOfParts > (sizeof(partsCopyArr) / (sizeof(partsCopyArr[0]))))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, GT_FAIL, "amount of packet parts more than supported");
    }

    firstL2Part = 0xFFFFFFFF;
    for (i = 0; (i < prvTgfPacketInfoPtr->numOfParts); i++)
    {
        if ((prvTgfPacketInfoPtr->partsArray[i].type == TGF_PACKET_PART_L2_E) 
            && (firstL2Part == 0xFFFFFFFF))
        {
            firstL2Part = i;
        }
    }
    if (firstL2Part == 0xFFFFFFFF)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, GT_FAIL, "not found L2 part");
    }
    prvTgfPacketL2PartPtr = (TGF_PACKET_L2_STC *)prvTgfPacketInfoPtr->partsArray[firstL2Part].partPtr;
    for (i = 0; (i < 4); i++)
    {
        prvTgfPacketL2PartCopy.daMac[i] = prvTgfPacketL2PartPtr->daMac[i];
    }
    prvTgfPacketL2PartCopy.daMac[4] = (GT_U8)((macDaBits_15_0 >> 8) & 0xFF);
    prvTgfPacketL2PartCopy.daMac[5] = (GT_U8)(macDaBits_15_0 & 0xFF);
    for (i = 0; (i < 6); i++)
    {
        prvTgfPacketL2PartCopy.saMac[i] = prvTgfPacketL2PartPtr->saMac[i];
    }

    for (i = 0; (i < firstL2Part); i++)
    {
        partsCopyArr[i] = prvTgfPacketInfoPtr->partsArray[i];
    }
    partsCopyArr[firstL2Part].partPtr = &prvTgfPacketL2PartCopy;
    partsCopyArr[firstL2Part].type    = TGF_PACKET_PART_L2_E;
    for (i = (firstL2Part + 1); (i < prvTgfPacketInfoPtr->numOfParts); i++)
    {
        partsCopyArr[i] = prvTgfPacketInfoPtr->partsArray[i];
    }
    prvTgfPacketInfoCopy.numOfParts = prvTgfPacketInfoPtr->numOfParts;
    prvTgfPacketInfoCopy.totalLen   = prvTgfPacketInfoPtr->totalLen;
    prvTgfPacketInfoCopy.partsArray = partsCopyArr;

    prvTgfVidModifyTrafficGenerateAndCheck(
        &prvTgfPacketInfoCopy,
        prvTgfDevsArray[0], prvTgfPortsArray[0],
        prvTgfDevsArray[1], prvTgfPortsArray[1],
        newVid);
}

/**
* @internal prvTgfPclQuadDiffKeyLookupTestVlanAndTraffic function
* @endinternal
*
* @brief   IPCL Quad lookup of parallel different keys test - VLANs and traffic
*/
GT_VOID prvTgfPclQuadDiffKeyLookupTestVlanAndTraffic
(
   IN PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC *testCgrPtr
)
{
    GT_STATUS rc;
    GT_U32    portsArray[2];
    GT_U8     devArray[2];
    GT_U8     tagArray[2];
    GT_U32    subLookup;

    portsArray[0] = prvTgfPortsArray[0];
    portsArray[1] = prvTgfPortsArray[1];
    devArray[0]   = testCgrPtr->devNum;
    devArray[1]   = testCgrPtr->devNum;
    tagArray[0]   = 1;
    tagArray[1]   = 1;

    prvTgfBrgDefVlanEntryWithPortsSet(
        testCgrPtr->srcVid /*vlanId*/, portsArray, devArray, tagArray, 2/*numPorts*/);

    for (subLookup = 0; (subLookup < 4); subLookup++)
    {
        if (((testCgrPtr->subLookupsBmp >> subLookup) & 1) == 0) continue;

        prvTgfBrgDefVlanEntryWithPortsSet(
            testCgrPtr->newVidArr[subLookup] /*vlanId*/, portsArray, devArray, tagArray, 2/*numPorts*/);

        prvTgfPclQuadDiffKeySendAndCheckPacket(
            testCgrPtr->prvTgfPacketInfoPtr,
            testCgrPtr->macDaBits_15_0_arr[subLookup]/*macDaBits_15_0*/,
            testCgrPtr->newVidArr[subLookup]/*newVid*/);

        prvTgfBrgDefVlanEntryInvalidate(testCgrPtr->newVidArr[subLookup] /*vlanId*/);
    }

    prvTgfBrgDefVlanEntryInvalidate(testCgrPtr->srcVid /*vlanId*/);

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfPclQuadDiffKeyLookupTestPclConfigure function
* @endinternal
*
* @brief   IPCL Quad lookup of parallel different keys test - PCL configuration
*/
GT_VOID prvTgfPclQuadDiffKeyLookupTestPclConfigure
(
   IN PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC *testCgrPtr
)
{
    GT_STATUS                           rc;
    GT_U32                              portNum;
    PRV_TGF_PCL_LOOKUP_CFG_STC          lookupCfg;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    keyType;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    GT_U32                              subLookup;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT       mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT       pattern;
    CPSS_DXCH_PCL_ACTION_STC            action;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    GT_U32 fieldOffset;

    portNum = 
        (testCgrPtr->pclDirection == CPSS_PCL_DIRECTION_INGRESS_E)
            ? testCgrPtr->srcPort : testCgrPtr->dstPort;

    keyType =
        (testCgrPtr->pclDirection == CPSS_PCL_DIRECTION_INGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E
            : PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_60_E;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup = GT_TRUE;
    lookupCfg.groupKeyTypes.nonIpKey = keyType;
    lookupCfg.groupKeyTypes.ipv4Key  = keyType;
    lookupCfg.groupKeyTypes.ipv6Key  = keyType;
    lookupCfg.tcamSegmentMode = PRV_TGF_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E;
    lookupCfg.udbKeyBitmapEnable = GT_FALSE;
    lookupCfg.exactMatchLookupSubProfileId = testCgrPtr->pclCfgEmProfileId;
    lookupCfg.pclId = testCgrPtr->pclCfgPclId;

    rc = prvTgfPclDefPortInitExt2(
        portNum, testCgrPtr->pclDirection, testCgrPtr->pclLookupNum, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInitExt2");

    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));
    udbSelect.udbSelectArr[testCgrPtr->superKeyFieldOffset]     = testCgrPtr->pclUdbBaseIndex;
    udbSelect.udbSelectArr[testCgrPtr->superKeyFieldOffset + 1] = testCgrPtr->pclUdbBaseIndex + 1;

    rc = prvTgfPclUserDefinedBytesSelectSet(
        keyType, testCgrPtr->pclPacketType, testCgrPtr->pclLookupNum,
        &udbSelect);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet");

    rc = prvTgfPclUserDefinedByteSet(
        keyType, testCgrPtr->pclPacketType, testCgrPtr->pclDirection,
        testCgrPtr->pclUdbBaseIndex/*udbIndex*/, PRV_TGF_PCL_OFFSET_L2_E, 4/*offset*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet");

    rc = prvTgfPclUserDefinedByteSet(
        keyType, testCgrPtr->pclPacketType, testCgrPtr->pclDirection,
        (testCgrPtr->pclUdbBaseIndex + 1)/*udbIndex*/, PRV_TGF_PCL_OFFSET_L2_E, 5/*offset*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet");

    /**********************************************************************/
    /* 4 rules for 4 sublookups */

    for (subLookup = 0; (subLookup < 4); subLookup++)
    {
        fieldOffset = (subLookup == 0) ? testCgrPtr->superKeyFieldOffset : testCgrPtr->ruleFieldOffsetArr[subLookup - 1];

        cpssOsMemSet(&mask, 0, sizeof(mask));
        cpssOsMemSet(&pattern , 0, sizeof(pattern));
        cpssOsMemSet(&action , 0, sizeof(action));
        if (testCgrPtr->pclDirection == CPSS_PCL_DIRECTION_INGRESS_E)
        {
            mask.ruleIngrUdbOnly.udb[fieldOffset]     = 0xFF;
            mask.ruleIngrUdbOnly.udb[fieldOffset + 1] = 0xFF;
            pattern.ruleIngrUdbOnly.udb[fieldOffset] = 
                (GT_U8)((testCgrPtr->macDaBits_15_0_arr[subLookup] >> 8) & 0xFF);
            pattern.ruleIngrUdbOnly.udb[fieldOffset + 1] = 
                (GT_U8)(testCgrPtr->macDaBits_15_0_arr[subLookup] & 0xFF);
            action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
            action.egressPolicy = GT_FALSE;
            action.vlan.ingress.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
            action.vlan.ingress.vlanId = testCgrPtr->newVidArr[subLookup];
            ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E +(testCgrPtr->ruleSizesArr[subLookup] - 1);
            if (ruleFormat == CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E)
            {
                ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E;
            }
        }
        else
        {
            mask.ruleEgrUdbOnly.udb[fieldOffset] = 0xFF;
            mask.ruleEgrUdbOnly.udb[fieldOffset + 1] = 0xFF;
            pattern.ruleEgrUdbOnly.udb[fieldOffset] = 
                (GT_U8)((testCgrPtr->macDaBits_15_0_arr[subLookup] >> 8) & 0xFF);
            pattern.ruleEgrUdbOnly.udb[fieldOffset + 1] = 
                (GT_U8)(testCgrPtr->macDaBits_15_0_arr[subLookup] & 0xFF);
            action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
            action.egressPolicy = GT_TRUE;
            action.vlan.egress.vlanCmd =
                CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
            action.vlan.egress.vlanId = testCgrPtr->newVidArr[subLookup];
            ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E + (testCgrPtr->ruleSizesArr[subLookup] - 1);
        }

        rc = cpssDxChPclRuleSet(
            testCgrPtr->devNum, 0/*tcamIndex*/,
            ruleFormat, testCgrPtr->ruleIndexArr[subLookup], 0/*ruleOptionsBmp*/,
            &mask, &pattern, &action);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPclRuleSet");
    }

    rc = cpssDxChPclMapCfgTableProfileIdToTcamProfileIdSet(
        testCgrPtr->devNum, testCgrPtr->pclDirection,
        testCgrPtr->pclPacketType, testCgrPtr->pclCfgEmProfileId,
        testCgrPtr->tcamProfileId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPclMapCfgTableProfileIdToTcamProfileIdSet");

}

/**
* @internal prvTgfPclQuadDiffKeyLookupTestPclCfgRestore function
* @endinternal
*
* @brief   IPCL Quad lookup of parallel different keys test - PCL configuration restore
*/
GT_VOID prvTgfPclQuadDiffKeyLookupTestPclCfgRestore
(
   IN PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC *testCgrPtr
)
{
    GT_U32                           portNum;
    GT_STATUS                        rc, rc1 = GT_OK;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;
    GT_U32                           subLookupIdx;
    static CPSS_PCL_RULE_SIZE_ENT    ruleSizesArr[] =
    {
        CPSS_PCL_RULE_SIZE_10_BYTES_E,
        CPSS_PCL_RULE_SIZE_20_BYTES_E,
        CPSS_PCL_RULE_SIZE_30_BYTES_E,
        CPSS_PCL_RULE_SIZE_40_BYTES_E,
        CPSS_PCL_RULE_SIZE_50_BYTES_E,
        CPSS_PCL_RULE_SIZE_60_BYTES_E,
    };

    portNum = 
        (testCgrPtr->pclDirection == CPSS_PCL_DIRECTION_INGRESS_E)
            ? testCgrPtr->srcPort : testCgrPtr->dstPort;

    /* AUTODOC: remove PCL configurations */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type                = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum    = testCgrPtr->devNum;
    lookupCfg.enableLookup            = GT_FALSE;  /* --> disable the PCL lookup */
    lookupCfg.pclId                   = 0;
    interfaceInfo.devPort.portNum     = portNum;
    if (testCgrPtr->pclDirection == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        lookupCfg.groupKeyTypes.nonIpKey =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/;
        lookupCfg.groupKeyTypes.ipv4Key  =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/;
        lookupCfg.groupKeyTypes.ipv6Key  =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/;
    }
    else
    {
        lookupCfg.groupKeyTypes.nonIpKey =
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E /*nonIpKey*/;
        lookupCfg.groupKeyTypes.ipv4Key  =
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E /*ipv4Key*/;
        lookupCfg.groupKeyTypes.ipv6Key  =
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv6Key*/;
    }

    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, testCgrPtr->pclDirection, testCgrPtr->pclLookupNum, &lookupCfg);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclCfgTblSet");

    if (testCgrPtr->pclDirection == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        rc = prvTgfPclPortIngressPolicyEnable(
            portNum, GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortIngressPolicyEnable");
    }
    else
    {
        rc = prvTgfPclEgressPclPacketTypesSet(
            testCgrPtr->devNum, portNum,
            PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");
    }

    for (subLookupIdx = 0; (subLookupIdx < 4); subLookupIdx++)
    { 
        /* AUTODOC: invalidate PCL rule */
        rc = cpssDxChPclRuleValidStatusSet(
            testCgrPtr->devNum, 0/*tcamIndex*/,
            ruleSizesArr[testCgrPtr->ruleSizesArr[subLookupIdx] - 1],
            testCgrPtr->ruleIndexArr[subLookupIdx],
            GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "cpssDxChPclRuleValidStatusSet");
    }

    rc = rc1; /* to avoid compiler warning */
}

/**
* @internal prvTgfPclQuadDiffKeyLookupTestTcamCfgSave function
* @endinternal
*
* @brief   IPCL Quad lookup of parallel different keys test - TCAM configuration save
*/
GT_VOID prvTgfPclQuadDiffKeyLookupTestTcamCfgSave
(
    IN  PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC *testCgrPtr,
    OUT PRV_TGF_TCAM_DIFF_KEYS_TEST_SAVE_CFG_STC *testSavePtr
)
{
    GT_STATUS rc;

    rc = cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet(
        testCgrPtr->devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        0 /*floorIndex*/, &(testSavePtr->floor0InfoArr[0]));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet");

    rc = cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet(
        testCgrPtr->devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        1 /*floorIndex*/, &(testSavePtr->floor1InfoArr[0]));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet");
}

/**
* @internal prvTgfPclQuadDiffKeyLookupTestTcamCfgRestore function
* @endinternal
*
* @brief   IPCL Quad lookup of parallel different keys test - TCAM configuration restore
*/
GT_VOID prvTgfPclQuadDiffKeyLookupTestTcamCfgRestore
(
    IN  PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC *testCgrPtr,
    IN  PRV_TGF_TCAM_DIFF_KEYS_TEST_SAVE_CFG_STC *testSavePtr
)
{
    GT_STATUS rc, rc1 = GT_OK;

    rc = cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet(
        testCgrPtr->devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        0 /*floorIndex*/, &(testSavePtr->floor0InfoArr[0]));
    PRV_UTF_VERIFY_RC1(rc, "cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet");

    rc = cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet(
        testCgrPtr->devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        1 /*floorIndex*/, &(testSavePtr->floor1InfoArr[0]));
    PRV_UTF_VERIFY_RC1(rc, "cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet");

    rc = rc1; /* to avoid compiler warning */
}

/**
* @internal prvTgfPclQuadDiffKeyLookupTestTcamCfgRestore function
* @endinternal
*
* @brief   IPCL Quad lookup of parallel different keys test - TCAM configuration restore
*/
GT_VOID prvTgfPclQuadDiffKeyLookupTestTcamConfigure
(
    IN  PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC *testCgrPtr
)
{
    GT_STATUS rc;
    GT_U32  clientGroup;
    GT_BOOL enable;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC    floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];
    GT_U32  numBanksForHitNumGranularity;
    GT_U32  hitCfgPerBlock;
    GT_U32  i;
    CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC  tcamLookupProfileCfg;
    CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC  tcamLookupMuxTableLineCfg;
    GT_U32  subLookup;

    rc = cpssDxChTcamPortGroupClientGroupGet(
        testCgrPtr->devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        testCgrPtr->tcamClient, &clientGroup, &enable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTcamPortGroupClientGroupGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, enable, "TCAM Client disable");

    numBanksForHitNumGranularity =  /* in BC2/BC3 = 2 , in BobK/Aldrin = 6 */
        PRV_CPSS_DXCH_PP_MAC(testCgrPtr->devNum)->hwInfo.tcam.numBanksForHitNumGranularity;
    hitCfgPerBlock = numBanksForHitNumGranularity / (12 / 6);

    /* hits 0-1 floor 0 */
    for (i = 0; (i < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS); i++)
    {
        floorInfoArr[i].group  = clientGroup;
        floorInfoArr[i].hitNum = (i / hitCfgPerBlock);
    }
    rc = cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet(
        testCgrPtr->devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        0 /*floorIndex*/, &(floorInfoArr[0]));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet");

    /* hits 2-3 floor 1 */
    for (i = 0; (i < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS); i++)
    {
        floorInfoArr[i].group  = clientGroup;
        floorInfoArr[i].hitNum = (i / hitCfgPerBlock) + 2;
    }
    rc = cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet(
        testCgrPtr->devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        1 /*floorIndex*/, &(floorInfoArr[0]));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet");

    cpssOsMemSet(&tcamLookupProfileCfg, 0, sizeof(tcamLookupProfileCfg));
    tcamLookupProfileCfg.subKey0_3_Sizes[0] = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E + (testCgrPtr->ruleSizesArr[0] - 1);
    tcamLookupProfileCfg.subKey0_3_Sizes[1] = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E + (testCgrPtr->ruleSizesArr[1] - 1);
    tcamLookupProfileCfg.subKey0_3_Sizes[2] = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E + (testCgrPtr->ruleSizesArr[2] - 1);
    tcamLookupProfileCfg.subKey0_3_Sizes[3] = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E + (testCgrPtr->ruleSizesArr[3] - 1);
    tcamLookupProfileCfg.subKey1_3_MuxTableLineIndexes[0] = testCgrPtr->tcamMuxTableLineIndexesArr[0];
    tcamLookupProfileCfg.subKey1_3_MuxTableLineIndexes[1] = testCgrPtr->tcamMuxTableLineIndexesArr[1];
    tcamLookupProfileCfg.subKey1_3_MuxTableLineIndexes[2] = testCgrPtr->tcamMuxTableLineIndexesArr[2];
    tcamLookupProfileCfg.subKey1_3_MuxTableLineOffsets[0] = testCgrPtr->tcamMuxTableLine10BOffsetsArr[0];
    tcamLookupProfileCfg.subKey1_3_MuxTableLineOffsets[1] = testCgrPtr->tcamMuxTableLine10BOffsetsArr[1];
    tcamLookupProfileCfg.subKey1_3_MuxTableLineOffsets[2] = testCgrPtr->tcamMuxTableLine10BOffsetsArr[2];

    rc = cpssDxChTcamLookupProfileCfgSet(
        testCgrPtr->devNum, testCgrPtr->tcamProfileId, &tcamLookupProfileCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTcamLookupProfileCfgSet");
    
    for (subLookup = 1; (subLookup < 4); subLookup++)
    {
        cpssOsMemSet(&tcamLookupMuxTableLineCfg, 0, sizeof(tcamLookupMuxTableLineCfg));
        tcamLookupMuxTableLineCfg.superKeyTwoByteUnitsOffsets[
            (testCgrPtr->tcamMuxTableLine10BOffsetsArr[subLookup - 1] * (10 / 2))
            + (testCgrPtr->ruleFieldOffsetArr[subLookup - 1] / 2)]
                = (testCgrPtr->superKeyFieldOffset / 2);

        rc = cpssDxChTcamLookupMuxTableLineSet(
            testCgrPtr->devNum,
            testCgrPtr->tcamMuxTableLineIndexesArr[subLookup - 1],
            &tcamLookupMuxTableLineCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTcamLookupMuxTableLineSet");
    }
}

/**
* @internal prvTgfPclQuadDiffKeyLookupTestTtiConfigure function
* @endinternal
*
* @brief   TTI Quad lookup of parallel different keys test - TTI configure
*/
GT_VOID prvTgfPclQuadDiffKeyLookupTestTtiConfigure
(
   IN PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC *testCgrPtr
)
{
    GT_STATUS                     rc;
    CPSS_DXCH_TTI_RULE_UNT        mask;
    CPSS_DXCH_TTI_RULE_UNT        pattern;
    CPSS_DXCH_TTI_ACTION_STC      ruleAction;
    GT_U32                        subLookup;
    GT_U32                        fieldOffset;
    GT_U32                        defaultRuleIndex;

     /* UDB for MAC_DA[15:0] */
    rc = cpssDxChTtiUserDefinedByteSet(
        testCgrPtr->devNum, testCgrPtr->ttiPacketType, testCgrPtr->ttiUdbBaseIndex,
        CPSS_DXCH_TTI_OFFSET_L2_E, 4/*offset*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiUserDefinedByteSet");

    rc = cpssDxChTtiUserDefinedByteSet(
        testCgrPtr->devNum, testCgrPtr->ttiPacketType, (testCgrPtr->ttiUdbBaseIndex + 1),
        CPSS_DXCH_TTI_OFFSET_L2_E, 5/*offset*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiUserDefinedByteSet");

    rc = cpssDxChTtiPortLookupEnableSet(
        testCgrPtr->devNum, testCgrPtr->srcPort,
        testCgrPtr->ttiPacketType, GT_TRUE /*enable*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiPortLookupEnableSet");

    rc = cpssDxChTtiPacketTypeKeySizeSet(
        testCgrPtr->devNum, testCgrPtr->ttiPacketType,
        CPSS_DXCH_TTI_KEY_SIZE_30_B_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiPacketTypeKeySizeSet");

    for (subLookup = 0; (subLookup < 4); subLookup++)
    {
        cpssOsMemSet(&mask, 0, sizeof(mask));
        cpssOsMemSet(&pattern, 0, sizeof(pattern));
        cpssOsMemSet(&ruleAction, 0, sizeof(ruleAction));

        fieldOffset = (subLookup == 0) ? testCgrPtr->superKeyFieldOffset : testCgrPtr->ruleFieldOffsetArr[subLookup - 1];

        mask.udbArray.udb[fieldOffset] = 0xFF;
        mask.udbArray.udb[fieldOffset + 1] = 0xFF;
        pattern.udbArray.udb[fieldOffset] = ((testCgrPtr->macDaBits_15_0_arr[subLookup] >> 8) & 0xFF);
        pattern.udbArray.udb[fieldOffset + 1] = (testCgrPtr->macDaBits_15_0_arr[subLookup] & 0xFF);

        ruleAction.command         = CPSS_PACKET_CMD_FORWARD_E;
        ruleAction.tunnelTerminate = GT_FALSE;
        ruleAction.tunnelStart     = GT_FALSE;
        ruleAction.tag1VlanCmd     = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
        ruleAction.tag0VlanCmd     = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
        ruleAction.tag0VlanId      = testCgrPtr->newVidArr[subLookup];
        ruleAction.continueToNextTtiLookup = GT_TRUE;

        rc = cpssDxChTtiRuleSet(
            testCgrPtr->devNum, testCgrPtr->ruleIndexArr[subLookup],
            (CPSS_DXCH_TTI_RULE_UDB_10_E + testCgrPtr->ruleSizesArr[subLookup] - 1),
            &pattern, &mask, &ruleAction);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiRuleSet");

        defaultRuleIndex = testCgrPtr->ruleIndexArr[subLookup] + 12; /* same block next row */
        cpssOsMemSet(&mask, 0, sizeof(mask));
        cpssOsMemSet(&pattern, 0, sizeof(pattern));
        cpssOsMemSet(&ruleAction, 0, sizeof(ruleAction));
        ruleAction.command         = CPSS_PACKET_CMD_FORWARD_E;
        ruleAction.tunnelTerminate = GT_FALSE;
        ruleAction.tunnelStart     = GT_FALSE;
        ruleAction.tag1VlanCmd     = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
        ruleAction.tag0VlanCmd     = CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E;
        ruleAction.continueToNextTtiLookup = GT_TRUE;
        rc = cpssDxChTtiRuleSet(
            testCgrPtr->devNum, defaultRuleIndex,
            (CPSS_DXCH_TTI_RULE_UDB_10_E + testCgrPtr->ruleSizesArr[subLookup] - 1),
            &pattern, &mask, &ruleAction);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiRuleSet");
    }

    switch (testCgrPtr->ttiTcamProfileIdMode)
    {
        case CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_PER_PORT_E:
            rc = cpssDxChTtiPortTcamProfileIdModeSet(
                testCgrPtr->devNum, testCgrPtr->srcPort,
                testCgrPtr->ttiTcamProfileIdMode, testCgrPtr->tcamProfileId);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiPortTcamProfileIdModeSet");
            break;

        case CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_PER_PACKET_TYPE_E:
            rc = cpssDxChTtiPortTcamProfileIdModeSet(
                testCgrPtr->devNum, testCgrPtr->srcPort,
                testCgrPtr->ttiTcamProfileIdMode, 0);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiPortTcamProfileIdModeSet");

            rc = cpssDxChTtiPacketTypeTcamProfileIdSet(
                testCgrPtr->devNum, testCgrPtr->ttiPacketType, testCgrPtr->tcamProfileId);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiPacketTypeTcamProfileIdSet");
            break;

        case CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_PER_PORT_AND_PACKET_TYPE_E:
            rc = cpssDxChTtiPortTcamProfileIdModeSet(
                testCgrPtr->devNum, testCgrPtr->srcPort,
                testCgrPtr->ttiTcamProfileIdMode, testCgrPtr->ttiTcamProfileIdPerPort);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiPortTcamProfileIdModeSet");

            rc = cpssDxChTtiPortAndPacketTypeTcamProfileIdSet(
                testCgrPtr->devNum, testCgrPtr->ttiPacketType,
                testCgrPtr->ttiTcamProfileIdPerPort, testCgrPtr->tcamProfileId);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiPortAndPacketTypeTcamProfileIdSet");
            break;

        default:
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "cpssDxChTtiPortTcamProfileIdModeSet - wrong mode");
            break;
    }
}

/**
* @internal prvTgfPclQuadDiffKeyLookupTestTtiCfgRestore function
* @endinternal
*
* @brief   TTI Quad lookup of parallel different keys test - TTI configuration restore
*/
GT_VOID prvTgfPclQuadDiffKeyLookupTestTtiCfgRestore
(
   IN PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC *testCgrPtr
)
{
    GT_STATUS rc, rc1 = GT_OK;
    GT_U32    subLookup;
    GT_U32    defaultRuleIndex;

    rc = cpssDxChTtiPortLookupEnableSet(
        testCgrPtr->devNum, testCgrPtr->srcPort,
        testCgrPtr->ttiPacketType, GT_FALSE /*enable*/);
    PRV_UTF_VERIFY_RC1(rc, "cpssDxChTtiPortLookupEnableSet");

     /* UDB for MAC_DA[15:0] */
    rc = cpssDxChTtiUserDefinedByteSet(
        testCgrPtr->devNum, testCgrPtr->ttiPacketType, testCgrPtr->ttiUdbBaseIndex,
        CPSS_DXCH_TTI_OFFSET_INVALID_E, 0/*offset*/);
    PRV_UTF_VERIFY_RC1(rc, "cpssDxChTtiUserDefinedByteSet");

    rc = cpssDxChTtiUserDefinedByteSet(
        testCgrPtr->devNum, testCgrPtr->ttiPacketType, (testCgrPtr->ttiUdbBaseIndex + 1),
        CPSS_DXCH_TTI_OFFSET_INVALID_E, 0/*offset*/);
    PRV_UTF_VERIFY_RC1(rc, "cpssDxChTtiUserDefinedByteSet");

    for (subLookup = 0; (subLookup < 4); subLookup++)
    {
        rc = cpssDxChTtiRuleValidStatusSet(
            testCgrPtr->devNum, testCgrPtr->ruleIndexArr[subLookup],
            GT_FALSE /*valid*/);
        PRV_UTF_VERIFY_RC1(rc, "cpssDxChTtiRuleValidStatusSet");

        defaultRuleIndex = testCgrPtr->ruleIndexArr[subLookup] + 12; /* same block next row */
        rc = cpssDxChTtiRuleValidStatusSet(
            testCgrPtr->devNum, defaultRuleIndex,
            GT_FALSE /*valid*/);
        PRV_UTF_VERIFY_RC1(rc, "cpssDxChTtiRuleValidStatusSet");
    }

    rc = cpssDxChTtiPortTcamProfileIdModeSet(
        testCgrPtr->devNum, testCgrPtr->srcPort,
        CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_PER_PORT_E, 0 /*TCAM profile Id*/);
    PRV_UTF_VERIFY_RC1(rc, "cpssDxChTtiRuleValidStatusSet");

    rc = cpssDxChTtiPacketTypeTcamProfileIdSet(
        testCgrPtr->devNum, testCgrPtr->ttiPacketType, 0);
    PRV_UTF_VERIFY_RC1(rc, "cpssDxChTtiPacketTypeTcamProfileIdSet");

    rc = cpssDxChTtiPortAndPacketTypeTcamProfileIdSet(
        testCgrPtr->devNum, testCgrPtr->ttiPacketType,
        testCgrPtr->ttiTcamProfileIdPerPort, 0);
    PRV_UTF_VERIFY_RC1(rc, "cpssDxChTtiPortAndPacketTypeTcamProfileIdSet");

    rc = rc1; /* to avoid compiler warning */
}

/*---------------------------------------------------------*/

static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x19},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x08}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, 2                                             /* pri, cfi, VlanId */
};

/* Ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthertypePart = {
    0x3333
};

/* DATA of bypass packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* Bypass PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PACKET to send info */
static TGF_PACKET_STC prvTgfPacketInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                             /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

static GT_U32 prvTgfPclQuadDiffKeyLookupTestLookupsBitmap = 0xF;

GT_VOID prvTgfPclQuadDiffKeyLookupTestLookupsBitmapSet(GT_U32 bitmap)
{
    prvTgfPclQuadDiffKeyLookupTestLookupsBitmap = bitmap;
}

/**
* @internal prvTgfPclQuadDiffKeyLookupIPCL0Test function
* @endinternal
*
* @brief   IPCL Quad lookup of parallel different keys test
*/
GT_VOID prvTgfPclQuadDiffKeyLookupIPCL0Test
(
    GT_VOID
)
{
    PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC testCfg;
    PRV_TGF_TCAM_DIFF_KEYS_TEST_SAVE_CFG_STC testSave;

    cpssOsMemSet(&testCfg, 0, sizeof(testCfg));
    testCfg.devNum = prvTgfDevNum;
    testCfg.srcPort = prvTgfPortsArray[0];
    testCfg.dstPort = prvTgfPortsArray[1];
    testCfg.srcVid  = 2;
    testCfg.newVidArr[0] = 3;
    testCfg.newVidArr[1] = 4;
    testCfg.newVidArr[2] = 5;
    testCfg.newVidArr[3] = 6;
    testCfg.subLookupsBmp = prvTgfPclQuadDiffKeyLookupTestLookupsBitmap;
    testCfg.macDaBits_15_0_arr[0] = 0x1111;
    testCfg.macDaBits_15_0_arr[1] = 0x2222;
    testCfg.macDaBits_15_0_arr[2] = 0x3333;
    testCfg.macDaBits_15_0_arr[3] = 0x4444;
    testCfg.prvTgfPacketInfoPtr = &prvTgfPacketInfo;
    testCfg.pclPacketType = CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E;
    testCfg.pclDirection = CPSS_PCL_DIRECTION_INGRESS_E;
    testCfg.pclLookupNum = CPSS_PCL_LOOKUP_NUMBER_0_E;
    /* both UDB index and offset in the key */
    testCfg.pclUdbBaseIndex = 20;
    testCfg.superKeyFieldOffset = 20;
    testCfg.ruleFieldOffsetArr[0] = 2;
    testCfg.ruleFieldOffsetArr[1] = 4;
    testCfg.ruleFieldOffsetArr[2] = 6;
    testCfg.pclCfgPclId = 0;
    testCfg.pclCfgEmProfileId = 3;
    /* index measured in rule alignments */
    testCfg.ruleIndexArr[0] = 0;
    testCfg.ruleIndexArr[1] = 6;
    testCfg.ruleIndexArr[2] = ((256 * 12) + 0);
    testCfg.ruleIndexArr[3] = ((256 * 12) + 6);
    testCfg.ruleSizesArr[0] = 3;
    testCfg.ruleSizesArr[1] = 2;
    testCfg.ruleSizesArr[2] = 4;
    testCfg.ruleSizesArr[3] = 5;
    /**/
    testCfg.tcamClient = CPSS_DXCH_TCAM_IPCL_0_E;
    testCfg.tcamProfileId = 5;
    testCfg.tcamMuxTableLineIndexesArr[0] = 5;
    testCfg.tcamMuxTableLineIndexesArr[1] = 6;
    testCfg.tcamMuxTableLineIndexesArr[2] = 7;
    testCfg.tcamMuxTableLine10BOffsetsArr[0] = 1;
    testCfg.tcamMuxTableLine10BOffsetsArr[1] = 2;
    testCfg.tcamMuxTableLine10BOffsetsArr[2] = 3;

    prvTgfPclQuadDiffKeyLookupTestTcamCfgSave(&testCfg, &testSave);
    prvTgfPclQuadDiffKeyLookupTestTcamConfigure(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestPclConfigure(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestVlanAndTraffic(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestPclCfgRestore(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestTcamCfgRestore(&testCfg, &testSave);
}

/**
* @internal prvTgfPclQuadDiffKeyLookupIPCL1Test function
* @endinternal
*
* @brief   IPCL Quad lookup of parallel different keys test
*/
GT_VOID prvTgfPclQuadDiffKeyLookupIPCL1Test
(
    GT_VOID
)
{
    PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC testCfg;
    PRV_TGF_TCAM_DIFF_KEYS_TEST_SAVE_CFG_STC testSave;

    cpssOsMemSet(&testCfg, 0, sizeof(testCfg));
    testCfg.devNum = prvTgfDevNum;
    testCfg.srcPort = prvTgfPortsArray[0];
    testCfg.dstPort = prvTgfPortsArray[1];
    testCfg.srcVid  = 2;
    testCfg.newVidArr[0] = 3;
    testCfg.newVidArr[1] = 4;
    testCfg.newVidArr[2] = 5;
    testCfg.newVidArr[3] = 6;
    testCfg.subLookupsBmp = prvTgfPclQuadDiffKeyLookupTestLookupsBitmap;
    testCfg.macDaBits_15_0_arr[0] = 0x1234;
    testCfg.macDaBits_15_0_arr[1] = 0x5678;
    testCfg.macDaBits_15_0_arr[2] = 0xABCD;
    testCfg.macDaBits_15_0_arr[3] = 0x9753;
    testCfg.prvTgfPacketInfoPtr = &prvTgfPacketInfo;
    testCfg.pclPacketType = CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E;
    testCfg.pclDirection = CPSS_PCL_DIRECTION_INGRESS_E;
    testCfg.pclLookupNum = CPSS_PCL_LOOKUP_NUMBER_1_E;
    /* both UDB index and offset in the key */
    testCfg.pclUdbBaseIndex = 10;
    testCfg.superKeyFieldOffset = 8;
    testCfg.ruleFieldOffsetArr[0] = 2;
    testCfg.ruleFieldOffsetArr[1] = 8;
    testCfg.ruleFieldOffsetArr[2] = 12;
    testCfg.pclCfgPclId = 0;
    testCfg.pclCfgEmProfileId = 7;
    /* index measured in rule alignments */
    testCfg.ruleIndexArr[0] = 0;
    testCfg.ruleIndexArr[1] = 8 /*6*/;
    testCfg.ruleIndexArr[2] = ((256 * 12) + 0);
    testCfg.ruleIndexArr[3] = ((256 * 12) + 6);
    testCfg.ruleSizesArr[0] = 2;
    testCfg.ruleSizesArr[1] = 4;
    testCfg.ruleSizesArr[2] = 3;
    testCfg.ruleSizesArr[3] = 6;
    /**/
    testCfg.tcamClient = CPSS_DXCH_TCAM_IPCL_1_E;
    testCfg.tcamProfileId = 11;
    testCfg.tcamMuxTableLineIndexesArr[0] = 5;
    testCfg.tcamMuxTableLineIndexesArr[1] = 6;
    testCfg.tcamMuxTableLineIndexesArr[2] = 7;
    testCfg.tcamMuxTableLine10BOffsetsArr[0] = 2;
    testCfg.tcamMuxTableLine10BOffsetsArr[1] = 1;
    testCfg.tcamMuxTableLine10BOffsetsArr[2] = 0;

    prvTgfPclQuadDiffKeyLookupTestTcamCfgSave(&testCfg, &testSave);
    prvTgfPclQuadDiffKeyLookupTestTcamConfigure(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestPclConfigure(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestVlanAndTraffic(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestPclCfgRestore(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestTcamCfgRestore(&testCfg, &testSave);
}

/**
* @internal prvTgfPclQuadDiffKeyLookupIPCL2Test function
* @endinternal
*
* @brief   IPCL Quad lookup of parallel different keys test
*/
GT_VOID prvTgfPclQuadDiffKeyLookupIPCL2Test
(
    GT_VOID
)
{
    PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC testCfg;
    PRV_TGF_TCAM_DIFF_KEYS_TEST_SAVE_CFG_STC testSave;

    cpssOsMemSet(&testCfg, 0, sizeof(testCfg));
    testCfg.devNum = prvTgfDevNum;
    testCfg.srcPort = prvTgfPortsArray[0];
    testCfg.dstPort = prvTgfPortsArray[1];
    testCfg.srcVid  = 2;
    testCfg.newVidArr[0] = 3;
    testCfg.newVidArr[1] = 4;
    testCfg.newVidArr[2] = 5;
    testCfg.newVidArr[3] = 6;
    testCfg.subLookupsBmp = prvTgfPclQuadDiffKeyLookupTestLookupsBitmap;
    testCfg.macDaBits_15_0_arr[0] = 0xAAAA;
    testCfg.macDaBits_15_0_arr[1] = 0xBBBB;
    testCfg.macDaBits_15_0_arr[2] = 0xCCCC;
    testCfg.macDaBits_15_0_arr[3] = 0xDDDD;
    testCfg.prvTgfPacketInfoPtr = &prvTgfPacketInfo;
    testCfg.pclPacketType = CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E;
    testCfg.pclDirection = CPSS_PCL_DIRECTION_INGRESS_E;
    testCfg.pclLookupNum = CPSS_PCL_LOOKUP_NUMBER_2_E;
    /* both UDB index and offset in the key */
    testCfg.pclUdbBaseIndex = 2;
    testCfg.superKeyFieldOffset = 4;
    testCfg.ruleFieldOffsetArr[0] = 2;
    testCfg.ruleFieldOffsetArr[1] = 4;
    testCfg.ruleFieldOffsetArr[2] = 6;
    testCfg.pclCfgPclId = 0;
    testCfg.pclCfgEmProfileId = 3;
    /* index measured in rule alignments */
    testCfg.ruleIndexArr[0] = 0;
    testCfg.ruleIndexArr[1] = 6;
    testCfg.ruleIndexArr[2] = ((256 * 12) + 0);
    testCfg.ruleIndexArr[3] = ((256 * 12) + 6);
    testCfg.ruleSizesArr[0] = 3;
    testCfg.ruleSizesArr[1] = 5;
    testCfg.ruleSizesArr[2] = 4;
    testCfg.ruleSizesArr[3] = 2;
    /**/
    testCfg.tcamClient = CPSS_DXCH_TCAM_IPCL_2_E;
    testCfg.tcamProfileId = 5;
    testCfg.tcamMuxTableLineIndexesArr[0] = 11;
    testCfg.tcamMuxTableLineIndexesArr[1] = 12;
    testCfg.tcamMuxTableLineIndexesArr[2] = 13;
    testCfg.tcamMuxTableLine10BOffsetsArr[0] = 1;
    testCfg.tcamMuxTableLine10BOffsetsArr[1] = 2;
    testCfg.tcamMuxTableLine10BOffsetsArr[2] = 1;

    prvTgfPclQuadDiffKeyLookupTestTcamCfgSave(&testCfg, &testSave);
    prvTgfPclQuadDiffKeyLookupTestTcamConfigure(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestPclConfigure(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestVlanAndTraffic(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestPclCfgRestore(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestTcamCfgRestore(&testCfg, &testSave);
}

/**
* @internal prvTgfPclQuadDiffKeyLookupEPCLTest function
* @endinternal
*
* @brief   IPCL Quad lookup of parallel different keys test
*/
GT_VOID prvTgfPclQuadDiffKeyLookupEPCLTest
(
    GT_VOID
)
{
    PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC testCfg;
    PRV_TGF_TCAM_DIFF_KEYS_TEST_SAVE_CFG_STC testSave;

    cpssOsMemSet(&testCfg, 0, sizeof(testCfg));
    testCfg.devNum = prvTgfDevNum;
    testCfg.srcPort = prvTgfPortsArray[0];
    testCfg.dstPort = prvTgfPortsArray[1];
    testCfg.srcVid  = 2;
    testCfg.newVidArr[0] = 3;
    testCfg.newVidArr[1] = 4;
    testCfg.newVidArr[2] = 5;
    testCfg.newVidArr[3] = 6;
    testCfg.subLookupsBmp = prvTgfPclQuadDiffKeyLookupTestLookupsBitmap;
    testCfg.macDaBits_15_0_arr[0] = 0x1111;
    testCfg.macDaBits_15_0_arr[1] = 0x2222;
    testCfg.macDaBits_15_0_arr[2] = 0x3333;
    testCfg.macDaBits_15_0_arr[3] = 0x4444;
    testCfg.prvTgfPacketInfoPtr = &prvTgfPacketInfo;
    testCfg.pclPacketType = CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E;
    testCfg.pclDirection = CPSS_PCL_DIRECTION_EGRESS_E;
    testCfg.pclLookupNum = CPSS_PCL_LOOKUP_NUMBER_0_E;
    /* both UDB index and offset in the key */
    testCfg.pclUdbBaseIndex = 20;
    testCfg.superKeyFieldOffset = 20;
    testCfg.ruleFieldOffsetArr[0] = 2;
    testCfg.ruleFieldOffsetArr[1] = 4;
    testCfg.ruleFieldOffsetArr[2] = 6;
    testCfg.pclCfgPclId = 0;
    testCfg.pclCfgEmProfileId = 3;
    /* index measured in rule alignments */
    testCfg.ruleIndexArr[0] = 0;
    testCfg.ruleIndexArr[1] = 6;
    testCfg.ruleIndexArr[2] = ((256 * 12) + 0);
    testCfg.ruleIndexArr[3] = ((256 * 12) + 6);
    testCfg.ruleSizesArr[0] = 3;
    testCfg.ruleSizesArr[1] = 2;
    testCfg.ruleSizesArr[2] = 4;
    testCfg.ruleSizesArr[3] = 5;
    /**/
    testCfg.tcamClient = CPSS_DXCH_TCAM_EPCL_E;
    testCfg.tcamProfileId = 5;
    testCfg.tcamMuxTableLineIndexesArr[0] = 5;
    testCfg.tcamMuxTableLineIndexesArr[1] = 6;
    testCfg.tcamMuxTableLineIndexesArr[2] = 7;
    testCfg.tcamMuxTableLine10BOffsetsArr[0] = 1;
    testCfg.tcamMuxTableLine10BOffsetsArr[1] = 2;
    testCfg.tcamMuxTableLine10BOffsetsArr[2] = 3;

    prvTgfPclQuadDiffKeyLookupTestTcamCfgSave(&testCfg, &testSave);
    prvTgfPclQuadDiffKeyLookupTestTcamConfigure(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestPclConfigure(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestVlanAndTraffic(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestPclCfgRestore(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestTcamCfgRestore(&testCfg, &testSave);
}

/**
* @internal prvTgfPclQuadDiffKeyLookupTTIPortModeTest function
* @endinternal
*
* @brief   TTI Quad lookup of parallel different keys test
*/
GT_VOID prvTgfPclQuadDiffKeyLookupTTIPortModeTest
(
    GT_VOID
)
{
    PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC testCfg;
    PRV_TGF_TCAM_DIFF_KEYS_TEST_SAVE_CFG_STC testSave;

    cpssOsMemSet(&testCfg, 0, sizeof(testCfg));
    testCfg.devNum = prvTgfDevNum;
    testCfg.srcPort = prvTgfPortsArray[0];
    testCfg.dstPort = prvTgfPortsArray[1];
    testCfg.srcVid  = 2;
    testCfg.newVidArr[0] = 3;
    testCfg.newVidArr[1] = 4;
    testCfg.newVidArr[2] = 5;
    testCfg.newVidArr[3] = 6;
    testCfg.subLookupsBmp = prvTgfPclQuadDiffKeyLookupTestLookupsBitmap;
    testCfg.macDaBits_15_0_arr[0] = 0x1212;
    testCfg.macDaBits_15_0_arr[1] = 0x2323;
    testCfg.macDaBits_15_0_arr[2] = 0x3434;
    testCfg.macDaBits_15_0_arr[3] = 0x4545;
    testCfg.prvTgfPacketInfoPtr = &prvTgfPacketInfo;

    testCfg.ttiPacketType = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;
    testCfg.ttiUdbBaseIndex = 8;
    testCfg.ttiTcamProfileIdMode = CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_PER_PORT_E;
    testCfg.ttiTcamProfileIdPerPort = 5;

    testCfg.superKeyFieldOffset = testCfg.ttiUdbBaseIndex; /* for TTI must be the same */
    testCfg.ruleFieldOffsetArr[0] = 2;
    testCfg.ruleFieldOffsetArr[1] = 6;
    testCfg.ruleFieldOffsetArr[2] = 4;
    testCfg.pclCfgPclId = 0;
    testCfg.pclCfgEmProfileId = 3;
    /* index measured in rule alignments */
    testCfg.ruleIndexArr[0] = 0;
    testCfg.ruleIndexArr[1] = 6;
    testCfg.ruleIndexArr[2] = ((256 * 12) + 0);
    testCfg.ruleIndexArr[3] = ((256 * 12) + 6);
    testCfg.ruleSizesArr[0] = 3;
    testCfg.ruleSizesArr[1] = 2;
    testCfg.ruleSizesArr[2] = 3;
    testCfg.ruleSizesArr[3] = 2;
    /**/
    testCfg.tcamClient = CPSS_DXCH_TCAM_TTI_E;
    testCfg.tcamProfileId = 9;
    testCfg.tcamMuxTableLineIndexesArr[0] = 8;
    testCfg.tcamMuxTableLineIndexesArr[1] = 9;
    testCfg.tcamMuxTableLineIndexesArr[2] = 10;
    testCfg.tcamMuxTableLine10BOffsetsArr[0] = 1;
    testCfg.tcamMuxTableLine10BOffsetsArr[1] = 2;
    testCfg.tcamMuxTableLine10BOffsetsArr[2] = 3;

    prvTgfPclQuadDiffKeyLookupTestTcamCfgSave(&testCfg, &testSave);
    prvTgfPclQuadDiffKeyLookupTestTcamConfigure(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestTtiConfigure(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestVlanAndTraffic(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestTtiCfgRestore(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestTcamCfgRestore(&testCfg, &testSave);
}

/**
* @internal prvTgfPclQuadDiffKeyLookupTTIPktTypeModeTest function
* @endinternal
*
* @brief   TTI Quad lookup of parallel different keys test
*/
GT_VOID prvTgfPclQuadDiffKeyLookupTTIPktTypeModeTest
(
    GT_VOID
)
{
    PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC testCfg;
    PRV_TGF_TCAM_DIFF_KEYS_TEST_SAVE_CFG_STC testSave;

    cpssOsMemSet(&testCfg, 0, sizeof(testCfg));
    testCfg.devNum = prvTgfDevNum;
    testCfg.srcPort = prvTgfPortsArray[0];
    testCfg.dstPort = prvTgfPortsArray[1];
    testCfg.srcVid  = 2;
    testCfg.newVidArr[0] = 3;
    testCfg.newVidArr[1] = 4;
    testCfg.newVidArr[2] = 5;
    testCfg.newVidArr[3] = 6;
    testCfg.subLookupsBmp = prvTgfPclQuadDiffKeyLookupTestLookupsBitmap;
    testCfg.macDaBits_15_0_arr[0] = 0x1212;
    testCfg.macDaBits_15_0_arr[1] = 0x2323;
    testCfg.macDaBits_15_0_arr[2] = 0x3434;
    testCfg.macDaBits_15_0_arr[3] = 0x4545;
    testCfg.prvTgfPacketInfoPtr = &prvTgfPacketInfo;

    testCfg.ttiPacketType = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;
    testCfg.ttiUdbBaseIndex = 8;
    testCfg.ttiTcamProfileIdMode = CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_PER_PACKET_TYPE_E;
    testCfg.ttiTcamProfileIdPerPort = 5;

    testCfg.superKeyFieldOffset = testCfg.ttiUdbBaseIndex; /* for TTI must be the same */
    testCfg.ruleFieldOffsetArr[0] = 2;
    testCfg.ruleFieldOffsetArr[1] = 6;
    testCfg.ruleFieldOffsetArr[2] = 4;
    testCfg.pclCfgPclId = 0;
    testCfg.pclCfgEmProfileId = 3;
    /* index measured in rule alignments */
    testCfg.ruleIndexArr[0] = 0;
    testCfg.ruleIndexArr[1] = 6;
    testCfg.ruleIndexArr[2] = ((256 * 12) + 0);
    testCfg.ruleIndexArr[3] = ((256 * 12) + 6);
    testCfg.ruleSizesArr[0] = 3;
    testCfg.ruleSizesArr[1] = 2;
    testCfg.ruleSizesArr[2] = 3;
    testCfg.ruleSizesArr[3] = 2;
    /**/
    testCfg.tcamClient = CPSS_DXCH_TCAM_TTI_E;
    testCfg.tcamProfileId = 9;
    testCfg.tcamMuxTableLineIndexesArr[0] = 8;
    testCfg.tcamMuxTableLineIndexesArr[1] = 9;
    testCfg.tcamMuxTableLineIndexesArr[2] = 10;
    testCfg.tcamMuxTableLine10BOffsetsArr[0] = 1;
    testCfg.tcamMuxTableLine10BOffsetsArr[1] = 2;
    testCfg.tcamMuxTableLine10BOffsetsArr[2] = 3;

    prvTgfPclQuadDiffKeyLookupTestTcamCfgSave(&testCfg, &testSave);
    prvTgfPclQuadDiffKeyLookupTestTcamConfigure(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestTtiConfigure(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestVlanAndTraffic(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestTtiCfgRestore(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestTcamCfgRestore(&testCfg, &testSave);
}

/**
* @internal prvTgfPclQuadDiffKeyLookupTTIPortAndPktTypeModeTest function
* @endinternal
*
* @brief   TTI Quad lookup of parallel different keys test
*/
GT_VOID prvTgfPclQuadDiffKeyLookupTTIPortAndPktTypeModeTest
(
    GT_VOID
)
{
    PRV_TGF_TCAM_DIFF_KEYS_TEST_PARAM_STC testCfg;
    PRV_TGF_TCAM_DIFF_KEYS_TEST_SAVE_CFG_STC testSave;

    cpssOsMemSet(&testCfg, 0, sizeof(testCfg));
    testCfg.devNum = prvTgfDevNum;
    testCfg.srcPort = prvTgfPortsArray[0];
    testCfg.dstPort = prvTgfPortsArray[1];
    testCfg.srcVid  = 2;
    testCfg.newVidArr[0] = 3;
    testCfg.newVidArr[1] = 4;
    testCfg.newVidArr[2] = 5;
    testCfg.newVidArr[3] = 6;
    testCfg.subLookupsBmp = prvTgfPclQuadDiffKeyLookupTestLookupsBitmap;
    testCfg.macDaBits_15_0_arr[0] = 0x1212;
    testCfg.macDaBits_15_0_arr[1] = 0x2323;
    testCfg.macDaBits_15_0_arr[2] = 0x3434;
    testCfg.macDaBits_15_0_arr[3] = 0x4545;
    testCfg.prvTgfPacketInfoPtr = &prvTgfPacketInfo;

    testCfg.ttiPacketType = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;
    testCfg.ttiUdbBaseIndex = 8;
    testCfg.ttiTcamProfileIdMode = CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_PER_PORT_AND_PACKET_TYPE_E;
    testCfg.ttiTcamProfileIdPerPort = 5;

    testCfg.superKeyFieldOffset = testCfg.ttiUdbBaseIndex; /* for TTI must be the same */
    testCfg.ruleFieldOffsetArr[0] = 2;
    testCfg.ruleFieldOffsetArr[1] = 6;
    testCfg.ruleFieldOffsetArr[2] = 4;
    testCfg.pclCfgPclId = 0;
    testCfg.pclCfgEmProfileId = 3;
    /* index measured in rule alignments */
    testCfg.ruleIndexArr[0] = 0;
    testCfg.ruleIndexArr[1] = 6;
    testCfg.ruleIndexArr[2] = ((256 * 12) + 0);
    testCfg.ruleIndexArr[3] = ((256 * 12) + 6);
    testCfg.ruleSizesArr[0] = 3;
    testCfg.ruleSizesArr[1] = 2;
    testCfg.ruleSizesArr[2] = 3;
    testCfg.ruleSizesArr[3] = 2;
    /**/
    testCfg.tcamClient = CPSS_DXCH_TCAM_TTI_E;
    testCfg.tcamProfileId = 9;
    testCfg.tcamMuxTableLineIndexesArr[0] = 8;
    testCfg.tcamMuxTableLineIndexesArr[1] = 9;
    testCfg.tcamMuxTableLineIndexesArr[2] = 10;
    testCfg.tcamMuxTableLine10BOffsetsArr[0] = 1;
    testCfg.tcamMuxTableLine10BOffsetsArr[1] = 2;
    testCfg.tcamMuxTableLine10BOffsetsArr[2] = 3;

    prvTgfPclQuadDiffKeyLookupTestTcamCfgSave(&testCfg, &testSave);
    prvTgfPclQuadDiffKeyLookupTestTcamConfigure(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestTtiConfigure(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestVlanAndTraffic(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestTtiCfgRestore(&testCfg);
    prvTgfPclQuadDiffKeyLookupTestTcamCfgRestore(&testCfg, &testSave);
}





