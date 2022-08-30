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
* @file prvTgfGlobalEport.c
*
* @brief L2 MLL Global ePort test
*
* @version   8
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfL2MllGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfConfigGen.h>
#include <common/tgfCscdGen.h>
#include <common/tgfIpGen.h>

#include <l2mll/prvTgfL2MllUtils.h>
#include <l2mll/prvTgfLttEvidxRange.h>
#include <common/tgfTrunkGen.h>
#include <trunk/prvTgfTrunk.h>
#include <common/tgfBridgeL2EcmpGen.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS                             1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS                      0

/* port number to receive traffic from */
#define PRV_TGF_RECV_PORT_IDX_CNS                      3

/* trunk port number to receive traffic from */
#define PRV_TGF_TRUNK_PORT_IDX_CNS                     2

/* target device */
#define PRV_TGF_TRG_DEV_CNS                            1

/* MLL pair entry index */
#define PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS               20

#define PRV_TGF_MLL_LTT_INDEX_CNS                      8
#define PRV_TGF_MLL_VIDX_INDEX_CNS                     9

/* Cascade link indexes used for device map table entry */
#define CASCADE_LINK_NUMBER1_CNS                       1

/* number of words that the FDB device table is using */
#define PRV_TGF_FDB_DEV_TABLE_SIZE_CNS                 4

/* addresses for incremental SA */
#define FDB_MAC_COUNT_CNS                              2

/* default number of packets to send */
static GT_U32  prvTgfBurstCount              = 1;

/* number of packets to send for MLL Ping Pong */
static GT_U32  prvTgfMllPingPongBurstCount   = 2;

/* remote HW Device Number */
static GT_HW_DEV_NUM  remoteHwDevNum;

/* Parameters needed to be restored */
static PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT               prvTgfSavedSrcPortTrunkHashEn;
static CPSS_CSCD_LINK_TYPE_STC                        prvTgfSavedCscdLinkType;

/*************************** Restore config ***********************************/
/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TRUNK_LBH_CRC_MODE_ENT                            crcHashModeGet;
    GT_U32                                                    crcSeedGet;
    CPSS_CSCD_LINK_TYPE_STC                                   cascadeLink;
    PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT                          srcPortTrunkHashEn;
    PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT                         globalHashModeGet;
    CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT selectionModeGet;
    PRV_TGF_L2_MLL_PAIR_STC                                   mllPairEntry;
    GT_BOOL                                                   eVidxEnableRestore;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC                              lttEntry;
    PRV_TGF_DEV_MAP_LOOKUP_MODE_ENT                           lookupModeGet;
    GT_U32                                                    startBit;
    GT_U32                                                    numBits;
    CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_ENT            replicationMode;
    CPSS_NET_RX_CPU_CODE_ENT                                  replicationDropCode;
    CPSS_INTERFACE_INFO_STC                                   origPhysicalInfo;
} prvTgfRestoreCfg;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x00, 0x00, 0x00, 0x00, 0x01},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x99}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};


/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
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
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfGlobalEportConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfGlobalEportConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC        lttEntry;
    PRV_TGF_PAIR_READ_WRITE_FORM_ENT    mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC             mllPairEntry;
    GT_U32                              portsArray[1];
    CPSS_CSCD_LINK_TYPE_STC             cascadeLink;

    cpssOsMemSet(&cascadeLink, 0, sizeof(cascadeLink));

    /* AUTODOC: SETUP CONFIGURATION: */

    /* Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* save the current cascade map table entry */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, PRV_TGF_TRG_DEV_CNS,
                                  0,
                                  0,
                                  &prvTgfSavedCscdLinkType,
                                  &prvTgfSavedSrcPortTrunkHashEn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet");

    /* create eVidx 1 */
    portsArray[0] = prvTgfPortsArray[1];

    /* AUTODOC: create eVIDX 1 with ports [1] */
    rc = prvTgfBrgVidxEntrySet(1, portsArray, NULL, 1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, 1);

    /* AUTODOC: add FDB entry with MAC 01:00:00:00:00:01, VLAN 1, eVIDX 1 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_CNS,
                                          1, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: eVidx 1");

    /* create L2 MLL LTT entries */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0;

    /* AUTODOC: add L2 MLL LTT entry 1 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=False, mllMaskProfile=0 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 1, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 1");

    /* create L2 MLL entries */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E;
    mllPairEntry.nextPointer = 0;
    mllPairEntry.entrySelector=PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    mllPairEntry.firstMllNode.last = GT_TRUE;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum + 1;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    /* AUTODOC: add L2 MLL entry 0: */
    /* AUTODOC:   nextMllPointer=0, dev=1, port=0, last=True */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 0, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 0");

    /* AUTODOC: set link up interface map as the cascade trunk with trunk number 0 for egress filtering */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    cascadeLink.linkNum  = 0;

    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,PRV_TGF_TRG_DEV_CNS,
                                  0, 0,
                                  &cascadeLink, 0, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

}

