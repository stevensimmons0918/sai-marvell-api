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
* @file prvTgfBasicIpv4UcRouting2Vrf.c
*
* @brief Basic IPV4 UC Routing
*
* @version   60
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTrunkGen.h>
#include <ip/prvTgfBasicIpv4UcRouting.h>
#include <ip/prvTgfBasicIpv4UcRouting2Vrf.h>
#include <trunk/prvTgfTrunk.h>
#include <common/tgfPacketGen.h>
#include <common/tgfCscdGen.h>
#include <common/tgfConfigGen.h>
#include <common/tgfLogicalTargetGen.h>
#include <gtOs/gtOsMem.h>
#include <extUtils/auEventHandler/auFdbUpdateLock.h>

/* indication that this is stacking system */
static PRV_TGF_IPV4_STACKING_SYSTEM_ENT tgfStackingSystemType = PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E;

extern GT_STATUS prvWrAppTrunkEPortGet_phy1690_WA_B(
    IN GT_TRUNK_ID              trunkId,
    OUT GT_PORT_NUM              *globalEPortPtr
);
extern void prvTgfCaptureForceTtiDisableModeSet(
    IN GT_BOOL                  forceTtiLookupUnchanged
);
extern GT_STATUS prvTgfDxChPclCaptureSet
(
    IN GT_U8    devNum,
    IN GT_PORT_NUM   portNum,
    IN GT_BOOL  start
);

/* hold all data of VLAN entry restore */
/** ports members bitmap */
static CPSS_PORTS_BMP_STC portsMembersStoreCfg;

/** ports tagging bitmap */
static CPSS_PORTS_BMP_STC portsTaggingStoreCfg;

/** is valid flag. */
static GT_BOOL isValidStoreCfg;

/** VLAN info */
static CPSS_DXCH_BRG_VLAN_INFO_STC vlanInfoStoreCfg;

/** tagging info */
static CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC taggingCmdStoreCfg;




/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* default number of packets to send */
#define PRV_TGF_BURST_COUNT_DEFAULT_CNS     1

/* nextHop VLAN Id  */
static GT_U16 prvTgfNextHopeVlanid = PRV_TGF_NEXTHOPE_VLANID_CNS;

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = PRV_TGF_BURST_COUNT_DEFAULT_CNS;

/* VLANs array */
static GT_U16         prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS};

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};
static TGF_MAC_ADDR  prvTgfArpMac1 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};


/* Restored Values */
static CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT globalHashModeGet;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
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
/* packet's IPv4 - used for bulk tests */
static TGF_PACKET_IPV4_STC prvTgfPacket2Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,  /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 3,  2,  2,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