/**
* @internal prvTgfGlobalEportTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfGlobalEportTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_L2_MLL_EXCEPTION_COUNTERS_STC  counters;
    PRV_TGF_CFG_GLOBAL_EPORT_STC ecmp;
    PRV_TGF_CFG_GLOBAL_EPORT_STC global;
    PRV_TGF_CFG_GLOBAL_EPORT_STC dlb;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: enable MLL lookup for all multi-target packets */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d",
                                 GT_TRUE);

    /* clear global ePorts */
    ecmp.enable = PRV_TGF_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;
    ecmp.pattern = 0x0;
    ecmp.mask = 0x0;
    ecmp.minValue = 0x0;
    ecmp.maxValue = 0x0;

    global.enable = PRV_TGF_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;
    global.pattern = 0x0;
    global.mask = 0x0;
    global.minValue = 0x0;
    global.maxValue = 0x0;

    dlb.enable = PRV_TGF_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;
    dlb.pattern = 0x0;
    dlb.mask = 0x0;
    dlb.maxValue = 0x0;
    dlb.maxValue = 0x0;

    /* AUTODOC: clear Global ePorts */
    rc = prvTgfCfgGlobalEportSet(prvTgfDevNum, &global , &ecmp, &dlb);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCfgGlobalEportSet: dev %d",
                                 prvTgfDevNum);

    /* clear Skip exception counter */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* read (& clear) Skip exception counter */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");

    /* AUTODOC: check L2 MLL exception Skip counter is 0 */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, counters.skip,
                                 "Skip counter different then expected");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* set prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] as global ePort */
    ecmp.enable = PRV_TGF_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E;
    ecmp.pattern = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    ecmp.mask = (GT_U32) -1;
    ecmp.minValue = 0x0;
    ecmp.maxValue = 0x0;
    global = ecmp;
    dlb.enable = PRV_TGF_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;

    /* AUTODOC: configure Global ePorts: */
    /* AUTODOC:   ECMP primary ePorts port=1 */
    rc = prvTgfCfgGlobalEportSet(prvTgfDevNum, &global , &ecmp, &dlb);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCfgGlobalEportSet: dev %d",
                                 prvTgfDevNum);

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* read (& clear) Skip exception counter */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");

    /* AUTODOC: check L2 MLL exception Skip counter is 1 */
    UTF_VERIFY_EQUAL0_STRING_MAC(1, counters.skip,
                                 "Skip counter different then expected");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* clear global ePorts */
    ecmp.enable = PRV_TGF_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;
    global.enable = PRV_TGF_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;
    dlb.enable = PRV_TGF_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;

    /* AUTODOC: clear Global ePorts */
    rc = prvTgfCfgGlobalEportSet(prvTgfDevNum, &global , &ecmp, &dlb);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCfgGlobalEportSet: dev %d",
                                 prvTgfDevNum);
}

/**
* @internal prvTgfGlobalEportConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfGlobalEportConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    lttIndexArray[1];
    GT_U32    mllIndexArray[1];

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: clear eVidx 1 */
    rc = prvTgfBrgVidxEntrySet(1, NULL, NULL, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, 1);

    lttIndexArray[0] = 0;
    mllIndexArray[0] = 0;

    /* AUTODOC: clear L2 MLL and LTT entries */
    prvTgfL2MllUtilsMllAndLttEntriesClear(lttIndexArray, 1, mllIndexArray, 1);

    /* AUTODOC: restore default L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationRestore();

    /* AUTODOC: restore the cascade map table's entry */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_TRG_DEV_CNS,
                                  0,
                                  0,
                                  &prvTgfSavedCscdLinkType,
                                  prvTgfSavedSrcPortTrunkHashEn,
                                  GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

}

/**
* @internal prvTgfMllPingPongConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfMllPingPongConfigurationSet
(
    GT_VOID
)
{
    PRV_TGF_L2_MLL_LTT_ENTRY_STC                  l2LttEntry;
    PRV_TGF_PAIR_READ_WRITE_FORM_ENT              mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC                       mllPairEntry;
    static CPSS_TRUNK_MEMBER_STC                  enabledMembersArray[2];
    GT_STATUS                                     rc;/*return code*/
    PRV_TGF_PCL_PACKET_TYPE_ENT                   trafficType;
    GT_U32                                        ii;
    PRV_TGF_TRUNK_LBH_CRC_MASK_ENTRY_STC          maskEntry;
    CPSS_CSCD_LINK_TYPE_STC                       cascadeLink; /* cascade link info */
    GT_TRUNK_ID                                   trunkId;
    CPSS_INTERFACE_INFO_STC                       physicalInfo;
    GT_HW_DEV_NUM                                 hwDevNum;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* Set Lookup Mode for Device Map Table */
    rc =  prvTgfCscdDevMapLookupModeGet(prvTgfDevNum,&prvTgfRestoreCfg.lookupModeGet);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc = prvTgfCscdDevMapLookupModeSet(PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc = cpssDxChCfgHwDevNumGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet \n");
    remoteHwDevNum = (hwDevNum + 1) % BIT_12;

    cpssOsMemSet(&cascadeLink, 0, sizeof(cascadeLink));
    /* set mac entries on 'Remote device' so we can point this device via the
       cascade port */
    /* cascade link info */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;

    /* save the current cascade map table */
    for(ii = 0 ;ii < FDB_MAC_COUNT_CNS;ii++)
    {
        rc = prvTgfCscdDevMapTableGet(prvTgfDevNum,remoteHwDevNum,
                0,0,
                &(prvTgfRestoreCfg.cascadeLink),&(prvTgfRestoreCfg.srcPortTrunkHashEn));
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    /* set this 'Remote device' to point to the device map table via cascade port */
    for(ii = 0 ;ii < FDB_MAC_COUNT_CNS;ii++)
    {
        cascadeLink.linkNum = prvTgfPortsArray[CASCADE_LINK_NUMBER1_CNS];
        rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,remoteHwDevNum,
                0,0,&cascadeLink,
                GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    trafficType = PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E;

    /* Get the general hashing mode of trunk hash generation as CRC Hash based on the packet's data*/
    rc =  prvTgfTrunkHashGlobalModeGet(prvTgfDevNum,&prvTgfRestoreCfg.globalHashModeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeGet: %d", prvTgfDevNum);

    /* Set the general hashing mode of trunk hash generation as CRC Hash based on the packet's data*/
    rc =  prvTgfTrunkHashGlobalModeSet(PRV_TGF_TRUNK_LBH_PACKETS_INFO_CRC_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeSet: %d", prvTgfDevNum);

    /* get crc hash parameters */
    rc =  prvTgfTrunkHashCrcParametersGet(prvTgfDevNum, &prvTgfRestoreCfg.crcHashModeGet, &prvTgfRestoreCfg.crcSeedGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashCrcParametersGet: %d", prvTgfDevNum);

    /* set Enhanced CRC-Based Hash Mode */
    rc =  prvTgfTrunkHashCrcParametersSet(PRV_TGF_TRUNK_LBH_CRC_32_MODE_E, 0/*crcSeed*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashCrcParametersSet: %d", prvTgfDevNum);

    cpssOsMemSet(&maskEntry,0,sizeof(maskEntry));
    maskEntry.macDaMaskBmp     = 0x3f;
    maskEntry.macSaMaskBmp     = 0x3f;

    /* set HASH mask */
    rc =  prvTgfTrunkHashMaskCrcEntrySet(GT_FALSE,0,0,trafficType,&maskEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashMaskCrcEntrySet: %d", prvTgfDevNum);

    /* get ingress to egress pipe hash bits passed */
    rc = cpssDxChCfgEgressHashSelectionModeGet(prvTgfDevNum, &prvTgfRestoreCfg.selectionModeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChCfgEgressHashSelectionModeGet: %d", prvTgfDevNum);

    /* set ingress to egress pipe hash bits passed */
    /* BC2, Caelum has default value 6LSB bits unlike other higher SIP5 devices
     * which have default mode of 12bits.
     */
    rc = cpssDxChCfgEgressHashSelectionModeSet(prvTgfDevNum, CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_12_BITS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChCfgEgressHashSelectionModeSet: %d", prvTgfDevNum);

    /* Set hash selection bits */
    rc = prvTgfTrunkHashNumBitsGet(prvTgfDevNum,
        PRV_TGF_TRUNK_HASH_CLIENT_TYPE_TRUNK_E, &prvTgfRestoreCfg.startBit, &prvTgfRestoreCfg.numBits);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashNumBitsGet %d %d",
        prvTgfDevNum, PRV_TGF_TRUNK_HASH_CLIENT_TYPE_TRUNK_E);

    rc = prvTgfTrunkHashNumBitsSet(prvTgfDevNum,
        PRV_TGF_TRUNK_HASH_CLIENT_TYPE_TRUNK_E, 0, 12);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashNumBitsSet %d %d %d %d",
        prvTgfDevNum, PRV_TGF_TRUNK_HASH_CLIENT_TYPE_TRUNK_E, 0, 12);

    /* AUTODOC: SETUP CONFIGURATION: */
    enabledMembersArray[0].hwDevice = prvTgfDevNum;
    enabledMembersArray[0].port     = prvTgfPortsArray[PRV_TGF_TRUNK_PORT_IDX_CNS];
    enabledMembersArray[1].hwDevice = remoteHwDevNum;
    enabledMembersArray[1].port     = prvTgfPortsArray[PRV_TGF_TRUNK_PORT_IDX_CNS];

    trunkId = PRV_TGF_MLL_VIDX_INDEX_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* AUTODOC: create trunk 25 with ports [0,2] */
    rc = prvTgfTrunkMembersSet(
        trunkId /*trunkId*/,
        2 /*numOfEnabledMembers*/,
        enabledMembersArray,
        0 /*numOfDisabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTrunkMembersSet");

    /* save ePort mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           prvTgfPortsArray[PRV_TGF_TRUNK_PORT_IDX_CNS],
                                                           &prvTgfRestoreCfg.origPhysicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableGet: "
                                            "dev %d, ePort %d",
                                            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TRUNK_PORT_IDX_CNS]);

    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_TRUNK_PORT_IDX_CNS];

    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TRUNK_PORT_IDX_CNS],
                                                           &physicalInfo);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,rc);


    /* AUTODOC: enable MLL lookup for all multi-target packets */
    rc = prvTgfL2MllLookupForAllEvidxEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.eVidxEnableRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableGet: enable %d",
                                 GT_FALSE);
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d",
                                 GT_FALSE);

    /* create L2 MLL LTT entries */
    rc = prvTgfL2MllLttEntryGet(prvTgfDevNum, PRV_TGF_MLL_LTT_INDEX_CNS, &prvTgfRestoreCfg.lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntryGet: index 0");

    cpssOsMemSet(&l2LttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    l2LttEntry.mllPointer = PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS;
    l2LttEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
    /* no mask profile - replicate to all:  */
    /* port 1                               */
    /* vidx 1 - ports 1                     */
    l2LttEntry.mllMaskProfileEnable = GT_FALSE;
    l2LttEntry.mllMaskProfile       = 0;

    /* AUTODOC: add L2 MLL LTT entry 0 with: */
    /* AUTODOC:   mllPointer=20, mllMaskProfileEnable=False, mllMaskProfile=0 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, PRV_TGF_MLL_LTT_INDEX_CNS, &l2LttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* AUTODOC: Store L2 MLL entry */
    rc = prvTgfL2MllPairRead(prvTgfDevNum, PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS,
                             PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                             &prvTgfRestoreCfg.mllPairEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairRead: %d", prvTgfDevNum);

    /* create L2 MLL entries */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_WHOLE_E;
    mllPairEntry.nextPointer = 1;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    /* AUTODOC:   nextMllPointer=1, maskBitmap=0x0 */
    mllPairEntry.firstMllNode.last = GT_FALSE;
    mllPairEntry.firstMllNode.maskBitmap = 0x0;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_TRUNK_E;
    mllPairEntry.firstMllNode.egressInterface.trunkId = PRV_TGF_MLL_VIDX_INDEX_CNS;

    mllPairEntry.secondMllNode.last = GT_TRUE;
    mllPairEntry.secondMllNode.maskBitmap = 0x0;
    mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.secondMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];

    /* AUTODOC: add L2 MLL entry 0: */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 0");
}

/**
* @internal prvTgfMllPingPongTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfMllPingPongTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                              rc;
    PRV_TGF_L2_MLL_EXCEPTION_COUNTERS_STC  counters;
    PRV_TGF_BRG_MAC_ENTRY_STC              macEntry;
    CPSS_PORT_MAC_COUNTER_SET_STC          portCntrs;
    GT_U8                                  portIter = 0;
    GT_U32                                 numVfd = 1;/* number of VFDs in vfdArray */
    TGF_VFD_INFO_STC                       vfdArray[1];/* vfd Array -- used for increment the tested bytes */
    GT_U32                                 txPacketsCount    = 0;
    GT_U32                                 stageNum;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear Skip exception counter */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: create VLAN 5 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);

    /* add a MAC entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: add FDB entry with MAC 01:00:00:00:00:22, prvtgfVlanId 2 and vidx 8 */
    /* make sure that ALL the mac SA that will be sent during the test will
       be known in the FDB on the needed vlans */
    macEntry.key.entryType                  = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_VIDX_E;
    macEntry.dstInterface.vidx              = PRV_TGF_MLL_LTT_INDEX_CNS;
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                        = GT_FALSE;
    macEntry.sourceId                       = 0;
    macEntry.userDefined                    = 0;

    /* AUTODOC: add FDB entry with MAC 01:00:00:00:00:22, prvtgfVlanId 2 and vidx 8 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 6 + 5;/* last byte of the mac SA */
    cpssOsMemSet(vfdArray[0].patternPtr,0,sizeof(TGF_MAC_ADDR));
    vfdArray[0].patternPtr[0] = 0;
    vfdArray[0].cycleCount = 1;/*single byte*/
    vfdArray[0].mode = TGF_VFD_MODE_INCREMENT_E;

    numVfd = 1;

    /*Save L2 ECMP MLL Replication mode and Replication drop code settings */
    rc = prvTgfBrgL2EcmpMllReplicationConfigGet(prvTgfDevNum, &prvTgfRestoreCfg.replicationMode, &prvTgfRestoreCfg.replicationDropCode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpMllReplicationConfigGet: %d", prvTgfDevNum);

    /* AUTODOC: Iterate thru 2 stages: */
    for (stageNum = 0; stageNum < 2; stageNum++ )
    {
        /* AUTODOC: for Stage#1 do the following: */
        if (stageNum == 1)
        {
            /* L2 ECMP MLL Replication mode and Replication drop code settings */
            rc = prvTgfBrgL2EcmpMllReplicationConfigSet(prvTgfDevNum, CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_CASCADED_SYSTEM_E, CPSS_NET_FIRST_USER_DEFINED_E);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpMllReplicationConfigSet: %d", prvTgfDevNum);
        }

        /* AUTODOC: send Ethernet packet from port 0 with: */
        /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
        /* setup Packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfMllPingPongBurstCount, numVfd, &vfdArray[0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

        /* send Packet from port portNum */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[0]);

        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
            switch(portIter)
            {
                case 0: /* port 0 */ txPacketsCount = 2;
                        break;
                case 1: /* port 1 */ txPacketsCount = (stageNum == 1) ? 0 : 1;
                        break;
                case 2: /* port 2 */ txPacketsCount = 1;
                        break;
                case 3: /* port 3 */ txPacketsCount = 2;
                        break;
                default: txPacketsCount = 0;
                         break;
            }

            UTF_VERIFY_EQUAL2_STRING_MAC(txPacketsCount, portCntrs.goodPktsSent.l[0],
                                         "get another Tx packets than expected on port[%d] = %d",
                                         portIter, prvTgfPortsArray[portIter]);
        }
    }
}

/**
* @internal prvTgfMllPingPongConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfMllPingPongConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                            rc;
    GT_U32                               ii;
    GT_TRUNK_ID                          trunkId;
    PRV_TGF_TRUNK_LBH_CRC_MASK_ENTRY_STC maskEntry;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           prvTgfPortsArray[PRV_TGF_TRUNK_PORT_IDX_CNS],
                                                           &prvTgfRestoreCfg.origPhysicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: "
                                            "dev %d, ePort %d",
                                            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TRUNK_PORT_IDX_CNS]);

    /* Restore Lookup Mode for Device Map Table */
    rc =  prvTgfCscdDevMapLookupModeSet(prvTgfRestoreCfg.lookupModeGet);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* Restore the current cascade map table */
    for(ii = 0 ;ii < FDB_MAC_COUNT_CNS;ii++)
    {
        rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,remoteHwDevNum,
                0,0,
                &(prvTgfRestoreCfg.cascadeLink),(prvTgfRestoreCfg.srcPortTrunkHashEn), GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet: %d", prvTgfDevNum);
    }

    /* Restore the general hashing mode of trunk hash generation as CRC Hash based on the packet's data*/
    rc =  prvTgfTrunkHashGlobalModeSet(prvTgfRestoreCfg.globalHashModeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeSet: %d", prvTgfDevNum);

    /* Restore crc hash parameters */
    rc =  prvTgfTrunkHashCrcParametersSet(prvTgfRestoreCfg.crcHashModeGet, prvTgfRestoreCfg.crcSeedGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashCrcParametersSet: %d", prvTgfDevNum);

    /* Restore ingress to egress pipe hash bits passed */
    rc = cpssDxChCfgEgressHashSelectionModeSet(prvTgfDevNum, prvTgfRestoreCfg.selectionModeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChCfgEgressHashSelectionModeSet: %d", prvTgfDevNum);

    /* Restore hash selection bits */
    rc = prvTgfTrunkHashNumBitsSet(prvTgfDevNum,
        PRV_TGF_TRUNK_HASH_CLIENT_TYPE_TRUNK_E, prvTgfRestoreCfg.startBit, prvTgfRestoreCfg.numBits);

    cpssOsMemSet(&maskEntry,0,sizeof(maskEntry));

    /* Restore HASH mask */
    rc =  prvTgfTrunkHashMaskCrcEntrySet(GT_FALSE,0,0,PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,&maskEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashMaskCrcEntrySet: %d", prvTgfDevNum);

    trunkId = PRV_TGF_MLL_VIDX_INDEX_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    rc = prvTgfTrunkMembersSet(
        trunkId /*trunkId*/,
        0 /*numOfEnabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL /*enabledMembersArray*/,
        0 /*numOfDisabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTrunkMembersSet");

    /* Restore L2 ECMP MLL Configuration */
    rc = prvTgfBrgL2EcmpMllReplicationConfigSet(prvTgfDevNum, prvTgfRestoreCfg.replicationMode, prvTgfRestoreCfg.replicationDropCode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpMllReplicationConfigSet: %d", prvTgfDevNum);

    /* Restore L2 MLL LTT entries */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, PRV_TGF_MLL_LTT_INDEX_CNS, &prvTgfRestoreCfg.lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet");

    /* AUTODOC: Restore L2 MLL entry */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS,
                             PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                             &prvTgfRestoreCfg.mllPairEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: %d", prvTgfDevNum);

    /* AUTODOC: restore default L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationRestore();

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

}