#define VLAN_TAG_INFO_INDEX  1  /* index in prvTgfPacketPartArray[],prvTgfPacket2PartArray[]*/
#define DSA_INFO_INDEX  2       /* index in prvTgfPacketPartArray[],prvTgfPacket2PartArray[]*/

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_SKIP_E ,      NULL},/*reserved for DSA tag (instead of the vlan tag)*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet - used for bulk tests */
static TGF_PACKET_PART_STC prvTgfPacket2PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_SKIP_E ,      NULL},/*reserved for DSA tag (instead of the vlan tag)*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket2Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/*capture type*/
static TGF_CAPTURE_MODE_ENT captureType = TGF_CAPTURE_MODE_MIRRORING_E;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBasicIpv4UcRoutingBaseConfiguration2VrfSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] fdbPortNum - the FDB port num to set in the macEntry destination Interface None
*/
GT_VOID prvTgfBasicIpv4UcRoutingBaseConfiguration2VrfSet
(
    GT_U32   fdbPortNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U8                                   tagArray[] = {0, 0, 1, 1};
    CPSS_PORTS_BMP_STC                      portsMembers;
    CPSS_PORTS_BMP_STC                      portsTagging;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC    portsTaggingCmd;
    CPSS_DXCH_BRG_VLAN_INFO_STC             defVlanInfo;
    GT_U32                                  portIter;

    CPSS_MAC_ENTRY_EXT_STC                  macEntry;
    GT_HW_DEV_NUM                           targetHwDevice;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum) && tgfStackingSystemType == PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E)
    {
        /* NOTE : the vid used also in other files (but without similar logic) as
            'HARD CODED' : 'PRV_TGF_NEXTHOPE_VLANID_CNS' ,
            so prvTgfNextHopeVlanid here must be +4K (or +0) value from 'PRV_TGF_NEXTHOPE_VLANID_CNS'

            meaning that we can not decide to work with any prvTgfNextHopeVlanid value
            for eArch devices !
        */
        prvTgfNextHopeVlanid =
            ALIGN_EVID_TO_4K_TILL_MAX_DEV_MAC(PRV_TGF_NEXTHOPE_VLANID_CNS + _4K);
    }
    else
    {
        prvTgfNextHopeVlanid = PRV_TGF_NEXTHOPE_VLANID_CNS;
    }

    /* update next hop VLAN ID according to eArch */
    prvTgfVlanArray[1] = prvTgfNextHopeVlanid;

    /* initialize default Brg settings */

    /* clear entry */
    cpssOsMemSet(&defVlanInfo, 0, sizeof(defVlanInfo));

    /* set default Vlan info entry */
    defVlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    defVlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    defVlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    defVlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    defVlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_FORWARD_E;
    defVlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    defVlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    defVlanInfo.ipv4IgmpToCpuEn      = GT_FALSE;
    defVlanInfo.mirrToRxAnalyzerEn   = GT_FALSE;
    defVlanInfo.ipv6IcmpToCpuEn      = GT_FALSE;
    defVlanInfo.ipCtrlToCpuEn        = PRV_TGF_BRG_IP_CTRL_NONE_E;
    defVlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    defVlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    defVlanInfo.ipv4IpmBrgEn         = GT_FALSE;
    defVlanInfo.ipv6IpmBrgEn         = GT_FALSE;
    defVlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    defVlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    defVlanInfo.ipv4McastRouteEn     = GT_FALSE;
    defVlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    defVlanInfo.ipv6McastRouteEn     = GT_FALSE;
    defVlanInfo.stgId                = 0;
    defVlanInfo.autoLearnDisable     = GT_TRUE;
    defVlanInfo.naMsgToCpuEn         = GT_TRUE;
    defVlanInfo.mruIdx               = 0;
    defVlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    defVlanInfo.vrfId                = 0;
    defVlanInfo.floodVidx            = 0xFFF;/* flood into vlan */

    /* start with no ports */
    /* all ports are 'untagged' */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* clear portsTaggingCmd */
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* add the needed ports to the vlan */
    for(portIter = 0; portIter < 2; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[portIter]);
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsTagging,prvTgfPortsArray[portIter]);

        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = (tagArray[portIter] & 1) ?
            PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E :
            PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    }

    /* default value */
    defVlanInfo.unregIpmEVidx = 0xFFF;
    /* if fidValue was not set with a legal value */
    if(defVlanInfo.fidValue==0)
        defVlanInfo.fidValue = PRV_TGF_SEND_VLANID_CNS;

    /* call device specific API */

    /* AUTODOC: create VLAN 5 with untagged ports 0,1 */
    rc = cpssDxChBrgVlanEntryWrite(prvTgfDevNum,
                                   PRV_TGF_SEND_VLANID_CNS,
                                   &portsMembers,
                                   &portsTagging,
                                   &defVlanInfo,
                                   &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanEntryWrite: %d", prvTgfDevNum);

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* clear portsTaggingCmd */
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* add the needed ports to the vlan */
    for(portIter = 0; portIter < 2; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[portIter+2]);
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsTagging,prvTgfPortsArray[portIter+2]);

        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter+2]] = (tagArray[portIter+2] & 1) ?
            PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E :
            PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;

    }

    /* default value */
    defVlanInfo.unregIpmEVidx = 0xFFF;
    /* if fidValue was not set with a legal value */
    if(defVlanInfo.fidValue==0)
        defVlanInfo.fidValue = prvTgfNextHopeVlanid;

    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = cpssDxChBrgVlanEntryWrite(prvTgfDevNum,
                                   prvTgfNextHopeVlanid,
                                   &portsMembers,
                                   &portsTagging,
                                   &defVlanInfo,
                                   &portsTaggingCmd);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: enable VLAN based MAC learning */
    rc = cpssDxChBrgVlanLearningStateSet(prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * Set the FDB entry With DA_ROUTE
     */

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    rc = cpssDxChCfgHwDevNumGet((GT_U8)prvTgfDevNum, &targetHwDevice);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChCfgHwDevNumGet: %d", prvTgfDevNum);
    macEntry.dstInterface.devPort.hwDevNum  = targetHwDevice;
    macEntry.dstInterface.devPort.portNum   = fdbPortNum;
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = CPSS_MAC_TABLE_FRWRD_E;
    macEntry.saCommand                      = CPSS_MAC_TABLE_FRWRD_E;
    macEntry.daRoute                        = GT_TRUE;
    macEntry.sourceID                       = 0;
    macEntry.userDefined                    = 0;
    macEntry.daQosIndex                     = 0;
    macEntry.saQosIndex                     = 0;
    macEntry.daSecurityLevel                = 0;
    macEntry.saSecurityLevel                = 0;
    macEntry.appSpecificCpuCode             = GT_FALSE;
    macEntry.mirrorToRxAnalyzerPortEn       = GT_FALSE;
    macEntry.age                            = GT_TRUE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    AU_FDB_UPDATE_LOCK();
    rc = cpssDxChBrgFdbMacEntrySet(prvTgfDevNum,&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbMacEntrySet: %d", prvTgfDevNum);
    AU_FDB_UPDATE_UNLOCK();
}

/**
* @internal prvTgfBasicIpv4UcRoutingLttRouteConfiguration2VrfSet
*           function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
*
*/
static GT_VOID prvTgfBasicIpv4UcRoutingLttRouteConfiguration2VrfSet
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex,
    GT_U32   nextHopPortNum
)
{
    GT_STATUS                               rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC         routeEntriesArray[1];
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC         *regularEntryPtr;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_U32                                  ii;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC          vrConfigInfo;
    CPSS_DXCH_IP_LTT_ENTRY_STC              *ipLttEntryPtr = NULL;
    GT_U32                                  numOfPaths;
    GT_HW_DEV_NUM                           targetHwDevice;
    GT_PORT_NUM                             targetPort;

    CPSS_DXCH_BRG_VLAN_INFO_STC             vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC    portsTaggingCmd;
    CPSS_PORTS_BMP_STC                      portsMembers;
    CPSS_PORTS_BMP_STC                      portsTagging;
    GT_BOOL                                 isValid;


    PRV_TGF_PORT_INDEX_CHECK_FOR_VOID_FUNC_MAC(sendPortIndex);

    PRV_UTF_LOG0_MAC("==== Setting LTT Route Configuration ====\n");

    rc = cpssDxChCfgHwDevNumGet((GT_U8)prvTgfDevNum, &targetHwDevice);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChCfgHwDevNumGet: %d", prvTgfDevNum);

    targetPort = nextHopPortNum;

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = cpssDxChIpPortRoutingEnable(prvTgfDevsArray[sendPortIndex],
                                     prvTgfPortsArray[sendPortIndex],
                                     CPSS_IP_UNICAST_E,
                                     CPSS_IP_PROTOCOL_IPV4_E,
                                     GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, sendPortIndex);

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    /* call device specific read API */
    rc = cpssDxChBrgVlanEntryRead(prvTgfDevNum,
                                PRV_TGF_SEND_VLANID_CNS,
                                &portsMembers,
                                &portsTagging,
                                &vlanInfo,
                                &isValid,
                                &portsTaggingCmd);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanEntryRead: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    if (isValid == GT_FALSE)
    {
        /* bypass invalid VLAN entry */
        rc = GT_BAD_STATE;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanEntryRead: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    }

    /* update VRF Id */
    vlanInfo.vrfId = prvUtfVrfId;

    /* call device specific write API */
    rc = cpssDxChBrgVlanEntryWrite(prvTgfDevNum,
                                   PRV_TGF_SEND_VLANID_CNS,
                                   &portsMembers,
                                   &portsTagging,
                                   &vlanInfo,
                                   &portsTaggingCmd);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanEntryWrite: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = cpssDxChBrgVlanIpUcRouteEnable(prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanIpUcRouteEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = cpssDxChIpRouterArpAddrWrite(prvTgfDevNum, prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->type                                               = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;
    regularEntryPtr->entry.regularEntry.cmd                             = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->entry.regularEntry.cpuCodeIdx                      = 0;
    regularEntryPtr->entry.regularEntry.appSpecificCpuCodeEnable        = GT_FALSE;
    regularEntryPtr->entry.regularEntry.unicastPacketSipFilterEnable    = GT_FALSE;
    regularEntryPtr->entry.regularEntry.ttlHopLimitDecEnable            = GT_FALSE;
    regularEntryPtr->entry.regularEntry.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    regularEntryPtr->entry.regularEntry.ingressMirror              = GT_FALSE;
    regularEntryPtr->entry.regularEntry.qosProfileMarkingEnable    = GT_FALSE;
    regularEntryPtr->entry.regularEntry.qosProfileIndex            = 0;
    regularEntryPtr->entry.regularEntry.qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    regularEntryPtr->entry.regularEntry.modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->entry.regularEntry.modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->entry.regularEntry.countSet                   = CPSS_IP_CNT_SET0_E;
    regularEntryPtr->entry.regularEntry.trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->entry.regularEntry.sipAccessLevel             = 0;
    regularEntryPtr->entry.regularEntry.dipAccessLevel             = 0;
    regularEntryPtr->entry.regularEntry.ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->entry.regularEntry.scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->entry.regularEntry.siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->entry.regularEntry.mtuProfileIndex            = 0;
    regularEntryPtr->entry.regularEntry.isTunnelStart              = GT_FALSE;
    regularEntryPtr->entry.regularEntry.nextHopVlanId              = prvTgfNextHopeVlanid;

    regularEntryPtr->entry.regularEntry.nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->entry.regularEntry.nextHopInterface.devPort.hwDevNum = targetHwDevice;
    regularEntryPtr->entry.regularEntry.nextHopInterface.devPort.portNum = targetPort;

    regularEntryPtr->entry.regularEntry.nextHopARPPointer          = prvTgfRouterArpIndex;
    regularEntryPtr->entry.regularEntry.nextHopTunnelPointer       = 0;

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = cpssDxChIpUcRouteEntriesWrite(prvTgfDevNum, (prvTgfRouteEntryBaseIndex+(prvUtfVrfId%2)), routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* AUTODOC: read and check the UC Route entry from the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    rc = cpssDxChIpUcRouteEntriesRead(prvTgfDevNum, (prvTgfRouteEntryBaseIndex+(prvUtfVrfId%2)), routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpUcRouteEntriesRead: %d", prvTgfDevNum);

    PRV_UTF_LOG2_MAC("nextHopVlanId = %d, portNum = %d\n",
                     routeEntriesArray[0].entry.regularEntry.nextHopVlanId,
                     routeEntriesArray[0].entry.regularEntry.nextHopInterface.devPort.portNum);

    /* -------------------------------------------------------------------------
     * 3. Create Virtual Router [prvUtfVrfId]
     */

    /* AUTODOC: create Virtual Router [prvUtfVrfId] */

    if (0 != prvUtfVrfId)
    {
        /* clear entry */
        cpssOsMemSet(&vrConfigInfo, 0,sizeof(CPSS_DXCH_IP_LPM_VR_CONFIG_STC));
        cpssOsMemSet(&vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry, 0,sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        cpssOsMemSet(&vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry, 0,sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        numOfPaths = 0;

        for (ii = 0; ii < 2; ii++)
        {
            if (ii == 0)
            {
                vrConfigInfo.supportIpv4Uc = GT_TRUE;
                ipLttEntryPtr = &vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry;
            }
            if (ii == 1)
            {
                vrConfigInfo.supportIpv6Uc = GT_TRUE;
                ipLttEntryPtr = &vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry;
            }
            /* set defUcRouteEntryInfo */
            ipLttEntryPtr->routeType                = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
            ipLttEntryPtr->numOfPaths               = numOfPaths;
            ipLttEntryPtr->routeEntryBaseIndex      = (prvTgfRouteEntryBaseIndex+(prvUtfVrfId%2));
            ipLttEntryPtr->ucRPFCheckEnable         = GT_FALSE;
            ipLttEntryPtr->sipSaCheckMismatchEnable = GT_FALSE;
            ipLttEntryPtr->ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        }

        /* disable Patricia trie validity */
        prvTgfIpValidityCheckEnable(GT_FALSE);

        /* create Virtual Router [prvUtfVrfId] with created default LTT entries */
        rc = cpssDxChIpLpmVirtualRouterAdd(prvTgfLpmDBId, prvUtfVrfId,&vrConfigInfo);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterAdd: %d",
                                     prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 4. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo.ipLttEntry, 0, sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = (prvTgfRouteEntryBaseIndex+(prvUtfVrfId%2));
    nextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;

    /* AUTODOC: for regular test add IPv4 UC prefix 1.1.1.3/32 */

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    rc = cpssDxChIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId, &ipAddr, 32,
                                      &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

}

/**
* @internal prvTgfBasicIpv4UcRoutingPbrConfiguration2VrfSet function
* @endinternal
*
* @brief   Set PBR Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortIndex            - index of the port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
*/
static GT_VOID prvTgfBasicIpv4UcRoutingPbrConfiguration2VrfSet
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex,
    GT_U32   nextHopPortNum
)
{
    GT_STATUS                               rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_TRUNK_ID senderTrunkId;/* trunk Id for the sender port that is member of */
    GT_TRUNK_ID nextHopTrunkId;/* trunk Id for the next hop port that is member of */
    CPSS_TRUNK_MEMBER_STC   trunkMember;/* temporary trunk member */
    GT_U32                  numOfEnabledMembers;
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32                                  ii;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    *defUcNextHopInfoPtr = NULL;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcNextHopInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcNextHopInfo;

    PRV_TGF_PORT_INDEX_CHECK_FOR_VOID_FUNC_MAC(sendPortIndex);

    PRV_UTF_LOG0_MAC("==== Setting PBR Configuration ====\n");

    /* Routing from trunk support:
       check if the port is trunk member.
       when trunk member , set all the members with same configuration.
    */
    trunkMember.port = prvTgfPortsArray[sendPortIndex];
    trunkMember.hwDevice = prvTgfDevNum;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&senderTrunkId);
    if(rc == GT_OK)
    {
        /* the port is member of the trunk */

        /* get all trunk members */
        numOfEnabledMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
        rc = prvCpssGenericTrunkDbEnabledMembersGet(prvTgfDevNum,senderTrunkId,&numOfEnabledMembers,enabledMembersArray);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvCpssGenericTrunkDbEnabledMembersGet: %d %d",
                                     prvTgfDevNum, senderTrunkId);
        if(numOfEnabledMembers == 0)
        {
            /* empty trunk ??? */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_EMPTY);
            return;
        }

        for(ii = 0 ; ii < numOfEnabledMembers; ii++)
        {
            /* enable Unicast IPv4 Routing on the trunk members */
            rc = prvTgfPclDefPortInitExt1(
                enabledMembersArray[ii].port,
                CPSS_PCL_DIRECTION_INGRESS_E,
                CPSS_PCL_LOOKUP_1_E, /* PBR must be in lookup 1 ! */
                PRV_TGF_PCL_PBR_ID_MAC(prvUtfVrfId), /* pclId */
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E /*ipv6Key*/);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfPclDefPortInitExt1: %d", prvTgfDevNum);
        }
    }
    else
    {
        /* -------------------------------------------------------------------------
         * 1. PCL Config
         */

        /* init PCL Engine for send port */
        rc = prvTgfPclDefPortInitExt1(
            prvTgfPortsArray[sendPortIndex],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_1_E, /* PBR must be in lookup 1 ! */
            PRV_TGF_PCL_PBR_ID_MAC(prvUtfVrfId), /* pclId */
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E /*ipv6Key*/);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInitExt1: %d", prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 2. IP Config
     */

    /* update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* enable IPv4 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* write ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 3. Create Virtual Router [prvUtfVrfId]
     */

    /* AUTODOC: create Virtual Router [prvUtfVrfId] */
    if (0 != prvUtfVrfId)
    {
        /* set next hop to all 0 and change only values other than 0 */
        cpssOsMemSet(&defIpv4UcNextHopInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        cpssOsMemSet(&defIpv6UcNextHopInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

        /* ipv4 uc and ipv6 uc default route */
        for (ii = 0; ii < 2; ii++)
        {
            if (ii == 0)
            {
                defUcNextHopInfoPtr = &defIpv4UcNextHopInfo;
                defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute.arpDaIndex =
                                                               prvTgfRouterArpIndex;
            }
            else
            {
                defUcNextHopInfoPtr = &defIpv6UcNextHopInfo;
            }
            defUcNextHopInfoPtr->pclIpUcAction.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            defUcNextHopInfoPtr->pclIpUcAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
            defUcNextHopInfoPtr->pclIpUcAction.vlan.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
            defUcNextHopInfoPtr->pclIpUcAction.redirect.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute.doIpUcRoute = GT_TRUE;
        }

        /* disable Patricia trie validity */
        prvTgfIpValidityCheckEnable(GT_FALSE);

        rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId,
                                         prvUtfVrfId,
                                         &defIpv4UcNextHopInfo,
                                         &defIpv6UcNextHopInfo,
                                         NULL, NULL, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterAdd: %d",
                                     prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 4. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    nextHopInfo.pclIpUcAction.pktCmd                        = CPSS_PACKET_CMD_FORWARD_E;
    nextHopInfo.pclIpUcAction.mirror.cpuCode                = (CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1);
    nextHopInfo.pclIpUcAction.matchCounter.enableMatchCount = GT_TRUE;
    nextHopInfo.pclIpUcAction.vlan.vlanId                   = prvTgfNextHopeVlanid;
    nextHopInfo.pclIpUcAction.ipUcRoute.doIpUcRoute         = GT_TRUE;
    nextHopInfo.pclIpUcAction.ipUcRoute.arpDaIndex          = prvTgfRouterArpIndex;

    nextHopInfo.pclIpUcAction.redirect.redirectCmd          = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    nextHopInfo.pclIpUcAction.redirect.data.vrfId           = prvUtfVrfId;


    /* Routing to trunk support:
       check if the NH port is trunk member.
       when trunk member --> use the trunk ID.
    */
    trunkMember.port = nextHopPortNum;
    trunkMember.hwDevice = prvTgfDevNum;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&nextHopTrunkId);
    if(rc == GT_OK)
    {
        /* the port is member of the trunk */
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.type     = CPSS_INTERFACE_TRUNK_E;
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.trunkId  = nextHopTrunkId;
    }
    else
    {
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.portNum = nextHopPortNum;
    }

    /* AUTODOC: for regular test add IPv4 UC prefix 1.1.1.3/32 */
    /* AUTODOC: for bulk test add bulk of 2 IPv4 UC prefixes: 1.1.1.3/32, 3.2.2.3/32 */
    /* call CPSS function */
    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfIpLpmIpv4UcPrefixAdd: %d",
                                 prvTgfDevNum);
}

/**
* @internal prvTgfBasicIpv4UcRoutingTraffic2VrfGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] useSecondPkt             - whether to use the second packet (prvTgfPacket2PartArray)
* @param[in] useSecondArp             - whether to use the second ARP MAC address (prvTgfArpMac1)
* @param[in] isUrpfTest               - whether this is unicast RPF test
* @param[in] expectNoTraffic          - whether to expect traffic or not
* @param[in] expectDoubleTag          - whether to expect packet with double tag
*
*/
GT_VOID prvTgfBasicIpv4UcRoutingTraffic2VrfGenerate
(
    GT_U32    sendPortNum,
    GT_U32    nextHopPortNum,
    GT_BOOL   useSecondPkt,
    GT_BOOL   useSecondArp,
    GT_BOOL   isUrpfTest,
    GT_BOOL   expectNoTraffic,
    GT_BOOL   expectDoubleTag
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_PART_STC             *packetPartsPtr;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;
    TGF_MAC_ADDR                    arpMacAddr;
    GT_TRUNK_ID currTrunkId;/* trunk Id for the current port */
    GT_TRUNK_ID senderTrunkId;/* trunk Id for the sender port that is member of */
    GT_TRUNK_ID nextHopTrunkId;/* trunk Id for the next hop port that is member of */
    CPSS_TRUNK_MEMBER_STC   trunkMember;/* temporary trunk member */
    GT_BOOL     stormingDetected;/*indicates that the traffic to check
                   LBH that egress the trunk was originally INGRESSED from the trunk.
                   but since in the 'enhanced UT' the CPU send traffic to a port
                   due to loopback it returns to it, we need to ensure that the
                   member mentioned here should get the traffic since it is the
                   'original sender'*/
    GT_U32      vlanIter;/*vlan iterator*/
    GT_U32      vlanCount = sizeof(prvTgfVlanArray)/sizeof(prvTgfVlanArray[0]);/*number of vlans*/
    GT_U32      numVfd = 0;/* number of VFDs in vfdArray */
    GT_HW_DEV_NUM                           targetHwDevice;
    GT_PORT_NUM                             targetPort;
    GT_U32       egressPortIndex = 0xFF;
    GT_BOOL      isRoutedPacket = GT_FALSE;
    GT_BOOL      is_prvTgfCaptureForceTtiDisableModeSet_GT_TRUE = GT_FALSE;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        if(prvTgfPortsArray[portIter] == nextHopPortNum)
        {
            egressPortIndex = portIter;
            break;
        }
    }

    targetHwDevice = prvTgfDevNum;
    targetPort = nextHopPortNum;

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     *
     */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            /* reset IP couters and set ROUTE_ENTRY mode */
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    /* Routing from trunk support:
       check if the port is trunk member.
       when trunk member , set all the members with same configuration.
    */
    trunkMember.port = sendPortNum;
    trunkMember.hwDevice = prvTgfDevNum;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&senderTrunkId);
    if(rc == GT_OK)
    {
    }
    else
    {
        senderTrunkId = 0;
    }

    /* Routing to trunk support:
       check if the NH port is trunk member.
       when trunk member --> use the trunk ID.
    */
    trunkMember.port = targetPort;
    trunkMember.hwDevice = targetHwDevice;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&nextHopTrunkId);
    if(rc == GT_OK)
    {
        /* the port is member of the trunk */

        /* setup nexthope portInterface for capturing */
        portInterface.type            = CPSS_INTERFACE_TRUNK_E;
        portInterface.trunkId         = nextHopTrunkId;

        /* send 64 packets in SIP5 due to new LBH of the trunk */
        prvTgfBurstCount = 64;

        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 6 + 5;/* last byte of the mac SA */
        cpssOsMemSet(vfdArray[0].patternPtr,0,sizeof(TGF_MAC_ADDR));
        vfdArray[0].patternPtr[0] = 0;
        vfdArray[0].cycleCount = 1;/*single byte*/
        vfdArray[0].mode = TGF_VFD_MODE_INCREMENT_E;

        numVfd = 1;
    }
    else
    {
        nextHopTrunkId = 0;

        /* setup nexthope portInterface for capturing */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = nextHopPortNum;
    }

    if((senderTrunkId != 0) && (nextHopTrunkId == senderTrunkId))
    {
        /* send 64 packets in SIP5 due to new LBH of the trunk */
        prvTgfBurstCount = 64;

        /* make all trunk ports members of the vlans */
        for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
        {
            for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
            {
                rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                            prvTgfPortsArray[portIter], GT_FALSE);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                             prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                             prvTgfPortsArray[portIter], GT_FALSE);
            }
        }

        /* we route from a trunk to the same trunk */
        /* the 'Mirroring capture' is not good because it sets :
           force PVID = 0 for all traffic that ingress the 'Captured port' .
           but since the 'sender' is also member of the 'NH trunk' this port must
           also be 'captured' , but this will not allow the packets from the CPU
           to initiate routing at all !

           so we use 'Special PCL capture' that 'skip' the ingress vlan (before the routing)
           by this the traffic before route can ingress the device but the
           traffic after the route will be captured to the CPU.
           */
        captureType = TGF_CAPTURE_MODE_PCL_E;

        /* state that no need to capture packets that are in the Vlan before the routing */
        rc = tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet(GT_TRUE,PRV_TGF_SEND_VLANID_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet: %d, %d\n",
                                     prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

        if(prvWrAppTrunkPhy1690_WA_B_Get())
        {
            GT_PORT_NUM eportOfTrunk;

            rc = prvWrAppTrunkEPortGet_phy1690_WA_B(senderTrunkId,&eportOfTrunk);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvWrAppTrunkEPortGet_phy1690_WA_B: prvTgfDevNum %d senderTrunkId[%d]",
                                         prvTgfDevNum, senderTrunkId);
            /* we must not kill the TTI lookups on the ingress port !!! */
            /* the WA is based on it */
            prvTgfCaptureForceTtiDisableModeSet(GT_TRUE);
            is_prvTgfCaptureForceTtiDisableModeSet_GT_TRUE = GT_TRUE;
            /* set PCL for the TRAP on the srcEPort */
            rc = prvTgfDxChPclCaptureSet(prvTgfDevNum,eportOfTrunk,GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfDxChPclCaptureSet: prvTgfDevNum %d eportOfTrunk[%d]",
                                         prvTgfDevNum, eportOfTrunk);
        }
    }
    else
    {
        captureType = TGF_CAPTURE_MODE_MIRRORING_E;
    }


    /* enable capture on next hop port/trunk */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, captureType);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    if (useSecondPkt == GT_TRUE)
    {
        partsCount = sizeof(prvTgfPacket2PartArray) / sizeof(prvTgfPacket2PartArray[0]);
        packetPartsPtr = prvTgfPacket2PartArray;
    }
    else
    {
        partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
        packetPartsPtr = prvTgfPacketPartArray;
    }

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(packetPartsPtr, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = packetPartsPtr;

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, numVfd, &vfdArray[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(1, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=1.1.1.1 */
    /* AUTODOC:   dstIP=1.1.1.3 for regular test, 1.1.1.3 or 3.2.2.3 for bulk test */

    /* send packet -- send from specific port -- even if member of a trunk */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, sendPortNum);
    cpssOsTimerWkAfter(1000);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* Routing trunk support:
           check if the port is trunk member.
        */
        trunkMember.port = prvTgfPortsArray[portIter];
        trunkMember.hwDevice = prvTgfDevNum;

        rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&currTrunkId);
        if(rc == GT_OK)
        {
            /* this port is member of trunk */

            if(currTrunkId == nextHopTrunkId)
            {
                /* this port is member of the next hop trunk */

                /* we need to SUM all those ports together before we check */
                /* we do the check in trunk dedicated function for 'target trunk'*/
                /* see after this loop */
                continue;
            }
        }

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);


        /* AUTODOC: verify routed packet on port 3 */
        if (isUrpfTest == GT_TRUE)
        {
            if ( (expectNoTraffic == GT_TRUE && prvTgfPortsArray[portIter]==sendPortNum) ||
                 (expectNoTraffic == GT_FALSE && ((prvTgfPortsArray[portIter]==sendPortNum) || (portIter ==egressPortIndex))) )
            {
                isRoutedPacket = GT_TRUE;
            }
        }
        else
        {   if ( (prvTgfPortsArray[portIter]==sendPortNum) ||
                 ((expectNoTraffic==GT_FALSE)&&(portIter ==egressPortIndex)))
            {
                isRoutedPacket = GT_TRUE;
            }
        }
        if (isRoutedPacket == GT_TRUE)
        {
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

                if((nextHopPortNum == prvTgfPortsArray[portIter]) && (expectDoubleTag)){
                    expectedCntrs.goodOctetsSent.l[0] = expectedCntrs.goodOctetsRcv.l[0]  = (packetSize+4 + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                }

                isRoutedPacket = GT_FALSE;
        }
        else
        {
                 /* for other ports */
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
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

    if (expectNoTraffic == GT_TRUE && isUrpfTest == GT_TRUE)
    {
        /* disable capture on nexthope port , before check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, captureType);
        return;
    }

    if(nextHopTrunkId)
    {
        /* the traffic should egress the next hop trunk */
        /* we need to SUM all those ports together before we check */
        /* we do the check in trunk dedicated function for 'target trunk'*/
        if(nextHopTrunkId == senderTrunkId)
        {
            trunkMember.port = sendPortNum;
            trunkMember.hwDevice = prvTgfDevNum;

            prvTgfTrunkLoadBalanceCheck(nextHopTrunkId,
                PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E,
                prvTgfBurstCount,
                0,/*tolerance*/
                &trunkMember,/* the port in the trunk that sent the traffic */
                &stormingDetected);/*was storming detected*/

            /* it will detect 'storming' but those are only the LBH of this port */
        }
        else
        {
            prvTgfTrunkLoadBalanceCheck(nextHopTrunkId,
                PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E,
                prvTgfBurstCount,
                0,/*tolerance*/
                NULL,/*NA*/
                NULL);/*NA*/
        }
    }


    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    if(expectNoTraffic==GT_FALSE)
    {
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", nextHopPortNum);

        /* get trigger counters where packet has MAC DA as prvTgfArpMac */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        if (useSecondArp)
        {
            cpssOsMemCpy(arpMacAddr, &prvTgfArpMac1, sizeof(TGF_MAC_ADDR));
        }
        else
        {
            cpssOsMemCpy(arpMacAddr, &prvTgfArpMac, sizeof(TGF_MAC_ADDR));
        }
        cpssOsMemCpy(vfdArray[0].patternPtr, arpMacAddr, sizeof(TGF_MAC_ADDR));

        /* disable capture on nexthope port , before check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, captureType);

        if(is_prvTgfCaptureForceTtiDisableModeSet_GT_TRUE)
        {
            CPSS_INTERFACE_INFO_STC eport_portInterface = portInterface;
            /* restore settings */
            prvTgfCaptureForceTtiDisableModeSet(GT_FALSE);
            if(prvWrAppTrunkPhy1690_WA_B_Get())
            {
                GT_PORT_NUM eportOfTrunk;

                rc = prvWrAppTrunkEPortGet_phy1690_WA_B(senderTrunkId,&eportOfTrunk);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvWrAppTrunkEPortGet_phy1690_WA_B: prvTgfDevNum %d senderTrunkId[%d]",
                                             prvTgfDevNum, senderTrunkId);
                /* unset PCL for the TRAP on the srcEPort */
                rc = prvTgfDxChPclCaptureSet(prvTgfDevNum,eportOfTrunk,GT_FALSE);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfDxChPclCaptureSet: prvTgfDevNum %d eportOfTrunk[%d]",
                                             prvTgfDevNum, eportOfTrunk);

                eport_portInterface.type = CPSS_INTERFACE_PORT_E;
                eport_portInterface.devPort.hwDevNum = prvTgfDevNum;
                eport_portInterface.devPort.portNum  = eportOfTrunk;
                tgfTrafficGeneratorExpectedInterfaceEport(PRV_TGF_EXPECTED_INTERFACE_TYPE_EPORT_NUM_E);
            }

            /* check the packets to the CPU */
            rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&eport_portInterface, 1, vfdArray, &numTriggers);
            PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);
        }
        else
        {
            /* check the packets to the CPU */
            rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
            PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);
        }

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        if(portInterface.type  == CPSS_INTERFACE_PORT_E)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                         portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, trunkId [%d] \n",
                                         prvTgfDevNum, nextHopTrunkId);
        }

        if(prvTgfBurstCount >= 32)
        {
            /* AUTODOC: for more than 32 transmits check that got only first 32 matches of the MAC of packet */
            UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFFFFFF, numTriggers,
                "\n for more than 32 transmits check that got only first 32 matches of the MAC of packet \n");
        }
        else
        {
            /* AUTODOC: check if packet has the same MAC DA as ARP MAC */
            /* number of triggers should be according to number of transmit*/
            UTF_VERIFY_EQUAL6_STRING_MAC(((1<<prvTgfBurstCount)-1), numTriggers,
                    "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                    arpMacAddr[0], arpMacAddr[1], arpMacAddr[2],
                    arpMacAddr[3], arpMacAddr[4], arpMacAddr[5]);
        }
    }
    else
    {
        /* disable capture on nexthope port , before check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, captureType);
    }

    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* AUTODOC: get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");

    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* disable the PCL exclude vid capturing */
    rc = tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet(GT_FALSE,0/*don't care*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet: %d, %d\n",
                                 prvTgfDevNum, GT_FALSE);

    /* just for 'cleanup' */
    captureType = TGF_CAPTURE_MODE_MIRRORING_E;

}

/**
* @internal prvTgfBasicIpv4UcRoutingRouteConfiguration2VrfSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortIndex            - index of port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
*
*/
GT_VOID prvTgfBasicIpv4UcRoutingRouteConfiguration2VrfSet
(
    GT_U32   prvUtfVrfId0,
    GT_U32   prvUtfVrfId1,
    GT_U8    sendPortIndex,
    GT_U32   nextHopPortNum0,
    GT_U32   nextHopPortNum1
)
{
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_STATUS                   rc = GT_OK;

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */

    PRV_TGF_PORT_INDEX_CHECK_FOR_VOID_FUNC_MAC(sendPortIndex);

    /* Set the general hashing mode of trunk hash generation based on packet data */
    rc =  cpssDxChTrunkHashGlobalModeSet(prvTgfDevNum,CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChTrunkHashGlobalModeSet: %d", prvTgfDevNum);

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

            prvTgfBasicIpv4UcRoutingPbrConfiguration2VrfSet(prvUtfVrfId0, sendPortIndex,
                                                        nextHopPortNum0);

             prvTgfBasicIpv4UcRoutingPbrConfiguration2VrfSet(prvUtfVrfId1, sendPortIndex,
                                                        nextHopPortNum1);
            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

            prvTgfBasicIpv4UcRoutingLttRouteConfiguration2VrfSet(prvUtfVrfId0, sendPortIndex,
                                                             nextHopPortNum0);

            prvTgfBasicIpv4UcRoutingLttRouteConfiguration2VrfSet(prvUtfVrfId1, sendPortIndex,
                                                             nextHopPortNum1);
            break;

        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:

            break;
    }

    rc = cpssDxChLpmPortSipLookupEnableSet(prvTgfDevNum, prvTgfPortsArray[sendPortIndex], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChLpmPortSipLookupEnableSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBasicIpv4UcRoutingConfiguration2VrfRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] prvUtfVrfId              - second virtual router index
* @param[in] sendPortIndex            - index of port sending traffic
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfBasicIpv4UcRoutingConfiguration2VrfRestore
(
    GT_U32   prvUtfVrfId0,
    GT_U32   prvUtfVrfId1,
    GT_U8    sendPortIndex
)
{
    GT_U16      vlanId=1;
    GT_U32      vlanIter  = 0;
    GT_U32      portIter  = 0;
    GT_U32      vlanCount = sizeof(prvTgfVlanArray)/sizeof(prvTgfVlanArray[0]);
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipAddr;
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    CPSS_MAC_ENTRY_EXT_STC  macEntry;

    PRV_TGF_PORT_INDEX_CHECK_FOR_VOID_FUNC_MAC(sendPortIndex);

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    prvTgfBurstCount = PRV_TGF_BURST_COUNT_DEFAULT_CNS;/* restore value to default */

    /* delete the FDB entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;

    /* AUTODOC: delete FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    AU_FDB_UPDATE_LOCK();
    rc = cpssDxChBrgFdbMacEntryDelete(prvTgfDevNum,&macEntry.key);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbMacEntrySet: %d", prvTgfDevNum);
    AU_FDB_UPDATE_UNLOCK();

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: delete the Ipv4 prefix */
    rc = cpssDxChIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId0,&ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    rc = cpssDxChIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId1,&ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
    {
            /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
            rc = cpssDxChIpPortRoutingEnable(prvTgfDevsArray[sendPortIndex],
                             prvTgfPortsArray[sendPortIndex],
                             CPSS_IP_UNICAST_E,
                             CPSS_IP_PROTOCOL_IPV4_E,
                             GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, sendPortIndex);
    }

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = cpssDxChBrgVlanIpUcRouteEnable(prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanIpUcRouteEnable: %d %d",prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* remove Virtual Router [prvUtfVrfId] */
    if (0 != prvUtfVrfId0)
    {
        rc = cpssDxChIpLpmVirtualRouterDel(prvTgfLpmDBId, prvUtfVrfId0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "cpssDxChIpLpmVirtualRouterDel: %d",
                                     prvTgfDevNum);
    }
    if (0 != prvUtfVrfId1)
    {
        rc = cpssDxChIpLpmVirtualRouterDel(prvTgfLpmDBId, prvUtfVrfId1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "cpssDxChIpLpmVirtualRouterDel: %d",
                                     prvTgfDevNum);
    }

    /* enable Patricia trie validity */
    prvTgfIpValidityCheckEnable(GT_TRUE);


    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

    /* delete port from both VLANs (support route from trunk A to trunk A) */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* delete Ports from VLANs */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            rc = cpssDxChBrgVlanPortDelete(prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                        prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortDelete: %d, %d, %d",
                                         prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                         prvTgfPortsArray[portIter]);
        }

        /* delete VLANs */

        /* AUTODOC: invalidate vlan entries 5,6 */
        rc = cpssDxChBrgVlanEntryInvalidate(prvTgfDevNum,prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }

    /* AUTODOC: disable VLAN based MAC learning */
    rc = cpssDxChBrgVlanLearningStateSet(prvTgfDevNum,PRV_TGF_SEND_VLANID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanLearningStateSet: %d", prvTgfDevNum);

   /* restore VLAN in HW */
    if (isValidStoreCfg==GT_TRUE)
    {
        rc = cpssDxChBrgVlanEntryWrite(prvTgfDevNum,
                                       vlanId,
                                       &portsMembersStoreCfg,
                                       &portsTaggingStoreCfg,
                                       &vlanInfoStoreCfg,
                                       &taggingCmdStoreCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanEntryWrite %d", prvTgfDevNum);
    }

    /* AUTODOC: restore to enhanced crc hash data */
    /* restore global hash mode */
    rc =  cpssDxChTrunkHashGlobalModeSet(prvTgfDevNum,globalHashModeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChTrunkHashGlobalModeSet: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

}

/**
* @internal prvTgfBasicIpv4UcRoutingConfiguration2VrfSave
*           function
* @endinternal
*
* @brief   Save configuration
*
* @param[in] devNum                   - device number
*
* @note none
*
*/
GT_VOID prvTgfBasicIpv4UcRoutingConfiguration2VrfSave
(
    GT_U32   devNum
)
{
    GT_STATUS                   rc=GT_OK;
    GT_U16                      vlanId=1;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Save Base Configuration =======\n");

    /* save default VLAN 1 */
    rc = cpssDxChBrgVlanEntryRead(devNum,
                                  vlanId,
                                  &portsMembersStoreCfg,
                                  &portsTaggingStoreCfg,
                                  &vlanInfoStoreCfg,
                                  &isValidStoreCfg,
                                  &taggingCmdStoreCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanEntryRead %d", devNum);

    /* Get the general hashing mode of trunk hash generation */
    rc = cpssDxChTrunkHashGlobalModeGet(prvTgfDevNum, &globalHashModeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChTrunkHashGlobalModeGet: %d", prvTgfDevNum);

}
